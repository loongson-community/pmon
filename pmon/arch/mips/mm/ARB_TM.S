/********************
Author: Chen Xinke
Function: Test memory read and write errors
note :  Memory size >= 1G
Usage:  include this file in the start.S, after initialize cache and before copy Pmon Text to the memory;
v1.2    Test address range is auto-configured by msize(use the default window)
        precondition:
        1. default L2-Xbar window(fullmsize ~ fullmsize * 2) must be configured.
v2.0    Support Multi-chip mode memory test and new msize map, Node ID is given by user input.
v2.2    Modify the structure of the program to reduce its size and make it more scalable.
v2.4    Modify code for ARB level use, remove some no use code.
********************/
/***************
note: don't change s0, s2(msize)
use register from caller view: a0~a3, v0,v1

register usage:
t0: test pattern content
t1: current address
t2: address interval
t3: max address
t4,t5: volatile
t6: test pattern base
t7: by subroutine--arb_hexserial64
t8: control print error num(debug use). 
t9: loop control
s5: store level byte mask
s6: error bits record
s7: RD error bits record
v0: output result(s6)
v1: output result(s7)
**************/
#include    "ARB_TM.h"

//#define ARB_TM_DBG
#ifdef  ARB_TM_DBG
#define ARB_TM_PRINTSTR(x) \
    .rdata;98: .asciz x; .text; la a0, 98b; bal stringserial; nop
#else
#define ARB_TM_PRINTSTR(x) ;
#endif

#define  ARB_TM_FLUSH_CACHE

/********************************
 * test_mem
 * output:  v0, v1
********************************/
arb_test_mem:

ARB_TM_start:
//save t0~t9,s1~s7
    dli     a2, MT_STACK_BASE
    GET_TM_NODE_ID_a1
    dsll    a1, a1, 44
    or      a2, a2, a1
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
    sd      ra, 0x90(a2)
     
#ifdef  LEVEL_SPECIFIED_BYTE_LANES
    //give the specified byte lanes directly.
    dli     s5, LEVEL_BYTES_MASK
#endif
    dli     s6, 0x0
    dli     s7, 0x0

    //set TM start addr
    //row + col pin num
    GET_ROW_SIZE
    move    t1, a1
    GET_COL_SIZE
    daddu   t1, t1, a1
    dli     a0, ROW_COL_UPPER_LIMIT
    dsubu   t1, a0, t1
    //bank addr pin num
    dli     a0, 0x2
    GET_EIGHT_BANK
    daddu   a0, a0, a1
    daddu   t1, t1, a0
    //data width
    dli     a0, 0x3
    GET_DIMM_WIDTH
    dsubu   a0, a0, a1
    daddu   t1, t1, a0
    //set 1 rank memsize
    dli     a0, 0x1
    dsll    s4, a0, t1
    //for loop control
    dsrl    s4, s4, 0x1

    GET_MC_CS_MAP
    move    s3, a1
    
ARB_TM_begin:
    beqz    s3, ARB_TM_end
    nop
1:
    and     a1, s3, 0x1
    bnez    a1, 1f
    nop
    dsrl    s3, s3, 1
    b       1b
    nop
1:
    dsrl    s3, s3, 1

    dsll    s4, s4, 0x1

#ifdef ARB_TM_DBG
    ARB_TM_PRINTSTR("\r\ns4= 0x")
    dsrl    a0, s4, 32
    bal     hexserial
    nop
    move    a0, s4
    bal     hexserial
    nop
    ARB_TM_PRINTSTR("\r\n")
#endif

    ARB_TM_PRINTSTR("\r\nStart Testing Memory...\r\n")

#if 1
    //initialization
    dli     t9, 0
    //set Test Pattern Base t6 and write content
    dli     t6, MT_PATTERN_BASE
    GET_TM_NODE_ID_a1
    dsll    a1, a1, 44
    or      t6, t6, a1

    dli     t4, PATTERN_D8_0_0
    sd      t4, 0x0(t6)
    dli     t4, PATTERN_D8_0_1
    sd      t4, 0x8(t6)
    dli     t4, PATTERN_D8_0_2
    sd      t4, 0x10(t6)
    dli     t4, PATTERN_D8_0_3
    sd      t4, 0x18(t6)
    dli     t4, PATTERN_D8_0_4
    sd      t4, 0x20(t6)
    dli     t4, PATTERN_D8_0_5
    sd      t4, 0x28(t6)
    dli     t4, PATTERN_D8_0_6
    sd      t4, 0x30(t6)
    dli     t4, PATTERN_D8_0_7
    sd      t4, 0x38(t6)

    daddu   t6, t6, 0x40
    dli     t4, PATTERN_D8_1_0
    sd      t4, 0x0(t6)
    dli     t4, PATTERN_D8_1_1
    sd      t4, 0x8(t6)
    dli     t4, PATTERN_D8_1_2
    sd      t4, 0x10(t6)
    dli     t4, PATTERN_D8_1_3
    sd      t4, 0x18(t6)
    dli     t4, PATTERN_D8_1_4
    sd      t4, 0x20(t6)
    dli     t4, PATTERN_D8_1_5
    sd      t4, 0x28(t6)
    dli     t4, PATTERN_D8_1_6
    sd      t4, 0x30(t6)
    dli     t4, PATTERN_D8_1_7
    sd      t4, 0x38(t6)
    
    daddu   t6, t6, 0x40
    dli     t4, PATTERN_D8_2_0
    sd      t4, 0x0(t6)
    dli     t4, PATTERN_D8_2_1
    sd      t4, 0x8(t6)
    dli     t4, PATTERN_D8_2_2
    sd      t4, 0x10(t6)
    dli     t4, PATTERN_D8_2_3
    sd      t4, 0x18(t6)
    dli     t4, PATTERN_D8_2_4
    sd      t4, 0x20(t6)
    dli     t4, PATTERN_D8_2_5
    sd      t4, 0x28(t6)
    dli     t4, PATTERN_D8_2_6
    sd      t4, 0x30(t6)
    dli     t4, PATTERN_D8_2_7
    sd      t4, 0x38(t6)
    
    daddu   t6, t6, 0x40
    dli     t4, PATTERN_D8_3_0
    sd      t4, 0x0(t6)
    dli     t4, PATTERN_D8_3_1
    sd      t4, 0x8(t6)
    dli     t4, PATTERN_D8_3_2
    sd      t4, 0x10(t6)
    dli     t4, PATTERN_D8_3_3
    sd      t4, 0x18(t6)
    dli     t4, PATTERN_D8_3_4
    sd      t4, 0x20(t6)
    dli     t4, PATTERN_D8_3_5
    sd      t4, 0x28(t6)
    dli     t4, PATTERN_D8_3_6
    sd      t4, 0x30(t6)
    dli     t4, PATTERN_D8_3_7
    sd      t4, 0x38(t6)

    daddu   t6, t6, 0x40
    dli     t4, PATTERN_D8_4_0
    sd      t4, 0x0(t6)
    dli     t4, PATTERN_D8_4_1
    sd      t4, 0x8(t6)
    dli     t4, PATTERN_D8_4_2
    sd      t4, 0x10(t6)
    dli     t4, PATTERN_D8_4_3
    sd      t4, 0x18(t6)
    dli     t4, PATTERN_D8_4_4
    sd      t4, 0x20(t6)
    dli     t4, PATTERN_D8_4_5
    sd      t4, 0x28(t6)
    dli     t4, PATTERN_D8_4_6
    sd      t4, 0x30(t6)
    dli     t4, PATTERN_D8_4_7
    sd      t4, 0x38(t6)

    daddu   t6, t6, 0x40
    dli     t4, PATTERN_D8_5_0
    sd      t4, 0x0(t6)
    dli     t4, PATTERN_D8_5_1
    sd      t4, 0x8(t6)
    dli     t4, PATTERN_D8_5_2
    sd      t4, 0x10(t6)
    dli     t4, PATTERN_D8_5_3
    sd      t4, 0x18(t6)
    dli     t4, PATTERN_D8_5_4
    sd      t4, 0x20(t6)
    dli     t4, PATTERN_D8_5_5
    sd      t4, 0x28(t6)
    dli     t4, PATTERN_D8_5_6
    sd      t4, 0x30(t6)
    dli     t4, PATTERN_D8_5_7
    sd      t4, 0x38(t6)

10:
    daddiu  t9, t9, 0x1

    dli     t4, 1
    bgtu    t9, t4, 1f  //normal code
    nop
    ARB_TM_PRINTSTR("\r\nPattern WalkOnes Test...\r\n")
    //set load content t6 start addr
    dli     t6, MT_PATTERN_BASE
    GET_TM_NODE_ID_a1
    dsll    a1, a1, 44
    or      t6, t6, a1
    //address interval
    dli     t2, ADDR_INTERVAL
    //set Test Base t1 and Test Limit t3
    GET_TM_UP_ADDR
    GET_TM_MSIZE
    dsubu   t1, t3, a1

    b       2f
    nop
1:  
    dli     t4, 2
    bgtu    t9, t4, 1f
    nop
    ARB_TM_PRINTSTR("\r\nPattern WalkInvOnes Test...\r\n")
    //set load content t6 start addr
    dli     t6, MT_PATTERN_BASE
    GET_TM_NODE_ID_a1
    dsll    a1, a1, 44
    or      t6, t6, a1
    daddu   t6, t6, 0x40
    //address interval
    dli     t2, ADDR_INTERVAL
    //set Test Base t1 and Test Limit t3
    GET_TM_UP_ADDR
    GET_TM_MSIZE
    dsubu   t1, t3, a1
    b       2f
    nop
1:  
    dli     t4, 3
    bgtu    t9, t4, 1f
    nop
    ARB_TM_PRINTSTR("\r\nPattern WalkZeros Test...\r\n")
    //set load content t6 start addr
    dli     t6, MT_PATTERN_BASE
    GET_TM_NODE_ID_a1
    dsll    a1, a1, 44
    or      t6, t6, a1
    daddu   t6, t6, 0x80
    //address interval
    dli     t2, ADDR_INTERVAL
    //set Test Base t1 and Test Limit t3
    GET_TM_UP_ADDR
    GET_TM_MSIZE
    dsubu   t1, t3, a1
    b       2f
    nop
1:  
    dli     t4, 4
    bgtu    t9, t4, 1f
    nop
    ARB_TM_PRINTSTR("\r\nPattern WalkSingleOnes Test...\r\n")
    //set load content t6 start addr
    dli     t6, MT_PATTERN_BASE
    GET_TM_NODE_ID_a1
    dsll    a1, a1, 44
    or      t6, t6, a1
    daddu   t6, t6, 0xc0
    //address interval
    dli     t2, ADDR_INTERVAL
    //set Test Base t1 and Test Limit t3
    GET_TM_UP_ADDR
    GET_TM_MSIZE
    dsubu   t1, t3, a1
    b       2f
    nop
1:  
    dli     t4, 5
    bgtu    t9, t4, 1f
    nop
    ARB_TM_PRINTSTR("\r\nPattern Inverse Test...\r\n")
    //set load content t6 start addr
    dli     t6, MT_PATTERN_BASE
    GET_TM_NODE_ID_a1
    dsll    a1, a1, 44
    or      t6, t6, a1
    daddu   t6, t6, 0x100
    //address interval
    dli     t2, ADDR_INTERVAL
    //set Test Base t1 and Test Limit t3
    GET_TM_UP_ADDR
    GET_TM_MSIZE
    dsubu   t1, t3, a1
    b       2f
    nop
1:
    dli     t4, 6
    bgtu    t9, t4, 1f
    nop
    ARB_TM_PRINTSTR("\r\nPattern Random Test...\r\n")
    //set load content t6 start addr
    dli     t6, MT_PATTERN_BASE
    GET_TM_NODE_ID_a1
    dsll    a1, a1, 44
    or      t6, t6, a1
    daddu   t6, t6, 0x140
    //address interval
    dli     t2, ADDR_INTERVAL
    //set Test Base t1 and Test Limit t3
    GET_TM_UP_ADDR
    GET_TM_MSIZE
    dsubu   t1, t3, a1
    b       2f
    nop
1:
    //(all the burst inverse Pattern test done)
    b       3f  //go to the end of diff burst test
    nop
2:
    dli     t8, TM_MAX_ERRORS
#ifdef  ARB_TM_DBG
    ARB_TM_PRINTSTR("Test address range: 0x")
    dsrl    a0, t1, 32
    bal     hexserial
    nop
    move    a0, t1
    bal     hexserial
    nop
    ARB_TM_PRINTSTR("~0x")
    dsrl    a0, t3, 32
    bal     hexserial
    nop
    move    a0, t3
    bal     hexserial
    nop
    ARB_TM_PRINTSTR("  @@  address interval: 0x")
    move    a0, t2
    bal     hexserial
    nop
    ARB_TM_PRINTSTR("\r\n")
#endif
//write memory
    ld      a0, 0x0(t6)
    ld      a1, 0x8(t6)
    ld      a2, 0x10(t6)
    ld      a3, 0x18(t6)
    ld      v0, 0x20(t6)
    ld      v1, 0x28(t6)
    ld      t4, 0x30(t6)
    ld      t5, 0x38(t6)
1:
    sd      a0, 0x0(t1)
    sd      a1, 0x8(t1)
    sd      a2, 0x10(t1)
    sd      a3, 0x18(t1)
    sd      v0, 0x20(t1)
    sd      v1, 0x28(t1)
    sd      t4, 0x30(t1)
    sd      t5, 0x38(t1)
#ifdef  ARB_TM_FLUSH_CACHE
    //hit write back invalidate(D-/S-cache) to memory
    cache   0x15, 0x0(t1)
    cache   0x15, 0x20(t1)
    cache   0x17, 0x0(t1)
    cache   0x17, 0x20(t1)
#endif
#if 0
//note: if we use hit write back + hit invalidate, then rd error occur. why???
    //hit write back(D-/S-cache) to memory
    cache   0x19, 0x0(t1)
    cache   0x19, 0x20(t1)
    cache   0x1b, 0x0(t1)
    cache   0x1b, 0x20(t1)
    //hit invalidate(D-/S-cache)
    cache   0x11, 0x0(t1)
    cache   0x11, 0x20(t1)
    cache   0x13, 0x0(t1)
    cache   0x13, 0x20(t1)
    sync
#endif
    daddu   t1, t1, t2
    bltu    t1, t3, 1b
    nop
    sync
    ARB_TM_PRINTSTR("write done. begin to read and compare...\r\n")
//read memory and compare
    //set Test Base t1
    GET_TM_UP_ADDR
    GET_TM_MSIZE
    dsubu   t1, t3, a1
1:
    ld      t0, 0x0(t6)
    ld      t4, 0x0(t1)
    beq     t4, t0, 2f
    nop
    //error detected!!! print address,expected data,read data and reread data
    bal     arb_hexserial64
    nop
#ifdef  ARB_TM_DBG
    bltz    t8, 10b  //detect enough errors, go to next test
    nop
#endif
2:
    ld      t0, 0x8(t6)
    ld      t4, 0x8(t1)
    beq     t4, t0, 2f
    nop
    //error detected!!! print address,expected data,read data and reread data
    daddiu  t1, t1, 0x8 //the arb_hexserial64 will use t1 directly
    bal     arb_hexserial64
    nop
    daddiu  t1, t1, -0x8
#ifdef  ARB_TM_DBG
    bltz    t8, 10b  //detect enough errors, go to next test
    nop
#endif
2:
    ld      t0, 0x10(t6)
    ld      t4, 0x10(t1)
    beq     t4, t0, 2f
    nop
    //error detected!!! print address,expected data,read data and reread data
    daddiu  t1, t1, 0x10 //the arb_hexserial64 will use t1 directly
    bal     arb_hexserial64
    nop
    daddiu  t1, t1, -0x10
#ifdef  ARB_TM_DBG
    bltz    t8, 10b  //detect enough errors, go to next test
    nop
#endif
2:
    ld      t0, 0x18(t6)
    ld      t4, 0x18(t1)
    beq     t4, t0, 2f
    nop
    //error detected!!! print address,expected data,read data and reread data
    daddiu  t1, t1, 0x18 //the arb_hexserial64 will use t1 directly
    bal     arb_hexserial64
    nop
    daddiu  t1, t1, -0x18
#ifdef  ARB_TM_DBG
    bltz    t8, 10b  //detect enough errors, go to next test
    nop
#endif
2:
    ld      t0, 0x20(t6)
    ld      t4, 0x20(t1)
    beq     t4, t0, 2f
    nop
    //error detected!!! print address,expected data,read data and reread data
    daddiu  t1, t1, 0x20 //the arb_hexserial64 will use t1 directly
    bal     arb_hexserial64
    nop
    daddiu  t1, t1, -0x20
#ifdef  ARB_TM_DBG
    bltz    t8, 10b  //detect enough errors, go to next test
    nop
#endif
2:
    ld      t0, 0x28(t6)
    ld      t4, 0x28(t1)
    beq     t4, t0, 2f
    nop
    //error detected!!! print address,expected data,read data and reread data
    daddiu  t1, t1, 0x28 //the arb_hexserial64 will use t1 directly
    bal     arb_hexserial64
    nop
    daddiu  t1, t1, -0x28
#ifdef  ARB_TM_DBG
    bltz    t8, 10b  //detect enough errors, go to next test
    nop
#endif
2:
    ld      t0, 0x30(t6)
    ld      t4, 0x30(t1)
    beq     t4, t0, 2f
    nop
    //error detected!!! print address,expected data,read data and reread data
    daddiu  t1, t1, 0x30 //the arb_hexserial64 will use t1 directly
    bal     arb_hexserial64
    nop
    daddiu  t1, t1, -0x30
#ifdef  ARB_TM_DBG
    bltz    t8, 10b  //detect enough errors, go to next test
    nop
#endif
2:
    ld      t0, 0x38(t6)
    ld      t4, 0x38(t1)
    beq     t4, t0, 2f
    nop
    //error detected!!! print address,expected data,read data and reread data
    daddiu  t1, t1, 0x38 //the arb_hexserial64 will use t1 directly
    bal     arb_hexserial64
    nop
    daddiu  t1, t1, -0x38
#ifdef  ARB_TM_DBG
    bltz    t8, 10b  //detect enough errors, go to next test
    nop
#endif
2:
    daddu   t1, t1, t2
    //check address range
    bltu    t1, t3, 1b
    nop
    ARB_TM_PRINTSTR("Pattern Testing done.\r\n")
    b       10b
    nop
3:
#endif

    b       ARB_TM_begin
    nop

arb_hexserial64:  //pseudo subroutine
/**********************
input:  t1: read address(read only)
        t0: expected data(read only)
        t4: read data
use reg:t5, t7
***********************/
    move    t7, ra
    xor     a0, t0, t4
    or      s6, s6, a0
#ifdef  ARB_TM_DBG
    daddiu  t8, t8, -0x1
#endif
    /* reread the wrong bytes */
#if 1
#ifdef  ARB_TM_FLUSH_CACHE
    //Hit Invalidate the Primary D-cache and Second cache.
    //ARB_TM_PRINTSTR("\r\nInvalidate Primary D-cache and S-cache.\r\n")
    cache   0x11, 0x0(t1)
    cache   0x13, 0x0(t1)
    sync
    move    t5, t1
#else
    dli     t5, 0xf7ffffffffffffff
    and     t5, t1, t5
#endif
#else
    move    t5, t1
#endif
    ld      t5, 0(t5)
    nop
#ifdef  ARB_TM_DBG
    ARB_TM_PRINTSTR("addr 0x")
    dsrl    a0, t1, 32
    bal     hexserial
    nop
    move    a0, t1
    bal     hexserial
    nop
    ARB_TM_PRINTSTR(" expected: ")
    dsrl    a0, t0, 32
    bal     hexserial
    nop
    move    a0, t0
    bal     hexserial
    nop
    ARB_TM_PRINTSTR(" read: ")
    dsrl    a0, t4, 32
    bal     hexserial
    nop
    move    a0, t4
    bal     hexserial
    nop
    ARB_TM_PRINTSTR(" reread: ")
    dsrl    a0, t5, 32
    bal     hexserial
    nop
    move    a0, t5
    bal     hexserial
    nop
#endif
    /* if the reread value differs the first read, print mark */
    xor     a0, t4, t5
    beqz    a0, 2f
    nop
    //Mark Read diff detected
    or      s7, s7, a0
    ARB_TM_PRINTSTR("  DDD")
    //---------------------
2:
    ARB_TM_PRINTSTR("\r\n")
    jr      t7    
    nop

ARB_TM_end:
#ifdef  LEVEL_SPECIFIED_BYTE_LANES
    and     s6, s6, s5
    and     s7, s7, s5
#endif
#if 0
//old code
    dli     v0, 0x0
    beqz    s6, 1f
    nop
    //s6 != 0, set error mark
    dli     v0, 0x1
1:
#else
    move    v0, s6
    move    v1, s7
#endif
//resume s1~s7, t1~t9
    GET_TM_NODE_ID_a1
    dsll    a1, a1, 44
    dli     a2, MT_STACK_BASE
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
    ld      ra, 0x90(a2)
    
    jr      ra
    nop
