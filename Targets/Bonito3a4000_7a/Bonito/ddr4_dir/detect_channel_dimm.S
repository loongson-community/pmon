/*************************
    Author: Huangshuai
    Funtion: detect all four(maximum) DIMM slots of MC1 and MC0 of one NODE and store the information in s1
    Origin : Chenxinke detect_node_dimm_all.S
    Change : detect dimm in one channel
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
|[07:04]| DIMM_WIDTH         | 4'h0    | RESERVED          |
|       |                    | 4'h1    | 16bit             |
|       |                    | 4'h2    | 32bit             |
|       |                    | 4'h3    | 64bit             |
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
#define GET_MC_SLOT1_ID dsrl a1, t1, 20; and a1, a1, 0xf;
#define GET_MC_SLOT0_ID dsrl a1, t1, 16; and a1, a1, 0xf;
#ifdef  MULTI_I2C_BUS
#define GET_I2C_NODE_ID_a2 and a2,t1,0xf; dsll a2, 44;
#else
#define GET_I2C_NODE_ID_a2 dli a2, 0x0;
#endif
//------------------------
#define DEBUG_PROBE_NODE_DIMM

LEAF(PROBE_CHANNEL_DIMM)
/*************************
    use registers:
    a0, a1, a2, a3
    v0, v1
    t0: store MC1 DIMM infor during detect MC0 DIMM
    t1: store s1
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

    GET_I2C_NODE_ID_a2
    bal     i2cinit
    nop

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

    dli     a1, 0xf
    and     t0, t1, a1
//-------------------------------------
11:
//detect MC0 if not define MC1_ONLY
//    GET_MC0_ENABLE
//    beqz    a1, 12f
//    nop
    //do auto probe DIMM
//    PRINTSTR("\r\nProbing DDR MC0 SLOT: ");
//    PRINTSTR("\r\nProbe MC0 slot 0.");
    dli     a1, 0xf
    and     s1, s1, a1
    GET_MC_SLOT0_ID
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
    PRINTSTR("\r\nSlot 0: s1 = 0x");
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

//    PRINTSTR("\r\nProbe MC0 slot 1.");
    dli     a1, 0xf
    and     s1, s1, a1
    GET_MC_SLOT1_ID
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
    PRINTSTR("\r\nSlot 1: s1 = 0x");
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
    GET_DIMM_MEMSIZE_V1
    beqz    a1, 1f
    nop
    move    s1, t4
    GET_DIMM_MEMSIZE_V1
    beqz    a1, 2f
    nop
    //both slot 0 and 1 has DIMM
    //step 1: compare the two DIMM type infor, if they differs, display errors
    xor     a1, t3, t4
//    dli     a0, S1_COMPARE_MASK
//    and     a1, a1, a0
    bnez    a1, 9f
    nop
    //step 2: if the two DIMM types are equal, merge the CS_MAP and MC0_MEMSIZE
    //1. Calculate new CS_MAP
    move    s1, t4
    GET_MC_CS_MAP_V1
    dsll    a0, a1, 4
    move    s1, t3
    GET_MC_CS_MAP_V1
    or      t6, a0, a1
    dsll    t6, t6, S1_MC_CS_MAP_OFFSET_V1    //store new MC_MAP in t6, don't move to s1 imediately because when merge MC_MEMSIZE, s1 will be damaged
    //2. merge MC0_MEMSIZE
    move    s1, t3
    GET_DIMM_MEMSIZE_V1
    move    a0, a1
    move    s1, t4
    GET_DIMM_MEMSIZE_V1
    daddu   a1, a1, a0
    dli     a0, MC_MEMSIZE_MASK_V1 //make sure a1 not exceed its range
    and     a1, a1, a0
    dsll    a1, a1, S1_DIMM_MEMSIZE_OFFSET_V1
    dli     a0, MC_MEMSIZE_MASK_V1
    dsll    a0, a0, S1_DIMM_MEMSIZE_OFFSET_V1
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
    dli     a0, 0xff
    dsll    a0, a0, S1_MC_CS_MAP_OFFSET_V1
    not     a0, a0
    and     s1, s1, a0
    or      s1, s1, t6
//    //4. merge i2c addr
//    GET_MC_SLOT0_ID
//    dsll    a1, a1, S1_I2C_ADDR_OFFSET_V1
//    dli     a0, 0xf
//    dsll    a0, a0, S1_I2C_ADDR_OFFSET_V1
//    not     a0, a0
//    and     s1, s1, a0
//    or      s1, s1, a1
    //config DIMM config
    dli     a1, 0x3
    dsll    a1, S1_DIMM_CONFIG_OFFSET_V1
    dli     a0, 0x3
    dsll    a0, S1_DIMM_CONFIG_OFFSET_V1
    not     a0, a0
    and     s1, s1, a0
    or      s1, s1, a1

    b       3f
    nop
9:      //two slot have different type DIMM, give ERROR message and use slot 0 only
    PRINTSTR("The probed MC has two different DIMM, please use same DIMM!!\r\n")
    PRINTSTR("Currently system will use only slot 0!!\r\n")
    b       2f
    nop
2:  //no DIMM in slot 1 and slot 0 has DIMM
    move    s1, t3
    b       4f
    nop
1:  //no DIMM in slot 0
    move    s1, t4
    GET_DIMM_MEMSIZE_V1
    beqz    a1, 1f
    nop
    //only slot 1 has DIMM, firstly shift the CS_MAP to upper 4 bit
    move    s1, t4
    GET_MC_CS_MAP_V1
    dsll    a1, a1, (S1_MC_CS_MAP_OFFSET_V1 + 4)
    dli     a0, 0xff
    dsll    a0, a0, S1_MC_CS_MAP_OFFSET_V1
    not     a0, a0
    and     s1, s1, a0
    or      s1, s1, a1
//    //4. merge i2c addr
//    GET_MC_SLOT1_ID
//    dsll    a1, a1, S1_I2C_ADDR_OFFSET_V1
//    dli     a0, 0xf
//    dsll    a0, a0, S1_I2C_ADDR_OFFSET_V1
//    not     a0, a0
//    and     s1, s1, a0
//    or      s1, s1, a1
    //config DIMM config
    dli     a1, 0x2
    dsll    a1, S1_DIMM_CONFIG_OFFSET_V1
    dli     a0, 0x3
    dsll    a0, S1_DIMM_CONFIG_OFFSET_V1
    not     a0, a0
    and     s1, s1, a0
    or      s1, s1, a1

    b       3f
    nop
4:  //move DIMM_MEMSIZE to MC0_MEMSIZE
//    GET_DIMM_MEMSIZE
//    dsll    a1, a1, S1_DIMM_MEMSIZE_OFFSET
//    dli     a0, MC_MEMSIZE_MASK
//    dsll    a0, a0, S1_DIMM_MEMSIZE_OFFSET
//    not     a0, a0
//    and     s1, s1, a0
//    or      s1, s1, a1
//    //4. merge i2c addr
//    GET_MC_SLOT0_ID
//    dsll    a1, a1, S1_I2C_ADDR_OFFSET_V1
//    dli     a0, 0xf
//    dsll    a0, a0, S1_I2C_ADDR_OFFSET_V1
//    not     a0, a0
//    and     s1, s1, a0
//    or      s1, s1, a1
    //config DIMM config
    dli     a1, 0x1
    dsll    a1, S1_DIMM_CONFIG_OFFSET_V1
    dli     a0, 0x3
    dsll    a0, S1_DIMM_CONFIG_OFFSET_V1
    not     a0, a0
    and     s1, s1, a0
    or      s1, s1, a1

    b       3f
    nop
1:  //no DIMM in slot 0 and 1   
    PRINTSTR("\r\nNO DIMM in probed MC slot.\r\n");
    b       3f
    nop
3:
    GET_MC_SLOT0_ID
    dsll    a1, a1, S1_I2C_ADDR_OFFSET_V1
    dli     a0, 0xf
    dsll    a0, a0, S1_I2C_ADDR_OFFSET_V1
    not     a0, a0
    and     s1, s1, a0
    or      s1, s1, a1
    GET_MC_SLOT1_ID
    dsll    a1, a1, (S1_I2C_ADDR_OFFSET_V1+4)
    dli     a0, 0xf
    dsll    a0, a0, (S1_I2C_ADDR_OFFSET_V1+4)
    not     a0, a0
    and     s1, s1, a0
    or      s1, s1, a1
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
END(PROBE_CHANNEL_DIMM)




/*************************
PROBE_DIMM:
function: probe the given slot(I2C device id is given in a0),
      if there is no DIMM in this slot, clear MC_CS_MAP to 0,
      else read the DIMM infor from the SPD and store the infor
      in s1
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

    GET_SPD_SDRAM_TYPE_V1
    beq     v0, SDRAM_DDR3_V1, DDR3
    nop
    beq     v0, SDRAM_DDR4_V1, DDR4
    nop

    PRINTSTR("\r\nNO DIMM in this slot.\r\n")
    b       ERROR_TYPE
    nop

DDR3://TODO
    dli     t5, 0x3
    dsll    t5, t5, S1_SDRAM_TYPE_OFFSET_V1
    or      s1, s1, t5
    b       1f
    nop

DDR4:
    dli     t5, 0x4
    dsll    t5, t5, S1_SDRAM_TYPE_OFFSET_V1
    or      s1, s1, t5

1:

//probe MODULE TYPE
    GET_SPD_MODULE_TYPE_V1
//    beq     v0, MODULE_UDIMM_V1, UDIMM
//    nop
//    beq     v0, MODULE_RDIMM_V1, RDIMM
//    nop
//    beq     v0, MODULE_SODIMM_V1, UDIMM    //SODIMM, deal as UDIMM
//    nop
    beq     v0, 1, RDIMM
    nop
    beq     v0, 5, RDIMM
    nop
    beq     v0, 8, RDIMM
    nop
    beq     v0, 2, UDIMM
    nop
    beq     v0, 6, UDIMM
    nop
    beq     v0, 4, LRDIMM
    nop
    beq     v0, 3, SODIMM
    nop
    beq     v0, 9, SODIMM
    nop
    beq     v0, 12, SODIMM
    nop
    beq     v0, 13, SODIMM
    nop
    PRINTSTR("\r\nERROR: DIMM type is not in support range(UDIMM or RDIMM).\r\n")
    b       ERROR_TYPE
    nop

RDIMM:
    dli     t5, 0x1//RDIMM
    dsll    t5, t5, S1_DIMM_TYPE_OFFSET_V1
    or      s1, s1, t5
    b       1f
    nop

UDIMM:
    dli     t5, 0x0//UDIMM
    dsll    t5, t5, S1_DIMM_TYPE_OFFSET_V1
    or      s1, s1, t5
    b       1f
    nop
SODIMM:
    dli     t5, 0x2//SODIMM
    dsll    t5, t5, S1_DIMM_TYPE_OFFSET_V1
    or      s1, s1, t5
    b       1f
    nop
LRDIMM:
    dli     t5, 0x3//LRDIMM
    dsll    t5, t5, S1_DIMM_TYPE_OFFSET_V1
    or      s1, s1, t5
1:

#if SPD_DBG
//probe BG NUM, save actual value
    PRINTSTR("\r\nbg=0x");
    nop
    dli     a0, 0xa3
    GET_SPD_BG_NUM_V1
    move    a0, v0
    bal     hexserial
    nop
    PRINTSTR("\r\n");
    nop

    PRINTSTR("\r\nba=0x");
    nop
    dli     a0, 0xa3
    GET_SPD_BA_NUM_V1
    move    a0, v0
    bal     hexserial
    nop
    PRINTSTR("\r\n");
    nop

    PRINTSTR("\r\ncs_map=0x");
    nop
    dli     a0, 0xa3
    GET_SPD_CS_MAP_V1
    move    a0, v0
    bal     hexserial
    nop
    PRINTSTR("\r\n");
    nop

    PRINTSTR("\r\nsdram_width=0x");
    nop
    dli     a0, 0xa3
    GET_SPD_SDRAM_WIDTH_V1
    move    a0, v0
    bal     hexserial
    nop
    PRINTSTR("\r\n");
    nop

    PRINTSTR("\r\ndimm_width=0x");
    nop
    dli     a0, 0xa3
    GET_SPD_DIMM_WIDTH_V1
    move    a0, v0
    bal     hexserial
    nop
    PRINTSTR("\r\n");
    nop

    PRINTSTR("\r\nrow_size=0x");
    nop
    dli     a0, 0xa3
    GET_SPD_ROW_SIZE_V1
    move    a0, v0
    bal     hexserial
    nop
    PRINTSTR("\r\n");
    nop

    PRINTSTR("\r\ncol_size=0x");
    nop
    dli     a0, 0xa3
    GET_SPD_COL_SIZE_V1
    move    a0, v0
    bal     hexserial
    nop
    PRINTSTR("\r\n");
    nop

    PRINTSTR("\r\nmirror=0x");
    nop
    dli     a0, 0xa3
    GET_SPD_ADDR_MIRROR_V1
    move    a0, v0
    bal     hexserial
    nop
    PRINTSTR("\r\n");
    nop

    PRINTSTR("\r\nmemsize=0x");
    nop
    dli     a0, 0xa3
    GET_SPD_DIMM_MEMSIZE_V1
    move    a0, v0
    bal     hexserial
    nop
    PRINTSTR("\r\n");
    nop

    PRINTSTR("\r\nmemsize=0x");
    nop
    dli     a0, 0xa3
    GET_SPD_DIMM_MEMSIZE_V1
    move    a0, v0
    bal     hexserial
    nop
    PRINTSTR("\r\n");
    nop
#endif

//    dli     a0, 0xa3
//    move    t5, v0
//    dsll    t5, t5, S1_BG_NUM_OFFSET_V1
//    or      s1, s1, t5

//probe BG NUM, save actual value
#ifndef DDR3_DIMM
    GET_SPD_BG_NUM_V1
#else
    li      v0, 0
#endif
    move    t5, v0
    dsll    t5, t5, S1_BG_NUM_OFFSET_V1
    or      s1, s1, t5

#ifndef DDR3_DIMM
    GET_SPD_CID_NUM_V1
#else
    li      v0, 0
#endif
    move    t5, v0
    dsll    t5, t5, S1_CID_NUM_OFFSET_V1
    or      s1, s1, t5

//probe BA NUM, save value as 0-4bank 1-8bank
    GET_SPD_BA_NUM_V1
    move    t5, v0
    dsll    t5, t5, S1_BA_NUM_OFFSET_V1
    or      s1, s1, t5

//probe physical rank
    dli     t6, 0x900000003ff00180
    GET_I2C_NODE_ID_a2
    or      t6, a2
    lw      a2, 0x0(t6)
    li      v1, 0x1
    sll     v1, v1, 4
    not     v1, v1
    and     a2, a2, v1
    sw      a2, 0x0(t6)
    sync

    GET_I2C_NODE_ID_a2
    dli     t6, 0x900000000ff00000
    or      t6, a2
    lb	    t5, 0x0(t6)
    beq	    t5, 0x11, 1f
    nop

    dli     t6, 0x900000003ff00180
    GET_I2C_NODE_ID_a2
    or      t6, a2
    lw      a2, 0x0(t6)
    li      v1, 0x1
    sll     v1, v1, 4
    or      a2, a2, v1 #set

    sw      a2, 0x0(t6)
    sync

    dli		t5, 0x1 
    b	    2f
    nop
1:
    GET_SPD_CS_MAP_V1
    move    t5, v0
2:
    dsll    t5, t5, S1_MC_CS_MAP_OFFSET_V1
    or      s1, s1, t5

//probe module width, save ecc in s1
#ifndef CHANGE_DIMM_WIDTH
    GET_SPD_DIMM_WIDTH_V1
    move    t5, v0
#else
    dsrl    t5, t1, 4
    andi    t5, 0xf
#endif
    dsll    t5, t5, S1_DIMM_WIDTH_OFFSET_V1
    or      s1, s1, t5
//probing module width must be done befor probing MEMSIZE
//probe dram_density, calculate memsize
    dli     t6, 0x900000003ff00180
    GET_I2C_NODE_ID_a2
    or      t6, a2
    lw      a2, 0x0(t6)
    li      v1, 0x1
    sll     v1, v1, 4
    not     v1, v1
    and     a2, a2, v1
    sw      a2, 0x0(t6)
    sync

    GET_I2C_NODE_ID_a2
    dli     t6, 0x900000000ff00000
    or      t6, a2
    lb	    t5, 0x0(t6)
    beq	    t5, 0x11, new_cs
    nop

    dli     t6, 0x900000003ff00180
    GET_I2C_NODE_ID_a2
    or      t6, a2
    lw      a2, 0x0(t6)
    li      v1, 0x1
    sll     v1, v1, 4
    or      a2, a2, v1 #set

    sw      a2, 0x0(t6)
    sync
    GET_SPD_DIMM_MEMSIZE_1CS_V1
    b	    old_cs
    nop
new_cs:
    GET_SPD_DIMM_MEMSIZE_V1
old_cs:
    move    t5, v0
    dsll    t5, t5, S1_DIMM_MEMSIZE_OFFSET_V1
    or      s1, s1, t5

//probe dram width, save in s1
    GET_SPD_SDRAM_WIDTH_V1
    move    t5, v0
    dsll    t5, t5, S1_SDRAM_WIDTH_OFFSET_V1
    or      s1, s1, t5

    GET_SPD_DIMM_ECC_V1
    move    t5, v0
    dsll    t5, t5, S1_DIMM_ECC_OFFSET_V1
    or      s1, s1, t5

//probe dram row size
    GET_SPD_ROW_SIZE_V1
    move    t5, v0
    dsll    t5, t5, S1_ROW_SIZE_OFFSET_V1
    or      s1, s1, t5

//probe dram col size
    GET_SPD_COL_SIZE_V1
    move    t5, v0
    dsll    t5, t5, S1_COL_SIZE_OFFSET_V1
    or      s1, s1, t5

//probe addr mirror
    GET_SPD_ADDR_MIRROR_V1
    move    t5, v0
    dsll    t5, t5, S1_ADDR_MIRROR_OFFSET_V1
    or      s1, s1, t5

    b       probe_dimm_end
    nop

ERROR_TYPE:
//no DIMM or unrecognized DIMM in this slot
    dli     t5, 0xff
    dsll    t5, t5, S1_MC_CS_MAP_OFFSET_V1
    not     t5, t5
    and     s1, s1, t5
probe_dimm_end:
    move    ra, t7
    jr      ra
    nop
END(PROBE_DIMM)
