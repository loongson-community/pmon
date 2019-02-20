/**********************************
    Author: chenxinke
    Date:   20150707
    mc_init for 3A8
    v1.0
    input:
    t7(option ARB_LEVEL)--do arb level, 0--not level; 1--do level;
    t3--MC select: 0--MC0; 1--MC1
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
                    0x0000000000000000, \
                    0xfffffffff0000000, \
                    0x00000000000000f0)
    beqz    t3, 1f
    nop
    GET_NODE_ID_a0;
    XBAR_CONFIG_NODE_a0(0x10, \
                    0x0000000000000000, \
                    0xfffffffff0000000, \
                    0x00000000000000f1)
1:

    sync
    nop
    nop
    nop
    nop

    TTYDBG("\r\nEnable register space of MEMORY\r\n")
    bal     enable_mc_conf_space
    nop

    GET_NODE_ID_a0;
    dli     t8, DDR_MC_CONFIG_BASE
    or      t8, t8, a0

    //set parameter start location
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
    b       3f
    nop
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
#if 1//def  MULTI_NODE_DDR_PARAM
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

#ifdef  PRINT_DDR_LEVELING   //print registers
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
    TTYDBG("\r\nDisable register space of MEMORY done.\r\n")

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
    no:p
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

    move    k0, t3  //store t3, because ddr3_leveling will demage t3
#ifdef DLL_DELAY_LOOP
    move    k1, s2
    dli     s2, 0x0
    sd      s2, 0x350(t8)
    sd      s2, 0x358(t8)

#ifdef DLL_CK_DELAY_DEBUG
    PRINTSTR("\r\nPlease input the min value of dllclkloop: ");
    bal     inputaddress
    nop
    move    s2, v0    #v0 is the input value

    PRINTSTR("\r\nPlease input the max value of dllclkloop: ");
    bal     inputaddress
    nop
    dli     t6, 0x0
    dsll    t6, v0, 32   #v0 is the input value
    or      k1, t6, k1
#else
    dli     t6, 0x7f00000000
    or      k1, t6, k1
#endif

loop_test_leveling:
    GET_DIMM_TYPE
    bnez    a1, 21f
    nop
    dla     a2, ddr3_reg_data
    beqz    k0, mc0
    nop
    dla     a2, ddr3_reg_data_mc1
    b       mc0
    nop
21:
    dla     a2, ddr3_RDIMM_reg_data
    beqz    k0, mc0
    nop
    dla     a2, ddr3_RDIMM_reg_data_mc1
mc0:
    ld      t2, 0x350(t8)
    ld      t9, 0x358(t8)

    bal     ddr2_config
    nop

    sd      t2, 0x350(t8)
    sd      t9, 0x358(t8)
   
    dli     t6, 0xfffffffffffffffe
    ld      t1, START_ADDR(t8)
    and     t1, t1, t6
    sd      t1, START_ADDR(t8)
    sync

    dli     t6, 0x0
    sb      t6, 0x162(t8)
    sync

    dli     t6, 0x0
    or      t6, t6, s2 
    dsll    t6, t6, 8
    or      t6, t6, s2 
    dsll    t6, t6, 8
    or      t6, t6, s2 
    dsll    t6, t6, 8
    or      t6, t6, s2 
    dsll    t6, t6, 32

    ld      t1, START_ADDR(t8)
    and     t1, t1, 0x00000000ffffffff
    or      t1, t1, t6
    sd      t1, START_ADDR(t8)
    sync

    dli     t6, 0x1
    ld      t1, START_ADDR(t8)
    or      t1, t1, t6
    sd      t1, START_ADDR(t8)
    sync

/* 1. wait until init done */
    dli     t1, 0x160
    or      t1, t1, t8
wait_dram_init_done1:
    ld      a0, 0x0(t1)
    dli     t4, 0x00000000ff000000
    and     a0, a0, t4
    beqz    a0, wait_dram_init_done1
    nop
#endif

    GET_NODE_ID_a0
    dli     t8, DDR_MC_CONFIG_BASE
    or      t8, t8, a0
    bal     ddr3_leveling
    nop

#ifdef  PRINT_DDR_LEVELING   //print registers
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

#ifdef DLL_DELAY_LOOP
    move    t3, k0 
/* test memory */
//    li      t0, 0xbfe10424
//    lb      a0, 0x1(t0)
//    and     a0, a0, 0xfd
//    sb      a0, 0x1(t0)
//    sync

    dmulou  t6, s2, 0x100
    GET_NODE_ID_a0
    or      t6, a0, t6
    dli     t0, 0x9000000000000000
    or      t0, t6, t0

    dli     a0, 0x5555555555555555
    sd      a0, 0x0(t0)
    dli     a0, 0xaaaaaaaaaaaaaaaa
    sd      a0, 0x8(t0)
    dli     a0, 0x3333333333333333
    sd      a0, 0x10(t0)
    dli     a0, 0xcccccccccccccccc
    sd      a0, 0x18(t0)
    dli     a0, 0x7777777777777777
    sd      a0, 0x20(t0)
    dli     a0, 0x8888888888888888
    sd      a0, 0x28(t0)
    dli     a0, 0x1111111111111111
    sd      a0, 0x30(t0)
    dli     a0, 0xeeeeeeeeeeeeeeee
    sd      a0, 0x38(t0)

    PRINTSTR("The uncache data is:\r\n")
    dli     t1, 0x8
    move    t5, t0
1:    
    ld      t6, 0x0(t5)
    move    a0, t5
    and     a0, a0, 0xffff
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
    beq     t1, 0x8, 2f
    nop
    beq     t1, 0x7, 3f
    nop
    beq     t1, 0x6, 4f
    nop
    beq     t1, 0x5, 5f
    nop
    beq     t1, 0x4, 6f
    nop
    beq     t1, 0x3, 7f
    nop
    beq     t1, 0x2, 8f
    nop
    beq     t1, 0x1, 9f
    nop

2: 
    beq     t6, 0x5555555555555555, testok
    nop
    b       testfail
    nop

3:
    beq     t6, 0xaaaaaaaaaaaaaaaa, testok
    nop
    b       testfail
    nop

4:
    beq     t6, 0x3333333333333333, testok
    nop
    b       testfail
    nop

5:
    beq     t6, 0xcccccccccccccccc, testok
    nop
    b       testfail
    nop

6:
    beq     t6, 0x7777777777777777, testok
    nop
    b       testfail
    nop

7:
    beq     t6, 0x8888888888888888, testok
    nop
    b       testfail
    nop

8:
    beq     t6, 0x1111111111111111, testok
    nop
    b       testfail
    nop

9:
    beq     t6, 0xeeeeeeeeeeeeeeee, testok
    nop

testfail:
    PRINTSTR("The uncached test failed\r\n")
//    li      t6, 0xbfe10424
//    lb      a0, 0x1(t6)
//    or      a0, a0, 0x02
//    sb      a0, 0x1(t6)
//    sync
    b       1f
    nop
    
testok:
    daddiu  t1, t1, -1
    daddiu  t5, t5, 8
    bnez    t1, 1b
    nop
//    li      t6, 0xbfe10424
//    lb      a0, 0x1(t6)
//    or      a0, a0, 0x02
//    sb      a0, 0x1(t6)
//    sync
    or      s2, s2, 0x100

1:
    and     t6, s2, 0xff    
    dli     t1, 0x350
    bgt     t6, 0x5f, 3f
    nop
    bgt     t6, 0x3f, 2f
    nop
    bgt     t6, 0x1f, 1f
    nop

    or      t1, t1, t8
    lw      a0, 0x0(t1)
    dsrl    t6, s2, 8
    and     s2, s2, 0xff
    dsll    t6, t6, s2
    b       4f
    nop

1:
    daddu   t1, t1, 0x4
    or      t1, t1, t8
    lw      a0, 0x0(t1)
    dsrl    t6, s2, 8
    and     s2, s2, 0xff
    dsubu   s2, s2, 0x20
    dsll    t6, t6, s2
    daddu   s2, s2, 0x20
    b       4f
    nop

2:
    daddu   t1, t1, 0x8
    or      t1, t1, t8
    lw      a0, 0x0(t1)
    dsrl    t6, s2, 8
    and     s2, s2, 0xff
    dsubu   s2, s2, 0x40
    dsll    t6, t6, s2
    daddu   s2, s2, 0x40
    b       4f
    nop

3:
    daddu   t1, t1, 0xc
    or      t1, t1, t8
    lw      a0, 0x0(t1)
    dsrl    t6, s2, 8
    and     s2, s2, 0xff
    dsubu   s2, s2, 0x60
    dsll    t6, t6, s2
    daddu   s2, s2, 0x60

4: 
    or      t6, a0, t6
    sw      t6, 0x0(t1)

    move    a0, s2
    bal     hexserial
    nop
    PRINTSTR("\r\n")

    daddiu  s2, s2, 0x1
    dli     t6, 0x0
    dsrl    t6, k1, 32    
    bleu    s2, t6, loop_test_leveling
    nop
    and     k1, k1, 0xffffffff
    
    PRINTSTR("\r\nmc_reg_vector_for_diff_dll_training\r\n")
    lw      a0, 0x35c(t8)
    bal     hexserial
    nop
    lw      a0, 0x358(t8)
    bal     hexserial
    nop
    lw      a0, 0x354(t8)
    bal     hexserial
    nop
    lw      a0, 0x350(t8)
    bal     hexserial
    nop
    PRINTSTR("\r\n")

    GET_DIMM_TYPE
    bnez    a1, 21f
    nop
    dla     a2, ddr3_reg_data
    beqz    k0, mc0_1
    nop
    dla     a2, ddr3_reg_data_mc1
    b       mc0_1
    nop
21:
    dla     a2, ddr3_RDIMM_reg_data
    beqz    k0, mc0_1
    nop
    dla     a2, ddr3_RDIMM_reg_data_mc1
mc0_1:
    ld      t2, 0x350(t8)
    ld      t9, 0x358(t8)

    bal     ddr2_config
    nop

    sd      t2, 0x350(t8)
    sd      t9, 0x358(t8)

    dli     t6, 0xfffffffffffffffe
    ld      t1, 0x18(t8)
    and     t1, t1, t6
    sd      t1, 0x18(t8)
    sync

calculate_best_successful_value:
//s2 reg struct explaination:
/*|15:8                                |7:0                           |
  |beginning value of biggest section  |currently scaning value       |
  |31:24                               |23:16                         |
  |temple beginning value to be compare|end value of biggest section  |
  |47:40                               |39:32                         |
  |end value of first section          |temple end value to be compare|*/

//    dli     a0, 0xfff0000ffff00000
//    sd      a0, 0x350(t8)      //for_test
//    dli     a0, 0x0fffffff00000000
//    sd      a0, 0x358(t8)

    dli     s2, 0x0
    dli     t6, 0x0
    dli     a0, 0x0
    dli     t1, 0x0

    ld      a0, 0x350(t8)
    and     a0, a0, 0x1
    beqz    a0, scan1
    nop
1:
    and     t6, s2, 0xff
    bgeu    t6, 63, 2f
    nop
    daddu   t6, t6, 0x1
    and     s2, s2, 0xffffffffffffff00
    or      s2, t6, s2
    ld      a0, 0x350(t8)
    dsrl    a0, a0, t6
    and     a0, a0, 0x1
    bnez    a0, 1b
    nop
    b       record_first_section
    nop
2:
    bgeu    t6, 127, calculate
    nop
    ld      a0, 0x358(t8)
    daddu   t6, t6, 0x1
    dsubu   t6, t6, 64
    dsrl    a0, a0, t6
    and     a0, a0, 0x1
    daddu   t6, t6, 64
    and     s2, s2, 0xffffffffffffff00
    or      s2, t6, s2
    bnez    a0, 2b
    nop
   
record_first_section:
    dsubu   t6, t6, 0x1
    dsll    t6, t6, 40
    or      s2, s2, t6
     
scan1:
    and     t6, s2, 0xff
    bgeu    t6, 63, 1f
    nop
    ld      a0, 0x350(t8)
    daddu   t6, t6, 0x1
    and     s2, s2, 0xffffffffffffff00
    or      s2, t6, s2
    dsrl    a0, a0, t6
    and     a0, a0, 0x1
    beqz    a0, scan1
    nop
    b       record_start1
    nop
        
1:
    bgeu    t6, 127, calculate
    nop
    ld      a0, 0x358(t8)
    daddu   t6, t6, 0x1
    and     s2, s2, 0xffffffffffffff00
    or      s2, t6, s2
    dsubu   t6, t6, 64
    dsrl    a0, a0, t6
    daddu   t6, t6, 64
    and     a0, a0, 0x1
    beqz    a0, scan1
    nop
    b       record_start1
    nop

record_start1:
    dsll    t6, t6, 24
    and     s2, s2, 0xffffffff00ffffff
    or      s2, t6, s2

scan0:
    and     t6, s2, 0xff
    bgeu    t6, 63, 1f
    nop
    ld      a0, 0x350(t8)
    daddu   t6, t6, 0x1
    and     s2, s2, 0xffffffffffffff00
    or      s2, t6, s2
    dsrl    a0, a0, t6
    and     a0, a0, 0x1
    bnez    a0, scan0
    nop
    b       record_end1
    nop
1:
    bgeu    t6, 127, calculate
    nop
    ld      a0, 0x358(t8)
    daddu   t6, t6, 0x1
    and     s2, s2, 0xffffffffffffff00
    or      s2, t6, s2
    dsubu   t6, t6, 64
    dsrl    a0, a0, t6
    daddu   t6, t6, 64
    and     a0, a0, 0x1
    bnez    a0, scan0
    nop

record_end1:
    dsubu   t6, t6, 0x1
    dsll    t6, t6, 32
    and     s2, s2, 0xffffff00ffffffff
    or      s2, t6, s2
    and     a0, s2, 0xff00
    dsrl    a0, a0, 8
    and     t6, s2, 0xff0000
    dsrl    t6, t6, 16
    dsubu   t6, t6, a0
    and     a0, s2, 0xff000000
    dsrl    a0, a0, 24
    and     t1, s2, 0xff00000000
    dsrl    t1, t1, 32
    dsubu   t1, t1, a0
    bge     t1, t6, exchange 
    nop
    b       scan1
    nop     
    
exchange:
    dsrl    t6, s2, 16
    and     t6, t6, 0xffff00
    and     s2, s2, 0xffffffffff0000ff
    or      s2, t6, s2
    b       scan1
    nop

calculate:
    ld      a0, 0x358(t8)
    and     a0, a0, 0x8000000000000000
    beqz    a0, 1f
    nop
    and     a0, s2, 0xff000000
    dsrl    a0, a0, 24
    dli     t6, 0x80
    dsubu   a0, t6, a0
    and     t6, s2, 0xff0000000000
    dsrl    t6, t6, 40
    daddu   t6, a0, t6
    and     a0, s2, 0xff00
    dsrl    a0, a0, 8
    and     t1, s2, 0xff0000
    dsrl    t1, t1, 16
    dsubu   t1, t1, a0
    bgeu    t1, t6, 2f
    nop
    dsrl    t6, t6, 1
    and     a0, s2, 0xff000000
    dsrl    a0, a0, 24
    dli     t1, 0x7f
    dsubu   t1, t1, a0
    and     a0, s2, 0xff0000000000
    dsrl    a0, a0, 40
    bgeu    t1, a0, 3f
    nop
    dsubu   s2, a0, t6
    b       finish
    nop
3:  
    and     a0, s2, 0xff000000
    dsrl    a0, a0, 24
    daddu   s2, a0, t6
    b       finish
    nop
2:  
    dsrl    t1, t1, 1
    daddu   s2, a0, t1
    b       finish
    nop
1:
    and     a0, s2, 0xff00
    dsrl    a0, a0, 8
    and     t1, s2, 0xff0000
    dsrl    t1, t1, 16
    dsubu   t1, t1, a0
    and     t6, s2, 0xff0000000000
    dsrl    t6, t6, 40
    bgeu    t1, t6, first_short
    nop
    dsrl    t6, t6, 1
    move    s2, t6
    b       finish
    nop 

first_short:
    dsrl    t1, t1, 1
    daddu   s2, a0, t1

finish:
    dli     t6, 0x0
    or      t6, t6, s2 
    dsll    t6, t6, 8
    or      t6, t6, s2 
    dsll    t6, t6, 8
    or      t6, t6, s2 
    dsll    t6, t6, 8
    or      t6, t6, s2 
    dsll    t6, t6, 32

    ld      t1, START_ADDR(t8)
    and     t1, t1, 0x00000000ffffffff
    or      t1, t1, t6
    sd      t1, START_ADDR(t8)
    sync

    move    a0, t1
    dsrl    a0, a0, 32
    bal     hexserial
    nop
    move    a0, t1
    bal     hexserial
    nop

    PRINTSTR("\r\nPlease write the dll_clk param to corresponding place in loongson_mc2_param.S ,the offset is 0x018!!!\r\n")
    PRINTSTR("\r\nPress enter to continue: ");
    bal     inputaddress
    nop
/*init start*/
    dli     t6, 0x1
    ld      t1, START_ADDR(t8)
    or      t1, t1, t6
    sd      t1, START_ADDR(t8)
    sync

/* 1. wait until init done */
    dli     t1, 0x160
    or      t1, t1, t8
wait_dram_init_done2:
    ld      a0, 0x0(t1)
    dli     t4, 0x00000000ff000000
    and     a0, a0, t4
    beqz    a0, wait_dram_init_done2
    nop

    GET_NODE_ID_a0
    dli     t8, DDR_MC_CONFIG_BASE
    or      t8, t8, a0
    bal     ddr3_leveling
    nop

    move    s2, k1
#endif

    move    t3, k0

    //Re-set t0&t2 because mc_init(ddr3_leveling) will change t0~t9
    GET_NODE_ID_a0
    dli     t2, 0x900000001fe00180
    dli     t0, 0x900000003ff00000
    or      t2, t2, a0
    or      t0, t0, a0

#if 1 //def  PRINT_DDR_LEVELING   //print registers
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

#if 0 //temp add for 3a3000-2h 8G RDIMM, use 32bit channel
    GET_NODE_ID_a0;
    dli     a1, DDR_MC_CONFIG_BASE
    or      t8, a0, a1

    dli     a1, 0x000801e4ff050101
    sd      a1, 0x1f0(t8)
    dli     a1, 0x0008010e01010006
    sd      a1, 0x210(t8)
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
    dli     t1, 0xb800001000000000
    GET_NODE_ID_a0
    or      t1, t1, a0
    GET_MC0_MEMSIZE
    beqz    t3, 1f
    nop
    GET_MC1_MEMSIZE
1:
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
    sd      $0, 0x20(t1)
    sd      $0, 0x28(t1)
    sd      $0, 0x30(t1)
    sd      $0, 0x38(t1)
    daddu   t1, t1, 0x40
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

    //PRINTSTR("\r\nDisable register space of MEMORY\r\n")
    bal     disable_mc_conf_space
    nop

    TTYDBG("MC ECC init done.\r\n")
4:
#endif

    move    ra, t7
    jr      ra
    nop
    .end    mc_init

//for 3A8
LEAF(enable_mc_conf_space)
/*********************
pre-condition::
    t2: chip configure register address
    t3: MC select
*********************/
    lw      a2, 0x0(t2)
    li      v1, 0x1
    beqz    t3, 1f
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
    t3: MC select
*********************/
    lw      a2, 0x0(t2)
    li      v1, 0x1
    beqz    t3, 1f
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

LEAF(disable_cpu_buffer_read)

    jr      ra
    nop
END(disable_cpu_buffer_read)

LEAF(get_mem_clk)
/*********************
    v0: current ddr freq setting
*********************/
    li t0,0xbfe00190
    ld t1,0x0(t0)
    dsrl t2,t1,37
    andi t2,t2,0x0000001f
    move v0,t2

    nop
    jr ra
    nop
END(get_mem_clk)
