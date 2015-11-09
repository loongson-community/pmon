/* $XFree86: xc/programs/Xserver/hw/xfree86/int10/generic.c,v 1.21 2001/05/15 10:19:41 eich Exp $ */
/*
 *                   XFree86 int10 module
 *   execute BIOS int 10h calls in x86 real mode environment
 *                 Copyright 1999 Egbert Eich
 */
#define _INT10_PRIVATE
#include <stdio.h>
#include <stdlib.h>

#include <dev/pci/pcivar.h>
#include "xf86int10.h"
#include "xf86x86emu.h"
#include "linux/io.h"

#include "mod_framebuffer.h"
#include "vesa.h"

#define vgaram_base (VGA_BASE + 0xa0000)

#define ALLOC_ENTRIES(x) ((V_RAM / x) - 1)

static CARD8 read_b(xf86Int10InfoPtr pInt, int addr);
static CARD16 read_w(xf86Int10InfoPtr pInt, int addr);
static CARD32 read_l(xf86Int10InfoPtr pInt, int addr);
static void write_b(xf86Int10InfoPtr pInt, int addr, CARD8 val);
static void write_w(xf86Int10InfoPtr pInt, int addr, CARD16 val);
static void write_l(xf86Int10InfoPtr pInt, int addr, CARD32 val);

/*
 * the emulator cannot pass a pointer to the current xf86Int10InfoRec
 * to the memory access functions therefore store it here.
 */

typedef struct {
	int shift;
	int entries;
	void *base;
	void *vRam;
	void *sysMem;
	char *alloc;
} genericInt10Priv;

#define INTPriv(x) ((genericInt10Priv*)x->private)

int10MemRec genericMem = {
	read_b,
	read_w,
	read_l,
	write_b,
	write_w,
	write_l
};

static void *sysMem = NULL;
#define CRT_C   24		/* 24 CRT Controller Registers */
#define ATT_C   21		/* 21 Attribute Controller Registers */
#define GRA_C   9		/* 9  Graphics Controller Registers */
#define SEQ_C   5		/* 5  Sequencer Registers */
#define MIS_C   1		/* 1  Misc Output Register */

/* VGA registers saving indexes */
#define CRT     0		/* CRT Controller Registers start */
#define ATT     (CRT+CRT_C)	/* Attribute Controller Registers start */
#define GRA     (ATT+ATT_C)	/* Graphics Controller Registers start */
#define SEQ     (GRA+GRA_C)	/* Sequencer Registers */
#define MIS     (SEQ+SEQ_C)	/* General Registers */
#define EXT     (MIS+MIS_C)	/* SVGA Extended Registers */

static void vgadelay(void)
{
	int i;
	for (i = 0; i < 10; i++) ;
}

static unsigned char regs[60] = {
	0x5F, 0x4F, 0x50, 0x02, 0x55, 0x81, 0xBF, 0x1F,	/* CR00-CR18 */
	0x00, 0x4F, 0x0D, 0x0E, 0x0, 0x0, 0x0, 0x0,
	0x9C, 0x00, 0x8F, 0x28, 0x1F, 0x96, 0xB9, 0xA3,
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,	/* AR00-AR15 */
	0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
	0x0C, 0x00, 0x0F, 0x08, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x0E, 0x00,	/* GR00-GR05 */
	0xFF,
	0x03, 0x00, 0x03, 0x00, 0x02,	/* SR00-SR05 */
	0x67,			/* MISC_OUT  */
};

unsigned short ScreenLineLength;
unsigned short ScreenWidth;
unsigned short ScreenHeight;
unsigned short ScreenDepth;

static void outseq(int index, unsigned char val)
{
#ifdef PCIE_GRAPHIC_CARD
	pci_linux_outb(index, 0x3c4);
	pci_linux_outb(val, 0x3c5);
#else
	linux_outb(index, 0x3c4);
	linux_outb(val, 0x3c5);
#endif
}
static unsigned char inseq(unsigned char index)
{
#ifdef PCIE_GRAPHIC_CARD
	pci_linux_outb(index, 0x3c4);
	return pci_linux_inb(0x3c5);
#else
	linux_outb(index, 0x3c4);
	return linux_inb(0x3c5);
#endif
}

static void outcrtc(int index, unsigned char val)
{
#ifdef PCIE_GRAPHIC_CARD
	pci_linux_outb(index, 0x3d4);
	pci_linux_outb(val, 0x3d5);
#else
	linux_outb(index, 0x3d4);
	linux_outb(val, 0x3d5);
#endif
}

static unsigned char incrtc(int index)
{
#ifdef PCIE_GRAPHIC_CARD
	pci_linux_outb(index, 0x3d4);
	return pci_linux_inb(0x3d5);
#else
	linux_outb(index, 0x3d4);
	return linux_inb(0x3d5);
#endif
}

static void outgra(int index, unsigned char val)
{
#ifdef PCIE_GRAPHIC_CARD
	pci_linux_outb(index, 0x3ce);
	pci_linux_outb(val, 0x3cf);
#else
	linux_outb(index, 0x3ce);
	linux_outb(val, 0x3cf);
#endif
}

static unsigned char ingra(int index)
{
#ifdef PCIE_GRAPHIC_CARD
	pci_linux_outb(index, 0x3ce);
	return pci_linux_inb(0x3cf);
#else
	linux_outb(index, 0x3ce);
	return linux_inb(0x3cf);
#endif
}

static void outatt(int index, unsigned char val)
{
#ifdef PCIE_GRAPHIC_CARD
	pci_linux_inb(0x3da);
#else
	linux_inb(0x3da);
#endif
	vgadelay();
#ifdef PCIE_GRAPHIC_CARD
	pci_linux_outb(index, 0x3c0);
#else
	linux_outb(index, 0x3c0);
#endif
	vgadelay();
#ifdef PCIE_GRAPHIC_CARD
	pci_linux_outb(val, 0x3c0);
#else
	linux_outb(val, 0x3c0);
#endif
	vgadelay();
}

static unsigned char inatt(int index)
{
#ifdef PCIE_GRAPHIC_CARD
	pci_linux_inb(0x3da);
#else
	linux_inb(0x3da);
#endif
	vgadelay();
#ifdef PCIE_GRAPHIC_CARD
	pci_linux_outb(index, 0x3c0);
#else
	linux_outb(index, 0x3c0);
#endif
	vgadelay();
#ifdef PCIE_GRAPHIC_CARD
	return pci_linux_inb(0x3c1);
#else
	return linux_inb(0x3c1);
#endif
}

static void setregs(const unsigned char *regs)
{
	int i;
	unsigned char val;

	// misc	
#ifdef PCIE_GRAPHIC_CARD
	pci_linux_outb(regs[MIS], 0x3c2);
#else
	linux_outb(regs[MIS], 0x3c2);
#endif

	// seq
	outseq(0x0, 0x1);
	outseq(0x01, regs[SEQ + 1] | 0x20);
	outseq(0x01, regs[SEQ + 1] | 0x20);
	for (i = 2; i < SEQ_C; i++) {
		outseq(i, regs[SEQ + i]);
	}
	outseq(0x0, 0x3);

	// crtc 
	// write enable
	val = incrtc(0x11);
	val &= 0x7F;
	outcrtc(0x11, val);
	// crtc setting
	for (i = 0; i < CRT_C; i++) {
		outcrtc(i, regs[CRT + i]);
	}

	// gra
	for (i = 0; i < GRA_C; i++) {
		outgra(i, regs[GRA + i]);
	}

	// att
	for (i = 0; i < ATT_C; i++) {
		outatt(i, regs[ATT + i]);
	}
	outseq(0x01, regs[SEQ + 1] & 0xDF);
#ifdef PCIE_GRAPHIC_CARD
	pci_linux_inb(0x3da);
#else
	linux_inb(0x3da);
#endif
	vgadelay();
#ifdef PCIE_GRAPHIC_CARD
	pci_linux_outb(0x20, 0x3c0);
#else
	linux_outb(0x20, 0x3c0);
#endif

	// misc readback
	vgadelay();
#ifdef PCIE_GRAPHIC_CARD
	pci_linux_outb(0x67, 0x3c2);
#else
	linux_outb(0x67, 0x3c2);
#endif

}
extern struct pci_device *vga_dev,*pcie_dev;
extern int vga_available;
extern int novga;
extern int vesa_mode;
extern struct vesamode *vesa_mode_head;

extern unsigned long long uma_memory_size;
unsigned long long  vbios_addr;
//static unsigned long long * vfb_top;
void *vbiosMem = 0;

int cacluate_vesamode();

int vga_bios_init(void)
{
	xf86Int10InfoPtr pInt;
	int screen;
	void *base = 0;
	legacyVGARec vga;
	pcitag_t vga_bridge;
	unsigned int val;
	
	pInt = (xf86Int10InfoPtr) malloc(sizeof(xf86Int10InfoRec));
	//pInt = (xf86Int10InfoPtr) calloc(1,sizeof(xf86Int10InfoRec));
	memset(pInt, 0, sizeof(xf86Int10InfoRec));
	if (!xf86Int10ExecSetup(pInt))
		goto error0;
	pInt->mem = &genericMem;
	pInt->private = (pointer) malloc(sizeof(genericInt10Priv));
	//pInt->private = (pointer) calloc(1,sizeof(genericInt10Priv));
	memset(pInt->private, 0, sizeof(genericInt10Priv));
	pInt->scrnIndex = 0;	/* screen */
	base = INTPriv(pInt)->base = malloc(0x100000);
#if defined(LS7A) || defined(LOONGSON_2K)
	unsigned int b_io_lo_val, b_io_hi_val, d_val;
	unsigned int lpc_old_cmd;

	if(pcie_dev != NULL) {
		unsigned int vga_tmp = _pci_make_tag(pcie_dev->parent->pa.pa_bus, pcie_dev->parent->pa.pa_device, pcie_dev->parent->pa.pa_function);//get the brige data
		//printf("bridge tag: 0x%x\n", vga_tmp);
#if defined(LS7A)
        //disable LPC IO space
		unsigned int lpc_tag = _pci_make_tag(0, 23, 0);//get the brige data
        lpc_old_cmd = _pci_conf_read(lpc_tag, 0x4);
		_pci_conf_write(lpc_tag, 0x4, lpc_old_cmd & ~0x1);
#endif
		//set io limit and io base to zero.
		//store old value first
		b_io_lo_val = _pci_conf_read(vga_tmp, 0x1c);
		b_io_hi_val = _pci_conf_read(vga_tmp, 0x30);
		//write zero to io limit and io base
		_pci_conf_write(vga_tmp, 0x1c, 0x0);
		_pci_conf_write(vga_tmp, 0x30, 0x0);

		vga_tmp = _pci_make_tag(pcie_dev->pa.pa_bus, pcie_dev->pa.pa_device, pcie_dev->pa.pa_function);//get the device data
		//printf("device tag: 0x%x\n", vga_tmp);
		d_val = _pci_conf_read(vga_tmp, 0x20);
		_pci_conf_write(vga_tmp, 0x20, 0x800);
	}
#endif
#ifdef PCIE_GRAPHIC_CARD
	INTPriv(pInt)->base = base;
#endif
    //base = INTPriv(pInt)->base = 0x80000000+memorysize-0x100000;
#if defined(RS690) || defined(RS780E) || defined(PCIE_GRAPHIC_CARD)
	{
#ifdef PCIE_GRAPHIC_CARD
               if(pcie_dev != NULL)
                  vga_bridge = _pci_make_tag(1, 0, 0);
#else
		if(pcie_dev != NULL)
		   vga_bridge = _pci_make_tag(0, 2, 0);
		
		if(vga_dev != NULL)
		   vga_bridge = _pci_make_tag(0, 1, 0);
#if defined(USE_BMC)
           vga_bridge = _pci_make_tag(0, 6, 0); /* BMC VGA */
#endif

#endif
		/* enable VGA legacy space decode */
		val = _pci_conf_read(vga_bridge, 0x3c);
		val |= 1 << 19;
		_pci_conf_write(vga_bridge, 0x3c, val);
		/* reallocate the prefetchable address */
#if		0
		//_pci_conf_write(vga_bridge, 0x20, 0x16101600);
		//_pci_conf_write(vga_bridge, 0x24, 0x15f01400);
	printf("\n\nvga_bridge, 0x20=%x,0x24=%x\n\n\n", 
		_pci_conf_read(vga_bridge, 0x20),_pci_conf_read(vga_bridge, 0x24));
		_pci_conf_write(vga_bridge, 0x20, 0x12101200);
		_pci_conf_write(vga_bridge, 0x24, 0x11f01000);
#else
		/* using the old one */
#endif
	}
#endif
#ifdef LOONGSON_3A92W
#ifdef USE_BMC
	{
		pcitag_t vga_bridge;
		unsigned int val;
		int bmc_vga_busnum;
		{
			pcitag_t tag;
			unsigned int id_reg;
			unsigned short deviceid, vendorid;
			unsigned int bmc_vga_find;
			int i;
			for (i = 0; i < 10; i++) {
				tag = _pci_make_tag(i, 0, 0);
				id_reg = _pci_conf_read(tag, 0x00);
				vendorid = (id_reg) & 0xffff;
				deviceid = (id_reg >> 16) & 0xffff;
				if (vendorid == 0x1a03 && deviceid == 0x2000) {
					bmc_vga_find = 1;
					break;
				}
			}
			if (bmc_vga_find != 1)
				printf("cannot find a AST2400 VGA device\n");
			else
				bmc_vga_busnum = i;
		}

		vga_bridge = _pci_make_tag(0, 10, 0); //pcie dev slot
		/* enable VGA legacy space decode */
		val = _pci_conf_read(vga_bridge, 0x3c);
		val |= 1 << 19;
		_pci_conf_write(vga_bridge, 0x3c, val);
		vga_bridge = _pci_make_tag(5, 0, 0);
		/* enable VGA legacy space decode */
		val = _pci_conf_read(vga_bridge, 0x3c);
		val |= 1 << 19;
		_pci_conf_write(vga_bridge, 0x3c, val);
	}
#endif
#endif

	/*
	 * we need to map video RAM MMIO as some chipsets map mmio
	 * registers into this range.
	 */
	INTPriv(pInt)->vRam = (void *)vgaram_base;
	if (!sysMem) {
		sysMem = malloc(BIOS_SIZE);
		setup_system_bios(sysMem);
	}
	INTPriv(pInt)->sysMem = sysMem;
	printf("memorysize=%llx,base=%x,sysMem=%x,vram=%x\n", memorysize,
	       INTPriv(pInt)->base, sysMem, INTPriv(pInt)->vRam);
	setup_int_vect(pInt);
	set_return_trap(pInt);

	vbiosMem = (char *)base + V_BIOS;
	(void)memset(vbiosMem, 0, 2 * V_BIOS_SIZE);
	{
		struct pci_device *pdev;
		unsigned long romsize = 0;
		unsigned long romaddress = 0;
		unsigned char magic[2];
		unsigned short ppcidata;	/* pointer to pci data structure */
		unsigned char pcisig[4];	/* signature of pci data structure */
		unsigned char codetype;

		if (pcie_dev != NULL) {
			pdev = pcie_dev;
			printk
			    ("Found discrete graphics device: vendor=0x%04x, device=0x%04x\n",
			     PCI_VENDOR(pdev->pa.pa_id),
			     PCI_PRODUCT(pdev->pa.pa_id));
		}else if(vga_dev != NULL){
			pdev = vga_dev;
			printk("USE inter-graphic device: vendor:%04x, device=0x:%04x\n",
			     PCI_VENDOR(pdev->pa.pa_id),
			     PCI_PRODUCT(pdev->pa.pa_id));
		} else
			return -1;

		if (PCI_VENDOR(pdev->pa.pa_id) == 0x102b) {
			printk("skipping matrox cards\n");
			return -1;
		}
		if (PCI_VENDOR(pdev->pa.pa_id) == 0x1002
		    && PCI_PRODUCT(pdev->pa.pa_id) == 0x4750)
			MEM_WW(pInt, 0xc015e, 0x4750);
		romaddress = _pci_conf_read(pdev->pa.pa_tag, 0x30);
		romaddress &= (~1);
		/* enable rom address decode */
		_pci_conf_write(pdev->pa.pa_tag, 0x30, romaddress | 1);

		if(pcie_dev != NULL){
#ifdef PCIE_GRAPHIC_CARD
			romaddress = (romaddress | 0xc0000000) & 0xfffffff0;
#else
			romaddress = romaddress | 0x80000000 & 0xfffffff0;
#endif
		}
		if(vga_dev != NULL)
#if defined(RADEON7000) || defined(RS690) || defined(VESAFB) || defined(RS780E)
		{
			extern unsigned char vgarom[];
			unsigned char *tmp;
			romaddress = (unsigned long)vgarom;
			tmp = (unsigned char *)vgarom;
#ifdef LOONGSON_3A92W
#ifdef USE_BMC
			extern unsigned char vgarom_bmc[];
			romaddress = (unsigned long)vgarom_bmc;
			tmp = (unsigned char *)vgarom_bmc;
#endif
#endif
#if defined(BONITOEL) && !( defined(RADEON7000) || defined(VESAFB) || defined(RS690) || defined(RS780E))
		romaddress |= 0x10000000;
#endif
			printk("vgarom romaddress:0x%x\n",romaddress);
		}
#endif

		if (romaddress == 0) {
			printk("No rom address assigned,skipped\n");
			return -1;
		}


		printk("Rom base addr: %lx\n", romaddress);

#ifdef LS3_HT
		magic[0] = *(unsigned char *)(romaddress);
		magic[1] = *(unsigned char *)(romaddress + 1);
#else
		magic[0] = readb(romaddress);
		magic[1] = readb(romaddress + 1);
#endif

		if (magic[0] == 0x55 && magic[1] == 0xaa) {
			printk("VGA bios found\n");

			/* rom size is stored at offset 2,in 512 byte unit */
#ifdef LS3_HT
			romsize = (*(unsigned char *)(romaddress + 2)) * 512;
#else
			romsize = (readb(romaddress + 2)) * 512;
#endif
			printk("rom size is %ldk\n", romsize / 1024);

#ifdef LS3_HT
			ppcidata = *(unsigned int *)(romaddress + 0x18);
			printk("PCI data structure at offset %x\n", ppcidata);
			pcisig[0] = *(unsigned char *)(romaddress + ppcidata);
			pcisig[1] = *(unsigned char *)(romaddress + ppcidata + 1);
			pcisig[2] = *(unsigned char *)(romaddress + ppcidata + 2);
			pcisig[3] = *(unsigned char *)(romaddress + ppcidata + 3);
#else
			ppcidata = readw(romaddress + 0x18);
			printk("PCI data structure at offset %x\n", ppcidata);
			pcisig[0] = readb(romaddress + ppcidata);
			pcisig[1] = readb(romaddress + ppcidata + 1);
			pcisig[2] = readb(romaddress + ppcidata + 2);
			pcisig[3] = readb(romaddress + ppcidata + 3);
#endif
			if (pcisig[0] != 'P' || pcisig[1] != 'C' ||
			    pcisig[2] != 'I' || pcisig[3] != 'R') {
				printk("PCIR expected,read %c%c%c%c\n",
				       pcisig[0], pcisig[1], pcisig[2],
				       pcisig[3]);
				printk("Invalid pci signature found,give up\n");
				return -1;
			}

#ifdef LS3_HT
			codetype = *(unsigned char *)(romaddress + ppcidata + 0x14);
#else
			codetype = readb(romaddress + ppcidata + 0x14);
#endif

			if (codetype != 0) {
				printk("Not x86 code in rom,give up\n");
				return -1;
			}

#if   defined(RS690) || defined(RS780E) // fixup PCI ID
			//*((volatile unsigned int *)(0xa0000000 | (romaddress + ppcidata + 4))) = _pci_conf_read(pdev->pa.pa_tag, 0x00);
			*((volatile unsigned int *)((romaddress + ppcidata + 4))) = _pci_conf_read(pdev->pa.pa_tag, 0x00);
#endif
		} else {
			printk("No valid bios found,magic=%x%x\n", magic[0],
			       magic[1]);
			return -1;
		}

		pInt->pdev = pdev;
		memcpy(vbiosMem, (char *)(0x00000000 | romaddress),
		       V_BIOS_SIZE);
#if   defined(RS780E)

		/*************************************************************************************************************************
		 ********************************************  address map table as below  ***********************************************
		 *************************************************************************************************************************
		  uma_memory_base  vbiosMem    memory_total uma_memory_size gfx_mode  (virtual address	    <-->   physical address)
		   0x50000000     0x53f00000    2G			  64M			  UMA	  0x40000000~0x7fffffff <--> 0x80000000~0xbfffffff
		   0x50000000     0x57f00000    2G			  128M			  UMA	  0x40000000~0x7fffffff <--> 0x80000000~0xbfffffff
		   0x50000000     0x5ff00000    2G			  256M			  UMA	  0x40000000~0x7fffffff <--> 0x80000000~0xbfffffff
		   0x50000000     0x5ff00000    2G			  512M			  UMA	  0x40000000~0x7fffffff <--> 0x80000000~0xbfffffff
		   0x50000000     0x53f00000    4G			  64M			  UMA	  0x40000000~0x7fffffff <--> 0x80000000~0xbfffffff
		   0x50000000     0x57f00000    4G			  128M			  UMA	  0x40000000~0x7fffffff <--> 0x80000000~0xbfffffff
		   0x50000000     0x5ff00000    4G			  256M			  UMA	  0x40000000~0x7fffffff <--> 0x80000000~0xbfffffff
		   0x50000000     0x5ff00000    4G			  512M			  UMA	  0x40000000~0x7fffffff <--> 0x80000000~0xbfffffff

		   0xc0000000     0xc3f00000    2G			  64M			  SP	  0xc0000000~0xffffffff <--> 0x40000000~0x7fffffff
		   0xc0000000     0xc7f00000    2G			  128M			  SP	  0xc0000000~0xffffffff <--> 0x40000000~0x7fffffff
		   0xc0000000     0xcff00000    2G			  256M			  SP      0xc0000000~0xffffffff <--> 0x40000000~0x7fffffff
		   0xc0000000     0xdff00000    2G			  512M			  SP	  0xc0000000~0xffffffff <--> 0x40000000~0x7fffffff
		   0xc0000000     0xc3f00000    4G			  64M			  SP	  0xc0000000~0xffffffff <--> 0x40000000~0x7fffffff
		   0xc0000000     0xc7f00000    4G			  128M			  SP	  0xc0000000~0xffffffff <--> 0x40000000~0x7fffffff
		   0xc0000000     0xcff00000    4G			  256M			  SP	  0xc0000000~0xffffffff <--> 0x40000000~0x7fffffff
		   0xc0000000     0xdff00000    4G			  512M			  SP	  0xc0000000~0xffffffff <--> 0x40000000~0x7fffffff

		*************************************************************************************************************************
		*************************************************************************************************************************
		// Rules:
		 1. In GFX_UMA mode, always use share video frame buffer from second 256M system memory no matter how much system mem is,
			That is because only 32-bit address is available to do config accessing for internal gpu in rs780e bridge;
		 2. Max size of video frame buffer supported at this moment is only 512M, because usb/ifnet device need pci memory space 
		 3. If 1G video memory need to be suported, should map 0x00000000~0x7fffffff to 0x00000e0000000000~0x00000e007fffffff 
			both for UMA and SP mode and change above confgiruation.
		*************************************************************************************************************************/
	
#ifdef CONFIG_GFXUMA
		vbiosMem	= 0x50000000 + uma_memory_size  - 0x100000; 
#else
		vbiosMem	= (unsigned long) (0xc0000000 + uma_memory_size  - 0x100000 );
#endif
		if(vga_dev != NULL){
		  printf("video bios address: %08x\n",vbiosMem);
		  memcpy(vbiosMem, (char *)(0x00000000 | romaddress), V_BIOS_SIZE);
		}
#endif
		if (PCI_VENDOR(pdev->pa.pa_id) == 0x1002
		    && PCI_PRODUCT(pdev->pa.pa_id) == 0x4750)
			MEM_WW(pInt, 0xc015e, 0x4750);

#if   defined(RS780E)
        //for test
        MEM_WW(pInt , 0xcac90 , 0x1 | MEM_RW(pInt , 0xcac90));
#endif
	}

#if	0
	setregs(regs);
#else
	pInt->BIOSseg = V_BIOS >> 4;
	pInt->num = 0xe6;
	//printf("lock vga\n");
	//LockLegacyVGA(screen, &vga);
	printf("starting bios emu...\n");
	M.x86.debug |= DEBUG_STEP_F | DEBUG_DECODE_F | DEBUG_TRACE_F | DEBUG_MEM_TRACE_F | DEBUG_IO_TRACE_F | DEBUG_DECODE_F;
	//X86EMU_trace_on();
	//printf("end of trace ......................................\n");
	//printf("ax=%lx,bx=%lx,cx=%lx,dx=%lx\n", pInt->ax, pInt->bx, pInt->cx, pInt->dx);
	xf86ExecX86int10(pInt);
	printf("just before emu done ax(0x%x)\n", pInt->ax);
	printf("bios emu done\n");
	
#if 	0
	pInt->num = 0x10;
	pInt->ax = 0x03;
	xf86ExecX86int10(pInt);
#endif

	//UnlockLegacyVGA(screen, &vga);
	setregs(regs);
	//linux_outb(0x67, 0x3c2);

#if NMOD_FRAMEBUFFER == 0
	printf("setting text mode...\n");
	//X86EMU_trace_on();
	pInt->BIOSseg = V_BIOS >> 4;
	pInt->num = 0x10;
	pInt->ax = 0x0003;
	xf86ExecX86int10(pInt);
#else
	//printf("setting fb mode...\n");
	pInt->BIOSseg = V_BIOS >> 4;
	pInt->num = 0x10;
	{
		{
		char *mode;
		mode = getenv("vesa_mode");
		if (mode != 0)
			vesa_mode = strtol(mode, 0, 0);
		else
			vesa_mode = cacluate_vesamode(0);
		}
	}

	for(; vesa_mode>=0 ; vesa_mode = cacluate_vesamode(vesa_mode)){
		printk("\n\nvesa_mode : 0x%x\n", vesa_mode);
		pInt->ax = 0x4f02;
	//	pInt->bx = 0x4114;
		pInt->bx = (USE_LINEAR_FRAMEBUFFER | vesa_mode_head[vesa_mode].mode);
		printk("ax %x bx %x\n", pInt->ax, pInt->bx);
		xf86ExecX86int10(pInt);
		if (pInt->ax != 0x004f){
			printk("set vesa mode failed,ax=%x mode(0x%x)\n", pInt->ax, pInt->bx);
		}else{
			pInt->ax = 0x4f01;	/* get mode information */
			pInt->cx = (USE_LINEAR_FRAMEBUFFER | vesa_mode_head[vesa_mode].mode);
			pInt->di = 0;
			pInt->es = 0;
			xf86ExecX86int10(pInt);
			if (pInt->ax != 0x004f)
				printk("get vesa mode info failed,ax=%x\n", pInt->ax);
				
			ScreenLineLength = MEM_RW(pInt, pInt->di + 16);
			printk("linelength=%d\n", ScreenLineLength);
			ScreenWidth = MEM_RW(pInt, pInt->di + 18);
			printk("width=%d\n", ScreenWidth);
			ScreenHeight = MEM_RW(pInt, pInt->di + 20);
			printk("height=%d\n", ScreenHeight);
			ScreenDepth = MEM_RB(pInt, pInt->di + 25);
			printk("depth=%d\n", ScreenDepth);
			printk("pages=0x%x\n", MEM_RB(pInt, pInt->di + 29));
			printk("base=0x%x\n", MEM_RL(pInt, pInt->di + 40));

			break;
		}
	}

#ifdef DEBUG
	pInt->ax = 0x4f01;	/* get mode information */
	pInt->cx = 0x4114;
	pInt->di = 0;
	pInt->es = 0;
	xf86ExecX86int10(pInt);
	if (pInt->ax != 0x004f)
		printk("get vesa mode info failed,ax=%x\n", pInt->ax);

	printk("linelength=%x\n", MEM_RW(pInt, pInt->di + 16));
	printk("width=%x\n", MEM_RW(pInt, pInt->di + 18));
	printk("height=%x\n", MEM_RW(pInt, pInt->di + 20));
	printk("depth=%x\n", MEM_RB(pInt, pInt->di + 25));
	printk("pages=%x\n", MEM_RB(pInt, pInt->di + 29));
	printk("base=%x\n", MEM_RL(pInt, pInt->di + 40));
#endif
#endif

#endif

#if 0
	radeon_init_regbase();
	radeon_init_mode();
	radeon_engine_init();
	//radeon_dump_regs();
#endif

	free(pInt->private);
	free(pInt);
	free(sysMem);
	free(base);

#if defined(LS7A) || defined(LOONGSON_2K)
	if(pcie_dev != NULL) {
#if defined(LS7A)
		unsigned int lpc_tag = _pci_make_tag(0, 23, 0);//get the brige data
		_pci_conf_write(lpc_tag, 0x4, lpc_old_cmd);
#endif
		unsigned int vga_tmp = _pci_make_tag(pcie_dev->parent->pa.pa_bus, pcie_dev->parent->pa.pa_device, pcie_dev->parent->pa.pa_function);//get the brige data
		_pci_conf_write(vga_tmp, 0x1c, b_io_lo_val);
		_pci_conf_write(vga_tmp, 0x30, b_io_hi_val);

		vga_tmp = _pci_make_tag(pcie_dev->pa.pa_bus, pcie_dev->pa.pa_device, pcie_dev->pa.pa_function);//get the device data
		_pci_conf_write(vga_tmp, 0x20, d_val);
	}
#endif
	return 1;
      error0:
	free(pInt);

	return -1;
}

Bool MapCurrentInt10(xf86Int10InfoPtr pInt)
{
	/* nothing to do here */
	return TRUE;
}

#define MMIO_IN8(base, offset) \
        *(volatile CARD8 *)(((CARD8*)(base)) + (offset))
#define MMIO_IN16(base, offset) \
        *(volatile CARD16 *)(((CARD8*)(base)) + (offset))
#define MMIO_IN32(base, offset) \
        *(volatile CARD32 *)(((CARD8*)(base)) + (offset))
#define MMIO_OUT8(base, offset, val) \
        *(volatile CARD8 *)(((CARD8*)(base)) + (offset)) = (val)
#define MMIO_OUT16(base, offset, val) \
        *(volatile CARD16 *)(((CARD8*)(base)) + (offset)) = (val)
#define MMIO_OUT32(base, offset, val) \
        *(volatile CARD32 *)(((CARD8*)(base)) + (offset)) = (val)

#define OFF(addr) ((addr) & 0xffff)
#define SYS(addr) ((addr) >= SYS_BIOS)
#define V_ADDR(addr) \
	  (SYS(addr) ? ((char*)INTPriv(pInt)->sysMem) + (addr - SYS_BIOS) \
	   : ((char*)(INTPriv(pInt)->base) + addr))
#define VRAM_ADDR(addr) (addr - V_RAM)
#define VRAM_BASE (INTPriv(pInt)->vRam)

#define VRAM(addr) ((addr >= V_RAM) && (addr < (V_RAM + VRAM_SIZE)))
#define V_ADDR_RB(addr) \
	(VRAM((addr))) ? MMIO_IN8((CARD8*)VRAM_BASE,VRAM_ADDR((addr))) \
	   : *(CARD8*) V_ADDR((addr))
#define V_ADDR_RW(addr) \
	(VRAM((addr))) ? MMIO_IN16((CARD16*)VRAM_BASE,VRAM_ADDR((addr))) \
	   : (*(CARD16*)V_ADDR((addr)))
#define V_ADDR_RL(addr) \
	(VRAM((addr))) ? MMIO_IN32((CARD32*)VRAM_BASE,VRAM_ADDR((addr))) \
	   : (*(CARD32*)V_ADDR((addr)))

#define V_ADDR_WB(addr,val) \
	if(VRAM((addr))) {	\
	    MMIO_OUT8((CARD8*)VRAM_BASE,VRAM_ADDR((addr)),val); \
	} else \
	    *(CARD8*) V_ADDR((addr)) = val;
#define V_ADDR_WW(addr,val) \
	if(VRAM((addr))) \
	    MMIO_OUT16((CARD16*)VRAM_BASE,VRAM_ADDR((addr)),val); \
	else \
	    *(CARD16*)(V_ADDR((addr))) = val;

#define V_ADDR_WL(addr,val) \
	if (VRAM((addr))) \
	    MMIO_OUT32((CARD32*)VRAM_BASE,VRAM_ADDR((addr)),val); \
	else \
	    *(CARD32*)(V_ADDR((addr))) = val;

static CARD8 read_b(xf86Int10InfoPtr pInt, int addr)
{
	return V_ADDR_RB(addr);
}

static CARD16 read_w(xf86Int10InfoPtr pInt, int addr)
{
//	return V_ADDR_RW(addr);
#ifdef DEBUG_READ
	printf("read_w %x %x %x\n", V_ADDR_RB(addr), ((CARD16)(V_ADDR_RB((addr + 1))) << 8),
			(CARD16)(V_ADDR_RB(addr)) | ((CARD16)(V_ADDR_RB(addr + 1)) << 8));
#endif
	return (CARD16)(V_ADDR_RB(addr)) | ((CARD16)(V_ADDR_RB(addr + 1)) << 8);
}

static CARD32 read_l(xf86Int10InfoPtr pInt, int addr)
{
//	return V_ADDR_RL(addr);
	return (CARD32)(V_ADDR_RB(addr)) |
	    ((CARD32)(V_ADDR_RB(addr + 1)) << 8) |
	    ((CARD32)(V_ADDR_RB(addr + 2)) << 16) | ((CARD32)(V_ADDR_RB(addr + 3)) << 24);
}

static void write_b(xf86Int10InfoPtr pInt, int addr, CARD8 val)
{
	V_ADDR_WB(addr, (val & 0xff));
#ifdef DEBUG_WRITE
	if (VRAM(addr) && val)
		printf("%08x wb %x, rd %x\n", addr, val, read_b(pInt, addr));
#endif
}

static void write_w(xf86Int10InfoPtr pInt, int addr, CARD16 val)
{
	V_ADDR_WB(addr, (val & 0xff));
	V_ADDR_WB(addr+1, ((val >> 8) & 0xff));
#ifdef DEBUG_WRITE
	if (VRAM(addr) && val)
		printf("%08x ww %x, rd %x\n", addr, val, read_w(pInt, addr));
#endif
}

static void write_l(xf86Int10InfoPtr pInt, int addr, CARD32 val)
{
	V_ADDR_WB(addr, (val & 0xff));
	V_ADDR_WB(addr+1, ((val >> 8) & 0xff));
	V_ADDR_WB(addr+2, ((val >> 16) & 0xff));
	V_ADDR_WB(addr+3, ((val >> 24) & 0xff));
#ifdef DEBUG_WRITE
	if (VRAM(addr) && val)
		printf("%08x wl %x, rd %x\n", addr, val, read_l(pInt, addr));
#endif
}

pointer xf86int10Addr(xf86Int10InfoPtr pInt, CARD32 addr)
{
	return V_ADDR(addr);
}
