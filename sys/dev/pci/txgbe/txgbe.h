#ifndef _SAPPHIRE_H
#define _SAPPHIRE_H

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
union sapphire_descriptor {
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
#define SAPPHIRE_TXD_DTYP_DATA             0x00 /* Adv Data Descriptor */

/** Report status */
#define SAPPHIRE_TXD_RS                    0x08 /* Report Status */

/** Insert frame checksum (CRC) */
#define SAPPHIRE_TXD_IFCS                  0x02 /* Insert FCS */

/** End of packet */
#define SAPPHIRE_TXD_EOP                   0x01  /* End of Packet */

/** Descriptor done */
#define SAPPHIRE_DESC_STATUS_DD 0x00000001UL

/** Receive error */
#define SAPPHIRE_DESC_STATUS_RXE 0x20000000UL

/** Payload length */
#define SAPPHIRE_DESC_STATUS_PAYLEN( len ) ( (len) << 13 )


/** Number of receive descriptors
 *
 * Minimum value is 8, since the descriptor ring length must be a
 * multiple of 128.
 */
#define SAPPHIRE_NUM_RX_DESC 128

/** Receive descriptor ring fill level */
#define SAPPHIRE_RX_FILL 8

/** Receive buffer length */
#define SAPPHIRE_RX_MAX_LEN 2048


/** Number of transmit descriptors
 *
 * Descriptor ring length must be a multiple of 16.  ICH8/9/10
 * requires a minimum of 16 TX descriptors.
 */
#define SAPPHIRE_NUM_TX_DESC 128

/** Transmit descriptor ring maximum fill level */
#define SAPPHIRE_TX_FILL ( SAPPHIRE_NUM_TX_DESC - 1 )

/** Receive/Transmit Descriptor Base Address Low (offset) */
#define SAPPHIRE_DBAL 0x00

/** Receive/Transmit Descriptor Base Address High (offset) */
#define SAPPHIRE_DBAH 0x04

/** Receive/Transmit Descriptor Head (offset) */
#define SAPPHIRE_DH 0x08

/** Receive/Transmit Descriptor Tail (offset) */
#define SAPPHIRE_DT 0x0C

/** Receive/Transmit Descriptor Control (offset) */
#define SAPPHIRE_DCTL 0x10
#define SAPPHIRE_DCTL_ENABLE	0x00000001UL	/**< Queue enable */
#define SAPPHIRE_DCTL_RING_SIZE_SHIFT 1
#define SAPPHIRE_DCTL_TX_WTHRESH_SHIFT 16
#define SAPPHIRE_DCTL_RX_THER_SHIFT 16
#define SAPPHIRE_DCTL_RX_HDR_SZ 0x0000F000UL
#define SAPPHIRE_DCTL_RX_BUF_SZ 0x00000F00UL
#define SAPPHIRE_DCTL_RX_SPLIT_MODE 0X04000000UL
#define SAPPHIRE_DCTL_RX_BSIZEPKT_SHIFT          2 /* so many KBs */
#define SAPPHIRE_DCTL_RX_BSIZEHDRSIZE_SHIFT      6
#define SAPPHIRE_RX_HDR_SIZE 256
#define SAPPHIRE_RX_BSIZE_DEFAULT 2048

/** Maximum time to wait for queue disable, in milliseconds */
#define SAPPHIRE_DISABLE_MAX_WAIT_MS 100

/** Receive address */
union sapphire_receive_address {
	struct {
		uint32_t low;
		uint32_t high;
	} __attribute__ (( packed )) reg;
	//uint8_t raw[ETH_ALEN];
};

/** A descriptor ring */
struct sapphire_ring {
	/** Descriptors */
	union sapphire_descriptor *desc;
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
	void ( * describe ) ( union sapphire_descriptor *desc, physaddr_t addr,
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
sapphire_init_ring ( struct sapphire_ring *ring, unsigned int count, unsigned int reg,
		  void ( * describe ) ( union sapphire_descriptor *desc,
					physaddr_t addr, size_t len ) ) {

	ring->len = ( count * sizeof ( ring->desc[0] ) );
	ring->reg = reg;
	ring->describe = describe;
}

/** A virtual function mailbox */
struct sapphire_mailbox {
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
sapphire_init_mbox ( struct sapphire_mailbox *mbox, unsigned int ctrl,
		  unsigned int mem ) {

	mbox->ctrl = ctrl;
	mbox->mem = mem;
}

enum sapphire_isb_idx {
	SAPPHIRE_ISB_HEADER,
	SAPPHIRE_ISB_MISC,
	SAPPHIRE_ISB_VEC0,
	SAPPHIRE_ISB_VEC1,
	SAPPHIRE_ISB_MAX
};


/** A network card */
struct sapphire_nic {
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
	struct sapphire_mailbox mbox;

	/** Transmit descriptor ring */
	struct sapphire_ring tx;
	/** Receive descriptor ring */
	struct sapphire_ring rx;
	/** Receive I/O buffers */
	struct io_buffer *rx_iobuf[SAPPHIRE_NUM_RX_DESC];
	physaddr_t isb_dma;
	uint32_t isb_mem[SAPPHIRE_ISB_MAX];

	uint32_t sfp_type;
	uint32_t speed;
	uint32_t count;
};

#if 0
/**
 * Dump diagnostic information
 *
 * @v sapphire device
 */
static inline void sapphire_diag ( struct sapphire_nic *sapphire ) {

	DBGC ( sapphire, "INTEL %p TX %04x(%02x)/%04x(%02x) "
	       "RX %04x(%02x)/%04x(%02x)\n", sapphire,
	       ( sapphire->tx.cons & 0xffff ),
	       readl ( sapphire->regs + sapphire->tx.reg + INTEL_xDH ),
	       ( sapphire->tx.prod & 0xffff ),
	       readl ( sapphire->regs + sapphire->tx.reg + INTEL_xDT ),
	       ( sapphire->rx.cons & 0xffff ),
	       readl ( sapphire->regs + sapphire->rx.reg + INTEL_xDH ),
	       ( sapphire->rx.prod & 0xffff ),
	       readl ( sapphire->regs + sapphire->rx.reg + INTEL_xDT ) );
}
#endif

extern void sapphire_describe_tx ( union sapphire_descriptor *tx,
				physaddr_t addr, size_t len );
extern void sapphire_describe_tx_adv ( union sapphire_descriptor *tx,
				    physaddr_t addr, size_t len );
extern void sapphire_describe_rx ( union sapphire_descriptor *rx,
				physaddr_t addr, size_t len );
extern void sapphire_reset_ring ( struct sapphire_nic *sapphire, unsigned int reg );
extern int sapphire_create_ring ( struct sapphire_nic *sapphire,
			       struct sapphire_ring *ring, int is_tx_ring);
extern void sapphire_destroy_ring ( struct sapphire_nic *sapphire,
				 struct sapphire_ring *ring );
extern void sapphire_refill_rx ( struct sapphire_nic *sapphire );
extern void sapphire_empty_rx ( struct sapphire_nic *sapphire );
extern int sapphire_transmit ( struct net_device *netdev,
			    struct io_buffer *iobuf );
extern void sapphire_poll_tx ( struct net_device *netdev );
extern void sapphire_poll_rx ( struct net_device *netdev );


/** Device Control Register */
#define SAPPHIRE_MIS_RST 0x1000CUL
#define SAPPHIRE_MIS_RST_LAN0_RST	0x00000002UL	/**< lan0 reset */
#define SAPPHIRE_MIS_RST_LAN1_RST	0x00000004UL	/**< lan1 reset */

/** Time to delay for device reset, in milliseconds */
#define SAPPHIRE_RESET_DELAY_MS 1000

/** Extended Interrupt Cause Read Register */
#define SAPPHIRE_EIMC_VEC0	0x1UL	/**< TX0/RX0 (via IVAR) */
#define SAPPHIRE_PX_ISB_ADDR_L 0x160
#define SAPPHIRE_PX_ISB_ADDR_H 0x164

#define SAPPHIRE_PX_MISC_IC_ETH_LKDN 0x00000100UL
#define SAPPHIRE_PX_MISC_IC_ETH_LK 0x00040000UL


#define SAPPHIRE_PX_MISC_IEN 0x108
#define SAPPHIRE_PX_MISC_IEN_ETH_LKDN 0x00000100UL
#define SAPPHIRE_PX_MISC_IEN_ETH_LK   0x00040000UL 

/** Interrupt Mask Set/Read Register */
#define SAPPHIRE_PX_IMS0 0x140UL


/** Interrupt Mask Clear Register */
#define SAPPHIRE_PX_IMC0 0x150UL

/** Interrupt Vector Allocation Register */
#define SAPPHIRE_PX_IVAR0 0x500UL
#define SAPPHIRE_PX_IVAR_RX0_DEFAULT	0x00
#define SAPPHIRE_PX_IVAR_RX0_VALID	0x00000080UL	/**< RX queue 0 valid */
#define SAPPHIRE_PX_IVAR_TX0_DEFAULT	0x00
#define SAPPHIRE_PX_IVAR_TX0_VALID	0x00008000UL	/**< TX queue 0 valid */
#define SAPPHIRE_PX_MISC_IVAR 0x4FCUL
#define SAPPHIRE_PX_MISC_IVAR_DEFAULT   0x1
#define SAPPHIRE_PX_MISC_IVAR_VALID     0x00000080UL

#define SAPPHIRE_PX_ITR0 0x200UL
#define SAPPHIRE_PX_ITR_CNT_WDIS 0x80000000UL
#define SAPPHIRE_DEFAULT_ITR 200

/** Receive Filter Control Register */
#define SAPPHIRE_PSR_CTL 0x15000UL
#define SAPPHIRE_PSR_CTL_MPE	0x00000100UL	/**< Multicast promiscuous */
#define SAPPHIRE_PSR_CTL_UPE	0x00000200UL	/**< Unicast promiscuous mode */
#define SAPPHIRE_PSR_CTL_BAM	0x00000400UL	/**< Broadcast accept mode */

#define SAPPHIRE_PSR_VLAN_CTL 0x15088UL
#define SAPPHIRE_PSR_VLAN_CTL_VFE 0x40000000UL

#define SAPPHIRE_PSR_MAC_SWC_IDX 0x16210
#define SAPPHIRE_PSR_MAC_SWC_AD_L  0x16200
#define SAPPHIRE_PSR_MAC_SWC_AD_H  0x16204

/** Receive Address Low
 *
 */


/** Receive Descriptor register block */
#define SAPPHIRE_RD 0x01000UL


/** Configure ETH MAC */
#define SAPPHIRE_MAC_PKT_FLT 0x11008UL
#define SAPPHIRE_MAC_RX_FLOW_CTRL 0x11090UL
#define SAPPHIRE_MAC_WDG_TIMEOUT 0x1100CUL
#define SAPPHIRE_MAC_RX_FLOW_CTRL_RFE 0x00000001UL
#define SAPPHIRE_MAC_PKT_FLT_PR (0x1) /* promiscuous mode */

/** Receive DMA Control Register */
#define SAPPHIRE_RSC_CTL 0x17000UL
#define SAPPHIRE_RSC_CTL_CRC_STRIP	0x00000004UL	/**< Strip CRC */

/** Receive Control Register */
#define SAPPHIRE_MAC_RX_CFG 0x11004UL
#define SAPPHIRE_MAC_RX_CFG_RE	0x00000001UL	/**< Receive enable */
#define SAPPHIRE_MAC_RX_CFG_JE	0x00000100UL	/**< Jumbo frame enable */

#define SAPPHIRE_RDB_PB_CTL 0x19000UL
#define SAPPHIRE_RDB_PB_CTL_RXEN 0x80000000UL

/** Transmit DMA Control Register */
#define SAPPHIRE_DMATXCTL 0x18000UL
#define SAPPHIRE_DMATXCTL_TE	0x00000001UL	/**< Transmit enable */

#define SAPPHIRE_MACTXCFG 0x11000UL
#define SAPPHIRE_MACTXCFG_TE 0x00000001UL
#define SAPPHIRE_MACTXCFG_SPEED_MASK     0x60000000UL
#define SAPPHIRE_MACTXCFG_SPEED_10G      0x00000000UL
#define SAPPHIRE_MACTXCFG_SPEED_1G       0x60000000UL



/** Transmit Descriptor register block */
#define SAPPHIRE_TD 0x03000UL


/** Maximum Frame Size Register */
#define SAPPHIRE_PSR_MAX_SZ 0x15020UL
#define SAPPHIRE_MAXFRS_MFS_DEFAULT \
		( ETH_FRAME_LEN + 4 /* VLAN */ + 4 /* CRC */ )


/** Link Status Register */
#define SAPPHIRE_CFG_PORT_ST 0x14404UL
#define SAPPHIRE_CFG_PORT_ST_LINK_UP       0x00000001	/**< Link up */

#define SAPPHIRE_GPIO_DR                   0x14800UL
#define SAPPHIRE_GPIO_DDR                  0x14804UL

#define SAPPHIRE_GPIO_DR_0         0x00000001U /* SDP0 Data Value */
#define SAPPHIRE_GPIO_DR_1         0x00000002U /* SDP1 Data Value */
#define SAPPHIRE_GPIO_DR_2         0x00000004U /* SDP2 Data Value */
#define SAPPHIRE_GPIO_DR_3         0x00000008U /* SDP3 Data Value */
#define SAPPHIRE_GPIO_DR_4         0x00000010U /* SDP4 Data Value */
#define SAPPHIRE_GPIO_DR_5         0x00000020U /* SDP5 Data Value */
#define SAPPHIRE_GPIO_DR_6         0x00000040U /* SDP6 Data Value */
#define SAPPHIRE_GPIO_DR_7         0x00000080U /* SDP7 Data Value */
#define SAPPHIRE_GPIO_DDR_0        0x00000001U /* SDP0 IO direction */
#define SAPPHIRE_GPIO_DDR_1        0x00000002U /* SDP1 IO direction */
#define SAPPHIRE_GPIO_DDR_2        0x00000004U /* SDP1 IO direction */
#define SAPPHIRE_GPIO_DDR_3        0x00000008U /* SDP3 IO direction */
#define SAPPHIRE_GPIO_DDR_4        0x00000010U /* SDP4 IO direction */
#define SAPPHIRE_GPIO_DDR_5        0x00000020U /* SDP5 IO direction */
#define SAPPHIRE_GPIO_DDR_6        0x00000040U /* SDP6 IO direction */
#define SAPPHIRE_GPIO_DDR_7        0x00000080U /* SDP7 IO direction */

#define SAPPHIRE_LINK_SPEED_1GB_FULL       2
#define SAPPHIRE_LINK_SPEED_10GB_FULL      4

#define SAPPHIRE_XPCS_POWER_GOOD_MAX_POLLING_TIME 100

#define SAPPHIRE_VR_XS_OR_PCS_MMD_DIGI_CTL1        	0x38000
#define SAPPHIRE_VR_XS_OR_PCS_MMD_DIGI_STATUS 		0x38010

#define SAPPHIRE_VR_XS_OR_PCS_MMD_DIGI_STATUS_PSEQ_MASK            0x1C
#define SAPPHIRE_VR_XS_OR_PCS_MMD_DIGI_STATUS_PSEQ_POWER_GOOD      0x10

#define SAPPHIRE_XPCS_IDA_ADDR    0x13000
#define SAPPHIRE_XPCS_IDA_DATA    0x13004

#define SAPPHIRE_FAILED_READ_REG       0xffffffffU

#define SAPPHIRE_MAC_TX_CFG                0x11000
#define SAPPHIRE_MAC_TX_CFG_TE             0x00000001U

#define SAPPHIRE_SR_AN_MMD_CTL                     0x70000
#define SAPPHIRE_SR_PCS_CTL2                       0x30007
#define SAPPHIRE_SR_PMA_MMD_CTL1                   0x10000
#define SAPPHIRE_SR_MII_MMD_CTL                    0x1F0000
#define SAPPHIRE_SR_MII_MMD_AN_CTL                 0x1F8001


#define SAPPHIRE_PHY_TX_GENCTRL1                   0x18031
#define SAPPHIRE_PHY_MISC_CTL0                     0x18090
#define SAPPHIRE_PHY_TX_EQ_CTL0                    0x18036
#define SAPPHIRE_PHY_TX_EQ_CTL1                    0x18037
#define SAPPHIRE_PHY_RX_EQ_ATT_LVL0                0x18057
#define SAPPHIRE_PHY_RX_EQ_CTL0                    0x18058
#define SAPPHIRE_PHY_DFE_TAP_CTL0                  0x1805E
#define SAPPHIRE_PHY_RX_GEN_CTL3                   0x18053
#define SAPPHIRE_PHY_MPLLA_CTL0                    0x18071
#define SAPPHIRE_PHY_MPLLA_CTL3                    0x18077
#define SAPPHIRE_PHY_VCO_CAL_LD0                   0x18092
#define SAPPHIRE_PHY_VCO_CAL_REF0                  0x18096
#define SAPPHIRE_PHY_AFE_DFE_ENABLE                0x1805D
#define SAPPHIRE_PHY_RX_EQ_CTL                     0x1805C
#define SAPPHIRE_PHY_TX_RATE_CTL                   0x18034
#define SAPPHIRE_PHY_RX_RATE_CTL                   0x18054
#define SAPPHIRE_PHY_TX_GEN_CTL2                   0x18032
#define SAPPHIRE_PHY_RX_GEN_CTL2                   0x18052
#define SAPPHIRE_PHY_MPLLA_CTL2                    0x18073
#define SAPPHIRE_PHY_RX_AD_ACK                     0x18098

#define SAPPHIRE_PHY_INIT_DONE_POLLING_TIME        100

#define SAPPHIRE_VR_XS_OR_PCS_MMD_DIGI_CTL1_VR_RST 0x8000

#define SAPPHIRE_I2C_ENABLE                0x1496C /* I2C Enable */
#define SAPPHIRE_I2C_CON                   0x14900 /* I2C Control */
#define SAPPHIRE_I2C_INTR_MASK             0x14930 /* I2C Interrupt Mask */
#define SAPPHIRE_I2C_TAR                   0x14904 /* I2C Target Address */
#define SAPPHIRE_I2C_DATA_CMD              0x14910 /* I2C Rx/Tx Data Buf and Cmd */
#define SAPPHIRE_I2C_SS_SCL_HCNT           0x14914 /* Standard speed I2C Clock SCL
                                                 * High Count */
#define SAPPHIRE_I2C_SS_SCL_LCNT           0x14918 /* Standard speed I2C Clock SCL
                                                 * Low Count */
#define SAPPHIRE_I2C_RAW_INTR_STAT         0x14934 /* I2C Raw Interrupt Status */
#define SAPPHIRE_I2C_RX_TL                 0x14938 /* I2C Receive FIFO Threshold */
#define SAPPHIRE_I2C_TX_TL                 0x1493C /* I2C TX FIFO Threshold */
#define SAPPHIRE_I2C_SCL_STUCK_TIMEOUT     0x149AC /* I2C SCL stuck at low timeout
                                                 * register */
#define SAPPHIRE_I2C_SDA_STUCK_TIMEOUT     0x149B0 /*I2C SDA Stuck at Low Timeout*/

#define SAPPHIRE_I2C_CON_SLAVE_DISABLE     ((1 << 6))
#define SAPPHIRE_I2C_CON_RESTART_EN        ((1 << 5))
#define SAPPHIRE_I2C_CON_SPEED(_v)         (((_v) & 0x3) << 1)
#define SAPPHIRE_I2C_CON_MASTER_MODE       ((1 << 0))
#define SAPPHIRE_I2C_INTR_STAT_RX_FULL     ((0x1) << 2)
#define SAPPHIRE_I2C_INTR_STAT_TX_EMPTY    ((0x1) << 4)
#define SAPPHIRE_I2C_DATA_CMD_STOP         ((1 << 9))
#define SAPPHIRE_I2C_DATA_CMD_READ         ((1 << 8) | SAPPHIRE_I2C_DATA_CMD_STOP)

#define SAPPHIRE_SFF_IDENTIFIER            0x0
#define SAPPHIRE_SFF_IDENTIFIER_SFP        0x3
#define SAPPHIRE_SFF_1GBE_COMP_CODES       0x6
#define SAPPHIRE_SFF_10GBE_COMP_CODES      0x3
#define SAPPHIRE_SFF_CABLE_TECHNOLOGY      0x8
#define SAPPHIRE_SFF_DA_PASSIVE_CABLE      0x4

#define SAPPHIRE_I2C_TIMEOUT  1000
#define SAPPHIRE_I2C_SLAVE_ADDR            (0x50)
#define SAPPHIRE_I2C_EEPROM_DEV_ADDR       0xA0

enum sapphire_sfp_type {
	sapphire_sfp_type_da_cu_core0 = 3,
	sapphire_sfp_type_unknown = 100
};

#define usec_delay(x) udelay(x)
#define msleep(x) mdelay(x)

#define SAPPHIRE_ERR									100
#define SAPPHIRE_ERR_INVALID_PARAM					-(SAPPHIRE_ERR+0)
#define SAPPHIRE_ERR_TIMEOUT						-(SAPPHIRE_ERR+1)
#define SAPPHIRE_ERR_I2C							-(SAPPHIRE_ERR+2)
#define SAPPHIRE_ERR_I2C_SFP_NOT_SUPPORTED			-(SAPPHIRE_ERR+3)
#define SAPPHIRE_ERR_NOT_SFP						-(SAPPHIRE_ERR+4)


#endif /* _SAPPHIRE_H */

