/*	$Id: flash_spi.c,v 1.1.1.1 2006/09/14 01:59:08 root Exp $ */

/*
 * Copyright (c) 2001 ipUnplugged AB   (www.ipunplugged.com)
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by
 *	ipUnplugged AB, Sweden.
 * 4. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <machine/pio.h>

#include <pmon.h>

#include <pflash.h>
#include <dev/pflash_tgt.h>

//static unsigned char loongson_spi_buf[512 * 1020]
static unsigned char loongson_spi_buf[512 * 2048]
__attribute__ ((section ("text"), aligned (512)));
static unsigned char tbuf[256]
__attribute__ ((section ("text")));



#define LOONGSON_SPI_PAGE_SIZE	256			/* why here it is 256 */
#define ECC_BACHIPERASE			0xfD0000	/* SAVE ECC for every page(256B). 3Bytes per page */
#ifdef LOONGSON_3B1500
#define LOONGSON_SPI_SIZE		0x100000	/* 8b */
#else
#define LOONGSON_SPI_SIZE		0x1000000	/* 32M b */
#endif
#define PMON_SIZE				0x80000

#define FLASH_VERIFY

#define GPIO_0			(0x01<<0)
#define GPIO_1			(0x01<<1)
#define GPIO_2			(0x01<<2)
#define GPIO_3			(0x01<<3)
#define GPIO_12			(0x01<<12)
#define GPIO_DATA_REG	0xbfe0011c
#define GPIO_CTRL_REG	0xbfe00120

#define GPIO_CS_BIT		GPIO_12

#define GPIO_CPU_CS_ENABLE (0x01<<15)
//#define DEBUG
#ifdef DEBUG
#define DEBUG_PRINTF(fmt, args...)  pritnf(##fmt, ##args)
#else
#define DEBUG_PRINTF(...) do{}while(0)
#endif

#define SPE_SHIFT		6
#define MSTR_SHIFT		4
#define CPOL_SHIFT		3
#define CPHA_SHIFT		2
#define SPR_SHIFT		0
#define SPE				(1<<SPE_SHIFT)
#define MSTR			(1<<MSTR_SHIFT)
#define FCR_SPCR		0x00
#define FCR_SPSR		0x01
#define FCR_SPDR		0x02
#define FCR_SPER		0x03
#define FCR_TIME		0x06
#define FCR_PARAM		0x04

#define WREN			0x06
#define WRDI			0x04
#define RDID			0x90
#define RDSR			0x05
#define WRSR			0x01
#define READ			0x03
#define AAI				0x02
#define EBSY			0x70
#define DBSY			0x80

#define EWSR			0x50
#define FAST_READ		0x0B
#define BYTE_WRITE		0x02  /* Byte Program Enable */
#define AAI_WRITE		0xad  /* Byte Program Enable */

//#ifdef LS3B_SPI_BOOT
#define BE4K			0x20  /* 4K Byte block Rrase, Sector Erase */
#define BE4KSIZE		0x1000  /* 4K Byte block Rrase, Sector Erase */
#define BE32K			0x52  /* 32K Byte block Rrase, Sector Erase */
#define BE32KSIZE		0x8000/* 32K Byte block Rrase, Sector Erase */
#define BE64K			0xD8  /* 64K Byte block Rrase, Sector Erase */
#define BE64KSIZE		0x10000/* 64K Byte block Rrase, Sector Erase */
#define CHIPERASE		0xC7  /* Chip Erase */

//#define BLKERASE		BE32K
//#define BLKSIZE			BE32KSIZE
//#define BLKERASE		BE64K
//#define BLKSIZE			BE64KSIZE

#define BLKERASE		BE4K
#define BLKSIZE			BE4KSIZE


//#else
//#define CHIPERASE		0xC7  /* Block Rrase, Sector Erase */
//#endif

#define MACID_ADDR     0x00
#define DEVID_ADDR     0x01
#define VOID_ADDR      0x00
#define VOID_CMD       0x00

#ifdef LS3B_SPI_BOOT
#define SPI_REG_BASE	0xbfe00220		/* 3B */
#else
#define SPI_REG_BASE	0xbfe001f0		/* 3A */
#endif

#define u8		unsigned char

#define KSEG1_STORE8(addr, value)			*(volatile u8 *)((addr)) = ((u8)value & 0xff)
#define KSEG1_LOAD8(addr)					*(volatile u8 *)((addr))
#define KCHIPERASEG1_STORE8(addr,value)		*(volatile unsigned char *)((addr)) = ((unsigned char)value & 0xff)
#define KCHIPERASEG1_LOAD8(addr)			*(volatile unsigned char *)((addr))
#define CHIPERASET_SPI(idx,value)			KCHIPERASEG1_STORE8(SPI_REG_BASE+idx, value)

#ifdef SPIBUG
#define GET_SPI(idx)	get_spi(idx)
#else
#define GET_SPI(idx)	KSEG1_LOAD8(SPI_REG_BASE + idx)
#endif

#define SET_SPI(idx,value) KSEG1_STORE8(SPI_REG_BASE+idx, value)

#define CHIPERASELECOFFCHIPERASET  0x4

#define FLASH_VERIFFY

#define PROGRAM_AAI_MODE   0x1
#define FLASH_SIZE		  0x100000

extern void delay __P((int));

int fl_erase_chip_spi __P((struct fl_map *, struct fl_device *));
int fl_erase_sector_spi __P((struct fl_map *, struct fl_device *, int ));
int fl_isbusy_spi __P((struct fl_map *, struct fl_device *, int, int, int));
int fl_reset_spi __P((struct fl_map *, struct fl_device *));
int fl_erase_suspend_spi __P((struct fl_map *, struct fl_device *));
int fl_erase_resume_spi __P((struct fl_map *, struct fl_device *));
int fl_program_spi __P((struct fl_map *, struct fl_device *, int , unsigned char *));

struct fl_functions fl_func_spi = {fl_erase_chip_spi,
                                   fl_erase_sector_spi,
                                   fl_isbusy_spi,
                                   fl_reset_spi,
                                   fl_erase_suspend_spi,
                                   fl_program_spi};

/*
 *   FLASH Programming functions.
 *
 *   Handle flash programing aspects such as different types
 *   organizations and actual HW design organization/interleaving.
 *   Combinations can be endless but an attempt to make something
 *   more MI may be done later when the variations become clear.
 */

static quad_t widedata;

#define	SETWIDE(x) do {						\
			u_int32_t __a = x;			\
			__a |= __a << 8;				\
			__a |= __a << 16;				\
			widedata = (quad_t)__a << 32 | __a;		\
		} while(0)




/*
 *  Function to erase sector
 */
int
fl_erase_sector_spi(map, dev, offset)
	struct fl_map *map;
	struct fl_device *dev;
	int offset;
{
#if 1
	spi_erase(offset, dev->fl_secsize * map->fl_map_chips);
#else
	int i, j, ret;
	char tmp;
	int n = dev->fl_secsize * map->fl_map_chips;

	enable_spi_cs();
	gpio_cs_init();
	spi_init0();

	DEBUG_PRINTF("Enable erase at %08x, size %d \n", offset, n);
	loongson_disabel_writeprotection();

	if (FLASH_SIZE > n)
	  loongson_spi_erase(offset, n);
	else {
	  printf("WARNING:Erase size larger than 1M, Full chip will be erased!\n");
	  loongson_chip_erase();
	}
#endif
	return 0;
}

/*
 *  Function to Program a byte
 */
int
fl_program_spi(map, dev, pa, pd)
	struct fl_map *map;
	struct fl_device *dev;
	int pa;
	unsigned char *pd;
{
	int i;

#if 1
#ifdef PROGRAM_AAI_MODE
	spi_program(pd,pa,1, 1); // in fast programm mode
#else
	spi_program(pd,pa,1, 0); // not fast programm mode
#endif
#else
	int fast = 1;

	if (fast)
	  loongson_spi_aai_write(pa, pd, 1);
	else
	  loongson_spi_write(pa, pd, 1);

	DEBUG_PRINTF("Programm over!\n");

#endif
	return(0);
}

/*
 *  Function to erase chip
 */
int
fl_erase_chip_spi(map, dev)
	struct fl_map *map;
	struct fl_device *dev;
{

#if 0
	DEBUG_PRINTF("%s: %d\n", __FILE__, __LINE__);

	enable_spi_cs();
	gpio_cs_init();
	spi_init0();

	//DEBUG_PRINTF("Enable erase at first ..... \n");
	DEBUG_PRINTF("%s: %d\n", __FILE__, __LINE__);
	loongson_disabel_writeprotection();

	/*  erase full spi flash */
	loongson_chip_erase();

	loongson_enabel_writeprotection();

	disable_spi_cs();

	DEBUG_PRINTF("Erase full spi flash chip done\n");
#else
	spi_chip_erase();
#endif
	return 0;

}

/*
 *  Function to "reset" flash, eg return to read mode.
 */
int
fl_reset_spi(map, dev)
	struct fl_map *map;
	struct fl_device *dev;
{
        return(0);
}

/*
 *  Function to poll flash BUSY if available.
 *  returns 1 if busy, 0 if OK, -1 if error.
 */
int
fl_isbusy_spi(map, dev, what, offset, erase)
	struct fl_device *dev;
	struct fl_map *map;
	int what;
        int offset;
	int erase;
{
	int busy;

	return 0;
}

/*
 *  Function to suspend erase sector
 */

int
fl_erase_suspend_spi(map, dev)
	struct fl_map *map;
	struct fl_device *dev;
{

	return(0);
}

/*
 *  Function to Disable the SPI write protection of SST25WF080/Disable the FIRMWARE HUB write protection of SST49LF008A
 *  For Loongson3 by wanghuandong(AdonWang, whd)
 */

	int
spi_write_protect_unlock(void)
{
	enable_spi_cs();
	gpio_cs_init();
	spi_init0();

	DEBUG_PRINTF("Enable erase at first ..... \n");
	loongson_disabel_writeprotection();

}

/*
 *  Function to enable the LPC write protection of SST25WF080/enable the FIRMWARE HUB write protection of SST49LF008A
 *  For Loongson3 by wanghuandong(AdonWang, whd)
 */

	int
spi_write_protect_lock(void)
{
	loongson_enabel_writeprotection();
	disable_spi_cs();
	return(1);

}


/* to enalbe read from 0xbfc00000 */
void disable_spi_cs(void)
{

#ifndef LS3B_SPI_BOOT
	delay(6);
	*(volatile unsigned int *) (GPIO_CTRL_REG) =  (~GPIO_CPU_CS_ENABLE) & (*(volatile unsigned int *) (GPIO_CTRL_REG));
#else
	volatile unsigned char * base = SPI_REG_BASE + CHIPERASELECOFFCHIPERASET;
	unsigned char  val = 0;

	val = *(base);
	val |= 0x01;
	*(base) = val;

#endif
}

void enable_spi_cs(void)
{
#ifndef LS3B_SPI_BOOT
	*(volatile unsigned int *) (GPIO_CTRL_REG) =  (GPIO_CPU_CS_ENABLE) | (*(volatile unsigned int *) (GPIO_CTRL_REG));
	delay(6);
#else
	volatile unsigned char * base = SPI_REG_BASE + CHIPERASELECOFFCHIPERASET;
	unsigned char  val = 0;

	val = *(base);
	val &= 0xfe; /* are you sure? ,set bit[0] = 1b'0 */
	*(base) = val;

#endif
}

void gpio_cs_init (void)
{
#ifdef LS3B_SPI_BOOT
	volatile unsigned char * base = SPI_REG_BASE + CHIPERASELECOFFCHIPERASET;
	unsigned char  val = 0;
	
	/* set off[5] bit 0 to be one to use bit[4] as cs bit */
	val = *(base+1);
	val |= 0x1;
	*(base+1)=val;

#else
	/* set gpio 0 output for godson3a, below for LS3A */
	*(volatile unsigned int *) (GPIO_CTRL_REG) =  (~GPIO_CS_BIT) & (*(volatile unsigned int *) (GPIO_CTRL_REG));

#endif
	delay(6);
}

/*
 * set gpio output value for godson3a
 */

inline void set_cs (int bit)
{
#ifdef LS3B_SPI_BOOT
#define CSCTLOFFCHIPERASET  0x5
	volatile unsigned char * base = SPI_REG_BASE + CSCTLOFFCHIPERASET;
	unsigned char  val = 0;

	//delay(6);

	val = *(base);
	if (!bit)
		val &= 0xef; /* are you sure? ,set bit[0] = 1b'0 */
	else
		val |= 0x10; /* are you sure? ,set bit[0] = 1b'1 */

	*(base) = val;
#else
	volatile unsigned int * base = GPIO_DATA_REG;
	unsigned int  val = 0;

	val = *(base);

	if (!bit)
		val |= GPIO_CS_BIT; /* are you sure? ,set bit[0] = 1b'1 */
	else
		val &= (~GPIO_CS_BIT); /* are you sure? ,set bit[0] = 1b'0 */

	*(base) = val;
#endif
	delay(6);
}

static unsigned char get_spi (int idx)
{
	KCHIPERASEG1_LOAD8 (SPI_REG_BASE + idx);
	return KCHIPERASEG1_LOAD8 (SPI_REG_BASE);
}

//static unsigned char loongson_spi_buf[512 * 1020]
static unsigned char loongson_spi_buf[512 * 2048]
__attribute__ ((section ("text"), aligned (512)));
static unsigned char tbuf[256]
__attribute__ ((section ("text")));

static inline unsigned char flash_writeb_cmd (unsigned char value)
{
	unsigned char ret;

	CHIPERASET_SPI (FCR_SPDR, value);
	while (GET_SPI (FCR_SPSR) & 0x01)
	{
		/* do nothing */
	}

	ret = GET_SPI (FCR_SPDR);
	return ret;
}

static inline unsigned char flash_read_data (void)
{
	unsigned char ret;

	CHIPERASET_SPI (FCR_SPDR, 0x00);
	while (GET_SPI (FCR_SPSR) & 0x01)
	{
		//delay(6); // by xqch
		/* do nothing */
	}

	ret = GET_SPI (FCR_SPDR);
	DEBUG_PRINTF("read_cmd: GET_SPI (FCR_SPDR) return %08x\n", ret);

	return ret;
}

void spi_init0(void)
{
	DEBUG_PRINTF("Enter spi_init0()\n");
	SET_SPI(FCR_SPCR, 0x10);  /* disable spi model */
	//SET_SPI(FCR_SPCR, 0x13);  /* disable spi model */
	//SET_SPI(FCR_SPCR, 0x12);  /* disable spi model */
	SET_SPI(FCR_SPSR, 0xc0);
	DEBUG_PRINTF("%s: %d\n", __FILE__, __LINE__);

	SET_SPI(FCR_SPER, 0x04);
	SET_SPI(FCR_PARAM, 0x20);
	//SET_SPI(FCR_PARAM, 0x32); // by xqch , divied 3, supprt continue read
	//SET_SPI(FCR_PARAM, 0x22); // by xqch , divied 3, supprt continue read
	DEBUG_PRINTF("%s: %d\n", __FILE__, __LINE__);

	//SET_SPI(FCR_SPCR, 0x51);  /* 00, id ok */
	SET_SPI(FCR_SPCR, 0x52);  /* 00, id ok */
	DEBUG_PRINTF("%s: %d\n", __FILE__, __LINE__);
}

int loongson_spi_readchipid (void)
{
	unsigned char vendor_id;
	unsigned char device_id;
	unsigned char device_id1;
	int i;

	enable_spi_cs();
	gpio_cs_init();
	spi_init0();

	set_cs (0);
	flash_writeb_cmd (RDID);
	flash_writeb_cmd (VOID_ADDR);
	flash_writeb_cmd (VOID_ADDR);
	flash_writeb_cmd (MACID_ADDR);
	vendor_id = flash_read_data ();
	device_id = flash_read_data();
	device_id1 = flash_read_data();
	device_id1 = flash_read_data();
	set_cs (1);

	disable_spi_cs();

	return device_id;

}



int loongson_spi_readid (void)
{
#define MACID_ADDR     0x00
#define DEVID_ADDR     0x01
#define VOID_ADDR      0x00
#define VOID_CMD       0x00

	unsigned char vendor_id;
	unsigned char device_id;
	unsigned char device_id1;
	unsigned char mem_type;
	unsigned char mem_size;
	int i;


	enable_spi_cs();
	gpio_cs_init();
	spi_init0();

	set_cs (0);
	flash_writeb_cmd (RDID);
	flash_writeb_cmd (VOID_ADDR);
	flash_writeb_cmd (VOID_ADDR);
	flash_writeb_cmd (MACID_ADDR);
	vendor_id = flash_read_data ();
	device_id = flash_read_data();
	device_id1 = flash_read_data();
	device_id1 = flash_read_data();
	set_cs (1);
	//DEBUG_PRINTF("vendor_id=0x%x\t,device_id=0x%x, device_id1=0x%x\n", vendor_id, device_id, device_id1);

	disable_spi_cs();

	return vendor_id;
}

#define SPI_FREE  0x00
#define SPI_BUSY  0x01

int spi_wait (void)
{
	int timeout = 10000;
	int ret = 1;
	int i;

	while ((ret & 1))	/* wait WIP */
	{
		set_cs (0);
		//delay(6);
		flash_writeb_cmd (RDSR);
		ret = flash_read_data (); /* Are you sure bit0 means busy or FREE? */
		//delay(6);
		set_cs (1);
	}
	return ((ret&1) == 1 ? SPI_BUSY: SPI_FREE) ;
}

/* byte write */
int loongson_spi_write(int offset, void *buffer, size_t n)
{
	unsigned int pos = offset;
	unsigned char *buf = (unsigned char *) buffer;
	int i,ret,j;
	unsigned char val_back;

	for(i=0;i<n;i++)
	{

		if ( (j & 0x3fff) == 0x0 )
		  printf(".");

		set_cs (0);
		delay(6);
		flash_writeb_cmd (WREN);
		delay(6);
		set_cs (1);
		delay(6);

		set_cs (0);
		delay(6);
		flash_writeb_cmd(0x2);
		flash_writeb_cmd (((pos+i) >> 16) & 0xff);
		flash_writeb_cmd (((pos+i) & 0x00ff00 )>> 8);
		flash_writeb_cmd ((pos+i) & 0xff);
		flash_writeb_cmd (buf[i]);
		delay(6);
		set_cs (1);

		ret = spi_wait();
	}

	//set_cs (0);
	//flash_writeb_cmd(WRDI);
	//set_cs (1);
	DEBUG_PRINTF("\nWrite %d bytes to SPI flash done!\n",  i);

	return 0;
}


int loongson_spi_aai_write(int offset, void *buffer, size_t n)
{
	unsigned int pos = offset;
	unsigned char *buf = (unsigned char *) buffer;
	int i,ret,j;
	unsigned char val_back;


	i = j = 0x0;
	set_cs (0);
	flash_writeb_cmd (WREN);
	set_cs (1);

	delay(6);
	set_cs (0);
	flash_writeb_cmd(AAI_WRITE); // byte programm enable
	flash_writeb_cmd(((pos) >> 16) & 0xff);
	flash_writeb_cmd(((pos) & 0x00ff00 )>> 8);
	flash_writeb_cmd((pos) & 0xff);
	flash_writeb_cmd(buf[j++]);
	flash_writeb_cmd(buf[j++]);
	set_cs (1);
	spi_wait();
	set_cs (0);

	while(j< ((n & 0x1) ? n+1:n))
	{

		if ( (j & 0x3fff) == 0x0 )
		  printf(".");
		flash_writeb_cmd(AAI_WRITE); // byte programm enable
		flash_writeb_cmd(buf[j++]);
		flash_writeb_cmd(buf[j++]);
		set_cs (1);
		spi_wait();
		set_cs (0);
	}

	set_cs (0);
	flash_writeb_cmd(WRDI);
	set_cs (1);

	spi_wait();

	return 0;
}


loongson_disabel_writeprotection(void)
{
	//DEBUG_PRINTF("disable write protection\n");
#if 0
	set_cs (0);
	flash_writeb_cmd(WREN);
	set_cs (1);
	delay(6);

#else
	set_cs (0);
	flash_writeb_cmd (EWSR);
	set_cs (1);
	delay(6);
#endif

	set_cs (0);
	flash_writeb_cmd (WRSR);
	flash_writeb_cmd (0);
	//flash_writeb_cmd (0);
	set_cs (1);

	delay(6);
	spi_wait();
	delay(6);

	set_cs (0);
	flash_writeb_cmd(WREN);
	set_cs (1);
	delay(6);
}

loongson_enabel_writeprotection(void)
{
	DEBUG_PRINTF("Enable write protection\n");
	set_cs (0);
	flash_writeb_cmd(WREN);
	set_cs (1);
	delay(6);

	set_cs (0);
	flash_writeb_cmd (WRSR);
	flash_writeb_cmd (0x1c);
	set_cs (1);

	delay(6);
	spi_wait();
	delay(6);

	set_cs (0); 
	flash_writeb_cmd(WRDI);
	set_cs (1);

	DEBUG_PRINTF("Enable write protection done\n");

	delay(0x100);
	
}

int spi_erase(int offset, size_t n)
{	
	int i;

	enable_spi_cs();
	gpio_cs_init();
	spi_init0();

	
	DEBUG_PRINTF("Enable erase at %08x, size %d \n", offset, n);
	loongson_disabel_writeprotection();

	if (FLASH_SIZE > n)
	  loongson_spi_erase(offset, n);
	else {
	  printf("WARNING:Erase size larger than 1M, Full chip will be erased!\n");
	  loongson_chip_erase();
	}

	loongson_spi_read(offset, loongson_spi_buf, n); // by xqch to check

	for( i = 0; i < n; i++)
	  if (loongson_spi_buf[i] != 0xff) {
		printf("Erasy Error @ %08x: %08x\n", i, loongson_spi_buf[i]);
		break;
	  }
	if (i == n)
	  DEBUG_PRINTF("Erasy verify ok!\n");
	else {
	  DEBUG_PRINTF("Erasy verify fail!\n");
	  return -1;
	}

	loongson_enabel_writeprotection();

	disable_spi_cs();

	return 0;
}

/* erase some part of the chip */
// NOTICE: assume offset divided by 4K
int loongson_spi_erase(int off, unsigned int n)
{
		
	unsigned int  offset = off;
	int i = 0;
	unsigned addr0;
	unsigned addr1;
	unsigned addr2;

	DEBUG_PRINTF("Erase size: %08x @ % 08x\n", n, off);

	do {
	  addr0 =  offset & 0xff;
	  addr1 =  (offset & 0xff00) >> 8;
	  addr2 =  (offset & 0xff0000) >> 16;

	  spi_wait();
	  set_cs (1);
	  spi_wait();
	  set_cs (0);
	  //flash_writeb_cmd (EWSR);
	  flash_writeb_cmd (WREN);
	  set_cs (1);
	  spi_wait();

	  DEBUG_PRINTF(".");
	  set_cs (0);
	  flash_writeb_cmd (BLKERASE);
	  flash_writeb_cmd (addr2);
	  flash_writeb_cmd (addr1);
	  flash_writeb_cmd (addr0);
	  set_cs (1);
	  
	  offset += BLKSIZE;
	  spi_wait();
	} while(offset < n + off);

	return 0;
}


/*
 * erase all chips.
 */

int loongson_chip_erase(void)
{
#if 0
	enable_spi_cs();
	gpio_cs_init();
	spi_init0();

	DEBUG_PRINTF("Enable erase at first ..... \n");
	DEBUG_PRINTF("Erase full chip \n");
	loongson_disabel_writeprotection();
#endif
	  set_cs (0);
	  //flash_writeb_cmd (EWSR);
	  flash_writeb_cmd (WREN);
	  set_cs (1);

	spi_wait();

	set_cs (0);
	flash_writeb_cmd (CHIPERASE);
	set_cs (1);

	spi_wait();

#if 0
	loongson_enabel_writeprotection();

	disable_spi_cs();

#endif
	return 0;

}


int loongson_spi_bulk_erase (void)
{
	int i, j, ret;
	unsigned char * buf;

	enable_spi_cs();
	gpio_cs_init();
	spi_init0();

	ret = 0;
	//DEBUG_PRINTF("bulk1erase\n");

	set_cs (0);
	flash_writeb_cmd (EWSR);
	set_cs (1);
	//delay(1);
	//DEBUG_PRINTF("bulk3erase\n");

	set_cs (0);
	flash_writeb_cmd (WRSR);
	flash_writeb_cmd (0);
	flash_writeb_cmd (0);
	set_cs (1);
	//delay(1);
	DEBUG_PRINTF("bulk4erase\n");

	set_cs (0);
	flash_writeb_cmd (WREN);
	set_cs (1);
	//delay(1);

	DEBUG_PRINTF("bulk5erase\n");
	set_cs (0);
	flash_writeb_cmd (CHIPERASE);
	set_cs (1);

	DEBUG_PRINTF("bulk6erase\n");
	//delay(1200);
#if 1
	for (j = 0; j < 1; j++)
	{
		ret = spi_wait ();
		if (!ret)
			break;
	}
	if(ret)
	{
		DEBUG_PRINTF("erase timeout\n");
		return ret;
	}	
#endif
	DEBUG_PRINTF("Erase over!\n");
#ifdef FLASH_VERIFFY
	for(i = 0; i < LOONGSON_SPI_SIZE/512; i++)
	{
		loongson_spi_read(i*512, loongson_spi_buf, 512);
		buf = (unsigned char *)loongson_spi_buf;
		for(j = 0; j < 512; j++)
		{
			if(buf[j] != 0xff)
			{
				DEBUG_PRINTF("erase verify err at %x\n", i*512+j);
				return 1;
			}
			else
			{
				if(i % 0x40 == 0 && 0 == j)
					DEBUG_PRINTF(".. %08x\n", i);
			}
		}
	}
	DEBUG_PRINTF("\nverify over\n");
#endif
	return 0;
}

int loongson_spi_read(int offset, void *buffer, size_t n)
{
	unsigned int pos = offset;
	unsigned char *buf = (unsigned char *) buffer;
	int i;

	//delay(1);
	set_cs (0);
	flash_writeb_cmd (FAST_READ);
	flash_writeb_cmd ((pos >> 16) & 0xff);
	flash_writeb_cmd ((pos >> 8) & 0xff);
	flash_writeb_cmd (pos & 0xff);
	flash_writeb_cmd (0);  /* dummy read */
	for (i = 0; i < n; i++)
		buf[i] = flash_read_data();
	set_cs (1);

	return (n-i);

}

int spi_read(int offset, void *buffer, size_t n)
{
	int ret;

	enable_spi_cs();
	gpio_cs_init();
	spi_init0();

	ret = loongson_spi_read(offset, buffer, n);

	disable_spi_cs();

	return ret;
}

/*
 * ease and program
 */
int spi_program(unsigned int base,unsigned int offset,unsigned long len, int fast)
{
	int i, j, ret;
	char tmp;
	int n = len;

	enable_spi_cs();
	gpio_cs_init();
	spi_init0();

#if 0
	DEBUG_PRINTF("Enable erase at %08x, size %d \n", offset, n);
	loongson_disabel_writeprotection();

	if (FLASH_SIZE > n)
	  loongson_spi_erase(offset, n);
	else {
	  printf("WARNING:Erase size larger than 1M, Full chip will be erased!\n");
	  loongson_chip_erase();
	}

	loongson_spi_read(offset, loongson_spi_buf, n); // by xqch to check

	for( i = 0; i < n; i++)
	  if (loongson_spi_buf[i] != 0xff) {
		printf("Erasy Error @ %08x: %08x\n", i, loongson_spi_buf[i]);
		break;
	  }
	if (i == n)
	  DEBUG_PRINTF("Erasy verify ok!\n");
	else {
	  printf("Erasy verify fail!\n");
	  return -1;
	}
#endif

	loongson_disabel_writeprotection();

	if (fast)
	  loongson_spi_aai_write(offset, base, len);
	else
	  loongson_spi_write(offset, base, len);

	//printf("Programm over!\n");
	DEBUG_PRINTF("Programm over!\n");

	loongson_enabel_writeprotection();

	loongson_spi_read(offset, loongson_spi_buf, len); // by xqch to check

	for( i = 0; i < len; i++)
	  if (loongson_spi_buf[i] != ((unsigned char *)base)[i]) {
		printf("Verify Error @ %08x: %08x\n", ((unsigned char *)base)[i], loongson_spi_buf[i]);
		break;
	  }

	if (i == len)
	  DEBUG_PRINTF("Program Verify ok!\n");
	  //printf("Program Verify ok!\n");
	else
	  DEBUG_PRINTF("Program Verify fail!\n");

	disable_spi_cs();
	return 0;
}

int cmd_getid(ac, av)
	int ac;
	char *av[];
{
	int mfgid, chipid;

	DEBUG_PRINTF("start wspi\n");
	//enable_spi_cs();
	mfgid = loongson_spi_readid();
	chipid = loongson_spi_readchipid();
	//disable_spi_cs();
	DEBUG_PRINTF("wspi done\n");
	//DEBUG_PRINTF("vendor_id=0x%x\n", vendor_id);
	//DEBUG_PRINTF("Mfg %2x\n", vendor_id);
	printf("Mfg %2x, Id %2x\n", mfgid, chipid);

	return(1);
}

spi_chip_erase(void)
{
	enable_spi_cs();
	gpio_cs_init();
	spi_init0();

	DEBUG_PRINTF("Enable erase at first ..... \n");
	loongson_disabel_writeprotection();

	/*  erase full spi flash */
	loongson_chip_erase();

	loongson_enabel_writeprotection();

	disable_spi_cs();

	DEBUG_PRINTF("Erase full spi flash chip done\n");
	return 0;

}
cmd_eraseall(ac, av)
	int ac;
	char *av[];
{

	/*  erase full spi flash */
	spi_chip_erase();

	DEBUG_PRINTF("Erase full spi flash chip done\n");

	return 0;
}

cmd_erase(ac, av)
	int ac;
	char *av[];
{
	unsigned int offset;
	unsigned long length;

	if ( ac != 3)
	{
		printf("Usage: spierase [offset] [length]\n");
		return;
	}
	offset = strtoul(av[1],0,0);
	length = strtoul(av[2],0,0);

	//DEBUG_PRINTF("my Erase offset: %08x  , length %08x\n", offset, length);
	spi_erase(offset, length);
	DEBUG_PRINTF("Erase done\n");

	return(1);
}




cmd_program(ac, av)
	int ac;
	char *av[];
{
	unsigned int base,mode;
	unsigned int offset;
	unsigned long length;

	if ( ac <= 3)
	{
		printf("Usage: spiprogram [src] [offset] [length] [fast=1]\n");
		return;
	}


	base = strtoul(av[1],0,0);
	offset = strtoul(av[2],0,0);
	length = strtoul(av[3],0,0);

	mode = 0x1; //default fast write

	if ( ac > 4)
	  mode = strtoul(av[4],0,0);

	//spi_erase(offset, length);
	spi_program(base,offset,length, mode);

	DEBUG_PRINTF("Programm done\n");
	return(1);
}



int cmd_read(argc, argv)
	int argc;
	char *argv[];
{
	//unsigned char *	buf = (unsigned char *)loongson_spi_buf;
	unsigned char *	buf;
	unsigned long offset, len, i;

	//DEBUG_PRINTF("spiread [offset]  [len]: Read %d bytes from spi flash offset %08x ....\n", len, offset);

	offset=strtoul(argv[1],0,0);
	buf =strtoul(argv[2],0,0);
	len =strtoul(argv[3],0,0);

	printf("Usage: spiread [offset] [buf] [len]\n", len, (unsigned long)buf,offset);
	spi_read(offset, buf, len);

//	for ( i = 0; i < 10; i++)
//	  DEBUG_PRINTF("%08x: %08x\n", offset+i, buf[i]);

	DEBUG_PRINTF("Read done\n");
	return(1);
}

/*
 *
 *  Command table registration
 *  ==========================
 */

static const Cmd Cmds[] =
{
	{"Misc"},
	{"spiid",	"", 0, "Show spi flash id ", cmd_getid, 1, 99, 0},
	{"spieraseall",	"", 0, "Erase spi flash full chip ", cmd_eraseall, 1, 99, 0},
	{"spierase",	"", 0, "Erase spi flash full chip ", cmd_erase, 1, 99, 0},
	{"spiprogram",	"", 0, "Programm spi flash with LPC Flash", cmd_program, 1, 99, 0},
	{"spiread",	"", 0, "Programm spi flash with LPC Flash", cmd_read, 1, 99, 0},
	{0, 0}
};

static void init_cmd __P((void)) __attribute__ ((constructor));

void init_cmd()
{
	cmdlist_expand(Cmds, 1);
}
