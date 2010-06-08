/*
 * SB600 registers definition
 *
 * Author : liujl <liujl@lemote.com>
 * Date : 2009-07-29
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

#ifndef	_SB600_H_
#define	_SB600_H_

/*------------------------------------------------------------------------------------------------*/
/* Power management index/data registers */
#define PM2_INDEX	0xCD0
#define PM2_DATA	0xCD1
#define PM_INDEX	0xCD6
#define PM_DATA		0xCD7

/* ABLINK REGISTER INDEX */
#define AB_INDX   0xCD8
#define AB_DATA   0xCDC
// internal index for accessing
#define AX_INDXC  0
#define AX_INDXP  1
#define AXCFG     2
#define ABCFG     3

/* ASIC chip revision */
#define	REV_SB600_A11		0x11
#define	REV_SB600_A12		0x12
#define	REV_SB600_A13		0x13
#define	REV_SB600_A21		0x21

/*-------------------------------------------------------------------------------------------------*/
#endif	/* _SB600_H_ */
