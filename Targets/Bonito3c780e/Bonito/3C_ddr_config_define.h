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
not affected by AUTO_DDR_CONFIG
|[ 7: 6]|                    | 2'b0    | RESERVED          |
|[ 5: 4]| SB NODE ID         |         |                   |
|[ 3: 2]| CONTROLLER_SELECT  | 2'b00   | USE BOTH          |
|       |                    | 2'b01   | MC0_ONLY          |
|       |                    | 2'b10   | MC1_ONLY          |
|[ 1: 0]| NODE ID            |         |                   |
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
|[26:24]| SDRAM_ROW_SIZE     | MC_ROW  | 16 - ROW_SIZE     |
|[23:23]| SDRAM_EIGHT_BANK   | 1'b0    | FOUR  BANKS       |
|       |                    | 1'b1    | EIGHT BANKS       |
|[22:22]| ADDR_MIRROR        | 1'b1    | ADDR MIRROR       |
|       |                    | 1'b0    | STANDARD          |
|[21:20]| SDRAM_COL_SIZE     | MC_COL  | 12 - COL_SIZE     |
|[19:16]| MC_CS_MAP          |         |                   |
------------------------------------------------------------
|[63:48]| MC1--like s1[31:16] for MC0
temparary used in PROBE_DIMM
|[38:32]| DIMM_MEMSIZE       | 7'b0000 | 0M                |
|       |                    | 7'b0001 | 512M              |
|       |                    | 7'b0010 | 1G                |
|       |                    | 7'b0011 | 1.5G              |
|       |                    | 7'hX    | X*512M            | 
***********************************************************/
/******************************************
s1: [ 3: 3]: MC1_ONLY
    [ 2: 2]: MC0_ONLY
    [ 1: 0]: NODE ID
    10: MC1_ONLY
    01: MC0_ONLY
    00: USE MC0&MC1
******************************************/
#ifndef LOONGSON3_DDR2_CONFIG
#define LOONGSON3_DDR2_CONFIG
#######################################################
/* Interleave pattern when both controller enabled */
//note: NO_INTERLEAVE has highest priority. and define interleave_X mode doesn't
//guarantee mem will be at interleave mode unless MC0 and MC1 have the same memsize
//#define NO_INTERLEAVE
//#define INTERLEAVE_27
//#define INTERLEAVE_13
//#define INTERLEAVE_12
//#define INTERLEAVE_11
//#define INTERLEAVE_10
#######################################################
#define LOCK_SCACHE_CONFIG_BASE_ADDR 0x900000003ff00200
#define L1XBAR_CONFIG_BASE_ADDR 0x900000003ff02000
#define L2XBAR_CONFIG_BASE_ADDR 0x900000003ff00000
#define CHIP_CONFIG_ADDR        0x900000001fe00180
#define CHIP_CONFIG_BASE_ADDR   0x900000001fe00180
#define DDR_CONFIG_DISABLE_OFFSET   4
#define	MC_INIT_TEMP_L2WINDOW_OFFSET    0x20

#define GET_NODE_ID_a0  dli a0, 0x00000003; and a0, s1, a0; dsll a0, 44;
#define GET_NODE_ID_a1  dli a1, 0x00000003; and a1, s1, a1;
#define GET_MC_SEL_BITS dli a1, 0x0000000c; and a1, s1, a1; dsrl a1, a1, 2;
#define GET_MC0_ONLY    dli a1, 0x00000004; and a1, s1, a1;
#define GET_MC1_ONLY    dli a1, 0x00000008; and a1, s1, a1;
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

