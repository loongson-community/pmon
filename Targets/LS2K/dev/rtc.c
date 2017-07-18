/*
 * This file is for DS1338 RTC.
 */
#include <sys/linux/types.h>
#include <pmon.h>
#include <stdio.h>
#include <machine/pio.h>
#include "target/ls2k.h"

#define	DS1338_ADDR	0xd0

#define PRER_LO_REG	0x0
#define PRER_HI_REG	0x1
#define CTR_REG    	0x2
#define TXR_REG    	0x3
#define RXR_REG    	0x3
#define CR_REG     	0x4
#define SR_REG     	0x4

#define ls2k_writeb(reg, val)	outb(LS2K_I2C0_REG_BASE + reg, val)
#define ls2k_readb(reg)		inb(LS2K_I2C0_REG_BASE + reg)

#define ls2k_i2c1_writeb(reg, val)	outb(LS2K_I2C1_REG_BASE + reg, val)
#define ls2k_i2c1_readb(reg)		inb(LS2K_I2C1_REG_BASE + reg)
static void ls2k_i2c_stop(void)
{
again:
	ls2k_writeb(CR_REG, CR_STOP);
	ls2k_readb(SR_REG);
	while (ls2k_readb(SR_REG) & SR_BUSY)
		goto again; 
}

static void i2c_init(void)
{
	ls2k_writeb(CTR_REG, 0x0);
	ls2k_writeb(PRER_LO_REG, 0x71);
	ls2k_writeb(PRER_HI_REG, 0x2);
	ls2k_writeb(CTR_REG, 0x80);
}

static int i2c_tx_byte(u8 data, u8 opt)
{
	ls2k_writeb(TXR_REG, data);
	ls2k_writeb(CR_REG, opt);
	while (ls2k_readb(SR_REG) & SR_TIP) ;

	if (ls2k_readb(SR_REG) & SR_NOACK) {
		printf("rtc has no ack, Pls check the hardware!\n");
		ls2k_i2c_stop();
		return -1;
	}

	return 0;
}

static void ls2k_i2c1_stop(void)
{
again:
	ls2k_i2c1_writeb(CR_REG, CR_STOP);
	ls2k_i2c1_readb(SR_REG);
	while (ls2k_i2c1_readb(SR_REG) & SR_BUSY)
		goto again; 
}

static void i2c1_init(void)
{
	delay(1000);
	ls2k_i2c1_writeb(CTR_REG, 0x0);
	delay(1000);
	ls2k_i2c1_writeb(PRER_LO_REG, 0x71);
	ls2k_i2c1_writeb(PRER_HI_REG, 0x2);
	ls2k_i2c1_writeb(CTR_REG, 0x80);
}

static int i2c1_tx_byte(u8 data, u8 opt)
{
	ls2k_i2c1_writeb(TXR_REG, data);
	ls2k_i2c1_writeb(CR_REG, opt);
	while (ls2k_i2c1_readb(SR_REG) & SR_TIP) ;

	if (ls2k_i2c1_readb(SR_REG) & SR_NOACK) {
		printf("rtc has no ack, Pls check the hardware!\n");
		ls2k_i2c1_stop();
		return -1;
	}

	return 0;
}

static int ltc_send_addr(u8 dev_addr ,u8 data_addr)
{

	if (i2c1_tx_byte(dev_addr, CR_START | CR_WRITE) < 0)
		return 0;
	if (i2c1_tx_byte(data_addr, CR_WRITE) < 0)
		return 0;

	return 1;
}

int ltc_write(u8 dev_addr,u8 data_addr, u8 *buf, int count)
{
	u8 i;
	if (!ltc_send_addr(dev_addr,data_addr))
		return 0;

	for (i = 0; i < count; i++)
		if (i2c1_tx_byte(buf[i] & 0xff, CR_WRITE) < 0)
			return 0;
	ls2k_i2c1_stop();

	return i;
}

static int ltc_read_cur(u8 dev_addr,u8 *buf, int count)
{
	u8 i;
	dev_addr |= 0x1;

	if (i2c1_tx_byte(dev_addr, CR_START | CR_WRITE) < 0)
		return 0;

	for (i = 0; i < count; i++) {
		ls2k_i2c1_writeb(CR_REG, ((i == count - 1) ? 
					(CR_READ | CR_ACK) : CR_READ));
		while (ls2k_i2c1_readb(SR_REG) & SR_TIP) ;
		buf[i] = ls2k_i2c1_readb(RXR_REG);
	}

	ls2k_i2c1_stop();
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
	i2c1_init();	
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
	ltc_test(0xd8);
	ltc_test(0xde);
}
static int rtc_send_addr(u8 data_addr)
{
	u8 dev_addr = DS1338_ADDR;

	if (i2c_tx_byte(dev_addr, CR_START | CR_WRITE) < 0)
		return 0;
	if (i2c_tx_byte(data_addr, CR_WRITE) < 0)
		return 0;

	return 1;
}

int rtc_write(u8 data_addr, u8 *buf, int count)
{
	u8 i;
	if (!rtc_send_addr(data_addr))
		return 0;

	for (i = 0; i < count; i++)
		if (i2c_tx_byte(buf[i] & 0xff, CR_WRITE) < 0)
			return 0;
	ls2k_i2c_stop();

	return i;
}

static int rtc_read_cur(u8 *buf, int count)
{
	u8 i;
	u8 dev_addr = DS1338_ADDR |  0x1;

	if (i2c_tx_byte(dev_addr, CR_START | CR_WRITE) < 0)
		return 0;

	for (i = 0; i < count; i++) {
		ls2k_writeb(CR_REG, ((i == count - 1) ? 
					(CR_READ | CR_ACK) : CR_READ));
		while (ls2k_readb(SR_REG) & SR_TIP) ;
		buf[i] = ls2k_readb(RXR_REG);
	}

	ls2k_i2c_stop();
	return i;
}
int rtc_read_rand(u8 data_addr, u8 *buf,int count)
{
	if (!rtc_send_addr(data_addr))
		return 0;

	if (!rtc_read_cur(buf,count))
		return 0;
	return 1;
}

int rtc_get_sec(void)
{
	u8 tmp,tmp1;
	u8 buf[1] = {0};
	//0 is the start register.
	if (rtc_read_rand(0, buf,1)) {
		/*Becuse the RTC API need ture value,we must calculate the ture number.*/
		tmp = buf[0] & 0xf;
		tmp1 = (buf[0] >> 4) * 10;
		tmp += tmp1;
		buf[0] = tmp;
		return buf[0];
	} else
		return 0;
}
int rtc_get_time(u8 *buf)
{
	u8 tmp,tmp1;

	//0 is the start register.
	if (rtc_read_rand(0, buf,7)) {
		/*Becuse the RTC API need ture value,we must calculate the ture number.*/
		tmp = buf[0] & 0xf;
		tmp1 = (buf[0] >> 4) * 10;
		tmp += tmp1;
		buf[0] = tmp;
		
		tmp = buf[1] & 0xf;
		tmp1 = (buf[1] >> 4) * 10;
		tmp += tmp1;
		buf[1] = tmp;
        
		tmp = buf[2] & 0xf;
		tmp1 = (buf[2] >> 4) * 10;
		tmp += tmp1;
		buf[2] = tmp;
        
		tmp = buf[5] & 0xf;
		tmp1 = (buf[5] >> 4) * 10;
		tmp += tmp1;
		buf[5] = tmp;

		return 1;
	} else
		return 0;
}
int rtc_set_time(u8 *buf)
{
	u8 tmp,tmp1;
	/*The RTC API return a number,we must calculate the register value.*/
	tmp = buf[0] % 10;
	tmp1 = ((buf[0] / 10) << 4);
	tmp += tmp1;
	buf[0] = tmp;

	tmp = buf[1] % 10;
	tmp1 = ((buf[1] / 10) << 4);
	tmp += tmp1;
	buf[1] = tmp;

	tmp = buf[2] % 10;
	tmp1 = ((buf[2] / 10) << 4);
	tmp += tmp1;
	buf[2] = tmp;

	tmp = buf[5] % 10;
	tmp1 = ((buf[5] / 10) << 4);
	tmp += tmp1;
	buf[5] = tmp;
#if 0 //debug
	for(tmp = 0;tmp < 7;tmp++)
		printf(" %02x ",buf[tmp]);	
	printf("\n");	
#endif	
	delay(1000);
	//0 is the start register.
	if (rtc_write(0, buf, 7))
		return 1;
	else
		return 0;
}
void rtc_test(void)
{
	u8 buf[8] = {0};
	int i;
	u8 data_addr = 0x0;

	i2c_init();

	rtc_read_rand(data_addr, buf,8);
	printf("Read the RTC registers!\n");
	for (i =0;i < 8;i++)
		printf("0x%02x ",buf[i]);
	printf("\n");

	buf[3] = 1;
	printf("new RTC SET THE TIME\n");
	rtc_write(data_addr, buf, 8);

	rtc_read_rand(data_addr, buf,8);
	for (i =0;i < 8;i++)
		printf("0x%02x ",buf[i]);
	printf("\n");
	
}
static const Cmd Cmds[] = {
	{"Misc"},
	{"rtc_test", "", NULL, "test the i2c rtc is ok?", rtc_test, 1, 5, 0},
	{"ltc_test", "", NULL, "test the i2c rtc is ok?", ltc_read, 1, 5, 0},
	{0, 0}
};

static void init_cmd __P((void)) __attribute__ ((constructor));
static void init_cmd()
{
	cmdlist_expand(Cmds, 1);
}
