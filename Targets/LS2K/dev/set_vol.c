#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pmon.h>
#include <ctype.h>
//#include <include/types.h>
#include <target/ls2k.h>
#include <machine/pio.h>

void set_cpu_vol(int argc, unsigned char**arg)
{
	int i, sum = 0;
	unsigned int val, vid;
	unsigned char *buf; 
	if (argc != 2)
		goto usage;

	if (strlen(arg[1]) != 6)
		goto usage;

	buf = arg[1];
	for (i = 0; i < 6; i++)
		sum += (buf[i] - '0') * (1 << i); 

	val = 0x10000003 | (sum << 3); 
	outl(LS2H_ACPI_REG_BASE, 0x33cc800);
	outl(LS2H_DVFS_CNT_REG, val);
	goto cur;

usage:
	printf("Set CPU core's voltage.Be carefull!!!\n");
	printf("The VID must be 6 bits binary.\nFor example:\n");
	printf("\tset_cpu_val 001110\n");
cur:
	vid = (inl(LS2H_DVFS_STS_REG) >> 4) & 0x3f;
	printf("Current VID: ");
	for (i = 0; i < 6; i++) {
		printf("%d", (vid >> i) & 1); 
	}
	printf("\n");
	return;
}

static const Cmd cmds[] =
{
	{"ls2h commands"},
	{"set_cpu_vol","",0,"Dynamic set cpu voltage",set_cpu_vol,0,99,CMD_REPEAT},
	{0,0}

};

static void init_cmd __P((void)) __attribute__ ((constructor));

static void init_cmd()
{
	cmdlist_expand(cmds,1);
}

