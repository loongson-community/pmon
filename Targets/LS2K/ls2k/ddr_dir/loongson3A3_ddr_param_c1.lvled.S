//DO NOT modify the content of this file!

	.align 12
//3A Node1 MC0/3B Node2 leveled param
c1_mc0_level_info:
//leveled_mark and ddr frequency clksel
.dword  0x0
c1_mc0_dimm0_info: 
.dword  0xffffffffffffffff
.dword  0xffffffffffffffff
c1_mc0_dimm1_info:
.dword  0xffffffffffffffff
.dword  0xffffffffffffffff
c1_mc0_ddr_clksel:
.dword  0xffffffffffffffff
n1_ddr2_reg_data_mc0_leveled:
n2_ddr2_reg_data_leveled:
N2_Leveled_MC0_CTL_000 : .dword 0x0000010000000100
//000000000000000_0 concurrentap(RW) 0000000_1 bank_split_en(RW) 0000000_0 auto_refresh_mode(RW) 0000000_0 arefresh(WR) 0000000_0 ap(RW) 0000000_1 addr_cmp_en(RW) 0000000_1 active_aging(RW) 
N2_Leveled_MC0_CTL_010 : .dword 0x0000000100010000
//0000000_0 fwc(WR) 0000000_0 fast_write(RW) 0000000_1 enable_quick_srefresh(RW) 0000000_1 eight_bank_mode(RW) 0000000_0 ecc_disable_w_uc_err(RW) 0000000_1 dqs_n_en(RW) 0000000_0 dll_bypass_mode(RW) 0000000_0 dlllockreg(RD) 
N2_Leveled_MC0_CTL_020 : .dword 0x0100010000000000
//0000000_1 priority_en(RW) 0000000_0 power_down(RW) 0000000_1 placement_en(RW) 0000000_1 odt_add_turn_clk_en(RD) 0000000_0 no_cmd_init(RW) 0000000_0 intrptwritea(RW) 0000000_0 intrptreada(RW) 0000000_0 intrptapburst(RW) 
N2_Leveled_MC0_CTL_030 : .dword 0x0000000001000000	
//0000000_1 swap_port_rw_same_en(RW) 0000000_1 swap_en(RW) 0000000_0 start(RW) 0000000_0 srefresh(RW+) 0000000_1 rw_same_en(RW) 0000000_0 reg_dimm_enable(RW) 0000000_0 reduc(RW) 0000000_0 pwrup_srefresh_exit(RW) 
N2_Leveled_MC0_CTL_040 : .dword 0x0002010200000100
//000000_00 rtt_0(RD) 000000_11 ctrl_raw(RW) 000000_01 axi0_w_priority(RW) 000000_01 axi0_r_priority(RW) 0000000_0 write_modereg(WR) 0000000_1 writeinterp(RW) 0000000_1 tref_enable(RW) 0000000_1 tras_lockout(RW) 
N2_Leveled_MC0_CTL_050 : .dword 0x0700000004050100 
//00000_000 q_fullness(RW) 00000_000 port_data_error_type(RD) 000_00000 out_of_range_type(RD) 00000_100 max_cs_reg(RD) 00000_100 column_size(RW) 0000_0101 caslat(RW) 00000_010 addr_pins(RW) 00000000
N2_Leveled_MC0_CTL_060 : .dword 0x0a04060805060003
//0000_1010 aprebit(RW) 0000_0101 wrlat(RW) 0000_0100 twtr(RW) 000_00110 twr_int(RW) 00000_011 trtp(RW) 00000_100 trrd(RW) 0000000000000_011 tcke(RW) 
N2_Leveled_MC0_CTL_070 : .dword 0x0000020000010a0a
//0000_1111 max_row_reg(RD) 0000_1110 max_col_reg(RD) 0000_0000 initaref(RW) 00000000000000000000_1111 cs_map(RW) 000_01010 caslat_lin_gate(RW) 000_01010 caslat_lin(RW) 
N2_Leveled_MC0_CTL_080 : .dword 0x0804020100000000
//0000_0001 odt_wr_map_cs3(RW) 0000_0010 odt_wr_map_cs2(RW) 0000_0100 odt_wr_map_cs1(RW) 0000_1000 odt_wr_map_cs0(RW) 0000_0001 odt_rd_map_cs3(RW) 0000_0010 odt_rd_map_cs2(RW) 0000_0100 odt_rd_map_cs1(RW) 0000_1000 odt_rd_map_cs0(RW) 
N2_Leveled_MC0_CTL_090 : .dword 0x0000070d00000000
//000_00000 ocd_adjust_pup_cs_0(RW) 000_00000 ocd_adjust_pdn_cs_0(RW) 0000_0101 trp(RW) 000_01011 tdal(RW) 000000000000_0000 port_cmd_error_type(RD) 0000000000000000
N2_Leveled_MC0_CTL_0a0 : .dword 0x0000003f3f180614
//00000000000000000000000000_111111 command_age_count(RW) 00_111111 age_count(RW) 000_10100 trc(RW) 000_00110 tmrd(RW) 000_10010 tfaw(RW) 
N2_Leveled_MC0_CTL_0b0 : .dword 0x0000000000000000
N2_Leveled_MC0_CTL_0c0 : .dword 0x0000330612000000
//000000000000000000101100 trfc(RW) 00000101 trcd_int(RW) 00001111 tras_min(RW) 00000000 out_of_range_length(RD) 00000000 ecc_u_synd(RD) 00000000 ecc_c_synd(RD) 
N2_Leveled_MC0_CTL_0d0 : .dword 0x0000000000000000
N2_Leveled_MC0_CTL_0e0 : .dword 0x0000000000000000
N2_Leveled_MC0_CTL_0f0 : .dword 0x0000000000000000
//Bit 21:16 dll_lock(RD)
N2_Leveled_MC0_CTL_100 : .dword 0x0000000000000000
//N2_Leveled_MC0_CTL_110 : .dword 0x00000000000002e0	#100M+
N2_Leveled_MC0_CTL_110 : .dword 0x00000000000005e0	#200M+
//N2_Leveled_MC0_CTL_110 : .dword 0x0000000000000900   #300M+
//N2_Leveled_MC0_CTL_110 : .dword 0x0000000000000c00	#400M
//0_000000000000000 emrs2_data_1(RW) 0_000000000000000 emrs2_data_0(RW) 000000000000000000_00110000101101 tref(RW) 
N2_Leveled_MC0_CTL_120 : .dword 0x001c000000000000
//0000000000011100 axi0_en_size_lt_width_instr(RW) 00000000000000000_000000000000000 emrs2_data_3(RW) 0_000000000000000 emrs2_data_2(RW) 
//N2_Leveled_MC0_CTL_130 : .dword 0x1b200003020000c8	#100M+
N2_Leveled_MC0_CTL_130 : .dword 0x36800003020000c8	#200M+
//N2_Leveled_MC0_CTL_130 : .dword 0x51d00003020000c8 	#300M+
//N2_Leveled_MC0_CTL_130 : .dword 0x6d300003020000c8	#400M
//0110110101010110 tras_max(RW) 0000000000000011 tpdex(RW) 0000001000000000 tdll(RW) 0000000000000000 tcpd(RW) 
N2_Leveled_MC0_CTL_140 : .dword 0x0000204002000060
//0000000000000000 xor_check_bits(RW) 0000000000000000 version(RD) 0000001000000000 txsr(RW) 0000000000110000 txsnr(RW) 
N2_Leveled_MC0_CTL_150 : .dword 0x0000000000027100
//000_0000000000000000000000000000000000000 ecc_c_addr(RD) 000000000000000000011011 tinit(RW) 
N2_Leveled_MC0_CTL_160 : .dword 0x0000000000000000
//000000000000000000000000000_0000000000000000000000000000000000000 ecc_u_addr(RD) 
N2_Leveled_MC0_CTL_170 : .dword 0x0000000000000000
//000000000000000000000000000_0000000000000000000000000000000000000 out_of_range_addr(RD) 
N2_Leveled_MC0_CTL_180 : .dword 0x0000000000000000
//000000000000000000000000000_0000000000000000000000000000000000000 port_cmd_error_addr(RD) 
N2_Leveled_MC0_CTL_190 : .dword 0x0000000000000000
//0000000000000000000000000000000000000000000000000000000000000000 ecc_c_data(RD) 
N2_Leveled_MC0_CTL_1a0 : .dword 0x0000000000000000
//0000000000000000000000000000000000000000000000000000000000000000 ecc_u_data(RD) 
N2_Leveled_MC0_CTL_1b0 : .dword 0x0000000000000000
//0000000000000000000000000000000000000000000000000000000000000_000 cke_delay(RW) 
N2_Leveled_MC0_CTL_1c0 : .dword 0x0000000000000000
N2_Leveled_MC0_CTL_1d0 : .dword 0x0200070000000001
//0000_0010 tdfi_phy_wrlat_base(RW) 0000_0000 tdfi_phy_wrlat(RD) 000_00111 tdfi_phy_rdlat(RW) 0000_0100 tdfi_ctrlupd_min(RD) 000000000000_0000 dram_clk_disable(RW) 0000000_1 odt_alt_en(RW) 0000000_1 drive_dq_dqs(RW) 
N2_Leveled_MC0_CTL_1e0 : .dword 0x0000000000000200
//00_00000000000000 tdfi_phyupd_type0(RD) 00_00000000000000 tdfi_phyupd_resp(RD) 00_00000000000000 tdfi_ctrlupd_max(RD) 000_00000 tdfi_rddata_en_base(RW) 000_00000 tdfi_rddata_en(RD) 
N2_Leveled_MC0_CTL_1f0 : .dword 0x0020008000000000
N2_Leveled_MC0_CTL_200 : .dword 0x0020008000200080
N2_Leveled_MC0_CTL_210 : .dword 0x0020008000200080
N2_Leveled_MC0_CTL_220 : .dword 0x0020008000200080
N2_Leveled_MC0_CTL_230 : .dword 0x0020008000200080
N2_Leveled_MC0_CTL_240 : .dword 0x0000200000002000
N2_Leveled_MC0_CTL_250 : .dword 0x0000200000002000
N2_Leveled_MC0_CTL_260 : .dword 0x0000200000002000
N2_Leveled_MC0_CTL_270 : .dword 0x0000200000002000
N2_Leveled_MC0_CTL_280 : .dword 0x0000000000002000

N2_Leveled_MC0_CTL_290 : .dword 0x0000000000000000
//hXXXXXXX 00_00 dll_obs_reg_0_2(RW) hXXXXXXX 00_00 dll_obs_reg_0_1(RW)
N2_Leveled_MC0_CTL_2a0 : .dword 0x0000000000000000
//hXXXXXXX 00_00 dll_obs_reg_0_4(RW) hXXXXXXX 00_00 dll_obs_reg_0_3(RW)
N2_Leveled_MC0_CTL_2b0 : .dword 0x0000000000000000
//hXXXXXXX 00_00 dll_obs_reg_0_6(RW) hXXXXXXX 00_00 dll_obs_reg_0_5(RW)
N2_Leveled_MC0_CTL_2c0 : .dword 0x0000000000000000
//hXXXXXXX 00_00 dll_obs_reg_0_8(RW) hXXXXXXX 00_00 dll_obs_reg_0_7(RW)
N2_Leveled_MC0_CTL_2d0 : .dword 0x1302480302e009bc
N2_Leveled_MC0_CTL_2e0 : .dword 0x1302480313024803
N2_Leveled_MC0_CTL_2f0 : .dword 0x1302480313024803
N2_Leveled_MC0_CTL_300 : .dword 0x1302480313024803
N2_Leveled_MC0_CTL_310 : .dword 0x1302480313024803
N2_Leveled_MC0_CTL_320 : .dword 0x26c0000126c00001
N2_Leveled_MC0_CTL_330 : .dword 0x26c0000126c00001
N2_Leveled_MC0_CTL_340 : .dword 0x26c0000126c00001
N2_Leveled_MC0_CTL_350 : .dword 0x26c0000126c00001
N2_Leveled_MC0_CTL_360 : .dword 0x0800e00026c00001
//00000000000000001100000000000101 phy_ctrl_reg_2(RW) 00000111110000000000001100000001 phy_ctrl_reg_1_8(RD) 
//--------------
N2_Leveled_MC0_CTL_370 : .dword 0x0000000000000000
N2_Leveled_MC0_CTL_380 : .dword 0x0000000000000000
N2_Leveled_MC0_CTL_390 : .dword 0x0000000000000000
N2_Leveled_MC0_CTL_3a0 : .dword 0x0000000000000000
N2_Leveled_MC0_CTL_3b0 : .dword 0x0000000000000000
N2_Leveled_MC0_CTL_3c0 : .dword 0x0000000000000000
N2_Leveled_MC0_CTL_3d0 : .dword 0x0000000000000000
N2_Leveled_MC0_CTL_3e0 : .dword 0x0000000000000000
N2_Leveled_MC0_CTL_3f0 : .dword 0x0000000000000000
N2_Leveled_MC0_CTL_400 : .dword 0x0000000000000000
N2_Leveled_MC0_CTL_410 : .dword 0x0000000000000000
N2_Leveled_MC0_CTL_420 : .dword 0x0000000000000000
N2_Leveled_MC0_CTL_430 : .dword 0x0000000000000000
N2_Leveled_MC0_CTL_440 : .dword 0x0000000000000000
N2_Leveled_MC0_CTL_450 : .dword 0x0000000000000000
N2_Leveled_MC0_CTL_460 : .dword 0x0000000000000000
N2_Leveled_MC0_CTL_470 : .dword 0x0000000000000000
N2_Leveled_MC0_CTL_480 : .dword 0x0000000000000000
N2_Leveled_MC0_CTL_490 : .dword 0x0000000000000000
N2_Leveled_MC0_CTL_4a0 : .dword 0x0000000000000000
N2_Leveled_MC0_CTL_4b0 : .dword 0x0000000000000000
N2_Leveled_MC0_CTL_4c0 : .dword 0x0000000000000000
N2_Leveled_MC0_CTL_4d0 : .dword 0x0000000000000000
N2_Leveled_MC0_CTL_4e0 : .dword 0x0000000000000000
N2_Leveled_MC0_CTL_4f0 : .dword 0x0000000000000000
N2_Leveled_MC0_CTL_500 : .dword 0x0000000000000000
N2_Leveled_MC0_CTL_510 : .dword 0x0000000000000000
N2_Leveled_MC0_CTL_520 : .dword 0x0000000000000000
N2_Leveled_MC0_CTL_530 : .dword 0x0000000000000000
N2_Leveled_MC0_CTL_540 : .dword 0x0000000000000000
N2_Leveled_MC0_CTL_550 : .dword 0x0000000000000000
N2_Leveled_MC0_CTL_560 : .dword 0x0000000000000000
N2_Leveled_MC0_CTL_570 : .dword 0x0000000000000000
N2_Leveled_MC0_CTL_580 : .dword 0x0000000000000000
N2_Leveled_MC0_CTL_590 : .dword 0x0000000000000000
N2_Leveled_MC0_CTL_5a0 : .dword 0x0000000000000000
N2_Leveled_MC0_CTL_5b0 : .dword 0x0000000000000000
N2_Leveled_MC0_CTL_5c0 : .dword 0x0000000000000000
N2_Leveled_MC0_CTL_5d0 : .dword 0x0000000000000000
N2_Leveled_MC0_CTL_5e0 : .dword 0x0000000000000000
N2_Leveled_MC0_CTL_5f0 : .dword 0x0000000000000000
N2_Leveled_MC0_CTL_600 : .dword 0x0000000000000000
N2_Leveled_MC0_CTL_610 : .dword 0x0000000000000000
N2_Leveled_MC0_CTL_620 : .dword 0x0000000000000000
N2_Leveled_MC0_CTL_630 : .dword 0x0000000000000000
N2_Leveled_MC0_CTL_640 : .dword 0x0000000000000000
N2_Leveled_MC0_CTL_650 : .dword 0x0000000000000000
N2_Leveled_MC0_CTL_660 : .dword 0x0000000000000000
N2_Leveled_MC0_CTL_670 : .dword 0x0000000000000000
N2_Leveled_MC0_CTL_680 : .dword 0x0000000000000000
N2_Leveled_MC0_CTL_690 : .dword 0x0000000000000000
N2_Leveled_MC0_CTL_6a0 : .dword 0x0000000000000000
N2_Leveled_MC0_CTL_6b0 : .dword 0x0000000000000000
N2_Leveled_MC0_CTL_6c0 : .dword 0x0000000000000000
N2_Leveled_MC0_CTL_6d0 : .dword 0x0000000000000000
N2_Leveled_MC0_CTL_6e0 : .dword 0x0000000000000000
N2_Leveled_MC0_CTL_6f0 : .dword 0x0000000000000000
N2_Leveled_MC0_CTL_700 : .dword 0x0000000000000000
//-------------
N2_Leveled_MC0_CTL_710 : .dword 0x0000000000000000
//bit 48 en_wr_leveling(RW)
N2_Leveled_MC0_CTL_720 : .dword 0x0000000000000000
//0000000_0 rdlvl_gate_req(WR) 0000000_0 rdlvl_gate_preamble_check_en(RW) 0000000_0 rdlvl_gate_en(RW) 0000000_0 rdlvl_en(RW) 0000000_0 rdlvl_begin_delay_en(RW) 00000000 00000000 0000000_0 swlvl_op_done(RD) 
N2_Leveled_MC0_CTL_730 : .dword 0x0000000000000000
//0000000_0 rdlvl_offset_dir_7(RW) 0000000_0 rdlvl_offset_dir_6(RW) 0000000_0 rdlvl_offset_dir_5(RW)0000000_0 rdlvl_offset_dir_4(RW) 0000000_0 rdlvl_offset_dir_3(RW) 0000000_0 rdlvl_offset_dir_2(RW) 0000000_0 rdlvl_offset_dir_1(RW) 0000000_0 rdlvl_offset_dir_0(RW) 
N2_Leveled_MC0_CTL_740 : .dword 0x0100000000000000
//000000_00 axi1_port_ordering(RW) 000000_00 axi0_port_ordering(RW) 0000000_0 wrlvl_req(WR) 0000000_0 wrlvl_interval_ct_en(RW) 0000000_0 weight_round_robin_weight_sharing(RW) 0000000_0 weight_round_robin_latency_control 0000000_0(RW) rdlvl_req 0000000_0(WR) rdlvl_offset_dir_8(RW)
N2_Leveled_MC0_CTL_750 : .dword 0x0100000101020101
//000000_01 wrlvl_cs(RW) 000000_00 sw_leveling_mode(RW) 000000_00 rdlvl_cs(RW) 000000_01 axi2_w_priority(RW) 000000_01 axi2_r_priority(RW) 000000_10 axi2_port_ordering(RW) 000000_01 axi1_w_priority(RW) 000000_01 axi1_r_priority(RW) 
N2_Leveled_MC0_CTL_760 : .dword 0x0303030000020000
//0000_0011 axi0_priority2_relative_priority(RW) 0000_0011 axi0_priority1_relative_priority(RW) 0000_0011 axi0_priority0_relative_priority(RW) 0000_0000 address_mirroring(RW) 00000_000 tdfi_dram_clk_disable(RW) 00000_010 bstlen(RW) 000000_00 zq_req(WR) 000000_01 zq_on_sref_exit(RW) 
N2_Leveled_MC0_CTL_770 : .dword 0x0101010202020203
//0000_0001 axi2_priority2_relative_priority(RW) 0000_0001 axi2_priority1_relative_priority(RW) 0000_0001 axi2_priority0_relative_priority(RW) 0000_0010 axi1_priority3_relative_priority(RW) 0000_0010 axi1_priority2_relative_priority(RW) 0000_0010 axi1_priority1_relative_priority(RW) 0000_0010 axi1_priority0_relative_priority(RW) 0000_0011 axi0_priority3_relative_priority(RW) 
N2_Leveled_MC0_CTL_780 : .dword 0x0102020400040c01
//0000_0001 tdfi_dram_clk_enable(RW) 0000_0010 tdfi_ctrl_delay(RW) 0000_0010 rdlvl_gate_dq_zero_count(RW) 0000_0100 rdlvl_dq_zero_count(RW) 0000_0000 lowpower_refresh_enable(RW) 0000_0110 dram_class(RW) 0000_1100 burst_on_fly_bit(RW) 0000_0001 axi2_priority3_relative_priority(RW) 
N2_Leveled_MC0_CTL_790 : .dword 0x281900000f000303
//00_101000 wlmrd(RW) 00_011001 wldqsen(RW) 000_00000 lowpower_control(RW) 000_00000 lowpower_auto_enable(RW) 0000_1111 zqcs_chip(RW) 0000_0000 wrr_param_value_err(RD) 0000_0011 tdfi_wrlvl_dll(RW) 0000_0011 tdfi_rdlvl_dll(RW) 
N2_Leveled_MC0_CTL_7a0 : .dword 0x0000000000000000
//_00000000 swlvl_resp_6(RW) _00000000 swlvl_resp_5(RW) _00000000 swlvl_resp_4 _00000000 swlvl_resp_3(RW) _00000000 swlvl_resp_2(RW) _00000000 swlvl_resp_1(RW) _00000000 swlvl_resp_0(RW) _00000000 dfi_wrlvl_max_delay(RW)
N2_Leveled_MC0_CTL_7b0 : .dword 0x0000000000000000
//_00000000 rdlvl_begin_delay_5(RW) _00000000 rdlvl_begin_delay_4(RW) _00000000 rdlvl_begin_delay_3(RW) _00000000 rdlvl_begin_delay_2(RW) _00000000 rdlvl_begin_delay_1(RW) _00000000 rdlvl_begin_delay_0(RW)  _00000000 swlvl_resp_8(RW) _00000000 swlvl_resp_7(RW)
N2_Leveled_MC0_CTL_7c0 : .dword 0x0000000000000000
//_00000000 rdlvl_end_delay_4(RW) _00000000 rdlvl_end_delay_3(RW) _00000000 rdlvl_end_delay_2(RW) _00000000 rdlvl_end_delay_1(RW) _00000000 rdlvl_end_delay_0(RW) _00000000 rdlvl_begin_delay_8(RW) _00000000 rdlvl_begin_delay_7(RW) _00000000 rdlvl_begin_delay_6(RW) 
N2_Leveled_MC0_CTL_7d0 : .dword 0x0000000000000000
//_00000000 rdlvl_gate_clk_adjust_3(RW) _00000000 rdlvl_gate_clk_adjust_2(RW) _00000000 rdlvl_gate_clk_adjust_1(RW) _00000000 rdlvl_gate_clk_adjust_0(RW) _00000000 rdlvl_end_delay_8(RW) _00000000 rdlvl_end_delay_7(RW) _00000000 rdlvl_end_delay_6(RW) 00000000 rdlvl_end_delay_5(RW) 
N2_Leveled_MC0_CTL_7e0 : .dword 0x0000000000000000
//00000000 rdlvl_gate_delay_2(RD) 00000000 rdlvl_gate_delay_1(RD) 00000000 rdlvl_gate_delay_0(RD) 00000000 rdlvl_gate_clk_adjust_8(RW) 00000000 rdlvl_gate_clk_adjust_7(RW) 00000000 rdlvl_gate_clk_adjust_6(RW) 00000000 rdlvl_gate_clk_adjust_5(RW) 00000000 rdlvl_gate_clk_adjust_4(RW) 
N2_Leveled_MC0_CTL_7f0 : .dword 0xff22000000000000
//11111111 rdlvl_max_delay(RW) 00001000 rdlvl_gate_max_delay(RW) 00000000 rdlvl_gate_delay_8(RD) 00000000 rdlvl_gate_delay_7(RD) 00000000 rdlvl_gate_delay_6(RD) 00000000 rdlvl_gate_delay_5(RD) 00000000 rdlvl_gate_delay_4(RD) 00000000 rdlvl_gate_delay_3(RD) 
N2_Leveled_MC0_CTL_800 : .dword 0x0000000000000000
//00000000 rdlvl_midpoint_delay_7(RD) 00000000 rdlvl_midpoint_delay_6(RD) 00000000 rdlvl_midpoint_delay_5(RD) 00000000 rdlvl_midpoint_delay_4(RD) 00000000 rdlvl_midpoint_delay_3(RD) 00000000 rdlvl_midpoint_delay_2(RD) 00000000 rdlvl_midpoint_delay_1(RD) 00000000 rdlvl_midpoint_delay_0(RD) 
N2_Leveled_MC0_CTL_810 : .dword 0x0000000000000000
//00000000 rdlvl_offset_delay_6(RW) 00000000 rdlvl_offset_delay_5(RW) 00000000 rdlvl_offset_delay_4(RW) 00000000 rdlvl_offset_delay_3(RW) 00000000 rdlvl_offset_delay_2(RW) 00000000 rdlvl_offset_delay_1(RW) 00000000 rdlvl_offset_delay_0(RW) 00000000 rdlvl_midpoint_delay_8(RD) 
N2_Leveled_MC0_CTL_820 : .dword 0x0400000c00400000
//00000100 tdfi_wrlvl_resplat(RW) 00000000 tdfi_wrlvl_resp(RD) 00000000 tdfi_rdlvl_rr(RW) 00001100 tdfi_rdlvl_resplat(RW) 00000000 tdfi_rdlvl_resp(RD) 01000000 refresh_per_zq(RW) 00000000 rdlvl_offset_delay_8(RW) 00000000 rdlvl_offset_delay_7(RW) 
N2_Leveled_MC0_CTL_830 : .dword 0x0000000000000500
//00000000 wrlvl_delay_5(RD) 00000000 wrlvl_delay_4(RD) 00000000 wrlvl_delay_3(RD) 00000000 wrlvl_delay_2(RD) 00000000 wrlvl_delay_1(RD) 00000000 wrlvl_delay_0(RD) 00000010 tmod(RW) 00000000 tdfi_wrlvl_ww(RW) 
N2_Leveled_MC0_CTL_840 : .dword 0x0000640064000000
//00000000000000_0001100100 axi1_priority_relax(RW) 000000_0001100100 axi0_priority_relax(RW) 00000000 wrlvl_delay_8(RD) 00000000 wrlvl_delay_7(RD) 00000000 wrlvl_delay_6(RD) 
N2_Leveled_MC0_CTL_850 : .dword 0x0000000000000064
//000000_0000000000 out_of_range_source_id(RD) 000000_0000000000 ecc_u_id(RD) 000000_0000000000 ecc_c_id(RD) 000000_0000000000 axi2_priority_relax(RW)
N2_Leveled_MC0_CTL_860 : .dword 0x0000000000000000
//0000_000000000000 zqini(RW) 0000_000000000000 zqcs(RW) 000000_0000000000 port_data_error_id(RD) 000000_0000000000 port_cmd_error_id(RD)
N2_Leveled_MC0_CTL_870 : .dword 0x0000000000000000
//0_000000000000010 emrs1_data_3(RD) 0_000000000000010 emrs1_data_2(RD) 0_000000000000010 emrs1_data_1(RD) 0_000000000000010 emrs1_data_0(RD) 
N2_Leveled_MC0_CTL_880 : .dword 0x0000000000000000
//0_000000000000010 emrs3_data_3(RW) 0_000000000000010 emrs3_data_2(RW) 0_000000000000010 emrs3_data_1(RW) 0_000000000000010 emrs3_data_0(RW) 
N2_Leveled_MC0_CTL_890 : .dword 0x0000000000000000
//0_000010000010000 mrs_data_3(RD) 0_000010000010000 mrs_data_2(RD) 0_000010000010000 mrs_data_1(RD) 0_000010000010000 mrs_data_0(RD) 
N2_Leveled_MC0_CTL_8a0 : .dword 0x00000000001c001c
//hXXXX lowpower_internal_cnt(RW) hXXXX lowpower_external_cnt(RW) hXXXX axi2_en_size_lt_width_instr(RW) hXXXX axi1_en_size_lt_width_instr(RW)
N2_Leveled_MC0_CTL_8b0 : .dword 0x0000000000000000
//hXXXX refresh_per_rdlvl(RW) hXXXX lowpower_self_refresh_cnt(RW) hXXXX lowpower_refresh_hold(RW) hXXXX lowpower_power_down_cnt(RW)
N2_Leveled_MC0_CTL_8c0 : .dword 0x0000000000000000
//hXXXX wrlvl_interval(RW) hXXXX tdfi_wrlvl_max(RW) hXXXX tdfi_rdlvl_max(RW) hXXXX refresh_per_rdlvl_gate(RW)
N2_Leveled_MC0_CTL_8d0 : .dword 0x002faf0800000000
//h00_XXXXXXXX cke_inactive(RW) 000000_000000000000000000 wrlvl_status(RD)
N2_Leveled_MC0_CTL_8e0 : .dword 0x0000000023c34600
//h00000000_XXXXXXXX trst_pwron(RW)
N2_Leveled_MC0_CTL_8f0 : .dword 0x0000000040404080
//hXXXXXXX 000_0 XXXXXXXX dll_ctrl_reg_2(RW) 
N2_Leveled_MC0_CTL_900 : .dword 0x0000000000000000
//h000000 00_00 X XXXXXXXX rdlvl_error_status(RW)
N2_Leveled_MC0_CTL_910 : .dword 0x0000000000000000
//hXXXXXXXX XXXXXXXX rdlvl_gate_resp_mask[63:0](RW)
N2_Leveled_MC0_CTL_920 : .dword 0x0000000000000000
//h00000000000000_XX rdlvl_gate_resp_mask[71:64](RW)
N2_Leveled_MC0_CTL_930 : .dword 0x0000000000000000
//hXXXXXXXX XXXXXXXX rdlvl_resp_mask[63:0](RW)
N2_Leveled_MC0_CTL_940 : .dword 0x0006060000050500
//0000_0000 tdfi_rdlvl_en(RW) 00000_000 w2r_samecs_delay(RW) 00000_000 w2r_diffcs_delay(RW) 00000_000 lvl_status(RD) 0000000_0 rdlvl_edge(RW) 0000_0000 cksrx(RW) 0000_0000 cksre(RW) _00000000 rdlvl_resp_mask[71:64](RW)
N2_Leveled_MC0_CTL_950 : .dword 0x0000000000001000
//hXXXXX 00_00 XXXX mask_int[17:0](RW) hXXXX txpdll(RW) 0000_0000 tdfi_wrlvl_en(RW)
N2_Leveled_MC0_CTL_960 : .dword 0x0604000000000000
//000_00101 rdlat_adj(RW) 0000_0100 wrlat_adj(RW) 0000000_0 swlvl_start(WR) 0000000_0 swlvl_load(WR) 0000000_0 swlvl_exit(WR) 000000_000000000000000000 int_status(RD) 
N2_Leveled_MC0_CTL_970 : .dword 0x0000000000004810
//h00000 000_0 XXXX int_ack[16:0](WR) hXXXX dll_rst_delay(RW) hXX dll_rst_adj_dly(RW)
N2_Leveled_MC0_CTL_980 : .dword 0x0001010001000101
//0000000_0 zq_in_progress(RD) 0000000_1 zqcs_rotate(RW) 0000000_0 wrlvl_reg_en(RW) 0000000_0 wrlvl_en(RW) 0000000_1 resync_dll_per_aref_en(RW) 0000000_0 resync_dll(WR) 0000000_0 rdlvl_reg_en(RW) 0000000_0 rdlvl_gate_reg_en(RW) 
N2_Leveled_MC0_CTL_990 : .dword 0x0606060606060600
//00000_000 w2w_samecs_dly(RW) 00000_001 w2w_diffcs_dly(RW) 00000_010 tbst_int_interval(RW) 00000_010 r2w_samecs_dly(RW) 00000_010 r2w_diffcs_dly(RW) 00000_000 r2r_samecs_dly(RW) 00000_001 r2r_diffcs_dly(RW) 00000_000 axi_aligned_strobe_disable(RW) 
N2_Leveled_MC0_CTL_9a0 : .dword 0x0707040200070106
//00000111 tdfi_wrlvl_load(RW) 00000111 tdfi_rdlvl_load(RW) 000_00011 tckesr(RW) 000_00010 tccd(RW) 000_00000 add_odt_clk_difftype_diffcs(RW) 0000_0110 trp_ab(RW) 0000_0001 add_odt_clk_sametype_diffcs(RW) 0000_0000 add_odt_clk_difftype_samecs(RW) 
N2_Leveled_MC0_CTL_9b0 : .dword 0x02000100000a000f
//0000_001000000000 zqinit(RW) 0000_000100000000 zqcl(RW) 000000_0000001010 tdfi_wrlvl_ww(RW) 000000_0000001111 tdfi_rdlvl_rr(RW) 
N2_Leveled_MC0_CTL_9c0 : .dword 0x0a520c2d0c2d0c2d
//0_000101001010010 mr0_data_0(RW) 00_00110000101101 tdfi_phyupd_type3(RW) 00_00110000101101 tdfi_phyupd_type2(RW) 00_00110000101101 tdfi_phyupd_type1(RW) 
N2_Leveled_MC0_CTL_9d0 : .dword 0x00440a520a520a52
//0_000000000000100 mr1_data_0(RW) 0_000101001010010 mr0_data_3(RW) 0_000101001010010 mr0_data_2(RW) 0_000101001010010 mr0_data_1(RW) 
N2_Leveled_MC0_CTL_9e0 : .dword 0x0000004400440044
//0_000000000000000 mr2_data_0(RW) 0_000000000000100 mr1_data_3(RW) 0_000000000000100 mr1_data_2(RW) 0_000000000000100 mr1_data_1(RW) 
N2_Leveled_MC0_CTL_9f0 : .dword 0x0000000000000000
//0_000000000000000 mr3_data_0(RW) 0_000000000000000 mr2_data_3(RW) 0_000000000000000 mr2_data_2(RW) 0_000000000000000 mr2_data_1(RW) 
N2_Leveled_MC0_CTL_a00 : .dword 0x00ff000000000000
//0000000011111111 dfi_wrlvl_max_delay(RW) 0_000000000000000 mr3_data_3(RW) 0_000000000000000 mr3_data_2(RW) 0_000000000000000 mr3_data_1(RW) 
N2_Leveled_MC0_CTL_a10 : .dword 0x0000000000000000
//0000000000000000 rdlvl_begin_delay_3(RD) 0000000000000000 rdlvl_begin_delay_2(RD) 0000000000000000 rdlvl_begin_delay_1(RD) 0000000000000000 rdlvl_begin_delay_0(RD) 
N2_Leveled_MC0_CTL_a20 : .dword 0x0000000000000000
//0000000000000000 rdlvl_begin_delay_7(RD) 0000000000000000 rdlvl_begin_delay_6(RD) 0000000000000000 rdlvl_begin_delay_5(RD) 0000000000000000 rdlvl_begin_delay_4(RD) 
N2_Leveled_MC0_CTL_a30 : .dword 0x0020002000200000
//0000111000001110 rdlvl_delay_2(RW) 0000111000001110 rdlvl_delay_1(RW) 0000111000001110 rdlvl_delay_0(RW) 0000000000000000 rdlvl_begin_delay_8(RD) 
N2_Leveled_MC0_CTL_a40 : .dword 0x0020002000200020
//0000111000001110 rdlvl_delay_6(RW) 0000111000001110 rdlvl_delay_5(RW) 0000111000001110 rdlvl_delay_4(RW) 0000111000001110 rdlvl_delay_3(RW) 
N2_Leveled_MC0_CTL_a50 : .dword 0x0000000000200020
//0000000000000000 rdlvl_end_delay_1(RD) 0000000000000000 rdlvl_end_delay_0(RD) 0000111000001110 rdlvl_delay_8(RW) 0000111000001110 rdlvl_delay_7(RW) 
N2_Leveled_MC0_CTL_a60 : .dword 0x0000000000000000
//0000000000000000 rdlvl_end_delay_5(RD) 0000000000000000 rdlvl_end_delay_4(RD) 0000000000000000 rdlvl_end_delay_3(RD) 0000000000000000 rdlvl_end_delay_2(RD) 
N2_Leveled_MC0_CTL_a70 : .dword 0x0000000000000000
//0000000000000000 rdlvl_gate_delay_0(RW+) 0000000000000000 rdlvl_end_delay_8(RD) 0000000000000000 rdlvl_end_delay_7(RD) 0000000000000000 rdlvl_end_delay_6(RD) 
N2_Leveled_MC0_CTL_a80 : .dword 0x0000000000000000
//0000000000000000 rdlvl_gate_delay_4(RW+) 0000000000000000 rdlvl_gate_delay_3(RW+) 0000000000000000 rdlvl_gate_delay_2(RW+) 0000000000000000 rdlvl_gate_delay_1(RW+) 
N2_Leveled_MC0_CTL_a90 : .dword 0x0000000000000000
//0000000000000000 rdlvl_gate_delay_8(RW+) 0000000000000000 rdlvl_gate_delay_7(RW+) 0000000000000000 rdlvl_gate_delay_6(RW+) 0000000000000000 rdlvl_gate_delay_5(RW+) 
N2_Leveled_MC0_CTL_aa0 : .dword 0x0000ffff00000010
//0000000000000000 rdlvl_midpoint_delay_0(RD) 1111111111111111 rdlvl_max_delay(RW) 0000000000000000 rdlvl_gate_refresh_interval(RW) 0000000000010000 rdlvl_gate_max_delay(RW) 
N2_Leveled_MC0_CTL_ab0 : .dword 0x0000000000000000
//0000000000000000 rdlvl_midpoint_delay_4(RD) 0000000000000000 rdlvl_midpoint_delay_3(RD) 0000000000000000 rdlvl_midpoint_delay_2(RD) 0000000000000000 rdlvl_midpoint_delay_1(RD) 
N2_Leveled_MC0_CTL_ac0 : .dword 0x0000000000000000
//0000000000000000 rdlvl_midpoint_delay_8(RD) 0000000000000000 rdlvl_midpoint_delay_7(RD) 0000000000000000 rdlvl_midpoint_delay_6(RD) 0000000000000000 rdlvl_midpoint_delay_5(RD) 
N2_Leveled_MC0_CTL_ad0 : .dword 0x0000000000000000
//0000000000000000 rdlvl_offset_delay_3(RW) 0000000000000000 rdlvl_offset_delay_2(RW) 0000000000000000 rdlvl_offset_delay_1(RW) 0000000000000000 rdlvl_offset_delay_0(RW) 
N2_Leveled_MC0_CTL_ae0 : .dword 0x0000000000000000
//0000000000000000 rdlvl_offset_delay_7(RW) 0000000000000000 rdlvl_offset_delay_6(RW) 0000000000000000 rdlvl_offset_delay_5(RW) 0000000000000000 rdlvl_offset_delay_4(RW) 
N2_Leveled_MC0_CTL_af0 : .dword 0x0040004000000000
//0000000000000000 wrlvl_delay_1(RW+) 0000000000000000 wrlvl_delay_0(RW+) 0000000000000000 rdlvl_refresh_interval(RW) 0000000000000000 rdlvl_offset_delay_8(RW) 
N2_Leveled_MC0_CTL_b00 : .dword 0x0040004000400040
//0000000000000000 wrlvl_delay_5(RW+) 0000000000000000 wrlvl_delay_4(RW+) 0000000000000000 wrlvl_delay_3(RW+) 0000000000000000 wrlvl_delay_2(RW+) 
N2_Leveled_MC0_CTL_b10 : .dword 0x0000004000400040
//0000000000000000 wrlvl_refresh_interval(RW) 0000000000000000 wrlvl_delay_8(RW+) 0000000000000000 wrlvl_delay_7(RW+) 0000000000000000 wrlvl_delay_6(RW+) 
N2_Leveled_MC0_CTL_b20 : .dword 0x00000c2d00000c2d
//00000000000000000000110000101101 tdfi_rdlvl_resp(RW) 00000000000000000000110000101101 tdfi_rdlvl_max(RW) 
N2_Leveled_MC0_CTL_b30 : .dword 0x00000c2d00000000
//00000000000000000000110000101101 tdfi_wrlvl_resp(RW) 00000000000000000000000000000000 tdfi_wrlvl_max(RW) 

	.align 12
//3A Node1 MC1/3B Node2 leveled param
c1_mc1_level_info:
//leveled_mark and ddr frequency clksel
.dword  0x0
c1_mc1_dimm0_info: 
.dword  0xffffffffffffffff
.dword  0xffffffffffffffff
c1_mc1_dimm1_info:
.dword  0xffffffffffffffff
.dword  0xffffffffffffffff
c1_mc1_ddr_clksel:
.dword  0xffffffffffffffff
n1_ddr2_reg_data_mc1_leveled:
n3_ddr2_reg_data_leveled:
N3_Leveled_MC0_CTL_000 : .dword 0x0000010000000100
//000000000000000_0 concurrentap(RW) 0000000_1 bank_split_en(RW) 0000000_0 auto_refresh_mode(RW) 0000000_0 arefresh(WR) 0000000_0 ap(RW) 0000000_1 addr_cmp_en(RW) 0000000_1 active_aging(RW) 
N3_Leveled_MC0_CTL_010 : .dword 0x0000000100010000
//0000000_0 fwc(WR) 0000000_0 fast_write(RW) 0000000_1 enable_quick_srefresh(RW) 0000000_1 eight_bank_mode(RW) 0000000_0 ecc_disable_w_uc_err(RW) 0000000_1 dqs_n_en(RW) 0000000_0 dll_bypass_mode(RW) 0000000_0 dlllockreg(RD) 
N3_Leveled_MC0_CTL_020 : .dword 0x0100010000000000
//0000000_1 priority_en(RW) 0000000_0 power_down(RW) 0000000_1 placement_en(RW) 0000000_1 odt_add_turn_clk_en(RD) 0000000_0 no_cmd_init(RW) 0000000_0 intrptwritea(RW) 0000000_0 intrptreada(RW) 0000000_0 intrptapburst(RW) 
N3_Leveled_MC0_CTL_030 : .dword 0x0000000001000000	
//0000000_1 swap_port_rw_same_en(RW) 0000000_1 swap_en(RW) 0000000_0 start(RW) 0000000_0 srefresh(RW+) 0000000_1 rw_same_en(RW) 0000000_0 reg_dimm_enable(RW) 0000000_0 reduc(RW) 0000000_0 pwrup_srefresh_exit(RW) 
N3_Leveled_MC0_CTL_040 : .dword 0x0002010200000100
//000000_00 rtt_0(RD) 000000_11 ctrl_raw(RW) 000000_01 axi0_w_priority(RW) 000000_01 axi0_r_priority(RW) 0000000_0 write_modereg(WR) 0000000_1 writeinterp(RW) 0000000_1 tref_enable(RW) 0000000_1 tras_lockout(RW) 
N3_Leveled_MC0_CTL_050 : .dword 0x0700000004050100 
//00000_000 q_fullness(RW) 00000_000 port_data_error_type(RD) 000_00000 out_of_range_type(RD) 00000_100 max_cs_reg(RD) 00000_100 column_size(RW) 0000_0101 caslat(RW) 00000_010 addr_pins(RW) 00000000
N3_Leveled_MC0_CTL_060 : .dword 0x0a04060805060003
//0000_1010 aprebit(RW) 0000_0101 wrlat(RW) 0000_0100 twtr(RW) 000_00110 twr_int(RW) 00000_011 trtp(RW) 00000_100 trrd(RW) 0000000000000_011 tcke(RW) 
N3_Leveled_MC0_CTL_070 : .dword 0x0000020000010a0a
//0000_1111 max_row_reg(RD) 0000_1110 max_col_reg(RD) 0000_0000 initaref(RW) 00000000000000000000_1111 cs_map(RW) 000_01010 caslat_lin_gate(RW) 000_01010 caslat_lin(RW) 
N3_Leveled_MC0_CTL_080 : .dword 0x0804020100000000
//0000_0001 odt_wr_map_cs3(RW) 0000_0010 odt_wr_map_cs2(RW) 0000_0100 odt_wr_map_cs1(RW) 0000_1000 odt_wr_map_cs0(RW) 0000_0001 odt_rd_map_cs3(RW) 0000_0010 odt_rd_map_cs2(RW) 0000_0100 odt_rd_map_cs1(RW) 0000_1000 odt_rd_map_cs0(RW) 
N3_Leveled_MC0_CTL_090 : .dword 0x0000070d00000000
//000_00000 ocd_adjust_pup_cs_0(RW) 000_00000 ocd_adjust_pdn_cs_0(RW) 0000_0101 trp(RW) 000_01011 tdal(RW) 000000000000_0000 port_cmd_error_type(RD) 0000000000000000
N3_Leveled_MC0_CTL_0a0 : .dword 0x0000003f3f180614
//00000000000000000000000000_111111 command_age_count(RW) 00_111111 age_count(RW) 000_10100 trc(RW) 000_00110 tmrd(RW) 000_10010 tfaw(RW) 
N3_Leveled_MC0_CTL_0b0 : .dword 0x0000000000000000
N3_Leveled_MC0_CTL_0c0 : .dword 0x0000330612000000
//000000000000000000101100 trfc(RW) 00000101 trcd_int(RW) 00001111 tras_min(RW) 00000000 out_of_range_length(RD) 00000000 ecc_u_synd(RD) 00000000 ecc_c_synd(RD) 
N3_Leveled_MC0_CTL_0d0 : .dword 0x0000000000000000
N3_Leveled_MC0_CTL_0e0 : .dword 0x0000000000000000
N3_Leveled_MC0_CTL_0f0 : .dword 0x0000000000000000
//Bit 21:16 dll_lock(RD)
N3_Leveled_MC0_CTL_100 : .dword 0x0000000000000000
//N3_Leveled_MC0_CTL_110 : .dword 0x00000000000002e0	#100M+
N3_Leveled_MC0_CTL_110 : .dword 0x00000000000005e0	#200M+
//N3_Leveled_MC0_CTL_110 : .dword 0x0000000000000900   #300M+
//N3_Leveled_MC0_CTL_110 : .dword 0x0000000000000c00	#400M
//0_000000000000000 emrs2_data_1(RW) 0_000000000000000 emrs2_data_0(RW) 000000000000000000_00110000101101 tref(RW) 
N3_Leveled_MC0_CTL_120 : .dword 0x001c000000000000
//0000000000011100 axi0_en_size_lt_width_instr(RW) 00000000000000000_000000000000000 emrs2_data_3(RW) 0_000000000000000 emrs2_data_2(RW) 
//N3_Leveled_MC0_CTL_130 : .dword 0x1b200003020000c8	#100M+
N3_Leveled_MC0_CTL_130 : .dword 0x36800003020000c8	#200M+
//N3_Leveled_MC0_CTL_130 : .dword 0x51d00003020000c8 	#300M+
//N3_Leveled_MC0_CTL_130 : .dword 0x6d300003020000c8	#400M
//0110110101010110 tras_max(RW) 0000000000000011 tpdex(RW) 0000001000000000 tdll(RW) 0000000000000000 tcpd(RW) 
N3_Leveled_MC0_CTL_140 : .dword 0x0000204002000060
//0000000000000000 xor_check_bits(RW) 0000000000000000 version(RD) 0000001000000000 txsr(RW) 0000000000110000 txsnr(RW) 
N3_Leveled_MC0_CTL_150 : .dword 0x0000000000027100
//000_0000000000000000000000000000000000000 ecc_c_addr(RD) 000000000000000000011011 tinit(RW) 
N3_Leveled_MC0_CTL_160 : .dword 0x0000000000000000
//000000000000000000000000000_0000000000000000000000000000000000000 ecc_u_addr(RD) 
N3_Leveled_MC0_CTL_170 : .dword 0x0000000000000000
//000000000000000000000000000_0000000000000000000000000000000000000 out_of_range_addr(RD) 
N3_Leveled_MC0_CTL_180 : .dword 0x0000000000000000
//000000000000000000000000000_0000000000000000000000000000000000000 port_cmd_error_addr(RD) 
N3_Leveled_MC0_CTL_190 : .dword 0x0000000000000000
//0000000000000000000000000000000000000000000000000000000000000000 ecc_c_data(RD) 
N3_Leveled_MC0_CTL_1a0 : .dword 0x0000000000000000
//0000000000000000000000000000000000000000000000000000000000000000 ecc_u_data(RD) 
N3_Leveled_MC0_CTL_1b0 : .dword 0x0000000000000000
//0000000000000000000000000000000000000000000000000000000000000_000 cke_delay(RW) 
N3_Leveled_MC0_CTL_1c0 : .dword 0x0000000000000000
N3_Leveled_MC0_CTL_1d0 : .dword 0x0200070000000001
//0000_0010 tdfi_phy_wrlat_base(RW) 0000_0000 tdfi_phy_wrlat(RD) 000_00111 tdfi_phy_rdlat(RW) 0000_0100 tdfi_ctrlupd_min(RD) 000000000000_0000 dram_clk_disable(RW) 0000000_1 odt_alt_en(RW) 0000000_1 drive_dq_dqs(RW) 
N3_Leveled_MC0_CTL_1e0 : .dword 0x0000000000000200
//00_00000000000000 tdfi_phyupd_type0(RD) 00_00000000000000 tdfi_phyupd_resp(RD) 00_00000000000000 tdfi_ctrlupd_max(RD) 000_00000 tdfi_rddata_en_base(RW) 000_00000 tdfi_rddata_en(RD) 
N3_Leveled_MC0_CTL_1f0 : .dword 0x0020008000000000
N3_Leveled_MC0_CTL_200 : .dword 0x0020008000200080
N3_Leveled_MC0_CTL_210 : .dword 0x0020008000200080
N3_Leveled_MC0_CTL_220 : .dword 0x0020008000200080
N3_Leveled_MC0_CTL_230 : .dword 0x0020008000200080
N3_Leveled_MC0_CTL_240 : .dword 0x0000200000002000
N3_Leveled_MC0_CTL_250 : .dword 0x0000200000002000
N3_Leveled_MC0_CTL_260 : .dword 0x0000200000002000
N3_Leveled_MC0_CTL_270 : .dword 0x0000200000002000
N3_Leveled_MC0_CTL_280 : .dword 0x0000000000002000

N3_Leveled_MC0_CTL_290 : .dword 0x0000000000000000
//hXXXXXXX 00_00 dll_obs_reg_0_2(RW) hXXXXXXX 00_00 dll_obs_reg_0_1(RW)
N3_Leveled_MC0_CTL_2a0 : .dword 0x0000000000000000
//hXXXXXXX 00_00 dll_obs_reg_0_4(RW) hXXXXXXX 00_00 dll_obs_reg_0_3(RW)
N3_Leveled_MC0_CTL_2b0 : .dword 0x0000000000000000
//hXXXXXXX 00_00 dll_obs_reg_0_6(RW) hXXXXXXX 00_00 dll_obs_reg_0_5(RW)
N3_Leveled_MC0_CTL_2c0 : .dword 0x0000000000000000
//hXXXXXXX 00_00 dll_obs_reg_0_8(RW) hXXXXXXX 00_00 dll_obs_reg_0_7(RW)
N3_Leveled_MC0_CTL_2d0 : .dword 0x1302480302e009bc
N3_Leveled_MC0_CTL_2e0 : .dword 0x1302480313024803
N3_Leveled_MC0_CTL_2f0 : .dword 0x1302480313024803
N3_Leveled_MC0_CTL_300 : .dword 0x1302480313024803
N3_Leveled_MC0_CTL_310 : .dword 0x1302480313024803
N3_Leveled_MC0_CTL_320 : .dword 0x26c0000126c00001
N3_Leveled_MC0_CTL_330 : .dword 0x26c0000126c00001
N3_Leveled_MC0_CTL_340 : .dword 0x26c0000126c00001
N3_Leveled_MC0_CTL_350 : .dword 0x26c0000126c00001
N3_Leveled_MC0_CTL_360 : .dword 0x0800e00026c00001
//00000000000000001100000000000101 phy_ctrl_reg_2(RW) 00000111110000000000001100000001 phy_ctrl_reg_1_8(RD) 
//--------------
N3_Leveled_MC0_CTL_370 : .dword 0x0000000000000000
N3_Leveled_MC0_CTL_380 : .dword 0x0000000000000000
N3_Leveled_MC0_CTL_390 : .dword 0x0000000000000000
N3_Leveled_MC0_CTL_3a0 : .dword 0x0000000000000000
N3_Leveled_MC0_CTL_3b0 : .dword 0x0000000000000000
N3_Leveled_MC0_CTL_3c0 : .dword 0x0000000000000000
N3_Leveled_MC0_CTL_3d0 : .dword 0x0000000000000000
N3_Leveled_MC0_CTL_3e0 : .dword 0x0000000000000000
N3_Leveled_MC0_CTL_3f0 : .dword 0x0000000000000000
N3_Leveled_MC0_CTL_400 : .dword 0x0000000000000000
N3_Leveled_MC0_CTL_410 : .dword 0x0000000000000000
N3_Leveled_MC0_CTL_420 : .dword 0x0000000000000000
N3_Leveled_MC0_CTL_430 : .dword 0x0000000000000000
N3_Leveled_MC0_CTL_440 : .dword 0x0000000000000000
N3_Leveled_MC0_CTL_450 : .dword 0x0000000000000000
N3_Leveled_MC0_CTL_460 : .dword 0x0000000000000000
N3_Leveled_MC0_CTL_470 : .dword 0x0000000000000000
N3_Leveled_MC0_CTL_480 : .dword 0x0000000000000000
N3_Leveled_MC0_CTL_490 : .dword 0x0000000000000000
N3_Leveled_MC0_CTL_4a0 : .dword 0x0000000000000000
N3_Leveled_MC0_CTL_4b0 : .dword 0x0000000000000000
N3_Leveled_MC0_CTL_4c0 : .dword 0x0000000000000000
N3_Leveled_MC0_CTL_4d0 : .dword 0x0000000000000000
N3_Leveled_MC0_CTL_4e0 : .dword 0x0000000000000000
N3_Leveled_MC0_CTL_4f0 : .dword 0x0000000000000000
N3_Leveled_MC0_CTL_500 : .dword 0x0000000000000000
N3_Leveled_MC0_CTL_510 : .dword 0x0000000000000000
N3_Leveled_MC0_CTL_520 : .dword 0x0000000000000000
N3_Leveled_MC0_CTL_530 : .dword 0x0000000000000000
N3_Leveled_MC0_CTL_540 : .dword 0x0000000000000000
N3_Leveled_MC0_CTL_550 : .dword 0x0000000000000000
N3_Leveled_MC0_CTL_560 : .dword 0x0000000000000000
N3_Leveled_MC0_CTL_570 : .dword 0x0000000000000000
N3_Leveled_MC0_CTL_580 : .dword 0x0000000000000000
N3_Leveled_MC0_CTL_590 : .dword 0x0000000000000000
N3_Leveled_MC0_CTL_5a0 : .dword 0x0000000000000000
N3_Leveled_MC0_CTL_5b0 : .dword 0x0000000000000000
N3_Leveled_MC0_CTL_5c0 : .dword 0x0000000000000000
N3_Leveled_MC0_CTL_5d0 : .dword 0x0000000000000000
N3_Leveled_MC0_CTL_5e0 : .dword 0x0000000000000000
N3_Leveled_MC0_CTL_5f0 : .dword 0x0000000000000000
N3_Leveled_MC0_CTL_600 : .dword 0x0000000000000000
N3_Leveled_MC0_CTL_610 : .dword 0x0000000000000000
N3_Leveled_MC0_CTL_620 : .dword 0x0000000000000000
N3_Leveled_MC0_CTL_630 : .dword 0x0000000000000000
N3_Leveled_MC0_CTL_640 : .dword 0x0000000000000000
N3_Leveled_MC0_CTL_650 : .dword 0x0000000000000000
N3_Leveled_MC0_CTL_660 : .dword 0x0000000000000000
N3_Leveled_MC0_CTL_670 : .dword 0x0000000000000000
N3_Leveled_MC0_CTL_680 : .dword 0x0000000000000000
N3_Leveled_MC0_CTL_690 : .dword 0x0000000000000000
N3_Leveled_MC0_CTL_6a0 : .dword 0x0000000000000000
N3_Leveled_MC0_CTL_6b0 : .dword 0x0000000000000000
N3_Leveled_MC0_CTL_6c0 : .dword 0x0000000000000000
N3_Leveled_MC0_CTL_6d0 : .dword 0x0000000000000000
N3_Leveled_MC0_CTL_6e0 : .dword 0x0000000000000000
N3_Leveled_MC0_CTL_6f0 : .dword 0x0000000000000000
N3_Leveled_MC0_CTL_700 : .dword 0x0000000000000000
//-------------
N3_Leveled_MC0_CTL_710 : .dword 0x0000000000000000
//bit 48 en_wr_leveling(RW)
N3_Leveled_MC0_CTL_720 : .dword 0x0000000000000000
//0000000_0 rdlvl_gate_req(WR) 0000000_0 rdlvl_gate_preamble_check_en(RW) 0000000_0 rdlvl_gate_en(RW) 0000000_0 rdlvl_en(RW) 0000000_0 rdlvl_begin_delay_en(RW) 00000000 00000000 0000000_0 swlvl_op_done(RD) 
N3_Leveled_MC0_CTL_730 : .dword 0x0000000000000000
//0000000_0 rdlvl_offset_dir_7(RW) 0000000_0 rdlvl_offset_dir_6(RW) 0000000_0 rdlvl_offset_dir_5(RW)0000000_0 rdlvl_offset_dir_4(RW) 0000000_0 rdlvl_offset_dir_3(RW) 0000000_0 rdlvl_offset_dir_2(RW) 0000000_0 rdlvl_offset_dir_1(RW) 0000000_0 rdlvl_offset_dir_0(RW) 
N3_Leveled_MC0_CTL_740 : .dword 0x0100000000000000
//000000_00 axi1_port_ordering(RW) 000000_00 axi0_port_ordering(RW) 0000000_0 wrlvl_req(WR) 0000000_0 wrlvl_interval_ct_en(RW) 0000000_0 weight_round_robin_weight_sharing(RW) 0000000_0 weight_round_robin_latency_control 0000000_0(RW) rdlvl_req 0000000_0(WR) rdlvl_offset_dir_8(RW)
N3_Leveled_MC0_CTL_750 : .dword 0x0100000101020101
//000000_01 wrlvl_cs(RW) 000000_00 sw_leveling_mode(RW) 000000_00 rdlvl_cs(RW) 000000_01 axi2_w_priority(RW) 000000_01 axi2_r_priority(RW) 000000_10 axi2_port_ordering(RW) 000000_01 axi1_w_priority(RW) 000000_01 axi1_r_priority(RW) 
N3_Leveled_MC0_CTL_760 : .dword 0x0303030000020000
//0000_0011 axi0_priority2_relative_priority(RW) 0000_0011 axi0_priority1_relative_priority(RW) 0000_0011 axi0_priority0_relative_priority(RW) 0000_0000 address_mirroring(RW) 00000_000 tdfi_dram_clk_disable(RW) 00000_010 bstlen(RW) 000000_00 zq_req(WR) 000000_01 zq_on_sref_exit(RW) 
N3_Leveled_MC0_CTL_770 : .dword 0x0101010202020203
//0000_0001 axi2_priority2_relative_priority(RW) 0000_0001 axi2_priority1_relative_priority(RW) 0000_0001 axi2_priority0_relative_priority(RW) 0000_0010 axi1_priority3_relative_priority(RW) 0000_0010 axi1_priority2_relative_priority(RW) 0000_0010 axi1_priority1_relative_priority(RW) 0000_0010 axi1_priority0_relative_priority(RW) 0000_0011 axi0_priority3_relative_priority(RW) 
N3_Leveled_MC0_CTL_780 : .dword 0x0102020400040c01
//0000_0001 tdfi_dram_clk_enable(RW) 0000_0010 tdfi_ctrl_delay(RW) 0000_0010 rdlvl_gate_dq_zero_count(RW) 0000_0100 rdlvl_dq_zero_count(RW) 0000_0000 lowpower_refresh_enable(RW) 0000_0110 dram_class(RW) 0000_1100 burst_on_fly_bit(RW) 0000_0001 axi2_priority3_relative_priority(RW) 
N3_Leveled_MC0_CTL_790 : .dword 0x281900000f000303
//00_101000 wlmrd(RW) 00_011001 wldqsen(RW) 000_00000 lowpower_control(RW) 000_00000 lowpower_auto_enable(RW) 0000_1111 zqcs_chip(RW) 0000_0000 wrr_param_value_err(RD) 0000_0011 tdfi_wrlvl_dll(RW) 0000_0011 tdfi_rdlvl_dll(RW) 
N3_Leveled_MC0_CTL_7a0 : .dword 0x0000000000000000
//_00000000 swlvl_resp_6(RW) _00000000 swlvl_resp_5(RW) _00000000 swlvl_resp_4 _00000000 swlvl_resp_3(RW) _00000000 swlvl_resp_2(RW) _00000000 swlvl_resp_1(RW) _00000000 swlvl_resp_0(RW) _00000000 dfi_wrlvl_max_delay(RW)
N3_Leveled_MC0_CTL_7b0 : .dword 0x0000000000000000
//_00000000 rdlvl_begin_delay_5(RW) _00000000 rdlvl_begin_delay_4(RW) _00000000 rdlvl_begin_delay_3(RW) _00000000 rdlvl_begin_delay_2(RW) _00000000 rdlvl_begin_delay_1(RW) _00000000 rdlvl_begin_delay_0(RW)  _00000000 swlvl_resp_8(RW) _00000000 swlvl_resp_7(RW)
N3_Leveled_MC0_CTL_7c0 : .dword 0x0000000000000000
//_00000000 rdlvl_end_delay_4(RW) _00000000 rdlvl_end_delay_3(RW) _00000000 rdlvl_end_delay_2(RW) _00000000 rdlvl_end_delay_1(RW) _00000000 rdlvl_end_delay_0(RW) _00000000 rdlvl_begin_delay_8(RW) _00000000 rdlvl_begin_delay_7(RW) _00000000 rdlvl_begin_delay_6(RW) 
N3_Leveled_MC0_CTL_7d0 : .dword 0x0000000000000000
//_00000000 rdlvl_gate_clk_adjust_3(RW) _00000000 rdlvl_gate_clk_adjust_2(RW) _00000000 rdlvl_gate_clk_adjust_1(RW) _00000000 rdlvl_gate_clk_adjust_0(RW) _00000000 rdlvl_end_delay_8(RW) _00000000 rdlvl_end_delay_7(RW) _00000000 rdlvl_end_delay_6(RW) 00000000 rdlvl_end_delay_5(RW) 
N3_Leveled_MC0_CTL_7e0 : .dword 0x0000000000000000
//00000000 rdlvl_gate_delay_2(RD) 00000000 rdlvl_gate_delay_1(RD) 00000000 rdlvl_gate_delay_0(RD) 00000000 rdlvl_gate_clk_adjust_8(RW) 00000000 rdlvl_gate_clk_adjust_7(RW) 00000000 rdlvl_gate_clk_adjust_6(RW) 00000000 rdlvl_gate_clk_adjust_5(RW) 00000000 rdlvl_gate_clk_adjust_4(RW) 
N3_Leveled_MC0_CTL_7f0 : .dword 0xff22000000000000
//11111111 rdlvl_max_delay(RW) 00001000 rdlvl_gate_max_delay(RW) 00000000 rdlvl_gate_delay_8(RD) 00000000 rdlvl_gate_delay_7(RD) 00000000 rdlvl_gate_delay_6(RD) 00000000 rdlvl_gate_delay_5(RD) 00000000 rdlvl_gate_delay_4(RD) 00000000 rdlvl_gate_delay_3(RD) 
N3_Leveled_MC0_CTL_800 : .dword 0x0000000000000000
//00000000 rdlvl_midpoint_delay_7(RD) 00000000 rdlvl_midpoint_delay_6(RD) 00000000 rdlvl_midpoint_delay_5(RD) 00000000 rdlvl_midpoint_delay_4(RD) 00000000 rdlvl_midpoint_delay_3(RD) 00000000 rdlvl_midpoint_delay_2(RD) 00000000 rdlvl_midpoint_delay_1(RD) 00000000 rdlvl_midpoint_delay_0(RD) 
N3_Leveled_MC0_CTL_810 : .dword 0x0000000000000000
//00000000 rdlvl_offset_delay_6(RW) 00000000 rdlvl_offset_delay_5(RW) 00000000 rdlvl_offset_delay_4(RW) 00000000 rdlvl_offset_delay_3(RW) 00000000 rdlvl_offset_delay_2(RW) 00000000 rdlvl_offset_delay_1(RW) 00000000 rdlvl_offset_delay_0(RW) 00000000 rdlvl_midpoint_delay_8(RD) 
N3_Leveled_MC0_CTL_820 : .dword 0x0400000c00400000
//00000100 tdfi_wrlvl_resplat(RW) 00000000 tdfi_wrlvl_resp(RD) 00000000 tdfi_rdlvl_rr(RW) 00001100 tdfi_rdlvl_resplat(RW) 00000000 tdfi_rdlvl_resp(RD) 01000000 refresh_per_zq(RW) 00000000 rdlvl_offset_delay_8(RW) 00000000 rdlvl_offset_delay_7(RW) 
N3_Leveled_MC0_CTL_830 : .dword 0x0000000000000500
//00000000 wrlvl_delay_5(RD) 00000000 wrlvl_delay_4(RD) 00000000 wrlvl_delay_3(RD) 00000000 wrlvl_delay_2(RD) 00000000 wrlvl_delay_1(RD) 00000000 wrlvl_delay_0(RD) 00000010 tmod(RW) 00000000 tdfi_wrlvl_ww(RW) 
N3_Leveled_MC0_CTL_840 : .dword 0x0000640064000000
//00000000000000_0001100100 axi1_priority_relax(RW) 000000_0001100100 axi0_priority_relax(RW) 00000000 wrlvl_delay_8(RD) 00000000 wrlvl_delay_7(RD) 00000000 wrlvl_delay_6(RD) 
N3_Leveled_MC0_CTL_850 : .dword 0x0000000000000064
//000000_0000000000 out_of_range_source_id(RD) 000000_0000000000 ecc_u_id(RD) 000000_0000000000 ecc_c_id(RD) 000000_0000000000 axi2_priority_relax(RW)
N3_Leveled_MC0_CTL_860 : .dword 0x0000000000000000
//0000_000000000000 zqini(RW) 0000_000000000000 zqcs(RW) 000000_0000000000 port_data_error_id(RD) 000000_0000000000 port_cmd_error_id(RD)
N3_Leveled_MC0_CTL_870 : .dword 0x0000000000000000
//0_000000000000010 emrs1_data_3(RD) 0_000000000000010 emrs1_data_2(RD) 0_000000000000010 emrs1_data_1(RD) 0_000000000000010 emrs1_data_0(RD) 
N3_Leveled_MC0_CTL_880 : .dword 0x0000000000000000
//0_000000000000010 emrs3_data_3(RW) 0_000000000000010 emrs3_data_2(RW) 0_000000000000010 emrs3_data_1(RW) 0_000000000000010 emrs3_data_0(RW) 
N3_Leveled_MC0_CTL_890 : .dword 0x0000000000000000
//0_000010000010000 mrs_data_3(RD) 0_000010000010000 mrs_data_2(RD) 0_000010000010000 mrs_data_1(RD) 0_000010000010000 mrs_data_0(RD) 
N3_Leveled_MC0_CTL_8a0 : .dword 0x00000000001c001c
//hXXXX lowpower_internal_cnt(RW) hXXXX lowpower_external_cnt(RW) hXXXX axi2_en_size_lt_width_instr(RW) hXXXX axi1_en_size_lt_width_instr(RW)
N3_Leveled_MC0_CTL_8b0 : .dword 0x0000000000000000
//hXXXX refresh_per_rdlvl(RW) hXXXX lowpower_self_refresh_cnt(RW) hXXXX lowpower_refresh_hold(RW) hXXXX lowpower_power_down_cnt(RW)
N3_Leveled_MC0_CTL_8c0 : .dword 0x0000000000000000
//hXXXX wrlvl_interval(RW) hXXXX tdfi_wrlvl_max(RW) hXXXX tdfi_rdlvl_max(RW) hXXXX refresh_per_rdlvl_gate(RW)
N3_Leveled_MC0_CTL_8d0 : .dword 0x002faf0800000000
//h00_XXXXXXXX cke_inactive(RW) 000000_000000000000000000 wrlvl_status(RD)
N3_Leveled_MC0_CTL_8e0 : .dword 0x0000000023c34600
//h00000000_XXXXXXXX trst_pwron(RW)
N3_Leveled_MC0_CTL_8f0 : .dword 0x0000000040404080
//hXXXXXXX 000_0 XXXXXXXX dll_ctrl_reg_2(RW) 
N3_Leveled_MC0_CTL_900 : .dword 0x0000000000000000
//h000000 00_00 X XXXXXXXX rdlvl_error_status(RW)
N3_Leveled_MC0_CTL_910 : .dword 0x0000000000000000
//hXXXXXXXX XXXXXXXX rdlvl_gate_resp_mask[63:0](RW)
N3_Leveled_MC0_CTL_920 : .dword 0x0000000000000000
//h00000000000000_XX rdlvl_gate_resp_mask[71:64](RW)
N3_Leveled_MC0_CTL_930 : .dword 0x0000000000000000
//hXXXXXXXX XXXXXXXX rdlvl_resp_mask[63:0](RW)
N3_Leveled_MC0_CTL_940 : .dword 0x0006060000050500
//0000_0000 tdfi_rdlvl_en(RW) 00000_000 w2r_samecs_delay(RW) 00000_000 w2r_diffcs_delay(RW) 00000_000 lvl_status(RD) 0000000_0 rdlvl_edge(RW) 0000_0000 cksrx(RW) 0000_0000 cksre(RW) _00000000 rdlvl_resp_mask[71:64](RW)
N3_Leveled_MC0_CTL_950 : .dword 0x0000000000001000
//hXXXXX 00_00 XXXX mask_int[17:0](RW) hXXXX txpdll(RW) 0000_0000 tdfi_wrlvl_en(RW)
N3_Leveled_MC0_CTL_960 : .dword 0x0604000000000000
//000_00101 rdlat_adj(RW) 0000_0100 wrlat_adj(RW) 0000000_0 swlvl_start(WR) 0000000_0 swlvl_load(WR) 0000000_0 swlvl_exit(WR) 000000_000000000000000000 int_status(RD) 
N3_Leveled_MC0_CTL_970 : .dword 0x0000000000004810
//h00000 000_0 XXXX int_ack[16:0](WR) hXXXX dll_rst_delay(RW) hXX dll_rst_adj_dly(RW)
N3_Leveled_MC0_CTL_980 : .dword 0x0001010001000101
//0000000_0 zq_in_progress(RD) 0000000_1 zqcs_rotate(RW) 0000000_0 wrlvl_reg_en(RW) 0000000_0 wrlvl_en(RW) 0000000_1 resync_dll_per_aref_en(RW) 0000000_0 resync_dll(WR) 0000000_0 rdlvl_reg_en(RW) 0000000_0 rdlvl_gate_reg_en(RW) 
N3_Leveled_MC0_CTL_990 : .dword 0x0606060606060600
//00000_000 w2w_samecs_dly(RW) 00000_001 w2w_diffcs_dly(RW) 00000_010 tbst_int_interval(RW) 00000_010 r2w_samecs_dly(RW) 00000_010 r2w_diffcs_dly(RW) 00000_000 r2r_samecs_dly(RW) 00000_001 r2r_diffcs_dly(RW) 00000_000 axi_aligned_strobe_disable(RW) 
N3_Leveled_MC0_CTL_9a0 : .dword 0x0707040200070106
//00000111 tdfi_wrlvl_load(RW) 00000111 tdfi_rdlvl_load(RW) 000_00011 tckesr(RW) 000_00010 tccd(RW) 000_00000 add_odt_clk_difftype_diffcs(RW) 0000_0110 trp_ab(RW) 0000_0001 add_odt_clk_sametype_diffcs(RW) 0000_0000 add_odt_clk_difftype_samecs(RW) 
N3_Leveled_MC0_CTL_9b0 : .dword 0x02000100000a000f
//0000_001000000000 zqinit(RW) 0000_000100000000 zqcl(RW) 000000_0000001010 tdfi_wrlvl_ww(RW) 000000_0000001111 tdfi_rdlvl_rr(RW) 
N3_Leveled_MC0_CTL_9c0 : .dword 0x0a520c2d0c2d0c2d
//0_000101001010010 mr0_data_0(RW) 00_00110000101101 tdfi_phyupd_type3(RW) 00_00110000101101 tdfi_phyupd_type2(RW) 00_00110000101101 tdfi_phyupd_type1(RW) 
N3_Leveled_MC0_CTL_9d0 : .dword 0x00440a520a520a52
//0_000000000000100 mr1_data_0(RW) 0_000101001010010 mr0_data_3(RW) 0_000101001010010 mr0_data_2(RW) 0_000101001010010 mr0_data_1(RW) 
N3_Leveled_MC0_CTL_9e0 : .dword 0x0000004400440044
//0_000000000000000 mr2_data_0(RW) 0_000000000000100 mr1_data_3(RW) 0_000000000000100 mr1_data_2(RW) 0_000000000000100 mr1_data_1(RW) 
N3_Leveled_MC0_CTL_9f0 : .dword 0x0000000000000000
//0_000000000000000 mr3_data_0(RW) 0_000000000000000 mr2_data_3(RW) 0_000000000000000 mr2_data_2(RW) 0_000000000000000 mr2_data_1(RW) 
N3_Leveled_MC0_CTL_a00 : .dword 0x00ff000000000000
//0000000011111111 dfi_wrlvl_max_delay(RW) 0_000000000000000 mr3_data_3(RW) 0_000000000000000 mr3_data_2(RW) 0_000000000000000 mr3_data_1(RW) 
N3_Leveled_MC0_CTL_a10 : .dword 0x0000000000000000
//0000000000000000 rdlvl_begin_delay_3(RD) 0000000000000000 rdlvl_begin_delay_2(RD) 0000000000000000 rdlvl_begin_delay_1(RD) 0000000000000000 rdlvl_begin_delay_0(RD) 
N3_Leveled_MC0_CTL_a20 : .dword 0x0000000000000000
//0000000000000000 rdlvl_begin_delay_7(RD) 0000000000000000 rdlvl_begin_delay_6(RD) 0000000000000000 rdlvl_begin_delay_5(RD) 0000000000000000 rdlvl_begin_delay_4(RD) 
N3_Leveled_MC0_CTL_a30 : .dword 0x0020002000200000
//0000111000001110 rdlvl_delay_2(RW) 0000111000001110 rdlvl_delay_1(RW) 0000111000001110 rdlvl_delay_0(RW) 0000000000000000 rdlvl_begin_delay_8(RD) 
N3_Leveled_MC0_CTL_a40 : .dword 0x0020002000200020
//0000111000001110 rdlvl_delay_6(RW) 0000111000001110 rdlvl_delay_5(RW) 0000111000001110 rdlvl_delay_4(RW) 0000111000001110 rdlvl_delay_3(RW) 
N3_Leveled_MC0_CTL_a50 : .dword 0x0000000000200020
//0000000000000000 rdlvl_end_delay_1(RD) 0000000000000000 rdlvl_end_delay_0(RD) 0000111000001110 rdlvl_delay_8(RW) 0000111000001110 rdlvl_delay_7(RW) 
N3_Leveled_MC0_CTL_a60 : .dword 0x0000000000000000
//0000000000000000 rdlvl_end_delay_5(RD) 0000000000000000 rdlvl_end_delay_4(RD) 0000000000000000 rdlvl_end_delay_3(RD) 0000000000000000 rdlvl_end_delay_2(RD) 
N3_Leveled_MC0_CTL_a70 : .dword 0x0000000000000000
//0000000000000000 rdlvl_gate_delay_0(RW+) 0000000000000000 rdlvl_end_delay_8(RD) 0000000000000000 rdlvl_end_delay_7(RD) 0000000000000000 rdlvl_end_delay_6(RD) 
N3_Leveled_MC0_CTL_a80 : .dword 0x0000000000000000
//0000000000000000 rdlvl_gate_delay_4(RW+) 0000000000000000 rdlvl_gate_delay_3(RW+) 0000000000000000 rdlvl_gate_delay_2(RW+) 0000000000000000 rdlvl_gate_delay_1(RW+) 
N3_Leveled_MC0_CTL_a90 : .dword 0x0000000000000000
//0000000000000000 rdlvl_gate_delay_8(RW+) 0000000000000000 rdlvl_gate_delay_7(RW+) 0000000000000000 rdlvl_gate_delay_6(RW+) 0000000000000000 rdlvl_gate_delay_5(RW+) 
N3_Leveled_MC0_CTL_aa0 : .dword 0x0000ffff00000010
//0000000000000000 rdlvl_midpoint_delay_0(RD) 1111111111111111 rdlvl_max_delay(RW) 0000000000000000 rdlvl_gate_refresh_interval(RW) 0000000000010000 rdlvl_gate_max_delay(RW) 
N3_Leveled_MC0_CTL_ab0 : .dword 0x0000000000000000
//0000000000000000 rdlvl_midpoint_delay_4(RD) 0000000000000000 rdlvl_midpoint_delay_3(RD) 0000000000000000 rdlvl_midpoint_delay_2(RD) 0000000000000000 rdlvl_midpoint_delay_1(RD) 
N3_Leveled_MC0_CTL_ac0 : .dword 0x0000000000000000
//0000000000000000 rdlvl_midpoint_delay_8(RD) 0000000000000000 rdlvl_midpoint_delay_7(RD) 0000000000000000 rdlvl_midpoint_delay_6(RD) 0000000000000000 rdlvl_midpoint_delay_5(RD) 
N3_Leveled_MC0_CTL_ad0 : .dword 0x0000000000000000
//0000000000000000 rdlvl_offset_delay_3(RW) 0000000000000000 rdlvl_offset_delay_2(RW) 0000000000000000 rdlvl_offset_delay_1(RW) 0000000000000000 rdlvl_offset_delay_0(RW) 
N3_Leveled_MC0_CTL_ae0 : .dword 0x0000000000000000
//0000000000000000 rdlvl_offset_delay_7(RW) 0000000000000000 rdlvl_offset_delay_6(RW) 0000000000000000 rdlvl_offset_delay_5(RW) 0000000000000000 rdlvl_offset_delay_4(RW) 
N3_Leveled_MC0_CTL_af0 : .dword 0x0040004000000000
//0000000000000000 wrlvl_delay_1(RW+) 0000000000000000 wrlvl_delay_0(RW+) 0000000000000000 rdlvl_refresh_interval(RW) 0000000000000000 rdlvl_offset_delay_8(RW) 
N3_Leveled_MC0_CTL_b00 : .dword 0x0040004000400040
//0000000000000000 wrlvl_delay_5(RW+) 0000000000000000 wrlvl_delay_4(RW+) 0000000000000000 wrlvl_delay_3(RW+) 0000000000000000 wrlvl_delay_2(RW+) 
N3_Leveled_MC0_CTL_b10 : .dword 0x0000004000400040
//0000000000000000 wrlvl_refresh_interval(RW) 0000000000000000 wrlvl_delay_8(RW+) 0000000000000000 wrlvl_delay_7(RW+) 0000000000000000 wrlvl_delay_6(RW+) 
N3_Leveled_MC0_CTL_b20 : .dword 0x00000c2d00000c2d
//00000000000000000000110000101101 tdfi_rdlvl_resp(RW) 00000000000000000000110000101101 tdfi_rdlvl_max(RW) 
N3_Leveled_MC0_CTL_b30 : .dword 0x00000c2d00000000
//00000000000000000000110000101101 tdfi_wrlvl_resp(RW) 00000000000000000000000000000000 tdfi_wrlvl_max(RW) 

    .align  12
