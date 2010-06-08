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

/*-------------------------------------------------------------------------------------*/

/* NOTICE: I can't find the defination, use default 0*/
static struct acpi_base sb_acpi_base_table = {
	0x9000,			/* ACPI PM1 event block : should size 4bytes */
	0x9010,			/* ACPI PM1 control block : should size 2bytes */
	0x9020,			/* ACPI PM timer block : should size 4bytes */
	0x9030,			/* ACPI PM cpu control block : should size 6bytes, so at least 8bytes */
	0x9040,			/* ACPI PM general purpose event block : should size 8bytes 2 * 32bits */
	0x9050,			/* ACPI SMI command block : should be 2bytes */
	0x9060,			/* ACPI PM additional control block : should be 1bytes */
	0x9070			/* ACPI Speed Step control block : should be 1bytes */
};

static u16 sb_bdf_table[]= {
#ifndef  CFG_SATA1_SUBSYSTEM_ID
    ATI_SATA1_BUS_DEV_FUN,
#endif
#ifndef  CFG_OHCI0_SUBSYSTEM_ID
    ATI_OHCI0_BUS_DEV_FUN,
#endif
#ifndef  CFG_OHCI1_SUBSYSTEM_ID
    ATI_OHCI1_BUS_DEV_FUN,
#endif
#ifndef  CFG_OHCI2_SUBSYSTEM_ID
    ATI_OHCI2_BUS_DEV_FUN,
#endif
#ifndef  CFG_OHCI3_SUBSYSTEM_ID
    ATI_OHCI3_BUS_DEV_FUN,
#endif
#ifndef  CFG_OHCI4_SUBSYSTEM_ID
    ATI_OHCI4_BUS_DEV_FUN,
#endif
#ifndef  CFG_EHCI_SUBSYSTEM_ID
    ATI_EHCI_BUS_DEV_FUN,
#endif
#ifndef  CFG_SMBUS_SUBSYSTEM_ID
    ATI_SMBUS_BUS_DEV_FUN,
#endif
#ifndef  CFG_IDE_SUBSYSTEM_ID
    ATI_IDE_BUS_DEV_FUN,
#endif
#ifndef  CFG_AZALIA_SUBSYSTEM_ID
    ATI_AZALIA_BUS_DEV_FUN,
#endif
#ifndef  CFG_LPC_SUBSYSTEM_ID
    ATI_LPC_BUS_DEV_FUN,
#endif
#ifndef  CFG_AC97_SUBSYSTEM_ID
    ATI_AC97_BUS_DEV_FUN,
#endif
#ifndef  CFG_MC97_SUBSYSTEM_ID
    ATI_MC97_BUS_DEV_FUN,
#endif
    0xff 
};


/*-----------------------------------------------------------------------------*/

/*
 * sb600_pci_cfg
 * Varies devices basic pre setting
 */
static void sb_pci_cfg(void)
{
	pcitag_t dev;
	u8 rev;
	u8 reg8, val;

	/* get chip revision */
	rev = get_sb600_revision();
//printf("-----1-----\n");
	/*** SMBUS init routine ***/
	dev = _pci_make_tag(0, 20, 0);

	/* enable A21 revision ID */
	set_sbcfg_enable_bits(dev, 0x70, 1 << 8, 1 << 8);
	
	/* enable watchdog timer */
	set_sbcfg_enable_bits(dev, 0x40, 1 << 11, 1 << 11);

	/* enable usbreset by pcireset enable */
	set_pm_enable_bits(0x65, 1 << 4, 1 << 4);

//printf("-----2-----\n");
	/*** IDE init routine ***/
	dev = _pci_make_tag(0, 20, 1);

	/* enable IDE Dynamic power saving */
	set_sbcfg_enable_bits(dev, 0x6c, 0x000fffff << 0, 0x000fffff << 0);
	
	/* eanble IDE explicit pre-fetch */
	set_sbcfg_enable_bits(dev, 0x63, 1 << 0, 0 << 0);

	/*** LPC init routine ***/
	dev = _pci_make_tag(0, 20, 3);

	/* enable lpc dma function */
	set_sbcfg_enable_bits(dev, 0x40, 1 << 2, 1 << 2);
	
	/*disalbe lpc timeout*/
	set_sbcfg_enable_bits(dev, 0x48, 1 << 7, 0 << 7);

//printf("-----3-----\n");
	/*** PCIB init routine ***/
	dev = _pci_make_tag(0, 20, 4);
	if(ati_sb_cfg.arb2_en){
		/* arbiter-2 enable : enable new PCI arbiter instead the old one */
		set_sbcfg_enable_bits(dev, 0x64, 1 << 9, 1 << 9);
	}

	/* rpr4.1 enable pci-bridge subtractive decode */
	set_sbcfg_enable_bits(dev, 0x40, 1 << 5, 1 << 5);
	set_sbcfg_enable_bits(dev, 0x48, 1 << 31, 1 << 31);

//printf("-----5-----\n");
    /* rpr4.2 pci-bridge upstream dual address window */
	set_sbcfg_enable_bits(dev, 0x50, 1 << 0, 1 << 0);

	/* rpr4.3 pci bus 64-bit DMA read access*/
	set_sbcfg_enable_bits(dev, 0x48, 1 << 28, 1 << 28);

	/* rpr4.4 pci bus DMA write cacheline alignment */
	set_sbcfg_enable_bits(dev, 0x40, 1 << 1, 1 << 1);

	/* rpr4.6 DMA read command match */
	set_sbcfg_enable_bits(dev, 0x48, 1 << 30, 1 << 30);

//printf("-----6-----\n");
	/* rpr4.7 enable idle to gnt# che */
	set_sbcfg_enable_bits(dev, 0x48, 1 << 24, 1 << 24);

    /* rpr4.8 Adjusts the GNT# timing */
	set_sbcfg_enable_bits(dev, 0x64, 1 << 12, 1 << 12);

    /*rpr4.9  Fast Back to Back transactions support */
	set_sbcfg_enable_bits(dev, 0x48, 1 << 2, 1 << 2);

    /* rpr4.10 Enable Lock Operation */
	set_sbcfg_enable_bits(dev, 0x48, 1 << 3, 1 << 3);
	set_sbcfg_enable_bits(dev, 0x40, 1 << 2, 1 << 2);

    /*rpr4.11 Enable additional optional PCI clock */
//	set_sbcfg_enable_bits(dev, 0x64, 1 << 8, 1 << 8);

    /* rpr4.12 Disable Fewer-Retry Mode for A11-A13 only. */
	if(rev < REV_SB600_A13 + 1){
		set_sbcfg_enable_bits(dev, 0x64, (1 << 4) | (1 << 5), (0 << 4) | (0 << 5));
	}

    /* rpr4.13 Enable One-Prefetch-Channel Mode */
	set_sbcfg_enable_bits(dev, 0x64, 1 << 20, 1 << 20);

    /* rpr4.14 Disable Downstream flush fix ,for A12 only */
	if(rev == REV_SB600_A12){
		set_sbcfg_enable_bits(dev, 0x64, 1 << 18, 1 << 18);
	} 

	/*** AZALIA init routine ***/
	dev = _pci_make_tag(0, 20, 2);

	/* azalia: enable s1 wake */
	set_sbcfg_enable_bits(dev, 0x4C, 1 << 0, 1 << 0);

    /* rpr2.20 disable timer irq enhancement */
	dev = _pci_make_tag(0, 20, 0);
	set_sbcfg_enable_bits(dev, 0xAC, 1 << 21, 1 << 21);

	/*** USB EHCI MSI init routine ***/
	dev = _pci_make_tag(0, 19, 5);

	/* rpr5.10 usb ehci msi support setting*/
	set_sbcfg_enable_bits(dev, 0x50, 1 << 6, 1 << 6);

	/*** USB OHCI MSI init routine ***/	
	dev = _pci_make_tag(0, 19, 0);

	/* rpr5.11 usb ohci0 msi support settig*/
	set_sbcfg_enable_bits(dev, 0x50, 0x1F << 8, 0x1F << 8);

	/*** SATA MSI init routine ***/
	dev = _pci_make_tag(0, 18, 0);

	/* rpr6.8 hide sata msi capability (for A13 and up)*/
	set_sbcfg_enable_bits(dev, 0x40, 1 << 23, 1 << 23);

	/*** LPC MSI init routine ***/
	dev = _pci_make_tag(0, 20, 3);

	/* rpr7.5 disables lpc msi capability */
	set_sbcfg_enable_bits(dev, 0x78, 1 << 1, 0 << 1);

#if 		0
	/*** AC97 AUDIO MSI init routine ***/
	dev = _pci_make_tag(0, 20, 5);
	
	/* rpr8.4 disables ac97 audio msi capability */
	set_sbcfg_enable_bits(dev, 0x40, 1 << 16, 0 << 16);

	/*** AC97 MODEM MSI init routine ***/
	dev = _pci_make_tag(0, 20, 6);
	
	/* rpr9.3 disables ac97 modem msi capability */
	set_sbcfg_enable_bits(dev, 0x40, 1 << 16, 0 << 16);

	/*** IDE MSI init routine ***/
	dev = _pci_make_tag(0, 20, 1);
	
	/* rpr9.3 disables ide msi capability */
	set_sbcfg_enable_bits(dev, 0x70, 1 << 16, 0 << 16);
#endif

	if(rev > REV_SB600_A12){
        /* rpr2.16 C-State Reset, PMIO 0x9f[7]. */
		set_pm_enable_bits(0x9F, 1 << 7, 1 << 7);
        
		/* rpr2.17 PCI Clock Period will increase to 30.8ns. 0x53[7]. */
		set_pm_enable_bits(0x53, 1 << 7, 1 << 7);
	}

	/* enable, disable pci clock */
	dev = _pci_make_tag(0, 20, 4);

	/* setting PCI clock enable bits for lower 4 clocks */
	val = ati_sb_cfg.pciclk;
	reg8 = _pci_conf_readn(dev, 0x42, 1);
	reg8 &= (val << 2) | 0xC3;
	_pci_conf_writen(dev, 0x42, reg8, 1);
	/* setting PCI clock enable bits for upper 4 clocks */
	reg8 = _pci_conf_readn(dev, 0x4A, 1);
	reg8 &= (val >> 4) | 0xF0;
	_pci_conf_writen(dev, 0x4A, reg8, 1);

	return;	
}

/*
 * sb_ablink_cfg :
 * sb600 ablink configuration
 */
static void sb_ablink_cfg(pcitag_t sm_dev)
{
	/* disable pmio decoding when ab is setting */
	set_sbcfg_enable_bits(sm_dev, 0x64, 1 << 2, 0 << 2);

	/* configure the k8 system with ABCFG reg table */
	if(get_sb600_platform() == K8_ID){
		if(ati_sb_cfg.ds_pt == SB_ENABLE){
			/* 3.8 Enabling Downstream Posted Transactions to Pass Non-Posted Transactions for 
			 * the K8 Platform ABCFG 0x10090[8]=1			 *
			 * 2.10 ABCFG 0x10090 [16] = 1 ensures the SMI# message to be sent before 
			 * the IO command is completed. The ordering of SMI# and IO is important for the 
			 * IO trap to work properly on the K8 platform.
			 */
			abcfg_reg(0x10090, (1 << 8) | (1 << 16), (1 << 8) | (1 << 16));
		}
	}
	
	/* configure sb600 with ABCFG reg table */
	
	/* 3.2	Enabling UpStream DMA Access AXCFG: 0x04 [2] = 1 */
	axcfg_reg(0x04, 1 << 2, 1 << 2);

	/* 3.4	Enabling OHCI Prefetch for Performance Enhancement  OHCI prefetch:   ABCFG 0x80 [0] = 0
	 * 3.5	Setting B-Link Prefetch Mode ABCFG 0x80 [17] = 1 ABCFG 0x80 [18] = 1
	 * 3.5 Enabling OHCI Prefetch for Performance Enhancement
	 */
	abcfg_reg(0x80, (1 << 0) | (1 << 17) | (1 << 18), (1 << 0) | (1 << 17) | (1 << 18));

	/* 3.6	Disable B-Link clients credit variable in downstream arbitration equation (for All Revisions)
	 * 		ABCFG 0x9c [0] = 1 Disable credit variable in downstream arbitration equation
	 * 		ABCFG 0x9c [1] = 1
	 */
	abcfg_reg(0x9C, (1 << 0) | (1 << 1), (1 << 0) | (1 << 1));

	/* 3.3	Enabling IDE/PCIB Prefetch for Performance Enhancement
	 * 	IDE prefetch    ABCFG 0x10060 [17] = 1   ABCFG 0x10064 [17] = 1
	 * 	PCIB prefetch   ABCFG 0x10060 [20] = 1   ABCFG 0x10064 [20] = 1
	 */
	abcfg_reg(0x10060, (1 << 17) | (1 << 20), (1 << 17) | (1 << 20));
	abcfg_reg(0x10064, (1 << 17) | (1 << 20), (1 << 17) | (1 << 20));

	/* 3.7	Enabling Detection of Upstream Interrupts ABCFG 0x94 [20] = 1 
	 * ABCFG 0x94 [19:0] = cpu interrupt delivery address [39:20].
	 */
	abcfg_reg(0x94, ~0xfff00000, 0x00100FEE);

	/* 3.9	Enabling AB & BIF clock gating ABCFG 0x54[24] = 1
	 * 	ABCFG 0x10054[31:16] = 0104h
	 * 	ABCFG 0x98 = 00004700h
	 */
	
	/* 3.10	Enabling AB Int_Arbiter Enhancement ABCFG 0x10054[15:0] =07FFh */
	abcfg_reg(0x10054, ~0x0, 0x000007FF | ((ati_sb_cfg.al_clk_delay & 0xff) << 16) | ((ati_sb_cfg.al_clk_gate & 0x01) << 24));
	abcfg_reg(0x98, ~0x0, 0x00004700);

	/* misc control */
	abcfg_reg(0x54, 0x00FF0000, (ati_sb_cfg.bl_clk_delay & 0xff) << 16);
	abcfg_reg(0x54, 0x01000000, (ati_sb_cfg.bl_clk_gate & 0x01) << 24);

	/* enable pmio decoding when ab is ok */
	set_sbcfg_enable_bits(sm_dev, 0x64, 1 << 2, 1 << 2);

	return;
}

/*
 * sb_pmio_cfg :
 * PM ACPI init
 * */
static void sb_pmio_cfg(void)
{
	
	int i, index = 0x20;
	u8 *p;

	p = (u8 *)(&sb_acpi_base_table);
	/* init acpi table base */
	for(i = 0; i < 16; i++){
		pm_iowrite(index + i, p[i]);
	}
	for(i = 0; i < 16; i += 2)
		DEBUG_INFO("sm_pmio_cfg : acpi all bases : 0x%x\n", (pm_ioread(index + i + 1) << 8) | pm_ioread(index + i));

	/* init pmio table */
	set_pm_enable_bits(0x0E, ~0xff, (1 << 2) | (1 << 3));
	set_pm_enable_bits(0x10, ~0x3E, (1 << 1) | (1 << 3) | (1 << 5));
	set_pm_enable_bits(0x03, 1 << 0, 0 << 0);
#ifdef	CFG_P92_TRAP_ON_PIO
#if	CFG_P92_TRAP_ON_PIO < 8
	set_pm_enable_bits(P92_TRAP_EN_REG, P92_TRAP_EN_BIT, 0);
#endif
#endif
	set_pm_enable_bits(0x01, ~0xff, 0x97);
	set_pm_enable_bits(0x05, ~0xff, 0xFF);
	set_pm_enable_bits(0x06, ~0xff, 0xFF);
	set_pm_enable_bits(0x07, ~0xff, 0xFF);
	set_pm_enable_bits(0x0F, ~0xff, 0x1F);
	set_pm_enable_bits(0x1D, ~0xff, 0xFF);
	set_pm_enable_bits(0x39, ~0xff, 0xFF);

	/* clear smi source */
	/* clear enable bits */	
	linux_outl(0, (sb_acpi_base_table.gpe0) + 4);
	linux_outw(0, (sb_acpi_base_table.pm1evt) + 2);

	/* clear status bits*/
	linux_outl(-1, sb_acpi_base_table.gpe0);
	linux_outw(-1, sb_acpi_base_table.pm1evt);

	return;
}

/*
 * sb_arbiter_cfg :
 * enable sb arbiter
 */
static void sb_arbiter_cfg(void)
{
	u16 base;

	/* get APCI PM additional control block base address */
	base = (pm_ioread(APCI_PMAC_BASE + 1) << 8) | pm_ioread(APCI_PMAC_BASE);
	
	/* write 0 to enable arbiter*/
	linux_outb(0, base + 0x00);

	return;
}

/*
 * sb_usb_pre_init :
 * sb pre init usb device
 */
static void sb_usb_pre_init(void)
{
	pcitag_t sm_dev, ohci0_dev;
	u8 rev = get_sb600_revision();
	u16 oc_map;
	u32	val;
	int i;
	
	sm_dev = _pci_make_tag(0, 20, 0);
	ohci0_dev = _pci_make_tag(0, 19, 0);
	
	/* disable all usb controllers and then enable them according to setup config */
	set_sbcfg_enable_bits(sm_dev, 0x68, 0x3F << 0, 0);

	if(ati_sb_cfg.usb11 == SB_ENABLE){
		/* enable all OHCI controllers */
		set_sbcfg_enable_bits(sm_dev, 0x68, 0x3E << 0, 0x3E << 0);
		/* setting USB controller link to which bridge : '1' ALINK, '0' BLINK */
		if(ati_sb_cfg.usb_xlink){
			set_sbcfg_enable_bits(ohci0_dev, 0x50, 1 << 0, 1 << 0);
		}

		if(ati_sb_cfg.usb20 == SB_ENABLE){
			/* enable EHCI controller */
			set_sbcfg_enable_bits(sm_dev, 0x68, 1 << 0, 1 << 0);
		}

#ifdef CFG_ATI_USB_OC_MAP0
		/* enable USB OverCurrent function */
		_pci_conf_write(ohci0_dev, 0x58, CFG_ATI_USB_OC_MAP0);
		ati_sb_cfg.usb_oc_map0 = CFG_ATI_USB_OC_MAP0;
		val = CFG_ATI_USB_OC_MAP0;
		for(i= 0; i < 8; i++){
			oc_map |= (1 << (val & 0x0000000f));	
			val >>= 4;
		}
#endif

#ifdef CFG_ATI_USB_OC_MAP1	
		_pci_conf_write(ohci0_dev, 0x5c, CFG_ATI_USB_OC_MAP1 & 0x000000ff);
		ati_sb_cfg.usb_oc_map1 = CFG_ATI_USB_OC_MAP1 & 0x000000ff;
		val = CFG_ATI_USB_OC_MAP1 & 0x000000ff;
		for(i= 0; i < 2; i++){
			oc_map |= (1 << (val & 0x0000000f));	
			val >>= 4;
		}
#endif
		
		/* store the OC map */
		if(ati_sb_cfg.usb_oc_wa){
			ati_sb_cfg.acaf = oc_map;
		}

		/* SB600 A21 OHCI0_PCI_Config 0x50 [15] = 1 
		 * Enables prevention of HC accessing memory address 0 
		 */
		if(rev == REV_SB600_A21){
			set_sbcfg_enable_bits(ohci0_dev, 0x50, 1 << 15, 1 << 15);
		
			/* SB600 A21 EHCI_PCI_Config 0x54 [0] = 1
			 * Enables USB PHY PLL Reset signal to come from ACPI
			 * Commented the following code since latest RPR recommends not to 
			 * enable this feature.
			 */
			//set_sbcfg_enable_bits(ohci0_dev, 0x54, 1 << 0, 1 << 0);
		}

		/* SB_OHCI_REG50[12] = 1 Disables SMI handshake in between USB and ACPI 
		 * for USB legacy support.   
		 * BIOS should always set this bit to prevent the malfunction on 
		 * USB legacy keyboard/mouse support. 
		 */
		set_sbcfg_enable_bits(ohci0_dev, 0x51, 1 << 4, 1 << 4);

		/* SB_OHCI_REG50[10] = 0 Disables OHCI 64byte data cache function to 
		 * avoid the potential of data packet corruption for USB1.1 device.
		 */
		set_sbcfg_enable_bits(ohci0_dev, 0x51, 1 << 2, 0 << 2);
	}

	return;
}


/*
 * sb_pci_pre_init
 */
static void sb_pci_pre_init(void)
{
	pcitag_t sm_dev;
	u8 rev = get_sb600_revision();
	u8 type = get_sb600_platform();

	sm_dev = _pci_make_tag(0, 20, 0);
	
	if(ati_sb_cfg.osc_en == SB_ENABLE){
		ati_sb_cfg.acaf |= (1 << 19);	
	}
			
	if(ati_sb_cfg.spd_spec == SB_ENABLE){
		set_pm_enable_bits(0x42, 1 << 7, 1 << 7);
	}else{
		set_pm_enable_bits(0x42, 1 << 7, 0 << 7);
	}
	
	/* rpr 2.22 pll reset */
	if(rev >= REV_SB600_A21){
		set_pm_enable_bits(0x86, 1 << 7, 1 << 7);
	}

	/* rpr 2.13 usb set bm_sts */
	set_pm_enable_bits(0x66, 1 << 6, 1 << 6);
	if(type == K8_ID){	
		if(rev == REV_SB600_A21){
			set_pm_enable_bits(0x66, 1 << 6, 0 << 6);
		}
	}

	if( (type == P4_ID) && (rev >= REV_SB600_A21) ){
		set_pm_enable_bits(0x51, 1 << 6, 1 << 6);		
	}

	/* pcie native mode */
	if(rev >= REV_SB600_A21){
		set_pm_enable_bits(0x55, 1 << 5, 1 << 5);
	}

	if(ati_sb_cfg.ide_controller == SB_ENABLE){
		set_pm2_enable_bits(0xE5, 1 << 2, 1 << 2);
	}

	return;
}

/*
 * sb_sid_cfg :
 * config and write the sb subsystem id
 */
static void sb_sid_cfg(void)
{
	int i;
	u32 id;
	pcitag_t dev;
	u32 sata_class;
	
#if CFG_SATA1_SUBSYSTEM_ID
	dev = _pci_make_tag(0, 18, 0);	
	_pci_conf_write(dev, 0x2C, CFG_SATA1_SUBSYSTEM_ID);
#endif

#if CFG_OHCI0_SUBSYSTEM_ID
	dev = _pci_make_tag(0, 19, 0);	
	_pci_conf_write(dev, 0x2C, CFG_OHCI0_SUBSYSTEM_ID);
#endif
#if CFG_OHCI1_SUBSYSTEM_ID
	dev = _pci_make_tag(0, 19, 1);	
	_pci_conf_write(dev, 0x2C, CFG_OHCI1_SUBSYSTEM_ID);
#endif
#if CFG_OHCI2_SUBSYSTEM_ID
	dev = _pci_make_tag(0, 19, 2);	
	_pci_conf_write(dev, 0x2C, CFG_OHCI2_SUBSYSTEM_ID);
#endif
#if CFG_OHCI3_SUBSYSTEM_ID
	dev = _pci_make_tag(0, 19, 3);	
	_pci_conf_write(dev, 0x2C, CFG_OHCI3_SUBSYSTEM_ID);
#endif
#if CFG_OHCI4_SUBSYSTEM_ID
	dev = _pci_make_tag(0, 19, 4);	
	_pci_conf_write(dev, 0x2C, CFG_OHCI4_SUBSYSTEM_ID);
#endif

#if CFG_EHCI_SUBSYSTEM_ID
	dev = _pci_make_tag(0, 18, 5);	
	_pci_conf_write(dev, 0x2C, CFG_EHCI_SUBSYSTEM_ID);
#endif
#if CFG_SMBUS_SUBSYSTEM_ID
	dev = _pci_make_tag(0, 20, 0);	
	_pci_conf_write(dev, 0x2C, CFG_SMBUS_SUBSYSTEM_ID);
#endif
#if CFG_IDE_SUBSYSTEM_ID
	dev = _pci_make_tag(0, 20, 1);	
	_pci_conf_write(dev, 0x2C, CFG_IDE_SUBSYSTEM_ID);
#endif
#if CFG_AZALIA_SUBSYSTEM_ID
	dev = _pci_make_tag(0, 20, 2);	
	_pci_conf_write(dev, 0x2C, CFG_AZALIA_SUBSYSTEM_ID);
#endif
#if CFG_LPC_SUBSYSTEM_ID
	dev = _pci_make_tag(0, 20, 3);	
	_pci_conf_write(dev, 0x2C, CFG_LPC_SUBSYSTEM_ID);
#endif
#if CFG_AC97_SUBSYSTEM_ID
	dev = _pci_make_tag(0, 20, 5);	
	_pci_conf_write(dev, 0x2C, CFG_AC97_SUBSYSTEM_ID);
#endif
#if CFG_MC97_SUBSYSTEM_ID
	dev = _pci_make_tag(0, 20, 6);	
	_pci_conf_write(dev, 0x2C, CFG_MC97_SUBSYSTEM_ID);
#endif

	/* if all the defined is ok, now set default ID */	
	i = 0;
	while(sb_bdf_table[i] != 0xff){
		id = _pci_conf_read(sb_bdf_table[i], 0x00);
		/* setting sata diff SID for diff mode */
		if(sb_bdf_table[i] == ATI_SATA1_BUS_DEV_FUN){
			sata_class = ati_sb_cfg.sata_class;
			if(sata_class == 4){
				sata_class = 2;
			}
			sata_class <<= 16;
			id &= ~(1 << 16);
			id |= sata_class;
		}
		/* setting other devices */
		_pci_conf_write(sb_bdf_table[i], 0x2C, id);
		++i;
	}
	/* fix Azalia subsystem id */	
	_pci_conf_write(ATI_AZALIA_BUS_DEV_FUN, 0x2C, ATI_AZALIA_ID);

	return;
}

/*---------------------------------------------------------------------------------*/

/*
 * sb_pre_init :
 * SB600 init before pci emulation
 */
void sb_pre_init(void)
{
	pcitag_t sm_dev;
	u16 sm_id;

	DEBUG_INFO("\n++++++++++++++++++++++++++++++++++++++++++SB PRE STAGE : start with type(0x%x), revision(0x%x)++++++++++++++++++++++++++++++++++++++\n", get_sb600_platform(), get_sb600_revision());

	/* judge the device : liujl ??? */
	sm_dev = _pci_make_tag(0, 20, 0);
	sm_id = _pci_conf_readn(sm_dev, 0x02, 2);
	if(sm_id != SB600_DEVICE_ID){
		if(sm_id != 0x43C5){
		DEBUG_INFO("SB PRE STAGE : out with ID mismatch (0x%x) match(0x%x)\n", sm_id, SB600_DEVICE_ID);
		return;
		}
	}
	DEBUG_INFO("SB PRE STAGE : ID judge 0x%x\n", sm_id);

	/*
	 *  For RTC
	 *  This register is protected by RTC battery and this registers sequence needs to be
	 *  written every time battery is removed (CMOS is corrupted).  To simplfy the programming
	 *  this register can also be written on every power up (there is no harm).
	 *  If this register is not written, after cmos corruption, there will be few seconds loss on
	 *  every power up
	 */
	_pci_conf_write(sm_dev, 0x7c, 0x02);
	_pci_conf_write(sm_dev, 0x7c, 0x00);

	/* SourthBridge pre init for every device */
	sb_pci_cfg();
	DEBUG_INFO("SB PRE STAGE : sb pci cfg done.\n");
	/* config the sb ablink */
	sb_ablink_cfg(sm_dev);
	DEBUG_INFO("SB PRE STAGE : sb ablink cfg done.\n");
	/* config enable the AC97 if needed */
	sb_azalia_cfg();
	DEBUG_INFO("SB PRE STAGE : sb azalia cfg done.\n");
	/* pm io module pre init */
	sb_pmio_cfg();
	DEBUG_INFO("SB PRE STAGE : sb pmio cfg done.\n");
	/* config ablink arbiter properly */
	sb_arbiter_cfg();
	DEBUG_INFO("SB PRE STAGE : sb arbiter cfg done.\n");
#if 0
	/* wgl modify */
	/* sb sata device pre init */
	sb_sata_pre_init();
	DEBUG_INFO("SB PRE STAGE : sb sata pre cfg done.\n");
	/* sb usb device pre init */
	sb_usb_pre_init();
	DEBUG_INFO("SB PRE STAGE : sb usb pre cfg done.\n");
	/* sb sata real init */
	sb_sata_init();
	DEBUG_INFO("SB PRE STAGE : sb sata init done.\n");
#endif
	/* sb all device init */
	sb_pci_pre_init();
	DEBUG_INFO("SB PRE STAGE : sb pci pre init done.\n");
	/* sb subsystem id pre init */
	sb_sid_cfg();
	DEBUG_INFO("SB PRE STAGE : sb sid cfg done.\n");

	DEBUG_INFO("------------------------------------------SB PRE STAGE DONE-------------------------------\n");

	return;
}

/*---------------------------------------------------------------------------------*/
