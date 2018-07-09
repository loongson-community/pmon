
#include <sys/queue.h>
#include <sys/param.h>
#include <sys/systm.h>
#include <sys/kernel.h>
#include <sys/malloc.h>
#include <sys/fcntl.h>
#include <sys/buf.h>
#include <sys/stat.h>
#include <sys/syslog.h>
#include <sys/device.h>

#define DEFAULT_LOOP_VARIABLE   10000
#define DEFAULT_DELAY_VARIABLE  10

#define GMAC0_MAC_REG_ADDR      ((*(volatile unsigned int *)(0xba000000 | (0 << 16) | (3 << 11) | (0 << 8) | 0x10)))
#define GMAC1_MAC_REG_ADDR      ((*(volatile unsigned int *)(0xba000000 | (0 << 16) | (3 << 11) | (1 << 8) | 0x10)))
#define Readl(Base) *(volatile unsigned int *)(Base)
enum GmacRegisters
{
	GmacConfig      	  = 0x0000,    /* Mac config Register                       */
	GmacFrameFilter  	  = 0x0004,    /* Mac frame filtering controls              */
	GmacHashHigh     	  = 0x0008,    /* Multi-cast hash table high                */
	GmacHashLow      	  = 0x000C,    /* Multi-cast hash table low                 */
	GmacGmiiAddr     	  = 0x0010,    /* GMII address Register(ext. Phy)           */
	GmacGmiiData     	  = 0x0014,    /* GMII data Register(ext. Phy)              */
	GmacFlowControl  	  = 0x0018,    /* Flow control Register                     */
	GmacVlan         	  = 0x001C,    /* VLAN tag Register (IEEE 802.1Q)           */

	GmacVersion     	  = 0x0020,    /* GMAC Core Version Register                */
	GmacWakeupAddr  	  = 0x0028,    /* GMAC wake-up frame filter adrress reg     */
	GmacPmtCtrlStatus  	  = 0x002C,    /* PMT control and status register           */

	GmacInterruptStatus	  = 0x0038,    /* Mac Interrupt ststus register	       */
	GmacInterruptMask     = 0x003C,    /* Mac Interrupt Mask register	       */

	GmacAddr0High    	  = 0x0040,    /* Mac address0 high Register                */
	GmacAddr0Low    	  = 0x0044,    /* Mac address0 low Register                 */
	GmacAddr1High    	  = 0x0048,    /* Mac address1 high Register                */
	GmacAddr1Low     	  = 0x004C,    /* Mac address1 low Register                 */
	GmacAddr2High   	  = 0x0050,    /* Mac address2 high Register                */
	GmacAddr2Low     	  = 0x0054,    /* Mac address2 low Register                 */
	GmacAddr3High    	  = 0x0058,    /* Mac address3 high Register                */
	GmacAddr3Low     	  = 0x005C,    /* Mac address3 low Register                 */
	GmacAddr4High    	  = 0x0060,    /* Mac address4 high Register                */
	GmacAddr4Low     	  = 0x0064,    /* Mac address4 low Register                 */
	GmacAddr5High    	  = 0x0068,    /* Mac address5 high Register                */
	GmacAddr5Low     	  = 0x006C,    /* Mac address5 low Register                 */
	GmacAddr6High    	  = 0x0070,    /* Mac address6 high Register                */
	GmacAddr6Low     	  = 0x0074,    /* Mac address6 low Register                 */
	GmacAddr7High    	  = 0x0078,    /* Mac address7 high Register                */
	GmacAddr7Low     	  = 0x007C,    /* Mac address7 low Register                 */
	GmacAddr8High    	  = 0x0080,    /* Mac address8 high Register                */
	GmacAddr8Low     	  = 0x0084,    /* Mac address8 low Register                 */
	GmacAddr9High    	  = 0x0088,    /* Mac address9 high Register                */
	GmacAddr9Low          = 0x008C,    /* Mac address9 low Register                 */
	GmacAddr10High        = 0x0090,    /* Mac address10 high Register               */
	GmacAddr10Low    	  = 0x0094,    /* Mac address10 low Register                */
	GmacAddr11High   	  = 0x0098,    /* Mac address11 high Register               */
	GmacAddr11Low    	  = 0x009C,    /* Mac address11 low Register                */
	GmacAddr12High   	  = 0x00A0,    /* Mac address12 high Register               */
	GmacAddr12Low         = 0x00A4,    /* Mac address12 low Register                */
	GmacAddr13High   	  = 0x00A8,    /* Mac address13 high Register               */
	GmacAddr13Low   	  = 0x00AC,    /* Mac address13 low Register                */
	GmacAddr14High   	  = 0x00B0,    /* Mac address14 high Register               */
	GmacAddr14Low   	  = 0x00B4,    /* Mac address14 low Register                */
	GmacAddr15High        = 0x00B8,    /* Mac address15 high Register               */
	GmacAddr15Low  	      = 0x00BC,    /* Mac address15 low Register                */

	/*Time Stamp Register Map*/
	GmacTSControl	          = 0x0700,  /* Controls the Timestamp update logic                         : only when IEEE 1588 time stamping is enabled in corekit            */

	GmacTSSubSecIncr     	  = 0x0704,  /* 8 bit value by which sub second register is incremented     : only when IEEE 1588 time stamping without external timestamp input */

	GmacTSHigh  	          = 0x0708,  /* 32 bit seconds(MS)                                          : only when IEEE 1588 time stamping without external timestamp input */
	GmacTSLow   	          = 0x070C,  /* 32 bit nano seconds(MS)                                     : only when IEEE 1588 time stamping without external timestamp input */

	GmacTSHighUpdate        = 0x0710,  /* 32 bit seconds(MS) to be written/added/subtracted           : only when IEEE 1588 time stamping without external timestamp input */
	GmacTSLowUpdate         = 0x0714,  /* 32 bit nano seconds(MS) to be writeen/added/subtracted      : only when IEEE 1588 time stamping without external timestamp input */

	GmacTSAddend            = 0x0718,  /* Used by Software to readjust the clock frequency linearly   : only when IEEE 1588 time stamping without external timestamp input */

	GmacTSTargetTimeHigh 	  = 0x071C,  /* 32 bit seconds(MS) to be compared with system time          : only when IEEE 1588 time stamping without external timestamp input */
	GmacTSTargetTimeLow     = 0x0720,  /* 32 bit nano seconds(MS) to be compared with system time     : only when IEEE 1588 time stamping without external timestamp input */

	GmacTSHighWord          = 0x0724,  /* Time Stamp Higher Word Register (Version 2 only); only lower 16 bits are valid                                                   */
	//GmacTSHighWordUpdate    = 0x072C,  /* Time Stamp Higher Word Update Register (Version 2 only); only lower 16 bits are valid                                            */

	GmacTSStatus            = 0x0728,  /* Time Stamp Status Register                                                                                                       */
};
/**********************************************************
 * GMAC DMA registers
 * For Pci based system address is BARx + GmaDmaBase
 * For any other system translation is done accordingly
 **********************************************************/


enum DmaRegisters
{
	DmaBusMode        = 0x0000,    /* CSR0 - Bus Mode Register                          */
	DmaTxPollDemand   = 0x0004,    /* CSR1 - Transmit Poll Demand Register              */
	DmaRxPollDemand   = 0x0008,    /* CSR2 - Receive Poll Demand Register               */
	DmaRxBaseAddr     = 0x000C,    /* CSR3 - Receive Descriptor list base address       */
	DmaTxBaseAddr     = 0x0010,    /* CSR4 - Transmit Descriptor list base address      */
	DmaStatus         = 0x0014,    /* CSR5 - Dma status Register                        */
	DmaControl        = 0x0018,    /* CSR6 - Dma Operation Mode Register                */
	DmaInterrupt      = 0x001C,    /* CSR7 - Interrupt enable                           */
	DmaMissedFr       = 0x0020,    /* CSR8 - Missed Frame & Buffer overflow Counter     */
	DmaTxCurrDesc     = 0x0048,    /*      - Current host Tx Desc Register              */
	DmaRxCurrDesc     = 0x004C,    /*      - Current host Rx Desc Register              */
	DmaTxCurrAddr     = 0x0050,    /* CSR20 - Current host transmit buffer address      */
	DmaRxCurrAddr     = 0x0054,    /* CSR21 - Current host receive buffer address       */
};


enum GmacGmiiAddrReg
{
	GmiiDevMask              = 0x0000F800,     /* (PA)GMII device address                 15:11     RW         0x00    */
	GmiiDevShift             = 11,

	GmiiRegMask              = 0x000007C0,     /* (GR)GMII register in selected Phy       10:6      RW         0x00    */
	GmiiRegShift             = 6,

	GmiiCsrClkMask           = 0x0000001C,     /*CSR Clock bit Mask			 4:2			     */
	GmiiCsrClk5              = 0x00000014,     /* (CR)CSR Clock Range     250-300 MHz      4:2      RW         000     */
	GmiiCsrClk4              = 0x00000010,     /*                         150-250 MHz                                  */
	GmiiCsrClk3              = 0x0000000C,     /*                         35-60 MHz                                    */
	GmiiCsrClk2              = 0x00000008,     /*                         20-35 MHz                                    */
	GmiiCsrClk1              = 0x00000004,     /*                         100-150 MHz                                  */
	GmiiCsrClk0              = 0x00000000,     /*                         60-100 MHz                                   */

	GmiiWrite                = 0x00000002,     /* (GW)Write to register                      1      RW                 */
	GmiiRead                 = 0x00000000,     /* Read from register                                            0      */

	GmiiBusy                 = 0x00000001,     /* (GB)GMII interface is busy                 0      RW          0      */
};
unsigned int gmac_read(unsigned long long base, unsigned int reg)
{
	unsigned long long addr;
	unsigned int data;

	addr = base + (unsigned long long)reg;
	data = Readl(addr);
	return data;
}

void gmac_write(unsigned long long base, unsigned int reg, unsigned int data)
{
	unsigned long long addr;

	addr = base + (unsigned long long)reg;
	Readl(addr) = data;
	return;
}

int gmac_phy_read(unsigned long long base,unsigned int PhyBase, unsigned int reg, unsigned short * data )
{
	unsigned int addr;
	unsigned int loop_variable;
	addr = ((PhyBase << GmiiDevShift) & GmiiDevMask) | ((reg << GmiiRegShift) & GmiiRegMask) | GmiiCsrClk3;
	addr = addr | GmiiBusy ;

	gmac_write(base,GmacGmiiAddr,addr);

	for(loop_variable = 0; loop_variable < DEFAULT_LOOP_VARIABLE; loop_variable++){
		if (!(gmac_read(base,GmacGmiiAddr) & GmiiBusy)){
			break;
		}
		int i = DEFAULT_DELAY_VARIABLE;
		while (i--);
	}
	if(loop_variable < DEFAULT_LOOP_VARIABLE)
		* data = (unsigned short)(gmac_read(base,GmacGmiiData) & 0xFFFF);
	else{
		printf( "\rError::: PHY not responding Busy bit didnot get cleared !!!!!!\n");
		return -2;
	}

	return 0;
}

int gmac_phy_write(unsigned long long base, unsigned int PhyBase, unsigned int reg, unsigned short data)
{
	unsigned int addr;
	unsigned int loop_variable;
	gmac_write(base,GmacGmiiData,data);

	addr = ((PhyBase << GmiiDevShift) & GmiiDevMask) | ((reg << GmiiRegShift) & GmiiRegMask) | GmiiWrite | GmiiCsrClk3;

	addr = addr | GmiiBusy ;
	gmac_write(base,GmacGmiiAddr,addr);
	for(loop_variable = 0; loop_variable < DEFAULT_LOOP_VARIABLE; loop_variable++){
		if (!(gmac_read(base,GmacGmiiAddr) & GmiiBusy)){
			break;
		}
		int i = DEFAULT_DELAY_VARIABLE;
		while (i--);
	}

	if(loop_variable < DEFAULT_LOOP_VARIABLE){
		return 0;
	}
	else{
		printf( "\rError::: PHY not responding Busy bit didnot get cleared !!!!!!\n");
	return -1;
	}
}
unsigned int synop_GMAC_set_mac_addr(unsigned long long mac_base, unsigned int MacHigh, unsigned int MacLow, unsigned char *MacAddr)
{
	unsigned int data;

	data = (MacAddr[5] << 8) | MacAddr[4];
	gmac_phy_write(mac_base,16,MacHigh,data);
	data = (MacAddr[3] << 24) | (MacAddr[2] << 16) | (MacAddr[1] << 8) | MacAddr[0] ;
	gmac_phy_write(mac_base,16,MacLow,data);

	return 0;
}
void gmac_mac_setting(unsigned short id,unsigned long long mac_base)
{
	unsigned char  mac_addr0[6] = {0};
	extern void ls7a_spi_read_mac(unsigned char * Inbuf,int num);
	extern unsigned char mac_read_spi_buf[22];

	ls7a_spi_read_mac(mac_read_spi_buf,id);
	if(id == 0)
		memcpy(mac_addr0,mac_read_spi_buf,6);
	else if(id == 1)
		memcpy(mac_addr0,mac_read_spi_buf + 16,6);

	synop_GMAC_set_mac_addr(mac_base,GmacAddr0High,GmacAddr0Low,mac_addr0);
}

void gmac_mac_init()
{
	unsigned short data;
	unsigned short id;
	unsigned long long mac_base;
	struct device *dev;
	unsigned int exists_syn0 = 0,exists_syn1 = 0;
	TAILQ_FOREACH(dev,&alldevs,dv_list){
		if ((strcmp(dev->dv_xname,"syn0") == 0))
			exists_syn0 = 1;
		if ((strcmp(dev->dv_xname,"syn1") == 0))
			exists_syn1 = 1;
	}
	if(exists_syn0 && exists_syn1)
		return;

	for (id = 0;id < 2;id++) {
		if (id == 0)
			mac_base = ((GMAC0_MAC_REG_ADDR & (~0xf)) | 0x80000000);
		else if (id == 1)
			mac_base = ((GMAC1_MAC_REG_ADDR & (~0xf)) | 0x80000000);

		gmac_phy_read(mac_base,16,0x14,&data);
		data = data | 0x82;
		gmac_phy_write(mac_base,16,0x14,data);
		gmac_phy_read(mac_base,16,0x00,&data);
		data = data | 0x8000;
		gmac_phy_write(mac_base,16,0x00,data);

		gmac_mac_setting(id,mac_base);

	}

}

