/*
 * Copyright (C) 2009 Lemote Inc.
 * Author: Xiang Yu, xiangy@lemote.com
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of  the GNU General  Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.
 */


#include <pmon.h>
#include <stdio.h>
#include <target/cs5536.h>
#include <sys/linux/types.h>


int gpio_base_inited = 0;
unsigned int GPIO_BASE_ADDR;

extern void _rdmsr(u32 , u32 *, u32 *);

static void get_gpio_base(void) 
{
	unsigned int hi, lo;

	_rdmsr(DIVIL_MSR_REG(DIVIL_LBAR_GPIO), &hi, &lo);
	GPIO_BASE_ADDR = lo;
	printf("GPIO_BASE_ADDR is %x\n", GPIO_BASE_ADDR);
	gpio_base_inited = 1;
}

static void GPIO_HI_BIT(int bit, int reg)
{
	int orig;
	
	orig = *(unsigned int *)(0xbfd00000 + reg);
	orig = orig | (1 << bit);
	orig = orig &( ~(1 << (16 + bit)));		
	*(unsigned int *)(0xbfd00000 + reg) = orig;
}

static void GPIO_LO_BIT(int bit, int reg)
{
	int orig;
	
	orig = *(unsigned int *)(0xbfd00000 + reg);
	orig = orig & (~(1 << bit));
	orig = orig | (1 << (16 + bit));		
	*(unsigned int *)(0xbfd00000 + reg) = orig;
}

extern int get_rsa(unsigned long*, char *);

int  cmd_gpio_low(int argc, char *argv[])
{
	unsigned long gpio;
	
	if(argc > 2)
		printf("sorry, too many parameters!");

	if(!gpio_base_inited)	
		get_gpio_base();

	get_rsa(&gpio, argv[1]);

	printf("gpio is %d\n", gpio);
	if(gpio >= 16) {	
		gpio = gpio - 16;

		GPIO_LO_BIT(gpio, GPIO_BASE_ADDR | GPIOH_OUT_VAL);
		GPIO_HI_BIT(gpio, GPIO_BASE_ADDR | GPIOH_OUT_EN);
		GPIO_LO_BIT(gpio, GPIO_BASE_ADDR | GPIOH_IN_EN);
		GPIO_LO_BIT(gpio, GPIO_BASE_ADDR | GPIOH_OUT_AUX1_SEL);
		GPIO_LO_BIT(gpio, GPIO_BASE_ADDR | GPIOH_OUT_AUX2_SEL);
		GPIO_LO_BIT(gpio, GPIO_BASE_ADDR | GPIOH_IN_AUX1_SEL);
		GPIO_HI_BIT(gpio, GPIO_BASE_ADDR | GPIOH_PU_EN);
	} else {
		GPIO_LO_BIT(gpio, GPIO_BASE_ADDR | GPIOL_OUT_VAL);
		GPIO_HI_BIT(gpio, GPIO_BASE_ADDR | GPIOL_OUT_EN);
		GPIO_LO_BIT(gpio, GPIO_BASE_ADDR | GPIOL_IN_EN);
		GPIO_LO_BIT(gpio, GPIO_BASE_ADDR | GPIOL_OUT_AUX1_SEL);
		GPIO_LO_BIT(gpio, GPIO_BASE_ADDR | GPIOL_OUT_AUX2_SEL);
		GPIO_LO_BIT(gpio, GPIO_BASE_ADDR | GPIOL_IN_AUX1_SEL);
		GPIO_HI_BIT(gpio, GPIO_BASE_ADDR | GPIOL_PU_EN);
	}
	
	return 0;
}

int cmd_gpio_high(int argc, char *argv[])
{
	unsigned long gpio;
	
	if(argc > 2)
		printf("sorry, too many parameters!");
	
	if(!gpio_base_inited)	
		get_gpio_base();

	get_rsa(&gpio, argv[1]);

	printf("gpio is %d\n", gpio);
	if(gpio >= 16) {	
		gpio = gpio - 16;

		GPIO_HI_BIT(gpio, GPIO_BASE_ADDR | GPIOH_OUT_VAL);
		GPIO_HI_BIT(gpio, GPIO_BASE_ADDR | GPIOH_OUT_EN);
		GPIO_LO_BIT(gpio, GPIO_BASE_ADDR | GPIOH_IN_EN);
		GPIO_LO_BIT(gpio, GPIO_BASE_ADDR | GPIOH_OUT_AUX1_SEL);
		GPIO_LO_BIT(gpio, GPIO_BASE_ADDR | GPIOH_OUT_AUX2_SEL);
		GPIO_LO_BIT(gpio, GPIO_BASE_ADDR | GPIOH_IN_AUX1_SEL);
		GPIO_HI_BIT(gpio, GPIO_BASE_ADDR | GPIOH_PU_EN);
	} else {
		GPIO_HI_BIT(gpio, GPIO_BASE_ADDR | GPIOL_OUT_VAL);
		GPIO_HI_BIT(gpio, GPIO_BASE_ADDR | GPIOL_OUT_EN);
		GPIO_LO_BIT(gpio, GPIO_BASE_ADDR | GPIOL_IN_EN);
		GPIO_LO_BIT(gpio, GPIO_BASE_ADDR | GPIOL_OUT_AUX1_SEL);
		GPIO_LO_BIT(gpio, GPIO_BASE_ADDR | GPIOL_OUT_AUX2_SEL);
		GPIO_LO_BIT(gpio, GPIO_BASE_ADDR | GPIOL_IN_AUX1_SEL);
		GPIO_HI_BIT(gpio, GPIO_BASE_ADDR | GPIOL_PU_EN);
	}

	return 0;
}

static const Optdesc cmd_gpio_opts[] = {
	{"<index>", "GPIO index (from 0-31)"},
	{0},
};

static const Cmd Cmds[] = {
	{"gpio func"},
	{"gpio_low", "<index>", 
			cmd_gpio_opts,
			"pull the index gpio low",
			cmd_gpio_low, 2, 2, 0
	},
	{"gpio_high", "<index>",
			cmd_gpio_opts,
			"pull the index gpio high",
			cmd_gpio_high, 2, 2, 0
	},
	{0, 0}
};

static void init_cmd __P((void)) __attribute__ ((constructor));

void init_cmd()
{
	cmdlist_expand(Cmds, 1);
}
