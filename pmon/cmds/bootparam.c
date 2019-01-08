#include <stdlib.h>
#include <stdio.h>
#include "bootparam.h"
#include "../common/smbios/smbios.h"

#ifdef LS3A2H_STR
#define LS3A2H_STR_FUNC_ADDR 0xffffffffbfc00500
#endif

struct loongson_params  g_lp = { 0 };
struct efi_memory_map_loongson g_map = { 0 };
struct efi_cpuinfo_loongson g_cpuinfo_loongson = { 0 };
struct system_loongson g_sysitem = { 0 };
struct irq_source_routing_table g_irq_source = { 0 };
struct interface_info g_interface = { 0 };
struct interface_info g_board = { 0 };
struct loongson_special_attribute g_special = { 0 };
#ifdef LS7A
unsigned char readspi_result[128 * 1024] = {0};
#include "../../Targets/Bonito3a3000_7a/dev/7aVbios.h"
#endif

extern void poweroff_kernel(void);
extern void reboot_kernel(void);
#ifdef RS780E
extern unsigned char vgarom[];
extern struct pci_device *vga_dev;
#endif

struct board_devices *board_devices_info();
struct interface_info *init_interface_info();
struct irq_source_routing_table *init_irq_source();
struct system_loongson *init_system_loongson();
struct efi_cpuinfo_loongson *init_cpu_info();
struct efi_memory_map_loongson * init_memory_map();
void init_loongson_params(struct loongson_params *lp);
void init_smbios(struct smbios_tables *smbios);
void init_efi(struct efi_loongson *efi);
struct loongson_special_attribute *init_special_info();
void init_reset_system(struct efi_reset_system_t *reset);

int init_boot_param(struct boot_params *bp)
{
  
  init_efi(&(bp->efi));
  init_reset_system(&(bp->reset_system));


  return bp;
}

void init_efi(struct efi_loongson *efi)
{
    init_smbios(&(efi->smbios));
}

void init_reset_system(struct efi_reset_system_t *reset)
{
  reset->Shutdown = &poweroff_kernel;
  reset->ResetWarm = &reboot_kernel;
#ifdef LS3A2H_STR
  reset->ResetCold = LS3A2H_STR_FUNC_ADDR;
#endif
#ifdef LOONGSON_3ASINGLE  
  reset->DoSuspend = 0xffffffffbfc00500;
#endif
}

extern struct pci_device * pcie_dev ;
void init_smbios(struct smbios_tables *smbios)
{
  smbios->vers = 0;
#ifdef RS780E
  if(vga_dev != NULL){
  smbios->vga_bios = vgarom;
 }else
  smbios->vga_bios = 0;
#elif defined LS7A
  if(!pcie_dev){
	  extern unsigned char * ls7a_spi_read_vgabios();
	  ls7a_spi_read_vgabios(readspi_result);
	  if(!ls7a_vgabios_crc_check(readspi_result))
		  smbios->vga_bios = readspi_result;
	  else
	      smbios->vga_bios = Vbios;
  }
  else
	      smbios->vga_bios = 0;
#else
  smbios->vga_bios = 0;
#endif
  init_loongson_params(&(smbios->lp)); 

}


void init_loongson_params(struct loongson_params *lp)
{

  lp->memory_offset = (unsigned long long)init_memory_map() - (unsigned long long)lp;
  lp->cpu_offset = (unsigned long long)init_cpu_info() - (unsigned long long)lp; 
  lp->system_offset = (unsigned long long)init_system_loongson() - (unsigned long long)lp;
  lp->irq_offset = (unsigned long long)init_irq_source() - (unsigned long long)lp; 
  lp->interface_offset = (unsigned long long)init_interface_info() - (unsigned long long)lp;
  lp->boarddev_table_offset = (unsigned long long)board_devices_info() - (unsigned long long)lp;
  lp->special_offset = (unsigned long long)init_special_info() - (unsigned long long)lp; 

  printf("memory_offset = 0x%llx;cpu_offset = 0x%llx; system_offset = 0x%llx; irq_offset = 0x%llx; interface_offset = 0x%llx;\n",lp->memory_offset,lp->cpu_offset,lp->system_offset,lp->irq_offset, lp->interface_offset);
}

/*struct efi_memory_map_loongson * init_memory_map()*/
/*{*/
  /*struct efi_memory_map_loongson *emap = &g_map;*/


  /*//map->mem_start_addr = 0x80000000;*/
/*#if (defined(LOONGSON_3BSERVER))*/
  /*emap->nr_map = 10; */
/*#else*/
  /*emap->nr_map = 6; */
/*#endif*/

  /*emap->mem_freq = 300000000; //300M*/
  /*//map->memsz_high = atoi(getenv("highmemsize"));*/
  /*//map->mem_size = atoi(getenv("memsize"));*/
  /*//map->memsz_reserved = 16;*/

/*#if 1*/
 
  /*emap->map[0].node_id = 0;*/
  /*//strcpy(emap->map[0].mem_name, "node0_low");*/
  /*emap->map[0].mem_type = 1;*/
  /*emap->map[0].mem_start = 0x01000000;*/
  /*emap->map[0].mem_size = atoi(getenv("memsize"));*/

  /*emap->map[1].node_id = 0;*/
  /*//strcpy(emap->map[1].mem_name, "node0_high");*/
  /*emap->map[1].mem_type = 2;*/
/*#if (defined LOONGSON_3A2H) || (defined LOONGSON_3C2H)*/
  /*emap->map[1].mem_start = 0x110000000;*/
/*#else*/
  /*emap->map[1].mem_start = 0x90000000;*/
/*#endif*/
  /*emap->map[1].mem_size = atoi(getenv("highmemsize"));*/
  

/*//support smbios*/
  /*emap->map[2].node_id = 0;*/
  /*emap->map[2].mem_type = 10;*/
  /*emap->map[2].mem_start = SMBIOS_PHYSICAL_ADDRESS;*/
 
  /*emap->map[3].node_id = 0;*/
  /*emap->map[3].mem_type = 3;*/
  /*emap->map[3].mem_start = SMBIOS_PHYSICAL_ADDRESS & 0x0fffffff;*/
  /*emap->map[3].mem_size = SMBIOS_SIZE_LIMIT >> 20;*/

/*#if (defined(MULTI_CHIP)) || (defined(LOONGSON_3BSINGLE))*/

  /*emap->map[5].mem_size = atoi(getenv("memorysize_high_n1"));*/
  /*if ( emap->map[5].mem_size != 0 )*/
  /*{*/
	/*emap->map[4].node_id = 1;*/
	/*//strcpy(emap->map[0].mem_name, "node0_low");*/
	/*emap->map[4].mem_type = 1;*/
	/*emap->map[4].mem_start = 0x01000000;*/
	/*emap->map[4].mem_size = atoi(getenv("memsize"));*/

	/*emap->map[5].node_id = 1;*/
	/*//strcpy(emap->map[1].mem_name, "node0_high");*/
	/*emap->map[5].mem_type = 2;*/
	/*emap->map[5].mem_start = 0x90000000;*/
  /*}*/

/*#endif*/

/*#if (defined(LOONGSON_3BSERVER))*/

  /*emap->map[5].mem_size = atoi(getenv("memorysize_high_n1"));*/
  /*if ( emap->map[5].mem_size != 0 )*/
  /*{*/
	/*emap->map[4].node_id = 1;*/
	/*//strcpy(emap->map[0].mem_name, "node0_low");*/
	/*emap->map[4].mem_type = 1;*/
	/*emap->map[4].mem_start = 0x01000000;*/
	/*emap->map[4].mem_size = atoi(getenv("memsize"));*/

	/*emap->map[5].node_id = 1;*/
	/*//strcpy(emap->map[1].mem_name, "node0_high");*/
	/*emap->map[5].mem_type = 2;*/
	/*emap->map[5].mem_start = 0x90000000;*/
  /*}*/

  /*emap->map[7].mem_size = atoi(getenv("memorysize_high_n2"));*/
  /*if ( emap->map[7].mem_size != 0 )*/
  /*{*/
	/*emap->map[6].node_id = 2;*/
	/*//strcpy(emap->map[0].mem_name, "node0_low");*/
	/*emap->map[6].mem_type = 1;*/
	/*emap->map[6].mem_start = 0x01000000;*/
	/*emap->map[6].mem_size = atoi(getenv("memsize"));*/

	/*emap->map[7].node_id = 2;*/
	/*//strcpy(emap->map[1].mem_name, "node0_high");*/
	/*emap->map[7].mem_type = 2;*/
	/*emap->map[7].mem_start = 0x90000000;*/
  /*}*/

  /*emap->map[9].mem_size = atoi(getenv("memorysize_high_n3"));*/
  /*if ( emap->map[9].mem_size != 0 )*/
  /*{*/
	/*emap->map[8].node_id = 3;*/
	/*//strcpy(emap->map[0].mem_name, "node0_low");*/
	/*emap->map[8].mem_type = 1;*/
	/*emap->map[8].mem_start = 0x01000000;*/
	/*emap->map[8].mem_size = atoi(getenv("memsize"));*/

	/*emap->map[9].node_id = 3;*/
	/*//strcpy(emap->map[1].mem_name, "node0_high");*/
	/*emap->map[9].mem_type = 2;*/
	/*emap->map[9].mem_start = 0x90000000;*/
  /*}*/

/*#endif*/
/*#endif*/


  /*return emap;*/
/*}*/

#define readl(addr)             ((*(volatile unsigned int *)((long)(addr))))

#ifdef LOONGSON_3BSINGLE
#ifdef LOONGSON_3B1500
  #define PRID_IMP_LOONGSON    0x6307
#else
  #define PRID_IMP_LOONGSON    0x6306
#endif
 enum loongson_cpu_type cputype = Legacy_3B;
#endif
#ifdef LOONGSON_3BSERVER
  #define PRID_IMP_LOONGSON    0x6306
  enum loongson_cpu_type cputype = Legacy_3B;
#endif
#if  defined ( LOONGSON_3ASINGLE) || defined ( LOONGSON_3A2H) || defined(LOONGSON_2K)
  #define PRID_IMP_LOONGSON    0x6305
  enum loongson_cpu_type cputype = Legacy_3A;
#endif
#ifdef LOONGSON_3ASERVER
  #define PRID_IMP_LOONGSON    0x6305
  enum loongson_cpu_type cputype = Legacy_3A;
#endif
#ifdef LOONGSON_3A84W
  #define PRID_IMP_LOONGSON    0x146308
  enum loongson_cpu_type cputype = Legacy_3A;
#endif
#if defined(LOONGSON_2G5536)||defined(LOONGSON_2G1A)
  #define PRID_IMP_LOONGSON    0x6305
  enum loongson_cpu_type cputype = Legacy_2G;
#endif
#ifdef LOONGSON_2F1A
  #define PRID_IMP_LOONGSON    0x6303
  enum loongson_cpu_type cputype = Legacy_2F;
#endif

struct efi_cpuinfo_loongson *init_cpu_info()
{
  struct efi_cpuinfo_loongson *c = &g_cpuinfo_loongson;
  unsigned int available_core_mask = 0;
  unsigned int available = 0;

  c->processor_id = PRID_IMP_LOONGSON;
  c->cputype  = cputype;

  c->cpu_clock_freq = atoi(getenv("cpuclock"));

#ifdef LOONGSON_3BSERVER
  c->total_node = 4; // total node means what? why it can't be 8 ? // by xqch
  c->nr_cpus = 16;
#endif
#ifdef LOONGSON_3ASERVER
  c->total_node = 4;
  c->nr_cpus = 8;
#endif
#ifdef LOONGSON_3BSINGLE
  c->total_node = 4;
  c->nr_cpus = 8;
#endif

#ifdef LOONGSON_3A84W
  c->total_node = 4;
  c->nr_cpus    = 16;
#endif

#ifdef LOONGSON_3A2H
  c->total_node = 1;
  c->nr_cpus = 4;
#endif

#ifdef LOONGSON_2K
  c->total_node = 1;
  c->nr_cpus = 2;
#endif

#ifdef LOONGSON_3ASINGLE
  c->total_node = 1;
  c->nr_cpus = 4;
  available_core_mask = (readl((void *)0xbfe00194ull) & 0xf0000);
  available = ((available_core_mask >> 16) & 0xf);
  while(available)
  {
        c->nr_cpus -= available & 0x1;
        available >>= 1;
  }
#endif

#if defined(LOONGSON_2G5536)||defined(LOONGSON_2G1A) || defined(LOONGSON_2F1A)
	c->total_node = 1;
	c->nr_cpus = 1;
#endif
#ifdef	BOOTCORE_ID
	c->cpu_startup_core_id = BOOTCORE_ID;
	c->reserved_cores_mask = RESERVED_COREMASK;
	board_info_fixup(c);
#else
	c->cpu_startup_core_id = 0;
	c->reserved_cores_mask = 0;
#endif


return c;
}

struct system_loongson *init_system_loongson()
{
 struct system_loongson *s = &g_sysitem;
  s->ccnuma_smp = 1;
  s->vers = 2;
#ifdef LOONGSON_3ASERVER
  s->ccnuma_smp = 1;
  s->sing_double_channel = 2;
#endif
#ifdef LOONGSON_3BSERVER
  s->ccnuma_smp = 1;
  s->sing_double_channel = 2; // means what?
#endif
#ifdef LOONGSON_3A84W
  s->ccnuma_smp = 1;
  s->sing_double_channel = 4; // means what?
#endif
#
#ifdef LOONGSON_3BSINGLE
  s->ccnuma_smp = 1;
  s->sing_double_channel = 1;
#endif
#ifdef LOONGSON_3ASINGLE
  s->ccnuma_smp = 0;
  s->sing_double_channel = 1;
#endif
#ifdef LOONGSON_3A2H
  s->ccnuma_smp = 0;
  s->sing_double_channel = 1;
#endif
#if defined(LOONGSON_2G5536)||defined(LOONGSON_2G1A) || defined(LOONGSON_2F1A)
  s->ccnuma_smp = 0;
  s->sing_double_channel = 1;
#endif

#ifdef loongson3A3
{
#ifdef	BONITO_33M
 int clk = 33333333;
#elif  defined(BONITO_25M)
 int clk = 25000000;
#else
 int clk = 50000000;
#endif
 struct uart_device uart0 = { 2, clk, 2, (int)0xbfe001e0 };
 s->nr_uarts = 1;
 s->uarts[0] = uart0;
}
#endif

  return s;
}

enum loongson_irq_source_enum
{
  HT,I8259,UNKNOWN
};



struct irq_source_routing_table *init_irq_source()
{

 struct irq_source_routing_table *irq_info = &g_irq_source ;

	
	irq_info->PIC_type = HT;


#ifdef LOONGSON_3BSINGLE
	irq_info->ht_int_bit = 1<<16;
#else
	irq_info->ht_int_bit = 1<<24;
#endif
	irq_info->ht_enable = 0x0000d17b;

#ifdef LOONGSON_3BSINGLE
	irq_info->node_id = 1;
#else
	irq_info->node_id = 0;
#endif

#if defined(LOONGSON_3BSINGLE) || defined(LOONGSON_3BSERVER)
	irq_info->pci_io_start_addr = 0x00001efdfc000000;
#elif defined(LOONGSON_2G5536)
	irq_info->pci_io_start_addr = 0xffffffffbfd00000;
#elif defined(LOONGSON_2G1A)
	irq_info->pci_io_start_addr = 0xffffffffbfd00000;
#elif defined(LOONGSON_2F1A)
	irq_info->pci_io_start_addr = 0xffffffffbfd00000;
#else
	irq_info->pci_io_start_addr = 0x00000efdfc000000;
#endif

#if defined(LOONGSON_2F1A)
	irq_info->pci_mem_start_addr = 0x44000000ul;
#else
	irq_info->pci_mem_start_addr = 0x40000000ul;
#endif
	irq_info->pci_mem_end_addr = 0x7ffffffful;
#if (defined RS780E || defined LS7A)
	irq_info->dma_mask_bits= 64;
#else
	irq_info->dma_mask_bits = 32;
#endif
	return irq_info;
}

struct interface_info *init_interface_info()
{

 struct interface_info *inter = &g_interface;
 int flashsize;

  tgt_flashinfo((void *)0xbfc00000, &flashsize);

  inter->vers = SPEC_VERS;
  inter->size = flashsize/0x400;
  inter->flag = 1;

#ifdef LOONGSON_3A2H
  strcpy(inter->description,"Loongson-PMON-V3.3.1");
#else
  strcpy(inter->description,"Loongson-PMON-V3.3.0");
#endif

  return inter;
}

struct board_devices * __attribute__((weak)) board_devices_info()
{

 struct board_devices *bd = &g_board;

#ifdef LOONGSON_3ASINGLE
  strcpy(bd->name,"Loongson-3A-780E-1w-V1.10-demo");
#endif
#ifdef LOONGSON_2K
  strcpy(bd->name, LS2K_BOARD_NAME);
#endif
#ifdef LOONGSON_3A2H
  strcpy(bd->name,"Loongson-3A-2H-1w-V0.7-demo");
#endif
#ifdef LOONGSON_3BSINGLE
#ifdef LOONGSON_3B1500
        strcpy(bd->name, "Loongson-3B-780E-1w-V0.5-demo");
#else
        strcpy(bd->name, "Loongson-3B-780E-1w-V1.03-demo");
#endif
#ifdef LOONGSON_3C2H
        strcpy(bd->name, "Loongson-3C-2H-1w-V0.1-demo");
#endif
#endif

#ifdef LOONGSON_3BSERVER
  strcpy(bd->name,"Loongson-3B-780E-2w-V0.2-demo");
#endif

#ifdef LOONGSON_3A84W
  strcpy(bd->name,"Loongson-3A2000-780E-4w-V0.1-demo");
#endif

#ifdef LOONGSON_3ASERVER
#ifdef USE_BMC
  strcpy(bd->name,"Loongson-3A-780E-2wBMC-V1.10-demo");
#else
  strcpy(bd->name,"Loongson-3A-780E-2w-V1.10-demo");
#endif
#endif
#ifdef LEMOTE_3AITX
  strcpy(bd->name,"lemote-3a-itx-a1101");
#endif
#ifdef LEMOTE_3ANOTEBOOK
  strcpy(bd->name,"lemote-3a-notebook-a1004");
#endif
#ifdef LOONGSON_2GQ2H
	strcpy(bd->name,"Loongson-2GQ-2H-1w-V0.1-demo");
#endif
#ifdef LOONGSON_2G5536
	strcpy(bd->name, "Loongson-2G-CS5536-1w-V0.1-demo");
#endif
#ifdef LOONGSON_2G1A
	strcpy(bd->name, "Loongson-2G-1A-1w-V0.1-demo");
#endif
#ifdef LOONGSON_2F1A
	strcpy(bd->name, "Loongson-2F-1A-1w-V0.1-demo");
#endif
  bd->num_resources = 10;

  return bd;
}


struct loongson_special_attribute *init_special_info()
{

  struct loongson_special_attribute  *special = &g_special;
  char update[11];
  
#if	defined(LOONGSON_2G5536)||defined(LOONGSON_2G1A) || defined(LOONGSON_2F1A)
  memset(update,0,11);
#endif
  get_update(update);

  strcpy(special->special_name,update);
#if	(!defined(LOONGSON_2G5536))&&(!defined(LOONGSON_2G1A) && (!defined(LOONGSON_2F1A)))
#ifdef CONFIG_GFXUMA
  special->resource[0].flags = 1;
  special->resource[0].start = 0;
  special->resource[0].end = VRAM_SIZE;
  strcpy(special->resource[0].name,"UMAMODULE");
#else
  special->resource[0].flags = 0;
  special->resource[0].start = 0;
  special->resource[0].end = VRAM_SIZE;
  strcpy(special->resource[0].name,"SPMODULE");
#endif
#endif
#if defined(LOONGSON_2G5536)||defined(LOONGSON_2G1A) || defined(LOONGSON_2F1A)
  special->resource[0].flags &= ~DMA64_SUPPORT;
#else
  special->resource[0].flags |= DMA64_SUPPORT;
#endif
  return special;
}

void __attribute__((weak)) board_info_fixup(struct efi_cpuinfo_loongson *c)
{

}
