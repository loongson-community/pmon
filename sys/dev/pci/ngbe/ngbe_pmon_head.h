#ifndef __LINUX_NET_HEAD_H__
#define __LINUX_NET_HEAD_H__
#define NGBE_NO_LRO
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

#define ETH_FRAME_LEN   1514        /* Max. octets in frame sans FCS */
#include <linux/types.h>
typedef unsigned long physaddr_t;
#define PCI_SUBSYSTEM_VENDOR_ID 0x2c
#define PCI_ANY_ID (~0)
#define KERN_DEBUG
#define kmalloc(size,...)  malloc(size,M_DEVBUF, M_DONTWAIT)
#define kfree(addr,...) free(addr,M_DEVBUF);
#define netdev_priv(dev) dev->priv
#define writeb(b,addr) ((*(volatile unsigned char *)(addr)) = (b))
#define writew(w,addr) ((*(volatile unsigned short *)(addr)) = (w))
#define writel(l,addr) ((*(volatile unsigned int *)(addr)) = (l))

#define readb(addr)     (*(volatile unsigned char *)(addr))
#define readw(addr)     (*(volatile unsigned short *)(addr))
#define readl(addr)     (*(volatile unsigned int *)(addr))
#define KERN_WARNING
#define printk printf
#define le16_to_cpu(x) (x)
#define KERN_INFO
#define PCI_COMMAND     0x04    /* 16 bits */
#define  PCI_COMMAND_MASTER 0x4 /* Enable bus mastering */
#define PCI_REVISION_ID 8

#define KERN_ERR
#define KERN_NOTICE
#define ETH_ALEN    6       /* Octets in one ethernet addr   */
#define udelay delay
#define msleep mdelay
typedef unsigned int bool;
#define __iomem
#define __maybe_unused
#define __aligned(x)			__attribute__((__aligned__(x)))
#define ____cacheline_internodealigned_in_smp __aligned(64)
#define ____cacheline_aligned_in_smp  __aligned(64)
#define  __always_unused
#define __stringify_1(x...)     #x
#define __stringify(x...)       __stringify_1(x)
#define DEFINE_DMA_UNMAP_ADDR(ADDR_NAME)	dma_addr_t ADDR_NAME
#define DEFINE_DMA_UNMAP_LEN(LEN_NAME)		unsigned int LEN_NAME
#ifndef DECLARE_BITMAP
#ifndef BITS_TO_LONGS
#define BITS_TO_LONGS(bits) (((bits) + BITS_PER_LONG - 1) / BITS_PER_LONG)
#endif
#define DECLARE_BITMAP(name, bits) long name[BITS_TO_LONGS(bits)]
#endif
#ifndef VLAN_N_VID
#define VLAN_N_VID	VLAN_GROUP_ARRAY_LEN
#endif /* VLAN_N_VID */
#define VLAN_GROUP_ARRAY_LEN          4096
static inline void mdelay(int microseconds)
{
    int i;
    for (i = 0; i < microseconds; i++)delay(1000);
}
#define le32_to_cpu(x) (x)
#define cpu_to_le32(x) (x)
#define spin_lock_irqsave(...)
#define spin_unlock_irqrestore(...)
#define __init

#define true 1
#define false 0

struct net_device ;
#include "ngbe.h"
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
    //We use mbuf here
    int     packetlen;
    struct pci_device pcidev;
    int mtu;
    char            name[IFNAMSIZ];
    unsigned char hw_addr[6];
    unsigned long       state;
    unsigned long trans_start, last_rx;
    struct emerald_nic nic;
    struct emerald_nic *priv;

    int (*open)(struct net_device *ndev);
    int (*stop)(struct net_device *ndev);
    int (*hard_start_xmit)(struct sk_buff *skb, struct net_device *ndev);
    struct io_buffer *iob_tx[EMERALD_NUM_TX_DESC];
};

#define alloc_etherdev(len) container_of(pci, struct net_device, pcidev)

struct pci_device_id
{
    unsigned int vendor, device;        /* Vendor and device ID or PCI_ANY_ID */
    unsigned long driver_data;      /* Data private to the driver */
};
#define PCI_ROM(v,d,name,info,data) {v,d,data}


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

struct io_buffer {
int len;
char *data;
};

static int iob_len(struct io_buffer *iob)
{
return iob->len;
}

static struct io_buffer *alloc_iob(int len)
{
    struct io_buffer *iobuf = malloc(sizeof(struct io_buffer), M_DEVBUF, M_DONTWAIT);
    iobuf->data = malloc(len, M_DEVBUF, M_DONTWAIT);
    iobuf->len = 0;
    return iobuf;
}

static int iob_put(struct io_buffer *iob, int len)
{
    iob->len = len;
    return 0;
}

static int free_iob(struct io_buffer *iob)
{
	free(iob->data, M_DEVBUF);
	free(iob,M_DEVBUF);
    return 0;
}


static inline int netdev_rx_err( struct net_device *netdev, struct io_buffer *iob, int err)
{
    struct mbuf *m;
    struct ether_header *eh;
    struct ifnet *ifp = &netdev->arpcom.ac_if;
    m = getmbuf();
    if (m == NULL)
    {
        printf("getmbuf failed in  netif_rx\n");
        return 0; // no successful
    }

    bcopy(iob->data, mtod(m, caddr_t), iob->len);

    //hand up  the received package to upper protocol for further dealt
    m->m_pkthdr.rcvif = ifp;
    m->m_pkthdr.len = m->m_len = iob->len - sizeof(struct ether_header);

    eh = mtod(m, struct ether_header *);

    m->m_data += sizeof(struct ether_header);
    ether_input(ifp, eh, m);
    free_iob(iob);
    return 0;
}

static inline int netdev_rx( struct net_device *netdev, struct io_buffer *iob)
{
 return netdev_rx_err(netdev, iob, 0);
}
//--------------------------------------------------------------------

#define HZ 100
#define MAX_ADDR_LEN    6       /* Largest hardware address length */
#define IFNAMSIZ 16

#include <linux/list.h>
struct pci_device;

#define PCI_MAP_REG_START 0x10
static int ioremap_bar0(struct pci_device *pdev, int iosize)
{
    int reg = PCI_MAP_REG_START + 0 * 4;
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

#define ioremap(p,size) ioremap_bar0(pci, size)


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



#include <linux/list.h>
#define container_of list_entry
#undef ALIGN
#define ALIGN(x,a) (((x)+(a)-1)&~((a)-1))
#define netdev_tx_t int



#define kzalloc(size,gfp) vzalloc(size)
#define kfree_rcu(_ptr, _offset) kfree(_ptr)

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
#define profile_start(...)
#define profile_stop(...)
#define profile_exclude(...)
#define DBGC(d, fmt...) printf(fmt)
#define DBGC2(d, fmt...) //printf(fmt)
#define virt_to_bus(x) ((long)(x)&0x1fffffff)
#define __unused
#define PCI_FUNC(tag)		((pci->pa.pa_tag>>8) & 0x07)

#define assert(p) do {	\
	if (!(p)) {	\
		printf("BUG at %s:%d assert(%s)\n",	\
		       __FILE__, __LINE__, #p);			\
		while(1);	\
	}		\
} while (0)
static void free_dma(void *cpu_addr, size_t size)
{
    free(cpu_addr,M_DEVBUF);
}

static void *malloc_dma(size_t size, size_t size1)
{
    void *buf;
    buf = malloc(size, M_DEVBUF, M_DONTWAIT);
    return (void *)buf;
}
#define netdev_tx_complete_next(netdev) do { \
	free_iob(netdev->iob_tx[tx_idx]); \
} while(0)
#define cpu_to_le64(x) (x)
#define cpu_to_le16(x) (x)
static inline netdev_link_up( struct net_device *netdev)
{
	if (!(netdev->state & 1))
		printk("%s: link up\n", netdev->name);
	netdev->state |= 1;
}
static inline netdev_link_down( struct net_device *netdev)
{
	if (netdev->state & 1)
		printk("%s: link down\n", netdev->name);
	netdev->state &= ~1;
}
#define adjust_pci_device(...)
#define netdev_nullify(...)
#define netdev_put(...)
#define eth_ntoa  ether_ntoa
#define register_netdev(...) 0
#define unregister_netdev(...) 0
#define netdev_init(...)
#endif
