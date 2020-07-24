/*************************
    Author: Huangshuai
    Funtion: detect all four(maximum) DIMM slots of MC1 and MC0 of one NODE and store the information in s1
    v1.0
    Limit : now only used for 3A4000
    input : s1
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
|[07:04]| RESERVED           | 4'b0    | RESERVED          |
|[03:00]| NODE_ID            | 4'hx    | x                 |
------------------------------------------------------------
    output : s1
------------------------------------------------------------
|[63:56]| MC1_CSMAP          | 8'bx    | CS7-CS0           |
|[55:44]| MC1_MEMSIZE        |12'bx    | x*1G              |
|[43:40]| MC1_I2C_ADDR       | 4'bx    | x                 |
|[39:36]| RESERVED           | 4'b0    | RESERVED          |
|[35:32]| RESERVED           | 4'b0    | RESERVED          |
|[31:24]| MC0_CSMAP          | 8'bx    | CS7-CS0           |
|[23:12]| MC0_MEMSIZE        |12'bx    | x*1G              |
|[11:08]| MC0_I2C_ADDR       | 4'bx    | x                 |
|[07:04]| RESERVED           | 4'b0    | RESERVED          |
|[03:00]| NODE_ID            | 4'hx    | x                 |

***********************************************************/

//------------------------
#define GET_MC1_SLOT1_ID dsrl a1, t1, 28; and a1, a1, 0xf;
#define GET_MC1_SLOT0_ID dsrl a1, t1, 24; and a1, a1, 0xf;
#define GET_MC0_SLOT1_ID dsrl a1, t1, 20; and a1, a1, 0xf;
#define GET_MC0_SLOT0_ID dsrl a1, t1, 16; and a1, a1, 0xf;
#ifdef  MULTI_I2C_BUS
#define GET_I2C_NODE_ID_a2 dsrl a2, t1, 4; and a2, a2, 0x3;
#else
#define GET_I2C_NODE_ID_a2  ;
#endif
//------------------------
#define DEBUG_PROBE_NODE_DIMM

LEAF(PROBE_NODE_DIMM)
/*************************
    use registers:
    a0, a1, a2, a3
    v0, v1
    t0: store MC1 DIMM infor during detect MC0 DIMM
    t1: store s1
    t2: reconstruct s1
    t3, t4: store DIMM infor temporary, should be reserved by PROBE_DIMM
    t5: by child PROBE_DIMM
    t6: temp variable
    t7: by child PROBE_DIMM
    t8: store ra

    child must reserve: t0, t1, t3, t8, s1
    i2cread must reserve: a0, t0, t1, t3, t5, t7, t8, s1
*************************/
    move    t8, ra
    
    move    t1, s1
    dli     t2, 0x0
#if 0  //for debug, give the SPD device id directly.
//scan the devices and display DIMM SPD values when the first device is detected.
    PRINTSTR("\r\nDIMM SPD register dump:");
    dli     a0, 0xa1;
    dli     a1, 0x2;
    GET_I2C_NODE_ID_a2
    bal     i2cread;
    nop;
    dli     t3, 0x80
    bltu    v0, t3, 2f
    nop
    dli     a0, 0xa3;
    dli     a1, 0x2;
    GET_I2C_NODE_ID_a2
    bal     i2cread;
    nop;
    dli     t3, 0x80
    bltu    v0, t3, 2f
    nop
    dli     a0, 0xa5;
    dli     a1, 0x2;
    GET_I2C_NODE_ID_a2
    bal     i2cread;
    nop;
    dli     t3, 0x80
    bltu    v0, t3, 2f
    nop
    dli     a0, 0xa7;
    dli     a1, 0x2;
    GET_I2C_NODE_ID_a2
    bal     i2cread;
    nop;
    dli     t3, 0x80
    bltu    v0, t3, 2f
    nop
    b       3f
    nop
2:
    move    t3, a0
    PRINTSTR("\r\na0=0x");
    move    a0, t3
    bal     hexserial
    nop
    PRINTSTR("\r\n");
    dli     t0, 0x0; //used as counter
1:
    move    a0, t0;
    bal     hexserial
    nop
    PRINTSTR(": ");
    move    a0, t3
    move    a1, t0;
    GET_I2C_NODE_ID_a2
    bal     i2cread;
    nop;
    move    a0, v0
    bal     hexserial
    nop
    PRINTSTR("\r\n");
    nop
    
    dli     a1, 0x80
    daddiu  t0, 0x1;
    bne     t0, a1, 1b;
    nop
3:  
    PRINTSTR("\r\n^^^^^^^^^^^^^^^^^^^^^^^^^^^^\r\n");
#endif 

    dli     a1, 0xff
    and     t0, t1, a1
//-------------------------------------
11:
//detect MC0 if not define MC1_ONLY
    GET_MC0_ENABLE
    beqz    a1, 12f
    nop
    //do auto probe DIMM
    PRINTSTR("\r\nProbing DDR MC0 SLOT: ");
    PRINTSTR("\r\nProbe MC0 slot 0.");
    dli     a1, 0xf
    and     s1, s1, a1
    GET_MC0_SLOT0_ID
    dli     a0, 0x8
    bgeu    a1, a0, 1f  //invalidate device id
    nop
    dsll    a1, a1, 1
    ori     a0, a1, 0xa1
    bal     PROBE_DIMM;
    nop;
1:
#ifdef  DEBUG_PROBE_NODE_DIMM
    /* show value of s1 */
    PRINTSTR(" s1 = 0x");
    dsrl    a0, s1, 32
    bal     hexserial
    nop
    PRINTSTR("__")
    move    a0, s1
    bal     hexserial
    nop;
    PRINTSTR("\r\n")
#endif
    //store slot 0 DIMM infor in t3
    move    t3, s1

    PRINTSTR("\r\nProbe MC0 slot 1.");
    dli     a1, 0xf
    and     s1, s1, a1
    GET_MC0_SLOT1_ID
    dli     a0, 0x8
    bgeu    a1, a0, 1f  //invalidate device id
    nop
    dsll    a1, a1, 1
    ori     a0, a1, 0xa1
    bal     PROBE_DIMM;
    nop;
1:
#ifdef  DEBUG_PROBE_NODE_DIMM
    /* show value of s1 */
    PRINTSTR(" s1 = 0x");
    dsrl    a0, s1, 32
    bal     hexserial
    nop
    PRINTSTR("__")
    move    a0, s1
    bal     hexserial
    nop;
    PRINTSTR("\r\n")
#endif
    //store slot 1 DIMM infor in t4
    move    t4, s1

    //compare the two slot DIMM infor and merge the CS_MAP and MC0_MEMSIZE if necessary
    move    s1, t3
    GET_DIMM_MEMSIZE
    beqz    a1, 1f
    nop
    move    s1, t4
    GET_DIMM_MEMSIZE
    beqz    a1, 2f
    nop
    //both slot 0 and 1 has DIMM
    //step 1: compare the two DIMM type infor, if they differs, display errors
    xor     a1, t3, t4
    dli     a0, 0xffffffff000
    and     a1, a1, a0
    bnez    a1, 9f
    nop
    //step 2: if the two DIMM types are equal, merge the CS_MAP and MC0_MEMSIZE
    //1. Calculate new CS_MAP
    move    s1, t4
    GET_MC_CS_MAP
    dsll    a0, a1, 4
    move    s1, t3
    GET_MC_CS_MAP
    or      t6, a0, a1
    dsll    t6, t6, S1_MC_CS_MAP_OFFSET    //store new MC_MAP in t6, don't move to s1 imediately because when merge MC_MEMSIZE, s1 will be damaged
    //2. merge MC0_MEMSIZE
    move    s1, t3
    GET_DIMM_MEMSIZE
    move    a0, a1
    move    s1, t4
    GET_DIMM_MEMSIZE
    daddu   a1, a1, a0
    dli     a0, MC_MEMSIZE_MASK //make sure a1 not exceed its range
    and     a1, a1, a0
    dsll    a1, a1, S1_DIMM_MEMSIZE_OFFSET
    dli     a0, MC_MEMSIZE_MASK
    dsll    a0, a0, S1_DIMM_MEMSIZE_OFFSET
    not     a0, a0
    and     s1, s1, a0
    or      s1, s1, a1
#ifdef  DEBUG_PROBE_NODE_DIMM
    /* show value of s1 */
    PRINTSTR("\r\n T3 s1 = 0x");
    dsrl    a0, s1, 32
    bal     hexserial
    nop
    PRINTSTR("__")
    move    a0, s1
    bal     hexserial
    nop;
    PRINTSTR("\r\n")
#endif
    //3. merge new MC_CS_MAP(in t6) to s1, do not affect other bits
    dli     a0, 0xf
    dsll    a0, a0, S1_MC_CS_MAP_OFFSET
    not     a0, a0
    and     s1, s1, a0
    or      s1, s1, t6
    b       3f
    nop
9:      //two slot have different type DIMM, give ERROR message and use slot 0 only
    PRINTSTR("MC0 has two different DIMM, please use same DIMM!!\r\n")
    PRINTSTR("Currently system will use only slot 0!!\r\n")
    b       2f
    nop
2:  //no DIMM in slot 1 and slot 0 has DIMM
    move    s1, t3
    b       4f
    nop
1:  //no DIMM in slot 0
    move    s1, t4
    GET_SDRAM_TYPE
    beqz    a1, 1f
    nop
    //only slot 1 has DIMM, firstly shift the CS_MAP to upper 2 bit
    move    s1, t4
    GET_MC_CS_MAP
    dsll    a1, a1, (S1_MC_CS_MAP_OFFSET + 4)
    dli     a0, 0xff
    dsll    a0, a0, S1_MC_CS_MAP_OFFSET
    not     a0, a0
    and     s1, s1, a0
    or      s1, s1, a1
    b       4f
    nop
4:  //move DIMM_MEMSIZE to MC0_MEMSIZE
//    GET_DIMM_MEMSIZE
//    dsll    a1, a1, S1_DIMM_MEMSIZE_OFFSET
//    dli     a0, MC_MEMSIZE_MASK
//    dsll    a0, a0, S1_DIMM_MEMSIZE_OFFSET
//    not     a0, a0
//    and     s1, s1, a0
//    or      s1, s1, a1
//    b       3f
    nop
1:  //no DIMM in slot 0 and 1   
    PRINTSTR("\r\nNO DIMM in MC0 slot.\r\n");
    b       3f
    nop
3:
#ifdef  DEBUG_PROBE_NODE_DIMM
    /* show value of s1 */
    PRINTSTR("\r\n T5 s1 = ");
    dsrl    a0, s1, 32
    bal     hexserial
    nop
    PRINTSTR("__")
    move    a0, s1
    bal     hexserial
    nop;
    PRINTSTR("\r\n")
    /* show value of t0 */
    PRINTSTR("\r\n t0 = 0x");
    dsrl    a0, t0, 32
    bal     hexserial
    nop
    PRINTSTR("__")
    move    a0, t0
    bal     hexserial
    nop;
    PRINTSTR("\r\n")
#endif
12:
    move    ra, t8
    jr      ra
    nop
END(PROBE_NODE_DIMM)

/*************************
PROBE_DIMM:
function: probe the given slot(I2C device id is given in a0),
      if there is no DIMM in this slot, clear SDRAM_TYPE to 0,
      else read the DIMM infor from the SPD and store the infor
      in s1(CS_MAP at s1[S1_MC_CS_MAP_OFFSET+2, S1_MC_CS_MAP_OFFSET], 
      MEMSIZE at s1[DIMM_MEMSIZE_OFFSET+7: DIMM_MEMSIZE_OFFSET]).
note: don't change t0, t1, t3, t8, s1

use register:
a0,a1,a2,a3
v0,v1
t5, t7

input: a0, t1
    a0:i2c device id(don't change it).
    t1[5:4]: NODE_ID
usage:
a1: register offset of i2c device
a2: I2C NODE ID
t5: temp vary.
t7: store ra

+++child must reserve: t7.

    child must reserve: a0, t0, t1, t3, t5, t7, t8, s1
*************************/
#if 0   //debug code, used in PROBE_DIMM, after read i2c, print v0
    //debug----------
    move    t5, a0
    PRINTSTR("\r\na0=0x");
    move    a0, t5
    bal     hexserial
    nop
    PRINTSTR("\r\n");
    move    a0, t5
    //------------debug

    //Test whether i2cread will dead loop
    move    t5, a0
    PRINTSTR("\r\nIn Probe_DIMM, before i2cread!")
    move    a0, t5
    dli     a1, 0
    GET_I2C_NODE_ID_a2
    bal     i2cread
    nop
    move    t5, a0
    PRINTSTR("\r\nIn Probe_DIMM, after i2cread!")
    move    a0, t5
#endif
LEAF(PROBE_DIMM) 
    move    t7, ra

//read the i2c spd for learn,read data is abandon
    dli     a1, 0
    GET_I2C_NODE_ID_a2
    bal     i2cread
    nop

    GET_SPD_SDRAM_TYPE
    beq     v0, SDRAM_DDR3, DDR3
    nop
    beq     v0, SDRAM_DDR4, DDR4
    nop

    PRINTSTR("\r\nNO DIMM in this slot.\r\n")
    b       ERROR_TYPE
    nop

DDR3://TODO
    dli     t5, 0x3
    dsll    t5, t5, S1_SDRAM_TYPE_OFFSET
    or      s1, s1, t5
    b       1f
    nop

DDR4:
    dli     t5, 0x4
    dsll    t5, t5, S1_SDRAM_TYPE_OFFSET
    or      s1, s1, t5

1:

//probe MODULE TYPE
    GET_SPD_MODULE_TYPE
    beq     v0, MODULE_UDIMM, UDIMM
    nop
    beq     v0, MODULE_RDIMM, RDIMM
    nop
    beq     v0, MODULE_SODIMM, UDIMM    //SODIMM, deal as UDIMM
    nop
    PRINTSTR("\r\nERROR: DIMM type is not in support range(UDIMM or RDIMM).\r\n")
    b       ERROR_TYPE
    nop

RDIMM:
    dli     t5, 0x1//RDIMM
    dsll    t5, t5, S1_DIMM_TYPE_OFFSET
    or      s1, s1, t5
    b       1f
    nop

UDIMM:
    dli     t5, 0x0//UDIMM
    dsll    t5, t5, S1_DIMM_TYPE_OFFSET
    or      s1, s1, t5
    b       1f
    nop
1:  

//probe BG NUM, save actual value
    GET_SPD_BG_NUM
    move    t5, v0
    dsll    t5, t5, S1_BG_NUM_OFFSET
    or      s1, s1, t5

//probe BA NUM, save value as 0-4bank 1-8bank
    GET_SPD_BA_NUM
    move    t5, v0
    dsll    t5, t5, S1_BA_NUM_OFFSET
    or      s1, s1, t5

//probe physical rank
    GET_SPD_CS_MAP
    move    t5, v0
    dsll    t5, t5, S1_MC_CS_MAP_OFFSET
    or      s1, s1, t5

//probe dram_density, calculate memsize
    GET_SPD_DIMM_MEMSIZE
    move    t5, v0
    dsll    t5, t5, S1_DIMM_MEMSIZE_OFFSET
    or      s1, s1, t4

//probe dram width, save in s1
    GET_SPD_SDRAM_WIDTH
    move    t5, v0
    dsll    t5, t5, S1_SDRAM_WIDTH_OFFSET
    or      s1, s1, t5

//probe module width, save ecc in s1
    GET_SPD_DIMM_WIDTH
    move    t5, v0
    dsll    t5, t5, S1_DIMM_WIDTH_OFFSET
    or      s1, s1, t5
    GET_SPD_DIMM_ECC
    move    t5, v0
    dsll    t5, t5, S1_DIMM_ECC_OFFSET
    or      s1, s1, t5

//probe dram row size
    GET_SPD_ROW_SIZE
    move    t5, v0
    dsll    t5, t5, S1_ROW_SIZE_OFFSET
    or      s1, s1, t5

//probe dram col size
    GET_SPD_COL_SIZE
    move    t5, v0
    dsll    t5, t5, S1_COL_SIZE_OFFSET
    or      s1, s1, t5

//probe addr mirror
    GET_SPD_ADDR_MIRROR
    move    t5, v0
    dsll    t5, t5, S1_ADDR_MIRROR_OFFSET
    or      s1, s1, t5

ERROR_TYPE:
//no DIMM or unrecognized DIMM in this slot
    dli     t5, 0xff
    dsll    t5, t5, S1_MC_CS_MAP_OFFSET
    not     t5, t5
    and     s1, s1, t5
probe_dimm_end:
    move    ra, t7
    jr      ra
    nop
END(PROBE_DIMM)


LEAF(READ_DIMM_IDENTIFIER)
/********************
input:
    a0: bit[3:0] i2c addr
    a2: i2c bus node id
output:
    t4/t5: dimm indentifier
use: a0~a2, v0, v1, t8
********************/
    move    t8, ra

    not     t4, $0
    not     t5, $0
    dli     a1, 0x8
    bgeu    a0, a1, 8f
    nop
    move    t4, $0
    move    t5, $0
    dsll    a0, a0, 1
    ori     a0, a0, 0xa1
    //test no meaning
    dli     a1, 0
    bal     i2cread
    nop
#ifdef  DDR3_DIMM
    dli     a1, 2
    bal     i2cread
    nop
    and     v0, v0, 0xff
    dli     a1, 0x0b
    beq     v0, a1, 1f
    nop
    not     t4, $0
    not     t5, $0
    b       8f
    nop
1:
    //read manufater JEDEC ID
    dli     a1, 117
    bal     i2cread
    nop
    and     v0, v0, 0xff
    or      t4, t4, v0
    dsll    t4, t4, 8
    daddu   a1, a1, 1
    bal     i2cread
    nop
    and     v0, v0, 0xff
    or      t4, t4, v0
    //read other info
    daddu   a1, a1, 1
    bal     i2cread
    nop
    and     v0, v0, 0xff
    or      t5, t5, v0
    dsll    t5, t5, 8
    daddu   a1, a1, 1
    bal     i2cread
    nop
    and     v0, v0, 0xff
    or      t5, t5, v0
    dsll    t5, t5, 8
    daddu   a1, a1, 1
    bal     i2cread
    nop
    and     v0, v0, 0xff
    or      t5, t5, v0
    dsll    t5, t5, 8
    daddu   a1, a1, 1
    bal     i2cread
    nop
    and     v0, v0, 0xff
    or      t5, t5, v0
    dsll    t5, t5, 8
    daddu   a1, a1, 1
    bal     i2cread
    nop
    and     v0, v0, 0xff
    or      t5, t5, v0
    dsll    t5, t5, 8
    daddu   a1, a1, 1
    bal     i2cread
    nop
    and     v0, v0, 0xff
    or      t5, t5, v0
    dsll    t5, t5, 8
    daddu   a1, a1, 1
    bal     i2cread
    nop
    and     v0, v0, 0xff
    or      t5, t5, v0
    dsll    t5, t5, 8
#else   //DDR2
    dli     a1, 2
    bal     i2cread
    nop
    and     v0, v0, 0xff
    dli     a1, 0x08
    beq     v0, a1, 1f
    nop
    not     t4, $0
    not     t5, $0
    b       8f
    nop
1:
    //read manufater JEDEC ID
    dli     a1, 64
    bal     i2cread
    nop
    and     v0, v0, 0xff
    or      t4, t4, v0
    dsll    t4, t4, 8
    daddu   a1, a1, 1
    bal     i2cread
    nop
    and     v0, v0, 0xff
    or      t4, t4, v0
    dsll    t4, t4, 8
    dli     a1, 64
    bal     i2cread
    nop
    and     v0, v0, 0xff
    or      t4, t4, v0
    dsll    t4, t4, 8
    daddu   a1, a1, 1
    bal     i2cread
    nop
    and     v0, v0, 0xff
    or      t4, t4, v0
    dsll    t4, t4, 8
    dli     a1, 64
    bal     i2cread
    nop
    and     v0, v0, 0xff
    or      t4, t4, v0
    dsll    t4, t4, 8
    daddu   a1, a1, 1
    bal     i2cread
    nop
    and     v0, v0, 0xff
    or      t4, t4, v0
    dsll    t4, t4, 8
    dli     a1, 64
    bal     i2cread
    nop
    and     v0, v0, 0xff
    or      t4, t4, v0
    dsll    t4, t4, 8
    daddu   a1, a1, 1
    bal     i2cread
    nop
    and     v0, v0, 0xff
    or      t4, t4, v0
    //read other info
    daddu   a1, a1, 1
    bal     i2cread
    nop
    and     v0, v0, 0xff
    or      t5, t5, v0
    dsll    t5, t5, 8
    daddu   a1, a1, 1
    bal     i2cread
    nop
    and     v0, v0, 0xff
    or      t5, t5, v0
    dsll    t5, t5, 8
    daddu   a1, a1, 1
    bal     i2cread
    nop
    and     v0, v0, 0xff
    or      t5, t5, v0
    dsll    t5, t5, 8
    daddu   a1, a1, 1
    bal     i2cread
    nop
    and     v0, v0, 0xff
    or      t5, t5, v0
    dsll    t5, t5, 8
    daddu   a1, a1, 1
    bal     i2cread
    nop
    and     v0, v0, 0xff
    or      t5, t5, v0
    dsll    t5, t5, 8
    daddu   a1, a1, 1
    bal     i2cread
    nop
    and     v0, v0, 0xff
    or      t5, t5, v0
    dsll    t5, t5, 8
    daddu   a1, a1, 1
    bal     i2cread
    nop
    and     v0, v0, 0xff
    or      t5, t5, v0
    dsll    t5, t5, 8
#endif
8:

    move    ra, t8
    jr      ra
    nop
END(READ_DIMM_IDENTIFIER)

#ifdef  MULTI_I2C_BUS
#define GET_I2C_NODE_ID_a2_1 dsrl a2, s1, 4; and a2, a2, 0x3;
#else
#define GET_I2C_NODE_ID_a2_1  ;
#endif

LEAF(CHECK_DIMM_CHANGE)
/**********************
    check whether the 2 DIMMs are changed
input:
    s1: NODE_ID, I2C node ID
    t1: MC slot I2C addr
    t7: mc_level_info address
output:
    v0: return value
register usage:
    t9: save ra
    t8: by child READ_DIMM_IDENTIFIER
    t4/t5: dimm info
**********************/

    move    t9, ra

    //check slot 0 DIMM
    and     a0, t1, 0xf
    GET_I2C_NODE_ID_a2_1
    bal     READ_DIMM_IDENTIFIER
    nop
    ld      a1, 0x8(t7)
    bne     t4, a1, do_arb_level
    nop
    ld      a1, 0x10(t7)
    bne     t5, a1, do_arb_level
    nop
    //check slot 1 DIMM
    dsrl    a0, t1, 4
    and     a0, a0, 0xf
    GET_I2C_NODE_ID_a2_1
    bal     READ_DIMM_IDENTIFIER
    nop
    ld      a1, 0x18(t7)
    bne     t4, a1, do_arb_level
    nop
    ld      a1, 0x20(t7)
    bne     t5, a1, do_arb_level
    nop

//do_not_arb_level:
    move    v0, $0
    move    ra, t9
    jr      ra
    nop

do_arb_level:
    or      v0, $0, 0x1
    move    ra, t9
    jr      ra
    nop
END(CHECK_DIMM_CHANGE)
