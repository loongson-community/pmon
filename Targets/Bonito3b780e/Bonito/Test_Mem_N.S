/********************
Author: Chen Xinke
Function: Test memory read and write errors
note :  Memory size >= 1G
Usage:  include this file in the start.S, after initialize cache and before copy Pmon Text to the memory;
        put the function at the end of this file--hexserial64_1-- in the code segment in start.S.
v0.8    The macro MEM_TEST_BASE and MT_PATTERN_BASE may need to be changed according to the address space.
v1.0    Add Address Range Test to Test Address map parameters configuration to
        avoid memory overlap problem which cann't be detected by other tests.
v1.2    Test address range is auto-configured by msize(use the default window)
        precondition:1. msize = full memory - 16M && full memory > 16M
                     2. default L2-Xbar window(fullmsize ~ fullmsize * 2) must be
                        configured.
v2.0    Support Multi-chip mode memory test and new msize map, Node ID is given by user input.
********************/
/***************
use register: s1, t0~t9, a0~a3, v0~v1
s1: store user input node ID.
!!!!!Pay attention: if the start.S use s1 for other purpose except for the ddr2_config code,
and we include this file, then pmon may fail.!!!!!!!!!!!!!!
t0: test pattern content
t1: current address
t6: test pattern base
t2: address interval
t3: max address
t4,t5: volatile
t7: by subroutine--hexserial64_1
t8: error counter---used to control the max detected error number
t9: loop control
**************/
//data patterns(8 bytes)
/**************
pattern: 
OneZero      0x0000000000000000 0xffffffffffffffff
JustZero     0x00000000
JustOne      0xffffffff
FiveA8       0x5aa5a55a5aa5a55a 0xa55a5aa5a55a5aa5
WalkOnes     0x01010101 0x02020202 0x04040404 0x08080808 0x10 0x20 0x40 0x80
WalkInvOnes  0x01010101 ~  0x02020202 ~ 0x04040404 ~ 0x08080808
WalkZeros    0xfefefefe 0xfdfdfdfd 0xfbfbfbfb 0xf7f7f7f7 0xef 0xdf 0xbf 0x7f
JustA        0xaaaaaaaa
JustFive     0x55555555
ZeroOne      0x00000000ffffffff
****************/
//address generator(address interval)
/********
* pattern 0: 8
* pattern 1: 8K(SDRAM page size 1KB)
* pattern 2: 16K(SDRAM page size 2KB)
*********/
#define PATTERN_1_0 0xb5b5b5b5b5b5b5b5
#define PATTERN_1_1 0x4a4a4a4a4a4a4a4a
#define PATTERN_2_0 0x5aa5a55a5aa5a55a
#define PATTERN_2_1 0xa55a5aa5a55a5aa5
#define PATTERN_3_0 0x0101010101010101
#define PATTERN_3_1 0x0202020202020202
#define PATTERN_3_2 0x0404040404040404
#define PATTERN_3_3 0x0808080808080808
#define PATTERN_3_4 0x1010101010101010
#define PATTERN_3_5 0x2020202020202020
#define PATTERN_3_6 0x4040404040404040
#define PATTERN_3_7 0x8080808080808080
#define PATTERN_4_0 0x0101010101010101
#define PATTERN_4_1 0xfefefefefefefefe
#define PATTERN_4_2 0x0202020202020202
#define PATTERN_4_3 0xfdfdfdfdfdfdfdfd
#define PATTERN_4_4 0x0404040404040404
#define PATTERN_4_5 0xfbfbfbfbfbfbfbfb
#define PATTERN_4_6 0x0808080808080808
#define PATTERN_4_7 0xf7f7f7f7f7f7f7f7
#define PATTERN_5_0 0xfefefefefefefefe
#define PATTERN_5_1 0xfdfdfdfdfdfdfdfd
#define PATTERN_5_2 0xfbfbfbfbfbfbfbfb
#define PATTERN_5_3 0xf7f7f7f7f7f7f7f7
#define PATTERN_5_4 0xefefefefefefefef
#define PATTERN_5_5 0xdfdfdfdfdfdfdfdf
#define PATTERN_5_6 0xbfbfbfbfbfbfbfbf
#define PATTERN_5_7 0x7f7f7f7f7f7f7f7f

#define PATTERN_JUST0   0x0000000000000000
#define PATTERN_JUST1   0xffffffffffffffff

#define PATTERN_JUSTA   0xaaaaaaaaaaaaaaaa
#define PATTERN_JUST5   0x5555555555555555
#define PATTERN_FiveA   0x55555555aaaaaaaa
#define PATTERN_ZEROONE 0x00000000ffffffff
#define PATTERN_L8b10b  0x1616161616161616
#define PATTERN_S8b10b  0xb5b5b5b5b5b5b5b5
#define PATTERN_Five7   0x5555555755575555
#define PATTERN_Zero2fd 0x00020002fffdfffd

#define MT_PATTERN_BASE   0x9800000000000000
#define MEM_TEST_BASE   0x9800000000100000
#define UNCACHED_MEM_TEST_BASE   0x9000000000100000
#define TM_MAX_ERRORS  0x10
#define REDUCED_MEM_TEST
//#define ONLY_UNCACHED_MT
#define GET_TM_NODE_ID_a1   dsrl a1, s1, 62;
#define GET_TM_NODE_MSIZE   dli  a1, 0xfffffffffff; and a1, s1, a1;
#define MANUAL_MSIZE

#ifdef  MANUAL_MSIZE  
    move    s7, msize
    dli	    msize, 0x0202
#endif

    PRINTSTR("\r\nmsize = ")
    move    a0, msize
    bal     hexserial
    nop
    PRINTSTR("\r\n")

TM_start:
#if 1
	PRINTSTR("\r\nPlease Select Test_Mem:\r\n[ 7: 4]: NODE ID\r\n[ 3: 0]: Do Test or Skip\r\n");
	li	    t6, 0x00
	bal	    inputaddress	#input value stored in v0
	nop
    dli     t6, 0xff
    and     v0, v0, t6
    beqz    v0, TM_end
    nop
#else
    //default input: Node 0, do Test
    dli     v0, 0x01
#endif
//-------------------------------
/* store the Node ID and decode node msize to s1 */
    //store NODE ID in s1[63:62]
    dli     t6, 0x30
    and     t6, t6, v0
    dsll    s1, t6, 58
    //store the selected NODE msize in s1[ 43: 0], if the Node msize=1(512M), skip the test,
    //because the window for 512M are 0~1000 0000 and 2000 0000 ~ 2fff ffff.
    GET_TM_NODE_ID_a1
    dsll    a1, a1, 3   //a1=a1 * 8
    dli     t6, 0xff
    dsll    t6, t6, a1
    and     t6, t6, msize
    dsrl    t6, t6, a1
    beqz    t6, TM_end1
    nop
    dli     a0, 1
    beq     t6, a0, TM_end1
    nop
    dsll    t6, t6, 29
    or      s1, s1, t6

    PRINTSTR("\r\ns1 = ")
    move    a0, s1
    bal     hexserial
    nop
    PRINTSTR("\r\n")

//---------------------------
	PRINTSTR("\r\nStart Testing Memory...\r\n")
    //-------------------
#if 1
    //Test whether the addr_pins,column_size,bank,rank param right.
    //This test must put before the Diff Burst Test.
	PRINTSTR("Address Range Testing...\r\n")
//write memory
    //use uncached address space here
    //set t1 to final Test Base
	dli 	t1, 0x9000000000000000
    GET_TM_NODE_ID_a1
    dsll    a1, a1, 44
    daddu   t1, t1, a1
    GET_TM_NODE_MSIZE
    daddu   t1, t1, a1
    sd      t1, 0x0(t1)
    dsrl    a1, a1, 2   //msize/4
    daddu   t1, t1, a1
    sd      t1, 0x0(t1)
    daddu   t1, t1, a1
    sd      t1, 0x0(t1)
    daddu   t1, t1, a1
    sd      t1, 0x0(t1)
	PRINTSTR("write done...\r\n")
//read memory and compare
    //set t1 to final Test Base
	dli 	t1, 0x9000000000000000
    GET_TM_NODE_ID_a1
    dsll    a1, a1, 44
    daddu   t1, t1, a1
    GET_TM_NODE_MSIZE
    daddu   t1, t1, a1
    ld	    t4, 0x0(t1)
    move    t0, t1
    beq     t4, t0, 2f
    nop
    //error detected!!! print address, expected data and read data
    bal     hexserial64_1
    nop
2:
    GET_TM_NODE_MSIZE
    dsrl    a1, a1, 2
    daddu   t1, t1, a1
    ld	    t4, 0x0(t1)
    move    t0, t1
    beq     t4, t0, 2f
    nop
    //error detected!!! print address, expected data and read data
    bal     hexserial64_1
    nop
2:
    GET_TM_NODE_MSIZE
    dsrl    a1, a1, 2
    daddu   t1, t1, a1
    ld	    t4, 0x0(t1)
    move    t0, t1
    beq     t4, t0, 2f
    nop
    //error detected!!! print address, expected data and read data
    bal     hexserial64_1
    nop
2:
    GET_TM_NODE_MSIZE
    dsrl    a1, a1, 2
    daddu   t1, t1, a1
    ld	    t4, 0x0(t1)
    move    t0, t1
    beq     t4, t0, 2f
    nop
    //error detected!!! print address, expected data and read data
    bal     hexserial64_1
    nop
2:
	PRINTSTR("Address Range Testing done!\r\n")
#endif
    //-------------------
#if 1
	PRINTSTR("Address Stuck Testing...\r\n")
//debug fatal errors
//write memory
    //set t1 to Test Base
    //use uncached address space here
	dli 	t1, UNCACHED_MEM_TEST_BASE
    GET_TM_NODE_ID_a1
    dsll    a1, a1, 44
    daddu   t1, t1, a1
    GET_TM_NODE_MSIZE
    daddu   t1, t1, a1
    dli     t8, 32  //max print errors 32 
    dli     t3, 128  //loop counter
1:	
    sd      t1, 0x0(t1)
    nop
    daddiu  t1, t1, 0x8
    daddiu  t3, t3, -1
    bnez 	t3, 1b
	nop
	PRINTSTR("write done...\r\n")
//read memory and compare
	dli 	t1, UNCACHED_MEM_TEST_BASE
    GET_TM_NODE_ID_a1
    dsll    a1, a1, 44
    daddu   t1, t1, a1
    GET_TM_NODE_MSIZE
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
    daddiu  t8, t8, -1
    bltz    t8, 20f  //detect enough errors, go to next test
    nop
2:
    daddiu  t1, t1, 0x8
    daddiu  t3, t3, -1
    bnez 	t3, 1b
	nop
	PRINTSTR("Stuck Testing done!\r\n")
#endif
//-------------------
20: //pattern WalkOnes
#ifdef  ONLY_UNCACHED_MT
    b       TM_end1
    nop
#endif
/*
 *Lock Scache 9800000000000000 ~ 9800000000001000(4K)
 */
    PRINTSTR("\r\nLock Scache--9800000000000000~4K...\r\n")
    GET_TM_NODE_ID_a1
    dsll    a1, a1, 44
    dli     t4, 0x900000003ff00200
    daddu   t4, t4, a1
    dli     t5, 0x8000000000000000
    sd      t5, 0x0(t4)
    dli     t5, 0x0000fffffffff000
    sd      t5, 0x40(t4)
    PRINTSTR("Lock Scache Done.\r\n")

#if 1
    PRINTSTR("\r\nPattern WalkOnes Test...\r\n")
    dli     t8, TM_MAX_ERRORS
    //address interval
    dli     t2, 0x40
    //set Test Base t1 and Test Limit t3
    dli     t1, MEM_TEST_BASE
    GET_TM_NODE_ID_a1
    dsll    a1, a1, 44
    daddu   t1, t1, a1
    GET_TM_NODE_MSIZE
    daddu   t1, t1, a1
    dsrl    a1, a1, 3
    daddu   t3, t1, a1  //t3 = t1 + fullmsize/8
    dli     a1, 0x100000    //t3-=1M  because Test Base start from Base Window + 1M
    dsubu   t3, t3, a1
    //debug--------
    PRINTSTR("Test address range: 0x")
    dsrl    a0, t1, 32
    bal     hexserial
    nop
    move    a0, t1
    bal     hexserial
    nop
    PRINTSTR("~0x")
    dsrl    a0, t3, 32
    bal     hexserial
    nop
    move    a0, t3
    bal     hexserial
    nop
    PRINTSTR("  @@  address interval: 0x")
    move    a0, t2
    bal     hexserial
    nop
    PRINTSTR("\r\n")
    //---------debug
//write memory
    //set Test Base t1
    dli     t1, MEM_TEST_BASE
    GET_TM_NODE_ID_a1
    dsll    a1, a1, 44
    daddu   t1, t1, a1
    GET_TM_NODE_MSIZE
    daddu   t1, t1, a1
1:
    dli     t4, PATTERN_3_0
    sd      t4, 0x0(t1)
    dli     t4, PATTERN_3_1
    sd      t4, 0x8(t1)
    dli     t4, PATTERN_3_2
    sd      t4, 0x10(t1)
    dli     t4, PATTERN_3_3
    sd      t4, 0x18(t1)
    dli     t4, PATTERN_3_4
    sd      t4, 0x20(t1)
    dli     t4, PATTERN_3_5
    sd      t4, 0x28(t1)
    dli     t4, PATTERN_3_6
    sd      t4, 0x30(t1)
    dli     t4, PATTERN_3_7
    sd      t4, 0x38(t1)

    daddu   t1, t1, t2
    bltu    t1, t3, 1b
    nop
	PRINTSTR("write done...\r\n")
//read memory and compare
    //set Test Base t1
    dli     t1, MEM_TEST_BASE
    GET_TM_NODE_ID_a1
    dsll    a1, a1, 44
    daddu   t1, t1, a1
    GET_TM_NODE_MSIZE
    daddu   t1, t1, a1
1:
    ld      t4, 0x0(t1)
    dli     t0, PATTERN_3_0
    beq     t4, t0, 2f
    nop
    //error detected!!! print address,expected data,read data and reread data
    bal     hexserial64_1
    nop
    daddiu  t8, t8, -1
    bltz    t8, 20f  //detect enough errors, go to next test
    nop
2:
    ld      t4, 0x8(t1)
    dli     t0, PATTERN_3_1
    beq     t4, t0, 2f
    nop
    //error detected!!! print address,expected data,read data and reread data
    daddiu  t1, t1, 0x8 //the hexserial64_1 will use t1 directly
    bal     hexserial64_1
    nop
    daddiu  t1, t1, -0x8
    daddiu  t8, t8, -1
2:
    ld      t4, 0x10(t1)
    dli     t0, PATTERN_3_2
    beq     t4, t0, 2f
    nop
    //error detected!!! print address,expected data,read data and reread data
    daddiu  t1, t1, 0x10
    bal     hexserial64_1
    nop
    daddiu  t1, t1, -0x10
    daddiu  t8, t8, -1
2:
    ld      t4, 0x18(t1)
    dli     t0, PATTERN_3_3
    beq     t4, t0, 2f
    nop
    //error detected!!! print address,expected data,read data and reread data
    daddiu  t1, t1, 0x18 //the hexserial64_1 will use t1 directly
    bal     hexserial64_1
    nop
    daddiu  t1, t1, -0x18
    daddiu  t8, t8, -1
2:
    ld      t4, 0x20(t1)
    dli     t0, PATTERN_3_4
    beq     t4, t0, 2f
    nop
    //error detected!!! print address,expected data,read data and reread data
    daddiu  t1, t1, 0x20
    bal     hexserial64_1
    nop
    daddiu  t1, t1, -0x20
    daddiu  t8, t8, -1
2:
    ld      t4, 0x28(t1)
    dli     t0, PATTERN_3_5
    beq     t4, t0, 2f
    nop
    //error detected!!! print address,expected data,read data and reread data
    daddiu  t1, t1, 0x28 //the hexserial64_1 will use t1 directly
    bal     hexserial64_1
    nop
    daddiu  t1, t1, -0x28
    daddiu  t8, t8, -1
2:
    ld      t4, 0x30(t1)
    dli     t0, PATTERN_3_6
    beq     t4, t0, 2f
    nop
    //error detected!!! print address,expected data,read data and reread data
    daddiu  t1, t1, 0x30
    bal     hexserial64_1
    nop
    daddiu  t1, t1, -0x30
    daddiu  t8, t8, -1
2:
    ld      t4, 0x38(t1)
    dli     t0, PATTERN_3_7
    beq     t4, t0, 2f
    nop
    //error detected!!! print address,expected data,read data and reread data
    daddiu  t1, t1, 0x38 //the hexserial64_1 will use t1 directly
    bal     hexserial64_1
    nop
    daddiu  t1, t1, -0x38
    daddiu  t8, t8, -1
2:
    bltz    t8, 20f  //detect enough errors, go to next test
    nop
    
    daddu   t1, t1, t2
    //check address range
    bltu    t1, t3, 1b
    nop

    PRINTSTR("Pattern Testing done.\r\n")
#endif
20: //pattern WalkInvOnes
#if 1
    PRINTSTR("\r\nPattern WalkInvOnes Test...\r\n")
    dli     t8, TM_MAX_ERRORS
    //address interval
    dli     t2, 0x80
    //set Test Base t1 and Test Limit t3
    dli     t1, MEM_TEST_BASE
    GET_TM_NODE_ID_a1
    dsll    a1, a1, 44
    daddu   t1, t1, a1
    GET_TM_NODE_MSIZE
    daddu   t1, t1, a1
    dsrl    a1, a1, 2
    daddu   t3, t1, a1  //t3 = t1 + fullmsize/8
    dli     a1, 0x100000    //t3-=1M  because Test Base start from Base Window + 1M
    dsubu   t3, t3, a1
    //debug--------
    PRINTSTR("Test address range: 0x")
    dsrl    a0, t1, 32
    bal     hexserial
    nop
    move    a0, t1
    bal     hexserial
    nop
    PRINTSTR("~0x")
    dsrl    a0, t3, 32
    bal     hexserial
    nop
    move    a0, t3
    bal     hexserial
    nop
    PRINTSTR("  @@  address interval: 0x")
    move    a0, t2
    bal     hexserial
    nop
    PRINTSTR("\r\n")
    //---------debug
//write memory
    //set Test Base t1
    dli     t1, MEM_TEST_BASE
    GET_TM_NODE_ID_a1
    dsll    a1, a1, 44
    daddu   t1, t1, a1
    GET_TM_NODE_MSIZE
    daddu   t1, t1, a1
1:
    dli     t4, PATTERN_4_0
    sd      t4, 0x0(t1)
    dli     t4, PATTERN_4_1
    sd      t4, 0x8(t1)
    dli     t4, PATTERN_4_2
    sd      t4, 0x10(t1)
    dli     t4, PATTERN_4_3
    sd      t4, 0x18(t1)
    dli     t4, PATTERN_4_4
    sd      t4, 0x20(t1)
    dli     t4, PATTERN_4_5
    sd      t4, 0x28(t1)
    dli     t4, PATTERN_4_6
    sd      t4, 0x30(t1)
    dli     t4, PATTERN_4_7
    sd      t4, 0x38(t1)

    daddu   t1, t1, t2
    bltu    t1, t3, 1b
    nop
	PRINTSTR("write done...\r\n")
//read memory and compare
    //set Test Base t1
    dli     t1, MEM_TEST_BASE
    GET_TM_NODE_ID_a1
    dsll    a1, a1, 44
    daddu   t1, t1, a1
    GET_TM_NODE_MSIZE
    daddu   t1, t1, a1
1:
    ld      t4, 0x0(t1)
    dli     t0, PATTERN_4_0
    beq     t4, t0, 2f
    nop
    //error detected!!! print address,expected data,read data and reread data
    bal     hexserial64_1
    nop
    daddiu  t8, t8, -1
    bltz    t8, 20f  //detect enough errors, go to next test
    nop
2:
    ld      t4, 0x8(t1)
    dli     t0, PATTERN_4_1
    beq     t4, t0, 2f
    nop
    //error detected!!! print address,expected data,read data and reread data
    daddiu  t1, t1, 0x8 //the hexserial64_1 will use t1 directly
    bal     hexserial64_1
    nop
    daddiu  t1, t1, -0x8
    daddiu  t8, t8, -1
2:
    ld      t4, 0x10(t1)
    dli     t0, PATTERN_4_2
    beq     t4, t0, 2f
    nop
    //error detected!!! print address,expected data,read data and reread data
    daddiu  t1, t1, 0x10
    bal     hexserial64_1
    nop
    daddiu  t1, t1, -0x10
    daddiu  t8, t8, -1
2:
    ld      t4, 0x18(t1)
    dli     t0, PATTERN_4_3
    beq     t4, t0, 2f
    nop
    //error detected!!! print address,expected data,read data and reread data
    daddiu  t1, t1, 0x18 //the hexserial64_1 will use t1 directly
    bal     hexserial64_1
    nop
    daddiu  t1, t1, -0x18
    daddiu  t8, t8, -1
2:
    ld      t4, 0x20(t1)
    dli     t0, PATTERN_4_4
    beq     t4, t0, 2f
    nop
    //error detected!!! print address,expected data,read data and reread data
    daddiu  t1, t1, 0x20
    bal     hexserial64_1
    nop
    daddiu  t1, t1, -0x20
    daddiu  t8, t8, -1
2:
    ld      t4, 0x28(t1)
    dli     t0, PATTERN_4_5
    beq     t4, t0, 2f
    nop
    //error detected!!! print address,expected data,read data and reread data
    daddiu  t1, t1, 0x28 //the hexserial64_1 will use t1 directly
    bal     hexserial64_1
    nop
    daddiu  t1, t1, -0x28
    daddiu  t8, t8, -1
2:
    ld      t4, 0x30(t1)
    dli     t0, PATTERN_4_6
    beq     t4, t0, 2f
    nop
    //error detected!!! print address,expected data,read data and reread data
    daddiu  t1, t1, 0x30
    bal     hexserial64_1
    nop
    daddiu  t1, t1, -0x30
    daddiu  t8, t8, -1
2:
    ld      t4, 0x38(t1)
    dli     t0, PATTERN_4_7
    beq     t4, t0, 2f
    nop
    //error detected!!! print address,expected data,read data and reread data
    daddiu  t1, t1, 0x38 //the hexserial64_1 will use t1 directly
    bal     hexserial64_1
    nop
    daddiu  t1, t1, -0x38
    daddiu  t8, t8, -1
2:
    bltz    t8, 20f  //detect enough errors, go to next test
    nop
    
    daddu   t1, t1, t2
    //check address range
    bltu    t1, t3, 1b
    nop

    PRINTSTR("Pattern Testing done.\r\n")
#endif
20: //pattern WalkZeros
#if 1
    PRINTSTR("\r\nPattern WalkZeros Test...\r\n")
    dli     t8, TM_MAX_ERRORS
    //address interval
    dli     t2, 0x100
    //set Test Base t1 and Test Limit t3
    dli     t1, MEM_TEST_BASE
    GET_TM_NODE_ID_a1
    dsll    a1, a1, 44
    daddu   t1, t1, a1
    GET_TM_NODE_MSIZE
    daddu   t1, t1, a1
    dsrl    a1, a1, 0
    daddu   t3, t1, a1  //t3 = t1 + fullmsize/x
    dli     a1, 0x100000    //t3-=1M  because Test Base start from Base Window + 1M
    dsubu   t3, t3, a1
    //debug--------
    PRINTSTR("Test address range: 0x")
    dsrl    a0, t1, 32
    bal     hexserial
    nop
    move    a0, t1
    bal     hexserial
    nop
    PRINTSTR("~0x")
    dsrl    a0, t3, 32
    bal     hexserial
    nop
    move    a0, t3
    bal     hexserial
    nop
    PRINTSTR("  @@  address interval: 0x")
    move    a0, t2
    bal     hexserial
    nop
    PRINTSTR("\r\n")
    //---------debug
//write memory
    //set Test Base t1
    dli     t1, MEM_TEST_BASE
    GET_TM_NODE_ID_a1
    dsll    a1, a1, 44
    daddu   t1, t1, a1
    GET_TM_NODE_MSIZE
    daddu   t1, t1, a1
1:
    dli     t4, PATTERN_5_0
    sd      t4, 0x0(t1)
    dli     t4, PATTERN_5_1
    sd      t4, 0x8(t1)
    dli     t4, PATTERN_5_2
    sd      t4, 0x10(t1)
    dli     t4, PATTERN_5_3
    sd      t4, 0x18(t1)
    dli     t4, PATTERN_5_4
    sd      t4, 0x20(t1)
    dli     t4, PATTERN_5_5
    sd      t4, 0x28(t1)
    dli     t4, PATTERN_5_6
    sd      t4, 0x30(t1)
    dli     t4, PATTERN_5_7
    sd      t4, 0x38(t1)

    daddu   t1, t1, t2
    bltu    t1, t3, 1b
    nop
	PRINTSTR("write done...\r\n")
//read memory and compare
    //set Test Base t1
    dli     t1, MEM_TEST_BASE
    GET_TM_NODE_ID_a1
    dsll    a1, a1, 44
    daddu   t1, t1, a1
    GET_TM_NODE_MSIZE
    daddu   t1, t1, a1
1:
    ld      t4, 0x0(t1)
    dli     t0, PATTERN_5_0
    beq     t4, t0, 2f
    nop
    //error detected!!! print address,expected data,read data and reread data
    bal     hexserial64_1
    nop
    daddiu  t8, t8, -1
    bltz    t8, 20f //detect enough errors, go to next test
    nop
2:
    ld      t4, 0x8(t1)
    dli     t0, PATTERN_5_1
    beq     t4, t0, 2f
    nop
    //error detected!!! print address,expected data,read data and reread data
    daddiu  t1, t1, 0x8 //the hexserial64_1 will use t1 directly
    bal     hexserial64_1
    nop
    daddiu  t1, t1, -0x8
    daddiu  t8, t8, -1
2:
    ld      t4, 0x10(t1)
    dli     t0, PATTERN_5_2
    beq     t4, t0, 2f
    nop
    //error detected!!! print address,expected data,read data and reread data
    daddiu  t1, t1, 0x10
    bal     hexserial64_1
    nop
    daddiu  t1, t1, -0x10
    daddiu  t8, t8, -1
2:
    ld      t4, 0x18(t1)
    dli     t0, PATTERN_5_3
    beq     t4, t0, 2f
    nop
    //error detected!!! print address,expected data,read data and reread data
    daddiu  t1, t1, 0x18 //the hexserial64_1 will use t1 directly
    bal     hexserial64_1
    nop
    daddiu  t1, t1, -0x18
    daddiu  t8, t8, -1
2:
    ld      t4, 0x20(t1)
    dli     t0, PATTERN_5_4
    beq     t4, t0, 2f
    nop
    //error detected!!! print address,expected data,read data and reread data
    daddiu  t1, t1, 0x20
    bal     hexserial64_1
    nop
    daddiu  t1, t1, -0x20
    daddiu  t8, t8, -1
2:
    ld      t4, 0x28(t1)
    dli     t0, PATTERN_5_5
    beq     t4, t0, 2f
    nop
    //error detected!!! print address,expected data,read data and reread data
    daddiu  t1, t1, 0x28 //the hexserial64_1 will use t1 directly
    bal     hexserial64_1
    nop
    daddiu  t1, t1, -0x28
    daddiu  t8, t8, -1
2:
    ld      t4, 0x30(t1)
    dli     t0, PATTERN_5_6
    beq     t4, t0, 2f
    nop
    //error detected!!! print address,expected data,read data and reread data
    daddiu  t1, t1, 0x30
    bal     hexserial64_1
    nop
    daddiu  t1, t1, -0x30
    daddiu  t8, t8, -1
2:
    ld      t4, 0x38(t1)
    dli     t0, PATTERN_5_7
    beq     t4, t0, 2f
    nop
    //error detected!!! print address,expected data,read data and reread data
    daddiu  t1, t1, 0x38 //the hexserial64_1 will use t1 directly
    bal     hexserial64_1
    nop
    daddiu  t1, t1, -0x38
    daddiu  t8, t8, -1
2:
    bltz    t8, 20f  //detect enough errors, go to next test
    nop
    
    daddu   t1, t1, t2
    //check address range
    bltu    t1, t3, 1b
    nop

    PRINTSTR("Pattern Testing done.\r\n")
#endif
20: //pattern Diff Burst Test
#if 1
    //initialization
    dli     t9, 0
    //set Test Pattern Base t6
    dli     t6, MT_PATTERN_BASE
    GET_TM_NODE_ID_a1
    dsll    a1, a1, 44
    daddu   t6, t6, a1
    GET_TM_NODE_MSIZE
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
    bgtu    t9, t4, 3f  //reduced test, only test 1~16
#endif
    nop
    PRINTSTR("\r\nPattern 0-f Test-----")
    //address interval
    dli     t2, 0x20
    //set Test Base t1 and Test Limit t3
    dli     t1, MEM_TEST_BASE
    GET_TM_NODE_ID_a1
    dsll    a1, a1, 44
    daddu   t1, t1, a1
    GET_TM_NODE_MSIZE
    daddu   t1, t1, a1
    dsrl    a1, a1, 2
    daddu   t3, t1, a1  //t3 = t1 + fullmsize/8
    dli     a1, 0x100000    //t3-=1M  because Test Base start from Base Window + 1M
    dsubu   t3, t3, a1
    dli     t4, 0
    dli     t0, PATTERN_JUST0
    dli     t5, PATTERN_JUST1
    b       2f
    nop
1:  
    dli     t4, 32
    bgtu    t9, t4, 1f
    nop
    PRINTSTR("\r\nPattern b5-4a Test-----")
    //address interval
    dli     t2, 0x20
    //set Test Base t1 and Test Limit t3
    dli     t1, MEM_TEST_BASE
    GET_TM_NODE_ID_a1
    dsll    a1, a1, 44
    daddu   t1, t1, a1
    GET_TM_NODE_MSIZE
    daddu   t1, t1, a1
    dsrl    a1, a1, 3
    daddu   t3, t1, a1  //t3 = t1 + fullmsize/8
    dli     a1, 0x100000    //t3-=1M  because Test Base start from Base Window + 1M
    dsubu   t3, t3, a1
    dli     t4, 16
    dli     t0, PATTERN_1_0
    dli     t5, PATTERN_1_1
    b       2f
    nop
1:  
    // t9 > 48 (all the burst inverse Pattern test done)
    dli     t4, 48
    bgtu    t9, t4, 3f
    nop
    PRINTSTR("\r\nPattern a-5 Test-----")
    //address interval
    dli     t2, 0x200
    //set Test Base t1 and Test Limit t3
    dli     t1, MEM_TEST_BASE
    GET_TM_NODE_ID_a1
    dsll    a1, a1, 44
    daddu   t1, t1, a1
    GET_TM_NODE_MSIZE
    daddu   t1, t1, a1
    dsrl    a1, a1, 3
    daddu   t3, t1, a1  //t3 = t1 + fullmsize/8
    dli     a1, 0x100000    //t3-=1M  because Test Base start from Base Window + 1M
    dsubu   t3, t3, a1
    dli     t4, 32
    dli     t0, PATTERN_JUSTA
    dli     t5, PATTERN_JUST5
    b       2f
    nop
2:
    daddiu  t4, 1
    bgtu    t9, t4, 1f
    nop
    //t9%16 == 1
    PRINTSTR("Pattern 0000 ....\r\n")
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
    PRINTSTR("Pattern 0001 ....\r\n")
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
    PRINTSTR("Pattern 0010 ....\r\n")
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
    PRINTSTR("Pattern 0011 ....\r\n")
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
    PRINTSTR("Pattern 0100 ....\r\n")
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
    PRINTSTR("Pattern 0101 ....\r\n")
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
    PRINTSTR("Pattern 0110 ....\r\n")
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
    PRINTSTR("Pattern 0111 ....\r\n")
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
    PRINTSTR("Pattern 1000 ....\r\n")
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
    PRINTSTR("Pattern 1001 ....\r\n")
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
    PRINTSTR("Pattern 1010 ....\r\n")
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
    PRINTSTR("Pattern 1011 ....\r\n")
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
    PRINTSTR("Pattern 1100 ....\r\n")
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
    PRINTSTR("Pattern 1101 ....\r\n")
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
    PRINTSTR("Pattern 1110 ....\r\n")
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
    PRINTSTR("Pattern 1111 ....\r\n")
    sd      t5, 0x0(t6)
    sd      t5, 0x8(t6)
    sd      t5, 0x10(t6)
    sd      t5, 0x18(t6)
    b       2f
    nop
1:
    // t9 > 48 (all the burst inverse Pattern test done)
    b       3f  //go to the end of diff burst test
    nop
2:
    dli     t8, TM_MAX_ERRORS
    //set Test Base t1
    dli     t1, MEM_TEST_BASE
    GET_TM_NODE_ID_a1
    dsll    a1, a1, 44
    daddu   t1, t1, a1
    GET_TM_NODE_MSIZE
    daddu   t1, t1, a1
    //debug--------
    PRINTSTR("Test address range: 0x")
    dsrl    a0, t1, 32
    bal     hexserial
    nop
    move    a0, t1
    bal     hexserial
    nop
    PRINTSTR("~0x")
    dsrl    a0, t3, 32
    bal     hexserial
    nop
    move    a0, t3
    bal     hexserial
    nop
    PRINTSTR("  @@  address interval: 0x")
    move    a0, t2
    bal     hexserial
    nop
    PRINTSTR("\r\n")
    //---------debug
//write memory
    //set Test Base t1
    dli     t1, MEM_TEST_BASE
    GET_TM_NODE_ID_a1
    dsll    a1, a1, 44
    daddu   t1, t1, a1
    GET_TM_NODE_MSIZE
    daddu   t1, t1, a1
    ld      t0, 0x0(t6)
    ld      t4, 0x8(t6)
    ld      t5, 0x10(t6)
    ld      a1, 0x18(t6)
1:
    sd      t0, 0x0(t1)
    sd      t4, 0x8(t1)
    sd      t5, 0x10(t1)
    sd      a1, 0x18(t1)
    daddu   t1, t1, t2
    bltu    t1, t3, 1b
    nop
	PRINTSTR("write done. begin to read and compare...\r\n")
//read memory and compare
    //set Test Base t1
    dli     t1, MEM_TEST_BASE
    GET_TM_NODE_ID_a1
    dsll    a1, a1, 44
    daddu   t1, t1, a1
    GET_TM_NODE_MSIZE
    daddu   t1, t1, a1
1:
    ld      t0, 0x0(t6)
    ld      t4, 0x0(t1)
    beq     t4, t0, 2f
    nop
    //error detected!!! print address,expected data,read data and reread data
    bal     hexserial64_1
    nop
    daddiu  t8, t8, -0x1
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
    daddiu  t8, t8, -0x1
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
    daddiu  t8, t8, -0x1
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
    daddiu  t8, t8, -0x1
    bltz    t8, 10b  //detect enough errors, go to next test
    nop
2:
    daddu   t1, t1, t2
    //check address range
    bltu    t1, t3, 1b
    nop
    PRINTSTR("Pattern Testing done.\r\n")
    b       10b
    nop
3:
#endif
//pattern FiveA8
#ifndef REDUCED_MEM_TEST
    PRINTSTR("\r\nPattern FiveA8 Test...\r\n")
    dli     t8, TM_MAX_ERRORS
    //address interval
    dli     t2, 0x200
    //set Test Base t1 and Test Limit t3
    dli     t1, MEM_TEST_BASE
    GET_TM_NODE_ID_a1
    dsll    a1, a1, 44
    daddu   t1, t1, a1
    GET_TM_NODE_MSIZE
    daddu   t1, t1, a1
    dsrl    a1, a1, 2
    daddu   t3, t1, a1  //t3 = t1 + fullmsize/x
    dli     a1, 0x100000    //t3-=1M  because Test Base start from Base Window + 1M
    dsubu   t3, t3, a1
    //debug--------
    PRINTSTR("Test address range: 0x")
    dsrl    a0, t1, 32
    bal     hexserial
    nop
    move    a0, t1
    bal     hexserial
    nop
    PRINTSTR("~0x")
    dsrl    a0, t3, 32
    bal     hexserial
    nop
    move    a0, t3
    bal     hexserial
    nop
    PRINTSTR("  @@  address interval: 0x")
    move    a0, t2
    bal     hexserial
    nop
    PRINTSTR("\r\n")
    //---------debug
//write memory
    //set Test Base t1
    dli     t1, MEM_TEST_BASE
    GET_TM_NODE_ID_a1
    dsll    a1, a1, 44
    daddu   t1, t1, a1
    GET_TM_NODE_MSIZE
    daddu   t1, t1, a1
1:
    dli     t4, PATTERN_2_0
    sd      t4, 0x0(t1)
    dli     t4, PATTERN_2_1
    sd      t4, 0x8(t1)
    daddu   t1, t1, t2
    bltu    t1, t3, 1b
    nop
	PRINTSTR("write done...\r\n")
//read memory and compare
    //set Test Base t1
    dli     t1, MEM_TEST_BASE
    GET_TM_NODE_ID_a1
    dsll    a1, a1, 44
    daddu   t1, t1, a1
    GET_TM_NODE_MSIZE
    daddu   t1, t1, a1
1:
    ld      t4, 0x0(t1)
    dli     t0, PATTERN_2_0
    beq     t4, t0, 2f
    nop
    //error detected!!! print address,expected data,read data and reread data
    bal     hexserial64_1
    nop
    daddiu  t8, t8, -1
    bltz    t8, 20f  //detect enough errors, go to next test
    nop
2:
    ld      t4, 0x8(t1)
    dli     t0, PATTERN_2_1
    beq     t4, t0, 2f
    nop
    //error detected!!! print address,expected data,read data and reread data
    daddiu  t1, t1, 0x8 //the hexserial64_1 will use t1 directly
    bal     hexserial64_1
    nop
    daddiu  t1, t1, -8
    daddiu  t8, t8, -1
2:
    daddu   t1, t1, t2
    //check address range
    bltu    t1, t3, 1b
    nop
    PRINTSTR("Pattern Testing done.\r\n")
#endif
20: //pattern JustX Test
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
    GET_TM_NODE_MSIZE
    daddu   t1, t1, a1
    dsrl    a1, a1, 3
    daddu   t3, t1, a1  //t3 = t1 + fullmsize/8
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
    PRINTSTR("\r\nPattern JustA Test...\r\n")
    dli     t0, PATTERN_JUSTA
    b       2f
    nop
1:
    dli     t4, 2
    bgtu    t9, t4, 1f
    nop
    //t9 == 2
    PRINTSTR("Pattern Just5 Test...\r\n")
    dli     t0, PATTERN_JUST5
    b       2f
    nop
1:
    dli     t4, 3
    bgtu    t9, t4, 1f
    nop
    //t9 == 3
    PRINTSTR("Pattern JustFiveA Test...\r\n")
    dli     t0, PATTERN_FiveA
    b       2f
    nop
1:
    dli     t4, 4
    bgtu    t9, t4, 1f
    nop
    //t9 == 4
    PRINTSTR("Pattern JustZeroOne Test...\r\n")
    dli     t0, PATTERN_ZEROONE
    b       2f
    nop
1:
    dli     t4, 5
    bgtu    t9, t4, 1f
    nop
    //t9 == 5
    PRINTSTR("Pattern JustL8b10b-16 Test...\r\n")
    dli     t0, PATTERN_L8b10b
    b       2f
    nop
1:
    dli     t4, 6
    bgtu    t9, t4, 1f
    nop
    //t9 == 6
    PRINTSTR("Pattern JustS8b10b-b5 Test...\r\n")
    dli     t0, PATTERN_S8b10b
    b       2f
    nop
1:
    dli     t4, 7
    bgtu    t9, t4, 1f
    nop
    //t9 == 7
    PRINTSTR("Pattern JustFive7 Test...\r\n")
    dli     t0, PATTERN_Five7
    b       2f
    nop
1:
    dli     t4, 8
    bgtu    t9, t4, 1f
    nop
    //t9 == 8
    PRINTSTR("Pattern JustZero2fd Test...\r\n")
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
    GET_TM_NODE_ID_a1
    dsll    a1, a1, 44
    daddu   t1, t1, a1
    GET_TM_NODE_MSIZE
    daddu   t1, t1, a1
    //debug--------
    PRINTSTR("Test address range: 0x")
    dsrl    a0, t1, 32
    bal     hexserial
    nop
    move    a0, t1
    bal     hexserial
    nop
    PRINTSTR("~0x")
    dsrl    a0, t3, 32
    bal     hexserial
    nop
    move    a0, t3
    bal     hexserial
    nop
    PRINTSTR("  @@  address interval: 0x")
    move    a0, t2
    bal     hexserial
    nop
    PRINTSTR("\r\n")
    //---------debug
//write memory
    //set Test Base t1
    dli     t1, MEM_TEST_BASE
    GET_TM_NODE_ID_a1
    dsll    a1, a1, 44
    daddu   t1, t1, a1
    GET_TM_NODE_MSIZE
    daddu   t1, t1, a1
1:
    sd      t0, 0x0(t1)
    sd      t0, 0x8(t1)
    daddu   t1, t1, t2
    bltu    t1, t3, 1b
    nop
	PRINTSTR("write done. begin to read and compare...\r\n")
//read memory and compare
    //set Test Base t1
    dli     t1, MEM_TEST_BASE
    GET_TM_NODE_ID_a1
    dsll    a1, a1, 44
    daddu   t1, t1, a1
    GET_TM_NODE_MSIZE
    daddu   t1, t1, a1
1:
    ld      t4, 0x0(t1)
    beq     t4, t0, 2f
    nop
    //error detected!!! print address,expected data,read data and reread data
    bal     hexserial64_1
    nop
    daddiu  t8, t8, -0x1
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
    daddiu  t8, t8, -0x1
    bltz    t8, 10b  //detect enough errors, go to next test
    nop
2:
    daddu   t1, t1, t2
    //check address range
    bltu    t1, t3, 1b
    nop
    PRINTSTR("Pattern Testing done.\r\n\r\n")
    b       10b
    nop
3:
#endif
20: //pattern 11

/*
 *Unlock Scache 9800000000000000 ~ 9800000000001000(4K)
 */
    PRINTSTR("\r\nUnlock Scache--9800000000000000~4K...\r\n")
    GET_TM_NODE_ID_a1
    dsll    a1, a1, 44
    dli     t4, 0x900000003ff00200
    daddu   t4, t4, a1
    dli     t5, 0x0000000000000000
    sd      t5, 0x0(t4)
    PRINTSTR("Unlock Scache Done.\r\n")
TM_end1:
    b       TM_start
    nop

hexserial64_1:  //pseudo subroutine
/**********************
input:  t1: read address(read only)
        t0: expected data(read only)
        t4: read data
use reg:t5, t7
***********************/
    move    t7,ra
	PRINTSTR("\r\naddr 0x")
	dsrl	a0, t1, 32
	bal	    hexserial
	nop
	move	a0, t1
	bal	    hexserial
	nop
    PRINTSTR(" expected: ")
	dsrl	a0, t0, 32
	bal	    hexserial
	nop
	move	a0, t0
	bal	    hexserial
	nop
    PRINTSTR(" read: ")
	dsrl	a0, t4, 32
	bal	    hexserial
	nop
	move	a0, t4
	bal	    hexserial
	nop
    /* reread the wrong byte, use uncached address*/
    PRINTSTR(" reread: ")
    dli     t5, 0xf7ffffffffffffff
    and     t5, t1, t5
    ld      t5, 0(t5)
    nop
	dsrl	a0, t5, 32
	bal	    hexserial
	nop
	move	a0, t5
	bal	    hexserial
	nop
    /* if the reread value differs the first read, print mark */
    beq     t4, t5, 2f
    nop
    PRINTSTR("  DDD")
2:
    PRINTSTR("\r\n")
    jr    t7    
    nop
TM_end:
#ifdef  MANUAL_MSIZE
    move    msize, s7
#endif
