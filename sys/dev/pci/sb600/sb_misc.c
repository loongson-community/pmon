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

/*----------------------------------------- RESET -------------------------------------------------*/

static void set_bios_reset(void)
{
	return;
}

void soft_reset(void)
{
	set_bios_reset();
	/* link reset */
	linux_outb(0x06, 0x0cf9);
}

void hard_reset(void)
{
	set_bios_reset();
	/* Try rebooting through port 0xcf9 */
	/* Actually it is not a real hard_reset --- it only resets coherent link table, but 
	 * does not reset link freq and width */
	linux_outb((0 << 3) | (0 << 2) | (1 << 1), 0xcf9);
	linux_outb((0 << 3) | (1 << 2) | (1 << 1), 0xcf9);
}

/* Get SB ASIC Revision.*/
u8 get_sb600_revision(void)
{
	pcitag_t sb_dev = _pci_make_tag(0, 0x14, 0);
	u8 rev;
	u32 reg32;

	rev = _pci_conf_readn(sb_dev, 0x08, 1);
	if(rev >= REV_SB600_A13){
		reg32 = _pci_conf_read(sb_dev, 0x70);
		reg32 |= 1 << 8;
		_pci_conf_write(sb_dev, 0x70, reg32);
		rev = _pci_conf_readn(sb_dev, 0x08, 1);
		reg32 &= ~(1 << 8);
		_pci_conf_write(sb_dev, 0x70, reg32);
	}

	return rev;
}

/* Get SB ASIC platform Revision.*/
u8 get_sb600_platform(void)
{
	pcitag_t sb_dev = _pci_make_tag(0, 0x14, 0);

	return _pci_conf_readn(sb_dev, 0x34, 1);
}

/*-------------------------------------- FID CHANGE  ---------------------------------------------*/
/*
SB600 VFSMAF (VID/FID System Management Action Field)  is 010b by default.
RPR 2.3.3 C-state and VID/FID change for the K8 platform.
*/
void enable_fid_change_on_sb(u32 sbbusn, u32 sbdn)
{
	u8 byte;
	byte = pm_ioread(0x9a);
	byte &= ~0x34;
	byte |= 0x04;	// single core
	pm_iowrite(0x9a, byte);

	byte = pm_ioread(0x8f);
	byte &= ~0x30;
	byte |= 0x20;
	pm_iowrite(0x8f, byte);

	pm_iowrite(0x8b, 0x01);
	pm_iowrite(0x8a, 0x90);

	if(get_sb600_revision() > REV_SB600_A13)
		pm_iowrite(0x88, 0x10);
	else
		pm_iowrite(0x88, 0x06);

	byte = pm_ioread(0x7c);
	byte &= ~0x01;
	byte |= 0x01;
	pm_iowrite(0x7c, byte);

	/*Must be 0 for K8 platform.*/
	byte = pm_ioread(0x68);
	byte &= ~0x01;
	pm_iowrite(0x68, byte);
	/*Must be 0 for K8 platform.*/
	byte = pm_ioread(0x8d);
	byte &= ~(1<<6);
	pm_iowrite(0x8d, byte);

	byte = pm_ioread(0x61);
	byte &= ~0x04;
	pm_iowrite(0x61, byte);

	byte = pm_ioread(0x42);
	byte &= ~0x04;
	pm_iowrite(0x42, byte);

	if(get_sb600_revision() == REV_SB600_A21) {
		pm_iowrite(0x89, 0x10);

		byte = pm_ioread(0x52);
		byte |= 0x80;
		pm_iowrite(0x52, byte);
	}
}

/*---------------------------------- PORT DEBUG ------------------------------------*/

void sb600_pci_port80(void)
{
	u8 byte;
	u32 dev;

	/* P2P Bridge */
	dev = _pci_make_tag(0, 20, 4);
	// pay attention to the seq of this two regs.
	byte = _pci_conf_readn(dev, 0x40, 1);
	byte |= 1 << 5;		// enable PCI bridge substractive decode 
	_pci_conf_writen(dev, 0x40, byte, 1);

	byte = _pci_conf_readn(dev, 0x4B, 1);
	byte |= 1 << 7;		// enable substractive decode function 
	_pci_conf_writen(dev, 0x4B, byte, 1);

	byte = _pci_conf_readn(dev, 0x1C, 1);
	byte |= 0xF << 4;		// manually define the IO base of PCI bridge? 
	_pci_conf_writen(dev, 0x1C, byte, 1);

	byte = _pci_conf_readn(dev, 0x1D, 1);
	byte |= 0xF << 4;		// manually define the IO limit of PCI bridge
	_pci_conf_writen(dev, 0x1D, byte, 1);

	byte = _pci_conf_readn(dev, 0x04, 1);
	byte |= 1 << 0;			// enable IO space
	_pci_conf_writen(dev, 0x04, byte, 1);

	/* LPC bridge */
	dev = _pci_make_tag(0, 20, 3);
	byte = _pci_conf_readn(dev, 0x4A, 1);
	byte &= ~(1 << 5);	/* disable lpc port 80 */
	_pci_conf_writen(dev, 0x4A, byte, 1);
}

void sb600_lpc_port80(void)
{
	u8 byte;
	u32 dev;
	u32 reg32;

	/* enable lpc controller */
	dev = _pci_make_tag(0, 20, 0);
	reg32 = _pci_conf_read(dev, 0x64);
	reg32 |= 0x00100000;	/* lpcEnable */
	_pci_conf_write(dev, 0x64, reg32);

	/* enable prot80 LPC decode in pci function 3 configuration space. */
	dev = _pci_make_tag(0, 20, 3);
	byte = _pci_conf_readn(dev, 0x4a, 1);
	byte |= 1 << 5;		/* enable port 80 */
	_pci_conf_writen(dev, 0x4a, byte, 1);
}

/*------------------------------------------- LPC INIT -----------------------------------------------------*/

/***************************************
* Legacy devices are mapped to LPC space.
*	serial port 0
*	KBC Port
*	ACPI Micro-controller port
*	LPC ROM size,
* NOTE: Call me ASAP, because I will reset LPC ROM size!
* NOTE : by general, it is called after HT emulation
***************************************/
void sb600_lpc_init(void)
{
	u8 reg8;
	u32 reg32;
	pcitag_t sm_dev = _pci_make_tag(0, 20, 0);
	pcitag_t lpc_dev = _pci_make_tag(0, 20, 3);

	/* Enable lpc controller */
	reg32 = _pci_conf_read(sm_dev, 0x64);
	reg32 |= 0x00100000;
	_pci_conf_write(sm_dev, 0x64, reg32);

	/* Serial 0 : 0x3f8~0x3ff */
	reg8 = _pci_conf_readn(lpc_dev, 0x44, 1);
	reg8 |= (1 << 6);
	_pci_conf_writen(lpc_dev, 0x44, reg8, 1);

	/* PS/2 keyboard, ACPI : 0x60/0x64 & 0x62/0x66 */
	reg8 = _pci_conf_readn(lpc_dev, 0x47, 1);
	reg8 |= (1 << 5) | (1 << 6);
	_pci_conf_writen(lpc_dev, 0x47, reg8, 1);

	/* SuperIO, LPC ROM */
	reg8 = _pci_conf_readn(lpc_dev, 0x48, 1);
	reg8 |= (1 << 1) | (1 << 0);	/* enable Super IO config port 2e-2h, 4e-4f */
	reg8 |= (1 << 3) | (1 << 4);	/* enable for LPC ROM address range1&2, Enable 512KB rom access at 0xFFF80000 - 0xFFFFFFFF  */
	reg8 |= 1 << 6;		/* enable for RTC I/O range */
	_pci_conf_writen(lpc_dev, 0x48, reg8, 1);

	/* hardware should enable LPC ROM by pin strapes */
	/*  rom access at 0xFFF80000/0xFFF00000 - 0xFFFFFFFF */
	/* See detail in BDG-215SB600-03.pdf page 15. */
	_pci_conf_writen(lpc_dev, 0x68, 0x000e, 2);	/* enable LPC ROM range, 0xfff8: 512KB, 0xfff0: 1MB; */
	_pci_conf_writen(lpc_dev, 0x6c, 0xfff0, 2);	/* enable LPC ROM range, 0xfff8: 512KB, 0xfff0: 1MB  */
}

/*
 * sb_hpet_cfg :
 * config the HPET module
 */
void sb_hpet_cfg(void)
{
	u8 rev = get_sb600_revision();
	pcitag_t sm_dev = _pci_make_tag(0, 20, 0);

	PRINTF_DEBUG();
#ifdef	CFG_ATI_HPET_BASE
#if	CFG_ATI_HPET_BASE
	if(ati_sb_cfg.hpet_en == SB_ENABLE){
		if(rev >= REV_SB600_A21){
			set_pm_enable_bits(0x55, 1 << 7, 0 << 7);
		}
		ati_sb_cfg.hpet_base = CFG_ATI_HPET_BASE;
		_pci_conf_write(sm_dev, 0x14, ati_sb_cfg.hpet_base);
		set_sbcfg_enable_bits(sm_dev, 0x64, 1 << 10, 1 << 10);
	}
#endif
#endif

	if(rev >= REV_SB600_A21){
		set_pm_enable_bits(0x55, 1 << 7, 1 << 7);
		set_pm_enable_bits(0x52, 1 << 6, 1 << 6);
	}

	return;
}

/*-----------------------------------------------------------------------------------------*/
