#include <sys/linux/types.h>
#include <pmon.h>
#include <stdio.h>
#include <string.h>
#include <machine/frame.h>

#define CONFIG_PAGE_SIZE_4KB
#include "../../../cmds/mipsregs.h"
#include "../../../cmds/bootparam.h"

#define PRID_REV_LOONGSON1B     0x0020
#define PRID_REV_LOONGSON2E     0x0002
#define PRID_REV_LOONGSON2F     0x0003
#define PRID_REV_LOONGSON3A_R1  0x0005
#define PRID_REV_LOONGSON3B_R1  0x0006 
#define PRID_REV_LOONGSON3B_R2  0x0007
#define PRID_REV_LOONGSON3A_R2  0x0008 
#define PRID_REV_LOONGSON3A_R3_0        0x0009
#define PRID_REV_LOONGSON3A_R3_1        0x000D 
#define PRID_REV_LOONGSON2K_R1  0x0001
#define PRID_REV_LOONGSON2K_R2  0x0003

#define HT_uncache_enable_reg0	0x90000efdfb0000f0ULL 
#define HT_uncache_base_reg0	0x90000efdfb0000f4ULL 
#define HT_uncache_enable_reg1	0x90000efdfb0000f8ULL 
#define HT_uncache_base_reg1	0x90000efdfb0000fcULL 
#define HT_uncache_enable_reg2	0x90000efdfb000168ULL 
#define HT_uncache_base_reg2	0x90000efdfb00016cULL 
#define HT_uncache_enable_reg3	0x90000efdfb000170ULL 
#define HT_uncache_base_reg3	0x90000efdfb000174ULL 
#define HT_cache_enable_reg1	0x90000efdfb000068ULL
#define HT_cache_base_reg1	0x90000efdfb00006cULL

extern u64 __raw__readq(u64 addr);
extern u64 __raw__writeq(u64 addr, u64 val);

static u32 __raw__readl(u64 addr)
{
	if(addr & 0x4ULL)
		return (u32)(__raw__readq(addr - 0x4ULL) >> 32);
	else
		return (u32)(__raw__readq(addr) & 0xffffffff);
}

static void __raw__writel(u64 addr, u32 val)
{
	u64 new_val;
	if(addr & 0x4ULL) {
		new_val = val;
		new_val = new_val << 32 | (__raw__readq(addr - 0x4ULL) & 0xffffffff);
		__raw__writeq(addr - 0x4ULL, new_val);
	}else {
		new_val = val | (__raw__readq(addr) & ~0xffffffffULL);
		__raw__writeq(addr, new_val);
	}
}

void cfg_coherent(int ac, char *av[])
{
	int i, prid;
	u16 nocoherent = 0;
	struct boot_params * bp;
	struct irq_source_routing_table * irq_tb;

	bp = (struct boot_params *)(cpuinfotab[whatcpu]->a2);

#ifndef LS7A_2WAY_CONNECT
	prid = read_c0_prid() & 0xf;

	if(prid == PRID_REV_LOONGSON3A_R2 || prid == PRID_REV_LOONGSON3A_R3_0)
		nocoherent = 1;
	for (i = 0; i < ac; i++) {
		if(strstr(av[i], "cached"))
			nocoherent = 0;
		if(strstr(av[i], "uncached"))
			nocoherent = 1;
	}

	printf("set %scoherent\n", nocoherent ? "no":"");
	if(nocoherent) {
		/*	uncache		*/
#ifdef LS7A
		__raw__writel(HT_uncache_enable_reg0	, __raw__readl(HT_cache_enable_reg1)); //for 7a gpu
		__raw__writel(HT_uncache_base_reg0	, __raw__readl(HT_cache_base_reg1));
#else
		__raw__writel(HT_uncache_enable_reg0	, 0xc0000000); //Low 256M
		__raw__writel(HT_uncache_base_reg0	, 0x0080fff0);
#endif

		__raw__writel(HT_uncache_enable_reg1	, 0xc0000000); //Node 0
		__raw__writel(HT_uncache_base_reg1	, 0x0000e000);
		__raw__writel(HT_uncache_enable_reg2	, 0xc0100000); //Node 1
		__raw__writel(HT_uncache_base_reg2	, 0x2000e000);
		__raw__writel(HT_uncache_enable_reg3	, 0xc0200000); //Node 2/3
		__raw__writel(HT_uncache_base_reg3	, 0x4000c000);
		__raw__writeq(0x900000003ff02708ULL, 0x0000202000000000ULL);
		__raw__writeq(0x900000003ff02748ULL, 0xffffffe000000000ULL);
		__raw__writeq(0x900000003ff02788ULL, 0x0000300000000086ULL);
	} else
#endif
	{
		/*	cache		*/
		__raw__writel(HT_uncache_enable_reg0	, 0x0);
		__raw__writel(HT_uncache_enable_reg1	, 0x0);
		__raw__writel(HT_uncache_enable_reg2	, 0x0);
		__raw__writel(HT_uncache_enable_reg3	, 0x0);
	}
	irq_tb = (struct irq_source_routing_table *)((u64)&(bp->efi.smbios.lp) + bp->efi.smbios.lp.irq_offset);
	irq_tb->dma_noncoherent = nocoherent;
}
