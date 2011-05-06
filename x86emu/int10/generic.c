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

static void outseq(int index, unsigned char val)
{
	int v;
	v = ((int)val << 8) + index;
	linux_outw(v, 0x3c4);
}
static unsigned char inseq(unsigned char index)
{
	linux_outb(index, 0x3c4);
	return linux_inb(0x3c5);
}
static void outcrtc(int index, unsigned char val)
{
	int v;
	v = ((int)val << 8) + index;
	linux_outw(v, 0x3d4);
}
static unsigned char incrtc(int index)
{
	linux_outb(index, 0x3d4);
	return linux_inb(0x3d5);
}

static void outgra(int index, unsigned char val)
{
	linux_outb(index, 0x3ce);
	linux_outb(val, 0x3cf);
}
static unsigned char ingra(int index)
{
	linux_outb(index, 0x3ce);
	return linux_inb(0x3cf);
}
static void outatt(int index, unsigned char val)
{
	linux_inb(0x3da);
	vgadelay();
	linux_outb(index, 0x3c0);
	vgadelay();
	linux_outb(val, 0x3c0);
	vgadelay();
}
static unsigned char inatt(int index)
{
	linux_inb(0x3da);
	vgadelay();
	linux_outb(index, 0x3c0);
	vgadelay();
	return linux_inb(0x3c1);
}
static void setregs(const unsigned char *regs)
{
	int i;
	linux_outb(regs[MIS], 0x3c2);
	outseq(0x0, 0x1);
	outseq(0x01, regs[SEQ + 1] | 0x20);
	linux_outb(0x1, 0x3c4);
	linux_outb(regs[SEQ + 1] | 0x20, 0x3c5);
	for (i = 2; i < SEQ_C; i++) {
		outseq(i, regs[SEQ + i]);
	}
	outseq(0x0, 0x3);
	outcrtc(0x11, incrtc(0x11) & 0x7f);

	for (i = 0; i < CRT_C; i++) {
		outcrtc(i, regs[CRT + i]);
	}

	for (i = 0; i < GRA_C; i++) {
		outgra(i, regs[GRA + i]);
	}

	for (i = 0; i < ATT_C; i++) {
		outatt(i, regs[ATT + i]);
	}
	outseq(0x01, regs[SEQ + 1] & 0xDF);
	linux_inb(0x3da);
	vgadelay();
	linux_outb(0x20, 0x3c0);

	vgadelay();
	linux_outb(0x67, 0x3c2);
}

extern struct pci_device *vga_dev;

extern int vga_available;
extern int novga;
extern int vesa_mode;
extern struct vesamode *vesa_mode_head;
int vga_bios_init(void)
{
	xf86Int10InfoPtr pInt;
	int screen;
	void *base = 0;
	void *vbiosMem = 0;
	legacyVGARec vga;
	pInt = (xf86Int10InfoPtr) malloc(sizeof(xf86Int10InfoRec));
	memset(pInt, 0, sizeof(xf86Int10InfoRec));
	if (!xf86Int10ExecSetup(pInt))
		goto error0;
	pInt->mem = &genericMem;
	pInt->private = (pointer) malloc(sizeof(genericInt10Priv));
	memset(pInt->private, 0, sizeof(genericInt10Priv));
	pInt->scrnIndex = 0;	/* screen */
	base = INTPriv(pInt)->base = malloc(0x100000);
//#ifdef	RS690
#if     defined(RS690) || defined(RS780E)
	{
		pcitag_t vga_bridge = _pci_make_tag(0, 1, 0);
		unsigned int val;
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
	printf("memorysize=%x,base=%lx,sysMem=%lx,vram=%lx\n", memorysize,
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

		if (vga_dev != NULL) {
			pdev = vga_dev;
			printk
			    ("Found VGA device: vendor=0x%04x, device=0x%04x\n",
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
#if defined(RADEON7000) || defined(RS690) || defined(VESAFB)
		{
			extern unsigned char vgarom[];
			unsigned char *tmp;
			romaddress = (unsigned long)vgarom;
			tmp = (unsigned char *)vgarom;
			printk("Here after vgarom romaddress:%x\n",tmp[4]);
			printk(" vgarom romaddress:0x%x\n",romaddress);
		}
#endif

		if (romaddress == 0) {
			printk("No rom address assigned,skipped\n");
			return -1;
		}
#if defined(BONITOEL) && !( defined(RADEON7000) || defined(VESAFB) || defined(RS690)||defined(RS780E) )
		romaddress |= 0x10000000;
#endif

		printk("Rom base addr: %lx\n", romaddress);

		magic[0] = readb(romaddress);
		magic[1] = readb(romaddress + 1);

		if (magic[0] == 0x55 && magic[1] == 0xaa) {
			printk("VGA bios found\n");

			/* rom size is stored at offset 2,in 512 byte unit */
			romsize = (readb(romaddress + 2)) * 512;
			printk("rom size is %ldk\n", romsize / 1024);

			ppcidata = readw(romaddress + 0x18);
			printk("PCI data structure at offset %x\n", ppcidata);
			pcisig[0] = readb(romaddress + ppcidata);
			pcisig[1] = readb(romaddress + ppcidata + 1);
			pcisig[2] = readb(romaddress + ppcidata + 2);
			pcisig[3] = readb(romaddress + ppcidata + 3);
			if (pcisig[0] != 'P' || pcisig[1] != 'C' ||
			    pcisig[2] != 'I' || pcisig[3] != 'R') {
				printk("PCIR expected,read %c%c%c%c\n",
				       pcisig[0], pcisig[1], pcisig[2],
				       pcisig[3]);
				printk("Invalid pci signature found,give up\n");
				return -1;
			}

			codetype = readb(romaddress + ppcidata + 0x14);

			if (codetype != 0) {
				printk("Not x86 code in rom,give up\n");
				return -1;
			}

//#ifdef	RS690
#if   defined(RS690) || defined(RS780E)
			*((volatile unsigned int *)((romaddress + ppcidata + 4))) = _pci_conf_read(pdev->pa.pa_tag, 0x00);
#endif
		} else {
			printk("No valid bios found,magic=%x%x\n", magic[0],
			       magic[1]);
			return -1;
		}

		pInt->pdev = pdev;
        #if defined(LOONGSON3A_3AEV)||defined(LOONGSON2G_2G690E)||defined(LOONGSON3A_3A780E)||defined(LOONGSON3A_3AITX)
        memcpy(vbiosMem, (char *)(0x00000000 | romaddress),
		       V_BIOS_SIZE);
        #else
		memcpy(vbiosMem, (char *)(0xa0000000 | romaddress),
		       V_BIOS_SIZE);
        #endif
		if (PCI_VENDOR(pdev->pa.pa_id) == 0x1002
		    && PCI_PRODUCT(pdev->pa.pa_id) == 0x4750)
			MEM_WW(pInt, 0xc015e, 0x4750);
        #if defined(LOONGSON3A_3AEV)||defined(LOONGSON2G_2G690E)||defined(LOONGSON3A_3A780E)||defined(LOONGSON3A_3AITX)
        //for test
        MEM_WW(pInt , 0xcac90 , 0x1 | MEM_RW(pInt , 0xcac90));
        #else
        #endif
	}

	pInt->BIOSseg = V_BIOS >> 4;
	pInt->num = 0xe6;
	printf("lock vga\n");
	//LockLegacyVGA(screen, &vga);
	printf("starting bios emu...\n");
	M.x86.debug |= /*DEBUG_STEP_F | DEBUG_DECODE_F | DEBUG_TRACE_F | DEBUG_MEM_TRACE_F */ DEBUG_IO_TRACE_F | DEBUG_DECODE_F;
	//X86EMU_trace_on();
	printf("end of trace ......................................\n");
	printf("ax=%lx,bx=%lx,cx=%lx,dx=%lx\n", pInt->ax, pInt->bx, pInt->cx, pInt->dx);
	xf86ExecX86int10(pInt);
	printf("bios emu done\n");
#if !defined(LOONGSON3A_3AEV)&&!defined(LOONGSON2G_2G690E)&&!defined(LOONGSON3A_3A780E)&&!defined(LOONGSON3A_3AITX)
	pInt->num = 0x10;
	pInt->ax = 0x03;
	xf86ExecX86int10(pInt);
#endif

	//UnlockLegacyVGA(screen, &vga);
	setregs(regs);
#if !defined(LOONGSON3A_3AEV)&&!defined(LOONGSON2G_2G690E)&&!defined(LOONGSON3A_3A780E)&&!defined(LOONGSON3A_3AITX)
	linux_outb(0x67, 0x3c2);
#endif



#if NMOD_FRAMEBUFFER == 0
	printf("setting text mode...\n");
	//X86EMU_trace_on();
	pInt->BIOSseg = V_BIOS >> 4;
	pInt->num = 0x10;
	pInt->ax = 0x0003;
	xf86ExecX86int10(pInt);
#else
    #if 1
	printf("setting fb mode...\n");

	pInt->BIOSseg = V_BIOS >> 4;
	pInt->num = 0x10;
	{
		char *mode;
		mode = getenv("vesa_mode");
		if (mode != 0)
			vesa_mode = strtol(mode, 0, 0);
		else
			vesa_mode = 0x00;
	}
    /*
	for(vesa_mode = 0; vesa_mode <= 24; vesa_mode++){
		printk("\n\nvesa_mode : 0x%x\n", vesa_mode);
     */   
	pInt->ax = 0x4f02;
//	pInt->bx = 0x4114;
	pInt->bx = (USE_LINEAR_FRAMEBUFFER | vesa_mode_head[vesa_mode].mode);
	printk("ax %x bx %x\n", pInt->ax, pInt->bx);
	xf86ExecX86int10(pInt);
		if (pInt->ax != 0x004f){
			printk("set vesa mode failed,ax=%x mode(0x%x)\n", pInt->ax, pInt->bx);
		}else{
			//break;
		}
	//}
    #endif

//#ifdef DEBUG
#if 1
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
