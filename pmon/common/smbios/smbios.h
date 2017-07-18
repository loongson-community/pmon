/*
 * smbios.h - interface for Loongson PMON SMBIOS generation
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 * Copyright (C) IBM Corporation, 2006
 * Authors: Andrew D. Ball <aball@us.ibm.com>
 *
 * Adapated for Loongson PMON
 * (C) Copyright 2012 <meiwenbin@loongson.cn> and <fandongdong@loongson.cn>
 */

#ifndef SMBIOS_H
#define SMBIOS_H

#include <stdlib.h>

/* These constants must agree with the memory map as the address and the ROMBIOS pulls 
 * the SMBIOS entry point from in the smbios_init subroutine.
 */
#define SMBIOS_PHYSICAL_ADDRESS 0x8fffe000
#define SMBIOS_SIZE_LIMIT 0x800
#if defined(LOONGSON_2K)
#define LS2K_BOARD_NAME "Loongson-2K-SOC-1w-V0.6-demo"
#endif

extern void loongson_smbios_init(void);
#endif /* SMBIOS_H */
