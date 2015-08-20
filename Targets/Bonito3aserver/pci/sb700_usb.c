#include "sb700.h"
#include "rs780_cmn.h"


static void usb_init(device_t dev)
{
#ifndef USE_BMC
	u8 byte;
	u16 word;
	u32 dword;
	device_t sm_dev;
//lycheng do in tgt_devinit()
#if 1
	/* Enable OHCI0-4 and EHCI Controllers */
	printk_info("Enable OHCI0-4 and EHCI Controllers\n");
	//sm_dev = dev_find_slot(0, PCI_DEVFN(0x14, 0));
	sm_dev = _pci_make_tag(0, 0x14, 0);
	byte = pci_read_config8(sm_dev, 0x68);
	//byte |= 0x3F;
	byte |= 0xFF;
	pci_write_config8(sm_dev, 0x68, byte);
#endif

#if 0
#if 1
	/* RPR 5.2 Enables the USB PME Event,Enable USB resume support */
	byte = pm_ioread(0x61);
	byte |= 1 << 6;
	pm_iowrite(0x61, byte);
	byte = pm_ioread(0x65);
	byte |= 1 << 2;
	pm_iowrite(0x65, byte);

	/* RPR 5.3 Support USB device wakeup from the S4/S5 state */
	byte = pm_ioread(0x65);
	byte &= ~(1 << 0);
	pm_iowrite(0x65, byte);

	/* RPR 5.6 Enable the USB controller to get reset by any software that generate a PCIRst# condition */
	byte = pm_ioread(0x65);
	byte |= (1 << 4);
	pm_iowrite(0x65, byte);
#endif
	/* RPR 5.11 Disable OHCI MSI Capability */
	printk_info("Disable OHCI MSI Capability\n");
	word = pci_read_config16(dev, 0x40);
	//word |= (0x1F << 8);
	word |= (0x3 << 8);
	pci_write_config16(dev, 0x40, word);

	/* RPR 5.8 Disable the OHCI Dynamic Power Saving feature  */
	printk("Disable the OHCI Dynamic Power Saving feature\n");
	dword = pci_read_config32(dev, 0x50);
	//dword &= ~(1 << 16);
	dword |= (1 << 31);
	pci_write_config32(dev, 0x50, dword);
#if 0
	/* Set a default latency timer. */
        pci_write_config8(dev, 0x0d, 0x40); //PCI_LATENCY_TIMER

        byte = pci_read_config8(dev, 0x3d); //PCI_INTERRUPT_PIN
        if (byte) {
                pci_write_config8(dev, 0x3c, 0); //PCI_INTERRUPT_LINE
        }
        /* Set the cache line size, so far 64 bytes is good for everyone. */
        pci_write_config8(dev, 0x0c, 64 >> 2); //PCI_CACHE_LINE_SIZE
        /*enable io/memory space*/
	pci_write_config8(dev, 0x04, (1<<0|1<<1|1<<2)); 
#endif
#endif
#endif
}

static void usb_init2(device_t dev)
{
#ifndef USE_BMC
	u8 byte;
	u16 word;
	u32 dword;
	u8 *usb2_bar0_pci;
	u8 *usb2_bar0_addr;


#if 1 //because of disable usb, lycheng
	usb2_bar0_pci = (u8 *) (pci_read_config32(dev, 0x10) & ~0xFF);

	usb2_bar0_addr = (u8 *)((usb2_bar0_pci - 0x40000000) + 0xc0000000);
	printk_info("usb2_bar0_pci=%p\n", usb2_bar0_addr);

	/* RPR5.4 Enables the USB PHY auto calibration resister to match 45ohm resistence */
	dword = 0x00020F00;
	WRITEL(dword, usb2_bar0_addr + 0xC0);

	/* RPR5.5 Sets In/OUT FIFO threshold for best performance */
	//dword = 0x00200040;
	dword = 0x00400040;
	WRITEL(dword, usb2_bar0_addr + 0xA4);
#endif
       byte = pci_read_config8(dev, 0x50);
       /* RPR5.10 Disable EHCI MSI support */
       printk_info("Disable EHCI MSI support\n");
       byte |= (1 << 6);
       pci_write_config8(dev, 0x50, byte);

       /* EHCI Dynamic Clock gating feature */
       dword = READL(usb2_bar0_addr + 0xbc);
       dword &= ~(1 << 12);
       WRITEL(dword, usb2_bar0_addr + 0xbc);


	dword = pci_read_config32(dev, 0x50);
	dword |= (1 << 7);
	pci_write_config32(dev, 0x50, dword);
	
	/* RPR6.15 EHCI Async Park Mode */
	printk_info("EHCI Async Park Mode\n");
	dword = pci_read_config32(dev, 0x50);
	dword |= (1 << 23);
	pci_write_config32(dev, 0x50, dword);

       /* RPR6.11 Disabling EHCI Advance Asynchronous Enhancement */
       dword = pci_read_config32(dev, 0x50);
       dword |= (1 << 3);
       pci_write_config32(dev, 0x50, dword);
       dword = pci_read_config32(dev, 0x50);
       dword &= ~(1 << 28);
       pci_write_config32(dev, 0x50, dword);

       /* USB Periodic cache setting */
       dword = pci_read_config32(dev, 0x50);
       dword |= (1 << 8);
       pci_write_config32(dev, 0x50, dword);
       dword = pci_read_config32(dev, 0x50);
       dword &= ~(1 << 27);
       pci_write_config32(dev, 0x50, dword);

#if 1 //because of disable usb, lycheng
	/* RPR6.17 Disable the EHCI Dynamic Power Saving feature */
	word = READL(usb2_bar0_addr + 0xBC);
	word &= ~(1 << 12);
	WRITEW(word, usb2_bar0_addr + 0xBC);

	/* Set a default latency timer. */
        pci_write_config8(dev, 0x0d, 0x40); //PCI_LATENCY_TIMER

        byte = pci_read_config8(dev, 0x3d); //PCI_INTERRUPT_PIN
        if (byte) {
                pci_write_config8(dev, 0x3c, 0); //PCI_INTERRUPT_LINE
        }
        /* Set the cache line size, so far 64 bytes is good for everyone. */
        pci_write_config8(dev, 0x0c, 64 >> 2); //PCI_CACHE_LINE_SIZE
	 /*enable io/memory space*/
        pci_write_config8(dev, 0x04, (1<<0|1<<1|1<<2));
#endif
#endif
}

