/*
 * define some functions used by ARB_level 
 * 
 * author:  Chen Xinke
 * date:    201209
 */

arb_modify_param:
/*******************
function:
    Modify specified one byte slice param
input param:
    t6: 1 ~ 7, param to be changed
    now only support: 1, 2, 3, 4, 5, 6, 7
    s3: 0 ~ 7---byte slice to be changed
        0xf ----change all 8 slices
    t2: target value
use register:
    t1: save ra
*******************/
    move    t1, ra

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
    sync
#endif

    //PRINTSTR("\r\nEnable DDR MC config space.")
    dli     t7, CHIP_CONFIG_BASE_ADDR
    GET_ARB_LEVEL_NODE_ID
#ifdef LS3B
    and     a1, a1, 0xe
#endif
    dsll    a1, a1, 44
    or      t7, t7, a1
    li      a2, 0x1
    sll     a2, a2, DDR_CONFIG_DISABLE_OFFSET
#ifdef LS3B
    //ODD NODE sll 5
    GET_ARB_LEVEL_NODE_ID
    and     a1, a1, 0x1
    beqz    a1, 1f
    nop
    sll     a2, a2, 5
1:
#endif
    not     a2, a2
    lw      a1, 0x0(t7)
    and     a1, a1, a2
    sw      a1, 0x0(t7)
    sync

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
    //PRINTSTR("\r\nChange param value.")

//!!!!!note: don't change the switch order of the code bellow, because we use
//add instr to change a1 instead of dli instr to reduce code size.
    dli     a1, 0x1
    beq     t6, a1, 1f
    nop
    daddiu  a1, a1, 0x1
    beq     t6, a1, 2f
    nop
    daddiu  a1, a1, 0x1
    beq     t6, a1, 3f
    nop
    daddiu  a1, a1, 0x1
    beq     t6, a1, 4f
    nop
    daddiu  a1, a1, 0x1
    beq     t6, a1, 5f
    nop
    daddiu  a1, a1, 0x1
    beq     t6, a1, 6f
    nop
    daddiu  a1, a1, 0x1
    beq     t6, a1, 7f
    nop
    //PRINTSTR("\r\n--------Wrong selection: no parameter will be changed.")
    b       88f
    nop
1:
    ld      a1, CLKLVL_DELAY_2_ADDR(t7)
    dli     a2, CLKLVL_DELAY_MASK
    dsll    a2, a2, CLKLVL_DELAY_2_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t2, CLKLVL_DELAY_2_OFFSET
    or      a1, a1, a2
    sd      a1, CLKLVL_DELAY_2_ADDR(t7)

    ld      a1, CLKLVL_DELAY_1_ADDR(t7)
    dli     a2, CLKLVL_DELAY_MASK
    dsll    a2, a2, CLKLVL_DELAY_1_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t2, CLKLVL_DELAY_1_OFFSET
    or      a1, a1, a2
    sd      a1, CLKLVL_DELAY_1_ADDR(t7)

    ld      a1, CLKLVL_DELAY_0_ADDR(t7)
    dli     a2, CLKLVL_DELAY_MASK
    dsll    a2, a2, CLKLVL_DELAY_0_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t2, CLKLVL_DELAY_0_OFFSET
    or      a1, a1, a2
    sd      a1, CLKLVL_DELAY_0_ADDR(t7)

    b       88f
    nop
2:
    dli     a1, 0x0
    beq     s3, a1, 20f
    nop
    daddiu  a1, a1, 0x1
    beq     s3, a1, 21f
    nop
    daddiu  a1, a1, 0x1
    beq     s3, a1, 22f
    nop
    daddiu  a1, a1, 0x1
    beq     s3, a1, 23f
    nop
    daddiu  a1, a1, 0x1
    beq     s3, a1, 24f
    nop
    daddiu  a1, a1, 0x1
    beq     s3, a1, 25f
    nop
    daddiu  a1, a1, 0x1
    beq     s3, a1, 26f
    nop
    daddiu  a1, a1, 0x1
    beq     s3, a1, 27f
    nop
    dli     a1, 0xf
    bne     s3, a1, 88f
    nop

20:
    ld      a1, RDLVL_GATE_DELAY_0_ADDR(t7)
    dli     a2, RDLVL_GATE_DELAY_MASK
    dsll    a2, a2, RDLVL_GATE_DELAY_0_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t2, RDLVL_GATE_DELAY_0_OFFSET
    or      a1, a1, a2
    sd      a1, RDLVL_GATE_DELAY_0_ADDR(t7)

    dli     a1, 0xf
    bne     s3, a1, 88f
    nop
21:
    ld      a1, RDLVL_GATE_DELAY_1_ADDR(t7)
    dli     a2, RDLVL_GATE_DELAY_MASK
    dsll    a2, a2, RDLVL_GATE_DELAY_1_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t2, RDLVL_GATE_DELAY_1_OFFSET
    or      a1, a1, a2
    sd      a1, RDLVL_GATE_DELAY_1_ADDR(t7)

    dli     a1, 0xf
    bne     s3, a1, 88f
    nop
22:
    ld      a1, RDLVL_GATE_DELAY_2_ADDR(t7)
    dli     a2, RDLVL_GATE_DELAY_MASK
    dsll    a2, a2, RDLVL_GATE_DELAY_2_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t2, RDLVL_GATE_DELAY_2_OFFSET
    or      a1, a1, a2
    sd      a1, RDLVL_GATE_DELAY_2_ADDR(t7)

    dli     a1, 0xf
    bne     s3, a1, 88f
    nop
23:
    ld      a1, RDLVL_GATE_DELAY_3_ADDR(t7)
    dli     a2, RDLVL_GATE_DELAY_MASK
    dsll    a2, a2, RDLVL_GATE_DELAY_3_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t2, RDLVL_GATE_DELAY_3_OFFSET
    or      a1, a1, a2
    sd      a1, RDLVL_GATE_DELAY_3_ADDR(t7)

    dli     a1, 0xf
    bne     s3, a1, 88f
    nop
24:
    ld      a1, RDLVL_GATE_DELAY_4_ADDR(t7)
    dli     a2, RDLVL_GATE_DELAY_MASK
    dsll    a2, a2, RDLVL_GATE_DELAY_4_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t2, RDLVL_GATE_DELAY_4_OFFSET
    or      a1, a1, a2
    sd      a1, RDLVL_GATE_DELAY_4_ADDR(t7)

    dli     a1, 0xf
    bne     s3, a1, 88f
    nop
25:
    ld      a1, RDLVL_GATE_DELAY_5_ADDR(t7)
    dli     a2, RDLVL_GATE_DELAY_MASK
    dsll    a2, a2, RDLVL_GATE_DELAY_5_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t2, RDLVL_GATE_DELAY_5_OFFSET
    or      a1, a1, a2
    sd      a1, RDLVL_GATE_DELAY_5_ADDR(t7)

    dli     a1, 0xf
    bne     s3, a1, 88f
    nop
26:
    ld      a1, RDLVL_GATE_DELAY_6_ADDR(t7)
    dli     a2, RDLVL_GATE_DELAY_MASK
    dsll    a2, a2, RDLVL_GATE_DELAY_6_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t2, RDLVL_GATE_DELAY_6_OFFSET
    or      a1, a1, a2
    sd      a1, RDLVL_GATE_DELAY_6_ADDR(t7)

    dli     a1, 0xf
    bne     s3, a1, 88f
    nop
27:
    ld      a1, RDLVL_GATE_DELAY_7_ADDR(t7)
    dli     a2, RDLVL_GATE_DELAY_MASK
    dsll    a2, a2, RDLVL_GATE_DELAY_7_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t2, RDLVL_GATE_DELAY_7_OFFSET
    or      a1, a1, a2
    sd      a1, RDLVL_GATE_DELAY_7_ADDR(t7)

    b       88f
    nop
3:
    dli     a1, 0x0
    beq     s3, a1, 30f
    nop
    daddiu  a1, a1, 0x1
    beq     s3, a1, 31f
    nop
    daddiu  a1, a1, 0x1
    beq     s3, a1, 32f
    nop
    daddiu  a1, a1, 0x1
    beq     s3, a1, 33f
    nop
    daddiu  a1, a1, 0x1
    beq     s3, a1, 34f
    nop
    daddiu  a1, a1, 0x1
    beq     s3, a1, 35f
    nop
    daddiu  a1, a1, 0x1
    beq     s3, a1, 36f
    nop
    daddiu  a1, a1, 0x1
    beq     s3, a1, 37f
    nop
    dli     a1, 0xf
    bne     s3, a1, 88f
    nop

30:
    ld      a1, RDLVL_DELAY_0_ADDR(t7)
    dli     a2, RDLVL_DELAY_MASK
    dsll    a2, a2, RDLVL_DELAY_0_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t2, RDLVL_DELAY_0_OFFSET
    or      a1, a1, a2
    sd      a1, RDLVL_DELAY_0_ADDR(t7)

    dli     a1, 0xf
    bne     s3, a1, 88f
    nop
31:
    ld      a1, RDLVL_DELAY_1_ADDR(t7)
    dli     a2, RDLVL_DELAY_MASK
    dsll    a2, a2, RDLVL_DELAY_1_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t2, RDLVL_DELAY_1_OFFSET
    or      a1, a1, a2
    sd      a1, RDLVL_DELAY_1_ADDR(t7)

    dli     a1, 0xf
    bne     s3, a1, 88f
    nop
32:
    ld      a1, RDLVL_DELAY_2_ADDR(t7)
    dli     a2, RDLVL_DELAY_MASK
    dsll    a2, a2, RDLVL_DELAY_2_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t2, RDLVL_DELAY_2_OFFSET
    or      a1, a1, a2
    sd      a1, RDLVL_DELAY_2_ADDR(t7)

    dli     a1, 0xf
    bne     s3, a1, 88f
    nop
33:
    ld      a1, RDLVL_DELAY_3_ADDR(t7)
    dli     a2, RDLVL_DELAY_MASK
    dsll    a2, a2, RDLVL_DELAY_3_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t2, RDLVL_DELAY_3_OFFSET
    or      a1, a1, a2
    sd      a1, RDLVL_DELAY_3_ADDR(t7)

    dli     a1, 0xf
    bne     s3, a1, 88f
    nop
34:
    ld      a1, RDLVL_DELAY_4_ADDR(t7)
    dli     a2, RDLVL_DELAY_MASK
    dsll    a2, a2, RDLVL_DELAY_4_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t2, RDLVL_DELAY_4_OFFSET
    or      a1, a1, a2
    sd      a1, RDLVL_DELAY_4_ADDR(t7)

    dli     a1, 0xf
    bne     s3, a1, 88f
    nop
35:
    ld      a1, RDLVL_DELAY_5_ADDR(t7)
    dli     a2, RDLVL_DELAY_MASK
    dsll    a2, a2, RDLVL_DELAY_5_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t2, RDLVL_DELAY_5_OFFSET
    or      a1, a1, a2
    sd      a1, RDLVL_DELAY_5_ADDR(t7)

    dli     a1, 0xf
    bne     s3, a1, 88f
    nop
36:
    ld      a1, RDLVL_DELAY_6_ADDR(t7)
    dli     a2, RDLVL_DELAY_MASK
    dsll    a2, a2, RDLVL_DELAY_6_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t2, RDLVL_DELAY_6_OFFSET
    or      a1, a1, a2
    sd      a1, RDLVL_DELAY_6_ADDR(t7)

    dli     a1, 0xf
    bne     s3, a1, 88f
    nop
37:
    ld      a1, RDLVL_DELAY_7_ADDR(t7)
    dli     a2, RDLVL_DELAY_MASK
    dsll    a2, a2, RDLVL_DELAY_7_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t2, RDLVL_DELAY_7_OFFSET
    or      a1, a1, a2
    sd      a1, RDLVL_DELAY_7_ADDR(t7)

    b       88f
    nop

4:
    dli     a1, 0x0
    beq     s3, a1, 40f
    nop
    daddiu  a1, a1, 0x1
    beq     s3, a1, 41f
    nop
    daddiu  a1, a1, 0x1
    beq     s3, a1, 42f
    nop
    daddiu  a1, a1, 0x1
    beq     s3, a1, 43f
    nop
    daddiu  a1, a1, 0x1
    beq     s3, a1, 44f
    nop
    daddiu  a1, a1, 0x1
    beq     s3, a1, 45f
    nop
    daddiu  a1, a1, 0x1
    beq     s3, a1, 46f
    nop
    daddiu  a1, a1, 0x1
    beq     s3, a1, 47f
    nop
    dli     a1, 0xf
    bne     s3, a1, 88f
    nop

40:
    ld      a1, RDLVL_DQSN_DELAY_0_ADDR(t7)
    dli     a2, RDLVL_DQSN_DELAY_MASK
    dsll    a2, a2, RDLVL_DQSN_DELAY_0_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t2, RDLVL_DQSN_DELAY_0_OFFSET
    or      a1, a1, a2
    sd      a1, RDLVL_DQSN_DELAY_0_ADDR(t7)

    dli     a1, 0xf
    bne     s3, a1, 88f
    nop
41:
    ld      a1, RDLVL_DQSN_DELAY_1_ADDR(t7)
    dli     a2, RDLVL_DQSN_DELAY_MASK
    dsll    a2, a2, RDLVL_DQSN_DELAY_1_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t2, RDLVL_DQSN_DELAY_1_OFFSET
    or      a1, a1, a2
    sd      a1, RDLVL_DQSN_DELAY_1_ADDR(t7)

    dli     a1, 0xf
    bne     s3, a1, 88f
    nop
42:
    ld      a1, RDLVL_DQSN_DELAY_2_ADDR(t7)
    dli     a2, RDLVL_DQSN_DELAY_MASK
    dsll    a2, a2, RDLVL_DQSN_DELAY_2_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t2, RDLVL_DQSN_DELAY_2_OFFSET
    or      a1, a1, a2
    sd      a1, RDLVL_DQSN_DELAY_2_ADDR(t7)

    dli     a1, 0xf
    bne     s3, a1, 88f
    nop
43:
    ld      a1, RDLVL_DQSN_DELAY_3_ADDR(t7)
    dli     a2, RDLVL_DQSN_DELAY_MASK
    dsll    a2, a2, RDLVL_DQSN_DELAY_3_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t2, RDLVL_DQSN_DELAY_3_OFFSET
    or      a1, a1, a2
    sd      a1, RDLVL_DQSN_DELAY_3_ADDR(t7)

    dli     a1, 0xf
    bne     s3, a1, 88f
    nop
44:
    ld      a1, RDLVL_DQSN_DELAY_4_ADDR(t7)
    dli     a2, RDLVL_DQSN_DELAY_MASK
    dsll    a2, a2, RDLVL_DQSN_DELAY_4_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t2, RDLVL_DQSN_DELAY_4_OFFSET
    or      a1, a1, a2
    sd      a1, RDLVL_DQSN_DELAY_4_ADDR(t7)

    dli     a1, 0xf
    bne     s3, a1, 88f
    nop
45:
    ld      a1, RDLVL_DQSN_DELAY_5_ADDR(t7)
    dli     a2, RDLVL_DQSN_DELAY_MASK
    dsll    a2, a2, RDLVL_DQSN_DELAY_5_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t2, RDLVL_DQSN_DELAY_5_OFFSET
    or      a1, a1, a2
    sd      a1, RDLVL_DQSN_DELAY_5_ADDR(t7)

    dli     a1, 0xf
    bne     s3, a1, 88f
    nop
46:
    ld      a1, RDLVL_DQSN_DELAY_6_ADDR(t7)
    dli     a2, RDLVL_DQSN_DELAY_MASK
    dsll    a2, a2, RDLVL_DQSN_DELAY_6_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t2, RDLVL_DQSN_DELAY_6_OFFSET
    or      a1, a1, a2
    sd      a1, RDLVL_DQSN_DELAY_6_ADDR(t7)

    dli     a1, 0xf
    bne     s3, a1, 88f
    nop
47:
    ld      a1, RDLVL_DQSN_DELAY_7_ADDR(t7)
    dli     a2, RDLVL_DQSN_DELAY_MASK
    dsll    a2, a2, RDLVL_DQSN_DELAY_7_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t2, RDLVL_DQSN_DELAY_7_OFFSET
    or      a1, a1, a2
    sd      a1, RDLVL_DQSN_DELAY_7_ADDR(t7)

    b       88f
    nop
5:
#ifdef  CLEAR_HALF_CLK_SHIFT
    //set phy_ctrl_reg_0[19:16][15:8]--one clk delay and add half clk delay and dqs_out_enable_window.
    dli     a2, WRLVL_1QUARTER_CLK_VALUE
    bge     t2, a2, 10f
    nop
    //WRLVL_1QUARTER_CLK_VALUE > wrlvl_delay
    dli     a1, PHY_0_DQSDQ_INC_VALUE_00P
    dli     a2, DQSDQ_OUT_WINDOW_VALUE
    daddu   a3, a1, a2
    b       18f
    nop
10:
    //t2 >= WRLVL_1QUARTER_CLK_VALUE
    dli     a2, WRLVL_HALF_CLK_VALUE
    bge     t2, a2, 11f
    nop
    //WRLVL_HALF_CLK_VALUE > wrlvl_delay >= WRLVL_1QUARTER_CLK_VALUE 
    dli     a1, PHY_0_DQSDQ_INC_VALUE_20P
    dli     a2, DQSDQ_OUT_WINDOW_VALUE
    daddu   a3, a1, a2
    b       18f
    nop
11:
    //t2 >= WRLVL_HALF_CLK_VALUE
    dli     a2, WRLVL_3QUARTER_CLK_VALUE
    bge     t2, a2, 12f
    nop
    //WRLVL_3QUARTER_CLK_VALUE > wrlvl_delay >= WRLVL_HALF_CLK_VALUE
    dli     a1, PHY_0_DQSDQ_INC_VALUE_40P
    dli     a2, DQSDQ_OUT_WINDOW_VALUE
    daddu   a3, a1, a2
    b       18f
    nop
12:
    //t2 >= WRLVL_3QUARTER_CLK_VALUE
    dli     a2, WRLVL_ONE_CLK_VALUE
    bge     t2, a2, 13f
    nop
    //WRLVL_ONE_CLK_VALUE > wrlvl_delay >= WRLVL_3QUARTER_CLK_VALUE
    dli     a1, PHY_0_DQSDQ_INC_VALUE_60P
    dli     a2, DQSDQ_OUT_WINDOW_VALUE
    daddu   a3, a1, a2
    b       18f
    nop
13:
    //t2 >= WRLVL_ONE_CLK_VALUE
    //wrlvl_delay >= WRLVL_ONE_CLK_VALUE
    dli     a1, PHY_0_DQSDQ_INC_VALUE_80P
    dli     a2, DQSDQ_OUT_WINDOW_VALUE
    daddu   a3, a1, a2
    b       18f
    nop
18:
#endif
    dli     a1, 0x0
    beq     s3, a1, 50f
    nop
    daddiu  a1, a1, 0x1
    beq     s3, a1, 51f
    nop
    daddiu  a1, a1, 0x1
    beq     s3, a1, 52f
    nop
    daddiu  a1, a1, 0x1
    beq     s3, a1, 53f
    nop
    daddiu  a1, a1, 0x1
    beq     s3, a1, 54f
    nop
    daddiu  a1, a1, 0x1
    beq     s3, a1, 55f
    nop
    daddiu  a1, a1, 0x1
    beq     s3, a1, 56f
    nop
    daddiu  a1, a1, 0x1
    beq     s3, a1, 57f
    nop
    dli     a1, 0xf
    bne     s3, a1, 88f
    nop

50:
    ld      a1, WRLVL_DELAY_0_ADDR(t7)
    dli     a2, WRLVL_DELAY_MASK
    dsll    a2, a2, WRLVL_DELAY_0_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t2, WRLVL_DELAY_0_OFFSET
    or      a1, a1, a2
    sd      a1, WRLVL_DELAY_0_ADDR(t7)

#ifdef  CLEAR_HALF_CLK_SHIFT
    //modify phy_ctrl_reg_0[19:16][15:8]
    ld      a1, PHY_CTRL_0_0_ADDR(t7)
    dli     a2, PHY_CTRL_0_ADJ_MASK
    dsll    a2, a2, PHY_CTRL_0_0_OFFSET + PHY_CTRL_0_ADDWLDLY_SHIFT
    not     a2, a2
    and     a1, a1, a2
    dsrl    a2, a3, 16
    dsll    a2, a2, PHY_CTRL_0_0_OFFSET + PHY_CTRL_0_ADDWLDLY_SHIFT
    or      a1, a1, a2
    sd      a1, PHY_CTRL_0_0_ADDR(t7)

#ifdef  MODIFY_DQSDQ_OUT_WINDOW
    ld      a1, PHY_CTRL_0_0_ADDR(t7)
    dli     a2, DQSDQ_OUT_WINDOW_MASK
    dsll    a2, a2, PHY_CTRL_0_0_OFFSET + DQSDQ_OUT_WINDOW_SHIFT
    not     a2, a2
    and     a1, a1, a2
    and     a2, a3, DQSDQ_OUT_WINDOW_MASK
    dsll    a2, a2, PHY_CTRL_0_0_OFFSET + DQSDQ_OUT_WINDOW_SHIFT
    or      a1, a1, a2
    sd      a1, PHY_CTRL_0_0_ADDR(t7)
#endif
#endif
    dli     a1, 0xf
    bne     s3, a1, 88f
    nop
51:
    ld      a1, WRLVL_DELAY_1_ADDR(t7)
    dli     a2, WRLVL_DELAY_MASK
    dsll    a2, a2, WRLVL_DELAY_1_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t2, WRLVL_DELAY_1_OFFSET
    or      a1, a1, a2
    sd      a1, WRLVL_DELAY_1_ADDR(t7)

#ifdef  CLEAR_HALF_CLK_SHIFT
    //modify phy_ctrl_reg_0[19:16][15:8]
    ld      a1, PHY_CTRL_0_1_ADDR(t7)
    dli     a2, PHY_CTRL_0_ADJ_MASK
    dsll    a2, a2, PHY_CTRL_0_1_OFFSET + PHY_CTRL_0_ADDWLDLY_SHIFT
    not     a2, a2
    and     a1, a1, a2
    dsrl    a2, a3, 16
    dsll    a2, a2, PHY_CTRL_0_1_OFFSET + PHY_CTRL_0_ADDWLDLY_SHIFT
    or      a1, a1, a2
    sd      a1, PHY_CTRL_0_1_ADDR(t7)

#ifdef  MODIFY_DQSDQ_OUT_WINDOW
    ld      a1, PHY_CTRL_0_1_ADDR(t7)
    dli     a2, DQSDQ_OUT_WINDOW_MASK
    dsll    a2, a2, PHY_CTRL_0_1_OFFSET + DQSDQ_OUT_WINDOW_SHIFT
    not     a2, a2
    and     a1, a1, a2
    and     a2, a3, DQSDQ_OUT_WINDOW_MASK
    dsll    a2, a2, PHY_CTRL_0_1_OFFSET + DQSDQ_OUT_WINDOW_SHIFT
    or      a1, a1, a2
    sd      a1, PHY_CTRL_0_1_ADDR(t7)
#endif
#endif
    dli     a1, 0xf
    bne     s3, a1, 88f
    nop
52:
    ld      a1, WRLVL_DELAY_2_ADDR(t7)
    dli     a2, WRLVL_DELAY_MASK
    dsll    a2, a2, WRLVL_DELAY_2_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t2, WRLVL_DELAY_2_OFFSET
    or      a1, a1, a2
    sd      a1, WRLVL_DELAY_2_ADDR(t7)

#ifdef  CLEAR_HALF_CLK_SHIFT
    //modify phy_ctrl_reg_0[19:16][15:8]
    ld      a1, PHY_CTRL_0_2_ADDR(t7)
    dli     a2, PHY_CTRL_0_ADJ_MASK
    dsll    a2, a2, PHY_CTRL_0_2_OFFSET + PHY_CTRL_0_ADDWLDLY_SHIFT
    not     a2, a2
    and     a1, a1, a2
    dsrl    a2, a3, 16
    dsll    a2, a2, PHY_CTRL_0_2_OFFSET + PHY_CTRL_0_ADDWLDLY_SHIFT
    or      a1, a1, a2
    sd      a1, PHY_CTRL_0_2_ADDR(t7)

#ifdef  MODIFY_DQSDQ_OUT_WINDOW
    ld      a1, PHY_CTRL_0_2_ADDR(t7)
    dli     a2, DQSDQ_OUT_WINDOW_MASK
    dsll    a2, a2, PHY_CTRL_0_2_OFFSET + DQSDQ_OUT_WINDOW_SHIFT
    not     a2, a2
    and     a1, a1, a2
    and     a2, a3, DQSDQ_OUT_WINDOW_MASK
    dsll    a2, a2, PHY_CTRL_0_2_OFFSET + DQSDQ_OUT_WINDOW_SHIFT
    or      a1, a1, a2
    sd      a1, PHY_CTRL_0_2_ADDR(t7)
#endif
#endif
    dli     a1, 0xf
    bne     s3, a1, 88f
    nop
53:
    ld      a1, WRLVL_DELAY_3_ADDR(t7)
    dli     a2, WRLVL_DELAY_MASK
    dsll    a2, a2, WRLVL_DELAY_3_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t2, WRLVL_DELAY_3_OFFSET
    or      a1, a1, a2
    sd      a1, WRLVL_DELAY_3_ADDR(t7)

#ifdef  CLEAR_HALF_CLK_SHIFT
    //modify phy_ctrl_reg_0[19:16][15:8]
    ld      a1, PHY_CTRL_0_3_ADDR(t7)
    dli     a2, PHY_CTRL_0_ADJ_MASK
    dsll    a2, a2, PHY_CTRL_0_3_OFFSET + PHY_CTRL_0_ADDWLDLY_SHIFT
    not     a2, a2
    and     a1, a1, a2
    dsrl    a2, a3, 16
    dsll    a2, a2, PHY_CTRL_0_3_OFFSET + PHY_CTRL_0_ADDWLDLY_SHIFT
    or      a1, a1, a2
    sd      a1, PHY_CTRL_0_3_ADDR(t7)

#ifdef  MODIFY_DQSDQ_OUT_WINDOW
    ld      a1, PHY_CTRL_0_3_ADDR(t7)
    dli     a2, DQSDQ_OUT_WINDOW_MASK
    dsll    a2, a2, PHY_CTRL_0_3_OFFSET + DQSDQ_OUT_WINDOW_SHIFT
    not     a2, a2
    and     a1, a1, a2
    and     a2, a3, DQSDQ_OUT_WINDOW_MASK
    dsll    a2, a2, PHY_CTRL_0_3_OFFSET + DQSDQ_OUT_WINDOW_SHIFT
    or      a1, a1, a2
    sd      a1, PHY_CTRL_0_3_ADDR(t7)
#endif
#endif
    dli     a1, 0xf
    bne     s3, a1, 88f
    nop
54:
    ld      a1, WRLVL_DELAY_4_ADDR(t7)
    dli     a2, WRLVL_DELAY_MASK
    dsll    a2, a2, WRLVL_DELAY_4_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t2, WRLVL_DELAY_4_OFFSET
    or      a1, a1, a2
    sd      a1, WRLVL_DELAY_4_ADDR(t7)

#ifdef  CLEAR_HALF_CLK_SHIFT
    //modify phy_ctrl_reg_0[19:16][15:8]
    ld      a1, PHY_CTRL_0_4_ADDR(t7)
    dli     a2, PHY_CTRL_0_ADJ_MASK
    dsll    a2, a2, PHY_CTRL_0_4_OFFSET + PHY_CTRL_0_ADDWLDLY_SHIFT
    not     a2, a2
    and     a1, a1, a2
    dsrl    a2, a3, 16
    dsll    a2, a2, PHY_CTRL_0_4_OFFSET + PHY_CTRL_0_ADDWLDLY_SHIFT
    or      a1, a1, a2
    sd      a1, PHY_CTRL_0_4_ADDR(t7)

#ifdef  MODIFY_DQSDQ_OUT_WINDOW
    ld      a1, PHY_CTRL_0_4_ADDR(t7)
    dli     a2, DQSDQ_OUT_WINDOW_MASK
    dsll    a2, a2, PHY_CTRL_0_4_OFFSET + DQSDQ_OUT_WINDOW_SHIFT
    not     a2, a2
    and     a1, a1, a2
    and     a2, a3, DQSDQ_OUT_WINDOW_MASK
    dsll    a2, a2, PHY_CTRL_0_4_OFFSET + DQSDQ_OUT_WINDOW_SHIFT
    or      a1, a1, a2
    sd      a1, PHY_CTRL_0_4_ADDR(t7)
#endif
#endif
    dli     a1, 0xf
    bne     s3, a1, 88f
    nop
55:
    ld      a1, WRLVL_DELAY_5_ADDR(t7)
    dli     a2, WRLVL_DELAY_MASK
    dsll    a2, a2, WRLVL_DELAY_5_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t2, WRLVL_DELAY_5_OFFSET
    or      a1, a1, a2
    sd      a1, WRLVL_DELAY_5_ADDR(t7)

#ifdef  CLEAR_HALF_CLK_SHIFT
    //modify phy_ctrl_reg_0[19:16][15:8]
    ld      a1, PHY_CTRL_0_5_ADDR(t7)
    dli     a2, PHY_CTRL_0_ADJ_MASK
    dsll    a2, a2, PHY_CTRL_0_5_OFFSET + PHY_CTRL_0_ADDWLDLY_SHIFT
    not     a2, a2
    and     a1, a1, a2
    dsrl    a2, a3, 16
    dsll    a2, a2, PHY_CTRL_0_5_OFFSET + PHY_CTRL_0_ADDWLDLY_SHIFT
    or      a1, a1, a2
    sd      a1, PHY_CTRL_0_5_ADDR(t7)

#ifdef  MODIFY_DQSDQ_OUT_WINDOW
    ld      a1, PHY_CTRL_0_5_ADDR(t7)
    dli     a2, DQSDQ_OUT_WINDOW_MASK
    dsll    a2, a2, PHY_CTRL_0_5_OFFSET + DQSDQ_OUT_WINDOW_SHIFT
    not     a2, a2
    and     a1, a1, a2
    and     a2, a3, DQSDQ_OUT_WINDOW_MASK
    dsll    a2, a2, PHY_CTRL_0_5_OFFSET + DQSDQ_OUT_WINDOW_SHIFT
    or      a1, a1, a2
    sd      a1, PHY_CTRL_0_5_ADDR(t7)
#endif
#endif
    dli     a1, 0xf
    bne     s3, a1, 88f
    nop
56:
    ld      a1, WRLVL_DELAY_6_ADDR(t7)
    dli     a2, WRLVL_DELAY_MASK
    dsll    a2, a2, WRLVL_DELAY_6_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t2, WRLVL_DELAY_6_OFFSET
    or      a1, a1, a2
    sd      a1, WRLVL_DELAY_6_ADDR(t7)

#ifdef  CLEAR_HALF_CLK_SHIFT
    //modify phy_ctrl_reg_0[19:16][15:8]
    ld      a1, PHY_CTRL_0_6_ADDR(t7)
    dli     a2, PHY_CTRL_0_ADJ_MASK
    dsll    a2, a2, PHY_CTRL_0_6_OFFSET + PHY_CTRL_0_ADDWLDLY_SHIFT
    not     a2, a2
    and     a1, a1, a2
    dsrl    a2, a3, 16
    dsll    a2, a2, PHY_CTRL_0_6_OFFSET + PHY_CTRL_0_ADDWLDLY_SHIFT
    or      a1, a1, a2
    sd      a1, PHY_CTRL_0_6_ADDR(t7)

#ifdef  MODIFY_DQSDQ_OUT_WINDOW
    ld      a1, PHY_CTRL_0_6_ADDR(t7)
    dli     a2, DQSDQ_OUT_WINDOW_MASK
    dsll    a2, a2, PHY_CTRL_0_6_OFFSET + DQSDQ_OUT_WINDOW_SHIFT
    not     a2, a2
    and     a1, a1, a2
    and     a2, a3, DQSDQ_OUT_WINDOW_MASK
    dsll    a2, a2, PHY_CTRL_0_6_OFFSET + DQSDQ_OUT_WINDOW_SHIFT
    or      a1, a1, a2
    sd      a1, PHY_CTRL_0_6_ADDR(t7)
#endif
#endif
    dli     a1, 0xf
    bne     s3, a1, 88f
    nop
57:
    ld      a1, WRLVL_DELAY_7_ADDR(t7)
    dli     a2, WRLVL_DELAY_MASK
    dsll    a2, a2, WRLVL_DELAY_7_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t2, WRLVL_DELAY_7_OFFSET
    or      a1, a1, a2
    sd      a1, WRLVL_DELAY_7_ADDR(t7)

#ifdef  CLEAR_HALF_CLK_SHIFT
    //modify phy_ctrl_reg_0[19:16][15:8]
    ld      a1, PHY_CTRL_0_7_ADDR(t7)
    dli     a2, PHY_CTRL_0_ADJ_MASK
    dsll    a2, a2, PHY_CTRL_0_7_OFFSET + PHY_CTRL_0_ADDWLDLY_SHIFT
    not     a2, a2
    and     a1, a1, a2
    dsrl    a2, a3, 16
    dsll    a2, a2, PHY_CTRL_0_7_OFFSET + PHY_CTRL_0_ADDWLDLY_SHIFT
    or      a1, a1, a2
    sd      a1, PHY_CTRL_0_7_ADDR(t7)

#ifdef  MODIFY_DQSDQ_OUT_WINDOW
    ld      a1, PHY_CTRL_0_7_ADDR(t7)
    dli     a2, DQSDQ_OUT_WINDOW_MASK
    dsll    a2, a2, PHY_CTRL_0_7_OFFSET + DQSDQ_OUT_WINDOW_SHIFT
    not     a2, a2
    and     a1, a1, a2
    and     a2, a3, DQSDQ_OUT_WINDOW_MASK
    dsll    a2, a2, PHY_CTRL_0_7_OFFSET + DQSDQ_OUT_WINDOW_SHIFT
    or      a1, a1, a2
    sd      a1, PHY_CTRL_0_7_ADDR(t7)
#endif
#endif
    b       88f
    nop
6:
    dli     a1, 0x0
    beq     s3, a1, 60f
    nop
    daddiu  a1, a1, 0x1
    beq     s3, a1, 61f
    nop
    daddiu  a1, a1, 0x1
    beq     s3, a1, 62f
    nop
    daddiu  a1, a1, 0x1
    beq     s3, a1, 63f
    nop
    daddiu  a1, a1, 0x1
    beq     s3, a1, 64f
    nop
    daddiu  a1, a1, 0x1
    beq     s3, a1, 65f
    nop
    daddiu  a1, a1, 0x1
    beq     s3, a1, 66f
    nop
    daddiu  a1, a1, 0x1
    beq     s3, a1, 67f
    nop
    dli     a1, 0xf
    bne     s3, a1, 88f
    nop

60:
    ld      a1, WRLVL_DQ_DELAY_0_ADDR(t7)
    dli     a2, WRLVL_DQ_DELAY_MASK
    dsll    a2, a2, WRLVL_DQ_DELAY_0_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t2, WRLVL_DQ_DELAY_0_OFFSET
    or      a1, a1, a2
    sd      a1, WRLVL_DQ_DELAY_0_ADDR(t7)

    dli     a1, 0xf
    bne     s3, a1, 88f
    nop
61:
    ld      a1, WRLVL_DQ_DELAY_1_ADDR(t7)
    dli     a2, WRLVL_DQ_DELAY_MASK
    dsll    a2, a2, WRLVL_DQ_DELAY_1_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t2, WRLVL_DQ_DELAY_1_OFFSET
    or      a1, a1, a2
    sd      a1, WRLVL_DQ_DELAY_1_ADDR(t7)

    dli     a1, 0xf
    bne     s3, a1, 88f
    nop
62:
    ld      a1, WRLVL_DQ_DELAY_2_ADDR(t7)
    dli     a2, WRLVL_DQ_DELAY_MASK
    dsll    a2, a2, WRLVL_DQ_DELAY_2_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t2, WRLVL_DQ_DELAY_2_OFFSET
    or      a1, a1, a2
    sd      a1, WRLVL_DQ_DELAY_2_ADDR(t7)

    dli     a1, 0xf
    bne     s3, a1, 88f
    nop
63:
    ld      a1, WRLVL_DQ_DELAY_3_ADDR(t7)
    dli     a2, WRLVL_DQ_DELAY_MASK
    dsll    a2, a2, WRLVL_DQ_DELAY_3_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t2, WRLVL_DQ_DELAY_3_OFFSET
    or      a1, a1, a2
    sd      a1, WRLVL_DQ_DELAY_3_ADDR(t7)

    dli     a1, 0xf
    bne     s3, a1, 88f
    nop
64:
    ld      a1, WRLVL_DQ_DELAY_4_ADDR(t7)
    dli     a2, WRLVL_DQ_DELAY_MASK
    dsll    a2, a2, WRLVL_DQ_DELAY_4_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t2, WRLVL_DQ_DELAY_4_OFFSET
    or      a1, a1, a2
    sd      a1, WRLVL_DQ_DELAY_4_ADDR(t7)

    dli     a1, 0xf
    bne     s3, a1, 88f
    nop
65:
    ld      a1, WRLVL_DQ_DELAY_5_ADDR(t7)
    dli     a2, WRLVL_DQ_DELAY_MASK
    dsll    a2, a2, WRLVL_DQ_DELAY_5_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t2, WRLVL_DQ_DELAY_5_OFFSET
    or      a1, a1, a2
    sd      a1, WRLVL_DQ_DELAY_5_ADDR(t7)

    dli     a1, 0xf
    bne     s3, a1, 88f
    nop
66:
    ld      a1, WRLVL_DQ_DELAY_6_ADDR(t7)
    dli     a2, WRLVL_DQ_DELAY_MASK
    dsll    a2, a2, WRLVL_DQ_DELAY_6_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t2, WRLVL_DQ_DELAY_6_OFFSET
    or      a1, a1, a2
    sd      a1, WRLVL_DQ_DELAY_6_ADDR(t7)

    dli     a1, 0xf
    bne     s3, a1, 88f
    nop
67:
    ld      a1, WRLVL_DQ_DELAY_7_ADDR(t7)
    dli     a2, WRLVL_DQ_DELAY_MASK
    dsll    a2, a2, WRLVL_DQ_DELAY_7_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t2, WRLVL_DQ_DELAY_7_OFFSET
    or      a1, a1, a2
    sd      a1, WRLVL_DQ_DELAY_7_ADDR(t7)

    b       88f
    nop
7:
    dli     a1, 0x0
    beq     s3, a1, 70f
    nop
    daddiu  a1, a1, 0x1
    beq     s3, a1, 71f
    nop
    daddiu  a1, a1, 0x1
    beq     s3, a1, 72f
    nop
    daddiu  a1, a1, 0x1
    beq     s3, a1, 73f
    nop
    daddiu  a1, a1, 0x1
    beq     s3, a1, 74f
    nop
    daddiu  a1, a1, 0x1
    beq     s3, a1, 75f
    nop
    daddiu  a1, a1, 0x1
    beq     s3, a1, 76f
    nop
    daddiu  a1, a1, 0x1
    beq     s3, a1, 77f
    nop
    dli     a1, 0xf
    bne     s3, a1, 88f
    nop

70:
    ld      a1, PHY_CTRL_1_0_ADDR(t7)
    dli     a2, PHY_CTRL_1_GATE_MASK
    dsll    a2, a2, PHY_CTRL_1_0_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t2, PHY_CTRL_1_0_OFFSET
    or      a1, a1, a2
    sd      a1, PHY_CTRL_1_0_ADDR(t7)

#ifdef  ADJUST_CPU_ODT
    ld      a1, PHY_CTRL_1_0_ADDR(t7)
    dli     a2, PHY_CTRL_1_RD_ODT_MASK
    dsll    a2, a2, PHY_CTRL_1_0_OFFSET + PHY_CTRL_1_RD_ODT_SHIFT
    not     a2, a2
    and     a1, a1, a2
    dli     a3, CPU_ODT_BASE_VALUE
    beqz    t2, 1f
    nop
    daddu   a3, a3, CPU_ODT_INC_VALUE
1:
    dsll    a2, a3, PHY_CTRL_1_0_OFFSET + PHY_CTRL_1_RD_ODT_SHIFT
    or      a1, a1, a2
    sd      a1, PHY_CTRL_1_0_ADDR(t7)
#endif

    dli     a1, 0xf
    bne     s3, a1, 88f
    nop
71:
    ld      a1, PHY_CTRL_1_1_ADDR(t7)
    dli     a2, PHY_CTRL_1_GATE_MASK
    dsll    a2, a2, PHY_CTRL_1_1_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t2, PHY_CTRL_1_1_OFFSET
    or      a1, a1, a2
    sd      a1, PHY_CTRL_1_1_ADDR(t7)

#ifdef  ADJUST_CPU_ODT
    ld      a1, PHY_CTRL_1_1_ADDR(t7)
    dli     a2, PHY_CTRL_1_RD_ODT_MASK
    dsll    a2, a2, PHY_CTRL_1_1_OFFSET + PHY_CTRL_1_RD_ODT_SHIFT
    not     a2, a2
    and     a1, a1, a2
    dli     a3, CPU_ODT_BASE_VALUE
    beqz    t2, 1f
    nop
    daddu   a3, a3, CPU_ODT_INC_VALUE
1:
    dsll    a2, a3, PHY_CTRL_1_1_OFFSET + PHY_CTRL_1_RD_ODT_SHIFT
    or      a1, a1, a2
    sd      a1, PHY_CTRL_1_1_ADDR(t7)
#endif

    dli     a1, 0xf
    bne     s3, a1, 88f
    nop
72:
    ld      a1, PHY_CTRL_1_2_ADDR(t7)
    dli     a2, PHY_CTRL_1_GATE_MASK
    dsll    a2, a2, PHY_CTRL_1_2_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t2, PHY_CTRL_1_2_OFFSET
    or      a1, a1, a2
    sd      a1, PHY_CTRL_1_2_ADDR(t7)

#ifdef  ADJUST_CPU_ODT
    ld      a1, PHY_CTRL_1_2_ADDR(t7)
    dli     a2, PHY_CTRL_1_RD_ODT_MASK
    dsll    a2, a2, PHY_CTRL_1_2_OFFSET + PHY_CTRL_1_RD_ODT_SHIFT
    not     a2, a2
    and     a1, a1, a2
    dli     a3, CPU_ODT_BASE_VALUE
    beqz    t2, 1f
    nop
    daddu   a3, a3, CPU_ODT_INC_VALUE
1:
    dsll    a2, a3, PHY_CTRL_1_2_OFFSET + PHY_CTRL_1_RD_ODT_SHIFT
    or      a1, a1, a2
    sd      a1, PHY_CTRL_1_2_ADDR(t7)
#endif

    dli     a1, 0xf
    bne     s3, a1, 88f
    nop
73:
    ld      a1, PHY_CTRL_1_3_ADDR(t7)
    dli     a2, PHY_CTRL_1_GATE_MASK
    dsll    a2, a2, PHY_CTRL_1_3_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t2, PHY_CTRL_1_3_OFFSET
    or      a1, a1, a2
    sd      a1, PHY_CTRL_1_3_ADDR(t7)

#ifdef  ADJUST_CPU_ODT
    ld      a1, PHY_CTRL_1_3_ADDR(t7)
    dli     a2, PHY_CTRL_1_RD_ODT_MASK
    dsll    a2, a2, PHY_CTRL_1_3_OFFSET + PHY_CTRL_1_RD_ODT_SHIFT
    not     a2, a2
    and     a1, a1, a2
    dli     a3, CPU_ODT_BASE_VALUE
    beqz    t2, 1f
    nop
    daddu   a3, a3, CPU_ODT_INC_VALUE
1:
    dsll    a2, a3, PHY_CTRL_1_3_OFFSET + PHY_CTRL_1_RD_ODT_SHIFT
    or      a1, a1, a2
    sd      a1, PHY_CTRL_1_3_ADDR(t7)
#endif

    dli     a1, 0xf
    bne     s3, a1, 88f
    nop
74:
    ld      a1, PHY_CTRL_1_4_ADDR(t7)
    dli     a2, PHY_CTRL_1_GATE_MASK
    dsll    a2, a2, PHY_CTRL_1_4_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t2, PHY_CTRL_1_4_OFFSET
    or      a1, a1, a2
    sd      a1, PHY_CTRL_1_4_ADDR(t7)

#ifdef  ADJUST_CPU_ODT
    ld      a1, PHY_CTRL_1_4_ADDR(t7)
    dli     a2, PHY_CTRL_1_RD_ODT_MASK
    dsll    a2, a2, PHY_CTRL_1_4_OFFSET + PHY_CTRL_1_RD_ODT_SHIFT
    not     a2, a2
    and     a1, a1, a2
    dli     a3, CPU_ODT_BASE_VALUE
    beqz    t2, 1f
    nop
    daddu   a3, a3, CPU_ODT_INC_VALUE
1:
    dsll    a2, a3, PHY_CTRL_1_4_OFFSET + PHY_CTRL_1_RD_ODT_SHIFT
    or      a1, a1, a2
    sd      a1, PHY_CTRL_1_4_ADDR(t7)
#endif

    dli     a1, 0xf
    bne     s3, a1, 88f
    nop
75:
    ld      a1, PHY_CTRL_1_5_ADDR(t7)
    dli     a2, PHY_CTRL_1_GATE_MASK
    dsll    a2, a2, PHY_CTRL_1_5_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t2, PHY_CTRL_1_5_OFFSET
    or      a1, a1, a2
    sd      a1, PHY_CTRL_1_5_ADDR(t7)

#ifdef  ADJUST_CPU_ODT
    ld      a1, PHY_CTRL_1_5_ADDR(t7)
    dli     a2, PHY_CTRL_1_RD_ODT_MASK
    dsll    a2, a2, PHY_CTRL_1_5_OFFSET + PHY_CTRL_1_RD_ODT_SHIFT
    not     a2, a2
    and     a1, a1, a2
    dli     a3, CPU_ODT_BASE_VALUE
    beqz    t2, 1f
    nop
    daddu   a3, a3, CPU_ODT_INC_VALUE
1:
    dsll    a2, a3, PHY_CTRL_1_5_OFFSET + PHY_CTRL_1_RD_ODT_SHIFT
    or      a1, a1, a2
    sd      a1, PHY_CTRL_1_5_ADDR(t7)
#endif

    dli     a1, 0xf
    bne     s3, a1, 88f
    nop
76:
    ld      a1, PHY_CTRL_1_6_ADDR(t7)
    dli     a2, PHY_CTRL_1_GATE_MASK
    dsll    a2, a2, PHY_CTRL_1_6_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t2, PHY_CTRL_1_6_OFFSET
    or      a1, a1, a2
    sd      a1, PHY_CTRL_1_6_ADDR(t7)

#ifdef  ADJUST_CPU_ODT
    ld      a1, PHY_CTRL_1_6_ADDR(t7)
    dli     a2, PHY_CTRL_1_RD_ODT_MASK
    dsll    a2, a2, PHY_CTRL_1_6_OFFSET + PHY_CTRL_1_RD_ODT_SHIFT
    not     a2, a2
    and     a1, a1, a2
    dli     a3, CPU_ODT_BASE_VALUE
    beqz    t2, 1f
    nop
    daddu   a3, a3, CPU_ODT_INC_VALUE
1:
    dsll    a2, a3, PHY_CTRL_1_6_OFFSET + PHY_CTRL_1_RD_ODT_SHIFT
    or      a1, a1, a2
    sd      a1, PHY_CTRL_1_6_ADDR(t7)
#endif

    dli     a1, 0xf
    bne     s3, a1, 88f
    nop
77:
    ld      a1, PHY_CTRL_1_7_ADDR(t7)
    dli     a2, PHY_CTRL_1_GATE_MASK
    dsll    a2, a2, PHY_CTRL_1_7_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t2, PHY_CTRL_1_7_OFFSET
    or      a1, a1, a2
    sd      a1, PHY_CTRL_1_7_ADDR(t7)

#ifdef  ADJUST_CPU_ODT
    ld      a1, PHY_CTRL_1_7_ADDR(t7)
    dli     a2, PHY_CTRL_1_RD_ODT_MASK
    dsll    a2, a2, PHY_CTRL_1_7_OFFSET + PHY_CTRL_1_RD_ODT_SHIFT
    not     a2, a2
    and     a1, a1, a2
    dli     a3, CPU_ODT_BASE_VALUE
    beqz    t2, 1f
    nop
    daddu   a3, a3, CPU_ODT_INC_VALUE
1:
    dsll    a2, a3, PHY_CTRL_1_7_OFFSET + PHY_CTRL_1_RD_ODT_SHIFT
    or      a1, a1, a2
    sd      a1, PHY_CTRL_1_7_ADDR(t7)
#endif

    b       88f
    nop

88:
    sync
#endif

    //enable Gather FIFO
    ld      a1, PHY_CTRL_2_ADDR(t7)
    dli     a2, 0x1
    dsll    a2, a2, PHY_CTRL_2_OFFSET + RESET_GFIFO_SHIFT
    not     a2, a2
    and     a1, a1, a2
    sd      a1, PHY_CTRL_2_ADDR(t7)

#if 0
    PRINTSTR("\r\nBefore set param_start to 1, INT_STATUS =0x")
    ld      a0, MC_INT_STATUS_ADDR(t7)
    bal     hexserial
    nop
    PRINTSTR("\r\nDLLLOCKREG =0x")
    ld      a0, DLLLOCKREG_ADDR(t7)
    bal     hexserial
    nop
#endif
    //set start to 1
    //PRINTSTR("\r\nSet param_start 1.")
    dli     a2, 0x1
    dsll    a2, a2, START_OFFSET
    ld      a1, START_ADDR(t7)
    or      a1, a1, a2
    sd      a1, START_ADDR(t7)
    sync

#if 0
    PRINTSTR("\r\nAfter set param_start to 1, INT_STATUS =0x")
    ld      a0, MC_INT_STATUS_ADDR(t7)
    bal     hexserial
    nop
    PRINTSTR("\r\nDLLLOCKREG =0x")
    ld      a0, DLLLOCKREG_ADDR(t7)
    bal     hexserial
    nop
#endif

    //poll until DLL locked.
    dli     a2, 0x1
1:
    ld      a1, DLLLOCKREG_ADDR(t7)
    and     a1, a1, a2
    beqz    a1, 1b
    nop

    //resync DLL
    //delay some time
    dli     a2, 0x400
1:
    daddiu  a2, a2, -1
    bnez    a2, 1b
    nop
    //PRINTSTR("\r\nResync DLL.")
    dli     a2, 0x1
    dsll    a2, a2, MC_RESYNC_DLL_OFFSET
    ld      a1, MC_RESYNC_DLL_ADDR(t7)
    or      a1, a1, a2
    sd      a1, MC_RESYNC_DLL_ADDR(t7)
    sync

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
    
    //PRINTSTR("\r\nDisable DDR MC config space.\r\n")
    dli     t7, CHIP_CONFIG_BASE_ADDR
    GET_ARB_LEVEL_NODE_ID
#ifdef LS3B
    and     a1, a1, 0xe
#endif
    dsll    a1, a1, 44
    or      t7, t7, a1
    li      a2, 0x1
    sll     a2, a2, DDR_CONFIG_DISABLE_OFFSET
#ifdef LS3B
    //ODD NODE sll 5
    GET_ARB_LEVEL_NODE_ID
    and     a1, a1, 0x1
    beqz    a1, 1f
    nop
    sll     a2, a2, 5
1:
#endif
    lw      a1, 0x0(t7)
    or      a1, a1, a2
    sw      a1, 0x0(t7)
    sync

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
    sync
#endif

#ifdef  ADD_DELAY_AFTER_RESET_PHY
    //this delay can't be removed. wired!
    //delay some time, how long is proper?
    dli     a2, MC_RST_DELAY
1:
    daddiu  a2, a2, -1
    bnez    a2, 1b
    nop
#endif
    
    move    ra, t1
    jr      ra
    nop

arb_modify_pop_delay_alter:
/*******************
function:
    alter specified byte slice phy_ctrl_0 pop delay setting(3 or 4)
input param:
    s3: 0 ~ 7---byte slice to be changed
        0xf ----change all 8 slices
use register:
    t1: save ra
*******************/
    move    t1, ra

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
    sync
#endif

    //PRINTSTR("\r\nEnable DDR MC config space.")
    dli     t7, CHIP_CONFIG_BASE_ADDR
    GET_ARB_LEVEL_NODE_ID
#ifdef LS3B
    and     a1, a1, 0xe
#endif
    dsll    a1, a1, 44
    or      t7, t7, a1
    li      a2, 0x1
    sll     a2, a2, DDR_CONFIG_DISABLE_OFFSET
#ifdef LS3B
    //ODD NODE sll 5
    GET_ARB_LEVEL_NODE_ID
    and     a1, a1, 0x1
    beqz    a1, 1f
    nop
    sll     a2, a2, 5
1:
#endif
    not     a2, a2
    lw      a1, 0x0(t7)
    and     a1, a1, a2
    sw      a1, 0x0(t7)
    sync
    
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
    //PRINTSTR("\r\nChange param value.")
    dli     a1, 0x0
    beq     s3, a1, 20f
    nop
    daddiu  a1, a1, 0x1
    beq     s3, a1, 21f
    nop
    daddiu  a1, a1, 0x1
    beq     s3, a1, 22f
    nop
    daddiu  a1, a1, 0x1
    beq     s3, a1, 23f
    nop
    daddiu  a1, a1, 0x1
    beq     s3, a1, 24f
    nop
    daddiu  a1, a1, 0x1
    beq     s3, a1, 25f
    nop
    daddiu  a1, a1, 0x1
    beq     s3, a1, 26f
    nop
    daddiu  a1, a1, 0x1
    beq     s3, a1, 27f
    nop
    dli     a1, 0xf
    bne     s3, a1, 88f
    nop

20:
    //get old value and reverse it
    ld      a1, PHY_CTRL_0_0_ADDR(t7)
    dsrl    a1, a1, PHY_CTRL_0_0_OFFSET + PHY_CTRL_0_POP_DELAY_SHIFT
    dli     a2, PHY_CTRL_0_POP_DELAY_MASK
    and     a1, a1, a2
    dli     a0, 0x4
    blt     a1, a0, 1f
    nop
    daddu   a0, a0, -1
1:
    ld      a1, PHY_CTRL_0_0_ADDR(t7)
    dli     a2, PHY_CTRL_0_POP_DELAY_MASK
    dsll    a2, a2, PHY_CTRL_0_0_OFFSET + PHY_CTRL_0_POP_DELAY_SHIFT
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, a0, PHY_CTRL_0_0_OFFSET + PHY_CTRL_0_POP_DELAY_SHIFT
    or      a1, a1, a2
    sd      a1, PHY_CTRL_0_0_ADDR(t7)

    dli     a1, 0xf
    bne     s3, a1, 88f
    nop
21:
    //get old value and reverse it
    ld      a1, PHY_CTRL_0_1_ADDR(t7)
    dsrl    a1, a1, PHY_CTRL_0_1_OFFSET + PHY_CTRL_0_POP_DELAY_SHIFT
    dli     a2, PHY_CTRL_0_POP_DELAY_MASK
    and     a1, a1, a2
    dli     a0, 0x4
    blt     a1, a0, 1f
    nop
    daddu   a0, a0, -1
1:
    ld      a1, PHY_CTRL_0_1_ADDR(t7)
    dli     a2, PHY_CTRL_0_POP_DELAY_MASK
    dsll    a2, a2, PHY_CTRL_0_1_OFFSET + PHY_CTRL_0_POP_DELAY_SHIFT
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, a0, PHY_CTRL_0_1_OFFSET + PHY_CTRL_0_POP_DELAY_SHIFT
    or      a1, a1, a2
    sd      a1, PHY_CTRL_0_1_ADDR(t7)

    dli     a1, 0xf
    bne     s3, a1, 88f
    nop
22:
    //get old value and reverse it
    ld      a1, PHY_CTRL_0_2_ADDR(t7)
    dsrl    a1, a1, PHY_CTRL_0_2_OFFSET + PHY_CTRL_0_POP_DELAY_SHIFT
    dli     a2, PHY_CTRL_0_POP_DELAY_MASK
    and     a1, a1, a2
    dli     a0, 0x4
    blt     a1, a0, 1f
    nop
    daddu   a0, a0, -1
1:
    ld      a1, PHY_CTRL_0_2_ADDR(t7)
    dli     a2, PHY_CTRL_0_POP_DELAY_MASK
    dsll    a2, a2, PHY_CTRL_0_2_OFFSET + PHY_CTRL_0_POP_DELAY_SHIFT
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, a0, PHY_CTRL_0_2_OFFSET + PHY_CTRL_0_POP_DELAY_SHIFT
    or      a1, a1, a2
    sd      a1, PHY_CTRL_0_2_ADDR(t7)

    dli     a1, 0xf
    bne     s3, a1, 88f
    nop
23:
    //get old value and reverse it
    ld      a1, PHY_CTRL_0_3_ADDR(t7)
    dsrl    a1, a1, PHY_CTRL_0_3_OFFSET + PHY_CTRL_0_POP_DELAY_SHIFT
    dli     a2, PHY_CTRL_0_POP_DELAY_MASK
    and     a1, a1, a2
    dli     a0, 0x4
    blt     a1, a0, 1f
    nop
    daddu   a0, a0, -1
1:
    ld      a1, PHY_CTRL_0_3_ADDR(t7)
    dli     a2, PHY_CTRL_0_POP_DELAY_MASK
    dsll    a2, a2, PHY_CTRL_0_3_OFFSET + PHY_CTRL_0_POP_DELAY_SHIFT
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, a0, PHY_CTRL_0_3_OFFSET + PHY_CTRL_0_POP_DELAY_SHIFT
    or      a1, a1, a2
    sd      a1, PHY_CTRL_0_3_ADDR(t7)

    dli     a1, 0xf
    bne     s3, a1, 88f
    nop
24:
    //get old value and reverse it
    ld      a1, PHY_CTRL_0_4_ADDR(t7)
    dsrl    a1, a1, PHY_CTRL_0_4_OFFSET + PHY_CTRL_0_POP_DELAY_SHIFT
    dli     a2, PHY_CTRL_0_POP_DELAY_MASK
    and     a1, a1, a2
    dli     a0, 0x4
    blt     a1, a0, 1f
    nop
    daddu   a0, a0, -1
1:
    ld      a1, PHY_CTRL_0_4_ADDR(t7)
    dli     a2, PHY_CTRL_0_POP_DELAY_MASK
    dsll    a2, a2, PHY_CTRL_0_4_OFFSET + PHY_CTRL_0_POP_DELAY_SHIFT
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, a0, PHY_CTRL_0_4_OFFSET + PHY_CTRL_0_POP_DELAY_SHIFT
    or      a1, a1, a2
    sd      a1, PHY_CTRL_0_4_ADDR(t7)

    dli     a1, 0xf
    bne     s3, a1, 88f
    nop
25:
    //get old value and reverse it
    ld      a1, PHY_CTRL_0_5_ADDR(t7)
    dsrl    a1, a1, PHY_CTRL_0_5_OFFSET + PHY_CTRL_0_POP_DELAY_SHIFT
    dli     a2, PHY_CTRL_0_POP_DELAY_MASK
    and     a1, a1, a2
    dli     a0, 0x4
    blt     a1, a0, 1f
    nop
    daddu   a0, a0, -1
1:
    ld      a1, PHY_CTRL_0_5_ADDR(t7)
    dli     a2, PHY_CTRL_0_POP_DELAY_MASK
    dsll    a2, a2, PHY_CTRL_0_5_OFFSET + PHY_CTRL_0_POP_DELAY_SHIFT
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, a0, PHY_CTRL_0_5_OFFSET + PHY_CTRL_0_POP_DELAY_SHIFT
    or      a1, a1, a2
    sd      a1, PHY_CTRL_0_5_ADDR(t7)

    dli     a1, 0xf
    bne     s3, a1, 88f
    nop
26:
    //get old value and reverse it
    ld      a1, PHY_CTRL_0_6_ADDR(t7)
    dsrl    a1, a1, PHY_CTRL_0_6_OFFSET + PHY_CTRL_0_POP_DELAY_SHIFT
    dli     a2, PHY_CTRL_0_POP_DELAY_MASK
    and     a1, a1, a2
    dli     a0, 0x4
    blt     a1, a0, 1f
    nop
    daddu   a0, a0, -1
1:
    ld      a1, PHY_CTRL_0_6_ADDR(t7)
    dli     a2, PHY_CTRL_0_POP_DELAY_MASK
    dsll    a2, a2, PHY_CTRL_0_6_OFFSET + PHY_CTRL_0_POP_DELAY_SHIFT
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, a0, PHY_CTRL_0_6_OFFSET + PHY_CTRL_0_POP_DELAY_SHIFT
    or      a1, a1, a2
    sd      a1, PHY_CTRL_0_6_ADDR(t7)

    dli     a1, 0xf
    bne     s3, a1, 88f
    nop
27:
    //get old value and reverse it
    ld      a1, PHY_CTRL_0_7_ADDR(t7)
    dsrl    a1, a1, PHY_CTRL_0_7_OFFSET + PHY_CTRL_0_POP_DELAY_SHIFT
    dli     a2, PHY_CTRL_0_POP_DELAY_MASK
    and     a1, a1, a2
    dli     a0, 0x4
    blt     a1, a0, 1f
    nop
    daddu   a0, a0, -1
1:
    ld      a1, PHY_CTRL_0_7_ADDR(t7)
    dli     a2, PHY_CTRL_0_POP_DELAY_MASK
    dsll    a2, a2, PHY_CTRL_0_7_OFFSET + PHY_CTRL_0_POP_DELAY_SHIFT
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, a0, PHY_CTRL_0_7_OFFSET + PHY_CTRL_0_POP_DELAY_SHIFT
    or      a1, a1, a2
    sd      a1, PHY_CTRL_0_7_ADDR(t7)

    b       88f
    nop

88:
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
    //PRINTSTR("\r\nSet param_start 1.")
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

    //resync DLL
    //delay some time
    dli     a2, 0x400
1:
    daddiu  a2, a2, -1
    bnez    a2, 1b
    nop
    //PRINTSTR("\r\nResync DLL.")
    dli     a2, 0x1
    dsll    a2, a2, MC_RESYNC_DLL_OFFSET
    ld      a1, MC_RESYNC_DLL_ADDR(t7)
    or      a1, a1, a2
    sd      a1, MC_RESYNC_DLL_ADDR(t7)
    sync

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
    
    //PRINTSTR("\r\nDisable DDR MC config space.\r\n")
    dli     t7, CHIP_CONFIG_BASE_ADDR
    GET_ARB_LEVEL_NODE_ID
#ifdef LS3B
    and     a1, a1, 0xe
#endif
    dsll    a1, a1, 44
    or      t7, t7, a1
    li      a2, 0x1
    sll     a2, a2, DDR_CONFIG_DISABLE_OFFSET
#ifdef LS3B
    //ODD NODE sll 5
    GET_ARB_LEVEL_NODE_ID
    and     a1, a1, 0x1
    beqz    a1, 1f
    nop
    sll     a2, a2, 5
1:
#endif
    lw      a1, 0x0(t7)
    or      a1, a1, a2
    sw      a1, 0x0(t7)
    sync

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
    sync
#endif

#ifdef  ADD_DELAY_AFTER_RESET_PHY
    //this delay can't be removed. wired!
    //delay some time, how long is proper?
    dli     a2, MC_RST_DELAY
1:
    daddiu  a2, a2, -1
    bnez    a2, 1b
    nop
#endif
    
    move    ra, t1
    jr      ra
    nop

#ifdef  MODIFY_PAD_COMP
arb_modify_pad_comp:
/*******************
function:
    when arb level failed, try to modify the pad compensation to make it work.
    Now we only test 3 value: Max, Min, Mid. And the trial sequence is fixed as listed before.
    Read the current value, then decide the next, if current is not Mid, then return success(0),
    else return fail(1).
output param:
    v0: 0 --- success
        1 --- fail
use register:
    t1: save ra
    t6: set return value
*******************/
    move    t1, ra

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
    sync
#endif

    //PRINTSTR("\r\nEnable DDR MC config space.")
    dli     t7, CHIP_CONFIG_BASE_ADDR
    GET_ARB_LEVEL_NODE_ID
#ifdef LS3B
    and     a1, a1, 0xe
#endif
    dsll    a1, a1, 44
    or      t7, t7, a1
    li      a2, 0x1
    sll     a2, a2, DDR_CONFIG_DISABLE_OFFSET
#ifdef LS3B
    //ODD NODE sll 5
    GET_ARB_LEVEL_NODE_ID
    and     a1, a1, 0x1
    beqz    a1, 1f
    nop
    sll     a2, a2, 5
1:
#endif
    not     a2, a2
    lw      a1, 0x0(t7)
    and     a1, a1, a2
    sw      a1, 0x0(t7)
    sync
    
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
    //PRINTSTR("\r\nChange param value.")

    //get current value
    ld      a1, PAD_CTRL_REG_ADDR(t7)
    dsrl    a1, a1, PAD_CTRL_COMP_OFFSET
    dli     a2, PAD_CTRL_COMP_MASK
    and     a2, a1, a2
    and     a2, a2, 0xf
    beqz    a2, cur_pad_max
    nop
    dli     a0, 0xf
    beq     a2, a0, cur_pad_min
    nop
    //cur pad == mid, return fail
    dli     t6, 0x1
    b       88f
    nop
cur_pad_max:
    //set pad comp to min
    ld      a1, PAD_CTRL_REG_ADDR(t7)
    dli     a2, PAD_CTRL_COMP_MASK
    dsll    a2, a2, PAD_CTRL_COMP_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dli     a2, 0x0f
    dsll    a2, a2, PAD_CTRL_COMP_OFFSET
    or      a1, a1, a2
    sd      a1, PAD_CTRL_REG_ADDR(t7)

    move    t6, $0
    b       88f
    nop
cur_pad_min:
    //set pad comp to mid
    ld      a1, PAD_CTRL_REG_ADDR(t7)
    dli     a2, PAD_CTRL_COMP_MASK
    dsll    a2, a2, PAD_CTRL_COMP_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dli     a2, 0x1e
    dsll    a2, a2, PAD_CTRL_COMP_OFFSET
    or      a1, a1, a2
    sd      a1, PAD_CTRL_REG_ADDR(t7)

    move    t6, $0
    b       88f
    nop

88:
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
    //PRINTSTR("\r\nSet param_start 1.")
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

    //resync DLL
    //delay some time
    dli     a2, 0x400
1:
    daddiu  a2, a2, -1
    bnez    a2, 1b
    nop
    //PRINTSTR("\r\nResync DLL.")
    dli     a2, 0x1
    dsll    a2, a2, MC_RESYNC_DLL_OFFSET
    ld      a1, MC_RESYNC_DLL_ADDR(t7)
    or      a1, a1, a2
    sd      a1, MC_RESYNC_DLL_ADDR(t7)
    sync

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
    
    //PRINTSTR("\r\nDisable DDR MC config space.\r\n")
    dli     t7, CHIP_CONFIG_BASE_ADDR
    GET_ARB_LEVEL_NODE_ID
#ifdef LS3B
    and     a1, a1, 0xe
#endif
    dsll    a1, a1, 44
    or      t7, t7, a1
    li      a2, 0x1
    sll     a2, a2, DDR_CONFIG_DISABLE_OFFSET
#ifdef LS3B
    //ODD NODE sll 5
    GET_ARB_LEVEL_NODE_ID
    and     a1, a1, 0x1
    beqz    a1, 1f
    nop
    sll     a2, a2, 5
1:
#endif
    lw      a1, 0x0(t7)
    or      a1, a1, a2
    sw      a1, 0x0(t7)
    sync

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
    sync
#endif

#ifdef  ADD_DELAY_AFTER_RESET_PHY
    //this delay can't be removed. wired!
    //delay some time, how long is proper?
    dli     a2, MC_RST_DELAY
1:
    daddiu  a2, a2, -1
    bnez    a2, 1b
    nop
#endif
    
    move    v0, t6
    move    ra, t1
    jr      ra
    nop
#endif
