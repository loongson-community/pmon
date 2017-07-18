#include <sys/fcntl.h>
//----------------------------------------------------------
#define INET
static void netdev_init(struct net_device *netdev, struct pci_attach_args *pa)
{
    unsigned short  vendor;
    unsigned short  device, class;
    unsigned short  subsystem_vendor;
    unsigned short  subsystem_device;
    unsigned int i;
    static int irq = 0;

    vendor = pa->pa_id & 0xffff;
    device = (pa->pa_id >> 16) & 0xffff;
    class = (pa->pa_class >> 8);
    i = pci_conf_read(0, pa->pa_tag, PCI_SUBSYSTEM_VENDOR_ID);
    subsystem_vendor = i & 0xffff;
    subsystem_device = i >> 16;
    netdev->pcidev.vendor = vendor;
    netdev->pcidev.device = device;
    netdev->pcidev.subsystem_vendor = subsystem_vendor;
    netdev->pcidev.subsystem_device = subsystem_device;
    netdev->pcidev.class = class;
    netdev->pcidev.devfn = (pa->pa_device<<3)|pa->pa_function;
    netdev->pcidev.pa = *pa;
    netdev->pcidev.irq = irq;
    netdev->priv = kmalloc(sizeof(struct igb_adapter), GFP_KERNEL); //&netdev->em;
    memset(netdev->priv, 0, sizeof(struct igb_adapter));
    netdev->addr_len = 6;
    netdev->irq = irq++;
}

static int igb_ether_ioctl(struct ifnet *ifp, FXP_IOCTLCMD_TYPE cmd, caddr_t data);
static const struct pci_device_id igb_pci_tbl[];

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

    while (ids->vendor || ids->subvendor || ids->class_mask)
    {
        if ((ids->vendor == PCI_ANY_ID || ids->vendor == vendor) &&
                (ids->device == PCI_ANY_ID || ids->device == device) &&
                (ids->subvendor == PCI_ANY_ID || ids->subvendor == subsystem_vendor) &&
                (ids->subdevice == PCI_ANY_ID || ids->subdevice == subsystem_device) &&
                !((ids->class ^ class) & ids->class_mask))
            return ids;
        ids++;
    }
    return NULL;
}

/*
 * Check if a device is an 82557.
 */
static void igb_start(struct ifnet *ifp);
static int igb_match(parent, match, aux)
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
    pci_id = pci_match_device(igb_pci_tbl, pa);
    return pci_id ? 1 : 0;
}

static void
em_shutdown(sc)
void *sc;
{
}


extern char activeif_name[];
static int igb_intr_handler(void *data)
{
	struct net_device *netdev = data;
	int irq = netdev->irq;
	struct ifnet *ifp = &netdev->arpcom.ac_if;
	if (ifp->if_flags & IFF_RUNNING && interrupt)
	{
		if(irqstate&(1<<irq))
		{
			interrupt(0, netdev->priv);
			run_task_queue(&tq_igb);
			if (ifp->if_snd.ifq_head != NULL)
				igb_start(ifp);
		}
		return 1;
	}
	return 0;
}

static void if_watchdog(struct ifnet *ifp)
{
    struct net_device *dev = ifp->if_softc;
    //net_em_timer(dev);
}

static void igb_attach(parent, self, aux)
struct device *parent, *self;
void *aux;
{
    struct net_device  *sc = (struct net_device *)self;
    struct pci_attach_args *pa = aux;
    //pci_chipset_tag_t pc = pa->pa_pc;
    pci_intr_handle_t ih;
    const char *intrstr = NULL;
    struct ifnet *ifp;
#ifdef __OpenBSD__
    //bus_space_tag_t iot = pa->pa_iot;
    //bus_addr_t iobase;
    //bus_size_t iosize;
#endif



    netdev_init(sc, pa);
    /* Do generic parts of attach. */
    if (igb_probe(sc, &sc->pcidev, igb_pci_tbl))
    {
        /* Failed! */
        return;
    }
    tgt_poll_register(IPL_NET, igb_intr_handler, sc);

#ifdef __OpenBSD__
    ifp = &sc->arpcom.ac_if;
    bcopy(sc->dev_addr, sc->arpcom.ac_enaddr, sc->addr_len);
#else
    ifp = &sc->sc_ethercom.ec_if;
#endif
    bcopy(sc->sc_dev.dv_xname, ifp->if_xname, IFNAMSIZ);
    bcopy(sc->sc_dev.dv_xname, sc->name, IFNAMSIZ);

    ifp->if_softc = sc;
    ifp->if_flags = IFF_BROADCAST | IFF_SIMPLEX | IFF_MULTICAST;
    ifp->if_ioctl = igb_ether_ioctl;
    ifp->if_start = igb_start;
    ifp->if_watchdog = if_watchdog;

    printf(": %s, address %s\n", intrstr,
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
#if NBPFILTER > 0
#ifdef __OpenBSD__
    bpfattach(&sc->arpcom.ac_if.if_bpf, ifp, DLT_EN10MB,
              sizeof(struct ether_header));
#else
    bpfattach(&sc->sc_ethercom.ec_if.if_bpf, ifp, DLT_EN10MB,
              sizeof(struct ether_header));
#endif
#endif

    /*
     * Add shutdown hook so that DMA is disabled prior to reboot. Not
     * doing do could allow DMA to corrupt kernel memory during the
     * reboot before the driver initializes.
     */
    shutdownhook_establish(em_shutdown, sc);

#ifndef PMON
    /*
     * Add suspend hook, for similiar reasons..
     */
    powerhook_establish(em_power, sc);
#endif
}


/*
 * Start packet transmission on the interface.
 */



static void igb_start(struct ifnet *ifp)
{
    struct net_device *sc = ifp->if_softc;
    struct mbuf *mb_head;
    struct sk_buff *skb;

    while (ifp->if_snd.ifq_head != NULL)
    {

        IF_DEQUEUE(&ifp->if_snd, mb_head);

        skb = dev_alloc_skb(mb_head->m_pkthdr.len);
        m_copydata(mb_head, 0, mb_head->m_pkthdr.len, skb->data);
        skb->len = mb_head->m_pkthdr.len;
        if(sc->hard_start_xmit(skb, sc) != NETDEV_TX_OK)
	 dev_kfree_skb_any(skb);

        m_freem(mb_head);
    }
}

static int igb_init(struct net_device *netdev)
{
    struct ifnet *ifp = &netdev->arpcom.ac_if;
    int stat = 0;
    ifp->if_flags |= IFF_RUNNING;
    if (!netdev->opencount)
    {
        stat = netdev->open(netdev);
        netdev->opencount++;
    }
    return stat;
}

static int igb_stop(struct net_device *netdev)
{
    struct ifnet *ifp = &netdev->arpcom.ac_if;
    ifp->if_timer = 0;
    ifp->if_flags &= ~(IFF_RUNNING | IFF_OACTIVE);
    if (netdev->opencount)
    {
        netdev->stop(netdev);
        netdev->opencount--;
    }
    return 0;
}

static int igb_ether_ioctl(ifp, cmd, data)
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
            error = igb_init(sc);
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
            error = igb_init(sc);
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
            error = igb_init(sc);
            if (error < 0)
                return (error);
        }
        else
        {
            if (ifp->if_flags & IFF_RUNNING)
                igb_stop(sc);
        }
        break;
    case SIOCETHTOOL:
    {
        long *p = data;
	int ac;
	char **av;
	ac = p[0];
	av = p[1];
	if(ac == 1)
	{
	    struct ethtool_eeprom eeprom;
	    unsigned char bytes[6];
	    int i;
	    eeprom.offset = 0;
	    eeprom.len = 6;

	    igb_get_eeprom(sc,&eeprom, bytes);

	    printf("mac address is ");
	    for (i = 0; i < 6; i++)
	    {
		    printf("%02x:", bytes[i]);
	    }
	    printf("\b.\n");
	}
	else
	{
	    struct ethtool_eeprom eeprom;
	    unsigned char bytes[6];
	    int i;

	    for (i = 0; i < 6; i++)
		  bytes[i] = strtoul(&av[1][i*3],0,16);

	    eeprom.magic = sc->pcidev.vendor | (sc->pcidev.device << 16);
	    eeprom.offset = 0;
	    eeprom.len = 6;
	    igb_set_eeprom(sc,&eeprom, bytes);
	}
    }
    break;
#if 0
    case SIOCGETHERADDR:
    {
        long long val;
        char *p = data;
        mynic_em = sc;
        //val =e1000_read_mac(mynic_em);
        p[5] = val >> 40 & 0xff;
        p[4] = val >> 32 & 0xff;
        p[3] = val >> 24 & 0xff;
        p[2] = val >> 16 & 0xff;
        p[1] = val >> 8 & 0xff;
        p[0] = val & 0xff;

    }
    break;

    case SIOCWRPHY:
    {
        long *p = data;
        int ac;
        char **av;
        int i;
        struct e1000_adapter *adapter = (struct e1000_adapter *)(sc->priv);
        mynic_em = sc;

        ac = p[0];
        av = p[1];
        if (ac > 1)
        {
            //offset:data,data
            int i;
            int offset;
            int data;
            for (i = 1; i < ac; i++)
            {
                char *p = av[i];
                char *nextp;
                int offset = strtoul(p, &nextp, 0);
                while (nextp != p)
                {
                    p = ++nextp;
                    data = strtoul(p, &nextp, 0);
                    if (nextp == p)break;
                    printf("offset=%d,data=0x%x\n", offset, data);
                    //e1000e_write_phy_reg_igp(&adapter->hw,offset, data);
                }
            }
        }
    }
    break;
    case SIOCRDPHY:
    {
        long *p = data;
        int ac;
        char **av;
        int i;
        unsigned data;
        int offset = 0, count = 32;
        struct e1000_adapter *adapter = (struct e1000_adapter *)(sc->priv);
        mynic_em = sc;
        ac = p[0];
        av = p[1];

        if (ac > 1) offset = strtoul(av[1], 0, 0);
        if (ac > 2) count = strtoul(av[2], 0, 0);

        for (i = 0; i < count; i++)
        {
            data = 0;
            e1000e_read_phy_reg_igp(&adapter->hw, i + offset, &data);
            if ((i & 0xf) == 0)printf("\n%02x: ", i + offset);
            printf("%04x ", data);
        }
        printf("\n");
    }
    break;
#endif

    case SIOCRDEEPROM:
    {

	    struct net_device *sc = ifp->if_softc;
	    struct e1000_adapter *adapter = (struct e1000_adapter *)(sc->priv);
	    struct ethtool_eeprom eeprom;
	    unsigned char bytes[512];
	    int i;
	    int offset;
	    int len;
	    int left, once;
	    long *p = data;
	    int ac;
	    char **av;
	    ac = p[0];
	    av = p[1];

	    offset = (ac <= 1)? 0 : strtoul(av[1], 0, 0);
	    len = (ac <= 2)? 512 : strtoul(av[2], 0, 0);

	    for (left = len; left; left -= once, offset += once)
	    {
		    once = (left > 512)? 512:left;
		    eeprom.offset = offset;
		    eeprom.len = once;

		    igb_get_eeprom(sc,&eeprom, bytes);

		    for (i = 0; i < once;)
		    {
			    if (i % 16 == 0) printf("\n%04x: ", i + offset);
			    printf("%02x ", bytes[i]);
			    ++i;
		    }
	    }
	    
	    printf("\n");

    }
    break;
    case SIOCWREEPROM:
    {
	    struct net_device *sc = ifp->if_softc;
	    struct e1000_adapter *adapter = (struct e1000_adapter *)(sc->priv);
	    struct ethtool_eeprom eeprom;
	    unsigned char bytes[512];
	    long *p = data;
	    int i=1;
	    int ac;
	    char **av;
	    ac = p[0];
	    av = p[1];

	    eeprom.magic = sc->pcidev.vendor | (sc->pcidev.device << 16);
	 
	    if(ac>1 && !(av[1][0] >= '0' && av[1][0] <= '9'))
	    {
		    int fd;
		    int ret;
		    int offset;
		    unsigned char buf[512];
		    fd = open(av[1],O_RDONLY);
		    if(fd<0) return -1;

		    for (offset = 0 ; ; offset += ret)
		    {
			    ret = read(fd,buf,512);
			    if (ret <= 0) break;
			    eeprom.len = ret;
			    eeprom.offset = offset;
			    igb_set_eeprom(sc, &eeprom, buf);
		    }

		    close(fd);

		    i=2;
	    }


	    if(ac>i)
	    {
		    //offset:data,data
		    int offset;
		    int data;
		    for(;i<ac;i++)
		    {
			    char *p=av[i];
			    char *nextp;
			    int offset=strtoul(p,&nextp,0);
			    while(nextp!=p)
			    {
				    p=++nextp;
				    data=strtoul(p,&nextp,0);
				    if(nextp==p)break;
				    eeprom.len = 1;
				    eeprom.offset = offset++;
				    igb_set_eeprom(sc,&eeprom, &data);
			    }
		    }
	    }

    }
    break;

    default:
        error = EINVAL;
    }

    splx(s);
    return (error);
}

struct cfattach igb_ca =
{
    sizeof(struct net_device), igb_match, igb_attach
};

struct cfdriver igb_cd =
{
    NULL, "igb", DV_IFNET
};

#if 0
static long long e1000_read_mac(struct net_device *nic)
{

    int i;
    long long mac_tmp = 0;
    struct e1000_adapter *adapter = (struct e1000_adapter *)(mynic_em->priv);

    e1000e_read_mac_addr(&adapter->hw);
    memcpy(mynic_em->dev_addr, adapter->hw.mac.addr, mynic_em->addr_len);

    for (i = 0; i < 6; i++)
    {
        mac_tmp <<= 8;
        mac_tmp |=  adapter->hw.mac.addr[i];
    }
    return mac_tmp;
}


#include <pmon.h>

unsigned short val = 0;
int cmd_setmac_em0(int ac, char *av[])
{
    int i;
    int n = 0;
    unsigned short v;
    struct net_device *nic = mynic_em ;
    struct e1000_adapter *adapter = (struct e1000_adapter *)(mynic_em->priv);

    if (nic == NULL)
    {
        printf("E1000 interface not initialized\n");
        return 0;
    }
    if (ac != 2)
    {
        long long macaddr;
        u_int8_t *paddr;
        u_int8_t enaddr[6];
        macaddr = e1000_read_mac(nic);
        paddr = (uint8_t *)&macaddr;
        enaddr[0] = paddr[5 - 0];
        enaddr[1] = paddr[5 - 1];
        enaddr[2] = paddr[5 - 2];
        enaddr[3] = paddr[5 - 3];
        enaddr[4] = paddr[5 - 4];
        enaddr[5] = paddr[5 - 5];
        printf("MAC ADDRESS ");
        for (i = 0; i < 6; i++)
        {
            printf("%02x", enaddr[i]);
            if (i == 5)
                printf("\n");
            else
                printf(":");
        }
//                printf("Use \"setmac <mac> \" to set mac address\n");
        return 0;
    }
    for (i = 0; i < 3; i++)
    {
        val = 0;
        gethex(&v, av[1], 2);
        val = v ;
        av[1] += 3;
        gethex(&v, av[1], 2);
        val = val | (v << 8);
        av[1] += 3;
        //e1000_write_eeprom(&adapter->hw,i,1,&val);
        e1000_write_nvm(&adapter->hw, i, 1, &val);
    }

    //if(e1000_update_eeprom_checksum(&adapter->hw) == 0) zxj
    if (e1000e_update_nvm_checksum(&adapter->hw) == 0)
        printf("the checksum is right!\n");
    printf("The MAC address have been written done\n");
    return 0;
}

#if 1
static unsigned long next = 1;
/* RAND_MAX assumed to be 32767 */
static int myrand(void)
{
    next = next * 1103515245 + 12345;
    return ((unsigned)(next / 65536) % 32768);
}

static void mysrand(unsigned int seed)
{
    next = seed;
}
#endif
int cmd_wrprom_em0(int ac, char **av)
{
    int i = 0;
    unsigned long clocks_num = 0;
    unsigned short eeprom_data;
    unsigned char tmp[4];
    unsigned short rom[EEPROM_CHECKSUM_REG + 1] =
    {
        /*0x1b00, 0x0821, 0x23a7, 0x0210, 0xffff, 0x1000 ,0xffff, 0xffff,
        0xc802, 0x3502, 0x640b, 0x1376, 0x8086, 0x107c, 0x8086, 0xb284,
        0x20dd, 0x5555, 0x0000, 0x2f90, 0x3200, 0x0012, 0x1e20, 0x0012,
        0x1e20, 0x0012, 0x1e20, 0x0012, 0x1e20, 0x0009, 0x0200, 0x0000,
        0x000c, 0x93a6, 0x280b, 0x0000, 0x0400, 0xffff, 0xffff, 0xffff,
        0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0x0602,
        0x0100, 0x4000, 0x1216, 0x4007, 0xffff, 0xffff, 0xffff, 0xffff,
        0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0x7dfa*/
        0x1b00, 0x3d21, 0x6602, 0x0420, 0xf746, 0x1080, 0xffff, 0xffff,
        0xe469, 0x8103, 0x026b, 0xa01f, 0x8086, 0x10d3, 0xffff, 0x9c58,
        0x0000, 0x2001, 0x7e94, 0xffff, 0x1000, 0x0048, 0x0000, 0x2704,
        0x6cc9, 0x3150, 0x073e, 0x460b, 0x2d84, 0x0140, 0xf000, 0x0706,
        0x6000, 0x7100, 0x1408, 0xffff, 0x4d01, 0x92ec, 0xfc5c, 0xf083,
        0x0028, 0x0233, 0x0050, 0x7d1f, 0x1961, 0x0453, 0x00a0, 0xffff,
        0x0100, 0x4000, 0x1315, 0x4003, 0xffff, 0xffff, 0xffff, 0xffff,
        0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0x0130, 0xffff, 0xb69e,
        /*zxj for e1000e 82574 */
    };
    struct e1000_adapter *adapter = (struct e1000_adapter *)(mynic_em->priv);

    printf("write the whole eeprom\n");

#if 1
    clocks_num = CPU_GetCOUNT();
    mysrand(clocks_num);
    for (i = 0; i < 4; i++)
    {
        tmp[i] = myrand() % 256;
        printf(" tmp[%d]=0x%2x\n", i, tmp[i]);
    }
    eeprom_data = tmp[1] | (tmp[0] << 8);
    rom[1] = eeprom_data ;
    printf("eeprom_data [1] = 0x%4x\n", eeprom_data);
    eeprom_data = tmp[3] | (tmp[2] << 8);
    rom[2] = eeprom_data;
    printf("eeprom_data [2] = 0x%4x\n", eeprom_data);
#endif
    for (i = 0; i <= EEPROM_CHECKSUM_REG; i++)
    {
        eeprom_data = rom[i];
        printf("rom[%d] = 0x%x\n", i, rom[i]);
        //e1000_write_eeprom(&adapter->hw, i, 1 , &eeprom_data) ;
        e1000_write_nvm(&adapter->hw, i, 1 , &eeprom_data) ;
    }
    //if(e1000_update_eeprom_checksum(&adapter->hw) == 0) zxj
    if (e1000e_update_nvm_checksum(&adapter->hw) == 0)
        printf("the checksum is right!\n");
    printf("The whole eeprom have been written done\n");
    return 0;
}

int cmd_reprom_em0(int ac, char *av)
{
    int i;
    unsigned short eeprom_data;
    struct e1000_adapter *adapter = (struct e1000_adapter *)(mynic_em->priv);

    printf("dump eprom:\n");

    for (i = 0; i <= EEPROM_CHECKSUM_REG;)
    {
        //if(e1000_read_eeprom(&adapter->hw, i, 1 , &eeprom_data) < 0) zxj
        if (e1000_read_nvm(&adapter->hw, i, 1 , &eeprom_data) < 0)
        {
            printf("EEPROM Read Error\n");
            //        return -E1000_ERR_EEPROM;
        }
        printf("%04x ", eeprom_data);
        ++i;
        if (i % 8 == 0)
            printf("\n");
    }
    return 0;
}

int cmd_msqt_lan(int ac, char *av[])
{
    struct e1000_adapter *adapter = (struct e1000_adapter *)(mynic_em->priv);
    struct e1000_hw *tp = &adapter->hw;

    printf("dummy!\n");
    return 0;
}

static const Optdesc netdmp_opts[] =
{
    {"<interface>", "Interface name"},
    {"<netdmp>", "IP Address"},
    {0}
};

static const Cmd Cmds[] =
{
    {"em"},
    //{"setmac_em0", "", NULL,
    {
        "setmac", "", NULL,
        "Set mac address into E1000 eeprom", cmd_setmac_em0, 1, 5, 0
    },

    //{"readrom_em0", "", NULL,
    {
        "readrom", "", NULL,
        "dump E1000 eprom content", cmd_reprom_em0, 1, 2, 0
    },

    //{"writerom_em0", "", NULL,
    {
        "writerom", "", NULL,
        "write E1000 eprom content", cmd_wrprom_em0, 1, 2, 0
    },
    {"msqt_lan", " [100M/1000M] [mode1(waveform)/mode4(distortion)]/[chana/chanb]", NULL, "Motherboard Signal Quality Test for RTL8111", cmd_msqt_lan, 3, 3, 0},

    {0, 0}
};



static void init_cmd __P((void)) __attribute__((constructor));

static void
init_cmd()
{
    cmdlist_expand(Cmds, 1);
}
#endif
