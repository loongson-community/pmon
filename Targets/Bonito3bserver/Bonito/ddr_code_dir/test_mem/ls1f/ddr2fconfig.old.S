#define REG_ADDRESS 0x0
#define CONFIG_BASE 0xaffffe00
    
	.text
        .global ddr2_config_old
        .ent    ddr2_config_old
        .set    noreorder
       # .set    mips3
ddr2_config_old:
    la      t0, ddr2_reg_data_old
    addu    t0, t0, s0
    li      t1, 0x1d
    li      t2, CONFIG_BASE

reg_write_old:
    ld      a1, 0x0(t0)
    sd      a1, REG_ADDRESS(t2)
    subu    t1, t1, 0x1
    addiu     t0, t0, 0x8
    bne     t1, $0, reg_write_old
    addiu     t2, t2, 0x10
    
    ############start##########
    li      t2, CONFIG_BASE
    la		t0,DDR2_CTL_start_DATA_LO_old
    #la		t0,ddr2_start_reg
    addu    	t0, t0, s0
    ld      a1, 0x0(t0)
    sd      a1, 0x30(t2)

    jr      ra
    nop
    .end    ddr2_config_old

	.rdata
	.align 5
ddr2_reg_data_old:
//0000000_0 arefresh 0000000_1 ap 0000000_1 addr_cmp_en 0000000_1 active_aging
DDR2_CTL_00_DATA_LO_old:	.word 0x00000101
// 0000000_1 ddrii_sdram_mode 0000000_1 concurrentap 0000000_1 bank_split_en 0000000_0 auto_refresh_mode 
DDR2_CTL_00_DATA_HI_old:	.word 0x01000100 #no_concurrentap
//DDR2_CTL_00_DATA_HI_old:	.word 0x01010100
//0000000_0 ecc_disable_w_uc_err 0000000_1 dqs_n_en 0000000_0 dll_bypass_mode 0000000_0 dlllockreg
//DDR2_CTL_01_DATA_LO_old:	.word 0x00010100 #dll_by_pass
DDR2_CTL_01_DATA_LO_old:	.word 0x00000000
//0000000_0 fwc 0000000_0 fast_write 0000000_0 enable_quick_srefresh 0000000_0 eight_bank_mode 
DDR2_CTL_01_DATA_HI_old:	.word 0x00000001
//0000000_0 no_cmd_init 0000000_0 intrptwritea 0000000_0 intrptreada 0000000_0 intrptapburst
DDR2_CTL_02_DATA_LO_old:	.word 0x00000000
//0000000_1 priority_en 0000000_0 power_down 0000000_1 placement_en 0000000_1 odt_add_turn_clk_en 
DDR2_CTL_02_DATA_HI_old:	.word 0x01000101
//0000000_1 rw_same_en 0000000_0 reg_dimm_enable 0000000_0 reduc 0000000_0 pwrup_srefresh_exit
DDR2_CTL_03_DATA_LO_old:	.word 0x01000100
//0000000_1 swap_port_rw_same_en 0000000_1 swap_en 0000000_0 start 0000000_0 srefresh 
DDR2_CTL_03_DATA_HI_old:	.word 0x01010000
//0000000_0 write_modereg 0000000_1 writeinterp 0000000_1 tref_enable 0000000_1 tras_lockout
DDR2_CTL_04_DATA_LO_old:	.word 0x00010101
//000000_01 rtt_0 000000_00 ctrl_raw 000000_10 axi0_w_priority 000000_10 axi0_r_priority 
DDR2_CTL_04_DATA_HI_old:	.word 0x01000202
//00000_100 column_size 00000_101 caslat 00000_010 addr_pins 000000_10 rtt_pad_termination
//DDR2_CTL_05_DATA_LO_old:	.word 0x04040102 #CL =4
DDR2_CTL_05_DATA_LO_old:	.word 0x04040202 #CL =5
//zgj DDR2_CTL_05_DATA_LO_old:	.word 0x04040202 #CL =4
//00000_000 q_fullness 00000_000 port_data_error_type 00000_000 out_of_range_type 00000_000 max_cs_reg 
DDR2_CTL_05_DATA_HI_old:	.word 0x00000000
//00000_010 trtp 00000_010 trrd 00000_010 temrs 00000_011 tcke
//DDR2_CTL_06_DATA_LO_old:	.word 0x01020203 #125 M
DDR2_CTL_06_DATA_LO_old:	.word 0x02020203 #400
//DDR2_CTL_06_DATA_LO_old:	.word 0x03050203 #800
//0000_1010 aprebit 00000_100 wrlat 00000_010 twtr 00000_100 twr_int 
//DDR2_CTL_06_DATA_HI_old:	.word 0x0a030203 #125 M
DDR2_CTL_06_DATA_HI_old:	.word 0x0a040204 #400
//DDR2_CTL_06_DATA_HI_old:	.word 0x0a040306 #800
//0000_0000 ecc_c_id 0000_1111 cs_map 0000_0111 caslat_lin_gate 0000_1010 caslat_lin
//DDR2_CTL_07_DATA_LO_old:	.word 0x000f0708 #CL=4//cs_map to cs0-cs3
//DDR2_CTL_07_DATA_LO_old:	.word 0x000f0a0b #CL=5//cs_map to cs0-cs3
//DDR2_CTL_07_DATA_LO_old:	.word 0x00010a0a #CL=5//cs_map to cs0-cs3
DDR2_CTL_07_DATA_LO_old:	.word 0x00010807 #CL=4//cs_map to cs2-cs3
//0000_0000 max_row_reg 0000_0000 max_col_reg 0000_0010 initaref 0000_0000 ecc_u_id 
DDR2_CTL_07_DATA_HI_old:	.word 0x00000200
//DDR2_CTL_07_DATA_HI_old:	.word 0x00000400 #800
//0000_0001 odt_rd_map_cs3 0000_0010 odt_rd_map_cs2 0000_0100 odt_rd_map_cs1 0000_1000 odt_rd_map_cs0
DDR2_CTL_08_DATA_LO_old:	.word 0x00000000
//0000_0001 odt_wr_map_cs3 0000_0010 odt_wr_map_cs2 0000_0100 odt_wr_map_cs1 0000_1000 odt_wr_map_cs0 
DDR2_CTL_08_DATA_HI_old:	.word 0x01020200
//0000_0000 port_data_error_id 0000_0000 port_cmd_error_type 0000_0000 port_cmd_error_id 0000_0000 out_of_range_source_id
DDR2_CTL_09_DATA_LO_old:	.word 0x00000000
//000_00000 ocd_adjust_pup_cs_0 000_00000 ocd_adjust_pdn_cs_0 0000_0100 trp 0000_1000 tdal 
//DDR2_CTL_09_DATA_HI_old:	.word 0x00000204 #125 M
DDR2_CTL_09_DATA_HI_old:	.word 0x00000408 #400
//DDR2_CTL_09_DATA_HI_old:	.word 0x0000060c #800
//00_111111 age_count 000_01111 trc 000_00010 tmrd 000_00000 tfaw
//DDR2_CTL_10_DATA_LO_old:	.word 0x3f070200 #125 M
DDR2_CTL_10_DATA_LO_old:	.word 0x3f0f021b #400
//DDR2_CTL_10_DATA_LO_old:	.word 0x3f1a021b #800
//0_0011101 dll_dqs_delay_2 0_0011101 dll_dqs_delay_1 0_0011101 dll_dqs_delay_0 00_111111 command_age_count 
DDR2_CTL_10_DATA_HI_old:	.word 0x0202023f
//0_0011101 dll_dqs_delay_6 0_0011101 dll_dqs_delay_5 0_0011101 dll_dqs_delay_4 0_0011101 dll_dqs_delay_3
DDR2_CTL_11_DATA_LO_old:	.word 0x1d1d1d02
//0_1011111 wr_dqs_shift 0_1111111 dqs_out_shift 0_0011101 dll_dqs_delay_8 0_0011101 dll_dqs_delay_7 
DDR2_CTL_11_DATA_HI_old:	.word 0x4f001d1d
//00001011 tras_min 00000000 out_of_range_length 00000000 ecc_u_synd 00000000 ecc_c_synd
//DDR2_CTL_12_DATA_LO_old:	.word 0x05000000 #125 M
DDR2_CTL_12_DATA_LO_old:	.word 0x0b000000 #400
//DDR2_CTL_12_DATA_LO_old:	.word 0x15000000 #800
//0000000_000101010 dll_dqs_delay_bypass_0 00011100 trfc 00000100 trcd_int 
//DDR2_CTL_12_DATA_HI_old:	.word 0x01ff0302 #125/2 M,read_dqs_delay_max
//DDR2_CTL_12_DATA_HI_old:	.word 0x002a0302 #125/2 M
DDR2_CTL_12_DATA_HI_old:	.word 0x002a0602 #125 M
//DDR2_CTL_12_DATA_HI_old:	.word 0x002a3c04 #400
//DDR2_CTL_12_DATA_HI_old:	.word 0x002a3c05 #800
//0000000_000101010 dll_dqs_delay_bypass_2 0000000_000101010 dll_dqs_delay_bypass_1
DDR2_CTL_13_DATA_LO_old:	.word 0x01ff01ff #read_dqs_delay_max
//DDR2_CTL_13_DATA_LO_old:	.word 0x002a002a 
//0000000_000101010 dll_dqs_delay_bypass_4 0000000_000101010 dll_dqs_delay_bypass_3 
DDR2_CTL_13_DATA_HI_old:	.word 0x01ff01ff #read_dqs_delay_max
//DDR2_CTL_13_DATA_HI_old:	.word 0x002a002a
//0000000_000101010 dll_dqs_delay_bypass_6 0000000_000101010 dll_dqs_delay_bypass_5
DDR2_CTL_14_DATA_LO_old:	.word 0x01ff01ff #read_dqs_delay_max
//DDR2_CTL_14_DATA_LO_old:	.word 0x002a002a
//0000000_000101010 dll_dqs_delay_bypass_8 0000000_000101010 dll_dqs_delay_bypass_7 
DDR2_CTL_14_DATA_HI_old:	.word 0x01ff01ff #read_dqs_delay_max
//DDR2_CTL_14_DATA_HI_old:	.word 0x002a002a
//0000000_000000000 dll_lock 0000000_000100100 dll_increment
DDR2_CTL_15_DATA_LO_old:	.word 0x00000004
//0000000_010110100 dqs_out_shift_bypass 0000000_010000111 dll_start_point 
DDR2_CTL_15_DATA_HI_old:	.word 0x00b40008
//000000_0000000000 int_ack 0000000_010000111 wr_dqs_shift_bypass
DDR2_CTL_16_DATA_LO_old:	.word 0x00000087
//00000_00000000000 int_status 00000_00000000000 int_mask 
DDR2_CTL_16_DATA_HI_old:	.word 0x000007ff #no_interrupt
//DDR2_CTL_16_DATA_HI_old:	.word 0x00000000 #no_masked
//0_000000000000000 emrs1_data 00_00100000011011 tref
//DDR2_CTL_17_DATA_LO_old:	.word 0x0000004b #125/16 M
//DDR2_CTL_17_DATA_LO_old:	.word 0x0000009c #20 M
DDR2_CTL_17_DATA_LO_old:	.word 0x000004b0 #125 M
//DDR2_CTL_17_DATA_LO_old:	.word 0x0000081b #400
//DDR2_CTL_17_DATA_LO_old:	.word 0x0000101b #800
//0_000000000000000 emrs2_data_1 0_000000000000000 emrs2_data_0 
DDR2_CTL_17_DATA_HI_old:	.word 0x00000000
//0_000000000000000 emrs2_data_3 0_000000000000000 emrs2_data_2
DDR2_CTL_18_DATA_LO_old:	.word 0x00000000
//0000000000011100 axi0_en_size_lt_width_instr 0_000000000000000 emrs3_data 
DDR2_CTL_18_DATA_HI_old:	.word 0xffff0000
//0000000011001000 tdll 0000000001101011 tcpd
DDR2_CTL_19_DATA_LO_old:	.word 0x00c8006b
//0100100011100001 tras_max 0000000000000010 tpdex 
DDR2_CTL_19_DATA_HI_old:	.word 0x04b00002 #125 M
//DDR2_CTL_19_DATA_HI_old:	.word 0x48e10002 #400
//DDR2_CTL_19_DATA_HI_old:	.word 0x68e10002 #800
//0000000011001000 txsr 0000000000011111 txsnr
DDR2_CTL_20_DATA_LO_old:	.word 0x00c8000f #125 M
//DDR2_CTL_20_DATA_LO_old:	.word 0x00c8001f #400
//DDR2_CTL_20_DATA_LO_old:	.word 0x00c8002f #800
//0000000000000000 xor_check_bits 0000000000000000 version 
DDR2_CTL_20_DATA_HI_old:	.word 0x00000000
 //000000000000000000110110 tinit
DDR2_CTL_21_DATA_LO_old:	.word 0x00030d40 #real
//DDR2_CTL_21_DATA_LO_old:	.word 0x00000036 #simulation
 //000_0000000000000000000000000000000000000 ecc_c_addr 
DDR2_CTL_21_DATA_HI_old:	.word 0x00000000
//000000000000000000000000000_0000000000000000000000000000000000000 ecc_u_addr
DDR2_CTL_22_DATA_LO_old:	.word 0x00000000
DDR2_CTL_22_DATA_HI_old:	.word 0x00000000
//000000000000000000000000000_0000000000000000000000000000000000000 out_of_range_addr
DDR2_CTL_23_DATA_LO_old:	.word 0x00000000
DDR2_CTL_23_DATA_HI_old:	.word 0x00000000
//000000000000000000000000000_0000000000000000000000000000000000000 port_cmd_error_addr
DDR2_CTL_24_DATA_LO_old:	.word 0x00000000
DDR2_CTL_24_DATA_HI_old:	.word 0x00000000
//0000000000000000000000000000000000000000000000000000000000000000 ecc_c_data
DDR2_CTL_25_DATA_LO_old:	.word 0x00000000
DDR2_CTL_25_DATA_HI_old:	.word 0x00000000
//0000000000000000000000000000000000000000000000000000000000000000 ecc_u_data
DDR2_CTL_26_DATA_LO_old:	.word 0x00000000
DDR2_CTL_26_DATA_HI_old:	.word 0x00000000
//0000000000000000000000000000000000000000000000000000000000000000 
DDR2_CTL_27_DATA_LO_old:	.word 0x00000000
DDR2_CTL_27_DATA_HI_old:	.word 0x00000000
//0000000000000000000000000000000000000000000000000000000000000000
DDR2_CTL_28_DATA_LO_old:	.word 0x00000000
DDR2_CTL_28_DATA_HI_old:	.word 0x00000000
//0000000_1 rw_same_en 0000000_0 reg_dimm_enable 0000000_0 reduc 0000000_0 pwrup_srefresh_exit
DDR2_CTL_start_DATA_LO_old: .word 0x01000100
//0000000_1 swap_port_rw_same_en 0000000_1 swap_en 0000000_0 start 0000000_0 srefresh
DDR2_CTL_start_DATA_HI_old: .word 0x01010100
