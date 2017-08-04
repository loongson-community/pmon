/** \file
 * This is the network dependent layer to handle network related functionality.
 * This file is tightly coupled to neworking frame work of linux 2.6.xx kernel.
 * The functionality carried out in this file should be treated as an example only
 * if the underlying operating system is not Linux. 
 * 
 * \note Many of the functions other than the device specific functions
 *  changes for operating system other than Linux 2.6.xx
 * \internal 
 *-----------------------------REVISION HISTORY-----------------------------------
 * Synopsys			01/Aug/2007				Created
 */

/*
#include <linux/config.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/pci.h>
#include <linux/init.h>

#include <linux/netdevice.h>
#include <linux/etherdevice.h>


#include <linux/ip.h>
#include <linux/tcp.h>
#include <linux/udp.h>
*/


#include "synopGMAC_Host.h"
#include "synopGMAC_plat.h"
#include "synopGMAC_network_interface.h"
#include "synopGMAC_Dev.h"
#if	(!defined(LOONGSON_2G5536))&&(!defined(LOONGSON_2G1A))&&(!defined(LOONGSON_2F1A))
#include "target/eeprom.h"
#endif

#if defined(LOONGSON_2G1A) || defined(LOONGSON_2F1A)
#include "target/ls1a.h"
#endif

#if defined(LOONGSON_2K)
#include "target/ls2k.h"
#endif

//sw:	ioctl in linux 		to be fixed
#define SIOCDEVPRIVATE	0x89f0
#define IOCTL_READ_REGISTER  SIOCDEVPRIVATE+1
#define IOCTL_WRITE_REGISTER SIOCDEVPRIVATE+2
#define IOCTL_READ_IPSTRUCT  SIOCDEVPRIVATE+3
#define IOCTL_READ_RXDESC    SIOCDEVPRIVATE+4
#define IOCTL_READ_TXDESC    SIOCDEVPRIVATE+5
#define IOCTL_POWER_DOWN     SIOCDEVPRIVATE+6

extern void ls2h_flush_cache2();

//static struct timer_list synopGMAC_cable_unplug_timer;
static u32 GMAC_Power_down; // This global variable is used to indicate the ISR whether the interrupts occured in the process of powering down the mac or not


/*These are the global pointers for their respecive structures*/
/*
extern struct synopGMACNetworkAdapter * synopGMACadapter;
extern synopGMACdevice	          * synopGMACdev;
//extern struct net_dev             * synopGMACnetdev;
//extern struct pci_dev             * synopGMACpcidev;
extern struct Pmon_Inet		  * PInetdev;
*/

//synopGMACdevice	          * synopGMACdev;
//extern struct net_dev             * synopGMACnetdev;
//extern struct pci_dev             * synopGMACpcidev;
//extern struct Pmon_Inet		  * PInetdev;



/*these are the global data for base address and its size*/
//extern u8* synopGMACMappedAddr;
//extern u32 synopGMACMappedAddrSize;
//extern u32 synop_pci_using_dac;

u32 synop_pci_using_dac = 0;
#define MAC_ADDR {0x00, 0x55, 0x7B, 0xB5, 0x7D, 0xF7}	//sw: may be it should be F7 7D B5 7B 55 00

//u32 regbase = 0xbfe10000;	//sw:	for debug only
#if	defined(LOONGSON_2G5536)||defined(LOONGSON_2G1A) || defined(LOONGSON_2F1A)
u64 regbase = 0x90000d0000000000;	// this is of no use in this driver! liyifu on 2010-01-12
#else
u64 regbase = 0xffffffffbbe10000;	// this is of no use in this driver! liyifu on 2010-01-12
#endif
//char mac_addr[6] = {0xF7,0x7D,0xB5,0x7B,0x55,0x00};
char mac_addr[6] = {0x00,0x55,0x7B,0xB5,0x7D,0xF7};

void dumppkghd(struct ether_header *eh,int tp);
unsigned int rx_test_count = 0;
unsigned int tx_normal_test_count = 0;
unsigned int tx_abnormal_test_count = 0;
unsigned int tx_stopped_test_count = 0;





/*Sample Wake-up frame filter configurations*/

u32 synopGMAC_wakeup_filter_config0[] = {
					0x00000000,	// For Filter0 CRC is not computed may be it is 0x0000
					0x00000000,	// For Filter1 CRC is not computed may be it is 0x0000
					0x00000000,	// For Filter2 CRC is not computed may be it is 0x0000
					0x5F5F5F5F,     // For Filter3 CRC is based on 0,1,2,3,4,6,8,9,10,11,12,14,16,17,18,19,20,22,24,25,26,27,28,30 bytes from offset
					0x09000000,     // Filter 0,1,2 are disabled, Filter3 is enabled and filtering applies to only multicast packets
					0x1C000000,     // Filter 0,1,2 (no significance), filter 3 offset is 28 bytes from start of Destination MAC address
					0x00000000,     // No significance of CRC for Filter0 and Filter1
					0xBDCC0000      // No significance of CRC for Filter2, Filter3 CRC is 0xBDCC
					};
u32 synopGMAC_wakeup_filter_config1[] = {
					0x00000000,	// For Filter0 CRC is not computed may be it is 0x0000
					0x00000000,	// For Filter1 CRC is not computed may be it is 0x0000
					0x7A7A7A7A,	// For Filter2 CRC is based on 1,3,4,5,6,9,11,12,13,14,17,19,20,21,25,27,28,29,30 bytes from offset
					0x00000000,     // For Filter3 CRC is not computed may be it is 0x0000
					0x00010000,     // Filter 0,1,3 are disabled, Filter2 is enabled and filtering applies to only unicast packets
					0x00100000,     // Filter 0,1,3 (no significance), filter 2 offset is 16 bytes from start of Destination MAC address
					0x00000000,     // No significance of CRC for Filter0 and Filter1
					0x0000A0FE      // No significance of CRC for Filter3, Filter2 CRC is 0xA0FE
					};
u32 synopGMAC_wakeup_filter_config2[] = {
					0x00000000,	// For Filter0 CRC is not computed may be it is 0x0000
					0x000000FF,	// For Filter1 CRC is computed on 0,1,2,3,4,5,6,7 bytes from offset
					0x00000000,	// For Filter2 CRC is not computed may be it is 0x0000
					0x00000000,     // For Filter3 CRC is not computed may be it is 0x0000
					0x00000100,     // Filter 0,2,3 are disabled, Filter 1 is enabled and filtering applies to only unicast packets
					0x0000DF00,     // Filter 0,2,3 (no significance), filter 1 offset is 223 bytes from start of Destination MAC address
					0xDB9E0000,     // No significance of CRC for Filter0, Filter1 CRC is 0xDB9E
					0x00000000      // No significance of CRC for Filter2 and Filter3
					};

/*
The synopGMAC_wakeup_filter_config3[] is a sample configuration for wake up filter.
Filter1 is used here
Filter1 offset is programmed to 50 (0x32)
Filter1 mask is set to 0x000000FF, indicating First 8 bytes are used by the filter
Filter1 CRC= 0x7EED this is the CRC computed on data 0x55 0x55 0x55 0x55 0x55 0x55 0x55 0x55

Refer accompanied software DWC_gmac_crc_example.c for CRC16 generation and how to use the same.
*/

u32 synopGMAC_wakeup_filter_config3[] = {
					0x00000000,	// For Filter0 CRC is not computed may be it is 0x0000
					0x000000FF,	// For Filter1 CRC is computed on 0,1,2,3,4,5,6,7 bytes from offset
					0x00000000,	// For Filter2 CRC is not computed may be it is 0x0000
					0x00000000,     // For Filter3 CRC is not computed may be it is 0x0000
					0x00000100,     // Filter 0,2,3 are disabled, Filter 1 is enabled and filtering applies to only unicast packets
					0x00003200,     // Filter 0,2,3 (no significance), filter 1 offset is 50 bytes from start of Destination MAC address
					0x7eED0000,     // No significance of CRC for Filter0, Filter1 CRC is 0x7EED,
					0x00000000      // No significance of CRC for Filter2 and Filter3
					};
/**
 * Function used to detect the cable plugging and unplugging.
 * This function gets scheduled once in every second and polls
 * the PHY register for network cable plug/unplug. Once the
 * connection is back the GMAC device is configured as per
 * new Duplex mode and Speed of the connection.
 * @param[in] u32 type but is not used currently.
 * \return returns void.
 * \note This function is tightly coupled with Linux 2.6.xx.
 * \callgraph
 */

/*
 * bcm5461 phy
 */

#define MII_BCM54XX_ECR		0x10	/* BCM54xx extended control register */
#define MII_BCM54XX_ECR_IM	0x1000	/* Interrupt mask */
#define MII_BCM54XX_ECR_IF	0x0800	/* Interrupt force */

#define MII_BCM54XX_ESR		0x11	/* BCM54xx extended status register */
#define MII_BCM54XX_ESR_IS	0x1000	/* Interrupt status */

#define MII_BCM54XX_ISR		0x1a	/* BCM54xx interrupt status register */
#define MII_BCM54XX_IMR		0x1b	/* BCM54xx interrupt mask register */
#define MII_BCM54XX_INT_CRCERr	0x0001	/* CRC error */
#define MII_BCM54XX_INT_LINK	0x0002	/* Link status changed */
#define MII_BCM54XX_INT_SPEED	0x0004	/* Link speed change */
#define MII_BCM54XX_INT_DUPLEX	0x0008	/* Duplex mode changed */
#define MII_BCM54XX_INT_LRS	0x0010	/* Local receiver status changed */
#define MII_BCM54XX_INT_RRS	0x0020	/* Remote receiver status changed */
#define MII_BCM54XX_INT_SSERR	0x0040	/* Scrambler synchronization error */
#define MII_BCM54XX_INT_UHCD	0x0080	/* Unsupported HCD negotiated */
#define MII_BCM54XX_INT_NHCD	0x0100	/* No HCD */
#define MII_BCM54XX_INT_NHCDL	0x0200	/* No HCD link */
#define MII_BCM54XX_INT_ANPR	0x0400	/* Auto-negotiation page received */
#define MII_BCM54XX_INT_LC	0x0800	/* All counters below 128 */
#define MII_BCM54XX_INT_HC	0x1000	/* Counter above 32768 */
#define MII_BCM54XX_INT_MDIX	0x2000	/* MDIX status change */
#define MII_BCM54XX_INT_PSERR	0x4000	/* Pair swap error */


#if UNUSED
static int bcm54xx_config_init(synopGMACdevice *gmacdev)
{
	int retval, err;
	u16 data;

	retval = synopGMAC_read_phy_reg(gmacdev->MacBase,gmacdev->PhyBase,MII_BCM54XX_ECR,&data);
	if (retval < 0)
		return retval;

	/* Mask interrupts globally.  */
	data |= MII_BCM54XX_ECR_IM;
	err = synopGMAC_write_phy_reg(gmacdev->MacBase,gmacdev->PhyBase,MII_BCM54XX_ECR,data);
	if (err < 0)
		return err;

	/* Unmask events we are interested in.  */
	data = ~(MII_BCM54XX_INT_DUPLEX |
		MII_BCM54XX_INT_SPEED |
		MII_BCM54XX_INT_LINK);
	err = synopGMAC_write_phy_reg(gmacdev->MacBase,gmacdev->PhyBase,MII_BCM54XX_IMR,data);
	if (err < 0)
		return err;
	return 0;
}
#endif

void dumpphyregg(synopGMACdevice *gmacdev)
{
	int i;
	u16 data;
	for(i = 0; i <= 31; i++){
		synopGMAC_read_phy_reg(gmacdev->MacBase,gmacdev->PhyBase,i,&data);
		printf("PHY REG 0x%x  value:%x", i, data);
	}
	printf("\n");

}
void dumpdmaregg(synopGMACdevice *gmacdev)
{
	int i;
	u16 data;
	for(i=0;i<=31;i++){
		data=synopGMACReadReg(gmacdev->DmaBase,i<<2);
		printf("DMA REG 0x%x  value:%x",i,data);
	}
	printf("\n");

}
void dumpmacregg(synopGMACdevice *gmacdev)
{
	int i;
	u16 data;
	for(i=0;i<=54;i++){
		data=synopGMACReadReg(gmacdev->MacBase,i<<2);
		printf("MAC REG 0x%x  value:%x",i,data);
	}
	printf("\n");

}

#if	defined(LOONGSON_2G5536)||defined(LOONGSON_2G1A) || defined(LOONGSON_2F1A)
static int rtl8211_config_init(synopGMACdevice *gmacdev)
{
	int retval, err;
	u16 data;

	/* Unmask events we are interested in.  */
	/*
	data = ~(MII_BCM54XX_INT_DUPLEX |
		MII_BCM54XX_INT_SPEED |
		MII_BCM54XX_INT_LINK);
		*/

	synopGMAC_read_phy_reg(gmacdev->MacBase,gmacdev->PhyBase,0x10,&data);
	data = data | 0x10;
	err = synopGMAC_write_phy_reg(gmacdev->MacBase,gmacdev->PhyBase,0x10,data);
	synopGMAC_read_phy_reg(gmacdev->MacBase,gmacdev->PhyBase,0x00,&data);
	data = data | 0x8000;
	err = synopGMAC_write_phy_reg(gmacdev->MacBase,gmacdev->PhyBase,0x00,data);

	synopGMAC_read_phy_reg(gmacdev->MacBase,gmacdev->PhyBase,0x14,&data);
	data = data | 0x82;
	err = synopGMAC_write_phy_reg(gmacdev->MacBase,gmacdev->PhyBase,0x14,data);
	synopGMAC_read_phy_reg(gmacdev->MacBase,gmacdev->PhyBase,0x00,&data);
	data = data | 0x8000;
	err = synopGMAC_write_phy_reg(gmacdev->MacBase,gmacdev->PhyBase,0x00,data);
#if SYNOP_PHY_LOOPBACK
	synopGMAC_read_phy_reg(gmacdev->MacBase,gmacdev->PhyBase,0x14,&data);
	data = data | 0x70;
	data = data & 0xffdf;
	err = synopGMAC_write_phy_reg(gmacdev->MacBase,gmacdev->PhyBase,0x14,data);
	data = 0x8000;
	err = synopGMAC_write_phy_reg(gmacdev->MacBase,gmacdev->PhyBase,0x00,data);
	data = 0x5140;
	err = synopGMAC_write_phy_reg(gmacdev->MacBase,gmacdev->PhyBase,0x00,data);
#endif

	/*	fix up the errors while the cables between the two Gbit network is two-pair(1,2,3,6) not four,
	 *	according to the downshift feature.
	 */
	synopGMAC_read_phy_reg(gmacdev->MacBase,gmacdev->PhyBase,0x14,&data);
	data &= 0xfffff1ff;
	data |= 0x100;
	err = synopGMAC_write_phy_reg(gmacdev->MacBase,gmacdev->PhyBase,0x14,data);
	synopGMAC_read_phy_reg(gmacdev->MacBase,gmacdev->PhyBase,0x00,&data);
	data = data | 0x8000;
	err = synopGMAC_write_phy_reg(gmacdev->MacBase,gmacdev->PhyBase,0x00,data);
	synopGMAC_read_phy_reg(gmacdev->MacBase,gmacdev->PhyBase,0x14,&data);
	printf("phy reg 0x14:0x%x============\n", data);	

	if (err < 0)
		return err;
	return 0;
}
#else
  static int rtl8211_config_init(synopGMACdevice *gmacdev)
  {
          int retval, err;
          u16 data;
          
          data = 0x6400;
          //data = 0x0;

          err = synopGMAC_write_phy_reg((u64 *)gmacdev->MacBase,gmacdev->PhyBase,0x12,data);

  #if SYNOP_PHY_FORCELINK
  //sw: set manual master/slave

          printf("phy force link master\n");
                  
  //sw: set-an slave      
          err = synopGMAC_read_phy_reg((u64 *)gmacdev->MacBase,gmacdev->PhyBase,0x9, &data);
          data = data | 0x1c00;
          err = synopGMAC_write_phy_reg((u64 *)gmacdev->MacBase,gmacdev->PhyBase,0x9,data);

  //sw: set-an 10m
          err = synopGMAC_read_phy_reg((u64 *)gmacdev->MacBase,gmacdev->PhyBase,0x4, &data);
          data = data & 0xfe7f;
          err = synopGMAC_write_phy_reg((u64 *)gmacdev->MacBase,gmacdev->PhyBase,0x4,data);

  //sw: set line-mdi mode 
          err = synopGMAC_read_phy_reg((u64 *)gmacdev->MacBase,gmacdev->PhyBase,0x10, &data);
          data = data & 0xff9f;
          data = data | 0x20;
          err = synopGMAC_write_phy_reg((u64 *)gmacdev->MacBase,gmacdev->PhyBase,0x10,data);

  //sw: restart an
  /*
 5         err = synopGMAC_read_phy_reg(0x5,1,0x0, &data,0);
 166         data = data | 0x0200;
 167         err = synopGMAC_write_phy_reg((u64 *)gmacdev->MacBase,gmacdev->PhyBase,0x0,data, sel);
 168 */
  //sw: reset phy
          err = synopGMAC_read_phy_reg((u64 *)gmacdev->MacBase,gmacdev->PhyBase,0x0, &data);
          data = data | 0x8000;
          err = synopGMAC_write_phy_reg((u64 *)gmacdev->MacBase,gmacdev->PhyBase,0x0,data);

  #endif


  #if SYNOP_PHY_LOOPBACK
          data = 0x5140;
          err = synopGMAC_write_phy_reg((u64 *)gmacdev->MacBase,gmacdev->PhyBase,0x00,data );
  #endif
          if (err < 0)
                  return err;
          return 0;
  }
#endif
#if UNUSED
static int bcm54xx_ack_interrupt(synopGMACdevice *gmacdev)
{
	int reg;
	unsigned short data;
	
	/* Clear pending interrupts.  */
	reg = synopGMAC_read_phy_reg(gmacdev->MacBase,gmacdev->PhyBase,MII_BCM54XX_ISR,&data);
	if (reg < 0)
		return reg;

	printf("===phy intr status: %04x\n",data);
	
	return 0;
}

static int bcm54xx_config_intr(struct synopGMACdevice *gmacdev)
{
	bcm54xx_config_init(gmacdev);
}



static void synopGMAC_linux_cable_unplug_function(struct synopGMACNetworkAdapter *adapter )
{
s32 status;
u16 data;
//struct synopGMACdevice            *gmacdev = adapter->synopGMACdev;
synopGMACdevice *gmacdev = adapter->synopGMACdev;

status = synopGMAC_read_phy_reg(gmacdev->MacBase,gmacdev->PhyBase,PHY_SPECIFIC_STATUS_REG, &data);
//status = synopGMAC_read_phy_reg(gmacdev->MacBase,1,PHY_SPECIFIC_STATUS_REG, &data);


if((data & Mii_phy_status_link_up) == 0){
	TR("No Link: %08x\n",data);
  	gmacdev->LinkState = 0;
	gmacdev->DuplexMode = 0;
	gmacdev->Speed = 0;
	gmacdev->LoopBackMode = 0;

}
else{
	TR("Link UP: %08x\n",data);
	if(!gmacdev->LinkState){
		status = synopGMAC_check_phy_init(gmacdev);
		synopGMAC_mac_init(gmacdev);

	}
}
	
}


static void synopGMAC_linux_powerdown_mac(synopGMACdevice *gmacdev)
{
	TR0("Put the GMAC to power down mode..\n");
	// Disable the Dma engines in tx path
	GMAC_Power_down = 1;	// Let ISR know that Mac is going to be in the power down mode
	synopGMAC_disable_dma_tx(gmacdev);
	plat_delay(10000);		//allow any pending transmission to complete
	// Disable the Mac for both tx and rx
	synopGMAC_tx_disable(gmacdev);
	synopGMAC_rx_disable(gmacdev);
        plat_delay(10000); 		//Allow any pending buffer to be read by host
	//Disable the Dma in rx path
        synopGMAC_disable_dma_rx(gmacdev);

	//enable the power down mode
	//synopGMAC_pmt_unicast_enable(gmacdev);
	
	//prepare the gmac for magic packet reception and wake up frame reception
	synopGMAC_magic_packet_enable(gmacdev);
	synopGMAC_write_wakeup_frame_register(gmacdev, synopGMAC_wakeup_filter_config3);

	synopGMAC_wakeup_frame_enable(gmacdev);

	//gate the application and transmit clock inputs to the code. This is not done in this driver :).

	//enable the Mac for reception
	synopGMAC_rx_enable(gmacdev);

	//Enable the assertion of PMT interrupt
	synopGMAC_pmt_int_enable(gmacdev);
	//enter the power down mode
	synopGMAC_power_down_enable(gmacdev);
	return;
}
#endif

static void synopGMAC_linux_powerup_mac(synopGMACdevice *gmacdev)
{
	GMAC_Power_down = 0;	// Let ISR know that MAC is out of power down now
	if( synopGMAC_is_magic_packet_received(gmacdev))
		TR("GMAC wokeup due to Magic Pkt Received\n");
	if(synopGMAC_is_wakeup_frame_received(gmacdev))
		TR("GMAC wokeup due to Wakeup Frame Received\n");
	//Disable the assertion of PMT interrupt
	synopGMAC_pmt_int_disable(gmacdev);
	//Enable the mac and Dma rx and tx paths
	synopGMAC_rx_enable(gmacdev);
       	synopGMAC_enable_dma_rx(gmacdev);

	synopGMAC_tx_enable(gmacdev);
	synopGMAC_enable_dma_tx(gmacdev);
	return;
}


/**
  * This sets up the transmit Descriptor queue in ring or chain mode.
  * This function is tightly coupled to the platform and operating system
  * Device is interested only after the descriptors are setup. Therefore this function
  * is not included in the device driver API. This function should be treated as an
  * example code to design the descriptor structures for ring mode or chain mode.
  * This function depends on the pcidev structure for allocation consistent dma-able memory in case of linux.
  * This limitation is due to the fact that linux uses pci structure to allocate a dmable memory
  *	- Allocates the memory for the descriptors.
  *	- Initialize the Busy and Next descriptors indices to 0(Indicating first descriptor).
  *	- Initialize the Busy and Next descriptors to first descriptor address.
  * 	- Initialize the last descriptor with the endof ring in case of ring mode.
  *	- Initialize the descriptors in chain mode.
  * @param[in] pointer to synopGMACdevice.
  * @param[in] pointer to pci_device structure.
  * @param[in] number of descriptor expected in tx descriptor queue.
  * @param[in] whether descriptors to be created in RING mode or CHAIN mode.
  * \return 0 upon success. Error code upon failure.
  * \note This function fails if allocation fails for required number of descriptors in Ring mode, but in chain mode
  *  function returns -ESYNOPGMACNOMEM in the process of descriptor chain creation. once returned from this function
  *  user should for gmacdev->TxDescCount to see how many descriptors are there in the chain. Should continue further
  *  only if the number of descriptors in the chain meets the requirements  
  */

s32 synopGMAC_setup_tx_desc_queue(synopGMACdevice * gmacdev,u32 no_of_desc, u32 desc_mode)
{
	s32 i;
	DmaDesc * bf1;

	DmaDesc *first_desc = NULL;
	dma_addr_t dma_addr;
	gmacdev->TxDescCount = 0;

	TR("Total size of memory required for \
			Tx Descriptors in Ring Mode = 0x%08x\n",((sizeof(DmaDesc) * no_of_desc)));

#if defined(LOONGSON_2G1A) || defined(LOONGSON_2F1A)
	if ((gmacdev->MacBase == LS1A_GMAC0_REG_BASE + MACBASE)
		|| (gmacdev->MacBase == LS1A_GMAC1_REG_BASE + MACBASE))
		/* first_desc is uncached addr */
		first_desc = (DmaDesc *)plat_alloc_consistent_dmaable_memory
					(sizeof(DmaDesc) * no_of_desc, &dma_addr);
	else
		/* first_desc is cached addr */
		first_desc = (DmaDesc *)plat_alloc_memory(sizeof(DmaDesc) * no_of_desc+15);	//sw: 128 aligned
#else
	/* first_desc is cached addr */
	first_desc = (DmaDesc *)plat_alloc_memory(sizeof(DmaDesc) * no_of_desc+15);
#endif
	if(first_desc == NULL){
		TR("Error in Tx Descriptors memory allocation\n");
		return -ESYNOPGMACNOMEM;
	}
#if defined(LOONGSON_2G1A) || defined(LOONGSON_2F1A)
	if ((gmacdev->MacBase != LS1A_GMAC0_REG_BASE + MACBASE)
		&& (gmacdev->MacBase != LS1A_GMAC1_REG_BASE + MACBASE))
#endif
		first_desc = ((u32)first_desc) & ~15;


	gmacdev->TxDescCount = no_of_desc;
	gmacdev->TxDesc      = first_desc;

#if defined(LOONGSON_2G1A) || defined(LOONGSON_2F1A)
	if ((gmacdev->MacBase == LS1A_GMAC0_REG_BASE + MACBASE)
		|| (gmacdev->MacBase == LS1A_GMAC1_REG_BASE + MACBASE)) {
		/* 1A dma addr 0x8000 0000 --> pci addr 0x8000 0000 --> DDR addr 0x0000 0000 */
		gmacdev->TxDescDma  = dma_addr;
	} else {
		bf1  = (DmaDesc *)CACHED_TO_UNCACHED((unsigned long)(gmacdev->TxDesc));	
		gmacdev->TxDescDma  = (unsigned long)UNCACHED_TO_PHYS((unsigned long)bf1);
	}
#else
	bf1  = (DmaDesc *)CACHED_TO_UNCACHED((unsigned long)(gmacdev->TxDesc));
	gmacdev->TxDescDma  = (unsigned long)UNCACHED_TO_PHYS((unsigned long)bf1);
#ifndef LOONGSON_2G5536
		gmacdev->TxDescDma &= 0x0fffffff;
#endif
#endif

	TR("\n===Tx first_desc: %x\n",gmacdev->TxDesc);

	for(i =0; i < gmacdev -> TxDescCount; i++){
		synopGMAC_tx_desc_init_ring(gmacdev->TxDesc + i, i == gmacdev->TxDescCount-1);
#if SYNOP_TOP_DEBUG
		printf("\n%02d %08x \n",i,(unsigned int)(gmacdev->TxDesc + i));
		printf("%08x ",(unsigned int)((gmacdev->TxDesc + i))->status);
		printf("%08x ",(unsigned int)((gmacdev->TxDesc + i)->length));
		printf("%08x ",(unsigned int)((gmacdev->TxDesc + i)->buffer1));
		printf("%08x ",(unsigned int)((gmacdev->TxDesc + i)->buffer2));
		printf("%08x ",(unsigned int)((gmacdev->TxDesc + i)->data1));
		printf("%08x ",(unsigned int)((gmacdev->TxDesc + i)->data2));
		printf("%08x ",(unsigned int)((gmacdev->TxDesc + i)->dummy1));
		printf("%08x ",(unsigned int)((gmacdev->TxDesc + i)->dummy2));
#endif
	}

	gmacdev->TxNext = 0;
	gmacdev->TxBusy = 0;
	gmacdev->TxNextDesc = gmacdev->TxDesc;
	gmacdev->TxBusyDesc = gmacdev->TxDesc;
	gmacdev->BusyTxDesc  = 0;

	return -ESYNOPGMACNOERR;
}


/**
  * This sets up the receive Descriptor queue in ring or chain mode.
  * This function is tightly coupled to the platform and operating system
  * Device is interested only after the descriptors are setup. Therefore this function
  * is not included in the device driver API. This function should be treated as an
  * example code to design the descriptor structures in ring mode or chain mode.
  * This function depends on the pcidev structure for allocation of consistent dma-able memory in case of linux.
  * This limitation is due to the fact that linux uses pci structure to allocate a dmable memory
  *	- Allocates the memory for the descriptors.
  *	- Initialize the Busy and Next descriptors indices to 0(Indicating first descriptor).
  *	- Initialize the Busy and Next descriptors to first descriptor address.
  * 	- Initialize the last descriptor with the endof ring in case of ring mode.
  *	- Initialize the descriptors in chain mode.
  * @param[in] pointer to synopGMACdevice.
  * @param[in] pointer to pci_device structure.
  * @param[in] number of descriptor expected in rx descriptor queue.
  * @param[in] whether descriptors to be created in RING mode or CHAIN mode.
  * \return 0 upon success. Error code upon failure.
  * \note This function fails if allocation fails for required number of descriptors in Ring mode, but in chain mode
  *  function returns -ESYNOPGMACNOMEM in the process of descriptor chain creation. once returned from this function
  *  user should for gmacdev->RxDescCount to see how many descriptors are there in the chain. Should continue further
  *  only if the number of descriptors in the chain meets the requirements  
  */
s32 synopGMAC_setup_rx_desc_queue(synopGMACdevice * gmacdev,u32 no_of_desc, u32 desc_mode)
{
	s32 i;
	DmaDesc * bf1;
	DmaDesc *first_desc = NULL;
	dma_addr_t dma_addr;
	gmacdev->RxDescCount = 0;

	TR("total size of memory required for Rx Descriptors in Ring Mode = 0x%08x\n",((sizeof(DmaDesc) * no_of_desc)));
#if defined(LOONGSON_2G1A) || defined(LOONGSON_2F1A)
	if ((gmacdev->MacBase == LS1A_GMAC0_REG_BASE + MACBASE)
		|| (gmacdev->MacBase == LS1A_GMAC1_REG_BASE + MACBASE))
		/* first_desc is uncached addr */
		first_desc = (DmaDesc *)plat_alloc_consistent_dmaable_memory
					(sizeof(DmaDesc) * no_of_desc, &dma_addr);
	else
		/* first_desc is cached addr */
		first_desc = (DmaDesc *)plat_alloc_memory
					(sizeof(DmaDesc) * no_of_desc + 15);//sw: 2word aligned
#else
	/* first_desc is cached addr */
	first_desc = (DmaDesc *)plat_alloc_memory (sizeof(DmaDesc) * no_of_desc + 15);
#endif
	if(first_desc == NULL){
		TR("Error in Rx Descriptor Memory allocation in Ring mode\n");
		return -ESYNOPGMACNOMEM;
	}
#if defined(LOONGSON_2G1A) || defined(LOONGSON_2F1A)
	if ((gmacdev->MacBase != LS1A_GMAC0_REG_BASE + MACBASE)
			&& (gmacdev->MacBase != LS1A_GMAC1_REG_BASE + MACBASE))
#endif
		first_desc = (DmaDesc *)((u32)first_desc & ~15);


	gmacdev->RxDescCount = no_of_desc;
	gmacdev->RxDesc      = first_desc;

#if defined(LOONGSON_2G1A) || defined(LOONGSON_2F1A)
	if ((gmacdev->MacBase == LS1A_GMAC0_REG_BASE + MACBASE)
			|| (gmacdev->MacBase == LS1A_GMAC1_REG_BASE + MACBASE)) {
		/* 1A dma addr 0x8000 0000 --> pci addr 0x8000 0000 --> DDR addr 0x0000 0000 */
		gmacdev->RxDescDma = dma_addr;
	} else {
		bf1  = (DmaDesc *)CACHED_TO_UNCACHED((unsigned long)(gmacdev->RxDesc));
		gmacdev->RxDescDma  = (unsigned long)UNCACHED_TO_PHYS((unsigned long)bf1);
	}
#else
	bf1  = (DmaDesc *)CACHED_TO_UNCACHED((unsigned long)(gmacdev->RxDesc));	
	gmacdev->RxDescDma  = (unsigned long)UNCACHED_TO_PHYS((unsigned long)bf1);
#endif

	for(i = 0; i < gmacdev -> RxDescCount; i++){
		synopGMAC_rx_desc_init_ring(gmacdev->RxDesc + i, i == gmacdev->RxDescCount-1);
#if SYNOP_TOP_DEBUG
		TR("%02d %08x \n",i, (unsigned int)(gmacdev->RxDesc + i));
#endif
	}

	gmacdev->RxNext = 0;
	gmacdev->RxBusy = 0;
	gmacdev->RxNextDesc = gmacdev->RxDesc;
	gmacdev->RxBusyDesc = gmacdev->RxDesc;

	gmacdev->BusyRxDesc   = 0;

	return -ESYNOPGMACNOERR;
}

/**
  * This gives up the receive Descriptor queue in ring or chain mode.
  * This function is tightly coupled to the platform and operating system
  * Once device's Dma is stopped the memory descriptor memory and the buffer memory deallocation,
  * is completely handled by the operating system, this call is kept outside the device driver Api.
  * This function should be treated as an example code to de-allocate the descriptor structures in ring mode or chain mode
  * and network buffer deallocation.
  * This function depends on the pcidev structure for dma-able memory deallocation for both descriptor memory and the
  * network buffer memory under linux.
  * The responsibility of this function is to 
  *     - Free the network buffer memory if any.
  *	- Fee the memory allocated for the descriptors.
  * @param[in] pointer to synopGMACdevice.
  * @param[in] pointer to pci_device structure.
  * @param[in] number of descriptor expected in rx descriptor queue.
  * @param[in] whether descriptors to be created in RING mode or CHAIN mode.
  * \return 0 upon success. Error code upon failure.
  * \note No referece should be made to descriptors once this function is called. This function is invoked when the device is closed.
  */

/*
void synopGMAC_giveup_rx_desc_queue(synopGMACdevice * gmacdev, struct pci_dev *pcidev, u32 desc_mode)
{
s32 i;

DmaDesc *first_desc = NULL;
dma_addr_t first_desc_dma_addr;
u32 status;
dma_addr_t dma_addr1;
dma_addr_t dma_addr2;
u32 length1;
u32 length2;
u32 data1;
u32 data2;

if(desc_mode == RINGMODE){
	for(i =0; i < gmacdev -> RxDescCount; i++){
		synopGMAC_get_desc_data(gmacdev->RxDesc + i, &status, &dma_addr1, &length1, &data1, &dma_addr2, &length2, &data2);
		if((length1 != 0) && (data1 != 0)){
			pci_unmap_single(pcidev,dma_addr1,0,PCI_DMA_FROMDEVICE);
			dev_kfree_skb((struct sk_buff *) data1);	// free buffer1
			TR("(Ring mode) rx buffer1 %08x of size %d from %d rx descriptor is given back\n",data1, length1, i);
		}
		if((length2 != 0) && (data2 != 0)){
			pci_unmap_single(pcidev,dma_addr2,0,PCI_DMA_FROMDEVICE);
			dev_kfree_skb((struct sk_buff *) data2);	//free buffer2
			TR("(Ring mode) rx buffer2 %08x of size %d from %d rx descriptor is given back\n",data2, length2, i);
		}
	}
	plat_free_consistent_dmaable_memory(pcidev,(sizeof(DmaDesc) * gmacdev->RxDescCount),gmacdev->RxDesc,gmacdev->RxDescDma); //free descriptors memory
	TR("Memory allocated %08x  for Rx Desriptors (ring) is given back\n",(u32)gmacdev->RxDesc);
}
else{
	TR("rx-------------------------------------------------------------------rx\n");
	first_desc          = gmacdev->RxDesc;
	first_desc_dma_addr = gmacdev->RxDescDma;
	for(i =0; i < gmacdev -> RxDescCount; i++){
		synopGMAC_get_desc_data(first_desc, &status, &dma_addr1, &length1, &data1, &dma_addr2, &length2, &data2);
		TR("%02d %08x %08x %08x %08x %08x %08x %08x\n",i,(u32)first_desc,first_desc->status,first_desc->length,first_desc->buffer1,first_desc->buffer2,first_desc->data1,first_desc->data2);
		if((length1 != 0) && (data1 != 0)){
			pci_unmap_single(pcidev,dma_addr1,0,PCI_DMA_FROMDEVICE);
			dev_kfree_skb((struct sk_buff *) data1);	// free buffer1
			TR("(Chain mode) rx buffer1 %08x of size %d from %d rx descriptor is given back\n",data1, length1, i);
		}
		plat_free_consistent_dmaable_memory(pcidev,(sizeof(DmaDesc)),first_desc,first_desc_dma_addr); //free descriptors
		TR("Memory allocated %08x for Rx Descriptor (chain) at  %d is given back\n",data2,i);

		first_desc = (DmaDesc *)data2;
		first_desc_dma_addr = dma_addr2;
	}

	TR("rx-------------------------------------------------------------------rx\n");
}
gmacdev->RxDesc    = NULL;
gmacdev->RxDescDma = 0;
return;
}
*/

/**
  * This gives up the transmit Descriptor queue in ring or chain mode.
  * This function is tightly coupled to the platform and operating system
  * Once device's Dma is stopped the memory descriptor memory and the buffer memory deallocation,
  * is completely handled by the operating system, this call is kept outside the device driver Api.
  * This function should be treated as an example code to de-allocate the descriptor structures in ring mode or chain mode
  * and network buffer deallocation.
  * This function depends on the pcidev structure for dma-able memory deallocation for both descriptor memory and the
  * network buffer memory under linux.
  * The responsibility of this function is to 
  *     - Free the network buffer memory if any.
  *	- Fee the memory allocated for the descriptors.
  * @param[in] pointer to synopGMACdevice.
  * @param[in] pointer to pci_device structure.
  * @param[in] number of descriptor expected in tx descriptor queue.
  * @param[in] whether descriptors to be created in RING mode or CHAIN mode.
  * \return 0 upon success. Error code upon failure.
  * \note No reference should be made to descriptors once this function is called. This function is invoked when the device is closed.
  */

/*
void synopGMAC_giveup_tx_desc_queue(synopGMACdevice * gmacdev,struct pci_dev * pcidev, u32 desc_mode)
{
s32 i;

DmaDesc *first_desc = NULL;
dma_addr_t first_desc_dma_addr;
u32 status;
dma_addr_t dma_addr1;
dma_addr_t dma_addr2;
u32 length1;
u32 length2;
u32 data1;
u32 data2;

if(desc_mode == RINGMODE){
	for(i =0; i < gmacdev -> TxDescCount; i++){
		synopGMAC_get_desc_data(gmacdev->TxDesc + i,&status, &dma_addr1, &length1, &data1, &dma_addr2, &length2, &data2);
		if((length1 != 0) && (data1 != 0)){
			pci_unmap_single(pcidev,dma_addr1,0,PCI_DMA_TODEVICE);
			dev_kfree_skb((struct sk_buff *) data1);	// free buffer1
			TR("(Ring mode) tx buffer1 %08x of size %d from %d rx descriptor is given back\n",data1, length1, i);
		}
		if((length2 != 0) && (data2 != 0)){
			pci_unmap_single(pcidev,dma_addr2,0,PCI_DMA_TODEVICE);
			dev_kfree_skb((struct sk_buff *) data2);	//free buffer2
			TR("(Ring mode) tx buffer2 %08x of size %d from %d rx descriptor is given back\n",data2, length2, i);
		}
	}
	plat_free_consistent_dmaable_memory(pcidev,(sizeof(DmaDesc) * gmacdev->TxDescCount),gmacdev->TxDesc,gmacdev->TxDescDma); //free descriptors
	TR("Memory allocated %08x for Tx Desriptors (ring) is given back\n",(u32)gmacdev->TxDesc);
}
else{
	TR("tx-------------------------------------------------------------------tx\n");
	first_desc          = gmacdev->TxDesc;
	first_desc_dma_addr = gmacdev->TxDescDma;
	for(i =0; i < gmacdev -> TxDescCount; i++){
		synopGMAC_get_desc_data(first_desc, &status, &dma_addr1, &length1, &data1, &dma_addr2, &length2, &data2);
		TR("%02d %08x %08x %08x %08x %08x %08x %08x\n",i,(u32)first_desc,first_desc->status,first_desc->length,first_desc->buffer1,first_desc->buffer2,first_desc->data1,first_desc->data2);
		if((length1 != 0) && (data1 != 0)){
			pci_unmap_single(pcidev,dma_addr1,0,PCI_DMA_TODEVICE);
			dev_kfree_skb((struct sk_buff *) data2);	// free buffer1
			TR("(Chain mode) tx buffer1 %08x of size %d from %d rx descriptor is given back\n",data1, length1, i);
		}
		plat_free_consistent_dmaable_memory(pcidev,(sizeof(DmaDesc)),first_desc,first_desc_dma_addr); //free descriptors
		TR("Memory allocated %08x for Tx Descriptor (chain) at  %d is given back\n",data2,i);

		first_desc = (DmaDesc *)data2;
		first_desc_dma_addr = dma_addr2;
	}
	TR("tx-------------------------------------------------------------------tx\n");

}
gmacdev->TxDesc    = NULL;
gmacdev->TxDescDma = 0;
return;
}
*/


/**
 * Function to handle housekeeping after a packet is transmitted over the wire.
 * After the transmission of a packet DMA generates corresponding interrupt
 * (if it is enabled). It takes care of returning the sk_buff to the linux
 * kernel, updating the networking statistics and tracking the descriptors.
 * @param[in] pointer to net_device structure. 
 * \return void.
 * \note This function runs in interrupt context
 */
void synop_handle_transmit_over(struct synopGMACNetworkAdapter * tp)
{
	struct	synopGMACNetworkAdapter *adapter;
	synopGMACdevice * gmacdev;
	s32 desc_index;
	u32 data1, data2;
	u32 status;
	u32 length1, length2;
	u64 dma_addr1, dma_addr2;
#ifdef ENH_DESC_8W
	u32 ext_status;
	u16 time_stamp_higher;
	u32 time_stamp_high;
	u32 time_stamp_low;
#endif
	adapter = tp;
	if(adapter == NULL){
#if SYNOP_TX_DEBUG
		TR("Unknown Device\n");
#endif
		return;
	}
	
	gmacdev = adapter->synopGMACdev;
	if(gmacdev == NULL){
#if SYNOP_TX_DEBUG
		TR("GMAC device structure is missing\n");
#endif
		return;
	}

	/*Handle the transmit Descriptors*/
	do {
#ifdef ENH_DESC_8W
	desc_index = synopGMAC_get_tx_qptr(gmacdev, &status, &dma_addr1, &length1, &data1, &dma_addr2, &length2, &data2,&ext_status,&time_stamp_high,&time_stamp_low);
        synopGMAC_TS_read_timestamp_higher_val(gmacdev, &time_stamp_higher);
#else
	desc_index = synopGMAC_get_tx_qptr(gmacdev, &status, &dma_addr1, &length1, &data1, &dma_addr2, &length2, &data2);
#if SYNOP_TX_DEBUG

	printf("===handle transmit_over: %d\n",desc_index);
#endif
#endif
	//desc_index = synopGMAC_get_tx_qptr(gmacdev, &status, &dma_addr, &length, &data1);
		if(desc_index >= 0 && data1 != 0){
#if SYNOP_TX_DEBUG
			printf("Finished Transmit at Tx Descriptor %d for skb 0x%08x and buffer = %08x whose status is %08x \n", desc_index,data1,dma_addr1,status);
#endif
			#ifdef	IPC_OFFLOAD
			if(synopGMAC_is_tx_ipv4header_checksum_error(gmacdev, status)){
#if SYNOP_TX_DEBUG
			TR("Harware Failed to Insert IPV4 Header Checksum\n");
#else
			;
#endif
			}
			if(synopGMAC_is_tx_payload_checksum_error(gmacdev, status)){
#if SYNOP_TX_DEBUG
			TR("Harware Failed to Insert Payload Checksum\n");
#else
			;
#endif
			}
			#endif
		
			plat_free_memory((void *)(data1));	//sw:	data1 = buffer1
			
			if(synopGMAC_is_desc_valid(status)){
				adapter->synopGMACNetStats.tx_bytes += length1;
				adapter->synopGMACNetStats.tx_packets++;
			}
			else {	
#if SYNOP_TX_DEBUG
				TR("Error in Status %08x\n",status);
#endif
				adapter->synopGMACNetStats.tx_errors++;
				adapter->synopGMACNetStats.tx_aborted_errors += synopGMAC_is_tx_aborted(status);
				adapter->synopGMACNetStats.tx_carrier_errors += synopGMAC_is_tx_carrier_error(status);
			}
		}	adapter->synopGMACNetStats.collisions += synopGMAC_get_tx_collision_count(status);
	} while(desc_index >= 0);
//	netif_wake_queue(netdev);
}




/**
 * Function to Receive a packet from the interface.
 * After Receiving a packet, DMA transfers the received packet to the system memory
 * and generates corresponding interrupt (if it is enabled). This function prepares
 * the sk_buff for received packet after removing the ethernet CRC, and hands it over
 * to linux networking stack.
 * 	- Updataes the networking interface statistics
 *	- Keeps track of the rx descriptors
 * @param[in] pointer to net_device structure. 
 * \return void.
 * \note This function runs in interrupt context.
 */

void synop_handle_received_data(struct synopGMACNetworkAdapter* tp)
{
	struct synopGMACNetworkAdapter *adapter;
	synopGMACdevice * gmacdev;
	struct PmonInet * pinetdev;
	s32 desc_index;
	struct ifnet* ifp;
	struct ether_header * eh;
	int i;
	char * ptr;
	u32 bf1;
	u32 data1;
	u32 data2;
	u32 len;
	u32 status;
	u64 dma_addr1;
	u64 dma_addr2;
	struct mbuf *skb; //This is the pointer to hold the received data

#if SYNOP_RX_DEBUG
	TR("%s\n",__FUNCTION__);	
#endif

	adapter = tp;
	if(adapter == NULL){
#if SYNOP_RX_DEBUG
		TR("Unknown Device\n");
#endif
		return;
	}

	gmacdev = adapter->synopGMACdev;
	if(gmacdev == NULL){
#if SYNOP_RX_DEBUG
		TR("GMAC device structure is missing\n");
#endif
		return;
	}	

	pinetdev = adapter->PInetdev;
	if(pinetdev == NULL){
#if SYNOP_RX_DEBUG
		TR("GMAC device structure is missing\n");
#endif
		return;
	}
	ifp = &(pinetdev->arpcom.ac_if);

	//dumpdesc(gmacdev);

	/*Handle the Receive Descriptors*/
	do{
		desc_index = synopGMAC_get_rx_qptr(gmacdev, &status,&dma_addr1,NULL, &data1,&dma_addr2,NULL,&data2);

		if(desc_index >= 0 && data1 != 0){
#if SYNOP_RX_DEBUG
			printf("Received Data at Rx Descriptor %d for skb 0x%08x whose status is %08x\n",desc_index,dma_addr1,status);
#endif

			if(synopGMAC_is_rx_desc_valid(status)||SYNOP_PHY_LOOPBACK){
				skb = getmbuf(adapter);

				if(skb == 0)
#if SYNOP_RX_DEBUG
					printf("===error in getmbuf\n");
#else						
				;
#endif

#if defined(LOONGSON_2G1A) || defined(LOONGSON_2F1A)
				if ((gmacdev->MacBase == LS1A_GMAC0_REG_BASE + MACBASE)
						|| (gmacdev->MacBase == LS1A_GMAC1_REG_BASE + MACBASE)) {
					dma_addr1 =  plat_dma_map_single(gmacdev, data1, RX_BUF_SIZE, SYNC_R);
				}
#endif
				/* Not interested in Ethernet CRC bytes */
				len =  synopGMAC_get_rx_desc_frame_length(status) - 4;
				bcopy((char *)data1, mtod(skb, char *), len); 

#if SYNOP_RX_DEBUG
				printf("==get pkg len: %d",len);
#endif

				skb->m_pkthdr.rcvif = ifp;
				skb->m_pkthdr.len = skb->m_len = len - sizeof(struct ether_header);

				eh = mtod(skb, struct ether_header *);
#if SYNOP_RX_DEBUG
				dumppkghd(eh,1);
				{
					int k;
					char temp;
					for (k=0;k<len;k++)
					{
						temp = (char)(*(char *)(data1 + k));
						printf("%02x  ",temp);
					}
					printf("\n");
				}

				if(eh->ether_shost[1] == 0x55)
				{
					printf("\n\n=!!! notice !!!===\n");
					printf("==!!! notice !!!===\n");
					for(i = 0;i < len;i++)
					{
						ptr = (char *)eh;
						printf(" %02x",*(ptr+i));
					}
					printf("\n\n\n\n\n\n\n\n\n");
				}
#endif

				skb->m_data += sizeof(struct ether_header);

				ether_input(ifp, eh, skb);
				memset((u32)data1,0,RX_BUF_SIZE);
#if defined(LOONGSON_2G1A) || defined(LOONGSON_2F1A)
				if ((gmacdev->MacBase == LS1A_GMAC0_REG_BASE + MACBASE)
					|| (gmacdev->MacBase == LS1A_GMAC1_REG_BASE + MACBASE))
					pci_sync_cache(NULL, (vm_offset_t)data1, RX_BUF_SIZE, SYNC_W);
#endif
				adapter->synopGMACNetStats.rx_packets++;
				adapter->synopGMACNetStats.rx_bytes += len;
			}
			else{
				adapter->synopGMACNetStats.rx_errors++;
				adapter->synopGMACNetStats.collisions       += synopGMAC_is_rx_frame_collision(status);
				adapter->synopGMACNetStats.rx_crc_errors    += synopGMAC_is_rx_crc(status);
				adapter->synopGMACNetStats.rx_frame_errors  += synopGMAC_is_frame_dribbling_errors(status);
				adapter->synopGMACNetStats.rx_length_errors += synopGMAC_is_rx_frame_length_errors(status);
			}

#if defined(LOONGSON_2G1A) || defined(LOONGSON_2F1A)
			if ((gmacdev->MacBase != LS1A_GMAC0_REG_BASE + MACBASE)
					&& (gmacdev->MacBase != LS1A_GMAC1_REG_BASE + MACBASE)) {
				dma_addr1  = (unsigned long)CACHED_TO_PHYS((unsigned long)(data1));
			}
#else
			dma_addr1  = (unsigned long)CACHED_TO_PHYS((unsigned long)(data1));
#endif
			desc_index = synopGMAC_set_rx_qptr(gmacdev, dma_addr1, RX_BUF_SIZE, (u32)data1,0,0,0);

			if(desc_index < 0){
#if SYNOP_RX_DEBUG
				TR("Cannot set Rx Descriptor for data1 %08x\n",(u32)data1);
#endif
				plat_free_memory((void *)data1);
			}

		}

	}while(desc_index >= 0);
}


/**
 * Interrupt service routing.
 * This is the function registered as ISR for device interrupts.
 * @param[in] interrupt number. 
 * @param[in] void pointer to device unique structure (Required for shared interrupts in Linux).
 * @param[in] pointer to pt_regs (not used).
 * \return Returns IRQ_NONE if not device interrupts IRQ_HANDLED for device interrupts.
 * \note This function runs in interrupt context
 *
 */

//irqreturn_t synopGMAC_intr_handler(s32 intr_num, void * dev_id, struct pt_regs *regs)
int synopGMAC_intr_handler(struct synopGMACNetworkAdapter * tp)
{
	/*Kernels passes the netdev structure in the dev_id. So grab it*/
        struct synopGMACNetworkAdapter *adapter;
        synopGMACdevice * gmacdev;
        u32 interrupt;
	u32 dma_status_reg;
	s32 status;
	u64 dma_addr;
	struct ifnet * ifp;

        adapter  = tp;
        if(adapter == NULL){
                TR("Adapter Structure Missing\n");
                return -1;
        }

        gmacdev = adapter->synopGMACdev;
        if(gmacdev == NULL){
                TR("GMAC device structure Missing\n");
                return -1;
        }

	ifp = &(adapter->PInetdev->arpcom.ac_if);

	if(gmacdev->LinkState == LINKUP)
	ifp->if_flags = ifp->if_flags | IFF_RUNNING;


	/*Read the Dma interrupt status to know whether the interrupt got generated by our device or not*/
	dma_status_reg = synopGMACReadReg(gmacdev->DmaBase, DmaStatus);

	
	if(dma_status_reg == 0)
		return 0;

#if defined(LOONGSON_2G1A) || defined(LOONGSON_2F1A)
	if ((gmacdev->MacBase != LS1A_GMAC0_REG_BASE + MACBASE)
			&& (gmacdev->MacBase != LS1A_GMAC1_REG_BASE + MACBASE))
#endif	
		if(dma_status_reg == 0x660004)	//sw: dbg
			return 0;
	
//sw: check phy status	
//	synopGMAC_linux_cable_unplug_function(tp);
	
        synopGMAC_disable_interrupt_all(gmacdev);

//	dumpreg(regbase);
	
	
	if(dma_status_reg & GmacPmtIntr){
		TR("%s:: Interrupt due to PMT module\n",__FUNCTION__);
		synopGMAC_linux_powerup_mac(gmacdev);
	}
	
	if(dma_status_reg & GmacMmcIntr){
		TR("%s:: Interrupt due to MMC module\n",__FUNCTION__);
		TR("%s:: synopGMAC_rx_int_status = %08x\n",__FUNCTION__,synopGMAC_read_mmc_rx_int_status(gmacdev));
		TR("%s:: synopGMAC_tx_int_status = %08x\n",__FUNCTION__,synopGMAC_read_mmc_tx_int_status(gmacdev));
	}

	if(dma_status_reg & GmacLineIntfIntr){
		u16 data;
		status = synopGMAC_read_phy_reg(gmacdev->MacBase,gmacdev->PhyBase,PHY_SPECIFIC_STATUS_REG, &data);
		//status = synopGMAC_read_phy_reg(gmacdev->MacBase,1,PHY_SPECIFIC_STATUS_REG, &data);


		if((data & Mii_phy_status_link_up) == 0){
			//TR("No Link: %08x\n",data);
			gmacdev->LinkState = 0;
			gmacdev->DuplexMode = 0;
			gmacdev->Speed = 0;
			gmacdev->LoopBackMode = 0; 

		}
		else{
			//TR("Link UP: %08x\n",data);
			if(gmacdev->LinkState!=data)
			{
				status = synopGMAC_check_phy_init(gmacdev);
				synopGMAC_mac_init(gmacdev);

			}
		}
	}
	/*Now lets handle the DMA interrupts*/
        interrupt = synopGMAC_get_interrupt_type(gmacdev);
//sw
	if(interrupt == 0)	
		return 0;
	
//	TR("%s:Interrupts to be handled: 0x%08x\n",__FUNCTION__,interrupt);

        if(interrupt & synopGMACDmaError){
#if	defined(LOONGSON_2G5536)||defined(LOONGSON_2G1A) || defined(LOONGSON_2F1A)
		u8 mac_addr[6] = DEFAULT_MAC_ADDRESS;//after soft reset, configure the MAC address to default value
		mac_addr[5] += atoi(getenv("synmac"));
#endif
		TR("%s::Fatal Bus Error Inetrrupt Seen\n",__FUNCTION__);
		printf("====DMA error!!!\n");
		
		synopGMAC_disable_dma_tx(gmacdev);
                synopGMAC_disable_dma_rx(gmacdev);
                
		synopGMAC_take_desc_ownership_tx(gmacdev);
		synopGMAC_take_desc_ownership_rx(gmacdev);
		
		synopGMAC_init_tx_rx_desc_queue(gmacdev);
		/* reset the DMA engine and the GMAC ip */
		synopGMAC_reset(gmacdev);

		/* after soft reset, configure the MAC address to default value */
#if	defined(LOONGSON_2G5536)||defined(LOONGSON_2G1A) || defined(LOONGSON_2F1A)
		synopGMAC_set_mac_addr(gmacdev,GmacAddr0High,GmacAddr0Low, mac_addr);
#else
		synopGMAC_set_mac_addr(gmacdev,GmacAddr0High,GmacAddr0Low, tp->PInetdev->dev_addr);
#endif
		synopGMAC_dma_bus_mode_init(gmacdev,DmaFixedBurstEnable| DmaBurstLength8 | DmaDescriptorSkip2 );
	 	synopGMAC_dma_control_init(gmacdev,DmaStoreAndForward);	
		synopGMAC_init_rx_desc_base(gmacdev);
		synopGMAC_init_tx_desc_base(gmacdev);
		synopGMAC_mac_init(gmacdev);
		synopGMAC_enable_dma_rx(gmacdev);
		synopGMAC_enable_dma_tx(gmacdev);

        }


	if(interrupt & synopGMACDmaRxNormal)
	{
#if SYNOP_RX_TEST
		rx_test_count += 1;
		printf("Rx: %d packets!\n",rx_test_count);
#endif
//		dumpreg(regbase);
		synop_handle_received_data(adapter);
	}

        if(interrupt & synopGMACDmaRxAbnormal){
	        TR("%s::Abnormal Rx Interrupt Seen\n",__FUNCTION__);
		#if 1
	
	       if(GMAC_Power_down == 0){	// If Mac is not in powerdown
			adapter->synopGMACNetStats.rx_over_errors++;
			/* Now Descriptors have been created in synop_handle_received_data().
			 * Just issue a poll demand to resume DMA operation
			 * */
			synopGMACWriteReg(gmacdev->DmaBase, DmaStatus ,0x80); 	//sw: clear the rxb ua bit
			synopGMAC_resume_dma_rx(gmacdev);//To handle GBPS with 12 descriptors
		}
		#endif
	}



        if(interrupt & synopGMACDmaRxStopped){
        	TR("%s::Receiver stopped seeing Rx interrupts\n",__FUNCTION__); //Receiver gone in to stopped state
	        if(GMAC_Power_down == 0){	// If Mac is not in powerdown
			adapter->synopGMACNetStats.rx_over_errors++;
			do{
				u32 skb = (u32)plat_alloc_memory(RX_BUF_SIZE);		//should skb aligned here?
				if(skb == NULL){
					TR0("ERROR in skb buffer allocation\n");
					break;
//					return -ESYNOPGMACNOMEM;
				}

#if defined(LOONGSON_2G1A) || defined(LOONGSON_2F1A)
				if ((gmacdev->MacBase == LS1A_GMAC0_REG_BASE + MACBASE)
					|| (gmacdev->MacBase == LS1A_GMAC1_REG_BASE + MACBASE))
					dma_addr = plat_dma_map_single(gmacdev, skb, RX_BUF_SIZE, SYNC_R);
				else
					dma_addr  = (unsigned long)UNCACHED_TO_PHYS((unsigned long)(skb));
#else
				dma_addr  = (unsigned long)UNCACHED_TO_PHYS((unsigned long)(skb));
#endif
				status = synopGMAC_set_rx_qptr(gmacdev,dma_addr,RX_BUF_SIZE, (u32)skb,0,0,0);
				//pci_sync_cache(0, (vm_offset_t)skb, len, SYNC_W);
				//printf("==rx sync cache\n");
				TR("%s::Set Rx Descriptor no %08x for skb %08x \n",__FUNCTION__,status,(u32)skb);
				if(status < 0)
				{	
					printf("==%s:no free\n",__FUNCTION__);
					plat_free_memory((void *)skb);
				}
			}while(status >= 0);

			synopGMAC_enable_dma_rx(gmacdev);
		}
	}

	if(interrupt & synopGMACDmaTxNormal)
	{
		//xmit function has done its job

#if SYNOP_TX_TEST
		tx_normal_test_count += 1;
		printf("Tx: %d normal packets!\n",tx_normal_test_count);
#endif
#if (SYNOP_TX_DEBUG||SYNOP_LOOPBACK_DEBUG)

		TR("%s::Finished Normal Transmission \n",__FUNCTION__);
//		dumpreg(regbase);
#endif
#if SYNOP_TX_DEBUG

		printf("Gmac_intr: dma_status = 0x%08x\n",dma_status_reg);
#endif
		synop_handle_transmit_over(adapter);	//Do whatever you want after the transmission is over
	}

        if(interrupt & synopGMACDmaTxAbnormal){
#if SYNOP_TX_TEST
		tx_abnormal_test_count += 1;
		printf("Tx: %d abnormal packets!\n",tx_abnormal_test_count);
#endif
#if	(!defined(LOONGSON_2G5536))&&(!defined(LOONGSON_2G1A)) && (!defined(LOONGSON_2F1A))
		printf("%s::Abnormal Tx Interrupt Seen\n",__FUNCTION__);
#endif
//		dumpreg(regbase);
#if 1
		if(GMAC_Power_down == 0){	// If Mac is not in powerdown
			synop_handle_transmit_over(adapter);
		}
#endif
	}



	if(interrupt & synopGMACDmaTxStopped){
#if SYNOP_TX_TEST
		tx_stopped_test_count += 1;
		printf("Tx: %d stopped packets!\n",tx_stopped_test_count);
#endif
		TR("%s::Transmitter stopped sending the packets\n",__FUNCTION__);
		printf("%s::Transmitter stopped sending the packets\n",__FUNCTION__);
		#if 1
		if(GMAC_Power_down == 0){	// If Mac is not in powerdown
			synopGMAC_disable_dma_tx(gmacdev);
			synopGMAC_take_desc_ownership_tx(gmacdev);

			synopGMAC_enable_dma_tx(gmacdev);
//			netif_wake_queue(netdev);
			TR("%s::Transmission Resumed\n",__FUNCTION__);
		}
		#endif
	}
//	synopGMAC_clear_interrupt(gmacdev);

        /* Enable the interrrupt before returning from ISR*/
//        synopGMAC_enable_interrupt(gmacdev,DmaIntEnable);
        return 0;
}



/**
 * Function used when the interface is opened for use.
 * We register synopGMAC_linux_open function to linux open(). Basically this 
 * function prepares the the device for operation . This function is called whenever ifconfig (in Linux)
 * activates the device (for example "ifconfig eth0 up"). This function registers
 * system resources needed 
 * 	- Attaches device to device specific structure
 * 	- Programs the MDC clock for PHY configuration
 * 	- Check and initialize the PHY interface 
 *	- ISR registration
 * 	- Setup and initialize Tx and Rx descriptors
 *	- Initialize MAC and DMA
 *	- Allocate Memory for RX descriptors (The should be DMAable)
 * 	- Initialize one second timer to detect cable plug/unplug
 *	- Configure and Enable Interrupts
 *	- Enable Tx and Rx
 *	- start the Linux network queue interface
 * @param[in] pointer to net_device structure. 
 * \return Returns 0 on success and error status upon failure.
 * \callgraph
 */

unsigned long synopGMAC_linux_open(struct synopGMACNetworkAdapter *tp)
{
	s32 status = 0;
	s32 retval = 0;
	int delay = 100;
//	DmaDesc * dbgdesc;	//sw: dbg
	
	//s32 reserve_len=2;
	u64 dma_addr;
	u32 skb;	//sw	we just use the name skb in pomn
	u32 skb1;
	struct synopGMACNetworkAdapter *adapter = tp;
        synopGMACdevice * gmacdev;
	struct PmonInet * PInetdev;
	TR0("%s called \n",__FUNCTION__);
	adapter = tp;
	gmacdev = (synopGMACdevice *)adapter->synopGMACdev;
	PInetdev = (struct PmonInet *)adapter->PInetdev;
	
	/*Now platform dependent initialization.*/

	/*Lets reset the IP*/
#if	defined(LOONGSON_2G5536)||defined(LOONGSON_2G1A) || defined(LOONGSON_2F1A)
	synopGMAC_reset(gmacdev);
#else
	TR("0xbfe10040:%x\n",synopGMACReadReg(0xffffffffbfe10040,0));
	TR("0xbfe18040:%x\n",synopGMACReadReg(0xffffffffbfe18040,0));
//	synopGMAC_reset(gmacdev);
	TR("0xbfe10040:%x\n",synopGMACReadReg(0xffffffffbfe10040,0));
	TR("0xbfe18040:%x\n",synopGMACReadReg(0xffffffffbfe18040,0));
#endif	
	/*Attach the device to MAC struct This will configure all the required base addresses
	  such as Mac base, configuration base, phy base address(out of 32 possible phys )*/
#if defined(LOONGSON_2G1A) || defined(LOONGSON_2F1A)
	if ((gmacdev->MacBase == LS1A_GMAC0_REG_BASE + MACBASE)
			|| (gmacdev->MacBase == LS1A_GMAC1_REG_BASE + MACBASE))
		synopGMAC_set_mac_addr(gmacdev,GmacAddr0High,GmacAddr0Low, PInetdev->dev_addr);
#elif defined(LOONGSON_2G5536)
#else
	synopGMAC_set_mac_addr(adapter->synopGMACdev, GmacAddr0High, GmacAddr0Low, PInetdev->dev_addr);
#endif
	/*Lets read the version of ip in to device structure*/	
	synopGMAC_read_version(gmacdev);
	
	synopGMAC_get_mac_addr(adapter->synopGMACdev,GmacAddr0High,GmacAddr0Low, PInetdev->dev_addr);

	/*Now set the broadcast address*/	

	/*Check for Phy initialization*/
	synopGMAC_set_mdc_clk_div(gmacdev,GmiiCsrClk3);
	gmacdev->ClockDivMdc = synopGMAC_get_mdc_clk_div(gmacdev);

//	dumpphyreg(synopGMACadapter->synopGMACdev);

#if SYNOP_TOP_DEBUG
	printf("check phy init status = 0x%x\n",status);
#endif



#if	(!defined(LOONGSON_2G5536))&&(!defined(LOONGSON_2G1A))
	TR("linux_open------------------------dbg 0\n");
#endif
	/*Set up the tx and rx descriptor queue/ring*/
//sw
	synopGMAC_setup_tx_desc_queue(gmacdev,TRANSMIT_DESC_SIZE, RINGMODE);
	synopGMAC_init_tx_desc_base(gmacdev);	//Program the transmit descriptor base address in to DmaTxBase addr

#if SYNOP_TOP_DEBUG
	//dumpreg(regbase);
#endif
	
	synopGMAC_setup_rx_desc_queue(gmacdev,RECEIVE_DESC_SIZE, RINGMODE);
	synopGMAC_init_rx_desc_base(gmacdev);	//Program the transmit descriptor base address in to DmaTxBase addr



//sw: debug
/*
	setup_tx_desc(gmacdev);	//sw: debug
	synopGMAC_init_tx_desc_base(gmacdev);	//Program the transmit descriptor base address in to DmaTxBase addr
	
	reg_init(gmacdev);
*/
#if SYNOP_TOP_DEBUG
	//dumpphyreg(regbase);
#endif




#ifdef ENH_DESC_8W
	/* pbl32 incr with rxthreshold 128 and Desc is 8 Words */
	synopGMAC_dma_bus_mode_init(gmacdev, DmaBurstLength32 | DmaDescriptorSkip2 | DmaDescriptor8Words );
#else
	/* pbl4 incr with rxthreshold 128 */
	synopGMAC_dma_bus_mode_init(gmacdev, DmaBurstLength4 | DmaDescriptorSkip1 );
#endif
	
	synopGMAC_dma_control_init(gmacdev,DmaStoreAndForward |DmaTxSecondFrame|DmaRxThreshCtrl128 );	


//sw: dbg	
/*
	gmacdev->DuplexMode = FULLDUPLEX ;
	gmacdev->Speed      =   SPEED1000;
*/

	
	/*Initialize the mac interface*/
	synopGMAC_check_phy_init(gmacdev);
	synopGMAC_mac_init(gmacdev);
//	dumpreg(regbase);
	
	TR("linux_open------------------------dbg1\n");
	
	synopGMAC_pause_control(gmacdev); // This enables the pause control in Full duplex mode of operation

	do{
		skb = (u32)plat_alloc_memory(RX_BUF_SIZE);		//should skb aligned here?
		if(skb == NULL){
			TR0("ERROR in skb buffer allocation\n");
			break;
//			return -ESYNOPGMACNOMEM;
		}
#if defined(LOONGSON_2G1A) || defined(LOONGSON_2F1A)
		if ((gmacdev->MacBase == LS1A_GMAC0_REG_BASE + MACBASE)
				|| (gmacdev->MacBase == LS1A_GMAC1_REG_BASE + MACBASE)) {
			dma_addr = plat_dma_map_single(gmacdev, skb, RX_BUF_SIZE, SYNC_R);
		} else {
			skb1 = (u32)CACHED_TO_UNCACHED((unsigned long)(skb));
			dma_addr  = (unsigned long)UNCACHED_TO_PHYS((unsigned long)(skb1));
		}
#else		
		skb1 = (u32)CACHED_TO_UNCACHED((unsigned long)(skb));	
		dma_addr  = (unsigned long)UNCACHED_TO_PHYS((unsigned long)(skb1));
#ifndef LOONGSON_2G5536
		dma_addr &= 0x0fffffff;
		dma_addr |= 0x00000000;
#endif
#endif
//		TR("rx_buf_PTR:%x, skb1:%x, dma_addr%x\n",skb, skb1, dma_addr);


		status = synopGMAC_set_rx_qptr(gmacdev,dma_addr,RX_BUF_SIZE, (u32)skb,0,0,0);
		//pci_sync_cache(0, (vm_offset_t)skb, len, SYNC_W);
		//printf("==rx sync cache\n");
		//status = 0;
		
		if(status < 0)
		{
//sw: something wrong with free ,just let it go now
			TR("synopGMAC_set_rx_qptr,status<0, error!!!!!!!!!!!!!\n");
			plat_free_memory((void *)skb);
		}	
	}while(status >= 0 && status < RECEIVE_DESC_SIZE-1);

//	ls2h_flush_cache2();


#if SYNOP_TOP_DEBUG
	dumpdesc(gmacdev);
#endif

	TR("linux_open------------------------dbg 2\n");
	synopGMAC_clear_interrupt(gmacdev);
	/*
	 * Disable the interrupts generated by MMC and IPC counters.
	 * If these are not disabled ISR 
	 * should be modified accordingly to handle these interrupts.
	 */	
	synopGMAC_disable_mmc_tx_interrupt(gmacdev, 0xFFFFFFFF);
	synopGMAC_disable_mmc_rx_interrupt(gmacdev, 0xFFFFFFFF);
	synopGMAC_disable_mmc_ipc_rx_interrupt(gmacdev, 0xFFFFFFFF);

	/* sw	no interrupts in pmon */
	synopGMAC_disable_interrupt_all(gmacdev);
	
	
	TR("linux_open------------------------dbg 3\n");
//	dumpreg(regbase);
	
	synopGMAC_enable_dma_rx(gmacdev);
	synopGMAC_enable_dma_tx(gmacdev);
#if SYNOP_TOP_DEBUG
	dumpreg(regbase);
#endif

//	synopGMAC_rx_enable(gmacdev);	

#if SYNOP_TOP_DEBUG
	//dumpphyreg();
#endif

#if SYNOP_PHY_LOOPBACK

	gmacdev->LinkState = LINKUP;
	gmacdev->DuplexMode = FULLDUPLEX;
	gmacdev->Speed      =   SPEED1000;

#endif
        plat_delay(DEFAULT_LOOP_VARIABLE);
	synopGMAC_check_phy_init(gmacdev);
	synopGMAC_mac_init(gmacdev);
	
	PInetdev->sc_ih = pci_intr_establish(0, 0, IPL_NET, synopGMAC_intr_handler, adapter, 0);
	TR("register poll interrupt: gmac 0\n");

	return retval;

}

/**
 * Function used when the interface is closed.
 *
 * This function is registered to linux stop() function. This function is 
 * called whenever ifconfig (in Linux) closes the device (for example "ifconfig eth0 down").
 * This releases all the system resources allocated during open call.
 * system resources int needs 
 * 	- Disable the device interrupts
 * 	- Stop the receiver and get back all the rx descriptors from the DMA
 * 	- Stop the transmitter and get back all the tx descriptors from the DMA 
 * 	- Stop the Linux network queue interface
 *	- Free the irq (ISR registered is removed from the kernel)
 * 	- Release the TX and RX descripor memory
 *	- De-initialize one second timer rgistered for cable plug/unplug tracking
 * @param[in] pointer to net_device structure. 
 * \return Returns 0 on success and error status upon failure.
 * \callgraph
 */

/*
s32 synopGMAC_linux_close(struct net_device *netdev)
{
	
//	s32 status = 0;
//	s32 retval = 0;
//	u32 dma_addr;
	synopGMACPciNetworkAdapter *adapter;
        synopGMACdevice * gmacdev;
	struct pci_dev *pcidev;
	
	TR0("%s\n",__FUNCTION__);
	adapter = (synopGMACPciNetworkAdapter *) netdev->priv;
	if(adapter == NULL){
		TR0("OOPS adapter is null\n");
		return -1;
	}

	gmacdev = (synopGMACdevice *) adapter->synopGMACdev;
	if(gmacdev == NULL){
		TR0("OOPS gmacdev is null\n");
		return -1;
	}

	pcidev = (struct pci_dev *)adapter->synopGMACpcidev;
	if(pcidev == NULL){
		TR("OOPS pcidev is null\n");
		return -1;
	}

	synopGMAC_disable_interrupt_all(gmacdev);
	TR("the synopGMAC interrupt has been disabled\n");

	synopGMAC_disable_dma_rx(gmacdev);
        synopGMAC_take_desc_ownership_rx(gmacdev);
	TR("the synopGMAC Reception has been disabled\n");

	synopGMAC_disable_dma_tx(gmacdev);
        synopGMAC_take_desc_ownership_tx(gmacdev);

	TR("the synopGMAC Transmission has been disabled\n");
	netif_stop_queue(netdev);
	
	free_irq(pcidev->irq, netdev);
	TR("the synopGMAC interrupt handler has been removed\n");
	
	TR("Now calling synopGMAC_giveup_rx_desc_queue \n");
	synopGMAC_giveup_rx_desc_queue(gmacdev, pcidev, RINGMODE);
//	synopGMAC_giveup_rx_desc_queue(gmacdev, pcidev, CHAINMODE);
	TR("Now calling synopGMAC_giveup_tx_desc_queue \n");
	synopGMAC_giveup_tx_desc_queue(gmacdev, pcidev, RINGMODE);
//	synopGMAC_giveup_tx_desc_queue(gmacdev, pcidev, CHAINMODE);
	
	TR("Freeing the cable unplug timer\n");	
	del_timer(&synopGMAC_cable_unplug_timer);

	return -ESYNOPGMACNOERR;

//	TR("%s called \n",__FUNCTION__);
}
*/

/**
 * Function to transmit a given packet on the wire.
 * Whenever Linux Kernel has a packet ready to be transmitted, this function is called.
 * The function prepares a packet and prepares the descriptor and 
 * enables/resumes the transmission.
 * @param[in] pointer to sk_buff structure. 
 * @param[in] pointer to net_device structure.
 * \return Returns 0 on success and Error code on failure. 
 * \note structure sk_buff is used to hold packet in Linux networking stacks.
 */
#define FIX_CACHE_ADDR_UNALIGNED
//s32 synopGMAC_linux_xmit_frames(struct sk_buff *skb, struct net_device *netdev)
s32 synopGMAC_linux_xmit_frames(struct ifnet* ifp)
{
	s32 status = 0;
	u64 dma_addr;
	u32 offload_needed = 0;
	u32 bf1;
	u32 bf2;
	u32 index;
	DmaDesc * dpr;
	int len;
	int i;
	char * ptr;
	struct mbuf *skb;	//sw	we just use the name skb
	struct ether_header * eh;

	//u32 flags;
	struct synopGMACNetworkAdapter *adapter;
	synopGMACdevice * gmacdev;
#if SYNOP_TX_DEBUG
	TR("%s called \n",__FUNCTION__);
	printf("===xmit  yeh!\n");
#endif
	
	adapter = (struct synopGMACNetworkAdapter *) ifp->if_softc;
	if(adapter == NULL)
		return -1;

	gmacdev = (synopGMACdevice *) adapter->synopGMACdev;
	if(gmacdev == NULL)
		return -1;
#if SYNOP_TX_DEBUG
	printf("xmit: TxBusy = %d\tTxNext = %d\n",gmacdev->TxBusy,gmacdev->TxNext);
#endif
#if	defined(LOONGSON_2G5536)||defined(LOONGSON_2G1A) || defined(LOONGSON_2F1A)
	if(((gmacdev->LinkState) & Mii_phy_status_link_up) != Mii_phy_status_link_up){
		while(ifp->if_snd.ifq_head != NULL){
			IF_DEQUEUE(&ifp->if_snd, skb);
			m_freem(skb);
		}
		return;
	}
#endif

	while(ifp->if_snd.ifq_head != NULL){
		if(!synopGMAC_is_desc_owned_by_dma(gmacdev->TxNextDesc))
		{

			bf1 = (u32)plat_alloc_memory(TX_BUF_SIZE);
			if(bf1 == 0)
				return -1;
#ifdef FIX_CACHE_ADDR_UNALIGNED /*fixed by tangyt 2011-03-20*/
#if defined(LOONGSON_2G1A) || defined(LOONGSON_2F1A)
			if ((gmacdev->MacBase != LS1A_GMAC0_REG_BASE + MACBASE)
					&&(gmacdev->MacBase != LS1A_GMAC1_REG_BASE + MACBASE))
#endif
				while(bf1 & 0x10)
				{
					plat_free_memory((void *)(bf1));	
					bf1 = (u32)plat_alloc_memory(TX_BUF_SIZE);
					if(bf1 == 0)
						return -1;
				}
#endif
			memset((char *)bf1,0,TX_BUF_SIZE);

			IF_DEQUEUE(&ifp->if_snd, skb);

			/*Now we have skb ready and OS invoked this function. Lets make our DMA know about this*/



			len = skb->m_pkthdr.len;


			//sw: i don't know weather it's right
			m_copydata(skb, 0, len,(char *)bf1);

#if defined(LOONGSON_2G1A) || defined(LOONGSON_2F1A)
			if ((gmacdev->MacBase == LS1A_GMAC0_REG_BASE + MACBASE)
					|| (gmacdev->MacBase == LS1A_GMAC1_REG_BASE + MACBASE)) {
				dma_addr = plat_dma_map_single(gmacdev, bf1, len, SYNC_W);
			}
#endif

#if defined(LOONGSON_2G1A) || defined(LOONGSON_2F1A)
			if ((gmacdev->MacBase != LS1A_GMAC0_REG_BASE + MACBASE)
					&& (gmacdev->MacBase != LS1A_GMAC1_REG_BASE + MACBASE))
#endif
				eh = mtod(skb, struct ether_header *);
#if SYNOP_TX_DEBUG
			dumppkghd(eh,0);

			for(i = 0;i < len;i++)
			{
				ptr = (u32)bf1;
				printf(" %02x",*(ptr+i));
			}
			printf("\n");
#endif


#if	defined(LOONGSON_2G5536)||defined(LOONGSON_2G1A) || defined(LOONGSON_2F1A)
			m_freem(skb);
#else
			plat_free_memory(skb);
#endif

#if defined(LOONGSON_2G1A) || defined(LOONGSON_2F1A)
			if ((gmacdev->MacBase != LS1A_GMAC0_REG_BASE + MACBASE)
					&& (gmacdev->MacBase != LS1A_GMAC1_REG_BASE + MACBASE)) {
				bf2  = (u32)CACHED_TO_UNCACHED((unsigned long)bf1);
				dma_addr  = (unsigned long)UNCACHED_TO_PHYS((unsigned long)(bf2));
			}
#else
			bf2  = (u32)CACHED_TO_UNCACHED((unsigned long)bf1);	
			dma_addr  = (unsigned long)UNCACHED_TO_PHYS((unsigned long)(bf2));
#ifndef LOONGSON_2G5536
			dma_addr &= 0x0fffffff;
#endif
#endif

			//status = synopGMAC_set_tx_qptr(gmacdev, dma_addr, TX_BUF_SIZE, bf1,0,0,0,offload_needed);
				
			status = synopGMAC_set_tx_qptr(gmacdev, dma_addr, len, bf1,0,0,0,offload_needed,&index,dpr);
//			printf("synopGMAC_set_tx_qptr: index = %d, dma_addr = %08x, 
//					len = %02x, buf1 = %08x\n", index, dma_addr, len, bf1);
//			printf("===%x: next txDesc belongs to DMA don't set it\n",gmacdev->TxNextDesc);
//			dumpdesc(gmacdev);	// by xqch to dump all desc
//			dumpreg(regbase);	//by xqch to dumpall dma reg
#if SYNOP_TX_DEBUG
			printf("status = %d \n",status);
#endif


			if(status < 0){
#if SYNOP_TX_DEBUG
				TR("%s No More Free Tx Descriptors\n",__FUNCTION__);
#endif
				//dev_kfree_skb (skb); //with this, system used to freeze.. ??
				return -EBUSY;
			}
		}
#if	defined(LOONGSON_2G5536)||defined(LOONGSON_2G1A) || defined(LOONGSON_2F1A)
		else{
			return;
		}
#endif
	}
	
//	ls2h_flush_cache2();
	/*Now force the DMA to start transmission*/	
#if SYNOP_TX_DEBUG
	{
		u32 data;
		data = synopGMACReadReg(gmacdev->DmaBase, 0x48);
		printf("TX DMA DESC ADDR = 0x%x\n",data);
	}
#endif
	synopGMAC_resume_dma_tx(gmacdev);

	return -ESYNOPGMACNOERR;
}

/**
 * Function provides the network interface statistics.
 * Function is registered to linux get_stats() function. This function is
 * called whenever ifconfig (in Linux) asks for networkig statistics
 * (for example "ifconfig eth0").
 * @param[in] pointer to net_device structure.
 * \return Returns pointer to net_device_stats structure.
 * \callgraph
 */
struct net_device_stats *  synopGMAC_linux_get_stats(struct synopGMACNetworkAdapter *tp)
{
	TR("%s called \n",__FUNCTION__);
	return( &(((struct synopGMACNetworkAdapter *)(tp))->synopGMACNetStats) );
}

/**
 * Function to set multicast and promiscous mode.
 * @param[in] pointer to net_device structure.
 * \return returns void.
 */

/*
void synopGMAC_linux_set_multicast_list(struct net_device *netdev)
{
TR("%s called \n",__FUNCTION__);
//todo Function not yet implemented.
return;
}
*/

/**
 * Function to set ethernet address of the NIC.
 * @param[in] pointer to net_device structure.
 * @param[in] pointer to an address structure.
 * \return Returns 0 on success Errorcode on failure.
 */

/*
s32 synopGMAC_linux_set_mac_address(struct net_device *netdev, void * macaddr)
{

synopGMACPciNetworkAdapter *adapter = NULL;
synopGMACdevice * gmacdev = NULL;
struct sockaddr *addr = macaddr;

adapter = (synopGMACPciNetworkAdapter *) netdev->priv;
if(adapter == NULL)
	return -1;

gmacdev = adapter->synopGMACdev;
if(gmacdev == NULL)
	return -1;

if(!is_valid_ether_addr(addr->sa_data))
	return -EADDRNOTAVAIL;

synopGMAC_set_mac_addr(gmacdev,GmacAddr0High,GmacAddr0Low, addr->sa_data);
synopGMAC_get_mac_addr(synopGMACadapter->synopGMACdev,GmacAddr0High,GmacAddr0Low, netdev->dev_addr);

TR("%s called \n",__FUNCTION__);
return 0;
}
*/



/**
 * Function to change the Maximum Transfer Unit.
 * @param[in] pointer to net_device structure.
 * @param[in] New value for maximum frame size.
 * \return Returns 0 on success Errorcode on failure.
 */

/*
s32 synopGMAC_linux_change_mtu(struct net_device *netdev, s32 newmtu)
{
TR("%s called \n",__FUNCTION__);
//todo Function not yet implemented.
return 0;

}
*/

/**
 * IOCTL interface.
 * This function is mainly for debugging purpose.
 * This provides hooks for Register read write, Retrieve descriptor status
 * and Retreiving Device structure information.
 * @param[in] pointer to net_device structure.
 * @param[in] pointer to ifreq structure.
 * @param[in] ioctl command.
 * \return Returns 0 on success Error code on failure.
 */

#if UNUSED
void set_phy_manu(synopGMACdevice * gmacdev)
{
	u16 data;
	int status;
	int i;
	i = 100;

	printf("set phy manu\n");
	
	status = synopGMAC_read_phy_reg(gmacdev->MacBase,gmacdev->PhyBase,PHY_CONTROL_REG, &data);
	data = data & ~0x1000 | 0x100;
	
	printf("===data %x\n",data);
	status = synopGMAC_write_phy_reg(gmacdev->MacBase,gmacdev->PhyBase,PHY_CONTROL_REG, &data);
	if (status != 0)
		printf("===write phy error\n");
	
	while(i)
		i--;
	//dumpphyreg();
	synopGMAC_check_phy_init(gmacdev);


}
#endif

#if defined(LOONGSON_2G1A) || defined(LOONGSON_2F1A) || (defined(LOONGSON_3A2H) && defined(LOONGSON_3A8)) || defined(LOONGSON_2K)
static int rtl88e1111_config_init(synopGMACdevice *gmacdev)
{
	int retval, err;
	u16 data;

	synopGMAC_read_phy_reg(gmacdev->MacBase,gmacdev->PhyBase,0x14,&data);
	data = data | 0x82;
	err = synopGMAC_write_phy_reg(gmacdev->MacBase,gmacdev->PhyBase,0x14,data);
	synopGMAC_read_phy_reg(gmacdev->MacBase,gmacdev->PhyBase,0x00,&data);
	data = data | 0x8000;
	err = synopGMAC_write_phy_reg(gmacdev->MacBase,gmacdev->PhyBase,0x00,data);

	if (err < 0)
		return err;
	return 0;
}
#endif

#if defined(LOONGSON_2G1A) || defined(LOONGSON_2F1A) || (defined(LOONGSON_3A2H) && defined(LOONGSON_3A8)) || defined(LOONGSON_2K)
int init_phy(synopGMACdevice *gmacdev)
#else
int init_phy(struct synopGMACdevice *gmacdev)
#endif
{
	int retval;
#if defined(LOONGSON_2G1A) || defined(LOONGSON_2F1A)
	u16 data;
	if ((gmacdev->MacBase == LS1A_GMAC0_REG_BASE + MACBASE)
		|| (gmacdev->MacBase == LS1A_GMAC1_REG_BASE + MACBASE)) {
		synopGMAC_read_phy_reg(gmacdev->MacBase,gmacdev->PhyBase,2,&data);
		if(data == 0x141)
			rtl88e1111_config_init(gmacdev);
		return 0;
	} else {
		retval = rtl8211_config_init(gmacdev);
		return retval;
	}
#elif (defined(LOONGSON_3A2H) && defined(LOONGSON_3A8)) || defined(LOONGSON_2K)
#if defined(LOONGSON_2K)
	u16 data;
	synopGMAC_read_phy_reg(gmacdev->MacBase,gmacdev->PhyBase,2,&data);
	/*set 88e1111 clock phase delay*/
	if(data == 0x141)
#endif
{
	rtl88e1111_config_init(gmacdev);
	return 0;
}
#else
	retval = rtl8211_config_init(gmacdev);
	return retval;
#endif
}


#if UNUSED
s32 synopGMAC_linux_do_ioctl(struct ifnet *ifp, struct ifreq *ifr, s32 cmd)
{
	s32 retval = 0;
	u16 temp_data = 0;
	struct synopGMACNetworkAdapter *adapter = NULL;
	synopGMACdevice * gmacdev = NULL;
	struct ifr_data_struct
	{
		u32 unit;
		u32 addr;
		u32 data;
	} *req;


	if(ifr == NULL)
		return -1;

	req = (struct ifr_data_struct *)ifr->ifr_data;

	adapter = (struct synopGMACNetworkAdapter *) ifp->if_softc;
	if(adapter == NULL)
		return -1;

	gmacdev = adapter->synopGMACdev;

	if(gmacdev == NULL)
		return -1;
//	TR("%s :: on device %s req->unit = %08x req->addr = %08x 
//			req->data = %08x cmd = %08x \n",__FUNCTION__,netdev->name,
//							req->unit,req->addr,req->data,cmd);

	switch(cmd)
	{
		case IOCTL_READ_REGISTER:		//IOCTL for reading IP registers : Read Registers
			if(req->unit == 0)		// Read Mac Register
				req->data = synopGMACReadReg(gmacdev->MacBase,req->addr);
			else if (req->unit == 1)	// Read DMA Register
				req->data = synopGMACReadReg(gmacdev->DmaBase,req->addr);
			else if (req->unit == 2){	// Read Phy Register
				retval = synopGMAC_read_phy_reg(gmacdev->MacBase,
						gmacdev->PhyBase,req->addr,&temp_data);
				req->data = (u32)temp_data;
				if(retval != -ESYNOPGMACNOERR)
					TR("ERROR in Phy read\n");	
			}
			break;

		case IOCTL_WRITE_REGISTER:		//IOCTL for reading IP registers : Read Registers
			if(req->unit == 0)		// Write Mac Register
				synopGMACWriteReg(gmacdev->MacBase,req->addr,req->data);
			else if (req->unit == 1)	// Write DMA Register
				synopGMACWriteReg(gmacdev->DmaBase,req->addr,req->data);
			else if (req->unit == 2){	// Write Phy Register
				retval = synopGMAC_write_phy_reg(gmacdev->MacBase,
						gmacdev->PhyBase,req->addr,req->data);
				if(retval != -ESYNOPGMACNOERR)
					TR("ERROR in Phy read\n");	
			}
			break;

		case IOCTL_READ_IPSTRUCT:		//IOCTL for reading GMAC DEVICE IP private structure
	        	memcpy(ifr->ifr_data, gmacdev, sizeof(synopGMACdevice));
			break;

		case IOCTL_READ_RXDESC:			//IOCTL for Reading Rx DMA DESCRIPTOR
			memcpy(ifr->ifr_data, gmacdev->RxDesc
				+ ((DmaDesc *) (ifr->ifr_data))->data1, sizeof(DmaDesc) );
			break;

		case IOCTL_READ_TXDESC:			//IOCTL for Reading Tx DMA DESCRIPTOR
			memcpy(ifr->ifr_data, gmacdev->TxDesc
				+ ((DmaDesc *) (ifr->ifr_data))->data1, sizeof(DmaDesc) );
			break;
		case IOCTL_POWER_DOWN:
			if(req->unit == 1){		//power down the mac
				TR("============I will Power down the MAC now =============\n");
				// If it is already in power down don't power down again
				retval = 0;
				if(((synopGMACReadReg(gmacdev->MacBase,GmacPmtCtrlStatus))
					& GmacPmtPowerDown)!= GmacPmtPowerDown){
					synopGMAC_linux_powerdown_mac(gmacdev);			
					retval = 0;
				}
			}
			if(req->unit == 2){		//Disable the power down  and wake up the Mac locally
				TR("============I will Power up the MAC now =============\n");
				//If already powered down then only try to wake up
				retval = -1;
				if(((synopGMACReadReg(gmacdev->MacBase,GmacPmtCtrlStatus))
					& GmacPmtPowerDown) == GmacPmtPowerDown){
					synopGMAC_power_down_disable(gmacdev);
					synopGMAC_linux_powerup_mac(gmacdev);
					retval = 0;
				}
			}
			break;
		default:
			retval = -1;

	}


	return retval;
}
#endif

void dumppkghd(struct ether_header *eh,int tp)
{
		int i;
//sw: dbg
		if(tp == 1)
	       		printf("\n===Rx:  pkg dst:  ");
		else
	       		printf("\n===Tx:  pkg dst:  ");
		
		for(i = 0;i < 6;i++)
	       		printf(" %02x",eh->ether_dhost[i]);
		
		if(tp == 1)
			printf("\n===Rx:  pkg sst:  ");
		else
			printf("\n===Tx:  pkg sst:  ");
	
		for(i = 0;i < 6;i++)
	       		printf(" %02x",eh->ether_shost[i]);
		
		if(tp == 1)
			printf("\n===Rx:  pkg type:  ");
		else
			printf("\n===Tx:  pkg type:  ");
		printf(" %12x",eh->ether_type);
		printf("\n");
		
//dbg
}




s32 synopGMAC_dummy_reset(struct ifnet *ifp)
{
	
	struct synopGMACNetworkAdapter * adapter;
	synopGMACdevice	* gmacdev;
	
	adapter = (struct synopGMACNetworkAdapter *)ifp->if_softc;
	gmacdev = adapter->synopGMACdev;
	
	return synopGMAC_reset(gmacdev);
}


s32 synopGMAC_dummy_ioctl(struct ifnet *ifp)
{
//	printf("==dummy ioctl\n");
	return 0;
}
//sw:	i just copy this function from rtl8169.c

static int gmac_ether_ioctl(struct ifnet *ifp, unsigned long cmd, caddr_t data)
{
	struct ifaddr *ifa;
	struct synopGMACNetworkAdapter *adapter;
	int error = 0;
	int s;

	adapter = ifp->if_softc;
	ifa = (struct ifaddr *) data;

	s = splimp();
	switch (cmd) {
#ifdef PMON
	case SIOCPOLL:
	{
		break;
	}
#endif
	case SIOCSIFADDR:
		switch (ifa->ifa_addr->sa_family) {
#ifdef INET
		case AF_INET:
			if (!(ifp->if_flags & IFF_UP))			
			{	
				error = synopGMAC_linux_open(adapter);
			}	
	
			if(error == -1){
				return(error);
			}	
			ifp->if_flags |= IFF_UP;
#ifdef __OpenBSD__
			arp_ifinit(&(adapter->PInetdev->arpcom), ifa);
			printf("==arp_ifinit done\n");
#else
			arp_ifinit(ifp, ifa);
#endif
			
			break;
#endif

		default:
			synopGMAC_linux_open(adapter);
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

		printf("===ioctl sifflags\n");
		if(ifp->if_flags & IFF_UP){
			synopGMAC_linux_open(adapter);
		}
		break;
/*
        case SIOCETHTOOL:
        {
        long *p=data;
        myRTL = sc;
        cmd_setmac(p[0],p[1]);
        }
        break;
       case SIOCRDEEPROM:
                {
                long *p=data;
                myRTL = sc;
                cmd_reprom(p[0],p[1]);
                }
                break;
       case SIOCWREEPROM:
                {
                long *p=data;
                myRTL = sc;
                cmd_wrprom(p[0],p[1]);
                }
                break;
*/
	default:
		printf("===ioctl default\n");
		dumpreg(regbase);
		error = EINVAL;
	}

	splx(s);

	if(error)
		printf("===ioctl error: %d\n",error);
	return (error);
}

void dumpdesc(synopGMACdevice	* gmacdev)
{
	int i;
	
	printf("\n===dump Tx desc");
	for(i =0; i < gmacdev -> TxDescCount; i++){
		printf("\n%02d %08x : ",i,(unsigned int)(gmacdev->TxDesc + i));
		printf("%08x ",(unsigned int)((gmacdev->TxDesc + i))->status);
		printf("%08x ",(unsigned int)((gmacdev->TxDesc + i)->length));
		printf("%08x ",(unsigned int)((gmacdev->TxDesc + i)->buffer1));
		printf("%08x ",(unsigned int)((gmacdev->TxDesc + i)->buffer2));
		printf("%08x ",(unsigned int)((gmacdev->TxDesc + i)->data1));
		printf("%08x ",(unsigned int)((gmacdev->TxDesc + i)->data2));
		printf("%08x ",(unsigned int)((gmacdev->TxDesc + i)->dummy1));
		printf("%08x ",(unsigned int)((gmacdev->TxDesc + i)->dummy2));
	}
	printf("\n===dump Rx desc");
	for(i =0; i < gmacdev -> RxDescCount; i++){
		printf("\n%02d %08x : ",i,(unsigned int)(gmacdev->RxDesc + i));
		printf("%08x ",(unsigned int)((gmacdev->RxDesc + i))->status);
		printf("%08x ",(unsigned int)((gmacdev->RxDesc + i)->length));
		printf("%08x ",(unsigned int)((gmacdev->RxDesc + i)->buffer1));
		printf("%08x ",(unsigned int)((gmacdev->RxDesc + i)->buffer2));
		printf("%08x ",(unsigned int)((gmacdev->RxDesc + i)->data1));
		printf("%08x ",(unsigned int)((gmacdev->RxDesc + i)->data2));
		printf("%08x ",(unsigned int)((gmacdev->RxDesc + i)->dummy1));
		printf("%08x ",(unsigned int)((gmacdev->RxDesc + i)->dummy2));
	}

	printf("\n\n");
	
}


void dumpreg(u64 gbase)
{
	int i;
	int k;
	u32 data;
	
	printf("\n==== gmac:0 dumpreg\n");
	for (i = 0,k = 0; i < 0xbc; i = i+4,k++)
	{
		data = synopGMACReadReg(gbase, i );
		printf("  reg:%2x value:%8x  ",i,data);
		if(k%4 == 3)
			printf("\n");
	}
	printf("\n");
	for (i = 0xc0,k = 0; i < 0xdc; i = i+4,k++){	
		data = synopGMACReadReg(gbase, i );
		printf("  reg:%2x value:%8x  ",i,data);
		if(k%4 == 3)
			printf("\n");
	}
	printf("\n");

	for (i = 0,k = 0; i < 0x5c; i = i+4,k++){
		data = synopGMACReadReg(gbase+0x1000, i );
		printf("  reg:%2x value:%8x  ",i,data);
		if(k%4 == 3)
			printf("\n");

	}
	printf("\n\n");
}

void dumpphyreg(gbase)
{
	u16 data;
	int i;
	printf("===dump mii phy regs of GMAC: 0\n");
	for(i = 0x0; i <= 0x1f; i++)
	{
		synopGMAC_read_phy_reg(gbase+0x1000,0x10,i, &data);
		printf("  mii phy reg: %x    value: %x  \n",i,data);
	}
	printf("\n");
}

/*
void setphysreg0(synopGMACdevice * gmacdev,int reg,int val)
{
//sw: exp-reg 111 shoud be 7;
	int i;
	u16 data0[5] = {0x7007,0x4007,0x2007,0x1007,0x0007};
	u16 sreg0[5] = {0x111,0x100,0x010,0x001,0x000};
	u16 data;
	
	for(i = 0;i < 5;i++)
	{
		if(sreg0[i] == reg)
			break;
	}	
	
	synopGMAC_write_phy_reg(gmacdev->MacBase,1,0x18, data0[i]);
	synopGMAC_read_phy_reg(gmacdev->MacBase,1,0x18, &data);

	synopGMAC_write_phy_reg(gmacdev->MacBase,1,reg, val);
}
*/

#if UNUSED
void test_tx(struct ifnet * ifp)
{
	int s;
	u32 skb;
	int error;
	struct mbuf *m;
	int i;

for(i = 0;i < 150;i++)
{	
	m = (struct mbuf *)plat_alloc_memory(sizeof(struct mbuf));

	skb = (u32)plat_alloc_memory(TX_BUF_SIZE);		//should skb aligned here?
	if(skb == NULL){
			TR0("ERROR in skb buffer allocation\n");
//			return -ESYNOPGMACNOMEM;
	}

	mtod(m,u32)  = skb;	
	
	s = splimp();
	/*
	 * Queue message on interface, and start output if interface
	 * not yet active.
	 */
	if (IF_QFULL(&ifp->if_snd)) {
		IF_DROP(&ifp->if_snd);
		splx(s);
	}
	ifp->if_obytes += m->m_pkthdr.len;
	IF_ENQUEUE(&ifp->if_snd, m);
	if (m->m_flags & M_MCAST)
		ifp->if_omcasts++;
	if ((ifp->if_flags & IFF_OACTIVE) == 0)
		(*ifp->if_start)(ifp);
	splx(s);

//	dumpreg(regbase);

}
	return (error);


}

int set_lpmode(synopGMACdevice * gmacdev)
{
	u16 data;
	int status;
	int delay;
	
	printf("===reset phy...\n");

	status = synopGMAC_read_phy_reg(gmacdev->MacBase,1,0,&data);
//sw: if you set bit 13,it resets!!
//	data = 0x6000;
	data = 0x4040;
	printf("===set phy loopback mode , reg0: %x\n",data);
	
	status = synopGMAC_write_phy_reg(gmacdev->MacBase,1,0,data);
	if(status != 0)
		return 0;

	delay = 200;
	while(delay > 0)
		delay--;
	
	status = synopGMAC_read_phy_reg(gmacdev->MacBase,1,0,&data);
	printf("===phy loopback mode , reg0: %x\n",data);
	
	return 1;
	
}


void memory_test()
{
	int dma_addr;
	int skb;
	
while(1){
	skb = (u32)plat_alloc_memory(RX_BUF_SIZE);		//should skb aligned here?
	printf("==malloc addr: %x   size:1536B \n",skb);
	if(skb == NULL){
			TR0("ERROR in skb buffer allocation\n");
	}
		
//	skb = (u32)CACHED_TO_UNCACHED((unsigned long)(skb));	
	//dma_addr  = (unsigned long)vtophys((unsigned long)(skb));
	dma_addr  = (unsigned long)UNCACHED_TO_PHYS((unsigned long)(skb));

#if	(!defined(LOONGSON_2G5536))&&(!defined(LOONGSON_2G1A))&&(!defined(LOONGSON_2F1A))
 		dma_addr &= 0x0fffffff;
		dma_addr |= 0x00000000;
#endif

	printf("==free addr: %x   size:1536B \n",skb);
	plat_free_memory(skb);
	}
}

void reg_init(synopGMACdevice * gmacdev)
{
	u32 data;

	synopGMACWriteReg(gmacdev->DmaBase, DmaBusMode,0x0);
	dumpreg(regbase);
	
	data = synopGMACReadReg(gmacdev->DmaBase, DmaBusMode);
  	data |= 0x400; 
	synopGMACWriteReg(gmacdev->DmaBase, DmaBusMode,data);

	data = synopGMACReadReg(gmacdev->MacBase, GmacConfig );
  	data |= 0x800; 
	synopGMACWriteReg(gmacdev->MacBase, GmacConfig,data);
	
	data = synopGMACReadReg(gmacdev->MacBase, GmacFrameFilter );
  	data |= 0x80000000; 
	synopGMACWriteReg(gmacdev->MacBase, GmacFrameFilter ,data);

	dumpreg(regbase);
	
	data = synopGMACReadReg(gmacdev->DmaBase, DmaControl );
  	data |= 0x2000; 
	synopGMACWriteReg(gmacdev->DmaBase, DmaControl,data);

	data = synopGMACReadReg(gmacdev->MacBase, GmacConfig );
  	data |= 0x4; 
	synopGMACWriteReg(gmacdev->MacBase, GmacConfig,data);

	data = synopGMACReadReg(gmacdev->MacBase, GmacConfig );
  	data |= 0x8; 
	synopGMACWriteReg(gmacdev->MacBase, GmacConfig,data);

	printf("====done! OK!\n");
	dumpreg(regbase);
	
}
	
void setup_tx_desc(synopGMACdevice * gmacdev)
{

	
	int i,j;
	int * first_desc;
	int * descbuf;
	int * TxDesc;
	int * buf;
	
	
	TR("Total size of memory required for Tx Descriptors in Ring Mode = 0x%08x\n",(4*(sizeof(int) * 4)));
//	first_desc = plat_alloc_consistent_dmaable_memory (pcidev, sizeof(DmaDesc) * no_of_desc,&dma_addr);
	first_desc = (int **)plat_alloc_memory(4 * sizeof(int) * 4);	//should first_desc aligned here?
	buf = (void *)plat_alloc_memory(1536);	//max frame
	if(first_desc == NULL){
		TR("Error in Tx Descriptors memory allocation\n");
		return -ESYNOPGMACNOMEM;
	}
	
	if(buf == NULL){
		TR("Error in Tx Descriptors buf memory allocation\n");
		return -ESYNOPGMACNOMEM;
	}
	
	TR("==first_desc: %x\n",first_desc);
	TR("==desc buf: %x\n",buf);

	gmacdev->TxDesc      = first_desc;
	gmacdev->TxDescDma   = first_desc;
	
	memset((u32)first_desc,0,4 * sizeof(int) * 4);
	
	TxDesc = first_desc;
	descbuf = buf;
	
	for(i = 0;i < 16;i = i+4)
	{
		TxDesc[i] = 0xb0000000;
		TxDesc[i+1] = 0x60000400;
		TxDesc[i+2] = descbuf;
	}
		
	TxDesc[12] == 0xb0200000;
		
	
		for(i =0; i < 16;i = i+4){
			TR("==%x\n",TxDesc[i]);
			TR("==%x\n",TxDesc[i+1]);
			TR("==%x\n",TxDesc[i+2]);
			TR("==%x\n\n",TxDesc[i+3]);
		}

	for(i = 0;i < 1536;i++)
	{
		descbuf[i] = 0xffffffff;
	}

		
}
#endif

#if	defined(LOONGSON_2G5536)||defined(LOONGSON_2G1A) || defined(LOONGSON_2F1A)
void parseenv(int index, u8 * buf)
{
	int i;
	char * buffer = (char *)0xbfc00000 + NVRAM_OFFS;

	if(index == 0)
		for(i = 0; i < 6; i++)
			buf[i] = buffer[ETHER_OFFS + i];
	else if(index == 1)
		for(i = 0; i < 6; i++)
			buf[i] = buffer[ETHER1_OFFS + i];
}
#endif

/**
 * Function to handle a Tx Hang.
 * This is a software hook (Linux) to handle transmitter hang if any.
 * We get transmitter hang in the device interrupt status, and is handled
 * in ISR. This function is here as a place holder.
 * @param[in] pointer to net_device structure 
 * \return void.
 */

/*
void synopGMAC_linux_tx_timeout(struct net_device *netdev)
{
TR("%s called \n",__FUNCTION__);
//todo Function not yet implemented
return;
}
*/


/**
 * Function to initialize the Linux network interface.
 * 
 * Linux dependent Network interface is setup here. This provides 
 * an example to handle the network dependent functionality.
 *
 * \return Returns 0 on success and Error code on failure.
 */

void set_phyled(struct synopGMACNetworkAdapter *synopGMACadapter)
{
        u16 data;
        s32 status = -ESYNOPGMACNOERR;
        struct synopGMACNetworkAdapter *adapter = synopGMACadapter;
        synopGMACdevice * gmacdev;
        adapter = synopGMACadapter;
        gmacdev = (synopGMACdevice *)adapter->synopGMACdev;

	synopGMAC_write_phy_reg(gmacdev->MacBase,gmacdev->PhyBase,0x1f, 0x0007);
        synopGMAC_write_phy_reg(gmacdev->MacBase,gmacdev->PhyBase,0x1e, 0x002c);
        synopGMAC_write_phy_reg(gmacdev->MacBase,gmacdev->PhyBase,0x1a, 0x10);
        synopGMAC_write_phy_reg(gmacdev->MacBase,gmacdev->PhyBase,0x1c, 0x420);
        synopGMAC_write_phy_reg(gmacdev->MacBase,gmacdev->PhyBase,0x1f, 0x0);

	printf("Set phy led end\n");
}

#if	defined(LOONGSON_2G5536)||defined(LOONGSON_2G1A) || defined(LOONGSON_2F1A)
s32  synopGMAC_init_network_interface(char* xname,u64 synopGMACMappedAddr)
{
	struct ifnet* ifp;

	static u8 mac_addr0[6] = DEFAULT_MAC_ADDRESS;
	int i;
	u16 data;
	struct synopGMACNetworkAdapter * synopGMACadapter;

	static int syn_num;

#ifdef LOONGSON_2F1A
	if(LS1A_GMAC0_REG_BASE == synopGMACMappedAddr)
		parseenv(0, mac_addr0);
	else
		parseenv(1, mac_addr0);
#else
	if(0x90000c0000000000LL == synopGMACMappedAddr)
		parseenv(0, mac_addr0);
	else
		parseenv(1, mac_addr0);
#endif
#elif defined(LOONGSON_2K)
extern unsigned char smbios_uuid_gmac[6];
s32  synopGMAC_init_network_interface(char* xname, u64 synopGMACMappedAddr)
{
	struct ifnet* ifp;
	static u8 mac_addr0[6];
	int i, v;
	u16 data;
	u64 gmac0, gmac1;
	struct synopGMACNetworkAdapter * synopGMACadapter;
	unsigned int eeprom_addr;
	i = strtoul(xname + 3, NULL, 0);
	eeprom_addr = i * 6;

	gmac0 = synopGMACMappedAddr;
//	gmac1 = 0xffffffff00000000ULL | LS2H_GMAC1_REG_BASE;
//	synopGMACMappedAddr = sc->dv_unit?gmac1:gmac0;
	synopGMACMappedAddr = gmac0;//mtf
//	i2c_init();//the i2s had init by file i2c.S
	mac_read(eeprom_addr, mac_addr0, 6);
	memcpy(smbios_uuid_gmac, mac_addr0, 6);
#else
s32  synopGMAC_init_network_interface(char* xname, struct device *sc )
{
	struct ifnet* ifp;
	extern unsigned char smbios_uuid_mac[6];

	u8 mac_addr0[6];
	int i,v;
	u16 data;
	u64 synopGMACMappedAddr = sc->dv_unit?0xffffffffbbe18000LL:0xffffffffbbe10000LL;
	unsigned int eeprom_addr = sc->dv_unit * 6;
	struct synopGMACNetworkAdapter *synopGMACadapter;

	memcpy(smbios_uuid_mac, mac_addr0, 6);
#endif
	TR("Now Going to Call register_netdev to \
			register the network interface for GMAC core\n");

	synopGMACadapter = (struct synopGMACNetworkAdapter * )
			plat_alloc_memory(sizeof (struct synopGMACNetworkAdapter));

	//sw:	should i put sync_cache here?
	memset((char *)synopGMACadapter, 0, sizeof (struct synopGMACNetworkAdapter));

	synopGMACadapter->synopGMACdev    = NULL;
	synopGMACadapter->PInetdev   = NULL;
	
	/*Allocate Memory for the the GMACip structure*/
	synopGMACadapter->synopGMACdev = (synopGMACdevice *) plat_alloc_memory(sizeof (synopGMACdevice));
	memset((char *)synopGMACadapter->synopGMACdev ,0, sizeof (synopGMACdevice));

	if(!synopGMACadapter->synopGMACdev){
		TR0("Error in Memory Allocataion \n");
	}
		
	/*Allocate Memory for the the GMAC-Pmon structure	sw*/
	synopGMACadapter->PInetdev = (struct PmonInet *) plat_alloc_memory(sizeof (struct PmonInet));
	memset((char *)synopGMACadapter->PInetdev ,0, sizeof (struct PmonInet));
	if(!synopGMACadapter->PInetdev){
		TR0("Error in Pdev-Memory Allocataion \n");
	}

	synopGMAC_attach(synopGMACadapter->synopGMACdev,
			(u64) synopGMACMappedAddr + MACBASE,
			(u64) synopGMACMappedAddr + DMABASE,	
			DEFAULT_PHY_BASE, mac_addr0);
#if SYNOP_TOP_DEBUG
	dumpphyreg(synopGMACadapter);
#endif

	init_phy(synopGMACadapter->synopGMACdev);
	synopGMAC_reset(synopGMACadapter->synopGMACdev);
#if	(!defined(LOONGSON_2G5536))&&(!defined(LOONGSON_2G1A)) && (!defined(LOONGSON_2F1A))
	synopGMAC_attach(synopGMACadapter->synopGMACdev,
			(u64) synopGMACMappedAddr + MACBASE,
			(u64) synopGMACMappedAddr + DMABASE,
			DEFAULT_PHY_BASE, mac_addr0);

	TR("0xbfe10040:%x\n",synopGMACReadReg(0xffffffffbfe10040,0));
	TR("0xbfe10040:%x\n",synopGMACReadReg(0xffffffffbfe10040,0));
#endif
	
	ifp = &(synopGMACadapter->PInetdev->arpcom.ac_if);
	ifp->if_softc = synopGMACadapter;
#if	(!defined(LOONGSON_2G5536))&&(!defined(LOONGSON_2G1A)) && (!defined(LOONGSON_2F1A)) && (!defined(LOONGSON_2K))
	memcpy(&synopGMACadapter->PInetdev->sc_dev, sc, sizeof(struct device));
#endif	
	memcpy(synopGMACadapter->PInetdev->dev_addr, mac_addr0,6);


	bcopy(synopGMACadapter->PInetdev->dev_addr, synopGMACadapter->PInetdev->arpcom.ac_enaddr, sizeof(synopGMACadapter->PInetdev->arpcom.ac_enaddr));		//sw: set mac addr manually


	bcopy(xname, ifp->if_xname, IFNAMSIZ);

	ifp->if_start = (void *)synopGMAC_linux_xmit_frames;
	ifp->if_ioctl = (int *)gmac_ether_ioctl;
//	ifp->if_ioctl = (int *)synopGMAC_dummy_ioctl;
	ifp->if_reset = (int *)synopGMAC_dummy_reset;

	ifp->if_snd.ifq_maxlen = TRANSMIT_DESC_SIZE - 1;	//defined in Dev.h value is 12, too small?


	/*Now start the network interface*/
	TR("\nNow Registering the netdevice\n");
	if_attach(ifp);
	ether_ifattach(ifp);
	ifp->if_flags = ifp->if_flags | IFF_RUNNING;

#if SYNOP_TOP_DEBUG
	dumpreg(regbase);
	dumpphyreg();
#if SYNOP_GMAC0
	dumpdesc(synopGMACadapter->synopGMACdev);
#endif
#endif
#ifdef LOONGSON_2GQ2H
	set_phyled(synopGMACadapter);
#endif
#if 0
	dumpphyregg(synopGMACadapter->synopGMACdev);
	dumpmacregg(synopGMACadapter->synopGMACdev);
	dumpdmaregg(synopGMACadapter->synopGMACdev);
#endif
#if	defined(LOONGSON_2G5536)||defined(LOONGSON_2G1A) || defined(LOONGSON_2F1A)
	mac_addr0[5]++;
#endif
	return 0;
}


/**
 * Function to initialize the Linux network interface.
 * Linux dependent Network interface is setup here. This provides
 * an example to handle the network dependent functionality.
 * \return Returns 0 on success and Error code on failure.
 */

/*
void __exit synopGMAC_exit_network_interface(void)
{
	TR0("Now Calling network_unregister\n");
	unregister_netdev(synopGMACadapter->synopGMACnetdev);	
}
*/

/*
module_init(synopGMAC_init_network_interface);
module_exit(synopGMAC_exit_network_interface);

MODULE_AUTHOR("Synopsys India");
MODULE_LICENSE("GPL/BSD");
MODULE_DESCRIPTION("SYNOPSYS GMAC DRIVER Network INTERFACE");

EXPORT_SYMBOL(synopGMAC_init_pci_bus_interface);
*/

void gmac_stop(void)
{
	register struct ifnet *ifp0,*ifp1;
	struct synopGMACNetworkAdapter *adapter0,*adapter1;
	synopGMACdevice *gmacdev0,*gmacdev1;

	ifp0 = ifunit("syn0");
	ifp1 = ifunit("syn1");

	if(ifp0 != 0){ 
		adapter0 = (struct synopGMACNetworkAdapter *)ifp0->if_softc;
		gmacdev0 = (synopGMACdevice *)adapter0->synopGMACdev;

		synopGMAC_disable_dma_tx(gmacdev0);
		synopGMAC_tx_disable(gmacdev0);
		synopGMAC_disable_dma_rx(gmacdev0);
		synopGMAC_rx_disable(gmacdev0);
	}   

	if(ifp1 != 0){ 
		adapter1 = ifp1->if_softc;
		gmacdev1 = (synopGMACdevice *)adapter1->synopGMACdev;

		synopGMAC_disable_dma_tx(gmacdev1);
		synopGMAC_tx_disable(gmacdev1);
		synopGMAC_disable_dma_rx(gmacdev1);
		synopGMAC_rx_disable(gmacdev1);
	}   
}

