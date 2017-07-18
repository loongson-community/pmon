/**********************************
    loongson3_ddr2_config.S
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
************************************/

#######################################################
/**************************
0. s1 reset code
**************************/
#ifdef  AUTO_ARB_LEVEL
//store memory system info into s3(dimm i2c addr)
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
/**************************
1. 1. check NODE memory size.
**************************/
    GET_MC0_MEMSIZE
    beqz    a1, 89f
    nop
    dli     t5, 0x10
    bgt     a1, t5, 89f
    nop
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

//config MC0
    dli     t3, 0x0

#ifdef  AUTO_ARB_LEVEL
    bal     SET_AUTO_ARB_LEVEL_MARK
    nop
#endif

    bal     mc_init
    nop

    PRINTSTR("\r\nMC0 Config DONE\r\n")
/*****************************
 3. set msize for this NODE(if the memsize is supported)
******************************/
//use MC0 only
    GET_MC0_MEMSIZE
    move    t5, a1
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
88:
    sync
    nop
    nop
    nop
    nop
