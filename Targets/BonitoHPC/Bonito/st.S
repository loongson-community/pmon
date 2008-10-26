ddr2_reg_data:
//0000000_0 arefresh 0000000_0 ap 0000000_1 addr_cmp_en 0000000_1 active_aging
DDR2_CTL_00_DATA_LO:    .word 0x01010101
// 0000000_1 ddrii_sdram_mode 0000000_0 concurrentap 0000000_1 bank_split_en 0000000_0 auto_refresh_mode 
DDR2_CTL_00_DATA_HI:    .word 0x01000100 #no_concurrentap
//0000000_0 ecc_disable_w_uc_err 0000000_1 dqs_n_en 0000000_0 dll_bypass_mode 0000000_0 dlllockreg
DDR2_CTL_01_DATA_LO:    .word 0x00010000
//0000000_0 fwc 0000000_0 fast_write 0000000_0 enable_quick_srefresh 0000000_0 eight_bank_mode 
DDR2_CTL_01_DATA_HI:    .word 0x00000000
//0000000_0 no_cmd_init 0000000_0 intrptwritea 0000000_0 intrptreada 0000000_0 intrptapburst 
DDR2_CTL_02_DATA_LO:    .word 0x00000000
//0000000_1 priority_en 0000000_0 power_down 0000000_1 placement_en 0000000_1 odt_add_turn_clk_en 
DDR2_CTL_02_DATA_HI:    .word 0x01000101
//0000000_1 rw_same_en 0000000_0 reg_dimm_enable 0000000_0 reduc 0000000_0 pwrup_srefresh_exit 
DDR2_CTL_03_DATA_LO:    .word 0x01000000
//0000000_1 swap_port_rw_same_en 0000000_1 swap_en 0000000_0 start 0000000_0 srefresh 
DDR2_CTL_03_DATA_HI:    .word 0x01010000
//0000000_0 write_modereg 0000000_1 writeinterp 0000000_1 tref_enable 0000000_1 tras_lockout 
DDR2_CTL_04_DATA_LO:    .word 0x01010101
//000000_01 rtt_0 000000_00 ctrl_raw 000000_10 axi0_w_priority 000000_10 axi0_r_priority 
DDR2_CTL_04_DATA_HI:    .word 0x03000202
//00000_100 column_size 00000_101 caslat 00000_010 addr_pins 000000_10 rtt_pad_termination 
DDR2_CTL_05_DATA_LO:    .word 0x00050002 #CL =5 Auto detect Row and Col
//DDR2_CTL_05_DATA_LO:    .word 0x04050202 #CL =5 32Mx16bit
//00000_000 q_fullness 00000_000 port_data_error_type 00000_000 out_of_range_type 00000_000 max_cs_reg 
DDR2_CTL_05_DATA_HI:    .word 0x00000000
//00000_010 trtp 00000_101 trrd 00000_010 temrs 00000_011 tcke
DDR2_CTL_06_DATA_LO:    .word 0x03050203 
//0000_1010 aprebit 00000_100 wrlat 00000_100 twtr 00000_110 twr_int  
DDR2_CTL_06_DATA_HI:    .word 0x0a040406 #CL=5
//0000_0000 ecc_c_id 0000_1111 cs_map 0000_1010 caslat_lin_gate 0000_1011 caslat_lin 
DDR2_CTL_07_DATA_LO:    .word 0x00030a0b #CL=5//cs_map to cs0-cs3
//0000_0000 max_row_reg 0000_0000 max_col_reg 0000_0100 initaref 0000_0000 ecc_u_id 
DDR2_CTL_07_DATA_HI:    .word 0x00000400
//0000_0000 odt_rd_map_cs3 0000_0000 odt_rd_map_cs2 0000_0000 odt_rd_map_cs1 0000_0000 odt_rd_map_cs0 
DDR2_CTL_08_DATA_LO:    .word 0x00000000
//0000_0000 odt_wr_map_cs3 0000_0000 odt_wr_map_cs2 0000_0010 odt_wr_map_cs1 0000_0001 odt_wr_map_cs0 
DDR2_CTL_08_DATA_HI:    .word 0x00000201
//0000_0000 port_data_error_id 0000_0000 port_cmd_error_type 0000_0000 port_cmd_error_id 0000_0000 out_of_range_source_id 
DDR2_CTL_09_DATA_LO:    .word 0x00000000
//000_00000 ocd_adjust_pup_cs_0 000_00000 ocd_adjust_pdn_cs_0 0000_0110 trp 0000_1100 tdal 
DDR2_CTL_09_DATA_HI:    .word 0x0000060c
//00_111111 age_count 000_11010 trc 000_00010 tmrd 000_00000 tfaw
DDR2_CTL_10_DATA_LO:    .word 0x3f1a0200
//0_0010010 dll_dqs_delay_2 0_0010010 dll_dqs_delay_1 0_0010010 dll_dqs_delay_0 00_111111 command_age_count 
DDR2_CTL_10_DATA_HI:    .word 0x1111113f
//0_0010010 dll_dqs_delay_6 0_0010010 dll_dqs_delay_5 0_0010010 dll_dqs_delay_4 0_0010010 dll_dqs_delay_3
DDR2_CTL_11_DATA_LO:    .word 0x11111111
//0_1011011 wr_dqs_shift 0_1111110 dqs_out_shift 0_0010010 dll_dqs_delay_8 0_0010010 dll_dqs_delay_7 
DDR2_CTL_11_DATA_HI:    .word 0x5f7f1111
//00010011 tras_min 00000000 out_of_range_length 00000000 ecc_u_synd 00000000 ecc_c_synd
DDR2_CTL_12_DATA_LO:    .word 0x13000000
//0000000_000101010 dll_dqs_delay_bypass_0 00111100 trfc 00000110 trcd_int 
DDR2_CTL_12_DATA_HI:    .word 0x002a3c06
//DDR2_CTL_12_DATA_HI:    .word 0x000a3c06
//0000000_000101010 dll_dqs_delay_bypass_2 0000000_000101010 dll_dqs_delay_bypass_1
DDR2_CTL_13_DATA_LO:    .word 0x002a002a
//0000000_000101010 dll_dqs_delay_bypass_4 0000000_000101010 dll_dqs_delay_bypass_3 
DDR2_CTL_13_DATA_HI:    .word 0x002a002a
//0000000_000101010 dll_dqs_delay_bypass_6 0000000_000101010 dll_dqs_delay_bypass_5
DDR2_CTL_14_DATA_LO:    .word 0x002a002a
//0000000_000101010 dll_dqs_delay_bypass_8 0000000_000101010 dll_dqs_delay_bypass_7 
DDR2_CTL_14_DATA_HI:    .word 0x002a002a
//0000000_000000000 dll_lock 0000000_000100100 dll_increment
DDR2_CTL_15_DATA_LO:    .word 0x00000001
//0000000_010110100 dqs_out_shift_bypass 0000000_000110000 dll_start_point 
//DDR2_CTL_15_DATA_HI:    .word 0x00b40030
DDR2_CTL_15_DATA_HI:    .word 0x00420030  
//000000_0000000000 int_ack 0000000_010000111 wr_dqs_shift_bypass
//DDR2_CTL_16_DATA_LO:    .word 0x00000087
DDR2_CTL_16_DATA_LO:    .word 0x00000032
//00000_00000000000 int_status 00000_00000000000 int_mask 
DDR2_CTL_16_DATA_HI:    .word 0x000007ff #no_interrupt 
//DDR2_CTL_16_DATA_HI:    .word 0x00000000 #no_masked
//0_000000000010110 emrs1_data 00_01000000011011 tref
DDR2_CTL_17_DATA_LO:    .word 0x0016101b #half strengh, AL=2, ODT=75ohm
//0_000000000000000 emrs2_data_1 0_000000000000000 emrs2_data_0 
DDR2_CTL_17_DATA_HI:    .word 0x00000000
//0_000000000000000 emrs2_data_3 0_000000000000000 emrs2_data_2
DDR2_CTL_18_DATA_LO:    .word 0x00000000
//0000000000011100 axi0_en_size_lt_width_instr 0_000000000000000 emrs3_data 
DDR2_CTL_18_DATA_HI:    .word 0x001c0000
//0000000011001000 tdll 0000000001101011 tcpd
DDR2_CTL_19_DATA_LO:    .word 0x00c8006b
//0110100011110001 tras_max 0000000000000010 tpdex 
DDR2_CTL_19_DATA_HI:    .word 0x68e10002 
//0000000011001000 txsr 0000000000110000 txsnr
DDR2_CTL_20_DATA_LO:    .word 0x00c80040
//0000000000000000 xor_check_bits 0000000000000000 version 
DDR2_CTL_20_DATA_HI:    .word 0x00000000
 //000000000000000000110110 tinit
DDR2_CTL_21_DATA_LO:    .word 0x00030d40 #real 
//DDR2_CTL_21_DATA_LO:    .word 0x00000036 #simulation
//000_0000000000000000000000000000000000000 ecc_c_addr 
DDR2_CTL_21_DATA_HI:    .word 0x00000000
//000000000000000000000000000_0000000000000000000000000000000000000 ecc_u_addr 
DDR2_CTL_22_DATA_LO:    .word 0x00000000
DDR2_CTL_22_DATA_HI:    .word 0x00000000
//000000000000000000000000000_0000000000000000000000000000000000000 out_of_range_addr
DDR2_CTL_23_DATA_LO:    .word 0x00000000 
DDR2_CTL_23_DATA_HI:    .word 0x00000000
//000000000000000000000000000_0000000000000000000000000000000000000 port_cmd_error_addr
DDR2_CTL_24_DATA_LO:    .word 0x00000000
DDR2_CTL_24_DATA_HI:    .word 0x00000000 
//0000000000000000000000000000000000000000000000000000000000000000 ecc_c_data
DDR2_CTL_25_DATA_LO:    .word 0x00000000
DDR2_CTL_25_DATA_HI:    .word 0x00000000
//0000000000000000000000000000000000000000000000000000000000000000 ecc_u_data 
DDR2_CTL_26_DATA_LO:    .word 0x00000000
DDR2_CTL_26_DATA_HI:    .word 0x00000000
//0000000000000000000000000000000000000000000000000000000000000000 
DDR2_CTL_27_DATA_LO:    .word 0x00000000
DDR2_CTL_27_DATA_HI:    .word 0x00000000 
//0000000000000000000000000000000000000000000000000000000000000000
DDR2_CTL_28_DATA_LO:    .word 0x00000001
DDR2_CTL_28_DATA_HI:    .word 0x00000000
//0000000_1 rw_same_en 0000000_0 reg_dimm_enable 0000000_0 reduc 0000000_0 pwrup_srefresh_exit 
DDR2_CTL_start_DATA_LO: .word 0x01000000
//0000000_1 swap_port_rw_same_en 0000000_1 swap_en 0000000_0 start 0000000_0 srefresh
DDR2_CTL_start_DATA_HI: .word 0x01010100 
