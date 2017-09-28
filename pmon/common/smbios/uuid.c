/*
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, and the entire permission notice in its entirety,
 *    including the disclaimer of warranties.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote
 *    products derived from this software without specific prior
 *    written permission.
 * 
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE, ALL OF
 * WHICH ARE HEREBY DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 * USE OF THIS SOFTWARE, EVEN IF NOT ADVISED OF THE POSSIBILITY OF SUCH
 * DAMAGE.
 */

#include <string.h>
#include <sys/time.h>

unsigned char smbios_uuid_rtl8168_mac[6];
unsigned char smbios_uuid_e1000e_mac[6];
unsigned char smbios_uuid_mac[6];

static void read_node(uint8_t *node)
{
	int i;
	for(i = 0; i < 6; i++){
#if defined(LOONGSON_3BSINGLE)
#if defined(LOONGSON_3B1500)
		node[i + 10] = smbios_uuid_e1000e_mac[i];
#else
		node[i + 10] = smbios_uuid_rtl8168_mac[i];
#endif
#endif
#if defined(LOONGSON_3ASINGLE)
                node[i + 10] = smbios_uuid_rtl8168_mac[i];
#endif
#if defined(LOONGSON_3ASERVER) || defined(LOONGSON_3BSERVER)
                node[i + 10] = smbios_uuid_e1000e_mac[i];
#endif
#if defined(LOONGSON_3A2H) || defined(LOONGSON_GMAC)
		node[i + 10] = smbios_uuid_mac[i];
#endif
#ifdef LOONGSON_2G5536
		node[i + 10] = smbios_uuid_mac[i];
#endif
	}
	node[0] |= 0x01;
}

static uint64_t read_time(void)
{
	uint64_t t;
	t = tgt_gettime();
	t += (uint64_t)1000 * 1000 * 10 * 60 * 60 * 24 *(17 + 30 + 31 + 365 * 18 + 5); 

	return t;
}

void
read_random(char * out)
{
	int t;

	srand(tgt_gettime());
	t = rand();
	out[9] = t & 0xff;
	out[8] = (t >> 8) & 0xff;

}

void
uuid_generate(char * out)
{
	uint64_t time;

	read_node(out);
	read_random(out);

	time = read_time();
	out[0] = (uint8_t)(time >> 24);
	out[1] = (uint8_t)(time >> 16);
	out[2] = (uint8_t)(time >> 8);
	out[3] = (uint8_t)time;
	out[4] = (uint8_t)(time >> 40);
	out[5] = (uint8_t)(time >> 32);
	out[6] = (uint8_t)(time >> 56);
	out[7] = (uint8_t)(time >> 48);
 
	out[7] = (out[7] & 0x0F) | 0x10;
}

