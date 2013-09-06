/*****************************
    Macro defination for Access Result Based Software Leveling
    Author: Chen Xinke
    v0.1    for LS3A3
*******************************/
//Used for Hard leveling--obsolete now
#define LVL_FIRST_STEP     (0x8)
#define LVL_SECOND_STEP    (0x1)
#define SET_CURRESP  dli  a1, 0x01; or   t3, t3, a1;
#define CLR_CURRESP  dli  a1, 0xfffffffffffffffe; and    t3, t3, a1;
#define GET_CURRESP  dli  a1, 0x01; and  a1, t3, a1;

#define SET_LVLSTAT  dli a1, 0x01;  or  t4, t4, a1;
#define CLR_LVLSTAT  dli a1, 0xfffffffffffffffe; and     t4, t4, a1;
#define GET_LVLSTAT  dli a1, 0x01;  and  a1, t4, a1;

#define SET_FIRST_LVL   dli a1, 0x8000000000000000; or  t4, t4, a1;
#define CLR_FIRST_LVL   dli a1, 0x7fffffffffffffff; and t4, t4, a1;
#define GET_FIRST_LVL   dsrl a1, t4, 63;

//ARB level
//the wrlvl can work without this delay(not sure now), but the rdlvl really need it. why???
//the current delay value should be enough
//#define MC_RST_DELAY        (0x1000)   //work ok
#define MC_RST_DELAY        (0x10000)   //work ok
//#define MC_RST_DELAY        (0x40000)   //work ok
//#define MC_RST_DELAY        (0x100000)    //work ok

//old level program tested data
//#define MC_RST_DELAY        0x4000000   //work good.
//#define MC_RST_DELAY        0x1000000   //seems not good.

#define LOG2_STEP   1   //log2 of TM step interval, remember to small WINDOW_ZERO_NUM when we use larger STEP
#define WINDOW_ZERO_NUM     0x8
#define GATE_LOG2_STEP  1   //log2 of TM step interval for read Gate level
#define GATE_ADJUST     0

//these value must be aligned to 4 or 2 according to LOG2_STEP/GATE_LOG2_STEP
#define CLKLVL_MAX_DELAY        (0x7c)
#define RDLVL_GATE_MAX_DELAY    (0x22)
#define RDLVL_MAX_DELAY         (0x4c)
#define WRLVL_MAX_DELAY         (0x68)
#define WRLVL_DQ_MAX_DELAY      (0x50)
#define WRLVL_DELAY_MINUS_VALUE (0x40)
#define WRLVL_DQ_DEFAULT_DLY    (0x1c)
#define WRLVL_DDR3_UDIMM_DEFAULT_VALUE (0x342e282418140e0c)   //for DDR3 UDIMM
//#define WRLVL_DDR3_UDIMM_DEFAULT_VALUE (0x100c0400100c0400)     //for DDR3 SODIMM 1R-x8
#define WRLVL_DDR3_UDIMM_DEFAULT_OFFSET (0x28221c180c060200)   //for DDR3 UDIMM

#define CPU_ODT_BASE_VALUE      (0x15)
#define CPU_ODT_INC_VALUE       (0x11)
//#define CPU_ODT_BASE_VALUE      (0x04)
//#define CPU_ODT_INC_VALUE       (0x12)

#define WRLVL_HALF_CLK_VALUE    (0x40)
#define DQSDQ_OUT_WINDOW_VALUE      (0x3733)
#define PHY_0_DQSDQ_INC_VALUE_80P   (0x84400)
#define PHY_0_DQSDQ_INC_VALUE_60P   (0x30400)
#define PHY_0_DQSDQ_INC_VALUE_40P   (0x30000)
#define PHY_0_DQSDQ_INC_VALUE_00P   (0x00000)
//#define  MODIFY_DQSDQ_OUT_WINDOW

#define GET_ARB_LEVEL_NODE_ID   dli a1, 0x3; and a1, a1, s1;

#define ARB_STORE_BASE  0x9800001000000600  //(600 ~ 800 -- 512B max, 64 registers)
#define ARB_STACK_BASE  0x9800001000000800  //(800 ~ a00 -- 512B max, 64 registers)
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

//those define use the same address as above, so they can't be used at the same time
#define BYTE_TM_RST         (0x0)
#define RDLVL_GATE_CFG      (0x8)
#define RDLVL_GATE_GD_MIN   (0x10)
#define RDLVL_GATE_GD_MAX   (0x18)
#define RDLVL_DELAYP_GD_MIN (0x20)
#define RDLVL_DELAYP_GD_MAX (0x28)
#define RDLVL_DELAYN_GD_MIN (0x30)
#define RDLVL_DELAYN_GD_MAX (0x38)
#define RDLVL_FAIL_MARK     (0x58)

#define CLKLVL_DELAY_VALUE  (0x68)
