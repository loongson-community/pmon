/**********************************
    Author: chenxinke
    Date:   20110125
    v2.0
        support both 3A and 3A3
        MC0/1 both use the same subroutine, add an input param: t3.
    input param:
        t8: input, Memory Controller config register base
        t3: controller select(config param select)
            0: MC0
            1: MC1
        t7(option ARB_LEVEL) : do arb level
            0: no, use stored leveled param
            1: yes, use default param
**********************************/

        .global ddr2_config
        .ent    ddr2_config
        .set    noreorder
        .set    mips3
ddr2_config:
#ifdef  ARB_LEVEL
    //if use leveled ddr param, the param location is fixed
    bnez    t7, 1f
    nop
    dla     a2, ddr2_reg_data_mc0_leveled
    beqz    t3, 21f
    nop
    dla     a2, ddr2_reg_data_mc1_leveled
21:
#ifdef  MULTI_CHIP
    GET_NODE_ID_a1
    beqz    a1, 21f
    nop
    dla     a2, n1_ddr2_reg_data_mc0_leveled
    beqz    t3, 21f
    nop
    dla     a2, n1_ddr2_reg_data_mc1_leveled
21:
#endif
    b       4f
    nop
1:
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
    beqz    t3, 21f
    nop
    dla     a2, ddr2_reg_data_mc1
21:
#ifdef  MULTI_NODE_DDR_PARAM
    GET_NODE_ID_a1
    beqz    a1, 4f
    nop
    dla     a2, n1_ddr2_reg_data
    beqz    t3, 21f
    nop
    dla     a2, n1_ddr2_reg_data_mc1
21:
#endif
    b       4f
    nop
1:  //RDIMM
    dla     a2, ddr2_RDIMM_reg_data
    beqz    t3, 21f
    nop
    dla     a2, ddr2_RDIMM_reg_data_mc1
21:
#ifdef  MULTI_NODE_DDR_PARAM
    GET_NODE_ID_a1
    beqz    a1, 4f
    nop
    dla     a2, n1_ddr2_RDIMM_reg_data
    beqz    t3, 21f
    nop
    dla     a2, n1_ddr2_RDIMM_reg_data_mc1
21:
#endif
    b       4f
    nop
3:  //DDR3
    GET_DIMM_TYPE
    bnez    a1, 1f
    nop
    //UDIMM
    dla     a2, ddr3_reg_data
    beqz    t3, 21f
    nop
    dla     a2, ddr3_reg_data_mc1
21:
#ifdef  MULTI_NODE_DDR_PARAM
    GET_NODE_ID_a1
    beqz    a1, 4f
    nop
    dla     a2, n1_ddr3_reg_data
    beqz    t3, 21f
    nop
    dla     a2, n1_ddr3_reg_data_mc1
21:
#endif
    b       4f
    nop
1:  //RDIMM
    dla     a2, ddr3_RDIMM_reg_data
    beqz    t3, 21f
    nop
    dla     a2, ddr3_RDIMM_reg_data_mc1
21:
#ifdef  MULTI_NODE_DDR_PARAM
    GET_NODE_ID_a1
    beqz    a1, 4f
    nop
    dla     a2, n1_ddr3_RDIMM_reg_data
    beqz    t3, 21f
    nop
    dla     a2, n1_ddr3_RDIMM_reg_data_mc1
21:
#endif
    b       4f
    nop
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
    daddiu  v0, v0, 0x10
    bnez    t1, 1b
    nop

#ifdef  ARB_LEVEL
    //if use leveled ddr param, don't re-write these param any more
    beqz    t7, 88f
    nop
#endif
    //rewrite eight_bank_mode
    ld      a2, EIGHT_BANK_MODE_ADDR(t8)
    dli     a1, 0xff
    dsll    a1, a1, EIGHT_BANK_MODE_OFFSET
    not     a1, a1
    and     a2, a2, a1
    GET_EIGHT_BANK
    dsll    a1, a1, EIGHT_BANK_MODE_OFFSET
    or      a2, a2, a1
    sd      a2, EIGHT_BANK_MODE_ADDR(t8)
    //rewrite column_size and addr_pins
    ld      a2, COLUMN_SIZE_ADDR(t8)
    dli     a1, 0xffffffff00ff00ff
    and     a2, a2, a1
    GET_ROW_SIZE
    dsll    a1, a1, ADDR_PINS_OFFSET
    or      a2, a2, a1
    GET_COL_SIZE
    daddu   a1, a1, 0x2
    dsll    a1, a1, COLUMN_SIZE_OFFSET
    or      a2, a2, a1
    sd      a2, COLUMN_SIZE_ADDR(t8)
    //rewrite cs_map
    ld      a2, CS_MAP_ADDR(t8)
    dli     a1, 0xff
    dsll    a1, a1, CS_MAP_OFFSET
    not     a1, a1
    and     a2, a2, a1
    GET_MC_CS_MAP
    dsll    a1, a1, CS_MAP_OFFSET
    or      a2, a2, a1
    sd      a2, CS_MAP_ADDR(t8)
    //reconfig address_mirroring
    ld      a2, ADDRESS_MIRROR_ADDR(t8)
    dli     a1, 0xff
    dsll    a1, a1, ADDRESS_MIRROR_OFFSET
    not     a1, a1
    and     a2, a2, a1
    GET_ADDR_MIRROR
    beqz    a1, 1f
    nop
    dli     a1, 0xa
1:
    dsll    a1, a1, ADDRESS_MIRROR_OFFSET
    or      a2, a2, a1
    sd      a2, ADDRESS_MIRROR_ADDR(t8)

    //reconfig ODT map
    //set default first
    dli     a2, 0x0804020100000000
    sd      a2, ODT_MAP_CS_ADDR(t8)

    //v0 store cs map
    GET_MC_CS_MAP
    move    v0, a1
#if 0   //Don't swap now, because we enable Dynamic ODT now, and
    //use different ODT value for WR and non-wr purpose
    //step 1: swap open wr odt if it's a Dual Rank DIMM
    //check cs_map[3]
    dsrl    a2, v0, 3
    beqz    a2, 1f
    nop
    //slot 1 is a DR DIMM
    ld      a2, ODT_MAP_CS_ADDR(t8)
    dli     a1, 0x0000ffffffffffff
    and     a2, a2, a1
    dli     a1, 0x0408000000000000
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
    dli     a1, 0xffff0000ffffffff
    and     a2, a2, a1
    dli     a1, 0x0000010200000000
    or      a2, a2, a1
    sd      a2, ODT_MAP_CS_ADDR(t8)
1:
#endif
    //step 2: open extra RD/WR ODT CS if there is 2 DIMM
    //check CS[0] and CS[2]
    dsrl    a2, v0, 2
    xor     a2, v0, a2
    and     a2, a2, 0x1
    bnez    a2, 1f
    nop
    //2 DIMM: open the extra rank of the non-target DIMM
//#define FINE_ODT_MAP_CFG
//the fine ODT map configure is board dependent(the layout of the 2 DIMMs)
#ifndef FINE_ODT_MAP_CFG
    //open the first rank of the non-target DIMM
    dli     a1, 0x0101040401010404
    ld      a2, ODT_MAP_CS_ADDR(t8)
    or      a2, a2, a1
    sd      a2, ODT_MAP_CS_ADDR(t8)
#else
#if 0   //for standard layout(cpu <0<1<2<3)
    //for slot 0, use the Rank 0(it is at the out side)
    ld      a2, ODT_MAP_CS_ADDR(t8)
    dli     a1, 0x0101000001010000
    or      a2, a2, a1
    sd      a2, ODT_MAP_CS_ADDR(t8)
    //for slot 1, check CS[3] to see slot 1(far side) is Dual Rank or Single Rank
    dli     a1, 0x0000040400000404
    dsrl    a2, v0, 3
    and     a2, a2, 0x1
    beqz    a2, 2f
    nop
    //Slot is Dual Rank, use the Rank 3(it is at the out side)
    dli     a1, 0x0000080800000808
2:
    ld      a2, ODT_MAP_CS_ADDR(t8)
    or      a2, a2, a1
    sd      a2, ODT_MAP_CS_ADDR(t8)
#endif
#if 1    //for 3A test board MC1 layout(cpu <1<0<3<2)
    //for slot 0, check CS[1] to see slot 0(near side) is Dual Rank or Single Rank
    dli     a1, 0x0101000001010000
    dsrl    a2, v0, 1
    and     a2, a2, 0x1
    beqz    a2, 2f
    nop
    //Slot is Dual Rank, use the Rank 1(it is at the out side)
    dli     a1, 0x0202000002020000
2:
    ld      a2, ODT_MAP_CS_ADDR(t8)
    or      a2, a2, a1
    sd      a2, ODT_MAP_CS_ADDR(t8)

    //for slot 1, use the Rank 2
    dli     a1, 0x0000040400000404
    ld      a2, ODT_MAP_CS_ADDR(t8)
    or      a2, a2, a1
    sd      a2, ODT_MAP_CS_ADDR(t8)
#endif
#endif
    b       2f
    nop

1:  //Only 1 DIMM
    //if it is DDR3_DIMM, Disable Dynamic ODT
    GET_SDRAM_TYPE
    dli     a2, 0x3
    bne     a2, a1, 2f
    nop
    //DDR3 DIMM, disable RTT_wr
    ld      a1, MR2_DATA_0_ADDR(t8)
    dli     a2, 0x3
    dsll    a2, a2, MR2_DATA_0_OFFSET + 9
    not     a2, a2
    and     a1, a1, a2
    sd      a1, MR2_DATA_0_ADDR(t8)

    ld      a1, MR2_DATA_1_ADDR(t8)
    dli     a2, 0x3
    dsll    a2, a2, MR2_DATA_1_OFFSET + 9
    not     a2, a2
    and     a1, a1, a2
    sd      a1, MR2_DATA_1_ADDR(t8)

    ld      a1, MR2_DATA_2_ADDR(t8)
    dli     a2, 0x3
    dsll    a2, a2, MR2_DATA_2_OFFSET + 9
    not     a2, a2
    and     a1, a1, a2
    sd      a1, MR2_DATA_2_ADDR(t8)

    ld      a1, MR2_DATA_3_ADDR(t8)
    dli     a2, 0x3
    dsll    a2, a2, MR2_DATA_3_OFFSET + 9
    not     a2, a2
    and     a1, a1, a2
    sd      a1, MR2_DATA_3_ADDR(t8)
2:

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
    dli     a1, 0xff
    dsll    a1, a1, CTRL_RAW_OFFSET
    not     a1, a1
    and     a2, a2, a1
    dli     a1, 0x02
    dsll    a1, a1, CTRL_RAW_OFFSET
    or      a2, a2, a1
    sd      a2, CTRL_RAW_ADDR(t8)

#define ADJUST_CLKLVL_DELAY
#ifdef  ADJUST_CLKLVL_DELAY
    //count Rank numeber in a0
    move    a0, $0
    GET_DIMM_TYPE
    bnez    a1, 2f
    nop
    //UDIMM -- count Rank number
    GET_MC_CS_MAP
    and     a2, a1, 0x1
    beqz    a2, 1f
    nop
    daddu   a0, a0, 0x1
1:
    dsrl    a1, a1, 1
    and     a2, a1, 0x1
    beqz    a2, 1f
    nop
    daddu   a0, a0, 0x1
1:
    dsrl    a1, a1, 1
    and     a2, a1, 0x1
    beqz    a2, 1f
    nop
    daddu   a0, a0, 0x1
1:
    dsrl    a1, a1, 1
    and     a2, a1, 0x1
    beqz    a2, 1f
    nop
    daddu   a0, a0, 0x1
1:
    b       8f
    nop
2:  //RDIMM -- count DIMM number(not Rank number)
    GET_MC_CS_MAP
    and     a2, a1, 0x1
    beqz    a2, 1f
    nop
    daddu   a0, a0, 0x1
1:
    dsrl    a1, a1, 2
    and     a2, a1, 0x1
    beqz    a2, 1f
    nop
    daddu   a0, a0, 0x1
1:

8:
    //compensate for heavier ADD/CTRL/CMD load
    dli     a2, 0x4
    dmul    a2, a2, a0

    dsll    a1, a2, 0x8
    or      a2, a2, a1
    dsll    a1, a1, 0x8
    or      a2, a2, a1

    ld      a1, CLKLVL_DELAY_0_ADDR(t8)
    dsll    a2, a2, CLKLVL_DELAY_0_OFFSET
    daddu   a1, a1, a2
    sd      a1, CLKLVL_DELAY_0_ADDR(t8)
#endif
88:

#ifdef DEBUG_DDR_PARAM   //debug
//input once, change all byte lanes parameters.
    /* store the ra */
    move    t1, ra

11:
    PRINTSTR("\r\nChange parameters:\r\n0--skip;1--clock;2--rdlvl_gate;3--rdlvl_dqs_delay;\r\n4--rdlvl_dqsn_delay;5--wrlvl_delay;6--wrlvl_dq_delay;7--phy_control_0;8--phy_control_1\r\n");
    dli     t6, 0x00
    bal     inputaddress
    nop
    beqz    v0, 90f;
    nop
    move    t5, v0
    PRINTSTR("\r\nPlease input the data-hex: ");
    dli     t6, 0x00
    bal     inputaddress
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
    PRINTSTR("\r\n--------Wrong selection: no parameter will be changed.");
    b       40f
    nop
1:
    and     t5, t5, 0x7f

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
    and     t5, t5, 0x7f

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
    and     t5, t5, 0x7f

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
    and     t5, t5, 0x7f

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
    and     t5, t5, 0x7f

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
    and     t5, t5, 0x7f

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
    ld      a1, PHY_CTRL_0_8_ADDR(t8)
    dli     a2, PHY_CTRL_0_MASK
    dsll    a2, a2, PHY_CTRL_0_8_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, PHY_CTRL_0_8_OFFSET
    or      a1, a1, a2
    sd      a1, PHY_CTRL_0_8_ADDR(t8)

    ld      a1, PHY_CTRL_0_7_ADDR(t8)
    dli     a2, PHY_CTRL_0_MASK
    dsll    a2, a2, PHY_CTRL_0_7_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, PHY_CTRL_0_7_OFFSET
    or      a1, a1, a2
    sd      a1, PHY_CTRL_0_7_ADDR(t8)

    ld      a1, PHY_CTRL_0_6_ADDR(t8)
    dli     a2, PHY_CTRL_0_MASK
    dsll    a2, a2, PHY_CTRL_0_6_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, PHY_CTRL_0_6_OFFSET
    or      a1, a1, a2
    sd      a1, PHY_CTRL_0_6_ADDR(t8)

    ld      a1, PHY_CTRL_0_5_ADDR(t8)
    dli     a2, PHY_CTRL_0_MASK
    dsll    a2, a2, PHY_CTRL_0_5_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, PHY_CTRL_0_5_OFFSET
    or      a1, a1, a2
    sd      a1, PHY_CTRL_0_5_ADDR(t8)

    ld      a1, PHY_CTRL_0_4_ADDR(t8)
    dli     a2, PHY_CTRL_0_MASK
    dsll    a2, a2, PHY_CTRL_0_4_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, PHY_CTRL_0_4_OFFSET
    or      a1, a1, a2
    sd      a1, PHY_CTRL_0_4_ADDR(t8)

    ld      a1, PHY_CTRL_0_3_ADDR(t8)
    dli     a2, PHY_CTRL_0_MASK
    dsll    a2, a2, PHY_CTRL_0_3_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, PHY_CTRL_0_3_OFFSET
    or      a1, a1, a2
    sd      a1, PHY_CTRL_0_3_ADDR(t8)

    ld      a1, PHY_CTRL_0_2_ADDR(t8)
    dli     a2, PHY_CTRL_0_MASK
    dsll    a2, a2, PHY_CTRL_0_2_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, PHY_CTRL_0_2_OFFSET
    or      a1, a1, a2
    sd      a1, PHY_CTRL_0_2_ADDR(t8)

    ld      a1, PHY_CTRL_0_1_ADDR(t8)
    dli     a2, PHY_CTRL_0_MASK
    dsll    a2, a2, PHY_CTRL_0_1_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, PHY_CTRL_0_1_OFFSET
    or      a1, a1, a2
    sd      a1, PHY_CTRL_0_1_ADDR(t8)

    ld      a1, PHY_CTRL_0_0_ADDR(t8)
    dli     a2, PHY_CTRL_0_MASK
    dsll    a2, a2, PHY_CTRL_0_0_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, PHY_CTRL_0_0_OFFSET
    or      a1, a1, a2
    sd      a1, PHY_CTRL_0_0_ADDR(t8)
    b       40f
    nop
8:
    ld      a1, PHY_CTRL_1_8_ADDR(t8)
    dli     a2, PHY_CTRL_1_MASK
    dsll    a2, a2, PHY_CTRL_1_8_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, PHY_CTRL_1_8_OFFSET
    or      a1, a1, a2
    sd      a1, PHY_CTRL_1_8_ADDR(t8)

    ld      a1, PHY_CTRL_1_7_ADDR(t8)
    dli     a2, PHY_CTRL_1_MASK
    dsll    a2, a2, PHY_CTRL_1_7_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, PHY_CTRL_1_7_OFFSET
    or      a1, a1, a2
    sd      a1, PHY_CTRL_1_7_ADDR(t8)

    ld      a1, PHY_CTRL_1_6_ADDR(t8)
    dli     a2, PHY_CTRL_1_MASK
    dsll    a2, a2, PHY_CTRL_1_6_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, PHY_CTRL_1_6_OFFSET
    or      a1, a1, a2
    sd      a1, PHY_CTRL_1_6_ADDR(t8)

    ld      a1, PHY_CTRL_1_5_ADDR(t8)
    dli     a2, PHY_CTRL_1_MASK
    dsll    a2, a2, PHY_CTRL_1_5_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, PHY_CTRL_1_5_OFFSET
    or      a1, a1, a2
    sd      a1, PHY_CTRL_1_5_ADDR(t8)

    ld      a1, PHY_CTRL_1_4_ADDR(t8)
    dli     a2, PHY_CTRL_1_MASK
    dsll    a2, a2, PHY_CTRL_1_4_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, PHY_CTRL_1_4_OFFSET
    or      a1, a1, a2
    sd      a1, PHY_CTRL_1_4_ADDR(t8)

    ld      a1, PHY_CTRL_1_3_ADDR(t8)
    dli     a2, PHY_CTRL_1_MASK
    dsll    a2, a2, PHY_CTRL_1_3_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, PHY_CTRL_1_3_OFFSET
    or      a1, a1, a2
    sd      a1, PHY_CTRL_1_3_ADDR(t8)

    ld      a1, PHY_CTRL_1_2_ADDR(t8)
    dli     a2, PHY_CTRL_1_MASK
    dsll    a2, a2, PHY_CTRL_1_2_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, PHY_CTRL_1_2_OFFSET
    or      a1, a1, a2
    sd      a1, PHY_CTRL_1_2_ADDR(t8)

    ld      a1, PHY_CTRL_1_1_ADDR(t8)
    dli     a2, PHY_CTRL_1_MASK
    dsll    a2, a2, PHY_CTRL_1_1_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, PHY_CTRL_1_1_OFFSET
    or      a1, a1, a2
    sd      a1, PHY_CTRL_1_1_ADDR(t8)

    ld      a1, PHY_CTRL_1_0_ADDR(t8)
    dli     a2, PHY_CTRL_1_MASK
    dsll    a2, a2, PHY_CTRL_1_0_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, PHY_CTRL_1_0_OFFSET
    or      a1, a1, a2
    sd      a1, PHY_CTRL_1_0_ADDR(t8)
    b       40f
    nop
40:
    sync
    b        11b
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
    PRINTSTR("\r\nPlease input the register number you want to change!!!(0xff:jump out.): ");
    dli     t6, 0x00
    bal     inputaddress
    nop
    move    t5, v0
    
    dli     a1, 0xb3
    bgt     t5, a1, 2f    #if input address offset exceed range,jump out
    nop
    dsll    t5, t5, 4    #t5 is the offset relative to a0
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
    dsrl    t5, t5, 4
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
    daddiu    v0, t8, 0x30
    dli     a2, 0x0000010000000000
    ld      a1, 0x0(v0)
    or      a1, a1, a2
    sd      a1, 0x0(v0)
    sync

    //wait initialization complete 
 //delay
    dli     v0, 0x100
1:
    bnez    v0, 1b
    daddi   v0, v0, -1
    nop

    daddiu  v0, t8, 0x960
1:
    ld      a1, 0x0(v0)
    andi    a1, a1, 0x100
    beqz    a1, 1b
    nop

    //wait(or make sure) dll is locked
    daddiu  v0, t8, 0x10
1:
    ld      a1, 0x0(v0)
    andi    a1, a1, 0x1
    beqz    a1, 1b
    nop

#if 0
//too long  delay can cause problem(stuck).
    //!!!!!!!!!!!!!!!!!!!!!!!!!!!
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
//input: t3--MC select, 0--MC0; 1--MC1
//       t7(option ARB_LEVEL)--do arb level, 0--not level; 1--do level;

        .global mc_init
        .ent    mc_init
        .set    noreorder
        .set    mips3
mc_init:
    move    t4, ra

    beqz    t3, 1f
    nop
    //shift MC1 DIMM info to low 32bit of s1
    dsrl    t5, s1, 32
    dli     a1, 0xffff0000
    and     t5, t5, a1
    dli     a1, 0xffffffff0000ffff
    and     s1, s1, a1
    or      s1, s1, t5
1:
    sync
    nop
    nop
    nop
    nop

    GET_NODE_ID_a0;
    XBAR_CONFIG_NODE_a0(0x10, \
                    0x000000000ff00000, \
                    0xfffffffffff00000, \
                    0x000000000ff000f0)
    beqz    t3, 1f
    nop
    GET_NODE_ID_a0;
    XBAR_CONFIG_NODE_a0(0x10, \
                    0x000000000ff00000, \
                    0xfffffffffff00000, \
                    0x000000000ff000f1)
1:

    sync
    nop
    nop
    nop
    nop

    TTYDBG("\r\nEnable register space of MEMORY\r\n")
    lw      a1, 0x0(t2)
    li      a2, 0x1
    sll     a2, a2, DDR_CONFIG_DISABLE_OFFSET
    not     a2, a2
    and     a1, a1, a2
    sw      a1, 0x0(t2)

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
    b       8f
    nop
10:
    dla     t1, N0_fix_ddr_reg_data_mc0
    beqz    t3, 11f
    nop
    dla     t1, N0_fix_ddr_reg_data_mc1
11:
    b       4f
    nop
1:
    dla     t1, N1_fix_ddr_reg_data_mc0
    beqz    t3, 11f
    nop
    dla     t1, N1_fix_ddr_reg_data_mc1
11:
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

    //ARB_Write_Leveled_param.S will disable DDR register space
    //TTYDBG("\r\nEnable register space of MEMORY\r\n")
    lw      a1, 0x0(t2)
    li      a2, 0x1
    sll     a2, a2, DDR_CONFIG_DISABLE_OFFSET
    not     a2, a2
    and     a1, a1, a2
    sw      a1, 0x0(t2)

#endif
#endif

#if 1   //def  DEBUG_DDR_PARAM   //print registers
    PRINTSTR("\r\nThe MC param is:\r\n")
    dli     t1, DDR_PARAM_NUM
    GET_NODE_ID_a0
    dli     t5, DDR_MC_CONFIG_BASE
    or      t5, t5, a0
1:
    ld      t6, 0x0(t5)
    dsrl    a0, t6, 32
    bal     hexserial
    nop
    PRINTSTR("  ")
    move    a0, t6
    bal     hexserial
    nop
    PRINTSTR("\r\n")

    daddiu  t1, t1, -1
    daddiu  t5, t5, 16
    bnez    t1, 1b
    nop
#endif

    TTYDBG("\r\nDisable register space of MEMORY\r\n")
    lw      a1,0x0(t2)
    li      a2, 0x1
    sll     a2, a2, DDR_CONFIG_DISABLE_OFFSET
    or      a1, a1, a2
    sw      a1,0x0(t2)

//cxk
#ifdef  ARB_LEVEL
#ifdef  DEBUG_DDR_PARAM
    PRINTSTR("\r\nSkip Memory training?(0: use mark to decide;1: skip ARB_level;)\r\n");
    dli     t6, 0x00
    bal     inputaddress    #input value stored in v0
    nop
    bnez    v0, 8f
    nop
#endif
    //read ARB_level
    beqz    t7, 8f
    nop

    //route 0x1000000000 ~ 0x1FFFFFFFFF(64G) to MC for ARB_level
    sync
    nop
    nop
    nop
    nop
    GET_NODE_ID_a0;
    XBAR_CONFIG_NODE_a0(ARB_TEMP_L2WINDOW_OFFSET, \
                    0x0000001000000000, \
                    0xFFFFFFF000000000, \
                    0x00000000000000F0)
    beqz    t3, 2f
    nop
    GET_NODE_ID_a0;
    XBAR_CONFIG_NODE_a0(ARB_TEMP_L2WINDOW_OFFSET, \
                    0x0000001000000000, \
                    0xFFFFFFF000000000, \
                    0x00000000000000F1)
2:
    sync
    nop
    nop
    nop
    nop

    bal     ARB_level
    nop

    sync
    nop
    nop
    nop
    nop

    L2XBAR_CLEAR_WINDOW(ARB_TEMP_L2WINDOW_OFFSET)

    sync
    nop
    nop
    nop
    nop

8:
#endif

#ifndef  DISABLE_DIMM_ECC
    //Init ECC according to DIMM ECC info
    GET_DIMM_ECC
    beqz    a1, 4f
    nop
    TTYDBG("ECC init start(maybe take 1 minute or so)....\r\n")

    //TTYDBG("Enable register space of MEMORY\r\n")
    lw      a1, 0x0(t2)
    li      a2, 0x1
    sll     a2, a2, DDR_CONFIG_DISABLE_OFFSET
    not     a2, a2
    and     a1, a1, a2
    sw      a1, 0x0(t2)

    //set ecc_disable_w_uc_err to 1'b1
    //TTYDBG("set ecc_disable_w_ur_err.\r\n")
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
    //TTYDBG("enable ECC.\r\n")
    ld      a2, CTRL_RAW_ADDR(t8)
    dli     a1, 0xff
    dsll    a1, a1, CTRL_RAW_OFFSET
    not     a1, a1
    and     a2, a2, a1
    dli     a1, 0x03
    dsll    a1, a1, CTRL_RAW_OFFSET
    or      a2, a2, a1
    sd      a2, CTRL_RAW_ADDR(t8)

    //TTYDBG("Disable register space of MEMORY\r\n")
    lw      a1,0x0(t2)
    li      a2, 0x1
    sll     a2, a2, DDR_CONFIG_DISABLE_OFFSET
    or      a1, a1, a2
    sw      a1,0x0(t2)

    //route 0x1000000000 ~ 0x1FFFFFFFFF(64G) to MC for ECC init
    sync
    nop
    nop
    nop
    nop
    GET_NODE_ID_a0;
    XBAR_CONFIG_NODE_a0(ARB_TEMP_L2WINDOW_OFFSET, \
                    0x0000001000000000, \
                    0xFFFFFFF000000000, \
                    0x00000000000000F0)
    beqz    t3, 1f
    nop
    GET_NODE_ID_a0;
    XBAR_CONFIG_NODE_a0(ARB_TEMP_L2WINDOW_OFFSET, \
                    0x0000001000000000, \
                    0xFFFFFFF000000000, \
                    0x00000000000000F1)

1:
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
    beqz    t3, 1f
    nop
    GET_MC1_MEMSIZE
1:
    dsll    a1, a1, 29   //a1*512M
    daddu   t3, t1, a1
//write memory
1:
    bgeu    t1, t3, 1f
    nop

    sd      $0, 0x0(t1)
    sd      $0, 0x8(t1)
    sd      $0, 0x10(t1)
    sd      $0, 0x18(t1)
    sync
    //hit write back invalidate(D-/S-cache) to memory
    cache   0x15, 0x0(t1)
    cache   0x17, 0x0(t1)
    daddu   t1, t1, 0x20
    b       1b
    nop
1:

    sync
    nop
    nop
    nop
    nop

    L2XBAR_CLEAR_WINDOW(ARB_TEMP_L2WINDOW_OFFSET)

    sync
    nop
    nop
    nop
    nop

    //TTYDBG("Enable register space of MEMORY\r\n")
    lw      a1, 0x0(t2)
    li      a2, 0x1
    sll     a2, a2, DDR_CONFIG_DISABLE_OFFSET
    not     a2, a2
    and     a1, a1, a2
    sw      a1, 0x0(t2)

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

    //TTYDBG("Disable register space of MEMORY\r\n")
    lw      a1,0x0(t2)
    li      a2, 0x1
    sll     a2, a2, DDR_CONFIG_DISABLE_OFFSET
    or      a1, a1, a2
    sw      a1,0x0(t2)

    TTYDBG("MC ECC init done.\r\n")
4:
#endif
#define CLEAR_INT_STATUS
#ifdef  CLEAR_INT_STATUS
    //clear possible out of physical range int_status bits introduced by arb_level or ECC initialization
    //TTYDBG("Enable register space of MEMORY\r\n")
    lw      a1, 0x0(t2)
    li      a2, 0x1
    sll     a2, a2, DDR_CONFIG_DISABLE_OFFSET
    not     a2, a2
    and     a1, a1, a2
    sw      a1, 0x0(t2)
    sync

    //set int_ack to 1
    ld      a1, MC_INT_ACK_ADDR(t8)
    dli     a2, 0x3
    dsll    a2, a2, MC_INT_ACK_OFFSET
    or      a1, a1, a2
    sd      a1, MC_INT_ACK_ADDR(t8)

    //delay some time
    dli     a2, 0x400
1:
    daddiu  a2, a2, -1
    bnez    a2, 1b
    nop

    //clear int_ack to 0
    ld      a1, MC_INT_ACK_ADDR(t8)
    dli     a2, 0x3
    dsll    a2, a2, MC_INT_ACK_OFFSET
    not     a2, a2
    and     a1, a1, a2
    sd      a1, MC_INT_ACK_ADDR(t8)

#if 0   //print registers
    PRINTSTR("\r\nThe MC param is:\r\n")
    dli     t1, DDR_PARAM_NUM
    GET_NODE_ID_a0
    dli     t5, DDR_MC_CONFIG_BASE
    or      t5, t5, a0
1:
    ld      t6, 0x0(t5)
    dsrl    a0, t6, 32
    bal     hexserial
    nop
    PRINTSTR("  ")
    move    a0, t6
    bal     hexserial
    nop
    PRINTSTR("\r\n")

    daddiu  t1, t1, -1
    daddiu  t5, t5, 16
    bnez    t1, 1b
    nop
#endif

    //TTYDBG("\r\nDisable register space of MEMORY\r\n")
    lw      a1,0x0(t2)
    li      a2, 0x1
    sll     a2, a2, DDR_CONFIG_DISABLE_OFFSET
    or      a1, a1, a2
    sw      a1,0x0(t2)
#endif

    //clear the L2 xbar window configured for mc_init
    sync
    nop
    nop
    nop
    nop

    L2XBAR_CLEAR_WINDOW(0x10)

    sync
    nop
    nop
    nop
    nop

    jr      t4
    nop
    .end    mc_init

#ifdef  CHECK_ARB_LEVEL_FREQ
LEAF(CHECK_DDR_FREQ_CHANGE)
/*********************
check whether the chip ddr clksel is changed
input:
    s3:
    [44:40] chip 0 ddr clksel
    [52:48] chip 1 ddr clksel
    t7: mc level info addr
output:
    v0: 0--not changed; !0--changed
*********************/
    ld      a0, 0x0(t7)
    dsrl    a0, a0, 32
    and     a0, a0, DDR_CLKSEL_MASK
    dsrl    a2, s3, 40
    GET_NODE_ID_a1
    dsll    a1, a1, 3
    dsrl    a2, a2, a1
    and     a2, a2, DDR_CLKSEL_MASK
    xor     v0, a0, a2

    jr      ra
    nop
END(CHECK_DDR_FREQ_CHANGE)
#endif

#ifdef  AUTO_ARB_LEVEL
LEAF(SET_AUTO_ARB_LEVEL_MARK)
    move    t6, ra

//set t7 to mc level info address
//if define CHECK_ARB_LEVEL_DIMM, set t1 at the same time for CHECK_DIMM_CHANGE
    dla     t7, c0_mc0_level_info
#ifdef  CHECK_ARB_LEVEL_DIMM
    move    t1, s3
#endif
    beqz    t3, 1f
    nop
    dla     t7, c0_mc1_level_info
#ifdef  CHECK_ARB_LEVEL_DIMM
    dsrl    t1, s3, 8
#endif
1:
#ifdef  MULTI_CHIP
    GET_NODE_ID_a1
    beqz    a1, 4f
    nop
    dla     t7, c1_mc0_level_info
#ifdef  CHECK_ARB_LEVEL_DIMM
    dsrl    t1, s3, 16
#endif
    beqz    t3, 4f
    nop
    dla     t7, c1_mc1_level_info
#ifdef  CHECK_ARB_LEVEL_DIMM
    dsrl    t1, s3, 24
#endif
4:
#endif
    daddu   t7, t7, s0
#ifdef  DEBUG_AUTO_ARB_LEVEL
    PRINTSTR("\r\nstored level info addr is 0x")
    dsrl    a0, t7, 32
    bal     hexserial
    nop
    move    a0, t7
    bal     hexserial
    nop
    PRINTSTR("\r\nsaved dimm infor is 0x")
    ld      t8, 0x0(t7)
    dsrl    a0, t8, 32
    bal     hexserial
    nop
    move    a0, t8
    bal     hexserial
    nop
    PRINTSTR("\r\n")
    ld      t8, 0x8(t7)
    dsrl    a0, t8, 32
    bal     hexserial
    nop
    move    a0, t8
    bal     hexserial
    nop
    PRINTSTR("\r\n")
    ld      t8, 0x10(t7)
    dsrl    a0, t8, 32
    bal     hexserial
    nop
    move    a0, t8
    bal     hexserial
    nop
    PRINTSTR("\r\n")
    ld      t8, 0x18(t7)
    dsrl    a0, t8, 32
    bal     hexserial
    nop
    move    a0, t8
    bal     hexserial
    nop
    PRINTSTR("\r\n")
    ld      t8, 0x20(t7)
    dsrl    a0, t8, 32
    bal     hexserial
    nop
    move    a0, t8
    bal     hexserial
    nop
    PRINTSTR("\r\n")
#endif
    //check level mark(first boot or first populate DIMM)
    ld      v0, 0x0(t7)
    and     v0, v0, 0x1
    beqz    v0, 1f 
    nop
    PRINTSTR("\r\nThis MC has been leveled.");

#ifdef  CHECK_ARB_LEVEL_FREQ
    bal     CHECK_DDR_FREQ_CHANGE
    nop
    bnez    v0, 1f
    nop
    PRINTSTR("\r\nThis MC frequency has not been changed.");
#endif

#ifdef  CHECK_ARB_LEVEL_DIMM
    bal     CHECK_DIMM_CHANGE
    nop
    bnez    v0, 1f
    nop
    PRINTSTR("\r\nThis MC DIMMs have not been changed.");
#endif
    //set no arb level mark
    move    t7, $0
    b       2f
    nop
1:
    //set do arb level mark
    or      t7, $0, 0x1
    or      v0, $0, 0x1
    dsll    a2, v0, 32
    GET_NODE_ID_a1
    dsll    a1, a1, 1
    dsll    a2, a2, a1
    dsll    a2, a2, t3
    or      s3, s3, a2
2:
#ifdef  DEBUG_AUTO_ARB_LEVEL
    PRINTSTR("\r\ns3 = 0x");
    dsrl    a0, s3, 32
    bal     hexserial
    nop
    PRINTSTR("__")
    move    a0, s3
    bal     hexserial
    nop
    PRINTSTR("\r\n")
#endif
    jr      t6
    nop
END(SET_AUTO_ARB_LEVEL_MARK)
#endif
