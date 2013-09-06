/************************************
    Function:   Access Result Based Software leveling
    Author: Chen Xinke
    v2.2    Modify the modify param routine
    v2.0    Multi Node support
    v1.x    Single Node mode
    v1.1    Raw code for LS3A3
    v0.2    used for test
    
note: don't use s0, because it will be use at some subroutine
use register: a0,a1,a2,v0,v1, t9

register usage:
s3: slice select.
s5: Level sequence pointer.
t0: RST store bit pointer during TM process;
    RST parse pointer during result process.
t1: 
t2: currently used delay value.
t3, t4: variables
t5: delay value interval.
t6: param to Modify_param.
t7: volatile value
t8: ARB_STORE_BASE
t9: one level result: 0: success; 1: Fail.
v0, v1: return value of arb_test_mem.
algrithm: 

***************************************/
/********************************
********************************/
#include "ARB_level.h"

//#define DEBUG_ARB_LEVEL

#define ADD_DELAY_AFTER_RESET_PHY   //necessary
#ifndef LS2HMC
#define CLEAR_HALF_CLK_SHIFT    //necessary
#endif

//Don't change
#define CONTROL_L2XBAR_DDR_WINDOW  //work ok
#define ARBLVL_PUT_DRAM_SREF
#define LOG2_STEP   1   //log2 of TM step interval, remember to small WINDOW_ZERO_NUM when we use larger STEP
#define WINDOW_ZERO_NUM     0x8
#define GATE_MINUS  -4

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

#ifdef  PRINT_MSG
    PRINTSTR("\r\nStart ARB Leveling....\r\n")
#endif

//leveling for Delay line setting.
One_ARB_level_begin:
    dli     t8, ARB_STORE_BASE
    GET_ARB_LEVEL_NODE_ID
    dsll    a1, a1, 44
    daddu   t8, t8, a1
    //initialize to avoid dead loop
    dli     a0, 0x0
    sd      a0, LEVEL_SEQ_BASE(t8)
    //set level sequence--byte 0 mark the end
#ifdef  DDR3_DIMM
    dli     a0, 0x0000020502050205
    sd      a0, LEVEL_SEQ_BASE(t8)
    daddiu  t8, t8, 0x8
    dli     a0, 0x00
#else
    dli     a0, 0x04030204030205
    sd      a0, LEVEL_SEQ_BASE(t8)
    daddiu  t8, t8, 0x8
    dli     a0, 0x0000
#endif
    sd      a0, LEVEL_SEQ_BASE(t8)
    daddiu  t8, t8, 0x8
    dli     a0, 0
    sd      a0, LEVEL_SEQ_BASE(t8)

    dli     s3, 0xf //change all 8 byte slices
    move    s5, $0

21:
//one level begin
    dli     t8, ARB_STORE_BASE
    GET_ARB_LEVEL_NODE_ID
    dsll    a1, a1, 44
    daddu   t8, t8, a1
    daddu   a0, t8, s5
    lb      a1, LEVEL_SEQ_BASE(a0)
    move    t6, a1
    beqz    t6, One_ARB_level_end
    nop

#ifdef  PRINT_MSG
    PRINTSTR("\r\nt6 = 0x")
    move    a0, t6
    bal     hexserial
    nop
    PRINTSTR("\r\n")
#endif

    //set t2 start value
    dli     t0, 0x1

    dli     a3, 0x1
    beq     t6, a3, 1f
    nop
    daddiu  a3, a3, 0x1
    beq     t6, a3, 2f
    nop
    daddiu  a3, a3, 0x1
    beq     t6, a3, 3f
    nop
    daddiu  a3, a3, 0x1
    beq     t6, a3, 4f
    nop
    daddiu  a3, a3, 0x1
    beq     t6, a3, 5f
    nop
    daddiu  a3, a3, 0x1
    beq     t6, a3, 6f
    nop
    b       10f
    nop

1:
    //1 clk max delay
    dli     t2, CLKLVL_MAX_DELAY
    b       10f
    nop
2:
    //2 rdlvl_gate max delay
    dli     t2, RDLVL_GATE_MAX_DELAY
    b       10f
    nop
3:
4:
    //3,4 rdlvl_dqs and rdlvl_dqsn max delay
    dli     t2, RDLVL_MAX_DELAY
    b       10f
    nop
5:
    //5 wrlvl_dqs max delay
    dli     t2, WRLVL_MAX_DELAY
    b       10f
    nop
6:
    //6 wrlvl_dq max delay
    dli     t2, WRLVL_DQ_MAX_DELAY
    b       10f
    nop

10:
    dsrl    a0, t2, LOG2_STEP
    dsll    t0, t0, a0

    dli     t5, 1 << LOG2_STEP 
    //initialize
    dli     t3, 0x0
    dli     t4, 0x0
    dli     t9, 0x0
    //clear store mem
    dli     t8, ARB_STORE_BASE
    GET_ARB_LEVEL_NODE_ID
    dsll    a1, a1, 44
    daddu   t8, t8, a1
    dli     a0, 0x0
    sd      a0, 0x0(t8)
    sd      a0, 0x8(t8)
    sd      a0, 0x10(t8)
    sd      a0, 0x18(t8)
    sd      a0, 0x20(t8)
    sd      a0, 0x28(t8)
    sd      a0, 0x30(t8)
    sd      a0, 0x38(t8)
    sd      a0, 0x40(t8)
    sd      a0, 0x48(t8)
    sd      a0, 0x50(t8)
31:
//write new delay value
    bal     arb_modify_param
    nop

//do Test and print test result
    bal     arb_test_mem
    nop
    move    t3, v0
    move    t4, v1
#ifdef  DEBUG_ARB_LEVEL
    PRINTSTR("\r\nt2 = 0x")
    move    a0, t2
    bal     hexserial
    nop
    PRINTSTR(":")
#endif
#ifdef  DEBUG_ARB_LEVEL
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
#if 1
//process TM result: translate Byte error info into 1 bit info in each BX_TM_RST of every Byte.
//64 bit BX_TM_RST work as a bit map corresponding to every param value(so the min step interval
//is 2, or there will be not enough space to store TM RST info), the 64 bit can be only part valid(
//step interval > 2).
#if 0
    //for level--1 ~ 4, use RD Error info
    dli     a0, 0x4
    bgtu    t6, a0, 1f
    nop
    move    t3, t4
1:
#endif
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
#endif
    //check wether delay value exceed max value
    dsubu   a2, t2, t5
    bltz    a2, 11f //check the new delay value whether exceed limitation
    nop
    /** not exceed **/
    move    t2, a2
    b       31b
    nop
11: 
#ifdef  DEBUG_ARB_LEVEL
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
    PRINTSTR("\r\n")
    PRINTSTR("\r\nt2 = 0x")
    dsrl    a0, t2, 32
    bal     hexserial
    nop
    move    a0, t2
    bal     hexserial
    nop
    PRINTSTR("\r\n")
#endif
#if 1
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
    dli     a3, 0x1
    beq     t6, a3, 1f
    nop
    daddiu  a3, a3, 0x1
    beq     t6, a3, 2f
    nop
    daddiu  a3, a3, 0x1
    beq     t6, a3, 3f
    nop
    daddiu  a3, a3, 0x1
    beq     t6, a3, 4f
    nop
    daddiu  a3, a3, 0x1
    beq     t6, a3, 5f
    nop
    daddiu  a3, a3, 0x1
    beq     t6, a3, 6f
    nop
    b       10f
    nop

1:
    //1 clk max delay
    dli     t2, CLKLVL_MAX_DELAY
    b       10f
    nop
2:
    //2 rdlvl_gate max delay
    dli     t2, RDLVL_GATE_MAX_DELAY
    b       10f
    nop
3:
4:
    //3,4 rdlvl_dqs and rdlvl_dqsn max delay
    dli     t2, RDLVL_MAX_DELAY
    b       10f
    nop
5:
    //5 wrlvl_dqs max delay
    dli     t2, WRLVL_MAX_DELAY
    b       10f
    nop
6:
    //6 wrlvl_dq max delay
    dli     t2, WRLVL_DQ_MAX_DELAY
    b       10f
    nop
10:
    dsrl    t2, t2, LOG2_STEP

    dli     t1, 0x38    //start from byte 7
    dli     t8, ARB_STORE_BASE
    GET_ARB_LEVEL_NODE_ID
    dsll    a1, a1, 44
    daddu   t8, t8, a1
11: //loop for 8 byte lane
    daddu   a1, t8, t1
    ld      t3, B0_TM_RST(a1)
#ifdef  DEBUG_ARB_LEVEL
    PRINTSTR("t3 = 0x")
    dsrl    a0, t3, 32
    bal     hexserial
    nop
    move    a0, t3
    bal     hexserial
    nop
    PRINTSTR("\r\n")
#endif
    dli     t0, 0x0
    dli     t4, WINDOW_ZERO_NUM
    dli     a0, 0x2
    bne     t6, a0, 9f
    nop
    daddiu  t4, t4, GATE_MINUS   //for rdlvl_gate, use small windows
9:
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
    dli     a0, 0x2
    bne     t6, a0, 9f
    nop
    daddiu  t4, t4, GATE_MINUS   //for rdlvl_gate, use small windows
9:
    //continue move
    daddiu  t0, t0, 0x1
    b       12b
    nop
2:  //window found
//<<<<<<<<<<<<<<<
    //calculate the MIN boundary
    dli     a1, WINDOW_ZERO_NUM
    dli     a0, 0x2
    bne     t6, a0, 9f
    nop
    daddiu  a1, a1, GATE_MINUS   //for rdlvl_gate, use small windows
9:
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
//>>>>>>>>>>>>>>>>>
3:  //parse to end, CAN NOT find a window
    or      t9, t9, 0x1
#ifdef  PRINT_MSG
    PRINTSTR("Error: This Byte Window not found\r\n")
    PRINTSTR("Failed byte is byte ")
    dsrl    a0, t1, 3
    bal     hexserial
    nop
    PRINTSTR("\r\n")
    PRINTSTR("t3 = 0x")
    dsrl    a0, t3, 32
    bal     hexserial
    nop
    move    a0, t3
    bal     hexserial
    nop
    PRINTSTR("\r\n")
#endif
2:  //continue for next byte lane
    beqz    t1, 2f
    nop
    daddiu  t1, t1, -0x8
    b       11b
    nop
2:  //All byte lane's MIN and MAX value stored or fail to find
    beqz    t9, 1f
    nop
    //some Byte lane can not find a window
#ifdef  PRINT_MSG
    PRINTSTR("\r\nThis level failed.\r\n")
#endif
#if 0
    //delay some time and re-level this delay
    dli     a0, 0x40000000
9:
    daddiu  a0, a0, -1
    bnez    a0, 9b
    nop

    b       21b
    nop
#endif
    //write standard value mandatory
    //rdlvl_gate_delay
    dli     a0, 0x0
    dli     a1, 0x2
    beq     t6, a1, 2f
    nop
    //clk delay or wrlvl_delay
    dli     a0, 0x4040404040404040
    dli     a1, 0x1
    beq     t6, a1, 2f
    nop
    dli     a1, 0x5
    beq     t6, a1, 2f
    nop
    //rdlvl_dqsP/N delay or wrlvl_dq delay
    dsrl    a0, a0, 1   //dli     a0, 0x2020202020202020
2:
    sd      a0, GD_MID(t8)
    b       One_Level_Caled
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
#if 1   //special deal with rdlvl_gate
    dli     a2, 0x2
    bne     t6, a2, 2f
    nop
#if 1
    //rdlvl gate use (max + mid value) / 2.
    ld      a1, GD_MAX(t8)
    daddu   a0, a0, a1
    dsrl    a0, a0, 1
    dli     a1, 0x7f7f7f7f7f7f7f7f
    and     a0, a0, a1
#endif
#if 1
    //if leveled rdlvl gate <= 8, use 0
    //minus as bigger as 7.
    dli     a1, 0xf8f8f8f8f8f8f8f8
    and     a0, a0, a1
#endif
2:
#endif
#if 0  //special deal with wrlvl_dq
    //add some offset for wrlvl_dq
    dli     a1, 0x6
    bne     t6, a1, 2f
    nop
    dli     a2, 0x0202020202020202
    daddu   a0, a0, a2
2:
#endif
    sd      a0, GD_MID(t8)
    dli     a0, 0x1
    bne     t6, a0, 2f
    nop
//seperately take care of clk because 1 clk control multiple SDRAM
    dli     a0, 0x0
    sd      a0, GD_MID(t8)
#ifdef  DDR3_DIMM
#if 0
//use avg of avg(byte 0~7)
//not finished!!!
    ld      a0, GD_MID(t8)
    dli     a1, 0x0
    dsrl    a2, a0, 0x0
    and     a2, a2, 0xff
    daddu   a1, a1, a2
    dsrl    a2, a0, 0x8
    and     a2, a2, 0xff
    daddu   a1, a1, a2
    dsrl    a2, a0, 0x10
    and     a2, a2, 0xff
    daddu   a1, a1, a2
    dsrl    a2, a0, 0x18
    and     a2, a2, 0xff
    daddu   a1, a1, a2
    dsrl    a2, a0, 0x20
    and     a2, a2, 0xff
    daddu   a1, a1, a2
    dsrl    a2, a0, 0x28
    and     a2, a2, 0xff
    daddu   a1, a1, a2
    dsrl    a2, a0, 0x30
    and     a2, a2, 0xff
    daddu   a1, a1, a2
    dsrl    a2, a0, 0x38
    and     a2, a2, 0xff
    daddu   a1, a1, a2

    dsrl    a1, a1, 3   //a1=a1/8
#else
//use Max(Min Byte0~7) and Min(Max Byte0~7)
    ld      a0, GD_MIN(t8)
    dsrl    a1, a0, 0x38
    and     a1, a1, 0xff
    dsrl    a2, a0, 0x30
    and     a2, a2, 0xff
    //a1=MAX(a1, a2)
    bgtu    a1, a2, 1f
    nop
    move    a1, a2
1:
    dsrl    a2, a0, 0x28
    and     a2, a2, 0xff
    //a1=MAX(a1, a2)
    bgtu    a1, a2, 1f
    nop
    move    a1, a2
1:  
    dsrl    a2, a0, 0x20
    and     a2, a2, 0xff
    //a1=MAX(a1, a2)
    bgtu    a1, a2, 1f
    nop
    move    a1, a2
1:  
    dsrl    a2, a0, 0x18
    and     a2, a2, 0xff
    //a1=MAX(a1, a2)
    bgtu    a1, a2, 1f
    nop
    move    a1, a2
1:  
    dsrl    a2, a0, 0x10
    and     a2, a2, 0xff
    //a1=MAX(a1, a2)
    bgtu    a1, a2, 1f
    nop
    move    a1, a2
1:  
    dsrl    a2, a0, 0x8
    and     a2, a2, 0xff
    //a1=MAX(a1, a2)
    bgtu    a1, a2, 1f
    nop
    move    a1, a2
1:  
    dsrl    a2, a0, 0x0
    and     a2, a2, 0xff
    //a1=MAX(a1, a2)
    bgtu    a1, a2, 1f
    nop
    move    a1, a2
1:  
    move    t7, a1
    
    ld      a0, GD_MAX(t8)
    dsrl    a1, a0, 0x38
    and     a1, a1, 0xff
    dsrl    a2, a0, 0x30
    and     a2, a2, 0xff
    //a1=MIN(a1, a2)
    bltu    a1, a2, 1f
    nop
    move    a1, a2
1:
    dsrl    a2, a0, 0x28
    and     a2, a2, 0xff
    //a1=MIN(a1, a2)
    bltu    a1, a2, 1f
    nop
    move    a1, a2
1:  
    dsrl    a2, a0, 0x20
    and     a2, a2, 0xff
    //a1=MIN(a1, a2)
    bltu    a1, a2, 1f
    nop
    move    a1, a2
1:  
    dsrl    a2, a0, 0x18
    and     a2, a2, 0xff
    //a1=MIN(a1, a2)
    bltu    a1, a2, 1f
    nop
    move    a1, a2
1:  
    dsrl    a2, a0, 0x10
    and     a2, a2, 0xff
    //a1=MIN(a1, a2)
    bltu    a1, a2, 1f
    nop
    move    a1, a2
1:  
    dsrl    a2, a0, 0x8
    and     a2, a2, 0xff
    //a1=MIN(a1, a2)
    bltu    a1, a2, 1f
    nop
    move    a1, a2
1:  
    dsrl    a2, a0, 0x0
    and     a2, a2, 0xff
    //a1=MIN(a1, a2)
    bltu    a1, a2, 1f
    nop
    move    a1, a2
1:  

    daddu   a1, a1, t7 
    dsrl    a1, a1, 1   //a1=a1/2
#endif

    dsll    a2, a1, 0x18
    or      a0, a1, a2
    dsll    a2, a1, 0x28
    or      a0, a0, a2

    sd      a0, GD_MID(t8)
#else
    //clk 2--start
    ld      a0, GD_MIN(t8)
    dsrl    a1, a0, 0x38
    and     a1, a1, 0xff
    dsrl    a2, a0, 0x30
    and     a2, a2, 0xff
    //a1=MAX(a1, a2)
    bgtu    a1, a2, 1f
    nop
    move    a1, a2
1:
    dsrl    a2, a0, 0x28
    and     a2, a2, 0xff
    //a1=MAX(a1, a2)
    bgtu    a1, a2, 1f
    nop
    move    a1, a2
1:  
    move    t7, a1
    
    ld      a0, GD_MAX(t8)
    dsrl    a1, a0, 0x38
    and     a1, a1, 0xff
    dsrl    a2, a0, 0x30
    and     a2, a2, 0xff
    //a1=MIN(a1, a2)
    bltu    a1, a2, 1f
    nop
    move    a1, a2
1:
    dsrl    a2, a0, 0x28
    and     a2, a2, 0xff
    //a1=MIN(a1, a2)
    bltu    a1, a2, 1f
    nop
    move    a1, a2
1:  
    daddu   t7, t7, a1
    dsrl    t7, t7, 1   //t7=t7/2
    dsll    t7, t7, 0x28
    ld      a1, GD_MID(t8)
    or      a1, a1, t7
    sd      a1, GD_MID(t8)
    //clk 2--end
    //clk 1--start
    ld      a0, GD_MIN(t8)
    dsrl    a1, a0, 0x20
    and     a1, a1, 0xff
    dsrl    a2, a0, 0x18
    and     a2, a2, 0xff
    //a1=MAX(a1, a2)
    bgtu    a1, a2, 1f
    nop
    move    a1, a2
1:
    move    t7, a1
    
    ld      a0, GD_MAX(t8)
    dsrl    a1, a0, 0x20
    and     a1, a1, 0xff
    dsrl    a2, a0, 0x18
    and     a2, a2, 0xff
    //a1=MIN(a1, a2)
    bltu    a1, a2, 1f
    nop
    move    a1, a2
1:
    daddu   t7, t7, a1
    dsrl    t7, t7, 1
    dsll    t7, t7, 0x18
    ld      a1, GD_MID(t8)
    or      a1, a1, t7
    sd      a1, GD_MID(t8)
    //clk 1--end
    //clk 0--start
    ld      a0, GD_MIN(t8)
    dsrl    a1, a0, 0x10
    and     a1, a1, 0xff
    dsrl    a2, a0, 0x8
    and     a2, a2, 0xff
    //a1=MAX(a1, a2)
    bgtu    a1, a2, 1f
    nop
    move    a1, a2
1:
    dsrl    a2, a0, 0x0
    and     a2, a2, 0xff
    //a1=MAX(a1, a2)
    bgtu    a1, a2, 1f
    nop
    move    a1, a2
1:  
    move    t7, a1
    
    ld      a0, GD_MAX(t8)
    dsrl    a1, a0, 0x10
    and     a1, a1, 0xff
    dsrl    a2, a0, 0x8
    and     a2, a2, 0xff
    //a1=MIN(a1, a2)
    bltu    a1, a2, 1f
    nop
    move    a1, a2
1:
    dsrl    a2, a0, 0x0
    and     a2, a2, 0xff
    //a1=MIN(a1, a2)
    bltu    a1, a2, 1f
    nop
    move    a1, a2
1:  
    daddu   t7, t7, a1
    dsrl    t7, t7, 1
    dsll    t7, t7, 0x0
    ld      a1, GD_MID(t8)
    or      a1, a1, t7
    sd      a1, GD_MID(t8)
    //clk 0--end
#endif

2:
#ifdef  PRINT_MSG
    PRINTSTR("\r\nCal Mid value: 0x")
    ld      t7, GD_MID(t8)
    dsrl    a0, t7, 32
    bal     hexserial
    nop
    move    a0, t7
    bal     hexserial
    nop
    PRINTSTR("\r\n")
#endif
One_Level_Caled:
    ld      t2, GD_MID(t8)
#ifdef DEBUG_ARB_LEVEL
    PRINTSTR("\r\nWrite param and value:\r\nt6 = 0x")
    dsrl    a0, t6, 32
    bal     hexserial
    nop
    move    a0, t6
    bal     hexserial
    nop
    PRINTSTR("\r\nt2 = 0x")
    dsrl    a0, t2, 32
    bal     hexserial
    nop
    move    a0, t2
    bal     hexserial
    nop
    PRINTSTR("\r\n")
#endif
#include    "ARB_Write_Leveled_param.S"
#endif

    //move to next level
    daddiu  s5, s5, 0x1
    b       21b
    nop

One_ARB_level_end:
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
2:

#ifdef  PRINT_MSG
    PRINTSTR("\r\nARB Leveling Finished.\r\n")
#endif

    //print final level result
#ifdef  PRINT_MSG
    PRINTSTR("After ARB level. The MC configuration is:\r\n")
    bal     enable_ddr_confspace
    nop

    dli     t1, DDR_PARAM_NUM
    dli     t7, DDR_MC_CONFIG_BASE
    GET_ARB_LEVEL_NODE_ID
    dsll    a1, a1, 44
    or      t7, t7, a1
1:
    ld      t6, 0x0(t7)
    dsrl    a0, t6, 32
    bal     hexserial
    nop
    PRINTSTR("  ")
    move    a0, t6
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

//<<<<<<<<<<<<<<<<<<<<<

enable_ddr_confspace:
/*
 * enable DDR MC register config space
 * use register: t7
 */
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

    jr      ra
    nop
    
disable_ddr_confspace:    
/*
 * disable DDR MC config reg space
 * use register: t7
 */
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

    jr      ra
    nop

#include "ARB_Modify_param_func.S"
#include "ARB_TM.S"
//>>>>>>>>>>>>>>>>>>>>>
