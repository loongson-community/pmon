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
********************/
/***************
use register: s1, s4, t8, a0~a3, v0~v1
note: don't change s0, s1, s2(msize)

t0: test pattern content
t1: current address
t2: address interval
t3: max address
t4,t5: volatile
t7: by subroutine--hexserial64_1
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
#include    "Test_Mem_S2.h"

#define TM_DBG
#ifdef  TM_DBG
#define	TM_PRINTSTR(x) \
	.rdata;98: .asciz x; .text; GET_DISPRINT_BIT; bnez a1, 97f; nop; la a0, 98b; bal stringserial; nop; 97:
#else
#define	TM_PRINTSTR(x) ;
#endif

#if 0
//call example
#ifdef  DEBUG_DDR

	PRINTSTR("\r\nDo test?(0xf: skip): ")
	bal     inputaddress
	nop
	and     v0, v0, 0xf
	dli     a1, 0x1
	bgt     v0, a1, 2f
	nop


	dli     s1, 0x0004000000000000
	PRINTSTR("\r\ndefault s1 = 0x");
	dsrl    a0, s1, 32
	bal     hexserial
	nop
	PRINTSTR("__")
	move    a0, s1
	bal     hexserial
	nop
	PRINTSTR("\r\nChange test param s1(0: skip)?: ")
	bal     inputaddress
	nop
	beqz    v0, 1f
	nop
	move    s1, v0
1:
	dli     t1, 0x0010
	bal     test_mem
	nop
	move    t1, v0
	PRINTSTR("\r\n")
	dsrl    a0, t1, 32
	bal     hexserial
	nop
	move    a0, t1
	bal     hexserial
	nop
	beqz    t1, 2f
	nop
	PRINTSTR("  Error found!!\r\n")
2:

#endif
#endif

    b       RL_end
    nop

/********************************
 * test_mem
 * input:   s1, t1
 s1:
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
    move    t6, ra
    move    s4, t1

TM_start:
     
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

#if 1
	TM_PRINTSTR("Uncached Address Stuck Testing...\r\n")
//debug fatal errors
//write memory
    //set t1 to Test Base
    //use uncached address space here
	dli 	t1, UNCACHED_MEM_TEST_BASE
    GET_TM_START_ADDR
    daddu   t1, t1, a1
#ifdef   PRINT_LESS_ERROR
    dli     t8, 32  //max print errors 32 
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
    bltz    t8, 3f  //detect enough errors, go to next test
    nop
#endif
2:
    daddiu  t1, t1, 0x8
    daddiu  t3, t3, -1
    bnez 	t3, 1b
	nop
	TM_PRINTSTR("Stuck Testing done!\r\n")
3:
#endif
#if 1
    //Test whether the addr_pins,column_size,bank,rank param right.
	TM_PRINTSTR("\r\nAddress Stuck Testing all space...\r\n")
    //set t1 to Test Base
	dli 	t1, UNCACHED_MEM_TEST_BASE
    GET_TM_START_ADDR
    daddu   t1, t1, a1
    //set test limit t3
    GET_TM_MSIZE
    daddu   t3, t1, a1
#ifdef   PRINT_LESS_ERROR
    dli     t8, TM_MAX_ERRORS
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
	dli 	t1, UNCACHED_MEM_TEST_BASE
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
#if 1
    //Test whether the addr_pins,column_size,bank,rank param right.
	TM_PRINTSTR("\r\nAddress Stuck Testing all space...\r\n")
    //set t1 to Test Base
    dli     t1, MEM_TEST_BASE
    GET_TM_START_ADDR
    daddu   t1, t1, a1
    //set test limit t3
    GET_TM_MSIZE
    daddu   t3, t1, a1
#ifdef   PRINT_LESS_ERROR
    dli     t8, TM_MAX_ERRORS
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
//-------------------
#if 1
    TM_PRINTSTR("\r\nPattern WalkOnes Test...\r\n")
    //address interval
    dli     t2, 0x100
    //set Test Base t1 and Test Limit t3
    dli     t1, MEM_TEST_BASE
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

    dli     t4, PATTERN_D8_0_0
    dli     t5, PATTERN_D8_0_1
    dli     v0, PATTERN_D8_0_2
    dli     v1, PATTERN_D8_0_3

    dli     a0, PATTERN_D8_0_4
    dli     a1, PATTERN_D8_0_5
    dli     a2, PATTERN_D8_0_6
    dli     a3, PATTERN_D8_0_7

    dli     t8, TM_MAX_ERRORS
//write memory
1:
    sd      t4, 0x0(t1)
    sd      t5, 0x8(t1)
    sd      v0, 0x10(t1)
    sd      v1, 0x18(t1)
    sd      a0, 0x20(t1)
    sd      a1, 0x28(t1)
    sd      a2, 0x30(t1)
    sd      a3, 0x38(t1)

    daddu   t1, t1, t2
    bltu    t1, t3, 1b
    nop
    sync
	TM_PRINTSTR("write done. begin to read and compare...\r\n")
//read memory and compare
    //set Test Base t1
    dli     t1, MEM_TEST_BASE
    GET_TM_START_ADDR
    daddu   t1, t1, a1
1:
    dli     t0, PATTERN_D8_0_0
    ld      t4, 0x0(t1)
    beq     t4, t0, 2f
    nop
    //error detected!!! print address,expected data,read data and reread data
    bal     hexserial64_1
    nop
    bltz    t8, 10f  //detect enough errors, go to next test
    nop
2:
    dli     t0, PATTERN_D8_0_1
    ld      t4, 0x8(t1)
    beq     t4, t0, 2f
    nop
    //error detected!!! print address,expected data,read data and reread data
    daddiu  t1, t1, 0x8 //the hexserial64_1 will use t1 directly
    bal     hexserial64_1
    nop
    daddiu  t1, t1, -0x8
    bltz    t8, 10f  //detect enough errors, go to next test
    nop
2:
    dli     t0, PATTERN_D8_0_2
    ld      t4, 0x10(t1)
    beq     t4, t0, 2f
    nop
    //error detected!!! print address,expected data,read data and reread data
    daddiu  t1, t1, 0x10 //the hexserial64_1 will use t1 directly
    bal     hexserial64_1
    nop
    daddiu  t1, t1, -0x10
    bltz    t8, 10f  //detect enough errors, go to next test
    nop
2:
    dli     t0, PATTERN_D8_0_3
    ld      t4, 0x18(t1)
    beq     t4, t0, 2f
    nop
    //error detected!!! print address,expected data,read data and reread data
    daddiu  t1, t1, 0x18 //the hexserial64_1 will use t1 directly
    bal     hexserial64_1
    nop
    daddiu  t1, t1, -0x18
    bltz    t8, 10f  //detect enough errors, go to next test
    nop
2:
    dli     t0, PATTERN_D8_0_4
    ld      t4, 0x20(t1)
    beq     t4, t0, 2f
    nop
    //error detected!!! print address,expected data,read data and reread data
    daddiu  t1, t1, 0x20 //the hexserial64_1 will use t1 directly
    bal     hexserial64_1
    nop
    daddiu  t1, t1, -0x20
    bltz    t8, 10f  //detect enough errors, go to next test
    nop
2:
    dli     t0, PATTERN_D8_0_5
    ld      t4, 0x28(t1)
    beq     t4, t0, 2f
    nop
    //error detected!!! print address,expected data,read data and reread data
    daddiu  t1, t1, 0x28 //the hexserial64_1 will use t1 directly
    bal     hexserial64_1
    nop
    daddiu  t1, t1, -0x28
    bltz    t8, 10f  //detect enough errors, go to next test
    nop
2:
    dli     t0, PATTERN_D8_0_6
    ld      t4, 0x30(t1)
    beq     t4, t0, 2f
    nop
    //error detected!!! print address,expected data,read data and reread data
    daddiu  t1, t1, 0x30 //the hexserial64_1 will use t1 directly
    bal     hexserial64_1
    nop
    daddiu  t1, t1, -0x30
    bltz    t8, 10f  //detect enough errors, go to next test
    nop
2:
    dli     t0, PATTERN_D8_0_7
    ld      t4, 0x38(t1)
    beq     t4, t0, 2f
    nop
    //error detected!!! print address,expected data,read data and reread data
    daddiu  t1, t1, 0x38 //the hexserial64_1 will use t1 directly
    bal     hexserial64_1
    nop
    daddiu  t1, t1, -0x38
    bltz    t8, 10f  //detect enough errors, go to next test
    nop
2:
    daddu   t1, t1, t2
    //check address range
    bltu    t1, t3, 1b
    nop
10:
    TM_PRINTSTR("Pattern Testing done.\r\n")

    TM_PRINTSTR("\r\nPattern WalkInvOnes Test...\r\n")
    //address interval
    dli     t2, 0x100
    //set Test Base t1 and Test Limit t3
    dli     t1, MEM_TEST_BASE
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

    dli     t4, PATTERN_D8_0_0
    dli     t5, PATTERN_D8_0_1
    dli     v0, PATTERN_D8_0_2
    dli     v1, PATTERN_D8_0_3

    dli     a0, PATTERN_D8_0_4
    dli     a1, PATTERN_D8_0_5
    dli     a2, PATTERN_D8_0_6
    dli     a3, PATTERN_D8_0_7

    dli     t8, TM_MAX_ERRORS
//write memory
1:
    sd      t4, 0x0(t1)
    sd      t5, 0x8(t1)
    sd      v0, 0x10(t1)
    sd      v1, 0x18(t1)
    sd      a0, 0x20(t1)
    sd      a1, 0x28(t1)
    sd      a2, 0x30(t1)
    sd      a3, 0x38(t1)

    daddu   t1, t1, t2
    bltu    t1, t3, 1b
    nop
    sync
	TM_PRINTSTR("write done. begin to read and compare...\r\n")
//read memory and compare
    //set Test Base t1
    dli     t1, MEM_TEST_BASE
    GET_TM_START_ADDR
    daddu   t1, t1, a1
1:
    dli     t0, PATTERN_D8_0_0
    ld      t4, 0x0(t1)
    beq     t4, t0, 2f
    nop
    //error detected!!! print address,expected data,read data and reread data
    bal     hexserial64_1
    nop
    bltz    t8, 10f  //detect enough errors, go to next test
    nop
2:
    dli     t0, PATTERN_D8_0_1
    ld      t4, 0x8(t1)
    beq     t4, t0, 2f
    nop
    //error detected!!! print address,expected data,read data and reread data
    daddiu  t1, t1, 0x8 //the hexserial64_1 will use t1 directly
    bal     hexserial64_1
    nop
    daddiu  t1, t1, -0x8
    bltz    t8, 10f  //detect enough errors, go to next test
    nop
2:
    dli     t0, PATTERN_D8_0_2
    ld      t4, 0x10(t1)
    beq     t4, t0, 2f
    nop
    //error detected!!! print address,expected data,read data and reread data
    daddiu  t1, t1, 0x10 //the hexserial64_1 will use t1 directly
    bal     hexserial64_1
    nop
    daddiu  t1, t1, -0x10
    bltz    t8, 10f  //detect enough errors, go to next test
    nop
2:
    dli     t0, PATTERN_D8_0_3
    ld      t4, 0x18(t1)
    beq     t4, t0, 2f
    nop
    //error detected!!! print address,expected data,read data and reread data
    daddiu  t1, t1, 0x18 //the hexserial64_1 will use t1 directly
    bal     hexserial64_1
    nop
    daddiu  t1, t1, -0x18
    bltz    t8, 10f  //detect enough errors, go to next test
    nop
2:
    dli     t0, PATTERN_D8_0_4
    ld      t4, 0x20(t1)
    beq     t4, t0, 2f
    nop
    //error detected!!! print address,expected data,read data and reread data
    daddiu  t1, t1, 0x20 //the hexserial64_1 will use t1 directly
    bal     hexserial64_1
    nop
    daddiu  t1, t1, -0x20
    bltz    t8, 10f  //detect enough errors, go to next test
    nop
2:
    dli     t0, PATTERN_D8_0_5
    ld      t4, 0x28(t1)
    beq     t4, t0, 2f
    nop
    //error detected!!! print address,expected data,read data and reread data
    daddiu  t1, t1, 0x28 //the hexserial64_1 will use t1 directly
    bal     hexserial64_1
    nop
    daddiu  t1, t1, -0x28
    bltz    t8, 10f  //detect enough errors, go to next test
    nop
2:
    dli     t0, PATTERN_D8_0_6
    ld      t4, 0x30(t1)
    beq     t4, t0, 2f
    nop
    //error detected!!! print address,expected data,read data and reread data
    daddiu  t1, t1, 0x30 //the hexserial64_1 will use t1 directly
    bal     hexserial64_1
    nop
    daddiu  t1, t1, -0x30
    bltz    t8, 10f  //detect enough errors, go to next test
    nop
2:
    dli     t0, PATTERN_D8_0_7
    ld      t4, 0x38(t1)
    beq     t4, t0, 2f
    nop
    //error detected!!! print address,expected data,read data and reread data
    daddiu  t1, t1, 0x38 //the hexserial64_1 will use t1 directly
    bal     hexserial64_1
    nop
    daddiu  t1, t1, -0x38
    bltz    t8, 10f  //detect enough errors, go to next test
    nop
2:
    daddu   t1, t1, t2
    //check address range
    bltu    t1, t3, 1b
    nop
10:
    TM_PRINTSTR("Pattern Testing done.\r\n")

    TM_PRINTSTR("\r\nPattern WalkSingleOnes Test...\r\n")
    //address interval
    dli     t2, 0x100
    //set Test Base t1 and Test Limit t3
    dli     t1, MEM_TEST_BASE
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

    dli     t4, PATTERN_D8_0_0
    dli     t5, PATTERN_D8_0_1
    dli     v0, PATTERN_D8_0_2
    dli     v1, PATTERN_D8_0_3

    dli     a0, PATTERN_D8_0_4
    dli     a1, PATTERN_D8_0_5
    dli     a2, PATTERN_D8_0_6
    dli     a3, PATTERN_D8_0_7

    dli     t8, TM_MAX_ERRORS
//write memory
1:
    sd      t4, 0x0(t1)
    sd      t5, 0x8(t1)
    sd      v0, 0x10(t1)
    sd      v1, 0x18(t1)
    sd      a0, 0x20(t1)
    sd      a1, 0x28(t1)
    sd      a2, 0x30(t1)
    sd      a3, 0x38(t1)

    daddu   t1, t1, t2
    bltu    t1, t3, 1b
    nop
    sync
	TM_PRINTSTR("write done. begin to read and compare...\r\n")
//read memory and compare
    //set Test Base t1
    dli     t1, MEM_TEST_BASE
    GET_TM_START_ADDR
    daddu   t1, t1, a1
1:
    dli     t0, PATTERN_D8_0_0
    ld      t4, 0x0(t1)
    beq     t4, t0, 2f
    nop
    //error detected!!! print address,expected data,read data and reread data
    bal     hexserial64_1
    nop
    bltz    t8, 10f  //detect enough errors, go to next test
    nop
2:
    dli     t0, PATTERN_D8_0_1
    ld      t4, 0x8(t1)
    beq     t4, t0, 2f
    nop
    //error detected!!! print address,expected data,read data and reread data
    daddiu  t1, t1, 0x8 //the hexserial64_1 will use t1 directly
    bal     hexserial64_1
    nop
    daddiu  t1, t1, -0x8
    bltz    t8, 10f  //detect enough errors, go to next test
    nop
2:
    dli     t0, PATTERN_D8_0_2
    ld      t4, 0x10(t1)
    beq     t4, t0, 2f
    nop
    //error detected!!! print address,expected data,read data and reread data
    daddiu  t1, t1, 0x10 //the hexserial64_1 will use t1 directly
    bal     hexserial64_1
    nop
    daddiu  t1, t1, -0x10
    bltz    t8, 10f  //detect enough errors, go to next test
    nop
2:
    dli     t0, PATTERN_D8_0_3
    ld      t4, 0x18(t1)
    beq     t4, t0, 2f
    nop
    //error detected!!! print address,expected data,read data and reread data
    daddiu  t1, t1, 0x18 //the hexserial64_1 will use t1 directly
    bal     hexserial64_1
    nop
    daddiu  t1, t1, -0x18
    bltz    t8, 10f  //detect enough errors, go to next test
    nop
2:
    dli     t0, PATTERN_D8_0_4
    ld      t4, 0x20(t1)
    beq     t4, t0, 2f
    nop
    //error detected!!! print address,expected data,read data and reread data
    daddiu  t1, t1, 0x20 //the hexserial64_1 will use t1 directly
    bal     hexserial64_1
    nop
    daddiu  t1, t1, -0x20
    bltz    t8, 10f  //detect enough errors, go to next test
    nop
2:
    dli     t0, PATTERN_D8_0_5
    ld      t4, 0x28(t1)
    beq     t4, t0, 2f
    nop
    //error detected!!! print address,expected data,read data and reread data
    daddiu  t1, t1, 0x28 //the hexserial64_1 will use t1 directly
    bal     hexserial64_1
    nop
    daddiu  t1, t1, -0x28
    bltz    t8, 10f  //detect enough errors, go to next test
    nop
2:
    dli     t0, PATTERN_D8_0_6
    ld      t4, 0x30(t1)
    beq     t4, t0, 2f
    nop
    //error detected!!! print address,expected data,read data and reread data
    daddiu  t1, t1, 0x30 //the hexserial64_1 will use t1 directly
    bal     hexserial64_1
    nop
    daddiu  t1, t1, -0x30
    bltz    t8, 10f  //detect enough errors, go to next test
    nop
2:
    dli     t0, PATTERN_D8_0_7
    ld      t4, 0x38(t1)
    beq     t4, t0, 2f
    nop
    //error detected!!! print address,expected data,read data and reread data
    daddiu  t1, t1, 0x38 //the hexserial64_1 will use t1 directly
    bal     hexserial64_1
    nop
    daddiu  t1, t1, -0x38
    bltz    t8, 10f  //detect enough errors, go to next test
    nop
2:
    daddu   t1, t1, t2
    //check address range
    bltu    t1, t3, 1b
    nop
10:
    TM_PRINTSTR("Pattern Testing done.\r\n")
#endif
//pattern JustX Test
#ifndef REDUCED_MEM_TEST
    //initialization
    dli     t9, 0
    //address interval
    dli     t2, 0x10
    //set Test Base t1 and Test Limit t3
    dli     t1, MEM_TEST_BASE
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
    dli     t8, TM_MAX_ERRORS
    //set Test Base t1
    dli     t1, MEM_TEST_BASE
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

    move    ra, t6
    jr      ra
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

	TM_PRINTSTR("\r\nStart simple test mem...\r\n")
    //set Test Base t1 and Test Limit t3
    dli     t1, SIMPLE_TM_BASE
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
    move    t4, t1
1:
    sd      t0, 0x0(t4)
    not     t0, t0
    sd      t0, 0x8(t4)
    not     t0, t0
    sd      t0, 0x10(t4)
    not     t0, t0
    sd      t0, 0x18(t4)
    not     t0, t0

    daddu   t4, t4, t2
    bltu    t4, t3, 1b
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

    move    ra, t6
    jr      ra
    nop
//===================================
//================================
test_mem_1:
    move    t6, ra
    move    s4, t1

    dli     s6, 0x0
    dli     s7, 0x0

	TM_PRINTSTR("\r\nStart Testing Memory...\r\n")

    TM_PRINTSTR("\r\nPattern DB_0 Test-----\r\n")
    //address interval
    dli     t2, 0x200
    //set Test Base t1 and Test Limit t3
    dli     t1, SIMPLE_TM_BASE
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
    dli     t9, 0x4000000

10:
    daddu   t9, t9, -0x1
    //set Test Base t1 and Test Limit t3
    dli     t1, SIMPLE_TM_BASE
    GET_TM_START_ADDR
    daddu   t1, t1, a1
    GET_TM_MSIZE
    daddu   t3, t1, a1

    dli     t8, TM_MAX_ERRORS
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

    bnez    t9, 10b
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

    move    ra, t6
    jr      ra
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
#ifdef  TM_DBG
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

RL_end:
