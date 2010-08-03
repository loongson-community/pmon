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

#ifndef	_RS690_CONFIG_H_
#include <sys/linux/types.h>
#include <dev/pci/pcivar.h>

/*******************************BUG FIXUP ROUTINE**************************/

/* on FPGA platform, the ID maybe changed, but if you use the following define, it will be ok. */
#define	FIXUP_ID_CHANGE_ON_FPGA_BUG
/* if not open, effect the scan of PCIE device  */
#define	FIXUP_REG65_ON_FPGA_BUG
/*----------------------------DEBUG---------------------------------------*/

#define	COREV2

#define	DEBUG_LEVEL_0		0x00	// used for big break point information
#define	DEBUG_LEVEL_1		0x01	// used for function internel informaton

#define	PCIE_GFX_GPPSB_CORE_ISSUE		"PCIE_GFX_GPPSB_CORE"
extern u32 debug_index;

#define	ISSUE_DEBUG(x, fmt, args...)	printf("ISSUE(%s) STEP(%d) : " fmt, x, debug_index++, ##args)

#define	RS690_DEBUG
#ifdef	RS690_DEBUG
#define	DEBUG_INFO(fmt, args...)		\
do{ 									\
	switch(0){ 							\
		case DEBUG_LEVEL_0 : 			\
			printf(fmt, ##args); 		\
			break;						\
		case DEBUG_LEVEL_1 :			\
			printf("\t" fmt, ##args);	\
			break;						\
	}									\
}while(0);
#else
#define	DEBUG_INFO(fmt, args...)		
#endif

/*------------------------DEFINE--------------------------------------------*/

#define	ATI_PCIE_VERSION		0x34

#define	ASPM_L0S_ENABLE		0x01
#define	ASPM_L1_ENABLE		0x02
#define	ASPM_MASK			0x03
#define	PCIE_CAP_ID			0x10

#define	SP_MC_DELAY	200

extern void delay(u32 val);

/*-----------------------------------CFG--------------------------------------*/

/* GLOBAL CONFIG */

// nb_por.c
/* enable PM2 block configuration */
#define	CFG_NBCONFIG_PM2_ENABLE

// gfx_pre.c
/* dual slot support or not, not dual slot enable */
//#define	CFG_DUAL_SLOT_SUPPORT
#undef	CFG_DUAL_SLOT_SUPPORT
/* reference clock share of GPP and GFX */
//#define	CFG_PCIE_REFCLK_SHARE
#undef	CFG_PCIE_REFCLK_SHARE

/* gfx_post.c */
#ifdef GPU_UMA
	#define	CFG_UMA_SUPPORT
#else
	#undef	CFG_UMA_SUPPORT
#endif

#ifdef GPU_SP
	#define	CFG_SP_SUPPORT
	#define	CFG_SP_SIZE					0x40		// 1MB unit default 32MB
#else
	#undef	CFG_SP_SUPPORT
	#undef	CFG_SP_SIZE
#endif

/* ati_integrated_system_information struct parameters setting */
// 1MHz unit, default 400MHz for graphic engine
#define CFG_GFX_BOOTUP_ENGINE_CLOCK	400
// PWM period cycle
#define	CFG_NUM_CYCLE_IN_PERIOD		0x0ffff		// used for PWM
// start PWM high time
#define	CFG_START_PWM_HIGH_TIME		0x00
// max & min NB voltage
#define	CFG_MAX_NB_VOLTAGE			0x0000
#define	CFG_MIN_NB_VOLTAGE			0x0000
// inter nb voltage low & high
#define	CFG_INTER_NB_VOLTAGE_LOW	0x0000
#define	CFG_INTER_NB_VOLTAGE_HIGH	0x0000
// reserved data
#define	CFG_SYS_INFO_RESERVED		0x00000000
// system info of major version and minor version
#define	MAJOR_REVISION	0x01
#define	MINOR_REVISION	0x01
/* ati_integrated_system_information struct parameters setting end */

/* we should choose only one from them */
#define	CFG_SP_MCLK_133
#undef	CFG_SP_MCLK_166
#undef	CFG_SP_MCLK_200
#undef	CFG_SP_MCLK_266
#undef	CFG_SP_MCLK_300
#undef	CFG_SP_MCLK_333
#undef	CFG_SP_MCLK_400
#undef	CFG_SP_MCLK_533
#undef	CFG_SP_MCLK_633

/* pcie_post.c */
#define	CFG_POST_PCIE_INIT_PORTS
#undef	CFG_POST_NBSB_ASPM_SUPPORT
#undef	CFG_POST_GFX_ASPM_SUPPORT
#undef	CFG_POST_GPP_ASPM_SUPPORT
#define	CFG_POST_L1_INACTIVITY_TIME			0x0e		// 1S
#define	CFG_POST_L0S_INACTIVITY_TIME		0x0e		// 1S

/*--------------------------------PCIE CFG-----------------------------------*/

/*** Config ***/
#define	CONFIG_DUAL_SLOT_SUPPORT_ENABLE		(1 << 0)
#define	CONFIG_DUAL_SLOT_SUPPORT_DISABLE	(0 << 0)
#define	CONFIG_OVERCLOCK_ENABLE				(1 << 1)
#define	CONFIG_OVERCLOCK_DISABLE			(0 << 1)
#define	CONFIG_GPP_CLK_GATING_ENABLE		(1 << 2)
#define	CONFIG_GPP_CLK_GATING_DISABLE		(0 << 2)
#define	CONFIG_STATIC_DEV_REMAP_ENABLE		(1 << 3)
#define	CONFIG_STATIC_DEV_REMAP_DISABLE		(0 << 3)	
#define CONFIG_OFF_UNUSED_GFX_LANES_ENABLE	(1 << 4)
#define	CONFIG_OFF_UNUSED_GFX_LANES_DISABLE	(0 << 4)
#define	CONFIG_OFF_UNUSED_GPP_LANES_ENABLE	(1 << 5)
#define	CONFIG_OFF_UNUSED_GPP_LANES_DISABLE	(0 << 5)
#define	CONFIG_10PER_OC_ENABLE				(1 << 6)
#define	CONFIG_10PER_OC_DISABLE				(0 << 6)
#define	CONFIG_HIDE_UNUSED_PORTS_ENABLE		(1 << 7)
#define	CONFIG_HIDE_UNUSED_PORTS_DISABLE	(0 << 7)
#define	CONFIG_LANE_REVERSAL_ENABLE			(1 << 8)
#define	CONFIG_LANE_REVERSAL_DISABLE		(0 << 8)
#define	CONFIG_GFXCARD_WORKAROUND_ENABLE	(1 << 9)
#define	CONFIG_GFXCARD_WORKAROUND_DISABLE	(0 << 9)
#define	CONFIG_TMDS_ENABLE					(1 << 10)
#define	CONFIG_TMDS_DISABLE					(0 << 10)
#define	CONFIG_GFX_CLK_GATING_ENABLE		(1 << 11)
#define	CONFIG_GFX_CLK_GATING_DISABLE		(0 << 11)
#define	CONFIG_DUALSLOT_AUTODETECT_ENABLE	(1 << 12)
#define	CONFIG_DUALSLOT_AUTODETECT_DISABLE	(0 << 12)
#define	CONFIG_TMDS_DETECT_ENABLE			(1 << 13)
#define	CONFIG_TMDS_DETECT_DISABLE			(0 << 13)
#define	CONFIG_GFX_COMPLIANCE_ENABLE		(1 << 14)
#define	CONFIG_GFX_COMPLIANCE_DISABLE		(0 << 14)
#define	CONFIG_GPP_COMPLIANCE_ENABLE		(1 << 15)
#define	CONFIG_GPP_COMPLIANCE_DISABLE		(0 << 15)

/*** Gfx0Width & Gfx1Width ***/
#define	LINK_WIDTH_X1			0x01
#define	LINK_WIDTH_X2			0x02
#define	LINK_WIDTH_X4			0x03
#define	LINK_WIDTH_X8			0x04
#define	LINK_WIDTH_X12			0x05
#define	LINK_WIDTH_X16			0x06

/*** PAYLOAD ***/
#define	PAYLOAD_SIZE_16B		0x02
#define	PAYLOAD_SIZE_32B		0x03
#define	PAYLOAD_SIZE_64B		0x04

/*** GppConfig ***/
#define	GPP_CONFIG_A			0		// 4 : 0 0 0 0
#define	GPP_CONFIG_B			1		// 4 : 4 0 0 0
#define	GPP_CONFIG_C			2		// 4 : 2 2 0 0 
#define	GPP_CONFIG_D			3		// 4 : 2 1 1 0
#define	GPP_CONFIG_E			4		// 4 : 1 1 1 1

/*** Port ***/
/*
 * used for PortEn(2~7), PortDetect(2~7), PortHp(4~7)
 *
 * GFX0 : 2
 * GFX1 : 3
 * GPP0 : 4
 * GPP1 : 5
 * GPP2 : 6
 * GPP3 : 7
 * SB	: 8
 */
#define	PORT_ENABLE		1
#define	PORT_DISABLE	0
#define	PORT(x)				(x)

/*** DBGCONFIG ***/
#define	DBG_CONFIG_BYPASS_SCRAMBLER			(1 << 0)
#define	DBG_CONFIG_LINK_LC_REVERSAL			(1 << 1)
#define	DBG_CONFIG_DYNAMIC_BUF_ALLOC		(1 << 2)
#define	DBG_CONFIG_SB_COMPLIANCE			(1 << 3)
#define	DBG_CONFIG_LONG_RECONFIG			(1 << 4)
#define	DBG_CONFIG_GFX_DEV2_DEV3			(1 << 5)
#define	DBG_CONFIG_NB_SB_VC					(1 << 9)

typedef struct __PCIE_CFG__ {
/* for PCIE PRE INIT */
	u32 Config;
	u8 ResetReleaseDelay;
	u8 Gfx0Width;	// '1' - x1; '2' - x2; '3' - x4; '4' - x8; '6' - x16
	u8 Gfx1Width;
	u8 GfxPayload;	// '0' - 16bytes; '1' - 32bytes; '2' - 64bytes
	u8 GppPayload;
	u8 GppConfig;
	u8 PortEn;		// '2' - GFX0 Port; '3' - GFX1 Port; '4' - GPP0 Port; '5' - GPP1 Port; '6' - GPP2 Port; '7' - GPP3 Port --- disable(0)/enable(1)
	u8 PortDetect;	// same as PortEn, --- Port Empty(0)/Device Attached(1)
	u8 PortHp;		/* hot plug */	// '4' - GPP0; '5' -GPP1; '6' - GPP2; '7' - GPP3 --- not hotplug(0)/hotplug(1)
	u16 DbgConfig;	// 
	u32 DbgConfig2;
	u8	NbRevision;	// returned NbRevision to upperlayer

/* for PCIE POST INIT */
	u8 GfxLx;
	u8 GppLx;
	u8 NBSBLx;
	u8 PortSlotInit;
	u8 Gfx0Pwr;
	u8 Gfx1Pwr;
	u8 GppPwr;
} PCIE_CFG;

/* global variable from rs690_struct.c */
extern PCIE_CFG	ati_pcie_cfg;

/*----------------------------- NB CONFIG ------------------------------------*/

struct ati_integrated_system_info {
	u16	struct_size;				// this struct size
	u8	major_ver;					// major version
	u8	minor_ver;					// minor version
	u32	bootup_engine_clock;		// unit : 10KHz
	u32	bootup_memory_clock;		// unit : 10KHz
	u32	max_system_memory_clock;	// unit : 10KHz
	u32 min_system_memory_clock;	// unit : 10KHz
	u8	num_of_cycles_in_period_hi;	
	u8	reserved1;
	u16	reserved2;
	u16	inter_nb_voltage_low;		// intermidiate PWM value to set the voltage
	u16	inter_nb_voltage_high;		
	u32	reserved3;
	u16	fsb;						// unit : MHz
#define	CAP_FLAG_FAKE_HDMI_SUPPORT		0x01
#define	CAP_FLAG_CLOCK_GATING_ENABLE	0x02

#define	CAP_FLAG_NO_CARD				0x00
#define	CAP_FLAG_AC_CARD				0x04
#define	CAP_FLAG_SDVO					0x08
	u16	cap_flag;

	u16	pcie_nbcfg_reg7;			// NBMISC 0x37 value
	u16 k8_memory_clock;			// k8 memory clock
	u16 k8_sync_start_delay;
	u16 k8_data_return_time;
	u8	max_nb_voltage;
	u8	min_nb_voltage;
	u8	memory_type;				// bits[7:4] = '0001'DDR1 '0010'DDR2 '0011'DDR3

	u8	num_of_cycles_in_period;
	u8	start_pwm_high_time;
	u8	ht_link_width;
	u8	max_nb_voltage_high;
	u8	min_nb_voltage_high;
};

struct vuma_gen_info {
	u16	uma_support_map;
	u16	rec_uma_size;
	u16	mclk_sss;
};

typedef struct __NBCONFIGINFO__ {
#define	GFX_SP_ENABLE		0x01
#define	GFX_UMA_ENABLE		0x02
#define	GFX_TMDS_PHY_MODE	0x08
#define	GFX_ASYNC_MODE		0x10
#define	GFX_CLK_REF_PCIE	0x20
#define	GFX_CLOCK_GATING	0x40
#define	GFX_2D_OPTIMIZATION	0x80
	u8	gfx_config;
#define	EXT_GFX_SVIEW_ENABLE			0x01
#define	EXT_GFX_MULTIFUNC_ENABLE		0x02
#define	EXT_DEBUG_NB_DYNAMIC_CLK		0x04
#define	EXT_DEBUG_GFX32BIT_MODE			0x08
#define	EXT_DEBUG_GFX_MANUAL_MODE		0x10
#define	EXT_AZALIA_ENABLE				0x20
#define	EXT_DEBUG_INTERLEAVE_256BYTE	0x40
#define	EXT_GFX_FORCE_IGFX_ENABLE		0x80
	u8	ext_config;
	u16	uma_fb_size;				// MB based
	u16	sp_fb_size;					// MB based
#define	COARSE_INTERLEAVE	0x00
#define	FINE_INTERLEAVE		0x01
#define	AUTO_INTERLEAVE		0x02
	u8	sp_interleave_mode;
	u16	sp_interleave_size;
	u8	sp_interleave_ratio;
	u32	sp_mc_init_table_ptr;
#define	SP_DBG_DYNAMIC_CKE_ENABLE		0x01
#define	SP_DBG_DYNAMIC_CMD_ENABLE		0x02
#define	SP_DBG_DYNAMIC_CLK_ENABLE		0x04
#define	SP_DBG_DYNAMIC_SELFREF_ENABLE	0x08
#define	SP_DBG_2T_ENABLE				0x10
#define	SP_DBG_DISABLE_NB_TERMINATION	0x20
#define	SP_DBG_DISABLE_ODT				0x40
	u16	sp_dbg_config;
	u8	nb_revision;
//	u8	cpu_revision;
	/* graphic card information : bit0 for pci, bit2~bit7 for pcie & gfx ports */
	u8	pcie_gfx_info;
	u8	ati_gfx_info;

	u16	system_memory_tom_lo;
	u16	system_memory_tom_hi;
	u8	cstate;
	u16	mclk;	// memory clock 1MHz unit
	u16	eclk;	// engine clock 1MHz unit
	struct ati_integrated_system_info	*vuma_rev3;
	struct vuma_gen_info	*vuma_gen;
} NBCONFIGINFO;

/* global variable from rs690_struct.c */
extern NBCONFIGINFO ati_nb_cfg;

/*---------------------------- Export Functions ------------------------------*/

/* from nb_por.c */
// rs690 NB por init routine
void nb_por_init(void);
// rs690 NB revision 
u8 get_nb_revision(void);
// enable BUS0DEV8FUNC0
void nb_enable_dev8(pcitag_t nb_dev);
// disable BUS0DEV8FUNC0
void nb_disable_dev8(pcitag_t nb_dev);
// enable BUS0DEV0FUNC0 bar3
void nb_enable_bar3(pcitag_t nb_dev);
// disable BUS0DEV0FUNC0 bar3
void nb_disable_bar3(pcitag_t nb_dev);

/* from nb_pre.c */
// nb init, spec. for GFX
void nb_pre_init(void);

/* from nb_post.c */
// nb post init
void nb_post_init(void);

/* from pcie_pre.c */
// pcie pre init
void pcie_pre_init(void);

/* from pcie_post.c */
// pcie post init
void pcie_post_init(void);

/* from gfx_pre.c */
// external gfx pre init
void gfx_pre_init(void);
// gfx disable
void gfx_disable(void);

/* from gfx_post.c */
// gfx post init
void gfx_post_init(void);
// gfx pci resouces open
void gfx_pci_init(void);

/* from ht.c */
// rs690 NB and CPU link width/speed training routine
void rs690_ht_init(void);

/* from rs690_dispatch.c */
// PCIE user defined pre init routine
void pcie_init_dispatch(void);
// PCIE user defined pre exit routine
void pcie_exit_dispatch(void);
// pcie check is it dual slot support
u8 pcie_check_dual_cfg_dispatch(void);
// pcie port2 route dispatch
void pcie_port2_route_dispatch(void);
// pcie port3 route dispatch
void pcie_port3_route_dispatch(void);
// pcie port2 reset routine
void pcie_port2_reset_dispatch(void);
// pcie port3 reset routine
void pcie_port3_reset_dispatch(void);
// pcie port2 deassert reset routine
void pcie_deassert_port2_reset_dispatch(void);
// pcie port2 deassert reset routine
void pcie_deassert_port2_reset_dispatch(void);
// pcie signal reset
void pcie_gen_reset_dispatch(void);
// gfx uma state information routine
void gfx_uma_state_dispatch(void);

/*-----------------------------------------------------------------------------*/
#endif /* _RS690_CONFIG_H_ */
