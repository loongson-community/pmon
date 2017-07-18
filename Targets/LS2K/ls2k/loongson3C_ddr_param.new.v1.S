
//DDR2 param
    .align 5
ddr2_reg_data:
ddr2_reg_data_mc1:

ddr2_RDIMM_reg_data:
ddr2_RDIMM_reg_data_mc1:

//DDR3 param
    .align  5
ddr3_reg_data:
ddr3_reg_data_mc1:
MC1_DDR3_CTRL_0x000: .dword 0x0000000000000000
//XXXX pm_dll_value_0(RD) XXXX pm_dll_value_ck(RD) XXXX pm_dll_init_done(RD) XXXX pm_version(RD)
MC1_DDR3_CTRL_0x008: .dword 0x0000000000000000
//XXXX pm_dll_value_4(RD) XXXX pm_dll_value_3 (RD) XXXX pm_dll_value_2(RD) XXXX pm_dll_value_1(RD)
MC1_DDR3_CTRL_0x010: .dword 0x0000000000000000
//XXXX pm_dll_value_8(RD) XXXX pm_dll_value_7 (RD) XXXX pm_dll_value_6(RD) XXXX pm_dll_value_5(RD)
MC1_DDR3_CTRL_0x018: .dword 0x8080808016600100
//_00000000 pm_dll_ck_3 _00000000 pm_dll_ck_2 _00000000 pm_dll_ck_1 _00000000 pm_dll_ck_0 _00000000 pm_dll_increment _00000000 pm_dll_start_point _0000000_0 pm_dll_bypass _0000000_0 pm_init_start
MC1_DDR3_CTRL_0x020: .dword 0x0202000000000100
//0000_0000 pm_dq_oe_end_0 0000_0000 pm_dq_oe_begin_0 000000_00 pm_dq_stop_edge_0 000000_00 pm_dq_start_edge_0 0000000_0 pm_rddata_delay_0 0000000_0 pm_rddqs_lt_half_0 0000000_0 pm_wrdqs_lt_half_0 0000000_0 pm_wrdq_lt_half_0
MC1_DDR3_CTRL_0x028: .dword 0x0000000002010101
//0000_0000 pm_rd_oe_end_0 0000_0000 pm_rd_oe_begin_0 000000_00 pm_rd_stop_edge_0 000000_00 pm_rd_start_edge_0 0000_0000 pm_dqs_oe_end_0 0000_0000 pm_dqs_oe_begin_0 000000_00 pm_dqs_stop_edge_0 000000_00 pm_dqs_start_edge_0
MC1_DDR3_CTRL_0x030: .dword 0x0000000101000301
//hXXXXXX (RD) 0000000_0 pm_wrdq_clkdelay_0 0000_0000 pm_odt_oe_end_0 0000_0000 pm_odt_oe_begin_0 000000_00 pm_odt_stop_edge_0 000000_00 pm_odt_start_edge_0
MC1_DDR3_CTRL_0x038: .dword 0x0000002020056500
//hXXXXXX (RD) _00000000 pm_dll_rddqs_n_0 _00000000 pm_dll_rddqs_p_0 _00000000 pm_dll_wrdqs_0 _00000000 pm_dll_wrdata_0 _00000000 pm_dll_gate_0
MC1_DDR3_CTRL_0x040: .dword 0x0202000000000100
//0000_0000 pm_dq_oe_end_1 0000_0000 pm_dq_oe_begin_1 000000_00 pm_dq_stop_edge_1 000000_00 pm_dq_start_edge_1 0000000_0 pm_rddata_delay_1 0000000_0 pm_rddqs_lt_half_1 0000000_0 pm_wrdqs_lt_half_1 0000000_0 pm_wrdq_lt_half_1
MC1_DDR3_CTRL_0x048: .dword 0x0000000002010101
//0000_0000 pm_rd_oe_end_1 0000_0000 pm_rd_oe_begin_1 000000_00 pm_rd_stop_edge_1 000000_00 pm_rd_start_edge_1 0000_0000 pm_dqs_oe_end_1 0000_0000 pm_dqs_oe_begin_1 000000_00 pm_dqs_stop_edge_1 000000_00 pm_dqs_start_edge_1
MC1_DDR3_CTRL_0x050: .dword 0x0000000101000301
//hXXXXXX (RD) 0000000_0 pm_wrdq_clkdelay_1 0000_0000 pm_odt_oe_end_1 0000_0000 pm_odt_oe_begin_1 000000_00 pm_odt_stop_edge_1 000000_00 pm_odt_start_edge_1
MC1_DDR3_CTRL_0x058: .dword 0x0000002020056500
//hXXXXXX (RD) _00000000 pm_dll_rddqs_n_1 _00000000 pm_dll_rddqs_p_1 _00000000 pm_dll_wrdqs_1 _00000000 pm_dll_wrdata_1 _00000000 pm_dll_gate_1
MC1_DDR3_CTRL_0x060: .dword 0x0202000000000100
//0000_0000 pm_dq_oe_end_2 0000_0000 pm_dq_oe_begin_2 000000_00 pm_dq_stop_edge_2 000000_00 pm_dq_start_edge_2 0000000_0 pm_rddata_delay_2 0000000_0 pm_rddqs_lt_half_2 0000000_0 pm_wrdqs_lt_half_2 0000000_0 pm_wrdq_lt_half_2
MC1_DDR3_CTRL_0x068: .dword 0x0000000002010101
//0000_0000 pm_rd_oe_end_2 0000_0000 pm_rd_oe_begin_2 000000_00 pm_rd_stop_edge_2 000000_00 pm_rd_start_edge_2 0000_0000 pm_dqs_oe_end_2 0000_0000 pm_dqs_oe_begin_2 000000_00 pm_dqs_stop_edge_2 000000_00 pm_dqs_start_edge_2
MC1_DDR3_CTRL_0x070: .dword 0x0000000101000301
//hXXXXXX (RD) 0000000_0 pm_wrdq_clkdelay_2 0000_0000 pm_odt_oe_end_2 0000_0000 pm_odt_oe_begin_2 000000_00 pm_odt_stop_edge_2 000000_00 pm_odt_start_edge_2
MC1_DDR3_CTRL_0x078: .dword 0x0000002020056500
//hXXXXXX (RD) _00000000 pm_dll_rddqs_n_2 _00000000 pm_dll_rddqs_p_2 _00000000 pm_dll_wrdqs_2 _00000000 pm_dll_wrdata_2 _00000000 pm_dll_gate_2
MC1_DDR3_CTRL_0x080: .dword 0x0202000000000100
//0000_0000 pm_dq_oe_end_3 0000_0000 pm_dq_oe_begin_3 000000_00 pm_dq_stop_edge_3 000000_00 pm_dq_start_edge_3 0000000_0 pm_rddata_delay_3 0000000_0 pm_rddqs_lt_half_3 0000000_0 pm_wrdqs_lt_half_3 0000000_0 pm_wrdq_lt_half_3
MC1_DDR3_CTRL_0x088: .dword 0x0000000002010101
//0000_0000 pm_rd_oe_end_3 0000_0000 pm_rd_oe_begin_3 000000_00 pm_rd_stop_edge_3 000000_00 pm_rd_start_edge_3 0000_0000 pm_dqs_oe_end_3 0000_0000 pm_dqs_oe_begin_3 000000_00 pm_dqs_stop_edge_3 000000_00 pm_dqs_start_edge_3
MC1_DDR3_CTRL_0x090: .dword 0x0000000101000301
//hXXXXXX (RD) 0000000_0 pm_wrdq_clkdelay_3 0000_0000 pm_odt_oe_end_3 0000_0000 pm_odt_oe_begin_3 000000_00 pm_odt_stop_edge_3 000000_00 pm_odt_start_edge_3
MC1_DDR3_CTRL_0x098: .dword 0x0000002020056500
//hXXXXXX (RD) _00000000 pm_dll_rddqs_n_3 _00000000 pm_dll_rddqs_p_3 _00000000 pm_dll_wrdqs_3 _00000000 pm_dll_wrdata_3 _00000000 pm_dll_gate_3
MC1_DDR3_CTRL_0x0a0: .dword 0x0202000000000100
//0000_0000 pm_dq_oe_end_4 0000_0000 pm_dq_oe_begin_4 000000_00 pm_dq_stop_edge_4 000000_00 pm_dq_start_edge_4 0000000_0 pm_rddata_delay_4 0000000_0 pm_rddqs_lt_half_4 0000000_0 pm_wrdqs_lt_half_4 0000000_0 pm_wrdq_lt_half_4
MC1_DDR3_CTRL_0x0a8: .dword 0x0000000002010101
//0000_0000 pm_rd_oe_end_4 0000_0000 pm_rd_oe_begin_4 000000_00 pm_rd_stop_edge_4 000000_00 pm_rd_start_edge_4 0000_0000 pm_dqs_oe_end_4 0000_0000 pm_dqs_oe_begin_4 000000_00 pm_dqs_stop_edge_4 000000_00 pm_dqs_start_edge_4
MC1_DDR3_CTRL_0x0b0: .dword 0x0000000101000301
//hXXXXXX (RD) 0000000_0 pm_wrdq_clkdelay_4 0000_0000 pm_odt_oe_end_4 0000_0000 pm_odt_oe_begin_4 000000_00 pm_odt_stop_edge_4 000000_00 pm_odt_start_edge_4
MC1_DDR3_CTRL_0x0b8: .dword 0x0000002020056500
//hXXXXXX (RD) _00000000 pm_dll_rddqs_n_4 _00000000 pm_dll_rddqs_p_4 _00000000 pm_dll_wrdqs_4 _00000000 pm_dll_wrdata_4 _00000000 pm_dll_gate_4
MC1_DDR3_CTRL_0x0c0: .dword 0x0202000000000100
//0000_0000 pm_dq_oe_end_5 0000_0000 pm_dq_oe_begin_5 000000_00 pm_dq_stop_edge_5 000000_00 pm_dq_start_edge_5 0000000_0 pm_rddata_delay_5 0000000_0 pm_rddqs_lt_half_5 0000000_0 pm_wrdqs_lt_half_5 0000000_0 pm_wrdq_lt_half_5
MC1_DDR3_CTRL_0x0c8: .dword 0x0000000002010101
//0000_0000 pm_rd_oe_end_5 0000_0000 pm_rd_oe_begin_5 000000_00 pm_rd_stop_edge_5 000000_00 pm_rd_start_edge_5 0000_0000 pm_dqs_oe_end_5 0000_0000 pm_dqs_oe_begin_5 000000_00 pm_dqs_stop_edge_5 000000_00 pm_dqs_start_edge_5
MC1_DDR3_CTRL_0x0d0: .dword 0x0000000101000301
//hXXXXXX (RD) 0000000_0 pm_wrdq_clkdelay_5 0000_0000 pm_odt_oe_end_5 0000_0000 pm_odt_oe_begin_5 000000_00 pm_odt_stop_edge_5 000000_00 pm_odt_start_edge_5
MC1_DDR3_CTRL_0x0d8: .dword 0x0000002020056500
//hXXXXXX (RD) _00000000 pm_dll_rddqs_n_5 _00000000 pm_dll_rddqs_p_5 _00000000 pm_dll_wrdqs_5 _00000000 pm_dll_wrdata_5 _00000000 pm_dll_gate_5
MC1_DDR3_CTRL_0x0e0: .dword 0x0202000000000100
//0000_0000 pm_dq_oe_end_6 0000_0000 pm_dq_oe_begin_6 000000_00 pm_dq_stop_edge_6 000000_00 pm_dq_start_edge_6 0000000_0 pm_rddata_delay_6 0000000_0 pm_rddqs_lt_half_6 0000000_0 pm_wrdqs_lt_half_6 0000000_0 pm_wrdq_lt_half_6
MC1_DDR3_CTRL_0x0e8: .dword 0x0000000002010101
//0000_0000 pm_rd_oe_end_6 0000_0000 pm_rd_oe_begin_6 000000_00 pm_rd_stop_edge_6 000000_00 pm_rd_start_edge_6 0000_0000 pm_dqs_oe_end_6 0000_0000 pm_dqs_oe_begin_6 000000_00 pm_dqs_stop_edge_6 000000_00 pm_dqs_start_edge_6
MC1_DDR3_CTRL_0x0f0: .dword 0x0000000101000301
//hXXXXXX (RD) 0000000_0 pm_wrdq_clkdelay_6 0000_0000 pm_odt_oe_end_6 0000_0000 pm_odt_oe_begin_6 000000_00 pm_odt_stop_edge_6 000000_00 pm_odt_start_edge_6
MC1_DDR3_CTRL_0x0f8: .dword 0x0000002020056500
//hXXXXXX (RD) _00000000 pm_dll_rddqs_n_6 _00000000 pm_dll_rddqs_p_6 _00000000 pm_dll_wrdqs_6 _00000000 pm_dll_wrdata_6 _00000000 pm_dll_gate_6
MC1_DDR3_CTRL_0x100: .dword 0x0202000000000100
//0000_0000 pm_dq_oe_end_7 0000_0000 pm_dq_oe_begin_7 000000_00 pm_dq_stop_edge_7 000000_00 pm_dq_start_edge_7 0000000_0 pm_rddata_delay_7 0000000_0 pm_rddqs_lt_half_7 0000000_0 pm_wrdqs_lt_half_7 0000000_0 pm_wrdq_lt_half_7
MC1_DDR3_CTRL_0x108: .dword 0x0000000002010101
//0000_0000 pm_rd_oe_end_7 0000_0000 pm_rd_oe_begin_7 000000_00 pm_rd_stop_edge_7 000000_00 pm_rd_start_edge_7 0000_0000 pm_dqs_oe_end_7 0000_0000 pm_dqs_oe_begin_7 000000_00 pm_dqs_stop_edge_7 000000_00 pm_dqs_start_edge_7
MC1_DDR3_CTRL_0x110: .dword 0x0000000101000301
//hXXXXXX (RD) 0000000_0 pm_wrdq_clkdelay_7 0000_0000 pm_odt_oe_end_7 0000_0000 pm_odt_oe_begin_7 000000_00 pm_odt_stop_edge_7 000000_00 pm_odt_start_edge_7
MC1_DDR3_CTRL_0x118: .dword 0x0000002020056500
//hXXXXXX (RD) _00000000 pm_dll_rddqs_n_7 _00000000 pm_dll_rddqs_p_7 _00000000 pm_dll_wrdqs_7 _00000000 pm_dll_wrdata_7 _00000000 pm_dll_gate_7
MC1_DDR3_CTRL_0x120: .dword 0x0202000000000100
//0000_0000 pm_dq_oe_end_8 0000_0000 pm_dq_oe_begin_8 000000_00 pm_dq_stop_edge_8 000000_00 pm_dq_start_edge_8 0000000_0 pm_rddata_delay_8 0000000_0 pm_rddqs_lt_half_8 0000000_0 pm_wrdqs_lt_half_8 0000000_0 pm_wrdq_lt_half_8
MC1_DDR3_CTRL_0x128: .dword 0x0101000002010101
//0000_0000 pm_rd_oe_end_8 0000_0000 pm_rd_oe_begin_8 000000_00 pm_rd_stop_edge_8 000000_00 pm_rd_start_edge_8 0000_0000 pm_dqs_oe_end_8 0000_0000 pm_dqs_oe_begin_8 000000_00 pm_dqs_stop_edge_8 000000_00 pm_dqs_start_edge_8
MC1_DDR3_CTRL_0x130: .dword 0x0000000001000301
//hXXXXXX (RD) 0000000_0 pm_wrdq_clkdelay_8 0000_0000 pm_odt_oe_end_8 0000_0000 pm_odt_oe_begin_8 000000_00 pm_odt_stop_edge_8 000000_00 pm_odt_start_edge_8
MC1_DDR3_CTRL_0x138: .dword 0x00000020207f6000
//hXXXXXX (RD) _00000000 pm_dll_rddqs_n_8 _00000000 pm_dll_rddqs_p_8 _00000000 pm_dll_wrdqs_8 _00000000 pm_dll_wrdata_8 _00000000 pm_dll_gate_8
MC1_DDR3_CTRL_0x140: .dword 0x0403000001ff01ff
//00000_000 pm_pad_ocd_clk 00000_000 pm_pad_ocd_ctl 0000000_0 pm_pad_ocd_dqs 0000000_0 pm_pad_ocd_dq 0000000_0_00000000 pm_pad_enzi 0000000_0 pm_pad_en_ctl _00000000 pm_pad_en_clk
MC1_DDR3_CTRL_0x148: .dword 0x0000000000010100
//_0000 pm_pad_adj_ncode_dq _0000 pm_pad_adj_pcode_dq 00000 _0 pm_pad_outodt_dq _0 pm_pad_slewrate_dq _0 pm_pad_code_dq _0000 pm_pad_adj_ncode_dq _0000 pm_pad_adj_pcode_dq 00000 _0 pm_pad_outodt_dq _0 pm_pad_slewrate_dq _0 00000000 0000000_0 pm_pad_vref_internal 0000000_0 pm_pad_odt_se 0000000_0 pm_pad_modezi1v8
MC1_DDR3_CTRL_0x150: .dword 0x00000000f0020000
//hXXXX _0000 pm_pad_adj_ncode_clk _0000 pm_pad_adj_pcode_clk 00000 _0 pm_pad_outodt_clk _0 pm_pad_slewrate_clk _0 pm_pad_code_clk _0000 pm_pad_adj_ncode_cmd _0000 pm_pad_adj_pcode_cmd 00000 _0 pm_pad_outodt_cmd _0 pm_pad_slewrate_cmd _0 pm_pad_code_cmd _0000 pm_pad_adj_ncode_addr _0000 pm_pad_adj_pcode_addr 00000 _0 pm_pad_outodt_addr _0 pm_pad_slewrate_addr _0 pm_pad_code_addr
MC1_DDR3_CTRL_0x158: .dword 0x00000000f0000000
//hXXXXXXXX (RD) _0000 pm_pad_comp_ncode_i _0000 pm_pad_comp_pcode_i 0000000_0 pm_pad_comp_mode 0000000_0 pm_pad_comp_tm 0000000_0 pm_pad_comp_pd
MC1_DDR3_CTRL_0x160: .dword 0x0000000000010001
//0000000_0_00000000 pm_rdfifo_empty(RD) 0000000_0_00000000 pm_overflow(RD) 0000_0000 pm_dram_init(RD) 0000000_0 pm_rdfifo_valid 000000_00 pm_cmd_timing 0000000_0 pm_ddr3_mode
MC1_DDR3_CTRL_0x168: .dword 0x000a000700030101
//000_00000 pm_bank_xor(OLD) 0000_0000 pm_addr_mirror 000000_00 pm_cmd_delay _00000000 pm_burst_length 00000_000 pm_bank 0000_0000 pm_cs_zq 0000_0000 pm_cs_mrs 0000_0000 pm_cs_enable
MC1_DDR3_CTRL_0x170: .dword 0x8421050084120501
//_00000000_00000000 pm_odt_wr_cs_map 0000_0000 pm_odt_wr_length 0000_0000 pm_odt_wr_delay _00000000_00000000 pm_odt_rd_cs_map 0000_0000 pm_odt_rd_length 0000_0000 pm_odt_rd_delay
MC1_DDR3_CTRL_0x178: .dword 0x0000000000000000
//hXXXXXXXXXXXXXXXX (RD)
MC1_DDR3_CTRL_0x180: .dword 0x0000000001100000
//_00000000 pm_lvl_resp_0(RD) 0000000_0 pm_lvl_done(RD) 0000000_0 pm_lvl_ready(RD) 00000000 0000_0000 pm_lvl_cs _00000000 pm_tLVL_DELAY 0000000_0 pm_leveling_req(WR) 000000_00 pm_leveling_mode
MC1_DDR3_CTRL_0x188: .dword 0x0000000000000000
//_00000000 pm_lvl_resp_8(RD) _00000000 pm_lvl_resp_7(RD) _00000000 _pm_lvl_resp_6(RD) _00000000 pm_lvl_resp_5(RD) _00000000 pm_lvl_resp_4(RD) _00000000 pm_lvl_resp_3(RD) _00000000 pm_lvl_resp_2(RD) _00000000 pm_lvl_resp_1(RD)

//CMD CONFIG
MC1_DDR3_CTRL_0x190: .dword 0x0000000000000000
//_00000000_00000000 pm_cmd_a 00000_000 pm_cmd_ba 00000_000 pm_cmd_cmd 0000_0000 pm_cmd_cs 0000000_0 pm_status_cmd(RD) 0000000_0 pm_cmd_req(WR) 0000000_0 pm_command
MC1_DDR3_CTRL_0x198: .dword 0x0000000000000000
//00000000 00000000 0000_0000 pm_status_sref(RD) 0000_0000 pm_srefresh_req 0000000_0 pm_pre_all_done(RD) 0000000_0 pm_pre_all_req(WR) 0000000_0 pm_mrs_done(RD) 0000000_0 pm_mrs_req(WR)
MC1_DDR3_CTRL_0x1a0: .dword 0x0000001800060d60
//_00000000_00000000 pm_mr_3_cs_0 _00000000_00000000 pm_mr_2_cs_0 _00000000_00000000 pm_mr_1_cs_0 _00000000_00000000 pm_mr_0_cs_0
MC1_DDR3_CTRL_0x1a8: .dword 0x0000001800060d60
//_00000000_00000000 pm_mr_3_cs_1 _00000000_00000000 pm_mr_2_cs_1 _00000000_00000000 pm_mr_1_cs_1 _00000000_00000000 pm_mr_0_cs_1
MC1_DDR3_CTRL_0x1b0: .dword 0x0000001800060d60
//_00000000_00000000 pm_mr_3_cs_2 _00000000_00000000 pm_mr_2_cs_2 _00000000_00000000 pm_mr_1_cs_2 _00000000_00000000 pm_mr_0_cs_2
MC1_DDR3_CTRL_0x1b8: .dword 0x0000001800060d60
//_00000000_00000000 pm_mr_3_cs_3 _00000000_00000000 pm_mr_2_cs_3 _00000000_00000000 pm_mr_1_cs_3 _00000000_00000000 pm_mr_0_cs_3
MC1_DDR3_CTRL_0x1c0: .dword 0x3030c80c03032006
//_00000000 pm_tRESET _00000000 pm_tCKE _00000000 pm_tXPR _00000000 pm_tMOD _00000000 pm_tZQCL _00000000 pm_tZQ_CMD _00000000 pm_tWLDQSEN 000_00000 pm_tRDDATA
MC1_DDR3_CTRL_0x1c8: .dword 0x1f06090903a04004
//00_000000 pm_tFAW 0000_0000 pm_tRRD 0000_0000 pm_tRCD _00000000 pm_tRP _00000000 pm_tREF _00000000 pm_tRFC _00000000 pm_tZQCS _00000000 pm_tZQperiod
MC1_DDR3_CTRL_0x1d0: .dword 0x0a0208030200001c
//0000_0000 pm_tODTL _00000000 pm_tXSRD 0000_0000 pm_tPHY_RDLAT 000_00000 pm_tPHY_WRLAT 000000_00_00000000_00000000 pm_tRAS_max 00_000000 pm_tRAS_min
MC1_DDR3_CTRL_0x1d8: .dword 0x14050c060a080506
//_00000000 pm_tXPDLL _00000000 pm_tXP 000_00000 pm_tWR 0000_0000 pm_tRTP 0000_0000 pm_tRL 0000_0000 pm_tWL 0000_0000 pm_tCCD 0000_0000 pm_tWTR
MC1_DDR3_CTRL_0x1e0: .dword 0x0404000000000000
//00_000000 pm_tW2R_diffcs_dly 00_000000 pm_tW2W_diffcs_adj_dly 00_000000 pm_tR2P_sameba_adj_dly 00_000000 pm_tW2P_sameba_adj_dly 00_000000 pm_tR2R_sameba_adj_dly 00_000000 pm_tR2W_sameba_adj_dly 00_000000 pm_tW2R_sameba_adj_dly 00_000000 pm_tW2W_sameba_adj_dly
MC1_DDR3_CTRL_0x1e8: .dword 0x0408000000000000
//00_000000 pm_tR2R_diffcs_adj_dly 00_000000 pm_tR2W_diffcs_dly 00_000000 pm_tR2P_samecs_dly 00_000000 pm_tW2P_samecs_dly 00_000000 pm_tR2R_samecs_adj_dly 00_000000 pm_tR2W_samecs_adj_dly 00_000000 pm_tW2R_samecs_adj_dly 00_000000 pm_tW2W_samecs_adj_dly
MC1_DDR3_CTRL_0x1f0: .dword 0x000801e4ff050101
//0000_0000 pm_power_up _00000000 pm_age_step _00000000 pm_tCPDED _00000000 pm_cs_map _00000000 pm_bs_config 00000_000 pm_channel_32 pm_channel_16 pm_nc 0000_0000 pm_pr_r2w 0000000_0 pm_placement_en
MC1_DDR3_CTRL_0x1f8: .dword 0x0000000004081001
//0000_0000 pm_hardware_pd_3 0000_0000 pm_hardware_pd_2 0000_0000 pm_hardware_pd_1 0000_0000 pm_hardware_pd_0 00_000000 pm_credit_16 00_000000 pm_credit_32 00_000000 pm_credit_64 0000000_0 pm_selection_en
MC1_DDR3_CTRL_0x200: .dword 0x0c000c000c000c00
//0000_0000_00000000 pm_cmdq_age_16 0000_0000_00000000 pm_cmdq_age_32 0000_0000_00000000 pm_cmdq_age_64 _00000000 pm_tCKESR _00000000 pm_tRDPDEN
MC1_DDR3_CTRL_0x208: .dword 0x0c000c0000000000
//0000_0000_00000000 pm_wrfifo_age 0000_0000_00000000 pm_rdfifo_age 0000_0000 pm_power_status_3 0000_0000 pm_power_status_2 0000_0000 pm_power_status_1 0000_0000 pm_power_status_0
MC1_DDR3_CTRL_0x210: .dword 0x0008000e02010006
//_00000000_00000000 pm_active_age 0000000_0 pm_cs_place_0 0000_0000 pm_addr_win_0 000000_00 pm_cs_diff_0 00000_000 pm_row_diff_0 000000_00 pm_ba_diff_0 00000_000 pm_col_diff_0
MC1_DDR3_CTRL_0x218: .dword 0x0008000f02010006
//_00000000_00000000 pm_fastpd_age 0000000_0 pm_cs_place_1 0000_0000 pm_addr_win_1 000000_00 pm_cs_diff_1 00000_000 pm_row_diff_1 000000_00 pm_ba_diff_1 00000_000 pm_col_diff_1
MC1_DDR3_CTRL_0x220: .dword 0x0008000f02010006
//_00000000_00000000 pm_slowpd_age 0000000_0 pm_cs_place_2 0000_0000 pm_addr_win_2 000000_00 pm_cs_diff_2 00000_000 pm_row_diff_2 000000_00 pm_ba_diff_2 00000_000 pm_col_diff_2
MC1_DDR3_CTRL_0x228: .dword 0x0008000f02010006
//_00000000_00000000 pm_selfref_age 0000000_0 pm_cs_place_3 0000_0000 pm_addr_win_3 000000_00 pm_cs_diff_3 00000_000 pm_row_diff_3 000000_00 pm_ba_diff_3 00000_000 pm_col_diff_3
MC1_DDR3_CTRL_0x230: .dword 0x0ffffc0000000000
//0000_0000_00000000_00000000_00000000 pm_addr_mask_0 0000_0000_00000000_00000000_00000000 pm_addr_base_0
MC1_DDR3_CTRL_0x238: .dword 0x0ffffc0000000400
//0000_0000_00000000_00000000_00000000 pm_addr_mask_1 0000_0000_00000000_00000000_00000000 pm_addr_base_1
MC1_DDR3_CTRL_0x240: .dword 0x0fffff000000f800
//0000_0000_00000000_00000000_00000000 pm_addr_mask_2 0000_0000_00000000_00000000_00000000 pm_addr_base_2
MC1_DDR3_CTRL_0x248: .dword 0x0fffff000000fc00
//0000_0000_00000000_00000000_00000000 pm_addr_mask_3 0000_0000_00000000_00000000_00000000 pm_addr_base_3
MC1_DDR3_CTRL_0x250: .dword 0x0000000000000000
//00000000 _00_00_00_00 pm_cmd_monitor_3_2_1_0 000000_00_00000000 pm_axi_monitor _00000000 pm_ecc_code(RD) 0000_0_000 pm_int_trigger pm_ecc_enable 000000_00 pm_int_vector 000000_00 pm_int_enable
MC1_DDR3_CTRL_0x258: .dword 0x0000000000000000
//XXXXXXXXXXXXXXXX (RD)
MC1_DDR3_CTRL_0x260: .dword 0x0000000000000000
//_XXXXXXXXXXXXXXXX pm_ecc_addr(RD)
MC1_DDR3_CTRL_0x268: .dword 0x0000000000000000
//_XXXXXXXXXXXXXXXX pm_ecc_data(RD)
MC1_DDR3_CTRL_0x270: .dword 0x0000001000000000
//000000_00 pm_lpbk_ecc_mask(RD) 0_0000000_00000000_00000000 pm_prbs_init 0000000_0 pm_lpbk_error(RD) 0000000_0 pm_prbs_23 0000000_0 pm_lpbk_start 0000000_0 pm_lpbk_en
MC1_DDR3_CTRL_0x278: .dword 0x0000000000000000
//_00000000_00000000 pm_lpbk_ecc(RD) _00000000_00000000 pm_lpbk_data_mask(RD) _00000000_00000000 pm_lpbk_correct(RD) _00000000_00000000 pm_lpbk_counter(RD)
MC1_DDR3_CTRL_0x280: .dword 0x0000000000000000
//_XXXXXXXXXXXXXXXX pm_lpbk_data[ 63: 0](RD)
MC1_DDR3_CTRL_0x288: .dword 0x0000000000000000
//_XXXXXXXXXXXXXXXX pm_lpbk_data[127:64](RD)

//Monitor fbck
MC1_DDR3_CTRL_0x290: .dword 0x0000000000000000
//_XXXXXXXXXXXXXXXX pm_axi0_fbck[ 63: 0](RD)
MC1_DDR3_CTRL_0x298: .dword 0x0000000000000000
//_XXXXXXXXXXXXXXXX pm_axi0_fbck[127:64](RD)
MC1_DDR3_CTRL_0x2a0: .dword 0x0000000000000000
//_XXXXXXXXXXXXXXXX pm_axi1_fbck[ 63: 0](RD)
MC1_DDR3_CTRL_0x2a8: .dword 0x0000000000000000
//_XXXXXXXXXXXXXXXX pm_axi1_fbck[127:64](RD)
MC1_DDR3_CTRL_0x2b0: .dword 0x0000000000000000
//_XXXXXXXXXXXXXXXX pm_axi2_fbck[ 63: 0](RD)
MC1_DDR3_CTRL_0x2b8: .dword 0x0000000000000000
//_XXXXXXXXXXXXXXXX pm_axi2_fbck[127:64](RD)
MC1_DDR3_CTRL_0x2c0: .dword 0x0000000000000000
//_XXXXXXXXXXXXXXXX pm_axi3_fbck[ 63: 0](RD)
MC1_DDR3_CTRL_0x2c8: .dword 0x0000000000000000
//_XXXXXXXXXXXXXXXX pm_axi3_fbck[127:64](RD)
MC1_DDR3_CTRL_0x2d0: .dword 0x0000000000000000
//_XXXXXXXXXXXXXXXX pm_axi4_fbck[ 63: 0](RD)
MC1_DDR3_CTRL_0x2d8: .dword 0x0000000000000000
//_XXXXXXXXXXXXXXXX pm_axi4_fbck[127:64](RD)
MC1_DDR3_CTRL_0x2e0: .dword 0x0000000000000000
//_XXXXXXXXXXXXXXXX pm_cmd_fbck0[ 63: 0](RD)
MC1_DDR3_CTRL_0x2e8: .dword 0x0000000000000000
//_XXXXXXXXXXXXXXXX pm_cmd_fbck0[127:64](RD)
MC1_DDR3_CTRL_0x2f0: .dword 0x0000000000000000
//_XXXXXXXXXXXXXXXX pm_cmd_fbck1[ 63: 0](RD)
MC1_DDR3_CTRL_0x2f8: .dword 0x0000000000000000
//_XXXXXXXXXXXXXXXX pm_cmd_fbck1[127:64](RD)
MC1_DDR3_CTRL_0x300: .dword 0x0000000000000000
//_XXXXXXXXXXXXXXXX pm_cmd_fbck2[ 63: 0](RD)
MC1_DDR3_CTRL_0x308: .dword 0x0000000000000000
//_XXXXXXXXXXXXXXXX pm_cmd_fbck2[127:64](RD)
MC1_DDR3_CTRL_0x310: .dword 0x0000000000000000
//_XXXXXXXXXXXXXXXX pm_cmd_fbck3[ 63: 0](RD)
MC1_DDR3_CTRL_0x318: .dword 0x0000000000000000
//_XXXXXXXXXXXXXXXX pm_cmd_fbck3[127:64](RD)
MC1_DDR3_CTRL_0x320: .dword 0x0008503000000005
//XXXXXXXXXXXX _XX pm_pre_predict[15:8] 0000_0000 pm_tREF[3:0]
MC1_DDR3_CTRL_0x328: .dword 0x0000000000000000
//XXXXXXXXXXXXXXXX
MC1_DDR3_CTRL_0x330: .dword 0x000000100000040a
//0000000_0 pm_stat_en(RW) 0_0000000 pm_rdbuffer_max(RD) 0000000_0 pm_retry 00_000000 pm_wr_pkg_num 0000000_0 pm_rwq_rb 0000 pm_space_stb_en 000_0 pm_stb_en 000_00000 pm_addr_new(OLD) 0000_0000 pm_tRDQidle
MC1_DDR3_CTRL_0x338: .dword 0x0000002000000000
//XXXXXX 00_0000_00 pm_rdfifo_depth(RW) XXXXXXXX pm_retry_cnt(RD)
MC1_DDR3_CTRL_0x340: .dword 0x0030d40000070f00
//_XXXXXXXX pm_tREFretention XX 00000_000 pm_ref_num _00000000 pm_tREF_IDLE 000000_00 pm_ref_sch_en
MC1_DDR3_CTRL_0x348: .dword 0x0000000000000000
//XXXXXXXXXXXXXXXX
MC1_DDR3_CTRL_0x350: .dword 0xffffffffffffffff
//_XXXXXXXXXXXXXXXX pm_lpbk_data_en
MC1_DDR3_CTRL_0x358: .dword 0x000000000001ffff
//XXXXXXXXXX 0000000_0 pm_lpbk_ecc_mask_en _00000000 pm_lpbk_ecc_en _00000000 pm_lpbk_data_mask_en
MC1_DDR3_CTRL_0x360: .dword 0x0001000000000000
//XX _XX pm_int_cnt _XX pm_int_cnt_fatal(RD) _XX pm_int_cnt_error(RD) _XX pm_ecc_cnt_cs_3(RD) _XX pm_ecc_cnt_cs_2(RD) _XX pm_ecc_cnt_cs_1(RD) _XX pm_ecc_cnt_cs_0(RD)
MC1_DDR3_CTRL_0x368: .dword 0x0000000000000000
//XXXXXXXXXXXXXXXX

ddr3_RDIMM_reg_data:
ddr3_RDIMM_reg_data_mc1:

    .align 5
