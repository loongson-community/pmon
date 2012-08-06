#include "sb700.h"
#include "rs780_cmn.h"

static void pci_init(device_t dev)
{
	u32 dword;
	u16 word;
	u8 byte;

	/* RPR 5.1 Enables the PCI-bridge subtractive decode */
	/* This setting is strongly recommended since it supports some legacy PCI add-on cards,such as BIOS debug cards */
	printk_info("Enables the PCI-bridge subtractive decode\n");
	byte = pci_read_config8(dev, 0x4B);
	byte |= 1 << 7;
	pci_write_config8(dev, 0x4B, byte);
	byte = pci_read_config8(dev, 0x40);
	byte |= 1 << 5;
	pci_write_config8(dev, 0x40, byte);

#ifdef USE_780E_VGA
//vga pci card support   //lycheng
	/* PCI Command: Enable IO response */
	byte = pci_read_config8(dev, 0x04);
	byte |= (1 << 0)|(1 << 5)|(1 << 1)|(1 << 2);
	pci_write_config8(dev, 0x04, byte);
	/* PCI Command: VGA enable */
	byte = pci_read_config8(dev, 0x3e);
	byte |= (1 << 3);
	pci_write_config8(dev, 0x3e, byte);
//end vga pci card support
#endif

	/* RPR5.2 PCI-bridge upstream dual address window */
	/* this setting is applicable if the system memory is more than 4GB,and the PCI device can support dual address access */
	printk_info("PCI-bridge upstream dual address window\n");
	byte = pci_read_config8(dev, 0x50);
	//byte |= 1 << 0;
	byte &= ~(1 << 0);
	pci_write_config8(dev, 0x50, byte);

	/* RPR 5.3 PCI bus 64-byte DMA read access */
	/* Enhance the PCI bus DMA performance */
	printk_info("PCI bus 64-byte DMA read access\n");
	byte = pci_read_config8(dev, 0x4B);
	byte |= 1 << 4;
	pci_write_config8(dev, 0x4B, byte);

	/* RPR 5.4 Enables the PCIB writes to be cacheline aligned. */
	/* The size of the writes will be set in the Cacheline Register */
	printk_info("Enables the PCIB writes to be cacheline aligned.\n");
	byte = pci_read_config8(dev, 0x40);
	byte |= 1 << 1;
	pci_write_config8(dev, 0x40, byte);

	/* RPR 5.5 Enables the PCIB to retain ownership of the bus on the Primary side and on the Secondary side when GNT# is deasserted */
	printk_info("Enables the PCIB to retain ownership of the bus\n");
	pci_write_config8(dev, 0x0D, 0x40);
	pci_write_config8(dev, 0x1B, 0x40);

	/* RPR 5.6 Enable the command matching checking function on "Memory Read" & "Memory Read Line" commands */
	printk_info("Enable the command matching checking function\n");
	byte = pci_read_config8(dev, 0x4B);
	byte |= 1 << 6;
	pci_write_config8(dev, 0x4B, byte);

	/* RPR 5.7 When enabled, the PCI arbiter checks for the Bus Idle before asserting GNT# */
	printk_info("the PCI arbiter checks for the Bus Idle before asserting GNT#\n");
	byte = pci_read_config8(dev, 0x4B);
	byte |= 1 << 0;
	pci_write_config8(dev, 0x4B, byte);

	/* RPR 5.8 Adjusts the GNT# de-assertion time */
	printk_info("Adjusts the GNT# de-assertion time\n");
	word = pci_read_config16(dev, 0x64);
	word |= 1 << 12;
	pci_write_config16(dev, 0x64, word);

	/* RPR 5.9 Fast Back to Back transactions support */
	printk_info("Fast Back to Back transactions support\n");
	byte = pci_read_config8(dev, 0x48);
	byte |= 1 << 2;
	/* pci_write_config8(dev, 0x48, byte); */

	/* RPR 5.10 Enable Lock Operation */
	/* byte = pci_read_config8(dev, 0x48); */
	byte |= 1 << 3;
	pci_write_config8(dev, 0x48, byte);

	/* RPR 5.11 Enable additional optional PCI clock */
	printk_info("Enable additional optional PCI clock\n");
	word = pci_read_config16(dev, 0x64);
	word |= 1 << 8;
	pci_write_config16(dev, 0x64, word);

	/* RPR 5.12 Enable One-Prefetch-Channel Mode */
	printk_info("Enable One-Prefetch-Channel Mode\n");
	dword = pci_read_config32(dev, 0x64);
	dword |= 1 << 20;
	pci_write_config32(dev, 0x64, dword);

	/* RPR 5.13 Disable PCIB MSI Capability */
	printk_info("Disable PCIB MSI Capability\n");
	byte = pci_read_config8(dev, 0x40);
	byte &= ~(1 << 3);
	pci_write_config8(dev, 0x40, byte);

	/* rpr5.14 Adjusting CLKRUN# */
	printk_info("Adjusting CLKRUN\n");
	dword = pci_read_config32(dev, 0x64);
	dword |= (1 << 15);
	pci_write_config32(dev, 0x64, dword);
}


