/* 
 * huhb@lemote.com 
 * 
 */
#include <stdio.h>
#include <linux/io.h>
#include <dev/pci/pcivar.h>
#include <linux/types.h>
#include <linux/pci.h>

#define I2C_SMBUS_READ  1
#define I2C_SMBUS_WRITE 0
#define I2C_SMBUS_BLOCK_MAX     32

/* PIIX4 SMBus address offsets */
#define SMBHSTSTS	(0 + piix4_smba)
#define SMBHSLVSTS	(1 + piix4_smba)
#define SMBHSTCNT	(2 + piix4_smba)
#define SMBHSTCMD	(3 + piix4_smba)
#define SMBHSTADD	(4 + piix4_smba)
#define SMBHSTDAT0	(5 + piix4_smba)
#define SMBHSTDAT1	(6 + piix4_smba)
#define SMBBLKDAT	(7 + piix4_smba)
#define SMBSLVCNT	(8 + piix4_smba)
#define SMBSHDWCMD	(9 + piix4_smba)
#define SMBSLVEVT	(0xA + piix4_smba)
#define SMBSLVDAT	(0xC + piix4_smba)

/* Other settings */
#define MAX_TIMEOUT	500
#define ENABLE_INT9	0

/* PIIX4 constants */
#define PIIX4_QUICK		0x00
#define PIIX4_BYTE		0x04
#define PIIX4_BYTE_DATA		0x08
#define PIIX4_WORD_DATA		0x0C
#define PIIX4_BLOCK_DATA	0x14

extern void delay(int microseconds);

u32 piix4_smba;
u16 addr = 0x69; /* slave addr */

static int i2c_op(int read_write, u32 addr, u8 index, u8 *block)
{
	u32 len, i, size, temp, timeout, result;

	size = PIIX4_BLOCK_DATA; /* block read or write */
	timeout = 0;
	result = 0;

	linux_outb_p((addr << 1) | read_write,
	       SMBHSTADD); /* addr */
	linux_outb_p(index, SMBHSTCMD); /* offset */
	if (read_write == I2C_SMBUS_WRITE) {
		len = block[0];
		if (len == 0 || len > I2C_SMBUS_BLOCK_MAX)
			return -1;
		linux_outb_p(len, SMBHSTDAT0);/* count */
		i = linux_inb_p(SMBHSTCNT);	/* Reset SMBBLKDAT */
		for (i = 1; i <= len; i++)
			linux_outb_p(block[i], SMBBLKDAT);
	}

	linux_outb_p((size & 0x1C) + (ENABLE_INT9 & 1), SMBHSTCNT); /* control */

	/* Make sure the SMBus host is ready to start transmitting */
	if ((temp = linux_inb_p(SMBHSTSTS)) != 0x00) {
		printf("SMBus busy (%02x). "
			"Resetting...\n", temp);
		linux_outb_p(temp, SMBHSTSTS);
		if ((temp = linux_inb_p(SMBHSTSTS)) != 0x00) {
			printf("Rest Failed! (%02x)\n", temp);
			return -1;
		} else {
			printf("Successful!\n");
		}
	}

	/* start the transaction by setting bit 6 */
	linux_outb_p(linux_inb_p(SMBHSTCNT) | 0x040, SMBHSTCNT);
	delay(2);

	while ((++timeout < MAX_TIMEOUT) &&
	       ((temp = linux_inb_p(SMBHSTSTS)) & 0x01))
		delay(1);

	/* If the SMBus is still busy, we give up */
	if (timeout == MAX_TIMEOUT) {
		printf("SMBus Timeout!\n");
		result = -1;
	}

	if (temp & 0x1c) {
		printf("Error: Failed bus transaction\n");
		result = -1;
	}

	if (linux_inb_p(SMBHSTSTS) != 0x00)
		linux_outb_p(linux_inb_p(SMBHSTSTS), SMBHSTSTS);

	if ((temp = linux_inb_p(SMBHSTSTS)) != 0x00) {
		printf("reset failed (%02x)\n", temp);
	}

	if (read_write == I2C_SMBUS_WRITE)
		return 0;

	block[0] = linux_inb_p(SMBHSTDAT0); /* Default count is 9 bytes */
	if (block[0] == 0 || block[0] > I2C_SMBUS_BLOCK_MAX)
		return -1;
	i = linux_inb_p(SMBHSTCNT);	/* Reset SMBBLKDAT */
	for (i = 1; i <= block[0]; i++)
		block[i] = linux_inb_p(SMBBLKDAT);
	return result;
}

int clock_spread(void)
{
	u32 index;
	u8 block[32];
	pcitag_t dev;
	dev = _pci_make_tag(0, 20, 0); //smbus device

	/* Get smbus base addr */
	piix4_smba = _pci_conf_read(dev, 0x90);
	piix4_smba &= ~0x1;
	printf("piix4_smba 0x%x\n", piix4_smba);

	i2c_op(I2C_SMBUS_READ, addr, 0x0, block);
	block[0] = 12;
	block[12] = 24;
	i2c_op(I2C_SMBUS_WRITE, addr, 0x0, block);

	i2c_op(I2C_SMBUS_READ, addr, 0x0, block);
	block[0] = 24;
	block[1] |= 0x1;
	block[19] |= 0x40; /* PLL1 */
	block[21] |= 0xc0; /* PLL3 */
	block[23] |= 0xc0; /* PLL4 */
	i2c_op(I2C_SMBUS_WRITE, addr, 0x0, block);

	i2c_op(I2C_SMBUS_READ, addr, 0x0, block);
#ifdef DEBUG_I2C
	for(index = 1; index < block[0]; index++)
		printf("block[%d]:0x%x\n", index, block[index]);
#endif
	return 0;
}
#if 0
static void exit_spread(void)
{
	u8 block[32];

	/* disable spread */	
	i2c_op(I2C_SMBUS_READ, addr, 0x0, block);
	block[0] = 12;
	block[12] = 24;
	i2c_op(I2C_SMBUS_WRITE, addr, 0x0, block);

	i2c_op(I2C_SMBUS_READ, addr, 0x0, block);
	block[0] = 24;
	block[1] &= ~0x1;
	block[19] &= ~0x40; /* PLL1 */
	block[21] &= ~0xc0; /* PLL3 */
	block[23] &= ~0xc0; /* PLL4 */
	i2c_op(I2C_SMBUS_WRITE, addr, 0x0, block);
}
#endif
