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

/* header files */
#include <linux/zlib.h>
#include <stdio.h>
#include <stdarg.h>
#define TEST_CPU 1
#define TEST_MEM 2
#define TEST_FXP0 4
#define TEST_EM0 8
#define TEST_EM1 16
#define TEST_PCI 32
#define TEST_VIDEO 64
#define TEST_HD 128
#define TEST_KBD 256
#define TEST_SERIAL 512
#define TEST_PPPORT 1024
#define TEST_FLOPPY 2048

#define INFO_Y  24
#define INFO_W  80
extern void delay1(int);
extern void (*__cprint)(int y, int x,int width,char color, const char *text);
static int pause()
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
#include "video.c"
#include "hd.c"
#include "kbd.c"
#include "pci.c"
#include "pp.c"
#include "fd.c"

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



static int cmd_test(int ac,char **av)
{
long tests;
int i;
char *serverip;
char *clientip;
char cmd[200];
tests=strtoul(av[1],0,0);

if(!(serverip=getenv("serverip")))
	serverip="172.16.21.66";
if(!(clientip=getenv("clientip")))
	clientip="172.16.21.65";

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
			serialtest();
			break;
		case TEST_PPPORT:
			pptest();
			break;
		case TEST_FXP0:
			sprintf(cmd,"ifconfig em0 remove;ifconfig em1 remove;ifconfig fxp0 remove;ifconfig fxp0 %s;",clientip);
			do_cmd(cmd);
			printf("Plese plug net wire into fxp0\n");
			pause();
			sprintf(cmd,"ping -c 3 %s",serverip);
			do_cmd(cmd);
			break;
		case TEST_EM0:
			sprintf(cmd,"ifconfig em0 remove;ifconfig em1 remove;ifconfig fxp0 remove;ifconfig em0 %s",clientip);
			do_cmd(cmd);
			printf("Plese plug net wire into em0\n");
			pause();
			sprintf(cmd,"ping -c 3 %s",serverip);
			do_cmd(cmd);
			break;
		case TEST_EM1:
			sprintf(cmd,"ifconfig em0 remove;ifconfig em1 remove;ifconfig fxp0 remove;ifconfig em1 %s;",clientip);
			do_cmd(cmd);
			printf("Plese plug net wire into em1\n");
			pause();
			sprintf(cmd,"ping -c 3 %s",serverip);
			do_cmd(cmd);
			break;
		case TEST_VIDEO:
			videotest();
			break;
		case TEST_HD:
			hdtest();
			break;
		case TEST_KBD:
			kbdtest();
			break;
		case TEST_PCI:
			pcitest();
			break;
		case TEST_FLOPPY:
			fdtest();
			break;
	}
			pause();
}

return 0;
}

//-------------------------------------------------------------------------------------------
static const Cmd Cmds[] =
{
	{"MyCmds"},
	{"test","val",0,"hardware test",cmd_test,0,99,CMD_REPEAT},
	{"serial","val",0,"hardware test",cmd_serial,0,99,CMD_REPEAT},
	{0,0}
};

static void init_cmd __P((void)) __attribute__ ((constructor));
static void init_cmd()
{
	cmdlist_expand(Cmds, 1);
}
