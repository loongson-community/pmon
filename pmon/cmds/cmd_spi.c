#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/device.h>
#include <sys/queue.h>
#include <pmon.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/kernel.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/buf.h>
#include <sys/uio.h>
#include <sys/malloc.h>
#include <sys/errno.h>
#include <sys/device.h>
#include <sys/disklabel.h>
#include <sys/disk.h>
#include <sys/queue.h>
#include <pmon.h>

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
#define DEBUG_PRINTF(fmt, args...)  printf(##fmt, ##args)
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

//#ifdef LS3B
#define BE4K			0x20  /* 4K Byte block Rrase, Sector Erase */
#define BE4KSIZE		0x1000  /* 4K Byte block Rrase, Sector Erase */
#define BE32K			0x52  /* 32K Byte block Rrase, Sector Erase */
#define BE32KSIZE		0x8000/* 32K Byte block Rrase, Sector Erase */
#define BE64K			0xD8  /* 64K Byte block Rrase, Sector Erase */
#define BE64KSIZE		0x10000/* 64K Byte block Rrase, Sector Erase */
#define CHIPERASE		0xC7  /* Chip Erase */

//#define BLKERASE		BE32K
//#define BLKSIZE			BE32KSIZE
#define BLKERASE		BE64K
#define BLKSIZE			BE64KSIZE
//#else
//#define CHIPERASE		0xC7  /* Block Rrase, Sector Erase */
//#endif

#define MACID_ADDR     0x00
#define DEVID_ADDR     0x01
#define VOID_ADDR      0x00
#define VOID_CMD       0x00

#ifdef LS3B
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

#define FLASH_VERIFFY

void disable_cpu_cs(void)
{
	*(volatile unsigned int *) (GPIO_CTRL_REG) =  (~GPIO_CPU_CS_ENABLE) & (*(volatile unsigned int *) (GPIO_CTRL_REG));
	delay(10);
}

void enable_cpu_cs(void)
{
	*(volatile unsigned int *) (GPIO_CTRL_REG) =  (GPIO_CPU_CS_ENABLE) | (*(volatile unsigned int *) (GPIO_CTRL_REG));
	delay(10);
}

void gpio_cs_init (void)
{
#ifdef LS3B
#define CHIPERASELECOFFCHIPERASET  0x4
	volatile unsigned char * base = SPI_REG_BASE + CHIPERASELECOFFCHIPERASET;
	unsigned char  val = 0;

	val = *(base);
	val &= 0xfe; /* are you sure? ,set bit[0] = 1b'0 */
	*(base) = val;

	/* set off[5] bit 0 to be one to use bit[4] as cs bit */
	val = *(base+1);
	val |= 0x1;
	*(base+1)=val;

#else
	/* set gpio 0 output for godson3a, below for LS3A */
	*(volatile unsigned int *) (GPIO_CTRL_REG) =  (~GPIO_CS_BIT) & (*(volatile unsigned int *) (GPIO_CTRL_REG));

#endif
	delay(10);
}

/*
 * set gpio output value for godson3a
 */

inline void set_cs (int bit)
{
#ifdef LS3B
#define CSCTLOFFCHIPERASET  0x5
	volatile unsigned char * base = SPI_REG_BASE + CSCTLOFFCHIPERASET;
	unsigned char  val = 0;

	/////delay(100);

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
	delay(10);
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
		//delay(10); // by xqch
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

	enable_cpu_cs();

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

	disable_cpu_cs();

	return device_id;

}



int loongson_spi_readid (void)
{

	unsigned char vendor_id;
	unsigned char device_id;
	unsigned char device_id1;
	int i;

	enable_cpu_cs();

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

	disable_cpu_cs();

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
		//delay(100);
		flash_writeb_cmd (RDSR);
		ret = flash_read_data (); /* Are you sure bit0 means busy or FREE? */
		//delay(100);
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

#if 0
		if ( (i & 0x1fff) == 0x0 )
		  printf("<");
#endif
		set_cs (0);
		//delay(100);
		flash_writeb_cmd (WREN);
		//delay(100);
		set_cs (1);

		set_cs (0);
		//delay(100);
		flash_writeb_cmd(0x2);
		flash_writeb_cmd (((pos+i) >> 16) & 0xff);
		flash_writeb_cmd (((pos+i) & 0x00ff00 )>> 8);
		flash_writeb_cmd ((pos+i) & 0xff);
		flash_writeb_cmd (buf[i]);
		//delay(100);
		set_cs (1);

		ret = spi_wait();
	}

	//set_cs (0);
	//flash_writeb_cmd(WRDI);
	//set_cs (1);
	//printf("\nWrite %d bytes to SPI flash done!\n",  i);

	return 0;
}


int loongson_spi_aai_write(int offset, void *buffer, size_t n)
{
	unsigned int pos = offset;
	unsigned char *buf = (unsigned char *) buffer;
	int i,ret,j;
	unsigned char val_back;

	//gpio_cs_init();
	//spi_init0();

	i = j = 0x0;
	set_cs (0);
	flash_writeb_cmd (WREN);
	set_cs (1);

	delay(100);
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

		if ( (j & 0x1fff) == 0x0 )
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
	//printf("disable write protection\n");
	set_cs (0);
	flash_writeb_cmd (EWSR);
	set_cs (1);

	delay(20);
	set_cs (0);
	flash_writeb_cmd (WRSR);
	flash_writeb_cmd (0);
	//flash_writeb_cmd (0);
	set_cs (1);

	spi_wait();
	delay(20);

	set_cs (0);
	flash_writeb_cmd(WREN);
	set_cs (1);

	spi_wait();
	delay(20);
}

loongson_enabel_writeprotection(void)
{
//	printf("Enable write protection\n");
	set_cs (0);
	flash_writeb_cmd (EWSR);
	set_cs (1);
	delay(20);

	//delay(1);
	set_cs (0);
	flash_writeb_cmd (WRSR);
	flash_writeb_cmd (0x1c);
	//flash_writeb_cmd (0x1c);
	set_cs (1);
	spi_wait();
	//delay(20);

	set_cs (0); 
	flash_writeb_cmd(WRDI);
	set_cs (1);

//	printf("Enable spi burst read\n");
	SET_SPI(FCR_PARAM, 0x03); // enable burst read

	
}

int spi_erase(int offset, size_t n)
{	
	enable_cpu_cs();
	gpio_cs_init();
	spi_init0();

	DEBUG_PRINTF("Enable erase at first ..... \n");
	loongson_disabel_writeprotection();

	loongson_spi_erase(offset, n);

	loongson_enabel_writeprotection();

	disable_cpu_cs();

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

	//printf("Erase size: %d\n", n);

	do {
	  addr0 =  offset & 0xff;
	  addr1 =  (offset & 0xff00) > 8;
	  addr2 =  (offset & 0xff0000) > 16;

	  set_cs (0);
	  //flash_writeb_cmd (EWSR);
	  flash_writeb_cmd (WREN);
	  set_cs (1);

	  spi_wait();

	  printf(".");
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
	enable_cpu_cs();
	gpio_cs_init();
	spi_init0();

	DEBUG_PRINTF("Enable erase at first ..... \n");
	printf("Erase full chip \n");
	loongson_disabel_writeprotection();
#endif
	  set_cs (0);
	  //flash_writeb_cmd (EWSR);
	  flash_writeb_cmd (WREN);
	  set_cs (1);


	set_cs (0);
	flash_writeb_cmd (CHIPERASE);
	set_cs (1);

	spi_wait();

#if 0
	loongson_enabel_writeprotection();

	disable_cpu_cs();

#endif
	return 0;

}


int loongson_spi_bulk_erase (void)
{
	int i, j, ret;
	unsigned char * buf;

	gpio_cs_init();
	spi_init0();

	ret = 0;
	//printf("bulk1erase\n");

	set_cs (0);
	flash_writeb_cmd (EWSR);
	set_cs (1);
	//delay(1);
	//printf("bulk3erase\n");

	set_cs (0);
	flash_writeb_cmd (WRSR);
	flash_writeb_cmd (0);
	flash_writeb_cmd (0);
	set_cs (1);
	//delay(1);
	printf("bulk4erase\n");

	set_cs (0);
	flash_writeb_cmd (WREN);
	set_cs (1);
	//delay(1);

	printf("bulk5erase\n");
	set_cs (0);
	flash_writeb_cmd (CHIPERASE);
	set_cs (1);

	printf("bulk6erase\n");
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
		printf("erase timeout\n");
		return ret;
	}	
#endif
	DEBUG_PRINTF("Erase over!\n");
#ifdef FLASH_VERIFFY
	for(i = 0; i < LOONGSON_SPI_SIZE/512; i++)
	{
		spi_read(i*512, loongson_spi_buf, 512);
		buf = (unsigned char *)loongson_spi_buf;
		for(j = 0; j < 512; j++)
		{
			if(buf[j] != 0xff)
			{
				printf("erase verify err at %x\n", i*512+j);
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

static int spi_read(int offset, void *buffer, size_t n)
{
	unsigned int pos = offset;
	unsigned char *buf = (unsigned char *) buffer;
	int i;

	enable_cpu_cs();
	gpio_cs_init();
	spi_init0();

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

	disable_cpu_cs();

	return (n-i);
}


int spi_program(unsigned int base,unsigned int offset,unsigned long len, int fast)
{
	int i, j, ret;
	char tmp;

	enable_cpu_cs();

	gpio_cs_init();
	spi_init0();

	DEBUG_PRINTF("Enable erase at first ..... \n");
	loongson_disabel_writeprotection();

	/*  erase full spi flash */
	//printf("Begin erase ..... \n");
#if 1
	loongson_spi_erase(offset, len);
#else
	loongson_chip_erase();
	printf("Erase full spi flash chip done\n");
#endif
	loongson_enabel_writeprotection();

	spi_read(offset, loongson_spi_buf, len); // by xqch to check

	for( i = 0; i < len; i++)
	  if (loongson_spi_buf[i] != 0xff) {
		printf("Erasy Error @ %08x: %08x\n", i, loongson_spi_buf[i]);
		break;
	  }
	if (i == len)
	  printf("Erasy ok!\n");
	else {
	  printf("Erasy fail!\n");
	  return -1;
	}

	loongson_disabel_writeprotection();
	printf("Now Begin programm\n");
	if (fast)
	  loongson_spi_aai_write(offset, base, len);
	else
	  loongson_spi_write(offset, base, len);

	printf("Programm over!\n");

	loongson_enabel_writeprotection();

	spi_read(offset, loongson_spi_buf, len); // by xqch to check

	for( i = 0; i < len; i++)
	  if (loongson_spi_buf[i] != ((unsigned char *)base)[i]) {
		printf("Verify Error @ %08x: %08x\n", ((unsigned char *)base)[i], loongson_spi_buf[i]);
		break;
	  }

	if (i == len)
	  printf("Verify ok!\n");
	else
	  printf("Verify fail!\n");


	disable_cpu_cs();

	return 0;
}


int cmd_getid(ac, av)
	int ac;
	char *av[];
{
	int mfgid, chipid;

	DEBUG_PRINTF("start wspi\n");
	//enable_cpu_cs();
	mfgid = loongson_spi_readid();
	//disable_cpu_cs();
	DEBUG_PRINTF("wspi done\n");
	//printf("vendor_id=0x%x\n", vendor_id);
	//printf("Mfg %2x\n", vendor_id);
	printf("Mfg %2x, Id %2x\n", mfgid, chipid);

	return(1);
}

cmd_eraseall(ac, av)
	int ac;
	char *av[];
{

	enable_cpu_cs();
	gpio_cs_init();
	spi_init0();

	DEBUG_PRINTF("Enable erase at first ..... \n");
	loongson_disabel_writeprotection();

	/*  erase full spi flash */
	loongson_chip_erase();

	loongson_enabel_writeprotection();

	disable_cpu_cs();

	printf("Erase full spi flash chip done\n");
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

	//printf("my Erase offset: %08x  , length %08x\n", offset, length);
	spi_erase(offset, length);
	printf("Erase done\n");

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

	spi_program(base,offset,length, mode);

	printf("Programm done\n");
	return(1);
}



int cmd_read(argc, argv)
	int argc;
	char *argv[];
{
	//unsigned char *	buf = (unsigned char *)loongson_spi_buf;
	unsigned char *	buf;
	unsigned long offset, len, i;

	//printf("spiread [offset]  [len]: Read %d bytes from spi flash offset %08x ....\n", len, offset);

	offset=strtoul(argv[1],0,0);
	buf =strtoul(argv[2],0,0);
	len =strtoul(argv[3],0,0);

	printf("spiread [offset] [buf] [len]: Read %d bytes from spi to %08x flash offset %08x ....\n", len, (unsigned long)buf,offset);
	spi_read(offset, buf, len);

//	for ( i = 0; i < 10; i++)
//	  printf("%08x: %08x\n", offset+i, buf[i]);

	printf("Read done\n");
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
