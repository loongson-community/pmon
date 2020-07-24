 /*
  * This file is for ls3a4000 i2c slave mode test
  * author: zbq
  */
#include <sys/linux/types.h>
#include <pmon.h>
#include <stdio.h>
#include <machine/pio.h>
#include "target/ls7a.h"

#define PRER_LO_REG	0x0
#define PRER_HI_REG	0x1
#define CTR_REG    	0x2
#define TXR_REG    	0x3
#define RXR_REG    	0x3
#define CR_REG     	0x4
#define SR_REG     	0x4
#define SLV_CTRL_REG	0x7

#define LS3A4000_I2C0_REG_BASE	0xbfe00120
#define LS3A4000_I2C1_REG_BASE	0xbfe00130
//#define I2C_BASE LS7A_I2C0_REG_BASE
//#define I2C_BASE LS7A_I2C1_REG_BASE
#define writeb(reg, val)	outb(reg, val)
#define readb(reg)		inb(reg)
unsigned int i2c_base_addr;
unsigned char word_offset = 0;

static void ls_i2c_stop(void)
{
	do {
		writeb(i2c_base_addr + CR_REG, CR_STOP);
		readb(i2c_base_addr + SR_REG);
	} while (readb(i2c_base_addr + SR_REG) & SR_BUSY);
}

void ls_i2c_init(void)
{
	printf("base 0x%lx \n", i2c_base_addr);
	readb(i2c_base_addr + CTR_REG) &=  ~0x80;
	writeb(i2c_base_addr + PRER_LO_REG, 0x71);
	writeb(i2c_base_addr + PRER_HI_REG, 0x1);
//	writeb(i2c_base_addr + PRER_LO_REG, 0x53);
//	writeb(i2c_base_addr + PRER_HI_REG, 0x7);
	readb(i2c_base_addr + CTR_REG) |=  0x80;
}

static int ls_i2c_tx_byte(unsigned char data, unsigned char opt)
{
	int times = 1000000;
	writeb(i2c_base_addr + TXR_REG, data);
	writeb(i2c_base_addr + CR_REG, opt);
	while ((readb(i2c_base_addr + SR_REG) & SR_TIP) && times--);
	if (times < 0) {
		printf("ls_i2c_tx_byte SR_TIP can not ready!\n");
		ls_i2c_stop();
		return -1;
	}

	if (readb(i2c_base_addr + SR_REG) & SR_NOACK) {
		//printf("device has no ack, Pls check the hardware!\n");
		ls_i2c_stop();
		return -1;
	}

	return 0;
}

static int ls_i2c_send_addr(unsigned char dev_addr,unsigned int data_addr)
{
	if (ls_i2c_tx_byte(dev_addr, CR_START | CR_WRITE) < 0)
		return 0;

	if (word_offset) {
	//some device need word size addr
		if (ls_i2c_tx_byte((data_addr >> 8) & 0xff, CR_WRITE) < 0)
			return 0;
	}
	if (ls_i2c_tx_byte(data_addr & 0xff, CR_WRITE) < 0)
		return 0;

	return 1;
}


 /*
 * the function write sequence data.
 * dev_addr : device id
 * data_addr : offset
 * buf : the write data buffer
 * count : size will be write
  */
int ls_i2c_write_seq(unsigned char dev_addr,unsigned int data_addr, unsigned char *buf, int count)
{
	int i;
	if (!ls_i2c_send_addr(dev_addr,data_addr))
		return 0;
	for (i = 0; i < count; i++)
		if (ls_i2c_tx_byte(buf[i] & 0xff, CR_WRITE) < 0)
			return 0;

	//the node voltage need 2 chips configure at the same time
	if (dev_addr == 0x20) {
		if (!ls_i2c_send_addr(dev_addr | 0x2,data_addr))
			return 0;
		for (i = 0; i < count; i++)
			if (ls_i2c_tx_byte(buf[i] & 0xff, CR_WRITE) < 0)
				return 0;
	}
	ls_i2c_stop();

	return i;
}

 /*
 * the function write one byte.
 * dev_addr : device id
 * data_addr : offset
 * buf : the write data
  */
int ls_i2c_write_byte(unsigned char dev_addr,unsigned int data_addr, unsigned char *buf)
{
	return ls_i2c_write_seq(dev_addr, data_addr, *buf, 1);
}
 /*
  * Sequential reads by a current address read.
 * dev_addr : device id
 * data_addr : offset
 * buf : the write data buffer
 * count : size will be write
  */
static int ls_i2c_read_seq_cur(unsigned char dev_addr,unsigned char *buf, int count)
{
	int i;
	dev_addr |= 0x1;

	if (ls_i2c_tx_byte(dev_addr, CR_START | CR_WRITE) < 0)
		return 0;

	for (i = 0; i < count; i++) {
		writeb(i2c_base_addr + CR_REG, ((i == count - 1) ?
					(CR_READ | CR_ACK) : CR_READ));
		while (readb(i2c_base_addr + SR_REG) & SR_TIP) ;
		buf[i] = readb(i2c_base_addr + RXR_REG);
	}

	ls_i2c_stop();
	return i;
}

int ls_i2c_read_seq_rand(unsigned char dev_addr,unsigned int data_addr,
				unsigned char *buf, int count)
{
	if (!ls_i2c_send_addr(dev_addr,data_addr))
		return 0;

	return ls_i2c_read_seq_cur(dev_addr,buf, count);
}
