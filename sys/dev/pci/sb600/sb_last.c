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
void update_hpet_table(void)
{
	/* relative to acpi table, may no need now */ 
	/* this code will be called by default --xiangy*/
}

void sb_last_post_setting(void)
{
	u8 rev = get_sb600_revision();
	pcitag_t pcib_dev;

	PRINTF_DEBUG();
	if(rev == REV_SB600_A11){
		update_hpet_table();
	}
	if(rev == REV_SB600_A12){
		/* clear HPET ECC */
		set_pm_enable_bits(0x9A, 1 << 7, 0 << 7);
	}
	if(rev >= REV_SB600_A13){
		/* clear HPET ECC */
		set_pm_enable_bits(0x9A, 1 << 7, 0 << 7);
		/* enable load new value for HPET */
		set_pm_enable_bits(0x9F, 1 << 5, 1 << 5);
		/* make HPET revision id to be 0x01 */
		set_pm_enable_bits(0x9E, (1 << 6) | (1 << 7), (1 << 6) | (1 << 7));
	}

	if(ati_sb_cfg.hpet_en == SB_DISABLE){
		update_hpet_table();
	}
	
	/* RPR4.5 Master Latency Timer PCIB_PCI_config 0x0D/0x1B = 0x40
	 * Enables the PCIB to retain ownership of the bus on the
	 * Primary side and on the Secondary side when GNT# is deasserted.
	 */
	pcib_dev = _pci_make_tag(0, 20, 4);
	_pci_conf_writen(pcib_dev, 0x0D, 0x40, 1);
	_pci_conf_writen(pcib_dev, 0x1B, 0x40, 1);

	return;
}

void sb_last_c3_popup_setting(void)
{
	pcitag_t dev;
	u32 base, reg10;
	u8 type = get_sb600_platform();

	PRINTF_DEBUG();
	if(type == K8_ID){
		dev = _pci_make_tag(0, 20, 5);
		reg10 = _pci_conf_read(dev, 0x10);
		if( (reg10 != 0xffffffff) && (reg10 != 0x00) ){
			/* RPR8.3 K8 C3Pop-up for AC97, AC97_Memory_Mapped 0x80[1] = 1.
			 * If the CPU is K8, and C3PopUp function is enabled, then the BIOS should 
			 * set this mirror bit to inform the ac97 driver that C3Pop-up is enabled.
			 * The driver will in turn not set the SetBusBusy bit in Memory_Mapped 0x04[14]
			 */
			base = (reg10 & 0xfffffff0) | 0xA0000000;
			*(volatile u8 *)(base + 0x80) |= 1 << 1;
		}		
	}else{
		/* RPR2.4.2	C-State Function for the P4 platform
		 * Intel cpu C3 setting
		 * Smbus_PCI_config 0x64 [5] = 0 BmReq# is no longer used in P4 platform. Clear this bit 
		 * to disable the BmReq# input.
		 * Smbus_PCI_config 0x64 [4] = 1
		 */
		dev = _pci_make_tag(0, 20, 0);
		set_sbcfg_enable_bits(dev, 0x64, 1 << 5, 0 << 5);
		set_sbcfg_enable_bits(dev, 0x64, 1 << 4, 1 << 4);
	}

	return;
}


void sb_last_c3_popup_next_setting(void)
{
	u8 rev = get_sb600_revision();
	u8 type = get_sb600_platform();

	PRINTF_DEBUG();
	if(type == K8_ID){
		/* RPR2.3.3 C-State and VID/FID Change for the K8 Platform
		 * BIOS should not report BM_STS or ARB_DIS to OS if C3 pop-up is enabled.
		 * With C3 pop-up, ARB_DIS should not be set in any case and BM_STS 
		 * is always cleared so OS will always issue C3.
		 */
		/* assume single core cpu is present */
		set_pm_enable_bits(0x9A, ((1 << 2) | (1 << 4) | (1 << 5)), 1 << 2);

		/* PM_IO 0x8F [4] = 0 for system with RS690
		 * Note: RS690 north bridge has AllowLdtStop built for both display and PCIE traffic to 
		 * wake up the HT link. BmReq# needs to be ignored otherwise may cause LDTSTP# not 
		 * to toggle. 
		 */

		/* assume RS690 chipset is present */
		set_pm_enable_bits(0x8F, (1 << 4) | (1 << 5), (1 << 5));

		/* StutterTime=01 for minimum LDTSTP# assertion duration of 1us in C3 */
		set_pm_enable_bits(0x8B, 0xFF, 0x01);

		/* Enable stutter mode for C3 and set 2us duration for LDTSTP# assertion during 
		 * VID/FID change
		 */
		set_pm_enable_bits(0x8A, 0xFF, 0x90);
	
		/* Minimum LDTSTP# deassertion duration(in micro seconds) in stutter mode */
		if(rev <= REV_SB600_A13){
			set_pm_enable_bits(0x88, 0xFF, 0x06);
		}else{
			set_pm_enable_bits(0x88, 0xFF, 0x10);			
		}

		/* Set to 1 to allow wakeup from C3 if break event happens before LDTSTOP# assertion */
		set_pm_enable_bits(0x7C, 1 << 0, 1 << 0);

		/* Must be 0 for k8 platforms as per RPR */
		set_pm_enable_bits(0x68, 1 << 1, 0 << 1);
		set_pm_enable_bits(0x8D, 1 << 6, 0 << 6);
		
		/* This bit should be cleared if C3 popup is enabled. No sideeffect even if it 
		 * is cleared otherwise
		 */
		set_pm_enable_bits(0x61, 1 << 2, 0 << 2);
	
		/* Setting this bit will convert C2 into C3. We should clear this bit for usb to 
		 * function properly.
		 */
		set_pm_enable_bits(0x42, 1 << 2, 0 << 2);

		if(rev == REV_SB600_A21){
			/* This setting provides 16us delay before the assertion of LDTSTOP# when C3 
			 * is entered. The delay will allow USB DMA to go on in a continous manner
			 */
			set_pm_enable_bits(0x89, 0xff, 0x10);

		 	/* Set this bit to allow pop-up request being latched during the minimum 
			 * LDTSTP# assertion time
			 */
			set_pm_enable_bits(0x52, 0x00, 1 << 7);
		}
	}else{
		/* RPR2.4.2	C-State Function for the P4 platform : Intel cpu C3 setting
		 * PM_IO 0x67 [2] = 1	For a P4 CPU that supports C-states deeper than C1, 
		 * 		this bit must be set to enable the SB600 C-state function.
		 * PM_IO 0x67 [1] = 1	For a P4 CPU that supports the C3 function, 
		 * 		this bit must be set to enable the SLP# function.
		 * PM_IO 0x68 [0] = 1	For a P4 CPU that supports the PBE# function, 
		 * 		this bit must be set to 1 for the C-state to work properly. 
		 * PM_IO 0x68 [1] = 0 	With this bit set to 1, pending break event will prevent SB
		 * 		from entering C state. LVL read will be completed with no effect. 
		 * 		The function will need to be qualified in the future. Currently this bit must be 0.
		 * PM_IO 0x8D [6] = 1 	Right after SB exits from C state, LVL read will have no
		 * 		effect until the previous break event has been cleared. Setting the bit will 
		 * 		prevent SB from sending STPCLK# message to NB during such time. 
		 * 		This bit must be set to 1.
		 * PM_IO 0x71 [1] = 1	This bit enables pop-up in C3/4 state. For the internal 
		 * 		bus mastering request, SB will deassert appropriate control signals and 
		 * 		put CPU into C2 state. In the meanwhile NB will enable its arbiter. 
		 * 		Then DMA can go through. When DMA finishes, SB will assert those signals 
		 * 		again and put CPU back into C3/4 after some idle time.
		 * PM_IO 0x73 [7:0] = 80h (default)	This register defines the idle time for C3/4 pop-up. 
		 * 		The value corresponds to Alink clocks. With the default value "80" the 
		 * 		delay is about 2us.
		 * PM_IO 0x72 [7:0] = 0Bh	This register defines the delay between the deassertion 
		 * 		of DPRSLPVR and CPU_STP# in C4 timing sequence. 
		 * 		With the value "0B" the delay is 20 to 22 us.
		 * PM_IO 0x51 [5:0] = 05h	This register defines the delay between the deassertion 
		 * 		of CPU_STP# and SLP# in C3/4 timing sequence. With the value "05" the 
		 * 		delay is 32 to 40 us.
		 * PM_IO 0x9A [5] = 1	For system with dual core CPU, set this bit to 1 to
		 * 		automatically clear BM_STS when the C3/4 sequence is being initiated..
		 * PM_IO 0x9A [4] = 1	Set this bit to 1 and BM_STS will cause C3 to pop-up or 
		 * 		wakeup even if BM_RLD is not set to 1.
		 * PM_IO 0x9A [3] = 1	Set this bit to 1 to automatically disable the internal 
		 * 		arbiter when C3/4 sequence is being started. The internal arbiter will be 
		 * 		enabled again during pop-up or after wakeup.
		 * Smbus_PCI_config 0x64 [5] = 0	BmReq# is no longer used in P4 platform. 
		 * 		Clear this bit to disable the BmReq# input.
		 * Smbus_PCI_config 0x64 [4] = 1
		 * PM_IO 0x61 [2] = 1	Set these two bits to enable the internal bus mastering 
		 * 		to cause C3 pop-up.
		 * SB600 A13 and above	PM_IO 0x51 [7] = 1	Set this bit to 1 to extend STPCLK# 
		 * 		hold time with respect to SLP# to be more than 3us.
		 */
		set_pm_enable_bits(0x67, (1 << 1) | (1 << 2), (1 << 1) | (1 << 2));
		set_pm_enable_bits(0x71, 1 << 1, 1 << 1);
		set_pm_enable_bits(0x68, (1 << 0) | (1 << 1), 1 << 0);
		set_pm_enable_bits(0x8D, 1 << 6, 1 << 6);
		set_pm_enable_bits(0x73, 0xFF, 0x80);
		set_pm_enable_bits(0x72, 0xFF, 0x0B);
		set_pm_enable_bits(0x51, 0x3F, 0x05);
		set_pm_enable_bits(0x9A, (1 << 3) | (1 << 4) | (1 << 5), (1 << 3) | (1 << 4) | (1 << 5));
		set_pm_enable_bits(0x61, 1 << 2, 1 << 2);
		if(rev > REV_SB600_A12){
			set_pm_enable_bits(0x51, 1 << 7, 1 << 7);		
		}

		/* Note: For SB600 to be paired with NB's that do not support C3/4 pop-up
		 * (rs690 SUPPORT this feature) following register settings should be followed : 
		 * PM_IO 0x9A[5] = 0
		 * PM_ IO 0x 9A[4] = 0
		 * PM_ IO 0x 9A[3] = 1 (PM_IO 0x2C/2D setting should be different from PM2_CNT_BLK 
		 * 		that BIOS reports to OS)
		 * PM_ IO 0x 8F[5] = 0
		 * PM_ IO 0x 71[2] = 0
		 * PM_ IO 0x 71[1] = 0
		 * PM_ IO 0x 68[1] = 1
		 * PM_ IO 0x 8D[6] = 0
		 */
	}
	
	return;
}

void sb_last_p4_init(void)
{
	u8 type = get_sb600_platform();

	PRINTF_DEBUG();
	if(type == P4_ID){
		/* our platform is K8, there is no need yet : we should judge the CPU id for
		 * PBE support setting
		 */
	}

	return;
}

void sb_last_cmos_init(void)
{
	/* store the ati_sb_cfg base addr to RTC, we are no need yet. */
	PRINTF_DEBUG();
	return;
}

void sb_last_init_pci_autoclock_run(void)
{
	u32	val;
	pcitag_t dev = _pci_make_tag(0, 20, 4);

	PRINTF_DEBUG();
	_pci_conf_write(dev, 0x4C, ati_sb_cfg.clkrun_ctrl);

	val = _pci_conf_readn(dev, 0x50, 2);
	val &= 0x3F;
	val |= ati_sb_cfg.clkrun_ow << 6;
	_pci_conf_writen(dev, 0x50, val, 2);

	if(ati_sb_cfg.clkrun_ctrl == 1){
		set_sbcfg_enable_bits(dev, 0x64, 1 << 15, 1 << 15);
	}

	return;
}

void sb_last_init(void)
{
	pcitag_t sm_dev;
	u16 sm_id;

	DEBUG_INFO("\n+++++++++++++++++SB LAST STAGE : start with type(0x%x), revision(0x%x)++++++++++++++++++++++++++\n", 
					get_sb600_platform(), get_sb600_revision());

	/* judge the device : liujl ??? */
	sm_dev = _pci_make_tag(0, 20, 0);
	sm_id = _pci_conf_readn(sm_dev, 0x02, 2);
	if(sm_id != SB600_DEVICE_ID){
		if(sm_id != 0x43C5){
		DEBUG_INFO("SB LAST STAGE : out with ID mismatch (0x%x) match(0x%x)\n", sm_id, SB600_DEVICE_ID);
		//while(1);
		}
	}
	DEBUG_INFO("SB LAST STAGE : ID judge 0x%x\n", sm_id);
	sb_last_post_setting();

	sb_last_c3_popup_setting();

	sb_last_c3_popup_next_setting();

	sb_last_p4_init();	
	
	sb_last_am97_init();

	sb_last_sata_unconnected_shutdown_clock();

	sb_last_cmos_init();
	
//	sb_last_sata_cache();

//	sb_last_sata_init();

	sb_last_init_pci_autoclock_run();

	sb_last_smi_service_check();

	DEBUG_INFO("---------------------------------- SB LAST STAGE DONE---------------------------------------\n");

	return;
}

/*---------------------------------------------------------------------------------*/
