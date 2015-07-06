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

//#include "e1000_main.c"
//#include "e1000_hw.c"
//#include "e1000_param.c"
#include "82571.c"
#include "es2lan.c"
#include "lib.c"
#include "netdev.c"
#include "phy.c"
#include "e1000.h"
//#include "ethtool.c"
//#include "ich8lan.c"
//#include "param.c"

static long long e1000_read_mac(struct net_device *nic);

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
int irq=netdev->pcidev.irq;
struct ifnet *ifp = &netdev->arpcom.ac_if;
	if(ifp->if_flags & IFF_RUNNING)
	{
		if(irqstate&(1<<irq))
		{
			e1000_intr(irq,data); //zxj
			run_task_queue(&tq_e1000);
		   if (ifp->if_snd.ifq_head != NULL)
		   e1000_start(ifp);
		}
	return 1;
	}
	return 0;
}


static int __devinit
em_probe(struct net_device *netdev,
		            const struct pci_device_id *ent,struct pci_dev *pdev)
{
    struct e1000_adapter *adapter;
    static int cards_found = 0;
    unsigned long mmio_start;
    int mmio_len;
	struct e1000_hw *hw;
    int pci_using_dac;
    int i,err;
    uint16_t eeprom_data;
	u16 eeprom_apme_mask = E1000_EEPROM_APME;
	const struct e1000_info *ei = e1000_info_tbl[ent->driver_data];
	adapter = netdev->priv;
	hw = &adapter->hw;
	adapter->netdev = netdev;
	adapter->pdev = pdev;
	adapter->hw.adapter = adapter;

	mmio_start = pci_resource_start(pdev, 0);
	mmio_len = pci_resource_len(pdev, 0);

	adapter->hw.hw_addr = ioremap(mmio_start, mmio_len);
	if(!adapter->hw.hw_addr)
		goto err_ioremap;

	printf("ent->driver_data is 0x%x\n",ent->driver_data);
#if 1 //zxj added
	adapter->ei = ei;
	adapter->pba = ei->pba;
	adapter->flags = ei->flags;
	adapter->hw.mac.type = ei->mac;
	adapter->msg_enable = (1 << NETIF_MSG_DRV | NETIF_MSG_PROBE) - 1;
	memcpy(&hw->mac.ops, ei->mac_ops, sizeof(hw->mac.ops));
	memcpy(&hw->nvm.ops, ei->nvm_ops, sizeof(hw->nvm.ops));
	memcpy(&hw->phy.ops, ei->phy_ops, sizeof(hw->phy.ops));
	err = ei->get_variants(adapter);
#if 0 //zxj
	if ((adapter->flags & FLAG_IS_ICH) &&
	    (adapter->flags & FLAG_READ_ONLY_NVM))
		e1000e_write_protect_nvm_ich8lan(&adapter->hw);
#endif
	hw->mac.ops.get_bus_info(&adapter->hw);
//	cmd_reprom_em0(0,0);//zxj

	adapter->hw.phy.autoneg_wait_to_complete = 0;

	/* Copper options */
	if (adapter->hw.phy.media_type == e1000_media_type_copper) {
		adapter->hw.phy.mdix = AUTO_ALL_MODES;
		adapter->hw.phy.disable_polarity_correction = 0;
		adapter->hw.phy.ms_type = e1000_ms_hw_default;
	}

	if (e1000_check_reset_block(&adapter->hw))
		e_info("PHY reset is blocked due to SOL/IDER session.\n");

#endif	
	for(i = 1; i <= 5; i++) {
		if(pci_resource_len(pdev, i) == 0)
			continue;
		//if(pci_resource_flags(pdev, i) & IORESOURCE_IO) {
		if(i==1){ //zxj
			adapter->hw.flash_address = pci_resource_start(pdev, i);
			break;
		}
	}


	adapter->bd_number = cards_found;

	/* setup the private structure */

//	cmd_reprom_em0(0,0);//zxj
	if(e1000_sw_init(adapter))
		goto err_sw_init;
#if 0 //zxj
	if(adapter->hw.mac.type >= e1000_82543) { 
		netdev->features = NETIF_F_SG |
				   NETIF_F_HW_CSUM |
				   NETIF_F_HW_VLAN_TX |
				   NETIF_F_HW_VLAN_RX |
				   NETIF_F_HW_VLAN_FILTER;
	} else {
		netdev->features = NETIF_F_SG;
	}
#else

	netdev->features = NETIF_F_SG |
			   NETIF_F_HW_CSUM |
			   NETIF_F_HW_VLAN_TX |
			   NETIF_F_HW_VLAN_RX;

	if (adapter->flags & FLAG_HAS_HW_VLAN_FILTER)
		netdev->features |= NETIF_F_HW_VLAN_FILTER;

	netdev->features |= NETIF_F_TSO;
	netdev->features |= NETIF_F_TSO6;

	netdev->vlan_features |= NETIF_F_TSO;
	netdev->vlan_features |= NETIF_F_TSO6;
	netdev->vlan_features |= NETIF_F_HW_CSUM;
	netdev->vlan_features |= NETIF_F_SG;

#endif


	if(pci_using_dac)
		netdev->features |= NETIF_F_HIGHDMA;

	netdev->features |= NETIF_F_LLTX; //zxj
#if 1 //zxj
	for (i = 0;; i++) {
		if (e1000_validate_nvm_checksum(&adapter->hw) >= 0)
		{
			break;
		}
		if (i == 2) {
			e_err("The NVM Checksum Is Not Valid\n");
			err = -EIO;
			goto err_eeprom;
		}
	}

	e1000_eeprom_checks(adapter);
#endif
	/* copy the MAC address out of the EEPROM */

	e1000e_read_mac_addr(&adapter->hw); //zxj
	memcpy(netdev->dev_addr, adapter->hw.mac.addr, netdev->addr_len);

	if(!is_valid_ether_addr(netdev->dev_addr))
		goto err_eeprom;

	init_timer(&adapter->watchdog_timer);
	adapter->watchdog_timer.function = &e1000_watchdog;
	adapter->watchdog_timer.data = (unsigned long) adapter;

	init_timer(&adapter->phy_info_timer);
	adapter->phy_info_timer.function = &e1000_update_phy_info;
	adapter->phy_info_timer.data = (unsigned long) adapter;

	//INIT_WORK(&adapter->reset_task, e1000_reset_task);
	//INIT_WORK(&adapter->watchdog_task, e1000_watchdog_task); zxj deleted
	//INIT_WORK(&adapter->downshift_task, e1000e_downshift_workaround);
	//INIT_WORK(&adapter->update_phy_task, e1000e_update_phy_task);

	/* Initialize link parameters. User can change them with ethtool */
	adapter->hw.mac.autoneg = 1;
	adapter->fc_autoneg = 1;
	adapter->hw.fc.original_type = e1000_fc_default;
	adapter->hw.fc.type = e1000_fc_default;
	adapter->hw.phy.autoneg_advertised = 0x2f;


	/* ring size defaults */
	adapter->rx_ring->count = 256;
	adapter->tx_ring->count = 256;

	/*
	 * Initial Wake on LAN setting - If APM wake is enabled in
	 * the EEPROM, enable the ACPI Magic Packet filter
	 */
	if (adapter->flags & FLAG_APME_IN_WUC) {
		/* APME bit in EEPROM is mapped to WUC.APME */
		//eeprom_data = er32(WUC);
		eeprom_data = __er32(&adapter->hw, E1000_WUC); //zxj
		eeprom_apme_mask = E1000_WUC_APME;
	} else if (adapter->flags & FLAG_APME_IN_CTRL3) {
		if (adapter->flags & FLAG_APME_CHECK_PORT_B &&
		    (adapter->hw.bus.func == 1))
			e1000_read_nvm(&adapter->hw,
				NVM_INIT_CONTROL3_PORT_B, 1, &eeprom_data);
		else
			e1000_read_nvm(&adapter->hw,
				NVM_INIT_CONTROL3_PORT_A, 1, &eeprom_data);
	}

	/* fetch WoL from EEPROM */
	if (eeprom_data & eeprom_apme_mask)
		adapter->eeprom_wol |= E1000_WUFC_MAG;

	/*
	 * now that we have the eeprom settings, apply the special cases
	 * where the eeprom may be wrong or the board simply won't support
	 * wake on lan on a particular port
	 */
	if (!(adapter->flags & FLAG_HAS_WOL))
		adapter->eeprom_wol = 0;

	/* initialize the wol settings based on the eeprom settings */
	adapter->wol = adapter->eeprom_wol;

	/* reset the hardware with the new settings */
	e1000e_reset(adapter);

	/*
	 * If the controller has AMT, do not set DRV_LOAD until the interface
	 * is up.  For all other cases, let the f/w know that the h/w is now
	 * under the control of the driver.
	 */
	if (!(adapter->flags & FLAG_HAS_AMT))
		e1000_get_hw_control(adapter);

	/* tell the stack to leave us alone until e1000_open() is called */
	netif_carrier_off(netdev);
	//netif_tx_stop_all_queues(netdev);
	netif_stop_queue(netdev); //zxj modified

	strcpy(netdev->name, "eth%d");
#if 0 //zxj deleted
	err = register_netdev(netdev);
	if (err)
		goto err_register;
	e1000_print_device_info(adapter);
#endif
	printk("em_probe done!\n");

	return 0;

err_register:
	if (!(adapter->flags & FLAG_HAS_AMT))
		e1000_release_hw_control(adapter);
err_eeprom:
	if (!e1000_check_reset_block(&adapter->hw))
	{
		e1000_phy_hw_reset(&adapter->hw);
	}
err_hw_init:

	kfree(adapter->tx_ring);
	kfree(adapter->rx_ring);
err_sw_init:
	if (adapter->hw.flash_address){
		iounmap(adapter->hw.flash_address);
	}
err_flashmap:
	iounmap(adapter->hw.hw_addr);
err_ioremap:
	//free_netdev(netdev);
	kfree(netdev); //zxj modified
#if 0 //zxj deleted
err_alloc_etherdev:
	pci_release_selected_regions(pdev,
	                             pci_select_bars(pdev, IORESOURCE_MEM));
err_pci_reg:
err_dma:
	pci_disable_device(pdev);
#endif
	return err;

}

struct net_device *mynic_em;
static unsigned int net_device_count = 0;
extern unsigned char smbios_uuid_e1000e_mac[6];

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
	int i;

#ifdef __OpenBSD__
	//bus_space_tag_t iot = pa->pa_iot;
	//bus_addr_t iobase;
	//bus_size_t iosize;
#endif
	mynic_em = sc;
	/*
	 * Allocate our interrupt.
	 */
	if (pci_intr_map(pa->pa_pc, pa->pa_intrtag, pa->pa_intrpin,
	    pa->pa_intrline, &ih)) {
		printf(": couldn't map interrupt\n");
		return;
	}
	
	intrstr = pci_intr_string(pa->pa_pc, ih);
#ifdef __OpenBSD__
	sc->sc_ih = pci_intr_establish(pa->pa_pc, ih, IPL_NET, em_intr, sc, self->dv_xname);
#else
	sc->sc_ih = pci_intr_establish(pa->pa_pc, ih, IPL_NET, em_intr, sc);
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
		printf("em_probe failed\n");
		cmd_wrprom_em0(0,0);  //zgj		
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

	for(i = 0; i < 6; i++)
		smbios_uuid_e1000e_mac[i] = sc->arpcom.ac_enaddr[i];

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
	//e1000_tx_timeout_task(netdev);
	e1000_tx_timeout(netdev); //zxj modified
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
	case SIOCGETHERADDR:
	{
		long long val;
		char *p=data;
		mynic_em = sc;
		val =e1000_read_mac(mynic_em);
		p[5] = val>>40&0xff; 
		p[4] = val>>32&0xff; 
		p[3] = val>>24&0xff; 
		p[2] = val>>16&0xff; 
		p[1] = val>>8&0xff; 
		p[0] = val&0xff; 

	}
	break;

	case SIOCWRPHY:
	{
		long *p=data;
		int ac;
		char **av;
		int i;
        struct e1000_adapter *adapter = (struct e1000_adapter *) (sc->priv);
		mynic_em = sc;

		ac = p[0];
		av = p[1];
		if(ac>1)
		{
			//offset:data,data
			int i;
			int offset;
			int data;
			for(i=1;i<ac;i++)
			{
				char *p=av[i];
				char *nextp;
				int offset=strtoul(p,&nextp,0);
				while(nextp!=p)
				{
					p=++nextp;
					data=strtoul(p,&nextp,0);
					if(nextp==p)break;
					printf("offset=%d,data=0x%x\n",offset,data);
					e1000e_write_phy_reg_igp(&adapter->hw,offset, data);
				}
			}
		}
	}
	break;
	case SIOCRDPHY:
	{
		long *p=data;
		int ac;
		char **av;
		int i;
		unsigned data;
		int offset=0,count=32;
        struct e1000_adapter *adapter = (struct e1000_adapter *) (sc->priv);
		mynic_em = sc;
		ac = p[0];
		av = p[1];

		if(ac>1) offset = strtoul(av[1],0,0);
		if(ac>2) count = strtoul(av[2],0,0);
		
		for(i=0;i<count;i++)
		{
			data = 0;
			e1000e_read_phy_reg_igp(&adapter->hw,i+offset,&data);
			if((i&0xf)==0)printf("\n%02x: ",i+offset);
			printf("%04x ",data);
		}
		printf("\n");
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

static long long e1000_read_mac(struct net_device *nic)
{

        int i;
        long long mac_tmp = 0;
        struct e1000_adapter *adapter = (struct e1000_adapter *) (mynic_em->priv);

        e1000e_read_mac_addr(&adapter->hw); 
        memcpy(mynic_em->dev_addr, adapter->hw.mac.addr, mynic_em->addr_len);

        for (i = 0; i < 6; i++) {
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
		int n=0;
        unsigned int v;
        struct net_device *nic = mynic_em ;
        struct e1000_adapter *adapter = (struct e1000_adapter *) (mynic_em->priv);

        if(nic == NULL){
               printf("E1000 interface not initialized\n");
                return 0;
        }
        if(ac != 2){
        long long macaddr;
        u_int8_t *paddr;
        u_int8_t enaddr[6];
        macaddr=e1000_read_mac(nic);
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
//                printf("Use \"setmac <mac> \" to set mac address\n");
                return 0;
        }
        for (i = 0; i < 3; i++) {
                val = 0;
                gethex(&v, av[1], 2);
                val = v ;
                av[1]+=3;
                gethex(&v, av[1], 2);
                val = val | (v << 8);
                av[1] += 3;
        //e1000_write_eeprom(&adapter->hw,i,1,&val);
        e1000_write_nvm(&adapter->hw,i,1,&val);
        }

        //if(e1000_update_eeprom_checksum(&adapter->hw) == 0) zxj
        if(e1000e_update_nvm_checksum(&adapter->hw) == 0)
                printf("the checksum is right!\n");
        printf("The MAC address have been written done\n");
        return 0;
}

#if 1
static unsigned long next = 1;
           /* RAND_MAX assumed to be 32767 */
static int myrand(void) {
               next = next * 1103515245 + 12345;
               return((unsigned)(next/65536) % 32768);
           }

static void mysrand(unsigned int seed) {
               next = seed;
           }
#endif
int cmd_wrprom_em0(int ac,char **av)
{
        int i=0;
	unsigned long clocks_num=0;
        unsigned short eeprom_data;
        unsigned char tmp[4];
	unsigned short rom[EEPROM_CHECKSUM_REG+1]={
                                /*0x1b00, 0x0821, 0x23a7, 0x0210, 0xffff, 0x1000 ,0xffff, 0xffff,
                                0xc802, 0x3502, 0x640b, 0x1376, 0x8086, 0x107c, 0x8086, 0xb284,
                                0x20dd, 0x5555, 0x0000, 0x2f90, 0x3200, 0x0012, 0x1e20, 0x0012,
                                0x1e20, 0x0012, 0x1e20, 0x0012, 0x1e20, 0x0009, 0x0200, 0x0000,
                                0x000c, 0x93a6, 0x280b, 0x0000, 0x0400, 0xffff, 0xffff, 0xffff,
                                0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0x0602,
                                0x0100, 0x4000, 0x1216, 0x4007, 0xffff, 0xffff, 0xffff, 0xffff,
                                0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0x7dfa*/
0x1b00, 0x3d21, 0x6602, 0x0420, 0xf742, 0x1080, 0xffff, 0xffff,
0xe469, 0x8103, 0x026b, 0xa01f, 0x8086, 0x10d3, 0xffff, 0x9c58,
0x0000, 0x2001, 0x7e94, 0xffff, 0x1000, 0x0048, 0x0000, 0x2704,
0x6cc9, 0x3150, 0x073e, 0x460b, 0x2d84, 0x0140, 0xf000, 0x0702,
0x6000, 0x7100, 0x1408, 0xffff, 0x4d01, 0x92ec, 0xfc5c, 0xf083,
0x0028, 0x0233, 0x0050, 0x7d1f, 0x1961, 0x0453, 0x00a0, 0xffff,
0x0100, 0x4000, 0x1315, 0x4003, 0xffff, 0xffff, 0xffff, 0xffff,
0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0x0130, 0xffff, 0xb69e,
								/*zxj for e1000e 82574 */	
};
        struct e1000_adapter *adapter = (struct e1000_adapter *)(mynic_em->priv);

        printf("write the whole eeprom\n");

#if 1
		clocks_num =CPU_GetCOUNT();
		mysrand(clocks_num);
		for( i = 0; i < 4;i++ )
		{
			tmp[i]=myrand()%256;
			printf( " tmp[%d]=0x%2x\n", i,tmp[i]);
		}
		eeprom_data =tmp[1] |( tmp[0]<<8);
		rom[1] = eeprom_data ;
		printf("eeprom_data [1] = 0x%4x\n",eeprom_data);
		eeprom_data =tmp[3] |( tmp[2]<<8);
		rom[2] = eeprom_data;
		printf("eeprom_data [2] = 0x%4x\n",eeprom_data);
#endif
        for(i=0; i<= EEPROM_CHECKSUM_REG; i++)
        {
                eeprom_data = rom[i];
				printf("rom[%d] = 0x%x\n",i,rom[i]);
                //e1000_write_eeprom(&adapter->hw, i, 1 , &eeprom_data) ;
                e1000_write_nvm(&adapter->hw, i, 1 , &eeprom_data) ;
        }
        //if(e1000_update_eeprom_checksum(&adapter->hw) == 0) zxj
        if(e1000e_update_nvm_checksum(&adapter->hw) == 0)
                printf("the checksum is right!\n");
        printf("The whole eeprom have been written done\n");
	return 0;
}

int cmd_reprom_em0(int ac, char *av)
{
        int i;
        unsigned short eeprom_data;
        struct e1000_adapter *adapter = (struct e1000_adapter *) (mynic_em->priv);

        printf("dump eprom:\n");

        for(i=0; i <= EEPROM_CHECKSUM_REG;)
        {
                //if(e1000_read_eeprom(&adapter->hw, i, 1 , &eeprom_data) < 0) zxj
                if(e1000_read_nvm(&adapter->hw, i, 1 , &eeprom_data) < 0)
                {
                        printf("EEPROM Read Error\n");
                //        return -E1000_ERR_EEPROM;
                }
                printf("%04x ", eeprom_data);
                ++i;
                if( i%8 == 0 )
                        printf("\n");
        }
        return 0;
}

int cmd_msqt_lan(int ac, char *av[])
{
struct e1000_adapter *adapter = (struct e1000_adapter *) (mynic_em->priv);
struct e1000_hw *tp= &adapter->hw;

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
        {"setmac", "", NULL,
                    "Set mac address into E1000 eeprom", cmd_setmac_em0, 1, 5, 0},

        //{"readrom_em0", "", NULL,
        {"readrom", "", NULL,
                        "dump E1000 eprom content", cmd_reprom_em0, 1, 2, 0},

        //{"writerom_em0", "", NULL,
        {"writerom", "", NULL,
                        "write E1000 eprom content", cmd_wrprom_em0, 1, 2, 0},
	{"msqt_lan", " [100M/1000M] [mode1(waveform)/mode4(distortion)]/[chana/chanb]", NULL, "Motherboard Signal Quality Test for RTL8111", cmd_msqt_lan, 3, 3, 0},

        {0, 0}
};



static void init_cmd __P((void)) __attribute__ ((constructor));

static void
init_cmd()
{
        cmdlist_expand(Cmds, 1);
}
