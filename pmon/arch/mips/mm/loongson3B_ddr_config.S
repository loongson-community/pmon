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
    v1.6    Modify for 3B which only has 1 MC(MC0).
    v1.8    Modify L2 Xbar configure manner for new pmon-kernel interface.
                ---high address always starts from 0x80000000.
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
    dli     a1, 0xff
    and     a2, a2, a1
    GET_NODE_ID_a1
    dsll    a1, a1, 3
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
1. check NODE memory size.
**************************/
    //MC0_ONLY
    GET_MC0_MEMSIZE
    beqz    a1, 70f
    nop
    dli     t5, 0x10
    bgt     a1, t5, 70f
    nop
#if 1
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
    //0x3ff00000 space need origin NODE ID
    GET_NODE_ID_a0
    dli     t0, 0x900000003ff00000
    or      t0, t0, a0
    //add 3B register location offset
    dsrl    a0, a0, 44
    dsll    a0, a0, 14
    daddu   t0, t0, a0
    //clear odd NODE_ID to even for chip config
    GET_NODE_ID_a1
    and     a1, a1, 0xe //clear bit[0]
    dli     t2, 0x900000001fe00180
    dsll    a1, a1, 44
    or      t2, t2, a1

#if 1
    PRINTSTR("\r\nt2 = 0x");
    dsrl    a0, t2, 32
    bal     hexserial
    nop
    move    a0, t2
    bal     hexserial
    nop
	PRINTSTR("\r\n")
#endif
#if 1
	/* show system config register  */
    PRINTSTR("\r\nSys Configure reg = 0x");
    ld	    t1, 0x0(t2) 
    dsrl    a0, t1, 32
    bal     hexserial
    nop
    move    a0, t1
    bal     hexserial
    nop
	PRINTSTR("\r\n")
#endif

#if 1   //necessary
    TTYDBG("Disable DDR buffer cpu\r\n")
    bal     disable_mc_read_buffer
    nop
#endif

#ifdef  AUTO_ARB_LEVEL
    bal     SET_AUTO_ARB_LEVEL_MARK
    nop
#endif

    bal     mc_init
    nop

    PRINTSTR("\r\nMC0 Config DONE\r\n")
/*****************************
 3. set msize for this NODE
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

    //disable default pci window
    L2XBAR_DISABLE_WINDOW(0x100);
/*MC0_ONLY */
    GET_NODE_ID_a0;
    XBAR_CONFIG_NODE_a0(0x10, \
                    0x0000000000000000, \
                    0xFFFFFFFFF0000000, \
                    0x00000000000000F0)
    PRINTSTR("MC0 space open : 0x00000000 - 0x0FFFFFFF\r\n")
    //Config PCI windows
    L2XBAR_CONFIG_PCI_AS_CPU(0x10);
    L2XBAR_CONFIG_PCI_BASE_0to8(0x110);
    PRINTSTR("PCI space open: 0x80000000 - 0x8FFFFFFF\r\n")

//config high memory windows
//use MC only, currently only support 512M, 1G, 2G, 3G, 4G
/* Assume MC0_ONLY */
    GET_MC0_MEMSIZE
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
    beq     a1, t5, 80f
    nop
    dli     t5, 0x10
    //temp code, MEM size >= 8G, use 8G only
    bgeu    a1, t5, 32f
    nop
    b       70f  //error condition
    nop
10:     //ddr_512MB_MC0:
    GET_NODE_ID_a0;
    XBAR_CONFIG_NODE_a0(0x20, \
                    0x0000000080000000, \
                    0xFFFFFFFFE0000000, \
                    0x00000000000000F0)
    PRINTSTR("MC0 space open : 0x80000000 - 0x9FFFFFFF\r\n")
    b       6f
    nop
20:     //ddr_1GB_MC:
    GET_NODE_ID_a0;
    XBAR_CONFIG_NODE_a0(0x20, \
                    0x0000000080000000, \
                    0xFFFFFFFFC0000000, \
                    0x00000000000000F0)
    PRINTSTR("MC0 space open : 0x80000000 - 0xBFFFFFFF\r\n")
    b       6f
    nop
40:     //ddr_2GB_MC0:
    GET_NODE_ID_a0;
    XBAR_CONFIG_NODE_a0(0x20, \
                    0x0000000080000000, \
                    0xFFFFFFFF80000000, \
                    0x00000000000000F0)
    PRINTSTR("MC0 space open : 0x80000000 - 0xFFFFFFFF\r\n")
    b       6f
    nop
60:     //ddr_3GB_MC0:
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
    b       6f
    nop
80:     //ddr_4GB_MC0:
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
    b       6f
    nop

32:     //ddr_8GB_MC0:
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
    b       6f
    nop

6:
    //Config other PCI space exactly as cpu windows
    L2XBAR_CONFIG_PCI_AS_CPU(0x20);
    L2XBAR_CONFIG_PCI_AS_CPU(0x28);
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
