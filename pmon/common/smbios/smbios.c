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

#define SPD_INFO_ADDR 0x8fffa000
static unsigned  short smbios_table_handle = 0;
static unsigned  short memory_array_handle = 0;

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
smbios_type_16_init(void *start, int dimmnum, int maximum_capacity);

static void *
smbios_type_17_init(void *start, int slotnum);

static void *
smbios_type_28_init(void *start);

static void *
smbios_type_127_init(void *start);

unsigned long long smbios_addr;

void loongson_smbios_init(void){
	write_smbios_tables((void *)SMBIOS_PHYSICAL_ADDRESS);
}

static size_t
write_smbios_tables(void *start)
{
	unsigned int cpu_num, nr_structs = 0, max_struct_size = 0;
	unsigned int dimmnum = 0;
        unsigned int maximum_capacity = 0;
        unsigned int i;
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

#ifdef LOONGSON_3A2H
       dimmnum = 4;
       maximum_capacity = 8 * 1024* 1024;
#endif
#ifdef LOONGSON_2G5536
       dimmnum = 0;
       maximum_capacity = 0;
#endif
#ifdef LOONGSON_3ASINGLE
       dimmnum = 4;
       maximum_capacity = 8 * 1024* 1024;
#endif
#ifdef LOONGSON_3BSINGLE
       dimmnum = 4;
       maximum_capacity = 32 * 1024* 1024;		//now the maximum capacity of one memory chip is 8G, so here is 8*4G 
#endif 
#ifdef LOONGSON_3ASERVER
       dimmnum = 8;
       maximum_capacity = 16 * 1024* 1024;
#endif
#ifdef LOONGSON_3BSERVER
       dimmnum = 8;
       maximum_capacity = 64 * 1024 * 1024;
#endif
       do_struct(smbios_type_16_init(p, dimmnum, maximum_capacity));
       for(i = 0; i < dimmnum; i++){
               do_struct(smbios_type_17_init(p, i));
       }
	
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

static inline int cpu_probe_release(void)
{
       unsigned long cputype_flag = 0;
        __asm__ volatile (
                ".set     mips3\r\n"
		"dli     $2, 0x90000efdfb000000\r\n"
                " lw      %0, 0x5c($2)\r\n"
                ".set     mips3\r\n"
                :"=r"(cputype_flag)::"$2");
        if(cputype_flag == 0x7778888) //test whether this is a 3A5 
               return 1;
        else
               return 0;
}
/* board_name information */
static void board_info(char *board_name)
{
#ifdef LOONGSON_3ASINGLE
	if(cpu_probe_release())
        	strcpy(board_name, "Loongson-3A5-780E-1w-V1.1-demo");
	else
        	strcpy(board_name, "Loongson-3A-780E-1w-V1.1-demo");
#endif

#ifdef LOONGSON_3A2H
	if(cpu_probe_release())
        	strcpy(board_name, "Loongson-3A-2H-1w-V0.6-demo");
	else
        	strcpy(board_name, "Loongson-3A5-2H-1w-V0.6-demo");
#endif

#ifdef LOONGSON_3C2H
        	strcpy(board_name, "Loongson-3C-2H-1w-V1.00-demo");
#endif

#ifdef LOONGSON_2G5536
        	strcpy(board_name, "Loongson-2G-CS5536-1w-V0.1-demo");
#endif


#ifdef LOONGSON_3BSINGLE
#ifdef LOONGSON_3B1500
        strcpy(board_name, "Loongson-3B-780E-1w-V0.4-demo");
#else
        strcpy(board_name, "Loongson-3B-780E-1w-V1.03-demo");
#endif
#endif
#ifdef LOONGSON_3BSERVER
        strcpy(board_name, "Loongson-3B-780E-2w-V0.2-demo");
#endif
#ifdef LOONGSON_3ASERVER
#ifdef USE_BMC
	if(cpu_probe_release())
        	strcpy(board_name, "Loongson-3A5-780E-2wBMC-V1.1-demo");
	else
        	strcpy(board_name, "Loongson-3A-780E-2wBMC-V1.1-demo");
#else
	if(cpu_probe_release())
        	strcpy(board_name, "Loongson-3A5-780E-2w-V1.1-demo");
	else
        	strcpy(board_name, "Loongson-3A-780E-2w-V1.1-demo");
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

	memset(p, 0, sizeof(*p));

  	get_update(update);
	strncpy(temp, update, 4);
	strncpy(temp + 4, update + 5, 2);
	strncpy(temp + 6, update + 8, 2);
	for(i = 0; i < 8; i++){
		if(temp[i] == ' ')
			temp[i] = '0';
	}
	temp[8] = '\0';
    	sprintf(pmon_version, "Loongson-PMON-V3.3-%s", temp);
	p->header.type = 0;
	p->header.length = sizeof(struct smbios_type_0);
	p->header.handle = smbios_table_handle++;
    
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
	strcpy((char *)start, "Loongson");
	start += strlen("Loongson") + 1;
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
#ifdef LOONGSON_2G5536
	char *board_family = "Loongson2";
#else
	char *board_family = "Loongson3";
#endif
	char loongson_version[10];
	char *q;
	int i;

	memset(p, 0, sizeof(*p));
	p->header.type = 1;
	p->header.length = sizeof(struct smbios_type_1);
	p->header.handle = smbios_table_handle++;

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
	strcpy((char *)start, "Loongson");
	start += strlen("Loongson") + 1;
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
	
	memset(p, 0, sizeof(*p));
 
	p->header.type = 2;
	p->header.length = sizeof(struct smbios_type_2);
	p->header.handle = smbios_table_handle++;

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
	strcpy((char *)start, "Loongson");
	start += strlen("Loongson") + 1;
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
	
	memset(p, 0, sizeof(*p));
        p->header.type = 4;
        p->header.length = sizeof(struct smbios_type_4);
        p->header.handle = smbios_table_handle++;

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
#ifdef LOONGSON_2G5536
		strcpy(cpu_version, "Loongson ICT Loongson-2G CPU @ 800MHz");
		p->max_speed = 800;
#else
		if(cpu_probe_release())
			strcpy(cpu_version, "Loongson ICT Loongson-3A5 CPU @ 1.0GHz");
		else
			strcpy(cpu_version, "Loongson ICT Loongson-3A CPU @ 1.0GHz");
		p->max_speed = 1000;
#endif
	}
	if(prid == 0x6306){
		strcpy(cpu_version, "Loongson ICT Loongson-3B CPU @ 1.0GHz");
		p->max_speed = 1000;
	}
	if(prid == 0x6307){
		strcpy(cpu_version, "Loongson ICT Loongson-3B CPU @ 1.5GHz");
		p->max_speed = 1500;
	}

        p->version_str = 2;
        p->voltage  = 0x02;
        p->external_clock = 25;
        p->current_speed = atoi(getenv("cpuclock")) / 1000000;
        p->status  = 0x02;
        p->upgrade = 0x01;
        p->l1_cache_handle = 0xffff;
        p->l2_cache_handle = 0xffff;
        p->l3_cache_handle = 0xffff;
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
#ifdef LOONGSON_3C2H
	p->core_count = p->core_enable = 8;
#endif
#ifdef LOONGSON_2G5536
	p->core_count = p->core_enable = 1;
#endif
        p->thread_count = 0;
        p->processor_characteristics = 0x02;
        p->processor_family2 = 0x01;

        start += sizeof(struct smbios_type_4);
	strcpy((char *)start, "Loongson");
        start += strlen("Loongson") +  1;
        strcpy((char *)start, cpu_version);
        start += strlen(cpu_version) +  1;

        *((uint8_t *)start) = 0;
        return start+1;
}
/*Type 16 -- Physical Memory Array*/
static void *
smbios_type_16_init(void *start, int dimmnum, int maximum_capacity){

       struct smbios_type_16 *p = (struct smbios_type_16 *)start;

       memset(p, 0, sizeof(*p));
       
       p->header.type = 16;
        p->header.length = sizeof(struct smbios_type_16);
        p->header.handle = memory_array_handle = smbios_table_handle++;

       p->location = 0x03;
       p->use     =  0x03;
       p->error_correction = 0x02;
       p->maximum_capacity = maximum_capacity;
       p->memory_error_information_handle = 0xfffe;
       p->number_of_memory_devices = dimmnum;
       
       start += sizeof(struct smbios_type_16);
        *((uint8_t *)start) = 0;
	*((uint8_t *)(start + 1)) =0;
        return start + 2;
}

/* Type 17 Memory Device */
unsigned char get_spd_byte(int slotnum, int offset){
       
       unsigned char spd_byte;

       spd_byte = *(volatile unsigned char *)(SPD_INFO_ADDR + slotnum * 0x100 + offset);

       return spd_byte;
}

unsigned int get_ddr_size(int slotnum, int ddrtype){
       unsigned char offset_rank,offset_density,offset_0x08;  
       unsigned char density,capacity,Width,BWidth,rank;  
        int value = 0;
       int size = 0;
       
       if(ddrtype == 0x8){
               offset_rank=0x05;
               rank = get_spd_byte(slotnum, offset_rank) & 0xf;
               rank++;
               
               offset_density=0x1F;
               if(density==0x20)  
                  value=128;  
                else if(density==0x40)  
                  value=256;  
                else if(density==0x80)  
                  value=512;  
                else if(density==0x01)  
                  value=1024;  
                else if(density==0x02)  
                  value=2048;  
                else if(density==0x04)  
                   value=4096;  
                else  
                   value=0;  

               size = rank * value;

       }
       if(ddrtype == 0xb){
               offset_density=0x04;//Capacity   
               capacity=get_spd_byte(slotnum, offset_density); 
               //Capacity  is the total capacity of SDRAM   
               //bank=density;   
                capacity&=0x0F;//b[0:3] at 0x04  

               if(capacity==1)  
                               value=512;  
               else if(capacity==2)  
                               value=1024;  
                else if(capacity==3)  
                               value=2048;  
               else if(capacity==4)  
                               value=4096;  
               else if(capacity==5)  
                               value=8192;  
               else if(capacity==6)  
                               value=16384;  
               else    
                       value=256;
               offset_rank=0x07;//Rank   
               rank=get_spd_byte(slotnum, offset_rank);
               Width=rank;  
               Width&=0x07;//b[0:2] at 0x07   
                rank&=0x38;//00111000 , b[3:5] at 0x07   
                rank=rank>>3;  
                rank++;

               switch(Width)  
                {  
                case 0:Width=4;     break;  
                case 1:Width=8;     break;  
                case 2:Width=16;    break;  
                case 3:Width=32;    break;  
               }  
                offset_0x08=0x08;  
               BWidth=get_spd_byte(slotnum,offset_0x08);  
               BWidth&=0x07;//b[0:2] at 0x08   
                switch(BWidth)  
                {  
                case 0:BWidth=8;   break;  
                case 1:BWidth=16;  break;  
                case 2:BWidth=32;  break;  
                case 3:BWidth=64;  break;  
                }
               size=(value/8)*BWidth/Width*rank;        
       }
       
       
       return size;
}

unsigned int get_ddr_speed(int slotnum, int ddrtype){
       unsigned char cycle=0,offset=0; 
       unsigned int speed = 0;

       if(ddrtype == 0x8){
               offset=0x09;  
           cycle=get_spd_byte(slotnum, offset);  
           if(cycle==0x50)  
               speed=400;  
           else if(cycle==0x3D)  
               speed=533;  
                  else if(cycle==0x30)  
               speed=667;  
           else if(cycle==0x25)  
               speed=800;  
       }  
       
       if(ddrtype == 0xb){
               offset=0x0C;  
            cycle=get_spd_byte(slotnum, offset);  
            if(cycle==0x14)  
                speed=800;  
            else if(cycle==0x0F)  
                speed=1066;  
            else if(cycle==0x0C)  
                speed=1333;  
            else if(cycle==0x0A)  
                speed=1600;
       }
       
       return speed;
       
}
void get_ddr_manustr(int slotnum,  char manustr[],int ddrtype){
       unsigned char offset=0,manu=0; 
       
       if(ddrtype == 0x8)  
       {  
         offset=0x40;  
         if((manu=get_spd_byte(slotnum, offset))==0x7F)  
            {//bank one   
                offset++;  
                if((manu=get_spd_byte(slotnum, offset))==0x7F)  
                    {//banktwo   
                        offset++;  
                        if((manu=get_spd_byte(slotnum, offset))==0x7F)  
                        {//bankthree   
                            offset++;  
                            if((manu=get_spd_byte(slotnum, offset))==0x7F)  
                            {//bankfour   
                                offset++;  
                                manu=get_spd_byte(slotnum, offset);  
                            }  
                        }  
                    }  
                }  
  
       }  
       
       if(ddrtype == 0xb){
        offset=0x76;
        manu=get_spd_byte(slotnum, offset);  
       }
       if(manu==0xAD)  
             strcpy(manustr,"Hynix");  
       else if(manu==0xCE)  
             strcpy(manustr,"SamSung");  
       else if(manu==0x0B)  
             strcpy(manustr,"Micron");  
       else if(manu==0x98)  
             strcpy(manustr,"Kingston");  
       else if(manu==0x25)  
             strcpy(manustr,"Kingmax");  
       else if(manu==0x4f)  
             strcpy(manustr,"Transcend");  
       else if(manu==0xCB)  
             strcpy(manustr,"A-DATA Technology");  
       else  
             strcpy(manustr,"Unknow");  
       
       return 0;

}
void get_ddr_snstr(int slotnum, char snstr[], int ddrtype){
       char i = 0, offset, tmp;
       if(ddrtype == 0x8)
               offset = 95;
       if(ddrtype == 0xb)
               offset = 122;
       for(i = 0; i < 4; i++){
               tmp = get_spd_byte(slotnum, i + offset);
               snstr[i * 2] = (((tmp & 0xf0) >> 4) > 9) ? (((tmp & 0xf0) >> 4) - 0xa + 'A') : (((tmp & 0xf0) >> 4) + '0');
               snstr[i * 2 + 1] = ((tmp & 0xf) > 9)  ? ((tmp  & 0xf) - 0xa + 'A') : ((tmp & 0xf) + '0');
       }
       snstr[i * 2] = '\0';
       return 0;
}
void get_ddr_pnstr(int slotnum, char pnstr[], int ddrtype){
       int i = 0, offset, tmp;
       if(ddrtype == 0x8)
               offset = 73;
       if(ddrtype == 0xb)
               offset = 128;
       for(i = 0; i < 18; i++){
		 pnstr[i] = get_spd_byte(slotnum, i + offset);
	}
        pnstr[i] = '\0';
	if(pnstr[0] == 0)
		strcpy(pnstr, "unknown");
       return 0;
}
static void *
smbios_type_17_init(void *start, int slotnum){
       struct smbios_type_17 *p = (struct smbios_type_17 *)start;
       int ddrtype = 0;
       char dimmstr[10] = {0};
       char  manustr[20] = {0};
       char snstr[10] = {0};
       char pnstr[20] = {0};

        memset(p, 0, sizeof(*p));

       
        p->header.type = 17;
        p->header.length = sizeof(struct smbios_type_17);
        p->header.handle = smbios_table_handle++;
       
       p->physical_memory_array_handle = memory_array_handle;
       p->memory_error_information_handle = 0xfffe;
               
       ddrtype = get_spd_byte(slotnum, 2);
       if(ddrtype == 0xb){
               p->data_width = (0x1 << ((get_spd_byte(slotnum, 8)) & 0x7)) * 8;
       	       p->total_width = (((get_spd_byte(slotnum, 8)) & 0x18) >> 3) * 8 + p->data_width;	
	}
       else{
               p->total_width = 64;
               p->data_width  = 64;
       }
       
       if(ddrtype == 0x8)
               p->memory_type = 0x13;
       if(ddrtype == 0xb)
               p->memory_type = 0x18;

       p->speed = get_ddr_speed(slotnum, ddrtype);
       p->size  = get_ddr_size(slotnum, ddrtype);
       p->form_factor = 0x09; /* DIMM */
       p->device_set = 0;
       p->device_locator_str = 1;
       p->bank_locator_str = 2;

       if(ddrtype == 0x8 || ddrtype == 0xb){
               p->manufacturer  = 3;
               p->serial_number = 4;
               p->part_number   = 5;
       }
       
       start += sizeof(struct smbios_type_17);
       if(slotnum < 8)
        sprintf(dimmstr, "DIMM %d", slotnum);
       strcpy(start, dimmstr);
       start += strlen(dimmstr) + 1;

       strcpy(start, "BANK 0");
       start += strlen("BANK 0") + 1;
       
       if(ddrtype == 0x8 || ddrtype == 0xb){
               get_ddr_manustr(slotnum, manustr, ddrtype);
               strcpy(start, manustr); 
               start += strlen(manustr) + 1;

               get_ddr_snstr(slotnum, snstr, ddrtype);
               strcpy(start, snstr); 
               start += strlen(snstr) + 1;

               get_ddr_pnstr(slotnum, pnstr, ddrtype);
               strcpy(start, pnstr); 
               start += strlen(pnstr) + 1;
       }
       *((uint8_t *)start) = 0;
        return start+1;

}
/* Type 28 -- Temperature Probe */
static void *
smbios_type_28_init(void *start)
{
	struct smbios_type_28 *p = (struct smbios_type_28 *)start;

	memset(p, 0, sizeof(*p));
	p->header.type = 28;
	p->header.length = sizeof(struct smbios_type_28);
	p->header.handle = smbios_table_handle++;
    	
	p->description = 1;
	p->location_status = 0x63;
	p->maximum_value = 1280;
	p->minimum_value = 0x8000;
	p->resolution = 0x8000;
	p->tolerance = 0x8000;
	p->accuracy = 0x8000;
	p->OEM_defined =  0x00;

#if (defined LOONGSON_3A2H) || (defined LOONGSON_3ASINGLE) || (defined LOONGSON_3ASERVER)
        p->nominal_value = (*(volatile unsigned char *)(0xffffffffbfe0019d) & 0x7f) * 10;
#elif defined LOONGSON_3B1500 
        p->nominal_value = (*(volatile unsigned char *)(0xffffffffbfe0019d) - 100) * 10;
#elif defined(LOONGSON_2G5536)
        p->nominal_value = (*(volatile unsigned char *)(0xffffffffbfe0019c) & 0x7f) * 10;
#endif

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
	memset(p, 0, sizeof(*p));
	p->header.type = 127;
	p->header.length = sizeof(struct smbios_type_127);
	p->header.handle = smbios_table_handle++;
	start += sizeof(struct smbios_type_127);
	*((uint8_t *)start) = 0;
	*((uint8_t *)(start + 1)) =0;
	return start + 2;
}
