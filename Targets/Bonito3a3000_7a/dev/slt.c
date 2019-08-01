 /*
  * This file is for SLT for 3A7A
  *
  * Note: if do SLT test,you must define LS7A_PCIE_NO_POWERDOWN
  */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/device.h>
#include <sys/queue.h>

#include <pmon.h>

#define u64 unsigned long long
#define u32 unsigned int

extern u32 __raw__writew(u64 addr, u32 val);
extern u32 __raw__readw(u64 q);

int slt(unsigned int port_num, unsigned int test_mode)
{
	unsigned int dev_num;
	unsigned long long header;
	unsigned long long bar0;
	unsigned int bar0_low;
	unsigned int bar0_high;
	unsigned int status;

	printf("pcie port = %d\n",port_num);
	if(port_num > 11) {
		printf("pcie port num error\n");
		return 0;
	}

	switch (port_num) {
		case 0:
			dev_num = 9;
			break;
		case 1:
			dev_num = 10;
			break;
		case 2:
			dev_num = 11;
			break;
		case 3:
			dev_num = 12;
			break;
		case 4:
			dev_num = 13;
			break;
		case 5:
			dev_num = 14;
			break;
		case 6:
			dev_num = 15;
			break;
		case 7:
			dev_num = 16;
			break;
		case 8:
			dev_num = 17;
			break;
		case 9:
			dev_num = 18;
			break;
		case 10:
			dev_num = 19;
			break;
		case 11:
			dev_num = 20;
			break;
		default:
			dev_num = 9;
			break;
	}

	header = 0x90000efe00000000ULL | (dev_num << 11);

	bar0_low  = (__raw__readw(header + 0x10) & 0xffffffff0);
	bar0_high = (__raw__readw(header + 0x14));
	bar0 = (bar0_high << 32 | bar0_low) + 0x9000000000000000ULL;
	//printf("pcie header = 0x%llx\n",header);
	//printf("pcie bar0_low = 0x%x\n",bar0_low);
	//printf("pcie bar0_high = 0x%x\n",bar0_high);
	//printf("pcie bar0 = 0x%llx\n",bar0);


	status = __raw__readw(bar0 + 0xc);
	if((status & 0x7f) != 0x51) {
		printf("!error: status %x ,dev 0x%x is not link up\n", status, dev_num);
		return -1;
	}

	printf("dev 0x%x link up\n", dev_num);

	status = __raw__readw(header + 0x80);
	if(((status >> 16) & 0xf) == 0x2) {
		printf("dev 0x%x speed is 5GT\n", dev_num);
	}else {
		printf("!error: status %x ,dev 0x%x speed is not 5GT\n", status, dev_num);
		return -1;
	}
	if(((status >> 20) & 0x3f) == test_mode) {
		printf("dev 0x%x width is X%d\n", dev_num, test_mode);
	}else {
		printf("!error: status %x ,dev 0x%x link width is not X%d\n", status, dev_num, test_mode);
		return -1;
	}
	return 0;
}
int slt_test(void)
{
	unsigned int test_mode;
	unsigned int port_num;
	int status;

	for(port_num = 4; port_num <= 10 ; port_num += 2)
	{
		if(port_num == 4) {
			test_mode = 4;
		}else {
			test_mode = 8;
		}

		status = slt(port_num, test_mode);
		if(status) {
			printf("error! :test failed \n");
			while(1);
		}else {
			printf("success! \n");
		}

	}
	return 0;
}
int
cmd_slttest(ac, av)
	int ac;
	char *av[];
{
	unsigned int test_mode;
	unsigned int port_num;
	int status;

	if (ac != 3) {
		printf("usage: pcietest <port num> [test mode for how many lines?]\n");
		printf("port num: 0 -> f0 port 0 x1/x4\n");
		printf("port num: 1 -> f0 port 1 x1\n");
		printf("port num: 2 -> f0 port 2 x1\n");
		printf("port num: 3 -> f0 port 3 x1\n");

		printf("port num: 4 -> f1 port 0 x1/x4\n");
		printf("port num: 5 -> f1 port 1 x1\n");

		printf("port num: 6 -> h port 0 x4/x8\n");
		printf("port num: 7 -> h port 0 x4\n");

		printf("port num: 8 -> g0 port 0 x4/x8\n");
		printf("port num: 9 -> g0 port 0 x4\n");

		printf("port num: 10 -> g1 port 0 x4/x8\n");
		printf("port num: 11 -> g1 port 1 x4\n");
		printf("test_mode: 1 ->0x1, 1 lines\n");
		printf("test_mode: 4 ->0x4, 4 lines\n");
		printf("test_mode: 8 ->0x8, 8 lines\n");
		printf("For example:slttest 0 1 \n");
		printf("For example:slttest 4 4 \n");
		return 0;
	}

	port_num = (unsigned int)atoi(av[1]);
	test_mode = (unsigned int)atoi(av[2]);

	status = slt(port_num, test_mode);
	if(status) {
		printf("error! :test failed \n");
	}else {
		printf("success! \n");
	}

	return 0;
}

/*
 *
 *  Command table registration
 *  ==========================
 */

static const Cmd Cmds[] =
{
	{"Misc"},
	{"slttest", "", 0, "LS7A pcie link status test: slttest ", cmd_slttest, 1, 99, 0},
	{0, 0}
};

static void init_cmd __P((void)) __attribute__ ((constructor));

void
init_cmd()
{
	cmdlist_expand(Cmds, 1);
}
