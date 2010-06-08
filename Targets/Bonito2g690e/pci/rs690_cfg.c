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

#include <sys/linux/types.h>
#include <sys/param.h>
#include <sys/device.h>
#include <sys/systm.h>
#include <sys/malloc.h>
#include <dev/pci/pcivar.h>
#include <dev/pci/pcireg.h>
#include <dev/pci/nppbreg.h>
#include <machine/bus.h>
#include <include/bonito.h>
#include <pmon.h>
#include <include/rs690.h>
//#include "dev/pci/rs690/rs690_config.h"
/*----------------------------------------------------------------------------------*/

u32 nb_read_index(pcitag_t tag, u32 index_reg, u32 index)
{
//	printf("before _pci_conf_write :0x%x,0x%x,0x%x\n",tag,index_reg,index);
		_pci_conf_write(tag, index_reg, index);
//		printf("after _pci_conf_write\n");
//		printf("_pci_conf_read data,...0x%x\n",_pci_conf_read(tag,index_reg+0x04));
	return _pci_conf_read(tag, index_reg + 0x04);
}

void nb_write_index(pcitag_t tag, u32 index_reg, u32 index, u32 data)
{
	_pci_conf_write(tag, index_reg, index);
	_pci_conf_write(tag, index_reg + 0x04, data);
}

/*-----------------------------------------------------------------------------------*/

u32 nbmisc_read_index(pcitag_t tag, u32 index)
{
	return nb_read_index(tag, NB_MISC_INDEX, (index));
}

void nbmisc_write_index(pcitag_t tag, u32 index, u32 data)
{
	nb_write_index(tag, NB_MISC_INDEX, ((index) | 0x80), (data));
}

u32 htiu_read_index(pcitag_t tag, u32 index)
{
	return nb_read_index((tag), HTIU_NB_INDEX, (index));
}

void htiu_write_index(pcitag_t tag, u32 index, u32 data)
{
	nb_write_index((tag), HTIU_NB_INDEX, ((index) | 0x100), (data));
}

u32 nbmc_read_index(pcitag_t tag, u32 index)
{
	return nb_read_index((tag), NB_MC_INDEX, (index));
}

void nbmc_write_index(pcitag_t tag, u32 index, u32 data)
{
	nb_write_index((tag), NB_MC_INDEX, ((index) | 1 << 9), (data));
}

u32 nbmc_ind_read_index(pcitag_t tag, u32 index)
{
	return nb_read_index((tag), NB_MC_IND_INDEX, (index));
}

void nbmc_ind_write_index(pcitag_t tag, u32 index, u32 data)
{
	nb_write_index((tag), NB_MC_IND_INDEX, ((index) | 1 << 23), (data));
}

void set_nbmc_ind_enable_bits(pcitag_t tag, u32 reg_pos, u32 mask, u32 val)
{
	u32 reg_old, reg;
	reg = reg_old = nbmc_ind_read_index(tag, reg_pos);
	reg &= ~mask;
	reg |= val;
	if (reg != reg_old) {
		nbmc_ind_write_index(tag, reg_pos, reg);
	}
}

void set_nbmc_enable_bits(pcitag_t tag, u32 reg_pos, u32 mask, u32 val)
{
	u32 reg_old, reg;
	reg = reg_old = nbmc_read_index(tag, reg_pos);
	reg &= ~mask;
	reg |= val;
	if (reg != reg_old) {
		nbmc_write_index(tag, reg_pos, reg);
	}
}

void set_htiu_enable_bits(pcitag_t tag, u32 reg_pos, u32 mask, u32 val)
{
	u32 reg_old, reg;
	reg = reg_old = htiu_read_index(tag, reg_pos);
	reg &= ~mask;
	reg |= val;
	if (reg != reg_old) {
		htiu_write_index(tag, reg_pos, reg);
	}
}

void set_nbmisc_enable_bits(pcitag_t tag, u32 reg_pos, u32 mask, u32 val)
{
	u32 reg_old, reg;
	reg = reg_old = nbmisc_read_index(tag, reg_pos);
	reg &= ~mask;
	reg |= val;
	if (reg != reg_old) {
		nbmisc_write_index(tag, reg_pos, reg);
	}
}

/*-----------------------------------------------------------------------------------------*/

u32 nbpcie_ind_gppsb_read_index(pcitag_t tag, u32 index)
{
	return nb_read_index((tag), NB_PCIE_INDX_ADDR, (index | PCIE_CORE_INDEX_GPPSB));
}

void nbpcie_ind_gppsb_write_index(pcitag_t tag, u32 index, u32 data)
{
	nb_write_index((tag), NB_PCIE_INDX_ADDR, (index | PCIE_CORE_INDEX_GPPSB), (data));
}

u32 nbpcie_ind_gfx_read_index(pcitag_t tag, u32 index)
{
	return nb_read_index((tag), NB_PCIE_INDX_ADDR, (index | PCIE_CORE_INDEX_GFX));
}

void nbpcie_ind_gfx_write_index(pcitag_t tag, u32 index, u32 data)
{
	nb_write_index((tag), NB_PCIE_INDX_ADDR, (index | PCIE_CORE_INDEX_GFX), (data));
}

void set_pcie_gppsb_enable_bits(pcitag_t tag, u32 reg_pos, u32 mask, u32 val)
{
	u32 reg_old, reg;
	reg = reg_old = nb_read_index(tag, NB_PCIE_INDX_ADDR | PCIE_CORE_INDEX_GPPSB, reg_pos);
	reg &= ~mask;
	reg |= val;
	if (reg != reg_old) {
		nb_write_index(tag, NB_PCIE_INDX_ADDR | PCIE_CORE_INDEX_GPPSB, reg_pos, reg);
	}
}

void set_pcie_gfx_enable_bits(pcitag_t tag, u32 reg_pos, u32 mask, u32 val)
{
	u32 reg_old, reg;
printf("nb_read_index,tag:=0x%x\n",tag);
	reg = reg_old = nb_read_index(tag, NB_PCIE_INDX_ADDR | PCIE_CORE_INDEX_GFX, reg_pos);
printf("nb_read_index reg[0x%x],=0x%x\n",tag,reg);
	reg &= ~mask;
	reg |= val;
	if (reg != reg_old) {
			printf("nb_read_index is ok\n");
		nb_write_index(tag, NB_PCIE_INDX_ADDR | PCIE_CORE_INDEX_GFX, reg_pos, reg);
	}
	printf("set_pcie_gfx_enable_bits is ok\n");
}

u32 nbpcie_p_read_index(pcitag_t tag, u32 index)
{
	return nb_read_index((tag), PCIE_PORT_INDEX, (index));
}

void nbpcie_p_write_index(pcitag_t tag, u32 index, u32 data)
{
	nb_write_index((tag), PCIE_PORT_INDEX, (index), (data));
}

void set_pcie_p_enable_bits(pcitag_t tag, u32 reg_pos, u32 mask, u32 val)
{
	u32 reg, reg_old;

	reg = reg_old = nbpcie_p_read_index(tag, reg_pos);
	reg &= ~mask;
	reg |= val;
	if(reg != reg_old){
		nbpcie_p_write_index(tag, reg_pos, reg);
	}
}

/*--------------------------------------------------------------------------------*/

void set_nbcfg_enable_bits(pcitag_t tag, u32 reg_pos, u32 mask, u32 val)
{
	u32 reg_old, reg;
	reg = reg_old = _pci_conf_read(tag, reg_pos);
	reg &= ~mask;
	reg |= val;
	if (reg != reg_old) {
		_pci_conf_write(tag, reg_pos, reg);
	}
}

void set_nbcfg_enable_bits_8(pcitag_t tag, u32 reg_pos, u8 mask, u8 val)
{
	u32 reg_old, reg, tmp, ext_mask;
	
	tmp = reg_pos & 0x03;
	reg_pos &= 0xfffffffc;

	reg =  reg_old = _pci_conf_read(tag, reg_pos);
	ext_mask = mask << (tmp * 8);
	ext_mask = ~ext_mask;
	reg &= ext_mask;
	reg |= (val << (tmp * 8));
	if (reg != reg_old) {
		_pci_conf_write(tag, reg_pos, reg);
	}
}

/*--------------------------------------------------------------------------------------*/

/* PCI extension registers */
u32 _pci_ext_conf_read(pcitag_t tag, u32 reg)
{
	u32 bus, dev, func, addr;
	pcitag_t bar3_tag = _pci_make_tag(0, 0, 0);

	_pci_break_tag(tag, &bus, &dev, &func);
	
	/*get BAR3 base address for nbcfg0x1c */
	addr = (_pci_conf_read(bar3_tag, 0x1c)  & ~(0x0f))| 0xA0000000;
	addr |= bus << 20 | dev << 15 | func << 12 | reg;
//	printf("addr=%x,bus=%x,dev=%x, fn=%x\n", addr, bus, dev, func);
	return *((volatile u32 *) addr);
}

void _pci_ext_conf_write(pcitag_t tag, u32 reg_pos, u32 val)
{
	u32 bus, dev, func, addr;
	pcitag_t bar3_tag = _pci_make_tag(0, 0, 0);

	_pci_break_tag(tag, &bus, &dev, &func);
	
	/*get BAR3 base address for nbcfg0x1c */
	addr = (_pci_conf_read(bar3_tag, 0x1c)  & ~(0x0f))| 0xA0000000;
	addr |= bus << 20 | dev << 15 | func << 12 | reg_pos;
//	printf("addr=%x,bus=%x,dev=%x, fn=%x\n", addr, bus, dev, func);
	*((volatile u32 *) addr) = val;
}

u16 _pci_ext_conf_read16(pcitag_t tag, u32 reg)
{
	u32 bus, dev, func, addr;
	pcitag_t bar3_tag = _pci_make_tag(0, 0, 0);

	_pci_break_tag(tag, &bus, &dev, &func);
	
	/*get BAR3 base address for nbcfg0x1c */
	addr = (_pci_conf_read(bar3_tag, 0x1c)  & ~(0x0f))| 0xA0000000;
	addr |= bus << 20 | dev << 15 | func << 12 | reg;
//	printf("addr=%x,bus=%x,dev=%x, fn=%x\n", addr, bus, dev, func);

	return *((volatile u16 *) addr);
}

void _pci_ext_conf_write16(pcitag_t tag, u32 reg_pos, u16 val)
{
	u32 bus, dev, func, addr;
	pcitag_t bar3_tag = _pci_make_tag(0, 0, 0);

	_pci_break_tag(tag, &bus, &dev, &func);
	
	/*get BAR3 base address for nbcfg0x1c */
	addr = (_pci_conf_read(bar3_tag, 0x1c)  & ~(0x0f))| 0xA0000000;
	addr |= bus << 20 | dev << 15 | func << 12 | reg_pos;
//	printf("addr=%x,bus=%x,dev=%x, fn=%x\n", addr, bus, dev, func);
	*((volatile u16 *) addr) = val;
}

/*--------------------------------------------------------------------------------------------------*/

/*
 * clkind_read : read gfx clock indirect register block
 */
u32 gfx_clkind_read(pcitag_t gfx_dev, u32 index)
{
	u32 gfx_bar2 = _pci_conf_read(gfx_dev, PCI_BASE_ADDRESS_2) & ~0xF;
	gfx_bar2 |= 0xa0000000;

	*(volatile u32 *)(gfx_bar2+CLK_CNTL_INDEX) = index & 0x7F;
	return *(volatile u32 *)(gfx_bar2+CLK_CNTL_DATA);
}

/*
 * clkind_write : write to gfx clock indirect register block
 */
void gfx_clkind_write(pcitag_t gfx_dev, u32 index, u32 val)
{
	u32 gfx_bar2 = _pci_conf_read(gfx_dev, PCI_BASE_ADDRESS_2) & ~0xF;
	gfx_bar2 |= 0xa0000000;

	*(volatile u32 *)(gfx_bar2+CLK_CNTL_INDEX) = index | 1<<7;
	*(volatile u32 *)(gfx_bar2+CLK_CNTL_DATA)  = val;
}

/*-----------------------------------------------------------------------------------------*/
