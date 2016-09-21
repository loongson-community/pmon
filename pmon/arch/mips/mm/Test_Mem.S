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
v2.4    Modify code for better user interface and flexible use.
********************/
/***************
use register: s1, s4, t8, a0~a3, v0~v1
note: don't change s0, s1, s2(msize)
 s1:
    [63:62]: NODE_ID
    [61:60]: CORE_ID
    [47: 0]: START_ADDR(offset on 0x9800000000000000)
t0: test pattern content
t1: current address
t2: address interval
t3: max address
t4,t5: volatile
t6: test pattern base
t7: by subroutine--hexserial64_1
t8: error counter---used to control the max detected error number
t9: loop control
s4: input param--t1
    bit[ 0]: 1: read level; 0: write level;
    bit[ 8]: 0: macro tune; 1: micro tune;
s5: store level byte mask
s6: error bits record
s7: RD error bits record
v0: output result
**************/
#include    "Test_Mem.h"

#define TM_DBG
#ifdef  TM_DBG
#define	TM_PRINTSTR(x) \
	.rdata;98: .asciz x; .text; GET_DISPRINT_BIT; bnez a1, 97f; nop; la a0, 98b; bal stringserial; nop; 97:
#else
#define	TM_PRINTSTR(x) ;
#endif

#if 0
//call example
#if 1
    TM_PRINTSTR("\r\nPlease input TM param(0xf: skip): ")
    bal     inputaddress
    nop
    move    t1, v0
    and     v0, v0, 0xf
    dli     a1, 0x1
    bgt     v0, a1, 1f
    nop
#else
    dli     t1, 0x01
#endif
    dli     s1, 0x0000000080000000  //NODE 0, start from 0x80000000
    bal     test_mem
    nop
    beqz    v0, 1f
    nop
    TM_PRINTSTR("\r\n Error found!! \r\n")
#if 1
2:
    b       2b
    nop
#endif
1:
#endif

    b       RL_end
    nop

/********************************
 * test_mem
 * input:   s1, t1
 s1:
    [63:62]: NODE_ID
    [61:60]: CORE_ID
    [55:48]: Test addr size(x128M)
    [43: 0]: START_ADDR(offset on 0x9800000000000000)
 t1:
   bit[ 0]: 1: read level; 0: write level;
   bit[ 4]: 0: macro tune; 1: micro tune;
   bit[11: 8]: byte to be leveled
   bit[12]: 1: don't print; 0: print message;
 * output:  v0
    0: no error
    1: error
********************************/
test_mem:
    move    t8, ra
    move    s4, t1

TM_start:
#ifdef  TM_DBG
    GET_DISPRINT_BIT
    bnez    a1, 88f
    nop
    TM_PRINTSTR("\r\nNODE ID:")
    GET_TM_NODE_ID_a1
    move    a0, a1
    bal     hexserial
    nop
    TM_PRINTSTR("    CORE ID:")
    GET_TM_CORE_ID_a1
    move    a0, a1
    bal     hexserial
    nop
88:
#endif
/*
 *Lock Scache 9800?00?00000000 ~ 9800?00?00001000(4K)
 *4 core use corresponding lock window
 */
    TM_PRINTSTR("\r\nLock Scache Node x--9800?00?00000000~4K...\r\n")
    dli     a2, LOCK_SCACHE_CONFIG_BASE_ADDR
    GET_TM_CORE_ID_a1
    dsll    a1, a1, 3
    daddu   a2, a2, a1
#ifdef LS3B
    GET_TM_NODE_ID_a1
    dsll    a1, a1, 14
    daddu   a2, a2, a1
#endif
    GET_TM_NODE_ID_a1
    dsll    a1, a1, 44
    or      a2, a2, a1
    dli     a3, 0x0000fffffffff000
    sd      a3, 0x40(a2)
    dli     a3, 0x8000000000000000
    or      a3, a3, a1
    GET_TM_START_ADDR
    daddu   a3, a3, a1
    sd      a3, 0x0(a2)
#if 0
    move    a0, a2
    bal     hexserial
    nop
#endif
    TM_PRINTSTR("Lock Scache Done.\r\n")
//save t0~t9,s1~s7
    dli     a2, MT_STACK_BASE
    GET_TM_NODE_ID_a1
    dsll    a1, a1, 44
    daddu   a2, a2, a1
    GET_TM_START_ADDR
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
     
#ifdef  LEVEL_SPECIFIED_BYTE_LANES
#ifdef  LEVEL_ONE_BYTE
    dli     a2, 0x700
    and     a2, t1, a2
    dsrl    a2, a2, 5   //a2>>8,and<<3(*8)
    dli     a1, 0xff
    dsll    s5, a1, a2
#else
    //give the specified byte lanes directly.
    dli     s5, LEVEL_BYTES_MASK
#endif
#endif
    dli     s6, 0x0
    dli     s7, 0x0

	TM_PRINTSTR("\r\nStart Testing Memory...\r\n")

#if 0
    //Test whether the addr_pins,column_size,bank,rank param right.
	TM_PRINTSTR("\r\nAddress Stuck Testing all space...\r\n")
    //set t1 to Test Base
    dli     t1, MEM_TEST_BASE
    GET_TM_NODE_ID_a1
    dsll    a1, a1, 44
    daddu   t1, t1, a1
    GET_TM_START_ADDR
    daddu   t1, t1, a1
    //set test limit t3
    GET_TM_MSIZE
    daddu   t3, t1, a1
    dli     a1, 0x100000    //t3-=1M  because Test Base start from Base Window + 1M
    dsubu   t3, t3, a1
#ifdef   PRINT_LESS_ERROR
    dli     t8, TM_MAX_RPT_ERRORS
#endif
#ifdef  TM_DBG
    GET_DISPRINT_BIT
    bnez    a1, 88f
    nop
    TM_PRINTSTR("Test address range: 0x")
    dsrl    a0, t1, 32
    bal     hexserial
    nop
    move    a0, t1
    bal     hexserial
    nop
    TM_PRINTSTR("~0x")
    dsrl    a0, t3, 32
    bal     hexserial
    nop
    move    a0, t3
    bal     hexserial
    nop
    TM_PRINTSTR("\r\n")
88:
#endif
    //write memory
1:	
    daddiu  a0, t1, 0x8
    daddiu  a1, t1, 0x10
    daddiu  a2, t1, 0x18
    sd      t1, 0x0(t1)
    sd      a0, 0x0(a0)
    sd      a1, 0x0(a1)
    sd      a2, 0x0(a2)
    daddiu  t1, t1, 0x200
    bltu 	t1, t3, 1b
	nop
    sync
	TM_PRINTSTR("write done...\r\n")
//read memory and compare
    dli     t1, MEM_TEST_BASE
    GET_TM_NODE_ID_a1
    dsll    a1, a1, 44
    daddu   t1, t1, a1
    GET_TM_START_ADDR
    daddu   t1, t1, a1
1:
    ld	    t4, 0x0(t1)
    move    t0, t1
    beq     t4, t0, 2f
    nop
    //error detected!!! print address, expected data and read data
    bal     hexserial64_1
    nop
#ifdef   PRINT_LESS_ERROR
    bltz    t8, 3f  //detect enough errors, go to next test
    nop
#endif
2:
    daddiu  t1, t1, 0x200
    bltu 	t1, t3, 1b
	nop
	TM_PRINTSTR("Stuck Testing done!\r\n")
3:
#endif
#if 0
	TM_PRINTSTR("Uncached Address Stuck Testing...\r\n")
//debug fatal errors
//write memory
    //set t1 to Test Base
    //use uncached address space here
	dli 	t1, UNCACHED_MEM_TEST_BASE
    GET_TM_NODE_ID_a1
    dsll    a1, a1, 44
    daddu   t1, t1, a1
    GET_TM_START_ADDR
    daddu   t1, t1, a1
#ifdef   PRINT_LESS_ERROR
    dli     t8, TM_MAX_RPT_ERRORS
#endif
    dli     t3, 128  //loop counter
1:	
    sd      t1, 0x0(t1)
    nop
    daddiu  t1, t1, 0x8
    daddiu  t3, t3, -1
    bnez 	t3, 1b
	nop
    sync
	TM_PRINTSTR("write done...\r\n")
//read memory and compare
	dli 	t1, UNCACHED_MEM_TEST_BASE
    GET_TM_NODE_ID_a1
    dsll    a1, a1, 44
    daddu   t1, t1, a1
    GET_TM_START_ADDR
    daddu   t1, t1, a1
    dli     t3, 128  //loop counter
1:
    ld	    t4, 0x0(t1)
    move    t0, t1
    beq     t4, t0, 2f
    nop
    //error detected!!! print address, expected data and read data
    bal     hexserial64_1
    nop
#ifdef   PRINT_LESS_ERROR
    bltz    t8, 20f  //detect enough errors, go to next test
    nop
#endif
2:
    daddiu  t1, t1, 0x8
    daddiu  t3, t3, -1
    bnez 	t3, 1b
	nop
	TM_PRINTSTR("Stuck Testing done!\r\n")
20:
#endif
//-------------------
//pattern Diff Burst Test
#if 0
    //initialization
    dli     t9, 0
    //set Test Pattern Base t6
    dli     t6, MT_PATTERN_BASE
    GET_TM_NODE_ID_a1
    dsll    a1, a1, 44
    daddu   t6, t6, a1
    GET_TM_START_ADDR
    daddu   t6, t6, a1
/*****************
* loop control
* t9:
* 1~16: Just0,Just1 set
* 17~32%16  Justb5, Just4a
* 33~48%32  JustA, Just5
*******************/
10:
    daddiu  t9, t9, 0x1
1:
    dli     t4, 16
#ifndef REDUCED_MEM_TEST
    bgtu    t9, t4, 1f  //normal code
#else
    bgtu    t9, t4, 3f  //reduced test
#endif
    nop
    TM_PRINTSTR("\r\nPattern DB_0 Test-----\r\n")
    //address interval
    dli     t2, 0x100
    //set Test Base t1 and Test Limit t3
    dli     t1, MEM_TEST_BASE
    GET_TM_NODE_ID_a1
    dsll    a1, a1, 44
    daddu   t1, t1, a1
    GET_TM_START_ADDR
    daddu   t1, t1, a1
    GET_TM_MSIZE
    //reduce TM size for this test
    dsrl    a1, a1, 2
    GET_MICRO_TUNE
    bnez    a2, 21f
    nop
    dsrl    a1, a1, MACRO_SCALE
21:
    daddu   t3, t1, a1
    dli     a1, 0x100000    //t3-=1M  because Test Base start from Base Window + 1M
    dsubu   t3, t3, a1
    dli     t4, 0
    dli     t0, PATTERN_DB_0_0
    not     t5, t0
    b       2f
    nop
1:  
    dli     t4, 32
    bgtu    t9, t4, 1f
    nop
    TM_PRINTSTR("\r\nPattern DB_1 Test-----\r\n")
    //address interval
    dli     t2, 0x20
    //set Test Base t1 and Test Limit t3
    dli     t1, MEM_TEST_BASE
    GET_TM_NODE_ID_a1
    dsll    a1, a1, 44
    daddu   t1, t1, a1
    GET_TM_START_ADDR
    daddu   t1, t1, a1
    GET_TM_MSIZE
    dsrl    a1, a1, 3
    GET_MICRO_TUNE
    bnez    a2, 21f
    nop
    dsrl    a1, a1, MACRO_SCALE
21:
    daddu   t3, t1, a1  //t3 = t1 + fullmsize/8
    dli     a1, 0x100000    //t3-=1M  because Test Base start from Base Window + 1M
    dsubu   t3, t3, a1
    dli     t4, 16
    dli     t0, PATTERN_DB_1_0
    not     t5, t0
    b       2f
    nop
1:  
    dli     t4, 48
    bgtu    t9, t4, 1f
    nop
    TM_PRINTSTR("\r\nPattern DB_2 Test-----\r\n")
    //address interval
    dli     t2, 0x20
    //set Test Base t1 and Test Limit t3
    dli     t1, MEM_TEST_BASE
    GET_TM_NODE_ID_a1
    dsll    a1, a1, 44
    daddu   t1, t1, a1
    GET_TM_START_ADDR
    daddu   t1, t1, a1
    GET_TM_MSIZE
    dsrl    a1, a1, 3
    GET_MICRO_TUNE
    bnez    a2, 21f
    nop
    dsrl    a1, a1, MACRO_SCALE
21:
    daddu   t3, t1, a1  //t3 = t1 + fullmsize/8
    dli     a1, 0x100000    //t3-=1M  because Test Base start from Base Window + 1M
    dsubu   t3, t3, a1
    dli     t4, 32
    dli     t0, PATTERN_DB_2_0
    not     t5, t0
    b       2f
    nop
1:  
    dli     t4, 64
    bgtu    t9, t4, 1f
    nop
    TM_PRINTSTR("\r\nPattern DB_3 Test-----\r\n")
    //address interval
    dli     t2, 0x20
    //set Test Base t1 and Test Limit t3
    dli     t1, MEM_TEST_BASE
    GET_TM_NODE_ID_a1
    dsll    a1, a1, 44
    daddu   t1, t1, a1
    GET_TM_START_ADDR
    daddu   t1, t1, a1
    GET_TM_MSIZE
    dsrl    a1, a1, 3
    GET_MICRO_TUNE
    bnez    a2, 21f
    nop
    dsrl    a1, a1, MACRO_SCALE
21:
    daddu   t3, t1, a1  //t3 = t1 + fullmsize/8
    dli     a1, 0x100000    //t3-=1M  because Test Base start from Base Window + 1M
    dsubu   t3, t3, a1
    dli     t4, 48
    dli     t0, PATTERN_DB_3_0
    not     t5, t0
    b       2f
    nop
1:
    // t9 > 64 (all the burst inverse Pattern test done)
    b       3f  //go to the end of diff burst test
    nop
2:
    daddiu  t4, 1
    bgtu    t9, t4, 1f
    nop
    //t9%16 == 1
#ifdef  TM_DBG
    GET_DISPRINT_BIT
    bnez    a1, 88f
    nop
    TM_PRINTSTR("Test address range: 0x")
    dsrl    a0, t1, 32
    bal     hexserial
    nop
    move    a0, t1
    bal     hexserial
    nop
    TM_PRINTSTR("~0x")
    dsrl    a0, t3, 32
    bal     hexserial
    nop
    move    a0, t3
    bal     hexserial
    nop
    TM_PRINTSTR("  @@  address interval: 0x")
    move    a0, t2
    bal     hexserial
    nop
    TM_PRINTSTR("\r\n")
88:
#endif
    TM_PRINTSTR("Pattern 0000 ....\r\n")
    sd      t0, 0x0(t6)
    sd      t0, 0x8(t6)
    sd      t0, 0x10(t6)
    sd      t0, 0x18(t6)
    b       2f
    nop
1:
    daddiu  t4, 1
    bgtu    t9, t4, 1f
    nop
    //t9%16 == 2
    TM_PRINTSTR("Pattern 0001 ....\r\n")
    sd      t0, 0x0(t6)
    sd      t0, 0x8(t6)
    sd      t0, 0x10(t6)
    sd      t5, 0x18(t6)
    b       2f
    nop
1:
    daddiu  t4, 1
    bgtu    t9, t4, 1f
    nop
    //t9%16 == 3
    TM_PRINTSTR("Pattern 0010 ....\r\n")
    sd      t0, 0x0(t6)
    sd      t0, 0x8(t6)
    sd      t5, 0x10(t6)
    sd      t0, 0x18(t6)
    b       2f
    nop
1:
    daddiu  t4, 1
    bgtu    t9, t4, 1f
    nop
    //t9%16 == 4
    TM_PRINTSTR("Pattern 0011 ....\r\n")
    sd      t0, 0x0(t6)
    sd      t0, 0x8(t6)
    sd      t5, 0x10(t6)
    sd      t5, 0x18(t6)
    b       2f
    nop
1:
    daddiu  t4, 1
    bgtu    t9, t4, 1f
    nop
    //t9%16 == 5
    TM_PRINTSTR("Pattern 0100 ....\r\n")
    sd      t0, 0x0(t6)
    sd      t5, 0x8(t6)
    sd      t0, 0x10(t6)
    sd      t0, 0x18(t6)
    b       2f
    nop
1:
    daddiu  t4, 1
    bgtu    t9, t4, 1f
    nop
    //t9%16 == 6
    TM_PRINTSTR("Pattern 0101 ....\r\n")
    sd      t0, 0x0(t6)
    sd      t5, 0x8(t6)
    sd      t0, 0x10(t6)
    sd      t5, 0x18(t6)
    b       2f
    nop
1:
    daddiu  t4, 1
    bgtu    t9, t4, 1f
    nop
    //t9%16 == 7
    TM_PRINTSTR("Pattern 0110 ....\r\n")
    sd      t0, 0x0(t6)
    sd      t5, 0x8(t6)
    sd      t5, 0x10(t6)
    sd      t0, 0x18(t6)
    b       2f
    nop
1:
    daddiu  t4, 1
    bgtu    t9, t4, 1f
    nop
    //t9%16 == 8
    TM_PRINTSTR("Pattern 0111 ....\r\n")
    sd      t0, 0x0(t6)
    sd      t5, 0x8(t6)
    sd      t5, 0x10(t6)
    sd      t5, 0x18(t6)
    b       2f
    nop
1:
    daddiu  t4, 1
    bgtu    t9, t4, 1f
    nop
    //t9%16 == 9
    TM_PRINTSTR("Pattern 1000 ....\r\n")
    sd      t5, 0x0(t6)
    sd      t0, 0x8(t6)
    sd      t0, 0x10(t6)
    sd      t0, 0x18(t6)
    b       2f
    nop
1:
    daddiu  t4, 1
    bgtu    t9, t4, 1f
    nop
    //t9%16 == 10
    TM_PRINTSTR("Pattern 1001 ....\r\n")
    sd      t5, 0x0(t6)
    sd      t0, 0x8(t6)
    sd      t0, 0x10(t6)
    sd      t5, 0x18(t6)
    b       2f
    nop
1:
    daddiu  t4, 1
    bgtu    t9, t4, 1f
    nop
    //t9%16 == 11
    TM_PRINTSTR("Pattern 1010 ....\r\n")
    sd      t5, 0x0(t6)
    sd      t0, 0x8(t6)
    sd      t5, 0x10(t6)
    sd      t0, 0x18(t6)
    b       2f
    nop
1:
    daddiu  t4, 1
    bgtu    t9, t4, 1f
    nop
    //t9%16 == 12
    TM_PRINTSTR("Pattern 1011 ....\r\n")
    sd      t5, 0x0(t6)
    sd      t0, 0x8(t6)
    sd      t5, 0x10(t6)
    sd      t5, 0x18(t6)
    b       2f
    nop
1:
    daddiu  t4, 1
    bgtu    t9, t4, 1f
    nop
    //t9%16 == 13
    TM_PRINTSTR("Pattern 1100 ....\r\n")
    sd      t5, 0x0(t6)
    sd      t5, 0x8(t6)
    sd      t0, 0x10(t6)
    sd      t0, 0x18(t6)
    b       2f
    nop
1:
    daddiu  t4, 1
    bgtu    t9, t4, 1f
    nop
    //t9%16 == 14
    TM_PRINTSTR("Pattern 1101 ....\r\n")
    sd      t5, 0x0(t6)
    sd      t5, 0x8(t6)
    sd      t0, 0x10(t6)
    sd      t5, 0x18(t6)
    b       2f
    nop
1:
    daddiu  t4, 1
    bgtu    t9, t4, 1f
    nop
    //t9%16 == 15
    TM_PRINTSTR("Pattern 1110 ....\r\n")
    sd      t5, 0x0(t6)
    sd      t5, 0x8(t6)
    sd      t5, 0x10(t6)
    sd      t0, 0x18(t6)
    b       2f
    nop
1:
    daddiu  t4, 1
    bgtu    t9, t4, 1f
    nop
    //t9%16 == 16
    TM_PRINTSTR("Pattern 1111 ....\r\n")
    sd      t5, 0x0(t6)
    sd      t5, 0x8(t6)
    sd      t5, 0x10(t6)
    sd      t5, 0x18(t6)
    b       2f
    nop
2:
    dli     t8, TM_MAX_RPT_ERRORS
//write memory
    ld      a0, 0x0(t6)
    ld      a1, 0x8(t6)
    ld      a2, 0x10(t6)
    ld      a3, 0x18(t6)
1:
    sd      a0, 0x0(t1)
    sd      a1, 0x8(t1)
    sd      a2, 0x10(t1)
    sd      a3, 0x18(t1)
    daddu   t1, t1, t2
    bltu    t1, t3, 1b
    nop
    sync
	TM_PRINTSTR("write done. begin to read and compare...\r\n")
//read memory and compare
    //set Test Base t1
    dli     t1, MEM_TEST_BASE
    GET_TM_NODE_ID_a1
    dsll    a1, a1, 44
    daddu   t1, t1, a1
    GET_TM_START_ADDR
    daddu   t1, t1, a1
1:
    ld      t0, 0x0(t6)
    ld      t4, 0x0(t1)
    beq     t4, t0, 2f
    nop
    //error detected!!! print address,expected data,read data and reread data
    bal     hexserial64_1
    nop
    bltz    t8, 10b  //detect enough errors, go to next test
    nop
2:
    ld      t0, 0x8(t6)
    ld      t4, 0x8(t1)
    beq     t4, t0, 2f
    nop
    //error detected!!! print address,expected data,read data and reread data
    daddiu  t1, t1, 0x8 //the hexserial64_1 will use t1 directly
    bal     hexserial64_1
    nop
    daddiu  t1, t1, -0x8
    bltz    t8, 10b  //detect enough errors, go to next test
    nop
2:
    ld      t0, 0x10(t6)
    ld      t4, 0x10(t1)
    beq     t4, t0, 2f
    nop
    //error detected!!! print address,expected data,read data and reread data
    daddiu  t1, t1, 0x10 //the hexserial64_1 will use t1 directly
    bal     hexserial64_1
    nop
    daddiu  t1, t1, -0x10
    bltz    t8, 10b  //detect enough errors, go to next test
    nop
2:
    ld      t0, 0x18(t6)
    ld      t4, 0x18(t1)
    beq     t4, t0, 2f
    nop
    //error detected!!! print address,expected data,read data and reread data
    daddiu  t1, t1, 0x18 //the hexserial64_1 will use t1 directly
    bal     hexserial64_1
    nop
    daddiu  t1, t1, -0x18
    bltz    t8, 10b  //detect enough errors, go to next test
    nop
2:
    daddu   t1, t1, t2
    //check address range
    bltu    t1, t3, 1b
    nop
    TM_PRINTSTR("Pattern Testing done.\r\n")
    b       10b
    nop
3:
#endif
#if 1
    //initialization
    dli     t9, 0
    //set Test Pattern Base t6
    dli     t6, MT_PATTERN_BASE
    GET_TM_NODE_ID_a1
    dsll    a1, a1, 44
    daddu   t6, t6, a1
    GET_TM_START_ADDR
    daddu   t6, t6, a1
/*****************
* loop control
* t9:
*******************/
10:
    daddiu  t9, t9, 0x1
1:
    dli     t4, 1
    bgtu    t9, t4, 1f  //normal code
    nop
    TM_PRINTSTR("\r\nPattern WalkOnes Test...\r\n")
    //address interval
    dli     t2, 0x40
    //set Test Base t1 and Test Limit t3
    dli     t1, MEM_TEST_BASE
    GET_TM_NODE_ID_a1
    dsll    a1, a1, 44
    daddu   t1, t1, a1
    GET_TM_START_ADDR
    daddu   t1, t1, a1
    GET_TM_MSIZE
    dsrl    a1, a1, 0
    GET_MICRO_TUNE
    bnez    a2, 21f
    nop
    dsrl    a1, a1, MACRO_SCALE
21:
    daddu   t3, t1, a1
    dli     a1, 0x100000    //t3-=1M  because Test Base start from Base Window + 1M
    dsubu   t3, t3, a1

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
    b       2f
    nop
1:  
    dli     t4, 2
    bgtu    t9, t4, 1f
    nop
    TM_PRINTSTR("\r\nPattern WalkInvOnes Test...\r\n")
    //address interval
    dli     t2, 0x40
    //set Test Base t1 and Test Limit t3
    dli     t1, MEM_TEST_BASE
    GET_TM_NODE_ID_a1
    dsll    a1, a1, 44
    daddu   t1, t1, a1
    GET_TM_START_ADDR
    daddu   t1, t1, a1
    GET_TM_MSIZE
    dsrl    a1, a1, 0
    GET_MICRO_TUNE
    bnez    a2, 21f
    nop
    dsrl    a1, a1, MACRO_SCALE
21:
    daddu   t3, t1, a1
    dli     a1, 0x100000    //t3-=1M  because Test Base start from Base Window + 1M
    dsubu   t3, t3, a1

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
    b       2f
    nop
1:  
    dli     t4, 3
    bgtu    t9, t4, 1f
    nop
    TM_PRINTSTR("\r\nPattern WalkZeros Test...\r\n")
    //address interval
    dli     t2, 0x40
    //set Test Base t1 and Test Limit t3
    dli     t1, MEM_TEST_BASE
    GET_TM_NODE_ID_a1
    dsll    a1, a1, 44
    daddu   t1, t1, a1
    GET_TM_START_ADDR
    daddu   t1, t1, a1
    GET_TM_MSIZE
    dsrl    a1, a1, 0
    GET_MICRO_TUNE
    bnez    a2, 21f
    nop
    dsrl    a1, a1, MACRO_SCALE
21:
    daddu   t3, t1, a1
    dli     a1, 0x100000    //t3-=1M  because Test Base start from Base Window + 1M
    dsubu   t3, t3, a1
    
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
    b       2f
    nop
1:  
    dli     t4, 4
    bgtu    t9, t4, 1f
    nop
    TM_PRINTSTR("\r\nPattern WalkSingleOnes Test...\r\n")
    //address interval
    dli     t2, 0x40
    //set Test Base t1 and Test Limit t3
    dli     t1, MEM_TEST_BASE
    GET_TM_NODE_ID_a1
    dsll    a1, a1, 44
    daddu   t1, t1, a1
    GET_TM_START_ADDR
    daddu   t1, t1, a1
    GET_TM_MSIZE
    dsrl    a1, a1, 0
    GET_MICRO_TUNE
    bnez    a2, 21f
    nop
    dsrl    a1, a1, MACRO_SCALE
21:
    daddu   t3, t1, a1
    dli     a1, 0x100000    //t3-=1M  because Test Base start from Base Window + 1M
    dsubu   t3, t3, a1
    
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
    b       2f
    nop
1:
    // t9 > 4 (all the burst inverse Pattern test done)
    b       3f  //go to the end of diff burst test
    nop
2:
    dli     t8, TM_MAX_RPT_ERRORS
#ifdef  TM_DBG
    GET_DISPRINT_BIT
    bnez    a1, 88f
    nop
    TM_PRINTSTR("Test address range: 0x")
    dsrl    a0, t1, 32
    bal     hexserial
    nop
    move    a0, t1
    bal     hexserial
    nop
    TM_PRINTSTR("~0x")
    dsrl    a0, t3, 32
    bal     hexserial
    nop
    move    a0, t3
    bal     hexserial
    nop
    TM_PRINTSTR("  @@  address interval: 0x")
    move    a0, t2
    bal     hexserial
    nop
    TM_PRINTSTR("\r\n")
88:
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

    daddu   t1, t1, t2
    bltu    t1, t3, 1b
    nop
    sync
	TM_PRINTSTR("write done. begin to read and compare...\r\n")
//read memory and compare
    //set Test Base t1
    dli     t1, MEM_TEST_BASE
    GET_TM_NODE_ID_a1
    dsll    a1, a1, 44
    daddu   t1, t1, a1
    GET_TM_START_ADDR
    daddu   t1, t1, a1
1:
    ld      t0, 0x0(t6)
    ld      t4, 0x0(t1)
    beq     t4, t0, 2f
    nop
    //error detected!!! print address,expected data,read data and reread data
    bal     hexserial64_1
    nop
    bltz    t8, 10b  //detect enough errors, go to next test
    nop
2:
    ld      t0, 0x8(t6)
    ld      t4, 0x8(t1)
    beq     t4, t0, 2f
    nop
    //error detected!!! print address,expected data,read data and reread data
    daddiu  t1, t1, 0x8 //the hexserial64_1 will use t1 directly
    bal     hexserial64_1
    nop
    daddiu  t1, t1, -0x8
    bltz    t8, 10b  //detect enough errors, go to next test
    nop
2:
    ld      t0, 0x10(t6)
    ld      t4, 0x10(t1)
    beq     t4, t0, 2f
    nop
    //error detected!!! print address,expected data,read data and reread data
    daddiu  t1, t1, 0x10 //the hexserial64_1 will use t1 directly
    bal     hexserial64_1
    nop
    daddiu  t1, t1, -0x10
    bltz    t8, 10b  //detect enough errors, go to next test
    nop
2:
    ld      t0, 0x18(t6)
    ld      t4, 0x18(t1)
    beq     t4, t0, 2f
    nop
    //error detected!!! print address,expected data,read data and reread data
    daddiu  t1, t1, 0x18 //the hexserial64_1 will use t1 directly
    bal     hexserial64_1
    nop
    daddiu  t1, t1, -0x18
    bltz    t8, 10b  //detect enough errors, go to next test
    nop
2:
    ld      t0, 0x20(t6)
    ld      t4, 0x20(t1)
    beq     t4, t0, 2f
    nop
    //error detected!!! print address,expected data,read data and reread data
    daddiu  t1, t1, 0x20 //the hexserial64_1 will use t1 directly
    bal     hexserial64_1
    nop
    daddiu  t1, t1, -0x20
    bltz    t8, 10b  //detect enough errors, go to next test
    nop
2:
    ld      t0, 0x28(t6)
    ld      t4, 0x28(t1)
    beq     t4, t0, 2f
    nop
    //error detected!!! print address,expected data,read data and reread data
    daddiu  t1, t1, 0x28 //the hexserial64_1 will use t1 directly
    bal     hexserial64_1
    nop
    daddiu  t1, t1, -0x28
    bltz    t8, 10b  //detect enough errors, go to next test
    nop
2:
    ld      t0, 0x30(t6)
    ld      t4, 0x30(t1)
    beq     t4, t0, 2f
    nop
    //error detected!!! print address,expected data,read data and reread data
    daddiu  t1, t1, 0x30 //the hexserial64_1 will use t1 directly
    bal     hexserial64_1
    nop
    daddiu  t1, t1, -0x30
    bltz    t8, 10b  //detect enough errors, go to next test
    nop
2:
    ld      t0, 0x38(t6)
    ld      t4, 0x38(t1)
    beq     t4, t0, 2f
    nop
    //error detected!!! print address,expected data,read data and reread data
    daddiu  t1, t1, 0x38 //the hexserial64_1 will use t1 directly
    bal     hexserial64_1
    nop
    daddiu  t1, t1, -0x38
    bltz    t8, 10b  //detect enough errors, go to next test
    nop
2:
    daddu   t1, t1, t2
    //check address range
    bltu    t1, t3, 1b
    nop
    TM_PRINTSTR("Pattern Testing done.\r\n")
    b       10b
    nop
3:
#endif
//pattern JustX Test
#ifndef REDUCED_MEM_TEST
    //initialization
    dli     t9, 0
    //address interval
    dli     t2, 0x10
    //set Test Base t1 and Test Limit t3
    dli     t1, MEM_TEST_BASE
    GET_TM_NODE_ID_a1
    dsll    a1, a1, 44
    daddu   t1, t1, a1
    GET_TM_START_ADDR
    daddu   t1, t1, a1
    GET_TM_MSIZE
    dsrl    a1, a1, 2
    GET_MICRO_TUNE
    bnez    a2, 21f
    nop
    dsrl    a1, a1, MACRO_SCALE
21:
    daddu   t3, t1, a1
    dli     a1, 0x100000    //t3-=1M  because Test Base start from Base Window + 1M
    dsubu   t3, t3, a1
10:
    //loop control
    daddiu  t9, t9, 0x1
1:
    dli     t4, 1
    bgtu    t9, t4, 1f
    nop
    //t9 == 1
    TM_PRINTSTR("\r\nPattern JustA Test...\r\n")
    dli     t0, PATTERN_JUSTA
    b       2f
    nop
1:
    dli     t4, 2
    bgtu    t9, t4, 1f
    nop
    //t9 == 2
    TM_PRINTSTR("\r\nPattern Just5 Test...\r\n")
    dli     t0, PATTERN_JUST5
    b       2f
    nop
1:
    dli     t4, 3
    bgtu    t9, t4, 1f
    nop
    //t9 == 3
    TM_PRINTSTR("\r\nPattern JustFiveA Test...\r\n")
    dli     t0, PATTERN_FiveA
    b       2f
    nop
1:
    dli     t4, 4
    bgtu    t9, t4, 1f
    nop
    //t9 == 4
    TM_PRINTSTR("\r\nPattern JustZeroOne Test...\r\n")
    dli     t0, PATTERN_ZEROONE
    b       2f
    nop
1:
    dli     t4, 5
    bgtu    t9, t4, 1f
    nop
    //t9 == 5
    TM_PRINTSTR("\r\nPattern JustL8b10b-16 Test...\r\n")
    dli     t0, PATTERN_L8b10b
    b       2f
    nop
1:
    dli     t4, 6
    bgtu    t9, t4, 1f
    nop
    //t9 == 6
    TM_PRINTSTR("\r\nPattern JustS8b10b-b5 Test...\r\n")
    dli     t0, PATTERN_S8b10b
    b       2f
    nop
1:
    dli     t4, 7
    bgtu    t9, t4, 1f
    nop
    //t9 == 7
    TM_PRINTSTR("\r\nPattern JustFive7 Test...\r\n")
    dli     t0, PATTERN_Five7
    b       2f
    nop
1:
    dli     t4, 8
    bgtu    t9, t4, 1f
    nop
    //t9 == 8
    TM_PRINTSTR("\r\nPattern JustZero2fd Test...\r\n")
    dli     t0, PATTERN_Zero2fd
    b       2f
    nop
1:  
    // t9 > 8 (all the JustX Pattern test done)
    b       3f  //go to the end of this loop
    nop
2:
    dli     t8, TM_MAX_RPT_ERRORS
    //set Test Base t1
    dli     t1, MEM_TEST_BASE
    GET_TM_NODE_ID_a1
    dsll    a1, a1, 44
    daddu   t1, t1, a1
    GET_TM_START_ADDR
    daddu   t1, t1, a1
#ifdef  TM_DBG
    GET_DISPRINT_BIT
    bnez    a1, 88f
    nop
    TM_PRINTSTR("Test address range: 0x")
    dsrl    a0, t1, 32
    bal     hexserial
    nop
    move    a0, t1
    bal     hexserial
    nop
    TM_PRINTSTR("~0x")
    dsrl    a0, t3, 32
    bal     hexserial
    nop
    move    a0, t3
    bal     hexserial
    nop
    TM_PRINTSTR("  @@  address interval: 0x")
    move    a0, t2
    bal     hexserial
    nop
    TM_PRINTSTR("\r\n")
88:
#endif
//write memory
1:
    sd      t0, 0x0(t1)
    sd      t0, 0x8(t1)
    daddu   t1, t1, t2
    bltu    t1, t3, 1b
    nop
    sync
	TM_PRINTSTR("write done. begin to read and compare...\r\n")
//read memory and compare
    //set Test Base t1
    dli     t1, MEM_TEST_BASE
    GET_TM_NODE_ID_a1
    dsll    a1, a1, 44
    daddu   t1, t1, a1
    GET_TM_START_ADDR
    daddu   t1, t1, a1
1:
    ld      t4, 0x0(t1)
    beq     t4, t0, 2f
    nop
    //error detected!!! print address,expected data,read data and reread data
    bal     hexserial64_1
    nop
    bltz    t8, 10b  //detect enough errors, go to next test
    nop
2:
    ld      t4, 0x8(t1)
    beq     t4, t0, 2f
    nop
    //error detected!!! print address,expected data,read data and reread data
    daddiu  t1, t1, 0x8 //the hexserial64_1 will use t1 directly
    bal     hexserial64_1
    nop
    daddiu  t1, t1, -0x8
    bltz    t8, 10b  //detect enough errors, go to next test
    nop
2:
    daddu   t1, t1, t2
    //check address range
    bltu    t1, t3, 1b
    nop
    TM_PRINTSTR("Pattern Testing done.\r\n")
    b       10b
    nop
3:
#endif

TM_end:
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
    dli     a2, MT_STACK_BASE
    GET_TM_NODE_ID_a1
    dsll    a1, a1, 44
    daddu   a2, a2, a1
    GET_TM_START_ADDR
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
    
    move    t7, v0
    move    t6, v1
/*
 *Unlock Scache 9800?00000000000 ~ 9800?00000001000(4K)
 */
    TM_PRINTSTR("\r\nUnlock Scache Node x--9800?00000000000~4K...\r\n")

    dli     a2, LOCK_SCACHE_CONFIG_BASE_ADDR
    GET_TM_CORE_ID_a1
    dsll    a1, a1, 3
    daddu   a2, a2, a1
#ifdef LS3B
    GET_TM_NODE_ID_a1
    dsll    a1, a1, 14
    daddu   a2, a2, a1
#endif
    GET_TM_NODE_ID_a1
    dsll    a1, a1, 44
    or      a2, a2, a1
    sd      $0, 0x0(a2)
    sd      $0, 0x40(a2)
    sync

    //Hit Invalidate the locked address
    dli     a0, 0x9800000000000000
    GET_TM_NODE_ID_a1
    dsll    a1, a1, 44
    or      a0, a0, a1
    GET_TM_START_ADDR
    daddu   a0, a0, a1
    dli     a1, 0x1000
    daddu   a2, a0, a1
1:
    cache   0x11, 0x0(a0)
    cache   0x13, 0x0(a0)

    daddu   a0, a0, 0x20
    blt     a0, a2, 1b
    nop
    TM_PRINTSTR("Unlock Scache Done.\r\n")

    move    v0, t7
    move    v1, t6
    jr      t8
    nop
//===================================

/**********************
* input:
    s1: NODE ID, Test mem size, Test start addr 
    t0: write content
    t1: 
**********************/

simple_test_mem:
    move    t8, ra
    move    s4, t1

	TM_PRINTSTR("\r\nStart simple test mem...\r\n")
    //set Test Base t1 and Test Limit t3
    dli     t1, SIMPLE_TM_BASE
    GET_TM_NODE_ID_a1
    dsll    a1, a1, 44
    daddu   t1, t1, a1
    GET_TM_START_ADDR
    daddu   t1, t1, a1
    GET_TM_MSIZE
    daddu   t3, t1, a1

	//TM_PRINTSTR("\r\nStart write data to Memory...\r\n")
    //address interval
    dli     t2, 0x20
    //set loop times
    dli     t9, TMF_PWRLOOP
2:
    daddiu  t9, -1
    move    t6, t1
1:
    sd      t0, 0x0(t6)
    not     t0, t0
    sd      t0, 0x8(t6)
    not     t0, t0
    sd      t0, 0x10(t6)
    not     t0, t0
    sd      t0, 0x18(t6)
    not     t0, t0

    daddu   t6, t6, t2
    bltu    t6, t3, 1b
    nop
    bnez    t9, 2b
    nop
	//TM_PRINTSTR("Data write done...\r\n")

    //TM_PRINTSTR("\r\nStart Read Memory...\r\n")
    //address interval
    dli     t2, 0x20
    //set loop times
    dli     t9, TMF_PRDLOOP
20:
    daddiu  t9, -1
    //set Test Base t1
    dli     t1, SIMPLE_TM_BASE
    GET_TM_NODE_ID_a1
    dsll    a1, a1, 44
    daddu   t1, t1, a1
    GET_TM_START_ADDR
    daddu   t1, t1, a1
10:
    ld      t4, 0x0(t1)
    beq     t4, t0, 2f
    nop
    //error detected!!! print address,expected data,read data and reread data
    bal     hexserial64_1
    nop
2:
    not     t0, t0
    ld      t4, 0x8(t1)
    beq     t4, t0, 2f
    nop
    //error detected!!! print address,expected data,read data and reread data
    daddiu  t1, t1, 0x8 //the hexserial64_1 will use t1 directly
    bal     hexserial64_1
    nop
    daddiu  t1, t1, -0x8
2:
    not     t0, t0
    ld      t4, 0x10(t1)
    beq     t4, t0, 2f
    nop
    //error detected!!! print address,expected data,read data and reread data
    daddiu  t1, t1, 0x10 //the hexserial64_1 will use t1 directly
    bal     hexserial64_1
    nop
    daddiu  t1, t1, -0x10
2:
    not     t0, t0
    ld      t4, 0x18(t1)
    beq     t4, t0, 2f
    nop
    //error detected!!! print address,expected data,read data and reread data
    daddiu  t1, t1, 0x18 //the hexserial64_1 will use t1 directly
    bal     hexserial64_1
    nop
    daddiu  t1, t1, -0x18
2:
    not     t0, t0

    daddu   t1, t1, t2
    bltu    t1, t3, 10b
    nop
    bnez    t9, 20b
    nop
    //TM_PRINTSTR("Reading done.\r\n")

    jr      t8
    nop
//===================================
//================================
test_mem_1:
    move    t9, ra
    move    s4, t1

    dli     s6, 0x0
    dli     s7, 0x0

	TM_PRINTSTR("\r\nStart Testing Memory...\r\n")

    TM_PRINTSTR("\r\nPattern DB_0 Test-----\r\n")
    //address interval
    dli     t2, 0x200
    //set Test Base t1 and Test Limit t3
    dli     t1, SIMPLE_TM_BASE
    GET_TM_NODE_ID_a1
    dsll    a1, a1, 44
    daddu   t1, t1, a1
    GET_TM_START_ADDR
    daddu   t1, t1, a1
    GET_TM_MSIZE
    daddu   t3, t1, a1

#ifdef  TM_DBG
    GET_DISPRINT_BIT
    bnez    a1, 88f
    nop
    TM_PRINTSTR("Test address range: 0x")
    dsrl    a0, t1, 32
    bal     hexserial
    nop
    move    a0, t1
    bal     hexserial
    nop
    TM_PRINTSTR("~0x")
    dsrl    a0, t3, 32
    bal     hexserial
    nop
    move    a0, t3
    bal     hexserial
    nop
    TM_PRINTSTR("  @@  address interval: 0x")
    move    a0, t2
    bal     hexserial
    nop
    TM_PRINTSTR("\r\n")
88:
#endif
    TM_PRINTSTR("Pattern 0110 ....\r\n")
    dli     t6, 0x4000000

10:
    daddu   t6, t6, -0x1
    //set Test Base t1 and Test Limit t3
    dli     t1, SIMPLE_TM_BASE
    GET_TM_NODE_ID_a1
    dsll    a1, a1, 44
    daddu   t1, t1, a1
    GET_TM_START_ADDR
    daddu   t1, t1, a1
    GET_TM_MSIZE
    daddu   t3, t1, a1

    dli     t8, TM_MAX_RPT_ERRORS
    dli     t0, PATTERN_DB_0_0
//write memory
    move    a0, t0
    not     a1, t0
    not     a2, t0
    move    a3, t0
1:
    sd      a0, 0x0(t1)
    sd      a1, 0x8(t1)
    sd      a2, 0x10(t1)
    sd      a3, 0x18(t1)
    daddu   t1, t1, t2
    bltu    t1, t3, 1b
    nop
    sync
	TM_PRINTSTR("write done. begin to read and compare...\r\n")
//read memory and compare
    //set Test Base t1
    dli     t1, MEM_TEST_BASE
    GET_TM_NODE_ID_a1
    dsll    a1, a1, 44
    daddu   t1, t1, a1
    GET_TM_START_ADDR
    daddu   t1, t1, a1
1:
    ld      t4, 0x0(t1)
    beq     t4, t0, 2f
    nop
    //error detected!!! print address,expected data,read data and reread data
    bal     hexserial64_1
    nop
    bltz    t8, 10b  //detect enough errors, go to next test
    nop
2:
    not     t0, t0
    ld      t4, 0x8(t1)
    beq     t4, t0, 2f
    nop
    //error detected!!! print address,expected data,read data and reread data
    daddiu  t1, t1, 0x8 //the hexserial64_1 will use t1 directly
    bal     hexserial64_1
    nop
    daddiu  t1, t1, -0x8
    bltz    t8, 10b  //detect enough errors, go to next test
    nop
2:
    ld      t4, 0x10(t1)
    beq     t4, t0, 2f
    nop
    //error detected!!! print address,expected data,read data and reread data
    daddiu  t1, t1, 0x10 //the hexserial64_1 will use t1 directly
    bal     hexserial64_1
    nop
    daddiu  t1, t1, -0x10
    bltz    t8, 10b  //detect enough errors, go to next test
    nop
2:
    not     t0, t0
    ld      t4, 0x18(t1)
    beq     t4, t0, 2f
    nop
    //error detected!!! print address,expected data,read data and reread data
    daddiu  t1, t1, 0x18 //the hexserial64_1 will use t1 directly
    bal     hexserial64_1
    nop
    daddiu  t1, t1, -0x18
    bltz    t8, 10b  //detect enough errors, go to next test
    nop
2:
    daddu   t1, t1, t2
    //check address range
    bltu    t1, t3, 1b
    nop

    bnez    t6, 10b
    nop
    TM_PRINTSTR("Pattern Testing done.\r\n")

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

    jr      t9
    nop
//===================================
//================================
test_mem_1_silence:
    move    t9, ra
    move    s4, t1

    dli     s6, 0x0
    dli     s7, 0x0

	//TM_PRINTSTR("\r\nStart Testing Memory...\r\n")

//pattern Diff Burst Test
#if 1
    //TM_PRINTSTR("\r\nPattern DB_0 Test-----\r\n")
    //address interval
    dli     t2, 0x40
    //set Test Base t1 and Test Limit t3
    dli     t1, SIMPLE_TM_BASE
    GET_TM_NODE_ID_a1
    dsll    a1, a1, 44
    daddu   t1, t1, a1
    GET_TM_START_ADDR
    daddu   t1, t1, a1
    GET_TM_MSIZE
    daddu   t3, t1, a1

#if 0
    GET_DISPRINT_BIT
    bnez    a1, 88f
    nop
    TM_PRINTSTR("Test address range: 0x")
    dsrl    a0, t1, 32
    bal     hexserial
    nop
    move    a0, t1
    bal     hexserial
    nop
    TM_PRINTSTR("~0x")
    dsrl    a0, t3, 32
    bal     hexserial
    nop
    move    a0, t3
    bal     hexserial
    nop
    TM_PRINTSTR("  @@  address interval: 0x")
    move    a0, t2
    bal     hexserial
    nop
    TM_PRINTSTR("\r\n")
88:
#endif
    //TM_PRINTSTR("Pattern 0110 ....\r\n")
    dli     t6, 0x40000000

10:
    daddu   t6, t6, -0x1
    //set Test Base t1 and Test Limit t3
    dli     t1, SIMPLE_TM_BASE
    GET_TM_NODE_ID_a1
    dsll    a1, a1, 44
    daddu   t1, t1, a1
    GET_TM_START_ADDR
    daddu   t1, t1, a1
    GET_TM_MSIZE
    daddu   t3, t1, a1

    dli     t8, TM_MAX_RPT_ERRORS
    dli     t0, PATTERN_DB_0_0
//write memory
    move    a0, t0
    not     a1, t0
    not     a2, t0
    move    a3, t0
1:
    sd      a0, 0x0(t1)
    sd      a1, 0x8(t1)
    sd      a2, 0x10(t1)
    sd      a3, 0x18(t1)
    daddu   t1, t1, t2
    bltu    t1, t3, 1b
    nop
    sync
	//TM_PRINTSTR("write done. begin to read and compare...\r\n")
//read memory and compare
    //set Test Base t1
    dli     t1, MEM_TEST_BASE
    GET_TM_NODE_ID_a1
    dsll    a1, a1, 44
    daddu   t1, t1, a1
    GET_TM_START_ADDR
    daddu   t1, t1, a1
1:
    ld      t4, 0x0(t1)
    beq     t4, t0, 2f
    nop
    //error detected!!! print address,expected data,read data and reread data
    bal     hexserial64_1_silence
    nop
    bltz    t8, 10b  //detect enough errors, go to next test
    nop
2:
    not     t0, t0
    ld      t4, 0x8(t1)
    beq     t4, t0, 2f
    nop
    //error detected!!! print address,expected data,read data and reread data
    daddiu  t1, t1, 0x8 //the hexserial64_1_silence will use t1 directly
    bal     hexserial64_1_silence
    nop
    daddiu  t1, t1, -0x8
    bltz    t8, 10b  //detect enough errors, go to next test
    nop
2:
    ld      t4, 0x10(t1)
    beq     t4, t0, 2f
    nop
    //error detected!!! print address,expected data,read data and reread data
    daddiu  t1, t1, 0x10 //the hexserial64_1_silence will use t1 directly
    bal     hexserial64_1_silence
    nop
    daddiu  t1, t1, -0x10
    bltz    t8, 10b  //detect enough errors, go to next test
    nop
2:
    not     t0, t0
    ld      t4, 0x18(t1)
    beq     t4, t0, 2f
    nop
    //error detected!!! print address,expected data,read data and reread data
    daddiu  t1, t1, 0x18 //the hexserial64_1_silence will use t1 directly
    bal     hexserial64_1_silence
    nop
    daddiu  t1, t1, -0x18
    bltz    t8, 10b  //detect enough errors, go to next test
    nop
2:
    daddu   t1, t1, t2
    //check address range
    bltu    t1, t3, 1b
    nop

    bnez    t6, 10b
    nop
    //TM_PRINTSTR("Pattern Testing done.\r\n")
#endif

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

    jr      t9
    nop
//===================================

hexserial64_1:  //pseudo subroutine
/**********************
input:  t1: read address(read only)
        t0: expected data(read only)
        t4: read data
use reg:t5, t7
***********************/
    move    t7, ra
    xor     a0, t0, t4
    or      s6, s6, a0
#ifdef  PRINT_LESS_ERROR
    daddiu  t8, t8, -0x1
#endif
    /* reread the wrong bytes */
#if 1
#if 1
    //Hit Invalidate the Primary D-cache and Second cache.
    //TM_PRINTSTR("\r\nInvalidate Primary D-cache and S-cache.\r\n")
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
#ifdef  TM_DBG
    GET_DISPRINT_BIT
    bnez    a1, 88f
    nop
	TM_PRINTSTR("addr 0x")
	dsrl	a0, t1, 32
	bal	    hexserial
	nop
	move	a0, t1
	bal	    hexserial
	nop
    TM_PRINTSTR(" expected: ")
	dsrl	a0, t0, 32
	bal	    hexserial
	nop
	move	a0, t0
	bal	    hexserial
	nop
    TM_PRINTSTR(" read: ")
	dsrl	a0, t4, 32
	bal	    hexserial
	nop
	move	a0, t4
	bal	    hexserial
	nop
    TM_PRINTSTR(" reread: ")
	dsrl	a0, t5, 32
	bal	    hexserial
	nop
	move	a0, t5
	bal	    hexserial
	nop
88:
#endif
    /* if the reread value differs the first read, print mark */
    xor     a0, t4, t5
    beqz    a0, 2f
    nop
    //Mark Read diff detected
    or      s7, s7, a0
    TM_PRINTSTR("  DDD")
    //---------------------
2:
    TM_PRINTSTR("\r\n")
    jr      t7    
    nop

//===================================
hexserial64_1_silence:  //pseudo subroutine
/**********************
input:  t1: read address(read only)
        t0: expected data(read only)
        t4: read data
use reg:t5, t7
***********************/
    move    t7, ra
    xor     a0, t0, t4
    or      s6, s6, a0
#ifdef  PRINT_LESS_ERROR
    daddiu  t8, t8, -0x1
#endif
    /* reread the wrong bytes */
#if 1
#if 1
    //Hit Invalidate the Primary D-cache and Second cache.
    //TM_PRINTSTR("\r\nInvalidate Primary D-cache and S-cache.\r\n")
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
#if 0
    GET_DISPRINT_BIT
    bnez    a1, 88f
    nop
	TM_PRINTSTR("addr 0x")
	dsrl	a0, t1, 32
	bal	    hexserial
	nop
	move	a0, t1
	bal	    hexserial
	nop
    TM_PRINTSTR(" expected: ")
	dsrl	a0, t0, 32
	bal	    hexserial
	nop
	move	a0, t0
	bal	    hexserial
	nop
    TM_PRINTSTR(" read: ")
	dsrl	a0, t4, 32
	bal	    hexserial
	nop
	move	a0, t4
	bal	    hexserial
	nop
    TM_PRINTSTR(" reread: ")
	dsrl	a0, t5, 32
	bal	    hexserial
	nop
	move	a0, t5
	bal	    hexserial
	nop
88:
#endif
    /* if the reread value differs the first read, print mark */
    xor     a0, t4, t5
    beqz    a0, 2f
    nop
    //Mark Read diff detected
    or      s7, s7, a0
    //TM_PRINTSTR("  DDD")
    //---------------------
2:
    //TM_PRINTSTR("\r\n")
    jr      t7    
    nop

RL_end:
