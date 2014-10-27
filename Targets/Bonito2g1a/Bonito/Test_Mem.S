/********************
Author: Chen Xinke
Function: Test memory read and write errors
note :  Memory size >= 1G
Usage:  include this file in the start.S, after initialize cache and before copy Pmon Text to the memory;
v1.2    Test address range is auto-configured by msize(use the default window)
        precondition:
        1. default L2-Xbar window(fullmsize ~ fullmsize * 2) must be configured.
v2.0    Support Multi-chip mode memory test and new msize map, Node ID is given by user input.
v2.2    Modify the structure of the program to reduce its size and make it more scaleable.
********************/
/***************
use register: s1, t0~t9, a0~a3, v0~v1
note: don't change s0, s2(msize)
s1: store user input node ID.
!!!!!Pay attention: this code will damage s1, if the start.S use s1
for other purpose except for the ddr2_config code,and we include this file,
then pmon may fail.!!!!!!!!!!!!!!
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
s6: error sum
v0: output result
**************/
//data patterns(8 bytes)
/**************
pattern: 
WalkOnes     0x01010101 0x02020202 0x04040404 0x08080808 0x10 0x20 0x40 0x80
WalkInvOnes  0x01010101 ~  0x02020202 ~ 0x04040404 ~ 0x08080808
WalkZeros    0xfefefefe 0xfdfdfdfd 0xfbfbfbfb 0xf7f7f7f7 0xef 0xdf 0xbf 0x7f
OneZero      0x0000000000000000 0xffffffffffffffff
JustZero     0x00000000
JustOne      0xffffffff
FiveA8       0x5aa5a55a5aa5a55a 0xa55a5aa5a55a5aa5
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
#define PATTERN_D8_0_0  0x0101010101010101
#define PATTERN_D8_0_1  0x0202020202020202
#define PATTERN_D8_0_2  0x0404040404040404
#define PATTERN_D8_0_3  0x0808080808080808
#define PATTERN_D8_0_4  0x1010101010101010
#define PATTERN_D8_0_5  0x2020202020202020
#define PATTERN_D8_0_6  0x4040404040404040
#define PATTERN_D8_0_7  0x8080808080808080
#define PATTERN_D8_1_0  0x0101010101010101
#define PATTERN_D8_1_1  0xfefefefefefefefe
#define PATTERN_D8_1_2  0x0202020202020202
#define PATTERN_D8_1_3  0xfdfdfdfdfdfdfdfd
#define PATTERN_D8_1_4  0x0404040404040404
#define PATTERN_D8_1_5  0xfbfbfbfbfbfbfbfb
#define PATTERN_D8_1_6  0x0808080808080808
#define PATTERN_D8_1_7  0xf7f7f7f7f7f7f7f7
#define PATTERN_D8_2_0  0xfefefefefefefefe
#define PATTERN_D8_2_1  0xfdfdfdfdfdfdfdfd
#define PATTERN_D8_2_2  0xfbfbfbfbfbfbfbfb
#define PATTERN_D8_2_3  0xf7f7f7f7f7f7f7f7
#define PATTERN_D8_2_4  0xefefefefefefefef
#define PATTERN_D8_2_5  0xdfdfdfdfdfdfdfdf
#define PATTERN_D8_2_6  0xbfbfbfbfbfbfbfbf
#define PATTERN_D8_2_7  0x7f7f7f7f7f7f7f7f

#define PATTERN_DB_0_0  0x0000000000000000
#define PATTERN_DB_0_1  0xffffffffffffffff
#define PATTERN_DB_1_0  0x0000000800000008
#define PATTERN_DB_1_1  0xfffffff7fffffff7
#define PATTERN_DB_2_0  0x5aa5a55a5aa5a55a
#define PATTERN_DB_2_1  0xa55a5aa5a55a5aa5
#define PATTERN_DB_3_0  0xb5b5b5b5b5b5b5b5
#define PATTERN_DB_3_1  0x4a4a4a4a4a4a4a4a

#define PATTERN_JUSTA   0xaaaaaaaaaaaaaaaa
#define PATTERN_JUST5   0x5555555555555555
#define PATTERN_FiveA   0x55555555aaaaaaaa
#define PATTERN_ZEROONE 0x00000000ffffffff
#define PATTERN_L8b10b  0x1616161616161616
#define PATTERN_S8b10b  0xb5b5b5b5b5b5b5b5
#define PATTERN_Five7   0x5555555755575555
#define PATTERN_Zero2fd 0x00020002fffdfffd

#define MEM_TEST_BASE   0x9800000000100000
#define UNCACHED_MEM_TEST_BASE   0x9000000000100000
#define MT_PATTERN_BASE 0x9800000000000000  //(0 ~ 400 -- 0 ~ 1K)
#define MT_STACK_BASE   0x9800000000000400  //(400 ~ 600 -- 512Byte max, 64 registers)
#define MT_CODE_BASE    0x9800000000000600  //(600 ~ 4000 -- 1.5K ~ 16K, 14.5K max)
#define MT_MSG_BASE     0x9800000000004000  //(4000 ~ 10000 -- 16K ~ 64K, 48K max)

#define TM_MAX_ERRORS  0x10
#define REDUCED_MEM_TEST
#define GET_TM_NODE_ID_a1   dsrl a1, s1, 62;
#define GET_TM_NODE_MSIZE   dli  a1, 0xfffffffffff; and a1, s1, a1;

#define MANUAL_MSIZE
#ifdef  MANUAL_MSIZE
#define MANUAL_NODE_MSIZE   0x80000000  //used for set start address
#define GET_MANUAL_TM_MSIZE    dli  a1, 0x40000000; //Memory size to be tested
#endif

#define GET_RD_LEVEL    dli a1, 0x1; and a1, s4, a1;
#define GET_MICRO_TUNE  dli a2, 0x100; and a2, s4, a2; dsrl a2, a2, 8;

    PRINTSTR("Please input TM param: ")
    bal     inputaddress
    nop
    dli     a1, 0x20
    bgt     v0, a1, RL_end
    nop
    move    t1, v0
    bal     test_mem
    nop
    beqz    v0, 1f
    nop
    PRINTSTR("\r\n Error found!! \r\n")
2:
    b       2b
    nop
1:
    b       RL_end
    nop

/********************************
 * test_mem
 * input:   t1
   bit[ 0]: 1: read level; 0: write level;
   bit[ 8]: 0: macro tune; 1: micro tune;
 * output:  v0
    0: no error
    1: error
********************************/
test_mem:
    move    t8, ra

TM_start:
#if 0
	PRINTSTR("\r\nPlease Select Test_Mem:\r\n[ 7: 4]: NODE ID\r\n[ 3: 0]: Do Test or Skip\r\n");
	bal	    inputaddress	#input value stored in v0
	nop
    dli     a2, 0xff
    and     v0, v0, a2
    beqz    v0, TM_end
    nop
#else
    //default input: Node 0, do Test
	PRINTSTR("\r\nDefault Test Node 0\r\n");
    dli     v0, 0x01
#endif

#ifndef MANUAL_MSIZE
/* store the Node ID and decode node msize to s1 */
    //store NODE ID in s1[63:62]
    dli     a2, 0x30
    and     a2, a2, v0
    dsll    s1, a2, 58
    //store the selected NODE msize in s1[ 43: 0], if the Node msize=1(512M), skip the test,
    //because the window for 512M are 0~1000 0000 and 2000 0000 ~ 2fff ffff.
    GET_TM_NODE_ID_a1
    dsll    a1, a1, 3   //a1=a1 * 8
    dli     a2, 0xff
    dsll    a2, a2, a1
    and     a2, a2, msize
    dsrl    a2, a2, a1
    beqz    a2, TM_end1
    nop
    dli     a0, 1
    beq     a2, a0, TM_end1
    nop
    dsll    a2, a2, 29
    or      s1, s1, a2

    PRINTSTR("\r\nmsize = 0x")
    move    a0, msize
    bal     hexserial
    nop
    PRINTSTR("\r\n")
#else
/* store the Node ID and set node msize to s1 */
    //store NODE ID in s1[63:62]
    dli     a2, 0x30
    and     a2, a2, v0
    dsll    s1, a2, 58

    dli     a2, MANUAL_NODE_MSIZE
    or      s1, s1, a2
    PRINTSTR("\r\nMANUAL_NODE_MSIZE = 0x")
    dli     a0, MANUAL_NODE_MSIZE
    bal     hexserial
    nop
    PRINTSTR("\r\n")
    PRINTSTR("\r\nTM_MSIZE = 0x")
    GET_MANUAL_TM_MSIZE
    move    a0, a1
    bal     hexserial
    nop
    PRINTSTR("\r\n")
#endif

    PRINTSTR("\r\ns1 = 0x")
    dsrl    a0, s1, 32
    bal     hexserial
    nop
    move    a0, s1
    bal     hexserial
    nop
    PRINTSTR("\r\n")
//-------------------------------

/*
 *Lock Scache 9800?00000000000 ~ 9800?00000001000(4K)
 */
    PRINTSTR("\r\nLock Scache Node x--9800?00000000000~4K...\r\n")
    GET_TM_NODE_ID_a1
    dsll    a1, a1, 44
    dli     a2, 0x900000003ff00200
    daddu   a2, a2, a1
    dli     a3, 0x8000000000000000
    daddu   a3, a3, a1
    sd      a3, 0x0(a2)
    dli     a3, 0x0000fffffffff000
    sd      a3, 0x40(a2)
    PRINTSTR("Lock Scache Done.\r\n")
//save t0~t9,s1~s7
    GET_TM_NODE_ID_a1
    dsll    a1, a1, 44
    dli     a2, MT_STACK_BASE
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
//if we run this code, the cpu will stall at the recover process. why?
//because Hit Invalidate doesn't write the D-cache back to S-cache, so the data
//will be lost, and PRINTSTR subroutine will use s0 to relocate data text address,
//so there may be an illegal ld address
    //Hit Invalidate the Primary D-cache.
    TTYDBG("\r\nFlush Primary D-cache...\r\n")
    GET_TM_NODE_ID_a1
    dsll    a1, a1, 44
    dli     a0, MT_STACK_BASE
    daddu   a0, a0, a1
    daddiu  a1, a0, 0x100
1:
    cache   0x11, 0x0(a0)
    daddiu  a0, a0, 0x20
    blt     a0, a1, 1b
    nop
    TTYDBG("\r\nFlush Primary D-cache done.\r\n")
#endif
     
    move    s4, t1
    dli     s6, 0x0

//---------------------------
	PRINTSTR("\r\nStart Testing Memory...\r\n")
#if 1
    //initialization
    dli     t9, 0
    //set Test Pattern Base t6
    dli     t6, MT_PATTERN_BASE
    GET_TM_NODE_ID_a1
    dsll    a1, a1, 44
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
    PRINTSTR("\r\nPattern WalkOnes Test...\r\n")
    //address interval
    dli     t2, 0x100
    //set Test Base t1 and Test Limit t3
    dli     t1, MEM_TEST_BASE
    GET_TM_NODE_ID_a1
    dsll    a1, a1, 44
    daddu   t1, t1, a1
    GET_TM_NODE_MSIZE
    daddu   t1, t1, a1
#ifdef  MANUAL_MSIZE
    GET_MANUAL_TM_MSIZE
#endif
    dsrl    a1, a1, 0
    GET_MICRO_TUNE
    bnez    a2, 21f
    nop
    dsrl    a1, a1, 1
21:
    daddu   t3, t1, a1  //t3 = t1 + fullmsize/
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
    PRINTSTR("\r\nPattern WalkInvOnes Test...\r\n")
    //address interval
    dli     t2, 0x80
    //set Test Base t1 and Test Limit t3
    dli     t1, MEM_TEST_BASE
    GET_TM_NODE_ID_a1
    dsll    a1, a1, 44
    daddu   t1, t1, a1
    GET_TM_NODE_MSIZE
    daddu   t1, t1, a1
#ifdef  MANUAL_MSIZE
    GET_MANUAL_TM_MSIZE
#endif
    dsrl    a1, a1, 1
    GET_MICRO_TUNE
    bnez    a2, 21f
    nop
    dsrl    a1, a1, 1
21:
    daddu   t3, t1, a1  //t3 = t1 + fullmsize/8
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
    PRINTSTR("\r\nPattern WalkZeros Test...\r\n")
    //address interval
    dli     t2, 0x40
    //set Test Base t1 and Test Limit t3
    dli     t1, MEM_TEST_BASE
    GET_TM_NODE_ID_a1
    dsll    a1, a1, 44
    daddu   t1, t1, a1
    GET_TM_NODE_MSIZE
    daddu   t1, t1, a1
#ifdef  MANUAL_MSIZE
    GET_MANUAL_TM_MSIZE
#endif
    dsrl    a1, a1, 2
    GET_MICRO_TUNE
    bnez    a2, 21f
    nop
    dsrl    a1, a1, 1
21:
    daddu   t3, t1, a1  //t3 = t1 + fullmsize/
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
    // t9 > 3 (all the burst inverse Pattern test done)
    b       3f  //go to the end of diff burst test
    nop
2:
    dli     t8, TM_MAX_ERRORS
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
    bgtz    s6, TM_end
    nop
    daddu   t1, t1, t2
    //check address range
    bltu    t1, t3, 1b
    nop
    PRINTSTR("Pattern Testing done.\r\n")
    b       10b
    nop
3:
#endif
//pattern Diff Burst Test
#if 1
    //initialization
    dli     t9, 0
    //set Test Pattern Base t6
    dli     t6, MT_PATTERN_BASE
    GET_TM_NODE_ID_a1
    dsll    a1, a1, 44
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
    PRINTSTR("\r\nPattern DB_0 Test-----\r\n")
    //address interval
    dli     t2, 0x20
    //set Test Base t1 and Test Limit t3
    dli     t1, MEM_TEST_BASE
    GET_TM_NODE_ID_a1
    dsll    a1, a1, 44
    daddu   t1, t1, a1
    GET_TM_NODE_MSIZE
    daddu   t1, t1, a1
#ifdef  MANUAL_MSIZE
    GET_MANUAL_TM_MSIZE
#endif
    dsrl    a1, a1, 3
    GET_MICRO_TUNE
    bnez    a2, 21f
    nop
    dsrl    a1, a1, 1
21:
    daddu   t3, t1, a1  //t3 = t1 + fullmsize/8
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
    PRINTSTR("\r\nPattern DB_1 Test-----\r\n")
    //address interval
    dli     t2, 0x20
    //set Test Base t1 and Test Limit t3
    dli     t1, MEM_TEST_BASE
    GET_TM_NODE_ID_a1
    dsll    a1, a1, 44
    daddu   t1, t1, a1
    GET_TM_NODE_MSIZE
    daddu   t1, t1, a1
#ifdef  MANUAL_MSIZE
    GET_MANUAL_TM_MSIZE
#endif
    dsrl    a1, a1, 3
    GET_MICRO_TUNE
    bnez    a2, 21f
    nop
    dsrl    a1, a1, 1
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
    PRINTSTR("\r\nPattern DB_2 Test-----\r\n")
    //address interval
    dli     t2, 0x20
    //set Test Base t1 and Test Limit t3
    dli     t1, MEM_TEST_BASE
    GET_TM_NODE_ID_a1
    dsll    a1, a1, 44
    daddu   t1, t1, a1
    GET_TM_NODE_MSIZE
    daddu   t1, t1, a1
#ifdef  MANUAL_MSIZE
    GET_MANUAL_TM_MSIZE
#endif
    dsrl    a1, a1, 3
    GET_MICRO_TUNE
    bnez    a2, 21f
    nop
    dsrl    a1, a1, 1
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
    PRINTSTR("\r\nPattern DB_3 Test-----\r\n")
    //address interval
    dli     t2, 0x200
    //set Test Base t1 and Test Limit t3
    dli     t1, MEM_TEST_BASE
    GET_TM_NODE_ID_a1
    dsll    a1, a1, 44
    daddu   t1, t1, a1
    GET_TM_NODE_MSIZE
    daddu   t1, t1, a1
#ifdef  MANUAL_MSIZE
    GET_MANUAL_TM_MSIZE
#endif
    dsrl    a1, a1, 3
    GET_MICRO_TUNE
    bnez    a2, 21f
    nop
    dsrl    a1, a1, 1
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
2:
    dli     t8, TM_MAX_ERRORS
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
    bgtz    s6, TM_end
    nop
    daddu   t1, t1, t2
    //check address range
    bltu    t1, t3, 1b
    nop
    PRINTSTR("Pattern Testing done.\r\n")
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
    PRINTSTR("\r\nPattern Just5 Test...\r\n")
    dli     t0, PATTERN_JUST5
    b       2f
    nop
1:
    dli     t4, 3
    bgtu    t9, t4, 1f
    nop
    //t9 == 3
    PRINTSTR("\r\nPattern JustFiveA Test...\r\n")
    dli     t0, PATTERN_FiveA
    b       2f
    nop
1:
    dli     t4, 4
    bgtu    t9, t4, 1f
    nop
    //t9 == 4
    PRINTSTR("\r\nPattern JustZeroOne Test...\r\n")
    dli     t0, PATTERN_ZEROONE
    b       2f
    nop
1:
    dli     t4, 5
    bgtu    t9, t4, 1f
    nop
    //t9 == 5
    PRINTSTR("\r\nPattern JustL8b10b-16 Test...\r\n")
    dli     t0, PATTERN_L8b10b
    b       2f
    nop
1:
    dli     t4, 6
    bgtu    t9, t4, 1f
    nop
    //t9 == 6
    PRINTSTR("\r\nPattern JustS8b10b-b5 Test...\r\n")
    dli     t0, PATTERN_S8b10b
    b       2f
    nop
1:
    dli     t4, 7
    bgtu    t9, t4, 1f
    nop
    //t9 == 7
    PRINTSTR("\r\nPattern JustFive7 Test...\r\n")
    dli     t0, PATTERN_Five7
    b       2f
    nop
1:
    dli     t4, 8
    bgtu    t9, t4, 1f
    nop
    //t9 == 8
    PRINTSTR("\r\nPattern JustZero2fd Test...\r\n")
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
    PRINTSTR("Pattern Testing done.\r\n")
    b       10b
    nop
3:
#endif

TM_end1:
    b       TM_end
    nop

hexserial64_1:  //pseudo subroutine
/**********************
input:  t1: read address(read only)
        t0: expected data(read only)
        t4: read data
use reg:t5, t7
***********************/
    move    t7, ra
    daddiu  t8, t8, -0x1
    /* reread the wrong bytes */
#if 1
    //Hit Invalidate the Primary D-cache and Second cache.
    //TTYDBG("\r\nInvalidate Primary D-cache and S-cache.\r\n")
    cache   0x11, 0x0(t1)
    cache   0x13, 0x0(t1)
    move    t5, t1
#else
    dli     t5, 0xf7ffffffffffffff
    and     t5, t1, t5
#endif
    ld      t5, 0(t5)
    nop

	PRINTSTR("addr 0x")
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
    PRINTSTR(" reread: ")
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
    //---------------------
    daddiu  s6, s6, 0x1
    b       3f
    nop
    //---------------------
2:
    //---------------------
    GET_RD_LEVEL
    bnez    a1, 3f
    nop
    daddiu  s6, s6, 0x1
    //---------------------
3:
    PRINTSTR("\r\n")
    jr      t7
    nop

TM_end:
    dli     v0, 0x0
    dli     a1, 0x1
    bltu    s6, a1, 1f
    nop
    //s6 >= a1, set error mark
    dli     v0, 0x1
1:
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
    
    move    t7, v0
/*
 *Unlock Scache 9800?00000000000 ~ 9800?00000001000(4K)
 */
    PRINTSTR("\r\nUnlock Scache Node x--9800?00000000000~4K...\r\n")
    GET_TM_NODE_ID_a1
    dsll    a1, a1, 44
    dli     a2, 0x900000003ff00200
    daddu   a2, a2, a1
    dli     a3, 0x0000000000000000
    sd      a3, 0x0(a2)
    PRINTSTR("Unlock Scache Done.\r\n")

    move    v0, t7
    jr      t8
    nop
RL_end:
