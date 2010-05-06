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
#ifndef NO_ROM_ID_NEEDED
#define	EC_ROM_ID_SIZE	3
unsigned char ec_rom_id[EC_ROM_ID_SIZE];
#endif
/* version array */
unsigned char ec_ver[OEMVER_MAX_SIZE];

FLASH_device_commands_t FLASH_commands;

#if 0 //(1)
#define inp(a)      (*(volatile unsigned short*)(a))
#endif	// end if 0 (1)

#ifdef LOONGSON2F_3GNB
WCB_struct	     WCB;
unsigned int FLASH_id;

#ifndef SHM_ACCESS
void Update_Flash_Init(void)
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
	FLASH_commands.page_size = PAGE_SIZE;
	FLASH_commands.read_dev_id_type = READ_DEV_ID_TYPE;

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
	printf("Calling PcMemWriteB\n");
	wrwbec(WCB_BASE_ADDR, rdwbec(WCB_BASE_ADDR) & ~SHAW2_SEM_EXE);	// Clear EXE bit
	// Setup the WCB for Init
	WCB.Command	= FLASH_COMMANDS_INIT_OP;
	WCB.Param.InitCommands = FLASH_commands; 
	printf("Calling SHMRunCommand\n");
	Update_Run_Cmd(1 + sizeof(FLASH_device_commands_t));	          // Execute the Command in WCB
}
#else
void Update_Flash_Init(void)
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
	FLASH_commands.page_size = PAGE_SIZE;
	FLASH_commands.read_dev_id_type = READ_DEV_ID_TYPE;
}
#endif

int Write_multibyte(unsigned long Address, unsigned long NumOfBytes, const void *Data)
{
	int i;
	unsigned char const *ByteDataP = Data;

	for(i = 0; i < NumOfBytes; i++){
		wrwbec(Address + i, *ByteDataP);
		ByteDataP++;
	}
	return 0;
}

void Update_Run_Cmd(unsigned short cmd_length)
{
	unsigned char sem_val = 0;
	unsigned long timeout = 100000;
	/* Wait until firmware is ready to accept a command - i.e. EXE and RDY bits in semaphore are cleared. */
	do{
		sem_val = rdwbec(WCB_BASE_ADDR);
		//printf("Debug : <Update run cmd [0]> read sem_val 0x%x, timeout %d\n", sem_val, timeout);
	}while((sem_val & (SHAW2_SEM_EXE | SHAW2_SEM_RDY)) && (timeout--));
	
	if(!timeout){
		printf("Firmware is not ready to accept command (semaphore = 0x%x)\n", sem_val);
	}

	/* Write Command Buffer to memory */
	Write_multibyte(WCB_BASE_ADDR + OFFSET_COMMAND, cmd_length, &WCB);
	wrwbec(WCB_BASE_ADDR, rdwbec(WCB_BASE_ADDR) | SHAW2_SEM_EXE);	/* Enable EXE bit. */
	printf("Debug : <Update run cmd [0]> read sem_val 0x%x, timeout %d\n",  rdwbec(WCB_BASE_ADDR), timeout);

	if(WCB.Command == RESET_EC_OP){
		return;
	}

	/* Wait until done (indicated by RDY) */
	timeout = 100000;
	do{
		sem_val = rdwbec(WCB_BASE_ADDR);
		//printf("Debug : <Update run cmd [1]> read sem_val 0x%x, timeout %d\n", sem_val, timeout);
	}while((!(sem_val & SHAW2_SEM_RDY)) && (timeout--));

	if(!timeout){
		printf("Firmware did not set the RDY bit (semaphore = 0x%x)\n", sem_val);
	}

	wrwbec(WCB_BASE_ADDR, rdwbec(WCB_BASE_ADDR) & ~SHAW2_SEM_EXE);	/* Clear EXE bit. */

	if(sem_val & SHAW2_SEM_ERR){
		printf("ERROR: Firmware reported error (semaphore value %x). Aborting\n", sem_val);
		exit(-1);
	}

	/* Wait until Host clears RDY bit to acknowledge operation end. */
	timeout = 100000;
	do{
		sem_val = rdwbec(WCB_BASE_ADDR);
		//printf("Debug : <Update run cmd [2]> read sem_val 0x%x, timeout %d\n", sem_val, timeout);
	}while((sem_val & SHAW2_SEM_RDY) && (timeout--));

	if(!timeout){
		printf("Host did not clear the RDY bit (semaphore = 0x%x)\n", sem_val);
		exit(-1);
	}
}

unsigned short Read_ECROM_IDs(void)
{
	unsigned char val;
	unsigned short ids;

	printf("Update Flash Read IDs...\n");

	/* Setup the WCB for Read ID */
	WCB.Command = READ_IDS_OP;
	Update_Run_Cmd(WCB_SIZE_READ_IDS_CMD);
	val = rdwbec(WCB_BASE_ADDR + OFFSET_DATA);
	ids = (((unsigned short)rdwbec(WCB_BASE_ADDR + OFFSET_DATA + 1)) << 8) | val;
	//printf("IDs value : 0x%x\n", ids);

	return ids;
}

int ec_update_rom(void *src, int size)
{
	unsigned char *buf;
	int i;

	buf = src;
	printf("buf[0] = %x, size = %d\n", *buf, size);
	for(i = 0; i < size; i++){
		delay(1000);
		if( (i % 0x400) == 0x00 ){
			printf(".");
		}
	}
	printf("\n");
	printf("Update wpce775l done.\n");

	return 0;
}

/* get EC version from EC rom */
unsigned char *get_ecver(void){
	int i;
	unsigned char *p;
	static unsigned char val[OEMVER_MAX_SIZE];

	for(i = 0; i < OEMVER_MAX_SIZE && rdacpi(CMD_RD_VER, i) != '\0'; i++){
		val[i] = rdacpi(CMD_RD_VER, i);
	}
	p = val;
	if (*p == 0){
		p = "undefined";
	}
	return p;
}
#endif
