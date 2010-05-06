/*
 * kb3310.c 
 * handle all the thing about the EC kb3310
 * so far, the following functions are included :
 * 1, fixup some patch for YEELOONG platform
 * 2, update EC rom(including version)
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
#include "include/kb3310.h"

/************************************************************/
#if 0
/* ec delay time 500us for register and status access */
#define	EC_REG_DELAY	500	//unit : us
/* ec rom flash id size and array : Manufacture ID[1], Device ID[2] */
#ifndef NO_ROM_ID_NEEDED
#define	EC_ROM_ID_SIZE	3
unsigned char ec_rom_id[EC_ROM_ID_SIZE];
#endif
/* delay function */
extern void delay(int microseconds);
/* version array */
unsigned char ec_ver[VER_MAX_SIZE];

/* base address for io access */
#undef	MIPS_IO_BASE
#define	MIPS_IO_BASE	(0xbfd00000)

/* EC access port for sci communication */
#define	EC_CMD_PORT	0x66
#define	EC_STS_PORT	0x66
#define	CMD_INIT_IDLE_MODE	0xdd
#define	CMD_EXIT_IDLE_MODE	0xdf
#define	CMD_INIT_RESET_MODE	0xd8
#define	CMD_REBOOT_SYSTEM	0x8c

/* ec internal register */
#define	REG_POWER_MODE		0xF710
#define	FLAG_NORMAL_MODE	0x00
#define	FLAG_IDLE_MODE		0x01
#define	FLAG_RESET_MODE		0x02

/* read and write operation */
#undef	read_port
#undef	write_port
#define	read_port(x)	 (*(volatile unsigned char *)(MIPS_IO_BASE | x))
#define	write_port(x, y) (*(volatile unsigned char *)(MIPS_IO_BASE | x) = y)	

/* ec update program flag */
#define	PROGRAM_FLAG_NONE	0x00
#define	PROGRAM_FLAG_VERSION	0x01
#define	PROGRAM_FLAG_ROM	0x02

#endif

#define EC_ROM_PROTECTION
/* ec rom flash id size and array : Manufacture ID[1], Device ID[2] */
unsigned char ec_rom_id[EC_ROM_ID_SIZE];
/* version array */
unsigned char ec_ver[VER_MAX_SIZE];

/***************************************************************/

/*
 * ec_query_seq
 * this function is used for ec command writing and the corresponding status query 
 */
int ec_query_seq(unsigned char cmd)
{
	int timeout;
	unsigned char status;
	unsigned long flags;
	int ret = 0;

	/* make chip goto reset mode */
	udelay(EC_REG_DELAY);
	write_port(EC_CMD_PORT, cmd);
	udelay(EC_REG_DELAY);

	/* check if the command is received by ec */
	timeout = EC_CMD_TIMEOUT;
	status = read_port(EC_STS_PORT);
	while(timeout--){
		if(status & (1 << 1)){
			status = read_port(EC_STS_PORT);
			udelay(EC_REG_DELAY);
			continue;
		}
		break;
	}
	
	if(timeout <= 0){
		printf("EC QUERY SEQ : deadable error : timeout...\n");
		ret = -1;
	}

	return ret;
}

/************************************************************************/

/* enable the chip reset mode */
int ec_init_reset_mode(void)
{
	int timeout;
	unsigned char status = 0;
	int ret = 0;
	
	/* make chip goto reset mode */
	ret = ec_query_seq(CMD_INIT_RESET_MODE);
	if(ret < 0){
		printf("ec init reset mode failed.\n");
		goto out;
	}

	/* make the action take active */
	timeout = EC_CMD_TIMEOUT;
	status = rdec(REG_POWER_MODE) & FLAG_RESET_MODE;
	while(timeout--){
		if(status){
			udelay(EC_REG_DELAY);
			break;
		}
		status = rdec(REG_POWER_MODE) & FLAG_RESET_MODE;
		udelay(EC_REG_DELAY);
	}
	if(timeout <= 0){
		printf("ec rom fixup : can't check reset status.\n");
		ret = -1;
	}

	/* set MCU to reset mode */
	udelay(EC_REG_DELAY);
	status = rdec(REG_PXCFG);
	status |= (1 << 0);
	wrec(REG_PXCFG, status);
	udelay(EC_REG_DELAY);

	/* disable FWH/LPC */
	udelay(EC_REG_DELAY);
	status = rdec(REG_LPCCFG);
	status &= ~(1 << 7);
	wrec(REG_LPCCFG, status);
	udelay(EC_REG_DELAY);

	//printf("entering reset mode ok..............\n");

out :
	return ret;
}

/* make ec exit from reset mode */
void ec_exit_reset_mode(void)
{
	unsigned char regval;

	udelay(EC_REG_DELAY);
	regval = rdec(REG_LPCCFG);
	regval |= (1 << 7);
	wrec(REG_LPCCFG, regval);
	regval = rdec(REG_PXCFG);
	regval &= ~(1 << 0);
	wrec(REG_PXCFG, regval);
	//printf("exit reset mode ok..................\n");

	return;
}

/* make ec disable WDD */
void ec_disable_WDD(void)
{
	unsigned char status;

	udelay(EC_REG_DELAY);
	status = rdec(REG_WDTCFG);
	wrec(REG_WDTPF, 0x03);
	wrec(REG_WDTCFG, (status & 0x80) | 0x48);
	//printf("Disable WDD ok..................\n");

	return;
}

/* make ec enable WDD */
void ec_enable_WDD(void)
{
	unsigned char status;

	udelay(EC_REG_DELAY);
	status = rdec(REG_WDTCFG);
	wrec(REG_WDT, 0x28);		//set WDT 5sec(0x28)
	wrec(REG_WDTCFG, (status & 0x80) | 0x03);
	//printf("Enable WDD ok..................\n");

	return;
}

#if 1
/* re-power the whole system for new ec firmware working correctly. */
void ec_reboot_system(void)
{
	ec_query_seq(CMD_REBOOT_SYSTEM);
	//printf("reboot system...................\n");
}
#endif

/* make ec goto idle mode */
int ec_init_idle_mode(void)
{
	int timeout;
	unsigned char status = 0;
	int ret = 0;

	ec_query_seq(CMD_INIT_IDLE_MODE);

	/* make the action take active */
	timeout = EC_CMD_TIMEOUT;
	status = rdec(REG_POWER_MODE) & FLAG_IDLE_MODE;
	while(timeout--){
		if(status){
			udelay(EC_REG_DELAY);
			break;
		}
		status = rdec(REG_POWER_MODE) & FLAG_IDLE_MODE;
		udelay(EC_REG_DELAY);
	}
	if(timeout <= 0){
		printf("ec rom fixup : can't check idle status.\n");
		ret = -1;
	}

	//printf("entering idle mode ok...................\n");

	return ret;
}

/* make ec exit from idle mode */
int ec_exit_idle_mode(void)
{

	ec_query_seq(CMD_EXIT_IDLE_MODE);

	//printf("exit idle mode ok...................\n");
	
	return 0;
}

/**********************************************************************/

int ec_instruction_cycle(void)
{
	unsigned long timeout;
	int ret = 0;

	timeout = EC_FLASH_TIMEOUT;
	while(timeout-- >= 0){
		if( !(rdec(REG_XBISPICFG) & SPICFG_SPI_BUSY) )
				break;
	}
	if(timeout <= 0){
		printf("EC_INSTRUCTION_CYCLE : timeout for check flag.\n");
		ret = -1;
		goto out;
	}

out :
	return ret;
}

/* To see if the ec is in busy state or not. */
static inline int ec_flash_busy(unsigned long timeout)
{
	/* assurance the first command be going to rom */
	if( ec_instruction_cycle() < 0 ){
		return EC_STATE_BUSY;
	}

	timeout = timeout / EC_MAX_DELAY_UNIT;
	while(timeout-- > 0){
		/* check the rom's status of busy flag */
		wrec(REG_XBISPICMD, SPICMD_READ_STATUS);
		if( ec_instruction_cycle() < 0 ){
			return EC_STATE_BUSY;
		}	
		if((rdec(REG_XBISPIDAT) & 0x01) == 0x00){
			return EC_STATE_IDLE;
		}
		udelay(EC_MAX_DELAY_UNIT);
	}
	if( timeout <= 0 ){
		printf("EC_FLASH_BUSY : timeout for check rom flag.\n");
		return EC_STATE_BUSY;
	}

	return EC_STATE_IDLE;
}

int rom_instruction_cycle(unsigned char cmd)
{
	unsigned long timeout = 0;

	switch(cmd){
		case	SPICMD_READ_STATUS :
		case	SPICMD_WRITE_ENABLE :
		case	SPICMD_WRITE_DISABLE :
		case	SPICMD_READ_BYTE :
		case	SPICMD_HIGH_SPEED_READ :
				timeout = 0;
				break;
		case	SPICMD_WRITE_STATUS :
				timeout = 300 * 1000;
				break;
		case	SPICMD_BYTE_PROGRAM :
				timeout = 5 * 1000;
				break;
		case	SPICMD_SST_SEC_ERASE :
		case	SPICMD_SEC_ERASE :
				timeout = 1000 * 1000;
				break;
		case	SPICMD_SST_BLK_ERASE :
		case	SPICMD_BLK_ERASE :
				timeout = 3 * 1000 * 1000;
				break;
		case	SPICMD_SST_CHIP_ERASE :
		case	SPICMD_CHIP_ERASE :
				timeout = 20 * 1000 * 1000;
				break;
		default :
				timeout = EC_SPICMD_STANDARD_TIMEOUT;
	}
	if(timeout == 0){
		return ec_instruction_cycle();
	}
	if(timeout < EC_SPICMD_STANDARD_TIMEOUT)
			timeout = EC_SPICMD_STANDARD_TIMEOUT;

	return ec_flash_busy(timeout);
}

/* delay for start/stop action */
void delay_spi(int n)
{
	while(n--)
		read_port(HIGH_PORT);
}

/* start the action to spi rom function */
void ec_start_spi(void)
{
	unsigned char val;

	delay_spi(SPI_FINISH_WAIT_TIME);
	val = rdec(REG_XBISPICFG) | SPICFG_EN_SPICMD | SPICFG_AUTO_CHECK;
	wrec(REG_XBISPICFG, val);
	delay_spi(SPI_FINISH_WAIT_TIME);
}

/* stop the action to spi rom function */
void ec_stop_spi(void)
{
	unsigned char val;

	delay_spi(SPI_FINISH_WAIT_TIME);
	val = rdec(REG_XBISPICFG) & (~(SPICFG_EN_SPICMD | SPICFG_AUTO_CHECK));
	wrec(REG_XBISPICFG, val);
	delay_spi(SPI_FINISH_WAIT_TIME);
}

/* read one byte from xbi interface */
int ec_read_byte(unsigned int addr, unsigned char *byte)
{
	int ret = 0;

	/* enable spicmd writing. */
	ec_start_spi();

	/* enable write spi flash */
	wrec(REG_XBISPICMD, SPICMD_WRITE_ENABLE);
	if(rom_instruction_cycle(SPICMD_WRITE_ENABLE) == EC_STATE_BUSY){
			printf("EC_READ_BYTE : SPICMD_WRITE_ENABLE failed.\n");
			ret = -1;
			goto out;
	}
	
	/* write the address */
	wrec(REG_XBISPIA2, (addr & 0xff0000) >> 16);
	wrec(REG_XBISPIA1, (addr & 0x00ff00) >> 8);
	wrec(REG_XBISPIA0, (addr & 0x0000ff) >> 0);
	/* start action */
	wrec(REG_XBISPICMD, SPICMD_HIGH_SPEED_READ);
	if(rom_instruction_cycle(SPICMD_HIGH_SPEED_READ) == EC_STATE_BUSY){
			printf("EC_READ_BYTE : SPICMD_HIGH_SPEED_READ failed.\n");
			ret = -1;
			goto out;
	}
	
	*byte = rdec(REG_XBISPIDAT);

out :
	/* disable spicmd writing. */
	ec_stop_spi();

	return ret;
}

/* write one byte to ec rom */
int ec_write_byte(unsigned int addr, unsigned char byte)
{
	int ret = 0;

	/* enable spicmd writing. */
	ec_start_spi();

	/* enable write spi flash */
	wrec(REG_XBISPICMD, SPICMD_WRITE_ENABLE);
	if(rom_instruction_cycle(SPICMD_WRITE_ENABLE) == EC_STATE_BUSY){
			printf("EC_WRITE_BYTE : SPICMD_WRITE_ENABLE failed.\n");
			ret = -1;
			goto out;
	}

	/* write the address */
	wrec(REG_XBISPIA2, (addr & 0xff0000) >> 16);
	wrec(REG_XBISPIA1, (addr & 0x00ff00) >> 8);
	wrec(REG_XBISPIA0, (addr & 0x0000ff) >> 0);
	wrec(REG_XBISPIDAT, byte);
	/* start action */
	wrec(REG_XBISPICMD, SPICMD_BYTE_PROGRAM);
	if(rom_instruction_cycle(SPICMD_BYTE_PROGRAM) == EC_STATE_BUSY){
			printf("EC_WRITE_BYTE : SPICMD_BYTE_PROGRAM failed.\n");
			ret = -1;
			goto out;
	}
	
out :
	/* disable spicmd writing. */
	ec_stop_spi();

	return ret;
}

/* unprotect SPI ROM */
/* EC_ROM_unprotect function code */
int EC_ROM_unprotect(void)
{
	unsigned char status;

	/* enable write spi flash */
	wrec(REG_XBISPICMD, SPICMD_WRITE_ENABLE);
	if(rom_instruction_cycle(SPICMD_WRITE_ENABLE) == EC_STATE_BUSY){
		printf("EC_UNIT_ERASE : SPICMD_WRITE_ENABLE failed.\n");
		return 1;
	}

	/* unprotect the status register of rom */
	wrec(REG_XBISPICMD, SPICMD_READ_STATUS);
	if(rom_instruction_cycle(SPICMD_READ_STATUS) == EC_STATE_BUSY){
		printf("EC_UNIT_ERASE : SPICMD_READ_STATUS failed.\n");
		return 1;
	}
	status = rdec(REG_XBISPIDAT);
	wrec(REG_XBISPIDAT, status & 0x02);
	if(ec_instruction_cycle() < 0){
		printf("EC_UNIT_ERASE : write status value failed.\n");
		return 1;
	}

	wrec(REG_XBISPICMD, SPICMD_WRITE_STATUS);
	if(rom_instruction_cycle(SPICMD_WRITE_STATUS) == EC_STATE_BUSY){
		printf("EC_UNIT_ERASE : SPICMD_WRITE_STATUS failed.\n");
		return 1;
	}

	/* enable write spi flash */
	wrec(REG_XBISPICMD, SPICMD_WRITE_ENABLE);
	if(rom_instruction_cycle(SPICMD_WRITE_ENABLE) == EC_STATE_BUSY){
		printf("EC_UNIT_ERASE : SPICMD_WRITE_ENABLE failed.\n");
		return 1;
	}

	return 0;
}

/* Starts to execute the unprotect SPI ROM function. */
int EC_ROM_start_unprotect(void)
{
	unsigned char status;
	int ret = 0, i = 0;
	int unprotect_count = 3;
	int check_flag =0;

	/* added for re-check SPICMD_READ_STATUS */
	while(unprotect_count-- > 0){
		if(EC_ROM_unprotect()){
			ret = -1;
			return ret;
		}
		
		for(i = 0; i < ((2 - unprotect_count) * 100 + 10); i++)	//first time:500ms --> 5.5sec -->10.5sec
			udelay(50000);
		wrec(REG_XBISPICMD, SPICMD_READ_STATUS);
		if(rom_instruction_cycle(SPICMD_READ_STATUS) == EC_STATE_BUSY){
			printf("EC_PROGRAM_ROM : SPICMD_READ_STATUS failed.\n");
		} else {
			status = rdec(REG_XBISPIDAT);
			//printf("Read unprotect status : 0x%x\n", status);
			if((status & 0x1C) == 0x00){
				//printf("Read unprotect status OK1 : 0x%x\n", status & 0x1C);
				check_flag = 1;
				break;
			}
		}	
	}

	if(!check_flag){
		printf("SPI ROM unprotect fail.\n");
		return 1;
	}
	//printf("SPI ROM unprotect success.\n");

	return ret;
}

/* Starts to execute the protect SPI ROM function. */
int EC_ROM_start_protect(void)
{
	unsigned char status;
	int j;

	/* we should start spi access firstly */
	ec_start_spi();

	/* enable write spi flash */
	wrec(REG_XBISPICMD, SPICMD_WRITE_ENABLE);
	if(rom_instruction_cycle(SPICMD_WRITE_ENABLE) == EC_STATE_BUSY){
			printf("EC_PROGRAM_ROM : SPICMD_WRITE_ENABLE failed.\n");
			goto out1;
	}
	
	/* protect the status register of rom */
	wrec(REG_XBISPICMD, SPICMD_READ_STATUS);
	if(rom_instruction_cycle(SPICMD_READ_STATUS) == EC_STATE_BUSY){
			printf("EC_PROGRAM_ROM : SPICMD_READ_STATUS failed.\n");
			goto out1;
	}
	status = rdec(REG_XBISPIDAT);

	wrec(REG_XBISPIDAT, status | 0x1C);
	if(ec_instruction_cycle() < 0){
			printf("EC_PROGRAM_ROM : write status value failed.\n");
			goto out1;
	}

	wrec(REG_XBISPICMD, SPICMD_WRITE_STATUS);
	if(rom_instruction_cycle(SPICMD_WRITE_STATUS) == EC_STATE_BUSY){
			printf("EC_PROGRAM_ROM : SPICMD_WRITE_STATUS failed.\n");
			goto out1;
	}

	/* disable the write action to spi rom */
	wrec(REG_XBISPICMD, SPICMD_WRITE_DISABLE);
	if(rom_instruction_cycle(SPICMD_WRITE_DISABLE) == EC_STATE_BUSY){
			printf("EC_PROGRAM_ROM : SPICMD_WRITE_DISABLE failed.\n");
			goto out1;
	}
	
out1:
	/* we should stop spi access firstly */
	ec_stop_spi();
out:
	/* for security */
	//for(j = 0; j < 2000; j++)
	//	udelay(1000);

	/* exit from the reset mode */
	//ec_exit_reset_mode();

	return 0;
}

/* erase one block or chip or sector as needed */
int ec_unit_erase(unsigned char erase_cmd, unsigned int addr)
{
	unsigned char status;
	int ret = 0, i = 0;
	int unprotect_count = 3;
	int check_flag =0;

	/* enable spicmd writing. */
	ec_start_spi();

#ifdef EC_ROM_PROTECTION
	/* added for re-check SPICMD_READ_STATUS */
	while(unprotect_count-- > 0){
		if(EC_ROM_unprotect()){
			ret = -1;
			goto out;
		}
		
		for(i = 0; i < ((2 - unprotect_count) * 100 + 10); i++)	//first time:500ms --> 5.5sec -->10.5sec
			udelay(50000);
		wrec(REG_XBISPICMD, SPICMD_READ_STATUS);
		if(rom_instruction_cycle(SPICMD_READ_STATUS) == EC_STATE_BUSY){
			printf("EC_PROGRAM_ROM : SPICMD_READ_STATUS failed.\n");
		} else {
			status = rdec(REG_XBISPIDAT);
			//printf("Read unprotect status : 0x%x\n", status);
			if((status & 0x1C) == 0x00){
				//printf("Read unprotect status OK1 : 0x%x\n", status & 0x1C);
				check_flag = 1;
				break;
			}
		}	
	}

	if(!check_flag){
		printf("SPI ROM unprotect fail.\n");
		return 1;
	}
#endif

	/* block address fill */
	if(erase_cmd == SPICMD_BLK_ERASE){
		wrec(REG_XBISPIA2, (addr & 0x00ff0000) >> 16);
		wrec(REG_XBISPIA1, (addr & 0x0000ff00) >> 8);
		wrec(REG_XBISPIA0, (addr & 0x000000ff) >> 0);
	}

	/* erase the whole chip first */
	wrec(REG_XBISPICMD, erase_cmd);
	if(rom_instruction_cycle(erase_cmd) == EC_STATE_BUSY){
			printf("EC_UNIT_ERASE : erase failed.\n");
			ret = -1;
			goto out;
	}

out :
	/* disable spicmd writing. */
	ec_stop_spi();

	return ret;
}

#ifdef LOONGSON2F_7INCH
/* update the whole rom content with H/W mode
 * PLEASE USING ec_unit_erase() FIRSTLY
 */
int ec_update_rom(void *src, int size)
{
	unsigned char *buf;
	unsigned int addr = 0;
	unsigned char val;
	unsigned char data;
	int ret = 0, ret1; 
	int i, j;
	unsigned char status;

	buf = src;
	if(size > EC_ROM_MAX_SIZE){
        printf("ec-update : out of range.\n");
        return;
	}
	
	/* goto reset mode */
	ret = ec_init_reset_mode();

	if(ret < 0){
		return ret;
	}

    printf("starting update ec ROM..............\n");

	ret = ec_unit_erase(SPICMD_BLK_ERASE, addr);
	if(ret){
		printf("program ec : erase block failed.\n");
		goto out;
	}
	printf("program ec : erase block OK.\n");

	i = 0;
	while(i < size){
		data = *(buf + i);
		ec_write_byte(addr, data);
		ec_read_byte(addr, &val);
		if(val != data){
			ec_write_byte(addr, data);
			ec_read_byte(addr, &val);
			if(val != data){
				printf("EC : Second flash program failed at:\t");
				printf("addr : 0x%x, source : 0x%x, dest: 0x%x\n", addr, data, val);
				printf("This should not happened... STOP\n");
				break;				
			}
		}
		if( (i % 0x400) == 0x00 ){
			printf(".");
		}
		i++;
		addr++;
	}
	printf("\n");
	printf("Update EC ROM OK.\n");

#ifdef	EC_ROM_PROTECTION
	/* we should start spi access firstly */
	ec_start_spi();

	/* enable write spi flash */
	wrec(REG_XBISPICMD, SPICMD_WRITE_ENABLE);
	if(rom_instruction_cycle(SPICMD_WRITE_ENABLE) == EC_STATE_BUSY){
			printf("EC_PROGRAM_ROM : SPICMD_WRITE_ENABLE failed.\n");
			goto out1;
	}
	
	/* protect the status register of rom */
	wrec(REG_XBISPICMD, SPICMD_READ_STATUS);
	if(rom_instruction_cycle(SPICMD_READ_STATUS) == EC_STATE_BUSY){
			printf("EC_PROGRAM_ROM : SPICMD_READ_STATUS failed.\n");
			goto out1;
	}
	status = rdec(REG_XBISPIDAT);

	wrec(REG_XBISPIDAT, status | 0x1C);
	if(ec_instruction_cycle() < 0){
			printf("EC_PROGRAM_ROM : write status value failed.\n");
			goto out1;
	}

	wrec(REG_XBISPICMD, SPICMD_WRITE_STATUS);
	if(rom_instruction_cycle(SPICMD_WRITE_STATUS) == EC_STATE_BUSY){
			printf("EC_PROGRAM_ROM : SPICMD_WRITE_STATUS failed.\n");
			goto out1;
	}
#endif

	/* disable the write action to spi rom */
	wrec(REG_XBISPICMD, SPICMD_WRITE_DISABLE);
	if(rom_instruction_cycle(SPICMD_WRITE_DISABLE) == EC_STATE_BUSY){
			printf("EC_PROGRAM_ROM : SPICMD_WRITE_DISABLE failed.\n");
			goto out1;
	}
	
out1:
	/* we should stop spi access firstly */
	ec_stop_spi();
out:
	/* for security */
	for(j = 0; j < 2000; j++)
		udelay(1000);

	/* exit from the reset mode */
	ec_exit_reset_mode();

	/* reboot bios and EC rom for syncing */
	//ec_reboot_system();
	printf("Please reboot system................\n");

	return 0;
}

//#ifndef NO_ROM_ID_NEEDED
/* get flash rom product id number */
void ec_get_product_id(void)
{
	u8 regval;
	int i;
	
	/* get product id from ec rom */
	delay(EC_REG_DELAY);
	regval = rdec(XBI_BANK | XBISPICFG);
	regval |= 0x18;
	wrec(XBI_BANK | XBISPICFG, regval);
	delay(EC_REG_DELAY);

	wrec(XBI_BANK | XBISPICMD, 0x9f);
	while( (rdec(XBI_BANK | XBISPICFG)) & (1 << 1) );

	for(i = 0; i < EC_ROM_ID_SIZE; i++){
		wrec(XBI_BANK | XBISPICMD, 0x00);
		while( (rdec(XBI_BANK | XBISPICFG)) & (1 << 1) );
		ec_rom_id[i] = rdec(XBI_BANK | XBISPIDAT);
	}
	
	delay(EC_REG_DELAY);
	regval = rdec(XBI_BANK | XBISPICFG);
	regval &= 0xE7;
	wrec(XBI_BANK | XBISPICFG, regval);
	delay(EC_REG_DELAY);

	return;
}

/* get ec rom type */
int ec_get_rom_type(void)
{
	int ret = 0;

	/* make chip goto idle mode */
	ret = ec_init_idle_mode();
	ec_disable_WDD();
	if(ret < 0){
		ec_enable_WDD();
		return ret;
	}

	/* get product id from ec rom */
	ec_get_product_id();

	/* make chip exit idle mode */
	ret = ec_exit_idle_mode();
	ec_enable_WDD();
	if(ret < 0){
		return ret;
	}
	
	printf("ec rom id : PRODUCT ID : 0x%2x, FIRST DEVICE ID : 0x%2x, SECOND DEVICE ID : 0x%2x\n", ec_rom_id[0], ec_rom_id[1], ec_rom_id[2]);
	
	return ret;
}

/* ec fixup routine */
void ec_fixup(void)
{
    //ec_fan_fixup();
	//printf("ec fan fixup ok.\n");
#ifndef NO_ROM_ID_NEEDED
	ec_get_rom_type();
	printf("ec rom fixup ok.\n");
#endif
}

/* get EC version from EC rom */
unsigned char *get_ecver(void){
	int i;
	unsigned char *p;
	static unsigned char val[VER_MAX_SIZE] = {0};
	unsigned int addr = VER_ADDR;

	for(i = 0; i < VER_MAX_SIZE && rdec(addr) != '\0'; i++){
		val[i] = rdec(addr);
		addr++;
	}
	p = val;
	if (*p == 0){
		p = "undefined";
	}
	return p;
}
#endif
