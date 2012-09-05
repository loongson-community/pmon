/*
 * smbios.c - Generate SMBIOS tables for Loongson PMON's.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 * Copyright (C) IBM Corporation, 2006
 * Authors: Andrew D. Ball <aball@us.ibm.com>
 * 
 * Adapated for Loongson PMON
 * (C) Copyright 2012 <meiwenbin@loongson.cn> and <fandongdong@loongson.cn>
 */

#include <stdlib.h>
#include <stdio.h>
#include "smbios.h"
#include "smbios_types.h"

void loongson_smbios_init(void);

static size_t
write_smbios_tables(void *start);

void
byte_to_hex(char *digits, uint8_t byte);

void
uuid_to_string(char *dest, uint8_t *uuid);

static void
smbios_entry_point_init(void *start,
			uint16_t max_structure_size,
			uint16_t structure_table_length,
			uint32_t structure_table_address,
			uint16_t number_of_structures);
static void *
smbios_type_0_init(void *start);

static void *
smbios_type_1_init(void *start);

static void *
smbios_type_2_init(void *start);

static void *
smbios_type_4_init(void *start);

static void *
smbios_type_28_init(void *start);

static void *
smbios_type_127_init(void *start);

void loongson_smbios_init(void){
	write_smbios_tables((void *)SMBIOS_PHYSICAL_ADDRESS);

}
static size_t
write_smbios_tables(void *start)
{
	unsigned cpu_num, nr_structs = 0, max_struct_size = 0;
	char *p, *q;


	p = (char *)start + sizeof(struct smbios_entry_point);

#define do_struct(fn) do {			\
	q = (fn);				\
	nr_structs++;				\
	if ((q - p) > max_struct_size)		\
		max_struct_size = q - p;	\
	p = q;					\
} while (0)

	do_struct(smbios_type_0_init(p));
	
	do_struct(smbios_type_1_init(p));
	do_struct(smbios_type_2_init(p));
	do_struct(smbios_type_4_init(p));
	do_struct(smbios_type_28_init(p));
	do_struct(smbios_type_127_init(p));
#undef do_struct

	smbios_entry_point_init(
		start, max_struct_size,
		(p - (char *)start) - sizeof(struct smbios_entry_point),
		SMBIOS_PHYSICAL_ADDRESS + sizeof(struct smbios_entry_point),
		nr_structs);

	return (size_t)((char *)p - (char *)start);
}

static void
smbios_entry_point_init(void *start,
			uint16_t max_structure_size,
			uint16_t structure_table_length,
			uint32_t structure_table_address,
			uint16_t number_of_structures)
{
	uint8_t sum;
	int i;
	struct smbios_entry_point *ep = (struct smbios_entry_point *)start;
	strncpy(ep->anchor_string, "_SM_", 4);
	ep->length = 0x1f;
	ep->smbios_major_version = 2;
	ep->smbios_minor_version = 6;
	ep->max_structure_size = max_structure_size;
	ep->entry_point_revision = 0;
	memset(ep->formatted_area, 0, 5);
	strncpy(ep->intermediate_anchor_string, "_DMI_", 5);
    
	ep->structure_table_length = structure_table_length;
	ep->structure_table_address = structure_table_address;
	ep->number_of_structures = number_of_structures;
	ep->smbios_bcd_revision = 0x00;

	ep->checksum = 0;
	ep->intermediate_checksum = 0;
	sum = 0;
	for (i = 0x0; i < 0x10; ++i)
		sum += ((int8_t *)start)[i];
	ep->checksum = -sum;
	sum = 0;
	for (i = 0x10; i < ep->length; ++i)
		sum += ((int8_t *)start)[i];
	ep->intermediate_checksum = -sum;
}

/* board_name information */
static void board_info(char *board_name)
{
#ifdef LOONGSON_3ASINGLE
        strcpy(board_name, "Loongson-3A-780E-1w-V1.03-demo");
#endif
#ifdef LOONGSON_3A2H
        strcpy(board_name, "Loongson-3A-2H-1w-V1.00-demo");
#endif
#ifdef LOONGSON_3BSINGLE
        strcpy(board_name, "Loongson-3B-780E-1w-V1.03-demo");
#endif
#ifdef LOONGSON_3BSERVER
        strcpy(board_name, "Loongson-3B-780E-2w-V1.03-demo");
#endif
#ifdef LOONGSON_3ASERVER
#ifdef USE_BMC
        strcpy(board_name, "Loongson-3A-780E-2wBMC-V1.02-demo");
#else
        strcpy(board_name, "Loongson-3A-780E-2w-V1.02-demo");
#endif
#endif

	return ;
}

static void prase_name(char *name, char *version)
{
	int i;
	char *q;

        q = strchr(name, 'V');
        for(i = 0, q++; *(q+i) != '-'; i++)
                version[i] = *(q+i);
        version[i] = '\0';

	return ;
}

/* Type 0 -- BIOS Information */
static void *
smbios_type_0_init(void *start)
{
	struct smbios_type_0 *p = (struct smbios_type_0 *)start;
	int flashsize = 0;
	char update[11];
	char release_date_string[11];
	char pmon_version[40];
	char temp[20];
	int i;

  	get_update(update);
	strncpy(temp, update, 4);
	strncpy(temp + 4, update + 5, 2);
	strncpy(temp + 6, update + 8, 2);
	for(i = 0; i < 8; i++){
		if(temp[i] == ' ')
			temp[i] = '0';
	}
	temp[8] = '\0';
    	sprintf(pmon_version, "LoongSon-PMON-V3.0-%s", temp);
	p->header.type = 0;
	p->header.length = sizeof(struct smbios_type_0);
	p->header.handle = 0;
    
	p->vendor_str = 1;
	p->version_str = 2;
	p->starting_address_segment =0xe000;
	p->release_date_str = 3;

	tgt_flashinfo((void *)0xbfc00000, &flashsize);
	p->rom_size = (flashsize>>16) - 1;
    
	memset(p->characteristics, 0, 8);
	//p->characteristics[7] = 0x08; /* BIOS characteristics not supported */
	//p->characteristics[0] = 0x80;
	p->characteristics[1] = 0x18;
	p->characteristics[2] = 0x05;
	
	
	p->characteristics_extension_bytes[0] = 0x80;
	p->characteristics_extension_bytes[1] = 0x02;
    
	p->major_release = 3;
	p->minor_release = 0;
	p->embedded_controller_major = 0xff;
	p->embedded_controller_minor = 0xff;

	start += sizeof(struct smbios_type_0);
	strcpy((char *)start, "LoongSon");
	start += strlen("LoongSon") + 1;
	strcpy((char *)start, pmon_version);
	start += strlen(pmon_version) + 1;
	
	strncpy(release_date_string, update + 5, 5);
	strncpy(release_date_string + 6, update, 4);
	release_date_string[2] = '/';
	release_date_string[5] = '/';
	release_date_string[10] = '\0';
	strcpy((char *)start, release_date_string);
	start += strlen(release_date_string) + 1;

	*((uint8_t *)start) = 0;
	return start + 1;
}


/* Type 1 -- System Information */
static void *
smbios_type_1_init(void *start)
{
	struct smbios_type_1 *p = (struct smbios_type_1 *)start;
	char product_name[50];
	char *board_family = "LoongSon3";
	char loongson_version[10];
	char *q;
	int i;

	p->header.type = 1;
	p->header.length = sizeof(struct smbios_type_1);
	p->header.handle = 0x1;

	p->manufacturer_str = 1;
	p->product_name_str = 2;
	p->version_str = 3;
	p->serial_number_str = 0;

	p->wake_up_type = 0x06; /* power switch */
	p->sku_str = 0;
	p->family_str = 4;

	board_info(product_name);
	prase_name(product_name, loongson_version);

	uuid_generate(p->uuid);

	start += sizeof(struct smbios_type_1);
	strcpy((char *)start, "LoongSon");
	start += strlen("LoongSon") + 1;
	strcpy((char *)start, product_name);
	start += strlen(product_name) + 1;
	strcpy((char *)start, loongson_version);
	start += strlen(loongson_version) + 1;
	strcpy((char *)start, board_family);
	start += strlen(board_family) + 1;
	
	*((uint8_t *)start) = 0;
    
	return start+1; 
}

/* Type 2 -- Board Information */
static void *
smbios_type_2_init(void *start)
{
	struct smbios_type_2 *p = (struct smbios_type_2 *)start;
	char board_name[50];	
	char board_version[10];
   	char *motherboard_serial[20];
 
	p->header.type = 2;
	p->header.length = sizeof(struct smbios_type_2);
	p->header.handle = 0x2;

	p->manufacturer_str = 1;
	p->product_name_str = 2;
	p->version_str = 3;
	p->serial_number_str = 0;
	p->asset_tag_str = 0;
	p->feature_flags = 0x9;
	p->location_in_chassis_str = 0;
	p->chassis_handle = 0;
	p->board_type = 0x0a;
	p->number_contained_object_handles = 0;
	p->contained_object_handles = 0;

	board_info(board_name);
	prase_name(board_name, board_version);
	
	start += sizeof(struct smbios_type_2);
	strcpy((char *)start, "LoongSon");
	start += strlen("LoongSon") + 1;
	strcpy((char *)start, board_name);
	start += strlen(board_name) + 1;
	strcpy((char *)start, board_version);
	start += strlen(board_version) + 1;

	*((uint8_t *)start) = 0;
	return start+ 1;
}

/* Type 4 -- Processor Information */
static void *
smbios_type_4_init(void *start)
{
        struct smbios_type_4 *p = (struct smbios_type_4 *)start;
        unsigned int prid;
	unsigned int cpus;
	char cpu_version[64];

        p->header.type = 4;
        p->header.length = sizeof(struct smbios_type_4);
        p->header.handle = 4;

        p->socket_designation_str = 0;
        p->processor_type = 0x03;
        p->processor_family = 0x01;
        p->manufacturer_str = 1;

	__asm__ volatile ( ".set     mips64\r\n"
		"mfc0 %0,$15,0\r\n"
		".set     mips3\r\n"
		:"=r"(prid));
	
        p->cpuid.ProcessorSteppingId  = (prid >> 0x8) & 0xf;
        p->cpuid.ProcessorModel  = (prid >> 0xc) & 0xf;
        p->cpuid.ProcessorFamily = (prid >> 0x0) & 0xf;
        p->cpuid.ProcessorType   = (prid >> 0x4) & 0xf;
        p->cpuid.ProcessorReserved1 =  0;
        p->cpuid.ProcessorXModel = 0;
        p->cpuid.ProcessorXFamily = 0;
        p->cpuid.ProcessorReserved2 = 0;
        p->cpuid.FeatureFlags = 0;

	if(prid == 0x6304){
		strcpy(cpu_version, "Loongson ICT Loongson-2F CPU @ 800MHz");
		p->max_speed = 800;
	}
	if(prid == 0x6305){
		strcpy(cpu_version, "Loongson ICT Loongson-3A CPU @ 1.0GHz");
		p->max_speed = 1000;
	}
	if(prid == 0x6306){
		strcpy(cpu_version, "Loongson ICT Loongson-3B CPU @ 1.0GHz");
		p->max_speed = 1000;
	}

        p->version_str = 2;
        p->voltage  = 0x02;
        p->external_clock = 25;
        p->current_speed = atoi(getenv("cpuclock")) / 1000000;
        p->status  = 0x02;
        p->upgrade = 0x01;
        p->l1_cache_handle = 0;
        p->l2_cache_handle = 0;
        p->l3_cache_handle = 0;
        p->serial_number_str = 0;
        p->assert_tag_str = 0;
        p->part_number_str = 0;

#ifdef LOONGSON_3ASINGLE
	p->core_count = p->core_enable = 4;
#endif
#ifdef LOONGSON_3ASERVER
	p->core_count = p->core_enable = 8;
#endif
#ifdef LOONGSON_3BSINGLE
	p->core_count = p->core_enable = 8;
#endif
#ifdef LOONGSON_3BSERVER
	p->core_count = p->core_enable = 16;
#endif
#ifdef LOONGSON_3A2H
	p->core_count = p->core_enable = 4;
#endif

        p->thread_count = 0;
        p->processor_characteristics = 0x02;
        p->processor_family2 = 0x01;

        start += sizeof(struct smbios_type_4);
	strcpy((char *)start, "LoongSon");
        start += strlen("LoongSon") +  1;
        strcpy((char *)start, cpu_version);
        start += strlen(cpu_version) +  1;

        *((uint8_t *)start) = 0;
        return start+1;
}

/* Type 28 -- Temperature Probe */
static void *
smbios_type_28_init(void *start)
{
	struct smbios_type_28 *p = (struct smbios_type_28 *)start;

	p->header.type = 28;
	p->header.length = sizeof(struct smbios_type_28);
	p->header.handle = 28;
    	
	p->description = 1;
	p->location_status = 0x63;
	p->maximum_value = 1280;
	p->minimum_value = 0x8000;
	p->resolution = 0x8000;
	p->tolerance = 0x8000;
	p->accuracy = 0x8000;
	p->OEM_defined =  0x00;
	p->nominal_value = (*(volatile unsigned char *)(0xffffffffbfe0019d) & 0x3f) * 10;
	
	start += sizeof(struct smbios_type_28);
	strcpy((char *)start, "CPU Temperature");
	start += strlen("CPU Temperature") +  1;

	*((uint8_t *)start) = 0;
	return start+1;
}


/* Type 127 -- End of Table */
static void *
smbios_type_127_init(void *start)
{
	struct smbios_type_127 *p = (struct smbios_type_127 *)start;
	p->header.type = 127;
	p->header.length = sizeof(struct smbios_type_127);
	p->header.handle = 127;
	start += sizeof(struct smbios_type_127);
	*((uint8_t *)start) = 0;
	*((uint8_t *)(start + 1)) =0;
	return start + 2;
}
