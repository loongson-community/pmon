/**********************************
    Author: chenxinke
    Date:   20110125
    v2.0
        support both 3A and 3A3 according macro defination--loongson3A3
        MC0/1 both use the same subroutine, add an input param: t3.
    input param:
        t8: input, Memory Controller config register base
        t3: controller select(config param select)
            0: MC0
            1: MC1
    v3.0    Modify for 3B which is MC0_only.
            not use t3 any more
**********************************/

        .global ddr2_config
        .ent    ddr2_config
        .set    noreorder
        .set    mips3
ddr2_config:
#ifdef  ARB_LEVEL
    //if use leveled ddr param, the param location is fixed
    dla     a2, ddr2_leveled_mark
    daddu   a2, a2, s0
    ld      a2, 0x0(a2)
    beqz    a2, 5f
    nop
    dla     a2, ddr2_reg_data_leveled
#ifdef  DUAL_3B
    GET_NODE_ID_a1
    dli     v0, 0x1
    beq     a1, v0, 1f
    nop
    daddiu  v0, v0, 0x1
    beq     a1, v0, 2f
    nop
    daddiu  v0, v0, 0x1
    beq     a1, v0, 3f
    nop
    b       4f
    nop
1:
    dla     a2, n1_ddr2_reg_data_leveled
    b       4f
    nop
2:
    dla     a2, n2_ddr2_reg_data_leveled
    b       4f
    nop
3:
    dla     a2, n3_ddr2_reg_data_leveled
#else
    GET_NODE_ID_a1
    beqz    a1, 4f
    nop
    dla     a2, n1_ddr2_reg_data_leveled
#endif
    b       4f
    nop
5:
#endif
    GET_SDRAM_TYPE
    dli     t1, 0x2
    beq     t1, a1, 2f
    nop
    dli     t1, 0x3
    beq     t1, a1, 3f
    nop
    //not DDR2 and not DDR3, errors
    move    t1, ra
    PRINTSTR("\r\n!!!! NOT recognized DDR SDRAM TYPE. MC parameters will not be written!!!!\r\n");
    move    ra, t1
    b       ddr2_config_end
    nop
2:  //DDR2
    GET_DIMM_TYPE
    bnez    a1, 1f
    nop
    //UDIMM
    dla     a2, ddr2_reg_data
    GET_NODE_ID_a1
    and     v1, a1, 0x1
    beqz    v1, 4f
    nop
    dla     a2, ddr2_reg_data_mc1
#ifdef  MULTI_NODE_DDR_PARAM
    GET_NODE_ID_a1
    dsrl    v1, a1, 1
    beqz    v1, 4f
    nop
    dla     a2, n1_ddr2_reg_data
    and     v1, a1, 0x1
    beqz    v1, 4f
    nop
    dla     a2, n1_ddr2_reg_data_mc1
#endif
    b       4f
    nop
1:  //RDIMM
    dla     a2, ddr2_RDIMM_reg_data
    GET_NODE_ID_a1
    and     v1, a1, 0x1
    beqz    v1, 4f
    nop
    dla     a2, ddr2_RDIMM_reg_data_mc1
#ifdef  MULTI_NODE_DDR_PARAM
    GET_NODE_ID_a1
    dsrl    v1, a1, 1
    beqz    v1, 4f
    nop
    dla     a2, n1_ddr2_RDIMM_reg_data
    and     v1, a1, 0x1
    beqz    v1, 4f
    nop
    dla     a2, n1_ddr2_RDIMM_reg_data_mc1
#endif
    b       4f
    nop
3:  //DDR3
    GET_DIMM_TYPE
    bnez    a1, 1f
    nop
    //UDIMM
    dla     a2, ddr3_reg_data
    GET_NODE_ID_a1
    and     v1, a1, 0x1
    beqz    v1, 4f
    nop
    dla     a2, ddr3_reg_data_mc1
#ifdef  MULTI_NODE_DDR_PARAM
    GET_NODE_ID_a1
    dsrl    v1, a1, 1
    beqz    v1, 4f
    nop
    dla     a2, n1_ddr3_reg_data
    and     v1, a1, 0x1
    beqz    v1, 4f
    nop
    dla     a2, n1_ddr3_reg_data_mc1
#endif
    b       4f
    nop
1:  //RDIMM
    dla     a2, ddr3_RDIMM_reg_data
    GET_NODE_ID_a1
    and     v1, a1, 0x1
    beqz    v1, 4f
    nop
    dla     a2, ddr3_RDIMM_reg_data_mc1
#ifdef  MULTI_NODE_DDR_PARAM
    GET_NODE_ID_a1
    dsrl    v1, a1, 1
    beqz    v1, 4f
    nop
    dla     a2, n1_ddr3_RDIMM_reg_data
    and     v1, a1, 0x1
    beqz    v1, 4f
    nop
    dla     a2, n1_ddr3_RDIMM_reg_data_mc1
#endif
4:

    daddu   a2, a2, s0
    dli     t1, DDR_PARAM_NUM
    daddiu  v0, t8, 0x0
//write param registers
1:
    ld      a1, 0x0(a2)
    sd      a1, 0x0(v0)
    daddiu  t1, t1, -1
    daddiu  a2, a2, 0x8
    daddiu  v0, v0, 0x8
    bnez    t1, 1b
    nop

    //rewrite eight_bank_mode
    //rewrite pm_bank_diff_0
    ld      a2, EIGHT_BANK_MODE_ADDR(t8)
    dli     a1, 0xff
    dsll    a1, a1, EIGHT_BANK_MODE_OFFSET
    not     a1, a1
    and     a2, a2, a1
    GET_EIGHT_BANK
    xor     a1, a1, 0x1
    dsll    a1, a1, EIGHT_BANK_MODE_OFFSET
    or      a2, a2, a1
    sd      a2, EIGHT_BANK_MODE_ADDR(t8)
    //rewrite pm_bank
    ld      a2, BANK_NUM_ADDR(t8)
    dli     a1, 0xff
    dsll    a1, a1, BANK_NUM_OFFSET
    not     a1, a1
    and     a2, a2, a1
    dli     v0, 0x7
    GET_EIGHT_BANK
    xor     a1, a1, 0x1
    dsrl    a1, v0, a1
    dsll    a1, a1, BANK_NUM_OFFSET
    or      a2, a2, a1
    sd      a2, BANK_NUM_ADDR(t8)

    //rewrite column_size and addr_pins
    ld      a2, ADDR_PINS_ADDR(t8)
    dli     a1, 0xff
    dsll    a1, a1, ADDR_PINS_OFFSET
    not     a1, a1
    and     a2, a2, a1
    GET_ROW_SIZE
    dsll    a1, a1, ADDR_PINS_OFFSET
    or      a2, a2, a1
    sd      a2, ADDR_PINS_ADDR(t8)

    ld      a2, COLUMN_SIZE_ADDR(t8)
    dli     a1, 0xff
    dsll    a1, a1, COLUMN_SIZE_OFFSET
    not     a1, a1
    and     a2, a2, a1
    GET_COL_SIZE
    daddu   a1, a1, 0x4
    dsll    a1, a1, COLUMN_SIZE_OFFSET
    or      a2, a2, a1
    sd      a2, COLUMN_SIZE_ADDR(t8)
    //rewrite cs_map cs_mrs and cs_zq
    ld      a2, CS_MAP_ADDR(t8)
    dli     a1, 0xff
    dsll    a1, a1, CS_MAP_OFFSET
    not     a1, a1
    and     a2, a2, a1
    GET_MC_CS_MAP
    dsll    a1, a1, CS_MAP_OFFSET
    or      a2, a2, a1
    sd      a2, CS_MAP_ADDR(t8)

    ld      a2, CS_MRS_ADDR(t8)
    dli     a1, 0xff
    dsll    a1, a1, CS_MRS_OFFSET
    not     a1, a1
    and     a2, a2, a1
    GET_MC_CS_MAP
    dsll    a1, a1, CS_MRS_OFFSET
    or      a2, a2, a1
    sd      a2, CS_MRS_ADDR(t8)

    ld      a2, CS_ZQ_ADDR(t8)
    dli     a1, 0xff
    dsll    a1, a1, CS_ZQ_OFFSET
    not     a1, a1
    and     a2, a2, a1
    GET_MC_CS_MAP
    dsll    a1, a1, CS_ZQ_OFFSET
    or      a2, a2, a1
    sd      a2, CS_ZQ_ADDR(t8)
    //reconfig address_mirroring
    ld      a2, ADDRESS_MIRROR_ADDR(t8)
    dli     a1, 0xff
    dsll    a1, a1, ADDRESS_MIRROR_OFFSET
    not     a1, a1
    and     a2, a2, a1
    //When use RDIMM, ignore address mirror bit
    GET_DIMM_TYPE
    bnez    a1, 1f
    nop
    GET_ADDR_MIRROR
    beqz    a1, 1f
    nop
    dli     a1, 0xa
    b       2f
    nop
1:
    dli     a1, 0x0
2:
    dsll    a1, a1, ADDRESS_MIRROR_OFFSET
    or      a2, a2, a1
    sd      a2, ADDRESS_MIRROR_ADDR(t8)
    //reconfig ODT map
    //set default first
    dli     a2, 0x8421050048120501
    sd      a2, ODT_MAP_CS_ADDR(t8)

    //v0 store cs map
    GET_MC_CS_MAP
    move    v0, a1
    //step 1: swap open wr odt if it's a Dual Rank DIMM
    //check cs_map[3]
    dsrl    a2, v0, 3
    beqz    a2, 1f
    nop
    //slot 1 is a DR DIMM
    ld      a2, ODT_MAP_CS_ADDR(t8)
    dli     a1, 0x00ffffffffffffff
    and     a2, a2, a1
    dli     a1, 0x4800000000000000
    or      a2, a2, a1
    sd      a2, ODT_MAP_CS_ADDR(t8)
1:
    //check cs_map[1]
    dsrl    a2, v0, 1
    and     a2, a2, 0x1
    beqz    a2, 1f
    nop
    //slot 0 is a DR DIMM
    ld      a2, ODT_MAP_CS_ADDR(t8)
    dli     a1, 0xff00ffffffffffff
    and     a2, a2, a1
    dli     a1, 0x0012000000000000
    or      a2, a2, a1
    sd      a2, ODT_MAP_CS_ADDR(t8)
1:
    //step 2: open extra RD/WR ODT CS if there is 2 DIMM
    //check CS[0] and CS[2]
    dsrl    a2, v0, 2
    xor     a2, v0, a2
    and     a2, a2, 0x1
    bnez    a2, 1f
    nop
    //2 DIMM: open the first rank of the non-target DIMM
    ld      a2, ODT_MAP_CS_ADDR(t8)
    dli     a1, 0x1144000011440000
    or      a2, a2, a1
    sd      a2, ODT_MAP_CS_ADDR(t8)
1:  //1 DIMM
#if 0
    //if its DDR3_DIMM, enable dynamic ODT and reset ODT value
    GET_SDRAM_TYPE
    dli     a2, 0x3
    bne     a2, a1, 1f
    nop
    //DDR3 DIMM, enable RTT_wr
    ld      a2, MR2_DATA_0_ADDR(t8)
    dli     a1, 0x0400
    dsll    a1, a1, MR2_DATA_0_OFFSET
    or      a2, a2, a1
    sd      a2, MR2_DATA_0_ADDR(t8)

    ld      a2, MR2_DATA_1_ADDR(t8)
    dli     a1, 0x0400
    dsll    a1, a1, MR2_DATA_1_OFFSET
    or      a2, a2, a1
    sd      a2, MR2_DATA_1_ADDR(t8)

    ld      a2, MR2_DATA_2_ADDR(t8)
    dli     a1, 0x0400
    dsll    a1, a1, MR2_DATA_2_OFFSET
    or      a2, a2, a1
    sd      a2, MR2_DATA_2_ADDR(t8)

    ld      a2, MR2_DATA_3_ADDR(t8)
    dli     a1, 0x0400
    dsll    a1, a1, MR2_DATA_3_OFFSET
    or      a2, a2, a1
    sd      a2, MR2_DATA_3_ADDR(t8)
1:
#endif
    //set data bus width
    ld      a2, REDUC_ADDR(t8)
    dli     a1, 0xff
    dsll    a1, a1, REDUC_OFFSET
    not     a1, a1
    and     a2, a2, a1
    GET_DIMM_WIDTH
    dsll    a1, a1, REDUC_OFFSET
    or      a2, a2, a1
    sd      a2, REDUC_ADDR(t8)
    //disable ECC module here for ARB_level, ECC will be enabled later
    ld      a2, CTRL_RAW_ADDR(t8)
    dli     a1, 0xf
    dsll    a1, a1, CTRL_RAW_OFFSET
    not     a1, a1
    and     a2, a2, a1
    dli     a1, 0x0
    dsll    a1, a1, CTRL_RAW_OFFSET
    or      a2, a2, a1
    sd      a2, CTRL_RAW_ADDR(t8)

    //reconfigure CS_1/2/3 addr info as CS_0
    ld      a2, ADDR_INFO_CS_0_ADDR(t8)
    sd      a2, ADDR_INFO_CS_1_ADDR(t8)
    sd      a2, ADDR_INFO_CS_2_ADDR(t8)
    sd      a2, ADDR_INFO_CS_3_ADDR(t8)

#ifdef DEBUG_DDR_PARAM   //debug
//input once, change all byte lanes parameters.
    /* store the ra */
    move    t1, ra

41:
    PRINTSTR("\r\nChange parameters:\r\n0--skip;1--dll_clock;2--dll_rddqs_gate;3--dll_rddqs_p;4--dll_rddqs_n;5--dll_wrdqs;6--dll_wrdq;\r\n7--rd_oe_end/start-edge_stop/start;8--wr_dqs_oe_end/start-edge_stop/start;9--wr_dq_oe_end/start-edge_stop/start;a--wr_odt_oe_end/start-edge_stop/start;\r\nb--wrdq_clk_delay;c--rddata_delay;d--rddqs_lt_half;e--wrdqs_lt_half;f--wrdq_lt_half;\r\n");
    dli     t6, 0x00
    bal     inputaddress
    nop
    beqz    v0, 90f;
    nop
    move    t5, v0
    PRINTSTR("\r\nPlease input the data-hex: ");
    dli        t6, 0x00
    bal        inputaddress
    nop
    move    a2, t5
    dli     a1, 0xffffffff
    and     t5, v0, a1
/*****************
a2: change select
t5: value
*****************/

//!!!!!note: don't change the switch order of the code bellow, because we use
//add instr to change a1 instead of dli instr to reduce code size.
    dli     a1, 0x1
    beq     a2, a1, 1f;
    nop
    daddiu  a1, a1, 0x1
    beq     a2, a1, 2f;
    nop
    daddiu  a1, a1, 0x1
    beq     a2, a1, 3f;
    nop
    daddiu  a1, a1, 0x1
    beq     a2, a1, 4f;
    nop
    daddiu  a1, a1, 0x1
    beq     a2, a1, 5f;
    nop
    daddiu  a1, a1, 0x1
    beq     a2, a1, 6f;
    nop
    daddiu  a1, a1, 0x1
    beq     a2, a1, 7f;
    nop
    daddiu  a1, a1, 0x1
    beq     a2, a1, 8f;
    nop
    daddiu  a1, a1, 0x1
    beq     a2, a1, 9f;
    nop
    daddiu  a1, a1, 0x1
    beq     a2, a1, 10f;
    nop
    daddiu  a1, a1, 0x1
    beq     a2, a1, 11f;
    nop
    daddiu  a1, a1, 0x1
    beq     a2, a1, 12f;
    nop
    daddiu  a1, a1, 0x1
    beq     a2, a1, 13f;
    nop
    daddiu  a1, a1, 0x1
    beq     a2, a1, 14f;
    nop
    daddiu  a1, a1, 0x1
    beq     a2, a1, 15f;
    nop
    PRINTSTR("\r\n--------Wrong selection: no parameter will be changed.");
    b       40f
    nop
1:
    and     t5, t5, CLKLVL_DELAY_MASK

    ld      a1, CLKLVL_DELAY_2_ADDR(t8)
    dli     a2, CLKLVL_DELAY_MASK
    dsll    a2, a2, CLKLVL_DELAY_2_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, CLKLVL_DELAY_2_OFFSET
    or      a1, a1, a2
    sd      a1, CLKLVL_DELAY_2_ADDR(t8)

    ld      a1, CLKLVL_DELAY_1_ADDR(t8)
    dli     a2, CLKLVL_DELAY_MASK
    dsll    a2, a2, CLKLVL_DELAY_1_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, CLKLVL_DELAY_1_OFFSET
    or      a1, a1, a2
    sd      a1, CLKLVL_DELAY_1_ADDR(t8)

    ld      a1, CLKLVL_DELAY_0_ADDR(t8)
    dli     a2, CLKLVL_DELAY_MASK
    dsll    a2, a2, CLKLVL_DELAY_0_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, CLKLVL_DELAY_0_OFFSET
    or      a1, a1, a2
    sd      a1, CLKLVL_DELAY_0_ADDR(t8)
    b       40f
    nop
2:
    and     t5, t5, RDLVL_GATE_DELAY_MASK

    ld      a1, RDLVL_GATE_DELAY_8_ADDR(t8)
    dli     a2, RDLVL_GATE_DELAY_MASK
    dsll    a2, a2, RDLVL_GATE_DELAY_8_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, RDLVL_GATE_DELAY_8_OFFSET
    or      a1, a1, a2
    sd      a1, RDLVL_GATE_DELAY_8_ADDR(t8)

    ld      a1, RDLVL_GATE_DELAY_7_ADDR(t8)
    dli     a2, RDLVL_GATE_DELAY_MASK
    dsll    a2, a2, RDLVL_GATE_DELAY_7_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, RDLVL_GATE_DELAY_7_OFFSET
    or      a1, a1, a2
    sd      a1, RDLVL_GATE_DELAY_7_ADDR(t8)

    ld      a1, RDLVL_GATE_DELAY_6_ADDR(t8)
    dli     a2, RDLVL_GATE_DELAY_MASK
    dsll    a2, a2, RDLVL_GATE_DELAY_6_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, RDLVL_GATE_DELAY_6_OFFSET
    or      a1, a1, a2
    sd      a1, RDLVL_GATE_DELAY_6_ADDR(t8)

    ld      a1, RDLVL_GATE_DELAY_5_ADDR(t8)
    dli     a2, RDLVL_GATE_DELAY_MASK
    dsll    a2, a2, RDLVL_GATE_DELAY_5_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, RDLVL_GATE_DELAY_5_OFFSET
    or      a1, a1, a2
    sd      a1, RDLVL_GATE_DELAY_5_ADDR(t8)

    ld      a1, RDLVL_GATE_DELAY_4_ADDR(t8)
    dli     a2, RDLVL_GATE_DELAY_MASK
    dsll    a2, a2, RDLVL_GATE_DELAY_4_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, RDLVL_GATE_DELAY_4_OFFSET
    or      a1, a1, a2
    sd      a1, RDLVL_GATE_DELAY_4_ADDR(t8)

    ld      a1, RDLVL_GATE_DELAY_3_ADDR(t8)
    dli     a2, RDLVL_GATE_DELAY_MASK
    dsll    a2, a2, RDLVL_GATE_DELAY_3_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, RDLVL_GATE_DELAY_3_OFFSET
    or      a1, a1, a2
    sd      a1, RDLVL_GATE_DELAY_3_ADDR(t8)

    ld      a1, RDLVL_GATE_DELAY_2_ADDR(t8)
    dli     a2, RDLVL_GATE_DELAY_MASK
    dsll    a2, a2, RDLVL_GATE_DELAY_2_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, RDLVL_GATE_DELAY_2_OFFSET
    or      a1, a1, a2
    sd      a1, RDLVL_GATE_DELAY_2_ADDR(t8)

    ld      a1, RDLVL_GATE_DELAY_1_ADDR(t8)
    dli     a2, RDLVL_GATE_DELAY_MASK
    dsll    a2, a2, RDLVL_GATE_DELAY_1_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, RDLVL_GATE_DELAY_1_OFFSET
    or      a1, a1, a2
    sd      a1, RDLVL_GATE_DELAY_1_ADDR(t8)

    ld      a1, RDLVL_GATE_DELAY_0_ADDR(t8)
    dli     a2, RDLVL_GATE_DELAY_MASK
    dsll    a2, a2, RDLVL_GATE_DELAY_0_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, RDLVL_GATE_DELAY_0_OFFSET
    or      a1, a1, a2
    sd      a1, RDLVL_GATE_DELAY_0_ADDR(t8)
    b       40f
    nop
3:
    and     t5, t5, RDLVL_DELAY_MASK

    ld      a1, RDLVL_DELAY_8_ADDR(t8)
    dli     a2, RDLVL_DELAY_MASK
    dsll    a2, a2, RDLVL_DELAY_8_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, RDLVL_DELAY_8_OFFSET
    or      a1, a1, a2
    sd      a1, RDLVL_DELAY_8_ADDR(t8)

    ld      a1, RDLVL_DELAY_7_ADDR(t8)
    dli     a2, RDLVL_DELAY_MASK
    dsll    a2, a2, RDLVL_DELAY_7_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, RDLVL_DELAY_7_OFFSET
    or      a1, a1, a2
    sd      a1, RDLVL_DELAY_7_ADDR(t8)

    ld      a1, RDLVL_DELAY_6_ADDR(t8)
    dli     a2, RDLVL_DELAY_MASK
    dsll    a2, a2, RDLVL_DELAY_6_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, RDLVL_DELAY_6_OFFSET
    or      a1, a1, a2
    sd      a1, RDLVL_DELAY_6_ADDR(t8)

    ld      a1, RDLVL_DELAY_5_ADDR(t8)
    dli     a2, RDLVL_DELAY_MASK
    dsll    a2, a2, RDLVL_DELAY_5_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, RDLVL_DELAY_5_OFFSET
    or      a1, a1, a2
    sd      a1, RDLVL_DELAY_5_ADDR(t8)

    ld      a1, RDLVL_DELAY_4_ADDR(t8)
    dli     a2, RDLVL_DELAY_MASK
    dsll    a2, a2, RDLVL_DELAY_4_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, RDLVL_DELAY_4_OFFSET
    or      a1, a1, a2
    sd      a1, RDLVL_DELAY_4_ADDR(t8)

    ld      a1, RDLVL_DELAY_3_ADDR(t8)
    dli     a2, RDLVL_DELAY_MASK
    dsll    a2, a2, RDLVL_DELAY_3_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, RDLVL_DELAY_3_OFFSET
    or      a1, a1, a2
    sd      a1, RDLVL_DELAY_3_ADDR(t8)

    ld      a1, RDLVL_DELAY_2_ADDR(t8)
    dli     a2, RDLVL_DELAY_MASK
    dsll    a2, a2, RDLVL_DELAY_2_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, RDLVL_DELAY_2_OFFSET
    or      a1, a1, a2
    sd      a1, RDLVL_DELAY_2_ADDR(t8)

    ld      a1, RDLVL_DELAY_1_ADDR(t8)
    dli     a2, RDLVL_DELAY_MASK
    dsll    a2, a2, RDLVL_DELAY_1_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, RDLVL_DELAY_1_OFFSET
    or      a1, a1, a2
    sd      a1, RDLVL_DELAY_1_ADDR(t8)

    ld      a1, RDLVL_DELAY_0_ADDR(t8)
    dli     a2, RDLVL_DELAY_MASK
    dsll    a2, a2, RDLVL_DELAY_0_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, RDLVL_DELAY_0_OFFSET
    or      a1, a1, a2
    sd      a1, RDLVL_DELAY_0_ADDR(t8)
    b       40f
    nop
4:
    and     t5, t5, RDLVL_DQSN_DELAY_MASK

    ld      a1, RDLVL_DQSN_DELAY_8_ADDR(t8)
    dli     a2, RDLVL_DQSN_DELAY_MASK
    dsll    a2, a2, RDLVL_DQSN_DELAY_8_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, RDLVL_DQSN_DELAY_8_OFFSET
    or      a1, a1, a2
    sd      a1, RDLVL_DQSN_DELAY_8_ADDR(t8)

    ld      a1, RDLVL_DQSN_DELAY_7_ADDR(t8)
    dli     a2, RDLVL_DQSN_DELAY_MASK
    dsll    a2, a2, RDLVL_DQSN_DELAY_7_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, RDLVL_DQSN_DELAY_7_OFFSET
    or      a1, a1, a2
    sd      a1, RDLVL_DQSN_DELAY_7_ADDR(t8)

    ld      a1, RDLVL_DQSN_DELAY_6_ADDR(t8)
    dli     a2, RDLVL_DQSN_DELAY_MASK
    dsll    a2, a2, RDLVL_DQSN_DELAY_6_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, RDLVL_DQSN_DELAY_6_OFFSET
    or      a1, a1, a2
    sd      a1, RDLVL_DQSN_DELAY_6_ADDR(t8)

    ld      a1, RDLVL_DQSN_DELAY_5_ADDR(t8)
    dli     a2, RDLVL_DQSN_DELAY_MASK
    dsll    a2, a2, RDLVL_DQSN_DELAY_5_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, RDLVL_DQSN_DELAY_5_OFFSET
    or      a1, a1, a2
    sd      a1, RDLVL_DQSN_DELAY_5_ADDR(t8)

    ld      a1, RDLVL_DQSN_DELAY_4_ADDR(t8)
    dli     a2, RDLVL_DQSN_DELAY_MASK
    dsll    a2, a2, RDLVL_DQSN_DELAY_4_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, RDLVL_DQSN_DELAY_4_OFFSET
    or      a1, a1, a2
    sd      a1, RDLVL_DQSN_DELAY_4_ADDR(t8)

    ld      a1, RDLVL_DQSN_DELAY_3_ADDR(t8)
    dli     a2, RDLVL_DQSN_DELAY_MASK
    dsll    a2, a2, RDLVL_DQSN_DELAY_3_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, RDLVL_DQSN_DELAY_3_OFFSET
    or      a1, a1, a2
    sd      a1, RDLVL_DQSN_DELAY_3_ADDR(t8)

    ld      a1, RDLVL_DQSN_DELAY_2_ADDR(t8)
    dli     a2, RDLVL_DQSN_DELAY_MASK
    dsll    a2, a2, RDLVL_DQSN_DELAY_2_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, RDLVL_DQSN_DELAY_2_OFFSET
    or      a1, a1, a2
    sd      a1, RDLVL_DQSN_DELAY_2_ADDR(t8)

    ld      a1, RDLVL_DQSN_DELAY_1_ADDR(t8)
    dli     a2, RDLVL_DQSN_DELAY_MASK
    dsll    a2, a2, RDLVL_DQSN_DELAY_1_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, RDLVL_DQSN_DELAY_1_OFFSET
    or      a1, a1, a2
    sd      a1, RDLVL_DQSN_DELAY_1_ADDR(t8)

    ld      a1, RDLVL_DQSN_DELAY_0_ADDR(t8)
    dli     a2, RDLVL_DQSN_DELAY_MASK
    dsll    a2, a2, RDLVL_DQSN_DELAY_0_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, RDLVL_DQSN_DELAY_0_OFFSET
    or      a1, a1, a2
    sd      a1, RDLVL_DQSN_DELAY_0_ADDR(t8)
    b       40f
    nop
5:
    and     t5, t5, WRLVL_DELAY_MASK

    ld      a1, WRLVL_DELAY_8_ADDR(t8)
    dli     a2, WRLVL_DELAY_MASK
    dsll    a2, a2, WRLVL_DELAY_8_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, WRLVL_DELAY_8_OFFSET
    or      a1, a1, a2
    sd      a1, WRLVL_DELAY_8_ADDR(t8)

    ld      a1, WRLVL_DELAY_7_ADDR(t8)
    dli     a2, WRLVL_DELAY_MASK
    dsll    a2, a2, WRLVL_DELAY_7_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, WRLVL_DELAY_7_OFFSET
    or      a1, a1, a2
    sd      a1, WRLVL_DELAY_7_ADDR(t8)

    ld      a1, WRLVL_DELAY_6_ADDR(t8)
    dli     a2, WRLVL_DELAY_MASK
    dsll    a2, a2, WRLVL_DELAY_6_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, WRLVL_DELAY_6_OFFSET
    or      a1, a1, a2
    sd      a1, WRLVL_DELAY_6_ADDR(t8)

    ld      a1, WRLVL_DELAY_5_ADDR(t8)
    dli     a2, WRLVL_DELAY_MASK
    dsll    a2, a2, WRLVL_DELAY_5_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, WRLVL_DELAY_5_OFFSET
    or      a1, a1, a2
    sd      a1, WRLVL_DELAY_5_ADDR(t8)

    ld      a1, WRLVL_DELAY_4_ADDR(t8)
    dli     a2, WRLVL_DELAY_MASK
    dsll    a2, a2, WRLVL_DELAY_4_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, WRLVL_DELAY_4_OFFSET
    or      a1, a1, a2
    sd      a1, WRLVL_DELAY_4_ADDR(t8)

    ld      a1, WRLVL_DELAY_3_ADDR(t8)
    dli     a2, WRLVL_DELAY_MASK
    dsll    a2, a2, WRLVL_DELAY_3_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, WRLVL_DELAY_3_OFFSET
    or      a1, a1, a2
    sd      a1, WRLVL_DELAY_3_ADDR(t8)

    ld      a1, WRLVL_DELAY_2_ADDR(t8)
    dli     a2, WRLVL_DELAY_MASK
    dsll    a2, a2, WRLVL_DELAY_2_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, WRLVL_DELAY_2_OFFSET
    or      a1, a1, a2
    sd      a1, WRLVL_DELAY_2_ADDR(t8)

    ld      a1, WRLVL_DELAY_1_ADDR(t8)
    dli     a2, WRLVL_DELAY_MASK
    dsll    a2, a2, WRLVL_DELAY_1_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, WRLVL_DELAY_1_OFFSET
    or      a1, a1, a2
    sd      a1, WRLVL_DELAY_1_ADDR(t8)

    ld      a1, WRLVL_DELAY_0_ADDR(t8)
    dli     a2, WRLVL_DELAY_MASK
    dsll    a2, a2, WRLVL_DELAY_0_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, WRLVL_DELAY_0_OFFSET
    or      a1, a1, a2
    sd      a1, WRLVL_DELAY_0_ADDR(t8)
    b       40f
    nop
6:
    and     t5, t5, WRLVL_DQ_DELAY_MASK

    ld      a1, WRLVL_DQ_DELAY_8_ADDR(t8)
    dli     a2, WRLVL_DQ_DELAY_MASK
    dsll    a2, a2, WRLVL_DQ_DELAY_8_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, WRLVL_DQ_DELAY_8_OFFSET
    or      a1, a1, a2
    sd      a1, WRLVL_DQ_DELAY_8_ADDR(t8)

    ld      a1, WRLVL_DQ_DELAY_7_ADDR(t8)
    dli     a2, WRLVL_DQ_DELAY_MASK
    dsll    a2, a2, WRLVL_DQ_DELAY_7_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, WRLVL_DQ_DELAY_7_OFFSET
    or      a1, a1, a2
    sd      a1, WRLVL_DQ_DELAY_7_ADDR(t8)

    ld      a1, WRLVL_DQ_DELAY_6_ADDR(t8)
    dli     a2, WRLVL_DQ_DELAY_MASK
    dsll    a2, a2, WRLVL_DQ_DELAY_6_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, WRLVL_DQ_DELAY_6_OFFSET
    or      a1, a1, a2
    sd      a1, WRLVL_DQ_DELAY_6_ADDR(t8)

    ld      a1, WRLVL_DQ_DELAY_5_ADDR(t8)
    dli     a2, WRLVL_DQ_DELAY_MASK
    dsll    a2, a2, WRLVL_DQ_DELAY_5_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, WRLVL_DQ_DELAY_5_OFFSET
    or      a1, a1, a2
    sd      a1, WRLVL_DQ_DELAY_5_ADDR(t8)

    ld      a1, WRLVL_DQ_DELAY_4_ADDR(t8)
    dli     a2, WRLVL_DQ_DELAY_MASK
    dsll    a2, a2, WRLVL_DQ_DELAY_4_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, WRLVL_DQ_DELAY_4_OFFSET
    or      a1, a1, a2
    sd      a1, WRLVL_DQ_DELAY_4_ADDR(t8)

    ld      a1, WRLVL_DQ_DELAY_3_ADDR(t8)
    dli     a2, WRLVL_DQ_DELAY_MASK
    dsll    a2, a2, WRLVL_DQ_DELAY_3_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, WRLVL_DQ_DELAY_3_OFFSET
    or      a1, a1, a2
    sd      a1, WRLVL_DQ_DELAY_3_ADDR(t8)

    ld      a1, WRLVL_DQ_DELAY_2_ADDR(t8)
    dli     a2, WRLVL_DQ_DELAY_MASK
    dsll    a2, a2, WRLVL_DQ_DELAY_2_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, WRLVL_DQ_DELAY_2_OFFSET
    or      a1, a1, a2
    sd      a1, WRLVL_DQ_DELAY_2_ADDR(t8)

    ld      a1, WRLVL_DQ_DELAY_1_ADDR(t8)
    dli     a2, WRLVL_DQ_DELAY_MASK
    dsll    a2, a2, WRLVL_DQ_DELAY_1_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, WRLVL_DQ_DELAY_1_OFFSET
    or      a1, a1, a2
    sd      a1, WRLVL_DQ_DELAY_1_ADDR(t8)

    ld      a1, WRLVL_DQ_DELAY_0_ADDR(t8)
    dli     a2, WRLVL_DQ_DELAY_MASK
    dsll    a2, a2, WRLVL_DQ_DELAY_0_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, WRLVL_DQ_DELAY_0_OFFSET
    or      a1, a1, a2
    sd      a1, WRLVL_DQ_DELAY_0_ADDR(t8)
    b       40f
    nop
7:
    and     t5, t5, RD_OE_EDGE_MASK

    ld      a1, RD_OE_EDGE_8_ADDR(t8)
    dli     a2, RD_OE_EDGE_MASK
    dsll    a2, a2, RD_OE_EDGE_8_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, RD_OE_EDGE_8_OFFSET
    or      a1, a1, a2
    sd      a1, RD_OE_EDGE_8_ADDR(t8)

    ld      a1, RD_OE_EDGE_7_ADDR(t8)
    dli     a2, RD_OE_EDGE_MASK
    dsll    a2, a2, RD_OE_EDGE_7_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, RD_OE_EDGE_7_OFFSET
    or      a1, a1, a2
    sd      a1, RD_OE_EDGE_7_ADDR(t8)

    ld      a1, RD_OE_EDGE_6_ADDR(t8)
    dli     a2, RD_OE_EDGE_MASK
    dsll    a2, a2, RD_OE_EDGE_6_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, RD_OE_EDGE_6_OFFSET
    or      a1, a1, a2
    sd      a1, RD_OE_EDGE_6_ADDR(t8)

    ld      a1, RD_OE_EDGE_5_ADDR(t8)
    dli     a2, RD_OE_EDGE_MASK
    dsll    a2, a2, RD_OE_EDGE_5_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, RD_OE_EDGE_5_OFFSET
    or      a1, a1, a2
    sd      a1, RD_OE_EDGE_5_ADDR(t8)

    ld      a1, RD_OE_EDGE_4_ADDR(t8)
    dli     a2, RD_OE_EDGE_MASK
    dsll    a2, a2, RD_OE_EDGE_4_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, RD_OE_EDGE_4_OFFSET
    or      a1, a1, a2
    sd      a1, RD_OE_EDGE_4_ADDR(t8)

    ld      a1, RD_OE_EDGE_3_ADDR(t8)
    dli     a2, RD_OE_EDGE_MASK
    dsll    a2, a2, RD_OE_EDGE_3_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, RD_OE_EDGE_3_OFFSET
    or      a1, a1, a2
    sd      a1, RD_OE_EDGE_3_ADDR(t8)

    ld      a1, RD_OE_EDGE_2_ADDR(t8)
    dli     a2, RD_OE_EDGE_MASK
    dsll    a2, a2, RD_OE_EDGE_2_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, RD_OE_EDGE_2_OFFSET
    or      a1, a1, a2
    sd      a1, RD_OE_EDGE_2_ADDR(t8)

    ld      a1, RD_OE_EDGE_1_ADDR(t8)
    dli     a2, RD_OE_EDGE_MASK
    dsll    a2, a2, RD_OE_EDGE_1_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, RD_OE_EDGE_1_OFFSET
    or      a1, a1, a2
    sd      a1, RD_OE_EDGE_1_ADDR(t8)

    ld      a1, RD_OE_EDGE_0_ADDR(t8)
    dli     a2, RD_OE_EDGE_MASK
    dsll    a2, a2, RD_OE_EDGE_0_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, RD_OE_EDGE_0_OFFSET
    or      a1, a1, a2
    sd      a1, RD_OE_EDGE_0_ADDR(t8)
    b       40f
    nop
8:
    and     t5, t5, WR_DQS_OE_EDGE_MASK

    ld      a1, WR_DQS_OE_EDGE_8_ADDR(t8)
    dli     a2, WR_DQS_OE_EDGE_MASK
    dsll    a2, a2, WR_DQS_OE_EDGE_8_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, WR_DQS_OE_EDGE_8_OFFSET
    or      a1, a1, a2
    sd      a1, WR_DQS_OE_EDGE_8_ADDR(t8)

    ld      a1, WR_DQS_OE_EDGE_7_ADDR(t8)
    dli     a2, WR_DQS_OE_EDGE_MASK
    dsll    a2, a2, WR_DQS_OE_EDGE_7_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, WR_DQS_OE_EDGE_7_OFFSET
    or      a1, a1, a2
    sd      a1, WR_DQS_OE_EDGE_7_ADDR(t8)

    ld      a1, WR_DQS_OE_EDGE_6_ADDR(t8)
    dli     a2, WR_DQS_OE_EDGE_MASK
    dsll    a2, a2, WR_DQS_OE_EDGE_6_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, WR_DQS_OE_EDGE_6_OFFSET
    or      a1, a1, a2
    sd      a1, WR_DQS_OE_EDGE_6_ADDR(t8)

    ld      a1, WR_DQS_OE_EDGE_5_ADDR(t8)
    dli     a2, WR_DQS_OE_EDGE_MASK
    dsll    a2, a2, WR_DQS_OE_EDGE_5_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, WR_DQS_OE_EDGE_5_OFFSET
    or      a1, a1, a2
    sd      a1, WR_DQS_OE_EDGE_5_ADDR(t8)

    ld      a1, WR_DQS_OE_EDGE_4_ADDR(t8)
    dli     a2, WR_DQS_OE_EDGE_MASK
    dsll    a2, a2, WR_DQS_OE_EDGE_4_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, WR_DQS_OE_EDGE_4_OFFSET
    or      a1, a1, a2
    sd      a1, WR_DQS_OE_EDGE_4_ADDR(t8)

    ld      a1, WR_DQS_OE_EDGE_3_ADDR(t8)
    dli     a2, WR_DQS_OE_EDGE_MASK
    dsll    a2, a2, WR_DQS_OE_EDGE_3_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, WR_DQS_OE_EDGE_3_OFFSET
    or      a1, a1, a2
    sd      a1, WR_DQS_OE_EDGE_3_ADDR(t8)

    ld      a1, WR_DQS_OE_EDGE_2_ADDR(t8)
    dli     a2, WR_DQS_OE_EDGE_MASK
    dsll    a2, a2, WR_DQS_OE_EDGE_2_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, WR_DQS_OE_EDGE_2_OFFSET
    or      a1, a1, a2
    sd      a1, WR_DQS_OE_EDGE_2_ADDR(t8)

    ld      a1, WR_DQS_OE_EDGE_1_ADDR(t8)
    dli     a2, WR_DQS_OE_EDGE_MASK
    dsll    a2, a2, WR_DQS_OE_EDGE_1_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, WR_DQS_OE_EDGE_1_OFFSET
    or      a1, a1, a2
    sd      a1, WR_DQS_OE_EDGE_1_ADDR(t8)

    ld      a1, WR_DQS_OE_EDGE_0_ADDR(t8)
    dli     a2, WR_DQS_OE_EDGE_MASK
    dsll    a2, a2, WR_DQS_OE_EDGE_0_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, WR_DQS_OE_EDGE_0_OFFSET
    or      a1, a1, a2
    sd      a1, WR_DQS_OE_EDGE_0_ADDR(t8)
    b       40f
    nop
9:
    and     t5, t5, WR_DQ_OE_EDGE_MASK

    ld      a1, WR_DQ_OE_EDGE_8_ADDR(t8)
    dli     a2, WR_DQ_OE_EDGE_MASK
    dsll    a2, a2, WR_DQ_OE_EDGE_8_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, WR_DQ_OE_EDGE_8_OFFSET
    or      a1, a1, a2
    sd      a1, WR_DQ_OE_EDGE_8_ADDR(t8)

    ld      a1, WR_DQ_OE_EDGE_7_ADDR(t8)
    dli     a2, WR_DQ_OE_EDGE_MASK
    dsll    a2, a2, WR_DQ_OE_EDGE_7_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, WR_DQ_OE_EDGE_7_OFFSET
    or      a1, a1, a2
    sd      a1, WR_DQ_OE_EDGE_7_ADDR(t8)

    ld      a1, WR_DQ_OE_EDGE_6_ADDR(t8)
    dli     a2, WR_DQ_OE_EDGE_MASK
    dsll    a2, a2, WR_DQ_OE_EDGE_6_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, WR_DQ_OE_EDGE_6_OFFSET
    or      a1, a1, a2
    sd      a1, WR_DQ_OE_EDGE_6_ADDR(t8)

    ld      a1, WR_DQ_OE_EDGE_5_ADDR(t8)
    dli     a2, WR_DQ_OE_EDGE_MASK
    dsll    a2, a2, WR_DQ_OE_EDGE_5_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, WR_DQ_OE_EDGE_5_OFFSET
    or      a1, a1, a2
    sd      a1, WR_DQ_OE_EDGE_5_ADDR(t8)

    ld      a1, WR_DQ_OE_EDGE_4_ADDR(t8)
    dli     a2, WR_DQ_OE_EDGE_MASK
    dsll    a2, a2, WR_DQ_OE_EDGE_4_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, WR_DQ_OE_EDGE_4_OFFSET
    or      a1, a1, a2
    sd      a1, WR_DQ_OE_EDGE_4_ADDR(t8)

    ld      a1, WR_DQ_OE_EDGE_3_ADDR(t8)
    dli     a2, WR_DQ_OE_EDGE_MASK
    dsll    a2, a2, WR_DQ_OE_EDGE_3_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, WR_DQ_OE_EDGE_3_OFFSET
    or      a1, a1, a2
    sd      a1, WR_DQ_OE_EDGE_3_ADDR(t8)

    ld      a1, WR_DQ_OE_EDGE_2_ADDR(t8)
    dli     a2, WR_DQ_OE_EDGE_MASK
    dsll    a2, a2, WR_DQ_OE_EDGE_2_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, WR_DQ_OE_EDGE_2_OFFSET
    or      a1, a1, a2
    sd      a1, WR_DQ_OE_EDGE_2_ADDR(t8)

    ld      a1, WR_DQ_OE_EDGE_1_ADDR(t8)
    dli     a2, WR_DQ_OE_EDGE_MASK
    dsll    a2, a2, WR_DQ_OE_EDGE_1_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, WR_DQ_OE_EDGE_1_OFFSET
    or      a1, a1, a2
    sd      a1, WR_DQ_OE_EDGE_1_ADDR(t8)

    ld      a1, WR_DQ_OE_EDGE_0_ADDR(t8)
    dli     a2, WR_DQ_OE_EDGE_MASK
    dsll    a2, a2, WR_DQ_OE_EDGE_0_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, WR_DQ_OE_EDGE_0_OFFSET
    or      a1, a1, a2
    sd      a1, WR_DQ_OE_EDGE_0_ADDR(t8)
    b       40f
    nop
10:
    and     t5, t5, WR_ODT_OE_EDGE_MASK

    ld      a1, WR_ODT_OE_EDGE_8_ADDR(t8)
    dli     a2, WR_ODT_OE_EDGE_MASK
    dsll    a2, a2, WR_ODT_OE_EDGE_8_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, WR_ODT_OE_EDGE_8_OFFSET
    or      a1, a1, a2
    sd      a1, WR_ODT_OE_EDGE_8_ADDR(t8)

    ld      a1, WR_ODT_OE_EDGE_7_ADDR(t8)
    dli     a2, WR_ODT_OE_EDGE_MASK
    dsll    a2, a2, WR_ODT_OE_EDGE_7_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, WR_ODT_OE_EDGE_7_OFFSET
    or      a1, a1, a2
    sd      a1, WR_ODT_OE_EDGE_7_ADDR(t8)

    ld      a1, WR_ODT_OE_EDGE_6_ADDR(t8)
    dli     a2, WR_ODT_OE_EDGE_MASK
    dsll    a2, a2, WR_ODT_OE_EDGE_6_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, WR_ODT_OE_EDGE_6_OFFSET
    or      a1, a1, a2
    sd      a1, WR_ODT_OE_EDGE_6_ADDR(t8)

    ld      a1, WR_ODT_OE_EDGE_5_ADDR(t8)
    dli     a2, WR_ODT_OE_EDGE_MASK
    dsll    a2, a2, WR_ODT_OE_EDGE_5_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, WR_ODT_OE_EDGE_5_OFFSET
    or      a1, a1, a2
    sd      a1, WR_ODT_OE_EDGE_5_ADDR(t8)

    ld      a1, WR_ODT_OE_EDGE_4_ADDR(t8)
    dli     a2, WR_ODT_OE_EDGE_MASK
    dsll    a2, a2, WR_ODT_OE_EDGE_4_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, WR_ODT_OE_EDGE_4_OFFSET
    or      a1, a1, a2
    sd      a1, WR_ODT_OE_EDGE_4_ADDR(t8)

    ld      a1, WR_ODT_OE_EDGE_3_ADDR(t8)
    dli     a2, WR_ODT_OE_EDGE_MASK
    dsll    a2, a2, WR_ODT_OE_EDGE_3_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, WR_ODT_OE_EDGE_3_OFFSET
    or      a1, a1, a2
    sd      a1, WR_ODT_OE_EDGE_3_ADDR(t8)

    ld      a1, WR_ODT_OE_EDGE_2_ADDR(t8)
    dli     a2, WR_ODT_OE_EDGE_MASK
    dsll    a2, a2, WR_ODT_OE_EDGE_2_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, WR_ODT_OE_EDGE_2_OFFSET
    or      a1, a1, a2
    sd      a1, WR_ODT_OE_EDGE_2_ADDR(t8)

    ld      a1, WR_ODT_OE_EDGE_1_ADDR(t8)
    dli     a2, WR_ODT_OE_EDGE_MASK
    dsll    a2, a2, WR_ODT_OE_EDGE_1_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, WR_ODT_OE_EDGE_1_OFFSET
    or      a1, a1, a2
    sd      a1, WR_ODT_OE_EDGE_1_ADDR(t8)

    ld      a1, WR_ODT_OE_EDGE_0_ADDR(t8)
    dli     a2, WR_ODT_OE_EDGE_MASK
    dsll    a2, a2, WR_ODT_OE_EDGE_0_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, WR_ODT_OE_EDGE_0_OFFSET
    or      a1, a1, a2
    sd      a1, WR_ODT_OE_EDGE_0_ADDR(t8)
    b       40f
    nop
11:
    and     t5, t5, WRDQ_CLK_DELAY_MASK

    ld      a1, WRDQ_CLK_DELAY_8_ADDR(t8)
    dli     a2, WRDQ_CLK_DELAY_MASK
    dsll    a2, a2, WRDQ_CLK_DELAY_8_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, WRDQ_CLK_DELAY_8_OFFSET
    or      a1, a1, a2
    sd      a1, WRDQ_CLK_DELAY_8_ADDR(t8)

    ld      a1, WRDQ_CLK_DELAY_7_ADDR(t8)
    dli     a2, WRDQ_CLK_DELAY_MASK
    dsll    a2, a2, WRDQ_CLK_DELAY_7_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, WRDQ_CLK_DELAY_7_OFFSET
    or      a1, a1, a2
    sd      a1, WRDQ_CLK_DELAY_7_ADDR(t8)

    ld      a1, WRDQ_CLK_DELAY_6_ADDR(t8)
    dli     a2, WRDQ_CLK_DELAY_MASK
    dsll    a2, a2, WRDQ_CLK_DELAY_6_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, WRDQ_CLK_DELAY_6_OFFSET
    or      a1, a1, a2
    sd      a1, WRDQ_CLK_DELAY_6_ADDR(t8)

    ld      a1, WRDQ_CLK_DELAY_5_ADDR(t8)
    dli     a2, WRDQ_CLK_DELAY_MASK
    dsll    a2, a2, WRDQ_CLK_DELAY_5_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, WRDQ_CLK_DELAY_5_OFFSET
    or      a1, a1, a2
    sd      a1, WRDQ_CLK_DELAY_5_ADDR(t8)

    ld      a1, WRDQ_CLK_DELAY_4_ADDR(t8)
    dli     a2, WRDQ_CLK_DELAY_MASK
    dsll    a2, a2, WRDQ_CLK_DELAY_4_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, WRDQ_CLK_DELAY_4_OFFSET
    or      a1, a1, a2
    sd      a1, WRDQ_CLK_DELAY_4_ADDR(t8)

    ld      a1, WRDQ_CLK_DELAY_3_ADDR(t8)
    dli     a2, WRDQ_CLK_DELAY_MASK
    dsll    a2, a2, WRDQ_CLK_DELAY_3_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, WRDQ_CLK_DELAY_3_OFFSET
    or      a1, a1, a2
    sd      a1, WRDQ_CLK_DELAY_3_ADDR(t8)

    ld      a1, WRDQ_CLK_DELAY_2_ADDR(t8)
    dli     a2, WRDQ_CLK_DELAY_MASK
    dsll    a2, a2, WRDQ_CLK_DELAY_2_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, WRDQ_CLK_DELAY_2_OFFSET
    or      a1, a1, a2
    sd      a1, WRDQ_CLK_DELAY_2_ADDR(t8)

    ld      a1, WRDQ_CLK_DELAY_1_ADDR(t8)
    dli     a2, WRDQ_CLK_DELAY_MASK
    dsll    a2, a2, WRDQ_CLK_DELAY_1_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, WRDQ_CLK_DELAY_1_OFFSET
    or      a1, a1, a2
    sd      a1, WRDQ_CLK_DELAY_1_ADDR(t8)

    ld      a1, WRDQ_CLK_DELAY_0_ADDR(t8)
    dli     a2, WRDQ_CLK_DELAY_MASK
    dsll    a2, a2, WRDQ_CLK_DELAY_0_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, WRDQ_CLK_DELAY_0_OFFSET
    or      a1, a1, a2
    sd      a1, WRDQ_CLK_DELAY_0_ADDR(t8)
    b       40f
    nop
12:
    and     t5, t5, RDDATA_DELAY_MASK

    ld      a1, RDDATA_DELAY_8_ADDR(t8)
    dli     a2, RDDATA_DELAY_MASK
    dsll    a2, a2, RDDATA_DELAY_8_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, RDDATA_DELAY_8_OFFSET
    or      a1, a1, a2
    sd      a1, RDDATA_DELAY_8_ADDR(t8)

    ld      a1, RDDATA_DELAY_7_ADDR(t8)
    dli     a2, RDDATA_DELAY_MASK
    dsll    a2, a2, RDDATA_DELAY_7_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, RDDATA_DELAY_7_OFFSET
    or      a1, a1, a2
    sd      a1, RDDATA_DELAY_7_ADDR(t8)

    ld      a1, RDDATA_DELAY_6_ADDR(t8)
    dli     a2, RDDATA_DELAY_MASK
    dsll    a2, a2, RDDATA_DELAY_6_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, RDDATA_DELAY_6_OFFSET
    or      a1, a1, a2
    sd      a1, RDDATA_DELAY_6_ADDR(t8)

    ld      a1, RDDATA_DELAY_5_ADDR(t8)
    dli     a2, RDDATA_DELAY_MASK
    dsll    a2, a2, RDDATA_DELAY_5_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, RDDATA_DELAY_5_OFFSET
    or      a1, a1, a2
    sd      a1, RDDATA_DELAY_5_ADDR(t8)

    ld      a1, RDDATA_DELAY_4_ADDR(t8)
    dli     a2, RDDATA_DELAY_MASK
    dsll    a2, a2, RDDATA_DELAY_4_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, RDDATA_DELAY_4_OFFSET
    or      a1, a1, a2
    sd      a1, RDDATA_DELAY_4_ADDR(t8)

    ld      a1, RDDATA_DELAY_3_ADDR(t8)
    dli     a2, RDDATA_DELAY_MASK
    dsll    a2, a2, RDDATA_DELAY_3_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, RDDATA_DELAY_3_OFFSET
    or      a1, a1, a2
    sd      a1, RDDATA_DELAY_3_ADDR(t8)

    ld      a1, RDDATA_DELAY_2_ADDR(t8)
    dli     a2, RDDATA_DELAY_MASK
    dsll    a2, a2, RDDATA_DELAY_2_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, RDDATA_DELAY_2_OFFSET
    or      a1, a1, a2
    sd      a1, RDDATA_DELAY_2_ADDR(t8)

    ld      a1, RDDATA_DELAY_1_ADDR(t8)
    dli     a2, RDDATA_DELAY_MASK
    dsll    a2, a2, RDDATA_DELAY_1_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, RDDATA_DELAY_1_OFFSET
    or      a1, a1, a2
    sd      a1, RDDATA_DELAY_1_ADDR(t8)

    ld      a1, RDDATA_DELAY_0_ADDR(t8)
    dli     a2, RDDATA_DELAY_MASK
    dsll    a2, a2, RDDATA_DELAY_0_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, RDDATA_DELAY_0_OFFSET
    or      a1, a1, a2
    sd      a1, RDDATA_DELAY_0_ADDR(t8)
    b       40f
    nop
13:
    and     t5, t5, RDDQS_LT_HALF_MASK

    ld      a1, RDDQS_LT_HALF_8_ADDR(t8)
    dli     a2, RDDQS_LT_HALF_MASK
    dsll    a2, a2, RDDQS_LT_HALF_8_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, RDDQS_LT_HALF_8_OFFSET
    or      a1, a1, a2
    sd      a1, RDDQS_LT_HALF_8_ADDR(t8)

    ld      a1, RDDQS_LT_HALF_7_ADDR(t8)
    dli     a2, RDDQS_LT_HALF_MASK
    dsll    a2, a2, RDDQS_LT_HALF_7_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, RDDQS_LT_HALF_7_OFFSET
    or      a1, a1, a2
    sd      a1, RDDQS_LT_HALF_7_ADDR(t8)

    ld      a1, RDDQS_LT_HALF_6_ADDR(t8)
    dli     a2, RDDQS_LT_HALF_MASK
    dsll    a2, a2, RDDQS_LT_HALF_6_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, RDDQS_LT_HALF_6_OFFSET
    or      a1, a1, a2
    sd      a1, RDDQS_LT_HALF_6_ADDR(t8)

    ld      a1, RDDQS_LT_HALF_5_ADDR(t8)
    dli     a2, RDDQS_LT_HALF_MASK
    dsll    a2, a2, RDDQS_LT_HALF_5_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, RDDQS_LT_HALF_5_OFFSET
    or      a1, a1, a2
    sd      a1, RDDQS_LT_HALF_5_ADDR(t8)

    ld      a1, RDDQS_LT_HALF_4_ADDR(t8)
    dli     a2, RDDQS_LT_HALF_MASK
    dsll    a2, a2, RDDQS_LT_HALF_4_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, RDDQS_LT_HALF_4_OFFSET
    or      a1, a1, a2
    sd      a1, RDDQS_LT_HALF_4_ADDR(t8)

    ld      a1, RDDQS_LT_HALF_3_ADDR(t8)
    dli     a2, RDDQS_LT_HALF_MASK
    dsll    a2, a2, RDDQS_LT_HALF_3_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, RDDQS_LT_HALF_3_OFFSET
    or      a1, a1, a2
    sd      a1, RDDQS_LT_HALF_3_ADDR(t8)

    ld      a1, RDDQS_LT_HALF_2_ADDR(t8)
    dli     a2, RDDQS_LT_HALF_MASK
    dsll    a2, a2, RDDQS_LT_HALF_2_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, RDDQS_LT_HALF_2_OFFSET
    or      a1, a1, a2
    sd      a1, RDDQS_LT_HALF_2_ADDR(t8)

    ld      a1, RDDQS_LT_HALF_1_ADDR(t8)
    dli     a2, RDDQS_LT_HALF_MASK
    dsll    a2, a2, RDDQS_LT_HALF_1_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, RDDQS_LT_HALF_1_OFFSET
    or      a1, a1, a2
    sd      a1, RDDQS_LT_HALF_1_ADDR(t8)

    ld      a1, RDDQS_LT_HALF_0_ADDR(t8)
    dli     a2, RDDQS_LT_HALF_MASK
    dsll    a2, a2, RDDQS_LT_HALF_0_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, RDDQS_LT_HALF_0_OFFSET
    or      a1, a1, a2
    sd      a1, RDDQS_LT_HALF_0_ADDR(t8)
    b       40f
    nop
14:
    and     t5, t5, WRDQS_LT_HALF_MASK

    ld      a1, WRDQS_LT_HALF_8_ADDR(t8)
    dli     a2, WRDQS_LT_HALF_MASK
    dsll    a2, a2, WRDQS_LT_HALF_8_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, WRDQS_LT_HALF_8_OFFSET
    or      a1, a1, a2
    sd      a1, WRDQS_LT_HALF_8_ADDR(t8)

    ld      a1, WRDQS_LT_HALF_7_ADDR(t8)
    dli     a2, WRDQS_LT_HALF_MASK
    dsll    a2, a2, WRDQS_LT_HALF_7_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, WRDQS_LT_HALF_7_OFFSET
    or      a1, a1, a2
    sd      a1, WRDQS_LT_HALF_7_ADDR(t8)

    ld      a1, WRDQS_LT_HALF_6_ADDR(t8)
    dli     a2, WRDQS_LT_HALF_MASK
    dsll    a2, a2, WRDQS_LT_HALF_6_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, WRDQS_LT_HALF_6_OFFSET
    or      a1, a1, a2
    sd      a1, WRDQS_LT_HALF_6_ADDR(t8)

    ld      a1, WRDQS_LT_HALF_5_ADDR(t8)
    dli     a2, WRDQS_LT_HALF_MASK
    dsll    a2, a2, WRDQS_LT_HALF_5_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, WRDQS_LT_HALF_5_OFFSET
    or      a1, a1, a2
    sd      a1, WRDQS_LT_HALF_5_ADDR(t8)

    ld      a1, WRDQS_LT_HALF_4_ADDR(t8)
    dli     a2, WRDQS_LT_HALF_MASK
    dsll    a2, a2, WRDQS_LT_HALF_4_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, WRDQS_LT_HALF_4_OFFSET
    or      a1, a1, a2
    sd      a1, WRDQS_LT_HALF_4_ADDR(t8)

    ld      a1, WRDQS_LT_HALF_3_ADDR(t8)
    dli     a2, WRDQS_LT_HALF_MASK
    dsll    a2, a2, WRDQS_LT_HALF_3_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, WRDQS_LT_HALF_3_OFFSET
    or      a1, a1, a2
    sd      a1, WRDQS_LT_HALF_3_ADDR(t8)

    ld      a1, WRDQS_LT_HALF_2_ADDR(t8)
    dli     a2, WRDQS_LT_HALF_MASK
    dsll    a2, a2, WRDQS_LT_HALF_2_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, WRDQS_LT_HALF_2_OFFSET
    or      a1, a1, a2
    sd      a1, WRDQS_LT_HALF_2_ADDR(t8)

    ld      a1, WRDQS_LT_HALF_1_ADDR(t8)
    dli     a2, WRDQS_LT_HALF_MASK
    dsll    a2, a2, WRDQS_LT_HALF_1_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, WRDQS_LT_HALF_1_OFFSET
    or      a1, a1, a2
    sd      a1, WRDQS_LT_HALF_1_ADDR(t8)

    ld      a1, WRDQS_LT_HALF_0_ADDR(t8)
    dli     a2, WRDQS_LT_HALF_MASK
    dsll    a2, a2, WRDQS_LT_HALF_0_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, WRDQS_LT_HALF_0_OFFSET
    or      a1, a1, a2
    sd      a1, WRDQS_LT_HALF_0_ADDR(t8)
    b       40f
    nop
15:
    and     t5, t5, WRDQ_LT_HALF_MASK

    ld      a1, WRDQ_LT_HALF_8_ADDR(t8)
    dli     a2, WRDQ_LT_HALF_MASK
    dsll    a2, a2, WRDQ_LT_HALF_8_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, WRDQ_LT_HALF_8_OFFSET
    or      a1, a1, a2
    sd      a1, WRDQ_LT_HALF_8_ADDR(t8)

    ld      a1, WRDQ_LT_HALF_7_ADDR(t8)
    dli     a2, WRDQ_LT_HALF_MASK
    dsll    a2, a2, WRDQ_LT_HALF_7_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, WRDQ_LT_HALF_7_OFFSET
    or      a1, a1, a2
    sd      a1, WRDQ_LT_HALF_7_ADDR(t8)

    ld      a1, WRDQ_LT_HALF_6_ADDR(t8)
    dli     a2, WRDQ_LT_HALF_MASK
    dsll    a2, a2, WRDQ_LT_HALF_6_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, WRDQ_LT_HALF_6_OFFSET
    or      a1, a1, a2
    sd      a1, WRDQ_LT_HALF_6_ADDR(t8)

    ld      a1, WRDQ_LT_HALF_5_ADDR(t8)
    dli     a2, WRDQ_LT_HALF_MASK
    dsll    a2, a2, WRDQ_LT_HALF_5_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, WRDQ_LT_HALF_5_OFFSET
    or      a1, a1, a2
    sd      a1, WRDQ_LT_HALF_5_ADDR(t8)

    ld      a1, WRDQ_LT_HALF_4_ADDR(t8)
    dli     a2, WRDQ_LT_HALF_MASK
    dsll    a2, a2, WRDQ_LT_HALF_4_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, WRDQ_LT_HALF_4_OFFSET
    or      a1, a1, a2
    sd      a1, WRDQ_LT_HALF_4_ADDR(t8)

    ld      a1, WRDQ_LT_HALF_3_ADDR(t8)
    dli     a2, WRDQ_LT_HALF_MASK
    dsll    a2, a2, WRDQ_LT_HALF_3_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, WRDQ_LT_HALF_3_OFFSET
    or      a1, a1, a2
    sd      a1, WRDQ_LT_HALF_3_ADDR(t8)

    ld      a1, WRDQ_LT_HALF_2_ADDR(t8)
    dli     a2, WRDQ_LT_HALF_MASK
    dsll    a2, a2, WRDQ_LT_HALF_2_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, WRDQ_LT_HALF_2_OFFSET
    or      a1, a1, a2
    sd      a1, WRDQ_LT_HALF_2_ADDR(t8)

    ld      a1, WRDQ_LT_HALF_1_ADDR(t8)
    dli     a2, WRDQ_LT_HALF_MASK
    dsll    a2, a2, WRDQ_LT_HALF_1_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, WRDQ_LT_HALF_1_OFFSET
    or      a1, a1, a2
    sd      a1, WRDQ_LT_HALF_1_ADDR(t8)

    ld      a1, WRDQ_LT_HALF_0_ADDR(t8)
    dli     a2, WRDQ_LT_HALF_MASK
    dsll    a2, a2, WRDQ_LT_HALF_0_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, WRDQ_LT_HALF_0_OFFSET
    or      a1, a1, a2
    sd      a1, WRDQ_LT_HALF_0_ADDR(t8)
    b       40f
    nop
40:
    sync
    b        41b
    nop

90:
    /* recover the ra */
    move    ra, t1
#endif

#ifdef DEBUG_DDR_PARAM   //debug
    /* store the ra */
    move    t1, ra

    PRINTSTR("\r\nChange some parameters of MC:");
1:
    PRINTSTR("\r\nPlease input the register number you want to change!!!(0xfff:jump out.): ");
    dli     t6, 0x00
    bal     inputaddress
    nop
    move    t5, v0
    
    dli     a1, 0x320
    bge     t5, a1, 2f    #if input address offset exceed range,jump out
    nop
    and     t5, t5, 0xff8
    daddu   t5, t5, t8

    PRINTSTR("\r\nPlease input the data-hex: ");
    dli     t6, 0x00
    bal     inputaddress
    nop
    sd      v0, 0x0(t5)    #v0 is the input value

    //print the new register value
    move    t6, t5
    PRINTSTR("\r\nRegister 0x")
    dsubu   t5, t5, t8
    move    a0, t5
    bal     hexserial
    nop
    PRINTSTR(": ")
    ld      t6, 0x0(t6)
    dsrl    a0, t6, 32
    bal     hexserial
    nop
    move    a0, t6
    bal     hexserial
    nop

    b       1b
    nop
2:    
    /* recover the ra */
    move    ra, t1
#endif
    sync

    ############start##########
    /***** set start to 1,start to initialize SDRAM *****/
    daddiu  v0, t8, START_ADDR
    dli     a2, 0x1
    dsll    a2, a2, START_OFFSET
    ld      a1, 0x0(v0)
    or      a1, a1, a2
    sd      a1, 0x0(v0)
    sync

#if 0
    /* store the ra */
    move    t1, ra

    PRINTSTR("run to after set start to 2\r\n")

    /* recover the ra */
    move    ra, t1
#endif

wait_dram_init:
    //wait initialization complete 
 //delay
    dli     v0, 0x100
1:
    daddi   v0, v0, -1
    bnez    v0, 1b
    nop  

#if 1
    /* store the ra */
    move    t1, ra

    PRINTSTR("run to wait dram init ok!3\r\n")

    /* recover the ra */
    move    ra, t1
#endif

    daddiu  v0, t8, DRAM_INIT_ADDR
    GET_MC_CS_MAP
1:
    ld      a2, 0x0(v0)
    dsrl    a2, a2, DRAM_INIT_OFFSET
    and     a2, a2, a1
    //bne     a1, a2, 1b
    bne     a1, a2, wait_dram_init
    nop

#if 0
    dli     a2, 0x4000
1:
    daddiu  a2, a2, -1
    bnez    a2, 1b
    nop
#endif
    ###############################
ddr2_config_end:
    jr      ra
    nop
    .end    ddr2_config


//subroutine: mc_init
//input: t3--MC select, 0--MC0; 1--MC1  --obselete
        .global mc_init
        .ent    mc_init
        .set    noreorder
        .set    mips3
mc_init:
    move    t7, ra

    sync
    nop
    nop
    nop
    nop
    GET_NODE_ID_a0;
    XBAR_CONFIG_NODE_a0(0x10, \
                    0x0000000000000000, \
                    0xfffffffff0000000, \
                    0x00000000000000f0)
    sync
    nop
    nop
    nop
    nop

    PRINTSTR("\r\nEnable register space of MEMORY\r\n")
    lw      a2, 0x0(t2)
    li      v1, 0x1
    GET_NODE_ID_a1
    and     a1, a1, 0x1
    beqz    a1, 1f
    nop
    sll     v1, v1, 5
1:
    sll     v1, v1, DDR_CONFIG_DISABLE_OFFSET
    not     v1, v1
    and     a2, a2, v1
    sw      a2, 0x0(t2)

    GET_NODE_ID_a0;
    dli     a1, DDR_MC_CONFIG_BASE
    or      t8, a0, a1
    bal     ddr2_config
    nop

#ifndef ARB_LEVEL
#ifdef FIX_DDR_PARAM
#define CLEAR_HALF_CLK_SHIFT
#define GET_ARB_LEVEL_NODE_ID   dli a1, 0x3; and a1, a1, s1;
    //save t2
    move    t5, t2

    GET_NODE_ID_a1
    dli     a0, 0x0
    beq     a0, a1, 10f
    nop
    daddu   a0, a0, 0x1
    beq     a0, a1, 1f
    nop
    daddu   a0, a0, 0x1
    beq     a0, a1, 2f
    nop
    daddu   a0, a0, 0x1
    beq     a0, a1, 3f
    nop
    b       8f
    nop
10:
    dla     t1, N0_fix_ddr_reg_data
    b       4f
    nop
1:
    dla     t1, N1_fix_ddr_reg_data
    b       4f
    nop
2:
    dla     t1, N2_fix_ddr_reg_data
    b       4f
    nop
3:
    dla     t1, N3_fix_ddr_reg_data
    b       4f
    nop
4:
    daddu   t1, t1, s0
    dli     t6, 0x2
Fix_ddr_param:
    ld      t2, 0x0(t1)
#include "ARB_Write_Leveled_param.S"
    daddu   t6, t6, 0x1
    daddu   t1, t1, 0x8
    dli     a1, 0x7
    blt     t6, a1, Fix_ddr_param
    nop
8:

    move    t2, t5
#endif
#endif

#if 1   //def  DEBUG_DDR_PARAM   //print registers
    PRINTSTR("The MC param is:\r\n")
    dli     t1, DDR_PARAM_NUM
    GET_NODE_ID_a0
    dli     t5, DDR_MC_CONFIG_BASE
    or      t5, t5, a0
1:
    ld      t6, 0x0(t5)
    move    a0, t5
    and     a0, a0, 0xfff
    bal     hexserial
    nop
    PRINTSTR(":  ")
    dsrl    a0, t6, 32
    bal     hexserial
    nop
    //PRINTSTR("  ")
    move    a0, t6
    bal     hexserial
    nop
    PRINTSTR("\r\n")

    daddiu  t1, t1, -1
    daddiu  t5, t5, 8
    bnez    t1, 1b
    nop
#endif
#if 1   //HARD leveling
    PRINTSTR("Start Hard Leveling...\r\n")

    GET_NODE_ID_a0
    dli     t8, DDR_MC_CONFIG_BASE
    or      t8, t8, a0

    bal     ddr3_leveling
    nop

#if 1   //def  DEBUG_DDR_PARAM   //print registers
    PRINTSTR("The MC param after leveling is:\r\n")
    dli     t1, DDR_PARAM_NUM
    GET_NODE_ID_a0
    dli     t5, DDR_MC_CONFIG_BASE
    or      t5, t5, a0
1:
    ld      t6, 0x0(t5)
    move    a0, t5
    and     a0, a0, 0xfff
    bal     hexserial
    nop
    PRINTSTR(":  ")
    dsrl    a0, t6, 32
    bal     hexserial
    nop
    //PRINTSTR("  ")
    move    a0, t6
    bal     hexserial
    nop
    PRINTSTR("\r\n")

    daddiu  t1, t1, -1
    daddiu  t5, t5, 8
    bnez    t1, 1b
    nop
#endif
#if 0   //def  DEBUG_DDR_PARAM   //Change parameters of MC

    GET_NODE_ID_a0;
    dli     a1, DDR_MC_CONFIG_BASE
    or      t8, a0, a1

    /* store the ra */
    move    t1, ra

    PRINTSTR("\r\nChange some parameters of MC:");
1:
    PRINTSTR("\r\nPlease input the register number you want to change!!!(0xfff:jump out.): ");
    dli     t6, 0x00
    bal     inputaddress
    nop
    move    t5, v0
    
    dli     a1, 0x320
    bge     t5, a1, 2f    #if input address offset exceed range,jump out
    nop
    and     t5, t5, 0xff8
    daddu   t5, t5, t8

    PRINTSTR("\r\nPlease input the data-hex: ");
    dli     t6, 0x00
    bal     inputaddress
    nop
    sd      v0, 0x0(t5)    #v0 is the input value

    //print the new register value
    move    t6, t5
    PRINTSTR("\r\nRegister 0x")
    dsubu   t5, t5, t8
    move    a0, t5
    bal     hexserial
    nop
    PRINTSTR(": ")
    ld      t6, 0x0(t6)
    dsrl    a0, t6, 32
    bal     hexserial
    nop
    move    a0, t6
    bal     hexserial
    nop

    b        1b
    nop
2:    
    /* recover the ra */
    move    ra, t1
#endif

    //Re-set t0&t2 because mc_init(ddr3_leveling) will change t0~t9
    GET_NODE_ID_a0
    dli     t2, 0x900000001fe00180
    dli     t0, 0x900000003ff00000
    or      t2, t2, a0
    or      t0, t0, a0
    //add 3B register location offset
    dsrl    a0, a0, 44
    dsll    a0, a0, 14
    or      t0, t0, a0
    //clear odd NODE_ID to even for chip config
    dli     a1, 0x1
    dsll    a1, a1, 44
    not     a1, a1
    and     t2, t2, a1

#endif

    PRINTSTR("\r\nDisable register space of MEMORY\r\n")
    lw      a2, 0x0(t2)
    li      v1, 0x1
    GET_NODE_ID_a1
    and     a1, a1, 0x1
    beqz    a1, 1f
    nop
    sll     v1, v1, 5
1:
    sll     v1, v1, DDR_CONFIG_DISABLE_OFFSET
    or      a2, a2, v1
    sw      a2, 0x0(t2)

//cxk
#ifdef  ARB_LEVEL
    sync
    nop
    nop
    nop
    nop
    //route 0x1000000000 ~ 0x1FFFFFFFFF(64G) to MC for ARB_level
    GET_NODE_ID_a0;
    XBAR_CONFIG_NODE_a0(0x20, \
                    0x0000001000000000, \
                    0xFFFFFFF000000000, \
                    0x00000000000000F0)
    sync
    nop
    nop
    nop
    nop

#ifdef  DEBUG_DDR_PARAM
    PRINTSTR("\r\nSkip Memory training?(0: use mark to decide;1: skip ARB_level;)\r\n");
    dli     t6, 0x00
    bal     inputaddress    #input value stored in v0
    nop
    bnez    v0, 1f
    nop
#endif
    //read ARB_level signal
    dla     a2, ddr2_leveled_mark
    daddu   a2, a2, s0
    ld      a2, 0x0(a2)
    bnez    a2, 1f
    nop

    bal     ARB_level
    nop
1:
#endif

#ifndef  DISABLE_DIMM_ECC
    //Init ECC according to DIMM ECC info
    GET_DIMM_ECC
    beqz    a1, 4f
    nop
    TTYDBG("ECC init start(maybe take 1 minute or so)....\r\n")

    TTYDBG("Enable MC read buffer\r\n")
    GET_NODE_ID_a1
    and     a1, a1, 0x1 //a1 == 0(even NODE) or 1(odd NODE)
    dsll    a2, a1, 2   //a2 == 0(even NODE) or 4(odd NODE)
    daddu   a1, a2, a1  //a1 == 0(even NODE) or 5(odd NODE)
    lw      a2, 0x0(t2)
    li      a0, 0x40    //bit[6]--MC0_read_buffer_bypass
    sll     a0, a0, a1  //sll 0 or 5
    not     a0, a0
    and     a2, a2, a0
    sw      a2, 0x0(t2)

    //PRINTSTR("\r\nEnable register space of MEMORY\r\n")
    lw      a2, 0x0(t2)
    li      v1, 0x1
    GET_NODE_ID_a1
    and     a1, a1, 0x1
    beqz    a1, 1f
    nop
    sll     v1, v1, 5
1:
    sll     v1, v1, DDR_CONFIG_DISABLE_OFFSET
    not     v1, v1
    and     a2, a2, v1
    sw      a2, 0x0(t2)

    //set ecc_disable_w_uc_err to 1'b1
    ld      a2, ECC_DISABLE_W_UC_ERR_ADDR(t8)
    dli     a1, 0xff
    dsll    a1, a1, ECC_DISABLE_W_UC_ERR_OFFSET
    not     a1, a1
    and     a2, a2, a1
    dli     a1, 0x01
    dsll    a1, a1, ECC_DISABLE_W_UC_ERR_OFFSET
    or      a2, a2, a1
    sd      a2, ECC_DISABLE_W_UC_ERR_ADDR(t8)
    //enable ECC module with ecc reporting and correcting.
    ld      a2, CTRL_RAW_ADDR(t8)
    dli     a1, 0xff
    dsll    a1, a1, CTRL_RAW_OFFSET
    not     a1, a1
    and     a2, a2, a1
    dli     a1, 0x03
    dsll    a1, a1, CTRL_RAW_OFFSET
    or      a2, a2, a1
    sd      a2, CTRL_RAW_ADDR(t8)

    //PRINTSTR("\r\nDisable register space of MEMORY\r\n")
    lw      a2, 0x0(t2)
    li      v1, 0x1
    GET_NODE_ID_a1
    and     a1, a1, 0x1
    beqz    a1, 1f
    nop
    sll     v1, v1, 5
1:
    sll     v1, v1, DDR_CONFIG_DISABLE_OFFSET
    or      a2, a2, v1
    sw      a2, 0x0(t2)

    //route 0x1000000000 ~ 0x1FFFFFFFFF(64G) to MC for ECC init
    sync
    nop
    nop
    nop
    nop
    GET_NODE_ID_a0;
    XBAR_CONFIG_NODE_a0(0x20, \
                    0x0000001000000000, \
                    0xFFFFFFF000000000, \
                    0x00000000000000F0)
    sync
    nop
    nop
    nop
    nop
//init mem to all 0
    dli     t1, 0x9800001000000000
    GET_NODE_ID_a0
    or      t1, t1, a0
    GET_MC0_MEMSIZE
    dsll    a1, a1, 29   //a1*512M
    daddu   t3, t1, a1
    dli     a1, 0x0
//write memory
1:
    bgeu    t1, t3, 1f
    nop

    sd      a1, 0x0(t1)
    sd      a1, 0x8(t1)
    sd      a1, 0x10(t1)
    sd      a1, 0x18(t1)
    sync
    //hit write back invalidate(D-/S-cache) to memory
    cache   0x15, 0x0(t1)
    cache   0x17, 0x0(t1)
    daddu   t1, t1, 0x20
    b       1b
    nop
1:

#if 1
    //PRINTSTR("\r\nEnable register space of MEMORY\r\n")
    lw      a2, 0x0(t2)
    li      v1, 0x1
    GET_NODE_ID_a1
    and     a1, a1, 0x1
    beqz    a1, 1f
    nop
    sll     v1, v1, 5
1:
    sll     v1, v1, DDR_CONFIG_DISABLE_OFFSET
    not     v1, v1
    and     a2, a2, v1
    sw      a2, 0x0(t2)

    GET_NODE_ID_a0;
    dli     t8, DDR_MC_CONFIG_BASE
    or      t8, t8, a0
    //clear disable_w_uc_err
    ld      a2, ECC_DISABLE_W_UC_ERR_ADDR(t8)
    dli     a1, 0xff
    dsll    a1, a1, ECC_DISABLE_W_UC_ERR_OFFSET
    not     a1, a1
    and     a2, a2, a1
    dli     a1, 0x00
    dsll    a1, a1, ECC_DISABLE_W_UC_ERR_OFFSET
    or      a2, a2, a1
    sd      a2, ECC_DISABLE_W_UC_ERR_ADDR(t8)

    //PRINTSTR("\r\nDisable register space of MEMORY\r\n")
    lw      a2, 0x0(t2)
    li      v1, 0x1
    GET_NODE_ID_a1
    and     a1, a1, 0x1
    beqz    a1, 1f
    nop
    sll     v1, v1, 5
1:
    sll     v1, v1, DDR_CONFIG_DISABLE_OFFSET
    or      a2, a2, v1
    sw      a2, 0x0(t2)
#endif

    TTYDBG("MC ECC init done.\r\n")
4:
#endif
    move    ra, t7
    jr      ra
    nop
    .end    mc_init
