/********************
Author: Chen Xinke
Function: Macro defination for Test_Mem.S
********************/
//===========================
#ifndef TM_PATTERN
//WalkOnes
#define PATTERN_D8_0_0  0x0101010101010101
#define PATTERN_D8_0_1  0x0202020202020202
#define PATTERN_D8_0_2  0x0404040404040404
#define PATTERN_D8_0_3  0x0808080808080808
#define PATTERN_D8_0_4  0x1010101010101010
#define PATTERN_D8_0_5  0x2020202020202020
#define PATTERN_D8_0_6  0x4040404040404040
#define PATTERN_D8_0_7  0x8080808080808080
//WalkInvOnes
#define PATTERN_D8_1_0  0x0101010101010101
#define PATTERN_D8_1_1  0xfefefefefefefefe
#define PATTERN_D8_1_2  0x0202020202020202
#define PATTERN_D8_1_3  0xfdfdfdfdfdfdfdfd
#define PATTERN_D8_1_4  0x0404040404040404
#define PATTERN_D8_1_5  0xfbfbfbfbfbfbfbfb
#define PATTERN_D8_1_6  0x0808080808080808
#define PATTERN_D8_1_7  0xf7f7f7f7f7f7f7f7
//WalkZeros
#define PATTERN_D8_2_0  0xfefefefefefefefe
#define PATTERN_D8_2_1  0xfdfdfdfdfdfdfdfd
#define PATTERN_D8_2_2  0xfbfbfbfbfbfbfbfb
#define PATTERN_D8_2_3  0xf7f7f7f7f7f7f7f7
#define PATTERN_D8_2_4  0xefefefefefefefef
#define PATTERN_D8_2_5  0xdfdfdfdfdfdfdfdf
#define PATTERN_D8_2_6  0xbfbfbfbfbfbfbfbf
#define PATTERN_D8_2_7  0x7f7f7f7f7f7f7f7f
//WalkFewerOnes
#define PATTERN_D8_3_0  0x0000000100000001
#define PATTERN_D8_3_1  0x0000010000000100
#define PATTERN_D8_3_2  0x0001000000010000
#define PATTERN_D8_3_3  0x0100000001000000
#define PATTERN_D8_3_4  0x0100000001000000
#define PATTERN_D8_3_5  0x0001000000010000
#define PATTERN_D8_3_6  0x0000010000000100
#define PATTERN_D8_3_7  0x0000000100000001

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
#endif

#define MEM_TEST_OFFSET 0x100000
#define MEM_TEST_BASE   0x9800000000000000
#define MT_PATTERN_BASE 0x9800000000000000  //(0 ~ 400 -- 0 ~ 1K)
#define MT_STACK_BASE   0x9800000000000400  //(400 ~ 600 -- 512Byte max, 64 registers)
#define MT_CODE_BASE    0x9800000000000600  //(600 ~ 4000 -- 1.5K ~ 16K, 14.5K max)
#define MT_MSG_BASE     0x9800000000004000  //(4000 ~ 10000 -- 16K ~ 64K, 48K max)

#define GET_TM_NODE_ID_a1   dsrl a1, s1, 62;
#define GET_TM_CORE_ID_a1   dsrl a1, s1, 60; and a1, a1, 0x3;
#define GET_TM_START_ADDR   dli  a1, 0xfffffffffff; and a1, s1, a1;
#define GET_TM_MSIZE    dli a1, 0x00ff000000000000;and a1, a1, s1; dsrl a1, a1, 21; //Memory size to be tested
#define GET_NODE_MSIZE  dli  a1, 0x80000000;

#define GET_RD_LEVEL    dli a1, 0x1; and a1, s4, a1;
#define GET_MICRO_TUNE  dli a2, 0x10; and a2, s4, a2; dsrl a2, a2, 4;
#define GET_DISPRINT_BIT    dli a1, 0x1000; and a1, s4, a1;

//#define LEVEL_SPECIFIED_BYTE_LANES
#ifdef  LEVEL_SPECIFIED_BYTE_LANES
//#define LEVEL_ONE_BYTE
#ifndef LEVEL_ONE_BYTE
#define LEVEL_BYTES_MASK    0xff00000000000000
#endif
#endif

#define REDUCED_MEM_TEST
#define MACRO_SCALE     3

#define PRINT_LESS_ERROR
#define TM_MAX_RPT_ERRORS    0x20
//===========================
//for simple_test_mem
#define TMF_PWRLOOP   0x10
#define TMF_PRDLOOP   0x40
#define SIMPLE_TM_BASE   0x9800000000000000
#define UNCACHED_SIMPLE_TM_BASE   0x9000000000000000
//===========================
#define SET_TM_BASE_t1 \
    dli     t1, MEM_TEST_BASE; \
    and     a1, s4, 0x10000; \
    dsll    a1, a1, (59 - 16); \
    xor     t1, t1, a1; \
    and     a1, s4, 0x40000; \
    dsll    a1, a1, (61 - 18); \
    xor     t1, t1, a1; \
    GET_TM_NODE_ID_a1; \
    dsll    a1, a1, 44; \
    daddu   t1, t1, a1; \
    GET_TM_START_ADDR; \
    daddu   t1, t1, a1; \
    dli     a1, MEM_TEST_OFFSET; \
    daddu   t1, t1, a1

#define SET_TM_LIMIT_t3 \
    GET_TM_MSIZE; \
    daddu   t3, t1, a1; \
    dli     a1, MEM_TEST_OFFSET; \
    dsubu   t3, t3, a1;

