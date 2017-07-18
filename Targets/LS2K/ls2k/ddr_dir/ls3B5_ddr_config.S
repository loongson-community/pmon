/**********************************
    Author: chenxinke
    Date:   20150707
    mc_init for 3B5~
    v1.0
    input:
    t7(option ARB_LEVEL)--do arb level, 0--not level; 1--do level;
**********************************/
#include "lsmc_config_param.S"

        .global mc_init
        .ent    mc_init
        .set    noreorder
        .set    mips3
mc_init:
    move    t7, ra

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
    bnez    t7, 5f
    nop
    //if use leveled ddr param, the param location is fixed
    dla     a2, ddr2_reg_data_leveled
    move    v0, $0
    GET_NODE_ID_a1
    beq     a1, v0, 4f
    nop
    dla     a2, n1_ddr2_reg_data_leveled
    daddu   v0, v0, 0x1
    beq     a1, v0, 4f
    nop
#ifdef  DUAL_3B
    dla     a2, n2_ddr2_reg_data_leveled
    daddu   v0, v0, 0x1
    beq     a1, v0, 4f
    nop
    dla     a2, n3_ddr2_reg_data_leveled
    daddu   v0, v0, 0x1
    beq     a1, v0, 4f
    nop
#endif
    b       4f
    nop
5:
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
    b       3f
    nop
2:  //DDR2
    GET_DIMM_TYPE
    bnez    a1, 1f
    nop
    //UDIMM
    dla     a2, ddr2_reg_data
    GET_NODE_ID_a1
    and     v1, a1, 0x1
    beqz    v1, 4f
    nop
    dla     a2, ddr2_reg_data_mc1
#ifdef  MULTI_NODE_DDR_PARAM
    GET_NODE_ID_a1
    dsrl    v1, a1, 1
    beqz    v1, 4f
    nop
    dla     a2, n1_ddr2_reg_data
    and     v1, a1, 0x1
    beqz    v1, 4f
    nop
    dla     a2, n1_ddr2_reg_data_mc1
#endif
    b       4f
    nop
1:  //RDIMM
    dla     a2, ddr2_RDIMM_reg_data
    GET_NODE_ID_a1
    and     v1, a1, 0x1
    beqz    v1, 4f
    nop
    dla     a2, ddr2_RDIMM_reg_data_mc1
#ifdef  MULTI_NODE_DDR_PARAM
    GET_NODE_ID_a1
    dsrl    v1, a1, 1
    beqz    v1, 4f
    nop
    dla     a2, n1_ddr2_RDIMM_reg_data
    and     v1, a1, 0x1
    beqz    v1, 4f
    nop
    dla     a2, n1_ddr2_RDIMM_reg_data_mc1
#endif
    b       4f
    nop
3:  //DDR3
    GET_DIMM_TYPE
    bnez    a1, 1f
    nop
    //UDIMM
    dla     a2, ddr3_reg_data
    GET_NODE_ID_a1
    and     v1, a1, 0x1
    beqz    v1, 4f
    nop
    dla     a2, ddr3_reg_data_mc1
#ifdef  MULTI_NODE_DDR_PARAM
    GET_NODE_ID_a1
    dsrl    v1, a1, 1
    beqz    v1, 4f
    nop
    dla     a2, n1_ddr3_reg_data
    and     v1, a1, 0x1
    beqz    v1, 4f
    nop
    dla     a2, n1_ddr3_reg_data_mc1
#endif
    b       4f
    nop
1:  //RDIMM
    dla     a2, ddr3_RDIMM_reg_data
    GET_NODE_ID_a1
    and     v1, a1, 0x1
    beqz    v1, 4f
    nop
    dla     a2, ddr3_RDIMM_reg_data_mc1
#ifdef  MULTI_NODE_DDR_PARAM
    GET_NODE_ID_a1
    dsrl    v1, a1, 1
    beqz    v1, 4f
    nop
    dla     a2, n1_ddr3_RDIMM_reg_data
    and     v1, a1, 0x1
    beqz    v1, 4f
    nop
    dla     a2, n1_ddr3_RDIMM_reg_data_mc1
#endif
4:
    move    t3, $0
    bal     ddr2_config
    nop

#if 1   //def  DEBUG_DDR_PARAM   //print registers
    PRINTSTR("The MC param is:\r\n")
    dli     t1, DDR_PARAM_NUM
    GET_NODE_ID_a0
    dli     t5, DDR_MC_CONFIG_BASE
    or      t5, t5, a0
1:
    ld      t6, 0x0(t5)
    move    a0, t5
    and     a0, a0, 0xfff
    bal     hexserial
    nop
    PRINTSTR(":  ")
    dsrl    a0, t6, 32
    bal     hexserial
    nop
    move    a0, t6
    bal     hexserial
    nop
    PRINTSTR("\r\n")

    daddiu  t1, t1, -1
    daddiu  t5, t5, 8
    bnez    t1, 1b
    nop
#endif

    TTYDBG("\r\nDisable register space of MEMORY\r\n")
    bal     disable_mc_conf_space
    nop

#ifdef  ARB_LEVEL   //Can not enable. because the ra destroy t7 and code is not port ok.
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
#else
#ifndef DISABLE_HARD_LEVELING
    dli     a2, 0x3
    GET_SDRAM_TYPE
    bne     a1, a2, 88f
    nop
    //DDR3 SDRAM, do hard leveling
    PRINTSTR("Start Hard Leveling...\r\n")
    TTYDBG("\r\nEnable register space of MEMORY\r\n")
    bal     enable_mc_conf_space
    nop

    GET_NODE_ID_a0
    dli     t8, DDR_MC_CONFIG_BASE
    or      t8, t8, a0
    bal     ddr3_leveling
    nop

    //Re-set t0&t2 because mc_init(ddr3_leveling) will change t0~t9
    GET_NODE_ID_a0
    dli     t2, 0x900000001fe00180
    dli     t0, 0x900000003ff00000
    or      t2, t2, a0
    or      t0, t0, a0
    //add 3B register location offset
    dsrl    a0, a0, 44
    dsll    a0, a0, 14
    or      t0, t0, a0
    //clear odd NODE_ID to even for chip config
    dli     a1, 0x1
    dsll    a1, a1, 44
    not     a1, a1
    and     t2, t2, a1

#if 1   //def  DEBUG_DDR_PARAM   //print registers
    PRINTSTR("The MC param after leveling is:\r\n")
    dli     t1, DDR_PARAM_NUM
    GET_NODE_ID_a0
    dli     t5, DDR_MC_CONFIG_BASE
    or      t5, t5, a0
1:
    ld      t6, 0x0(t5)
    move    a0, t5
    and     a0, a0, 0xfff
    bal     hexserial
    nop
    PRINTSTR(":  ")
    dsrl    a0, t6, 32
    bal     hexserial
    nop
    //PRINTSTR("  ")
    move    a0, t6
    bal     hexserial
    nop
    PRINTSTR("\r\n")

    daddiu  t1, t1, -1
    daddiu  t5, t5, 8
    bnez    t1, 1b
    nop
#endif
#if 0   //def  DEBUG_DDR_PARAM   //Change parameters of MC
    GET_NODE_ID_a0;
    dli     a1, DDR_MC_CONFIG_BASE
    or      t8, a0, a1

    PRINTSTR("\r\nChange some parameters of MC:");
1:
    PRINTSTR("\r\nPlease input the register number you want to change!!!(0xfff:jump out.): ");
    dli     t6, 0x00
    bal     inputaddress
    nop
    move    t5, v0
    
    dli     a1, 0x320
    bge     t5, a1, 2f    #if input address offset exceed range,jump out
    nop
    and     t5, t5, 0xff8
    daddu   t5, t5, t8

    PRINTSTR("\r\nPlease input the data-hex: ");
    dli     t6, 0x00
    bal     inputaddress
    nop
    sd      v0, 0x0(t5)    #v0 is the input value

    //print the new register value
    move    t6, t5
    PRINTSTR("\r\nRegister 0x")
    dsubu   t5, t5, t8
    move    a0, t5
    bal     hexserial
    nop
    PRINTSTR(": ")
    ld      t6, 0x0(t6)
    dsrl    a0, t6, 32
    bal     hexserial
    nop
    move    a0, t6
    bal     hexserial
    nop

    b        1b
    nop
2:    
#endif

    //TTYDBG("Disable register space of MEMORY\r\n")
    bal     disable_mc_conf_space
    nop
88:
#endif
#endif

#ifndef  DISABLE_DIMM_ECC
    //Init ECC according to DIMM ECC info
    GET_DIMM_ECC
    beqz    a1, 4f
    nop
    TTYDBG("ECC init start(maybe take 1 minute or so)....\r\n")

    //TTYDBG("Enable register space of MEMORY\r\n")
    bal     enable_mc_conf_space
    nop

    GET_NODE_ID_a0;
    dli     t8, DDR_MC_CONFIG_BASE
    or      t8, t8, a0

    //disable ECC interrupt
    ld      a2, ECC_INT_ENABLE_ADDR(t8)
    dli     a1, 0x3
    dsll    a1, a1, ECC_INT_ENABLE_OFFSET 
    not     a1, a2
    and     a2, a2, a1
    sd      a2, ECC_INT_ENABLE_ADDR(t8)

    //enable ECC function but without reporting error
    ld      a2, ECC_ENABLE_ADDR(t8)
    dli     a1, 0x7
    dsll    a1, a1, ECC_ENABLE_OFFSET
    not     a1, a1
    and     a2, a2, a1
    dli     a1, 0x1
    dsll    a1, a1, ECC_ENABLE_OFFSET
    or      a2, a2, a1
    sd      a2, ECC_ENABLE_ADDR(t8)

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
    sync
    nop
    nop
    nop
    nop
//init mem to all 0
    dli     t1, 0xb800001000000000
    GET_NODE_ID_a0
    or      t1, t1, a0
    GET_MC0_MEMSIZE
    dsll    a1, a1, 29   //a1*512M
    daddu   t5, t1, a1
//write memory
1:
    bgeu    t1, t5, 1f
    nop

    sd      $0, 0x0(t1)
    sd      $0, 0x8(t1)
    sd      $0, 0x10(t1)
    sd      $0, 0x18(t1)
    daddu   t1, t1, 0x20
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

    //TTYDBG("Enable register space of MEMORY\r\n")
    bal     enable_mc_conf_space
    nop

    //enable ECC function with reporting error
    ld      a2, ECC_ENABLE_ADDR(t8)
    dli     a1, 0x7
    dsll    a1, a1, ECC_ENABLE_OFFSET
    not     a1, a1
    and     a2, a2, a1
    dli     a1, 0x7
    dsll    a1, a1, ECC_ENABLE_OFFSET
    or      a2, a2, a1
    sd      a2, ECC_ENABLE_ADDR(t8)

    //enable ECC interrupt
    ld      a2, ECC_INT_ENABLE_ADDR(t8)
    dli     a1, 0x3
    dsll    a1, a1, ECC_INT_ENABLE_OFFSET 
    or      a2, a2, a1
    sd      a2, ECC_INT_ENABLE_ADDR(t8)

    //TTYDBG("Disable register space of MEMORY\r\n")
    bal     disable_mc_conf_space
    nop

    TTYDBG("MC ECC init done.\r\n")
4:
#endif

    move    ra, t7
    jr      ra
    nop
    .end    mc_init

//for 3B5~
LEAF(enable_mc_conf_space)
/*********************
pre-condition::
    t2: chip configure register address
*********************/
    lw      a2, 0x0(t2)
    li      v1, 0x1
    GET_NODE_ID_a1
    and     a1, a1, 0x1
    beqz    a1, 1f
    nop
    sll     v1, v1, 5
1:
    sll     v1, v1, DDR_CONFIG_DISABLE_OFFSET
    not     v1, v1
    and     a2, a2, v1
    sw      a2, 0x0(t2)
    sync

    jr      ra
    nop
END(enable_mc_conf_space)

LEAF(disable_mc_conf_space)
/*********************
pre-condition::
    t2: chip configure register address
*********************/
    lw      a2, 0x0(t2)
    li      v1, 0x1
    GET_NODE_ID_a1
    and     a1, a1, 0x1
    beqz    a1, 1f
    nop
    sll     v1, v1, 5
1:
    sll     v1, v1, DDR_CONFIG_DISABLE_OFFSET
    or      a2, a2, v1
    sw      a2, 0x0(t2)
    sync

    jr      ra
    nop
END(disable_mc_conf_space)

LEAF(enable_mc_read_buffer)

    jr      ra
    nop
END(enable_mc_read_buffer)

LEAF(disable_mc_read_buffer)

    jr      ra
    nop
END(disable_mc_read_buffer)

