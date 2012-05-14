/********************
Author: Chen Xinke
Function: Test memory read and write errors
Usage:  include this file in the start.S, after initialize cache and before copy Pmon Text to the memory;
v1.2    Test address range is auto-configured by msize(use the default window)
        precondition:
        1. default L2-Xbar window(fullmsize ~ fullmsize * 2) must be configured.
v2.0    Support Multi-chip mode memory test and new msize map, Node ID is given by user input.
v2.2    Modify the structure of the program to reduce its size and make it more scalable.
v2.4    Modify code for better user interface and flexible use.
v2.6    Modify code for Single Node Chip(LS2X).
v2.8    Modify code for Single Node Chip(LS1X).
********************/
/***************
use register: s1, s4, t8, a0~a3, v0~v1
note: don't change s0, s1, s2(msize)

t0: test pattern content
t1: current address
t2: address interval
t3: max address
t4,t5: volatile
t7: by subroutine--hexserial_1
t8: error counter---used to control the max detected error number
t9: loop control
t6: store ra
s4: input param--t1
    bit[ 0]: 1: read level; 0: write level;
    bit[ 8]: 0: macro tune; 1: micro tune;
s5: store level byte mask
s6: error bits record
s7: RD error bits record
v0: output result
**************/
#include    "Test_Mem_S1.h"

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
    li      a1, 0x1
    bgt     v0, a1, 1f
    nop
#else
    li      t1, 0x01
#endif
    li      s1, 0x0000000080000000  //NODE 0, start from 0x80000000
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
    [31:30]: Test memory size(x128M)
    [29: 0]: START_ADDR(offset on 0x0)
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
    move    t6, ra
    move    s4, t1

TM_start:
     
#ifdef  LEVEL_SPECIFIED_BYTE_LANES
#ifdef  LEVEL_ONE_BYTE
    li      a2, 0x700
    and     a2, t1, a2
    srl     a2, a2, 5   //a2>>8,and<<3(*8)
    li      a1, 0xff
    sll     s5, a1, a2
#else
    //give the specified byte lanes directly.
    li      s5, LEVEL_BYTES_MASK
#endif
#endif
    li      s6, 0x0
    li      s7, 0x0

	TM_PRINTSTR("\r\nStart Testing Memory...\r\n")

#if 1
    //Test whether the addr_pins,column_size,bank,rank param right.
    TM_PRINTSTR("\r\nAddress Stuck Testing all space...\r\n")
    //set t1 to Test Base
    li      t1, MEM_TEST_BASE
    GET_TM_START_ADDR
    addu    t1, t1, a1
    //set test limit t3
    GET_TM_MSIZE
    addu    t3, t1, a1
#ifdef   PRINT_LESS_ERROR
    li      t8, TM_MAX_ERRORS
#endif
#ifdef  TM_DBG
    GET_DISPRINT_BIT
    bnez    a1, 88f
    nop
    TM_PRINTSTR("Test address range: 0x")
    move    a0, t1
    bal     hexserial
    nop
    TM_PRINTSTR("~0x")
    move    a0, t3
    bal     hexserial
    nop
    TM_PRINTSTR("\r\n")
88:
#endif
    //write memory
1:	
    addu    a0, t1, 0x4
    addu    a1, t1, 0x8
    addu    a2, t1, 0xc
    sw      t1, 0x0(t1)
    sw      a0, 0x0(a0)
    sw      a1, 0x0(a1)
    sw      a2, 0x0(a2)
    addu    t1, t1, 0x200
    bltu 	t1, t3, 1b
    nop
    sync
    TM_PRINTSTR("write done...\r\n")
//read memory and compare
    li      t1, MEM_TEST_BASE
    GET_TM_START_ADDR
    addu    t1, t1, a1
1:
    lw	    t4, 0x0(t1)
    move    t0, t1
    beq     t4, t0, 2f
    nop
    //error detected!!! print address, expected data and read data
    bal     hexserial_1
    nop
#ifdef   PRINT_LESS_ERROR
    bltz    t8, 3f  //detect enough errors, go to next test
    nop
#endif
2:
    addu    t1, t1, 0x200
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
	li  	t1, UNCACHED_MEM_TEST_BASE
    GET_TM_START_ADDR
    addu    t1, t1, a1
#ifdef   PRINT_LESS_ERROR
    li      t8, 32  //max print errors 32 
#endif
    li      t3, 128  //loop counter
1:	
    sw      t1, 0x0(t1)
    nop
    addu    t1, t1, 0x8
    addu    t3, t3, -1
    bnez 	t3, 1b
	nop
    sync
	TM_PRINTSTR("write done...\r\n")
//read memory and compare
	li  	t1, UNCACHED_MEM_TEST_BASE
    GET_TM_START_ADDR
    addu    t1, t1, a1
    li      t3, 128  //loop counter
1:
    lw	    t4, 0x0(t1)
    move    t0, t1
    beq     t4, t0, 2f
    nop
    //error detected!!! print address, expected data and read data
    bal     hexserial_1
    nop
#ifdef   PRINT_LESS_ERROR
    bltz    t8, 20f  //detect enough errors, go to next test
    nop
#endif
2:
    addu    t1, t1, 0x8
    addu    t3, t3, -1
    bnez 	t3, 1b
	nop
	TM_PRINTSTR("Stuck Testing done!\r\n")
20:
#endif
//-------------------
//pattern Diff Burst Test
#if 0
    //initialization
    li      t9, 0
    //set Test Pattern Base t6
    li      t6, MT_PATTERN_BASE
    GET_TM_NODE_ID_a1
    sll     a1, a1, 44
    addu    t6, t6, a1
    GET_TM_START_ADDR
    addu    t6, t6, a1
/*****************
* loop control
* t9:
* 1~16: Just0,Just1 set
* 17~32%16  Justb5, Just4a
* 33~48%32  JustA, Just5
*******************/
10:
    addu    t9, t9, 0x1
1:
    li      t4, 16
#ifndef REDUCED_MEM_TEST
    bgtu    t9, t4, 1f  //normal code
#else
    bgtu    t9, t4, 3f  //reduced test
#endif
    nop
    TM_PRINTSTR("\r\nPattern DB_0 Test-----\r\n")
    //address interval
    li      t2, 0x20
    //set Test Base t1 and Test Limit t3
    li      t1, MEM_TEST_BASE
    GET_TM_NODE_ID_a1
    sll     a1, a1, 44
    addu    t1, t1, a1
    GET_TM_START_ADDR
    addu    t1, t1, a1
    GET_TM_MSIZE
    //reduce TM size for this test
    srl     a1, a1, 2
    GET_MICRO_TUNE
    bnez    a2, 21f
    nop
    srl     a1, a1, MACRO_SCALE
21:
    addu    t3, t1, a1
    li      a1, 0x100000    //t3-=1M  because Test Base start from Base Window + 1M
    dsubu   t3, t3, a1
    li      t4, 0
    li      t0, PATTERN_DB_0_0
    not     t5, t0
    b       2f
    nop
1:  
    li      t4, 32
    bgtu    t9, t4, 1f
    nop
    TM_PRINTSTR("\r\nPattern DB_1 Test-----\r\n")
    //address interval
    li      t2, 0x20
    //set Test Base t1 and Test Limit t3
    li      t1, MEM_TEST_BASE
    GET_TM_NODE_ID_a1
    sll     a1, a1, 44
    addu    t1, t1, a1
    GET_TM_START_ADDR
    addu    t1, t1, a1
    GET_TM_MSIZE
    srl     a1, a1, 3
    GET_MICRO_TUNE
    bnez    a2, 21f
    nop
    srl     a1, a1, MACRO_SCALE
21:
    addu    t3, t1, a1  //t3 = t1 + fullmsize/8
    li      a1, 0x100000    //t3-=1M  because Test Base start from Base Window + 1M
    dsubu   t3, t3, a1
    li      t4, 16
    li      t0, PATTERN_DB_1_0
    not     t5, t0
    b       2f
    nop
1:  
    li      t4, 48
    bgtu    t9, t4, 1f
    nop
    TM_PRINTSTR("\r\nPattern DB_2 Test-----\r\n")
    //address interval
    li      t2, 0x20
    //set Test Base t1 and Test Limit t3
    li      t1, MEM_TEST_BASE
    GET_TM_NODE_ID_a1
    sll     a1, a1, 44
    addu    t1, t1, a1
    GET_TM_START_ADDR
    addu    t1, t1, a1
    GET_TM_MSIZE
    srl     a1, a1, 3
    GET_MICRO_TUNE
    bnez    a2, 21f
    nop
    srl     a1, a1, MACRO_SCALE
21:
    addu    t3, t1, a1  //t3 = t1 + fullmsize/8
    li      a1, 0x100000    //t3-=1M  because Test Base start from Base Window + 1M
    dsubu   t3, t3, a1
    li      t4, 32
    li      t0, PATTERN_DB_2_0
    not     t5, t0
    b       2f
    nop
1:  
    li      t4, 64
    bgtu    t9, t4, 1f
    nop
    TM_PRINTSTR("\r\nPattern DB_3 Test-----\r\n")
    //address interval
    li      t2, 0x200
    //set Test Base t1 and Test Limit t3
    li      t1, MEM_TEST_BASE
    GET_TM_NODE_ID_a1
    sll     a1, a1, 44
    addu    t1, t1, a1
    GET_TM_START_ADDR
    addu    t1, t1, a1
    GET_TM_MSIZE
    srl     a1, a1, 3
    GET_MICRO_TUNE
    bnez    a2, 21f
    nop
    srl     a1, a1, MACRO_SCALE
21:
    addu    t3, t1, a1  //t3 = t1 + fullmsize/8
    li      a1, 0x100000    //t3-=1M  because Test Base start from Base Window + 1M
    dsubu   t3, t3, a1
    li      t4, 48
    li      t0, PATTERN_DB_3_0
    not     t5, t0
    b       2f
    nop
1:
    // t9 > 64 (all the burst inverse Pattern test done)
    b       3f  //go to the end of diff burst test
    nop
2:
    addu    t4, 1
    bgtu    t9, t4, 1f
    nop
    //t9%16 == 1
#ifdef  TM_DBG
    GET_DISPRINT_BIT
    bnez    a1, 88f
    nop
    TM_PRINTSTR("Test address range: 0x")
    srl     a0, t1, 32
    bal     hexserial
    nop
    move    a0, t1
    bal     hexserial
    nop
    TM_PRINTSTR("~0x")
    srl     a0, t3, 32
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
    sw      t0, 0x0(t6)
    sw      t0, 0x8(t6)
    sw      t0, 0x10(t6)
    sw      t0, 0x18(t6)
    b       2f
    nop
1:
    addu    t4, 1
    bgtu    t9, t4, 1f
    nop
    //t9%16 == 2
    TM_PRINTSTR("Pattern 0001 ....\r\n")
    sw      t0, 0x0(t6)
    sw      t0, 0x8(t6)
    sw      t0, 0x10(t6)
    sw      t5, 0x18(t6)
    b       2f
    nop
1:
    addu    t4, 1
    bgtu    t9, t4, 1f
    nop
    //t9%16 == 3
    TM_PRINTSTR("Pattern 0010 ....\r\n")
    sw      t0, 0x0(t6)
    sw      t0, 0x8(t6)
    sw      t5, 0x10(t6)
    sw      t0, 0x18(t6)
    b       2f
    nop
1:
    addu    t4, 1
    bgtu    t9, t4, 1f
    nop
    //t9%16 == 4
    TM_PRINTSTR("Pattern 0011 ....\r\n")
    sw      t0, 0x0(t6)
    sw      t0, 0x8(t6)
    sw      t5, 0x10(t6)
    sw      t5, 0x18(t6)
    b       2f
    nop
1:
    addu    t4, 1
    bgtu    t9, t4, 1f
    nop
    //t9%16 == 5
    TM_PRINTSTR("Pattern 0100 ....\r\n")
    sw      t0, 0x0(t6)
    sw      t5, 0x8(t6)
    sw      t0, 0x10(t6)
    sw      t0, 0x18(t6)
    b       2f
    nop
1:
    addu    t4, 1
    bgtu    t9, t4, 1f
    nop
    //t9%16 == 6
    TM_PRINTSTR("Pattern 0101 ....\r\n")
    sw      t0, 0x0(t6)
    sw      t5, 0x8(t6)
    sw      t0, 0x10(t6)
    sw      t5, 0x18(t6)
    b       2f
    nop
1:
    addu    t4, 1
    bgtu    t9, t4, 1f
    nop
    //t9%16 == 7
    TM_PRINTSTR("Pattern 0110 ....\r\n")
    sw      t0, 0x0(t6)
    sw      t5, 0x8(t6)
    sw      t5, 0x10(t6)
    sw      t0, 0x18(t6)
    b       2f
    nop
1:
    addu    t4, 1
    bgtu    t9, t4, 1f
    nop
    //t9%16 == 8
    TM_PRINTSTR("Pattern 0111 ....\r\n")
    sw      t0, 0x0(t6)
    sw      t5, 0x8(t6)
    sw      t5, 0x10(t6)
    sw      t5, 0x18(t6)
    b       2f
    nop
1:
    addu    t4, 1
    bgtu    t9, t4, 1f
    nop
    //t9%16 == 9
    TM_PRINTSTR("Pattern 1000 ....\r\n")
    sw      t5, 0x0(t6)
    sw      t0, 0x8(t6)
    sw      t0, 0x10(t6)
    sw      t0, 0x18(t6)
    b       2f
    nop
1:
    addu    t4, 1
    bgtu    t9, t4, 1f
    nop
    //t9%16 == 10
    TM_PRINTSTR("Pattern 1001 ....\r\n")
    sw      t5, 0x0(t6)
    sw      t0, 0x8(t6)
    sw      t0, 0x10(t6)
    sw      t5, 0x18(t6)
    b       2f
    nop
1:
    addu    t4, 1
    bgtu    t9, t4, 1f
    nop
    //t9%16 == 11
    TM_PRINTSTR("Pattern 1010 ....\r\n")
    sw      t5, 0x0(t6)
    sw      t0, 0x8(t6)
    sw      t5, 0x10(t6)
    sw      t0, 0x18(t6)
    b       2f
    nop
1:
    addu    t4, 1
    bgtu    t9, t4, 1f
    nop
    //t9%16 == 12
    TM_PRINTSTR("Pattern 1011 ....\r\n")
    sw      t5, 0x0(t6)
    sw      t0, 0x8(t6)
    sw      t5, 0x10(t6)
    sw      t5, 0x18(t6)
    b       2f
    nop
1:
    addu    t4, 1
    bgtu    t9, t4, 1f
    nop
    //t9%16 == 13
    TM_PRINTSTR("Pattern 1100 ....\r\n")
    sw      t5, 0x0(t6)
    sw      t5, 0x8(t6)
    sw      t0, 0x10(t6)
    sw      t0, 0x18(t6)
    b       2f
    nop
1:
    addu    t4, 1
    bgtu    t9, t4, 1f
    nop
    //t9%16 == 14
    TM_PRINTSTR("Pattern 1101 ....\r\n")
    sw      t5, 0x0(t6)
    sw      t5, 0x8(t6)
    sw      t0, 0x10(t6)
    sw      t5, 0x18(t6)
    b       2f
    nop
1:
    addu    t4, 1
    bgtu    t9, t4, 1f
    nop
    //t9%16 == 15
    TM_PRINTSTR("Pattern 1110 ....\r\n")
    sw      t5, 0x0(t6)
    sw      t5, 0x8(t6)
    sw      t5, 0x10(t6)
    sw      t0, 0x18(t6)
    b       2f
    nop
1:
    addu    t4, 1
    bgtu    t9, t4, 1f
    nop
    //t9%16 == 16
    TM_PRINTSTR("Pattern 1111 ....\r\n")
    sw      t5, 0x0(t6)
    sw      t5, 0x8(t6)
    sw      t5, 0x10(t6)
    sw      t5, 0x18(t6)
    b       2f
    nop
2:
    li      t8, TM_MAX_ERRORS
//write memory
    lw      a0, 0x0(t6)
    lw      a1, 0x8(t6)
    lw      a2, 0x10(t6)
    lw      a3, 0x18(t6)
1:
    sw      a0, 0x0(t1)
    sw      a1, 0x8(t1)
    sw      a2, 0x10(t1)
    sw      a3, 0x18(t1)
    addu    t1, t1, t2
    bltu    t1, t3, 1b
    nop
    sync
	TM_PRINTSTR("write done. begin to read and compare...\r\n")
//read memory and compare
    //set Test Base t1
    li      t1, MEM_TEST_BASE
    GET_TM_NODE_ID_a1
    sll     a1, a1, 44
    addu    t1, t1, a1
    GET_TM_START_ADDR
    addu    t1, t1, a1
1:
    lw      t0, 0x0(t6)
    lw      t4, 0x0(t1)
    beq     t4, t0, 2f
    nop
    //error detected!!! print address,expected data,read data and reread data
    bal     hexserial_1
    nop
    bltz    t8, 10b  //detect enough errors, go to next test
    nop
2:
    lw      t0, 0x8(t6)
    lw      t4, 0x8(t1)
    beq     t4, t0, 2f
    nop
    //error detected!!! print address,expected data,read data and reread data
    addu    t1, t1, 0x8 //the hexserial_1 will use t1 directly
    bal     hexserial_1
    nop
    addu    t1, t1, -0x8
    bltz    t8, 10b  //detect enough errors, go to next test
    nop
2:
    lw      t0, 0x10(t6)
    lw      t4, 0x10(t1)
    beq     t4, t0, 2f
    nop
    //error detected!!! print address,expected data,read data and reread data
    addu    t1, t1, 0x10 //the hexserial_1 will use t1 directly
    bal     hexserial_1
    nop
    addu    t1, t1, -0x10
    bltz    t8, 10b  //detect enough errors, go to next test
    nop
2:
    lw      t0, 0x18(t6)
    lw      t4, 0x18(t1)
    beq     t4, t0, 2f
    nop
    //error detected!!! print address,expected data,read data and reread data
    addu    t1, t1, 0x18 //the hexserial_1 will use t1 directly
    bal     hexserial_1
    nop
    addu    t1, t1, -0x18
    bltz    t8, 10b  //detect enough errors, go to next test
    nop
2:
#ifdef  PRINT_LESS_ERROR
    li      t4, TM_MAX_RPT_ERRORS
    bgt     s6, t4, 3f
    nop
#endif
    addu    t1, t1, t2
    //check address range
    bltu    t1, t3, 1b
    nop
    TM_PRINTSTR("Pattern Testing done.\r\n")
    b       10b
    nop
3:
#endif
#if 1
    TM_PRINTSTR("\r\nPattern WalkOnes Test...\r\n")
    //address interval
    li      t2, 0x100
    //set Test Base t1 and Test Limit t3
    li      t1, MEM_TEST_BASE
    GET_TM_START_ADDR
    addu    t1, t1, a1
    GET_TM_MSIZE
    GET_MICRO_TUNE
    bnez    a2, 21f
    nop
    srl     a1, a1, MACRO_SCALE
21:
    addu    t3, t1, a1
#ifdef  TM_DBG
    GET_DISPRINT_BIT
    bnez    a1, 88f
    nop
    TM_PRINTSTR("Test address range: 0x")
    move    a0, t1
    bal     hexserial
    nop
    TM_PRINTSTR("~0x")
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

    li      t4, PATTERN_D8_0_0
    li      t5, PATTERN_D8_0_1
    li      v0, PATTERN_D8_0_2
    li      v1, PATTERN_D8_0_3

    li      a0, PATTERN_D8_0_4
    li      a1, PATTERN_D8_0_5
    li      a2, PATTERN_D8_0_6
    li      a3, PATTERN_D8_0_7

    li      t8, TM_MAX_ERRORS
//write memory
1:
    sw      t4, 0x0(t1)
    sw      t5, 0x4(t1)
    sw      v0, 0x8(t1)
    sw      v1, 0xc(t1)
    sw      a0, 0x10(t1)
    sw      a1, 0x14(t1)
    sw      a2, 0x18(t1)
    sw      a3, 0x1c(t1)

    addu    t1, t1, t2
    bltu    t1, t3, 1b
    nop
    sync
	TM_PRINTSTR("write done. begin to read and compare...\r\n")
//read memory and compare
    //set Test Base t1
    li      t1, MEM_TEST_BASE
    GET_TM_START_ADDR
    addu    t1, t1, a1
1:
    li      t0, PATTERN_D8_0_0
    lw      t4, 0x0(t1)
    beq     t4, t0, 2f
    nop
    //error detected!!! print address,expected data,read data and reread data
    bal     hexserial_1
    nop
    bltz    t8, 10f  //detect enough errors, go to next test
    nop
2:
    li      t0, PATTERN_D8_0_1
    lw      t4, 0x4(t1)
    beq     t4, t0, 2f
    nop
    //error detected!!! print address,expected data,read data and reread data
    addu    t1, t1, 0x4 //the hexserial_1 will use t1 directly
    bal     hexserial_1
    nop
    addu    t1, t1, -0x4
    bltz    t8, 10f  //detect enough errors, go to next test
    nop
2:
    li      t0, PATTERN_D8_0_2
    lw      t4, 0x8(t1)
    beq     t4, t0, 2f
    nop
    //error detected!!! print address,expected data,read data and reread data
    addu    t1, t1, 0x8 //the hexserial_1 will use t1 directly
    bal     hexserial_1
    nop
    addu    t1, t1, -0x8
    bltz    t8, 10f  //detect enough errors, go to next test
    nop
2:
    li      t0, PATTERN_D8_0_3
    lw      t4, 0xc(t1)
    beq     t4, t0, 2f
    nop
    //error detected!!! print address,expected data,read data and reread data
    addu    t1, t1, 0xc //the hexserial_1 will use t1 directly
    bal     hexserial_1
    nop
    addu    t1, t1, -0xc
    bltz    t8, 10f  //detect enough errors, go to next test
    nop
2:
    li      t0, PATTERN_D8_0_4
    lw      t4, 0x10(t1)
    beq     t4, t0, 2f
    nop
    //error detected!!! print address,expected data,read data and reread data
    addu    t1, t1, 0x10 //the hexserial_1 will use t1 directly
    bal     hexserial_1
    nop
    addu    t1, t1, -0x10
    bltz    t8, 10f  //detect enough errors, go to next test
    nop
2:
    li      t0, PATTERN_D8_0_5
    lw      t4, 0x14(t1)
    beq     t4, t0, 2f
    nop
    //error detected!!! print address,expected data,read data and reread data
    addu    t1, t1, 0x14 //the hexserial_1 will use t1 directly
    bal     hexserial_1
    nop
    addu    t1, t1, -0x14
    bltz    t8, 10f  //detect enough errors, go to next test
    nop
2:
    li      t0, PATTERN_D8_0_6
    lw      t4, 0x18(t1)
    beq     t4, t0, 2f
    nop
    //error detected!!! print address,expected data,read data and reread data
    addu    t1, t1, 0x18 //the hexserial_1 will use t1 directly
    bal     hexserial_1
    nop
    addu    t1, t1, -0x18
    bltz    t8, 10f  //detect enough errors, go to next test
    nop
2:
    li      t0, PATTERN_D8_0_7
    lw      t4, 0x1c(t1)
    beq     t4, t0, 2f
    nop
    //error detected!!! print address,expected data,read data and reread data
    addu    t1, t1, 0x1c //the hexserial_1 will use t1 directly
    bal     hexserial_1
    nop
    addu    t1, t1, -0x1c
    bltz    t8, 10f  //detect enough errors, go to next test
    nop
2:
#ifdef  PRINT_LESS_ERROR
    li      t4, TM_MAX_RPT_ERRORS
    bgt     s6, t4, 3f
    nop
#endif
    addu    t1, t1, t2
    //check address range
    bltu    t1, t3, 1b
    nop
10:
    TM_PRINTSTR("Pattern Testing done.\r\n")

    TM_PRINTSTR("\r\nPattern WalkInvOnes Test...\r\n")
    //address interval
    li      t2, 0x100
    //set Test Base t1 and Test Limit t3
    li      t1, MEM_TEST_BASE
    GET_TM_START_ADDR
    addu    t1, t1, a1
    GET_TM_MSIZE
    GET_MICRO_TUNE
    bnez    a2, 21f
    nop
    srl     a1, a1, MACRO_SCALE
21:
    addu    t3, t1, a1
#ifdef  TM_DBG
    GET_DISPRINT_BIT
    bnez    a1, 88f
    nop
    TM_PRINTSTR("Test address range: 0x")
    move    a0, t1
    bal     hexserial
    nop
    TM_PRINTSTR("~0x")
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

    li      t4, PATTERN_D8_1_0
    li      t5, PATTERN_D8_1_1
    li      v0, PATTERN_D8_1_2
    li      v1, PATTERN_D8_1_3

    li      a0, PATTERN_D8_1_4
    li      a1, PATTERN_D8_1_5
    li      a2, PATTERN_D8_1_6
    li      a3, PATTERN_D8_1_7

    li      t8, TM_MAX_ERRORS
//write memory
1:
    sw      t4, 0x0(t1)
    sw      t5, 0x4(t1)
    sw      v0, 0x8(t1)
    sw      v1, 0xc(t1)
    sw      a0, 0x10(t1)
    sw      a1, 0x14(t1)
    sw      a2, 0x18(t1)
    sw      a3, 0x1c(t1)

    addu    t1, t1, t2
    bltu    t1, t3, 1b
    nop
    sync
	TM_PRINTSTR("write done. begin to read and compare...\r\n")
//read memory and compare
    //set Test Base t1
    li      t1, MEM_TEST_BASE
    GET_TM_START_ADDR
    addu    t1, t1, a1
1:
    li      t0, PATTERN_D8_1_0
    lw      t4, 0x0(t1)
    beq     t4, t0, 2f
    nop
    //error detected!!! print address,expected data,read data and reread data
    bal     hexserial_1
    nop
    bltz    t8, 10f  //detect enough errors, go to next test
    nop
2:
    li      t0, PATTERN_D8_1_1
    lw      t4, 0x4(t1)
    beq     t4, t0, 2f
    nop
    //error detected!!! print address,expected data,read data and reread data
    addu    t1, t1, 0x4 //the hexserial_1 will use t1 directly
    bal     hexserial_1
    nop
    addu    t1, t1, -0x4
    bltz    t8, 10f  //detect enough errors, go to next test
    nop
2:
    li      t0, PATTERN_D8_1_2
    lw      t4, 0x8(t1)
    beq     t4, t0, 2f
    nop
    //error detected!!! print address,expected data,read data and reread data
    addu    t1, t1, 0x8 //the hexserial_1 will use t1 directly
    bal     hexserial_1
    nop
    addu    t1, t1, -0x8
    bltz    t8, 10f  //detect enough errors, go to next test
    nop
2:
    li      t0, PATTERN_D8_1_3
    lw      t4, 0xc(t1)
    beq     t4, t0, 2f
    nop
    //error detected!!! print address,expected data,read data and reread data
    addu    t1, t1, 0xc //the hexserial_1 will use t1 directly
    bal     hexserial_1
    nop
    addu    t1, t1, -0xc
    bltz    t8, 10f  //detect enough errors, go to next test
    nop
2:
    li      t0, PATTERN_D8_1_4
    lw      t4, 0x10(t1)
    beq     t4, t0, 2f
    nop
    //error detected!!! print address,expected data,read data and reread data
    addu    t1, t1, 0x10 //the hexserial_1 will use t1 directly
    bal     hexserial_1
    nop
    addu    t1, t1, -0x10
    bltz    t8, 10f  //detect enough errors, go to next test
    nop
2:
    li      t0, PATTERN_D8_1_5
    lw      t4, 0x14(t1)
    beq     t4, t0, 2f
    nop
    //error detected!!! print address,expected data,read data and reread data
    addu    t1, t1, 0x14 //the hexserial_1 will use t1 directly
    bal     hexserial_1
    nop
    addu    t1, t1, -0x14
    bltz    t8, 10f  //detect enough errors, go to next test
    nop
2:
    li      t0, PATTERN_D8_1_6
    lw      t4, 0x18(t1)
    beq     t4, t0, 2f
    nop
    //error detected!!! print address,expected data,read data and reread data
    addu    t1, t1, 0x18 //the hexserial_1 will use t1 directly
    bal     hexserial_1
    nop
    addu    t1, t1, -0x18
    bltz    t8, 10f  //detect enough errors, go to next test
    nop
2:
    li      t0, PATTERN_D8_1_7
    lw      t4, 0x1c(t1)
    beq     t4, t0, 2f
    nop
    //error detected!!! print address,expected data,read data and reread data
    addu    t1, t1, 0x1c //the hexserial_1 will use t1 directly
    bal     hexserial_1
    nop
    addu    t1, t1, -0x1c
    bltz    t8, 10f  //detect enough errors, go to next test
    nop
2:
#ifdef  PRINT_LESS_ERROR
    li      t4, TM_MAX_RPT_ERRORS
    bgt     s6, t4, 3f
    nop
#endif
    addu    t1, t1, t2
    //check address range
    bltu    t1, t3, 1b
    nop
10:
    TM_PRINTSTR("Pattern Testing done.\r\n")

    TM_PRINTSTR("\r\nPattern WalkZeros Test...\r\n")
    //address interval
    li      t2, 0x100
    //set Test Base t1 and Test Limit t3
    li      t1, MEM_TEST_BASE
    GET_TM_START_ADDR
    addu    t1, t1, a1
    GET_TM_MSIZE
    GET_MICRO_TUNE
    bnez    a2, 21f
    nop
    srl     a1, a1, MACRO_SCALE
21:
    addu    t3, t1, a1
#ifdef  TM_DBG
    GET_DISPRINT_BIT
    bnez    a1, 88f
    nop
    TM_PRINTSTR("Test address range: 0x")
    move    a0, t1
    bal     hexserial
    nop
    TM_PRINTSTR("~0x")
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

    li      t4, PATTERN_D8_2_0
    li      t5, PATTERN_D8_2_1
    li      v0, PATTERN_D8_2_2
    li      v1, PATTERN_D8_2_3

    li      a0, PATTERN_D8_2_4
    li      a1, PATTERN_D8_2_5
    li      a2, PATTERN_D8_2_6
    li      a3, PATTERN_D8_2_7

    li      t8, TM_MAX_ERRORS
//write memory
1:
    sw      t4, 0x0(t1)
    sw      t5, 0x4(t1)
    sw      v0, 0x8(t1)
    sw      v1, 0xc(t1)
    sw      a0, 0x10(t1)
    sw      a1, 0x14(t1)
    sw      a2, 0x18(t1)
    sw      a3, 0x1c(t1)

    addu    t1, t1, t2
    bltu    t1, t3, 1b
    nop
    sync
	TM_PRINTSTR("write done. begin to read and compare...\r\n")
//read memory and compare
    //set Test Base t1
    li      t1, MEM_TEST_BASE
    GET_TM_START_ADDR
    addu    t1, t1, a1
1:
    li      t0, PATTERN_D8_2_0
    lw      t4, 0x0(t1)
    beq     t4, t0, 2f
    nop
    //error detected!!! print address,expected data,read data and reread data
    bal     hexserial_1
    nop
    bltz    t8, 10f  //detect enough errors, go to next test
    nop
2:
    li      t0, PATTERN_D8_2_1
    lw      t4, 0x4(t1)
    beq     t4, t0, 2f
    nop
    //error detected!!! print address,expected data,read data and reread data
    addu    t1, t1, 0x4 //the hexserial_1 will use t1 directly
    bal     hexserial_1
    nop
    addu    t1, t1, -0x4
    bltz    t8, 10f  //detect enough errors, go to next test
    nop
2:
    li      t0, PATTERN_D8_2_2
    lw      t4, 0x8(t1)
    beq     t4, t0, 2f
    nop
    //error detected!!! print address,expected data,read data and reread data
    addu    t1, t1, 0x8 //the hexserial_1 will use t1 directly
    bal     hexserial_1
    nop
    addu    t1, t1, -0x8
    bltz    t8, 10f  //detect enough errors, go to next test
    nop
2:
    li      t0, PATTERN_D8_2_3
    lw      t4, 0xc(t1)
    beq     t4, t0, 2f
    nop
    //error detected!!! print address,expected data,read data and reread data
    addu    t1, t1, 0xc //the hexserial_1 will use t1 directly
    bal     hexserial_1
    nop
    addu    t1, t1, -0xc
    bltz    t8, 10f  //detect enough errors, go to next test
    nop
2:
    li      t0, PATTERN_D8_2_4
    lw      t4, 0x10(t1)
    beq     t4, t0, 2f
    nop
    //error detected!!! print address,expected data,read data and reread data
    addu    t1, t1, 0x10 //the hexserial_1 will use t1 directly
    bal     hexserial_1
    nop
    addu    t1, t1, -0x10
    bltz    t8, 10f  //detect enough errors, go to next test
    nop
2:
    li      t0, PATTERN_D8_2_5
    lw      t4, 0x14(t1)
    beq     t4, t0, 2f
    nop
    //error detected!!! print address,expected data,read data and reread data
    addu    t1, t1, 0x14 //the hexserial_1 will use t1 directly
    bal     hexserial_1
    nop
    addu    t1, t1, -0x14
    bltz    t8, 10f  //detect enough errors, go to next test
    nop
2:
    li      t0, PATTERN_D8_2_6
    lw      t4, 0x18(t1)
    beq     t4, t0, 2f
    nop
    //error detected!!! print address,expected data,read data and reread data
    addu    t1, t1, 0x18 //the hexserial_1 will use t1 directly
    bal     hexserial_1
    nop
    addu    t1, t1, -0x18
    bltz    t8, 10f  //detect enough errors, go to next test
    nop
2:
    li      t0, PATTERN_D8_2_7
    lw      t4, 0x1c(t1)
    beq     t4, t0, 2f
    nop
    //error detected!!! print address,expected data,read data and reread data
    addu    t1, t1, 0x1c //the hexserial_1 will use t1 directly
    bal     hexserial_1
    nop
    addu    t1, t1, -0x1c
    bltz    t8, 10f  //detect enough errors, go to next test
    nop
2:
#ifdef  PRINT_LESS_ERROR
    li      t4, TM_MAX_RPT_ERRORS
    bgt     s6, t4, 3f
    nop
#endif
    addu    t1, t1, t2
    //check address range
    bltu    t1, t3, 1b
    nop
10:
    TM_PRINTSTR("Pattern Testing done.\r\n")
#endif
//pattern JustX Test
#ifndef REDUCED_MEM_TEST
    //initialization
    li      t9, 0
    //address interval
    li      t2, 0x200
    //set Test Base t1 and Test Limit t3
    li      t1, MEM_TEST_BASE
    GET_TM_START_ADDR
    addu    t1, t1, a1
    GET_TM_MSIZE
    srl     a1, a1, 2
    GET_MICRO_TUNE
    bnez    a2, 21f
    nop
    srl     a1, a1, MACRO_SCALE
21:
    addu    t3, t1, a1
10:
    //loop control
    addu    t9, t9, 0x1
1:
    li      t4, 1
    bgtu    t9, t4, 1f
    nop
    //t9 == 1
    TM_PRINTSTR("\r\nPattern JustA Test...\r\n")
    li      t0, PATTERN_JUSTA
    b       2f
    nop
1:
    li      t4, 2
    bgtu    t9, t4, 1f
    nop
    //t9 == 2
    TM_PRINTSTR("\r\nPattern Just5 Test...\r\n")
    li      t0, PATTERN_JUST5
    b       2f
    nop
1:
    li      t4, 3
    bgtu    t9, t4, 1f
    nop
    //t9 == 3
    TM_PRINTSTR("\r\nPattern JustFiveA Test...\r\n")
    li      t0, PATTERN_FiveA
    b       2f
    nop
1:
    li      t4, 4
    bgtu    t9, t4, 1f
    nop
    //t9 == 4
    TM_PRINTSTR("\r\nPattern JustZeroOne Test...\r\n")
    li      t0, PATTERN_ZEROONE
    b       2f
    nop
1:
    li      t4, 5
    bgtu    t9, t4, 1f
    nop
    //t9 == 5
    TM_PRINTSTR("\r\nPattern JustL8b10b-16 Test...\r\n")
    li      t0, PATTERN_L8b10b
    b       2f
    nop
1:
    li      t4, 6
    bgtu    t9, t4, 1f
    nop
    //t9 == 6
    TM_PRINTSTR("\r\nPattern JustS8b10b-b5 Test...\r\n")
    li      t0, PATTERN_S8b10b
    b       2f
    nop
1:
    li      t4, 7
    bgtu    t9, t4, 1f
    nop
    //t9 == 7
    TM_PRINTSTR("\r\nPattern JustFive7 Test...\r\n")
    li      t0, PATTERN_Five7
    b       2f
    nop
1:
    li      t4, 8
    bgtu    t9, t4, 1f
    nop
    //t9 == 8
    TM_PRINTSTR("\r\nPattern JustZero2fd Test...\r\n")
    li      t0, PATTERN_Zero2fd
    b       2f
    nop
1:  
    // t9 > 8 (all the JustX Pattern test done)
    b       3f  //go to the end of this loop
    nop
2:
    li      t8, TM_MAX_ERRORS
    //set Test Base t1
    li      t1, MEM_TEST_BASE
    GET_TM_START_ADDR
    addu    t1, t1, a1
#ifdef  TM_DBG
    GET_DISPRINT_BIT
    bnez    a1, 88f
    nop
    TM_PRINTSTR("Test address range: 0x")
    srl     a0, t1, 32
    bal     hexserial
    nop
    move    a0, t1
    bal     hexserial
    nop
    TM_PRINTSTR("~0x")
    srl     a0, t3, 32
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
    sw      t0, 0x0(t1)
    sw      t0, 0x8(t1)
    addu    t1, t1, t2
    bltu    t1, t3, 1b
    nop
    sync
	TM_PRINTSTR("write done. begin to read and compare...\r\n")
//read memory and compare
    //set Test Base t1
    li      t1, MEM_TEST_BASE
    GET_TM_START_ADDR
    addu    t1, t1, a1
1:
    lw      t4, 0x0(t1)
    beq     t4, t0, 2f
    nop
    //error detected!!! print address,expected data,read data and reread data
    bal     hexserial_1
    nop
    bltz    t8, 10b  //detect enough errors, go to next test
    nop
2:
    lw      t4, 0x8(t1)
    beq     t4, t0, 2f
    nop
    //error detected!!! print address,expected data,read data and reread data
    addu    t1, t1, 0x8 //the hexserial_1 will use t1 directly
    bal     hexserial_1
    nop
    addu    t1, t1, -0x8
    bltz    t8, 10b  //detect enough errors, go to next test
    nop
2:
#ifdef  PRINT_LESS_ERROR
    li      t4, TM_MAX_RPT_ERRORS
    bgt     s6, t4, 3f
    nop
#endif
    addu    t1, t1, t2
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
//olw code
    li      v0, 0x0
    beqz    s6, 1f
    nop
    //s6 != 0, set error mark
    li      v0, 0x1
1:
#else
    move    v0, s6
    move    v1, s7
#endif

    jr      t6
    nop
//===================================

/**********************
* input:
    s1: Test mem size, Test start addr 
    t0: write content
    t1: 
**********************/

simple_test_mem:
    move    t6, ra
    move    s4, t1

    li      t0, PATTERN_DB_0_0
    TM_PRINTSTR("\r\nStart simple test mem...\r\n")
    //set Test Base t1 and Test Limit t3
    li      t1, SIMPLE_TM_BASE
    GET_TM_START_ADDR
    addu    t1, t1, a1
    GET_TM_MSIZE
    addu    t3, t1, a1

    TM_PRINTSTR("\r\nStart write data to Memory...")
    //address interval
    li      t2, 0x20
    //set loop times
    li      t9, TMF_PWRLOOP
2:
    addu    t9, -1
    move    t4, t1
1:
    sw      t0, 0x0(t4)
    not     t0, t0
    sw      t0, 0x4(t4)
    not     t0, t0
    sw      t0, 0x8(t4)
    not     t0, t0
    sw      t0, 0xc(t4)
    not     t0, t0

    addu    t4, t4, t2
    bltu    t4, t3, 1b
    nop
    bnez    t9, 2b
    nop
    TM_PRINTSTR("\r\nData write done.")

    TM_PRINTSTR("\r\nStart Read Memory...")
    //address interval
    li      t2, 0x20
    //set loop times
    li      t9, TMF_PRDLOOP
20:
    addu    t9, -1
    //set Test Base t1
    li      t1, SIMPLE_TM_BASE
    GET_TM_START_ADDR
    addu    t1, t1, a1
10:
    lw      t4, 0x0(t1)
    beq     t4, t0, 2f
    nop
    //error detected!!! print address,expected data,read data and reread data
    bal     hexserial_1
    nop
2:
    not     t0, t0
    lw      t4, 0x4(t1)
    beq     t4, t0, 2f
    nop
    //error detected!!! print address,expected data,read data and reread data
    addu    t1, t1, 0x4 //the hexserial_1 will use t1 directly
    bal     hexserial_1
    nop
    addu    t1, t1, -0x4
2:
    not     t0, t0
    lw      t4, 0x8(t1)
    beq     t4, t0, 2f
    nop
    //error detected!!! print address,expected data,read data and reread data
    addu    t1, t1, 0x8 //the hexserial_1 will use t1 directly
    bal     hexserial_1
    nop
    addu    t1, t1, -0x8
2:
    not     t0, t0
    lw      t4, 0xc(t1)
    beq     t4, t0, 2f
    nop
    //error detected!!! print address,expected data,read data and reread data
    addu    t1, t1, 0xc //the hexserial_1 will use t1 directly
    bal     hexserial_1
    nop
    addu    t1, t1, -0xc
2:
    not     t0, t0

    addu    t1, t1, t2
    bltu    t1, t3, 10b
    nop
    bnez    t9, 20b
    nop
    TM_PRINTSTR("\r\nReading done.")

    jr      t6
    nop
//===================================
//================================
test_mem_1:
    move    t6, ra
    move    s4, t1

    li      s6, 0x0
    li      s7, 0x0

	TM_PRINTSTR("\r\nStart Testing Memory...\r\n")

    TM_PRINTSTR("\r\nPattern DB_0 Test-----\r\n")
    //address interval
    li      t2, 0x200
    //set Test Base t1 and Test Limit t3
    li      t1, SIMPLE_TM_BASE
    GET_TM_START_ADDR
    addu    t1, t1, a1
    GET_TM_MSIZE
    addu    t3, t1, a1

#ifdef  TM_DBG
    GET_DISPRINT_BIT
    bnez    a1, 88f
    nop
    TM_PRINTSTR("Test address range: 0x")
    move    a0, t1
    bal     hexserial
    nop
    TM_PRINTSTR("~0x")
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
    li      t9, 0x4000000

10:
    addu    t9, t9, -0x1
    //set Test Base t1 and Test Limit t3
    li      t1, SIMPLE_TM_BASE
    GET_TM_START_ADDR
    addu    t1, t1, a1
    GET_TM_MSIZE
    addu    t3, t1, a1

    li      t8, TM_MAX_ERRORS
    li      t0, PATTERN_DB_0_0
//write memory
    move    a0, t0
    not     a1, t0
    not     a2, t0
    move    a3, t0
1:
    sw      a0, 0x0(t1)
    sw      a1, 0x8(t1)
    sw      a2, 0x10(t1)
    sw      a3, 0x18(t1)
    addu    t1, t1, t2
    bltu    t1, t3, 1b
    nop
    sync
	TM_PRINTSTR("write done. begin to read and compare...\r\n")
//read memory and compare
    //set Test Base t1
    li      t1, MEM_TEST_BASE
    GET_TM_START_ADDR
    addu    t1, t1, a1
1:
    lw      t4, 0x0(t1)
    beq     t4, t0, 2f
    nop
    //error detected!!! print address,expected data,read data and reread data
    bal     hexserial_1
    nop
    bltz    t8, 10b  //detect enough errors, go to next test
    nop
2:
    not     t0, t0
    lw      t4, 0x8(t1)
    beq     t4, t0, 2f
    nop
    //error detected!!! print address,expected data,read data and reread data
    addu    t1, t1, 0x8 //the hexserial_1 will use t1 directly
    bal     hexserial_1
    nop
    addu    t1, t1, -0x8
    bltz    t8, 10b  //detect enough errors, go to next test
    nop
2:
    lw      t4, 0x10(t1)
    beq     t4, t0, 2f
    nop
    //error detected!!! print address,expected data,read data and reread data
    addu    t1, t1, 0x10 //the hexserial_1 will use t1 directly
    bal     hexserial_1
    nop
    addu    t1, t1, -0x10
    bltz    t8, 10b  //detect enough errors, go to next test
    nop
2:
    not     t0, t0
    lw      t4, 0x18(t1)
    beq     t4, t0, 2f
    nop
    //error detected!!! print address,expected data,read data and reread data
    addu    t1, t1, 0x18 //the hexserial_1 will use t1 directly
    bal     hexserial_1
    nop
    addu    t1, t1, -0x18
    bltz    t8, 10b  //detect enough errors, go to next test
    nop
2:
    addu    t1, t1, t2
    //check address range
    bltu    t1, t3, 1b
    nop

    bnez    t9, 10b
    nop
    TM_PRINTSTR("Pattern Testing done.\r\n")

#ifdef  LEVEL_SPECIFIED_BYTE_LANES
    and     s6, s6, s5
    and     s7, s7, s5
#endif
#if 0
//olw code
    li      v0, 0x0
    beqz    s6, 1f
    nop
    //s6 != 0, set error mark
    li      v0, 0x1
1:
#else
    move    v0, s6
    move    v1, s7
#endif

    jr      t6
    nop
//===================================

hexserial_1:  //pseudo subroutine
/**********************
input:  t1: read address(read only)
        t0: expected data(read only)
        t4: read data
use reg:t5, t7
***********************/
    move    t7, ra
    xor     a0, t0, t4
    or      s6, s6, a0
#ifdef  TM_DBG
    addu    t8, t8, -0x1
#endif
    /* reread the wrong bytes */
#if 1
#if 0
    //Hit Invalidate the Primary D-cache and Second cache.
    //TM_PRINTSTR("\r\nInvalidate Primary D-cache and S-cache.\r\n")
    cache   0x11, 0x0(t1)
    cache   0x13, 0x0(t1)
    sync
    move    t5, t1
#else
    li      t5, 0xa0000000
    or      t5, t1, t5
#endif
#else
    move    t5, t1
#endif
    lw      t5, 0(t5)
    nop
#ifdef  TM_DBG
    GET_DISPRINT_BIT
    bnez    a1, 88f
    nop
	TM_PRINTSTR("addr 0x")
	move	a0, t1
	bal	    hexserial
	nop
    TM_PRINTSTR(" expected: ")
	move	a0, t0
	bal	    hexserial
	nop
    TM_PRINTSTR(" read: ")
	move	a0, t4
	bal	    hexserial
	nop
    TM_PRINTSTR(" reread: ")
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
RL_end:
