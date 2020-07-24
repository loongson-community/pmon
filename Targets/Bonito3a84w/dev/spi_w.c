#include <stdio.h>
//#include "include/fcr.h"
#include <stdlib.h>
#include <ctype.h>
#undef _KERNEL
#include <errno.h>
#include <pmon.h>
#include <types.h>
#include <pflash.h>

//#define SPI_BASE  0x1be70000 //for 3A2H 2H
//#define SPI_BASE  0x1fe001f0 //for 3A
#ifndef LOONGSON3A4000
#define SPI_BASE 0x1fe00220		/* 3A2000 */
#else
#define SPI_BASE 0x1fe001f0		/* 3A4000 */
#endif
#define PMON_ADDR 0xa1000000
#define FLASH_ADDR 0x000000

#define SPCR      0x0
#define SPSR      0x1
#define TXFIFO    0x2
#define RXFIFO    0x2
#define SPER      0x3
#define PARAM     0x4
#define SOFTCS    0x5
#define PARAM2    0x6

#define RFEMPTY 1
#define KSEG1_STORE8(addr,val)	(*(volatile unsigned char *)(0xa0000000 | addr) = val)
#define KSEG1_LOAD8(addr)	(*(volatile unsigned char *)(0xa0000000 | addr))

#define SET_SPI(addr,val)	KSEG1_STORE8((SPI_BASE + addr),val)
#define GET_SPI(addr)		KSEG1_LOAD8((SPI_BASE + addr))

#define SET_CE(x)		SET_SPI(SOFTCS,x)

int write_sr(char val);
void spi_initw(void)
{
	//ls3a4000 need change this code?
	SET_SPI(SPSR, 0xc0);
	SET_SPI(PARAM, 0x40);//espr:0100
	SET_SPI(SPER, 0x05);//spre:01
	SET_SPI(PARAM2,0x01);
	SET_SPI(SPCR, 0x50);
}

void spi_initr(void)
{
	SET_SPI(PARAM, 0x47);	     //espr:0100
}

int spi_wait_busy(void)
{
	int timeout = 1000;
	unsigned char res;

	do {
		res = read_sr();
	} while ((res & 0x01) && timeout--);

	if (timeout < 0) {
		printf("wait status register busy bit time out!\n");
		return -1;
	}
	return 0;
}

int send_spi_cmd(unsigned char command)
{
	int timeout = 1000;
	unsigned char val;

	SET_SPI(TXFIFO,command);
	while(((GET_SPI(SPSR)) & RFEMPTY) && timeout--);
	val = GET_SPI(RXFIFO);

	if (timeout < 0) {
		printf("wait rfempty timeout!\n");
		return -1;
	}
	return val;
}

///////////////////read status reg /////////////////
int read_sr(void)
{
	int val;

	SET_CE(0x01);
	send_spi_cmd(0x05);
	val = send_spi_cmd(0x00);
	SET_CE(0x11);

	return val;
}


////////////set write enable//////////
int set_wren(void)
{
	if (spi_wait_busy() < 0) {
		return -1;
	}

	SET_CE(0x01);
	send_spi_cmd(0x6);
	SET_CE(0x11);

	return 1;
}

////////////Enable-Write-Status-Register//////////
int en_write_sr(void)
{
	if (spi_wait_busy() < 0)
		return -1;

	SET_CE(0x01);
	send_spi_cmd(0x50);
	SET_CE(0x11);

	return 1;
}

///////////////////////write status reg///////////////////////
int write_sr(char val)
{
	/*this command do'nt used to check busy bit otherwise cause error*/
	en_write_sr();

	SET_CE(0x01);
	send_spi_cmd(0x01);
	/*set status register*/
	send_spi_cmd(val);
	SET_CE(0x11);

	return 1;
}

///////////erase all memory/////////////
int erase_all(void)
{
	int i=1;
	spi_initw();
	set_wren();
	if (spi_wait_busy() < 0)
		return -1;

	SET_CE(0x01);
	send_spi_cmd(0xc7);
	SET_CE(0x11);
	while(i++) {
		if(read_sr() & 0x1) {
			if(i % 10000 == 0)
				printf(".");
		} else {
			printf("done...\n");
			break;
		}
	}
	return 1;
}

void spi_read_id(void)
{
	unsigned char val;
	char i;

	spi_initw();
	if (spi_wait_busy() < 0)
		return;

	/*CE 0*/
	SET_CE(0x01);
	/*READ ID CMD*/
	send_spi_cmd(0x90);

	/*address bit [23-0]*/
	for (i = 0;i < 3;i++) {
		send_spi_cmd(0);
	}

	/*Manufacturer’s ID*/
	val = send_spi_cmd(0);
	printf("Manufacturer's ID:         %x\n",val);
	/*Device ID*/
	val = send_spi_cmd(0);
	printf("Device ID:                 %x\n",val);
	/*CE 1*/
	SET_CE(0x11);
}

void spi_jedec_id(void)
{
	unsigned char val;
	spi_initw();

	if (spi_wait_busy() < 0)
		return;
	/*CE 0*/
	SET_CE(0x01);
	/*JEDEC ID CMD*/
	send_spi_cmd(0x9f);

	/*Manufacturer’s ID*/
	val = send_spi_cmd(0x00);
	printf("Manufacturer's ID:         %x\n",val);

	/*Device ID:Memory Type*/
	val = send_spi_cmd(0x00);
 	printf("Device ID-memory_type:     %x\n",val);

	/*Device ID:Memory Capacity*/
	val = send_spi_cmd(0x00);
	printf("Device ID-memory_capacity: %x\n",val);

	/*CE 1*/
	SET_CE(0x11);
}

void spi_write_byte(unsigned int addr,unsigned char data)
{
	write_sr(0x0);
	set_wren();
	if (spi_wait_busy() < 0)
		return;

	SET_CE(0x01);/*CE 0*/

	send_spi_cmd(0x2);

	/*send addr [23 16]*/
	send_spi_cmd((addr >> 16) & 0xff);
	/*send addr [15 8]*/
	send_spi_cmd((addr >> 8) & 0xff);
	/*send addr [8 0]*/
	send_spi_cmd(addr & 0xff);

	/*send data(one byte)*/
	send_spi_cmd(data);

	/*CE 1*/
	SET_CE(0x11);
}

int write_pmon_byte(int argc,char ** argv)
{
	unsigned int addr;
	unsigned char val;
	if(argc != 3){
		printf("\nuse: write_pmon_byte  dst(flash addr) data\n");
		return -1;
	}
	addr = strtoul(argv[1],0,0);
	val = strtoul(argv[2],0,0);
	spi_write_byte(addr,val);
	return 0;
}

int write_pmon(int argc,char **argv)
{
	long int j=0;
	unsigned char val;
	unsigned int ramaddr,flashaddr,size;
	if(argc != 4){
		printf("\nuse: write_pmon src(ram addr) dst(flash addr) size\n");
		return -1;
	}

	ramaddr = strtoul(argv[1],0,0);
	flashaddr = strtoul(argv[2],0,0);
	size = strtoul(argv[3],0,0);

	spi_initw();
	write_sr(0);
	//read flash id command
	spi_read_id();
	val = GET_SPI(SPSR);
	printf("====spsr value:%x\n",val);

	SET_SPI(0x5,0x10);
	//erase the flash
	write_sr(0x00);
	//erase_all();
	printf("\nfrom ram 0x%08x  to flash 0x%08x size 0x%08x \n\nprogramming	    ",ramaddr,flashaddr,size);
	for(j=0;size > 0;flashaddr++,ramaddr++,size--,j++)
	{
		spi_write_byte(flashaddr,*((unsigned char*)ramaddr));
		if(j % 0x1000 == 0)
		printf("\b\b\b\b\b\b\b\b\b\b0x%08x",j);
	}
	printf("\b\b\b\b\b\b\b\b\b\b0x%08x end...\n",j);

	SET_CE(0x11);
	return 1;
}

int read_pmon_byte(unsigned int addr)
{
	unsigned char data;

	spi_wait_busy();
	SET_CE(0x01);
	// read flash command
	send_spi_cmd(0x03);
	/*send addr [23 16]*/
	send_spi_cmd((addr >> 16) & 0xff);
	/*send addr [15 8]*/
	send_spi_cmd((addr >> 8) & 0xff);
	/*send addr [8 0]*/
	send_spi_cmd(addr & 0xff);

	data = send_spi_cmd(0x00);
	SET_CE(0x11);
	return data;
}

int read_pmon(int argc,char **argv)
{
	unsigned char data;
	int base = 0;
	int addr;
	int i;
	if(argc != 3) {
		printf("\nuse: read_pmon addr(flash) size\n");
		return -1;
	}
	addr = strtoul(argv[1],0,0);
	i = strtoul(argv[2],0,0);
	spi_initw();

	if (spi_wait_busy() < 0) {
		return -1;
	}
	/*CE 0*/
	SET_CE(0x01);
	// read flash command
	send_spi_cmd(0x3);
	/*send addr [23 16]*/
	send_spi_cmd((addr >> 16) & 0xff);
	/*send addr [15 8]*/
	send_spi_cmd((addr >> 8) & 0xff);
	/*send addr [8 0]*/
	send_spi_cmd(addr & 0xff);

	printf("\n");
	while(i--) {
		data = send_spi_cmd(0x0);
		if(base % 16 == 0 ){
			printf("0x%08x    ",base);
		}
		printf("%02x ",data);
		if(base % 16 == 7)
			printf("  ");
		if(base % 16 == 15)
			printf("\n");
		base++;
	}
	printf("\n");
	return 1;
}

int spi_erase_area(unsigned int saddr,unsigned int eaddr,unsigned sectorsize)
{
	unsigned int addr;
	spi_initw();

	for(addr=saddr;addr<eaddr;addr+=sectorsize) {
		SET_CE(0x11);

		set_wren();
		write_sr(0x00);
		while(read_sr()&1);
		set_wren();
		SET_CE(0x01);
		/*
		 * 0x20 erase 4kbyte of memory array
		 * 0x52 erase 32kbyte of memory array
		 * 0xd8 erase 64kbyte of memory array
		 */
		send_spi_cmd(0x20);

		/*send addr [23 16]*/
		send_spi_cmd((addr >> 16) & 0xff);
		/*send addr [15 8]*/
		send_spi_cmd((addr >> 8) & 0xff);
		/*send addr [8 0]*/
		send_spi_cmd(addr & 0xff);
		SET_CE(0x11);

		while(read_sr()&1);
	}
	SET_CE(0x11);
	delay(10);

	return 0;
}

int spi_write_area(int flashaddr,char *buffer,int size)
{
	int j;
	spi_initw();
	SET_CE(0x10);
	write_sr(0x00);
	for(j=0;size > 0;flashaddr++,size--,j++) {
		spi_write_byte(flashaddr,buffer[j]);
		dotik(32, 0);
	}

	SET_CE(0x11);
	while(read_sr() & 1);
	SET_CE(0x11);
	delay(10);
	return 0;
}


int spi_read_area(int addr,char *buffer,int size)
{
	int i;
	spi_initw();

	SET_CE(0x01);

	send_spi_cmd(0x3);

	/*send addr [23 16]*/
	send_spi_cmd((addr >> 16) & 0xff);
	/*send addr [15 8]*/
	send_spi_cmd((addr >> 8) & 0xff);
	/*send addr [8 0]*/
	send_spi_cmd(addr & 0xff);

	for(i = 0;i < size; i++) {
		buffer[i] = send_spi_cmd(0x0);
	}

	SET_CE(0x11);
	delay(10);
	return 0;
}

struct fl_device myflash = {
	.fl_name="spiflash",
	.fl_size=0x100000,
	.fl_secsize=0x10000,
};

struct fl_device *spi_fl_devident(void *base, struct fl_map **m)
{
	if(m)
	*m = fl_find_map(base);
	return &myflash;
}

int spi_fl_program_device(void *fl_base, void *data_base, int data_size, int verbose)
{
	struct fl_map *map;
	int off;
	map = fl_find_map(fl_base);
	off = (int)(fl_base - map->fl_map_base) + map->fl_map_offset;
	spi_write_area(off,data_base,data_size);
	spi_initr();
	return 0;
}


int spi_fl_erase_device(void *fl_base, int size, int verbose)
{
	struct fl_map *map;
	int off;
	map = fl_find_map(fl_base);
	off = (int)(fl_base - map->fl_map_base) + map->fl_map_offset;
	spi_erase_area(off,off+size,0x1000);
	spi_initr();
	return 0;
}

#define readl(addr) *(volatile unsigned int*)(addr)
#define LS3A8_PONCFG 0Xbfe00100
#define SPI_SEL (1 << 16)
int selected_lpc_spi(void)
{
#ifndef LOONGSON3A4000
	return readl(LS3A8_PONCFG) & SPI_SEL;
#else
	return 1;
#endif
}

static const Cmd Cmds[] =
{
	{"MyCmds"},
	{"spi_initw","",0,"spi_initw(sst25vf080b)",spi_initw,0,99,CMD_REPEAT},
	{"read_pmon","",0,"read_pmon(sst25vf080b)",read_pmon,0,99,CMD_REPEAT},
	{"write_pmon","",0,"write_pmon(sst25vf080b)",write_pmon,0,99,CMD_REPEAT},
	{"erase_all","",0,"erase_all(sst25vf080b)",erase_all,0,99,CMD_REPEAT},
	{"write_pmon_byte","",0,"write_pmon_byte(sst25vf080b)",write_pmon_byte,0,99,CMD_REPEAT},
	{"read_flash_id","",0,"read_flash_id(sst25vf080b)",spi_read_id,0,99,CMD_REPEAT},
	{"spi_id","",0,"read_flash_id(sst25vf080b)",spi_jedec_id,0,99,CMD_REPEAT},
	{0,0}
};

static void init_cmd __P((void)) __attribute__ ((constructor));

static void
init_cmd()
{
	cmdlist_expand(Cmds,1);
}
