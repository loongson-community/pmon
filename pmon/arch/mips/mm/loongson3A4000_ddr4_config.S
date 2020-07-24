/**********************************
    loongson3A4000_ddr4_config.S
        used to set up ddr controllers MC0 and MC1
        and set up the memory space on L2 Xbar
    input: s1--MC1 & MC0 DIMM info and Node ID
    note: s1 is damaged by the end of this file
    original: whd
    rewrite by Chen Xinke on 11/11/2010
    rewrite by Zeng Lu on 03/27/2019
    rewrite by Huangshuai on 06/12/2019
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
    v3.0    support 3A4000 DDR4
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

//init mc0
#ifdef  AUTO_DDR_CONFIG
    move    s3, s1
    PRINTSTR("\r\nProbing DDR MC0 SLOT: ");
    bal     PROBE_CHANNEL_DIMM
    nop
#endif

    GET_DIMM_MEMSIZE_V1
    beqz    a1, 11f
    nop

    dli     t3, 0x0
    bal     mc_init
    nop

    PRINTSTR("\r\nMC0 Config DONE\r\n")
11:

    GET_DIMM_MEMSIZE_V1
//save MC0 MEMSIZE
    dsll    a1, S1_MC0_MEMSIZE_OFFSET_V1
    dli     t0, 0xf
    and     t0, s1
    or      t0, a1
//init mc1
    move    s1, s3
#ifdef  AUTO_DDR_CONFIG
    dsrl    s1, 24
    andi    s1, 0xff
    dsll    s1, 16
    dli     t1, 0xffff0000
    not     t1, t1
    and     s3, t1
    or      s1, s3
    move    s3, t0
    PRINTSTR("\r\nProbing DDR MC1 SLOT: ");
    bal     PROBE_CHANNEL_DIMM
    nop
#else
    move    s3, t0
#endif

    GET_DIMM_MEMSIZE_V1
    beqz    a1, 11f
    nop

    dli     t3, 0x1
    bal     mc_init
    nop

    PRINTSTR("\r\nMC1 Config DONE\r\n")
11:

//reconstruc s1, save MC0_MEMSIZE and MC1_MEMSIZE
    GET_DIMM_MEMSIZE_V1
    dsll    a1, S1_MC1_MEMSIZE_OFFSET_V1
    dli     t0, 0xf
    and     s1, t0
    or      s1, a1
    or      s1, s3

#if 1
    PRINTSTR("\r\ns1 = 0x")
    move    a0, s1
    bal     hexserial
    nop
    PRINTSTR("\r\n")
#endif

    GET_MC0_MEMSIZE_V1
    move    t4, a1
    GET_MC1_MEMSIZE_V1
    daddu   a1, a1, t4
    bnez    a1, 2f
    nop
    GET_NODE_ID_a1
    bnez    a1, 1f
    nop
    b       89f
    nop
1:
    b       88f
    nop
2:
/**************************
1. 1. check NODE memory size.
*  2. set MC0/1_ONLY if the following 2 conditions are satisfied:
*     (1). s1[3:2]=0b'00
*     (2). MC0 or MC1 MEMSIZE > 0.
* when use AUTO_DDR_CONFIG, one MC may have no DIMM while the other has, in this case,
* the code should set MC0_ONLY or MC1_ONLY in s1 automatically, because the code of 
* configuring L2-Xbar will use this message.
**************************/
#if 0
    GET_MC0_MEMSIZE
    beqz    a1, 1f
    nop
    GET_MC1_MEMSIZE
    beqz    a1, 1f
    nop
    GET_MC0_ONLY
    bnez    a1, 1f
    nop
    GET_MC1_ONLY
    bnez    a1, 2f
    nop
//s1[3:2]=0b'00
    //check memory size in this case
    GET_MC0_MEMSIZE
    move    t4, a1
    GET_MC1_MEMSIZE
    daddu   a1, a1, t4
    beqz    a1, 89f
    nop
    dli     t5, (DDR_WIN_BASE_ADDR >> 23) #max 2+4+8+16+32+64+2 or 4+8+16+32+64+128+4 ....
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
    dli     t5, 508
    bgt     a1, t5, 89f
    nop
4:
#endif
#if 0
    PRINTSTR("\r\ns1 = 0x")
    move    a0, s1
    bal     hexserial
    nop
    PRINTSTR("\r\n")
#endif
#if 0
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
    dli     t0, 0x900000003ff02000 #3A4000 win base change to 0x2000
    or      t2, t2, a0
    or      t0, t0, a0

//init MC1 will damage MC0 s1 info
//config MC0 if not define MC1_ONLY
//-------------------------------------
10:
    GET_MC1_ONLY
    bnez    a1, 11f
    nop


#ifdef  AUTO_ARB_LEVEL
    bal     SET_AUTO_ARB_LEVEL_MARK
    nop
#endif
    dli     t3, 0x0

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

    //shift MC1 DIMM info to low 32bit of s1
    dsrl    t5, s1, 32
    dli     a1, 0xffff8000
    and     t5, t5, a1
    dli     a1, 0xffffffff8000ffff
    and     s1, s1, a1
    or      s1, s1, t5

#ifdef  AUTO_ARB_LEVEL
    bal     SET_AUTO_ARB_LEVEL_MARK
    nop
#endif

    bal     mc_init
    nop

    TTYDBG("\r\nMC1 Config DONE\r\n")
//-------------------------------------
#endif
12:
/*******************************
 3. config memory windows
    code procedure: first, MC*_ONLY bits in s1 decides whether this MC is
    used,then according to MC*_MEMSIZE bits in s1 decide memory size and how
    the memory windows will be configured.
    note: currently,when use only 1 MC,support memory size: 2G, 3G, 4G;
      when use MC0&MC1 both, only support 1G, 2G or 4G Memory size of each Controller.
*******************************/

    sync
    nop
    nop
    nop
    nop


########################################################################
#check if only one mc has memory, USE MC0_ONLY or MC1_ONLY
    GET_MC0_MEMSIZE_V1
    move    a0, a1
    GET_MC1_MEMSIZE_V1
    and     a1, a0
    bnez    a1, 8f //winconf_both
    nop
/*MC0_ONLY */
    GET_MC0_MEMSIZE_V1
    beqz    a1, 9f //winconf_mc1_only
    nop
    MC_ENABLE(MC0_ONLY_EN)
    dli     a1, 0
    bal     enable_mc_regs_default
    nop
    dli     a1, 0
    bal     enable_mc_conf_space
    nop
    TTYDBG("\r\nEnable memory space of MC0:\r\n")
    GET_MC0_MEMSIZE_V1 #memsize in a1
    b       1f
    nop

9:// winconf_mc1_only:
/* MC1_ONLY */
    MC_ENABLE(MC1_ONLY_EN)
    dli     a1, 1
    bal     enable_mc_regs_default
    nop
    dli     a1, 1
    bal     enable_mc_conf_space
    nop
    TTYDBG("\r\nEnable memory space of MC1:\r\n")
    GET_MC1_MEMSIZE_V1 #memsize in a1

1:
    dli     t5, (DDR_WIN_BASE_ADDR >> 23) #this value * 512M == max 2+4+8+16+32+64+2 or 4+8+16+32+64+128+4 ....
    //error condition
    bgtu    a1, t5, 89f
    nop

    bal     winconf_mc_one
    nop

    dsll    t5, a1, 30
    dli     t6, DDR_WIN_BASE_ADDR
    daddu   t5, t5, t6
    dsubu   t5, t5, 1
    PRINTSTR("0x00000000_80000000 -- 0x")
    dsrl    a0, t5, 32
    bal     hexserial
    nop
    PRINTSTR("_")
    move    a0, t5 
    bal     hexserial
    nop
    PRINTSTR("\r\n")

    b       81f
    nop

########################################################################
#New mc win method only support interleaved mode.
#No interleave mode not supported, least common memory size will be used
#when memory size of the two channels are not equal.
########################################################################
8: //winconf_mc_both:
1://BOTH EN
//if MC1 memsize != MC0 memsize, use the small size x2
    GET_MC0_MEMSIZE_V1
    move    t4, a1
    GET_MC1_MEMSIZE_V1
    bne     t4, a1, 1f
    nop
3:
    b       2f
    nop
1:
    PRINTSTR("!!!MEMSIZE NOT equal, least common memory size will be used.\r\n To maximize memory capacity, install same DIMMs to each channel\r\n")

2:
//if MC1 memsize != MC0 memsize, use the small size x2
    PRINTSTR("\r\nEnable memory space of MC0/1(interleaved at bit ")
    li      a0, MC_INTERLEAVE_OFFSET
    bal     hexserial
    nop
    PRINTSTR("):\r\n")

#calc total memsize to a1
    GET_MC0_MEMSIZE_V1
    move    t4, a1
    GET_MC1_MEMSIZE_V1
    bgeu    t4, a1, 1f
    nop
    dsll    a1, t4, 1 //mc0 < mc1 use mc0 size
    b 2f
    nop
1:
    dsll    a1, a1, 1
2:

#check memsize range supported
    dli     t5, (DDR_WIN_BASE_ADDR >> 23) #max 2+4+8+16+32+64+2 or 4+8+16+32+64+128+4 ....
    //error condition
    bgtu    a1, t5, 89f
    nop
    dsll    t5, a1, 30
    dli     t6, DDR_WIN_BASE_ADDR
    daddu   t5, t5, t6
    dsubu   t5, t5, 1
    PRINTSTR("0x00000000_80000000 -- 0x")
    dsrl    a0, t5, 32
    bal     hexserial
    nop
    PRINTSTR("_")
    move    a0, t5 
    bal     hexserial
    nop
    PRINTSTR("\r\n")

#re-calc total memsize to a1
    GET_MC0_MEMSIZE_V1
    move    t4, a1
    GET_MC1_MEMSIZE_V1
    bgeu    t4, a1, 1f
    nop
    dsll    a1, t4, 1 //mc0 < mc1 use mc0 size
    b 2f
    nop
1:
    dsll    a1, a1, 1
2:

#set mc0 windows
    MC_ENABLE(MC0_ONLY_EN)
    move    a2, a1
    dli     a1, 0
    bal     enable_mc_regs_default
    nop
    dli     a1, 0
    bal     enable_mc_conf_space
    nop
    move    a1, a2
    bal     winconf_mc_one
    nop

#set mc1 windows
    MC_ENABLE(MC1_ONLY_EN)
    move    a2, a1
    dli     a1, 1
    bal     enable_mc_regs_default
    nop
    dli     a1, 1
    bal     enable_mc_conf_space
    nop
    move    a1, a2
    bal     winconf_mc_one
    nop

    MC_ENABLE(MC_BOTH_EN)
    MC_INTERLEAVE_ENBALE(MC_INTERLEAVE_OFFSET)

81:
    GET_NODE_ID_a0
    dli     t2, 0x900000001fe00180
    or      t2, t2, a0
    dli     a1, 0
    bal     disable_mc_conf_space
    nop
    dli     a1, 1
    bal     disable_mc_conf_space
    nop

    dli     a1, 0
    bal     disable_mc_regs_default
    nop
    dli     a1, 1
    bal     disable_mc_regs_default
    nop
    //MC window config complete

/*****************************
 4. set msize for this NODE(if the memsize is supported)
******************************/
#if 0
    GET_MC0_ONLY
    beqz    a1, 1f
    nop
//use MC0 only
    GET_MC0_MEMSIZE
    move    t4, a1
    b       2f
    nop
1:
    GET_MC1_ONLY
    beqz    a1, 1f
    nop
//use MC1 only
    GET_MC1_MEMSIZE
    move    t4, a1
    b       2f
    nop
1:
#endif
//use MC0&MC1
    GET_MC0_MEMSIZE_V1
    move    t4, a1
    GET_MC1_MEMSIZE_V1
    daddu   t4, t4, a1
2:
    GET_NODE_ID_a0;
    dsrl    a0, a0, 44  //because of the macro define
    dsll    a0, a0, 3   //a0=a0*8
    dsll    t4, t4, a0
    or      msize, msize, t4

#ifdef  PRINT_MSG
    PRINTSTR("\r\nmsize = 0x")
    move    a0, msize
    bal     hexserial64
    nop
    PRINTSTR("\r\n")
#endif
    b       88f
    nop

89: //error: memory size not in support range
    PRINTSTR("The MEMSIZE is not supported or there is no DIMM, *MEMORY WINDOW* will not be configured!!!\r\n")
    PRINTSTR("Please make sure and reboot!!!\r\n")
    PRINTSTR("-------------------------------------------\r\n")
1:
    b       1b
    nop
    //do not set this node memsize
88:

    sync
    nop
    nop
    nop
    nop
