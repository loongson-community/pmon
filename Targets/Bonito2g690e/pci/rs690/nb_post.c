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
 * RS690_NB :
 * 	NorthBridge relative init routine
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

/*
 * nb_misc_clock :
 * rs690 misc clock parameters setting
 */
static void nb_misc_clock(void)
{
	pcitag_t clk_dev = _pci_make_tag(0, 0, 1);
	pcitag_t nb_dev = _pci_make_tag(0, 0, 0);
	pcitag_t gfx_dev2 = _pci_make_tag(0, 2, 0);
	u8 rev = get_nb_revision();
	u32 val;

	/* visible CLK func */
	set_nbcfg_enable_bits(nb_dev, 0x4C, 1 << 0, 1 << 0);

	if(ati_nb_cfg.ext_config & EXT_DEBUG_NB_DYNAMIC_CLK){
		/* disable NB dynamic clock to htiu rx */
		set_nbcfg_enable_bits(clk_dev, 0xE8, 0x07 << 12, 1 << 13);
		/* ENABLE : CLKGATE_DIS_GFX_TXCLK & CLKGATE_DIS_GPPSB_CCLK & CLKGATE_DIS_CFG_S1X */
		set_nbcfg_enable_bits(clk_dev, 0x94, (1 << 16) | (1 << 24) | (1 << 28), 0);
		/* ENABEL : CLKDATE_DIS_IOC_CCLK_MST/SLV, enabel clkdate for C/MCLK goto BIF branch  */
		set_nbcfg_enable_bits(clk_dev, 0x8C, (1 << 13) | (1 << 14) | (1 << 24) | (1 << 25), 0);

		if(rev < REV_RS690_A21){
			/* CKLGATE_DIS_IO_CCLK_MST  */
			set_nbcfg_enable_bits(clk_dev, 0x8C, 1 << 13, 1 << 13);
		}
	
		/* Powering Down efuse and strap block clocks in GFX mode as default */
		set_nbcfg_enable_bits(clk_dev, 0xCC, 1 << 24, 1 << 24);
		/* dynamic clock setting for MC and HTIU */
		val = nbmc_read_index(nb_dev, 0x7A);
		val &= 0xffffffc0;
		val |= 1 << 2;
		if(rev >= REV_RS690_A21){
			val &= ~(1 << 6);
			set_htiu_enable_bits(nb_dev, 0x05, 1 << 11, 1 << 11);
		}
		nbmc_write_index(nb_dev, 0x7A, val);

		if(ati_nb_cfg.gfx_config & (GFX_SP_ENABLE | GFX_UMA_ENABLE)){
			/* Powering Down efuse and strap block clocks in GFX mode : PWM???*/
			set_nbcfg_enable_bits(clk_dev, 0xCC, (1 << 23) | (1 << 24), 1 << 24);
		}else{
			/* nb only mode */
			/* Powers down reference clock to graphics core PLL */
			set_nbcfg_enable_bits(clk_dev, 0x8C, 1 << 21, 1 << 21);
			/* Powering Down efuse and strap block clocks after boot-up */
			set_nbcfg_enable_bits(clk_dev, 0xCC, (1 << 23) | (1 << 24), (1 << 23) | (1 << 24));
			/* powerdown clock to MC */
			set_nbcfg_enable_bits(clk_dev, 0xE4, 1 << 0, 1 << 0);
		}

		if(ati_nb_cfg.pcie_gfx_info == 0){
			if(_pci_conf_read(gfx_dev2, 0x00) == 0xffffffff){
				/* Powerdown GFX ports clock when no external GFX detected */
				set_nbcfg_enable_bits(clk_dev, 0xE8, 1 << 17, 1 << 17);
			}
		}
	}

	/* hide CLK func */
	set_nbcfg_enable_bits(nb_dev, 0x4C, 1 << 0, 0 << 0);

	if(rev >= REV_RS690_A21){
		set_htiu_enable_bits(nb_dev, 0x05, (1 << 8) | (1 << 9), (1 << 8) | (1 << 9));
		set_htiu_enable_bits(nb_dev, 0x05, (1 << 10), (1 << 10));
	}
	
	DEBUG_INFO("NB POST STAGE : nb_misc_clock function : should we use PWM for efuse and strap powerdown?\n");

	return;
}

/*
 * nb_lock :
 * 	close or hide unsed modules and lock the whole NB registers
 */
static void nb_lock(void)
{
	pcitag_t nb_dev = _pci_make_tag(0, 0, 0);
	pcitag_t igfx_dev = _pci_make_tag(1, 5, 0);
	u32 base;
	u32 val;

	/* enable 2D optimization as configured */
	if(ati_nb_cfg.gfx_config & (GFX_SP_ENABLE | GFX_UMA_ENABLE)){
		if( (!(ati_nb_cfg.ext_config & EXT_DEBUG_GFX32BIT_MODE)) && (ati_nb_cfg.gfx_config & GFX_2D_OPTIMIZATION) ){
			base = (_pci_conf_read(igfx_dev, 0x18) & 0xfffffff0) | 0xA0000000;
			if(base){
				/* open memory access */
				set_nbcfg_enable_bits(igfx_dev, 0x04, 1 << 1, 1 << 1);
				/* enable 2D accelerator */
				val = *(volatile u32 *)(base | 0x4100);	// 0x40FC + 4 ???
				val |= 1 << 0;
				*(volatile u32 *)(base | 0x4100) = val;
				DEBUG_INFO("NB POST STAGE : nb_lock : 2D enable : address abnormal?\n");
			}
		}
	}

	/* enable decode for debug BAR */
	if( (ati_nb_cfg.gfx_config & (GFX_SP_ENABLE | GFX_UMA_ENABLE)) && (_pci_conf_read(nb_dev, 0x8C) & (1 << 9)) ){
		set_nbcfg_enable_bits(nb_dev, 0x8C, 1 << 10, 1 << 10);
	}else{
		set_nbcfg_enable_bits(nb_dev, 0x8C, 3 << 9, 0 << 9);
	}

	/* Hide PM2 bar */
	set_nbcfg_enable_bits(nb_dev, 0x4C, 1 << 17, 0 << 17);

	/* set proper unused index for inderect space */

	/* lock the NB */
	set_nbmisc_enable_bits(nb_dev, 0x00, (1 << 0) | (1 << 7), (1 << 0) | (1 << 7));

	return;
}

/*
 * rs690_nb_post_init :
 * rs690 NorthBridge init after PCI emulation
 */
void nb_post_init(void)
{
	pcitag_t igfx_dev = _pci_make_tag(1, 5, 0);

	DEBUG_INFO("\n++++++++++++++++++++NB POST STAGE WITH REV(%d)+++++++++++++++++++++++++++++++++\n", get_nb_revision());


#if	defined(CFG_UMA_SUPPORT) || defined(CFG_SP_ENABLE)
	/* gfx_late_init */
	if( (ati_nb_cfg.gfx_config & (GFX_UMA_ENABLE | GFX_SP_ENABLE)) && (ati_nb_cfg.ext_config & EXT_GFX_SVIEW_ENABLE) ){
		set_nbcfg_enable_bits(igfx_dev, 0x04, 1 << 1, 1 << 1);	// enabel memory access
	}
#endif

	/* NB misc clock setting, powerdown as needed */
	nb_misc_clock();

	/* lock the whole NB registers */
	nb_lock();

	DEBUG_INFO("---------------------------------------------- NB POST STAGE DONE------------------------------\n");
	return;
}

/*-------------------------------------------------------------------------------*/

