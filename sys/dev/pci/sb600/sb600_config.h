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

/*--------------------------------------------------------------------*/

#ifndef _SB600_CONFIG_H_
#define _SB600_CONFIG_H_

#include <sys/linux/types.h>
#include <dev/pci/pcivar.h>

/*-----------------------------------------------------------------*/

/* handle X86 likely configuration for our platform */
#define	X86_DEBUG
//#undef	X86_DEBUG

/* DEBUG INFO */
#define	SB600_DEBUG
#ifdef	SB600_DEBUG
#ifndef DEBUG_INFO
#define	DEBUG_INFO(fmt, args...)	printf(fmt, ##args)
#endif
#else
#define	DEBUG_INFO
#endif

extern void delay(u32 val);

/*-----------------------------------------------------------------------------------------*/

#define	CFG_ATI_HPET_BASE		0x12300000

#define	CFG_ATI_USB_OC_MAP0		0xffffffff	// default disable
#define	CFG_ATI_USB_OC_MAP1		0x000000ff	// default disable

#define	CFG_UNSUPPORT_AC97
//#undef	CFG_UNSUPPORT_AC97

/* 0-7 to select trap on programIO, 8 for disable the feather */
#ifndef	CFG_P92_TRAP_ON_PIO
#define	CFG_P92_TRAP_ON_PIO		8
#endif

#ifdef	CFG_P92_TRAP_ON_PIO
#if	CFG_P92_TRAP_ON_PIO < 4
#define	P92_TRAP_BASE  		(0x14 + 2 * CFG_P92_TRAP_ON_PIO)
#define	P92_TRAP_EN_REG		0x1c
#define	P92_TRAP_EN_BIT		((1 << 7) >> CFG_P92_TRAP_ON_PIO)
#elif	CFG_P92_TRAP_ON_PIO < 8
#define	P92_TRAP_BASE		(0xA0 + 2 * (CFG_P92_TRAP_ON_PIO - 4))
#define	P92_TRAP_EN_REG		0xA8
#define	P92_TRAP_EN_BIT		((1 << 0) << (CFG_P92_TRAP_ON_PIO - 4))
#endif
#endif

#undef	CFG_SATA1_SUBSYSTEM_ID
#undef	CFG_OHCI0_SUBSYSTEM_ID
#undef	CFG_OHCI1_SUBSYSTEM_ID
#undef	CFG_OHCI2_SUBSYSTEM_ID
#undef	CFG_OHCI3_SUBSYSTEM_ID
#undef	CFG_OHCI4_SUBSYSTEM_ID
#undef	CFG_EHCI_SUBSYSTEM_ID
#undef	CFG_SMBUS_SUBSYSTEM_ID
#undef	CFG_IDE_SUBSYSTEM_ID
#undef	CFG_AZALIA_SUBSYSTEM_ID
#undef	CFG_LPC_SUBSYSTEM_ID
#undef	CFG_AC97_SUBSYSTEM_ID
#undef	CFG_MC97_SUBSYSTEM_ID
#define ATI_SATA2_BUS_DEV_FUN   (((0x11 << 3) + 0) << 8)
#define ATI_SATA1_BUS_DEV_FUN   (((0x12 << 3) + 0) << 8)
#define ATI_OHCI0_BUS_DEV_FUN   (((0x13 << 3) + 0) << 8)
#define ATI_OHCI1_BUS_DEV_FUN   (((0x13 << 3) + 1) << 8)
#define ATI_OHCI2_BUS_DEV_FUN   (((0x13 << 3) + 2) << 8)
#define ATI_OHCI3_BUS_DEV_FUN   (((0x13 << 3) + 3) << 8)
#define ATI_OHCI4_BUS_DEV_FUN   (((0x13 << 3) + 4) << 8)
#define ATI_EHCI_BUS_DEV_FUN    (((0x13 << 3) + 5) << 8)
#define ATI_SMBUS_BUS_DEV_FUN   (((0x14 << 3) + 0) << 8)
#define ATI_IDE_BUS_DEV_FUN		(((0x14 << 3) + 1) << 8)
#define ATI_AZALIA_BUS_DEV_FUN  (((0x14 << 3) + 2) << 8)
#define ATI_LPC_BUS_DEV_FUN     (((0x14 << 3) + 3) << 8)
#define ATI_SBP2P_BUS_DEV_FUN   (((0x14 << 3) + 4) << 8)
#define ATI_AC97_BUS_DEV_FUN    (((0x14 << 3) + 5) << 8)
#define ATI_MC97_BUS_DEV_FUN    (((0x14 << 3) + 6) << 8)

/*-----------------------------------------------------------------------------------------*/

/* SMBUS PROTOCOL REGISTERS */
#define SMBHSTSTAT 0x0
#define SMBSLVSTAT 0x1
#define SMBHSTCTRL 0x2
#define SMBHSTCMD  0x3
#define SMBHSTADDR 0x4
#define SMBHSTDAT0 0x5
#define SMBHSTDAT1 0x6
#define SMBHSTBLKDAT 0x7

#define SMBSLVCTRL 0x8
#define SMBSLVCMD_SHADOW 0x9
#define SMBSLVEVT 0xa
#define SMBSLVDAT 0xc

/* Is it a temporary SMBus I/O base address? yes */
#define SMBUS_IO_BASE 0x1000

/* Between 1-10 seconds, We should never timeout normally 
 * Longer than this is just painful when a timeout condition occurs.
 */
#define SMBUS_TIMEOUT (100*1000*10)

/*-----------------------------------------------------*/

#define	SB_ENABLE	0x01
#define	SB_DISABLE	0x00
#define SB600_DEVICE_ID		0x4385
#define K8_ID	0xB0
#define	P4_ID	0x00
#define ATI_AZALIA_ID 0x43831002
#define	APCI_PMAC_BASE	0x2C
#define	TEMP_SATA_BAR5_BASE		0x12200000

#define	SATA_CLASS_IDE		0x00
#define	SATA_CLASS_RAID		0x01
#define	SATA_CLASS_AHCI		0x02
#define	SATA_CLASS_LEGIDE	0x03
//#define	SATA_CLASS_IDE		0x04
#define	SATA_CLASS_STORAGE	0x05


#define	AZALIA_AUTO			0x00
#define	AZALIA_DISABLE		0x01
#define	AZALIA_ENABLE		0x02
/*-------------------------------------------------------*/

/* ABLINK REGISTER */
#define abcfg_reg(reg, mask, val)	\
	alink_ab_indx((ABCFG), (reg), (mask), (val))
#define axcfg_reg(reg, mask, val)	\
	alink_ab_indx((AXCFG), (reg), (mask), (val))
#define axindxc_reg(reg, mask, val)	\
	alink_ax_indx(AX_INDXC, (reg), (mask), (val))
#define axindxp_reg(reg, mask, val)	\
	alink_ax_indx(AX_INDXP, (reg), (mask), (val))

#define PRINTF_DEBUG() printf(" %s, %d\n", __FUNCTION__, __LINE__)
/*-----------------------------------------------------------------------------*/

struct ati_sb_aza_set_config_d4 {
	u8	nid;
	u32	byte4;
};

struct ati_sb_aza_codec_tbl {
	u32	codec_id;
	struct ati_sb_aza_set_config_d4 *tbl_ptr;
};

struct acpi_base {
	u16	pm1evt;
	u16	pm1ctr;
	u16	pmtmr;
	u16 cpuctr;
	u16	gpe0;
	u16	smicmd;
	u16	pmactl;
	u16	ssctl;
};

struct hwm_fan {
	u8	ctrl; 	//0:Disabled     1:En w/o Temp   2:En w/ Temp0   3:En w/ AMDSI
	u8	mode;	//0:Disabled 1:Enabled
	u8	polarity;	//0:Active low   1:Active High
	u8	freqdiv;
	u8	lowduty;
	u8	medduty;
	u8	multiplier;
	u8	lowtemp;
	u8	lowtemphi;
	u8	medtemp;	
	u8	medtemphi;
	u8	hightemp;	
	u8	hightemphi;
	u8	linearrange;
	u8	linearholdcount;
};

struct ati_sb_cfg {
	u32 acaf;
	u32 pcie_base;
	u32 hpet_base;
	u32 usb_oc_map0;
	u32 usb_oc_map1;
	u8	p92t;
	u8	sata_channel;
	u8	sata_class;
	u8	sata_ck_au_off;
	u8	usb11;
	u8	usb20;
	u8	ac97_audio;
	u8	mc97_modem;
	u8	azalia;
	u8	aza_snoop;
	u8	aza_pin_cfg;
	u8	front_panel;
	u8	aza_sdin0;
	u8	aza_sdin1;
	u8	aza_sdin2;
	u8	aza_sdin3;
	u8	fp_detected;
	u8	sata_smbus;	
	u8	sata_cache;
	u32	sata_cache_base;
	u32	sata_cache_size;
	u8	arb2_en;
	u8	kbrst_smi;
	u8	spd_spec;
	u8	al_clk_delay;
	u8	bl_clk_gate;
	u8	bl_clk_delay;
	u8	al_clk_gate;
	u8	ds_pt;
	u8	usb_xlink;
	u8	usb_oc_wa;
	u8	sata_phy_avdd;
	u8	reserved;
	u8	sata_port_mult_cap;
	u8	reserved1;
	u8	sata_hot_plug_cap;
	u8	sb600_sata_sts;
	u8	hpet_en;
	u8	osc_en;
	u16	clkrun_ctrl;
	u16	clkrun_ctrl_h;
	u16	clkrun_ow;
	u8	tpm_en;
	u8	fan_ctrl;
	struct hwm_fan *fan0;		
	struct hwm_fan *fan1;		
	struct hwm_fan *fan2;		
	u8	pciclk;
	u8	ide_controller;	
};
extern struct ati_sb_cfg ati_sb_cfg;


#ifndef P92_TRAP_ON_PIO
#define	P92_TRAP_ON_PIO 8
#endif

#ifdef P92_TRAP_ON_PIO 
#if	P92_TRAP_ON_PIO < 8
#if P92_TRAP_ON_PIO < 4 
	#define	P92_TRAP_BASE	(0x14 + 2*P92_TRAP_ON_PIO) /*SB_PMIO_REG14 */
	#define P92_TRAP_EN_REG	0x1c	/*SB_PMIO_REG1C */
	#define P92_TRAP_EN_BIT	((1 << 7) >> P92_TRAP_ON_PIO)	/*SB_PMIO_REG1C */
#else
	#define	P92_TRAP_BASE	(0xa0 + 2*(P92_TRAP_ON_PIO-4)) /*SB_PMIO_REGA0 */
	#define P92_TRAP_EN_REG	0xa8	/*SB_PMIO_REGA8 */
	#define P92_TRAP_EN_BIT	(1 << (P92_TRAP_ON_PIO - 4))	/*SB_PMIO_REG1C */
#endif
#endif
#endif

#define EXCLUSIVE_RESOURCE_MODE 0
#define CIM_SB_SMI_32BIT	1

struct ati_sb_smi {
/*
#ifdef CIM_SB_32BIT
		u32	check_routine;
#else
		u16 check_routine;
#endif
*/
		int (*check_routine)(void);	 // may this definition is fit
#ifdef	CIM_SB_SMI_32BIT
		u32	service_routine;
#else
		u16 service_routine;
#endif
		u8	status_register;
		u8	status_mask;
		u8	enable_register;
		u8	enable_mask;
		u8	resource_mode;
};

#define RUNTBL_SIZE 5
/*-------------------------------------------------------------------------------------------*/
/* Functions export */

/* from sb_misc.c */
// get sb600 revision
u8 get_sb600_revision(void);
// get sb600 platfrom type
u8 get_sb600_platform(void);
// software reset 
void soft_reset(void);
// hardware reset
void hard_reset(void);
// fid change
void enable_fid_change_on_sb(u32 sbbusn, u32 sbdn);
// sb pci port80 for debug
void sb600_pci_port80(void);
// sb lpc port80 for debug
void sb600_lpc_port80(void);
// sb lpc init
void sb600_lpc_init(void);
// sb hpet configure
void sb_hpet_cfg(void);

/* from sb_ac97.c */
// sb am97 init last stage
void sb_last_am97_init(void);

/* from sb_azalia.c */
// sb pre azalia cfg
void sb_azalia_cfg(void);
// sb post azalia cfg
void sb_azalia_post_cfg(void);

/* from sb_hwm.c */
// sb hwm cfg
void sb_hwm_cfg(void);

/* from sb_ide.c */
// disable the secondary ide port
void sb_disable_sec_ide(void);

/* from sb_sata.c */
// sb sata pre init
void sb_sata_pre_init(void);
// sb sata init
void sb_sata_init(void);
// sb sata init 
void sb_last_sata_init(void);
// sb sata cache
void sb_last_sata_cache(void);
void sb_sata_channel_reset(void);
void sb_last_sata_unconnected_shutdown_clock(void);

/* from sb_smi.c */
void sb_last_smi_service_check(void);

/* from sb_usb.c */
void sb_usb_post_init(void);

/* from sb_por.c */
void sb_por_init(void);
/* from sb_pre.c */
void sb_pre_init(void);
/* from sb_post.c */
void sb_post_init(void);
/* from sb_last.c */
void sb_last_init(void);

/*--------------------------------------------------------------------------------------------*/

#endif /* _SB600_CONFIG_H_ */
