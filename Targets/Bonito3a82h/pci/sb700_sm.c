#include "sb700.h"
#include "rs780_cmn.h"

#include "sb700_smbus.c"

/*
* SB600 enables all USB controllers by default in SMBUS Control.
* SB600 enables SATA by default in SMBUS Control.
*/
static void sm_init(device_t dev)
{
	u8 byte;
	u8 byte_old;
	u32 dword;
	u32 ioapic_base;
	u32 on;
	u32 nmi_option;

	printk_info("sm_init().\n");

	//ioapic_base = pci_read_config32(dev, 0x74) & (0xffffffe0);	/* some like mem resource, but does not have  enable bit */
	//setup_ioapic(ioapic_base);

	dword = pci_read_config8(dev, 0x62);
	dword |= 1 << 2;
	pci_write_config8(dev, 0x62, dword);

	printk_info("enable 0xCD6 0xCD7\n");
	dword = pci_read_config32(dev, 0x78);
	dword |= 1 << 9;
	pci_write_config32(dev, 0x78, dword);	/* enable 0xCD6 0xCD7 */
#if 0
	//add by lycheng
	printk_info("clear sata and ide controller into combined mode\n");
	printk_info("PATA is primary\n");
	dword = pci_read_config32(dev, 0xAD);
	dword |= (1 << 4);
	dword &= ~(1 << 3);
	pci_write_config32(dev, 0xAD, dword);	
#endif	
	/* bit 10: MultiMediaTimerIrqEn */
	printk_info("MultiMediaTimerIrqEn\n"); 
	dword = pci_read_config8(dev, 0x64);
	dword |= 1 << 10;
	pci_write_config8(dev, 0x64, dword);
	
	/* enable serial irq */
	printk_info("enable serial irq\n");
	byte = pci_read_config8(dev, 0x69);
	byte |= 1 << 7;		/* enable serial irq function */
	byte &= ~(0xF << 2);
	byte |= 4 << 2;		/* set NumSerIrqBits=4 */
	pci_write_config8(dev, 0x69, byte);
#if 1
	printk_info("Set to enable NB/SB handshake during IOAPIC interrupt for AMD K8/K7\n");
	byte = pm_ioread(0x61);
	byte |= 1 << 1;		/* Set to enable NB/SB handshake during IOAPIC interrupt for AMD K8/K7 */
	pm_iowrite(0x61, byte);

	/* disable SMI */
	printk_info("disable SMI\n");
	byte = pm_ioread(0x53);
	byte |= 1 << 3;
	pm_iowrite(0x53, byte);

	/* power after power fail */
	//on = MAINBOARD_POWER_ON_AFTER_POWER_FAIL;
	//get_option(&on, "power_on_after_fail");
	printk_info("power after power fail\n");
	on = 0;
	byte = pm_ioread(0x74);
	byte &= ~0x03;
	if (on) {
		byte |= 2;
	}
	byte |= 1 << 2;
	pm_iowrite(0x74, byte);
	printk_info("set power %s after power fail\n", on ? "on" : "off");
#endif
#if 1
	printk_info("1\n");
	byte = pm_ioread(0x68);
	byte &= ~(1 << 1);
	/* 2.6 */
	byte |= 1 << 2;
	pm_iowrite(0x68, byte);

	/* 2.6 */
	printk_info("2\n");
	byte = pm_ioread(0x65);
	byte &= ~(1 << 7);
	pm_iowrite(0x65, byte);
#endif
#if 1
	/* 2.16 */
	printk_info("2.16\n");
	byte = pm_ioread(0x55);
	byte |= 1 << 5;
	pm_iowrite(0x55, byte);
	byte = pm_ioread(0xD7);
	byte |= 1 << 6 | 1 << 1;;
	pm_iowrite(0xD7, byte);
	/* 2.15 */
	printk_info("2.15\n");
	byte = pm_ioread(0x42);
	byte &= ~(1 << 2);
	pm_iowrite(0x42, byte);
#if 0
	/* Set up NMI on errors */
	printk_info("set up NMI on errors\n");
	byte = INB(0xba000070);	/* RTC70 */
	byte_old = byte;
	//nmi_option = NMI_OFF;
	nmi_option = 0;
	//get_option(&nmi_option, "nmi");
	if (nmi_option) {
		byte &= ~(1 << 7);	/* set NMI */
		printk_info("++++++++++set NMI+++++\n");
	} else {
		byte |= (1 << 7);	/* Can not mask NMI from PCI-E and NMI_NOW */
		printk_info("++++++++++no set NMI+++++\n");
	}
	byte &= ~(1 << 7);
	if (byte != byte_old) {
		OUTB(byte, 0xba000070);
	}
#endif
#endif
	/* 2.10 IO Trap Settings */
	//try move later
	printk_info("IO Trap Setting\n");
	abcfg_reg(0x10090, 1 << 16, 1 << 16);

	/* ab index */
	printk_info("ab index\n");
	//pci_write_config32(dev, 0xF0, AB_INDX);
	pci_write_config32(dev, 0xF0, 0x00000cd8);
	/* Initialize the real time clock */
	//rtc_init(0);


	/* 4.3 Enabling Upstream DMA Access */
	printk_info("Enabling Upstream DMA Access\n");
	axcfg_reg(0x04, 1 << 2, 1 << 2); //780

	/*3.4 Enabling IDE/PCIB Prefetch for Performance Enhancement */
	printk_info("Enabling IDE/PCIB Prefetch for Performance Enhancement\n");
	abcfg_reg(0x10060, 9 << 17, 9 << 17);
	abcfg_reg(0x10064, 9 << 17, 9 << 17);

	/* 3.5 Enabling OHCI Prefetch for Performance Enhancement */
	printk_info("Enabling OHCI Prefetch for Performance Enhancement\n");
	abcfg_reg(0x80, 1 << 0, 1<< 0);

	/* 3.6 B-Link Client's Credit Variable Settings for the Downstream Arbitration Equation */
	/* 3.7 Enabling Additional Address Bits Checking in Downstream */
	//abcfg_reg(0x9c, 3 << 0, 3 << 0);
	printk_info("Enabling Additional Address Bits Checking in Downstream\n");
	abcfg_reg(0x9c, 3 << 0 | 1 << 8, 3 << 0 | 1 << 8);

	/* 3.8 Set B-Link Prefetch Mode */
	printk_info("Set B-Link Prefetch Mode\n");
	abcfg_reg(0x80, 3 << 1, 3 << 1);


	/* 3.9 Enabling Detection of Upstream Interrupts */
	printk_info("Enabling Detection of Upstream Interrupts\n");
	abcfg_reg(0x94, 1 << 20 | 0x7FFFF,1 << 20 | 0x00FEE);

	/* 3.10: Enabling Downstream Posted Transactions to Pass Non-Posted
	 *  Transactions for the K8 Platform (for All Revisions) */
	printk_info("Enabling Downstream Posted Transactions to Pass Non-Posted\n");
	//try
	abcfg_reg(0x10090, 1 << 8, 1 << 8);

	/* 3.11:Programming Cycle Delay for AB and BIF Clock Gating */
	/* 3.12: Enabling AB and BIF Clock Gating */
	abcfg_reg(0x10054, 0xFFFF0000, 0x1040000);
	abcfg_reg(0x54, 0xFF << 16, 4 << 16);
	printk_info("3.11, ABCFG:0x54\n");
	abcfg_reg(0x54, 1 << 24, 1 << 24);
	printk_info("3.12, ABCFG:0x54\n");
	abcfg_reg(0x98, 0x0000FF00, 0x00004700);
#if 0
	//lycheng
	abcfg_reg(0x10056, 0xFF << 0, 4 << 0);
	printk_info("3.11, ABCFG:0x10056\n");
	abcfg_reg(0x10056, 1 << 8, 1 << 8);
	printk_info("Set A-Link Prefetch Mode\n");
	abcfg_reg(0x1006c, 3 << 1, 3 << 1);
	axindxc_reg(0x10, 9 << 1, 9 << 1);
	axindxc_reg(0x21, 1 << 0, 1 << 0);
	abcfg_reg(0x10050, 1 << 2, 1 << 2);
	//end
#endif	
	/* 4.13:Enabling AB Int_Arbiter Enhancement (for All Revisions) */
	printk_info("Enabling AB Int_Arbiter Enhancement\n");
	abcfg_reg(0x10054, 0x0000FFFF, 0x07FF);
#if 1
	/* 4.14:Enabling Requester ID for upstream traffic. */
	printk_info("Enabling Requester ID for upstream traffic\n");
	abcfg_reg(0x98, 1 << 16, 0 << 16);
#endif

#if 1
	/* 9.2: Enableing IDE Data Bus DD7 Pull Down Resistor */
	printk_info("Enableing IDE Data Bus DD7 Pull Down Resistor\n");
	byte = pm2_ioread(0xE5);
	byte |= 1 << 2;
	pm2_iowrite(0xE5, byte);
	/* Enable IDE controller. */
	printk_info("Enable IDE controller.\n");
	byte = pm_ioread(0x59);
	byte &= ~(1 << 1);
	pm_iowrite(0x59, byte);
#endif

#if 1
	/* Enable NbSb virtual channel */
	printk_info("Enable NbSb virtual channel\n");
	axcfg_reg(0x114, 0x3f << 1, 0 << 1);
	axcfg_reg(0x120, 0x7f << 1, 0x7f << 1);
	axcfg_reg(0x120, 7 << 24, 1 << 24);
	axcfg_reg(0x120, 1 << 31, 1 << 31);
	abcfg_reg(0x50, 1 << 3, 1 << 3);
#endif

	//lycheng  for debug 8259 interrupt
#if 0
	printk_info("Shadow PIC register enable\n");
        dword = pci_read_config32(dev, 0x48);
        dword |= 1 << 23;
        pci_write_config32(dev, 0x48, dword);
#endif
	printk_info("K8 INTR enable\n");
        dword = pci_read_config32(dev, 0x60);
        dword |= 1 << 19;
        pci_write_config32(dev, 0x60, dword);   

	printk_info("Features enable\n");
        dword = pci_read_config32(dev, 0x64);
        dword |= ((1 << 0));
	dword &= ~((1 << 3)|(1 << 7));
        pci_write_config32(dev, 0x64, dword);   

	printk_info("INTAFix\n");
        dword = pci_read_config32(dev, 0xe0);
        dword |= 1 << 11;
        pci_write_config32(dev, 0xe0, dword);   
	//end lycheng
	
	printk_info("sm_init() end\n");
}

