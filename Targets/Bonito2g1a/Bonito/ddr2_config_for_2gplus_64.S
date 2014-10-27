#######################################
#define REG_ADDRESS 0x0
//#define CONFIG_BASE 0xaff00000
//#define CONFIG_BASE 0x900000002ff00000
#define CONFIG_BASE 0x900000000ff00000
    

        .global ddr2_config
        .ent    ddr2_config
        .set    noreorder
        .set    mips3
ddr2_config:
    dli     a0, CONFIG_BASE
    la      a2, ddr2_reg_data
    addu    a2, a2, s0
    li      t1, 152
    daddiu	v0, a0, 0x0

reg_write:

    ld      a1, 0x0(a2)
    sd      a1, REG_ADDRESS(v0)

    subu    t1, t1, 0x1
    addiu   a2, a2, 0x8
    bne     t1, $0, reg_write
    daddiu   v0, v0, 0x10

//#define DEBUG_DDR_MC0
#ifdef DEBUG_DDR_MC0
    /* store the a0,ra */
	move	t8, a0
	move	t1, ra

    PRINTSTR("Change some parameters of MC0:");
1:
	PRINTSTR("\r\nPlease input the register number you want to change!!!(0:jump out.1~0x97): ");
	li	t6, 0x00
	bal	inputaddress	#input value stored in v0
	nop
	move	t5, v0	#store v0 to t5
	
	beqz	t5, 2f	#if input 0,jump out
	nop
	dsll	t5, t5, 4	#t5 is the offset relative to a0
	daddu	t5, t5, t8

	PRINTSTR("\r\nPlease input the data-hex: ");
	li	t6, 0x00
	bal	inputaddress	#input value stored in v0
	nop
	sd	v0, 0x0(t5)	#v0 is the input value
	b	1b
	nop
2:	
    /* recover the a0,ra */
	move	a0, t8
	move	ra, t1
#endif

    ############start##########
    /***** set start to 1,start to initialize SDRAM *****/
    daddiu	v0, a0, 0x0
    dli     a2, 0x0000010000000000
    ld      a1, 0x30(v0)
    or      a1, a1, a2
    sd      a1, 0x30(v0)

    daddiu  v0, a0, 0x960 //wait initialization complete 
1:
    ld      a1, 0x0(v0)
    andi    a1, a1, 0x100
    beqz    a1, 1b
    nop
    ###############################

    jr      ra
    nop
    .end    ddr2_config

	.rdata
	.align 5
    .global ddr2_reg_data
ddr2_reg_data:
MC0_CTL_000 : .dword 0x0000010000010101
//000000000000000_0 concurrentap(RW) 0000000_1 bank_split_en(RW) 0000000_0 auto_refresh_mode(RW) 0000000_0 arefresh(WR) 0000000_0 ap(RW) 0000000_1 addr_cmp_en(RW) 0000000_1 active_aging(RW) 
MC0_CTL_010 : .dword 0x0000000100010000
//0000000_0 fwc(WR) 0000000_0 fast_write(RW) 0000000_1 enable_quick_srefresh(RW) 0000000_1 eight_bank_mode(RW) 0000000_0 ecc_disable_w_uc_err(RW) 0000000_1 dqs_n_en(RW) 0000000_0 dll_bypass_mode(RW) 0000000_0 dlllockreg(RD) 
MC0_CTL_020 : .dword 0x0100010101000000
//0000000_1 priority_en(RW) 0000000_0 power_down(RW) 0000000_1 placement_en(RW) 0000000_1 odt_add_turn_clk_en(RW) 0000000_0 no_cmd_init(RW) 0000000_0 intrptwritea(RW) 0000000_0 intrptreada(RW) 0000000_0 intrptapburst(RW) 
MC0_CTL_030 : .dword 0x0001000001000000	
//0000000_1 swap_port_rw_same_en(RW) 0000000_1 swap_en(RW) 0000000_0 start(RW) 0000000_0 srefresh(RW+) 0000000_1 rw_same_en(RW) 0000000_0 reg_dimm_enable(RW) 0000000_0 reduc(RW) 0000000_0 pwrup_srefresh_exit(RW) 
MC0_CTL_040 : .dword 0x0100010200000100
//000000_01 rtt_0(RW) 000000_11 ctrl_raw(RW) 000000_01 axi0_w_priority(RW) 000000_01 axi0_r_priority(RW) 0000000_0 write_modereg(WR) 0000000_1 writeinterp(RW) 0000000_1 tref_enable(RW) 0000000_1 tras_lockout(RW) 
MC0_CTL_050 : .dword 0x0200000004060100
//00000_000 q_fullness(RW) 00000_000 port_data_error_type(RD) 000_00000 out_of_range_type(RD) 00000_000 max_cs_reg(RD) 00000_100 column_size(RW) 0000_0101 caslat(RW) 00000_010 addr_pins(RW) 00000000
MC0_CTL_060 : .dword 0x0a05030503030003
//0000_1010 aprebit(RW) 0000_0101 wrlat(RW) 0000_0100 twtr(RW) 000_00110 twr_int(RW) 00000_011 trtp(RW) 00000_100 trrd(RW) 0000000000000_011 tcke(RW) 
MC0_CTL_070 : .dword 0x0000020000030c0c
//0000_0000 max_row_reg(RD) 0000_0000 max_col_reg(RD) 0000_0000 initaref(RW) 00000000000000000000_1111 cs_map(RW) 000_01010 caslat_lin_gate(RW) 000_01010 caslat_lin(RW) 
MC0_CTL_080 : .dword 0x0804020108040201
//0000_0001 odt_wr_map_cs3(RW) 0000_0010 odt_wr_map_cs2(RW) 0000_0100 odt_wr_map_cs1(RW) 0000_1000 odt_wr_map_cs0(RW) 0000_0001 odt_rd_map_cs3(RW) 0000_0010 odt_rd_map_cs2(RW) 0000_0100 odt_rd_map_cs1(RW) 0000_1000 odt_rd_map_cs0(RW) 
MC0_CTL_090 : .dword 0x0000060c00000000
//000_00000 ocd_adjust_pup_cs_0(RW) 000_00000 ocd_adjust_pdn_cs_0(RW) 0000_0101 trp(RW) 000_01011 tdal(RW) 000000000000_0000 port_cmd_error_type(RD) 0000000000000000
MC0_CTL_0a0 : .dword 0x0000003f3f18020e
//00000000000000000000000000_111111 command_age_count(RW) 00_111111 age_count(RW) 000_10100 trc(RW) 000_00110 tmrd(RW) 000_10010 tfaw(RW) 
MC0_CTL_0b0 : .dword 0x0000000000000000
MC0_CTL_0c0 : .dword 0x0000330612000000
//000000000000000000101100 trfc(RW) 00000101 trcd_int(RW) 00001111 tras_min(RW) 00000000 out_of_range_length(RD) 00000000 ecc_u_synd(RD) 00000000 ecc_c_synd(RD) 
MC0_CTL_0d0 : .dword 0x0000000000000000
MC0_CTL_0e0 : .dword 0x0000000000000000
MC0_CTL_0f0 : .dword 0x0000000000000000
//Bit 21:16 dll_lock(RD)
MC0_CTL_100 : .dword 0x0000000000000000
MC0_CTL_110 : .dword 0x0000000000000300	#100M+
//MC0_CTL_110 : .dword 0x0000000000000618	#200M+
//MC0_CTL_110 : .dword 0x0000000000000924     #300M+
//MC0_CTL_110 : .dword 0x0000000000000c30	#400M
//0_000000000000000 emrs2_data_1(RW) 0_000000000000000 emrs2_data_0(RW) 000000000000000000_00110000101101 tref(RW) 
MC0_CTL_120 : .dword 0xffff000000000000
//0000000000011100 axi0_en_size_lt_width_instr(RW) 00000000000000000_000000000000000 emrs2_data_3(RW) 0_000000000000000 emrs2_data_2(RW) 
MC0_CTL_130 : .dword 0x1a00000202000000	#100M+
//MC0_CTL_130 : .dword 0x36b0000202000000	#200M+
//MC0_CTL_130 : .dword 0x51f0000202000000 	#300M+
//MC0_CTL_130 : .dword 0x6d60000202000000	#400M
//0110110101010110 tras_max(RW) 0000000000000011 tpdex(RW) 0000001000000000 tdll(RW) 0000000000000000 tcpd(RW) 
MC0_CTL_140 : .dword 0x0000204000c80037
//0000000000000000 xor_check_bits(RW) 0000000000000000 version(RD) 0000001000000000 txsr(RW) 0000000000110000 txsnr(RW) 
MC0_CTL_150 : .dword 0x0000000000020000
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
MC0_CTL_1d0 : .dword 0x0200070000000001
//0000_0010 tdfi_phy_wrlat_base(RW) 0000_0000 tdfi_phy_wrlat(RD) 000_00111 tdfi_phy_rdlat(RW) 0000_0000 tdfi_ctrlupd_min(RD) 000000000000_0000 dram_clk_disable(RW) 0000000_1 odt_alt_en(RW) 0000000_1 drive_dq_dqs(RW) 
MC0_CTL_1e0 : .dword 0x0000000000000200
//00_00000000000000 tdfi_phyupd_type0(RD) 00_00000000000000 tdfi_phyupd_resp(RD) 00_00000000000000 tdfi_ctrlupd_max(RD) 000_00000 tdfi_rddata_en_base(RW) 000_00000 tdfi_rddata_en(RD) 

/********
MC0_CTL_1f0 : .dword 0x0020208000000000
MC0_CTL_200 : .dword 0x0020208000202080
MC0_CTL_210 : .dword 0x0020208000202080
MC0_CTL_220 : .dword 0x0020208000202080
MC0_CTL_230 : .dword 0x0020208000202080

MC0_CTL_240 : .dword 0x00000e0000000e00
MC0_CTL_250 : .dword 0x00000e0000000e00
MC0_CTL_260 : .dword 0x00000e0000000e00
MC0_CTL_270 : .dword 0x00000e0000000e00
MC0_CTL_280 : .dword 0x0000000000000e00
**********/
MC0_CTL_1f0 : .dword 0x0014148000000000
//hXXXXXXXX dll_ctrl_reg_0_0(RW) h000000 hXX dft_ctrl_reg(RW)
MC0_CTL_200 : .dword 0x0014148000141480
MC0_CTL_210 : .dword 0x0014148000141480
MC0_CTL_220 : .dword 0x0014148000141480
MC0_CTL_230 : .dword 0x0014148000141480
//00000000001000000000111510000000 dll_ctrl_reg_0_8(RW) 00000000001000000000111010000000 dll_ctrl_reg_0_7(RW) 
MC0_CTL_240 : .dword 0x0000140000001400
MC0_CTL_250 : .dword 0x0000140000001400
MC0_CTL_260 : .dword 0x0000140000001400
MC0_CTL_270 : .dword 0x0000140000001400
MC0_CTL_280 : .dword 0x0000000000001400
//hXXXXXXX 00_00 dll_obs_reg_0_0(RW) 00000000000000000000111000000000 dll_ctrl_reg_1_8(RW) 
MC0_CTL_290 : .dword 0x0000000000000000
//hXXXXXXX 00_00 dll_obs_reg_0_2(RW) hXXXXXXX 00_00 dll_obs_reg_0_1(RW)
MC0_CTL_2a0 : .dword 0x0000000000000000
//hXXXXXXX 00_00 dll_obs_reg_0_4(RW) hXXXXXXX 00_00 dll_obs_reg_0_3(RW)
MC0_CTL_2b0 : .dword 0x0000000000000000
//hXXXXXXX 00_00 dll_obs_reg_0_6(RW) hXXXXXXX 00_00 dll_obs_reg_0_5(RW)
MC0_CTL_2c0 : .dword 0x0000000000000000
//hXXXXXXX 00_00 dll_obs_reg_0_8(RW) hXXXXXXX 00_00 dll_obs_reg_0_7(RW)
//MC0_CTL_2d0 : .dword 0xf300494603c0019d
MC0_CTL_2d0 : .dword 0xc400484403fc09b4
//11110100000000000011101100100111 phy_ctrl_reg_0_0(RD) 000000_00000000000000000110011101 pad_ctrl_reg_0(RW) 
MC0_CTL_2e0 : .dword 0xc4004844c4004844
MC0_CTL_2f0 : .dword 0xc4004844c4004844
MC0_CTL_300 : .dword 0xc4004844c4004844
MC0_CTL_310 : .dword 0xc4004844c4004844
/********  
MC0_CTL_2d0 : .dword 0x1300483303c009b4
//11110100000000000011101100100111 phy_ctrl_reg_0_0(RD) 000000_00000000000000000110011101 pad_ctrl_reg_0(RW) 
MC0_CTL_2e0 : .dword 0x1300483313004833
MC0_CTL_2f0 : .dword 0x1300483313004833
MC0_CTL_300 : .dword 0x1300483313004833
MC0_CTL_310 : .dword 0x1300483313004833
**********/
MC0_CTL_320 : .dword 0x26c0000126c00001
MC0_CTL_330 : .dword 0x26c0000126c00001
MC0_CTL_340 : .dword 0x26c0000126c00001
MC0_CTL_350 : .dword 0x26c0000126c00001
MC0_CTL_360 : .dword 0x0800e00526c00001
//00000000000000001100000000000101 phy_ctrl_reg_2(RW) 00000111110000000000001100000001 phy_ctrl_reg_1_8(RD) 
//--------------
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
//-------------
MC0_CTL_710 : .dword 0x0000000000000000
//bit 48 en_wr_leveling(RW)
MC0_CTL_720 : .dword 0x0000000000000000
//0000000_0 rdlvl_gate_req(WR) 0000000_0 rdlvl_gate_preamble_check_en(RW) 0000000_0 rdlvl_gate_en(RW) 0000000_0 rdlvl_en(RW) 0000000_0 rdlvl_begin_delay_en(RW) 00000000 00000000 0000000_0 swlvl_op_done(RD) 
MC0_CTL_730 : .dword 0x0000000000000000
//0000000_0 rdlvl_offset_dir_7(RW) 0000000_0 rdlvl_offset_dir_6(RW) 0000000_0 rdlvl_offset_dir_5(RW)0000000_0 rdlvl_offset_dir_4(RW) 0000000_0 rdlvl_offset_dir_3(RW) 0000000_0 rdlvl_offset_dir_2(RW) 0000000_0 rdlvl_offset_dir_1(RW) 0000000_0 rdlvl_offset_dir_0(RW) 
MC0_CTL_740 : .dword 0x0100000000000000
//000000_00 axi1_port_ordering(RW) 000000_00 axi0_port_ordering(RW) 0000000_0 wrlvl_req(WR) 0000000_0 wrlvl_interval_ct_en(RW) 0000000_0 weight_round_robin_weight_sharing(RW) 0000000_0 weight_round_robin_latency_control 0000000_0(RW) rdlvl_req 0000000_0(WR) rdlvl_offset_dir_8(RW)
MC0_CTL_750 : .dword 0x0000000101020101
//000000_01 wrlvl_cs(RW) 000000_00 sw_leveling_mode(RW) 000000_00 rdlvl_cs(RW) 000000_01 axi2_w_priority(RW) 000000_01 axi2_r_priority(RW) 000000_10 axi2_port_ordering(RW) 000000_01 axi1_w_priority(RW) 000000_01 axi1_r_priority(RW) 
MC0_CTL_760 : .dword 0x0303030000020000
//0000_0011 axi0_priority2_relative_priority(RW) 0000_0011 axi0_priority1_relative_priority(RW) 0000_0011 axi0_priority0_relative_priority(RW) 0000_0000 address_mirroring(RW) 00000_000 tdfi_dram_clk_disable(RW) 00000_010 bstlen(RW) 000000_00 zq_req(WR) 000000_01 zq_on_sref_exit(RW) 
MC0_CTL_770 : .dword 0x0101010202020203
//0000_0001 axi2_priority2_relative_priority(RW) 0000_0001 axi2_priority1_relative_priority(RW) 0000_0001 axi2_priority0_relative_priority(RW) 0000_0010 axi1_priority3_relative_priority(RW) 0000_0010 axi1_priority2_relative_priority(RW) 0000_0010 axi1_priority1_relative_priority(RW) 0000_0010 axi1_priority0_relative_priority(RW) 0000_0011 axi0_priority3_relative_priority(RW) 
MC0_CTL_780 : .dword 0x0102000000040001
//0000_0001 tdfi_dram_clk_enable(RW) 0000_0010 tdfi_ctrl_delay(RW) 0000_0010 rdlvl_gate_dq_zero_count(RW) 0000_0100 rdlvl_dq_zero_count(RW) 0000_0000 lowpower_refresh_enable(RW) 0000_0110 dram_class(RW) 0000_1100 burst_on_fly_bit(RW) 0000_0001 axi2_priority3_relative_priority(RW) 
MC0_CTL_790 : .dword 0x0000000000000000
//00_101000 wlmrd(RW) 00_011001 wldqsen(RW) 000_00000 lowpower_control(RW) 000_00000 lowpower_auto_enable(RW) 0000_1111 zqcs_chip(RW) 0000_0000 wrr_param_value_err(RD) 0000_0011 tdfi_wrlvl_dll(RW) 0000_0011 tdfi_rdlvl_dll(RW) 
MC0_CTL_7a0 : .dword 0x0000000000000000
//_00000000 swlvl_resp_6(RW) _00000000 swlvl_resp_5(RW) _00000000 swlvl_resp_4 _00000000 swlvl_resp_3(RW) _00000000 swlvl_resp_2(RW) _00000000 swlvl_resp_1(RW) _00000000 swlvl_resp_0(RW) _00000000 dfi_wrlvl_max_delay(RW)
MC0_CTL_7b0 : .dword 0x0000000000000000
//_00000000 rdlvl_begin_delay_5(RW) _00000000 rdlvl_begin_delay_4(RW) _00000000 rdlvl_begin_delay_3(RW) _00000000 rdlvl_begin_delay_2(RW) _00000000 rdlvl_begin_delay_1(RW) _00000000 rdlvl_begin_delay_0(RW)  _00000000 swlvl_resp_8(RW) _00000000 swlvl_resp_7(RW)
MC0_CTL_7c0 : .dword 0x0000000000000000
//_00000000 rdlvl_end_delay_4(RW) _00000000 rdlvl_end_delay_3(RW) _00000000 rdlvl_end_delay_2(RW) _00000000 rdlvl_end_delay_1(RW) _00000000 rdlvl_end_delay_0(RW) _00000000 rdlvl_begin_delay_8(RW) _00000000 rdlvl_begin_delay_7(RW) _00000000 rdlvl_begin_delay_6(RW) 
MC0_CTL_7d0 : .dword 0x0000000000000000
//_00000000 rdlvl_gate_clk_adjust_3(RW) _00000000 rdlvl_gate_clk_adjust_2(RW) _00000000 rdlvl_gate_clk_adjust_1(RW) _00000000 rdlvl_gate_clk_adjust_0(RW) _00000000 rdlvl_end_delay_8(RW) _00000000 rdlvl_end_delay_7(RW) _00000000 rdlvl_end_delay_6(RW) 00000000 rdlvl_end_delay_5(RW) 
MC0_CTL_7e0 : .dword 0x0000000000000000
//00000000 rdlvl_gate_delay_2(RD) 00000000 rdlvl_gate_delay_1(RD) 00000000 rdlvl_gate_delay_0(RD) 00000000 rdlvl_gate_clk_adjust_8(RW) 00000000 rdlvl_gate_clk_adjust_7(RW) 00000000 rdlvl_gate_clk_adjust_6(RW) 00000000 rdlvl_gate_clk_adjust_5(RW) 00000000 rdlvl_gate_clk_adjust_4(RW) 
MC0_CTL_7f0 : .dword 0x0000000000000000
//11111111 rdlvl_max_delay(RW) 00001000 rdlvl_gate_max_delay(RW) 00000000 rdlvl_gate_delay_8(RD) 00000000 rdlvl_gate_delay_7(RD) 00000000 rdlvl_gate_delay_6(RD) 00000000 rdlvl_gate_delay_5(RD) 00000000 rdlvl_gate_delay_4(RD) 00000000 rdlvl_gate_delay_3(RD) 
MC0_CTL_800 : .dword 0x0000000000000000
//00000000 rdlvl_midpoint_delay_7(RD) 00000000 rdlvl_midpoint_delay_6(RD) 00000000 rdlvl_midpoint_delay_5(RD) 00000000 rdlvl_midpoint_delay_4(RD) 00000000 rdlvl_midpoint_delay_3(RD) 00000000 rdlvl_midpoint_delay_2(RD) 00000000 rdlvl_midpoint_delay_1(RD) 00000000 rdlvl_midpoint_delay_0(RD) 
MC0_CTL_810 : .dword 0x0000000000000000
//00000000 rdlvl_offset_delay_6(RW) 00000000 rdlvl_offset_delay_5(RW) 00000000 rdlvl_offset_delay_4(RW) 00000000 rdlvl_offset_delay_3(RW) 00000000 rdlvl_offset_delay_2(RW) 00000000 rdlvl_offset_delay_1(RW) 00000000 rdlvl_offset_delay_0(RW) 00000000 rdlvl_midpoint_delay_8(RD) 
MC0_CTL_820 : .dword 0x0000000000000000
//00000100 tdfi_wrlvl_resplat(RW) 00000000 tdfi_wrlvl_resp(RD) 00000000 tdfi_rdlvl_rr(RW) 00001100 tdfi_rdlvl_resplat(RW) 00000000 tdfi_rdlvl_resp(RD) 01000000 refresh_per_zq(RW) 00000000 rdlvl_offset_delay_8(RW) 00000000 rdlvl_offset_delay_7(RW) 
//MC0_CTL_830 : .dword 0x000000000000020a
MC0_CTL_830 : .dword 0x1c24241c1c1c0c0a
//MC0_CTL_830 : .dword 0x40444b474543020a
//00000000 wrlvl_delay_5(RD) 00000000 wrlvl_delay_4(RD) 00000000 wrlvl_delay_3(RD) 00000000 wrlvl_delay_2(RD) 00000000 wrlvl_delay_1(RD) 00000000 wrlvl_delay_0(RD) 00000010 tmod(RW) 00001010 tdfi_wrlvl_ww(RW) 
//MC0_CTL_840 : .dword 0x0000640064000000
MC0_CTL_840 : .dword 0x0000640064001c1c
//MC0_CTL_840 : .dword 0x0000640064003b3c
//00000000000000_0001100100 axi1_priority_relax(RW) 000000_0001100100 axi0_priority_relax(RW) 00000000 wrlvl_delay_8(RD) 00000000 wrlvl_delay_7(RD) 00000000 wrlvl_delay_6(RD) 
MC0_CTL_850 : .dword 0x0000000000000064
//000000_0000000000 out_of_range_source_id(RD) 000000_0000000000 ecc_u_id(RD) 000000_0000000000 ecc_c_id(RD) 000000_0000000000 axi2_priority_relax(RW)
MC0_CTL_860 : .dword 0x0000000000000000
//0000_000000000000 zqini(RW) 0000_000000000000 zqcs(RW) 000000_0000000000 port_data_error_id(RD) 000000_0000000000 port_cmd_error_id(RD)
MC0_CTL_870 : .dword 0x0046004600460046
//0_000000000000010 emrs1_data_3(RW) 0_000000000000010 emrs1_data_2(RW) 0_000000000000010 emrs1_data_1(RW) 0_000000000000010 emrs1_data_0(RW) 
MC0_CTL_880 : .dword 0x0000000000000000
//0_000000000000010 emrs3_data_3(RW) 0_000000000000010 emrs3_data_2(RW) 0_000000000000010 emrs3_data_1(RW) 0_000000000000010 emrs3_data_0(RW) 
MC0_CTL_890 : .dword 0x0a620a620a620a62
//0_000010000010000 mrs_data_3(RW) 0_000010000010000 mrs_data_2(RW) 0_000010000010000 mrs_data_1(RW) 0_000010000010000 mrs_data_0(RW) 
MC0_CTL_8a0 : .dword 0x00000000001c001c
//hXXXX lowpower_internal_cnt(RW) hXXXX lowpower_external_cnt(RW) hXXXX axi2_en_size_lt_width_instr(RW) hXXXX axi1_en_size_lt_width_instr(RW)
MC0_CTL_8b0 : .dword 0x0000000000000000
//hXXXX refresh_per_rdlvl(RW) hXXXX lowpower_self_refresh_cnt(RW) hXXXX lowpower_refresh_hold(RW) hXXXX lowpower_power_down_cnt(RW)
MC0_CTL_8c0 : .dword 0x0000000000000000
//hXXXX wrlvl_interval(RW) hXXXX tdfi_wrlvl_max(RW) hXXXX tdfi_rdlvl_max(RW) hXXXX refresh_per_rdlvl_gate(RW)
MC0_CTL_8d0 : .dword 0x00000000c8000000
//h00_XXXXXXXX cke_inactive(RW) 000000_000000000000000000 wrlvl_status(RD)
MC0_CTL_8e0 : .dword 0x0000000000000050
//h00000000_XXXXXXXX trst_powon(RW)
//MC0_CTL_8f0 : .dword 0x0000000020242080
//MC0_CTL_8f0 : .dword 0x000000002b352180
MC0_CTL_8f0 : .dword 0x0000000030303080
//MC0_CTL_8f0 : .dword 0x0000000030343080
//hXXXXXXX 000_0 XXXXXXXX dll_ctrl_reg_2(RW) 
MC0_CTL_900 : .dword 0x0000000000000000
//h000000 00_00 X XXXXXXXX rdlvl_error_status(RW)
MC0_CTL_910 : .dword 0x0000000000000000
//hXXXXXXXX XXXXXXXX rdlvl_gate_resp_mask[63:0](RW)
MC0_CTL_920 : .dword 0x0000000000000000
//h00000000000000_XX rdlvl_gate_resp_mask[71:64](RW)
MC0_CTL_930 : .dword 0x0000000000000000
//hXXXXXXXX XXXXXXXX rdlvl_resp_mask[63:0](RW)
MC0_CTL_940 : .dword 0x0007070000050500
//0000_0000 tdfi_rdlvl_en(RW) 00000_000 w2r_samecs_delay(RW) 00000_000 w2r_diffcs_delay(RW) 00000_000 lvl_status(RD) 0000000_0 rdlvl_edge(RW) 0000_0000 cksrx(RW) 0000_0000 cksre(RW) _00000000 rdlvl_resp_mask[71:64](RW)
MC0_CTL_950 : .dword 0x0000000000000a00
//hXXXXX 00_00 XXXX mask_int[17:0](RW) hXXXX txpdll(RW) 0000_0000 tdfi_wrlvl_en(RW)
MC0_CTL_960 : .dword 0x0705000000000000
//000_00101 rdlat_adj(RW) 0000_0100 wrlat_adj(RW) 0000000_0 swlvl_start(WR) 0000000_0 swlvl_load(WR) 0000000_0 swlvl_exit(WR) 000000_000000000000000000 int_status(RD) 
MC0_CTL_970 : .dword 0x000000000003e805
//h00000 000_0 XXXX int_ack[16:0](WR) hXXXX dll_rst_delay(RW) hXX dll_rst_adj_dly(RW)
