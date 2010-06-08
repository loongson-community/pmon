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

#ifndef	_RS690_CFG_H_
#define	_RS690_CFG_H_
#include <sys/linux/types.h>
#include <dev/pci/pcivar.h>
/*------------------------------------ REG ACCESS FUNCTIONS-------------------------------------*/

extern u32 nbmisc_read_index(pcitag_t tag, u32 index);
extern void nbmisc_write_index(pcitag_t tag, u32 index, u32 data);
extern u32 htiu_read_index(pcitag_t tag, u32 index);
extern void htiu_write_index(pcitag_t tag, u32 index, u32 data);
extern u32 nbmc_read_index(pcitag_t tag, u32 index);
extern void nbmc_write_index(pcitag_t tag, u32 index, u32 data);
extern u32 nbmc_ind_read_index(pcitag_t tag, u32 index);
extern void nbmc_ind_write_index(pcitag_t tag, u32 index, u32 data);
extern void set_nbmc_ind_enable_bits(pcitag_t tag, u32 reg_pos, u32 mask, u32 val);
extern void set_nbmc_enable_bits(pcitag_t tag, u32 reg_pos, u32 mask, u32 val);
extern void set_htiu_enable_bits(pcitag_t tag, u32 reg_pos, u32 mask, u32 val);
extern void set_nbmisc_enable_bits(pcitag_t tag, u32 reg_pos, u32 mask, u32 val);

extern u32 nbpcie_ind_gppsb_read_index(pcitag_t tag, u32 index);
extern void nbpcie_ind_gppsb_write_index(pcitag_t tag, u32 index, u32 data);
extern u32 nbpcie_ind_gfx_read_index(pcitag_t tag, u32 index);
extern void nbpcie_ind_gfx_write_index(pcitag_t tag, u32 index, u32 data);
extern void set_pcie_gppsb_enable_bits(pcitag_t tag, u32 reg_pos, u32 mask, u32 val);
extern void set_pcie_gfx_enable_bits(pcitag_t tag, u32 reg_pos, u32 mask, u32 val);

extern u32 nbpcie_p_read_index(pcitag_t tag, u32 index);
extern void nbpcie_p_write_index(pcitag_t, u32 index, u32 data);
extern void set_pcie_p_enable_bits(pcitag_t tag, u32 reg_pos, u32 mask, u32 val);

extern void set_nbcfg_enable_bits(pcitag_t tag, u32 reg_pos, u32 mask, u32 val);
extern void set_nbcfg_enable_bits_8(pcitag_t tag, u32 reg_pos, u8 mask, u8 val);

extern  u32 _pci_ext_conf_read(pcitag_t tag, u32 reg);
extern void _pci_ext_conf_write(pcitag_t tag, u32 reg_pos, u32 val);
extern  u16 _pci_ext_conf_read16(pcitag_t tag, u32 reg);
extern void _pci_ext_conf_write16(pcitag_t tag, u32 reg_pos, u16 val);

extern u32 gfx_clkind_read(pcitag_t gfx_dev, u32 index);
extern void gfx_clkind_write(pcitag_t gfx_dev, u32 index, u32 val);

#endif	/* _RS690_CFG_H_ */
/*------------------------------------------------------------------------------------------------------*/
