#ifndef _EMERALD_H
#define _EMERALD_H

/** @file
 *
 * Sapphire 10 Gigabit Ethernet network card driver
 *
 */
#ifndef PMON
FILE_LICENCE ( GPL2_OR_LATER_OR_UBDL );

#include <stdint.h>
#include <stdbool.h>
#include <ipxe/if_ether.h>
#include <ipxe/nvs.h>
#endif

/** Sapphire BAR size */
#define INTEL_BAR_SIZE ( 128 * 1024 )

/** A packet descriptor */
union emerald_descriptor {
	struct {
		/** Buffer address */
		uint64_t address;
		/** Length */
		uint16_t length;
		/** Flags */
		uint8_t flags;
		/** Command */
		uint8_t command;
		/** Status */
		uint32_t status;
	}tx;
	struct {
		/** Buffer address */
		uint64_t address;
		uint32_t status_error; /* ext status/error */
		uint16_t length; /* Packet length */
		uint16_t vlan; /* VLAN tag */
	}rx;
} __attribute__ (( packed ));

/** Descriptor extension */
#define EMERALD_TXD_DTYP_DATA             0x00 /* Adv Data Descriptor */

/** Report status */
#define EMERALD_TXD_RS                    0x08 /* Report Status */

/** Insert frame checksum (CRC) */
#define EMERALD_TXD_IFCS                  0x02 /* Insert FCS */

/** End of packet */
#define EMERALD_TXD_EOP                   0x01  /* End of Packet */

/** Descriptor done */
#define EMERALD_DESC_STATUS_DD 0x00000001UL

/** Receive error */
#define EMERALD_DESC_STATUS_RXE 0x20000000UL

/** Payload length */
#define EMERALD_DESC_STATUS_PAYLEN( len ) ( (len) << 13 )


/** Number of receive descriptors
 *
 * Minimum value is 8, since the descriptor ring length must be a
 * multiple of 128.
 */
#define EMERALD_NUM_RX_DESC 128

/** Receive descriptor ring fill level */
#define EMERALD_RX_FILL 8

/** Receive buffer length */
#define EMERALD_RX_MAX_LEN 2048


/** Number of transmit descriptors
 *
 * Descriptor ring length must be a multiple of 16.  ICH8/9/10
 * requires a minimum of 16 TX descriptors.
 */
#define EMERALD_NUM_TX_DESC 128

/** Transmit descriptor ring maximum fill level */
#define EMERALD_TX_FILL ( EMERALD_NUM_TX_DESC - 1 )

/** Receive/Transmit Descriptor Base Address Low (offset) */
#define EMERALD_DBAL 0x00

/** Receive/Transmit Descriptor Base Address High (offset) */
#define EMERALD_DBAH 0x04

/** Receive/Transmit Descriptor Head (offset) */
#define EMERALD_DH 0x08

/** Receive/Transmit Descriptor Tail (offset) */
#define EMERALD_DT 0x0C

/** Receive/Transmit Descriptor Control (offset) */
#define EMERALD_DCTL 0x10
#define EMERALD_DCTL_ENABLE	0x00000001UL	/**< Queue enable */
#define EMERALD_DCTL_RING_SIZE_SHIFT 1
#define EMERALD_DCTL_TX_WTHRESH_SHIFT 16
#define EMERALD_DCTL_RX_THER_SHIFT 16
#define EMERALD_DCTL_RX_HDR_SZ 0x0000F000UL
#define EMERALD_DCTL_RX_BUF_SZ 0x00000F00UL
#define EMERALD_DCTL_RX_SPLIT_MODE 0X04000000UL
#define EMERALD_DCTL_RX_BSIZEPKT_SHIFT          2 /* so many KBs */
#define EMERALD_DCTL_RX_BSIZEHDRSIZE_SHIFT      6
#define EMERALD_RX_HDR_SIZE 256
#define EMERALD_RX_BSIZE_DEFAULT 2048

/** Maximum time to wait for queue disable, in milliseconds */
#define EMERALD_DISABLE_MAX_WAIT_MS 100

/** Receive address */
union emerald_receive_address {
	struct {
		uint32_t low;
		uint32_t high;
	} __attribute__ (( packed )) reg;
	//uint8_t raw[ETH_ALEN];
};

/** A descriptor ring */
struct emerald_ring {
	/** Descriptors */
	union emerald_descriptor *desc;
	/** Producer index */
	unsigned int prod;
	/** Consumer index */
	unsigned int cons;

	/** Register block */
	unsigned int reg;
	/** Length (in bytes) */
	size_t len;

	/** Populate descriptor
	 *
	 * @v desc		Descriptor
	 * @v addr		Data buffer address
	 * @v len		Length of data
	 */
	void ( * describe ) ( union emerald_descriptor *desc, physaddr_t addr,
			      size_t len );
};

/**
 * Initialise descriptor ring
 *
 * @v ring		Descriptor ring
 * @v count		Number of descriptors
 * @v reg		Descriptor register block
 * @v describe		Method to populate descriptor
 */
static inline __attribute__ (( always_inline)) void
emerald_init_ring ( struct emerald_ring *ring, unsigned int count, unsigned int reg,
		  void ( * describe ) ( union emerald_descriptor *desc,
					physaddr_t addr, size_t len ) ) {

	ring->len = ( count * sizeof ( ring->desc[0] ) );
	ring->reg = reg;
	ring->describe = describe;
}

/** A virtual function mailbox */
struct emerald_mailbox {
	/** Mailbox control register */
	unsigned int ctrl;
	/** Mailbox memory base */
	unsigned int mem;
};

/**
 * Initialise mailbox
 *
 * @v mbox		Mailbox
 * @v ctrl		Mailbox control register
 * @v mem		Mailbox memory register base
 */
static inline __attribute__ (( always_inline )) void
emerald_init_mbox ( struct emerald_mailbox *mbox, unsigned int ctrl,
		  unsigned int mem ) {

	mbox->ctrl = ctrl;
	mbox->mem = mem;
}

enum emerald_isb_idx {
	EMERALD_ISB_HEADER,
	EMERALD_ISB_MISC,
	EMERALD_ISB_VEC0,
	EMERALD_ISB_VEC1,
	EMERALD_ISB_MAX
};


/** A network card */
struct emerald_nic {
	/** Registers */
	void *regs;
	/** Port number (for multi-port devices) */
	unsigned int port;
	/** Flags */
	unsigned int flags;
	/** Forced interrupts */
	unsigned int force_icr;

#ifndef PMON
	/** EEPROM */
	struct nvs_device eeprom;
#endif
	/** EEPROM done flag */
	uint32_t eerd_done;
	/** EEPROM address shift */
	unsigned int eerd_addr_shift;

	/** Mailbox */
	struct emerald_mailbox mbox;

	/** Transmit descriptor ring */
	struct emerald_ring tx;
	/** Receive descriptor ring */
	struct emerald_ring rx;
	/** Receive I/O buffers */
	struct io_buffer *rx_iobuf[EMERALD_NUM_RX_DESC];
	physaddr_t isb_dma;
	uint32_t isb_mem[EMERALD_ISB_MAX];

	uint32_t sfp_type;
	uint32_t speed;
	uint32_t count;
	uint32_t phy_addr;
	bool link_up;

	struct pci_device *pdev;
	u16 subsystem_device;
};

#if 0
/**
 * Dump diagnostic information
 *
 * @v emerald device
 */
static inline void emerald_diag ( struct emerald_nic *emerald ) {

	DBGC ( emerald, "INTEL %p TX %04x(%02x)/%04x(%02x) "
	       "RX %04x(%02x)/%04x(%02x)\n", emerald,
	       ( emerald->tx.cons & 0xffff ),
	       readl ( emerald->regs + emerald->tx.reg + INTEL_xDH ),
	       ( emerald->tx.prod & 0xffff ),
	       readl ( emerald->regs + emerald->tx.reg + INTEL_xDT ),
	       ( emerald->rx.cons & 0xffff ),
	       readl ( emerald->regs + emerald->rx.reg + INTEL_xDH ),
	       ( emerald->rx.prod & 0xffff ),
	       readl ( emerald->regs + emerald->rx.reg + INTEL_xDT ) );
}
#endif

extern void emerald_describe_tx ( union emerald_descriptor *tx,
				physaddr_t addr, size_t len );
extern void emerald_describe_tx_adv ( union emerald_descriptor *tx,
				    physaddr_t addr, size_t len );
extern void emerald_describe_rx ( union emerald_descriptor *rx,
				physaddr_t addr, size_t len );
extern void emerald_reset_ring ( struct emerald_nic *emerald, unsigned int reg );
extern int emerald_create_ring ( struct emerald_nic *emerald,
			       struct emerald_ring *ring, int is_tx_ring);
extern void emerald_destroy_ring ( struct emerald_nic *emerald,
				 struct emerald_ring *ring );
extern void emerald_refill_rx ( struct emerald_nic *emerald );
extern void emerald_empty_rx ( struct emerald_nic *emerald );
extern int emerald_transmit ( struct net_device *netdev,
			    struct io_buffer *iobuf );
extern void emerald_poll_tx ( struct net_device *netdev );
extern void emerald_poll_rx ( struct net_device *netdev );


/** Device Control Register */
#define EMERALD_MIS_RST 0x1000CUL
#define EMERALD_MIS_RST_LAN0_RST	0x00000002UL	/**< lan0 reset */
#define EMERALD_MIS_RST_LAN1_RST	0x00000004UL	/**< lan1 reset */
#define EMERALD_MIS_RST_LAN2_RST	0x00000008UL	/**< lan2 reset */
#define EMERALD_MIS_RST_LAN3_RST	0x00000010UL	/**< lan3 reset */

/** Time to delay for device reset, in milliseconds */
#define EMERALD_RESET_DELAY_MS 1500

/** Extended Interrupt Cause Read Register */
#define EMERALD_EIMC_VEC0	0x1UL	/**< TX0/RX0 (via IVAR) */
#define EMERALD_PX_ISB_ADDR_L 0x160
#define EMERALD_PX_ISB_ADDR_H 0x164

#define EMERALD_PX_MISC_IC_ETH_LKDN 0x00000100UL
#define EMERALD_PX_MISC_IC_ETH_LK 0x00040000UL


#define EMERALD_PX_MISC_IEN 0x108
#define EMERALD_PX_MISC_IEN_ETH_LKDN 0x00000100UL
#define EMERALD_PX_MISC_IEN_ETH_LK   0x00040000UL

/** Interrupt Mask Set/Read Register */
#define EMERALD_PX_IMS0 0x140UL


/** Interrupt Mask Clear Register */
#define EMERALD_PX_IMC0 0x150UL

/** Interrupt Vector Allocation Register */
#define EMERALD_PX_IVAR0 0x500UL
#define EMERALD_PX_IVAR_RX0_DEFAULT	0x00
#define EMERALD_PX_IVAR_RX0_VALID	0x00000080UL	/**< RX queue 0 valid */
#define EMERALD_PX_IVAR_TX0_DEFAULT	0x00
#define EMERALD_PX_IVAR_TX0_VALID	0x00008000UL	/**< TX queue 0 valid */
#define EMERALD_PX_MISC_IVAR 0x4FCUL
#define EMERALD_PX_MISC_IVAR_DEFAULT   0x1
#define EMERALD_PX_MISC_IVAR_VALID     0x00000080UL

#define EMERALD_PX_ITR0 0x200UL
#define EMERALD_PX_ITR_CNT_WDIS 0x80000000UL
#define EMERALD_DEFAULT_ITR 200

#define EMERALD_MIS_ST 0x10028UL
#define EMERALD_MIS_ST_GPHY_IN_RST(_r)    (0x00000200UL << (_r))

#define EMERALD_PHY_CONFIG(_r)    (0x14000UL + ((_r) * 4))
#define EMERALD_INTPHY_PAGE_SELECT   31
#define EMERALD_INTPHY_PAGE_MAX   32

#define EMERALD_PHY_SPEED_SELECT1             0x0040
#define EMERALD_PHY_DUPLEX                    0x0100
#define EMERALD_PHY_RESTART_AN                0x0200
#define EMERALD_PHY_ANE                       0x1000
#define EMERALD_PHY_SPEED_SELECT0             0x2000
#define EMERALD_PHY_RESET                     0x8000



/** Receive Filter Control Register */
#define EMERALD_PSR_CTL 0x15000UL
#define EMERALD_PSR_CTL_MPE	0x00000100UL	/**< Multicast promiscuous */
#define EMERALD_PSR_CTL_UPE	0x00000200UL	/**< Unicast promiscuous mode */
#define EMERALD_PSR_CTL_BAM	0x00000400UL	/**< Broadcast accept mode */

#define EMERALD_PSR_VLAN_CTL 0x15088UL
#define EMERALD_PSR_VLAN_CTL_VFE 0x40000000UL

#define EMERALD_PSR_MAC_SWC_IDX 0x16210
#define EMERALD_PSR_MAC_SWC_AD_L  0x16200
#define EMERALD_PSR_MAC_SWC_AD_H  0x16204

/** Receive Address Low
 *
 */


/** Receive Descriptor register block */
#define EMERALD_RD 0x01000UL


#define EMERALD_TSEC_CTL 0x1D000UL
//#define EMERALD_RSEC_CTL 0x17000UL

/** Configure ETH MAC */
#define EMERALD_MAC_PKT_FLT 0x11008UL
#define EMERALD_MAC_RX_FLOW_CTRL 0x11090UL
#define EMERALD_MAC_WDG_TIMEOUT 0x1100CUL
#define EMERALD_MAC_RX_FLOW_CTRL_RFE 0x00000001UL
#define EMERALD_MAC_PKT_FLT_PR (0x1) /* promiscuous mode */

/** Receive DMA Control Register */
#define EMERALD_RSEC_CTL 0x17000UL
#define EMERALD_RSEC_CTL_CRC_STRIP	0x00000004UL	/**< Strip CRC */

/** Receive Control Register */
#define EMERALD_MAC_RX_CFG 0x11004UL
#define EMERALD_MAC_RX_CFG_RE	0x00000001UL	/**< Receive enable */
#define EMERALD_MAC_RX_CFG_JE	0x00000100UL	/**< Jumbo frame enable */

#define EMERALD_RDB_PB_CTL 0x19000UL
#define EMERALD_RDB_PB_CTL_RXEN 0x80000000UL

/** Transmit DMA Control Register */
#define EMERALD_DMATXCTL 0x18000UL
#define EMERALD_DMATXCTL_TE	0x00000001UL	/**< Transmit enable */

#define EMERALD_MACTXCFG 0x11000UL
#define EMERALD_MACTXCFG_TE 0x00000001UL
#define EMERALD_MACTXCFG_SPEED_MASK     0x60000000UL
#define EMERALD_MACTXCFG_SPEED_10G      0x00000000UL
#define EMERALD_MACTXCFG_SPEED_1G       0x60000000UL



/** Transmit Descriptor register block */
#define EMERALD_TD 0x03000UL


/** Maximum Frame Size Register */
#define EMERALD_PSR_MAX_SZ 0x15020UL
#define EMERALD_MAXFRS_MFS_DEFAULT \
		( ETH_FRAME_LEN + 4 /* VLAN */ + 4 /* CRC */ )

#define EMERALD_CFG_LAN_SPEED             0x14440



/** Link Status Register */
#define EMERALD_CFG_PORT_ST 0x14404UL
#define EMERALD_CFG_PORT_ST_LINK_UP       0x00000001	/**< Link up */

#define EMERALD_GPIO_DR                   0x14800UL
#define EMERALD_GPIO_DDR                  0x14804UL
#if 0

#define EMERALD_GPIO_DR_0         0x00000001U /* SDP0 Data Value */
#define EMERALD_GPIO_DR_1         0x00000002U /* SDP1 Data Value */
#define EMERALD_GPIO_DR_2         0x00000004U /* SDP2 Data Value */
#define EMERALD_GPIO_DR_3         0x00000008U /* SDP3 Data Value */
#define EMERALD_GPIO_DR_4         0x00000010U /* SDP4 Data Value */
#define EMERALD_GPIO_DR_5         0x00000020U /* SDP5 Data Value */
#define EMERALD_GPIO_DR_6         0x00000040U /* SDP6 Data Value */
#define EMERALD_GPIO_DR_7         0x00000080U /* SDP7 Data Value */
#define EMERALD_GPIO_DDR_0        0x00000001U /* SDP0 IO direction */
#define EMERALD_GPIO_DDR_1        0x00000002U /* SDP1 IO direction */
#define EMERALD_GPIO_DDR_2        0x00000004U /* SDP1 IO direction */
#define EMERALD_GPIO_DDR_3        0x00000008U /* SDP3 IO direction */
#define EMERALD_GPIO_DDR_4        0x00000010U /* SDP4 IO direction */
#define EMERALD_GPIO_DDR_5        0x00000020U /* SDP5 IO direction */
#define EMERALD_GPIO_DDR_6        0x00000040U /* SDP6 IO direction */
#define EMERALD_GPIO_DDR_7        0x00000080U /* SDP7 IO direction */
#endif
#define EMERALD_LINK_SPEED_1GB_FULL       2
#define EMERALD_LINK_SPEED_100M_FULL      1
#define EMERALD_LINK_SPEED_10M_FULL       8
#define EMERALD_LINK_SPEED_UNKNOWN        0
//#define EMERALD_LINK_SPEED_10GB_FULL      4

#define EMERALD_XPCS_POWER_GOOD_MAX_POLLING_TIME 100

//#define EMERALD_VR_XS_OR_PCS_MMD_DIGI_CTL1        	0x38000
//#define EMERALD_VR_XS_OR_PCS_MMD_DIGI_STATUS 		0x38010

//#define EMERALD_VR_XS_OR_PCS_MMD_DIGI_STATUS_PSEQ_MASK            0x1C
//#define EMERALD_VR_XS_OR_PCS_MMD_DIGI_STATUS_PSEQ_POWER_GOOD      0x10

//#define EMERALD_XPCS_IDA_ADDR    0x13000
//#define EMERALD_XPCS_IDA_DATA    0x13004

#define EMERALD_FAILED_READ_REG       0xffffffffU

#define EMERALD_MAC_TX_CFG                0x11000
#define EMERALD_MAC_TX_CFG_TE             0x00000001U

//#define EMERALD_SR_AN_MMD_CTL                     0x70000
//#define EMERALD_SR_PCS_CTL2                       0x30007
//#define EMERALD_SR_PMA_MMD_CTL1                   0x10000
//#define EMERALD_SR_MII_MMD_CTL                    0x1F0000
//#define EMERALD_SR_MII_MMD_AN_CTL                 0x1F8001

#if 0
#define EMERALD_PHY_TX_GENCTRL1                   0x18031
#define EMERALD_PHY_MISC_CTL0                     0x18090
#define EMERALD_PHY_TX_EQ_CTL0                    0x18036
#define EMERALD_PHY_TX_EQ_CTL1                    0x18037
#define EMERALD_PHY_RX_EQ_ATT_LVL0                0x18057
#define EMERALD_PHY_RX_EQ_CTL0                    0x18058
#define EMERALD_PHY_DFE_TAP_CTL0                  0x1805E
#define EMERALD_PHY_RX_GEN_CTL3                   0x18053
#define EMERALD_PHY_MPLLA_CTL0                    0x18071
#define EMERALD_PHY_MPLLA_CTL3                    0x18077
#define EMERALD_PHY_VCO_CAL_LD0                   0x18092
#define EMERALD_PHY_VCO_CAL_REF0                  0x18096
#define EMERALD_PHY_AFE_DFE_ENABLE                0x1805D
#define EMERALD_PHY_RX_EQ_CTL                     0x1805C
#define EMERALD_PHY_TX_RATE_CTL                   0x18034
#define EMERALD_PHY_RX_RATE_CTL                   0x18054
#define EMERALD_PHY_TX_GEN_CTL2                   0x18032
#define EMERALD_PHY_RX_GEN_CTL2                   0x18052
#define EMERALD_PHY_MPLLA_CTL2                    0x18073
#define EMERALD_PHY_RX_AD_ACK                     0x18098
#endif
#define EMERALD_PHY_INIT_DONE_POLLING_TIME        100

//#define EMERALD_VR_XS_OR_PCS_MMD_DIGI_CTL1_VR_RST 0x8000
#if 0
#define EMERALD_I2C_ENABLE                0x1496C /* I2C Enable */
#define EMERALD_I2C_CON                   0x14900 /* I2C Control */
#define EMERALD_I2C_INTR_MASK             0x14930 /* I2C Interrupt Mask */
#define EMERALD_I2C_TAR                   0x14904 /* I2C Target Address */
#define EMERALD_I2C_DATA_CMD              0x14910 /* I2C Rx/Tx Data Buf and Cmd */
#define EMERALD_I2C_SS_SCL_HCNT           0x14914 /* Standard speed I2C Clock SCL
                                                 * High Count */
#define EMERALD_I2C_SS_SCL_LCNT           0x14918 /* Standard speed I2C Clock SCL
                                                 * Low Count */
#define EMERALD_I2C_RAW_INTR_STAT         0x14934 /* I2C Raw Interrupt Status */
#define EMERALD_I2C_RX_TL                 0x14938 /* I2C Receive FIFO Threshold */
#define EMERALD_I2C_TX_TL                 0x1493C /* I2C TX FIFO Threshold */
#define EMERALD_I2C_SCL_STUCK_TIMEOUT     0x149AC /* I2C SCL stuck at low timeout
                                                 * register */
#define EMERALD_I2C_SDA_STUCK_TIMEOUT     0x149B0 /*I2C SDA Stuck at Low Timeout*/

#define EMERALD_I2C_CON_SLAVE_DISABLE     ((1 << 6))
#define EMERALD_I2C_CON_RESTART_EN        ((1 << 5))
#define EMERALD_I2C_CON_SPEED(_v)         (((_v) & 0x3) << 1)
#define EMERALD_I2C_CON_MASTER_MODE       ((1 << 0))
#define EMERALD_I2C_INTR_STAT_RX_FULL     ((0x1) << 2)
#define EMERALD_I2C_INTR_STAT_TX_EMPTY    ((0x1) << 4)
#define EMERALD_I2C_DATA_CMD_STOP         ((1 << 9))
#define EMERALD_I2C_DATA_CMD_READ         ((1 << 8) | EMERALD_I2C_DATA_CMD_STOP)

#define EMERALD_SFF_IDENTIFIER            0x0
#define EMERALD_SFF_IDENTIFIER_SFP        0x3
#define EMERALD_SFF_1GBE_COMP_CODES       0x6
#define EMERALD_SFF_10GBE_COMP_CODES      0x3
#define EMERALD_SFF_CABLE_TECHNOLOGY      0x8
#define EMERALD_SFF_DA_PASSIVE_CABLE      0x4

#define EMERALD_I2C_TIMEOUT  1000
#define EMERALD_I2C_SLAVE_ADDR            (0x50)
#define EMERALD_I2C_EEPROM_DEV_ADDR       0xA0

enum emerald_sfp_type {
	emerald_sfp_type_da_cu_core0 = 3,
	emerald_sfp_type_unknown = 100
};
#endif

#define EMERALD_MDIO_TIMEOUT 1000

#define EMERALD_MSCA                      0x11200
#define EMERALD_MSCA_RA(v)                ((0xFFFF & (v)))
#define EMERALD_MSCA_PA(v)                ((0x1F & (v)) << 16)
#define EMERALD_MSCA_DA(v)                ((0x1F & (v)) << 21)
#define EMERALD_MSCC                      0x11204
#define EMERALD_MSCC_DATA(v)              ((0xFFFF & (v)))
#define EMERALD_MSCC_CMD(v)               ((0x3 & (v)) << 16)
enum EMERALD_MSCA_CMD_VALUE {
	EMERALD_MSCA_CMD_RSV = 0,
	EMERALD_MSCA_CMD_WRITE,
	EMERALD_MSCA_CMD_POST_READ,
	EMERALD_MSCA_CMD_READ,
};
#define EMERALD_MSCC_SADDR                ((0x1U) << 18)
#define EMERALD_MSCC_CR(v)                ((0x8U & (v)) << 19)
#define EMERALD_MSCC_BUSY                 ((0x1U) << 22)
#define EMERALD_MDIO_CLK(v)               ((0x7 & (v)) << 19)

#define EMERALD_MDIO_CLAUSE22             0x11220

#define EMERALD_EXPHY_SPEED_SELECT_H      0x00000040
#define EMERALD_EXPHY_DUPLEX              0x00000100
#define EMERALD_EXPHY_AUTONEG             0x00001000
#define EMERALD_EXPHY_SPEED_SELECT_L      0x00002000
#define EMERALD_EXPHY_RESET               0x00008000

#define EMERALD_EXPHY_TX_TIMING_CTRL      0x00000010
#define EMERALD_EXPHY_RX_TIMING_CTRL      0x00000020

#define EMERALD_EXPHY_RESET_POLLING_TIME  5

#define EMERALD_LINK_UP_TIME              5

#define usec_delay(x) udelay(x)
#define msleep(x) mdelay(x)

#define EMERALD_ERR									100
#define EMERALD_ERR_INVALID_PARAM					-(EMERALD_ERR+0)
#define EMERALD_ERR_TIMEOUT							-(EMERALD_ERR+1)
#define EMERALD_ERR_I2C								-(EMERALD_ERR+2)
#define EMERALD_ERR_I2C_SFP_NOT_SUPPORTED			-(EMERALD_ERR+3)
#define EMERALD_ERR_NOT_SFP							-(EMERALD_ERR+4)
#define EMERALD_ERR_PHY								-(EMERALD_ERR+5)
#define EMERALD_ERR_GPHY							-(EMERALD_ERR+6)
#define EMERALD_ERR_LINK_SPEED						-(EMERALD_ERR+7)
#define EMERALD_ERR_DEV_ID							-(EMERALD_ERR+8)


#endif /* _EMERALD_H */

