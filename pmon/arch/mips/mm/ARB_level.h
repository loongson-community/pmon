/*****************************
    Macro defination for Access Result Based Software Leveling
    Author: Chen Xinke
    v0.1    for LS3A3
*******************************/
//these define can be adjusted
#define WRLVL_DELAY_MINUS_VALUE (0x40)
#define WRLVL_DELAY_ADD_VALUE   (0x38)
#define WRLVL_DQ_SMALL_DLY      (0x18)
#define WRLVL_DQ_DEFAULT_DLY    (0x1c)
#ifdef  DDR3_SODIMM
//#define WRLVL_DDR3_UDIMM_DEFAULT_OFFSET (0x100c0400100c0400)    //for DDR3 SODIMM 1R-x8
#define WRLVL_DDR3_UDIMM_DEFAULT_OFFSET (0x16160e0e08080000)    //for SCS DDR3 SODIMM 1R-x8
#else
#ifdef  DDR3_SMT
#define WRLVL_DDR3_UDIMM_DEFAULT_OFFSET (0x12120c0c06060000)  //for DDR3 SMT    300M
//#define WRLVL_DDR3_UDIMM_DEFAULT_OFFSET (0x212116160b0b0000)  //for DDR3 SMT    330M
//#define WRLVL_DDR3_UDIMM_DEFAULT_OFFSET (0x242418180c0c0000)  //for DDR3 SMT    400M
#else
#define WRLVL_DDR3_UDIMM_DEFAULT_OFFSET (0x322a241e120c0600)    //for DDR3 UDIMM
#endif
#endif
//using when wrlvl fail
#ifdef  DDR3_DIMM
#define WRLVL_DEFAULT_VALUE     (0x5e5a545044403a34)
#else
#define WRLVL_DEFAULT_VALUE     (0x2020202020202020)
#endif
//for skip wrlvl use
#define WRLVL_CLK_OFFSET_VALUE  (0x5e5a545044403a34)    //for DDR3 UDIMM
//#define WRLVL_CLK_OFFSET_VALUE  (0x181812120c0c0606)    //for DDR3 SMT-771

#define RDLVL_GATE_INIT_DELAY   (0x0)
#define RDLVL_DEFAULT_DELAY     (0x20)
#define CLKLVL_DEFAULT_VALUE    (0x20)

//for wrlvl
#define LOG2_STEP   1   //log2 of TM step interval, remember to small WINDOW_ZERO_NUM when we use larger STEP
#define WINDOW_ZERO_NUM     0x8
//these value must be aligned to 4 or 2 according to LOG2_STEP/GATE_LOG2_STEP
#define CLKLVL_MAX_DELAY        (0x7c)
#define WRLVL_MAX_DELAY         (0x68)
#define WRLVL_DQ_MAX_DELAY      (0x50)
#define WRLVL_1QUARTER_CLK_VALUE    (0x20)
#define WRLVL_HALF_CLK_VALUE        (0x40)
#define WRLVL_3QUARTER_CLK_VALUE    (0x60)
#define WRLVL_ONE_CLK_VALUE         (0x80)
#define WRLVL_DELAY_LEVEL_UP_LIMIT  (0x80 - (1 << LOG2_STEP))
#define WRLVL_DELAY_PARAM_UP_LIMIT  (0x80 - 1)

//for rdlvl
#define GATE_LOG2_STEP  1   //log2 of TM step interval for read Gate level
#define GATE_ADJUST     0
#define RDLVL_LOG2_STEP   1   //log2 of TM step interval, remember to small WINDOW_ZERO_NUM when we use larger STEP
#define RDLVL_WINDOW_ZERO_NUM   (0x8)
#define RDLVL_GATE_MAX_DELAY    (0x22)
#define RDLVL_MAX_DELAY         (0x4c)

#define CPU_ODT_BASE_VALUE      (0x15)
#define CPU_ODT_INC_VALUE       (0x11)
//#define CPU_ODT_BASE_VALUE      (0x04)
//#define CPU_ODT_INC_VALUE       (0x12)

//#define MODIFY_DQSDQ_OUT_WINDOW
#define DQSDQ_OUT_WINDOW_VALUE      (0x03733)
#define PHY_0_DQSDQ_INC_VALUE_80P   (0x84400)
#define PHY_0_DQSDQ_INC_VALUE_60P   (0x30400)
#define PHY_0_DQSDQ_INC_VALUE_40P   (0x30000)
#define PHY_0_DQSDQ_INC_VALUE_20P   (0x00000)
#define PHY_0_DQSDQ_INC_VALUE_00P   (0x00000)

//the wrlvl can work without this delay(not sure now), but the rdlvl really need it. why???
//the current delay value should be enough
//#define MC_RST_DELAY        (0x1000)   //work ok
#define MC_RST_DELAY        (0x10000)   //work ok
//#define MC_RST_DELAY        (0x40000)   //work ok
//#define MC_RST_DELAY        (0x100000)    //work ok

#define GET_ARB_LEVEL_NODE_ID   dli a1, 0x3; and a1, a1, s1;

#define ARB_STORE_BASE  0x9800001000000600  //(600 ~ 800 -- 512B max, 64 registers)
#define ARB_STACK_BASE  0x9800001000000800  //(800 ~ a00 -- 512B max, 64 registers)
//don't change the relative address offset of the 8 slices
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
#define WRLVL_DQ_VALUE_ADDR (0x70)
