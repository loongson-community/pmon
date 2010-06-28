#include <sys/linux/types.h>
#include <sys/param.h>
#include <sys/device.h>
#include <sys/systm.h>
#include <sys/malloc.h>

#include <dev/pci/pcivar.h>
#include <dev/pci/pcireg.h>
#include <dev/pci/nppbreg.h>

#include <machine/bus.h>

#include <include/bonito.h>
#include <include/wpce775x.h>
#include <include/cs5536.h>
#include <include/cs5536_pci.h>
#include <pmon.h>
#include "cs5536_io.h"

/*********************************MSR debug*********************************/

static int cmd_rdmsr(int ac, char *av[])
{
	u32 msr, hi, lo;

	if(!get_rsa(&msr, av[1])) {
		printf("msr : access error!\n");
		return -1;
	}

	_rdmsr(msr, &hi, &lo);
	
	printf("msr : address  %x    hi  %x   lo  %x\n", msr, hi, lo);
	
	return 0;
}

static int cmd_wrmsr(int ac, char *av[])
{
	u32 msr, hi, lo;

	if(!get_rsa(&msr, av[1])){
		printf("msr : access error!\n");
		return -1;
	}else if(!get_rsa(&hi, av[2])){
		printf("hi : access error!\n");
		return -1;
	}else if(!get_rsa(&lo, av[3])){
		printf("lo : access error!\n");
		return -1;
	}

	_wrmsr(msr, hi, lo);
	
	return 0;
}

/*******************************Display debug*******************************/

#define	DISPLAY_MODE_NORMAL		0	// Screen(on);	Hsync(on);	Vsync(on)
#define	DISPLAY_MODE_STANDBY	1	// Screen(off);	Hsync(off);	Vsync(on)
#define	DISPLAY_MODE_SUSPEND	2	// Screen(off);	Hsync(on);	Vsync(off)
#define	DISPLAY_MODE_OFF		3	// Screen(off);	Hsync(off);	Vsync(off)
#define	SEQ_INDEX	0x3c4
#define	SEQ_DATA	0x3c5
static inline unsigned char read_seq(int reg)
{
	*((volatile unsigned char *)(0xbfd00000 | SEQ_INDEX)) = reg;
	return (*(volatile unsigned char *)(0xbfd00000 | SEQ_DATA));
}

static inline void write_seq(int reg, unsigned char data)
{
	*((volatile unsigned char *)(0xbfd00000 | SEQ_INDEX)) = reg;
	*((volatile unsigned char *)(0xbfd00000 | SEQ_DATA)) = data;

	return;
}

static u8 cur_mode;
static u8 SavedSR01, SavedSR20, SavedSR21, SavedSR22, SavedSR23, SavedSR24, SavedSR31, SavedSR34;
static int cmd_powerdebug(int ac, char *av[])
{
	u8 index;
	u8 SR01, SR20, SR21, SR22, SR23, SR24, SR31, SR34;

	if(ac < 2){
		printf("usage : powerdebug %index\n");
		return -1;
	}

	if(!get_rsa(&index, av[1])) {
		printf("powerdebug : access error!\n");
		return -1;
	}
	
	if(cur_mode == DISPLAY_MODE_NORMAL){
		SavedSR01 = read_seq(0x01);
		SavedSR20 = read_seq(0x20);
		SavedSR21 = read_seq(0x21);
		SavedSR22 = read_seq(0x22);
		SavedSR23 = read_seq(0x23);
		SavedSR24 = read_seq(0x24);
		SavedSR31 = read_seq(0x31);
		SavedSR34 = read_seq(0x34);
		printf("normal power register : \n");
		printf("SR01 0x%x   SR20 0x%x   SR21 0x%x   SR22 0x%x   SR23 0x%x\n", 
						SavedSR01, SavedSR20, SavedSR21, SavedSR22, SavedSR23, SavedSR24);
		printf("SR24 0x%x   SR31 0x%x   SR34 0x%x\n", SavedSR24, SavedSR31, SavedSR34);
	}
	SR01 = read_seq(0x01);
	SR20 = read_seq(0x20);
	SR21 = read_seq(0x21);
	SR22 = read_seq(0x22);
	SR23 = read_seq(0x23);
	SR24 = read_seq(0x24);
	SR31 = read_seq(0x31);
	SR34 = read_seq(0x34);

	printf("powerdebug : mode %d\n", index);
	switch(index){
		case	DISPLAY_MODE_NORMAL :
				SR01 &= ~0x20;
				SR20 = SavedSR20;
		        SR21 = SavedSR21;
				SR22 &= ~0x30; 
				SR23 &= ~0xC0;
				SR24 |= 0x01;
				SR31 = SavedSR31;
				SR34 = SavedSR34;
				break;
		case	DISPLAY_MODE_STANDBY :
				SR01 |= 0x20;
				SR20 = (SR20 & ~0xB0) | 0x10;
				SR21 |= 0x88;
				SR22 = (SR22 & ~0x30) | 0x10;
				SR23 = (SR23 & ~0x07) | 0xD8;
				SR24 &= ~0x01;
				SR31 = (SR31 & ~0x07) | 0x00;
				SR34 |= 0x80;
				break;
		case	DISPLAY_MODE_SUSPEND :
				SR01 |= 0x20;
				SR20 = (SR20 & ~0xB0) | 0x10;
				SR21 |= 0x88;
				SR22 = (SR22 & ~0x30) | 0x20;
				SR23 = (SR23 & ~0x07) | 0xD8;
				SR24 &= ~0x01;
				SR31 = (SR31 & ~0x07) | 0x00;
				SR34 |= 0x80;
				break;
		case	DISPLAY_MODE_OFF :
				SR01 |= 0x20;
			    SR20 = (SR20 & ~0xB0) | 0x10;
				SR21 |= 0x88;
				SR22 = (SR22 & ~0x30) | 0x30;
				SR23 = (SR23 & ~0x07) | 0xD8;
				SR24 &= ~0x01;
				SR31 = (SR31 & ~0x07) | 0x00;
				SR34 |= 0x80;
				break;
		default :
				printf("powerdebug : not supported mode setting for display.\n");
				break;
	}

	if( (*((volatile unsigned char *)(0xbfd00000 | 0x3cc))) & 0x01 ){
		while( (*((volatile unsigned char *)(0xbfd00000 | 0x3da))) & 0x08 );
		while( !((*((volatile unsigned char *)(0xbfd00000 | 0x3da))) & 0x08) );
	}else{
		while( (*((volatile unsigned char *)(0xbfd00000 | 0x3ba))) & 0x08 );
		while( !((*((volatile unsigned char *)(0xbfd00000 | 0x3ba))) & 0x08) );
	}

	write_seq(0x01, SR01);
	write_seq(0x20, SR20);
	write_seq(0x21, SR21);
	write_seq(0x22, SR22);
	write_seq(0x23, SR23);
	write_seq(0x24, SR24);
	write_seq(0x31, SR31);
	write_seq(0x34, SR34);

	cur_mode = index;

	return 0;
}

#ifdef LOONGSON2F_3GNB
/*******************************************************************************/

/* Write EC port */
static int cmd_wrport(int ac, char *av[])
{
	u8 port;
	u8 value = 0;

	if(ac < 3){
		printf("usage : wrport port value\n");
		return -1;
	}

	port = strtoul(av[1], NULL, 16);
	value = strtoul(av[2], NULL, 16);
	printf("Write EC port : port 0x%x, value 0x%x\n", port, value);
	/* Write value to EC port */
	write_port(port, value);

	return 0;
} 

/* Read EC port */
static int cmd_rdport(int ac, char *av[])
{
	u8 port;
	u8 val = 0;
	
	if(ac < 2){
		printf("usage : rdport port\n");
		return -1;
	}
	
	port = strtoul(av[1], NULL, 16);

	printf("Read EC port : port 0x%x", port);

	val = read_port(port);
	printf(" value 0x%x\n", val);

	return 0;
}

static int cmd_wr775(int ac, char *av[])
{
	u8 index;
	u8 cmd;
	u8 value = 0;

	if(ac < 3){
		printf("usage : wr775 cmd [index] value\n");
		printf("  e.g.: wr775 81 5A 1 (adjust brightness level to 1)\n");
		printf("        wr775 49 0/1 (close/open backlight)\n");
		return -1;
	}

	cmd = strtoul(av[1], NULL, 16);

	if(ac == 4){
		index = strtoul(av[2], NULL, 16);
		value = strtoul(av[3], NULL, 16);
		printf("Write EC 775 : command 0x%x, index 0x%x, data 0x%x \n", cmd, index, value);
		/* Write index value to EC 775 space */
		ec_write(cmd, index, value);
	}else{
		value = strtoul(av[2], NULL, 16);
		printf("Write EC 775 : command 0x%x, data 0x%x \n", cmd, value);
		/* Write index value to EC 775 space */
		ec_wr_noindex(cmd, value);
	}

	return 0;
} 

/*
 * rd775 : read a space of EC 775.
 * usage : rd775 cmd index size (hex)
 *  e.g. : rd775 4f [0] [21] (read EC version)
 * return: value (hex, dec, char)
 * daway added 2010-01-13
 */
static int cmd_rd775(int ac, char *av[])
{
	u8 i;
	u8 cmd, index;
	u8 size = 1, val = 0;
	
	if(ac < 2){
		printf("usage : rd775 cmd [index] [size]\n");
		return -1;
	}
	
	if(strcmp(av[1], "-h") == 0){
		printf("usage : rd775 {-h | cmd [index] [size]}\n");
		printf("  e.g.: rd775 -h      (this help)\n");
		printf("        rd775 80 5A 1 (read command 80h, index 5A, size 1 data)\n");
		printf("        rd775 46 1    (read command 46h, size 1 data)\n");
		printf("        rd775 46      (read command 46h, i.e., read device status)\n");
		return 0;
	}
	
	cmd = strtoul(av[1], NULL, 16);

	if(ac == 4){
		index = strtoul(av[2], NULL, 16);
		if(!get_rsa(&size, av[3])){
			printf("ERROR : arguments error!\n");
			return -1;
		}
	}

	if(ac == 3){
		if(!get_rsa(&size, av[2])){
			printf("ERROR : arguments error!\n");
			return -1;
		}
	}

	if(ac == 2){
		size = 1;
	}

	printf("Read EC 775 : command 0x%x, size %d\n", cmd, size);

	for(i = 0; i < size; i++){
		/* Read EC 775 space value */
		if(ac == 4){
			val = ec_read(cmd, index);
			printf("index 0x%x, value 0x%x %d '%c'\n", index, val, val, val);
			index++;
		}else{
			val = ec_rd_noindex(cmd + i);
			printf("value 0x%x %d '%c'\n", val, val, val);
		}
	}

	return 0;
}

static int cmd_wrsio(int ac, char *av[])
{
    u8 index, data;
 
    if(ac < 2){
        printf("usage : wrsio index data\n");
        return -1;
    }

	index = strtoul(av[1], NULL, 16);
	data = strtoul(av[2], NULL, 16);

	printf("Write EC775 SuperIO : index 0x%x, data 0x%x\n", index, data);

	wrsio(index, data);

	return 0;
}

static int cmd_rdsio(int ac, char *av[])
{
    u8 index;
    u8 val = 0;
 
    if(ac < 2){
        printf("usage : rdsio index\n");
        return -1;
    }

	index = strtoul(av[1], NULL, 16);

	printf("Read EC775 SuperIO : index 0x%x, ", index);

	val = rdsio(index);
	printf("data 0x%x\n", val);

	return 0;
}

static int cmd_wrldn(int ac, char *av[])
{
    u8 ld_num, index, data;
 
    if(ac < 3){
        printf("usage : wrldn logical_device_num index data\n");
        return -1;
    }

	ld_num = strtoul(av[1], NULL, 16);
	index = strtoul(av[2], NULL, 16);
	data = strtoul(av[3], NULL, 16);

	printf("Write EC775 LDN : logical device number 0x%x, index 0x%x, data 0x%x\n", ld_num, index, data);

	wrldn(ld_num, index, data);

	return 0;
}

static int cmd_rdldn(int ac, char *av[])
{
    u8 ld_num, index;
    u8 val = 0;
 
    if(ac < 2){
        printf("usage : rdldn logical_device_num index\n");
        return -1;
    }

	ld_num = strtoul(av[1], NULL, 16);
	index = strtoul(av[2], NULL, 16);

	printf("Read EC775 LDN : logical device number 0x%x, index 0x%x, ", ld_num, index);

	val = rdldn(ld_num, index);
	printf("data 0x%x\n", val);

	return 0;
}

/*
 * rdbat : read a register of battery.
 */
static int cmd_rdbat(int ac, char *av[])
{
	u8 bat_present;
	u8 bat_capacity;
	u16 bat_voltage;
	u16 bat_current;
	u16 bat_temperature;
	char current_sign;
	
	/* read battery status */
	bat_present = ec_read(CMD_READ_EC, INDEX_POWER_STATUS) & BIT_POWER_BATPRES;

	/* read battery voltage */
	bat_voltage = (u16) ec_read(CMD_READ_EC, INDEX_BATTERY_VOL_LOW);
	bat_voltage |= (((u16) ec_read(CMD_READ_EC, INDEX_BATTERY_VOL_HIGH)) << 8);
	bat_voltage = bat_voltage * 2;

	/* read battery current */
	bat_current = (u16) ec_read(CMD_READ_EC, INDEX_BATTERY_AI_LOW);
	bat_current |= (((u16) ec_read(CMD_READ_EC, INDEX_BATTERY_AI_HIGH)) << 8);
	bat_current = bat_current * 357 / 2000;
	if( (ec_read(CMD_READ_EC, INDEX_BATTERY_FLAG) & BIT_BATTERY_CURRENT_PN) != 0 ){
		current_sign = '-';
	}else{
		current_sign = ' ';
	}

	/* read battery capacity % */
	bat_capacity = ec_read(CMD_READ_EC, INDEX_BATTERY_CAPACITY);

	/* read battery temperature */
	bat_temperature = (u16) ec_read(CMD_READ_EC, INDEX_BATTERY_TEMP_LOW);
	bat_temperature |= (((u16) ec_read(CMD_READ_EC, INDEX_BATTERY_TEMP_HIGH)) << 8);
	if(bat_present){	// && bat_temperature <= 0x5D4){	/* temp <= 100 (0x5D4 = 1492(d)) */
		bat_temperature = bat_temperature / 4 - 273;
	}else{
		bat_temperature = 0;
	}

    printf("Read battery current information:\n");
	printf("Voltage %dmV, Current %c%dmA, Capacity %d%%, Temperature %d\n",
			bat_voltage, current_sign, bat_current, bat_capacity, bat_temperature);

	return 0;
}

/* 
 * rdfan : read a register of temperature IC. 
 */
static int cmd_rdfan(int ac, char *av[])
{
    u16 speed = 0;
	u8 temperature;
	char sign = ' ';

	speed = (u16)ec_read(CMD_READ_EC, INDEX_FAN_SPEED_LOW);
    speed |= (((u16)ec_read(CMD_READ_EC, INDEX_FAN_SPEED_HIGH)) << 8); 

	temperature = ec_read(CMD_READ_EC, INDEX_TEMPERATURE_VALUE);
	if(temperature & BIT_TEMPERATURE_PN){
		temperature = (temperature & TEMPERATURE_VALUE_MASK) - 128;
		sign = '-';
	}

    printf("Fan speed: %dRPM, CPU temperature: %c%d.\n", speed, sign, temperature);

	return 0;
}

static int cmd_test_sci(int ac, char *av[])
{
    u32 i = 0;
	u8 sci_num;
 
	while(1){
		delay(100000);
		while( (read_port(EC_STS_PORT) & (EC_SCI_EVT | EC_OBF)) == 0 );
		if(!ec_query_seq(CMD_QUERY)){
			sci_num = recv_ec_data();
		}
		printf("sci number (%d): 0x%x\n", ++i, sci_num);
	}

	return 0;
}

#if 1	// test update flash function, daway 2010-05-10
#include "include/flupdate.h"
extern void set_interface_cfg(u8 flag);
extern unsigned short Read_Flash_IDs(void);
extern void Update_Flash_Init(void);
extern void Init_Flash_Command(void);
extern int ec_update_rom_test(u32 Address, void *src, int size);
extern unsigned char PcMemReadB (unsigned long Address);
extern unsigned char Flash_Data[MAX_FLASH_CAPACITY];
extern void Enter_Flash_Update(void);
extern void flash_sector_erase(unsigned long Address);
extern void exit_flash_update(WCB_exit_t exit_type);
extern void Flash_Set_Address(unsigned long Address);
extern void Flash_program_byte(u8 data);
extern unsigned char read_status_register(void);
extern void write_status_register(u8 data);
extern void Flash_write_byte(u8 data);
extern WCB_exit_t exit_type;                           // Type of protocol termination to use
extern void Flash_Program(unsigned char * Data, unsigned int Size);

static int cmd_rdstsreg(int ac, char *av[])
{
    u8 val = 0;
 
    set_interface_cfg(0);
	Init_Flash_Command();

	val = read_status_register();

	printf("Read EC FLASH status register: 0x%x\n", val);

	return 0;
}

static int cmd_wrstsreg(int ac, char *av[])
{
    u8 data;

	if(ac < 2){
        printf("usage : wrstsreg status_value\n");
        return -1;
    }
 
	data = strtoul(av[1], NULL, 16);

	printf("Write EC FLASH status register: data 0x%x\n", data);
    set_interface_cfg(0);
	Init_Flash_Command();

	write_status_register(data);

	return 0;
}

static int cmd_rdids(int ac, char *av[])
{
    u16 ec_ids = 0;
	u8 mf_id, dev_id;

    set_interface_cfg(0);
	Init_Flash_Command();
	ec_ids = Read_Flash_IDs();
	printf("Read wpce775l flash ID: 0x%x\n", ec_ids);

	mf_id = ec_ids & 0x00FF;
	dev_id = (u8) ((ec_ids & 0xFF00) >> 8);
	printf("Manufacture ID 0x%x, Device ID : 0x%x\n", mf_id, dev_id);

    printf("EC ROM manufacturer: ");
    switch(mf_id){
        case EC_ROM_PRODUCT_ID_SPANSION :
            printf("SPANSION.\n");
            break;
        case EC_ROM_PRODUCT_ID_MXIC :
            printf("MXIC.\n");
            break;
        case EC_ROM_PRODUCT_ID_AMIC :
            printf("AMIC.\n");
            break;
        case EC_ROM_PRODUCT_ID_EONIC :
            printf("EONIC.\n");
            break;
        case EC_ROM_PRODUCT_ID_WINBOND :
            printf("WINBOND W25x80.\n");
            break;
 
		default :
            printf("Unknown chip type.\n");
            break;
    }

	return 0;
}
         
static int cmd_rdshmreg(int ac, char *av[])
{
	u8 data;
	u16 i;

	for(i = 0; i < 16; i++)
	{
		data = rdldn(0xf, SHM_CFG + i);
		printf("Read SHM configuration: reg 0x%x, data 0x%x\n", SHM_CFG + i, data);
	}

	return 0;
}
       
static int cmd_setshm_ram_flash(int ac, char *av[])
{
	u8 shm_ram_flg;	// 0 = shmram, 1 = shmflash
 
    if(ac < 2){
        printf("usage : setshmram flag\n");
        printf(" note : flag = 1 as shared flash access\n");
        printf("        flag = 0 as shared ram access\n");
        return -1;
    }

	shm_ram_flg = strtoul(av[1], NULL, 16);

	if(shm_ram_flg)
    	printf("set SHM interface is shared FLASH access: 0x%x\n", shm_ram_flg);
	else
    	printf("set SHM interface is shared RAM access: 0x%x\n", shm_ram_flg);

    set_interface_cfg(shm_ram_flg);

    printf("set SHM interface success.\n");

	return 0;
}

static int cmd_initcmd(int ac, char *av[])
{
	u16 i;

    set_interface_cfg(0);

	printf("Before Init: ");
	for(i = 0; i < 16; i++)
	{
		printf("0x%x ", rdwcb(WCB_BASE_ADDR + i));
	}
	printf("\n");

	Init_Flash_Command();
	
	printf("After Init: ");
	for(i = 0; i < 16; i++)
	{
		printf("0x%x ", rdwcb(WCB_BASE_ADDR + i));
	}
	printf("\n");

	return 0;
}

static int cmd_wrwcb(int ac, char *av[])
{
    u32 wcb_addr;
	u8 data;
 
    if(ac < 2){
        printf("usage : wrwcb address data\n");
        return -1;
    }

	wcb_addr = strtoul(av[1], NULL, 16);
	data = strtoul(av[2], NULL, 16);

	printf("Write EC775 WCB : address 0x%x, data 0x%x\n", wcb_addr, data);

	wrwcb(wcb_addr, data);

	return 0;
}

static int cmd_rdwcb(int ac, char *av[])
{
    u32 wcb_addr;
	u8 size = 1;
    u8 val = 0;
	int i;
 
    if(ac < 2){
        printf("usage : rdwcb address [size]\n");
        return -1;
    }

	wcb_addr = strtoul(av[1], NULL, 16);

	if(ac == 3){
		size = strtoul(av[2], NULL, 16);
	}else{
		size = 1;
	}

	if(size > 16){
		printf("ERROR : out of size range.\n");
		return -1;
	}

	printf("Read EC775 WCB : start address 0x%x, size %d\n", wcb_addr, size);

	for(i = 0; i < size; i++){
		val = rdwcb(wcb_addr + i);
		printf("WCB address 0x%x, data 0x%x\n", wcb_addr + i, val);
	}

	return 0;
}

static int cmd_sector_erase(int ac, char *av[])
{
    u32 address;

	if(ac < 2){
        printf("usage : secerase address\n");
        return -1;
    }
 
	address = strtoul(av[1], NULL, 16);

	printf("Erase sector: start address 0x%x\n", address);
    set_interface_cfg(0);
	Init_Flash_Command();

    Enter_Flash_Update();
	flash_sector_erase(address);
	printf("Erase sector OK...\n");
    exit_flash_update(EXIT_OP);

	return 0;
}

/* program data to SPI data */
void Flash_program_data(u32 addr, u8 *src, u32 size)
{
	u8 *buf;
	u16 sector, page_cnt = 0; 
	u16 page, page1, i, j, sec_cnt = 0;
	u32 program_addr, erase_addr;
	u8 flags, remainder, status;
	
    exit_type = WCB_EXIT_NORMAL;
	sector = 1;
	page = 1;
	erase_addr = program_addr = addr;
	buf = src;
	
	printf("Progarm data to address :  0x%lx\n", addr);
	printf("Progarm data size is %d\n", size);
	
	remainder = size % 256;
	
	/* start sector erase active */
	if(size > 0x1000){
		sector = size / 0x1000 + 1;
	}
	if(size > 256){
		page = size / 256 + 1;
	}
	printf("The sum total has %d sectors to need to erase:\n", sector);
	
	Update_Flash_Init();
    set_interface_cfg(0);
	Init_Flash_Command();

	printf("Ready enter flash update...\n");
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
		printf("Starting erase sector %d ...\n", sec_cnt);
		flash_sector_erase(erase_addr);
		printf("Erase sector %d OK...\n", sec_cnt++);
		
		if((page * 256) > (4 * 1024)){
			page1 = (4 * 1024) / 256;
			flags = 1;
		}else{
			page1 = page;
			flags = 0;
		}
		
	Init_Flash_Command();
		/* start page program active */
		printf("The sum total has %d page to need to program:\n", page);
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
			printf("\npage %d program ok.\n", page_cnt++);
			page1--;
		}
		page -= (4 * 1024) / 256;
		printf("All page programming completes for sector %d.\n", sec_cnt - 1);
		
		erase_addr += 0x1000;
		sector--;
	}
	
	/*Flash_write_status_reg(status | 0x1C);
	printf("All page programming completes for all sectors.\n");*/
	
    // Indicate Flash Update termination to Firmware
    exit_flash_update(exit_type);
}
        
static int cmd_rdrom(int ac, char *av[])
{
    u16 i, val;
	u32 start_addr;
	u32 size;

    if(ac < 2){
        printf("usage : rdrom start_address size\n");
        return -1;
    }

	if(!(strcmp(av[1], "-h")) || (!strcmp(av[1], "--help"))){
        printf("Read data from EC flash tool for wpce775l.\n");
        printf("usage : rdrom start_address size\n");
        printf(" Note : start_address is from 0 to (1MB - 1) range.\n");
        printf("        size is up to (1MB - start_address).\n");
		return 0;
	}


	start_addr = (strtoul(av[1], NULL, 16) | FLASH_WIN_BASE_ADDR);
	size = strtoul(av[2], NULL, 16);

	if(size > (1024 * 1024 - start_addr)){
		printf("Warnning: size out of range, reset size is 1.\n");
		size = 1;
	}

	printf("Read data in EC ROM from address 0x%x, size %d:\n", start_addr, size);
	printf("Address ");
	for(i = 0; i < 15; i++){
		if(((i % 8 ) == 0) && (i != 0)){
			printf(" -");
		}
		printf(" 0%x", i);
	}
	printf(" 0%x\n%5x", i, start_addr);
    set_interface_cfg(1);
	for(i = 0; i < size; i++){
		val = PcMemReadB(start_addr + i);
		if(((i % 8 ) == 0) && ((i % 16) != 0)){
			printf(" -");
		}
		if((val >> 4) == 0){
			printf(" 0%x", val);
		}else{
			printf(" %x", val);
		}
		if((i + 1) % 16 == 0){
			printf("\n");
			printf("%5x", (start_addr + i + 1));
		}
	}
	printf("\n");
    set_interface_cfg(0);

	return 0;
}

/* test program data to SPI data */
void Test_program_multbyte(u32 address, u32 size)
{
	u16 sector, page, i = 0, j = 0;
	u32 program_addr, erase_addr;
	u16 remainder;

    exit_type = WCB_EXIT_NORMAL;
	sector = 1;
	page = 1;
	erase_addr = program_addr = address;

	printf("Progarm data to address: 0x%x, size %d\n", address, size);
	
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

	printf("Ready enter flash update...\n");
    // Indicate Flash Update beginning to Firmware
    Enter_Flash_Update();

	/* start sector erase active */
	printf("The sum total has %d sectors to need to erase:\n", sector);
	while(sector){
		printf("Starting erase sector %d ...\n", j);
		flash_sector_erase(erase_addr);
		printf("Erase sector %d OK...\n", j++);

	Init_Flash_Command();
		/* start page program active */
		printf("The sum total has %d page to need to program:\n", page);
		while(page){
			if(size > 0xFF){
				remainder = 256;
			}else{
				remainder = size;
			}
			for(i = 0; i < remainder; i++){
				Flash_Set_Address(program_addr);
				Flash_write_byte(i);
				program_addr++;
				size--;
			}
			printf(".");
			page--;
		}
		printf("\nAll page programming completes.\n");
		erase_addr += 0x1000;
		sector--;
	}
    // Indicate Flash Update termination to Firmware
    exit_flash_update(exit_type);
}

static int cmd_program_multbytes(int ac, char *av[])
{
	u32 addr, size;

    if(ac < 2){
        printf("usage : wrmb address size\n");
        return -1;
    }

	addr = strtoul(av[1], NULL, 16);
	size = strtoul(av[2], NULL, 16);

	printf("Start multbyte program: address 0x%x, size %d\n", addr, size);

    Test_program_multbyte(addr, size);

	return 0;
}

static int cmd_program_data(int ac, char *av[])
{
	u32 start_addr;
	u8 *data;
	u32 size;

    if(ac < 2){
        printf("usage : wrdata address data\n");
        return -1;
    }

	start_addr = strtoul(av[1], NULL, 16);
	data = av[2];
	size = strlen(data);

	printf("Progarm data: start address 0x%x, data %s, size %d\n", start_addr, data, size);

	Flash_program_data(start_addr, data, size);

	printf("Program data completed.\n");

	return 0;
}
#endif	// end if 1, test update flash function, daway 2010-05-10

#endif	//end ifdef LOONGSON2F_3GNB

int cmd_testvideo(int ac, char *av[])
{
	unsigned long addr;
	if(ac < 2)
			return -1;
	addr = strtoul(av[1], NULL, 16);
	printf("addr = 0x%x", addr);
	video_display_bitmap(addr, 280, 370);
	return 1;
}

static const Cmd Cmds[] =
{
	{"cs5536 debug"},
	{"powerdebug", "reg", NULL, "for debug the power state of sm712 graphic", cmd_powerdebug, 2, 99, CMD_REPEAT},
	{"rdmsr", "reg", NULL, "msr read test", cmd_rdmsr, 2, 99, CMD_REPEAT},
	{"wrmsr", "reg", NULL, "msr write test", cmd_wrmsr, 2, 99, CMD_REPEAT},
#ifdef LOONGSON2F_3GNB	
	{"wrport", "reg", NULL, "WPCE775L write port test", cmd_wrport, 2, 99, CMD_REPEAT},
	{"rdport", "reg", NULL, "WPCE775L read port test", cmd_rdport, 2, 99, CMD_REPEAT},
	{"wrsio", "reg", NULL, "WPCE775L Super IO port write test", cmd_wrsio, 2, 99, CMD_REPEAT},
	{"rdsio", "reg", NULL, "WPCE775L Super IO port read test", cmd_rdsio, 2, 99, CMD_REPEAT},
	{"wrwcb", "reg", NULL, "WPCE775L WCB(Write Command Buffer) write test", cmd_wrwcb, 2, 99, CMD_REPEAT},
	{"rdwcb", "reg", NULL, "WPCE775L WCB(Write Command Buffer) read test", cmd_rdwcb, 2, 99, CMD_REPEAT},
	{"wrldn", "reg", NULL, "WPCE775L logical device write test", cmd_wrldn, 2, 99, CMD_REPEAT},
	{"rdldn", "reg", NULL, "WPCE775L logical device read test", cmd_rdldn, 2, 99, CMD_REPEAT},

	{"updec", "reg", NULL, "WPCE775L Flash program test", cmd_program_data, 2, 99, CMD_REPEAT},
	{"rdrom", "reg", NULL, "WPCE775L read flash test", cmd_rdrom, 2, 99, CMD_REPEAT},
	{"wrmb", "reg", NULL, "WPCE775L Flash program test", cmd_program_multbytes, 2, 99, CMD_REPEAT},
	{"secerase", "reg", NULL, "WPCE775L Flash sector erase test", cmd_sector_erase, 2, 99, CMD_REPEAT},
	{"setshmmode", "reg", NULL, "WPCE775L read flash test", cmd_setshm_ram_flash, 2, 99, CMD_REPEAT},
	{"rdshmreg", "reg", NULL, "WPCE775L EC ID read test", cmd_rdshmreg, 0, 99, CMD_REPEAT},
	{"initcmd", "reg", NULL, "WPCE775L EC ID read test", cmd_initcmd, 0, 99, CMD_REPEAT},
	{"rdstsreg", "reg", NULL, "WPCE775L EC ID read test", cmd_rdstsreg, 0, 99, CMD_REPEAT},
	{"wrstsreg", "reg", NULL, "WPCE775L read flash test", cmd_wrstsreg, 2, 99, CMD_REPEAT},

	{"tsci", "reg", NULL, "WPCE775L EC ID read test", cmd_test_sci, 0, 99, CMD_REPEAT},
	{"wr775", "reg", NULL, "WPCE775L Space write test", cmd_wr775, 2, 99, CMD_REPEAT},
	{"rd775", "reg", NULL, "WPCE775L Space read test", cmd_rd775, 2, 99, CMD_REPEAT},
	{"rdids", "reg", NULL, "WPCE775L EC ID read test", cmd_rdids, 0, 99, CMD_REPEAT},
	{"rdbat", "reg", NULL, "WPCE775L battery reg read test", cmd_rdbat, 0, 99, CMD_REPEAT},
	{"rdfan", "reg", NULL, "WPCE775L CPU temperature and fan reg read test", cmd_rdfan, 0, 99, CMD_REPEAT},
#endif

	{"testvideo", "reg", NULL, "for debug read data from xbi interface of ec", cmd_testvideo, 2, 99, CMD_REPEAT},
	{0},
};


static void init_cmd __P((void)) __attribute__ ((constructor));

static void
init_cmd()
{
	cmdlist_expand(Cmds, 1);
}

