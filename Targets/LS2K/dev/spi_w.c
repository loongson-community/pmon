#include <stdio.h>
//#include "include/fcr.h"
#include <stdlib.h>
#include <ctype.h>
#undef _KERNEL
#include <errno.h>
#include <pmon.h>
//#include <include/types.h>
#include <pflash.h>
#include <linux/spi.h>
#include <linux/bitops.h>
#include "spinand_mt29f.h"
#include "spinand_lld.h"
#include "m25p80.h"

#define SPI_BASE  0x1fff0220
#define PMON_ADDR 0xa1000000
#define FLASH_ADDR 0x000000

#define SPCR      0x0
#define SPSR      0x1
#define FIFO	  0x2
#define TXFIFO    0x2
#define RXFIFO    0x2
#define SPER      0x3
#define PARAM     0x4
#define SOFTCS    0x5
#define PARAM2    0x6

#define RFEMPTY 1
#define KSEG1_STORE8(addr,val)	 *(volatile char *)(0xffffffffa0000000 | addr) = val
#define KSEG1_LOAD8(addr)	 *(volatile char *)(0xffffffffa0000000 | addr) 

#define SET_SPI(addr,val)        KSEG1_STORE8(SPI_BASE+addr,val)
#define GET_SPI(addr)            KSEG1_LOAD8(SPI_BASE+addr)


int write_sr(char val);
void spi_initw()
{ 
	int d;
  	SET_SPI(SPSR, 0xc0); 
  	SET_SPI(PARAM, 0x40);             //espr:0100
  	SET_SPI(PARAM2,0x01); 
#ifdef LS2H_SPI_HIGHSPEED
	d=1;
#else
	d=4;
#endif
 	SET_SPI(SPER, 0x00|((d>>2)&3)); //spre:01 
  	SET_SPI(SPCR, 0x50|(d&3));
	SET_SPI(SOFTCS,0xff);
}

void spi_initr()
{
  	SET_SPI(PARAM, 0x41);             //espr:0100
}




///////////////////read status reg /////////////////

int read_sr(void)
{
	int val;
	
	SET_SPI(SOFTCS,0x01|0xee);
	SET_SPI(TXFIFO,0x05);
	while((GET_SPI(SPSR))&RFEMPTY);

	val = GET_SPI(RXFIFO);
	SET_SPI(TXFIFO,0x00);

	while((GET_SPI(SPSR))&RFEMPTY == RFEMPTY);

	val = GET_SPI(RXFIFO);
	
	SET_SPI(SOFTCS,0x11|0xee);
      
	return val;
}

void wait_sr(void)
{
	int val;
	
	SET_SPI(SOFTCS,0x01|0xee);
	SET_SPI(TXFIFO,0x05);
	while((GET_SPI(SPSR))&RFEMPTY);

	val = GET_SPI(RXFIFO);
	do{
	SET_SPI(TXFIFO,0x00);

	while((GET_SPI(SPSR)&RFEMPTY) == RFEMPTY);

	val = GET_SPI(RXFIFO);
	} while(val & 1);
	
	SET_SPI(SOFTCS,0x11|0xee);
      
	return val;
}

////////////set write enable//////////
int set_wren(void)
{
	int res;
	
	res = read_sr();
	while(res&0x01 == 1)
	{
		res = read_sr();
	}
	
	SET_SPI(SOFTCS,0x01|0xee);
	
	SET_SPI(TXFIFO,0x6);
       	while((GET_SPI(SPSR))&RFEMPTY == RFEMPTY){
	}
	GET_SPI(RXFIFO);

	SET_SPI(SOFTCS,0x11|0xee);

	return 1;
}

///////////////////////write status reg///////////////////////
int write_sr(char val)
{
	int res;
	
	set_wren();
	
	res = read_sr();
	while(res&0x01 == 1)
	{
		res = read_sr();
	}
	
	SET_SPI(SOFTCS,0x01|0xee);

	SET_SPI(TXFIFO,0x01);
       	while((GET_SPI(SPSR))&RFEMPTY == RFEMPTY){
      			
	}
	GET_SPI(RXFIFO);

	SET_SPI(TXFIFO,val);
       	while((GET_SPI(SPSR))&RFEMPTY == RFEMPTY){
      			
	}
	GET_SPI(RXFIFO);
	SET_SPI(SOFTCS,0x11|0xee);
	
	return 1;
	
}

///////////erase all memory/////////////
int erase_all(void)
{
	int res;
	int i=1;
        spi_initw();
	set_wren();
	res = read_sr();
	while(res&0x01 == 1)
	{
		res = read_sr();
	}
	SET_SPI(SOFTCS,0x1|0xee);
	
	SET_SPI(TXFIFO,0xC7);
       	while((GET_SPI(SPSR))&RFEMPTY == RFEMPTY){
      			
	}
	GET_SPI(RXFIFO);
	
	SET_SPI(SOFTCS,0x11|0xee);
        while(i++){
            if(read_sr() & 0x1 == 0x1){
                if(i % 10000 == 0)
                    printf(".");
            }else{
                printf("done...\n");
                break;
            }   
        }
	return 1;
}



void spi_read_id(void)
{
    unsigned char val;
    spi_initw();
    val = read_sr();
    while(val&0x01 == 1)
    {
        val = read_sr();
    }
    /*CE 0*/
    SET_SPI(SOFTCS,0x01|0xee);
    /*READ ID CMD*/
    SET_SPI(TXFIFO,0x9f);
    while((GET_SPI(SPSR))&RFEMPTY == RFEMPTY){

    }
    GET_SPI(RXFIFO);

    /*Manufacturer’s ID*/
    SET_SPI(TXFIFO,0x00);
    while((GET_SPI(SPSR))&RFEMPTY == RFEMPTY){

    }
    val = GET_SPI(RXFIFO);
    printf("Manufacturer's ID:         %x\n",val);

    /*Device ID:Memory Type*/
    SET_SPI(TXFIFO,0x00);
    while((GET_SPI(SPSR))&RFEMPTY == RFEMPTY){

    }
    val = GET_SPI(RXFIFO);
    printf("Device ID-memory_type:     %x\n",val);
    
    /*Device ID:Memory Capacity*/
    SET_SPI(TXFIFO,0x00);
    while((GET_SPI(SPSR))&RFEMPTY == RFEMPTY){

    }
    val = GET_SPI(RXFIFO);
    printf("Device ID-memory_capacity: %x\n",val);
    
    /*CE 1*/
    SET_SPI(SOFTCS,0x11|0xee);

}

void spi_write_byte(unsigned int addr,unsigned char data)
{
    /*byte_program,CE 0, cmd 0x2,addr2,addr1,addr0,data in,CE 1*/
	unsigned char addr2,addr1,addr0;
        unsigned char val;
	addr2 = (addr & 0xff0000)>>16;
    	addr1 = (addr & 0x00ff00)>>8;
	addr0 = (addr & 0x0000ff);
        set_wren();
        val = read_sr();
        while(val&0x01 == 1)
        {
            val = read_sr();
        }
	SET_SPI(SOFTCS,0x01|0xee);/*CE 0*/

        SET_SPI(TXFIFO,0x2);/*byte_program */
        while((GET_SPI(SPSR))&RFEMPTY == RFEMPTY){

        }
        val = GET_SPI(RXFIFO);
        
        /*send addr2*/
        SET_SPI(TXFIFO,addr2);     
        while((GET_SPI(SPSR))&RFEMPTY == RFEMPTY){

        }
        val = GET_SPI(RXFIFO);
        
        /*send addr1*/
        SET_SPI(TXFIFO,addr1);
        while((GET_SPI(SPSR))&RFEMPTY == RFEMPTY){

        }
        val = GET_SPI(RXFIFO);
        
        /*send addr0*/
        SET_SPI(TXFIFO,addr0);
        while((GET_SPI(SPSR))&RFEMPTY == RFEMPTY){

        }
        val = GET_SPI(RXFIFO);

        /*send data(one byte)*/
       	SET_SPI(TXFIFO,data);
        while((GET_SPI(SPSR))&RFEMPTY == RFEMPTY){

        }
        val = GET_SPI(RXFIFO);
        
        /*CE 1*/
	SET_SPI(SOFTCS,0x11|0xee);

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
// read flash id command
        spi_read_id();
	val = GET_SPI(SPSR);
	printf("====spsr value:%x\n",val);
	
	SET_SPI(0x5,0x10);
// erase the flash     
	write_sr(0x00);
//	erase_all();
        printf("\nfrom ram 0x%08x  to flash 0x%08x size 0x%08x \n\nprogramming            ",ramaddr,flashaddr,size);
        for(j=0;size > 0;flashaddr++,ramaddr++,size--,j++)
        {
            spi_write_byte(flashaddr,*((unsigned char*)ramaddr));
            if(j % 0x1000 == 0)
                printf("\b\b\b\b\b\b\b\b\b\b0x%08x",j);
        }
        printf("\b\b\b\b\b\b\b\b\b\b0x%08x end...\n",j);

        SET_SPI(0x5,0x11);
	return 1;
}

int read_pmon_byte(unsigned int addr,unsigned int num)
{
        unsigned char val,data;
	val = read_sr();
	while(val&0x01 == 1)
	{
		val = read_sr();
	}
	
	SET_SPI(0x5,0x01);
// read flash command 
	SET_SPI(TXFIFO,0x03);
	while((GET_SPI(SPSR))&RFEMPTY == RFEMPTY){
      			
	}
	GET_SPI(RXFIFO);
	
// addr
	SET_SPI(TXFIFO,0x00);
	while((GET_SPI(SPSR))&RFEMPTY == RFEMPTY){
      			
	}
        GET_SPI(RXFIFO);
	
	SET_SPI(TXFIFO,0x00);
	while((GET_SPI(SPSR))&RFEMPTY == RFEMPTY){
      			
	}
	GET_SPI(RXFIFO);
	
	SET_SPI(TXFIFO,0x00);
	while((GET_SPI(SPSR))&RFEMPTY == RFEMPTY){
      			
	}
	GET_SPI(RXFIFO);
	
        
        SET_SPI(TXFIFO,0x00);
        while((GET_SPI(SPSR))&RFEMPTY == RFEMPTY){

        }
        data = GET_SPI(RXFIFO);
	SET_SPI(0x5,0x11);
        return data;
}

int read_pmon(int argc,char **argv)
{
	unsigned char addr2,addr1,addr0;
	unsigned char data;
	int val,base=0;
	int addr;
	int i;
        if(argc != 3)
        {
            printf("\nuse: read_pmon addr(flash) size\n");
            return -1;
        }
        addr = strtoul(argv[1],0,0);
        i = strtoul(argv[2],0,0);
	spi_initw();
	val = read_sr();
	while(val&0x01 == 1)
	{
		val = read_sr();
	}
	
	SET_SPI(0x5,0x01);
// read flash command 
	SET_SPI(TXFIFO,0x03);
	while((GET_SPI(SPSR))&RFEMPTY == RFEMPTY){
      			
	}
	GET_SPI(RXFIFO);
	
// addr
	SET_SPI(TXFIFO,((addr >> 16)&0xff));
	while((GET_SPI(SPSR))&RFEMPTY == RFEMPTY){
      			
	}
        GET_SPI(RXFIFO);
	
	SET_SPI(TXFIFO,((addr >> 8)&0xff));
	while((GET_SPI(SPSR))&RFEMPTY == RFEMPTY){
      			
	}
	GET_SPI(RXFIFO);
	
	SET_SPI(TXFIFO,(addr & 0xff));
	while((GET_SPI(SPSR))&RFEMPTY == RFEMPTY){
      			
	}
	GET_SPI(RXFIFO);
// addr end
	
        
        printf("\n");
        while(i--)
	{
		SET_SPI(TXFIFO,0x00);
		while((GET_SPI(SPSR))&RFEMPTY == RFEMPTY){
      			
		}
	        data = GET_SPI(RXFIFO);
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

	for(addr=saddr;addr<eaddr;addr+=sectorsize)
	{

	SET_SPI(SOFTCS,0x11|0xee);

	set_wren();

	write_sr(0x00);

	while(read_sr()&1);

	set_wren();

	SET_SPI(SOFTCS,0x01|0xee);

        /* 
         * 0x20 erase 4kbyte of memory array
         * 0x52 erase 32kbyte of memory array
         * 0xd8 erase 64kbyte of memory array                                                                                                           
         */
	SET_SPI(TXFIFO,0x20);
       	while((GET_SPI(SPSR))&RFEMPTY);
	GET_SPI(RXFIFO);
	SET_SPI(TXFIFO,addr >> 16);

       	while((GET_SPI(SPSR))&RFEMPTY);
	GET_SPI(RXFIFO);

	SET_SPI(TXFIFO,addr >> 8);
       	while((GET_SPI(SPSR))&RFEMPTY);
	GET_SPI(RXFIFO);

	SET_SPI(TXFIFO,addr);
	
       	while((GET_SPI(SPSR))&RFEMPTY);
	GET_SPI(RXFIFO);
	
	SET_SPI(SOFTCS,0x11|0xee);

	while(read_sr()&1);
	}
	SET_SPI(SOFTCS,0x11|0xee);
	delay(10);

	return 0;
}

int spi_write_bytes(unsigned int addr,unsigned char *buffer, int size)
{
	int i;
    /*byte_program,CE 0, cmd 0x2,addr2,addr1,addr0,data in,CE 1*/
	unsigned char addr2,addr1,addr0;
        unsigned char val;
	addr2 = (addr & 0xff0000)>>16;
    	addr1 = (addr & 0x00ff00)>>8;
	addr0 = (addr & 0x0000ff);
	size = (size>(256-addr0))?(256-addr0):size;
        set_wren();

	SET_SPI(SOFTCS,0x01|0xee);/*CE 0*/
	/*write 5 data to fifo, depth>4?*/

        SET_SPI(TXFIFO,0x2);/*byte_program */
        
        /*send addr2*/
        SET_SPI(TXFIFO,addr2);     
        /*send addr1*/
        SET_SPI(TXFIFO,addr1);
        /*send addr0*/
        SET_SPI(TXFIFO,addr0);

	for(i=0;i<4;i++)
	{
        while((GET_SPI(SPSR)&RFEMPTY) == RFEMPTY);
        val = GET_SPI(RXFIFO);
	}

	for(i=0;i<size;i++)
	{
        /*send data(one byte)*/
       	SET_SPI(TXFIFO,buffer[i]);


        while((GET_SPI(SPSR)&RFEMPTY) == RFEMPTY);
        val = GET_SPI(RXFIFO);
	}
        
        /*CE 1*/
	SET_SPI(SOFTCS,0x11|0xee);

        wait_sr();
	return size;	
}
#ifdef LS2H_SPI_WRITEBYTES
int spi_write_area(int flashaddr,char *buffer,int size)
{
	int cnt;
	spi_initw();
	//SET_SPI(SOFTCS,0x10|0xee);
	write_sr(0x00);
        for(;size > 0;)
        {
	///	if((flashaddr&0xfffff)==0)printf("%x\n",flashaddr);
            cnt = spi_write_bytes(flashaddr,buffer, size);
	    size -= cnt;
	    flashaddr += cnt;
	    buffer += cnt;
        }

	SET_SPI(SOFTCS,0x11|0xee);
	delay(10);
	return 0;
}
#else
int spi_write_area(int flashaddr,char *buffer,int size)
{
	int j;
	spi_initw();
	SET_SPI(0x5,0x10);
	write_sr(0x00);
        for(j=0;size > 0;flashaddr++,size--,j++)
        {
            spi_write_byte(flashaddr,buffer[j]);
	    dotik(32, 0);
        }

	SET_SPI(SOFTCS,0x11|0xee);
	delay(10);
	return 0;
}
#endif


int spi_read_area(int flashaddr,char *buffer,int size)
{
	int i;
	spi_initw();

	SET_SPI(SOFTCS,0x01|0xee);

	SET_SPI(TXFIFO,0x03);

        while((GET_SPI(SPSR))&RFEMPTY);
        GET_SPI(RXFIFO);
        
        SET_SPI(TXFIFO,flashaddr>>16);     
        while((GET_SPI(SPSR))&RFEMPTY);
        GET_SPI(RXFIFO);

        SET_SPI(TXFIFO,flashaddr>>8);     
        while((GET_SPI(SPSR))&RFEMPTY);
        GET_SPI(RXFIFO);

        SET_SPI(TXFIFO,flashaddr);     
        while((GET_SPI(SPSR))&RFEMPTY);
        GET_SPI(RXFIFO);
        

        for(i=0;i<size;i++)
        {
        SET_SPI(TXFIFO,0);     
        while((GET_SPI(SPSR))&RFEMPTY);
        buffer[i] = GET_SPI(RXFIFO);
        }

        SET_SPI(SOFTCS,0x11|0xee);
	delay(10);
	return 0;
}

struct fl_device myflash = {
	.fl_name="spiflash",
	.fl_size=0x100000,
	.fl_secsize=0x10000,
};

struct fl_device *fl_devident(void *base, struct fl_map **m)
{
	if(m)
	*m = fl_find_map(base);
	return &myflash;
}

int fl_program_device(void *fl_base, void *data_base, int data_size, int verbose)
{
	struct fl_map *map;
	int off;
	map = fl_find_map(fl_base);
	off = (int)(fl_base - map->fl_map_base) + map->fl_map_offset;
	spi_write_area(off,data_base,data_size);
	spi_initr();
	return 0;
}


int fl_erase_device(void *fl_base, int size, int verbose)
{
	struct fl_map *map;
	int off;
	map = fl_find_map(fl_base);
	off = (int)(fl_base - map->fl_map_base) + map->fl_map_offset;
	spi_erase_area(off,off+size,0x1000);
	spi_initr();
return 0;
}

//---------------------------------------
#define prefetch(x) (x)

static struct ls1x_spi {
	void	*base;
	int hz;
}  ls1x_spi0 = { 0xbfff0220} ;

struct spi_device spi_nand = 
{
.dev = &ls1x_spi0,
.chip_select = 1,
.max_speed_hz = 100000000,
}; 

struct spi_device spi_nand1 = 
{
.dev = &ls1x_spi0,
.chip_select = 0,
.max_speed_hz = 40000000,
}; 


static char ls1x_spi_write_reg(struct ls1x_spi *spi, 
				unsigned char reg, unsigned char data)
{
	(*(volatile unsigned char *)(spi->base +reg)) = data;
}

static char ls1x_spi_read_reg(struct ls1x_spi *spi, 
				unsigned char reg)
{
	return(*(volatile unsigned char *)(spi->base + reg));
}


static int 
ls1x_spi_write_read_8bit(struct spi_device *spi,
  const u8 **tx_buf, u8 **rx_buf, unsigned int num)
{
	struct ls1x_spi *ls1x_spi = spi->dev;
	unsigned char value;
	int i, ret;
	
	for(i = 0; i < 4 && i < num; i++) {
		if (tx_buf && *tx_buf)
			value = *((*tx_buf)++);
		else 
			value = 0;
		ls1x_spi_write_reg(ls1x_spi, FIFO, value);
	}

	ret = i;

	for(;i > 0; i--) {
 		while((ls1x_spi_read_reg(ls1x_spi, SPSR) & 0x1) == 1);
		value = ls1x_spi_read_reg(ls1x_spi, FIFO);
		if (rx_buf && *rx_buf) 
			*(*rx_buf)++ = value;
	}


	return ret;
}


static unsigned int
ls1x_spi_write_read(struct spi_device *spi, struct spi_transfer *xfer)
{
	struct ls1x_spi *ls1x_spi;
	unsigned int count;
	int ret;
	const u8 *tx = xfer->tx_buf;
	u8 *rx = xfer->rx_buf;

	ls1x_spi = spi->dev;
	count = xfer->len;

	do {
		if ((ret = ls1x_spi_write_read_8bit(spi, &tx, &rx, count)) < 0)
			goto out;
		count -= ret;
	} while (count);

out:
	return xfer->len - count;
	//return count;

}


#define DIV_ROUND_UP(n,d)       (((n) + (d) - 1) / (d))
static int ls_spi_setup(struct ls1x_spi *ls1x_spi,  struct spi_device *spi)
{
	unsigned int hz;
	unsigned int div, div_tmp;
	unsigned int bit;
	unsigned long clk;
	unsigned char val, spcr, sper;
	const char rdiv[12]= {0,1,4,2,3,5,6,7,8,9,10,11}; 

	hz  = spi->max_speed_hz;

	if (hz) {
		clk = 100000000;
		div = DIV_ROUND_UP(clk, hz);

		if (div < 2)
			div = 2;

		if (div > 4096)
			div = 4096;

		bit = fls(div) - 1;
		if((1<<bit) == div) bit--;
		div_tmp = rdiv[bit];
		ls1x_spi->hz = hz;
		spcr = div_tmp & 3;
		sper = (div_tmp >> 2) & 3;

		val = ls1x_spi_read_reg(ls1x_spi, SPCR);
		ls1x_spi_write_reg(ls1x_spi, SPCR, (val & ~3) | spcr);
		val = ls1x_spi_read_reg(ls1x_spi, SPER);
		ls1x_spi_write_reg(ls1x_spi, SPER, (val & ~3) | sper);

	}
	return 0;
}



int spi_sync(struct spi_device *spi, struct spi_message *m)
{

	struct ls1x_spi *ls1x_spi = &ls1x_spi0;
	struct spi_transfer *t = NULL;
	unsigned long flags;
	int cs;
	int param;

	ls_spi_setup(ls1x_spi, spi);
	
	m->actual_length = 0;
	m->status		 = 0;

	if (list_empty(&m->transfers) /*|| !m->complete*/)
		return -EINVAL;


	list_for_each_entry(t, &m->transfers, transfer_list) {
		
		if (t->tx_buf == NULL && t->rx_buf == NULL && t->len) {
			printf("message rejected : "
				"invalid transfer data buffers\n");
			goto msg_rejected;
		}

	/*other things not check*/

	}

	param = ls1x_spi_read_reg(ls1x_spi, PARAM);
	ls1x_spi_write_reg(ls1x_spi, PARAM, param&~1);

	cs = 0xff & ~(0x11<<spi->chip_select);
	ls1x_spi_write_reg(ls1x_spi, SOFTCS, (0x1 << spi->chip_select)|cs);

	list_for_each_entry(t, &m->transfers, transfer_list) {

		if (t->len)
			m->actual_length +=
				ls1x_spi_write_read(spi, t);
	}

	ls1x_spi_write_reg(ls1x_spi, SOFTCS, (0x11<<spi->chip_select)|cs);
	ls1x_spi_write_reg(ls1x_spi, PARAM, param);

	return 0;
msg_rejected:

	m->status = -EINVAL;
 	if (m->complete)
		m->complete(m->context);
	return -EINVAL;
}


#if NSPINAND_MT29F||NSPINAND_LLD
int spinand_probe(struct spi_device *spi_nand);

int ls2h_spi_nand_probe()
{
    spi_initw();

#ifdef CONFIG_SPINAND_CS
spi_nand.chip_select = CONFIG_SPINAND_CS;
#endif
spinand_probe(&spi_nand);
    spi_initr();
}
#endif

#if NM25P80
int ls2h_m25p_probe()
{
    spi_initw();
    m25p_probe(&spi_nand1, "gd25q80");
    m25p_probe(&spi_nand, "gd25q256");
    spi_initr();
}
#endif

//----------------------------------------

int qt_read_pmon(int addr, int size)
{
	unsigned char addr2,addr1,addr0;
	static unsigned char data;
	int val,base=0;
    int i = size;

    spi_initw();
	val = read_sr();
	while(val&0x01 == 1){
		val = read_sr();
	}

	SET_SPI(0x5,0x01);
// read flash command
	SET_SPI(TXFIFO,0x03);
	while((GET_SPI(SPSR))&RFEMPTY == RFEMPTY){

	}
	GET_SPI(RXFIFO);

// addr
	SET_SPI(TXFIFO,((addr >> 16)&0xff));
	while((GET_SPI(SPSR))&RFEMPTY == RFEMPTY){

	}
    GET_SPI(RXFIFO);

	SET_SPI(TXFIFO,((addr >> 8)&0xff));
	while((GET_SPI(SPSR))&RFEMPTY == RFEMPTY){

	}
	GET_SPI(RXFIFO);

	SET_SPI(TXFIFO,(addr & 0xff));
	while((GET_SPI(SPSR))&RFEMPTY == RFEMPTY){

	}
	GET_SPI(RXFIFO);
// addr end

    //    printf("\n");
    while(i--){
		SET_SPI(TXFIFO,0x00);
		while((GET_SPI(SPSR))&RFEMPTY == RFEMPTY){

		}
	    data = GET_SPI(RXFIFO);
        if(base % 16 == 0 ){
        //        printf("0x%08x    ",base);
        }
        //    printf("%02x ",data);
        if(base % 16 == 7)
        //         printf("  ");
        if(base % 16 == 15)
        //       printf("\n");
		base++;
	}
//    printf("\n");
	return data;

}

void get_qt_result()
{
    unsigned char buf, buf1, buf2, buf3;
    int i, j;

    printf("*****************************************************************************************\n");
    /*start time*/
    buf = qt_read_pmon(0xff300, 1);
    buf1 = qt_read_pmon(0xff301, 1);
    buf2 = qt_read_pmon(0xff302, 1);
    buf3 = qt_read_pmon(0xff303, 1);

    printf("start test time: %c%c%c%c-", buf, buf1, buf2, buf3);

    buf = qt_read_pmon(0xff304, 1);
    buf1 = qt_read_pmon(0xff305, 1);
    buf2 = qt_read_pmon(0xff308, 1);
    buf3 = qt_read_pmon(0xff309, 1);
    printf("%c%c-%c%c", buf, buf1, buf2, buf3);

    buf = qt_read_pmon(0xff30c, 1);
    buf1 = qt_read_pmon(0xff30d, 1);
    buf2 = qt_read_pmon(0xff310, 1);
    buf3 = qt_read_pmon(0xff311, 1);
    printf(" %c%c-%c%c", buf, buf1, buf2, buf3);

    buf = qt_read_pmon(0xff314, 1);
    buf1 = qt_read_pmon(0xff315, 1);
    printf("-%c%c\n", buf, buf1);

    /*end time*/
    buf = qt_read_pmon(0xff320, 1);
    buf1 = qt_read_pmon(0xff321, 1);
    buf2 = qt_read_pmon(0xff322, 1);
    buf3 = qt_read_pmon(0xff323, 1);

    printf("end test time: %c%c%c%c-", buf, buf1, buf2, buf3);

    buf = qt_read_pmon(0xff324, 1);
    buf1 = qt_read_pmon(0xff325, 1);
    buf2 = qt_read_pmon(0xff328, 1);
    buf3 = qt_read_pmon(0xff329, 1);
    printf("%c%c-%c%c", buf, buf1, buf2, buf3);

    buf = qt_read_pmon(0xff32c, 1);
    buf1 = qt_read_pmon(0xff32d, 1);
    buf2 = qt_read_pmon(0xff330, 1);
    buf3 = qt_read_pmon(0xff331, 1);
    printf(" %c%c-%c%c", buf, buf1, buf2, buf3);

    buf = qt_read_pmon(0xff334, 1);
    buf1 = qt_read_pmon(0xff335, 1);
    printf("-%c%c\n", buf, buf1);

    /*test numbers*/
    buf = qt_read_pmon(0xff340, 1);
    buf1 = qt_read_pmon(0xff341, 1);
    if(buf1 == 0xf)
        printf("test numbers:%c\n", buf);
    else
        printf("test numbers:%c%c\n", buf1, buf);

    /*usb*/
    for(i = 0; i < 12; i++)
    {
        buf = qt_read_pmon(0xff350 + 4 * i + 2, 1);
        if(buf == 0x31)
        {
            printf("USB%d:^ ^ test successful! ^ ^\n", i);

        }else if(buf == 0x30){
            printf("USB%d:test failed!\n", i);
        }else
            printf("USB%d:not find!\n", i);
    }

    /*sata*/
    for(i = 0; i < 8; i++)
    {
        buf = qt_read_pmon(0xff380 + 4 * i + 2, 1);
        if(buf == 0x31)
        {
            printf("SATA%d:^ ^ test successful! ^ ^\n", i);

        }else if(buf == 0x30){
            printf("SATA%d:test failed!\n", i);
        }else
            printf("SATA%d:not detect!\n", i);
    }

    /*net*/
    for(i = 0; i < 12; i++)
    {
        buf = qt_read_pmon(0xff3a0 + 4 * i + 2, 1);
        if(buf == 0x31)
        {
            printf("eth%d:^ ^ test successful! ^ ^\n", i);

        }else if(buf == 0x30){
            printf("eth%d:test failed!\n", i);
        }else
            printf("eth%d:not find!\n", i);
    }

    /*gpio*/
    for(i = 0, j = 0; j < 32; i++, j++)
    {
        buf = qt_read_pmon(0xff3d0 + 4 * j + 2, 1);
        if(buf == 0x31)
        {
            printf("GPIO%d:^ ^ test successful! ^ ^\n", i);
        }else if(buf == 0x30){
            printf("GPIO%d:test failed!\n", i);
        }else
            printf("GPIO%d:no test\n", i);

        i = i + 1;

        buf = qt_read_pmon(0xff3d0 + 4 * j + 3, 1);
        if(buf == 0x31)
        {
            printf("GPIO%d:^ ^ test successful! ^ ^", i);
        }else if(buf == 0x30){
            printf("GPIO%d:test failed!\n", i);
        }else{
            printf("GPIO%d:no test\n", i);
        }
    }

    /*uart*/
    for(i = 0; i < 12; i++)
    {
        buf = qt_read_pmon(0xff4b0 + 4 * i + 2, 1);
        if(buf == 0x31)
        {
            printf("UART%d:^ ^ test successful! ^ ^\n", i);

        }else if(buf == 0x30){
            printf("UART%d:test failed!\n", i);
        }else
            printf("UART%d:not find!\n", i);
    }

    /*i2c*/
    for(i = 0; i < 8; i++)
    {
        buf = qt_read_pmon(0xff500 + 4 * i + 2, 1);
        if(buf == 0x31)
        {
            printf("I2C%d:^ ^ test successful! ^ ^\n", i);

        }else if(buf == 0x30){
            printf("I2C%d:test failed!\n", i);
        }else
            printf("I2C%d:not find!\n", i);
    }

    /*can*/
    for(i = 0; i < 2; i++)
    {
        buf = qt_read_pmon(0xff520 + 4 * i + 2, 1);
        if(buf == 0x31)
        {
            printf("CAN%d:^ ^ test successful! ^ ^\n", i);

        }else if(buf == 0x30){
            printf("CAN%d:test failed!\n", i);
        }else
            printf("CAN%d:not find!\n", i);
    }

    /*rtc*/
    for(i = 0; i < 2; i++)
    {
        buf = qt_read_pmon(0xff540 + 4 * i + 2, 1);
        if(buf == 0x31)
        {
            printf("RTC%d:^ ^ test successful! ^ ^\n", i);

        }else if(buf == 0x30){
            printf("RTC%d:test failed!\n", i);
        }else
            printf("RTC%d:not find!\n", i);
    }

    printf("*****************************************************************************************\n");
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
    {"read_test_result", "", 0, "read qt test result", get_qt_result, 0, 99, CMD_REPEAT},
	{0,0}
};

static void init_cmd __P((void)) __attribute__ ((constructor));

static void
init_cmd()
{
	cmdlist_expand(Cmds,1);
}

