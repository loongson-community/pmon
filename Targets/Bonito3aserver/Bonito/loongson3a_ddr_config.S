/*
 * Copyright 2009 Lemote, Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE COPYRIGHT HOLDER(S) OR AUTHOR(S) BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 * Authors: wanghd <wanghuandong@ict.ac.cn>
 * Date : 2009-12-01
 *
 */

/*---------------------------------------------------------------------*/

#define REG_ADDRESS 0x0
#define CONFIG_BASE 0x900000000ff00000
    
/********************************************** MC0 *******************************************************/
		.global ddr2_config
		.ent    ddr2_config
		.set    noreorder
		.set    mips3
ddr2_config:
		/* 152 registers and with base address a0(0x900000000ff00000)
		 * t0 : parameters for setting mc0
		 * t1 : parameters number
		 * t2 : memory controller mc0 registers base addr
		 */
		la      t0, ddr2_reg_data
		addu    t0, t0, s0
		li      t1, 152 ##0x72
		#dli      t2, CONFIG_BASE
		#move    t2, a0         #old:li      t2, CONFIG_BASE
		daddiu	t2, a0, 0x0
reg_write:
		ld      a1, 0x0(t0)
		sd      a1, REG_ADDRESS(t2)

		subu    t1, t1, 0x1
		addiu   t0, t0, 0x8
		bne     t1, $0, reg_write
		daddiu   t2, t2, 0x10
		// waiting    
		li      t2, 0xfff
1:
		bnez    t2, 1b
		addi    t2, t2, -1
		nop
		sync

		############start##########
		daddiu	t2, a0, 0x0
		la		t0,MC0_CTL_start_DATA_LO
		addu    	t0, t0, s0
		ld      a1, 0x0(t0)
		sd      a1, 0x30(t2)

		############wait with int status############
		daddiu  t2, a0, 0x960
1:
		ld      a1, 0x0(t2)
		andi    a1, a1, 0x100
		beqz    a1, 1b
		nop
		
		############ set to srefresh##############
		daddiu  t2, a0, 0x30
		dli     a1, 0x0000000100000000
		ld      t0, 0x0(t2)
		or      a1, a1, t0
		sd      a1, 0x0(t2)

		li      t2, 0xfff
1:
		bnez    t2, 1b
		addi    t2, t2, -1
		nop
		sync
		
		############# out of srefresh##############
		daddiu  t2, a0, 0x30
		dli     a1, 0xffffff00ffffffff
		ld      t0, 0x0(t2)
		and     a1, a1, t0
		sd      a1, 0x0(t2)

		li      t2, 0xfff
1:
		bnez    t2, 1b
		addi    t2, t2, -1
		nop
		sync

		############ write mode regs##################
		daddiu  t2, a0, 0x40
		dli     a1, 0x0000000001000000
		ld      t0, 0x0(t2)
		or      a1, a1, t0
		sd      a1, 0x0(t2)

		jr      ra
		nop
		.end    ddr2_config

/********************************************** MC1 *******************************************************/
		.global ddr2_config_mc1
		.ent    ddr2_config_mc1
		.set    noreorder
		.set    mips3
ddr2_config_mc1:
		/* 152 registers and with base address a0(0x900000000ff00000)
		 * t0 : parameters for setting mc0
		 * t1 : parameters number
		 * t2 : memory controller mc0 registers base addr
		 */
		la      t0, ddr2_reg_data_mc1
		addu    t0, t0, s0
		li      t1, 152 ##0x72
		#dli      t2, CONFIG_BASE
		#move    t2, a0         #old:li      t2, CONFIG_BASE
		daddiu	t2, a0, 0x0
reg_write_mc1:
		ld      a1, 0x0(t0)
		sd      a1, REG_ADDRESS(t2)
		
		subu    t1, t1, 0x1
		addiu   t0, t0, 0x8
		bne     t1, $0, reg_write_mc1
		daddiu   t2, t2, 0x10
    
		li      t2, 0xfff
1:
		bnez    t2, 1b
		addi    t2, t2, -1
		nop
		sync

		############start##########
		daddiu	t2, a0, 0x0
		la		t0,MC1_CTL_start_DATA_LO
		addu    	t0, t0, s0
		ld      a1, 0x0(t0)
		sd      a1, 0x30(t2)

		############wait with int status############
		daddiu  t2, a0, 0x960
1:
		ld      a1, 0x0(t2)
		andi    a1, a1, 0x100
		beqz    a1, 1b
		nop
		
		############ set to srefresh##############
		daddiu  t2, a0, 0x30
		dli     a1, 0x0000000100000000
		ld      t0, 0x0(t2)
		or      a1, a1, t0
		sd      a1, 0x0(t2)

		li      t2, 0xfff
1:
		bnez    t2, 1b
		addi    t2, t2, -1
		nop
		sync
		
		############# out of srefresh##############
		daddiu  t2, a0, 0x30
		dli     a1, 0xffffff00ffffffff
		ld      t0, 0x0(t2)
		and     a1, a1, t0
		sd      a1, 0x0(t2)

		li      t2, 0xfff
1:
		bnez    t2, 1b
		addi    t2, t2, -1
		nop
		sync

		############ write mode regs##################
		daddiu  t2, a0, 0x40
		dli     a1, 0x0000000001000000
		ld      t0, 0x0(t2)
		or      a1, a1, t0
		sd      a1, 0x0(t2)

		jr      ra
		nop
		
		.end    ddr2_config_mc1


	.data
	.align	5
data_aaa0:
        .dword 0x0

#include "ddr2config.S"
#include "ddr2config_mc1.S"

#if 0
	.rdata
	.align 5
    .global ddr2_reg_data
ddr2_reg_data:
MC0_CTL_000 : .dword 0x0000000000000101
//000000000000000_0 concurrentap(RW) 0000000_1 bank_split_en(RW) 0000000_0 auto_refresh_mode(RW) 0000000_0 arefresh(WR) 0000000_0 ap(RW) 0000000_1 addr_cmp_en(RW) 0000000_1 active_aging(RW) 
MC0_CTL_010 : .dword 0x0001000000010000
//0000000_0 fwc(WR) 0000000_0 fast_write(RW) 0000000_1 enable_quick_srefresh(RW) 0000000_1 eight_bank_mode(RW) 0000000_0 ecc_disable_w_uc_err(RW) 0000000_1 dqs_n_en(RW) 0000000_0 dll_bypass_mode(RW) 0000000_0 dlllockreg(RD) 
MC0_CTL_020 : .dword 0x0100010101000000
//0000000_1 priority_en(RW) 0000000_0 power_down(RW) 0000000_1 placement_en(RW) 0000000_1 odt_add_turn_clk_en(RW) 0000000_0 no_cmd_init(RW) 0000000_0 intrptwritea(RW) 0000000_0 intrptreada(RW) 0000000_0 intrptapburst(RW) 
MC0_CTL_030 : .dword 0x0101000001000000
//0000000_1 swap_port_rw_same_en(RW) 0000000_1 swap_en(RW) 0000000_0 start(RW) 0000000_0 srefresh(RW+) 0000000_1 rw_same_en(RW) 0000000_0 reg_dimm_enable(RW) 0000000_0 reduc(RW) 0000000_0 pwrup_srefresh_exit(RW) 
MC0_CTL_040 : .dword 0x0100010100010101
//000000_01 rtt_0(RW) 000000_11 ctrl_raw(RW) 000000_01 axi0_w_priority(RW) 000000_01 axi0_r_priority(RW) 0000000_0 write_modereg(WR) 0000000_1 writeinterp(RW) 0000000_1 tref_enable(RW) 0000000_1 tras_lockout(RW) 
MC0_CTL_050 : .dword 0x0000000404050100
//00000_000 q_fullness(RW) 00000_000 port_data_error_type(RD) 000_00000 out_of_range_type(RD) 00000_000 max_cs_reg(RD) 00000_100 column_size(RW) 0000_0101 caslat(RW) 00000_010 addr_pins(RW) 00000000
MC0_CTL_060 : .dword 0x0a04040603040003
//0000_1010 aprebit(RW) 0000_0101 wrlat(RW) 0000_0100 twtr(RW) 000_00110 twr_int(RW) 00000_011 trtp(RW) 00000_100 trrd(RW) 0000000000000_011 tcke(RW) 
MC0_CTL_070 : .dword 0x0f0e020000010a08
//0000_0000 max_row_reg(RD) 0000_0000 max_col_reg(RD) 0000_0000 initaref(RW) 00000000000000000000_1111 cs_map(RW) 000_01010 caslat_lin_gate(RW) 000_01010 caslat_lin(RW) 
MC0_CTL_080 : .dword 0x0104040101000400
//0000_0001 odt_wr_map_cs3(RW) 0000_0010 odt_wr_map_cs2(RW) 0000_0100 odt_wr_map_cs1(RW) 0000_1000 odt_wr_map_cs0(RW) 0000_0001 odt_rd_map_cs3(RW) 0000_0010 odt_rd_map_cs2(RW) 0000_0100 odt_rd_map_cs1(RW) 0000_1000 odt_rd_map_cs0(RW) 
MC0_CTL_090 : .dword 0x0000050b00000000
//000_00000 ocd_adjust_pup_cs_0(RW) 000_00000 ocd_adjust_pdn_cs_0(RW) 0000_0101 trp(RW) 000_01011 tdal(RW) 000000000000_0000 port_cmd_error_type(RD) 0000000000000000
MC0_CTL_0a0 : .dword 0x0000003f3f140612
//MC0_CTL_0a0 : .dword 0x0000003f3f14021b
//00000000000000000000000000_111111 command_age_count(RW) 00_111111 age_count(RW) 000_10100 trc(RW) 000_00110 tmrd(RW) 000_10010 tfaw(RW) 
MC0_CTL_0b0 : .dword 0x0000000000000000
MC0_CTL_0c0 : .dword 0x00002c050f000000
//000000000000000000101100 trfc(RW) 00000101 trcd_int(RW) 00001111 tras_min(RW) 00000000 out_of_range_length(RD) 00000000 ecc_u_synd(RD) 00000000 ecc_c_synd(RD) 
MC0_CTL_0d0 : .dword 0x0000000000000000
MC0_CTL_0e0 : .dword 0x0000000000000000
MC0_CTL_0f0 : .dword 0x0000000000000000
MC0_CTL_100 : .dword 0x0000000000000000
MC0_CTL_110 : .dword 0x0000000000001c2d
//0_000000000000000 emrs2_data_1(RW) 0_000000000000000 emrs2_data_0(RW) 000000000000000000_00110000101101 tref(RW) 
MC0_CTL_120 : .dword 0xffff000000000000
//0000000000011100 axi0_en_size_lt_width_instr(RW) 00000000000000000_000000000000000 emrs2_data_3(RW) 0_000000000000000 emrs2_data_2(RW) 
MC0_CTL_130 : .dword 0x0d56000302000000
//0110110101010110 tras_max(RW) 0000000000000011 tpdex(RW) 0000001000000000 tdll(RW) 0000000000000000 tcpd(RW) 
MC0_CTL_140 : .dword 0x0000204002000030
//0000000000000000 xor_check_bits(RW) 0000000000000000 version(RD) 0000001000000000 txsr(RW) 0000000000110000 txsnr(RW) 
MC0_CTL_150 : .dword 0x0000000011000004
//000_0000000000000000000000000000000000000 ecc_c_addr(RD) 000000000000000000011011 tinit(RW) 
MC0_CTL_160 : .dword 0x0000000000000000
//000000000000000000000000000_0000000000000000000000000000000000000 ecc_u_addr(RD) 
MC0_CTL_170 : .dword 0x0000000000000000
//000000000000000000000000000_0000000000000000000000000000000000000 out_of_range_addr(RD) 
MC0_CTL_180 : .dword 0x0000000000000000
//000000000000000000000000000_0000000000000000000000000000000000000 port_cmd_error_addr(RD) 
MC0_CTL_190 : .dword 0x0000000000000000
//0000000000000000000000000000000000000000000000000000000000000000 ecc_c_data(RD) 
MC0_CTL_1a0 : .dword 0x0000000000000000
//0000000000000000000000000000000000000000000000000000000000000000 ecc_u_data(RD) 
MC0_CTL_1b0 : .dword 0x0000000000000000
//0000000000000000000000000000000000000000000000000000000000000_000 cke_delay(RW) 
MC0_CTL_1c0 : .dword 0x0000000000000000
MC0_CTL_1d0 : .dword 0x0203070400000101
//0000_0010 tdfi_phy_wrlat_base(RW) 0000_0000 tdfi_phy_wrlat(RD) 000_00111 tdfi_phy_rdlat(RW) 0000_0000 tdfi_ctrlupd_min(RD) 000000000000_0000 dram_clk_disable(RW) 0000000_1 odt_alt_en(RW) 0000000_1 drive_dq_dqs(RW) 
MC0_CTL_1e0 : .dword 0x0c2d0c2d0c2d0205
//00_00000000000000 tdfi_phyupd_type0(RD) 00_00000000000000 tdfi_phyupd_resp(RD) 00_00000000000000 tdfi_ctrlupd_max(RD) 000_00000 tdfi_rddata_en_base(RW) 000_00000 tdfi_rddata_en(RD) 
MC0_CTL_1f0 : .dword 0x00200e8000000000
MC0_CTL_200 : .dword 0x00200e8000200e80
MC0_CTL_210 : .dword 0x00200e8000200e80
//00000000001000000000111510000000 dll_ctrl_reg_0_4(RW) 00000000001000000000111010000000 dll_ctrl_reg_0_3(RW) 
MC0_CTL_220 : .dword 0x00200e8000200e80
//00000000001000000000111510000000 dll_ctrl_reg_0_6(RW) 00000000001000000000111010000000 dll_ctrl_reg_0_5(RW) 
MC0_CTL_230 : .dword 0x00200e8000200e80
MC0_CTL_240 : .dword 0x00000e0000000e00
//00000000000000000000111000000000 dll_ctrl_reg_1_1(RW) 00000000000000000000111000000000 dll_ctrl_reg_1_0(RW) 
MC0_CTL_250 : .dword 0x00000e0000000e00
MC0_CTL_260 : .dword 0x00000e0000000e00
MC0_CTL_270 : .dword 0x00000e0000000e00
MC0_CTL_280 : .dword 0x0000000000000e00
MC0_CTL_290 : .dword 0x0000000000000000
MC0_CTL_2a0 : .dword 0x0000000000000000
MC0_CTL_2b0 : .dword 0x0000000000000000
MC0_CTL_2c0 : .dword 0x0000000000000000
//MC0_CTL_2d0 : .dword 0xf300494603c0019d
MC0_CTL_2d0 : .dword 0xf300494603fc099c
//11110100000000000011101100100111 phy_ctrl_reg_0_0(RD) 000000_00000000000000000110011101 pad_ctrl_reg_0(RW) 
MC0_CTL_2e0 : .dword 0xf3004946f3004946
MC0_CTL_2f0 : .dword 0xf3004946f3004946
MC0_CTL_300 : .dword 0xf3004946f3004946
MC0_CTL_310 : .dword 0xf3004946f3004946
//MC0_CTL_2d0 : .dword 0xf3005a470000019d
////11110100000000000011101100100111 phy_ctrl_reg_0_0(RD) 000000_00000000000000000110011101 pad_ctrl_reg_0(RW) 
//MC0_CTL_2e0 : .dword 0xf3005a47f3005a47
//MC0_CTL_2f0 : .dword 0xf3005a47f3005a47
//MC0_CTL_300 : .dword 0xf3005a47f3005a47
//MC0_CTL_310 : .dword 0xf3005a47f3005a47
MC0_CTL_320 : .dword 0x06a0000006a00000
MC0_CTL_330 : .dword 0x06a0000006a00000
MC0_CTL_340 : .dword 0x06a0000006a00000
MC0_CTL_350 : .dword 0x06a0000006a00000
MC0_CTL_360 : .dword 0x0800c00506a00000
//00000000000000001100000000000101 phy_ctrl_reg_2(RW) 00000111110000000000001100000001 phy_ctrl_reg_1_8(RD) 
MC0_CTL_370 : .dword 0x0000000000000000
MC0_CTL_380 : .dword 0x0000000000000000
MC0_CTL_390 : .dword 0x0000000000000000
MC0_CTL_3a0 : .dword 0x0000000000000000
MC0_CTL_3b0 : .dword 0x0000000000000000
MC0_CTL_3c0 : .dword 0x0000000000000000
MC0_CTL_3d0 : .dword 0x0000000000000000
MC0_CTL_3e0 : .dword 0x0000000000000000
MC0_CTL_3f0 : .dword 0x0000000000000000
MC0_CTL_400 : .dword 0x0000000000000000
MC0_CTL_410 : .dword 0x0000000000000000
MC0_CTL_420 : .dword 0x0000000000000000
MC0_CTL_430 : .dword 0x0000000000000000
MC0_CTL_440 : .dword 0x0000000000000000
MC0_CTL_450 : .dword 0x0000000000000000
MC0_CTL_460 : .dword 0x0000000000000000
MC0_CTL_470 : .dword 0x0000000000000000
MC0_CTL_480 : .dword 0x0000000000000000
MC0_CTL_490 : .dword 0x0000000000000000
MC0_CTL_4a0 : .dword 0x0000000000000000
MC0_CTL_4b0 : .dword 0x0000000000000000
MC0_CTL_4c0 : .dword 0x0000000000000000
MC0_CTL_4d0 : .dword 0x0000000000000000
MC0_CTL_4e0 : .dword 0x0000000000000000
MC0_CTL_4f0 : .dword 0x0000000000000000
MC0_CTL_500 : .dword 0x0000000000000000
MC0_CTL_510 : .dword 0x0000000000000000
MC0_CTL_520 : .dword 0x0000000000000000
MC0_CTL_530 : .dword 0x0000000000000000
MC0_CTL_540 : .dword 0x0000000000000000
MC0_CTL_550 : .dword 0x0000000000000000
MC0_CTL_560 : .dword 0x0000000000000000
MC0_CTL_570 : .dword 0x0000000000000000
MC0_CTL_580 : .dword 0x0000000000000000
MC0_CTL_590 : .dword 0x0000000000000000
MC0_CTL_5a0 : .dword 0x0000000000000000
MC0_CTL_5b0 : .dword 0x0000000000000000
MC0_CTL_5c0 : .dword 0x0000000000000000
MC0_CTL_5d0 : .dword 0x0000000000000000
MC0_CTL_5e0 : .dword 0x0000000000000000
MC0_CTL_5f0 : .dword 0x0000000000000000
MC0_CTL_600 : .dword 0x0000000000000000
MC0_CTL_610 : .dword 0x0000000000000000
MC0_CTL_620 : .dword 0x0000000000000000
MC0_CTL_630 : .dword 0x0000000000000000
MC0_CTL_640 : .dword 0x0000000000000000
MC0_CTL_650 : .dword 0x0000000000000000
MC0_CTL_660 : .dword 0x0000000000000000
MC0_CTL_670 : .dword 0x0000000000000000
MC0_CTL_680 : .dword 0x0000000000000000
MC0_CTL_690 : .dword 0x0000000000000000
MC0_CTL_6a0 : .dword 0x0000000000000000
MC0_CTL_6b0 : .dword 0x0000000000000000
MC0_CTL_6c0 : .dword 0x0000000000000000
MC0_CTL_6d0 : .dword 0x0000000000000000
MC0_CTL_6e0 : .dword 0x0000000000000000
MC0_CTL_6f0 : .dword 0x0000000000000000
MC0_CTL_700 : .dword 0x0000000000000000
MC0_CTL_710 : .dword 0x0000000000000000
MC0_CTL_720 : .dword 0x0000000000000000
MC0_CTL_730 : .dword 0x0000000000000000
MC0_CTL_740 : .dword 0x0100000000000000
//MC0_CTL_750 : .dword 0x0100000101020101
MC0_CTL_750 : .dword 0x0101000101020101
//000000_01 wrlvl_cs(RW) 000000_00 sw_leveling_mode(RW) 000000_00 rdlvl_cs(RW) 000000_01 axi2_w_priority(RW) 000000_01 axi2_r_priority(RW) 000000_10 axi2_port_ordering(RW) 000000_01 axi1_w_priority(RW) 000000_01 axi1_r_priority(RW) 
MC0_CTL_760 : .dword 0x0303030000020001
//0000_0011 axi0_priority2_relative_priority(RW) 0000_0011 axi0_priority1_relative_priority(RW) 0000_0011 axi0_priority0_relative_priority(RW) 0000_0000 address_mirroring(RW) 00000_000 tdfi_dram_clk_disable(RW) 00000_010 bstlen(RW) 000000_00 zq_req(WR) 000000_01 zq_on_sref_exit(RW) 
MC0_CTL_770 : .dword 0x0101010202020203
//0000_0001 axi2_priority2_relative_priority(RW) 0000_0001 axi2_priority1_relative_priority(RW) 0000_0001 axi2_priority0_relative_priority(RW) 0000_0010 axi1_priority3_relative_priority(RW) 0000_0010 axi1_priority2_relative_priority(RW) 0000_0010 axi1_priority1_relative_priority(RW) 0000_0010 axi1_priority0_relative_priority(RW) 0000_0011 axi0_priority3_relative_priority(RW) 
MC0_CTL_780 : .dword 0x0102020400040001
//0000_0001 tdfi_dram_clk_enable(RW) 0000_0010 tdfi_ctrl_delay(RW) 0000_0010 rdlvl_gate_dq_zero_count(RW) 0000_0100 rdlvl_dq_zero_count(RW) 0000_0000 lowpower_refresh_enable(RW) 0000_0110 dram_class(RW) 0000_1100 burst_on_fly_bit(RW) 0000_0001 axi2_priority3_relative_priority(RW) 
MC0_CTL_790 : .dword 0x281900000f000303
//00_101000 wlmrd(RW) 00_011001 wldqsen(RW) 000_00000 lowpower_control(RW) 000_00000 lowpower_auto_enable(RW) 0000_1111 zqcs_chip(RW) 0000_0000 wrr_param_value_err(RD) 0000_0011 tdfi_wrlvl_dll(RW) 0000_0011 tdfi_rdlvl_dll(RW) 
MC0_CTL_7a0 : .dword 0x00000000000000ff
MC0_CTL_7b0 : .dword 0x0000000000000000
MC0_CTL_7c0 : .dword 0x0000000000000000
MC0_CTL_7d0 : .dword 0x0000000000000000
MC0_CTL_7e0 : .dword 0x0000000000000000
//00000000 rdlvl_gate_delay_2(RD) 00000000 rdlvl_gate_delay_1(RD) 00000000 rdlvl_gate_delay_0(RD) 00000000 rdlvl_gate_clk_adjust_8(RW) 00000000 rdlvl_gate_clk_adjust_7(RW) 00000000 rdlvl_gate_clk_adjust_6(RW) 00000000 rdlvl_gate_clk_adjust_5(RW) 00000000 rdlvl_gate_clk_adjust_4(RW) 
MC0_CTL_7f0 : .dword 0xff08000000000000
//11111111 rdlvl_max_delay(RW) 00001000 rdlvl_gate_max_delay(RW) 00000000 rdlvl_gate_delay_8(RD) 00000000 rdlvl_gate_delay_7(RD) 00000000 rdlvl_gate_delay_6(RD) 00000000 rdlvl_gate_delay_5(RD) 00000000 rdlvl_gate_delay_4(RD) 00000000 rdlvl_gate_delay_3(RD) 
MC0_CTL_800 : .dword 0x0000000000000000
//00000000 rdlvl_midpoint_delay_7(RD) 00000000 rdlvl_midpoint_delay_6(RD) 00000000 rdlvl_midpoint_delay_5(RD) 00000000 rdlvl_midpoint_delay_4(RD) 00000000 rdlvl_midpoint_delay_3(RD) 00000000 rdlvl_midpoint_delay_2(RD) 00000000 rdlvl_midpoint_delay_1(RD) 00000000 rdlvl_midpoint_delay_0(RD) 
MC0_CTL_810 : .dword 0x000000000000000e
//00000000 rdlvl_offset_delay_6(RW) 00000000 rdlvl_offset_delay_5(RW) 00000000 rdlvl_offset_delay_4(RW) 00000000 rdlvl_offset_delay_3(RW) 00000000 rdlvl_offset_delay_2(RW) 00000000 rdlvl_offset_delay_1(RW) 00000000 rdlvl_offset_delay_0(RW) 00000000 rdlvl_midpoint_delay_8(RD) 
MC0_CTL_820 : .dword 0x0420000c20400000
//00000100 tdfi_wrlvl_resplat(RW) 00000000 tdfi_wrlvl_resp(RD) 00000000 tdfi_rdlvl_rr(RW) 00001100 tdfi_rdlvl_resplat(RW) 00000000 tdfi_rdlvl_resp(RD) 01000000 refresh_per_zq(RW) 00000000 rdlvl_offset_delay_8(RW) 00000000 rdlvl_offset_delay_7(RW) 
MC0_CTL_830 : .dword 0x0000000000000c0a
//MC0_CTL_830 : .dword 0x1313131313130c0a
//00000000 wrlvl_delay_5(RD) 00000000 wrlvl_delay_4(RD) 00000000 wrlvl_delay_3(RD) 00000000 wrlvl_delay_2(RD) 00000000 wrlvl_delay_1(RD) 00000000 wrlvl_delay_0(RD) 00000010 tmod(RW) 00001010 tdfi_wrlvl_ww(RW) 
MC0_CTL_840 : .dword 0x0000640064000000
//MC0_CTL_840 : .dword 0x0000640064001313
//00000000000000_0001100100 axi1_priority_relax(RW) 000000_0001100100 axi0_priority_relax(RW) 00000000 wrlvl_delay_8(RD) 00000000 wrlvl_delay_7(RD) 00000000 wrlvl_delay_6(RD) 
MC0_CTL_850 : .dword 0x0000000000000064
MC0_CTL_860 : .dword 0x0200004000000000
MC0_CTL_870 : .dword 0x0046004600460046
//0_000000000000010 emrs1_data_3(RW) 0_000000000000010 emrs1_data_2(RW) 0_000000000000010 emrs1_data_1(RW) 0_000000000000010 emrs1_data_0(RW) 
MC0_CTL_880 : .dword 0x0000000000000000
MC0_CTL_890 : .dword 0x0a520a520a520a52
//0_000010000010000 mrs_data_3(RW) 0_000010000010000 mrs_data_2(RW) 0_000010000010000 mrs_data_1(RW) 0_000010000010000 mrs_data_0(RW) 
MC0_CTL_8a0 : .dword 0x00000000001c001c
MC0_CTL_8b0 : .dword 0x0000000000000000
MC0_CTL_8c0 : .dword 0x0004000000000000
MC0_CTL_8d0 : .dword 0x00000000c8000000
MC0_CTL_8e0 : .dword 0x0000000000000050
//MC0_CTL_8f0 : .dword 0x0000000020202080
//MC0_CTL_8f0 : .dword 0x000000002b352180
//MC0_CTL_8f0 : .dword 0x000000000a150080 //clk skew cleared
MC0_CTL_8f0 : .dword 0x000000002f3a2580 //clk skew cleared
//MC0_CTL_8f0 : .dword 0x0000000040404080
//0000000000000000000000000111100_000000000000000000000000001111000 dll_ctrl_reg_2(RW) 
MC0_CTL_900 : .dword 0x0000000000000000
MC0_CTL_910 : .dword 0x0000000000000000
MC0_CTL_920 : .dword 0x0000000000000000
MC0_CTL_930 : .dword 0x0000000000000000
MC0_CTL_940 : .dword 0x0300000000050500
MC0_CTL_950 : .dword 0x0000000000000a03
MC0_CTL_960 : .dword 0x0604000100000000
//000_00101 rdlat_adj(RW) 0000_0100 wrlat_adj(RW) 0000000_0 swlvl_start(WR) 0000000_0 swlvl_load(WR) 0000000_0 swlvl_exit(WR) 000000_000000000000000000 int_status(RD) 
MC0_CTL_970 : .dword 0x000000000003e805
MC0_CTL_start_DATA_LO: .word  0x00000000
//0000000_1 rw_same_en 0000000_0 reg_dimm_enable 0000000_0 reduc 0000000_0 pwrup_srefresh_exit
MC0_CTL_start_DATA_HI: .word  0x01010100
//0000000_1 swap_port_rw_same_en 0000000_1 swap_en 0000000_0 start 0000000_0 srefresh

ddr2_reg_data_mc1:
MC1_CTL_000 : .dword 0x0000000000000101
//000000000000000_0 concurrentap(RW) 0000000_1 bank_split_en(RW) 0000000_0 auto_refresh_mode(RW) 0000000_0 arefresh(WR) 0000000_0 ap(RW) 0000000_1 addr_cmp_en(RW) 0000000_1 active_aging(RW) 
MC1_CTL_010 : .dword 0x0001000100010000
//0000000_0 fwc(WR) 0000000_0 fast_write(RW) 0000000_1 enable_quick_srefresh(RW) 0000000_1 eight_bank_mode(RW) 0000000_0 ecc_disable_w_uc_err(RW) 0000000_1 dqs_n_en(RW) 0000000_0 dll_bypass_mode(RW) 0000000_0 dlllockreg(RD) 
MC1_CTL_020 : .dword 0x0100010101000000
//0000000_1 priority_en(RW) 0000000_0 power_down(RW) 0000000_1 placement_en(RW) 0000000_1 odt_add_turn_clk_en(RW) 0000000_0 no_cmd_init(RW) 0000000_0 intrptwritea(RW) 0000000_0 intrptreada(RW) 0000000_0 intrptapburst(RW) 
MC1_CTL_030 : .dword 0x0101000001000000
//0000000_1 swap_port_rw_same_en(RW) 0000000_1 swap_en(RW) 0000000_0 start(RW) 0000000_0 srefresh(RW+) 0000000_1 rw_same_en(RW) 0000000_0 reg_dimm_enable(RW) 0000000_0 reduc(RW) 0000000_0 pwrup_srefresh_exit(RW) 
MC1_CTL_040 : .dword 0x0100010200010101
//000000_01 rtt_0(RW) 000000_11 ctrl_raw(RW) 000000_01 axi0_w_priority(RW) 000000_01 axi0_r_priority(RW) 0000000_0 write_modereg(WR) 0000000_1 writeinterp(RW) 0000000_1 tref_enable(RW) 0000000_1 tras_lockout(RW) 
MC1_CTL_050 : .dword 0x0000000404050100
//00000_000 q_fullness(RW) 00000_000 port_data_error_type(RD) 000_00000 out_of_range_type(RD) 00000_000 max_cs_reg(RD) 00000_100 column_size(RW) 0000_0101 caslat(RW) 00000_010 addr_pins(RW) 00000000
MC1_CTL_060 : .dword 0x0a04040603040003
//0000_1010 aprebit(RW) 0000_0101 wrlat(RW) 0000_0100 twtr(RW) 000_00110 twr_int(RW) 00000_011 trtp(RW) 00000_100 trrd(RW) 0000000000000_011 tcke(RW) 
MC1_CTL_070 : .dword 0x0f0e020000010a08
//0000_0000 max_row_reg(RD) 0000_0000 max_col_reg(RD) 0000_0000 initaref(RW) 00000000000000000000_1111 cs_map(RW) 000_01010 caslat_lin_gate(RW) 000_01010 caslat_lin(RW) 
MC1_CTL_080 : .dword 0x0004000100000000
//0000_0001 odt_wr_map_cs3(RW) 0000_0010 odt_wr_map_cs2(RW) 0000_0100 odt_wr_map_cs1(RW) 0000_1000 odt_wr_map_cs0(RW) 0000_0001 odt_rd_map_cs3(RW) 0000_0010 odt_rd_map_cs2(RW) 0000_0100 odt_rd_map_cs1(RW) 0000_1000 odt_rd_map_cs0(RW) 
MC1_CTL_090 : .dword 0x0000050b00000000
//000_00000 ocd_adjust_pup_cs_0(RW) 000_00000 ocd_adjust_pdn_cs_0(RW) 0000_0101 trp(RW) 000_01011 tdal(RW) 000000000000_0000 port_cmd_error_type(RD) 0000000000000000
MC1_CTL_0a0 : .dword 0x0000003f3f140612
//00000000000000000000000000_111111 command_age_count(RW) 00_111111 age_count(RW) 000_10100 trc(RW) 000_00110 tmrd(RW) 000_10010 tfaw(RW) 
MC1_CTL_0b0 : .dword 0x0000000000000000
MC1_CTL_0c0 : .dword 0x00002c050f000000
//000000000000000000101100 trfc(RW) 00000101 trcd_int(RW) 00001111 tras_min(RW) 00000000 out_of_range_length(RD) 00000000 ecc_u_synd(RD) 00000000 ecc_c_synd(RD) 
MC1_CTL_0d0 : .dword 0x0000000000000000
MC1_CTL_0e0 : .dword 0x0000000000000000
MC1_CTL_0f0 : .dword 0x0000000000000000
MC1_CTL_100 : .dword 0x0000000000000000
MC1_CTL_110 : .dword 0x0000000000001c2d
//0_000000000000000 emrs2_data_1(RW) 0_000000000000000 emrs2_data_0(RW) 000000000000000000_00110000101101 tref(RW) 
MC1_CTL_120 : .dword 0xffff000000000000
//0000000000011100 axi0_en_size_lt_width_instr(RW) 00000000000000000_000000000000000 emrs2_data_3(RW) 0_000000000000000 emrs2_data_2(RW) 
MC1_CTL_130 : .dword 0x0d56000302000000
//0110110101010110 tras_max(RW) 0000000000000011 tpdex(RW) 0000001000000000 tdll(RW) 0000000000000000 tcpd(RW) 
MC1_CTL_140 : .dword 0x0000204002000030
//0000000000000000 xor_check_bits(RW) 0000000000000000 version(RD) 0000001000000000 txsr(RW) 0000000000110000 txsnr(RW) 
MC1_CTL_150 : .dword 0x0000000011000004
//000_0000000000000000000000000000000000000 ecc_c_addr(RD) 000000000000000000011011 tinit(RW) 
MC1_CTL_160 : .dword 0x0000000000000000
//000000000000000000000000000_0000000000000000000000000000000000000 ecc_u_addr(RD) 
MC1_CTL_170 : .dword 0x0000000000000000
//000000000000000000000000000_0000000000000000000000000000000000000 out_of_range_addr(RD) 
MC1_CTL_180 : .dword 0x0000000000000000
//000000000000000000000000000_0000000000000000000000000000000000000 port_cmd_error_addr(RD) 
MC1_CTL_190 : .dword 0x0000000000000000
//0000000000000000000000000000000000000000000000000000000000000000 ecc_c_data(RD) 
MC1_CTL_1a0 : .dword 0x0000000000000000
//0000000000000000000000000000000000000000000000000000000000000000 ecc_u_data(RD) 
MC1_CTL_1b0 : .dword 0x0000000000000000
//0000000000000000000000000000000000000000000000000000000000000_000 cke_delay(RW) 
MC1_CTL_1c0 : .dword 0x0000000000000000
MC1_CTL_1d0 : .dword 0x0203070400000101
//0000_0010 tdfi_phy_wrlat_base(RW) 0000_0000 tdfi_phy_wrlat(RD) 000_00111 tdfi_phy_rdlat(RW) 0000_0000 tdfi_ctrlupd_min(RD) 000000000000_0000 dram_clk_disable(RW) 0000000_1 odt_alt_en(RW) 0000000_1 drive_dq_dqs(RW) 
MC1_CTL_1e0 : .dword 0x0c2d0c2d0c2d0205
//00_00000000000000 tdfi_phyupd_type0(RD) 00_00000000000000 tdfi_phyupd_resp(RD) 00_00000000000000 tdfi_ctrlupd_max(RD) 000_00000 tdfi_rddata_en_base(RW) 000_00000 tdfi_rddata_en(RD) 
MC1_CTL_1f0 : .dword 0x00200e8000000000
MC1_CTL_200 : .dword 0x00200e8000200e80
MC1_CTL_210 : .dword 0x00200e8000200e80
//00000000001000000000111510000000 dll_ctrl_reg_0_4(RW) 00000000001000000000111010000000 dll_ctrl_reg_0_3(RW) 
MC1_CTL_220 : .dword 0x00200e8000200e80
//00000000001000000000111510000000 dll_ctrl_reg_0_6(RW) 00000000001000000000111010000000 dll_ctrl_reg_0_5(RW) 
MC1_CTL_230 : .dword 0x00200e8000200e80
MC1_CTL_240 : .dword 0x00000e0000000e00
//00000000000000000000111000000000 dll_ctrl_reg_1_1(RW) 00000000000000000000111000000000 dll_ctrl_reg_1_0(RW) 
MC1_CTL_250 : .dword 0x00000e0000000e00
MC1_CTL_260 : .dword 0x00000e0000000e00
MC1_CTL_270 : .dword 0x00000e0000000e00
MC1_CTL_280 : .dword 0x0000000000000e00
MC1_CTL_290 : .dword 0x0000000000000000
MC1_CTL_2a0 : .dword 0x0000000000000000
MC1_CTL_2b0 : .dword 0x0000000000000000
MC1_CTL_2c0 : .dword 0x0000000000000000
MC1_CTL_2d0 : .dword 0xf300494603fc099c
//11110100000000000011101100100111 phy_ctrl_reg_0_0(RD) 000000_00000000000000000110011101 pad_ctrl_reg_0(RW) 
MC1_CTL_2e0 : .dword 0xf3004946f3004946
MC1_CTL_2f0 : .dword 0xf3004946f3004946
MC1_CTL_300 : .dword 0xf3004946f3004946
MC1_CTL_310 : .dword 0xf3004946f3004946
//MC1_CTL_2d0 : .dword 0xf3005a470000019d
////11110100000000000011101100100111 phy_ctrl_reg_0_0(RD) 000000_00000000000000000110011101 pad_ctrl_reg_0(RW) 
//MC1_CTL_2e0 : .dword 0xf3005a47f3005a47
//MC1_CTL_2f0 : .dword 0xf3005a47f3005a47
//MC1_CTL_300 : .dword 0xf3005a47f3005a47
//MC1_CTL_310 : .dword 0xf3005a47f3005a47
MC1_CTL_320 : .dword 0x06a0000006a00000
MC1_CTL_330 : .dword 0x06a0000006a00000
MC1_CTL_340 : .dword 0x06a0000006a00000
MC1_CTL_350 : .dword 0x06a0000006a00000
MC1_CTL_360 : .dword 0x0800c00506a00000
//00000000000000001100000000000101 phy_ctrl_reg_2(RW) 00000111110000000000001100000001 phy_ctrl_reg_1_8(RD) 
MC1_CTL_370 : .dword 0x0000000000000000
MC1_CTL_380 : .dword 0x0000000000000000
MC1_CTL_390 : .dword 0x0000000000000000
MC1_CTL_3a0 : .dword 0x0000000000000000
MC1_CTL_3b0 : .dword 0x0000000000000000
MC1_CTL_3c0 : .dword 0x0000000000000000
MC1_CTL_3d0 : .dword 0x0000000000000000
MC1_CTL_3e0 : .dword 0x0000000000000000
MC1_CTL_3f0 : .dword 0x0000000000000000
MC1_CTL_400 : .dword 0x0000000000000000
MC1_CTL_410 : .dword 0x0000000000000000
MC1_CTL_420 : .dword 0x0000000000000000
MC1_CTL_430 : .dword 0x0000000000000000
MC1_CTL_440 : .dword 0x0000000000000000
MC1_CTL_450 : .dword 0x0000000000000000
MC1_CTL_460 : .dword 0x0000000000000000
MC1_CTL_470 : .dword 0x0000000000000000
MC1_CTL_480 : .dword 0x0000000000000000
MC1_CTL_490 : .dword 0x0000000000000000
MC1_CTL_4a0 : .dword 0x0000000000000000
MC1_CTL_4b0 : .dword 0x0000000000000000
MC1_CTL_4c0 : .dword 0x0000000000000000
MC1_CTL_4d0 : .dword 0x0000000000000000
MC1_CTL_4e0 : .dword 0x0000000000000000
MC1_CTL_4f0 : .dword 0x0000000000000000
MC1_CTL_500 : .dword 0x0000000000000000
MC1_CTL_510 : .dword 0x0000000000000000
MC1_CTL_520 : .dword 0x0000000000000000
MC1_CTL_530 : .dword 0x0000000000000000
MC1_CTL_540 : .dword 0x0000000000000000
MC1_CTL_550 : .dword 0x0000000000000000
MC1_CTL_560 : .dword 0x0000000000000000
MC1_CTL_570 : .dword 0x0000000000000000
MC1_CTL_580 : .dword 0x0000000000000000
MC1_CTL_590 : .dword 0x0000000000000000
MC1_CTL_5a0 : .dword 0x0000000000000000
MC1_CTL_5b0 : .dword 0x0000000000000000
MC1_CTL_5c0 : .dword 0x0000000000000000
MC1_CTL_5d0 : .dword 0x0000000000000000
MC1_CTL_5e0 : .dword 0x0000000000000000
MC1_CTL_5f0 : .dword 0x0000000000000000
MC1_CTL_600 : .dword 0x0000000000000000
MC1_CTL_610 : .dword 0x0000000000000000
MC1_CTL_620 : .dword 0x0000000000000000
MC1_CTL_630 : .dword 0x0000000000000000
MC1_CTL_640 : .dword 0x0000000000000000
MC1_CTL_650 : .dword 0x0000000000000000
MC1_CTL_660 : .dword 0x0000000000000000
MC1_CTL_670 : .dword 0x0000000000000000
MC1_CTL_680 : .dword 0x0000000000000000
MC1_CTL_690 : .dword 0x0000000000000000
MC1_CTL_6a0 : .dword 0x0000000000000000
MC1_CTL_6b0 : .dword 0x0000000000000000
MC1_CTL_6c0 : .dword 0x0000000000000000
MC1_CTL_6d0 : .dword 0x0000000000000000
MC1_CTL_6e0 : .dword 0x0000000000000000
MC1_CTL_6f0 : .dword 0x0000000000000000
MC1_CTL_700 : .dword 0x0000000000000000
MC1_CTL_710 : .dword 0x0000000000000000
MC1_CTL_720 : .dword 0x0000000000000000
MC1_CTL_730 : .dword 0x0000000000000000
MC1_CTL_740 : .dword 0x0100000000000000
//MC1_CTL_750 : .dword 0x0100000101020101
MC1_CTL_750 : .dword 0x0101000101020101
//000000_01 wrlvl_cs(RW) 000000_00 sw_leveling_mode(RW) 000000_00 rdlvl_cs(RW) 000000_01 axi2_w_priority(RW) 000000_01 axi2_r_priority(RW) 000000_10 axi2_port_ordering(RW) 000000_01 axi1_w_priority(RW) 000000_01 axi1_r_priority(RW) 
MC1_CTL_760 : .dword 0x0303030000020001
//0000_0011 axi0_priority2_relative_priority(RW) 0000_0011 axi0_priority1_relative_priority(RW) 0000_0011 axi0_priority0_relative_priority(RW) 0000_0000 address_mirroring(RW) 00000_000 tdfi_dram_clk_disable(RW) 00000_010 bstlen(RW) 000000_00 zq_req(WR) 000000_01 zq_on_sref_exit(RW) 
MC1_CTL_770 : .dword 0x0101010202020203
//0000_0001 axi2_priority2_relative_priority(RW) 0000_0001 axi2_priority1_relative_priority(RW) 0000_0001 axi2_priority0_relative_priority(RW) 0000_0010 axi1_priority3_relative_priority(RW) 0000_0010 axi1_priority2_relative_priority(RW) 0000_0010 axi1_priority1_relative_priority(RW) 0000_0010 axi1_priority0_relative_priority(RW) 0000_0011 axi0_priority3_relative_priority(RW) 
MC1_CTL_780 : .dword 0x0102020400040001
//0000_0001 tdfi_dram_clk_enable(RW) 0000_0010 tdfi_ctrl_delay(RW) 0000_0010 rdlvl_gate_dq_zero_count(RW) 0000_0100 rdlvl_dq_zero_count(RW) 0000_0000 lowpower_refresh_enable(RW) 0000_0110 dram_class(RW) 0000_1100 burst_on_fly_bit(RW) 0000_0001 axi2_priority3_relative_priority(RW) 
MC1_CTL_790 : .dword 0x281900000f000303
//00_101000 wlmrd(RW) 00_011001 wldqsen(RW) 000_00000 lowpower_control(RW) 000_00000 lowpower_auto_enable(RW) 0000_1111 zqcs_chip(RW) 0000_0000 wrr_param_value_err(RD) 0000_0011 tdfi_wrlvl_dll(RW) 0000_0011 tdfi_rdlvl_dll(RW) 
MC1_CTL_7a0 : .dword 0x00000000000000ff
MC1_CTL_7b0 : .dword 0x0000000000000000
MC1_CTL_7c0 : .dword 0x0000000000000000
MC1_CTL_7d0 : .dword 0x0000000000000000
MC1_CTL_7e0 : .dword 0x0000000000000000
//00000000 rdlvl_gate_delay_2(RD) 00000000 rdlvl_gate_delay_1(RD) 00000000 rdlvl_gate_delay_0(RD) 00000000 rdlvl_gate_clk_adjust_8(RW) 00000000 rdlvl_gate_clk_adjust_7(RW) 00000000 rdlvl_gate_clk_adjust_6(RW) 00000000 rdlvl_gate_clk_adjust_5(RW) 00000000 rdlvl_gate_clk_adjust_4(RW) 
MC1_CTL_7f0 : .dword 0xff08000000000000
//11111111 rdlvl_max_delay(RW) 00001000 rdlvl_gate_max_delay(RW) 00000000 rdlvl_gate_delay_8(RD) 00000000 rdlvl_gate_delay_7(RD) 00000000 rdlvl_gate_delay_6(RD) 00000000 rdlvl_gate_delay_5(RD) 00000000 rdlvl_gate_delay_4(RD) 00000000 rdlvl_gate_delay_3(RD) 
MC1_CTL_800 : .dword 0x0000000000000000
//00000000 rdlvl_midpoint_delay_7(RD) 00000000 rdlvl_midpoint_delay_6(RD) 00000000 rdlvl_midpoint_delay_5(RD) 00000000 rdlvl_midpoint_delay_4(RD) 00000000 rdlvl_midpoint_delay_3(RD) 00000000 rdlvl_midpoint_delay_2(RD) 00000000 rdlvl_midpoint_delay_1(RD) 00000000 rdlvl_midpoint_delay_0(RD) 
MC1_CTL_810 : .dword 0x000000000000000e
//00000000 rdlvl_offset_delay_6(RW) 00000000 rdlvl_offset_delay_5(RW) 00000000 rdlvl_offset_delay_4(RW) 00000000 rdlvl_offset_delay_3(RW) 00000000 rdlvl_offset_delay_2(RW) 00000000 rdlvl_offset_delay_1(RW) 00000000 rdlvl_offset_delay_0(RW) 00000000 rdlvl_midpoint_delay_8(RD) 
MC1_CTL_820 : .dword 0x0420000c20400000
//00000100 tdfi_wrlvl_resplat(RW) 00000000 tdfi_wrlvl_resp(RD) 00000000 tdfi_rdlvl_rr(RW) 00001100 tdfi_rdlvl_resplat(RW) 00000000 tdfi_rdlvl_resp(RD) 01000000 refresh_per_zq(RW) 00000000 rdlvl_offset_delay_8(RW) 00000000 rdlvl_offset_delay_7(RW) 
//MC1_CTL_830 : .dword 0x0000000000000c0a
MC1_CTL_830 : .dword 0x1313131313130c0a
//00000000 wrlvl_delay_5(RD) 00000000 wrlvl_delay_4(RD) 00000000 wrlvl_delay_3(RD) 00000000 wrlvl_delay_2(RD) 00000000 wrlvl_delay_1(RD) 00000000 wrlvl_delay_0(RD) 00000010 tmod(RW) 00001010 tdfi_wrlvl_ww(RW) 
MC1_CTL_840 : .dword 0x0000640064001313
//00000000000000_0001100100 axi1_priority_relax(RW) 000000_0001100100 axi0_priority_relax(RW) 00000000 wrlvl_delay_8(RD) 00000000 wrlvl_delay_7(RD) 00000000 wrlvl_delay_6(RD) 
MC1_CTL_850 : .dword 0x0000000000000064
MC1_CTL_860 : .dword 0x0200004000000000
MC1_CTL_870 : .dword 0x0046004600460046
//0_000000000000010 emrs1_data_3(RW) 0_000000000000010 emrs1_data_2(RW) 0_000000000000010 emrs1_data_1(RW) 0_000000000000010 emrs1_data_0(RW) 
MC1_CTL_880 : .dword 0x0000000000000000
//MC1_CTL_890 : .dword 0x0a520a520a520a52
MC1_CTL_890 : .dword 0x0a520a520a520a52
//0_000010000010000 mrs_data_3(RW) 0_000010000010000 mrs_data_2(RW) 0_000010000010000 mrs_data_1(RW) 0_000010000010000 mrs_data_0(RW) 
MC1_CTL_8a0 : .dword 0x00000000001c001c
MC1_CTL_8b0 : .dword 0x0000000000000000
MC1_CTL_8c0 : .dword 0x0004000000000000
MC1_CTL_8d0 : .dword 0x00000000c8000000
MC1_CTL_8e0 : .dword 0x0000000000000050
//MC1_CTL_8f0 : .dword 0x000000000a150080 //clk skew cleared
//MC1_CTL_8f0 : .dword 0x000000000a100080 //clk skew cleared
MC1_CTL_8f0 : .dword 0x000000002b352180
//MC1_CTL_8f0 : .dword 0x000000001b251180
//MC1_CTL_8f0 : .dword 0x0000000040404080
//0000000000000000000000000111100_000000000000000000000000001111000 dll_ctrl_reg_2(RW) 
MC1_CTL_900 : .dword 0x0000000000000000
MC1_CTL_910 : .dword 0x0000000000000000
MC1_CTL_920 : .dword 0x0000000000000000
MC1_CTL_930 : .dword 0x0000000000000000
MC1_CTL_940 : .dword 0x0300000000050500
MC1_CTL_950 : .dword 0x0000000000000a03
MC1_CTL_960 : .dword 0x0604000100000000
//000_00101 rdlat_adj(RW) 0000_0100 wrlat_adj(RW) 0000000_0 swlvl_start(WR) 0000000_0 swlvl_load(WR) 0000000_0 swlvl_exit(WR) 000000_000000000000000000 int_status(RD) 
MC1_CTL_970 : .dword 0x000000000003e805
MC1_CTL_start_DATA_LO: .word  0x00000000
//0000000_1 rw_same_en 0000000_0 reg_dimm_enable 0000000_0 reduc 0000000_0 pwrup_srefresh_exit
MC1_CTL_start_DATA_HI: .word  0x01010100
//0000000_1 swap_port_rw_same_en 0000000_1 swap_en 0000000_0 start 0000000_0 srefresh
#endif

