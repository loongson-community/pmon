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
**********************************/
//#define MULTI_NODE_DDR_PARAM
#define CONFIG_BASE 0x900000000ff00000

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
    beqz    a2, 1f
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
    //step 2: open extra RD/WR ODT CS if there is 2 DIMM
    //check CS[0] and CS[2]
    dsrl    a2, v0, 2
    xor     a2, v0, a2
    and     a2, a2, 0x1
    bnez    a2, 1f
    nop
    //2 DIMM: open the first rank of the non-target DIMM
    ld      a2, ODT_MAP_CS_ADDR(t8)
    dli     a1, 0x0101040401010404
    or      a2, a2, a1
    sd      a2, ODT_MAP_CS_ADDR(t8)
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
1:  //1 DIMM
#if 0
    //check 1 rank or 2 ranks populated by xor CS[0] and CS[2]
    GET_MC_CS_MAP
    dsrl    a2, a1, 2
    xor     a2, a1, a2
    and     v0, a2, 0x1
    beqz    v0, 1f
    nop
    //1 DIMM, check its SR DIMM or DR DIMM
    //here a2[1:0] store the valid CS
    and     v0, a2, 0x3
    dsrl    v0, v0, 1
    beqz    v0, 2f
    nop
    //Dual Rank DIMM
    dli     a2, 0x0408010200000000
    sd      a2, ODT_MAP_CS_ADDR(t8)
2:  //1 SR DIMM, use the default setting
    b       8f
    nop
1:  //2 DIMM
    

8:
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
    dli     a1, 0xff
    dsll    a1, a1, CTRL_RAW_OFFSET
    not     a1, a1
    and     a2, a2, a1
    dli     a1, 0x02
    dsll    a1, a1, CTRL_RAW_OFFSET
    or      a2, a2, a1
    sd      a2, CTRL_RAW_ADDR(t8)

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
    dli        t6, 0x00
    bal        inputaddress
    nop
    move    t5, v0
    
    dli     a1, 0xb3
    bgt     t5, a1, 2f    #if input address offset exceed range,jump out
    nop
    dsll    t5, t5, 4    #t5 is the offset relative to a0
    daddu    t5, t5, t8

    PRINTSTR("\r\nPlease input the data-hex: ");
    dli     t6, 0x00
    bal        inputaddress
    nop
    sd        v0, 0x0(t5)    #v0 is the input value

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

    b        1b
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
                    0x0000000000000000, \
                    0xfffffffff0000000, \
                    0x00000000000000f0)
    beqz    t3, 1f
    nop
    GET_NODE_ID_a0;
    XBAR_CONFIG_NODE_a0(0x10, \
                    0x0000000000000000, \
                    0xfffffffff0000000, \
                    0x00000000000000f1)
1:

    sync
    nop
    nop
    nop
    nop

    TTYDBG("\r\nEnable register space of MEMORY\r\n")
    lw      a1, 0x0(t2)
    li      a0, 0xfffffeff
    and     a1, a1,a0
    sw      a1, 0x0(t2)

    GET_NODE_ID_a0;
    dli     a1, 0x900000000ff00000
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
2:
    dla     t1, N2_fix_ddr_reg_data_mc0
    b       4f
    nop
3:
    dla     t1, N3_fix_ddr_reg_data_mc0
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
    li      a0, 0xfffffeff
    and     a1, a1,a0
    sw      a1, 0x0(t2)
#endif
#endif

#if 1   //def  DEBUG_DDR_PARAM   //print registers
    PRINTSTR("The MC param is:\r\n")
    dli     t1, DDR_PARAM_NUM
    GET_NODE_ID_a0
    dli     t5, 0x900000000ff00000
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

    TTYDBG("Disable register space of MEMORY\r\n")
    lw      a1,0x0(t2)
    or      a1,a1,0x100
    sw      a1,0x0(t2)

//cxk
#ifdef  ARB_LEVEL
    //route 0x1000000000 ~ 0x1FFFFFFFFF(64G) to MC for ARB_level
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
    beqz    t3, 1f
    nop
    GET_NODE_ID_a0;
    XBAR_CONFIG_NODE_a0(0x20, \
                    0x0000001000000000, \
                    0xFFFFFFF000000000, \
                    0x00000000000000F1)
1:
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
    lw      a1, 0x4(t2)
    li      a0, 0x8
    sll     a0, a0, t3
    not     a0, a0
    and     a1, a1, a0
    sw      a1, 0x4(t2)

    //TTYDBG("Enable register space of MEMORY\r\n")
    lw      a1, 0x0(t2)
    li      a0, 0xfffffeff
    and     a1, a1,a0
    sw      a1, 0x0(t2)

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

    //TTYDBG("Disable register space of MEMORY\r\n")
    lw      a1,0x0(t2)
    or      a1,a1,0x100
    sw      a1,0x0(t2)

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
    beqz    t3, 1f
    nop
    GET_NODE_ID_a0;
    XBAR_CONFIG_NODE_a0(0x20, \
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
    GET_MC0_MEMSIZE
    beqz    t3, 1f
    nop
    GET_MC1_MEMSIZE
1:
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

    //TTYDBG("Enable register space of MEMORY\r\n")
    lw      a1, 0x0(t2)
    li      a0, 0xfffffeff
    and     a1, a1,a0
    sw      a1, 0x0(t2)

    GET_NODE_ID_a0;
    dli     a1, 0x900000000ff00000
    or      t8, a0, a1
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
    or      a1,a1,0x100
    sw      a1,0x0(t2)

    TTYDBG("MC ECC init done.\r\n")
4:
#endif
    move    ra, t4
    jr      ra
    nop
    .end    mc_init
