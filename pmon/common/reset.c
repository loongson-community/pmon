/*
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of  the GNU General  Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.
 *
 * Copyright (C) 2007 Lemote, Inc. & Institute of Computing Technology
 * Author: Fuxin Zhang, zhangfx@lemote.com
 */
#include <stdio.h>

void poweroff_kernel()
{
	unsigned char *watch_dog_base = 0xb8000cd6;
	unsigned char *watch_dog_config = 0xba00a041;
	unsigned char *watch_dog_mem = 0xbe010000;
	unsigned char *reg_cf9 = (unsigned char *)0xb8000cf9;
	int i;

#if (defined LOONGSON_3A2H) || (defined LOONGSON_3C2H)
	//*(volatile unsigned int *)0xffffffffbbef0014 &= (0x1 << 10) | (0x1 << 11) | (0x1 << 12) | (0x1 << 13);
	volatile unsigned int * pm_ctrl_reg = 0xffffffffbbef0014;
	volatile unsigned int * pm_statu_reg = 0xffffffffbbef000c;

	* pm_statu_reg = 0x100;	 // clear bit8: PWRBTN_STS
	for ( i = 0x8000000; i > 0; i--){}
	* pm_ctrl_reg = 0x3c00;  // sleep enable, and enter S5 state 
#else
	for (i=0; i<0x10000; i++);
	*reg_cf9 = 4;


	/* Enable WatchDogTimer */
	for (i=0; i<0x10000; i++);
	*watch_dog_base  = 0x69;
	*(watch_dog_base + 1) = 0x0;

	/* Set WatchDogTimer base address is 0x10000 */
	for (i=0; i<0x10000; i++);
	*watch_dog_base = 0x6c;
	*(watch_dog_base + 1) = 0x0;

	for (i=0; i<0x10000; i++);
	*watch_dog_base = 0x6d;
	*(watch_dog_base + 1) = 0x0;

	for (i=0; i<0x10000; i++);
	*watch_dog_base = 0x6e;
	*(watch_dog_base + 1) = 0x1;

	for (i=0; i<0x10000; i++);
	*watch_dog_base = 0x6f;
	*(watch_dog_base + 1) = 0x0;

	for (i=0; i<0x10000; i++);
	*watch_dog_config = 0xff;

	/* Set WatchDogTimer to starting */
	for (i=0; i<0x10000; i++);
		*watch_dog_mem = 0x05;
	for (i=0; i<0x10000; i++);
	*(watch_dog_mem + 1) = 0x500;
	for (i=0; i<0x10000; i++);
	//*watch_dog_mem = 0x85;
	*watch_dog_mem |= 0x80;

#endif
}

void reboot_kernel()
{
	volatile   unsigned int *p = 0xbfe0011c;
	int i;

#ifdef FULL_RST
	unsigned char * watch_dog_base = (unsigned char *)0xb8000cd6;

	/* use the FullRst to realize reboot */
	*watch_dog_base  = 0x85;
	*(watch_dog_base + 1) = 0x0e;
#else

#if (defined LOONGSON_3A2H) || (defined LOONGSON_3C2H)
	volatile char * hard_reset_reg = 0xffffffffbbef0030;
	* hard_reset_reg = ( * hard_reset_reg) | 0x01; // watch dog hardreset
#endif
#ifdef LOONGSON_3ASERVER
#ifdef DDR3_DIMM
        p[0] |= ((0x1 << 4) | (0x1 << 5) | (0x1 << 6) | (0x1 << 13));
        p[1] &= ~((0x1 << 4) | (0x1 << 5) | (0x1 << 6) | (0x1 << 13));
 
        p[0] &= ~(0x1 << 13);
        p[1] &= ~(0x1 << 13);

	for (i=0; i<0x100; i++);

        p[0] |= (0x1 << 14);
        p[1] &= ~(0x1 << 14);

	for (i=0; i<0x1000; i++);

        p[0] &= ~(0x1 << 14);
        p[1] &= ~(0x1 << 14);
#endif
#elif  LOONGSON_3B1500
		 /*3b1500(aka 3c780e) board does not use watch dog for reset, it 
		  * just use reset chip connected to gpio10*/ 
	
        p[0] &= ~(0x1 << 10);
        p[1] &= ~(0x1 << 10);

	for (i=0; i<0x10000; i++);

        p[0] |= (0x1 << 10);
        p[1] &= ~(0x1 << 10);
#else 
        p[0] &= ~(0x1 << 13);
        p[1] &= ~(0x1 << 13);

        p[0] |= (0x1 << 14);
        p[1] &= ~(0x1 << 14);

        p[0] |= (0x1 << 5);
        p[1] &= ~(0x1 << 5);

        p[0] &= ~(0x1 << 4);
        p[1] &= ~(0x1 << 4);

        p[0] |= (0x1 << 3);
        p[1] &= ~(0x1 << 3);

        p[0] &= ~(0x1 << 14);
        p[1] &= ~(0x1 << 14);

	for (i=0; i<0x10000; i++);

        p[0] |= (0x1 << 13);
        p[1] &= ~(0x1 << 13);
#endif
#endif

}

void (*poweroff_pt)(void) = poweroff_kernel;
void (*reboot_pt)(void) = reboot_kernel;
