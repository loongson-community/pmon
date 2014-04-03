/*******************************************************************************

  Intel PRO/1000 Linux driver
  Copyright(c) 1999 - 2008 Intel Corporation.

  This program is free software; you can redistribute it and/or modify it
  under the terms and conditions of the GNU General Public License,
  version 2, as published by the Free Software Foundation.

  This program is distributed in the hope it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
  more details.

  You should have received a copy of the GNU General Public License along with
  this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA.

  The full GNU General Public License is included in this distribution in
  the file called "COPYING".

  Contact Information:
  Linux NICS <linux.nics@intel.com>
  e1000-devel Mailing List <e1000-devel@lists.sourceforge.net>
  Intel Corporation, 5200 N.E. Elam Young Parkway, Hillsboro, OR 97124-6497

*******************************************************************************/

/* Linux PRO/1000 Ethernet Driver main header file */

#ifndef _E1000_H_
#define _E1000_H_

#ifndef PMON //zxj
#include <linux/types.h>
#include <linux/timer.h>
#include <linux/workqueue.h>
#include <linux/io.h>
#include <linux/netdevice.h>
#endif

#include "hw.h"

#if 1//zxj
#define PCI_CAP_ID_EXP          0x10
#define ETH_ALEN    6
#define SPEED_10            10
#define SPEED_100           100
#define CHECKSUM_NONE 0
#define CHECKSUM_UNNECESSARY 1
#define CHECKSUM_COMPLETE 2
#define NET_IP_ALIGN    2
#define GFP_ATOMIC 0x20u
enum km_type {
   KM_BOUNCE_READ,
   KM_SKB_SUNRPC_DATA,
   KM_SKB_DATA_SOFTIRQ,
   KM_USER0,
   KM_USER1,
   KM_BIO_SRC_IRQ,
   KM_BIO_DST_IRQ,
   KM_PTE0,
   KM_PTE1,
   KM_IRQ0,
   KM_IRQ1,
   KM_SOFTIRQ0,
   KM_SOFTIRQ1,
   KM_TYPE_NR
};
#define true 1
#define false 0
#define ETH_ALEN    6       /* Octets in one ethernet addr   */
#define ETH_HLEN    14      /* Total octets in header.   */
#define ETH_ZLEN    60      /* Min. octets in frame sans FCS */
#define ETH_DATA_LEN    1500        /* Max. octets in payload    */
#define ETH_FRAME_LEN   1514        /* Max. octets in frame sans FCS */
#define ETH_FCS_LEN 4  

#define IRQF_DISABLED       0x00000020
#define IRQF_SAMPLE_RANDOM  0x00000040
#define IRQF_SHARED     0x00000080
#define IRQF_PROBE_SHARED   0x00000100
#define IRQF_TIMER      0x00000200
#define IRQF_PERCPU     0x00000400
#define IRQF_NOBALANCING    0x00000800
#define IRQF_IRQPOLL        0x00001000

#define VLAN_GROUP_ARRAY_LEN          4096
#define VLAN_GROUP_ARRAY_SPLIT_PARTS  8
#define VLAN_GROUP_ARRAY_PART_LEN     (VLAN_GROUP_ARRAY_LEN/VLAN_GROUP_ARRAY_SPLIT_PARTS)

#define DMA_32BIT_MASK  0x00000000ffffffffULL

#define PAGE_SHIFT  14

#define PM_QOS_RESERVED 0
#define PM_QOS_CPU_DMA_LATENCY 1
#define PM_QOS_NETWORK_LATENCY 2
#define PM_QOS_NETWORK_THROUGHPUT 3
 
#define PM_QOS_NUM_CLASSES 4
#define PM_QOS_DEFAULT_VALUE -1

#define ETH_P_8021Q 0x8100          /* 802.1Q VLAN Extended Header  */

#define VLAN_HLEN   4       /* The additional bytes (on top of the Ethernet header)*/
#define  PCI_COMMAND_SERR   0x100   /* Enable SERR */
#define BMCR_ANENABLE       0x1000    // Enable auto negotiation
#define BMCR_FULLDPLX           0x0100  /* Full duplex                 */
#define BMSR_100FULL            0x4000  /* Can do 100mbps, full-duplex */
#define BMSR_100HALF            0x2000  /* Can do 100mbps, half-duplex */
#define BMSR_10FULL             0x1000  /* Can do 10mbps, full-duplex  */
#define BMSR_10HALF             0x0800  /* Can do 10mbps, half-duplex  */
#define BMSR_ESTATEN        0x0100  /* Extended Status in R15 */
#define BMSR_ANEGCAPABLE        0x0008  /* Able to do auto-negotiation */
#define BMSR_ERCAP              0x0001  /* Ext-reg capability          */
#define BMSR_JCD                0x0002  /* Jabber detected             */
#define BMSR_LSTATUS            0x0004  /* Link status                 */

#define ADVERTISE_SLCT          0x001f  /* Selector bits               */
#define ADVERTISE_CSMA          0x0001  /* Only selector supported     */
#define ADVERTISE_10HALF        0x0020  /* Try for 10mbps half-duplex  */
#define ADVERTISE_1000XFULL     0x0020  /* Try for 1000BASE-X full-duplex */
#define ADVERTISE_10FULL        0x0040  /* Try for 10mbps full-duplex  */
#define ADVERTISE_1000XHALF     0x0040  /* Try for 1000BASE-X half-duplex */
#define ADVERTISE_100HALF       0x0080  /* Try for 100mbps half-duplex */
#define ADVERTISE_1000XPAUSE    0x0080  /* Try for 1000BASE-X pause    */
#define ADVERTISE_100FULL       0x0100  /* Try for 100mbps full-duplex */
#define ADVERTISE_1000XPSE_ASYM 0x0100  /* Try for 1000BASE-X asym pause */
#define ADVERTISE_100BASE4      0x0200  /* Try for 100mbps 4k packets  */
#define ADVERTISE_PAUSE_CAP     0x0400  /* Try for pause               */
#define ADVERTISE_PAUSE_ASYM    0x0800  /* Try for asymetric pause     */
#define ADVERTISE_RESV          0x1000  /* Unused...                   */
#define ADVERTISE_RFAULT        0x2000  /* Say we can detect faults    */
#define ADVERTISE_LPACK         0x4000  /* Ack link partners response  */
#define ADVERTISE_NPAGE         0x8000  /* Next page bit               */

#define ADVERTISE_FULL (ADVERTISE_100FULL | ADVERTISE_10FULL | \
             ADVERTISE_CSMA)
#define ADVERTISE_ALL (ADVERTISE_10HALF | ADVERTISE_10FULL | \
                        ADVERTISE_100HALF | ADVERTISE_100FULL)

#define EXPANSION_NWAY          0x0001  /* Can do N-way auto-nego      */
#define EXPANSION_LCWP          0x0002  /* Got new RX page code word   */
#define EXPANSION_ENABLENPAGE   0x0004  /* This enables npage words    */
#define EXPANSION_NPCAPABLE     0x0008  /* Link partner supports npage */
#define EXPANSION_MFAULTS       0x0010  /* Multiple faults detected    */
#define EXPANSION_RESV          0xffe0  /* Unused...                   */

#define ADVERTISE_1000FULL      0x0200  /* Advertise 1000BASE-T full duplex */
#define ADVERTISE_1000HALF      0x0100  /* Advertise 1000BASE-T half duplex */

#define ESTATUS_1000_TFULL  0x2000  /* Can do 1000BT Full */
#define ESTATUS_1000_THALF  0x1000  /* Can do 1000BT Half */

enum {
     SKB_GSO_TCPV4 = 1 << 0,
     SKB_GSO_UDP = 1 << 1,
 
     /* This indicates the skb is from an untrusted source. */
     SKB_GSO_DODGY = 1 << 2,
 
     /* This indicates the tcp segment has CWR set. */
     SKB_GSO_TCP_ECN = 1 << 3,
 
     SKB_GSO_TCPV6 = 1 << 4,
};
#define NETIF_F_GSO_SHIFT   16
#define NETIF_F_GSO_MASK    0xffff0000
#define NETIF_F_TSO     (SKB_GSO_TCPV4 << NETIF_F_GSO_SHIFT)
#define NETIF_F_UFO     (SKB_GSO_UDP << NETIF_F_GSO_SHIFT)
#define NETIF_F_GSO_ROBUST  (SKB_GSO_DODGY << NETIF_F_GSO_SHIFT)
#define NETIF_F_TSO_ECN     (SKB_GSO_TCP_ECN << NETIF_F_GSO_SHIFT)
#define NETIF_F_TSO6        (SKB_GSO_TCPV6 << NETIF_F_GSO_SHIFT)

#define ETH_P_IP    0x0800      /* Internet Protocol packet */
#define CHECKSUM_NONE 0
#define CHECKSUM_UNNECESSARY 1
#define CHECKSUM_COMPLETE 2
#define CHECKSUM_PARTIAL 3

#define NETDEV_TX_OK 0      /* driver took care of packet */
#define NETDEV_TX_BUSY 1    /* driver tx path was busy*/
#define NETDEV_TX_LOCKED -1 /* driver tx lock was already taken */

#define SIOCGMIIPHY 0x8947      /* Get address of MII PHY in use. */
#define SIOCGMIIREG 0x8948      /* Read MII PHY register.   */
#define SIOCSMIIREG 0x8949      /* Write MII PHY register.  */

#define CAP_NET_ADMIN        12

#define MII_BMCR            0x00        /* Basic mode control register */
#define MII_BMSR            0x01        /* Basic mode status register  */
#define MII_PHYSID1         0x02        /* PHYS ID 1                   */
#define MII_PHYSID2         0x03        /* PHYS ID 2                   */
#define MII_ADVERTISE       0x04        /* Advertisement control reg   */
#define MII_LPA             0x05        /* Link partner ability reg    */
#define MII_EXPANSION       0x06        /* Expansion register          */
#define MII_CTRL1000        0x09        /* 1000BASE-T control          */
#define MII_STAT1000        0x0a        /* 1000BASE-T status           */
#define MII_ESTATUS     0x0f    /* Extended Status */
#define MII_DCOUNTER        0x12        /* Disconnect counter          */
#define MII_FCSCOUNTER      0x13        /* False carrier counter       */
#define MII_NWAYTEST        0x14        /* N-way auto-neg test reg     */
#define MII_RERRCOUNTER     0x15        /* Receive error counter       */
#define MII_SREVISION       0x16        /* Silicon revision            */
#define MII_RESV1           0x17        /* Reserved...                 */
#define MII_LBRERROR        0x18        /* Lpback, rx, bypass error    */
#define MII_PHYADDR         0x19        /* PHY address                 */
#define MII_RESV2           0x1a        /* Reserved...                 */
#define MII_TPISTATUS       0x1b        /* TPI status for 10mbps       */
#define MII_NCONFIG         0x1c        /* Network interface config    */

#define PCI_D3hot   3
#define PCI_D3cold  4

#define PCI_EXP_LNKCTL      16  /* Link Control */

typedef int pci_power_t;

#define PMSG_SUSPEND    ((struct pm_message){ .event = PM_EVENT_SUSPEND, })

#define PM_EVENT_ON     0x0000
#define PM_EVENT_FREEZE     0x0001
#define PM_EVENT_SUSPEND    0x0002
#define PM_EVENT_HIBERNATE  0x0004
#define PM_EVENT_QUIESCE    0x0008
#define PM_EVENT_RESUME     0x0010
#define PM_EVENT_THAW       0x0020
#define PM_EVENT_RESTORE    0x0040
#define PM_EVENT_RECOVER    0x0080
#define PM_EVENT_USER       0x0100
#define PM_EVENT_REMOTE     0x0200
#define PM_EVENT_AUTO       0x0400

#define PM_EVENT_SLEEP      (PM_EVENT_SUSPEND | PM_EVENT_HIBERNATE)
#define PM_EVENT_USER_SUSPEND   (PM_EVENT_USER | PM_EVENT_SUSPEND)
#define PM_EVENT_USER_RESUME    (PM_EVENT_USER | PM_EVENT_RESUME)
#define PM_EVENT_REMOTE_WAKEUP  (PM_EVENT_REMOTE | PM_EVENT_RESUME)
#define PM_EVENT_AUTO_SUSPEND   (PM_EVENT_AUTO | PM_EVENT_SUSPEND)
#define PM_EVENT_AUTO_RESUME    (PM_EVENT_AUTO | PM_EVENT_RESUME)

typedef unsigned int pci_ers_result_t;

typedef unsigned int pci_channel_state_t;

typedef unsigned int resource_size_t;

#   define DMA_64BIT_MASK  0xffffffffffffffffULL
enum {
     NETIF_MSG_DRV       = 0x0001,
     NETIF_MSG_PROBE     = 0x0002,
     NETIF_MSG_LINK      = 0x0004,
     NETIF_MSG_TIMER     = 0x0008,
     NETIF_MSG_IFDOWN    = 0x0010,
     NETIF_MSG_IFUP      = 0x0020,
     NETIF_MSG_RX_ERR    = 0x0040,
     NETIF_MSG_TX_ERR    = 0x0080,
     NETIF_MSG_TX_QUEUED = 0x0100,
     NETIF_MSG_INTR      = 0x0200,
     NETIF_MSG_TX_DONE   = 0x0400,
     NETIF_MSG_RX_STATUS = 0x0800,
     NETIF_MSG_PKTDATA   = 0x1000,
     NETIF_MSG_HW        = 0x2000,
     NETIF_MSG_WOL       = 0x4000,
 };

enum pci_ers_result {
     /* no result/none/not supported in device driver */
     PCI_ERS_RESULT_NONE =  1,
 
     /* Device driver can recover without slot reset */
     PCI_ERS_RESULT_CAN_RECOVER =  2,
 
     /* Device driver wants slot to be reset. */
     PCI_ERS_RESULT_NEED_RESET =  3,
 
     /* Device has completely failed, is unrecoverable */
     PCI_ERS_RESULT_DISCONNECT =  4,
 
     /* Device driver is fully recovered and operational */
     PCI_ERS_RESULT_RECOVERED =  5,
};

#define PCI_VDEVICE(vendor, device)     \
     PCI_VENDOR_ID_##vendor, (device),   \
     PCI_ANY_ID, PCI_ANY_ID, 0, 0

#define PCI_VENDOR_ID_INTEL     0x8086

#define ETH_GSTRING_LEN     32

#define SUPPORTED_10baseT_Half      (1 << 0)
 #define SUPPORTED_10baseT_Full      (1 << 1)
 #define SUPPORTED_100baseT_Half     (1 << 2)
 #define SUPPORTED_100baseT_Full     (1 << 3)
 #define SUPPORTED_1000baseT_Half    (1 << 4)
 #define SUPPORTED_1000baseT_Full    (1 << 5)
 #define SUPPORTED_Autoneg       (1 << 6)
 #define SUPPORTED_TP            (1 << 7)
 #define SUPPORTED_AUI           (1 << 8)
 #define SUPPORTED_MII           (1 << 9)
 #define SUPPORTED_FIBRE         (1 << 10)
 #define SUPPORTED_BNC           (1 << 11)
 #define SUPPORTED_10000baseT_Full   (1 << 12)
 #define SUPPORTED_Pause         (1 << 13)
 #define SUPPORTED_Asym_Pause        (1 << 14)
 #define SUPPORTED_2500baseX_Full    (1 << 15)

#define ADVERTISED_10baseT_Half     (1 << 0)
 #define ADVERTISED_10baseT_Full     (1 << 1)
 #define ADVERTISED_100baseT_Half    (1 << 2)
 #define ADVERTISED_100baseT_Full    (1 << 3)
 #define ADVERTISED_1000baseT_Half   (1 << 4)
 #define ADVERTISED_1000baseT_Full   (1 << 5)
 #define ADVERTISED_Autoneg      (1 << 6)
 #define ADVERTISED_TP           (1 << 7)
 #define ADVERTISED_AUI          (1 << 8)
 #define ADVERTISED_MII          (1 << 9)
 #define ADVERTISED_FIBRE        (1 << 10)
 #define ADVERTISED_BNC          (1 << 11)
 #define ADVERTISED_10000baseT_Full  (1 << 12)
 #define ADVERTISED_Pause        (1 << 13)
 #define ADVERTISED_Asym_Pause       (1 << 14)
 #define ADVERTISED_2500baseX_Full   (1 << 15)

#define PORT_TP         0x00
 #define PORT_AUI        0x01
 #define PORT_MII        0x02
 #define PORT_FIBRE      0x03
 #define PORT_BNC        0x04

#define XCVR_INTERNAL       0x00
 #define XCVR_EXTERNAL       0x01
 #define XCVR_DUMMY1     0x02
 #define XCVR_DUMMY2     0x03
 #define XCVR_DUMMY3     0x04

#define DUPLEX_HALF     0x00
#define DUPLEX_FULL     0x01

#define AUTONEG_DISABLE     0x00
#define AUTONEG_ENABLE      0x01

#define EEPROM_CHECKSUM_REG           0x003F

typedef struct skb_frag_struct skb_frag_t;
 
 struct skb_frag_struct {
     struct page *page;
     __u32 page_offset;
     __u32 size;
 };

struct skb_shared_info {
     atomic_t    dataref;
     unsigned short  nr_frags;
     unsigned short  gso_size;
     /* Warning: this field is not always filled in (UFO)! */
     unsigned short  gso_segs;
     unsigned short  gso_type;
     __be32          ip6_frag_id;
     struct sk_buff  *frag_list;
     skb_frag_t  frags[MAX_SKB_FRAGS];
 };

#define skb_shinfo(SKB) ((struct skb_shared_info *)(skb_end_pointer(SKB)))

static inline unsigned char *skb_network_header(const struct sk_buff *skb)
{
     return (unsigned char*)((u32)skb->head + (u32)skb->network_header);
}
static inline struct iphdr *ip_hdr(const struct sk_buff *skb)
{
     return (struct iphdr *)skb_network_header(skb);
}

static inline struct tcphdr *tcp_hdr(const struct sk_buff *skb)
{
     return (struct tcphdr *)skb_transport_header(skb);
}
struct napi_struct {
   /* The poll_list must only be managed by the entity which
      * changes the state of the NAPI_STATE_SCHED bit.  This means
      * whoever atomically sets that bit can add this napi_struct
      * to the per-cpu poll_list, and whoever clears that bit
      * can remove from the list right before clearing the bit.
   */
   struct list_head    poll_list;
 
   unsigned long       state;
   int         weight;
   int         (*poll)(struct napi_struct *, int);
 };

struct e1000_desc_ring {
     /* pointer to the descriptor ring memory */
     void *desc;
     /* physical address of the descriptor ring */
     dma_addr_t dma;  
     /* length of descriptor ring in bytes */
     unsigned int size;
     /* number of descriptors in the ring */
     unsigned int count;
     /* next descriptor to associate a buffer with */
     unsigned int next_to_use;
     /* next descriptor to check for DD status bit */
     unsigned int next_to_clean;
     /* array of buffer information structs */
     struct e1000_buffer *buffer_info;
	u8 * head, *tail;
};  

struct msix_entry {
     u16     vector; /* kernel uses to write allocated vector */
     u16 entry;  /* driver uses to specify entry, OS writes */
 };

static inline void msleep(int microseconds){
 int i;
 for(i=0;i<microseconds;i++)delay(1000);
}
#define netif_receive_skb(x) netif_rx(x)
#define pci_dma_mapping_error(x,y) (0)
#define VLAN_TAG_SIZE                     4     /* 802.3ac tag (not DMAed) */
#define CARRIER_EXTENSION   0x0F
#define TBI_ACCEPT(adapter, status, errors, length, last_byte) \
     ((adapter)->tbi_compatibility_on && \
      (((errors) & E1000_RXD_ERR_FRAME_ERR_MASK) == E1000_RXD_ERR_CE) && \
      ((last_byte) == CARRIER_EXTENSION) && \
      (((status) & E1000_RXD_STAT_VP) ? \
           (((length) > ((adapter)->min_frame_size - VLAN_TAG_SIZE)) && \
            ((length) <= ((adapter)->max_frame_size + 1))) : \
           (((length) > (adapter)->min_frame_size) && \
            ((length) <= ((adapter)->max_frame_size + VLAN_TAG_SIZE + 1)))))

#define netdev_priv(dev) dev->priv
struct hlist_node {            
     struct hlist_node *next, **pprev;
};
struct rcu_head {
     struct rcu_head *next;
     void (*func)(struct rcu_head *head);
 };
struct vlan_group {
     struct net_device   *real_dev; /* The ethernet(like) device
                         * the vlan is attached to.
                         */
     unsigned int        nr_vlans;
     struct hlist_node   hlist;  /* linked list */
     struct net_device **vlan_devices_arrays[VLAN_GROUP_ARRAY_SPLIT_PARTS];
     struct rcu_head     rcu;
 };
static inline void vlan_group_set_device(struct vlan_group *vg,
                      u16 vlan_id,
                      struct net_device *dev)
 {
     struct net_device **array;
     if (!vg)
         return;
     array = vg->vlan_devices_arrays[vlan_id / VLAN_GROUP_ARRAY_PART_LEN];
     array[vlan_id % VLAN_GROUP_ARRAY_PART_LEN] = dev;
 }
static inline struct net_device *vlan_group_get_device(struct vlan_group *vg,
                                u16 vlan_id)
 {
     struct net_device **array;
     array = vg->vlan_devices_arrays[vlan_id / VLAN_GROUP_ARRAY_PART_LEN];
     return array ? array[vlan_id % VLAN_GROUP_ARRAY_PART_LEN] : NULL;
 }
static inline void napi_enable(struct napi_struct *n)
 {
     clear_bit(0, &n->state);
 }
static inline void napi_disable(struct napi_struct *n)
 {
     set_bit(1, &n->state);
     while (test_and_set_bit(1, &n->state))
         msleep(1);
     clear_bit(1, &n->state);
 }
#define E1000_DESC_UNUSED(R)   ((((R)->next_to_clean > (R)->next_to_use) ? 0 : (R)->count) + (R)->next_to_clean - (R)->next_to_use - 1)
#define IGP3_VR_CTRL    PHY_REG(776, 18) /* Voltage Regulator Control */
#define IGP3_VR_CTRL_DEV_POWERDOWN_MODE_MASK 0x0300
#define IGP3_VR_CTRL_MODE_SHUTDOWN  0x0200
#endif

struct e1000_info;

#define e_printk(level, adapter, format, arg...) \
	printk(level "%s: %s: " format, e1000e_driver_name, \
	       adapter->netdev->name, ## arg) //zxj modified

#ifdef DEBUG
#define e_dbg(format, arg...) \
	e_printk(KERN_DEBUG , adapter, format, ## arg)
#else
#define e_dbg(format, arg...) do { (void)(adapter); } while (0)
#endif

#define e_err(format, arg...) \
	e_printk(KERN_ERR, adapter, format, ## arg)
#define e_info(format, arg...) \
	e_printk(KERN_INFO, adapter, format, ## arg)
#define e_warn(format, arg...) \
	e_printk(KERN_WARNING, adapter, format, ## arg)
#define e_notice(format, arg...) \
	e_printk(KERN_NOTICE, adapter, format, ## arg)


/* Interrupt modes, as used by the IntMode paramter */
#define E1000E_INT_MODE_LEGACY		0
#define E1000E_INT_MODE_MSI		1
#define E1000E_INT_MODE_MSIX		2

/* Tx/Rx descriptor defines */
#define E1000_DEFAULT_TXD		256
#define E1000_MAX_TXD			4096
#define E1000_MIN_TXD			64

#define E1000_DEFAULT_RXD		256
#define E1000_MAX_RXD			4096
#define E1000_MIN_RXD			64

#define E1000_MIN_ITR_USECS		10 /* 100000 irq/sec */
#define E1000_MAX_ITR_USECS		10000 /* 100    irq/sec */

/* Early Receive defines */
#define E1000_ERT_2048			0x100

#define E1000_FC_PAUSE_TIME		0x0680 /* 858 usec */

/* How many Tx Descriptors do we need to call netif_wake_queue ? */
/* How many Rx Buffers do we bundle into one write to the hardware ? */
#define E1000_RX_BUFFER_WRITE		16 /* Must be power of 2 */

#define AUTO_ALL_MODES			0
#define E1000_EEPROM_APME		0x0400

#define E1000_MNG_VLAN_NONE		(-1)

/* Number of packet split data buffers (not including the header buffer) */
#define PS_PAGE_BUFFERS			(MAX_PS_BUFFERS - 1)

enum e1000_boards {
	board_82571,
	board_82572,
	board_82573,
	board_82574,
	board_80003es2lan,
	board_ich8lan,
	board_ich9lan,
};

struct e1000_queue_stats {
	u64 packets;
	u64 bytes;
};

struct e1000_ps_page {
	struct page *page;
	u64 dma; /* must be u64 - written to hw */
};

/*
 * wrappers around a pointer to a socket buffer,
 * so a DMA handle can be stored along with the buffer
 */
#if 0 //zxj
struct e1000_buffer {
	dma_addr_t dma;
	struct sk_buff *skb;
	union {
		/* Tx */
		struct {
			unsigned long time_stamp;
			u16 length;
			u16 next_to_watch;
		};
		/* Rx */
		/* arrays of page information for packet split */
		struct e1000_ps_page *ps_pages;
	};
	struct page *page;
};
#else
struct e1000_buffer {
    struct sk_buff *skb;
    uint64_t dma;
    unsigned long length;
    unsigned long time_stamp;
    unsigned int next_to_watch;
};
#endif

struct e1000_ring {
	void *desc;			/* pointer to ring memory  */
	dma_addr_t dma;			/* phys address of ring    */
	unsigned int size;		/* length of ring in bytes */
	unsigned int count;		/* number of desc. in ring */

	unsigned int next_to_use;
	unsigned int next_to_clean;

	u16 head;
	u16 tail;

	/* array of buffer information structs */
	struct e1000_buffer *buffer_info;

	char name[IFNAMSIZ + 5];
	u32 ims_val;
	u32 itr_val;
	u16 itr_register;
	int set_itr;

	struct sk_buff *rx_skb_top;

	struct e1000_queue_stats stats;
};

/* PHY register snapshot values */
struct e1000_phy_regs {
	u16 bmcr;		/* basic mode control register    */
	u16 bmsr;		/* basic mode status register     */
	u16 advertise;		/* auto-negotiation advertisement */
	u16 lpa;		/* link partner ability register  */
	u16 expansion;		/* auto-negotiation expansion reg */
	u16 ctrl1000;		/* 1000BASE-T control register    */
	u16 stat1000;		/* 1000BASE-T status register     */
	u16 estatus;		/* extended status register       */
};

typedef void (*work_func_t)(struct work_struct *work);

struct work_struct {
     atomic_t data;
 #define WORK_STRUCT_PENDING 0       /* T if work item pending execution */
 #define WORK_STRUCT_FLAG_MASK (3UL)
 #define WORK_STRUCT_WQ_DATA_MASK (~WORK_STRUCT_FLAG_MASK)
     struct list_head entry;
     work_func_t func;
 #ifdef CONFIG_LOCKDEP
     struct lockdep_map lockdep_map;
 #endif
 };
/* board specific private data structure */
struct e1000_adapter {
	uint32_t smartspeed; //zxj
	struct timer_list watchdog_timer;
	struct timer_list phy_info_timer;
	struct timer_list blink_timer;

	struct work_struct reset_task;
	struct work_struct watchdog_task;

	const struct e1000_info *ei;

	struct vlan_group *vlgrp;
	u32 bd_number;
	u32 rx_buffer_len;
	u16 mng_vlan_id;
	u16 link_speed;
	u16 link_duplex;

	spinlock_t tx_queue_lock; /* prevent concurrent tail updates */

	/* track device up/down/testing state */
	unsigned long state;

	/* Interrupt Throttle Rate */
	u32 itr;
	u32 itr_setting;
	u16 tx_itr;
	u16 rx_itr;

	/*
	 * Tx
	 */
	struct e1000_ring *tx_ring /* One per active queue */;
//						____cacheline_aligned_in_smp; zxj

	struct napi_struct napi; 

	unsigned long tx_queue_len;
	unsigned int restart_queue;
	u32 txd_cmd;

	bool detect_tx_hung;
	u8 tx_timeout_factor;

	u32 tx_int_delay;
	u32 tx_abs_int_delay;

	unsigned int total_tx_bytes;
	unsigned int total_tx_packets;
	unsigned int total_rx_bytes;
	unsigned int total_rx_packets;

	/* Tx stats */
	u64 tpt_old;
	u64 colc_old;
	u32 gotc;
	u64 gotc_old;
	u32 tx_timeout_count;
	u32 tx_fifo_head;
	u32 tx_head_addr;
	u32 tx_fifo_size;
	u32 tx_dma_failed;

	/*
	 * Rx
	 */
	bool (*clean_rx) (struct e1000_adapter *adapter,
			  int *work_done, int work_to_do);
	//					____cacheline_aligned_in_smp; zxj
	void (*alloc_rx_buf) (struct e1000_adapter *adapter,
			      int cleaned_count);
	struct e1000_ring *rx_ring;

	u32 rx_int_delay;
	u32 rx_abs_int_delay;

	/* Rx stats */
	u64 hw_csum_err;
	u64 hw_csum_good;
	u64 rx_hdr_split;
	u32 gorc;
	u64 gorc_old;
	u32 alloc_rx_buff_failed;
	u32 rx_dma_failed;

	unsigned int rx_ps_pages;
	u16 rx_ps_bsize0;
	u32 max_frame_size;
	u32 min_frame_size;

	/* OS defined structs */
	struct net_device *netdev;
	struct pci_dev *pdev;
	struct net_device_stats net_stats;

	/* structs defined in e1000_hw.h */
	struct e1000_hw hw;

	struct e1000_hw_stats stats;
	struct e1000_phy_info phy_info;
	struct e1000_phy_stats phy_stats;

	/* Snapshot of PHY registers */
	struct e1000_phy_regs phy_regs;

	struct e1000_ring test_tx_ring;
	struct e1000_ring test_rx_ring;
	u32 test_icr;

	u32 msg_enable;
	struct msix_entry *msix_entries;
	int int_mode;
	u32 eiac_mask;

	u32 eeprom_wol;
	u32 wol;
	u32 pba;

	bool fc_autoneg;

	unsigned long led_status;

	unsigned int flags;
	struct work_struct downshift_task;
	struct work_struct update_phy_task;
};

struct e1000_info {
	enum e1000_mac_type	mac;
	unsigned int		flags;
	u32			pba;
	s32			(*get_variants)(struct e1000_adapter *);
	struct e1000_mac_operations *mac_ops;
	struct e1000_phy_operations *phy_ops;
	struct e1000_nvm_operations *nvm_ops;
};

/* hardware capability, feature, and workaround flags */
#define FLAG_HAS_AMT                      (1 << 0)
#define FLAG_HAS_FLASH                    (1 << 1)
#define FLAG_HAS_HW_VLAN_FILTER           (1 << 2)
#define FLAG_HAS_WOL                      (1 << 3)
#define FLAG_HAS_ERT                      (1 << 4)
#define FLAG_HAS_CTRLEXT_ON_LOAD          (1 << 5)
#define FLAG_HAS_SWSM_ON_LOAD             (1 << 6)
#define FLAG_HAS_JUMBO_FRAMES             (1 << 7)
#define FLAG_READ_ONLY_NVM                (1 << 8)
#define FLAG_IS_ICH                       (1 << 9)
#define FLAG_HAS_MSIX                     (1 << 10)
#define FLAG_HAS_SMART_POWER_DOWN         (1 << 11)
#define FLAG_IS_QUAD_PORT_A               (1 << 12)
#define FLAG_IS_QUAD_PORT                 (1 << 13)
#define FLAG_TIPG_MEDIUM_FOR_80003ESLAN   (1 << 14)
#define FLAG_APME_IN_WUC                  (1 << 15)
#define FLAG_APME_IN_CTRL3                (1 << 16)
#define FLAG_APME_CHECK_PORT_B            (1 << 17)
#define FLAG_DISABLE_FC_PAUSE_TIME        (1 << 18)
#define FLAG_NO_WAKE_UCAST                (1 << 19)
#define FLAG_MNG_PT_ENABLED               (1 << 20)
#define FLAG_RESET_OVERWRITES_LAA         (1 << 21)
#define FLAG_TARC_SPEED_MODE_BIT          (1 << 22)
#define FLAG_TARC_SET_BIT_ZERO            (1 << 23)
#define FLAG_RX_NEEDS_RESTART             (1 << 24)
#define FLAG_LSC_GIG_SPEED_DROP           (1 << 25)
#define FLAG_SMART_POWER_DOWN             (1 << 26)
#define FLAG_MSI_ENABLED                  (1 << 27)
#define FLAG_RX_CSUM_ENABLED              (1 << 28)
#define FLAG_TSO_FORCE                    (1 << 29)
#define FLAG_RX_RESTART_NOW               (1 << 30)
#define FLAG_MSI_TEST_FAILED              (1 << 31)

#define E1000_RX_DESC_PS(R, i)	    \
	(&(((union e1000_rx_desc_packet_split *)((R).desc))[i]))
#define E1000_GET_DESC(R, i, type)	(&(((struct type *)((R).desc))[i]))
#define E1000_RX_DESC(R, i)		E1000_GET_DESC(R, i, e1000_rx_desc)
#define E1000_TX_DESC(R, i)		E1000_GET_DESC(R, i, e1000_tx_desc)
#define E1000_CONTEXT_DESC(R, i)	E1000_GET_DESC(R, i, e1000_context_desc)

enum e1000_state_t {
	__E1000_TESTING,
	__E1000_RESETTING,
	__E1000_DOWN
};

enum latency_range {
	lowest_latency = 0,
	low_latency = 1,
	bulk_latency = 2,
	latency_invalid = 255
};

extern char e1000e_driver_name[];
extern const char e1000e_driver_version[];

extern void e1000e_check_options(struct e1000_adapter *adapter);
extern void e1000e_set_ethtool_ops(struct net_device *netdev);

extern int e1000e_up(struct e1000_adapter *adapter);
extern void e1000e_down(struct e1000_adapter *adapter);
extern void e1000e_reinit_locked(struct e1000_adapter *adapter);
extern void e1000e_reset(struct e1000_adapter *adapter);
extern void e1000e_power_up_phy(struct e1000_adapter *adapter);
static int e1000e_setup_rx_resources(struct e1000_adapter *adapter);
static int e1000e_setup_tx_resources(struct e1000_adapter *adapter);
static  void e1000e_free_rx_resources(struct e1000_adapter *adapter);
static void e1000e_free_tx_resources(struct e1000_adapter *adapter);
extern void e1000e_update_stats(struct e1000_adapter *adapter);
extern void e1000e_set_interrupt_capability(struct e1000_adapter *adapter);
extern void e1000e_reset_interrupt_capability(struct e1000_adapter *adapter);

extern unsigned int copybreak;

extern char *e1000e_get_hw_dev_name(struct e1000_hw *hw);

extern struct e1000_info e1000_82571_info;
extern struct e1000_info e1000_82572_info;
extern struct e1000_info e1000_82573_info;
extern struct e1000_info e1000_82574_info;
extern struct e1000_info e1000_ich8_info;
extern struct e1000_info e1000_ich9_info;
extern struct e1000_info e1000_es2_info;

extern s32 e1000e_read_pba_num(struct e1000_hw *hw, u32 *pba_num);

extern s32  e1000e_commit_phy(struct e1000_hw *hw);

extern bool e1000e_enable_mng_pass_thru(struct e1000_hw *hw);

extern bool e1000e_get_laa_state_82571(struct e1000_hw *hw);
extern void e1000e_set_laa_state_82571(struct e1000_hw *hw, bool state);

extern void e1000e_write_protect_nvm_ich8lan(struct e1000_hw *hw);
extern void e1000e_set_kmrn_lock_loss_workaround_ich8lan(struct e1000_hw *hw,
						 bool state);
extern void e1000e_igp3_phy_powerdown_workaround_ich8lan(struct e1000_hw *hw);
extern void e1000e_gig_downshift_workaround_ich8lan(struct e1000_hw *hw);
extern void e1000e_disable_gig_wol_ich8lan(struct e1000_hw *hw);

extern s32 e1000e_check_for_copper_link(struct e1000_hw *hw);
extern s32 e1000e_check_for_fiber_link(struct e1000_hw *hw);
extern s32 e1000e_check_for_serdes_link(struct e1000_hw *hw);
extern s32 e1000e_cleanup_led_generic(struct e1000_hw *hw);
extern s32 e1000e_led_on_generic(struct e1000_hw *hw);
extern s32 e1000e_led_off_generic(struct e1000_hw *hw);
extern s32 e1000e_get_bus_info_pcie(struct e1000_hw *hw);
extern s32 e1000e_get_speed_and_duplex_copper(struct e1000_hw *hw, u16 *speed, u16 *duplex);
extern s32 e1000e_get_speed_and_duplex_fiber_serdes(struct e1000_hw *hw, u16 *speed, u16 *duplex);
extern s32 e1000e_disable_pcie_master(struct e1000_hw *hw);
extern s32 e1000e_get_auto_rd_done(struct e1000_hw *hw);
extern s32 e1000e_id_led_init(struct e1000_hw *hw);
extern void e1000e_clear_hw_cntrs_base(struct e1000_hw *hw);
extern s32 e1000e_setup_fiber_serdes_link(struct e1000_hw *hw);
extern s32 e1000e_copper_link_setup_m88(struct e1000_hw *hw);
extern s32 e1000e_copper_link_setup_igp(struct e1000_hw *hw);
extern s32 e1000e_setup_link(struct e1000_hw *hw);
extern void e1000e_clear_vfta(struct e1000_hw *hw);
extern void e1000e_init_rx_addrs(struct e1000_hw *hw, u16 rar_count);
extern void e1000e_update_mc_addr_list_generic(struct e1000_hw *hw,
					       u8 *mc_addr_list,
					       u32 mc_addr_count,
					       u32 rar_used_count,
					       u32 rar_count);
extern void e1000e_rar_set(struct e1000_hw *hw, u8 *addr, u32 index);
extern s32 e1000e_set_fc_watermarks(struct e1000_hw *hw);
extern void e1000e_set_pcie_no_snoop(struct e1000_hw *hw, u32 no_snoop);
extern s32 e1000e_get_hw_semaphore(struct e1000_hw *hw);
extern s32 e1000e_valid_led_default(struct e1000_hw *hw, u16 *data);
extern void e1000e_config_collision_dist(struct e1000_hw *hw);
extern s32 e1000e_config_fc_after_link_up(struct e1000_hw *hw);
extern s32 e1000e_force_mac_fc(struct e1000_hw *hw);
extern s32 e1000e_blink_led(struct e1000_hw *hw);
extern void e1000e_write_vfta(struct e1000_hw *hw, u32 offset, u32 value);
extern void e1000e_reset_adaptive(struct e1000_hw *hw);
extern void e1000e_update_adaptive(struct e1000_hw *hw);

extern s32 e1000e_setup_copper_link(struct e1000_hw *hw);
extern s32 e1000e_get_phy_id(struct e1000_hw *hw);
extern void e1000e_put_hw_semaphore(struct e1000_hw *hw);
extern s32 e1000e_check_reset_block_generic(struct e1000_hw *hw);
extern s32 e1000e_phy_force_speed_duplex_igp(struct e1000_hw *hw);
extern s32 e1000e_get_cable_length_igp_2(struct e1000_hw *hw);
extern s32 e1000e_get_phy_info_igp(struct e1000_hw *hw);
extern s32 e1000e_read_phy_reg_igp(struct e1000_hw *hw, u32 offset, u16 *data);
extern s32 e1000e_phy_hw_reset_generic(struct e1000_hw *hw);
extern s32 e1000e_set_d3_lplu_state(struct e1000_hw *hw, bool active);
extern s32 e1000e_write_phy_reg_igp(struct e1000_hw *hw, u32 offset, u16 data);
extern s32 e1000e_phy_sw_reset(struct e1000_hw *hw);
extern s32 e1000e_phy_force_speed_duplex_m88(struct e1000_hw *hw);
extern s32 e1000e_get_cfg_done(struct e1000_hw *hw);
extern s32 e1000e_get_cable_length_m88(struct e1000_hw *hw);
extern s32 e1000e_get_phy_info_m88(struct e1000_hw *hw);
extern s32 e1000e_read_phy_reg_m88(struct e1000_hw *hw, u32 offset, u16 *data);
extern s32 e1000e_write_phy_reg_m88(struct e1000_hw *hw, u32 offset, u16 data);
extern enum e1000_phy_type e1000e_get_phy_type_from_id(u32 phy_id);
extern s32 e1000e_determine_phy_address(struct e1000_hw *hw);
extern s32 e1000e_write_phy_reg_bm(struct e1000_hw *hw, u32 offset, u16 data);
extern s32 e1000e_read_phy_reg_bm(struct e1000_hw *hw, u32 offset, u16 *data);
extern s32 e1000e_read_phy_reg_bm2(struct e1000_hw *hw, u32 offset, u16 *data);
extern s32 e1000e_write_phy_reg_bm2(struct e1000_hw *hw, u32 offset, u16 data);
extern void e1000e_phy_force_speed_duplex_setup(struct e1000_hw *hw, u16 *phy_ctrl);
extern s32 e1000e_write_kmrn_reg(struct e1000_hw *hw, u32 offset, u16 data);
extern s32 e1000e_read_kmrn_reg(struct e1000_hw *hw, u32 offset, u16 *data);
extern s32 e1000e_phy_has_link_generic(struct e1000_hw *hw, u32 iterations,
			       u32 usec_interval, bool *success);
extern s32 e1000e_phy_reset_dsp(struct e1000_hw *hw);
extern s32 e1000e_read_phy_reg_mdic(struct e1000_hw *hw, u32 offset, u16 *data);
extern s32 e1000e_write_phy_reg_mdic(struct e1000_hw *hw, u32 offset, u16 data);
extern s32 e1000e_check_downshift(struct e1000_hw *hw);

static inline s32 e1000_phy_hw_reset(struct e1000_hw *hw)
{
	return hw->phy.ops.reset_phy(hw);
}

static inline s32 e1000_check_reset_block(struct e1000_hw *hw)
{
	return hw->phy.ops.check_reset_block(hw);
}

static inline s32 e1e_rphy(struct e1000_hw *hw, u32 offset, u16 *data)
{
	return hw->phy.ops.read_phy_reg(hw, offset, data);
}

static inline s32 e1e_wphy(struct e1000_hw *hw, u32 offset, u16 data)
{
	return hw->phy.ops.write_phy_reg(hw, offset, data);
}

static inline s32 e1000_get_cable_length(struct e1000_hw *hw)
{
	return hw->phy.ops.get_cable_length(hw);
}

extern s32 e1000e_acquire_nvm(struct e1000_hw *hw);
extern s32 e1000e_write_nvm_spi(struct e1000_hw *hw, u16 offset, u16 words, u16 *data);
extern s32 e1000e_update_nvm_checksum_generic(struct e1000_hw *hw);
extern s32 e1000e_poll_eerd_eewr_done(struct e1000_hw *hw, int ee_reg);
extern s32 e1000e_read_nvm_eerd(struct e1000_hw *hw, u16 offset, u16 words, u16 *data);
extern s32 e1000e_validate_nvm_checksum_generic(struct e1000_hw *hw);
extern void e1000e_release_nvm(struct e1000_hw *hw);
extern void e1000e_reload_nvm(struct e1000_hw *hw);
extern s32 e1000e_read_mac_addr(struct e1000_hw *hw);

static inline s32 e1000_validate_nvm_checksum(struct e1000_hw *hw)
{
	return hw->nvm.ops.validate_nvm(hw);
}

static inline s32 e1000e_update_nvm_checksum(struct e1000_hw *hw)
{
	return hw->nvm.ops.update_nvm(hw);
}

static inline s32 e1000_read_nvm(struct e1000_hw *hw, u16 offset, u16 words, u16 *data)
{
	return hw->nvm.ops.read_nvm(hw, offset, words, data);
}

static inline s32 e1000_write_nvm(struct e1000_hw *hw, u16 offset, u16 words, u16 *data)
{
	return hw->nvm.ops.write_nvm(hw, offset, words, data);
}

static inline s32 e1000_get_phy_info(struct e1000_hw *hw)
{
	return hw->phy.ops.get_phy_info(hw);
}

static inline s32 e1000e_check_mng_mode(struct e1000_hw *hw)
{
	return hw->mac.ops.check_mng_mode(hw);
}

extern bool e1000e_check_mng_mode_generic(struct e1000_hw *hw);
extern bool e1000e_enable_tx_pkt_filtering(struct e1000_hw *hw);
extern s32 e1000e_mng_write_dhcp_info(struct e1000_hw *hw, u8 *buffer, u16 length);

static inline u32 __er32(struct e1000_hw *hw, unsigned long reg)
{
	return readl(hw->hw_addr + reg);
}

static inline void __ew32(struct e1000_hw *hw, unsigned long reg, u32 val)
{
	writel(val, hw->hw_addr + reg);
}

#endif /* _E1000_H_ */
