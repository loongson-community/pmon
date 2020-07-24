/***********************************************************
 * Author: Huangshuai
 * Date  : 2019.6.12
 * Usage : define meaning of s1 at different ddr init phase
 *         for each mc channel
 *
 * Limit : now only used for 3A4000
 * 
 * V1.0  : raw
***********************************************************/

/*************************
//when define AUTO_DDR_CONFIG, before read spd
------------------------------------------------------------
|[63:32]|                    | 32'b0   | RESERVED          |
|[31:28]|                    | 4'bx    | MC1_SLOT1 I2C ADDR|
|[27:24]|                    | 4'bx    | MC1_SLOT0 I2C ADDR|
|[23:20]|                    | 4'bx    | MC0_SLOT1 I2C ADDR|
|[19:16]|                    | 4'bx    | MC0_SLOT0 I2C ADDR|
|[15:08]| MC_ENABLE          | 8'h0    | NO MC ENABLE      |
|       |                    | 8'h1    | MC0_ENABLE        |
|       |                    | 8'h2    | MC1_ENABLE        |
|       |                    | 8'h3    | BOTH_ENABLE       |
|       |                    | 8'h4-7  | RESERVED          |
|[07:04]| DIMM_WIDTH         | 4'b0    | RESERVED          |
|       |                    | 4'h1    | 16bit             |
|       |                    | 4'h2    | 32bit             |
|       |                    | 4'h3    | 64bit             |
|[03:00]| NODE_ID            | 4'hx    | x                 |
------------------------------------------------------------

0x00000280__118c0100
//when define AUTO_DDR_CONFIG, after read spd for each dimm
//compare [54:14] for each dimm, if not same, report error
------------------------------------------------------------
|[54:53]| CID_NUM            | 2'h0    | no cid            |
|       |                    | 2'h1    | 2dies             |
|       |                    | 2'h2    | 4dies             |
|       |                    | 2'h3    | 8dies             |
|[52:51]| SDRAM_BG_SIZE      | 2'h0    | no bank group     |
|       |                    | 2'h1    | 2  bank group     |
|       |                    | 2'h2    | 4  bank group     |
|[50:50]| SDRAM_BA_SIZE      | 1'h0    | 4 bank            |
|       |                    | 1'h1    | 8 bank            |
|[49:47]| SDRAM_ROW_SIZE     | 3'hx    | 18-x              |
|[46:45]| SDRAM_COL_SIZE     | 2'hx    | 12-x              |
|[44:44]| ADDR_MIRROR        | 1'b1    | ADDR MIRROR       |
|       |                    | 1'b0    | STANDARD          |
|[43:32]| DIMM_MEMSIZE       |12'bx    | x*1G              |
|[31:30]| DIMM_WIDTH         | 2'h0    | 8bit//not support |
|       |                    | 2'h1    | 16bit             |
|       |                    | 2'h2    | 32bit             |
|       |                    | 2'h3    | 64bit             |
|[29:29]| DIMM_ECC           | 1'b1    | With ECC          |
|       |                    | 1'b0    | No ECC            |
|[28:27]| DIMM_TYPE          | 2'h0    | Unbuffered DIMM   |
|       |                    | 2'h1    | Registered DIMM   |
|       |                    | 2'h2    | SO-DIMM           |
|       |                    | 2'h3    | Load Reduced DIMM |
|[26:25]| SDRAM_WIDTH        | 2'h0    | x4                |
|       |                    | 2'h1    | x8                |
|       |                    | 2'h2    | x16               |
|[24:22]| SDRAM_TYPE         | 3'h0    | NO_DIMM           |
|       |                    | 3'h3    | DDR3              |
|       |                    | 3'h4    | DDR4              |
|[21:14]| MC_CS_MAP          | 8'b0    | CS7-CS0           |
|[13:12]| DIMM_CONFIG        | 2'h1    | only slot0        |
|       |                    | 2'h2    | only slot1        |
|       |                    | 2'h3    | slot0 + slot1     |
|[11:08]| SLOT1_I2C_ADDR     | 4'bx    | slot1 i2c addr    |
|[07:04]| SLOT0_I2C_ADDR     | 4'bx    | slot0 i2c addr    |
|[03:00]| NODE_ID            | 4'hx    | x                 |
------------------------------------------------------------

//when define AUTO_DDR_CONFIG, after read spd for all dimm of all channel
//or not define AUTO_DDR_CONFIG, set s1 manually
------------------------------------------------------------
|[63:56]| MC1_CS_MAP         | 8'bx    | CS7-CS0           |
|[55:44]| MC1_MEMSIZE        |12'bx    | x*1G              |
|[43:40]| MC1_I2C_ADDR       | 4'bx    | x                 |
|[39:36]| RESERVED           | 4'b0    | RESERVED          |
|[35:32]| RESERVED           | 4'b0    | RESERVED          |
|[31:24]| MC0_CS_MAP         | 8'bx    | CS7-CS0           |
|[23:12]| MC0_MEMSIZE        |12'bx    | x*1G              |
|[11:08]| MC0_I2C_ADDR       | 4'bx    | x                 |
|[07:04]| RESERVED           | 4'b0    | RESERVED          |
|[03:00]| NODE_ID            | 4'hx    | x                 |
------------------------------------------------------------

***********************************************************/

#ifdef LOONGSON3A4000
//define value from SPD SPEC 
#define SDRAM_DDR3_V1            0xb
#define SDRAM_DDR4_V1            0xc
#define MODULE_RDIMM_V1          0x1
#define MODULE_UDIMM_V1          0x2
#define MODULE_SODIMM_V1         0x3

//define offset accroding to s1 definition as below
#define S1_CID_NUM_OFFSET_V1        53
#define S1_BG_NUM_OFFSET_V1         51
#define S1_BA_NUM_OFFSET_V1         50
#define S1_ROW_SIZE_OFFSET_V1       47
#define S1_COL_SIZE_OFFSET_V1       45
#define S1_ADDR_MIRROR_OFFSET_V1    44
#define S1_DIMM_MEMSIZE_OFFSET_V1   32
#define S1_DIMM_WIDTH_OFFSET_V1     30
#define S1_DIMM_ECC_OFFSET_V1       29
#define S1_DIMM_TYPE_OFFSET_V1      27
#define S1_SDRAM_WIDTH_OFFSET_V1    25
#define S1_SDRAM_TYPE_OFFSET_V1     22
#define S1_MC_CS_MAP_OFFSET_V1      14
#define S1_DIMM_CONFIG_OFFSET_V1    12
#define S1_I2C_ADDR_OFFSET_V1       4

#define GET_SPD_MODULE_TYPE_V1 \
    dli     a1, 0x3;\
    GET_I2C_NODE_ID_a2;\
    bal     i2cread;\
    nop;\
    andi    v0, v0, 0xf;
#ifndef DDR3_DIMM
#define GET_SPD_SDRAM_WIDTH_V1 \
    dli     a1, 0xc;\
    GET_I2C_NODE_ID_a2;\
    bal     i2cread;\
    nop;\
    andi    v0, v0, 0x3
#define GET_SPD_ADDR_MIRROR_V1 \
    GET_SPD_MODULE_TYPE_V1;\
    beq     v0, MODULE_UDIMM_V1, 2f;\
    nop;\
    beq     v0, MODULE_RDIMM_V1, 1f;\
    nop;\
    beq     v0, MODULE_SODIMM_V1, 2f;\
    nop;\
1:  dli     a1, 0x88;\
    GET_I2C_NODE_ID_a2;\
    bal     i2cread;\
    nop;\
    andi    v0, v0, 0x1;\
    b       3f;\
    nop;\
2:  dli     a1, 0x83;\
    GET_I2C_NODE_ID_a2;\
    bal     i2cread;\
    nop;\
    andi    v0, v0, 0x1;\
3:
#define GET_SPD_DIMM_WIDTH_V1 \
    dli     a1, 0xd;\
    GET_I2C_NODE_ID_a2;\
    bal     i2cread;\
    nop;\
    andi    v0, v0, 0x3;\
    bnez    v0, 1f;\
    nop;\
    PRINTSTR("\r\nERROR: The DIMM Width is not supported (8bit width)!\r\n");\
    b       ERROR_TYPE;\
    nop;\
1:
#define GET_SPD_DIMM_ECC_V1 \
    dli     a1, 0xd;\
    GET_I2C_NODE_ID_a2;\
    bal     i2cread;\
    nop;\
    dsrl    v0, 3;\
    andi    v0, v0, 0x1
#define GET_SPD_BA_NUM_V1 \
    dli     a1, 0x4;\
    GET_I2C_NODE_ID_a2;\
    bal     i2cread;\
    nop;\
    dsrl    v0, 4;\
    andi    v0, v0, 0x1
#else
#define GET_SPD_SDRAM_WIDTH_V1 \
    dli     a1, 0x7;\
    GET_I2C_NODE_ID_a2;\
    bal     i2cread;\
    nop;\
    andi    v0, v0, 0x3;\
    bne     v0, 0x3, 1f;\
    nop;\
    PRINTSTR("\r\nError: SDRAM Width is 32bit, not supported!");\
    b       ERROR_TYPE;\
    nop;\
1:
#define GET_SPD_ADDR_MIRROR_V1 \
    dli     a1, 0x3f;\
    GET_I2C_NODE_ID_a2;\
    bal     i2cread;\
    nop;\
    andi    v0, v0, 0x1
#define GET_SPD_DIMM_WIDTH_V1 \
    dli     a1, 0x8;\
    GET_I2C_NODE_ID_a2;\
    bal     i2cread;\
    nop;\
    andi    v0, v0, 0x3;\
    bnez    v0, 1f;\
    nop;\
    PRINTSTR("\r\nERROR: The DIMM Width is not supported (8bit width)!\r\n");\
    b       ERROR_TYPE;\
    nop;\
1:
#define GET_SPD_DIMM_ECC_V1 \
    dli     a1, 0x8;\
    GET_I2C_NODE_ID_a2;\
    bal     i2cread;\
    nop;\
    dsrl    v0, 3;\
    andi    v0, v0, 0x1
#define GET_SPD_BA_NUM_V1 \
    dli     a1, 0x4;\
    GET_I2C_NODE_ID_a2;\
    bal     i2cread;\
    nop;\
    dsrl    v0, 4;\
    andi    v0, v0, 0x7;\
    beqz    v0, 1f;\
    nop;\
    PRINTSTR("\r\nError: Bank Number is more than 8, not supported!");\
    b       ERROR_TYPE;\
    nop;\
1:;\
    daddu   v0, v0, 1
#endif

#define GET_SPD_CID_NUM_V1 \
    dli     a1, 0x6;\
    GET_I2C_NODE_ID_a2;\
    bal     i2cread;\
    nop;\
    andi    t5, v0, 0x3;\
    beq     t5, 0x3, 1f;\
    nop;\
    li      v0, 0;\
    b       20f;\
    nop;\
1:;\
    dsrl    t5, v0, 4;\
    andi    t5, t5, 0x7;\
    beq     t5, 0x1, 3f;\
    nop;\
    beq     t5, 0x3, 3f;\
    nop;\
    beq     t5, 0x7, 3f;\
    nop;\
    b       11f;\
    nop;\
3:;\
    li      v0, 0;\
2:;\
    andi    a1, t5, 0x1;\
    beqz    a1, 20f;\
    nop;\
    daddu   v0, v0, 1;\
    dsrl    t5, t5, 1;\
    b       2b;\
    nop;\
11: PRINTSTR("\r\nERROR: SDRAM Package Type is not supported!\r\n");\
    nop; \
    b       ERROR_TYPE;\
    nop;\
20:
#ifdef  ONE_CS_PER_MC
#define GET_SPD_CS_NUM_V1   li v0, 0
#define GET_SPD_CS_MAP_V1   li v0, 1
#else
#ifndef DDR3_DIMM
#define GET_SPD_CS_NUM_V1 \
    dli     a1, 0xc;\
    GET_I2C_NODE_ID_a2;\
    bal     i2cread;\
    nop;\
    dsrl    v0, 3;\
    andi    v0, v0, 0x7
#define GET_SPD_CS_MAP_V1 \
    dli     a1, 0xc;\
    GET_I2C_NODE_ID_a2;\
    bal     i2cread;\
    nop;\
    dsrl    v0, 0x3; \
    daddu   v0, v0 ,1;\
    dli     t5, 1;\
    dsll    v0, t5, v0;\
    dsubu   v0, v0, 1
#else
#define GET_SPD_CS_NUM_V1 \
    dli     a1, 0x7;\
    GET_I2C_NODE_ID_a2;\
    bal     i2cread;\
    nop;\
    dsrl    v0, 3;\
    andi    v0, v0, 0x7
#define GET_SPD_CS_MAP_V1 \
    dli     a1, 0x7;\
    GET_I2C_NODE_ID_a2;\
    bal     i2cread;\
    nop;\
    dsrl    v0, 0x3; \
    daddu   v0, v0 ,1;\
    dli     t5, 1;\
    dsll    v0, t5, v0;\
    dsubu   v0, v0, 1
#endif
#endif
#define GET_SPD_SDRAM_CAPACITY_V1  \
    dli     a1, 0x4;\
    GET_I2C_NODE_ID_a2;\
    bal     i2cread;\
    nop;\
    andi    v0, v0, 0xf
#define GET_SPD_BG_NUM_V1 \
    dli     a1, 0x4;\
    GET_I2C_NODE_ID_a2;\
    bal     i2cread;\
    nop;\
    dsrl    v0, 6;\
    andi    v0, v0, 0x3
#define GET_SPD_SDRAM_TYPE_V1 \
    dli     a1, 0x2;\
    GET_I2C_NODE_ID_a2;\
    bal     i2cread;\
    nop;\
    andi    v0, v0, 0xff
#define GET_SPD_ROW_SIZE_V1 \
    dli     a1, 0x5;\
    GET_I2C_NODE_ID_a2;\
    bal     i2cread;\
    nop;\
    dsrl    v0, 3;\
    andi    v0, v0, 0x7;\
    dli     t5, 6;\
    bleu    v0, t5, 1f;\
    nop;\
    PRINTSTR("\r\nERROR: The SDRAM Row size is too big to support (>18)!\r\n");\
    b       ERROR_TYPE;\
    nop;\
1:  dsubu   v0, t5, v0
#define GET_SPD_COL_SIZE_V1 \
    dli     a1, 0x5;\
    GET_I2C_NODE_ID_a2;\
    bal     i2cread;\
    nop;\
    andi    v0, 0x7;\
    dli     t5, 3;\
    bleu    v0, t5, 1f;\
    nop;\
    PRINTSTR("\r\nERROR: The SDRAM Col size is too big to support (>12)!\r\n");\
    b       ERROR_TYPE;\
    nop;\
1:  dsubu   v0, t5, v0
#define GET_SPD_DIMM_MEMSIZE_V1 \
    GET_SPD_SDRAM_WIDTH_V1;\
    move    t2, v0;\
    daddu   t2, t2, 5;\
    GET_SPD_SDRAM_CAPACITY_V1;\
    move    t4, v0;\
    GET_DIMM_WIDTH_V1;\
    move    t6, a1;\
    daddu   t6, t6, 3;\
    beq     t4, 0x8, 1f;\
    nop;\
    beq     t4, 0x9, 2f;\
    nop;\
    dli     t5, 1;\
    daddu   t4, t4, 8;\
    b       3f;\
    nop;\
1:  dli     t5, 3;\
    dli     t4, 12;\
    b       3f;\
    nop;\
2:  dli     t5, 3;\
    dli     t4, 13;\
3:  daddu   t4, t4, t6;\
    dsubu   t4, t4, t2;\
    GET_SPD_CS_NUM_V1;\
    move    t2, v0;\
    daddu   t2, t2, 1;\
    dmulou  t2, t2, t5;\
    dli     t5, 0x1;\
    bgeu    t4, 10, 4f;\
    nop;\
    PRINTSTR("\r\nERROR: MEMSIZE is too small to support!(< 1GB)");\
    b       ERROR_TYPE;\
    nop;\
4:  dsubu   t4, t4, 10;\
    dsll    t4, t5, t4;\
    dmulou  t4, t4, t2;\
    move    v0, t4;\

#define GET_SPD_DIMM_MEMSIZE_1CS_V1 \
    GET_SPD_SDRAM_WIDTH_V1;\
    move    t2, v0;\
    daddu   t2, t2, 5;\
    GET_SPD_SDRAM_CAPACITY_V1;\
    move    t4, v0;\
    GET_DIMM_WIDTH_V1;\
    move    t6, a1;\
    daddu   t6, t6, 3;\
    beq     t4, 0x8, 1f;\
    nop;\
    beq     t4, 0x9, 2f;\
    nop;\
    dli     t5, 1;\
    daddu   t4, t4, 8;\
    b       3f;\
    nop;\
1:  dli     t5, 3;\
    dli     t4, 12;\
    b       3f;\
    nop;\
2:  dli     t5, 3;\
    dli     t4, 13;\
3:  daddu   t4, t4, t6;\
    dsubu   t4, t4, t2;\
    dli     t5, 0x1;\
    bgeu    t4, 10, 4f;\
    nop;\
    PRINTSTR("\r\nERROR: MEMSIZE is too small to support!(< 1GB)");\
    b       ERROR_TYPE;\
    nop;\
4:  dsubu   t4, t4, 10;\
    dsll    t4, t5, t4;\
    move    v0, t4;\

#define GET_CID_NUM_V1 \
    dli     a1, 0x7;\
    dsll    a1, a1, S1_CID_NUM_OFFSET_V1;\
    and     a1, s1, a1;\
    dsrl    a1, a1, S1_CID_NUM_OFFSET_V1
#define GET_BG_NUM_V1 \
    dli     a1, 0x3;\
    dsll    a1, a1, S1_BG_NUM_OFFSET_V1;\
    and     a1, s1, a1;\
    dsrl    a1, a1, S1_BG_NUM_OFFSET_V1
#define GET_BA_NUM_V1 \
    dli     a1, 0x1;\
    dsll    a1, a1, S1_BA_NUM_OFFSET_V1;\
    and     a1, s1, a1;\
    dsrl    a1, a1, S1_BA_NUM_OFFSET_V1
#define GET_ROW_SIZE_V1 \
    dli     a1, 0x7;\
    dsll    a1, a1, S1_ROW_SIZE_OFFSET_V1;\
    and     a1, a1, s1;\
    dsrl    a1, a1, S1_ROW_SIZE_OFFSET_V1
#define GET_COL_SIZE_V1 \
    dli     a1, 0x3;\
    dsll    a1, a1, S1_COL_SIZE_OFFSET_V1;\
    and     a1, a1, s1;\
    dsrl    a1, a1, S1_COL_SIZE_OFFSET_V1
#define GET_ADDR_MIRROR_V1 \
    dli     a1, 0x1;\
    dsll    a1, a1, S1_ADDR_MIRROR_OFFSET_V1;\
    and     a1, a1, s1;\
    dsrl    a1, a1, S1_ADDR_MIRROR_OFFSET_V1
#define GET_DIMM_MEMSIZE_V1 \
    dli     a1, 0xfff;\
    dsll    a1, a1, S1_DIMM_MEMSIZE_OFFSET_V1;\
    and     a1, a1, s1;\
    dsrl    a1, a1, S1_DIMM_MEMSIZE_OFFSET_V1
#define GET_DIMM_WIDTH_V1   \
    dli     a1, 0x3;\
    dsll    a1, a1, S1_DIMM_WIDTH_OFFSET_V1;\
    and     a1, a1, s1;\
    dsrl    a1, a1, S1_DIMM_WIDTH_OFFSET_V1
#define GET_DIMM_ECC_V1    \
    dli     a1, 0x1;\
    dsll    a1, a1, S1_DIMM_ECC_OFFSET_V1;\
    and     a1, a1, s1;\
    dsrl    a1, a1, S1_DIMM_ECC_OFFSET_V1
#define GET_DIMM_TYPE_V1   \
    dli     a1, 0x3;\
    dsll    a1, a1, S1_DIMM_TYPE_OFFSET_V1;\
    and     a1, a1, s1;\
    dsrl    a1, a1, S1_DIMM_TYPE_OFFSET_V1
#define GET_SDRAM_WIDTH_V1 \
    dli     a1, 0x3;\
    dsll    a1, a1, S1_SDRAM_WIDTH_OFFSET_V1;\
    and     a1, a1, s1;\
    dsrl    a1, a1, S1_SDRAM_WIDTH_OFFSET_V1
#define GET_SDRAM_TYPE_V1  \
    dli     a1, 0x7;\
    dsll    a1, a1, S1_SDRAM_TYPE_OFFSET_V1;\
    and     a1, a1, s1;\
    dsrl    a1, a1, S1_SDRAM_TYPE_OFFSET_V1
#define GET_MC_CS_MAP_V1   \
    dli     a1, 0xff;\
    dsll    a1, a1, S1_MC_CS_MAP_OFFSET_V1;\
    and     a1, a1, s1;\
    dsrl    a1, a1, S1_MC_CS_MAP_OFFSET_V1
#define GET_NODE_ID_V1 \
    dli     a1, 0xf;\
    and     a1, a1, s1
#define GET_I2C_ADDR_SLOT_V1(slotid) \
    dli     a1, 0xf;\
    dsll    a1, a1, (S1_I2C_ADDR_OFFSET_V1+slotid<<2);\
    and     a1, a1, s1;\
    dsrl    a1, a1, (S1_I2C_ADDR_OFFSET_V1+slotid<<2)
#define GET_DIMM_CONFIG_V1 \
    dli     a1, 0x3;\
    dsll    a1, a1, S1_DIMM_CONFIG_OFFSET_V1;\
    and     a1, a1, s1;\
    dsrl    a1, a1, S1_DIMM_CONFIG_OFFSET_V1
#define GET_I2C_ADDR_V1 \
    GET_DIMM_CONFIG_V1; \
    beq     a1, 0x2, 1f; \
    nop;\
    dli     a1, 0xf; \
    dsll    a1, a1, S1_I2C_ADDR_OFFSET_V1; \
    and     a1, a1, s1;\
    dsrl    a1, a1, S1_I2C_ADDR_OFFSET_V1;\
    b       2f;\
    nop;\
1:;\
    dli     a1, 0xf; \
    dsll    a1, a1, (S1_I2C_ADDR_OFFSET_V1+4); \
    and     a1, a1, s1;\
    dsrl    a1, a1, (S1_I2C_ADDR_OFFSET_V1+4);\
2:

#define MC_MEMSIZE_MASK_V1 0xfff
#define S1_COMPARE_MASK 0x

#define GET_MC_ENABLE 0xffffffff0000

#define S1_MC0_MEMSIZE_OFFSET_V1 4
#define S1_MC1_MEMSIZE_OFFSET_V1 16

#define GET_MC0_MEMSIZE_V1      \
dli     a1, MC_MEMSIZE_MASK_V1;\
dsll    a1, a1, S1_MC0_MEMSIZE_OFFSET_V1;\
and     a1, s1, a1;\
dsrl    a1, a1, S1_MC0_MEMSIZE_OFFSET_V1;

#define GET_MC1_MEMSIZE_V1      \
dli     a1, MC_MEMSIZE_MASK_V1;\
dsll    a1, a1, S1_MC1_MEMSIZE_OFFSET_V1;\
and     a1, s1, a1;\
dsrl    a1, a1, S1_MC1_MEMSIZE_OFFSET_V1;

#define S1_VALUE \
(0x0   << S1_CID_NUM_OFFSET_V1     )|\
(0x2   << S1_BG_NUM_OFFSET_V1      )|\
(0x0   << S1_BA_NUM_OFFSET_V1      )|\
(0x2   << S1_ROW_SIZE_OFFSET_V1    )|\
(0x2   << S1_COL_SIZE_OFFSET_V1    )|\
(0x1   << S1_ADDR_MIRROR_OFFSET_V1 )|\
(0x2   << S1_DIMM_MEMSIZE_OFFSET_V1)|\
(0x1   << S1_DIMM_WIDTH_OFFSET_V1  )|\
(0x0   << S1_DIMM_ECC_OFFSET_V1    )|\
(0x0   << S1_DIMM_TYPE_OFFSET_V1   )|\
(0x1   << S1_SDRAM_WIDTH_OFFSET_V1 )|\
(0x4   << S1_SDRAM_TYPE_OFFSET_V1  )|\
(0x10  << S1_MC_CS_MAP_OFFSET_V1   )|\
(0x1   << S1_I2C_ADDR_OFFSET_V1    )

#define S3_VALUE \
(0x0   << S1_CID_NUM_OFFSET_V1     )|\
(0x2   << S1_BG_NUM_OFFSET_V1      )|\
(0x0   << S1_BA_NUM_OFFSET_V1      )|\
(0x2   << S1_ROW_SIZE_OFFSET_V1    )|\
(0x2   << S1_COL_SIZE_OFFSET_V1    )|\
(0x1   << S1_ADDR_MIRROR_OFFSET_V1 )|\
(0x8   << S1_DIMM_MEMSIZE_OFFSET_V1)|\
(0x3   << S1_DIMM_WIDTH_OFFSET_V1  )|\
(0x0   << S1_DIMM_ECC_OFFSET_V1    )|\
(0x0   << S1_DIMM_TYPE_OFFSET_V1   )|\
(0x1   << S1_SDRAM_WIDTH_OFFSET_V1 )|\
(0x4   << S1_SDRAM_TYPE_OFFSET_V1  )|\
(0x10  << S1_MC_CS_MAP_OFFSET_V1   )|\
(0x3   << S1_I2C_ADDR_OFFSET_V1    )
#endif
#if 0
    bleu    v0, 0x3, 1f;\
    nop;\
    PRINTSTR("\r\n ERROR: CS_NUM is not in support range (> 4)!\r\n");\
    nop; \
    b       ERROR_TYPE;\
    nop;\
1:  bne     v0, 0x2, 2f;\
    nop;\
    PRINTSTR("\r\n ERROR: CS_NUM is not in support range (= 3)!\r\n");\
    nop; \
    b       ERROR_TYPE;\
    nop;\
2:
    
#endif
