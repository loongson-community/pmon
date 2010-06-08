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

/*-------------------------------------------------------------------------*/

/*Pin Config for ALC880, ALC882 and ALC883*/
static struct ati_sb_aza_set_config_d4 AzaliaCodecAlc882TableStart[] = {
	{0x14,	0x01014010},
	{0x15,	0x01011012},
	{0x16,	0x01016011},
	{0x17,	0x01012014},
	{0x18,	0x01A19030},
	{0x19,	0x411111F0},
	{0x1a,	0x01813080},
	{0x1b,	0x411111F0},
	{0x1C,	0x411111F0},
	{0x1d,	0x411111F0},
	{0x1e,	0x01441150},
	{0x1f,	0x01C46160},
	{0xff, 0}	
};

/*Pin Config for ALC262*/
static struct ati_sb_aza_set_config_d4  AzaliaCodecAlc262TableStart[] = {
	{0x14,0x01014010},
	{0x15,0x411111F0},
	{0x16,0x411111F0},
//	{0x17,0x01012014},
	{0x18,0x01A19830},
	{0x19,0x02A19C40},
	{0x1a,0x01813031},
	{0x1b,0x02014C20},
	{0x1C,0x411111F0},
	{0x1d,0x411111F0},
	{0x1e,0x0144111E},
	{0x1f,0x01C46150},
	{0xff, 0}	
};

/*Pin Config for  ALC0861*/
static struct ati_sb_aza_set_config_d4 AzaliaCodecAlc861TableStart[] = {
	{0x01,0x8086C601},
	{0x0B,0x01014110},
	{0x0C,0x01813140},
	{0x0D,0x01A19941},
	{0x0E,0x411111F0},
	{0x0F,0x02214420},
	{0x10,0x02A1994E},
	{0x11,0x99330142},
	{0x12,0x01451130},
	{0x1F,0x411111F0},
	{0x20,0x411111F0},
	{0x23,0x411111F0},
	{0xff,0} 
};

static struct ati_sb_aza_set_config_d4 AzaliaCodecFpEnableTableStart[] = {
	{0x19,0x02A19040},
	{0x1b,0x02214020},
	{0xff,0}
};

/*Azalia Code Table*/
static struct ati_sb_aza_codec_tbl AzaliaCodecTableListStart[] = {
	{0x10ec0880,	AzaliaCodecAlc882TableStart},
	{0x10ec0882,	AzaliaCodecAlc882TableStart},
	{0x10ec0883,	AzaliaCodecAlc882TableStart},
	{0x10ec0885,	AzaliaCodecAlc882TableStart},
	{0x10ec0262,	AzaliaCodecAlc262TableStart},
	{0x10ec0861,	AzaliaCodecAlc861TableStart},
	{0xffffffff, 0}
};


/*
 * azalia_set_config_d4 :
 * config the externel codec parameters, 
 */
void azalia_set_config_d4(struct ati_sb_aza_set_config_d4 *p, u8 channel, u32 bar0)
{
	u8 val;
	u32 temp;

	while(p->nid != 0xff){
		if(p->nid == 1){
			val = 0x24;	// VERB will be 0x720/1/2/3(subsystem ID)
		}else{
			val = 0x20; // VERB will be 0x71C/D/E/F(subsystem ID)
		}
		val -= channel + 1;
		
		temp = p->nid << 20;	// [27:20] NID BIT
		temp |= channel << 28;	// [31:28] CAD BIT
		temp |= 0x700 << 8;		// [19:8] VERB ID

		while(*(volatile u32 *)(bar0 + 0x68) & (1 << 0));

		temp |= p->byte4 & 0x000000ff;
		temp |= val << 8;

		*(volatile u32 *)(bar0 + 0x60) = temp;
		delay(60);

		p++;
	}

	return;	
}

/*
 * sb_azalia_cfg :
 * AZALIA config for AC97 audio or AC97 Modem or just AZALIAZ
 */
void sb_azalia_cfg(void)
{
	/* enable GPIO45 pullup for AC97 detection */
	set_pm2_enable_bits(0xEB, 1 << 5, 0 << 5);

#ifndef CFG_UNSUPPORT_AC97
	/* Enable AZALIA and AC97 and MC97 before detection */
	set_pm_enable_bits(0x59, (1 << 0 ) | (1 << 1) | (1 << 3), (1 << 0) | (1 << 1) | (1 << 3));
#else
	/* Enable AZALIA, disable AC97 and MC97 before detection */
	set_pm_enable_bits(0x59, (1 << 0 ) | (1 << 1) | (1 << 3), 1 << 3);
#endif

	return;
}


/*
 * azalia_pincmd_config :
 * config the codec table properly
 */
void azalia_pincmd_config(u32 bar0, u8 channel)
{
	struct ati_sb_aza_codec_tbl *p;
	u32 response, send;
	int flag = 0;
	
	PRINTF_DEBUG();
	if(ati_sb_cfg.aza_pin_cfg == SB_ENABLE){
		send = (channel << 28) | 0x000f0000;
		*(volatile u32 *)(bar0 + 0x60) = send;
		delay(60); //delay 60 us
		response = _pci_conf_read(bar0, 0x64);

		/* ac97 setting */
		p = AzaliaCodecTableListStart;
		while(p->codec_id != -1){
			if(p->codec_id == response){ // id is matched
				azalia_set_config_d4(p->tbl_ptr, channel, bar0);
			}
			p++;
		}

		/* panel setting */
		if(ati_sb_cfg.front_panel == AZALIA_ENABLE){
			flag = 1;
		}else if(ati_sb_cfg.front_panel == AZALIA_DISABLE){
			flag = 0;
		}else{
			flag = 0;
			if(ati_sb_cfg.fp_detected == SB_ENABLE){
				flag = 1;
			}
		}
		if(flag){
			azalia_set_config_d4(AzaliaCodecFpEnableTableStart, channel, bar0);
		}
	}

	return;
}

/*
 * sb_azalia_post_cfg :
 * his routine detects Azalia and if present initializes Azalia
 */
void sb_azalia_post_cfg(void)
{
	u8 rev = get_sb600_revision();
	pcitag_t sm_dev, azalia_dev;
	u32 bar0;
	u8 temp;
	u8 flag = 0;
	int i;
	int channel;

	PRINTF_DEBUG();
    /* judge the device */
    sm_dev = _pci_make_tag(0, 20, 0);
	azalia_dev = _pci_make_tag(0, 20, 2);
   	/*
	 * check if azalia is disabled as a setup option
     * 0:AUTO  1:Disable  2:Enable
	 */
	DEBUG_INFO("azalia config : 0x%x\n", ati_sb_cfg.azalia);
	if(ati_sb_cfg.azalia == AZALIA_DISABLE){		// disable mode
		goto disable_azalia;
	}
	if(ati_sb_cfg.azalia == AZALIA_ENABLE){			// enable mode
		goto enable_azalia;
	}
	/* disable it when rev is A11,A12 when in auto mode */
	if(rev <= REV_SB600_A12){						// auto mode
		goto disable_azalia;
	}
      
	/* enabel azalia routine */	
enable_azalia :
	/* enable device memory access */
	set_sbcfg_enable_bits(azalia_dev, 0x04, 1 << 1, 1 << 1);
	/* config subsystem id */
	_pci_conf_write(azalia_dev, 0x2c, ATI_AZALIA_ID);
	/* check bar0 */
	bar0 = _pci_conf_read(azalia_dev, 0x10) & 0xFFFFC000;
	if( (bar0 == 0) || (bar0 == 0xffffffff) ){
		goto disable_azalia;
	}
	bar0 |= 0xa0000000;
	DEBUG_INFO("azalia bar0 : 0x%x\n", bar0);

	/* get SDIN config in idnex 0 */
	_pci_conf_write(sm_dev, 0xf8, 0);
	temp |= ati_sb_cfg.aza_sdin3;
	temp <<= 2;
	temp |= ati_sb_cfg.aza_sdin2;
	temp <<= 2;
	temp |= ati_sb_cfg.aza_sdin1;
	temp <<= 2;
	temp |= ati_sb_cfg.aza_sdin0;
	_pci_conf_writen(sm_dev, 0xfc, temp, 1);
	DEBUG_INFO("0xfc reg val : 0x%x\n", _pci_conf_read(sm_dev, 0xfc));
	
	/* route INTA for PCI int */
	_pci_conf_writen(sm_dev, 0x63, 0, 1);

	DEBUG_INFO("begin controller reset of azalia\n");
	/* azalia controller reset */
	i = 10;
	while(i--){
		/* reset the controller */
		*(volatile u32 *)(bar0 + 0x08) |= 1 << 0;
		delay(1000); // delay 1 ms
		if( (*(volatile u32 *)(bar0 + 0x08) & 0x01) != 0 ){
			break;
		}
	}
	if(i <= 0){
		DEBUG_INFO("azalia controller reset timeout.\n");
		return ; //timeout
	}
	DEBUG_INFO("end controller reset of azalia : 0x%x\n", *(volatile u32 *)(bar0 + 0x08));

	DEBUG_INFO("wake status of azalia : 0x%x\n", *(volatile u16 *)(bar0 + 0x0e));
	/* reset the controller done. */	
	delay(1000);
	/* wake status register */
	if((*(volatile u16 *)(bar0 + 0x0e) & 0x000f) != 0){
		/* at lease one azalia state changed */
		/* get SDIN config */		
		_pci_conf_write(sm_dev, 0xf8, 0);
		temp = _pci_conf_readn(sm_dev, 0xfc, 1);

		channel = 0; // channel index
		do{
			if(!(temp & (1 << 0))){
				if(temp & (1 << 1)){
					azalia_pincmd_config(bar0, channel);
				}
			}
			temp >>= 2;
		}while(++channel < 4);

		flag = 1;
	}

	if(ati_sb_cfg.azalia == AZALIA_ENABLE){
		flag = 1;
	}

	DEBUG_INFO("azalia flag : 0x%x\n", flag);
	if(flag){
		/* re do Clear Reset*/
		do {
			*(volatile u16 *)(bar0 + 0x0c) = 0;
			*(volatile u32 *)(bar0 + 0x08) &= ~(1 << 0);
			delay(1000);
		}while( (*(volatile u32 *)(bar0 + 0x08) & (1 << 0)) );

		DEBUG_INFO("azalia clear reset : 0x%x\n", *(volatile u32 *)(bar0 + 0x08));
		return;
	}

disable_azalia :
	DEBUG_INFO("azalia disable routine\n");
	/* disable all */
	_pci_conf_writen(azalia_dev, 0x4, 0, 2);
	set_pm_enable_bits(0x59, 1 << 3, 0 << 3);
	/* set pin route to ac97 */	
	_pci_conf_write(sm_dev, 0xF8, 0);
	set_sbcfg_enable_bits(sm_dev, 0xFC, 0xFF << 0, 0x55);

	return;
}
