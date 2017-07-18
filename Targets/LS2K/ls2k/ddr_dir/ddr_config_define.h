/***********************************************************
 * Author: Chen Xinke
***********************************************************/
/***********************************************************
 msize map
[ 7: 0]: Node 0 memorysize
[15: 8]: Node 1 memorysize
[23:16]: Node 2 memorysize
[31:24]: Node 3 memorysize

 s1: new map
not affected by AUTO_DDR_CONFIG
|[ 7: 6]|                    | 2'b0    | RESERVED          |
|[ 5: 4]| SB NODE ID         |         |                   |
|[ 3: 2]| CONTROLLER_SELECT  | 2'b00   | USE BOTH          |
|       |                    | 2'b01   | MC0_ONLY          |
|       |                    | 2'b10   | MC1_ONLY          |
|[ 1: 0]| NODE ID            |         |                   |
------------------------------------------------------------
 when use AUTO_DDR_CONFIG

|[63:32]|                    | 32'b0   | RESERVED          |
|[31:28]|                    | 4'bx    | MC1_SLOT1 I2C ADDR|
|[27:24]|                    | 4'bx    | MC1_SLOT0 I2C ADDR|
|[23:20]|                    | 4'bx    | MC0_SLOT1 I2C ADDR|
|[19:16]|                    | 4'bx    | MC0_SLOT0 I2C ADDR|
|[15: 8]|                    | 8'b0    | RESERVED          |
value:
4'1xxx: there is no DIMM populated
4'0xxx: there is DIMM populated

SLOT1: the slot connected to DDR2_SCSn3/2
SLOT0: the slot connected to DDR2_SCSn1/0
when don't use AUTO_DDR_CONFIG, set these info manually
------------------------------------------------------------
not affected by PROBE_DIMM
|[46:40]| MC1_MEMSIZE        | 7'b0000 | 0M                |
|       |                    | 7'b0001 | 512M              |
|       |                    | 7'b0010 | 1G                |
|       |                    | 7'b0011 | 1.5G              |
|       |                    | 7'hX    | X*512M            | 
|[14: 8]| MC0_MEMSIZE        | 7'b0000 | 0M                |
|       |                    | 7'b0001 | 512M              |
|       |                    | 7'b0010 | 1G                |
|       |                    | 7'b0011 | 1.5G              |
|       |                    | 7'hX    | X*512M            | 
|       |                    | max     | 127*512M          | 
------------------------------------------------------------
DIMM infor:
|[31:30]| SDRAM_TYPE         | 2'b00   | NO_DIMM           |
|       |                    | 2'b10   | DDR2              |
|       |                    | 2'b11   | DDR3              |
|[29:29]| DIMM_ECC           | 1'b1    | With ECC          |
|       |                    | 1'b0    | No ECC            |
|[28:28]| DIMM_TYPE          | 1'b1    | Registered DIMM   |
|       |                    | 1'b0    | Unbuffered DIMM   |
|[27:27]| DIMM_WIDTH         | 1'b1    | REDUC--32bits     |
|       |                    | 1'b0    | NORMAL--64 bits   |
|[26:24]| SDRAM_ROW_SIZE     | MC_ROW  | 15 - ROW_SIZE(for lsmcd3)  |
|                                        16 - ROW_SIZE(for lsmc)    |
|[23:23]| SDRAM_EIGHT_BANK   | 1'b0    | FOUR  BANKS       |
|       |                    | 1'b1    | EIGHT BANKS       |
|[22:22]| ADDR_MIRROR        | 1'b1    | ADDR MIRROR       |
|       |                    | 1'b0    | STANDARD          |
|[21:20]| SDRAM_COL_SIZE     | MC_COL  | 12 - COL_SIZE     |
|[19:16]| MC_CS_MAP          |         |                   |
|[15:15]| SDRAM_WIDTH        | 1'b1    | x16               |
|       |                    | 1'b0    | x8                |
------------------------------------------------------------
|[63:47]| MC1--like s1[31:16] for MC0
temparary used in PROBE_DIMM
|[38:32]| DIMM_MEMSIZE       | 7'b0000 | 0M                |
|       |                    | 7'b0001 | 512M              |
|       |                    | 7'b0010 | 1G                |
|       |                    | 7'b0011 | 1.5G              |
|       |                    | 7'hX    | X*512M            | 
***********************************************************/
#ifndef LOONGSON3_DDR2_CONFIG
#define LOONGSON3_DDR2_CONFIG
#######################################################
/* Interleave pattern when both controller enabled */
//note: NO_INTERLEAVE has highest priority. and define interleave_X mode doesn't
//guarantee mem will be at interleave mode unless MC0 and MC1 have the same special memsize
//#define NO_INTERLEAVE
#define MC_INTERLEAVE_OFFSET 20 //only work when NO_INTERLEAVE is not defined.(32 or above is not tested!!)
#######################################################
#define MC_SDRAM_TYPE_DDR2  (2<<30)
#define MC_SDRAM_TYPE_DDR3  (3<<30)
#define MC_DIMM_ECC_YES     (1<<29)
#define MC_DIMM_ECC_NO      (0<<29)
#define MC_DIMM_BUF_REG_YES (1<<28)
#define MC_DIMM_BUF_REG_NO  (0<<28)
#define MC_DIMM_WIDTH_32    (1<<27)
#define MC_DIMM_WIDTH_64    (0<<27)
#ifdef  LSMCD3_2
#define MC_SDRAM_ROW_15     (0<<24)
#define MC_SDRAM_ROW_14     (1<<24)
#define MC_SDRAM_ROW_13     (2<<24)
#define MC_SDRAM_ROW_12     (3<<24)
#define MC_SDRAM_ROW_11     (4<<24)
#else
#define MC_SDRAM_ROW_16     (0<<24)
#define MC_SDRAM_ROW_15     (1<<24)
#define MC_SDRAM_ROW_14     (2<<24)
#define MC_SDRAM_ROW_13     (3<<24)
#define MC_SDRAM_ROW_12     (4<<24)
#define MC_SDRAM_ROW_11     (5<<24)
#endif
#define MC_SDRAM_BANK_4     (0<<23)
#define MC_SDRAM_BANK_8     (1<<23)
#define MC_ADDR_MIRROR_YES  (1<<22)
#define MC_ADDR_MIRROR_NO   (0<<22)
#define MC_SDRAM_COL_12     (0<<20)
#define MC_SDRAM_COL_11     (1<<20)
#define MC_SDRAM_COL_10     (2<<20)
#define MC_SDRAM_COL_9      (3<<20)
#define MC_USE_CS_0         (0x1<<16)
#define MC_USE_CS_1         (0x2<<16)
#define MC_USE_CS_2         (0x4<<16)
#define MC_USE_CS_3         (0x8<<16)
#define MC_USE_CS_0_1       (0x3<<16)
#define MC_USE_CS_0_2       (0x5<<16)
#define MC_USE_CS_0_3       (0x9<<16)
#define MC_USE_CS_1_2       (0x6<<16)
#define MC_USE_CS_1_3       (0xa<<16)
#define MC_USE_CS_2_3       (0xc<<16)
#define MC_USE_CS_0_1_2     (0x7<<16)
#define MC_USE_CS_0_1_3     (0xb<<16)
#define MC_USE_CS_0_2_3     (0xd<<16)
#define MC_USE_CS_1_2_3     (0xe<<16)
#define MC_USE_CS_0_1_2_3   (0xf<<16)
#define MC_SDRAM_WIDTH_X16  (1<<15)
#define MC_SDRAM_WIDTH_X8   (0<<15)
#define USE_MC_0            (0x4)
#define USE_MC_1            (0x8)
#define USE_MC_0_1          (0x0)
#define MC_MEMSIZE_(x)      ((x&0x7f)<<8)
#define MC_NODE_ID_0        (0)
#define MC_NODE_ID_1        (1)
#define MC_NODE_ID_2        (2)
#define MC_NODE_ID_3        (3)

#ifdef  loongson3A3
#define LOCK_SCACHE_CONFIG_BASE_ADDR 0x900000003ff00200
#define L1XBAR_CONFIG_BASE_ADDR 0x900000003ff02000
#define L2XBAR_CONFIG_BASE_ADDR 0x900000003ff00000
#define CHIP_CONFIG_ADDR        0x900000001fe00180
#define CHIP_CONFIG_BASE_ADDR   0x900000001fe00180
#define CHIP_SAMPLE_BASE_ADDR   0x900000001fe00190
#define DDR_CLKSEL_EN_OFFSET        3
#define DDR_CLKSEL_OFFSET           37
#define DDR_CLKSEL_MASK             0x1F
#define DDR_CLKSEL_WIDTH            5
#define DDR_CLKSEL_SOFT_OFFSET      24
#define DDR_CLKSEL_SOFT_MASK        0x1F
#ifdef  LSMC_2
#define DDR_CONFIG_DISABLE_OFFSET   4
#else
#define DDR_CONFIG_DISABLE_OFFSET   8
#endif
#define ARB_TEMP_L2WINDOW_OFFSET    0x20
#else
#ifdef  LS3B
#define LOCK_SCACHE_CONFIG_BASE_ADDR 0x900000003ff00200
#define L1XBAR_CONFIG_BASE_ADDR 0x900000003ff02000
#define L2XBAR_CONFIG_BASE_ADDR 0x900000003ff00000
#define CHIP_CONFIG_ADDR        0x900000001fe00180
#define CHIP_CONFIG_BASE_ADDR   0x900000001fe00180
#define CHIP_SAMPLE_BASE_ADDR   0x900000001fe00190
#define DDR_CLKSEL_EN_OFFSET        3
#define DDR_CLKSEL_OFFSET           37
#define DDR_CLKSEL_MASK             0x1F
#define DDR_CLKSEL_WIDTH            5
#define DDR_CLKSEL_SOFT_OFFSET      40
#define DDR_CLKSEL_SOFT_MASK        0x1F
#define DDR_CONFIG_DISABLE_OFFSET   4
#define ARB_TEMP_L2WINDOW_OFFSET    0x20
#else
#ifdef  LS2HMC
#define LOCK_SCACHE_CONFIG_BASE_ADDR 0x900000001fd84200
#define L1XBAR_CONFIG_BASE_ADDR 0x900000001fd80000
#define L2XBAR_CONFIG_BASE_ADDR 0x900000001fd80000
#define CHIP_CONFIG_ADDR        0x900000001fd00200
#define CHIP_CONFIG_BASE_ADDR   0x900000001fd00200
#define CHIP_SAMPLE_BASE_ADDR   0x900000001fd00210
#define LS2H_CLOCK_CTRL0_ADDR   0x900000001fd00220
#define DDR_CLKSEL_EN_OFFSET        25
#define DDR_CLKSEL_OFFSET           23
#define DDR_CLKSEL_MASK             0x3
#define DDR_CLKSEL_WIDTH            2
#define DDR_CLKSEL_SOFT_OFFSET      16
#define DDR_CLKSEL_SOFT_MASK        0xfffa
#define DDR_CONFIG_DISABLE_OFFSET   13
#define ARB_TEMP_L2WINDOW_OFFSET    0x38
#define NO_L2XBAR_CONFIGURE
#else //default current set as loosong3A3
#define LOCK_SCACHE_CONFIG_BASE_ADDR 0x900000003ff00200
#define L1XBAR_CONFIG_BASE_ADDR 0x900000003ff02000
#define L2XBAR_CONFIG_BASE_ADDR 0x900000003ff00000
#define CHIP_CONFIG_ADDR        0x900000001fe00180
#define CHIP_CONFIG_BASE_ADDR   0x900000001fe00180
#define CHIP_SAMPLE_BASE_ADDR   0x900000001fe00190
#define DDR_CLKSEL_EN_OFFSET        3
#define DDR_CLKSEL_OFFSET           37
#define DDR_CLKSEL_MASK             0x1F
#define DDR_CLKSEL_WIDTH            5
#define DDR_CLKSEL_SOFT_OFFSET      24
#define DDR_CLKSEL_SOFT_MASK        0x1F
#define DDR_CONFIG_DISABLE_OFFSET   8
#define ARB_TEMP_L2WINDOW_OFFSET    0x20
#endif
#endif
#endif

#define GET_NODE_ID_a0  dli a0, 0x00000003; and a0, s1, a0; dsll a0, 44;
#define GET_NODE_ID_a1  dli a1, 0x00000003; and a1, s1, a1;
#define GET_MC_SEL_BITS dli a1, 0x0000000c; and a1, s1, a1; dsrl a1, a1, 2;
#define GET_MC0_ONLY    dli a1, 0x00000004; and a1, s1, a1;
#define GET_MC1_ONLY    dli a1, 0x00000008; and a1, s1, a1;
#ifdef  NO_L2XBAR_CONFIGURE
#define XBAR_CONFIG_NODE_a0(OFFSET, BASE, MASK, MMAP) ;
#define L2XBAR_CLEAR_WINDOW(OFFSET) ;
#else
#define XBAR_CONFIG_NODE_a0(OFFSET, BASE, MASK, MMAP) \
                        daddiu  v0, t0, OFFSET;       \
                        dli     v1, BASE;             \
                        or      v1, v1, a0;           \
                        sd      v1, 0x00(v0);         \
                        dli     v1, MASK;             \
                        sd      v1, 0x40(v0);         \
                        dli     v1, MMAP;             \
                        sd      v1, 0x80(v0);
#define L2XBAR_CLEAR_WINDOW(OFFSET) \
                        daddiu  v0, t0, OFFSET;       \
                        sd      $0, 0x80(v0);         \
                        sd      $0, 0x00(v0);         \
                        sd      $0, 0x40(v0);
#define L2XBAR_CONFIG_INTERLEAVE(OFFSET, BASE, MASK, MMAP) \
                        daddiu  v0, t0, OFFSET;       \
                        ld      v1, 0x00(v0);         \
                        or      v1, v1, BASE;         \
                        sd      v1, 0x00(v0);         \
                        ld      v1, 0x40(v0);         \
                        or      v1, v1, MASK;         \
                        sd      v1, 0x40(v0);         \
                        ld      v1, 0x80(v0);         \
                        or      v1, v1, MMAP;         \
                        sd      v1, 0x80(v0);
//special used, not general, you must guarantee the original MMAP is 0xxxxF1.
#define L2XBAR_RECONFIG_TO_MC0(OFFSET) \
                        daddiu  v0, t0, OFFSET;       \
                        ld      v1, 0x80(v0);         \
                        xor     v1, v1, 0x1;          \
                        sd      v1, 0x80(v0);
#define L2XBAR_RECONFIG_TO_MC1(OFFSET) \
                        daddiu  v0, t0, OFFSET;       \
                        ld      v1, 0x80(v0);         \
                        ori     v1, v1, 0x1;          \
                        sd      v1, 0x80(v0);
#define L2XBAR_CONFIG_PCI_AS_CPU(OFFSET) \
                        daddiu  v0, t0, OFFSET;       \
                        ld      v1, 0x0(v0);          \
                        sd      v1, 0x100(v0);        \
                        ld      v1, 0x40(v0);         \
                        sd      v1, 0x140(v0);        \
                        ld      v1, 0x80(v0);         \
                        sd      v1, 0x180(v0);
//special used, not general.
#define L2XBAR_CONFIG_PCI_BASE_0to8(OFFSET) \
                        daddiu  v0, t0, OFFSET;       \
                        ld      v1, 0x0(v0);          \
                        dli     a1, 0x80000000;       \
                        or      v1, v1, a1;           \
                        sd      v1, 0x0(v0);
#define L2XBAR_DISABLE_WINDOW(OFFSET) \
                        daddiu  v0, t0, OFFSET;       \
                        sd      $0, 0x80(v0);
#define L2XBAR_ENABLE_WINDOW(OFFSET) \
                        daddiu  v0, t0, OFFSET;       \
                        ld      v1, 0x80(v0);         \
                        ori     v1, v1, 0x80;         \
                        sd      v1, 0x80(v0);
#endif
//------------------------
//defination for s1
#define SDRAM_TYPE_OFFSET   30
#define DIMM_ECC_OFFSET     29
#define DIMM_TYPE_OFFSET    28
#define DIMM_WIDTH_OFFSET   27
#define ROW_SIZE_OFFSET     24
#define EIGHT_BANK_OFFSET   23
#define ADDR_MIRROR_OFFSET  22
#define COL_SIZE_OFFSET     20
#define MC_CS_MAP_OFFSET    16
#define SDRAM_WIDTH_OFFSET  15
#define MC_CS_MAP_MASK      (0xf)
#define MC1_MEMSIZE_OFFSET  40
#define MC0_MEMSIZE_OFFSET  8
#define MC_MEMSIZE_MASK     (0x7f)
#define DIMM_MEMSIZE_OFFSET 32
#define DIMM_MEMSIZE_MASK   (0x7f)
//------------------------
#define GET_SDRAM_TYPE      \
dli     a1, 0x3;\
dsll    a1, a1, SDRAM_TYPE_OFFSET;\
and     a1, s1, a1;\
dsrl    a1, a1, SDRAM_TYPE_OFFSET;
#define GET_SDRAM_WIDTH      \
dli     a1, 0x1;\
dsll    a1, a1, SDRAM_WIDTH_OFFSET;\
and     a1, s1, a1;\
dsrl    a1, a1, SDRAM_WIDTH_OFFSET;
#define GET_DIMM_ECC       \
dli     a1, 0x1;\
dsll    a1, a1, DIMM_ECC_OFFSET;\
and     a1, s1, a1;\
dsrl    a1, a1, DIMM_ECC_OFFSET;
#define GET_DIMM_TYPE      \
dli     a1, 0x1;\
dsll    a1, a1, DIMM_TYPE_OFFSET;\
and     a1, s1, a1;\
dsrl    a1, DIMM_TYPE_OFFSET;
#define GET_DIMM_WIDTH          \
dli     a1, 0x1;\
dsll    a1, a1, DIMM_WIDTH_OFFSET;\
and     a1, s1, a1;\
dsrl    a1, a1, DIMM_WIDTH_OFFSET;
#define GET_ROW_SIZE      \
dli     a1, 0x7;\
dsll    a1, a1, ROW_SIZE_OFFSET;\
and     a1, s1, a1;\
dsrl    a1, a1, ROW_SIZE_OFFSET;
#define GET_EIGHT_BANK      \
dli     a1, 0x1;\
dsll    a1, a1, EIGHT_BANK_OFFSET;\
and     a1, s1, a1;\
dsrl    a1, a1, EIGHT_BANK_OFFSET;
#define GET_ADDR_MIRROR      \
dli     a1, 0x1;\
dsll    a1, a1, ADDR_MIRROR_OFFSET;\
and     a1, s1, a1;\
dsrl    a1, a1, ADDR_MIRROR_OFFSET;
#define GET_COL_SIZE      \
dli     a1, 0x3;\
dsll    a1, a1, COL_SIZE_OFFSET;\
and     a1, s1, a1;\
dsrl    a1, a1, COL_SIZE_OFFSET;
#define GET_MC_CS_MAP      \
dli     a1, MC_CS_MAP_MASK;\
dsll    a1, a1, MC_CS_MAP_OFFSET;\
and     a1, s1, a1;\
dsrl    a1, a1, MC_CS_MAP_OFFSET;
#define GET_DIMM_MEMSIZE      \
dli     a1, DIMM_MEMSIZE_MASK;\
dsll    a1, a1, DIMM_MEMSIZE_OFFSET;\
and     a1, s1, a1;\
dsrl    a1, a1, DIMM_MEMSIZE_OFFSET;
#define GET_MC1_MEMSIZE      \
dli     a1, MC_MEMSIZE_MASK;\
dsll    a1, a1, MC1_MEMSIZE_OFFSET;\
and     a1, s1, a1;\
dsrl    a1, a1, MC1_MEMSIZE_OFFSET;
#define GET_MC0_MEMSIZE      \
dli     a1, MC_MEMSIZE_MASK;\
dsll    a1, a1, MC0_MEMSIZE_OFFSET;\
and     a1, s1, a1;\
dsrl    a1, a1, MC0_MEMSIZE_OFFSET;
#endif

