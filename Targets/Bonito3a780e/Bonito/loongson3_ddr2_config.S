/*whd : loongson3_ddr2_config.S
        used to set up all ddr controllers
        and set up the memory space on L2 Xbar
*/

#######################################################
/* Undefine the two to enable both */
/* !!!NOT USE ANYMORE !!!!! */
#define MC0_ONLY
//#define MC1_ONLY

/* Size of each DDR controller */
/* !!!NOT USE ANYMORE !!!!! */
//#define DDR_512
//#define DDR_1G
#define DDR_2G

/* Only DDR_1G CAN USE INTERLEAVE by now           */
/* Interleave pattern when both controller enabled */
//#define NO_INTERLEAVE
//#define INTERLEAVE_27
#define INTERLEAVE_13
//#define INTERLEAVE_12
//#define INTERLEAVE_11
#######################################################

/***********************************************************
 s1: 
|[30:28]| MC1_ROW            | MC0_ROW | 15 - MC1_ROW_SIZE |
|[27:27]| MC1_EIGHT_BANK     | 1'b0    | FOUR  BANKS       |
|       |                    | 1'b1    | EIGHT BANKS       |
|[26:24]| MC1_COL            | MC0_COL | 14 - MC1_COL_SIZE |
|[22:20]| MC0_ROW            | MC0_ROW | 15 - MC0_ROW_SIZE |
|[19:19]| MC0_EIGHT_BANK     | 1'b0    | FOUR  BANKS       |
|       |                    | 1'b1    | EIGHT BANKS       |
|[18:16]| MC0_COL_SIZE       | MC0_COL | 14 - COL_SIZE     |
|[15:12]| MC1_CS_MAP         |         |                   |
|[11: 8]| MC0_CS_MAP         |         |                   |
|[ 7: 7]| DIMM_TYPE          | 1'b0    | DDR2              |
|       |                    | 1'b1    | DDR3              |
|[ 6: 4]| SIZE_PER_CONTROLLER| 3'b001  | 512MB             |
|       |                    | 3'b010  | 1G                |
|       |                    | 3'b011  | 2G                |
|[ 3: 2]| CONTROLLER_SELECT  | 2'b00   | USE BOTH          |
|       |                    | 2'b01   | MC0_ONLY          |
|       |                    | 2'b10   | MC1_ONLY          |
|[ 1: 0]| NODE ID            |         |                   |
***********************************************************/
#define GET_NODE_ID_a0  dli a0, 0x00000003; and a0, s1, a0; dsll a0, 44;
#define GET_MC0_ONLY    dli a0, 0x00000004; and a0, s1, a0;
#define GET_MC1_ONLY    dli a0, 0x00000008; and a0, s1, a0;
#define GET_DDR_SIZE    dli a0, 0x00000070; and a0, s1, a0;
#define GET_DDR_TYPE    dli a1, 0x00000080; and a1, s1, a1;
#define GET_MC0_CS_MAP  dli a1,   0x00000f00; and a1, s1, a1; dsll a1, 8
#define GET_MC1_CS_MAP  dli a1,   0x0000f000; and a1, s1, a1; dsll a1, 4;
#define GET_MC0_EIGHT   dli a1, 0x00080000; and a1, s1, a1; dsll a1, 13;
#define GET_MC0_ROW     dli a1, 0x00700000; and a1, s1, a1; dsrl a1, 12;
#define GET_MC0_COL     dli a1, 0x00070000; and a1, s1, a1; dsll a1, 8;
#define GET_MC1_EIGHT   dli a1, 0x08000000; and a1, s1, a1; dsll a1, 5;
#define GET_MC1_ROW     dli a1, 0x70000000; and a1, s1, a1; dsrl a1, 20;
#define GET_MC1_COL     dli a1, 0x07000000; and a1, s1, a1;
#define XBAR_CONFIG_NODE_a0(OFFSET, BASE, MASK, MMAP) \
                        daddi   v0, t0, OFFSET;       \
                        dli     t1, BASE;             \
                        or      t1, t1, a0;           \
                        sd      t1, 0x00(v0);         \
                        dli     t1, MASK;             \
                        sd      t1, 0x40(v0);         \
                        dli     t1, MMAP;             \
                        sd      t1, 0x80(v0);
#######################################################
        
        GET_NODE_ID_a0
        dli     t2, 0x900000001fe00180
        dli     t0, 0x900000003ff00000
        or      t2, t2, a0
        or      t0, t0, a0

#ONLY NODE 0 USEs msize
        bnez    a0, 3f
        nop

        GET_DDR_SIZE
        dli     a1, 0x10
        beq     a1, a0, ddr_512MB
        nop
        dli     a1, 0x20
        beq     a1, a0, ddr_1GB
        nop
ddr_2GB:
        /*
        b       2f
	    //li	    msize, 0x7f000000
	    li	    msize, 0x8
        */
        GET_MC0_ONLY
        beqz    a0, 1f
        nop
	    //li	    msize, 0x3f000000
	    li	    msize, 0x8
        b       3f
        nop
1:
        GET_MC1_ONLY
        beqz    a0, 1f
        nop
	    //li	    msize, 0x3f000000
	    li	    msize, 0x8
        b       3f
        nop
1:
	    //li	    msize, 0x7f000000
	    li	    msize, 0x10
        b    3f
        nop

ddr_512MB:
        /*
        b       2f
	    //li	    msize, 0x1f000000
	    li	    msize, 0x2
        */
        GET_MC0_ONLY
        beqz    a0, 1f
        nop
	    //li	    msize, 0x3f000000
	    li	    msize, 0x2
        b       3f
        nop
1:
        GET_MC1_ONLY
        beqz    a0, 1f
        nop
	    //li	    msize, 0x3f000000
	    li	    msize, 0x2
        b       3f
        nop
1:
	    //li	    msize, 0x7f000000
	    li	    msize, 0x4
        b        3f
        nop

ddr_1GB:
        GET_MC0_ONLY
        beqz    a0, 1f
        nop
	    //li	    msize, 0x3f000000
	    li	    msize, 0x4
        b       3f
        nop
1:
        GET_MC1_ONLY
        beqz    a0, 1f
        nop
	    //li	    msize, 0x3f000000
	    li	    msize, 0x4
        b       3f
        nop
1:
	    //li	    msize, 0x7f000000
	    li	    msize, 0x8
	    nop
3:

/* RESERVE MEMORY FOR VRAM */
        //li      a0, 0x00000000
//#if (SHARED_VRAM == 64)
//        li      a0, 0x04000000
//#elif (SHARED_VRAM == 32)
//        li      a0, 0x02000000
//#endif
        //sub     msize, msize, a0

//3:

###Enable the reg space###
#if 1
	TTYDBG	("\r\nEnable register space of MEMORY\r\n")
        #li  t2, 0xbfe00180
        #dli t2, 0x900000001fe00180
        lw  a1, 0x0(t2)
        li  a0, 0xfffffeff
        and a1, a1,a0
        sw  a1, 0x0(t2)
#endif    

#if 1
//#ifdef AUTO_DDR_CONFIG
        GET_MC0_CS_MAP
        beqz    a1, 1f
        nop
//#endif
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
1:
#endif

#if 1
#ifdef AUTO_DDR_CONFIG
        GET_MC1_CS_MAP
        beqz    a1, 1f
        nop
#endif
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
1:
#endif

/* MC0_ONLY */
        GET_MC0_ONLY
        beqz    a0, 1f
        nop
	    PRINTSTR("MC0 space open : 0x00000000 - 0x0FFFFFFF\r\n")

        GET_NODE_ID_a0;
        XBAR_CONFIG_NODE_a0(0x10, \
                            0x0000000000000000, \
                            0xfffffffff0000000, \
                            0x00000000000000f0)

        b       2f
        nop
1:
/* MC1_ONLY */
        GET_MC1_ONLY
        beqz    a0, 1f
        nop
	    PRINTSTR("MC1 space open : 0x00000000 - 0x0FFFFFFF\r\n")

        GET_NODE_ID_a0;
        XBAR_CONFIG_NODE_a0(0x10, \
                            0x0000000000000000, \
                            0xfffffffff0000000, \
                            0x00000000000000f1)

        b       2f
        nop
1:
#ifdef NO_INTERLEAVE
	    PRINTSTR("DDR space open : 0x00000000 - 0x0FFFFFFF\r\n")

        GET_NODE_ID_a0;
        XBAR_CONFIG_NODE_a0(0x10, \
                            0x0000000000000000, \
                            0xfffffffff0000000, \
                            0x00000000000000f0)

#else
#ifdef INTERLEAVE_27
	    PRINTSTR("DDR Interleave space open : 0x00000000 - 0x0FFFFFFF\r\n")
	    PRINTSTR("DDR Interleave using Bit 27\r\n")

        GET_NODE_ID_a0;
        XBAR_CONFIG_NODE_a0(0x10, \
                            0x0000000000000000, \
                            0xfffffffff8000000, \
                            0x00000000000000f0)
        XBAR_CONFIG_NODE_a0(0x18, \
                            0x0000000008000000, \
                            0xfffffffff8000000, \
                            0x00000000000000f1)

#else
#ifdef INTERLEAVE_13
	    PRINTSTR("DDR Interleave space open : 0x00000000 - 0x0FFFFFFF\r\n")
	    PRINTSTR("DDR Interleave using Bit 13\r\n")

        GET_NODE_ID_a0;
        XBAR_CONFIG_NODE_a0(0x10, \
                            0x0000000000000000, \
                            0xfffffffff0002000, \
                            0x00000000000000f0)
        XBAR_CONFIG_NODE_a0(0x18, \
                            0x0000000000002000, \
                            0xfffffffff0002000, \
                            0x00000000000000f1)

#else
#ifdef INTERLEAVE_12
	    PRINTSTR("DDR Interleave space open : 0x00000000 - 0x0FFFFFFF\r\n")
	    PRINTSTR("DDR Interleave using Bit 12\r\n")

        GET_NODE_ID_a0;
        XBAR_CONFIG_NODE_a0(0x10, \
                            0x0000000000000000, \
                            0xfffffffff0001000, \
                            0x00000000000000f0)
        XBAR_CONFIG_NODE_a0(0x18, \
                            0x0000000000001000, \
                            0xfffffffff0001000, \
                            0x00000000000000f1)

#else
#ifdef INTERLEAVE_11
	    PRINTSTR("DDR Interleave space open : 0x00000000 - 0x0FFFFFFF\r\n")
	    PRINTSTR("DDR Interleave using Bit 11\r\n")

        GET_NODE_ID_a0;
        XBAR_CONFIG_NODE_a0(0x10, \
                            0x0000000000000000, \
                            0xfffffffff0000800, \
                            0x00000000000000f0)
        XBAR_CONFIG_NODE_a0(0x18, \
                            0x0000000000000800, \
                            0xfffffffff0000800, \
                            0x00000000000000f1)

#else
	    PRINTSTR("DDR Interleave space open : 0x00000000 - 0x0FFFFFFF\r\n")

        GET_NODE_ID_a0;
        XBAR_CONFIG_NODE_a0(0x10, \
                            0x0000000000000000, \
                            0xfffffffff0000400, \
                            0x00000000000000f0)
        XBAR_CONFIG_NODE_a0(0x18, \
                            0x0000000000000400, \
                            0xfffffffff0000400, \
                            0x00000000000000f1)

#endif
#endif
#endif
#endif
#endif

2:


/* MC0_ONLY */
        GET_MC0_ONLY
        beqz    a0, 1f
        nop

        GET_DDR_SIZE
        dli     a1, 0x10
        beq     a1, a0, ddr_512MB_MC0
        nop
        dli     a1, 0x20
        beq     a1, a0, ddr_1GB_MC0
        nop
ddr_2GB_MC0:
	    PRINTSTR("MC0 space open : 0x1_1000_0000 - 0x1_7FFF_FFFF\r\n")

        GET_NODE_ID_a0;
        XBAR_CONFIG_NODE_a0(0x20, \
                            0x0000000100000000, \
                            0xffffffff80000000, \
                            0x00000000000000f0)
        b       2f
        nop
ddr_1GB_MC0:
	    PRINTSTR("MC0 space open : 0x1_1000_0000 - 0x1_3FFF_FFFF\r\n")

        GET_NODE_ID_a0;
        XBAR_CONFIG_NODE_a0(0x20, \
                            0x0000000100000000, \
                            0xffffffffc0000000, \
                            0x00000000000000f0)
        b       2f
        nop
ddr_512MB_MC0:
	    PRINTSTR("MC0 pace open : 0x1_1000_0000 - 0x1_1FFF_FFFF\r\n")

        GET_NODE_ID_a0;
        XBAR_CONFIG_NODE_a0(0x20, \
                            0x0000000100000000, \
                            0xffffffffe0000000, \
                            0x00000000000000f0)
        b       2f
        nop

1:
/* MC1_ONLY */
        GET_MC1_ONLY
        beqz    a0, 1f
        nop

        GET_DDR_SIZE
        dli     a1, 0x10
        beq     a1, a0, ddr_512MB_MC1
        nop
        dli     a1, 0x20
        beq     a1, a0, ddr_1GB_MC1
        nop
ddr_2GB_MC1:
	    PRINTSTR("MC1 space open : 0x1_1000_0000 - 0x1_7FFF_FFFF\r\n")

        GET_NODE_ID_a0;
        XBAR_CONFIG_NODE_a0(0x20, \
                            0x0000000100000000, \
                            0xffffffff80000000, \
                            0x00000000000000f1)
        b       3f
        nop
ddr_1GB_MC1:
	    PRINTSTR("MC1 space open : 0x1_1000_0000 - 0x1_3FFF_FFFF\r\n")

        GET_NODE_ID_a0;
        XBAR_CONFIG_NODE_a0(0x20, \
                            0x0000000100000000, \
                            0xffffffffc0000000, \
                            0x00000000000000f1)
        b       3f
        nop
ddr_512MB_MC1:
	    PRINTSTR("MC1 space open : 0x1_1000_0000 - 0x1_1FFF_FFFF\r\n")

        GET_NODE_ID_a0;
        XBAR_CONFIG_NODE_a0(0x20, \
                            0x0000000100000000, \
                            0xffffffffe0000000, \
                            0x00000000000000f1)
3:
	    PRINTSTR("PCI DMA SPACE reassigned to MC1\r\n")
        dli     t1, 0x00000000000000f1
        sd      t1, 0x180(t0)

        b       2f
        nop

1:
/* ONLY 1GB has INTERLEAVE mode by now */
#ifdef NO_INTERLEAVE
	    PRINTSTR("DDR space open : 0x80000000 - 0xFFFFFFFF\r\n")

        GET_NODE_ID_a0;
        XBAR_CONFIG_NODE_a0(0x20, \
                            0x0000000080000000, \
                            0xffffffffc0000000, \
                            0x00000000000000f0)
        XBAR_CONFIG_NODE_a0(0x38, \
                            0x00000000c0000000, \
                            0xffffffffc0000000, \
                            0x00000000000000f1)

	    PRINTSTR("PCI DDR space open : 0x80000000 - 0xFFFFFFFF\r\n")

        // Disable the default Windows
        dli     t1, 0x0
        sd      t1, 0x180(t0)

        // Enable others
        GET_NODE_ID_a0;
        XBAR_CONFIG_NODE_a0(0x120, \
                            0x0000000080000000, \
                            0xffffffffc0000000, \
                            0x00000000000000f0)
        XBAR_CONFIG_NODE_a0(0x138, \
                            0x00000000c0000000, \
                            0xffffffffc0000000, \
                            0x00000000000000f1)

#else
#ifdef INTERLEAVE_27
	    PRINTSTR("DDR Interleave space open : 0x80000000 - 0xFFFFFFFF\r\n")
	    PRINTSTR("DDR Interleave using Bit 27\r\n")

        GET_NODE_ID_a0;
        XBAR_CONFIG_NODE_a0(0x20, \
                            0x0000000080000000, \
                            0xffffffffc8000000, \
                            0x00000000000000f0)
        XBAR_CONFIG_NODE_a0(0x28, \
                            0x0000000088000000, \
                            0xffffffffc8000000, \
                            0x00000000000000f1)
        XBAR_CONFIG_NODE_a0(0x30, \
                            0x00000000c0000000, \
                            0xffffffffc8000000, \
                            0x00000000080000f0)
        XBAR_CONFIG_NODE_a0(0x38, \
                            0x00000000c8000000, \
                            0xffffffffc8000000, \
                            0x00000000080000f1)

#else
#ifdef INTERLEAVE_13
	beq msize, 0x10, INTERLEAVE_2GB	
	nop

	beq msize, 0x4, INTERLEAVE_512MB	
	nop

INTERLEAVE_1GB:  // 1Gx2
	PRINTSTR("DDR Interleave space open : 0x1_1000_0000 - 0x1_7FFF_FFFF\r\n")
	PRINTSTR("DDR Interleave using Bit 13\r\n")

        GET_NODE_ID_a0;
        XBAR_CONFIG_NODE_a0(0x20, \
                            0x0000000100000000, \
                            0xffffffffc0002000, \
                            0x00000000000000f0)
        XBAR_CONFIG_NODE_a0(0x28, \
                            0x0000000100002000, \
                            0xffffffffc0002000, \
                            0x00000000000000f1)
        XBAR_CONFIG_NODE_a0(0x30, \
                            0x0000000140000000, \
                            0xffffffffc0002000, \
                            0x00000000000020f0)
        XBAR_CONFIG_NODE_a0(0x38, \
                            0x0000000140002000, \
                            0xffffffffc0002000, \
                            0x00000000000020f1)
	b 1f

INTERLEAVE_512MB:  // 512MBx2
	PRINTSTR("DDR Interleave space open : 0x1_1000_0000 - 0x1_3FFF_FFFF\r\n")
	PRINTSTR("DDR Interleave using Bit 13\r\n")

        GET_NODE_ID_a0;
        XBAR_CONFIG_NODE_a0(0x20, \
                            0x0000000100000000, \
                            0xffffffffe0002000, \
                            0x00000000000000f0)
        XBAR_CONFIG_NODE_a0(0x28, \
                            0x0000000100002000, \
                            0xffffffffe0002000, \
                            0x00000000000000f1)
        XBAR_CONFIG_NODE_a0(0x30, \
                            0x0000000120000000, \
                            0xffffffffe0002000, \
                            0x00000000000020f0)
        XBAR_CONFIG_NODE_a0(0x38, \
                            0x0000000120002000, \
                            0xffffffffe0002000, \
                            0x00000000000020f1)

	b 1f

INTERLEAVE_2GB: // 2Gx2
	PRINTSTR("DDR Interleave space open : 0x1_1000_0000 - 0x1_FFFF_FFFF\r\n")
	PRINTSTR("DDR Interleave using Bit 13\r\n")

	GET_NODE_ID_a0;
        XBAR_CONFIG_NODE_a0(0x20, \
                            0x0000000100000000, \
                            0xffffffff80002000, \
                            0x00000000000000f0)
        XBAR_CONFIG_NODE_a0(0x28, \
                            0x0000000100002000, \
                            0xffffffff80002000, \
                            0x00000000000000f1)
        XBAR_CONFIG_NODE_a0(0x30, \
                            0x0000000180000000, \
                            0xffffffff80002000, \
                            0x00000000000020f0)
        XBAR_CONFIG_NODE_a0(0x38, \
                            0x0000000180002000, \
                            0xffffffff80002000, \
                            0x00000000000020f1)
	b 1f

1:
#else
#ifdef INTERLEAVE_12
	    PRINTSTR("DDR Interleave space open : 0x80000000 - 0xFFFFFFFF\r\n")
	    PRINTSTR("DDR Interleave using Bit 12\r\n")

        GET_NODE_ID_a0;
        XBAR_CONFIG_NODE_a0(0x20, \
                            0x0000000080000000, \
                            0xffffffffc0001000, \
                            0x00000000000000f0)
        XBAR_CONFIG_NODE_a0(0x28, \
                            0x0000000080001000, \
                            0xffffffffc0001000, \
                            0x00000000000000f1)
        XBAR_CONFIG_NODE_a0(0x30, \
                            0x00000000c0000000, \
                            0xffffffffc0001000, \
                            0x00000000000010f0)
        XBAR_CONFIG_NODE_a0(0x38, \
                            0x00000000c0001000, \
                            0xffffffffc0001000, \
                            0x00000000000010f1)

#else
#ifdef INTERLEAVE_11
	    PRINTSTR("DDR Interleave space open : 0x80000000 - 0xFFFFFFFF\r\n")
	    PRINTSTR("DDR Interleave using Bit 11\r\n")

        GET_NODE_ID_a0;
        XBAR_CONFIG_NODE_a0(0x20, \
                            0x0000000080000000, \
                            0xffffffffc0000800, \
                            0x00000000000000f0)
        XBAR_CONFIG_NODE_a0(0x28, \
                            0x0000000080000800, \
                            0xffffffffc0000800, \
                            0x00000000000000f1)
        XBAR_CONFIG_NODE_a0(0x30, \
                            0x00000000c0000000, \
                            0xffffffffc0000800, \
                            0x00000000000008f0)
        XBAR_CONFIG_NODE_a0(0x38, \
                            0x00000000c0000800, \
                            0xffffffffc0000800, \
                            0x00000000000008f1)

	    PRINTSTR("PCI DDR Interleave space open : 0x80000000 - 0xFFFFFFFF\r\n")
	    PRINTSTR("PCI DDR Interleave using Bit 11\r\n")

        // Disable the default Windows
        dli     t1, 0x0
        sd      t1, 0x180(t0)

        // Enable others
        GET_NODE_ID_a0;
        XBAR_CONFIG_NODE_a0(0x120, \
                            0x0000000080000000, \
                            0xffffffffc0000800, \
                            0x00000000000000f0)
        XBAR_CONFIG_NODE_a0(0x128, \
                            0x0000000080000800, \
                            0xffffffffc0000800, \
                            0x00000000000000f1)
        XBAR_CONFIG_NODE_a0(0x130, \
                            0x00000000c0000000, \
                            0xffffffffc0000800, \
                            0x00000000000008f0)
        XBAR_CONFIG_NODE_a0(0x138, \
                            0x00000000c0000800, \
                            0xffffffffc0000800, \
                            0x00000000000008f1)

#else
	    PRINTSTR("DDR Interleave space open : 0x80000000 - 0xFFFFFFFF\r\n")

        GET_NODE_ID_a0;
        XBAR_CONFIG_NODE_a0(0x20, \
                            0x0000000080000000, \
                            0xffffffffc0000400, \
                            0x00000000000000f0)
        XBAR_CONFIG_NODE_a0(0x28, \
                            0x0000000080000400, \
                            0xffffffffc0000400, \
                            0x00000000000000f1)
        XBAR_CONFIG_NODE_a0(0x30, \
                            0x00000000c0000000, \
                            0xffffffffc0000400, \
                            0x00000000000004f0)
        XBAR_CONFIG_NODE_a0(0x38, \
                            0x00000000c0000400, \
                            0xffffffffc0000400, \
                            0x00000000000004f1)

#endif
#endif
#endif
#endif
#endif
2:

#if 0 //print registers
    //li      t1, 152 ##0x72
    li      t1, 180 ##0x72
    li      t2, 0xaff00000

reg_read:
    ld      a0, 0x0(t2)

    //bal	    hexserial
    bal	    hexserial64
    nop
    PRINTSTR("\r\n")

    addiu   t2, t2, 16
    bnez    t1, reg_read
    addiu   t1, t1, -1

#endif


#if 0 /* read ddr2 registers */
/* No use in Loongson 3A */
        li	t0, 0xaff00000
        
not_locked:
        ld	t1, 0x10(t0)
        andi    t1, 0x01
        beqz    t1, not_locked
        nop

        PRINTSTR("DDR2 DLL locked\r\n")
        
        ld	    t1, 0xf0(t0)
        move    a0, t1
        bal     hexserial
        nop

#endif
        
###disable the reg space###
#if 1
	    TTYDBG("Disable register space of MEMORY\r\n")
        #li  t2,0xbfe00180
        #dli t2,0x900000001fe00180
        lw  a1,0x0(t2)
        or  a1,a1,0x100
        sw  a1,0x0(t2)
#endif

#if 0 // AdonWang disable ddr3 readbuff
/*      May Affect the Performance     */
       TTYDBG("Disable read buffer\r\n")
       #dli     t2, 0x900000001fe00180
       lw      t1, 0x4(t2)
       li      a0, 0x18
       or      t1, t1, a0
       sw      t1, 0x4(t2)
#endif

#if 1 // AdonWang disable cpu buffered read
/* !!!!!!!!!!!!! IMPORTANT !!!!!!!!!!!! */
       TTYDBG("Disable cpu buffered read\r\n")
       #dli     t2, 0x900000001fe00180 
       lw      t1, 0x0(t2)
       li      a0, 0xfffffdff
       and     t1, t1, a0
       sw      t1, 0x0(t2)
#endif
