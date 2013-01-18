/*whd : loongson3_ddr2_config.S
        used to set up all ddr controllers
        and set up the memory space on L2 Xbar
*/

#######################################################
/* Undefine the two to enable both */
/* !!!NOT USE ANYMORE !!!!! */
///////#define MC0_ONLY
///////#define MC1_ONLY

/* Size of each DDR controller */
/* !!!NOT USE ANYMORE !!!!! */
//#define DDR_512
//#define DDR_1G
//#define DDR_2G

/* Only DDR_1G CAN USE INTERLEAVE by now           */
/* Interleave pattern when both controller enabled */
/* NOTICE HERE: bit 10 is used to be interleave bit if no interleave bit is assigned */ 
//#define NO_INTERLEAVE
//#define INTERLEAVE_27
#define INTERLEAVE_13
//#define INTERLEAVE_12
//#define INTERLEAVE_11

#######################################################

/***********************************************************
| s1:						 |		   |				   |						   |
|[36:36]| FREQ_ADDAPTER      | 1'b1    |				   |	
|       |				     | 1'b0    | Enable freq adapt |	
|       |					 | 1'b1    | Disable freq adapt|  	
|[35:35]| MC1_DIMM_TYPE      | 1'b1    | Registered Dual   |	
|       |                    | 1'b0    | Unbuffered Dual   |
|[34:34]| MC1_ECC            | 1'b1    | WITH DATA ECC     |	
|       |                    | 1'b0    | NO DATA ECC       |
|[33:33]| MC0_DIMM_TYPE      | 1'b1    | Registered Dual   |	
|       |                    | 1'b0    | Unbuffered Dual   |
|[32:32]| MC0_ECC            | 1'b1    | WITH DATA ECC     |	
|       |                    | 1'b0    | NO	  DATA ECC     |
|[31:31]|                    | 1'b0    | NOT USED          |
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
|[ 7: 7]| DDR_TYPE           | 1'b0    | DDR2              |
|       |                    | 1'b1    | DDR3              |
|[ 6: 4]| SIZE_PER_CONTROLLER| 3'b001  | 512MB             |
|       |                    | 3'b010  | 1G                |
|       |                    | 3'b011  | 2G                |
|       |                    | 3'b100  | 4G                |
|[ 3: 2]| CONTROLLER_SELECT  | 2'b00   | USE BOTH          |
|       |                    | 2'b01   | MC0_ONLY          |
|       |                    | 2'b10   | MC1_ONLY          |
|[ 1: 0]| NODE ID            |         |                   |
***********************************************************/
#define GET_NODE_ID_a0  dli a0, 0x00000003; and a0, s1, a0; dsll a0, 44;
#define GET_MC0_ONLY    dli a0, 0x00000004; and a0, s1, a0;
#define GET_MC1_ONLY    dli a0, 0x00000008; and a0, s1, a0;
#define GET_DDR_ADAPTER dli a0, 0x1000000000; and a0, s1, a0;
#define GET_DDR_SIZE    dli a0, 0x00000070; and a0, s1, a0;
#define GET_DDR_TYPE    dli a1, 0x00000080; and a1, s1, a1;
#define GET_MC0_CS_MAP  dli a1, 0x00000f00; and a1, s1, a1; dsll a1, 8
#define GET_MC1_CS_MAP  dli a1, 0x0000f000; and a1, s1, a1; dsll a1, 4;
#define GET_MC0_EIGHT   dli a1, 0x00080000; and a1, s1, a1; dsll a1, 13;
#define GET_MC0_ROW     dli a1, 0x00700000; and a1, s1, a1; dsrl a1, 12;
#define GET_MC0_COL     dli a1, 0x00070000; and a1, s1, a1; dsll a1, 8;
#define GET_MC1_EIGHT   dli a1, 0x08000000; and a1, s1, a1; dsll a1, 5;
#define GET_MC1_ROW     dli a1, 0x70000000; and a1, s1, a1; dsrl a1, 20;
#define GET_MC1_COL     dli a1, 0x07000000; and a1, s1, a1;
#define GET_MC0_ECC     dli a1, 0x100000000; and a1, s1, a1; dsrl a1, 32;
#define GET_MC0_DIMM    dli a1, 0x200000000; and a1, s1, a1; dsrl a1, 33;
#define GET_MC1_ECC     dli a1, 0x400000000; and a1, s1, a1; dsrl a1, 34;
#define GET_MC1_DIMM    dli a1, 0x800000000; and a1, s1, a1; dsrl a1, 35;
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

###Set DDR frequency ### 
#if 1 
		/* software ddr clk_sel can take effect when hardware ddrclk is in bypass mode */
        dli     t2, 0x900000001fe00180
        lw  a1, 0x14(t2)
        li  a0, 0x000003e0
        and a1, a1, a0
        bne a0, a1, no_software_ddrclksel
        nop 

        TTYDBG  ("\r\nSet DDR frequency by software\r\n")
        dli t2, 0x900000001fe00180

		/* clear ddr_clksel */
        lw  a1, 0x0(t2)
        li  a0, 0xe0ffffff
        and a1, a1,a0

		/* set ddr_clksel according memory size and scatter in DIMMS */
		// NOTICE HERE: autocalily decrease ddr freq only when two ddr in one channel 
		// if decrease freq     : ddr_clksel = 5'10110 means X09
		// if not decrease freq : ddr_clksel = 5'00110 means X12

		GET_DDR_ADAPTER 
		beqz  a0, 1f
		nop
        li  a0, 0x16000000 // ddr_freq X 0x09
		b	2f
		nop
1:
        li  a0, 0x06000000 // ddr_freq X 0x12
2:
        or  a1, a1, a0
        sw  a1, 0x0(t2)

		/* enable software ddr_clksel */
        li  a0, 0x00000008
        or  a1, a1, a0
        sw  a1, 0x0(t2)

no_software_ddrclksel:

#endif
        
        GET_NODE_ID_a0
        dli     t2, 0x900000001fe00180
        dli     t0, 0x900000003ff00000
        or      t2, t2, a0
        or      t0, t0, a0

#ONLY NODE 0 USEs msize
        bnez    a0, 3f
        nop

#if 0
        GET_DDR_SIZE
        dli     a1, 0x10
        beq     a1, a0, ddr_512MB
        nop
        dli     a1, 0x20
        beq     a1, a0, ddr_1GB
        nop
ddr_2GB:
        b       2f
	    li	    msize, 0x7f000000

ddr_512MB:
        b       2f
	    li	    msize, 0x1f000000

ddr_1GB:
        GET_MC0_ONLY
        beqz    a0, 1f
        nop
	    li	    msize, 0x3f000000
        b       2f
        nop
1:
        GET_MC1_ONLY
        beqz    a0, 1f
        nop
	    li	    msize, 0x3f000000
        b       2f
        nop
1:
	    li	    msize, 0x7f000000
2:

/* RESERVE MEMORY FOR VRAM */
        li      a0, 0x00000000
//#if (SHARED_VRAM == 64)
//        li      a0, 0x04000000
//#elif (SHARED_VRAM == 32)
//        li      a0, 0x02000000
//#endif
        sub     msize, msize, a0
#endif

3:

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
#ifdef AUTO_DDR_CONFIG
        GET_MC0_CS_MAP
        beqz    a1, 1f
        nop
#endif
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
	    PRINTSTR(" DDR and PCI Interleave space open : 0x00000000 - 0x0FFFFFFF\r\n")
	    PRINTSTR(" DDR and PCI Interleave using Bit 27\r\n")

        GET_NODE_ID_a0;
        XBAR_CONFIG_NODE_a0(0x10, \
                            0x0000000000000000, \
                            0xfffffffff8000000, \
                            0x00000000000000f0)
        XBAR_CONFIG_NODE_a0(0x18, \
                            0x0000000008000000, \
                            0xfffffffff8000000, \
                            0x00000000000000f1)

        XBAR_CONFIG_NODE_a0(0x110, \
                            0x0000000000000000, \
                            0xfffffffff8000000, \
                            0x00000000000000f0)
        XBAR_CONFIG_NODE_a0(0x118, \
                            0x0000000008000000, \
                            0xfffffffff8000000, \
                            0x00000000000000f1)

#else
#ifdef INTERLEAVE_13
	    PRINTSTR(" DDR and PCI Interleave space open : 0x00000000 - 0x0FFFFFFF\r\n")
	    PRINTSTR(" DDR and PCI Interleave using Bit 13\r\n")

        GET_NODE_ID_a0;
        XBAR_CONFIG_NODE_a0(0x10, \
                            0x0000000000000000, \
                            0xfffffffff0002000, \
                            0x00000000000000f0)
        XBAR_CONFIG_NODE_a0(0x18, \
                            0x0000000000002000, \
                            0xfffffffff0002000, \
                            0x00000000000000f1)

        XBAR_CONFIG_NODE_a0(0x110, \
                            0x0000000000000000, \
                            0xfffffffff0002000, \
                            0x00000000000000f0)
        XBAR_CONFIG_NODE_a0(0x118, \
                            0x0000000000002000, \
                            0xfffffffff0002000, \
                            0x00000000000000f1)

#else
#ifdef INTERLEAVE_12
	    PRINTSTR(" DDR and PCI Interleave space open : 0x00000000 - 0x0FFFFFFF\r\n")
	    PRINTSTR(" DDR and PCI Interleave using Bit 12\r\n")

        GET_NODE_ID_a0;
        XBAR_CONFIG_NODE_a0(0x10, \
                            0x0000000000000000, \
                            0xfffffffff0001000, \
                            0x00000000000000f0)
        XBAR_CONFIG_NODE_a0(0x18, \
                            0x0000000000001000, \
                            0xfffffffff0001000, \
                            0x00000000000000f1)

        XBAR_CONFIG_NODE_a0(0x110, \
                            0x0000000000000000, \
                            0xfffffffff0001000, \
                            0x00000000000000f0)
        XBAR_CONFIG_NODE_a0(0x118, \
                            0x0000000000001000, \
                            0xfffffffff0001000, \
                            0x00000000000000f1)

#else
#ifdef INTERLEAVE_11
	    PRINTSTR(" DDR and PCI Interleave space open : 0x00000000 - 0x0FFFFFFF\r\n")
	    PRINTSTR(" DDR and PCI Interleave using Bit 11\r\n")

        GET_NODE_ID_a0;
        XBAR_CONFIG_NODE_a0(0x10, \
                            0x0000000000000000, \
                            0xfffffffff0000800, \
                            0x00000000000000f0)
        XBAR_CONFIG_NODE_a0(0x18, \
                            0x0000000000000800, \
                            0xfffffffff0000800, \
                            0x00000000000000f1)

        XBAR_CONFIG_NODE_a0(0x110, \
                            0x0000000000000000, \
                            0xfffffffff0000800, \
                            0x00000000000000f0)
        XBAR_CONFIG_NODE_a0(0x118, \
                            0x0000000000000800, \
                            0xfffffffff0000800, \
                            0x00000000000000f1)

#else
	    PRINTSTR(" DDR and PCI Interleave space open : 0x00000000 - 0x0FFFFFFF\r\n")
	    PRINTSTR(" DDR and PCI Interleave using Bit 10\r\n")

        GET_NODE_ID_a0;
        XBAR_CONFIG_NODE_a0(0x10, \
                            0x0000000000000000, \
                            0xfffffffff0000400, \
                            0x00000000000000f0)
        XBAR_CONFIG_NODE_a0(0x18, \
                            0x0000000000000400, \
                            0xfffffffff0000400, \
                            0x00000000000000f1)

        XBAR_CONFIG_NODE_a0(0x110, \
                            0x0000000000000000, \
                            0xfffffffff0000400, \
                            0x00000000000000f0)
        XBAR_CONFIG_NODE_a0(0x118, \
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
        dli     a1, 0x30
        beq     a1, a0, ddr_2GB_MC0
        nop

ddr_4GB_MC0:
	    PRINTSTR("MC0 space open : 0x100000000 - 0x1FFFFFFFF\r\n")

        GET_NODE_ID_a0;
        XBAR_CONFIG_NODE_a0(0x20, \
                            0x0000000100000000, \
                            0xffffffff00000000, \
                            0x00000000000000f0)
        b       2f
        nop

ddr_2GB_MC0:
	    PRINTSTR("MC0 space open : 0x80000000 - 0xFFFFFFFF\r\n")

        GET_NODE_ID_a0;
        XBAR_CONFIG_NODE_a0(0x20, \
                            0x0000000080000000, \
                            0xffffffff80000000, \
                            0x00000000000000f0)
        b       2f
        nop
ddr_1GB_MC0:
	    PRINTSTR("MC0 space open : 0x80000000 - 0xbFFFFFFF\r\n")

        GET_NODE_ID_a0;
        XBAR_CONFIG_NODE_a0(0x20, \
                            0x0000000080000000, \
                            0xffffffffc0000000, \
                            0x00000000000000f0)
        b       2f
        nop
ddr_512MB_MC0:
	    PRINTSTR("MC0 pace open : 0x20000000 - 0x2FFFFFFF\r\n")

        GET_NODE_ID_a0;
        XBAR_CONFIG_NODE_a0(0x20, \
                            0x0000000020000000, \
                            0xfffffffff0000000, \
                            0x00000000100000f0)
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
        dli     a1, 0x30
        beq     a1, a0, ddr_2GB_MC1
        nop

ddr_4GB_MC1:
	    PRINTSTR("MC1 space open : 0x100000000 - 0x1FFFFFFFF\r\n")

        GET_NODE_ID_a0;
        XBAR_CONFIG_NODE_a0(0x20, \
                            0x0000000100000000, \
                            0xffffffff00000000, \
                            0x00000000000000f1)
        b       2f
        nop


ddr_2GB_MC1:
	    PRINTSTR("MC1 space open : 0x80000000 - 0xFFFFFFFF\r\n")

        GET_NODE_ID_a0;
        XBAR_CONFIG_NODE_a0(0x20, \
                            0x0000000080000000, \
                            0xffffffff80000000, \
                            0x00000000000000f1)
        b       3f
        nop
ddr_1GB_MC1:
	    PRINTSTR("MC1 space open : 0x80000000 - 0xbFFFFFFF\r\n")

        GET_NODE_ID_a0;
        XBAR_CONFIG_NODE_a0(0x20, \
                            0x0000000080000000, \
                            0xffffffffc0000000, \
                            0x00000000000000f1)
        b       3f
        nop
ddr_512MB_MC1:
	    PRINTSTR("MC1 space open : 0x20000000 - 0x2FFFFFFF\r\n")

        GET_NODE_ID_a0;
        XBAR_CONFIG_NODE_a0(0x20, \
                            0x0000000020000000, \
                            0xfffffffff0000000, \
                            0x00000000100000f1)
3:
	    PRINTSTR("PCI DMA SPACE reassigned to MC1\r\n")
        dli     t1, 0x00000000000000f1
        sd      t1, 0x180(t0)

        b       2f
        nop

1:
#define PER_CONTROLLER_512M 0x10
#define PER_CONTROLLER_1G	0x20
#define PER_CONTROLLER_2G	0x30
#define PER_CONTROLLER_4G	0x40

		GET_DDR_SIZE    
//		beq	a0, PER_CONTROLLER_512M, interleave_512m  ; almost no 512M mem used now
		nop
		beq	a0, PER_CONTROLLER_1G, interleave_1g 
		nop
		beq	a0, PER_CONTROLLER_2G, interleave_2g 
		nop
		beq	a0, PER_CONTROLLER_4G, interleave_4g 
		nop
		
/* 1GB INTERLEAVE below*/

interleave_1g:
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
	    PRINTSTR("DDR and PCI Interleave space open : 0x80000000 - 0xFFFFFFFF\r\n")
	    PRINTSTR("Interleave using Bit 27\r\n")

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

        // Disable the default Windows
        dli     t1, 0x0
        sd      t1, 0x180(t0)

        // Enable others
		XBAR_CONFIG_NODE_a0(0x120, \
                            0x0000000080000000, \
                            0xffffffffc8000000, \
                            0x00000000000000f0)
        XBAR_CONFIG_NODE_a0(0x128, \
                            0x0000000088000000, \
                            0xffffffffc8000000, \
                            0x00000000000000f1)
        XBAR_CONFIG_NODE_a0(0x130, \
                            0x00000000c0000000, \
                            0xffffffffc8000000, \
                            0x00000000080000f0)
        XBAR_CONFIG_NODE_a0(0x138, \
                            0x00000000c8000000, \
                            0xffffffffc8000000, \
                            0x00000000080000f1)

#else
#ifdef INTERLEAVE_13
	    PRINTSTR("DDR and PCI Interleave space open : 0x80000000 - 0xFFFFFFFF\r\n")
	    PRINTSTR("Interleave using Bit 13\r\n")

        GET_NODE_ID_a0;
        XBAR_CONFIG_NODE_a0(0x20, \
                            0x0000000080000000, \
                            0xffffffffc0002000, \
                            0x00000000000000f0)
        XBAR_CONFIG_NODE_a0(0x28, \
                            0x0000000080002000, \
                            0xffffffffc0002000, \
                            0x00000000000000f1)
        XBAR_CONFIG_NODE_a0(0x30, \
                            0x00000000c0000000, \
                            0xffffffffc0002000, \
                            0x00000000000020f0)
        XBAR_CONFIG_NODE_a0(0x38, \
                            0x00000000c0002000, \
                            0xffffffffc0002000, \
                            0x00000000000020f1)

        // Disable the default Windows
        dli     t1, 0x0
        sd      t1, 0x180(t0)

        // Enable others
		XBAR_CONFIG_NODE_a0(0x120, \
                            0x0000000080000000, \
                            0xffffffffc0002000, \
                            0x00000000000000f0)
        XBAR_CONFIG_NODE_a0(0x128, \
                            0x0000000080002000, \
                            0xffffffffc0002000, \
                            0x00000000000000f1)
        XBAR_CONFIG_NODE_a0(0x130, \
                            0x00000000c0000000, \
                            0xffffffffc0002000, \
                            0x00000000000020f0)
        XBAR_CONFIG_NODE_a0(0x138, \
                            0x00000000c0002000, \
                            0xffffffffc0002000, \
                            0x00000000000020f1)


#else
#ifdef INTERLEAVE_12
	    PRINTSTR("DDR and PCI Interleave space open : 0x80000000 - 0xFFFFFFFF\r\n")
	    PRINTSTR("Interleave using Bit 12\r\n")

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

        // Disable the default Windows
        dli     t1, 0x0
        sd      t1, 0x180(t0)

        // Enable others
        XBAR_CONFIG_NODE_a0(0x120, \
                            0x0000000080000000, \
                            0xffffffffc0001000, \
                            0x00000000000000f0)
        XBAR_CONFIG_NODE_a0(0x128, \
                            0x0000000080001000, \
                            0xffffffffc0001000, \
                            0x00000000000000f1)
        XBAR_CONFIG_NODE_a0(0x130, \
                            0x00000000c0000000, \
                            0xffffffffc0001000, \
                            0x00000000000010f0)
        XBAR_CONFIG_NODE_a0(0x138, \
                            0x00000000c0001000, \
                            0xffffffffc0001000, \
                            0x00000000000010f1)

#else
#ifdef INTERLEAVE_11
	    PRINTSTR("DDR and PCI Interleave space open : 0x80000000 - 0xFFFFFFFF\r\n")
	    PRINTSTR(" DDR and PCI Interleave using Bit 11\r\n")

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

	    PRINTSTR("PCI  DDR and PCI Interleave space open : 0x80000000 - 0xFFFFFFFF\r\n")
	    PRINTSTR("PCI  DDR and PCI Interleave using Bit 11\r\n")

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
	    PRINTSTR(" DDR and PCI Interleave space open : 0x80000000 - 0xFFFFFFFF\r\n")
	    PRINTSTR(" DDR and PCI Interleave using Bit 10\r\n")

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

        // Disable the default Windows
        dli     t1, 0x0
        sd      t1, 0x180(t0)

        // Enable others
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
		  b  interleave_out;
		  nop

interleave_2g:
#ifdef NO_INTERLEAVE
	    PRINTSTR("DDR space open : 0x100000000 - 0x1FFFFFFFF\r\n")

        GET_NODE_ID_a0;
        XBAR_CONFIG_NODE_a0(0x20, \
                            0x0000000100000000, \
                            0xffffffff80000000, \
                            0x00000000000000f0)
        XBAR_CONFIG_NODE_a0(0x38, \
                            0x0000000180000000, \
                            0xffffffff80000000, \
                            0x00000000000000f1)

	    PRINTSTR("PCI DDR space open : 0x100000000 - 0x1FFFFFFFF\r\n")

        // Disable the default Windows
        dli     t1, 0x0
        sd      t1, 0x180(t0)

        // Enable others
        GET_NODE_ID_a0;
        XBAR_CONFIG_NODE_a0(0x120, \
                            0x0000000100000000, \
                            0xffffffff80000000, \
                            0x00000000000000f0)
        XBAR_CONFIG_NODE_a0(0x138, \
                            0x0000000180000000, \
                            0xffffffff80000000, \
                            0x00000000000000f1)

#else
#ifdef INTERLEAVE_27
	    PRINTSTR(" DDR and PCI Interleave space open : 0x100000000 - 0x1FFFFFFFF\r\n")
	    PRINTSTR(" DDR and PCI Interleave using Bit 27\r\n")

        GET_NODE_ID_a0;
        XBAR_CONFIG_NODE_a0(0x20, \
                            0x0000000100000000, \
                            0xffffffff88000000, \
                            0x00000000000000f0)
        XBAR_CONFIG_NODE_a0(0x28, \
                            0x0000000108000000, \
                            0xffffffff88000000, \
                            0x00000000000000f1)
        XBAR_CONFIG_NODE_a0(0x30, \
                            0x0000000180000000, \
                            0xffffffff88000000, \
                            0x00000000080000f0)
        XBAR_CONFIG_NODE_a0(0x38, \
                            0x0000000188000000, \
                            0xffffffff88000000, \
                            0x00000000080000f1)
		// Disable the default Windows
        dli     t1, 0x0
        sd      t1, 0x180(t0)

        // Enable others
		XBAR_CONFIG_NODE_a0(0x120, \
                            0x0000000100000000, \
                            0xffffffff88000000, \
                            0x00000000000000f0)
        XBAR_CONFIG_NODE_a0(0x128, \
                            0x0000000108000000, \
                            0xffffffff88000000, \
                            0x00000000000000f1)
        XBAR_CONFIG_NODE_a0(0x130, \
                            0x0000000180000000, \
                            0xffffffff88000000, \
                            0x00000000080000f0)
        XBAR_CONFIG_NODE_a0(0x138, \
                            0x0000000188000000, \
                            0xffffffff88000000, \
                            0x00000000080000f1)

#else
#ifdef INTERLEAVE_13

	    PRINTSTR(" DDR and PCI Interleave space open : 0x80000000 - 0x17FFFFFFF for GFX UMA\r\n")
	    PRINTSTR(" DDR and PCI Interleave using Bit 13\r\n")

        GET_NODE_ID_a0;
        XBAR_CONFIG_NODE_a0(0x20, \
                            0x0000000080000000, \
                            0xffffffff80002000, \
                            0x00000000000000f0)
        XBAR_CONFIG_NODE_a0(0x28, \
                            0x0000000080002000, \
                            0xffffffff80002000, \
                            0x00000000000000f1)
        XBAR_CONFIG_NODE_a0(0x30, \
                            0x0000000100000000, \
                            0xffffffff80002000, \
                            0x00000000000020f0)
        XBAR_CONFIG_NODE_a0(0x38, \
                            0x0000000100002000, \
                            0xffffffff80002000, \
                            0x00000000000020f1)
		// Disable the default Windows
        dli     t1, 0x0
        sd      t1, 0x180(t0)

        // Enable others
        XBAR_CONFIG_NODE_a0(0x120, \
                            0x0000000080000000, \
                            0xffffffff80002000, \
                            0x00000000000000f0)
        XBAR_CONFIG_NODE_a0(0x128, \
                            0x0000000080002000, \
                            0xffffffff80002000, \
                            0x00000000000000f1)
        XBAR_CONFIG_NODE_a0(0x130, \
                            0x0000000100000000, \
                            0xffffffff80002000, \
                            0x00000000000020f0)
        XBAR_CONFIG_NODE_a0(0x138, \
                            0x0000000100002000, \
                            0xffffffff80002000, \
                            0x00000000000020f1)


#else
#ifdef INTERLEAVE_12
	    PRINTSTR(" DDR and PCI Interleave space open : 0x100000000 - 0x1FFFFFFFF\r\n")
	    PRINTSTR(" DDR and PCI Interleave using Bit 12\r\n")

        GET_NODE_ID_a0;
        XBAR_CONFIG_NODE_a0(0x20, \
                            0x0000000100000000, \
                            0xffffffff80001000, \
                            0x00000000000000f0)
        XBAR_CONFIG_NODE_a0(0x28, \
                            0x0000000100001000, \
                            0xffffffff80001000, \
                            0x00000000000000f1)
        XBAR_CONFIG_NODE_a0(0x30, \
                            0x0000000180000000, \
                            0xffffffff80001000, \
                            0x00000000000010f0)
        XBAR_CONFIG_NODE_a0(0x38, \
                            0x0000000180001000, \
                            0xffffffff80001000, \
                            0x00000000000010f1)

		// Disable the default Windows
        dli     t1, 0x0
        sd      t1, 0x180(t0)

        // Enable others
        XBAR_CONFIG_NODE_a0(0x120, \
                            0x0000000100000000, \
                            0xffffffff80001000, \
                            0x00000000000000f0)
        XBAR_CONFIG_NODE_a0(0x128, \
                            0x0000000100001000, \
                            0xffffffff80001000, \
                            0x00000000000000f1)
        XBAR_CONFIG_NODE_a0(0x130, \
                            0x0000000180000000, \
                            0xffffffff80001000, \
                            0x00000000000010f0)
        XBAR_CONFIG_NODE_a0(0x138, \
                            0x0000000180001000, \
                            0xffffffff80001000, \
                            0x00000000000010f1)

#else
#ifdef INTERLEAVE_11
	    PRINTSTR(" DDR and PCI Interleave space open : 0x100000000 - 0x1FFFFFFFF\r\n")
	    PRINTSTR(" DDR and PCI Interleave using Bit 11\r\n")

        GET_NODE_ID_a0;
        XBAR_CONFIG_NODE_a0(0x20, \
                            0x0000000100000000, \
                            0xffffffff80000800, \
                            0x00000000000000f0)
        XBAR_CONFIG_NODE_a0(0x28, \
                            0x0000000100000800, \
                            0xffffffff80000800, \
                            0x00000000000000f1)
        XBAR_CONFIG_NODE_a0(0x30, \
                            0x0000000180000000, \
                            0xffffffff80000800, \
                            0x00000000000008f0)
        XBAR_CONFIG_NODE_a0(0x38, \
                            0x0000000180000800, \
                            0xffffffff80000800, \
                            0x00000000000008f1)

        // Disable the default Windows
        dli     t1, 0x0
        sd      t1, 0x180(t0)

        // Enable others
        GET_NODE_ID_a0;
        XBAR_CONFIG_NODE_a0(0x120, \
                            0x0000000100000000, \
                            0xffffffff80000800, \
                            0x00000000000000f0)
        XBAR_CONFIG_NODE_a0(0x128, \
                            0x0000000100000800, \
                            0xffffffff80000800, \
                            0x00000000000000f1)
        XBAR_CONFIG_NODE_a0(0x130, \
                            0x0000000180000000, \
                            0xffffffff80000800, \
                            0x00000000000008f0)
        XBAR_CONFIG_NODE_a0(0x138, \
                            0x0000000180000800, \
                            0xffffffff80000800, \
                            0x00000000000008f1)

#else
	    PRINTSTR(" DDR and PCI Interleave space open : 0x100000000 - 0x1FFFFFFFF\r\n")
	    PRINTSTR(" DDR and PCI Interleave using Bit 10\r\n")

        GET_NODE_ID_a0;
        XBAR_CONFIG_NODE_a0(0x20, \
                            0x0000000100000000, \
                            0xffffffff80000400, \
                            0x00000000000000f0)
        XBAR_CONFIG_NODE_a0(0x28, \
                            0x0000000100000400, \
                            0xffffffff80000400, \
                            0x00000000000000f1)
        XBAR_CONFIG_NODE_a0(0x30, \
                            0x0000000180000000, \
                            0xffffffff80000400, \
                            0x00000000000004f0)
        XBAR_CONFIG_NODE_a0(0x38, \
                            0x0000000180000400, \
                            0xffffffff80000400, \
                            0x00000000000004f1)

        // Disable the default Windows
        dli     t1, 0x0
        sd      t1, 0x180(t0)

        // Enable others
        XBAR_CONFIG_NODE_a0(0x120, \
                            0x0000000100000000, \
                            0xffffffff80000400, \
                            0x00000000000000f0)
        XBAR_CONFIG_NODE_a0(0x128, \
                            0x0000000100000400, \
                            0xffffffff80000400, \
                            0x00000000000000f1)
        XBAR_CONFIG_NODE_a0(0x130, \
                            0x0000000180000000, \
                            0xffffffff80000400, \
                            0x00000000000004f0)
        XBAR_CONFIG_NODE_a0(0x138, \
                            0x0000000180000400, \
                            0xffffffff80000400, \
                            0x00000000000004f1)
#endif
#endif
#endif
#endif
#endif
		  b  interleave_out;
		  nop

interleave_4g:
#ifdef NO_INTERLEAVE
	    PRINTSTR("DDR space open : 0x200000000 - 0x3FFFFFFFF\r\n")

        GET_NODE_ID_a0;
        XBAR_CONFIG_NODE_a0(0x20, \
                            0x0000000200000000, \
                            0xffffffff00000000, \
                            0x00000000000000f0)
        XBAR_CONFIG_NODE_a0(0x38, \
                            0x0000000300000000, \
                            0xffffffff00000000, \
                            0x00000000000000f1)

	    PRINTSTR("PCI DDR space open : 0x200000000 - 0x3FFFFFFFF\r\n")

        // Disable the default Windows
        dli     t1, 0x0
        sd      t1, 0x180(t0)

        // Enable others
        GET_NODE_ID_a0;
        XBAR_CONFIG_NODE_a0(0x120, \
                            0x0000000200000000, \
                            0xffffffff00000000, \
                            0x00000000000000f0)
        XBAR_CONFIG_NODE_a0(0x138, \
                            0x0000000300000000, \
                            0xffffffff00000000, \
                            0x00000000000000f1)

#else
#ifdef INTERLEAVE_27
	    PRINTSTR(" DDR and PCI Interleave space open : 0x200000000 - 0x3FFFFFFFF\r\n")
	    PRINTSTR(" DDR and PCI Interleave using Bit 27\r\n")

        GET_NODE_ID_a0;
        XBAR_CONFIG_NODE_a0(0x20, \
                            0x0000000200000000, \
                            0xffffffff08000000, \
                            0x00000000000000f0)
        XBAR_CONFIG_NODE_a0(0x28, \
                            0x0000000208000000, \
                            0xffffffff08000000, \
                            0x00000000000000f1)
        XBAR_CONFIG_NODE_a0(0x30, \
                            0x0000000300000000, \
                            0xffffffff08000000, \
                            0x00000000080000f0)
        XBAR_CONFIG_NODE_a0(0x38, \
                            0x0000000308000000, \
                            0xffffffff08000000, \
                            0x00000000080000f1)

        // Disable the default Windows
        dli     t1, 0x0
        sd      t1, 0x180(t0)

        // Enable others
        XBAR_CONFIG_NODE_a0(0x120, \
                            0x0000000200000000, \
                            0xffffffff08000000, \
                            0x00000000000000f0)
        XBAR_CONFIG_NODE_a0(0x128, \
                            0x0000000208000000, \
                            0xffffffff08000000, \
                            0x00000000000000f1)
        XBAR_CONFIG_NODE_a0(0x130, \
                            0x0000000300000000, \
                            0xffffffff08000000, \
                            0x00000000080000f0)
        XBAR_CONFIG_NODE_a0(0x138, \
                            0x0000000308000000, \
                            0xffffffff08000000, \
                            0x00000000080000f1)
#else
#ifdef INTERLEAVE_13
	    PRINTSTR(" DDR and PCI Interleave space open : 0x200000000 - 0x3FFFFFFFF\r\n")
	    PRINTSTR(" DDR and PCI Interleave using Bit 13\r\n")

        GET_NODE_ID_a0;
        XBAR_CONFIG_NODE_a0(0x20, \
                            0x0000000200000000, \
                            0xffffffff00002000, \
                            0x00000000000000f0)
        XBAR_CONFIG_NODE_a0(0x28, \
                            0x0000000200002000, \
                            0xffffffff00002000, \
                            0x00000000000000f1)
        XBAR_CONFIG_NODE_a0(0x30, \
                            0x0000000300000000, \
                            0xffffffff00002000, \
                            0x00000000000020f0)
        XBAR_CONFIG_NODE_a0(0x38, \
                            0x0000000300002000, \
                            0xffffffff00002000, \
                            0x00000000000020f1)

        // Disable the default Windows
        dli     t1, 0x0
        sd      t1, 0x180(t0)

        // Enable others
        XBAR_CONFIG_NODE_a0(0x120, \
                            0x0000000200000000, \
                            0xffffffff00002000, \
                            0x00000000000000f0)
        XBAR_CONFIG_NODE_a0(0x128, \
                            0x0000000200002000, \
                            0xffffffff00002000, \
                            0x00000000000000f1)
        XBAR_CONFIG_NODE_a0(0x130, \
                            0x0000000300000000, \
                            0xffffffff00002000, \
                            0x00000000000020f0)
        XBAR_CONFIG_NODE_a0(0x138, \
                            0x0000000300002000, \
                            0xffffffff00002000, \
                            0x00000000000020f1)


#else
#ifdef INTERLEAVE_12
	    PRINTSTR(" DDR and PCI Interleave space open : 0x200000000 - 0x3FFFFFFFF\r\n")
	    PRINTSTR(" DDR and PCI Interleave using Bit 12\r\n")

        GET_NODE_ID_a0;
        XBAR_CONFIG_NODE_a0(0x20, \
                            0x0000000200000000, \
                            0xffffffff00001000, \
                            0x00000000000000f0)
        XBAR_CONFIG_NODE_a0(0x28, \
                            0x0000000200001000, \
                            0xffffffff00001000, \
                            0x00000000000000f1)
        XBAR_CONFIG_NODE_a0(0x30, \
                            0x0000000300000000, \
                            0xffffffff00001000, \
                            0x00000000000010f0)
        XBAR_CONFIG_NODE_a0(0x38, \
                            0x0000000300001000, \
                            0xffffffff00001000, \
                            0x00000000000010f1)

        // Disable the default Windows
        dli     t1, 0x0
        sd      t1, 0x180(t0)

        // Enable others
        XBAR_CONFIG_NODE_a0(0x120, \
                            0x0000000200000000, \
                            0xffffffff00001000, \
                            0x00000000000000f0)
        XBAR_CONFIG_NODE_a0(0x128, \
                            0x0000000200001000, \
                            0xffffffff00001000, \
                            0x00000000000000f1)
        XBAR_CONFIG_NODE_a0(0x130, \
                            0x0000000300000000, \
                            0xffffffff00001000, \
                            0x00000000000010f0)
        XBAR_CONFIG_NODE_a0(0x138, \
                            0x0000000300001000, \
                            0xffffffff00001000, \
                            0x00000000000010f1)


#else
#ifdef INTERLEAVE_11
	    PRINTSTR(" DDR and PCI Interleave space open : 0x200000000 - 0x3FFFFFFFF\r\n")
	    PRINTSTR(" DDR and PCI Interleave using Bit 11\r\n")

        GET_NODE_ID_a0;
        XBAR_CONFIG_NODE_a0(0x20, \
                            0x0000000200000000, \
                            0xffffffff00000800, \
                            0x00000000000000f0)
        XBAR_CONFIG_NODE_a0(0x28, \
                            0x0000000200000800, \
                            0xffffffff00000800, \
                            0x00000000000000f1)
        XBAR_CONFIG_NODE_a0(0x30, \
                            0x0000000300000000, \
                            0xffffffff00000800, \
                            0x00000000000008f0)
        XBAR_CONFIG_NODE_a0(0x38, \
                            0x0000000300000800, \
                            0xffffffff00000800, \
                            0x00000000000008f1)

        // Disable the default Windows
        dli     t1, 0x0
        sd      t1, 0x180(t0)

        // Enable others
        XBAR_CONFIG_NODE_a0(0x120, \
                            0x0000000200000000, \
                            0xffffffff80000800, \
                            0x00000000000000f0)
        XBAR_CONFIG_NODE_a0(0x128, \
                            0x0000000200000800, \
                            0xffffffff80000800, \
                            0x00000000000000f1)
        XBAR_CONFIG_NODE_a0(0x130, \
                            0x0000000280000000, \
                            0xffffffff80000800, \
                            0x00000000000008f0)
        XBAR_CONFIG_NODE_a0(0x138, \
                            0x0000000280000800, \
                            0xffffffff80000800, \
                            0x00000000000008f1)

#else
	    PRINTSTR(" DDR and PCI Interleave space open : 0x200000000 - 0x2FFFFFFFF\r\n")
	    PRINTSTR(" DDR and PCI Interleave using Bit 10\r\n")

        GET_NODE_ID_a0;
        XBAR_CONFIG_NODE_a0(0x20, \
                            0x0000000200000000, \
                            0xffffffff00000400, \
                            0x00000000000000f0)
        XBAR_CONFIG_NODE_a0(0x28, \
                            0x0000000200000400, \
                            0xffffffff00000400, \
                            0x00000000000000f1)
        XBAR_CONFIG_NODE_a0(0x30, \
                            0x0000000300000000, \
                            0xffffffff00000400, \
                            0x00000000000004f0)
        XBAR_CONFIG_NODE_a0(0x38, \
                            0x0000000300000400, \
                            0xffffffff00000400, \
                            0x00000000000004f1)

        // Disable the default Windows
        dli     t1, 0x0
        sd      t1, 0x180(t0)

        // Enable others
        XBAR_CONFIG_NODE_a0(0x120, \
                            0x0000000200000000, \
                            0xffffffff00000400, \
                            0x00000000000000f0)
        XBAR_CONFIG_NODE_a0(0x128, \
                            0x0000000200000400, \
                            0xffffffff00000400, \
                            0x00000000000000f1)
        XBAR_CONFIG_NODE_a0(0x130, \
                            0x0000000300000000, \
                            0xffffffff00000400, \
                            0x00000000000004f0)
        XBAR_CONFIG_NODE_a0(0x138, \
                            0x0000000300000400, \
                            0xffffffff00000400, \
                            0x00000000000004f1)
#endif
#endif
#endif
#endif
#endif


2:

interleave_out:

#if 0 //print registers
    li      t1, 151
    li      t2, 0xaff00000

reg_read:
    lw      a0, 0x4(t2)
    bal	    hexserial
    nop
    PRINTSTR("  ")

    lw      a0, 0x0(t2)
    bal	    hexserial
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
#if 0
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

