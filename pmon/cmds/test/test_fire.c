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
#define TEST_MEM 1
#define TEST_NET 2
#define TEST_DISK 4
#define TEST_SERIAL 8
#define TEST_PPPORT 16
#define TEST_PCI 32

#define INFO_Y  24
#define INFO_W  80
extern void delay1(int);
extern void (*__cprint)(int y, int x,int width,char color, const char *text);
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
#include "pp.c"
#include "fd.c"
#include "net.c"

#include "../setup.h"


struct setupMenu testmenu={
0,POP_W,POP_H,
(struct setupMenuitem[])
{
{POP_Y,POP_X,1,1,TYPE_NONE,"    board test"},
{POP_Y+1,POP_X,2,2,TYPE_CMD,"(1)memory:${?&#mytest 1}=[on=| _or mytest 1||off=| _andn mytest 1]test 1"},
{POP_Y+2,POP_X,3,3,TYPE_CMD,"(2)netcard :${?&#mytest 2}=[on=| _or mytest 2||off=| _andn mytest 2]test 2"},
{POP_Y+3,POP_X,4,4,TYPE_CMD,"(3)disk:${?&#mytest 4}=[on=| _or mytest 4||off=| _andn mytest 4]test 4"},
{POP_Y+4,POP_X,5,5,TYPE_CMD,"(4)serial:${?&#mytest 8}=[on=| _or mytest 8||off=| _andn mytest 8]test 8"},
{POP_Y+5,POP_X,6,6,TYPE_CMD,"(5)parallel:${?&#mytest 16}=[on=| _or mytest 16||off=| _andn mytest 16]test 16"},
{POP_Y+6,POP_X,7,7,TYPE_CMD,"(6)all selected=test ${#mytest}"},
{POP_Y+7,POP_X,1,1,TYPE_CMD,"(7)quit=| _quit",0},
{}
}
};


static int cmd_test(int ac,char **av)
{
long tests;
int i;
char cmd[200];

__console_alloc();
if(ac==1)
{
do_menu(&testmenu);
return 0;
}
else
tests=strtoul(av[1],0,0);

for(i=0;i<31;i++)
{
	if(!(tests&(1<<i)))continue;
	switch(1<<i)
	{
		case TEST_MEM:
			strcpy(cmd,"mt -v 0x88000000 0x89000000");
			do_cmd(cmd);
			break;
		case TEST_SERIAL:
			serial_selftest(1);
			break;
		case TEST_PPPORT:
			pptest();
			break;
		case TEST_NET:
			net_looptest();
			break;
		case TEST_DISK:
			disktest();
			break;
	}
	if(ac==2)	lpause();
}


return 0;
}

static int cmd_functest(int ac,char **av)
{
char cmd[200];
printf("--------------------memory test---------------\n");
			strcpy(cmd,"mt -v 0x88000000 0x89000000");
			do_cmd(cmd);
printf("--------------------serial test---------------\n");
			serial_selftest(1);
printf("--------------------paraport test---------------\n");
			pptest();
printf("--------------------net test---------------\n");
			net_looptest();
printf("--------------------disk test---------------\n");
			disktest();
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
