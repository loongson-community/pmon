#include <sys/linux/types.h>

/* MAC address length */
#ifndef MAC_ADDR_LEN
#define MAC_ADDR_LEN	6
#endif

#ifndef MAC_PROTOCOL_LEN
#define MAC_PROTOCOL_LEN	2
#endif

#define Reserved2_data	7
#define RX_DMA_BURST	7	/* Maximum PCI burst, '6' is 1024 */
#define TX_DMA_BURST_unlimited	7
#define TX_DMA_BURST_1024	6
#define TX_DMA_BURST_512	5
#define TX_DMA_BURST_256	4
#define TX_DMA_BURST_128	3
#define TX_DMA_BURST_64		2
#define TX_DMA_BURST_32		1
#define TX_DMA_BURST_16		0
#define Reserved1_data 	0x3F
#define RxPacketMaxSize	0x3FE8	/* 16K - 1 - ETH_HLEN - VLAN - CRC... */
#define Jumbo_Frame_2k	(2 * 1024)
#define Jumbo_Frame_3k	(3 * 1024)
#define Jumbo_Frame_4k	(4 * 1024)
#define Jumbo_Frame_5k	(5 * 1024)
#define Jumbo_Frame_6k	(6 * 1024)
#define Jumbo_Frame_7k	(7 * 1024)
#define Jumbo_Frame_8k	(8 * 1024)
#define Jumbo_Frame_9k	(9 * 1024)
#define InterFrameGap	0x03	/* 3 means InterFrameGap = the shortest one */

#define R8168_REGS_SIZE		256
#define R8168_NAPI_WEIGHT	64

#define RX_BUF_SIZE	0x05F3	/* 0x05F3 = 1522bye + 1 */
#define TX_BUF_SIZE	0x05F3	/* 0x05F3 = 1522bye + 1 */
#define R8168_TX_RING_BYTES	(NUM_TX_DESC * sizeof(struct TxDesc))
#define R8168_RX_RING_BYTES	(NUM_RX_DESC * sizeof(struct RxDesc))

#define RTL8168_TX_TIMEOUT	(6 * HZ)
#define RTL8168_LINK_TIMEOUT	(1 * HZ)
#define RTL8168_ESD_TIMEOUT	(2 * HZ)
#if 0
#define NUM_TX_DESC	1024	/* Number of Tx descriptor registers */
#define NUM_RX_DESC	1024	/* Number of Rx descriptor registers */
#else
#define NUM_TX_DESC	64	/* Number of Tx descriptor registers */
#define NUM_RX_DESC	64	/* Number of Rx descriptor registers */
#endif
#define NODE_ADDRESS_SIZE 6

/* write/read MMIO register */
#define writeb(val, addr) (*(volatile u8*)(addr) = (val))
#define writew(val, addr) (*(volatile u16*)(addr) = (val))
#define writel(val, addr) (*(volatile u32*)(addr) = (val))
#define readb(addr) (*(volatile u8*)(addr))
#define readw(addr) (*(volatile u16*)(addr))
#define readl(addr) (*(volatile u32*)(addr))

#define RTL_W8(tp,reg, val8)	writeb ((val8), tp->ioaddr + (reg))
#define RTL_W16(tp,reg, val16)	writew ((val16), tp->ioaddr + (reg))
#define RTL_W32(tp,reg, val32)	writel ((val32), tp->ioaddr + (reg))
#define RTL_R8(tp,reg)		readb (tp->ioaddr + (reg))
#define RTL_R16(tp,reg)		readw (tp->ioaddr + (reg))
#define RTL_R32(tp,reg)		((unsigned long) readl (tp->ioaddr + (reg)))

#define RTL8168_DEBUG 
#ifdef RTL8168_DEBUG
#define dprintk(fmt, args...)  do { printf(fmt,## args); } while (0)
#else
#define dprintk(fmt, args...)  do { } while (0)
#endif 

#define R8168_MSG_DEFAULT \
    (NETIF_MSG_DRV | NETIF_MSG_PROBE | NETIF_MSG_IFUP | NETIF_MSG_IFDOWN)

#define TX_BUFFS_AVAIL(tp) \
    (tp->dirty_tx + NUM_TX_DESC - tp->cur_tx - 1)

#ifndef	DMA_64BIT_MASK
#define DMA_64BIT_MASK	0xffffffffffffffffULL
#endif

#ifndef	DMA_32BIT_MASK
#define DMA_32BIT_MASK	0xffffffffUL
#endif

#ifndef	NETDEV_TX_OK
#define NETDEV_TX_OK 0		/* driver took care of packet */
#endif

#ifndef	NETDEV_TX_BUSY
#define NETDEV_TX_BUSY 1	/* driver tx path was busy*/
#endif

#ifndef	NETDEV_TX_LOCKED
#define NETDEV_TX_LOCKED -1	/* driver tx lock was already taken */
#endif

#ifndef	ADVERTISED_Pause
#define ADVERTISED_Pause	(1 << 13)
#endif

#ifndef	ADVERTISED_Asym_Pause
#define ADVERTISED_Asym_Pause	(1 << 14)
#endif

#ifndef	ADVERTISE_PAUSE_CAP
#define ADVERTISE_PAUSE_CAP	0x400
#endif

#ifndef	ADVERTISE_PAUSE_ASYM
#define ADVERTISE_PAUSE_ASYM	0x800
#endif

#ifndef	MII_CTRL1000
#define MII_CTRL1000		0x09
#endif

#ifndef	ADVERTISE_1000FULL
#define ADVERTISE_1000FULL	0x200
#endif

#ifndef	ADVERTISE_1000HALF
#define ADVERTISE_1000HALF	0x100
#endif

#define PCI_PM_CTRL 0x04
#define  PCI_PM_CTRL_STATE_MASK 0x0003  /* Current power state (D0 to D3) */
#define  PCI_PM_CTRL_NO_SOFT_RESET  0x0004  /* No reset for D3hot->D0 */
#define  PCI_PM_CTRL_PME_ENABLE 0x0100  /* PME pin enable */
#define  PCI_PM_CTRL_DATA_SEL_MASK  0x1e00  /* Data select (??) */
#define  PCI_PM_CTRL_DATA_SCALE_MASK    0x6000  /* Data scale (??) */
#define  PCI_PM_CTRL_PME_STATUS 0x8000  /* PME pin status */
#define PCI_PM_PPB_EXTENSIONS   6   /* PPB support extensions (??) */
#define  PCI_PM_PPB_B2_B3   0x40    /* Stop clock when in D3hot (??) */
#define  PCI_PM_BPCC_ENABLE 0x80    /* Bus power/clock control enable (??) */
#define PCI_PM_DATA_REGISTER    7   /* (??) */
#define PCI_PM_SIZEOF       8

static int if_in_attach;
static int if_frequency;

enum _DescStatusBit {
	DescOwn		= (1 << 31), /* Descriptor is owned by NIC */
	RingEnd		= (1 << 30), /* End of descriptor ring */
	FirstFrag	= (1 << 29), /* First segment of a packet */
	LastFrag	= (1 << 28), /* Final segment of a packet */

	/* Tx private */
	/*------ offset 0 of tx descriptor ------*/
	LargeSend	= (1 << 27), /* TCP Large Send Offload (TSO) */
	MSSShift	= 16,        /* MSS value position */
	MSSMask		= 0xfff,     /* MSS value + LargeSend bit: 12 bits */
	TxIPCS		= (1 << 18), /* Calculate IP checksum */
	TxUDPCS		= (1 << 17), /* Calculate UDP/IP checksum */
	TxTCPCS		= (1 << 16), /* Calculate TCP/IP checksum */
	TxVlanTag	= (1 << 17), /* Add VLAN tag */

	/*@@@@@@ offset 4 of tx descriptor => bits for RTL8168C/CP only		begin @@@@@@*/
	TxUDPCS_C	= (1 << 31), /* Calculate UDP/IP checksum */
	TxTCPCS_C	= (1 << 30), /* Calculate TCP/IP checksum */
	TxIPCS_C	= (1 << 29), /* Calculate IP checksum */
	/*@@@@@@ offset 4 of tx descriptor => bits for RTL8168C/CP only		end @@@@@@*/


	/* Rx private */
	/*------ offset 0 of rx descriptor ------*/
	PID1		= (1 << 18), /* Protocol ID bit 1/2 */
	PID0		= (1 << 17), /* Protocol ID bit 2/2 */

#define RxProtoUDP	(PID1)
#define RxProtoTCP	(PID0)
#define RxProtoIP	(PID1 | PID0)
#define RxProtoMask	RxProtoIP

	RxIPF		= (1 << 16), /* IP checksum failed */
	RxUDPF		= (1 << 15), /* UDP/IP checksum failed */
	RxTCPF		= (1 << 14), /* TCP/IP checksum failed */
	RxVlanTag	= (1 << 16), /* VLAN tag available */

	/*@@@@@@ offset 0 of rx descriptor => bits for RTL8168C/CP only		begin @@@@@@*/
	RxUDPT		= (1 << 18),
	RxTCPT		= (1 << 17),
	/*@@@@@@ offset 0 of rx descriptor => bits for RTL8168C/CP only		end @@@@@@*/

	/*@@@@@@ offset 4 of rx descriptor => bits for RTL8168C/CP only		begin @@@@@@*/
	RxV6F		= (1 << 31),
	RxV4F		= (1 << 30),
	/*@@@@@@ offset 4 of rx descriptor => bits for RTL8168C/CP only		end @@@@@@*/
};

enum features {
//	RTL_FEATURE_WOL	= (1 << 0),
	RTL_FEATURE_MSI	= (1 << 1),
};

enum wol_capability {
	WOL_DISABLED = 0,
	WOL_ENABLED = 1
};

enum bits {
	BIT_0 = (1 << 0),
	BIT_1 = (1 << 1),
	BIT_2 = (1 << 2),
	BIT_3 = (1 << 3),
	BIT_4 = (1 << 4),
	BIT_5 = (1 << 5),
	BIT_6 = (1 << 6),
	BIT_7 = (1 << 7),
	BIT_8 = (1 << 8),
	BIT_9 = (1 << 9),
	BIT_10 = (1 << 10),
	BIT_11 = (1 << 11),
	BIT_12 = (1 << 12),
	BIT_13 = (1 << 13),
	BIT_14 = (1 << 14),
	BIT_15 = (1 << 15),
	BIT_16 = (1 << 16),
	BIT_17 = (1 << 17),
	BIT_18 = (1 << 18),
	BIT_19 = (1 << 19),
	BIT_20 = (1 << 20),
	BIT_21 = (1 << 21),
	BIT_22 = (1 << 22),
	BIT_23 = (1 << 23),
	BIT_24 = (1 << 24),
	BIT_25 = (1 << 25),
	BIT_26 = (1 << 26),
	BIT_27 = (1 << 27),
	BIT_28 = (1 << 28),
	BIT_29 = (1 << 29),
	BIT_30 = (1 << 30),
	BIT_31 = (1 << 31)
};

enum effuse {
	EFUSE_SUPPORT = 1,
	EFUSE_NOT_SUPPORT = 0,
};
#define RsvdMask	0x3fffc000


//typedef unsigned long long u64;

struct TxDesc {
	u32 opts1;
	u32 opts2;
       u64 addr;
};

struct RxDesc {
	u32 opts1;
	u32 opts2;
       u64 addr;
};


struct ring_info {
	struct mbuf *mb;
	u32		len;
	u8		__pad[sizeof(void *) - sizeof(u32)];
};

struct ethtool_cmd {
	u32   cmd;
	u32   supported;  /* Features this interface supports */
	u32   advertising;    /* Features this interface advertises */
	u16   speed;      /* The forced speed, 10Mb, 100Mb, gigabit */
	u8    duplex;     /* Duplex, half or full */
	u8    port;       /* Which connector port */
	u8    phy_address;
	u8    transceiver;    /* Which transceiver to use */
	u8    autoneg;    /* Enable or disable autonegotiation */
	u32   maxtxpkt;   /* Tx pkts before generating tx int */
	u32   maxrxpkt;   /* Rx pkts before generating rx int */
	u32   reserved[4];
};

struct net_device_stats
{
	unsigned long	rx_packets;		/* total packets received	*/
	unsigned long	tx_packets;		/* total packets transmitted	*/
	unsigned long	rx_bytes;		/* total bytes received 	*/
	unsigned long	tx_bytes;		/* total bytes transmitted	*/
	unsigned long	rx_errors;		/* bad packets received		*/
	unsigned long	tx_errors;		/* packet transmit problems	*/
	unsigned long	rx_dropped;		/* no space in linux buffers	*/
	unsigned long	tx_dropped;		/* no space available in linux	*/
	unsigned long	multicast;		/* multicast packets received	*/
	unsigned long	collisions;

	/* detailed rx_errors: */
	unsigned long	rx_length_errors;
	unsigned long	rx_over_errors;		/* receiver ring buff overflow	*/
	unsigned long	rx_crc_errors;		/* recved pkt with crc error	*/
	unsigned long	rx_frame_errors;	/* recv'd frame alignment error */
	unsigned long	rx_fifo_errors;		/* recv'r fifo overrun		*/
	unsigned long	rx_missed_errors;	/* receiver missed packet	*/

	/* detailed tx_errors */
	unsigned long	tx_aborted_errors;
	unsigned long	tx_carrier_errors;
	unsigned long	tx_fifo_errors;
	unsigned long	tx_heartbeat_errors;
	unsigned long	tx_window_errors;
	
	/* for cslip etc */
	unsigned long	rx_compressed;
	unsigned long	tx_compressed;
};

enum RTL8168_DSM_STATE {
	DSM_MAC_INIT = 1,
	DSM_NIC_GOTO_D3 = 2,
	DSM_IF_DOWN = 3,
	DSM_NIC_RESUME_D3 = 4,
	DSM_IF_UP = 5,
};

struct pci_resource {
    u8  cmd;
    u8  cls;
    u16 io_base_h;
    u16 io_base_l;
    u16 mem_base_h;
    u16 mem_base_l;
    u8  ilr;
    u16 resv_0x20_h;
    u16 resv_0x20_l;
    u16 resv_0x24_h;
    u16 resv_0x24_l;
};


struct rtl8168_private {
	struct device sc_dev;		/* generic device structures */
	void *sc_ih;			/* interrupt handler cookie */
	bus_space_tag_t sc_st;		/* bus space tag */
	bus_space_handle_t sc_sh;	/* bus space handle */
	pci_chipset_tag_t sc_pc;	/* chipset handle needed by mips */

	struct pci_attach_args *pa; 

	struct arpcom arpcom;		/* per-interface network data !!we use this*/
	struct mii_data sc_mii;		/* MII media information */
	unsigned char   dev_addr[6]; //the net interface's address
	unsigned long ioaddr;

	struct net_device_stats stats;
	int flags;
	int mc_count;
	void *mmio_addr;	/* memory map physical address */
	u32 msg_enable;
	u32 tx_tcp_csum_cmd;
	u32 tx_udp_csum_cmd;
	u32 tx_ip_csum_cmd;
	int max_jumbo_frame_size;
	int chipset;
	int mcfg;
	int phy_version;
	u32 cur_rx; /* Index into the Rx descriptor buffer of next Rx pkt. */
	u32 cur_tx; /* Index into the Tx descriptor buffer of next Rx pkt. */
	u32 dirty_rx;
	u32 dirty_tx;
	struct TxDesc *TxDescArray;	/* 256-aligned Tx descriptor ring */
	struct RxDesc *RxDescArray;	/* 256-aligned Rx descriptor ring */
	unsigned long TxPhyAddr;
	unsigned long RxPhyAddr;
	unsigned char *rx_buffer[NUM_RX_DESC];	/* Rx data buffers */
	unsigned char *tx_buffer[NUM_TX_DESC];	/* Tx data buffers */
	unsigned rx_buf_sz;
	unsigned tx_buf_sz;
	int rx_fifo_overflow;
	struct pci_resource pci_cfg_space;
	unsigned int pci_cfg_is_read;
	unsigned int rtl8168_rx_config;
	u16 cp_cmd;
	u16 intr_mask;
	int phy_auto_nego_reg;
	int phy_1000_ctrl_reg;
#ifdef CONFIG_R8168_VLAN
	struct vlan_group *vlgrp;
#endif
	u8 autoneg;
	u16 speed;
	u8 duplex;
	int (*set_speed)(struct rtl8168_private *, u8 autoneg, u16 speed, u8 duplex);
	void (*get_settings)(struct rtl8168_private *, struct ethtool_cmd *);
	void (*phy_reset_enable)(struct rtl8168_private *);
	unsigned int (*phy_reset_pending)(struct rtl8168_private *);
	unsigned int (*link_ok)(struct rtl8168_private *);
	unsigned wol_enabled : 1;
	unsigned features;
	int efuse;
};

enum mcfg {
	CFG_METHOD_1 = 0x01,
	CFG_METHOD_2 = 0x02,
	CFG_METHOD_3 = 0x03,
	CFG_METHOD_4 = 0x04,
	CFG_METHOD_5 = 0x05,
	CFG_METHOD_6 = 0x06,
	CFG_METHOD_7 = 0x07,
	CFG_METHOD_8 = 0x08,
	CFG_METHOD_9 = 0x09,
	CFG_METHOD_10 = 0x0a,
	CFG_METHOD_11 = 0x0b,
};

enum RTL8168_registers {
	MAC0			= 0x00,		/* Ethernet hardware address. */
	MAC4			= 0x04,
	MAR0			= 0x08,		/* Multicast filter. */
	CounterAddrLow		= 0x10,
	CounterAddrHigh		= 0x14,
	TxDescStartAddrLow	= 0x20,
	TxDescStartAddrHigh	= 0x24,
	TxHDescStartAddrLow	= 0x28,
	TxHDescStartAddrHigh	= 0x2c,
	FLASH			= 0x30,
	ERSR			= 0x36,
	ChipCmd			= 0x37,
	TxPoll			= 0x38,
	IntrMask		= 0x3C,
	IntrStatus		= 0x3E,
	TxConfig		= 0x40,
	RxConfig		= 0x44,
	TCTR			= 0x48,
	Cfg9346			= 0x50,
	Config0			= 0x51,
	Config1			= 0x52,
	Config2			= 0x53,
	Config3			= 0x54,
	Config4			= 0x55,
	Config5			= 0x56,
	TimeIntr		= 0x58,
	PHYAR			= 0x60,
	CSIDR			= 0x64,
	CSIAR			= 0x68,
	PHYstatus		= 0x6C,
	MACDBG			= 0x6D,
	GPIO			= 0x6E,
	PMCH			= 0x6F,
	ERIDR			= 0x70,
	ERIAR			= 0x74,
	EPHYAR			= 0x80,
	OCPDR			= 0xB0,
	OCPAR			= 0xB4,
	DBG_reg			= 0xD1,
	RxMaxSize		= 0xDA,
	EFUSEAR			= 0xDC,
	CPlusCmd		= 0xE0,
	IntrMitigate		= 0xE2,
	RxDescAddrLow		= 0xE4,
	RxDescAddrHigh		= 0xE8,
	Reserved1		= 0xEC,
	FuncEvent		= 0xF0,
	FuncEventMask		= 0xF4,
	FuncPresetState		= 0xF8,
	FuncForceEvent		= 0xFC,
};
enum RTL8168_register_content {
	/* InterruptStatusBits */
	SYSErr		= 0x8000,
	PCSTimeout	= 0x4000,
	SWInt		= 0x0100,
	TxDescUnavail	= 0x0080,
	RxFIFOOver	= 0x0040,
	LinkChg		= 0x0020,
	RxDescUnavail	= 0x0010,
	TxErr		= 0x0008,
	TxOK		= 0x0004,
	RxErr		= 0x0002,
	RxOK		= 0x0001,

	/* RxStatusDesc */
	RxRWT = (1 << 22),
	RxRES = (1 << 21),
	RxRUNT = (1 << 20),
	RxCRC = (1 << 19),

	/* ChipCmdBits */
	StopReq  = 0x80,
	CmdReset = 0x10,
	CmdRxEnb = 0x08,
	CmdTxEnb = 0x04,
	RxBufEmpty = 0x01,

	/* Cfg9346Bits */
	Cfg9346_Lock = 0x00,
	Cfg9346_Unlock = 0xC0,

	/* rx_mode_bits */
	AcceptErr = 0x20,
	AcceptRunt = 0x10,
	AcceptBroadcast = 0x08,
	AcceptMulticast = 0x04,
	AcceptMyPhys = 0x02,
	AcceptAllPhys = 0x01,

	/* Transmit Priority Polling*/
	HPQ = 0x80,
	NPQ = 0x40,
	FSWInt = 0x01,

	/* RxConfigBits */
	Reserved2_shift = 13,
	RxCfgDMAShift = 8,
	RxCfg_128_int_en = (1 << 15),
	RxCfg_fet_multi_en = (1 << 14),
	RxCfg_half_refetch = (1 << 13),

	/* TxConfigBits */
	TxInterFrameGapShift = 24,
	TxDMAShift = 8,	/* DMA burst value (0-7) is shift this many bits */
	TxMACLoopBack = (1 << 17),	/* MAC loopback */

	/* Config1 register p.24 */
	LEDS1		= (1 << 7),
	LEDS0		= (1 << 6),
	Speed_down	= (1 << 4),
	MEMMAP		= (1 << 3),
	IOMAP		= (1 << 2),
	VPD		= (1 << 1),
	PMEnable	= (1 << 0),	/* Power Management Enable */

	/* Config3 register */
	MagicPacket	= (1 << 5),	/* Wake up when receives a Magic Packet */
	LinkUp		= (1 << 4),	/* This bit is reserved in RTL8168B.*/
					/* Wake up when the cable connection is re-established */
	ECRCEN		= (1 << 3),	/* This bit is reserved in RTL8168B*/
	Jumbo_En0	= (1 << 2),	/* This bit is reserved in RTL8168B*/
	RDY_TO_L23	= (1 << 1),	/* This bit is reserved in RTL8168B*/
	Beacon_en	= (1 << 0),	/* This bit is reserved in RTL8168B*/

	/* Config4 register */
	Jumbo_En1	= (1 << 1),	/* This bit is reserved in RTL8168B*/

	/* Config5 register */
	BWF		= (1 << 6),	/* Accept Broadcast wakeup frame */
	MWF		= (1 << 5),	/* Accept Multicast wakeup frame */
	UWF		= (1 << 4),	/* Accept Unicast wakeup frame */
	LanWake		= (1 << 1),	/* LanWake enable/disable */
	PMEStatus	= (1 << 0),	/* PME status can be reset by PCI RST# */

	/* CPlusCmd */
	EnableBist	= (1 << 15),
	Macdbgo_oe	= (1 << 14),
	Normal_mode	= (1 << 13),
	Force_halfdup	= (1 << 12),
	Force_rxflow_en	= (1 << 11),
	Force_txflow_en	= (1 << 10),
	Cxpl_dbg_sel	= (1 << 9),//This bit is reserved in RTL8168B
	ASF		= (1 << 8),//This bit is reserved in RTL8168C
	PktCntrDisable	= (1 << 7),
	RxVlan		= (1 << 6),
	RxChkSum	= (1 << 5),
	Macdbgo_sel	= 0x001C,
	INTT_0		= 0x0000,
	INTT_1		= 0x0001,
	INTT_2		= 0x0002,
	INTT_3		= 0x0003,
	
	/* rtl8168_PHYstatus */
	TxFlowCtrl = 0x40,
	RxFlowCtrl = 0x20,
	_1000bpsF = 0x10,
	_100bps = 0x08,
	_10bps = 0x04,
	LinkStatus = 0x02,
	FullDup = 0x01,

	/* DBG_reg */
	Fix_Nak_1 = (1 << 4),
	Fix_Nak_2 = (1 << 3),
	DBGPIN_E2 = (1 << 0),

	/* DumpCounterCommand */
	CounterDump = 0x8,

	/* PHY access */
	PHYAR_Flag = 0x80000000,
	PHYAR_Write = 0x80000000,
	PHYAR_Read = 0x00000000,
	PHYAR_Reg_Mask = 0x1f,
	PHYAR_Reg_shift = 16,
	PHYAR_Data_Mask = 0xffff,
	//PHY_CTRL_REG = 0x0;

	/* EPHY access */
	EPHYAR_Flag = 0x80000000,
	EPHYAR_Write = 0x80000000,
	EPHYAR_Read = 0x00000000,
	EPHYAR_Reg_Mask = 0x1f,
	EPHYAR_Reg_shift = 16,
	EPHYAR_Data_Mask = 0xffff,

	/* CSI access */	
	CSIAR_Flag = 0x80000000,
	CSIAR_Write = 0x80000000,
	CSIAR_Read = 0x00000000,
	CSIAR_ByteEn = 0x0f,
	CSIAR_ByteEn_shift = 12,
	CSIAR_Addr_Mask = 0x0fff,

	/* ERI access */
	ERIAR_Flag = 0x80000000,
	ERIAR_Write = 0x80000000,
	ERIAR_Read = 0x00000000,
	ERIAR_Addr_Align = 4, /* ERI access register address must be 4 byte alignment */
	ERIAR_ExGMAC = 0,
	ERIAR_MSIX = 1,
	ERIAR_ASF = 2,
	ERIAR_Type_shift = 16,
	ERIAR_ByteEn = 0x0f,
	ERIAR_ByteEn_shift = 12,

	/* E-FUSE access */
	EFUSE_WRITE	= 0x80000000,
	EFUSE_WRITE_OK	= 0x00000000,
	EFUSE_READ	= 0x00000000,
	EFUSE_READ_OK	= 0x80000000,
	EFUSE_Reg_Mask	= 0x03FF,
	EFUSE_Reg_Shift	= 8,
	EFUSE_Check_Cnt	= 300,
	EFUSE_READ_FAIL	= 0xFF,
	EFUSE_Data_Mask	= 0x000000FF,

	/* GPIO */
	GPIO_en = (1 << 0),	

};


/* Indicates what features are supported by the interface. */
#define SUPPORTED_10baseT_Half		(1 << 0)
#define SUPPORTED_10baseT_Full		(1 << 1)
#define SUPPORTED_100baseT_Half		(1 << 2)
#define SUPPORTED_100baseT_Full		(1 << 3)
#define SUPPORTED_1000baseT_Half	(1 << 4)
#define SUPPORTED_1000baseT_Full	(1 << 5)
#define SUPPORTED_Autoneg		(1 << 6)
#define SUPPORTED_TP			(1 << 7)
#define SUPPORTED_AUI			(1 << 8)
#define SUPPORTED_MII			(1 << 9)
#define SUPPORTED_FIBRE			(1 << 10)
#define SUPPORTED_BNC			(1 << 11)
#define SUPPORTED_10000baseT_Full	(1 << 12)
#define SUPPORTED_Pause			(1 << 13)
#define SUPPORTED_Asym_Pause		(1 << 14)
#define SUPPORTED_2500baseX_Full	(1 << 15)

/* Indicates what features are advertised by the interface. */
#define ADVERTISED_10baseT_Half		(1 << 0)
#define ADVERTISED_10baseT_Full		(1 << 1)
#define ADVERTISED_100baseT_Half	(1 << 2)
#define ADVERTISED_100baseT_Full	(1 << 3)
#define ADVERTISED_1000baseT_Half	(1 << 4)
#define ADVERTISED_1000baseT_Full	(1 << 5)
#define ADVERTISED_Autoneg		(1 << 6)
#define ADVERTISED_TP			(1 << 7)
#define ADVERTISED_AUI			(1 << 8)
#define ADVERTISED_MII			(1 << 9)
#define ADVERTISED_FIBRE		(1 << 10)
#define ADVERTISED_BNC			(1 << 11)
#define ADVERTISED_10000baseT_Full	(1 << 12)
#define ADVERTISED_Pause		(1 << 13)
#define ADVERTISED_Asym_Pause		(1 << 14)
#define ADVERTISED_2500baseX_Full	(1 << 15)

/* The following are all involved in forcing a particular link
 * mode for the device for setting things.  When getting the
 * devices settings, these indicate the current mode and whether
 * it was foced up into this mode or autonegotiated.
 */


/* The forced speed, 10Mb, 100Mb, gigabit, 2.5Gb, 10GbE. */
#define SPEED_10		10
#define SPEED_100		100
#define SPEED_1000		1000
#define SPEED_2500		2500
#define SPEED_10000		10000

/* Duplex, half or full. */
#define DUPLEX_HALF		0x00
#define DUPLEX_FULL		0x01

/* Which connector port. */
#define PORT_TP			0x00
#define PORT_AUI		0x01
#define PORT_MII		0x02
#define PORT_FIBRE		0x03
#define PORT_BNC		0x04

/* Which transceiver to use. */
#define XCVR_INTERNAL		0x00
#define XCVR_EXTERNAL		0x01
#define XCVR_DUMMY1		0x02
#define XCVR_DUMMY2		0x03
#define XCVR_DUMMY3		0x04

/* Enable or disable autonegotiation.  If this is set to enable,
 * the forced link modes above are completely ignored.
 */
#define AUTONEG_DISABLE		0x00
#define AUTONEG_ENABLE		0x01

/* Wake-On-Lan options. */
#define WAKE_PHY		(1 << 0)
#define WAKE_UCAST		(1 << 1)
#define WAKE_MCAST		(1 << 2)
#define WAKE_BCAST		(1 << 3)
#define WAKE_ARP		(1 << 4)
#define WAKE_MAGIC		(1 << 5)
#define WAKE_MAGICSECURE	(1 << 6) /* only meaningful if WAKE_MAGIC */

/* L3-L4 network traffic flow types */
#define	TCP_V4_FLOW	0x01
#define	UDP_V4_FLOW	0x02
#define	SCTP_V4_FLOW	0x03
#define	AH_ESP_V4_FLOW	0x04
#define	TCP_V6_FLOW	0x05
#define	UDP_V6_FLOW	0x06
#define	SCTP_V6_FLOW	0x07
#define	AH_ESP_V6_FLOW	0x08

/* L3-L4 network traffic flow hash options */
#define	RXH_DEV_PORT	(1 << 0)
#define	RXH_L2DA	(1 << 1)
#define	RXH_VLAN	(1 << 2)
#define	RXH_L3_PROTO	(1 << 3)
#define	RXH_IP_SRC	(1 << 4)
#define	RXH_IP_DST	(1 << 5)
#define	RXH_L4_B_0_1	(1 << 6) /* src port in case of TCP/UDP/SCTP */
#define	RXH_L4_B_2_3	(1 << 7) /* dst port in case of TCP/UDP/SCTP */
#define	RXH_DISCARD	(1 << 31)

#define le32_to_cpu(x) (x)
#define le64_to_cpu(x) (x)
#define cpu_to_le32(x) (x)
#define cpu_to_le64(x) (x)

#define ETH_HLEN 14
#define ETH_ZLEN 60
#define 		NET_IP_ALIGN 2	

#define MAX_FRAGS  (65536/16*1024)

