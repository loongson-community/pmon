/*--------------------------------------------------------------------------
 * Copyright (c) 2005 by Winbond Electronics Corporation
 * All rights reserved.
 *<<<-----------------------------------------------------------------------
 * File Contents: shm.h - Shared Memory Access header file			
 *
 *--------------------------------------------------------------------->>>*/

#ifndef _shm_h_
#define _shm_h_


/*------------------------------------------------------------------------*/
/*-----------------   Data types                      --------------------*/
/*------------------------------------------------------------------------*/

typedef enum
{
    SHM_MEM_LPC = 0,
    SHM_MEM_FWH
} SHM_mem_access_t;

typedef struct
{
	 unsigned char read_device_id;
     unsigned char write_status_enable;
     unsigned char write_enable;
     unsigned char read_status_reg;
     unsigned char write_status_reg;
     unsigned char page_program;
     unsigned char sector_erase;
     unsigned char status_busy_mask;
	 unsigned char status_reg_val;
	 unsigned char program_unit_size;
	 unsigned char page_size;
     unsigned char read_dev_id_type;
} FLASH_device_commands_t;

typedef struct SectorDef
{
  int num;
  int size;
} Sector_t;  

/*------------------------------------------------------------------------*/
/*-----------------   Global Variables                --------------------*/
/*------------------------------------------------------------------------*/
extern SHM_mem_access_t SHM_interface;
extern FLASH_device_commands_t FLASH_commands;
extern unsigned int FLASH_id;
extern unsigned int FLASH_size;
extern unsigned int FLASH_sector_size;
extern unsigned int FLASH_page_size;
extern unsigned int FLASH_block_size;
extern unsigned char FLASH_block_erase, FLASH_sector_erase;
extern unsigned char protected_sectors[64];
extern Sector_t Flash_sectors[20];
extern int NumOfSectoresInBlock;
extern int NumOfSectores;
extern int verbose;
extern char Config_file_name[];

#endif //_shm_h_