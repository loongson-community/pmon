#include "sb700.h"
#include "rs780_cmn.h"

static void ide_init(device_t dev)
{
	/* Enable ide devices so the linux ide driver will work */
	u32 dword;
	u8 byte;

	/* RPR10.1 disable MSI */
	printk_info("disable MSI\n");
	dword = pci_read_config32(dev, 0x70);
	dword &= ~(1 << 16);
	pci_write_config32(dev, 0x70, dword);
#if 1
	/* Ultra DMA mode */
	/* enable UDMA */
	printk_info("enable  UDMA\n");
	byte = pci_read_config8(dev, 0x54);
	byte |= 1 << 0;
	pci_write_config8(dev, 0x54, byte);
#endif
	/* Enable I/O Access&& Bus Master */
	printk_info("enable I/O Access bus master");
	dword = pci_read_config16(dev, 0x4);
	//dword |= ((1 << 2)|(1 << 0));
	dword |= (1 << 2);
	pci_write_config16(dev, 0x4, dword);
#if 0
	/* Set a default latency timer. */
	pci_write_config8(dev, 0x0d, 0x40); //PCI_LATENCY_TIMER

	byte = pci_read_config8(dev, 0x3d); //PCI_INTERRUPT_PIN
	if (byte) {
		pci_write_config8(dev, 0x3c, 0); //PCI_INTERRUPT_LINE
	}
	/* Set the cache line size, so far 64 bytes is good for everyone. */
	pci_write_config8(dev, 0x0c, 64 >> 2); //PCI_CACHE_LINE_SIZE
#endif
//#if CONFIG_PCI_ROM_RUN == 1
//	pci_dev_init(dev);
//#endif

}

