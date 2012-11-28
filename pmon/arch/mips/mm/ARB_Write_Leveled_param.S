/*******************
input param:
    t6: 1 ~ 6, param to be changed
    t2: param value of 8 byte lane
    for clk(t6==1): clk 2,1,0, store at byte 5, 3, 0 of t2
    use register: t7
*******************/

#if 0
#define  ALIGN_WLDQS_DLY \
    dli     a0, 0x8;; \
    bge     a2, a0, 58f; \
    nop; \
    and     a2, a2, 0xf8; \
    or      a2, a2, 0x8; \
    b       55f; \
    nop; \
58: \
    dli     a0, 0x30; \
    ble     a2, a0, 55f; \
    nop; \
    dli     a0, 0x40; \
    bge     a2, a0, 58f; \
    nop; \
    and     a2, a2, 0xf8; \
    b       55f; \
    nop; \
58: \
    dli     a2, 0x44; \
55:
#else
#define  ALIGN_WLDQS_DLY    
#endif
/***************
    dli     a0, 0x48; \
    bge     a2, a0, 55f; \
    nop; \
    and     a2, a2, 0xf8; \
    or      a2, a2, 0x8; \
***************/
#ifdef  CONTROL_L2XBAR_DDR_WINDOW
    //PRINTSTR("\r\nDisable DDR access window.")
    sync
    sync
    sync
    sync
    sync
    sync
    sync
    sync
    //Disable L2XBAR_WINDOW
    dli     t7, L2XBAR_CONFIG_BASE_ADDR
#ifdef LS3B
    GET_ARB_LEVEL_NODE_ID
    dsll    a1, a1, 14
    daddu   t7, t7, a1
#endif
    GET_ARB_LEVEL_NODE_ID
    dsll    a1, a1, 44
    or      t7, t7, a1
    daddu   t7, t7, ARB_TEMP_L2WINDOW_OFFSET
    ld      a0, 0x80(t7)
    and     a0, a0, 0xf
    sd      a0, 0x80(t7)
#endif

    //PRINTSTR("\r\nEnable DDR MC config space.");
    bal     enable_ddr_confspace
    nop
    
    dli     t7, DDR_MC_CONFIG_BASE
    GET_ARB_LEVEL_NODE_ID
    dsll    a1, a1, 44
    or      t7, t7, a1
    
#ifdef  ARBLVL_PUT_DRAM_SREF
    //put memory into self-refresh
    ld      a1, SREFRESH_ADDR(t7)
    dli     a2, 0x1
    dsll    a2, a2, SREFRESH_OFFSET
    or      a1, a1, a2
    sd      a1, SREFRESH_ADDR(t7)
    sync

    //delay some time
    dli     a2, 0x400
1:
    daddiu  a2, a2, -1
    bnez    a2, 1b
    nop
#endif
    
    //clear param_start
    //PRINTSTR("\r\nClear param_start.")
    dli     a2, 0xff
    dsll    a2, a2, START_OFFSET
    not     a2, a2
    ld      a1, START_ADDR(t7)
    and     a1, a1, a2
    sd      a1, START_ADDR(t7)

    //reset Gather FIFO
    ld      a1, PHY_CTRL_2_ADDR(t7)
    dli     a2, 0x1
    dsll    a2, a2, PHY_CTRL_2_OFFSET + RESET_GFIFO_SHIFT
    or      a1, a1, a2
    sd      a1, PHY_CTRL_2_ADDR(t7)

#if 1
    //PRINTSTR("\r\nChange param value.");

//!!!!!note: don't change the switch order of the code bellow, because we use
//add instr to change a1 instead of dli instr to reduce code size.
    //set modify target to a2
    andi    a2, t6, 0xf

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
    //PRINTSTR("\r\n--------Wrong selection: no parameter will be changed.");
    b       40f
    nop
1:
    ld      a1, CLKLVL_DELAY_2_ADDR(t7)
    dli     a2, CLKLVL_DELAY_MASK
    dsll    a2, a2, CLKLVL_DELAY_2_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsrl    a2, t2, 0x28
    and     a2, a2, 0x7f
    dsll    a2, a2, CLKLVL_DELAY_2_OFFSET
    or      a1, a1, a2
    sd      a1, CLKLVL_DELAY_2_ADDR(t7)

    ld      a1, CLKLVL_DELAY_1_ADDR(t7)
    dli     a2, CLKLVL_DELAY_MASK
    dsll    a2, a2, CLKLVL_DELAY_1_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsrl    a2, t2, 0x18
    and     a2, a2, 0x7f
    dsll    a2, a2, CLKLVL_DELAY_1_OFFSET
    or      a1, a1, a2
    sd      a1, CLKLVL_DELAY_1_ADDR(t7)

    ld      a1, CLKLVL_DELAY_0_ADDR(t7)
    dli     a2, CLKLVL_DELAY_MASK
    dsll    a2, a2, CLKLVL_DELAY_0_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsrl    a2, t2, 0x0
    and     a2, a2, 0x7f
    dsll    a2, a2, CLKLVL_DELAY_0_OFFSET
    or      a1, a1, a2
    sd      a1, CLKLVL_DELAY_0_ADDR(t7)
    b       40f
    nop
2:
    ld      a1, RDLVL_GATE_DELAY_8_ADDR(t7)
    dli     a2, RDLVL_GATE_DELAY_MASK
    dsll    a2, a2, RDLVL_GATE_DELAY_8_OFFSET
    not     a2, a2
    and     a1, a1, a2
#ifdef  DDR3_DIMM
    dsrl    a2, t2, 0x20
#else
    dsrl    a2, t2, 0x18
#endif
    and     a2, a2, 0x7f
    dsll    a2, a2, RDLVL_GATE_DELAY_8_OFFSET
    or      a1, a1, a2
    sd      a1, RDLVL_GATE_DELAY_8_ADDR(t7)

    ld      a1, RDLVL_GATE_DELAY_7_ADDR(t7)
    dli     a2, RDLVL_GATE_DELAY_MASK
    dsll    a2, a2, RDLVL_GATE_DELAY_7_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsrl    a2, t2, 0x38
    and     a2, a2, 0x7f
    dsll    a2, a2, RDLVL_GATE_DELAY_7_OFFSET
    or      a1, a1, a2
    sd      a1, RDLVL_GATE_DELAY_7_ADDR(t7)

    ld      a1, RDLVL_GATE_DELAY_6_ADDR(t7)
    dli     a2, RDLVL_GATE_DELAY_MASK
    dsll    a2, a2, RDLVL_GATE_DELAY_6_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsrl    a2, t2, 0x30
    and     a2, a2, 0x7f
    dsll    a2, a2, RDLVL_GATE_DELAY_6_OFFSET
    or      a1, a1, a2
    sd      a1, RDLVL_GATE_DELAY_6_ADDR(t7)

    ld      a1, RDLVL_GATE_DELAY_5_ADDR(t7)
    dli     a2, RDLVL_GATE_DELAY_MASK
    dsll    a2, a2, RDLVL_GATE_DELAY_5_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsrl    a2, t2, 0x28
    and     a2, a2, 0x7f
    dsll    a2, a2, RDLVL_GATE_DELAY_5_OFFSET
    or      a1, a1, a2
    sd      a1, RDLVL_GATE_DELAY_5_ADDR(t7)

    ld      a1, RDLVL_GATE_DELAY_4_ADDR(t7)
    dli     a2, RDLVL_GATE_DELAY_MASK
    dsll    a2, a2, RDLVL_GATE_DELAY_4_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsrl    a2, t2, 0x20
    and     a2, a2, 0x7f
    dsll    a2, a2, RDLVL_GATE_DELAY_4_OFFSET
    or      a1, a1, a2
    sd      a1, RDLVL_GATE_DELAY_4_ADDR(t7)

    ld      a1, RDLVL_GATE_DELAY_3_ADDR(t7)
    dli     a2, RDLVL_GATE_DELAY_MASK
    dsll    a2, a2, RDLVL_GATE_DELAY_3_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsrl    a2, t2, 0x18
    and     a2, a2, 0x7f
    dsll    a2, a2, RDLVL_GATE_DELAY_3_OFFSET
    or      a1, a1, a2
    sd      a1, RDLVL_GATE_DELAY_3_ADDR(t7)

    ld      a1, RDLVL_GATE_DELAY_2_ADDR(t7)
    dli     a2, RDLVL_GATE_DELAY_MASK
    dsll    a2, a2, RDLVL_GATE_DELAY_2_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsrl    a2, t2, 0x10
    and     a2, a2, 0x7f
    dsll    a2, a2, RDLVL_GATE_DELAY_2_OFFSET
    or      a1, a1, a2
    sd      a1, RDLVL_GATE_DELAY_2_ADDR(t7)

    ld      a1, RDLVL_GATE_DELAY_1_ADDR(t7)
    dli     a2, RDLVL_GATE_DELAY_MASK
    dsll    a2, a2, RDLVL_GATE_DELAY_1_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsrl    a2, t2, 0x8
    and     a2, a2, 0x7f
    dsll    a2, a2, RDLVL_GATE_DELAY_1_OFFSET
    or      a1, a1, a2
    sd      a1, RDLVL_GATE_DELAY_1_ADDR(t7)

    ld      a1, RDLVL_GATE_DELAY_0_ADDR(t7)
    dli     a2, RDLVL_GATE_DELAY_MASK
    dsll    a2, a2, RDLVL_GATE_DELAY_0_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsrl    a2, t2, 0x0
    and     a2, a2, 0x7f
    dsll    a2, a2, RDLVL_GATE_DELAY_0_OFFSET
    or      a1, a1, a2
    sd      a1, RDLVL_GATE_DELAY_0_ADDR(t7)
    b       40f
    nop
3:
    ld      a1, RDLVL_DELAY_8_ADDR(t7)
    dli     a2, RDLVL_DELAY_MASK
    dsll    a2, a2, RDLVL_DELAY_8_OFFSET
    not     a2, a2
    and     a1, a1, a2
#ifdef  DDR3_DIMM
    dsrl    a2, t2, 0x20
#else
    dsrl    a2, t2, 0x18
#endif
    and     a2, a2, 0x7f
    dsll    a2, a2, RDLVL_DELAY_8_OFFSET
    or      a1, a1, a2
    sd      a1, RDLVL_DELAY_8_ADDR(t7)

    ld      a1, RDLVL_DELAY_7_ADDR(t7)
    dli     a2, RDLVL_DELAY_MASK
    dsll    a2, a2, RDLVL_DELAY_7_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsrl    a2, t2, 0x38
    and     a2, a2, 0x7f
    dsll    a2, a2, RDLVL_DELAY_7_OFFSET
    or      a1, a1, a2
    sd      a1, RDLVL_DELAY_7_ADDR(t7)

    ld      a1, RDLVL_DELAY_6_ADDR(t7)
    dli     a2, RDLVL_DELAY_MASK
    dsll    a2, a2, RDLVL_DELAY_6_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsrl    a2, t2, 0x30
    and     a2, a2, 0x7f
    dsll    a2, a2, RDLVL_DELAY_6_OFFSET
    or      a1, a1, a2
    sd      a1, RDLVL_DELAY_6_ADDR(t7)

    ld      a1, RDLVL_DELAY_5_ADDR(t7)
    dli     a2, RDLVL_DELAY_MASK
    dsll    a2, a2, RDLVL_DELAY_5_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsrl    a2, t2, 0x28
    and     a2, a2, 0x7f
    dsll    a2, a2, RDLVL_DELAY_5_OFFSET
    or      a1, a1, a2
    sd      a1, RDLVL_DELAY_5_ADDR(t7)

    ld      a1, RDLVL_DELAY_4_ADDR(t7)
    dli     a2, RDLVL_DELAY_MASK
    dsll    a2, a2, RDLVL_DELAY_4_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsrl    a2, t2, 0x20
    and     a2, a2, 0x7f
    dsll    a2, a2, RDLVL_DELAY_4_OFFSET
    or      a1, a1, a2
    sd      a1, RDLVL_DELAY_4_ADDR(t7)

    ld      a1, RDLVL_DELAY_3_ADDR(t7)
    dli     a2, RDLVL_DELAY_MASK
    dsll    a2, a2, RDLVL_DELAY_3_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsrl    a2, t2, 0x18
    and     a2, a2, 0x7f
    dsll    a2, a2, RDLVL_DELAY_3_OFFSET
    or      a1, a1, a2
    sd      a1, RDLVL_DELAY_3_ADDR(t7)

    ld      a1, RDLVL_DELAY_2_ADDR(t7)
    dli     a2, RDLVL_DELAY_MASK
    dsll    a2, a2, RDLVL_DELAY_2_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsrl    a2, t2, 0x10
    and     a2, a2, 0x7f
    dsll    a2, a2, RDLVL_DELAY_2_OFFSET
    or      a1, a1, a2
    sd      a1, RDLVL_DELAY_2_ADDR(t7)

    ld      a1, RDLVL_DELAY_1_ADDR(t7)
    dli     a2, RDLVL_DELAY_MASK
    dsll    a2, a2, RDLVL_DELAY_1_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsrl    a2, t2, 0x8
    and     a2, a2, 0x7f
    dsll    a2, a2, RDLVL_DELAY_1_OFFSET
    or      a1, a1, a2
    sd      a1, RDLVL_DELAY_1_ADDR(t7)

    ld      a1, RDLVL_DELAY_0_ADDR(t7)
    dli     a2, RDLVL_DELAY_MASK
    dsll    a2, a2, RDLVL_DELAY_0_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsrl    a2, t2, 0x0
    and     a2, a2, 0x7f
    dsll    a2, a2, RDLVL_DELAY_0_OFFSET
    or      a1, a1, a2
    sd      a1, RDLVL_DELAY_0_ADDR(t7)
    b       40f
    nop
4:
    ld      a1, RDLVL_DQSN_DELAY_8_ADDR(t7)
    dli     a2, RDLVL_DQSN_DELAY_MASK
    dsll    a2, a2, RDLVL_DQSN_DELAY_8_OFFSET
    not     a2, a2
    and     a1, a1, a2
#ifdef  DDR3_DIMM
    dsrl    a2, t2, 0x20
#else
    dsrl    a2, t2, 0x18
#endif
    and     a2, a2, 0x7f
    dsll    a2, a2, RDLVL_DQSN_DELAY_8_OFFSET
    or      a1, a1, a2
    sd      a1, RDLVL_DQSN_DELAY_8_ADDR(t7)

    ld      a1, RDLVL_DQSN_DELAY_7_ADDR(t7)
    dli     a2, RDLVL_DQSN_DELAY_MASK
    dsll    a2, a2, RDLVL_DQSN_DELAY_7_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsrl    a2, t2, 0x38
    and     a2, a2, 0x7f
    dsll    a2, a2, RDLVL_DQSN_DELAY_7_OFFSET
    or      a1, a1, a2
    sd      a1, RDLVL_DQSN_DELAY_7_ADDR(t7)

    ld      a1, RDLVL_DQSN_DELAY_6_ADDR(t7)
    dli     a2, RDLVL_DQSN_DELAY_MASK
    dsll    a2, a2, RDLVL_DQSN_DELAY_6_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsrl    a2, t2, 0x30
    and     a2, a2, 0x7f
    dsll    a2, a2, RDLVL_DQSN_DELAY_6_OFFSET
    or      a1, a1, a2
    sd      a1, RDLVL_DQSN_DELAY_6_ADDR(t7)

    ld      a1, RDLVL_DQSN_DELAY_5_ADDR(t7)
    dli     a2, RDLVL_DQSN_DELAY_MASK
    dsll    a2, a2, RDLVL_DQSN_DELAY_5_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsrl    a2, t2, 0x28
    and     a2, a2, 0x7f
    dsll    a2, a2, RDLVL_DQSN_DELAY_5_OFFSET
    or      a1, a1, a2
    sd      a1, RDLVL_DQSN_DELAY_5_ADDR(t7)

    ld      a1, RDLVL_DQSN_DELAY_4_ADDR(t7)
    dli     a2, RDLVL_DQSN_DELAY_MASK
    dsll    a2, a2, RDLVL_DQSN_DELAY_4_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsrl    a2, t2, 0x20
    and     a2, a2, 0x7f
    dsll    a2, a2, RDLVL_DQSN_DELAY_4_OFFSET
    or      a1, a1, a2
    sd      a1, RDLVL_DQSN_DELAY_4_ADDR(t7)

    ld      a1, RDLVL_DQSN_DELAY_3_ADDR(t7)
    dli     a2, RDLVL_DQSN_DELAY_MASK
    dsll    a2, a2, RDLVL_DQSN_DELAY_3_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsrl    a2, t2, 0x18
    and     a2, a2, 0x7f
    dsll    a2, a2, RDLVL_DQSN_DELAY_3_OFFSET
    or      a1, a1, a2
    sd      a1, RDLVL_DQSN_DELAY_3_ADDR(t7)

    ld      a1, RDLVL_DQSN_DELAY_2_ADDR(t7)
    dli     a2, RDLVL_DQSN_DELAY_MASK
    dsll    a2, a2, RDLVL_DQSN_DELAY_2_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsrl    a2, t2, 0x10
    and     a2, a2, 0x7f
    dsll    a2, a2, RDLVL_DQSN_DELAY_2_OFFSET
    or      a1, a1, a2
    sd      a1, RDLVL_DQSN_DELAY_2_ADDR(t7)

    ld      a1, RDLVL_DQSN_DELAY_1_ADDR(t7)
    dli     a2, RDLVL_DQSN_DELAY_MASK
    dsll    a2, a2, RDLVL_DQSN_DELAY_1_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsrl    a2, t2, 0x8
    and     a2, a2, 0x7f
    dsll    a2, a2, RDLVL_DQSN_DELAY_1_OFFSET
    or      a1, a1, a2
    sd      a1, RDLVL_DQSN_DELAY_1_ADDR(t7)

    ld      a1, RDLVL_DQSN_DELAY_0_ADDR(t7)
    dli     a2, RDLVL_DQSN_DELAY_MASK
    dsll    a2, a2, RDLVL_DQSN_DELAY_0_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsrl    a2, t2, 0x0
    and     a2, a2, 0x7f
    dsll    a2, a2, RDLVL_DQSN_DELAY_0_OFFSET
    or      a1, a1, a2
    sd      a1, RDLVL_DQSN_DELAY_0_ADDR(t7)
    b       40f
    nop
5:
    ld      a1, WRLVL_DELAY_8_ADDR(t7)
    dli     a2, WRLVL_DELAY_MASK
    dsll    a2, a2, WRLVL_DELAY_8_OFFSET
    not     a2, a2
    and     a1, a1, a2
#ifdef  DDR3_DIMM
    dsrl    a2, t2, 0x20
#else
    dsrl    a2, t2, 0x18
#endif
    and     a2, a2, 0x7f
    ALIGN_WLDQS_DLY
    dsll    a2, a2, WRLVL_DELAY_8_OFFSET
    or      a1, a1, a2
    sd      a1, WRLVL_DELAY_8_ADDR(t7)

    ld      a1, WRLVL_DELAY_7_ADDR(t7)
    dli     a2, WRLVL_DELAY_MASK
    dsll    a2, a2, WRLVL_DELAY_7_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsrl    a2, t2, 0x38
    and     a2, a2, 0x7f
    ALIGN_WLDQS_DLY
    dsll    a2, a2, WRLVL_DELAY_7_OFFSET
    or      a1, a1, a2
    sd      a1, WRLVL_DELAY_7_ADDR(t7)

    ld      a1, WRLVL_DELAY_6_ADDR(t7)
    dli     a2, WRLVL_DELAY_MASK
    dsll    a2, a2, WRLVL_DELAY_6_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsrl    a2, t2, 0x30
    and     a2, a2, 0x7f
    ALIGN_WLDQS_DLY
    dsll    a2, a2, WRLVL_DELAY_6_OFFSET
    or      a1, a1, a2
    sd      a1, WRLVL_DELAY_6_ADDR(t7)

    ld      a1, WRLVL_DELAY_5_ADDR(t7)
    dli     a2, WRLVL_DELAY_MASK
    dsll    a2, a2, WRLVL_DELAY_5_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsrl    a2, t2, 0x28
    and     a2, a2, 0x7f
    ALIGN_WLDQS_DLY
    dsll    a2, a2, WRLVL_DELAY_5_OFFSET
    or      a1, a1, a2
    sd      a1, WRLVL_DELAY_5_ADDR(t7)

    ld      a1, WRLVL_DELAY_4_ADDR(t7)
    dli     a2, WRLVL_DELAY_MASK
    dsll    a2, a2, WRLVL_DELAY_4_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsrl    a2, t2, 0x20
    and     a2, a2, 0x7f
    ALIGN_WLDQS_DLY
    dsll    a2, a2, WRLVL_DELAY_4_OFFSET
    or      a1, a1, a2
    sd      a1, WRLVL_DELAY_4_ADDR(t7)

    ld      a1, WRLVL_DELAY_3_ADDR(t7)
    dli     a2, WRLVL_DELAY_MASK
    dsll    a2, a2, WRLVL_DELAY_3_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsrl    a2, t2, 0x18
    and     a2, a2, 0x7f
    ALIGN_WLDQS_DLY
    dsll    a2, a2, WRLVL_DELAY_3_OFFSET
    or      a1, a1, a2
    sd      a1, WRLVL_DELAY_3_ADDR(t7)

    ld      a1, WRLVL_DELAY_2_ADDR(t7)
    dli     a2, WRLVL_DELAY_MASK
    dsll    a2, a2, WRLVL_DELAY_2_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsrl    a2, t2, 0x10
    and     a2, a2, 0x7f
    ALIGN_WLDQS_DLY
    dsll    a2, a2, WRLVL_DELAY_2_OFFSET
    or      a1, a1, a2
    sd      a1, WRLVL_DELAY_2_ADDR(t7)

    ld      a1, WRLVL_DELAY_1_ADDR(t7)
    dli     a2, WRLVL_DELAY_MASK
    dsll    a2, a2, WRLVL_DELAY_1_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsrl    a2, t2, 0x8
    and     a2, a2, 0x7f
    ALIGN_WLDQS_DLY
    dsll    a2, a2, WRLVL_DELAY_1_OFFSET
    or      a1, a1, a2
    sd      a1, WRLVL_DELAY_1_ADDR(t7)

    ld      a1, WRLVL_DELAY_0_ADDR(t7)
    dli     a2, WRLVL_DELAY_MASK
    dsll    a2, a2, WRLVL_DELAY_0_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsrl    a2, t2, 0x0
    and     a2, a2, 0x7f
    ALIGN_WLDQS_DLY
    dsll    a2, a2, WRLVL_DELAY_0_OFFSET
    or      a1, a1, a2
    sd      a1, WRLVL_DELAY_0_ADDR(t7)

#ifdef  CLEAR_HALF_CLK_SHIFT
    //reset phy_ctrl_reg_0[17]--add_half_clk_shift
    //byte_8
    dli     a1, WRLVL_HALF_CLK_VALUE
#ifdef  DDR3_DIMM
    dsrl    a2, t2, 0x20
#else
    dsrl    a2, t2, 0x18
#endif
    and     a2, a2, 0x7f
    blt     a2, a1, 51f
    nop
    //set add_half_clk_shift
    ld      a1, PHY_CTRL_0_8_ADDR(t7)
    dli     a2, 0x3
    dsll    a2, a2, PHY_CTRL_0_8_OFFSET + PHY_CTRL_0_ADDWLDLY_SHIFT
    or      a1, a1, a2
    sd      a1, PHY_CTRL_0_8_ADDR(t7)
    b       52f
    nop
51: //clear add_half_clk_shift
    ld      a1, PHY_CTRL_0_8_ADDR(t7)
    dli     a2, 0x3
    dsll    a2, a2, PHY_CTRL_0_8_OFFSET + PHY_CTRL_0_ADDWLDLY_SHIFT
    not     a2, a2
    and     a1, a1, a2
    sd      a1, PHY_CTRL_0_8_ADDR(t7)
52:
    //byte_7
    dli     a1, WRLVL_HALF_CLK_VALUE
    dsrl    a2, t2, 0x38
    and     a2, a2, 0x7f
    blt     a2, a1, 51f
    nop
    //set add_half_clk_shift
    ld      a1, PHY_CTRL_0_7_ADDR(t7)
    dli     a2, 0x3
    dsll    a2, a2, PHY_CTRL_0_7_OFFSET + PHY_CTRL_0_ADDWLDLY_SHIFT
    or      a1, a1, a2
    sd      a1, PHY_CTRL_0_7_ADDR(t7)
    b       52f
    nop
51: //clear add_half_clk_shift
    ld      a1, PHY_CTRL_0_7_ADDR(t7)
    dli     a2, 0x3
    dsll    a2, a2, PHY_CTRL_0_7_OFFSET + PHY_CTRL_0_ADDWLDLY_SHIFT
    not     a2, a2
    and     a1, a1, a2
    sd      a1, PHY_CTRL_0_7_ADDR(t7)
52:
    //byte_6
    dli     a1, WRLVL_HALF_CLK_VALUE
    dsrl    a2, t2, 0x30
    and     a2, a2, 0x7f
    blt     a2, a1, 51f
    nop
    //set add_half_clk_shift
    ld      a1, PHY_CTRL_0_6_ADDR(t7)
    dli     a2, 0x3
    dsll    a2, a2, PHY_CTRL_0_6_OFFSET + PHY_CTRL_0_ADDWLDLY_SHIFT
    or      a1, a1, a2
    sd      a1, PHY_CTRL_0_6_ADDR(t7)
    b       52f
    nop
51: //clear add_half_clk_shift
    ld      a1, PHY_CTRL_0_6_ADDR(t7)
    dli     a2, 0x3
    dsll    a2, a2, PHY_CTRL_0_6_OFFSET + PHY_CTRL_0_ADDWLDLY_SHIFT
    not     a2, a2
    and     a1, a1, a2
    sd      a1, PHY_CTRL_0_6_ADDR(t7)
52:
    //byte_5
    dli     a1, WRLVL_HALF_CLK_VALUE
    dsrl    a2, t2, 0x28
    and     a2, a2, 0x7f
    blt     a2, a1, 51f
    nop
    //set add_half_clk_shift
    ld      a1, PHY_CTRL_0_5_ADDR(t7)
    dli     a2, 0x3
    dsll    a2, a2, PHY_CTRL_0_5_OFFSET + PHY_CTRL_0_ADDWLDLY_SHIFT
    or      a1, a1, a2
    sd      a1, PHY_CTRL_0_5_ADDR(t7)
    b       52f
    nop
51: //clear add_half_clk_shift
    ld      a1, PHY_CTRL_0_5_ADDR(t7)
    dli     a2, 0x3
    dsll    a2, a2, PHY_CTRL_0_5_OFFSET + PHY_CTRL_0_ADDWLDLY_SHIFT
    not     a2, a2
    and     a1, a1, a2
    sd      a1, PHY_CTRL_0_5_ADDR(t7)
52:
    //byte_4
    dli     a1, WRLVL_HALF_CLK_VALUE
    dsrl    a2, t2, 0x20
    and     a2, a2, 0x7f
    blt     a2, a1, 51f
    nop
    //set add_half_clk_shift
    ld      a1, PHY_CTRL_0_4_ADDR(t7)
    dli     a2, 0x3
    dsll    a2, a2, PHY_CTRL_0_4_OFFSET + PHY_CTRL_0_ADDWLDLY_SHIFT
    or      a1, a1, a2
    sd      a1, PHY_CTRL_0_4_ADDR(t7)
    b       52f
    nop
51: //clear add_half_clk_shift
    ld      a1, PHY_CTRL_0_4_ADDR(t7)
    dli     a2, 0x3
    dsll    a2, a2, PHY_CTRL_0_4_OFFSET + PHY_CTRL_0_ADDWLDLY_SHIFT
    not     a2, a2
    and     a1, a1, a2
    sd      a1, PHY_CTRL_0_4_ADDR(t7)
52:
    //byte_3
    dli     a1, WRLVL_HALF_CLK_VALUE
    dsrl    a2, t2, 0x18
    and     a2, a2, 0x7f
    blt     a2, a1, 51f
    nop
    //set add_half_clk_shift
    ld      a1, PHY_CTRL_0_3_ADDR(t7)
    dli     a2, 0x3
    dsll    a2, a2, PHY_CTRL_0_3_OFFSET + PHY_CTRL_0_ADDWLDLY_SHIFT
    or      a1, a1, a2
    sd      a1, PHY_CTRL_0_3_ADDR(t7)
    b       52f
    nop
51: //clear add_half_clk_shift
    ld      a1, PHY_CTRL_0_3_ADDR(t7)
    dli     a2, 0x3
    dsll    a2, a2, PHY_CTRL_0_3_OFFSET + PHY_CTRL_0_ADDWLDLY_SHIFT
    not     a2, a2
    and     a1, a1, a2
    sd      a1, PHY_CTRL_0_3_ADDR(t7)
52:
    //byte_2
    dli     a1, WRLVL_HALF_CLK_VALUE
    dsrl    a2, t2, 0x10
    and     a2, a2, 0x7f
    blt     a2, a1, 51f
    nop
    //set add_half_clk_shift
    ld      a1, PHY_CTRL_0_2_ADDR(t7)
    dli     a2, 0x3
    dsll    a2, a2, PHY_CTRL_0_2_OFFSET + PHY_CTRL_0_ADDWLDLY_SHIFT
    or      a1, a1, a2
    sd      a1, PHY_CTRL_0_2_ADDR(t7)
    b       52f
    nop
51: //clear add_half_clk_shift
    ld      a1, PHY_CTRL_0_2_ADDR(t7)
    dli     a2, 0x3
    dsll    a2, a2, PHY_CTRL_0_2_OFFSET + PHY_CTRL_0_ADDWLDLY_SHIFT
    not     a2, a2
    and     a1, a1, a2
    sd      a1, PHY_CTRL_0_2_ADDR(t7)
52:
    //byte_1
    dli     a1, WRLVL_HALF_CLK_VALUE
    dsrl    a2, t2, 0x8
    and     a2, a2, 0x7f
    blt     a2, a1, 51f
    nop
    //set add_half_clk_shift
    ld      a1, PHY_CTRL_0_1_ADDR(t7)
    dli     a2, 0x3
    dsll    a2, a2, PHY_CTRL_0_1_OFFSET + PHY_CTRL_0_ADDWLDLY_SHIFT
    or      a1, a1, a2
    sd      a1, PHY_CTRL_0_1_ADDR(t7)
    b       52f
    nop
51: //clear add_half_clk_shift
    ld      a1, PHY_CTRL_0_1_ADDR(t7)
    dli     a2, 0x3
    dsll    a2, a2, PHY_CTRL_0_1_OFFSET + PHY_CTRL_0_ADDWLDLY_SHIFT
    not     a2, a2
    and     a1, a1, a2
    sd      a1, PHY_CTRL_0_1_ADDR(t7)
52:
    //byte_0
    dli     a1, WRLVL_HALF_CLK_VALUE
    dsrl    a2, t2, 0x0
    and     a2, a2, 0x7f
    blt     a2, a1, 51f
    nop
    //set add_half_clk_shift
    ld      a1, PHY_CTRL_0_0_ADDR(t7)
    dli     a2, 0x3
    dsll    a2, a2, PHY_CTRL_0_0_OFFSET + PHY_CTRL_0_ADDWLDLY_SHIFT
    or      a1, a1, a2
    sd      a1, PHY_CTRL_0_0_ADDR(t7)
    b       52f
    nop
51: //clear add_half_clk_shift
    ld      a1, PHY_CTRL_0_0_ADDR(t7)
    dli     a2, 0x3
    dsll    a2, a2, PHY_CTRL_0_0_OFFSET + PHY_CTRL_0_ADDWLDLY_SHIFT
    not     a2, a2
    and     a1, a1, a2
    sd      a1, PHY_CTRL_0_0_ADDR(t7)
52:
#endif

    b       40f
    nop
6:
    and     a2, a2, 0x7f

    ld      a1, WRLVL_DQ_DELAY_8_ADDR(t7)
    dli     a2, WRLVL_DQ_DELAY_MASK
    dsll    a2, a2, WRLVL_DQ_DELAY_8_OFFSET
    not     a2, a2
    and     a1, a1, a2
#ifdef  DDR3_DIMM
    dsrl    a2, t2, 0x20
#else
    dsrl    a2, t2, 0x18
#endif
    and     a2, a2, 0x7f
    dsll    a2, a2, WRLVL_DQ_DELAY_8_OFFSET
    or      a1, a1, a2
    sd      a1, WRLVL_DQ_DELAY_8_ADDR(t7)

    ld      a1, WRLVL_DQ_DELAY_7_ADDR(t7)
    dli     a2, WRLVL_DQ_DELAY_MASK
    dsll    a2, a2, WRLVL_DQ_DELAY_7_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsrl    a2, t2, 0x38
    and     a2, a2, 0x7f
    dsll    a2, a2, WRLVL_DQ_DELAY_7_OFFSET
    or      a1, a1, a2
    sd      a1, WRLVL_DQ_DELAY_7_ADDR(t7)

    ld      a1, WRLVL_DQ_DELAY_6_ADDR(t7)
    dli     a2, WRLVL_DQ_DELAY_MASK
    dsll    a2, a2, WRLVL_DQ_DELAY_6_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsrl    a2, t2, 0x30
    and     a2, a2, 0x7f
    dsll    a2, a2, WRLVL_DQ_DELAY_6_OFFSET
    or      a1, a1, a2
    sd      a1, WRLVL_DQ_DELAY_6_ADDR(t7)

    ld      a1, WRLVL_DQ_DELAY_5_ADDR(t7)
    dli     a2, WRLVL_DQ_DELAY_MASK
    dsll    a2, a2, WRLVL_DQ_DELAY_5_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsrl    a2, t2, 0x28
    and     a2, a2, 0x7f
    dsll    a2, a2, WRLVL_DQ_DELAY_5_OFFSET
    or      a1, a1, a2
    sd      a1, WRLVL_DQ_DELAY_5_ADDR(t7)

    ld      a1, WRLVL_DQ_DELAY_4_ADDR(t7)
    dli     a2, WRLVL_DQ_DELAY_MASK
    dsll    a2, a2, WRLVL_DQ_DELAY_4_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsrl    a2, t2, 0x20
    and     a2, a2, 0x7f
    dsll    a2, a2, WRLVL_DQ_DELAY_4_OFFSET
    or      a1, a1, a2
    sd      a1, WRLVL_DQ_DELAY_4_ADDR(t7)

    ld      a1, WRLVL_DQ_DELAY_3_ADDR(t7)
    dli     a2, WRLVL_DQ_DELAY_MASK
    dsll    a2, a2, WRLVL_DQ_DELAY_3_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsrl    a2, t2, 0x18
    and     a2, a2, 0x7f
    dsll    a2, a2, WRLVL_DQ_DELAY_3_OFFSET
    or      a1, a1, a2
    sd      a1, WRLVL_DQ_DELAY_3_ADDR(t7)

    ld      a1, WRLVL_DQ_DELAY_2_ADDR(t7)
    dli     a2, WRLVL_DQ_DELAY_MASK
    dsll    a2, a2, WRLVL_DQ_DELAY_2_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsrl    a2, t2, 0x10
    and     a2, a2, 0x7f
    dsll    a2, a2, WRLVL_DQ_DELAY_2_OFFSET
    or      a1, a1, a2
    sd      a1, WRLVL_DQ_DELAY_2_ADDR(t7)

    ld      a1, WRLVL_DQ_DELAY_1_ADDR(t7)
    dli     a2, WRLVL_DQ_DELAY_MASK
    dsll    a2, a2, WRLVL_DQ_DELAY_1_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsrl    a2, t2, 0x8
    and     a2, a2, 0x7f
    dsll    a2, a2, WRLVL_DQ_DELAY_1_OFFSET
    or      a1, a1, a2
    sd      a1, WRLVL_DQ_DELAY_1_ADDR(t7)

    ld      a1, WRLVL_DQ_DELAY_0_ADDR(t7)
    dli     a2, WRLVL_DQ_DELAY_MASK
    dsll    a2, a2, WRLVL_DQ_DELAY_0_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsrl    a2, t2, 0x0
    and     a2, a2, 0x7f
    dsll    a2, a2, WRLVL_DQ_DELAY_0_OFFSET
    or      a1, a1, a2
    sd      a1, WRLVL_DQ_DELAY_0_ADDR(t7)
    b       40f
    nop
7:
    ld      a1, PHY_CTRL_0_8_ADDR(t7)
    dli     a2, PHY_CTRL_0_MASK
    dsll    a2, a2, PHY_CTRL_0_8_OFFSET
    not     a2, a2
    and     a1, a1, a2
#ifdef  DDR3_DIMM
    dsrl    a2, t2, 0x20
#else
    dsrl    a2, t2, 0x18
#endif
    and     a2, a2, 0x7f
    dsll    a2, a2, PHY_CTRL_0_8_OFFSET
    or      a1, a1, a2
    sd      a1, PHY_CTRL_0_8_ADDR(t7)

    ld      a1, PHY_CTRL_0_7_ADDR(t7)
    dli     a2, PHY_CTRL_0_MASK
    dsll    a2, a2, PHY_CTRL_0_7_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsrl    a2, t2, 0x38
    and     a2, a2, 0x7f
    dsll    a2, a2, PHY_CTRL_0_7_OFFSET
    or      a1, a1, a2
    sd      a1, PHY_CTRL_0_7_ADDR(t7)

    ld      a1, PHY_CTRL_0_6_ADDR(t7)
    dli     a2, PHY_CTRL_0_MASK
    dsll    a2, a2, PHY_CTRL_0_6_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsrl    a2, t2, 0x30
    and     a2, a2, 0x7f
    dsll    a2, a2, PHY_CTRL_0_6_OFFSET
    or      a1, a1, a2
    sd      a1, PHY_CTRL_0_6_ADDR(t7)

    ld      a1, PHY_CTRL_0_5_ADDR(t7)
    dli     a2, PHY_CTRL_0_MASK
    dsll    a2, a2, PHY_CTRL_0_5_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsrl    a2, t2, 0x28
    and     a2, a2, 0x7f
    dsll    a2, a2, PHY_CTRL_0_5_OFFSET
    or      a1, a1, a2
    sd      a1, PHY_CTRL_0_5_ADDR(t7)

    ld      a1, PHY_CTRL_0_4_ADDR(t7)
    dli     a2, PHY_CTRL_0_MASK
    dsll    a2, a2, PHY_CTRL_0_4_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsrl    a2, t2, 0x20
    and     a2, a2, 0x7f
    dsll    a2, a2, PHY_CTRL_0_4_OFFSET
    or      a1, a1, a2
    sd      a1, PHY_CTRL_0_4_ADDR(t7)

    ld      a1, PHY_CTRL_0_3_ADDR(t7)
    dli     a2, PHY_CTRL_0_MASK
    dsll    a2, a2, PHY_CTRL_0_3_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsrl    a2, t2, 0x18
    and     a2, a2, 0x7f
    dsll    a2, a2, PHY_CTRL_0_3_OFFSET
    or      a1, a1, a2
    sd      a1, PHY_CTRL_0_3_ADDR(t7)

    ld      a1, PHY_CTRL_0_2_ADDR(t7)
    dli     a2, PHY_CTRL_0_MASK
    dsll    a2, a2, PHY_CTRL_0_2_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsrl    a2, t2, 0x10
    and     a2, a2, 0x7f
    dsll    a2, a2, PHY_CTRL_0_2_OFFSET
    or      a1, a1, a2
    sd      a1, PHY_CTRL_0_2_ADDR(t7)

    ld      a1, PHY_CTRL_0_1_ADDR(t7)
    dli     a2, PHY_CTRL_0_MASK
    dsll    a2, a2, PHY_CTRL_0_1_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsrl    a2, t2, 0x8
    and     a2, a2, 0x7f
    dsll    a2, a2, PHY_CTRL_0_1_OFFSET
    or      a1, a1, a2
    sd      a1, PHY_CTRL_0_1_ADDR(t7)

    ld      a1, PHY_CTRL_0_0_ADDR(t7)
    dli     a2, PHY_CTRL_0_MASK
    dsll    a2, a2, PHY_CTRL_0_0_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsrl    a2, t2, 0x0
    and     a2, a2, 0x7f
    dsll    a2, a2, PHY_CTRL_0_0_OFFSET
    or      a1, a1, a2
    sd      a1, PHY_CTRL_0_0_ADDR(t7)
    b       40f
    nop
8:
    ld      a1, PHY_CTRL_1_8_ADDR(t7)
    dli     a2, PHY_CTRL_1_MASK
    dsll    a2, a2, PHY_CTRL_1_8_OFFSET
    not     a2, a2
    and     a1, a1, a2
#ifdef  DDR3_DIMM
    dsrl    a2, t2, 0x20
#else
    dsrl    a2, t2, 0x18
#endif
    and     a2, a2, 0x7f
    dsll    a2, a2, PHY_CTRL_1_8_OFFSET
    or      a1, a1, a2
    sd      a1, PHY_CTRL_1_8_ADDR(t7)

    ld      a1, PHY_CTRL_1_7_ADDR(t7)
    dli     a2, PHY_CTRL_1_MASK
    dsll    a2, a2, PHY_CTRL_1_7_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsrl    a2, t2, 0x38
    and     a2, a2, 0x7f
    dsll    a2, a2, PHY_CTRL_1_7_OFFSET
    or      a1, a1, a2
    sd      a1, PHY_CTRL_1_7_ADDR(t7)

    ld      a1, PHY_CTRL_1_6_ADDR(t7)
    dli     a2, PHY_CTRL_1_MASK
    dsll    a2, a2, PHY_CTRL_1_6_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsrl    a2, t2, 0x30
    and     a2, a2, 0x7f
    dsll    a2, a2, PHY_CTRL_1_6_OFFSET
    or      a1, a1, a2
    sd      a1, PHY_CTRL_1_6_ADDR(t7)

    ld      a1, PHY_CTRL_1_5_ADDR(t7)
    dli     a2, PHY_CTRL_1_MASK
    dsll    a2, a2, PHY_CTRL_1_5_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsrl    a2, t2, 0x28
    and     a2, a2, 0x7f
    dsll    a2, a2, PHY_CTRL_1_5_OFFSET
    or      a1, a1, a2
    sd      a1, PHY_CTRL_1_5_ADDR(t7)

    ld      a1, PHY_CTRL_1_4_ADDR(t7)
    dli     a2, PHY_CTRL_1_MASK
    dsll    a2, a2, PHY_CTRL_1_4_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsrl    a2, t2, 0x20
    and     a2, a2, 0x7f
    dsll    a2, a2, PHY_CTRL_1_4_OFFSET
    or      a1, a1, a2
    sd      a1, PHY_CTRL_1_4_ADDR(t7)

    ld      a1, PHY_CTRL_1_3_ADDR(t7)
    dli     a2, PHY_CTRL_1_MASK
    dsll    a2, a2, PHY_CTRL_1_3_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsrl    a2, t2, 0x18
    and     a2, a2, 0x7f
    dsll    a2, a2, PHY_CTRL_1_3_OFFSET
    or      a1, a1, a2
    sd      a1, PHY_CTRL_1_3_ADDR(t7)

    ld      a1, PHY_CTRL_1_2_ADDR(t7)
    dli     a2, PHY_CTRL_1_MASK
    dsll    a2, a2, PHY_CTRL_1_2_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsrl    a2, t2, 0x10
    and     a2, a2, 0x7f
    dsll    a2, a2, PHY_CTRL_1_2_OFFSET
    or      a1, a1, a2
    sd      a1, PHY_CTRL_1_2_ADDR(t7)

    ld      a1, PHY_CTRL_1_1_ADDR(t7)
    dli     a2, PHY_CTRL_1_MASK
    dsll    a2, a2, PHY_CTRL_1_1_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsrl    a2, t2, 0x8
    and     a2, a2, 0x7f
    dsll    a2, a2, PHY_CTRL_1_1_OFFSET
    or      a1, a1, a2
    sd      a1, PHY_CTRL_1_1_ADDR(t7)

    ld      a1, PHY_CTRL_1_0_ADDR(t7)
    dli     a2, PHY_CTRL_1_MASK
    dsll    a2, a2, PHY_CTRL_1_0_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsrl    a2, t2, 0x0
    and     a2, a2, 0x7f
    dsll    a2, a2, PHY_CTRL_1_0_OFFSET
    or      a1, a1, a2
    sd      a1, PHY_CTRL_1_0_ADDR(t7)
    b       40f
    nop
40:
    sync
#endif

    //enable Gather FIFO
    ld      a1, PHY_CTRL_2_ADDR(t7)
    dli     a2, 0x1
    dsll    a2, a2, PHY_CTRL_2_OFFSET + RESET_GFIFO_SHIFT
    not     a2, a2
    and     a1, a1, a2
    sd      a1, PHY_CTRL_2_ADDR(t7)

    //set start to 1
    //PRINTSTR("\r\nSet param_start 1.");
    dli     a2, 0x1
    dsll    a2, a2, START_OFFSET
    ld      a1, START_ADDR(t7)
    or      a1, a1, a2
    sd      a1, START_ADDR(t7)
    sync

    //poll until DLL locked.
    dli     a2, 0x1
1:
    ld      a1, DLLLOCKREG_ADDR(t7)
    and     a1, a1, a2
    beqz    a1, 1b
    nop

#ifdef  ARBLVL_PUT_DRAM_SREF
    //pull memory out of self-refresh
    ld      a1, SREFRESH_ADDR(t7)
    dli     a2, 0x1
    dsll    a2, a2, SREFRESH_OFFSET
    not     a2, a2
    and     a1, a1, a2
    sd      a1, SREFRESH_ADDR(t7)
    sync

    //delay some time
    dli     a2, 0x400
1:
    daddiu  a2, a2, -1
    bnez    a2, 1b
    nop
#endif
    
    //PRINTSTR("\r\nDisable DDR MC config space.\r\n");
    bal     disable_ddr_confspace
    nop

#ifdef  CONTROL_L2XBAR_DDR_WINDOW
    //PRINTSTR("\r\nEnable DDR access window.")
    sync
    sync
    sync
    sync
    sync
    sync
    sync
    sync
    //Enable L2XBAR_WINDOW
    dli     t7, L2XBAR_CONFIG_BASE_ADDR
#ifdef LS3B
    GET_ARB_LEVEL_NODE_ID
    dsll    a1, a1, 14
    daddu   t7, t7, a1
#endif
    GET_ARB_LEVEL_NODE_ID
    dsll    a1, a1, 44
    or      t7, t7, a1
    daddu   t7, t7, ARB_TEMP_L2WINDOW_OFFSET
    ld      a0, 0x80(t7)
    or      a0, a0, 0xf0
    sd      a0, 0x80(t7)
#endif

#ifdef  ADD_DELAY_AFTER_RESET_PHY
    //!!!!!!!!!!!!!!!!!!!!!!!!!!!
    //this delay can't be removed. wired!
    //delay some time, how long is proper?
    dli     a2, MC_RST_DELAY
1:
    daddiu  a2, a2, -1
    bnez    a2, 1b
    nop
#endif
