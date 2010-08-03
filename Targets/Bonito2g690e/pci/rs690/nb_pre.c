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
 *
 * NB_PRE :
 * 	NorthBridge pre init routine
 */

/*-----------------------------------------------------------------------*/

#include <stdio.h>
#include <stdarg.h>
#include <sys/linux/types.h>
#include <include/bonito.h>
#include <machine/pio.h> 
#include <include/rs690.h>
#include <pci/rs690_cfg.h>
#include "rs690_config.h"

/*---------------------------------------------------------------------*/

extern int memorysize_high;

/*
 * nb_pre_init :
 * NorthBridge init before PCI emulation
 */
void nb_pre_init(void)
{
	pcitag_t nb_dev = _pci_make_tag(0, 0, 0);

	DEBUG_INFO("+++++++++++++++++++++++++++++++++++++++++++++++ NB PRE STAGE ++++++++++++++++++++++++++++++++++++\n");

	/* disable NB_BAR2_PM2 */
	set_nbcfg_enable_bits(nb_dev, 0x4C, 1 << 17, 0 << 17);

	/* make preparation for pci_nb_cfg struct */
	ati_nb_cfg.nb_revision = get_nb_revision();

	/* Readjust the size of system_memory_tom_lo */
	if(memorysize_high > (1000 << 20))
		ati_nb_cfg.system_memory_tom_lo = 0x1000;
	else
		ati_nb_cfg.system_memory_tom_lo = 0x7c0;

	/* PCI memory top configure.
	 * The address is like a filter to walk thougth PCI's address.
	 * FIXME (wgl): when systerm_memory_tom_lo set 0x800 in sp mode, the network don't work;
	 *         But when systerm_memory_tom_lo set 0x800 in sp mode, the network work correctly.
	 */
#ifdef GPU_SP
		ati_nb_cfg.system_memory_tom_lo = 0xff8;
#endif

	/* setting TOM LOW basically */
	_pci_conf_write(nb_dev, 0x90, ati_nb_cfg.system_memory_tom_lo << 20);

	/* gfx pre init */
#if	defined(CFG_UMA_SUPPORT) || defined (CFG_SP_SUPPORT)
	gfx_pre_init();
#endif

	/* report UMA information to user  */
	gfx_uma_state_dispatch();

#if		defined(CFG_UMA_SUPPORT) || defined(CFG_SP_SUPPORT)
	if(ati_nb_cfg.gfx_config & (GFX_UMA_ENABLE | GFX_SP_ENABLE)){
		if(ati_nb_cfg.ext_config & EXT_AZALIA_ENABLE){
			/* open F2 for azalia : GFX HD  */
			set_nbcfg_enable_bits(nb_dev, 0x8C, 1 << 8, 1 << 8);
		}
		if(ati_nb_cfg.ext_config & EXT_GFX_MULTIFUNC_ENABLE){
			/* enable multifunctions for GFX */
			set_nbcfg_enable_bits(nb_dev, 0x8C, 1 << 7, 1 << 7);
		}
	}else{
		/* disable GFX device */
		gfx_disable();
	}
#endif

	/* resetting TOM LO if UMA is enabled */
	if(ati_nb_cfg.gfx_config & GFX_UMA_ENABLE){
		_pci_conf_write(nb_dev, 0x90, (ati_nb_cfg.system_memory_tom_lo - ati_nb_cfg.uma_fb_size) << 20);
	}
	/* setting TOM HI if needed */
	if(ati_nb_cfg.system_memory_tom_hi){
		htiu_write_index(nb_dev, 0x31, (ati_nb_cfg.system_memory_tom_hi >> 12) & 0x00000fff);	// (x << 20) >> 32
		htiu_write_index(nb_dev, 0x30, (ati_nb_cfg.system_memory_tom_hi << 20) & 0xfff00000);	// x << 20
	}

	DEBUG_INFO("----------------------------------------------------NB PRE STAGE DONE-----------------------------------\n");
	return;
}

/*----------------------------------------------------------------------------------*/

