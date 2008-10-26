#include <sys/param.h>
#include <stdio.h>
#include <stdarg.h>
#include <progress.h>
#include <sys/device.h>
#include <sys/malloc.h>
#include <machine/bus.h>
#include <include/pmon_target.h>
#include <linux/io.h>

#include "pcivar.h"
#include "pcireg.h"

#define IRQ_ROUTE_REG1 0x51
#define IRQ_ROUTE_REG2 0x52
#define IRQ_ROUTE_REG4 0x55
#define IRQ_ROUTE_REG5 0x56
#define IRQ_ROUTE_REG6 0x57
#define PCI_IRQ_TYPE_REG 0x54
#define IRQ(x) x
#define PARALLEL_IRQ 	(IRQ(7)<<4)
#define FLOPPY_IRQ 		(IRQ(6))
#define COM1_IRQ 		(IRQ(4))
#define COM2_IRQ 		(IRQ(3)<<4)
#define PCIA_IRQ		(IRQ(10)<<4)
#define PCIB_IRQ		(IRQ(11))
#define PCIC_IRQ		(IRQ(12)<<4)
#define PCID_IRQ		(IRQ(13)<<4)

static void initIRQ(void)
{
	pcitag_t tag;
	char val;
	tag=_pci_make_tag(VTSB_BUS,VTSB_DEV, VTSB_ISA_FUNC);
	
	_pci_conf_writen(tag,IRQ_ROUTE_REG1,PARALLEL_IRQ|FLOPPY_IRQ,1);	
	
	_pci_conf_writen(tag,IRQ_ROUTE_REG2,COM2_IRQ|COM1_IRQ,1);	
	
	val=_pci_conf_readn(tag,IRQ_ROUTE_REG4,1);
	val &=0xf;
	val |=PCIA_IRQ;
	_pci_conf_writen(tag,IRQ_ROUTE_REG4,val,1);	
	
	_pci_conf_writen(tag,IRQ_ROUTE_REG5,PCIC_IRQ|PCIB_IRQ,1);	
	
	val=_pci_conf_readn(tag,IRQ_ROUTE_REG6,1);
	val &=0xf0;
	val |=PCID_IRQ;
	_pci_conf_writen(tag,IRQ_ROUTE_REG6,val,1);	
	
	val=_pci_conf_readn(tag,PCI_IRQ_TYPE_REG,1);
	val &= 0xf0;
	_pci_conf_writen(tag,PCI_IRQ_TYPE_REG,PARALLEL_IRQ|FLOPPY_IRQ,1);	
}

#define IDE_CHIPEN_REG 0x40
#define IDE_CFG_REG 0x41
static void initIDE(void)
{
	pcitag_t tag;
	char val;
	/*南桥外设默认都是非使能状态,我们这里将其一一使能*/
	/*硬盘使能*/
#if 1
		/* IDE controller enable */
		tag=_pci_make_tag(VTSB_BUS,VTSB_DEV, VTSB_ISA_FUNC);
		val=_pci_conf_readn(tag,0x48,1);
		val=val & ~2;
		_pci_conf_writen(tag,0x48,val,1);
		
		/* IDE IRQ Route */
		val=_pci_conf_readn(tag,0x4a,1);
		val=(val&0xf0)|0x4;
		_pci_conf_writen(tag,0x4a,val,1);
#endif

		tag=_pci_make_tag(VTSB_BUS,VTSB_DEV, VTSB_IDE_FUNC);

		/* enable IO space */
		_pci_conf_writen(tag,0x04,7,1);	
		/* set to compatible mode */
		_pci_conf_writen(tag,0x09,0x8A,1);	
		/* latency */
		_pci_conf_writen(tag,0x0d,0xd0,1);	
		/* set to legacy interrupt */
		_pci_conf_writen(tag,0x3d,0x00,1);	

		/* enable primary/secondary channel */
		_pci_conf_writen(tag,0x40,0xb,0x1);	
		/* disable prefetch buffer & post write buffer */
		_pci_conf_writen(tag,0x41,0x2,0x1);	
		_pci_conf_writen(tag,0x43,0xa,0x1);	

		/* set zero wait state for master read/write
		 * to make ict nb happy 
		 */
		_pci_conf_writen(tag,0x44,0x0,1);	

		/* disable memory read multiple/memory write and invalidate
		 */
		_pci_conf_writen(tag,0x45,0x0,1);	
#if 1
		_pci_conf_writen(tag, 0x10, 0x1f1,4);
		_pci_conf_writen(tag, 0x14, 0x3f5,4);
		_pci_conf_writen(tag, 0x18, 0x171,4);
		_pci_conf_writen(tag, 0x1c, 0x375,4);
		_pci_conf_writen(tag, 0x20, 0xcc1,4);
#endif

}

#define SUPERIO_CFG_REG 0x85
static void initSerial(void)
{
	pcitag_t tag;
	char confval,val;
	/*使能串口
	 * 这个需要在汇编代码serialinit中设置
	 * */
#define E2_EPP 2
#define E2_S1 (1<<2)
#define E2_S2 (1<<3)
#define E2_FLOPPY (1<<4)
	/*配置super io*/
	tag=_pci_make_tag(VTSB_BUS,VTSB_DEV, VTSB_ISA_FUNC);
	confval=_pci_conf_readn(tag,SUPERIO_CFG_REG,1);
	_pci_conf_writen(tag,SUPERIO_CFG_REG,confval|2,1);	
#ifdef HIGH_SPEED_SERIAL
	linux_outb(0xee,0x3f0);
	val=linux_inb(0x3f1);
	linux_outb(val|0xc0,0x3f1); /* both ports on high speed*/
#endif
	
#if 0
	outb(PCI_IO_SPACE_BASE+0x3f0,0xe7);
	outb(PCI_IO_SPACE_BASE+0x3f1,(COM1_BASE_ADDR-PCI_IO_SPACE_BASE)>>2); /* com1 serial base address*/

	outb(PCI_IO_SPACE_BASE+0x3f0,0xe8);
	outb(PCI_IO_SPACE_BASE+0x3f1,(COM2_BASE_ADDR-PCI_IO_SPACE_BASE)>>2); /* com2 serial base address*/
#endif

	linux_outb(0xe2,0x3f0);
	val=linux_inb(0x3f1);
	linux_outb(val|E2_S2|E2_S1,0x3f1);
	_pci_conf_writen(tag,SUPERIO_CFG_REG,confval,1);	

	printf("0x3f8=%x\n",linux_inb(0x3f0));
}

static void initFloppy(void)
{
	pcitag_t tag;
	char confval,val;
	tag=_pci_make_tag(VTSB_BUS,VTSB_DEV, VTSB_ISA_FUNC);
	confval=_pci_conf_readn(tag,SUPERIO_CFG_REG,1);
	_pci_conf_writen(tag,SUPERIO_CFG_REG,confval|2,1);	
	linux_outb(0xe2,0x3f0);
	val=linux_inb(0x3f1);
	linux_outb(val|E2_FLOPPY,0x3f1);
	_pci_conf_writen(tag,SUPERIO_CFG_REG,confval,1);
}

static void init_keyboard(void)
{
	pcitag_t tag;
	tag=_pci_make_tag(VTSB_BUS,VTSB_DEV, VTSB_ISA_FUNC);
	_pci_conf_writen(tag,0x5a,0xff,1);	
}

static void disable_usb(void)
{
	pcitag_t tag;
	char val;
	tag=_pci_make_tag(VTSB_BUS,VTSB_DEV, VTSB_ISA_FUNC);
	val=_pci_conf_readn(tag,0x48,1);	
	_pci_conf_writen(tag,0x48,val|4,1);	
	val=_pci_conf_readn(tag,0x85,1);	
	_pci_conf_writen(tag,0x85,val|0x10,1);	
}
static void enable_io_decode(void)
{
	pcitag_t tag;
	char val;
	tag=_pci_make_tag(VTSB_BUS,VTSB_DEV, VTSB_ISA_FUNC);
	/*enable on-board io*/
	val=_pci_conf_readn(tag,0x81,1);	
	_pci_conf_writen(tag,0x81,val|0x80,1);	
	/*enable com1 and com2*/	
	_pci_conf_writen(tag,0x83,0x80|0x1| 0x8,1);	
}

static void nvram_on(void)
{
	pcitag_t tag;
	tag=_pci_make_tag(VTSB_BUS,VTSB_DEV, VTSB_ISA_FUNC);
	/* ??? */
	_pci_conf_writen(tag,0x43,0xc0,1);	
}

static void initUSB(void)
{
}

void vt82c686_init(void)
{
	initSerial();
	init_keyboard();
	initIDE();	
	initIRQ();
	//disable_usb();
	enable_io_decode();
	initUSB();
}

