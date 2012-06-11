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

#ifdef LOONGSON_3A2H
	//*(volatile unsigned int *)0xffffffffbbef0014 &= (0x1 << 10) | (0x1 << 11) | (0x1 << 12) | (0x1 << 13);
	*(volatile unsigned int *)0xffffffffbbef0014 = 0x3c00;
#endif
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

}

void reboot_kernel()
{
        unsigned int *p = 0xbfe0011c;
	int i;

#ifdef LOONGSON_3A2H
	*(volatile unsigned int *)0xffffffffbbef0030 = 1;
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
}

void (*poweroff_pt)(void) = poweroff_kernel;
void (*reboot_pt)(void) = reboot_kernel;
