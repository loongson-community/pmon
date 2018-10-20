/*
 * For loongson7a dc controller dvo1 i2c
 * Use gpio to access ch7034 chip
 * */

#include <pmon.h>
#include <stdio.h>

#define SCL1	3
#define	SDA1	2

#define SCL0	1
#define	SDA0	0

#define	LOW	0
#define HIGH	1

#define	OUT	0
#define	IN	1

#define CHIP_7034_ADDR	0xea
#define CHIP_9022_ADDR	0x72
#define	ADV7513_ADDR	0x7a
#define	ADV7511_ADDR	0x72

static unsigned int dc_base = 0;
static unsigned int gpio_val_base = 0;
static unsigned int gpio_dir_base = 0;

unsigned char CH7034_VGA_REG_TABLE2[][131][2] = {
      {	//mtfvga
      //IN 800x600,out 800x600,ch7034,bypassmode vga,mode_idx=1
          { 0x03, 0x04 },//page 4
          { 0x52, 0xC3 },
          { 0x5A, 0x06 },
          { 0x5A, 0x04 },
          { 0x5A, 0x06 },
          { 0x52, 0xC1 },
          { 0x52, 0xC3 },
          { 0x5A, 0x04 },

          { 0x03, 0x00 },//page 1
          { 0x07, 0xD9 },
          { 0x08, 0xF1 },
          { 0x09, 0x13 },
          { 0x0A, 0xBE },
          { 0x0B, 0x23 },
          { 0x0C, 0x20 },
          { 0x0D, 0x20 },
          { 0x0E, 0x00 },
          { 0x0F, 0x28 },
          { 0x10, 0x80 },
          { 0x11, 0x12 },
          { 0x12, 0x58 },
          { 0x13, 0x74 },
          { 0x14, 0x00 },
          { 0x15, 0x01 },
          { 0x16, 0x04 },
          { 0x17, 0x00 },
          { 0x18, 0x00 },//mtf modify 888RGB
         // { 0x18, 0x20 },//565RGB
//          { 0x18, 0x02 },//888GRB
          { 0x19, 0xF8 },//freq
          { 0x1A, 0x9B },
          { 0x1B, 0x78 },
          { 0x1C, 0x69 },
          { 0x1D, 0x78 },
          { 0x1E, 0x00 },//output is  progressive
          { 0x1F, 0x23 },
          { 0x20, 0x20 },
          { 0x21, 0x20 },
          { 0x22, 0x00 },
          { 0x23, 0x10 },
          { 0x24, 0x60 },
          { 0x25, 0x12 },
          { 0x26, 0x58 },
          { 0x27, 0x74 },
          { 0x28, 0x00 },
          { 0x29, 0x0A },
          { 0x2A, 0x02 },
          { 0x2B, 0x09 },//vga output format:bypass mode
          { 0x2C, 0x00 },
          { 0x2D, 0x00 },
          { 0x2E, 0x3D },
          { 0x2F, 0x00 },//??
          { 0x32, 0xC0 },//???
          { 0x36, 0x40 },
          { 0x38, 0x47 },
          { 0x3D, 0x86 },
          { 0x3E, 0x00 },
          { 0x40, 0x0E },
          { 0x4B, 0x40 },//pwm control
          { 0x4C, 0x40 },//lvds output channel order register
          { 0x4D, 0x80 },
          { 0x54, 0x80 },//lvds
          { 0x55, 0x28 },//lvds
          { 0x56, 0x80 },//lvds
          { 0x57, 0x00 },//lvds
          { 0x58, 0x01 },//lvds
          { 0x59, 0x04 },//lvds
          { 0x5A, 0x02 },
          { 0x5B, 0xF2 },
          { 0x5C, 0xB9 },
          { 0x5D, 0xD6 },
          { 0x5E, 0x54 },
          { 0x60, 0x00 },
          { 0x61, 0x00 },
          { 0x64, 0x2D },
          { 0x68, 0x44 },
          { 0x6A, 0x40 },
          { 0x6B, 0x00 },
          { 0x6C, 0x10 },
          { 0x6D, 0x00 },
          { 0x6E, 0xA0 },
          { 0x70, 0x98 },
          { 0x74, 0x30 },//scaling control
          { 0x75, 0x80 },//scaling control
          { 0x7E, 0x0F },//de and pwm control
          { 0x7F, 0x00 },//test pattern

          { 0x03, 0x01 },//page 2
          { 0x08, 0x05 },
          { 0x09, 0x04 },//diffen register
          { 0x0B, 0x65 },
          { 0x0C, 0x4A },
          { 0x0D, 0x29 },
          { 0x0F, 0x9C },
          { 0x12, 0xD4 },
          { 0x13, 0x28 },
          { 0x14, 0x83 },
          { 0x15, 0x00 },
          { 0x16, 0x00 },
          { 0x1A, 0x6C },//DAC termination control register
          { 0x1B, 0x00 },
          { 0x1C, 0x00 },
          { 0x1D, 0x00 },
          { 0x23, 0x63 },
          { 0x24, 0xB4 },
          { 0x28, 0x54 },
          { 0x29, 0x60 },
          { 0x41, 0x60 },//lvds
          { 0x63, 0x2D },//DE polarity
          { 0x6B, 0x11 },
          { 0x6C, 0x06 },

          { 0x03, 0x03 },//page3
          { 0x26, 0x00 },
          { 0x28, 0x08 },//output control:DAC output is VGA
          { 0x2A, 0x00 },//output control:HDTV output through scaler

          { 0x03, 0x04 },//page 4
          { 0x10, 0x00 },
          { 0x11, 0x9B },
          { 0x12, 0x78 },
          { 0x13, 0x02 },
          { 0x14, 0x88 },
          { 0x15, 0x70 },
          { 0x20, 0x00 },
          { 0x21, 0x00 },
          { 0x22, 0x00 },
          { 0x23, 0x00 },
          { 0x24, 0x00 },
          { 0x25, 0x00 },
          { 0x26, 0x00 },
          { 0x54, 0xC4 },
          { 0x55, 0x5B },
          { 0x56, 0x4D },
          { 0x60, 0x01 },
          { 0x61, 0x62 },
      },
};

#define CH7034_REGMAP_LENGTH_VGA (sizeof(CH7034_VGA_REG_TABLE2[0]) / (2*sizeof(unsigned char)))

void gpioi2c_init()
{
	dc_base = *(volatile unsigned int *)0xba003110;
	dc_base &= 0xfffffff0;
	dc_base |= 0x80000000;

	gpio_val_base = dc_base + 0x1650;
	gpio_dir_base = dc_base + 0x1660;
#if 0
	printf("gpioi2c init regs base\n");
	printf("gpio_val_base 0x%x\n", gpio_val_base);
	printf("gpio_dir_base 0x%x\n", gpio_dir_base);
#endif
}

static void set_gpio_value(int gpio, int val)
{
	if (val == 1)
		*(volatile unsigned int *)gpio_val_base |= (1U << gpio);
	else
		*(volatile unsigned int *)gpio_val_base &= ~(1U << gpio);
}

static unsigned int get_gpio_value(int gpio)
{
	return (*(volatile unsigned int *)gpio_val_base & (1U << gpio)) ? 1 : 0;
}

static void set_gpio_direction(unsigned int gpio, int val)
{
	if (val == 1)
		*(volatile unsigned int *)gpio_dir_base |= (1 << gpio); 
	else
		*(volatile unsigned int *)gpio_dir_base &= ~(1 << gpio); 
}

static int gpioi2c_read_ack(int scl, int sda)
{
	set_gpio_direction(sda, IN);
	delay(200);

	set_gpio_value(scl, HIGH);
	delay(200);

	/* the ack signal will hold on sda untill scl falling edge */
	while(get_gpio_value(sda))
		delay(200);

	set_gpio_value(scl, LOW);
	delay(2000);

	return 0;
}

#if 0
static void gpioi2c_send_ack()
{
	set_gpio_direction(SDA, OUT);
	set_gpio_value(SCL, LOW);
	delay(200);
	set_gpio_value(SDA, 0);
	delay(200);
	set_gpio_value(SCL, HIGH);
	delay(200);
}
#endif

static int gpioi2c_write_byte(unsigned char dev_addr, unsigned char c)
{
	int i;
	int sda, scl;

	if (dev_addr == CHIP_7034_ADDR) {
		sda = SDA1;
		scl = SCL1;
	} else if (dev_addr == CHIP_9022_ADDR) {
		sda = SDA0;
		scl = SCL0;
	}

	set_gpio_direction(sda, OUT);

	for(i = 7; i >= 0; i--)
	{
		set_gpio_value(scl, LOW);
		delay(200);
		set_gpio_value(sda, (c & (1U << i)) ? 1 : 0);//high bit --> low bit
		delay(200);
		set_gpio_value(scl, HIGH);
		delay(200);
	}

	set_gpio_value(scl, LOW);
	delay(200);

	if (gpioi2c_read_ack(scl, sda))
	{
		printf("read slave dev ack invalid\n");//0:valid
		return 0;
	}
	return 1;
	
}

static void gpioi2c_read_byte(unsigned char dev_addr, unsigned char *c)
{
	int i;
	int sda, scl;

	if (dev_addr == CHIP_7034_ADDR) {
		sda = SDA1;
		scl = SCL1;
	} else if (dev_addr == CHIP_9022_ADDR) {
		sda = SDA0;
		scl = SCL0;
	}

	*c = 0;
	set_gpio_direction(sda, IN);

	for (i = 7; i >= 0; i--)
	{
		set_gpio_value(scl, HIGH);
		delay(200);
		*c = (*c << 1) | get_gpio_value(sda);
		set_gpio_value(scl, LOW);
		delay(200);
	}
//	gpioi2c_send_ack();
}

static void gpioi2c_start(unsigned char dev_addr)
{
	int sda, scl;

	if (dev_addr == CHIP_7034_ADDR) {
		sda = SDA1;
		scl = SCL1;
	} else if (dev_addr == CHIP_9022_ADDR) {
		sda = SDA0;
		scl = SCL0;
	}

	/* if set sda output without setting sda high,
 	 * the sda output value may be low
 	 * */
	set_gpio_value(sda, HIGH);

	set_gpio_direction(sda, OUT);
	set_gpio_direction(scl, OUT);

	set_gpio_value(scl, HIGH);
	delay(200);

	/* start signal: sda from high to low */
	set_gpio_value(sda, HIGH);
	delay(200);
	set_gpio_value(sda, LOW);
	delay(200);
}

static void gpioi2c_stop(unsigned char dev_addr)
{
	int sda, scl;

	if (dev_addr == CHIP_7034_ADDR) {
		sda = SDA1;
		scl = SCL1;
	} else if (dev_addr == CHIP_9022_ADDR) {
		sda = SDA0;
		scl = SCL0;
	}

	set_gpio_direction(sda, OUT);

	set_gpio_value(scl, HIGH);
	delay(200);

	set_gpio_value(sda, LOW);
	delay(200);
	set_gpio_value(sda, HIGH);
	delay(200);
}

static void gpioi2c_write(unsigned char dev_addr, unsigned char data_addr, unsigned char data)
{
	gpioi2c_start(dev_addr);
	if (!gpioi2c_write_byte(dev_addr, dev_addr))
	{
		printf("gpioi2c write dev_addr fail\n");
		return;
	}
	if (!gpioi2c_write_byte(dev_addr, data_addr))
	{
		printf("gpioi2c write data_addr fail\n");
		return;
	}
	if (!gpioi2c_write_byte(dev_addr, data))
	{
		printf("gpioi2c write data fail\n");
		return;
	}
	gpioi2c_stop(dev_addr);
}

static void dvo1_gpioi2c_write(int ac, unsigned char *av[])
{
	unsigned char dev_addr = CHIP_7034_ADDR;
	unsigned char data_addr;
	unsigned char data;

	switch (ac) {
	case 3:
		if (gpio_val_base == 0)
			gpioi2c_init();

		data_addr = strtoul(av[1], NULL, 0);
		data = strtoul(av[2], NULL, 0);

		gpioi2c_write(dev_addr, data_addr, data);
		printf("0x%2x <= 0x%2x \n", data_addr, data);

		break;
	default:
		printf("need 3 param: gw1 3 4");
		break;
	}
}

static void gpioi2c_read(unsigned char dev_addr, unsigned char data_addr, unsigned char *data)
{
	/* bit0 :1 read, 0 write */

	gpioi2c_start(dev_addr);

	if (!gpioi2c_write_byte(dev_addr, dev_addr))
	{
		printf("gpioi2c write dev_addr fail\n");
		return;
	}

	if (!gpioi2c_write_byte(dev_addr, data_addr))
	{
		printf("gpioi2c write data_addr fail\n");
		return;
	}

	gpioi2c_start(dev_addr);
	if (!gpioi2c_write_byte(dev_addr, dev_addr | 0x01))//for read
	{
		printf("gpioi2c write dev_addr fail\n");
		return;
	}

	gpioi2c_read_byte(dev_addr, data);

	gpioi2c_stop(dev_addr);
}

static void dvo1_gpioi2c_read(int ac, unsigned char *av[])
{
	/* bit0 :1 read, 0 write */
	unsigned char dev_addr = CHIP_7034_ADDR;
	unsigned char data_addr;
	unsigned char ret_val;

	switch (ac) {
	case 2:
		if (gpio_val_base == 0)
			gpioi2c_init();

		data_addr = strtoul(av[1], NULL, 0);
		gpioi2c_read(dev_addr, data_addr, &ret_val);
		printf("0x%2x : 0x%2x\n", data_addr, ret_val);
		break;
	default:
		printf("need 2 param: gr1 0");
		break;
	}
}

void gpioi2c_config_ch7034()
{
	int count;
	unsigned char dev_addr = CHIP_7034_ADDR;
	unsigned char data_addr;
	unsigned char data;
	unsigned char data2;

	/* get reg base addr */
	gpioi2c_init();

	for (count = 0; count < CH7034_REGMAP_LENGTH_VGA; count++)
	{
		data_addr = CH7034_VGA_REG_TABLE2[0][count][0];
		data = CH7034_VGA_REG_TABLE2[0][count][1];

		gpioi2c_write(dev_addr, data_addr, data);
#if DEBUG_CH7034
		gpioi2c_read(dev_addr, data_addr, &data2);

		if (data != data2)
			printf("not eq, data 0x%2x, data2 0x%2x\n\n\n", data, data2);
#endif
	}
}

void gpioi2c_config_sii9022a(void)
{
	unsigned char id0;
	unsigned char id1;
	unsigned char id2;
	unsigned char data;

	unsigned char dev_addr = CHIP_9022_ADDR;

	gpioi2c_init();

	gpioi2c_write(dev_addr, 0xc7, 0x00);
	gpioi2c_read(dev_addr, 0x1b, &id0);
	gpioi2c_read(dev_addr, 0x1c, &id1);
	gpioi2c_read(dev_addr, 0x1d, &id2);

	if (id0 != 0xb0 || id1 != 0x2 || id2 != 0x3) {
		printf("id err\n");
		return;
	}

	gpioi2c_read(dev_addr, 0x1e, &data);
	data &= ~(0x3);
	gpioi2c_write(dev_addr, 0x1e, data);

	gpioi2c_read(dev_addr, 0x1a, &data);
	data &= ~(1 << 4);
	data |= (1 << 0);
	gpioi2c_write(dev_addr, 0x1a, data);
	gpioi2c_write(dev_addr, 0x26, 0x40);
}

#ifdef USE_ADV7511
void dvo_hdmi_init()
{
	unsigned char dev_addr = CHIP_9022_ADDR;
	unsigned char d;

	gpioi2c_init();

	gpioi2c_write(dev_addr, 0x41, 0x10);
	gpioi2c_read(dev_addr, 0x41, &d); printf("0x41:0x%x\n",d);
	gpioi2c_write(dev_addr, 0x98, 0x3);
	gpioi2c_read(dev_addr, 0x98, &d); printf("0x98:0x%x\n",d);
	gpioi2c_write(dev_addr, 0x9a, 0xe0);
	gpioi2c_write(dev_addr, 0x9c, 0x30);
	gpioi2c_write(dev_addr, 0x9d, 0x61);
	gpioi2c_write(dev_addr, 0xa2, 0xa4);
	gpioi2c_write(dev_addr, 0xa3, 0xa4);
	gpioi2c_write(dev_addr, 0xe0, 0xd0);
	gpioi2c_write(dev_addr, 0xf9, 0x0);
	gpioi2c_write(dev_addr, 0x41, 0x10);

}
#endif



static const Cmd Cmds[] = {
	{"Misc"},
	{"gr1", "", NULL, "read a data from a CH7034 chip", dvo1_gpioi2c_read, 1, 5, 0},
	{"gw1", "", NULL, "write a data to a CH704 chip", dvo1_gpioi2c_write, 1, 5, 0},
	{"cfg", "", NULL, "write stream data to a CH7034 chip", gpioi2c_config_ch7034, 1, 5, 0},
	{0, 0}
};

static void init_cmd __P((void)) __attribute__ ((constructor));
static void init_cmd()
{
	cmdlist_expand(Cmds, 1);
}
