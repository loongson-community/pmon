/************************************************************************

 Copyright (C)
 File name:     atl1c.h
 Author:        Version:  ***      Date: ***
 Description: Atheros's AR8131 ether apdater's driver of pmon,
              ported from linux.
 Others:        
 Function List:
 
 Revision History:
 
 -----------------------------------------------------------------------------------------------------------
  Date          Author          Activity ID     Activity Headline
  2010-08-25    QianYuli        PMON20100825    Created it
************************************************************************************/


#ifndef _ATL1C_H_
#define _ATL1C_H_
#include <sys/linux/types.h>
#include <machine/bus.h>

#define DEV_ID_ATL1E    	0x1026
#define DEV_ID_ATL1C    	0x1067   /* TODO change */
#define DEV_ID_ATL2C    	0x1066
#define DEV_ID_ATL1C_2_0 	0x1063
#define DEV_ID_ATL2C_2_0	0x1062
#define DEV_ID_ATL2C_B		0x2060
#define DEV_ID_ATL2C_B_2	0x2062
#define DEV_ID_ATL1D		0x1073
#define L2CB_V10		0xc0
#define L2CB_V11		0xc1

#define SPEED_0             0xffff
#define SPEED_10            10
#define SPEED_100           100
#define SPEED_1000          1000
#define HALF_DUPLEX         1
#define FULL_DUPLEX         2

#define MEDIA_TYPE_AUTO_SENSOR		0
#define MEDIA_TYPE_1000M_FULL		1
#define MEDIA_TYPE_100M_FULL		2	
#define MEDIA_TYPE_100M_HALF		3
#define MEDIA_TYPE_10M_FULL		4
#define MEDIA_TYPE_10M_HALF		5

#define ADVERTISE_10_HALF               0x0001
#define ADVERTISE_10_FULL               0x0002
#define ADVERTISE_100_HALF              0x0004
#define ADVERTISE_100_FULL              0x0008
#define ADVERTISE_1000_HALF             0x0010 /* Not used, just FYI */
#define ADVERTISE_1000_FULL             0x0020

#define AUTONEG_ADVERTISE_SPEED_DEFAULT 0x002F /* Everything but 1000-Half */
#define AUTONEG_ADVERTISE_10_100_ALL    0x000F /* All 10/100 speeds*/
#define AUTONEG_ADVERTISE_10_ALL        0x0003 /* 10Mbps Full & Half speeds*/

/* Error Codes */
#define AT_ERR_EEPROM      1
#define AT_ERR_PHY         2
#define AT_ERR_CONFIG      3
#define AT_ERR_PARAM       4
#define AT_ERR_MAC_TYPE    5
#define AT_ERR_PHY_TYPE    6
#define AT_ERR_PHY_SPEED   7
#define AT_ERR_PHY_RES     8
#define AT_ERR_TIMEOUT     9

#define AT_RX_BUF_SIZE		1536
#define MAX_JUMBO_FRAME_SIZE 	(9*1024)	
#define MAX_TSO_FRAME_SIZE	(7*1024)
#define MAX_TX_OFFLOAD_THRESH	(9*1024)

#define AT_MAX_RECEIVE_QUEUE    4
#define AT_DEF_RECEIVE_QUEUE	1
#define AT_MAX_TRANSMIT_QUEUE	2

#define AT_DMA_HI_ADDR_MASK     0xffffffff00000000ULL
#define AT_DMA_LO_ADDR_MASK     0x00000000ffffffffULL

#define AT_TX_WATCHDOG  (5 * HZ)
#define AT_MAX_INT_WORK		20
#define AT_TWSI_EEPROM_TIMEOUT 	100
#define AT_HW_MAX_IDLE_DELAY 	10
#define AT_SUSPEND_LINK_TIMEOUT 28

#define AT_ASPM_L0S_TIMER	6
#define AT_ASPM_L1_TIMER	12
#define AT_LCKDET_TIMER		12

#define ATL1C_PCIE_L0S_L1_DISABLE 	0x01
#define ATL1C_PCIE_PHY_RESET		0x02

#define ATL1C_ASPM_L0s_ENABLE		0x0001
#define ATL1C_ASPM_L1_ENABLE		0x0002

#define AT_REGS_LEN	75
#define AT_EEPROM_LEN 	512
#define AT_ADV_MASK	(ADVERTISE_10_HALF  |\
			 ADVERTISE_10_FULL  |\
			 ADVERTISE_100_HALF |\
			 ADVERTISE_100_FULL |\
			 ADVERTISE_1000_FULL)
#define ATL1C_GET_DESC(R, i, type)	(&(((type *)((R)->desc))[i]))
#define ATL1C_RFD_DESC(R, i)	ATL1C_GET_DESC(R, i, struct atl1c_rx_free_desc)
#define ATL1C_TPD_DESC(R, i)	ATL1C_GET_DESC(R, i, struct atl1c_tpd_desc)
#define ATL1C_RRD_DESC(R, i)	ATL1C_GET_DESC(R, i, struct atl1c_recv_ret_status)

/* tpd word 1 bit 0:7 General Checksum task offload */
#define TPD_L4HDR_OFFSET_MASK	0x00FF
#define TPD_L4HDR_OFFSET_SHIFT	0

/* tpd word 1 bit 0:7 Large Send task offload (IPv4/IPV6) */
#define TPD_TCPHDR_OFFSET_MASK	0x00FF
#define TPD_TCPHDR_OFFSET_SHIFT	0

/* tpd word 1 bit 0:7 Custom Checksum task offload */
#define TPD_PLOADOFFSET_MASK	0x00FF
#define TPD_PLOADOFFSET_SHIFT	0

/* tpd word 1 bit 8:17 */
#define TPD_CCSUM_EN_MASK	0x0001
#define TPD_CCSUM_EN_SHIFT	8
#define TPD_IP_CSUM_MASK	0x0001
#define TPD_IP_CSUM_SHIFT	9
#define TPD_TCP_CSUM_MASK	0x0001
#define TPD_TCP_CSUM_SHIFT	10
#define TPD_UDP_CSUM_MASK	0x0001
#define TPD_UDP_CSUM_SHIFT	11
#define TPD_LSO_EN_MASK		0x0001	/* TCP Large Send Offload */
#define TPD_LSO_EN_SHIFT	12
#define TPD_LSO_VER_MASK	0x0001
#define TPD_LSO_VER_SHIFT	13 	/* 0 : ipv4; 1 : ipv4/ipv6 */
#define TPD_CON_VTAG_MASK	0x0001
#define TPD_CON_VTAG_SHIFT	14
#define TPD_INS_VTAG_MASK	0x0001
#define TPD_INS_VTAG_SHIFT	15
#define TPD_IPV4_PACKET_MASK	0x0001  /* valid when LSO VER  is 1 */
#define TPD_IPV4_PACKET_SHIFT	16
#define TPD_ETH_TYPE_MASK	0x0001
#define TPD_ETH_TYPE_SHIFT	17	/* 0 : 802.3 frame; 1 : Ethernet */

/* tpd word 18:25 Custom Checksum task offload */
#define TPD_CCSUM_OFFSET_MASK	0x00FF
#define TPD_CCSUM_OFFSET_SHIFT	18
#define TPD_CCSUM_EPAD_MASK	0x0001
#define TPD_CCSUM_EPAD_SHIFT	30

/* tpd word 18:30 Large Send task offload (IPv4/IPV6) */
#define TPD_MSS_MASK            0x1FFF
#define TPD_MSS_SHIFT		18

#define TPD_EOP_MASK		0x0001
#define TPD_EOP_SHIFT		31

//16 bytes aligned
struct atl1c_tpd_desc {
	u16	buffer_len; /* include 4-byte CRC */
	u16	vlan_tag;
	u32	word1;
	//unsigned long	buffer_addr;
	u32 buffer_addr;
    u32 pad;
};

struct atl1c_tpd_ext_desc {
	u32 reservd_0;
	u32 word1;
    u32 pkt_len;
	u32 reservd_1;	
};
/* rrs word 0 bit 0:31 */
#define RRS_RX_CSUM_MASK	0xFFFF
#define RRS_RX_CSUM_SHIFT	0
#define RRS_RX_RFD_CNT_MASK	0x000F
#define RRS_RX_RFD_CNT_SHIFT	16
#define RRS_RX_RFD_INDEX_MASK	0x0FFF
#define RRS_RX_RFD_INDEX_SHIFT	20

/* rrs flag bit 0:16 */
#define RRS_HEAD_LEN_MASK	0x00FF
#define RRS_HEAD_LEN_SHIFT	0
#define RRS_HDS_TYPE_MASK	0x0003
#define RRS_HDS_TYPE_SHIFT	8
#define RRS_CPU_NUM_MASK	0x0003
#define	RRS_CPU_NUM_SHIFT	10
#define RRS_HASH_FLG_MASK	0x000F
#define RRS_HASH_FLG_SHIFT	12

#define RRS_HDS_TYPE_HEAD	1
#define RRS_HDS_TYPE_DATA	2

#define RRS_IS_NO_HDS_TYPE(flag) \
	(((flag) >> (RRS_HDS_TYPE_SHIFT)) & RRS_HDS_TYPE_MASK == 0)

#define RRS_IS_HDS_HEAD(flag) \
	(((flag) >> (RRS_HDS_TYPE_SHIFT)) & RRS_HDS_TYPE_MASK == \
			RRS_HDS_TYPE_HEAD)

#define RRS_IS_HDS_DATA(flag) \
        (((flag) >> (RRS_HDS_TYPE_SHIFT)) & RRS_HDS_TYPE_MASK == \
                        RRS_HDS_TYPE_DATA)

/* rrs word 3 bit 0:31 */
#define RRS_PKT_SIZE_MASK	0x3FFF
#define RRS_PKT_SIZE_SHIFT	0	
#define RRS_ERR_L4_CSUM_MASK	0x0001
#define RRS_ERR_L4_CSUM_SHIFT	14
#define RRS_ERR_IP_CSUM_MASK	0x0001
#define RRS_ERR_IP_CSUM_SHIFT	15
#define RRS_VLAN_INS_MASK	0x0001
#define RRS_VLAN_INS_SHIFT	16
#define RRS_PROT_ID_MASK	0x0007
#define RRS_PROT_ID_SHIFT	17
#define RRS_RX_ERR_SUM_MASK	0x0001
#define RRS_RX_ERR_SUM_SHIFT	20
#define RRS_RX_ERR_CRC_MASK	0x0001
#define RRS_RX_ERR_CRC_SHIFT	21
#define RRS_RX_ERR_FAE_MASK	0x0001
#define RRS_RX_ERR_FAE_SHIFT	22
#define RRS_RX_ERR_TRUNC_MASK	0x0001
#define RRS_RX_ERR_TRUNC_SHIFT	23
#define RRS_RX_ERR_RUNC_MASK	0x0001
#define RRS_RX_ERR_RUNC_SHIFT	24
#define RRS_RX_ERR_ICMP_MASK	0x0001 /* Incomplete Packet: due to insufficient rxf */
#define RRS_RX_ERR_ICMP_SHIFT	25	
#define RRS_PACKET_BCAST_MASK	0x0001
#define RRS_PACKET_BCAST_SHIFT	26
#define RRS_PACKET_MCAST_MASK	0x0001
#define RRS_PACKET_MCAST_SHIFT	27
#define RRS_PACKET_TYPE_MASK	0x0001
#define RRS_PACKET_TYPE_SHIFT	28
#define RRS_FIFO_FULL_MASK	0x0001
#define RRS_FIFO_FULL_SHIFT	29
#define RRS_802_3_LEN_ERR_MASK 	0x0001
#define RRS_802_3_LEN_ERR_SHIFT 30
#define RRS_RXD_UPDATED_MASK	0x0001
#define RRS_RXD_UPDATED_SHIFT	31

#define RRS_ERR_L4_CSUM         0x00004000
#define RRS_ERR_IP_CSUM         0x00008000
#define RRS_VLAN_INS            0x00010000
#define RRS_RX_ERR_SUM          0x00100000
#define RRS_RX_ERR_CRC          0x00200000
#define RRS_802_3_LEN_ERR	0x40000000
#define RRS_RXD_UPDATED		0x80000000

#define RRS_PACKET_TYPE_802_3  	1 
#define RRS_PACKET_TYPE_ETH	0
#define RRS_PACKET_IS_ETH(word) \
	(((word) >> RRS_PACKET_TYPE_SHIFT) & RRS_PACKET_TYPE_MASK == \
			RRS_PACKET_TYPE_ETH)
#define RRS_RXD_IS_VALID(word) \
	((((word) >> RRS_RXD_UPDATED_SHIFT) & RRS_RXD_UPDATED_MASK) == 1)

#define RRS_PACKET_PROT_IS_IPV4_ONLY(word) \
	((((word) >> RRS_PROT_ID_SHIFT) & RRS_PROT_ID_MASK) == 1)
#define RRS_PACKET_PROT_IS_IPV6_ONLY(word) \
	((((word) >> RRS_PROT_ID_SHIFT) & RRS_PROT_ID_MASK) == 6)

struct atl1c_recv_ret_status {
	u32 word0;
	u32	rss_hash;
	u16	vlan_tag;
	u16	flag;
	u32 word3;
};

/* RFD desciptor */
//8 bytes aligned
struct atl1c_rx_free_desc {
	u32 buffer_addr;
    u32 pad;
};

/* DMA Order Settings */
enum atl1c_dma_order {
	atl1c_dma_ord_in = 1,
	atl1c_dma_ord_enh = 2,
	atl1c_dma_ord_out = 4
};

enum atl1c_dma_rcb {
	atl1c_rcb_64 = 0,
	atl1c_rcb_128 = 1
};

enum atl1c_mac_speed {
	atl1c_mac_speed_0 = 0,
	atl1c_mac_speed_10_100 = 1,
	atl1c_mac_speed_1000 = 2
};

enum atl1c_dma_req_block {
	atl1c_dma_req_128 = 0,
	atl1c_dma_req_256 = 1,
	atl1c_dma_req_512 = 2,
	atl1c_dma_req_1024 = 3,
	atl1c_dma_req_2048 = 4,
	atl1c_dma_req_4096 = 5
};

enum atl1c_rss_mode {
	atl1c_rss_mode_disable = 0,
	atl1c_rss_sig_que = 1,
	atl1c_rss_mul_que_sig_int = 2,
	atl1c_rss_mul_que_mul_int = 4,
};

enum atl1c_rss_type {
	atl1c_rss_disable = 0,
	atl1c_rss_ipv4 = 1,
	atl1c_rss_ipv4_tcp = 2,
	atl1c_rss_ipv6 = 4,
	atl1c_rss_ipv6_tcp = 8
};

enum atl1c_nic_type {
	athr_l1c = 0,
	athr_l2c, 
	athr_l2c_b,
	athr_l2c_b2,
	athr_l1d,
};

enum atl1c_trans_queue {
	atl1c_trans_normal = 0,
	atl1c_trans_high = 1
};

#ifndef ETH_ALEN
#define ETH_ALEN    6       /* Octets in one ethernet addr   */
#endif
#ifndef ETH_HLEN
#define ETH_HLEN    14      /* Total octets in header.   */
#endif
#ifndef ETH_ZLEN
#define ETH_ZLEN    60      /* Min. octets in frame sans FCS */
#endif
#ifndef ETH_DATA_LEN
#define ETH_DATA_LEN    1500        /* Max. octets in payload    */
#endif
#ifndef ETH_FRAME_LEN
#define ETH_FRAME_LEN   1514        /* Max. octets in frame sans FCS */
#endif
#ifndef ETH_FCS_LEN
#define ETH_FCS_LEN 4 
#endif

#define ADVERTISE_DEFAULT (ADVERTISE_10_HALF  |\
			   ADVERTISE_10_FULL  |\
			   ADVERTISE_100_HALF |\
			   ADVERTISE_100_FULL |\
			   ADVERTISE_1000_FULL)	
#define ATL1C_INTR_CLEAR_ON_READ	0x0001
#define ATL1C_INTR_MODRT_ENABLE	 	0x0002
#define ATL1C_CMB_ENABLE		0x0004		
#define ATL1C_SMB_ENABLE		0x0010
#define ATL1C_TXQ_MODE_ENHANCE		0x0020
#define ATL1C_RX_IPV6_CHKSUM		0x0040
#define ATL1C_ASPM_L0S_SUPPORT		0x0080
#define ATL1C_ASPM_L1_SUPPORT		0x0100
#define ATL1C_ASPM_CTRL_MON		0x0200
#define ATL1C_HIB_DISABLE		0x0400
#define ATL1C_APS_MODE_ENABLE		0x0800
#define ATL1C_LINK_EXT_SYNC		0x1000
#define ATL1C_CLK_GATING_EN		0x2000
#define ATL1C_FPGA_VERSION		0x8000
#define LINK_CAP_SPEED_1000 		0x0001

struct atl1c_private;
struct atl1c_hw {
	u8       *hw_addr;            /* inner register address */
	struct atl1c_private *ap;
	enum atl1c_nic_type  nic_type;
	enum atl1c_dma_order dma_order;
	enum atl1c_dma_req_block dmar_block;
	enum atl1c_dma_req_block dmaw_block;
	
	u16 device_id;
	u16 vendor_id;
	u8 revision_id;	
	u16 phy_id1;
	u16 phy_id2;

	u32 intr_mask;
	u8 dmaw_dly_cnt;
	u8 dmar_dly_cnt;

	u8 preamble_len;
	u16 max_frame_size;
	u16 min_frame_size;

	enum atl1c_mac_speed mac_speed;
	unsigned int  mac_duplex;
	u16 media_type;
	
	u16 autoneg_advertised;

	u16 mii_autoneg_adv_reg;
	u16 mii_1000t_ctrl_reg;

	u16 tx_imt;	/* TX Interrupt Moderator timer ( 2us resolution) */
	u16 rx_imt;	/* RX Interrupt Moderator timer ( 2us resolution) */
	u16 ict;        /* Interrupt Clear timer (2us resolution) */
	u16 ctrl_flags;

	u16 link_cap_flags;
	u16 cmb_tpd;
	u16 cmb_rrd;
	u16 cmb_rx_timer; /* 2us resolution */
	u16 cmb_tx_timer;
	u32 smb_timer;
	
	u16 rrd_thresh; /* Threshold of number of RRD produced to trigger
			  interrupt request */
	u16 tpd_thresh;
	u8 tpd_burst;   /* Number of TPD to prefetch in cache-aligned burst. */
	u8 rfd_burst;
	enum atl1c_rss_type rss_type;
	enum atl1c_rss_mode rss_mode;
	u8 rss_hash_bits;
	u32 base_cpu;
	u32 indirect_tab;
	u8 mac_addr[ETH_ALEN];
	u8 perm_mac_addr[ETH_ALEN];

	unsigned int phy_configured;
	unsigned int  re_autoneg;
};

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

#define ATL1C_REGS_SIZE		256
#define ATL1C_NAPI_WEIGHT	64

#define RX_BUF_SIZE	0x05F3	/* 0x05F3 = 1522bye + 1 */
#define TX_BUF_SIZE	0x05F3	/* 0x05F3 = 1522bye + 1 */
#define ATL1C_TX_RING_BYTES	(NUM_TX_DESC * sizeof(struct TxDesc))
#define ATL1C_RX_RING_BYTES	(NUM_RX_DESC * sizeof(struct RxDesc))

#define ATL1C_TX_TIMEOUT	(6 * HZ)
#define ATL1C_LINK_TIMEOUT	(1 * HZ)
#define ATL1C_ESD_TIMEOUT	(2 * HZ)

#define NODE_ADDRESS_SIZE 6

/* write/read MMIO register */
#define writeb(val, addr) (*(volatile u8*)(addr) = (val))
#define writew(val, addr) (*(volatile u16*)(addr) = (val))
#define writel(val, addr) (*(volatile u32*)(addr) = (val))
#define readb(addr) (*(volatile u8*)(addr))
#define readw(addr) (*(volatile u16*)(addr))
#define readl(addr) (*(volatile u32*)(addr))

#define AT_WRITE_REG(a, reg, value) ({ 			\
		unsigned int _tmp;			\
		if ((a)->nic_type == athr_l2c_b2)		\
			if ((reg) < 0x1400)		\
				AT_READ_REG(a, reg, &_tmp);	\
		writel((value), ((a)->hw_addr + reg));})

#define AT_WRITE_FLUSH(a) (\
		readl((a)->hw_addr))

#define AT_READ_REG(a, reg, pdata) do {					\
			*(u32 *)pdata = readl((a)->hw_addr + reg);	\
	} while (0)

#define AT_WRITE_REGB(a, reg, value) (\
		writeb((value), ((a)->hw_addr + reg)))

#define AT_READ_REGB(a, reg) (\
		readb((a)->hw_addr + reg))

#define AT_WRITE_REGW(a, reg, value) (\
		writew((value), ((a)->hw_addr + reg)))

#define AT_READ_REGW(a, reg) (\
		readw((a)->hw_addr + reg))

#define AT_WRITE_REG_ARRAY(a, reg, offset, value) ( \
		writel((value), (((a)->hw_addr + reg) + ((offset) << 2))))

#define AT_READ_REG_ARRAY(a, reg, offset) ( \
		readl(((a)->hw_addr + reg) + ((offset) << 2)))

#define ATL1C_DEBUG 
#ifdef ATL1C_DEBUG
#define dprintk(fmt, args...)  do { printf(fmt,## args); } while (0)
#else
#define dprintk(fmt, args...)  do { } while (0)
#endif 

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

enum features {
//	RTL_FEATURE_WOL	= (1 << 0),
	RTL_FEATURE_MSI	= (1 << 1),
};

enum ATL1C_DSM_STATE {
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

/* Coalescing Message Block */
struct coals_msg_block {
	int test;
};

/*
 * atl1c_ring_header represents a single, contiguous block of DMA space
 * mapped for the three descriptor rings (tpd, rfd, rrd) and the two
 * message blocks (cmb, smb) described below
 */
struct atl1c_ring_header {
        void *desc;             /* virtual address */
        dma_addr_t dma;         /* physical address*/
        unsigned int size;      /* length in bytes */
};

/*
 * atl1c_buffer is wrapper around a pointer to a socket buffer
 * so a DMA handle can be stored along with the skb
 */
#define ATL1_BUFFER_FREE	0
#define ATL1_BUFFER_BUSY	1
struct atl1c_buffer {
        struct sk_buff *skb;    /* socket buffer */
        u16 length;             /* rx buffer length */
        u16 state;		/* state of buffer */
       dma_addr_t dma;
};

/* transimit packet descriptor (tpd) ring */
struct atl1c_tpd_ring {
	void *desc;		/* descriptor ring virtual address */
	dma_addr_t dma;		/* descriptor ring physical address */
	u16 size;		/* descriptor ring length in bytes */
	u16 count;		/* number of descriptors in the ring */
	u16 next_to_use; 	/* this is protectd by adapter->tx_lock */
	int next_to_clean;
	struct atl1c_buffer *buffer_info;
};

/* receive free descriptor (rfd) ring */
struct atl1c_rfd_ring {
	void *desc;		/* descriptor ring virtual address */
	dma_addr_t dma;		/* descriptor ring physical address */
	u16 size;		/* descriptor ring length in bytes */
	u16 count;		/* number of descriptors in the ring */
	u16 next_to_use;
	u16 next_to_clean;
	struct atl1c_buffer *buffer_info;
};

/* receive return desciptor (rrd) ring */
struct atl1c_rrd_ring {
	void *desc;		/* descriptor ring virtual address */
	dma_addr_t dma;		/* descriptor ring physical address */
	u16 size;		/* descriptor ring length in bytes */
	u16 count;		/* number of descriptors in the ring */
	u16 next_to_use;
	u16 next_to_clean;
};

struct atl1c_cmb {
	void *cmb;
	dma_addr_t dma;
};

struct atl1c_smb {
	void *smb;
	dma_addr_t dma;
};


struct atl1c_private {
	struct device sc_dev;		/* generic device structures */
	void *sc_ih;			/* interrupt handler cookie */
	bus_space_tag_t sc_st;		/* bus space tag */
	bus_space_handle_t sc_sh;	/* bus space handle */
	pci_chipset_tag_t sc_pc;	/* chipset handle needed by mips */

    u16 rx_buffer_len;
	u32 wol;
	u16 link_speed;
	u16 link_duplex;

    struct atl1c_hw hw;
	struct pci_attach_args *pa; 

	struct arpcom arpcom;		/* per-interface network data !!we use this*/
	unsigned char   dev_addr[6]; //the net interface's address
	unsigned long ioaddr;
	int flags;	
	u16 intr_mask;

	/* All Descriptor memory */
	struct atl1c_ring_header ring_header;
	struct atl1c_tpd_ring tpd_ring[AT_MAX_TRANSMIT_QUEUE];
	struct atl1c_rfd_ring rfd_ring[AT_MAX_RECEIVE_QUEUE];
	struct atl1c_rrd_ring rrd_ring[AT_MAX_RECEIVE_QUEUE];
	struct atl1c_cmb cmb;
	struct atl1c_smb smb;
	int num_rx_queues;
};
/* register definition */
#define REG_PM_CTRLSTAT             0x44
#define     PM_CTRLSTAT_PME_EN		0x100
#define     PM_CTRLSTAT_PMES_EN		0x8000

#define REG_PCIE_CAP_LIST           	0x58

#define REG_DEVICE_CAP              	0x5C
#define     DEVICE_CAP_MAX_PAYLOAD_MASK     0x7
#define     DEVICE_CAP_MAX_PAYLOAD_SHIFT    0

#define REG_DEVICE_CTRL			0x60
#define     DEVICE_CTRL_MAX_PAYLOAD_MASK    0x7
#define     DEVICE_CTRL_MAX_PAYLOAD_SHIFT   5
#define     DEVICE_CTRL_MAX_RREQ_SZ_MASK    0x7
#define     DEVICE_CTRL_MAX_RREQ_SZ_SHIFT   12

#define REG_LINK_CTRL			0x68
#define     LINK_CTRL_L0S_EN		0x01
#define     LINK_CTRL_L1_EN			0x02
#define     LINK_CTRL_EXT_SYNC		0x80

#define REG_VPD_CAP			0x6C
#define VPD_CAP_ID_MASK                 0xff
#define VPD_CAP_ID_SHIFT                0
#define VPD_CAP_NEXT_PTR_MASK           0xFF
#define VPD_CAP_NEXT_PTR_SHIFT          8
#define VPD_CAP_VPD_ADDR_MASK           0x7FFF
#define VPD_CAP_VPD_ADDR_SHIFT          16
#define VPD_CAP_VPD_FLAG                0x80000000

#define REG_VPD_DATA                	0x70

#define REG_PCIE_UC_SEVERITY		0x10C
#define PCIE_UC_SERVRITY_TRN		0x00000001
#define PCIE_UC_SERVRITY_DLP		0x00000010
#define PCIE_UC_SERVRITY_PSN_TLP	0x00001000
#define PCIE_UC_SERVRITY_FCP		0x00002000
#define PCIE_UC_SERVRITY_CPL_TO		0x00004000
#define PCIE_UC_SERVRITY_CA		0x00008000
#define PCIE_UC_SERVRITY_UC		0x00010000
#define PCIE_UC_SERVRITY_ROV		0x00020000
#define PCIE_UC_SERVRITY_MLFP		0x00040000
#define PCIE_UC_SERVRITY_ECRC		0x00080000
#define PCIE_UC_SERVRITY_UR		0x00100000

#define REG_DEV_SERIALNUM_CTRL		0x200
#define REG_DEV_MAC_SEL_MASK		0x0 /* 0:EUI; 1:MAC */
#define REG_DEV_MAC_SEL_SHIFT		0
#define REG_DEV_SERIAL_NUM_EN_MASK	0x1
#define REG_DEV_SERIAL_NUM_EN_SHIFT	1

#define REG_TWSI_CTRL               	0x218
#define TWSI_CTRL_LD_OFFSET_MASK        0xFF
#define TWSI_CTRL_LD_OFFSET_SHIFT       0
#define TWSI_CTRL_LD_SLV_ADDR_MASK      0x7
#define TWSI_CTRL_LD_SLV_ADDR_SHIFT     8
#define TWSI_CTRL_SW_LDSTART            0x800
#define TWSI_CTRL_HW_LDSTART            0x1000
#define TWSI_CTRL_SMB_SLV_ADDR_MASK     0x7F
#define TWSI_CTRL_SMB_SLV_ADDR_SHIFT    15
#define TWSI_CTRL_LD_EXIST              0x400000
#define TWSI_CTRL_READ_FREQ_SEL_MASK    0x3
#define TWSI_CTRL_READ_FREQ_SEL_SHIFT   23
#define TWSI_CTRL_FREQ_SEL_100K         0
#define TWSI_CTRL_FREQ_SEL_200K         1
#define TWSI_CTRL_FREQ_SEL_300K         2
#define TWSI_CTRL_FREQ_SEL_400K         3
#define TWSI_CTRL_SMB_SLV_ADDR
#define TWSI_CTRL_WRITE_FREQ_SEL_MASK   0x3
#define TWSI_CTRL_WRITE_FREQ_SEL_SHIFT  24


#define REG_PCIE_DEV_MISC_CTRL      	0x21C
#define PCIE_DEV_MISC_EXT_PIPE     	0x2
#define PCIE_DEV_MISC_RETRY_BUFDIS 	0x1
#define PCIE_DEV_MISC_SPIROM_EXIST 	0x4
#define PCIE_DEV_MISC_SERDES_ENDIAN    	0x8
#define PCIE_DEV_MISC_SERDES_SEL_DIN   	0x10

#define REG_PCIE_PHYMISC	    	0x1000
#define PCIE_PHYMISC_FORCE_RCV_DET	0x4

#define REG_PCIE_PHYMISC2		0x1004
#define PCIE_PHYMISC2_SERDES_CDR_MASK	0x3
#define PCIE_PHYMISC2_SERDES_CDR_SHIFT	16
#define PCIE_PHYMISC2_SERDES_TH_MASK	0x3
#define PCIE_PHYMISC2_SERDES_TH_SHIFT	18

#define REG_TWSI_DEBUG			0x1108
#define TWSI_DEBUG_DEV_EXIST		0x20000000

#define REG_EEPROM_CTRL			0x12C0
#define EEPROM_CTRL_DATA_HI_MASK	0xFFFF
#define EEPROM_CTRL_DATA_HI_SHIFT	0
#define EEPROM_CTRL_ADDR_MASK		0x3FF
#define EEPROM_CTRL_ADDR_SHIFT		16
#define EEPROM_CTRL_ACK			0x40000000
#define EEPROM_CTRL_RW			0x80000000

#define REG_EEPROM_DATA_LO		0x12C4

#define REG_OTP_CTRL			0x12F0
#define OTP_CTRL_CLK_EN			0x0002

#define REG_PM_CTRL			0x12F8
#define PM_CTRL_SDES_EN			0x00000001
#define PM_CTRL_RBER_EN			0x00000002
#define PM_CTRL_CLK_REQ_EN		0x00000004
#define PM_CTRL_ASPM_L1_EN		0x00000008
#define PM_CTRL_SERDES_L1_EN		0x00000010
#define PM_CTRL_SERDES_PLL_L1_EN	0x00000020
#define PM_CTRL_SERDES_PD_EX_L1		0x00000040
#define PM_CTRL_SERDES_BUDS_RX_L1_EN	0x00000080
#define PM_CTRL_L0S_ENTRY_TIMER_MASK	0xF
#define PM_CTRL_L0S_ENTRY_TIMER_SHIFT	8
#define PM_CTRL_ASPM_L0S_EN		0x00001000
#define PM_CTRL_CLK_SWH_L1		0x00002000
#define PM_CTRL_CLK_PWM_VER1_1		0x00004000
#define PM_CTRL_RCVR_WT_TIMER		0x00008000
#define PM_CTRL_L1_ENTRY_TIMER_MASK	0xF
#define PM_CTRL_L1_ENTRY_TIMER_SHIFT	16
#define PM_CTRL_PM_REQ_TIMER_MASK	0xF
#define PM_CTRL_PM_REQ_TIMER_SHIFT	20
#define PM_CTRL_LCKDET_TIMER_MASK	0xF
#define PM_CTRL_LCKDET_TIMER_SHIFT	24
#define PM_CTRL_EN_BUFS_RX_L0S		0x10000000
#define PM_CTRL_SA_DLY_EN		0x20000000
#define PM_CTRL_MAC_ASPM_CHK		0x40000000
#define PM_CTRL_HOTRST			0x80000000

#define REG_LTSSM_ID_CTRL		0x12FC
#define LTSSM_ID_EN_WRO			0x1000
/* Selene Master Control Register */
#define REG_MASTER_CTRL			0x1400
#define MASTER_CTRL_SOFT_RST            0x1
#define MASTER_CTRL_TEST_MODE_MASK	0x3
#define MASTER_CTRL_TEST_MODE_SHIFT	2
#define MASTER_CTRL_BERT_START		0x10
#define MASTER_CTRL_OOB_DIS_OFF		0x40
#define MASTER_CTRL_SA_TIMER_EN		0x80
#define MASTER_CTRL_MTIMER_EN           0x100
#define MASTER_CTRL_MANUAL_INT          0x200
#define MASTER_CTRL_TX_ITIMER_EN	0x400
#define MASTER_CTRL_RX_ITIMER_EN	0x800
#define MASTER_CTRL_CLK_SEL_DIS		0x1000
#define MASTER_CTRL_CLK_SWH_MODE	0x2000
#define MASTER_CTRL_INT_RDCLR		0x4000
#define MASTER_CTRL_REV_NUM_SHIFT	16
#define MASTER_CTRL_REV_NUM_MASK	0xff
#define MASTER_CTRL_DEV_ID_SHIFT	24
#define MASTER_CTRL_DEV_ID_MASK		0x7f
#define MASTER_CTRL_OTP_SEL		0x80000000

/* Timer Initial Value Register */
#define REG_MANUAL_TIMER_INIT       	0x1404

/* IRQ ModeratorTimer Initial Value Register */
#define REG_IRQ_MODRT_TIMER_INIT     	0x1408
#define IRQ_MODRT_TIMER_MASK		0xffff
#define IRQ_MODRT_TX_TIMER_SHIFT    	0
#define IRQ_MODRT_RX_TIMER_SHIFT	16

#define REG_GPHY_CTRL               	0x140C
#define GPHY_CTRL_EXT_RESET         	0x1
#define GPHY_CTRL_RTL_MODE		0x2
#define GPHY_CTRL_LED_MODE		0x4
#define GPHY_CTRL_ANEG_NOW		0x8
#define GPHY_CTRL_REV_ANEG		0x10
#define GPHY_CTRL_GATE_25M_EN       	0x20
#define GPHY_CTRL_LPW_EXIT          	0x40
#define GPHY_CTRL_PHY_IDDQ          	0x80
#define GPHY_CTRL_PHY_IDDQ_DIS      	0x100
#define GPHY_CTRL_GIGA_DIS		0x200
#define GPHY_CTRL_HIB_EN            	0x400
#define GPHY_CTRL_HIB_PULSE         	0x800
#define GPHY_CTRL_SEL_ANA_RST       	0x1000
#define GPHY_CTRL_PHY_PLL_ON        	0x2000
#define GPHY_CTRL_PWDOWN_HW		0x4000
#define GPHY_CTRL_PHY_PLL_BYPASS	0x8000

#define GPHY_CTRL_DEFAULT (		 \
		GPHY_CTRL_SEL_ANA_RST	|\
		GPHY_CTRL_HIB_PULSE	|\
		GPHY_CTRL_HIB_EN)

#define GPHY_CTRL_PW_WOL_DIS (		 \
		GPHY_CTRL_SEL_ANA_RST	|\
		GPHY_CTRL_HIB_PULSE	|\
		GPHY_CTRL_HIB_EN	|\
		GPHY_CTRL_PWDOWN_HW	|\
		GPHY_CTRL_PHY_IDDQ)

#define GPHY_CTRL_POWER_SAVING (	\
		GPHY_CTRL_SEL_ANA_RST	|\
		GPHY_CTRL_HIB_EN	|\
		GPHY_CTRL_HIB_PULSE	|\
		GPHY_CTRL_PWDOWN_HW	|\
		GPHY_CTRL_PHY_IDDQ)
/* Block IDLE Status Register */
#define REG_IDLE_STATUS  		0x1410
#define IDLE_STATUS_MASK		0x00FF
#define IDLE_STATUS_RXMAC_NO_IDLE      	0x1
#define IDLE_STATUS_TXMAC_NO_IDLE      	0x2
#define IDLE_STATUS_RXQ_NO_IDLE        	0x4     
#define IDLE_STATUS_TXQ_NO_IDLE        	0x8
#define IDLE_STATUS_DMAR_NO_IDLE       	0x10 	
#define IDLE_STATUS_DMAW_NO_IDLE       	0x20 	
#define IDLE_STATUS_SMB_NO_IDLE        	0x40 	
#define IDLE_STATUS_CMB_NO_IDLE        	0x80 	

/* MDIO Control Register */
#define REG_MDIO_CTRL           	0x1414
#define MDIO_DATA_MASK          	0xffff  /* On MDIO write, the 16-bit
						 * control data to write to PHY 
						 * MII management register */
#define MDIO_DATA_SHIFT         	0       /* On MDIO read, the 16-bit
						 * status data that was read 
						 * from the PHY MII management register */
#define MDIO_REG_ADDR_MASK      	0x1f    /* MDIO register address */
#define MDIO_REG_ADDR_SHIFT     	16
#define MDIO_RW                 	0x200000  /* 1: read, 0: write */
#define MDIO_SUP_PREAMBLE       	0x400000  /* Suppress preamble */
#define MDIO_START              	0x800000  /* Write 1 to initiate the MDIO 
						   * master. And this bit is self
						   * cleared after one cycle */
#define MDIO_CLK_SEL_SHIFT      	24
#define MDIO_CLK_25_4           	0
#define MDIO_CLK_25_6           	2
#define MDIO_CLK_25_8           	3
#define MDIO_CLK_25_10          	4
#define MDIO_CLK_25_14          	5
#define MDIO_CLK_25_20          	6
#define MDIO_CLK_25_28          	7
#define MDIO_BUSY               	0x8000000
#define MDIO_AP_EN              	0x10000000
#define MDIO_WAIT_TIMES         	10

/* MII PHY Status Register */
#define REG_PHY_STATUS           	0x1418
#define PHY_GENERAL_STATUS_MASK		0xFFFF
#define PHY_STATUS_RECV_ENABLE		0x0001
#define PHY_OE_PWSP_STATUS_MASK		0x07FF
#define PHY_OE_PWSP_STATUS_SHIFT	16
#define PHY_STATUS_LPW_STATE		0x80000000
/* BIST Control and Status Register0 (for the Packet Memory) */
#define REG_BIST0_CTRL              	0x141c
#define BIST0_NOW                   	0x1 
#define BIST0_SRAM_FAIL             	0x2 /* 1: The SRAM failure is
					     * un-repairable  because 
					     * it has address decoder 
					     * failure or more than 1 cell 
					     * stuck-to-x failure */
#define BIST0_FUSE_FLAG             	0x4

/* BIST Control and Status Register1(for the retry buffer of PCI Express) */
#define REG_BIST1_CTRL			0x1420
#define BIST1_NOW                   	0x1 
#define BIST1_SRAM_FAIL             	0x2
#define BIST1_FUSE_FLAG             	0x4

/* SerDes Lock Detect Control and Status Register */
#define REG_SERDES_LOCK            	0x1424
#define SERDES_LOCK_DETECT          	0x1  /* SerDes lock detected. This signal
					      * comes from Analog SerDes */
#define SERDES_LOCK_DETECT_EN       	0x2  /* 1: Enable SerDes Lock detect function */
#define SERDES_LOCK_STS_SELFB_PLL_SHIFT 0xE
#define SERDES_LOCK_STS_SELFB_PLL_MASK  0x3
#define SERDES_OVCLK_18_25		0x0
#define SERDES_OVCLK_12_18		0x1
#define SERDES_OVCLK_0_4		0x2
#define SERDES_OVCLK_4_12		0x3
#define SERDES_MAC_CLK_SLOWDOWN		0x20000
#define SERDES_PYH_CLK_SLOWDOWN		0x40000

/* MAC Control Register  */
#define REG_MAC_CTRL         		0x1480
#define MAC_CTRL_TX_EN			0x1 
#define MAC_CTRL_RX_EN			0x2 
#define MAC_CTRL_TX_FLOW		0x4  
#define MAC_CTRL_RX_FLOW            	0x8  
#define MAC_CTRL_LOOPBACK          	0x10      
#define MAC_CTRL_DUPLX              	0x20      
#define MAC_CTRL_ADD_CRC            	0x40      
#define MAC_CTRL_PAD                	0x80      
#define MAC_CTRL_LENCHK             	0x100     
#define MAC_CTRL_HUGE_EN            	0x200     
#define MAC_CTRL_PRMLEN_SHIFT       	10        
#define MAC_CTRL_PRMLEN_MASK        	0xf
#define MAC_CTRL_RMV_VLAN           	0x4000    
#define MAC_CTRL_PROMIS_EN          	0x8000    
#define MAC_CTRL_TX_PAUSE           	0x10000   
#define MAC_CTRL_SCNT               	0x20000   
#define MAC_CTRL_SRST_TX            	0x40000   
#define MAC_CTRL_TX_SIMURST         	0x80000   
#define MAC_CTRL_SPEED_SHIFT        	20        
#define MAC_CTRL_SPEED_MASK         	0x3
#define MAC_CTRL_DBG_TX_BKPRESURE   	0x400000  
#define MAC_CTRL_TX_HUGE            	0x800000  
#define MAC_CTRL_RX_CHKSUM_EN       	0x1000000 
#define MAC_CTRL_MC_ALL_EN          	0x2000000 
#define MAC_CTRL_BC_EN              	0x4000000 
#define MAC_CTRL_DBG                	0x8000000 
#define MAC_CTRL_SINGLE_PAUSE_EN	0x10000000
#define MAC_CTRL_HASH_ALG_CRC32		0x20000000
#define MAC_CTRL_SPEED_MODE_SW		0x40000000
/* MAC IPG/IFG Control Register  */
#define REG_MAC_IPG_IFG             	0x1484
#define MAC_IPG_IFG_IPGT_SHIFT      	0 	/* Desired back to back 
						 * inter-packet gap. The 
						 * default is 96-bit time */
#define MAC_IPG_IFG_IPGT_MASK       	0x7f
#define MAC_IPG_IFG_MIFG_SHIFT      	8       /* Minimum number of IFG to 
					         * enforce in between RX frames */
#define MAC_IPG_IFG_MIFG_MASK       	0xff  	/* Frame gap below such IFP is dropped */
#define MAC_IPG_IFG_IPGR1_SHIFT     	16   	/* 64bit Carrier-Sense window */
#define MAC_IPG_IFG_IPGR1_MASK      	0x7f
#define MAC_IPG_IFG_IPGR2_SHIFT     	24    	/* 96-bit IPG window */
#define MAC_IPG_IFG_IPGR2_MASK      	0x7f

/* MAC STATION ADDRESS  */
#define REG_MAC_STA_ADDR		0x1488

/* Hash table for multicast address */
#define REG_RX_HASH_TABLE		0x1490

/* MAC Half-Duplex Control Register */
#define REG_MAC_HALF_DUPLX_CTRL     	0x1498
#define MAC_HALF_DUPLX_CTRL_LCOL_SHIFT  0      /* Collision Window */
#define MAC_HALF_DUPLX_CTRL_LCOL_MASK   0x3ff
#define MAC_HALF_DUPLX_CTRL_RETRY_SHIFT 12 
#define MAC_HALF_DUPLX_CTRL_RETRY_MASK  0xf
#define MAC_HALF_DUPLX_CTRL_EXC_DEF_EN  0x10000 
#define MAC_HALF_DUPLX_CTRL_NO_BACK_C   0x20000 
#define MAC_HALF_DUPLX_CTRL_NO_BACK_P   0x40000 /* No back-off on backpressure,
						 * immediately start the 
						 * transmission after back pressure */
#define MAC_HALF_DUPLX_CTRL_ABEBE        0x80000 /* 1: Alternative Binary Exponential Back-off Enabled */
#define MAC_HALF_DUPLX_CTRL_ABEBT_SHIFT  20      /* Maximum binary exponential number */
#define MAC_HALF_DUPLX_CTRL_ABEBT_MASK   0xf
#define MAC_HALF_DUPLX_CTRL_JAMIPG_SHIFT 24      /* IPG to start JAM for collision based flow control in half-duplex */
#define MAC_HALF_DUPLX_CTRL_JAMIPG_MASK  0xf     /* mode. In unit of 8-bit time */

/* Maximum Frame Length Control Register   */
#define REG_MTU                     	0x149c

/* Wake-On-Lan control register */
#define REG_WOL_CTRL                	0x14a0
#define WOL_PATTERN_EN              	0x00000001
#define WOL_PATTERN_PME_EN              0x00000002
#define WOL_MAGIC_EN                    0x00000004
#define WOL_MAGIC_PME_EN                0x00000008
#define WOL_LINK_CHG_EN                 0x00000010
#define WOL_LINK_CHG_PME_EN             0x00000020
#define WOL_PATTERN_ST                  0x00000100
#define WOL_MAGIC_ST                    0x00000200
#define WOL_LINKCHG_ST                  0x00000400
#define WOL_CLK_SWITCH_EN               0x00008000
#define WOL_PT0_EN                      0x00010000
#define WOL_PT1_EN                      0x00020000
#define WOL_PT2_EN                      0x00040000
#define WOL_PT3_EN                      0x00080000
#define WOL_PT4_EN                      0x00100000
#define WOL_PT5_EN                      0x00200000
#define WOL_PT6_EN                      0x00400000

/* WOL Length ( 2 DWORD ) */
#define REG_WOL_PATTERN_LEN         	0x14a4
#define WOL_PT_LEN_MASK                 0x7f
#define WOL_PT0_LEN_SHIFT               0
#define WOL_PT1_LEN_SHIFT               8
#define WOL_PT2_LEN_SHIFT               16
#define WOL_PT3_LEN_SHIFT               24
#define WOL_PT4_LEN_SHIFT               0
#define WOL_PT5_LEN_SHIFT               8
#define WOL_PT6_LEN_SHIFT               16

/* Internal SRAM Partition Register */
#define RFDX_HEAD_ADDR_MASK		0x03FF
#define RFDX_HARD_ADDR_SHIFT		0
#define RFDX_TAIL_ADDR_MASK		0x03FF
#define RFDX_TAIL_ADDR_SHIFT            16

#define REG_SRAM_RFD0_INFO		0x1500
#define REG_SRAM_RFD1_INFO		0x1504
#define REG_SRAM_RFD2_INFO		0x1508
#define	REG_SRAM_RFD3_INFO		0x150C

#define REG_RFD_NIC_LEN			0x1510 /* In 8-bytes */
#define RFD_NIC_LEN_MASK		0x03FF

#define REG_SRAM_TRD_ADDR           	0x1518
#define TPD_HEAD_ADDR_MASK		0x03FF
#define TPD_HEAD_ADDR_SHIFT		0
#define TPD_TAIL_ADDR_MASK		0x03FF
#define TPD_TAIL_ADDR_SHIFT		16

#define REG_SRAM_TRD_LEN            	0x151C /* In 8-bytes */
#define TPD_NIC_LEN_MASK		0x03FF

#define REG_SRAM_RXF_ADDR          	0x1520
#define REG_SRAM_RXF_LEN            	0x1524
#define REG_SRAM_TXF_ADDR           	0x1528
#define REG_SRAM_TXF_LEN            	0x152C
#define REG_SRAM_TCPH_ADDR          	0x1530
#define REG_SRAM_PKTH_ADDR          	0x1532

/*
 * Load Ptr Register 
 * Software sets this bit after the initialization of the head and tail */
#define REG_LOAD_PTR                	0x1534 

/*
 * addresses of all descriptors, as well as the following descriptor
 * control register, which triggers each function block to load the head
 * pointer to prepare for the operation. This bit is then self-cleared
 * after one cycle.
 */
#define REG_RX_BASE_ADDR_HI		0x1540
#define REG_TX_BASE_ADDR_HI		0x1544
#define REG_SMB_BASE_ADDR_HI		0x1548
#define REG_SMB_BASE_ADDR_LO		0x154C
#define REG_RFD0_HEAD_ADDR_LO		0x1550
#define REG_RFD1_HEAD_ADDR_LO		0x1554
#define REG_RFD2_HEAD_ADDR_LO		0x1558
#define REG_RFD3_HEAD_ADDR_LO		0x155C
#define REG_RFD_RING_SIZE		0x1560
#define RFD_RING_SIZE_MASK		0x0FFF
#define REG_RX_BUF_SIZE			0x1564
#define RX_BUF_SIZE_MASK		0xFFFF
#define REG_RRD0_HEAD_ADDR_LO		0x1568
#define REG_RRD1_HEAD_ADDR_LO		0x156C
#define REG_RRD2_HEAD_ADDR_LO		0x1570
#define REG_RRD3_HEAD_ADDR_LO		0x1574
#define REG_RRD_RING_SIZE		0x1578
#define RRD_RING_SIZE_MASK		0x0FFF
#define REG_HTPD_HEAD_ADDR_LO		0x157C
#define REG_NTPD_HEAD_ADDR_LO		0x1580
#define REG_TPD_RING_SIZE		0x1584
#define TPD_RING_SIZE_MASK		0xFFFF
#define REG_CMB_BASE_ADDR_LO		0x1588

/* RSS about */
#define REG_RSS_KEY0                    0x14B0
#define REG_RSS_KEY1                    0x14B4
#define REG_RSS_KEY2                    0x14B8
#define REG_RSS_KEY3                    0x14BC
#define REG_RSS_KEY4                    0x14C0
#define REG_RSS_KEY5                    0x14C4
#define REG_RSS_KEY6                    0x14C8
#define REG_RSS_KEY7                    0x14CC
#define REG_RSS_KEY8                    0x14D0
#define REG_RSS_KEY9                    0x14D4
#define REG_IDT_TABLE0                	0x14E0
#define REG_IDT_TABLE1                  0x14E4
#define REG_IDT_TABLE2                  0x14E8
#define REG_IDT_TABLE3                  0x14EC
#define REG_IDT_TABLE4                  0x14F0
#define REG_IDT_TABLE5                  0x14F4
#define REG_IDT_TABLE6                  0x14F8
#define REG_IDT_TABLE7                  0x14FC
#define REG_IDT_TABLE                   REG_IDT_TABLE0
#define REG_RSS_HASH_VALUE              0x15B0
#define REG_RSS_HASH_FLAG               0x15B4
#define REG_BASE_CPU_NUMBER             0x15B8

/* TXQ Control Register */
#define REG_TXQ_CTRL                	0x1590
#define	TXQ_NUM_TPD_BURST_MASK     	0xF
#define TXQ_NUM_TPD_BURST_SHIFT    	0
#define TXQ_CTRL_IP_OPTION_EN		0x10
#define TXQ_CTRL_EN                     0x20
#define TXQ_CTRL_ENH_MODE               0x40
#define TXQ_CTRL_LS_8023_EN		0x80
#define TXQ_TXF_BURST_NUM_SHIFT    	16 
#define TXQ_TXF_BURST_NUM_MASK     	0xFFFF

/* Jumbo packet Threshold for task offload */
#define REG_TX_TSO_OFFLOAD_THRESH	0x1594 /* In 8-bytes */
#define TX_TSO_OFFLOAD_THRESH_MASK	0x07FF

#define	REG_TXF_WATER_MARK		0x1598 /* In 8-bytes */
#define TXF_WATER_MARK_MASK		0x0FFF
#define TXF_LOW_WATER_MARK_SHIFT	0
#define TXF_HIGH_WATER_MARK_SHIFT 	16
#define TXQ_CTRL_BURST_MODE_EN		0x80000000

#define REG_THRUPUT_MON_CTRL		0x159C
#define THRUPUT_MON_RATE_MASK		0x3
#define THRUPUT_MON_RATE_SHIFT		0
#define THRUPUT_MON_EN			0x80

/* RXQ Control Register */
#define REG_RXQ_CTRL                	0x15A0
#define ASPM_THRUPUT_LIMIT_MASK		0x3
#define ASPM_THRUPUT_LIMIT_SHIFT	0
#define ASPM_THRUPUT_LIMIT_NO		0x00
#define ASPM_THRUPUT_LIMIT_1M		0x01
#define ASPM_THRUPUT_LIMIT_10M		0x02
#define ASPM_THRUPUT_LIMIT_100M		0x04
#define RXQ1_CTRL_EN			0x10
#define RXQ2_CTRL_EN			0x20
#define RXQ3_CTRL_EN			0x40
#define IPV6_CHKSUM_CTRL_EN		0x80
#define RSS_HASH_BITS_MASK		0x00FF
#define RSS_HASH_BITS_SHIFT		8
#define RSS_HASH_IPV4			0x10000
#define RSS_HASH_IPV4_TCP		0x20000
#define RSS_HASH_IPV6			0x40000
#define RSS_HASH_IPV6_TCP		0x80000
#define RXQ_RFD_BURST_NUM_MASK		0x003F
#define RXQ_RFD_BURST_NUM_SHIFT		20
#define RSS_MODE_MASK			0x0003
#define RSS_MODE_SHIFT			26
#define RSS_NIP_QUEUE_SEL_MASK		0x1
#define RSS_NIP_QUEUE_SEL_SHIFT		28
#define RRS_HASH_CTRL_EN		0x20000000
#define RX_CUT_THRU_EN			0x40000000
#define RXQ_CTRL_EN			0x80000000

#define REG_RFD_FREE_THRESH		0x15A4
#define RFD_FREE_THRESH_MASK		0x003F
#define RFD_FREE_HI_THRESH_SHIFT	0
#define RFD_FREE_LO_THRESH_SHIFT	6

/* RXF flow control register */
#define REG_RXQ_RXF_PAUSE_THRESH    	0x15A8
#define RXQ_RXF_PAUSE_TH_HI_SHIFT       0
#define RXQ_RXF_PAUSE_TH_HI_MASK        0x0FFF
#define RXQ_RXF_PAUSE_TH_LO_SHIFT       16
#define RXQ_RXF_PAUSE_TH_LO_MASK        0x0FFF

#define REG_RXD_DMA_CTRL		0x15AC
#define RXD_DMA_THRESH_MASK		0x0FFF	/* In 8-bytes */
#define RXD_DMA_THRESH_SHIFT		0
#define RXD_DMA_DOWN_TIMER_MASK		0xFFFF
#define RXD_DMA_DOWN_TIMER_SHIFT	16

/* DMA Engine Control Register */
#define REG_DMA_CTRL                	0x15C0
#define DMA_CTRL_DMAR_IN_ORDER          0x1
#define DMA_CTRL_DMAR_ENH_ORDER         0x2
#define DMA_CTRL_DMAR_OUT_ORDER         0x4
#define DMA_CTRL_RCB_VALUE              0x8
#define DMA_CTRL_DMAR_BURST_LEN_MASK    0x0007
#define DMA_CTRL_DMAR_BURST_LEN_SHIFT   4
#define DMA_CTRL_DMAW_BURST_LEN_MASK    0x0007
#define DMA_CTRL_DMAW_BURST_LEN_SHIFT   7
#define DMA_CTRL_DMAR_REQ_PRI           0x400
#define DMA_CTRL_DMAR_DLY_CNT_MASK      0x001F
#define DMA_CTRL_DMAR_DLY_CNT_SHIFT     11
#define DMA_CTRL_DMAW_DLY_CNT_MASK      0x000F
#define DMA_CTRL_DMAW_DLY_CNT_SHIFT     16
#define DMA_CTRL_CMB_EN               	0x100000
#define DMA_CTRL_SMB_EN			0x200000	
#define DMA_CTRL_CMB_NOW		0x400000
#define MAC_CTRL_SMB_DIS		0x1000000
#define DMA_CTRL_SMB_NOW		0x80000000

/* CMB/SMB Control Register */
#define REG_SMB_STAT_TIMER		0x15C4	/* 2us resolution */
#define SMB_STAT_TIMER_MASK		0xFFFFFF
#define REG_CMB_TPD_THRESH		0x15C8
#define CMB_TPD_THRESH_MASK		0xFFFF
#define REG_CMB_TX_TIMER		0x15CC	/* 2us resolution */
#define CMB_TX_TIMER_MASK		0xFFFF

/* Mail box */
#define MB_RFDX_PROD_IDX_MASK		0xFFFF
#define REG_MB_RFD0_PROD_IDX		0x15E0
#define REG_MB_RFD1_PROD_IDX		0x15E4
#define REG_MB_RFD2_PROD_IDX		0x15E8
#define REG_MB_RFD3_PROD_IDX		0x15EC

#define MB_PRIO_PROD_IDX_MASK		0xFFFF
#define REG_MB_PRIO_PROD_IDX		0x15F0
#define MB_HTPD_PROD_IDX_SHIFT		0
#define MB_NTPD_PROD_IDX_SHIFT		16

#define MB_PRIO_CONS_IDX_MASK		0xFFFF
#define REG_MB_PRIO_CONS_IDX		0x15F4
#define MB_HTPD_CONS_IDX_SHIFT		0
#define MB_NTPD_CONS_IDX_SHIFT		16

#define REG_MB_RFD01_CONS_IDX		0x15F8
#define MB_RFD0_CONS_IDX_MASK		0x0000FFFF
#define MB_RFD1_CONS_IDX_MASK		0xFFFF0000
#define REG_MB_RFD23_CONS_IDX		0x15FC
#define MB_RFD2_CONS_IDX_MASK		0x0000FFFF
#define MB_RFD3_CONS_IDX_MASK		0xFFFF0000
#define MB_RFD0_CONS_IDX_SHIFT		0
#define MB_RFD1_CONS_IDX_SHIFT		16
#define MB_RFD2_CONS_IDX_SHIFT		0
#define MB_RFD3_CONS_IDX_SHIFT		16

/* Interrupt Status Register */
#define REG_ISR    			0x1600
#define ISR_SMB				0x00000001
#define ISR_TIMER			0x00000002
/*
 * Software manual interrupt, for debug. Set when SW_MAN_INT_EN is set
 * in Table 51 Selene Master Control Register (Offset 0x1400).
 */
#define ISR_MANUAL         		0x00000004
#define ISR_HW_RXF_OV          		0x00000008 /* RXF overflow interrupt */
#define ISR_RFD0_UR			0x00000010 /* RFD0 under run */
#define ISR_RFD1_UR			0x00000020
#define ISR_RFD2_UR			0x00000040
#define ISR_RFD3_UR			0x00000080
#define ISR_TXF_UR			0x00000100
#define ISR_DMAR_TO_RST			0x00000200
#define ISR_DMAW_TO_RST			0x00000400
#define ISR_TX_CREDIT			0x00000800
#define ISR_GPHY			0x00001000
/* GPHY low power state interrupt */
#define ISR_GPHY_LPW           		0x00002000
#define ISR_TXQ_TO_RST			0x00004000
#define ISR_TX_PKT			0x00008000
#define ISR_RX_PKT_0			0x00010000
#define ISR_RX_PKT_1			0x00020000
#define ISR_RX_PKT_2			0x00040000
#define ISR_RX_PKT_3			0x00080000
#define ISR_MAC_RX			0x00100000
#define ISR_MAC_TX			0x00200000
#define ISR_UR_DETECTED			0x00400000
#define ISR_FERR_DETECTED		0x00800000
#define ISR_NFERR_DETECTED		0x01000000
#define ISR_CERR_DETECTED		0x02000000
#define ISR_PHY_LINKDOWN		0x04000000
#define ISR_DIS_INT			0x80000000

/* Interrupt Mask Register */
#define REG_IMR				0x1604

#define IMR_NORMAL_MASK		(\
		ISR_MANUAL	|\
		ISR_HW_RXF_OV	|\
		ISR_RFD0_UR	|\
		ISR_TXF_UR	|\
		ISR_DMAR_TO_RST	|\
		ISR_TXQ_TO_RST  |\
		ISR_DMAW_TO_RST	|\
		ISR_GPHY	|\
		ISR_TX_PKT	|\
		ISR_RX_PKT_0	|\
		ISR_GPHY_LPW    |\
		ISR_PHY_LINKDOWN)

#define ISR_RX_PKT 	(\
	ISR_RX_PKT_0    |\
	ISR_RX_PKT_1    |\
	ISR_RX_PKT_2    |\
	ISR_RX_PKT_3    )

#define ISR_OVER	(\
	ISR_RFD0_UR 	|\
	ISR_RFD1_UR	|\
	ISR_RFD2_UR	|\
	ISR_RFD3_UR	|\
	ISR_HW_RXF_OV	|\
	ISR_TXF_UR)	
	
#define ISR_ERROR	(\
	ISR_DMAR_TO_RST	|\
	ISR_TXQ_TO_RST  |\
	ISR_DMAW_TO_RST	|\
	ISR_PHY_LINKDOWN)

#define REG_INT_RETRIG_TIMER		0x1608
#define INT_RETRIG_TIMER_MASK		0xFFFF

#define REG_HDS_CTRL			0x160C
#define HDS_CTRL_EN			0x0001
#define HDS_CTRL_BACKFILLSIZE_SHIFT	8
#define HDS_CTRL_BACKFILLSIZE_MASK	0x0FFF
#define HDS_CTRL_MAX_HDRSIZE_SHIFT	20
#define HDS_CTRL_MAC_HDRSIZE_MASK	0x0FFF

#define REG_MAC_RX_STATUS_BIN 		0x1700
#define REG_MAC_RX_STATUS_END 		0x175c
#define REG_MAC_TX_STATUS_BIN 		0x1760
#define REG_MAC_TX_STATUS_END 		0x17c0

#define REG_CLK_GATING_CTRL		0x1814
#define CLK_GATING_DMAW_EN		0x0001
#define CLK_GATING_DMAR_EN		0x0002
#define CLK_GATING_TXQ_EN		0x0004
#define CLK_GATING_RXQ_EN		0x0008
#define CLK_GATING_TXMAC_EN		0x0010
#define CLK_GATING_RXMAC_EN		0x0020

#define CLK_GATING_EN_ALL	(CLK_GATING_DMAW_EN |\
				 CLK_GATING_DMAR_EN |\
				 CLK_GATING_TXQ_EN  |\
				 CLK_GATING_RXQ_EN  |\
				 CLK_GATING_TXMAC_EN|\
				 CLK_GATING_RXMAC_EN) 

/* DEBUG ADDR */
#define REG_DEBUG_DATA0 		0x1900
#define REG_DEBUG_DATA1 		0x1904

/* PHY Control Register */
#define MII_BMCR			0x00
#define BMCR_SPEED_SELECT_MSB		0x0040  /* bits 6,13: 10=1000, 01=100, 00=10 */
#define BMCR_COLL_TEST_ENABLE		0x0080  /* Collision test enable */
#define BMCR_FULL_DUPLEX		0x0100  /* FDX =1, half duplex =0 */
#define BMCR_RESTART_AUTO_NEG		0x0200  /* Restart auto negotiation */
#define BMCR_ISOLATE			0x0400  /* Isolate PHY from MII */
#define BMCR_POWER_DOWN			0x0800  /* Power down */
#define BMCR_AUTO_NEG_EN		0x1000  /* Auto Neg Enable */
#define BMCR_SPEED_SELECT_LSB		0x2000  /* bits 6,13: 10=1000, 01=100, 00=10 */
#define BMCR_LOOPBACK			0x4000  /* 0 = normal, 1 = loopback */
#define BMCR_RESET			0x8000  /* 0 = normal, 1 = PHY reset */
#define BMCR_SPEED_MASK			0x2040
#define BMCR_SPEED_1000			0x0040
#define BMCR_SPEED_100			0x2000
#define BMCR_SPEED_10			0x0000

/* PHY Status Register */
#define MII_BMSR			0x01
#define BMMSR_EXTENDED_CAPS		0x0001  /* Extended register capabilities */
#define BMSR_JABBER_DETECT		0x0002  /* Jabber Detected */
#define BMSR_LINK_STATUS		0x0004  /* Link Status 1 = link */
#define BMSR_AUTONEG_CAPS		0x0008  /* Auto Neg Capable */
#define BMSR_REMOTE_FAULT		0x0010  /* Remote Fault Detect */
#define BMSR_AUTONEG_COMPLETE		0x0020  /* Auto Neg Complete */
#define BMSR_PREAMBLE_SUPPRESS		0x0040  /* Preamble may be suppressed */
#define BMSR_EXTENDED_STATUS		0x0100  /* Ext. status info in Reg 0x0F */
#define BMSR_100T2_HD_CAPS		0x0200  /* 100T2 Half Duplex Capable */
#define BMSR_100T2_FD_CAPS		0x0400  /* 100T2 Full Duplex Capable */
#define BMSR_10T_HD_CAPS		0x0800  /* 10T   Half Duplex Capable */
#define BMSR_10T_FD_CAPS		0x1000  /* 10T   Full Duplex Capable */
#define BMSR_100X_HD_CAPS		0x2000  /* 100X  Half Duplex Capable */
#define BMMII_SR_100X_FD_CAPS		0x4000  /* 100X  Full Duplex Capable */
#define BMMII_SR_100T4_CAPS		0x8000  /* 100T4 Capable */

#define MII_PHYSID1			0x02
#define MII_PHYSID2			0x03
#define L1D_MPW_PHYID1 			0xD01C  /* V7 */
#define L1D_MPW_PHYID2 			0xD01D  /* V1-V6 */
#define L1D_MPW_PHYID3 			0xD01E  /* V8 */


/* Autoneg Advertisement Register */
#define MII_ADVERTISE			0x04
#define ADVERTISE_SELECTOR_FIELD	0x0001  /* indicates IEEE 802.3 CSMA/CD */
#define ADVERTISE_10T_HD_CAPS		0x0020  /* 10T   Half Duplex Capable */
#define ADVERTISE_10T_FD_CAPS		0x0040  /* 10T   Full Duplex Capable */
#define ADVERTISE_100TX_HD_CAPS		0x0080  /* 100TX Half Duplex Capable */
#define ADVERTISE_100TX_FD_CAPS		0x0100  /* 100TX Full Duplex Capable */
#define ADVERTISE_100T4_CAPS		0x0200  /* 100T4 Capable */
#define ADVERTISE_PAUSE			0x0400  /* Pause operation desired */
#define ADVERTISE_ASM_DIR		0x0800  /* Asymmetric Pause Direction bit */
#define ADVERTISE_REMOTE_FAULT		0x2000  /* Remote Fault detected */
#define ADVERTISE_NEXT_PAGE		0x8000  /* Next Page ability supported */
#define ADVERTISE_SPEED_MASK		0x01E0
#define ADVERTISE_DEFAULT_CAP		0x0DE0

/* 1000BASE-T Control Register */
#define MII_GIGA_CR			0x09
#define GIGA_CR_1000T_HD_CAPS		0x0100  /* Advertise 1000T HD capability */
#define GIGA_CR_1000T_FD_CAPS		0x0200  /* Advertise 1000T FD capability  */
#define GIGA_CR_1000T_REPEATER_DTE	0x0400  /* 1=Repeater/switch device port
		  				   0=DTE device */

#define GIGA_CR_1000T_MS_VALUE		0x0800  /* 1=Configure PHY as Master 
		    				   0=Configure PHY as Slave */
#define GIGA_CR_1000T_MS_ENABLE		0x1000  /* 1=Master/Slave manual config value 
		    				   0=Automatic Master/Slave config */
#define GIGA_CR_1000T_TEST_MODE_NORMAL	0x0000  /* Normal Operation */
#define GIGA_CR_1000T_TEST_MODE_1	0x2000  /* Transmit Waveform test */
#define GIGA_CR_1000T_TEST_MODE_2	0x4000  /* Master Transmit Jitter test */
#define GIGA_CR_1000T_TEST_MODE_3	0x6000  /* Slave Transmit Jitter test */
#define GIGA_CR_1000T_TEST_MODE_4	0x8000  /* Transmitter Distortion test */
#define GIGA_CR_1000T_SPEED_MASK	0x0300
#define GIGA_CR_1000T_DEFAULT_CAP	0x0300

/* PHY Specific Status Register */
#define MII_GIGA_PSSR			0x11
#define GIGA_PSSR_SPD_DPLX_RESOLVED	0x0800  /* 1=Speed & Duplex resolved */
#define GIGA_PSSR_DPLX			0x2000  /* 1=Duplex 0=Half Duplex */
#define GIGA_PSSR_SPEED			0xC000  /* Speed, bits 14:15 */
#define GIGA_PSSR_10MBS			0x0000  /* 00=10Mbs */
#define GIGA_PSSR_100MBS		0x4000  /* 01=100Mbs */
#define GIGA_PSSR_1000MBS		0x8000  /* 10=1000Mbs */

/* PHY Interrupt Enable Register */
#define MII_IER				0x12
#define IER_LINK_UP			0x0400
#define IER_LINK_DOWN			0x0800

/* PHY Interrupt Status Register */
#define MII_ISR				0x13
#define ISR_LINK_UP			0x0400
#define ISR_LINK_DOWN			0x0800

/* Cable-Detect-Test Control Register */
#define MII_CDTC			0x16
#define CDTC_EN_OFF			0   /* sc */
#define CDTC_EN_BITS			1
#define CDTC_PAIR_OFF			8
#define CDTC_PAIR_BIT			2

/* Cable-Detect-Test Status Register */
#define MII_CDTS			0x1C
#define CDTS_STATUS_OFF			8
#define CDTS_STATUS_BITS		2
#define CDTS_STATUS_NORMAL		0
#define CDTS_STATUS_SHORT		1
#define CDTS_STATUS_OPEN		2
#define CDTS_STATUS_INVALID		3

#define MII_DBG_ADDR			0x1D
#define MII_DBG_DATA			0x1E
/*********************PCI Command Register ******************************/
// PCI Command Register Bit Definitions
#define PCI_REG_COMMAND         0x04    // PCI Command Register
#define CMD_IO_SPACE            0x0001
#define CMD_MEMORY_SPACE        0x0002
#define CMD_BUS_MASTER          0x0004

/************************************************************************************************************************/

/* This defines the direction arg to the DMA mapping routines. */
#ifndef PCI_DMA_BIDIRECTIONAL
#define PCI_DMA_BIDIRECTIONAL   0
#define PCI_DMA_TODEVICE    1
#define PCI_DMA_FROMDEVICE  2
#define PCI_DMA_NONE        3
#endif

/* The size (in bytes) of a ethernet packet */
#define ENET_HEADER_SIZE                14
#define MAXIMUM_ETHERNET_FRAME_SIZE     1518 /* with FCS */
#define MINIMUM_ETHERNET_FRAME_SIZE     64   /* with FCS */
#define ETHERNET_FCS_SIZE               4
//#define MAX_JUMBO_FRAME_SIZE            0x2000
#define VLAN_SIZE                                               4
#ifndef VLAN_HLEN
#define VLAN_HLEN 4
#endif

#define PHY_AUTO_NEG_TIME               45          /* 4.5 Seconds */
#define PHY_FORCE_TIME                  20          /* 2.0 Seconds */

/* For checksumming , the sum of all words in the EEPROM should equal 0xBABA */
#define EEPROM_SUM                      0xBABA
#define NODE_ADDRESS_SIZE               6
#endif
