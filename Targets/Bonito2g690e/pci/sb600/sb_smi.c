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


extern struct ati_sb_smi atiSbSmiPostTbl[];
extern struct ati_sb_smi atiSbSmiRunTbl[];

int disable_kb_rst_smi(void)
{
	pcitag_t sm_dev = _pci_make_tag(0, 20, 0);;

	/* Disable GEvent1 to generate SMI# */	
	set_pm_enable_bits(0x30, 1 << 2, 0);

	/* Timing from STP_GNT# to Sleep = 64 Alink clocks*/
	set_pm_enable_bits(0x53, 1 << 0, 0);

	/* GEvent1 is used as KBRST# input (Dev 14h,func 0,reg 64h, bit 9)*/
	set_sbcfg_enable_bits(sm_dev, 0x64, 1 << 9, 0);
	return 1;
}

int enable_kb_rst_smi(void)
{
	u8 type = get_sb600_platform();

	/*enable keyboard reset only for Intel systerm*/
	if(type == K8_ID){
		disable_kb_rst_smi();
		return 1; // set flag
	} else {	
		/*not needed for us*/;	
		return 0; // clear flag
	}
}

int atiKbRstWaCheck(void)
{
	u8 type = get_sb600_platform();

	PRINTF_DEBUG();
	/*enable keyboard reset only if not AMD systerm*/
	if(type == K8_ID)
		return disable_kb_rst_smi();
	else	
		return enable_kb_rst_smi();	
}

/*the same as disable_kb_rst_smi*/
void disable_kb_rst_trap_smi(void)
{
	pcitag_t sm_dev = _pci_make_tag(0, 20, 0);;

	/* Disable GEvent1 to generate SMI# */	
	set_pm_enable_bits(0x30, 1 << 2, 0);

	/* Timing from STP_GNT# to Sleep = 64 Alink clocks*/
	set_pm_enable_bits(0x53, 1 << 0, 0);

	/* GEvent1 is used as KBRST# input (Dev 14h,func 0,reg 64h, bit 9)*/
	set_sbcfg_enable_bits(sm_dev, 0x64, 1 << 9, 0);
}

void soft_pci_reset_smi(void)
{
	/* reset system through 0xCF9 reset port.*/	
	linux_outb(6, 0xcf9);
}

void atiKbRstWa(void)
{
	set_pm_enable_bits(0x7c, 1 << 5, 1 << 5);
	set_pm_enable_bits(0x65, 1 << 4, 1 << 4);
	set_pm_enable_bits(0x39, 1 << 1, 0);
	disable_kb_rst_trap_smi();
	soft_pci_reset_smi();
}

#if P92_TRAP_ON_PIO < 8
int atiP92WaCheck(void)
{
	pcitag_t sm_dev = _pci_make_tag(0, 20, 0);;
	u8 type = get_sb600_platform();

	PRINTF_DEBUG();
	DEBUG_INFO("p92_TRAP_BASE is %x\n", P92_TRAP_BASE);
	DEBUG_INFO("p92_TRAP_EN_REG is %x\n", P92_TRAP_EN_REG);
	/*enable keyboard reset only if not AMD systerm*/
	if(type == K8_ID){
		/* enable Port 92 trap on ProgramIoX*/	
		set_pm_enable_bits(P92_TRAP_BASE, 0xff, 0x92);
		set_pm_enable_bits(P92_TRAP_BASE + 1, 0xff, 0);
		set_pm_enable_bits(P92_TRAP_EN_REG, P92_TRAP_EN_BIT, P92_TRAP_EN_BIT);
		/*disable port 92 decoding*/
		set_sbcfg_enable_bits(sm_dev, 0x78, 1 << 14, 0);	
		/* set ACPI flag for port 92 trap*/
		ati_sb_cfg.p92t = P92_TRAP_ON_PIO;
		return 0; 
	} else	
		return 1;	
}

void atiP92Wa(void)
{
	/*i dont know what happen, but only intel cpu come here*/
}
#endif

int atiSbirqSmiCheck(void)
{
	u8 type = get_sb600_platform();

	PRINTF_DEBUG();
	if(type == K8_ID){
		set_pm_enable_bits(0x3, 1, 1);
		set_pm_enable_bits(0x6, 1, 1);
		return 0;
	}
	return 1;
}

void atiSbirqSmi(void)
{
	u32 base, temp;

	pcitag_t sm_dev = _pci_make_tag(0, 20, 0);;

	base = _pci_conf_read(sm_dev, 0x74) & 0xffffffe0;
	base |= 0xa0000000;

	*(volatile u32 *)base = 0x10;
	temp = *(volatile u32 *)(base + 0x10);
	if (temp & 0x100) { // entry0 enable ? 
		*(volatile u32 *)base = 0x14;
		temp = *(volatile u32 *)(base + 0x14);

		if (temp & 0x100) { // entryr2 enable ? 
			*(volatile u32 *)base = 0x20;
			temp = *(volatile u32 *)(base + 0x20);

			if (temp & 0x100) { // entryr8 enable ? 
				/*flip-flop PIC interrupt maskbits */
				u8	reg21;

				reg21 = linux_inb(0x21);
				linux_outb(reg21 | 1, 0x21);
				linux_outb(reg21, 0x21);
				return ;
			}
		}
	}

	set_pm_enable_bits(0x03, 1, 0);
	return ;
}


void sb_last_smi_service_check(void)
{
	int i = 0;
	struct ati_sb_smi *p = atiSbSmiPostTbl;
	struct ati_sb_smi *q = atiSbSmiRunTbl;

	PRINTF_DEBUG();
	while(1){
		if(p->service_routine == ~0)	
			return;
		if((u32)p->check_routine == ~0) {
			p++;
			continue;	
		}
		if(p->check_routine() == 0)
			return;	
		memcpy(q, q, sizeof(struct ati_sb_smi));	// copy a struct
		i++;
		if(i >= (RUNTBL_SIZE - 1))
			return;
		p++;
		q++;
	}
}


