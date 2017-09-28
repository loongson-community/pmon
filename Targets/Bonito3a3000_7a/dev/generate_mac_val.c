#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>

#define ETH_ALEN 6
#define EXTRACT_SIZE 10
#define INPUT_POOL_WORDS 128
#define OUTPUT_POOL_WORDS 32
#define SHA_WORKSPACE_WORDS 80
#define K1  0x5A827999L
#define K2  0x6ED9EBA1L
#define K3  0x8F1BBCDCL
#define K4  0xCA62C1D6L

#define f1(x,y,z)   (z ^ (x & (y ^ z)))         /* x ? y : z */
#define f2(x,y,z)   (x ^ y ^ z)                 /* XOR */
#define f3(x,y,z)   ((x & y) + (z & (x ^ y)))

#define min_t(type, x, y) ({                    \
        type __min1 = (x);                      \
        type __min2 = (y);                      \
        __min1 < __min2 ? __min1: __min2; })

int random_read_wakeup_thresh = 64;

struct entropy_store {
        /* read-only data: */
        struct poolinfo *poolinfo;
        u32 *pool;
        const char *name;
        int limit;
        struct entropy_store *pull;

        /* read-write data: */
        unsigned add_ptr;
        int entropy_count;
        int input_rotate;
        u8 *last_data;
};

u32 input_pool_data[INPUT_POOL_WORDS];
u32 blocking_pool_data[OUTPUT_POOL_WORDS];
u32 nonblocking_pool_data[OUTPUT_POOL_WORDS];

struct poolinfo {
        int poolwords;
        int tap1, tap2, tap3, tap4, tap5;
} poolinfo_table[] = {
        /* x^128 + x^103 + x^76 + x^51 +x^25 + x + 1 -- 105 */
        { 128,  103,    76,     51,     25,     1 },
        /* x^32 + x^26 + x^20 + x^14 + x^7 + x + 1 -- 15 */
        { 32,   26,     20,     14,     7,      1 },
};

struct entropy_store input_pool = {
        .poolinfo = &poolinfo_table[0],
        .name = "input",
        .limit = 1,
        .pool = input_pool_data
};

struct entropy_store blocking_pool = {
        .poolinfo = &poolinfo_table[1],
        .name = "blocking",
        .limit = 1,
        .pull = &input_pool,
        .pool = blocking_pool_data
};

struct entropy_store nonblocking_pool = {
        .poolinfo = &poolinfo_table[1],
        .name = "nonblocking",
        .pull = &input_pool,
        .pool = nonblocking_pool_data
};

u32 rol32(u32 word, unsigned int shift)
{
	return (word << shift) | (word >> (32 - shift));
}

void sha_init(u32 *buf)
{
        buf[0] = 0x67452301;
        buf[1] = 0xefcdab89;
        buf[2] = 0x98badcfe;
        buf[3] = 0x10325476;
        buf[4] = 0xc3d2e1f0;
}

void mix_pool_bytes_extract(struct entropy_store *r, const void *in,
                                   int nbytes, u8 out[64])
{
        u32 const twist_table[8] = {
                0x00000000, 0x3b6e20c8, 0x76dc4190, 0x4db26158,
                0xedb88320, 0xd6d6a3e8, 0x9b64c2b0, 0xa00ae278 };
        unsigned long i, j, tap1, tap2, tap3, tap4, tap5;
        int input_rotate;
        int wordmask = r->poolinfo->poolwords - 1;
        const char *bytes = in;
        u32 w;

        /* Taps are constant, so we can load them without holding r->lock.  */
        tap1 = r->poolinfo->tap1;
        tap2 = r->poolinfo->tap2;
        tap3 = r->poolinfo->tap3;
        tap4 = r->poolinfo->tap4;
        tap5 = r->poolinfo->tap5;

        input_rotate = r->input_rotate;
        i = r->add_ptr;

        /* mix one byte at a time to simplify size handling and churn faster */
        while (nbytes--) {
                w = rol32(*bytes++, input_rotate & 31);
                i = (i - 1) & wordmask;

                /* XOR in the various taps */
                w ^= r->pool[i];
                w ^= r->pool[(i + tap1) & wordmask];
                w ^= r->pool[(i + tap2) & wordmask];
                w ^= r->pool[(i + tap3) & wordmask];
                w ^= r->pool[(i + tap4) & wordmask];
                w ^= r->pool[(i + tap5) & wordmask];

                /* Mix the result back in with a twist */
                r->pool[i] = (w >> 3) ^ twist_table[w & 7];

                /*
                 * Normally, we add 7 bits of rotation to the pool.
                 * At the beginning of the pool, add an extra 7 bits
                 * rotation, so that successive passes spread the
                 * input bits across the pool evenly.
                */
                input_rotate += i ? 7 : 14;
        }

        r->input_rotate = input_rotate;
        r->add_ptr = i;

        if (out)
                for (j = 0; j < 16; j++)
                        ((u32 *)out)[j] = r->pool[(i - j) & wordmask];

}

void sha_transform(u32 *digest, const u32 *in,u32 *W)
{
        u32 a, b, c, d, e, t, i;

        for (i = 0; i < 16; i++)
		W[i] = in[i];

	for (i = 0; i < 64; i++)
                W[i+16] = rol32(W[i+13] ^ W[i+8] ^ W[i+2] ^ W[i], 1);

        a = digest[0];
        b = digest[1];
        c = digest[2];
        d = digest[3];
        e = digest[4];

        for (i = 0; i < 20; i++) {
                t = f1(b, c, d) + K1 + rol32(a, 5) + e + W[i];
                e = d; d = c; c = rol32(b, 30); b = a; a = t;
        }

        for (; i < 40; i ++) {
                t = f2(b, c, d) + K2 + rol32(a, 5) + e + W[i];
                e = d; d = c; c = rol32(b, 30); b = a; a = t;
        }

        for (; i < 60; i ++) {
                t = f3(b, c, d) + K3 + rol32(a, 5) + e + W[i];
                e = d; d = c; c = rol32(b, 30); b = a; a = t;
        }

        for (; i < 80; i ++) {
                t = f2(b, c, d) + K4 + rol32(a, 5) + e + W[i];
                e = d; d = c; c = rol32(b, 30); b = a; a = t;
        }

        digest[0] += a;
        digest[1] += b;
        digest[2] += c;
        digest[3] += d;
        digest[4] += e;
}

void extract_buf(struct entropy_store *r, u8 *out)
{
        int i;
        u32 hash[5], workspace[SHA_WORKSPACE_WORDS];
        u8 extract[64];
	u32 tmp_data[OUTPUT_POOL_WORDS];

	for (i =0;i < OUTPUT_POOL_WORDS; i++)
		tmp_data[i] = rand() | (rand() << 16);

        /* Generate a hash across the pool, 16 words (512 bits) at a time */
        sha_init(hash);

        for (i = 0; i < r->poolinfo->poolwords; i += 16)
               sha_transform(hash, &tmp_data[i], workspace);
            //    sha_transform(hash, (u8 *)(r->pool + i), workspace);
        /*
         * We mix the hash back into the pool to prevent backtracking
         * attacks (where the attacker knows the state of the pool
         * plus the current outputs, and attempts to find previous
         * ouputs), unless the hash function can be inverted. By
         * mixing at least a SHA1 worth of hash data back, we make
         * brute-forcing the feedback as hard as brute-forcing the
         * hash.
        */
        mix_pool_bytes_extract(r, hash, sizeof(hash), extract);

        /*
         * To avoid duplicates, we atomically extract a portion of the
         * pool while mixing, and hash one final time.
         */
        sha_transform(hash, (u32 *)extract, workspace);
        memset(extract, 0, sizeof(extract));
        memset(workspace, 0, sizeof(workspace));

        /*
         * In case the hash function has some recognizable output
         * pattern, we fold it in half. Thus, we always feed back
         * twice as much data as we output.
         */
        hash[0] ^= hash[3];
        hash[1] ^= hash[4];
        hash[2] ^= rol32(hash[2], 16);
        memcpy(out, hash, EXTRACT_SIZE);
        memset(hash, 0, sizeof(hash));
}

ssize_t extract_entropy(struct entropy_store *r, void *buf,
                               size_t nbytes, int min, int reserved)
{
        ssize_t ret = 0, i;
        u8 tmp[EXTRACT_SIZE];

	while (nbytes) {
                extract_buf(r, tmp);
/*
                if (r->last_data) {
                        if (!strncmp(tmp, r->last_data, EXTRACT_SIZE))
                                printf("Hardware RNG duplicated output!\n");
                        memcpy(r->last_data, tmp, EXTRACT_SIZE);
                }
*/
                i = min_t(int, nbytes, EXTRACT_SIZE);
                memcpy(buf, tmp, i);
                nbytes -= i;
                buf += i;
                ret += i;
	    }

        /* Wipe data just returned from memory */
        memset(tmp, 0, sizeof(tmp));

        return ret;
}

void get_random_bytes(void *buf, int nbytes)
{
        char *p = buf;
        extract_entropy(&nonblocking_pool, p, nbytes, 0, 0);
}

void eth_random_addr(u8 *addr)
{
        get_random_bytes(addr, ETH_ALEN);

	addr[0] &= 0xfe;        /* clear multicast bit */
        addr[0] |= 0x02;        /* set local assignment bit (IEEE802) */
}

int is_zero_ether_addr(const u8 *addr)
{
        return !(addr[0] | addr[1] | addr[2] | addr[3] | addr[4] | addr[5]);
}

int is_multicast_ether_addr(const u8 *addr)
{
        return (0x01 & addr[0]);
}

int is_valid_ether_addr_linux(const u8 *addr)
{
        /* FF:FF:FF:FF:FF:FF is a multicast address so we don't need to
         * explicitly check for it here. */
        return !is_multicast_ether_addr(addr) && !is_zero_ether_addr(addr);
}

char * generate_mac_val(unsigned char * buf)
{
	while(!is_valid_ether_addr_linux((u8 *)buf))
		eth_random_addr((u8 *)buf);
	return buf;
}
