/*****************************
    Macro defination for Access Result Based Software Leveling
    Author: Chen Xinke
    v0.1    for LS3A3
*******************************/
//select level which MC--2: MC1; 1: MC0
#define MC_SELECT 0x2
#define DDR_MC_CONFIG_BASE      0x900000000ff00000

#define LVL_FIRST_STEP     (0x8)
#define LVL_SECOND_STEP    (0x1)
#define CLKLVL_MAX_DELAY        (0x80)
#define WRLVL_MAX_DELAY         (0x80)
#define RDLVL_MAX_DELAY         (0x80)
#define RDLVL_GATE_MAX_DELAY    (0x22)

#define GET_ARB_LEVEL_NODE_ID   dli a1, 0x3; and a1, a1, s1;
#define SET_CURRESP  dli  a1, 0x01; or   t3, t3, a1;
#define CLR_CURRESP  dli  a1, 0xfffffffffffffffe; and    t3, t3, a1;
#define GET_CURRESP  dli  a1, 0x01; and  a1, t3, a1;

#define SET_LVLSTAT  dli a1, 0x01;  or  t4, t4, a1;
#define CLR_LVLSTAT  dli a1, 0xfffffffffffffffe; and     t4, t4, a1;
#define GET_LVLSTAT  dli a1, 0x01;  and  a1, t4, a1;

#define SET_FIRST_LVL   dli a1, 0x8000000000000000; or  t4, t4, a1;
#define CLR_FIRST_LVL   dli a1, 0x7fffffffffffffff; and t4, t4, a1;
#define GET_FIRST_LVL   dsrl a1, t4, 63;

#define ARB_STORE_BASE  0x9800000000000600  //(600 ~ 800 -- 512B max, 64 registers)
#define ARB_STACK_BASE  0x9800000000000800  //(800 ~ a00 -- 512B max, 64 registers)
#define B0_TM_RST   (0x0)
#define B1_TM_RST   (0x8)
#define B2_TM_RST   (0x10)
#define B3_TM_RST   (0x18)
#define B4_TM_RST   (0x20)
#define B5_TM_RST   (0x28)
#define B6_TM_RST   (0x30)
#define B7_TM_RST   (0x38)
//for all 8 bytes
#define GD_MAX      (0x40)
#define GD_MIN      (0x48)
#define GD_MID      (0x50)
#define BS_LEN      (0x58)
#define BS_VAL      (0x60)
#define LEVEL_SEQ_BASE   (0x80)
