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

/*----------------------------------------------------------------------*/

/*
 * get_nbsb_link_width :
 * get NB SB pcie link width
 */
u8 get_nbsb_link_width(void)
{
	u32 val;
	u8 space = AX_INDXP;

	PRINTF_DEBUG();
	/* read axindc to tmp */
	linux_outl(space << 30 | space << 3 | 0x30, AB_INDX);
	linux_outl(0xA2, AB_DATA);
	linux_outl(space << 30 | space << 3 | 0x34, AB_INDX);
	val = linux_inl(AB_DATA);
	DEBUG_INFO("++++++++++++++++ nbsb_link_width val is %x\n", val);
	return ((val & 0xF0) >> 4);
}

/*--------------------------------------------------------------*/

/*
 *  sb_post_init
 *	sb600 init after pci init 
 */
void sb_post_init(void)
{
    pcitag_t sm_dev;
	u16 id;

	DEBUG_INFO("\n++++++++++++++++++++SB POST STAGE : start with type(0x%x), revision(0x%x)++++++++++++++++++++++++++\n", get_sb600_platform(), get_sb600_revision());

	PRINTF_DEBUG();
    /* judge the device */
    sm_dev = _pci_make_tag(0, 20, 0);

	id = _pci_conf_readn(sm_dev, 0x02, 2);
    if( (id != SB600_DEVICE_ID) && (id != 0x43c5) ){
		DEBUG_INFO("sb600 id : match 0x%x, mismatch 0x%x\n", SB600_DEVICE_ID, _pci_conf_readn(sm_dev, 0x02, 2));
        return;
    }
	DEBUG_INFO("smdev id : 0x%x\n", _pci_conf_read(sm_dev, 0x00));

#if 1
	/* wgl test */
	/* sata channel reset */
	sb_sata_channel_reset();
	/* azalia config */
	sb_azalia_post_cfg();
#endif

#ifndef UNSUPPORT_AC97
	/* ac97 config */
//	sb_detect_audiocodec();
//	/* am97 config */
//	sb_detect_modemcodec();
#endif

	/* disable sec ide */
	sb_disable_sec_ide();
#if 1
	/* wgl test */
	/* config usb */
	sb_usb_post_init();
#endif
	/* config hpet */
	sb_hpet_cfg();
	/* config hwm */
	sb_hwm_cfg();
	
	DEBUG_INFO("\n--------------------------------- SB POST STAGE DONE---------------------------------\n");
	return;
}

/*---------------------------------------------------------------------------------*/
