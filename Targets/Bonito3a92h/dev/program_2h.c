#include <stdio.h>
#include <pmon.h>
#include <stdarg.h>
#include <termio.h>
#include <string.h>
#include <signal.h>
#include <setjmp.h>
#include <ctype.h>
#include <unistd.h>
#include <stdlib.h>
#include "cmd_hist.h"
#include <machine/pio.h>
#include <endian.h>
#include <string.h>
#include <exec.h>
#include <file.h>
#include <sys/device.h>
#include <pflash.h>
#include <flash.h>
#include <dev/pflash_tgt.h>
#ifdef _KERNEL
#undef _KERNEL
#include <sys/ioctl.h>
#define _KERNEL
#else
#include <sys/ioctl.h>
#endif

#define ADDR_3A 0x9800000003000000
#define ADDR_2H 0x90000e0003000000

extern long dl_minaddr;
extern long dl_maxaddr;

#define u64 unsigned long long 
extern u64 __raw__writeq(u64 addr, u64 val);
extern u64 __raw__readq(u64 q);

int
cmd_load_2h(int ac,char *av[])
{
	u64 val, val_3a, val_2h;
	int j, i, count, rede, retid, error = 0, dly = 10;
	char load[256], boottype[256];
	volatile unsigned int *mailbox;
	long size;
	mailbox = (volatile unsigned int *)0xbbd80130;	
	memset(load,0,256);
	memset(boottype,0,256);
	strcpy(boottype,av[1]);	
	sprintf(load, "load -r -o 0x83000000 %s", boottype);
	retid = do_cmd(load);	
	if (retid != 0)
	{
		printf("load error..\n");
		return -1;
	}	
	size = dl_maxaddr - dl_minaddr;		
	rede = size%8;	
	if(rede == 0)
		count = (int)(size/8);
	else
		count = (int)(size/8) + 1;
		
	for(j=0;count>0;count--,j+=8)
	{
		val = __raw__readq(ADDR_3A+j);
		__raw__writeq(ADDR_2H+j,val);	
	}
	for(j=0;count>0;count--,j+=8)
	{
		val_3a = __raw__readq(ADDR_3A+j);
		val_2h = __raw__readq(ADDR_2H+j);
		if(val_3a == val_2h){
			continue;
		}
		else{
			error = 1;
			break;
		}
	}	
	if(error == 1)
		printf("Verifying Failure\n");
	else
		printf("Verifying No Errors Found\n");

	mailbox[0] = 0x90000;
	mailbox[3] = 0x1;	
	printf("Programming to 2H Please Waite..\n");
	do {
		printf ("\b\b%02d", --dly);
		for(i = 0; i < 9000; i++){
			delay(100);
		}
	} while (dly != 0);
	putchar ('\n');
	return 0;
}

static const Cmd Cmds[] = {
        {"Programming 2H BIOS"},
        {"load_2h", "NULL", 0, "Programming 2H BIOS", cmd_load_2h, 0, 99,
         0},
        {0, 0}
};

static void init_cmd __P((void)) __attribute__ ((constructor));

static void init_cmd()
{
        cmdlist_expand(Cmds, 1);
}
