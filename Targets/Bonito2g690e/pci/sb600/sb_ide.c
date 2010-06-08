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
 * sb_disable_sec_id :
 * disable second ide if needed
 */
void sb_disable_sec_ide(void)
{
	pcitag_t dev = _pci_make_tag(0, 20, 1);

	PRINTF_DEBUG();
	set_sbcfg_enable_bits(dev, 0x64, 1 << 3, 1 << 3);

	/* If SB IDE is in Native mode, Secondary IDE can not be disabled;
	 * Otherwise, Windows reboot with error code "7B"
	 */
	if(!(_pci_conf_read(dev, 0x08) & (1 << 5))){
		/* Dont disable Secondary IDE because if SB Secondary IDE is disabled,
		 *  W2K S3 doesn't work
		 */
	//	set_sbcfg_enable_bits(dev, 0x48, 1 << 0, 1 << 0);
	}

	return;
}


