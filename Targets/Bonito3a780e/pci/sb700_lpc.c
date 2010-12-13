#include "sb700.h"
#include "rs780_cmn.h"


#define DMA1_RESET_REG		0x0D	/* Master Clear (w) */

#define DMA2_RESET_REG		0xDA	/* Master Clear (w) */
#define DMA2_MASK_REG		0xD4	/* single-channel mask (w) */
#define DMA2_MODE_REG		0xD6	/* mode register (w) */

#define DMA_MODE_CASCADE 0xC0   /* pass thru DREQ->HRQ, DACK<-HLDA only */

#define INB(addr) (*(volatile unsigned char *) (addr))
#define INW(addr) (*(volatile unsigned short *) (addr))
#define INL(addr) (*(volatile unsigned int *) (addr))

#define OUTB(b,addr) (*(volatile unsigned char *) (addr) = (b))
#define OUTW(b,addr) (*(volatile unsigned short *) (addr) = (b))
#define OUTL(b,addr) (*(volatile unsigned int *) (addr) = (b))


static void isa_dma_init(void)
{
	/* slave at 0x00 - 0x0f */
	/* master at 0xc0 - 0xdf */
	/* 0x80 - 0x8f DMA page registers */
	/* DMA: 0x00, 0x02, 0x4, 0x06 base address for DMA channel */
	OUTB(0, DMA1_RESET_REG);
	OUTB(0, DMA2_RESET_REG);
	OUTB(DMA_MODE_CASCADE, DMA2_MODE_REG);
	OUTB(0, DMA2_MASK_REG);
}


static void lpc_init(device_t dev)
{
	u8 byte;
	u32 dword;
	device_t sm_dev;

	/* Enable the LPC Controller */
	printk_info("Enable the LPC Controller\n");
	//sm_dev = dev_find_slot(0, PCI_DEVFN(0x14, 0));
	sm_dev = _pci_make_tag(0, 0x14, 0);
	dword = pci_read_config32(sm_dev, 0x64);
	dword |= 1 << 20;
	pci_write_config32(sm_dev, 0x64, dword);

	/* Initialize isa dma */
	//printk_info("Initialize isa dma\n");
	//isa_dma_init();

	/* RPR 7.2 Enable DMA transaction on the LPC bus */
	printk_info("Enable DMA transaction on the LPC bus\n");
	byte = pci_read_config8(dev, 0x40);
	byte |= (1 << 2);
	pci_write_config8(dev, 0x40, byte);

	/* RPR 7.3 Disable the timeout mechanism on LPC */
	printk_info("Disable the timeout mechanism on LPC\n");
	byte = pci_read_config8(dev, 0x48);
	byte &= ~(1 << 7);
	pci_write_config8(dev, 0x48, byte);

	/* RPR 7.5 Disable LPC MSI Capability */
	printk_info("Disable LPC MSI Capability\n");
	byte = pci_read_config8(dev, 0x78);
	byte &= ~(1 << 1);
	pci_write_config8(dev, 0x78, byte);
#if 0	
	//add by lycheng
	printk_info("IO/Mem Decoding\n");
        byte = pci_read_config8(dev, 0xbb);
        byte |= ((1<<3)|(1<<6)|(1<<7));
        pci_write_config8(dev, 0xbb, byte);
	
	/* Set a default latency timer. */
        pci_write_config8(dev, 0x0d, 0x40); //PCI_LATENCY_TIMER

        byte = pci_read_config8(dev, 0x3d); //PCI_INTERRUPT_PIN
        if (byte) {
                pci_write_config8(dev, 0x3c, 0); //PCI_INTERRUPT_LINE
        }
        /* Set the cache line size, so far 64 bytes is good for everyone. */
        pci_write_config8(dev, 0x0c, 64 >> 2); //PCI_CACHE_LINE_SIZE
#endif
}

