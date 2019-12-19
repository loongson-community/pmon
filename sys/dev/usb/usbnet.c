#include <sys/types.h>
#include <sys/param.h>
#include <sys/systm.h>
#include <sys/kernel.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/buf.h>
#include <sys/uio.h>
#include <sys/malloc.h>
#include <sys/errno.h>
#include <sys/device.h>
#include <sys/disklabel.h>
#include <sys/disk.h>
#include <sys/queue.h>
#include <pmon.h>
#include <dev/pci/etherboot.h>

#include "usb.h"

#include <sys/socket.h>
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
#include <sys/mbuf.h>
#include <ctype.h>
#define debug printf


#define USB_CLASS_CDC_DATA		0x0a
#define USB_CDC_SUBCLASS_ETHERNET		0x06
#define USB_CDC_SUBCLASS_ACM			0x02

struct net_device 
{
	struct device sc_dev;		/* generic device structures */
	void *sc_ih;			/* interrupt handler cookie */
	struct arpcom arpcom;		/* per-interface network data !!we use this*/
	unsigned char dev_addr[6];
	int addr_len;
	struct usb_device *dev;
	int ep_in, ep_out, ep_int;
	int irqinterval;
};

static int ep_in, ep_out, ep_int;
static int irqinterval;
static char *NetRxPacket;
static struct usb_interface_descriptor *iface;

static int
usbnet_match(parent, match, aux)
	struct device *parent;
#if defined(__BROKEN_INDIRECT_CONFIG) || defined(__OpenBSD__)
	void *match;
#else
	struct cfdata *match;
#endif
	void *aux;
{
	struct usb_device *dev = aux;
	int i, ret;
	unsigned int flags = 0;

	int protocol = 0;
	int subclass = 0;
	int ifnum = 0;

	for(ifnum = 0;ifnum < dev->config.no_of_if; ifnum++) {
	/* let's examine the device now */
	iface = &dev->config.if_desc[ifnum];

printf("\n\n\nusbnet match 0x%x 0x%x 0x%x 0x%x\n", dev->descriptor.bDeviceClass, iface->bInterfaceClass, iface->bInterfaceSubClass, ifnum);

	if (!((dev->descriptor.bDeviceClass == USB_CLASS_COMM && iface->bInterfaceClass == USB_CLASS_COMM && iface->bInterfaceSubClass == USB_CDC_SUBCLASS_ETHERNET) \
	||(dev->descriptor.bDeviceClass == USB_CLASS_COMM && iface->bInterfaceClass == USB_CLASS_CDC_DATA) \
	||(dev->descriptor.bDeviceClass == 0 && iface->bInterfaceClass == USB_CLASS_CDC_DATA) \
	)) {
		/* if it's not a mass storage, we go no further */
		continue;
	}

printf("\n\n\nok now usbnet match 0x%x 0x%x 0x%x\n", dev->descriptor.bDeviceClass, iface->bInterfaceClass, iface->bInterfaceSubClass);
printf("iface->bNumEndpoints=%d\n", iface->bNumEndpoints);
	for (i = 0; i < iface->bNumEndpoints; i++) {
		/* is it an BULK endpoint? */
		if ((iface->ep_desc[i].bmAttributes &  USB_ENDPOINT_XFERTYPE_MASK)
		    == USB_ENDPOINT_XFER_BULK) {
			if (iface->ep_desc[i].bEndpointAddress & USB_DIR_IN)
				ep_in = iface->ep_desc[i].bEndpointAddress &
					USB_ENDPOINT_NUMBER_MASK;
			else
				ep_out = iface->ep_desc[i].bEndpointAddress &
					USB_ENDPOINT_NUMBER_MASK;
		}

		/* is it an interrupt endpoint? */
		if ((iface->ep_desc[i].bmAttributes & USB_ENDPOINT_XFERTYPE_MASK)
		    == USB_ENDPOINT_XFER_INT) {
			ep_int = iface->ep_desc[i].bEndpointAddress &
				USB_ENDPOINT_NUMBER_MASK;
			irqinterval = iface->ep_desc[i].bInterval;
		}
	}

	if (!ep_in  || !ep_out)
          continue;
	printf("Endpoints In %d Out %d Int %d\n",
		  ep_in, ep_out, ep_int);

	ret = usb_set_interface(dev, iface->bInterfaceNumber, iface->bAlternateSetting);
	if (ret) {
		debug("usbnet: %s: Cannot set interface: %d\n", __func__,
		      ret);
	  return 0;
	}
	return 1;
}
}

int usb_eth_send(struct ifnet *ifp, void *packet, int length)
{
	struct net_device *sc = ifp->if_softc;
	struct usb_device * dev = sc->dev;
	unsigned int pipein, pipeout;
	int partial, result, stat;

	pipein = usb_rcvbulkpipe(dev, sc->ep_in);
	pipeout = usb_sndbulkpipe(dev, sc->ep_out);
	result = usb_bulk_msg(dev, pipeout, packet,
					      length, &partial, USB_CNTL_TIMEOUT);

	if(dev->status!=0) {
		/* if we stall, we need to clear it before we go on */
		if (dev->status & USB_ST_STALLED) {
			printf("stalled ->clearing endpoint halt for pipe 0x%x\n", pipeout);
			stat = dev->status;
			usb_clear_halt(dev, pipeout);
			dev->status=stat;
			if(length == partial) {
				printf("bulk transferred with error %X, but data ok\n",dev->status);
				return 0;
			}
			else
				return result;
		}
		if (dev->status & USB_ST_NAK_REC) {
			printf("Device NAKed bulk_msg\n");
			return result;
		}
		if(length == partial) {
			printf("bulk transferred with error %d, but data ok\n",dev->status);
			return 0;
		}
		return result;
	}
	return 0;
}

static void usbnet_start(struct ifnet *ifp)
{
	struct net_device *sc = ifp->if_softc;
	struct mbuf *mb_head;		
	void *packet;

	while(ifp->if_snd.ifq_head != NULL ){
		
		IF_DEQUEUE(&ifp->if_snd, mb_head);
		
		packet = malloc(mb_head->m_pkthdr.len,M_DEVBUF, M_DONTWAIT);
		m_copydata(mb_head, 0, mb_head->m_pkthdr.len, packet);
		usb_eth_send(ifp, packet, mb_head->m_pkthdr.len);
		m_freem(mb_head);
		free(packet, M_DEVBUF);
		wbflush();
	} 
}


#define	SYNC_R	0	/* Sync caches for reading data */
#define	SYNC_W	1	/* Sync caches for writing data */

#define	RFA_ALIGNMENT_FUDGE	2

static struct mbuf * getmbuf()
{
	struct mbuf *m;


	MGETHDR(m, M_DONTWAIT, MT_DATA);
	if(m == NULL){
		printf("getmbuf for reception failed\n");
		return  NULL;
	} else {
		MCLGET(m, M_DONTWAIT);
		if ((m->m_flags & M_EXT) == 0) {
			m_freem(m);
			return NULL;
		}
		if(m->m_data != m->m_ext.ext_buf){
			printf("m_data not equal to ext_buf!!!\n");
		}
	}
	
#if defined(__mips__)
	/*
	 * Sync the buffer so we can access it uncached.
	 */
#if 0
	if (m->m_ext.ext_buf!=NULL) {
    	CPU_IOFlushDCache((vm_offset_t)m->m_ext.ext_buf, MCLBYTES, SYNC_R);
	}
#endif
	m->m_data += RFA_ALIGNMENT_FUDGE;
#else
	m->m_data += RFA_ALIGNMENT_FUDGE;
#endif
	return m;
}


void
NetReceive(struct ifnet *ifp, volatile unsigned char * inpkt, int len)
{
	struct mbuf *m;
	struct ether_header *eh;

	m =getmbuf();

	if (m == NULL){
		printf("getmbuf failed in  netif_rx\n");
		return 0; // no successful
	}

	bcopy(inpkt, mtod(m, caddr_t), len);


    //hand up  the received package to upper protocol for further dealt
    m->m_pkthdr.rcvif = ifp;
    m->m_pkthdr.len = m->m_len = len -sizeof(struct ether_header);

    eh=mtod(m, struct ether_header *);

    m->m_data += sizeof(struct ether_header);
	ether_input(ifp, eh, m);

}

static int usb_net_irq(struct usb_device *dev)
{
	int i,res;
	int k_index=0;
	struct net_device  *sc;
	struct ifnet *ifp;
	int ret = 1;
	sc = dev->match;
	ifp = &sc->arpcom.ac_if;

	//printf("usb_neet_irq run:%d state 0x%x\n",  !!(ifp->if_flags & IFF_RUNNING), dev->status);
	if (!(ifp->if_flags & IFF_RUNNING)) {
	   dev->irq_handle_ep[sc->ep_in] = NULL;
	   ret = 0;
       }

	if (dev->status)
	  return ret;

	if((dev->irq_status!=0)||(!dev->irq_act_len))
	  return ret;

	NetReceive(ifp, NetRxPacket, dev->irq_act_len);
	return ret;
}


#define USB_CDC_SET_ETHERNET_PACKET_FILTER	0x43
#define	USB_CDC_PACKET_TYPE_PROMISCUOUS		(1 << 0)
#define	USB_CDC_PACKET_TYPE_ALL_MULTICAST	(1 << 1) /* no filter */
#define	USB_CDC_PACKET_TYPE_DIRECTED		(1 << 2)
#define	USB_CDC_PACKET_TYPE_BROADCAST		(1 << 3)
#define	USB_CDC_PACKET_TYPE_MULTICAST		(1 << 4) /* filtered */

#define PKTSIZE			1518
#define PKTSIZE_ALIGN		1536
#define RX_EXTRA	20		/* guard against rx overflows */
int usb_eth_init(struct ifnet *ifp)
{
	struct net_device *sc = ifp->if_softc;
	struct usb_device * dev = sc->dev;
	unsigned int pipe, pipein, pipeout;
	int partial, result, size;
	int maxpacket;

	if (!ifp) {
		printf("received NULL ptr\n");
		return 0;
	}

	pipein = usb_rcvbulkpipe(dev, sc->ep_in);
	pipeout = usb_sndbulkpipe(dev, sc->ep_out);
	dev->irq_handle_ep[sc->ep_in]= usb_net_irq;

	u16 cdc_filter = USB_CDC_PACKET_TYPE_DIRECTED
			| USB_CDC_PACKET_TYPE_BROADCAST;

	/* filtering on the device is an optional feature and not worth
	 * the hassle so we just roughly care about snooping and if any
	 * multicast is requested, we take every multicast
	 */
	//if (net->flags & IFF_PROMISC)
		cdc_filter |= USB_CDC_PACKET_TYPE_PROMISCUOUS;
	//if (!netdev_mc_empty(net) || (net->flags & IFF_ALLMULTI))
	//	cdc_filter |= USB_CDC_PACKET_TYPE_ALL_MULTICAST;

	usb_control_msg(dev,
			usb_sndctrlpipe(dev, 0),
			USB_CDC_SET_ETHERNET_PACKET_FILTER,
			USB_TYPE_CLASS | USB_RECIP_INTERFACE,
			cdc_filter,
			iface->bInterfaceNumber,
			NULL,
			0,
			USB_CNTL_TIMEOUT*5
		);

	size = (ETHER_HDR_SIZE + PKTSIZE_ALIGN + RX_EXTRA);
	maxpacket = usb_maxpacket(dev, pipein);
	size += maxpacket - 1;
	size -= size % maxpacket;
	result = usb_bulk_msg(dev, pipein, NetRxPacket,
					      size, &partial, USB_CNTL_TIMEOUT);

	return 0;
}

void usb_eth_halt(void *ifp)
{
	if (!ifp) {
		error("received NULL ptr");
		return;
	}

}

static int
usbnet_ether_ioctl(ifp, cmd, data)
	struct ifnet *ifp;
	unsigned long cmd;
	caddr_t data;
{
	struct ifaddr *ifa = (struct ifaddr *) data;
	struct net_device *sc = ifp->if_softc;
	int error = 0;
	
	int s;
	s = splimp();
		
	switch (cmd) {
#ifdef PMON
	case SIOCPOLL:
		break;
#endif
	case SIOCSIFADDR:

		switch (ifa->ifa_addr->sa_family) {
#ifdef INET
			case AF_INET:
				if ((ifp->if_flags & IFF_UP) == 0) {
					ifp->if_flags |= IFF_RUNNING;
					error = usb_eth_init(ifp);
					if(error < 0 ) {
						ifp->if_flags &= ~(IFF_RUNNING | IFF_UP);
						break;
					}
					ifp->if_flags |= IFF_UP;
				}

#ifdef __OpenBSD__
				arp_ifinit(&sc->arpcom, ifa);
#else
				arp_ifinit(ifp, ifa);
#endif

				break;
#endif

			default:
				if ((ifp->if_flags & IFF_UP) == 0) {
					ifp->if_flags |= IFF_RUNNING;
					error = usb_eth_init(ifp);
					if(error < 0 ) {
						ifp->if_flags &= ~(IFF_RUNNING | IFF_UP);
						break;
					}
					ifp->if_flags |= IFF_UP;
				}
				break;
		}
		break;
	case SIOCSIFFLAGS:

		/*
		 * If interface is marked up and not running, then start it.
		 * If it is marked down and running, stop it.
		 * XXX If it's up then re-initialize it. This is so flags
		 * such as IFF_PROMISC are handled.
		 */
		if (ifp->if_flags & IFF_UP) {
			ifp->if_flags |= IFF_RUNNING;
			error = usb_eth_init(ifp);
			if(error <0 )
			ifp->if_flags &= ~IFF_RUNNING;
		} else {
			if (ifp->if_flags & IFF_RUNNING)
				usb_eth_halt(ifp);
			ifp->if_flags &= ~IFF_RUNNING;
		}
		break;

	default:
		error = EINVAL;
	}

	splx(s);
	return (error);
}

static inline int is_valid_ether_addr( u_int8_t *addr )
{
    const char zaddr[6] = {0,};

    return !(addr[0]&1) && bcmp( addr, zaddr, 6);
}


static int is_eth_addr_valid(char *str)
{
	if (strlen(str) == 17) {
		int i;
		char *p, *q;
		unsigned char ea[6];

		/* see if it looks like an ethernet address */

		p = str;

		for (i = 0; i < 6; i++) {
			char term = (i == 5 ? '\0' : ':');

			ea[i] = strtoul(p, &q, 16);

			if ((q - p) != 2 || *q++ != term)
				break;

			p = q;
		}

		/* Now check the contents. */
		return is_valid_ether_addr(ea);
	}
	return 0;
}

static u8 nibble(unsigned char c)
{
	if (isdigit(c))
		return c - '0';
	c = toupper(c);
	if (isxdigit(c))
		return 10 + c - 'A';
	return 0;
}

static int get_ether_addr(const char *str, u8 *dev_addr)
{
	if (str) {
		unsigned	i;

		for (i = 0; i < 6; i++) {
			unsigned char num;

			if ((*str == '.') || (*str == ':'))
				str++;
			num = nibble(*str++) << 4;
			num |= (nibble(*str++));
			dev_addr[i] = num;
		}
		if (is_valid_ether_addr(dev_addr))
			return 0;
	}
	return 1;
}

static char dev_addr[18]="00:01:02:03:04:05";
#define USB_DT_CS_INTERFACE		(USB_TYPE_CLASS | USB_DT_INTERFACE)
#define USB_CDC_ETHERNET_TYPE		0x0f	/* ether_desc */

/* "Ethernet Networking Functional Descriptor" from CDC spec 5.2.3.16 */
struct usb_cdc_ether_desc {
	unsigned char	bLength;
	unsigned char	bDescriptorType;
	unsigned char	bDescriptorSubType;

	unsigned char	iMACAddress;
	int	bmEthernetStatistics;
	short	wMaxSegmentSize;
	short	wNumberMCFilters;
	unsigned char	bNumberPowerFilters;
} __attribute__ ((packed));

int usbnet_get_ethernet_addr(struct usb_device *dev, int iMACAddress, char *dev_addr)
{
	int 		tmp = -1, ret;
	unsigned char	buf [13];

	ret = usb_string(dev, iMACAddress, buf, sizeof buf);
	if (ret == 12)
		tmp = get_ether_addr(buf, dev_addr)?-1:0;
	return tmp;
}

struct usb_cdc_ether_desc *cdc_parse_cdc_header(u8 *buffer, int buflen)
{
	struct usb_cdc_ether_desc *ether = NULL;

	unsigned int elength;

	while (buflen > 0) {
		elength = buffer[0];
		if (!elength) {
			printf("skipping garbage byte\n");
			elength = 1;
			goto next_desc;
		}
		if (buffer[1] != USB_DT_CS_INTERFACE) {
			goto next_desc;
		}

		switch (buffer[2]) {
		case USB_CDC_ETHERNET_TYPE:
			if (elength != sizeof(struct usb_cdc_ether_desc))
				goto next_desc;
			ether = (struct usb_cdc_ether_desc *)buffer;
			return ether;
			break;
		}
next_desc:
		buflen -= elength;
		buffer += elength;
	}

	return ether;
}

#define USB_BUFSIZ	512
int usbnet_get_mac(struct usb_device * dev, u8 *addr)
{
	struct usb_cdc_ether_desc *ether;
	unsigned char tmpbuf[USB_BUFSIZ];
	usb_get_configuration_no(dev,&tmpbuf[0],dev->configno);
	
	ether = cdc_parse_cdc_header(tmpbuf, dev->config.wTotalLength);
	if (!ether)
	  return -ENODEV;
        return usbnet_get_ethernet_addr(dev, ether->iMACAddress, addr);

}

int get_ether_dev_addr(struct usb_device * dev, u8 *addr)
{
 
 if (usbnet_get_mac(dev, addr))
   get_ether_addr(getenv("ether")?:dev_addr, addr);
 return 0;
}

static void
usbnet_attach(parent, self, aux)
	struct device *parent, *self;
	void *aux;
{

	struct ifnet* ifp;
	struct net_device  *sc = (struct net_device *)self;
	struct usb_device * dev = aux;
	dev->match = self;
	sc->dev = dev;
	sc->ep_in = ep_in;
	sc->ep_out = ep_out;
	sc->ep_int = ep_int;
	sc->irqinterval = irqinterval;

	NetRxPacket = malloc(0x1000, M_DEVBUF, M_DONTWAIT);

	sc->addr_len = 6;
	get_ether_dev_addr(dev, sc->dev_addr);
#ifdef __OpenBSD__
	ifp = &sc->arpcom.ac_if;
	bcopy(sc->dev_addr, sc->arpcom.ac_enaddr, sc->addr_len);
#else
	ifp = &sc->sc_ethercom.ec_if;
#endif
	bcopy(sc->sc_dev.dv_xname, ifp->if_xname, IFNAMSIZ);
	
	ifp->if_softc = sc;
	ifp->if_flags = IFF_BROADCAST | IFF_SIMPLEX | IFF_MULTICAST;
	ifp->if_ioctl = usbnet_ether_ioctl;
	ifp->if_start = usbnet_start;
	//ifp->if_watchdog = usbnet_watchdog;

	printf(":  address %s\n", 
	    ether_sprintf(sc->arpcom.ac_enaddr));

	/*
	 * Attach the interface.
	 */
	if_attach(ifp);
	/*
	 * Let the system queue as many packets as we have available
	 * TX descriptors.
	 */
	ifp->if_snd.ifq_maxlen = 4;
#ifdef __NetBSD__
	ether_ifattach(ifp, sc->dev_addr);
#else
	ether_ifattach(ifp);
#endif
	
	/*
	 * Add shutdown hook so that DMA is disabled prior to reboot. Not
	 * doing do could allow DMA to corrupt kernel memory during the
	 * reboot before the driver initializes.
	 */
	//shutdownhook_establish(usbnet_shutdown, sc);

	return 0;
}

int ether_start(struct ifnet *ifp)
{
			ifp->if_flags |= IFF_RUNNING;
}

int ether_stop(struct ifnet *ifp)
{
			ifp->if_flags &= ~IFF_RUNNING;
}

struct cfattach usbnet_ca = {
	sizeof(struct net_device), usbnet_match, usbnet_attach
};

struct cfdriver usbnet_cd = {
	NULL, "usbnet", DV_IFNET
};


#if 0
//-------------
#define USB_BULK_RECV_TIMEOUT 500


int usb_ether_receive(struct ueth_data *ueth, int rxsize)
{
	int actual_len;
	int ret;

	if (rxsize > ueth->rxsize)
		return -EINVAL;
	ret = usb_bulk_msg(ueth->pusb_dev,
			   usb_rcvbulkpipe(ueth->pusb_dev, ueth->ep_in),
			   ueth->rxbuf, rxsize, &actual_len,
			   USB_BULK_RECV_TIMEOUT);
	debug("Rx: len = %u, actual = %u, err = %d\n", rxsize, actual_len, ret);
	if (ret) {
		printf("Rx: failed to receive: %d\n", ret);
		return ret;
	}
	if (actual_len > rxsize) {
		debug("Rx: received too many bytes %d\n", actual_len);
		return -ENOSPC;
	}
	ueth->rxlen = actual_len;
	ueth->rxptr = 0;

	return actual_len ? 0 : -EAGAIN;
}

void usb_ether_advance_rxbuf(struct ueth_data *ueth, int num_bytes)
{
	ueth->rxptr += num_bytes;
	if (num_bytes < 0 || ueth->rxptr >= ueth->rxlen)
		ueth->rxlen = 0;
}

int usb_ether_get_rx_bytes(struct ueth_data *ueth, uint8_t **ptrp)
{
	if (!ueth->rxlen)
		return 0;

	*ptrp = &ueth->rxbuf[ueth->rxptr];

	return ueth->rxlen - ueth->rxptr;
}
#endif
