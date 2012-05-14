/********************
Author: Chen Xinke
Function: Macro defination for Test_Mem.S
********************/
//===========================
//WalkOnes
#define PATTERN_D8_0_0  0x01010101
#define PATTERN_D8_0_1  0x02020202
#define PATTERN_D8_0_2  0x04040404
#define PATTERN_D8_0_3  0x08080808
#define PATTERN_D8_0_4  0x10101010
#define PATTERN_D8_0_5  0x20202020
#define PATTERN_D8_0_6  0x40404040
#define PATTERN_D8_0_7  0x80808080
//WalkInvOnes
#define PATTERN_D8_1_0  0x01010101
#define PATTERN_D8_1_1  0xfefefefe
#define PATTERN_D8_1_2  0x02020202
#define PATTERN_D8_1_3  0xfdfdfdfd
#define PATTERN_D8_1_4  0x04040404
#define PATTERN_D8_1_5  0xfbfbfbfb
#define PATTERN_D8_1_6  0x08080808
#define PATTERN_D8_1_7  0xf7f7f7f7
//WalkZeros
#define PATTERN_D8_2_0  0xfefefefe
#define PATTERN_D8_2_1  0xfdfdfdfd
#define PATTERN_D8_2_2  0xfbfbfbfb
#define PATTERN_D8_2_3  0xf7f7f7f7
#define PATTERN_D8_2_4  0xefefefef
#define PATTERN_D8_2_5  0xdfdfdfdf
#define PATTERN_D8_2_6  0xbfbfbfbf
#define PATTERN_D8_2_7  0x7f7f7f7f
//WalkFewerOnes
#define PATTERN_D8_3_0  0x00000001
#define PATTERN_D8_3_1  0x00000100
#define PATTERN_D8_3_2  0x00010000
#define PATTERN_D8_3_3  0x01000000
#define PATTERN_D8_3_4  0x01000000
#define PATTERN_D8_3_5  0x00010000
#define PATTERN_D8_3_6  0x00000100
#define PATTERN_D8_3_7  0x00000001

#define PATTERN_DB_0_0  0x00000000
#define PATTERN_DB_0_1  0xffffffff
#define PATTERN_DB_1_0  0x00000008
#define PATTERN_DB_1_1  0xfffffff7
#define PATTERN_DB_2_0  0x5aa5a55a
#define PATTERN_DB_2_1  0xa55a5aa5
#define PATTERN_DB_3_0  0xb5b5b5b5
#define PATTERN_DB_3_1  0x4a4a4a4a

#define PATTERN_JUSTA   0xaaaaaaaa
#define PATTERN_JUST5   0x55555555
#define PATTERN_FiveA   0x5555aaaa
#define PATTERN_ZEROONE 0x0000ffff
#define PATTERN_L8b10b  0x16161616
#define PATTERN_S8b10b  0xb5b5b5b5
#define PATTERN_Five7   0x55555557
#define PATTERN_Zero2fd 0x0002fffd

#define MEM_TEST_BASE   0x80000000
#define UNCACHED_MEM_TEST_BASE   0xa0000000

#define GET_TM_START_ADDR   li   a1, 0x3fffffff; and a1, s1, a1;
#define GET_TM_MSIZE    li  a1, 0xc0000000;and a1, a1, s1; srl a1, a1, 6;
//Memory size to be tested(x16MB)

#define GET_RD_LEVEL    li  a1, 0x1; and a1, s4, a1;
#define GET_MICRO_TUNE  li  a2, 0x10; and a2, s4, a2; srl a2, a2, 4;
#define GET_DISPRINT_BIT    li  a1, 0x1000; and a1, s4, a1;

//#define LEVEL_SPECIFIED_BYTE_LANES
#ifdef  LEVEL_SPECIFIED_BYTE_LANES
//#define LEVEL_ONE_BYTE
#ifndef LEVEL_ONE_BYTE
#define LEVEL_BYTES_MASK    0xffffff00
#endif
#endif

#define REDUCED_MEM_TEST
#define MACRO_SCALE     3

#define TM_MAX_RPT_ERRORS    0x10
#define TM_MAX_ERRORS  0x10
//#define   PRINT_LESS_ERROR
//===========================
//for simple_test_mem
#define TMF_PWRLOOP   0x1
#define TMF_PRDLOOP   0x1
#define SIMPLE_TM_BASE   0x80000000
#define UNCACHED_SIMPLE_TM_BASE   0xa0000000
//===========================
