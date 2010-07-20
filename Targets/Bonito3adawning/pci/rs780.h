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

#ifndef __RS780_H__
#define __RS780_H__

#include <time.h>
#include "sb700.h"

#define NBMISC_INDEX 	0x60
#define NBHTIU_INDEX 	0xA8
#define NBMC_INDEX 		0xE8
#define NBPCIE_INDEX  	0xE0
#define EXT_CONF_BASE_ADDRESS 0x17000000
#define	TEMP_MMIO_BASE_ADDRESS	0xC0000000

typedef struct __PCIE_CFG__ {
	u16 Config;
	u8 ResetReleaseDelay;
	u8 Gfx0Width;
	u8 Gfx1Width;
	u8 GfxPayload;
	u8 GppPayload;
	u8 PortDetect;
	u8 PortHp;		/* hot plug */
	u16 DbgConfig;
	u32 DbgConfig2;
	u8 GfxLx;
	u8 GppLx;
	u8 NBSBLx;
	u8 PortSlotInit;
	u8 Gfx0Pwr;
	u8 Gfx1Pwr;
	u8 GppPwr;
} PCIE_CFG;

/* PCIE config flags */
#define	PCIE_DUALSLOT_CONFIG			(1 << 0)
#define	PCIE_OVERCLOCK_ENABLE		(1 << 1)
#define	PCIE_GPP_CLK_GATING			(1 << 2)
#define	PCIE_ENABLE_STATIC_DEV_REMAP	(1 << 3)
#define	PCIE_OFF_UNUSED_GFX_LANES		(1 << 4)
#define	PCIE_OFF_UNUSED_GPP_LANES		(1 << 5)
#define	PCIE_DISABLE_HIDE_UNUSED_PORTS	(1 << 7)
#define	PCIE_GFX_CLK_GATING			(1 << 11)
#define	PCIE_GFX_COMPLIANCE			(1 << 14)
#define	PCIE_GPP_COMPLIANCE			(1 << 15)

typedef enum _NB_REVISION_ {
	REV_RS780_A11 = 5,
	REV_RS780_A12 = 6,
	REV_RS780_A21 = 7,
} NB_REVISION;


#define PCI_ADDR(BUS, DEV, FN, WHERE) ( \
	(((BUS) & 0xFF) << 16) | \
	(((DEV) & 0x1f) << 11) | \
	(((FN) & 0x07) << 8) | \
	((WHERE) & 0xFF))

#define PCI_DEV(BUS, DEV, FN) ( \
	(((BUS) & 0xFF) << 16) | \
	(((DEV) & 0x1f) << 11) | \
	(((FN)  & 0x7) << 8)) 

/* -------------------- ----------------------
* NBMISCIND
 ------------------- -----------------------*/
#define	PCIE_LINK_CFG			0x8
#define	PCIE_NBCFG_REG7		0x37
#define	STRAPS_OUTPUT_MUX_7		0x67
#define	STRAPS_OUTPUT_MUX_A		0x6a

/* -------------------- ----------------------
* PCIEIND
 ------------------- -----------------------*/
#define	PCIE_CI_CNTL			0x20
#define	PCIE_LC_LINK_WIDTH		0xa2
#define   PCIE_LC_STATE0			0xa5
//#define	PCIE_VC0_RESOURCE_STATUS	0x11a	/* 16bit read only */
#define	PCIE_VC0_RESOURCE_STATUS	0x12a	/* 16bit read only */

#define	PCIE_CORE_INDEX_GFX			(0 << 16) /* see 5.2.2 */
#define	PCIE_CORE_INDEX_GPPSB		(1 << 16)
//add for rs780
#define	PCIE_CORE_INDEX_GPP		(0x02 << 16)
#define	PCIE_CORE_INDEX_BRDCST		(0x03 << 16)

/* contents of PCIE_NBCFG_REG7 */
#define   RECONFIG_GPPSB_EN			(1 << 12)
#define	RECONFIG_GPPSB_GPPSB			(1 << 14)
#define   RECONFIG_GPPSB_LINK_CONFIG		(1 << 15)
#define	RECONFIG_GPPSB_ATOMIC_RESET		(1 << 17)

/* contents of PCIE_VC0_RESOURCE_STATUS */
#define	VC_NEGOTIATION_PENDING		(1 << 1)

#define	LC_STATE_RECONFIG_GPPSB		0x10



/* ------------------------------------------------
* Global variable
* ------------------------------------------------- */
extern PCIE_CFG AtiPcieCfg;


/* ----------------- export funtions ----------------- */
u32 nbmisc_read_index(device_t nb_dev, u32 index);
void nbmisc_write_index(device_t nb_dev, u32 index, u32 data);
u32 nbpcie_p_read_index(device_t dev, u32 index);
void nbpcie_p_write_index(device_t dev, u32 index, u32 data);
u32 nbpcie_ind_read_index(device_t nb_dev, u32 index);
void nbpcie_ind_write_index(device_t nb_dev, u32 index, u32 data);
u32 htiu_read_index(device_t nb_dev, u32 index);
void htiu_write_index(device_t nb_dev, u32 index, u32 data);
u32 nbmc_read_index(device_t nb_dev, u32 index);
void nbmc_write_index(device_t nb_dev, u32 index, u32 data);

#if 0
u32 pci_ext_read_config32(device_t nb_dev, device_t dev, u32 reg);
void pci_ext_write_config32(device_t nb_dev, device_t dev, u32 reg, u32 mask, u32 val);
#endif 

u16 pci_ext_read_config16(device_t nb_dev, device_t dev, u32 reg);
void pci_ext_write_config16(device_t nb_dev, device_t dev, u32 reg, u32 mask, u32 val);

void set_nbcfg_enable_bits(device_t nb_dev, u32 reg_pos, u32 mask, u32 val);
void set_nbcfg_enable_bits_8(device_t nb_dev, u32 reg_pos, u8 mask, u8 val);
void set_nbmc_enable_bits(device_t nb_dev, u32 reg_pos, u32 mask, u32 val);
void set_htiu_enable_bits(device_t nb_dev, u32 reg_pos, u32 mask, u32 val);
void set_nbmisc_enable_bits(device_t nb_dev, u32 reg_pos, u32 mask, u32 val);
void set_pcie_enable_bits(device_t dev, u32 reg_pos, u32 mask, u32 val);
void rs780_set_tom(device_t nb_dev);

void ProgK8TempMmioBase(u8 in_out, u32 pcie_base_add, u32 mmio_base_add);
void enable_pcie_bar3(device_t nb_dev);
void disable_pcie_bar3(device_t nb_dev);

void rs780_enable(device_t dev);
void rs780_gpp_sb_init(device_t nb_dev, device_t dev, u32 port);
void rs780_gfx_init(device_t nb_dev, device_t dev, u32 port);
void avoid_lpc_dma_deadlock(device_t nb_dev, device_t sb_dev);
void config_gpp_core(device_t nb_dev, device_t sb_dev);
void PcieReleasePortTraining(device_t nb_dev, device_t dev, u32 port);
u8 PcieTrainPort(device_t nb_dev, device_t dev, u32 port);
#endif				/* RS780_H */


