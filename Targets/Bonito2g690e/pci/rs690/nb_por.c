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
 * NB_POR :
 * 	PowerOnReset init for NB misc, mc, mcind, htiu, nbpci index register block
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


/*****************************************
* Compliant with CIM_33's ATINB_PCICFG_POR_TABLE
*****************************************/
static void nb_por_nbconfig_init(pcitag_t nb_tag)
{
	/* enable PCI Memory Access */
	set_nbcfg_enable_bits_8(nb_tag, 0x04, (u8)(~0xFD), 0x02);
	/* Set RCRB Enable */
	set_nbcfg_enable_bits_8(nb_tag, 0x84, (u8)(~0xFF), 0x1);

#ifdef	CFG_NBCONFIG_PM2_ENABLE
	/* enable r/w access to NB_BAR_PM2 regs */
	set_nbcfg_enable_bits_8(nb_tag, 0x4E, (u8)(~0x00), 0x02);
	/* setting the NB_BAR_PM2 temp io base address */
	set_nbcfg_enable_bits_8(nb_tag, 0x18, (u8)(~0x00), 
					(TEMP_PM2_BAR_BASE_ADDRESS | 0x01) & 0xff);
	set_nbcfg_enable_bits_8(nb_tag, 0x19, (u8)(~0x00), 
					(TEMP_PM2_BAR_BASE_ADDRESS & 0xff00) >> 8);
	/* enable decoding and broadcast pm2 to SB */
	set_nbcfg_enable_bits_8(nb_tag, 0x84, (u8)(~0x7A), 0x85);
	/* disable r/w access to NB_BAR_PM2 regs */
	set_nbcfg_enable_bits_8(nb_tag, 0x4E, (u8)(~0xFD), 0x00);
#endif	

	/* allow decode of 640k-1MB */
	set_nbcfg_enable_bits_8(nb_tag, 0x84, (u8)(~0xEF), 0x10);

	/* Reg4Ch[1]=1 (APIC_ENABLE) force cpu request with address 0xFECx_xxxx to south-bridge
	 * Reg4Ch[6]=1 (BMMsgEn) enable BM_Set message generation
	 * BMMsgEn */
	set_nbcfg_enable_bits_8(nb_tag, 0x4C, (u8)(~0x00), 0x42);
	
	/* Reg4Ch[16]=1 (WakeC2En) enable Wake_from_C2 message generation.
	 * Reg4Ch[18]=1 (P4IntEnable) Enable north-bridge to accept MSI with address 0xFEEx_xxxx from south-bridge */
	set_nbcfg_enable_bits_8(nb_tag, 0x4E, (u8)(~0xFF), 0x05);
	/* Reg94h[4:0] = 0x0  P drive strength offset 0
	 * Reg94h[6:5] = 0x2  P drive strength additive adjust */
	set_nbcfg_enable_bits_8(nb_tag, 0x94, (u8)(~0x80), 0x40);
	/* Reg94h[20:16] = 0x0  N drive strength offset 0
	 * Reg94h[22:21] = 0x2  N drive strength additive adjust */
	set_nbcfg_enable_bits_8(nb_tag, 0x96, (u8)(~0x80), 0x40);

	/* Reg80h[4:0] = 0x0  Termination offset
	 * Reg80h[6:5] = 0x2  Termination additive adjust */
	set_nbcfg_enable_bits_8(nb_tag, 0x80, (u8)(~0x80), 0x40);
	/* Reg80h[14] = 0x1   Enable receiver termination control */
	set_nbcfg_enable_bits_8(nb_tag, 0x81, (u8)(~0xFF), 0x40);

	/* Reg94h[15] = 0x1 Enables HT transmitter advanced features to be turned on
	 * Reg94h[14] = 0x1  Enable drive strength control */
	set_nbcfg_enable_bits_8(nb_tag, 0x95, (u8)(~0x3F), 0xC0);		// falling edge comp.
#ifndef	FIXUP_ID_CHANGE_ON_FPGA_BUG
	/* Reg94h[31:29] = 0x7 Enables HT transmitter de-emphasis */
	set_nbcfg_enable_bits_8(nb_tag, 0x97, (u8)(~0x1F), 0xE0);
	printf("WE HAVE SETTING THE NBCFG0x97 DE-EMPHASIS.\n");	
#endif

	/*Reg8Ch[10:9] = 0x3 Enables Gfx Debug BAR,
	 * force this BAR as mem type in rs690_gfx.c */
	set_nbcfg_enable_bits_8(nb_tag, 0x8D, (u8)(~0xFF), 0x06);
}

/*****************************************
* Compliant with CIM_33's ATINB_MCIndex_POR_TABLE
*****************************************/
static void nb_por_mc_index_init(pcitag_t nb_tag)
{
	set_nbmc_enable_bits(nb_tag, 0x7A, ~0xFFFFFF80, 0x0000005F);
	set_nbmc_enable_bits(nb_tag, 0xD8, ~0x00000000, 0x00600060);
	set_nbmc_enable_bits(nb_tag, 0xD9, ~0x00000000, 0x00600060);
	set_nbmc_enable_bits(nb_tag, 0xE0, ~0x00000000, 0x00000000);
	set_nbmc_enable_bits(nb_tag, 0xE1, ~0x00000000, 0x00000000);
	set_nbmc_enable_bits(nb_tag, 0xE8, ~0x00000000, 0x003E003E);
	set_nbmc_enable_bits(nb_tag, 0xE9, ~0x00000000, 0x003E003E);
	
}

/*****************************************
* Compliant with CIM_33's ATINB_MISCIND_POR_TABLE
* Compliant with CIM_33's MISC_INIT_TBL
*****************************************/
static void nb_por_misc_index_init(pcitag_t nb_tag)
{
	/* NB_MISC_IND_WR_EN + IOC_PCIE_CNTL
	 * Block non-snoop DMA request if PMArbDis is set.
	 * Set BMSetDis */
	set_nbmisc_enable_bits(nb_tag, 0x0B, ~0xFFFF0000, 0x00000180);
	set_nbmisc_enable_bits(nb_tag, 0x01, ~0xFFFFFFFF, 0x00000050);

	/* NBCFG (NBMISCIND 0x0): NB_CNTL -
	 *   HIDE_NB_AGP_CAP  ([0], default=1)HIDE
	 *   HIDE_P2P_AGP_CAP ([1], default=1)HIDE
	 *   HIDE_NB_GART_BAR ([2], default=1)HIDE
	 *   AGPMODE30        ([4], default=0)DISABLE
	 *   AGP30ENCHANCED   ([5], default=0)DISABLE
	 *   HIDE_AGP_CAP     ([8], default=1)ENABLE */
	set_nbmisc_enable_bits(nb_tag, 0x00, ~0xFFFF0000, 0x00000506);	/* set bit 10 for MSI */

	/* NBMISCIND:0x6A[16]= 1 SB link can get a full swing
	 *      set_nbmisc_bdf_enable_bits(nb_bdf, 0x6A, 0ffffffffh, 000010000);
	 * NBMISCIND:0x6A[17]=1 Set CMGOOD_OVERRIDE. */
	set_nbmisc_enable_bits(nb_tag, 0x6A, ~0xffffffff, 0x00020000);

	/* NBMISIND:0x40 Bit[8]=1 and Bit[10]=1 following bits are required to set in order to allow LVDS or PWM features to work. */
	set_nbmisc_enable_bits(nb_tag, 0x40, ~0xffffffff, 0x00000500);

	/* NBMISIND:0xC Bit[13]=1 Enable GSM mode for C1e or C3 with pop-up. */
	set_nbmisc_enable_bits(nb_tag, 0x0C, ~0xffffffff, 0x00002000);

	/* Set NBMISIND:0x1F[3] to map NB F2 interrupt pin to INTB# */
	set_nbmisc_enable_bits(nb_tag, 0x1F, ~0xffffffff, 0x00000008);
	
#ifdef	REG4C_DEBUG
	set_nbmisc_enable_bits(nb_tag, 0x00, 1 << 6, 1 << 6);
	set_nbmisc_enable_bits(nb_tag, 0x0b, 1 << 20, 1 << 20);
	set_nbmisc_enable_bits(nb_tag, 0x51, 1 << 20, 1 << 20);
	set_nbmisc_enable_bits(nb_tag, 0x53, 1 << 20, 1 << 20);
	set_nbmisc_enable_bits(nb_tag, 0x55, 1 << 20, 1 << 20);
	set_nbmisc_enable_bits(nb_tag, 0x57, 1 << 20, 1 << 20);
	set_nbmisc_enable_bits(nb_tag, 0x59, 1 << 20, 1 << 20);
	set_nbmisc_enable_bits(nb_tag, 0x5B, 1 << 20, 1 << 20);

	set_nbmisc_enable_bits(nb_tag, 0x00, 1 << 7, 1 << 7);
	set_nbmisc_enable_bits(nb_tag, 0x07, 0x000000f0, 0x30);
	/* Disable bus-master trigger event from SB and Enable set_slot_power message to SB */
	set_nbmisc_enable_bits(nb_tag, 0x0B, 0xffffffff, 0x500180);

#endif
}

/*****************************************
* Compliant with CIM_33's ATINB_HTIUNBIND_POR_TABLE
*****************************************/
static void nb_por_htiu_index_init(pcitag_t nb_tag)
{
	/* 0xBC:
	* Enables GSM mode for C1e or C3 with pop-up
	* Prevents AllowLdtStop from being asserted during HT link recovery
	* Allows FID cycles to be serviced faster. Needed for RS690 A12. No harm in RS690 A11 */
	set_htiu_enable_bits(nb_tag, 0x05, ~0xffffffff, 0x0BC);
	/* 0x4203A202:
	* Enables writes to pass in-progress reads
	* Enables streaming of CPU writes
	* Enables extended write buffer for CPU writes
	* Enables additional response buffers
	* Enables special reads to pass writes
	* Enables decoding of C1e/C3 and FID cycles
	* Enables HTIU-display handshake bypass.
	* Enables tagging fix */
	set_htiu_enable_bits(nb_tag, 0x06, ~0xFFFFFFFE, 0x4203A202);

	/* Enables byte-write optimization for IOC requests
	* Disables delaying STPCLK de-assert during FID sequence. Needed when enhanced UMA arbitration is used.
	* Disables upstream system-management delay */
	set_htiu_enable_bits(nb_tag, 0x07, ~0xFFFFFFF9, 0x001);

	/* HTIUNBIND 0x16 [1] = 0x1     Enable crc decoding fix */
	set_htiu_enable_bits(nb_tag, 0x16, ~0xFFFFFFFF, 0x2);
	
}

/*****************************************
* Compliant with CIM CLK CFG
*****************************************/
static void nb_por_clk_cfg_init(pcitag_t clk_tag)
{
	u8 val;
	pcitag_t nb_dev = _pci_make_tag(0, 0, 0);

	/* enable access to BUS0DEV0FUNC1 */
	val = _pci_conf_readn(nb_dev, 0x4c, 1);
	val |= 1 << 0;
	_pci_conf_writen(nb_dev, 0x4c, val, 1);

	/* HT clock gating setting */
	_pci_conf_writen(clk_tag, 0xF8, 0, 1);

	val = _pci_conf_readn(clk_tag, 0xF9, 1);
	val &= 0x30;
	val |= 0x0F;
	_pci_conf_writen(clk_tag, 0xF9, val, 1);

	_pci_conf_writen(clk_tag, 0xFA, 0, 1);

	_pci_conf_writen(clk_tag, 0xFB, 0, 1);

	/* MCLK switch GFX enable */
	val = _pci_conf_readn(clk_tag, 0xE0, 1);
	val |= 0x01;
	_pci_conf_writen(clk_tag, 0xE0, val, 1);

	/* disable access to BUS0DEV0FUNC1 */
	val = _pci_conf_readn(nb_dev, 0x4c, 1);
	val &= ~(1 << 0);
	_pci_conf_writen(nb_dev, 0x4c, val, 1);
	DEBUG_INFO("\r\nliujl 7 : nbconfig4c 0x%x\n", _pci_conf_read(nb_dev, 0x4C));
}

/* Whether enable sb full swing or not? */
static void nb_por_sb_fullswing(pcitag_t nb_tag)
{
	u32 rev;

	rev = get_nb_revision();
	if(rev < REV_RS690_A12){
		set_nbmisc_enable_bits(nb_tag, 0x6A, ~0xFFFEFFFF, 0x00010000);
	}
	
	return;
}

/*--------------------------------------------------------------------------*/

/*****************************************
* Compliant with CIM_33's ATINB_POR_INIT_JMPDI
* Configure RS690 registers to power-on default RPR.
* POR: Power On Reset
* RPR: Register Programming Requirements
*****************************************/
void nb_por_init(void)
{
	pcitag_t nb_tag = _pci_make_tag(0, 0, 0);
	pcitag_t clk_tag = _pci_make_tag(0, 0, 1);

	DEBUG_INFO("\n+++++++++++++++++++++++++++++++++++++++ NB POR STAGE WITH REV(%d)++++++++++++++++++++++++++++++++++++++++\n", get_nb_revision());

	/* ATINB_PCICFG_POR_TABLE, initialize the values for rs690 PCI Config registers */
	nb_por_nbconfig_init(nb_tag);
	/* ATINB_MCIND_POR_TABLE */
	nb_por_mc_index_init(nb_tag);

	/* ATINB_MCIND_POR_TABLE */
	/* ATINB_MISCIND_POR_TABLE */
	nb_por_misc_index_init(nb_tag);

	/* ATINB_HTIUNBIND_POR_TABLE */
	nb_por_htiu_index_init(nb_tag);

	/* ATINB_CLKCFG_POR_TABLE */
	nb_por_clk_cfg_init(clk_tag);

	/* rs690 A11 SB Link full swing? */
	nb_por_sb_fullswing(nb_tag);

	DEBUG_INFO("\n--------------------------------------------- NB POR STAGE DONE -----------------------------------------\n");

	return;
}

/*--------------------------------------------------------------------------*/

/* NB chip revision */
u8 get_nb_revision(void)
{
	pcitag_t nb_dev = _pci_make_tag(0, 0, 0);
	u32 val;

	val = _pci_conf_read(nb_dev, 0x00);
	if( (val >> 16) == 0x7911 ){
		return REV_RS690_A21;
	}

	val = _pci_conf_readn(nb_dev, 0x89, 1);
	if(val & (1 << 1)){
		return REV_RS690_A21;
	}else if(val & (1 << 0)){
		return REV_RS690_A12;
	}else{
		return REV_RS690_A11;
	}

	return REV_RS690_A11;
}

/* enable CFG access to Dev8, which is the SB P2P Bridge */
void nb_enable_dev8(pcitag_t nb_dev)
{
	set_nbmisc_enable_bits(nb_dev, 0x00, 1 << 6, 1 << 6);
}

/* disable CFG access to Dev8, which is the SB P2P Bridge */
void nb_disable_dev8(pcitag_t nb_dev)
{
	set_nbmisc_enable_bits(nb_dev, 0x00, 1 << 6, 0 << 6);
}

/*
 * The rs690 uses NBCONFIG:0x1c (BAR3) to map the PCIE Extended Configuration
 * Space to a 256MB range within the first 4GB of addressable memory.
 */
void nb_enable_bar3(pcitag_t nb_dev)
{
	DEBUG_INFO("rs690_enable_bar3\n");

	/* Enables writes to the BAR3 register. */
	set_nbcfg_enable_bits(nb_dev, 0x7C, 1 << 30, 1 << 30);

	/* setting the BAR size 2M */
	set_nbcfg_enable_bits(nb_dev, 0x84, 7 << 16, 1 << 16);

	/* setting the size and address */
	_pci_conf_write(nb_dev, PCI_BASE_ADDRESS_3, TEMP_EXT_CONF_BASE_ADDRESS);
	_pci_conf_write(nb_dev, PCI_BASE_ADDRESS_4, 0x00000000);

	/* enabel decode */
	set_htiu_enable_bits(nb_dev, 0x32, 1 << 28, 1 << 28);
}

/*
 * We should disable bar3 when we want to exit rs690_enable, because bar3 will be
 * remapped in set_resource later.
 */
void nb_disable_bar3(pcitag_t nb_dev)
{
	DEBUG_INFO("rs690_disable_bar3\n");

	/* disable decode */
	set_htiu_enable_bits(nb_dev, 0x32, 1 << 28, 0 << 28);

	/* enable write to BAR3 */
	set_nbcfg_enable_bits(nb_dev, 0x7C, 1 << 30, 1 << 30);

	/* clear the size and address */
	_pci_conf_write(nb_dev, PCI_BASE_ADDRESS_3, 0);

	/* disable write to the BAR3 reg */
	set_nbcfg_enable_bits(nb_dev, 0x7C, 1 << 30, 0 << 30);
}

/*----------------------------------------------------------------------------------*/
