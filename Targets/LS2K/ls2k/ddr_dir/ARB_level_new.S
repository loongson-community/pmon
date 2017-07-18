/************************************
    Function:   Access Result Based Software leveling
    Author: Chen Xinke
    v3.2    write level trys all possible Read setting(pop delay * gate cfg), and one success
            means this value is ok, write level all byte slice at the same time.
    v3.1    Write level in byte slice order, and when the two read fit(not necessarily means
            there is no read error!), the write result is recorded.
    v3.0    Rewrite the training method
    v2.0    Multi Node support
    v1.x    Single Node mode
    v1.1    Raw code for LS3A3
    v0.2    used for test

note: don't use s0, because it will be use at some subroutine

change register: a0,a1,a2,v0,v1, t9

register usage:
s3: Level sequence pointer(slice select).
s4: rd pop delay control(local loop control).
s5: Leveled byte mask.
t0: RST store bit pointer during TM process;
    RST parse pointer during result process.
t1: stage global variable, also used by modify_param functions.
t2: currently used delay value.
t3, t4: variables
t5: delay value interval.
t6: param to Modify_param (object select).
t7: volatile value
t8: ARB_STORE_BASE
t9: save ra; (local used by wrlvl/rdlvl)
v0, v1: return value of arb_test_mem.
algrithm:

***************************************/
/********************************
********************************/
#include "ARB_level.h"

//#define DEBUG_ARB_LEVEL_WR
//#define DEBUG_ARB_LEVEL_WR_TM
//#define DEBUG_ARB_LEVEL_WR_CFG
//#define DEBUG_ARB_LEVEL_RD
//#define DEBUG_ARB_LEVEL_RD_TM
//#define DEBUG_ARB_LEVEL_RD_CFG

//#define CLOCK_LEVEL

//#define USE_WRLVL_CLK_OFFSET_VALUE

//#define ARB_SKIP_WRLVL
#define ADJUST_CPU_ODT  //work ok
#define ADD_DELAY_AFTER_RESET_PHY   //not need any more ?

//Don't change
#define MODIFY_PAD_COMP
#define CONTROL_L2XBAR_DDR_WINDOW  //work ok
#define USE_BIG_GF_POP_DELAY  //work ok
#define ARBLVL_PUT_DRAM_SREF
#ifdef  LS2HMC
//#define CLEAR_HALF_CLK_SHIFT    //for 2H3~1 don't define it
//#define USE_DEFAULT_RDLVL_DELAY
#else
#define CLEAR_HALF_CLK_SHIFT    //necessary
//#define USE_DEFAULT_RDLVL_DELAY
#endif

//#define ALIGN_GATE_DELAY  //obsolete because we use new method

ARB_level:
    move    t9, ra
ARB_start:
#ifdef  PRINT_MSG
    PRINTSTR("\r\nNODE ID:");
    GET_ARB_LEVEL_NODE_ID
    move    a0, a1
    bal     hexserial
    nop
#endif

/*
 *Lock Scache 9800?01000000000 ~ 9800?01000001000(4K)
 */
#ifdef  PRINT_MSG
    PRINTSTR("\r\nLock Scache Node x--9800?01000000000~4K...\r\n")
#endif
    dli     a2, LOCK_SCACHE_CONFIG_BASE_ADDR
#ifdef LS3B
    GET_ARB_LEVEL_NODE_ID
    dsll    a1, a1, 14
    daddu   a2, a2, a1
#endif
    GET_ARB_LEVEL_NODE_ID
    dsll    a1, a1, 44
    or      a2, a2, a1
    dli     a3, 0x0000fffffffff000  //set Mask first
    sd      a3, 0x40(a2)
    dli     a3, 0x8000001000000000
    or      a3, a3, a1
    sd      a3, 0x0(a2)
#ifdef  PRINT_MSG
    PRINTSTR("Lock Scache Done.\r\n")
#endif

//save t0~t9,s1~s7
    dli     a2, ARB_STACK_BASE
    GET_ARB_LEVEL_NODE_ID
    dsll    a1, a1, 44
    daddu   a2, a2, a1
    sd      s0, 0x0(a2)
    sd      s1, 0x8(a2)
    sd      s2, 0x10(a2)
    sd      s3, 0x18(a2)
    sd      s4, 0x20(a2)
    sd      s5, 0x28(a2)
    sd      s6, 0x30(a2)
    sd      s7, 0x38(a2)
    sd      t0, 0x40(a2)
    sd      t1, 0x48(a2)
    sd      t2, 0x50(a2)
    sd      t3, 0x58(a2)
    sd      t4, 0x60(a2)
    sd      t5, 0x68(a2)
    sd      t6, 0x70(a2)
    sd      t7, 0x78(a2)
    sd      t8, 0x80(a2)
    sd      t9, 0x88(a2)

#if 0
    bal     arb_test_mem
    nop
#endif
#ifdef  PRINT_MSG
    PRINTSTR("\r\nStart ARB Leveling....\r\n")
#endif

ARB_level_begin:

#ifndef ARB_SKIP_WRLVL

#ifdef  PRINT_MSG
    PRINTSTR("\r\nStart Write Leveling. Wait a while...")
#endif

//prepare for wrlvl
    bal     enable_ddr_confspace
    nop

    GET_ARB_LEVEL_NODE_ID
    dsll    a1, a1, 44
    dli     t8, ARB_STORE_BASE
    or      t8, t8, a1
    dli     t7, DDR_MC_CONFIG_BASE
    or      t7, t7, a1

    //Get CLKLVL_DELAY_VALUE
    ld      a1, CLKLVL_DELAY_0_ADDR(t7)
    dsrl    a2, a1, CLKLVL_DELAY_0_OFFSET
    and     a2, a2, 0x7f
    sd      a2, CLKLVL_DELAY_VALUE(t8)

    //get the current wrlvl_dq_dly value
    move    a3, $0
    dli     a2, WRLVL_DQ_DELAY_MASK

    ld      a1, WRLVL_DQ_DELAY_0_ADDR(t7)
    dsrl    a1, a1, WRLVL_DQ_DELAY_0_OFFSET
    and     a1, a1, a2
    dsll    a1, a1, 0
    or      a3, a1, a3

    ld      a1, WRLVL_DQ_DELAY_1_ADDR(t7)
    dsrl    a1, a1, WRLVL_DQ_DELAY_1_OFFSET
    and     a1, a1, a2
    dsll    a1, a1, 8
    or      a3, a1, a3

    ld      a1, WRLVL_DQ_DELAY_2_ADDR(t7)
    dsrl    a1, a1, WRLVL_DQ_DELAY_2_OFFSET
    and     a1, a1, a2
    dsll    a1, a1, 16
    or      a3, a1, a3

    ld      a1, WRLVL_DQ_DELAY_3_ADDR(t7)
    dsrl    a1, a1, WRLVL_DQ_DELAY_3_OFFSET
    and     a1, a1, a2
    dsll    a1, a1, 24
    or      a3, a1, a3

    ld      a1, WRLVL_DQ_DELAY_4_ADDR(t7)
    dsrl    a1, a1, WRLVL_DQ_DELAY_4_OFFSET
    and     a1, a1, a2
    dsll    a1, a1, 32
    or      a3, a1, a3

    ld      a1, WRLVL_DQ_DELAY_5_ADDR(t7)
    dsrl    a1, a1, WRLVL_DQ_DELAY_5_OFFSET
    and     a1, a1, a2
    dsll    a1, a1, 40
    or      a3, a1, a3

    ld      a1, WRLVL_DQ_DELAY_6_ADDR(t7)
    dsrl    a1, a1, WRLVL_DQ_DELAY_6_OFFSET
    and     a1, a1, a2
    dsll    a1, a1, 48
    or      a3, a1, a3

    ld      a1, WRLVL_DQ_DELAY_7_ADDR(t7)
    dsrl    a1, a1, WRLVL_DQ_DELAY_7_OFFSET
    and     a1, a1, a2
    dsll    a1, a1, 56
    or      a3, a1, a3

    sd      a3, WRLVL_DQ_VALUE_ADDR(t8)

    bal     disable_ddr_confspace
    nop

#ifdef  USE_WRLVL_CLK_OFFSET_VALUE
    b       arb_wrlvl_default_value
    nop
#endif

arb_wrlvl_start:
#ifdef  DEBUG_ARB_LEVEL_WR_CFG
    PRINTSTR("\r\nThe MC configuration is:\r\n")
    bal     enable_ddr_confspace
    nop

    dli     t1, DDR_PARAM_NUM
    dli     t7, DDR_MC_CONFIG_BASE
    GET_ARB_LEVEL_NODE_ID
    dsll    a1, a1, 44
    or      t7, t7, a1
1:
    ld      t8, 0x0(t7)
    dsrl    a0, t8, 32
    bal     hexserial
    nop
    PRINTSTR("  ")
    move    a0, t8
    bal     hexserial
    nop
    PRINTSTR("\r\n")

    daddiu  t1, t1, -1
    daddiu  t7, t7, 16
    bnez    t1, 1b
    nop

    bal     disable_ddr_confspace
    nop
#endif
    dli     s3, 0xf //change all 8 byte slices

#ifndef KEEP_GATE_INIT_DLY_FIXED
    //set rdlvl gate delay value
    dli     t6, 0x2
    dli     t2, RDLVL_GATE_INIT_DELAY
    bal     arb_modify_param
    nop
#endif
#ifndef KEEP_DQ_DLY_FIXED
    //small wrlvl dq delay value temporary
    dli     t6, 0x6
    dli     t2, WRLVL_DQ_SMALL_DLY
    bal     arb_modify_param
    nop
#endif

//1. level Write DQS Delay line setting.
    //clear store mem
    dli     t8, ARB_STORE_BASE
    GET_ARB_LEVEL_NODE_ID
    dsll    a1, a1, 44
    daddu   t8, t8, a1
    sd      $0, 0x0(t8)
    sd      $0, 0x8(t8)
    sd      $0, 0x10(t8)
    sd      $0, 0x18(t8)
    sd      $0, 0x20(t8)
    sd      $0, 0x28(t8)
    sd      $0, 0x30(t8)
    sd      $0, 0x38(t8)
    sd      $0, 0x40(t8)
    sd      $0, 0x48(t8)
    sd      $0, 0x50(t8)
    sd      $0, 0x58(t8)

    //set test interval
    dli     t5, 1 << LOG2_STEP
    //set t2 start value
    dli     t2, WRLVL_MAX_DELAY

    dli     t0, 0x1
    dsrl    a0, t2, LOG2_STEP
    dsll    t0, t0, a0

wrlvl_test_one_delay:
    move    t3, $0
    move    t4, $0
    not     t3, t3
    not     t4, t4

#ifdef  DEBUG_ARB_LEVEL_WR_TM
    PRINTSTR("\r\n\r\nt2 = 0x")
    move    a0, t2
    bal     hexserial
    nop
#endif
    move    s4, $0
    dli     t6, 0x5
//write new delay value
    bal     arb_modify_param
    nop
#ifdef  DEBUG_ARB_LEVEL_WR_CFG
    PRINTSTR("\r\nThe wrlvl_delay configuration is:\r\n")
    bal     enable_ddr_confspace
    nop

    dli     t1, 0x3         //set print num
    dli     t7, DDR_MC_CONFIG_BASE
    GET_ARB_LEVEL_NODE_ID
    dsll    a1, a1, 44
    or      t7, t7, a1
    daddu   t7, t7, 0xaf0   //set start offset
1:
    ld      t8, 0x0(t7)
    dsrl    a0, t8, 32
    bal     hexserial
    nop
    PRINTSTR("  ")
    move    a0, t8
    bal     hexserial
    nop
    PRINTSTR("\r\n")

    daddiu  t1, t1, -1
    daddiu  t7, t7, 16
    bnez    t1, 1b
    nop

    bal     disable_ddr_confspace
    nop
#endif

    //store t2 of current wrlvl_delay value
    move    s5, t2
17:
    move    t2, $0
18:
    dli     t6, 0x7
#ifdef  DEBUG_ARB_LEVEL_WR_CFG
    PRINTSTR("\r\nModify read gate cfg: = 0x")
    move    a0, t2
    bal     hexserial
    nop
#endif
    bal     arb_modify_param
    nop

//do Test and print test result
#ifdef  DEBUG_ARB_LEVEL_WR_CFG
    PRINTSTR("\r\nThe phy_ctrl_0 configuration is:\r\n")
    bal     enable_ddr_confspace
    nop

    dli     t1, 0xa         //set print num
    dli     t7, DDR_MC_CONFIG_BASE
    GET_ARB_LEVEL_NODE_ID
    dsll    a1, a1, 44
    or      t7, t7, a1
    daddu   t7, t7, 0x2d0   //set start offset
1:
    ld      t8, 0x0(t7)
    dsrl    a0, t8, 32
    bal     hexserial
    nop
    PRINTSTR("  ")
    move    a0, t8
    bal     hexserial
    nop
    PRINTSTR("\r\n")

    daddiu  t1, t1, -1
    daddiu  t7, t7, 16
    bnez    t1, 1b
    nop

    bal     disable_ddr_confspace
    nop
#endif
    bal     arb_test_mem
    nop
#ifdef  DEBUG_ARB_LEVEL_WR_TM
    move    t7, v0
    move    t8, v1

    PRINTSTR("\r\nRW Diff 0x")
    dsrl    a0, t7, 32
    bal     hexserial
    nop
    move    a0, t7
    bal     hexserial
    nop

    PRINTSTR("\r\nRD Diff 0x")
    dsrl    a0, t8, 32
    bal     hexserial
    nop
    move    a0, t8
    bal     hexserial
    nop

    move    v0, t7
    move    v1, t8
#endif
    //process test result, only when the entire byte is correct(0x00) clear fail mark in t3.
    //byte 7
    dsrl    a0, v0, 0x38
    and     a0, a0, 0xff
    bnez    a0, 1f
    nop
    //find a pass
    dli     a0, 0xff
    dsll    a0, a0, 0x38
    not     a0, a0
    and     t3, t3, a0
1:
    //byte 6
    dsrl    a0, v0, 0x30
    and     a0, a0, 0xff
    bnez    a0, 1f
    nop
    //find a pass
    dli     a0, 0xff
    dsll    a0, a0, 0x30
    not     a0, a0
    and     t3, t3, a0
1:
    //byte 5
    dsrl    a0, v0, 0x28
    and     a0, a0, 0xff
    bnez    a0, 1f
    nop
    //find a pass
    dli     a0, 0xff
    dsll    a0, a0, 0x28
    not     a0, a0
    and     t3, t3, a0
1:
    //byte 4
    dsrl    a0, v0, 0x20
    and     a0, a0, 0xff
    bnez    a0, 1f
    nop
    //find a pass
    dli     a0, 0xff
    dsll    a0, a0, 0x20
    not     a0, a0
    and     t3, t3, a0
1:
    //byte 3
    dsrl    a0, v0, 0x18
    and     a0, a0, 0xff
    bnez    a0, 1f
    nop
    //find a pass
    dli     a0, 0xff
    dsll    a0, a0, 0x18
    not     a0, a0
    and     t3, t3, a0
1:
    //byte 2
    dsrl    a0, v0, 0x10
    and     a0, a0, 0xff
    bnez    a0, 1f
    nop
    //find a pass
    dli     a0, 0xff
    dsll    a0, a0, 0x10
    not     a0, a0
    and     t3, t3, a0
1:
    //byte 1
    dsrl    a0, v0, 0x08
    and     a0, a0, 0xff
    bnez    a0, 1f
    nop
    //find a pass
    dli     a0, 0xff
    dsll    a0, a0, 0x08
    not     a0, a0
    and     t3, t3, a0
1:
    //byte 0
    dsrl    a0, v0, 0x0
    and     a0, a0, 0xff
    bnez    a0, 1f
    nop
    //find a pass
    dli     a0, 0xff
    dsll    a0, a0, 0x0
    not     a0, a0
    and     t3, t3, a0
1:
#ifdef  DEBUG_ARB_LEVEL_WR_TM
    //record RD history in t4, normally, t4 should be 0x0 after all these trying
    //byte 7
    dsrl    a0, v1, 0x38
    and     a0, a0, 0xff
    bnez    a0, 1f
    nop
    //find a pass
    dli     a0, 0xff
    dsll    a0, a0, 0x38
    not     a0, a0
    and     t4, t4, a0
1:
    //byte 6
    dsrl    a0, v1, 0x30
    and     a0, a0, 0xff
    bnez    a0, 1f
    nop
    //find a pass
    dli     a0, 0xff
    dsll    a0, a0, 0x30
    not     a0, a0
    and     t4, t4, a0
1:
    //byte 5
    dsrl    a0, v1, 0x28
    and     a0, a0, 0xff
    bnez    a0, 1f
    nop
    //find a pass
    dli     a0, 0xff
    dsll    a0, a0, 0x28
    not     a0, a0
    and     t4, t4, a0
1:
    //byte 4
    dsrl    a0, v1, 0x20
    and     a0, a0, 0xff
    bnez    a0, 1f
    nop
    //find a pass
    dli     a0, 0xff
    dsll    a0, a0, 0x20
    not     a0, a0
    and     t4, t4, a0
1:
    //byte 3
    dsrl    a0, v1, 0x18
    and     a0, a0, 0xff
    bnez    a0, 1f
    nop
    //find a pass
    dli     a0, 0xff
    dsll    a0, a0, 0x18
    not     a0, a0
    and     t4, t4, a0
1:
    //byte 2
    dsrl    a0, v1, 0x10
    and     a0, a0, 0xff
    bnez    a0, 1f
    nop
    //find a pass
    dli     a0, 0xff
    dsll    a0, a0, 0x10
    not     a0, a0
    and     t4, t4, a0
1:
    //byte 1
    dsrl    a0, v1, 0x08
    and     a0, a0, 0xff
    bnez    a0, 1f
    nop
    //find a pass
    dli     a0, 0xff
    dsll    a0, a0, 0x08
    not     a0, a0
    and     t4, t4, a0
1:
    //byte 0
    dsrl    a0, v1, 0x0
    and     a0, a0, 0xff
    bnez    a0, 1f
    nop
    //find a pass
    dli     a0, 0xff
    dsll    a0, a0, 0x0
    not     a0, a0
    and     t4, t4, a0
1:
#endif

    //tryed all setting(0,1,2)
    dli     a1, 0x2
    beq     t2, a1, 22f
    nop
    dsll    t2, t2, 0x1
    bnez    t2, 1f
    nop
    xor     t2, t2, 0x1
1:
    b       18b
    nop
22: //test another rd pop delay cfg
    bnez    s4, 26f
    nop
    daddu   s4, s4, 1
#ifdef  DEBUG_ARB_LEVEL_WR_CFG
    PRINTSTR("\r\nAlter read pop delay.")
#endif
    bal     arb_modify_pop_delay_alter
    nop

    b       17b
    nop

26: //tryed all setting(combination: pop delay and rd gat cfg)
    move    t2, s5

#ifdef  DEBUG_ARB_LEVEL_WR_TM
    PRINTSTR("\r\nAfter tried all the RD setting:\r\nRW Diff 0x")
    dsrl    a0, t3, 32
    bal     hexserial
    nop
    move    a0, t3
    bal     hexserial
    nop

    PRINTSTR("\r\nRD Diff 0x")
    dsrl    a0, t4, 32
    bal     hexserial
    nop
    move    a0, t4
    bal     hexserial
    nop
    beqz    t4, 1f
    nop
    PRINTSTR("\r\nWarning!!! some byte slice can't find a correct read setting")
1:
#endif

//process TM result: translate Byte error info into 1 bit info in each BX_TM_RST of every Byte.
//64 bit BX_TM_RST work as a bit map corresponding to every param value(so the min step interval
//is 2, or there will be not enough space to store TM RST info), the 64 bit can be only part valid(
//step interval > 2).
    dli     t8, ARB_STORE_BASE
    GET_ARB_LEVEL_NODE_ID
    dsll    a1, a1, 44
    daddu   t8, t8, a1

    dsrl    t7, t3, 56
    and     t7, t7, 0xff
    beqz    t7, 1f
    nop
    //error detected
    ld      a0, B7_TM_RST(t8)
    or      a0, a0, t0
    sd      a0, B7_TM_RST(t8)
1:
    dsrl    t7, t3, 48
    and     t7, t7, 0xff
    beqz    t7, 1f
    nop
    //error detected
    ld      a0, B6_TM_RST(t8)
    or      a0, a0, t0
    sd      a0, B6_TM_RST(t8)
1:
    dsrl    t7, t3, 40
    and     t7, t7, 0xff
    beqz    t7, 1f
    nop
    //error detected
    ld      a0, B5_TM_RST(t8)
    or      a0, a0, t0
    sd      a0, B5_TM_RST(t8)
1:
    dsrl    t7, t3, 32
    and     t7, t7, 0xff
    beqz    t7, 1f
    nop
    //error detected
    ld      a0, B4_TM_RST(t8)
    or      a0, a0, t0
    sd      a0, B4_TM_RST(t8)
1:
    dsrl    t7, t3, 24
    and     t7, t7, 0xff
    beqz    t7, 1f
    nop
    //error detected
    ld      a0, B3_TM_RST(t8)
    or      a0, a0, t0
    sd      a0, B3_TM_RST(t8)
1:
    dsrl    t7, t3, 16
    and     t7, t7, 0xff
    beqz    t7, 1f
    nop
    //error detected
    ld      a0, B2_TM_RST(t8)
    or      a0, a0, t0
    sd      a0, B2_TM_RST(t8)
1:
    dsrl    t7, t3, 8
    and     t7, t7, 0xff
    beqz    t7, 1f
    nop
    //error detected
    ld      a0, B1_TM_RST(t8)
    or      a0, a0, t0
    sd      a0, B1_TM_RST(t8)
1:
    dsrl    t7, t3, 0
    and     t7, t7, 0xff
    beqz    t7, 1f
    nop
    //error detected
    ld      a0, B0_TM_RST(t8)
    or      a0, a0, t0
    sd      a0, B0_TM_RST(t8)
1:
    dsrl    t0, t0, 1

    //check wether delay value exceed max value
    dsubu   a2, t2, t5
    bltz    a2, 11f //check the new delay value whether exceed limitation
    nop
    /** not exceed **/
    move    t2, a2
    b       wrlvl_test_one_delay
    nop
11:

#ifdef  DEBUG_ARB_LEVEL_WR
    dli     t8, ARB_STORE_BASE
    GET_ARB_LEVEL_NODE_ID
    dsll    a1, a1, 44
    daddu   t8, t8, a1
    PRINTSTR("\r\nlevel result is:\r\n")
    ld      t7, B7_TM_RST(t8)
    dsrl    a0, t7, 32
    bal     hexserial
    nop
    move    a0, t7
    bal     hexserial
    nop
    PRINTSTR("\r\n")
    ld      t7, B6_TM_RST(t8)
    dsrl    a0, t7, 32
    bal     hexserial
    nop
    move    a0, t7
    bal     hexserial
    nop
    PRINTSTR("\r\n")
    ld      t7, B5_TM_RST(t8)
    dsrl    a0, t7, 32
    bal     hexserial
    nop
    move    a0, t7
    bal     hexserial
    nop
    PRINTSTR("\r\n")
    ld      t7, B4_TM_RST(t8)
    dsrl    a0, t7, 32
    bal     hexserial
    nop
    move    a0, t7
    bal     hexserial
    nop
    PRINTSTR("\r\n")
    ld      t7, B3_TM_RST(t8)
    dsrl    a0, t7, 32
    bal     hexserial
    nop
    move    a0, t7
    bal     hexserial
    nop
    PRINTSTR("\r\n")
    ld      t7, B2_TM_RST(t8)
    dsrl    a0, t7, 32
    bal     hexserial
    nop
    move    a0, t7
    bal     hexserial
    nop
    PRINTSTR("\r\n")
    ld      t7, B1_TM_RST(t8)
    dsrl    a0, t7, 32
    bal     hexserial
    nop
    move    a0, t7
    bal     hexserial
    nop
    PRINTSTR("\r\n")
    ld      t7, B0_TM_RST(t8)
    dsrl    a0, t7, 32
    bal     hexserial
    nop
    move    a0, t7
    bal     hexserial
    nop
#endif
//calculate mid value for each byte lane
/***********
boundary sign: contain at least WINDOW_ZERO_NUM consecutive 0(TM success)
    t0: parse pointer
    t1: BYTE OFFSET, and work as loop control
    t2: parse max position
    t3: BYTE_X_LEVEL_RST
    t4: WINDOW_ZERO_NUM
***********/
    //set t2 max value for each level object
    dli     t2, WRLVL_MAX_DELAY
    dsrl    t2, t2, LOG2_STEP

    move    t9, $0
    dli     t8, ARB_STORE_BASE
    GET_ARB_LEVEL_NODE_ID
    dsll    a1, a1, 44
    daddu   t8, t8, a1

    dli     t1, 0x38    //start from byte 7
    GET_DIMM_WIDTH
    beqz    a1, 1f
    nop
    //for 32 bit data width, add high 32 bits result to low 32 bits
    ld      a1, B4_TM_RST(t8)
    ld      a2, B0_TM_RST(t8)
    or      a1, a1, a2
    sd      a1, B0_TM_RST(t8)

    ld      a1, B5_TM_RST(t8)
    ld      a2, B1_TM_RST(t8)
    or      a1, a1, a2
    sd      a1, B1_TM_RST(t8)

    ld      a1, B6_TM_RST(t8)
    ld      a2, B2_TM_RST(t8)
    or      a1, a1, a2
    sd      a1, B2_TM_RST(t8)

    ld      a1, B7_TM_RST(t8)
    ld      a2, B3_TM_RST(t8)
    or      a1, a1, a2
    sd      a1, B3_TM_RST(t8)

    dli     t1, 0x18    //start from byte 3
1:

11: //loop for all byte lanes
    daddu   t7, t8, t1
    ld      t3, B0_TM_RST(t7)
#ifdef  DEBUG_ARB_LEVEL_WR
    PRINTSTR("\r\nt3 = 0x")
    dsrl    a0, t3, 32
    bal     hexserial
    nop
    move    a0, t3
    bal     hexserial
    nop
#endif
    move    t0, $0
    dli     t4, WINDOW_ZERO_NUM
12:
    bgtu    t0, t2, 3f
    nop
    dsrl    t7, t3, t0
    and     t7, t7, 0x1
    bnez    t7, 1f
    nop
    //find a TM success
    daddiu  t4, t4, -1
    beqz    t4, 2f
    nop
    //continue move
    daddiu  t0, t0, 0x1
    b       12b
    nop
1:  //find a TM fail
    dli     t4, WINDOW_ZERO_NUM
    //continue move
    daddiu  t0, t0, 0x1
    b       12b
    nop
2:  //window found
    //calculate the MIN boundary
    dli     a1, WINDOW_ZERO_NUM
    daddiu  a0, t0, 1
    dsubu   a0, a0, a1
    dsll    a0, a0, LOG2_STEP   //a0 = a0 * 2n
    and     a0, a0, 0xff
    dsll    a0, a0, t1
    ld      a1, GD_MIN(t8)
    or      a1, a1, a0
    sd      a1, GD_MIN(t8)
    //move forward to the next Fail to cal the MAX boundary
1:
    daddiu  t0, t0, 0x1
    bgtu    t0, t2, 1f
    nop
    dsrl    t7, t3, t0
    and     t7, t7, 0x1
    beqz    t7, 1b  //continue move
    nop
1:
    //find a TM FAIL or reach the max test value
    daddiu  a0, t0, -1
    dsll    a0, a0, LOG2_STEP   //a0 = a0 * 2n
    and     a0, a0, 0xff
    dsll    a0, a0, t1
    ld      a1, GD_MAX(t8)
    or      a1, a1, a0
    sd      a1, GD_MAX(t8)
    b       2f
    nop
3:  //parse to end, CAN NOT find a window
    or      t9, t9, 0x1
#ifdef  PRINT_MSG
    PRINTSTR("\r\nWrlvl Error: This Byte Window not found.")
    PRINTSTR("\r\nFailed byte is byte: ")
    dsrl    a0, t1, 3
    bal     hexserial
    nop
#endif
2:  //continue for next byte lane
    beqz    t1, 2f
    nop
    daddu   t1, t1, -0x8
    b       11b
    nop
2:  //All byte lane's MIN and MAX value stored or fail to find
    beqz    t9, 1f
    nop
    //some Byte lane can not find a window
#ifdef  MODIFY_PAD_COMP
    PRINTSTR("\r\nTry another pad compensation.\r\n")
    bal     arb_modify_pad_comp
    nop
    beqz    v0, arb_wrlvl_start
    nop
#endif
#ifdef  PRINT_MSG
    PRINTSTR("\r\nWrite level failed. Write default value.\r\n")
#endif
    //write standard value mandatory
    //wrlvl_delay
    dli     a0, WRLVL_DEFAULT_VALUE
    sd      a0, GD_MID(t8)
    b       arb_wrlvl_value_caled
    nop
1:
#ifdef  PRINT_MSG
    PRINTSTR("\r\nMin value: 0x")
    ld      t7, GD_MIN(t8)
    dsrl    a0, t7, 32
    bal     hexserial
    nop
    move    a0, t7
    bal     hexserial
    nop
    PRINTSTR("\r\nMax value: 0x")
    ld      t7, GD_MAX(t8)
    dsrl    a0, t7, 32
    bal     hexserial
    nop
    move    a0, t7
    bal     hexserial
    nop
#endif

//calculate final value for each byte lane
#if 1   //def  DDR3_DIMM
    dli     a0, 0x2
    GET_SDRAM_TYPE
    beq     a0, a1, 2f
    nop
    GET_DIMM_TYPE
    bnez    a1, 4f
    nop
2:
    //DDR3 UDIMM/DDR2 DIMM
    //if we find the lower limit(MIN > 0), then we use (MIN + compensation_value) as the final result.
    //else, if we find the upper limit(MAX < param upper limit), then we use (MAX - compensation_value)
    //as the final result. else(MAX reach param upper limit), we use byte0 as reference, we
    //use (MAX_byte0 - compensation_value + manual_specific_offset).
    move    t4, $0
    dli     t7, 7   //slice control
arb_level_wrlvl_ddr3_cal_begin:
    dsll    a0, t7, 3

    ld      a1, GD_MIN(t8)
    ld      a3, GD_MAX(t8)
    dsrl    a1, a1, a0
    dsrl    a3, a3, a0
    and     a1, a1, 0x7f
    and     a3, a3, 0x7f

    beqz    a1, 2f
    nop
    //MIN > 0
    dli     v0, WRLVL_DELAY_LEVEL_UP_LIMIT
#ifndef KEEP_DQ_DLY_FIXED
    dli     v1, WRLVL_DQ_SMALL_DLY
#else
    ld      v1, WRLVL_DQ_VALUE_ADDR(t8)
    dsrl    v1, v1, a0
    and     v1, v1, 0x7f
#endif
    dsubu   v0, v0, v1
    bge     a3, v0, 3f
    nop
    //MAX < upper limit, use (MAX + MIN)/2
    daddu   a3, a3, a1
    dsrl    a3, a3, 1
    b       arb_level_ddr3_cal_end
    nop
3:
    //MAX >= upper limit, use min + defined offset
    dli     v0, WRLVL_DELAY_ADD_VALUE
    daddu   a3, a1, v0
    b       arb_level_ddr3_cal_end
    nop

2:  //MIN == 0
    dli     v0, WRLVL_DELAY_LEVEL_UP_LIMIT
#ifndef KEEP_DQ_DLY_FIXED
    dli     v1, WRLVL_DQ_SMALL_DLY
#else
    ld      v1, WRLVL_DQ_VALUE_ADDR(t8)
    dsrl    v1, v1, a0
    and     v1, v1, 0x7f
#endif
    dsubu   v0, v0, v1
    bge     a3, v0, 3f
    nop
    //MAX < upper limit, use MAX - defined offset
    dli     v0, WRLVL_DELAY_MINUS_VALUE
    bge     a3, v0, 1f
    nop
    move    v0, a3
1:
    dsubu   a3, a3, v0
    b       arb_level_ddr3_cal_end
    nop
3:
    //MAX >= upper limit
#ifdef  DDR3_DIMM
    //use byte 0 as reference
    ld      a1, GD_MAX(t8)
    and     a1, a1, 0x7f
    dli     v0, WRLVL_DELAY_MINUS_VALUE
    bge     a1, v0, 1f
    nop
    move    v0, a1
1:
    dsubu   a1, a1, v0
    dli     v0, WRLVL_DDR3_UDIMM_DEFAULT_OFFSET
    dsrl    v0, v0, a0
    and     v0, v0, 0x7f
    daddu   a3, a1, v0
#else
    //for DDR2, use default value
    dli     v0, WRLVL_DEFAULT_VALUE
    dsrl    v0, v0, a0
    and     a3, v0, 0x7f
#endif

arb_level_ddr3_cal_end:
    //check the calculated result whether exceed the max value
    //max value <= min(0x80-DQ_DLY, 0x60)
    dli     v0, WRLVL_DELAY_PARAM_UP_LIMIT
#ifndef KEEP_DQ_DLY_FIXED
#ifndef USE_SPECIAL_WRLVL_DQ_DELAY
    dli     v1, WRLVL_DQ_DEFAULT_DLY
#else
    dli     v1, WRLVL_DQ_SPECIAL_DLY
    dsrl    v1, v1, a0
    and     v1, v1, 0x7f
#endif
#else
    ld      v1, WRLVL_DQ_VALUE_ADDR(t8)
    dsrl    v1, v1, a0
    and     v1, v1, 0x7f
#endif
    dsubu   v0, v0, v1
    //get min(0x80-DQ_DLY, 0x60)
    dli     v1, WRLVL_3QUARTER_CLK_VALUE
    ble     v0, v1, 1f
    nop
    move    v0, v1
1:
    //get min(v0, a3)
    ble     a3, v0, 1f
    nop
    move    a3, v0
1:
#if 0
    //check whether the value is near 0x40
    dli     v0, 0x40
    ble     a3, v0, 1f
    nop
    //a3 > 0x40
    dsubu   v0, a3, v0
    dli     v1, 0x8
    bgt     v0, v1, 2f
    nop
    dli     a3, 0x48
2:  //abs(delta) > 0x8
    b       8f
    nop
1:  //a3 < 0x40
    dsubu   v0, v0, a3
    dli     v1, 0x8
    bgt     v0, v1, 2f
    nop
    dli     a3, 0x38
2:  //abs(delta) > 0x8
    b       8f
    nop
8:
#endif
    dsll    a3, a3, a0
    or      t4, t4, a3

    daddu   t7, t7, -1
    bge     t7, $0, arb_level_wrlvl_ddr3_cal_begin
    nop

    move    a0, t4
    b       88f
    nop

4:  //DDR3 RDIMM
    ld      a1, GD_MIN(t8)
    ld      a3, GD_MAX(t8)
    //select the MIN(byte 3, byte 4, byte 5), normally the byte 4 should be the smaller.
    //find the MIN of byte 3 and byte 4
    dsrl    a2, a3, 32
    dsrl    v0, a3, 24
    and     a2, a2, 0xff
    and     v0, v0, 0xff
    ble     a2, v0, 1f
    nop
    move    a2, v0
1:
    //find the MIN of a2 and byte 5
    dsrl    v0, a3, 40
    and     v0, v0, 0xff
    ble     a2, v0, 1f
    nop
    move    a2, v0
1:
    //select the MIN(byte 3,4,5 MIN, WRLVL_DELAY_MINUS_VALUE), normally the byte 4 MIN should be closer to 0x40
    dli     v0, WRLVL_DELAY_MINUS_VALUE
    ble     a2, v0, 1f
    nop
    move    a2, v0
1:
    and     a2, a2, 0x7f
    dsll    v0, a2, 8
    or      a2, a2, v0  //2
    dsll    v0, a2, 16
    or      a2, a2, v0  //4
    dsll    v0, a2, 32
    or      a2, a2, v0  //8

    dsubu   v0, a3, a2
    dli     v1, 0x8080808080808080
    and     v1, v1, v0
    bnez    v1, 1f
    nop
    move    a0, v0
    b       88f
    nop
1:  //abnormal: some byte is smaller than MIN(byte 3, 4)
    //use mid/2
    dsrl    a0, a0, 1
    dli     a1, 0x7f7f7f7f7f7f7f7f
    and     a0, a0, a1

88:
#else
#if 0
    //DDR2 DIMM
    ld      a1, GD_MIN(t8)
    ld      a3, GD_MAX(t8)
    //Refer to Byte 0
    dsrl    a2, a1, 0
    and     a2, a2, 0x7f
    beqz    a2, 2f
    nop
    //MIN != 0
    dsrl    a2, a3, 0
    and     a2, a2, 0x7f
    dli     v0, (WRLVL_MAX_DELAY - 0x4)
    blt     a2, v0, 1f
    nop
    //MAX reach boundary
    //MIN not reach boundary, MAX reach boundary, use (MAX+MID)/2
    daddu   v0, a3, a0
    dsrl    v0, v0, 1
    dli     v1, 0x7f7f7f7f7f7f7f7f
    and     v0, v0, v1

    daddu   v0, v0, a0
    dsrl    v0, v0, 1
    dli     v1, 0x7f7f7f7f7f7f7f7f
    and     v0, v0, v1

    dli     v1, 0xffffffffffffffff
    not     v1, v1
    and     a0, a0, v1
    or      a0, a0, v0

    b       88f
    nop
1:
    //MAX not reach boundary
    //MIN && MAX neither reach boundary, use mid_value
    b       88f
    nop
2:
    //MIN == 0
    dsrl    a2, a3, 0
    and     a2, a2, 0x7f
    dli     v0, (WRLVL_MAX_DELAY - 0x4)
    blt     a2, v0, 1f
    nop
    //MAX reach boundary
    //MIN && MAX both reach boundary, use mid_value / 2
    b       88f
    nop
1:
    //MIN reach boundary, MAX not reach boundary
#if 0   //mid_value / 2
    dsrl    v0, a0, 1
    dli     a2, 0x7f7f7f7f7f7f7f7f
    and     v0, v0, a2
    dli     a2, 0xffffffffffffffff
    not     a2, a2
    and     a0, a0, a2
    or      a0, a0, v0
#endif
#if 1    //use max_value - offset
    dli     a2, 0x3030303030303030
    dsubu   a0, a3, a2
    dli     a2, 0x7f7f7f7f7f7f7f7f
    and     a0, a0, a2
#endif
88:
#endif
#endif
    sd      a0, GD_MID(t8)
#ifdef  PRINT_MSG
    PRINTSTR("\r\nCal Mid value: 0x")
    ld      t7, GD_MID(t8)
    dsrl    a0, t7, 32
    bal     hexserial
    nop
    move    a0, t7
    bal     hexserial
    nop
#endif

#ifdef  USE_WRLVL_CLK_OFFSET_VALUE
    b       arb_wrlvl_value_caled
    nop

arb_wrlvl_default_value:
    //get clk_dll value
    ld      a2, CLKLVL_DELAY_VALUE(t8)
    //shift byte 0 value to 8 bytes of a2
    dsll    v0, a2, 0x8
    daddu   a2, a2, v0
    dsll    v0, a2, 0x10
    daddu   a2, a2, v0
    dsll    v0, a2, 0x20
    daddu   a2, a2, v0
    //load different value under different freq and DIMM number
    //currently, all use the same default value
    dli     a0, WRLVL_CLK_OFFSET_VALUE
    daddu   a0, a0, a2
    sd      a0, GD_MID(t8)
#endif

arb_wrlvl_value_caled:
    ld      t2, GD_MID(t8)
#ifdef DEBUG_ARB_LEVEL_WR
    PRINTSTR("\r\nWrite param and value:\r\nt2 = 0x")
    dsrl    a0, t2, 32
    bal     hexserial
    nop
    move    a0, t2
    bal     hexserial
    nop
#endif
    dli     t6, 0x5
    bal     arb_write_param
    nop

#ifndef KEEP_DQ_DLY_FIXED
    //recover wrlvl dq delay value
    dli     t6, 0x6
#ifndef USE_SPECIAL_WRLVL_DQ_DELAY
    dli     t2, WRLVL_DQ_DEFAULT_DLY
    bal     arb_modify_param
    nop
#else
    dli     t2, WRLVL_DQ_SPECIAL_DLY
    bal     arb_write_param
    nop
#endif
#endif

#ifdef DEBUG_ARB_LEVEL_WR
    PRINTSTR("\r\nAfter write leveling. The MC configuration is:\r\n")

    bal     enable_ddr_confspace
    nop

    dli     t1, DDR_PARAM_NUM
    dli     t7, DDR_MC_CONFIG_BASE
    GET_ARB_LEVEL_NODE_ID
    dsll    a1, a1, 44
    or      t7, t7, a1
1:
    ld      t3, 0x0(t7)
    dsrl    a0, t3, 32
    bal     hexserial
    nop
    PRINTSTR("  ")
    move    a0, t3
    bal     hexserial
    nop
    PRINTSTR("\r\n")

    daddiu  t1, t1, -1
    daddiu  t7, t7, 16
    bnez    t1, 1b
    nop

    bal     disable_ddr_confspace
    nop
#endif
#endif

//2. read_level read param cfg: include: POP delay(3 or 4), rd_gate_cfg, rd gate delay line and rdlvl_dqsP/N
//use RW result, because we have done wr_dqs leveling, so there should not be RW errors.
/*
 * s3: level byte slice--global
 * t8: ARB_STORE_BASE--global
 * s5: slice mask--global
 * s4: slice inner loop control--global
 * t6: lvl sel-- stage global
 * t2: lvl value--stage global
 * t9: gate lvl value control--stage global
 */
#ifdef  PRINT_MSG
    PRINTSTR("\r\n\r\nStart read leveling..")
#endif

    //s3---level byte lane
    dli     s3, 0x7
    GET_DIMM_WIDTH
    beqz    a1, 1f
    nop
    dli     s3, 0x3
1:
    //set t8
    dli     t8, ARB_STORE_BASE
    GET_ARB_LEVEL_NODE_ID
    dsll    a1, a1, 44
    daddu   t8, t8, a1

//loop for 8 slice
rdlvl_one_byte_start:
    //one bye lane level begin
    bltz    s3, rdlvl_end
    nop
    //clear store mem
    sd      $0, 0x0(t8)
    sd      $0, 0x8(t8)
    sd      $0, 0x10(t8)
    sd      $0, 0x18(t8)
    sd      $0, 0x20(t8)
    sd      $0, 0x28(t8)
    sd      $0, 0x30(t8)
    sd      $0, 0x38(t8)
    sd      $0, 0x40(t8)
    sd      $0, 0x48(t8)
    sd      $0, 0x50(t8)
    sd      $0, 0x58(t8)

    //set specified byte lanes Mask
    dli     a0, 0x8
    dmul    a0, a0, s3
    dli     a1, 0xff
    dsll    s5, a1, a0

    GET_DIMM_WIDTH
    beqz    a1, 1f
    nop
    //for reduc setting, set mask to high
    dsll    a1, s5, 32
    or      s5, s5, a1
1:

#ifdef  PRINT_MSG
    PRINTSTR("\r\n\r\n\r\nLevel slice: 0x")
    move    a0, s3
    bal     hexserial
    nop
    PRINTSTR("\r\nWait a while...")
#endif
#ifdef  DEBUG_ARB_LEVEL_RD
    PRINTSTR("\r\ns5 = 0x")
    dsrl    a0, s5, 32
    bal     hexserial
    nop
    move    a0, s5
    bal     hexserial
    nop
#endif

    //test the possible two RD pop delay
    //loop twice: for each POP delay cfg(3 or 4), find a best gate cfg(include phy_gate_cfg and rdlvl_gate_delay) first,
    //then calculate the range of rdlvl_delay_P/N, use the larger range as final setting.
    move    s4, $0
rdlvl_pop_delay_start:
    //set rdlvl_delay_P/N to default value
    dli     t2, RDLVL_DEFAULT_DELAY
    dli     t6, 0x3
    bal     arb_modify_param
    nop
    dli     t6, 0x4
    bal     arb_modify_param
    nop

    //leveling phy_1 read gate cfg and rdlvl_gate_delay
    //out loop. Coarse leveling.---adjust phy_1 read gate cfg
    dli     t9, 0x2
    //decrease from bigger to small because we want to use as bigger as possible.
rdlvl_gate_cfg_start:
    move    t2, t9
#ifdef  DEBUG_ARB_LEVEL_RD_CFG
    PRINTSTR("\r\nModify read gate cfg: t2 = 0x")
    move    a0, t2
    bal     hexserial
    nop
#endif
    dli     t6, 0x7
    bal     arb_modify_param
    nop
#ifdef  DEBUG_ARB_LEVEL_RD_CFG
    PRINTSTR("\r\nThe MC configuration is:\r\n")
    bal     enable_ddr_confspace
    nop

    //dli     t1, DDR_PARAM_NUM
    dli     t1, 0xa         //set print num
    dli     t7, DDR_MC_CONFIG_BASE
    GET_ARB_LEVEL_NODE_ID
    dsll    a1, a1, 44
    or      t7, t7, a1
    daddu   t7, t7, 0x2d0   //set start offset
1:
    ld      t3, 0x0(t7)
    dsrl    a0, t3, 32
    bal     hexserial
    nop
    PRINTSTR("  ")
    move    a0, t3
    bal     hexserial
    nop
    PRINTSTR("\r\n")

    daddiu  t1, t1, -1
    daddiu  t7, t7, 16
    bnez    t1, 1b
    nop

    bal     disable_ddr_confspace
    nop
#endif
    //inner loop. fine leveling.---adjust rdlvl_gate_delay_X

    //clear Store space
    sd      zero, BYTE_TM_RST(t8)
    sd      zero, GD_MIN(t8)
    sd      zero, GD_MAX(t8)
    sd      zero, GD_MID(t8)

    //initialize
    dli     t6, 0x2
    //set t2 start value
    dli     t2, RDLVL_GATE_MAX_DELAY

    dli     t0, 0x1
    dsrl    a0, t2, GATE_LOG2_STEP
    dsll    t0, t0, a0

    dli     t5, 1 << GATE_LOG2_STEP
31:
//write new delay value
    bal     arb_modify_param
    nop

//do Test and print test result
    bal     arb_test_mem
    nop
    move    t3, v0
    move    t4, v1
#ifdef  DEBUG_ARB_LEVEL_RD_TM
    PRINTSTR("\r\nt2 = 0x")
    move    a0, t2
    bal     hexserial
    nop
    PRINTSTR(":")
    beqz    t3, 1f
    nop
    PRINTSTR("\r\nRW Diff 0x")
    dsrl    a0, t3, 32
    bal     hexserial
    nop
    move    a0, t3
    bal     hexserial
    nop
    PRINTSTR("\r\nRD Diff 0x")
    dsrl    a0, t4, 32
    bal     hexserial
    nop
    move    a0, t4
    bal     hexserial
    nop
    b       2f
    nop
1:
    PRINTSTR("\r\nNo Error detected.")
2:
#endif
    //Mask out other byte lanes info
    and     t3, t3, s5
    and     t4, t4, s5
//process TM result: translate Byte error info into 1 bit info in each BX_TM_RST of every Byte.
//64 bit BX_TM_RST work as a bit map corresponding to every param value(so the min step interval
//is 2, or there will be not enough space to store TM RST info), the 64 bit can be only part valid(
//step interval > 2).
    beqz    t3, 1f
    nop
    //error detected
    ld      a0, BYTE_TM_RST(t8)
    or      a0, a0, t0
    sd      a0, BYTE_TM_RST(t8)
1:
    dsrl    t0, t0, 1

    //check whether delay value reach the boundary
    dsubu   a2, t2, t5
    bltz    a2, 11f //check the new delay value whether exceed limitation
    nop
    /** not exceed **/
    move    t2, a2
    b       31b
    nop
11:

#ifdef  DEBUG_ARB_LEVEL_RD
    PRINTSTR("\r\nlevel result is:\r\n")
    ld      t7, BYTE_TM_RST(t8)
    dsrl    a0, t7, 32
    bal     hexserial
    nop
    move    a0, t7
    bal     hexserial
    nop
#endif
//calculate mid value for this byte lane
/***********
boundary sign: contain at least RDLVL_WINDOW_ZERO_NUM consecutive 0(TM success)
    t0: parse pointer
    t2: parse max position
    t3: BYTE_X_LEVEL_RST
    t4: RDLVL_WINDOW_ZERO_NUM
***********/
    //set t2 max value for each level object
    dli     t2, RDLVL_GATE_MAX_DELAY
    dsrl    t2, t2, GATE_LOG2_STEP

    ld      t3, BYTE_TM_RST(t8)
#ifdef  DEBUG_ARB_LEVEL_RD_TM
    PRINTSTR("\r\nt3 = 0x")
    dsrl    a0, t3, 32
    bal     hexserial
    nop
    move    a0, t3
    bal     hexserial
    nop
#endif
    move    t0, $0
    dli     t4, RDLVL_WINDOW_ZERO_NUM + (GATE_ADJUST)
12:
    bgtu    t0, t2, 3f
    nop
    dsrl    t7, t3, t0
    and     t7, t7, 0x1
    bnez    t7, 1f
    nop
    //find a TM success
    daddiu  t4, t4, -1
    beqz    t4, 2f
    nop
    //continue move
    daddiu  t0, t0, 0x1
    b       12b
    nop
1:  //find a TM fail
    dli     t4, RDLVL_WINDOW_ZERO_NUM + (GATE_ADJUST)
    //continue move
    daddiu  t0, t0, 0x1
    b       12b
    nop
2:  //window found
    //calculate the MIN boundary
    dli     a1, RDLVL_WINDOW_ZERO_NUM + (GATE_ADJUST)
    daddiu  a0, t0, 1
    dsubu   a0, a0, a1
    dsll    a0, a0, GATE_LOG2_STEP   //a0 = a0 * 2n
    and     a0, a0, 0xff
    sd      a0, GD_MIN(t8)
    //move forward to the next Fail to cal the MAX boundary
1:
    daddiu  t0, t0, 0x1
    bgtu    t0, t2, 1f
    nop
    dsrl    t7, t3, t0
    and     t7, t7, 0x1
    beqz    t7, 1b  //continue move
    nop
1:
    //find a TM FAIL or reach the max test value
    daddiu  a0, t0, -1
    dsll    a0, a0, GATE_LOG2_STEP   //a0 = a0 * 2n
    and     a0, a0, 0xff
    sd      a0, GD_MAX(t8)
    b       2f
    nop
3:  //parse to end, CAN NOT find a window
#ifdef  DEBUG_ARB_LEVEL_RD
    PRINTSTR("\r\nError: RDLVL Gate delay Window not found.")
    PRINTSTR("\r\nt3 = 0x")
    dsrl    a0, t3, 32
    bal     hexserial
    nop
    move    a0, t3
    bal     hexserial
    nop
#endif
    beqz    t9, rdlvl_gate_cfg_fail
    nop
    dsrl    t9, t9, 0x1
    b       rdlvl_gate_cfg_start
    nop

rdlvl_gate_cfg_fail:    //tryed all setting(0,1), can NOT find a right cfg, try another pop delay.
    //write fail mark
    dli     a1, 0x1
    dsll    a1, a1, s4
    ld      a0, RDLVL_FAIL_MARK(t8)
    or      a0, a0, a1
    sd      a0, RDLVL_FAIL_MARK(t8)

    b       rdlvl_pop_delay_end
    nop

2:  //find this byte lane's window
    //Find a right cfg
    //store rd gate cfg
    dsll    a1, s4, 3
    dsll    a1, t9, a1
    ld      a0, RDLVL_GATE_CFG(t8)
    or      a0, a0, a1
    sd      a0, RDLVL_GATE_CFG(t8)

#ifdef  DEBUG_ARB_LEVEL_RD
    PRINTSTR("\r\nMin value: 0x")
    ld      t7, GD_MIN(t8)
    move    a0, t7
    bal     hexserial
    nop
    PRINTSTR("\r\nMax value: 0x")
    ld      t7, GD_MAX(t8)
    move    a0, t7
    bal     hexserial
    nop
#endif
    //store rdlvl_gate_delay min / max
    dsll    a2, s4, 3

    ld      a1, GD_MIN(t8)
    dsll    a1, a1, a2
    ld      a0, RDLVL_GATE_GD_MIN(t8)
    or      a0, a0, a1
    sd      a0, RDLVL_GATE_GD_MIN(t8)

    ld      a1, GD_MAX(t8)
    dsll    a1, a1, a2
    ld      a0, RDLVL_GATE_GD_MAX(t8)
    or      a0, a0, a1
    sd      a0, RDLVL_GATE_GD_MAX(t8)

//calculate mid value for this byte lane
    ld      a0, GD_MIN(t8)
    ld      a1, GD_MAX(t8)
    daddu   a0, a0, a1
    dsrl    a0, a0, 1
#if 1
    //max rdlvl_gate_delay == RDLVL_GATE_MAX_DELAY, use (mid + max) / 2
    //else use max value - 0x10
    dli     a2, RDLVL_GATE_MAX_DELAY
    blt     a1, a2, 1f
    nop
    daddu   a0, a0, a1
    dsrl    a0, a0, 1
    b       8f
    nop
1:
    dli     a2, 0x10
    dsubu   a0, a1, a2
    bge     a0, $0, 8f
    nop
    move    a0, $0
8:
#endif
    sd      a0, GD_MID(t8)
    move    t2, a0

#ifdef  DEBUG_ARB_LEVEL_RD
    PRINTSTR("\r\nCal Mid value: 0x")
    ld      t7, GD_MID(t8)
    move    a0, t7
    bal     hexserial
    nop
#endif
    bal     arb_modify_param
    nop

    //3. find the rdlvl_delay_P/N range.
    //level RD_DQSp/n delay line setting
    //use t6 to control rdlvl_delayP or rdlvl_delayN
    dli     t6, 0x3
21:
    dli     a1, 0x4
    bgt     t6, a1, rdlvl_pop_delay_end
    nop
//one level begin
    //clear Store space
    sd      zero, BYTE_TM_RST(t8)
    sd      zero, GD_MIN(t8)
    sd      zero, GD_MAX(t8)
    sd      zero, GD_MID(t8)

    //set t2 start value
    dli     t2, RDLVL_MAX_DELAY

    dli     t0, 0x1
    dsrl    a0, t2, RDLVL_LOG2_STEP
    dsll    t0, t0, a0

    dli     t5, 1 << RDLVL_LOG2_STEP
31:
//write new delay value
    bal     arb_modify_param
    nop

//do Test and print test result
    bal     arb_test_mem
    nop
    move    t3, v0
    move    t4, v1
#ifdef  DEBUG_ARB_LEVEL_RD_TM
    PRINTSTR("\r\nt2 = 0x")
    move    a0, t2
    bal     hexserial
    nop
    PRINTSTR(":")
    beqz    t3, 1f
    nop
    PRINTSTR("\r\nRW Diff 0x")
    dsrl    a0, t3, 32
    bal     hexserial
    nop
    move    a0, t3
    bal     hexserial
    nop
    PRINTSTR("\r\nRD Diff 0x")
    dsrl    a0, t4, 32
    bal     hexserial
    nop
    move    a0, t4
    bal     hexserial
    nop
    b       2f
    nop
1:
    PRINTSTR("\r\nNo Error detected.")
2:
#endif
    //Mask out this byte lane info
    and     t3, t3, s5
    and     t4, t4, s5
//process TM result: translate Byte error info into 1 bit info in each BX_TM_RST of every Byte.
//64 bit BX_TM_RST work as a bit map corresponding to every param value(so the min step interval
//is 2, or there will be not enough space to store TM RST info), the 64 bit can be only part valid(
//step interval > 2).
    beqz    t3, 1f
    nop
    //error detected
    ld      a0, BYTE_TM_RST(t8)
    or      a0, a0, t0
    sd      a0, BYTE_TM_RST(t8)
1:
    dsrl    t0, t0, 1

    //check whether delay value reach the boundary
    dsubu   a2, t2, t5
    bltz    a2, 11f //check the new delay value whether exceed limitation
    nop
    /** not exceed **/
    move    t2, a2
    b       31b
    nop
11:

#ifdef  DEBUG_ARB_LEVEL_RD_TM
    PRINTSTR("\r\nlevel result is:\r\n")
    ld      t7, BYTE_TM_RST(t8)
    dsrl    a0, t7, 32
    bal     hexserial
    nop
    move    a0, t7
    bal     hexserial
    nop
#endif
//calculate mid value for this byte lane
/***********
boundary sign: contain at least RDLVL_WINDOW_ZERO_NUM consecutive 0(TM success)
    t0: parse pointer
    t2: parse max position
    t3: BYTE_X_LEVEL_RST
    t4: RDLVL_WINDOW_ZERO_NUM
***********/
    //set t2 max value for each level object
    dli     t2, RDLVL_MAX_DELAY
    dsrl    t2, t2, RDLVL_LOG2_STEP

    ld      t3, BYTE_TM_RST(t8)
#ifdef  DEBUG_ARB_LEVEL_RD_TM
    PRINTSTR("\r\nt3 = 0x")
    dsrl    a0, t3, 32
    bal     hexserial
    nop
    move    a0, t3
    bal     hexserial
    nop
#endif
    move    t0, $0
    dli     t4, RDLVL_WINDOW_ZERO_NUM
12:
    bgtu    t0, t2, 3f
    nop
    dsrl    t7, t3, t0
    and     t7, t7, 0x1
    bnez    t7, 1f
    nop
    //find a TM success
    daddiu  t4, t4, -1
    beqz    t4, 2f
    nop
    //continue move
    daddiu  t0, t0, 0x1
    b       12b
    nop
1:  //find a TM fail
    dli     t4, RDLVL_WINDOW_ZERO_NUM
    //continue move
    daddiu  t0, t0, 0x1
    b       12b
    nop
2:  //window found
    //calculate the MIN boundary
    dli     a1, RDLVL_WINDOW_ZERO_NUM
    daddiu  a0, t0, 1
    dsubu   a0, a0, a1
    dsll    a0, a0, RDLVL_LOG2_STEP   //a0 = a0 * 2n
    and     a0, a0, 0xff
    sd      a0, GD_MIN(t8)
    //move forward to the next Fail to cal the MAX boundary
1:
    daddiu  t0, t0, 0x1
    bgtu    t0, t2, 1f
    nop
    dsrl    t7, t3, t0
    and     t7, t7, 0x1
    beqz    t7, 1b  //continue move
    nop
1:
    //find a TM FAIL or reach the max test value
    daddiu  a0, t0, -1
    dsll    a0, a0, RDLVL_LOG2_STEP   //a0 = a0 * 2n
    and     a0, a0, 0xff
    sd      a0, GD_MAX(t8)
    b       2f
    nop
3:  //parse to end, CAN NOT find a window
#ifdef  DEBUG_ARB_LEVEL_RD
    PRINTSTR("\r\nError: rdlvl_delay Byte Window not found.")
    PRINTSTR("\r\nt3 = 0x")
    dsrl    a0, t3, 32
    bal     hexserial
    nop
    move    a0, t3
    bal     hexserial
    nop
#endif
    dli     a1, 0x1
    dsll    a1, a1, s4
    ld      a0, RDLVL_FAIL_MARK(t8)
    or      a0, a0, a1
    sd      a0, RDLVL_FAIL_MARK(t8)

    b       rdlvl_pop_delay_end
    nop
2:  //this byte lane's MIN and MAX value stored
#ifdef  DEBUG_ARB_LEVEL_RD
    PRINTSTR("\r\nMin value: 0x")
    ld      t7, GD_MIN(t8)
    move    a0, t7
    bal     hexserial
    nop
    PRINTSTR("\r\nMax value: 0x")
    ld      t7, GD_MAX(t8)
    move    a0, t7
    bal     hexserial
    nop
#endif

    //store rdlvl_delay_p/n min / max
    dsll    a2, s4, 3

    dli     a1, 0x3
    bne     t6, a1, 1f
    nop
    ld      a1, GD_MIN(t8)
    dsll    a1, a1, a2
    ld      a0, RDLVL_DELAYP_GD_MIN(t8)
    or      a0, a0, a1
    sd      a0, RDLVL_DELAYP_GD_MIN(t8)

    ld      a1, GD_MAX(t8)
    dsll    a1, a1, a2
    ld      a0, RDLVL_DELAYP_GD_MAX(t8)
    or      a0, a0, a1
    sd      a0, RDLVL_DELAYP_GD_MAX(t8)
    b       2f
    nop
1:
    ld      a1, GD_MIN(t8)
    dsll    a1, a1, a2
    ld      a0, RDLVL_DELAYN_GD_MIN(t8)
    or      a0, a0, a1
    sd      a0, RDLVL_DELAYN_GD_MIN(t8)

    ld      a1, GD_MAX(t8)
    dsll    a1, a1, a2
    ld      a0, RDLVL_DELAYN_GD_MAX(t8)
    or      a0, a0, a1
    sd      a0, RDLVL_DELAYN_GD_MAX(t8)
2:
    //write default value to this rdlvl_delayP/N for next loop
    dli     t2, RDLVL_DEFAULT_DELAY
    bal     arb_modify_param
    nop

    //move to next level
    daddiu  t6, t6, 0x1
    b       21b
    nop

rdlvl_pop_delay_end:
#ifdef  DEBUG_ARB_LEVEL_RD_CFG
    PRINTSTR("\r\nThe MC configuration is:\r\n")
    bal     enable_ddr_confspace
    nop

    //dli     t1, DDR_PARAM_NUM
    dli     t1, 0xa         //set print num
    dli     t7, DDR_MC_CONFIG_BASE
    GET_ARB_LEVEL_NODE_ID
    dsll    a1, a1, 44
    or      t7, t7, a1
    daddu   t7, t7, 0x2d0   //set start offset
1:
    ld      t3, 0x0(t7)
    dsrl    a0, t3, 32
    bal     hexserial
    nop
    PRINTSTR("  ")
    move    a0, t3
    bal     hexserial
    nop
    PRINTSTR("\r\n")

    daddiu  t1, t1, -1
    daddiu  t7, t7, 16
    bnez    t1, 1b
    nop

    bal     disable_ddr_confspace
    nop
#endif

    bnez    s4, 26f
    nop
    daddu   s4, s4, 1
#ifdef  DEBUG_ARB_LEVEL_RD
    PRINTSTR("\r\nalter pop delay setting.\r\n")
#endif
    bal     arb_modify_pop_delay_alter
    nop
    b       rdlvl_pop_delay_start
    nop

26: //two pop delay value has been tested.
#ifdef  PRINT_MSG
    PRINTSTR("\r\nRDLVL_FAIL_MARK: 0x")
    ld      t7, RDLVL_FAIL_MARK(t8)
    move    a0, t7
    bal     hexserial
    nop
    PRINTSTR("\r\nRDLVL_GATE_CFG: 0x")
    ld      t7, RDLVL_GATE_CFG(t8)
    move    a0, t7
    bal     hexserial
    nop
    PRINTSTR("\r\nRDLVL_GATE_GD_MIN: 0x")
    ld      t7, RDLVL_GATE_GD_MIN(t8)
    move    a0, t7
    bal     hexserial
    nop
    PRINTSTR("\r\nRDLVL_GATE_GD_MAX: 0x")
    ld      t7, RDLVL_GATE_GD_MAX(t8)
    move    a0, t7
    bal     hexserial
    nop
    PRINTSTR("\r\nRDLVL_DELAYP_GD_MIN: 0x")
    ld      t7, RDLVL_DELAYP_GD_MIN(t8)
    move    a0, t7
    bal     hexserial
    nop
    PRINTSTR("\r\nRDLVL_DELAYP_GD_MAX: 0x")
    ld      t7, RDLVL_DELAYP_GD_MAX(t8)
    move    a0, t7
    bal     hexserial
    nop
    PRINTSTR("\r\nRDLVL_DELAYN_GD_MIN: 0x")
    ld      t7, RDLVL_DELAYN_GD_MIN(t8)
    move    a0, t7
    bal     hexserial
    nop
    PRINTSTR("\r\nRDLVL_DELAYN_GD_MAX: 0x")
    ld      t7, RDLVL_DELAYN_GD_MAX(t8)
    move    a0, t7
    bal     hexserial
    nop
#endif
    ld      a0, RDLVL_FAIL_MARK(t8)
    beqz    a0, both_pop_delay_ok
    nop
    //at least 1 pop delay failed
    dli     a1, 0x3
    beq     a0, a1, both_pop_delay_fail
    nop
    dli     a1, 0x2
    //a0 == 2'b10
    beq     a0, a1, first_pop_delay_ok
    nop
    //a0 == 2'b01
    b       second_pop_delay_ok
    nop

both_pop_delay_fail:
#ifdef  MODIFY_PAD_COMP
    PRINTSTR("\r\nTry another pad compensation.\r\n")
    bal     arb_modify_pad_comp
    nop
    beqz    v0, rdlvl_one_byte_start
    nop
#endif
#ifdef  PRINT_MSG
    PRINTSTR("\r\nERROR: This Slice level failed, write default value.")
#endif
//write standard value
    //write the phy_1_gate_cfg
    dli     t6, 0x7
    dli     t2, 0x1
    bal     arb_modify_param
    nop
    //write the rdlvl_gate_delay
    dli     t6, 0x2
    dli     t2, 0x19
    bal     arb_modify_param
    nop
    //write rdlvl_delay_p/n
    dli     t2, RDLVL_DEFAULT_DELAY
    dli     t6, 0x3
    bal     arb_modify_param
    nop
    daddu   t6, t6, 0x1
    bal     arb_modify_param
    nop
    b       rdlvl_one_byte_end
    nop

both_pop_delay_ok:
    //use test result stored in RDLVL_DELAYP/N_MIN and RDLVL_DELAYP/N_MAX to
    //decide use which pop delay and relative phy_1_gate_cfg
    //t1, t3--first pop delay result
    //t2, t4--second pop delay result
    ld      a0, RDLVL_DELAYP_GD_MIN(t8)
    ld      a1, RDLVL_DELAYP_GD_MAX(t8)
    dsubu   a2, a1, a0
    dsrl    t1, a2, 0
    dsrl    t2, a2, 8
    ld      a0, RDLVL_DELAYN_GD_MIN(t8)
    ld      a1, RDLVL_DELAYN_GD_MAX(t8)
    dsubu   a2, a1, a0
    dsrl    t3, a2, 0
    dsrl    t4, a2, 8
    and     t1, t1, 0xff
    and     t2, t2, 0xff
    and     t3, t3, 0xff
    and     t4, t4, 0xff

    //find the min of first pop delay and second pop delay
    //t1=MIN(RDLVL_DELAYP/N) first
    //t2=MIN(RDLVL_DELAYP/N) second

    ble     t1, t3, 1f
    nop
    move    t1, t3
1:
    ble     t2, t4, 1f
    nop
    move    t2, t4
1:
#ifdef  DDR3_DIMM
    //for slice 4~7
    //dli     a0, 0x4
    //blt     s3, a0, 1f
    //nop

    //give the second cfg(4) priority, if the difference <= 8, use second_pop_delay
    dsubu   t1, t1, 0x8
1:
#endif
    ble     t1, t2, second_pop_delay_ok
    nop
    //the first cfg is better
first_pop_delay_ok:
#ifdef  PRINT_MSG
    PRINTSTR("\r\nThis Slice level success, use first value.")
#endif
    bal     arb_modify_pop_delay_alter
    nop
    //write the phy_1_gate_cfg
    dli     t6, 0x7
    ld      a1, RDLVL_GATE_CFG(t8)
    and     t2, a1, 0xff
    bal     arb_modify_param
    nop
    //write the rdlvl_gate_delay
    ld      a0, RDLVL_GATE_GD_MIN(t8)
    ld      a1, RDLVL_GATE_GD_MAX(t8)
    and     a0, a0, 0xff
    and     a1, a1, 0xff
    daddu   a0, a0, a1
    dsrl    a0, a0, 1
#if 1
    //max rdlvl_gate_delay == RDLVL_GATE_MAX_DELAY, use (mid + max) / 2
    //else use max value - 0x10
    dli     a2, RDLVL_GATE_MAX_DELAY
    blt     a1, a2, 1f
    nop
    daddu   a0, a0, a1
    dsrl    a0, a0, 1
    b       8f
    nop
1:
    dli     a2, 0x10
    dsubu   a0, a1, a2
    bge     a0, $0, 8f
    nop
    move    a0, $0
8:
#endif
    sd      a0, GD_MID(t8)
    move    t2, a0

    dli     t6, 0x2
    bal     arb_modify_param
    nop

    //write rdlvl_delay_p/n
    dli     t2, RDLVL_DEFAULT_DELAY
#ifndef USE_DEFAULT_RDLVL_DELAY
    ld      a0, RDLVL_DELAYP_GD_MIN(t8)
    ld      a1, RDLVL_DELAYP_GD_MAX(t8)
    and     a0, a0, 0xff
    and     a1, a1, 0xff
    daddu   a0, a0, a1
    dsrl    a0, a0, 1
    move    t2, a0
#endif
    dli     t6, 0x3
    bal     arb_modify_param
    nop
    daddu   t6, t6, 0x1
#ifndef USE_DEFAULT_RDLVL_DELAY
    ld      a0, RDLVL_DELAYN_GD_MIN(t8)
    ld      a1, RDLVL_DELAYN_GD_MAX(t8)
    and     a0, a0, 0xff
    and     a1, a1, 0xff
    daddu   a0, a0, a1
    dsrl    a0, a0, 1
    move    t2, a0
#endif
    bal     arb_modify_param
    nop

    b       rdlvl_one_byte_end
    nop

second_pop_delay_ok:
#ifdef  PRINT_MSG
    PRINTSTR("\r\nThis Slice level success, use second value.")
#endif
#ifndef USE_DEFAULT_RDLVL_DELAY
    //write rdlvl_delay_p/n
    ld      a0, RDLVL_DELAYP_GD_MIN(t8)
    ld      a1, RDLVL_DELAYP_GD_MAX(t8)
    dsrl    a0, a0, 0x8
    dsrl    a1, a1, 0x8
    and     a0, a0, 0xff
    and     a1, a1, 0xff
    daddu   a0, a0, a1
    dsrl    a0, a0, 1
    move    t2, a0

    dli     t6, 0x3
    bal     arb_modify_param
    nop
    daddu   t6, t6, 0x1

    ld      a0, RDLVL_DELAYN_GD_MIN(t8)
    ld      a1, RDLVL_DELAYN_GD_MAX(t8)
    dsrl    a0, a0, 0x8
    dsrl    a1, a1, 0x8
    and     a0, a0, 0xff
    and     a1, a1, 0xff
    daddu   a0, a0, a1
    dsrl    a0, a0, 1
    move    t2, a0

    bal     arb_modify_param
    nop
#endif
#if 0
    //need to do nothing as all the correct cfg has been written.
    dli     t2, RDLVL_DEFAULT_DELAY
    dli     t6, 0x3
    bal     arb_modify_param
    nop
    daddu   t6, t6, 0x1
    bal     arb_modify_param
    nop
#endif

rdlvl_one_byte_end:

#ifdef  DEBUG_ARB_LEVEL_RD_CFG
    PRINTSTR("\r\nAfter rdlvl_one_byte test. The MC configuration is:\r\n")
    bal     enable_ddr_confspace
    nop

    dli     t1, DDR_PARAM_NUM
    dli     t7, DDR_MC_CONFIG_BASE
    GET_ARB_LEVEL_NODE_ID
    dsll    a1, a1, 44
    or      t7, t7, a1
1:
    ld      t3, 0x0(t7)
    dsrl    a0, t3, 32
    bal     hexserial
    nop
    PRINTSTR("  ")
    move    a0, t3
    bal     hexserial
    nop
    PRINTSTR("\r\n")

    daddiu  t1, t1, -1
    daddiu  t7, t7, 16
    bnez    t1, 1b
    nop

    bal     disable_ddr_confspace
    nop
#endif
//level next byte slice
    daddu   s3, s3, -1
    b       rdlvl_one_byte_start
    nop

//rdlvl finished
rdlvl_end:
#ifdef  DEBUG_ARB_LEVEL_RD_CFG
    PRINTSTR("\r\nAfter read level. The MC configuration is:\r\n")
    bal     enable_ddr_confspace
    nop

    dli     t1, DDR_PARAM_NUM
    dli     t7, DDR_MC_CONFIG_BASE
    GET_ARB_LEVEL_NODE_ID
    dsll    a1, a1, 44
    or      t7, t7, a1
1:
    ld      t3, 0x0(t7)
    dsrl    a0, t3, 32
    bal     hexserial
    nop
    PRINTSTR("  ")
    move    a0, t3
    bal     hexserial
    nop
    PRINTSTR("\r\n")

    daddiu  t1, t1, -1
    daddiu  t7, t7, 16
    bnez    t1, 1b
    nop

    bal     disable_ddr_confspace
    nop
#endif

#ifdef  CLOCK_LEVEL   //don't enable it
//clk level---cannot be used under DDR3 mode!!!

//#define DEBUG_ARB_LEVEL_CLK
//#define DEBUG_ARB_LEVEL_CLK_TM
//#define DEBUG_ARB_LEVEL_CLK_CFG

#ifdef  PRINT_MSG
    PRINTSTR("\r\n\r\nStart Clock Leveling. Wait a while...")
#endif

arb_clklvl_start:
#ifdef  DEBUG_ARB_LEVEL_CLK_CFG
    PRINTSTR("\r\nThe MC configuration is:\r\n")
    bal     enable_ddr_confspace
    nop

    dli     t1, DDR_PARAM_NUM
    dli     t7, DDR_MC_CONFIG_BASE
    GET_ARB_LEVEL_NODE_ID
    dsll    a1, a1, 44
    or      t7, t7, a1
1:
    ld      t8, 0x0(t7)
    dsrl    a0, t8, 32
    bal     hexserial
    nop
    PRINTSTR("  ")
    move    a0, t8
    bal     hexserial
    nop
    PRINTSTR("\r\n")

    daddiu  t1, t1, -1
    daddiu  t7, t7, 16
    bnez    t1, 1b
    nop

    bal     disable_ddr_confspace
    nop
#endif
    dli     s3, 0xf //change all 8 byte slices

//1. level Write DQS Delay line setting.
    //clear store mem
    dli     t8, ARB_STORE_BASE
    GET_ARB_LEVEL_NODE_ID
    dsll    a1, a1, 44
    daddu   t8, t8, a1
    sd      $0, 0x0(t8)
    sd      $0, 0x8(t8)
    sd      $0, 0x10(t8)
    sd      $0, 0x18(t8)
    sd      $0, 0x20(t8)
    sd      $0, 0x28(t8)
    sd      $0, 0x30(t8)
    sd      $0, 0x38(t8)
    sd      $0, 0x40(t8)
    sd      $0, 0x48(t8)
    sd      $0, 0x50(t8)
    sd      $0, 0x58(t8)

    //set test interval
    dli     t5, 1 << LOG2_STEP
    //set t2 start value
    move    t2, $0
    dli     s4, CLKLVL_MAX_DELAY
#if 0
    PRINTSTR("\r\nplease input clklvl max value: ")
    bal     inputaddress
    nop
    move    s4, v0
#endif

    dli     t0, 0x1
    dsrl    a0, t2, LOG2_STEP
    dsll    t0, t0, a0

clklvl_test_one_delay:
    move    t3, $0
    move    t4, $0
    not     t3, t3
    not     t4, t4

#ifdef  DEBUG_ARB_LEVEL_CLK_TM
    PRINTSTR("\r\n\r\nt2 = 0x")
    move    a0, t2
    bal     hexserial
    nop
#endif
    dli     t6, 0x1
//write new delay value
    bal     arb_modify_param
    nop

//do Test and print test result
#ifdef  DEBUG_ARB_LEVEL_CLK_CFG
    PRINTSTR("\r\nThe clk dll configuration is:\r\n")
    bal     enable_ddr_confspace
    nop

    dli     t1, 0x1         //set print num
    dli     t7, DDR_MC_CONFIG_BASE
    GET_ARB_LEVEL_NODE_ID
    dsll    a1, a1, 44
    or      t7, t7, a1
    daddu   t7, t7, 0x8f0   //set start offset
1:
    ld      t8, 0x0(t7)
    dsrl    a0, t8, 32
    bal     hexserial
    nop
    PRINTSTR("  ")
    move    a0, t8
    bal     hexserial
    nop
    PRINTSTR("\r\n")

    daddiu  t1, t1, -1
    daddiu  t7, t7, 16
    bnez    t1, 1b
    nop

    bal     disable_ddr_confspace
    nop
#endif
    bal     arb_test_mem
    nop
#ifdef  DEBUG_ARB_LEVEL_CLK_TM
    move    t7, v0
    move    t8, v1

    PRINTSTR("\r\nRW Diff 0x")
    dsrl    a0, t7, 32
    bal     hexserial
    nop
    move    a0, t7
    bal     hexserial
    nop

    PRINTSTR("\r\nRD Diff 0x")
    dsrl    a0, t8, 32
    bal     hexserial
    nop
    move    a0, t8
    bal     hexserial
    nop

    move    v0, t7
    move    v1, t8
#endif
    //process test result, only when the entire byte is correct(0x00) clear fail mark in t3.
    //byte 7
    dsrl    a0, v0, 0x38
    and     a0, a0, 0xff
    bnez    a0, 1f
    nop
    //find a pass
    dli     a0, 0xff
    dsll    a0, a0, 0x38
    not     a0, a0
    and     t3, t3, a0
1:
    //byte 6
    dsrl    a0, v0, 0x30
    and     a0, a0, 0xff
    bnez    a0, 1f
    nop
    //find a pass
    dli     a0, 0xff
    dsll    a0, a0, 0x30
    not     a0, a0
    and     t3, t3, a0
1:
    //byte 5
    dsrl    a0, v0, 0x28
    and     a0, a0, 0xff
    bnez    a0, 1f
    nop
    //find a pass
    dli     a0, 0xff
    dsll    a0, a0, 0x28
    not     a0, a0
    and     t3, t3, a0
1:
    //byte 4
    dsrl    a0, v0, 0x20
    and     a0, a0, 0xff
    bnez    a0, 1f
    nop
    //find a pass
    dli     a0, 0xff
    dsll    a0, a0, 0x20
    not     a0, a0
    and     t3, t3, a0
1:
    //byte 3
    dsrl    a0, v0, 0x18
    and     a0, a0, 0xff
    bnez    a0, 1f
    nop
    //find a pass
    dli     a0, 0xff
    dsll    a0, a0, 0x18
    not     a0, a0
    and     t3, t3, a0
1:
    //byte 2
    dsrl    a0, v0, 0x10
    and     a0, a0, 0xff
    bnez    a0, 1f
    nop
    //find a pass
    dli     a0, 0xff
    dsll    a0, a0, 0x10
    not     a0, a0
    and     t3, t3, a0
1:
    //byte 1
    dsrl    a0, v0, 0x08
    and     a0, a0, 0xff
    bnez    a0, 1f
    nop
    //find a pass
    dli     a0, 0xff
    dsll    a0, a0, 0x08
    not     a0, a0
    and     t3, t3, a0
1:
    //byte 0
    dsrl    a0, v0, 0x0
    and     a0, a0, 0xff
    bnez    a0, 1f
    nop
    //find a pass
    dli     a0, 0xff
    dsll    a0, a0, 0x0
    not     a0, a0
    and     t3, t3, a0
1:
#ifdef  DEBUG_ARB_LEVEL_CLK_TM
    //record RD history in t4, normally, t4 should be 0x0 after all these trying
    //byte 7
    dsrl    a0, v1, 0x38
    and     a0, a0, 0xff
    bnez    a0, 1f
    nop
    //find a pass
    dli     a0, 0xff
    dsll    a0, a0, 0x38
    not     a0, a0
    and     t4, t4, a0
1:
    //byte 6
    dsrl    a0, v1, 0x30
    and     a0, a0, 0xff
    bnez    a0, 1f
    nop
    //find a pass
    dli     a0, 0xff
    dsll    a0, a0, 0x30
    not     a0, a0
    and     t4, t4, a0
1:
    //byte 5
    dsrl    a0, v1, 0x28
    and     a0, a0, 0xff
    bnez    a0, 1f
    nop
    //find a pass
    dli     a0, 0xff
    dsll    a0, a0, 0x28
    not     a0, a0
    and     t4, t4, a0
1:
    //byte 4
    dsrl    a0, v1, 0x20
    and     a0, a0, 0xff
    bnez    a0, 1f
    nop
    //find a pass
    dli     a0, 0xff
    dsll    a0, a0, 0x20
    not     a0, a0
    and     t4, t4, a0
1:
    //byte 3
    dsrl    a0, v1, 0x18
    and     a0, a0, 0xff
    bnez    a0, 1f
    nop
    //find a pass
    dli     a0, 0xff
    dsll    a0, a0, 0x18
    not     a0, a0
    and     t4, t4, a0
1:
    //byte 2
    dsrl    a0, v1, 0x10
    and     a0, a0, 0xff
    bnez    a0, 1f
    nop
    //find a pass
    dli     a0, 0xff
    dsll    a0, a0, 0x10
    not     a0, a0
    and     t4, t4, a0
1:
    //byte 1
    dsrl    a0, v1, 0x08
    and     a0, a0, 0xff
    bnez    a0, 1f
    nop
    //find a pass
    dli     a0, 0xff
    dsll    a0, a0, 0x08
    not     a0, a0
    and     t4, t4, a0
1:
    //byte 0
    dsrl    a0, v1, 0x0
    and     a0, a0, 0xff
    bnez    a0, 1f
    nop
    //find a pass
    dli     a0, 0xff
    dsll    a0, a0, 0x0
    not     a0, a0
    and     t4, t4, a0
1:
#endif

//process TM result: translate Byte error info into 1 bit info in each BX_TM_RST of every Byte.
//64 bit BX_TM_RST work as a bit map corresponding to every param value(so the min step interval
//is 2, or there will be not enough space to store TM RST info), the 64 bit can be only part valid(
//step interval > 2).
    dli     t8, ARB_STORE_BASE
    GET_ARB_LEVEL_NODE_ID
    dsll    a1, a1, 44
    daddu   t8, t8, a1

    dsrl    t7, t3, 56
    and     t7, t7, 0xff
    beqz    t7, 1f
    nop
    //error detected
    ld      a0, B7_TM_RST(t8)
    or      a0, a0, t0
    sd      a0, B7_TM_RST(t8)
1:
    dsrl    t7, t3, 48
    and     t7, t7, 0xff
    beqz    t7, 1f
    nop
    //error detected
    ld      a0, B6_TM_RST(t8)
    or      a0, a0, t0
    sd      a0, B6_TM_RST(t8)
1:
    dsrl    t7, t3, 40
    and     t7, t7, 0xff
    beqz    t7, 1f
    nop
    //error detected
    ld      a0, B5_TM_RST(t8)
    or      a0, a0, t0
    sd      a0, B5_TM_RST(t8)
1:
    dsrl    t7, t3, 32
    and     t7, t7, 0xff
    beqz    t7, 1f
    nop
    //error detected
    ld      a0, B4_TM_RST(t8)
    or      a0, a0, t0
    sd      a0, B4_TM_RST(t8)
1:
    dsrl    t7, t3, 24
    and     t7, t7, 0xff
    beqz    t7, 1f
    nop
    //error detected
    ld      a0, B3_TM_RST(t8)
    or      a0, a0, t0
    sd      a0, B3_TM_RST(t8)
1:
    dsrl    t7, t3, 16
    and     t7, t7, 0xff
    beqz    t7, 1f
    nop
    //error detected
    ld      a0, B2_TM_RST(t8)
    or      a0, a0, t0
    sd      a0, B2_TM_RST(t8)
1:
    dsrl    t7, t3, 8
    and     t7, t7, 0xff
    beqz    t7, 1f
    nop
    //error detected
    ld      a0, B1_TM_RST(t8)
    or      a0, a0, t0
    sd      a0, B1_TM_RST(t8)
1:
    dsrl    t7, t3, 0
    and     t7, t7, 0xff
    beqz    t7, 1f
    nop
    //error detected
    ld      a0, B0_TM_RST(t8)
    or      a0, a0, t0
    sd      a0, B0_TM_RST(t8)
1:
    dsll    t0, t0, 1

    //check wether delay value exceed max value
    move    a1, s4
    daddu   a2, t2, t5
    bgt     a2, a1, 11f //check the new delay value whether exceed limitation
    nop
    /** not exceed **/
    move    t2, a2
    b       clklvl_test_one_delay
    nop
11:

#ifdef  DEBUG_ARB_LEVEL_CLK
    dli     t8, ARB_STORE_BASE
    GET_ARB_LEVEL_NODE_ID
    dsll    a1, a1, 44
    daddu   t8, t8, a1
    PRINTSTR("\r\nlevel result is:\r\n")
    ld      t7, B7_TM_RST(t8)
    dsrl    a0, t7, 32
    bal     hexserial
    nop
    move    a0, t7
    bal     hexserial
    nop
    PRINTSTR("\r\n")
    ld      t7, B6_TM_RST(t8)
    dsrl    a0, t7, 32
    bal     hexserial
    nop
    move    a0, t7
    bal     hexserial
    nop
    PRINTSTR("\r\n")
    ld      t7, B5_TM_RST(t8)
    dsrl    a0, t7, 32
    bal     hexserial
    nop
    move    a0, t7
    bal     hexserial
    nop
    PRINTSTR("\r\n")
    ld      t7, B4_TM_RST(t8)
    dsrl    a0, t7, 32
    bal     hexserial
    nop
    move    a0, t7
    bal     hexserial
    nop
    PRINTSTR("\r\n")
    ld      t7, B3_TM_RST(t8)
    dsrl    a0, t7, 32
    bal     hexserial
    nop
    move    a0, t7
    bal     hexserial
    nop
    PRINTSTR("\r\n")
    ld      t7, B2_TM_RST(t8)
    dsrl    a0, t7, 32
    bal     hexserial
    nop
    move    a0, t7
    bal     hexserial
    nop
    PRINTSTR("\r\n")
    ld      t7, B1_TM_RST(t8)
    dsrl    a0, t7, 32
    bal     hexserial
    nop
    move    a0, t7
    bal     hexserial
    nop
    PRINTSTR("\r\n")
    ld      t7, B0_TM_RST(t8)
    dsrl    a0, t7, 32
    bal     hexserial
    nop
    move    a0, t7
    bal     hexserial
    nop
#endif
//calculate mid value for each byte lane
/***********
boundary sign: contain at least WINDOW_ZERO_NUM consecutive 0(TM success)
    t0: parse pointer
    t1: BYTE OFFSET, and work as loop control
    t2: parse max position
    t3: BYTE_X_LEVEL_RST
    t4: WINDOW_ZERO_NUM
***********/
    //set t2 max value for each level object
    move    t2, s4
    dsrl    t2, t2, LOG2_STEP

    move    t9, $0
    dli     t8, ARB_STORE_BASE
    GET_ARB_LEVEL_NODE_ID
    dsll    a1, a1, 44
    daddu   t8, t8, a1

    dli     t1, 0x38    //start from byte 7

11: //loop for all byte lanes
    daddu   t7, t8, t1
    ld      t3, B0_TM_RST(t7)
#ifdef  DEBUG_ARB_LEVEL_CLK
    PRINTSTR("\r\nt3 = 0x")
    dsrl    a0, t3, 32
    bal     hexserial
    nop
    move    a0, t3
    bal     hexserial
    nop
#endif
    move    t0, $0
    dli     t4, WINDOW_ZERO_NUM
12:
    bgtu    t0, t2, 3f
    nop
    dsrl    t7, t3, t0
    and     t7, t7, 0x1
    bnez    t7, 1f
    nop
    //find a TM success
    daddiu  t4, t4, -1
    beqz    t4, 2f
    nop
    //continue move
    daddiu  t0, t0, 0x1
    b       12b
    nop
1:  //find a TM fail
    dli     t4, WINDOW_ZERO_NUM
    //continue move
    daddiu  t0, t0, 0x1
    b       12b
    nop
2:  //window found
    //calculate the MIN boundary
    dli     a1, WINDOW_ZERO_NUM
    daddiu  a0, t0, 1
    dsubu   a0, a0, a1
    dsll    a0, a0, LOG2_STEP   //a0 = a0 * 2n
    and     a0, a0, 0xff
    dsll    a0, a0, t1
    ld      a1, GD_MIN(t8)
    or      a1, a1, a0
    sd      a1, GD_MIN(t8)
    //move forward to the next Fail to cal the MAX boundary
1:
    daddiu  t0, t0, 0x1
    bgtu    t0, t2, 1f
    nop
    dsrl    t7, t3, t0
    and     t7, t7, 0x1
    beqz    t7, 1b  //continue move
    nop
1:
    //find a TM FAIL or reach the max test value
    daddiu  a0, t0, -1
    dsll    a0, a0, LOG2_STEP   //a0 = a0 * 2n
    and     a0, a0, 0xff
    dsll    a0, a0, t1
    ld      a1, GD_MAX(t8)
    or      a1, a1, a0
    sd      a1, GD_MAX(t8)
    b       2f
    nop
3:  //parse to end, CAN NOT find a window
    or      t9, t9, 0x1
#ifdef  PRINT_MSG
    PRINTSTR("\r\nWrlvl Error: This Byte Window not found.")
    PRINTSTR("\r\nFailed byte is byte: ")
    dsrl    a0, t1, 3
    bal     hexserial
    nop
#endif
2:  //continue for next byte lane
    beqz    t1, 2f
    nop
    daddu   t1, t1, -0x8
    b       11b
    nop
2:  //All byte lane's MIN and MAX value stored or fail to find
    beqz    t9, 1f
    nop
    //some Byte lane can not find a window
#ifdef  PRINT_MSG
    PRINTSTR("\r\nWrite level failed. Write default value.\r\n")
#endif
    //write standard value mandatory
    //clklvl_delay
    dli     a0, CLKLVL_DEFAULT_VALUE
    sd      a0, GD_MID(t8)
    b       arb_clklvl_value_caled
    nop
1:
#ifdef  PRINT_MSG
    PRINTSTR("\r\nMin value: 0x")
    ld      t7, GD_MIN(t8)
    dsrl    a0, t7, 32
    bal     hexserial
    nop
    move    a0, t7
    bal     hexserial
    nop
    PRINTSTR("\r\nMax value: 0x")
    ld      t7, GD_MAX(t8)
    dsrl    a0, t7, 32
    bal     hexserial
    nop
    move    a0, t7
    bal     hexserial
    nop
#endif

//calculate mid value for each byte lane
    ld      a0, GD_MIN(t8)
    ld      a1, GD_MAX(t8)
    daddu   a0, a0, a1
    //divide a0 by 2 every byte
    dsrl    a0, a0, 1
    dli     a1, 0x7f7f7f7f7f7f7f7f
    and     a0, a0, a1
    sd      a0, GD_MID(t8)
#ifdef  PRINT_MSG
    PRINTSTR("\r\nCal Mid value: 0x")
    ld      t7, GD_MID(t8)
    dsrl    a0, t7, 32
    bal     hexserial
    nop
    move    a0, t7
    bal     hexserial
    nop
#endif

arb_clklvl_value_caled:
    ld      t2, GD_MID(t8)
#ifdef DEBUG_ARB_LEVEL_CLK
    PRINTSTR("\r\nWrite param and value:\r\nt2 = 0x")
    dsrl    a0, t2, 32
    bal     hexserial
    nop
    move    a0, t2
    bal     hexserial
    nop
#endif
    dli     t2, 0x2020202020202020
    dli     t6, 0x1
    bal     arb_write_param
    nop

#ifdef DEBUG_ARB_LEVEL_CLK
    PRINTSTR("\r\nAfter clock leveling. The MC configuration is:\r\n")

    bal     enable_ddr_confspace
    nop

    dli     t1, DDR_PARAM_NUM
    dli     t7, DDR_MC_CONFIG_BASE
    GET_ARB_LEVEL_NODE_ID
    dsll    a1, a1, 44
    or      t7, t7, a1
1:
    ld      t3, 0x0(t7)
    dsrl    a0, t3, 32
    bal     hexserial
    nop
    PRINTSTR("  ")
    move    a0, t3
    bal     hexserial
    nop
    PRINTSTR("\r\n")

    daddiu  t1, t1, -1
    daddiu  t7, t7, 16
    bnez    t1, 1b
    nop

    bal     disable_ddr_confspace
    nop
#endif
#endif

#include    "ARB_Write_Slice_8_param.S"

    //print final level result
#ifdef  PRINT_MSG
    PRINTSTR("\r\nAfter ARB level. The MC configuration is:\r\n")
    bal     enable_ddr_confspace
    nop

    dli     t1, DDR_PARAM_NUM
    dli     t7, DDR_MC_CONFIG_BASE
    GET_ARB_LEVEL_NODE_ID
    dsll    a1, a1, 44
    or      t7, t7, a1
1:
    ld      t3, 0x0(t7)
    and     a0, t7, 0xfff
    bal     hexserial
    nop
    PRINTSTR(":  ")
    dsrl    a0, t3, 32
    bal     hexserial
    nop
    PRINTSTR("  ")
    move    a0, t3
    bal     hexserial
    nop
    PRINTSTR("\r\n")

    daddiu  t1, t1, -1
    daddiu  t7, t7, 16
    bnez    t1, 1b
    nop

    bal     disable_ddr_confspace
    nop
#endif

ARB_level_end:
    bal     arb_test_mem
    nop
    beqz    v0, 2f
    nop
    move    t3, v0
    move    t4, v1
#ifdef  PRINT_MSG
    PRINTSTR("\r\n\r\nERROR!!!: ARB Leveling Failed.\r\n")
    PRINTSTR("\r\nRW Diff 0x")
    dsrl    a0, t3, 32
    bal     hexserial
    nop
    move    a0, t3
    bal     hexserial
    nop
    PRINTSTR("\r\nRD Diff 0x")
    dsrl    a0, t4, 32
    bal     hexserial
    nop
    move    a0, t4
    bal     hexserial
    nop
#endif
#if 0   //seems can not help
    //level fail, delay some time and level again
    dli     a0, 0x4000000
1:
    daddiu  a0, a0, -1
    bnez    a0, 1b
    nop
    b       ARB_level_begin
    nop
#endif
    //not     t7, $0
    b       arb_level_fail
    nop
2:
#ifdef  PRINT_MSG
    PRINTSTR("\r\n\r\nARB Leveling Finished.\r\n")
#endif

    //move    t7, $0

arb_level_fail:

//recover t0~t9,s1~s7
    dli     a2, ARB_STACK_BASE
    GET_ARB_LEVEL_NODE_ID
    dsll    a1, a1, 44
    daddu   a2, a2, a1
    ld      s0, 0x0(a2)
    ld      s1, 0x8(a2)
    ld      s2, 0x10(a2)
    ld      s3, 0x18(a2)
    ld      s4, 0x20(a2)
    ld      s5, 0x28(a2)
    ld      s6, 0x30(a2)
    ld      s7, 0x38(a2)
    ld      t0, 0x40(a2)
    ld      t1, 0x48(a2)
    ld      t2, 0x50(a2)
    ld      t3, 0x58(a2)
    ld      t4, 0x60(a2)
    ld      t5, 0x68(a2)
    ld      t6, 0x70(a2)
    ld      t7, 0x78(a2)
    ld      t8, 0x80(a2)
    ld      t9, 0x88(a2)

/*
 *Unlock Scache 9800?01000000000 ~ 9800?01000001000(4K)
 */
#ifdef  PRINT_MSG
    PRINTSTR("\r\nUnlock Scache Node x--9800?01000000000~4K...\r\n")
#endif
    dli     a2, LOCK_SCACHE_CONFIG_BASE_ADDR
#ifdef LS3B
    GET_ARB_LEVEL_NODE_ID
    dsll    a1, a1, 14
    daddu   a2, a2, a1
#endif
    GET_ARB_LEVEL_NODE_ID
    dsll    a1, a1, 44
    or      a2, a2, a1
    sd      $0, 0x0(a2)
    sd      $0, 0x40(a2)
    sync

    //Hit Invalidate the locked address
    dli     a0, 0x9800001000000000
    GET_ARB_LEVEL_NODE_ID
    dsll    a1, a1, 44
    or      a0, a0, a1
    dli     a1, 0x1000
    daddu   a2, a0, a1
1:
    cache   0x11, 0x0(a0)
    cache   0x13, 0x0(a0)

    daddu   a0, a0, 0x20
    blt     a0, a2, 1b
    nop

#ifdef  PRINT_MSG
    PRINTSTR("Unlock Scache Done.\r\n")
#endif

    move    ra, t9
    jr      ra
    nop

//<<<<<<<<<<<<<<<<<<<<<<<
//child function defination

enable_ddr_confspace:
/*
 * enable DDR MC register config space
 * use register: t7
 */
#if 0
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
#endif
	li t7, 0xbfe10420
    	li a2, 0x100
	not a2,a2
	lw a1, 0x4(t7)
    	and     a1, a1, a2
   	sw      a1, 0x0(t7)
    	sync
    jr      ra
    nop

disable_ddr_confspace:
/*
 * disable DDR MC config reg space
 * use register: t7
 */
#if 0
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
#endif
	li t7, 0xbfe10420
    	li a2, 0x100
	lw a1, 0x4(t7)
    	or     a1, a1, a2
   	sw      a1, 0x0(t7)
    	sync

    jr      ra
    nop

#include "ARB_Modify_param_func.S"
#include "ARB_Write_param_func.S"
#include "ARB_TM.S"
//>>>>>>>>>>>>>>>>>>>>>
