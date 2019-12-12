#include <sys/fcntl.h>
//----------------------------------------------------------
#define INET
static int ngbe_ether_ioctl(struct ifnet *ifp, FXP_IOCTLCMD_TYPE cmd, caddr_t data);
static const struct pci_device_id ngbe_pci_tbl[];

const static struct pci_device_id *
pci_match_device(const struct pci_device_id *ids,   struct pci_attach_args *pa)
{
    unsigned short  vendor;
    unsigned short  device, class;
    unsigned short  subsystem_vendor;
    unsigned short  subsystem_device;
    unsigned int i;

    vendor = pa->pa_id & 0xffff;
    device = (pa->pa_id >> 16) & 0xffff;
    class = (pa->pa_class >> 8);
    i = pci_conf_read(0, pa->pa_tag, PCI_SUBSYSTEM_VENDOR_ID);
    subsystem_vendor = i & 0xffff;
    subsystem_device = i >> 16;

    while (ids->vendor)
    {
        if ((ids->vendor == PCI_ANY_ID || ids->vendor == vendor) &&
                (ids->device == PCI_ANY_ID || ids->device == device))
            return ids;
        ids++;
    }
    return NULL;
}

/*
 * Check if a device is an 82557.
 */
static void ngbe_start(struct ifnet *ifp);
static int ngbe_match(parent, match, aux)
struct device *parent;
#if defined(__BROKEN_INDIRECT_CONFIG) || defined(__OpenBSD__)
void *match;
#else
struct cfdata *match;
#endif
void *aux;
{
    struct pci_attach_args *pa = aux;
    struct pci_device_id *pci_id;
    pci_id = pci_match_device(emerald_nics, pa);
    return pci_id ? 1 : 0;
}

extern char activeif_name[];
static int ngbe_intr_handler(void *data)
{
	struct net_device *netdev = data;
	struct ifnet *ifp = &netdev->arpcom.ac_if;
	if (ifp->if_flags & IFF_RUNNING) {
	emerald_poll(netdev);
	}
	return 0;
}

static void if_watchdog(struct ifnet *ifp)
{
    struct net_device *dev = ifp->if_softc;
    //net_em_timer(dev);
}

static void ngbe_attach(parent, self, aux)
struct device *parent, *self;
void *aux;
{
    struct net_device  *sc = (struct net_device *)self;
    struct pci_attach_args *pa = aux;
    //pci_chipset_tag_t pc = pa->pa_pc;
    pci_intr_handle_t ih;
    struct ifnet *ifp;
#ifdef __OpenBSD__
    //bus_space_tag_t iot = pa->pa_iot;
    //bus_addr_t iobase;
    //bus_size_t iosize;
#endif

    sc->pcidev.pa = *pa;
    sc->priv = &sc->nic;
    /* Do generic parts of attach. */
    if (emerald_probe(&sc->pcidev))
    {
        /* Failed! */
        return;
    }
    tgt_poll_register(IPL_NET, ngbe_intr_handler, sc);

#ifdef __OpenBSD__
    ifp = &sc->arpcom.ac_if;
    bcopy(sc->hw_addr, sc->arpcom.ac_enaddr, 6);
#else
    ifp = &sc->sc_ethercom.ec_if;
#endif
    bcopy(sc->sc_dev.dv_xname, ifp->if_xname, IFNAMSIZ);
    bcopy(sc->sc_dev.dv_xname, sc->name, IFNAMSIZ);

    ifp->if_softc = sc;
    ifp->if_flags = IFF_BROADCAST | IFF_SIMPLEX | IFF_MULTICAST;
    ifp->if_ioctl = ngbe_ether_ioctl;
    ifp->if_start = ngbe_start;
    ifp->if_watchdog = if_watchdog;

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
    ether_ifattach(ifp, sc->hw_addr);
#else
    ether_ifattach(ifp);
#endif
#if NBPFILTER > 0
#ifdef __OpenBSD__
    bpfattach(&sc->arpcom.ac_if.if_bpf, ifp, DLT_EN10MB,
              sizeof(struct ether_header));
#else
    bpfattach(&sc->sc_ethercom.ec_if.if_bpf, ifp, DLT_EN10MB,
              sizeof(struct ether_header));
#endif
#endif

}


/*
 * Start packet transmission on the interface.
 */



static void ngbe_start(struct ifnet *ifp)
{
    struct net_device *sc = ifp->if_softc;
    struct mbuf *mb_head;
    struct io_buffer *iob;

    while (ifp->if_snd.ifq_head != NULL)
    {

        IF_DEQUEUE(&ifp->if_snd, mb_head);

        iob = alloc_iob(mb_head->m_pkthdr.len);
        m_copydata(mb_head, 0, mb_head->m_pkthdr.len, iob->data);
        iob->len = mb_head->m_pkthdr.len;
	if (emerald_transmit (sc, iob) < 0)
	   free_iob(iob);

        m_freem(mb_head);
    }
}

static int ngbe_init(struct net_device *netdev)
{
	struct ifnet *ifp = &netdev->arpcom.ac_if;
	if (!(ifp->if_flags & IFF_RUNNING)) {
		ifp->if_flags |= IFF_RUNNING;
		emerald_open(netdev);
	}
	return 0;
}

static int ngbe_stop(struct net_device *netdev)
{
	struct ifnet *ifp = &netdev->arpcom.ac_if;
	ifp->if_timer = 0;
	if (ifp->if_flags & IFF_RUNNING) {
		ifp->if_flags &= ~(IFF_RUNNING | IFF_OACTIVE);
		emerald_close(netdev);
	}
	return 0;
}

static int ngbe_ether_ioctl(ifp, cmd, data)
struct ifnet *ifp;
FXP_IOCTLCMD_TYPE cmd;
caddr_t data;
{
    struct ifaddr *ifa = (struct ifaddr *) data;
    struct net_device *sc = ifp->if_softc;
    int error = 0;

    int s;
    s = splimp();

    switch (cmd)
    {
#ifdef PMON
    case SIOCPOLL:
        break;
#endif
    case SIOCSIFADDR:

        switch (ifa->ifa_addr->sa_family)
        {
#ifdef INET
        case AF_INET:
            error = ngbe_init(sc);
            if (error < 0)
                return (error);
            ifp->if_flags |= IFF_UP;

#ifdef __OpenBSD__
            arp_ifinit(&sc->arpcom, ifa);
#else
            arp_ifinit(ifp, ifa);
#endif

            break;
#endif

        default:
            error = ngbe_init(sc);
            if (error < 0)
                return (error);
            ifp->if_flags |= IFF_UP;
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
        if (ifp->if_flags & IFF_UP)
        {
            error = ngbe_init(sc);
            if (error < 0)
                return (error);
        }
        else
        {
            if (ifp->if_flags & IFF_RUNNING)
                ngbe_stop(sc);
        }
        break;
    break;

    default:
        error = EINVAL;
    }

    splx(s);
    return (error);
}

struct cfattach ngbe_ca =
{
    sizeof(struct net_device), ngbe_match, ngbe_attach
};

struct cfdriver ngbe_cd =
{
    NULL, "ngbe", DV_IFNET
};

