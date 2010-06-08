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
 * GFX_POST :
 * 	GFX relative, spec. for Internal GFX
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

/*------------------------------------------------------------------------*/

/* sp & uma fine interleave mode size comparation array */
static u8 mc_interleave_divider_table[] = {
	1, 1,
	1, 3,
	3, 5,
	1, 7, 
	3, 1,
	5, 3,
	7, 1	
};

/* sp & uma fine interleave mode ratio */
static u8 mc_interleave_ratio_table[] = {
	0xAA,	// 1 : 1 (SP : UMA)
   	0x77, 	// 1 : 3
	0x1F, 	// 3 : 5
	0x7F, 	// 1 : 7
	0x11, 	// 3 : 1
	0x07, 	// 5 : 3
	0x01	// 7 : 1
};

/*
 * get_interleave_chunk :
 * get proper interleave chunk size, we should consider the
 * sp and uma size comparation
 */
static u32 get_interleave_chunk(void)
{
	u8 inter_mode = ati_nb_cfg.sp_interleave_mode;
	u32 sp_val, uma_val;

	do{
		sp_val = mc_interleave_divider_table[inter_mode];
		sp_val = ati_nb_cfg.sp_interleave_size / sp_val;
		uma_val = mc_interleave_divider_table[inter_mode + 1];
		uma_val = ati_nb_cfg.uma_fb_size / uma_val;
		--ati_nb_cfg.sp_interleave_size;
	}while(!( (sp_val != 0) && (uma_val != 0)) );
	++ati_nb_cfg.sp_interleave_size;
	if(uma_val > sp_val)
		uma_val = sp_val;

	return uma_val;
}

/* sp memory timing table */
static u32	sp_mclk_param[] = {
	// 133MHz
	0x0012C478, 0x0000C478, 0x00001200,
	// 166MHz
	0x00108020, 0x00008020, 0x00001000,
	// 200MHz
	0x00004018, 0x00004018, 0x00000000,
	// 266MHz
	0x00124478, 0x00004478, 0x00001200,
	// 300MHz
	0x00104028, 0x00004028, 0x00001000,
	// 333MHz
	0x00234498, 0x00004498, 0x00002300,
	// 400MHz
	0x00000018, 0x00000018, 0x00000000,
	// 533MHz
	0x00120478, 0x00000478, 0x00001200,
	// 633MHz
	0x00230490, 0x00000490, 0x00002300
};

/*
 * sp_get_mclk
 * get the sp memory clock according to config
 */
static int sp_get_mclk(void)
{
	/* default to lowest */
	int index = 0;

#ifdef	CFG_SP_MCLK_133
	index = 0;
#elif defined (CFG_SP_MCLK_166)
	index = 1;
#elif defined (CFG_SP_MCLK_200)
	index = 2;
#elif defined (CFG_SP_MCLK_266)
	index = 3;
#elif defined (CFG_SP_MCLK_300)
	index = 4;
#elif defined (CFG_SP_MCLK_333)
	index = 5;
#elif defined (CFG_SP_MCLK_400)
	index = 6;
#elif defined (CFG_SP_MCLK_533)
	index = 7;
#elif defined (CFG_SP_MCLK_633)
	index = 8;
#endif

	return index;
}

/*--------------------------------------------------------------*/

/*
 * sp_mc_startup_table :
 * sp memory controller startup setting, after this the controller
 * should be inited ok and start active.
 */
static void sp_mc_startup_table(pcitag_t nb_dev)
{
	/* startup / restart (s2 exit, resume from standby) */
	set_nbmc_enable_bits(nb_dev, 0x91, 1 << 0, 1 << 0);
	set_nbmc_enable_bits(nb_dev, 0xA0, 1 << 29, 0 << 29);
	set_nbmc_enable_bits(nb_dev, 0xB4, ~0xffffffff, 0x3F << 0);
	set_nbmc_enable_bits(nb_dev, 0xB4, 1 << 6, 1 << 6);
	set_nbmc_enable_bits(nb_dev, 0xC3, 1 << 11, 0 << 11);
	nbmc_write_index(nb_dev, 0xA4, 0x1845671F);
	nbmc_write_index(nb_dev, 0xA5, 0x22259422);
	nbmc_write_index(nb_dev, 0xA6, 0x00000000);
	nbmc_write_index(nb_dev, 0xA7, 0x00000000);

	/* DLL power-up */
	set_nbmc_enable_bits(nb_dev, 0xC3, 1 << 8, 0 << 8);
	delay(SP_MC_DELAY);
	/* DLL RESET, lock */
	set_nbmc_enable_bits(nb_dev, 0xC3, 1 << 9, 0 << 9);
	delay(SP_MC_DELAY);
	/* memory init complete, enable clients */
	set_nbmc_enable_bits(nb_dev, 0xA0, 1 << 30, 1 << 30);
	delay(SP_MC_DELAY);
	/* release power-up control */
	set_nbmc_enable_bits(nb_dev, 0x91, 1 << 2, 1 << 2);
	delay(SP_MC_DELAY);
	/* release resume control */
	set_nbmc_enable_bits(nb_dev, 0x91, 1 << 3, 1 << 3);
	delay(SP_MC_DELAY);
	/* memory init exec. */
	set_nbmc_enable_bits(nb_dev, 0xA0, 1 << 31, 1 << 31);
	/* memory init status */
	delay(6000);	// 500 CIM_DELAY
	/* memory init nominal */
	set_nbmc_enable_bits(nb_dev, 0xA0, 1 << 31, 0 << 31);
	
	/* protocol init - MRS seq. to occur without interrupting sideport activity */
	set_nbmc_enable_bits(nb_dev, 0xA0, 1 << 29, 0 << 29);
	nbmc_write_index(nb_dev, 0xA4, 0x00000248);
	nbmc_write_index(nb_dev, 0xA5, 0x00000000);
	nbmc_write_index(nb_dev, 0xA6, 0x00000000);
	nbmc_write_index(nb_dev, 0xA7, 0x00000000);

	nbmc_write_index(nb_dev, 0xD6, 0x05555555);
	nbmc_write_index(nb_dev, 0xD7, 0x05555555);

	nbmc_write_index(nb_dev, 0x00, 0x00000000);
	nbmc_write_index(nb_dev, 0x01, 0x00000000);
	nbmc_write_index(nb_dev, 0x08, 0x003E003E);
	nbmc_write_index(nb_dev, 0x09, 0x003E003E);

	return;
}

/*
 * sp_mc_init_table :
 * sp memory controller first init table
 */
static void sp_mc_init_table(pcitag_t nb_dev)
{
	u32 val;

	nbmc_write_index(nb_dev, 0x00, 0x80030025);
	nbmc_write_index(nb_dev, 0x4E, 0x02000000);
	nbmc_write_index(nb_dev, 0x4F, 0x00020202);
	nbmc_write_index(nb_dev, 0x7B, 0x00200020);
	nbmc_write_index(nb_dev, 0x7C, 0x001FFFC7);
	nbmc_write_index(nb_dev, 0x7D, 0x00080808);
	nbmc_write_index(nb_dev, 0x7E, 0x00000004);
	nbmc_write_index(nb_dev, 0x80, 0x00000808);
	nbmc_write_index(nb_dev, 0x81, 0x00070007);
	nbmc_write_index(nb_dev, 0x82, 0x08080808);
	nbmc_write_index(nb_dev, 0x83, 0x00070007);
	nbmc_write_index(nb_dev, 0x84, 0x00070007);
	nbmc_write_index(nb_dev, 0x85, 0x00080007);
	/* UMA performance setting and address translation */
	nbmc_write_index(nb_dev, 0x86, 0x0000003D);
	nbmc_write_index(nb_dev, 0x90, 0xFFFFFFF7);
	nbmc_write_index(nb_dev, 0x92, 0x08881018);
	nbmc_write_index(nb_dev, 0x93, 0x0000BBBB);
	nbmc_write_index(nb_dev, 0x99, 0x00000000);
	/* memory relative parameters */
	nbmc_write_index(nb_dev, 0xA0, 0x20F00A6B);
	nbmc_write_index(nb_dev, 0xA1, 0x01F10004);
	nbmc_write_index(nb_dev, 0xA2, 0x74F20000);
	nbmc_write_index(nb_dev, 0xA3, 0x4AF30000);
	nbmc_write_index(nb_dev, 0xA4, 0x22224851);
	nbmc_write_index(nb_dev, 0xA5, 0x95DAD5DB);
	nbmc_write_index(nb_dev, 0xA6, 0x02040502);
	nbmc_write_index(nb_dev, 0xA7, 0x2F2E2D22);

	nbmc_write_index(nb_dev, 0xA8, 0x46366656);
	nbmc_write_index(nb_dev, 0xA9, 0x182A1812);
	nbmc_write_index(nb_dev, 0xAA, 0x23223332);
	nbmc_write_index(nb_dev, 0xAB, 0x20002188);
	nbmc_write_index(nb_dev, 0xAC, 0x0000000A);
	nbmc_write_index(nb_dev, 0xAD, 0x00008421);
	nbmc_write_index(nb_dev, 0xAE, 0x84218421);
	nbmc_write_index(nb_dev, 0xAF, 0x2FC4A718);

	nbmc_write_index(nb_dev, 0xB0, 0x888000B3);
	nbmc_write_index(nb_dev, 0xB1, 0x33330044);
	nbmc_write_index(nb_dev, 0xB2, 0x02020202);
	nbmc_write_index(nb_dev, 0xB3, 0x02020202);

	nbmc_write_index(nb_dev, 0xB4, 0x05000B7F);
	nbmc_write_index(nb_dev, 0xB5, 0x04366564);
	nbmc_write_index(nb_dev, 0xB6, 0x33333333);
	nbmc_write_index(nb_dev, 0xB7, 0x44444444);
	nbmc_write_index(nb_dev, 0xB8, 0x88888888);
	nbmc_write_index(nb_dev, 0xB9, 0x88888888);
	nbmc_write_index(nb_dev, 0xBA, 0x55555555);

	nbmc_write_index(nb_dev, 0xC1, 0x00000000);
	nbmc_write_index(nb_dev, 0xC2, 0x00000000);
	nbmc_write_index(nb_dev, 0xC3, 0x00006B00);
	nbmc_write_index(nb_dev, 0xC4, 0x00000000);
	nbmc_write_index(nb_dev, 0xC5, 0x00000000);

	nbmc_write_index(nb_dev, 0xC8, 0x64F00A6B);
	nbmc_write_index(nb_dev, 0xC9, 0x01F10000);
	nbmc_write_index(nb_dev, 0xCA, 0x00F20000);
	nbmc_write_index(nb_dev, 0xCB, 0x00F30000);
	nbmc_write_index(nb_dev, 0xCC, 0x46366656);
	nbmc_write_index(nb_dev, 0xCD, 0x182A1812);
	nbmc_write_index(nb_dev, 0xCE, 0x23223332);
	nbmc_write_index(nb_dev, 0xCF, 0x20002188);

	nbmc_write_index(nb_dev, 0xD0, 0x02020202);
	nbmc_write_index(nb_dev, 0xD1, 0x02020202);
	nbmc_write_index(nb_dev, 0xD2, 0x33333333);
	nbmc_write_index(nb_dev, 0xD3, 0x44444444);
	nbmc_write_index(nb_dev, 0xD6, 0x55555555);
	nbmc_write_index(nb_dev, 0xD7, 0x55555555);
	nbmc_write_index(nb_dev, 0xD8, 0x00600060);
	nbmc_write_index(nb_dev, 0xD9, 0x00600060);

	nbmc_write_index(nb_dev, 0xE0, 0x00000000);
	nbmc_write_index(nb_dev, 0xE1, 0x00000000);
	nbmc_write_index(nb_dev, 0xE8, 0x003E003E);
	nbmc_write_index(nb_dev, 0xE9, 0x003E003E);

	nbmc_write_index(nb_dev, 0x104, 0xFF000000);
	nbmc_write_index(nb_dev, 0x105, 0xF0000000);
	nbmc_write_index(nb_dev, 0x106, 0x2222FFFF);
	nbmc_write_index(nb_dev, 0x107, 0x00000106);
	nbmc_write_index(nb_dev, 0x108, 0x0F030203);

	val = nbmc_read_index(nb_dev, 0x1C);
	val &= 0xFFF00FFF;
	val |= 0x10000;	
	nbmc_write_index(nb_dev, 0xE9, val);
	
	return;
}

/*
 * gfx_sp_uma_init :
 * 	Internal GFX SP and UMA dual mode or SP-only mode init.
 *	for SP-only mode, the CFG_UMA_MODE should be disabled.
 */
static void gfx_sp_uma_init(pcitag_t nb_dev)
{
	u8 ratio, mode;
	u32 chunk, sp_no_interleave_size, uma_interleave_size;
	u32 sp_temp, uma_temp;
	u32 fb_start, fb_end;
	u32 val, temp;

	if(ati_nb_cfg.gfx_config & GFX_UMA_ENABLE){
		if(ati_nb_cfg.sp_interleave_mode == COARSE_INTERLEAVE){
			/* coarse mode ratio(just up/down), and mode */
			ratio = 0x00;
			mode = 0x0b;
		}else{
			if(ati_nb_cfg.sp_interleave_size > ati_nb_cfg.sp_fb_size){
				/* we can't use the interleave outside it has */
				ati_nb_cfg.sp_interleave_size = ati_nb_cfg.sp_fb_size;
			}
			ratio = mc_interleave_ratio_table[ati_nb_cfg.sp_interleave_mode];
			mode = 0x09;
			/* caculate the chunk for both using */
			chunk = get_interleave_chunk();
			/* caculate the interleave size used for sp and uma */
			sp_temp = mc_interleave_divider_table[ati_nb_cfg.sp_interleave_mode];
			sp_temp *= chunk;
			sp_no_interleave_size = ati_nb_cfg.sp_fb_size - sp_temp;
			uma_temp = mc_interleave_divider_table[ati_nb_cfg.sp_interleave_mode];
			uma_temp *= chunk;
			uma_interleave_size = uma_temp;
		}
	}else{
		/* if uma enable, this reg has been filled */
		nbmc_write_index(nb_dev, 0x1E, ati_nb_cfg.system_memory_tom_lo << 20);
	}
	
	/* fill the 0x100, 0x1B, 0x1C, 0x1D for sp/uma mode  */
	/* caculate the FB_START, FB_END with unit 1BYTE */
	fb_start = ati_nb_cfg.system_memory_tom_lo;
	if(ati_nb_cfg.gfx_config & GFX_UMA_ENABLE){
		fb_start = fb_start - ati_nb_cfg.uma_fb_size;
	}
	fb_start = (fb_start - ati_nb_cfg.sp_fb_size) << 20;
	fb_end = (ati_nb_cfg.system_memory_tom_lo << 20) - 1;
	/* setting local fb */
	nbmc_write_index(nb_dev, 0x100, (fb_end & 0xffff0000) | (fb_start >> 16));
	/* setting mode and INTERLEAVE_START */
	temp = fb_start >> 20;
	temp += sp_no_interleave_size;
	val = nbmc_read_index(nb_dev, 0x1C);
	val &= 0x000ffff0;
	val |= temp << 20;
	val |= mode;
	nbmc_write_index(nb_dev, 0x1C, val);
	/* setting UMA size INTERLEAVE SIZE */
	nbmc_write_index(nb_dev, 0x1B, uma_interleave_size);
	/* setting INTERLEAVE_END and ratio */
	val = nbmc_read_index(nb_dev, 0x1D);
	temp = fb_start >> 20;
	temp = temp + ati_nb_cfg.sp_fb_size + uma_interleave_size;
	temp <<= 20;
	val &= 0xfff00000;
	val |= ratio << 12;
	val |= temp;
	if(ati_nb_cfg.ext_config & EXT_DEBUG_INTERLEAVE_256BYTE){
		val |= 1 << 28;
	}
	nbmc_write_index(nb_dev, 0x1D, val);

	DEBUG_INFO("gfx_sp_uma_init : mcreg1B (0x%x)\n", nbmc_read_index(nb_dev, 0x1B));
	DEBUG_INFO("gfx_sp_uma_init : mcreg1C (0x%x)\n", nbmc_read_index(nb_dev, 0x1C));
	DEBUG_INFO("gfx_sp_uma_init : mcreg1D (0x%x)\n", nbmc_read_index(nb_dev, 0x1D));
	DEBUG_INFO("gfx_sp_uma_init : mcreg1E (0x%x)\n", nbmc_read_index(nb_dev, 0x1E));
	DEBUG_INFO("gfx_sp_uma_init : mcreg100 (0x%x)\n", nbmc_read_index(nb_dev, 0x100));
	DEBUG_INFO("gfx_sp_uma_init : TOM nb90 (0x%x)\n", _pci_conf_read(nb_dev, 0x90));

	return;
}

/*-----------------------------------------------------------------------*/

/*
 * uma_cs_config :
 * configure the ChipSelect configuration according to AMD K8 platform
 * I don't think it is useful when just setting the NB part without CPU
 * supporting???
 */
static void uma_cs_config(pcitag_t nb_dev)
{
	/* we just copy it from K8 system, maybe we should change it 
	 * according to our platform or just ignore it???
	 */
	nbmc_write_index(nb_dev, 0x63, 0);
	nbmc_write_index(nb_dev, 0x64, 0);
	nbmc_write_index(nb_dev, 0x65, 0);
	nbmc_write_index(nb_dev, 0x66, 0);
	nbmc_write_index(nb_dev, 0x67, 1);
	nbmc_write_index(nb_dev, 0x68, 0);
	nbmc_write_index(nb_dev, 0x69, 0);
	nbmc_write_index(nb_dev, 0x6a, 0);
	nbmc_write_index(nb_dev, 0x6b, 0);
	nbmc_write_index(nb_dev, 0x6c, 0);
	nbmc_write_index(nb_dev, 0x6d, 0x183fe0);
	nbmc_write_index(nb_dev, 0x6e, 0);
	nbmc_write_index(nb_dev, 0x6f, 0);
	nbmc_write_index(nb_dev, 0x70, 0);
	nbmc_write_index(nb_dev, 0x71, 0x66);
	nbmc_write_index(nb_dev, 0x72, 0x400);
}

#if	1
/*
 * gfx_uma_config_fb :
 * Configure the FB 0x100, 0x1b, 0x1c, 0x1d reg block when in UMA-only
 * mode.
 */
static void gfx_uma_config_fb(pcitag_t nb_dev)
{
	u32 uma_start, uma_end;
	u32 val, temp1, temp2;

	/* we suppose only UMA mode first */
	nbmc_write_index(nb_dev, 0x1B, 0x00000000);
	/* caculate the uma size  */
	uma_end = ati_nb_cfg.system_memory_tom_lo << 20;
	uma_start = uma_end - (ati_nb_cfg.uma_fb_size << 20);
	/* interleave start and mode */
	val = nbmc_read_index(nb_dev, 0x1C);
	val &= 0x000ffff0;
	val |= uma_end;	// primary channel is UMA now. single mode
	val |= 0x04;
	nbmc_write_index(nb_dev, 0x1C, val);
	/* interleave end and ratio */
	val = nbmc_read_index(nb_dev, 0x1D);
	val &= 0xfffff000;
	val |= uma_end >> 20; // primary channel is UMA now. single mode
	nbmc_write_index(nb_dev, 0x1D, val);

	/* setting system memory start frame buffer address */
	nbmc_write_index(nb_dev, 0x1E, uma_start);

	/* setting TOM */
	_pci_conf_write(nb_dev, 0x90, 0x90000000);

	/* setting local frame buffer which uses internal address space */
	temp1 = uma_start >> 16;
	temp2 = uma_end - 1;
	val = (temp2 & 0xffff0000) | (temp1 & 0x0000ffff);
	nbmc_write_index(nb_dev, 0x100, val);

#if	0
/* no interleave */
nbmc_write_index(nb_dev, 0x1B, 0x00);
/*
 * |INTERLEAVE_START(31:20)|BANK_1_MAP|BANK_0_MAP|REV  | BANK_2_MAP | NUMBER_CHANNEL | PRIMARY_CHANNEL|INTERLEAVE_MODE|
 * |   31 : 20             | 19:16    | 15:12    |11:8 | 7 : 4      | 3              | 2              | 1:0	      |
 * --------------------------------------------------------------------------------------------------------------------
 */
nbmc_write_index(nb_dev, 0x1C, (0x800 << 20) | (5 << 16) | (4 << 12) | (0 << 8) | (6 << 4) | (0 << 3) | (0 << 2) | (0 << 0));
/*
 * | REV  | INTERLEAVE_PAGE_SIZE| REV   | INTERLEAVE_RATIO| INTERLEAVE_END |
 * | 31:29| 28			| 27:20 | 19 : 12	  | 11:0	   |
 * -------------------------------------------------------------------------
 */
nbmc_write_index(nb_dev, 0x1D, (0 << 29) | (0 << 28) | (0 << 20) | (0x0F << 12) | (0x800 << 0));
/*
 * K8_FB_LOCATION : start of system memory in K8 system memory space
 */ 
nbmc_write_index(nb_dev, 0x1E, 0x10000000);
/*
 * TOM0 : top of main system memory
 */
_pci_conf_write(nb_dev, 0x90, 0xF0000000);
/*
 * start/top of local fb section in the internal address space
 * | MC_FB_TOP | MC_FB_START |
 * | 31:16     | 15:0	     |
 * ---------------------------
 */
nbmc_write_index(nb_dev, 0x100, 0x7fff7e00);
#endif

	DEBUG_INFO("gfx_uma_config_fb : system_memory_tom_lo : 0x%x, uma_fb_size : 0x%x\n",
		ati_nb_cfg.system_memory_tom_lo, ati_nb_cfg.uma_fb_size);
	DEBUG_INFO("gfx_uma_config_fb : uma_start(0x%x), uma_end(0x%x)\n", uma_start, uma_end);
	DEBUG_INFO("gfx_uma_config_fb : mcreg1B (0x%x)\n", nbmc_read_index(nb_dev, 0x1B));
	DEBUG_INFO("gfx_uma_config_fb : mcreg1C (0x%x)\n", nbmc_read_index(nb_dev, 0x1C));
	DEBUG_INFO("gfx_uma_config_fb : mcreg1D (0x%x)\n", nbmc_read_index(nb_dev, 0x1D));
	DEBUG_INFO("gfx_uma_config_fb : mcreg1E (0x%x)\n", nbmc_read_index(nb_dev, 0x1E));
	DEBUG_INFO("gfx_uma_config_fb : mcreg100 (0x%x)\n", nbmc_read_index(nb_dev, 0x100));
	DEBUG_INFO("gfx_uma_config_fb : TOM nb90 (0x%x)\n", _pci_conf_read(nb_dev, 0x90));
	
	return;
}
#else
/*
 * gfx_uma_config_fb :
 * Configure the FB 0x100, 0x1b, 0x1c, 0x1d reg block when in UMA-only
 * mode.
 */
static void gfx_uma_config_fb(pcitag_t nb_dev)
{
	u32 uma_start, uma_end;
	u32 val;

	/* we suppose only UMA mode first */
	nbmc_write_index(nb_dev, 0x1B, 0x00000000);
	/* caculate the uma size  */
	uma_end = ati_nb_cfg.system_memory_tom_lo << 20;
	uma_start = uma_end - (ati_nb_cfg.uma_fb_size << 20);
	/* interleave start and mode */
	val = nbmc_read_index(nb_dev, 0x1C);
	val &= 0x000ffff0;
	val |= uma_end;
	val |= 0x04;
	nbmc_write_index(nb_dev, 0x1C, val);
	/* interleave end and ratio */
	val = nbmc_read_index(nb_dev, 0x1D);
	val &= 0xfffff000;
	val |= uma_end >> 20;
	nbmc_write_index(nb_dev, 0x1D, val);

	/* setting system memory start frame buffer address */
	nbmc_write_index(nb_dev, 0x1E, uma_start);

	/* setting TOM : so there is no need for sp setting it. */
	_pci_conf_write(nb_dev, 0x90, uma_start);

	/* setting local frame buffer which uses internal address space */
	uma_start >>= 16;
	uma_end -= 1;
	val = (uma_end & 0xffff0000) | (uma_start & 0x0000ffff);
	nbmc_write_index(nb_dev, 0x100, val);

	DEBUG_INFO("gfx_uma_config_fb : mcreg1B (0x%x)\n", nbmc_read_index(nb_dev, 0x1B));
	DEBUG_INFO("gfx_uma_config_fb : mcreg1C (0x%x)\n", nbmc_read_index(nb_dev, 0x1C));
	DEBUG_INFO("gfx_uma_config_fb : mcreg1D (0x%x)\n", nbmc_read_index(nb_dev, 0x1D));
	DEBUG_INFO("gfx_uma_config_fb : mcreg1E (0x%x)\n", nbmc_read_index(nb_dev, 0x1E));
	DEBUG_INFO("gfx_uma_config_fb : mcreg100 (0x%x)\n", nbmc_read_index(nb_dev, 0x100));
	DEBUG_INFO("gfx_uma_config_fb : TOM nb90 (0x%x)\n", _pci_conf_read(nb_dev, 0x90));
	
	return;
}
#endif

/*-----------------------------------------------------------------*/
/*
 * gfx_sp_async_mode_init :
 * init the sp async mode including clock setting
 */
static void gfx_sp_async_mode_init(pcitag_t nb_dev)
{
	u32 val;
	u8 index;

	/* memory clock 1x output clock control */
	set_nbmc_enable_bits(nb_dev, 0x74, 0x07 << 8, 1 << 8);

	/* program current control for SCL for PLL to 0% : 0x76[20:18]
	 * select memory PLL ref clock : '0'=100Mhz PCIE ref clk, '1'=CPU ref clk
	 */
	set_nbmc_enable_bits(nb_dev, 0x76, 0x07 << 18, 1 << 19);
	if(!(ati_nb_cfg.gfx_config & GFX_CLK_REF_PCIE)){
		set_nbmc_enable_bits(nb_dev, 0x76, 1 << 10, 1 << 10);
	}

	/* setting memory PLL setting for different operating freq. */
	index = sp_get_mclk();
	nbmc_write_index(nb_dev, 0x75, sp_mclk_param[index * 3 + 0]);
	nbmc_write_index(nb_dev, 0x79, sp_mclk_param[index * 3 + 1]);
	val = nbmc_read_index(nb_dev, 0x77);
	val &= 0xFFF0C0FF;
	val |= sp_mclk_param[index * 3 + 2];
	nbmc_write_index(nb_dev, 0x77, val);

	/* async init table for bringing up memory */
	set_nbmc_enable_bits(nb_dev, 0x74, 1 << 0, 1 << 0);
	delay(3000);
	set_nbmc_enable_bits(nb_dev, 0x74, 1 << 1, 1 << 1);
	set_nbmc_enable_bits(nb_dev, 0x74, 1 << 0, 0 << 0);
	set_nbmc_enable_bits(nb_dev, 0x7E, 1 << 22, 0 << 22);

	return;
}

/*
 * gfx_sp_init :
 * Internal GFX SP init routine
 */
static void gfx_sp_init(pcitag_t nb_dev)
{
	/* configure the parameters for SP memory */
	sp_mc_init_table(nb_dev);

	/* NB CHa termination off */
	if(ati_nb_cfg.sp_dbg_config & SP_DBG_DISABLE_NB_TERMINATION){
		set_nbmc_enable_bits(nb_dev, 0xB1, 0x77770000, 0x00000000);
	}

	/* disable ODT for MC memory CHa */
	if(ati_nb_cfg.sp_dbg_config & SP_DBG_DISABLE_ODT){
		set_nbmc_enable_bits(nb_dev, 0xB4, 1 << 8, 0 << 8);
	}

	/* user defined mc timing init if exist */
	if(ati_nb_cfg.sp_mc_init_table_ptr){
		/* add user defined mc init here */
		//sp_user_mc_init(nb_dev);
	}

	/* mc CHa 2T enable */
	if(ati_nb_cfg.sp_dbg_config & SP_DBG_2T_ENABLE){
		set_nbmc_enable_bits(nb_dev, 0xB0, 1 << 9, 1 << 9);
	}

	/* init the mode for SP-only or SP & UMA dual mode */
	gfx_sp_uma_init(nb_dev);

	/* active the controller */
	sp_mc_startup_table(nb_dev);

	/* init power option according to configuration */
	if(ati_nb_cfg.sp_dbg_config & (SP_DBG_DYNAMIC_CKE_ENABLE 
		| SP_DBG_DYNAMIC_CMD_ENABLE 
		| SP_DBG_DYNAMIC_CLK_ENABLE
	    | SP_DBG_DYNAMIC_SELFREF_ENABLE)){
		set_nbmc_enable_bits(nb_dev, 0xB4, 1 << 7, 1 << 7);
	}
	if(ati_nb_cfg.sp_dbg_config & (SP_DBG_DYNAMIC_CMD_ENABLE
		| SP_DBG_DYNAMIC_CLK_ENABLE
		| SP_DBG_DYNAMIC_SELFREF_ENABLE)){
		set_nbmc_enable_bits(nb_dev, 0xC3, 1 << 1, 1 << 1);
	}
	if(ati_nb_cfg.sp_dbg_config & (SP_DBG_DYNAMIC_CLK_ENABLE
		| SP_DBG_DYNAMIC_SELFREF_ENABLE)){
		set_nbmc_enable_bits(nb_dev, 0xC3, (1 << 2 | 1 << 3), (1 << 2 | 1 << 3));
	}
	if(ati_nb_cfg.sp_dbg_config & (SP_DBG_DYNAMIC_SELFREF_ENABLE)){
		set_nbmc_enable_bits(nb_dev, 0xC3, 1 << 4, 1 << 4);
		set_nbmc_enable_bits(nb_dev, 0x91, 1 << 7, 1 << 7);
	}

	return;
}

/*
 * gfx_uma_init :
 * GFX uma only mode init
 */
static void gfx_uma_init(pcitag_t nb_dev)
{
	DEBUG_INFO("gfx_uma_init entering.\n");
	/* cs config, copy from K8 platform */
	uma_cs_config(nb_dev);
	DEBUG_INFO("gfx_uma_init uma_cs_config copy from K8 ???.\n");

	/* AMD cpu optimization */
//	uma_k8_init(nb_dev);

	/* UMA fb size and location */
	gfx_uma_config_fb(nb_dev);
	DEBUG_INFO("gfx_uma_init ---> gfx_uma_config_fb done.\n");
}

/*-------------------------------------------------------------------*/

/*
 * rs690_gfx_init :
 * RS690 Internal GFX init 
 */
void rs690_gfx_init(pcitag_t nb_dev)
{
	/* performance length setting */
	/* enable 64/32bits mode for GFX */
	if(!(ati_nb_cfg.ext_config & EXT_DEBUG_GFX32BIT_MODE)){
		/* enable 64bytes for GFX */
		set_nbmc_enable_bits(nb_dev, 0x5F, 1 << 9, 1 << 9);
		/* large GFX burst length */
		set_nbmc_enable_bits(nb_dev, 0xB0, 1 << 8, 1 << 8);
	}
	
	/* channel performance setting */
	/* init write-combine latency to a non-zero value */
	set_nbmc_enable_bits(nb_dev, 0x5F, 0x1F << 10, 1 << 11);
	/* enable dedicate write path and make the wack available, and make the setting
	 * available now
	 */
	nbmc_write_index(nb_dev, 0x86, 0x3D);
	/* according to CIM, clear it, conflict with the reg define */
	set_htiu_enable_bits(nb_dev, 0x07, 1 << 7, 0 << 7);

	/* configure the MCLK mode */
	if(ati_nb_cfg.gfx_config & GFX_ASYNC_MODE){
		/* init async mode of timing and make it up */
		gfx_sp_async_mode_init(nb_dev);
	}else{
		/* power down memory PLL */
		set_nbmc_enable_bits(nb_dev, 0x74, 1 << 31, 1 << 31);
	}

	/* configure the UMA */
#ifdef	CFG_UMA_SUPPORT
	if(ati_nb_cfg.gfx_config & GFX_UMA_ENABLE){
		gfx_uma_init(nb_dev);
		if(!(ati_nb_cfg.gfx_config & GFX_SP_ENABLE)){
			/* start mc controller */
			set_nbmc_enable_bits(nb_dev, 0x00, 1 << 31, 1 << 31);
			set_nbmc_enable_bits(nb_dev, 0x91, ~0xffffffff, 5 << 0);
			set_nbmc_enable_bits(nb_dev, 0xB1, 1 << 6, 1 << 6);
			set_nbmc_enable_bits(nb_dev, 0xC3, 1 << 0, 1 << 0);
		}
	}
#endif

#ifdef	CFG_SP_SUPPORT
	if(ati_nb_cfg.gfx_config & GFX_SP_ENABLE){
		gfx_sp_init(nb_dev);
	}
#endif

	return;
}

/*--------------------------------------------------------------------*/

/*
 * gfx_voltage_init :
 * adjust the GFX external voltage throught PWM
 */
static void gfx_voltage_init(pcitag_t nb_dev)
{
#if	(CFG_MAX_NB_VOLTAGE || CFG_MIN_NB_VOLTAGE)
	pcitag_t igfx_dev = _pci_make_tag(1, 5, 0);
	pcitag_t clk_dev = _pci_make_tag(0, 0, 1);
	u32 val;

	if(_pci_conf_read(igfx_dev, 0x04) & 0x02){
		/* visible CLK FUNC */
		set_nbcfg_enable_bits(nb_dev, 0x4c, 1 << 0, 1 << 0);

		/* setting clock */
		val = _pci_conf_read(clk_dev, 0xCC);
		val &= ~0x027fffff;
		_pci_conf_write(clk_dev, 0xCC, val);

		/* hide CLK FUNC */
		set_nbcfg_enable_bits(nb_dev, 0x4c, 1 << 0, 0 << 0);

		/* setting nb pwm */
		if(_pci_conf_read(igfx_dev, 0x04) & 0x02){
			gfx_clkind_write(igfx_dev, 0x66, 0x00001000);
			gfx_clkind_write(igfx_dev, 0x4A, 0x000A70A7);
			gfx_clkind_write(igfx_dev, 0x4B, 0x000A7000);
			val = gfx_clkind_read(igfx_dev, 0x4C);
			val &= 0x0000f000;
			val |= 0x00000005;
			gfx_clkind_write(igfx_dev, 0x4C, val);
			val = gfx_clkind_read(igfx_dev, 0x51);
			if(CFG_NUM_CYCLE_IN_PERIOD == 0x800){
				val |= 1 << 25;
			}
			val |= (CFG_NUM_CYCLE_IN_PERIOD & 0x07ff) << 12;
			val |= (CFG_MAX_NB_VOLTAGE & 0x0fff);
			val |= 1 << 24;
			gfx_clkind_write(igfx_dev, 0x51, val);

			val = gfx_clkind_read(igfx_dev, 0x3A);
			val |= 1 << 31 | 1 << 27;
			gfx_clkind_write(igfx_dev, 0x3A, val);
		}
	}
#endif
}

/*
 * mclk_translate_table :
 * memory clock supported table, 100KHz unit, only for SP
 */
static u32 mclk_translate_table [] = {
	133 * 100,			// 10KHz unit
	166 * 100,
	200 * 100,
	266 * 100,
	300 * 100,
	333 * 100,
	400 * 100
};

void gfx_set_amdmc_clock(struct ati_integrated_system_info *info)
{
	info->k8_memory_clock = 200;
}

void gfx_set_amdmc_param(struct ati_integrated_system_info *info)
{
	info->k8_data_return_time = 286;
	info->k8_sync_start_delay = 522;
}

void gfx_set_amdmc_fsb_freq(struct ati_integrated_system_info *info)
{
	info->fsb = 200;
}

void gfx_set_amdht_width(struct ati_integrated_system_info *info)
{
	info->ht_link_width = 8;
}

/*
 * rs690_gfx_init_vuma_info :
 * make preparation for GFX VUMA information
 */
static void gfx_init_vuma_info(pcitag_t nb_dev)
{
	struct ati_integrated_system_info *info = ati_nb_cfg.vuma_rev3;
	u32 val;

	/* setting K8 MC regs */
	gfx_set_amdmc_clock(info);
	gfx_set_amdmc_param(info);
	gfx_set_amdmc_fsb_freq(info);
	gfx_set_amdht_width(info);	

	/* storing the MISC37 to struct */
	info->pcie_nbcfg_reg7 = nbmisc_read_index(nb_dev, 0x37);
	
	/* report engine clock */
	if(ati_nb_cfg.eclk == 0){
		ati_nb_cfg.eclk = CFG_GFX_BOOTUP_ENGINE_CLOCK;
	}
	info->bootup_engine_clock = ati_nb_cfg.eclk * 100;
	
	/* report SP bootup clock */
	if(ati_nb_cfg.gfx_config & GFX_ASYNC_MODE){
		info->bootup_memory_clock = mclk_translate_table[ati_nb_cfg.mclk];
	}
	
	info->min_system_memory_clock = info->max_system_memory_clock = 0x00;
	
	/* report TMDS card presence or not */
	val = nbpcie_ind_gppsb_read_index(nb_dev, 0x01);
	if(val & (1 << 10)){
		info->cap_flag |= 1 << 2;
	}
	
	/* report dynamic clock enable to VBIOS through VUMA Get Integrate Info func */
	if(ati_nb_cfg.gfx_config & GFX_TMDS_PHY_MODE){
		info->cap_flag |= 1 << 4;
	}
	if(ati_nb_cfg.gfx_config & GFX_CLOCK_GATING){
		info->cap_flag |= 1 << 1;
	}
	
	/* PWM4 : HPD2 workaround, if PWM4 enable report it to VBIOS to disable HPD2 */
	set_nbcfg_enable_bits(nb_dev, 0x4C, 1 << 0, 1 << 0);
	info->min_nb_voltage = 0;
	if(_pci_conf_read(_pci_make_tag(0, 0, 1), 0x4C) & (3 << 24)){
		info->min_nb_voltage = 1;
	}
	set_nbcfg_enable_bits(nb_dev, 0x4C, 1 << 0, 0 << 0);
	
	/* fields setting */
	info->memory_type = 0x20;		//DDR2
	// pwm voltage control
	info->num_of_cycles_in_period_hi = CFG_NUM_CYCLE_IN_PERIOD >> 8;
	info->num_of_cycles_in_period = CFG_NUM_CYCLE_IN_PERIOD & 0xff;
	info->start_pwm_high_time = CFG_START_PWM_HIGH_TIME;
	// nb voltage
	info->max_nb_voltage_high = CFG_MAX_NB_VOLTAGE >> 8;
	info->max_nb_voltage = CFG_MAX_NB_VOLTAGE & 0x00ff;
	info->min_nb_voltage_high = CFG_MIN_NB_VOLTAGE >> 8;
	info->min_nb_voltage = CFG_MIN_NB_VOLTAGE & 0x00ff;
	info->inter_nb_voltage_low = CFG_INTER_NB_VOLTAGE_LOW;
	info->inter_nb_voltage_high = CFG_INTER_NB_VOLTAGE_HIGH;
	// others
	info->reserved3 = CFG_SYS_INFO_RESERVED;
	info->major_ver = MAJOR_REVISION;
	info->minor_ver = MINOR_REVISION;
	info->struct_size = sizeof(struct ati_integrated_system_info);
	
	return;
} 

/*-------------------------------------------------------------------*/

/*
 * gfx_post_init :
 * GFX init, spec. for IGFX, uam/sp mode setting etc.
 * */
void gfx_post_init(void)
{
	pcitag_t nb_dev = _pci_make_tag(0, 0, 0);

	DEBUG_INFO("\n++++++++++++++++++++GFX POST STAGE WITH REV(%d)+++++++++++++++++++++++++++++++\n", get_nb_revision());

#if	defined(CFG_UMA_SUPPORT) || defined(CFG_SP_SUPPORT)
	if(ati_nb_cfg.gfx_config & (GFX_UMA_ENABLE | GFX_SP_ENABLE)){
		/* init rs690 GFX devices properly */
		rs690_gfx_init(nb_dev);
		/* init the integrated_system struct according to config */
		gfx_init_vuma_info(nb_dev);
		/* gfx voltage config */
		gfx_voltage_init(nb_dev);
	}
#endif

	DEBUG_INFO("-----------------------------------GFX POST STAGE DONE -------------------------------------\n");
	return;
}

/*------------------------------------------------------------------------*/

/*
 * rs690_gfx_pci_init :
 * equal to RS690 GFX pci emulation setup procedure.
 */
void gfx_pci_init(void)
{
	pcitag_t igfx_dev = _pci_make_tag(1, 5, 0);

	/* enable GFX device memory access */
	if( (ati_nb_cfg.gfx_config & (GFX_UMA_ENABLE | GFX_SP_ENABLE)) 
		&& (ati_nb_cfg.ext_config & EXT_GFX_SVIEW_ENABLE) ){
		set_nbcfg_enable_bits(igfx_dev, 0x04, 1 << 1, 1 << 1);
	}
	return;
}

/*---------------------------------------------------------------*/

