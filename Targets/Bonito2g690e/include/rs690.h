/*
 * RS690 registers definition
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

#ifndef	_RS690_H_
#define	_RS690_H_
/*----------------------------------EXT NBCONFIG----------------------------------------*/

#define	NB_ECC_CTRL		0x48	
#define	NB_PCI_CTRL		0x4C			// BAR & FUNCTION ENABLE
#define	NB_ADAPTER_ID_W		0x50
#define	NB_SCRATCH_1		0x54
#define	NB_FDHC		0x68
#define	NB_SMRAM	0x69
#define	NB_EXSMRAM	0x6A
#define	NB_PMCR		0x6B
#define	NB_STRAP_READ_BACK	0x6C
#define	NB_SCRATCH		0x78
#define	NB_IOC_CFG_CNTL		0x7C			// BAR3 wr enable & disable INTGFX
#define	NB_HT_CLK_CNTL_RECEIVER_COMP_CNTL	0x80
#define	NB_PCI_ARB		0x84			// ENABLE SOME FUNCTIONS
#define	NB_GC_STRAPS		0x8C
#define	NB_TOP_OF_DRAM_SLOT1	0x90
#define	NB_HT_TRANS_COMP_CNTL	0x94
#define	NB_SCRATCH_2		0x98
#define	NB_CFG_Q_F1000_800	0x9C
#define	NB_AGP_ADDRESS_SPACE_SIZE	0xF8		// GART SETTING
#define	NB_AGP_MODE_CNTL	0xFC

/*----------------------------------NBMISCIND--------------------------------------------*/
// ALL ARE 32BITS
#define	NB_MISC_INDEX		0x60
#define	NB_MISC_DATA		0x64

#define	NB_CNTL				0x00		// space & function enable
#define	NB_IOC_DEBUG			0x01
#define	NB_SPARE1			0x02
#define	NB_STRAPS_READBACK_MUX		0x03
#define	NB_STRAPS_READBACK_DATA		0x04
#define	DFT_CNTL0			0x05
#define	DFT_CNTL1			0x06
#define	PCIE_PDNB_CNTL			0x07		// power down or gate clock 
#define	PCIE_LINK_CFG			0x08
#define	IOC_DMA_ARBITER			0x09
#define	IOC_PCIE_CSR_COUNT		0x0A
#define	IOC_PCIE_CNTL			0x0B
#define	IOC_P2P_CNTL			0x0C		// HIDE each device
#define	IOC_ISOC_MAP_ADDR_LO		0x0E
#define	IOC_ISOC_MAP_ADDR_HI		0x0F
#define	DFT_CNTL2			0x10
#define	NB_TOM_PCI			0x16		// TOM
#define	NB_MMIOBASE			0x17		// MMIO BASE
#define	NB_MMIOLIMIT			0x18		// MMIO LIMIT
#define	NB_INTERRUPT_PIN		0x1F		// AGP INT
#define	NB_PROG_DEVICE_REMAP_0		0x20
#define	IOC_LAT_PERF_CNTR_CNTL		0x30
#define	IOC_LAT_PERF_CNTR_OUT		0x31
#define	PCIE_NBCFG_REG0			0x32
#define	PCIE_NBCFG_REG3			0x33
#define	PCIE_NBCFG_REG4			0x34
#define	PCIE_NBCFG_REG5			0x35
#define	PCIE_NBCFG_REG6			0x36
#define	PCIE_NBCFG_REG7			0x37
#define		RECONFIG_GPPSB_EN		(1 << 12)
#define		RECONFIG_GPPSB_GPPSB		(1 << 14)
#define		RECONFIG_GPPSB_LINK_CONFIG	(1 << 15)
#define		RECONFIG_GPPSB_ATOMIC_RESET	(1 << 17)
#define	PCIE_NBCFG_REG8			0x38
#define	PCIE_STRAP_REG2			0x39
#define	NB_BROADCAST_BASE_LO		0x3A
#define	NB_BROADCAST_BASE_HI		0x3B
#define	NB_BROADCAST_CNTL		0x3C
#define	NB_APIC_P2P_CNTL		0x3D		// APIC
#define	NB_APIC_P2P_RANGE_0		0x3E
#define	NB_APIC_P2P_RANGE_1		0x3F
#define	GPIO_PAD			0x40
#define	GPIO_PAD_CNTL_PU_PD		0x41
#define	GPIO_PAD_SCHMEM_OE		0x42
#define	GPIO_PAD_SP_SN			0x43
#define	DFT_VIP_IO_GPIO			0x44
#define	DFT_VIP_IO_GPIO_OR		0x45
#define	IOC_PCIE_D2_CSR_COUNT		0x50		// EACH device IO feature control
#define	IOC_PCIE_D2_CNTL		0x51
#define	IOC_PCIE_D3_CSR_COUNT		0x52
#define	IOC_PCIE_D3_CNTL		0x53
#define	IOC_PCIE_D4_CSR_COUNT		0x54
#define	IOC_PCIE_D4_CNTL		0x55
#define	IOC_PCIE_D5_CSR_COUNT		0x56
#define	IOC_PCIE_D5_CNTL		0x57
#define	IOC_PCIE_D6_CSR_COUNT		0x58
#define	IOC_PCIE_D6_CNTL		0x59
#define	IOC_PCIE_D7_CSR_COUNT		0x5A
#define	IOC_PCIE_D7_CNTL		0x5B		// IOC_PCIEx
#define	STRAPS_OUTPUT_MUX0		0x60
#define	STRAPS_OUTPUT_MUX1		0x61
#define	STRAPS_OUTPUT_MUX2		0x62
#define	STRAPS_OUTPUT_MUX3		0x63
#define	STRAPS_OUTPUT_MUX4		0x64
#define	STRAPS_OUTPUT_MUX5		0x65
#define	STRAPS_OUTPUT_MUX6		0x66
#define	STRAPS_OUTPUT_MUX7		0x67
#define	STRAPS_OUTPUT_MUX8		0x68
#define	STRAPS_OUTPUT_MUX9		0x69
#define	STRAPS_OUTPUT_MUXA		0x6A
#define	STRAPS_OUTPUT_MUXB		0x6B
#define	STRAPS_OUTPUT_MUXC		0x6C
#define	STRAPS_OUTPUT_MUXD		0x6D
#define	STRAPS_OUTPUT_MUXE		0x6E
#define	STRAPS_OUTPUT_MUXF		0x6F
#define	SCRATCH_0			0x70
#define	SCRATCH_1			0x71
#define	SCRATCH_2			0x72
#define	SCRATCH_3			0x73
#define	SCRATCH_4			0x74
#define	SCRATCH_5			0x75
#define	SCRATCH_6			0x76
#define	SCRATCH_7			0x77
#define	SCRATCH_8			0x78
#define	SCRATCH_9			0x79
#define	SCRATCH_A			0x7A
#define	DFT_SPARE			0x7F

/*------------------------------------NBHTIUIND-------------------------------------------*/
// ALL ARE 32BITS
#define	HTIU_NB_INDEX		0xA8
#define	HTIU_NB_DATA		0xAC

#define	HTIU_CNTL_1			0x00
#define	HTIU_CNTL_2			0x01
#define	HTIU_PERF_CNTL			0x02
#define	HTIU_PERF_COUNT_0		0x03
#define	HTIU_PERF_COUNT_1		0x04
#define	HTIU_DEBUG			0x05
#define	HTIU_DOWNSTREAM_CONFIG		0x06
#define	HTIU_UPSTREAM_CONFIG_0		0x07
#define	HTIU_UPSTREAM_CONFIG_1		0x08
#define	HTIU_UPSTREAM_CONFIG_2		0x09
#define	HTIU_UPSTREAM_CONFIG_3		0x0A
#define	HTIU_UPSTREAM_CONFIG_4		0x0B
#define	HTIU_UPSTREAM_CONFIG_5		0x0C
#define	HTIU_UPSTREAM_CONFIG_6		0x0D
#define	HTIU_UPSTREAM_CONFIG_7		0x0E
#define	HTIU_UPSTREAM_CONFIG_8		0x0F
#define	HTIU_UPSTREAM_CONFIG_9		0x10
#define	HTIU_UPSTREAM_CONFIG_10		0x11
#define	HTIU_UPSTREAM_CONFIG_11		0x12
#define	HTIU_UPSTREAM_CONFIG_12		0x13
#define	HTIU_UPSTREAM_CONFIG_13		0x14
#define	HTIU_UPSTREAM_CONFIG_14		0x15
#define	HTIU_UPSTREAM_CONFIG_15		0x16
#define	HTIU_UPSTREAM_CONFIG_16		0x17
#define	HTIU_UPSTREAM_CONFIG_17		0x18
#define	HTIU_UPSTREAM_CONFIG_18		0x19
#define	HTIU_UPSTREAM_CONFIG_19		0x1A
#define	HTIU_UPSTREAM_CONFIG_20		0x1B
#define	HTIU_UPSTREAM_CONFIG_21		0x1C
#define	HTIU_UPSTREAM_CONFIG_22		0x1D
#define	HTIU_UPSTREAM_CONFIG_23		0x1E
#define	HTIU_UPSTREAM_CONFIG_24		0x1F
#define	NB_LOWER_TOP_OF_DRAM2		0x30			// TOM2
#define	NB_UPPER_TOP_OF_DRAM2		0x31			// TOM2
#define	NB_HTIU_CFG			0x32			// BAR3

/*------------------------------------NBPCIEIND--------------------------------------------*/
// ALL ARE 32BITS
#define	NB_PCIE_INDX_ADDR	0xE0
#define	NB_PCIE_INDX_DATA	0xE4
#define		PCIE_CORE_INDEX_GFX			(0 << 16)
#define		PCIE_CORE_INDEX_GPPSB		(1 << 16)	// used for access SB

#define	PCIE_RESERVED			0x00
#define	PCIE_SCRATCH			0x01
#define	PCIE_CNTL			0x10
#define	PCIE_CONFIG_CNTL		0x11
#define	PCIE_DEBUG_CNTL			0x12
#define	PCIE_CI_CNTL			0x20
#define	PCIE_BUS_CNTL			0x21
#define	PCIE_WPR_CNTL			0x30
#define	PCIE_P_CNTL			0x40
#define	PCIE_P_BUF_STATUS		0x41
#define	PCIE_P_DECODER_STATUS		0x42
#define	PCIE_P_PLL_CNTL			0x44			// HW voltage
#define	PCIE_P_IMP_CNTL_STRENGTH	0x60
#define	PCIE_P_IMP_CNTL_UPDATE		0x61
#define	PCIE_P_STR_CNTL_UPDATE		0x62
#define	PCIE_P_PAD_MISC_CNTL		0x63
#define	PCIE_P_PAD_FORCE_EN		0x64
#define	PCIE_P_PAD_FORCE_DIS		0x65
#define	PCIE_PERF_LATENCY_CNTL		0x70
#define	PCIE_PERF_LATENCY_REQ_ID	0x71
#define	PCIE_PERF_LATENCY_TAG		0x72
#define	PCIE_PERF_LATENCY_THRESHOLD	0x73
#define	PCIE_PERF_LATENCY_MAX		0x74
#define	PCIE_PERF_LATENCY_TIMER_LO	0x75
#define	PCIE_PERF_LATENCY_TIMER_HI	0x76
#define	PCIE_PERF_LATENCY_COUNTER0	0x77
#define	PCIE_PERF_LATENCY_COUNTER1	0x78
#define	PCIE_PRBS23_BITCNT_0		0xD0
#define	PCIE_PRBS23_BITCNT_1		0xD1
#define	PCIE_PRBS23_BITCNT_2		0xD2
#define	PCIE_PRBS23_BITCNT_3		0xD3
#define	PCIE_PRBS23_BITCNT_4		0xD4
#define	PCIE_PRBS23_BITCNT_5		0xD5
#define	PCIE_PRBS23_BITCNT_6		0xD6
#define	PCIE_PRBS23_BITCNT_7		0xD7
#define	PCIE_PRBS23_BITCNT_8		0xD8
#define	PCIE_PRBS23_BITCNT_9		0xD9
#define	PCIE_PRBS23_BITCNT_10		0xDA
#define	PCIE_PRBS23_BITCNT_11		0xDB
#define	PCIE_PRBS23_BITCNT_12		0xDC
#define	PCIE_PRBS23_BITCNT_13		0xDD
#define	PCIE_PRBS23_BITCNT_14		0xDE
#define	PCIE_PRBS23_BITCNT_15		0xDF
#define	PCIE_PRBS23_ERRCNT_0		0xE0
#define	PCIE_PRBS23_ERRCNT_1		0xE1
#define	PCIE_PRBS23_ERRCNT_2		0xE2
#define	PCIE_PRBS23_ERRCNT_3		0xE3
#define	PCIE_PRBS23_ERRCNT_4		0xE4
#define	PCIE_PRBS23_ERRCNT_5		0xE5
#define	PCIE_PRBS23_ERRCNT_6		0xE6
#define	PCIE_PRBS23_ERRCNT_7		0xE7
#define	PCIE_PRBS23_ERRCNT_8		0xE8
#define	PCIE_PRBS23_ERRCNT_9		0xE9
#define	PCIE_PRBS23_ERRCNT_10		0xEA
#define	PCIE_PRBS23_ERRCNT_11		0xEB
#define	PCIE_PRBS23_ERRCNT_12		0xEC
#define	PCIE_PRBS23_ERRCNT_13		0xED
#define	PCIE_PRBS23_ERRCNT_14		0xEE
#define	PCIE_PRBS23_ERRCNT_15		0xEF
#define	PCIE_PRBS23_CLR			0xF0
#define	PCIE_PRBS23_ERRSTAT		0xF1
#define	PCIE_PRBS23_LOCKED		0xF2
#define	PCIE_PRBS23_FREERUN		0xF3
#define	PCIE_PRBS23_LOCK_CNT		0xF4
#define	PCIE_PRBS23_EN			0xF5
#define	PCIE_P90RX_PRBS_CLR		0xF6
#define	PCIE_P90_BRX_PRBS_ER		0xF7
#define	PCIE_P90TX_PRBS_EN		0xF8
#define	PCIE_B_P90_CNTL			0xF9
//////////////////////////////////EXTENEDED
#define	PCIE_VC0_RESOURCE_STATUS		0x11a	// 16bits
#define		VC_NEGOTIATION_PENDING	(1 << 1)
#define		LC_STATE_RECONFIG_GPPSB	(1 << 4)

/*------------------------------------PCIEPIND---------------------------------------------*/
// ALL ARE 32BITS WIDTH
#define	PCIE_PORT_INDEX		0xE0
#define	PCIE_PORT_DATA		0xE4

#define	PCIEP_RESERVER			0x00
#define	PCIEP_SCRATCH			0x01
#define	PCIEP_PORT_CNTL			0x10			// PORT control
#define	PCIE_TX_CNTL			0x20			// TX regulator
#define	PCIE_TX_REQUESTER_ID		0x21
#define	PCIE_TX_VENDOR_SPECIFIC		0x22
#define	PCIE_TX_REQUEST_NUM_CNTL	0x23
#define	PCIE_TX_SEQ			0x24
#define	PCIE_TX_REPLAY			0x25
#define	PCIE_TX_ACK_LATENCY_LIMIT	0x26
#define	PCIE_TX_CREDITS_CONSUMED_P	0x30
#define	PCIE_TX_CREDITS_CONSUMED_NP	0x31
#define	PCIE_TX_CREDITS_CONSUMED_CPL	0x32
#define	PCIE_TX_CREDITS_LIMIT_P		0x33
#define	PCIE_TX_CREDITS_LIMIT_NP	0x34
#define	PCIE_TX_CREDITS_LIMIT_CPL	0x35
#define	PCIE_P_PORT_LANE_STATUS		0x50
#define	PCIE_FC_P			0x60
#define	PCIE_FC_NP			0x61
#define	PCIE_FC_CPL			0x62
#define	PCIE_ERR_CNTL			0x6A
#define	PCIE_RX_CNTL			0x70
#define	PCIE_RX_VENDOR_SPECIFIC		0x72
#define	PCIE_RX_CREDITS_ALLOCATED_P	0x80
#define	PCIE_RX_CREDITS_ALLOCATED_NP	0x81
#define	PCIE_RX_CREDITS_ALLOCATED_CPL	0x82
#define	PCIE_RX_CREDITS_RECEIVED_P	0x83
#define	PCIE_RX_CREDITS_RECEIVED_NP	0x84
#define	PCIE_RX_CREDITS_RECEIVED_CPL	0x85
//#define	PCIE_RX_LASTACK_SEQNUM		0x84
#define	PCIE_LC_CNTL			0xA0			// LINK CONTROL
#define	PCIE_LC_TRAINING_CNTL		0xA1
#define	PCIE_LC_LINK_WIDTH_CNTL		0xA2
#define	PCIE_LC_N_FTS_CNTL		0xA3
#define	PCIE_LC_STATE0			0xA5
#define	PCIE_LC_STATE1			0xA6
#define	PCIE_LC_STATE2			0xA7
#define	PCIE_LC_STATE3			0xA8
#define	PCIE_LC_STATE4			0xA9
#define	PCIE_LC_STATE5			0xAA

/*-----------------------------------NBMCINDIND------------------------------------------*/
#define	NB_MC_IND_INDEX		0x70
#define	NB_MC_IND_DATA		0x74

/*------------------------------------NBMCIND----------------------------------------------*/
// ALL ARE 32BITS
#define	NB_MC_INDEX		0xE8
#define	NB_MC_DATA		0xEC

#define	MC_GENERAL_PURPOSE		0x00			// INIT bit
#define	MC_MISC_CNTL			0x18			// GART & MCIND switch
#define	NB_MEM_CH_CNTL2			0x1B
#define	NB_MEM_CH_CNTL0			0x1C			// MEMORY CHANNEL CONFIG WITHOUT INTERLEAVE
#define	NB_MEM_CH_CNTL1			0x1D
#define	K8_FB_LOCATION			0x1E			// FB start in system memory ***
#define	NB_MC_DEBUG			0x1F
#define	MC_PM_CNTL			0x26
#define	GART_FEATURE_ID			0x2B			// GART CONFIG
#define	GART_BASE			0x2C			// GART BASE ADDRESS
#define	GART_CACHE_SZBASE		0x2D			// GART CACHE & TLB FLUSH OPER
#define	GART_CACHE_CNTRL		0x2E			// 
#define	GART_CACHE_ENTRY_CNTRL		0x2F			// 
#define	GART_ERROR_0			0x30
#define	GART_ERROR_1			0x31
#define	GART_ERROR_2			0x32
#define	GART_ERROR_3			0x33
#define	GART_ERROR_4			0x34
#define	GART_ERROR_5			0x35
#define	GART_ERROR_6			0x36
#define	GART_ERROR_7			0x37
#define	AGP_ADDRESS_SPACE_SIZE		0x38			// AGP SIZE
#define	AGP_MODE_CONTROL		0x39
#define	AIC_CTRL_SCRATCH		0x3A
#define	MC_GART_ERROR_ADDRESS		0x3B
#define	MC_GART_ERROR_ADDRESS_HI	0x3C
#define	MC_MISC_CNTL2			0x4E
#define	MC_MISC_CNTL3			0x4F
#define	MC_MISC_UMA_CNTL		0x5F			// UMA REQUEST setting & sideport present
#define	K8_DRAM_CS0_BASE		0x63			// WE SHOULD DEFINE ONE CS0 IF THE MIRROR FUNCTION IS
#define	K8_DRAM_CS1_BASE		0x64			// USED FOR SYSTEM MEMORY
#define	K8_DRAM_CS2_BASE		0x65
#define	K8_DRAM_CS3_BASE		0x66
#define	K8_DRAM_CS4_BASE		0x67
#define	K8_DRAM_CS5_BASE		0x68
#define	K8_DRAM_CS6_BASE		0x69
#define	K8_DRAM_CS7_BASE		0x6A
#define	K8_DRAM_CS0_MASK		0x6B
#define	K8_DRAM_CS1_MASK		0x6C
#define	K8_DRAM_CS2_MASK		0x6D
#define	K8_DRAM_CS3_MASK		0x6E
#define	K8_DRAM_CS4_MASK		0x6F
#define	K8_DRAM_CS5_MASK		0x70
#define	K8_DRAM_CS6_MASK		0x71
#define	K8_DRAM_CS7_MASK		0x72
#define	K8_DRAM_BANK_ADDR_MAPPING	0x73			// for CSx
#define	MC_MPLL_CONTROL			0x74
#define	MC_MPLL_CONTROL2		0x75
#define	MC_MPLL_CONTROL3		0x76
#define	MC_MPLL_FREQ_CONTROL		0x77
#define	MC_MPLL_SEQ_CONTROL		0x78
#define	MC_MPLL_DIV_CONTROL		0x79
#define	MC_MCLK_CONTROL			0x7A
#define	MC_UMA_WC_GRP_TMR		0x80
#define	MC_UMA_WC_GRP_CNTL		0x81
#define	MC_UMA_RW_GRP_TMR		0x82
#define	MC_UMA_RW_G3DR_GRP_CNTL		0x83
#define	MC_UMA_RW_TXR_E2R_GRP_CNTL	0x84
#define	MC_UMA_AGP_GRP_CNTL		0x85
#define	MC_UMA_DUALCH_CNTL		0x86
#define	MC_SYSTEM_STATUS		0x90
#define	MC_INTFC_GENERAL_PURPOSE	0x91			// MC control & start
#define	MC_INTFC_IMP_CTRL_CNTL		0x92
#define	MC_INTFC_IMP_CTRL_REF		0x93
#define	MC_LATENCY_COUNT_CNTL		0x94
#define	MC_LATENCY_COUNT_EVENT		0x95
#define	MCS_PERF_COUNT0			0x96
#define	MCS_PERF_COUNT1			0x97
#define	MCS_PERF_CNTL			0x98
#define	MC_AZ_DEFAULT_ADDR		0x99
#define	MCA_MEMORY_INIT_MRS		0xA0			// MCA memory init sequences ::: MAYBE ALL THE MCA ARE JUST
#define	MCA_MEMORY_INIT_EMRS		0xA1			// USED FOR SIDE-PORT MODE
#define	MCA_MEMORY_INIT_EMRS2		0xA2
#define	MCA_MEMORY_INIT_EMRS3		0xA3
#define	MCA_MEMORY_INIT_SEQUENCE_1	0xA4
#define	MCA_MEMORY_INIT_SEQUENCE_2	0xA5
#define	MCA_MEMORY_INIT_SEQUENCE_3	0xA6
#define	MCA_MEMORY_INIT_SEQUENCE_4	0xA7			// for MCA memory init
#define	MCA_TIMING_PARAMETERS_1		0xA8
#define	MCA_TIMING_PARAMETERS_2		0xA9
#define	MCA_TIMING_PARAMETERS_3		0xAA
#define	MCA_TIMING_PARAMETERS_4		0xAB
#define	MCA_MEMORY_TYPE			0xAC
#define	MCA_CKE_MUX_SELECT		0xAD
#define	MCA_ODT_MUX_SELECT		0xAE
#define	MCA_SEQ_PERF_CNTL		0xAF
#define	MCA_SEQ_CONTROL			0xB0
#define	MCA_RECEIVING			0xB1
#define	MCA_IN_TIMING_DQS_3210		0xB2
#define	MCA_IN_TIMING_DQS_7654		0xB3
#define	MCA_DRIVING			0xB4
#define	MCA_OUT_TIMING			0xB5
#define	MCA_OUT_TIMING_DQ		0xB6
#define	MCA_OUT_TIMING_DQS		0xB7
#define	MCA_STRENGTH_N			0xB8
#define	MCA_STRENGTH_P			0xB9
#define	MCA_STRENGTH_STEP		0xBA
#define	MCA_STRENGTH_READ_BACK_N	0xBB
#define	MCA_STRENGTH_READ_BACK_P	0xBC
#define	MCA_PREAMP			0xBD
#define	MCA_PREAMP_N			0xBE
#define	MCA_PREAMP_P			0xBF
#define	MCA_PREAMP_STEP			0xC0
#define	MCA_PREBUF_SLEW_N		0xC1
#define	MCA_PREBUF_SLEW_P		0xC2
#define	MCA_GENERAL_PURPOSE		0xC3
#define	MCA_GENERAL_PURPOSE_2		0xC4
#define	MCA_OCD_CONTROL			0xC5
#define	MCA_DQ_DQS_READ_BACK		0xC6
#define	MCA_DQS_CLK_READ_BACK		0xC7
#define	MCA_MEMORY_INIT_MRS_PM		0xC8
#define	MCA_MEMORY_INIT_EMRS_PM		0xC9
#define	MCA_MEMORY_INIT_EMRS2_PM	0xCA
#define	MCA_MEMORY_INIT_EMRS3_PM	0xCB
#define	MCA_TIMING_PARAMETERS_1_PM	0xCC
#define	MCA_TIMING_PARAMETERS_2_PM	0xCD
#define	MCA_TIMING_PARAMETERS_3_PM	0xCE
#define	MCA_TIMING_PARAMETERS_4_PM	0xCF
#define	MCA_IN_TIMING_DQS_3210_PM	0xD0
#define	MCA_IN_TIMING_DQS_7654_PM	0xD1
#define	MCA_OUT_TIMING_DQ_PM		0xD2
#define	MCA_OUT_TIMING_DQS_PM		0xD3
#define	MCA_MISCELLANEOUS		0xD4
#define	MCA_MISCELLANEOUS_2		0xD5
#define	MCA_MX1X2X_DQ			0xD6
#define	MCA_MX1X2X_DQS			0xD7
#define	MCA_DLL_MASTER_0		0xD8
#define	MCA_DLL_MASTER_1		0xD9
#define	MCA_DLL_SLAVE_RD_0		0xE0
#define	MCA_DLL_SLAVE_RD_1		0xE1
#define	MCA_DLL_SLAVE_WR_0		0xE8
#define	MCA_DLL_SLAVE_WR_1		0xE9
#define	MCA_RESERVED_0			0xF0
#define	MCA_RESERVED_1			0xF1
#define	MCA_RESERVED_2			0xF2
#define	MCA_RESERVED_3			0xF3
#define	MCA_RESERVED_4			0xF4
#define	MCA_RESERVED_5			0xF5
#define	MCA_RESERVED_6			0xF6
#define	MCA_RESERVED_7			0xF7
#define	MCA_RESERVED_8			0xF8
#define	MCA_RESERVED_9			0xF9
#define	MCA_RESERVED_A			0xFA
#define	MCA_RESERVED_B			0xFB
#define	MCA_RESERVED_C			0xFC
#define	MCA_RESERVED_D			0xFD
#define	MCA_RESERVED_E			0xFE
#define	MCA_RESERVED_F			0xFF
#define	MCCFG_FB_LOCATION		0x100			// FB START OF INTERNAL ADDR
#define	MCCFG_AGP_LOCATION		0x101			// AGP START OF INTERNAL ADDR
#define	MCCFG_AGP_BASE			0x102			// AGP BASE FOR SYSTEM MEMORY
#define	MCCFG_AGP_BASE_2		0x103			// EXT-TO 40BITS
#define	MC_INIT_MISC_LAT_TIMER		0x104
#define	MC_INIT_GFX_LAT_TIMER		0x105
#define	MC_INIT_WR_LAT_TIMER		0x106
#define	MC_ARB_CNTL			0x107			// GFX ARBITOR
#define	MC_DEBUG_CNTL			0x108
#define	MC_BIST_CNTL0			0x111
#define	MC_BIST_CNTL1			0x112
#define	MC_BIST_MISMATCH_L		0x113
#define	MC_BIST_MISMATCH_H		0x114
#define	MC_BIST_PATTERNOL		0x115
#define	MC_BIST_PATTERN0H		0x116
#define	MC_BIST_PATTERN1L		0x117
#define	MC_BIST_PATTERN1H		0x118
#define	MC_BIST_PATTERN2L		0x119
#define	MC_BIST_PATTERN2H		0x11A
#define	MC_BIST_PATTERN3L		0x11B
#define	MC_BIST_PATTERN3H		0x11C
#define	MC_BIST_PATTERN4L		0x11D
#define	MC_BIST_PATTERN4H		0x11E
#define	MC_BIST_PATTERN5L		0x11F
#define	MC_BIST_PATTERN5H		0x120
#define	MC_BIST_PATTERN6L		0x121
#define	MC_BIST_PATTERN6H		0x122
#define	MC_BIST_PATTERN7L		0x123
#define	MC_BIST_PATTERN7H		0x124

/*----------------------------------------------BUS0DEV0FUNC1 : SYSTEM CLOCK CSR-----------------------------*/
// THIS FUNC IS WITHOUT STD PCI HEADER
#define	OSC_CONTROL			0x40
#define	CPLL_CONTROL			0x44
#define	CLK_TOP_PWM4_CTRL		0x4C
#define	CLK_TOP_PWM5_CTRL		0x50
#define	DELAY_SET_IOC_CCLK		0x5C
#define	CT_DISABLE_BIU			0x68
#define	CPLL_CONTROL3			0x70
#define	GC_CLK_CNTRL			0x74
#define	SCRATCH_1_CLKCFG		0x78
#define	SCRATCH_2_CLKCFG		0x7C
#define	SCRATCH_CLKCFG			0x84
#define	CLKGATE_DISABLE2		0x8C
#define	CLKGATE_DISABLE			0x94
#define	CPLL_CONTROL2			0x98
#define	CLK_TOP_PERF_CNTL		0xAC
#define	CLK_TOP_PWM1_CTRL		0xB0
#define	CLK_TOP_PWM2_CTRL		0xB4
#define	CLK_TOP_SET_CTRL		0xB8
#define	CLK_TOP_THERMAL_ALERT_INTR_EN	0xC0
#define	CLK_TOP_THERMAL_ALERT_STATUS	0xC4
#define	CLK_TOP_THERMAL_ALERT_WAIT_WINDOW	0xC8
#define	CLK_TOP_PWM3_CTRL		0xCC
#define	CLK_TOP_SPARE_PLL		0xD0
#define	CLK_CFG_HTPLL_CNTL		0xD4
#define CLK_TOP_SPARE_A			0xE0
#define CLK_TOP_SPARE_B			0xE4
#define CLK_TOP_SPARE_C			0xE8
#define CLK_TOP_SPARE_D			0xEC
#define	CFG_CT_CLKGATE_HTIU		0xF8

/*---------------------------------------------------------------------------------------------------------*/
/* chip revision */
#define	REV_RS690_A11	5
#define	REV_RS690_A12	6
#define	REV_RS690_A21	7

/* PCI standard BAR */
#define	PCI_BASE_ADDRESS_0			0x10
#define	PCI_BASE_ADDRESS_1			0x14
#define	PCI_BASE_ADDRESS_2			0x18
#define	PCI_BASE_ADDRESS_3			0x1C
#define	PCI_BASE_ADDRESS_4			0x20
#define	PCI_BASE_ADDRESS_5			0x24

/* some others */
#define	TEMP_PM2_BAR_BASE_ADDRESS		0xd000
#define	TEMP_EXT_CONF_BASE_ADDRESS		0x17000000	// 0x12000000 ~ 0x12200000
//#define	TEMP_EXT_CONF_BASE_ADDRESS		0x12000000	// 0x12000000 ~ 0x12200000
#define	TEMP_MMIO_BASE_ADDRESS			0x30000000

/* gfx clock module : MMREG base */
#define CLK_CNTL_INDEX	0x08
#define CLK_CNTL_DATA	0x0C

#endif	/* _RS690_H_ */
/*------------------------------------------------------------------------------------------------------*/
