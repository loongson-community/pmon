/*
 * Copyright (C) 2009 Lemote, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 *
 * Author : liujl <liujl@lemote.com>
 * Date : 2009-08-18
 */
/*-----------------------------------------------------------------------*/

#include <stdio.h>
#include <stdarg.h>
#include <sys/linux/types.h>
#include <include/bonito.h>
#include <linux/io.h>
#include <machine/pio.h> 
#include <include/sb600.h>
#include <pci/sb600_cfg.h>
#include "sb600_config.h"

/*---------------------------- POR -------------------------------------*/

/* sbDevicesPorInitTable */
static void sb600_devices_por_init(void)
{
	pcitag_t dev;
	u8 byte;

	DEBUG_INFO("sb600_devices_por_init()\n");
	
	/* SMBus Device, BDF:0-20-0 */
	DEBUG_INFO("sb600_devices_por_init(): SMBus Device, BDF:0-20-0\n");
	dev = _pci_make_tag(0, 20, 0);

	/* start of sbPorAtStartOfTblCfg */
	/* Set A-Link bridge access address. This address is set at device 14h, 
	 * function 0, register 0xf0. This is an I/O address. The I/O address 
	 * must be on 16-byte boundry.  */
	_pci_conf_write(dev, 0xf0, AB_INDX);

	/* To enable AB/BIF DMA access, a specific register inside the BIF 
	 * register space needs to be configured first. 
	 * Enables the SB600 to send transactions upstream over A-Link 
	 * Express interface. */
	axcfg_reg(0x04, 1 << 2, 1 << 2);
	axindxc_reg(0x21, 0xff, 0);

	/* 2.3.5:Enabling Non-Posted Memory Write for the K8 Platform */
	axindxc_reg(0x10, 1 << 9, 1 << 9);
	/* END of sbPorAtStartOfTblCfg */

	/* sbDevicesPorInitTables */
	/* set smbus iobase : this should be a temp one. */
	_pci_conf_write(dev, 0x10, SMBUS_IO_BASE | 1);

	/* enable smbus controller interface */
	byte = _pci_conf_readn(dev, 0xd2, 1);
	byte |= (1 << 0);
	_pci_conf_writen(dev, 0xd2, byte, 1);

	/* set smbus 1, ASF 2.0 (Alert Standard Format), iobase */
	_pci_conf_writen(dev, 0x58, SMBUS_IO_BASE | 0x11, 2);

	/* TODO: I don't know the useage of followed two lines. I copied them from CIM. */
	_pci_conf_writen(dev, 0x0a, 0x1, 1);
	_pci_conf_writen(dev, 0x0b, 0x6, 1);

	/* KB2RstEnable */
	_pci_conf_writen(dev, 0x40, 0xd4, 1);

	/* Enable ISA Address 0-960K decoding */
	_pci_conf_writen(dev, 0x48, 0x0F, 1);

	/* Enable ISA  Address 0xC0000-0xDFFFF decode */
	_pci_conf_writen(dev, 0x49, 0xff, 1);

	/* Enable decode cycles to IO C50, C51, C52 GPM controls. */
	byte = _pci_conf_readn(dev, 0x41, 1);
	byte &= 0x80;
	byte |= 0x33;
	_pci_conf_writen(dev, 0x41, byte, 1);

#ifdef	X86_DEBUG
	/* set ISA/LPC DMA control decode */
	_pci_conf_writen(dev, 0x48, 0x0F, 1);

	/* same as 0x48, but more memory address */
	_pci_conf_writen(dev, 0x49, 0xFF, 1);

	/* misc function enable : intr, apic, gevent */
	_pci_conf_writen(dev, 0x65, 0x79, 1);

	/* misc function enable : PIC, timer, PMIO, ioapic, BMREQEN */
	_pci_conf_writen(dev, 0x64, 0xBF, 1);
	
	/* misc function enable : lpc, gevent enable */
	_pci_conf_writen(dev, 0x66, 0x9E, 1);

	/* misc function enable : IRQ1/12, DMA Verify En */
	_pci_conf_writen(dev, 0x67, 0x82, 1);
#endif

	/* SerialIrq Control */
	_pci_conf_writen(dev, 0x69, 0x90, 1);

	/* Test Mode, PCIB_SReset_En Mask is set. */
	_pci_conf_writen(dev, 0x6c, 0x20, 1);

	/* IO Address Enable, CIM set 0x78 only and masked 0x79. */
	/*pci_conf1_write_config8(dev, 0x79, 0x4F); */
	_pci_conf_writen(dev, 0x78, 0xFF, 1);

	/* This register is not used on sb600. It came from older chipset. */
	/*pci_conf1_write_config8(dev, 0x95, 0xFF); */

	/* Set smbus iospace enable, I don't know why write 0x04 into reg5 
	 * that is reserved 
	 * bit10 disable asserting MSI interrupt, refer to PCI spec.
	 */
	_pci_conf_writen(dev, 0x4, 0x0407, 2);

	/* clear any lingering errors, so the transaction will run */
	byte = linux_inb(SMBUS_IO_BASE + SMBHSTSTAT);
	linux_outb(byte, SMBUS_IO_BASE + SMBHSTSTAT);

	/* IDE Device, BDF:0-20-1 */
	DEBUG_INFO("sb600_devices_por_init(): IDE Device, BDF:0-20-1\n");
	dev = _pci_make_tag(0, 20, 1);
	/* Disable prefetch */
	byte = _pci_conf_readn(dev, 0x63, 1);
	byte |= 0x1;
	_pci_conf_writen(dev, 0x63, byte, 1);

	/* LPC Device, BDF:0-20-3 */
	dev = _pci_make_tag(0, 20, 3);
	DEBUG_INFO("sb600_devices_por_init(): LPC Device, BDF:0-20-3\n");
	/* DMA enable */
	_pci_conf_writen(dev, 0x40, 0x04, 1);

	/* IO Port Decode Enable */
	_pci_conf_writen(dev, 0x44, 0xFF, 1);
	_pci_conf_writen(dev, 0x45, 0xFF, 1);
	_pci_conf_writen(dev, 0x46, 0xC3, 1);
	_pci_conf_writen(dev, 0x47, 0xFF, 1);

	/* IO/Mem Port Decode Enable, I don't know why CIM disable some ports.
	 * Disable LPC TimeOut counter, enable SuperIO Configuration Port (2e/2f),
	 * Alternate SuperIO Configuration Port (4e/4f), Wide Generic IO Port (64/65).
	 * Enable bits for LPC ROM memory address range 1&2 for 1M ROM setting.*/
	byte = _pci_conf_readn(dev, 0x48, 1);
	byte |= (1 << 1) | (1 << 0);	/* enable Super IO config port 2e-2h, 4e-4f */
	byte |= (1 << 3) | (1 << 4);	/* enable for LPC ROM address range1&2, Enable 512KB rom access at 0xFFF80000 - 0xFFFFFFFF */
	byte |= 1 << 6;		/* enable for RTC I/O range */
	_pci_conf_writen(dev, 0x48, byte, 1);	// They just setting 0x07
	_pci_conf_writen(dev, 0x49, 0xFF, 1);
	/* Enable 0x480-0x4bf, 0x4700-0x470B */
	byte = _pci_conf_readn(dev, 0x4A, 1);
	byte |= ((1 << 1) + (1 << 6));	/*0x42, save the configuraion for port 0x80. */
	_pci_conf_writen(dev, 0x4A, byte, 1);

	/* Set LPC ROM size, it has been done in sb600_lpc_init().
	 * enable LPC ROM range, 0xfff8: 512KB, 0xfff0: 1MB;
	 * enable LPC ROM range, 0xfff8: 512KB, 0xfff0: 1MB
	 * pci_conf1_write_config16(dev, 0x68, 0x000e)
	 * pci_conf1_write_config16(dev, 0x6c, 0xfff0);*/

	/* Enable Tpm12_en and Tpm_legacy. I don't know what is its usage and copied from CIM. */
	_pci_conf_writen(dev, 0x7C, 0x05, 1);

	/* P2P Bridge, BDF:0-20-4, the configuration of the registers in this dev are copied from CIM,
	 * TODO: I don't know what these mean? */
	DEBUG_INFO("sb600_devices_por_init(): P2P Bridge, BDF:0-20-4\n");
	dev = _pci_make_tag(0, 20, 4);
	/* I don't know why CIM tried to write into a read-only reg! */
	/*pci_conf1_write_config8(dev, 0x0c, 0x20) */ ;

	_pci_conf_writen(dev, 0x0C, 0x20, 1);

	/* dma or request setting */
	byte = _pci_conf_readn(dev, 0x64, 1);
	byte &= 0x30;
	byte |= 0x8a;
	_pci_conf_writen(dev, 0x64, byte, 1);

	/* Arbiter enable. */
	_pci_conf_writen(dev, 0x43, 0xff, 1);

	/* Set PCDMA request into hight priority list. */
	/* pci_conf1_write_config8(dev, 0x49, 0x1) */ ;	
	_pci_conf_writen(dev, 0x49, 0x01, 1);

	_pci_conf_writen(dev, 0x40, 0x26, 1);

	/* I don't know why CIM set reg0x1c as 0x11.
	 * System will block at sdram_initialize() if I set it before call sdram_initialize().
	 * If it is necessary to set reg0x1c as 0x11, please call this function after sdram_initialize().
	 * pci_conf1_write_config8(dev, 0x1c, 0x11);
	 * pci_conf1_write_config8(dev, 0x1d, 0x11);*/
	/* we don't get io address for PCI first, maybe added later */

	/*CIM set this register; but I didn't find its description in RPR.
	On DBM690T platform, I didn't find different between set and skip this register.
	But on Filbert platform, the DEBUG message from serial port on Peanut board can't be displayed
	after the bit0 of this register is set.
	pci_conf1_write_config8(dev, 0x04, 0x21);
	*/
	/* maybe open it later */

	/* latency timer */
	_pci_conf_writen(dev, 0x0d, 0x40, 1);
	_pci_conf_writen(dev, 0x1b, 0x40, 1);

	/* Enable PCIB_DUAL_EN_UP will fix potential problem with PCI cards. */
	byte = _pci_conf_readn(dev, 0x50, 1);
	byte &= 0x02;
	byte |= 1;
	_pci_conf_writen(dev, 0x50, byte, 1);

	/* SATA Device, BDF:0-18-0, Non-Raid-5 SATA controller */
	dev = _pci_make_tag(0, 18, 0);
	DEBUG_INFO("sb600_devices_por_init(): SATA Device, BDF:0-18-0\n");

	/*PHY Global Control, we are using A14.
	 * default:  0x2c40 for ASIC revision A12 and below
	 *      0x2c00 for ASIC revision A13 and above.*/
	if(get_sb600_revision() <= REV_SB600_A12)
		_pci_conf_writen(dev, 0x86, 0x2C40, 2);
	else
		_pci_conf_writen(dev, 0x86, 0x2C00, 2);

	/* PHY Port0-3 Control */
	_pci_conf_write(dev, 0x88, 0xB400DA);
	_pci_conf_write(dev, 0x8c, 0xB400DA);
	_pci_conf_write(dev, 0x90, 0xB400DA);
	_pci_conf_write(dev, 0x94, 0xB400DA);

	/* Port0-3 BIST Control/Status */
	_pci_conf_writen(dev, 0xa5, 0xB8, 1);
	_pci_conf_writen(dev, 0xad, 0xB8, 1);
	_pci_conf_writen(dev, 0xb5, 0xB8, 1);
	_pci_conf_writen(dev, 0xbd, 0xB8, 1);
}

/* sbPmioPorInitTable, Pre-initializing PMIO register space
* The power management (PM) block is resident in the PCI/LPC/ISA bridge.
* The PM regs are accessed via IO mapped regs 0xcd6 and 0xcd7.
* The index address is first programmed into IO reg 0xcd6.
* Read or write values are accessed through IO reg 0xcd7.
*/
static void sb600_pmio_por_init(void)
{
	u8 byte;

	DEBUG_INFO("sb600_pmio_por_init()\n");

	/* C state enable and SLP enable in C states. */
	byte = pm_ioread(0x67);
	byte |= 0x6;
	pm_iowrite(0x67, byte);

	/* CIM sets 0x0e, but bit1 is for P4 system. */
	byte = pm_ioread(0x68);
	byte &= 0xf0;
	byte |= 0x0c;
	pm_iowrite(0x68, byte);

	/* Watch Dog Timer Control
	 * Set watchdog time base to 0xfec000f0 to avoid SCSI card boot failure.
	 * But I don't find WDT is enabled in SMBUS 0x41 bit3 in CIM.
	 */
#if	0	// we should change the WDT base addr for MIPS platform
	pm_iowrite(0x6c, 0xf0);
	pm_iowrite(0x6d, 0x00);
	pm_iowrite(0x6e, 0xc0);
	pm_iowrite(0x6f, 0xfe);
#endif

	/* power failure setting */
	set_pm_enable_bits(0x74, (u8)(~0xF0), 0x00);
	/* enable Alt-Centry RTC register */
	set_pm_enable_bits(0x7C, (u8)(~0xDF), 0x10);
	/* BM_REQ# not just pop-up the machine */
	set_pm_enable_bits(0x8F, (u8)(~0xEF), 0x00);
	/* GPM level config */
	set_pm_enable_bits(0x37, (u8)(~0xFF), 0x84);
	/* CPU_STP# config */
	set_pm_enable_bits(0x50, (u8)(~0x00), 0xE0);
	/* GPIO2 as Speaker output */
	set_pm_enable_bits(0x60, (u8)(~0xFF), 0x20);
	/* USB PME enable */
	set_pm_enable_bits(0x61, (u8)(~0xFF), 0x40);
	/* switch GHI Timer */
	set_pm_enable_bits(0x64, (u8)(~0x00), 0x00);
	/* wake up pin config */
	set_pm_enable_bits(0x84, (u8)(~0x0F), 0x0B);
	/* USB PM control */
	set_pm_enable_bits(0x65, (u8)(~0xEF), 0x00);
	/* disable HPET period mode HW support  */
	set_pm_enable_bits(0x9A, (u8)(~0x7F), 0x00);
	/* clear software PCIRST */
	set_pm_enable_bits(0x55, (u8)(~0xBF), 0x00);
	/* vid/fid time setting */
	set_pm_enable_bits(0x8A, (u8)(~0x8F), 0x10);
	/* THRMTRIP_enable */
	set_pm_enable_bits(0x68, (u8)(~0xFF), 0x08);
	/* software PCIRST enable, GPM7 as User RST  */
	set_pm_enable_bits(0x55, (u8)(~0xFF), 0x05);
	/* USB PM spec. cycle. */
	set_pm_enable_bits(0x65, (u8)(~0x7F), 0x00);
	/* mask apic enable */
	set_pm_enable_bits(0x68, (u8)(~0xFF), 0x04);
	/* SUS_STAT# timing option */
	set_pm_enable_bits(0x7C, (u8)(~0xFF), 0x20);
	/* S0S3TOS5 enable */
	set_pm_enable_bits(0x79, (u8)(~0xFB), 0x00);
	/* AUTO ARB disable from Wakeup */
	set_pm_enable_bits(0x9F, (u8)(~0xFF), 0x40);
	/* PME MSG enable */
	set_pm_enable_bits(0x8D, (u8)(~0xFF), 0x01);
}

void sb600_last_por_init(void)
{
	pcitag_t sb_dev = _pci_make_tag(0, 20, 0);
	u8 byte;

	DEBUG_INFO("sb600_last_por_init()\n");
	/* USB SMI & K8_INTR */
	byte = _pci_conf_readn(sb_dev, 0x62, 1);
	byte |= 0x24;
	_pci_conf_writen(sb_dev, 0x62, byte, 1);

	/* K8KbRstEn, KB_RST# control for K8 system. */
	byte = pm_ioread(0x66);
	byte |= 0x20;
	pm_iowrite(0x66, byte);

	/* RPR2.3.4 S3/S4/S5 Function for the K8 Platform. */
	byte = pm_ioread(0x52);
	byte &= 0xc0;
	byte |= 0x08;
	pm_iowrite(0x52, byte);

	/* maybe we should judge the CMOS parameters and setting some base address
	 * of some functions
	 */
}

/*------------------------------before pci init---------------------------------*/

/*
* Compliant with CIM_48's sbPciCfg.
* Add any south bridge setting.
*/
void sb600_pci_cfg(void)
{
	u32 dev;
	u8 byte;

	/* SMBus Device, BDF:0-20-0 */
	dev = _pci_make_tag(0, 20, 0);
	/* Eable the hidden revision ID, available after A13. */
	byte = _pci_conf_readn(dev, 0x70, 1);
	byte |= (1 << 8);
	_pci_conf_writen(dev, 0x70, byte, 1);
	/* rpr2.20 Disable Timer IRQ Enhancement for proper operation of the 8254 timer, 0xae[5]. */
	byte = _pci_conf_readn(dev, 0xae, 1);
	byte |= (1 << 5);
	_pci_conf_writen(dev, 0xae, byte, 1);

	/* Enable watchdog decode timer */
	byte = _pci_conf_readn(dev, 0x41, 1);
	byte |= (1 << 3);
	_pci_conf_writen(dev, 0x41, byte, 1);

	/* Set to 1 to reset USB on the software (such as IO-64 or IO-CF9 cycles)
	 * generated PCIRST#. */
	byte = pm_ioread(0x65);
	byte |= (1 << 4);
	pm_iowrite(0x65, byte);
	/*For A13 and above. */
	if (get_sb600_revision() > REV_SB600_A12) {
		/* rpr2.16 C-State Reset, PMIO 0x9f[7]. */
		byte = pm_ioread(0x9f);
		byte |= (1 << 7);
		pm_iowrite(0x9f, byte);
		/* rpr2.17 PCI Clock Period will increase to 30.8ns. 0x53[7]. */
		byte = pm_ioread(0x53);
		byte |= (1 << 7);
		pm_iowrite(0x53, byte);
	}

	/* IDE Device, BDF:0-20-1 */
	dev = _pci_make_tag(0, 20, 1);
	/* Enable IDE Explicit prefetch, 0x63[0] clear */
	byte = _pci_conf_readn(dev, 0x63, 1);
	byte &= 0xfe;
	_pci_conf_writen(dev, 0x63, byte, 1);

	/* LPC Device, BDF:0-20-3 */
	dev = _pci_make_tag(0, 20, 3);
	/* rpr7.2 Enabling LPC DMA function. */
	byte = _pci_conf_readn(dev, 0x40, 1);
	byte |= (1 << 2);
	_pci_conf_writen(dev, 0x40, byte, 1);
	/* rpr7.3 Disabling LPC TimeOut. 0x48[7] clear. */
	byte = _pci_conf_readn(dev, 0x48, 1);
	byte &= 0x7f;
	_pci_conf_writen(dev, 0x48, byte, 1);
	/* rpr7.5 Disabling LPC MSI Capability, 0x78[1] clear. */
	byte = _pci_conf_readn(dev, 0x78, 1);
	byte &= 0xfd;
	_pci_conf_writen(dev, 0x78, byte, 1);

	/* SATA Device, BDF:0-18-0, Non-Raid-5 SATA controller */
	dev = _pci_make_tag(0, 18, 0);
	/* rpr6.8 Disabling SATA MSI Capability, for A13 and above, 0x42[7]. */
	if (REV_SB600_A12 < get_sb600_revision()) {
		u32 reg32;
		reg32 = _pci_conf_read(dev, 0x40);
		reg32 |= (1 << 23);
		_pci_conf_write(dev, 0x40, reg32);
	}

	/* EHCI Device, BDF:0-19-5, ehci usb controller */
	dev = _pci_make_tag(0, 19, 5);
	/* rpr5.10 Disabling USB EHCI MSI Capability. 0x50[6]. */
	byte = _pci_conf_readn(dev, 0x50, 1);
	byte |= (1 << 6);
	_pci_conf_writen(dev, 0x50, byte, 1);

	/* OHCI0 Device, BDF:0-19-0, ohci usb controller #0 */
	dev = _pci_make_tag(0, 19, 0);
	/* rpr5.11 Disabling USB OHCI MSI Capability. 0x40[12:8]=0x1f. */
	byte = _pci_conf_readn(dev, 0x41, 1);
	byte |= 0x1f;
	_pci_conf_writen(dev, 0x41, byte, 1);

}

/*
* Compliant with CIM_48's ATSBPowerOnResetInitJSP
*/
void sb_por_init(void)
{

	DEBUG_INFO("\n++++++++++++++++++++++++++++++++++ SB POR STAGE WITH REV(%d)+++++++++++++++++++++++++++\n", get_sb600_revision());
	/* sbDevicesPorInitTable + sbK8PorInitTable */
	sb600_devices_por_init();

	/* sbPmioPorInitTable + sbK8PmioPorInitTable */
	sb600_pmio_por_init();

	/* the last setting */
	sb600_last_por_init();

	sb600_pci_cfg();
	
	DEBUG_INFO("\n------------------------------------- SB POR STAGE DONE ------------------------------------\n");
}


