/*	$Id: check_window.c,v 1.1.1.1 2006/09/14 01:59:08 root Exp $ */

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

/*
 *  check all windows.
 */


// test : master
int

cmd_check_window(ac, av)
	int ac;
	char *av[];
{
	printf("start check check_window\n");
	check_window();
	printf("check_window check done\n");
	return(1);
}



#define u64 unsigned long long 
extern u64 __raw__writeq(u64 addr, u64 val);
extern u64 __raw__readq(u64 q);
/*
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
*/

#define BASE_OFF 0x0
#define MASK_OFF 0x1
#define MMAP_OFF 0x2
#define MAX_INDEX 0x8

/* return the index for std[][] which is hitted */
static int hit_entry(unsigned long long basic, int index, unsigned long long std[][3], int cnt)
{
	u64 base, mask, mmap, addr;
	int i;

	if(index > MAX_INDEX  || index < 0)
	  return -1;

	//printf("basic = %llx, std = %llx, index= %x, cnt = %llx\n",basic, std, index, cnt);
	//printf("basic + 0x00 + 0x8 * index = %x, cnt = %llx\n",basic + 0x00 + 0x8 * index, cnt);

	addr = basic + 0x00ull + 0x8ull * index;
	base = __raw__readq(addr);
	  //printf("%d:, addr = %llx, base = %llx\n",  __LINE__, addr,base);

	addr = basic + 0x40ull + 0x8ull * index;
	mask = __raw__readq(addr);
	  //printf("%d: addr = %llx, mask = %llx\n",  __LINE__, addr, mask);

	addr = basic + 0x80ull + 0x8ull * index;
	mmap = __raw__readq(addr);
	//printf("%d: addr = %llx, mmap = %llx\n",  __LINE__, addr, mmap);

	for (i = 0; i <  cnt; i++)
	{
	  
	  //printf("%d: i = %llx\n",  __LINE__, i);

	  if ( base == std[i][BASE_OFF] &&  mask == std[i][MASK_OFF] && mmap == std[i][MMAP_OFF])
	  {
		//printf("%d: i = %llx\n",  __LINE__, 0);
		return i;
	  }
	}
	
	return -1;
}

// check windows configuration by below contions (order isn't considered):
// 1. all entry of the window hit the standard window table;
// 2. all entry of the standard table is in the configration table

#define HIT	  0x1
#define MISS  0x0
#define PASS  0x1
#define FAIL  0x0

static int hit_window(unsigned long long base, unsigned long long std[][3], int cnt)
{
	int i, ret, record;
	int expected_table[]={0x1,0x3,0x7,0xf,0x1f,0x3f,0x7f,0xff};
	

	record = 0;

	//printf("base = %llx, std = %llx, cnt = %llx\n",base, std, cnt);
	for (i = 0; i < MAX_INDEX ; i++)
	{
	  ret = hit_entry(base,i,std,cnt);
	  if ( ret < 0)
		  return  ret;
	  else
		record |= 0x1 << ret;
	}
	
	return (record == expected_table[cnt-1] ? HIT: MISS);
}

static void check_l1xbar(u64 std[][3], int cnt)
{
	u64 base = 0x900000003ff02000ull ;
	int i,ret;

////	//printf("std = %llx, cnt = %llx\n", std, cnt);
	printf("check l1xbar @ cpu-%d:\n", 0);
	for (i = 0; i < 4; i++, base += 0x100) {
	  printf("i = %d, base = %08x\n", i, base);
	  ret = hit_window(base, std, cnt);
	  if ( ret != HIT) {
		printf(" FAIL \n");
		return FAIL;
	  }
	}
	printf(" PASS \n");
	return PASS;
}

static int check_l2xbar(u64 std[][3], int cnt)
{
	u64 base = 0x900000003ff00000ull ;
	int ret;

	//printf("check l2xbar @ cpu-%d:", 0);
	ret = hit_window(base, std, cnt);
	if ( ret != HIT) {
	  ////printf(" FAIL \n");
	  return FAIL;
	}
	////printf(" PASS \n");
	return PASS;
}


//#define LS3B
#ifdef LS3B
static void node1_dump_l1xbar(int node)
{
	u64 q = 0x900010003ff06000 | ((u64)node << 44);
	u64 p;
	int i, j;
	//printf("l1xbar @ cpu-%d\n", node);
	for (i = 0; i < 8; i++) {
		p = q;
		for (j = 0; j < 3; j++) {
			//printf("%llx = %016llx\t\n", p, __raw__readq(p));
			p+=0x40;
			////printf("%p = %llx\n", p, *((unsigned long long *)p));

		}
		//printf("\n");
		q += 0x8;
	}
}

static void node1_dump_ht0(int node)
{
	u64 q = 0x900010003ff06600 | ((u64)node << 44);
	u64 p;
	int i, j;
	//printf("HT0 @ cpu-%d\n", node);
	for (i = 0; i < 8; i++) {
		p = q;
		for (j = 0; j < 3; j++) {
			//printf("%llx = %016llx\t\n", p, __raw__readq(p));
			p+=0x40;
			////printf("%p = %llx\n", p, *((unsigned long long *)p));

		}
		//printf("\n");
		q += 0x8;
	}
}

static void node1_dump_ht1(int node)
{
	u64 q = 0x900010003ff06700 | ((u64)node << 44);
	u64 p;
	int i, j;
	//printf("HT1 @ cpu-%d\n", node);
	for (i = 0; i < 8; i++) {
		p = q;
		for (j = 0; j < 3; j++) {
			//printf("%llx = %016llx\t\n", p, __raw__readq(p));
			p+=0x40;
			////printf("%p = %llx\n", p, *((unsigned long long *)p));

		}
		//printf("\n");
		q += 0x8;
	}
}


static void node1_dump_l2xbar(int node)
{
	u64 q = 0x900010003ff04000 | ((u64)node << 44);
	u64 p;
	int i, j;

	//printf("l2xbar @ cpu-%d\n", node);
	for (i = 0; i < 8; i++) {
		p = q;
		for (j = 0; j < 3; j++) {
			//printf("%llx = %016llx\t\n", p, __raw__readq(p));
			p+=0x40;
			////printf("%p = %llx\n", p, *((unsigned long long *)p));

		}
		//printf("\n");
		q += 0x8;
	}
}

static void node1_dump_pcibar(int node)
{
	u64 q = 0x900010003ff04100 | ((u64)node << 44);
	u64 p;
	int i, j;

	//printf("pcibar @ cpu-%d\n", node);
	for (i = 0; i < 8; i++) {
		p = q;
		for (j = 0; j < 3; j++) {
			//printf("%llx = %016llx\t\n", p, __raw__readq(p));
			p+=0x40;
			////printf("%p = %llx\n", p, *((unsigned long long *)p));

		}
		//printf("\n");
		q += 0x8;
	}
}


#endif

//void dump_showXBAR(void)
// Rules 1: set the windows's register to be zero if it isn't used
// Rules 2: Read out all register value from configurabe windows, and compare it with standard configuration
// Rules 3: don't care the order of windows at this moment
void check_window(void)
{
	int i = 0;
	int ret = 0;

	unsigned long long l1xbar[8][3]={{0x0000000018000000ull,0xfffffffffc000000ull,0x00000efdfc0000f7ull},/*used*/
									 {0x00000e0000000000ull,0xffffff0000000000ull,0x00000e00000000f7ull},/*used*/
									 {0x0000000000000000ull,0x0000000000000000ull,0x0000000000000000ull},/*fixup*/
									 {0x0000000040000000ull,0xffffffffc0000000ull,0x00000e00400000f7ull},/*used*/
									 {0x000000001e000000ull,0xffffffffff000000ull,0x00000e00000000f7ull},/*used*/
									 {0x00000c0000000000ull,0x00000c0000000000ull,0x00000c00000000f7ull},/*fixup*/
									 {0x0000200000000000ull,0x0000200000000000ull,0x00002000000000f7ull},/*fixup*/
									 {0x0000100000000000ull,0x0000300000000000ull,0x00001000000000f7ull},/*fixup*/
  };
	unsigned long long ht0bar[8][3]={
  };// don't care at this moment
	unsigned long long ht1bar[8][3]={
  };
	unsigned long long l2xbar_mc02g[8][3]={
									{0x000000001fc00000ull,0xfffffffffff00000ull,0x000000001fc000f2ull},/*fixup*/
									{0x0000000010000000ull,0xfffffffff0000000ull,0x0000000010000082ull},/*fixup*/
									{0x0000000000000000ull,0xfffffffff0000000ull,0x00000000000000f0ull},/*used*/
									{0x0000000080000000ull,0xffffffff80000000ull,0x00000000000000f0ull},/*used*/
									{0x0000000000000000ull,0x0000000000000000ull,0x0000000000000000ull},/*fixup*/
  };
	unsigned long long l2xbar_mc12g[8][3]={  
									{0x000000001fc00000ull,0xfffffffffff00000ull,0x000000001fc000f2ull},/*fixup*/
									{0x0000000010000000ull,0xfffffffff0000000ull,0x0000000010000082ull},/*fixup*/
									{0x0000000000000000ull,0xfffffffff0000000ull,0x00000000000000f1ull},/*used*/
									{0x0000000080000000ull,0xffffffff80000000ull,0x00000000000000f1ull},/*used*/
									{0x0000000000000000ull,0x0000000000000000ull,0x0000000000000000ull},/*fixup*/
  };       
	unsigned long long l2xbar_2gx2[8][3]={ /* default :ull interleave 13 */ 
									{0x000000001fc00000ull,0xfffffffffff00000ull,0x000000001fc000f2ull},/*fixup*/
									{0x0000000010000000ull,0xfffffffff0000000ull,0x0000000010000082ull},/*fixup*/
									{0x0000000000000000ull,0xfffffffff0000000ull,0x00000000000020f0ull},/*used*/
									{0x0000000000002000ull,0xfffffffff0000000ull,0x00000000000020f1ull},/*used*/
									{0x0000000100000000ull,0xffffffff80002000ull,0x00000000000020f0ull},/*used*/
									{0x0000000100002000ull,0xffffffff80002000ull,0x00000000000020f1ull},/*used*/
									{0x0000000180000000ull,0xffffffff80002000ull,0x00000000000020f0ull},/*used*/
									{0x0000000180002000ull,0xffffffff80002000ull,0x00000000000020f1ull},/*used*/
  };       
	  // check L1XBAR
	  printf("check l1xbaar  @ cpu-%d:", 0);
	  check_l1xbar(l1xbar,8); // only check core(0~3) bar
//	  check_htar(l1xbar,8);	  // check 
//	  check_htrang(l1xbar,8); // check receive windows size, later

	  // check L2XBAR
	  printf("check l2xbar @ cpu-%d:", 0);
	  if ( check_l2xbar(l2xbar_mc02g, 5) ||   check_l2xbar(l2xbar_mc02g, 5) || check_l2xbar(l2xbar_2gx2, 8))
		printf(" PASS \n");
	  else
		printf(" FAIL \n");
	
}


/*
 *
 *  Command table registration
 *  ==========================
 */

static const Cmd Cmds[] =
{
	{"Misc"},
	{"windows",	"", 0, "check all windows configuration for 3A", cmd_check_window, 1, 99, 0},
//	{"interrupt",	"", 0, "List interrupt of all pci/pcie device", cmd_check_interrupt, 1, 99, 0},
	{0, 0}
};

static void init_cmd __P((void)) __attribute__ ((constructor));

void
init_cmd()
{
	cmdlist_expand(Cmds, 1);
}

