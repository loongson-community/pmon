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

#ifdef LOONGSON2F_3GNB
#define boolean unsigned char
WCB_struct	     WCB;
FLASH_device_commands_t FLASH_commands;
unsigned int FLASH_id;
unsigned int FLASH_size;
unsigned int FLASH_sector_size;
unsigned int FLASH_block_size;
unsigned int FLASH_page_size;
unsigned char FLASH_block_erase, FLASH_sector_erase;
unsigned char protected_sectors[64];
Sector_t Flash_sectors[20];
int NumOfSectoresInBlock;
int FlashLastblock;
int NumOfSectors;
int NumOfBlocks;
int PrintProgres;
SHM_mem_access_t SHM_interface;
WCB_exit_t exit_type;                           // Type of protocol termination to use
int verbose = 0;
unsigned char    Flash_Data[MAX_FLASH_CAPACITY];
static boolean UseProtectionWindow = false;

const char PP[4] = {'|','/','-','\\'};

#if 1 // daway added for test update flash, 2010-05-10

//-----------------------------------------------------------------------------
// Function:
//    Set_Window_Base_Address
// Description:
//    Sets the base address of the Shared Memory Window 2.
//    This window is used to implement the Windows Command Buffer.
//    
// Parameters:
//   BaseAddr     Address to be set as base for host shared memory access
// Return Value:
//   None.
//-----------------------------------------------------------------------------
static void Set_Window_Base_Address(unsigned long BaseAddr)
{
    if(verbose) printf("SetWindowBaseAddress(BaseAddr = %Xh) \n", BaseAddr);

    PcIoWriteB(SIO_INDEX_PORT, SHAW2BA_0);			// Configure address bits 0-7
    PcIoWriteB(SIO_DATA_PORT, (unsigned char)((BaseAddr) & 0xFF));
    PcIoWriteB(SIO_INDEX_PORT, SHAW2BA_1);			// Configure address bits 8-15
    PcIoWriteB(SIO_DATA_PORT, (unsigned char)((BaseAddr >> 8) & 0xFF));
    PcIoWriteB(SIO_INDEX_PORT, SHAW2BA_2);			// Configure address bits 16-23
    PcIoWriteB(SIO_DATA_PORT, (unsigned char)((BaseAddr >> 16) & 0xFF));
    PcIoWriteB(SIO_INDEX_PORT, SHAW2BA_3);           // Configure address bits 24-31
    PcIoWriteB(SIO_DATA_PORT, (unsigned char)((BaseAddr >> 24) & 0x0F));
}

//-----------------------------------------------------------------------------
// Function:
//    Set_Flash_Window
// Description:
//    Sets the base address of the Shared Flash Window 1.
//    This window allows direct flash read access to the Host.
//    
// Parameters:
//   BaseAddr     Address to be set as base for host flash access
//   FlashSize    Size of flash window (must be size of total flash)
// Return Value:
//   None.
//-----------------------------------------------------------------------------
static void Set_Flash_Window(unsigned long BaseAddr, unsigned long FlashSize)
{
    if(verbose) printf("SetFlashWindow(BaseAddr = %Xh)\n", BaseAddr);

    // Select shared window 1 for either flash or RAM access
    PcIoWriteB(SIO_INDEX_PORT, SHM_CFG);
    if (BaseAddr != 0)
    {
        // Flash window base address must be aligned to flash size
        if (BaseAddr % FlashSize)
            return;

        // Select Flash Access
        PcIoWriteB(SIO_DATA_PORT, PcIoReadB(SIO_DATA_PORT) | FLASH_ACC_EN);
    }
    else
    {
        // Disable Flash Access
        PcIoWriteB(SIO_DATA_PORT, PcIoReadB(SIO_DATA_PORT) & ~FLASH_ACC_EN);
    }

    // Configure the flash window size according to total flash size
    PcIoWriteB(SIO_INDEX_PORT, WIN_CFG);
    switch (FlashSize)
    {
    case (512*1024):
        PcIoWriteB(SIO_DATA_PORT, (unsigned char)FWIN1_SIZE_512_KB |
                              (PcIoReadB(SIO_DATA_PORT) & ~FWIN1_SIZE_MASK));
        break;
    case (1024*1024):
        PcIoWriteB(SIO_DATA_PORT, (unsigned char)FWIN1_SIZE_1_MB |
                              (PcIoReadB(SIO_DATA_PORT) & ~FWIN1_SIZE_MASK));
        break;
    case (2 * 1024*1024):
        PcIoWriteB(SIO_DATA_PORT, (unsigned char)FWIN1_SIZE_2_MB |
                              (PcIoReadB(SIO_DATA_PORT) & ~FWIN1_SIZE_MASK));
        break;
    case (4 * 1024*1024):
        PcIoWriteB(SIO_DATA_PORT, (unsigned char)FWIN1_SIZE_4_MB |
                              (PcIoReadB(SIO_DATA_PORT) & ~FWIN1_SIZE_MASK));
        break;
    case 0:
        break;
    default:
        // Illegal flash size
        exit(-1);
    }

    PcIoWriteB(SIO_INDEX_PORT, SHAW1BA_2);			// Configure address bits 17-23
    PcIoWriteB(SIO_DATA_PORT, (unsigned char)((BaseAddr >> 16) & 0xFE) |
                              (PcIoReadB(SIO_DATA_PORT) & 0x1));

    if (SHM_interface == SHM_MEM_LPC)
    {
        PcIoWriteB (SIO_INDEX_PORT, SHAW1BA_3);          
        PcIoWriteB (SIO_DATA_PORT, (unsigned char)((BaseAddr >> 24) & 0xFF)); // Configure address bits 24-31
    }
    else  // FWH
    {
        PcIoWriteB (SIO_INDEX_PORT, SHAW1BA_3);          // Configure address bits 24-27
        PcIoWriteB (SIO_DATA_PORT, (unsigned char)((BaseAddr >> 24) & 0x0F) |
                              (PcIoReadB(SIO_DATA_PORT) & 0xF0));
    }	
}

//-----------------------------------------------------------------------------
// Function: 
//    LDNSelectDevice
// Description:
//   Select a specified Logical Device to be the currently addressed device
// Parameters:
//   LDNNum - Logical Device Number of the requested SIO logcal device
// Return Value:
//   None.
// Side Effects:
//   The logical device to be addressed by all LDN functions remains the same
//   until the next call to this function.
//-----------------------------------------------------------------------------
static void LDNSelectDevice(unsigned char LDNNum)
{
    PcIoWriteB(SIO_INDEX_PORT, REG_SLDN);     // Logical device number register
    PcIoWriteB(SIO_DATA_PORT,  LDNNum);      // Select LDN
    if(verbose) printf("LDNSelectDevice(Device = SIO, LDNNum = %02Xh)\n", LDNNum);
}

//-----------------------------------------------------------------------------
// Function: 
//    LDNEnableDevice
// Description:
//   Sets the enable state of the currently selected Logical Device.
// Parameters:
//   En          TRUE - enable the device
//               FALSE - disable the device
// Return Value:
//   None.
//-----------------------------------------------------------------------------
static void LDNEnableDevice(boolean En)
{   
    PcIoWriteB(SIO_INDEX_PORT, REG_ACTIVATE);            // Select Activate register
    if (En)
        PcIoWriteB(SIO_DATA_PORT, PcIoReadB(SIO_DATA_PORT) | ACTIVATE_EN);
    else
        PcIoWriteB(SIO_DATA_PORT, PcIoReadB(SIO_DATA_PORT) & ~ACTIVATE_EN);
    if(verbose) printf("LDNEnable(Device = SIO, En = %s)\n", En?"True":"False");
}

void Update_Flash_Init(void)
{
	FLASH_block_size = EC_BLOCK_SIZE;
	FLASH_size = EC_FLASH_SIZE;
	FLASH_sector_size = EC_SECTOR_SIZE * 1024;
	FLASH_page_size = EC_PAGE_SIZE;
	FLASH_block_erase = CMD_BLOCK_ERASE;
	NumOfSectoresInBlock = 16;
}

void set_interface_cfg(u8 flag)
{
	if(flag){
		/* Enable shared flash access. */
		wrldn(LDN_SHM, SHM_CFG, FLASH_ACC_EN);
		wrldn(LDN_SHM, SHAW1BA_2, FLASH_WIN_BASE_ADDR >> 16);
		wrldn(LDN_SHM, SHAW1BA_3, (FLASH_WIN_BASE_ADDR >> 24) & 0x0F);
	}else{
		/* Enable shared ram access. */
		wrldn(LDN_SHM, SHM_CFG, rdldn(LDN_SHM, SHM_CFG) & ~FLASH_ACC_EN);
		wrldn(LDN_SHM, SHAW2BA_2, WCB_BASE_ADDR >> 16);
		wrldn(LDN_SHM, SHAW2BA_3, (WCB_BASE_ADDR >> 24) & 0x0F);
	}
	wrldn(LDN_SHM, WIN_CFG, rdldn(LDN_SHM, WIN_CFG) & ~SHWIN_ACC_FWH);
}

void Init_Flash_Command(void)
{
	FLASH_commands.read_device_id = CMD_READ_DEV_ID;
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
int PcMemWrite(u32 Address, unsigned long NumOfBytes, int UnitSize, const void* Data)
{    
    int i;
    unsigned char const *ByteDataP = Data;
    unsigned short const *WordDataP = Data;
    unsigned long const *DWordDataP = Data;

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

unsigned short Read_Flash_IDs(void)
{
	unsigned char val;
	unsigned short ids;

	/* Setup the WCB for Read ID */
	WCB.Command = READ_IDS_OP;
	SHMRunCommand(WCB_SIZE_READ_IDS_CMD);
	val = rdwcb(WCB_BASE_ADDR + OFFSET_DATA);
	ids = (((unsigned short)rdwcb(WCB_BASE_ADDR + OFFSET_DATA + 1)) << 8) | val;
	if(verbose) printf("IDs value : 0x%x\n", ids);

	return ids;
}

/* Write flash status register. 
 * Parameters: 
 *     data: 0 = unprotected, 1 = protected, other = value of write status register
 * Return Value:
 *     None
 */
void write_status_register(u8 data)
{
	if(verbose) printf("Write Flash Status Register...\n");

	/* Setup the WCB for Write Status Register. */
	WCB.Command = WRITE_STSREG_OP;
    WCB.Param.Byte = data;
	SHMRunCommand(WCB_SIZE_BYTE_CMD);
}

/* Read flash status register. */ 
unsigned char read_status_register(void)
{
	unsigned char status;

	/* Setup the WCB for Read ID */
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
    // Setup the WCB for Set Address
    WCB.Command				 = ADDRESS_SET_OP;   // Byte Program
    WCB.Param.Address		 = Address;	         // Address to program

    SHMRunCommand(WCB_SIZE_ADDRESS_SET_CMD);	// Execute the Command in WCB
}

//-----------------------------------------------------------------------------
// Function:
//    Flash_Program
// Description:
//    Uses the WCB protocol to request programming of the specified number of
//    bytes in the flash device with the provided data.
//    
// Parameters:
//   Data        Pointer to the lsb byte of the data to be programmed
//   Size        Number of bytes of the Data to be programmed
// Return Value:
//   None.
//-----------------------------------------------------------------------------
void Flash_Program(unsigned char * Data, unsigned int Size)
{
 	unsigned short i, Length;
	unsigned long * WCBPointer, * DataPointer;

	if ((Size < 1) || (Size > 8))
		return;

	// Setup the WCB for Program

	WCB.Command = PROGRAM_OP | ((unsigned char)Size & 0xF);

	if (Size == sizeof(unsigned char))
	{
		WCB.Param.Byte = *Data;
		Length = WCB_SIZE_BYTE_CMD;
	}
	else
	if (Size == sizeof(unsigned short))
	{
		WCB.Param.Word = *(unsigned short *)Data;
		Length = WCB_SIZE_WORD_CMD;
	}
	else
	if (Size == sizeof(unsigned long))
	{
		WCB.Param.DWord = *(unsigned long *)Data;
        Length = WCB_SIZE_DWORD_CMD;
	}

    else
 	if (Size == sizeof(unsigned long) * 2)   {
		WCBPointer = (unsigned long *)&WCB.Param.EightBytes.DWord1;
        DataPointer = (unsigned long *)Data;
		Length = WCB_SIZE_LONGLONG_CMD;

        for (i = 0; i < 2; i ++)                   // Copy two dwords
        {
            *WCBPointer = *DataPointer;
            WCBPointer++;
            DataPointer++;
        }
    }
	if(verbose)
		printf("WCB.Param.EightBytes: DWord1 0x%x, DWord2 0x%x\n",
				WCB.Param.EightBytes.DWord1, WCB.Param.EightBytes.DWord2);

	SHMRunCommand(Length);							    // Execute the Command in WCB
}

void program_flash(unsigned long Address, unsigned char *src, unsigned long size){
    unsigned long LastAddress;
    unsigned long Addr;
    unsigned long start_address;
    unsigned char data, *pntr;
    int error, retry;
    
	pntr = src;
    printf("program_flash: address 0x%x, data %s\n", Address, pntr); 
    retry = 3;
    do {
        error = 0;
        retry--;
        printf("%cProgress: Address [%X] |", 0xD,Address); 
        Flash_Set_Address(Address);
        LastAddress = Address + size;
        for (Addr = Address; Addr < LastAddress; Addr += 8, pntr += 8) {
        //for (Addr = Address; Addr < LastAddress; Addr++, pntr++) {
    		printf("program_flash (for loop): address 0x%x, last address 0x%x, data %s\n", Address, LastAddress, pntr);
            Flash_Program(pntr, 8);
            //Flash_Program(pntr, 1);
           /* if ((Addr & 0x3ff) == 0) {
                printf("%c%c",0x08,PP[PrintProgres]);
                PrintProgres ++;
                if (PrintProgres == 4){
                    PrintProgres = 0;
                }
            }*/
        }

		pntr = src;
        // Verify data.
        printf("%cV",0x08);
		set_interface_cfg(1);
        start_address = FLASH_WIN_BASE_ADDR + Address;
        if (verbose) printf(" Verify data from address 0x%x to 0x%x, start address 0x%x\n", Address, Address + size, start_address);
        for (Addr = start_address; (Addr < (start_address + size)) && !error ; Addr += 8, pntr += 8) 
        //for (Addr = start_address; (Addr < (start_address + size)) && !error ; Addr++, pntr++) 
		{
            data = PcMemReadB(Addr);
            if (data != *pntr) {
                error++;
                if (verbose) printf("Flash program error: Address = 0x%x, Data = 0x%x, Expected = 0x%x\n",
						Addr, data, *pntr);
				set_interface_cfg(0);
				//flash_sector_erase(Address);
				break;
            }
        }
    } while (retry && error);
    if (error) {
        printf("\n******************************************************************\n");
        printf("ERROR: Fail programming address %X to %X. Aborting", Address, Address + size);
        printf("\n******************************************************************\n");
	    // Indicate Flash Update termination to Firmware
    	exit_flash_update(WCB_EXIT_NORMAL);
        exit(-1);
    }
}

/* test write flash and read flash, daway added 2010-05-10 */
int ec_update_rom_test(u32 Address, void *src, int size)
{
    unsigned long   Addr;                        // Current flash address being programmed
    unsigned long   start_address, last_address;                  // Offset in flash to start programming from
    unsigned long   Data;                           // Data read from flash
    unsigned long   flash_byte;                     // Offset of data in binary file which is being programmed
    unsigned short  id;
    int i, sector, LastSector;    
	unsigned char *pntr;
	unsigned char status;

    // **********************************************************************
    // Initialize variables to default values
    // **********************************************************************
    verbose                = 1;
    start_address          = FLASH_WIN_BASE_ADDR;
    exit_type              = WCB_EXIT_NORMAL;
	pntr = src;

	//printf("start program: Address 0x%x, data \"%s\", zize %d, flash window base address 0x%x\n",
	//		Address, pntr, size, start_address);

	if(!(size % (EC_SECTOR_SIZE * 1024)))	/* Is a multiple of 4KB. */
	{
		NumOfSectors = size / (EC_SECTOR_SIZE * 1024);
	} else
	{
		NumOfSectors = size / (EC_SECTOR_SIZE * 1024) + 1;
	}

#if 0
	if(NumOfSectors % (EC_BLOCK_SIZE * 1024))	/* Is a multiple of 64KB. */
	{
		NumOfBlocks = NumOfSectors / NumOfSectoresInBlock;
	} else
	{
		NumOfBlocks = NumOfSectors / NumOfSectoresInBlock + 1;
	}
#endif
	printf("program test: address 0x%x, data %s, size %d, sectors: %d\n",
			Address, pntr, size, NumOfSectors);

    // **********************************************************************
    // Program the flash through the WCB
    // **********************************************************************
    // Define the flash device in use
	
	Update_Flash_Init();
    set_interface_cfg(0);
	Init_Flash_Command();

	printf("Ready enter flash update...\n");
    // Indicate Flash Update beginning to Firmware
    Enter_Flash_Update();

	//Init_Flash_Command();
   /* id = Read_Flash_IDs();
    if (id != FLASH_ID) {
        printf("ERROR: Wrong flash Device ID.  Expected %x ,  Found %x .\nAborting\n", FLASH_ID, id);
        exit(-1);
    }
	printf("Read_flash_ids: id %s\n", id);
*/
#if 1
	Addr = Address;
	last_address = Address + size;
	Init_Flash_Command();
	/* unlock block protection active */
    //printf("Ready unlock block protection...\n");
	//status = generic_operation(CMD_READ_STAT, 0, 0, 0, 0, 0);
    //printf("Read status register: 0x%x\n", status);
	//Init_Flash_Command();
	//if(status & 0x1C){
	//generic_operation(CMD_WRITE_STAT, 0, 0, 0, 0, 0);
	//}
	//Init_Flash_Command();
	//status = generic_operation(CMD_READ_STAT, 0, 0, 0, 0, 0);
    //printf("After unlock, read status register: 0x%x\n", status);

	for(sector = 0; sector < NumOfSectors; sector++)
	{
    	printf("Program sector %d: address 0x%x, last address 0x%x, data: %s, size %d\n", sector, Addr, last_address,  pntr, size);
		if(Addr < last_address)
		{
			/* Send Sector Erase command. */
			Init_Flash_Command();
			flash_sector_erase(Addr);
			program_flash(Addr, pntr, size);
			Addr += FLASH_sector_size;
			if(size > FLASH_sector_size){
				pntr += FLASH_sector_size;
			}
		}
	}
	/* lock block protection active */
	//Init_Flash_Command();
	//status = generic_operation(CMD_READ_STAT, 0, 1, 0, 0, 0);
    //printf("Program end, read status register: 0x%x\n", status);
	//Init_Flash_Command();
	//generic_operation(CMD_WRITE_STAT, 0, 0, 0, 0, 0);
    printf("Program flash completed, ready verifying data in flash...\n");

#endif

    // Indicate Flash Update termination to Firmware
    exit_flash_update(exit_type);

    // **********************************************************************
    // Verify the flash only for EB with shm disable
    // **********************************************************************

#if 0
    // Set base address for Shared Flash window
	set_interface_cfg(1);
	Addr = Address;
    printf("Verifying data in flash... ");
    for(sector = 0; sector < NumOfSectors ; sector++)
	{
       //// if (!protected_sector(sector))
		////{
        if (Addr >= size)
		{
            break;
        }
        for (flash_byte = Addr; flash_byte < (Addr + FLASH_sector_size); flash_byte++)
        {
            Data = PcMemReadB(start_address + flash_byte);
                
            if (Data != Flash_Data[flash_byte])
	    	{
                printf("Flash program error: Address = %08Xh, Data = %Xh, Expected = %Xh\n",
                        start_address + flash_byte,
                        Data, Flash_Data[flash_byte]);
                exit(-1);
            }
        }
		Addr += FLASH_sector_size;
    }
#endif

    printf("Done!\n");
    
    return(0);
}
#endif

//////////////////////////////////////////////////////////////////////////////////////////
// Update Flash OK, use Byte Write to finished, daway 2010-05-21

/* program data to SPI data */
int ec_update_rom(void *src, int size)
{
	u8 *buf;
	u32 addr = 0;
	u16 sector, page_cnt = 0; 
	u16 page, page1, i, j, sec_cnt = 0;
	u32 program_addr, erase_addr;
	u8 flags, status;
	u8 val;
	u8 data;
	
    //exit_type = WCB_EXIT_NORMAL;
    exit_type = WCB_EXIT_RESET_EC;
    //exit_type = WCB_EXIT_GOTO_BOOT_BLOCK;
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
	Init_Flash_Command();

	if(verbose) printf("Ready enter flash update...\n");
    // Indicate Flash Update beginning to Firmware
    Enter_Flash_Update();

#if 0
	/* unlock block protection active. */
	status = Flash_read_status_reg();
	if(status & 0x1C){
		Flash_write_status_reg(status & ~0x1C);
		printf("Block protection unlocked OK.\n");
	}else{
		printf("No block protection.\n");
	}
#endif

	/* Start sector erase active. */
	while(sector){
		if(verbose) printf("Starting erase sector %d ...\n", sec_cnt);
		flash_sector_erase(erase_addr);
		if(verbose) printf("Erase sector %d OK...\n", sec_cnt++);
		sec_cnt++;
		
		if((page * 256) > (4 * 1024)){
			page1 = (4 * 1024) / 256;
			flags = 1;
		}else{
			page1 = page;
			flags = 0;
		}
		
		Init_Flash_Command();
		/* start page program active */
		if(verbose) printf("The sum total has %d page to need to program:\n", page);
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
	
	/*Flash_write_status_reg(status | 0x1C);
	printf("All page programming completes for all sectors.\n");*/
out:
    // Indicate Flash Update termination to Firmware
    exit_flash_update(exit_type);
	printf("Update wpce775l done.\n");

	return 0;
}

/* get EC version from EC rom */
unsigned char *get_ecver(void)
{
	int i;
	unsigned char *p;
	static unsigned char val[OEMVER_MAX_SIZE];

	for(i = 0; i < OEMVER_MAX_SIZE && ec_read(CMD_RD_VER, i) != '\0'; i++){
		val[i] = ec_read(CMD_RD_VER, i);
	}
	p = val;
	if (*p == 0){
		p = "undefined";
	}
	return p;
}
#endif
