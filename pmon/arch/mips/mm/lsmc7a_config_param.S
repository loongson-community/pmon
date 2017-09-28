/**********************************
    Author: chenxinke
    Date:   20150708
    v1.0
        support lsmc(3A8, 3B5~)
    input param:
    a2: mc param address
    t8: input, Memory Controller config register base
**********************************/

        .global ls7a_ddr2_config
        .ent    ls7a_ddr2_config
        .set    noreorder
        .set    mips3
ls7a_ddr2_config:
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
    //rewrite pm_bank_diff_0 and pm_bank
    ld      a2, EIGHT_BANK_MODE_ADDR(t8)
    dli     a1, 0x3
    dsll    a1, a1, EIGHT_BANK_MODE_OFFSET
    not     a1, a1
    and     a2, a2, a1
    GET_EIGHT_BANK
    xor     a1, a1, 0x1
    dsll    a1, a1, EIGHT_BANK_MODE_OFFSET
    or      a2, a2, a1
    sd      a2, EIGHT_BANK_MODE_ADDR(t8)

    //rewrite pm_addr_win
    ld      a2, ADDR_WIN_BANK_NUM_ADDR(t8)
    dli     a1, 0x3
    dsll    a1, a1, ADDR_WIN_BANK_NUM_OFFSET
    not     a1, a1
    and     a2, a2, a1
    dli     v0, 0x2
    GET_EIGHT_BANK
    or      a1, v0, a1
    dsll    a1, a1, ADDR_WIN_BANK_NUM_OFFSET
    or      a2, a2, a1
    sd      a2, ADDR_WIN_BANK_NUM_ADDR(t8)

    //rewrite row_diff and column_diff
    ld      a2, ROW_DIFF_ADDR(t8)
    dli     a1, 0x7
    dsll    a1, a1, ROW_DIFF_OFFSET
    not     a1, a1
    and     a2, a2, a1
    GET_ROW_SIZE
    dsll    a1, a1, ROW_DIFF_OFFSET
    or      a2, a2, a1
    sd      a2, ROW_DIFF_ADDR(t8)

    ld      a2, COLUMN_DIFF_ADDR(t8)
    dli     a1, 0x7
    dsll    a1, a1, COLUMN_DIFF_OFFSET
    not     a1, a1
    and     a2, a2, a1
    GET_COL_SIZE
    daddu   a1, a1, 0x4
    dsll    a1, a1, COLUMN_DIFF_OFFSET
    or      a2, a2, a1
    sd      a2, COLUMN_DIFF_ADDR(t8)

    //reconfig address_mirroring
    ld      a2, ADDRESS_MIRROR_ADDR(t8)
    dli     a1, 0xf
    dsll    a1, a1, ADDRESS_MIRROR_OFFSET
    not     a1, a1
    and     a2, a2, a1
    dli     a1, 0x0
    dsll    a1, a1, ADDRESS_MIRROR_OFFSET
    or      a2, a2, a1
    sd      a2, ADDRESS_MIRROR_ADDR(t8)
#ifndef MANAUL_ODT_MAP
    //reconfig ODT map
    //set default first
    //clear map first
    dli     a1, 0x0000ffff0000ffff
    ld      a2, ODT_MAP_CS_ADDR(t8)
    and     a2, a2, a1
    sd      a2, ODT_MAP_CS_ADDR(t8)

    dli     a2, 0x8421000000000000  //DDR3
    GET_SDRAM_TYPE
    dli     a0, 0x3
    beq     a0, a1, 2f
    nop

    dli     a2, 0x8421000084210000  //DDR2
2:
    ld      a1, ODT_MAP_CS_ADDR(t8)
    or      a2, a2, a1
    sd      a2, ODT_MAP_CS_ADDR(t8)
#endif

#ifdef DEBUG_GMEM_PARAM   //debug
    /* store the ra */
    move    t1, ra

#if 1
    PRINTSTR("Gmem parameter is:\r\n")
    move    t6, t8
1:
    ld      t5, 0x0(t6)

    and     a0, t6, 0xfff
    bal     hexserial
    nop
    PRINTSTR(":  ")
    dsrl    a0, t5, 32
    bal     hexserial
    nop
    move    a0, t5
    bal     hexserial
    nop
    PRINTSTR("\r\n")

    daddiu  t6, t6, 8
    dli     a0, (DDR_PARAM_NUM * 8)
    daddu   a0, t8, a0
    blt     t6, a0, 1b
    nop
#endif

//input once, change all byte lanes parameters.

41:
    PRINTSTR("\r\nChange parameters:\r\n0--skip;1--dll_clock;2--dll_rddqs_gate;3--dll_rddqs_p;4--dll_rddqs_n;5--dll_wrdqs;6--dll_wrdq;\r\n7--rd_oe_end/start-edge_stop/start;8--wr_dqs_oe_end/start-edge_stop/start;9--wr_dq_oe_end/start-edge_stop/start;a--wr_odt_oe_end/start-edge_stop/start;\r\nb--wrdq_clk_delay;c--rddata_delay;d--rddqs_lt_half;e--wrdqs_lt_half;f--wrdq_lt_half;\r\n");
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

    PRINTSTR("\r\nChange some parameters of MC:");
1:
    PRINTSTR("\r\nPlease input the register number you want to change!!!(0xfff:jump out.): ");
    dli     t6, 0x00
    bal     inputaddress
    nop
    move    t5, v0
    
    dli     a1, DDR_PARAM_NUM
    dsll    a1, a1, 3
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

    daddiu  v0, t8, DRAM_INIT_ADDR
    GET_MC_CS_MAP
1:
    ld      a2, 0x0(v0)
    dsrl    a2, a2, DRAM_INIT_OFFSET
    and     a2, a2, a1
    bne     a1, a2, 1b
    nop

    jr      ra
    nop
    .end    ls7a_ddr2_config
