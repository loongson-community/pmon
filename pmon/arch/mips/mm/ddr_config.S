/**********************************
    Author: chenxinke
    Date:   20150707
    v1.0 support ~3A3
    subroutine: mc_init
    input:
    t7(option ARB_LEVEL)--do arb level, 0--not level; 1--do level;
    t3--MC select: 0--MC0; 1--MC1
**********************************/
//#define ADJUST_CLKLVL_DELAY
#include "lsmcd3_config_param.S"

        .global mc_init
        .ent    mc_init
        .set    noreorder
        .set    mips3
mc_init:
    move    t4, ra

    sync
    nop
    nop
    nop
    nop

    GET_NODE_ID_a0;
    XBAR_CONFIG_NODE_a0(0x10, \
                    0x000000000ff00000, \
                    0xfffffffffff00000, \
                    0x000000000ff000f0)
    beqz    t3, 1f
    nop
    GET_NODE_ID_a0;
    XBAR_CONFIG_NODE_a0(0x10, \
                    0x000000000ff00000, \
                    0xfffffffffff00000, \
                    0x000000000ff000f1)
1:

    sync
    nop
    nop
    nop
    nop

    //TTYDBG("\r\nEnable register space of MEMORY\r\n")
    bal     enable_mc_conf_space
    nop

    GET_NODE_ID_a0;
    dli     t8, DDR_MC_CONFIG_BASE
    or      t8, t8, a0

    //set param location
#ifdef  ARB_LEVEL
    bnez    t7, 1f
    nop
    //if use leveled ddr param, the param location is fixed
    dla     a2, ddr2_reg_data_mc0_leveled
    beqz    t3, 21f
    nop
    dla     a2, ddr2_reg_data_mc1_leveled
21:
#ifdef  MULTI_CHIP
    GET_NODE_ID_a1
    beqz    a1, 21f
    nop
    dla     a2, n1_ddr2_reg_data_mc0_leveled
    beqz    t3, 21f
    nop
    dla     a2, n1_ddr2_reg_data_mc1_leveled
21:
#endif
    b       4f
    nop
1:
#endif
    GET_SDRAM_TYPE
    dli     t1, 0x2
    beq     t1, a1, 2f
    nop
    dli     t1, 0x3
    beq     t1, a1, 3f
    nop
    //not DDR2 and not DDR3, errors
    PRINTSTR("\r\n!!! ERROR: NOT recognized DDR SDRAM TYPE. !!!\r\n");
2:  //DDR2
    GET_DIMM_TYPE
    bnez    a1, 1f
    nop
    //UDIMM
    dla     a2, ddr2_reg_data
    beqz    t3, 21f
    nop
    dla     a2, ddr2_reg_data_mc1
21:
#ifdef  MULTI_NODE_DDR_PARAM
    GET_NODE_ID_a1
    beqz    a1, 4f
    nop
    dla     a2, n1_ddr2_reg_data
    beqz    t3, 21f
    nop
    dla     a2, n1_ddr2_reg_data_mc1
21:
#endif
    b       4f
    nop
1:  //RDIMM
    dla     a2, ddr2_RDIMM_reg_data
    beqz    t3, 21f
    nop
    dla     a2, ddr2_RDIMM_reg_data_mc1
21:
#ifdef  MULTI_NODE_DDR_PARAM
    GET_NODE_ID_a1
    beqz    a1, 4f
    nop
    dla     a2, n1_ddr2_RDIMM_reg_data
    beqz    t3, 21f
    nop
    dla     a2, n1_ddr2_RDIMM_reg_data_mc1
21:
#endif
    b       4f
    nop
3:  //DDR3
    GET_DIMM_TYPE
    bnez    a1, 1f
    nop
    //UDIMM
    dla     a2, ddr3_reg_data
    beqz    t3, 21f
    nop
    dla     a2, ddr3_reg_data_mc1
21:
#ifdef  MULTI_NODE_DDR_PARAM
    GET_NODE_ID_a1
    beqz    a1, 4f
    nop
    dla     a2, n1_ddr3_reg_data
    beqz    t3, 21f
    nop
    dla     a2, n1_ddr3_reg_data_mc1
21:
#endif
    b       4f
    nop
1:  //RDIMM
    dla     a2, ddr3_RDIMM_reg_data
    beqz    t3, 21f
    nop
    dla     a2, ddr3_RDIMM_reg_data_mc1
21:
#ifdef  MULTI_NODE_DDR_PARAM
    GET_NODE_ID_a1
    beqz    a1, 4f
    nop
    dla     a2, n1_ddr3_RDIMM_reg_data
    beqz    t3, 21f
    nop
    dla     a2, n1_ddr3_RDIMM_reg_data_mc1
21:
#endif
    b       4f
    nop
4:

    bal     ddr2_config
    nop

#ifdef  PRINT_MSG1
    //print MC registers
    PRINTSTR("\r\nThe MC param is:\r\n")
    dli     t1, DDR_PARAM_NUM
    GET_NODE_ID_a0
    dli     t5, DDR_MC_CONFIG_BASE
    or      t5, t5, a0
1:
    ld      t6, 0x0(t5)
    and     a0, t5, 0xfff
    bal     hexserial
    nop
    PRINTSTR(":  ")
    dsrl    a0, t6, 32
    bal     hexserial
    nop
    PRINTSTR("  ")
    move    a0, t6
    bal     hexserial
    nop
    PRINTSTR("\r\n")

    daddiu  t1, t1, -1
    daddiu  t5, t5, 16
    bnez    t1, 1b
    nop
#endif

    TTYDBG("\r\nDisable register space of MEMORY\r\n")
    bal     disable_mc_conf_space
    nop

//cxk
#ifdef  ARB_LEVEL
#ifdef  DEBUG_DDR_PARAM
    PRINTSTR("\r\nSkip Memory training?(0: use mark to decide;1: skip ARB_level;)\r\n");
    dli     t6, 0x00
    bal     inputaddress    #input value stored in v0
    nop
    bnez    v0, 8f
    nop
#endif
    //read ARB_level
    beqz    t7, 8f
    nop

    //route 0x1000000000 ~ 0x1FFFFFFFFF(64G) to MC for ARB_level
    sync
    nop
    nop
    nop
    nop
    GET_NODE_ID_a0;
    XBAR_CONFIG_NODE_a0(ARB_TEMP_L2WINDOW_OFFSET, \
                    0x0000001000000000, \
                    0xFFFFFFF000000000, \
                    0x00000000000000F0)
    beqz    t3, 2f
    nop
    GET_NODE_ID_a0;
    XBAR_CONFIG_NODE_a0(ARB_TEMP_L2WINDOW_OFFSET, \
                    0x0000001000000000, \
                    0xFFFFFFF000000000, \
                    0x00000000000000F1)
2:
    sync
    nop
    nop
    nop
    nop

    bal     ARB_level
    nop

    sync
    nop
    nop
    nop
    nop

    L2XBAR_CLEAR_WINDOW(ARB_TEMP_L2WINDOW_OFFSET)

    sync
    nop
    nop
    nop
    nop

8:
#endif

#ifndef  DISABLE_DIMM_ECC
    //Init ECC according to DIMM ECC info
    GET_DIMM_ECC
    beqz    a1, 4f
    nop
    TTYDBG("ECC init start(maybe take 1 minute or so)....\r\n")

    TTYDBG("Enable MC read buffer\r\n")
    bal     enable_mc_read_buffer
    nop

    //TTYDBG("Enable register space of MEMORY\r\n")
    bal     enable_mc_conf_space
    nop

    //set ecc_disable_w_uc_err to 1'b1
    //TTYDBG("set ecc_disable_w_ur_err.\r\n")
    ld      a2, ECC_DISABLE_W_UC_ERR_ADDR(t8)
    dli     a1, 0xff
    dsll    a1, a1, ECC_DISABLE_W_UC_ERR_OFFSET
    not     a1, a1
    and     a2, a2, a1
    dli     a1, 0x01
    dsll    a1, a1, ECC_DISABLE_W_UC_ERR_OFFSET
    or      a2, a2, a1
    sd      a2, ECC_DISABLE_W_UC_ERR_ADDR(t8)
    //enable ECC module with ecc reporting and correcting.
    //TTYDBG("enable ECC.\r\n")
    ld      a2, CTRL_RAW_ADDR(t8)
    dli     a1, 0xff
    dsll    a1, a1, CTRL_RAW_OFFSET
    not     a1, a1
    and     a2, a2, a1
    dli     a1, 0x03
    dsll    a1, a1, CTRL_RAW_OFFSET
    or      a2, a2, a1
    sd      a2, CTRL_RAW_ADDR(t8)

    //TTYDBG("Disable register space of MEMORY\r\n")
    bal     disable_mc_conf_space
    nop

    //route 0x1000000000 ~ 0x1FFFFFFFFF(64G) to MC for ECC init
    sync
    nop
    nop
    nop
    nop
    GET_NODE_ID_a0;
    XBAR_CONFIG_NODE_a0(ARB_TEMP_L2WINDOW_OFFSET, \
                    0x0000001000000000, \
                    0xFFFFFFF000000000, \
                    0x00000000000000F0)
    beqz    t3, 1f
    nop
    GET_NODE_ID_a0;
    XBAR_CONFIG_NODE_a0(ARB_TEMP_L2WINDOW_OFFSET, \
                    0x0000001000000000, \
                    0xFFFFFFF000000000, \
                    0x00000000000000F1)

1:
    sync
    nop
    nop
    nop
    nop
//init mem to all 0
#ifdef FASTECC2
    dli     t1, 0xb800001000000000
#else
    dli     t1, 0x9800001000000000
#endif
    GET_NODE_ID_a0
    or      t1, t1, a0
    GET_MC0_MEMSIZE
    beqz    t3, 1f
    nop
    GET_MC1_MEMSIZE
1:
    dsll    a1, a1, 29   //a1*512M
#if defined(FASTECC) || defined(FASTECC1) || defined(FASTECC2)
    li a1,0x10000000
#endif

    daddu   t5, t1, a1
//write memory
1:
    bgeu    t1, t5, 1f
    nop

#ifdef FASTECC2
    sd      $0, 0x0(t1)
    sd      $0, 0x8(t1)
    sd      $0, 0x10(t1)
    sd      $0, 0x18(t1)

    sd      $0, 0x20(t1)
    sd      $0, 0x28(t1)
    sd      $0, 0x30(t1)
    sd      $0, 0x38(t1)

    sd      $0, 0x40(t1)
    sd      $0, 0x48(t1)
    sd      $0, 0x50(t1)
    sd      $0, 0x58(t1)

    sd      $0, 0x60(t1)
    sd      $0, 0x68(t1)
    sd      $0, 0x70(t1)
    sd      $0, 0x78(t1)

    sd      $0, 0x80(t1)
    sd      $0, 0x88(t1)
    sd      $0, 0x90(t1)
    sd      $0, 0x98(t1)

    sd      $0, 0xa0(t1)
    sd      $0, 0xa8(t1)
    sd      $0, 0xb0(t1)
    sd      $0, 0xb8(t1)

    sd      $0, 0xc0(t1)
    sd      $0, 0xc8(t1)
    sd      $0, 0xd0(t1)
    sd      $0, 0xd8(t1)

    sd      $0, 0xe0(t1)
    sd      $0, 0xe8(t1)
    sd      $0, 0xf0(t1)
    sd      $0, 0xf8(t1)
#else

    sd      $0, 0x0(t1)

    sd      $0, 0x20(t1)

    sd      $0, 0x40(t1)

    sd      $0, 0x60(t1)

    sd      $0, 0x80(t1)

    sd      $0, 0xa0(t1)

    sd      $0, 0xc0(t1)

    sd      $0, 0xe0(t1)
#endif


    sync

#ifndef FASTECC2
    //hit write back invalidate(D-/S-cache) to memory
#    cache   0x15, 0x0(t1)
    cache   0x17, 0x0(t1)

#    cache   0x15, 0x20(t1)
    cache   0x17, 0x20(t1)

#    cache   0x15, 0x40(t1)
    cache   0x17, 0x40(t1)

#    cache   0x15, 0x60(t1)
    cache   0x17, 0x60(t1)

#    cache   0x15, 0x80(t1)
    cache   0x17, 0x80(t1)

#    cache   0x15, 0xa0(t1)
    cache   0x17, 0xa0(t1)

#    cache   0x15, 0xc0(t1)
    cache   0x17, 0xc0(t1)

#    cache   0x15, 0xe0(t1)
    cache   0x17, 0xe0(t1)
#endif

    daddu   t1, t1, 0x100
    b       1b
    nop
1:

    sync
    nop
    nop
    nop
    nop

    L2XBAR_CLEAR_WINDOW(ARB_TEMP_L2WINDOW_OFFSET)

    sync
    nop
    nop
    nop
    nop
#if ! (defined(FASTECC)|| defined(FASTECC1)||defined(FASTECC2))
    //TTYDBG("Enable register space of MEMORY\r\n")
    bal     enable_mc_conf_space
    nop

    //clear disable_w_uc_err
    ld      a2, ECC_DISABLE_W_UC_ERR_ADDR(t8)
    dli     a1, 0xff
    dsll    a1, a1, ECC_DISABLE_W_UC_ERR_OFFSET
    not     a1, a1
    and     a2, a2, a1
    dli     a1, 0x00
    dsll    a1, a1, ECC_DISABLE_W_UC_ERR_OFFSET
    or      a2, a2, a1
    sd      a2, ECC_DISABLE_W_UC_ERR_ADDR(t8)

    //TTYDBG("Disable register space of MEMORY\r\n")
    bal     disable_mc_conf_space
    nop

    TTYDBG("Disable MC read buffer\r\n")
    bal     disable_mc_read_buffer
    nop
#endif

    TTYDBG("MC ECC init done.\r\n")
4:
#endif
#define CLEAR_INT_STATUS
#ifdef  CLEAR_INT_STATUS
    //TTYDBG("Enable register space of MEMORY\r\n")
    bal     enable_mc_conf_space
    nop

    //set int_mask to mask out unrelated mc_int
    ld      a1, MC_INT_MASK_ADDR(t8)
    dli     a2, MC_INT_MASK_VALUE
    dsll    a2, a2, MC_INT_MASK_OFFSET
    or      a1, a1, a2
    sd      a1, MC_INT_MASK_ADDR(t8)

    //clear all int status info
    //set int_ack to 1
    ld      a1, MC_INT_ACK_ADDR(t8)
    dli     a2, MC_INT_ACK_CLEAR_VALUE
    dsll    a2, a2, MC_INT_ACK_OFFSET
    or      a1, a1, a2
    sd      a1, MC_INT_ACK_ADDR(t8)
    sync

    //delay some time
    dli     a2, 0x40
1:
    daddiu  a2, a2, -1
    bnez    a2, 1b
    nop

    //clear int_ack to 0
    ld      a1, MC_INT_ACK_ADDR(t8)
    dli     a2, MC_INT_ACK_CLEAR_VALUE
    dsll    a2, a2, MC_INT_ACK_OFFSET
    not     a2, a2
    and     a1, a1, a2
    sd      a1, MC_INT_ACK_ADDR(t8)
    sync

#ifdef  PRINT_MSG1
    //print MC registers
    PRINTSTR("\r\nThe MC param is:\r\n")
    dli     t1, DDR_PARAM_NUM
    GET_NODE_ID_a0
    dli     t5, DDR_MC_CONFIG_BASE
    or      t5, t5, a0
1:
    ld      t6, 0x0(t5)
    and     a0, t5, 0xfff
    bal     hexserial
    nop
    PRINTSTR(":  ")
    dsrl    a0, t6, 32
    bal     hexserial
    nop
    PRINTSTR("  ")
    move    a0, t6
    bal     hexserial
    nop
    PRINTSTR("\r\n")

    daddiu  t1, t1, -1
    daddiu  t5, t5, 16
    bnez    t1, 1b
    nop
#endif

    //TTYDBG("\r\nDisable register space of MEMORY\r\n")
    bal     disable_mc_conf_space
    nop
#endif

    //clear the L2 xbar window configured for mc_init
    sync
    nop
    nop
    nop
    nop

    L2XBAR_CLEAR_WINDOW(0x10)

    sync
    nop
    nop
    nop
    nop

    jr      t4
    nop
    .end    mc_init

LEAF(enable_mc_conf_space)
/*********************
pre-condition::
    t2: chip configure register address
*********************/
    lw      a1, 0x0(t2)
    li      a2, 0x1
    sll     a2, a2, DDR_CONFIG_DISABLE_OFFSET
    not     a2, a2
    and     a1, a1, a2
    sw      a1, 0x0(t2)
    sync

    jr      ra
    nop
END(enable_mc_conf_space)

LEAF(disable_mc_conf_space)
/*********************
pre-condition::
    t2: chip configure register address
*********************/
    lw      a1, 0x0(t2)
    li      a2, 0x1
    sll     a2, a2, DDR_CONFIG_DISABLE_OFFSET
    or      a1, a1, a2
    sw      a1, 0x0(t2)
    sync

    jr      ra
    nop
END(disable_mc_conf_space)

LEAF(enable_mc_read_buffer)
/*********************
pre-condition::
    t2: chip configure register address
*********************/
    lw      a1, 0x4(t2)
    li      a0, 0x8
    sll     a0, a0, t3
    not     a0, a0
    and     a1, a1, a0
    sw      a1, 0x4(t2)
    sync

    jr      ra
    nop
END(enable_mc_read_buffer)

LEAF(disable_mc_read_buffer)
/*********************
pre-condition::
    t2: chip configure register address
*********************/
    lw      a1, 0x4(t2)
    li      a0, 0x8
    sll     a0, a0, t3
    or      a1, a1, a0
    sw      a1, 0x4(t2)
    sync

    jr      ra
    nop
END(disable_mc_read_buffer)

LEAF(disable_cpu_buffer_read)
    lw      a1, 0x0(t2)
    li      a0, 0xfffffdff
    and     a1, a1, a0
    sw      a1, 0x0(t2)

    jr      ra
    nop
END(disable_cpu_buffer_read)

LEAF(get_mem_clk)
/*********************
input(reserved now):
    a1: node id(currently fix a1 to 0x0)
output:
    v0: current ddr freq setting
*********************/
    move    a1, $0
    dsll    a1, a1, 44

    dli     a2, CHIP_CONFIG_BASE_ADDR
    or      a2, a2, a1
    ld      a0, 0x0(a2)
    dsrl    a0, a0, DDR_CLKSEL_EN_OFFSET
    and     v0, a0, 0x1

    bnez    v0, 1f
    nop
    //hard configure
    dli     a2, CHIP_SAMPLE_BASE_ADDR
    or      a2, a2, a1
    ld      a0, 0x0(a2)
    dsrl    a0, a0, DDR_CLKSEL_OFFSET
    and     a0, a0, DDR_CLKSEL_MASK
    b       2f
    nop
1:
    //soft configure
    dli     a2, CHIP_CONFIG_BASE_ADDR
    or      a2, a2, a1
    ld      a0, 0x0(a2)
    dsrl    a0, a0, DDR_CLKSEL_SOFT_OFFSET
    and     a0, a0, DDR_CLKSEL_SOFT_MASK
2:
    move    v0, a0

    jr      ra
    nop
END(get_mem_clk)

#ifdef  CHECK_ARB_LEVEL_FREQ
LEAF(GET_DDR_FREQ_CONFIG)
/*********************
input:
    a1: node id
output:
    v0: current ddr freq setting
*********************/
    dsll    a1, a1, 44

    dli     a2, CHIP_CONFIG_BASE_ADDR
    or      a2, a2, a1
    ld      a0, 0x0(a2)
    dsrl    a0, a0, DDR_CLKSEL_EN_OFFSET
    and     v0, a0, 0x1

    dli     a2, CHIP_SAMPLE_BASE_ADDR
    or      a2, a2, a1
    ld      a0, 0x0(a2)
    dsrl    a0, a0, DDR_CLKSEL_OFFSET
    and     a0, a0, DDR_CLKSEL_MASK
    dsll    a0, a0, 1
    or      v0, v0, a0

    dli     a2, CHIP_CONFIG_BASE_ADDR
    or      a2, a2, a1
    ld      a0, 0x0(a2)
    dsrl    a0, a0, DDR_CLKSEL_SOFT_OFFSET
    and     a0, a0, DDR_CLKSEL_SOFT_MASK
    dsll    a0, a0, (1 + DDR_CLKSEL_WIDTH)
    or      v0, v0, a0

    jr      ra
    nop
END(GET_DDR_FREQ_CONFIG)

LEAF(CHECK_DDR_FREQ_CHANGE)
/*********************
check whether the chip ddr clksel is changed
input:
    t7: mc level info addr
    v0: stored ddr clksel info
output:
    v0: 0--not changed; !0--changed
*********************/
    ld      a0, 0x28(t7)
    xor     v0, v0, a0
    //check whether in the same mode(soft or hard)
    and     a1, v0, 1
    bnez    a1, 88f
    nop
    //if clksel_en is equal, mask out the uncare part
    and     a1, a0, 1
    beqz    a1, 1f
    nop
    //soft configure
    dli     a0, DDR_CLKSEL_SOFT_MASK
    dsll    a0, a0, (1 + DDR_CLKSEL_WIDTH)
    and     v0, v0, a0
    b       88f
    nop
1:  //hard configure
    dli     a0, DDR_CLKSEL_MASK
    dsll    a0, a0, 1
    and     v0, v0, a0
88:
    jr      ra
    nop
END(CHECK_DDR_FREQ_CHANGE)
#endif

#ifdef  AUTO_ARB_LEVEL
LEAF(SET_AUTO_ARB_LEVEL_MARK)
/******************************
    input:
    t3: MC select;
    s1: node info, I2C node ID
    s3: MC slot I2C address
******************************/
    move    t6, ra

//set t7 to mc level info address
//if define CHECK_ARB_LEVEL_DIMM, set t1 at the same time for CHECK_DIMM_CHANGE
    dla     t7, c0_mc0_level_info
#ifdef  CHECK_ARB_LEVEL_DIMM
    move    t1, s3
#endif
    beqz    t3, 1f
    nop
    dla     t7, c0_mc1_level_info
#ifdef  CHECK_ARB_LEVEL_DIMM
    dsrl    t1, s3, 8
#endif
1:
#ifdef  MULTI_CHIP
    GET_NODE_ID_a1
    beqz    a1, 4f
    nop
    dla     t7, c1_mc0_level_info
#ifdef  CHECK_ARB_LEVEL_DIMM
    dsrl    t1, s3, 16
#endif
    beqz    t3, 4f
    nop
    dla     t7, c1_mc1_level_info
#ifdef  CHECK_ARB_LEVEL_DIMM
    dsrl    t1, s3, 24
#endif
4:
#endif
    daddu   t7, t7, s0
#ifdef  DEBUG_AUTO_ARB_LEVEL
    PRINTSTR("\r\nstored level info addr is 0x")
    dsrl    a0, t7, 32
    bal     hexserial
    nop
    move    a0, t7
    bal     hexserial
    nop
    PRINTSTR("\r\nsaved dimm infor is 0x")
    ld      t8, 0x0(t7)
    dsrl    a0, t8, 32
    bal     hexserial
    nop
    move    a0, t8
    bal     hexserial
    nop
    PRINTSTR("\r\n")
    ld      t8, 0x8(t7)
    dsrl    a0, t8, 32
    bal     hexserial
    nop
    move    a0, t8
    bal     hexserial
    nop
    PRINTSTR("\r\n")
    ld      t8, 0x10(t7)
    dsrl    a0, t8, 32
    bal     hexserial
    nop
    move    a0, t8
    bal     hexserial
    nop
    PRINTSTR("\r\n")
    ld      t8, 0x18(t7)
    dsrl    a0, t8, 32
    bal     hexserial
    nop
    move    a0, t8
    bal     hexserial
    nop
    PRINTSTR("\r\n")
    ld      t8, 0x20(t7)
    dsrl    a0, t8, 32
    bal     hexserial
    nop
    move    a0, t8
    bal     hexserial
    nop
    PRINTSTR("\r\n")
    ld      t8, 0x28(t7)
    dsrl    a0, t8, 32
    bal     hexserial
    nop
    move    a0, t8
    bal     hexserial
    nop
    PRINTSTR("\r\n")
#endif
    //check level mark(first boot or first populate DIMM)
    ld      v0, 0x0(t7)
    and     v0, v0, 0x1
    beqz    v0, 1f 
    nop
    PRINTSTR("\r\nThis MC has been leveled.");

#ifdef  CHECK_ARB_LEVEL_FREQ
    GET_NODE_ID_a1
    bal     GET_DDR_FREQ_CONFIG
    nop
    bal     CHECK_DDR_FREQ_CHANGE
    nop
    bnez    v0, 1f
    nop
    PRINTSTR("\r\nThis MC frequency has not been changed.");
#endif

#ifdef  CHECK_ARB_LEVEL_DIMM
    bal     CHECK_DIMM_CHANGE
    nop
    bnez    v0, 1f
    nop
    PRINTSTR("\r\nThis MC DIMMs have not been changed.");
#endif
    //set no arb level mark
    move    t7, $0
    b       2f
    nop
1:
    //set do arb level mark
    or      t7, $0, 0x1
    or      v0, $0, 0x1
    dsll    a2, v0, 32
    GET_NODE_ID_a1
    dsll    a1, a1, 1
    dsll    a2, a2, a1
    dsll    a2, a2, t3
    or      s3, s3, a2
2:
#ifdef  DEBUG_AUTO_ARB_LEVEL
    PRINTSTR("\r\ns3 = 0x");
    dsrl    a0, s3, 32
    bal     hexserial
    nop
    PRINTSTR("__")
    move    a0, s3
    bal     hexserial
    nop
    PRINTSTR("\r\n")
#endif
    jr      t6
    nop
END(SET_AUTO_ARB_LEVEL_MARK)
#endif

LEAF(config_mc_frequency)
/******************************
    input:
    a2: node id
    t5: MC frequency clksel value
******************************/
    dli     t2, CHIP_CONFIG_BASE_ADDR
    dsll    a2, a2, 44
    or      t2, t2, a2

    /* clear ddr_clksel */
    lw      a1, 0x0(t2)
    li      a0, 0x1f
    sll     a0, a0, 24
    not     a0, a0
    and     a1, a1,a0

    /* write new clksel */
    sll     a0, t5, 24
    or      a1, a1, a0
    sw      a1, 0x0(t2)

    /* enable software ddr_clksel */
    li      a0, 0x1
    sll     a0, a0, 3
    or      a1, a1, a0
    sw      a1, 0x0(t2)

    jr      ra
    nop
END(config_mc_frequency)
