/**********************************
    loongson3_ddr2_config.S
        used to set up ddr controllers MC0 and MC1
        and set up the memory space on L2 Xbar
    input: s1--MC1 & MC0 DIMM info and Node ID
    note: s1 is damaged by the end of this file
    original: whd
    rewrite by cxk on 11/11/2010
    1: reorder the program
    2: DIMM info and memory size is set according to s1[MC1&0_MEMSIZE]
    note: config L2 Xbar still need to be finished,currently only support limited MEMSIZE.
    v1.0    raw
    v1.2    add support for 4G memsize per MC, modify the L2-Xbar config manner of MC1
            to reduce code size.
    v1.4    Modify L2 Xbar config reg code at Interleave mode to reduce code size
            new code:
            1. according to Memsize config open space
            2. config interleave bits
************************************/

#######################################################
/**************************
0. s1 reset code
**************************/
//set use which MC: 01-MC0; 10-MC1; 00-MC0&MC1
#ifdef DEBUG_DDR_PARAM
    PRINTSTR("Please input DDR SELLECT :\r\n[ 8]:MC1_ONLY\r\n[ 4]:MC0_ONLY\r\n");
    dli     t6, 0x00
    bal     inputaddress    #input value stored in v0
    nop
    dli     t6, 0xc
    and     v0, v0, t6
    dli     t6, 0xc
    not     t6, t6
    and     s1, s1, t6
    or      s1, s1, v0
#endif

#ifdef  AUTO_ARB_LEVEL
//store memory system info into s3(include frequency and dimm i2c addr)
#ifdef  CHECK_ARB_LEVEL_FREQ
    dli     t0, CHIP_SAMPLE_BASE_ADDR
    GET_NODE_ID_a0
    or      t0, t0, a0
    ld      a0, 0x0(t0)
    dsrl    a0, a0, DDR_CLKSEL_OFFSET
    and     a0, a0, DDR_CLKSEL_MASK
    dsll    a0, a0, 40
    GET_NODE_ID_a1
    dsll    a1, a1, 3
    dsll    a0, a0, a1
    or      s3, s3, a0
#endif

#ifdef  CHECK_ARB_LEVEL_DIMM
    //store dimm i2c addr info to s3
    dsrl    a2, s1, 16
    dli     a1, 0xffff
    and     a2, a2, a1
    GET_NODE_ID_a1
    dsll    a1, a1, 4
    dsll    a2, a2, a1
    or      s3, s3, a2
#endif

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
#endif

#ifdef  AUTO_DDR_CONFIG
    bal     PROBE_NODE_DIMM
    nop
#endif

#ifdef  PRINT_MSG
    /* show value of s1 */
    PRINTSTR("\r\n\r\ns1 = 0x");
    dsrl    a0, s1, 32
    bal     hexserial
    nop
    PRINTSTR("__")
    move    a0, s1
    bal     hexserial
    nop
    PRINTSTR("\r\n")
#endif

#ifdef  AUTO_DDR_CONFIG
//for DDR3 RDIMM, if it has 2 rank, use only 1. temp code for 3A MC
    //check MC1 first
    dsrl    t1, s1, 32
    dli     a1, 0xd0000000
    and     a1, t1, a1
    dli     a0, 0xd0000000
    bne     a1, a0, 1f      //not DDR3 RDIMM, do nothing
    nop
    dli     t2, 0xff00
    and     t2, t1, t2      //t2 store memsize

    dli     a1, 0x30000     //check slot 0 CS_MAP
    and     a1, t1, a1
    dli     a0, 0x30000
    bne     a1, a0, 2f      //not 2 rank
    nop
    //slot 0 has 2 rank DIMM
    dli     a1, 0xfffdffff
    and     t1, t1, a1      //clear cs 1
    dsrl    t2, t2, 1       //half memsize
2:
    dli     a1, 0xc0000     //check slot 1 CS_MAP
    and     a1, t1, a1
    dli     a0, 0xc0000
    bne     a1, a0, 2f      //not 2 rank
    nop
    //slot 1 has 2 rank DIMM
    dli     a1, 0xfff7ffff
    and     t1, t1, a1      //clear cs 3
    dsrl    t2, t2, 1       //half memsize
2:
    //use reduced(if needed) memsize
    dli     a1, 0xffff00ff
    and     t1, t1, a1
    or      t1, t1, t2
    //use modified infor for MC1
    dli     a1, 0xffffffff
    and     s1, s1, a1
    dsll    t1, t1, 32
    or      s1, s1, t1
1:
    //check MC0
    dli     a1, 0xffffffff
    and     t1, s1, a1
    dli     a1, 0xd0000000
    and     a1, t1, a1
    dli     a0, 0xd0000000
    bne     a1, a0, 1f      //not DDR3 RDIMM, do nothing
    nop
    dli     t2, 0xff00
    and     t2, t1, t2      //t2 store memsize

    dli     a1, 0x30000     //check slot 0 CS_MAP
    and     a1, t1, a1
    dli     a0, 0x30000
    bne     a1, a0, 2f      //not 2 rank
    nop
    //slot 0 has 2 rank DIMM
    dli     a1, 0xfffdffff
    and     t1, t1, a1      //clear cs 1
    dsrl    t2, t2, 1       //half memsize
2:
    dli     a1, 0xc0000     //check slot 1 CS_MAP
    and     a1, t1, a1
    dli     a0, 0xc0000
    bne     a1, a0, 2f      //not 2 rank
    nop
    //slot 1 has 2 rank DIMM
    dli     a1, 0xfff7ffff
    and     t1, t1, a1      //clear cs 3
    dsrl    t2, t2, 1       //half memsize
2:
    //use reduced(if needed) memsize
    dli     a1, 0xffff00ff
    and     t1, t1, a1
    or      t1, t1, t2
    //use modified infor for MC0
    dli     a1, 0xffffffff00000000
    and     s1, s1, a1
    or      s1, s1, t1
#if 1
    /* show value of s1 */
    PRINTSTR("\r\nnew s1 = 0x");
    dsrl    a0, s1, 32
    bal     hexserial
    nop
    PRINTSTR("__")
    move    a0, s1
    bal     hexserial
    nop
    PRINTSTR("\r\n")
#endif
1:
#endif

#ifdef DEBUG_DDR_PARAM
    PRINTSTR("\r\nChange s1?:\r\n");
    dli     t6, 0x00
    bal     inputaddress    #input value stored in v0
    nop
    beqz    v0, 1f
    nop
    move    s1, v0
1:
#endif

//make sure s1[3:2] is correctly set.
    GET_MC_SEL_BITS
    dli     a2, 3
    bne     a1, a2, 1f
    nop
//s1[3:2]=0b'11, clear to 0b'00
    dli     a2, 0xc
    not     a2, a2
    and     s1, s1, a2
1:
/**************************
1. 1. check NODE memory size.
*  2. set MC0/1_ONLY if the following 2 conditions are satisfied:
*     (1). s1[3:2]=0b'00
*     (2). MC0 or MC1 MEMSIZE > 0.
* when use AUTO_DDR_CONFIG, one MC may have no DIMM while the other has, in this case,
* the code should set MC0_ONLY or MC1_ONLY in s1 automatically, because the code of 
* configuring L2-Xbar will use this message.
**************************/
    GET_MC0_ONLY
    bnez    a1, 1f
    nop
    GET_MC1_ONLY
    bnez    a1, 2f
    nop
//s1[3:2]=0b'00
    //check memory size in this case
    GET_MC0_MEMSIZE
    move    t5, a1
    GET_MC1_MEMSIZE
    daddu   a1, a1, t5
    beqz    a1, 70f
    nop
    dli     t5, 0x10
    bgt     a1, t5, 70f
    nop
    GET_MC0_MEMSIZE
    bnez    a1, 3f
    nop
    //MC0_MEMSIZE=0, MC1_MEMSIZE must !=0, set MC1_ONLY
    dli     t5, 0x8
    or      s1, t5
    b       4f
    nop
3:  //MC0_MEMSIZE!=0
    GET_MC1_MEMSIZE
    bnez    a1, 4f
    nop
    //MC1_MEMSIZE=0 set use MC0_ONLY
    dli     t5, 0x4
    or      s1, t5
    b       4f
    nop
1:  //MC0_ONLY
    GET_MC0_MEMSIZE
    b       5f
    nop
2:  //MC1_ONLY
    GET_MC1_MEMSIZE
5:
    beqz    a1, 70f
    nop
    dli     t5, 0x8
    bgt     a1, t5, 70f
    nop
4:
#if 0
    PRINTSTR("\r\ns1 = 0x")
    move    a0, s1
    bal     hexserial
    nop
    PRINTSTR("\r\n")
#endif
/************************
2. set up Memory Controller.
************************/
/***********************
for single chip or multi-chip:
t0: X-bar config base
t2: chip configuration register location
t0,t2 shouldn't be changed to the end of this file.
**********************/
    GET_NODE_ID_a0
    dli     t2, 0x900000001fe00180
    dli     t0, 0x900000003ff00000
    or      t2, t2, a0
    or      t0, t0, a0

#if 1 // AdonWang disable cpu buffered read
/* !!!!!!!!!!!!! IMPORTANT !!!!!!!!!!!! */
    PRINTSTR("Disable cpu buffered read\r\n")
    lw      a1, 0x0(t2)
    li      a0, 0xfffffdff
    and     a1, a1, a0
    sw      a1, 0x0(t2)
#endif

#if 1 // AdonWang disable ddr3 readbuff
/*      May Affect the Performance     */
//This seems better for the spec2000
//if we enable ECC, this bit will be reset
    PRINTSTR("Disable read buffer\r\n")
    lw      a1, 0x4(t2)
    li      a0, 0x18
    or      a1, a1, a0
    sw      a1, 0x4(t2)
#endif

//init MC1 will damage MC0 s1 info
//config MC0 if not define MC1_ONLY
//-------------------------------------
10:
    GET_MC1_ONLY
    bnez    a1, 11f
    nop

    dli     t3, 0x0
#ifndef  DISABLE_DIMM_ECC
    //Enable MC read buffer for ECC Initial
    GET_DIMM_ECC
    beqz    a1, 8f
    nop
    TTYDBG("Enable MC read buffer\r\n")
    lw      a1, 0x4(t2)
    li      a0, 0x8
    sll     a0, a0, t3
    not     a0, a0
    and     a1, a1, a0
    sw      a1, 0x4(t2)
    sync
8:
#endif

#ifdef  AUTO_ARB_LEVEL
    bal     SET_AUTO_ARB_LEVEL_MARK
    nop
#endif

    bal     mc_init
    nop
    PRINTSTR("\r\nMC0 Config DONE\r\n")
//-------------------------------------
//config MC1 if not define MC0_ONLY
11:
    GET_MC0_ONLY
    bnez    a1, 12f
    nop

    dli     t3, 0x1
#ifndef  DISABLE_DIMM_ECC
    //Enable MC read buffer for ECC Initial
    GET_DIMM_ECC
    beqz    a1, 8f
    nop
    TTYDBG("Enable MC read buffer\r\n")
    lw      a1, 0x4(t2)
    li      a0, 0x8
    sll     a0, a0, t3
    not     a0, a0
    and     a1, a1, a0
    sw      a1, 0x4(t2)
    sync
8:
#endif

#ifdef  AUTO_ARB_LEVEL
    bal     SET_AUTO_ARB_LEVEL_MARK
    nop
#endif

    bal     mc_init
    nop
    TTYDBG("\r\nMC1 Config DONE\r\n")
//-------------------------------------
12:
/*****************************
 3. set msize for this NODE
******************************/
    GET_MC0_ONLY
    beqz    a1, 1f
    nop
//use MC0 only
    GET_MC0_MEMSIZE
    move    t5, a1
    b       2f
    nop
1:
    GET_MC1_ONLY
    beqz    a1, 1f
    nop
//use MC1 only
    GET_MC1_MEMSIZE
    move    t5, a1
    b       2f
    nop
1:
//use MC0&MC1
    GET_MC0_MEMSIZE
    move    t5, a1
    GET_MC1_MEMSIZE
    daddu   t5, t5, a1
2:
    GET_NODE_ID_a0;
    dsrl    a0, a0, 44  //because of the macro define
    dsll    a0, a0, 3   //a0=a0*8
    dsll    t5, t5, a0
    or      msize, msize, t5

#ifdef  PRINT_MSG
    PRINTSTR("\r\nmsize = 0x")
    move    a0, msize
    bal     hexserial
    nop
    PRINTSTR("\r\n")
#endif
/*******************************
 4. config L2 X-bar
    code procedure: first, MC*_ONLY bits in s1 decides whether this MC is
    used,then according to MC*_MEMSIZE bits in s1 decide memory size and how
    the L2 X-bar windows will be configured.
    note: currently,when use only 1 MC,support memory size: 512M, 1G, 2G, 3G, 4G;
      when use MC0&MC1 both, only support 1G, 2G or 4G Memory size of each Controller.
*******************************/
    sync
    nop
    nop
    nop
    nop
#ifndef NO_INTERLEAVE
    //clear L2 window(introduced by ARB_LEVEL or ECC initialize)
    L2XBAR_DISABLE_WINDOW(0x20);
#endif
    //disable default pci window
    L2XBAR_DISABLE_WINDOW(0x100);

    GET_MC_SEL_BITS
    beqz    a1, 1f
    nop
/*Assume MC0_ONLY */
    GET_NODE_ID_a0;
    XBAR_CONFIG_NODE_a0(0x10, \
                    0x0000000000000000, \
                    0xFFFFFFFFF0000000, \
                    0x00000000000000F0)
    PRINTSTR("MC0 space open : 0x00000000 - 0x0FFFFFFF\r\n")
    GET_MC1_ONLY
    beqz    a1, 2f
    nop
/* MC1_ONLY */
    L2XBAR_RECONFIG_TO_MC1(0x10)
    PRINTSTR("The opened MC0 Window is reassigned to MC1\r\n")
    b       2f
    nop
1:
#ifndef NO_INTERLEAVE
//interleave first, if MC1 memsize != MC0 memsize, do NO_interleave
    GET_MC0_MEMSIZE
    move    t5, a1
    GET_MC1_MEMSIZE
    bne     t5, a1, 1f
    nop
#ifdef INTERLEAVE_27
    PRINTSTR("DDR Interleave using Bit 27\r\n")
    GET_NODE_ID_a0;
    XBAR_CONFIG_NODE_a0(0x10, \
                    0x0000000000000000, \
                    0xFFFFFFFFF8000000, \
                    0x00000000000000F0)
    XBAR_CONFIG_NODE_a0(0x18, \
                    0x0000000008000000, \
                    0xFFFFFFFFF8000000, \
                    0x00000000000000F1)
#else
#ifdef INTERLEAVE_13
    PRINTSTR("DDR Interleave using Bit 13\r\n")
    GET_NODE_ID_a0;
    XBAR_CONFIG_NODE_a0(0x10, \
                    0x0000000000000000, \
                    0xFFFFFFFFF0002000, \
                    0x00000000000000F0)
    XBAR_CONFIG_NODE_a0(0x18, \
                    0x0000000000002000, \
                    0xFFFFFFFFF0002000, \
                    0x00000000000000F1)
#else
#ifdef INTERLEAVE_12
    PRINTSTR("DDR Interleave using Bit 12\r\n")
    GET_NODE_ID_a0;
    XBAR_CONFIG_NODE_a0(0x10, \
                    0x0000000000000000, \
                    0xFFFFFFFFF0001000, \
                    0x00000000000000F0)
    XBAR_CONFIG_NODE_a0(0x18, \
                    0x0000000000001000, \
                    0xFFFFFFFFF0001000, \
                    0x00000000000000F1)
#else
#ifdef INTERLEAVE_11
    PRINTSTR("DDR Interleave using Bit 11\r\n")
    GET_NODE_ID_a0;
    XBAR_CONFIG_NODE_a0(0x10, \
                    0x0000000000000000, \
                    0xFFFFFFFFF0000800, \
                    0x00000000000000F0)
    XBAR_CONFIG_NODE_a0(0x18, \
                    0x0000000000000800, \
                    0xFFFFFFFFF0000800, \
                    0x00000000000000F1)
#else   //default INTERLEAVE_10
    PRINTSTR("DDR Interleave using Bit 10\r\n")
    GET_NODE_ID_a0;
    XBAR_CONFIG_NODE_a0(0x10, \
                    0x0000000000000000, \
                    0xFFFFFFFFF0000400, \
                    0x00000000000000F0)
    XBAR_CONFIG_NODE_a0(0x18, \
                    0x0000000000000400, \
                    0xFFFFFFFFF0000400, \
                    0x00000000000000F1)

#endif
#endif
#endif
#endif
    PRINTSTR("DDR Interleave space open : 0x00000000 - 0x0FFFFFFF\r\n")
    b       2f
    nop
#endif
1:
    PRINTSTR("!!!MEM is at NO_INTERLEAVE mode. If this is not the expected setting, \r\nplease check whether the two MC_MEMSIZE is equal\r\n")
    GET_NODE_ID_a0;
    XBAR_CONFIG_NODE_a0(0x10, \
                    0x0000000000000000, \
                    0xFFFFFFFFF0000000, \
                    0x00000000000000F0)
    PRINTSTR("DDR space open : 0x00000000 - 0x0FFFFFFF\r\n")

2:
    //Config PCI windows
    L2XBAR_CONFIG_PCI_AS_CPU(0x10);
    L2XBAR_CONFIG_PCI_AS_CPU(0x18);
    L2XBAR_CONFIG_PCI_BASE_0to8(0x110);
    L2XBAR_CONFIG_PCI_BASE_0to8(0x118);
    PRINTSTR("PCI space open: 0x80000000 - 0x8FFFFFFF\r\n")

//config high memory windows
    GET_MC_SEL_BITS
    beqz    a1, 4f
    nop
/* Assume MC0_ONLY */
    GET_MC0_MEMSIZE
    move    t5, a1
    GET_MC1_ONLY
    beqz    a1, 1f
    nop
    //MC1_ONLY
    GET_MC1_MEMSIZE    
    move    t5, a1
1:     
//use MC only, currently only support 512M, 1G, 2G, 3G, 4G
    move    a1, t5
    dli     t5, 0x1
    beq     a1, t5, 10f
    nop
    dli     t5, 0x2
    beq     a1, t5, 20f
    nop
    dli     t5, 0x4
    beq     a1, t5, 40f
    nop
    dli     t5, 0x6
    beq     a1, t5, 60f
    nop
    dli     t5, 0x8
    //temp code, MEM size >= 4G, use 4G only
    bgeu    a1, t5, 80f
    nop
    b       70f  //error condition
    nop
10:     //ddr_512MB_MC0:
    GET_NODE_ID_a0;
    XBAR_CONFIG_NODE_a0(0x20, \
                    0x0000000100000000, \
                    0xFFFFFFFFe0000000, \
                    0x00000000000000F0)
    PRINTSTR("MC0 pace open : 0x100000000 - 0x11FFFFFFF\r\n")
    b       6f
    nop
20:     //ddr_1GB_MC:
    GET_NODE_ID_a0;
    XBAR_CONFIG_NODE_a0(0x20, \
                    0x0000000100000000, \
                    0xFFFFFFFFC0000000, \
                    0x00000000000000F0)
    PRINTSTR("MC0 space open : 0x100000000 - 0x13FFFFFFF\r\n")
    b       6f
    nop
40:     //ddr_2GB_MC0:
    GET_NODE_ID_a0;
    XBAR_CONFIG_NODE_a0(0x20, \
                    0x0000000100000000, \
                    0xFFFFFFFF80000000, \
                    0x00000000000000F0)
    PRINTSTR("MC0 space open : 0x100000000 - 0x17FFFFFFF\r\n")
    b       6f
    nop
60:     //ddr_3GB_MC0:
    GET_NODE_ID_a0;
    XBAR_CONFIG_NODE_a0(0x20, \
                    0x0000000100000000, \
                    0xFFFFFFFF80000000, \
                    0x00000000000000F0)
    GET_NODE_ID_a0;
    XBAR_CONFIG_NODE_a0(0x28, \
                    0x0000000180000000, \
                    0xFFFFFFFFc0000000, \
                    0x00000000000000F0)
    PRINTSTR("MC0 space open : 0x100000000 - 0x1bFFFFFFF\r\n")
    b       6f
    nop
80:     //ddr_4GB_MC0:
    GET_NODE_ID_a0;
    XBAR_CONFIG_NODE_a0(0x20, \
                    0x0000000100000000, \
                    0xFFFFFFFF00000000, \
                    0x00000000000000F0)
    PRINTSTR("MC0 space open : 0x100000000 - 0x1FFFFFFFF\r\n")
    b       6f
    nop
6:
    GET_MC1_ONLY
    beqz    a1, 2f
    nop
/* MC1_ONLY */
    L2XBAR_RECONFIG_TO_MC1(0x20)
    L2XBAR_RECONFIG_TO_MC1(0x28)
    PRINTSTR("The opened MC0 Window is reassigned to MC1\r\n")
    b       2f
    nop
4:  
//use MC0&MC1
#ifndef NO_INTERLEAVE
//interleave first, if MC1 memsize != MC0 memsize, do NO_interleave
    GET_MC0_MEMSIZE
    move    t5, a1
    GET_MC1_MEMSIZE
    bne     t5, a1, 3f
    nop

/* ONLY 1GB, 2GB and 4G has INTERLEAVE mode now */
#if 0 // by xqch comment to support ls3a2h swiotlb
    //universal set
    GET_NODE_ID_a0;
    //config mask  
    dli     a2, 0xffffffffffffffff
    dsll    a2, a2, 28
1:
    dsll    a2, a2, 1
    dsrl    a1, a1, 1
    bnez    a1, 1b
    nop
    sd      a2, 0x60(t0)
    sd      a2, 0x68(t0)
    sd      a2, 0x70(t0)
    sd      a2, 0x78(t0)
    //config base
    GET_MC0_MEMSIZE
    dsll    a2, a1, 30  //a2=a1*512M*2
    or      a2, a2, a0
    sd      a2, 0x20(t0)
    sd      a2, 0x28(t0)
    dsll    a1, a1, 29  //a1=a1*512M
    or      a2, a2, a1  //a2=a1+a2
    or      a2, a2, a0
    sd      a2, 0x30(t0)
    sd      a2, 0x38(t0)
    PRINTSTR("DDR all space open\r\n")
#else
    dli     t5, 0x2
    beq     a1, t5, 20f
    nop
    dli     t5, 0x4
    beq     a1, t5, 40f
    nop
    dli     t5, 0x8
    //temp code. >= 4G, use 4G only
    bgeu    a1, t5, 80f
    nop
    b       70f
    nop
20:
    GET_NODE_ID_a0;
    XBAR_CONFIG_NODE_a0(0x20, \
                0x0000000100000000, \
                0xFFFFFFFFC0000000, \
                0x0000000000000000)
    XBAR_CONFIG_NODE_a0(0x28, \
                0x0000000100000000, \
                0xFFFFFFFFC0000000, \
                0x0000000000000000)
    XBAR_CONFIG_NODE_a0(0x30, \
                0x0000000140000000, \
                0xFFFFFFFFC0000000, \
                0x0000000000000000)
    XBAR_CONFIG_NODE_a0(0x38, \
                0x0000000140000000, \
                0xFFFFFFFFC0000000, \
                0x0000000000000000)
    PRINTSTR("DDR space open : 0x100000000 - 0x17FFFFFFF\r\n")
    b       4f
    nop
40:     //2G in each MC
    GET_NODE_ID_a0;
    XBAR_CONFIG_NODE_a0(0x20, \
                0x0000000100000000, \
                0xFFFFFFFF80000000, \
                0x0000000000000000)
    XBAR_CONFIG_NODE_a0(0x28, \
                0x0000000100000000, \
                0xFFFFFFFF80000000, \
                0x0000000000000000)
    XBAR_CONFIG_NODE_a0(0x30, \
                0x0000000180000000, \
                0xFFFFFFFF80000000, \
                0x0000000000000000)
    XBAR_CONFIG_NODE_a0(0x38, \
                0x0000000180000000, \
                0xFFFFFFFF80000000, \
                0x0000000000000000)
    PRINTSTR("DDR space open : 0x100000000 - 0x1FFFFFFFF\r\n")
    b       4f
    nop
80:     //4G in each MC
    GET_NODE_ID_a0;
    XBAR_CONFIG_NODE_a0(0x20, \
                0x0000000100000000, \
                0xFFFFFFFF00000000, \
                0x0000000000000000)
    XBAR_CONFIG_NODE_a0(0x28, \
                0x0000000100000000, \
                0xFFFFFFFF00000000, \
                0x0000000000000000)
    XBAR_CONFIG_NODE_a0(0x30, \
                0x0000000200000000, \
                0xFFFFFFFF00000000, \
                0x0000000000000000)
    XBAR_CONFIG_NODE_a0(0x38, \
                0x0000000200000000, \
                0xFFFFFFFF00000000, \
                0x0000000000000000)
    PRINTSTR("DDR space open : 0x100000000 - 0x2FFFFFFFF\r\n")
    b       4f
    nop
4:
#endif
//set interleave mode
#ifdef INTERLEAVE_27
    L2XBAR_CONFIG_INTERLEAVE(0x20, \
                0x0000000000000000, \
                0x0000000008000000, \
                0x00000000000000F0)
    L2XBAR_CONFIG_INTERLEAVE(0x28, \
                0x0000000008000000, \
                0x0000000008000000, \
                0x00000000000000F1)
    L2XBAR_CONFIG_INTERLEAVE(0x30, \
                0x0000000000000000, \
                0x0000000008000000, \
                0x00000000080000F0)
    L2XBAR_CONFIG_INTERLEAVE(0x38, \
                0x0000000008000000, \
                0x0000000008000000, \
                0x00000000080000F1)
    PRINTSTR("DDR Interleave using Bit 27\r\n")
#else
#ifdef INTERLEAVE_13
    L2XBAR_CONFIG_INTERLEAVE(0x20, \
                0x0000000000000000, \
                0x0000000000002000, \
                0x00000000000000F0)
    L2XBAR_CONFIG_INTERLEAVE(0x28, \
                0x0000000000002000, \
                0x0000000000002000, \
                0x00000000000000F1)
    L2XBAR_CONFIG_INTERLEAVE(0x30, \
                0x0000000000000000, \
                0x0000000000002000, \
                0x00000000000020F0)
    L2XBAR_CONFIG_INTERLEAVE(0x38, \
                0x0000000000002000, \
                0x0000000000002000, \
                0x00000000000020F1)
    PRINTSTR("DDR Interleave using Bit 13\r\n")
#else
#ifdef INTERLEAVE_12
    L2XBAR_CONFIG_INTERLEAVE(0x20, \
                0x0000000000000000, \
                0x0000000000001000, \
                0x00000000000000F0)
    L2XBAR_CONFIG_INTERLEAVE(0x28, \
                0x0000000000001000, \
                0x0000000000001000, \
                0x00000000000000F1)
    L2XBAR_CONFIG_INTERLEAVE(0x30, \
                0x0000000000000000, \
                0x0000000000001000, \
                0x00000000000010F0)
    L2XBAR_CONFIG_INTERLEAVE(0x38, \
                0x0000000000001000, \
                0x0000000000001000, \
                0x00000000000010F1)
    PRINTSTR("DDR Interleave using Bit 12\r\n")
#else
#ifdef INTERLEAVE_11
    L2XBAR_CONFIG_INTERLEAVE(0x20, \
                0x0000000000000000, \
                0x0000000000000800, \
                0x00000000000000F0)
    L2XBAR_CONFIG_INTERLEAVE(0x28, \
                0x0000000000000800, \
                0x0000000000000800, \
                0x00000000000000F1)
    L2XBAR_CONFIG_INTERLEAVE(0x30, \
                0x0000000000000000, \
                0x0000000000000800, \
                0x00000000000008F0)
    L2XBAR_CONFIG_INTERLEAVE(0x38, \
                0x0000000000000800, \
                0x0000000000000800, \
                0x00000000000008F1)
    PRINTSTR("DDR Interleave using Bit 11\r\n")
#else   //default use INTERLEAVE_10
    L2XBAR_CONFIG_INTERLEAVE(0x20, \
                0x0000000000000000, \
                0x0000000000000400, \
                0x00000000000000F0)
    L2XBAR_CONFIG_INTERLEAVE(0x28, \
                0x0000000000000400, \
                0x0000000000000400, \
                0x00000000000000F1)
    L2XBAR_CONFIG_INTERLEAVE(0x30, \
                0x0000000000000000, \
                0x0000000000000400, \
                0x00000000000004F0)
    L2XBAR_CONFIG_INTERLEAVE(0x38, \
                0x0000000000000400, \
                0x0000000000000400, \
                0x00000000000004F1)
    PRINTSTR("DDR Interleave using Bit 10\r\n")
#endif
#endif
#endif
#endif
    b       2f
    nop
#endif

3:
//NO_INTERLEAVE
//only support 2G(1+1),3G(1+2 or 2+1),4G(2+2),6G(2+4 or 4+2),8G(4+4).
    GET_MC0_MEMSIZE
    dli     t5, 0x2
    beq     a1, t5, 20f
    nop
    dli     t5, 0x4
    beq     a1, t5, 40f
    nop
    dli     t5, 0x8
    //temp code. >= 4G, use 4G only
    bgeu    a1, t5, 80f
    nop
    b       70f
    nop
20:     //MC0_SIZE_1G
    GET_MC1_MEMSIZE
    dli     t5, 0x2
    beq     a1, t5, 22f
    nop
    dli     t5, 0x4
    //temp code. >= 2G, use 2G only
    bgeu    a1, t5, 24f
    nop
    b       70f
    nop
22:     //MC1_SIZE_1G
//2G space: MC0: 8~bf; MC1: c~ff
    GET_NODE_ID_a0;
    XBAR_CONFIG_NODE_a0(0x20, \
                    0x0000000100000000, \
                    0xFFFFFFFFC0000000, \
                    0x00000000000000F0)
    PRINTSTR("MC0 space open : 0x100000000 - 0x13FFFFFFF\r\n")
    GET_NODE_ID_a0;
    XBAR_CONFIG_NODE_a0(0x28, \
                    0x0000000140000000, \
                    0xFFFFFFFFC0000000, \
                    0x00000000000000F1)
    PRINTSTR("MC1 space open : 0x100000000 - 0x13FFFFFFF\r\n")
    PRINTSTR("DDR space open : 0x140000000 - 0x17FFFFFFF\r\n")
    b       2f
    nop
24:     //MC1_SIZE_2G
//3G space: MC0: c~ff; MC1: 10~17f
    GET_NODE_ID_a0;
    XBAR_CONFIG_NODE_a0(0x20, \
                    0x0000000180000000, \
                    0xFFFFFFFFC0000000, \
                    0x00000000000000F0)
    PRINTSTR("MC0 space open : 0x180000000 - 0x1bFFFFFFF\r\n")
    GET_NODE_ID_a0;
    XBAR_CONFIG_NODE_a0(0x28, \
                    0x0000000100000000, \
                    0xFFFFFFFF80000000, \
                    0x00000000000000F1)
    PRINTSTR("MC1 space open : 0x100000000 - 0x17FFFFFFF\r\n")
    PRINTSTR("DDR space open : 0x100000000 - 0x1bFFFFFFF\r\n")
    b       2f
    nop
40:     //MC0_SIZE_2G
    GET_MC1_MEMSIZE
    dli     t5, 0x2
    beq     a1, t5, 42f
    nop
    dli     t5, 0x4
    beq     a1, t5, 44f
    nop
    dli     t5, 0x8
    //temp code. >= 4G, use 4G only
    bgeu    a1, t5, 48f
    nop
    b       70f
    nop
42:     //MC1_SIZE_1G
//3G space: MC1: c~ff; MC0: 10~17f   !!!!note: here use MC1 first for simple.
    GET_NODE_ID_a0;
    XBAR_CONFIG_NODE_a0(0x20, \
                    0x0000000180000000, \
                    0xFFFFFFFFC0000000, \
                    0x00000000000000F1)
    PRINTSTR("MC1 space open : 0x180000000 - 0x1BFFFFFFF\r\n")
    GET_NODE_ID_a0;
    XBAR_CONFIG_NODE_a0(0x28, \
                    0x0000000100000000, \
                    0xFFFFFFFF80000000, \
                    0x00000000000000F0)
    PRINTSTR("MC0 space open : 0x100000000 - 0x17FFFFFFF\r\n")
    PRINTSTR("DDR space open : 0x100000000 - 0x1BFFFFFFF\r\n")
    L2XBAR_RECONFIG_TO_MC1(0x10)
    L2XBAR_RECONFIG_TO_MC1(0x110)
    PRINTSTR("The opened CPU 0x00000000~0x0FFFFFFF Window is reassigned to MC1\r\n")
    PRINTSTR("The opened PCI 0x80000000~0x8FFFFFFF Window is reassigned to MC1\r\n")
    b       2f
    nop
44:     //MC1_SIZE_2G
//4G space: MC0: 10~17f; MC1: 18~1ff
    GET_NODE_ID_a0;
    XBAR_CONFIG_NODE_a0(0x20, \
                    0x0000000100000000, \
                    0xFFFFFFFF80000000, \
                    0x00000000000000F0)
    PRINTSTR("MC0 space open : 0x100000000 - 0x17FFFFFFF\r\n")
    GET_NODE_ID_a0;
    XBAR_CONFIG_NODE_a0(0x28, \
                    0x0000000180000000, \
                    0xFFFFFFFF80000000, \
                    0x00000000000000F1)
    PRINTSTR("MC1 space open : 0x180000000 - 0x1FFFFFFFF\r\n")
    PRINTSTR("DDR space open : 0x100000000 - 0x1FFFFFFFF\r\n")
    b       2f
    nop
48:     //MC1_SIZE_4G
//6G space: MC0: 18~1f; MC1: 20~30
    GET_NODE_ID_a0;
    XBAR_CONFIG_NODE_a0(0x20, \
                    0x0000000100000000, \
                    0xFFFFFFF180000000, \
                    0x00000000000000F0)
    PRINTSTR("MC0 space open : 0x100000000 - 0x17FFFFFFF\r\n")
    GET_NODE_ID_a0;
    XBAR_CONFIG_NODE_a0(0x28, \
                    0x0000000180000000, \
                    0xFFFFFFFF80000000, \
                    0x00000000000000F1)
    XBAR_CONFIG_NODE_a0(0x30, \
                    0x0000000200000000, \
                    0xFFFFFFFF80000000, \
                    0x00000000800000F1)
 
    PRINTSTR("MC1 space open : 0x180000000 - 0x27FFFFFFF\r\n")
    PRINTSTR("DDR space open : 0x100000000 - 0x27FFFFFFF\r\n")
    b       2f
    nop
80:     //MC0_SIZE_4G
    GET_MC1_MEMSIZE
    dli     t5, 0x4
    beq     a1, t5, 84f
    nop
    dli     t5, 0x8
    //temp code. >= 4G, use 4G only
    bgeu    a1, t5, 88f
    nop
    b       70f
    nop
84:     //MC1_SIZE_2G
//6G space: MC1: 18~1f; MC0: 20~30   !!!!note: here use MC1 first for simple.
    GET_NODE_ID_a0;
    XBAR_CONFIG_NODE_a0(0x20, \
                    0x0000000100000000, \
                    0xFFFFFFF180000000, \
                    0x00000000000000F1)
    PRINTSTR("MC1 space open : 0x100000000 - 0x17FFFFFFF\r\n")
    GET_NODE_ID_a0;
    XBAR_CONFIG_NODE_a0(0x28, \
                    0x0000000180000000, \
                    0xFFFFFFFF80000000, \
                    0x00000000000000F0)
    XBAR_CONFIG_NODE_a0(0x30, \
                    0x0000000200000000, \
                    0xFFFFFFFF80000000, \
                    0x00000000800000F0)
    PRINTSTR("MC0 space open : 0x180000000 - 0x27FFFFFFF\r\n")
    PRINTSTR("DDR space open : 0x100000000 - 0x27FFFFFFF\r\n")
    L2XBAR_RECONFIG_TO_MC1(0x10)
    L2XBAR_RECONFIG_TO_MC1(0x110)
    PRINTSTR("The opened CPU 0x00000000~0x0FFFFFFF Window is reassigned to MC1\r\n")
    PRINTSTR("The opened PCI 0x80000000~0x8FFFFFFF Window is reassigned to MC1\r\n")
    b       2f
    nop
88:     //MC1_SIZE_4G
//8G space: MC0: 20~2f; MC1: 30~3f
    GET_NODE_ID_a0;
    XBAR_CONFIG_NODE_a0(0x20, \
                    0x0000000100000000, \
                    0xFFFFFFFF00000000, \
                    0x00000000000000F0)
    PRINTSTR("MC0 space open : 0x100000000 - 0x1FFFFFFFF\r\n")
    GET_NODE_ID_a0;
    XBAR_CONFIG_NODE_a0(0x28, \
                    0x0000000200000000, \
                    0xFFFFFFFF00000000, \
                    0x00000000000000F1)
    PRINTSTR("MC1 space open : 0x200000000 - 0x2FFFFFFFF\r\n")
    PRINTSTR("DDR space open : 0x100000000 - 0x2FFFFFFFF\r\n")
    b       2f
    nop

2:
    //Config other PCI space exactly as cpu windows
    L2XBAR_CONFIG_PCI_AS_CPU(0x20);
    L2XBAR_CONFIG_PCI_AS_CPU(0x28);
    L2XBAR_CONFIG_PCI_AS_CPU(0x30);
    L2XBAR_CONFIG_PCI_AS_CPU(0x38);
    PRINTSTR("Full PCI space opened as cpu.\r\n")
    b       1f
    nop

70: //error: memory size not in support range
    PRINTSTR("The MEMSIZE is not supported, the L2-Xbar will not be configured!!!\r\n")
    PRINTSTR("-------------------------------------------\r\n")
#if 0
    GET_NODE_ID_a0;
    XBAR_CONFIG_NODE_a0(0x0, \
                0x0, \
                0x0, \
                0x0)
    PRINTSTR("!!!!!!!!!!MC space is disabled\r\n")
#endif
1:
    sync
    nop
    nop
    nop
    nop
