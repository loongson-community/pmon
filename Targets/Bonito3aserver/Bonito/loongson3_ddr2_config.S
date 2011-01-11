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
************************************/

#######################################################
//make sure s1[3:2] is correctly set.
        GET_MC_SEL_BITS
        dli     t5, 3
        bne     a1, t5, 1f
        nop
    //s1[3:2]=0b'11, clear to 0b'00
        dli     t5, 0xc
        not     t5, t5
        and     s1, s1, t5
1:
/************************
1. set up Memory Controller.
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

###Enable the reg space###
#if 1
	TTYDBG	("\r\nEnable register space of MEMORY\r\n")
        lw      a1, 0x0(t2)
        li      a0, 0xfffffeff
        and     a1, a1,a0
        sw      a1, 0x0(t2)
#endif    

//config MC0 if not define MC1_ONLY
//-------------------------------------
10:
        GET_MC1_ONLY
        bnez    a1, 11f
        nop
        GET_NODE_ID_a0;
        XBAR_CONFIG_NODE_a0(0x10, \
                            0x0000000000000000, \
                            0xfffffffff0000000, \
                            0x00000000000000f0)
        sync

        dli     a1, 0x900000000ff00000
        or      a0, a0, a1
	    bal	    ddr2_config
	    nop
	    PRINTSTR("\r\nMC0 Config DONE\r\n")
#ifdef  DEBUG_DDR   //print registers
    PRINTSTR("The MC0 configuration is:\r\n")
#ifdef  loongson3A3
    dli     t1, 179
#else
    dli     t1, 151
#endif
    GET_NODE_ID_a0
    dli     t5, 0x900000000ff00000
    or      t5, t5, a0
1:
    ld      t6, 0x0(t5)
    dsrl    a0, t6, 32
    bal	    hexserial
    nop
    PRINTSTR("  ")
    move    a0, t6
    bal	    hexserial
    nop
    PRINTSTR("\r\n")

    daddiu  t5, t5, 16
    bnez    t1, 1b
    daddiu  t1, t1, -1
#endif
//-------------------------------------
//config MC1 if not define MC0_ONLY
11:
        GET_MC0_ONLY
        bnez    a1, 12f
        nop
//shift MC1 DIMM info to low 32bit of s1
        dsrl    t5, s1, 32
        dli     a1, 0xffff0000
        and     t5, t5, a1
        dli     a1, 0xffffffff0000ffff
        and     s1, s1, a1
        or      s1, s1, t5

        GET_NODE_ID_a0;
        XBAR_CONFIG_NODE_a0(0x10, \
                            0x0000000000000000, \
                            0xfffffffff0000000, \
                            0x00000000000000f1)
        sync

        dli     a1, 0x900000000ff00000
        or      a0, a0, a1
	    bal	    ddr2_config_mc1
	    nop
	    PRINTSTR("\r\nMC1 Config DONE\r\n")
#ifdef  DEBUG_DDR   //print registers
    PRINTSTR("The MC1 configuration is:\r\n")
#ifdef  loongson3A3
    dli     t1, 179
#else
    dli     t1, 151
#endif
    GET_NODE_ID_a0
    dli     t5, 0x900000000ff00000
    or      t5, t5, a0
1:
    ld      t6, 0x0(t5)
    dsrl    a0, t6, 32
    bal	    hexserial
    nop
    PRINTSTR("  ")
    move    a0, t6
    bal	    hexserial
    nop
    PRINTSTR("\r\n")

    daddiu  t5, t5, 16
    bnez    t1, 1b
    daddiu  t1, t1, -1
#endif
//-------------------------------------
12:

###disable the reg space###
#if 1
        TTYDBG("Disable register space of MEMORY\r\n")
        lw      a1,0x0(t2)
        or      a1,a1,0x100
        sw      a1,0x0(t2)
#endif

#if 1 // AdonWang disable ddr3 readbuff
/*      May Affect the Performance     */
//This seems better for the spec2000
        TTYDBG("Disable read buffer\r\n")
        lw      t1, 0x4(t2)
        li      a0, 0x18
        or      t1, t1, a0
        sw      t1, 0x4(t2)
#endif

#if 1 // AdonWang disable cpu buffered read
/* !!!!!!!!!!!!! IMPORTANT !!!!!!!!!!!! */
        TTYDBG("Disable cpu buffered read\r\n")
        lw      t1, 0x0(t2)
        li      a0, 0xfffffdff
        and     t1, t1, a0
        sw      t1, 0x0(t2)
#endif
/**************************
2. 1. check NODE memory size.
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
#ifdef  DEBUG_DDR
        PRINTSTR("\r\ns1 = 0x")
        move    a0, s1
        bal     hexserial
        nop
        PRINTSTR("\r\n")
#endif
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

        PRINTSTR("\r\nmsize = 0x")
        move    a0, msize
        bal     hexserial
        nop
        PRINTSTR("\r\n")
/*******************************
 4. config L2 X-bar
    code procedure: first, MC*_ONLY bits in s1 decides whether this MC is
    used,then according to MC*_MEMSIZE bits in s1 decide memory size and how
    the L2 X-bar windows will be configured.
    note: currently,when use only 1 MC,support memory size: 512M, 1G, 2G, 3G, 4G;
          when use MC0&MC1 both, only support 1G, 2G or 4G Memory size of each Controller.
*******************************/
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
        GET_NODE_ID_a0;
        XBAR_CONFIG_NODE_a0(0x100, \
                            0x0000000080000000, \
                            0xFFFFFFFFF0000000, \
                            0x00000000000000F0)
	    PRINTSTR("PCI space open to MC0: 0x80000000 - 0x8FFFFFFF\r\n")
        GET_MC1_ONLY
        beqz    a1, 2f
        nop
/* MC1_ONLY */
        L2XBAR_RECONFIG_TO_MC1(0x10)
        L2XBAR_RECONFIG_TO_MC1(0x100)
	    PRINTSTR("The opened MC0 Window is reassigned to MC1\r\n")
        b       2f
        nop
1:
#ifdef NO_INTERLEAVE
        GET_NODE_ID_a0;
        XBAR_CONFIG_NODE_a0(0x10, \
                            0x0000000000000000, \
                            0xFFFFFFFFF0000000, \
                            0x00000000000000F0)
	    PRINTSTR("DDR space open : 0x00000000 - 0x0FFFFFFF\r\n")
        GET_NODE_ID_a0;
        XBAR_CONFIG_NODE_a0(0x100, \
                            0x0000000080000000, \
                            0xFFFFFFFFF0000000, \
                            0x00000000000000F0)
	    PRINTSTR("PCI space open to MC0: 0x80000000 - 0x8FFFFFFF\r\n")
#else
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
#else   //default use INTERLEAVE_10
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
        //temp code. NO interleave PCI space
        GET_NODE_ID_a0;
        XBAR_CONFIG_NODE_a0(0x100, \
                            0x0000000080000000, \
                            0xFFFFFFFFF0000000, \
                            0x00000000000000F0)
	    PRINTSTR("PCI space open to MC0: 0x80000000 - 0x8FFFFFFF\r\n")
#endif
2:

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
                            0x0000000020000000, \
                            0xFFFFFFFFF0000000, \
                            0x00000000100000F0)
	    PRINTSTR("MC0 pace open : 0x20000000 - 0x2FFFFFFF\r\n")
        GET_NODE_ID_a0;
        XBAR_CONFIG_NODE_a0(0x120, \
                            0x0000000080000000, \
                            0xFFFFFFFFE0000000, \
                            0x00000000000000F0)
	    PRINTSTR("PCI space open to MC0: 0x80000000 - 0x9FFFFFFF\r\n")
        b       6f
        nop
20:     //ddr_1GB_MC0:
        GET_NODE_ID_a0;
        XBAR_CONFIG_NODE_a0(0x20, \
                            0x0000000040000000, \
                            0xFFFFFFFFC0000000, \
                            0x00000000000000F0)
	    PRINTSTR("MC0 space open : 0x40000000 - 0x7FFFFFFF\r\n")
        GET_NODE_ID_a0;
        XBAR_CONFIG_NODE_a0(0x120, \
                            0x0000000080000000, \
                            0xFFFFFFFFC0000000, \
                            0x00000000000000F0)
	    PRINTSTR("PCI space open to MC0: 0x80000000 - 0xBFFFFFFF\r\n")
        b       6f
        nop
40:     //ddr_2GB_MC0:
        GET_NODE_ID_a0;
        XBAR_CONFIG_NODE_a0(0x20, \
                            0x0000000080000000, \
                            0xFFFFFFFF80000000, \
                            0x00000000000000F0)
	    PRINTSTR("MC0 space open : 0x80000000 - 0xFFFFFFFF\r\n")
        b       3f
        nop
60:     //ddr_3GB_MC0:
        GET_NODE_ID_a0;
        XBAR_CONFIG_NODE_a0(0x20, \
                            0x00000000C0000000, \
                            0xFFFFFFFFC0000000, \
                            0x00000000000000F0)
        GET_NODE_ID_a0;
        XBAR_CONFIG_NODE_a0(0x28, \
                            0x0000000100000000, \
                            0xFFFFFFFF80000000, \
                            0x00000000000000F0)
	    PRINTSTR("MC0 space open : 0x0C0000000 - 0x17FFFFFFF\r\n")
        b       3f
        nop
80:     //ddr_4GB_MC0:
        GET_NODE_ID_a0;
        XBAR_CONFIG_NODE_a0(0x20, \
                            0x0000000100000000, \
                            0xFFFFFFFF00000000, \
                            0x00000000000000F0)
	    PRINTSTR("MC0 space open : 0x100000000 - 0x1FFFFFFFF\r\n")
        b       3f
        nop
3:
        //memsize >= 2G, PCIDMA still use 2G.
        GET_NODE_ID_a0;
        XBAR_CONFIG_NODE_a0(0x120, \
                            0x0000000080000000, \
                            0xFFFFFFFF80000000, \
                            0x00000000000000F0)
	    PRINTSTR("PCI space open to MC0: 0x80000000 - 0xFFFFFFFF\r\n")
        b       6f
        nop
6:
        GET_MC1_ONLY
        beqz    a1, 2f
        nop
/* MC1_ONLY */
        L2XBAR_RECONFIG_TO_MC1(0x20)
        L2XBAR_RECONFIG_TO_MC1(0x28)
        L2XBAR_RECONFIG_TO_MC1(0x120)
	    PRINTSTR("The opened MC0 Window is reassigned to MC1\r\n")
        b       2f
        nop

4:  
//use MC0&MC1
    //temp code. Config PCI space. No matter what the Interleave mode and MEMSIZE are, use this PCI config.
        GET_NODE_ID_a0;
        XBAR_CONFIG_NODE_a0(0x120, \
                            0x0000000080000000, \
                            0xFFFFFFFFC0000000, \
                            0x00000000000000F0)
	    PRINTSTR("PCI space open to MC0: 0x80000000 - 0xBFFFFFFF\r\n")
        GET_NODE_ID_a0;
        XBAR_CONFIG_NODE_a0(0x138, \
                            0x00000000C0000000, \
                            0xFFFFFFFFC0000000, \
                            0x00000000000000F1)
	    PRINTSTR("PCI space open to MC1: 0xC0000000 - 0xFFFFFFFF\r\n")
	    PRINTSTR("PCI space open: 0x80000000 - 0xFFFFFFFF\r\n")
#ifdef NO_INTERLEAVE
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
        b       2f
        nop
24:     //MC1_SIZE_2G
//3G space: MC0: c~ff; MC1: 10~17f
        GET_NODE_ID_a0;
        XBAR_CONFIG_NODE_a0(0x20, \
                            0x00000000C0000000, \
                            0xFFFFFFFFC0000000, \
                            0x00000000000000F0)
	    PRINTSTR("MC0 space open : 0xC0000000 - 0xFFFFFFFF\r\n")
        GET_NODE_ID_a0;
        XBAR_CONFIG_NODE_a0(0x28, \
                            0x0000000100000000, \
                            0xFFFFFFFF80000000, \
                            0x00000000000000F1)
	    PRINTSTR("MC1 space open : 0x100000000 - 0x17FFFFFFF\r\n")
	    PRINTSTR("DDR space open : 0x0C0000000 - 0x17FFFFFFF\r\n")
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
                            0x00000000C0000000, \
                            0xFFFFFFFFC0000000, \
                            0x00000000000000F1)
	    PRINTSTR("MC1 space open : 0xC0000000 - 0xFFFFFFFF\r\n")
        GET_NODE_ID_a0;
        XBAR_CONFIG_NODE_a0(0x28, \
                            0x0000000100000000, \
                            0xFFFFFFFF80000000, \
                            0x00000000000000F0)
	    PRINTSTR("MC0 space open : 0x100000000 - 0x17FFFFFFF\r\n")
	    PRINTSTR("DDR space open : 0x0C0000000 - 0x17FFFFFFF\r\n")
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
                            0x0000000180000000, \
                            0xFFFFFFF180000000, \
                            0x00000000000000F0)
	    PRINTSTR("MC0 space open : 0x180000000 - 0x1FFFFFFFF\r\n")
        GET_NODE_ID_a0;
        XBAR_CONFIG_NODE_a0(0x28, \
                            0x0000000200000000, \
                            0xFFFFFFFF00000000, \
                            0x00000000000000F1)
	    PRINTSTR("MC1 space open : 0x200000000 - 0x2FFFFFFFF\r\n")
	    PRINTSTR("DDR space open : 0x180000000 - 0x2FFFFFFFF\r\n")
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
                            0x0000000180000000, \
                            0xFFFFFFF180000000, \
                            0x00000000000000F1)
	    PRINTSTR("MC1 space open : 0x180000000 - 0x1FFFFFFFF\r\n")
        GET_NODE_ID_a0;
        XBAR_CONFIG_NODE_a0(0x28, \
                            0x0000000200000000, \
                            0xFFFFFFFF00000000, \
                            0x00000000000000F0)
	    PRINTSTR("MC0 space open : 0x200000000 - 0x2FFFFFFFF\r\n")
	    PRINTSTR("DDR space open : 0x180000000 - 0x2FFFFFFFF\r\n")
        b       2f
        nop
88:     //MC1_SIZE_4G
//8G space: MC0: 20~2f; MC1: 30~3f
        GET_NODE_ID_a0;
        XBAR_CONFIG_NODE_a0(0x20, \
                            0x0000000200000000, \
                            0xFFFFFFFF00000000, \
                            0x00000000000000F0)
	    PRINTSTR("MC0 space open : 0x200000000 - 0x2FFFFFFFF\r\n")
        GET_NODE_ID_a0;
        XBAR_CONFIG_NODE_a0(0x28, \
                            0x0000000300000000, \
                            0xFFFFFFFF00000000, \
                            0x00000000000000F1)
	    PRINTSTR("MC1 space open : 0x300000000 - 0x3FFFFFFFF\r\n")
	    PRINTSTR("DDR space open : 0x200000000 - 0x3FFFFFFFF\r\n")
        b       2f
        nop
#else   //INTERLEAVE MODE
/* ONLY 1GB, 2GB and 4G has INTERLEAVE mode now */
        GET_MC0_MEMSIZE
        move    t5, a1
        GET_MC1_MEMSIZE
        bne     a1, t5, 3f
        nop
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
#ifdef INTERLEAVE_27
	    PRINTSTR("DDR Interleave using Bit 27\r\n")
        GET_NODE_ID_a0;
        XBAR_CONFIG_NODE_a0(0x20, \
                            0x0000000080000000, \
                            0xFFFFFFFFC8000000, \
                            0x00000000000000F0)
        XBAR_CONFIG_NODE_a0(0x28, \
                            0x0000000088000000, \
                            0xFFFFFFFFC8000000, \
                            0x00000000000000F1)
        XBAR_CONFIG_NODE_a0(0x30, \
                            0x00000000C0000000, \
                            0xFFFFFFFFC8000000, \
                            0x00000000080000F0)
        XBAR_CONFIG_NODE_a0(0x38, \
                            0x00000000C8000000, \
                            0xFFFFFFFFC8000000, \
                            0x00000000080000F1)
#else
#ifdef INTERLEAVE_16
	    PRINTSTR("DDR Interleave using Bit 16\r\n")
        GET_NODE_ID_a0;
        XBAR_CONFIG_NODE_a0(0x20, \
                            0x0000000080000000, \
                            0xFFFFFFFFC0010000, \
                            0x00000000000000F0)
        XBAR_CONFIG_NODE_a0(0x28, \
                            0x0000000080010000, \
                            0xFFFFFFFFC0010000, \
                            0x00000000000000F1)
        XBAR_CONFIG_NODE_a0(0x30, \
                            0x00000000C0000000, \
                            0xFFFFFFFFC0010000, \
                            0x00000000000100F0)
        XBAR_CONFIG_NODE_a0(0x38, \
                            0x00000000C0010000, \
                            0xFFFFFFFFC0010000, \
                            0x00000000000100F1)
#else
#ifdef INTERLEAVE_13
	    PRINTSTR("DDR Interleave using Bit 13\r\n")
        GET_NODE_ID_a0;
        XBAR_CONFIG_NODE_a0(0x20, \
                            0x0000000080000000, \
                            0xFFFFFFFFC0002000, \
                            0x00000000000000F0)
        XBAR_CONFIG_NODE_a0(0x28, \
                            0x0000000080002000, \
                            0xFFFFFFFFC0002000, \
                            0x00000000000000F1)
        XBAR_CONFIG_NODE_a0(0x30, \
                            0x00000000C0000000, \
                            0xFFFFFFFFC0002000, \
                            0x00000000000020F0)
        XBAR_CONFIG_NODE_a0(0x38, \
                            0x00000000C0002000, \
                            0xFFFFFFFFC0002000, \
                            0x00000000000020F1)
#else
#ifdef INTERLEAVE_12
	    PRINTSTR("DDR Interleave using Bit 12\r\n")
        GET_NODE_ID_a0;
        XBAR_CONFIG_NODE_a0(0x20, \
                            0x0000000080000000, \
                            0xFFFFFFFFC0001000, \
                            0x00000000000000F0)
        XBAR_CONFIG_NODE_a0(0x28, \
                            0x0000000080001000, \
                            0xFFFFFFFFC0001000, \
                            0x00000000000000F1)
        XBAR_CONFIG_NODE_a0(0x30, \
                            0x00000000C0000000, \
                            0xFFFFFFFFC0001000, \
                            0x00000000000010F0)
        XBAR_CONFIG_NODE_a0(0x38, \
                            0x00000000C0001000, \
                            0xFFFFFFFFC0001000, \
                            0x00000000000010F1)
#else
#ifdef INTERLEAVE_11
	    PRINTSTR("DDR Interleave using Bit 11\r\n")
        GET_NODE_ID_a0;
        XBAR_CONFIG_NODE_a0(0x20, \
                            0x0000000080000000, \
                            0xFFFFFFFFC0000800, \
                            0x00000000000000F0)
        XBAR_CONFIG_NODE_a0(0x28, \
                            0x0000000080000800, \
                            0xFFFFFFFFC0000800, \
                            0x00000000000000F1)
        XBAR_CONFIG_NODE_a0(0x30, \
                            0x00000000C0000000, \
                            0xFFFFFFFFC0000800, \
                            0x00000000000008F0)
        XBAR_CONFIG_NODE_a0(0x38, \
                            0x00000000C0000800, \
                            0xFFFFFFFFC0000800, \
                            0x00000000000008F1)
#else   //default use INTERLEAVE_10
	    PRINTSTR("DDR Interleave using Bit 10\r\n")
        GET_NODE_ID_a0;
        XBAR_CONFIG_NODE_a0(0x20, \
                            0x0000000080000000, \
                            0xFFFFFFFFC0000400, \
                            0x00000000000000F0)
        XBAR_CONFIG_NODE_a0(0x28, \
                            0x0000000080000400, \
                            0xFFFFFFFFC0000400, \
                            0x00000000000000F1)
        XBAR_CONFIG_NODE_a0(0x30, \
                            0x00000000C0000000, \
                            0xFFFFFFFFC0000400, \
                            0x00000000000004F0)
        XBAR_CONFIG_NODE_a0(0x38, \
                            0x00000000C0000400, \
                            0xFFFFFFFFC0000400, \
                            0x00000000000004F1)
#endif
#endif
#endif
#endif
#endif
	    PRINTSTR("DDR Interleave space open : 0x80000000 - 0xFFFFFFFF\r\n")
        b       2f
        nop
40:     //2G in each MC
#ifdef INTERLEAVE_27
	    PRINTSTR("DDR Interleave using Bit 27\r\n")
        GET_NODE_ID_a0;
        XBAR_CONFIG_NODE_a0(0x20, \
                            0x0000000100000000, \
                            0xFFFFFFFF88000000, \
                            0x00000000000000F0)
        XBAR_CONFIG_NODE_a0(0x28, \
                            0x0000000108000000, \
                            0xFFFFFFFF88000000, \
                            0x00000000000000F1)
        XBAR_CONFIG_NODE_a0(0x30, \
                            0x0000000180000000, \
                            0xFFFFFFFF88000000, \
                            0x00000000080000F0)
        XBAR_CONFIG_NODE_a0(0x38, \
                            0x0000000188000000, \
                            0xFFFFFFFF88000000, \
                            0x00000000080000F1)
#else
#ifdef INTERLEAVE_18
	    PRINTSTR("DDR Interleave using Bit 18\r\n")
        GET_NODE_ID_a0;
        XBAR_CONFIG_NODE_a0(0x20, \
                            0x0000000100000000, \
                            0xFFFFFFFF80040000, \
                            0x00000000000000F0)
        XBAR_CONFIG_NODE_a0(0x28, \
                            0x0000000100040000, \
                            0xFFFFFFFF80040000, \
                            0x00000000000000F1)
        XBAR_CONFIG_NODE_a0(0x30, \
                            0x0000000180000000, \
                            0xFFFFFFFF80040000, \
                            0x00000000000400F0)
        XBAR_CONFIG_NODE_a0(0x38, \
                            0x0000000180040000, \
                            0xFFFFFFFF80040000, \
                            0x00000000000400F1)
#else
#ifdef INTERLEAVE_16
	    PRINTSTR("DDR Interleave using Bit 16\r\n")
        GET_NODE_ID_a0;
        XBAR_CONFIG_NODE_a0(0x20, \
                            0x0000000100000000, \
                            0xFFFFFFFF80010000, \
                            0x00000000000000F0)
        XBAR_CONFIG_NODE_a0(0x28, \
                            0x0000000100010000, \
                            0xFFFFFFFF80010000, \
                            0x00000000000000F1)
        XBAR_CONFIG_NODE_a0(0x30, \
                            0x0000000180000000, \
                            0xFFFFFFFF80010000, \
                            0x00000000000100F0)
        XBAR_CONFIG_NODE_a0(0x38, \
                            0x0000000180010000, \
                            0xFFFFFFFF80010000, \
                            0x00000000000100F1)
#else
#ifdef INTERLEAVE_13
	    PRINTSTR("DDR Interleave using Bit 13\r\n")
        GET_NODE_ID_a0;
        XBAR_CONFIG_NODE_a0(0x20, \
                            0x0000000100000000, \
                            0xFFFFFFFF80002000, \
                            0x00000000000000F0)
        XBAR_CONFIG_NODE_a0(0x28, \
                            0x0000000100002000, \
                            0xFFFFFFFF80002000, \
                            0x00000000000000F1)
        XBAR_CONFIG_NODE_a0(0x30, \
                            0x0000000180000000, \
                            0xFFFFFFFF80002000, \
                            0x00000000000020F0)
        XBAR_CONFIG_NODE_a0(0x38, \
                            0x0000000180002000, \
                            0xFFFFFFFF80002000, \
                            0x00000000000020F1)
#else
#ifdef INTERLEAVE_12
	    PRINTSTR("DDR Interleave using Bit 12\r\n")
        GET_NODE_ID_a0;
        XBAR_CONFIG_NODE_a0(0x20, \
                            0x0000000100000000, \
                            0xFFFFFFFF80001000, \
                            0x00000000000000F0)
        XBAR_CONFIG_NODE_a0(0x28, \
                            0x0000000100001000, \
                            0xFFFFFFFF80001000, \
                            0x00000000000000F1)
        XBAR_CONFIG_NODE_a0(0x30, \
                            0x0000000180000000, \
                            0xFFFFFFFF80001000, \
                            0x00000000000010F0)
        XBAR_CONFIG_NODE_a0(0x38, \
                            0x0000000180001000, \
                            0xFFFFFFFF80001000, \
                            0x00000000000010F1)
#else
#ifdef INTERLEAVE_11
	    PRINTSTR("DDR Interleave using Bit 11\r\n")
        GET_NODE_ID_a0;
        XBAR_CONFIG_NODE_a0(0x20, \
                            0x0000000100000000, \
                            0xFFFFFFFF80000800, \
                            0x00000000000000F0)
        XBAR_CONFIG_NODE_a0(0x28, \
                            0x0000000100000800, \
                            0xFFFFFFFF80000800, \
                            0x00000000000000F1)
        XBAR_CONFIG_NODE_a0(0x30, \
                            0x0000000180000000, \
                            0xFFFFFFFF80000800, \
                            0x00000000000008F0)
        XBAR_CONFIG_NODE_a0(0x38, \
                            0x0000000180000800, \
                            0xFFFFFFFF80000800, \
                            0x00000000000008F1)
#else   //default use INTERLEAVE_10
	    PRINTSTR("DDR Interleave using Bit 10\r\n")
        GET_NODE_ID_a0;
        XBAR_CONFIG_NODE_a0(0x20, \
                            0x0000000100000000, \
                            0xFFFFFFFF80000400, \
                            0x00000000000000F0)
        XBAR_CONFIG_NODE_a0(0x28, \
                            0x0000000100000400, \
                            0xFFFFFFFF80000400, \
                            0x00000000000000F1)
        XBAR_CONFIG_NODE_a0(0x30, \
                            0x0000000180000000, \
                            0xFFFFFFFF80000400, \
                            0x00000000000004F0)
        XBAR_CONFIG_NODE_a0(0x38, \
                            0x0000000180000400, \
                            0xFFFFFFFF80000400, \
                            0x00000000000004F1)
#endif
#endif
#endif
#endif
#endif
#endif
	    PRINTSTR("DDR Interleave space open : 0x100000000 - 0x1FFFFFFFF\r\n")
        b       2f
        nop
80:     //4G in each MC
#ifdef INTERLEAVE_27
	    PRINTSTR("DDR Interleave using Bit 27\r\n")
        GET_NODE_ID_a0;
        XBAR_CONFIG_NODE_a0(0x20, \
                            0x0000000200000000, \
                            0xFFFFFFFF08000000, \
                            0x00000000000000F0)
        XBAR_CONFIG_NODE_a0(0x28, \
                            0x0000000208000000, \
                            0xFFFFFFFF08000000, \
                            0x00000000000000F1)
        XBAR_CONFIG_NODE_a0(0x30, \
                            0x0000000300000000, \
                            0xFFFFFFFF08000000, \
                            0x00000000080000F0)
        XBAR_CONFIG_NODE_a0(0x38, \
                            0x0000000308000000, \
                            0xFFFFFFFF08000000, \
                            0x00000000080000F1)
#else
#ifdef INTERLEAVE_16
	    PRINTSTR("DDR Interleave using Bit 16\r\n")
        GET_NODE_ID_a0;
        XBAR_CONFIG_NODE_a0(0x20, \
                            0x0000000200000000, \
                            0xFFFFFFFF00010000, \
                            0x00000000000000F0)
        XBAR_CONFIG_NODE_a0(0x28, \
                            0x0000000200010000, \
                            0xFFFFFFFF00010000, \
                            0x00000000000000F1)
        XBAR_CONFIG_NODE_a0(0x30, \
                            0x0000000300000000, \
                            0xFFFFFFFF00010000, \
                            0x00000000000100F0)
        XBAR_CONFIG_NODE_a0(0x38, \
                            0x0000000300010000, \
                            0xFFFFFFFF00010000, \
                            0x00000000000100F1)
#else
#ifdef INTERLEAVE_13
	    PRINTSTR("DDR Interleave using Bit 13\r\n")
        GET_NODE_ID_a0;
        XBAR_CONFIG_NODE_a0(0x20, \
                            0x0000000200000000, \
                            0xFFFFFFFF00002000, \
                            0x00000000000000F0)
        XBAR_CONFIG_NODE_a0(0x28, \
                            0x0000000200002000, \
                            0xFFFFFFFF00002000, \
                            0x00000000000000F1)
        XBAR_CONFIG_NODE_a0(0x30, \
                            0x0000000300000000, \
                            0xFFFFFFFF00002000, \
                            0x00000000000020F0)
        XBAR_CONFIG_NODE_a0(0x38, \
                            0x0000000300002000, \
                            0xFFFFFFFF00002000, \
                            0x00000000000020F1)
#else
#ifdef INTERLEAVE_12
	    PRINTSTR("DDR Interleave using Bit 12\r\n")
        GET_NODE_ID_a0;
        XBAR_CONFIG_NODE_a0(0x20, \
                            0x0000000200000000, \
                            0xFFFFFFFF00001000, \
                            0x00000000000000F0)
        XBAR_CONFIG_NODE_a0(0x28, \
                            0x0000000200001000, \
                            0xFFFFFFFF00001000, \
                            0x00000000000000F1)
        XBAR_CONFIG_NODE_a0(0x30, \
                            0x0000000300000000, \
                            0xFFFFFFFF00001000, \
                            0x00000000000010F0)
        XBAR_CONFIG_NODE_a0(0x38, \
                            0x0000000300001000, \
                            0xFFFFFFFF00001000, \
                            0x00000000000010F1)
#else
#ifdef INTERLEAVE_11
	    PRINTSTR("DDR Interleave using Bit 11\r\n")
        GET_NODE_ID_a0;
        XBAR_CONFIG_NODE_a0(0x20, \
                            0x0000000200000000, \
                            0xFFFFFFFF00000800, \
                            0x00000000000000F0)
        XBAR_CONFIG_NODE_a0(0x28, \
                            0x0000000200000800, \
                            0xFFFFFFFF00000800, \
                            0x00000000000000F1)
        XBAR_CONFIG_NODE_a0(0x30, \
                            0x0000000300000000, \
                            0xFFFFFFFF00000800, \
                            0x00000000000008F0)
        XBAR_CONFIG_NODE_a0(0x38, \
                            0x0000000300000800, \
                            0xFFFFFFFF00000800, \
                            0x00000000000008F1)
#else   //default use INTERLEAVE_10
	    PRINTSTR("DDR Interleave using Bit 10\r\n")
        GET_NODE_ID_a0;
        XBAR_CONFIG_NODE_a0(0x20, \
                            0x0000000200000000, \
                            0xFFFFFFFF00000400, \
                            0x00000000000000F0)
        XBAR_CONFIG_NODE_a0(0x28, \
                            0x0000000200000400, \
                            0xFFFFFFFF00000400, \
                            0x00000000000000F1)
        XBAR_CONFIG_NODE_a0(0x30, \
                            0x0000000300000000, \
                            0xFFFFFFFF00000400, \
                            0x00000000000004F0)
        XBAR_CONFIG_NODE_a0(0x38, \
                            0x0000000300000400, \
                            0xFFFFFFFF00000400, \
                            0x00000000000004F1)
#endif
#endif
#endif
#endif
#endif
	    PRINTSTR("DDR Interleave space open : 0x200000000 - 0x3FFFFFFFF\r\n")
        b       2f
        nop
#endif
        b       2f
        nop
3:      //error: MC0_MEMSIZE != MC1_MEMSIZE
        PRINTSTR("When use MC0&MC1 interleave mode,the two MC_MEMSIZE must be equal,the L2-Xbar will not be configured!!!\r\n")
        PRINTSTR("-------------------------------------------\r\n")
        b       71f
        nop
70:     //error: memory size not in support range
        PRINTSTR("The MEMSIZE is not supported, the L2-Xbar will not be configured!!!\r\n")
        PRINTSTR("-------------------------------------------\r\n")
71:
#if 0
        GET_NODE_ID_a0;
        XBAR_CONFIG_NODE_a0(0x0, \
                            0x0, \
                            0x0, \
                            0x0)
	    PRINTSTR("!!!!!!!!!!MC space is disabled\r\n")
#endif
2:
