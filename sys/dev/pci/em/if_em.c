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
typedef struct FILE {
	int fd;
	int valid;
	int ungetcflag;
	int ungetchar;
} FILE;
extern FILE _iob[];
#define serialout (&_iob[1])

#ifdef __alpha__		/* XXX */
/* XXX XXX NEED REAL DMA MAPPING SUPPORT XXX XXX */
#undef vtophys
#define	vtophys(va)	alpha_XXX_dmamap((vm_offset_t)(va))
#endif /* __alpha__ */

#else /* __FreeBSD__ */

#include <sys/sockio.h>

#include <netinet/if_ether.h>

#include <vm/vm.h>		/* for vtophys */
#include <vm/vm_param.h>	/* for vtophys */
#include <vm/pmap.h>		/* for vtophys */
#include <machine/clock.h>	/* for DELAY */

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
#define	RFA_ALIGNMENT_FUDGE	4
#else
#define	RFA_ALIGNMENT_FUDGE	2
#endif

#include <linux/types.h>
//#include <linux/pci.h>
#include "e1000_pmon.c"

#include "e1000_main.c"
#include "e1000_hw.c"
#include "e1000_param.c"


static int pci_read_config_dword(struct pci_dev *linuxpd, int reg, u32 *val)
{
	if ((reg & 3) || reg < 0 || reg >= 0x100) {
        	printf ("pci_read_config_dword: bad reg %x\n", reg);
        	return -1;
    	}
	*val=_pci_conf_read(linuxpd->pa.pa_tag, reg);       
	return 0;
}

static int pci_write_config_dword(struct pci_dev *linuxpd, int reg, u32 val)
{
    if ((reg & 3) || reg < 0 || reg >= 0x100) {
	    printf ("pci_write_config_dword: bad reg %x\n", reg);
	return -1;
    }
   _pci_conf_write(linuxpd->pa.pa_tag,reg,val); 
   return 0;
}

static int pci_read_config_word(struct pci_dev *linuxpd, int reg, u16 *val)
{
	if ((reg & 1) || reg < 0 || reg >= 0x100) {
        	printf ("pci_read_config_word: bad reg %x\n", reg);
        	return -1;
    	}
	*val=_pci_conf_readn(linuxpd->pa.pa_tag,reg,2);       
	return 0;
}

static int pci_write_config_word(struct pci_dev *linuxpd, int reg, u16 val)
{
    if ((reg & 1) || reg < 0 || reg >= 0x100) {
	    printf ("pci_write_config_word: bad reg %x\n", reg);
	return -1;
    }
   _pci_conf_writen(linuxpd->pa.pa_tag,reg,val,2); 
   return 0;
}

static int pci_read_config_byte(struct pci_dev *linuxpd, int reg, u8 *val)
{
    if (reg < 0 || reg >= 0x100) {
	    printf ("pci_write_config_word: bad reg %x\n", reg);
	return -1;
    }
	*val=_pci_conf_readn(linuxpd->pa.pa_tag,reg,1);       
	return 0;
}

static int pci_write_config_byte(struct pci_dev *linuxpd, int reg, u8 val)
{
    if (reg < 0 || reg >= 0x100) {
	    printf ("pci_write_config_word: bad reg %x\n", reg);
	return -1;
    }
   _pci_conf_writen(linuxpd->pa.pa_tag,reg,val,1); 
   return 0;
}

void netdev_init(struct net_device *netdev,struct pci_attach_args *pa)
{
    unsigned short  vendor;
    unsigned short  device,class;
    unsigned short  subsystem_vendor;
    unsigned short  subsystem_device;
    unsigned int i;
	static int irq=0;

        vendor = pa->pa_id & 0xffff;
        device = (pa->pa_id >> 16) & 0xffff;
        class=(pa->pa_class >>8);
        i=pci_conf_read(0,pa->pa_tag,PCI_SUBSYSTEM_VENDOR_ID);
        subsystem_vendor=i&0xffff;
        subsystem_device=i>>16;
	netdev->pcidev.vendor=vendor;
	netdev->pcidev.device=device;
	netdev->pcidev.subsystem_vendor=subsystem_vendor;
    netdev->pcidev.subsystem_device=subsystem_device;
	netdev->pcidev.pa=*pa;
	netdev->priv=kmalloc(sizeof(struct e1000_adapter),GFP_KERNEL);//&netdev->em;
	memset(netdev->priv,0,sizeof(struct e1000_adapter));
	netdev->addr_len=6;
	netdev->pcidev.irq=irq++;
}

static int e1000_ether_ioctl(struct ifnet *ifp,FXP_IOCTLCMD_TYPE cmd,caddr_t data);

static struct pci_device_id *e1000_pci_id=0;
/*
 * Check if a device is an 82557.
 */
static void e1000_start(struct ifnet *ifp);
static int
em_match(parent, match, aux)
	struct device *parent;
#if defined(__BROKEN_INDIRECT_CONFIG) || defined(__OpenBSD__)
	void *match;
#else
	struct cfdata *match;
#endif
	void *aux;
{
	struct pci_attach_args *pa = aux;
if(getenv("noem"))return 0;
e1000_pci_id=pci_match_device(e1000_pci_tbl,pa);
return e1000_pci_id?1:0;
}

static void
e1000_shutdown(sc)
        void *sc;
{
struct e1000_adapter *adapter=((struct net_device *)sc)->priv;
        e1000_suspend(adapter->pdev, 3);
}


extern char activeif_name[];
static int em_intr(void *data)
{
struct net_device *netdev = data;
int irq=netdev->irq;
struct ifnet *ifp = &netdev->arpcom.ac_if;
	if(ifp->if_flags & IFF_RUNNING)
	{
		if(irqstate&(1<<irq))
		{
			e1000_intr(irq,data,0);
			run_task_queue(&tq_e1000);
		   if (ifp->if_snd.ifq_head != NULL)
		   e1000_start(ifp);
		}
	return 1;
	}
	return 0;
}

struct net_device *mynic_em;

static void
em_attach(parent, self, aux)
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

	mynic_em = sc;
	/*
	 * Allocate our interrupt.
	 */
	if (pci_intr_map(pc, pa->pa_intrtag, pa->pa_intrpin,
	    pa->pa_intrline, &ih)) {
		printf(": couldn't map interrupt\n");
		return;
	}
	
	intrstr = pci_intr_string(pc, ih);
#ifdef __OpenBSD__
	sc->sc_ih = pci_intr_establish(pc, ih, IPL_NET, em_intr, sc, self->dv_xname);
#else
	sc->sc_ih = pci_intr_establish(pc, ih, IPL_NET, em_intr, sc);
#endif
	
	if (sc->sc_ih == NULL) {
		printf(": couldn't establish interrupt");
		if (intrstr != NULL)
			printf(" at %s", intrstr);
		printf("\n");
		return;
	}
	
	netdev_init(sc,pa);
	/* Do generic parts of attach. */
	if (em_probe(sc,e1000_pci_id,&sc->pcidev)) {
		/* Failed! */
		return;
	}

#ifdef __OpenBSD__
	ifp = &sc->arpcom.ac_if;
	bcopy(sc->dev_addr, sc->arpcom.ac_enaddr, sc->addr_len);
#else
	ifp = &sc->sc_ethercom.ec_if;
#endif
	bcopy(sc->sc_dev.dv_xname, ifp->if_xname, IFNAMSIZ);
	
	ifp->if_softc = sc;
	ifp->if_flags = IFF_BROADCAST | IFF_SIMPLEX | IFF_MULTICAST;
	ifp->if_ioctl = e1000_ether_ioctl;
	ifp->if_start = e1000_start;
	ifp->if_watchdog = 0;

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
	shutdownhook_establish(e1000_shutdown, sc);

#ifndef PMON
	/*
	 * Add suspend hook, for similiar reasons..
	 */
	powerhook_establish(e1000_power, sc);
#endif
}


/*
 * Start packet transmission on the interface.
 */



static void e1000_start(struct ifnet *ifp)
{
	struct net_device *sc = ifp->if_softc;
	struct mbuf *mb_head;		
	struct sk_buff *skb;

	while(ifp->if_snd.ifq_head != NULL ){
		
		IF_DEQUEUE(&ifp->if_snd, mb_head);
		
		skb=dev_alloc_skb(mb_head->m_pkthdr.len);
		m_copydata(mb_head, 0, mb_head->m_pkthdr.len, skb->data);
		skb->len=mb_head->m_pkthdr.len;
		e1000_xmit_frame(skb,sc);

		m_freem(mb_head);
		wbflush();
	} 
}

static int
em_init(struct net_device *netdev)
{
    struct ifnet *ifp = &netdev->arpcom.ac_if;
	int stat=0;
    ifp->if_flags |= IFF_RUNNING;
	if(!netdev->opencount){ stat=e1000_open(netdev);netdev->opencount++;}
	e1000_tx_timeout_task(netdev);
	return stat;
}

static int
em_stop(struct net_device *netdev)
{
    struct ifnet *ifp = &netdev->arpcom.ac_if;
	ifp->if_timer = 0;
	ifp->if_flags &= ~(IFF_RUNNING | IFF_OACTIVE);
	if(netdev->opencount){e1000_close(netdev);netdev->opencount--;}
	return 0;
}

static int
e1000_ether_ioctl(ifp, cmd, data)
	struct ifnet *ifp;
	FXP_IOCTLCMD_TYPE cmd;
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
			error = em_init(sc);
			if(error <0 )
				return(error);
			ifp->if_flags |= IFF_UP;

#ifdef __OpenBSD__
			arp_ifinit(&sc->arpcom, ifa);
#else
			arp_ifinit(ifp, ifa);
#endif
			
			break;
#endif

		default:
			error = em_init(sc);
			if(error <0 )
				return(error);
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
		if (ifp->if_flags & IFF_UP) {
			error = em_init(sc);
			if(error <0 )
				return(error);
		} else {
			if (ifp->if_flags & IFF_RUNNING)
				em_stop(sc);
		}
		break;
       case SIOCETHTOOL:
       		{
       		long *p=data;
		mynic_em = sc;
       		cmd_setmac_em0(p[0],p[1]);
       		}
       		break;
       case SIOCRDEEPROM:
                {
                long *p=data;
		mynic_em = sc;
                cmd_reprom_em0(p[0],p[1]);
                }
                break;
       case SIOCWREEPROM:
                {
                long *p=data;
		mynic_em = sc;
                cmd_wrprom_em0(p[0],p[1]);
                }
                break;

	default:
		error = EINVAL;
	}

	splx(s);
	return (error);
}

struct cfattach em_ca = {
	sizeof(struct net_device), em_match, em_attach
};

struct cfdriver em_cd = {
	NULL, "em", DV_IFNET
};

//////////////////////////////////////////////////////////////////////////////////////////
//#define EEPROM_SA_OFFSET 0x10
long uIOBase=0xbfd04380;

static long long ip100a_read_mac(struct net_device *nic)
{

        int i;
        long long mac_tmp = 0;

        struct e1000_adapter *adapter = (struct e1000_adapter *) (mynic_em->priv);
        struct e1000_hw *hw = &adapter->hw;

        e1000_read_mac_addr(&adapter->hw);
        memcpy(mynic_em->dev_addr, adapter->hw.mac_addr, mynic_em->addr_len);

        for (i = 0; i < 6; i++) {
                mac_tmp <<= 8;
                mac_tmp |=  adapter->hw.mac_addr[i];
}
        return mac_tmp;
}


/////////////////////////////////////////////////////////////////////////////////////////////
#if 1
#include <pmon.h>
int cmd_ifm_em0(int ac, char *av[])
{
#if 0
        struct ip100a_private *np = mynic_em->priv;
#ifdef MY_NET
        struct net_device *nic_ifm;
#else
        struct nic *nic_ifm;
#endif
        int speed100=0, fullduplex=0, mii_ctrl = 0x0;
        long ioaddr = uIOBase ;
        int phys[4];
        np = mynic_em->priv;
        phys[0] = np->phys[0];
        nic_ifm = mynic_em;
//        printf("np->phys[0] : %d \n",phys[0]);
        if(nic_ifm  == NULL){
                printf("IP100A interface not initialized\n");
                return 0;
        }

        if(ac !=1 && ac!=2 && ac!=3){
                printf("usage: ifm_netdev [100|10|auto]  [full|half]\n");
                return 0;
        }
        if(ac == 1){
                //zgj speed10 = RTL_READ_1(nic,  MediaStatus) & MSRSpeed10;
                printf("MIICtrl : 0x%x \n",readb(ioaddr+MIICtrl));
                printf("ASIC_Ctrl : 0x%x \n",readl(ioaddr+ASICCtrl));
                speed100 = readb(ioaddr+MIICtrl) & PhySpeedStatus ;
                fullduplex = readb(ioaddr+MIICtrl) & PhyDuplexStatus ;
                printf(" %sMbps %s-DUPLEX.\n", speed100 ? "100" : "10",
                                                fullduplex ? "FULL" : "HALF");
                return 0;
        }

        if(strcmp("100", av[1]) == 0){
                mii_ctrl = 0;
                 mii_ctrl |= BMCR_SPEED100;
//              mii_ctrl =  mdio_read(nic_ifm , phys[0] , MII_BMCR);
//              printf("mii_ctrl_100 read: 0x%x\n",mii_ctrl);
                if(strcmp("full", av[2]) == 0)
                        mii_ctrl |= BMCR_FULLDPLX ;
                else
                        mii_ctrl &= ~BMCR_FULLDPLX;
                printf("mii_ctrl_100 or : 0x%x\n",mii_ctrl);
                mdio_write (nic_ifm , phys[0] , MII_BMCR, mii_ctrl);
                printf("mii_ctrl_100 write : 0x%x\n",mii_ctrl);
mdelay(10);
                mii_ctrl= mdio_read(nic_ifm, phys[0], MII_BMCR);
                printf("mii_ctrl_100 read status : 0x%x\n",mii_ctrl);

        } else if(strcmp("10", av[1]) ==0){
//                mii_ctrl =  mdio_read (nic_ifm , phys[0] , MII_BMCR);
//              printf("mii_ctrl_10 read : 0x%x\n",mii_ctrl);
                mii_ctrl = 0x0;
                mii_ctrl &= ~BMCR_SPEED100 ;
                if(strcmp("full", av[2]) == 0)
                        mii_ctrl |= BMCR_FULLDPLX ;
                else
                        mii_ctrl &= ~BMCR_FULLDPLX;
                printf("mii_ctrl_10 and : 0x%x\n",mii_ctrl);
                mdio_write (nic_ifm , phys[0] , MII_BMCR, mii_ctrl);
//              mdio_write (nic_ifm , phys[0] , MII_BMCR, mii_ctrl);
                printf("mii_ctrl_10 write : 0x%x\n",mii_ctrl);
mdelay(10);
                mii_ctrl= mdio_read(nic_ifm, phys[0], MII_BMCR);
                printf("mii_ctrl_10 read status : 0x%x\n",mii_ctrl);
        } else if(strcmp("auto", av[1])==0){
//                mii_ctrl =  mdio_read (nic_ifm , phys[0] , MII_BMCR);
//              printf("mii_ctrl_auto read : 0x%x\n",mii_ctrl);
                mii_ctrl = 0x0;
                mii_ctrl |= BMCR_ANENABLE|BMCR_ANRESTART ;
                mdio_write (nic_ifm, phys[0] , MII_BMCR, mii_ctrl);
//              mdio_write (nic_ifm , phys[0] , MII_BMCR, mii_ctrl);
                printf("mii_ctrl_auto write : 0x%x\n",mii_ctrl);
mdelay(10);
                mii_ctrl= mdio_read(nic_ifm, phys[0], MII_BMCR);
                printf("mii_ctrl_auto read status : 0x%x\n",mii_ctrl);
        }
#if 0
 else if(strcmp("full", av[1])==0){
                mii_ctrl =  mdio_read (nic_ifm , phys[0] , MII_BMCR);
                printf("mii_ctrl_full read : 0x%x\n",mii_ctrl);
                mii_ctrl |= BMCR_FULLDPLX ;
                mdio_write (nic_ifm, phys[0] , MII_BMCR, mii_ctrl);
                printf("mii_ctrl_full write : 0x%x\n",mii_ctrl);
mdelay(10);
                mii_ctrl= mdio_read(nic_ifm, phys[0], MII_BMCR);
                printf("mii_ctrl_full read status : 0x%x\n",mii_ctrl);
        } else if(strcmp("half", av[1])==0){
                mii_ctrl =  mdio_read (nic_ifm , phys[0] , MII_BMCR);
                printf("mii_ctrl_half read : 0x%x\n",mii_ctrl);
                mii_ctrl &= ~BMCR_FULLDPLX ;
                mdio_write (nic_ifm, phys[0] , MII_BMCR, mii_ctrl);
                printf("mii_ctrl_half write : 0x%x\n",mii_ctrl);
mdelay(10);
                mii_ctrl= mdio_read(nic_ifm, phys[0], MII_BMCR);
                printf("mii_ctrl_half read status : 0x%x\n",mii_ctrl);
        }
#endif
         else{
                printf("usage: ifm_netdev [100|10|auto|full|half]\n");
        }
#endif
        return 0;

}

        unsigned short val = 0;
int cmd_setmac_em0(int ac, char *av[])
{
        int i;
        unsigned short v;
        struct net_device *nic = mynic_em ;
        long ioaddr= uIOBase;
        struct e1000_adapter *adapter = (struct e1000_adapter *) (mynic_em->priv);
        struct e1000_hw *hw = &adapter->hw;


        if(nic == NULL){
               printf("E1000 interface not initialized\n");
                return 0;
        }
#if 0
        if (ac != 4) {
                printf("MAC ADDRESS ");
                for(i=0; i<6; i++){
                        printf("%02x",nic->arpcom.ac_enaddr[i]);
                        if(i==5)
                                printf("\n");
                        else
                                printf(":");
                }
                printf("Use \"setmac word1(16bit) word2 word3\"\n");
                return 0;
        }
        printf("set mac to ");
        for (i = 0; i < 3; i++) {
                val = strtoul(av[i+1],0,0);
                printf("%04x ", val);
                write_eeprom(ioaddr, 0x7 + i, val);
        }
        printf("\n");
        printf("The machine should be restarted to make the mac change to take effect!!\n");
#else
        if(ac != 2){
        long long macaddr;
        u_int8_t *paddr;
        u_int8_t enaddr[6];
        macaddr=ip100a_read_mac(nic);
        paddr=(uint8_t*)&macaddr;
        enaddr[0] = paddr[5- 0];
        enaddr[1] = paddr[5- 1];
        enaddr[2] = paddr[5- 2];
        enaddr[3] = paddr[5- 3];
        enaddr[4] = paddr[5- 4];
        enaddr[5] = paddr[5- 5];
                printf("MAC ADDRESS ");
                for(i=0; i<6; i++){
                        printf("%02x",enaddr[i]);
                        if(i==5)
                                printf("\n");
                        else
                                printf(":");
                }
                printf("Use \"setmac <mac> \" to set mac address\n");
                return 0;
        }
        for (i = 0; i < 3; i++) {
                val = 0;
printf(" av[1] = %s\n",av[1]);

                gethex(&v, av[1], 2);
printf("v= 0x%x\n",v);
                val = v ;
printf("val= 0x%x \n",val);
                av[1]+=3;

printf(" av[1] = %s \n",av[1]);

                gethex(&v, av[1], 2);
printf("val= 0x%x \n",val);
                val = val | (v << 8);
                av[1] += 3;

printf("v= %d\n",v);
printf("val = 0x%4x , i= %d \n", val, i);
        e1000_write_eeprom(&adapter->hw,i,1,&val);
printf("=========================>>>>>>>>\n");
        }

        if(e1000_update_eeprom_checksum(&adapter->hw) == 0)
                printf("the checksum is right!\n");
#endif
        return 0;
}

int cmd_wrprom_em0(int ac,char *av)
{
        int i=0;
        unsigned short eeprom_data;
        unsigned short rom[EEPROM_CHECKSUM_REG+1]={
                                0x1b00, 0x0821, 0x23a7, 0x0210, 0xffff, 0x1000 ,0xffff, 0xffff,
                                0xc802, 0x3502, 0x640b, 0x1376, 0x8086, 0x107c, 0x8086, 0xb284,
                                0x20dd, 0x5555, 0x0000, 0x2f90, 0x3200, 0x0012, 0x1e20, 0x0012,
                                0x1e20, 0x0012, 0x1e20, 0x0012, 0x1e20, 0x0009, 0x0200, 0x0000,
                                0x000c, 0x93a6, 0x280b, 0x0000, 0x0400, 0xffff, 0xffff, 0xffff,
                                0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0x0602,
                                0x0100, 0x4000, 0x1216, 0x4007, 0xffff, 0xffff, 0xffff, 0xffff,
                                0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0x7dfa
                        };
        struct e1000_adapter *adapter = (struct e1000_adapter *)(mynic_em->priv);
//      struct e1000_hw *hw = &adapter->hw;
        printf("write eprom\n");

        for(i=0; i< EEPROM_CHECKSUM_REG; i++)
        {
                eeprom_data = rom[i];
                printf("rom[%d]=0x%4x , eeprom_data : 0x%4x\n",i,rom[i], eeprom_data);
                e1000_write_eeprom(&adapter->hw, i, 1 , &eeprom_data) ;
        }

        if(e1000_update_eeprom_checksum(&adapter->hw) == 0)
                printf("the checksum is right!\n");
        return 0;
}

int cmd_reprom_em0(int ac, char *av)
{
        int i;
        unsigned short eeprom_data;
        struct e1000_adapter *adapter = (struct e1000_adapter *) (mynic_em->priv);
        struct e1000_hw *hw = &adapter->hw;
        printf("dump eprom:\n");

        for(i=0; i <= EEPROM_CHECKSUM_REG;)
        {
                if(e1000_read_eeprom(hw, i, 1 , &eeprom_data) < 0)
                {
                        printf("EEPROM Read Error\n");
                        return -E1000_ERR_EEPROM;
                }
                printf("%04x ", eeprom_data);
                ++i;
                if( i%8 == 0 )
                        printf("\n");
        }
//      printf("\n");
        return 0;
}

int dbE1000=0;
int max_interrupt_work=5;

int netdmp_cmd_em0 (int ac, char *av[])
{
        struct ifnet *ifp;
        int i;
#if 0
        ifp = &mynic_em->arpcom.ac_if;
        printf("if_snd.mb_head: %x\n", ifp->if_snd.ifq_head);
        printf("if_snd.ifq_snd.ifqlen =%d\n", ifp->if_snd.ifq_len);
        printf("MIICtrl= %x\n", RTL_READ_1(mynic_em, MIICtrl));
        printf("ASICCtrl= %x\n", RTL_READ_1(mynic_em, ASICCtrl));
        printf("ifnet address=%8x\n", ifp);
        printf("if_flags = %x\n", ifp->if_flags);
        printf("Intr =%x\n", RTL_READ_2(mynic_em, IntrStatus));
        printf("TxConfig =%x\n", RTL_READ_4(mynic_em, TxConfig));
        printf("RxConfig =%x\n", RTL_READ_4(mynic_em, RxConfig));
        printf("RxBufPtr= %x\n", RTL_READ_2(mynic_em, RxBufPtr));
        printf("RxBufAddr =%x\n", RTL_READ_2(mynic_em, RxBufAddr));
        printf("cur_rx =%x\n", cur_rx);
        printf("rx_ring: %x\n",mynic_em->rx_dma);
        printf("tx_dma: %x\n",mynic_em->tx_dma);
        printf("cur_tx =%d, dirty_tx=%d\n", cur_tx, dirty_tx);
        for (i =0; i<4; i++){
                printf("Txstatus[%d]=%x\n", i, RTL_READ_4(mynic_em, TxStatus0+i*4));
        }
#endif
        if(ac==2){

                if(strcmp(av[1], "on")==0){
                        dbE1000=1;
                }
                else if(strcmp(av[1], "off")==0){
                        dbE1000=0;
                }else {
                        int x=atoi(av[1]);
                        max_interrupt_work=x;
                }

        }
        printf("dbE1000=%d\n",dbE1000);
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
        {"em0"},
        {"netdump_em0",      "",
                        0,
                        "em1000 helper", netdmp_cmd_em0, 1, 3, 0},
        {"ifm_em0", "", NULL,
                    "Set E1000 interface mode: Usage: ifm_netdev [1000|100|10|auto] [full|half] ", cmd_ifm_em0, 1, 3, 0},
        {"setmac_em0", "", NULL,
                    "Set mac address into E1000 eeprom", cmd_setmac_em0, 1, 5, 0},
        {"reprom_em0", "", NULL,
                        "dump E1000 eprom content", cmd_reprom_em0, 1, 2, 0},
        {"wrprom_em0", "", NULL,
                        "write E1000 eprom content", cmd_wrprom_em0, 1, 2, 0},
        {0, 0}
};


static void init_cmd __P((void)) __attribute__ ((constructor));

static void
init_cmd()
{
        cmdlist_expand(Cmds, 1);
}

#endif

/////////////////////////////////////////////////////////////////////////////////////////////

