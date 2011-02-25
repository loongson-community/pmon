/*	$Id: wspi.c,v 1.1.1.1 2006/09/14 01:59:08 root Exp $ */

/*
 * Copyright (c) 2001 Opsycon AB  (www.opsycon.se)
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
 *	This product includes software developed by Opsycon AB, Sweden.
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
#include <sys/device.h>
#include <sys/queue.h>

#include <pmon.h>


////////////////////////////////////////////////////////////////////////////

/*
 * =====================================================================================
 * spi_m25p128.c
 *
 * Description:
 *  
 *
 * Copyright (c) 2007 National Research Center for Intelligent Computing
 * Systems. ALL RIGHTS RECHIPERASERVED. See "COPYRIGHT" for detail
 *
 * Created:  2010??08??03?? 00ʱ08??51??
 * Revision:  none
 * Compiler:  gcc
 *
 * Author:  JiangTao (), jiangtao@ncic.ac.cn
 * Company:  CAR Group, NCIC,ICT,CAS.
 *
 *spi_m25p128????
 *
 *???ߣ????? ???ڣ?2010/08/23
 *
 *?޸??ļ???
 *sys/dev/spi/spi_m25p128.c	spi??m25p128???????ļ?
 *pmon/cmds/cmd_spi_flash.c	spi flash???????ļ?
 *pmon/cmds/load.c			load???????ļ?
 *pmon/fs/devfs.c				PMON?豸?ļ?ϵͳ?????Ӷ?spi flash?ķ??ʿ???
 *Targets/Bonito3amcp68/conf/files.Bonito3amcp68	?????????ļ?
 *
 *
 *?????????
 *bootflash	??flash????linux?ں?
 *flash_be	??Ƭ?��?flash
 *flash_se	flash?��????ÿ?β��?1??ҳ?棨256?ֽڣ?
 *flash_w		??дflash
 *flash_id	??ȡflash??ID
 *reset_flash	????????spi??Ƶ?ʣ???λ?Ȳ???
 *load -p	-r	??дlinux?ں˵?flash??512?ֽڴ????ں˵?sizeд??flash??0?ֽڴ???
 *
 *
 *bootflash	??flash????linux?ں?
 *	linux?ں˴?????flash??512?ֽ?λ?á???1??ҳ????ǰ4?ֽ????ڴ????ں˵?size??Ϣ????ΪPMON???豸??д??????512?ֽڶ??룬?????ں˷???512?ֽڡ?
 *	??????linux?����??????в???
 *	ʾ????bootflash root=/....
 *
 *flash_be	??Ƭ?��?flash
 *	????????
 *	ʾ????flash_be
 *	
 *flash_se	flash?��????ÿ?β��?1??ҳ?棨256?ֽڣ?
 *	??????-s ?��??Ĵ?С??-f ?��?????ʼ??ַ
 *	ʾ????flash_se -f 0 -s 256
 *	ע?⣺?��?????????ʼ??ַ?ʹ?С??????256?ֽڶ??룬?????????????룬????ʼ??ַ????Բ????256????size??offset???Ӳ?????Բ????256??ԭ?򣺰?????Ҫ?��????䡣
 *	
 *flash_w		??дflash
 *	??????-s ??С??-f ??д????ʼ??ַ??-o ??д?ļ????ڴ???ַ
 *	ʾ????flash_w -f 512 -s 0x600000 -o 0x81000000
 *
 *flash_id	??ȡflash??id
 *	????????
 *	ʾ????	flash_id
 *	
 *reset_flash	????????spi????
 *	??????-m ģʽ??0 cpol=cpha=0; 1 cpol=0,cpha=1; 2 cpol=1,cpha=0; 3 cpol=cpha=1;??-d spi??Ƶϵ????ԭʼƵ??��??PCI 33MHz??
 *	ʾ????reset_flash -m 0 -d 2
 *	
 *load -p	-r	??д?ں˵?flash??512?ֽڴ????˴??ں??ļ?????Ϊelf??ʽ
 *	??????load?????????в???
 *	ʾ????load -p -r tftp://10.10.102.6/vmlinux
 *
 * TODO:
 *
 * =====================================================================================
 */

#include <sys/types.h>
#include <sys/param.h>
//#include <sys/systm.h>
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
//#include "flash_ecc.h"

#define M25P128_PAGE_SIZE    256      // why here it is 256
#define ECC_BACHIPERASE            0xfD0000 //SAVE ECC for every page(256B). 3Bytes per page
//#define M25P128_SIZE    0x1000000
#define M25P128_SIZE    0x200000      //32M b
#define PMON_SIZE		0x80000
//#define PMON_SIZE		20

#define FLASH_VERIFY


#define GPIO_0 (0x01<<0)
#define GPIO_1 (0x01<<1)
#define GPIO_2 (0x01<<2)
#define GPIO_3 (0x01<<3)

#define GPIO_DATA_REG	0xbfe0011c
#define GPIO_CTRL_REG	0xbfe00120

#define GPIO_CS_BIT GPIO_0

//#define DEBUG
#ifdef DEBUG
#define DEBUG_PRINTF(fmt, args...)  printf(##fmt, ##args)
#else
#define DEBUG_PRINTF(...) do{}while(0)
#endif



#define SPE_SHIFT   6
#define MSTR_SHIFT   4
#define CPOL_SHIFT   3
#define CPHA_SHIFT   2
#define SPR_SHIFT   0
#define SPE   (1<<SPE_SHIFT)
#define MSTR   (1<<MSTR_SHIFT)
#define FCR_SPCR        0x00
#define FCR_SPSR        0x01
#define FCR_SPDR        0x02
#define FCR_SPER        0x03
#define FCR_TIME		0x06
#define FCR_PARAM		0x04

#define FCR_SPSR        0x01
#define FCR_SPDR        0x02
#define FCR_SPER        0x03


#define WREN    0x06
#define WRDI    0x04
#define RDID    0x90
#define RDSR    0x05
#define WRSR    0x01
#define READ    0x03
#define AAI	    0xAD
#define EBSY	0x70
#define DBSY	0x80

#define	EWSR	0x50
#define FAST_READ    0x0B
#define PP		0x02  // Byte Program Enable

#ifdef LS3B
#define BE4K      0x20  // 64K Byte block Rrase, Sector Erase
#define BE32K     0x52  // 64K Byte block Rrase, Sector Erase
#define BE64K     0xD8  // 64K Byte block Rrase, Sector Erase
#define BE        0xD8  // 64K Byte block Rrase, Sector Erase
#define CHIPERASE 0xC7  // Chip Erase
#else
#define CHIPERASE 0xD8  // Block Rrase, Sector Erase
#define BE        0xC7  // Chip Erase
#endif

#ifdef LOONGSON_2F
#define SPI_REG_BASE 0xbfe80000		// 2F
#else
#ifdef LS3B
#define SPI_REG_BASE 0xbfe00220		// 3B
#else
#define SPI_REG_BASE 0xbfe001f0		// 3A
#endif
#endif

#define FAST_PROGRAMM
#define u8  unsigned char

#define KSEG1_STORE8(addr, value)   *(volatile u8 *)((addr)) = ((u8)value & 0xff)
#define KSEG1_LOAD8(addr)   *(volatile u8 *)((addr))
#define KCHIPERASEG1_STORE8(addr, value)   *(volatile unsigned char *)((addr)) = ((unsigned char)value & 0xff)
#define KCHIPERASEG1_LOAD8(addr)   *(volatile unsigned char *)((addr))
#define CHIPERASET_SPI(idx,value) KCHIPERASEG1_STORE8(SPI_REG_BASE+idx, value)

#ifdef SPIBUG
#define GET_SPI(idx)	get_spi(idx)
#else
#define GET_SPI(idx)	KSEG1_LOAD8(SPI_REG_BASE + idx)
#endif

#define SET_SPI(idx,value) KSEG1_STORE8(SPI_REG_BASE+idx, value)


#define FLASH_VERIFFY
#if 1
//extern int SPI_FLASH_INITD;
static int m25p128_match __P ((struct device *, void *, void *));
static void m25p128_attach __P ((struct device *, struct device *, void *));
void m25p128_init (int , int);
struct m25p128_softc
{
    struct device sc_dev;
    int temp;
};
struct cfattach spi_flash_ca = {
    sizeof (struct m25p128_softc), m25p128_match, m25p128_attach
};

struct cfdriver spi_flash_cd = {
    NULL, "spi_flash", DV_DULL
};
    static int
m25p128_match (parent, match, aux)
    struct device *parent;
#if defined(__BROKEN_INDIRECT_CONFIG) || defined(__OpenBSD__)
    void *match;
#else
    struct cfdata *match;
#endif
    void *aux;
{
    DEBUG_PRINTF ("I am m25p128\n");
    return (1);
}

#endif

    void
gpio_cs_init (void)
{

#ifdef LS3B
#define CHIPERASELECOFFCHIPERASET  0x4
  volatile unsigned char * base = SPI_REG_BASE + CHIPERASELECOFFCHIPERASET;
  unsigned char  val = 0;

  val = *(base);
  val &= 0xfe; // are you sure? ,set bit[0] = 1b'0
  *(base) = val;

  // set off[5] bit 0 to be one to use bit[4] as cs bit
  val = *(base+1);
  val |= 0x1;
  *(base+1)=val;

#else
    //set gpio 0 output for godson3a, below for LS3A
    *(volatile unsigned char *) (GPIO_CTRL_REG) =  (~GPIO_CS_BIT) & (*(volatile unsigned char *) (GPIO_CTRL_REG));
	
#endif
}

//set gpio output value for godson3a
    inline void
set_cs (int bit)
{

#ifdef LS3B
#define CSCTLOFFCHIPERASET  0x5
  volatile unsigned char * base = SPI_REG_BASE + CSCTLOFFCHIPERASET;
  unsigned char  val = 0;

  val = *(base);
  if (!bit)
    val &= 0xef; // are you sure? ,set bit[0] = 1b'0
  else
    val |= 0x10; // are you sure? ,set bit[0] = 1b'1

  *(base) = val;
#else
  volatile unsigned char * base = GPIO_DATA_REG;
  unsigned char  val = 0;

  val = *(base);

  if (!bit)
	  val &= (~GPIO_CS_BIT); // are you sure? ,set bit[0] = 1b'0
       // *(volatile unsigned char *) (0x1fe0011c) = 0x4;
  else
	  val |= GPIO_CS_BIT; // are you sure? ,set bit[0] = 1b'1
       // *(volatile unsigned char *) (0x1fe0011c) = 0;

  *(base) = val;
#endif

}

    static void
m25p128_attach (struct device *parent, struct device *self, void *aux)
{
    gpio_cs_init ();
    delay(1);
    set_cs (1);
    //m25p128_init (3, 2);
}
    static unsigned char
get_spi (int idx)
{
    KCHIPERASEG1_LOAD8 (SPI_REG_BASE + idx);
    return KCHIPERASEG1_LOAD8 (SPI_REG_BASE);
}

static unsigned char m25p128_buf[512 * 1020]
__attribute__ ((section ("data"), aligned (512)));
static unsigned char tbuf[256]
__attribute__ ((section ("data")));

    static inline unsigned char
flash_writeb_cmd (unsigned char value)
{
    unsigned char ret;

    CHIPERASET_SPI (FCR_SPDR, value);
    while (GET_SPI (FCR_SPSR) & 0x01)
    {
        //DEBUG_PRINTF("flash_writeb_cmd while loop\n");
    }

    ret = GET_SPI (FCR_SPDR);
    DEBUG_PRINTF("writeb_cmd: write into %08x, return %08x\n", value, ret);
    return ret;
}
    static inline unsigned char
flash_read_data (void)
{
    unsigned char ret;

    CHIPERASET_SPI (FCR_SPDR, 0x00);
    while (GET_SPI (FCR_SPSR) & 0x01)
    {
        //DEBUG_PRINTF("flash_writeb_cmd while loop\n");
    }

    ret = GET_SPI (FCR_SPDR);
    DEBUG_PRINTF("read_cmd: GET_SPI (FCR_SPDR) return %08x\n", ret);

    return ret;
    //return flash_writeb_cmd (0);
}

void
m25p128_init (int mode, int div)
{
    int spcr, sper, cpol, cpha, spr, spre;
//    DEBUG_PRINTF ("Enter m25p128_init(), mode = %d, div = %d\n", mode, div);

    CHIPERASET_SPI (FCR_SPCR, 0x10);	//reset default value
    CHIPERASET_SPI (FCR_SPSR, 0xc0);	// clear sr
    //DEBUG_PRINTF("%s: %d\n", __FILE__, __LINE__);
    switch (div)
    {
        case 2:
            spr = spre = 0;
            break;
        case 4:
            spre = 0;
            spr = 1;
            break;
        case 16:
            spre = 0;
            spr = 2;
            break;
        case 32:
            spre = 0;
            spr = 3;
            break;
        case 8:
            spre = 1;
            spr = 0;
            break;
        case 64:
            spre = 1;
            spr = 1;
            break;
        case 128:
            spre = 1;
            spr = 2;
            break;
        case 256:
            spre = 1;
            spr = 3;
            break;
        case 512:
            spre = 2;
            spr = 0;
            break;
        case 1024:
            spre = 2;
            spr = 1;
            break;
        case 2048:
            spre = 2;
            spr = 2;
            break;
        case 4096:
            spre = 2;
            spr = 3;
            break;
        default:
            spr = spre = 0;
            break;
    }
    switch (mode)
    {
        case 0:
            cpol = cpha = 0;
            break;
        case 1:
            cpol = 0;
            cpha = 1;
            break;
        case 2:
            cpol = 1;
            cpha = 0;
            break;
        case 3:
            cpol = cpha = 1;
            break;
        default:
            cpol = cpha = 0;
            break;
    }
    spcr = SPE | MSTR | (cpha << CPHA_SHIFT) | (cpol << CPOL_SHIFT) | (spr << SPR_SHIFT);
    sper = spre;
    CHIPERASET_SPI (FCR_SPER, sper);
    CHIPERASET_SPI (FCR_SPCR, spcr);	//00
    //dummy read to align read/write pointer
    GET_SPI (FCR_SPDR);
    GET_SPI (FCR_SPDR);
    GET_SPI (FCR_SPDR);

    m25p128_readid ();
}

void spi_init0()
{ 

  DEBUG_PRINTF("Enter spi_init0()\n");
  SET_SPI(FCR_SPCR, 0x10);  // disable spi model
  SET_SPI(FCR_SPSR, 0xc0);
  DEBUG_PRINTF("%s: %d\n", __FILE__, __LINE__);

  //SET_SPI(0x4, 0x00); // ?
//  gpio_cs_init();

  //DEBUG_PRINTF("%s: %d\n", __FILE__, __LINE__);
//  SET_SPI(FCR_SPER, 0x04);
  //SET_SPI(FCR_SPER, 0x04);
  //SET_SPI(FCR_SPER, 0x02);
  SET_SPI(FCR_SPER, 0x00);
  SET_SPI(FCR_PARAM, 0x60);
  //SET_SPI(FCR_TIME, 0x02); // 
  DEBUG_PRINTF("%s: %d\n", __FILE__, __LINE__);

  //SET_SPI(0x5, 0x01);		 // ?	//softcs
 // set_cs(1);

  //DEBUG_PRINTF("%s: %d\n", __FILE__, __LINE__);
  SET_SPI(FCR_SPCR, 0x51);  //00, id ok
  //SET_SPI(FCR_SPCR, 0x50);  //00master mode
  //SET_SPI(FCR_SPCR, 0x55);  //01
  //SET_SPI(FCR_SPCR, 0x59);  //10, id error
  //SET_SPI(FCR_SPCR, 0x5e);  //10
  //SET_SPI(FCR_SPCR, 0x5d); //11
  DEBUG_PRINTF("%s: %d\n", __FILE__, __LINE__);
  
}


void m25p128_readid (void)
{

#define MACID_ADDR     0x00
#define DEVID_ADDR     0x01
#define VOID_ADDR      0x00
#define VOID_CMD       0x00

    unsigned char vendor_id;
    unsigned char device_id;
    unsigned char mem_type;
    unsigned char mem_size;
    int i;

	gpio_cs_init();
	spi_init0();

    set_cs (0);
  DEBUG_PRINTF("%s: %d\n", __FILE__, __LINE__);
    flash_writeb_cmd (RDID);
  DEBUG_PRINTF("%s: %d\n", __FILE__, __LINE__);
    flash_writeb_cmd (VOID_ADDR);
  DEBUG_PRINTF("%s: %d\n", __FILE__, __LINE__);
    flash_writeb_cmd (VOID_ADDR);
  DEBUG_PRINTF("%s: %d\n", __FILE__, __LINE__);
    flash_writeb_cmd (MACID_ADDR);
  DEBUG_PRINTF("%s: %d\n", __FILE__, __LINE__);
    vendor_id = flash_read_data ();
    set_cs (1);
    //  for(i=0; i<20; i++)

    set_cs (0);
     flash_writeb_cmd (RDID);
    flash_writeb_cmd (VOID_ADDR);
    flash_writeb_cmd (VOID_ADDR);
    flash_writeb_cmd (DEVID_ADDR);
    device_id = flash_read_data();
    //    DEBUG_PRINTF("id[%d] = 0x%x\n", i, flash_read_data ());

   printf("the vendor_id=0x%x\t,the device_id=0x%x\n", vendor_id, device_id);

	set_cs (1);

}
////////////////////////////////////////////////////////////////////////////////
// for ERASE

//busy wait for 1ms
#define SPI_FREE  0x00
#define SPI_BUSY  0x01

int spi_wait (void)
{
    int timeout = 10000;
    int ret = 1;
    int i;


	delay(100);
    while ((ret & 1))	//wait WIP
    {
		set_cs (0);
	    flash_writeb_cmd (RDSR);
		 ret = flash_read_data (); // Are you sure bit0 means busy or FREE?
//		  DEBUG_PRINTF("%s: %d: RDSR = 0x%x\n",__FUNCTION__, __LINE__, ret);
	    set_cs (1);
		delay(10);
    }

//#ifdef DEBUG
#if 0
	DEBUG_PRINTF("%s: %d: RDSR = 0x%x\n",__FUNCTION__, __LINE__, ret);
#endif
    return ((ret&1) == 1 ? SPI_BUSY: SPI_FREE) ;
//	return;
}

int m25p128_busy_wait_1ms(void)
{
    int timeout = 10000;
    int ret = 0;
    int i;

    set_cs (0);
    flash_writeb_cmd (RDSR);
    ret = flash_read_data (); // Are you sure bit0 means busy or FREE?
    set_cs (1);

    //while ((ret & 1) && --timeout)	//wait WIP
    while ((ret & 1))	//wait WIP
    {
        //delay(1);
        ret = flash_read_data ();
#ifdef DEBUG
        DEBUG_PRINTF("%s: %d: RDSR = 0x%x\n",__FUNCTION__, __LINE__, ret);
#endif
    }
//#ifdef DEBUG
#if 1
	DEBUG_PRINTF("%s: %d: RDSR = 0x%x\n",__FUNCTION__, __LINE__, ret);
#endif
    //return timeout ;
    return ((ret&1) == 1 ? SPI_BUSY: SPI_FREE) ;
}


int m25p128_slow_read_noecc (int offset, void *buffer, size_t n)
{
    unsigned int pos = offset;
    unsigned char *buf = (unsigned char *) buffer;
    int i;

#if 0
    if(!SPI_FLASH_INITD)
        return 0;
#endif
    delay(1);
    set_cs (0);
    flash_writeb_cmd (READ);
    flash_writeb_cmd ((pos >> 16) & 0xff);
    flash_writeb_cmd ((pos >> 8) & 0xff);
    flash_writeb_cmd (pos & 0xff);
    for (i = 0; i < n; i++) // Read out many datas while set address once?
        buf[i] = flash_read_data (); // I'm not sure
    set_cs (1);

    //dotik (3000, 0); // why dotik? what means?
    return (n);

}

int m25p128_aai_write(int offset, void *buffer, size_t n)
{
    unsigned int pos = offset;
    //unsigned short *buf = (unsigned short *) buffer;
    unsigned char *buf = (unsigned char *) buffer;
    int i,ret;
	unsigned char val_back;


    set_cs (0);
    flash_writeb_cmd (WREN);
    set_cs (1);
    delay(1);

#if 0
    set_cs (0);
    flash_writeb_cmd(EBSY);
    set_cs (1);
    delay(1);
#endif

	set_cs (0);
    flash_writeb_cmd(AAI);
    flash_writeb_cmd ((pos >> 16) & 0xff);
    flash_writeb_cmd ((pos & 0x00ff00 )>> 8);
    flash_writeb_cmd (pos & 0xff);
    //flash_writeb_cmd ((buf[0])&0xff);
    //flash_writeb_cmd (((buf[0])>>8)&0xff);
    flash_writeb_cmd (buf[0]);
    flash_writeb_cmd (buf[1]);
	set_cs (1);

	//ret = GET_SPI()
    //ret = GET_SPI (FCR_SPDR);
	ret = spi_wait ();

#if 1
	//for(i = 2; i < n && ret == SPI_FREE; i+=2)
	for(i = 2; i < n ; i+=2)
	{
	//	set_cs (0);
	//	delay(10);
	//	set_cs (1);
	    set_cs (0);
	    flash_writeb_cmd (WREN);
		set_cs (1);
	    delay(10);

		set_cs (0);
		flash_writeb_cmd(AAI);
		flash_writeb_cmd (buf[i]);
	    flash_writeb_cmd (buf[i+1]);
		set_cs (1);
		ret = spi_wait();
}
#endif
		set_cs (0);
		flash_writeb_cmd(WRDI);
		set_cs (1);

	    flash_writeb_cmd (RDSR);
		ret = flash_read_data (); // Are you sure bit0 means busy or FREE?
		DEBUG_PRINTF("After AAI/WRDI, RDSR = 0x%x\n", ret);

		/* fast read out one byte and to check it */
		pos = offset;
		set_cs (0);
        flash_writeb_cmd(FAST_READ); //To enable Program One Byte 
        flash_writeb_cmd((pos >> 16) & 0xff);
        flash_writeb_cmd((pos >> 8) & 0xff);
        flash_writeb_cmd(pos & 0xff);
        flash_writeb_cmd (VOID_CMD);
        //ret = flash_read_cmd (VOID_CMD);
		//ret = flash_read_data ();
		for ( i = 0;  i < n; i++, pos++)
		{
			val_back  = flash_read_data ();

			if(buf[i] != val_back)
				DEBUG_PRINTF("error @ buf[%d] ==  %08x, sould be %08x\n", i, val_back, buf[i]);
	  		else
				if ( i % 0x800 == 0)
				  DEBUG_PRINTF("PASS: %08x\n", i);
		}
        set_cs (1);

	DEBUG_PRINTF("\nAAI Write %08x bytes to SPI flash done, should write %08x bytes!\n",  i, n);

	return 0;
}


    int
m25p128_bulk_erase (void) // erase all chips
{
    int i, j, ret;
    unsigned char * buf;

#if 0
    if(!SPI_FLASH_INITD)
        return 0;
#endif

	gpio_cs_init();
	spi_init0();

	//gpio_cs_init();
    //set_cs (0);
	//spi_init0();

    ret = 0;
    DEBUG_PRINTF("bulk erase\n");

    set_cs (0);
    flash_writeb_cmd (EWSR);
    set_cs (1);
    delay(1);


    set_cs (0);
    flash_writeb_cmd (WRSR);
    flash_writeb_cmd (0);
    set_cs (1);
    delay(1);

    set_cs (0);
    flash_writeb_cmd (WREN);
    set_cs (1);
    delay(1);

    set_cs (0);
    flash_writeb_cmd (CHIPERASE);
//    flash_writeb_cmd (VOID_ADDR);
 //   flash_writeb_cmd (VOID_ADDR);
  //  flash_writeb_cmd (VOID_ADDR);
    set_cs (1);

    delay(1200);
    //wait for 400s, why? by xqch
    //for (j = 0; j < 400000; j++)
    //for (j = 0; j < 400; j++)
    for (j = 0; j < 1; j++)
    {
        ret = spi_wait ();
        //dotik (2560, 0);
        if (!ret)
            break;
    }
    if(ret)
    {
        DEBUG_PRINTF("erase timeout\n");
        return ret;
    }
    DEBUG_PRINTF("Erase over!\n");
#ifdef FLASH_VERIFFY
    for(i = 0; i < M25P128_SIZE/512; i++)
    {
        //m25p128_slow_read_noecc(i*512, m25p128_buf, 512);
        m25p128_fast_read_noecc(i*512, m25p128_buf, 512);
        buf = (unsigned char *)m25p128_buf;
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



////////////////////////////////////////////////////////////////////////////////
// m25p128_programm
//////////////////////////
    static int
m25p128_fast_read_noecc (int offset, void *buffer, size_t n)
{
    unsigned int pos = offset;
    unsigned char *buf = (unsigned char *) buffer;
    int i;

#if 0
    if(!SPI_FLASH_INITD)
        return 0;
#endif
    delay(1);
    set_cs (0);
    flash_writeb_cmd (FAST_READ);
    flash_writeb_cmd ((pos >> 16) & 0xff);
    flash_writeb_cmd ((pos >> 8) & 0xff);
    flash_writeb_cmd (pos & 0xff);
    flash_writeb_cmd (0);  //dummy read

    for (i = 0; i < n; i++)
        buf[i] = flash_read_data ();
    set_cs (1);

//    dotik (3000, 0);
    return (n);

}

int m25p128_read0(int offset, void *buffer, size_t n)
{
    if(offset >= ECC_BACHIPERASE || (offset + n) >= ECC_BACHIPERASE)
    {
        DEBUG_PRINTF("flash size overflow\n");
        return 0;
    }
//    return m25p128_slow_read_noecc (offset, buffer, n);
    return m25p128_fast_read_noecc (offset, buffer, n);
//    return m25p128_slow_read_ecc (offset, buffer, n);
 //   return m25p128_fast_read_ecc (offset, buffer, n);
}
/////////////////////////////////////////////////////////////
int
m25p128_byte_write(int offset, void *buffer, size_t n)
{
    unsigned int pos = offset;
    unsigned char *buf = (unsigned char *) buffer;
	unsigned char val_back;
    int  j, ret;
	int i = 0;

#if 0
    if(!SPI_FLASH_INITD)
        return 0;
#endif
    ret = 0;
    DEBUG_PRINTF("Enter programm %08x bytes\n", n);

#if 0
    set_cs (0);
    flash_writeb_cmd (EWSR);
    set_cs (1);
    delay(1);


    set_cs (0);
    flash_writeb_cmd (WRSR);
    flash_writeb_cmd (0);
    set_cs (1);
    delay(1);
#endif


	for ( i = 0, pos = offset; i < n; i++, pos++)
    {
       // delay(1);
    set_cs (0);
    flash_writeb_cmd (WREN);
    set_cs (1);
    delay(1);

		/* byte programm one byte */
        set_cs (0);
        flash_writeb_cmd(PP); //To enable Program One Byte 
        flash_writeb_cmd((pos >> 16) & 0xff);
        flash_writeb_cmd((pos >> 8) & 0xff);
        flash_writeb_cmd(pos & 0xff);
        flash_writeb_cmd (buf[i]);
        set_cs (1);
        spi_wait ();
	}

		///* slow read out one byte */
		/* fast read out one byte */
		pos = offset;
		set_cs (0);
        flash_writeb_cmd(FAST_READ); //To enable Program One Byte 
        flash_writeb_cmd((pos >> 16) & 0xff);
        flash_writeb_cmd((pos >> 8) & 0xff);
        flash_writeb_cmd(pos & 0xff);
        flash_writeb_cmd (VOID_CMD);
        //ret = flash_read_cmd (VOID_CMD);
		//ret = flash_read_data ();
		for ( i = 0;  i < n; i++, pos++)
		{
			val_back  = flash_read_data ();

			if(buf[i] != val_back)
				DEBUG_PRINTF("error @ buf[%d] ==  %08x, sould be %08x\n", i, val_back, buf[i]);
	  		else
				if ( i % 0x800 == 0)
				  DEBUG_PRINTF("PASS: %08x\n", i);
		}
        set_cs (1);

	return 1;
}
/////////////////////////////////////////////////////////////


int
m25p128_write_noecc (int offset, void *buffer, size_t n)
{
    unsigned int pos = offset;
    unsigned int col = pos & (M25P128_PAGE_SIZE - 1);
    unsigned char *buf = (unsigned char *) buffer;
    int left = n;
    int i, j, ret, small;

#if 0
    if(!SPI_FLASH_INITD)
        return 0;
#endif
    ret = 0;

    while (left)
    {
        small = mmin (left, (M25P128_PAGE_SIZE-col));

        set_cs (0);
        flash_writeb_cmd (WREN);
        set_cs (1);
        delay(1);
        set_cs (0);
        flash_writeb_cmd (PP); //To enable Program One Byte 
        flash_writeb_cmd ((pos >> 16) & 0xff);
        flash_writeb_cmd ((pos >> 8) & 0xff);
        flash_writeb_cmd (pos & 0xff);
        for (i = 0; i < small; i++)
            flash_writeb_cmd (buf[i]);
        set_cs (1);
        delay(1);
        dotik (3000, 0);
        //wait for 20ms
        for (j = 0; j < 20; j++)
        {
            ret = spi_wait ();
            if (!ret)
                break;
        }
        if (ret)
        {
            DEBUG_PRINTF("write timeout\n");
            return (n - left);
        }
        m25p128_slow_read_noecc(pos, m25p128_buf, small);
        for(j = 0; j < small; j++)
        {
            if(buf[j] != m25p128_buf[j])
            {
                DEBUG_PRINTF("write verify error at :%x, s:%x, t:%x\n", pos+j, buf[j], m25p128_buf[j]);
                return (n - left);
            }
        }
        left -= small;
        pos += small;
        buf += small;
        col = 0;
    }
    return (n);
}


int m25p128_programm(unsigned long len) // erase all chips
{
    int i, j, ret;
    unsigned char * buf;
	char tmp;

//	unsigned int offset = 0;
	unsigned int offset = 0x0000;
	unsigned int base = 0xbfc00000;

#if 0
    if(!SPI_FLASH_INITD)
        return 0;
#endif

	// 1. erase full spi flash
	gpio_cs_init();
	spi_init0();

    ret = 0;
    DEBUG_PRINTF("bulk erase\n");

    set_cs (0);
    flash_writeb_cmd (EWSR);
    set_cs (1);
    delay(1);


    set_cs (0);
    flash_writeb_cmd (WRSR);
    flash_writeb_cmd (0);
    set_cs (1);
    delay(1);

    set_cs (0);
    flash_writeb_cmd (WREN);
    set_cs (1);
    delay(1);


    set_cs (0);
    flash_writeb_cmd (CHIPERASE);
    set_cs (1);
	ret = spi_wait ();

//#endif

	// Now begin to Programm
	//unsigned short * buf = bios;
    DEBUG_PRINTF("Begin programm ..... \n");

	//m25p128_aai_write(0, 0xbfc00000, 0x40000);
	//m25p128_aai_write(0, 0xbfc00000, 0x80000);
#ifdef FAST_PROGRAMM
	m25p128_aai_write(offset, base, len > PMON_SIZE? PMON_SIZE: len);
#else
	m25p128_byte_write(offset, base, len > PMON_SIZE? PMON_SIZE: len);
#endif
	//m25p128_write_noecc(0,0xbfc00000, 0x80000); 
    DEBUG_PRINTF("\nProgramm over!\n");

#if 0
//#define FLASH_VERIFY
//#ifdef FLASH_VERIFFY
#if	0
    for(i = 0; i < PMON_SIZE/512; i++)
    {
        m25p128_fast_read_noecc(i*512, m25p128_buf, 512);
        buf = m25p128_buf;
        for(j = 0; j < 512; j++)
		{

			tmp = *((unsigned char*)(0xbfc00000 + i*512 + j));
            if(buf[j] != tmp)
            {
                DEBUG_PRINTF("Programm err: buf[%x] = %08x, should be %08x\n", i*512+j, buf[j], tmp);
                //return 1;
            }
			else
			{
				if (i % 0x80 == 0 && j == 0)
				{
					DEBUG_PRINTF("buf[%d*512+%d] = %08x",i,j, buf[j]);
				}
			}
		}
    }

#else
        m25p128_fast_read_noecc(offset, m25p128_buf, (len > PMON_SIZE) ? PMON_SIZE: len);
        buf = m25p128_buf;
	    for(j = 0; j < ((len > PMON_SIZE) ? PMON_SIZE: len); j++)
		{

			tmp = *((unsigned char*)(0xbfc00000 +  j));
            if(buf[j] != tmp)
            {
                DEBUG_PRINTF("Programm err: buf[%x] = %08x, should be %08x\n", j, buf[j], tmp);
                //return 1;
            }
			else
			{
//				if (j % 0x40 == 0 )
//				{
					DEBUG_PRINTF("PASS: buf[%d] = %08x\n",j, buf[j]);
//				}
			}
		}
	
#endif
    DEBUG_PRINTF("\nverify over\n");
#endif
    return 0;
}

int m25p128_read(unsigned offset)
{

  
  
  
}
    static int
mmin (int a, int b)
{
    return a < b ? a : b;
}


#if 0

int m25p128_write_ecc (int offset, void *buffer, size_t n)
{
    unsigned int pos = offset;
    unsigned char *buf = (unsigned char *) buffer;
    unsigned char *mbuf;
    int left = n;
    int i, small;
    unsigned char calcEcc[3];

    DEBUG_PRINTF ("write ===pos: %d\n", pos);
    if(offset % 512 != 0)
    {
        DEBUG_PRINTF ("addr must align 512\n");
        return 0;
    }
    while (left)
    {
        mbuf = buf;
        small = mmin (left, M25P128_PAGE_SIZE);

        if(small < M25P128_PAGE_SIZE)
        {
            for(i = 0; i < small; i++)
                tbuf[i] = buf[i];
            for(i = small; i < M25P128_PAGE_SIZE; i++)
                tbuf[i] = 0xff;
            mbuf = tbuf;
        }
        //flash_ECCCalculate(mbuf, calcEcc);
        m25p128_write_noecc(pos, mbuf, M25P128_PAGE_SIZE);
        m25p128_write_noecc(ECC_BACHIPERASE + pos / M25P128_PAGE_SIZE * 3, calcEcc, 3);
        left -= small;
        pos += small;
        buf += small;
    }
    return (n);
}



int m25p128_slow_read_ecc (int offset, void *buffer, size_t n)
{
    unsigned int pos = offset;
    unsigned char *buf = (unsigned char *) buffer;
    unsigned char *mbuf;
    int i, small, eccResult;
    size_t left = n;
    unsigned char calcEcc[3];
    unsigned char rEcc[3];
    int fixed = 0;
    int unfixed = 0;

    while(left)
    {
        mbuf = buf;
        small = mmin (left, M25P128_PAGE_SIZE);
        if(small < M25P128_PAGE_SIZE)
        {
            mbuf = tbuf;
        }
        m25p128_slow_read_noecc(pos, mbuf, M25P128_PAGE_SIZE);
//        //flash_ECCCalculate(mbuf, calcEcc);
        m25p128_slow_read_noecc(ECC_BACHIPERASE + pos / M25P128_PAGE_SIZE * 3, rEcc, 3);
//        eccResult = flash_ECCCorrect (mbuf, rEcc, calcEcc);
        if(eccResult > 0)
        {
            DEBUG_PRINTF("ecc error fix performed on chunk 0x%x\n", pos);
        }
        if(eccResult < 0)
        {
            DEBUG_PRINTF("ecc error unfixed on chunk 0x%x\n", pos);
            DEBUG_PRINTF("read again");
            if(++unfixed == 5)
                return 0;
            continue;
        }
        if(small < M25P128_PAGE_SIZE)
        {
            for(i = 0; i < small; i++)
                buf[i] = mbuf[i];
        }
        left -= small;
        pos += small;
        buf += small;

    }
    return (n);

}

int m25p128_fast_read_ecc (int offset, void *buffer, size_t n)
{
    unsigned int pos = offset;
    unsigned char *buf = (unsigned char *) buffer;
    unsigned char *mbuf;
    int i, small, eccResult;
    size_t left = n;
    unsigned char calcEcc[3];
    unsigned char rEcc[3];
    int fixed = 0;
    int unfixed = 0;

    while(left)
    {
        mbuf = buf;
        small = mmin (left, M25P128_PAGE_SIZE);
        if(small < M25P128_PAGE_SIZE)
        {
            mbuf = tbuf;
        }
        m25p128_fast_read_noecc(pos, mbuf, M25P128_PAGE_SIZE);
        //flash_ECCCalculate(mbuf, calcEcc);
        m25p128_fast_read_noecc(ECC_BACHIPERASE + pos / M25P128_PAGE_SIZE * 3, rEcc, 3);
        eccResult = flash_ECCCorrect (mbuf, rEcc, calcEcc);
        if(eccResult > 0)
        {
            DEBUG_PRINTF("ecc error fix performed on chunk 0x%x\n", pos);
            fixed++;
        }
        if(eccResult < 0)
        {
            DEBUG_PRINTF("ecc error unfixed on chunk 0x%x\n", pos);
            DEBUG_PRINTF("read again");
            if(++unfixed == 5)
                return 0;
            continue;
        }
        if(small < M25P128_PAGE_SIZE)
        {
            for(i = 0; i < small; i++)
                buf[i] = mbuf[i];
        }
        left -= small;
        pos += small;
        buf += small;

    }
    return (n);

}
int m25p128_write0 (int offset, void *buffer, size_t n)
{
    //m25p128_write_noecc(offset, buffer, n);
    m25p128_write_ecc(offset, buffer, n);
}

int
m25p128_sector_erase (int offset, size_t n)
{
    unsigned int pos = offset & (~(M25P128_PAGE_SIZE - 1));
    int left = ((offset & (M25P128_PAGE_SIZE - 1)) + n + M25P128_PAGE_SIZE - 1) / M25P128_PAGE_SIZE;
    int j, ret;
    unsigned int * buf;

#if 0
    if(!SPI_FLASH_INITD)
        return 1;
#endif
    ret = 0;
    DEBUG_PRINTF("erase addr: %x, %d pages\n", pos, left);

    while (left)
    {
        set_cs (0);
        flash_writeb_cmd (WREN);
        set_cs (1);
        delay(1);
        set_cs (0);
        flash_writeb_cmd (4KBE);
        flash_writeb_cmd ((pos >> 16) & 0xff);
        flash_writeb_cmd ((pos >> 8) & 0xff);
        flash_writeb_cmd (pos & 0xff);
        set_cs (1);
        delay(1);
        //wait for 20s
        for (j = 0; j < 20000; j++)
        {
            ret = spi_wait ();
            if (!ret)
                break;
        }
        if (ret)
        {
            DEBUG_PRINTF("erase timeout\n");
            return 1;
        }
        DEBUG_PRINTF("erase sector 0x%x over, now verify\n", pos);
        m25p128_slow_read_noecc(pos, m25p128_buf, M25P128_PAGE_SIZE);
        buf = (unsigned int *)m25p128_buf;
        for(j = 0; j < M25P128_PAGE_SIZE/4; j++)
        {
            if(0xffffffff != buf[j])
            {
                DEBUG_PRINTF("erase verify error at %x\n", pos+4*j);
                return 1;
            }
        }
        left--;
        pos += M25P128_PAGE_SIZE;
    }
    DEBUG_PRINTF("verify over\n");
    return 0;
}

    void
m25p128_strategy (struct buf *bp)
{
    unsigned int dev, blkno, blkcnt;
    unsigned int d_secsize;
    int ret;
    char *buf;

    dev = bp->b_dev;
    blkno = bp->b_blkno;		//offset

    buf = bp->b_data;		//buf
    blkcnt = bp->b_bcount;	//len

    d_secsize = 512;


    //      blkcnt = howmany(bp->b_bcount, d_secsize); 


    /* Valid request?  */
    if (bp->b_blkno < 0 ||
            (bp->b_bcount % d_secsize) != 0 ||
            (bp->b_bcount / d_secsize) >= (1 << NBBY))
    {				//?
        bp->b_error = EINVAL;
        DEBUG_PRINTF ("Invalid request \n");
        goto bad;
    }

    /* If it's a null transfer, return immediately. */
    if (bp->b_bcount == 0)
        goto done;


    if (bp->b_flags & B_READ)
    {
        if ((unsigned long) bp->b_data & (d_secsize - 1))
        {
            ret =
                m25p128_read0 (blkno * 512, (unsigned long *) m25p128_buf,
                        blkcnt);
            memcpy (bp->b_data, m25p128_buf, bp->b_bcount);
        }
        else
            ret =
                m25p128_read0 (blkno * 512, (unsigned long *) bp->b_data, blkcnt);
        if (ret != blkcnt)
            bp->b_flags |= B_ERROR;
    }

done:
    biodone (bp);
    return;
bad:
    bp->b_flags |= B_ERROR;
    biodone (bp);
}

    int
m25p128_open (dev_t dev, int flag, int fmt, struct proc *p)
{
    return 0;
}

    int
m25p128_close (dev_t dev, int flag, int fmt, struct proc *p)
{
    return 0;
}

    int
m25p128_read (dev_t dev, struct uio *uio, int ioflag)
{
    return physio (m25p128_strategy, NULL, dev, B_READ, NULL, uio);
}

    int
m25p128_write (dev_t dev, struct uio *uio, int ioflag)
{
    return -EINVAL;
}

cmd_write()
{
}

#endif


int
cmd_getid(ac, av)
	int ac;
	char *av[];
{
	DEBUG_PRINTF("start wspi\n");
	m25p128_readid();
	DEBUG_PRINTF("wspi done\n");
	return(1);
}

cmd_erase(ac, av)
	int ac;
	char *av[];
{
	printf("erase full spi flash chip\n");
	//m25p128_readid();
	m25p128_bulk_erase();
	printf("Full Chip Erase done\n");
	return(1);
}

cmd_programm(ac, av)
	int ac;
	char *av[];
{
	unsigned long length;

	printf("erase full spi flash chip\n");
	//m25p128_readid();
	if ( ac > 1)
	{
		DEBUG_PRINTF("argv[1] is %s\n ", av[1]);
		length=strtoul(av[1],0,0);
	}
	else
	{
		length=0x80000; //512k 
	}
	m25p128_programm(length);
	printf("Full Chip Programm done\n");
	return(1);
}

int cmd_read(argc, argv)
	int argc;
	char *argv[];
{

	unsigned char buf;
	unsigned long offset;

	DEBUG_PRINTF("Read out a value from spi flash\n");
	DEBUG_PRINTF("argv[1] is %s\n ", argv[1]);

	offset=strtoul(argv[1],0,0);
	m25p128_fast_read_noecc (offset, &buf, 1);
	printf("Offset: value -- %08llp: %08x\n",offset, (unsigned short)buf);

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
	{"spierase",	"", 0, "Erase spi flash full chip ", cmd_erase, 1, 99, 0},
	{"spiprogramm",	"", 0, "Programm spi flash with LPC Flash", cmd_programm, 1, 99, 0},
	{"spiread",	"", 0, "Programm spi flash with LPC Flash", cmd_read, 1, 99, 0},
	{0, 0}
};

static void init_cmd __P((void)) __attribute__ ((constructor));

void
init_cmd()
{
	cmdlist_expand(Cmds, 1);
}

