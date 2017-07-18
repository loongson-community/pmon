/**********************************
    loongson3A2000_ddr2_config.S
        used to set up ddr controllers MC0 and MC1
        and set up the memory space on L2 Xbar
    input: s1--MC1 & MC0 DIMM info and Node ID
    note: s1 is damaged by the end of this file
    original: whd
    rewrite by Chen Xinke on 11/11/2010
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
    v1.6    Modify L2 Xbar address window configuration to new pmon-kernel interface.
            (default use NO_INTERLEAVE)
    v1.8    Modify supported Interleave bit. Add Interleave support when each MC uses 1GB or 2GB.
    v2.0    Add support for 8GB per MC.
    v2.1    support for interleave using the X2 interleave function for 3A2000
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

#ifdef MULTI_NODE_INIT_MEM
//set next free
    dli     t0, NODE0_CORE0_BUF0
    dli	    a0, 0x3
    and     a0, a0, s1
    dsll    a0, 44
    or      t0, a0, t0
    li      a1, NODE_SCACHE_ENABLED
    sw      a1, SP_OFF(t0)

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


#ifdef LS_DDR_HALF
#ifndef LSMCD3_2
//hot fix for lack of address pin A15 when use LSMC. For example RS780E R1.03
    //check MC1 first
    dsrl    t1, s1, 32
    //save s1
    move    t2, s1
    dsrl    s1, s1, 32
    GET_ROW_SIZE
    //restore s1
    move    s1, t2
    bnez    a1, 1f  //row size diff not 0, do nothing
    nop
    dli     t2, 0xff00
    and     t2, t1, t2      //t2 store memsize

    dli     a1, 1
    dsll    a1, a1, ROW_SIZE_OFFSET
    or      t1, t1, a1
    dsrl    t2, t2, 1       //half memsize
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
    GET_ROW_SIZE
    bnez    a1, 1f  //row size diff not 0, do nothing
    nop
    dli     t2, 0xff00
    and     t2, t1, t2      //t2 store memsize

    dli     a1, 1
    dsll    a1, a1, ROW_SIZE_OFFSET
    or      t1, t1, a1
    dsrl    t2, t2, 1       //half memsize
    //use reduced(if needed) memsize
    dli     a1, 0xffff00ff
    and     t1, t1, a1
    or      t1, t1, t2
    //use modified infor for MC0
    dli     a1, 0xffffffff00000000
    and     s1, s1, a1
    or      s1, s1, t1
1:
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
#endif
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
    beqz    a1, 89f
    nop
    dli     t5, 0x20
    bgt     a1, t5, 89f
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
    beqz    a1, 89f
    nop
    dli     t5, 0x10
    bgt     a1, t5, 89f
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

#ifndef  DISABLE_DIMM_ECC
    //Disable MC read buffer after ECC Initial
    GET_DIMM_ECC
    beqz    a1, 8f
    nop
    TTYDBG("Disable MC read buffer\r\n")
    lw      a1, 0x4(t2)
    li      a0, 0x8
    sll     a0, a0, t3
    or      a1, a1, a0
    sw      a1, 0x4(t2)
    sync
8:
#endif
    PRINTSTR("\r\nMC0 Config DONE\r\n")
//-------------------------------------
//config MC1 if not define MC0_ONLY
11:
    GET_MC0_ONLY
    bnez    a1, 12f
    nop

    dli     t3, 0x1

    //shift MC1 DIMM info to low 32bit of s1
    dsrl    t5, s1, 32
    dli     a1, 0xffff8000
    and     t5, t5, a1
    dli     a1, 0xffffffff8000ffff
    and     s1, s1, a1
    or      s1, s1, t5

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

#ifndef  DISABLE_DIMM_ECC
    //Disable MC read buffer after ECC Initial
    GET_DIMM_ECC
    beqz    a1, 8f
    nop
    TTYDBG("Disable MC read buffer\r\n")
    lw      a1, 0x4(t2)
    li      a0, 0x8
    sll     a0, a0, t3
    or      a1, a1, a0
    sw      a1, 0x4(t2)
    sync
8:
#endif
    TTYDBG("\r\nMC1 Config DONE\r\n")
//-------------------------------------
12:
/*******************************
 3. config L2 X-bar
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
/*
    //ONLY 1GB or 2GB each MC support interleave mode.
    dli     a1, 0x2
    beq     t5, a1, 3f
    nop
    dli     a1, 0x4
    beq     t5, a1, 3f
    nop
    b       1f
    nop
*/
3:
    GET_NODE_ID_a0;
    XBAR_CONFIG_NODE_a0(0x10, \
                    0x0000000000000000, \
                    0xFFFFFFFFF0000000, \
                    0x00000000000000F0 | (MC_INTERLEAVE_OFFSET << 56))
    //XBAR_CONFIG_NODE_a0(0x18, \
    //                0x0000000000000000 | (1 << MC_INTERLEAVE_OFFSET), \
    //                0xFFFFFFFFF0000000 | (1 << MC_INTERLEAVE_OFFSET), \
    //                0x00000000000000F1)
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
#if 0
    GET_NODE_ID_a0;
    XBAR_CONFIG_NODE_a0(0x18, \
                    0x000000002FF00000, \
                    0xFFFFFFFFFFF00000, \
                    0x000000000FF000F1)
    PRINTSTR("MC1 space open : 0x2FF00000 - 0x2FFFFFFF for MC configure space\r\n")
#endif

2:
    //Config PCI windows
    L2XBAR_CONFIG_PCI_AS_CPU(0x10);
    L2XBAR_CONFIG_PCI_BASE_0to8(0x110);
#ifndef NO_INTERLEAVE
    L2XBAR_CONFIG_PCI_AS_CPU(0x18);
    L2XBAR_CONFIG_PCI_BASE_0to8(0x118);
#endif
    PRINTSTR("PCI space open: 0x80000000 - 0x8FFFFFFF\r\n")

//config high memory windows
    GET_MC_SEL_BITS
    beqz    a1, 84f
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
//use one MC only, currently only support 512M, 1G, 2G, 3G, 4G, 6G, 8G
    move    a1, t5
    dli     t5, 0x1
    beq     a1, t5, 1f
    nop
    dli     t5, 0x2
    beq     a1, t5, 2f
    nop
    dli     t5, 0x4
    beq     a1, t5, 3f
    nop
    dli     t5, 0x6
    beq     a1, t5, 4f
    nop
    dli     t5, 0x8
    beq     a1, t5, 5f
    nop
    dli     t5, 0xc
    beq     a1, t5, 6f
    nop
    dli     t5, 0x10
    //temp code, MEM size >= 8G, use 8G only
    bgeu    a1, t5, 7f
    nop
    b       89f  //error condition
    nop
1:     //ddr_512MB_MC0:
    GET_NODE_ID_a0;
    XBAR_CONFIG_NODE_a0(0x20, \
                    0x0000000080000000, \
                    0xFFFFFFFFE0000000, \
                    0x00000000000000F0)
    PRINTSTR("MC0 space open : 0x80000000 - 0x9FFFFFFF\r\n")
    b       81f
    nop
2:     //ddr_1GB_MC:
    GET_NODE_ID_a0;
    XBAR_CONFIG_NODE_a0(0x20, \
                    0x0000000080000000, \
                    0xFFFFFFFFC0000000, \
                    0x00000000000000F0)
    PRINTSTR("MC0 space open : 0x80000000 - 0xBFFFFFFF\r\n")
    b       81f
    nop
3:     //ddr_2GB_MC0:
    GET_NODE_ID_a0;
    XBAR_CONFIG_NODE_a0(0x20, \
                    0x0000000080000000, \
                    0xFFFFFFFF80000000, \
                    0x00000000000000F0)
    PRINTSTR("MC0 space open : 0x80000000 - 0xFFFFFFFF\r\n")
    b       81f
    nop
4:     //ddr_3GB_MC0:
    GET_NODE_ID_a0;
    XBAR_CONFIG_NODE_a0(0x20, \
                    0x0000000080000000, \
                    0xFFFFFFFF80000000, \
                    0x00000000000000F0)
    GET_NODE_ID_a0;
    XBAR_CONFIG_NODE_a0(0x28, \
                    0x0000000100000000, \
                    0xFFFFFFFFC0000000, \
                    0x00000000800000F0)
    PRINTSTR("MC0 space open : 0x080000000 - 0x13FFFFFFF\r\n")
    b       81f
    nop
5:     //ddr_4GB_MC0:
    GET_NODE_ID_a0;
    XBAR_CONFIG_NODE_a0(0x20, \
                    0x0000000080000000, \
                    0xFFFFFFFF80000000, \
                    0x00000000000000F0)
    GET_NODE_ID_a0;
    XBAR_CONFIG_NODE_a0(0x28, \
                    0x0000000100000000, \
                    0xFFFFFFFF80000000, \
                    0x00000000800000F0)
    PRINTSTR("MC0 space open : 0x080000000 - 0x17FFFFFFF\r\n")
    b       81f
    nop
6:     //ddr_6GB_MC0:
    GET_NODE_ID_a0;
    XBAR_CONFIG_NODE_a0(0x20, \
                    0x0000000080000000, \
                    0xFFFFFFFF80000000, \
                    0x00000000000000F0)
    XBAR_CONFIG_NODE_a0(0x28, \
                    0x0000000100000000, \
                    0xFFFFFFFF80000000, \
                    0x00000000800000F0)
    XBAR_CONFIG_NODE_a0(0x30, \
                    0x0000000180000000, \
                    0xFFFFFFFF80000000, \
                    0x00000001000000F0)
    PRINTSTR("MC0 space open : 0x080000000 - 0x1FFFFFFFF\r\n")
    b       81f
    nop
7:     //ddr_8GB_MC0:
    GET_NODE_ID_a0;
    XBAR_CONFIG_NODE_a0(0x20, \
                    0x0000000080000000, \
                    0xFFFFFFFF80000000, \
                    0x00000000000000F0)
    XBAR_CONFIG_NODE_a0(0x28, \
                    0x0000000100000000, \
                    0xFFFFFFFF00000000, \
                    0x00000001000000F0)
    XBAR_CONFIG_NODE_a0(0x30, \
                    0x0000000200000000, \
                    0xFFFFFFFF80000000, \
                    0x00000000800000F0)
    PRINTSTR("MC0 space open : 0x080000000 - 0x27FFFFFFF\r\n")
    b       81f
    nop
81:
    GET_MC1_ONLY
    beqz    a1, 81f
    nop
/* MC1_ONLY */
    L2XBAR_RECONFIG_TO_MC1(0x20)
    L2XBAR_RECONFIG_TO_MC1(0x28)
    L2XBAR_RECONFIG_TO_MC1(0x30)
    PRINTSTR("The opened MC0 Window is reassigned to MC1\r\n")
    b       81f
    nop
84:  
//use MC0&MC1
#ifndef NO_INTERLEAVE
//interleave first, if MC1 memsize != MC0 memsize, do NO_interleave
    GET_MC0_MEMSIZE
    move    t5, a1
    GET_MC1_MEMSIZE
    bne     t5, a1, 85f
    nop
#if 0 //before 3A2000
#if 0
    //universal set, address start from memsize
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
/* ONLY 1GB, 2GB has INTERLEAVE mode now */
    dli     t5, 0x2
    beq     a1, t5, 1f
    nop
    dli     t5, 0x4
    beq     a1, t5, 2f
    nop
    b       85f
    nop
1:     //1G in each MC
    GET_NODE_ID_a0;
    XBAR_CONFIG_NODE_a0(0x20, \
                0x0000000080000000, \
                0xFFFFFFFFC0000000, \
                0x0000000000000000)
    XBAR_CONFIG_NODE_a0(0x28, \
                0x0000000080000000, \
                0xFFFFFFFFC0000000, \
                0x0000000000000000)
    XBAR_CONFIG_NODE_a0(0x30, \
                0x00000000C0000000, \
                0xFFFFFFFFC0000000, \
                0x0000000000000000)
    XBAR_CONFIG_NODE_a0(0x38, \
                0x00000000C0000000, \
                0xFFFFFFFFC0000000, \
                0x0000000000000000)
    PRINTSTR("DDR space open : 0x80000000 - 0xFFFFFFFF\r\n")
    b       86f
    nop
2:     //2G in each MC
    GET_NODE_ID_a0;
    XBAR_CONFIG_NODE_a0(0x20, \
                0x0000000080000000, \
                0xFFFFFFFF80000000, \
                0x0000000000000000)
    XBAR_CONFIG_NODE_a0(0x28, \
                0x0000000080000000, \
                0xFFFFFFFF80000000, \
                0x0000000000000000)
    XBAR_CONFIG_NODE_a0(0x30, \
                0x0000000100000000, \
                0xFFFFFFFF80000000, \
                0x0000000000000000)
    XBAR_CONFIG_NODE_a0(0x38, \
                0x0000000100000000, \
                0xFFFFFFFF80000000, \
                0x0000000000000000)
    PRINTSTR("DDR space open : 0x080000000 - 0x17FFFFFFF\r\n")
    b       86f
    nop
86:
#endif
//set interleave mode
    L2XBAR_CONFIG_INTERLEAVE(0x20, \
                0x0000000000000000, \
                0x0000000000000000 | (1 << MC_INTERLEAVE_OFFSET), \
                0x00000000000000F0)
    L2XBAR_CONFIG_INTERLEAVE(0x28, \
                0x0000000000000000 | (1 << MC_INTERLEAVE_OFFSET), \
                0x0000000000000000 | (1 << MC_INTERLEAVE_OFFSET), \
                0x00000000000000F1)
    L2XBAR_CONFIG_INTERLEAVE(0x30, \
                0x0000000000000000, \
                0x0000000000000000 | (1 << MC_INTERLEAVE_OFFSET), \
                0x00000000000000F0 | (1 << MC_INTERLEAVE_OFFSET))
    L2XBAR_CONFIG_INTERLEAVE(0x38, \
                0x0000000000000000 | (1 << MC_INTERLEAVE_OFFSET), \
                0x0000000000000000 | (1 << MC_INTERLEAVE_OFFSET), \
                0x00000000000000F1 | (1 << MC_INTERLEAVE_OFFSET))
    PRINTSTR("DDR Interleave space open : 0x080000000 - 0x17FFFFFFF\r\n")
    b       81f
    nop
#else //3A2000

//each channel, currently only support 512M, 1G, 2G, 3G, 4G, 6G, 8G
    move    a1, t5
    dli     t5, 0x1
    beq     a1, t5, 1f
    nop
    dli     t5, 0x2
    beq     a1, t5, 2f
    nop
    dli     t5, 0x4
    beq     a1, t5, 3f
    nop
    dli     t5, 0x6
    beq     a1, t5, 4f
    nop
    dli     t5, 0x8
    beq     a1, t5, 5f
    nop
    dli     t5, 0xc
    beq     a1, t5, 6f
    nop
    dli     t5, 0x10
    //temp code, MEM size >= 8G, use 8G only
    bgeu    a1, t5, 7f
    nop
    b       89f  //error condition
    nop
1:     //ddr_512MB_MC0:
    GET_NODE_ID_a0;
    XBAR_CONFIG_NODE_a0(0x20, \
                    0x0000000080000000, \
                    0xFFFFFFFFC0000000, \
                    0x00000000000000F0 | (MC_INTERLEAVE_OFFSET << 56))
    PRINTSTR("MC0/1 space open : 0x80000000 - 0xBFFFFFFF\r\n")
    b       81f
    nop
2:     //ddr_1GB_MC:
    GET_NODE_ID_a0;
    XBAR_CONFIG_NODE_a0(0x20, \
                    0x0000000080000000, \
                    0xFFFFFFFF80000000, \
                    0x00000000000000F0 | (MC_INTERLEAVE_OFFSET << 56))
    PRINTSTR("MC0/1 space open : 0x80000000 - 0xFFFFFFFF\r\n")
    b       81f
    nop
3:     //ddr_2GB_MC0:
    GET_NODE_ID_a0;
    XBAR_CONFIG_NODE_a0(0x20, \
                    0x0000000080000000, \
                    0xFFFFFFFF80000000, \
                    0x00000000000000F0 | (MC_INTERLEAVE_OFFSET << 56))
    GET_NODE_ID_a0;
    XBAR_CONFIG_NODE_a0(0x28, \
                    0x0000000100000000, \
                    0xFFFFFFFF80000000, \
                    0x00000000800000F0 | (MC_INTERLEAVE_OFFSET << 56))
    PRINTSTR("MC0/1 space open : 0x80000000 - 0x17FFFFFFF\r\n")
    b       81f
    nop
4:     //ddr_3GB_MC0:
    GET_NODE_ID_a0;
    XBAR_CONFIG_NODE_a0(0x20, \
                    0x0000000080000000, \
                    0xFFFFFFFF80000000, \
                    0x00000000000000F0 | (MC_INTERLEAVE_OFFSET << 56))
    XBAR_CONFIG_NODE_a0(0x28, \
                    0x0000000100000000, \
                    0xFFFFFFFF80000000, \
                    0x00000000800000F0 | (MC_INTERLEAVE_OFFSET << 56))
    XBAR_CONFIG_NODE_a0(0x30, \
                    0x0000000180000000, \
                    0xFFFFFFFF80000000, \
                    0x00000001000000F0 | (MC_INTERLEAVE_OFFSET << 56))
    PRINTSTR("MC0/1 space open : 0x080000000 - 0x1FFFFFFFF\r\n")
    b       81f
    nop
5:     //ddr_4GB_MC0:
    GET_NODE_ID_a0;
    XBAR_CONFIG_NODE_a0(0x20, \
                    0x0000000080000000, \
                    0xFFFFFFFF80000000, \
                    0x00000000000000F0 | (MC_INTERLEAVE_OFFSET << 56))
    XBAR_CONFIG_NODE_a0(0x28, \
                    0x0000000100000000, \
                    0xFFFFFFFF00000000, \
                    0x00000001000000F0 | (MC_INTERLEAVE_OFFSET << 56))
    XBAR_CONFIG_NODE_a0(0x30, \
                    0x0000000200000000, \
                    0xFFFFFFFF80000000, \
                    0x00000000800000F0 | (MC_INTERLEAVE_OFFSET << 56))
    PRINTSTR("MC0/1 space open : 0x080000000 - 0x27FFFFFFF\r\n")
    b       81f
    nop
6:     //ddr_6GB_MC0:
    GET_NODE_ID_a0;
    XBAR_CONFIG_NODE_a0(0x20, \
                    0x0000000080000000, \
                    0xFFFFFFFF80000000, \
                    0x00000000000000F0 | (MC_INTERLEAVE_OFFSET << 56))
    XBAR_CONFIG_NODE_a0(0x28, \
                    0x0000000100000000, \
                    0xFFFFFFFF00000000, \
                    0x00000001000000F0 | (MC_INTERLEAVE_OFFSET << 56))
    XBAR_CONFIG_NODE_a0(0x30, \
                    0x0000000200000000, \
                    0xFFFFFFFF00000000, \
                    0x00000002000000F0 | (MC_INTERLEAVE_OFFSET << 56))
    XBAR_CONFIG_NODE_a0(0x38, \
                    0x0000000300000000, \
                    0xFFFFFFFF80000000, \
                    0x00000000800000F0 | (MC_INTERLEAVE_OFFSET << 56))
    PRINTSTR("MC0/1 space open : 0x080000000 - 0x37FFFFFFF\r\n")
    b       81f
    nop
7:     //ddr_8GB_MC0:
    GET_NODE_ID_a0;
    XBAR_CONFIG_NODE_a0(0x20, \
                    0x0000000080000000, \
                    0xFFFFFFFF80000000, \
                    0x00000000000000F0 | (MC_INTERLEAVE_OFFSET << 56))
    XBAR_CONFIG_NODE_a0(0x28, \
                    0x0000000100000000, \
                    0xFFFFFFFF00000000, \
                    0x00000001000000F0 | (MC_INTERLEAVE_OFFSET << 56))
    XBAR_CONFIG_NODE_a0(0x30, \
                    0x0000000200000000, \
                    0xFFFFFFFE00000000, \
                    0x00000002000000F0 | (MC_INTERLEAVE_OFFSET << 56))
    XBAR_CONFIG_NODE_a0(0x38, \
                    0x0000000400000000, \
                    0xFFFFFFFF80000000, \
                    0x00000000800000F0 | (MC_INTERLEAVE_OFFSET << 56))
    PRINTSTR("MC0/1 space open : 0x080000000 - 0x47FFFFFFF\r\n")
    b       81f
    nop
#endif
#endif

85:
//NO_INTERLEAVE
//only support 2G(1+1),3G(1+2 or 2+1),4G(2+2),6G(2+4 or 4+2),8G(4+4),12G(8+4 or 4+8),16G(8+8).
    GET_MC0_MEMSIZE
    dli     t5, 0x2
    beq     a1, t5, 11f
    nop
    dli     t5, 0x4
    beq     a1, t5, 12f
    nop
    dli     t5, 0x8
    beq     a1, t5, 13f
    nop
    dli     t5, 0x10
    //temp code. >= 8G, use 8G only
    bgeu    a1, t5, 14f
    nop
    b       89f
    nop
11:     //MC0_SIZE_1G
    GET_MC1_MEMSIZE
    dli     t5, 0x2
    beq     a1, t5, 1f
    nop
    dli     t5, 0x4
    //temp code. >= 2G, use 2G only
    bgeu    a1, t5, 2f
    nop
    b       89f
    nop
1:     //MC1_SIZE_1G
//2G space: MC0: 8~bf; MC1: c~ff
    GET_NODE_ID_a0;
    XBAR_CONFIG_NODE_a0(0x20, \
                    0x0000000080000000, \
                    0xFFFFFFFFC0000000, \
                    0x00000000000000F0)
    PRINTSTR("MC0 space open : 0x80000000 - 0xBFFFFFFF\r\n")
    GET_NODE_ID_a0;
    XBAR_CONFIG_NODE_a0(0x28, \
                    0x00000000C0000000, \
                    0xFFFFFFFFC0000000, \
                    0x00000000000000F1)
    PRINTSTR("MC1 space open : 0xC0000000 - 0xFFFFFFFF\r\n")
    PRINTSTR("DDR space open : 0x80000000 - 0xFFFFFFFF\r\n")
    b       81f
    nop
2:     //MC1_SIZE_2G
//3G space: MC0: 8~bf; MC1: c~13f
    GET_NODE_ID_a0;
    XBAR_CONFIG_NODE_a0(0x20, \
                    0x0000000080000000, \
                    0xFFFFFFFFC0000000, \
                    0x00000000000000F0)
    PRINTSTR("MC0 space open : 0x80000000 - 0xBFFFFFFF\r\n")
    GET_NODE_ID_a0;
    XBAR_CONFIG_NODE_a0(0x28, \
                    0x00000000C0000000, \
                    0xFFFFFFFFC0000000, \
                    0x00000000000000F1)
    XBAR_CONFIG_NODE_a0(0x30, \
                    0x0000000100000000, \
                    0xFFFFFFFFC0000000, \
                    0x00000000400000F1)
    PRINTSTR("MC1 space open : 0x0C0000000 - 0x13FFFFFFF\r\n")
    PRINTSTR("DDR space open : 0x080000000 - 0x13FFFFFFF\r\n")
    b       81f
    nop

12:     //MC0_SIZE_2G
    GET_MC1_MEMSIZE
    dli     t5, 0x2
    beq     a1, t5, 1f
    nop
    dli     t5, 0x4
    beq     a1, t5, 2f
    nop
    dli     t5, 0x8
    beq     a1, t5, 3f
    nop
    dli     t5, 0x10
    //temp code. >= 8G, use 8G only
    bgeu    a1, t5, 4f
    nop
    b       89f
    nop
1:     //MC1_SIZE_1G
//3G space: MC0: 08~0f; MC1: 10~13
    GET_NODE_ID_a0;
    XBAR_CONFIG_NODE_a0(0x20, \
                    0x0000000080000000, \
                    0xFFFFFFFF80000000, \
                    0x00000000000000F0)
    PRINTSTR("MC0 space open : 0x80000000 - 0xFFFFFFFF\r\n")
    GET_NODE_ID_a0;
    XBAR_CONFIG_NODE_a0(0x28, \
                    0x0000000100000000, \
                    0xFFFFFFFFC0000000, \
                    0x00000000000000F1)
    PRINTSTR("MC1 space open : 0x100000000 - 0x13FFFFFFF\r\n")
    PRINTSTR("DDR space open : 0x080000000 - 0x13FFFFFFF\r\n")
    b       81f
    nop
2:     //MC1_SIZE_2G
//4G space: MC0: 08~0f; MC1: 10~17
    GET_NODE_ID_a0;
    XBAR_CONFIG_NODE_a0(0x20, \
                    0x0000000080000000, \
                    0xFFFFFFFF80000000, \
                    0x00000000000000F0)
    PRINTSTR("MC0 space open : 0x80000000 - 0xFFFFFFFF\r\n")
    GET_NODE_ID_a0;
    XBAR_CONFIG_NODE_a0(0x28, \
                    0x0000000100000000, \
                    0xFFFFFFFF80000000, \
                    0x00000000000000F1)
    PRINTSTR("MC1 space open : 0x100000000 - 0x17FFFFFFF\r\n")
    PRINTSTR("DDR space open : 0x080000000 - 0x17FFFFFFF\r\n")
    b       81f
    nop
3:     //MC1_SIZE_4G
//6G space: MC0: 08~0f; MC1: 10~1F
    GET_NODE_ID_a0;
    XBAR_CONFIG_NODE_a0(0x20, \
                    0x0000000080000000, \
                    0xFFFFFFFF80000000, \
                    0x00000000000000F0)
    PRINTSTR("MC0 space open : 0x80000000 - 0xFFFFFFFF\r\n")
    GET_NODE_ID_a0;
    XBAR_CONFIG_NODE_a0(0x28, \
                    0x0000000100000000, \
                    0xFFFFFFFF00000000, \
                    0x00000000000000F1)
    PRINTSTR("MC1 space open : 0x100000000 - 0x1FFFFFFFF\r\n")
    PRINTSTR("DDR space open : 0x080000000 - 0x1FFFFFFFF\r\n")
    b       81f
    nop
4:     //MC1_SIZE_8G
//10G space: MC0: 08~0f; MC1: 10~2F
    GET_NODE_ID_a0;
    XBAR_CONFIG_NODE_a0(0x20, \
                    0x0000000080000000, \
                    0xFFFFFFFF80000000, \
                    0x00000000000000F0)
    PRINTSTR("MC0 space open : 0x80000000 - 0xFFFFFFFF\r\n")
    GET_NODE_ID_a0;
    XBAR_CONFIG_NODE_a0(0x28, \
                    0x0000000100000000, \
                    0xFFFFFFFF00000000, \
                    0x00000000000000F1)
    XBAR_CONFIG_NODE_a0(0x30, \
                    0x0000000200000000, \
                    0xFFFFFFFF00000000, \
                    0x00000001000000F1)
    PRINTSTR("MC1 space open : 0x100000000 - 0x2FFFFFFFF\r\n")
    PRINTSTR("DDR space open : 0x080000000 - 0x2FFFFFFFF\r\n")
    b       81f
    nop

13:     //MC0_SIZE_4G
    GET_MC1_MEMSIZE
    dli     t5, 0x4
    beq     a1, t5, 1f
    nop
    dli     t5, 0x8
    beq     a1, t5, 2f
    nop
    dli     t5, 0x10
    //temp code. >= 8G, use 8G only
    bgeu    a1, t5, 3f
    nop
    b       89f
    nop
1:     //MC1_SIZE_2G
//6G space: MC0: 08~0f, 10~17; MC1: 18~1F
    GET_NODE_ID_a0;
    XBAR_CONFIG_NODE_a0(0x20, \
                    0x0000000080000000, \
                    0xFFFFFFFF80000000, \
                    0x00000000000000F0)
    XBAR_CONFIG_NODE_a0(0x28, \
                    0x0000000100000000, \
                    0xFFFFFFFF80000000, \
                    0x00000000800000F0)
    PRINTSTR("MC0 space open : 0x080000000 - 0x17FFFFFFF\r\n")
    GET_NODE_ID_a0;
    XBAR_CONFIG_NODE_a0(0x30, \
                    0x0000000180000000, \
                    0xFFFFFFFF80000000, \
                    0x00000000000000F1)
    PRINTSTR("MC1 space open : 0x180000000 - 0x1FFFFFFFF\r\n")
    PRINTSTR("DDR space open : 0x080000000 - 0x1FFFFFFFF\r\n")
    b       81f
    nop
2:     //MC1_SIZE_4G
//8G space: MC0: 08~0f, 20~27; MC1: 10~1f
    GET_NODE_ID_a0;
    XBAR_CONFIG_NODE_a0(0x20, \
                    0x0000000080000000, \
                    0xFFFFFFFF80000000, \
                    0x00000000000000F0)
    PRINTSTR("MC0 space open : 0x80000000 - 0xFFFFFFFF\r\n")
    GET_NODE_ID_a0;
    XBAR_CONFIG_NODE_a0(0x28, \
                    0x0000000100000000, \
                    0xFFFFFFFF00000000, \
                    0x00000000000000F1)
    PRINTSTR("MC1 space open : 0x100000000 - 0x1FFFFFFFF\r\n")
    GET_NODE_ID_a0;
    XBAR_CONFIG_NODE_a0(0x30, \
                    0x0000000200000000, \
                    0xFFFFFFFF80000000, \
                    0x00000000800000F0)
    PRINTSTR("MC0 space open : 0x200000000 - 0x27FFFFFFF\r\n")
    PRINTSTR("DDR space open : 0x080000000 - 0x27FFFFFFF\r\n")
    b       81f
    nop
3:     //MC1_SIZE_8G
//12G space: MC0: 10~1f; MC1: 08~0f, 20~37  note: here configure MC1 first
    GET_NODE_ID_a0;
    XBAR_CONFIG_NODE_a0(0x20, \
                    0x0000000100000000, \
                    0xFFFFFFFF00000000, \
                    0x00000000000000F0)
    PRINTSTR("MC0 space open : 0x100000000 - 0x1FFFFFFFF\r\n")
    GET_NODE_ID_a0;
    XBAR_CONFIG_NODE_a0(0x28, \
                    0x0000000080000000, \
                    0xFFFFFFFF80000000, \
                    0x00000000000000F1)
    PRINTSTR("MC1 space open : 0x080000000 - 0x0FFFFFFFF\r\n")
    GET_NODE_ID_a0;
    XBAR_CONFIG_NODE_a0(0x30, \
                    0x0000000200000000, \
                    0xFFFFFFFF00000000, \
                    0x00000001000000F1)
    XBAR_CONFIG_NODE_a0(0x38, \
                    0x0000000300000000, \
                    0xFFFFFFFF80000000, \
                    0x00000000800000F1)
    PRINTSTR("MC1 space open : 0x200000000 - 0x37FFFFFFF\r\n")
    PRINTSTR("DDR space open : 0x080000000 - 0x37FFFFFFF\r\n")
    L2XBAR_RECONFIG_TO_MC1(0x10)
    L2XBAR_RECONFIG_TO_MC1(0x110)
    PRINTSTR("The opened CPU 0x00000000~0x0FFFFFFF Window is reassigned to MC1\r\n")
    PRINTSTR("The opened PCI 0x80000000~0x8FFFFFFF Window is reassigned to MC1\r\n")
    b       81f
    nop

14:     //MC0_SIZE_8G
    GET_MC1_MEMSIZE
    dli     t5, 0x4
    beq     a1, t5, 1f
    nop
    dli     t5, 0x8
    beq     a1, t5, 2f
    nop
    dli     t5, 0x10
    //temp code. >= 8G, use 8G only
    bgeu    a1, t5, 3f
    nop
    b       89f
    nop
1:     //MC1_SIZE_2G
//10G space: MC1: 08~0f; MC0: 10~2f
    GET_NODE_ID_a0;
    XBAR_CONFIG_NODE_a0(0x20, \
                    0x0000000080000000, \
                    0xFFFFFFFF80000000, \
                    0x00000000000000F1)
    PRINTSTR("MC1 space open : 0x080000000 - 0x0FFFFFFFF\r\n")
    GET_NODE_ID_a0;
    XBAR_CONFIG_NODE_a0(0x28, \
                    0x0000000100000000, \
                    0xFFFFFFFF00000000, \
                    0x00000000000000F0)
    XBAR_CONFIG_NODE_a0(0x30, \
                    0x0000000200000000, \
                    0xFFFFFFFF00000000, \
                    0x00000001000000F0)
    PRINTSTR("MC0 space open : 0x100000000 - 0x2FFFFFFFF\r\n")
    PRINTSTR("DDR space open : 0x080000000 - 0x2FFFFFFFF\r\n")
    L2XBAR_RECONFIG_TO_MC1(0x10)
    L2XBAR_RECONFIG_TO_MC1(0x110)
    PRINTSTR("The opened CPU 0x00000000~0x0FFFFFFF Window is reassigned to MC1\r\n")
    PRINTSTR("The opened PCI 0x80000000~0x8FFFFFFF Window is reassigned to MC1\r\n")
    b       81f
    nop
2:     //MC1_SIZE_4G
//12G space: MC0: 08~0f, 20~2f, 30~37; MC1: 10~1f
    GET_NODE_ID_a0;
    XBAR_CONFIG_NODE_a0(0x20, \
                    0x0000000080000000, \
                    0xFFFFFFFF80000000, \
                    0x00000000000000F0)
    PRINTSTR("MC0 space open : 0x080000000 - 0x0FFFFFFFF\r\n")
    GET_NODE_ID_a0;
    XBAR_CONFIG_NODE_a0(0x28, \
                    0x0000000100000000, \
                    0xFFFFFFFF00000000, \
                    0x00000000000000F1)
    PRINTSTR("MC1 space open : 0x100000000 - 0x1FFFFFFFF\r\n")
    GET_NODE_ID_a0;
    XBAR_CONFIG_NODE_a0(0x30, \
                    0x0000000200000000, \
                    0xFFFFFFFF00000000, \
                    0x00000001000000F0)
    XBAR_CONFIG_NODE_a0(0x38, \
                    0x0000000300000000, \
                    0xFFFFFFFF80000000, \
                    0x00000000800000F0)
    PRINTSTR("MC0 space open : 0x200000000 - 0x37FFFFFFF\r\n")
    PRINTSTR("DDR space open : 0x080000000 - 0x37FFFFFFF\r\n")
    b       81f
    nop
3:     //MC1_SIZE_8G
//16G space: MC0: 08~0f, 10~1f, 40~4f; MC1: 20~3f
    GET_NODE_ID_a0;
    XBAR_CONFIG_NODE_a0(0x20, \
                    0x0000000080000000, \
                    0xFFFFFFFF80000000, \
                    0x00000000000000F0)
    XBAR_CONFIG_NODE_a0(0x28, \
                    0x0000000100000000, \
                    0xFFFFFFFF00000000, \
                    0x00000001000000F0)
    PRINTSTR("MC0 space open : 0x080000000 - 0x1FFFFFFFF\r\n")
    GET_NODE_ID_a0;
    XBAR_CONFIG_NODE_a0(0x30, \
                    0x0000000200000000, \
                    0xFFFFFFFE00000000, \
                    0x00000000000000F1)
    PRINTSTR("MC1 space open : 0x200000000 - 0x3FFFFFFFF\r\n")
    GET_NODE_ID_a0;
    XBAR_CONFIG_NODE_a0(0x38, \
                    0x0000000400000000, \
                    0xFFFFFFFF80000000, \
                    0x00000000800000F0)
    PRINTSTR("MC0 space open : 0x400000000 - 0x47FFFFFFF\r\n")
    PRINTSTR("DDR space open : 0x080000000 - 0x47FFFFFFF\r\n")
    b       81f
    nop

81:
    //Config other PCI space exactly as cpu windows
    L2XBAR_CONFIG_PCI_AS_CPU(0x20);
    L2XBAR_CONFIG_PCI_AS_CPU(0x28);
    L2XBAR_CONFIG_PCI_AS_CPU(0x30);
    L2XBAR_CONFIG_PCI_AS_CPU(0x38);
    PRINTSTR("Full PCI space opened as cpu.\r\n")

/*****************************
 4. set msize for this NODE(if the memsize is supported)
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
    b       88f
    nop

89: //error: memory size not in support range
    PRINTSTR("The MEMSIZE is not supported, the L2-Xbar will not be configured!!!\r\n")
    PRINTSTR("-------------------------------------------\r\n")
    //do not set this node memsize
#if 0
    GET_NODE_ID_a0;
    XBAR_CONFIG_NODE_a0(0x0, \
                0x0, \
                0x0, \
                0x0)
    PRINTSTR("!!!!!!!!!!MC space is disabled\r\n")
#endif
88:
    sync
    nop
    nop
    nop
    nop
