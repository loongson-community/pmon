#include "sb700.h"
#include "rs780_cmn.h"

extern struct southbridge_ati_sb700_config conf_info;

static void sata_init(device_t dev)
{
	u8 byte;
	u16 word;
	u32 dword;
	u8 *sata_bar5;
	u16 sata_bar0, sata_bar1, sata_bar2, sata_bar3, sata_bar4;
	int i, j;
	u8 rev_id;

	struct southbridge_ati_sb700_config *conf = &conf_info;

	device_t sm_dev;
	/* SATA SMBus Disable */
	/* sm_dev = pci_locate_device(PCI_ID(0x1002, 0x4385), 0); */
	//sm_dev = dev_find_slot(0, PCI_DEVFN(0x14, 0));
	sm_dev = _pci_make_tag(0, 0x14, 0);

	/* Disable SATA SMBUS */
	printk_info("Disable SATA SMBUS\n");
	byte = pci_read_config8(sm_dev, 0xad);
	byte |= (1 << 1);
	pci_write_config8(sm_dev, 0xad, byte);

	/* Enable SATA and power saving */
	printk_info("Enable SATA and power saving\n");
	byte = pci_read_config8(sm_dev, 0xad);
	byte |= (1 << 0);
	byte |= (1 << 5);
	pci_write_config8(sm_dev, 0xad, byte);
	/* Set the interrupt Mapping to INTG# */
	printk_info("Set the interrupt Mapping to INTG#\n");
	byte = pci_read_config8(sm_dev, 0xaf);
	byte = 0x6 << 2;
	pci_write_config8(sm_dev, 0xaf, byte);

	//add for sb700
	/* get rev_id */
	rev_id = pci_read_config8(sm_dev, 0x08) - 0x28;

	/* get base addresss */
	printk_info("get base address\n");
	sata_bar5 = (u8 *) (pci_read_config32(dev, 0x24) & ~0x3FF);
	sata_bar0 = pci_read_config16(dev, 0x10) & ~0x7;
	sata_bar1 = pci_read_config16(dev, 0x14) & ~0x7;
	sata_bar2 = pci_read_config16(dev, 0x18) & ~0x7;
	sata_bar3 = pci_read_config16(dev, 0x1C) & ~0x7;
	sata_bar4 = pci_read_config16(dev, 0x20) & ~0x7;

	printk_spew("sata_bar0=%x\n", sata_bar0);	/* 3030 */
	printk_spew("sata_bar1=%x\n", sata_bar1);	/* 3070 */
	printk_spew("sata_bar2=%x\n", sata_bar2);	/* 3040 */
	printk_spew("sata_bar3=%x\n", sata_bar3);	/* 3080 */
	printk_spew("sata_bar4=%x\n", sata_bar4);	/* 3000 */
	printk_spew("sata_bar5=%p\n", sata_bar5);	/* e0309000 */

	//add for sb700
	/* disable combined mode to */
	printk_info("disable combined mode to\n");
	byte = pci_read_config8(sm_dev, 0xAD);
	byte &= ~(1 << 3);
	pci_write_config8(sm_dev, 0xAD, byte);

	/* Program the 2C to 0x43801002 */
	printk_info("Program the 2C to 0x43801002\n");
	dword = 0x43801002;
	pci_write_config32(dev, 0x2c, dword);

	/* SERR-Enable */
	printk_info("serr-enable\n");
	word = pci_read_config16(dev, 0x04);
	word |= (1 << 8);
	pci_write_config16(dev, 0x04, word);

	/* Dynamic power saving */
	printk_info("Dynamic power saving\n");
	byte = pci_read_config8(dev, 0x40);
	byte |= (1 << 2);
	pci_write_config8(dev, 0x40, byte);

	/* Set SATA Operation Mode, Set to IDE mode */
	printk("Set SATA operation mode, set to IDE mode\n");
	byte = pci_read_config8(dev, 0x40);
	byte |= (1 << 0);
	byte |= (1 << 4);
	pci_write_config8(dev, 0x40, byte);

	dword = 0x01018f00;
	pci_write_config32(dev, 0x8, dword);

	byte = pci_read_config8(dev, 0x40);
	byte &= ~(1 << 0);
	pci_write_config8(dev, 0x40, byte);

	/* Enable the SATA watchdog counter */
	printk_info("Enable the SATA watchdog counter\n");
	byte = pci_read_config8(dev, 0x44);
	byte |= (1 << 0);
	pci_write_config8(dev, 0x44, byte);

	//add for sb700
	/* Set bit 29 and 24 for A12 */
	dword = pci_read_config32(dev, 0x40);
	if (rev_id < 0x14)	/* before A12 */
		dword |= (1 << 29);
	else
		dword &= ~(1 << 29); /* A14 and above */
	pci_write_config32(dev, 0x40, dword);
	/* set bit 21 for A12 */
	dword = pci_read_config32(dev, 0x48);
	if (rev_id < 0x14)	/* before A12 */
		dword |= 1 << 24 | 1 << 21;
	else {
		dword &= ~(1 << 24 | 1 << 21); /* A14 and above */
		dword &= ~0xFF80; /* 15:7 */
		dword |= 1 << 15 | 0x7F << 7;
	}
	pci_write_config32(dev, 0x48, dword);

	
	/* Program the watchdog counter to 0x10 */
	printk_info("Program the watchdog counter to 0x10\n");
	byte = 0x10;
	pci_write_config8(dev, 0x46, byte);

	/* RPR6.5 Program the PHY Global Control to 0x2C00 for A13 */
	printk_info("Program the PHY Global Control to 0x2C00 for A13\n");
	word = 0x2c00;
	pci_write_config16(dev, 0x86, word);
	/* RPR7.6.2 SATA GENI PHY ports setting */
	printk_info("sata geni phy ports setting\n");
	pci_write_config32(dev, 0x88, 0x01B48017);
	pci_write_config32(dev, 0x8c, 0x01B48019);
	pci_write_config32(dev, 0x90, 0x01B48016);
	pci_write_config32(dev, 0x94, 0x01B48016);
	pci_write_config32(dev, 0x98, 0x01B48016);
	pci_write_config32(dev, 0x9C, 0x01B48016);

	/* RPR7.6.3 SATA GEN II PHY port setting for port [0~5]. */
	printk_info("sata gen II PHY port setting for port\n");
	pci_write_config16(dev, 0xA0, 0xA09A);
	pci_write_config16(dev, 0xA2, 0xA09F);
	pci_write_config16(dev, 0xA4, 0xA07A);
	pci_write_config16(dev, 0xA6, 0xA07A);
	pci_write_config16(dev, 0xA8, 0xA07A);
	pci_write_config16(dev, 0xAA, 0xA07A);

	/* Enable the I/O, MM, BusMaster access for SATA */
	printk_info("Enable the I/O, MM, BusMaster access for SATA\n");
	byte = pci_read_config8(dev, 0x4);
	byte |= 7 << 0;
	pci_write_config8(dev, 0x4, byte);
	/* RPR6.6 SATA drive detection. */
	/* Use BAR5+0x128,BAR0 for Primary Slave */
	/* Use BAR5+0x1A8,BAR0 for Primary Slave */
	/* Use BAR5+0x228,BAR2 for Secondary Master */
	/* Use BAR5+0x2A8,BAR2 for Secondary Slave */
#if 0
	for (i = 0; i < 4; i++) {
		byte = readb(sata_bar5 + 0x128 + 0x80 * i);
		printk_spew("SATA port %i status = %x\n", i, byte);
		byte &= 0xF;

		if( byte == 0x1 ) {
			/* If the drive status is 0x1 then we see it but we aren't talking to it. */
			/* Try to do something about it. */
			printk_spew("SATA device detected but not talking. Trying lower speed.\n");

			/* Read in Port-N Serial ATA Control Register */
			byte = readb(sata_bar5 + 0x12C + 0x80 * i);

			/* Set Reset Bit and 1.5g bit */
			byte |= 0x11; 
			writeb(byte, (sata_bar5 + 0x12C + 0x80 * i));
			
			/* Wait 1ms */			
			delay(1 * 1000); 

			/* Clear Reset Bit */
			byte &= ~0x01; 
			writeb(byte, (sata_bar5 + 0x12C + 0x80 * i));

			/* Wait 1ms */
			delay(1 * 1000);

			/* Reread status */
			byte = readb(sata_bar5 + 0x128 + 0x80 * i);
			printk_spew("SATA port %i status = %x\n", i, byte);
			byte &= 0xF;
		}

		if (byte == 0x3) {
			for (j = 0; j < 10; j++) {
				if (!sata_drive_detect(i, ((i / 2) == 0) ? sata_bar0 : sata_bar2))
					break;
			}
			printk_debug("%s %s device is %sready after %i tries\n",
					(i / 2) ? "Secondary" : "Primary",
					(i % 2 ) ? "Slave" : "Master",
					(j == 10) ? "not " : "",
					(j == 10) ? j : j + 1);
		} else {
			printk_debug("No %s %s SATA drive on Slot%i\n",
					(i / 2) ? "Secondary" : "Primary",
					(i % 2 ) ? "Slave" : "Master", i);
		}
	}
#endif

#if 0
	/* Below is CIM InitSataLateFar */
	/* Enable interrupts from the HBA  */
	printk_info("Enable interrupts from the HBA\n");
	byte = readb(sata_bar5 + 0x4);
	byte |= 1 << 1;
	writeb(byte, (sata_bar5 + 0x4));
	/* Clear error status */
	printk_info("Clear error status\n");
	writel(0xFFFFFFFF, (sata_bar5 + 0x130));
	writel(0xFFFFFFFF, (sata_bar5 + 0x1b0));
	writel(0xFFFFFFFF, (sata_bar5 + 0x230));
	writel(0xFFFFFFFF, (sata_bar5 + 0x2b0));
	writel(0xFFFFFFFF, (sata_bar5 + 0x330));
	writel(0xFFFFFFFF, (sata_bar5 + 0x3b0));
#endif

	/* Clear SATA status,Firstly we get the AcpiGpe0BlkAddr */
	/* ????? why CIM does not set the AcpiGpe0BlkAddr , but use it??? */

	/* word = 0x0000; */
	/* word = pm_ioread(0x28); */
	/* byte = pm_ioread(0x29); */
	/* word |= byte<<8; */
	/* printk_debug("AcpiGpe0Blk addr = %x\n", word); */
	/* writel(0x80000000 , word); */
}

