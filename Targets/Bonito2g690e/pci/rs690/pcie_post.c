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
 * PCIE_POST :
 * 	PCIE init routine after PCI/PCIE emulation
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
/* used for AXCFG config */
#include "../sb600/sb600_config.h"
#include <pci/sb600_cfg.h>
#include <include/sb600.h>

/*----------------------------------------------------------------------*/

static u16 get_pcie_capability(pcitag_t dev, u8 cap)
{
	u16 cap_pointer = 0;
	u16 val;
	u16 next;

	next = 0x34;
	val = _pci_conf_readn(dev, next, 1);
	if(val){
		next = val;
	}else{
		return 0;
	}
	
	do{
		val = _pci_conf_readn(dev, next, 2);
		if( (val & 0x00ff) == cap ){
			cap_pointer = next;
			break;
		}else{
			if(val & 0xff00){
				next = (val & 0xff00) >> 8;
			}else{
				cap_pointer = 0;
				break;
			}
		}
	}while(!cap_pointer);

	return cap_pointer;
}

/*--------------------------------------------------------------------------*/

/*
 * pcie_enable_aspm_for_device :
 * 	Enable Active Support Power Management for device 
 * 	according to configuration and capability
 */
static u8 pcie_enable_aspm_for_device(pcitag_t dev, u8 aspm_mask)
{
	u8 rev = get_nb_revision();
	u8 func_num = 0;
	u16 cap_pointer = 0;
	u32 aspm_cap;
	int i;
	
	/* workaround for Atheros XB6X device */
	if( (rev < REV_RS690_A12) && (_pci_conf_readn(dev, 0x00, 2) == 0x168C) ){
		aspm_mask &= 0xFE;
	}
	
	func_num = _pci_conf_readn(dev, 0x0E, 1);
	if(func_num & 0x80){
		func_num = 7;
	}else{
		func_num = 1;
	}
	for(i = 0; i < func_num; i++){
		dev = dev | (i << 8);
		if(_pci_conf_read(dev, 0x00) != 0xffffffff){
			cap_pointer = get_pcie_capability(dev, PCIE_CAP_ID);
			if(cap_pointer != 0){
				aspm_cap = _pci_conf_read(dev, cap_pointer + 0x0C);
				set_nbcfg_enable_bits(dev, cap_pointer + 0x10, ((1 << 0) | (1 << 1)), ((0 << 0) | (0 << 1)));
				/* L0s enable is the base condition */
				if(aspm_cap & (1 << 10)){
					if(aspm_mask & (1 << 0)){
						set_nbcfg_enable_bits(dev, cap_pointer + 0x10, 1 << 0, 1 << 0);
					}
					if( (aspm_mask & (1 << 1)) && (aspm_cap & (1 << 11)) ){
						set_nbcfg_enable_bits(dev, cap_pointer + 0x10, 1 << 1, 1 << 1);
						/* workaround for Athores XB6X device */
						if( (rev < REV_RS690_A12) && (_pci_conf_readn(dev, 0x00, 2) == 0x168C) ){
							_pci_ext_conf_write(dev, 0x70C, 0x0F003F01);
						}
					}else{
						aspm_mask &= ~(1 << 1);
					}
				}else{
					aspm_mask &= ~((1 << 0) | (1 << 1));
				}
			}
		} // if device ID is ok
	} // for...

	DEBUG_INFO("pcie_enable_aspm_for_device cap+10reg : lastdev0x%x, cap0x%x, val0x%x, aspm_mask0x%x\n", dev, cap_pointer, _pci_conf_read(dev, cap_pointer + 0x10), aspm_mask);

	return aspm_mask;
}

/*
 * pcie_aspm_enable :
 * enable one ASPM for the port
 */
static void pcie_aspm_enable(pcitag_t root_dev, int aspm_mask)
{
	pcitag_t dev;
	u8 sec_bus = 0;
	u8 ret = 0;
	u32 val;
	
	if(_pci_conf_read(root_dev, 0x00) != 0xffffffff){
		sec_bus = _pci_conf_readn(dev, 0x19, 1);
		dev = _pci_make_tag(sec_bus, 0, 0);
		if(_pci_conf_read(dev, 0x00) != 0xffffffff){
			/* enable ASPM in device */
			ret = pcie_enable_aspm_for_device(dev, aspm_mask);
			/* enable ASPM for PCIE port */
			if(ret){
				/* L1/L0 latency reduction  */
				val = nbpcie_p_read_index(root_dev, 0xA0);
				val &= 0xffff000f;
				val |= (((CFG_POST_L1_INACTIVITY_TIME << 4) 
						| CFG_POST_L0S_INACTIVITY_TIME) << 8) | (1 << 4) | (1 << 5);
				nbpcie_p_write_index(root_dev, 0xA0, val);
				/* enable ASPM in port */
				pcie_enable_aspm_for_device(root_dev, aspm_mask);
			}
		}
	}

	return;
}

/*
 * gfx_aspm_enable :
 * gfx ports aspm enable
 */
static void gfx_aspm_enable(pcitag_t gfx_dev, u8 aspm_mask)
{	
	pcitag_t nb_dev = _pci_make_tag(0, 0, 0);

#ifdef	CFG_POST_GFX_ASPM_SUPPORT
	pcie_aspm_enable(gfx_dev, aspm_mask);

	/* set the idle detector threshold to +dV in the PHY:PCIE_B_P90_CNTL[14:13]=10 */
	set_pcie_gfx_enable_bits(nb_dev, 0xF9, (1 << 13) | (1 << 14), 1 << 14);
#endif

	return;
}

/*
 * gpp_aspm_enable :
 * gpp ports aspm enable
 */
static void gpp_aspm_enable(pcitag_t gpp_dev, u8 aspm_mask)
{	
	pcitag_t nb_dev = _pci_make_tag(0, 0, 0);

#ifdef	CFG_POST_GPP_ASPM_SUPPORT
	pcie_aspm_enable(gpp_dev, aspm_mask);

	/* set the idle detector threshold to +dV in the PHY:PCIE_B_P90_CNTL[14:13]=10 */
	set_pcie_gppsb_enable_bits(nb_dev, 0xF9, (1 << 13) | (1 << 14), 1 << 14);
#endif

	return;
}

/*
 * nbsb_aspm_enable :
 *
 * */
static void nbsb_aspm_enable(pcitag_t sb_dev, u8 aspm_mask)
{	
	pcitag_t nb_dev = _pci_make_tag(0, 0, 0);
	u32 val;

#ifdef	CFG_POST_NBSB_ASPM_SUPPORT
	// enable sb visiable
	set_nbmisc_enable_bits(nb_dev, 0x00, 1 << 6, 1 << 6);

	/* Bridge L1/L0 latency reduction  */
	val = nbpcie_p_read_index(sb_dev, 0xA0);
	val &= 0xffff000f;
	val |= (0x39 << 8) | (1 << 4) | (1 << 5);
	nbpcie_p_write_index(sb_dev, 0xA0, val);

	/* enable ASPM for sb */
	pcie_enable_aspm_for_device(sb_dev, aspm_mask);

	/* ABlink relative setting */
	axindxp_reg(0xA0, 0xff << 8, 0x69 << 8);

	/* setting bridge PM control */
	axcfg_reg(0x68, 3 << 0, ASPM_MASK << 0);

	/* set the idle detector threshold to +dV in the PHY:PCIE_B_P90_CNTL[14:13]=10 */
	set_pcie_gppsb_enable_bits(nb_dev, 0xF9, (1 << 13) | (1 << 14), 1 << 14);

	// hiden sb dev
	set_nbmisc_enable_bits(nb_dev, 0x00, 1 << 6, 0 << 6);
#endif

	return;
}

/*--------------------------------------------------------------------*/

/*
 * pcie_set_slot_capability :
 * Set slot power scale, hotplug etc.
 */
static void pcie_set_slot_capability(pcitag_t port_dev)
{
	int bus, port, func;
	u8 value;
	u32	reg32;

	/* get port number from device */
	_pci_break_tag(port_dev, &bus, &port, &func);

	switch(port){
		case 2 :
			if(ati_pcie_cfg.Gfx0Pwr){
				value = ati_pcie_cfg.Gfx0Pwr;
			}else{
				value = 0x4B;	// default to be 75W
			}
			break;
		case 3 :
			if(ati_pcie_cfg.Gfx1Pwr){
				value = ati_pcie_cfg.Gfx1Pwr;
			}else{
				value = 0x4B;		// default to be 75W
			}
			break;
		case 4 :
		case 5 :
		case 6 :
		case 7 :
			if(ati_pcie_cfg.GppPwr){
				value = ati_pcie_cfg.GppPwr;
			}else{
				value = 0x19;		// default to be 25W
			}
			break;
		default :
			break;
	}
	
	/* setting slot param : slot power limit value, HP cap, slot number and
	 * use default slot power limit scale 1.0X
	 */
	reg32 = _pci_conf_read(port_dev, 0x6C);
	reg32 &= 0x0007807f;
	reg32 |= value << 7;	// slot power limit value
	reg32 |= port << 19;	// slot number
	if( (1 << port) & ati_pcie_cfg.PortHp ){	// if HP configed ok
		reg32 |= (1 << 5) | (1 << 6);
	}
	_pci_conf_write(port_dev, 0x6C, reg32);

	/* make sure the slot implement bit is setted */
	set_nbcfg_enable_bits(port_dev, 0x58, 1 << 24, 1 << 24);

	DEBUG_INFO("set_slot_cap : tag(0x%x) slot6c(0x%x), slot58(0x%x)\n", port_dev, _pci_conf_read(port_dev, 0x6C), _pci_conf_read(port_dev, 0x58));

	return;
}

static void pcie_enable_common_clock(pcitag_t port_dev, pcitag_t device_dev)
{
	int i, func_num = 0;
	u8 pcie_pointer;
	int flag = 0;
	int val = 1;

	/* judge the multi-function or not */
	if(_pci_conf_readn(device_dev, 0x0E, 1) & (1 << 7)){
		func_num = 7;
	}else{
		func_num = 1;
	}

	/* if no device in the port */
	if(_pci_conf_read(device_dev, 0x00) == 0xffffffff){
		return;
	}

	for(i = 0; i < func_num; i++){
		pcie_pointer = get_pcie_capability(device_dev, PCIE_CAP_ID);
		if(pcie_pointer == 0){
			continue;
		}
		/* enable common clock for device-func */
		set_nbcfg_enable_bits(device_dev, pcie_pointer + 0x10, 1 << 6, 1 << 6);
		flag++;
	}
	
	if(flag){
		/* enable common clock for port */
		set_nbcfg_enable_bits(port_dev, 0x68, 1 << 6, 1 << 6);
		/* retrain the link */
		set_nbcfg_enable_bits(port_dev, 0x68, 1 << 5, 1 << 5);
		/* waiting the training flag to be ok */
		while(val){
			delay(800);	// delay 800us for polling
			val = _pci_conf_read(port_dev, 0x68) & (1 << 27);
		}
	}

	DEBUG_INFO("pcie_enable_common_clock---> pcie_pointer(0x%x) device68(0x%x) port68(0x%x)\n", pcie_pointer, _pci_conf_read(device_dev, pcie_pointer + 0x10), _pci_conf_read(port_dev, 0x68));
	return;
}

/*
 * pcie_init_pcie_port :
 * init all pcie port common clock and slot capability feature.
 */
static void pcie_pcie_ports_init(void)
{
	int i;
	u8 sec_bus;
	pcitag_t port_dev, device_dev;

	for(i = 2; i < 8; i++){
		port_dev = _pci_make_tag(0, i, 0);
		if(_pci_conf_read(port_dev, 0x00) == 0xffffffff){
			continue;
		}else{
			sec_bus = _pci_conf_readn(port_dev, 0x19, 1);
			device_dev = _pci_make_tag(sec_bus, 0, 0);
			DEBUG_INFO("pcie_init_pcie_port() ---> port_dev(0x%x), sec_bus(0x%x), device(0x%x)\n", port_dev, sec_bus, device_dev);
			if(_pci_conf_read(device_dev, 0x00) != 0xffffffff){
				pcie_enable_common_clock(port_dev, device_dev);
			}
			pcie_set_slot_capability(port_dev);
		}
	}

	return;
}

/*
 * nbsb_enable_vc :
 * enable NB-SB Virtual Channel
 */
static void pcie_nbsb_enable_vc(void)
{
	pcitag_t nb_dev = _pci_make_tag(0, 0, 0);
	pcitag_t sb_dev = _pci_make_tag(0, 8, 0);
	u32 val;

	/* make dev8 visible */
	set_nbmisc_enable_bits(nb_dev, 0x00, 1 << 6, 1 << 6);
	
	/* map tc 7:1 to VC1 */
	set_nbcfg_enable_bits(sb_dev, 0x114, 0x7F << 1, 0x0 << 1);
	set_nbcfg_enable_bits(sb_dev, 0x120, 0x7F << 1, 0x7F << 1);
	set_nbcfg_enable_bits(sb_dev, 0x120, 0x07 << 24, 0x01 << 24);
	set_nbcfg_enable_bits(sb_dev, 0x120, 1 << 31, 1 << 31);

	/* set Alink ABCFG */
	axcfg_reg(0x114, 0x7F << 1, 0x0 << 1);
	axcfg_reg(0x120, 0x7F << 1, 0x7F << 1);
	axcfg_reg(0x120, 0x07 << 24, 0x01 << 24);
	axcfg_reg(0x120, 1 << 31, 1 << 31);

	/* waiting for link ok */
	do{
		val = nbpcie_p_read_index(sb_dev, 0x124);
		val &= 0x02;
	}while(val);

	/* enable alt dma */
	abcfg_reg(0x50, 1 << 3, 1 << 3);

	/* hiden dev8 */
	set_nbmisc_enable_bits(nb_dev, 0x00, 1 << 6, 0 << 6);

	return;
}

/*--------------------------------------------------------------------*/

void pcie_post_init(void)
{
	pcitag_t nb_dev = _pci_make_tag(0, 0, 0);
	u8 aspm_mask;
	u32 val;

	DEBUG_INFO("\n++++++++++++++++++++PCIE POST STAGE : start with type(0x%x), revision(0x%x)++++++++++++++++++++++++++\n", get_sb600_platform(), get_sb600_revision());

	/* setting pcie port common clock and others */
#ifdef	CFG_POST_PCIE_INIT_PORTS
	if(ati_pcie_cfg.PortSlotInit)
		pcie_pcie_ports_init();
#endif

	/* setting aspm for NBSB */
#ifdef	CFG_POST_NBSB_ASPM_SUPPORT
	aspm_mask = ati_pcie_cfg.NBSBLx;
	aspm_mask &= 0xFE;	// remove the L0S because SB600 do not support it.
	if(aspm_mask){
		nbsb_aspm_enable(_pci_make_tag(0, 8, 0), aspm_mask);
	}
#endif

	/* setting aspm for GFX */
#ifdef	CFG_POST_GFX_ASPM_SUPPORT
	aspm_mask = ati_pcie_cfg.GfxLx;
	if(aspm_mask){
		gfx_aspm_enable(_pci_make_tag(0, 2, 0), aspm_mask);
		gfx_aspm_enable(_pci_make_tag(0, 3, 0), aspm_mask);
	}
#endif

	/* setting aspm for GPP */
#ifdef	CFG_POST_GPP_ASPM_SUPPORT
	aspm_mask = ati_pcie_cfg.GppLx;
	if(aspm_mask){
		int port;
		for(port = 4; port < 8; port++){
			gpp_aspm_enable(_pci_make_tag(0, port, 0), aspm_mask);
		}
	}
#endif

	/* enable VC if needed */
	if(ati_pcie_cfg.DbgConfig & DBG_CONFIG_NB_SB_VC){
		pcie_nbsb_enable_vc();
	}

	/* pcieind : gfx and gpp HW init lock */
	set_pcie_gfx_enable_bits(nb_dev, 0x10, 1 << 0, 1 << 0);
	set_pcie_gppsb_enable_bits(nb_dev, 0x10, 1 << 0, 1 << 0);
	/* check if TMDS enabled : we should set it before in this scratch */
	val = nbpcie_ind_gppsb_read_index(nb_dev, 0x01);
	DEBUG_INFO("pcie_post_init() ---> TMDS scratch checking 0x%x\n", val);
	if(!(val & ( 1 << 10))){
		u32 temp;
		/* turn off GFX TX pad if needed */
		temp = nbpcie_ind_gfx_read_index(nb_dev, 0x65);
		if(temp == 0x00ffffff){
			set_nbmisc_enable_bits(nb_dev, 0x07, 1 << 3, 1 << 3);
		}
	}
	DEBUG_INFO("------------------------------------------PCIE POST STAGE DONE---------------------------------------\n");

	return;
}

/*---------------------------------------------------------------------*/


