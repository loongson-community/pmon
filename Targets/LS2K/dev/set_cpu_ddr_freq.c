#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pmon.h>
#include<machine/types.h>
#include<sys/malloc.h>
#include <asm.h>

#define CLOCK_CTRL0   ((volatile unsigned int *)0xbfd00220)
#define CLOCK_CTRL1   ((volatile unsigned int *)0xbfd00224)
#define CHIP_SAMPLE0  ((volatile unsigned int *)0xbfd00210)
#define NAND_CMD_REG  ((volatile unsigned int *)0xbfee0000)
#define DDR_CONF_CTL_00  ((volatile unsigned int *)0xaff00000)
#define DDR_CONF_CTL_03  ((volatile unsigned int *)0xaff00034)
#define DDR_CONF_CTL_20  ((volatile unsigned int *)0xaff00140)

#define FRAM_BUFF_CONFIG_0  ((volatile unsigned int *)0xbfe51240)
#define FRAM_BUFF_CONFIG_1  ((volatile unsigned int *)0xbfe51250)
#define CHIP_CONFIG0        ((volatile unsigned int *)0xbfd00200)

static int delay(int value)
{
	int i, j;
	for (i = 0; i < value; i++) {
		for (j = 0; j < 1000; j++) {
			;
		}
	}

	return 0;
}

static int calculate_cpu_freq(unsigned int freq, unsigned int *state)
{
	unsigned int i, j, tmp_i, tmp_j;
	unsigned int cal_freq, min_freq, tmp_min_freq;

	for (i = 10; i <= 30; i++) {
		for (j = 0; j <= 7; j++) {
			cal_freq = (i * 100) / (0x1 << j);
			if (freq >= cal_freq) {
				min_freq = freq - cal_freq;
			} else {
				min_freq = cal_freq - freq;
			}
			if (((i == 7) && (j == 0)) || (min_freq < tmp_min_freq)) {
				tmp_min_freq = min_freq;
				tmp_i = i;
				tmp_j = j;
			}

		}

	}
	*state |= (tmp_i << 1);	//ldf 
	*state |= (tmp_j << 8);	//odf
	printf("calculate freq ldf = %d,odf = %d \n", tmp_i, tmp_j);
	return 0;
}

static int set_cpu_clk(int set_freq)
{
	unsigned int ctrl0_state, state_pt;

	(*CLOCK_CTRL0) |= 0x1;	//set _set

	(*CLOCK_CTRL0) &= ~(0x1 << 12);	//set _sel 
	delay(1);

	(*CLOCK_CTRL0) |= (0x1 << 11);	//set _pd 

	ctrl0_state = *CLOCK_CTRL0;
	ctrl0_state &= (~(0x3ff << 1));	//clear ldf and odf

	calculate_cpu_freq(set_freq, &ctrl0_state);
	printf("cpu clock : %d MHz\n",
	       ((ctrl0_state >> 1) & 0x7f) * 100 /
	       (0x1 << ((ctrl0_state >> 8) & 0x07)));

	*CLOCK_CTRL0 = ctrl0_state;	//set  ldf /odf

	delay(100);

	(*CLOCK_CTRL0) &= (~(0x1 << 11));	//clear _pd 

	while (((*CHIP_SAMPLE0) & (0x1 << 8)) == 0) {
		delay(1);
	}

	(*CLOCK_CTRL0) |= 0x1 << 12;	//clear _sel 

	(*CLOCK_CTRL0) &= ~0x1;	//clear _set 

	return 0;
}

static unsigned int get_cpu_freq(void)
{

	int i, timeout, cur, sec, cnt, clk_invalid;
	unsigned int md_pipefreq, md_cpufreq;

	md_pipefreq = 660000000;	/* NB FPGA */
	md_cpufreq = 60000000;

	(*(volatile unsigned int *)(0xbfef8040)) =
	    (1 << 13) | (1 << 8) | (1 << 11);
	(*(volatile unsigned int *)(0xbfef8020)) = 0;
	(*(volatile unsigned int *)(0xbfef8060)) = 0;

	for (i = 2; i != 0; i--) {
		timeout = 10000000;
		sec = (*(volatile unsigned int *)(0xbfef802c) & 0x3f0) >> 0x4;
		while ((cur =
			(*(volatile unsigned int *)(0xbfef802c) & 0x3f0) >> 0x4)
		       == sec) {
		}
		cnt = CPU_GetCOUNT();
		do {
			sec =
			    (*(volatile unsigned int *)(0xbfef802c) & 0x3f0) >>
			    0x4;
			timeout--;
		} while (timeout != 0 && (cur == sec));
		cnt = CPU_GetCOUNT() - cnt;
		if (timeout == 0) {
			tgt_printf("time out!\n");
			break;	/* Get out if clock is not running */
		}
	}

	if (timeout != 0) {
		clk_invalid = 0;
		md_pipefreq = cnt / 10000;
		md_pipefreq *= 20000;
		/* we have no simple way to read multiplier value
		 */
		md_cpufreq = 66000000;
	}
	printf("cpu freq %u\n", md_pipefreq);
	return md_pipefreq;
}

static int calculate_ddr_freq(unsigned int freq, unsigned int *state)
{
	unsigned int i, j, k, tmp_i, tmp_j, tmp_k, count;
	unsigned int cal_freq, min_freq, tmp_min_freq;

	count = 0;

	for (k = 1; k <= 7; k++)	//idf
	{
		if ((100 / k < 4) || (100 / k > 75)) {
			continue;
		}

		for (i = 8; i <= 225; i++)	//ldf
		{

			if (((100 / k) * 2 * i < 600)
			    || ((100 / k) * 2 * i > 1800)) {
				continue;
			}

			for (j = 0; j <= 3; j++)	//odf
			{
				if ((((100 / k) * 2 * i) / (0x1 << j) < 75)
				    || (((100 / k) * 2 * i) / (0x1 << j) >
					1800)) {
					continue;
				}
				cal_freq = ((100 / k) * 2 * i) / (0x1 << j);
				if (freq >= cal_freq) {
					min_freq = freq - cal_freq;
				} else {
					min_freq = cal_freq - freq;
				}
				if ((count == 0) || (min_freq < tmp_min_freq)) {
					tmp_min_freq = min_freq;
					tmp_i = i;
					tmp_j = j;
					tmp_k = k;
					count = 1;
				}

			}

		}
	}
	*state = 0;
	*state |= (tmp_i << 24);	//ldf 
	*state |= (tmp_j << 22);	//odf
	*state |= (tmp_k << 19);	//idf
	printf("calculate freq : %d Mhz  idf = %d, ldf = %d,odf = %d \n", freq,
	       tmp_k, tmp_i, tmp_j);
	return 0;
}

static int set_ddr_config_reg(unsigned int state)
{

	unsigned int framebuffer_reg_0, framebuffer_reg_1;
	unsigned int ctrl0_state;
	int i;

	framebuffer_reg_0 = *FRAM_BUFF_CONFIG_0;
	framebuffer_reg_1 = *FRAM_BUFF_CONFIG_1;	//save frame config reg

	*FRAM_BUFF_CONFIG_0 = 0;
	*FRAM_BUFF_CONFIG_1 = 0;	//stop dc output

	for (i = 0; i < 1000; i++) {
		;
	}

	*DDR_CONF_CTL_03 |= 0x00000001;	//set auto-refresh 

	for (i = 0; i < 1000; i++) {
		;
	}

	*DDR_CONF_CTL_03 &= 0xfffffeff;	//clear start

	for (i = 0; i < 1000; i++) {
		;
	}

	(*CLOCK_CTRL0) |= 0x1 << 16;	//set _set
	(*CLOCK_CTRL0) &= ~(0x1 << 17);	//set _sel 
	for (i = 0; i < 1000; i++) {
		;
	}

	*CLOCK_CTRL1 &= ~(0x1 << 29);	//set normal mode 
	(*CLOCK_CTRL0) |= (0x1 << 18);	//set _pd 
	(*CLOCK_CTRL0) |= (0x1 << 18);	//set _pd 
	ctrl0_state = *CLOCK_CTRL0;
	ctrl0_state &= (~(0x1fff << 19));	//clear ldf idf and odf
	state &= (0x1fff << 19);

	*CLOCK_CTRL0 = (ctrl0_state | state);	//set  idf ldf /odf

	for (i = 0; i < 5000; i++) {
		;
	}
	(*CLOCK_CTRL0) &= (~(0x1 << 18));	//clear _pd 

	while (((*CHIP_SAMPLE0) & (0x1 << 9)) == 0) {
		for (i = 0; i < 500; i++) {
			;
		}
	}

	(*CLOCK_CTRL0) |= 0x1 << 17;	//clear _sel 
	(*CLOCK_CTRL0) &= ~(0x1 << 16);	//clear _set 
	*DDR_CONF_CTL_03 |= 0x1 << 8;	// set start 
	for (i = 0; i < 0x1000; i++) {
		;
	}
	*DDR_CONF_CTL_03 &= ~(0x1 << 0);	//clear auto-refresh 
	for (i = 0; i < 0x3000; i++) {
		;
	}

	*FRAM_BUFF_CONFIG_0 = framebuffer_reg_0;	//restore framebuffer config 
	*FRAM_BUFF_CONFIG_1 = framebuffer_reg_1;

	ctrl0_state = *CLOCK_CTRL0;
	return ctrl0_state;
}

static int set_ddr_clk(int set_freq)
{
	unsigned int ctrl0_state, state_pt;
	int i;

	ctrl0_state = 0;
	calculate_ddr_freq(set_freq, &ctrl0_state);
	printf("ddr frequency  : %d MHz\n",
	       ((100 / ((ctrl0_state >> 19) & 0x07)) * 2 *
		((ctrl0_state >> 24) & 0xff)) /
	       (0x1 << ((ctrl0_state >> 22) & 0x3)));

	*NAND_CMD_REG |= (0x1 << 15);	//enable 1k ram of nand controller
	*CHIP_CONFIG0 &= ~(0x1 << 13);	//enable ddr config register space
	*((unsigned int *)0xbfd800a0) |= 0xf << 4;

	for (i = 0; i < 250; i++) {

		*((unsigned int *)(0xbfee0400 + i * 4)) =
		    *((unsigned int *)(set_ddr_config_reg + i * 4));

	}

	state_pt = ((int (*)(unsigned int))0xbfee0400) (ctrl0_state);

	*((unsigned int *)0xbfd800a0) &= ~(0xf << 4);
	*((unsigned int *)0xbfd800a0) |= 0x8 << 4;

	*CHIP_CONFIG0 |= 0x1 << 13;	//disable ddr config register space

	return 0;
}

static int cmd_set_cup_freq __P((int, char *[]));
static int cmd_set_ddr_freq __P((int, char *[]));
static int cmd_display_cpu_freq __P((int, char *[]));

static int cmd_set_cpu_freq(ac, av)
int ac;
char *av[];
{
	unsigned int set_freq;

	if (ac != 2) {
		printf("input error ! must be : set_cup_freq   xxx\n");
		return -1;
	}

	set_freq = atoi(av[1]);

	set_cpu_clk(set_freq);
	get_cpu_freq();

	return (1);
}

static int cmd_display_cpu_freq(ac, av)
int ac;
char *av[];
{
	unsigned int get_freq;

	if (ac != 1) {
		printf("input error ! must be : get_cpu_freq\n");
		return -1;
	}

	get_freq = get_cpu_freq();
	get_freq /= 1000 * 1000;

	printf("current cup frequency : %d MHz\n", get_freq);

	return (1);
}

static int cmd_set_ddr_freq(ac, av)
int ac;
char *av[];
{
	unsigned int set_freq;

	if (ac != 2) {
		printf("input error ! must be:set_ddr_freq   xxx\n");
		return -1;
	}

	set_freq = atoi(av[1]);

	set_ddr_clk(set_freq);
	return (1);
}

static const Optdesc cmd_myshow_opts[] = {
	{0},
	{0},
};

static const Cmd Cmds[] = { {"Set CPU and DDR Frequency Command"},
{"set_cpu_freq", 0, cmd_myshow_opts, "set cpu freq cmd: cmd freq",
 cmd_set_cpu_freq, 1, 99, 0},
{"set_ddr_freq", 0, cmd_myshow_opts, "set ddr freq cmd: cmd freq",
 cmd_set_ddr_freq, 1, 99, 0},
{"get_cpu_freq", 0, cmd_myshow_opts, "get cpu freq cmd: cmd ",
 cmd_display_cpu_freq, 1, 99, 0},
{0, 0},

};

static void init_cmd __P((void)) __attribute__ ((constructor));

static void init_cmd()
{
	cmdlist_expand(Cmds, 1);
}
