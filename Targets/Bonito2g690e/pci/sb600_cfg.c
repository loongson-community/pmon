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
#include <linux/io.h>
#include <dev/pci/pcivar.h>
#include <dev/pci/pcireg.h>
#include <dev/pci/nppbreg.h>
#include <machine/bus.h>
#include <include/bonito.h>
#include <pmon.h>
#include <include/sb600.h>

#define	BASE_IO_VA	0xbfd00000
/*----------------------------------------------------------------------------------*/

/* PM basic read/write function */
void pmio_write_index(u16 port_base, u8 reg, u8 value)
{
	*(volatile u8 *)(BASE_IO_VA | port_base) = reg;
	*(volatile u8 *)(BASE_IO_VA | (port_base + 1)) = value;
}

u8 pmio_read_index(u16 port_base, u8 reg)
{
	*(volatile u8 *)(BASE_IO_VA | port_base) = reg;
	return *(volatile u8 *)(BASE_IO_VA | (port_base + 1));
}

/* pm register group access */
void pm_iowrite(u8 reg, u8 value)
{
	pmio_write_index(PM_INDEX, reg, value);
}

u8 pm_ioread(u8 reg)
{
	return pmio_read_index(PM_INDEX, reg);
}

void pm2_iowrite(u8 reg, u8 value)
{
	pmio_write_index(PM2_INDEX, reg, value);
}

u8 pm2_ioread(u8 reg)
{
	return pmio_read_index(PM2_INDEX, reg);
}

void set_pm_enable_bits(u32 reg_pos, u32 mask, u32 val)
{
	u8 reg_old, reg;
	
	reg = reg_old = pm_ioread(reg_pos);
	reg &= ~mask;
	reg |= val;
	if (reg != reg_old) {
		pm_iowrite(reg_pos, reg);
	}
}

void set_pm2_enable_bits(u32 reg_pos, u32 mask, u32 val)
{
	u8 reg_old, reg;
	
	reg = reg_old = pm2_ioread(reg_pos);
	reg &= ~mask;
	reg |= val;
	if (reg != reg_old) {
		pm2_iowrite(reg_pos, reg);
	}
}

/*----------------------------------------------------------------------------------*/

/* sb600 devices pci CSR access */
void set_sbcfg_enable_bits(pcitag_t sb_dev, u32 reg_pos, u32 mask, u32 val)
{
	u32 reg_old, reg;
	
	reg = reg_old = _pci_conf_read(sb_dev, reg_pos);
	reg &= ~mask;
	reg |= val;
	if (reg != reg_old) {
		_pci_conf_write(sb_dev, reg_pos, reg);
	}
}

/*--------------------------------------------------------------------------------------*/
/* ablink : axconfig & abconfig */
void alink_ab_indx(unsigned int reg_space, unsigned int reg_addr, unsigned int mask, unsigned int val)
{
	unsigned int tmp;
	linux_outl((reg_space & 0x3) << 30 | reg_addr, AB_INDX);
	tmp = linux_inl(AB_DATA);

	tmp &= ~mask;
	tmp |= val;

	/* DEBUG_INFO("about write %x, index=%x", tmp, (reg_space&0x3)<<30 | reg_addr); */
	linux_outl((reg_space & 0x3) << 30 | reg_addr, AB_INDX);	/* probably we dont have to do it again. */
	linux_outl(tmp, AB_DATA);
}

/* ablink : common index & personal index */
/* space = 0: AX_INDXC, AX_DATAC
*   space = 1: AX_INDXP, AX_DATAP
 */
void alink_ax_indx(unsigned int space /*c or p? */ , unsigned int axindc, unsigned int mask, unsigned int val)
{
	unsigned int tmp;

	/* read axindc to tmp */
	linux_outl(space << 30 | space << 3 | 0x30, AB_INDX);
	linux_outl(axindc, AB_DATA);
	linux_outl(space << 30 | space << 3 | 0x34, AB_INDX);
	tmp = linux_inl(AB_DATA);

	tmp &= ~mask;
	tmp |= val;

	/* write tmp */
	linux_outl(space << 30 | space << 3 | 0x30, AB_INDX);
	linux_outl(axindc, AB_DATA);
	linux_outl(space << 30 | space << 3 | 0x34, AB_INDX);
	linux_outl(tmp, AB_DATA);
}

/*--------------------------------------------------------------------------------------------------------*/
