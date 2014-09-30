#ifndef __ASM_MACH_LOONGSON_BOOT_PARAM_H_
#define __ASM_MACH_LOONGSON_BOOT_PARAM_H_

#define DMA64_SUPPORT 0x2

#define SYSTEM_RAM_LOW 1
#define SYSTEM_RAM_HIGH 2
#define MEM_RESERVEI 3
#define PCI_IO 4
#define PCI_MEM 5
#define LOONGSON_CFG_REG 6
#define VIDEO_ROM 7
#define DAPTER_ROM 8 
#define ACPI_TABLE 9
#define SMBIOS_TABLE 10
#define UMA_VIDEO_RAM 11
#define MAX_MEMORY_TYPE 12


/*no-use for pmon*/
struct efi_systab {
	// efi_table_hdr_t hdr;  /*EFI header*/
	unsigned long long fw_vendor;     /* physical addr of CHAR16 vendor string */
	unsigned long long fw_revision;   /*vesion of  efi*/
	unsigned long long con_in_handle; /*control handle of input*/
	unsigned long long con_in;   /*control input,support for inputing in kernel*/
	unsigned long long con_out_handle;  /*control handle of input*/
	unsigned long long con_out; /*control input,support for inputing in kernel*/
	unsigned long long stderr_handle; /*standed error  handle*/
	// unsigned long stderr; /*standed error */
	// efi_runtime_services_t runtime; /*runtime service */
	unsigned long long boottime; /*boottime */
	unsigned int nr_tables; /*table id*/
	unsigned long long tables;  /*all tables `entry */
};

#define BOOT_MEM_MAP_MAX 128
 struct efi_memory_map_loongson{
  unsigned short vers;/*version of efi_memory_map*/
  unsigned int nr_map; /*number of memory_maps*/
  unsigned int mem_freq;/*memory frequence*/
  struct mem_map{
	unsigned int node_id;/*recorde the node_id*/
	unsigned int mem_type;/*recorde the memory type*/
	unsigned long long mem_start;/*memory map start address*/
	unsigned int mem_size; /*for each memory_map size,not the total size*/
  }map[BOOT_MEM_MAP_MAX];
}__attribute__((packed));

enum loongson_cpu_type
{
	Loongson_2F,Loongson_2E, Loongson_3A, Loongson_3B,Loongson_1A,Loongson_1B,Loongson_2G,
};

struct efi_cpuinfo_loongson {
	/*
	 * Capability and feature descriptor structure for MIPS CPU
	 */
	unsigned short vers; /*version of efi_cpuinfo_loongson*/
	unsigned int        processor_id;//6505, 6506-3b prid
	enum loongson_cpu_type  cputype;//3a-3b
	unsigned int         total_node;   /* physical core number */

	unsigned short cpu_startup_core_id; /* Core id: */
	unsigned short reserved_cores_mask; /* Core id: */
	unsigned int  cpu_clock_freq; //cpu_clock
	unsigned int nr_cpus;
}__attribute__((packed));


struct system_loongson{
	unsigned short vers;/*version of system_loongson*/
	unsigned int ccnuma_smp; // 0:no numa; 1: has numa
	unsigned int sing_double_channel;//1:single; 2:double
}__attribute__((packed));

struct irq_source_routing_table {

	unsigned short vers;
	unsigned short size;
	unsigned short bus,devfn;
	unsigned int vendor,device, PIC_type;   //conform use HT or PCI to route to CPU-PIC

	unsigned long long ht_int_bit;   //3a: 1<<24; 3b:1<<16
	unsigned long long ht_enable;   //irqs used in this PIC.eg:3a-0x0000d17a
	unsigned int   node_id;  // node id, 0x0 —0, 0x1—1; 0x10—2；0x11—3；0x100—4

	unsigned long long  pci_mem_start_addr;
	unsigned long long  pci_mem_end_addr;
	unsigned long long  pci_io_start_addr;
	unsigned long long  pci_io_end_addr;
	unsigned long long  pci_config_addr;
	unsigned int dma_mask_bits;

}__attribute__((packed));


struct interface_info{
	unsigned short vers; /*version of the specificition*/
	unsigned short size; /**/
	unsigned char flag;/**/
	char description[64]; /**/
}__attribute__((packed));


#define MAX_RESOUCR_NUMBER 128
struct resource_loongson {
  unsigned long long start; /*resource start address*/
  unsigned long long end; /*resource end address*/
  char name[64];
  unsigned int flags;
};

struct archdev_data {}; /*arch specific additions*/

struct board_devices{
  char name[64]; /*hold the device name*/
  unsigned int num_resources; /*number of device_resource*/
  struct resource_loongson resource[MAX_RESOUCR_NUMBER]; /*for each device`s resource*/
  /*arch specific additions*/
  struct archdev_data archdata; 
};

struct loongson_special_attribute{
	unsigned short vers; /*version of this special*/
	char special_name[64]; /*special_atribute_name*/
	unsigned int loongson_special_type; /*tyoe of special device*/
    struct resource_loongson resource[MAX_RESOUCR_NUMBER];/*special_vlaue resource*/
};

struct loongson_params{
	unsigned long long	memory_offset;/*efi_memory_map_loongson struct offset*/
	unsigned long long cpu_offset;  /*efi_cpuinfo_loongson struct offset*/
	unsigned long long system_offset;  /*system_info struct offset*/
	unsigned long long irq_offset;  /*irq_source_routing_table struct offset*/
	unsigned long long interface_offset;  /*interface_info struct offset*/
	unsigned long long special_offset;  /*loongson_special_attribute struct offset*/
	unsigned long long boarddev_table_offset;  /*board_device offset*/
};

struct smbios_tables {
	unsigned short vers; /*version of smbios*/
	unsigned long long vga_bios;/*vga_bios address*/
	struct loongson_params lp;
};

struct efi_reset_system_t
{
        unsigned long long  ResetCold;
        unsigned long long  ResetWarm;
        unsigned long long  ResetType;
        unsigned long long  Shutdown;
};

struct efi {
	//efi_system_table_t systab;       /* EFI system table */
	unsigned long long  mps;              /* MPS table */
	unsigned long long  acpi;              /* ACPI table  (IA64 ext 0.71) */
	unsigned long long acpi20;            /* ACPI table  (ACPI 2.0) */
	struct smbios_tables smbios;            /* SM BIOS table */
	unsigned long long sal_systab;         /* SAL system table */
	unsigned long long boot_info;         /* boot info table */
	/*have non this struct for stage1-1
	  efi_get_time_t *get_time;
	  efi_set_time_t *set_time;
	  efi_get_wakeup_time_t *get_wakeup_time;
	  efi_set_wakeup_time_t *set_wakeup_time;
	  efi_get_variable_t *get_variable;
	  efi_get_next_variable_t *get_next_variable;
	  efi_set_variable_t *set_variable;
	  efi_get_next_high_mono_count_t *get_next_high_mono_count;
	  efi_reset_system_t *reset_system;
	  efi_set_virtual_address_map_t *set_virtual_address_map;
	 */
};

struct boot_params{
	//struct screen_info *screen_info;
	//struct sys_desc_table *sys_desc_table;
	struct efi efi;
        struct efi_reset_system_t  reset_system;
};

#endif
