/*
 * Modifications to support Loongson Arch:
 * Copyright (c) 2008  Lemote.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice unmodified, this list of conditions, and the following
 *    disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */

/*
 * VESA framebuffer support. Author: Lj.Peng <penglj@lemote.com>
 */


#include <stdio.h>
#include <stdlib.h>

#include <dev/pci/pcivar.h>
#include "xf86int10.h"
#include "xf86x86emu.h"
#include "linux/io.h"

#include "mod_framebuffer.h"
#include "vesa.h"
#include <mod_sisfb.h>
//#include "bonito.h"

extern struct pci_device *vga_dev,*pcie_dev;

int vesa_mode = 1;

/*
 * The following macros are especially useful for __asm__
 * inline assembler.
 */
#ifndef __STR
#define __STR(x) #x
#endif
#ifndef STR
#define STR(x) __STR(x)
#endif

/*
 *  Configure language
 */
#ifdef __ASSEMBLY__
#define _ULCAST_
#else
#define _ULCAST_ (unsigned long)
#endif

/*
 * Coprocessor 0 register names
 */
#define CP0_INDEX $0
#define CP0_RANDOM $1
#define CP0_ENTRYLO0 $2
#define CP0_ENTRYLO1 $3
#define CP0_CONF $3
#define CP0_CONTEXT $4
#define CP0_PAGEMASK $5
#define CP0_WIRED $6
#define CP0_INFO $7
#define CP0_BADVADDR $8
#define CP0_COUNT $9
#define CP0_ENTRYHI $10
#define CP0_COMPARE $11
#define CP0_STATUS $12
#define CP0_CAUSE $13
#define CP0_EPC $14
#define CP0_PRID $15
#define CP0_CONFIG $16
#define CP0_LLADDR $17
#define CP0_WATCHLO $18
#define CP0_WATCHHI $19
#define CP0_XCONTEXT $20
#define CP0_FRAMEMASK $21
#define CP0_DIAGNOSTIC $22
#define CP0_DEBUG $23
#define CP0_DEPC $24
#define CP0_PERFORMANCE $25
#define CP0_ECC $26
#define CP0_CACHEERR $27
#define CP0_TAGLO $28
#define CP0_TAGHI $29
#define CP0_ERROREPC $30
#define CP0_DESAVE $31

/*
 * R4640/R4650 cp0 register names.  These registers are listed
 * here only for completeness; without MMU these CPUs are not useable
 * by Linux.  A future ELKS port might take make Linux run on them
 * though ...
 */
#define CP0_IBASE $0
#define CP0_IBOUND $1
#define CP0_DBASE $2
#define CP0_DBOUND $3
#define CP0_CALG $17
#define CP0_IWATCH $18
#define CP0_DWATCH $19

/*
 * Coprocessor 0 Set 1 register names
 */
#define CP0_S1_DERRADDR0  $26
#define CP0_S1_DERRADDR1  $27
#define CP0_S1_INTCONTROL $20

/*
 *  TX39 Series
 */
#define CP0_TX39_CACHE	$7

/*
 * Coprocessor 1 (FPU) register names
 */
#define CP1_REVISION   $0
#define CP1_STATUS     $31


#define PM_4K		0x00000000
#define PM_16K		0x00006000
#define PM_64K		0x0001e000
#define PM_256K		0x0007e000
#define PM_1M		0x001fe000
#define PM_4M		0x007fe000
#define PM_16M		0x01ffe000
#define PM_64M		0x07ffe000
#define PM_256M		0x1fffe000

/* Page size 64kb */
#define CONFIG_PAGE_SIZE_1M

/*
 * Default page size for a given kernel configuration
 */
#ifdef CONFIG_PAGE_SIZE_4KB
#define PM_DEFAULT_MASK	PM_4K
#elif defined(CONFIG_PAGE_SIZE_16KB)
#define PM_DEFAULT_MASK	PM_16K
#elif defined(CONFIG_PAGE_SIZE_64KB)
#define PM_DEFAULT_MASK	PM_64K
#elif defined(CONFIG_PAGE_SIZE_256KB)
#define PM_DEFAULT_MASK	PM_256K
#elif defined(CONFIG_PAGE_SIZE_1M)
#define PM_DEFAULT_MASK	PM_1M
#elif defined(CONFIG_PAGE_SIZE_4M)
#define PM_DEFAULT_MASK	PM_4M
#else
#error Bad page size configuration!
#endif


/*
 * Values used for computation of new tlb entries
 */
#define PL_4K		12
#define PL_16K		14
#define PL_64K		16
#define PL_256K		18
#define PL_1M		20
#define PL_4M		22
#define PL_16M		24
#define PL_64M		26
#define PL_256M		28

/*
 * Macros to access the system control coprocessor
 */

#define __read_32bit_c0_register(source, sel)				\
({ int __res;								\
	if (sel == 0)							\
		__asm__ __volatile__(					\
			"mfc0\t%0, " #source "\n\t"			\
			: "=r" (__res));				\
	else								\
		__asm__ __volatile__(					\
			".set\tmips32\n\t"				\
			"mfc0\t%0, " #source ", " #sel "\n\t"		\
			".set\tmips0\n\t"				\
			: "=r" (__res));				\
	__res;								\
})

#define __read_64bit_c0_register(source, sel)				\
({ unsigned long __res;							\
	if (sel == 0)							\
		__asm__ __volatile__(					\
			".set\tmips3\n\t"				\
			"dmfc0\t%0, " #source "\n\t"			\
			".set\tmips0"					\
			: "=r" (__res));				\
	else								\
		__asm__ __volatile__(					\
			".set\tmips64\n\t"				\
			"dmfc0\t%0, " #source ", " #sel "\n\t"		\
			".set\tmips0"					\
			: "=r" (__res));				\
	__res;								\
})

#define __write_32bit_c0_register(register, sel, value)			\
do {									\
	if (sel == 0)							\
		__asm__ __volatile__(					\
			"mtc0\t%z0, " #register "\n\t"			\
			: : "Jr" (value));				\
	else								\
		__asm__ __volatile__(					\
			".set\tmips32\n\t"				\
			"mtc0\t%z0, " #register ", " #sel "\n\t"	\
			".set\tmips0"					\
			: : "Jr" (value));				\
} while (0)

#define __write_64bit_c0_register(register, sel, value)			\
do {									\
	if (sel == 0)							\
		__asm__ __volatile__(					\
			".set\tmips3\n\t"				\
			"dmtc0\t%z0, " #register "\n\t"			\
			".set\tmips0"					\
			: : "Jr" (value));				\
	else								\
		__asm__ __volatile__(					\
			".set\tmips64\n\t"				\
			"dmtc0\t%z0, " #register ", " #sel "\n\t"	\
			".set\tmips0"					\
			: : "Jr" (value));				\
} while (0)

#define __read_ulong_c0_register(reg, sel)				\
	((sizeof(unsigned long) == 4) ?					\
	__read_32bit_c0_register(reg, sel) :				\
	__read_64bit_c0_register(reg, sel))

#define __write_ulong_c0_register(reg, sel, val)			\
do {									\
	if (sizeof(unsigned long) == 4)					\
		__write_32bit_c0_register(reg, sel, val);		\
	else								\
		__write_64bit_c0_register(reg, sel, val);		\
} while (0)

#define read_c0_index()		__read_32bit_c0_register($0, 0)
#define write_c0_index(val)	__write_32bit_c0_register($0, 0, val)

#define read_c0_entrylo0()	__read_ulong_c0_register($2, 0)
#define write_c0_entrylo0(val)	__write_ulong_c0_register($2, 0, val)

#define read_c0_entrylo1()	__read_ulong_c0_register($3, 0)
#define write_c0_entrylo1(val)	__write_ulong_c0_register($3, 0, val)

#define read_c0_conf()		__read_32bit_c0_register($3, 0)
#define write_c0_conf(val)	__write_32bit_c0_register($3, 0, val)

#define read_c0_context()	__read_ulong_c0_register($4, 0)
#define write_c0_context(val)	__write_ulong_c0_register($4, 0, val)

#define read_c0_pagemask()	__read_32bit_c0_register($5, 0)
#define write_c0_pagemask(val)	__write_32bit_c0_register($5, 0, val)

#define read_c0_wired()		__read_32bit_c0_register($6, 0)
#define write_c0_wired(val)	__write_32bit_c0_register($6, 0, val)

#define read_c0_info()		__read_32bit_c0_register($7, 0)

#define read_c0_cache()		__read_32bit_c0_register($7, 0)	/* TX39xx */
#define write_c0_cache(val)	__write_32bit_c0_register($7, 0, val)

#define read_c0_count()		__read_32bit_c0_register($9, 0)
#define write_c0_count(val)	__write_32bit_c0_register($9, 0, val)

#define read_c0_entryhi()	__read_ulong_c0_register($10, 0)
#define write_c0_entryhi(val)	__write_ulong_c0_register($10, 0, val)

#define read_c0_compare()	__read_32bit_c0_register($11, 0)
#define write_c0_compare(val)	__write_32bit_c0_register($11, 0, val)

#define read_c0_status()	__read_32bit_c0_register($12, 0)
#define write_c0_status(val)	__write_32bit_c0_register($12, 0, val)

#define read_c0_cause()		__read_32bit_c0_register($13, 0)
#define write_c0_cause(val)	__write_32bit_c0_register($13, 0, val)

#define read_c0_prid()		__read_32bit_c0_register($15, 0)

#define read_c0_config()	__read_32bit_c0_register($16, 0)
#define read_c0_config1()	__read_32bit_c0_register($16, 1)
#define read_c0_config2()	__read_32bit_c0_register($16, 2)
#define read_c0_config3()	__read_32bit_c0_register($16, 3)
#define write_c0_config(val)	__write_32bit_c0_register($16, 0, val)
#define write_c0_config1(val)	__write_32bit_c0_register($16, 1, val)
#define write_c0_config2(val)	__write_32bit_c0_register($16, 2, val)
#define write_c0_config3(val)	__write_32bit_c0_register($16, 3, val)

#define read_c0_framemask()	__read_32bit_c0_register($21, 0)
#define write_c0_framemask(val)	__write_32bit_c0_register($21, 0, val)

#define read_c0_debug()		__read_32bit_c0_register($23, 0)
#define write_c0_debug(val)	__write_32bit_c0_register($23, 0, val)

#define read_c0_depc()		__read_ulong_c0_register($24, 0)
#define write_c0_depc(val)	__write_ulong_c0_register($24, 0, val)

#define read_c0_ecc()		__read_32bit_c0_register($26, 0)
#define write_c0_ecc(val)	__write_32bit_c0_register($26, 0, val)

#define read_c0_derraddr0()	__read_ulong_c0_register($26, 1)
#define write_c0_derraddr0(val)	__write_ulong_c0_register($26, 1, val)

#define read_c0_cacheerr()	__read_32bit_c0_register($27, 0)

#define read_c0_derraddr1()	__read_ulong_c0_register($27, 1)
#define write_c0_derraddr1(val)	__write_ulong_c0_register($27, 1, val)

#define read_c0_taglo()		__read_32bit_c0_register($28, 0)
#define write_c0_taglo(val)	__write_32bit_c0_register($28, 0, val)

#define read_c0_taghi()		__read_32bit_c0_register($29, 0)
#define write_c0_taghi(val)	__write_32bit_c0_register($29, 0, val)

#define read_c0_errorepc()	__read_ulong_c0_register($30, 0)
#define write_c0_errorepc(val)	__write_ulong_c0_register($30, 0, val)

/*
 * TLB operations.
 */
static inline void tlb_probe(void)
{
	__asm__ __volatile__(
		".set noreorder\n\t"
		"tlbp\n\t"
		".set reorder");
}

static inline void tlb_read(void)
{
	__asm__ __volatile__(
		".set noreorder\n\t"
		"tlbr\n\t"
		".set reorder");
}

static inline void tlb_write_indexed(void)
{
	__asm__ __volatile__(
		".set noreorder\n\t"
		"tlbwi\n\t"
		".set reorder");
}

static inline void tlb_write_random(void)
{
	__asm__ __volatile__(
		".set noreorder\n\t"
		"tlbwr\n\t"
		".set reorder");
}

#define ASID_INC	0x1
#define ASID_MASK	0xff


/*
 * PAGE_SHIFT determines the page size
 */
#ifdef CONFIG_PAGE_SIZE_4KB
#define PAGE_SHIFT	12
#endif
#ifdef CONFIG_PAGE_SIZE_16KB
#define PAGE_SHIFT	14
#endif
#ifdef CONFIG_PAGE_SIZE_64KB
#define PAGE_SHIFT	16
#endif
#ifdef CONFIG_PAGE_SIZE_256KB
#define PAGE_SHIFT	18
#endif
#ifdef CONFIG_PAGE_SIZE_1M
#define PAGE_SHIFT	20
#endif
#define PAGE_SIZE	(1UL << PAGE_SHIFT)
#define PAGE_MASK	(~(PAGE_SIZE-1))


/* For vesa mode control */
#define GRAPHIC_MODE_100	0x100	/* 640x400 256*/
#define GRAPHIC_MODE_101	0x101	/* 640x480 256*/
#define GRAPHIC_MODE_102	0x102	/* 800x600 16 */
#define GRAPHIC_MODE_103	0x103	/* 800x600 256*/
#define GRAPHIC_MODE_104	0x104	/* 1024x768 16*/
#define GRAPHIC_MODE_105	0x105	/* 1024x768 256*/
#define GRAPHIC_MODE_106	0x106	/* 1280x1024 16*/
#define GRAPHIC_MODE_107	0x107	/* 1280x1024 256*/
#define GRAPHIC_MODE_10d	0x10d	/* 320x200 32K(1:5:5:5)*/
#define GRAPHIC_MODE_10e	0x10e	/* 320x200 64K(5:6:5)*/
#define GRAPHIC_MODE_10f	0x10f	/* 320x200 16.8M(8:8:8)*/
#define GRAPHIC_MODE_110	0x110	/* 640x480 32K*/
#define GRAPHIC_MODE_111	0x111	/* 640x480 64K*/
#define GRAPHIC_MODE_112	0x112	/* 640x480 16.8M*/
#define GRAPHIC_MODE_113	0x113	/* 800x600 32K*/
#define GRAPHIC_MODE_114	0x114	/* 800x600 64K*/
#define GRAPHIC_MODE_115	0x115	/* 800x600 16.8M*/
#define GRAPHIC_MODE_116	0x116	/* 1024x768 32K*/
#define GRAPHIC_MODE_117	0x117	/* 1024x768 64K*/
#define GRAPHIC_MODE_118	0x118	/* 1024x768 16.8M*/
#define GRAPHIC_MODE_119	0x119	/* 1280x1024 32K*/
#define GRAPHIC_MODE_11a	0x11a	/* 1280x1024 64K*/
#define GRAPHIC_MODE_11b	0x11b	/* 1280x1024 16.8M*/
#define USE_LINEAR_FRAMEBUFFER 0x4000
struct vesamode vesamode[] = {
	{GRAPHIC_MODE_114,800,600,16}, /* default 800x600x16 */
	{GRAPHIC_MODE_100,640,400,8},
	{GRAPHIC_MODE_101,640,480,8},
	{GRAPHIC_MODE_102,800,600,4},
	{GRAPHIC_MODE_103,800,600,8},
	{GRAPHIC_MODE_104,1024,768,16},
	{GRAPHIC_MODE_105,1024,768,8},
	{GRAPHIC_MODE_106,1280,1024,16},
	{GRAPHIC_MODE_107,1280,1024,8},
	{GRAPHIC_MODE_10d,320,200,15}, 
	{GRAPHIC_MODE_10e,320,200,16}, 
	{GRAPHIC_MODE_10f,320,200,24},
	{GRAPHIC_MODE_110,640,480,15},
	{GRAPHIC_MODE_111,640,480,16},
	{GRAPHIC_MODE_112,640,480,24},
	{GRAPHIC_MODE_113,800,600,15},
	{GRAPHIC_MODE_114,800,600,16},
	{GRAPHIC_MODE_115,800,600,24},
	{GRAPHIC_MODE_116,1024,768,15},
	{GRAPHIC_MODE_117,1024,768,16},
	{GRAPHIC_MODE_118,1024,768,24},
	{GRAPHIC_MODE_119,1280,1024,15},
	{GRAPHIC_MODE_11a,1280,1024,16},
	{GRAPHIC_MODE_11b,1280,1024,24},
};

struct vesamode *vesa_mode_head = vesamode;


static u32 io_vaddr;

int cacluate_vesamode(int start)
{
	int x,y,depth,mode;
	int i;
	x=800;
	y=600;
	depth=16;
	mode=-1;
#ifdef FB_XSIZE
	x=FB_XSIZE;
#endif

#ifdef FB_YSIZE
	y=FB_YSIZE;
#endif
	for(i=start;i<sizeof(vesamode)/sizeof(vesamode[0]);i++)
		if(x==vesamode[i].width && y==vesamode[i].height && depth==vesamode[i].bpp && vesamode[i].mode>0x107)
		{
			mode = i;
			break;
		}
	return mode;
}


void tlbmap(u32 viraddr, u32 phyaddr, u32 size)
{
	u32 tmp_size;
	u32	pid, idx;
	write_c0_pagemask(PM_DEFAULT_MASK);

	pid = read_c0_entryhi() & ASID_MASK;

	printf("tlbmap vaddr %x paddr %x size %x\n", viraddr, phyaddr, size);
	for (tmp_size = 0 ;tmp_size < size; tmp_size += (2*PAGE_SIZE)) {
		viraddr &= (PAGE_MASK << 1);
		write_c0_entryhi(viraddr | (pid));
		tlb_probe();
		idx = read_c0_index();
		printf("viraddr=%08x,phyaddr=%08x,pid=%x,idx=%x\n",viraddr,phyaddr,pid,idx);
		/* Uncached accelerate */
		write_c0_entrylo0((phyaddr >> 6)|0x3f);
		write_c0_entrylo1(((phyaddr+PAGE_SIZE) >> 6)|0x3f);
		write_c0_entryhi(viraddr | (pid));

		if(idx < 0) {
			tlb_write_random();
		} else {
			tlb_write_indexed();
		}
		viraddr += (PAGE_SIZE*2);
		phyaddr += (PAGE_SIZE*2);
	}
	write_c0_entryhi(pid);
}

#define BONITO_REGBASE			0x100
#define BONITO_PCIMAP			BONITO(BONITO_REGBASE + 0x10)
#define BONITO(x)	*(volatile unsigned long *)(0xbfe00000+(x))
#define BONITO_PCIMAP			BONITO(BONITO_REGBASE + 0x10)
#define BONITO_PCIMAP_PCIMAP_LO0	0x0000003f
#define BONITO_PCIMAP_WIN(WIN,ADDR)	((((ADDR)>>26) & BONITO_PCIMAP_PCIMAP_LO0) << ((WIN)*6))

int  vesafb_init(void)
{
	u32 video_mem_size;
	u32 fb_address, io_address;
	u32 tmp;

	if(vga_dev != NULL){
		fb_address  =_pci_conf_read(vga_dev->pa.pa_tag,0x10);
		io_address  =_pci_conf_read(vga_dev->pa.pa_tag,0x18);
	}
	if(pcie_dev != NULL){
		fb_address  =_pci_conf_read(pcie_dev->pa.pa_tag,0x10);
		io_address  =_pci_conf_read(pcie_dev->pa.pa_tag,0x18);
	}
	//io_vaddr = io_address | 0xbfd00000;
	//io_vaddr = io_address | BONITO_PCIIO_BASE_VA;
	io_vaddr = io_address | 0xb8000000;

	/* We assume all the framebuffer is required remmapping */
#ifdef USETLB
	/* Remap framebuffer address to 0x40000000, which can be accessed by same physical address from cpu */
	_pci_conf_write(vga_dev->pa.pa_tag, 0x10, 0x40000000);
	/* FIXME: video memory size should be detected by software, but now fixed in 2MB that's enough in PMON */
	video_mem_size = 0x200000;

#if 1
	/* Map cpu physical address 0x40000000 to BONITO address 0x40000000 -> PCI address 0x40000000.
	 * Master0 window 3 */
	asm (
			".set mips3\n"
			".set noreorder\n"
			"dli $2, 0x900000003ff00000\n"
			"dli $3, 0x0000000040000000\n"
			"sd  $3, 0x18($2)\n"
			"dli $3, 0x0000000040000001\n"
			"sd  $3, 0x58($2)\n"
			"dli $3, 0xffffffffc0000000\n"
			"sd  $3, 0x38($2)\n"
			".set reorder\n"
			".set mips0\n"
			:::"$2","$3","memory"
		);
#endif

	/* If framebuffer bar is NULL, then it has too large memory to be alloced in kernel mode */
	/* TLB map physical address to virtual address in kseg3. Start address is 0xe0000000 */
	tlbmap(0xe0000000, 0x40000000, video_mem_size);
#else

#if	0
	/* 0x10000000 -> 0x40000000 PCI mapping */
	_pci_conf_write(vga_dev->pa.pa_tag, 0x10, 0x40000000);
	tmp = BONITO_PCIMAP;
	BONITO_PCIMAP = 
	    BONITO_PCIMAP_WIN(0, 0x40000000) |	
		(tmp & ~BONITO_PCIMAP_PCIMAP_LO0);
#endif
#endif

	printf("VESA FB init complete.\n");

	return 0;
}

/* dummy implementation */
void video_set_lut2(int index, int rgb)
{
	return;
}

int GetXRes(void)
{
	return vesamode[vesa_mode].width;
}

int GetYRes(void)
{
	return vesamode[vesa_mode].height;
}

int GetBytesPP(void)
{
	return (vesamode[vesa_mode].bpp+1)/8;
}

void video_set_lut(int index, int r, int g, int b)
{				     
	linux_outb(index, 0x03C8);
	linux_outb(r >> 2, 0x03C9);
	linux_outb(g >> 2, 0x03C9);
	linux_outb(b >> 2, 0x03C9);
}
