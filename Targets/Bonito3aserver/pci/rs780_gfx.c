/*
 * This file is part of the coreboot project.
 *
 * Copyright (C) 2010 Advanced Micro Devices, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 */

/*
 *  for rs780 internal graphics device
 *  device id of internal grphics:
 *	RS780:	0x9610
 *	RS780C:	0x9611
 *	RS780M:	0x9612
 *	RS780MC:0x9613
 *	RS780E: 0x9615
 *	RS785G: 0x9710 - just works, not much tested
 */
#include "rs780.h"

/* Trust the original resource allocation. Don't do it again. */
#undef DONT_TRUST_RESOURCE_ALLOCATION
//#define DONT_TRUST_RESOURCE_ALLOCATION

#define CLK_CNTL_INDEX	0x8
#define CLK_CNTL_DATA	0xC

/* The Integrated Info Table. */
//ATOM_INTEGRATED_SYSTEM_INFO_V2 vgainfo;

#ifdef UNUSED_CODE
static u32 clkind_read(device_t dev, u32 index)
{
	u32	gfx_bar2 = BONITO_PCILO_BASE_VA | (pci_read_config32(dev, 0x18) & ~0xF);

	*(u32*)(gfx_bar2+CLK_CNTL_INDEX) = index & 0x7F;
	return *(u32*)(gfx_bar2+CLK_CNTL_DATA);
}
#endif

static void clkind_write(device_t dev, u32 index, u32 data)
{
	u32	gfx_bar2 = (pci_read_config32(dev, 0x18) & ~0xF) | BONITO_PCILO_BASE_VA;

	*(u32*)(gfx_bar2+CLK_CNTL_INDEX) = index | 1<<7;
	*(u32*)(gfx_bar2+CLK_CNTL_DATA)  = data;
}

#define PCI_VENDOR_ID       0x00    /* 16 bits */
#define PCI_DEVICE_ID       0x02    /* 16 bits */

static void internal_gfx_pci_dev_init(device_t nb , device_t dev)
{
	unsigned char * bpointer;
	volatile u32 * GpuF0MMReg;
	volatile u32 * pointer;
	int i;
	u16 command;
	u32 value;
	u16 deviceid, vendorid;
	device_t nb_dev = _pci_make_tag(0, 0, 0);

	deviceid = pci_read_config16(dev, PCI_DEVICE_ID);
	vendorid = pci_read_config16(dev, PCI_VENDOR_ID);
	printk_info( "internal_gfx_pci_dev_init device=%x, vendor=%x.\n",
	     deviceid, vendorid);

	command = pci_read_config16(dev, 0x04);
	command |= 0x7;
	pci_write_config16(dev, 0x04, command);

    //enable apc_dev again for it was closed before 
    pci_write_config32(_pci_make_tag(0,1,0) ,0x04 , 0x7 | pci_read_config32(_pci_make_tag(0,1,0) , 0x04));
    printk_info("apc_dev------04 ----------%x \n" , pci_read_config32(_pci_make_tag(0,1,0) , 0x04));

	GpuF0MMReg = (u32 *)(BONITO_PCILO_BASE_VA | pci_read_config32(dev, 0x18));

    printf("GpuF0MMReg :   %x =============\n" , GpuF0MMReg);
	/* GFX_InitFBAccess. */
    *(GpuF0MMReg + 0x0) = 0x544c;
    printk_info("first reg succeeded:%x \n",*(GpuF0MMReg + 0x0));
    printk_info("first reg succeeded:%x \n",*(GpuF0MMReg + 0x4));

	value = nbmc_read_index(nb_dev, 0x10);
	*(GpuF0MMReg + 0x2000/4) = 0x11;
    printk_info("first reg succeeded:%x \n",*(GpuF0MMReg + 0x2000/4));

	*(GpuF0MMReg + 0x2180/4) = ((value&0xff00)>>8)|((value&0xff000000)>>8);
	*(GpuF0MMReg + 0x2c04/4) = ((value&0xff00)<<8);
	*(GpuF0MMReg + 0x5428/4) = ((value&0xffff0000)+0x10000)-((value&0xffff)<<16);
	*(GpuF0MMReg + 0x2000/4) = 0x00000011;
	*(GpuF0MMReg + 0x200c/4) = 0x00000020;
	*(GpuF0MMReg + 0x2010/4) = 0x10204810;
    printk_info("first reg succeeded:%x\n",*(GpuF0MMReg + 0x2010/4));
    
	*(GpuF0MMReg + 0x2010/4) = 0x00204810;
	*(GpuF0MMReg + 0x2014/4) = 0x10408810;
	*(GpuF0MMReg + 0x2014/4) = 0x00408810;
	*(GpuF0MMReg + 0x2414/4) = 0x00000080;
	*(GpuF0MMReg + 0x2418/4) = 0x84422415;
	*(GpuF0MMReg + 0x2418/4) = 0x04422415;
	*(GpuF0MMReg + 0x5490/4) = 0x00000001;
	*(GpuF0MMReg + 0x7de4/4) |= (1<<3) | (1<<4);
	/* Force allow LDT_STOP Cool'n'Quiet workaround. */
	*(GpuF0MMReg + 0x655c/4) |= 1<<4;
	/* GFX_InitFBAccess finished. */

	/* GFX_StartMC. */
#ifdef CONFIG_GFXUMA /* for UMA mode. */
	/* MC_INIT_COMPLETE. */
	set_nbmc_enable_bits(nb_dev, 0x2, 0, 1<<31);
	/* MC_STARTUP, MC_POWERED_UP and MC_VMODE.*/
	set_nbmc_enable_bits(nb_dev, 0x1, 1<<18, 1|1<<2);

	set_nbmc_enable_bits(nb_dev, 0xb1, 0, 1<<6);
	set_nbmc_enable_bits(nb_dev, 0xc3, 0, 1);
	nbmc_write_index(nb_dev, 0x07, 0x18);
	nbmc_write_index(nb_dev, 0x06, 0x00000102);
	nbmc_write_index(nb_dev, 0x09, 0x40000008);
	set_nbmc_enable_bits(nb_dev, 0x6, 0, 1<<31);
	/* GFX_StartMC finished. */
#else
#if 0
// sp mode is not tested
	/* for SP mode. */
	set_nbmc_enable_bits(nb_dev, 0xaa, 0xf0, 0x30);
	set_nbmc_enable_bits(nb_dev, 0xce, 0xf0, 0x30);
	set_nbmc_enable_bits(nb_dev, 0xca, 0xff000000, 0x47000000);
	set_nbmc_enable_bits(nb_dev, 0xcb, 0x3f000000, 0x01000000);
	set_nbmc_enable_bits(nb_dev, 0x01, 0, 1<<0);
	set_nbmc_enable_bits(nb_dev, 0x04, 0, 1<<31);
	set_nbmc_enable_bits(nb_dev, 0xb4, 0x3f, 0x3f);
	set_nbmc_enable_bits(nb_dev, 0xb4, 0, 1<<6);
	set_nbmc_enable_bits(nb_dev, 0xc3, 1<<11, 0);
	set_nbmc_enable_bits(nb_dev, 0xa0, 1<<29, 0);
	nbmc_write_index(nb_dev, 0xa4, 0x3484576f);
	nbmc_write_index(nb_dev, 0xa5, 0x222222df);
	nbmc_write_index(nb_dev, 0xa6, 0x00000000);
	nbmc_write_index(nb_dev, 0xa7, 0x00000000);
	set_nbmc_enable_bits(nb_dev, 0xc3, 1<<8, 0);
	udelay(10);
	set_nbmc_enable_bits(nb_dev, 0xc3, 1<<9, 0);
	udelay(10);
	set_nbmc_enable_bits(nb_dev, 0x01, 0, 1<<2);
	udelay(200);
	set_nbmc_enable_bits(nb_dev, 0x01, 0, 1<<3);
	set_nbmc_enable_bits(nb_dev, 0xa0, 0, 1<<31);
	udelay(500);
	set_nbmc_enable_bits(nb_dev, 0x02, 0, 1<<31);
	set_nbmc_enable_bits(nb_dev, 0xa0, 0, 1<<30);
	set_nbmc_enable_bits(nb_dev, 0xa0, 1<<31, 0);
	set_nbmc_enable_bits(nb_dev, 0xa0, 0, 1<<29);
	nbmc_write_index(nb_dev, 0xa4, 0x23484576);
	nbmc_write_index(nb_dev, 0xa5, 0x00000000);
	nbmc_write_index(nb_dev, 0xa6, 0x00000000);
	nbmc_write_index(nb_dev, 0xa7, 0x00000000);
	/* GFX_StartMC finished. */

	/* GFX_SPPowerManagment, don't care for new. */
	/* Post MC Init table programming. */
	set_nbmc_enable_bits(nb_dev, 0xac, ~(0xfffffff0), 0x0b);

	/* Do we need Write and Read Calibration? */
	/* GFX_Init finished. */
#endif
#endif
	/* GFX_InitLate. */
	{
		u8 temp8;
		temp8 = pci_read_config8(dev, 0x4);
		//temp8 &= ~1; /* CIM clears this bit. Strangely, I can'd. */
		temp8 |= 1<<1|1<<2;
		pci_write_config8(dev, 0x4, temp8);
	}

	/* clk ind */
	clkind_write(dev, 0x08, 0x01);
	clkind_write(dev, 0x0C, 0x22);
	clkind_write(dev, 0x0F, 0x0);
	clkind_write(dev, 0x11, 0x0);
	clkind_write(dev, 0x12, 0x0);
	clkind_write(dev, 0x14, 0x0);
	clkind_write(dev, 0x15, 0x0);
	clkind_write(dev, 0x16, 0x0);
	clkind_write(dev, 0x17, 0x0);
	clkind_write(dev, 0x18, 0x0);
	clkind_write(dev, 0x19, 0x0);
	clkind_write(dev, 0x1A, 0x0);
	clkind_write(dev, 0x1B, 0x0);
	clkind_write(dev, 0x1C, 0x0);
	clkind_write(dev, 0x1D, 0x0);
	clkind_write(dev, 0x1E, 0x0);
	clkind_write(dev, 0x26, 0x0);
	clkind_write(dev, 0x27, 0x0);
	clkind_write(dev, 0x28, 0x0);
	clkind_write(dev, 0x5C, 0x0);
}


void rs780_internal_gfx_init(device_t nb_dev,device_t dev)
{

    /* BTDC: NB_InitGFXStraps */
    u32 u32temp;
    u8  u8temp;
    u32 MMIOBase, apc04, apc18, apc24;
    volatile u32 * strap;

    /* BTDC: Get PCIe configuration space. */
    printk_info("Get PCIe configuration space.\n");
    MMIOBase = pci_read_config32(nb_dev, 0x1c) & 0xfffffff0;
    printk_info("MMIOBase=%08x\n", MMIOBase);

    /* BTDC: Temporarily disable PCIe configuration space. */
    printk_info("Temporarily disable PCIe configuration space\n");
    set_htiu_enable_bits(nb_dev, 0x32, 1<<28, 0);
    set_nbmisc_enable_bits(nb_dev, 0x1e, 0xffffffff, 1<<1 | 1<<4 | 1<<6 | 1 << 7);

    /* BTDC: Set a temporary Bus number. */
    printk_info("Set a temporary Bus number.\n");
    apc18 = pci_read_config32(dev, 0x18);
    pci_write_config32(dev, 0x18, 0x010100);


    /* BTDC: Set MMIO for AGP target(graphics controller). base = 0xe0000000, limit = 0x20000 */
    printk_info("Set MMIO for AGP target(graphics controller).\n");
    apc24 = pci_read_config32(dev, 0x24);
    pci_write_config32(dev, 0x24, (MMIOBase>>16)+((MMIOBase+0x20000)&0xffff0000));

    /* BTDC: Enable memory access. */
    printk_info("Enable memory access\n");
    apc04 = pci_read_config32(dev, 0x04);
    pci_write_config8(dev, 0x04, 0x02);
    
    /* BTDC: Program Straps. */
    printk_info("Program Straps\n");
    //lycheng
  //  MMIOBase |= 0xb6000000;
   // MMIOBase = BONITO_PCICFG1_BASE_VA;
    MMIOBase |= BONITO_PCILO_BASE_VA;
    printk_info("MMIOBase=%08x\n", MMIOBase);
    strap = MMIOBase + 0x15000;
    *strap = 0x2c006300;
    strap = MMIOBase + 0x15010;
    *strap = 0x03015330;
    strap = MMIOBase + 0x15020;
#ifdef CONFIG_GFXUMA 
    *strap = 0x2 << 7; /* BTDC: the format of BIF_MEM_AP_SIZE. 001->256MB? */
#else
    *strap = 0; /* BTDC: 128M SP memory, 000 -> 128MB */
#endif
    //strap = MMIOBase + 0x15020;
    //*strap |= 0x00000040; /* BTDC: Disable HDA device. */
    strap = MMIOBase + 0x15030;
    *strap = 0x00001002;
    strap = MMIOBase + 0x15040;
    *strap = 0x00000000;
    strap = MMIOBase + 0x15050;
    *strap = 0x00000000;
    strap = MMIOBase + 0x15220;
    *strap = 0x03c03800;
    strap = MMIOBase + 0x15060;
    *strap = 0x00000000;
    /* BTDC: BIF switches into normal functional mode. */
    printk_info("BIF switches into normal functional mode.\n");
    set_nbmisc_enable_bits(nb_dev, 0x1e, 1<<4 | 1<<5, 1<<5);
    /* BTDC: NB Revision is A12. */
    printk_info("NB Revision is A12\n");
    set_nbmisc_enable_bits(nb_dev, 0x1e, 1<<9, 1<<9);
    /* BTDC: Restore APC04, APC18, APC24. */
    printk_info("Restore APC04, APC18, APC24.\n");
    pci_write_config32(dev, 0x04, apc04);
    pci_write_config32(dev, 0x18, apc18);
    pci_write_config32(dev, 0x24, apc24);

    /* BTDC: Enable PCIe configuration space. */
    printk_info("Enable PCIe configuration space.\n");
    set_htiu_enable_bits(nb_dev, 0x32, 0, 1<<28);
    printk_info("BTDC: GC is accessible from now on.\n");
//	set_nbcfg_enable_bits(nb_dev, 0x7C, 1 << 30, 1 << 30);
}


/*
* Set registers in RS780 and CPU to enable the internal GFX.
* Please refer to CIM source code and BKDG.
*/
static uint64_t uma_memory_base = 0xf8000000;
static uint64_t uma_memory_size = 0x04000000;
static uint64_t uma_memory_top = 0xfc000000; //base + size

static void rs780_internal_gfx_enable(device_t nb , device_t dev)
{
	u32 l_dword;
	int i;
	device_t nb_dev = _pci_make_tag(0, 0,0);
	device_t k8_f0 = _pci_make_tag(0, 0x18, 0);
	device_t k8_f2 = _pci_make_tag(0, 0x18, 2);
    u32 temp;

#ifndef CONFIG_GFXUMA 
    u32 FB_Start, FB_End;
#endif
	printk_info( "rs780_internal_gfx_enable dev = 0x%p, nb_dev = 0x%p.\n", dev, nb_dev);

	/* The system top memory in 780. */
//	pci_write_config32(nb_dev, 0x90, 0xf0000000);
	htiu_write_index(nb_dev, 0x30, 0);
	htiu_write_index(nb_dev, 0x31, 0);

	/* Disable external GFX and enable internal GFX. */
	l_dword = pci_read_config32(nb_dev, 0x8c);
	l_dword &= ~(1<<0);
	l_dword |= 1<<1;
	pci_write_config32(nb_dev, 0x8c, l_dword);

	/* NB_SetDefaultIndexes */
	pci_write_config32(nb_dev, 0x94, 0x7f);
	pci_write_config32(nb_dev, 0x60, 0x7f);
	pci_write_config32(nb_dev, 0xe0, 0);

	/* NB_InitEarlyNB finished. */
	/* GFX_InitCommon. */
	nbmc_write_index(nb_dev, 0x23, 0x00c00010);
	set_nbmc_enable_bits(nb_dev, 0x16, 1<<15, 1<<15);
	set_nbmc_enable_bits(nb_dev, 0x25, 0xffffffff, 0x111f111f);
	set_htiu_enable_bits(nb_dev, 0x37, 1<<24, 1<<24);

#ifdef  CONFIG_GFXUMA
	/* Set UMA in the 780 side. */
	/* UMA start address, size. */
	/* The same value in spite of system memory size. */
	//nbmc_write_index(nb_dev, 0x10, 0xcfffc000);
	nbmc_write_index(nb_dev, 0x12, 0);
	nbmc_write_index(nb_dev, 0xd, (uma_memory_top & 0xfff00000) | 0x00054064);
    set_nbmc_enable_bits(nb_dev , 0xe , 0xfffffff , 0xffff000 | uma_memory_top >> 20);
    nbmc_write_index(nb_dev, 0x11, uma_memory_base);
 //   pci_write_config32(nb_dev,0x90,0x90000000);
	nbmc_write_index(nb_dev, 0x10, ((uma_memory_top - 1) & 0xff000000 ) | (uma_memory_base >> 16) & 0xffff);
    printf("config UMA!!!!!!!!!!!!!!!!!!!!!!!!!!!!1\n");
    printf("mc : %d  ============value: %x\n" , 0xd , nbmc_read_index(nb_dev,0xd));
    printf("mc : %d  ============value: %x\n" , 0xe , nbmc_read_index(nb_dev,0xe));
    printf("mc : %d  ============value: %x\n" , 0x10 , nbmc_read_index(nb_dev,0x10));
    printf("mc : %d  ============value: %x\n" , 0x11 , nbmc_read_index(nb_dev,0x11));
    printf("mc : %d  ============value: %x\n" , 0x12 , nbmc_read_index(nb_dev,0x12));
    printf("nb : %d  ============value: %x\n" , 0x90 , pci_read_config32(nb_dev,0x90));

	/* GFX_InitUMA finished. */
#else
#if 0
//sp mode is not tested  
	/* GFX_InitSP. */
	/* SP memory:Hynix HY5TQ1G631ZNFP. 128MB = 64M * 16. 667MHz. DDR3. */

	/* Enable Async mode. */
	set_nbmc_enable_bits(nb_dev, 0x06, 7<<8, 1<<8);
	set_nbmc_enable_bits(nb_dev, 0x08, 1<<10, 0);
	/* The last item in AsynchMclkTaskFileIndex. Why? */
	/* MC_MPLL_CONTROL2. */
	nbmc_write_index(nb_dev, 0x07, 0x40100028);
	/* MC_MPLL_DIV_CONTROL. */
	nbmc_write_index(nb_dev, 0x0b, 0x00000028);
	/* MC_MPLL_FREQ_CONTROL. */
	set_nbmc_enable_bits(nb_dev, 0x09, 3<<12|15<<16|15<<8, 1<<12|4<<16|0<<8);
	/* MC_MPLL_CONTROL3. For PM. */
	set_nbmc_enable_bits(nb_dev, 0x08, 0xff<<13, 1<<13|1<<18);
	/* MPLL_CAL_TRIGGER. */
	set_nbmc_enable_bits(nb_dev, 0x06, 0, 1<<0);
	udelay(200); /* time is long enough? */
	set_nbmc_enable_bits(nb_dev, 0x06, 0, 1<<1);
	set_nbmc_enable_bits(nb_dev, 0x06, 1<<0, 0);
	/* MCLK_SRC_USE_MPLL. */
	set_nbmc_enable_bits(nb_dev, 0x02, 0, 1<<20);

	/* Pre Init MC. */
	nbmc_write_index(nb_dev, 0x01, 0x88108280);
	set_nbmc_enable_bits(nb_dev, 0x02, ~(1<<20), 0x00030200);
	nbmc_write_index(nb_dev, 0x04, 0x08881018);
	nbmc_write_index(nb_dev, 0x05, 0x000000bb);
	nbmc_write_index(nb_dev, 0x0c, 0x0f00001f);
	nbmc_write_index(nb_dev, 0xa1, 0x01f10000);
	/* MCA_INIT_DLL_PM. */
	set_nbmc_enable_bits(nb_dev, 0xc9, 1<<24, 1<<24);
	nbmc_write_index(nb_dev, 0xa2, 0x74f20000);
	nbmc_write_index(nb_dev, 0xa3, 0x8af30000);
	nbmc_write_index(nb_dev, 0xaf, 0x47d0a41c);
	nbmc_write_index(nb_dev, 0xb0, 0x88800130);
	nbmc_write_index(nb_dev, 0xb1, 0x00000040);
	nbmc_write_index(nb_dev, 0xb4, 0x41247000);
	nbmc_write_index(nb_dev, 0xb5, 0x00066664);
	nbmc_write_index(nb_dev, 0xb6, 0x00000022);
	nbmc_write_index(nb_dev, 0xb7, 0x00000044);
	nbmc_write_index(nb_dev, 0xb8, 0xbbbbbbbb);
	nbmc_write_index(nb_dev, 0xb9, 0xbbbbbbbb);
	nbmc_write_index(nb_dev, 0xba, 0x55555555);
	nbmc_write_index(nb_dev, 0xc1, 0x00000000);
	nbmc_write_index(nb_dev, 0xc2, 0x00000000);
	nbmc_write_index(nb_dev, 0xc3, 0x80006b00);
	nbmc_write_index(nb_dev, 0xc4, 0x00066664);
	nbmc_write_index(nb_dev, 0xc5, 0x00000000);
	nbmc_write_index(nb_dev, 0xd2, 0x00000022);
	nbmc_write_index(nb_dev, 0xd3, 0x00000044);
	nbmc_write_index(nb_dev, 0xd6, 0x00050005);
	nbmc_write_index(nb_dev, 0xd7, 0x00000000);
	nbmc_write_index(nb_dev, 0xd8, 0x00700070);
	nbmc_write_index(nb_dev, 0xd9, 0x00700070);
	nbmc_write_index(nb_dev, 0xe0, 0x00200020);
	nbmc_write_index(nb_dev, 0xe1, 0x00200020);
	nbmc_write_index(nb_dev, 0xe8, 0x00200020);
	nbmc_write_index(nb_dev, 0xe9, 0x00200020);
	nbmc_write_index(nb_dev, 0xe0, 0x00180018);
	nbmc_write_index(nb_dev, 0xe1, 0x00180018);
	nbmc_write_index(nb_dev, 0xe8, 0x00180018);
	nbmc_write_index(nb_dev, 0xe9, 0x00180018);

	/* Misc options. */
	/* Memory Termination. */
	set_nbmc_enable_bits(nb_dev, 0xa1, 0x0ff, 0x044);
	set_nbmc_enable_bits(nb_dev, 0xb4, 0xf00, 0xb00);
#if 0
	/* Controller Termation. */
	set_nbmc_enable_bits(nb_dev, 0xb1, 0x77770000, 0x77770000);
#endif

	/* OEM Init MC. 667MHz. */
	nbmc_write_index(nb_dev, 0xa8, 0x7a5aaa78);
	nbmc_write_index(nb_dev, 0xa9, 0x514a2319);
	nbmc_write_index(nb_dev, 0xaa, 0x54400520);
	nbmc_write_index(nb_dev, 0xab, 0x441460ff);
	nbmc_write_index(nb_dev, 0xa0, 0x20f00a48);
	set_nbmc_enable_bits(nb_dev, 0xa2, ~(0xffffffc7), 0x10);
	nbmc_write_index(nb_dev, 0xb2, 0x00000303);
	set_nbmc_enable_bits(nb_dev, 0xb1, ~(0xffffff70), 0x45);
	/* Do it later. */
	/* set_nbmc_enable_bits(nb_dev, 0xac, ~(0xfffffff0), 0x0b); */

	/* Init PM timing. */
	for(i=0; i<4; i++)
	{
		l_dword = nbmc_read_index(nb_dev, 0xa0+i);
		nbmc_write_index(nb_dev, 0xc8+i, l_dword);
	}
	for(i=0; i<4; i++)
	{
		l_dword = nbmc_read_index(nb_dev, 0xa8+i);
		nbmc_write_index(nb_dev, 0xcc+i, l_dword);
	}
	l_dword = nbmc_read_index(nb_dev, 0xb1);
	set_nbmc_enable_bits(nb_dev, 0xc8, 0xff<<24, ((l_dword&0x0f)<<24)|((l_dword&0xf00)<<20));

	/* Init MC FB. */
	/* FB_Start = ; FB_End = ; iSpSize = 0x0080, 128MB. */
	nbmc_write_index(nb_dev, 0x11, 0x78000000);
	FB_Start = 0x700 + 0x080;
	FB_End = 0x700 + 0x0c0;
	nbmc_write_index(nb_dev, 0x10, (((FB_End&0xfff)<<20)-0x10000)|(((FB_Start&0xfff))<<4));
	set_nbmc_enable_bits(nb_dev, 0x0d, ~0x000ffff0, (FB_End&0xfff)<<20);
	nbmc_write_index(nb_dev, 0x0f, 0);
	nbmc_write_index(nb_dev, 0x0e, (FB_End&0xfff)|(0xaaaa<<12));
    printf("mc : %d  ============value: %x\n" , 0xd , nbmc_read_index(nb_dev,0xd));
    printf("mc : %d  ============value: %x\n" , 0xe , nbmc_read_index(nb_dev,0xe));
    printf("mc : %d  ============value: %x\n" , 0x10 , nbmc_read_index(nb_dev,0x10));
    printf("mc : %d  ============value: %x\n" , 0x11 , nbmc_read_index(nb_dev,0x11));
    printf("mc : %d  ============value: %x\n" , 0x12 , nbmc_read_index(nb_dev,0x12));
    printf("nb : %d  ============value: %x\n" , 0x90 , pci_read_config32(nb_dev,0x90));
#endif
#endif
}

#if 1
static void pcie_commoncoreinit(device_t nb_dev, device_t dev)
{
	u32 reg;
	u16 word;
	u8  byte;

	set_pcie_enable_bits(nb_dev,0x10, 0x7 << 10, 0x4 << 10);
	set_pcie_enable_bits(nb_dev,0x20, 0x3 << 8,  0x3 << 8);
	set_pcie_enable_bits(nb_dev,0x02, 0x0, 1 << 0 | 1 << 8);
	set_pcie_enable_bits(nb_dev,0x40, 0x3 << 14, 0x2 << 14);
	set_pcie_enable_bits(nb_dev,0x40, 0x1 << 28, 0x0 << 28);
	set_pcie_enable_bits(nb_dev,0xc2, 1 << 14 | 1 << 25, 1 << 14 | 1 << 25);
	set_pcie_enable_bits(nb_dev,0xc1, 1 << 0 | 1 << 2, 1 << 0);
	set_pcie_enable_bits(nb_dev,0x1c, 0xffffffff, 4 << 6 | 4 << 1);


}

static void pcie_commonportinit(device_t nb_dev, device_t dev)
{
	u32 reg;
	u16 word;
	u8  byte;

	reg = pci_read_config32(dev,0x88);
	reg &= 0xfffffff0;
	reg |= 0x1;
	pci_write_config32(dev, 0x88, reg);

	set_pcie_enable_bits(dev, 0x02, 1 << 11, 1 << 11);
	set_pcie_enable_bits(dev, 0xa4, 1 << 0 , 0 << 0);
	set_pcie_enable_bits(dev, 0xa0, 0xf << 4 , 0x3 << 4);
	set_pcie_enable_bits(dev, 0xa1, 1 << 11 | 1 << 24 | 1 << 26, 1 << 11);
	set_pcie_enable_bits(dev, 0xb1, 0x0 , 1 << 28 | 1 << 23 | 1 << 19 | 1<< 20);
	set_pcie_enable_bits(dev, 0xa2, 0x0 , 1 << 13);
	set_pcie_enable_bits(dev, 0x70, 1 << 16 | 1 << 17 | 1 << 18 | 1 << 19, 1 << 18 | 1 << 19); 
}

//this include PCIE_InitGen2(Port, 1) and PCIE_InitGen2(Port, 0)

static void pcie_initgen2(device_t nb_dev, device_t dev)
{
	u32 reg;
	u16 word;
	u8  byte;

	set_pcie_enable_bits(dev, 0xa4, 1 << 0, 0 << 0);
	reg = pci_read_config32(dev,0x88);
	reg &= 0xfffffff0;
	reg |= 0x1;
	pci_write_config32(dev, 0x88, reg);
	set_nbmisc_enable_bits(nb_dev, 0x34, 1 << 5, 0 << 5);


	set_pcie_enable_bits(dev, 0xa4, 1 << 0, 1 << 0);
	reg = pci_read_config32(dev, 0x88);
	reg &= 0xfffffff0;
	reg |= 0x2;
	pci_write_config32(dev, 0x88, reg);
	set_nbmisc_enable_bits(nb_dev, 0x34, 1 << 5, 1 << 5);


	set_pcie_enable_bits(dev, 0xa4, 0, 1 << 29);
	set_pcie_enable_bits(dev, 0xc0, 1 << 15, 0 << 15);
	set_pcie_enable_bits(dev, 0xa2, 1 << 13, 0 << 13);

//set interrupt pin info
	pci_write_config8(dev, 0x3d, 0x1);


//	while(1);

}

static void pcie_gen2workaround(device_t nb_dev, device_t dev)
{
	u32 reg;
	u16 word;
	u8  byte;
	void set_pcie_reset();
	void set_pcie_dereset();



	set_pcie_enable_bits(dev, 0xa4, 1 << 0, 0 << 0);
	reg = pci_read_config32(dev,0x88);
	reg &= 0xfffffff0;
	reg |= 0x1;
	pci_write_config32(dev, 0x88, reg);
	set_nbmisc_enable_bits(nb_dev, 0x34, 1 << 5, 0 << 5);

	set_pcie_enable_bits(dev, 0xa4, 1 << 29, 0 << 29);
	set_pcie_enable_bits(dev, 0xc0, 1 << 15, 1 << 15);
	set_pcie_enable_bits(dev, 0xa2, 1 << 13, 1 << 13);

	reg = htiu_read_index(nb_dev, 0x15);
	reg &= ~ (1 << 4);
	htiu_write_index(nb_dev, 0x15, 0);

	set_pcie_reset();
	delay(1000);
	set_pcie_dereset();


}

/* step 12 ~ step 14 from rpr */
static void single_port_configuration(device_t nb_dev, device_t dev)
{
	u8 result, width;
	u32 reg32;
	struct southbridge_amd_rs780_config *cfg =&chip_info;

	printk_info("rs780_gfx_init single_port_configuration.\n");

	/* step 12 training, releases hold training for GFX port 0 (device 2) */
	PcieReleasePortTraining(nb_dev, dev, 2);
	result = PcieTrainPort(nb_dev, dev, 2);
	printk_info("rs780_gfx_init single_port_configuration step12.\n");

	/* step 13 Power Down Control */
	/* step 13.1 Enables powering down transmitter and receiver pads along with PLL macros. */
	set_pcie_enable_bits(nb_dev, 0x40, 1 << 0, 1 << 0);

	/* step 13.a Link Training was NOT successful */
	if (!result) {
		set_nbmisc_enable_bits(nb_dev, 0x8, 0x1 << 4, 0x1 << 4); /* prevent from training. */
		set_nbmisc_enable_bits(nb_dev, 0xc, 0x1 << 2, 0x1 << 2); /* hide the GFX bridge. */

		if (cfg->gfx_tmds)
			nbpcie_ind_write_index(nb_dev, 0x65, 0xccf0f0);
		else {
			nbpcie_ind_write_index(nb_dev, 0x65, 0xffffffff);
			set_nbmisc_enable_bits(nb_dev, 0x7, 1 << 3, 1 << 3);
		}
	} else {		/* step 13.b Link Training was successful */

		set_pcie_enable_bits(dev, 0xA2, 0xFF, 0x1);
			
		reg32 = nbpcie_p_read_index(dev, 0x29);
		width = reg32 & 0xFF;
		printk_debug("GFX Inactive Lanes = 0x%x.\n", width);
		switch (width) {
		case 1:
		case 2:
			nbpcie_ind_write_index(nb_dev, 0x65,
					       cfg->gfx_lane_reversal ? 0x7f7f : 0xccfefe);
			break;
		case 4:
			nbpcie_ind_write_index(nb_dev, 0x65,
					       cfg->gfx_lane_reversal ? 0x3f3f : 0xccfcfc);
			break;
		case 8:
			nbpcie_ind_write_index(nb_dev, 0x65,
					       cfg->gfx_lane_reversal ? 0x0f0f : 0xccf0f0);
			break;
		}



	}
	printk_info("rs780_gfx_init single_port_configuration step13.\n");



	/* step 14 Reset Enumeration Timer, disables the shortening of the enumeration timer */
	set_pcie_enable_bits(dev, 0x70, 1 << 19, 1 << 19);
	printk_info("rs780_gfx_init single_port_configuration step14.\n");
}

/* step 15 ~ step 18 from rpr */
static void dual_port_configuration(device_t nb_dev, device_t dev)
{
	u8 result, width;
	u32 reg32, dev_ind;
	struct southbridge_amd_rs780_config *cfg =&chip_info;

        _pci_break_tag(dev, NULL, &dev_ind, NULL);
	/* 5.4.1.2 Dual Port Configuration */
	set_nbmisc_enable_bits(nb_dev, 0x36, 1 << 31, 1 << 31);
	set_nbmisc_enable_bits(nb_dev, 0x08, 0xF << 8, 0x5 << 8);
	set_nbmisc_enable_bits(nb_dev, 0x36, 1 << 31, 0 << 31);

	/* Training for Device 2 */
	/* Releases hold training for GFX port 0 (device 2) */
	PcieReleasePortTraining(nb_dev, dev, dev_ind);
	/* PCIE Link Training Sequence */
	result = PcieTrainPort(nb_dev, dev, dev_ind);

	/* step 16: Power Down Control for Device 2 */
	/* step 16.a Link Training was NOT successful */
	if (!result) {
		/* Powers down all lanes for port A */
		/* nbpcie_ind_write_index(nb_dev, 0x65, 0x0f0f); */
		/* Note: I have to disable the slot where there isnt a device,
		 * otherwise the system will hang. I dont know why. Do you guys know? */
		set_nbmisc_enable_bits(nb_dev, 0x0c, 1 << dev_ind, 1 << dev_ind);

	} else {		/* step 16.b Link Training was successful */
		/* FIXME: I am confused about this. It looks like about powerdown or something. */
		reg32 = nbpcie_p_read_index(dev, 0xa2);
		width = (reg32 >> 4) & 0x7;
		printk_debug("GFX LC_LINK_WIDTH = 0x%x.\n", width);
		switch (width) {
		case 1:
		case 2:
			nbpcie_ind_write_index(nb_dev, 0x65,
					       cfg->gfx_lane_reversal ? 0x0707 : 0x0e0e);
			break;
		case 4:
			nbpcie_ind_write_index(nb_dev, 0x65,
					       cfg->gfx_lane_reversal ? 0x0303 : 0x0c0c);
			break;
		}
	}
#if 0		/* This function will be called twice, so we dont have to do dev 3 here. */
	/* step 17: Training for Device 3 */
	//set_nbmisc_enable_bits(nb_dev, 0x8, 1 << 5, 0 << 5);
	/* Releases hold training for GFX port 0 (device 3) */
	PcieReleasePortTraining(nb_dev, dev, 3);
	/* PCIE Link Training Sequence */
	result = PcieTrainPort(nb_dev, dev, 3);

	/*step 18: Power Down Control for Device 3 */
	/* step 18.a Link Training was NOT successful */
	if (!result) {
		/* Powers down all lanes for port B and PLL1 */
		nbpcie_ind_write_index(nb_dev, 0x65, 0xccf0f0);
	} else {		/* step 18.b Link Training was successful */

		reg32 = nbpcie_p_read_index(dev, 0xa2);
		width = (reg32 >> 4) & 0x7;
		printk_debug("GFX LC_LINK_WIDTH = 0x%x.\n", width);
		switch (width) {
		case 1:
		case 2:
			nbpcie_ind_write_index(nb_dev, 0x65,
					       cfg->gfx_lane_reversal ? 0x7070 : 0xe0e0);
			break;
		case 4:
			nbpcie_ind_write_index(nb_dev, 0x65,
					       cfg->gfx_lane_reversal ? 0x3030 : 0x0f0f);
			break;
		}
	}
#endif
}


/* For single port GFX configuration Only
* width:
* 	000 = x16
* 	001 = x1
*	010 = x2
*	011 = x4
*	100 = x8
*	101 = x12 (not supported)
*	110 = x16
*/
static void dynamic_link_width_control(device_t nb_dev, device_t dev, u8 width)
{
	u32 reg32;
	device_t sb_dev;
	struct southbridge_amd_rs780_config *cfg =&chip_info;

	/* step 5.9.1.1 */
	reg32 = nbpcie_p_read_index(dev, 0xa2);

	/* step 5.9.1.2 */
	set_pcie_enable_bits(nb_dev, 0x40, 1 << 0, 1 << 0);
	/* step 5.9.1.3 */
	set_pcie_enable_bits(dev, 0xa2, 3 << 0, width << 0);
	/* step 5.9.1.4 */
	set_pcie_enable_bits(dev, 0xa2, 1 << 8, 1 << 8);
	/* step 5.9.2.4 */
	if (0 == cfg->gfx_reconfiguration)
		set_pcie_enable_bits(dev, 0xa2, 1 << 11, 1 << 11);

	/* step 5.9.1.5 */
	do {
		reg32 = nbpcie_p_read_index(dev, 0xa2);
	}
	while (reg32 & 0x100);

	/* step 5.9.1.6 */
	sb_dev = _pci_make_tag(0, 8, 0);
	do {
		reg32 = pci_ext_read_config16(nb_dev, sb_dev,
					  PCIE_VC0_RESOURCE_STATUS);
	} while (reg32 & VC_NEGOTIATION_PENDING);

	/* step 5.9.1.7 */
	reg32 = nbpcie_p_read_index(dev, 0xa2);
	if (((reg32 & 0x70) >> 4) != 0x6) {
		/* the unused lanes should be powered off. */
	}

	/* step 5.9.1.8 */
	set_pcie_enable_bits(nb_dev, 0x40, 1 << 0, 0 << 0);
}

/*
* GFX Core initialization, dev2, dev3
*/
void rs780_gfx_init(device_t nb_dev, device_t dev, u32 port)
{
	u8  byte;
	u16 reg16;
	u32 reg32;
        u32 dev_ind;
	void set_pcie_reset();
	void set_pcie_dereset();
	//u8   is_dev3_present();

	struct southbridge_amd_rs780_config *cfg =
		&chip_info;
        _pci_break_tag(dev, NULL, &dev_ind, NULL);
	printk_info("rs780_gfx_init, nb_dev=0x%p, dev=0x%p, port=0x%x.\n",
		    nb_dev, dev, port);

	/* GFX Core Initialization */
	//if (port == 2) return;



	/* step 2, TMDS, (only need if CMOS option is enabled) */
	if (cfg->gfx_tmds) {
	}

#if 1				/* external clock mode */
	/* table 5-22, 5.9.1. REFCLK */
	/* 5.9.1.1. Disables the GFX REFCLK transmitter so that the GFX
	 * REFCLK PAD can be driven by an external source. */
	/* 5.9.1.2. Enables GFX REFCLK receiver to receive the REFCLK from an external source. */
	set_nbmisc_enable_bits(nb_dev, 0x38, 1 << 29 | 1 << 28, 0 << 29 | 1 << 28);

	/* 5.9.1.3 Selects the GFX REFCLK to be the source for PLL A. */
	/* 5.9.1.4 Selects the GFX REFCLK to be the source for PLL B. */
	/* 5.9.1.5 Selects the GFX REFCLK to be the source for PLL C. */
	set_nbmisc_enable_bits(nb_dev, 0x28, 3 << 6 | 3 << 8 | 3 << 10,
			       1 << 6 | 1 << 8 | 1 << 10);
	reg32 = nbmisc_read_index(nb_dev, 0x28);
	printk_info("misc 28 = %x\n", reg32);

	/* 5.9.1.6.Selects the single ended GFX REFCLK to be the source for core logic. */
	set_nbmisc_enable_bits(nb_dev, 0x6C, 1 << 31, 1 << 31);
#else				/* internal clock mode */
	/* table 5-23, 5.9.1. REFCLK */
	/* 5.9.1.1. Enables the GFX REFCLK transmitter so that the GFX
	 * REFCLK PAD can be driven by the SB REFCLK. */
	/* 5.9.1.2. Disables GFX REFCLK receiver from receiving the
	 * REFCLK from an external source.*/
	set_nbmisc_enable_bits(nb_dev, 0x38, 1 << 29 | 1 << 28, 1 << 29 | 0 << 28);

	/* 5.9.1.3 Selects the GFX REFCLK to be the source for PLL A. */
	/* 5.9.1.4 Selects the GFX REFCLK to be the source for PLL B. */
	/* 5.9.1.5 Selects the GFX REFCLK to be the source for PLL C. */
	set_nbmisc_enable_bits(nb_dev, 0x28, 3 << 6 | 3 << 8 | 3 << 10,
			       0);
	reg32 = nbmisc_read_index(nb_dev, 0x28);
	printk_info("misc 28 = %x\n", reg32);

	/* 5.9.1.6.Selects the single ended GFX REFCLK to be the source for core logic. */
	set_nbmisc_enable_bits(nb_dev, 0x6C, 1 << 31, 0 << 31);
#endif

	/* step 5.9.3, GFX overclocking, (only need if CMOS option is enabled) */
	/* 5.9.3.1. Increases PLL BW for 6G operation.*/
	/* set_nbmisc_enable_bits(nb_dev, 0x36, 0x3FF << 4, 0xB5 << 4); */
	/* skip */

	/* step 5.9.4, reset the GFX link */
	/* step 5.9.4.1 asserts both calibration reset and global reset */
	set_nbmisc_enable_bits(nb_dev, 0x8, 0x3 << 14, 0x3 << 14);

	/* step 5.9.4.2 de-asserts calibration reset */
	set_nbmisc_enable_bits(nb_dev, 0x8, 1 << 14, 0 << 14);

	/* step 5.9.4.3 wait for at least 200us */
	udelay(300);

	/* step 5.9.4.4 de-asserts global reset */
	set_nbmisc_enable_bits(nb_dev, 0x8, 1 << 15, 0 << 15);

	/* 5.9.5 Reset PCIE_GFX Slot */
	/* It is done in mainboard.c */
	set_pcie_reset();
	delay(1000);
	set_pcie_dereset();

	/* step 5.9.8 program PCIE memory mapped configuration space */
	/* done by enable_pci_bar3() before */

	/* step 7 compliance state, (only need if CMOS option is enabled) */
	/* the compliance stete is just for test. refer to 4.2.5.2 of PCIe specification */
	if (cfg->gfx_compliance) {
		/* force compliance */
		set_nbmisc_enable_bits(nb_dev, 0x32, 1 << 6, 1 << 6);
		/* release hold training for device 2. GFX initialization is done. */
		set_nbmisc_enable_bits(nb_dev, 0x8, 1 << 4, 0 << 4);
		dynamic_link_width_control(nb_dev, dev, cfg->gfx_link_width);
		printk_info("rs780_gfx_init step7.\n");
		return;
	}

	/* 5.9.12 Core Initialization. */
	/* 5.9.12.1 sets RCB timeout to be 25ms */
	/* 5.9.12.2. RCB Cpl timeout on link down. */
	set_pcie_enable_bits(dev, 0x70, 7 << 16 | 1 << 19, 4 << 16 | 1 << 19);
	printk_info("rs780_gfx_init step5.9.12.1.\n");

	/* step 5.9.12.3 disables slave ordering logic */
	set_pcie_enable_bits(nb_dev, 0x20, 1 << 8, 1 << 8);
	printk_info("rs780_gfx_init step5.9.12.3.\n");

	/* step 5.9.12.4 sets DMA payload size to 64 bytes */
	set_pcie_enable_bits(nb_dev, 0x10, 7 << 10, 4 << 10);
	/* 5.9.12.5. Blocks DMA traffic during C3 state. */
	set_pcie_enable_bits(dev, 0x10, 1 << 0, 0 << 0);

	/* 5.9.12.6. Disables RC ordering logic */
	set_pcie_enable_bits(nb_dev, 0x20, 1 << 9, 1 << 9);

	/* Enabels TLP flushing. */
	/* Note: It is got from RS690. The system will hang without this action. */
	set_pcie_enable_bits(dev, 0x20, 1 << 19, 0 << 19);

	/* 5.9.12.7. Ignores DLLPs during L1 so that txclk can be turned off */
	set_pcie_enable_bits(nb_dev, 0x2, 1 << 0, 1 << 0);

	/* 5.9.12.8 Prevents LC to go from L0 to Rcv_L0s if L1 is armed. */
	set_pcie_enable_bits(dev, 0xA1, 1 << 11, 1 << 11);

	/* 5.9.12.9 CMGOOD_OVERRIDE for end point initiated lane degradation. */
	set_nbmisc_enable_bits(nb_dev, 0x6a, 1 << 17, 1 << 17);
	printk_info("rs780_gfx_init step5.9.12.9.\n");

	/* 5.9.12.10 Sets the timer in Config state from 20us to */
	/* 5.9.12.11 De-asserts RX_EN in L0s. */
	/* 5.9.12.12 Enables de-assertion of PG2RX_CR_EN to lock clock
	 * recovery parameter when lane is in electrical idle in L0s.*/
	set_pcie_enable_bits(dev, 0xB1, 1 << 23 | 1 << 19 | 1 << 28, 1 << 23 | 1 << 19 | 1 << 28);

	/* 5.9.12.13. Turns off offset calibration. */
	/* 5.9.12.14. Enables Rx Clock gating in CDR */
	set_nbmisc_enable_bits(nb_dev, 0x34, 1 << 10/* | 1 << 22 */, 1 << 10/* | 1 << 22 */);

	/* 5.9.12.15. Sets number of TX Clocks to drain TX Pipe to 3. */
	set_pcie_enable_bits(dev, 0xA0, 0xF << 4, 3 << 4);

	/* 5.9.12.16. Lets PI use Electrical Idle from PHY when
	 * turning off PLL in L1 at Gen2 speed instead Inferred Electrical Idle. */
	set_pcie_enable_bits(nb_dev, 0x40, 3 << 14, 2 << 14);

	/* 5.9.12.17. Prevents the Electrical Idle from causing a transition from Rcv_L0 to Rcv_L0s. */
	set_pcie_enable_bits(dev, 0xB1, 1 << 20, 1 << 20);

	/* 5.9.12.18. Prevents the LTSSM from going to Rcv_L0s if it has already
	 * acknowledged a request to go to L1. */
	set_pcie_enable_bits(dev, 0xA1, 1 << 11, 1 << 11);

	/* 5.9.12.19. LDSK only taking deskew on deskewing error detect */
	set_pcie_enable_bits(nb_dev, 0x40, 1 << 28, 0 << 28);

	/* 5.9.12.20. Bypasses lane de-skew logic if in x1 */
	set_pcie_enable_bits(nb_dev, 0xC2, 1 << 14, 1 << 14);

	/* 5.9.12.21. Sets Electrical Idle Threshold. */
	set_nbmisc_enable_bits(nb_dev, 0x35, 3 << 21, 2 << 21);

	/* 5.9.12.22. Advertises -6 dB de-emphasis value in TS1 Data Rate Identifier
	 * Only if CMOS Option in section. skip */

	/* 5.9.12.23. Disables GEN2 capability of the device. */
	set_pcie_enable_bits(dev, 0xA4, 1 << 0, 0 << 0);

	/* 5.9.12.24.Disables advertising Upconfigure Support. */
	set_pcie_enable_bits(dev, 0xA2, 1 << 13, 1 << 13);

	/* 5.9.12.25. No comment in RPR. */
	set_nbmisc_enable_bits(nb_dev, 0x39, 1 << 10, 0 << 10);

	/* 5.9.12.26. This capacity is required since links wider than x1 and/or multiple link
	 * speed are supported */
	set_pcie_enable_bits(nb_dev, 0xC1, 1 << 0, 1 << 0);

	/* 5.9.12.27. Enables NVG86 ECO. A13 above only. */
	/* TODO: Check if it is A13. */
	if (0)			/* A12 */
		set_pcie_enable_bits(dev, 0x02, 1 << 11, 1 << 11);

	/* 5.9.12.28 Hides and disables the completion timeout method. */
	set_pcie_enable_bits(nb_dev, 0xC1, 1 << 2, 0 << 2);

	/* 5.9.12.29. Use the bif_core de-emphasis strength by default. */
	/* set_nbmisc_enable_bits(nb_dev, 0x36, 1 << 28, 1 << 28); */

	/* 5.9.12.30. Set TX arbitration algorithm to round robin */
	set_pcie_enable_bits(nb_dev, 0x1C,
			     1 << 0 | 0x1F << 1 | 0x1F << 6,
			     1 << 0 | 0x04 << 1 | 0x04 << 6);

	pcie_commoncoreinit(nb_dev, dev);
	pcie_commonportinit(nb_dev, dev);
	pcie_initgen2(nb_dev, dev);
	pcie_gen2workaround(nb_dev, dev);


	/* Single-port/Dual-port configureation. */
	switch (cfg->gfx_dual_slot) {
	case 0:
		/* step 1, lane reversal (only need if CMOS option is enabled) */
		if (cfg->gfx_lane_reversal) {
			set_nbmisc_enable_bits(nb_dev, 0x33, 1 << 2, 1 << 2);
		}
		printk_info("rs780_gfx_init step1.\n");
		printk_info("rs780_gfx_init step2.\n");

		printk_info("device = %x\n", dev_ind);
		if(dev_ind == 2)
			single_port_configuration(nb_dev, dev);
		else{
			set_nbmisc_enable_bits(nb_dev, 0xc, 0x2 << 2, 0x2 << 2); /* hide the GFX bridge. */
			printk_info("If dev3.., single port. Do nothing.\n");
		}

		break;
	case 1:
		/* step 1, lane reversal (only need if CMOS option is enabled) */
		if (cfg->gfx_lane_reversal) {
			set_nbmisc_enable_bits(nb_dev, 0x33, 1 << 2, 1 << 2);
			set_nbmisc_enable_bits(nb_dev, 0x33, 1 << 3, 1 << 3);
		}
		printk_info("rs780_gfx_init step1.\n");
		/* step 1.1, dual-slot gfx configuration (only need if CMOS option is enabled) */
		/* AMD calls the configuration CrossFire */
		set_nbmisc_enable_bits(nb_dev, 0x0, 0xf << 8, 5 << 8);
		printk_info("rs780_gfx_init step2.\n");

		printk_info("device = %x\n", dev_ind);
		dual_port_configuration(nb_dev, dev);
		break;
	default:
		printk_info("Incorrect configuration of external gfx slot.\n");
		break;
	}
}
void rs780_gfx_3_init(device_t nb_dev, device_t dev, u32 port)
{
	u8  byte;
	u16 reg16;
	u32 reg32;
        u32 dev_ind;
        u8 result, width;
	void set_pcie_reset();
	void set_pcie_dereset();
	//u8   is_dev3_present();

	struct southbridge_amd_rs780_config *cfg =
		&chip_info;
        _pci_break_tag(dev, NULL, &dev_ind, NULL);
	printk_info("rs780_gfx_init, nb_dev=0x%p, dev=0x%p, port=0x%x.\n",
		    nb_dev, dev, port);

	/* GFX Core Initialization */
	//if (port == 2) return;



	/* step 2, TMDS, (only need if CMOS option is enabled) */
	if (cfg->gfx_tmds) {
	}

//#if 1				/* external clock mode */
	/* table 5-22, 5.9.1. REFCLK */
	/* 5.9.1.1. Disables the GFX REFCLK transmitter so that the GFX
	 * REFCLK PAD can be driven by an external source. */
	/* 5.9.1.2. Enables GFX REFCLK receiver to receive the REFCLK from an external source. */
	//set_nbmisc_enable_bits(nb_dev, 0x38, 1 << 29 | 1 << 28, 0 << 29 | 1 << 28);

	/* 5.9.1.3 Selects the GFX REFCLK to be the source for PLL A. */
	/* 5.9.1.4 Selects the GFX REFCLK to be the source for PLL B. */
	/* 5.9.1.5 Selects the GFX REFCLK to be the source for PLL C. */
	//set_nbmisc_enable_bits(nb_dev, 0x28, 3 << 6 | 3 << 8 | 3 << 10,
			    //   1 << 6 | 1 << 8 | 1 << 10);
	//reg32 = nbmisc_read_index(nb_dev, 0x28);
	//printk_info("misc 28 = %x\n", reg32);

	/* 5.9.1.6.Selects the single ended GFX REFCLK to be the source for core logic. */
	//set_nbmisc_enable_bits(nb_dev, 0x6C, 1 << 31, 1 << 31);
//#else				/* internal clock mode */
	/* table 5-23, 5.9.1. REFCLK */
	/* 5.9.1.1. Enables the GFX REFCLK transmitter so that the GFX
	 * REFCLK PAD can be driven by the SB REFCLK. */
	/* 5.9.1.2. Disables GFX REFCLK receiver from receiving the
	 * REFCLK from an external source.*/
	//set_nbmisc_enable_bits(nb_dev, 0x38, 1 << 29 | 1 << 28, 1 << 29 | 0 << 28);

	/* 5.9.1.3 Selects the GFX REFCLK to be the source for PLL A. */
	/* 5.9.1.4 Selects the GFX REFCLK to be the source for PLL B. */
	/* 5.9.1.5 Selects the GFX REFCLK to be the source for PLL C. */
	//set_nbmisc_enable_bits(nb_dev, 0x28, 3 << 6 | 3 << 8 | 3 << 10,
	//		       0);
	//reg32 = nbmisc_read_index(nb_dev, 0x28);
	//printk_info("misc 28 = %x\n", reg32);

	/* 5.9.1.6.Selects the single ended GFX REFCLK to be the source for core logic. */
	//set_nbmisc_enable_bits(nb_dev, 0x6C, 1 << 31, 0 << 31);
//#endif

	/* step 5.9.3, GFX overclocking, (only need if CMOS option is enabled) */
	/* 5.9.3.1. Increases PLL BW for 6G operation.*/
	/* set_nbmisc_enable_bits(nb_dev, 0x36, 0x3FF << 4, 0xB5 << 4); */
	/* skip */

	/* step 5.9.4, reset the GFX link */
	/* step 5.9.4.1 asserts both calibration reset and global reset */
	//set_nbmisc_enable_bits(nb_dev, 0x8, 0x3 << 14, 0x3 << 14);

	/* step 5.9.4.2 de-asserts calibration reset */
	//set_nbmisc_enable_bits(nb_dev, 0x8, 1 << 14, 0 << 14);

	/* step 5.9.4.3 wait for at least 200us */
	//udelay(300);

	/* step 5.9.4.4 de-asserts global reset */
	//set_nbmisc_enable_bits(nb_dev, 0x8, 1 << 15, 0 << 15);

	/* 5.9.5 Reset PCIE_GFX Slot */
	/* It is done in mainboard.c */
	set_pcie_reset();
	delay(1000);
	set_pcie_dereset();

	/* step 5.9.8 program PCIE memory mapped configuration space */
	/* done by enable_pci_bar3() before */

	/* step 7 compliance state, (only need if CMOS option is enabled) */
	/* the compliance stete is just for test. refer to 4.2.5.2 of PCIe specification */
	if (cfg->gfx_compliance) {
		/* force compliance */
		//set_nbmisc_enable_bits(nb_dev, 0x32, 1 << 6, 1 << 6);
		/* release hold training for device 2. GFX initialization is done. */
		//set_nbmisc_enable_bits(nb_dev, 0x8, 1 << 4, 0 << 4);
		dynamic_link_width_control(nb_dev, dev, cfg->gfx_link_width);
		printk_info("rs780_gfx_init step7.\n");
		return;
	}

	/* 5.9.12 Core Initialization. */
	/* 5.9.12.1 sets RCB timeout to be 25ms */
	/* 5.9.12.2. RCB Cpl timeout on link down. */
	set_pcie_enable_bits(dev, 0x70, 7 << 16 | 1 << 19, 4 << 16 | 1 << 19);
	printk_info("rs780_gfx_init step5.9.12.1.\n");

	/* step 5.9.12.3 disables slave ordering logic */
	//set_pcie_enable_bits(nb_dev, 0x20, 1 << 8, 1 << 8);
	//printk_info("rs780_gfx_init step5.9.12.3.\n");

	/* step 5.9.12.4 sets DMA payload size to 64 bytes */
	//set_pcie_enable_bits(nb_dev, 0x10, 7 << 10, 4 << 10);
	/* 5.9.12.5. Blocks DMA traffic during C3 state. */
	set_pcie_enable_bits(dev, 0x10, 1 << 0, 0 << 0);

	/* 5.9.12.6. Disables RC ordering logic */
	//set_pcie_enable_bits(nb_dev, 0x20, 1 << 9, 1 << 9);

	/* Enabels TLP flushing. */
	/* Note: It is got from RS690. The system will hang without this action. */
	set_pcie_enable_bits(dev, 0x20, 1 << 19, 0 << 19);

	/* 5.9.12.7. Ignores DLLPs during L1 so that txclk can be turned off */
	//set_pcie_enable_bits(nb_dev, 0x2, 1 << 0, 1 << 0);

	/* 5.9.12.8 Prevents LC to go from L0 to Rcv_L0s if L1 is armed. */
	set_pcie_enable_bits(dev, 0xA1, 1 << 11, 1 << 11);

	/* 5.9.12.9 CMGOOD_OVERRIDE for end point initiated lane degradation. */
	//set_nbmisc_enable_bits(nb_dev, 0x6a, 1 << 17, 1 << 17);
	//printk_info("rs780_gfx_init step5.9.12.9.\n");

	/* 5.9.12.10 Sets the timer in Config state from 20us to */
	/* 5.9.12.11 De-asserts RX_EN in L0s. */
	/* 5.9.12.12 Enables de-assertion of PG2RX_CR_EN to lock clock
	 * recovery parameter when lane is in electrical idle in L0s.*/
	set_pcie_enable_bits(dev, 0xB1, 1 << 23 | 1 << 19 | 1 << 28, 1 << 23 | 1 << 19 | 1 << 28);

	/* 5.9.12.13. Turns off offset calibration. */
	/* 5.9.12.14. Enables Rx Clock gating in CDR */
	//set_nbmisc_enable_bits(nb_dev, 0x34, 1 << 10/* | 1 << 22 */, 1 << 10/* | 1 << 22 */);

	/* 5.9.12.15. Sets number of TX Clocks to drain TX Pipe to 3. */
	set_pcie_enable_bits(dev, 0xA0, 0xF << 4, 3 << 4);

	/* 5.9.12.16. Lets PI use Electrical Idle from PHY when
	 * turning off PLL in L1 at Gen2 speed instead Inferred Electrical Idle. */
	//set_pcie_enable_bits(nb_dev, 0x40, 3 << 14, 2 << 14);

	/* 5.9.12.17. Prevents the Electrical Idle from causing a transition from Rcv_L0 to Rcv_L0s. */
	set_pcie_enable_bits(dev, 0xB1, 1 << 20, 1 << 20);

	/* 5.9.12.18. Prevents the LTSSM from going to Rcv_L0s if it has already
	 * acknowledged a request to go to L1. */
	set_pcie_enable_bits(dev, 0xA1, 1 << 11, 1 << 11);

	/* 5.9.12.19. LDSK only taking deskew on deskewing error detect */
	//set_pcie_enable_bits(nb_dev, 0x40, 1 << 28, 0 << 28);

	/* 5.9.12.20. Bypasses lane de-skew logic if in x1 */
	//set_pcie_enable_bits(nb_dev, 0xC2, 1 << 14, 1 << 14);

	/* 5.9.12.21. Sets Electrical Idle Threshold. */
	//set_nbmisc_enable_bits(nb_dev, 0x35, 3 << 21, 2 << 21);

	/* 5.9.12.22. Advertises -6 dB de-emphasis value in TS1 Data Rate Identifier
	 * Only if CMOS Option in section. skip */

	/* 5.9.12.23. Disables GEN2 capability of the device. */
	set_pcie_enable_bits(dev, 0xA4, 1 << 0, 0 << 0);

	/* 5.9.12.24.Disables advertising Upconfigure Support. */
	set_pcie_enable_bits(dev, 0xA2, 1 << 13, 1 << 13);

	/* 5.9.12.25. No comment in RPR. */
	//set_nbmisc_enable_bits(nb_dev, 0x39, 1 << 10, 0 << 10);

	/* 5.9.12.26. This capacity is required since links wider than x1 and/or multiple link
	 * speed are supported */
	//set_pcie_enable_bits(nb_dev, 0xC1, 1 << 0, 1 << 0);

	/* 5.9.12.27. Enables NVG86 ECO. A13 above only. */
	/* TODO: Check if it is A13. */
	if (0)			/* A12 */
		set_pcie_enable_bits(dev, 0x02, 1 << 11, 1 << 11);

	/* 5.9.12.28 Hides and disables the completion timeout method. */
	//set_pcie_enable_bits(nb_dev, 0xC1, 1 << 2, 0 << 2);

	/* 5.9.12.29. Use the bif_core de-emphasis strength by default. */
	/* set_nbmisc_enable_bits(nb_dev, 0x36, 1 << 28, 1 << 28); */

	/* 5.9.12.30. Set TX arbitration algorithm to round robin */
	//set_pcie_enable_bits(nb_dev, 0x1C,
			//     1 << 0 | 0x1F << 1 | 0x1F << 6,
			 //    1 << 0 | 0x04 << 1 | 0x04 << 6);




	/* Single-port/Dual-port configureation. */
	switch (cfg->gfx_dual_slot) {
	case 0:
		/* step 1, lane reversal (only need if CMOS option is enabled) */
		if (cfg->gfx_lane_reversal) {
			set_nbmisc_enable_bits(nb_dev, 0x33, 1 << 2, 1 << 2);
		}
		printk_info("rs780_gfx_init step1.\n");
		printk_info("rs780_gfx_init step2.\n");

		printk_info("device = %x\n", dev_ind);
		if(dev_ind == 2)
			single_port_configuration(nb_dev, dev);
		else{
			set_nbmisc_enable_bits(nb_dev, 0xc, 0x2 << 2, 0x2 << 2); /* hide the GFX bridge. */
			printk_info("If dev3.., single port. Do nothing.\n");
		}

		break;
	case 1:
	/* This function will be called twice, so we dont have to do dev 3 here. */
	/* step 17: Training for Device 3 */
	//set_nbmisc_enable_bits(nb_dev, 0x8, 1 << 5, 0 << 5);
	/* Releases hold training for GFX port 0 (device 3) */
	PcieReleasePortTraining(nb_dev, dev, 3);
	/* PCIE Link Training Sequence */
	result = PcieTrainPort(nb_dev, dev, 3);

	/*step 18: Power Down Control for Device 3 */
	/* step 18.a Link Training was NOT successful */
	if (!result) {
		/* Powers down all lanes for port B and PLL1 */
		nbpcie_ind_write_index(nb_dev, 0x65, 0xccf0f0);
	} else {		/* step 18.b Link Training was successful */

		reg32 = nbpcie_p_read_index(dev, 0xa2);
		width = (reg32 >> 4) & 0x7;
		printk_debug("GFX LC_LINK_WIDTH = 0x%x.\n", width);
		switch (width) {
		case 1:
		case 2:
			nbpcie_ind_write_index(nb_dev, 0x65,
					       cfg->gfx_lane_reversal ? 0x7070 : 0xe0e0);
			break;
		case 4:
			nbpcie_ind_write_index(nb_dev, 0x65,
					       cfg->gfx_lane_reversal ? 0x3030 : 0x0f0f);
			break;
		}
	}
		break;
	default:
		printk_info("Incorrect configuration of external gfx slot.\n");
		break;
	}
}
#endif
