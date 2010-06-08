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

#ifndef	_SB600_CFG_H_
#define	_SB600_CFG_H_
#include <sys/linux/types.h>
#include <dev/pci/pcivar.h>
/*------------------------------------ REG ACCESS FUNCTIONS-------------------------------------*/

extern u8 pm_ioread(u8 reg);
extern void pm_iowrite(u8 reg, u8 value);
extern u8 pm2_ioread(u8 reg);
extern void pm2_iowrite(u8 reg, u8 value);
extern void set_pm_enable_bits(u32 reg_pos, u32 mask, u32 val);
extern void set_pm2_enable_bits(u32 reg_pos, u32 mask, u32 val);

extern void set_sbcfg_enable_bits(pcitag_t sb_dev, u32 reg_pos, u32 mask, u32 val);

void alink_ab_indx(unsigned int reg_space, unsigned int reg_addr, unsigned int mask, unsigned int val);
void alink_ax_indx(unsigned int space, unsigned int axindc, unsigned int mask, unsigned int val);
	
#endif	/* _SB600_CFG_H_ */
/*------------------------------------------------------------------------------------------------------*/
