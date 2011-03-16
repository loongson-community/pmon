/*
 * wpce775x.c 
 * 
 * Author : huangwei <huangw@lemote.com>
 * Date   : 2010-03-11
 *
 * handle all the thing about the EC wpce775x (wpce775l)
 * so far, the following functions are included :
 * 1, fixup some patch for 3GNB platform
 * 2, update EC rom
 *
 * NOTE :
 *	This device is connected to the LPC bus and then to 
 *	PCI bus through cs5536 chip
 */

#include <stdio.h>
#include <stdarg.h>
#include <sys/linux/types.h>
#include <include/bonito.h>
#include <machine/pio.h> 
#include "include/wpce775x.h"
#include "include/shm.h"
#include "include/flupdate.h"

/************************************************************/

//#ifdef LOONGSON2F_3GNB
#define boolean unsigned char
WCB_struct	     WCB;
FLASH_device_commands_t FLASH_commands;
unsigned int FLASH_size;
unsigned int FLASH_sector_size;
unsigned int FLASH_block_size;
unsigned int FLASH_page_size;
int NumOfSectoresInBlock;
WCB_exit_t exit_type;                           // Type of protocol termination to use
int verbose = 0;

void Update_Flash_Init(void)
{
	FLASH_block_size = EC_BLOCK_SIZE;			// 64KB
	FLASH_size = EC_FLASH_SIZE;					// 1MB
	FLASH_sector_size = EC_SECTOR_SIZE * 1024;	// 4KB
	FLASH_page_size = EC_PAGE_SIZE;				// 256B
	NumOfSectoresInBlock = 16;
}

void set_interface_cfg(u8 flag)
{
	if(flag){
		/* Enable shared flash access. */
		wrldn(LDN_SHM, SHM_CFG, FLASH_ACC_EN);
		wrldn(LDN_SHM, SHAW1BA_2, (FLASH_WIN_BASE_ADDR & 0x00FF0000) >> 16);
#ifdef LOONGSON3A_3A780E
		wrldn(LDN_SHM, SHAW1BA_3, ((FLASH_WIN_BASE_ADDR & 0x1F000000) >> 24));
#endif
#ifdef LOONGSON2F_3GNB
		wrldn(LDN_SHM, SHAW1BA_3, ((FLASH_WIN_BASE_ADDR & 0xFF000000) >> 24) & 0x0F);
#endif
	}else{
		/* Enable shared ram access. */
		wrldn(LDN_SHM, SHM_CFG, rdldn(LDN_SHM, SHM_CFG) & ~FLASH_ACC_EN);
		wrldn(LDN_SHM, SHAW2BA_2, (WCB_BASE_ADDR & 0x00FF0000) >> 16);
#ifdef LOONGSON3A_3A780E
		wrldn(LDN_SHM, SHAW2BA_3, ((WCB_BASE_ADDR & 0x1F000000) >> 24));
#endif
#ifdef LOONGSON2F_3GNB
		wrldn(LDN_SHM, SHAW2BA_3, ((WCB_BASE_ADDR & 0xFF000000) >> 24) & 0x0F);
#endif
	}
	wrldn(LDN_SHM, WIN_CFG, rdldn(LDN_SHM, WIN_CFG) & ~SHWIN_ACC_FWH);
}

void Init_Flash_Command(unsigned char rd_devid_cmd)
{
	FLASH_commands.read_device_id = rd_devid_cmd;
	FLASH_commands.write_status_enable = CMD_WRITE_STAT_EN;
	FLASH_commands.write_enable = CMD_WRITE_EN;
	FLASH_commands.read_status_reg = CMD_READ_STAT;
	FLASH_commands.write_status_reg = CMD_WRITE_STAT;
	FLASH_commands.page_program = CMD_PROG;
	FLASH_commands.sector_erase = CMD_SECTOR_ERASE;
	FLASH_commands.status_busy_mask = STATUS_BUSY_MASK;
	FLASH_commands.status_reg_val = STATUS_REG_VAL;
	FLASH_commands.program_unit_size = PROG_SIZE;
	FLASH_commands.page_size = EC_PAGE_SIZE - 1;
	FLASH_commands.read_dev_id_type = READ_DEV_ID_TYPE;

	if(verbose)
		printf("\nSHMFlashInit(%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x)\n", 
			FLASH_commands.read_device_id, 
			FLASH_commands.write_status_enable,
			FLASH_commands.write_enable, 
			FLASH_commands.read_status_reg, 
			FLASH_commands.write_status_reg, 
			FLASH_commands.page_program, 
			FLASH_commands.sector_erase, 
			FLASH_commands.status_busy_mask,
			FLASH_commands.status_reg_val, 
			FLASH_commands.program_unit_size, 
			FLASH_commands.page_size,
			FLASH_commands.read_dev_id_type);

	/* Clear protocol. */
	wrwcb(WCB_BASE_ADDR, rdwcb(WCB_BASE_ADDR) & ~SHAW2_SEM_EXE);	// Clear EXE bit
	// Setup the WCB for Init
	WCB.Command	= FLASH_COMMANDS_INIT_OP;
	WCB.Param.InitCommands = FLASH_commands; 
	SHMRunCommand(1 + sizeof(FLASH_device_commands_t));	          // Execute the Command in WCB
}

/******************************************************************************
 * Function: PcMemWrite
 *           
 * Purpose:  write a block to PC physical memory
 *           
 * Params:   Address - the PC physical memory address to write into
 *           NoOfBytes - total number of bytes to write
 *           UnitSize - the unit size access that will be used
 *                      (1-BYTE, 2-WORD, 4-DWORD)
 *           Data - pointer to the buffer that stores the block to write
 *           
 * Returns:  1 if successful
 *           0 in the case of an error.
 *           
 ******************************************************************************/
int PcMemWrite(unsigned long Address, unsigned long NumOfBytes, int UnitSize, const void* Data)
{    
    int i;
    unsigned char const *ByteDataP = Data;
    unsigned short const *WordDataP = Data;
    unsigned long const *DWordDataP = Data;

	if(verbose) printf("PcMemWrie: NumOfBytes 0x%x, UnitSize %d\n", NumOfBytes, UnitSize);
	switch(UnitSize)
	{
		case 1:
			/* write WCB.command byte */
			*(unsigned char *)(Address) = *ByteDataP;
			/* pointer to WCB DATA byte, because the 4-byte aligned */
			Address++;
			ByteDataP += 4;
			break;
		case 2:
			/* write WCB.command byte */
			*(unsigned char *)(Address++) = *WordDataP;
			/* pointer to WCB DATA byte, because the 4-byte aligned */
			WordDataP += 1;
			break;
		case 4:
			/* write WCB.command byte */
			*(unsigned char *)(Address++) = *DWordDataP;
			/* pointer to WCB DATA byte, because the 4-byte aligned */
			break;
		default:
			return 0;
	}

    for (i = 0; i < NumOfBytes - 1; i += UnitSize)
    {
        switch(UnitSize)
        {
            case 1:
                *(unsigned char *)(Address + i) = *ByteDataP;
                ByteDataP++;
                break;
			case 2:
			    *(unsigned short *)(Address + i) = *WordDataP;
			    WordDataP++;
			    break; 
			case 4:
				*(unsigned long *)(Address + i)= *DWordDataP;
				DWordDataP++;
				break;
			default:
			    return 0;
		}
	} 

	return 1;
}

void SHMRunCommand(unsigned short cmd_length)
{
	unsigned char sem_val = 0;
	unsigned long timeout = 100000;
	/* Wait until firmware is ready to accept a command - i.e. EXE and RDY bits in semaphore are cleared. */
	do{
		sem_val = rdwcb(WCB_BASE_ADDR);
	}while((sem_val & (SHAW2_SEM_EXE | SHAW2_SEM_RDY)) && (timeout--));
	
	if(!timeout){
		printf("Firmware is not ready to accept command (semaphore = 0x%x)\n", sem_val);
		exit(-1);
	}

	/* Write Command Buffer to memory */
	PcMemWrite(WCB_BASE_ADDR + OFFSET_COMMAND, cmd_length, sizeof(unsigned char), &WCB);
	/* Enable EXE bit. */
	wrwcb(WCB_BASE_ADDR, rdwcb(WCB_BASE_ADDR) | SHAW2_SEM_EXE);

	if(WCB.Command == RESET_EC_OP){
		return;
	}

	/* Wait until done (indicated by RDY) */
	timeout = 100000;
	do{
		sem_val = rdwcb(WCB_BASE_ADDR);
	}while((!(sem_val & SHAW2_SEM_RDY)) && (timeout--));

	if(!timeout){
		printf("Firmware did not set the RDY bit (semaphore = 0x%x)\n", sem_val);
		exit(-1);
	}

	/* Clear EXE bit. */
	wrwcb(WCB_BASE_ADDR, rdwcb(WCB_BASE_ADDR) & ~SHAW2_SEM_EXE);

	if(sem_val & SHAW2_SEM_ERR){
		printf("ERROR: Firmware reported error (semaphore value %x). Aborting\n", sem_val);
		exit(-1);
	}

	/* Wait until Host clears RDY bit to acknowledge operation end. */
	timeout = 100000;
	do{
		sem_val = rdwcb(WCB_BASE_ADDR);
	}while((sem_val & SHAW2_SEM_RDY) && (timeout--));

	if(!timeout){
		printf("Host did not clear the RDY bit (semaphore = 0x%x)\n", sem_val);
		exit(-1);
	}
}

//-----------------------------------------------------------------------------
// Function:
//    Enter_Flash_Update
// Description:
//    Uses the WCB protocol to indicate the beginning of Flash Update procedure.
//    
// Parameters:
//    None.
// Return Value:
//   None.
//-----------------------------------------------------------------------------
void Enter_Flash_Update(void)
{
    // Clear protocol
    wrwcb(WCB_BASE_ADDR, rdwcb(WCB_BASE_ADDR) & ~SHAW2_SEM_EXE);	// Clear EXE bit

    // Setup the WCB for Enter
    WCB.Command	= ENTER_OP;
    WCB.Param.EnterCode	= WCB_ENTER_CODE;
    SHMRunCommand(WCB_SIZE_ENTER_CMD);	     // Execute the Command in WCB
}

void Flash_write_byte(u8 data)
{
    // Clear protocol
    wrwcb(WCB_BASE_ADDR, rdwcb(WCB_BASE_ADDR) & ~SHAW2_SEM_EXE);	// Clear EXE bit

    // Setup the WCB for Enter
    WCB.Command	= PROGRAM_OP;
    WCB.Param.Byte = data;
    SHMRunCommand(WCB_SIZE_BYTE_CMD);     // Execute the Command in WCB
}

//-----------------------------------------------------------------------------
// Function:
//    Exit_Flash_Update
// Description:
//    Uses the WCB protocol to indicate the end of Flash Update procedure.
//    
// Parameters:
//    exit type   Type of termination to indicate through WCB protocol
// Return Value:
//   None.
//-----------------------------------------------------------------------------
void exit_flash_update(WCB_exit_t exit_type)
{
    /* Setup the WCB for Exit */
    switch (exit_type)
    {
    case WCB_EXIT_NORMAL:
        WCB.Command	= EXIT_OP;
        break;
    case WCB_EXIT_GOTO_BOOT_BLOCK:
        WCB.Command	= GOTO_BOOT_BLOCK_OP;
        break;
    case WCB_EXIT_RESET_EC:
        WCB.Command	= RESET_EC_OP;
        break;
    }
    
    SHMRunCommand(WCB_SIZE_EXIT_CMD);	     // Execute the Command in WCB
}

unsigned int Read_Flash_IDs(void)
{
	unsigned int ids = 0, i;

	/* Setup the WCB for Read ID */
	WCB.Command = READ_IDS_OP;
	SHMRunCommand(WCB_SIZE_READ_IDS_CMD);
	for(i = 0; i < 3; i++){
		ids |= rdwcb(WCB_BASE_ADDR + OFFSET_DATA + i) << (i * 8);
	}

	return ids;
}

/* Write flash status register. 
 * Parameters: 
 *     data: 0 = unprotected, 1 = protected, other = value of write status register
 * Return Value:
 *     None
 */
void Flash_write_status_register(u8 data)
{
	if(verbose) printf("Write Flash Status Register...\n");

	/* Setup the WCB for Write Status Register. */
	WCB.Command = WRITE_STSREG_OP;
    WCB.Param.Byte = data;
	SHMRunCommand(WCB_SIZE_BYTE_CMD);
}

/* Read flash status register. */ 
unsigned char Flash_read_status_register(void)
{
	unsigned char status;

	/* Setup the WCB for Read flash status register */
	WCB.Command = READ_STSREG_OP;
	SHMRunCommand(WCB_SIZE_READ_STS_CMD);
	status = rdwcb(WCB_BASE_ADDR + OFFSET_DATA);

	return status;
}

//-----------------------------------------------------------------------------
// Function:
//    flash_sector_erase
// Description:
//    Uses the WCB protocol to request erase of the sector in which Address is
//    located.
//    
// Parameters:
//   Address     Address of a byte located in the sector which is to be erased.
// Return Value:
//   None.
//-----------------------------------------------------------------------------
void flash_sector_erase(unsigned long Address)
{
    // Setup the WCB for Sector Erase
    WCB.Command	= ERASE_OP;		    // Erase Command
    WCB.Param.Address = Address;    // Address which resides in the sector to be erased
    SHMRunCommand(WCB_SIZE_ERASE_CMD);     // Execute the Command in WCB
	
	if(verbose) printf("Flash sector erase OK.\n");
}

//-----------------------------------------------------------------------------
// Function:
//    Flash_Set_Address
// Description:
//    Uses the WCB protocol to set the flash address from which flash will
//    be programmed.
//    
// Parameters:
//   Address     Address of the data to be programmed
// Return Value:
//   None.
//-----------------------------------------------------------------------------
void Flash_Set_Address(unsigned long Address)
{
    /* Setup the WCB for Set Address */
    WCB.Command = ADDRESS_SET_OP;   		 /* Byte Program */
    WCB.Param.Address = Address;	         /* Address to program */
    SHMRunCommand(WCB_SIZE_ADDRESS_SET_CMD); /* Execute the Command in WCB */
}

/* Program data to SPI data for test command "updec". */
void Flash_program_data(unsigned long addr, u8 *src, u32 size)
{
	u8 *buf;
	u16 sector, page_cnt = 0; 
	u16 page, page1, i, j, sec_cnt = 0;
	u16 id = 0;
	unsigned long program_addr, erase_addr;
	u8 flags, remainder, status;
	u8 debug_msg = 0;

	verbose = 0;
	exit_type = WCB_EXIT_NORMAL;
	sector = 1;
	page = 1;
	erase_addr = program_addr = addr;
	buf = src;

	printf("Progarm data to address 0x%lx, size %d\n", addr, size);

	remainder = size % 256;

	/* start sector erase active */
	if(size > 0x1000){
		sector = size / 0x1000 + 1;
	}
	if(size > 256){
		page = size / 256 + 1;
	}
	if(debug_msg) printf("The sum total has %d sectors to need to erase:\n", sector);

	Update_Flash_Init();
	set_interface_cfg(0);
	Init_Flash_Command(CMD_READ_DEV_ID);

	printf("Ready enter flash update...\n");
	/* Indicate Flash Update beginning to Firmware */
	Enter_Flash_Update();

	id = Read_Flash_IDs();
    if (id != FLASH_ID) {
        printf("ERROR: Wrong flash Device ID.  Expected %x ,  Found %x .\nAborting\n", FLASH_ID, id);
        exit(-1);
    }
	printf("Read flash ids: 0x%x\n", id);

	/* unlock block protection active. */
	status = Flash_read_status_register();
	if(status & 0x1C){
		Flash_write_status_register(status & ~0x1C);
		if(Flash_read_status_register() & 0x1C){
			printf("Block protection unlocked failed.\n");
			goto out;
		}else{
			printf("Block protection unlocked OK.\n");
		}
	}else{
		printf("No block protection.\n");
	}

	/* Start sector erase active. */
	while(sector){
		if(debug_msg) printf("Starting erase sector %d ...\n", sec_cnt);
		flash_sector_erase(erase_addr);
		if(debug_msg) printf("Erase sector %d OK...\n", sec_cnt++);

		if((page * 256) > (4 * 1024)){
			page1 = (4 * 1024) / 256;
			flags = 1;
		}else{
			page1 = page;
			flags = 0;
		}

		Init_Flash_Command(CMD_READ_DEV_ID);
		/* start page program active */
		if(debug_msg) printf("The sum total has %d page to need to program:\n", page);
		while(page1){
			if(!flags){
				if(size > 256){
					j = 256;
				}
				else{
					j = size;
				}
			}
			else{
				j = 256;
			}
			for(i = 0; i < j; i++){
				Flash_Set_Address(program_addr);
				Flash_write_byte(*buf);
				if(!(i % 0xF)) printf(".");
				program_addr++;
				buf++;
				size--;
			}
			if(debug_msg) printf("\npage %d program ok.\n", page_cnt++);
			page1--;
		}
		page -= (4 * 1024) / 256;
		if(debug_msg) printf("All page programming completes for sector %d.\n", sec_cnt - 1);

		erase_addr += 0x1000;
		sector--;
	}

	/* lock block protection active. */
	Flash_write_status_register(status | 0x1C);
	if(Flash_read_status_register() & 0x1C){
		printf("\nBlock protection locked OK.\n");
	}else{
		printf("Block protection locked failed.\n");
	}
	if(debug_msg) printf("All page programming completes for all sectors.\n");
	printf("Program data to wpce775l flash done.\n");

out:
	/* Indicate Flash Update termination to Firmware */
	exit_flash_update(exit_type);
}
 

//////////////////////////////////////////////////////////////////////////////////////////
// Update Flash OK, use Byte Write to finished, daway 2010-05-21

/* program data to SPI data */
int ec_update_rom(void *src, int size)
{
	u8 *buf;
	unsigned long addr = 0;
	u16 sector, page_cnt = 0; 
	u16 page, page1, i, j, sec_cnt = 0;
	unsigned long program_addr, erase_addr;
	u8 flags, status;
	u8 val;
	u8 data;
	u16 id;
	u8 debug_msg = 1;
	u8 verbose1 = 0;
	
	verbose = 0;
    exit_type = WCB_EXIT_RESET_EC;
	sector = 1;
	page = 1;
	erase_addr = program_addr = addr;
	buf = src;
	
	if(size > EC_ROM_MAX_SIZE){
        printf("ec-update : out of range.\n");
        return 1;
	}
	
    printf("starting update ec ROM...\n");

	/* start sector erase active */
	if(size > 0x1000){
		sector = size / 0x1000 + 1;
	}
	if(size > 256){
		page = size / 256 + 1;
	}
	
	Update_Flash_Init();
    set_interface_cfg(0);
	Init_Flash_Command(CMD_READ_DEV_ID);

	if(debug_msg) printf("Ready enter flash update...\n");
    /* Indicate Flash Update beginning to Firmware */
    Enter_Flash_Update();

	id = Read_Flash_IDs();
    if (id != FLASH_ID) {
        printf("ERROR: Wrong flash Device ID.  Expected %x ,  Found %x .\nAborting\n", FLASH_ID, id);
        exit(-1);
    }
	printf("Read flash ids: 0x%x\n", id);

	/* unlock block protection active. */
	status = Flash_read_status_register();
	if(status & 0x1C){
		Flash_write_status_register(status & ~0x1C);
		if(Flash_read_status_register() & 0x1C){
			printf("Block protection unlocked failed.\n");
			goto out;
		}else{
			if(debug_msg) printf("Block protection unlocked OK.\n");
		}
	}else{
		if(debug_msg) printf("No block protection.\n");
	}

	/* Start sector erase active. */
	while(sector){
		if(verbose1) printf("Starting erase sector %d ...\n", sec_cnt);
		flash_sector_erase(erase_addr);
		if(verbose1) printf("Erase sector %d OK...\n", sec_cnt++);
		sec_cnt++;
		
		if((page * 256) > (4 * 1024)){
			page1 = (4 * 1024) / 256;
			flags = 1;
		}else{
			page1 = page;
			flags = 0;
		}
		
		Init_Flash_Command(CMD_READ_DEV_ID);
		/* start page program active */
		if(verbose1) printf("The sum total has %d page to need to program:\n", page);
		while(page1){
			if(!flags){
				if(size > 256){
					j = 256;
				}
				else{
					j = size;
				}
			}
			else{
				j = 256;
			}
			for(i = 0; i < j; i++){
				data = *buf;
				Flash_Set_Address(program_addr);
				/* program a byte data */
				Flash_write_byte(data);
				/* Enable shared flash access. */
    			set_interface_cfg(1);
				val = PcMemReadB(FLASH_WIN_BASE_ADDR + program_addr);
				/* Enable shared ram access. */
    			set_interface_cfg(0);
				/* Verify data. */
				if(val != data){
					Flash_Set_Address(program_addr);
					Flash_write_byte(data);
					set_interface_cfg(1);
					val = PcMemReadB(FLASH_WIN_BASE_ADDR + program_addr);
					set_interface_cfg(0);
					if(val != data){
						printf("EC : Second flash program failed at:\t");
						printf("addr : 0x%x, source : 0x%x, dest: 0x%x\n", program_addr, data, val);
						printf("This should not happened... STOP\n");
						printf("Update wpce775l failed.\n");
						goto out;
					}				
				}
				program_addr++;
				buf++;
				size--;
			}
			if(!(page1 % 4)) printf(".");
			page_cnt++;
			page1--;
		}
		page -= (4 * 1024) / 256;
		
		erase_addr += 0x1000;
		sector--;
	}
	printf("\n");
	
	/* lock block protection active. */
	Flash_write_status_register(status | 0x1C);
	if(Flash_read_status_register() & 0x1C){
		if(debug_msg) printf("Block protection locked OK.\n");
	}else{
		if(debug_msg) printf("Block protection locked failed.\n");
	}
	if(verbose1) printf("All page programming completes for all sectors.\n");
	printf("Update wpce775l done.\n");
out:
    /* Indicate Flash Update termination to Firmware */
    exit_flash_update(exit_type);

	return 0;
}

/* get EC version from EC rom */
unsigned char *get_ecver(void)
{
	int i;
	unsigned char *p;
	static unsigned char val[OEMVER_MAX_SIZE];

	//val[0] = ec_read(CMD_RD_VER, 0);
	for(i = 0; i < OEMVER_MAX_SIZE && (val[i] = ec_read(CMD_RD_VER, i)) != '\0'; i++){
        delay(10);
		//val[i] = ec_read(CMD_RD_VER, i);
	}
	p = val;
	if (*p == 0){
		p = "undefined";
	}

	return p;
}
//#endif	// end ifdef LOONGSON2F_3GNB
