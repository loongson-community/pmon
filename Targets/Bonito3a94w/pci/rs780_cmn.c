/*
 * This file is part of the coreboot project.
 *
 * Copyright (C) 2008 Advanced Micro Devices, Inc.
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
 */
#include <time.h>
#include "rs780.h"
#include "rs780_cmn.h"

#define HT_CONF_TYPE0_ADDR 0x90000efdfe000000
//#define HT_MAP_TYPE0_CONF_ADDR  0xbfe80000
#define HT_MAP_TYPE0_CONF_ADDR  BONITO_PCICFG0_BASE_VA


//typedef unsigned long long u64;
u32 pci_read_type0_config32(u32 dev, u32 func, u32 reg){
    //u64 addr = 0x90000efdfe000000;
    u32 addr = HT_MAP_TYPE0_CONF_ADDR;
 
    //printk("addr=%x,dev=%x, fn=%x, reg=%x\n", addr, dev, func, reg);
    
    addr |= (dev << 11 | func << 8 | reg);
    
    return *((u32 *) addr);
}

void pci_write_type0_config32(u32 dev, u32 func, u32 reg, u32 val){
    //u64 addr = 0x90000efdfe000000;
    u32 addr = HT_MAP_TYPE0_CONF_ADDR;
    
    //printk("addr=%x,dev=%x, fn=%x, reg=%x\n", addr, dev, func, reg);
    
    addr |= (dev << 11 | func << 8 | reg);
    
    *((u32 *) addr) = val; 
   
}


#define HT_CONF_TYPE1_ADDR 0x90000efdff000000
//#define HT_MAP_TYPE1_CONF_ADDR  0xbe000000
#define HT_MAP_TYPE1_CONF_ADDR  BONITO_PCICFG1_BASE_VA

u32 pci_read_type1_config32(u32 bus, u32 dev, u32 func, u32 reg){
    //u64 addr = 0x90000efdff000000;
    u32 addr = HT_MAP_TYPE1_CONF_ADDR;
    
    //printk("addr=%x,bus=%x,dev=%x, fn=%x, reg=%x\n", addr, bus, dev, func, reg);
    
    addr |= (bus << 16 | dev << 11 | func << 8 | reg);
    
    return *((u32 *) addr);
}

void pci_write_type1_config32(u32 bus, u32 dev, u32 func, u32 reg, u32 val){
    //u64 addr = 0x90000efdff000000;
    u32 addr = HT_MAP_TYPE1_CONF_ADDR;

    
    //printk("addr=%x,bus=%x,dev=%x, fn=%x, reg=%x\n", addr, bus, dev, func, reg);
    
    addr |= (bus << 16 | dev << 11 | func << 8 | reg);
    
    *((u32 *) addr) = val;
}

u32 _pci_conf_read(device_t tag,int reg)
{
	return _pci_conf_readn(tag,reg,4);
}


u32 _pci_conf_read32(device_t tag,int reg)
{
	return _pci_conf_readn(tag,reg,4);
}


u8 _pci_conf_read8(device_t tag,int reg)
{
	u32 data;
	u32 offset;
	u32 new_reg;
	
	new_reg = reg & 0xfc;
	data = _pci_conf_readn(tag,new_reg,4);
	offset = reg & 3;
	data = data >> (offset * 8);
	data &= 0xff;
	
	return (u8)data;
}

u16 _pci_conf_read16(device_t tag,int reg)
{
	u32 data;
	u32 offset;
	u32 new_reg;
	
	new_reg = reg & 0xfc;
	data = _pci_conf_readn(tag,new_reg,4);
	offset = reg & 2;
	data = data >> (offset << 3);
	data &= 0xffff;
	
	return (u16)data;
}



u32 _pci_conf_readn(device_t tag, int reg, int width)
{
    int bus, device, function;

    if ((width != 4) || (reg & 3) || reg < 0 || reg >= 0x100) {
    	printk("_pci_conf_readn: bad reg 0x%x, tag 0x%x, width 0x%x\n", reg, tag, width);
    	return ~0;
    }

    _pci_break_tag (tag, &bus, &device, &function); 
    if (bus == 0) {
    	/* Type 0 configuration on onboard PCI bus */
    	if (device > 31 || function > 7)
    	{	
		printk("_pci_conf_readn: bad device 0x%x, function 0x%x\n", device, function);
    	    	return ~0;		/* device out of range */
	}
    	return pci_read_type0_config32(device, function, reg);
    }
    else {
    	/* Type 1 configuration on offboard PCI bus */
    	if (bus > 255 || device > 31 || function > 7)
    	{	
		printk("_pci_conf_readn: bad bus 0x%x, device 0x%x, function 0x%x\n", bus, device, function);
    	    	return ~0;		/* device out of range */
	}
    	return pci_read_type1_config32(bus, device, function, reg);
    }

}
void
_pci_conf_write(device_t tag, int reg, u32 data)
{
	return _pci_conf_writen(tag,reg,data,4);
}

void
_pci_conf_write32(device_t tag, int reg, u32 data)
{
	return _pci_conf_writen(tag,reg,data,4);
}

void
_pci_conf_write8(device_t tag, int reg, u8 data)
{
	return _pci_conf_writen(tag,reg,data,1);
}

void
_pci_conf_write16(device_t tag, int reg, u16 data)
{
	return _pci_conf_writen(tag,reg,data,2);
}



void _pci_conf_writen(device_t tag, int reg, u32 data,int width)
{
    int bus, device, function;
    u32 ori;
    u32 mask = 0x0;

    if ((reg & (width -1)) || reg < 0 || reg >= 0x100) {
    	printk("_pci_conf_writen: bad reg 0x%x, tag 0x%x, width 0x%x\n", reg, tag, width);
    	return;
    }

    _pci_break_tag (tag, &bus, &device, &function);

    if (bus == 0) {
    	/* Type 0 configuration on onboard PCI bus */
    	if (device > 31 || function > 7)
    	{	
		printk("_pci_conf_writen: bad device 0x%x, function 0x%x\n", device, function);
    	    	return;		/* device out of range */
	}
    }
    else {
    	/* Type 1 configuration on offboard PCI bus */
    	if (bus > 255 || device > 31 || function > 7)
    	{	
		printk("_pci_conf_writen: bad bus 0x%x, device 0x%x, function 0x%x\n", bus, device, function);
    	    	return;		/* device out of range */
	}
    }
    
    ori = _pci_conf_read(tag, reg & 0xfc);
    if(width == 2){
        if(reg & 2){
            mask = 0xffff;
        }
        else{
            mask = 0xffff0000;
        }
    }
    else if(width == 1){
        if ((reg & 3) == 1) {
    	  mask = 0xffff00ff;
    	}else if ((reg & 3) == 2) {
    	  mask = 0xff00ffff;
    	}else if ((reg & 3) == 3) {
    	  mask = 0x00ffffff;
    	}else{
    	  mask = 0xffffff00;
    	}
    }
    
    data = data << ((reg & 3) * 8);
    data = (ori & mask) | data;
    
    
    if (bus == 0) {
    	return pci_write_type0_config32(device, function, reg & 0xfc, data);
    }
    else {
    	return pci_write_type1_config32(bus, device, function, reg & 0xfc, data);
    }
}

static u32 nb_read_index(device_t tag, u32 index_reg, u32 index)
{
	pci_write_config32(tag, index_reg, index);
	return pci_read_config32(tag, index_reg + 0x4);
}

static void nb_write_index(device_t tag, u32 index_reg, u32 index, u32 data)
{

	pci_write_config32(tag, index_reg, index);
	pci_write_config32(tag, index_reg + 0x4, data);

}

#if 0

/* extension registers */
u32 pci_ext_read_config32(device_t nb_tag, device_t tag, u32 reg)
{
	int bus, device, function;
	/*get BAR3 base address for nbcfg0x1c */
	u32 addr = pci_read_config32(nb_tag, 0x1c) & ~0xF;
	//_pci_tagprintf (nb_tag, "pci_ext_read_config32: addr=%x\n", addr);
	_pci_break_tag (tag, &bus, &device, &function); 
	addr |= bus << 20 | device << 15 | function << 12 | reg;
	return *((u32 *) addr);
}

void pci_ext_write_config32(device_t nb_tag, device_t tag, u32 reg_pos, u32 mask, u32 val)
{
	u32 reg_old, reg;
	int bus, device, function;

	/*get BAR3 base address for nbcfg0x1c */
	u32 addr = pci_read_config32(nb_tag, 0x1c) & ~0xF;
	    
	_pci_break_tag (tag, &bus, &device, &function); 
	addr |= bus << 20 | device << 15 | function << 12 | reg_pos;

	reg = reg_old = *((u32 *) addr);
	reg &= ~mask;
	reg |= val;
	if (reg != reg_old) {
		*((u32 *) addr) = val;
	}
}

#endif


/* extension registers */
u16 pci_ext_read_config16(device_t nb_tag, device_t tag, u32 reg)
{
	u32 bus, dev, func, addr;
	device_t bar3_tag = _pci_make_tag(0, 0, 0);

	_pci_break_tag(tag, &bus, &dev, &func);
	
	/*get BAR3 base address for nbcfg0x1c */
	addr = (_pci_conf_read(bar3_tag, 0x1c)  & ~(0x0f))| 0x80000000;
	addr |= bus << 20 | dev << 15 | func << 12 | reg;
    addr |= 0x0e000000;  //added by oldtai for the cpu win changed  maybe cause ide not passing 
	//printf("ext ===========   addr=%x,bus=%x,dev=%x, fn=%x\n", addr, bus, dev, func);

	return *((volatile u16 *) addr);
}

void pci_ext_write_config16(device_t nb_tag, device_t tag, u32 reg_pos, u32 mask, u32 val)
{
        u32 bus, dev, func, addr;
	device_t bar3_tag = _pci_make_tag(0, 0, 0);

	_pci_break_tag(tag, &bus, &dev, &func);
	
	/*get BAR3 base address for nbcfg0x1c */
	addr = (_pci_conf_read(bar3_tag, 0x1c)  & ~(0x0f))| 0x80000000;
	addr |= bus << 20 | dev << 15 | func << 12 | reg_pos;
    addr |= 0x0e000000;  //added by oldtai for the cpu win changed
	//printf("addr=%x,bus=%x,dev=%x, fn=%x\n", addr, bus, dev, func);
	*((volatile u16 *) addr) = val;

}


u32 nbmisc_read_index(device_t nb_dev, u32 index)
{
	return nb_read_index((nb_dev), NBMISC_INDEX, (index));
}

void nbmisc_write_index(device_t nb_dev, u32 index, u32 data)
{
	nb_write_index((nb_dev), NBMISC_INDEX, ((index) | 0x80), (data));
}

u32 nbpcie_p_read_index(device_t dev, u32 index)
{
	return nb_read_index((dev), NBPCIE_INDEX, (index));
}

void nbpcie_p_write_index(device_t dev, u32 index, u32 data)
{
	nb_write_index((dev), NBPCIE_INDEX, (index), (data));
}

u32 nbpcie_ind_read_index(device_t nb_dev, u32 index)
{
	return nb_read_index((nb_dev), NBPCIE_INDEX, (index));
}

void nbpcie_ind_write_index(device_t nb_dev, u32 index, u32 data)
{
	nb_write_index((nb_dev), NBPCIE_INDEX, (index), (data));
}

u32 htiu_read_index(device_t nb_dev, u32 index)
{
	return nb_read_index((nb_dev), NBHTIU_INDEX, (index));
}

void htiu_write_index(device_t nb_dev, u32 index, u32 data)
{
	nb_write_index((nb_dev), NBHTIU_INDEX, ((index) | 0x100), (data));
}

u32 htiu_read_indexN(device_t nb_dev, u32 index)
{
	return nb_read_index((nb_dev), 0x94, (index));
}

void htiu_write_indexN(device_t nb_dev, u32 index, u32 data)
{
	nb_write_index((nb_dev), 0x94, ((index) | 0x100), (data));
}

u32 nbmc_read_index(device_t nb_dev, u32 index)
{
	return nb_read_index((nb_dev), NBMC_INDEX, (index));
}

void nbmc_write_index(device_t nb_dev, u32 index, u32 data)
{
	nb_write_index((nb_dev), NBMC_INDEX, ((index) | 1 << 9), (data));
}

void set_nbcfg_enable_bits(device_t nb_dev, u32 reg_pos, u32 mask, u32 val)
{
	u32 reg_old, reg;
	reg = reg_old = pci_read_config32(nb_dev, reg_pos);
	reg &= ~mask;
	reg |= val;
	if (reg != reg_old) {
		pci_write_config32(nb_dev, reg_pos, reg);
	}
}

void set_nbcfg_enable_bits_8(device_t nb_dev, u32 reg_pos, u8 mask, u8 val)
{
	u8 reg_old, reg;
	reg = reg_old = pci_read_config8(nb_dev, reg_pos);
	reg &= ~mask;
	reg |= val;
	if (reg != reg_old) {
		pci_write_config8(nb_dev, reg_pos, reg);
	}
}

void set_nbmc_enable_bits(device_t nb_dev, u32 reg_pos, u32 mask, u32 val)
{
	u32 reg_old, reg;
	reg = reg_old = nbmc_read_index(nb_dev, reg_pos);
	reg &= ~mask;
	reg |= val;
	if (reg != reg_old) {
		nbmc_write_index(nb_dev, reg_pos, reg);
	}
}

void set_htiu_enable_bits(device_t nb_dev, u32 reg_pos, u32 mask, u32 val)
{
	u32 reg_old, reg;
	reg = reg_old = htiu_read_index(nb_dev, reg_pos);
	reg &= ~mask;
	reg |= val;
	if (reg != reg_old) {
		htiu_write_index(nb_dev, reg_pos, reg);
	}
}

void set_nbmisc_enable_bits(device_t nb_dev, u32 reg_pos, u32 mask, u32 val)
{
	u32 reg_old, reg;
	reg = reg_old = nbmisc_read_index(nb_dev, reg_pos);
	reg &= ~mask;
	reg |= val;
	if (reg != reg_old) {
		nbmisc_write_index(nb_dev, reg_pos, reg);
	}
}

void set_pcie_enable_bits(device_t dev, u32 reg_pos, u32 mask, u32 val)
{
	u32 reg_old, reg;
	reg = reg_old = nb_read_index(dev, NBPCIE_INDEX, reg_pos);
	reg &= ~mask;
	reg |= val;
	if (reg != reg_old) {
		nb_write_index(dev, NBPCIE_INDEX, reg_pos, reg);
	}
}

void PcieReleasePortTraining(device_t nb_dev, device_t dev, u32 port)
{
	switch (port) {
	case 2:		/* GFX, bit4-5 */
	case 3:
		set_nbmisc_enable_bits(nb_dev, PCIE_LINK_CFG,
				       1 << (port + 2), 0 << (port + 2));
		break;
	case 4:		/* GPP, bit20-24 */
	case 5:
	case 6:
	case 7:
		set_nbmisc_enable_bits(nb_dev, PCIE_LINK_CFG,
				       1 << (port + 17), 0 << (port + 17));
		break;
	case 9:		/* GPP, bit 4,5 of miscind 0x2D */
	case 10:
		set_nbmisc_enable_bits(nb_dev, 0x2D,
				      1 << (port - 5), 0 << (port - 5));
		break;
	}
}

void set_pcie_dereset()
{
	u16 word;
	device_t sm_dev;

	sm_dev = _pci_make_tag(0, 0x14, 0);

	word = pci_read_config16(sm_dev, 0xA8);
	word |= (1 << 0) | (1 << 2);	/* Set Gpio6,4 as output */
	word &= ~((1 << 8) | (1 << 10));
	//pci_write_config16(sm_dev, word, 0xA8);
	pci_write_config16(sm_dev, 0xA8, word);
}


void set_pcie_reset()
{
	u16 word;
        device_t sm_dev;

        sm_dev = _pci_make_tag(0, 0x14, 0);

        word = pci_read_config16(sm_dev, 0xA8);
        word &= ~((1 << 0) | (1 << 2));    /* Set Gpio6,4 as output */
        word &= ~((1 << 8) | (1 << 10));
        pci_write_config16(sm_dev, 0xA8, word);

}	

/********************************************************************************************************
* Output:
*	0: no device is present.
*	1: device is present and is trained.
********************************************************************************************************/
u8 PcieTrainPort(device_t nb_dev, device_t dev, u32 port)
{
	u16 count = 5000;
	u32 lc_state, reg;
	u8 current, res = 0;
	u32 gfx_gpp_sb_sel;
	u32 current_link_width;
	u32 lane_mask;

	//void set_pcie_dereset();
	//void set_pcie_reset();	

	switch (port) {
	case 2 ... 3:
		gfx_gpp_sb_sel = PCIE_CORE_INDEX_GFX;
		break;
	case 4 ... 7:
		gfx_gpp_sb_sel = PCIE_CORE_INDEX_GPPSB;
		break;
	case 9 ... 10:
		gfx_gpp_sb_sel = PCIE_CORE_INDEX_GPP;
		break;
	}

	while (count--) {
		/* 5.7.5.21 step 2, delay 200us */
		udelay(300);
		lc_state = nbpcie_p_read_index(dev, 0xa5);	/* lc_state */
		printk_debug("PcieLinkTraining port=%x:lc current state=%x\n",
			     port, lc_state);
		current = lc_state & 0x3f;	/* get LC_CURRENT_STATE, bit0-5 */

		switch (current) {
		case 0x00:	/* 0x00-0x04 means no device is present */
		case 0x01:
		case 0x02:
		case 0x03:
		case 0x04:
			res = 0;
			count = 0;
			break;
		case 0x06:
			/* read back current link width [6:4]. */
			current_link_width = (nbpcie_p_read_index(dev, 0xA2) >> 4) & 0x7;
			/* 4 means 7:4 and 15:12
			 * 3 means 7:2 and 15:10
			 * 2 means 7:1 and 15:9
			 * egnoring the reversal case
			 */
			lane_mask = (0xFF << (current_link_width - 2) * 2) & 0xFF;
			reg = nbpcie_ind_read_index(nb_dev, 0x65 | gfx_gpp_sb_sel);
			reg |= lane_mask << 8 | lane_mask;
			reg = 0xE0E0; /* TODO: See the comments in rs780_pcie.c, at about line 145. */
			nbpcie_ind_write_index(nb_dev, 0x65 | gfx_gpp_sb_sel, reg);
			printk_debug("link_width=%x, lane_mask=%x",
				     current_link_width, lane_mask);
			set_pcie_reset();
			delay(1);
			set_pcie_dereset();
			break;
		case 0x07:	/* device is in compliance state (training sequence is doen). Move to train the next device */
			res = 1;	/* TODO: CIM sets it to 0 */
			count = 0;
			break;
		case 0x10:
			printk_info("PcieTrainPort reg\n");

			/*
			 * If access the pci_e express configure reg, should be
			 * enable the bar3 before, and close it when access done.
			 */
			enable_pcie_bar3(nb_dev);
			reg = pci_ext_read_config16(nb_dev, dev,PCIE_VC0_RESOURCE_STATUS);
			disable_pcie_bar3(nb_dev);
			printk_info("PcieTrainPort reg=0x%x\n", reg);
#if 0
			reg =
			    pci_ext_read_config32(nb_dev, dev,
						  PCIE_VC0_RESOURCE_STATUS);
			printk_debug("PcieTrainPort reg=0x%x\n", reg);
#endif
			/* check bit1 */
			if (reg & VC_NEGOTIATION_PENDING) {	/* bit1=1 means the link needs to be re-trained. */
				/* set bit8=1, bit0-2=bit4-6 */
				u32 tmp;
				reg =
				    nbpcie_p_read_index(dev,
							PCIE_LC_LINK_WIDTH);
				tmp = (reg >> 4) && 0x3;	/* get bit4-6 */
				reg &= 0xfff8;	/* clear bit0-2 */
				reg += tmp;	/* merge */
				reg |= 1 << 8;
				count++;	/* CIM said "keep in loop"?  */
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
	}
	return res;
}


