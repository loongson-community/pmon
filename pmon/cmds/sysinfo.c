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
#include "setup.h"
extern unsigned long long memorysize_total;
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


extern void (*__msgbox)(int yy,int xx,int height,int width,char *msg);
static int cpuinfo(){

#if defined(LOONGSON_2G1A) || defined(LOONGSON_2F1A)
#define MEM_BASE_CLK_2G 25
	int clk567, clk89;
#endif
	int	memsize, freq;
	char	fs[10], *fp;
	char	*s;
	int clk,clk30,clk4, clk20, clk34, mem_vco;
	unsigned short div_refc, div_loopc, div_out;
        int memfreq;
	printf("cpu info:\n");
	freq = tgt_pipefreq ();
	sprintf(fs, "%d", freq);
	fp = fs + strlen(fs) - 6;
	fp[3] = '\0';
	fp[2] = fp[1];
	fp[1] = fp[0];
	fp[0] = '.';
	printf (" %s MHz", fs);
      
 	clk = get_mem_clk();
#if defined(LOONGSON_3BSINGLE) || defined (LOONGSON_3BSERVER)
	 /*for 3c/3b1500 ddr control*/ 
        if(clk != 0xf) {
#ifndef LOONGSON_3B1500
		clk20 = clk & 0x07;
		clk34 = (clk >> 3) & 0x03;
		mem_vco = 100 * (20 + clk20 * 2 + (clk20 == 7) * 2) / 3;
		memfreq = mem_vco / (1 << (clk34 + 1));
#else
		if ((clk & 0x8) == 0x0) { /* set ddr frequency by software */
			div_refc = ((*(volatile unsigned int *)(0xbfe001c0)) >> 8) & 0x3f; 
			div_loopc = ((*(volatile unsigned int *)(0xbfe001c0)) >> 14 ) & 0x3ff; 
			div_out = ((*(volatile unsigned int *)(0xbfe001c0)) >> 24) & 0x3f; 
			memfreq = ((100 / div_refc) * div_loopc) / div_out / 3;
		} else { /* set ddr frequency by hareware */
			clk20 = clk & 0x07;
			clk34 = (clk >> 3) & 0x03;
			mem_vco = 100 * (20 + clk20 * 2 + (clk20 == 7) * 2) / 3;
			memfreq = mem_vco / (1 << (clk34 + 1));
		}
		printf("/ Bus @ %d MHz\n",memfreq);
#endif
	}
        else
		printf("/ Bus @ 33 MHz\n");
#elif defined(LOONGSON_2G1A) || defined(LOONGSON_2F1A)
	/* to calculate memory frequency.
	 * we can find this function in loongson 2G manual,
	 * memclk * (clksel[7:5] * 2 + 20 + clksel[7:5] == 7 ? 2 : 0)/((clksel[9:8] + 1) * 2)
	 */
        if(clk != 0x1f)
	{
		clk567 = clk & 0x07;
		clk89 = (clk >> 3) & 0x3;
		memfreq = MEM_BASE_CLK_2G *(clk567 * 2 + 20 + (clk567 == 7 ? 2 : 0))/(2 * (clk89 + 1));

		printf("/ Bus @ %d MHz\n", memfreq);
	}
        else
		printf("/ Bus @ %d MHz\n", MEM_BASE_CLK_2G);

#else  /*for 3a/3b ddr controller */ 
	if(clk != 0x1f) {
		if ((*(volatile unsigned char*)0xbfe00187) == 0x37) {
			if ((clk & 0xf) == 0xf) { /* set ddr frequency by software */
				div_loopc = ((*(volatile unsigned int *)(0xbfe001c0)) >> 14 ) & 0x3ff;
				div_out = ((*(volatile unsigned int *)(0xbfe001c0)) >> 24) & 0x3f;
				memfreq = (100 * div_loopc) / div_out / 3;
			} else { /* set ddr frequency by hareware */
				clk30 = clk & 0x0f;
				clk4 = (clk >> 4) & 0x01;
				/* to calculate memory frequency.
				 * we can find this function in loongson 3A manual,
				 * memclk * (clksel[8:5] + 30)/(clksel[9] + 3)
				 */
				memfreq = 100*(clk30 + 30)/(clk4 + 3)/3;
			}
			}else{
				clk30 = clk & 0x0f;
				clk4 = (clk >> 4) & 0x01;
				/* to calculate memory frequency.
				 * we can find this function in loongson 3A manual,
				 * memclk * (clksel[8:5] + 30)/(clksel[9] + 3)
				 */
				memfreq = 100*(clk30 + 30)/(clk4 + 3)/3;
			}

				printf("/ Bus @ %d MHz\n",memfreq);
			}else
				printf("/ Bus @ 33 MHz\n");
#endif

		}


static int meminfo(){
	printf("mem info:\n");
	printf ("Memory size %lld MB .\n",memorysize_total);
	return 0;
}

static int uartinfo()
{
	printf("uart info:\n");
	printf("serial max baud 115200\n");
//	__msgbox(MSG_Y,MSG_X,MSG_H,MSG_W,menu[item].arg);
	return 0;
}

static int (*oldwrite)(int fd,char *buf,int len)=0;
static char *buffer;
static int total;
void *restdout(int  (*newwrite) (int fd, const void *buf, size_t n));
static int newwrite(int fd,char *buf,int len)
{
memcpy(buffer+total,buf,len);
total+=len;
return len;
}


static int netinfo()
{
char cmd[100];
#if 0
oldwrite=restdout(newwrite);
total=0;
buffer=heaptop+0x100000;
#endif
printf("net info:\n");

#if defined(LOONGSON_3ASINGLE) || defined( LOONGSON_3BSINGLE)

printf("RTL8111 rte0 info:\n");
strcpy(cmd,"ifconfig rte0;ifconfig rte0 status;");
do_cmd(cmd);

#else
#if defined LOONGSON_3ASERVER
printf("82546 em0 info:\n");
strcpy(cmd,"ifconfig em0;ifconfig em0 status;");
do_cmd(cmd);
printf("82546 em1 info:\n");
strcpy(cmd,"ifconfig em1;ifconfig em1 status;");
do_cmd(cmd);
#else
#if (defined LOONGSON_3A2H) || (defined LOONGSON_3C2H)
printf("PHY syn0 info:\n");
strcpy(cmd,"ifconfig syn0;ifconfig syn0 status");
do_cmd(cmd);
printf("PHY syn1 info:\n");
strcpy(cmd,"ifconfig syn1;ifconfig syn1 status");
do_cmd(cmd);
//printf("82559 fxp0 info:\n");
//strcpy(cmd,"ifconfig fxp0;ifconfig fxp0 status");
//do_cmd(cmd);
#endif
#endif
#endif

printf("link speed up to 100 Mbps\n");
#if 0
restdout(oldwrite);
buffer[total]='\n';
buffer[total+1]=0;
__msgbox(0,0,24,80,buffer);
#endif
//	__msgbox(MSG_Y,MSG_X,MSG_H,MSG_W,menu[item].arg);
}

static int cmd_test(int ac,char **av)
{
__console_alloc();
if(ac==1){
cpuinfo();
meminfo();
uartinfo();
netinfo();
}
else if(!strcmp(av[1],"cpu")) cpuinfo();
else if(!strcmp(av[1],"mem")) meminfo();
else if(!strcmp(av[1],"uart")) uartinfo();
else if(!strcmp(av[1],"net")) netinfo();
			lpause();
}

static struct setupMenu testmenu={
0,POP_W,POP_H,
(struct setupMenuitem[])
{
{POP_Y,POP_X,1,1,TYPE_NONE,"    board test"},
{POP_Y+1,POP_X,2,2,TYPE_CMD,"(1)cpu info=sysinfo cpu"},
{POP_Y+2,POP_X,3,3,TYPE_CMD,"(2)memory info=sysinfo mem"},
{POP_Y+3,POP_X,4,4,TYPE_CMD,"(3)uart info=sysinfo uart"},
{POP_Y+4,POP_X,5,5,TYPE_CMD,"(4)net info=sysinfo net"},
{POP_Y+5,POP_X,1,1,TYPE_CMD,"(5)quit=| _quit"},
{}
}
};

static int cmd_setup(int ac,char **av)
{
__console_alloc();
do_menu(&testmenu);
__console_free();
return 0;
}


static const Cmd Cmds[] =
{
	{"MyCmds"},
	{"sysinfo","[cpu|mem|uart|net]",0,"hardware test",cmd_test,0,99,CMD_REPEAT},
	{"info","",0,"hardware test",cmd_setup,0,99,CMD_REPEAT},
	{0,0}
};

static void init_cmd __P((void)) __attribute__ ((constructor));
static void init_cmd()
{
	cmdlist_expand(Cmds, 1);
}
