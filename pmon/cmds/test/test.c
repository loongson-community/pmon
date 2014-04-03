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

#include <machine/cpu.h>

#include <pmon.h>
#include <dev/pci/pcivar.h>
#include <dev/pci/pcidevs.h>
#include <flash.h>
#include <time.h>
#include "mod_vgacon.h"

/* header files */
#include <linux/zlib.h>
#include <stdio.h>
#include <stdarg.h>
#define TEST_CPU 1
#define TEST_MEM 2
#define TEST_FXP0 4
#define TEST_EM0 8
#define TEST_RTE0 16
#define TEST_PCI 32
#define TEST_VIDEO 64
#define TEST_HD 128
#define TEST_KBD 256
#define TEST_SERIAL 512
#define TEST_PPPORT 1024
#define TEST_FLOPPY 2048
#define TEST_ALL	4096

#define INFO_Y  24
#define INFO_W  80
extern void delay1(int);
extern void (*__cprint)(int y, int x,int width,char color, const char *text);
static int cmd_functest(int ac,char **av);
static int lpause()
{
	char cmd[20];
	struct termio tbuf;
	printf("\n");
	__cprint(INFO_Y,0,INFO_W,0x70,"press enter to continue");
	ioctl (STDIN, SETNCNE, &tbuf);
	gets(cmd);
	ioctl (STDIN, TCSETAW, &tbuf);
	__cprint(INFO_Y,0,INFO_W,0x7,0);
	return 0;
}

static int printticks(int n,int d)
{
int i;
char c[4]={'\\','|','/','-'};
for(i=0;i<n;i++)
{
	printf("%c",c[i%4]);
	delay1(d);	
	printf("\b");
}
return 0;
}

#include "cpu.c"
#include "mem.c"
#include "serial.c"
#include "hd.c"
#if NMOD_VGACON
#include "kbd.c"
#include "video.c"
#endif
#include "pci.c"
//#include "pp.c"
//#include "fd.c"

#include "../setup.h"

struct setupMenu nextmenu={
0,10,3,
(struct setupMenuitem[])
{
{20,60,1,1,TYPE_CMD,"GPIO Setup",0,10},
{21,60,2,2,TYPE_CMD,"(1)uart1"},
{22,60,0,0,TYPE_CMD,"(2)uart2"},
{}
}
};

struct setupMenu testmenu={
0,POP_W,POP_H,
(struct setupMenuitem[])
{
{POP_Y,POP_X,1,1,TYPE_NONE,"    board test"},
{POP_Y+1,POP_X,2,2,TYPE_CMD,"(1)cpu:${?&#mytest 1}=[on=| _or mytest 1||off=| _andn mytest 1]test 1"},
{POP_Y+2,POP_X,3,3,TYPE_CMD,"(2)memory:${?&#mytest 2}=[on=| _or mytest 2||off=| _andn mytest 2]test 2"},
{POP_Y+3,POP_X,4,4,TYPE_CMD,"(3)netcard net0:${?&#mytest 4}=[on=| _or mytest 4||off=| _andn mytest 4]test 4"},
{POP_Y+4,POP_X,5,5,TYPE_CMD,"(4)netcard net1:${?&#mytest 8}=[on=| _or mytest 8||off=| _andn mytest 8]test 8"},
{POP_Y+5,POP_X,6,6,TYPE_CMD,"(5)netcard net2:${?&#mytest 16}=[on=| _or mytest 16||off=| _andn mytest 16]test 16"},
{POP_Y+6,POP_X,7,7,TYPE_CMD,"(6)pci:${?&#mytest 32}=[on=| _or mytest 32||off=| _andn mytest 32]test 32"},
{POP_Y+7,POP_X,8,8,TYPE_CMD,"(7)video:${?&#mytest 64}=[on=| _or mytest 64||off=| _andn mytest 64]test 64"},
{POP_Y+8,POP_X,9,9,TYPE_CMD,"(8)harddisk:${?&#mytest 128}=[on=| _or mytest 128||off=| _andn mytest 128]test 128"},
{POP_Y+9,POP_X,10,10,TYPE_CMD,"(9)keyboard:${?&#mytest 256}=[on=| _or mytest 256||off=| _andn mytest 256]test 256"},
{POP_Y+10,POP_X,11,11,TYPE_CMD,"(10)serial:${?&#mytest 512}=[on=| _or mytest 512||off=| _andn mytest 512]test 512"},
{POP_Y+11,POP_X,12,12,TYPE_CMD,"(11)all selected=test ${#mytest}"},
{POP_Y+12,POP_X,1,1,TYPE_CMD,"(12)quit=| _quit",0},
{}
}
};

#ifndef LOONGSON_3A2H
struct setupMenu testmenu1={
	0,POP_W,POP_H,
	(struct setupMenuitem[])
	{
		{POP_Y,POP_X,1,1,TYPE_NONE,"    board test"},
		{POP_Y+1,POP_X,2,2,TYPE_CMD,"(1)cpu:${?&#mytest 1}=[on=| _or mytest 1||off=| _andn mytest 1]test 1"},
		{POP_Y+2,POP_X,3,3,TYPE_CMD,"(2)memory:${?&#mytest 2}=[on=| _or mytest 2||off=| _andn mytest 2]test 2"},
		{POP_Y+3,POP_X,4,4,TYPE_CMD,"(3)netcard net0:${?&#mytest 4}=[on=| _or mytest 4||off=| _andn mytest 16]test 16"},
		{POP_Y+4,POP_X,5,5,TYPE_CMD,"(4)pci:${?&#mytest 32}=[on=| _or mytest 32||off=| _andn mytest 32]test 32"},
		{POP_Y+5,POP_X,6,6,TYPE_CMD,"(5)video:${?&#mytest 64}=[on=| _or mytest 64||off=| _andn mytest 64]test 64"},
		{POP_Y+6,POP_X,7,7,TYPE_CMD,"(6)harddisk:${?&#mytest 128}=[on=| _or mytest 128||off=| _andn mytest 128]test 128"},
		{POP_Y+7,POP_X,8,8,TYPE_CMD,"(7)keyboard:${?&#mytest 256}=[on=| _or mytest 256||off=| _andn mytest 256]test 256"},
		{POP_Y+8,POP_X,9,9,TYPE_CMD,"(8)serial:${?&#mytest 512}=[on=| _or mytest 512||off=| _andn mytest 512]test 512"},
		{POP_Y+9,POP_X,10,10,TYPE_CMD,"(9)all selected:${?&#mytest 4096}=[on=| _or mytest 4096||off=| _andn mytest 4096]test 4096"},
		{POP_Y+10,POP_X,1,1,TYPE_CMD,"(10)quit=| _quit",0},
		{}
	}
};
#else
#if (defined LOONGSON_3A2H) || (defined LOONGSON_3C2H)
struct setupMenu testmenu1={
	0,POP_W,POP_H,
	(struct setupMenuitem[])
	{
		{POP_Y,POP_X,1,1,TYPE_NONE,"    board test"},
		{POP_Y+1,POP_X,2,2,TYPE_CMD,"(1)cpu:${?&#mytest 1}=[on=| _or mytest 1||off=| _andn mytest 1]test 1"},
		{POP_Y+2,POP_X,3,3,TYPE_CMD,"(2)memory:${?&#mytest 2}=[on=| _or mytest 2||off=| _andn mytest 2]test 2"},
		{POP_Y+3,POP_X,4,4,TYPE_CMD,"(3)netcard net0:${?&#mytest 4}=[on=| _or mytest 4||off=| _andn mytest 16]test 16"},
		{POP_Y+4,POP_X,5,5,TYPE_CMD,"(4)video:${?&#mytest 64}=[on=| _or mytest 64||off=| _andn mytest 64]test 64"},
		{POP_Y+5,POP_X,6,6,TYPE_CMD,"(5)harddisk:${?&#mytest 128}=[on=| _or mytest 128||off=| _andn mytest 128]test 128"},
		{POP_Y+6,POP_X,7,7,TYPE_CMD,"(6)keyboard:${?&#mytest 256}=[on=| _or mytest 256||off=| _andn mytest 256]test 256"},
		{POP_Y+7,POP_X,8,8,TYPE_CMD,"(7)serial:${?&#mytest 512}=[on=| _or mytest 512||off=| _andn mytest 512]test 512"},
		{POP_Y+8,POP_X,9,9,TYPE_CMD,"(8)all selected:${?&#mytest 4096}=[on=| _or mytest 4096||off=| _andn mytest 4096]test 4096"},
		{POP_Y+9,POP_X,1,1,TYPE_CMD,"(9)quit=| _quit",0},
		{}
	}
};
#endif
#endif

static int cmd_test(int ac,char **av)
{
	long tests;
	int i;
	char *serverip;
	char *clientip;
	char cmd[200];

	__console_alloc();
	if(ac==1)
	{
		if(getenv("testem")) do_menu(&testmenu);
		else do_menu(&testmenu1);
		return 0;
	}
	else
		tests=strtoul(av[1],0,0);

	if(!(serverip=getenv("serverip")))
		serverip="10.2.5.22";
	if(!(clientip=getenv("clientip")))
		clientip="10.2.1.104";

	for(i=0;i<31;i++)
	{
		if(!(tests&(1<<i)))continue;
		switch(1<<i)
		{
			case TEST_CPU:
				cputest();
				break;
			case TEST_MEM:
				memtest();
				break;
			case TEST_SERIAL:
				//serialtest();
				serial_selftest(1);
				break;

				/* no paraport dirver support in 3A780E */
				/*
				   case TEST_PPPORT:
				   pptest();
				   break;
				   */
			case TEST_FXP0:
				sprintf(cmd,"ifconfig em0 remove;ifconfig em1 remove;ifconfig fxp0 remove;ifconfig fxp0 %s;",clientip);
				do_cmd(cmd);
				printf("Plese plug net wire into fxp0\n");
				lpause();
				sprintf(cmd,"ping -c 8 %s",serverip);
				do_cmd(cmd);
				break;
			case TEST_EM0:
				sprintf(cmd,"ifconfig em0 remove;ifconfig em1 remove;ifconfig fxp0 remove;ifconfig em0 %s",clientip);
				do_cmd(cmd);
				printf("Plese plug net wire into em0\n");
				lpause();
				sprintf(cmd,"ping -c 8 %s",serverip);
				do_cmd(cmd);
				break;
			case TEST_RTE0:
				sprintf(cmd,"ifconfig em0 remove;ifconfig em1 remove;ifconfig fxp0 remove;ifconfig rte0 %s;",clientip);
				do_cmd(cmd);
				lpause();
				sprintf(cmd,"ping -c 8 %s",serverip);
				do_cmd(cmd);
				break;
#if NMOD_VGACON
			case TEST_VIDEO:
				videotest();
				break;
#endif
			case TEST_HD:
				hdtest();
				break;
#if NMOD_VGACON
			case TEST_KBD:
				kbdtest();
				break;
#endif
			case TEST_PCI:
				pcitest();
				break;

			case TEST_ALL:
				cmd_functest(1,NULL);
				break;
		}

		/* no floppy dirver support now in 3A780E */
		/*
		   case TEST_FLOPPY:
		   fdtest();
		   break;
		   */
		lpause();
	}
}

static int cmd_functest(int ac,char **av)
{
	char *serverip;
	char *clientip;
	char cmd[200];

	if(!(serverip=getenv("serverip")))
		serverip="10.2.5.22";
	if(!(clientip=getenv("clientip")))
		clientip="10.2.1.104";

	printf("--------------------cpu test---------------\n");
	cputest();
	printf("--------------------memory test---------------\n");
	strcpy(cmd,"mt -v 0x88000000 0x89000000");
	do_cmd(cmd);
	printf("--------------------serial test---------------\n");
	serial_selftest(1);
#if 0
	printf("--------------------paraport test---------------\n");
	pptest();
#endif
	printf("--------------------net test---------------\n");
	sprintf(cmd,"ifconfig em0 remove;ifconfig em1 remove;ifconfig fxp0 remove;ifconfig rte0 %s;",clientip);
	do_cmd(cmd);
	sprintf(cmd,"ping -c 8 %s",serverip);
	do_cmd(cmd);
#if NMOD_VGACON
	printf("--------------------video test---------------\n");
	videotest();
	printf("--------------------keyboard test---------------\n");
	kbdtest();
#endif

	printf("--------------------hd test---------------\n");
	hdtest();
	printf("--------------------disk test---------------\n");
	disktest();
	printf("--------------------pci test---------------\n");
	pcitest();
	printf("--------------------all done---------------\n");

	return 0;
}



//-------------------------------------------------------------------------------------------
static const Cmd Cmds[] =
{
	{"MyCmds"},
	{"test","val",0,"hardware test",cmd_test,0,99,CMD_REPEAT},
	{"functest","",0,"function test",cmd_functest,0,99,CMD_REPEAT},
	{"serial","val",0,"hardware test",cmd_serial,0,99,CMD_REPEAT},
	{0,0}
};

static void init_cmd __P((void)) __attribute__ ((constructor));
static void init_cmd()
{
	cmdlist_expand(Cmds, 1);
}
