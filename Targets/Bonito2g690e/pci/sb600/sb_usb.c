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

/*----------------------------------------------------------------------*/

extern u8 get_nbsb_link_width(void);
/*
 * sb_usb_post_init :
 * usb post init routine
 */
void sb_usb_post_init(void)
{
	pcitag_t ehci_dev = _pci_make_tag(0, 19, 5);	// EHCI
	pcitag_t sm_dev = _pci_make_tag(0, 20, 0);
	u8 rev = get_sb600_revision();
	u32 base;
	u8 cap_len;

	PRINTF_DEBUG();
	/* enable memory decode */
	set_sbcfg_enable_bits(ehci_dev, 0x04, 1 << 1, 1 << 1);

	if(rev <= REV_SB600_A12){
		/* RPR5.7 USB Data Cache Time Out Counter Setting for SB600 A11/A12, 
		 * EHCI_PCI_Config 0x50 [19:16] = 0xF
		 * Disables the data cache time out counter. When these 4 bits are set to 0xF, 
		 * the data cache time out counter will never expire.
		 */
		set_sbcfg_enable_bits(sm_dev, 0x50, 0x0f << 16, 0x0f << 16);
	}

	/* get bar0 base addr */
	if((_pci_conf_read(ehci_dev, 0x10) & 0xfffffff0) != 0x00){
		base = (_pci_conf_read(ehci_dev, 0x10) & 0xfffffff0) | 0xA0000000;
	} else {
		DEBUG_INFO("bar0 is empty!!!!\n");
		return ;
	}
	cap_len = *(volatile u8 *)(base + 0x00);
	DEBUG_INFO("usb base : 0x%x, cap length : 0x%x\n", base, cap_len);

	/* USB phy auto calibration setting */
	*(volatile u32 *)(base + cap_len + 0xA0) = 0x00020F00;
	/* USB IN/OUT FIFO threshold setting */
	*(volatile u32 *)(base + cap_len + 0x84) = 0x00200010;
	/* RPR5.8 USB OHCI Dynamic Power Saving Setting,OHCI0_PCI_Config 0x50 [16] = 1,
	 * Enables OHCI Dynamic Power Saving feature
	 */
//	set_sbcfg_enable_bits(ohci0_dev, 0x50, 1 << 16, 1 << 16);
	
	/* RPR5.9 USB EHCI Dynamic Power Saving Setting, EHCI_BAR 0xBC [12] = 0, 
	 * Disables EHCI Dynamic Power Saving feature
	 */
	*(volatile u32 *)(base + cap_len + 0x9C) &= ~(1 << 12);

	if(get_nbsb_link_width() >= 2){
		*(volatile u32 *)(base + cap_len + 0x84) = 0x00200040;		
	}	

	return;
}


