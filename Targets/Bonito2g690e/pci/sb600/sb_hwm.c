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

/*----------------------------------------------------------------------*/

/*
 * convert_temp_to_duty :
 * convert the caculation
 */
static u32 convert_temp_to_duty(u32 temp)
{
	u32 base = 273;

	/* negtive temp */
	if(temp & (1 << 15)){
		temp &= ~(1 << 15);
		temp = base - temp;
	}else{
		/* positive temp */
		temp = base + temp;
	}
	temp &= 0x0000ffff;

	temp = ((temp * 10000) / 4344) << 6;

	return temp;
}


/* 
 * hwm_fan_config :
 * Programs the fan control registers based on setup option.
 */
static void hwm_fan_config(struct hwm_fan *fan, u8 number)
{
	u8 fan_misc_reg = (number * 13) + 1;
	u8 val;
	u8 *p;
	u32 temp;
	int i;

	PRINTF_DEBUG();
	val = pm2_ioread(fan_misc_reg);
	val &= 0xF8;
	/* Check whether software control or temperature control */
	if(fan->ctrl & 0x02){
		val |= 1 << 0;
	}
	/* Check whether step mode or linear mode */
	if(fan->mode & 0x01){
		val |= 1 << 1;
	}
	/* Check the polarity */
	if(fan->polarity & 0x01){
		val |= 1 << 2;
	}
	pm2_iowrite(fan_misc_reg, val);

	/* Calculate low temperature in kelvin */
	temp = convert_temp_to_duty((fan->lowtemphi << 8) | fan->lowtemp);
	fan->lowtemphi = (temp & 0x0000ff00) >> 8;
	fan->lowtemp = temp & 0x000000ff;

	temp = convert_temp_to_duty((fan->medtemphi << 8) | fan->medtemp);
	fan->medtemphi = (temp & 0x0000ff00) >> 8;
	fan->medtemp = temp & 0x000000ff;

	temp = convert_temp_to_duty((fan->hightemphi << 8) | fan->hightemp);
	fan->hightemphi = (temp & 0x0000ff00) >> 8;
	fan->hightemp = temp & 0x000000ff;

	/* fan struct fill */
	p = (u8 *)(&(fan->freqdiv));
	for(i = 0; i < 11; i++){
		pm2_iowrite(fan_misc_reg + 1 + i, p[i]);
	}

	return;
}


/*	1 Intialize the hardware monitor, enables the fan speed and temperature sensors
 *	2 configure all the 3 fan speed controls based on setup options
 *	3 Fan control structures(HWM_FAN) for all 3 fans should be initialized
 */
void sb_hwm_cfg(void)
{
	u8 ctrl;

	PRINTF_DEBUG();
	/* init the HWM registers */
	set_pm2_enable_bits(0x31, ~0xFF, 0x03);	// enable fan0 speed detect
	set_pm2_enable_bits(0x36, ~0xFF, 0x03);	// enable fan1 speed detect
	set_pm2_enable_bits(0x3B, ~0xFF, 0x03);	// enable fan2 speed detect
	set_pm2_enable_bits(0x40, ~0xFF, 0x08);
	set_pm2_enable_bits(0x42, ~0xFF, 0xAA);
	set_pm2_enable_bits(0x56, ~0xFF, 0x55);
	set_pm2_enable_bits(0x57, ~0xFF, 0x55);
	set_pm2_enable_bits(0x01, ~0xFF, 1 << 2);

	/* fan config */
	hwm_fan_config(ati_sb_cfg.fan0, 0);
	ctrl = ati_sb_cfg.fan0->ctrl;
	hwm_fan_config(ati_sb_cfg.fan1, 1);
	ctrl |= ati_sb_cfg.fan1->ctrl << 2;
	hwm_fan_config(ati_sb_cfg.fan2, 2);
	ctrl |= ati_sb_cfg.fan2->ctrl << 4;

	/* setting ctrl */
	set_pm2_enable_bits(0x00, 0x3F, ctrl);
	
	return;
}


