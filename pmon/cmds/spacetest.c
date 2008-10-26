/*	$Id: spacetest.c,v 1.1.1.1 2006/06/29 06:43:25 cpu Exp $ */

/*
 * Copyright (c) 2000-2001 Opsycon AB  (www.opsycon.se)
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by Opsycon AB, Sweden.
 * 4. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */

#include <stdio.h>
#include <termio.h>
#include <string.h>
#include <setjmp.h>
#include <sys/endian.h>
#include <ctype.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#ifdef _KERNEL
#undef _KERNEL
#include <sys/ioctl.h>
#define _KERNEL
#else
#include <sys/ioctl.h>
#endif

#include <pmon.h>

static const Optdesc s_opts[] =
{
	{"-b", "access bytes"},
	{"-h", "access half-words"},
	{"-w", "access words"},
	{"-x", "swap bytes"},
	{"-d", "access double-words"},
	{0}
};

unsigned char sreadb(unsigned long long addr)
{
	unsigned char value;
	addr &= 0xffffffffULL;
	addr |= 0x9000000000000000ULL; 
	__asm__(
			".set mips3\n"
			"lb $2, (%0)\n"
			"sb $2, (%1)\n"
			".set mips0\n"
			::"r"(addr),"r"(&value)
			:"$2"
			);
	return value;
}

void swriteb(unsigned long long addr, unsigned long long val)
{
	addr |= 0x9000000000000000ULL; 
	__asm__(
			".set mips3\n"
			"sb %1, (%0)\n"
			".set mips0\n"
			::"r"(addr),"r"(val)
			:"memory"
			);
}

unsigned short sreadh(unsigned long long addr)
{
	unsigned short value;
	addr &= 0xffffffffULL;
	addr |= 0x9000000000000000ULL; 
	__asm__(
			".set mips3\n"
			"lh $2, (%0)\n"
			"sh $2, (%1)\n"
			".set mips0\n"
			::"r"(addr),"r"(&value)
			:"$2"
			);
	return value;
}

void swriteh(unsigned long long addr, unsigned long long val)
{
	addr |= 0x9000000000000000ULL; 
	__asm__(
			".set mips3\n"
			"sh %1, (%0)\n"
			".set mips0\n"
			::"r"(addr),"r"(val)
			:"memory"
			);
}


unsigned int sreadw(unsigned long long addr)
{
	unsigned int value;
	addr &= 0xffffffffULL;
	addr |= 0x9000000000000000ULL; 
	__asm__(
			".set mips3\n"
			"lw $2, (%0)\n"
			"sw $2, (%1)\n"
			".set mips0\n"
			::"r"(addr),"r"(&value)
			:"$2"
			);
	return value;
}

void swritew(unsigned long long addr, unsigned long long val)
{
	addr |= 0x9000000000000000ULL; 
	__asm__(
			".set mips3\n"
			"sw %1, (%0)\n"
			".set mips0\n"
			::"r"(addr),"r"(val)
			:"memory"
			);
}

unsigned long long sreadd(unsigned long long addr)
{
	unsigned long long value;
	addr &= 0xffffffffULL;
	addr |= 0x9000000000000000ULL; 
	__asm__(
			".set mips3\n"
			"ld $2, (%0)\n"
			"sd $2, (%1)\n"
			".set mips0\n"
			::"r"(addr),"r"(&value)
			:"$2"
			);
	return value;
}

void swrited(unsigned long long addr, unsigned long long val)
{
	addr |= 0x9000000000000000ULL; 
	__asm__(
			".set mips3\n"
			"sd %1, (%0)\n"
			".set mips0\n"
			::"r"(addr),"r"(val)
			:"memory"
			);
}

int	cmd_sread __P((int, char *[])); 
int	cmd_sset __P((int, char *[])); 
static const Cmd Cmds[] =
{
	{"space test"},
	{"sread",		"[-bhwd] adr size",
			s_opts,
			"read memory",
			cmd_sread, 3, 4, CMD_REPEAT},
	{"sset",		"[-bhwd] adr val_lo [val_hi]",
			s_opts,
			"modify memory",
			cmd_sset, 3, 5, CMD_REPEAT},
	{0, 0},

};

int
cmd_sread (ac, av)
	int ac;
	char *av[];
{
	int datasz = 0;
	int c;
	int addr;
	unsigned long long start, end, size;

	optind = 0;
	while((c = getopt (ac, av, "bhwd")) != EOF) {
		switch (c) {
		case 'd':
			datasz |= 8;
			break;
		case 'w':
			datasz |= 4;
			break;

		case 'h':
			datasz |= 2;
			break;

		case 'b':
			datasz |= 1;
			break;

		default:
			printf("Unsupported option");
			return -1;

		}
	}
	if((datasz & (datasz - 1)) != 0) {
		printf ("multiple data types specified\n");
		return(-1);
	}

	if (!get_rsa (&addr, av[optind++])) {
		return (-1);
	}

	if (!get_rsa (&size, av[optind++])) {
		return (-1);
	}
	if (datasz == 0) datasz = 4;

	start = addr;
	end = addr + (size & (-datasz));

	while (start < end ){
		switch (datasz) {
			case 1:
				printf("%08x: %02x\n", start, sreadb(start));
				break;
			case 2:
				printf("%08x: %04x\n", start, sreadh(start));
				break;
			case 4:
				printf("%08x: %08x\n", start, sreadw(start));
				break;
			case 8:
				printf("%08x: %016llx\n", start, sreadd(start));
				break;
			default:
				break;
		}
		start += datasz;
	}

	return 0;
}

int
cmd_sset (ac, av)
	int ac;
	char *av[];
{
	int datasz = 0;
	int c;
	unsigned long addr;
	unsigned long long value, high = 0;

	optind = 0;
	while((c = getopt (ac, av, "bhwd")) != EOF) {
		switch (c) {
		case 'd':
			datasz |= 8;
			break;
		case 'w':
			datasz |= 4;
			break;

		case 'h':
			datasz |= 2;
			break;

		case 'b':
			datasz |= 1;
			break;

		default:
			printf("Unsupported option");
			return -1;

		}
	}
	if((datasz & (datasz - 1)) != 0) {
		printf ("multiple data types specified\n");
		return(-1);
	}

	if (!get_rsa (&addr, av[optind++])) {
		return (-1);
	}

	if (!get_rsa (&value, av[optind++])) {
		return (-1);
	}

	if (datasz == 0) datasz = 4;

	if (datasz == 8 && ac == 5) {
		if(!get_rsa (&high, av[optind++]))
			return -1;
		value |= high << 32;
	}
	switch (datasz) {
		case 1:
			swriteb(addr, value);
			break;
		case 2:
			swriteh(addr, value);
			break;
		case 4:
			swritew(addr, value);
			break;
		case 8: /*Bug*/
			swrited(addr, value);
			break;
		default:
			break;
	}

	return 0;
}

static void init_cmd __P((void)) __attribute__ ((constructor));

static void
init_cmd()
{
	cmdlist_expand(Cmds, 1);
}
