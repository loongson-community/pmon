/*
 * Copyright (C) 2009 Lemote, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 *
 * Author : liujl <liujl@lemote.com>
 * Date : 2009-08-18
 */

/*-----------------------------------------------------------------------*/

#include <stdio.h>
#include <stdarg.h>
#include <sys/linux/types.h>
#include <include/bonito.h>
#include <linux/io.h>
#include <machine/pio.h> 
#include <include/sb600.h>
#include <pci/sb600_cfg.h>
#include "sb600_config.h"

/*-----------------------------------------------------------------------*/
struct hwm_fan fan0 = {
	1,  /* 0:Disabled 1:En w/o Temp 2:En w/ Temp0 3:En w/ AMDSI*/
	0,  /* 0:Disabled 1:Enabled*/
	0,  /* 0:Active low   1:Active High*/
	0,
	0xe0,
	0xf0,
	10,
	0x20,
	0,
	0x40,
	0,
	0x60,
	0,
	5,
	8
};

struct hwm_fan fan1 = {
	1,  /* 0:Disabled 1:En w/o Temp 2:En w/ Temp0 3:En w/ AMDSI*/
	0,  /* 0:Disabled 1:Enabled*/
	0,  /* 0:Active low   1:Active High*/
	0,
	0xe0,
	0xf0,
	10,
	0x20,
	0,
	0x40,
	0,
	0x60,
	0,
	5,
	8
};


struct hwm_fan fan2 = {
	1,  /* 0:Disabled 1:En w/o Temp 2:En w/ Temp0 3:En w/ AMDSI*/
	0,  /* 0:Disabled 1:Enabled*/
	0,  /* 0:Active low   1:Active High*/
	0,
	0xe0,
	0xf0,
	10,
	0x20,
	0,
	0x40,
	0,
	0x60,
	0,
	5,
	8
};

struct ati_sb_cfg ati_sb_cfg = {
	0,			/* ati cfg asl flags : acaf */
	0,			/* pcie base */
	0,			/* hpet base addr */
	-1,			/* usb overcurrent map on port 7-0 */
	-1,			/* usb overcurrent map on port 9-8 */
	8,			/* trap port 0x92 */

	1,			/* sata channel enable */
	0,			/* class type :0 ide */
	0,			/* 0:not auto off ,other:auto off */
	1,			/* enable usb11 */
	1,			/* enable usb20 */
	0,			/* ac97 audio auto */
	0,			/* mc97 modem auto */
	0,			/* azalia auto */
	0,			/* aza snoop disable */
	1,			/* aza pin cfg enable */
	0,			/* front panel auto */
	2,
	2,
	2,
	2,
	0,			/* not detected */
	0,			/* disable sata smbus */
	0,			/* disable sata cache */
	0, 
	0,
	0,			/* disable arb2_en */
	0,			/* disable abrst_smi */
	1,			/* enable spd spec */
	4,
	1,			/* enable al clk gate */
	4, 
	1, 			/* enable bl clk gate */
	1,			/* enable ds pt */
	0,			/* usb xlink:blink */
	1,			/* enable usb_oc_wa : enable write OC map to this struct asaf */
	0,			/* sata avdd :1.2v */	
	0,
	1,			/* sata port mult cap on */
	0,
	1,			/* sata hot plug cap on */
	0xf,		/* all 4 port unconnected : sata_sts */
	0,			/* disable hpet */
	0,			/* disable osc */
	0,			/* disable clkrun_ctrl */
	0,			/* clkrun_ctrl_h */
	0, 			/* clkrun_ow */
	0, 			/* disable tpm */
	0x15, 
	&fan0, 		/* fan0 */
	&fan1,		/* fan1 */
	&fan2, 		/* fan2 */
	0xff, 		/* pciclk : enable all 8 clocks
				 * bit0 for PCICLK0, bit1 for PCICLK1, bit2 for PCICLK2, bit3 for PCICLK3 
				 * bit4 for PCICLK4, bit5 for PCICLK5, bit6 for PCICLK6, bit7 for PCICLK7 */
	1			/* primary ide_controller */	
};

extern int atiKbRstWaCheck(void);
extern void atiKbRstWa(void);
#if P92_TRAP_ON_PIO < 8
	extern int atiP92WaCheck(void);
	extern void atiP92Wa(void);
#endif
extern int atiSbirqSmiCheck(void);
extern void atiSbirqSmi(void);


struct ati_sb_smi atiSbSmiPostTbl[] = { 
	{atiKbRstWaCheck, atiKbRstWa, 0x39, 0x1},
#if P92_TRAP_ON_PIO < 8
	{atiP92WaCheck, atiP92Wa, P92_TRAP_EN_REG + 1, P92_TRAP_EN_REG, P92_TRAP_EN_BIT},
#endif
	{atiSbirqSmiCheck, atiSbirqSmi, 0x06, 1, 0X03, 1},
	{-1, -1, -1, -1, -1, -1, EXCLUSIVE_RESOURCE_MODE} /*end flag*/
};


struct ati_sb_smi  atiSbSmiRunTbl[RUNTBL_SIZE] = {
	{-1, -1, -1, -1, -1, -1, EXCLUSIVE_RESOURCE_MODE},
	{-1, -1, -1, -1, -1, -1, EXCLUSIVE_RESOURCE_MODE},
	{-1, -1, -1, -1, -1, -1, EXCLUSIVE_RESOURCE_MODE},
	{-1, -1, -1, -1, -1, -1, EXCLUSIVE_RESOURCE_MODE},
	{-1, -1, -1, -1, -1, -1, EXCLUSIVE_RESOURCE_MODE}
};
