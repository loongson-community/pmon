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
 * GFX_PRE :
 * 	GFX relative, spec. for Internal GFX
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

/*-------------------------------------------------------------------------*/

#define	TEMP_SEC_BUS		0x20
#define	MAX_TEMP_SEC_BUS	0x40
#define	is_dev_present(x)	(_pci_conf_read(x, 0x00) != 0xffffffff)

static u8	g_sec_bus = TEMP_SEC_BUS;
static void gfx_scan_func(pcitag_t dev); 
static void gfx_scan_dev(pcitag_t dev);
static void gfx_scan_bus(pcitag_t dev);

/*
 * gfx_scan_dev :
 * 	Scan one device with detecting GFX card device.
 */
static void gfx_scan_dev(pcitag_t dev)
{
	int i;
	int func_num;

	if(_pci_conf_readn(dev, 0x0E, 1) & (1 << 7)){
		func_num = 7;
	}else{
		func_num = 1;
	}
func_num = 1 ; //jianf
	for(i = 0; i < func_num; i++){
		dev = (dev & 0xfffff8ff) | (i << 8);
		if(is_dev_present(dev)){
			gfx_scan_func(dev);
		}
	}

	return;
}

/*
 * gfx_scan_bus :
 * 	Scan one bus with detecting GFX card device.
 */
static void gfx_scan_bus(pcitag_t dev)
{
	int i;
	pcitag_t sub_dev;
	u8 cur_bus = _pci_conf_readn(dev, 0x19, 1);

	for(i = 0; i < 32; i++){
		sub_dev = _pci_make_tag(cur_bus, i, 0);
			printf("gfx_scan_bus,devtag=0x%x,cur_bus=%d,i=%d,sub_dev=%x\n",dev,cur_bus,i,sub_dev);
		if(is_dev_present(sub_dev)){
			gfx_scan_dev(sub_dev);
		}
	}

	return;
}

/*
 * gfx_scan_func :
 * 	Scan one function for any GFX device and storing the information
 * 	on ati_nb_cfg struct
 */
static void gfx_scan_func(pcitag_t dev)
{
	u8 reg8;
	u32 reg32;
	int bri_bus, bri_dev, bri_func;
	u8 is_pci_bus = 0;

	_pci_break_tag(dev, &bri_bus, &bri_dev, &bri_func);
	if(dev == _pci_make_tag(0, 0x14, 4)){
		is_pci_bus = 1;
	DEBUG_INFO("gfx_scan_fuc will scan pci bus(0x%x)\n", dev);
	}

	/* judge is bridge or not, so we will scan the next bus */
	reg8 = _pci_conf_readn(dev, 0x0E, 1);
	DEBUG_INFO("gfx_scan_func() ---> reg0E(0x%x) id(0x%x)\n", reg8, _pci_conf_read(dev, 0x00));
	reg8 &= 0x7F;
	if(reg8 == 1){
		/* this is the bridge system */
		_pci_conf_writen(dev, 0x1B,0x40, 1);
		/* setting the pri_bus */
		_pci_conf_writen(dev, 0x18, bri_bus, 1);
		/* setting the subordinate bus */
		_pci_conf_writen(dev, 0x1A, MAX_TEMP_SEC_BUS, 1);
		/* setting the sec bus */
		_pci_conf_writen(dev, 0x19, g_sec_bus++, 1);
	DEBUG_INFO("gfx_scan_func() ---> bridge config reg0x18(0x%x)\n", _pci_conf_read(dev, 0x18));
		/* scan the next bus */
		gfx_scan_bus(dev);
		/* clean it */
		reg32 = _pci_conf_read(dev, 0x18);
		reg32 &= 0xff000000;
	//	_pci_conf_write(dev, 0x18, reg32);
	DEBUG_INFO("after gfx_scan_func() ---> bridge config reg0x18(0x%x)\n", _pci_conf_read(dev, 0x18));
	}else{
	DEBUG_INFO("gfx_scan_func() -none bridge:0x%x--> device config reg0x18(0x%x)\n",reg8, _pci_conf_read(dev, 0x18));
		/* this is the device system */
		if(_pci_conf_readn(dev, 0x1B, 1) == 0x03){
			if(is_pci_bus){
				ati_nb_cfg.pcie_gfx_info |= 1 << 0;
			}else{
				ati_nb_cfg.pcie_gfx_info |= 1 << ((dev & 0x0000f800) >> 11);
			}
			if(_pci_conf_readn(dev, 0x00, 2) == 0x1002){
				if(is_pci_bus){
					ati_nb_cfg.ati_gfx_info |= 1 << 0;
				}else{
					ati_nb_cfg.ati_gfx_info |= 1 << ((dev & 0x0000f800) >> 11);
				}
			}
		}
	}

	return;
}

/*
 * get_pci_gfx_card : 
 * collection PCI gfx card information
 */
static void get_pci_gfx_card(void)
{
	pcitag_t dev = _pci_make_tag(0, 0x14, 4);

	gfx_scan_func(dev);
	
	return;
}

/*
 * get_pcie_gfx_card :
 * 	Collecting all the GFX card information in the fabric
 * 	and storing the information to ati_nb_cfg struct
 */
static void get_pcie_gfx_card(void)
{
	pcitag_t dev;
	int i;
	
	for(i = 2; i < 8; i++){
		dev = _pci_make_tag(0, i, 0);
		if(is_dev_present(dev)){
			DEBUG_INFO("dev(%d) id(0x%x)\n", i, _pci_conf_read(dev, 0));
			gfx_scan_func(dev);
		}
	}

	return;
}

/*------------------------------------------------------------------*/

/*
 * uma_size : used for caculating interleave mode
 */
static u8 uma_size[] = {
#if (SHARED_VRAM == 128)
	0x80,		// 128M -> 2G 0x80 : 0x20 for debugging with (uma_fb_size equal) : by liujl
#elif (SHARED_VRAM == 64)
	0x40,		// 128M -> 2G 0x80 : 0x20 for debugging with (uma_fb_size equal) : by liujl
#elif (SHARED_VRAM == 32)
	0x20,
#endif
	0x80,		// 128M -> 1G
	0x40,		// 64M -> 512M
	0x40,		// 64M -> 384M
	0x20,		// 32M -> 256M
	0x20,		// 32M -> 128M
	0x10,		// 16M -> 64M
	0x08		// 8M -> 32M
};

/*
 * aperture_size : BAR0 aperture size for both or single fb
 */
static u8 aperture_size[] = {
#if (SHARED_VRAM == 128)
	0x00,	// 2G 0x01 : 0x80000000 for testing with  32MB aperture with properly(uma_fb_size) : by liujl
#elif (SHARED_VRAM == 64)
	0x02,	// 2G 0x01 : 0x80000000 for testing with  32MB aperture with properly(uma_fb_size) : by liujl
#elif (SHARED_VRAM == 32)
	0x03,	// 2G 0x01 : 0x80000000 for testing with  32MB aperture with properly(uma_fb_size) : by liujl
#endif
	0x01,	// 1G	---> 256MB
	0x01,	// 512M	---> 256MB
	0x01,	// 384M	---> 256MB
	0x01,	// 256M
	0x00,	// 128M
	0x02,	// 64M
	0x03	// 32M
};

/*
 * gfx_config_fb_size :
 * validate UMA FB size and setting GFX FB aperture
 */
static u8 get_uma_size_index(void)
{
	u32 memory_size;
	u8 index;

	memory_size = ati_nb_cfg.system_memory_tom_lo;
	if(memory_size >= 0x800){					// 2G
		index = 0;
	}else if(memory_size >= 0x400){		// 1G
		index = 1;
	}else if(memory_size >= 0x200){		// 512M
		index = 2;
	}else if(memory_size >= 0x180){		// 384M
		index = 3;
	}else if(memory_size >= 0x100){		// 256M
		index = 4;
	}else if(memory_size >= 0x80){		// 128M
		index = 5;
	}else if(memory_size >= 0x40){		// 64M
		index = 6;
	}else{														// 32M
		index = 7;
	}

	return index;
}

/*
 * gfx_auto_config :
 * GFX auto configuration for ati_nb_cfg config
 */
static void gfx_auto_config(void)
{
	/* sp fb size */
#ifdef	CFG_SP_SUPPORT
	if(ati_nb_cfg.gfx_config & GFX_SP_ENABLE){
		if(ati_nb_cfg.sp_fb_size == 0){
			ati_nb_cfg.sp_fb_size = CFG_SP_SIZE;
		}			
	}
#endif
	
	/* auto config the uma fb size according to the recomentation : 
	 * for dual mode we donot using CFG_ yet.
	 */
	if( (ati_nb_cfg.uma_fb_size == 0) && (ati_nb_cfg.gfx_config & GFX_UMA_ENABLE) ){
		ati_nb_cfg.uma_fb_size = 0x40;		// default 64MB
		if(ati_nb_cfg.system_memory_tom_lo >= 0x400){
			ati_nb_cfg.uma_fb_size = 0x80;	// 128MB if large system memory(at lease 1GB)
		}
	
		/* sp uma interleave and mode setting, we can just setting when SP enabled and AUTO interleave mode used */
#ifdef	CFG_SP_SUPPORT
		if( (ati_nb_cfg.gfx_config & GFX_SP_ENABLE) && (ati_nb_cfg.sp_interleave_mode == AUTO_INTERLEAVE) ){
			if(ati_nb_cfg.sp_fb_size == 0x40){
				ati_nb_cfg.sp_interleave_ratio = 1;
				if(ati_nb_cfg.uma_fb_size >= 0x80){
					ati_nb_cfg.sp_interleave_size = 42;
				}else{
					ati_nb_cfg.sp_interleave_size = 21;
				}
				ati_nb_cfg.sp_interleave_mode = FINE_INTERLEAVE;
			}else{
				ati_nb_cfg.sp_interleave_mode = COARSE_INTERLEAVE;
			}
		}
#endif
	}
	
	/* setting 32/64BITS mode, 64BIT for auto setting */
	if(!(ati_nb_cfg.ext_config & EXT_DEBUG_GFX_MANUAL_MODE)){
		ati_nb_cfg.ext_config &= ~EXT_DEBUG_GFX32BIT_MODE;
	}
	
	return;
}

/*
 * gfx_config_sb_size :
 * config the sb size according to configuration
 */
static void gfx_config_fb_size(void)
{
	pcitag_t nb_dev = _pci_make_tag(0, 0, 0);
	u8 index = 0;
		
	if( (ati_nb_cfg.uma_fb_size != 0) && (ati_nb_cfg.gfx_config & GFX_UMA_ENABLE) ){
		index = get_uma_size_index();
		if(ati_nb_cfg.uma_fb_size > uma_size[index]){
			ati_nb_cfg.uma_fb_size = uma_size[index];
		}
	}
	
	if(ati_nb_cfg.uma_fb_size + ati_nb_cfg.sp_fb_size > 0x400){
		ati_nb_cfg.uma_fb_size /= 2;			// just for adapter
	}
	
	set_nbcfg_enable_bits(nb_dev, 0x8C, 0x07 << 4, aperture_size[index] << 4);
	
	ati_nb_cfg.vuma_gen->uma_support_map = uma_size[index];
	
	return;
}

/*-----------------------------------------------------------------------*/

/*
 * rs690_gfx_pre_init :
 * 	GFX pre init routine
 */
void gfx_pre_init(void)
{
	pcitag_t nb_dev = _pci_make_tag(0, 0, 0);
	
	DEBUG_INFO("GFX PRE STAGE : start gfx pre init.\n");
	
	/* collection PCIE GFX device */
//	get_pcie_gfx_card();
	/* collection PCI GFX device */
	get_pci_gfx_card();

	DEBUG_INFO("GFX CARDS : 0x%x\n", ati_nb_cfg.pcie_gfx_info);
	DEBUG_INFO("RADEON GFX CARDS : 0x%x\n", ati_nb_cfg.ati_gfx_info);

	/* Internal GFX enable */
	if(_pci_conf_read(nb_dev, 0x8C) & (1 << 1)){
		/* is ati_nb_cfg enable SP or UMA  */
		if(ati_nb_cfg.gfx_config & (GFX_SP_ENABLE | GFX_UMA_ENABLE)){
			if(!(ati_nb_cfg.ext_config & EXT_GFX_FORCE_IGFX_ENABLE)){
				if(ati_nb_cfg.ati_gfx_info == 0){
					ati_nb_cfg.ext_config &= ~EXT_GFX_SVIEW_ENABLE;
				}
				if(ati_nb_cfg.pcie_gfx_info != 0){
					if(!(ati_nb_cfg.ext_config & EXT_GFX_SVIEW_ENABLE)){
						ati_nb_cfg.gfx_config &= ~(GFX_SP_ENABLE | GFX_UMA_ENABLE);
					}
				}
			}
		}
	}else{
		ati_nb_cfg.gfx_config &= ~(GFX_SP_ENABLE | GFX_UMA_ENABLE);	
	}

	/* gfx with SP or UMA init */
	if(ati_nb_cfg.gfx_config & (GFX_SP_ENABLE | GFX_UMA_ENABLE)){
		if(!(ati_nb_cfg.ext_config & EXT_GFX_SVIEW_ENABLE)){
			set_nbcfg_enable_bits(nb_dev, 0x8C, 1 << 0, 0 << 0);
		}
		/* auto config parameters */
		gfx_auto_config();
		/* config fb size */
		gfx_config_fb_size();
	}else{
		ati_nb_cfg.sp_fb_size = ati_nb_cfg.uma_fb_size = 0x00;
	}
	
	DEBUG_INFO("GFX PRE STAGE : done.\n");

	return;
}

/*------------------------------------------------------------------*/
/*
 * gfx_disable :
 * RS690 GFX disable routine
 */
void gfx_disable(void)
{
	pcitag_t nb_dev = _pci_make_tag(0, 0, 0);
	pcitag_t apc_dev = _pci_make_tag(0, 1, 0);

	/* disable GFX bridge decode and bus arbiter */
	_pci_conf_write(apc_dev, 0x04, 0xFFFF0000);
	_pci_conf_write(apc_dev, 0x18, 0x00000000);

	/* disable Internal GFX and make it hiden */
	set_nbcfg_enable_bits(nb_dev, 0x8C, 1 << 2, 1 << 2);
	set_nbcfg_enable_bits(nb_dev, 0x7C, 1 << 0, 1 << 0);

	return;
}

