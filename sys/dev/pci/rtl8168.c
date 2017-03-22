#include <stdlib.h>
#include "bpfilter.h"

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/mbuf.h>
#include <sys/malloc.h>
#include <sys/kernel.h>
#include <sys/socket.h>
#include <sys/syslog.h>

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

#if defined(__OpenBSD__)
#include <sys/ioctl.h>
#include <sys/errno.h>
#include <sys/device.h>
#include <netinet/if_ether.h>
#endif

#include <dev/mii/miivar.h>
#include <dev/mii/mii.h>

#include <dev/pci/pcivar.h>
#include <dev/pci/pcireg.h>

#include "rtl8168.h"

#define udelay(m)	delay(m)

/* Maximum events (Rx packets, etc.) to handle at each interrupt. */
static const int max_interrupt_work = 20;

/* Maximum number of multicast addresses to filter (vs. Rx-all-multicast).
 *    The RTL chips use a 64 element hash table based on the Ethernet CRC. */
static const int multicast_filter_limit = 32;

#define _R(NAME,MAC,RCR,MASK, JumFrameSz) \
	{ .name = NAME, .mcfg = MAC, .RCR_Cfg = RCR, .RxConfigMask = MASK, .jumbo_frame_sz = JumFrameSz }

static const struct {
	const char *name;
	u8 mcfg;
	u32 RCR_Cfg;
	u32 RxConfigMask;	/* Clears the bits supported by this chip */
	u32 jumbo_frame_sz;
} rtl_chip_info[] = {
	_R("RTL8168B/8111B",
	   CFG_METHOD_1,
	   (Reserved2_data << Reserved2_shift) | (RX_DMA_BURST << RxCfgDMAShift),
	   0xff7e1880,
	   Jumbo_Frame_4k),

	_R("RTL8168B/8111B",
	   CFG_METHOD_2,
	   (Reserved2_data << Reserved2_shift) | (RX_DMA_BURST << RxCfgDMAShift),
	   0xff7e1880,
	   Jumbo_Frame_4k),

	_R("RTL8168B/8111B",
	   CFG_METHOD_3,
	   (Reserved2_data << Reserved2_shift) | (RX_DMA_BURST << RxCfgDMAShift),
	   0xff7e1880,
	   Jumbo_Frame_4k),

	_R("RTL8168C/8111C",
	   CFG_METHOD_4, RxCfg_128_int_en | RxCfg_fet_multi_en | (RX_DMA_BURST << RxCfgDMAShift),
	   0xff7e1880,
	   Jumbo_Frame_6k),	

	_R("RTL8168C/8111C",
	   CFG_METHOD_5,
	   RxCfg_128_int_en | RxCfg_fet_multi_en | (RX_DMA_BURST << RxCfgDMAShift),
	   0xff7e1880,
	   Jumbo_Frame_6k),	

	_R("RTL8168C/8111C",
	   CFG_METHOD_6,
	   RxCfg_128_int_en | RxCfg_fet_multi_en | (RX_DMA_BURST << RxCfgDMAShift),
	   0xff7e1880,
	   Jumbo_Frame_6k),

	_R("RTL8168CP/8111CP",
	   CFG_METHOD_7,
	   RxCfg_128_int_en | RxCfg_fet_multi_en | (RX_DMA_BURST << RxCfgDMAShift),
	   0xff7e1880,
	   Jumbo_Frame_6k),

	_R("RTL8168CP/8111CP",
	   CFG_METHOD_8,
	   RxCfg_128_int_en | RxCfg_fet_multi_en | (RX_DMA_BURST << RxCfgDMAShift),
	   0xff7e1880,
	   Jumbo_Frame_6k),

	_R("RTL8168D/8111D",
	   CFG_METHOD_9,
	   RxCfg_128_int_en | (RX_DMA_BURST << RxCfgDMAShift),
	   0xff7e1880,
	   Jumbo_Frame_9k),

	_R("RTL8168D/8111D",
	   CFG_METHOD_10,
	   RxCfg_128_int_en | (RX_DMA_BURST << RxCfgDMAShift),
	   0xff7e1880,
	   Jumbo_Frame_9k),

	_R("RTL8168DP/8111DP",
	   CFG_METHOD_11,
	   RxCfg_128_int_en | (RX_DMA_BURST << RxCfgDMAShift),
	   0xff7e1880,
	   Jumbo_Frame_9k),
};
#undef _R

/* media options */
#define MAX_UNITS 8
static int speed[MAX_UNITS] = { -1, -1, -1, -1, -1, -1, -1, -1 };
static int duplex[MAX_UNITS] = { -1, -1, -1, -1, -1, -1, -1, -1 };
static int autoneg[MAX_UNITS] = { -1, -1, -1, -1, -1, -1, -1, -1 };

typedef struct rtl8168_private r8168;
static int r8168_match( struct device *, void *, void *);
static void r8168_attach(struct device *, struct device *, void *);
static void rtl8168_get_mac_version(struct rtl8168_private *, void *);
static void rtl8168_print_mac_version(struct rtl8168_private *);
static int rtl8168_set_speed_xmii(struct rtl8168_private *, u8, u16, u8);
static void rtl8168_phy_power_up (struct rtl8168_private *);
static void rtl8168_phy_power_down (struct rtl8168_private *);
static void mdio_write(struct rtl8168_private *, int , int );
static int mdio_read(struct rtl8168_private *, int);
static void rtl8168_gset_xmii(struct rtl8168_private * ,struct ethtool_cmd *);
static void rtl8168_xmii_reset_enable(struct rtl8168_private * );
static unsigned int rtl8168_xmii_reset_pending(struct rtl8168_private *);
static unsigned int rtl8168_xmii_link_ok(struct rtl8168_private * );
static unsigned rtl8168_try_msi(struct rtl8168_private *);
static void rtl8168_hw_phy_config(struct rtl8168_private *);
static u8 rtl8168_efuse_read(struct rtl8168_private *, u16);
static void rtl8168_link_option(int , u8 *, u16 *, u8 *);
static int rtl8168_set_speed(struct rtl8168_private *, u8 , u16 , u8);
static int rtl8168_ether_ioctl(struct ifnet *, unsigned long , caddr_t);
static void rtl8168_start_xmit(struct ifnet *);
static int rtl8168_xmit_frags(struct rtl8168_private *, struct ifnet *);
static int rtl8168_open(struct rtl8168_private *);
static void rtl8168_init_ring_indexes(struct rtl8168_private *tp);
static int rtl8168_init_ring(struct rtl8168_private *tp);
static u32 rtl8168_rx_fill(struct rtl8168_private *, u32, u32, int );
static u32 rtl8168_tx_fill(struct rtl8168_private *,u32, u32, u32);
static void rtl8168_rx_clear(struct rtl8168_private *);
static void rtl8168_tx_clear(struct rtl8168_private *);
static void rtl8168_free_tx(struct rtl8168_private *, unsigned char **, struct TxDesc * );
static void rtl8168_free_rx(struct rtl8168_private *,unsigned char *, struct RxDesc *);
static inline void rtl8168_make_unusable_by_asic(struct RxDesc *);
static inline void rtl8168_map_to_asic(struct RxDesc *, unsigned long, u32 , int);
static inline void rtl8168_mark_to_asic(struct RxDesc *, u32, int);
static inline void rtl8168_mark_as_last_descriptor(struct RxDesc *);
static void rtl8168_hw_reset(struct  rtl8168_private *);
static void rtl8168_hw_start(struct rtl8168_private *);
static void rtl8168_check_link_status(struct rtl8168_private *tp);
static void rtl8168dp_10mbps_gphy_para(struct rtl8168_private *tp);
static void rtl8168_set_rx_mode(struct rtl8168_private *tp);
static int rtl8168_csi_read(struct rtl8168_private * tp,int addr);
static void rtl8168_csi_write(struct rtl8168_private * tp,int addr,int value);
static void rtl8168_ephy_write(struct rtl8168_private *tp, int RegAddr, int value);
static u16 rtl8168_ephy_read(struct rtl8168_private * tp, int RegAddr);
int rtl8168_eri_read(struct rtl8168_private * tp, int addr, int len, int type);
int rtl8168_eri_write(struct rtl8168_private * tp, int addr, int len, int value, int type);
static int rtl8168_alloc_tx(unsigned char **tx_buffer, struct TxDesc *desc, int tx_buf_sz);
static int rtl8168_alloc_rx(unsigned char **rx_buffer, struct RxDesc *desc, int rx_buf_sz, int caller);
static void rtl8168_nic_reset(struct rtl8168_private *tp);
static void rtl8168_irq_mask_and_ack(struct rtl8168_private *tp);
static void rtl8168_dsm(struct rtl8168_private *tp, int dev_state);
static int rtl8168_interrupt(void *arg);
static void rtl8168_pcierr_interrupt(struct rtl8168_private *tp);
static int rtl8168_rx_interrupt(struct rtl8168_private *tp);
static void rtl8168_tx_interrupt(struct rtl8168_private *tp);
static int rtl8168_rx_csum(struct rtl8168_private *tp,struct RxDesc *desc);
static struct mbuf * getmbuf(struct rtl8168_private *tp);






static u16 rtl8168_intr_mask =
    SYSErr | LinkChg | RxDescUnavail | TxErr | TxOK | RxErr | RxOK;
static const u16 rtl8168_napi_event =
    RxOK | RxDescUnavail | RxFIFOOver | TxOK | TxErr;

struct cfattach rte_ca = {
	sizeof(r8168), r8168_match, r8168_attach, 
};

struct cfdriver rte_cd = {
	NULL, "rte", DV_IFNET,
};

 /* Provide Interface for BIOS Interface */
unsigned char MACAddr0[6] = {0};
unsigned char MACAddr1[6] = {0};
extern unsigned char smbios_uuid_rtl8168_mac[6];


#define PCI_VENDOR_ID_REALTEK       0x10ec

static int r8168_match(struct device *parent,void *match,void * aux)
{
	struct pci_attach_args *pa = aux;

	if(PCI_VENDOR(pa->pa_id) == PCI_VENDOR_ID_REALTEK) {
		if(PCI_PRODUCT(pa->pa_id) == 0x8168)
			return 1;
	}
	return 0;
}

static int
rtl8168_init_board(struct rtl8168_private *tp, struct pci_attach_args *pa)
{
	void *ioaddr = NULL;
	int offset, value;
	int rc = 0, i, acpi_idle_state = 0, pm_cap;
	pci_chipset_tag_t pc = pa->pa_pc;
	int tmp;
    int iobase, iosize;

	/* save power state before pci_enable_device overwrites it */
	pm_cap = pci_get_capability(pc, pa->pa_tag, PCI_CAP_PWRMGMT, &offset, &value);
	if (pm_cap) {
		u16 pwr_command;

		pwr_command = _pci_conf_read(pa->pa_tag, offset + PCI_PM_CTRL);
		acpi_idle_state = pwr_command & PCI_PM_CTRL_STATE_MASK;
	} else {
		printf("PowerManagement capability not found.\n");
	}
    //tp->cp_cmd = PCIMulRW | RxChkSum;  // by xiangy

	//pci_set_master(tp);
	tmp = _pci_conf_read(pa->pa_tag, PCI_COMMAND_STATUS_REG);

	if(!(tmp & PCI_COMMAND_MASTER_ENABLE)) {
		tmp |= PCI_COMMAND_MASTER_ENABLE;
		_pci_conf_write(pa->pa_tag, PCI_COMMAND_STATUS_REG, tmp);
	}

    //disable SERREN
	tmp = _pci_conf_read(pa->pa_tag, PCI_COMMAND_STATUS_REG);
	tmp &= ~PCI_COMMAND_SERR_ENABLE;
	_pci_conf_write(pa->pa_tag, PCI_COMMAND_STATUS_REG, tmp);

    if (pci_io_find(pc, pa->pa_tag, 0x10, &iobase, &iosize)) {
		printf(": can't find i/o space\n");
		return -1;
	}

	if (bus_space_map(pa->pa_iot, iobase, iosize, 0, &tp->sc_sh)) {
		printf(": can't map i/o space\n");
		return -1;
	}

	tp->sc_st = pa->pa_iot;
	tp->sc_pc = pc;
//	tp->cp_cmd = PCIMulRW | RxChkSum; //by xiangy

	tp->ioaddr = (unsigned long)tp->sc_sh;
#if 0
    //by liuqi
    status = RTL_R16(tp, IntrStatus);
    printf("before softreset -> %x ", status);
	/* Soft reset the chip. */
	RTL_W8(tp, ChipCmd, CmdReset);
 
    //by liuqi
    status = RTL_R16(tp, IntrStatus);
    printf("After softreset -> %x ", status);
	/* Check that the chip has finished the reset. */
	for (i = 1000; i > 0; i--) {
		if ((RTL_R8(tp, ChipCmd) & CmdReset) == 0)
			break;
		delay(100);
	}

	if (RTL_R8(tp, Config2) & 0x7) {
		RTL_W32(tp, 0x7c, 0x00ffffff);
		printf("PCI clock is 66M\n");
	}
	printf("r8168: Reset device %s \n", i ? "success": "timeout");
#endif

	/* Identify chip attached to board */
	rtl8168_get_mac_version(tp, ioaddr);

	rtl8168_print_mac_version(tp);
	for (i = sizeof(rtl_chip_info)/sizeof(rtl_chip_info[0]) - 1; i >= 0; i--) {
		if (tp->mcfg == rtl_chip_info[i].mcfg)
			break;
	}
	if (i < 0) {
		/* Unknown chip: assume array element #0, original RTL-8168 */
		printf( "unknown chip version, assuming %s\n", rtl_chip_info[0].name);
		i++;
	}
	tp->chipset = i;

	{
		int val = 0x0;
		val = 0xff & (read_eeprom(tp, 41));

		RTL_W8(tp, Cfg9346, Cfg9346_Unlock);
		//RTL_W8( tp, Config0, RTL_R8(tp, Config0) | 0x15);
		RTL_W8(tp, Config1, val);
		//RTL_W8( tp, Config3, RTL_R8(tp, Config3) | 0xA1);
		//RTL_W8( tp, Config4, RTL_R8(tp, Config4) | 0x80);
		RTL_W8(tp, Config5, (RTL_R8(tp, Config5) & PMEStatus));
		RTL_W8(tp, Cfg9346, Cfg9346_Lock);
	}
#if 0
	rtl8168_get_phy_version(tp, ioaddr);
	rtl8168_print_phy_version(tp);
#endif

	return rc;

}
struct rtl8168_private *mytp;
static struct rtl8168_private* myRTL[2] = {0, 0};
static int pn = 0;
static int cnt = 0;

static void
r8168_attach(struct device * parent, struct device * self, void *aux)
{
	struct rtl8168_private *tp = (struct rtl8168_private *)self;
	static int board_idx = -1;
	u8 autoneg, duplex;
	u16 speed;
	int i, rc;
	struct  ifnet *ifp;
	pci_intr_handle_t ih;
	const char *intrstr = NULL;
    u32 status;
	struct pci_attach_args *pa = aux;

	if (cnt >= 2)
		cnt = 0;
	pn += cnt;
	myRTL[pn] = tp;
	cnt++;

	/*
	rtl8168_config_reg();
	rtl8168_io_reg();
	*/

	tp->pa = (struct pci_attach_args *)aux;
	myRTL[pn] = tp;

	board_idx++;
    if_in_attach = 1;
    if_frequency = 0x13;
	rc = rtl8168_init_board(tp, aux);
	if (rc < 0)
		return;

    tp->set_speed = rtl8168_set_speed_xmii;
    tp->get_settings = rtl8168_gset_xmii;
    tp->phy_reset_enable = rtl8168_xmii_reset_enable;
    tp->phy_reset_pending = rtl8168_xmii_reset_pending;
    tp->link_ok = rtl8168_xmii_link_ok;

    RTL_W8(tp, Cfg9346, Cfg9346_Unlock);
    tp->features |= rtl8168_try_msi(tp);
    RTL_W8(tp, Cfg9346, Cfg9346_Lock);

	mytp = tp;

	//#define DEBUG
#ifdef DEBUG
#undef DEBUG
	/* Read eeprom content */
	{
		int data;
		for (i = 0; i < 64; i++){
			data = read_eeprom(tp, i);

			printf("%2x ", data);
			if ((i+1)>0 && (i+1)%4==0)
				printf("\n");
		}
		printf("\n");
	}

	if(pn == 0)
	{
		/* Set MAC ADDRESS */
		tp->dev_addr[0] = 0x00;
		tp->dev_addr[1] = 0x00;
		tp->dev_addr[2] = 0x22;
		tp->dev_addr[3] = 0x33;
		tp->dev_addr[4] = 0x44;
		tp->dev_addr[5] = 0x1f;
	}
	else
	{
		/* Set MAC ADDRESS */
		tp->dev_addr[0] = 0x00;
		tp->dev_addr[1] = 0x00;
		tp->dev_addr[2] = 0x22;
		tp->dev_addr[3] = 0x33;
		tp->dev_addr[4] = 0x44;
		tp->dev_addr[5] = 0x2f;
	}
#else
    /* Get MAC address by reading EEPROM */
	{
		unsigned int MAC;
		u16 data = 0;
		u16 data1 = 0;
		u16 data2 = 0;

		data = read_eeprom(tp, 7);
		tp->dev_addr[0] = 0xff & data;
		tp->dev_addr[1] = 0xff & (data >> 8);
		data1 = read_eeprom(tp, 8);
		tp->dev_addr[2] = 0xff & data1;
		tp->dev_addr[3] = 0xff & (data1 >> 8);
		data2 = read_eeprom(tp, 9);
		tp->dev_addr[4] = 0xff & data2;
		tp->dev_addr[5] = 0xff & (data2 >> 8);
 #ifdef INTERFACE_3A780E
          {
              int i = 0;
              for (; i < 6; i++) {
                  MACAddr0[i] = tp->dev_addr[i];
              }
          }
  #endif

		RTL_W8(tp, Cfg9346, Cfg9346_Unlock);
		MAC = data | data1 <<16;
		RTL_W32(tp, MAC0, MAC);
		MAC = data2;
		RTL_W32(tp, MAC4, MAC);
		RTL_W8(tp, Cfg9346, Cfg9346_Lock);
	}
#endif
		printf("MAC ADDRESS: ");
//		printf("%02x:%02x:%02x:%02x:%02x:%02x\n", tp->dev_addr[0],tp->dev_addr[1],
//				tp->dev_addr[2],tp->dev_addr[3],tp->dev_addr[4],tp->dev_addr[5]);

//   printf("MAC addr is ");
     for (i = 0; i < MAC_ADDR_LEN; i++){

         tp->dev_addr[i] = RTL_R8(tp, MAC0 + i);

         /* wgl add for BIOS Interface display macaddress */
         if (pn == 0){
             MACAddr0[i] = tp->dev_addr[i];
         }
         else if (pn == 1){
             MACAddr1[i] = tp->dev_addr[i];
         }

	 smbios_uuid_rtl8168_mac[i] = tp->dev_addr[i];
         printf("%02x", tp->dev_addr[i]);

         if ( i == 5 )
                 printf("\n");
         else
                 printf(":");
     }


#ifdef CONFIG_R8168_NAPI
    RTL_NAPI_CONFIG(dev, tp, rtl8168_poll, R8168_NAPI_WEIGHT);
#endif

#ifdef CONFIG_R8168_VLAN
    dev->features |= NETIF_F_HW_VLAN_TX | NETIF_F_HW_VLAN_RX;
    dev->vlan_rx_register = rtl8168_vlan_rx_register;
    dev->vlan_rx_kill_vid = rtl8168_vlan_rx_kill_vid;
#endif

#ifdef CONFIG_NET_POLL_CONTROLLER
    dev->poll_controller = rtl8168_netpoll;
#endif

    tp->cp_cmd |= RxChkSum;
    tp->cp_cmd |= RTL_R16(tp, CPlusCmd);
    tp->intr_mask = rtl8168_intr_mask;
    tp->mmio_addr = tp->ioaddr;
    tp->max_jumbo_frame_size = rtl_chip_info[tp->chipset].jumbo_frame_sz;
	rtl8168_hw_phy_config(tp);

#define PCI_LATENCY_TIMER 0x0d
	_pci_conf_writen(pa->pa_tag, PCI_LATENCY_TIMER, 0x40, 1);
    rtl8168_link_option(board_idx, &autoneg, &speed, &duplex);
    rtl8168_set_speed(tp, autoneg, speed, duplex);
	ifp = &tp->arpcom.ac_if;
	bcopy(tp->dev_addr, tp->arpcom.ac_enaddr, sizeof(tp->arpcom.ac_enaddr));
	bcopy(tp->sc_dev.dv_xname, ifp->if_xname, IFNAMSIZ);

	ifp->if_softc = tp;
	ifp->if_ioctl = rtl8168_ether_ioctl;
	ifp->if_start = rtl8168_start_xmit;
	ifp->if_snd.ifq_maxlen = NUM_TX_DESC - 1;

	if_attach(ifp);
	ether_ifattach(ifp);
    rc = rtl8168_open(tp);
	if(rc) {
		printf("rtl8168_open: error code %d \n", rc);
		return;
	}

    status = RTL_R16(tp, IntrStatus);
	if (pci_intr_map(pc, pa->pa_intrtag, pa->pa_intrpin, pa->pa_intrline, &ih)) {
		printf(": couldn't map interrupt\n");
		return;
	}

	intrstr = pci_intr_string(pc, ih);
    status = RTL_R16(tp, IntrStatus);
    tp->sc_ih = pci_intr_establish(pc, ih, IPL_NET, rtl8168_interrupt, tp, self->dv_xname);
#if 0
	if(tp->sc_ih == NULL){
             printf("error: could not establish interrupt !");
	      return;
	}else{
             printf("interrupt established!");
	}
#endif
    if_in_attach = 1;

	return ;
}

static void 
rtl8168_get_mac_version(struct rtl8168_private *tp,
			void *ioaddr)
{
	u32 reg,val32;
	u32 ICVerID;

	val32 = RTL_R32(tp, TxConfig);
	reg = val32 & 0x7c800000;
	ICVerID = val32 & 0x00700000;

	switch(reg) {
		case 0x30000000:
			tp->mcfg = CFG_METHOD_1;
			tp->efuse = EFUSE_NOT_SUPPORT;
			break;
		case 0x38000000:
			if(ICVerID == 0x00000000) {
				tp->mcfg = CFG_METHOD_2;
			} else if(ICVerID == 0x00500000) {
				tp->mcfg = CFG_METHOD_3;
			} else {
				tp->mcfg = CFG_METHOD_3;
			}
			tp->efuse = EFUSE_NOT_SUPPORT;
			break;
		case 0x3C000000:
			if(ICVerID == 0x00000000) {
				tp->mcfg = CFG_METHOD_4;
			} else if(ICVerID == 0x00200000) {
				tp->mcfg = CFG_METHOD_5;
			} else if(ICVerID == 0x00400000) {
				tp->mcfg = CFG_METHOD_6;
			} else {
				tp->mcfg = CFG_METHOD_6;
			}
			tp->efuse = EFUSE_NOT_SUPPORT;
			break;
		case 0x3C800000:
			if (ICVerID == 0x00100000){
				tp->mcfg = CFG_METHOD_7;
			} else if (ICVerID == 0x00300000){
				tp->mcfg = CFG_METHOD_8;
			} else {
				tp->mcfg = CFG_METHOD_8;
			}
			tp->efuse = EFUSE_NOT_SUPPORT;
			break;
		case 0x28000000:
			if(ICVerID == 0x00100000) {
				tp->mcfg = CFG_METHOD_9;
			} else if(ICVerID == 0x00300000) {
				tp->mcfg = CFG_METHOD_10;
			} else {
				tp->mcfg = CFG_METHOD_10;
			}
			tp->efuse = EFUSE_SUPPORT;
			break;
		case 0x28800000:
			tp->mcfg = CFG_METHOD_11;
			tp->efuse = EFUSE_SUPPORT;
			break;
		default:
			tp->mcfg = 0xFFFFFFFF;
			dprintk("unknown chip version (%x)\n",reg);
			break;
	}
	printf("mcfg is %x\n", tp->mcfg);
}


static void
rtl8168_print_mac_version(struct rtl8168_private *tp)
{
    int i;
    for (i = sizeof(rtl_chip_info)/sizeof(rtl_chip_info[0]) - 1; i >= 0; i--) {
        if (tp->mcfg == rtl_chip_info[i].mcfg){
            dprintk("mcfg == %s (%04d)\n", rtl_chip_info[i].name, rtl_chip_info[i].mcfg);
            return;
        }
    }

    dprintk("mac_version == Unknown\n");
}

static int 
rtl8168_set_speed_xmii(struct rtl8168_private *tp,
		       u8 autoneg, 
		       u16 speed, 
		       u8 duplex)
{
	int auto_nego = 0;
	int giga_ctrl = 0;
	int bmcr_true_force = 0;

	if ((speed != SPEED_1000) && 
	    (speed != SPEED_100) && 
	    (speed != SPEED_10)) {
		speed = SPEED_1000;
		duplex = DUPLEX_FULL;
	}
	if ((autoneg == AUTONEG_ENABLE) || (speed == SPEED_1000)) {
		/*n-way force*/
		if ((speed == SPEED_10) && (duplex == DUPLEX_HALF)) {
			auto_nego |= ADVERTISE_10HALF;
		} else if ((speed == SPEED_10) && (duplex == DUPLEX_FULL)) {
			auto_nego |= ADVERTISE_10HALF |
				     ADVERTISE_10FULL;
		} else if ((speed == SPEED_100) && (duplex == DUPLEX_HALF)) {
			auto_nego |= ADVERTISE_100HALF | 
				     ADVERTISE_10HALF | 
				     ADVERTISE_10FULL;
		} else if ((speed == SPEED_100) && (duplex == DUPLEX_FULL)) {
			auto_nego |= ADVERTISE_100HALF | 
				     ADVERTISE_100FULL |
				     ADVERTISE_10HALF | 
				     ADVERTISE_10FULL;
		} else if (speed == SPEED_1000) {
			giga_ctrl |= ADVERTISE_1000HALF | 
				     ADVERTISE_1000FULL;

			auto_nego |= ADVERTISE_100HALF | 
				     ADVERTISE_100FULL |
				     ADVERTISE_10HALF | 
				     ADVERTISE_10FULL;
		}

		//disable flow contorol
		auto_nego &= ~ADVERTISE_PAUSE_CAP;
		auto_nego &= ~ADVERTISE_PAUSE_ASYM;

		tp->phy_auto_nego_reg = auto_nego;
		tp->phy_1000_ctrl_reg = giga_ctrl;

		tp->autoneg = autoneg;
		tp->speed = speed;
		tp->duplex = duplex;

		rtl8168_phy_power_up (tp);

		delay(100);

		mdio_write(tp, 0x1f, 0x0000);
		mdio_write(tp, MII_ADVERTISE, auto_nego);
		mdio_write(tp, MII_CTRL1000, giga_ctrl);
		mdio_write(tp, MII_BMCR, BMCR_RESET | BMCR_ANENABLE | BMCR_ANRESTART);

		delay(100);
	} else {
		/*true force*/
#ifndef BMCR_SPEED100
#define BMCR_SPEED100	0x0040
#endif

#ifndef BMCR_SPEED10
#define BMCR_SPEED10	0x0000
#endif
		if ((speed == SPEED_10) && (duplex == DUPLEX_HALF)) {
			bmcr_true_force = BMCR_SPEED10;
		} else if ((speed == SPEED_10) && (duplex == DUPLEX_FULL)) {
			bmcr_true_force = BMCR_SPEED10 | BMCR_FULLDPLX;
		} else if ((speed == SPEED_100) && (duplex == DUPLEX_HALF)) {
			bmcr_true_force = BMCR_SPEED100;
		} else if ((speed == SPEED_100) && (duplex == DUPLEX_FULL)) {
			bmcr_true_force = BMCR_SPEED100 | BMCR_FULLDPLX;
		}

		mdio_write(tp, 0x1f, 0x0000);
		mdio_write(tp, MII_BMCR, bmcr_true_force);
	}

	if (tp->mcfg == CFG_METHOD_11) {
		if (speed == SPEED_10) {
			mdio_write(tp, 0x1F, 0x0000);
			mdio_write(tp, 0x10, 0x04EE);
		} else {
			mdio_write(tp, 0x1F, 0x0000);
			mdio_write(tp, 0x10, 0x01EE);
		}
	}
	return 0;
}

static void rtl8168_phy_power_up (struct rtl8168_private *tp)
{
	mdio_write(tp, 0x1F, 0x0000);
	mdio_write(tp, 0x0E, 0x0000);
	mdio_write(tp, MII_BMCR, BMCR_ANENABLE);
}

static void rtl8168_phy_power_down (struct rtl8168_private *tp)
{
	mdio_write(tp, 0x1F, 0x0000);
	mdio_write(tp, 0x0E, 0x0200);
	mdio_write(tp, MII_BMCR, BMCR_PDOWN);
}

static void mdio_write(struct rtl8168_private *tp, int RegAddr, int value)
{
	int i;
	RTL_W32(tp, PHYAR, PHYAR_Write | (RegAddr & PHYAR_Reg_Mask) << PHYAR_Reg_shift | (value & PHYAR_Data_Mask));

	for (i = 0; i < 10; i++) {
		/* Check if the RTL8168 has completed writing to the specified MII register */
		if (!(RTL_R32(tp, PHYAR) & PHYAR_Flag)) 
			break;
		delay(100);
	}
}

static int mdio_read(struct rtl8168_private * tp, int RegAddr)
{
	int i, value = -1;

	RTL_W32(tp, PHYAR, 
		PHYAR_Read | (RegAddr & PHYAR_Reg_Mask) << PHYAR_Reg_shift);

	for (i = 0; i < 10; i++) {
		/* Check if the RTL8168 has completed retrieving data from the specified MII register */
		if (RTL_R32(tp, PHYAR) & PHYAR_Flag) {
			value = (int) (RTL_R32(tp, PHYAR) & PHYAR_Data_Mask);
			break;
		}
		delay(100);
	}

	return value;
}

static void rtl8168_gset_xmii(struct rtl8168_private * tp,struct ethtool_cmd *cmd)
{
	u8 status;
	cmd->supported = SUPPORTED_10baseT_Half |
			 SUPPORTED_10baseT_Full |
			 SUPPORTED_100baseT_Half |
			 SUPPORTED_100baseT_Full |
			 SUPPORTED_1000baseT_Full |
			 SUPPORTED_Autoneg |
		         SUPPORTED_TP;

	cmd->autoneg = (mdio_read(tp, MII_BMCR) & BMCR_ANENABLE) ? 1 : 0;
	cmd->advertising = ADVERTISED_TP | ADVERTISED_Autoneg;

	if (tp->phy_auto_nego_reg & ADVERTISE_10HALF)
		cmd->advertising |= ADVERTISED_10baseT_Half;
	if (tp->phy_auto_nego_reg & ADVERTISE_10FULL)
		cmd->advertising |= ADVERTISED_10baseT_Full;
	if (tp->phy_auto_nego_reg & ADVERTISE_100HALF)
		cmd->advertising |= ADVERTISED_100baseT_Half;
	if (tp->phy_auto_nego_reg & ADVERTISE_100FULL)
		cmd->advertising |= ADVERTISED_100baseT_Full;
	if (tp->phy_1000_ctrl_reg & ADVERTISE_1000FULL)
		cmd->advertising |= ADVERTISED_1000baseT_Full;

	status = RTL_R8(tp, PHYstatus);

	if (status & _1000bpsF)
		cmd->speed = SPEED_1000;
	else if (status & _100bps)
		cmd->speed = SPEED_100;
	else if (status & _10bps)
		cmd->speed = SPEED_10;

	if (status & TxFlowCtrl)
		cmd->advertising |= ADVERTISED_Asym_Pause;

	if (status & RxFlowCtrl)
		cmd->advertising |= ADVERTISED_Pause;

	cmd->duplex = ((status & _1000bpsF) || (status & FullDup)) ?
		      DUPLEX_FULL : DUPLEX_HALF;

	tp->autoneg = cmd->autoneg; 
	tp->speed = cmd->speed; 
	tp->duplex = cmd->duplex; 
}

static void 
rtl8168_xmii_reset_enable(struct rtl8168_private * tp)
{
	int i;

	mdio_write(tp, 0x1f, 0x0000);
	mdio_write(tp, MII_BMCR, mdio_read(tp, MII_BMCR) | BMCR_RESET);

	for(i = 0; i < 2500; i++) {
		if(!(mdio_read(tp, MII_BMSR) & BMCR_RESET))
			return;

		delay(100);
	}
}

static unsigned int 
rtl8168_xmii_reset_pending(struct rtl8168_private *tp)
{
	unsigned int retval;

	mdio_write(tp, 0x1f, 0x0000);
	retval = mdio_read(tp, MII_BMCR) & BMCR_RESET;

	return retval;
}

static unsigned int 
rtl8168_xmii_link_ok(struct rtl8168_private * tp)
{
	mdio_write(tp, 0x1f, 0x0000);

	return RTL_R8(tp, PHYstatus) & LinkStatus;
}

/* Cfg9346_Unlock assumed. */
static unsigned rtl8168_try_msi(struct rtl8168_private *tp)
{
	return 0;	
}

static void rtl8168_hw_phy_config(struct rtl8168_private *tp)
{
	unsigned int gphy_val;

	if (tp->mcfg == CFG_METHOD_1) {
		mdio_write(tp, 0x1F, 0x0001);
		mdio_write(tp, 0x0B, 0x94B0);

		mdio_write(tp, 0x1F, 0x0003);
		mdio_write(tp, 0x12, 0x6096);
		mdio_write(tp, 0x1F, 0x0000);

		mdio_write(tp, 0x0D, 0xF8A0);
	} else if (tp->mcfg == CFG_METHOD_2) {
		mdio_write(tp, 0x1F, 0x0001);
		mdio_write(tp, 0x0B, 0x94B0);

		mdio_write(tp, 0x1F, 0x0003);
		mdio_write(tp, 0x12, 0x6096);

		mdio_write(tp, 0x1F, 0x0000);
	} else if (tp->mcfg == CFG_METHOD_3) {
		mdio_write(tp, 0x1F, 0x0001);
		mdio_write(tp, 0x0B, 0x94B0);

		mdio_write(tp, 0x1F, 0x0003);
		mdio_write(tp, 0x12, 0x6096);

		mdio_write(tp, 0x1F, 0x0000);
	} else if (tp->mcfg == CFG_METHOD_4) {
		mdio_write(tp, 0x1F, 0x0001);
		mdio_write(tp, 0x12, 0x2300);
		mdio_write(tp, 0x1F, 0x0000);
		mdio_write(tp, 0x1F, 0x0003);
		mdio_write(tp, 0x16, 0x000A);
		mdio_write(tp, 0x1F, 0x0000);

		mdio_write(tp, 0x1F, 0x0003);
		mdio_write(tp, 0x12, 0xC096);
		mdio_write(tp, 0x1F, 0x0000);

		mdio_write(tp, 0x1F, 0x0002);
		mdio_write(tp, 0x00, 0x88DE);
		mdio_write(tp, 0x01, 0x82B1);
		mdio_write(tp, 0x1F, 0x0000);
	
		mdio_write(tp, 0x1F, 0x0002);
		mdio_write(tp, 0x08, 0x9E30);
		mdio_write(tp, 0x09, 0x01F0);
		mdio_write(tp, 0x1F, 0x0000);

		mdio_write(tp, 0x1F, 0x0002);
		mdio_write(tp, 0x0A, 0x5500);
		mdio_write(tp, 0x1F, 0x0000);

		mdio_write(tp, 0x1F, 0x0002);
		mdio_write(tp, 0x03, 0x7002);
		mdio_write(tp, 0x1F, 0x0000);

		mdio_write(tp, 0x1F, 0x0002);
		mdio_write(tp, 0x0C, 0x00C8);
		mdio_write(tp, 0x1F, 0x0000);

		mdio_write(tp, 0x1F, 0x0000);
		mdio_write(tp, 0x14, mdio_read(tp, 0x14) | (1 << 5));
		mdio_write(tp, 0x0D, mdio_read(tp, 0x0D) & ~(1 << 5));
	} else if (tp->mcfg == CFG_METHOD_5) {
		mdio_write(tp, 0x1F, 0x0001);
		mdio_write(tp, 0x12, 0x2300);
		mdio_write(tp, 0x1F, 0x0003);
		mdio_write(tp, 0x16, 0x0F0A);
		mdio_write(tp, 0x1F, 0x0000);

		mdio_write(tp, 0x1F, 0x0002);
		mdio_write(tp, 0x00, 0x88DE);
		mdio_write(tp, 0x01, 0x82B1);
		mdio_write(tp, 0x1F, 0x0000);

		mdio_write(tp, 0x1F, 0x0002);
		mdio_write(tp, 0x0C, 0x7EB8);
		mdio_write(tp, 0x1F, 0x0000);

		mdio_write(tp, 0x1F, 0x0002);
		mdio_write(tp, 0x06, 0x0761);
		mdio_write(tp, 0x1F, 0x0000);

		mdio_write(tp, 0x1F, 0x0001);
		mdio_write(tp, 0x03, 0x802F);
		mdio_write(tp, 0x02, 0x4F02);
		mdio_write(tp, 0x01, 0x0409);
		mdio_write(tp, 0x00, 0xF099);
		mdio_write(tp, 0x04, 0x9800);
		mdio_write(tp, 0x04, 0x9000);
		mdio_write(tp, 0x1F, 0x0000);

		mdio_write(tp, 0x1F, 0x0000);
		mdio_write(tp, 0x16, mdio_read(tp, 0x16) | (1 << 0));

		mdio_write(tp, 0x1F, 0x0000);
		mdio_write(tp, 0x14, mdio_read(tp, 0x14) | (1 << 5));
		mdio_write(tp, 0x0D, mdio_read(tp, 0x0D) & ~(1 << 5));

		mdio_write(tp, 0x1F, 0x0001);
		mdio_write(tp, 0x1D, 0x3D98);
		mdio_write(tp, 0x1F, 0x0000);

		mdio_write(tp, 0x1F, 0x0001);
		mdio_write(tp, 0x17, 0x0CC0);
		mdio_write(tp, 0x1F, 0x0000);
	} else if (tp->mcfg == CFG_METHOD_6) {
		mdio_write(tp, 0x1F, 0x0001);
		mdio_write(tp, 0x12, 0x2300);
		mdio_write(tp, 0x1F, 0x0003);
		mdio_write(tp, 0x16, 0x0F0A);
		mdio_write(tp, 0x1F, 0x0000);

		mdio_write(tp, 0x1F, 0x0002);
		mdio_write(tp, 0x00, 0x88DE);
		mdio_write(tp, 0x01, 0x82B1);
		mdio_write(tp, 0x1F, 0x0000);

		mdio_write(tp, 0x1F, 0x0002);
		mdio_write(tp, 0x0C, 0x7EB8);
		mdio_write(tp, 0x1F, 0x0000);

		mdio_write(tp, 0x1F, 0x0002);
		mdio_write(tp, 0x06, 0x0761);
		mdio_write(tp, 0x1F, 0x0000);

		mdio_write(tp, 0x1F, 0x0000);
		mdio_write(tp, 0x16, mdio_read(tp, 0x16) | (1 << 0));

		mdio_write(tp, 0x1F, 0x0000);
		mdio_write(tp, 0x14, mdio_read(tp, 0x14) | (1 << 5));
		mdio_write(tp, 0x0D, mdio_read(tp, 0x0D) & ~(1 << 5));

		mdio_write(tp, 0x1F, 0x0001);
		mdio_write(tp, 0x1D, 0x3D98);
		mdio_write(tp, 0x1F, 0x0000);

		mdio_write(tp, 0x1f, 0x0001);
		mdio_write(tp, 0x17, 0x0CC0);
		mdio_write(tp, 0x1F, 0x0000);
	} else if (tp->mcfg == CFG_METHOD_7) {
		mdio_write(tp, 0x1F, 0x0000);
		mdio_write(tp, 0x14, mdio_read(tp, 0x14) | (1 << 5));
		mdio_write(tp, 0x0D, mdio_read(tp, 0x0D) & ~(1 << 5));

		mdio_write(tp, 0x1F, 0x0001);
		mdio_write(tp, 0x1D, 0x3D98);

		mdio_write(tp, 0x1F, 0x0001);
		mdio_write(tp, 0x14, 0xCAA3);
		mdio_write(tp, 0x1C, 0x000A);
		mdio_write(tp, 0x18, 0x65D0);
		
		mdio_write(tp, 0x1F, 0x0003);
		mdio_write(tp, 0x17, 0xB580);
		mdio_write(tp, 0x18, 0xFF54);
		mdio_write(tp, 0x19, 0x3954);

		mdio_write(tp, 0x1F, 0x0002);
		mdio_write(tp, 0x0D, 0x310C);
		mdio_write(tp, 0x0E, 0x310C);
		mdio_write(tp, 0x0F, 0x311C);
		mdio_write(tp, 0x06, 0x0761);

		mdio_write(tp, 0x1F, 0x0003);
		mdio_write(tp, 0x18, 0xFF55);
		mdio_write(tp, 0x19, 0x3955);
		mdio_write(tp, 0x18, 0xFF54);
		mdio_write(tp, 0x19, 0x3954);

		mdio_write(tp, 0x1F, 0x0001);
		mdio_write(tp, 0x17, 0x0CC0);

		mdio_write(tp, 0x1F, 0x0000);
	} else if (tp->mcfg == CFG_METHOD_8) {
		mdio_write(tp, 0x1F, 0x0000);
		mdio_write(tp, 0x14, mdio_read(tp, 0x14) | (1 << 5));
		mdio_write(tp, 0x0D, mdio_read(tp, 0x0D) & ~(1 << 5));

		mdio_write(tp, 0x1F, 0x0001);
		mdio_write(tp, 0x14, 0xCAA3);
		mdio_write(tp, 0x1C, 0x000A);
		mdio_write(tp, 0x18, 0x65D0);
		
		mdio_write(tp, 0x1F, 0x0003);
		mdio_write(tp, 0x17, 0xB580);
		mdio_write(tp, 0x18, 0xFF54);
		mdio_write(tp, 0x19, 0x3954);

		mdio_write(tp, 0x1F, 0x0002);
		mdio_write(tp, 0x0D, 0x310C);
		mdio_write(tp, 0x0E, 0x310C);
		mdio_write(tp, 0x0F, 0x311C);
		mdio_write(tp, 0x06, 0x0761);

		mdio_write(tp, 0x1F, 0x0003);
		mdio_write(tp, 0x18, 0xFF55);
		mdio_write(tp, 0x19, 0x3955);
		mdio_write(tp, 0x18, 0xFF54);
		mdio_write(tp, 0x19, 0x3954);

		mdio_write(tp, 0x1F, 0x0001);
		mdio_write(tp, 0x17, 0x0CC0);

		mdio_write(tp, 0x1F, 0x0000);
		mdio_write(tp, 0x16, mdio_read(tp, 0x16) | (1 << 0));

		mdio_write(tp, 0x1F, 0x0000);
	} else if (tp->mcfg == CFG_METHOD_9) {
		mdio_write(tp, 0x1F, 0x0001);
		mdio_write(tp, 0x06, 0x4064);
		mdio_write(tp, 0x07, 0x2863);
		mdio_write(tp, 0x08, 0x059C);
		mdio_write(tp, 0x09, 0x26B4);
		mdio_write(tp, 0x0A, 0x6A19);
		mdio_write(tp, 0x0B, 0xDCC8);
		mdio_write(tp, 0x10, 0xF06D);
		mdio_write(tp, 0x14, 0x7F68);
		mdio_write(tp, 0x18, 0x7FD9);
		mdio_write(tp, 0x1C, 0xF0FF);
		mdio_write(tp, 0x1D, 0x3D9C);
		mdio_write(tp, 0x1F, 0x0003);
		mdio_write(tp, 0x12, 0xF49F);
		mdio_write(tp, 0x13, 0x070B);
		mdio_write(tp, 0x1A, 0x05AD);
		mdio_write(tp, 0x14, 0x94C0);

		mdio_write(tp, 0x1F, 0x0002);
		gphy_val = mdio_read(tp, 0x0B) & 0xFF00;
		gphy_val |= 0x10;
		mdio_write(tp, 0x0B, gphy_val);
		gphy_val = mdio_read(tp, 0x0C) & 0x00FF;
		gphy_val |= 0xA200;
		mdio_write(tp, 0x0C, gphy_val);

		mdio_write(tp, 0x1F, 0x0002);
		mdio_write(tp, 0x06, 0x5561);
		mdio_write(tp, 0x1F, 0x0005);
		mdio_write(tp, 0x05, 0x8332);
		mdio_write(tp, 0x06, 0x5561);
		
		if (rtl8168_efuse_read(tp, 0x01) == 0xb1) {
			mdio_write(tp, 0x1F, 0x0002);
			mdio_write(tp, 0x05, 0x669A);
			mdio_write(tp, 0x1F, 0x0005);
			mdio_write(tp, 0x05, 0x8330);
			mdio_write(tp, 0x06, 0x669A);

			mdio_write(tp, 0x1F, 0x0002);
			gphy_val = mdio_read(tp, 0x0D);
			if ((gphy_val & 0x00FF) != 0x006C) {
				gphy_val &= 0xFF00;
				mdio_write(tp, 0x1F, 0x0002);
				mdio_write(tp, 0x0D, gphy_val | 0x0065);
				mdio_write(tp, 0x0D, gphy_val | 0x0066);
				mdio_write(tp, 0x0D, gphy_val | 0x0067);
				mdio_write(tp, 0x0D, gphy_val | 0x0068);
				mdio_write(tp, 0x0D, gphy_val | 0x0069);
				mdio_write(tp, 0x0D, gphy_val | 0x006A);
				mdio_write(tp, 0x0D, gphy_val | 0x006B);
				mdio_write(tp, 0x0D, gphy_val | 0x006C);
			}
		} else {
			mdio_write(tp, 0x1F, 0x0002);
			mdio_write(tp, 0x05, 0x6662);
			mdio_write(tp, 0x1F, 0x0005);
			mdio_write(tp, 0x05, 0x8330);
			mdio_write(tp, 0x06, 0x6662);
		}

		mdio_write(tp, 0x1F, 0x0002);
		gphy_val = mdio_read(tp, 0x0D);
		gphy_val |= BIT_9;
		gphy_val |= BIT_8;
		mdio_write(tp, 0x0D, gphy_val);
		gphy_val = mdio_read(tp, 0x0F);
		gphy_val |= BIT_4;
		mdio_write(tp, 0x0F, gphy_val);

		mdio_write(tp, 0x1F, 0x0002);
		gphy_val = mdio_read(tp, 0x02);
		gphy_val &= ~BIT_10;
		gphy_val &= ~BIT_9;
		gphy_val |= BIT_8;
		mdio_write(tp, 0x02, gphy_val);
		gphy_val = mdio_read(tp, 0x03);
		gphy_val &= ~BIT_15;
		gphy_val &= ~BIT_14;
		gphy_val &= ~BIT_13;
		mdio_write(tp, 0x03, gphy_val);

		mdio_write(tp, 0x1F, 0x0005);
		mdio_write(tp, 0x05, 0xFFC2);
		mdio_write(tp, 0x1F, 0x0005);
		mdio_write(tp, 0x05, 0x8000);
		mdio_write(tp, 0x06, 0xF8F9);
		mdio_write(tp, 0x06, 0xFAEF);
		mdio_write(tp, 0x06, 0x59EE);
		mdio_write(tp, 0x06, 0xF8EA);
		mdio_write(tp, 0x06, 0x00EE);
		mdio_write(tp, 0x06, 0xF8EB);
		mdio_write(tp, 0x06, 0x00E0);
		mdio_write(tp, 0x06, 0xF87C);
		mdio_write(tp, 0x06, 0xE1F8);
		mdio_write(tp, 0x06, 0x7D59);
		mdio_write(tp, 0x06, 0x0FEF);
		mdio_write(tp, 0x06, 0x0139);
		mdio_write(tp, 0x06, 0x029E);
		mdio_write(tp, 0x06, 0x06EF);
		mdio_write(tp, 0x06, 0x1039);
		mdio_write(tp, 0x06, 0x089F);
		mdio_write(tp, 0x06, 0x2AEE);
		mdio_write(tp, 0x06, 0xF8EA);
		mdio_write(tp, 0x06, 0x00EE);
		mdio_write(tp, 0x06, 0xF8EB);
		mdio_write(tp, 0x06, 0x01E0);
		mdio_write(tp, 0x06, 0xF87C);
		mdio_write(tp, 0x06, 0xE1F8);
		mdio_write(tp, 0x06, 0x7D58);
		mdio_write(tp, 0x06, 0x409E);
		mdio_write(tp, 0x06, 0x0F39);
		mdio_write(tp, 0x06, 0x46AA);
		mdio_write(tp, 0x06, 0x0BBF);
		mdio_write(tp, 0x06, 0x8290);
		mdio_write(tp, 0x06, 0xD682);
		mdio_write(tp, 0x06, 0x9802);
		mdio_write(tp, 0x06, 0x014F);
		mdio_write(tp, 0x06, 0xAE09);
		mdio_write(tp, 0x06, 0xBF82);
		mdio_write(tp, 0x06, 0x98D6);
		mdio_write(tp, 0x06, 0x82A0);
		mdio_write(tp, 0x06, 0x0201);
		mdio_write(tp, 0x06, 0x4FEF);
		mdio_write(tp, 0x06, 0x95FE);
		mdio_write(tp, 0x06, 0xFDFC);
		mdio_write(tp, 0x06, 0x05F8);
		mdio_write(tp, 0x06, 0xF9FA);
		mdio_write(tp, 0x06, 0xEEF8);
		mdio_write(tp, 0x06, 0xEA00);
		mdio_write(tp, 0x06, 0xEEF8);
		mdio_write(tp, 0x06, 0xEB00);
		mdio_write(tp, 0x06, 0xE2F8);
		mdio_write(tp, 0x06, 0x7CE3);
		mdio_write(tp, 0x06, 0xF87D);
		mdio_write(tp, 0x06, 0xA511);
		mdio_write(tp, 0x06, 0x1112);
		mdio_write(tp, 0x06, 0xD240);
		mdio_write(tp, 0x06, 0xD644);
		mdio_write(tp, 0x06, 0x4402);
		mdio_write(tp, 0x06, 0x8217);
		mdio_write(tp, 0x06, 0xD2A0);
		mdio_write(tp, 0x06, 0xD6AA);
		mdio_write(tp, 0x06, 0xAA02);
		mdio_write(tp, 0x06, 0x8217);
		mdio_write(tp, 0x06, 0xAE0F);
		mdio_write(tp, 0x06, 0xA544);
		mdio_write(tp, 0x06, 0x4402);
		mdio_write(tp, 0x06, 0xAE4D);
		mdio_write(tp, 0x06, 0xA5AA);
		mdio_write(tp, 0x06, 0xAA02);
		mdio_write(tp, 0x06, 0xAE47);
		mdio_write(tp, 0x06, 0xAF82);
		mdio_write(tp, 0x06, 0x13EE);
		mdio_write(tp, 0x06, 0x834E);
		mdio_write(tp, 0x06, 0x00EE);
		mdio_write(tp, 0x06, 0x834D);
		mdio_write(tp, 0x06, 0x0FEE);
		mdio_write(tp, 0x06, 0x834C);
		mdio_write(tp, 0x06, 0x0FEE);
		mdio_write(tp, 0x06, 0x834F);
		mdio_write(tp, 0x06, 0x00EE);
		mdio_write(tp, 0x06, 0x8351);
		mdio_write(tp, 0x06, 0x00EE);
		mdio_write(tp, 0x06, 0x834A);
		mdio_write(tp, 0x06, 0xFFEE);
		mdio_write(tp, 0x06, 0x834B);
		mdio_write(tp, 0x06, 0xFFE0);
		mdio_write(tp, 0x06, 0x8330);
		mdio_write(tp, 0x06, 0xE183);
		mdio_write(tp, 0x06, 0x3158);
		mdio_write(tp, 0x06, 0xFEE4);
		mdio_write(tp, 0x06, 0xF88A);
		mdio_write(tp, 0x06, 0xE5F8);
		mdio_write(tp, 0x06, 0x8BE0);
		mdio_write(tp, 0x06, 0x8332);
		mdio_write(tp, 0x06, 0xE183);
		mdio_write(tp, 0x06, 0x3359);
		mdio_write(tp, 0x06, 0x0FE2);
		mdio_write(tp, 0x06, 0x834D);
		mdio_write(tp, 0x06, 0x0C24);
		mdio_write(tp, 0x06, 0x5AF0);
		mdio_write(tp, 0x06, 0x1E12);
		mdio_write(tp, 0x06, 0xE4F8);
		mdio_write(tp, 0x06, 0x8CE5);
		mdio_write(tp, 0x06, 0xF88D);
		mdio_write(tp, 0x06, 0xAF82);
		mdio_write(tp, 0x06, 0x13E0);
		mdio_write(tp, 0x06, 0x834F);
		mdio_write(tp, 0x06, 0x10E4);
		mdio_write(tp, 0x06, 0x834F);
		mdio_write(tp, 0x06, 0xE083);
		mdio_write(tp, 0x06, 0x4E78);
		mdio_write(tp, 0x06, 0x009F);
		mdio_write(tp, 0x06, 0x0AE0);
		mdio_write(tp, 0x06, 0x834F);
		mdio_write(tp, 0x06, 0xA010);
		mdio_write(tp, 0x06, 0xA5EE);
		mdio_write(tp, 0x06, 0x834E);
		mdio_write(tp, 0x06, 0x01E0);
		mdio_write(tp, 0x06, 0x834E);
		mdio_write(tp, 0x06, 0x7805);
		mdio_write(tp, 0x06, 0x9E9A);
		mdio_write(tp, 0x06, 0xE083);
		mdio_write(tp, 0x06, 0x4E78);
		mdio_write(tp, 0x06, 0x049E);
		mdio_write(tp, 0x06, 0x10E0);
		mdio_write(tp, 0x06, 0x834E);
		mdio_write(tp, 0x06, 0x7803);
		mdio_write(tp, 0x06, 0x9E0F);
		mdio_write(tp, 0x06, 0xE083);
		mdio_write(tp, 0x06, 0x4E78);
		mdio_write(tp, 0x06, 0x019E);
		mdio_write(tp, 0x06, 0x05AE);
		mdio_write(tp, 0x06, 0x0CAF);
		mdio_write(tp, 0x06, 0x81F8);
		mdio_write(tp, 0x06, 0xAF81);
		mdio_write(tp, 0x06, 0xA3AF);
		mdio_write(tp, 0x06, 0x81DC);
		mdio_write(tp, 0x06, 0xAF82);
		mdio_write(tp, 0x06, 0x13EE);
		mdio_write(tp, 0x06, 0x8348);
		mdio_write(tp, 0x06, 0x00EE);
		mdio_write(tp, 0x06, 0x8349);
		mdio_write(tp, 0x06, 0x00E0);
		mdio_write(tp, 0x06, 0x8351);
		mdio_write(tp, 0x06, 0x10E4);
		mdio_write(tp, 0x06, 0x8351);
		mdio_write(tp, 0x06, 0x5801);
		mdio_write(tp, 0x06, 0x9FEA);
		mdio_write(tp, 0x06, 0xD000);
		mdio_write(tp, 0x06, 0xD180);
		mdio_write(tp, 0x06, 0x1F66);
		mdio_write(tp, 0x06, 0xE2F8);
		mdio_write(tp, 0x06, 0xEAE3);
		mdio_write(tp, 0x06, 0xF8EB);
		mdio_write(tp, 0x06, 0x5AF8);
		mdio_write(tp, 0x06, 0x1E20);
		mdio_write(tp, 0x06, 0xE6F8);
		mdio_write(tp, 0x06, 0xEAE5);
		mdio_write(tp, 0x06, 0xF8EB);
		mdio_write(tp, 0x06, 0xD302);
		mdio_write(tp, 0x06, 0xB3FE);
		mdio_write(tp, 0x06, 0xE2F8);
		mdio_write(tp, 0x06, 0x7CEF);
		mdio_write(tp, 0x06, 0x325B);
		mdio_write(tp, 0x06, 0x80E3);
		mdio_write(tp, 0x06, 0xF87D);
		mdio_write(tp, 0x06, 0x9E03);
		mdio_write(tp, 0x06, 0x7DFF);
		mdio_write(tp, 0x06, 0xFF0D);
		mdio_write(tp, 0x06, 0x581C);
		mdio_write(tp, 0x06, 0x551A);
		mdio_write(tp, 0x06, 0x6511);
		mdio_write(tp, 0x06, 0xA190);
		mdio_write(tp, 0x06, 0xD3E2);
		mdio_write(tp, 0x06, 0x8348);
		mdio_write(tp, 0x06, 0xE383);
		mdio_write(tp, 0x06, 0x491B);
		mdio_write(tp, 0x06, 0x56AB);
		mdio_write(tp, 0x06, 0x08EF);
		mdio_write(tp, 0x06, 0x56E6);
		mdio_write(tp, 0x06, 0x8348);
		mdio_write(tp, 0x06, 0xE783);
		mdio_write(tp, 0x06, 0x4910);
		mdio_write(tp, 0x06, 0xD180);
		mdio_write(tp, 0x06, 0x1F66);
		mdio_write(tp, 0x06, 0xA004);
		mdio_write(tp, 0x06, 0xB9E2);
		mdio_write(tp, 0x06, 0x8348);
		mdio_write(tp, 0x06, 0xE383);
		mdio_write(tp, 0x06, 0x49EF);
		mdio_write(tp, 0x06, 0x65E2);
		mdio_write(tp, 0x06, 0x834A);
		mdio_write(tp, 0x06, 0xE383);
		mdio_write(tp, 0x06, 0x4B1B);
		mdio_write(tp, 0x06, 0x56AA);
		mdio_write(tp, 0x06, 0x0EEF);
		mdio_write(tp, 0x06, 0x56E6);
		mdio_write(tp, 0x06, 0x834A);
		mdio_write(tp, 0x06, 0xE783);
		mdio_write(tp, 0x06, 0x4BE2);
		mdio_write(tp, 0x06, 0x834D);
		mdio_write(tp, 0x06, 0xE683);
		mdio_write(tp, 0x06, 0x4CE0);
		mdio_write(tp, 0x06, 0x834D);
		mdio_write(tp, 0x06, 0xA000);
		mdio_write(tp, 0x06, 0x0CAF);
		mdio_write(tp, 0x06, 0x81DC);
		mdio_write(tp, 0x06, 0xE083);
		mdio_write(tp, 0x06, 0x4D10);
		mdio_write(tp, 0x06, 0xE483);
		mdio_write(tp, 0x06, 0x4DAE);
		mdio_write(tp, 0x06, 0x0480);
		mdio_write(tp, 0x06, 0xE483);
		mdio_write(tp, 0x06, 0x4DE0);
		mdio_write(tp, 0x06, 0x834E);
		mdio_write(tp, 0x06, 0x7803);
		mdio_write(tp, 0x06, 0x9E0B);
		mdio_write(tp, 0x06, 0xE083);
		mdio_write(tp, 0x06, 0x4E78);
		mdio_write(tp, 0x06, 0x049E);
		mdio_write(tp, 0x06, 0x04EE);
		mdio_write(tp, 0x06, 0x834E);
		mdio_write(tp, 0x06, 0x02E0);
		mdio_write(tp, 0x06, 0x8332);
		mdio_write(tp, 0x06, 0xE183);
		mdio_write(tp, 0x06, 0x3359);
		mdio_write(tp, 0x06, 0x0FE2);
		mdio_write(tp, 0x06, 0x834D);
		mdio_write(tp, 0x06, 0x0C24);
		mdio_write(tp, 0x06, 0x5AF0);
		mdio_write(tp, 0x06, 0x1E12);
		mdio_write(tp, 0x06, 0xE4F8);
		mdio_write(tp, 0x06, 0x8CE5);
		mdio_write(tp, 0x06, 0xF88D);
		mdio_write(tp, 0x06, 0xE083);
		mdio_write(tp, 0x06, 0x30E1);
		mdio_write(tp, 0x06, 0x8331);
		mdio_write(tp, 0x06, 0x6801);
		mdio_write(tp, 0x06, 0xE4F8);
		mdio_write(tp, 0x06, 0x8AE5);
		mdio_write(tp, 0x06, 0xF88B);
		mdio_write(tp, 0x06, 0xAE37);
		mdio_write(tp, 0x06, 0xEE83);
		mdio_write(tp, 0x06, 0x4E03);
		mdio_write(tp, 0x06, 0xE083);
		mdio_write(tp, 0x06, 0x4CE1);
		mdio_write(tp, 0x06, 0x834D);
		mdio_write(tp, 0x06, 0x1B01);
		mdio_write(tp, 0x06, 0x9E04);
		mdio_write(tp, 0x06, 0xAAA1);
		mdio_write(tp, 0x06, 0xAEA8);
		mdio_write(tp, 0x06, 0xEE83);
		mdio_write(tp, 0x06, 0x4E04);
		mdio_write(tp, 0x06, 0xEE83);
		mdio_write(tp, 0x06, 0x4F00);
		mdio_write(tp, 0x06, 0xAEAB);
		mdio_write(tp, 0x06, 0xE083);
		mdio_write(tp, 0x06, 0x4F78);
		mdio_write(tp, 0x06, 0x039F);
		mdio_write(tp, 0x06, 0x14EE);
		mdio_write(tp, 0x06, 0x834E);
		mdio_write(tp, 0x06, 0x05D2);
		mdio_write(tp, 0x06, 0x40D6);
		mdio_write(tp, 0x06, 0x5554);
		mdio_write(tp, 0x06, 0x0282);
		mdio_write(tp, 0x06, 0x17D2);
		mdio_write(tp, 0x06, 0xA0D6);
		mdio_write(tp, 0x06, 0xBA00);
		mdio_write(tp, 0x06, 0x0282);
		mdio_write(tp, 0x06, 0x17FE);
		mdio_write(tp, 0x06, 0xFDFC);
		mdio_write(tp, 0x06, 0x05F8);
		mdio_write(tp, 0x06, 0xE0F8);
		mdio_write(tp, 0x06, 0x60E1);
		mdio_write(tp, 0x06, 0xF861);
		mdio_write(tp, 0x06, 0x6802);
		mdio_write(tp, 0x06, 0xE4F8);
		mdio_write(tp, 0x06, 0x60E5);
		mdio_write(tp, 0x06, 0xF861);
		mdio_write(tp, 0x06, 0xE0F8);
		mdio_write(tp, 0x06, 0x48E1);
		mdio_write(tp, 0x06, 0xF849);
		mdio_write(tp, 0x06, 0x580F);
		mdio_write(tp, 0x06, 0x1E02);
		mdio_write(tp, 0x06, 0xE4F8);
		mdio_write(tp, 0x06, 0x48E5);
		mdio_write(tp, 0x06, 0xF849);
		mdio_write(tp, 0x06, 0xD000);
		mdio_write(tp, 0x06, 0x0282);
		mdio_write(tp, 0x06, 0x5BBF);
		mdio_write(tp, 0x06, 0x8350);
		mdio_write(tp, 0x06, 0xEF46);
		mdio_write(tp, 0x06, 0xDC19);
		mdio_write(tp, 0x06, 0xDDD0);
		mdio_write(tp, 0x06, 0x0102);
		mdio_write(tp, 0x06, 0x825B);
		mdio_write(tp, 0x06, 0x0282);
		mdio_write(tp, 0x06, 0x77E0);
		mdio_write(tp, 0x06, 0xF860);
		mdio_write(tp, 0x06, 0xE1F8);
		mdio_write(tp, 0x06, 0x6158);
		mdio_write(tp, 0x06, 0xFDE4);
		mdio_write(tp, 0x06, 0xF860);
		mdio_write(tp, 0x06, 0xE5F8);
		mdio_write(tp, 0x06, 0x61FC);
		mdio_write(tp, 0x06, 0x04F9);
		mdio_write(tp, 0x06, 0xFAFB);
		mdio_write(tp, 0x06, 0xC6BF);
		mdio_write(tp, 0x06, 0xF840);
		mdio_write(tp, 0x06, 0xBE83);
		mdio_write(tp, 0x06, 0x50A0);
		mdio_write(tp, 0x06, 0x0101);
		mdio_write(tp, 0x06, 0x071B);
		mdio_write(tp, 0x06, 0x89CF);
		mdio_write(tp, 0x06, 0xD208);
		mdio_write(tp, 0x06, 0xEBDB);
		mdio_write(tp, 0x06, 0x19B2);
		mdio_write(tp, 0x06, 0xFBFF);
		mdio_write(tp, 0x06, 0xFEFD);
		mdio_write(tp, 0x06, 0x04F8);
		mdio_write(tp, 0x06, 0xE0F8);
		mdio_write(tp, 0x06, 0x48E1);
		mdio_write(tp, 0x06, 0xF849);
		mdio_write(tp, 0x06, 0x6808);
		mdio_write(tp, 0x06, 0xE4F8);
		mdio_write(tp, 0x06, 0x48E5);
		mdio_write(tp, 0x06, 0xF849);
		mdio_write(tp, 0x06, 0x58F7);
		mdio_write(tp, 0x06, 0xE4F8);
		mdio_write(tp, 0x06, 0x48E5);
		mdio_write(tp, 0x06, 0xF849);
		mdio_write(tp, 0x06, 0xFC04);
		mdio_write(tp, 0x06, 0x4D20);
		mdio_write(tp, 0x06, 0x0002);
		mdio_write(tp, 0x06, 0x4E22);
		mdio_write(tp, 0x06, 0x0002);
		mdio_write(tp, 0x06, 0x4DDF);
		mdio_write(tp, 0x06, 0xFF01);
		mdio_write(tp, 0x06, 0x4EDD);
		mdio_write(tp, 0x06, 0xFF01);
		mdio_write(tp, 0x05, 0x83D4);
		mdio_write(tp, 0x06, 0x8000);
		mdio_write(tp, 0x05, 0x83D8);
		mdio_write(tp, 0x06, 0x8051);
		mdio_write(tp, 0x02, 0x6010);
		mdio_write(tp, 0x03, 0xDC00);
		mdio_write(tp, 0x05, 0xFFF6);
		mdio_write(tp, 0x06, 0x00FC);
		mdio_write(tp, 0x1F, 0x0000);

		mdio_write(tp, 0x1F, 0x0000);
		mdio_write(tp, 0x0D, 0xF880);
		mdio_write(tp, 0x1F, 0x0000);
	} else if (tp->mcfg == CFG_METHOD_10) {
		mdio_write(tp, 0x1F, 0x0001);
		mdio_write(tp, 0x06, 0x4064);
		mdio_write(tp, 0x07, 0x2863);
		mdio_write(tp, 0x08, 0x059C);
		mdio_write(tp, 0x09, 0x26B4);
		mdio_write(tp, 0x0A, 0x6A19);
		mdio_write(tp, 0x0B, 0xDCC8);
		mdio_write(tp, 0x10, 0xF06D);
		mdio_write(tp, 0x14, 0x7F68);
		mdio_write(tp, 0x18, 0x7FD9);
		mdio_write(tp, 0x1C, 0xF0FF);
		mdio_write(tp, 0x1D, 0x3D9C);
		mdio_write(tp, 0x1F, 0x0003);
		mdio_write(tp, 0x12, 0xF49F);
		mdio_write(tp, 0x13, 0x070B);
		mdio_write(tp, 0x1A, 0x05AD);
		mdio_write(tp, 0x14, 0x94C0);

		mdio_write(tp, 0x1F, 0x0002);
		mdio_write(tp, 0x06, 0x5561);
		mdio_write(tp, 0x1F, 0x0005);
		mdio_write(tp, 0x05, 0x8332);
		mdio_write(tp, 0x06, 0x5561);

		if (rtl8168_efuse_read(tp, 0x01) == 0xb1) {
			mdio_write(tp, 0x1F, 0x0002);
			mdio_write(tp, 0x05, 0x669A);
			mdio_write(tp, 0x1F, 0x0005);
			mdio_write(tp, 0x05, 0x8330);
			mdio_write(tp, 0x06, 0x669A);

			mdio_write(tp, 0x1F, 0x0002);
			gphy_val = mdio_read(tp, 0x0D);
			if ((gphy_val & 0x00FF) != 0x006C) {
				gphy_val &= 0xFF00;
				mdio_write(tp, 0x1F, 0x0002);
				mdio_write(tp, 0x0D, gphy_val | 0x0065);
				mdio_write(tp, 0x0D, gphy_val | 0x0066);
				mdio_write(tp, 0x0D, gphy_val | 0x0067);
				mdio_write(tp, 0x0D, gphy_val | 0x0068);
				mdio_write(tp, 0x0D, gphy_val | 0x0069);
				mdio_write(tp, 0x0D, gphy_val | 0x006A);
				mdio_write(tp, 0x0D, gphy_val | 0x006B);
				mdio_write(tp, 0x0D, gphy_val | 0x006C);
			}
		} else {
			mdio_write(tp, 0x1F, 0x0002);
			mdio_write(tp, 0x05, 0x2642);
			mdio_write(tp, 0x1F, 0x0005);
			mdio_write(tp, 0x05, 0x8330);
			mdio_write(tp, 0x06, 0x2642);
		}

		mdio_write(tp, 0x1F, 0x0002);
		gphy_val = mdio_read(tp, 0x02);
		gphy_val &= ~BIT_10;
		gphy_val &= ~BIT_9;
		gphy_val |= BIT_8;
		mdio_write(tp, 0x02, gphy_val);
		gphy_val = mdio_read(tp, 0x03);
		gphy_val &= ~BIT_15;
		gphy_val &= ~BIT_14;
		gphy_val &= ~BIT_13;
		mdio_write(tp, 0x03, gphy_val);

		mdio_write(tp, 0x1F, 0x0001);
		mdio_write(tp, 0x17, 0x0CC0);

		mdio_write(tp, 0x1F, 0x0002);
		gphy_val = mdio_read(tp, 0x0F);
		gphy_val |= BIT_4;
		gphy_val |= BIT_2;
		gphy_val |= BIT_1;
		gphy_val |= BIT_0;
		mdio_write(tp, 0x0F, gphy_val);

		mdio_write(tp, 0x1F, 0x0005);
		mdio_write(tp, 0x05, 0xFFC2);
		mdio_write(tp, 0x1F, 0x0005);
		mdio_write(tp, 0x05, 0x8000);
		mdio_write(tp, 0x06, 0xF8F9);
		mdio_write(tp, 0x06, 0xFAEE);
		mdio_write(tp, 0x06, 0xF8EA);
		mdio_write(tp, 0x06, 0x00EE);
		mdio_write(tp, 0x06, 0xF8EB);
		mdio_write(tp, 0x06, 0x00E2);
		mdio_write(tp, 0x06, 0xF87C);
		mdio_write(tp, 0x06, 0xE3F8);
		mdio_write(tp, 0x06, 0x7DA5);
		mdio_write(tp, 0x06, 0x1111);
		mdio_write(tp, 0x06, 0x12D2);
		mdio_write(tp, 0x06, 0x40D6);
		mdio_write(tp, 0x06, 0x4444);
		mdio_write(tp, 0x06, 0x0281);
		mdio_write(tp, 0x06, 0xC6D2);
		mdio_write(tp, 0x06, 0xA0D6);
		mdio_write(tp, 0x06, 0xAAAA);
		mdio_write(tp, 0x06, 0x0281);
		mdio_write(tp, 0x06, 0xC6AE);
		mdio_write(tp, 0x06, 0x0FA5);
		mdio_write(tp, 0x06, 0x4444);
		mdio_write(tp, 0x06, 0x02AE);
		mdio_write(tp, 0x06, 0x4DA5);
		mdio_write(tp, 0x06, 0xAAAA);
		mdio_write(tp, 0x06, 0x02AE);
		mdio_write(tp, 0x06, 0x47AF);
		mdio_write(tp, 0x06, 0x81C2);
		mdio_write(tp, 0x06, 0xEE83);
		mdio_write(tp, 0x06, 0x4E00);
		mdio_write(tp, 0x06, 0xEE83);
		mdio_write(tp, 0x06, 0x4D0F);
		mdio_write(tp, 0x06, 0xEE83);
		mdio_write(tp, 0x06, 0x4C0F);
		mdio_write(tp, 0x06, 0xEE83);
		mdio_write(tp, 0x06, 0x4F00);
		mdio_write(tp, 0x06, 0xEE83);
		mdio_write(tp, 0x06, 0x5100);
		mdio_write(tp, 0x06, 0xEE83);
		mdio_write(tp, 0x06, 0x4AFF);
		mdio_write(tp, 0x06, 0xEE83);
		mdio_write(tp, 0x06, 0x4BFF);
		mdio_write(tp, 0x06, 0xE083);
		mdio_write(tp, 0x06, 0x30E1);
		mdio_write(tp, 0x06, 0x8331);
		mdio_write(tp, 0x06, 0x58FE);
		mdio_write(tp, 0x06, 0xE4F8);
		mdio_write(tp, 0x06, 0x8AE5);
		mdio_write(tp, 0x06, 0xF88B);
		mdio_write(tp, 0x06, 0xE083);
		mdio_write(tp, 0x06, 0x32E1);
		mdio_write(tp, 0x06, 0x8333);
		mdio_write(tp, 0x06, 0x590F);
		mdio_write(tp, 0x06, 0xE283);
		mdio_write(tp, 0x06, 0x4D0C);
		mdio_write(tp, 0x06, 0x245A);
		mdio_write(tp, 0x06, 0xF01E);
		mdio_write(tp, 0x06, 0x12E4);
		mdio_write(tp, 0x06, 0xF88C);
		mdio_write(tp, 0x06, 0xE5F8);
		mdio_write(tp, 0x06, 0x8DAF);
		mdio_write(tp, 0x06, 0x81C2);
		mdio_write(tp, 0x06, 0xE083);
		mdio_write(tp, 0x06, 0x4F10);
		mdio_write(tp, 0x06, 0xE483);
		mdio_write(tp, 0x06, 0x4FE0);
		mdio_write(tp, 0x06, 0x834E);
		mdio_write(tp, 0x06, 0x7800);
		mdio_write(tp, 0x06, 0x9F0A);
		mdio_write(tp, 0x06, 0xE083);
		mdio_write(tp, 0x06, 0x4FA0);
		mdio_write(tp, 0x06, 0x10A5);
		mdio_write(tp, 0x06, 0xEE83);
		mdio_write(tp, 0x06, 0x4E01);
		mdio_write(tp, 0x06, 0xE083);
		mdio_write(tp, 0x06, 0x4E78);
		mdio_write(tp, 0x06, 0x059E);
		mdio_write(tp, 0x06, 0x9AE0);
		mdio_write(tp, 0x06, 0x834E);
		mdio_write(tp, 0x06, 0x7804);
		mdio_write(tp, 0x06, 0x9E10);
		mdio_write(tp, 0x06, 0xE083);
		mdio_write(tp, 0x06, 0x4E78);
		mdio_write(tp, 0x06, 0x039E);
		mdio_write(tp, 0x06, 0x0FE0);
		mdio_write(tp, 0x06, 0x834E);
		mdio_write(tp, 0x06, 0x7801);
		mdio_write(tp, 0x06, 0x9E05);
		mdio_write(tp, 0x06, 0xAE0C);
		mdio_write(tp, 0x06, 0xAF81);
		mdio_write(tp, 0x06, 0xA7AF);
		mdio_write(tp, 0x06, 0x8152);
		mdio_write(tp, 0x06, 0xAF81);
		mdio_write(tp, 0x06, 0x8BAF);
		mdio_write(tp, 0x06, 0x81C2);
		mdio_write(tp, 0x06, 0xEE83);
		mdio_write(tp, 0x06, 0x4800);
		mdio_write(tp, 0x06, 0xEE83);
		mdio_write(tp, 0x06, 0x4900);
		mdio_write(tp, 0x06, 0xE083);
		mdio_write(tp, 0x06, 0x5110);
		mdio_write(tp, 0x06, 0xE483);
		mdio_write(tp, 0x06, 0x5158);
		mdio_write(tp, 0x06, 0x019F);
		mdio_write(tp, 0x06, 0xEAD0);
		mdio_write(tp, 0x06, 0x00D1);
		mdio_write(tp, 0x06, 0x801F);
		mdio_write(tp, 0x06, 0x66E2);
		mdio_write(tp, 0x06, 0xF8EA);
		mdio_write(tp, 0x06, 0xE3F8);
		mdio_write(tp, 0x06, 0xEB5A);
		mdio_write(tp, 0x06, 0xF81E);
		mdio_write(tp, 0x06, 0x20E6);
		mdio_write(tp, 0x06, 0xF8EA);
		mdio_write(tp, 0x06, 0xE5F8);
		mdio_write(tp, 0x06, 0xEBD3);
		mdio_write(tp, 0x06, 0x02B3);
		mdio_write(tp, 0x06, 0xFEE2);
		mdio_write(tp, 0x06, 0xF87C);
		mdio_write(tp, 0x06, 0xEF32);
		mdio_write(tp, 0x06, 0x5B80);
		mdio_write(tp, 0x06, 0xE3F8);
		mdio_write(tp, 0x06, 0x7D9E);
		mdio_write(tp, 0x06, 0x037D);
		mdio_write(tp, 0x06, 0xFFFF);
		mdio_write(tp, 0x06, 0x0D58);
		mdio_write(tp, 0x06, 0x1C55);
		mdio_write(tp, 0x06, 0x1A65);
		mdio_write(tp, 0x06, 0x11A1);
		mdio_write(tp, 0x06, 0x90D3);
		mdio_write(tp, 0x06, 0xE283);
		mdio_write(tp, 0x06, 0x48E3);
		mdio_write(tp, 0x06, 0x8349);
		mdio_write(tp, 0x06, 0x1B56);
		mdio_write(tp, 0x06, 0xAB08);
		mdio_write(tp, 0x06, 0xEF56);
		mdio_write(tp, 0x06, 0xE683);
		mdio_write(tp, 0x06, 0x48E7);
		mdio_write(tp, 0x06, 0x8349);
		mdio_write(tp, 0x06, 0x10D1);
		mdio_write(tp, 0x06, 0x801F);
		mdio_write(tp, 0x06, 0x66A0);
		mdio_write(tp, 0x06, 0x04B9);
		mdio_write(tp, 0x06, 0xE283);
		mdio_write(tp, 0x06, 0x48E3);
		mdio_write(tp, 0x06, 0x8349);
		mdio_write(tp, 0x06, 0xEF65);
		mdio_write(tp, 0x06, 0xE283);
		mdio_write(tp, 0x06, 0x4AE3);
		mdio_write(tp, 0x06, 0x834B);
		mdio_write(tp, 0x06, 0x1B56);
		mdio_write(tp, 0x06, 0xAA0E);
		mdio_write(tp, 0x06, 0xEF56);
		mdio_write(tp, 0x06, 0xE683);
		mdio_write(tp, 0x06, 0x4AE7);
		mdio_write(tp, 0x06, 0x834B);
		mdio_write(tp, 0x06, 0xE283);
		mdio_write(tp, 0x06, 0x4DE6);
		mdio_write(tp, 0x06, 0x834C);
		mdio_write(tp, 0x06, 0xE083);
		mdio_write(tp, 0x06, 0x4DA0);
		mdio_write(tp, 0x06, 0x000C);
		mdio_write(tp, 0x06, 0xAF81);
		mdio_write(tp, 0x06, 0x8BE0);
		mdio_write(tp, 0x06, 0x834D);
		mdio_write(tp, 0x06, 0x10E4);
		mdio_write(tp, 0x06, 0x834D);
		mdio_write(tp, 0x06, 0xAE04);
		mdio_write(tp, 0x06, 0x80E4);
		mdio_write(tp, 0x06, 0x834D);
		mdio_write(tp, 0x06, 0xE083);
		mdio_write(tp, 0x06, 0x4E78);
		mdio_write(tp, 0x06, 0x039E);
		mdio_write(tp, 0x06, 0x0BE0);
		mdio_write(tp, 0x06, 0x834E);
		mdio_write(tp, 0x06, 0x7804);
		mdio_write(tp, 0x06, 0x9E04);
		mdio_write(tp, 0x06, 0xEE83);
		mdio_write(tp, 0x06, 0x4E02);
		mdio_write(tp, 0x06, 0xE083);
		mdio_write(tp, 0x06, 0x32E1);
		mdio_write(tp, 0x06, 0x8333);
		mdio_write(tp, 0x06, 0x590F);
		mdio_write(tp, 0x06, 0xE283);
		mdio_write(tp, 0x06, 0x4D0C);
		mdio_write(tp, 0x06, 0x245A);
		mdio_write(tp, 0x06, 0xF01E);
		mdio_write(tp, 0x06, 0x12E4);
		mdio_write(tp, 0x06, 0xF88C);
		mdio_write(tp, 0x06, 0xE5F8);
		mdio_write(tp, 0x06, 0x8DE0);
		mdio_write(tp, 0x06, 0x8330);
		mdio_write(tp, 0x06, 0xE183);
		mdio_write(tp, 0x06, 0x3168);
		mdio_write(tp, 0x06, 0x01E4);
		mdio_write(tp, 0x06, 0xF88A);
		mdio_write(tp, 0x06, 0xE5F8);
		mdio_write(tp, 0x06, 0x8BAE);
		mdio_write(tp, 0x06, 0x37EE);
		mdio_write(tp, 0x06, 0x834E);
		mdio_write(tp, 0x06, 0x03E0);
		mdio_write(tp, 0x06, 0x834C);
		mdio_write(tp, 0x06, 0xE183);
		mdio_write(tp, 0x06, 0x4D1B);
		mdio_write(tp, 0x06, 0x019E);
		mdio_write(tp, 0x06, 0x04AA);
		mdio_write(tp, 0x06, 0xA1AE);
		mdio_write(tp, 0x06, 0xA8EE);
		mdio_write(tp, 0x06, 0x834E);
		mdio_write(tp, 0x06, 0x04EE);
		mdio_write(tp, 0x06, 0x834F);
		mdio_write(tp, 0x06, 0x00AE);
		mdio_write(tp, 0x06, 0xABE0);
		mdio_write(tp, 0x06, 0x834F);
		mdio_write(tp, 0x06, 0x7803);
		mdio_write(tp, 0x06, 0x9F14);
		mdio_write(tp, 0x06, 0xEE83);
		mdio_write(tp, 0x06, 0x4E05);
		mdio_write(tp, 0x06, 0xD240);
		mdio_write(tp, 0x06, 0xD655);
		mdio_write(tp, 0x06, 0x5402);
		mdio_write(tp, 0x06, 0x81C6);
		mdio_write(tp, 0x06, 0xD2A0);
		mdio_write(tp, 0x06, 0xD6BA);
		mdio_write(tp, 0x06, 0x0002);
		mdio_write(tp, 0x06, 0x81C6);
		mdio_write(tp, 0x06, 0xFEFD);
		mdio_write(tp, 0x06, 0xFC05);
		mdio_write(tp, 0x06, 0xF8E0);
		mdio_write(tp, 0x06, 0xF860);
		mdio_write(tp, 0x06, 0xE1F8);
		mdio_write(tp, 0x06, 0x6168);
		mdio_write(tp, 0x06, 0x02E4);
		mdio_write(tp, 0x06, 0xF860);
		mdio_write(tp, 0x06, 0xE5F8);
		mdio_write(tp, 0x06, 0x61E0);
		mdio_write(tp, 0x06, 0xF848);
		mdio_write(tp, 0x06, 0xE1F8);
		mdio_write(tp, 0x06, 0x4958);
		mdio_write(tp, 0x06, 0x0F1E);
		mdio_write(tp, 0x06, 0x02E4);
		mdio_write(tp, 0x06, 0xF848);
		mdio_write(tp, 0x06, 0xE5F8);
		mdio_write(tp, 0x06, 0x49D0);
		mdio_write(tp, 0x06, 0x0002);
		mdio_write(tp, 0x06, 0x820A);
		mdio_write(tp, 0x06, 0xBF83);
		mdio_write(tp, 0x06, 0x50EF);
		mdio_write(tp, 0x06, 0x46DC);
		mdio_write(tp, 0x06, 0x19DD);
		mdio_write(tp, 0x06, 0xD001);
		mdio_write(tp, 0x06, 0x0282);
		mdio_write(tp, 0x06, 0x0A02);
		mdio_write(tp, 0x06, 0x8226);
		mdio_write(tp, 0x06, 0xE0F8);
		mdio_write(tp, 0x06, 0x60E1);
		mdio_write(tp, 0x06, 0xF861);
		mdio_write(tp, 0x06, 0x58FD);
		mdio_write(tp, 0x06, 0xE4F8);
		mdio_write(tp, 0x06, 0x60E5);
		mdio_write(tp, 0x06, 0xF861);
		mdio_write(tp, 0x06, 0xFC04);
		mdio_write(tp, 0x06, 0xF9FA);
		mdio_write(tp, 0x06, 0xFBC6);
		mdio_write(tp, 0x06, 0xBFF8);
		mdio_write(tp, 0x06, 0x40BE);
		mdio_write(tp, 0x06, 0x8350);
		mdio_write(tp, 0x06, 0xA001);
		mdio_write(tp, 0x06, 0x0107);
		mdio_write(tp, 0x06, 0x1B89);
		mdio_write(tp, 0x06, 0xCFD2);
		mdio_write(tp, 0x06, 0x08EB);
		mdio_write(tp, 0x06, 0xDB19);
		mdio_write(tp, 0x06, 0xB2FB);
		mdio_write(tp, 0x06, 0xFFFE);
		mdio_write(tp, 0x06, 0xFD04);
		mdio_write(tp, 0x06, 0xF8E0);
		mdio_write(tp, 0x06, 0xF848);
		mdio_write(tp, 0x06, 0xE1F8);
		mdio_write(tp, 0x06, 0x4968);
		mdio_write(tp, 0x06, 0x08E4);
		mdio_write(tp, 0x06, 0xF848);
		mdio_write(tp, 0x06, 0xE5F8);
		mdio_write(tp, 0x06, 0x4958);
		mdio_write(tp, 0x06, 0xF7E4);
		mdio_write(tp, 0x06, 0xF848);
		mdio_write(tp, 0x06, 0xE5F8);
		mdio_write(tp, 0x06, 0x49FC);
		mdio_write(tp, 0x06, 0x044D);
		mdio_write(tp, 0x06, 0x2000);
		mdio_write(tp, 0x06, 0x024E);
		mdio_write(tp, 0x06, 0x2200);
		mdio_write(tp, 0x06, 0x024D);
		mdio_write(tp, 0x06, 0xDFFF);
		mdio_write(tp, 0x06, 0x014E);
		mdio_write(tp, 0x06, 0xDDFF);
		mdio_write(tp, 0x06, 0x0100);
		mdio_write(tp, 0x05, 0x83D8);
		mdio_write(tp, 0x06, 0x8000);
		mdio_write(tp, 0x03, 0xDC00);
		mdio_write(tp, 0x05, 0xFFF6);
		mdio_write(tp, 0x06, 0x00FC);
		mdio_write(tp, 0x1F, 0x0000);

		mdio_write(tp, 0x1F, 0x0000);
		mdio_write(tp, 0x0D, 0xF880);
		mdio_write(tp, 0x1F, 0x0000);
	} else if (tp->mcfg == CFG_METHOD_11) {
		mdio_write(tp, 0x1F, 0x0002);
		mdio_write(tp, 0x10, 0x0008);
		mdio_write(tp, 0x0D, 0x006C);

		mdio_write(tp, 0x1F, 0x0000);
		mdio_write(tp, 0x0D, 0xF880);

		mdio_write(tp, 0x1F, 0x0001);
		mdio_write(tp, 0x17, 0x0CC0);

		mdio_write(tp, 0x1F, 0x0001);
		mdio_write(tp, 0x0B, 0xA4D8);
		mdio_write(tp, 0x09, 0x281C);
		mdio_write(tp, 0x07, 0x2883);
		mdio_write(tp, 0x0A, 0x6B35);
		mdio_write(tp, 0x1D, 0x3DA4);
		mdio_write(tp, 0x1C, 0xEFFD);
		mdio_write(tp, 0x14, 0x7F52);
		mdio_write(tp, 0x18, 0x7FC6);
		mdio_write(tp, 0x08, 0x0601);
		mdio_write(tp, 0x06, 0x4063);
		mdio_write(tp, 0x10, 0xF074);
		mdio_write(tp, 0x1F, 0x0003);
		mdio_write(tp, 0x13, 0x0789);
		mdio_write(tp, 0x12, 0xF4BD);
		mdio_write(tp, 0x1A, 0x04FD);
		mdio_write(tp, 0x14, 0x84B0);
		mdio_write(tp, 0x1F, 0x0000);
		mdio_write(tp, 0x00, 0x9200);

		mdio_write(tp, 0x1F, 0x0005);
		mdio_write(tp, 0x01, 0x0340);
		mdio_write(tp, 0x1F, 0x0001);
		mdio_write(tp, 0x04, 0x4000);
		mdio_write(tp, 0x03, 0x1D21);
		mdio_write(tp, 0x02, 0x0C32);
		mdio_write(tp, 0x01, 0x0200);
		mdio_write(tp, 0x00, 0x5554);
		mdio_write(tp, 0x04, 0x4800);
		mdio_write(tp, 0x04, 0x4000);
		mdio_write(tp, 0x04, 0xF000);
		mdio_write(tp, 0x03, 0xDF01);
		mdio_write(tp, 0x02, 0xDF20);
		mdio_write(tp, 0x01, 0x101A);
		mdio_write(tp, 0x00, 0xA0FF);
		mdio_write(tp, 0x04, 0xF800);
		mdio_write(tp, 0x04, 0xF000);
		mdio_write(tp, 0x1F, 0x0000);

		mdio_write(tp, 0x1F, 0x0007);
		mdio_write(tp, 0x1E, 0x0023);
		mdio_write(tp, 0x16, 0x0000);
		mdio_write(tp, 0x1F, 0x0000);
	}

}

static u8 rtl8168_efuse_read(struct rtl8168_private *tp, u16 reg)
{
	u8 efuse_data;
	u32 temp;
	int cnt;

	if (tp->efuse == EFUSE_NOT_SUPPORT)
		return EFUSE_READ_FAIL;

	temp = EFUSE_READ | ((reg & EFUSE_Reg_Mask) << EFUSE_Reg_Shift);
	RTL_W32(tp, EFUSEAR, temp);

	do {
		delay(100);
		temp = RTL_R32(tp, EFUSEAR);
		cnt++;
	} while (!(temp & EFUSE_READ_OK) && (temp < EFUSE_Check_Cnt));

	if (temp == EFUSE_Check_Cnt)
		efuse_data = EFUSE_READ_FAIL;
	else
		efuse_data = (u8)(RTL_R32(tp, EFUSEAR) & EFUSE_Data_Mask);

	return efuse_data;
}

static void rtl8168_link_option(int idx, u8 *aut, u16 *spd, u8 *dup)
{
	unsigned char opt_speed;
	unsigned char opt_duplex;
	unsigned char opt_autoneg;

	opt_speed = ((idx < MAX_UNITS) && (idx >= 0)) ? speed[idx] : 0xff;
	opt_duplex = ((idx < MAX_UNITS) && (idx >= 0)) ? duplex[idx] : 0xff;
	opt_autoneg = ((idx < MAX_UNITS) && (idx >= 0)) ? autoneg[idx] : 0xff;

	if ((opt_speed == 0xff) |
	    (opt_duplex == 0xff) |
	    (opt_autoneg == 0xff)) {
		*spd = SPEED_1000;
		*dup = DUPLEX_FULL;
		*aut = AUTONEG_ENABLE;
	} else {
		*spd = speed[idx];
		*dup = duplex[idx];
		*aut = autoneg[idx];	
	}
}

static int rtl8168_set_speed(struct rtl8168_private *tp, u8 autoneg, u16 speed, u8 duplex)
{
	int ret;

	ret = tp->set_speed(tp, autoneg, speed, duplex);

	return ret;
}

static int rtl8168_ether_ioctl(struct ifnet *ifp, unsigned long cmd, caddr_t data)
{
	struct ifaddr *ifa = (struct ifaddr *) data;
	struct rtl8168_private *sc = ifp->if_softc;
	int error = 0;
	int s;

	s = splimp();
	switch (cmd) {
	case SIOCPOLL:
		break;
	case SIOCSIFADDR:
		switch (ifa->ifa_addr->sa_family) {
#ifdef INET
		case AF_INET:
			error = rtl8168_open(sc);
			if(error == -1){
				return(error);
			}	
			ifp->if_flags |= IFF_UP;
			
#ifdef __OpenBSD__
			arp_ifinit(&sc->arpcom, ifa);
			printf("\n");
#else
			arp_ifinit(ifp, ifa);
#endif
			
			break;
#endif

		default:
		       rtl8168_open(sc);
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
		if(ifp->if_flags & IFF_UP){
             rtl8168_open(sc);
		}
		break;
	default:
		error = EINVAL;
	}

	splx(s);

	return (error);
}

static void rtl8168_start_xmit(struct ifnet *ifp)
{
	struct rtl8168_private *tp = ifp->if_softc;
	unsigned int entry = tp->cur_tx % NUM_TX_DESC;
	struct TxDesc *txd = tp->TxDescArray + entry;

	if(tp->cur_tx - tp->dirty_tx == NUM_TX_DESC){
		dprintk("Full ????\n"); 
		return;
	}
	
	if (le32_to_cpu(txd->opts1) & DescOwn){
		dprintk("DescOwn Error!Desc NUM = %d", entry); 
		tp->stats.tx_dropped++;
		return;
	}

	if(rtl8168_xmit_frags(tp, ifp) < 0){
		dprintk("TX Error!  %s, %d\n", __FUNCTION__, __LINE__);
		return;
	}//transmit all

	RTL_W8(tp, TxPoll, 0x40);	// set polling bit 


	if (TX_BUFFS_AVAIL(tp) < MAX_FRAGS) { //FIXME
		if (TX_BUFFS_AVAIL(tp) >= MAX_FRAGS){
		    ;//FIXME netif_wake_queue(dev);
		}
	
	}

	return ;
}

static int rtl8168_xmit_frags(struct rtl8168_private *tp,  struct ifnet *ifp)
{
	struct TxDesc* td;
	struct mbuf *mb_head;
	unsigned int entry = 0;

	while(ifp->if_snd.ifq_head != NULL){
		u32 status, len;
		u32 IfRingEnd = 0;
		  
		entry = tp->cur_tx % NUM_TX_DESC;
		if(tp->TxDescArray[entry].opts1 & DescOwn){
				printf("NO available TX buffer ! %d\n", __LINE__);
				break;
		}
		
		td = tp->TxDescArray + entry;

		IF_DEQUEUE(&ifp->if_snd, mb_head);
		m_copydata(mb_head, 0, mb_head->m_pkthdr.len, tp->tx_buffer[entry]);
	             
		len = mb_head->m_pkthdr.len;
		IfRingEnd = ((entry + 1) % NUM_TX_DESC) ? 0 : RingEnd;
	    status = DescOwn |FirstFrag | LastFrag |len | IfRingEnd;//no fragement.
		td->opts1 = cpu_to_le32(status);

#ifdef __mips__
#if !(defined(LS3_HT) || defined(LS2G_HT))
		pci_sync_cache(tp->sc_pc, (vm_offset_t)tp->tx_buffer[entry], 
				len, SYNC_W);
#endif
#endif
		m_freem(mb_head);
		wbflush();  
		tp->cur_tx ++;
		
	}

	return 0;
}

static int rtl8168_open_times = 0; 
static int rtl8168_open(struct rtl8168_private *tp)
{
	int retval = 0;
    u32 status;

	/*
	 * Rx and Tx desscriptors needs 256 bytes alignment.
	 * pci_alloc_consistent provides more.
	 */
    if(!tp->TxDescArray){
		tp->TxDescArray = (struct TxDesc*)malloc(R8168_TX_RING_BYTES + 255, M_DEVBUF, M_DONTWAIT);
		if (!tp->TxDescArray) {
			retval = -ENOMEM;
			goto err_free_tx;
		}
		memset((caddr_t)tp->TxDescArray, 0, R8168_TX_RING_BYTES + 255);
#if !(defined(LS3_HT) || defined(LS2G_HT))
       	pci_sync_cache(tp->sc_pc, (vm_offset_t)(tp->TxDescArray),  R8168_TX_RING_BYTES + 255, SYNC_W);
#endif
       	tp->TxDescArray = (struct TxDesc *)(((unsigned long)(tp->TxDescArray) + 255) & ~255);	
	}
#if !(defined(LS3_HT) || defined(LS2G_HT))
	tp->TxDescArray = (struct TxDesc*)CACHED_TO_UNCACHED((unsigned long)(tp->TxDescArray));
#else
	tp->TxDescArray = (struct TxDesc*)((unsigned long)(tp->TxDescArray));
#endif
	tp->TxPhyAddr = (unsigned long)vtophys((unsigned long)(tp->TxDescArray));

	if(!tp->RxDescArray){
		tp->RxDescArray = (struct RxDesc*)malloc(R8168_RX_RING_BYTES + 255, M_DEVBUF, M_DONTWAIT);
	       if (!tp->RxDescArray) {
		      retval = -ENOMEM;
		      goto err_free_rx;
		   }		
		   memset((caddr_t)(tp->RxDescArray), 0, R8168_RX_RING_BYTES + 255);
#if !(defined(LS3_HT) || defined(LS2G_HT))
		   pci_sync_cache(tp->sc_pc, (vm_offset_t)(tp->RxDescArray),  R8168_RX_RING_BYTES + 255, SYNC_W);
#endif
		   tp->RxDescArray = (struct RxDesc *)(((unsigned long)(tp->RxDescArray) + 255) & ~255);	
	}
#if !(defined(LS3_HT) || defined(LS2G_HT))
	tp->RxDescArray = (struct RxDesc*)CACHED_TO_UNCACHED((unsigned long)(tp->RxDescArray));
#else
	tp->RxDescArray = (struct RxDesc*)((unsigned long)(tp->RxDescArray));
#endif
	tp->RxPhyAddr = (unsigned long)vtophys((unsigned long)(tp->RxDescArray));

	tp->tx_buf_sz = TX_BUF_SIZE;
	tp->rx_buf_sz = RX_BUF_SIZE;
	retval = rtl8168_init_ring(tp);
    
	if (retval < 0){
		goto err_free_rx;
	}
	rtl8168_hw_reset(tp);//by AdonWang
    
	rtl8168_hw_start(tp);
    
	rtl8168_check_link_status(tp);
    tp->arpcom.ac_if.if_flags |=  IFF_RUNNING;

    status = RTL_R16(tp, IntrStatus);

out:
	return retval;

err_free_rx:
	free(tp->RxDescArray, M_DEVBUF);
err_free_tx:
	free(tp->TxDescArray, M_DEVBUF);

	goto out;
}


static void rtl8168_init_ring_indexes(struct rtl8168_private *tp)
{
	tp->dirty_tx = tp->dirty_rx = tp->cur_tx = tp->cur_rx = 0;
}

static int rtl8168_init_ring(struct rtl8168_private *tp)
{
	int tmpres;

	rtl8168_init_ring_indexes(tp);

	if(rtl8168_open_times== 1){
		memset(tp->tx_buffer, 0x0, sizeof(char*) * NUM_TX_DESC);
		memset(tp->rx_buffer, 0x0, sizeof(char*) * NUM_RX_DESC);
	}

	if ((tmpres = rtl8168_rx_fill(tp, 0, NUM_RX_DESC, 0)) != NUM_RX_DESC){
		goto err_out1;
	}

	if (rtl8168_tx_fill(tp, 0, NUM_TX_DESC, 0) != NUM_TX_DESC){
		goto err_out2;
	}

	rtl8168_mark_as_last_descriptor(tp->RxDescArray + NUM_RX_DESC - 1);
	return 0;

err_out2:
	rtl8168_rx_clear(tp);
err_out1:
	rtl8168_tx_clear(tp);

	return -ENOMEM;
}

static u32 rtl8168_rx_fill(struct rtl8168_private *tp,
			   u32 start, u32 end, int caller)
{
	u32 cur = 0, tmprecord = 0;
	int ret;
	if (caller == 0) {
		for(cur = 0; cur < NUM_RX_DESC; cur++) {
			if (tp->rx_buffer[cur])
				continue;

       		ret = rtl8168_alloc_rx(&(tp->rx_buffer[cur]), tp->RxDescArray + cur, tp->rx_buf_sz, caller);
			if(ret < 0){
				printf("ERROR: %s, %d\n", __FUNCTION__, __LINE__);
				break;
			}
		}
		tp->RxDescArray[cur - 1].opts1 |= RingEnd;
		return cur;
	}

	//caller == 1
	for (cur = start; cur < end; cur++) {
		int  entry = cur % NUM_RX_DESC;

		if(tp->RxDescArray[entry].opts1& DescOwn) {
			break;
		}

		tmprecord = tp->RxDescArray[entry].opts1 & RingEnd;
		tp->RxDescArray[entry].opts1 = 0;
		tp->RxDescArray[entry].opts1 = tmprecord | DescOwn | tp->rx_buf_sz;
	}

	return cur - start;
}

static u32 rtl8168_tx_fill(struct rtl8168_private *tp,
			   u32 start, u32 end, u32 caller)
{
	u32 cur, ret, entry;
	for (cur = start;  cur < end; cur++) {
		entry = cur % NUM_TX_DESC;
		if(caller == 0){
			if(tp->tx_buffer[entry])
				continue;
			tp->TxDescArray[entry].opts1 = 0;
			tp->TxDescArray[entry].opts2 = 0;
			tp->TxDescArray[entry].addr = 0;
			tp->tx_buffer[entry] = NULL;

			ret = rtl8168_alloc_tx(&(tp->tx_buffer[entry]),
						tp->TxDescArray + entry, tp->tx_buf_sz);
			if (ret < 0) {
				printf("ret = %d. %s, %d\n", ret, __FILE__, __LINE__);
				break;
			}
		}

		if(caller == 1) {
			if(tp->TxDescArray[entry].opts1 & DescOwn)
				break;
			tp->TxDescArray[entry].opts1 &= RingEnd;
		}
	}
	return cur - start;
}


static void rtl8168_rx_clear(struct rtl8168_private *tp)
{
	int i;

	for (i = 0; i < NUM_RX_DESC; i++) {
		if (tp->rx_buffer[i]) {
			rtl8168_free_rx(tp, tp->rx_buffer[i],tp->RxDescArray + i);
		}
	}
}

static void rtl8168_tx_clear(struct rtl8168_private *tp)
{
	unsigned int i;

	for (i = tp->dirty_tx; i < tp->dirty_tx + NUM_TX_DESC; i++) {
		unsigned int entry = i % NUM_TX_DESC;
		if(tp->tx_buffer[entry]){
			rtl8168_free_tx( tp, &tp->tx_buffer[entry], tp->TxDescArray+entry);
		}
		tp->stats.tx_dropped++;
	}
	tp->cur_tx = tp->dirty_tx = 0;
}

static void rtl8168_free_tx(struct rtl8168_private * tp, unsigned char ** buf, struct TxDesc * desc)
{
	free(buf, M_DEVBUF);
	*buf = NULL;
	rtl8168_make_unusable_by_asic(desc);
	return;
}

static void rtl8168_free_rx(struct rtl8168_private *tp,
				unsigned char *buf, struct RxDesc *desc)
{
	free(buf, M_DEVBUF);
	buf = NULL;
	rtl8168_make_unusable_by_asic(desc);
}

static inline void rtl8168_make_unusable_by_asic(struct RxDesc *desc)
{
	desc->addr = 0x0badbadbadbadbadull;;
	desc->opts1 &=  ~cpu_to_le32(DescOwn | RsvdMask);
}


static int rtl8168_alloc_tx(unsigned char **tx_buffer,
				struct TxDesc *desc, int tx_buf_sz)
{
	unsigned long mapping;
	int ret = 0;
	unsigned long tmp;


  	tmp = (unsigned long)malloc(tx_buf_sz, M_DEVBUF, M_DONTWAIT);
	if(!tmp){
		goto err_out;
	}

#ifdef __mips__
#if !(defined(LS3_HT) || defined(LS2G_HT))
	pci_sync_cache(NULL, (vm_offset_t)tmp, tx_buf_sz, SYNC_W);
#endif
#endif

#if !(defined(LS3_HT) || defined(LS2G_HT))
	*tx_buffer = (unsigned char *)CACHED_TO_UNCACHED(tmp);
#else
	*tx_buffer = (unsigned char *)(tmp);
#endif

	mapping = (unsigned long)vtophys(*tx_buffer);

	desc->addr = cpu_to_le64(mapping);

	return ret;
err_out:
	ret = -ENOMEM;
	return ret;
}

static int rtl8168_alloc_rx(unsigned char **rx_buffer,
				struct RxDesc *desc, int rx_buf_sz, int caller)
{
	unsigned long mapping;
	int ret = 0;


	*rx_buffer = (char*)malloc(rx_buf_sz + 7, M_DEVBUF, M_DONTWAIT);
	if(!(*rx_buffer)){
		goto err_out;
	}
	*rx_buffer = (unsigned char *)(((unsigned long)(*rx_buffer) + 7) & ~7);
#if !(defined(LS3_HT) || defined(LS2G_HT))
	*rx_buffer = (unsigned char *)CACHED_TO_UNCACHED(*rx_buffer);	
#else
	*rx_buffer = (unsigned char *)(*rx_buffer);	
#endif

	mapping = (unsigned long)vtophys(*rx_buffer);

	rtl8168_map_to_asic(desc, mapping, rx_buf_sz, caller);

out:
	return ret;

err_out:
	ret = -ENOMEM;
	rtl8168_make_unusable_by_asic(desc);
	goto out;
}

static inline void rtl8168_map_to_asic(struct RxDesc *desc, unsigned long mapping, u32 rx_buf_sz, int caller)
{
       desc->addr = cpu_to_le64(mapping);

	rtl8168_mark_to_asic(desc, rx_buf_sz, caller);
}


static inline void rtl8168_mark_to_asic(struct RxDesc *desc, u32 rx_buf_sz, int caller)
{
        unsigned int eor = 0;
	if(caller == 1)
            eor = le32_to_cpu(desc->opts1) & RingEnd;

      	desc->opts1 = cpu_to_le32(DescOwn | eor | (u16)rx_buf_sz);
}

static inline void rtl8168_mark_as_last_descriptor(struct RxDesc *desc)
{
	desc->opts1 |= cpu_to_le32(RingEnd);
}


static void 
rtl8168_hw_reset(struct  rtl8168_private *tp)
{
	/* Disable interrupts */
	rtl8168_irq_mask_and_ack(tp);

	rtl8168_nic_reset(tp);
}

static void 
rtl8168_irq_mask_and_ack(struct rtl8168_private *tp)
{
	RTL_W16(tp, IntrMask, 0x0000);
}

static void 
rtl8168_nic_reset(struct rtl8168_private *tp)
{
	int i;

	if ((tp->mcfg != CFG_METHOD_1) && 
	    (tp->mcfg != CFG_METHOD_2) &&
	    (tp->mcfg != CFG_METHOD_2) &&
	    (tp->mcfg != CFG_METHOD_11)) {
		RTL_W8(tp, ChipCmd, StopReq | CmdRxEnb | CmdTxEnb);
		delay(100);
	}

	if (tp->mcfg == CFG_METHOD_11)
		RTL_W32(tp, RxConfig, RTL_R32(tp, RxConfig) & ~(AcceptErr | AcceptRunt | AcceptBroadcast | AcceptMulticast | AcceptMyPhys |  AcceptAllPhys));

	/* Soft reset the chip. */
	RTL_W8(tp, ChipCmd, CmdReset);

	/* Check that the chip has finished the reset. */
	for (i = 1000; i > 0; i--) {
		if ((RTL_R8(tp, ChipCmd) & CmdReset) == 0)
			break;
		delay(100);
	}

	if (tp->mcfg == CFG_METHOD_11) {
		RTL_W32(tp, OCPDR, 0x01);
		RTL_W32(tp, OCPAR, 0x80001030);
		RTL_W32(tp, OCPAR, 0x0000F034);

		for (i = 1000; i > 0; i--) {
			if ((RTL_R32(tp, OCPAR) & 0xFFFF) == 0)
				break;
			delay(100);
		}
	}
}

static void rtl8168_hw_start(struct rtl8168_private *tp)
{
	u8 device_control, options1, options2;
	u16 ephy_data;
	u32 csi_tmp;
	struct pci_attach_args *pa = tp->pa;

	rtl8168_nic_reset(tp);

	RTL_W8(tp, Cfg9346, Cfg9346_Unlock);

	RTL_W8(tp, Reserved1, Reserved1_data);

	tp->cp_cmd |= PktCntrDisable | INTT_1;
	RTL_W16(tp, CPlusCmd, tp->cp_cmd);

	RTL_W16(tp, IntrMitigate, 0x5151);

	//Work around for RxFIFO overflow
	if (tp->mcfg == CFG_METHOD_1) {
		rtl8168_intr_mask |= RxFIFOOver | PCSTimeout;
		rtl8168_intr_mask &= ~RxDescUnavail;
	}

	RTL_W32(tp, TxDescStartAddrLow, ((u64) tp->TxPhyAddr & DMA_32BIT_MASK));
	RTL_W32(tp, TxDescStartAddrHigh, ((u64) tp->TxPhyAddr >> 32));
	RTL_W32(tp, RxDescAddrLow, ((u64) tp->RxPhyAddr & DMA_32BIT_MASK));
	RTL_W32(tp, RxDescAddrHigh, ((u64) tp->RxPhyAddr >> 32));

	/* Set Rx Config register */
	//rtl8168_set_rx_mode(tp);
	RTL_W32(tp, RxConfig, RTL_R32(tp, RxConfig) | (AcceptErr | AcceptRunt | AcceptBroadcast | AcceptMulticast
				| AcceptMyPhys | AcceptAllPhys));

	/* Set DMA burst size and Interframe Gap Time */
	if (tp->mcfg == CFG_METHOD_1) {
		RTL_W32(tp, TxConfig, (TX_DMA_BURST_512 << TxDMAShift) | 
				  (InterFrameGap << TxInterFrameGapShift));
	} else {
		RTL_W32(tp, TxConfig, (TX_DMA_BURST_unlimited << TxDMAShift) | 
				  (InterFrameGap << TxInterFrameGapShift));
	}

	/* Clear the interrupt status register. */
	RTL_W16(tp, IntrStatus, 0xFFFF);

	if (tp->rx_fifo_overflow == 0) {
		/* Enable all known interrupts by setting the interrupt mask. */
		RTL_W16(tp, IntrMask, rtl8168_intr_mask);
		//netif_start_queue(tp);  xiangy
	}

	if (tp->mcfg == CFG_METHOD_4) {
		/*set PCI configuration space offset 0x70F to 0x27*/
		/*When the register offset of PCI configuration space larger than 0xff, use CSI to access it.*/
		csi_tmp = rtl8168_csi_read(tp, 0x70c) & 0x00ffffff;
		rtl8168_csi_write(tp, 0x70c, csi_tmp | 0x27000000);

		RTL_W8(tp, DBG_reg, (0x0E << 4) | Fix_Nak_1 | Fix_Nak_2);

		/*Set EPHY registers	begin*/
		/*Set EPHY register offset 0x02 bit 11 to 0 and bit 12 to 1*/
		ephy_data = rtl8168_ephy_read(tp, 0x02);
		ephy_data &= ~(1 << 11);
		ephy_data |= (1 << 12);
		rtl8168_ephy_write(tp, 0x02, ephy_data);

		/*Set EPHY register offset 0x03 bit 1 to 1*/
		ephy_data = rtl8168_ephy_read(tp, 0x03);
		ephy_data |= (1 << 1);
		rtl8168_ephy_write(tp, 0x03, ephy_data);

		/*Set EPHY register offset 0x06 bit 7 to 0*/
		ephy_data = rtl8168_ephy_read(tp, 0x06);
		ephy_data &= ~(1 << 7);
		rtl8168_ephy_write(tp, 0x06, ephy_data);
		/*Set EPHY registers	end*/

		RTL_W8(tp, Config3, RTL_R8(tp, Config3) & ~Beacon_en);

		//disable clock request.
		_pci_conf_writen(pa->pa_tag, 0x81, 0x00, 1);

		RTL_W16(tp, CPlusCmd, RTL_R16(tp, CPlusCmd) & 
			~(EnableBist | Macdbgo_oe | Force_halfdup | Force_rxflow_en | Force_txflow_en | 
			  Cxpl_dbg_sel | ASF | PktCntrDisable | Macdbgo_sel));

#if 0
		if (tp->mtu > ETH_DATA_LEN) {
			RTL_W8(tp, Reserved1, Reserved1_data);
			RTL_W8(tp, Config3, RTL_R8(tp, Config3) | Jumbo_En0);
			RTL_W8(tp, Config4, RTL_R8(tp, Config4) | Jumbo_En1);

			//Set PCI configuration space offset 0x79 to 0x20
			/*Increase the Tx performance*/
			device_control = _pci_conf_readn(pa->pa_tag, 0x79, 1);
			device_control &= ~0x70;
			device_control |= 0x20;
			_pci_conf_writen(pa->pa_tag, 0x79, device_control, 1);

			//tx checksum offload disable
			tp->features &= ~NETIF_F_IP_CSUM;

			//rx checksum offload disable
			tp->cp_cmd &= ~RxChkSum;
			RTL_W16(tp, CPlusCmd, tp->cp_cmd);
		} else {
			RTL_W8(tp, Reserved1, Reserved1_data);
			RTL_W8(tp, Config3, RTL_R8(tp, Config3) & ~Jumbo_En0);
			RTL_W8(tp, Config4, RTL_R8(tp, Config4) & ~Jumbo_En1);

			//Set PCI configuration space offset 0x79 to 0x50
			/*Increase the Tx performance*/
			device_control = _pci_conf_readn(pa->pa_tag, 0x79, 1);
			device_control &= ~0x70;
			device_control |= 0x50;
			_pci_conf_writen(pa->pa_tag, 0x79, device_control, 1);

			//tx checksum offload enable
			tp->features |= NETIF_F_IP_CSUM;

			//rx checksum offload enable
			tp->cp_cmd |= RxChkSum;
			RTL_W16(tp, CPlusCmd, tp->cp_cmd);
		}
#endif
	} else if (tp->mcfg == CFG_METHOD_5) {
		/*set PCI configuration space offset 0x70F to 0x27*/
		/*When the register offset of PCI configuration space larger than 0xff, use CSI to access it.*/
		csi_tmp = rtl8168_csi_read(tp, 0x70c) & 0x00ffffff;
		rtl8168_csi_write(tp, 0x70c, csi_tmp | 0x27000000);

		/******set EPHY registers for RTL8168CP	begin******/
		//Set EPHY register offset 0x01 bit 0 to 1.
		ephy_data = rtl8168_ephy_read(tp, 0x01);
		ephy_data |= (1 << 0);
		rtl8168_ephy_write(tp, 0x01, ephy_data);

		//Set EPHY register offset 0x03 bit 10 to 0, bit 9 to 1 and bit 5 to 1.
		ephy_data = rtl8168_ephy_read(tp, 0x03);
		ephy_data &= ~(1 << 10);
		ephy_data |= (1 << 9);
		ephy_data |= (1 << 5);
		rtl8168_ephy_write(tp, 0x03, ephy_data);
		/******set EPHY registers for RTL8168CP	end******/

		RTL_W8(tp, Config3, RTL_R8(tp, Config3) & ~Beacon_en);

		//disable clock request.
		_pci_conf_writen(pa->pa_tag, 0x81, 0x00, 1);

		RTL_W16(tp, CPlusCmd, RTL_R16(tp, CPlusCmd) & 
			~(EnableBist | Macdbgo_oe | Force_halfdup | Force_rxflow_en | Force_txflow_en | 
			  Cxpl_dbg_sel | ASF | PktCntrDisable | Macdbgo_sel));
#if 0
		if (dev->mtu > ETH_DATA_LEN) {
			RTL_W8(tp, Reserved1, Reserved1_data);
			RTL_W8(tp, Config3, RTL_R8(tp, Config3) | Jumbo_En0);
			RTL_W8(tp, Config4, RTL_R8(tp, Config4) | Jumbo_En1);

			//Set PCI configuration space offset 0x79 to 0x20
			/*Increase the Tx performance*/
			device_control = pci_conf_readn(tp, 0x79, 1);
			device_control &= ~0x70;
			device_control |= 0x20;
			_pci_conf_writen(pa->pa_tag, 0x79, device_control, 1);

			//tx checksum offload disable
			dev->features &= ~NETIF_F_IP_CSUM;

			//rx checksum offload disable
			tp->cp_cmd &= ~RxChkSum;
			RTL_W16(tp, CPlusCmd, tp->cp_cmd);
		} else {
			RTL_W8(tp, Reserved1, Reserved1_data);
			RTL_W8(tp, Config3, RTL_R8(tp, Config3) & ~Jumbo_En0);
			RTL_W8(tp, Config4, RTL_R8(tp, Config4) & ~Jumbo_En1);

			//Set PCI configuration space offset 0x79 to 0x50
			/*Increase the Tx performance*/
			device_control = _pci_conf_readn(pa->pa_tag, 0x79, 1);
			device_control &= ~0x70;
			device_control |= 0x50;
			pci_conf_writen(tp, 0x79, device_control, 1);

			//tx checksum offload enable
			dev->features |= NETIF_F_IP_CSUM;

			//rx checksum offload enable
			tp->cp_cmd |= RxChkSum;
			RTL_W16(tp, CPlusCmd, tp->cp_cmd);
		}
#endif
	} else if (tp->mcfg == CFG_METHOD_6) {
		/*set PCI configuration space offset 0x70F to 0x27*/
		/*When the register offset of PCI configuration space larger than 0xff, use CSI to access it.*/
		csi_tmp = rtl8168_csi_read(tp, 0x70c) & 0x00ffffff;
		rtl8168_csi_write(tp, 0x70c, csi_tmp | 0x27000000);

		RTL_W8(tp, Config3, RTL_R8(tp, Config3) & ~Beacon_en);

		RTL_W16(tp, CPlusCmd, RTL_R16(tp, CPlusCmd) & 
			~(EnableBist | Macdbgo_oe | Force_halfdup | Force_rxflow_en | Force_txflow_en | 
			  Cxpl_dbg_sel | ASF | PktCntrDisable | Macdbgo_sel));
#if 0
		if (dev->mtu > ETH_DATA_LEN) {
			RTL_W8(tp, Reserved1, Reserved1_data);
			RTL_W8(tp, Config3, RTL_R8(tp, Config3) | Jumbo_En0);
			RTL_W8(tp, Config4, RTL_R8(tp, Config4) | Jumbo_En1);

			//Set PCI configuration space offset 0x79 to 0x20
			/*Increase the Tx performance*/
			device_control = _pci_conf_read(tp, 0x79, 1);
			device_control &= ~0x70;
			device_control |= 0x20;
			_pci_conf_writen(pa->pa_tag, 0x79, device_control, 1);

			//tx checksum offload disable
			dev->features &= ~NETIF_F_IP_CSUM;

			//rx checksum offload disable
			tp->cp_cmd &= ~RxChkSum;
			RTL_W16(tp, CPlusCmd, tp->cp_cmd);
		} else {
			RTL_W8(tp, Reserved1, Reserved1_data);
			RTL_W8(tp, Config3, RTL_R8(tp, Config3) & ~Jumbo_En0);
			RTL_W8(tp, Config4, RTL_R8(tp, Config4) & ~Jumbo_En1);

			//Set PCI configuration space offset 0x79 to 0x50
			/*Increase the Tx performance*/
			_pci_conf_readn(pa->pa_tag, 0x79, &device_control);
			device_control &= ~0x70;
			device_control |= 0x50;
			_pci_conf_writen(pa->pa_tag, 0x79, device_control);

			//tx checksum offload enable
			dev->features |= NETIF_F_IP_CSUM;

			//rx checksum offload enable
			tp->cp_cmd |= RxChkSum;
			RTL_W16(tp, CPlusCmd, tp->cp_cmd);
		}
#endif
	} else if (tp->mcfg == CFG_METHOD_7) {
		/*set PCI configuration space offset 0x70F to 0x27*/
		/*When the register offset of PCI configuration space larger than 0xff, use CSI to access it.*/
		csi_tmp = rtl8168_csi_read(tp, 0x70c) & 0x00ffffff;
		rtl8168_csi_write(tp, 0x70c, csi_tmp | 0x27000000);
		rtl8168_eri_write(tp, 0x1EC, 1, 0x07, ERIAR_ASF);

		RTL_W16(tp, CPlusCmd, RTL_R16(tp, CPlusCmd) & 
			~(EnableBist | Macdbgo_oe | Force_halfdup | Force_rxflow_en | Force_txflow_en | 
			  Cxpl_dbg_sel | ASF | PktCntrDisable | Macdbgo_sel));

		RTL_W8(tp, Config3, RTL_R8(tp, Config3) & ~Beacon_en);
#if 0
		if (dev->mtu > ETH_DATA_LEN) {
			RTL_W8(tp, Reserved1, Reserved1_data);
			RTL_W8(tp, Config3, RTL_R8(tp, Config3) | Jumbo_En0);
			RTL_W8(tp, Config4, RTL_R8(tp, Config4) | Jumbo_En1);

			//Set PCI configuration space offset 0x79 to 0x20
			/*Increase the Tx performance*/
			_pci_conf_readn(pa->pa_tag, 0x79, &device_control);
			device_control &= ~0x70;
			device_control |= 0x20;
			_pci_conf_writen(pa->pa_tag, 0x79, device_control);

			//tx checksum offload disable
			dev->features &= ~NETIF_F_IP_CSUM;

			//rx checksum offload disable
			tp->cp_cmd &= ~RxChkSum;
			RTL_W16(tp, CPlusCmd, tp->cp_cmd);
		} else {
			RTL_W8(tp, Reserved1, Reserved1_data);
			RTL_W8(tp, Config3, RTL_R8(tp, Config3) & ~Jumbo_En0);
			RTL_W8(tp, Config4, RTL_R8(tp, Config4) & ~Jumbo_En1);

			//Set PCI configuration space offset 0x79 to 0x50
			/*Increase the Tx performance*/
			_pci_conf_readn(pa->pa_tag, 0x79, &device_control);
			device_control &= ~0x70;
			device_control |= 0x50;
			_pci_conf_writen(pa->pa_tag, 0x79, device_control);

			//tx checksum offload enable
			dev->features |= NETIF_F_IP_CSUM;

			//rx checksum offload enable
			tp->cp_cmd |= RxChkSum;
			RTL_W16(tp, CPlusCmd, tp->cp_cmd);
		}
#endif
	} else if (tp->mcfg == CFG_METHOD_8) {
		/*set PCI configuration space offset 0x70F to 0x27*/
		/*When the register offset of PCI configuration space larger than 0xff, use CSI to access it.*/
		csi_tmp = rtl8168_csi_read(tp, 0x70c) & 0x00ffffff;
		rtl8168_csi_write(tp, 0x70c, csi_tmp | 0x27000000);
		rtl8168_eri_write(tp, 0x1EC, 1, 0x07, ERIAR_ASF);

		RTL_W16(tp, CPlusCmd, RTL_R16(tp, CPlusCmd) & 
			~(EnableBist | Macdbgo_oe | Force_halfdup | Force_rxflow_en | Force_txflow_en | 
			  Cxpl_dbg_sel | ASF | PktCntrDisable | Macdbgo_sel));

		RTL_W8(tp, Config3, RTL_R8(tp, Config3) & ~Beacon_en);

		RTL_W8(tp, 0xD1, 0x20);
#if 0
		if (dev->mtu > ETH_DATA_LEN) {
			RTL_W8(tp, Reserved1, Reserved1_data);
			RTL_W8(tp, Config3, RTL_R8(tp, Config3) | Jumbo_En0);
			RTL_W8(tp, Config4, RTL_R8(tp, Config4) | Jumbo_En1);

			//Set PCI configuration space offset 0x79 to 0x20
			/*Increase the Tx performance*/
			_pci_conf_readn(pa->pa_tag, 0x79, &device_control);
			device_control &= ~0x70;
			device_control |= 0x20;
			_pci_conf_writen(pa->pa_tag, 0x79, device_control);

			//tx checksum offload disable
			dev->features &= ~NETIF_F_IP_CSUM;

			//rx checksum offload disable
			tp->cp_cmd &= ~RxChkSum;
			RTL_W16(tp, CPlusCmd, tp->cp_cmd);
		} else {
			RTL_W8(tp, Reserved1, Reserved1_data);
			RTL_W8(tp, Config3, RTL_R8(tp, Config3) & ~Jumbo_En0);
			RTL_W8(tp, Config4, RTL_R8(tp, Config4) & ~Jumbo_En1);

			//Set PCI configuration space offset 0x79 to 0x50
			/*Increase the Tx performance*/
			_pci_conf_readn(pa->pa_tag, 0x79, &device_control);
			device_control &= ~0x70;
			device_control |= 0x50;
			_pci_conf_writen(pa->pa_tag, 0x79, device_control);

			//tx checksum offload enable
			dev->features |= NETIF_F_IP_CSUM;

			//rx checksum offload enable
			tp->cp_cmd |= RxChkSum;
			RTL_W16(tp, CPlusCmd, tp->cp_cmd);
		}
#endif
	} else if (tp->mcfg == CFG_METHOD_9) {
		/*set PCI configuration space offset 0x70F to 0x13*/
		/*When the register offset of PCI configuration space larger than 0xff, use CSI to access it.*/
		csi_tmp = rtl8168_csi_read(tp, 0x70c) & 0x00ffffff;
		rtl8168_csi_write(tp, 0x70c, csi_tmp | 0x13000000);

		/* disable clock request. */
		_pci_conf_writen(pa->pa_tag, 0x81, 0x00, 1);

		RTL_W8(tp, Config3, RTL_R8(tp, Config3) & ~BIT_4);
		RTL_W8(tp, DBG_reg, RTL_R8(tp, DBG_reg) | BIT_7 | BIT_1);
#if 0
		if (dev->mtu > ETH_DATA_LEN) {
			RTL_W8(tp, Reserved1, Reserved1_data);
			RTL_W8(tp, Config3, RTL_R8(tp, Config3) | Jumbo_En0);
			RTL_W8(tp, Config4, RTL_R8(tp, Config4) | Jumbo_En1);

			/* Set PCI configuration space offset 0x79 to 0x20 */
			_pci_conf_readn(pa->pa_tag, 0x79, &device_control);
			device_control &= ~0x70;
			device_control |= 0x20;
			_pci_conf_writen(pa->pa_tag, 0x79, device_control);

			/* tx checksum offload disable */
			dev->features &= ~NETIF_F_IP_CSUM;

			/* rx checksum offload disable */
			tp->cp_cmd &= ~RxChkSum;
			RTL_W16(tp, CPlusCmd, tp->cp_cmd);
		} else {
			RTL_W8(tp, Reserved1, Reserved1_data);
			RTL_W8(tp, Config3, RTL_R8(tp, Config3) & ~Jumbo_En0);
			RTL_W8(tp, Config4, RTL_R8(tp, Config4) & ~Jumbo_En1);

			/* Set PCI configuration space offset 0x79 to 0x50 */
			_pci_conf_readn(pa->pa_tag, 0x79, &device_control);
			device_control &= ~0x70;
			device_control |= 0x50;
			_pci_conf_writen(pa->pa_tag, 0x79, device_control);

			/* tx checksum offload enable */
			dev->features |= NETIF_F_IP_CSUM;

			/* rx checksum offload enable */
			tp->cp_cmd |= RxChkSum;
			RTL_W16(tp, CPlusCmd, tp->cp_cmd);
		}
#endif
		/* set EPHY registers */
		rtl8168_ephy_write(tp, 0x01, 0x7C7D);
		rtl8168_ephy_write(tp, 0x02, 0x091F);
		rtl8168_ephy_write(tp, 0x06, 0xB271);
		rtl8168_ephy_write(tp, 0x07, 0xCE00);
	} else if (tp->mcfg == CFG_METHOD_10) {
		/*set PCI configuration space offset 0x70F to 0x13*/
		/*When the register offset of PCI configuration space larger than 0xff, use CSI to access it.*/
		csi_tmp = rtl8168_csi_read(tp, 0x70c) & 0x00ffffff;
		rtl8168_csi_write(tp, 0x70c, csi_tmp | 0x13000000);

		RTL_W8(tp, DBG_reg, RTL_R8(tp, DBG_reg) | BIT_7 | BIT_1);
#if 0
		if (dev->mtu > ETH_DATA_LEN) {
			RTL_W8(tp, Reserved1, Reserved1_data);
			RTL_W8(tp, Config3, RTL_R8(tp, Config3) | Jumbo_En0);
			RTL_W8(tp, Config4, RTL_R8(tp, Config4) | Jumbo_En1);

			/* Set PCI configuration space offset 0x79 to 0x20 */
			_pci_conf_readn(pa->pa_tag, 0x79, &device_control);
			device_control &= ~0x70;
			device_control |= 0x20;
			_pci_conf_writen(pa->pa_tag, 0x79, device_control);

			/* tx checksum offload disable */
			dev->features &= ~NETIF_F_IP_CSUM;

			/* rx checksum offload disable */
			tp->cp_cmd &= ~RxChkSum;
			RTL_W16(tp, CPlusCmd, tp->cp_cmd);
		} else {
			RTL_W8(tp, Reserved1, Reserved1_data);
			RTL_W8(tp, Config3, RTL_R8(tp, Config3) & ~Jumbo_En0);
			RTL_W8(tp, Config4, RTL_R8(tp, Config4) & ~Jumbo_En1);

			/* Set PCI configuration space offset 0x79 to 0x50 */
			_pci_conf_readn(pa->pa_tag, 0x79, &device_control);
			device_control &= ~0x70;
			device_control |= 0x50;
			_pci_conf_writen(pa->pa_tag, 0x79, device_control);

			/* tx checksum offload enable */
			dev->features |= NETIF_F_IP_CSUM;

			/* rx checksum offload enable */
			tp->cp_cmd |= RxChkSum;
			RTL_W16(tp, CPlusCmd, tp->cp_cmd);
		}
#endif
		RTL_W8(tp, Config1, RTL_R8(tp, Config1) | 0x0F);

		/* set EPHY registers */
		rtl8168_ephy_write(tp, 0x01, 0x6C7F);
		rtl8168_ephy_write(tp, 0x02, 0x011F);
		rtl8168_ephy_write(tp, 0x03, 0xC1B2);
		rtl8168_ephy_write(tp, 0x1A, 0x0546);
		rtl8168_ephy_write(tp, 0x1C, 0x80C4);
		rtl8168_ephy_write(tp, 0x1D, 0x78E4);
		rtl8168_ephy_write(tp, 0x0A, 0x8100);

		/* disable clock request. */
		_pci_conf_writen(pa->pa_tag, 0x81, 0x00, 1);

		RTL_W8(tp, 0xF3, RTL_R8(tp, 0xF3) | (1 << 2));

	} else if (tp->mcfg == CFG_METHOD_11) {
		/*set PCI configuration space offset 0x70F to 0x37*/
		/*When the register offset of PCI configuration space larger than 0xff, use CSI to access it.*/
		csi_tmp = rtl8168_csi_read(tp, 0x70c) & 0x00ffffff;
		rtl8168_csi_write(tp, 0x70c, csi_tmp | 0x37000000);

		/* Set PCI configuration space offset 0x79 to 0x50 */
		device_control = _pci_conf_readn(pa->pa_tag, 0x79, 1);
		device_control &= ~0x70;
		device_control |= 0x50;
		_pci_conf_writen(pa->pa_tag, 0x79, device_control, 1);
#if 0
		if (dev->mtu > ETH_DATA_LEN) {
			RTL_W8(tp, Reserved1, Reserved1_data);
			RTL_W8(tp, Config3, RTL_R8(tp, Config3) | Jumbo_En0);

			/* tx checksum offload disable */
			dev->features &= ~NETIF_F_IP_CSUM;

			/* rx checksum offload disable */
			tp->cp_cmd &= ~RxChkSum;
			RTL_W16(tp, CPlusCmd, tp->cp_cmd);
		} else {
			RTL_W8(tp, Reserved1, Reserved1_data);
			RTL_W8(tp, Config3, RTL_R8(tp, Config3) & ~Jumbo_En0);

			/* tx checksum offload enable */
			dev->features |= NETIF_F_IP_CSUM;

			/* rx checksum offload enable */
			tp->cp_cmd |= RxChkSum;
			RTL_W16(tp, CPlusCmd, tp->cp_cmd);
		}
#endif
		//RTL_W8(tp, Config1, 0xDF);
		RTL_W8(tp, Config1, RTL_R8(tp, Config1) | 0x0F);

	} else if (tp->mcfg == CFG_METHOD_1) {
		RTL_W8(tp, Config3, RTL_R8(tp, Config3) & ~Beacon_en);

		RTL_W16(tp, CPlusCmd, RTL_R16(tp, CPlusCmd) & 
			~(EnableBist | Macdbgo_oe | Force_halfdup | Force_rxflow_en | Force_txflow_en | 
			  Cxpl_dbg_sel | ASF | PktCntrDisable | Macdbgo_sel));
#if 0
		if (dev->mtu > ETH_DATA_LEN) {
			_pci_conf_readn(pa->pa_tag, 0x69, &device_control);
			device_control &= ~0x70;
			device_control |= 0x28;
			_pci_conf_writen(pa->pa_tag, 0x69, device_control);
		} else {
			_pci_conf_readn(pa->pa_tag, 0x69, &device_control);
			device_control &= ~0x70;
			device_control |= 0x58;
			_pci_conf_writen(pa->pa_tag, 0x69, device_control);
		}
#endif
	} else if (tp->mcfg == CFG_METHOD_2) {
		RTL_W8(tp, Config3, RTL_R8(tp, Config3) & ~Beacon_en);

		RTL_W16(tp, CPlusCmd, RTL_R16(tp, CPlusCmd) & 
			~(EnableBist | Macdbgo_oe | Force_halfdup | Force_rxflow_en | Force_txflow_en | 
			  Cxpl_dbg_sel | ASF | PktCntrDisable | Macdbgo_sel));
#if 0
		if (dev->mtu > ETH_DATA_LEN) {
			_pci_conf_readn(pa->pa_tag, 0x69, &device_control);
			device_control &= ~0x70;
			device_control |= 0x28;
			_pci_conf_writen(pa->pa_tag, 0x69, device_control);

			RTL_W8(tp, Reserved1, Reserved1_data);
			RTL_W8(tp, Config4, RTL_R8(tp, Config4) | (1 << 0));
		} else {
			_pci_conf_readn(pa->pa_tag, 0x69, &device_control);
			device_control &= ~0x70;
			device_control |= 0x58;
			_pci_conf_writen(pa->pa_tag, 0x69, device_control);

			RTL_W8(tp, Reserved1, Reserved1_data);
			RTL_W8(tp, Config4, RTL_R8(tp, Config4) & ~(1 << 0));
		}
#endif
	} else if (tp->mcfg == CFG_METHOD_3) {
		RTL_W8(tp, Config3, RTL_R8(tp, Config3) & ~Beacon_en);

		RTL_W16(tp, CPlusCmd, RTL_R16(tp, CPlusCmd) & 
			~(EnableBist | Macdbgo_oe | Force_halfdup | Force_rxflow_en | Force_txflow_en | 
			  Cxpl_dbg_sel | ASF | PktCntrDisable | Macdbgo_sel));
#if 0
		if (dev->mtu > ETH_DATA_LEN) {
			_pci_conf_readn(pa->pa_tag, 0x69, &device_control);
			device_control &= ~0x70;
			device_control |= 0x28;
			_pci_conf_writen(pa->pa_tag, 0x69, device_control);

			RTL_W8(tp, Reserved1, Reserved1_data);
			RTL_W8(tp, Config4, RTL_R8(tp, Config4) | (1 << 0));
		} else {
			_pci_conf_readn(pa->pa_tag, 0x69, &device_control);
			device_control &= ~0x70;
			device_control |= 0x58;
			_pci_conf_writen(pa->pa_tag, 0x69, device_control);

			RTL_W8(tp, Reserved1, Reserved1_data);
			RTL_W8(tp, Config4, RTL_R8(tp, Config4) & ~(1 << 0));
		}
#endif
	}
	if ((tp->mcfg == CFG_METHOD_1) || (tp->mcfg == CFG_METHOD_2) || (tp->mcfg == CFG_METHOD_3)) {
		/* csum offload command for RTL8168B/8111B */
		tp->tx_tcp_csum_cmd = TxIPCS | TxTCPCS;
		tp->tx_udp_csum_cmd = TxIPCS | TxUDPCS;
		tp->tx_ip_csum_cmd = TxIPCS;
	} else {
		/* csum offload command for RTL8168C/8111C and RTL8168CP/8111CP */
		tp->tx_tcp_csum_cmd = TxIPCS_C | TxTCPCS_C;
		tp->tx_udp_csum_cmd = TxIPCS_C | TxUDPCS_C;
		tp->tx_ip_csum_cmd = TxIPCS_C;
	}

	RTL_W8(tp, ChipCmd, CmdTxEnb | CmdRxEnb);

	RTL_W8(tp, Cfg9346, Cfg9346_Lock);
#define PCI_CACHE_LINE_SIZE 0x0c    /* 8 bits */
#define PCI_BASE_ADDRESS_0  0x10    /* 32 bits */
#define PCI_BASE_ADDRESS_1  0x14    /* 32 bits [htype 0,1 only] */
#define PCI_BASE_ADDRESS_2  0x18    /* 32 bits [htype 0 only] */
#define PCI_BASE_ADDRESS_3  0x1c    /* 32 bits */
#define PCI_BASE_ADDRESS_4  0x20    /* 32 bits */
#define PCI_BASE_ADDRESS_5  0x24    /* 32 bits */
#define PCI_INTERRUPT_LINE  0x3c    /* 8 bits */

#if 1
	if (!tp->pci_cfg_is_read) {
		tp->pci_cfg_space.cmd = _pci_conf_readn(pa->pa_tag, PCI_COMMAND_STATUS_REG, 1);
		tp->pci_cfg_space.cls = _pci_conf_readn(pa->pa_tag, PCI_CACHE_LINE_SIZE, 1);
		tp->pci_cfg_space.io_base_l = _pci_conf_readn(pa->pa_tag, PCI_BASE_ADDRESS_0, 2);
		tp->pci_cfg_space.io_base_h = _pci_conf_readn(pa->pa_tag, PCI_BASE_ADDRESS_0 + 2, 2);
		tp->pci_cfg_space.mem_base_l = _pci_conf_readn(pa->pa_tag, PCI_BASE_ADDRESS_2, 2);
		tp->pci_cfg_space.mem_base_h = _pci_conf_readn(pa->pa_tag, PCI_BASE_ADDRESS_2 + 2, 2);
		tp->pci_cfg_space.ilr - _pci_conf_readn(pa->pa_tag, PCI_INTERRUPT_LINE, 2);
		tp->pci_cfg_space.resv_0x20_l = _pci_conf_readn(pa->pa_tag, PCI_BASE_ADDRESS_4, 2);
		tp->pci_cfg_space.resv_0x20_h = _pci_conf_readn(pa->pa_tag, PCI_BASE_ADDRESS_4 + 2, 2);
		tp->pci_cfg_space.resv_0x24_l = _pci_conf_readn(pa->pa_tag, PCI_BASE_ADDRESS_5, 2);
		tp->pci_cfg_space.resv_0x24_h = _pci_conf_readn(pa->pa_tag, PCI_BASE_ADDRESS_5 + 2, 2);

		tp->pci_cfg_is_read = 1;
	}
#endif

	rtl8168_dsm(tp, DSM_MAC_INIT);

	options1 = RTL_R8(tp, Config3);
	options2 = RTL_R8(tp, Config5);
	if ((options1 & LinkUp) || (options1 & MagicPacket) || (options2 & UWF) || (options2 & BWF) || (options2 & MWF))
		tp->wol_enabled = WOL_ENABLED;
	else
		tp->wol_enabled = WOL_DISABLED;
	delay(100);
}


static void rtl8168_set_rx_mode(struct rtl8168_private *tp)
{
	u32 mc_filter[2];	/* Multicast hash filter */
	int rx_mode;
	u32 tmp = 0;

	if (tp->flags & IFF_PROMISC) {
		/* Unconditionally log net taps. */
		printf("Promiscuous mode enabled.\n");
		rx_mode =
		    AcceptBroadcast | AcceptMulticast | AcceptMyPhys |
		    AcceptAllPhys;
		mc_filter[1] = mc_filter[0] = 0xffffffff;
	} else if ((tp->mc_count > multicast_filter_limit)
		   || (tp->flags & IFF_ALLMULTI)) {
		/* Too many to filter perfectly -- accept all multicasts. */
		rx_mode = AcceptBroadcast | AcceptMulticast | AcceptMyPhys;
		mc_filter[1] = mc_filter[0] = 0xffffffff;
	} else {
		//struct dev_mc_list *mclist;
		rx_mode = AcceptBroadcast | AcceptMyPhys;
		mc_filter[1] = mc_filter[0] = 0;
	}

	tmp = tp->rtl8168_rx_config | rx_mode | (RTL_R32(tp, RxConfig) & rtl_chip_info[tp->chipset].RxConfigMask);

	RTL_W32(tp, RxConfig, tmp);

	RTL_W32(tp, MAR0 + 0, mc_filter[0]);
	RTL_W32(tp, MAR0 + 4, mc_filter[1]);

}

static void rtl8168_check_link_status(struct rtl8168_private *tp)
{
    int i; 
    int old_value;

	if (tp->link_ok(tp)) {
		printf("r8110: link up :  ");
        printf("PHY status:0x%x\n",RTL_R8(tp,0x6c));
        if(RTL_R8(tp,0x6c) == if_frequency){
            return;
        }
	} else {
		printf("r8110: link down\n");
        //printf("PHY status:0x%x\n",RTL_R8(tp,0x6c));
	}
#if 0
    //by liuqi we try to get the 8110 up
    if (if_in_attach != 1){
 	   for (i = 0; i<10; i++){
    	    printf("Trying the %d time--->\n", i);

	    	old_value = mdio_read(tp, PHY_CTRL_REG);
	        old_value |= 0x0200;       //set to re auto negotiation
    	    mdio_write(tp, PHY_CTRL_REG, old_value);
        	delay(10000000);
	        if(RTL_R8(tp,0x6c) == if_frequency){
    	        printf("Connect done!!\n");
        	    break; 
        	}
    	}
    }
#endif

    rtl8168dp_10mbps_gphy_para(tp);

}


static void rtl8168dp_10mbps_gphy_para(struct rtl8168_private *tp)
{
    u8 status = RTL_R8(tp, PHYstatus);

    if (tp->mcfg != CFG_METHOD_11)
        return;

    if ((status & LinkStatus) && (status & _10bps)) {
        mdio_write(tp, 0x1f, 0x0000);
        mdio_write(tp, 0x10, 0x04EE);
    } else {
        mdio_write(tp, 0x1f, 0x0000);
        mdio_write(tp, 0x10, 0x01EE);
    }
}


static void rtl8168_ephy_write(struct rtl8168_private *tp, int RegAddr, int value)
{
	int i;

	RTL_W32(tp, EPHYAR, 
		EPHYAR_Write | 
		(RegAddr & EPHYAR_Reg_Mask) << EPHYAR_Reg_shift | 
		(value & EPHYAR_Data_Mask));

	for (i = 0; i < 10; i++) {
		udelay(100);

		/* Check if the RTL8168 has completed EPHY write */
		if (!(RTL_R32(tp, EPHYAR) & EPHYAR_Flag)) 
			break;
	}

	udelay(20);
}

static u16
rtl8168_ephy_read(struct rtl8168_private * tp, int RegAddr)
{
	int i;
	u16 value = 0xffff;

	RTL_W32(tp, EPHYAR, 
		EPHYAR_Read | (RegAddr & EPHYAR_Reg_Mask) << EPHYAR_Reg_shift);

	for (i = 0; i < 10; i++) {
		udelay(100);

		/* Check if the RTL8168 has completed EPHY read */
		if (RTL_R32(tp, EPHYAR) & EPHYAR_Flag) {
			value = (u16) (RTL_R32(tp, EPHYAR) & EPHYAR_Data_Mask);
			break;
		}
	}

	udelay(100);

	return value;
}

static void
rtl8168_csi_write(struct rtl8168_private * tp, 
		   int addr,
		   int value)
{
	int i;

	RTL_W32(tp, CSIDR, value);
	RTL_W32(tp, CSIAR, 
		CSIAR_Write |
		CSIAR_ByteEn << CSIAR_ByteEn_shift |
		(addr & CSIAR_Addr_Mask));

	for (i = 0; i < 10; i++) {
		udelay(100);

		/* Check if the RTL8168 has completed CSI write */
		if (!(RTL_R32(tp, CSIAR) & CSIAR_Flag)) 
			break;
	}

	udelay(20);
}

int 
rtl8168_eri_read(struct rtl8168_private * tp, 
		 int addr,
		 int len,
		 int type)
{
	int i, val_shift, shift = 0;
	u32 value1 = 0, value2 = 0, mask;

	if (len > 4 || len <= 0)
		return -1;

	while (len > 0) {
		val_shift = addr % ERIAR_Addr_Align;
		addr = addr & ~0x3;

		RTL_W32(tp, ERIAR, 
			ERIAR_Read |
			type << ERIAR_Type_shift |
			ERIAR_ByteEn << ERIAR_ByteEn_shift |
			addr);

		for (i = 0; i < 10; i++) {
			udelay(100);

			/* Check if the RTL8168 has completed ERI read */
			if (RTL_R32(tp, ERIAR) & ERIAR_Flag) 
				break;
		}

		if (len == 1)		mask = (0xFF << (val_shift * 8)) & 0xFFFFFFFF;
		else if (len == 2)	mask = (0xFFFF << (val_shift * 8)) & 0xFFFFFFFF;
		else if (len == 3)	mask = (0xFFFFFF << (val_shift * 8)) & 0xFFFFFFFF;
		else			mask = (0xFFFFFFFF << (val_shift * 8)) & 0xFFFFFFFF;

		value1 = RTL_R32(tp, ERIDR) & mask;
		value2 |= (value1 >> val_shift * 8) << shift * 8;

		if (len <= 4 - val_shift)
			len = 0;
		else {
			len -= (4 - val_shift);
			shift = 4 - val_shift;
			addr += 4;
		}
	}

	return value2;
}

int
rtl8168_eri_write(struct rtl8168_private * tp, 
		  int addr,
		  int len,
		  int value,
		  int type)
{

	int i, val_shift, shift = 0;
	u32 value1 = 0, mask;

	if (len > 4 || len <= 0)
		return -1;

	while(len > 0) {
		val_shift = addr % ERIAR_Addr_Align;
		addr = addr & ~0x3;

		if (len == 1)		mask = (0xFF << (val_shift * 8)) & 0xFFFFFFFF;
		else if (len == 2)	mask = (0xFFFF << (val_shift * 8)) & 0xFFFFFFFF;
		else if (len == 3)	mask = (0xFFFFFF << (val_shift * 8)) & 0xFFFFFFFF;
		else			mask = (0xFFFFFFFF << (val_shift * 8)) & 0xFFFFFFFF;

		value1 = rtl8168_eri_read(tp, addr, 4, type) & ~mask;
		value1 |= (((value << val_shift * 8) >> shift * 8));

		RTL_W32(tp, ERIDR, value1);
		RTL_W32(tp, ERIAR, 
			ERIAR_Write |
			type << ERIAR_Type_shift |
			ERIAR_ByteEn << ERIAR_ByteEn_shift |
			addr);

		for (i = 0; i < 10; i++) {
			udelay(100);

			/* Check if the RTL8168 has completed ERI write */
			if (!(RTL_R32(tp, ERIAR) & ERIAR_Flag)) 
				break;
		}

		if (len <= 4 - val_shift)
			len = 0;
		else {
			len -= (4 - val_shift);
			shift = 4 - val_shift;
			addr += 4;
		}
	}

	return 0;
}

static int rtl8168_csi_read(struct rtl8168_private * tp,int addr)
{
	int i, value = -1;

	RTL_W32(tp, CSIAR, 
		CSIAR_Read | 
		CSIAR_ByteEn << CSIAR_ByteEn_shift |
		(addr & CSIAR_Addr_Mask));

	for (i = 0; i < 10; i++) {
		udelay(100);

		/* Check if the RTL8168 has completed CSI read */
		if (RTL_R32(tp, CSIAR) & CSIAR_Flag) {
			value = (int)RTL_R32(tp, CSIDR);
			break;
		}
	}

	udelay(20);

	return value;
}

static void
rtl8168_dsm(struct rtl8168_private *tp, int dev_state)
{
	switch (dev_state) {
	case DSM_MAC_INIT:
		if ((tp->mcfg == CFG_METHOD_5) || (tp->mcfg == CFG_METHOD_6)) {
			if (RTL_R8(tp, MACDBG) & 0x80) {
				RTL_W8(tp, GPIO, RTL_R8(tp, GPIO) | GPIO_en);
			} else {
				RTL_W8(tp, GPIO, RTL_R8(tp, GPIO) & ~GPIO_en);
			}
		}

		break;
	case DSM_NIC_GOTO_D3:
	case DSM_IF_DOWN:
		if ((tp->mcfg == CFG_METHOD_5) || (tp->mcfg == CFG_METHOD_6))
			if (RTL_R8(tp, MACDBG) & 0x80)
				RTL_W8(tp, GPIO, RTL_R8(tp, GPIO) & ~GPIO_en);

		break;
	case DSM_NIC_RESUME_D3:
	case DSM_IF_UP:
		if ((tp->mcfg == CFG_METHOD_5) || (tp->mcfg == CFG_METHOD_6))
			if (RTL_R8(tp, MACDBG) & 0x80)
				RTL_W8(tp, GPIO, RTL_R8(tp, GPIO) | GPIO_en);

		break;
	}
}

static int rtl8168_interrupt(void *arg)
{
	struct rtl8168_private *tp = (struct rtl8168_private *)arg; 
	int boguscnt = max_interrupt_work;
	u32 status = 0;
	int handled = 0;
    u16 intr_clean_mask = SYSErr | PCSTimeout | SWInt | LinkChg | RxDescUnavail	| TxErr | TxOK | RxErr | RxOK;

	/* FIXME: We should not get an error interrupt here. */
	if (RTL_R16(tp, IntrStatus) == 0x80)
		return 0;

	do {
		status = RTL_R16(tp, IntrStatus);

		/* hotplug/major error/no more work/shared irq */
		if ((status == 0xFFFF) || !status){
			break;
		}

		handled = 1;

		status &= tp->intr_mask;
		RTL_W16(tp, IntrStatus,	intr_clean_mask);

		if (!(status & rtl8168_intr_mask)){
			break;
		}

		if ((status & SYSErr)) { 
			rtl8168_pcierr_interrupt(tp);
			break;
		}

		/* Rx interrupt */
		if (status & (RxOK | RxDescUnavail | RxFIFOOver)) {
			rtl8168_rx_interrupt(tp);
		}

		/* Tx interrupt */		
		if (status & (TxOK | TxErr)){
			rtl8168_tx_interrupt(tp);
		}

		boguscnt--;
	} while (boguscnt > 0);

	if (boguscnt <= 0) {
		/* Clear all interrupt sources. */
		RTL_W16(tp, IntrStatus, 0xffff);
	}
	return (handled);
}

static void rtl8168_pcierr_interrupt(struct rtl8168_private *tp)
{
	u16 pci_status, pci_cmd;
	u32 tmp;
	struct pci_attach_args *pa = tp->pa;

	tmp = _pci_conf_readn(pa->pa_tag, PCI_COMMAND_STATUS_REG,4);
	pci_cmd = tmp & 0xffff; 
	pci_status = tmp & 0xffff0000;

	/*
	 * The recovery sequence below admits a very elaborated explanation:
	 * - it seems to work;
	 * - I did not see what else could be done.
	 *
	 * Feel free to adjust to your needs.
	 */
	_pci_conf_writen(pa->pa_tag, PCI_COMMAND_STATUS_REG,
			      pci_cmd | PCI_COMMAND_SERR_ENABLE | PCI_COMMAND_PARITY_ENABLE, 2);

	pci_status &= (PCI_STATUS_PARITY_ERROR |
		PCI_STATUS_SPECIAL_ERROR | PCI_STATUS_MASTER_ABORT |
		PCI_STATUS_TARGET_TARGET_ABORT| PCI_STATUS_MASTER_TARGET_ABORT);
	pci_status >>= 16;

	_pci_conf_writen(pa->pa_tag, PCI_COMMAND_STATUS_REG + 2, pci_status, 2);

	rtl8168_hw_reset(tp);
}

static int rtl8168_rx_interrupt(struct rtl8168_private *tp)
{
	unsigned int cur_rx, rx_left;
	unsigned int delta, count;
	struct ether_header *eh;
	struct  ifnet *ifp = &tp->arpcom.ac_if;
	u32 status = 0;
	struct RxDesc *desc;

	cur_rx = tp->cur_rx;
	rx_left = NUM_RX_DESC + tp->dirty_rx - cur_rx;     

	for (; rx_left > 0; rx_left--, cur_rx++) {
		unsigned int entry = cur_rx % NUM_RX_DESC;
		desc = &(tp->RxDescArray[entry]);
		status = 0;

		status = le32_to_cpu(desc->opts1);

		if (status & DescOwn)
			break;

  		if ((status & RxRES)) {
			printf( "Rx ERROR. status = %08x\n", status);
			tp->stats.rx_errors++;
			if (status & (RxRWT | RxRUNT))
				tp->stats.rx_length_errors++;
			if (status & RxCRC)
				tp->stats.rx_crc_errors++;
			rtl8168_mark_to_asic(desc, tp->rx_buf_sz, 1);
		} else {
			struct mbuf *m;
			int pkt_size = (status & 0x00001FFF) - 4;

			/*
			 * The driver does not support incoming fragmented
			 * frames. They are seen as a symptom of over-mtu
			 * sized frames.
			 */
			if ((status & (FirstFrag|LastFrag)) != (FirstFrag|LastFrag)) {
    				tp->stats.rx_dropped++;
				tp->stats.rx_length_errors++;
				rtl8168_mark_to_asic(desc, tp->rx_buf_sz, 1);
				continue;
			}

			//rtl8168_rx_csum(tp, desc); //FIXME

			m = getmbuf(tp);
			if(m == NULL) {
				printf("rx_interrupt getmbuf failed\n");
				return -1;
			}

			bcopy(tp->rx_buffer[entry], mtod(m, caddr_t), pkt_size);

			m->m_pkthdr.rcvif = ifp;
			m->m_pkthdr.len = m->m_len = pkt_size - sizeof(struct ether_header);

			eh = mtod(m, struct ether_header *);

			m->m_data += sizeof(struct ether_header);

			ether_input(ifp, eh, m);

			tp->stats.rx_bytes += pkt_size;
			tp->stats.rx_packets++;

			delta = rtl8168_rx_fill(tp,  entry, entry + 1, 1);//should this work be done here ? lihui.
			tp->dirty_rx += delta; //FIXME
		}
	}

	count = cur_rx - tp->cur_rx;
	tp->cur_rx = cur_rx;

	/*
	 * FIXME: until there is periodic timer to try and refill the ring,
	 * a temporary shortage may definitely kill the Rx process.
	 * - disable the asic to try and avoid an overflow and kick it again
	 *   after refill ?
	 * - how do others driver handle this condition (Uh oh...).
	 */
	if ((tp->dirty_rx + NUM_RX_DESC) == tp->cur_rx)
		printf("Rx buffers exhausted\n");

	return count;
}

#define CHECKSUM_UNNECESSARY 0x01
#define CHECKSUM_NONE        0x02

static int rtl8168_rx_csum(struct rtl8168_private *tp,struct RxDesc *desc)
{
    u32 opts1 = le32_to_cpu(desc->opts1);
    u32 opts2 = le32_to_cpu(desc->opts2);
    u32 status = opts1 & RxProtoMask;

    if ((tp->mcfg == CFG_METHOD_1) ||
        (tp->mcfg == CFG_METHOD_2) ||
        (tp->mcfg == CFG_METHOD_3)) {
        /* rx csum offload for RTL8168B/8111B */
        if (((status == RxProtoTCP) && !(opts1 & RxTCPF)) ||
            ((status == RxProtoUDP) && !(opts1 & RxUDPF)) ||
            ((status == RxProtoIP) && !(opts1 & RxIPF)))
            return  CHECKSUM_UNNECESSARY;
        else
            return CHECKSUM_NONE;
    } else {
        /* rx csum offload for RTL8168C/8111C and RTL8168CP/8111CP */
        if (((status == RxTCPT) && !(opts1 & RxTCPF)) ||
            ((status == RxUDPT) && !(opts1 & RxUDPF)) ||
            ((status == 0) && (opts2 & RxV4F) && !(opts1 & RxIPF)))
            return CHECKSUM_UNNECESSARY;
        else
            return CHECKSUM_NONE;
    }
}


static struct mbuf * getmbuf(struct rtl8168_private *tp)
{
	struct mbuf *m;

	MGETHDR(m, M_DONTWAIT, MT_DATA);
	if(m == NULL){
		printf("getmbuf failed, Out of memory!!!\n");
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
	if (m->m_ext.ext_buf!=NULL) {
#if !(defined(LS3_HT) || defined(LS2G_HT))
		pci_sync_cache(tp->sc_pc, (vm_offset_t)m->m_ext.ext_buf, MCLBYTES, SYNC_R);
#endif
	}
#define RFA_ALIGNMENT_FUDGE 2
	m->m_data += RFA_ALIGNMENT_FUDGE;
#else
	m->m_data += RFA_ALIGNMENT_FUDGE;
#endif
	return m;
}

/*
 *	type : 0 is tx;
 *		   1 is rx;
 */
void print_rt(struct rtl8168_private *tp, int entry, int type) {
	int j, m = 0, n = 0;
	char s[16] = {0};
	int count = 0;

	if (type == 0)
		count = tp->tx_buf_sz;
	else if (type == 1)
		count = tp->rx_buf_sz;

	printf("0000000: ");
	for (j = 0; j < tp->tx_buf_sz; j += 2) {
		if ((j%16 == 0 && j > 0)) {
			if (m == 0) {
				sprintf(s, "0000%x%x0: ", n, m);
			}
			else if (m == 1)
				sprintf(s, "0000%x%x0: ", n, m);
			else if (m == 2)
				sprintf(s, "0000%x%x0: ", n, m);
			else if (m == 3)
				sprintf(s, "0000%x%x0: ", n, m);
			else if (m == 4)
				sprintf(s, "0000%x%x0: ", n, m);
			else if (m == 5)
				sprintf(s, "0000%x%x0: ", n, m);
			else if (m == 6)
				sprintf(s, "0000%x%x0: ", n, m);
			else if (m == 7)
				sprintf(s, "0000%x%x0: ", n, m);
			else if (m == 8)
				sprintf(s, "0000%x%x0: ", n, m);
			else if (m == 9)
				sprintf(s, "0000%x%x0: ", n, m);
			else if (m == 0xa)
				sprintf(s, "0000%x%x0: ", n, m);
			else if (m == 0xb)
				sprintf(s, "0000%x%x0: ", n, m);
			else if (m == 0xc)
				sprintf(s, "0000%x%x0: ", n, m);
			else if (m == 0xd)
				sprintf(s, "0000%x%x0: ", n, m);
			else if (m == 0xe)
				sprintf(s, "0000%x%x0: ", n, m);
			else if (m == 0xf)
				sprintf(s, "0000%x%x0: ", n, m);

			printf("\n");
			printf("%s", s);

			if (m == 0xf) {
				n++;
				m = 0;
			} else {
				m++;
			}
		}
		if (j == 14)
			m++;
		if (type == 0)
			sprintf(s, "%02x%02x ", tp->tx_buffer[entry][j], tp->tx_buffer[entry][j+1]);
		else if (type == 1)
			sprintf(s, "%02x%02x ", tp->rx_buffer[entry][j], tp->rx_buffer[entry][j+1]);
		printf("%s ", s);
	}
	printf("\n");

}

static void rtl8168_tx_interrupt(struct rtl8168_private *tp)
{
	unsigned int dirty_tx, tx_left;

	dirty_tx = tp->dirty_tx;
	tx_left = tp->cur_tx - dirty_tx;

	while (tx_left > 0) {
		unsigned int entry = dirty_tx % NUM_TX_DESC;
		u32 status = 0;

		status = le32_to_cpu(tp->TxDescArray[entry].opts1);
		if (status & DescOwn){
            printf("TX Interrupt error!  %s, %d\n", __FUNCTION__, __LINE__);
			break;
		}

		tp->stats.tx_packets++;

		if(rtl8168_tx_fill(tp, entry, entry + 1, 1) != 1){//should I do it here ? lihui
			printf("TX fill eror!  %s, %d\n", __FUNCTION__, __LINE__);
			return;
		}
		dirty_tx++;
		tx_left--;


	}
	tp->dirty_tx = dirty_tx;

	return;
}

void rtl8168_config_reg()
{
	int i;
	pcitag_t rtl8168_dev;

	rtl8168_dev = _pci_make_tag(2, 0, 0);

	printf("\n");
	for(i = 0; i < 0x100; i += 16){
		printf("reg 0x%x: %8x %8x %8x %8x\n", i,
					   			_pci_conf_read(rtl8168_dev, i),
								_pci_conf_read(rtl8168_dev, i+4),
								_pci_conf_read(rtl8168_dev, i+8),
								_pci_conf_read(rtl8168_dev, i+12));
	}

}

void rtl8168_io_reg()
{
	int i;
	u32	rtl8168_iobase;
	pcitag_t rtl8168_dev;

	rtl8168_dev = _pci_make_tag(2, 0, 0);
	rtl8168_iobase = 0xb8000000 | (_pci_conf_read(rtl8168_dev, 0x10) & 0xfffffff0);

	for(i = 0; i < 0x100; i += 4){
		printf("io reg 0x%x: %02x %02x %02x %02x\n", i,
						*(volatile unsigned char *)(rtl8168_iobase + i),
						*(volatile unsigned char *)(rtl8168_iobase + i + 1),
						*(volatile unsigned char *)(rtl8168_iobase + i + 2),
						*(volatile unsigned char *)(rtl8168_iobase + i + 3));
	}
}

#define RTL_CLOCK_RATE	3
#define RTL_EEPROM_READ_OPCODE      06
#define RTL_EEPROM_WRITE_OPCODE     05
#define RTL_EEPROM_ERASE_OPCODE     07
#define RTL_EEPROM_EWEN_OPCODE      19
#define RTL_EEPROM_EWDS_OPCODE      16

#define Cfg9346_EESK (1<<2)
#define Cfg9346_EEDI (1<<1)
#define Cfg9346_EEDO (1<<0)
#define Cfg9346_EECS (1<<3)
#define Cfg9346_EEM1 (1<<7)
#define Cfg9346 0x50

void rtl_raise_clock(u8 *x, struct rtl8168_private *tp)
{
	*x = *x | Cfg9346_EESK;
	RTL_W8(tp, Cfg9346, *x);
	udelay(RTL_CLOCK_RATE);
}

void rtl_eeprom_cleanup(struct rtl8168_private *tp)
{
	u8 x;

	x = RTL_R8(tp, Cfg9346);
	x &= ~(Cfg9346_EEDI | Cfg9346_EECS);

	RTL_W8(tp, Cfg9346, x);

	rtl_raise_clock(&x, tp);
	rtl_lower_clock(&x, tp);
}

void rtl_lower_clock(unsigned char *x, struct rtl8168_private *tp)
{
  	*x = *x & ~Cfg9346_EESK;
	RTL_W8(tp, Cfg9346, *x);
	udelay(RTL_CLOCK_RATE);
}

u16 rtl_shift_in_bits(struct rtl8168_private *tp)
{
	u8 x;
	u16 d, i;

	x = RTL_R8(tp, Cfg9346);
	x &= ~(Cfg9346_EEDI | Cfg9346_EEDO);

	d = 0;

	for (i = 0; i < 16; i++) {
		d = d << 1;
		rtl_raise_clock(&x, tp);

		x = RTL_R8(tp, Cfg9346);
		x &= ~Cfg9346_EEDI;

		if (x & Cfg9346_EEDO)
			d |= 1;

		rtl_lower_clock(&x, tp);
	}

	return d;
}

void rtl_shift_out_bits(int data, int count, struct rtl8168_private *tp)
{
	u8 x;
	int  mask;

	mask = 0x01 << (count - 1);
	x = RTL_R8(tp, Cfg9346);
	x &= ~(Cfg9346_EEDI | Cfg9346_EEDO);

	do {
		if (data & mask)
			x |= Cfg9346_EEDI;
		else
			x &= ~Cfg9346_EEDI;

		RTL_W8(tp, Cfg9346, x);
		udelay(RTL_CLOCK_RATE);
		rtl_raise_clock(&x, tp);
		rtl_lower_clock(&x, tp);
		mask = mask >> 1;
	} while(mask);

		x &= ~Cfg9346_EEDI;
		RTL_W8(tp, Cfg9346, x);
}

/* EEPROM CODE */
#define EE_WEN_CMD      ( (4 << 6) | ( 3 << 4 ) )
#define EE_WRITE_CMD    (5 << 6)
#define EE_WDS_CMD      (4 << 6)
#define EE_READ_CMD     (6 << 6)
#define EE_ERASE_CMD    (7 << 6)

/*  EEPROM_Ctrl bits. */
#define EE_CS           0x08    /* EEPROM chip select. */
#define EE_SHIFT_CLK    0x04    /* EEPROM shift clock. */
#define EE_DATA_WRITE   0x02    /* EEPROM chip data in. */
#define EE_WRITE_0      0x00
#define EE_WRITE_1      0x02
#define EE_DATA_READ    0x01    /* EEPROM chip data out. */
#define EE_ENB          (0x80 | EE_CS)

#define myoutb(v, a)  (*(volatile unsigned char *)(a) = (v))
#define myoutw(v, a)  (*(volatile unsigned short*)(a) = (v))
#define myoutl(v, a)  (*(volatile unsigned long *)(a) = (v))
//#define eeprom_delay()  inl(ee_addr)
#define eeprom_delay()	delay(100)
int read_eeprom(struct rtl8168_private *tp, int RegAddr)
{
	int i;
	long ee_addr = tp->ioaddr + Cfg9346;
	int read_cmd = RegAddr | EE_READ_CMD;
	int retval = 0;

	RTL_W8(tp, Cfg9346, EE_ENB & ~EE_CS);
	RTL_W8(tp, Cfg9346, EE_ENB);

#ifndef EPLC46
	/* Shift the read command bits out. */
		for (i = 10; i >= 0; i--) {
			int dataval = (read_cmd & (1 << i)) ? EE_DATA_WRITE : 0;
			RTL_W8(tp, Cfg9346, EE_ENB | dataval);
			delay(100);
			RTL_W8(tp, Cfg9346, EE_ENB | dataval | EE_SHIFT_CLK);
			delay(100);
		}
#else
	for (i = 11; i >= 0; i--) {
		int dataval = (read_cmd & (1 << i)) ? EE_DATA_WRITE : 0;
		RTL_W8(tp, Cfg9346, EE_ENB | dataval);
		delay(100);
		RTL_W8(tp, Cfg9346, EE_ENB | dataval | EE_SHIFT_CLK);
		delay(100);
	}
#endif
	RTL_W8(tp, Cfg9346, EE_ENB);

#ifndef EPLC46
		retval = 0;
	for( i = 16; i > 0; i--) {
		delay(100);
		RTL_W8(tp, Cfg9346, EE_ENB | EE_SHIFT_CLK);
		delay(100);
		retval = (retval << 1) | ((RTL_R8(tp, Cfg9346) & EE_DATA_READ) ? 1 : 0);
		RTL_W8(tp, Cfg9346, EE_ENB);
	}
#else
	for( i = 16; i > 0; i--) {
		RTL_W8(tp, Cfg9346, EE_ENB | EE_SHIFT_CLK);
		delay(100);
		retval = (retval << 1) | ((RTL_R8(tp, Cfg9346) & EE_DATA_READ) ? 1 : 0);
		RTL_W8(tp, Cfg9346, EE_ENB);
		delay(100);
	}
#endif
	return retval;
}

static void write_eeprom_enable(long ioaddr){
	int i;
	long ee_addr = ioaddr + Cfg9346;
	int  cmd = EE_WEN_CMD;

	myoutb(EE_ENB & ~EE_CS, ee_addr);
	myoutb(EE_ENB, ee_addr);

	/* Shift the read command bits out. */
#ifndef EPLC46
	for (i = 8; i >= 0; i--) {
		int dataval = (cmd & (1 << i)) ? EE_DATA_WRITE : 0;
		myoutb(EE_ENB | dataval, ee_addr);
		eeprom_delay();
		myoutb(EE_ENB | dataval | EE_SHIFT_CLK, ee_addr);
		eeprom_delay();
	}
#else
	for (i = 11; i >= 0; i--) {
		int dataval = (cmd & (1 << i)) ? EE_DATA_WRITE : 0;
		myoutb(EE_ENB | dataval, ee_addr);
		eeprom_delay();
		myoutb(EE_ENB | dataval | EE_SHIFT_CLK, ee_addr);
		eeprom_delay();
	}
#endif
	myoutb(~EE_CS, ee_addr);

}

static void write_eeprom_disable(unsigned long ioaddr)
{
	int i;
	long ee_addr = ioaddr + Cfg9346;
	int  cmd = EE_WDS_CMD;

	myoutb(EE_ENB & ~EE_CS, ee_addr);
	myoutb(EE_ENB, ee_addr);

	/* Shift the read command bits out. */

#ifndef EPLC46
	i=10;
#else
	i=11;
#endif
	for (; i >= 0; i--) {
		int dataval = (cmd & (1 << i)) ? EE_DATA_WRITE : 0;
		myoutb(EE_ENB | dataval, ee_addr);
		eeprom_delay();
		myoutb(EE_ENB | dataval | EE_SHIFT_CLK, ee_addr);
		eeprom_delay();
	}
	myoutb(~EE_CS, ee_addr);
}

int rtl8168_write_eeprom(long ioaddr, int location,unsigned short data)
{
	volatile int i;
	long ee_addr = ioaddr + Cfg9346;
	int  cmd = location | EE_WRITE_CMD;

	write_eeprom_enable(ioaddr);

	cmd <<=16;
	cmd |= data;
	myoutb(EE_ENB & ~EE_CS, ee_addr);
	myoutb(EE_ENB, ee_addr);

	/* Shift the read command bits out. */
	for (i = 24; i >= 0; i--) {
		int dataval = (cmd & (1 << i)) ? EE_DATA_WRITE : 0;
		myoutb(EE_ENB | dataval, ee_addr);
		eeprom_delay();
		myoutb(EE_ENB | dataval | EE_SHIFT_CLK, ee_addr);
		eeprom_delay();
	}
	/* Terminate the EEPROM access. */
	myoutb(~EE_CS, ee_addr);
		delay(10);

	myoutb(EE_ENB, ee_addr);
	 
    while( ! (inb(ee_addr) & EE_DATA_READ) ){
		myoutb(EE_ENB, ee_addr);
		delay(10);
	}
	
	return 0;
}

static int cmd_reprom(int ac, char *av[])
{
	int i;
	int n = 0;
	unsigned short data;	
	char    net_type[5];
	printf("dump eprom:\n");
#if     defined (LOONGSON_3ASINGLE) || defined (LOONGSON_3BSINGLE)
        strcpy(net_type,av[1]);
        if(strstr(net_type,"eth0")!=NULL)
        {
                strcpy(av[1],"rte0");
        }
#endif
	if (!strcmp(av[1], "rte0")){
		n = 0; 
		//printf("Now read rte0 e2prom\n");
		printf("Now read eth0 e2prom\n");
	}
	else if (!strcmp(av[1], "rte1")){
		n = 1; 
		//printf("Now read rte1 e2prom\n");
		printf("Now read eth1 e2prom\n");
	}
	else{
		printf("input device Error \n");
		return 0;
	}

    if(!myRTL[n]){
           printf("r8111/8168 interface not initialized\n");
           return 0;
    }
	printf("myRTL[%d] = %x\n", n, myRTL[n]);

	for(i=0; i< 64; i++){
#ifndef EPLC46
		data = read_eeprom(myRTL[n],  i); //run 
#else
		data = read_eeprom(myRTL[n], 2*i);
		data = data | (read_eeprom(myRTL[n],2*i+1)) << 8;
#endif
		printf("%04x ", data);
		if((i+1)%8 == 0)
			printf("\n");
	}

	return data;
}

#if 0
 static int cmd_set_811(int ac, char *av[])
{
     int old_value;
     struct rtl8169_private *tp;

     tp = myRTL[pn];

     old_value = mdio_read(tp, PHY_CTRL_REG);
     printf("the old PHY_CTRL_REG value is %x\n", old_value);
     old_value |= 0x0200;       //set to re auto negotiation
     mdio_write(tp, PHY_CTRL_REG, old_value);

     return 0;
 }
#endif
static int cmd_wrprom(int ac, char *av[])
{
    	int i;
	int n = 0; 
    	unsigned short data;
	char    net_type[5];
static unsigned char rom[]={
/*00000000:*/0x29,0x81,0xEC,0x10,0x68,0x81,0xEC,0x10,0x68,0x81,0x04,0x01,0x9C,0x62,0x00,0xE0,
/*00000010:*/0x4C,0x68,0x00,0x01,0x05,0x4F,0xC3,0xFF,0x04,0x02,0xC0,0x8C,0x80,0x02,0x00,0x00,
/*00000020:*/0x11,0x3C,0x07,0x00,0x10,0x20,0x76,0x00,0x63,0x01,0x01,0xFF,0x00,0x27,0xAA,0x03,
/*00000030:*/0x02,0x20,0x99,0xA5,0x80,0x02,0x00,0x20,0x04,0x40,0x20,0x00,0x04,0x40,0x20,0x20,
/*00000040:*/0x00,0x00,0x20,0xE1,0x22,0xB5,0x60,0x00,0x0A,0x00,0xE0,0x00,0x68,0x4C,0x00,0x00,
/*00000050:*/0x01,0x00,0x0f,0x00,0xB2,0x73,0x75,0xEA,0x87,0x75,0x7A,0x39,0xCA,0x98,0x00,0x00,
/*00000060:*/0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
/*00000070:*/0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
#if     defined (LOONGSON_3ASINGLE) || defined (LOONGSON_3BSINGLE)
        strcpy(net_type,av[1]);
        if(strstr(net_type,"eth0")!=NULL)
        {
                strcpy(av[1],"rte0");
        }
#endif
	if (!strcmp(av[1], "rte0")){
		n = 0; 
		//printf("Now write rte0 e2prom\n");
		printf("Now write eth0 e2prom\n");
	}
	else if (!strcmp(av[1], "rte1")){
		n = 1; 
		//printf("Now write rte1 e2prom\n");
		printf("Now write eth1 e2prom\n");
	}
	else{
		//printf("Error rte device\n");
		printf("Error device\n");
		return 0;
	}

    if(!myRTL[n]){
    	printf("rtl8111/8168 interface not initialized\n");
        return 0;
    }

    printf("myRTL[%d]->ioaddr = %x\n", n, myRTL[n]->ioaddr);

    for(i = 0; i < 0x40; i++)
    {
        int j = 5;
        printf("program %02x:%04x\n",2*i,(rom[2*i+1]<<8)|rom[2*i]);
rewrite:
        rtl8168_write_eeprom(myRTL[n]->ioaddr, i, (rom[2*i+1]<<8)|rom[2*i]);
        data = read_eeprom(myRTL[n],  i);
        printf("data = %x\n", data);

        if(data != ((rom[2*i+1]<<8)|rom[2*i])){
            printf("write failed, rewrite now\n");
			j--;
			if (j < 0){
				printf("write e2prom failed\n");

				return -1;
			}
            goto rewrite;
        }

    }
    printf("The whole eeprom have been programmed!\n");

	/* set mac address */
//	if (NULL != av[2]){
	if (3 == ac) {
		int v;
		char *p = av[2];
		unsigned short val = 0;

		printf("Set Macaddress\n");
		for (i = 0; i < 3; i++){
			val = 0;
			gethex(&v, av[2], 2);
			val = v;
			av[2] += 3;

			gethex(&v, av[2], 2);
			val = val | (v << 8);
			av[2] += 3;	

			printf("value[%x] = %4x\n", i, val);

			rtl8168_write_eeprom(myRTL[n]->ioaddr, 0x7 + i, val);
		}
		printf("Set Macaddress : %s End\n", p);

	}
	else
		printf("No Set Macaddress\n");

	return 0;
}
#if     defined (LOONGSON_3ASINGLE)
#include "generate_mac_val.c"
#endif
static int cmd_setmac(int ac, char *av[])
{
	int i, n;
	unsigned short val = 0;
	int32_t v;
	char	net_type[5];

#if     defined (LOONGSON_3ASINGLE)
        unsigned short data = 0;
        unsigned short data1 = 0;
        unsigned short data2 = 0;

        unsigned short data_tmp = 0;

        int count, j, vd;

        unsigned char buf[32];
        char * addr;
	unsigned char tmp[6];
#endif

#if     defined (LOONGSON_3ASINGLE) || defined (LOONGSON_3BSINGLE)
        strcpy(net_type,av[1]);
        if(strstr(net_type,"eth0")!=NULL)
        {
                strcpy(av[1],"rte0");
        }
#endif
	if (!strcmp(av[1], "rte0")){
		n = 0; 
		//printf("Now set rte0 MAC\n");
		printf("Now set eth0 MAC\n");
	}
	else if (!strcmp(av[1], "rte1")){
		n = 1; 
		//printf("Now set rte1 MAC\n");
		printf("Now set eth1 MAC\n");
	}
	else{
		//printf("Error rte device\n");
		printf("Error device\n");
		return 0;
	}

	if(!myRTL[n]){
		printf("r8111/8168 interface not initialized\n");
		return 0;
	}

#if     defined (LOONGSON_3ASINGLE)
	if((ac == 1) || (ac ==2)){
	{
              for (i=0 ; i < 6; i++) {
                  buf[i] = myRTL[n]->dev_addr[i];
	          tmp[i] = buf[i];
              }
          }
#if 0//debug
	{
                printf(" eth0  Mac address buf: ");
                   for (vd = 0;vd < 6;vd++)
                        	printf("%2x%s",buf[vd],(5-vd)?":":" ");
                        printf("\n");
      }
#endif
		addr = generate_mac_val(buf);
#if 0//debug
	{
                printf(" eth0  Mac address addr= %x: ",&addr);
                   for (vd = 0;vd < 6;vd++)
                        	printf("%2x%s",*(addr + vd) & 0xff,(5-vd)?":":" ");
                        printf("\n");
      }
		printf("buf[0]= %x ,addr = %x\n",buf[0],*addr & 0xff);
#endif
	if((*addr & 0xff)!=tmp[0]){    //addr and buf are different
		data = buf[1];
                data = buf[0] | (data << 8);

                data1 = buf[3];
                data1 = buf[2] | (data1 << 8);

                data2 = buf[5];
                data2 = buf[4] | (data2 << 8);
                                
		count = rtl8168_write_eeprom(myRTL[n]->ioaddr, 0x7, data);
                if(count != 0)
                	printf("eeprom write data error\n");

                count = rtl8168_write_eeprom(myRTL[n]->ioaddr, 0x8, data1);
                if(count != 0)
                	printf("eeprom write data1 error\n");

                count = rtl8168_write_eeprom(myRTL[n]->ioaddr, 0x9, data2);
                if(count != 0)
                	printf("eeprom write data2 error\n");

		if(count == 0){
                	printf("set eth0  Mac address: ");
                        for (vd = 0;vd < 6;vd++)
                        	printf("%02x%s",*(addr + vd) & 0xff,(5-vd)?":":" ");
                        printf("\n");
                        printf("The machine should be restarted to make the mac change to take effect!!\n");
                } else
                	printf("eeprom write error!\n");
                printf("you can set it by youself\n");
       	  }	
	  else{
                printf(" eth0  Mac address addr: ");
                   for (vd = 0;vd < 6;vd++)
                        	printf("%02x%s",*(addr + vd) & 0xff,(5-vd)?":":" ");
                        printf("\n");
 
		}
		 
	}else{
                for (i = 0; i < 3; i++) {
                        val = 0;
                        gethex(&v, av[2], 2);
                        val = v ;
                        av[2]+=3;

                        printf("eth0 Mac address bit [%d]: %x, val = %x\n",2*i, v, val);

                        gethex(&v, av[2], 2);
                        val = val | (v << 8);
                        av[2] += 3;

                        printf("eth0 Mac address bit [%d]: %x, val = %x\n",2*i+1, v, val);

                        count = rtl8168_write_eeprom(myRTL[n]->ioaddr, 0x7 + i, val);
                        if(count == 0)
                                printf("eeprom write done!\n");
                }

        printf("The machine should be restarted to make the mac change to take effect!!(./dev/pci/rtl8168.c:)\n");
        }
        return 0;
#else
	if(ac != 3){
		printf("MAC ADDRESS ");
		for(i=0; i<6; i++){
			printf("%02x", myRTL[n]->dev_addr[i]);
			if(i==5)
				printf("\n");
			else
				printf(":");
		}
		//printf("Use \"setmac rte0/1 <mac> \" to set mac address\n");
		printf("Use \"setmac eth0/1 <mac> \" to set mac address\n");
		return 0;
	}
	for (i = 0; i < 3; i++) {
		val = 0;
		gethex(&v, av[2], 2);
		val = v ;
		av[2]+=3;

		gethex(&v, av[2], 2);
		val = val | (v << 8);
		av[2] += 3;

		rtl8168_write_eeprom(myRTL[n]->ioaddr, 0x7 + i, val);

	}

	printf("The machine should be restarted to make the mac change to take effect!!\n");

	return 0;
#endif
}

int cmd_msqt_lan(int ac, char *av[])
{
struct rtl8168_private *tp;
	tp = mytp;
	if (!strcmp(av[1], "100M")) {
		if (!strcmp(av[2], "chana")) {
			mdio_write(tp, 14, 0x0660);
			mdio_write(tp, 16, 0x0020);
		} else if (!strcmp(av[2], "chanb")) {
			mdio_write(tp, 14, 0x0660);
			mdio_write(tp, 16, 0x0000);
		} else {
			printf("Error options\n");
			return 0;
		}
	} else if (!strcmp(av[1], "1000M")) {
		if (!strcmp(av[2], "mode1"))
			mdio_write(tp, 0x9, 0x2000);
		else if (!strcmp(av[2], "mode4"))
			mdio_write(tp, 0x9, 0x8000);
		else {
			printf("Error options\n");
			return 0;
		}
	} else {
		printf("Error options\n");
		return 0;
	}
		return 0;
}

static const Optdesc netdmp_opts[] =
{
    {"<interface>", "Interface name"},
    {0}
};

static const Cmd Cmds[] =
{
	{"Realtek 8111dl/8168"},

	{"readrom", "", NULL,
			"dump rtl8111dl/8168 eeprom content and mac address", cmd_reprom, 2, 2,0},
	{"writerom", "", NULL,
			"dump rtl8111dl/8168 eeprom content", cmd_wrprom, 2, 3, 0},
	{"setmac", "", NULL,
		    "Set mac address into rtl8111dl/8168 eeprom", cmd_setmac, 2, 5, 0},
	{"msqt_lan", " [100M/1000M] [mode1(waveform)/mode4(distortion)]/[chana/chanb]", NULL, "Motherboard Signal Quality Test for RTL8111", cmd_msqt_lan, 3, 3, 0},
	{0, 0}
};


static void init_cmd __P((void)) __attribute__ ((constructor));

static void
init_cmd()
{
	cmdlist_expand(Cmds, 1);
}
