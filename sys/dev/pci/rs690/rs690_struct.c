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
 * RS690_STRUCT :
 * 	define RS690 relative structure
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

/*-----------------------------------------------------------------------*/

u32 debug_index = 0;

/* PCIE configuration structure */
PCIE_CFG ati_pcie_cfg = {
	  CONFIG_DUAL_SLOT_SUPPORT_DISABLE 
	| CONFIG_OVERCLOCK_DISABLE 
	| CONFIG_GPP_CLK_GATING_DISABLE 
	| CONFIG_STATIC_DEV_REMAP_DISABLE 
	| CONFIG_OFF_UNUSED_GFX_LANES_ENABLE 
	| CONFIG_OFF_UNUSED_GPP_LANES_ENABLE 
	| CONFIG_10PER_OC_DISABLE 
	| CONFIG_HIDE_UNUSED_PORTS_ENABLE 
	| CONFIG_LANE_REVERSAL_DISABLE 
	| CONFIG_GFXCARD_WORKAROUND_DISABLE  		//only needed for RV370 & RV380 CHIPSET.
	| CONFIG_TMDS_DISABLE 
	| CONFIG_GFX_CLK_GATING_DISABLE 
	| CONFIG_DUALSLOT_AUTODETECT_DISABLE 
	| CONFIG_TMDS_DETECT_DISABLE 
	| CONFIG_GFX_COMPLIANCE_DISABLE 
	| CONFIG_GPP_COMPLIANCE_DISABLE,		/*Config*/

	1,						/* ResetReleaseDelay */

	LINK_WIDTH_X8,			/* Gfx0Width */

	LINK_WIDTH_X8,			/* Gfx1Width */

	PAYLOAD_SIZE_64B,		/* GfxPayload */
	
	PAYLOAD_SIZE_64B,		/* GppPayload */

	GPP_CONFIG_E,			/* GppConfig */

	  (PORT_ENABLE << PORT(2))
	| (PORT_ENABLE << PORT(3))
	| (PORT_ENABLE << PORT(4))
    | (PORT_ENABLE << PORT(5))
   	| (PORT_ENABLE << PORT(6))
   	| (PORT_ENABLE << PORT(7)),		/* PortEn */

	0,						/* PortDetect, filled by GppSbInit */

#if	1							// HP is not supported yet
	0,							/* PortHp disable */
#else
	  (PORT_ENABLE << PORT(4))
	| (PORT_ENABLE << PORT(5))
 	| (PORT_ENABLE << PORT(6))
	| (PORT_ENABLE << PORT(7)),							/* PortHp */
#endif

	 DBG_CONFIG_LONG_RECONFIG
//	| DBG_CONFIG_NB_SB_VC			/* we will use it later */
//	| DBG_CONFIG_SB_COMPLIANCE		/* open it if test used */
//	| DBG_CONFIG_LINK_LC_REVERSAL	/* there is no reversal */
//	| DBG_CONFIG_BYPASS_SCRAMBLER	/* bypass the scrambler when training if you want it. default no */
	| DBG_CONFIG_DYNAMIC_BUF_ALLOC	/* liujl ??? : the system will hange if open it */
	| DBG_CONFIG_GFX_DEV2_DEV3,		/* DbgConfig */

	0,								/* DbgConfig2 */

	0,						/* NbRevision, filled by rs690_misc_init */

/******************************* POST USED ***********************************/

	0x03,				/* GfxLx : ASPM MASK */
	0x03,				/* GppLx : ASPM_MASK */
	0x03,				/* NBSBLx  : ASPM_MASK */
	0x01,				/* PortSlotInit : slot init or not*/
	0x4B,				/* Gfx0Pwr : slot power cap, unit W by default */
	0x4B,				/* Gfx1Pwr : slot power cap, unit W by default */
	0x19				/* GppPwr : slot power cap, unit W by default */
};

/*--------------------------------------------------------------------------------*/

/*
 * internal used, most will be filled by vuma init
 */
struct ati_integrated_system_info ati_int_info = {
/* basic */
	0,							// size of this struct, filled by init
	
	0,							// major_ver, filled by init

	0,							// minor_ver, filled by init

	0,							// bootup_engine_clock, filled by init, 10KHz unit

	0,							// bootup_memory_clock, filled by init, 10KHz unit

	0,							// max_system_memory_clock, filled by init, 10KHz unit

	0,							// min_system_memory_clock, filled by init, 10KHz unit

	0,							// num_of_cycle_in_period_hi, filled by init

	0,							// Reserved1

	0,							// Reserved2

	0,							// inter_nb_voltage_low

	0,							// inter_nb_voltage_high

	0,							// Reserved3

	0,							// fsb, MHz unit

	0,							// cap_flag

/* upstream part */
	0,							// pcie_nbcfg_reg7, filled by init

	0,							// k8 memory clock, filled by init

	0,							// k8 sync start delay, filled by init

	0,							// k8 data return time, filled by init

	0,							// max_nb_voltage, filled by init
	
	0,							// min_nb_voltage, filled by init

	0,							// memory_type, filled by init
	
/* others */
	0,							// num of cycles in period, filled by init

	0,							// start_pwm_high_time, filled by init
	
	0,							// ht_link_width, filled by init
	
	0,							// max_nb_voltage_high, filled by init
	
	0							// min_nb_voltage_high, filled by init
};

struct vuma_gen_info vuma_gen_info = {
	0,				// uma_support_map

	0, 				// rec_uma_size		recored uma size

	0				// mclk_sss
};

/* nb basic struct */
NBCONFIGINFO	ati_nb_cfg = {
	0
#ifdef GPU_UMA
	| GFX_UMA_ENABLE
#endif
#ifdef GPU_SP
	| GFX_SP_ENABLE
	| GFX_ASYNC_MODE
#endif
//	| GFX_TMDS_PHY_MODE
//	| GFX_ASYNC_MODE
//	| GFX_CLK_REF_PCIE
//	| GFX_CLOCK_GATING
	| GFX_2D_OPTIMIZATION,				// gfx_config

	  EXT_GFX_SVIEW_ENABLE
	| EXT_GFX_MULTIFUNC_ENABLE
	| EXT_DEBUG_NB_DYNAMIC_CLK
//	| EXT_DEBUG_GFX32BIT_MODE
//	| EXT_DEBUG_GFX_MANUAL_MODE
//	| EXT_DEBUG_INTERLEAVE_256BYTE
//	| EXT_GFX_FORCE_IGFX_ENABLE
	| EXT_AZALIA_ENABLE,				// ext config

#if (SHARED_VRAM == 128)
    0x80,   // MB unit chengguipeng         // uma_fb_size, filled by init
    0x80,                       // sp_fb_size, filled by init
#elif (SHARED_VRAM == 64)
	0x40,	// MB unit			// uma_fb_size, filled by init
    0x40,                       // sp_fb_size, filled by init
#elif (SHARED_VRAM == 32)
    0x20,   // MB unit chengguipeng         // uma_fb_size, filled by init
    0x20,                       // sp_fb_size, filled by init
#endif

	AUTO_INTERLEAVE,					// sp_interleave_mode

	0,									// sp_interleave_size

	0, 									// sp_interleave_ratio

	0,									// sp_mc_init_table_ptr, fill when you want to init mc by yourself

	0,									// sp_dbg_config

	0,									// nb_revision, filled by init

	0, 									// pcie_gfx_info, filled by detection, bit0 for PCI, others for every slot

	0, 									// ati_gfx_info, same as pcie_gfx_info, set when ATI GFX founded

#ifdef GPU_UMA
	0x7c0,	 							// system_memory_tom_lo : 1MB based, 1G
#elif GPU_SP
	0xff8,
#endif
	0,									// system_memory_tom_hi : 1MB based

	0,									// cstate

	0,									// memory clock, filled by init

	0,									// engine clock, filled by init

	&ati_int_info,						// struct ati_integrated_system_info
	
	&vuma_gen_info						// struct of struct vuma_gen_info
};


