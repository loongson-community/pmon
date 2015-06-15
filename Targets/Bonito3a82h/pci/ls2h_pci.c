#include <linux/types.h>
#include <types.h>
#include <stdbool.h>
#include "Targets/Bonito3a82h/include/bonito.h"
#include "ls2h.h"
#include "ls2h_int.h"
#include "sys/dev/pci/pcireg.h"
//#include "sys/dev/pci/pcivar.h"

#define PCI_ACCESS_READ  0
#define PCI_ACCESS_WRITE 1
#define PCIBIOS_SUCCESSFUL              0x00
#define PCIBIOS_DEVICE_NOT_FOUND        0x86

#define PCI_CACHE_LINE_SIZE 0x0c    /* 8 bits */
#define PCI_BASE_ADDRESS_0  0x10    /* 32 bits */
#define PCI_BASE_ADDRESS_1  0x14    /* 32 bits [htype 0,1 only] */
#define PCI_BASE_ADDRESS_2  0x18    /* 32 bits [htype 0 only] */
#define PCI_BASE_ADDRESS_3  0x1c    /* 32 bits */
#define PCI_BASE_ADDRESS_4  0x20    /* 32 bits */
#define PCI_BASE_ADDRESS_5  0x24    /* 32 bits */
#define PCI_INTERRUPT_LINE  0x3c    /* 8 bits */
#define  PCI_BASE_ADDRESS_SPACE_IO 0x01
#define  PCI_BASE_ADDRESS_SPACE_MEMORY 0x00
#define  PCI_BASE_ADDRESS_MEM_PREFETCH  0x08    /* prefetchable? */
#define IORESOURCE_IO       0x00000100  /* Resource type */
#define IORESOURCE_MEM      0x00000200
#define IORESOURCE_IRQ      0x00000400
#define IORESOURCE_DMA      0x00000800
#define IORESOURCE_PREFETCH 0x00001000  /* No side effects */
#define PCI_CLASS_BRIDGE_PCI		0x0604

#define PCI_CLASS_REVISION	0x08	/* High 24 bits are class, low 8 revision */
#define PCI_REVISION_ID		0x08	/* Revision ID */
#define PCI_CLASS_PROG		0x09	/* Reg. Level Programming Interface */
#define PCI_CLASS_DEVICE	0x0a	/* Device class */

#define le32_to_cpu(x) (x)
#define le64_to_cpu(x) (x)
#define cpu_to_le32(x) (x)
#define cpu_to_le64(x) (x)


#define CKSEG1ADDR(x) (x|0xa0000000)

#ifdef PCIE_GRAPHIC_CARD
extern bool is_pcie_vga_card();
#endif
typedef u_int32_t pcireg_t;		/* configuration space register XXX */
typedef unsigned long device_t;

u32 _pci_conf_readn(device_t tag, int reg, int width)
{
	tgt_printf("[%s]--ERROR!!\n",__func__);
	return 0;
}

void _pci_conf_writen(device_t tag, int reg, u32 data,int width)
{
	tgt_printf("[%s]-ERROR!!\n",__func__);
	return ;
}

u32 _pci_conf_read(device_t tag, int reg)
{

	u32 val;

	int busnum;
	int device;
	int function;
	int port_num;

	_pci_break_tag (tag, &busnum, &device, &function);
	port_num = busnum / 2;
	ls2h_pcibios_read_port(tag, reg, 4, &val, port_num);

	return  val;
}

u32 _pci_conf_read32(device_t tag,int reg)
{

	u32 val;
	int busnum;
	int device;
	int function;
	int port_num;
	
	_pci_break_tag (tag, &busnum, &device, &function);
	port_num = busnum / 2;

	ls2h_pcibios_read_port(tag, reg, 4, &val,port_num);

	return  val;
}

u32 _pci_conf_read8(device_t tag,int reg)
{

	u32 val;
	int busnum;
	int device;
	int function;
	int port_num;

	_pci_break_tag (tag, &busnum, &device, &function);
	port_num = busnum / 2;

	ls2h_pcibios_read_port(tag, reg, 1, &val,port_num);

	return  (u8)val;
}

u32 _pci_conf_read16(device_t tag,int reg)
{

	u32 val;
	int busnum;
	int device;
	int function;
	int port_num;

	_pci_break_tag (tag, &busnum, &device, &function);
	port_num = busnum / 2;

	ls2h_pcibios_read_port(tag, reg, 2, &val,port_num);

	return  (u16)val;
}

void _pci_conf_write(device_t tag, int reg, u32 data)
{
	int busnum;
	int device;
	int function;
	int port_num;

	_pci_break_tag (tag, &busnum, &device, &function);
	port_num = busnum / 2;

	ls2h_pcibios_write_port(tag, reg, 4, data,port_num);

	return ;
}

void _pci_conf_write32(device_t tag, int reg, u32 data)
{
	int busnum;
	int device;
	int function;
	int port_num;

	_pci_break_tag (tag, &busnum, &device, &function);
	port_num = busnum / 2;
	ls2h_pcibios_write_port(tag, reg, 4, data,port_num);

	return ;
}

void _pci_conf_write8(device_t tag, int reg, u8 data)
{
	int busnum;
	int device;
	int function;
	int port_num;

	_pci_break_tag (tag, &busnum, &device, &function);
	port_num = busnum / 2;

	u32 val = data;
	ls2h_pcibios_write_port(tag, reg, 1, val,port_num);

	return;
}

void _pci_conf_write16(device_t tag, int reg, u16 data)
{
	int busnum;
	int device;
	int function;
	int port_num;

	_pci_break_tag (tag, &busnum, &device, &function);
	port_num = busnum / 2;
	u16 val = data;
	ls2h_pcibios_write_port(tag, reg, 2, val,port_num);
	return ;
}

u32 ls2h_pcie_bar_translate(unsigned char access_type, u32 bar_in, unsigned char portnum)
{
#if 0
	atic unsigned char tag_mem = 0, tag_io = 0;

	if (portnum > LS2H_PCIE_MAX_PORTNUM)
		return bar_in;

	if ((access_type == PCI_ACCESS_WRITE) && LIE_IN_WINDOW(bar_in, \
			LS2H_PCIE_MEM0_DOWN_BASE, LS2H_PCIE_MEM0_DOWN_MASK))
		return MAP_2_WINDOW(bar_in, LS2H_PCIE_MEM0_UP_BASE,
				LS2H_PCIE_MEM0_UP_MASK);
	//it's a little tricky here

	if ((access_type == PCI_ACCESS_READ) && LIE_IN_WINDOW(bar_in, \
		LS2H_PCIE_MEM0_UP_BASE, LS2H_PCIE_MEM0_UP_MASK)) {
		if (tag_mem)
			return MAP_2_WINDOW(bar_in, LS2H_PCIE_MEM0_BASE_PORT(portnum),
					LS2H_PCIE_MEM0_UP_MASK);
		else {
			tag_mem = 1;
			return bar_in;
		}
	}

	if ((access_type == PCI_ACCESS_WRITE) && LIE_IN_WINDOW(bar_in, \
			LS2H_PCIE_IO_DOWN_BASE, LS2H_PCIE_IO_DOWN_MASK))
		return MAP_2_WINDOW(bar_in, LS2H_PCIE_IO_UP_BASE,
			LS2H_PCIE_IO_UP_MASK);

	if ((access_type == PCI_ACCESS_READ) && LIE_IN_WINDOW(bar_in, \
			LS2H_PCIE_IO_UP_BASE, LS2H_PCIE_IO_UP_MASK) &&
			(bar_in & PCI_BASE_ADDRESS_SPACE_IO)) {
		if (tag_io)
			return bar_in;
		else {
			tag_io = 1;
			return bar_in;
		}
	}
#endif
	return bar_in;
}
void cfg_device_read(
		unsigned int type, unsigned int bus_num,
		unsigned int dev_num, unsigned int func_num,
		unsigned int reg_id, unsigned int * read_data,
		unsigned int port_id
		)
{
	unsigned int port_ctrl = CKSEG1ADDR(LS2H_PCIE_REG_BASE_PORT(port_id));
	unsigned int port_cfg = CKSEG1ADDR(LS2H_PCIE_DEV_HEAD_BASE_PORT(port_id));
	*(volatile unsigned char *)( port_ctrl + 0x24) = (type&0x1) | (bus_num<< 24) | (dev_num <<19) | (func_num <<16);
	//*(read_data) = *(volatile unsigned int *)( port_cfg + (reg_id<<2));
	*(read_data) = *(volatile unsigned int *)( port_cfg + (reg_id));
}
void cfg_device_write(
		unsigned int type, unsigned int bus_num,
		unsigned int dev_num, unsigned int func_num,
		unsigned int reg_id, unsigned int write_data,
		unsigned int port_id
		)
{
	unsigned int port_ctrl = CKSEG1ADDR(LS2H_PCIE_REG_BASE_PORT(port_id));
	unsigned int port_cfg = CKSEG1ADDR(LS2H_PCIE_DEV_HEAD_BASE_PORT(port_id));
	*(volatile unsigned char *)( port_ctrl + 0x24) = (type&0x1) | (bus_num << 24) | (dev_num <<19) | (func_num <<16);
	//*(volatile unsigned int *)( port_cfg + (reg_id<<2)) = write_data;
	*(volatile unsigned int *)( port_cfg + (reg_id)) = write_data;
}

int ls2h_pci_config_access(unsigned char access_type,
   device_t tag, int where, u32 * data, unsigned char  portnum)
{

	int busnum ;
	u32 addr, type;
	u32 addr_i, cfg_addr, reg_data;
	u32 datarp;
	void *addrp;
	int device;
	int function;
	int reg = where & ~3;
	unsigned char need_bar_translate = 0;
	static int mytmp = 0;

	_pci_break_tag (tag, &busnum, &device, &function);

	if (portnum > LS2H_PCIE_MAX_PORTNUM)
		return PCIBIOS_DEVICE_NOT_FOUND;

	// if (!bus->parent) {
	if(!(busnum % 2)){
		/* in-chip virtual-bus has no parent,
		    so access is routed to PORT_HEAD */
		if (device > 0 || function > 0) {
			*data = -1; /* only one Controller lay on a virtual-bus */
			return PCIBIOS_DEVICE_NOT_FOUND;
		}
		else {
			addr = LS2H_PCIE_PORT_HEAD_BASE_PORT(portnum) | reg;
			if (reg == PCI_BASE_ADDRESS_0)
		        /* the default value of PCI_BASE_ADDRESS_0 of
		            PORT_HEAD is wrong, use PCI_BASE_ADDESS_1 instead */
		        addr += 4;
		}
	} else {
		/* offboard PCIE-bus has parent,
		 so access is routed to DEV_HEAD */
		/* check if LTSSM of controller link-up */

		addr_i = LS2H_PCIE_REG_BASE_PORT(portnum)
		            | LS2H_PCIE_PORT_REG_STAT1;
		addrp = (void *)CKSEG1ADDR(addr_i);

		reg_data = le32_to_cpu(*(volatile unsigned int *)addrp);
		if (busnum > 255 || device > 31 || function > 1
			    || !(reg_data & LS2H_PCIE_REG_STAT1_BIT_LINKUP)) {
		    *data = -1;	/* link is not up at all  */
		    return PCIBIOS_DEVICE_NOT_FOUND;
		}

		//if (!bus->parent->parent) {
		if(1){
			/* the bus is child of virtual-bus(pcie slot),
			so use Type 0 access for device on it */
			if (device > 0) {
			    *data = -1;	/* only one device on PCIE slot */
			    return PCIBIOS_DEVICE_NOT_FOUND;
			}
			type = 0;
		} else {
			/* the bus is emitted from offboard-bridge,
			    so use Type 1 access for device on it */
			type = 1;
		}

		/* write busnum/devnum/funcnum/type into PCIE_REG_BASE +0x24 */
		addr_i = LS2H_PCIE_REG_BASE_PORT(portnum)
			| LS2H_PCIE_PORT_REG_CFGADDR;
		cfg_addr = (busnum << 16) | (device << 11) | (function << 8) | type;
		addrp = (void *)CKSEG1ADDR(addr_i);
		*(volatile unsigned int *)addrp = cpu_to_le32(cfg_addr);

		/* access mapping memory instead of direct configuration access */
		addr = LS2H_PCIE_DEV_HEAD_BASE_PORT(portnum) | reg;

		if (reg >= PCI_BASE_ADDRESS_0 && reg <= PCI_BASE_ADDRESS_5)
			/* Base Address Register need to be translated */
			need_bar_translate = 1;
	}

	addrp = (void *)CKSEG1ADDR(addr);
	if (access_type == PCI_ACCESS_WRITE) {
		if (need_bar_translate) {
			datarp = ls2h_pcie_bar_translate(PCI_ACCESS_WRITE, *data, portnum);
		} else
			datarp = *data;
		*(volatile unsigned int *)addrp = cpu_to_le32(datarp);
	} else {
		datarp = le32_to_cpu(*(volatile unsigned int *)addrp);
		if (need_bar_translate) {
			*data = ls2h_pcie_bar_translate(PCI_ACCESS_READ, datarp, portnum);
		} else
			*data = datarp;
	}

	return PCIBIOS_SUCCESSFUL;
}
int ls2h_pcibios_read_port(device_t tag, int where, int size, u32 * val,int port_num)
{
	u32 data = 0;
	*val = -1;

	if (ls2h_pci_config_access(PCI_ACCESS_READ, tag, where, &data, port_num))
		return PCIBIOS_DEVICE_NOT_FOUND;

	if (size == 1)
		*val = (data >> ((where & 3) << 3)) & 0xff;
	else if (size == 2)
		*val = (data >> ((where & 3) << 3)) & 0xffff;
	else
		*val = data;

	return PCIBIOS_SUCCESSFUL;
}


int ls2h_pcibios_write_port(device_t tag, int where, int size, u32 val,int port_num)
{
	u32 data = 0;

	if (size == 4)
		data = val;
	else {
		if (ls2h_pci_config_access(PCI_ACCESS_READ, tag, where, &data, port_num))
			return PCIBIOS_DEVICE_NOT_FOUND;

		if (size == 1)
			data = (data & ~(0xff << ((where & 3) << 3))) |
					(val << ((where & 3) << 3));
		else if (size == 2)
			data = (data & ~(0xffff << ((where & 3) << 3))) |
					(val << ((where & 3) << 3));
	}

	if (ls2h_pci_config_access(PCI_ACCESS_WRITE, tag,  where, &data, port_num))
		return PCIBIOS_DEVICE_NOT_FOUND;

	return PCIBIOS_SUCCESSFUL;
}

static void en_ref_clock(void)
{
	unsigned int data;

	data = ls2h_readl(LS2H_CLK_CTRL3_REG);
	data |= (LS2H_CLK_CTRL3_BIT_PEREF_EN(0)
		 | LS2H_CLK_CTRL3_BIT_PEREF_EN(1)
		 | LS2H_CLK_CTRL3_BIT_PEREF_EN(2)
		 | LS2H_CLK_CTRL3_BIT_PEREF_EN(3));
	ls2h_writel(data, LS2H_CLK_CTRL3_REG);
}

static int is_rc_mode(void)
{
	unsigned data;

	data = ls2h_readl(LS2H_PCIE_REG_BASE_PORT(0)
			| LS2H_PCIE_PORT_REG_CTR_STAT);

	return data & LS2H_PCIE_REG_CTR_STAT_BIT_ISRC;
}

int is_x4_mode(void)
{
	unsigned data;

	data = ls2h_readl(LS2H_PCIE_REG_BASE_PORT(0)
			| LS2H_PCIE_PORT_REG_CTR_STAT);

	return data & LS2H_PCIE_REG_CTR_STAT_BIT_ISX4;
}

void ls2h_pcie_port_init(int port)
{
	unsigned reg, data;

	reg = LS2H_PCIE_PORT_HEAD_BASE_PORT(port) | 0x7c;
	data = ls2h_readl(reg);
	data &= ~0xf;
	data |=1;
	ls2h_writel(data, reg);

	reg = LS2H_PCIE_REG_BASE_PORT(port) | LS2H_PCIE_PORT_REG_CTR0;
	ls2h_writel(0xff204c, reg);

	reg = LS2H_PCIE_PORT_HEAD_BASE_PORT(port) | PCI_CLASS_REVISION;
	data = ls2h_readl(reg);
	data &= 0xffff;
	data |= (PCI_CLASS_BRIDGE_PCI << 16);
	ls2h_writel(data, reg);
}

int ls2h_pcibios_init(void)
{
	tgt_printf("arch_initcall:pcibios_init\n");
	en_ref_clock();

	if (!is_rc_mode())
		return 0;

	ls2h_pcie_port_init(0);

#ifdef PCIE_GRAPHIC_CARD
	if ( is_x4_mode() || is_pcie_vga_card() )
#else
	if (is_x4_mode())
#endif
	{
		tgt_printf(" x4 mode\n");
		return 0;
	}
	ls2h_pcie_port_init(1);

	ls2h_pcie_port_init(2);

	ls2h_pcie_port_init(3);

	return 0;
}
