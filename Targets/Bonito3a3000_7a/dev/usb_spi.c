#include <sys/linux/types.h>
#include <pmon.h>
#include <stdio.h>
#include <machine/pio.h>

#include "usb_spi.h"

#define EXRIR 0xec
#define EXRCR 0xf0
#define FWDCS 0xf4
#define EXRACS  0xf6
#define DATA0 0xf8
#define DATA1 0xfc
#define BASE 0xbb020000 //bus 2 ,dev 0,func 0

//#define USBSPI_DEBUG

#define readl(a) *(volatile unsigned int*)(a + BASE)

int usb_spi_erase(void)
{

	unsigned int tmp;

	tmp = readl(FWDCS);
	tmp &= (1 << 31);
	if (!tmp) {
		printf("no external rom exists!\n");
		return -1;
	}
	readl(DATA0) = 0x5a65726f;
	readl(FWDCS) |= (1 << 17);
	tmp = readl(FWDCS);
	while ((tmp & (1 << 17))) {
		tmp = readl(FWDCS);
	}
	printf("erase down.\n");
	return 0;
}

int usb_spi_prepare(void)
{
	unsigned int tmp;

	tmp = readl(FWDCS);
	tmp &= (1 << 31);
	if (!tmp) {
		printf("no external rom exists!\n");
		return -1;
	}
	readl(DATA0) = 0x53524f4d;
	readl(FWDCS) |= (1 << 16);
	tmp = readl(FWDCS);
	tmp &= (7 << 20);
	if (tmp & (2 << 20)) {
		printf("result code error %x\n",tmp);
		return -1;
	}
	return 0;
}

/*sie minimum value is 4*/
int usb_spi_write(int size,unsigned int *spi_buf)
{
	unsigned int *buf = spi_buf;
	unsigned int tmp, times = 10000;

	usb_spi_prepare();

	size /= 2;
	tmp = readl(FWDCS);
	tmp &= (1 << 24);
	if (tmp) {
		printf("set data0 status error.\n");
		return 1;
	}
	readl(DATA0) = *buf++;
	tmp = readl(FWDCS);
	tmp &= (2 << 24);
	if (tmp) {
		printf("set data1 status error.\n");
		return 1;
	}
	readl(DATA1) = *buf++;
	readl(FWDCS) |= (3 << 24);
	size -= 1;
	while(size--) {
		tmp = readl(FWDCS);
		while ((tmp & (1 << 24)) && times--) {
			delay(10);
			tmp = readl(FWDCS);
		}
		if (tmp & (1 << 24)) {
			printf(">> set data0 status error. %x\n",tmp);
			return 1;
		}
		readl(DATA0) = *buf++;
		readl(FWDCS) |= (1 << 24);

		times = 10000;
		tmp = readl(FWDCS);
		while ((tmp & (2 << 24)) && times--) {
			delay(10);
			tmp = readl(FWDCS);
		}
		if (tmp & (2 << 24)) {
			printf(">> set data0 status error. %x\n",tmp);
			return 1;
		}
		readl(DATA1) = *buf++;
		readl(FWDCS) |= (2 << 24);
	}
	readl(FWDCS) &= ~(1 << 16);
	times = 10000;
	tmp = readl(FWDCS);
	while (!(tmp & (1 << 20)) && times--) {
		delay(10);
		tmp = readl(FWDCS);
	}
	if (tmp & (2 << 20)) {
		printf("write result code is %x\n",tmp);
		return 1;
	}
	printf("Fw write done.\n");
	return size;
}

int usb_spi_read(int size,unsigned int *ret_buf)
{
	unsigned int tmp, times = 100;
	unsigned int *buf = ret_buf;

	usb_spi_prepare();
	readl(FWDCS) |= (3 << 26);
	size /= 2;

	while(size--) {
		tmp = readl(FWDCS);
		while ((tmp & (1 << 26)) && times--) {
			tmp = readl(FWDCS);
		}
		if (tmp & (1 << 26)) {
			printf("get data0 now is %x\n",tmp);
			return -1;
		}
		*buf++ = readl(DATA0);
#ifdef USBSPI_DEBUG
		tmp = readl(DATA0);
		printf("0x%08x ",tmp);
#endif
		readl(FWDCS) |= (1 << 26);

		tmp = readl(FWDCS);
		times = 100;
		while ((tmp & (2 << 26)) && times--) {
			tmp = readl(FWDCS);
		}
		if (tmp & (2 << 26)) {
			printf("get data1 now is %x\n",tmp);
			return -1;
		}
		*buf++ = readl(DATA1);
#ifdef USBSPI_DEBUG
		tmp = readl(DATA1);
		printf("0x%08x ",tmp);
#endif
		readl(FWDCS) |= (2 << 26);
#ifdef USBSPI_DEBUG
		if (!(size % 5))
			printf("\n");
#endif
	}
	readl(FWDCS) &= ~(1 << 16);
	return size;
}

int usb_spi_init(void)
{
	unsigned int ret_buf[20];
	int  i, size = 20;

	i = usb_spi_read(size,ret_buf);
	if (i != -1) {
		printf("usb spi read error i = %d.\n",i);
		return -1;
	}

	for (i = 0;i < size;i++) {
		if (ret_buf[i] != usb_spi_buf[i]) {
			printf(">> 0x%08x ",ret_buf[i]);
			printf("-- 0x%08x ",usb_spi_buf[i]);
			printf("\n");
			break;
		}
	}
	if (i != size) {
		usb_spi_erase();
		size = 3260;//FW size
		usb_spi_write(size,usb_spi_buf);
	}
	return 0;
}
static const Cmd Cmds[] = {
	{"Misc"},
	{"usb_spi_read", "", NULL, "read the usb spi data", usb_spi_read, 1, 5, 0},
	{"usb_spi_write", "", NULL, "read the usb spi data", usb_spi_write, 1, 5, 0},
	{"usb_spi_erase", "", NULL, "read the usb spi data", usb_spi_erase, 1, 5, 0},
	{"usb_spi_init", "", NULL, "read the usb spi data", usb_spi_init, 1, 5, 0},
	{0, 0}
};

static void init_cmd __P((void)) __attribute__ ((constructor));
static void init_cmd()
{
	cmdlist_expand(Cmds, 1);
}
