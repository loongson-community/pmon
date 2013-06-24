/********************
Author: Chen Xinke
Function: Macro defination for Test_Mem.S
********************/
#ifndef TM_PATTERN
#define TM_PATTERN
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
//Inverse
#define PATTERN_D8_4_0  0xaaaaaaaaaaaaaaaa
#define PATTERN_D8_4_1  0x5555555555555555
#define PATTERN_D8_4_2  0x0000000800000008
#define PATTERN_D8_4_3  0xfffffff7fffffff7
#define PATTERN_D8_4_4  0x5aa5a55a5aa5a55a
#define PATTERN_D8_4_5  0xa55a5aa5a55a5aa5
#define PATTERN_D8_4_6  0xb5b5b5b5b5b5b5b5
#define PATTERN_D8_4_7  0x4a4a4a4a4a4a4a4a
//Random
#define PATTERN_D8_5_0  0x0000000000000000
#define PATTERN_D8_5_1  0xffffffffffffffff
#define PATTERN_D8_5_2  0x55555555aaaaaaaa
#define PATTERN_D8_5_3  0x00000000ffffffff
#define PATTERN_D8_5_4  0x1616161616161616
#define PATTERN_D8_5_5  0xb5b5b5b5b5b5b5b5
#define PATTERN_D8_5_6  0x5555555755575555
#define PATTERN_D8_5_7  0x00020002fffdfffd

#define PATTERN_DB_0_0  0xaaaaaaaaaaaaaaaa
#define PATTERN_DB_0_1  0x5555555555555555
//#define PATTERN_DB_0_0  0x0000000000000000
//#define PATTERN_DB_0_1  0xffffffffffffffff
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

#define ARB_TM_BASE     0x9800001000000000
#define MT_PATTERN_BASE 0x9800001000000000  //(0 ~ 400 -- 0 ~ 1K)
#define MT_STACK_BASE   0x9800001000000400  //(400 ~ 600 -- 512Byte max, 64 registers)

#define GET_TM_NODE_ID_a1 GET_ARB_LEVEL_NODE_ID 

#define ROW_COL_UPPER_LIMIT 27
#define GET_TM_UP_ADDR  dli t3, ARB_TM_BASE; GET_TM_NODE_ID_a1; dsll a1, a1, 44; or t3, t3, a1; daddu t3, t3, s4;
#define GET_TM_MSIZE    dli a1, 0x40000;
//Memory size to be tested

//#define LEVEL_SPECIFIED_BYTE_LANES
#ifdef  LEVEL_SPECIFIED_BYTE_LANES
#define LEVEL_BYTES_MASK    0xffffff0000000000
#endif

#define REDUCED_MEM_TEST
#define ADDR_INTERVAL   (0x400)

#define TM_MAX_ERRORS  0x10

//obsolete now
//#define GET_RD_LEVEL    dli a1, 0x0;
//#define GET_MICRO_TUNE  dli a2, 0x0;
//#define MACRO_SCALE     3
//obsolete now
