#include "sb700.h"
#include "rs780_cmn.h"

#define NULL (void*)0

void set_sm_enable_bits(device_t sm_dev, u32 reg_pos, u32 mask, u32 val)
{
	u32 reg_old, reg;
	reg = reg_old = pci_read_config32(sm_dev, reg_pos);
	reg &= ~mask;
	reg |= val;
	if (reg != reg_old) {
		pci_write_config32(sm_dev, reg_pos, reg);
	}
}

static void pmio_write_index(u32 port_base, u8 reg, u8 value)
{
	OUTB(reg, port_base);
	OUTB(value, port_base + 1);
}

static u8 pmio_read_index(u32 port_base, u8 reg)
{
	OUTB(reg, port_base);
	return INB(port_base + 1);
}

void pm_iowrite(u8 reg, u8 value)
{
	printk_info("pm_iowrite\n");
	pmio_write_index(PM_INDEX, reg, value);
}

u8 pm_ioread(u8 reg)
{
	printk_info("pm_ioread\n");
	return pmio_read_index(PM_INDEX, reg);
}

void pm2_iowrite(u8 reg, u8 value)
{
	printk_info("pm2_iowrite\n");
	pmio_write_index(PM2_INDEX, reg, value);
}

u8 pm2_ioread(u8 reg)
{
	printk_info("pm2_ioread\n");
	return pmio_read_index(PM2_INDEX, reg);
}

static void set_pmio_enable_bits(device_t sm_dev, u32 reg_pos,
				 u32 mask, u32 val)
{
	u8 reg_old, reg;
	reg = reg_old = pm_ioread(reg_pos);
	reg &= ~mask;
	reg |= val;
	if (reg != reg_old) {
		pm_iowrite(reg_pos, reg);
	}
}

void sb700_sata(int enabled)
{
	device_t sm_dev;
	int index = 8;

	sm_dev = _pci_make_tag(0, 0x14, 0);
	set_sm_enable_bits(sm_dev, 0xac, 1 << index, (enabled ? 1 : 0) << index);	
}


void sb700_usb(device_t usb_dev, int enabled, int index)
{
	device_t sm_dev;
	//int index;
	//int function;

	//_pci_break_tag(usb_dev, NULL, NULL, &function);
	//index = (function + 1) % 6;

	sm_dev = _pci_make_tag(0, 0x14, 0);
	set_sm_enable_bits(sm_dev, 0x68, 1 << index, (enabled ? 1 : 0) << index);	
}

void sb700_hda(int enabled)
{
        device_t sm_dev;
        int index = 3;

        sm_dev = _pci_make_tag(0, 0x14, 0);
        set_pmio_enable_bits(sm_dev, 0x59, 1 << index, (enabled ? 1 : 0) << index);
}


void sb700_lpc(int enabled)
{
        device_t sm_dev;
        int index = 20;
        
        sm_dev = _pci_make_tag(0, 0x14, 0);
        set_sm_enable_bits(sm_dev, 0x64, 1 << index, (enabled ? 1 : 0) << index);
}

/* Set Memory Range Port Enable/Disable for LPC memory target range. daway added 2011-02-18 */
void sb700_lpc_memory(int enabled)
{
	device_t sm_dev;
	int index = 5;

	sm_dev = _pci_make_tag(0, 0x14, 3);
	set_sm_enable_bits(sm_dev, 0x48, 1 << index, (enabled ? 1 : 0) << index);	
}

/* Set PCI Memory Start Address for LPC target Cycle - PCI_REG: 60h,
 * and set PCI Memory End Address for LPC target Cycle - PCI_REG: 62h.
 * This range can be enabled/disabled using register 48h, bit5.
 * daway added 2011-02-18 */
void sb700_lpcmem_address(int value)
{
	device_t sm_dev;

	sm_dev = _pci_make_tag(0, 0x14, 3);
	pci_write_config32(sm_dev, 0x60, value);
}

void sb700_aci(device_t dev, int enabled)
{
        device_t sm_dev;
        int index;
	int function;

	_pci_break_tag(dev, NULL, NULL, &function);
        index = function - 5;
        sm_dev = _pci_make_tag(0, 0x14, 0);
        set_pmio_enable_bits(sm_dev, 0x59, 1 << index, (enabled ? 1 : 0) << index);
}


void sb700_mci(device_t dev, int enabled)
{
        device_t sm_dev;
        int index;
	int function;

	_pci_break_tag(dev, NULL, NULL, &function);
        index = function - 5;
        sm_dev = _pci_make_tag(0, 0x14, 0);
        set_pmio_enable_bits(sm_dev, 0x59, 1 << index, (enabled ? 1 : 0) << index);
}



void sb700_enable()
{
	/*
	 *	0:11.0  SATA	bit 8 of sm_dev 0xac : 1 - enable, default         + 32 * 3
	 *	0:12.0  OHCI0-USB1	bit 0 of sm_dev 0x68
	 *	0:12.1  OHCI1-USB1	bit 1 of sm_dev 0x68
	 *	0:12.2  EHCI-USB1	bit 2 of sm_dev 0x68
	 *	0:13.0  OHCI0-USB2	bit 4 of sm_dev 0x68
	 *	0:13.1  OHCI1-USB2	bit 5 of sm_dev 0x68
	 *	0:13.2  EHCI-USB2	bit 6 of sm_dev 0x68
	 *	0:14.5  OHCI0-USB3	bit 7 of sm_dev 0x68
	 *	0:14.0  SMBUS							0
	 *	0:14.1  IDE							1
	 *	0:14.2  HDA	bit 3 of pm_io 0x59 : 1 - enable, default	    + 32 * 4
	 *	0:14.3  LPC	bit 20 of sm_dev 0x64 : 0 - disable, default  + 32 * 1
	 *	0:14.4  PCI							4
	 */
#ifdef ENABLE_SATA	
	printk_info("enable_sata\n");
	sb700_sata(1);
#endif

	printk_info("enable usb0\n");
	sb700_usb(_pci_make_tag(0, 0x12, 0), 1, 0);

	printk_info("enable usb1\n");
	sb700_usb(_pci_make_tag(0, 0x12, 1), 1, 1);
#if  1
	//printk_info("enable usb2\n");
	//sb700_usb(_pci_make_tag(0, 0x12, 2), 1, 2);
	printk_info("enable usb4\n");
	sb700_usb(_pci_make_tag(0, 0x13, 0), 1, 4);
	printk_info("enable usb5\n");
	sb700_usb(_pci_make_tag(0, 0x13, 1), 1, 5);
	//printk_info("enable usb6\n");
	//sb700_usb(_pci_make_tag(0, 0x13, 2), 1, 6);
	printk_info("enable usb7\n");
	sb700_usb(_pci_make_tag(0, 0x14, 5), 1, 7);
#endif
	//printk_info("enable hda\n");
	//sb700_hda(1);
	printk_info("enable lpc\n");
	sb700_lpc(1);
	//sb700_aci(_pci_make_tag(0, 0x14, 5), 1);
	//sb700_mci(_pci_make_tag(0, 0x14, 6), 1);

	/* Enable LPC memory target range. daway added 2011-02-18 */
	printk_info("enable lpc memory\n");
	sb700_lpc_memory(1);
	/* Set PCI Memory Start Address for LPC target Cycle - PCI_REG: 60h,
	 * and PCI Memory End Address for LPC target Cycle - PCI_REG: 62h,
	 * as EC Shared Memory map to host memory, using to update ec firmware.
	 * Start address(upper 16-bits): 0x1700, End address(upper 16-bits): 0x171f.
	 * i.e. 0x17000000 - 0x171fffff, tatol 2MB size.
	 * daway added 2011-02-18 */
	printk_info("Set lpc memory range: 0x17000000 - 0x171fffff\n");
	sb700_lpcmem_address(0x171f1700);
}

