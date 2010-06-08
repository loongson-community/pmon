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
 *
 * PCIE_PRE :
 * 	pre init all PCIE ports, including 2-GFX ports and upto 4-GPP ports and
 * 	1-SB ports(AB-LINK)
 * 	after this init, all the ports will be configured with width, lanes, reversal
 * 	or maybe hiden according to the user configuration and the actual device status.
 * 	You need to hide unused ports or lanes for powersaving or other problems.
 */

/*-----------------------------------------------------------------------*/

#include <stdio.h>
#include <stdarg.h>
#include <sys/linux/types.h>
#include <include/bonito.h>
#include <machine/pio.h> 
#include <include/rs690.h>
#include <pci/rs690_cfg.h>
#include "rs690_config.h"

/*-----------------------------------------------------------------------*/

/* used for lane hide or reversal  */
u32 port2_lanes[] = {
#ifdef	FIXUP_REG65_ON_FPGA_BUG
	0x00ffffff,		// X0	0	// 0x00ffffff for should be, 0x0000ffff for debug
#else
	0x0000ffff,		// X0	0
#endif
	0x00eefefe,		// X1	1
	0x00eefefe,		// X2	2
	0x00eefcfc,		// X4	3
	0x00ccf0f0,		// X8	4
	0x00000000,		// X12	5
	0x00000000,		// X16	6
};

u32 port3_lanes[] = {
#ifdef	FIXUP_REG65_ON_FPGA_BUG
	0x00ffffff,		// X0	0	// 0x00ffffff for should be, 0x0000ffff for debug
#else
	0x0000ffff,		// X0	0
#endif
	0x00aaefef,		// X1	1
	0x00aaefef,		// X2	2
	0x00aacfcf,		// X4	3
	0x00220f0f,		// X8	4
	0x00000000,		// X12	5
	0x00000000,		// X16	6
};

u32 port2_reversal_lanes[] = {
	0x00ffffff,		// X0	0
	0x00777f7f,		// X1	1
	0x00777f7f,		// X2	2
	0x00773f3f,		// X4	3
	0x00330f0f,		// X8	4
	0x00000000,		// X12	5
	0x00000000,		// X16	6
};

u32 port3_reversal_lanes[] = {
	0x00ffffff,		// X0	0
	0x0055fefe,		// X1	1
	0x0055fefe,		// X2	2
	0x00aafcfc,		// X4	3
	0x0022f0f0,		// X8	4
	0x00000000,		// X12	5
	0x00000000,		// X16	6
};

#ifdef	CFG_DUAL_SLOT_SUPPORT
u32 port2_dual_reversal_lanes[] = {
	0x00ffffff,		// X0	0
	0x00ddf7f7,		// X1	1
	0x00ddf7f7,		// X2	2
	0x00ddf3f3,		// X4	3
	0x00ccf0f0,		// X8	4
	0x00000000,		// X12	5
	0x00000000,		// X16	6
};

u32 port3_dual_reversal_lanes[] = {
	0x00ffffff,		// X0	0
	0x00557f7f,		// X1	1
	0x00557f7f,		// X2	2
	0x00aa3f3f,		// X4	3
	0x00220f0f,		// X8	4
	0x00000000,		// X12	5
	0x00000000,		// X16	6
};
#endif

/*-----------------------------------------------------------------*/

/*
 * pcie_gfx_txflushtlp_disable
 * 	clear TX_FLUSH_TLP_DIS before power down the GFX lane if bridge is unhiden
 */
static void pcie_gfx_txflushtlp_disable(pcitag_t nb_dev)
{
	u32 port_train, port_hide;

	/* flush TLPs when the DLL is down */
	port_train = (nbmisc_read_index(nb_dev, 0x0C) >> 2) & 0x0C;
	port_hide = nbmisc_read_index(nb_dev, 0x08);
	if(port_train != port_hide){
		if(port_hide & (1 << 2)){
			set_pcie_p_enable_bits(_pci_make_tag(0, 2, 0), 0x20, 1 << 19, 0 << 19);
		}else if(port_hide & (1 << 3)){
			set_pcie_p_enable_bits(_pci_make_tag(0, 3, 0), 0x20, 1 << 19, 0 << 19);	
		}
	}

	return;	
}

/*
 * Release Training for particular port
 * 	To release training for a given port, we need to take the status of
 * 	STATIC_DEV_REMAP into consideration when mode C/D.
 */
static void pcie_release_port_training(int port)
{
	u32 bit_port = 0;
	pcitag_t nb_dev = _pci_make_tag(0, 0, 0);

	switch(port){
		case 2 :
		case 3 :
			bit_port = port + 2;
			break;
		case 4 :
			bit_port = port + 17;
			break;
		case 5 :
			bit_port = port + 17;
			if( (ati_pcie_cfg.Config & CONFIG_STATIC_DEV_REMAP_ENABLE) 
				&& (ati_pcie_cfg.GppConfig == GPP_CONFIG_D) ){
				++bit_port;
			}
		case 6 :
			bit_port = port + 17;
			break;
		case 7 :
			bit_port = port + 17;
			break;
	}
	set_nbmisc_enable_bits(nb_dev, 0x08, 1 << bit_port, 0 << bit_port);

	DEBUG_INFO("liujl : release port status 0x%x\n", nbmisc_read_index(nb_dev, 0x08));

	return;
}

/*
 * pcie_train_port :
 * 	training one port and setting the correct parameters
 * 	NOTE : until now, I do not know how to train with spec. width
 */
static int pcie_train_port(int port, int width)
{
	int res = 1;
	u32 count = 5000;
	u32 lc_state, cur_state, temp;
	u32 reg16, reg32;
	pcitag_t dev = _pci_make_tag(0, port, 0);

	while (count--) {
		delay(40 * 1000);
		delay(200);
		lc_state = nbpcie_p_read_index(dev, 0xa5);	/* lc_state */
		printf("link training : 1st lc_state 0x%x\n", lc_state);
		do{
			temp = lc_state & 0x3F;
			lc_state >>= 8;
			if(temp == 0x3F){
				res = 0;
				/* we should reset the PCIE port : X86 will issue CF9  */
				pcie_gen_reset_dispatch();
				break;
			}
		}while(temp);
		if(res == 0){
			break;
		}

		lc_state = nbpcie_p_read_index(dev, 0xa5);	/* lc_state */
		printf("link training : 2nd lc_state 0x%x\n", lc_state);
		cur_state = lc_state & 0x3f;	/* get LC_CURRENT_STATE, bit0-5 */
		switch (cur_state) {
			case 0x00:	/* 0x00-0x04 means no device is present */
			case 0x01:
			case 0x02:
			case 0x03:
			case 0x04:
				res = 0;
				count = 0;
				break;
			case 0x07:	/* device is in compliance state (training sequence is done). Move to train the next device */
				res = 1;	/* TODO: CIM sets it to 0 */
				count = 0;
				break;
			case 0x10:
				reg16 = _pci_ext_conf_read16(dev, PCIE_VC0_RESOURCE_STATUS);
				printf("link training : reg(0x%x) VC0 resouces 0x%x\n", 0X11a, reg16);
				/* check bit1 : means the link needs to be re-trained*/
				if (reg16 & VC_NEGOTIATION_PENDING) {	
					/* set bit8=1, bit0-2=bit4-6 */
					u32 tmp;
					reg32 = nbpcie_p_read_index(dev, PCIE_LC_LINK_WIDTH_CNTL);
					tmp = reg32;
					tmp = (tmp & 0x000000ff) >> 4;
					reg32 &= 0xffffff00;
					reg32 |= tmp | (1 << 8);
					nbpcie_p_write_index(dev, PCIE_LC_LINK_WIDTH_CNTL, reg32);
					delay(5000);	// delay 5ms
					count++;	/* keep it in loop */
					printf("link training : pcie_p(0x%x) VC NEGO 0x%x\n", PCIE_LC_LINK_WIDTH_CNTL, reg32);
				} else {
					res = 1;
					count = 0;
				}
				break;
			default:	/* reset pcie */
				res = 0;
				count = 0;	/* break loop */
				break;
		}
	}	/* while(count--) */

	return res;
}

/*
 * pcie_gfx_workaround :
 * 	workaround action for specially RV370/RV380 with
 * 	ATI/NV graphic card
 */
static void pcie_gfx_workaround(void)
{
	if(ati_pcie_cfg.Config & CONFIG_GFXCARD_WORKAROUND_ENABLE){
		DEBUG_INFO("GFX WorkAround : we should add GFX WA init here.\n");	
	}

	return;
}	

/*
 * pcie_gfx_init_linkwidth :
 * 	GFX link width setting, change or reconfiguring.
 */
static void pcie_gfx_init_linkwidth(int port, int width)
{
	u32 new_width, cur_width, val;
	pcitag_t gfx_dev = _pci_make_tag(0, port, 0);
	pcitag_t nb_dev = _pci_make_tag(0, 0, 0);

	cur_width = (nbpcie_p_read_index(gfx_dev, 0xA2) >> 4) & 0x07;
	new_width = width;
	if((new_width > cur_width) || (new_width == 0)){	//???
		new_width = cur_width;
	}

	/* get the external GFX device version, if ATI, we should set it as X1 */
	{
		
	}

	/* fixup the X12 problem */
	if(new_width == LINK_WIDTH_X12){
		new_width = LINK_WIDTH_X8;
	}
	
	/* link width setting */
	if(new_width != cur_width){
		/* enable GFX PLL down  */
		set_pcie_gfx_enable_bits(nb_dev, 0x40, 1 << 0, 1 << 0);
		/* link setting */
		val = nbpcie_p_read_index(gfx_dev, 0xA2);
		val &= 0xfffffff0;
		val |= new_width & 0x0f;
		val |= 1 << 8;
		nbpcie_p_write_index(gfx_dev, 0xA2, val);
		/* wait until ok */
		do{
			delay(10);
			val = nbpcie_p_read_index(gfx_dev, 0xA2);
		}while(!(val & (1 << 8)));
		/* check LINK LAYER */
		do{
			delay(10);
			val = _pci_conf_readn(gfx_dev, 0x11A, 2);
		}while(!(val & (1 << 1)));
		/* disable GFX PLL down  */
		set_pcie_gfx_enable_bits(nb_dev, 0x40, 1 << 0, 0 << 0);
	}
	
	return;
}

/*
 * get_gpp_width :
 * 	get the default GPP port width according to 
 * 	GPP configuration
 */
static u32 get_gpp_width(int port)
{
	u32 width = 0;

	switch(ati_pcie_cfg.GppConfig){
		case GPP_CONFIG_A :
			break;
		case GPP_CONFIG_B :
			if(port == 4)
				width = 4;
			break;
		case GPP_CONFIG_C :
			if( (port == 4) || (port == 5) )
				width = 2;
			break;
		case GPP_CONFIG_D :
			if(port == 4){
				width = 2;
			}else if( (port = 5) || (port == 6) ){
				width = 1;
			}
			break;
		case GPP_CONFIG_E :
			width = 1;
			break;
	}	

	return width;
}

/*
 * get_gpp_poweroff_mask
 * 	get GPP4~7 lane mask according to GppConfig, STATIC_DEV_REMAP etc.
 */
static u32 get_gpp_poweroff_mask(int port)
{
	u32 mask;
	u32 reg;
	u32 port_detected;
   
	mask = 0;
	port_detected = ati_pcie_cfg.PortDetect & (1 << port);
	reg = (nbpcie_p_read_index(_pci_make_tag(0, port, 0), 0xa2) >> 4) & 0x07;
	switch(ati_pcie_cfg.GppConfig){
		case GPP_CONFIG_A :
			/* 4 : 0 0 0 0 */
			mask = 0xf0f0;
			break;
		case GPP_CONFIG_B :
			/* 4 : 4 0 0 0 */
			if(port == 4){
				if(port_detected){
					if(reg == LINK_WIDTH_X1){
						mask = 0xe0e0;
					}else if(reg == LINK_WIDTH_X2){
						mask = 0xc0c0;
					}else if(reg == LINK_WIDTH_X4){
						mask = 0x0000;
					}
				}else{
					mask = 0xf0f0;
				}
			}
			break;
		case GPP_CONFIG_C :
			/* 4 : 2 2 0 0 */
			if(reg == LINK_WIDTH_X1){
				if(ati_pcie_cfg.Config & CONFIG_STATIC_DEV_REMAP_ENABLE){
					if(port == 4){
						if(port_detected){
							mask = 0x2020;
						}else{
							mask = 0x3030;
						}
					}else if(port == 5){
						mask = 0x0000;
					}else if(port == 6){
						if(port_detected)
							mask = 0x8080;
						else
							mask = 0xc0c0;
					}
				}else{
					if(port == 4){
						if(port_detected)
							mask = 0x2020;
						else
							mask = 0x3030;
					}else if(port == 5){
						if(port_detected)
							mask = 0x8080;
						else
							mask = 0xc0c0;
					}else if(port == 6){
						mask = 0x0000;
					}					
				}
			}else if(reg == LINK_WIDTH_X2){
				if(port == 4){
					if(port_detected)
						mask = 0x0000;
					else
						mask = 0x3030;
				}
				if( (port == 5) && (!(ati_pcie_cfg.Config & 
							CONFIG_STATIC_DEV_REMAP_ENABLE)) ){
					if(port_detected)
						mask = 0x0000;
					else
						mask = 0xc0c0;				
				}
				if( (port == 6) && (ati_pcie_cfg.Config & 
							CONFIG_STATIC_DEV_REMAP_ENABLE) ){
					if(port_detected)
						mask = 0x0000;
					else
						mask = 0xc0c0;				
				}
			}
			break;
		case GPP_CONFIG_D :
			/* 4 : 2 1 1 0 */
			if(reg == LINK_WIDTH_X1){
				if(port == 4){
					if(port_detected)
						mask = 0x2020;
					else
						mask = 0x3030;
				}
				if(port == 5){
					if(port_detected)
						mask = 0x0000;
					else
						mask = 0x4040;
				}
				if( (port == 6) && (!(ati_pcie_cfg.Config & 
								CONFIG_STATIC_DEV_REMAP_ENABLE)) ){
					if(port_detected)
						mask = 0x0000;
					else
						mask = 0x8080;
				}
				if( (port == 7) && (ati_pcie_cfg.Config & 
								CONFIG_STATIC_DEV_REMAP_ENABLE) ){
					if(port_detected)
						mask = 0x0000;
					else
						mask = 0x8080;
				}
			}else if(reg == LINK_WIDTH_X2){
				if(port == 4){
					if(port_detected)
						mask = 0x0000;
					else
						mask = 0x3030;
				}
			}
			break;
		case GPP_CONFIG_E :
			/* 4 : 1 1 1 1 */
			if( (port == 4) && (!port_detected) )
				mask = 0x1010;
			if( (port == 5) && (!port_detected) )
				mask = 0x2020;
			if( (port == 6) && (!port_detected) )
				mask = 0x4040;
			if( (port == 7) && (!port_detected) )
				mask = 0x8080;
			break;
		default :
			break;
	}

	return mask;
}

/*-----------------------------------------------------------------*/

/*
 * pcie_misc_init :
 * 	program misc NB registers
 */
static void pcie_misc_init(pcitag_t nb_dev)
{

	/* enable BUS0DEV0FUNC0 BAR3 */
	nb_enable_bar3(nb_dev);
	
	/* unhiden CLKCONFIG */
	set_nbcfg_enable_bits(nb_dev, 0x4C, 1 << 0, 1 << 0);

	/* Visible BAR3 and enable access to DEV8 */
	set_nbmisc_enable_bits(nb_dev, 0x00, ~0xfffffff7, (1 << 6));
	
	/* MISCIND SET_POWER message setting */
	set_nbmisc_enable_bits(nb_dev, 0x0B, ~0xffffffff, (1 << 20));
	set_nbmisc_enable_bits(nb_dev, 0x51, ~0xffffffff, (1 << 20));
	set_nbmisc_enable_bits(nb_dev, 0x53, ~0xffffffff, (1 << 20));
	set_nbmisc_enable_bits(nb_dev, 0x55, ~0xffffffff, (1 << 20));
	set_nbmisc_enable_bits(nb_dev, 0x57, ~0xffffffff, (1 << 20));
	set_nbmisc_enable_bits(nb_dev, 0x59, ~0xffffffff, (1 << 20));
	set_nbmisc_enable_bits(nb_dev, 0x5B, ~0xffffffff, (1 << 20));

	/* call the pcie start_init dispatch */
	pcie_init_dispatch();
	
	/* store the NB revision */
	ati_pcie_cfg.NbRevision = get_nb_revision();
	
	return;
}

/*
 * pcie_misc_exit :
 *	exec at the end of PCIE initialization, clear all temporary setting(MMIO etc.)
 */
static void pcie_misc_exit(pcitag_t nb_dev)
{
	pcitag_t clk_dev = _pci_make_tag(0, 1, 0);

	/* hide device8 */
	set_nbmisc_enable_bits(nb_dev, 0x00, 1 << 6, 0 << 6);

	/* save PCIE module revision in CLKCONFIG : SCRATCH 0x84  */
	_pci_conf_write(clk_dev, 0x84, ATI_PCIE_VERSION);

	/* hide CLKCONFIG */
	set_nbcfg_enable_bits(nb_dev, 0x4c, 1 << 0, 0 << 0);

	/* clear reset count : SCRATCH 0x98[31:16] */
	_pci_conf_writen(nb_dev, 0x9A, 0x00, 2);

	/* LPC DMA DeadLock fixup */
	/* Enable Special NP protocol in NB PCIE */
	set_pcie_gppsb_enable_bits(nb_dev, 0x10, 1 << 9, 1 << 9);
	/* Enable Special NP protocol in HTIU */
	set_htiu_enable_bits(nb_dev, 0x06, 1 << 26, 1 << 26);

	/* store the ati_pcie_cfg.Config to PCIE_SCRATCH */
	nbpcie_ind_gppsb_write_index(nb_dev, 0x01, ati_pcie_cfg.Config);

	/* call the pcie_exit dispatch */
	pcie_exit_dispatch();

	/* disable BAR3 */
	nb_disable_bar3(nb_dev);

	return;
}

/*
 * pcie_lane_reversal :
 * 	enable the PCIE lane reversal according to configuration
 */
static void pcie_lane_reversal(pcitag_t nb_dev)
{
	u32 val;

	val = nbmisc_read_index(nb_dev, 0x33);
	val &= ~(3 << 2);
	if(ati_pcie_cfg.Config & CONFIG_LANE_REVERSAL_ENABLE){
		val |= 1 << 2;
	}
#ifdef	CFG_DUAL_SLOT_SUPPORT
	if(ati_pcie_cfg.Config & CONFIG_DUAL_SLOT_SUPPORT_ENABLE){
		val |= 1 << 3;
	}
#endif
	if(ati_pcie_cfg.DbgConfig & DBG_CONFIG_LINK_LC_REVERSAL){
		val |= 1 << 0;
	}
	nbmisc_write_index(nb_dev, 0x33, val);
}

/*
 * pcie_configure_gfx_core :
 * 	configure GFX core
 * 	we should consider the GFX CORE, PHY 
 */
static void pcie_configure_gfx_core(pcitag_t nb_dev)
{
	u32 val;

	/* reset the GFX core and phy properly */
	val = nbmisc_read_index(nb_dev, 0x08);
	val &= 0xfffffff0;
	if(ati_pcie_cfg.Config & CONFIG_DUAL_SLOT_SUPPORT_ENABLE){
		val |= (1 << 2) | (1 << 0);	// we should reset core if DUAL
	}
	val |= 1 << 7;	// reset phy
	nbmisc_write_index(nb_dev, 0x08, val);
	/* delay for reset ok */
	delay(2000);
	val &= ~(1 << 7);
	/* deassert the reset phy */
	nbmisc_write_index(nb_dev, 0x08, val);
	delay(2000);
	// do we need to deassert the CORE AUTOMATIC RESET, or it will do itself???
	return;
}

/*-------------------------------------------------------------------*/

/*
 * pcie_gfx_port_traning :
 * 	make the preparation for port training and 
 * 	training the port for proper configuration
 */
static void pcie_gfx_port_training(int port, int width)
{
	int ret;
	pcitag_t gfx_dev = _pci_make_tag(0, port, 0);
	pcitag_t nb_dev = _pci_make_tag(0, 0, 0);

	/* GFX port training pre init register */
	/* enable shorten enumeration timer and set RCB timeout to be 25ms */
	set_pcie_p_enable_bits(gfx_dev, 0x70, 0x0F << 16, (1 << 16)|(1 << 17)|(1 << 19));

	/* if the LTSSM could not see all 8TS1 during polling active, 
	 * it can still timeout and go back to DetectIdle instead of Hanging
	 */
	set_pcie_p_enable_bits(gfx_dev, 0x02, ~0xffffffff, (1 << 14));

	/* block DMA traffic during C3 state */
	set_pcie_p_enable_bits(gfx_dev, 0x10, (1 << 0), (0 << 0));

	/* BIT31 : do not gate the elec. idle from PHY
	 * BIT30 : enable escape from L1L23
	 */
	set_pcie_p_enable_bits(gfx_dev, 0xA0, ~0xffffffff, (1 << 30)|(1 << 31));

	/* REGS_LC_DONT_GO_TO_L0S_IF_L1_ARMED to prevent lc to go to from L0 to RCV_L0S 
	 * if L1 is armed
	 */
	set_pcie_p_enable_bits(gfx_dev, 0x02, ~0xffffffff, (1 << 6));

	/* port training */
	ret = pcie_train_port(port, width);

	/* if training ok  */
	if(ret){
		/* pcie GFX workaround for RV370 RV380 */
		pcie_gfx_workaround();
		/* change the link width */
		pcie_gfx_init_linkwidth(port, width);
		/* setting the detect bit for GFX */
		ati_pcie_cfg.PortDetect |= 1 << port;
	}

	/* needed for workaround gfx card */
	set_pcie_p_enable_bits(nb_dev, 0x70, 1 << 19, 0 << 19);

	return;
}

/*
 * pcie_poweroff_gfx_lanes :
 *	power off the unused gfx lanes for power saving
 */
static void pcie_poweroff_gfx_lanes(pcitag_t nb_dev)
{
	u32 port_enabled = ati_pcie_cfg.PortEn;
	u32 port_disabled;
	u32 val;
	u32 gfx0_lane_width, gfx1_lane_width;
	u32 reg65;

	/* default to be all off. */
	gfx0_lane_width = gfx1_lane_width = 0;

	/* want to fix some bug */
#ifdef	FIXUP_REG65_ON_FPGA_BUG
	nbpcie_ind_gfx_write_index(nb_dev, 0x64, 0x00FF0000);
#endif

	/* hiden unused GFX ports */
	if(ati_pcie_cfg.Config & CONFIG_HIDE_UNUSED_PORTS_ENABLE){
		port_enabled &= ati_pcie_cfg.PortDetect;
	}
	val = nbmisc_read_index(nb_dev, 0x0C);
	port_disabled = ~port_enabled & (3 << 2);
	val |= port_disabled;
	nbmisc_write_index(nb_dev, 0x0C, val);

	/* prevent ports from training */
	set_nbmisc_enable_bits(nb_dev, 0x08, 3 << 4, port_disabled << 2);

	/* tx flush TLP disable  */
	pcie_gfx_txflushtlp_disable(nb_dev);

	/* setting 0x65 for pad power down properly */
	if( (ati_pcie_cfg.Config & CONFIG_OFF_UNUSED_GFX_LANES_ENABLE) 
		&& (ati_pcie_cfg.Config & CONFIG_HIDE_UNUSED_PORTS_ENABLE)
		&& (!(ati_pcie_cfg.Config & CONFIG_GFX_COMPLIANCE_ENABLE)) ){
		if(ati_pcie_cfg.PortDetect & (1 << 2)){
			/* get the PCIE LINK width for GFX0 */
			gfx0_lane_width = nbpcie_p_read_index(_pci_make_tag(0, 2, 0), 0xA2);
			gfx0_lane_width = (gfx0_lane_width >> 4) & 0x07;
		}

		if(ati_pcie_cfg.PortDetect & (1 << 3)){
			/* get the PCIE LINK width for GFX1 */
			gfx1_lane_width = nbpcie_p_read_index(_pci_make_tag(0, 3, 0), 0xA2);
			gfx1_lane_width = (gfx1_lane_width >> 4) & 0x07;
		}
	ISSUE_DEBUG(PCIE_GFX_GPPSB_CORE_ISSUE, "gppsb(0x%x) gfx(0x%x)\n", nbpcie_ind_gppsb_read_index(nb_dev, 0x10), nbpcie_ind_gfx_read_index(nb_dev, 0x10));
	DEBUG_INFO("gfx0_lane_width : 0x%x, gfx1_lane_width : 0x%x\n", gfx0_lane_width, gfx1_lane_width);
	DEBUG_INFO("gfx0 A2 : 0x%x, gfx1 A2 : 0x%x\n", nbpcie_p_read_index(_pci_make_tag(0, 2, 0), 0xA2),
			nbpcie_p_read_index(_pci_make_tag(0, 2, 0), 0xA2));

		/* setting poweroff some proper pads and PLL */
		if(ati_pcie_cfg.Config & CONFIG_TMDS_ENABLE){
			reg65 = 0x00CCF0F0;
		}else{
			reg65 = 0xffffffff;
			if(ati_pcie_cfg.Config & CONFIG_LANE_REVERSAL_ENABLE){
#ifdef	CFG_DUAL_SLOT_SUPPORT
				if(ati_pcie_cfg.Config & CONFIG_DUAL_SLOT_SUPPORT_ENABLE){
					reg65 &= port2_dual_reversal_lanes[gfx0_lane_width];
					reg65 &= port3_dual_reversal_lanes[gfx1_lane_width];
				}else{
					reg65 &= port2_reversal_lanes[gfx0_lane_width];
					reg65 &= port3_reversal_lanes[gfx1_lane_width];
				}
#else
				reg65 &= port2_reversal_lanes[gfx0_lane_width];
				reg65 &= port3_reversal_lanes[gfx1_lane_width];
#endif
				/* select GFX TX CLK PLL1  */
				set_nbmisc_enable_bits(nb_dev, 0x07, 1 << 16, 1 << 16);
				/* select PLL1 as source clock for TXCLK send and recv */
				set_nbmisc_enable_bits(nb_dev, 0x07, (1 << 8)|(1 << 10)|(1 << 12)|(1 << 14), (1 << 8)|(1 << 10)|(1 << 12)|(1 << 14));
				/* select PLL1 as the source clock for IO_TXCLK0/1_CNTL */
				set_nbmisc_enable_bits(nb_dev, 0x07, (1 << 24)|(1 << 26), (1 << 24)|(1 << 26));
			}else{
				reg65 &= port2_lanes[gfx0_lane_width];
				reg65 &= port3_lanes[gfx1_lane_width];
			} /* CONFIG_LANE_REVERSAL_ENABLE */
		}	/* CONFIG_TMDS_ENABLE */
	ISSUE_DEBUG(PCIE_GFX_GPPSB_CORE_ISSUE, "gppsb(0x%x) gfx(0x%x)\n", nbpcie_ind_gppsb_read_index(nb_dev, 0x10), nbpcie_ind_gfx_read_index(nb_dev, 0x10));

	DEBUG_INFO("just before setting gfx 0x65 reg : 0x%x, reg64 : 0x%x\n", nbpcie_ind_gfx_read_index(nb_dev, 0x65), nbpcie_ind_gfx_read_index(nb_dev, 0x64));
		reg65 |= nbpcie_ind_gfx_read_index(nb_dev, 0x65);
		nbpcie_ind_gfx_write_index(nb_dev, 0x65, reg65);
	DEBUG_INFO("just after setting gfx 0x65 reg with 0x%x, reg64 0x%x\n", nbpcie_ind_gfx_read_index(nb_dev, 0x65), nbpcie_ind_gfx_read_index(nb_dev, 0x64));
	ISSUE_DEBUG(PCIE_GFX_GPPSB_CORE_ISSUE, "gppsb(0x%x) gfx(0x%x)\n", nbpcie_ind_gppsb_read_index(nb_dev, 0x10), nbpcie_ind_gfx_read_index(nb_dev, 0x10));

	}

	return;
}

/*
 * pcie_gfx_init :
 * 	init the External GFX device, train ports, changed the width 
 * 	according to the input parameters
 * 	NOTE : the GFX0 & GFX1 all inited.
 */
static void pcie_gfx_init(pcitag_t nb_dev)
{
	pcitag_t gfx0_dev = _pci_make_tag(0, 2, 0);

#ifdef	CFG_DUAL_SLOT_SUPPORT
	pcitag_t gfx1_dev = _pci_make_tag(0, 3, 0);
	int is_dual_slot;
#endif
	/* bypass the scrambler when training */
	if(ati_pcie_cfg.DbgConfig & DBG_CONFIG_BYPASS_SCRAMBLER){
		set_pcie_p_enable_bits(gfx0_dev, 0xA1, 0x0F << 0, 0x08 << 0);
	}
#ifdef	CFG_DUAL_SLOT_SUPPORT
	/* GFX DUAL SLOT Config */
	if(ati_pcie_cfg.Config & CONFIG_DUAL_SLOT_SUPPORT_ENABLE){
		is_dual_slot = 1;
		if(ati_pcie_cfg.Config & CONFIG_DUALSLOT_AUTODETECT_ENABLE){
			is_dual_slot = pcie_check_dual_cfg_dispatch();
		}
		if(!is_dual_slot){
			ati_pcie_cfg.Config &= ~(CONFIG_DUALSLOT_AUTODETECT_ENABLE 
							| CONFIG_DUAL_SLOT_SUPPORT_ENABLE);
		}else{
			pcie_configure_gfx_core(is_dual_slot);
		}
	}
	
	/* do dispatch for dual slot config */
	if(ati_pcie_cfg.Config & CONFIG_DUAL_SLOT_SUPPORT_ENABLE){
		pcie_port3_reset_dispatch();
		pcie_port3_route_dispatch();
	}else{
		pcie_port2_route_dispatch();
	}
#else	/* CFG_DUAL_SLOT_SUPPORT */
	ati_pcie_cfg.Config &= ~(CONFIG_DUALSLOT_AUTODETECT_ENABLE 
					| CONFIG_DUAL_SLOT_SUPPORT_ENABLE);
#endif	/* CFG_DUAL_SLOT_SUPPORT */

//DEBUG_INFO("jpcie_gfx_init--1\n");
	//ISSUE_DEBUG(PCIE_GFX_GPPSB_CORE_ISSUE, "gppsb(0x%x) gfx(0x%x)\n", nbpcie_ind_gppsb_read_index(nb_dev, 0x10), nbpcie_ind_gfx_read_index(nb_dev, 0x10));

	/* pcie gfx port registers init */
	/* disable SLV ordering logic */
	set_pcie_gfx_enable_bits(nb_dev, 0x20, ~0xffffffff, 1 << 8);
//DEBUG_INFO("jpcie_gfx_init--2\n");
	/* set REGS_DLP_IGNORE_IN_L1_EN to ignore DLLPs during L1 so that the TXCLK can be turned off */
	set_pcie_gfx_enable_bits(nb_dev, 0x02, ~0xffffffff, 1 << 0);
//	ISSUE_DEBUG(PCIE_GFX_GPPSB_CORE_ISSUE, "gppsb(0x%x) gfx(0x%x)\n", nbpcie_ind_gppsb_read_index(nb_dev, 0x10), nbpcie_ind_gfx_read_index(nb_dev, 0x10));
//DEBUG_INFO("JPCie_gfx_init--3\n");
	/* workaround for RV370 & RV380 */
	if(!(ati_pcie_cfg.Config & CONFIG_DUAL_SLOT_SUPPORT_ENABLE)){
		if((ati_pcie_cfg.Config & CONFIG_GFXCARD_WORKAROUND_ENABLE)){
			/* disable RC logical ordering */
			set_pcie_gfx_enable_bits(nb_dev, 0x20, ~0xffffffff, 1 << 9);
		}
	}
//DEBUG_INFO("jpcie_gfx_init--4\n");
	/* pcie gfx lane reversal */
	pcie_lane_reversal(nb_dev);
//DEBUG_INFO("jpcie_gfx_init--5\n");
	//ISSUE_DEBUG(PCIE_GFX_GPPSB_CORE_ISSUE, "gppsb(0x%x) gfx(0x%x)\n", nbpcie_ind_gppsb_read_index(nb_dev, 0x10), nbpcie_ind_gfx_read_index(nb_dev, 0x10));

	/* pcie gfx compliance  */
	if(ati_pcie_cfg.Config & CONFIG_GFX_COMPLIANCE_ENABLE){
		// why just setting GFX0???
		set_nbmisc_enable_bits(nb_dev, 0x32, 1 << 6, 1 << 6);
	}
//DEBUG_INFO("jpcie_gfx_init--7\n");
	/* port 2 training */
	if(ati_pcie_cfg.PortEn & (PORT_ENABLE << PORT(2))){
		/* release holding training */
		pcie_release_port_training(PORT(2));
		if(!(ati_pcie_cfg.Config & CONFIG_GFX_COMPLIANCE_ENABLE)){
			/* start training */
			pcie_gfx_port_training(PORT(2), ati_pcie_cfg.Gfx0Width);
		}
	}
	//ISSUE_DEBUG(PCIE_GFX_GPPSB_CORE_ISSUE, "gppsb(0x%x) gfx(0x%x)\n", nbpcie_ind_gppsb_read_index(nb_dev, 0x10), nbpcie_ind_gfx_read_index(nb_dev, 0x10));
//DEBUG_INFO("jpcie_gfx_init--8\n");
	/* port3 train */
#ifdef	CFG_DUAL_SLOT_SUPPORT
	if(ati_pcie_cfg.Config & CONFIG_DUAL_SLOT_SUPPORT_ENABLE){
		/* for DUAL SLOT mode, GFX1 must be enable */
		ati_pcie_cfg.PortEn |= PORT_ENABLE << PORT(3);
		/* release hold training */
		pcie_release_port_training(PORT(3));
		/* pcie deassert port3 dispatch */
		pcie_dessert_port3_reset_dispatch();
		/* start training */
		if(!(ati_pcie_cfg.Config & CONFIG_GFX_COMPLIANCE)){
			pcie_gfx_port_training(PORT(3), ati_pcie_cfg.Gfx1Width);
		}
	}
#endif

	/* reset RC logical ordering : since RV370 RV380 workaround is done */
set_pcie_gfx_enable_bits(nb_dev, 0x20, 1 << 9, 0 << 9);
	//ISSUE_DEBUG(PCIE_GFX_GPPSB_CORE_ISSUE, "gppsb(0x%x) gfx(0x%x)\n", nbpcie_ind_gppsb_read_index(nb_dev, 0x10), nbpcie_ind_gfx_read_index(nb_dev, 0x10));
//DEBUG_INFO("jpcie_gfx_init--9\n");
	/* power off unused gfx lanes */
	pcie_poweroff_gfx_lanes(nb_dev);
//DEBUG_INFO("jpcie_gfx_init--10\n");
	//ISSUE_DEBUG(PCIE_GFX_GPPSB_CORE_ISSUE, "gppsb(0x%x) gfx(0x%x)\n", nbpcie_ind_gppsb_read_index(nb_dev, 0x10), nbpcie_ind_gfx_read_index(nb_dev, 0x10));
	return;
}

/*-------------------------------------------------------------------*/

/*
 * pcie_tmds_init :
 * 	tmds output init if supported
 */
static void pcie_tmds_init(void)
{
	if(ati_pcie_cfg.Config & CONFIG_TMDS_ENABLE){
		DEBUG_INFO("TMDS : we should add TMDS init here.\n");
	}

	return;
}

/*
 * pcie_cpl_buffer_allocation :
 * 	Allocate the proper CPL buffer dynamically.
 * 	NOTE : Only for GPPSB
 */
static void pcie_cpl_buffer_allocation(pcitag_t nb_dev)
{
	int port = 0;
	pcitag_t dev;

	/* we should make sure the DEV8 is visible here which is done by misc init */

	/* judge the dynamic allocation is enabled or not */
	if(ati_pcie_cfg.DbgConfig & DBG_CONFIG_DYNAMIC_BUF_ALLOC){
		for(port = 4; port <= 8; port++){
			dev = _pci_make_tag(0, port, 0);
			switch(ati_pcie_cfg.GppConfig){
				case GPP_CONFIG_A :
				case GPP_CONFIG_B :
						break;
				case GPP_CONFIG_C :
					if( (port == 4) || (port == 6) || (port == 8) ){
						set_pcie_p_enable_bits(dev, 0x10, 0x1f << 8, 0x10 << 8);
					}
					break;
				case GPP_CONFIG_D :
					if( (port == 4) || (port == 8) ){
						set_pcie_p_enable_bits(dev, 0x10, 0x1f << 8, 0x10 << 8);
					}else if( (port == 6) || (port == 7) ){
						set_pcie_p_enable_bits(dev, 0x10, 0x1f << 8, 0x08 << 8);	
					}
					break;
				case GPP_CONFIG_E :
					if(port == 8){
						set_pcie_p_enable_bits(dev, 0x10, 0x1f << 8, 0x10 << 8);
					}else{
						set_pcie_p_enable_bits(dev, 0x10, 0x1f << 8, 0x08 << 8);
					}
					break;
			} /* switch */
		} /* for */

		/* enable the buf allocation */
		set_pcie_gppsb_enable_bits(nb_dev, 0x20, 1 << 11, 1 << 11);
	} /* if */
	
	return;
}

/*
 * pcie_set_payloadsize :
 * 	set payload size for selected core(GPPSB/GFX)
 * 	type :
 * 		1 : GPPSB
 * 		0 : GFX
 * 	size :
 * 		2 : 16bytes
 * 		3 : 32bytes
 * 		4 : 64bytes
 */
static void pcie_set_payloadsize(int size, int core_type)
{
	pcitag_t nb_dev = _pci_make_tag(0, 0, 0);

	if(core_type){
		set_pcie_gppsb_enable_bits(nb_dev, 0x10, 0x07 << 10, size << 10);
	}else{
		set_pcie_gfx_enable_bits(nb_dev, 0x10, 0x07 << 10, size << 10);
	}

	return;
}

/*
 * pcie_misc_clk_program :
 * 	misc clock program for clock gating
 */
static void pcie_misc_clk_program(void)
{
	pcitag_t nb_dev = _pci_make_tag(0, 0, 0);
	pcitag_t clk_dev = _pci_make_tag(0, 0, 1);

	if(ati_pcie_cfg.Config & CONFIG_GFX_CLK_GATING_ENABLE){
		/* TXCLK clock gating */
		set_nbmisc_enable_bits(nb_dev, 0x07, (1 << 0) | (1 << 1) | (1 << 22),
					(1 << 0) | (1 << 1) | (1 << 22));
		set_pcie_gfx_enable_bits(nb_dev, 0x11, 0xf0 << 0, (1 << 6) | (1 << 7));
		/* LCLK clock gating */
		set_nbcfg_enable_bits(clk_dev, 0x94, 1 << 16, 0 << 16);
	}

	if(ati_pcie_cfg.Config & CONFIG_GPP_CLK_GATING_ENABLE){
		/* TXCLK clock gating */
		set_nbmisc_enable_bits(nb_dev, 0x07, (1 << 4) | (1 << 5) | (1 << 22),
					(1 << 4) | (1 << 5) | (1 << 22));
		set_pcie_gppsb_enable_bits(nb_dev, 0x11, 0xf0 << 0, (1 << 6) | (1 << 7));
		/* LCLK clock gating */
		set_nbcfg_enable_bits(clk_dev, 0x94, 1 << 24, 0 << 24);
	}

	return;
}

/*
 * pcie_share_clkref :
 * 	configure sharing the ref clock for GPP and GFX
 */
static void pcie_refclk_share(pcitag_t nb_dev)
{
#ifdef	CFG_PCIE_REFCLK_SHARE
	set_nbmisc_enable_bits(nb_dev, 0x6A, 1 << 9, 1 << 9);
#endif
	if(ati_pcie_cfg.NbRevision < REV_RS690_A12){
		set_nbmisc_enable_bits(nb_dev, 0x6A, 1 << 9, 1 << 9);
	}

	return;
}

/*--------------------------------------------------------------------*/

/*
 * pcie_configure_gpp_core :
 * 	configure the PCIE GPPSB core
 * 	we should consider the DEV_REMAP, reconfig GppConfig and
 * 	SB link ok
 */
static void pcie_configure_gpp_core(pcitag_t nb_dev)
{
	pcitag_t sb_dev = _pci_make_tag(0, 8, 0);
	u32 val;

	/* setting the static dev remap properly */
	if(ati_pcie_cfg.Config & CONFIG_STATIC_DEV_REMAP_ENABLE){
		set_nbmisc_enable_bits(nb_dev, 0x20, 1 << 1, 0 << 1);
	}else{
		set_nbmisc_enable_bits(nb_dev, 0x20, 1 << 1, 1 << 1);
	}
	
	/* setting the GppConfig */
	if(ati_pcie_cfg.GppConfig == 0){
		ati_pcie_cfg.GppConfig = (nbmisc_read_index(nb_dev, 0x67) >> 4) & 0x0f;
		goto validate_port;
	}

	/* gppsb enable reconfig, link config, auto reset */
	set_nbmisc_enable_bits(nb_dev, 0x37, (1 << 12) | (1 << 15) | (1 << 17), (1 << 12) | (1 << 15) | (1 << 17));
	/* setting SB compliance mode */
	val = nbmisc_read_index(nb_dev, 0x67);
	val &= 0xffffff0f;
	val |= (ati_pcie_cfg.GppConfig  & 0x0000000f) << 4;
	nbmisc_write_index(nb_dev, 0x67, val);
	/* disable reconfig */
	nbmisc_read_index(nb_dev, 0x37);
	val ^= 1 << 14;
	nbmisc_write_index(nb_dev, 0x37, val);
	/* delay 1ms for taking effect */
	delay(1000);

	/* wait for the SB to goto L0 */
	do{
		val = nbpcie_p_read_index(sb_dev, 0xA5);
		val &= 0x3f;	/* check the status */
	}while(0/*val != 0x10*/);

	/* wait for the SB link layer to goto active */
	do{
		val = _pci_ext_conf_read16(sb_dev, 0x11a);
		val &= 1 << 1; 
	}while(0/*val*/);

validate_port :
	ati_pcie_cfg.PortEn &= 0x000000FC;	// PORT2~PORT7 available
	ati_pcie_cfg.PortHp &= 0x000000F0;	// PORT4~PORT7 available

	DEBUG_INFO("configure gpp core misc0x67 0x%x\n", nbmisc_read_index(nb_dev, 0x67));
	return;
}

/*
 * pcie_poweroff_gpp_lanes :
 *  power off unused gpp lanes, if all the ports are poweroff, we should
 *  disable the port too.
 */
static void pcie_poweroff_gpp_lanes(pcitag_t nb_dev)
{
	u32 reg;
	u8 state;
	u32 state_save;
	u32 temp_mask, poweroff_mask = 0;
	int port;
	pcitag_t sb_dev = _pci_make_tag(0, 8, 0);

	/* get the PortEnable, so get the poweroff port */
	state = ati_pcie_cfg.PortEn; 
	if (ati_pcie_cfg.Config & CONFIG_HIDE_UNUSED_PORTS_ENABLE)
		state &= ati_pcie_cfg.PortDetect;
	state = ~state;
	state &= (1 << 4) + (1 << 5) + (1 << 6) + (1 << 7);
	state_save = state << 17;
	state &= ~(ati_pcie_cfg.PortHp);		// power on PortHp ports

	/* hiden it */
	reg = nbmisc_read_index(nb_dev, 0x0c);
	reg |= state;
	nbmisc_write_index(nb_dev, 0x0c, reg);
	/* prevent from training */
	reg = nbmisc_read_index(nb_dev, 0x08);
	reg |= state_save;
	nbmisc_write_index(nb_dev, 0x08, reg);

	DEBUG_INFO("pcie_poweroff_gpp_lanes ---> miscreg0x0c(0x%x) miscreg0x08(0x%x)\n", 
					nbmisc_read_index(nb_dev, 0x0C), nbmisc_read_index(nb_dev, 0x08));

	/* get unused gpp ports mask  */
	if ((ati_pcie_cfg.Config & CONFIG_OFF_UNUSED_GPP_LANES_ENABLE) 
		&& 	(ati_pcie_cfg.Config & CONFIG_HIDE_UNUSED_PORTS_ENABLE)
		&& !(ati_pcie_cfg.Config & CONFIG_GFX_COMPLIANCE_ENABLE)) {
		/* get all gpp used ports mask value */
		for(port = 4; port < 8; port++){
			temp_mask = get_gpp_poweroff_mask(port);
			poweroff_mask |= temp_mask;
		}
	}

	/* get unused sb ports mask */
	reg = (nbpcie_p_read_index(sb_dev, 0xa2) >> 4) & 0x07;
	switch(reg){
		case LINK_WIDTH_X1 :
			poweroff_mask |= 0x0e0e;
			break;
		case LINK_WIDTH_X2 :
			poweroff_mask |= 0x0c0c;
		default :
			poweroff_mask |= 0x0000;
	}
	
	/* poweroff the mask */
	DEBUG_INFO("gppsb poweroff lanes : GppConfig(0x%x), STATIC_DEV_REMAP_ENABLE(0x%x), poweroff_mask 0x%x, sb width 0x%x\n", ati_pcie_cfg.GppConfig, ati_pcie_cfg.Config & CONFIG_STATIC_DEV_REMAP_ENABLE, poweroff_mask, reg);

	reg = nbpcie_ind_gppsb_read_index(nb_dev, 0x65) | poweroff_mask;
	nbpcie_ind_gppsb_write_index(nb_dev, 0x65, reg);
//	DEBUG_INFO("gppsb poweroff reg0x65 = 0x%x\n", nbpcie_ind_gppsb_read_index(nb_dev, 0x65));

	return;
}

/* 
 * pcie_gppsb_init :
 * 	init the GPP and SB ports and training them to be proper configuration
 */
static void pcie_gppsb_init(pcitag_t nb_dev)
{
	pcitag_t dev8, gpp_dev;
	int port;
	u32 width;
	u32 val;

	DEBUG_INFO("pcie_gppsb_init() ---> GPP core init.\n");
	/* GPP core init */
	/* disable SLV ordering Logic */
	set_pcie_gppsb_enable_bits(nb_dev, 0x20, 1 << 8, 1 << 8);
	/* set REGS_DLP_IGNORE_IN_L1_EN to ignore DLLPs during L1 so that txclk can be turned off */
	set_pcie_gppsb_enable_bits(nb_dev, 0x02, 1 << 0, 1 << 0);
	
	DEBUG_INFO("pcie_gppsb_init() ---> SB dev8 init.\n");
	/* SB dev8 init */
	dev8 = _pci_make_tag(0, 8, 0);
	/* set RCB timeout to be 100ms and shorten the enumeration time */
	set_pcie_p_enable_bits(dev8, 0x70, 0x07 << 16, (1 << 16) | (1 << 18) | (1 << 19));
	/* forbiden hang */
	set_pcie_p_enable_bits(dev8, 0x02, 1 << 14, (1 << 14));
	/* enable the escape from L1L23 and do not gate elec. idle from the PHY */
	set_pcie_p_enable_bits(dev8, 0xA0, (1 << 30) | (1 << 31), (1 << 30) | (1 << 31));
	/* prevent LC to goto from L0 to RCV_L0s if L1 is armed */
	set_pcie_p_enable_bits(dev8, 0x02, 1 << 6, (1 << 6));

	DEBUG_INFO("pcie_gppsb_init() ---> compliance GPP.\n");
	/* compliance setting : GPP */
	if(ati_pcie_cfg.Config & CONFIG_GPP_COMPLIANCE_ENABLE){
		set_nbmisc_enable_bits(nb_dev, 0x67, 1 << 3, 1 << 3);
	}
	
	DEBUG_INFO("pcie_gppsb_init() ---> compliance SB.\n");
	/* compliance setting : SB */
	if(ati_pcie_cfg.DbgConfig & DBG_CONFIG_SB_COMPLIANCE){
		/* gppsb enable reconfig */
		set_nbmisc_enable_bits(nb_dev, 0x37, (1 << 12), (1 << 12));
		/* gppsb enable link config */
		set_nbmisc_enable_bits(nb_dev, 0x37, (1 << 15), (1 << 15));
		/* gppsb enable auto reset */
		set_nbmisc_enable_bits(nb_dev, 0x37, (1 << 17), (1 << 17));
		/* setting SB compliance mode */
		set_nbmisc_enable_bits(nb_dev, 0x67, (1 << 2), (1 << 2));
		/* disable reconfig */
		nbmisc_read_index(nb_dev, 0x37);
		val ^= 1 << 14;
		nbmisc_write_index(nb_dev, 0x37, val);
		/* delay 1ms for taking effect */
		delay(1000);
	}


	DEBUG_INFO("pcie_gppsb_init() ---> training 4~7 ports.\n");
	/* config and training for every GPPSB ports */
	for(port = 4; port < 8; port++){
		gpp_dev = _pci_make_tag(0, port, 0);
		width = get_gpp_width(port);
		DEBUG_INFO("gppconfig(0x%x) portenabel(0x%x) port(%d) width(%d)\n", ati_pcie_cfg.GppConfig, ati_pcie_cfg.PortEn, port, width);
		/* BLock DMA traffic during C3 state */
		set_pcie_p_enable_bits(gpp_dev, 0x10, 1 << 0, 0 << 0);
		/* enable TLP flushing */
		set_pcie_p_enable_bits(gpp_dev, 0x20, 1 << 19, 0 << 19);
		/* if port enabled */
		if(ati_pcie_cfg.PortEn & (1 << port)){
			/* release hold port training */
			pcie_release_port_training(port);
			/* if not compliance, do the training */
			if(!(ati_pcie_cfg.Config & CONFIG_GPP_COMPLIANCE_ENABLE)){
				if(pcie_train_port(port, width) > 0){
					ati_pcie_cfg.PortDetect |= 1 << port;
				}
			}
		} /* PortEn */
	} /* port from 4 to 7 */

	DEBUG_INFO("pcie_gppsb_init() ---> poweroff gpp lanes with PortDetect 0x%x.\n", ati_pcie_cfg.PortDetect);
	/* shutdown ununsed gpp ports, NOTE : every ports for sb should not be shutdown */
	pcie_poweroff_gpp_lanes(nb_dev);
	DEBUG_INFO("pcie_gppsb_init() ---> end.\n");
	return;
}

/*--------------------------------------------------------------------*/

/*
 * pcie_pre_init :
 * 	pcie pre-system init
 */
void pcie_pre_init(void)
{
	pcitag_t nb_dev = _pci_make_tag(0, 0, 0);

	DEBUG_INFO("\n+++++++++++++++++++++++PCIE PRE STAGE WITH REV(%d)++++++++++++++++++++++++++++++++++\n", get_nb_revision());

	DEBUG_INFO("pcie_pre_init() ---> pcie_misc_init()\n");
	/* pcie misc init : NbRevision is gotten */
	pcie_misc_init(nb_dev);

	DEBUG_INFO("pcie_pre_init() ---> pcie_refclk_share()\n");
	/* share clock setting */
	pcie_refclk_share(nb_dev);

	DEBUG_INFO("pcie_pre_init() ---> pcie_configure_gfx_core()\n");
	/* configure the GFX core one time */
	pcie_configure_gfx_core(nb_dev);

	DEBUG_INFO("pcie_pre_init() ---> pcie_configure_gpp_core()\n");
	/* configure the GPP core */
	pcie_configure_gpp_core(nb_dev);

	/* maybe we should reset all the ports here */

	DEBUG_INFO("pcie_pre_init() ---> pcie_tmds_init()\n");
	/* tmds init */
	pcie_tmds_init();

	DEBUG_INFO("pcie_pre_init() ---> pcie_gfx_init()\n");
	/* init GFX port */
	pcie_gfx_init(nb_dev);

	DEBUG_INFO("pcie_pre_init() ---> pcie_cpl_buffer_allocation()\n");
	/* CPL buffer allocation */
	pcie_cpl_buffer_allocation(nb_dev);

	DEBUG_INFO("pcie_pre_init() ---> pcie_gppsb_init()\n");
	/* init GPPSB port */
	pcie_gppsb_init(nb_dev);

	DEBUG_INFO("pcie_pre_init() ---> pcie_set_payloadsize()\n");
	/* setting the payload size */
	pcie_set_payloadsize(ati_pcie_cfg.GfxPayload, 0);
	pcie_set_payloadsize(ati_pcie_cfg.GppPayload, 1);

	DEBUG_INFO("pcie_pre_init() ---> pcie_misc_clk_program()\n");
	/* misc clk program */
	pcie_misc_clk_program();

	DEBUG_INFO("pcie_pre_init() ---> pcie_misc_exit()\n");
	/* misc exit */
	pcie_misc_exit(nb_dev);
	DEBUG_INFO("------------------------------------------PCIE PRE STAGE DONE------------------------------------\n");

	return;
}
