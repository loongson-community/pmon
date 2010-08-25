/************************************************************************

 Copyright (C)
 File name:     atl1c.c
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


#include <stdlib.h>
#include "bpfilter.h"

#include <string.h>

#include <sys/param.h>
#include <sys/mbuf.h>
#include <sys/malloc.h>
#include <sys/kernel.h>
#include <sys/socket.h>
#include <sys/syslog.h>

#include <net/if.h>
#include <net/if_dl.h>
#include <net/if_media.h>
#include <net/if_types.h>

#include <machine/cpu.h>

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

#include "atl1c.h"



/* Transmit Memory count
 *
 * Valid Range: 16-1024
 *
 * Default Value: 32
 */
#define ATL1C_MIN_TX_DESC_CNT		16
#define ATL1C_MAX_TX_DESC_CNT		1024
#define ATL1C_DEFAULT_TX_DESC_CNT	32//1024

/* Receive Memory Block Count
 *
 * Valid Range: 32-1024
 *
 * Default Value: 128
 */
#define ATL1C_MIN_RX_MEM_SIZE		32 
#define ATL1C_MAX_RX_MEM_SIZE		1024
#define ATL1C_DEFAULT_RX_MEM_SIZE	128//512


struct sk_buff {
    unsigned int    len;            /* Length of actual data            */
    unsigned char   *data;          /* Data head pointer                */
    unsigned char   *head;          /* Data head pointer                */
	unsigned char protocol;
    struct atl1c_private *ap;
};

#define udelay(m)	delay(m)

/* Maximum events (Rx packets, etc.) to handle at each interrupt. */
static const int max_interrupt_work = 20;

static int atl1c_match( struct device *, void *, void *);
static void atl1c_attach(struct device *, struct device *, void *);
static int atl1c_ether_ioctl(struct ifnet *, unsigned long , caddr_t);
static int atl1c_open(struct atl1c_private *);
static int atl1c_interrupt(void *arg);
static int atl1c_sw_init(struct atl1c_private *);
int atl1c_phy_reset(struct atl1c_hw *);
int atl1c_phy_init(struct atl1c_hw *);
static int atl1c_phy_config(struct atl1c_hw *);
static int atl1c_reset_mac(struct atl1c_hw *);
int atl1c_read_mac_addr(struct atl1c_hw *);
void atl1c_hw_set_mac_addr(struct atl1c_hw *);
static int atl1c_phy_setup_adv(struct atl1c_hw *);
static int atl1c_setup_mac_funcs(struct atl1c_hw *);
static void atl1c_set_mac_type(struct atl1c_hw *);
static void atl1c_set_rxbufsize(struct atl1c_private *);
static void atl1c_phy_magic_data(struct atl1c_hw *);
static int atl1c_stop_mac(struct atl1c_hw *);
static int atl1c_get_permanent_address(struct atl1c_hw *);
int atl1c_check_eeprom_exist(struct atl1c_hw *);
static int atl1c_setup_ring_resources(struct atl1c_private *);
int atl1c_up(struct atl1c_private *);
static void atl1c_init_ring_ptrs(struct atl1c_private *);
static int atl1c_alloc_rx_buffer(struct atl1c_private *, const int);
static int atl1c_configure(struct atl1c_private *);
static void atl1c_configure_des_ring(const struct atl1c_private *);
static void atl1c_configure_tx(struct atl1c_private *);
static void atl1c_configure_rx(struct atl1c_private *);
static void atl1c_configure_rss(struct atl1c_private *);
static void atl1c_configure_dma(struct atl1c_private *);
static void atl1c_irq_enable(struct atl1c_private *);
static void atl1c_check_link_status(struct atl1c_private *);
static void atl1c_set_aspm(struct atl1c_hw *hw, unsigned int linkup);
int atl1c_get_speed_and_duplex(struct atl1c_hw *hw, u16 *speed, u16 *duplex);
static void atl1c_enable_tx_ctrl(struct atl1c_hw *hw);
static void atl1c_enable_rx_ctrl(struct atl1c_hw *hw);
static void atl1c_setup_mac_ctrl(struct atl1c_private *ap);
static void atl1c_clean_rx_ring(struct atl1c_private *ap);
static void atl1c_free_ring_resources(struct atl1c_private *ap);
void atl1c_down(struct atl1c_private *ap);
static inline void atl1c_irq_disable(struct atl1c_private *ap);
static void atl1c_clean_tx_ring(struct atl1c_private *ap,enum atl1c_trans_queue type);
static void atl1c_clear_phy_int(struct atl1c_private *ap);
static void atl1c_clean_rx_irq(struct atl1c_private *ap, u8 que);
static void atl1c_clean_rrd(struct atl1c_rrd_ring *rrd_ring,
			struct	atl1c_recv_ret_status *rrs, u16 num);
static void atl1c_clean_rfd(struct atl1c_rfd_ring *rfd_ring,
	struct atl1c_recv_ret_status *rrs, u16 num);
static unsigned int atl1c_clean_tx_irq(struct atl1c_private *ap,
				enum atl1c_trans_queue type);
static void atl1c_link_chg_event(struct atl1c_private *ap);
static void atl1c_start(struct ifnet *ifp);


struct cfattach atl_ca = {
	sizeof(struct atl1c_private), atl1c_match, atl1c_attach, 
};

struct cfdriver atl_cd = {
	NULL, "atl", DV_IFNET,
};

static const u16 atl1c_pay_load_size[] = {
	128, 256, 512, 1024, 2048, 4096,
};

static const u16 atl1c_rfd_prod_idx_regs[AT_MAX_RECEIVE_QUEUE] =
{
	REG_MB_RFD0_PROD_IDX,
	REG_MB_RFD1_PROD_IDX,
	REG_MB_RFD2_PROD_IDX,
	REG_MB_RFD3_PROD_IDX
};

static const u16 atl1c_rfd_addr_lo_regs[AT_MAX_RECEIVE_QUEUE] =
{
	REG_RFD0_HEAD_ADDR_LO,
	REG_RFD1_HEAD_ADDR_LO,
	REG_RFD2_HEAD_ADDR_LO,
	REG_RFD3_HEAD_ADDR_LO
};

static const u16 atl1c_rrd_addr_lo_regs[AT_MAX_RECEIVE_QUEUE] =
{
	REG_RRD0_HEAD_ADDR_LO,
	REG_RRD1_HEAD_ADDR_LO,
	REG_RRD2_HEAD_ADDR_LO,
	REG_RRD3_HEAD_ADDR_LO
};

#define PCI_VENDOR_ID_ATHEROS       0x1969
#define PCI_PRODUCT_ID_ATL1C       0x1063

/* 
 * The little-endian AUTODIN II ethernet CRC calculations.
 * A big-endian version is also available.
 * This is slow but compact code.  Do not use this routine 
 * for bulk data, use a table-based routine instead.
 * This is common code and should be moved to net/core/crc.c.
 * Chips may use the upper or lower CRC bits, and may reverse 
 * and/or invert them.  Select the endian-ness that results 
 * in minimal calculations.
 */
static u32 ether_crc_le(int length, unsigned char *data)
{
    u32 crc = ~0;  /* Initial value. */
    while(--length >= 0) {
        unsigned char current_octet = *data++;
        int bit;
        for (bit = 8; --bit >= 0; current_octet >>= 1) {
            if ((crc ^ current_octet) & 1) {
                crc >>= 1;
                crc ^= 0xedb88320;
            } 
            else
                crc >>= 1;
        }
    }
    return ~crc;
}

/*
 * atl1c_hash_mc_addr
 *  purpose
 *      set hash value for a multicast address
 *      hash calcu processing :
 *          1. calcu 32bit CRC for multicast address
 *          2. reverse crc with MSB to LSB
 */
u32 atl1c_hash_mc_addr(struct atl1c_hw *hw, u8 *mc_addr)
{
	u32 crc32;
	u32 value = 0;
	int i;

	crc32 = ether_crc_le(6, mc_addr);
	for (i = 0; i < 32; i++)
		value |= (((crc32 >> i) & 1) << (31 - i));

	return value;
}

static struct sk_buff *dev_alloc_skb(unsigned int length)
{
    struct sk_buff *skb=(struct sk_buff *)malloc(sizeof(struct sk_buff),M_DEVBUF,M_DONTWAIT);
	if(skb)	{
        skb->len=0;
        skb->data=skb->head=(void *)malloc(length,M_DEVBUF,M_DONTWAIT);
	}
	if(!skb||!skb->data){printf("dev_alloc_skb :not enough memory!malloc() failed.");}
	return skb&&skb->data?skb:0;
}

static int netif_rx(struct sk_buff *skb)
{
	struct mbuf *m;
	struct ether_header *eh;
	struct atl1c_private *ap = skb->ap;
	struct ifnet *ifp = &ap->arpcom.ac_if;
    m =getmbuf();
    if (m == NULL){
        printf("getmbuf failed in  netif_rx\n");
        return 0; // no successful
    }
    skb->len=skb->len - 4;
    bcopy(skb->data, mtod(m, caddr_t), skb->len);

    //hand up  the received package to upper protocol for further dealt
    m->m_pkthdr.rcvif = ifp;
    m->m_pkthdr.len = m->m_len = skb->len -sizeof(struct ether_header);

    eh=mtod(m, struct ether_header *);

    m->m_data += sizeof(struct ether_header);
    ether_input(ifp, eh, m);
	return 0;
}

/*
 * Reads the value from a PHY register
 * hw - Struct containing variables accessed by shared code
 * reg_addr - address of the PHY register to read
 */
int atl1c_read_phy_reg(struct atl1c_hw *hw, u16 reg_addr, u16 *phy_data)
{
	u32 val;
	int i;

	val = ((u32)(reg_addr & MDIO_REG_ADDR_MASK)) << MDIO_REG_ADDR_SHIFT |
		MDIO_START | MDIO_SUP_PREAMBLE | MDIO_RW |
		MDIO_CLK_25_4 << MDIO_CLK_SEL_SHIFT;

	AT_WRITE_REG(hw, REG_MDIO_CTRL, val);

	for (i = 0; i < MDIO_WAIT_TIMES; i++) {
		udelay(2);
		AT_READ_REG(hw, REG_MDIO_CTRL, &val);
		if (!(val & (MDIO_START | MDIO_BUSY)))
			break;
	}
	if (!(val & (MDIO_START | MDIO_BUSY))) {
		*phy_data = (u16)val;
		return 0;
	}

	return AT_ERR_PHY;
}

/*
 * Writes a value to a PHY register
 * hw - Struct containing variables accessed by shared code
 * reg_addr - address of the PHY register to write
 * data - data to write to the PHY
 */
int atl1c_write_phy_reg(struct atl1c_hw *hw, u32 reg_addr, u16 phy_data)
{
	int i;
	u32 val;

	val = ((u32)(phy_data & MDIO_DATA_MASK)) << MDIO_DATA_SHIFT   |
	       (reg_addr & MDIO_REG_ADDR_MASK) << MDIO_REG_ADDR_SHIFT |
	       MDIO_SUP_PREAMBLE | MDIO_START |
	       MDIO_CLK_25_4 << MDIO_CLK_SEL_SHIFT;

	AT_WRITE_REG(hw, REG_MDIO_CTRL, val);

	for (i = 0; i < MDIO_WAIT_TIMES; i++) {
		udelay(2);
		AT_READ_REG(hw, REG_MDIO_CTRL, &val);
		if (!(val & (MDIO_START | MDIO_BUSY)))
			break;
	}

	if (!(val & (MDIO_START | MDIO_BUSY)))
		return 0;

	return AT_ERR_PHY;
}



static int atl1c_match(struct device *parent,void *match,void * aux)
{
	struct pci_attach_args *pa = aux;

	if(PCI_VENDOR(pa->pa_id) == PCI_VENDOR_ID_ATHEROS) {
		if(PCI_PRODUCT(pa->pa_id) == PCI_PRODUCT_ID_ATL1C)
			return 1;
	}
	return 0;
}

static int atl1c_init_board(struct atl1c_private *ap, struct pci_attach_args *pa)
{
	int rc = 0;
	pci_chipset_tag_t pc = pa->pa_pc;
	int tmp;
    bus_addr_t membase; 

	tmp = _pci_conf_read(pa->pa_tag, PCI_COMMAND_STATUS_REG);

	if(!(tmp & PCI_COMMAND_MASTER_ENABLE)) {
		tmp |= PCI_COMMAND_MASTER_ENABLE;
		_pci_conf_write(pa->pa_tag, PCI_COMMAND_STATUS_REG, tmp);
	}
              
    membase   =_pci_conf_read(pa->pa_tag,0x10);
    membase   = membase & PTR_PAD(0xffffff00); //laster 8 bit
    membase  |= PTR_PAD(0xb0000000);
    
	ap->sc_st = pa->pa_iot;
	ap->sc_pc = pc;

	ap->ioaddr = (unsigned long)membase;
    ap->hw.ap = ap;
    ap->hw.hw_addr = (u8 *)ap->ioaddr;
    
    return rc;
}

static void atl1c_set_mac_type(struct atl1c_hw *hw)
{
	switch (hw->device_id) {
	case DEV_ID_ATL1C_2_0:
		hw->nic_type = athr_l1c;
		break;
	case DEV_ID_ATL2C_2_0:
		hw->nic_type = athr_l2c;
		break;
	case DEV_ID_ATL2C_B:
		hw->nic_type = athr_l2c_b;
		break;
	case DEV_ID_ATL2C_B_2:
		hw->nic_type = athr_l2c_b2;
		break;
	case DEV_ID_ATL1D:
		hw->nic_type = athr_l1d;
		break;
	default:
		break;
	}
	
	return;
}

static int atl1c_setup_mac_funcs(struct atl1c_hw *hw)
{
	u32 phy_status_data;
	u32 link_ctrl_data;
	
	atl1c_set_mac_type(hw);
	AT_READ_REG(hw, REG_PHY_STATUS, &phy_status_data);
	AT_READ_REG(hw, REG_LINK_CTRL, &link_ctrl_data);
	
	hw->ctrl_flags = ATL1C_INTR_MODRT_ENABLE  |
        ATL1C_INTR_CLEAR_ON_READ |
			 ATL1C_TXQ_MODE_ENHANCE;
	if (link_ctrl_data & LINK_CTRL_L0S_EN)
		hw->ctrl_flags |= ATL1C_ASPM_L0S_SUPPORT;
	if (link_ctrl_data & LINK_CTRL_L1_EN)
		hw->ctrl_flags |= ATL1C_ASPM_L1_SUPPORT;
	if (link_ctrl_data & LINK_CTRL_EXT_SYNC)
		hw->ctrl_flags |= ATL1C_LINK_EXT_SYNC; 
	/* FIXME:
 	 * JUST FOR TEST VERSION
 	 */
	hw->ctrl_flags |= ATL1C_ASPM_CTRL_MON;

	if (hw->nic_type == athr_l1c || hw->nic_type == athr_l1d) {
		hw->link_cap_flags |= LINK_CAP_SPEED_1000;
	}
	return 0;	
}

static void atl1c_set_rxbufsize(struct atl1c_private *ap)
{
    ap->rx_buffer_len = AT_RX_BUF_SIZE;
}

static int atl1c_sw_init(struct atl1c_private *ap)
{
    struct atl1c_hw *hw = &ap->hw;
    u32 revision;

    hw->vendor_id = PCI_VENDOR(ap->pa->pa_id);
    hw->device_id = PCI_PRODUCT(ap->pa->pa_id);

    AT_READ_REG(hw, PCI_CLASS_REG, &revision);
	hw->revision_id = revision & 0xFF;

	if (atl1c_setup_mac_funcs(hw) != 0) {
        printf("set mac function pointers failed\n");
        return -1;
	}   
	ap->wol = 0;
	ap->link_speed = SPEED_0;
	ap->link_duplex = FULL_DUPLEX;
	ap->num_rx_queues = AT_DEF_RECEIVE_QUEUE;//AT_MAX_RECEIVE_QUEUE;//AT_DEF_RECEIVE_QUEUE;

	hw->intr_mask = IMR_NORMAL_MASK;
	hw->phy_configured = false;
	hw->preamble_len = 7;
	hw->max_frame_size = 1422;//mtu???
	if (ap->num_rx_queues < 2) {
		hw->rss_type = atl1c_rss_disable;
		hw->rss_mode = atl1c_rss_mode_disable;
	} else {
		hw->rss_type = atl1c_rss_ipv4;
		hw->rss_mode = atl1c_rss_mul_que_mul_int;
		hw->rss_hash_bits = 16;
	}
	hw->autoneg_advertised = ADVERTISE_DEFAULT;
	hw->indirect_tab = 0xE4E4E4E4;
	hw->base_cpu = 0; 

	hw->ict = 50000;		/* 100ms */
	hw->smb_timer = 200000;	  	/* 400ms */
	hw->cmb_tpd = 4;
	hw->cmb_tx_timer = 1;		/* 2 us  */
	hw->rx_imt = 200;
	hw->tx_imt = 1000;

	hw->tpd_burst = 5;
	hw->rfd_burst = 8;
	hw->dma_order = atl1c_dma_ord_out;
	hw->dmar_block = atl1c_dma_req_1024;
	hw->dmaw_block = atl1c_dma_req_1024;
	hw->dmar_dly_cnt = 15;
	hw->dmaw_dly_cnt = 4; 
    hw->media_type = MEDIA_TYPE_AUTO_SENSOR;

    ap->tpd_ring[0].count = ATL1C_DEFAULT_TX_DESC_CNT;
    ap->rfd_ring[0].count = ATL1C_DEFAULT_RX_MEM_SIZE;

	atl1c_set_rxbufsize(ap);    
    return 0;
}

/*
 * atl1c_clean_tx_ring - Free Tx-skb
 * @adapter: board private structure
 */
static void atl1c_clean_tx_ring(struct atl1c_private *ap,
				enum atl1c_trans_queue type)
{
	struct atl1c_tpd_ring *tpd_ring = &ap->tpd_ring[type];
	struct atl1c_buffer *buffer_info;
	u16 index, ring_count;

	ring_count = tpd_ring->count;
	for (index = 0; index < ring_count; index++) {
		buffer_info = &tpd_ring->buffer_info[index];
		if (buffer_info->state == ATL1_BUFFER_FREE)
			continue;
		if (buffer_info->dma)
			pci_unmap_single(NULL, buffer_info->dma,
					buffer_info->length,
					PCI_DMA_TODEVICE);
		if (buffer_info->skb){
			free(buffer_info->skb->head,M_DEVBUF);
            free(buffer_info->skb,M_DEVBUF);
        }
		buffer_info->dma = 0;
		buffer_info->skb = NULL;
		buffer_info->state = ATL1_BUFFER_FREE;
	}

	/* Zero out Tx-buffers */
	memset(tpd_ring->desc, 0, sizeof(struct atl1c_tpd_desc) *
				ring_count);
	tpd_ring->next_to_clean = 0;
	tpd_ring->next_to_use = 0;
}

static void atl1c_attach(struct device * parent, struct device * self, void *aux)
{
	struct atl1c_private *ap = (struct atl1c_private *)self;

	int rc;
	struct  ifnet *ifp;
	pci_intr_handle_t ih;


	ap->pa = (struct pci_attach_args *)aux;

	rc = atl1c_init_board(ap, aux);
	if (rc < 0)
		return;
   
    atl1c_sw_init(ap);  
    
    atl1c_phy_reset(&ap->hw);

    atl1c_reset_mac(&ap->hw);

	/* reset the controller to
	 * put the device in a known good starting state */
    atl1c_phy_init(&ap->hw);
    
    atl1c_read_mac_addr(&ap->hw); 

    atl1c_hw_set_mac_addr(&ap->hw); 

    atl1c_phy_config(&ap->hw);
    
	ifp = &ap->arpcom.ac_if;

	bcopy(ap->hw.mac_addr, ap->arpcom.ac_enaddr, sizeof(ap->arpcom.ac_enaddr));
    bcopy(ap->sc_dev.dv_xname, ifp->if_xname, IFNAMSIZ);

	ifp->if_softc = ap;
	ifp->if_ioctl = atl1c_ether_ioctl;
	ifp->if_start = atl1c_start;
	ifp->if_snd.ifq_maxlen = ATL1C_DEFAULT_TX_DESC_CNT - 1;// NUM_TX_DESC - 1;

	if_attach(ifp);
	ether_ifattach(ifp);
    rc = atl1c_open(ap);
	if(rc) {
		printf("atl1c_open: error code %d \n", rc);
		return;
	}
    
	if (pci_intr_map(pc, pa->pa_intrtag, pa->pa_intrpin, pa->pa_intrline, &ih)) {
		printf(": couldn't map interrupt\n");
		return;
	}

    ap->sc_ih = pci_intr_establish(pc, ih, IPL_NET, atl1c_interrupt, ap, self->dv_xname);

	return ;
}

/*
 * atl1c_clean_rx_ring - Free rx-reservation skbs
 * @adapter: board private structure
 */
static void atl1c_clean_rx_ring(struct atl1c_private *ap)
{
	struct atl1c_rfd_ring *rfd_ring = ap->rfd_ring;
	struct atl1c_rrd_ring *rrd_ring = ap->rrd_ring;
	struct atl1c_buffer *buffer_info;
	int i, j;

	for (i = 0; i < ap->num_rx_queues; i++) {
		for (j = 0; j < rfd_ring[i].count; j++) {
			buffer_info = &rfd_ring[i].buffer_info[j];
			if (buffer_info->state == ATL1_BUFFER_FREE)
				continue;
			if (buffer_info->dma)
				pci_unmap_single(NULL, buffer_info->dma,
						buffer_info->length,
						PCI_DMA_FROMDEVICE);
			if (buffer_info->skb){
				free(buffer_info->skb->head,M_DEVBUF);
                free(buffer_info->skb,M_DEVBUF);
             }
            
			buffer_info->dma = 0;
			buffer_info->skb = NULL;
			buffer_info->state = ATL1_BUFFER_FREE;
		}
		/* zero out the descriptor ring */
		memset(rfd_ring[i].desc, 0, rfd_ring[i].size);
		rfd_ring[i].next_to_clean = 0;
		rfd_ring[i].next_to_use = 0;
		rrd_ring[i].next_to_use = 0;
		rrd_ring[i].next_to_clean = 0;
	}
}


/*
 * Read / Write Ptr Initialize:
 */
static void atl1c_init_ring_ptrs(struct atl1c_private *ap)
{
	struct atl1c_tpd_ring *tpd_ring = ap->tpd_ring;
	struct atl1c_rfd_ring *rfd_ring = ap->rfd_ring;
	struct atl1c_rrd_ring *rrd_ring = ap->rrd_ring;
	struct atl1c_buffer *buffer_info;
	int i, j;

	for (i = 0; i < AT_MAX_TRANSMIT_QUEUE; i++) {
		tpd_ring[i].next_to_use = 0;
		tpd_ring[i].next_to_clean = 0;
		for (j = 0; j < tpd_ring[i].count; j++){
			buffer_info = &tpd_ring[i].buffer_info[j];
			buffer_info->state = ATL1_BUFFER_FREE;
          }
	}
	for (i = 0; i < ap->num_rx_queues; i++) {
		rfd_ring[i].next_to_use = 0;
		rfd_ring[i].next_to_clean = 0;
		rrd_ring[i].next_to_use = 0;
		rrd_ring[i].next_to_clean = 0;
		for (j = 0; j < rfd_ring[i].count; j++) {
			buffer_info = &rfd_ring[i].buffer_info[j];
			buffer_info->state = ATL1_BUFFER_FREE;
		}
	}
}


/*
 * atl1c_setup_mem_resources - allocate Tx / RX descriptor resources
 * @adapter: board private structure
 *
 * Return 0 on success, negative on failure
 */
static int atl1c_setup_ring_resources(struct atl1c_private *ap)
{
	struct atl1c_tpd_ring *tpd_ring = ap->tpd_ring;
	struct atl1c_rfd_ring *rfd_ring = ap->rfd_ring;
	struct atl1c_rrd_ring *rrd_ring = ap->rrd_ring;
	struct atl1c_ring_header *ring_header = &ap->ring_header;
	int num_rx_queues = ap->num_rx_queues;
	int size;
	int i;
	int count = 0;
	int rx_desc_count = 0;
	u32 offset = 0;

	rrd_ring[0].count = rfd_ring[0].count;
	for (i = 1; i < AT_MAX_TRANSMIT_QUEUE; i++)
		tpd_ring[i].count = tpd_ring[0].count;

	for (i = 0; i < ap->num_rx_queues; i++)
		rfd_ring[i].count = rrd_ring[i].count = rfd_ring[0].count;

	/* 2 tpd queue, one high priority queue,
 	 * another normal priority queue */
	size = sizeof(struct atl1c_buffer) * (tpd_ring->count * 2 +
		rfd_ring->count * num_rx_queues);

    tpd_ring->buffer_info = malloc(size, M_DEVBUF, M_DONTWAIT);
	if (!tpd_ring->buffer_info) {
		printf("malloc failed, size = %d\n",size);
        free(tpd_ring->buffer_info, M_DEVBUF);
		goto err_nomem;
	}
	for (i = 0; i < AT_MAX_TRANSMIT_QUEUE; i++) {
		tpd_ring[i].buffer_info =
			(struct atl1c_buffer *) (tpd_ring->buffer_info + count);
		count += tpd_ring[i].count;
	}

	for (i = 0; i < num_rx_queues; i++) {
		rfd_ring[i].buffer_info =
			(struct atl1c_buffer *) (tpd_ring->buffer_info + count);
		count += rfd_ring[i].count;
		rx_desc_count += rfd_ring[i].count;
	}
	/*
	 * real ring DMA buffer
	 * each ring/block may need up to 8 bytes for alignment, hence the
	 * additional bytes tacked onto the end.
	 */
	ring_header->size = size =
		sizeof(struct atl1c_tpd_desc) * tpd_ring->count * 2 +
		sizeof(struct atl1c_rx_free_desc) * rx_desc_count +
		sizeof(struct atl1c_recv_ret_status) * rx_desc_count +
		sizeof(struct coals_msg_block) +
		8 * 4 + 8 * 2 * num_rx_queues;

	ring_header->desc = pci_alloc_consistent(NULL, ring_header->size,
				&ring_header->dma);
	if (!ring_header->desc) {
		printf("pci_alloc_consistend failed\n");
        pci_free_consistent(NULL, ring_header->size, ring_header->desc, ring_header->dma);
        goto err_nomem;
	}
	memset(ring_header->desc, 0, ring_header->size);
	/* init TPD ring */

	tpd_ring[0].dma = roundup(ring_header->dma, 8);
	offset = tpd_ring[0].dma - ring_header->dma;
	for (i = 0; i < AT_MAX_TRANSMIT_QUEUE; i++) {
		tpd_ring[i].dma = ring_header->dma + offset;
		tpd_ring[i].desc = (u8 *) ring_header->desc + offset;
		tpd_ring[i].size =
			sizeof(struct atl1c_tpd_desc) * tpd_ring[i].count;
		offset += roundup(tpd_ring[i].size, 8);
	}
	/* init RFD ring */
	for (i = 0; i < num_rx_queues; i++) {
		rfd_ring[i].dma = ring_header->dma + offset;
		rfd_ring[i].desc = (u8 *) ring_header->desc + offset;
		rfd_ring[i].size = sizeof(struct atl1c_rx_free_desc) *
				rfd_ring[i].count;
		offset += roundup(rfd_ring[i].size, 8);
	}

	/* init RRD ring */
	for (i = 0; i < num_rx_queues; i++) {
		rrd_ring[i].dma = ring_header->dma + offset;
		rrd_ring[i].desc = (u8 *) ring_header->desc + offset;
		rrd_ring[i].size = sizeof(struct atl1c_recv_ret_status) *
				rrd_ring[i].count;
		offset += roundup(rrd_ring[i].size, 8);
	}

	/* Init CMB dma address */
	ap->cmb.dma = ring_header->dma + offset;
	ap->cmb.cmb = (u8 *) ring_header->desc + offset;

	offset += roundup(sizeof(struct coals_msg_block), 8);
	ap->smb.dma = ring_header->dma + offset;
	ap->smb.smb = (u8 *)ring_header->desc + offset;
	return 0;

err_nomem:
	//kfree(tpd_ring->buffer_info);
	return -ENOMEM;
}

/*
 * atl1c_free_ring_resources - Free Tx / RX descriptor Resources
 * @adapter: board private structure
 *
 * Free all transmit software resources
 */
static void atl1c_free_ring_resources(struct atl1c_private *ap)
{
	pci_free_consistent(NULL, ap->ring_header.size,
					ap->ring_header.dma);
	ap->ring_header.desc = NULL;

	/* Note: just free tdp_ring.buffer_info,
	*  it contain rfd_ring.buffer_info, do not double free */
	if (ap->tpd_ring[0].buffer_info) {
		free(ap->tpd_ring[0].buffer_info, M_DEVBUF);
		ap->tpd_ring[0].buffer_info = NULL;
	}
}

static int atl1c_alloc_rx_buffer(struct atl1c_private *ap, const int ringid)
{
	struct atl1c_rfd_ring *rfd_ring = &ap->rfd_ring[ringid];
	struct atl1c_buffer *buffer_info;//, *next_info;
	struct sk_buff *skb;
	struct atl1c_rx_free_desc *rfd_desc;
    int i;

	buffer_info = &rfd_ring->buffer_info[0];

    for(i = 0; i<rfd_ring->count;i++){
		rfd_desc = ATL1C_RFD_DESC(rfd_ring, i);
		skb = dev_alloc_skb(ap->rx_buffer_len);
		if (!skb) {
		    printf("alloc rx buffer failed\n");
            goto alloc_rx_buffer_err;
		}
		/*
		 * Make buffer alignment 2 beyond a 16 byte boundary
		 * this will result in a 16 byte aligned IP header after
		 * the 14 byte MAC header is removed
		 */
		buffer_info[i].state = ATL1_BUFFER_BUSY;
		buffer_info[i].skb = skb;
		buffer_info[i].length = ap->rx_buffer_len;
		buffer_info[i].dma = pci_map_single(NULL, (void *)skb->data,
						buffer_info[i].length,
						PCI_DMA_FROMDEVICE);
		rfd_desc->buffer_addr = buffer_info[i].dma;
    }
    rfd_ring->next_to_use = rfd_ring->count - 1;    
	AT_WRITE_REG(&ap->hw, atl1c_rfd_prod_idx_regs[ringid],
	rfd_ring->next_to_use & MB_RFDX_PROD_IDX_MASK);
    return 1;
    
alloc_rx_buffer_err:
	return -ENOMEM;
}



//move foward the next buffer of rfd_ring.
static int atl1c_forward_rx_buffer(struct atl1c_private *ap, const int ringid, u16 count)

{
	struct atl1c_rfd_ring *rfd_ring = &ap->rfd_ring[ringid];
	u16 next_next;

    while(count){
	    next_next = rfd_ring->next_to_use;
	    if (++next_next == rfd_ring->count)
		    next_next = 0;
        rfd_ring->next_to_use = next_next;
        count--;
    }
    
    AT_WRITE_REG(&ap->hw, atl1c_rfd_prod_idx_regs[ringid],
			rfd_ring->next_to_use & MB_RFDX_PROD_IDX_MASK);

	return 1;
}

static void atl1c_configure_des_ring(const struct atl1c_private *ap)
{
	struct atl1c_hw *hw = (struct atl1c_hw *)&ap->hw;
	struct atl1c_rfd_ring *rfd_ring = (struct atl1c_rfd_ring *)
				ap->rfd_ring;
	struct atl1c_rrd_ring *rrd_ring = (struct atl1c_rrd_ring *)
				ap->rrd_ring;
	struct atl1c_tpd_ring *tpd_ring = (struct atl1c_tpd_ring *)
				ap->tpd_ring;
	struct atl1c_cmb *cmb = (struct atl1c_cmb *) &ap->cmb;
	struct atl1c_smb *smb = (struct atl1c_smb *) &ap->smb;
	int i;
	u32 data;

	/* TPD */
	AT_WRITE_REG(hw, REG_TX_BASE_ADDR_HI,
			(u32)((tpd_ring[atl1c_trans_normal].dma &
				AT_DMA_HI_ADDR_MASK) >> 32));    
	/* just enable normal priority TX queue */
	AT_WRITE_REG(hw, REG_NTPD_HEAD_ADDR_LO,
			(u32)(tpd_ring[atl1c_trans_normal].dma &
				AT_DMA_LO_ADDR_MASK));

    AT_WRITE_REG(hw, REG_HTPD_HEAD_ADDR_LO,
			(u32)(tpd_ring[atl1c_trans_high].dma &
				AT_DMA_LO_ADDR_MASK));

	AT_WRITE_REG(hw, REG_TPD_RING_SIZE,
			(u32)(tpd_ring[0].count & TPD_RING_SIZE_MASK));

	/* RFD */
	AT_WRITE_REG(hw, REG_RX_BASE_ADDR_HI,
			(u32)((rfd_ring[0].dma & AT_DMA_HI_ADDR_MASK) >> 32));
	for (i = 0; i < ap->num_rx_queues; i++)
		AT_WRITE_REG(hw, atl1c_rfd_addr_lo_regs[i],
			(u32)(rfd_ring[i].dma & AT_DMA_LO_ADDR_MASK));

	AT_WRITE_REG(hw, REG_RFD_RING_SIZE,
			rfd_ring[0].count & RFD_RING_SIZE_MASK);
   
	AT_WRITE_REG(hw, REG_RX_BUF_SIZE,
			ap->rx_buffer_len & RX_BUF_SIZE_MASK);

	/* RRD */
	for (i = 0; i < ap->num_rx_queues; i++)
		AT_WRITE_REG(hw, atl1c_rrd_addr_lo_regs[i],
			(u32)(rrd_ring[i].dma & AT_DMA_LO_ADDR_MASK));
	AT_WRITE_REG(hw, REG_RRD_RING_SIZE,
			(rrd_ring[0].count & RRD_RING_SIZE_MASK));

	/* CMB */
	AT_WRITE_REG(hw, REG_CMB_BASE_ADDR_LO, cmb->dma & AT_DMA_LO_ADDR_MASK);

	/* SMB */
	AT_WRITE_REG(hw, REG_SMB_BASE_ADDR_HI,
			(u32)((smb->dma & AT_DMA_HI_ADDR_MASK) >> 32));
	AT_WRITE_REG(hw, REG_SMB_BASE_ADDR_LO,
			(u32)(smb->dma & AT_DMA_LO_ADDR_MASK));
	if (hw->nic_type == athr_l2c_b) {
		AT_WRITE_REG(hw, REG_SRAM_RXF_LEN, 0x02a0L);
		AT_WRITE_REG(hw, REG_SRAM_TXF_LEN, 0x0100L);
		AT_WRITE_REG(hw, REG_SRAM_RXF_ADDR, 0x029f0000L);
		AT_WRITE_REG(hw, REG_SRAM_RFD0_INFO, 0x02bf02a0L);
		AT_WRITE_REG(hw, REG_SRAM_TXF_ADDR, 0x03bf02c0L);
		AT_WRITE_REG(hw, REG_SRAM_TRD_ADDR, 0x03df03c0L);
		AT_WRITE_REG(hw, REG_TXF_WATER_MARK, 0); /* TX watermark, to enter l1 state. */
		AT_WRITE_REG(hw, REG_RXD_DMA_CTRL, 0);   /* RXD threshold. */
		
	}
	if (hw->nic_type == athr_l2c_b || hw->nic_type == athr_l2c_b2) {
			/* Power Saving for L2c_B */
		AT_READ_REG(hw, REG_SERDES_LOCK, &data);
		data |= SERDES_MAC_CLK_SLOWDOWN;
		data |= SERDES_PYH_CLK_SLOWDOWN;
		AT_WRITE_REG(hw, REG_SERDES_LOCK, data);
	}
	/* Load all of base address above */
	AT_WRITE_REG(hw, REG_LOAD_PTR, 1);

	return;
}

static void atl1c_configure_tx(struct atl1c_private *ap)
{
	struct atl1c_hw *hw = (struct atl1c_hw *)&ap->hw;
	u32 dev_ctrl_data;
	u32 max_pay_load;
	u16 tx_offload_thresh;
	u32 txq_ctrl_data;
	u32 extra_size = 0;     /* Jumbo frame threshold in QWORD unit */
	u32 max_pay_load_data;

	extra_size = ETH_HLEN+ VLAN_HLEN + ETH_FCS_LEN;
	tx_offload_thresh = MAX_TX_OFFLOAD_THRESH;
	AT_WRITE_REG(hw, REG_TX_TSO_OFFLOAD_THRESH,
		(tx_offload_thresh >> 3) & TX_TSO_OFFLOAD_THRESH_MASK);
	AT_READ_REG(hw, REG_DEVICE_CTRL, &dev_ctrl_data);
	max_pay_load  = (dev_ctrl_data >> DEVICE_CTRL_MAX_PAYLOAD_SHIFT) &
			DEVICE_CTRL_MAX_PAYLOAD_MASK;
	hw->dmaw_block = min(max_pay_load, hw->dmaw_block);
	max_pay_load  = (dev_ctrl_data >> DEVICE_CTRL_MAX_RREQ_SZ_SHIFT) &
			DEVICE_CTRL_MAX_RREQ_SZ_MASK;
	hw->dmar_block = min(max_pay_load, hw->dmar_block);

	txq_ctrl_data = (hw->tpd_burst & TXQ_NUM_TPD_BURST_MASK) <<
			TXQ_NUM_TPD_BURST_SHIFT;
	if (hw->ctrl_flags & ATL1C_TXQ_MODE_ENHANCE)
		txq_ctrl_data |= TXQ_CTRL_ENH_MODE;
	max_pay_load_data = (atl1c_pay_load_size[hw->dmar_block] &
                        TXQ_TXF_BURST_NUM_MASK) << TXQ_TXF_BURST_NUM_SHIFT;
	if (hw->nic_type == athr_l2c_b || hw->nic_type == athr_l2c_b2)
		max_pay_load_data >>= 1;
	txq_ctrl_data |= max_pay_load_data; 

	AT_WRITE_REG(hw, REG_TXQ_CTRL, txq_ctrl_data);

	return;
}

static void atl1c_configure_rx(struct atl1c_private *ap)
{
	struct atl1c_hw *hw = (struct atl1c_hw *)&ap->hw;
	u32 rxq_ctrl_data;

	
	rxq_ctrl_data = (hw->rfd_burst & RXQ_RFD_BURST_NUM_MASK) <<
			RXQ_RFD_BURST_NUM_SHIFT;
	
	if (hw->ctrl_flags & ATL1C_RX_IPV6_CHKSUM)
		rxq_ctrl_data |= IPV6_CHKSUM_CTRL_EN;
	if (hw->rss_type == atl1c_rss_ipv4)
		rxq_ctrl_data |= RSS_HASH_IPV4;
	if (hw->rss_type == atl1c_rss_ipv4_tcp)
		rxq_ctrl_data |= RSS_HASH_IPV4_TCP;
	if (hw->rss_type == atl1c_rss_ipv6)
		rxq_ctrl_data |= RSS_HASH_IPV6;
	if (hw->rss_type == atl1c_rss_ipv6_tcp)
		rxq_ctrl_data |= RSS_HASH_IPV6_TCP;
	if (hw->rss_type != atl1c_rss_disable)
		rxq_ctrl_data |= RRS_HASH_CTRL_EN;

	rxq_ctrl_data |= (hw->rss_mode & RSS_MODE_MASK) <<
			RSS_MODE_SHIFT;
	rxq_ctrl_data |= (hw->rss_hash_bits & RSS_HASH_BITS_MASK) <<
			RSS_HASH_BITS_SHIFT;
	if (hw->ctrl_flags & ATL1C_ASPM_CTRL_MON) 
		rxq_ctrl_data |= (ASPM_THRUPUT_LIMIT_1M & 
			ASPM_THRUPUT_LIMIT_MASK) << ASPM_THRUPUT_LIMIT_SHIFT;

	AT_WRITE_REG(hw, REG_RXQ_CTRL, rxq_ctrl_data);

	return;
}

static void atl1c_configure_rss(struct atl1c_private *ap)
{
	struct atl1c_hw *hw = (struct atl1c_hw *)&ap->hw;

	AT_WRITE_REG(hw, REG_IDT_TABLE, hw->indirect_tab);
	AT_WRITE_REG(hw, REG_BASE_CPU_NUMBER, hw->base_cpu);

	return;
}

static void atl1c_configure_dma(struct atl1c_private *ap)
{
	struct atl1c_hw *hw = &ap->hw;
	u32 dma_ctrl_data;

	dma_ctrl_data = DMA_CTRL_DMAR_REQ_PRI;
	if (hw->ctrl_flags & ATL1C_CMB_ENABLE)
		dma_ctrl_data |= DMA_CTRL_CMB_EN;
	if (hw->ctrl_flags & ATL1C_SMB_ENABLE)
		dma_ctrl_data |= DMA_CTRL_SMB_EN;
	else
		dma_ctrl_data |= MAC_CTRL_SMB_DIS;

	switch (hw->dma_order) {
	case atl1c_dma_ord_in:
		dma_ctrl_data |= DMA_CTRL_DMAR_IN_ORDER;
		break;
	case atl1c_dma_ord_enh:
		dma_ctrl_data |= DMA_CTRL_DMAR_ENH_ORDER;
		break;
	case atl1c_dma_ord_out:
		dma_ctrl_data |= DMA_CTRL_DMAR_OUT_ORDER;
		break;
	default:
		break;
	}

	dma_ctrl_data |= (((u32)hw->dmar_block) & DMA_CTRL_DMAR_BURST_LEN_MASK)
		<< DMA_CTRL_DMAR_BURST_LEN_SHIFT;
	dma_ctrl_data |= (((u32)hw->dmaw_block) & DMA_CTRL_DMAW_BURST_LEN_MASK)
		<< DMA_CTRL_DMAW_BURST_LEN_SHIFT;
	dma_ctrl_data |= (((u32)hw->dmar_dly_cnt) & DMA_CTRL_DMAR_DLY_CNT_MASK)
		<< DMA_CTRL_DMAR_DLY_CNT_SHIFT;
	dma_ctrl_data |= (((u32)hw->dmaw_dly_cnt) & DMA_CTRL_DMAW_DLY_CNT_MASK)
		<< DMA_CTRL_DMAW_DLY_CNT_SHIFT;

	AT_WRITE_REG(hw, REG_DMA_CTRL, dma_ctrl_data);

	return;
}

/*
 * atl1c_configure - Configure Transmit&Receive Unit after Reset
 * @adapter: board private structure
 *
 * Configure the Tx /Rx unit of the MAC after a reset.
 */
static int atl1c_configure(struct atl1c_private *ap)
{
	struct atl1c_hw *hw = &ap->hw;
	u32 master_ctrl_data = 0;
	u32 intr_modrt_data;
    
	/* clear interrupt status */
	AT_WRITE_REG(hw, REG_ISR, 0xFFFFFFFF);
	/*  Clear any WOL status */
	AT_WRITE_REG(hw, REG_WOL_CTRL, 0);
	/* set Interrupt Clear Timer
	 * HW will enable self to assert interrupt event to system after
	 * waiting x-time for software to notify it accept interrupt.
	 */

	AT_WRITE_REG(hw, REG_INT_RETRIG_TIMER,
		hw->ict & INT_RETRIG_TIMER_MASK);

	atl1c_configure_des_ring(ap);

	if (hw->ctrl_flags & ATL1C_INTR_MODRT_ENABLE) {
		intr_modrt_data = (hw->tx_imt & IRQ_MODRT_TIMER_MASK) <<
					IRQ_MODRT_TX_TIMER_SHIFT;
		intr_modrt_data |= (hw->rx_imt & IRQ_MODRT_TIMER_MASK) <<
					IRQ_MODRT_RX_TIMER_SHIFT;
		AT_WRITE_REG(hw, REG_IRQ_MODRT_TIMER_INIT, intr_modrt_data);
		master_ctrl_data |=
			MASTER_CTRL_TX_ITIMER_EN | MASTER_CTRL_RX_ITIMER_EN;
	}

	if (hw->ctrl_flags & ATL1C_INTR_CLEAR_ON_READ)
		master_ctrl_data |= MASTER_CTRL_INT_RDCLR;
	
	master_ctrl_data |= MASTER_CTRL_SA_TIMER_EN;
	AT_WRITE_REG(hw, REG_MASTER_CTRL, master_ctrl_data);

	if (hw->ctrl_flags & ATL1C_CMB_ENABLE) {
		AT_WRITE_REG(hw, REG_CMB_TPD_THRESH,
			hw->cmb_tpd & CMB_TPD_THRESH_MASK);
		AT_WRITE_REG(hw, REG_CMB_TX_TIMER,
			hw->cmb_tx_timer & CMB_TX_TIMER_MASK);
	}

	if (hw->ctrl_flags & ATL1C_SMB_ENABLE)
		AT_WRITE_REG(hw, REG_SMB_STAT_TIMER,
			hw->smb_timer & SMB_STAT_TIMER_MASK);

    /* set MTU */
	AT_WRITE_REG(hw, REG_MTU, hw->max_frame_size + ETH_HLEN +
			VLAN_HLEN + ETH_FCS_LEN);
    
	/* HDS, disable */
	AT_WRITE_REG(hw, REG_HDS_CTRL, 0);
	
	atl1c_configure_tx(ap);
	atl1c_configure_rx(ap);
	atl1c_configure_rss(ap);
	atl1c_configure_dma(ap);
	
	return 0;
}

static inline void atl1c_clear_phy_int(struct atl1c_private *ap)
{
	u16 phy_data;

	atl1c_read_phy_reg(&ap->hw, MII_ISR, &phy_data);
}

static void atl1c_clean_rrd(struct atl1c_rrd_ring *rrd_ring,
			struct	atl1c_recv_ret_status *rrs, u16 num)
{
	u16 i;
	/* the relationship between rrd and rfd is one map one */
	for (i = 0; i < num; i++, rrs = ATL1C_RRD_DESC(rrd_ring,
					rrd_ring->next_to_clean)) {
		rrs->word3 &= ~RRS_RXD_UPDATED;
		if (++rrd_ring->next_to_clean == rrd_ring->count)
			rrd_ring->next_to_clean = 0;
	}
	return;
}

static void atl1c_clean_rfd(struct atl1c_rfd_ring *rfd_ring,
	struct atl1c_recv_ret_status *rrs, u16 num)
{
	u16 i;
	u16 rfd_index;

	rfd_index = (rrs->word0 >> RRS_RX_RFD_INDEX_SHIFT) &
			RRS_RX_RFD_INDEX_MASK;
	for (i = 0; i < num; i++) {
		if (++rfd_index == rfd_ring->count)
			rfd_index = 0;
	}
	rfd_ring->next_to_clean = rfd_index;

	return;
}

static void atl1c_clean_rx_irq(struct atl1c_private *ap, u8 que)
{
	u16 rfd_num, rfd_index;
	u16 count = 0;
	u16 length;
	struct atl1c_rfd_ring *rfd_ring = &ap->rfd_ring[que];
	struct atl1c_rrd_ring *rrd_ring = &ap->rrd_ring[que];
	struct sk_buff *skb;
	struct atl1c_recv_ret_status *rrs;
	struct atl1c_buffer *buffer_info;
    u16 u_cons_idx;
    u32 data0;
    
    switch(que){
        case 0:
            AT_READ_REG(&ap->hw, REG_MB_RFD01_CONS_IDX,&data0);
            u_cons_idx = (data0&MB_RFD0_CONS_IDX_MASK)>>MB_RFD0_CONS_IDX_SHIFT;
            break;
        case 1:
            AT_READ_REG(&ap->hw, REG_MB_RFD01_CONS_IDX,&data0);
            u_cons_idx = (data0&MB_RFD1_CONS_IDX_MASK)>>MB_RFD1_CONS_IDX_SHIFT;
            break;

        case 2:
            AT_READ_REG(&ap->hw, REG_MB_RFD23_CONS_IDX,&data0);
            u_cons_idx = (data0&MB_RFD2_CONS_IDX_MASK)>>MB_RFD2_CONS_IDX_SHIFT;
            break;
        case 3:
            AT_READ_REG(&ap->hw, REG_MB_RFD23_CONS_IDX,&data0);
            u_cons_idx = (data0&MB_RFD3_CONS_IDX_MASK)>>MB_RFD3_CONS_IDX_SHIFT;
            break;
            
       default:
            AT_READ_REG(&ap->hw, REG_MB_RFD01_CONS_IDX,&data0);
            u_cons_idx = (data0&MB_RFD0_CONS_IDX_MASK)>>MB_RFD0_CONS_IDX_SHIFT;
            break;
        }
    while(rrd_ring->next_to_clean != u_cons_idx){
		rrs = ATL1C_RRD_DESC(rrd_ring, rrd_ring->next_to_clean);    
		if (RRS_RXD_IS_VALID(rrs->word3)) {
			rfd_num = (rrs->word0 >> RRS_RX_RFD_CNT_SHIFT) &
				RRS_RX_RFD_CNT_MASK;
			if (rfd_num != 1)
				/* TODO support mul rfd*/
				printf("Multi rfd not support yet!\n");
			goto rrs_checked;
		} else {
			break;
		}
rrs_checked:
		atl1c_clean_rrd(rrd_ring, rrs, rfd_num);
		if (rrs->word3 & (RRS_RX_ERR_SUM | RRS_802_3_LEN_ERR)) {
			atl1c_clean_rfd(rfd_ring, rrs, rfd_num);
			printf( "wrong packet! rrs word3 is %x\n", rrs->word3);
			continue;
		}

		length =(rrs->word3 >> RRS_PKT_SIZE_SHIFT) & RRS_PKT_SIZE_MASK;		
        /* Good Receive */
		if (rfd_num == 1) {
			rfd_index = (rrs->word0 >> RRS_RX_RFD_INDEX_SHIFT) &
					RRS_RX_RFD_INDEX_MASK;
			buffer_info = &rfd_ring->buffer_info[rfd_index];
			skb = buffer_info->skb;
		} else {
			/* TODO */
			printf( "Multil rfd not support yet!\n");
			break;
		}        
            
		atl1c_clean_rfd(rfd_ring, rrs, rfd_num);
        skb->len = length;
        skb->ap = ap;        
        netif_rx(skb);
		count++;
	}
	if (count)
		atl1c_forward_rx_buffer(ap, que, count);
}

/*
 * atl1c_irq_enable - Enable default interrupt generation settings
 * @adapter: board private structure
 */
static void atl1c_irq_enable(struct atl1c_private *ap)
{
    AT_WRITE_REG(&ap->hw, REG_ISR, 0x7FFFFFFF);
    AT_WRITE_REG(&ap->hw, REG_IMR, ap->hw.intr_mask);
    AT_WRITE_FLUSH(&ap->hw);
}
int atl1c_up(struct atl1c_private *ap)
{
	int num;
	int err = 0;
	int i;

	atl1c_init_ring_ptrs(ap);

	for (i = 0; i < ap->num_rx_queues; i++) {
		num = atl1c_alloc_rx_buffer(ap, i);
		if (num == 0) {
			err = -ENOMEM;
			goto err_alloc_rx;
		}
	}
    
	if (atl1c_configure(ap)) {
		err = -EIO;
		goto err_up;
	}
    
	atl1c_irq_enable(ap);
	atl1c_check_link_status(ap);
	return err;

err_up:
err_alloc_rx:
	atl1c_clean_rx_ring(ap);
	return err;
}

void atl1c_down(struct atl1c_private *ap)
{

	atl1c_irq_disable(ap);
	/* reset MAC to disable all RX/TX */
	atl1c_reset_mac(&ap->hw);
	mdelay(1);

	ap->link_speed = SPEED_0;
	ap->link_duplex = -1 ;
	atl1c_clean_tx_ring(ap, atl1c_trans_normal);
	atl1c_clean_tx_ring(ap, atl1c_trans_high);
	atl1c_clean_rx_ring(ap);
}

/*
 * atl1c_open - Called when a network interface is made active
 * @netdev: network interface device structure
 *
 * Returns 0 on success, negative value on failure
 *
 * The open entry point is called when a network interface is made
 * active by the system (IFF_UP).  At this point all resources needed
 * for transmit and receive operations are allocated, the interrupt
 * handler is registered with the OS, the watchdog timer is started,
 * and the stack is notified that the interface is ready.
 */
static int atl1c_open(struct atl1c_private *ap)
{
	int err = 0;
    int num,i;

	/* allocate rx/tx dma buffer & descriptors */
	err = atl1c_setup_ring_resources(ap);
	if (err)
		return err;

	atl1c_init_ring_ptrs(ap);

	for (i = 0; i < ap->num_rx_queues; i++) {
		num = atl1c_alloc_rx_buffer(ap, i);
		if (num == 0) {
			err = -ENOMEM;
			goto err_up;
		}
	}

	if (atl1c_configure(ap)) {
		err = -EIO;
		goto err_up;
	}

	atl1c_irq_enable(ap);
	atl1c_check_link_status(ap);

	if (ap->hw.ctrl_flags & ATL1C_FPGA_VERSION) {
		u32 phy_data;

		AT_READ_REG(&ap->hw, 0x1414, &phy_data);
		phy_data |= 0x10000000;
		AT_WRITE_REG(&ap->hw, 0x1414, phy_data);
	}

    ap->arpcom.ac_if.if_flags |=  IFF_RUNNING;
    
	return 0;

err_up:
	atl1c_free_ring_resources(ap);
	atl1c_reset_mac(&ap->hw);
	return err;
}

static int atl1c_ether_ioctl(struct ifnet *ifp, unsigned long cmd, caddr_t data)
{
	struct ifaddr *ifa = (struct ifaddr *) data;
	struct atl1c_private *ap = ifp->if_softc;
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
			ifp->if_flags |= IFF_UP;
#ifdef __OpenBSD__
			arp_ifinit(&ap->arpcom, ifa);
			printf("\n");
#else
			arp_ifinit(ifp, ifa);
#endif
			
			break;
#endif

		default:
		       atl1c_open(ap);
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
             atl1c_open(ap);
		}
		break;
	default:
		error = EINVAL;
	}

	splx(s);

	return (error);
}


/*
 * get next usable tpd
 * Note: should call atl1c_tdp_avail to make sure
 * there is enough tpd to use
 */
static struct atl1c_tpd_desc *atl1c_get_tpd(struct atl1c_private *ap,
	enum atl1c_trans_queue type)
{
	struct atl1c_tpd_ring *tpd_ring = &ap->tpd_ring[type];
	struct atl1c_tpd_desc *tpd_desc;
	u16 next_to_use = 0;

	next_to_use = tpd_ring->next_to_use;
	if (++tpd_ring->next_to_use == tpd_ring->count)
		tpd_ring->next_to_use = 0;
  
	tpd_desc = ATL1C_TPD_DESC(tpd_ring, next_to_use);
	memset(tpd_desc, 0, sizeof(struct atl1c_tpd_desc));
	return	tpd_desc;
}

static struct atl1c_buffer *atl1c_get_tx_buffer(struct atl1c_private *ap, struct atl1c_tpd_desc *tpd)
{
	struct atl1c_tpd_ring *tpd_ring = ap->tpd_ring;

	return &tpd_ring->buffer_info[tpd -
			(struct atl1c_tpd_desc *)tpd_ring->desc];
}


#define ATL1C_MAX_TXD_PWR	12
#define ATL1C_MAX_DATA_PER_TXD	(1<<ATL1C_MAX_TXD_PWR)
static void atl1c_tx_map(struct atl1c_private *ap,
		      struct sk_buff *skb, enum atl1c_trans_queue type)
{
	struct atl1c_tpd_desc *use_tpd = NULL;
	struct atl1c_buffer *buffer_info = NULL;
	u16 len = skb->len;
	unsigned int offset = 0, size;
    

	while(len) {

		use_tpd = atl1c_get_tpd(ap, type);
		buffer_info = atl1c_get_tx_buffer(ap, use_tpd);
		size = len < ATL1C_MAX_DATA_PER_TXD ? len : ATL1C_MAX_DATA_PER_TXD;
		buffer_info->length = size;
		buffer_info->dma = pci_map_single(NULL, skb->data + offset,	size, PCI_DMA_TODEVICE);
		buffer_info->state = ATL1_BUFFER_BUSY;
        len -= size;
        offset += size;

		use_tpd->buffer_addr = buffer_info->dma;
		use_tpd->buffer_len  = buffer_info->length;
    }

	/* The last tpd */
	use_tpd->word1 |= 1 << TPD_EOP_SHIFT;
	/* The last buffer info contain the skb address,
	   so it will be free after unmap */
	buffer_info->skb = skb;
}

static void atl1c_tx_queue(struct atl1c_private *ap, enum atl1c_trans_queue type)
{
	struct atl1c_tpd_ring *tpd_ring = &ap->tpd_ring[type];
	u32 prod_data;
    
	AT_READ_REG(&ap->hw, REG_MB_PRIO_PROD_IDX, &prod_data);

    
	switch (type) {
	case atl1c_trans_high:
		prod_data &= 0xFFFF0000;
		prod_data |= tpd_ring->next_to_use & 0xFFFF;
		break;
	case atl1c_trans_normal:
		prod_data &= 0x0000FFFF;
		prod_data |= (tpd_ring->next_to_use & 0xFFFF) << 16;
		break;
	default:
		break;
	}

	AT_WRITE_REG(&ap->hw, REG_MB_PRIO_PROD_IDX, prod_data);

}

static int atl1c_xmit_frame(struct sk_buff *skb, struct atl1c_private *ap)
{    
	atl1c_tx_map(ap, skb, atl1c_trans_normal);
	atl1c_tx_queue(ap, atl1c_trans_normal);
	return NETDEV_TX_OK;
}

static void atl1c_start(struct ifnet *ifp)
{

	struct atl1c_private *ap = (struct atl1c_private *)ifp->if_softc;
	struct mbuf *mb_head;		
	struct sk_buff *skb;

	while(ifp->if_snd.ifq_head != NULL ){
        IF_DEQUEUE(&ifp->if_snd, mb_head);
		skb=dev_alloc_skb(mb_head->m_pkthdr.len);
		m_copydata(mb_head, 0, mb_head->m_pkthdr.len, skb->data);
		skb->len=mb_head->m_pkthdr.len;
        atl1c_xmit_frame(skb,ap);
        m_freem(mb_head);
        wbflush();
	} 
}

static int atl1c_phy_setup_adv(struct atl1c_hw *hw)
{
	u16 mii_adv_data = ADVERTISE_DEFAULT_CAP & ~ADVERTISE_SPEED_MASK;
	u16 mii_giga_ctrl_data = GIGA_CR_1000T_DEFAULT_CAP &
				~GIGA_CR_1000T_SPEED_MASK;
	u16 tmp_data;

	if (hw->autoneg_advertised & ADVERTISE_10_HALF)
		mii_adv_data |= ADVERTISE_10T_HD_CAPS;
	if (hw->autoneg_advertised & ADVERTISE_10_FULL)
		mii_adv_data |= ADVERTISE_10T_FD_CAPS;
	if (hw->autoneg_advertised & ADVERTISE_100_HALF)
		mii_adv_data |= ADVERTISE_100TX_HD_CAPS;
	if (hw->autoneg_advertised & ADVERTISE_100_FULL)
		mii_adv_data |= ADVERTISE_100TX_FD_CAPS;
	if (hw->link_cap_flags & LINK_CAP_SPEED_1000) {
		if (hw->autoneg_advertised & ADVERTISE_1000_HALF)
			mii_giga_ctrl_data |= GIGA_CR_1000T_HD_CAPS;
		if (hw->autoneg_advertised & ADVERTISE_1000_FULL)
			mii_giga_ctrl_data |= GIGA_CR_1000T_FD_CAPS;
	}
	if (hw->nic_type == athr_l2c_b2) {
		atl1c_read_phy_reg(hw, MII_BMSR, &tmp_data);
		tmp_data |= 0x02;
		atl1c_write_phy_reg(hw, 0x3C, tmp_data);
	}	
	if (atl1c_write_phy_reg(hw, MII_ADVERTISE, mii_adv_data) != 0 ||
	    atl1c_write_phy_reg(hw, MII_GIGA_CR, mii_giga_ctrl_data) != 0)
		return AT_ERR_PHY_SPEED;
	return 0;
}

static void atl1c_phy_magic_data(struct atl1c_hw *hw)
{
	u16 data;

	atl1c_write_phy_reg(hw, MII_DBG_ADDR, 0x12);
	atl1c_write_phy_reg(hw, MII_DBG_DATA, 0x4C04);

	atl1c_write_phy_reg(hw, MII_DBG_ADDR, 0x05);
	atl1c_write_phy_reg(hw, MII_DBG_DATA, 0x2C46);

	atl1c_write_phy_reg(hw, MII_DBG_ADDR, 0x36);
	atl1c_write_phy_reg(hw, MII_DBG_DATA, 0xE12C);

	atl1c_write_phy_reg(hw, MII_DBG_ADDR, 0x04);
	atl1c_write_phy_reg(hw, MII_DBG_DATA, 0x88BB);

	atl1c_write_phy_reg(hw, MII_DBG_ADDR, 0x00);
	atl1c_write_phy_reg(hw, MII_DBG_DATA, 0x02EF);
	
	atl1c_write_phy_reg(hw, MII_DBG_ADDR, 0x36);
	if (atl1c_read_phy_reg(hw, MII_DBG_DATA, &data) != 0)
        	return;
	data |= 0x80;
	atl1c_write_phy_reg(hw, MII_DBG_DATA, data);
		
	if (hw->ctrl_flags & ATL1C_HIB_DISABLE) {
		atl1c_write_phy_reg(hw, MII_DBG_ADDR, 0x29);
		if (atl1c_read_phy_reg(hw, MII_DBG_DATA, &data) != 0)
			return;
		data &= 0x7FFF;
		atl1c_write_phy_reg(hw, MII_DBG_DATA, data);
	
		atl1c_write_phy_reg(hw, MII_DBG_ADDR, 0xB);
		if (atl1c_read_phy_reg(hw, MII_DBG_DATA, &data) != 0)
			return;
		data &= 0x7FFF;
		atl1c_write_phy_reg(hw, MII_DBG_DATA, data);
	}
}	

int atl1c_phy_reset(struct atl1c_hw *hw)
{
	u16 phy_data;
	u32 phy_ctrl_data = GPHY_CTRL_DEFAULT;
	u32 mii_ier_data = IER_LINK_UP | IER_LINK_DOWN;
	int err;

	if (hw->ctrl_flags & ATL1C_HIB_DISABLE)
		phy_ctrl_data &= ~GPHY_CTRL_HIB_EN;
 
	AT_WRITE_REG(hw, REG_GPHY_CTRL, phy_ctrl_data);
	AT_WRITE_FLUSH(hw);
	mdelay(40);
	phy_ctrl_data |= GPHY_CTRL_EXT_RESET;
	AT_WRITE_REG(hw, REG_GPHY_CTRL, phy_ctrl_data);
	AT_WRITE_FLUSH(hw);
	mdelay(40);

	if (hw->nic_type == athr_l2c_b) {
		atl1c_write_phy_reg(hw, MII_DBG_ADDR, 0x0A);
		atl1c_read_phy_reg(hw, MII_DBG_DATA, &phy_data);
		atl1c_write_phy_reg(hw, MII_DBG_DATA, phy_data & 0xDFFF);
		
	}
	if (hw->nic_type == athr_l2c_b || hw->nic_type == athr_l2c_b2 ||
		hw->nic_type == athr_l1d) {	
		atl1c_write_phy_reg(hw, MII_DBG_ADDR, 0x3B);
		atl1c_read_phy_reg(hw, MII_DBG_DATA, &phy_data);
		atl1c_write_phy_reg(hw, MII_DBG_DATA, phy_data & 0xFFF7);
		mdelay(20);
	}
	if (hw->nic_type == athr_l1d) {
		atl1c_write_phy_reg(hw, MII_DBG_ADDR, 0x29);
		atl1c_write_phy_reg(hw, MII_DBG_DATA, 0x929D);
	}
	if (hw->nic_type == athr_l1c || hw->nic_type == athr_l2c_b2
		|| hw->nic_type == athr_l2c || hw->nic_type == athr_l2c) {
		atl1c_write_phy_reg(hw, MII_DBG_ADDR, 0x29);
		atl1c_write_phy_reg(hw, MII_DBG_DATA, 0xB6DD);
	}
	err = atl1c_write_phy_reg(hw, MII_IER, mii_ier_data);
	if (err) {
		printf("Error enable PHY linkChange Interrupt\n");
        return err;
	}
	if (!(hw->ctrl_flags & ATL1C_FPGA_VERSION))
		atl1c_phy_magic_data(hw);
	return 0; 
}

/*
 * Stop the mac, transmit and receive units
 * hw - Struct containing variables accessed by shared code
 * return : 0  or  idle status (if error)
 */
static int atl1c_stop_mac(struct atl1c_hw *hw)
{
	u32 data;
	int timeout;

	AT_READ_REG(hw, REG_RXQ_CTRL, &data);
	data &= ~(RXQ1_CTRL_EN | RXQ2_CTRL_EN |
                  RXQ3_CTRL_EN | RXQ_CTRL_EN);
	AT_WRITE_REG(hw, REG_RXQ_CTRL, data);

	AT_READ_REG(hw, REG_TXQ_CTRL, &data);
	data &= ~TXQ_CTRL_EN;
	AT_WRITE_REG(hw, REG_TXQ_CTRL, data);
 
	for (timeout = 0; timeout < AT_HW_MAX_IDLE_DELAY; timeout++) {
		AT_READ_REG(hw, REG_IDLE_STATUS, &data);
		if ((data & (IDLE_STATUS_RXQ_NO_IDLE |
			IDLE_STATUS_TXQ_NO_IDLE)) == 0)
			break;
		mdelay(5);
	}

	AT_READ_REG(hw, REG_MAC_CTRL, &data);
	data &= ~(MAC_CTRL_TX_EN | MAC_CTRL_RX_EN);
	AT_WRITE_REG(hw, REG_MAC_CTRL, data);

	for (timeout = 0; timeout < AT_HW_MAX_IDLE_DELAY; timeout++) {
		AT_READ_REG(hw, REG_IDLE_STATUS, &data);
		if ((data & IDLE_STATUS_MASK) == 0)
			return 0;
		mdelay(5);
	}
	return data;
}
/*
 * Reset the transmit and receive units; mask and clear all interrupts.
 * hw - Struct containing variables accessed by shared code
 * return : 0  or  idle status (if error)
 */
static int atl1c_reset_mac(struct atl1c_hw *hw)
{
	u32 idle_status_data = 0;
	u32 master_ctrl_data = 0;
	int timeout = 0;

	AT_WRITE_REG(hw, REG_IMR, 0);
	AT_WRITE_REG(hw, REG_ISR, ISR_DIS_INT);

	atl1c_stop_mac(hw);
	/*
	 * Issue Soft Reset to the MAC.  This will reset the chip's
	 * transmit, receive, DMA.  It will not effect
	 * the current PCI configuration.  The global reset bit is self-
	 * clearing, and should clear within a microsecond.
	 */
	AT_READ_REG(hw, REG_MASTER_CTRL, &master_ctrl_data);
	master_ctrl_data |= MASTER_CTRL_OOB_DIS_OFF;
	AT_WRITE_REGW(hw, REG_MASTER_CTRL, ((master_ctrl_data | MASTER_CTRL_SOFT_RST)
			& 0xFFFF));
	mdelay(10);
	/* Wait at least 10ms for All module to be Idle */
	for (timeout = 0; timeout < AT_HW_MAX_IDLE_DELAY; timeout++) {
		AT_READ_REG(hw, REG_IDLE_STATUS, &idle_status_data);
		if ((idle_status_data & IDLE_STATUS_MASK) == 0)
			break;
	    mdelay(10);
	}
	if (timeout >= AT_HW_MAX_IDLE_DELAY) {
		printf("MAC state machine cann't be idle since, disabled for 10ms second\n");
        return AT_ERR_TIMEOUT;
	}
	return 0;
}

int atl1c_phy_init(struct atl1c_hw * hw)
{
	int ret_val;
	u16 mii_bmcr_data = BMCR_RESET;

	if ((atl1c_read_phy_reg(hw, MII_PHYSID1, &hw->phy_id1) != 0) ||
		(atl1c_read_phy_reg(hw, MII_PHYSID2, &hw->phy_id2) != 0)) {
		printf("Error get phy ID\n");
        return -1;
	}
    
	switch (hw->media_type) {
	case MEDIA_TYPE_AUTO_SENSOR:
	case MEDIA_TYPE_1000M_FULL:
		ret_val = atl1c_phy_setup_adv(hw);
		if (ret_val) {
			printf("Error Setting up Auto-Negotiation\n");
            return ret_val;
		}
		mii_bmcr_data |= BMCR_AUTO_NEG_EN | BMCR_RESTART_AUTO_NEG;
		break;
	case MEDIA_TYPE_100M_FULL:
		mii_bmcr_data |= BMCR_SPEED_100 | BMCR_FULL_DUPLEX;
		break;
	case MEDIA_TYPE_100M_HALF:
		mii_bmcr_data |= BMCR_SPEED_100;
		break;
	case MEDIA_TYPE_10M_FULL:
		mii_bmcr_data |= BMCR_SPEED_10 | BMCR_FULL_DUPLEX;
		break;
	case MEDIA_TYPE_10M_HALF:
		mii_bmcr_data |= BMCR_SPEED_10;
		break;
	default:
		printf("Wrong Media type %d\n", hw->media_type);
        return AT_ERR_PARAM;
		break;
	}

	ret_val = atl1c_write_phy_reg(hw, MII_BMCR, mii_bmcr_data);
	if (ret_val)
		return ret_val;
	hw->phy_configured = true;

	return 0;
}

/*
 * atl1c_irq_disable - Mask off interrupt generation on the NIC
 * @adapter: board private structure
 */
static inline void atl1c_irq_disable(struct atl1c_private *ap)
{
	AT_WRITE_REG(&ap->hw, REG_IMR, 0);
	AT_WRITE_REG(&ap->hw, REG_ISR, ISR_DIS_INT);
	AT_WRITE_FLUSH(&ap->hw);
}

/*
 * Configures PHY autoneg and flow control advertisement settings
 *
 */
static int atl1c_phy_config(struct atl1c_hw *hw)
{
    int err = 0;
	u16 mii_bmcr_data = BMCR_RESET;

	err = atl1c_phy_setup_adv(hw);
	if (err)
		return err;
	mii_bmcr_data |= BMCR_AUTO_NEG_EN | BMCR_RESTART_AUTO_NEG;

	return atl1c_write_phy_reg(hw, MII_BMCR, mii_bmcr_data);
}

/*
 * Set ASPM state.
 * Enable/disable L0s/L1 depend on link state.
 */
static void atl1c_set_aspm(struct atl1c_hw *hw, unsigned int linkup)
{
	u32 pm_ctrl_data;
	u32 link_ctrl_data;
	u32 link_l1_timer = 0xF;

	AT_READ_REG(hw, REG_PM_CTRL, &pm_ctrl_data);
	AT_READ_REG(hw, REG_LINK_CTRL, &link_ctrl_data);
	
	pm_ctrl_data &= ~PM_CTRL_SERDES_PD_EX_L1;
	pm_ctrl_data &=  ~(PM_CTRL_L1_ENTRY_TIMER_MASK <<
			PM_CTRL_L1_ENTRY_TIMER_SHIFT);
	pm_ctrl_data &= ~(PM_CTRL_LCKDET_TIMER_MASK << 
			PM_CTRL_LCKDET_TIMER_SHIFT);
	pm_ctrl_data |= AT_LCKDET_TIMER	<< PM_CTRL_LCKDET_TIMER_SHIFT;

	if (hw->nic_type == athr_l2c_b || hw->nic_type == athr_l1d ||
		hw->nic_type == athr_l2c_b2) {
		link_ctrl_data &= ~LINK_CTRL_EXT_SYNC;
		if (!(hw->ctrl_flags & ATL1C_APS_MODE_ENABLE)) {
			if (hw->nic_type == athr_l2c_b && hw->revision_id == L2CB_V10)
				link_ctrl_data |= LINK_CTRL_EXT_SYNC;
		} 

		AT_WRITE_REG(hw, REG_LINK_CTRL, link_ctrl_data);
		
		pm_ctrl_data |= PM_CTRL_RCVR_WT_TIMER;
		pm_ctrl_data &= ~(PM_CTRL_PM_REQ_TIMER_MASK << 
			PM_CTRL_PM_REQ_TIMER_SHIFT);
		pm_ctrl_data |= AT_ASPM_L1_TIMER <<  
			PM_CTRL_PM_REQ_TIMER_SHIFT;
		pm_ctrl_data &= ~PM_CTRL_SA_DLY_EN;
		pm_ctrl_data &= ~PM_CTRL_HOTRST;
		pm_ctrl_data |= 1 << PM_CTRL_L1_ENTRY_TIMER_SHIFT;
		pm_ctrl_data |= PM_CTRL_SERDES_PD_EX_L1;  
	}
	pm_ctrl_data |= PM_CTRL_MAC_ASPM_CHK;
	if (linkup) {
		pm_ctrl_data &= ~PM_CTRL_ASPM_L1_EN;
		pm_ctrl_data &= ~PM_CTRL_ASPM_L0S_EN;
		if (hw->ctrl_flags & ATL1C_ASPM_L1_SUPPORT)
			pm_ctrl_data |= PM_CTRL_ASPM_L1_EN;
		if (hw->ctrl_flags & ATL1C_ASPM_L0S_SUPPORT)
			pm_ctrl_data |= PM_CTRL_ASPM_L0S_EN;

		if (hw->nic_type == athr_l2c_b || hw->nic_type == athr_l1d ||
			hw->nic_type == athr_l2c_b2) {
			if (hw->nic_type == athr_l2c_b)
				if (!(hw->ctrl_flags & ATL1C_APS_MODE_ENABLE))
                                	pm_ctrl_data &= ~PM_CTRL_ASPM_L0S_EN;
			pm_ctrl_data &= ~PM_CTRL_SERDES_L1_EN;
                        pm_ctrl_data &= ~PM_CTRL_SERDES_PLL_L1_EN;
                        pm_ctrl_data &= ~PM_CTRL_SERDES_BUDS_RX_L1_EN;
                        pm_ctrl_data |= PM_CTRL_CLK_SWH_L1;
                        if (hw->ap->link_speed == SPEED_100 ||
                                hw->ap->link_speed == SPEED_1000) {
                                pm_ctrl_data &=  ~(PM_CTRL_L1_ENTRY_TIMER_MASK <<
                                        PM_CTRL_L1_ENTRY_TIMER_SHIFT);
				if (hw->nic_type == athr_l2c_b)
					link_l1_timer = 7;
				else if (hw->nic_type == athr_l2c_b2)
					link_l1_timer = 4;
                                pm_ctrl_data |= link_l1_timer <<
						PM_CTRL_L1_ENTRY_TIMER_SHIFT;
                        }
		} else {
			pm_ctrl_data |= PM_CTRL_SERDES_L1_EN;
			pm_ctrl_data |= PM_CTRL_SERDES_PLL_L1_EN;
			pm_ctrl_data |= PM_CTRL_SERDES_BUDS_RX_L1_EN;
			pm_ctrl_data &= ~PM_CTRL_CLK_SWH_L1;
			pm_ctrl_data &= ~PM_CTRL_ASPM_L0S_EN;
			pm_ctrl_data &= ~PM_CTRL_ASPM_L1_EN;

		}
	} else {
		pm_ctrl_data &= ~PM_CTRL_SERDES_L1_EN; 
		pm_ctrl_data &= ~PM_CTRL_ASPM_L0S_EN;
		pm_ctrl_data &= ~PM_CTRL_SERDES_PLL_L1_EN;
		pm_ctrl_data |= PM_CTRL_CLK_SWH_L1;
	
		if (hw->ctrl_flags & ATL1C_ASPM_L1_SUPPORT)
			pm_ctrl_data |= PM_CTRL_ASPM_L1_EN;
		else
			pm_ctrl_data &= ~PM_CTRL_ASPM_L1_EN;
	}
	AT_WRITE_REG(hw, REG_PM_CTRL, pm_ctrl_data);

	return;
}

/*
 * Detects the current speed and duplex settings of the hardware.
 *
 * hw - Struct containing variables accessed by shared code
 * speed - Speed of the connection
 * duplex - Duplex setting of the connection
 */
int atl1c_get_speed_and_duplex(struct atl1c_hw *hw, u16 *speed, u16 *duplex)
{
	int err;
	u16 phy_data;

	/* Read   PHY Specific Status Register (17) */
	err = atl1c_read_phy_reg(hw, MII_GIGA_PSSR, &phy_data);
	if (err)
		return err;

	if (!(phy_data & GIGA_PSSR_SPD_DPLX_RESOLVED))
		return AT_ERR_PHY_RES;

	switch (phy_data & GIGA_PSSR_SPEED) {
	case GIGA_PSSR_1000MBS:
		*speed = SPEED_1000;
		break;
	case GIGA_PSSR_100MBS:
		*speed = SPEED_100;
		break;
	case  GIGA_PSSR_10MBS:
		*speed = SPEED_10;
		break;
	default:
		return AT_ERR_PHY_SPEED;
		break;
	}

	if (phy_data & GIGA_PSSR_DPLX)
		*duplex = FULL_DUPLEX;
	else
		*duplex = HALF_DUPLEX;

	return 0;
}

static void atl1c_enable_rx_ctrl(struct atl1c_hw *hw)
{
	u32 data;

	AT_READ_REG(hw, REG_RXQ_CTRL, &data);
	switch (hw->ap->num_rx_queues) {
        case 4:
                data |= (RXQ3_CTRL_EN | RXQ2_CTRL_EN | RXQ1_CTRL_EN);
                break;
        case 3:
                data |= (RXQ2_CTRL_EN | RXQ1_CTRL_EN);
                break;
        case 2:
                data |= RXQ1_CTRL_EN;
                break;
        default:
                break;
        }
	data |= RXQ_CTRL_EN;
	AT_WRITE_REG(hw, REG_RXQ_CTRL, data);
}


static void atl1c_enable_tx_ctrl(struct atl1c_hw *hw)
{
	u32 data;

	AT_READ_REG(hw, REG_TXQ_CTRL, &data);
	data |= TXQ_CTRL_EN;
	AT_WRITE_REG(hw, REG_TXQ_CTRL, data);
}

static void atl1c_setup_mac_ctrl(struct atl1c_private *ap)
{
	struct atl1c_hw *hw = &ap->hw;
	u32 mac_ctrl_data;

	mac_ctrl_data = MAC_CTRL_TX_EN | MAC_CTRL_RX_EN;
	mac_ctrl_data |= (MAC_CTRL_TX_FLOW | MAC_CTRL_RX_FLOW);


	if (ap->link_duplex == FULL_DUPLEX) {
		hw->mac_duplex = true;
		mac_ctrl_data |= MAC_CTRL_DUPLX;
	}

	if (ap->link_speed == SPEED_1000)
		hw->mac_speed = atl1c_mac_speed_1000;
	else
		hw->mac_speed = atl1c_mac_speed_10_100;

	mac_ctrl_data |= (hw->mac_speed & MAC_CTRL_SPEED_MASK) <<
			MAC_CTRL_SPEED_SHIFT;

	mac_ctrl_data |= (MAC_CTRL_ADD_CRC | MAC_CTRL_PAD);
	mac_ctrl_data |= ((hw->preamble_len & MAC_CTRL_PRMLEN_MASK) <<
			MAC_CTRL_PRMLEN_SHIFT);

    

	mac_ctrl_data |= MAC_CTRL_BC_EN;

	if (ap->flags & IFF_PROMISC)
		mac_ctrl_data |= MAC_CTRL_PROMIS_EN;
	if (ap->flags & IFF_ALLMULTI)
		mac_ctrl_data |= MAC_CTRL_MC_ALL_EN;

	mac_ctrl_data |= MAC_CTRL_SINGLE_PAUSE_EN;
	if (hw->nic_type == athr_l1d || hw->nic_type == athr_l2c_b2) {
		mac_ctrl_data |= MAC_CTRL_SPEED_MODE_SW;
		mac_ctrl_data |= MAC_CTRL_HASH_ALG_CRC32;
	}	
	AT_WRITE_REG(hw, REG_MAC_CTRL, mac_ctrl_data);
}


static void atl1c_check_link_status(struct atl1c_private *ap)
{
	struct atl1c_hw *hw = &ap->hw;
	int err = 0;
	u16 speed, duplex, phy_data;

	/* MII_BMSR must read twise */
	atl1c_read_phy_reg(hw, MII_BMSR, &phy_data);
	atl1c_read_phy_reg(hw, MII_BMSR, &phy_data);

	if ((phy_data & BMSR_LSTATUS) == 0) {
		/* link down */
		if (atl1c_stop_mac(hw) != 0)
			printf("stop mac failed\n");
		atl1c_set_aspm(hw, false);
		atl1c_phy_reset(hw);
		atl1c_phy_init(&ap->hw);
	} else {
		/* Link Up */
		err = atl1c_get_speed_and_duplex(hw, &speed, &duplex);
		if (err)
			return;
		/* link result is our setting */
		if (ap->link_speed != speed ||
		    ap->link_duplex != duplex) {
			ap->link_speed  = speed;
			ap->link_duplex = duplex;
			atl1c_set_aspm(hw, true);
			atl1c_enable_tx_ctrl(hw);
			atl1c_enable_rx_ctrl(hw);
			atl1c_setup_mac_ctrl(ap);
			printf("%s:  NIC Link is Up<%d Mbps %s>\n",
				"ATL1C", 
				ap->link_speed,
				ap->link_duplex == FULL_DUPLEX ?
				"Full Duplex" : "Half Duplex");		
        }        
	}
}

static void atl1c_link_chg_event(struct atl1c_private *ap)
{
	u16 phy_data = 0;
	u16 link_up = 0;

	atl1c_read_phy_reg(&ap->hw, MII_BMSR, &phy_data);
	atl1c_read_phy_reg(&ap->hw, MII_BMSR, &phy_data);
	link_up = phy_data & BMSR_LSTATUS;

	/* notify upper layer link down ASAP */
	if (!link_up) {

			printf("%s: NIC Link is Down\n","ATL1C");
            ap->link_speed = SPEED_0;
	}
    atl1c_check_link_status(ap);
}

static unsigned int atl1c_clean_tx_irq(struct atl1c_private *ap,
				enum atl1c_trans_queue type)
{
	struct atl1c_tpd_ring *tpd_ring = (struct atl1c_tpd_ring *)
				&ap->tpd_ring[type];
	struct atl1c_buffer *buffer_info;
	u16 next_to_clean = tpd_ring->next_to_clean;
	u16 hw_next_to_clean;
	u16 shift;
	u32 data;
	struct atl1c_tpd_desc *use_tpd = NULL;

	if (type == atl1c_trans_high)
		shift = MB_HTPD_CONS_IDX_SHIFT;
	else
		shift = MB_NTPD_CONS_IDX_SHIFT;

	AT_READ_REG(&ap->hw, REG_MB_PRIO_CONS_IDX, &data);
	hw_next_to_clean = (data >> shift) & MB_PRIO_PROD_IDX_MASK; 	

	while (next_to_clean != hw_next_to_clean) {
		buffer_info = &tpd_ring->buffer_info[next_to_clean];
        use_tpd = ATL1C_TPD_DESC(tpd_ring, next_to_clean);        
        memset(use_tpd,0,sizeof(struct atl1c_tpd_desc)) ;        
	    buffer_info->skb = NULL;
		if (++next_to_clean == tpd_ring->count)
			next_to_clean = 0;
		tpd_ring->next_to_clean = next_to_clean;
	}
    
	return true;    
}

static int atl1c_interrupt(void *arg)
{
	struct atl1c_private *ap = (struct atl1c_private *)arg;
	struct atl1c_hw *hw = &ap->hw;
	int max_ints = AT_MAX_INT_WORK;
	int handled = -1;

	u32 status;
	u32 reg_data;
	int i;

	do {
        //configed to read to clear
        AT_READ_REG(hw, REG_ISR, &reg_data);
		status = reg_data & hw->intr_mask;
        max_ints--;
		if (status == 0) {
			continue;
		}

		handled = 1;
		/* link event */
		if (status & ISR_GPHY)
			atl1c_clear_phy_int(ap);
		
		/* check if PCIE PHY Link down */
		if (status & ISR_ERROR) {
			printf("atl1c hardware error (status = 0x%x)\n",status & ISR_ERROR);
            /* reset MAC */
                atl1c_down(ap);
                atl1c_up(ap);
			return 1;
		}
		if (status & ISR_RX_PKT) {
		    for (i = 0; i < ap->num_rx_queues; i++)
                	atl1c_clean_rx_irq(ap, i);
		}
		if (status & ISR_TX_PKT){
			atl1c_clean_tx_irq(ap, atl1c_trans_normal);
        }

		if (status & ISR_OVER)
			printf("TX/RX over flow (status = 0x%x)\n",status & ISR_OVER);
		/* link event */
		if (status & (ISR_GPHY | ISR_MANUAL)) {
			atl1c_link_chg_event(ap);
			break;
		}

	} while (max_ints > 0);
	return handled;
}

/*
 * check_eeprom_exist
 * return 1 if eeprom exist
 */
int atl1c_check_eeprom_exist(struct atl1c_hw *hw)
{
	u32 data;
	
	AT_READ_REG(hw, REG_TWSI_DEBUG, &data);
	if (data & TWSI_DEBUG_DEV_EXIST)
		return 1;
	
	AT_READ_REG(hw, REG_MASTER_CTRL, &data);
	if (data & MASTER_CTRL_OTP_SEL)
		return 1;
	return 0;
}

static inline int is_valid_ether_addr( u_int8_t *addr )
{
    const char zaddr[6] = {0,};

    //return !(addr[0]&1) && memcmp( addr, zaddr, 6);
    return !(addr[0]&1) && bcmp( addr, zaddr, 6);
}

//future, this macro should be defined in some .h file
#define swab16(x) \
        ((unsigned short)( \
                (((unsigned short)(x) & (unsigned short)0x00ffU) << 8) | \
                (((unsigned short)(x) & (unsigned short)0xff00U) >> 8) ))

#define swab32(x) \
        ((unsigned int)( \
                (((unsigned int)(x) & (unsigned int)0x000000ffUL) << 24) | \
                (((unsigned int)(x) & (unsigned int)0x0000ff00UL) <<  8) | \
                (((unsigned int)(x) & (unsigned int)0x00ff0000UL) >>  8) | \
                (((unsigned int)(x) & (unsigned int)0xff000000UL) >> 24) ))

/*
 * atl1c_get_permanent_address
 * return 0 if get valid mac address,
 */
static int atl1c_get_permanent_address(struct atl1c_hw *hw)
{
	u32 addr[2];
	u32 i;
	u32 otp_ctrl_data;
	u32 twsi_ctrl_data;
	u32 ltssm_ctrl_data;
	u32 wol_data;
	u8  eth_addr[ETH_ALEN];
	u16 phy_data;
	u32 raise_vol = 0;
    u32 addr32;
    u16 addr16;
	

	/* init */
	addr[0] = addr[1] = 0;
	AT_READ_REG(hw, REG_OTP_CTRL, &otp_ctrl_data);
	if (atl1c_check_eeprom_exist(hw)) {
		/* Enable OTP CLK */
		if (hw->nic_type == athr_l1c || hw->nic_type == athr_l2c_b) {
			if (!(otp_ctrl_data & OTP_CTRL_CLK_EN)) {
				otp_ctrl_data |= OTP_CTRL_CLK_EN;
				AT_WRITE_REG(hw, REG_OTP_CTRL, otp_ctrl_data);
				mdelay(1);
			}
		}
		if (hw->nic_type == athr_l2c_b || hw->nic_type == athr_l2c_b2 ||
			hw->nic_type == athr_l1d) {
			atl1c_write_phy_reg(hw, MII_DBG_ADDR, 0x00);
			if (atl1c_read_phy_reg(hw, MII_DBG_DATA, &phy_data) != 0)
				goto out;
			phy_data &= 0xFF7F;
			atl1c_write_phy_reg(hw, MII_DBG_DATA, phy_data);
			
			atl1c_write_phy_reg(hw, MII_DBG_ADDR, 0x3B);
			if (atl1c_read_phy_reg(hw, MII_DBG_DATA, &phy_data) != 0)
				goto out;
			phy_data |= 0x8;
			atl1c_write_phy_reg(hw, MII_DBG_DATA, phy_data);
			udelay(20);
			raise_vol = 1;	
		}
		/* close open bit of ReadOnly*/
		AT_READ_REG(hw, REG_LTSSM_ID_CTRL, &ltssm_ctrl_data);
		ltssm_ctrl_data &= ~LTSSM_ID_EN_WRO;
		AT_WRITE_REG(hw, REG_LTSSM_ID_CTRL, ltssm_ctrl_data);

		/* clear any WOL settings */
		AT_WRITE_REG(hw, REG_PM_CTRLSTAT, 0);
		AT_WRITE_REG(hw, REG_WOL_CTRL, 0);
		AT_READ_REG(hw, REG_WOL_CTRL, &wol_data);


		AT_READ_REG(hw, REG_TWSI_CTRL, &twsi_ctrl_data);
		twsi_ctrl_data |= TWSI_CTRL_SW_LDSTART;
		AT_WRITE_REG(hw, REG_TWSI_CTRL, twsi_ctrl_data);
		for (i = 0; i < AT_TWSI_EEPROM_TIMEOUT; i++) {
			mdelay(10);
			AT_READ_REG(hw, REG_TWSI_CTRL, &twsi_ctrl_data);
			if ((twsi_ctrl_data & TWSI_CTRL_SW_LDSTART) == 0)
				break;
		}
		if (i >= AT_TWSI_EEPROM_TIMEOUT)
			return AT_ERR_TIMEOUT;
	}
	/* Disable OTP_CLK */	
	if ((hw->nic_type == athr_l1c || hw->nic_type == athr_l2c)) {
		otp_ctrl_data &= ~OTP_CTRL_CLK_EN;
		AT_WRITE_REG(hw, REG_OTP_CTRL, otp_ctrl_data);
		mdelay(1);
	}
	if (raise_vol) {
		if (hw->nic_type == athr_l2c_b || hw->nic_type == athr_l2c_b2 ||
			hw->nic_type == athr_l1d) {
			atl1c_write_phy_reg(hw, MII_DBG_ADDR, 0x00);
			if (atl1c_read_phy_reg(hw, MII_DBG_DATA, &phy_data) != 0)
				goto out;
			phy_data |= 0x80;
			atl1c_write_phy_reg(hw, MII_DBG_DATA, phy_data);
			
			atl1c_write_phy_reg(hw, MII_DBG_ADDR, 0x3B);
			if (atl1c_read_phy_reg(hw, MII_DBG_DATA, &phy_data) != 0)
				goto out;
			phy_data &= 0xFFF7;
			atl1c_write_phy_reg(hw, MII_DBG_DATA, phy_data);
			udelay(20);
		}

	}	
	/* maybe MAC-address is from BIOS */
	AT_READ_REG(hw, REG_MAC_STA_ADDR, &addr[0]);
	AT_READ_REG(hw, REG_MAC_STA_ADDR + 4, &addr[1]);
	//*(u32 *) &eth_addr[2] = swab32(addr[0]);
	addr32 = swab32(addr[0]);
    eth_addr[2] = addr32 & 0xff;
    eth_addr[3] = (addr32>>8) & 0xff;
    eth_addr[4] = (addr32>>16) & 0xff;
    eth_addr[5] = (addr32>>24) & 0xff;
	//*(u16 *) &eth_addr[0] = swab16(*(u16 *)&addr[1]);
	addr16 = swab16(*(u16 *)&addr[1]);
    eth_addr[0] = addr16 & 0xff;
    eth_addr[1] = (addr16>>8) & 0xff;

	if (is_valid_ether_addr(eth_addr)) {
		memcpy(hw->perm_mac_addr, eth_addr, ETH_ALEN);
		return 0;
	}
out:
	return AT_ERR_EEPROM;
}


/*
 * Reads the adapter's MAC address from the EEPROM
 *
 * hw - Struct containing variables accessed by shared code
 */
int atl1c_read_mac_addr(struct atl1c_hw *hw)
{
	int err = 0;

	err = atl1c_get_permanent_address(hw);
	if (err) {
        hw->perm_mac_addr[0] = 0x00;
        hw->perm_mac_addr[1] = 0x13;
        hw->perm_mac_addr[2] = 0x74;
        hw->perm_mac_addr[3] = 0x00;
        hw->perm_mac_addr[4] = 0x5c;
        hw->perm_mac_addr[5] = 0x38;
    }

	memcpy(hw->mac_addr, hw->perm_mac_addr, sizeof(hw->perm_mac_addr));
	return 0;
}

void atl1c_hw_set_mac_addr(struct atl1c_hw *hw)
{
	u32 value;
	/*
	 * 00-0B-6A-F6-00-DC
	 * 0:  6AF600DC 1: 000B
	 * low dword
	 */
	value = (((u32)hw->mac_addr[2]) << 24) |
		(((u32)hw->mac_addr[3]) << 16) |
		(((u32)hw->mac_addr[4]) << 8)  |
		(((u32)hw->mac_addr[5])) ;
    
	AT_WRITE_REG_ARRAY(hw, REG_MAC_STA_ADDR, 0, value);
	/* hight dword */
	value = (((u32)hw->mac_addr[0]) << 8) |
		(((u32)hw->mac_addr[1])) ;
    
	AT_WRITE_REG_ARRAY(hw, REG_MAC_STA_ADDR, 1, value);
}


#if 0
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
#define eeprom_delay()  inl(ee_addr)
#endif

int atl1c_read_eeprom(struct atl1c_private *tp, int RegAddr)
{
    return 0;
}

static void write_eeprom_enable(long ioaddr){
    return;

}

static void write_eeprom_disable(unsigned long ioaddr)
{
    return;
}

int atl1c_write_eeprom(long ioaddr, int location,unsigned short data)
{
	return 0;
}

static int cmd_reprom(int ac, char *av[])
{
    return 0;
}

static int cmd_wrprom(int ac, char *av[])
{
	return 0;
}

static int cmd_setmac(int ac, char *av[])
{
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
			"dump rtl8111dl/8168 eeprom content and mac address", cmd_reprom, 1, 2,0},
	{"writerom", "", NULL,
			"dump rtl8111dl/8168 eeprom content", cmd_wrprom, 1, 3, 0},
	{"setmac", "", NULL,
		    "Set mac address into rtl8111dl/8168 eeprom", cmd_setmac, 1, 5, 0},
	{0, 0}
};


static void init_cmd __P((void)) __attribute__ ((constructor));

static void
init_cmd()
{
	cmdlist_expand(Cmds, 1);
}
