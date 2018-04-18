#ifndef __LINUX_NET_HEAD_H__
#define __LINUX_NET_HEAD_H__
#include <sys/param.h>
#include <sys/systm.h>
#include <sys/mbuf.h>
#include <sys/malloc.h>
#include <sys/kernel.h>
#include <sys/socket.h>
#include <sys/syslog.h>

#include <sys/systm.h>
#include <net/if.h>
#include <net/if_dl.h>
#include <net/if_media.h>
#include <net/if_types.h>

#ifdef INET
#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/in_var.h>
#include <netinet/ip.h>
#endif

#ifdef IPX
#include <netipx/ipx.h>
#include <netipx/ipx_if.h>
#endif

#ifdef NS
#include <netns/ns.h>
#include <netns/ns_if.h>
#endif

#if NBPFILTER > 0
#include <net/bpf.h>
#include <net/bpfdesc.h>
#endif

#if defined(__NetBSD__) || defined(__OpenBSD__)

#include <sys/ioctl.h>
#include <sys/errno.h>
#include <sys/device.h>

#if defined(__NetBSD__)
#include <net/if_ether.h>
#include <netinet/if_inarp.h>
#endif

#if defined(__OpenBSD__)
#include <netinet/if_ether.h>
#endif

#include <vm/vm.h>

#include <machine/cpu.h>
#include <machine/bus.h>
#include <machine/intr.h>

#include <dev/mii/miivar.h>

#include <dev/pci/pcivar.h>
#include <dev/pci/pcireg.h>
#include <dev/pci/pcidevs.h>

#include <dev/pci/if_fxpreg.h>
#include <dev/pci/if_fxpvar.h>
typedef struct FILE
{
    int fd;
    int valid;
    int ungetcflag;
    int ungetchar;
} FILE;
extern FILE _iob[];
#define serialout (&_iob[1])


#else /* __FreeBSD__ */

#include <sys/sockio.h>

#include <netinet/if_ether.h>

#include <vm/vm.h>      /* for vtophys */
#include <vm/vm_param.h>    /* for vtophys */
#include <vm/pmap.h>        /* for vtophys */
#include <machine/clock.h>  /* for DELAY */

#include <pci/pcivar.h>

#endif /* __NetBSD__ || __OpenBSD__ */

#define offsetof(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)

/*
 * NOTE!  On the Alpha, we have an alignment constraint.  The
 * card DMAs the packet immediately following the RFA.  However,
 * the first thing in the packet is a 14-byte Ethernet header.
 * This means that the packet is misaligned.  To compensate,
 * we actually offset the RFA 2 bytes into the cluster.  This
 * aligns the packet after the Ethernet header at a 32-bit
 * boundary.  HOWEVER!  This means that the RFA is misaligned!
 */

#ifdef BADPCIBRIDGE
#define BADPCIBRIDGE
#define RFA_ALIGNMENT_FUDGE 4
#else
#define RFA_ALIGNMENT_FUDGE 2
#endif

#include <linux/types.h>
//#include <linux/pci.h>
typedef unsigned int dma_addr_t;
#define PCI_SUBSYSTEM_VENDOR_ID 0x2c
#define PCI_ANY_ID (~0)
#define KERN_DEBUG
#define kmalloc(size,...)  malloc(size,M_DEVBUF, M_DONTWAIT)
#define kfree(addr,...) free(addr,M_DEVBUF);
#define netdev_priv(dev) dev->priv
#define iowrite8(b,addr) ((*(volatile unsigned char *)(addr)) = (b))
#define iowrite16(w,addr) ((*(volatile unsigned short *)(addr)) = (w))
#define iowrite32(l,addr) ((*(volatile unsigned int *)(addr)) = (l))

#define ioread8(addr)     (*(volatile unsigned char *)(addr))
#define ioread16(addr)     (*(volatile unsigned short *)(addr))
#define ioread32(addr)     (*(volatile unsigned int *)(addr))
#define KERN_WARNING
#define printk printf
#define le16_to_cpu(x) (x)
#define KERN_INFO
#define PCI_COMMAND     0x04    /* 16 bits */
#define  PCI_COMMAND_MASTER 0x4 /* Enable bus mastering */
#define PCI_REVISION_ID 8
#define spin_lock_init(...)
#define spin_lock(...)
#define spin_unlock(...)
extern int ticks;
#define jiffies ticks
#define round_jiffies(x) x

#define KERN_ERR
#define KERN_NOTICE
#define ETH_ALEN    6       /* Octets in one ethernet addr   */
#define udelay delay
#define msleep mdelay
static inline void mdelay(int microseconds)
{
    int i;
    for (i = 0; i < microseconds; i++)delay(1000);
}
#define le32_to_cpu(x) (x)
#define cpu_to_le32(x) (x)
#define spin_lock_irqsave(...)
#define spin_unlock_irqrestore(...)
#define netif_start_queue(...)
#define netif_stop_queue(...)
#define netif_wake_queue(...)
#define netif_queue_stopped(...) 0
#define __init
#define         NET_IP_ALIGN 2
#define NET_SKB_PAD 16

typedef  int irqreturn_t;



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

/* Basic mode control register. */
#define BMCR_RESV               0x003f  /* Unused...                   */
#define BMCR_SPEED1000      0x0040  /* MSB of Speed (1000)         */
#define BMCR_CTST               0x0080  /* Collision test              */
#define BMCR_FULLDPLX           0x0100  /* Full duplex                 */
#define BMCR_ANRESTART          0x0200  /* Auto negotiation restart    */
#define BMCR_ISOLATE            0x0400  /* Disconnect DP83840 from MII */
#define BMCR_PDOWN              0x0800  /* Powerdown the DP83840       */
#define BMCR_ANENABLE           0x1000  /* Enable auto negotiation     */
#define BMCR_SPEED100           0x2000  /* Select 100Mbps              */
#define BMCR_LOOPBACK           0x4000  /* TXD loopback bits           */
#define BMCR_RESET              0x8000  /* Reset the DP83840           */

/* Basic mode status register. */
#define BMSR_ERCAP              0x0001  /* Ext-reg capability          */
#define BMSR_JCD                0x0002  /* Jabber detected             */
#define BMSR_LSTATUS            0x0004  /* Link status                 */
#define BMSR_ANEGCAPABLE        0x0008  /* Able to do auto-negotiation */
#define BMSR_RFAULT             0x0010  /* Remote fault detected       */
#define BMSR_ANEGCOMPLETE       0x0020  /* Auto-negotiation complete   */
#define BMSR_RESV               0x00c0  /* Unused...                   */
#define BMSR_ESTATEN        0x0100  /* Extended Status in R15 */
#define BMSR_100FULL2       0x0200  /* Can do 100BASE-T2 HDX */
#define BMSR_100HALF2       0x0400  /* Can do 100BASE-T2 FDX */
#define BMSR_10HALF             0x0800  /* Can do 10mbps, half-duplex  */
#define BMSR_10FULL             0x1000  /* Can do 10mbps, full-duplex  */
#define BMSR_100HALF            0x2000  /* Can do 100mbps, half-duplex */
#define BMSR_100FULL            0x4000  /* Can do 100mbps, full-duplex */
#define BMSR_100BASE4           0x8000  /* Can do 100mbps, 4k packets  */
/* Advertisement control register. */
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
typedef unsigned long spinlock_t;
struct net_device_stats
{
    unsigned long   rx_packets;     /* total packets received   */
    unsigned long   tx_packets;     /* total packets transmitted    */
    unsigned long   rx_bytes;       /* total bytes received     */
    unsigned long   tx_bytes;       /* total bytes transmitted  */
    unsigned long   rx_errors;      /* bad packets received     */
    unsigned long   tx_errors;      /* packet transmit problems */
    unsigned long   rx_dropped;     /* no space in linux buffers    */
    unsigned long   tx_dropped;     /* no space available in linux  */
    unsigned long   multicast;      /* multicast packets received   */
    unsigned long   collisions;

    /* detailed rx_errors: */
    unsigned long   rx_length_errors;
    unsigned long   rx_over_errors;     /* receiver ring buff overflow  */
    unsigned long   rx_crc_errors;      /* recved pkt with crc error    */
    unsigned long   rx_frame_errors;    /* recv'd frame alignment error */
    unsigned long   rx_fifo_errors;     /* recv'r fifo overrun      */
    unsigned long   rx_missed_errors;   /* receiver missed packet   */

    /* detailed tx_errors */
    unsigned long   tx_aborted_errors;
    unsigned long   tx_carrier_errors;
    unsigned long   tx_fifo_errors;
    unsigned long   tx_heartbeat_errors;
    unsigned long   tx_window_errors;

    /* for cslip etc */
    unsigned long   rx_compressed;
    unsigned long   tx_compressed;
};

struct pci_device_id
{
    unsigned int vendor, device;        /* Vendor and device ID or PCI_ANY_ID */
    unsigned int subvendor, subdevice;  /* Subsystem ID's or PCI_ANY_ID */
    unsigned int class, class_mask;     /* (class,subclass,prog-if) triplet */
    unsigned long driver_data;      /* Data private to the driver */
};
struct pci_dev
{
    struct pci_attach_args pa;
    struct net_device *dev;
    int irq;
    unsigned int    devfn;      /* encoded device & function index */
    unsigned short  vendor;
    unsigned short  device;
    unsigned short  subsystem_vendor;
    unsigned short  subsystem_device;
    unsigned int    class;      /* 3 bytes: (base,sub,prog-if) */
};
struct net_device
{
//most of the fields come from struct fxp_softc
#if defined(__NetBSD__) || defined(__OpenBSD__)
    struct device sc_dev;       /* generic device structures */
    void *sc_ih;            /* interrupt handler cookie */
    bus_space_tag_t sc_st;      /* bus space tag */
    bus_space_handle_t sc_sh;   /* bus space handle */
    pci_chipset_tag_t sc_pc;    /* chipset handle needed by mips */
#else
    struct caddr_t csr;     /* control/status registers */
#endif /* __NetBSD__ || __OpenBSD__ */
#if defined(__OpenBSD__) || defined(__FreeBSD__)
    struct arpcom arpcom;       /* per-interface network data !!we use this*/
#endif
#if defined(__NetBSD__)
    struct ethercom sc_ethercom;    /* ethernet common part */
#endif
    struct mii_data sc_mii;     /* MII media information */
    char        node_addr[6]; //the net interface's address

    unsigned char *tx_ad[4];   // Transmit buffer
    unsigned char *tx_dma;     // used by dma
    unsigned char *tx_buffer;  // Transmit buffer
    unsigned char *tx_buf[4];  // used by driver coping data

    unsigned char *rx_dma;     // Receive buffer, used by dma
    unsigned char *rx_buffer;     // used by driver
    //We use mbuf here
    int     packetlen;
    struct pci_dev pcidev;
    void        *priv;  /* driver can hang private data here */
    unsigned long       mem_end;    /* shared mem end   */
    unsigned long       mem_start;  /* shared mem start */
    unsigned long       base_addr;  /* device I/O address   */
    unsigned int        irq;        /* device IRQ number    */
    int         features;

    int flags;
    int mtu;
    struct dev_mc_list  *mc_list;   /* Multicast mac addresses  */
    char            name[IFNAMSIZ];
    unsigned char dev_addr[6];
    unsigned char       addr_len;   /* hardware address length  */
    unsigned long       state;
    unsigned long trans_start, last_rx;
    unsigned int opencount;
    unsigned char       perm_addr[6]; /* permanent hw address */
    struct net_device_stats stats;

    int (*open)(struct net_device *ndev);
    int (*stop)(struct net_device *ndev);
    int (*hard_start_xmit)(struct sk_buff *skb, struct net_device *ndev);
};


#define dma_free_coherent dma_free_consistent

static void dma_free_consistent(void *pdev, size_t size, void *cpu_addr,
                                dma_addr_t dma_addr)
{
    kfree(cpu_addr);
}

static void *dma_alloc_coherent(void *hwdev, size_t size,
                                dma_addr_t *dma_handle, int flag)
{
    void *buf;
    buf = malloc(size, M_DEVBUF, M_DONTWAIT);
#if defined(LS3_HT)||defined(LOONGSON_2K)
#else
    CPU_IOFlushDCache(buf, size, SYNC_R);

    buf = (unsigned char *)CACHED_TO_UNCACHED(buf);
#endif
    *dma_handle = vtophys(buf);

    return (void *)buf;
}

static unsigned short  read_eeprom(unsigned long ioaddr, unsigned int eep_addr);
static  void  write_eeprom(unsigned long ioaddr, unsigned int eep_addr, unsigned short writedata);

struct sk_buff
{
    unsigned int    len;            /* Length of actual data            */
    unsigned int    data_len;            /* Length of  data            */
    unsigned char   *data;          /* Data head pointer                */
    unsigned char   *head;          /* Data head pointer                */
    unsigned char protocol;
    struct net_device *dev;
};

#define MAX_SKB_FRAGS 6
typedef struct skb_frag_struct skb_frag_t;
 
 struct skb_frag_struct {
     void *page;
     u32 page_offset;
     u32 size;
 };

struct skb_shared_info {
     unsigned short  nr_frags;
     skb_frag_t  frags[MAX_SKB_FRAGS];
 };

#define skb_shinfo(SKB) ((struct skb_shared_info *)(0))


#define dev_kfree_skb dev_kfree_skb_any
#define dev_kfree_skb_irq dev_kfree_skb_any

static inline unsigned int skb_headlen(const struct sk_buff *skb)
{
    return skb->len - skb->data_len;
}

static inline void dev_kfree_skb_any(struct sk_buff *skb)
{
    kfree(skb->head);
    kfree(skb);
}

static void skb_put(struct sk_buff *skb, unsigned int len)
{
    skb->len += len;
    //skb->tail += len;
}


#define alloc_skb(len,flags) dev_alloc_skb(len)

static struct sk_buff *dev_alloc_skb(unsigned int length)
{
    struct sk_buff *skb = kmalloc(sizeof(struct sk_buff), GFP_KERNEL);
    skb->len = 0;
    skb->data_len = 0;
    skb->data = skb->head = (void *) kmalloc(length, GFP_KERNEL);
#ifdef PACKET_CHECK
	/*fix me*/
    memset(skb->data, 0, length);
#endif
    return skb;
}

static struct sk_buff *netdev_alloc_skb(struct net_device *dev,
                                        unsigned int length)
{
    struct sk_buff *skb;
    skb = dev_alloc_skb(length);
    skb->dev = dev;
    return skb;
}


static inline void skb_reserve(struct sk_buff *skb, unsigned int len)
{
    skb->data += len;
}

static inline struct sk_buff *netdev_alloc_skb_ip_align(struct net_device *dev,
        unsigned int length)
{
    struct sk_buff *skb;

    skb = alloc_skb(length + NET_SKB_PAD + NET_IP_ALIGN, GFP_ATOMIC);
    if (skb)
    {
#if (NET_IP_ALIGN + NET_SKB_PAD)
        skb_reserve(skb, NET_IP_ALIGN + NET_SKB_PAD);
#endif
        skb->dev = dev;
    }
    return skb;
}

#define  dma_unmap_page(dev,hwaddr,size,dir)  dma_unmap_single(dev,hwaddr,size,dir)

static inline dma_addr_t dma_map_single(struct pci_dev *hwdev, void *ptr,
                                        size_t size, int direction)
{
    unsigned long addr = (unsigned long) ptr;


#if defined(LS3_HT)||defined(LOONGSON_2K)
#else

    printf("pci_map_single1\n");
    pci_sync_cache(hwdev, addr, size, SYNC_W);
#endif
#if 0
    dma_addr_t address = _pci_dmamap(addr, size);
    printf("pci_map_single: input addr is 0x%x, output addr is 0x%x\n", addr, address);
#endif
    return _pci_dmamap(addr, size);
}

static inline void dma_unmap_single(struct pci_dev *hwdev, dma_addr_t dma_addr,
                                    size_t size, int direction)
{
#if defined(LS3_HT) || defined(LOONGSON_2K)
#else
    pci_sync_cache(hwdev, _pci_cpumap(dma_addr, size), size, SYNC_R);
#endif
}

#define netif_receive_skb(x) netif_rx(x)


#define PCI_DMA_BIDIRECTIONAL   0
#define PCI_DMA_TODEVICE    1
#define PCI_DMA_FROMDEVICE  2
#define PCI_DMA_NONE        3

static struct mbuf *getmbuf()
{
    struct mbuf *m;


    MGETHDR(m, M_DONTWAIT, MT_DATA);
    if (m == NULL)
    {
        printf("getmbuf for reception failed\n");
        return  NULL;
    }
    else
    {
        MCLGET(m, M_DONTWAIT);
        if ((m->m_flags & M_EXT) == 0)
        {
            m_freem(m);
            return NULL;
        }
        if (m->m_data != m->m_ext.ext_buf)
        {
            printf("m_data not equal to ext_buf!!!\n");
        }
    }

    m->m_data += RFA_ALIGNMENT_FUDGE;
    return m;
}

static inline int netif_rx(struct sk_buff *skb)
{
    struct mbuf *m;
    struct ether_header *eh;
    struct net_device *netdev = skb->dev;
    struct ifnet *ifp = &netdev->arpcom.ac_if;
    m = getmbuf();
    if (m == NULL)
    {
        printf("getmbuf failed in  netif_rx\n");
        return 0; // no successful
    }
    skb->len = skb->len + 4; //bug
#if 0
    {
        char str[80];
        sprintf(str, "pcs -1;d1 0x%x %d", skb->data, skb->len);
        printf("m=%x,m->m_data=%x\n%s\n", m, m->m_data, str);
        do_cmd(str);
    }
#endif
    bcopy(skb->data, mtod(m, caddr_t), skb->len);

    //hand up  the received package to upper protocol for further dealt
    m->m_pkthdr.rcvif = ifp;
    m->m_pkthdr.len = m->m_len = skb->len - sizeof(struct ether_header);

    eh = mtod(m, struct ether_header *);

    m->m_data += sizeof(struct ether_header);
    //printf("%s, etype %x:\n", __FUNCTION__, eh->ether_type);
    ether_input(ifp, eh, m);
    dev_kfree_skb_any(skb);
    return 0;
}
//--------------------------------------------------------------------
static int irqstate = 0;

static irqreturn_t (*interrupt)(int irq, void *dev_id);
#define  request_irq(irq,b,c,d,e) (irqstate|=(1<<irq),interrupt=b,0)
#define free_irq(irq,b) (irqstate &=~(1<<irq),0)
#define HZ 100
#define MAX_ADDR_LEN    6       /* Largest hardware address length */
#define IFNAMSIZ 16

#include <linux/list.h>
struct timer_list
{
    struct list_head list;
    unsigned long expires;
    unsigned long data;
    void (*function)(unsigned long);
};
//

struct mii_if_info
{
    int phy_id;
    int advertising;
    int phy_id_mask;
    int reg_num_mask;

    unsigned int full_duplex : 1;   /* is full duplex? */
    unsigned int force_media : 1;   /* is autoneg. disabled? */
    unsigned int supports_gmii : 1; /* are GMII registers supported? */

    struct net_device *dev;
    int (*mdio_read)(struct net_device *dev, int phy_id, int location);
    void (*mdio_write)(struct net_device *dev, int phy_id, int location, int val);
};
enum
{
    SUCCESS,
    ERR_FAILURE
};

#define IRQ_NONE 0
#define IRQ_HANDLED 1
#define ETH_ALEN    6       /* Octets in one ethernet addr   */
#define ETH_HLEN    14      /* Total octets in header.   */
#define ETH_ZLEN    60      /* Min. octets in frame sans FCS */
#define ETH_DATA_LEN    1500        /* Max. octets in payload    */
#define ETH_FRAME_LEN   1514        /* Max. octets in frame sans FCS */
#define VLAN_ETH_ALEN   6       /* Octets in one ethernet addr   */
#define VLAN_ETH_HLEN   18      /* Total octets in header.   */
#define VLAN_ETH_ZLEN   64      /* Min. octets in frame sans FCS */
#define DMA_FROM_DEVICE  0
#define DMA_TO_DEVICE 1
#define false 0
#define true 1
#define __devinit
#define NETLAKERS_ADDR 0xBFE10000
#define GFP_KERNEL 0


/* Link partner ability register. */
#define LPA_SLCT                0x001f  /* Same as advertise selector  */
#define LPA_10HALF              0x0020  /* Can do 10mbps half-duplex   */
#define LPA_1000XFULL           0x0020  /* Can do 1000BASE-X full-duplex */
#define LPA_10FULL              0x0040  /* Can do 10mbps full-duplex   */
#define LPA_1000XHALF           0x0040  /* Can do 1000BASE-X half-duplex */
#define LPA_100HALF             0x0080  /* Can do 100mbps half-duplex  */
#define LPA_1000XPAUSE          0x0080  /* Can do 1000BASE-X pause     */
#define LPA_100FULL             0x0100  /* Can do 100mbps full-duplex  */
#define LPA_1000XPAUSE_ASYM     0x0100  /* Can do 1000BASE-X pause asym*/
#define LPA_100BASE4            0x0200  /* Can do 100mbps 4k packets   */
#define LPA_PAUSE_CAP           0x0400  /* Can pause                   */
#define LPA_PAUSE_ASYM          0x0800  /* Can pause asymetrically     */
#define LPA_RESV                0x1000  /* Unused...                   */
#define LPA_RFAULT              0x2000  /* Link partner faulted        */
#define LPA_LPACK               0x4000  /* Link partner acked us       */
#define LPA_NPAGE               0x8000  /* Next page bit               */

#define LPA_DUPLEX      (LPA_10FULL | LPA_100FULL)
#define LPA_100         (LPA_100FULL | LPA_100HALF | LPA_100BASE4)


/* Expansion register for auto-negotiation. */
#define EXPANSION_NWAY          0x0001  /* Can do N-way auto-nego      */
#define EXPANSION_LCWP          0x0002  /* Got new RX page code word   */
#define EXPANSION_ENABLENPAGE   0x0004  /* This enables npage words    */
#define EXPANSION_NPCAPABLE     0x0008  /* Link partner supports npage */
#define EXPANSION_MFAULTS       0x0010  /* Multiple faults detected    */
#define EXPANSION_RESV          0xffe0  /* Unused...                   */

#define ESTATUS_1000_TFULL  0x2000  /* Can do 1000BT Full */
#define ESTATUS_1000_THALF  0x1000  /* Can do 1000BT Half */

/* N-way test register. */
#define NWAYTEST_RESV1          0x00ff  /* Unused...                   */
#define NWAYTEST_LOOPBACK       0x0100  /* Enable loopback for N-way   */
#define NWAYTEST_RESV2          0xfe00  /* Unused...                   */

/* 1000BASE-T Control register */
#define ADVERTISE_1000FULL      0x0200  /* Advertise 1000BASE-T full duplex */
#define ADVERTISE_1000HALF      0x0100  /* Advertise 1000BASE-T half duplex */

/* 1000BASE-T Status register */
#define LPA_1000LOCALRXOK       0x2000  /* Link partner local receiver status */
#define LPA_1000REMRXOK         0x1000  /* Link partner remote receiver status */
#define LPA_1000FULL            0x0800  /* Link partner 1000BASE-T full duplex */

#define LPA_1000HALF            0x0400  /* Link partner 1000BASE-T half duplex */
#define netif_msg_link(...) (1)

static inline int netif_carrier_ok(struct net_device *dev)
{
    return dev->state & 1;
}

static inline void netif_carrier_on(struct net_device *dev)
{
    dev->state |= 1;
}

static inline void netif_carrier_off(struct net_device *dev)
{
    dev->state &= ~1;
}

int mii_link_ok(struct mii_if_info *mii)
{
//  int read_num= 0x100000;
    /* first, a dummy read, needed to latch some MII phys */
    mii->mdio_read(mii->dev, mii->phy_id, MII_BMSR);
    /*  while(read_num --)
        {
            if (mii->mdio_read(mii->dev, mii->phy_id, MII_BMSR) & BMSR_LSTATUS)
            {
                printf("read_num = %d \n",read_num);
                return 1;
            }
            delay(5);
        }*/
    if (mii->mdio_read(mii->dev, mii->phy_id, MII_BMSR) & BMSR_LSTATUS)
        return 1;
    return 0;
}

static inline unsigned int mii_nway_result(unsigned int negotiated)
{
    unsigned int ret;

    if (negotiated & LPA_100FULL)
        ret = LPA_100FULL;
    else if (negotiated & LPA_100BASE4)
        ret = LPA_100BASE4;
    else if (negotiated & LPA_100HALF)
        ret = LPA_100HALF;
    else if (negotiated & LPA_10FULL)
        ret = LPA_10FULL;
    else
        ret = LPA_10HALF;

    return ret;
}

static unsigned int mii_check_media(struct mii_if_info *mii,
                                    unsigned int ok_to_print,
                                    unsigned int init_media)
{
    unsigned int old_carrier, new_carrier;
    int advertise, lpa, media, duplex;
    int lpa2 = 0;

    /* if forced media, go no further */
    if (mii->force_media)
        return 0; /* duplex did not change */

    /* check current and old link status */
    old_carrier = netif_carrier_ok(mii->dev) ? 1 : 0;
    new_carrier = (unsigned int) mii_link_ok(mii);

    /* if carrier state did not change, this is a "bounce",
     * just exit as everything is already set correctly
     */
    if ((!init_media) && (old_carrier == new_carrier))
        return 0; /* duplex did not change */

    /* no carrier, nothing much to do */
    if (!new_carrier)
    {
        netif_carrier_off(mii->dev);
        if (ok_to_print)
            printk(KERN_INFO "%s: link down\n", mii->dev->name);
        return 0; /* duplex did not change */
    }

    /*
     * we have carrier, see who's on the other end
     */
    netif_carrier_on(mii->dev);

    /* get MII advertise and LPA values */
    if ((!init_media) && (mii->advertising))
        advertise = mii->advertising;
    else
    {
        advertise = mii->mdio_read(mii->dev, mii->phy_id, MII_ADVERTISE);
        mii->advertising = advertise;
    }
    lpa = mii->mdio_read(mii->dev, mii->phy_id, MII_LPA);
    if (mii->supports_gmii)
        lpa2 = mii->mdio_read(mii->dev, mii->phy_id, MII_STAT1000);

    /* figure out media and duplex from advertise and LPA values */

    media = mii_nway_result(lpa & advertise);
    duplex = (media & ADVERTISE_FULL) ? 1 : 0;
    if (lpa2 & LPA_1000FULL)
        duplex = 1;

    if (ok_to_print)
        printk(KERN_INFO "%s: link up, %sMbps, %s-duplex, lpa 0x%04X\n",
               mii->dev->name,
               lpa2 & (LPA_1000FULL | LPA_1000HALF) ? "1000" :
               media & (ADVERTISE_100FULL | ADVERTISE_100HALF) ? "100" : "10",
               duplex ? "full" : "half",
               lpa);
    if ((init_media) || (mii->full_duplex != duplex))
    {
        mii->full_duplex = duplex;
        return 1; /* duplex changed */
    }

    return 0; /* duplex did not change */
}

#define test_and_clear_bit clear_bit
#define test_and_set_bit set_bit
static __inline__ int set_bit(int nr, long *addr)
{
    int mask, retval;

    addr += nr >> 5;
    mask = 1 << (nr & 0x1f);
//    cli();
    retval = (mask & *addr) != 0;
    *addr |= mask;
//    sti();
    return retval;
}

static __inline__ int clear_bit(int nr, long *addr)
{
    int mask, retval;

    addr += nr >> 5;
    mask = 1 << (nr & 0x1f);
//    cli();
    retval = (mask & *addr) != 0;
    *addr &= ~mask;
//    sti();
    return retval;
}

static __inline__ int test_bit(int nr, long *addr)
{
    int mask;

    addr += nr >> 5;
    mask = 1 << (nr & 0x1f);
    return ((mask & *addr) != 0);
}


struct napi_struct
{
    /* The poll_list must only be managed by the entity which
       * changes the state of the NAPI_STATE_SCHED bit.  This means
       * whoever atomically sets that bit can add this napi_struct
       * to the per-cpu poll_list, and whoever clears that bit
       * can remove from the list right before clearing the bit.
    */
    struct list_head    poll_list;

    unsigned long       state;
    int         weight;
    int (*poll)(struct napi_struct *, int);
};

#define netif_tx_stop_all_queues(...)
#define napi_disable(...)

static inline void init_timer(struct timer_list *timer)
{
    timer->list.next = timer->list.prev = NULL;
}

static inline int mod_timer(struct timer_list *timer, unsigned long expires)
{
    untimeout((void (*) __P((void *)))timer->function, (void *)timer->data);
    timeout((void (*) __P((void *)))timer->function, (void *)timer->data, expires - ticks);
    return 0;
}

static inline int del_timer_sync(struct timer_list *timer)
{
    untimeout((void (*) __P((void *)))timer->function, (void *)timer->data);
    return 0;
}



struct tq_struct
{
    struct list_head list;      /* linked list of active bh's */
    unsigned long sync;     /* must be initialized to zero */
    void (*routine)(void *);    /* function to call */
    void *data;         /* argument to function */
};

#define INIT_WORK(a,b) INIT_TQUEUE(a,(void (*)(void *))b,a)
#define schedule_work schedule_task
#define work_struct tq_struct
#define INIT_TQUEUE(_tq, _routine, _data)       \
    do {                        \
        INIT_LIST_HEAD(&(_tq)->list);       \
        (_tq)->sync = 0;            \
        (_tq)->routine = _routine;      \
        (_tq)->data = _data;            \
    } while (0)

#define TQ_ACTIVE(q)        (!list_empty(&q))
typedef struct list_head task_queue;
static inline int queue_task(struct tq_struct *bh_pointer, task_queue *bh_list)
{
    int ret = 0;
    if (!test_and_set_bit(0, &bh_pointer->sync))
    {
        list_add_tail(&bh_pointer->list, bh_list);
        ret = 1;
    }
    return ret;
}

static inline void __run_task_queue(task_queue *list)
{
    struct list_head head, *next;

    list_add(&head, list);
    list_del_init(list);

    next = head.next;
    while (next != &head)
    {
        void (*f)(void *);
        struct tq_struct *p;
        void *data;

        p = list_entry(next, struct tq_struct, list);
        next = next->next;
        f = p->routine;
        data = p->data;
        p->sync = 0;
        if (f)
        {
            f(data);
        }
    }
}

#define TQ_ACTIVE(q)        (!list_empty(&q))
static inline void run_task_queue(task_queue *list)
{
    if (TQ_ACTIVE(*list))
        __run_task_queue(list);
}


#define DECLARE_TASK_QUEUE(q)   LIST_HEAD1(q)
static DECLARE_TASK_QUEUE(tq_igb);

static int schedule_task(struct tq_struct *task)
{
    queue_task(task, &tq_igb);
    return 0;
}

#define time_after(t1,t2)            (((long)t1-t2) > 0)


static int netif_running(struct net_device *netdev)
{
    struct ifnet *ifp = &netdev->arpcom.ac_if;
    return (ifp->if_flags & IFF_RUNNING) ? 1 : 0;
}

enum
{
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

#define NETIF_F_SG      1   /* Scatter/gather IO. */
#define NETIF_F_IP_CSUM     2   /* Can checksum only TCP/UDP over IPv4. */
#define NETIF_F_NO_CSUM     4   /* Does not require checksum. F.e. loopack. */
#define NETIF_F_HW_CSUM     8   /* Can checksum all the packets. */
#define NETIF_F_DYNALLOC    16  /* Self-dectructable device. */
#define NETIF_F_HIGHDMA     32  /* Can DMA to high memory. */
#define NETIF_F_FRAGLIST    64  /* Scatter/gather IO. */
#define NETIF_F_HW_VLAN_TX  128 /* Transmit VLAN hw acceleration */
#define NETIF_F_HW_VLAN_RX  256 /* Receive VLAN hw acceleration */
#define NETIF_F_HW_VLAN_FILTER  512 /* Receive filtering on VLAN */
#define NETIF_F_VLAN_CHALLENGED 1024    /* Device cannot handle VLAN packets */
#define NETIF_F_LLTX        4096    /* LockLess TX - deprecated. Please */

#ifndef NETIF_F_RXCSUM
#define NETIF_F_RXCSUM      (1 << 29)
#endif
#ifndef NETIF_F_SCTP_CSUM
#define NETIF_F_SCTP_CSUM 0
#endif
#define PCI_EXP_LNKCAP2     44  /* Link Capability 2 */

#ifndef MDIO_EEE_100TX
#define MDIO_EEE_100TX      0x0002  /* 100TX EEE cap */
#endif
#ifndef MDIO_EEE_1000T
#define MDIO_EEE_1000T      0x0004  /* 1000T EEE cap */
#endif
#ifndef MDIO_EEE_10GT
#define MDIO_EEE_10GT       0x0008  /* 10GT EEE cap */
#endif
#ifndef MDIO_EEE_1000KX
#define MDIO_EEE_1000KX     0x0010  /* 1000KX EEE cap */
#endif
#ifndef MDIO_EEE_10GKX4
#define MDIO_EEE_10GKX4     0x0020  /* 10G KX4 EEE cap */
#endif
#ifndef MDIO_EEE_10GKR
#define MDIO_EEE_10GKR      0x0040  /* 10G KR EEE cap */
#endif
#ifndef VLAN_HLEN
#define VLAN_HLEN 4
#endif

#ifndef VLAN_ETH_HLEN
#define VLAN_ETH_HLEN 18
#endif

#ifndef VLAN_ETH_FRAME_LEN
#define VLAN_ETH_FRAME_LEN 1518
#endif

struct pci_device;

#define PCI_MAP_REG_START 0x10
static int pci_resource_start(struct pci_dev *pdev, int bar)
{
    int reg = PCI_MAP_REG_START + bar * 4;
    int rv, start, size, flags, type;
    type = pci_mapreg_type(0, pdev->pa.pa_tag, reg);
    rv = pci_mapreg_info(0, pdev->pa.pa_tag, reg, type, &start, &size, &flags);
    if (PCI_MAPREG_TYPE(type) == PCI_MAPREG_TYPE_IO)
    {
        start = pdev->pa.pa_iot->bus_base | start;
    }
    else
    {
        start = pdev->pa.pa_memt->bus_base | start;
    }
    return rv ? 0 : start;
}

static int pci_resource_end(struct pci_dev *pdev, int bar)
{
    int reg = PCI_MAP_REG_START + bar * 4;
    int rv, start, size, flags, type;
    type = pci_mapreg_type(0, pdev->pa.pa_tag, reg);
    rv = pci_mapreg_info(0, pdev->pa.pa_tag, reg, type, &start, &size, &flags);
    if (PCI_MAPREG_TYPE(type) == PCI_MAPREG_TYPE_IO)
        start = pdev->pa.pa_iot->bus_base + start;
    else
        start = pdev->pa.pa_memt->bus_base + start;
    return rv ? 0 : start + size;
}
static int pci_resource_len(struct pci_dev *pdev, int bar)
{
    int reg = PCI_MAP_REG_START + bar * 4;
    int rv, start, size, flags;
    rv = pci_mapreg_type(0, pdev->pa.pa_tag, reg);
    rv = pci_mapreg_info(0, pdev->pa.pa_tag, reg, rv, &start, &size, &flags);
    return rv ? 0 : size;
}

#define ioremap(a,l) (a)
#define iounmap(...)

static inline int is_valid_ether_addr(u_int8_t *addr)
{
    const char zaddr[6] = {0,};

    return !(addr[0] & 1) && !bcmp(addr, zaddr, 6);
}
#define dev_info(d,fmt...) printf(fmt)
#define dev_err(d,fmt...) printf(fmt)
#define dev_warn(d,fmt...) printf(fmt)
#define netdev_info(d,fmt...) printf(fmt)


#define setup_timer(_timer, _function, _data) \
do { \
    (_timer)->function = _function; \
    (_timer)->data = _data; \
    init_timer(_timer); \
} while (0)

#define napi_enable(...) do {} while(0)

#define PCI_STATUS      0x06    /* 16 bits */
#define  PCI_STATUS_INTERRUPT   0x08    /* Interrupt status */
#define  PCI_STATUS_CAP_LIST    0x10    /* Support Capability List */
#define  PCI_STATUS_66MHZ   0x20    /* Support 66 Mhz PCI 2.1 bus */
#define  PCI_STATUS_UDF     0x40    /* Support User Definable Features [obsolete] */
#define  PCI_STATUS_FAST_BACK   0x80    /* Accept fast-back to back */
#define  PCI_STATUS_PARITY  0x100   /* Detected parity error */
#define  PCI_STATUS_DEVSEL_MASK 0x600   /* DEVSEL timing */
#define  PCI_STATUS_DEVSEL_FAST     0x000
#define  PCI_STATUS_DEVSEL_MEDIUM   0x200
#define  PCI_STATUS_DEVSEL_SLOW     0x400
#define  PCI_STATUS_SIG_TARGET_ABORT    0x800 /* Set on target abort */
#define  PCI_STATUS_REC_TARGET_ABORT    0x1000 /* Master ack of " */
#define  PCI_STATUS_REC_MASTER_ABORT    0x2000 /* Set on master abort */
#define  PCI_STATUS_SIG_SYSTEM_ERROR    0x4000 /* Set when we drive SERR */
#define  PCI_STATUS_DETECTED_PARITY 0x8000 /* Set on parity error */

//------------------------------------
#include "e1000_api.h"
#include <linux/list.h>
#define container_of list_entry
#undef ALIGN
#define ALIGN(x,a) (((x)+(a)-1)&~((a)-1))
#define netdev_tx_t int

#include "igb.h"

#define netdev_err(dev,s...) printf(s)
#define vlan_get_protocol(skb) skb->protocol
#define skb_padto(...) -1
#define NETDEV_TX_OK 0      /* driver took care of packet */
#define NETDEV_TX_BUSY 1    /* driver tx path was busy*/
#define NETDEV_TX_LOCKED -1 /* driver tx lock was already taken */

#define NETIF_F_MULTI_QUEUE 0
#define netif_is_multiqueue(a) 0
#define netif_wake_subqueue(a, b)
#define skb_record_rx_queue(a, b) do {} while (0)
#define dma_mapping_error(...) 0

#ifndef DIV_ROUND_UP
#define DIV_ROUND_UP(n,d) (((n) + (d) - 1) / (d))
#endif
#define netdev_tx_sent_queue(_q, _b) do {} while (0)
#define __skb_put skb_put
#define eth_type_trans(...) 0

#define  PCI_CAP_ID_EXP     0x10    /* PCI Express */

#define  PCI_HEADER_TYPE_NORMAL     0
#define  PCI_HEADER_TYPE_BRIDGE     1
#define  PCI_HEADER_TYPE_CARDBUS    2
#define PCI_CAPABILITY_LIST 0x34    /* Offset of first capability list entry */
#define PCI_CB_CAPABILITY_LIST  0x14
#define PCI_FIND_CAP_TTL    48
#define PCI_CAP_LIST_ID     0   /* Capability ID */
#define PCI_CAP_LIST_NEXT   1   /* Next capability in the list */
#define PCI_HEADER_TYPE     0x0e    /* 8 bits */

#define vfree(x,...) free(x,M_DEVBUF)
#define napi_gro_receive(_napi, _skb) netif_receive_skb(_skb)
#define read_barrier_depends()
#define dma_unmap_addr(PTR, ADDR_NAME)      ((PTR)->ADDR_NAME)
#define dma_unmap_len(PTR, LEN_NAME)        ((PTR)->LEN_NAME)
#define dma_unmap_len_set(PTR, LEN_NAME, VAL)   (((PTR)->LEN_NAME) = (VAL))
#define dma_unmap_addr_set(PTR, ADDR_NAME, VAL)	(((PTR)->ADDR_NAME) = (VAL))
#define cpu_to_le64(x) (x)
#define usleep_range(min, max)  msleep(DIV_ROUND_UP(min, 1000))
#define netif_set_real_num_tx_queues(_netdev, _count) do {} while(0)
#define netif_set_real_num_rx_queues(...) 0
#define netif_tx_start_all_queues(...)
#define prefetch(...)
#define netdev_tx_completed_queue(_q, _p, _b) do {} while (0)
#define netdev_completed_queue(_n, _p, _b) do {} while (0)
#define netdev_tx_sent_queue(_q, _b) do {} while (0)
#define netdev_sent_queue(_n, _b) do {} while (0)
#define netdev_tx_reset_queue(_q) do {} while (0)
#define netdev_reset_queue(_n) do {} while (0)
#define netif_napi_del(...)
#define pm_schedule_suspend(...)
#define pm_runtime_resume(...)
#define netif_tx_wake_all_queues(...)

#define kzalloc(size,gfp) vzalloc(size)
#define kfree_rcu(_ptr, _offset) kfree(_ptr)

#define PCI_VDEVICE(vendor, device)     \
     PCI_VENDOR_ID_##vendor, (device),   \
     PCI_ANY_ID, PCI_ANY_ID, 0, 0

#define PCI_VENDOR_ID_INTEL     0x8086
static int pci_read_config_word(struct pci_dev *linuxpd, int reg, u16 *val)
{
    int data;
    if ((reg & 1) || reg < 0 || reg >= 0x100)
    {
        printf("pci_read_config_word: bad reg %x\n", reg);
        return -1;
    }

    data = _pci_conf_readn(linuxpd->pa.pa_tag, reg & ~3, 4);
    *val = (data >> ((reg & 3) << 3)) & 0xffff;
    return 0;
}

static int pci_write_config_word(struct pci_dev *linuxpd, int reg, u16 val)
{
    unsigned int data;
    if ((reg & 1) || reg < 0 || reg >= 0x100)
    {
        printf("pci_write_config_word: bad reg %x\n", reg);
        return -1;
    }
    data = _pci_conf_readn(linuxpd->pa.pa_tag, reg & ~3, 4);

    data = (data & ~(0xffff << ((reg & 3) << 3))) |
           (val << ((reg & 3) << 3));

    _pci_conf_writen(linuxpd->pa.pa_tag, reg&~3, data, 4);
    return 0;
}

static int pci_read_config_byte(struct pci_dev *linuxpd, int reg, u8 *val)
{
    unsigned int data;
    if (reg < 0 || reg >= 0x100)
    {
        printf("pci_write_config_word: bad reg %x\n", reg);
        return -1;
    }

    data = _pci_conf_readn(linuxpd->pa.pa_tag, reg & ~3, 4);
    *val = (data >> ((reg & 3) << 3)) & 0xff;
    return 0;
}

static inline void *vzalloc(unsigned long size)
{
    void *addr = malloc(size, M_DEVBUF, M_DONTWAIT);
    if (addr)
        memset(addr, 0, size);
    return addr;
}



static int __pci_bus_find_cap_start(struct pci_dev *dev, u8 hdr_type)
{
    u16 status;

    pci_read_config_word(dev, PCI_STATUS, &status);
    if (!(status & PCI_STATUS_CAP_LIST))
        return 0;

    switch (hdr_type)
    {
    case PCI_HEADER_TYPE_NORMAL:
    case PCI_HEADER_TYPE_BRIDGE:
        return PCI_CAPABILITY_LIST;
    case PCI_HEADER_TYPE_CARDBUS:
        return PCI_CB_CAPABILITY_LIST;
    default:
        return 0;
    }

    return 0;
}


static int __pci_find_next_cap_ttl(struct pci_dev *dev,
                                   u8 pos, int cap, int *ttl)
{
    u8 id;

    while ((*ttl)--)
    {
        pci_read_config_byte(dev, pos, &pos);
        if (pos < 0x40)
            break;
        pos &= ~3;
        pci_read_config_byte(dev, pos + PCI_CAP_LIST_ID,
                             &id);
        if (id == 0xff)
            break;
        if (id == cap)
            return pos;
        pos += PCI_CAP_LIST_NEXT;
    }
    return 0;
}

static int __pci_find_next_cap(struct pci_dev *dev,
                               u8 pos, int cap)
{
    int ttl = PCI_FIND_CAP_TTL;

    return __pci_find_next_cap_ttl(dev, pos, cap, &ttl);
}


/**
 * pci_find_capability - query for devices' capabilities
 * @dev: PCI device to query
 * @cap: capability code
 *
 * Tell if a device supports a given PCI capability.
 * Returns the address of the requested capability structure within the
 * device's PCI configuration space or 0 in case the device does not
 * support it.  Possible values for @cap:
 *
 *  %PCI_CAP_ID_PM           Power Management
 *  %PCI_CAP_ID_AGP          Accelerated Graphics Port
 *  %PCI_CAP_ID_VPD          Vital Product Data
 *  %PCI_CAP_ID_SLOTID       Slot Identification
 *  %PCI_CAP_ID_MSI          Message Signalled Interrupts
 *  %PCI_CAP_ID_CHSWP        CompactPCI HotSwap
 *  %PCI_CAP_ID_PCIX         PCI-X
 *  %PCI_CAP_ID_EXP          PCI Express
 */
int pci_find_capability(struct pci_dev *dev, int cap)
{
    int pos;
    char hdr_type;

    pci_read_config_byte(dev, PCI_HEADER_TYPE, &hdr_type);
    hdr_type &= 0x7f;

    pos = __pci_bus_find_cap_start(dev, hdr_type);
    if (pos)
        pos = __pci_find_next_cap(dev, pos, cap);

    return pos;
}

static char ether_addr[6];
static int  ether_inited;


static int get_ethernet_addr(u8 *macaddr)
{
    int i;
    if (ether_inited)
    {
        ether_addr[5]++;
    }
    else
    {
        u8 v;
        char *s = getenv("ethaddr");
        if (s)
        {
            int allz, allf;
            u8 macaddr[6];

            for (i = 0, allz = 1, allf = 1; i < 6; i++)
            {
                gethex(&v, s, 2);
                macaddr[i] = v;
                s += 3;         /* Don't get to fancy here :-) */
                if (v != 0) allz = 0;
                if (v != 0xff) allf = 0;
            }
            if (!allz && !allf)
            {
                ether_inited = 1;
                memcpy(ether_addr, macaddr, 6);
            }
        }

        if (!ether_inited)
        {
            extern int ticks;
            int i;
            srand(ticks);
            for (i = 0; i < 6; i++)
                ether_addr[i] = rand() & 0xff;

            ether_addr [0] &= 0xfe; /* clear multicast bit */
            ether_addr [0] |= 0x02; /* set local assignment bit (IEEE802) */
            ether_inited = 1;
        }

    }

    memcpy(macaddr, ether_addr, 6);

    return 0;
}

#define in_interrupt(...) 0


static inline void pci_set_master(struct pci_dev *dev)
{
    u16 cmd;

    pci_read_config_word(dev, PCI_COMMAND, &cmd);
    if (!(cmd & PCI_COMMAND_MASTER))
    {
        printf("PCI: Enabling bus mastering for device\n");
        cmd |= PCI_COMMAND_MASTER;
        pci_write_config_word(dev, PCI_COMMAND, cmd);
    }
}

static inline void pci_set_drvdata(struct pci_dev *dev, void *data)
{
    dev->dev = data;
}

#define BUG() do { printf("BUG on %s:%d\n", __FUNCTION__,__LINE__); while(1); } while(0)

#define BUG_ON(x) do { if(x) BUG(); } while(0)
#define WARN_ON(x) do { if(x) BUG(); } while(0)

#define __sync()				\
	__asm__ __volatile__(			\
		".set	push\n\t"		\
		".set	noreorder\n\t"		\
		".set	mips2\n\t"		\
		"sync\n\t"			\
		".set	pop"			\
		: /* no output */		\
		: /* no input */		\
		: "memory")

# define wmb()	__sync()
# define rmb()	__sync()
# define mb()	__sync()
#define mmiowb() __sync()
#define smp_mb(...) __sync()

#define igb_rx_checksum(...)
#define igb_check_options(...)
#define igb_tx_queue_mapping(_adapter, _skb) ((_adapter)->tx_ring[0])
#define igb_reset_interrupt_capability(...)
#define igb_set_sriov_capability(...) do {} while(0)
#define igb_reset_sriov_capability(...) do {} while(0)
#define igb_configure_msix(...) do {} while(0)
static void igb_free_q_vector(struct igb_adapter *adapter, int v_idx);

#define max_t(type,x,y) ({ \
    type _x = (x); \
    type _y = (y); \
    _x > _y ? _x : _y; })

#define PCI_VENDOR_ID_HP        0x103c

static irqreturn_t igb_intr(int irq, void *data);
static void igb_free_all_tx_resources(struct igb_adapter *adapter);
static void igb_clean_tx_ring(struct igb_ring *tx_ring);
static void igb_free_all_rx_resources(struct igb_adapter *adapter);
static int igb_poll(struct napi_struct *napi, int budget);

#ifndef ETHTOOL_GEEPROM
#define ETHTOOL_GEEPROM 0xb
#undef ETHTOOL_GREGS
struct ethtool_eeprom {
	u32 cmd;
	u32 magic;
	u32 offset;
	u32 len;
	u8 data[0];
};

struct ethtool_value {
	u32 cmd;
	u32 data;
};
#endif /* ETHTOOL_GEEPROM */


#endif
