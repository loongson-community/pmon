/*	$Id: showwindows.c,v 1.1.1.1 2006/09/14 01:59:08 root Exp $ */

/*
 * Copyright (c) 2001 Opsycon AB  (www.opsycon.se)
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
#include <string.h>
#include <stdlib.h>
#include <sys/device.h>
#include <sys/queue.h>

#include <pmon.h>

static int rtest(unsigned long long start, unsigned long long end, unsigned long long step);
static int wtest(unsigned long long start, unsigned long long end, unsigned long long step);
static int wrtest(unsigned long long start, unsigned long long end, unsigned long long step);
/*
 *  Show all windows.
 */


// test : master
int
cmd_showwindows(ac, av)
	int ac;
	char *av[];
{
	printf("start showwindows\n");
	showwindows();
	printf("showwindows done\n");
	return(1);
}

int 
cmd_wtest(ac, av)
	int ac;
	char *av[];
{
	printf("start write slef rtest\n");
	wtest(0x9000000006000000,0x9000000008000000, 0x100000);
	printf("start write CPU1 rtest\n");
	wtest(0x9000100006000000,0x9000100008000000, 0x100000);
	printf("start write CPU3 rtest\n");
	wtest(0x9000300006000000,0x9000300008000000, 0x100000);
	printf("\n end wtest\n");
	//printf("start group wtest\n");
	//wtest(0x98001e0006000000,0x98001e0008000000, 0x100000);
	//printf("end wtest\n");
	return(1);
}

cmd_rtest(ac, av)
	int ac;
	char *av[];
{
	printf("start read slef rtest\n");
	rtest(0x9000000006000000,0x9000000008000000, 0x100000);
	printf("start read CPU1 rtest\n");
	rtest(0x9000100006000000,0x9000100008000000, 0x100000);
	printf("start read CPU3 rtest\n");
	rtest(0x9000300006000000,0x9000300008000000, 0x100000);
	printf("\n end rtest\n");
	//printf("start group wtest\n");
	//rtest(0x98001e0006000000,0x98001e0008000000, 0x100000);
	//printf("end wtest\n");

	return(1);
}

cmd_wrtest(ac, av)
	int ac;
	char *av[];
{
/*
	printf("start slef wrtest\n");
	wtest(0x9800100006000000,0x9800100008000000, 0x100000);
	rtest(0x9800100006000000,0x9800100008000000, 0x100000);
	printf("end slef wrtest\n");
*/
	//printf("start access SYS1 CPU1 HT config space\n");
	//rtest(0x90001efdfe000050,0x90001efdfe000060, 0x8);

	printf("start cached group wrtest: SYS0 write SYS1 \n");
	wrtest(0x98001e0006000000,0x98001e0008000000, 0x100000);
	
	//printf("start uncached group wrtest: SYS0 write SYS1 \n");
	//wrtest(0x90001e0006000000,0x90001e0008000000, 0x100000);

	//rtest(0x90001e0006000000,0x90001e0008000000, 0x100000);
	//wtest(0x90001e0006000000,0x90001e0008000000, 0x100000);
	//rtest(0x90001e0006000000,0x90001e0008000000, 0x100000);

	//rtest(0x98001e0006000000,0x98001e0008000000, 0x100000);
	//wtest(0x98001e0006000000,0x98001e0008000000, 0x100000);
	//rtest(0x98001e0006000000,0x98001e0008000000, 0x100000);
	printf("end group wrtest\n");
	return(1);
}

cmd_wrtest_sys1(ac, av)
	int ac;
	char *av[];
{
/*
	printf("start slef wrtest\n");
	wtest(0x9800100006000000,0x9800100008000000, 0x100000);
	rtest(0x9800100006000000,0x9800100008000000, 0x100000);
	printf("end slef wrtest\n");
*/
	//printf("start access SYS1 CPU1 HT config space\n");
	//rtest(0x90001efdfe000050,0x90001efdfe000060, 0x8);

	printf("start cached group wrtest: SYS0 write SYS1 \n");
	wrtest(0x98001f0006000000,0x98001f0008000000, 0x100000);
	
	//printf("start uncached group wrtest: SYS0 write SYS1 \n");
	//wrtest(0x90001e0006000000,0x90001e0008000000, 0x100000);

	//rtest(0x90001e0006000000,0x90001e0008000000, 0x100000);
	//wtest(0x90001e0006000000,0x90001e0008000000, 0x100000);
	//rtest(0x90001e0006000000,0x90001e0008000000, 0x100000);

	//rtest(0x98001e0006000000,0x98001e0008000000, 0x100000);
	//wtest(0x98001e0006000000,0x98001e0008000000, 0x100000);
	//rtest(0x98001e0006000000,0x98001e0008000000, 0x100000);
	printf("end group wrtest\n");
	return(1);
}


cmd_wrtest_B1(ac, av)
	int ac;
	char *av[];
{
/*
	printf("start slef wrtest\n");
	wtest(0x9800100006000000,0x9800100008000000, 0x100000);
	rtest(0x9800100006000000,0x9800100008000000, 0x100000);
	printf("end slef wrtest\n");
*/
	//printf("start access SYS0 CPU3 HT config space\n");
	//rtest(0x90003ffdfe000050,0x90003ffdfe000060, 0x8);

	printf("start cached group wrtest: SYS1 write SYS0 \n");
	wrtest(0x98003e0006000000,0x98003e0008000000, 0x100000);
	
	//printf("start uncached group wrtest: SYS1 write SYS0 \n");
	//wrtest(0x90003e0006000000,0x90003e0008000000, 0x100000);

	//rtest(0x90001e0006000000,0x90001e0008000000, 0x100000);
	//wtest(0x90001e0006000000,0x90001e0008000000, 0x100000);
	//rtest(0x90001e0006000000,0x90001e0008000000, 0x100000);

	//rtest(0x98001e0006000000,0x98001e0008000000, 0x100000);
	//wtest(0x98001e0006000000,0x98001e0008000000, 0x100000);
	//rtest(0x98001e0006000000,0x98001e0008000000, 0x100000);
	printf("end group wrtest\n");
	return(1);
}


#define u64 unsigned long long 
extern u64 __raw__readq(u64 addr);
extern u64 __raw__writeq(u64 addr, u64 val);
#if 0
u64 __raw__readq(u64 q)
{
  u64 ret;

  asm volatile(
	  ".set mips3;\r\n"  \
	  "move	$8,%1;\r\n"	\
	  "ld	%0,0($8);\r\n"	\
	  :"=r"(ret)
	  :"r" (q)
	  :"$8");

	return ret;
}

u64 __raw__writeq(u64 addr, u64 val)
{
  
  u64 ret;

  asm volatile(
	  ".set mips3;\r\n"  \
	  "move	$8,%1;\r\n"	\
	  "move	$9,%2;\r\n"	\
	  "sd	$9,0($8);\r\n"	\
	  "ld	%0,0($8);\r\n"	\
	  :"=r"(ret)
	  :"r" (addr), "r" (val)
	  :"$8", "$9");

	return ret;
}
#endif


static void dump_l1xbar(int node)
{
	u64 q = 0x900000003ff02000 | ((u64)node << 44);
	u64 p;
	int i, j;
	printf("l1xbar @ cpu-%d\n", node);
	for (i = 0; i < 8; i++) {
		p = q;
		for (j = 0; j < 3; j++) {
			printf("%llx = %016llx\t\n", p, __raw__readq(p));
			p+=0x40;
			//printf("%p = %llx\n", p, *((unsigned long long *)p));

		}
		printf("\n");
		q += 0x8;
	}
}

static void dump_ht0(int node)
{
	u64 q = 0x900000003ff02600 | ((u64)node << 44);
	u64 p;
	int i, j;
	printf("HT0 @ cpu-%d\n", node);
	for (i = 0; i < 8; i++) {
		p = q;
		for (j = 0; j < 3; j++) {
			printf("%llx = %016llx\t\n", p, __raw__readq(p));
			p+=0x40;
			//printf("%p = %llx\n", p, *((unsigned long long *)p));

		}
		printf("\n");
		q += 0x8;
	}
}

static void dump_ht1(int node)
{
	u64 q = 0x900000003ff02700 | ((u64)node << 44);
	u64 p;
	int i, j;
	printf("HT1 @ cpu-%d\n", node);
	for (i = 0; i < 8; i++) {
		p = q;
		for (j = 0; j < 3; j++) {
			printf("%llx = %016llx\t\n", p, __raw__readq(p));
			p+=0x40;
			//printf("%p = %llx\n", p, *((unsigned long long *)p));

		}
		printf("\n");
		q += 0x8;
	}
}


static void dump_l2xbar(int node)
{
	u64 q = 0x900000003ff00000 | ((u64)node << 44);
	u64 p;
	int i, j;

	printf("l2xbar @ cpu-%d\n", node);
	for (i = 0; i < 8; i++) {
		p = q;
		for (j = 0; j < 3; j++) {
			printf("%llx = %016llx\t\n", p, __raw__readq(p));
			p+=0x40;
			//printf("%p = %llx\n", p, *((unsigned long long *)p));

		}
		printf("\n");
		q += 0x8;
	}
}

static void dump_pcibar(int node)
{
	u64 q = 0x900000003ff00100 | ((u64)node << 44);
	u64 p;
	int i, j;

	printf("pcibar @ cpu-%d\n", node);
	for (i = 0; i < 8; i++) {
		p = q;
		for (j = 0; j < 3; j++) {
			printf("%llx = %016llx\t\n", p, __raw__readq(p));
			p+=0x40;
			//printf("%p = %llx\n", p, *((unsigned long long *)p));

		}
		printf("\n");
		q += 0x8;
	}
}

//#define LS3B
#ifdef LS3B
static void node1_dump_l1xbar(int node)
{
	u64 q = 0x900010003ff06000 | ((u64)node << 44);
	u64 p;
	int i, j;
	printf("l1xbar @ cpu-%d\n", node);
	for (i = 0; i < 8; i++) {
		p = q;
		for (j = 0; j < 3; j++) {
			printf("%llx = %016llx\t\n", p, __raw__readq(p));
			p+=0x40;
			//printf("%p = %llx\n", p, *((unsigned long long *)p));

		}
		printf("\n");
		q += 0x8;
	}
}

static void node1_dump_ht0(int node)
{
	u64 q = 0x900010003ff06600 | ((u64)node << 44);
	u64 p;
	int i, j;
	printf("HT0 @ cpu-%d\n", node);
	for (i = 0; i < 8; i++) {
		p = q;
		for (j = 0; j < 3; j++) {
			printf("%llx = %016llx\t\n", p, __raw__readq(p));
			p+=0x40;
			//printf("%p = %llx\n", p, *((unsigned long long *)p));

		}
		printf("\n");
		q += 0x8;
	}
}

static void node1_dump_ht1(int node)
{
	u64 q = 0x900010003ff06700 | ((u64)node << 44);
	u64 p;
	int i, j;
	printf("HT1 @ cpu-%d\n", node);
	for (i = 0; i < 8; i++) {
		p = q;
		for (j = 0; j < 3; j++) {
			printf("%llx = %016llx\t\n", p, __raw__readq(p));
			p+=0x40;
			//printf("%p = %llx\n", p, *((unsigned long long *)p));

		}
		printf("\n");
		q += 0x8;
	}
}


static void node1_dump_l2xbar(int node)
{
	u64 q = 0x900010003ff04000 | ((u64)node << 44);
	u64 p;
	int i, j;

	printf("l2xbar @ cpu-%d\n", node);
	for (i = 0; i < 8; i++) {
		p = q;
		for (j = 0; j < 3; j++) {
			printf("%llx = %016llx\t\n", p, __raw__readq(p));
			p+=0x40;
			//printf("%p = %llx\n", p, *((unsigned long long *)p));

		}
		printf("\n");
		q += 0x8;
	}
}

static void node1_dump_pcibar(int node)
{
	u64 q = 0x900010003ff04100 | ((u64)node << 44);
	u64 p;
	int i, j;

	printf("pcibar @ cpu-%d\n", node);
	for (i = 0; i < 8; i++) {
		p = q;
		for (j = 0; j < 3; j++) {
			printf("%llx = %016llx\t\n", p, __raw__readq(p));
			p+=0x40;
			//printf("%p = %llx\n", p, *((unsigned long long *)p));

		}
		printf("\n");
		q += 0x8;
	}
}


#endif
//void dump_showXBAR(void)
void showwindows(void)
{
	int i = 0;

#ifdef MULTI_CHIP
	for ( i = 0; i < 4; i++)
#else
	for ( i = 0; i < 1; i++)
#endif

	{
	  
		dump_l1xbar(i);
		dump_ht0(i);
		dump_ht1(i);
		dump_l2xbar(i);
		dump_pcibar(i);
	}

#ifdef LS3B
#ifdef MULTI_CHIP
	for ( i = 1; i < 5; i++)
#else
	for ( i = 1; i < 2; i++)
#endif

	{
	  
		node1_dump_l1xbar(i);
		node1_dump_ht0(i);
		node1_dump_ht1(i);
		node1_dump_l2xbar(i);
		node1_dump_pcibar(i);
	}


#endif
	  
}

/*************************************************/
//#define MEMTEST(d, s, n)	memcpy((d), (s), (n))
//extern void godson_memcpy2r(unsigned char *,unsigned  char *, unsigned);
#define MEMTEST(d, s, n)	godson_memcpy2r((d), (s), (n))
#define COPYSIZE	1024
//unsigned long long AA[1048576];
//unsigned long long BB[1048576];

static int wtest(unsigned long long start, unsigned long long end, unsigned long long step)
{
	int i = 0;
	unsigned long long tmp;

	printf("enter wtest\n");

	while(start < end)
	{
	  tmp = __raw__writeq(start,start);
	  if (tmp != start)
		printf("write error: @%llx == %llx\n", start, tmp);
	  start+=step;
	  printf(".");
	}

	printf("\nreturn from wtest\n");
	return 0;
}

static int rtest(unsigned long long start, unsigned long long end, unsigned long long step)
{
	unsigned long long   tmp;
	int i = 0;

	printf("enter rtest\n");

	while(start < end )
	{
	  tmp =  __raw__readq(start);
	  if (tmp != start)
		printf("Read error: @%llx == %llx\n", start,tmp);
	  start+=step;
	  printf("=");
	}

	printf("\nreturn from rtest\n");
	return 0;
}

static int wrtest(unsigned long long start, unsigned long long end, unsigned long long step)
{
	wtest(start, end, step);
	rtest(start, end, step);

}

/*
 *
 *  Command table registration
 *  ==========================
 */

static const Cmd Cmds[] =
{
	{"Misc"},
	{"showwindows",	"", 0, "Show all windows configuration for 3A", cmd_showwindows, 1, 99, 0},
#if 0
	{"wtest",	"", 0, "write meory test", cmd_wtest, 1, 99, 0},
	{"rtest",	"", 0, "read meory test", cmd_rtest, 1, 99, 0},
	{"wrtest",	"", 0, "write/read meory test", cmd_wrtest, 1, 99, 0},
	{"wrtestb",	"", 0, "write/read meory test", cmd_wrtest_B1, 1, 99, 0},
	{"wrtestsys1",	"", 0, "write/read meory test", cmd_wrtest_sys1, 1, 99, 0},
#endif
	{0, 0}
};

static void init_cmd __P((void)) __attribute__ ((constructor));

void
init_cmd()
{
	cmdlist_expand(Cmds, 1);
}

