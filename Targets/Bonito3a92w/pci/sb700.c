#include "sb700.h"
#include "rs780_cmn.h"
#include <include/bonito.h>
#define NULL (void*)0
/* Base Address */
#ifndef CONFIG_TTYS0_BASE
//#define CONFIG_TTYS0_BASE 0x3f8
#define CONFIG_TTYS0_BASE 0x2f8
#endif
 
#ifndef CONFIG_TTYS0_BAUD
#define CONFIG_TTYS0_BAUD 115200
#endif
 
#if ((115200%CONFIG_TTYS0_BAUD) != 0)
#error Bad ttys0 baud rate
#endif
 
#ifndef CONFIG_TTYS0_DIV
#define CONFIG_TTYS0_DIV        (115200/CONFIG_TTYS0_BAUD)
#endif
 
/* Line Control Settings */
#ifndef CONFIG_TTYS0_LCS
/* Set 8bit, 1 stop bit, no parity */
#define CONFIG_TTYS0_LCS        0x3
#endif
 
#define UART_LCS        CONFIG_TTYS0_LCS


/* Data */
#define UART_RBR 0x00
#define UART_TBR 0x00
 
/* Control */
#define UART_IER 0x01
#define UART_IIR 0x02
#define UART_FCR 0x02
#define UART_LCR 0x03
#define UART_MCR 0x04
#define UART_DLL 0x00
#define UART_DLM 0x01
 
#define UART_LSR 0x05
#define UART_MSR 0x06
#define UART_SCR 0x07


int uart8250_can_tx_byte(unsigned base_port)
{
        return INB(base_port + UART_LSR+BONITO_PCIIO_BASE_VA) & 0x20;
}
 
void uart8250_wait_to_tx_byte(unsigned base_port)
{
        while(!uart8250_can_tx_byte(base_port))
                ;
}
 
void uart8250_wait_until_sent(unsigned base_port)
{
        while(!(INB(base_port + UART_LSR+BONITO_PCIIO_BASE_VA) & 0x40))
                ;
}
 
void uart8250_tx_byte(unsigned base_port, unsigned char data)
{
        uart8250_wait_to_tx_byte(base_port);
        OUTB(data, base_port + UART_TBR+BONITO_PCIIO_BASE_VA);
        /* Make certain the data clears the fifos */
        uart8250_wait_until_sent(base_port);
}
 
int uart8250_can_rx_byte(unsigned base_port)
{
        return INB(base_port + UART_LSR+BONITO_PCIIO_BASE_VA) & 0x01;
}
 
unsigned char uart8250_rx_byte(unsigned base_port)
{
        while(!uart8250_can_rx_byte(base_port))
                ;
        return INB(base_port + UART_RBR+BONITO_PCIIO_BASE_VA);
}
 
void uart8250_init(unsigned base_port, unsigned divisor, unsigned lcs)
{
        lcs &= 0x7f;
        /* disable interrupts */
        OUTB(0x0, base_port + UART_IER+BONITO_PCIIO_BASE_VA);
        /* enable fifo's */
        OUTB(0x01, base_port + UART_FCR+BONITO_PCIIO_BASE_VA);
        /* assert DTR and RTS so the other end is happy */
        OUTB(0x03, base_port + UART_MCR+BONITO_PCIIO_BASE_VA);
        /* Set Baud Rate Divisor to 12 ==> 115200 Baud */
        OUTB(0x80 | lcs, base_port + UART_LCR+BONITO_PCIIO_BASE_VA);
        OUTB(divisor & 0xFF,   base_port + UART_DLL+BONITO_PCIIO_BASE_VA);
        OUTB((divisor >> 8) & 0xFF,    base_port + UART_DLM+BONITO_PCIIO_BASE_VA);
        OUTB(lcs, base_port + UART_LCR+BONITO_PCIIO_BASE_VA);
}
/* CONFIG_USE_PRINTK_IN_CAR == 1 */
 
//extern void uart8250_init(unsigned base_port, unsigned divisor, unsigned lcs);
void uart_init(void)
{
        //for port 0
        uart8250_init(0x3f8, CONFIG_TTYS0_DIV, UART_LCS);
        //for port 1
        uart8250_init(CONFIG_TTYS0_BASE, CONFIG_TTYS0_DIV, UART_LCS);
}


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
}

