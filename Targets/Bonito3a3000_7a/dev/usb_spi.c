#include <sys/linux/types.h>
#include <pmon.h>
#include <stdio.h>
#include <machine/pio.h>

#include "sys/dev/pci/pcireg.h"
#include "sys/dev/pci/pcivar.h"
#include "usb_spi.h"

#define EXRIR 0xec
#define EXRCR 0xf0
#define FWDCS 0xf4
#define EXRACS  0xf6
#define DATA0 0xf8
#define DATA1 0xfc

//#define USBSPI_DEBUG

unsigned int Base = 0xbb020000;//bus 2 ,dev 0,func 0

#define readl(a) *(volatile unsigned int*)(a + Base)

int usb_spi_erase(void)
{

	unsigned int tmp,times = 100000;
	if (!(readl(FWDCS) & (1 << 31))) {
		printf("no external rom exists!\n");
		return 1;
	}

	readl(DATA0) = 0x5a65726f;
	readl(FWDCS) |= (1 << 17);
	while ((readl(FWDCS) & (1 << 17)) && times-- );
	if (!times){
		printf("erase error.\n");
		return 1;
	}

	printf("erase down.\n");
	return 0;
}

int usb_spi_prepare(void)
{
	unsigned int tmp;

	if (!(readl(FWDCS) & (1 << 31))) {
		printf("no external rom exists!\n");
		return 1;
	}
	readl(DATA0) = 0x53524f4d;
	readl(FWDCS) |= (1 << 16);
	if ((readl(FWDCS) & (0x7<<20)) != 0x0) {
		printf("result code error %x\n",readl(FWDCS));
		return 1;
	}

	return 0;
}

/*sie minimum value is 4*/
int usb_spi_write(int size,unsigned int *spi_buf)
{
	unsigned int *buf = spi_buf;
	unsigned int tmp, times = 100000000;

	if (usb_spi_prepare())
		return 1;

	size /= 2;
	if (readl(FWDCS) & (1 << 24)) {
		printf("set data0 status error.\n");
		return 1;
	}
	readl(DATA0) = *buf++;
	if (readl(FWDCS) & (2 << 24)) {
		printf("set data1 status error.\n");
		return 1;
	}
	readl(DATA1) = *buf++;
	readl(FWDCS) |= (3 << 24);
	size -= 1;
	while(size--) {
		times = 100000000;
		while ((readl(FWDCS) & (1 << 24)) && times--);
		if (!times) {
			printf(">> set data0 status error. \n");
			return 1;
		}
		readl(DATA0) = *buf++;
		readl(FWDCS) |= (1 << 24);

		times = 100000000;
		while ((readl(FWDCS) & (2 << 24)) && times--);
		if (!times) {
			printf(">> set data0 status error. \n");
			return 1;
		}
		readl(DATA1) = *buf++;
		readl(FWDCS) |= (2 << 24);
	}
	times = 100000000;
	while ((readl(FWDCS) & (3 << 24)) && times--);
	if (!times) {
		printf("error\n");
		return 1;
	}

	readl(FWDCS) &= ~(1 << 16);
	times = 100000;
	while (((readl(FWDCS) & (0x7<<20)) != (0x1<<20)) && times--);
	if (!times) {
		printf("write result code is %x\n",readl(FWDCS));
		return 1;
	}
	printf("Fw write done.\n");

	readl(FWDCS) |= (1 << 18);
	times = 100000;
	while (readl(FWDCS) & (1<< 18) && times--);
	if (!times) {
		printf("reload error \n");
		return 1;
	}
	return size;
}

int usb_spi_download(int size,unsigned int *spi_buf)
{
	unsigned int *buf = spi_buf;
	unsigned int tmp, times = 100000000;
	readl(FWDCS) |= 1;
	size /= 2;
	if (readl(FWDCS) & (1 << 8)) {
		printf("set data0 status error.\n");
		return 1;
	}
	readl(DATA0) = *buf++;
	if (readl(FWDCS) & (2 << 8)) {
		printf("set data1 status error.\n");
		return 1;
	}
	readl(DATA1) = *buf++;
	readl(FWDCS) |= (3 << 8);
	size -= 1;
	while(size--) {
		times = 100000000;
		while ((readl(FWDCS) & (1 << 8)) && times--);
		if (!times) {
			printf(">> set data0 status error. \n");
			return 1;
		}
		readl(DATA0) = *buf++;
		readl(FWDCS) |= (1 << 8);

		times = 100000000;
		while ((readl(FWDCS) & (2 << 8)) && times--);
		if (!times) {
			printf(">> set data0 status error. \n");
			return 1;
		}
		readl(DATA1) = *buf++;
		readl(FWDCS) |= (2 << 8);
	}
	times = 100000000;
	while ((readl(FWDCS) & (3 << 8)) && times--);
	if (!times) {
		printf("error\n");
		return 1;
	}

	readl(FWDCS) &= ~1;
	times = 100000;
	while (((readl(FWDCS) & (0x7<<4)) != (0x1<<4)) && times--);
	if (!times) {
		printf("write result code is %x\n",readl(FWDCS));
		return 1;
	}
	printf("Fw download done.\n");

	return size;
}

int usb_spi_read(int size,unsigned int *ret_buf)
{
	unsigned int tmp, times = 100000000;
	unsigned int *buf = ret_buf;
	if (usb_spi_prepare()) {
		return 1;
	}
	readl(FWDCS) |= (3 << 26);
	size /= 2;

	while(size--) {
		while ((readl(FWDCS) & (1 << 26)) && times--);
		if (!times) {
			printf("error get data0 now is %x\n",readl(FWDCS));
			return 1;
		}
		*buf++ = readl(DATA0);
#ifdef USBSPI_DEBUG
		printf("0x%08x ",readl(DATA0));
#endif
		readl(FWDCS) |= (1 << 26);

		times = 100000000;
		while ((readl(FWDCS) & (2 << 26)) && times--);
		if (!times) {
			printf("error get data1 now is %x\n",readl(FWDCS));
			return 1;
		}
		*buf++ = readl(DATA1);
#ifdef USBSPI_DEBUG
		printf("0x%08x ",readl(DATA1));
#endif
		readl(FWDCS) |= (2 << 26);
#ifdef USBSPI_DEBUG
		if (!(size % 5))
			printf("\n");
#endif
	}
    times = 100000000;
    while ((readl(FWDCS) & (3 << 26)) && times--);
    if (!times) {
    	printf("error\n");
    	return 1;
    }

    readl(FWDCS) &= ~(1 << 16);
	return size;
}

int _find_xhci_pci_base(struct pci_device *parent)
{
	struct pci_device *pd;
	int cnt = 0;

	for (pd = parent->bridge.child; pd ; pd = pd->next) {
		pcitag_t tag = pd->pa.pa_tag;
		pcireg_t id,class;
		class = _pci_conf_read(tag, PCI_CLASS_REG);
		id = _pci_conf_read(tag, PCI_ID_REG);

		if(id == 0x00141912){
			Base = (0xbb000000 | tag );
//			printf("xhci base %x\n",Base);
			 _usb_spi_init();
			cnt++;
		}

		if((PCI_CLASS(class) == PCI_CLASS_BRIDGE && PCI_SUBCLASS(class) == PCI_SUBCLASS_BRIDGE_PCI) || pd->bridge.child != NULL) {
			cnt +=_find_xhci_pci_base(pd);
		}
	}

	return cnt;

}

int _usb_spi_init()
{
	unsigned int ret_buf[20];
	int  i, size = 20;
	i = usb_spi_read(size,ret_buf);
	if (i != -1) {
		printf("usb spi read error i = %d.\n",i);
		goto out;
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
    else {
		printf("usb firmware no error\n");
    }

out:
		size = 3260;//FW size
		usb_spi_download(size,usb_spi_buf);
}

int find_xhci_pci_base()
{
	int i;
	struct pci_device *pd;

	extern struct pci_device *_pci_head;
	extern int pci_roots;
	int cnt = 0;

	for(i = 0, pd = _pci_head; i < pci_roots; i++, pd = pd->next) {
		cnt += _find_xhci_pci_base (pd);
	}
	return cnt;
}


int usb_spi_init(void)
{

	if(!find_xhci_pci_base()){
		printf("can't find xhci\n");
		return 0;
	}

	return 0;
}


int cmd_usb_spi_read(ac, av)
    int ac;
    char *av[];
{
	unsigned int ret_buf[3260];
	int  i, size = 50;
	if(ac>2)
	{
	   int tag = strtoul(av[2], 0, 0);
		Base = (0xbb000000 | tag );
	}
	size = (int) strtoul(av[1],0,0);
	printf("size= %x \n",size);
	if (size > 3260){
		printf("max is 3260\n");
		return 1;
	}
	if ((size % 2)){
		printf("it must be even number\n");
		return 1;
	}
	if (usb_spi_read(size,ret_buf) != -1) {
		printf("usb spi read error.\n");
		return 1;
	}

	for (i = 0;i < size;i++) {
		printf(">> 0x%08x ",ret_buf[i]);
		printf("-- 0x%08x ",usb_spi_buf[i]);
		printf("\n");
	}
}


int cmd_usb_spi_write(int ac, char **av)
{
	if(ac>1)
	{
	   int tag = strtoul(av[1], 0, 0);
		Base = (0xbb000000 | tag );
	}
		usb_spi_write(3260,usb_spi_buf);
 return 0;
}

int cmd_usb_spi_erase(int ac, char **av)
{
	if(ac>1)
	{
	   int tag = strtoul(av[1], 0, 0);
		Base = (0xbb000000 | tag );
	}
	usb_spi_erase();
	return 0;
}

static const Cmd Cmds[] = {
	{"Misc"},
	{"usb_spi_read", "", NULL, "read the usb spi data", cmd_usb_spi_read, 1, 5, 0},
	{"usb_spi_write", "", NULL, "read the usb spi data", cmd_usb_spi_write, 1, 5, 0},
	{"usb_spi_erase", "", NULL, "read the usb spi data", cmd_usb_spi_erase, 1, 5, 0},
	{"usb_spi_init", "", NULL, "read the usb spi data", usb_spi_init, 1, 5, 0},
	{0, 0}
};

static void init_cmd __P((void)) __attribute__ ((constructor));
static void init_cmd()
{
	cmdlist_expand(Cmds, 1);
}
