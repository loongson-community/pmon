#include <pmon.h>
#include <linux/types.h>
void i2c_init(int speed,  int slaveaddr);
int i2c_read(u8 chip, uint addr, int alen, u8 *buffer, int len);
int i2c_write(u8 chip, uint addr, int alen, u8 *buffer, int len);

#define CHIP_9022_ADDR	(0x72/2)

void config_sii9022a(void)
{
	unsigned char id0;
	unsigned char id1;
	unsigned char id2;
	unsigned char data;

	unsigned char dev_addr = CHIP_9022_ADDR;

#define  gpioi2c_write(chip, reg, val) data = val; i2c_write(chip, reg, 1, &data, 1);
#define  gpioi2c_read(chip, reg, data) i2c_read(chip, reg, 1, data, 1);

	ls2k_i2c_init(0, 0xbfe01800);

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
	gpioi2c_write(dev_addr, 0x1a, data);
}
