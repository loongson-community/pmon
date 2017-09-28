/*
 * This file is for DS1338 RTC.
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

#define ls7a_i2c_writeb(reg, val)	outb(LS7A_I2C1_REG_BASE + reg, val)
#define ls7a_i2c_readb(reg)		inb(LS7A_I2C1_REG_BASE + reg)

static void ls7a_i2c_stop(void)
{
again:
	ls7a_i2c_writeb(CR_REG, CR_STOP);
	ls7a_i2c_readb(SR_REG);
	while (ls7a_i2c_readb(SR_REG) & SR_BUSY)
		goto again; 
}

static void i2c_init(void)
{
	delay(1000);
	ls7a_i2c_writeb(CTR_REG, 0x0);
	delay(1000);
	ls7a_i2c_writeb(PRER_LO_REG, 0x71);
	ls7a_i2c_writeb(PRER_HI_REG, 0x2);
	ls7a_i2c_writeb(CTR_REG, 0x80);
}

static int i2c_tx_byte(u8 data, u8 opt)
{
	ls7a_i2c_writeb(TXR_REG, data);
	ls7a_i2c_writeb(CR_REG, opt);
	while (ls7a_i2c_readb(SR_REG) & SR_TIP) ;

	if (ls7a_i2c_readb(SR_REG) & SR_NOACK) {
		printf("ltc has no ack, Pls check the hardware!\n");
		ls7a_i2c_stop();
		return -1;
	}

	return 0;
}

static int ltc_send_addr(u8 dev_addr ,u8 data_addr)
{

	if (i2c_tx_byte(dev_addr, CR_START | CR_WRITE) < 0)
		return 0;
	if (i2c_tx_byte(data_addr, CR_WRITE) < 0)
		return 0;

	return 1;
}

int ltc_write(u8 dev_addr,u8 data_addr, u8 *buf, int count)
{
	u8 i;
	if (!ltc_send_addr(dev_addr,data_addr))
		return 0;

	for (i = 0; i < count; i++)
		if (i2c_tx_byte(buf[i] & 0xff, CR_WRITE) < 0)
			return 0;
	ls7a_i2c_stop();

	return i;
}

static int ltc_read_cur(u8 dev_addr,u8 *buf, int count)
{
	u8 i;
	dev_addr |= 0x1;

	if (i2c_tx_byte(dev_addr, CR_START | CR_WRITE) < 0)
		return 0;

	for (i = 0; i < count; i++) {
		ls7a_i2c_writeb(CR_REG, ((i == count - 1) ? 
					(CR_READ | CR_ACK) : CR_READ));
		while (ls7a_i2c_readb(SR_REG) & SR_TIP) ;
		buf[i] = ls7a_i2c_readb(RXR_REG);
	}

	ls7a_i2c_stop();
	return i;
}

int ltc_read_rand(u8 dev_addr,u8 data_addr, u8 *buf,int count)
{
	if (!ltc_send_addr(dev_addr,data_addr))
		return 0;

	if (!ltc_read_cur(dev_addr,buf,count))
		return 0;
	return 1;
}

int ltc_test(u8 dev_addr)
{
	u8 buf[0xe8] = {0};
	u8 data_addr = 0;
	int i;
	u32 tmp;
	unsigned int value,  value_max,  value_min, value1, value1_max, value1_min;
	i2c_init();	
	buf[0] = 0x10;
	ltc_write(dev_addr,0,buf,1);
	ltc_read_rand(dev_addr,data_addr,buf, 0xe8);

	for (i = 0; i < 0xe8; i++) {
		if (!(i % 16))
			printf("\n");
		printf("%02x ", buf[i]);
	}
	printf("\n");

        printf("result value should divide 10000 !\n");
        tmp = (buf[0x28] << 4) | (buf[0x29] >> 4);
	value = ((tmp/2) - 500)*10000;
	value = value/133;
        printf("CPU VDD %d\n",value);
        tmp = (buf[0x2a] << 4) | (buf[0x2b] >> 4);
        value_max = ((tmp/2) - 500)*10000;
	value_max = value_max/133;
        printf("MAX CPU VDD %d\n",value_max);
        tmp = (buf[0x2c] << 4) | (buf[0x2d] >> 4);
        value_min = ((tmp/2) - 500) * 10000;
	value_min /= 133;
        printf("MIN CPU VDD %d\n",value_min);
#if 1
        tmp = (buf[0x14] << 4) | (buf[0x15] >> 4);
        value1 = tmp * 0.50;
        printf("PEST_1V1 %d\n",value1);
        tmp = (buf[0x16] << 4) | (buf[0x17] >> 4);
        value1_max = tmp * 0.50;
        printf("MAX PEST_1V1 %d\n",value1_max);
        tmp = (buf[0x18] << 4) | (buf[0x19] >> 4);
        value1_min = tmp * 0.50;
        printf("MIN PEST_1V1 %d\n",value1_min);
#endif
#if 0
        tmp = (buf[0x2a] << 4) | (buf[0x2b] >> 4);
        value_max = ((tmp * 0.5)/1000 - 0.5)/0.133;
        printf("MAX CPU VDD %lf\n",value_max);
        tmp = (buf[0x2c] << 4) | (buf[0x2d] >> 4);
        value_min = ((tmp * 0.5)/1000 - 0.5)/0.133;
        printf("MIN CPU VDD %lf\n",value_min);

        tmp = (buf[0x14] << 4) | (buf[0x15] >> 4);
        value1 = tmp * 0.000050;
        printf("PEST_1V1 %lf\n",value1);
        tmp = (buf[0x16] << 4) | (buf[0x17] >> 4);
        value1_max = tmp * 0.000050;
        printf("MAX PEST_1V1 %lf\n",value1_max);
        tmp = (buf[0x18] << 4) | (buf[0x19] >> 4);
        value1_min = tmp * 0.000050;
        printf("MIN PEST_1V1 %lf\n",value1_min);
#endif
}

int ltc_read(void)
{
	ltc_test(0xd8);//device addr
	ltc_test(0xd2);
	ltc_test(0xce);
}
static const Cmd Cmds[] = {
	{"Misc"},
	{"ltc_test", "", NULL, "test the i2c rtc is ok?", ltc_read, 1, 5, 0},
	{0, 0}
};

static void init_cmd __P((void)) __attribute__ ((constructor));
static void init_cmd()
{
	cmdlist_expand(Cmds, 1);
}
