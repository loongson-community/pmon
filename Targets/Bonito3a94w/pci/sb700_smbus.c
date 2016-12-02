/*
 * This file is part of the coreboot project.
 *
 * Copyright (C) 2008 Advanced Micro Devices, Inc.
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
 */

#include "sb700_smbus.h"

static void alink_ab_indx(u32 reg_space, u32 reg_addr,
			  u32 mask, u32 val)
{
	u32 tmp;

	OUTL((reg_space & 0x3) << 30 | reg_addr, AB_INDX);
	tmp = INL(AB_DATA);

	//add for sb700
	reg_addr & 0x10000 ? OUTL(0, AB_INDX) : NULL;

	tmp &= ~mask;
	tmp |= val;

	/* printk_debug("about write %x, index=%x", tmp, (reg_space&0x3)<<30 | reg_addr); */
	OUTL((reg_space & 0x3) << 30 | reg_addr, AB_INDX);	/* probably we dont have to do it again. */
	OUTL(tmp, AB_DATA);
	
	//add for sb700
	reg_addr & 0x10000 ? OUTL(0, AB_INDX) : NULL;
}

/* space = 0: AX_INDXC, AX_DATAC
*   space = 1: AX_INDXP, AX_DATAP
 */
static void alink_ax_indx(u32 space /*c or p? */ , u32 axindc,
			  u32 mask, u32 val)
{
	u32 tmp;

	/* read axindc to tmp */
	OUTL(space << 30 | space << 3 | 0x30, AB_INDX);
	OUTL(axindc, AB_DATA);
	OUTL(space << 30 | space << 3 | 0x34, AB_INDX);
	tmp = INL(AB_DATA);

	tmp &= ~mask;
	tmp |= val;

	/* write tmp */
	OUTL(space << 30 | space << 3 | 0x30, AB_INDX);
	OUTL(axindc, AB_DATA);
	OUTL(space << 30 | space << 3 | 0x34, AB_INDX);
	OUTL(tmp, AB_DATA);
}


