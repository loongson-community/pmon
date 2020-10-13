 /*
  * This file is for CAT24C16 eeprom.
  * Author: Liu Shaozong
  */

#ifdef LS2K_EEPROM_MAC

#include <sys/linux/types.h>
#include <pmon.h>
#include <stdio.h>
#include <machine/pio.h>
#include "target/ls2k.h"
#include "target/board.h"
#include "target/eeprom.h"

#define	CAT24C16_ADDR	0xae

#define PRER_LO_REG	0x0
#define PRER_HI_REG	0x1
#define CTR_REG    	0x2
#define TXR_REG    	0x3
#define RXR_REG    	0x3
#define CR_REG     	0x4
#define SR_REG     	0x4

#define ee_outb(reg, val)	outb(LS2K_I2C0_REG_BASE + reg, val)
#define ee_inb(reg)		inb(LS2K_I2C0_REG_BASE + reg)

static void ls2k_i2c_stop(void)
{
again:
	ee_outb(CR_REG, CR_STOP);
	ee_inb(SR_REG);
	while (ee_inb(SR_REG) & SR_BUSY)
		goto again; 
}

void i2c_init(void)
{

	ee_outb(CTR_REG, 0x0);
	ee_outb(PRER_LO_REG, 0x71);
	ee_outb(PRER_HI_REG, 0x2);
	ee_outb(CTR_REG, 0x80);
}

static int i2c_tx_byte(unsigned char data, unsigned char opt)
{
	ee_outb(TXR_REG, data);
	ee_outb(CR_REG, opt);
	while (ee_inb(SR_REG) & SR_TIP) ;

	if (ee_inb(SR_REG) & SR_NOACK) {
		printf("Eeprom has no ack, Pls check the hardware!\n");
		ls2k_i2c_stop();
		return -1;
	}

	return 0;
}

static int i2c_send_addr(unsigned char data_addr)
{
	unsigned char ee_dev_addr = CAT24C16_ADDR;

	if (i2c_tx_byte(ee_dev_addr, CR_START | CR_WRITE) < 0)
		return 0;
	if (i2c_tx_byte(data_addr & 0xff, CR_WRITE) < 0)
		return 0;

	return 1;
}

 /*
  * BYTE WRITE: A write operation requires two 8-bit data word addresses following
  * the device address word and acknowledgment. Upon receipt of this address, 
  * the EEPROM will again respond with a zero and then clock in the first 8-bit
  * data word. Following receipt of the 8-bit data word, the EEPROM will output 
  * a zero and the addressing device, such as a microcontroller, must terminate 
  * the write sequence with a stop condition.
  *
  **/
int ls2k_eeprom_write_byte(unsigned char data_addr, unsigned char *buf)
{
	if (!i2c_send_addr(data_addr))
		return 0;

	if (i2c_tx_byte(*buf & 0xff, CR_WRITE) < 0)
		return 0;

	ls2k_i2c_stop();

	return 1;
}

 /*
  * PAGE WRITE: The 34K/64K EEPROM is capable of an 32-byte page writes.
  * A page write is initiated the same as a byte write, but the microcontroller 
  * does not send a stop condition after the first data word is clocked in. Insted,
  * after the EEPROM acknowledges receipt of the first data word, the microcontroller
  * can transmit up to 31 more data words. The EEPROM will respond with a zero after
  * each data word received. The microcontroller must terminate the page write 
  * sequence with a stop condition. If more than 32 data words are transmitted 
  * to the EEPROM, the data word address will "roll over" and previous data will be
  * overwritten.
  **/
int ls2k_eeprom_write_page(unsigned char data_addr, unsigned char *buf, int count)
{
	int i;
	if (!i2c_send_addr(data_addr))
		return 0;

	for (i = 0; i < count; i++) 
		if (i2c_tx_byte(buf[i] & 0xff, CR_WRITE) < 0)
			return 0;

	ls2k_i2c_stop();

	return i;
}
 /* 
  * The internal data word address counter maintains the last address
  * accessed during the lase read or write operation, incremented by one.
  * This address stays valid between operations as longs as the chip 
  * power is maintained. The address "roll over" during read is from the 
  * last byte if the last memory pate to the first byte of the first page.
  * The address "roll over" during write is from the last byte of the 
  * same page.
  *
  **/
int ls2k_eeprom_read_cur(unsigned char *buf)
{
	unsigned char ee_dev_addr = CAT24C16_ADDR | 0x1;
  
	if (i2c_tx_byte(ee_dev_addr, CR_START | CR_WRITE) < 0)
		return 0;

	ee_outb(CR_REG, CR_READ);
	while (ee_inb(SR_REG) & SR_TIP) ;

	*buf = ee_inb(RXR_REG);
	ls2k_i2c_stop();

	return 1;
}

 /*
  * A random read requires a "dummy" byte write sequence to load in the 
  * data word address. Once the device address word and data word address
  * are clocked in and acknoeledged by the EEPROM, the microcontroller must
  * generate another start condition. The microcontroller now initiates a
  * current address read by sending a device address and serially clocks 
  * out the data word. The microcontroller does not respond with a zero but
  * does generate a following stop condition.
  *
  **/
int ls2k_eeprom_read_rand(unsigned char data_addr, unsigned char *buf)
{
	if (!i2c_send_addr(data_addr))
		return 0;

	if (!ls2k_eeprom_read_cur(buf));
		return 0;

	return 1;
}

 /*
  * Sequential reads are initiated by either a current address read or a random 
  * address read. After the microcontroller receives a data word, it responds with 
  * acknowledge. As longs as the EEPROM receives an acknowledge, it will continue
  * to increment the data word address and serially cloke out sequential data words.
  * When the memory addrsss limit is reached, the data word address will "roll over"
  * and the sequential read will continue. The sequential read operation is terminated
  * when the microcontroller does not respond with a zero but does generate a following
  * stop condition.
  *
  */
static int i2c_read_seq_cur(unsigned char *buf, int count)
{
	int i;
	unsigned char ee_dev_addr = CAT24C16_ADDR |  0x1;

	if (i2c_tx_byte(ee_dev_addr, CR_START | CR_WRITE) < 0)
		return 0;

	for (i = 0; i < count; i++) {
		ee_outb(CR_REG, ((i == count - 1) ? 
					(CR_READ | CR_ACK) : CR_READ));
		while (ee_inb(SR_REG) & SR_TIP) ;
		buf[i] = ee_inb(RXR_REG);
	}

	ls2k_i2c_stop();
	return i;
}

static int i2c_read_seq_rand(unsigned char data_addr,
				unsigned char *buf, int count)
{
	if (!i2c_send_addr(data_addr))
		return 0;

	return i2c_read_seq_cur(buf, count);
}

int ls2k_eeprom_read_seq(unsigned char data_addr, unsigned char *buf, int count)
{
	int i;

	i = i2c_read_seq_rand(data_addr, buf, count);
	return i;
}

int cmd_eeprom_read(int ac, unsigned char *av[])
{
	unsigned char data_addr;
	int i, count;
	unsigned char buf[32];

	switch (ac) {
	case 2:
		data_addr = strtoul(av[1], NULL, 0);

		if (ls2k_eeprom_read_rand(data_addr, buf))
			printf("%x: 0x%x \n", data_addr, buf[0]);
		else
			printf("eeprom read error!\n");

		return 1;
	case 3:
		data_addr = strtoul(av[1], NULL, 0);
		count = strtoul(av[2], NULL, 0);
		if (ls2k_eeprom_read_seq(data_addr, buf, count) == count) {
			for (i = 0; i < count; i++) {
				printf("%4x: 0x%02x \n", data_addr + i, buf[i]);
			}
		} else {
			printf("eeprom read error!\n");
			return 0;
		}
		return i;
	default:
		printf("eepread @start_addr @count \n");
		printf("read \"count\" bytes from eeprom at address: start_addr \n");
	}
}

int cmd_eeprom_write(int ac, unsigned char *av[])
{
	int i, v, count;
	unsigned char *s = NULL;
	unsigned char buf[32];
	unsigned char data_addr = strtoul(av[1], NULL, 0);

	if (av[2]) {
		s = av[2];
	} else { 
		printf("Please accord to correct format, for example:\n");
		printf("\teepwrite a0 \"00 11 af\"\n");
		printf("This means 0xa0 = 0x00; 0xa1 = 0x11; 0xa2 = 0xaf\n");
		return 0;
	}

	count = strlen(s) / 3 + 1;

	for (i = 0; i < count; i++) {
		gethex(&v, s, 2); 
		buf[i] = v;
		s += 3;
	}

	if (ls2k_eeprom_write_page(data_addr, buf, count) == count)
		for (i = 0; i < count; i++) 
			printf("%4x <= 0x%x \n", data_addr + i, buf[i]);
	else
		printf("eeprom write error!\n");

	return 0;
}

static const Cmd Cmds[] = {
	{"Misc"},
	{"eepread", "", NULL, "read a address from an eeprom chip", cmd_eeprom_read,
	1, 5, 0},
	{"eepwrite", "", NULL, "write data to an eeprom chip", cmd_eeprom_write,
	1, 5, 0},
	{0, 0}
};

static void init_cmd __P((void)) __attribute__ ((constructor));
static void init_cmd()
{
	cmdlist_expand(Cmds, 1);
}

#endif
