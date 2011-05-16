#include <pmon.h>
#include <stdio.h>

typedef unsigned long ul;
typedef unsigned long long ull;
typedef unsigned long volatile ulv;

extern char *getenv (const char *);

int err_continue = 0;
char progress[] = "-\\|/";
#define PROGRESSLEN 4
#define PROGRESSOFTEN 2500
#define UL_LEN 32
#define CHECKERBOARD1 0x55555555
#define CHECKERBOARD2 0xaaaaaaaa
#define UL_ONEBITS 0xffffffff
#define UL_BYTE(x) ((x | x << 8 | x << 16 | x << 24))

struct test
{
	char *name;
	int (*fp)(ulv *, ulv *, size_t );
};

int test_random_value(ulv *bufa, ulv *bufb, size_t count);
int test_xor_comparison(ulv *bufa, ulv *bufb, size_t count); 
int test_sub_comparison(ulv *bufa, ulv *bufb, size_t count); 
int test_mul_comparison(ulv *bufa, ulv *bufb, size_t count);
int test_div_comparison(ulv *bufa, ulv *bufb, size_t count);
int test_or_comparison(ulv *bufa, ulv *bufb, size_t count); 
int test_and_comparison(ulv *bufa, ulv *bufb, size_t count);
int test_seqinc_comparison(ulv *bufa, ulv *bufb, size_t count); 
int test_solidbits_comparison(ulv *bufa, ulv *bufb, size_t count); 
int test_blockseq_comparison(ulv *bufa, ulv *bufb, size_t count); 
int test_checkerboard_comparison(ulv *bufa, ulv *bufb, size_t count); 
int test_bitspread_comparison(ulv *bufa, ulv *bufb, size_t count); 
int test_bitflip_comparison(ulv *bufa, ulv *bufb, size_t count); 
int test_walkbits1_comparison(ulv *bufa, ulv *bufb, size_t count); 
int test_walkbits0_comparison(ulv *bufa, ulv *bufb, size_t count); 

struct test tests[] = {
#if 1
    { "Random Value", test_random_value },
    { "Compare XOR", test_xor_comparison },
    { "Compare SUB", test_sub_comparison },
    { "Compare MUL", test_mul_comparison },
    { "Compare DIV",test_div_comparison },
    { "Compare OR", test_or_comparison },
    { "Compare AND", test_and_comparison },
    { "Sequential Increment", test_seqinc_comparison },
//#else
    { "Solid Bits", test_solidbits_comparison },
    { "Block Sequential", test_blockseq_comparison },
    { "Checkerboard", test_checkerboard_comparison },
    { "Bit Spread", test_bitspread_comparison },
    { "Bit Flip", test_bitflip_comparison },
    { "Walking Ones", test_walkbits1_comparison },
    { "Walking Zeroes", test_walkbits0_comparison },
#endif
    { NULL, NULL }

};

static unsigned int seed_x = 521288629;
static unsigned int seed_y = 362436069;


unsigned int rand_ul ()
   {
   static unsigned int a = 18000, b = 30903;

   seed_x = a*(seed_x&65535) + (seed_x>>16);
   seed_y = b*(seed_y&65535) + (seed_y>>16);

   return ((seed_x<<16) + (seed_y&65535));
   }

int compare_regions(ulv *bufa, ulv *bufb, size_t count) {
    int r = 0;
    size_t i;
    ulv *p1 = bufa;
    ulv *p2 = bufb;

    for (i = 0; i < count; i++, p1++, p2++) {
        if (*p1 != *p2) {
            printf("FAILURE: 0x%08lx != 0x%08lx at offset 0x%08lx.\n", 
                (ul) *p1, (ul) *p2, (ul) i);
            /* printf("Skipping to next test..."); */
            r = -1;
        }
    }
    return r;
}
int test_random_value(ulv *bufa, ulv *bufb, size_t count) {
    ulv *p1 = bufa;
    ulv *p2 = bufb;
    ul j = 0;
    size_t i;

	putchar(' ');
    for (i = 0; i < count; i++) {
        *p1++ = *p2++ = rand_ul();
		if (!(i % PROGRESSOFTEN)) {
			putchar('\b');
			putchar(progress[++j % PROGRESSLEN]);
		}
    }
	printf("\b \b");
    return compare_regions(bufa, bufb, count);
}

int test_xor_comparison(ulv *bufa, ulv *bufb, size_t count) {
    ulv *p1 = bufa;
    ulv *p2 = bufb;
    size_t i;
    ul q = rand_ul();

    for (i = 0; i < count; i++) {
        *p1++ ^= q;
        *p2++ ^= q;
    }
    return compare_regions(bufa, bufb, count);
}

int test_sub_comparison(ulv *bufa, ulv *bufb, size_t count) {
    ulv *p1 = bufa;
    ulv *p2 = bufb;
    size_t i;
    ul q = rand_ul();

    for (i = 0; i < count; i++) {
        *p1++ -= q;
        *p2++ -= q;
    }
    return compare_regions(bufa, bufb, count);
}

int test_mul_comparison(ulv *bufa, ulv *bufb, size_t count) {
    ulv *p1 = bufa;
    ulv *p2 = bufb;
    size_t i;
    ul q = rand_ul();

    for (i = 0; i < count; i++) {
        *p1++ *= q;
        *p2++ *= q;
    }
    return compare_regions(bufa, bufb, count);
}

int test_div_comparison(ulv *bufa, ulv *bufb, size_t count) {
    ulv *p1 = bufa;
    ulv *p2 = bufb;
    size_t i;
    ul q = rand_ul();

    for (i = 0; i < count; i++) {
        if (!q) {
            q++;
        }
        *p1++ /= q;
        *p2++ /= q;
    }
    return compare_regions(bufa, bufb, count);
}

int test_or_comparison(ulv *bufa, ulv *bufb, size_t count) {
    ulv *p1 = bufa;
    ulv *p2 = bufb;
    size_t i;
    ul q = rand_ul();

    for (i = 0; i < count; i++) {
        *p1++ |= q;
        *p2++ |= q;
    }
    return compare_regions(bufa, bufb, count);
}

int test_and_comparison(ulv *bufa, ulv *bufb, size_t count) {
    ulv *p1 = bufa;
    ulv *p2 = bufb;
    size_t i;
    ul q = rand_ul();

    for (i = 0; i < count; i++) {
        *p1++ &= q;
        *p2++ &= q;
    }
    return compare_regions(bufa, bufb, count);
}

int test_seqinc_comparison(ulv *bufa, ulv *bufb, size_t count) {
    ulv *p1 = bufa;
    ulv *p2 = bufb;
    size_t i;
    ul q = rand_ul();

    for (i = 0; i < count; i++) {
        *p1++ = *p2++ = (i + q);
    }
    return compare_regions(bufa, bufb, count);
}
int test_solidbits_comparison(ulv *bufa, ulv *bufb, size_t count) {
    ulv *p1 = bufa;
    ulv *p2 = bufb;
    unsigned int j;
    ul q;
    size_t i;

	printf("           ");
    for (j = 0; j < 64; j++) {
	    printf("\b\b\b\b\b\b\b\b\b\b\b");
        q = (j % 2) == 0 ? UL_ONEBITS : 0;
        printf("setting %3u", j);
        p1 = (ulv *) bufa;
        p2 = (ulv *) bufb;
        for (i = 0; i < count; i++) {
            *p1++ = *p2++ = (i % 2) == 0 ? q : ~q;
        }
        printf("\b\b\b\b\b\b\b\b\b\b\b");
        printf("testing %3u", j);
        if (compare_regions(bufa, bufb, count)) {
            return -1;
        }
    }
    printf("\b\b\b\b\b\b\b\b\b\b\b           \b\b\b\b\b\b\b\b\b\b\b");
    return 0;
}

int test_checkerboard_comparison(ulv *bufa, ulv *bufb, size_t count) {
    ulv *p1 = bufa;
    ulv *p2 = bufb;
    unsigned int j;
    ul q;
    size_t i;

	printf("           ");
    for (j = 0; j < 64; j++) {
	    printf("\b\b\b\b\b\b\b\b\b\b\b");
        q = (j % 2) == 0 ? CHECKERBOARD1 : CHECKERBOARD2;
        printf("setting %3u", j);
        p1 = (ulv *) bufa;
        p2 = (ulv *) bufb;
        for (i = 0; i < count; i++) {
            *p1++ = *p2++ = (i % 2) == 0 ? q : ~q;
        }
        printf("\b\b\b\b\b\b\b\b\b\b\b");
        printf("testing %3u", j);
        if (compare_regions(bufa, bufb, count)) {
            return -1;
        }
    }
    printf("\b\b\b\b\b\b\b\b\b\b\b           \b\b\b\b\b\b\b\b\b\b\b");
    return 0;
}

int test_blockseq_comparison(ulv *bufa, ulv *bufb, size_t count) {
    ulv *p1 = bufa;
    ulv *p2 = bufb;
    unsigned int j;
    size_t i;

	printf("           ");
    for (j = 0; j < 256; j++) {
	    printf("\b\b\b\b\b\b\b\b\b\b\b");
        p1 = (ulv *) bufa;
        p2 = (ulv *) bufb;
        printf("setting %3u", j);
        for (i = 0; i < count; i++) {
            *p1++ = *p2++ = (ul) UL_BYTE(j);
        }
        printf("\b\b\b\b\b\b\b\b\b\b\b");
        printf("testing %3u", j);
        if (compare_regions(bufa, bufb, count)) {
            return -1;
        }
    }
    printf("\b\b\b\b\b\b\b\b\b\b\b           \b\b\b\b\b\b\b\b\b\b\b");
    return 0;
}

int test_walkbits0_comparison(ulv *bufa, ulv *bufb, size_t count) {
    ulv *p1 = bufa;
    ulv *p2 = bufb;
    unsigned int j;
    size_t i;

	printf("           ");
    for (j = 0; j < 64; j++) {
	    printf("\b\b\b\b\b\b\b\b\b\b\b");
        p1 = (ulv *) bufa;
        p2 = (ulv *) bufb;
        printf("setting %3u", j);
        for (i = 0; i < count; i++) {
            if (j < UL_LEN) { /* Walk it up. */
                *p1++ = *p2++ = 0x00000001 << j;
            } else { /* Walk it back down. */
                *p1++ = *p2++ = 0x00000001 << (UL_LEN * 2 - j - 1);
            }
        }
        printf("\b\b\b\b\b\b\b\b\b\b\b");
        printf("testing %3u", j);
        if (compare_regions(bufa, bufb, count)) {
            return -1;
        }
    }
    printf("\b\b\b\b\b\b\b\b\b\b\b           \b\b\b\b\b\b\b\b\b\b\b");
    return 0;
}

int test_walkbits1_comparison(ulv *bufa, ulv *bufb, size_t count) {
    ulv *p1 = bufa;
    ulv *p2 = bufb;
    unsigned int j;
    size_t i;

	printf("           ");
    for (j = 0; j < UL_LEN * 2; j++) {
	    printf("\b\b\b\b\b\b\b\b\b\b\b");
        p1 = (ulv *) bufa;
        p2 = (ulv *) bufb;
        printf("setting %3u", j);
        for (i = 0; i < count; i++) {
            if (j < UL_LEN) { /* Walk it up. */
                *p1++ = *p2++ = UL_ONEBITS ^ (0x00000001 << j);
            } else { /* Walk it back down. */
                *p1++ = *p2++ = UL_ONEBITS ^ (0x00000001 << (UL_LEN * 2 - j - 1));
            }
        }
        printf("\b\b\b\b\b\b\b\b\b\b\b");
        printf("testing %3u", j);
        if (compare_regions(bufa, bufb, count)) {
            return -1;
        }
    }
    printf("\b\b\b\b\b\b\b\b\b\b\b           \b\b\b\b\b\b\b\b\b\b\b");
    return 0;
}

int test_bitspread_comparison(ulv *bufa, ulv *bufb, size_t count) {
    ulv *p1 = bufa;
    ulv *p2 = bufb;
    unsigned int j;
    size_t i;

	printf("           ");
    for (j = 0; j < UL_LEN * 2; j++) {
	    printf("\b\b\b\b\b\b\b\b\b\b\b");
        p1 = (ulv *) bufa;
        p2 = (ulv *) bufb;
        printf("setting %3u", j);
        for (i = 0; i < count; i++) {
            if (j < UL_LEN) { /* Walk it up. */
                *p1++ = *p2++ = (i % 2 == 0)
                    ? (0x00000001 << j) | (0x00000001 << (j + 2))
                    : UL_ONEBITS ^ ((0x00000001 << j)
                                    | (0x00000001 << (j + 2)));
            } else { /* Walk it back down. */
                *p1++ = *p2++ = (i % 2 == 0)
                    ? (0x00000001 << (UL_LEN * 2 - 1 - j)) | (0x00000001 << (UL_LEN * 2 + 1 - j))
                    : UL_ONEBITS ^ (0x00000001 << (UL_LEN * 2 - 1 - j)
                                    | (0x00000001 << (UL_LEN * 2 + 1 - j)));
            }
        }
        printf("\b\b\b\b\b\b\b\b\b\b\b");
        printf("testing %3u", j);
        if (compare_regions(bufa, bufb, count)) {
            return -1;
        }
    }
    printf("\b\b\b\b\b\b\b\b\b\b\b           \b\b\b\b\b\b\b\b\b\b\b");
    return 0;
}

int test_bitflip_comparison(ulv *bufa, ulv *bufb, size_t count) {
    ulv *p1 = bufa;
    ulv *p2 = bufb;
    unsigned int j, k;
    ul q;
    size_t i;

	printf("           ");
    for (k = 0; k < UL_LEN; k++) {
        q = 0x00000001 << k;
        for (j = 0; j < 8; j++) {
    	    printf("\b\b\b\b\b\b\b\b\b\b\b");
            q = ~q;
            printf("setting %3u", k * 8 + j);
            p1 = (ulv *) bufa;
            p2 = (ulv *) bufb;
            for (i = 0; i < count; i++) {
                *p1++ = *p2++ = (i % 2) == 0 ? q : ~q;
            }
            printf("\b\b\b\b\b\b\b\b\b\b\b");
            printf("testing %3u", k * 8 + j);
            if (compare_regions(bufa, bufb, count)) {
                return -1;
            }
        }
    }
    printf("\b\b\b\b\b\b\b\b\b\b\b           \b\b\b\b\b\b\b\b\b\b\b");
    return 0;
}


static int cmd_mt (int argc, char *av[])
{
	int loop=10, i, j;
	int halfsize, size; 
	int count;
	unsigned int base;
	unsigned long *bufa ;
	unsigned long *bufb ;
	char *err_action;
	
	if(argc != 4) {
		printf("Usage: nmt <base> <size> <loop>\n");
		return 0;
	}

	if(err_action= getenv("err_action") && !strcmp("c", err_action)) {
		err_continue = 1;	
	}
	if(!get_rsa(&base, av[1])) {
		return 0;
	}
	base |= 0x80000000;
        if(!get_rsa(&size, av[2])) {
		return 0;
	}
	if(!get_rsa(&loop, av[3])) {
		return 0;
	}

	halfsize = (size/2) << 20;
	count = halfsize/sizeof(unsigned long);

	bufa = (unsigned long *)(base);
	bufb = (unsigned long *)(base + halfsize);
	for(i=0; i<loop; i++){
		printf("LOOP %d\n", i);
		for(j=0;; j++){
			if(!tests[j].name)
				break;
			printf(" %s ", tests[j].name);
			if(!tests[j].fp(bufa, bufb, count))
				printf("ok\n");	
			else {
				printf("%s failure\n", tests[j].name);	
				if(err_continue == 0)
					goto out;
			}
		}
				
	}
out:
	return 0;
}

static int cmd_wm (int argc, char *av[])
{
	__asm__(
			".set mips3\n"
			".set noreorder\n"
			"move $3, $0\n"
			//"dli $3, 0xaaaaaaaaaaaaaaaa\n"
			//"dli $5, 0x5555555555555555\n"
			"li $3, 0xaa\n"
			"li $5, 0x55\n"
			"1:\n"
			"lui $2, 0x8030\n"
			"lui $4, 0x8040\n"
			"2:\n"
			"sb $3, 0x7($2)\n"
			"sb $5, 0xf($2)\n"
			"sb $5, 0x17($2)\n"
			"sb $3, 0x1f($2)\n"
			"nop\n"
			"add $2, 32\n"
			"blt $2, $4, 2b\n"
			"nop\n"
			"b 1b\n"
			"nop\n"
			".set reorder\n"
			".set mips0\n"
			:::"$2","$3","$4","$5"
		   );

	return 0;
}

static const Cmd Cmds[] =
{
	{"memt"},
	{"nmt", "", NULL,
			"Memory test", cmd_mt, 1, 5,0},
	{"wm", "", NULL,
	        "Write Memory", cmd_wm, 1, 1, 0},
	{0, 0}
};


static void init_cmd __P((void)) __attribute__ ((constructor));

static void
init_cmd()
{
	cmdlist_expand(Cmds, 1);
}

