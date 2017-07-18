/**********************************
    Author: chenxinke
    Date:   20110125
    v1.0
        support lsmcd3(~3A5, ~3B4, 2G, 2GQ, 2H)
        MC0/1 both use the same subroutine, add an input param: t3.
    input param:
    a2: mc param address
    t7(option ARB_LEVEL)--do arb level, 0--not level; 1--do level;
    t8: Memory Controller config register base
    t3: controller select
        0: MC0
        1: MC1
**********************************/

        .global ddr2_config
        .ent    ddr2_config
        .set    noreorder
        .set    mips3
ddr2_config:
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
#ifdef  SDRAM_HIGH_TEMPERATURE
    //rewrite tref, half the tref
    ld      a2, TREF_ADDR(t8)
    dli     a1, TREF_MASK
    dsrl    v0, a2, TREF_OFFSET
    and     v0, v0, a1
    dsll    a1, a1, TREF_OFFSET
    not     a1, a1
    and     a2, a2, a1
    dsll    a1, v0, TREF_OFFSET
    or      a2, a2, a1
    sd      a2, TREF_ADDR(t8)
#endif
#if 1
    //rewrite some Timing paramters according to SDRAM density and page size
    //here assuming the upper runing frequency is 400MHz, if not, please don't use this code and set the default paramters correspondingly

    //rewrite tFAW according to page size
    //assuming default for 2KB
    GET_SDRAM_WIDTH
    bnez    a1, 8f
    nop
    ld      a2, TFAW_ADDR(t8)
    dli     a1, TFAW_MASK
    dsll    a1, a1, TFAW_OFFSET
    not     a1, a1
    and     a2, a2, a1
    dli     v1, 0xc + 2 //DDR3
    GET_SDRAM_TYPE
    dli     v0, 0x3
    beq     v0, a1, 2f
    nop
    dli     v1, 0xf + 2 //DDR2
2:
    move    a1, v1
    dsll    a1, a1, TFAW_OFFSET
    or      a2, a2, a1
    sd      a2, TFAW_ADDR(t8)
8:

    //rewrite tRFC and txsnr according to SDRAM density
    //figure out SDRAM density first: 1: 512Mb, 2: 1Gb, 4: 2Gb, 8: 4Gb(x16 only)
    //count Rank number
    GET_MC_CS_MAP
    and     v1, a1, 0x1
    dsrl    a1, a1, 1
    and     v0, a1, 0x1
    daddu   v1, v1, v0
    dsrl    a1, a1, 1
    and     v0, a1, 0x1
    daddu   v1, v1, v0
    dsrl    a1, a1, 1
    and     v0, a1, 0x1
    daddu   v1, v1, v0

    GET_MC0_MEMSIZE
    beqz    t3, 1f
    nop
    GET_MC1_MEMSIZE
1:
    //get one rank capacity
    divu    v1, a1, v1
    //trick used here is for x8 SDRAM, SDRAM density is equal to Module density in net value, for x16, just double it
    GET_SDRAM_WIDTH
    dsll    v1, v1, a1

    GET_SDRAM_TYPE
    dli     v0, 0x3
    beq     v0, a1, 2f
    nop
    //DDR2
    dli     a1, 0x1
    beq     a1, v1, D2_density_512Mb
    nop
    dsll    a1, a1, 1
    beq     a1, v1, D2_density_1Gb
    nop
    dsll    a1, a1, 1
    beq     a1, v1, D2_density_2Gb
    nop
    dsll    a1, a1, 1
    beq     a1, v1, D2_density_4Gb
    nop
    //unexpect condition, make no change to default parameter
    //256Mb or larger than 4Gb
    b       8f
    nop
D2_density_512Mb:
    dli     v1, 0x2a + TRFC_MARGIN
    b       4f
    nop
D2_density_1Gb:
    dli     v1, 0x33 + TRFC_MARGIN
    b       4f
    nop
D2_density_2Gb:
    dli     v1, 0x4e + TRFC_MARGIN
    b       4f
    nop
D2_density_4Gb:
    dli     v1, 0x83 + TRFC_MARGIN
    b       4f
    nop
2:
    //DDR3
    dli     a1, 0x1
    beq     a1, v1, D3_density_512Mb
    nop
    dsll    a1, a1, 1
    beq     a1, v1, D3_density_1Gb
    nop
    dsll    a1, a1, 1
    beq     a1, v1, D3_density_2Gb
    nop
    dsll    a1, a1, 1
    beq     a1, v1, D3_density_4Gb
    nop
    //unexpect condition, make no change to default parameter
    //larger than 4Gb
    b       8f
    nop
D3_density_512Mb:
    dli     v1, 0x24 + TRFC_MARGIN
    b       4f
    nop
D3_density_1Gb:
    dli     v1, 0x2c + TRFC_MARGIN
    b       4f
    nop
D3_density_2Gb:
    dli     v1, 0x40 + TRFC_MARGIN
    b       4f
    nop
D3_density_4Gb:
    dli     v1, 0x78 + TRFC_MARGIN
    b       4f
    nop
4:
    ld      a2, TRFC_ADDR(t8)
    dli     a1, 0xff
    dsll    a1, a1, TRFC_OFFSET
    not     a1, a1
    and     a2, a2, a1
    move    a1, v1
    dsll    a1, a1, TRFC_OFFSET
    or      a2, a2, a1
    sd      a2, TRFC_ADDR(t8)

    ld      a2, TXSNR_ADDR(t8)
    dli     a1, 0xffff
    dsll    a1, a1, TXSNR_OFFSET
    not     a1, a1
    and     a2, a2, a1
    daddu   a1, v1, 0x4
    dsll    a1, a1, TXSNR_OFFSET
    or      a2, a2, a1
    sd      a2, TXSNR_ADDR(t8)
8:
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

#if 1
    //reconfig ODT map
    //set default first
#ifdef  DDR3_DIMM
    dli     a2, 0x0804020100000000
#else
    dli     a2, 0x0804020108040201
#endif
    sd      a2, ODT_MAP_CS_ADDR(t8)

    //v0 store cs map
    GET_MC_CS_MAP
    move    v0, a1
#if 1
    //Don't swap now, because we enable Dynamic ODT now, and
    //use different ODT value for WR and non-wr purpose
    //the above is obsoleted
    //DO NOT use dynamic ODT now, because it seems no good.
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

    PRINTSTR("\r\nChange some parameters of MC:");
1:
    PRINTSTR("\r\nPlease input the register number you want to change!!!(0xff:jump out.): ");
    dli     t6, 0x00
    bal     inputaddress
    nop
    move    t5, v0
    
    dli     a1, DDR_PARAM_NUM
    bge     t5, a1, 2f    #if input address offset exceed range,jump out
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
    ld      a1, START_ADDR(t8)
    dli     a2, 0x1
    dsll    a2, a2, START_OFFSET
    or      a1, a1, a2
    sd      a1, START_ADDR(t8)
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

    ###############################
ddr2_config_end:
    jr      ra
    nop
    .end    ddr2_config

