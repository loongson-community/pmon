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


static u32 sata_class_table[] = {
	0x01018F00,		// temp sata class ID of IDE
	0x01040000,		// temp sata class ID of RAID
	0x01060100,		// temp sata class ID of AHCI
	0x01018A00,		// temp sata class ID of Legacy IDE
	0x01018F00,		// temp sata class ID of IDE
	0x01800000		// temp sata class ID of storage
};

/*
 * sb_sata_pre_init :
 * sb sata pre init
 */
void sb_sata_pre_init(void)
{
	/*set sata hot plug support flag*/
	if(ati_sb_cfg.sata_channel == SB_ENABLE){
		if( (ati_sb_cfg.sata_class == 0) 
			|| (ati_sb_cfg.sata_class == 3) 
			|| (ati_sb_cfg.sata_class == 4) ){
			ati_sb_cfg.acaf |= (1 << 17);
		}
	}
	
	return;
}

/*
 * sb_sata_init :
 * sb sata init, for real device init and make it available
 */
void sb_sata_init(void)
{
	pcitag_t sm_dev = _pci_make_tag(0, 20, 0);
	pcitag_t ide_dev = _pci_make_tag(0, 20, 1);
	pcitag_t sata_dev = _pci_make_tag(0, 18, 0);
	u32 port_phy;
	u8  port_bist;
	u32 reg04;
	u32 sata_bar5;
	u8 rev, class;
	
	/* enable the sata controller according to config */
	if(ati_sb_cfg.sata_smbus == SB_ENABLE){
		// enable
		set_sbcfg_enable_bits(sm_dev, 0xAD, 1 << 1, 0 << 1);
	}else{
		// disable
		set_sbcfg_enable_bits(sm_dev, 0xAD, 1 << 1, 1 << 1);
	}

	/* enable proper SATA channel based on config : '0' none, '1' SATA1 */
	if(ati_sb_cfg.sata_channel == SB_DISABLE){
		// disable SATA1
		set_sbcfg_enable_bits(sm_dev, 0xAD, 1 << 0, 0 << 0);
	}else{
		// enable SATA1 & set power saving mode
		set_sbcfg_enable_bits(sm_dev, 0xAD, (1 << 0) | (1 << 5), (1 << 0) | (1 << 5));
	}

	/* BIT4 : disable fast boot
	 * BIT0 : enable write subsystem id
	 */
	set_sbcfg_enable_bits(sata_dev, 0x40, (1 << 0) | (1 << 4), (1 << 0) | (1 << 4));
	
	/* disable SATA MSI cap */
	set_sbcfg_enable_bits(sata_dev, 0x40, 1 << 23, 1 << 23);

	/* disable IDP cap */
	if(rev <= REV_SB600_A21){
		set_sbcfg_enable_bits(sata_dev, 0x40, 1 << 25, 1 << 25);		
	}

	/* setting device class */
	class = ati_sb_cfg.sata_class;
	if(class == 3){
		/* IDE native mode setting */
		set_sbcfg_enable_bits(ide_dev, 0x08, 0xFF << 8, 0x8F << 8);
	}
	/* SATA class mode setting */
	_pci_conf_write(sata_dev, 0x08, sata_class_table[class]);

	/* disable SATA class write */
	set_sbcfg_enable_bits(sata_dev, 0x40, 1 << 0, 0 << 0);

	/* program wdt with 16 retries before timer timeout */
	set_sbcfg_enable_bits(sata_dev, 0x44, 0xff << 16, 0x10 << 16);

	if(rev == REV_SB600_A11){
		/* PHY global tuning x86 to 0x2400 */
		_pci_conf_writen(sata_dev, 0x86, 0x2400, 2);
		/* PHY tuning for ports */
		port_phy = 0x00B420D8;
		port_bist = 0x00;
	}else{
		/* PHY global tuning x86 to 0x2400 */
		_pci_conf_writen(sata_dev, 0x86, 0x2c00, 2);
		if(rev == REV_SB600_A12){
			port_phy = 0x00B4005A;
			port_bist = 0xB8;
		}else if(rev == REV_SB600_A13){
			port_phy = 0x00B401DA;
			port_bist = 0xB8;
			if(ati_sb_cfg.sata_phy_avdd == SB_ENABLE){	// phy AVDD is 1.25V
				port_phy = 0x00B401D5;
				port_bist = 0x78;	
			}
		}else{
			port_phy = 0x00B401D6;
			port_bist = 0xB8;
		}
	}
	/* tuning 4 ports phy */
	_pci_conf_write(sata_dev, 0x88, port_phy);
	_pci_conf_write(sata_dev, 0x8C, port_phy);
	_pci_conf_write(sata_dev, 0x90, port_phy);
	_pci_conf_write(sata_dev, 0x94, port_phy);

	_pci_conf_writen(sata_dev, 0xA5, port_bist, 1);
	_pci_conf_writen(sata_dev, 0xAD, port_bist, 1);
	_pci_conf_writen(sata_dev, 0xB5, port_bist, 1);
	_pci_conf_writen(sata_dev, 0xBD, port_bist, 1);

    //patch for pass test
	set_sbcfg_enable_bits(sata_dev, 0x8c, 3 << 0, 3 << 2);
	set_sbcfg_enable_bits(sata_dev, 0xac, 1 << 0, 7 << 13);

	/* port reset */

	delay(1000);		// delay 1ms
	/* get sata bar5 base address  */
	reg04 = _pci_conf_read(sata_dev, 0x04);
	if( (_pci_conf_read(sata_dev, 0x24) & 0xfffffff0) == 0 ){
		_pci_conf_write(sata_dev, 0x24, TEMP_SATA_BAR5_BASE);
		sata_bar5 = TEMP_SATA_BAR5_BASE | 0xA0000000;
	}else{
		sata_bar5 = (_pci_conf_read(sata_dev, 0x24) & 0xfffffff0) | 0xA0000000;
	}
	/* enable io/mem access */
	set_sbcfg_enable_bits(sata_dev, 0x04, (1 << 0) | (1 << 1), (1 << 0) | (1 << 1));

	/* RPR 6.10 Hide Support-Aggressive-Link-Power-Management Capability 
	 * in AHCI HBA Capabilities Register (For all SB600)
	 * 1. SATA_PCI_config 0x40 [0] = 1
	 * Unlocks configuration register so that HBA AHCI Capabilities Register 
	 * can be modified.
	 * 2. SATA_BAR5 + 0xFC [11] = 0
	 * (CFG_CAP_SALP Disabled) Clearing this bit has the following effects:
	 * Support-Aggressive-Link-Power-Management Capability is hidden from 
	 * software in AHCI HBA Capabilities Register. 
	 * As a result,;software will not enable the HBA to aggressively enter 
	 * power-saving (Partial/Slumber) mode. 
	 * 6.12	Hiding Slumber and Partial State Capabilities in AHCI HBA Capabilities 
	 * Register 
	 * 2. SATA_BAR5 + 0xFC [27] = 1 (CFG_CAP_SALP Disabled)
	 * 3. SATA_PCI_config 0x40 [0]= 0
	 * Clears the bit to lock configuration registers so that AHCI HBA 
	 * Capabilities Register is read-only.
	 */
	set_sbcfg_enable_bits(sata_dev, 0x40, 1 << 0, 1 << 0);

	*(volatile u32 *)(sata_bar5 + 0xFC) &= ~(1 << 11);
	*(volatile u32 *)(sata_bar5 + 0xFC) |= 1 << 27;
	if(ati_sb_cfg.sata_port_mult_cap != SB_ENABLE){
		*(volatile u32 *)(sata_bar5 + 0xFC) &= ~(1 << 12);
	}

	if(rev >= REV_SB600_A21){
		if(ati_sb_cfg.sata_hot_plug_cap != SB_ENABLE){
			*(volatile u32 *)(sata_bar5 + 0xFC) &= ~(1 << 17);			
		}
	}

	set_sbcfg_enable_bits(sata_dev, 0x40, 1 << 0, 0 << 0);

	/* RPR6.11 Disabling SATA Interface Partial/Slumber States Power Management 
	 * Transitions (For all SB600 )
	 * Port0: SATA_BAR5 + 0x12C[11:08] = 0x03
	 * Port1: SATA_BAR5 + 0x1AC[11:08] = 0x03
	 * Port2: SATA_BAR5 + 0x22C[11:08] = 0x03
	 * Port3: SATA_BAR5 + 0x2AC[11:08] = 0x03
	 * Setting PxSCTL (Port X Serial ATA Control) register bits[11:8] (PxSCTL.IPM) 
	 * to 0x03 will disable interface power management states, 
	 * both Partial and Slumber. HBA is not allowed to initiate these two states, 
	 * and HBA must PMNAK any request from device to enter these states.
	 */
	*(volatile u32 *)(sata_bar5 + 0x12C) |= 3 << 8;
	*(volatile u32 *)(sata_bar5 + 0x1AC) |= 3 << 8;			
	*(volatile u32 *)(sata_bar5 + 0x22C) |= 3 << 8;			
	*(volatile u32 *)(sata_bar5 + 0x2AC) |= 3 << 8;

	/* restore pci cmd */
	_pci_conf_write(sata_dev, 0x04, reg04);

	return;
}

/*
 * sata_drive_detect :
 * detect the sata drive and put it into proper config
 */
static void sata_drive_detecting(void)
{
	int ports = 4, count;
	int timeout;
	u32	base, reg32, reg04;
	u32 val;
    pcitag_t sata_dev = _pci_make_tag(0, 18, 0);
	
	PRINTF_DEBUG();
	reg04 = _pci_conf_read(sata_dev, 0x04);
	/* force memory and io enabled */
	set_sbcfg_enable_bits(sata_dev, 0x04, 0x03 << 0, 0x03 << 0);
	
	/* ahci base addr */
	base = (_pci_conf_read(sata_dev, 0x24) & 0xfffffff0) | 0xA0000000;
	DEBUG_INFO("ahci base addr is %x \n", base);
	
	/* 4/PM 3/SM 2/PS 1/SS */
	while(ports){
		count = 3;	
		while(count){
			/* judge is the phy and communication is ok */
			delay(1000);	// delay 1 ms
			reg32 = *(volatile u32 *)(base + 0x128);
			DEBUG_INFO("sata status : reg128 0x%x\n", reg32);
			if((reg32 & 0x0000000F) == 0x03){
				goto drive_is_stable;	
			}

			/* test if bsy bit is set, it means the drive is connected */
			reg32 = *(volatile u32 *)(base + 0x120);
			if((reg32 & 0x80) == 0){
				val = *(volatile u32 *)(base + 0x128);
				if((val & 0x0f) != 0x1){
					goto drive_is_stable;	
				}
			}
			count--;
		}

		/* now drive is not stable even after waitting for 1 sec,so downgrade to GEN 1 */	
		reg32 = *(volatile u32 *)(base + 0x12C);
		if((reg32 & 0x0f) == 0x10){
			goto drive_is_stable;	// jump if already GEN1
		}
				
		/* store the sata status */
		ati_sb_cfg.sb600_sata_sts |= (0x8 << ports);
		
		/* set to GEN1 : reset the status*/
		reg32 = *(volatile u32 *)(base + 0x12C);
		reg32 = (reg32 & 0x0f) | 0x10;
		*(volatile u32 *)(base + 0x12C) = reg32;
		reg32 |= 0x01;
		*(volatile u32 *)(base + 0x12C) = reg32; // force 00B
		delay(1000); //wait 1 ms
		reg32 = *(volatile u32 *)(base + 0x12C);
		reg32 &= 0xfe;
		*(volatile u32 *)(base + 0x12C) = reg32;
		/* re detect the device again */
		ports--;
				continue;

drive_is_stable:
		reg32 = *(volatile u32 *)(base + 0x128);
		if ((reg32 & 0x0f) == 0x3){
			u16  sata_bar;
			
			sata_bar = _pci_conf_readn(sata_dev, 0x10, 2);
			if(ports & 0x01){
				sata_bar = _pci_conf_readn(sata_dev, 0x18, 2);
			}
			sata_bar &= ~(7);
			sata_bar += 0x06;

			if(ports <= 2){
				val |= 0xA0;
			}else{
				val |= 0xB0;
			}
			linux_outb(val, sata_bar);

			sata_bar++;
			timeout = 3000;
			while(timeout--){
				val = linux_inb(sata_bar);
				val &= 0x88000000;
				if(val == 0x00){
					break;
				}
				delay(10 * 1000);
			}
			/* update the status */
			ati_sb_cfg.sb600_sata_sts ^= (1 << (4 - ports));
		}
		base += 0x80; // increase status offset by 80h for next port
		--ports;
	} /* while(i) */

	/* restore PCI conf cmd */
	_pci_conf_write(sata_dev, 0x04, reg04);
	return;
}

/*
 * satachannel_reset
 * detect and reset sata channel
 */
void sb_sata_channel_reset(void)
{
	PRINTF_DEBUG();
	if(ati_sb_cfg.sata_channel == SB_ENABLE){
		if( (ati_sb_cfg.sata_class == 0) 		/* native ide mode */
			|| (ati_sb_cfg.sata_class == 3)		/* legacy ide mode */
			|| (ati_sb_cfg.sata_class == 4) ){	/* ide->ahci mode*/
			sata_drive_detecting();
		}
	}

	return;
}

void sb_last_sata_unconnected_shutdown_clock(void)
{
	u8 sata_class = ati_sb_cfg.sata_class;
	pcitag_t sata_dev = _pci_make_tag(0, 18, 0);
	u32 temp;

	PRINTF_DEBUG();
	if(ati_sb_cfg.sata_ck_au_off == SB_ENABLE){	// auto off
		if(ati_sb_cfg.sata_channel > 0){		// enable sata
			switch(sata_class){
				case 0 :
				case 3 :
				case 4 :
					temp = _pci_conf_read(sata_dev, 0x40);
					temp |= (ati_sb_cfg.sb600_sata_sts & 0xFF0F) << 16;
					_pci_conf_write(sata_dev, 0x40, temp);
				default :

			}
		}
	}

	return;
}

	
void sb_last_sata_cache(void)
{
	u32 base;
	u32 val;
	pcitag_t sata_dev = _pci_make_tag(0, 0x12, 0);

	PRINTF_DEBUG();
	if(ati_sb_cfg.sata_cache_base){
		/* store sata base, size and signature */
		base = ati_sb_cfg.sata_cache_base | 0xa0000000;
		*(volatile u32 *)(base + 0x08) = ati_sb_cfg.sata_cache_base;
		*(volatile u32 *)(base + 0x04) = ati_sb_cfg.sata_cache_size;
		*(volatile u8 *)(base + 0x00) = 'A';
		*(volatile u8 *)(base + 0x01) = 'T';
		*(volatile u8 *)(base + 0x02) = 'i';
		*(volatile u8 *)(base + 0x03) = 'B';

		/* disable IO access */
		set_sbcfg_enable_bits(sata_dev, 0x04, 1 << 0, 0 << 0);

		/* reserved bit */
		set_sbcfg_enable_bits(sata_dev, 0x40, 1 << 20, 1 << 20);

		/* set cache base addr */
		val = _pci_conf_read(sata_dev, 0x10);
		val &= 0x00ffffff;
		val |= (ati_sb_cfg.sata_cache_base >> 24) << 24;
		_pci_conf_write(sata_dev, 0x10, val);
	}

	return;
}


void sb_last_sata_init(void)
{
	pcitag_t dev = _pci_make_tag(0, 0x12, 0);
	u32 base;
	u32 reg04;

	PRINTF_DEBUG();
	if(ati_sb_cfg.sata_channel == SB_ENABLE){
		if(ati_sb_cfg.sata_class == 4){	// IDE-AHCI
			set_sbcfg_enable_bits(dev, 0x40, 1 << 0, 1 << 0);
			_pci_conf_write(dev, 0x08, 0x01060100);
			set_sbcfg_enable_bits(dev, 0x40, 1 << 0, 0 << 0);
			base = (_pci_conf_read(dev, 0x24) & 0xfffffff0) | 0xa0000000;
			*(volatile u8 *)(base + 0x04) |= 1 << 1;
		}
	}
	
	reg04 = _pci_conf_read(dev, 0x04);
	if((_pci_conf_read(dev, 0x24) & 0xfffffff0) == 0){
		_pci_conf_write(dev, 0x24, TEMP_SATA_BAR5_BASE);
	}
	base = (_pci_conf_read(dev, 0x24) & 0xfffffff0) | 0xa0000000;
	
	set_sbcfg_enable_bits(dev, 0x04, 3 << 0, 3 << 0);

	/* clear error status */
	*(volatile u32 *)(base + 0x130) = 0xffffffff;
	*(volatile u32 *)(base + 0x1B0) = 0xffffffff;
	*(volatile u32 *)(base + 0x230) = 0xffffffff;
	*(volatile u32 *)(base + 0x2B0) = 0xffffffff;

	_pci_conf_write(dev, 0x04, reg04);

	/* clear sata status */
	/* get ACPIGP0BLK base */
	base = (pm_ioread(0x28) << 8) | pm_ioread(0x29);

	DEBUG_INFO("sata bar5 base is %x\n", base);
	DEBUG_INFO("base[0] is %x\n", linux_inl(base));
	linux_outl(1 << 31, base);
	DEBUG_INFO("now, base[0] is %x\n", linux_inl(base));
	DEBUG_INFO("return from init sata\n");
	return;
}


