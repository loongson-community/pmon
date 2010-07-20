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
#include <linux/io.h>
#include <sys/linux/types.h>
#include <include/bonito.h>
#include <machine/pio.h> 
#include <include/rs690.h>
#include <pci/rs690_cfg.h>
#include "rs690_config.h"

/*-----------------------------------------------------------------------*/
/*
 * pcie_gen_reset_dispatch
 * issue CF9 reset signal
 */
void pcie_gen_reset_dispatch(void)
{
	pcitag_t nb_dev = _pci_make_tag(0, 0, 0);

	/* enable PLL PLLBUF of PCIE */
	set_pcie_gppsb_enable_bits(nb_dev, 0x64, 0xFF << 16, 0xFF << 16);

	/* issue reset signal */
	linux_outb(0x06, 0xCF9);

	return;
}

/*
 * pcie_init_dispatch :
 * 	some pcie init for resources access
 */
void pcie_init_dispatch(void)
{
	/* inited the Legacy Timer which is @0xbfd00041 & 0xbfd00043 TIMER1 */

	/* for CIM, they configured the PCI MMIO7 In K8 for Acessing BAR in BB */
	return;
}

/*
 * pcie_exit_dispatch :
 *  Clear K8 MMIO7 after PCIE init
 */
void pcie_exit_dispatch(void)
{
	/* cleanup the PCIE MMIO7 K8 resources  */
	
	return;
}

/*
 * pcie_check_dual_cfg_dispatch :
 * 0 : single slot
 * 1 : dual slot
 */
u8 pcie_check_dual_cfg_dispatch(void)
{
	return 0;
}

/*
 * pcie_port2_route_dispatch :
 * GFX PORT2 route routine, none so far
 */
void pcie_port2_route_dispatch(void)
{
	return;
}

/*
 * pcie_port3_route_dispatch :
 * GFX PORT3 route routine, none so far
 */
void pcie_port3_route_dispatch(void)
{
	return;
}

/*
 * pcie_port2_reset_dispatch :
 * GFX PORT2 user defined reset routine
 * Maybe we could use 0xCF9 here.
 */
void pcie_port2_reset_dispatch(void)
{
	return;
}

/*
 * pcie_port3_reset_dispatch :
 * GFX PORT3 user defined reset routine
 * Maybe we could use 0xCF9 here.
 */
void pcie_port3_reset_dispatch(void)
{
	return;
}

/*
 * pcie_deassert_port2_reset_dispatch :
 * GFX PORT2 user defined deassert reset routine
 */
void pcie_deassert_port2_reset_dispatch(void)
{
	return;
}

/*
 * pcie_deassert_port3_reset_dispatch :
 * GFX PORT3 user defined deassert reset routine
 */
void pcie_deassert_port3_reset_dispatch(void)
{
	return;
}

/*--------------------------------------------------------------------*/

void gfx_uma_state_dispatch(void)
{
	return;
}


