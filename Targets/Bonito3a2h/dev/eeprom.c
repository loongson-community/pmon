 /*
  * This file is for AT24C64 eeprom.
  * Author: liushaozong
  */
#include <pmon.h>
#include <stdio.h>
#include "target/ls2h.h"
#include "target/eeprom.h"

#define	AT24C64_ADDR	0xa0
static void ls2h_i2c_stop()
{
again:
	write_reg_byte(LS2H_I2C1_CR_REG, CR_STOP);
	read_reg_byte(LS2H_I2C1_SR_REG);
	while (read_reg_byte(LS2H_I2C1_SR_REG) & SR_BUSY)
		goto again; 
}
void ls2h_i2c1_init()
{
	write_reg_byte(LS2H_I2C1_CTR_REG, 0x0);
	write_reg_byte(LS2H_I2C1_PRER_LO_REG, 0x2c);
	write_reg_byte(LS2H_I2C1_PRER_HI_REG, 0x1);
	write_reg_byte(LS2H_I2C1_CTR_REG, 0x80);
}

static int i2c_send_addr(int data_addr)
{
	unsigned char ee_dev_addr = AT24C64_ADDR;
	int i = (data_addr >> 8) & 0x1f;

	write_reg_byte(LS2H_I2C1_TXR_REG, ee_dev_addr);
	write_reg_byte(LS2H_I2C1_CR_REG, (CR_START | CR_WRITE));
	while (read_reg_byte(LS2H_I2C1_SR_REG) & SR_TIP) ;

	if (read_reg_byte(LS2H_I2C1_SR_REG) & SR_NOACK) {
		printf("%s:%d  eeprom has no ack\n", __func__, __LINE__);
		ls2h_i2c_stop();
		return 0;
	}

	write_reg_byte(LS2H_I2C1_TXR_REG, (i & 0xff));
	write_reg_byte(LS2H_I2C1_CR_REG, CR_WRITE);
	while (read_reg_byte(LS2H_I2C1_SR_REG) & SR_TIP) ;

	if (read_reg_byte(LS2H_I2C1_SR_REG) & SR_NOACK) {
		printf("%s:%d  eeprom has no ack\n", __func__, __LINE__);
		ls2h_i2c_stop();
		return 0;
	}

	write_reg_byte(LS2H_I2C1_TXR_REG, (data_addr & 0xff));
	write_reg_byte(LS2H_I2C1_CR_REG, CR_WRITE);
	while (read_reg_byte(LS2H_I2C1_SR_REG) & SR_TIP) ;

	if (read_reg_byte(LS2H_I2C1_SR_REG) & SR_NOACK) {
		printf("%s:%d  eeprom has no ack\n", __func__, __LINE__);
		ls2h_i2c_stop();
		return 0;
	}

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
int eeprom_write_byte(int data_addr, unsigned char *buf)
{
	int i;
	i = i2c_send_addr(data_addr);
	if (!i) {
		printf("%s:%d send addr failed!\n", __func__, __LINE__);
		return 0;
	}
	write_reg_byte(LS2H_I2C1_TXR_REG, (*buf & 0xff));
	write_reg_byte(LS2H_I2C1_CR_REG, CR_WRITE);
	while (read_reg_byte(LS2H_I2C1_SR_REG) & SR_TIP) ;

	if (read_reg_byte(LS2H_I2C1_SR_REG) & SR_NOACK) {
		printf("%s:%d  eeprom has no ack\n", __func__, __LINE__);
		ls2h_i2c_stop();
		return 0;
	}

	ls2h_i2c_stop();
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
int eeprom_write_page(int data_addr, unsigned char *buf, int count)
{
	int i;
	i = i2c_send_addr(data_addr);
	if (!i) {
		printf("%s:%d send addr error!\n", __func__, __LINE__);
		return 0;
	}

	for (i = 0; i < count; i++) {
		write_reg_byte(LS2H_I2C1_TXR_REG, (buf[i] & 0xff));
		write_reg_byte(LS2H_I2C1_CR_REG, CR_WRITE);
		while (read_reg_byte(LS2H_I2C1_SR_REG) & SR_TIP);

		if (read_reg_byte(LS2H_I2C1_SR_REG) & SR_NOACK) {
			printf("%s:%d  eeprom has no ack\n", __func__,
			       __LINE__);
			ls2h_i2c_stop();
			return 0;
		}
	}

	ls2h_i2c_stop();
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
int eeprom_read_cur(unsigned char *buf)
{
	unsigned char ee_dev_addr = AT24C64_ADDR | 0x1;
  
	write_reg_byte(LS2H_I2C1_TXR_REG, ee_dev_addr);
	write_reg_byte(LS2H_I2C1_CR_REG, (CR_START | CR_WRITE));
	while (read_reg_byte(LS2H_I2C1_SR_REG) & SR_TIP) ;

	if (read_reg_byte(LS2H_I2C1_SR_REG) & SR_NOACK) {
		printf("%s:%d  eeprom has no ack\n", __func__, __LINE__);
		ls2h_i2c_stop();
		return 0;
	}

	write_reg_byte(LS2H_I2C1_CR_REG, CR_READ);
	while (read_reg_byte(LS2H_I2C1_SR_REG) & SR_TIP) ;

	*buf = read_reg_byte(LS2H_I2C1_RXR_REG);
	ls2h_i2c_stop();

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
int eeprom_read_rand(int data_addr, unsigned char *buf)
{
	int i;
	i = i2c_send_addr(data_addr);
	if (!i) {
		printf("%s:%d eeprpm random send addr error!\n", __func__,
			__LINE__);
		return 0;
	}

	i = eeprom_read_cur(buf);
	if (!i) {
		printf("%s:%d eeprom random read failed!\n", __func__,
		       __LINE__);
		return 0;
	}

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
	unsigned char ee_dev_addr = AT24C64_ADDR |  0x1;

	write_reg_byte(LS2H_I2C1_TXR_REG, ee_dev_addr);
	write_reg_byte(LS2H_I2C1_CR_REG, (CR_START | CR_WRITE));
	while (read_reg_byte(LS2H_I2C1_SR_REG) & SR_TIP) ;

	if (read_reg_byte(LS2H_I2C1_SR_REG) & SR_NOACK) {
		printf("%s:%d  eeprom has no ack\n", __func__, __LINE__);
		ls2h_i2c_stop();
		return 0;
	}

	for (i = 0; i < count; i++) {
		write_reg_byte(LS2H_I2C1_CR_REG, 
				((i == count - 1) ? (CR_READ | CR_ACK) : CR_READ));
		while (read_reg_byte(LS2H_I2C1_SR_REG) & SR_TIP) ;
		buf[i] = read_reg_byte(LS2H_I2C1_RXR_REG);
	}
	ls2h_i2c_stop();
	return i;
}

static int i2c_read_seq_rand(int data_addr, unsigned char *buf, int count)
{
	int i;
	i = i2c_send_addr(data_addr);
	if (!i) {
		printf("%s:%d send addr failed!\n", __func__, __LINE__);
		return i;
	}

	i = i2c_read_seq_cur(buf, count);

	if (!i) {
		printf("%s:%d eeprom random read failed!\n", __func__,
		       __LINE__);
		return 0;
	}

	return i;
}

int eeprom_read_seq(int data_addr, unsigned char *buf, int count)
{
	int i;
	i = i2c_read_seq_rand(data_addr, buf, count);
	return i;
}

int cmd_eeprom_read(int ac, unsigned char *av[])
{
	int i, data_addr, count;
	unsigned char buf[32];
	switch (ac) {
	case 2:
		data_addr = strtoul(av[1], NULL, 0);

		if (eeprom_read_rand(data_addr, buf))
			printf("%x: 0x%x \n", data_addr, buf[0]);
		else
			printf("eeprom read error!\n");

		return 1;
	case 3:
		data_addr = strtoul(av[1], NULL, 0);
		count = strtoul(av[2], NULL, 0);
		if (eeprom_read_seq(data_addr, buf, count) == count) {
			for (i = 0; i < count; i++) {
				printf("%4x: 0x%02x \n", data_addr + i, buf[i]);
			}
		} else {
			printf("eeprom read error!\n");
			return 0;
		}
		return i;
	default:
		printf("e2pread @start_addr @count \n");
		printf("read \"count\" bytes from eeprom at address: start_addr \n");
	}
}
int cmd_eeprom_write(int ac, unsigned char *av[])
{
	int i, v, count;
	unsigned char *s = NULL;
	unsigned char buf[32];
	int data_addr = strtoul(av[1], NULL, 0);
	if (av[2]) s = av[2];
	else { 
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

	if (eeprom_write_page(data_addr, buf, count) == count)
		for (i = 0; i < count; i++) 
			printf("%4x <= 0x%x \n", data_addr + i, buf[i]);
	else
		printf("eeprom write error!\n");

	return 0;
}

int cmd_setmac(int ac, unsigned char *av[])
{
	int i, j, v, count, data_addr;
	unsigned char *s = NULL;
	unsigned char buf[32];
	data_addr = strtoul(av[1] + 3, NULL, 0);
	data_addr *= 6;

	if (ac == 1) {
		for (i = 0; i < 2; i++) {	
			if (eeprom_read_seq(i * 6, buf, 6) == 6) {
				printf("eth%d Mac address: ", i);
				for (j = 0; j < 6; j++)
					printf("%02x%s", buf[j], (5-j)?":":" ");
				printf("\n");
			} else {
				printf("eeprom write error!\n");
				return 0;
			}
		}
		goto warning;
	}

	if (av[2]) s = av[2];
	else goto warning; 

	count = strlen(s) / 3 + 1;
	if (count - 6) goto warning;

	for (i = 0; i < count; i++) {
		gethex(&v, s, 2); 
		buf[i] = v;
		s += 3;
	}

	if (eeprom_write_page(data_addr, buf, count) == count) {
		printf("set eth%d  Mac address: %s\n",data_addr / 6, av[2]);
		printf("The machine should be restarted to make the mac change to take effect!!\n");
	} else 
		printf("eeprom write error!\n");
	return 0;
warning:
	printf("Please accord to correct format.\nFor example:\n");
	printf("\tsetmac eth1 \"00:11:22:33:44:55\"\n");
	printf("\tThis means set eth1's Mac address 00:11:22:33:44:55\n");
	return 0;
}

static const Cmd Cmds[] = {
	{"Misc"},
	{"i2c1init", "", NULL, "init LS2H I2C1", ls2h_i2c1_init, 1, 5, 0},
	{"setmac", "", NULL, "set the Mac address of LS2H syn0 and syn1", cmd_setmac, 1, 5, 0},
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
