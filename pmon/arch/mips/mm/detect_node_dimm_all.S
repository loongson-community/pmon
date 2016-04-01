/*************************
    Author: Chen xinke
    Funtion: detect all four(maximum) DIMM slots of MC1 and MC0 of one NODE and store the information in s1
    v1.0
    input:
    s1[31:16] MC1 slot1/0 MC0 slot1/0 SPD id
    output:
    s1[63:40] MC1 DIMM info
      [31: 8] MC0 DIMM info
*************************/
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
//#define DEBUG_PROBE_NODE_DIMM

LEAF(PROBE_NODE_DIMM)
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
#ifdef USE_SB_I2C
    PRINTSTR("\r\nOpen SMBUS controller\r\n");
    /* Open the SMBUS controller */
    dli     a1, 0x90000efdfe00a090 ; #b:d:f:r=0:14:0:90 set tmp config address
#ifdef  MULTI_I2C_BUS
    GET_I2C_NODE_ID_a2
    dsll    a2, a2, 44
    or      a1, a1, a2
#endif
    li      a0, SMBUS_IO_BASE_VALUE | 0x1
    sw      a0, 0x0(a1);

    /* enable the host controller */
    dli     a1, 0x90000efdfe00a0d0 ; #b:d:f:r=0:14:0:d2 bit0=1
#ifdef  MULTI_I2C_BUS
    GET_I2C_NODE_ID_a2
    dsll    a2, a2, 44
    or      a1, a1, a2
#endif
    lw      a0, 0x0(a1)
    li      a2, 0x10000
    or      a0, a0, a2
    sw      a0, 0x0(a1)
#endif
#if defined(USE_LS2H_I2C) || defined(USE_LS1A_I2C)
    bal     i2cinit
    nop
#endif
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
10:

//detect MC1 if not define MC0_ONLY
    GET_MC0_ONLY
    bnez    a1, 11f
    nop
    //do auto probe DIMM
    PRINTSTR("\r\nProbing DDR MC1 SLOT: ");
    PRINTSTR("\r\nProbe MC1 slot 0.");
    dli     a1, 0xff
    and     s1, s1, a1
    GET_MC1_SLOT0_ID
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

    PRINTSTR("\r\nProbe MC1 slot 1.");
    dli     a1, 0xff
    and     s1, s1, a1
    GET_MC1_SLOT1_ID
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

    //compare the two slot DIMM infor and merge the CS_MAP and MC1_MEMSIZE if necessary
    move    s1, t3
    GET_SDRAM_TYPE
    beqz    a1, 1f
    nop
    move    s1, t4
    GET_SDRAM_TYPE
    beqz    a1, 2f
    nop
    //both slot 0 and 1 has DIMM
    //step 1: compare the two DIMM type infor, if they differs, display errors
    xor     a1, t3, t4
    dli     a0, 0xfff08000
    and     a1, a1, a0
    bnez    a1, 9f
    nop
    //step 2: if the two DIMM types are equal, merge the CS_MAP and MC1_MEMSIZE
    //1. Calculate new CS_MAP
    move    s1, t4
    GET_MC_CS_MAP
    dsll    a0, a1, 2
    move    s1, t3
    GET_MC_CS_MAP
    or      t6, a0, a1
    dsll    t6, t6, MC_CS_MAP_OFFSET    //store new MC_MAP in t6
    //2. merge MC1_MEMSIZE
    move    s1, t3
    GET_DIMM_MEMSIZE
    move    a0, a1
    move    s1, t4
    GET_DIMM_MEMSIZE
    daddu   a1, a1, a0
    dli     a0, MC_MEMSIZE_MASK //make sure a1 not exceed its range
    and     a1, a1, a0
    dsll    a1, a1, MC0_MEMSIZE_OFFSET
    dli     a0, MC_MEMSIZE_MASK
    dsll    a0, a0, MC0_MEMSIZE_OFFSET
    not     a0, a0
    and     s1, s1, a0
    or      s1, s1, a1
    //3. merge new MC_CS_MAP(in t6) to s1, do not affect other bits
    dli     a0, 0xf
    dsll    a0, a0, MC_CS_MAP_OFFSET
    not     a0, a0
    and     s1, s1, a0
    or      s1, s1, t6
    b       3f
    nop
9:  //two slot have different type DIMM, give ERROR message and use slot 0 only
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
    dsll    a1, a1, (MC_CS_MAP_OFFSET + 2)
    dli     a0, 0xf
    dsll    a0, a0, MC_CS_MAP_OFFSET
    not     a0, a0
    and     s1, s1, a0
    or      s1, s1, a1
    b       4f
    nop
4:  //move DIMM_MEMSIZE to MC0_MEMSIZE
    GET_DIMM_MEMSIZE
    dsll    a1, a1, MC0_MEMSIZE_OFFSET
    dli     a0, MC_MEMSIZE_MASK
    dsll    a0, a0, MC0_MEMSIZE_OFFSET
    not     a0, a0
    and     s1, s1, a0
    or      s1, s1, a1
    b       3f
    nop
1:  //no DIMM in slot 0 and 1   
    PRINTSTR("\r\nNO DIMM in MC1 slot.\r\n");
    b       11f
    nop
3:  //move MC1 DIMM info to t0 high 32 bit
    dli     a0, 0xffffffff
    and     s1, s1, a0
    dsll    a1, s1, 32
    or      a1, s1, a1
    dli     a0, 0xffffff00000000ff
    and     s1, a1, a0
    move    t0, s1
#ifdef  DEBUG_PROBE_NODE_DIMM
    /* show value of s1 */
    PRINTSTR("\r\n T5 s1 = 0x");
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
//-------------------------------------
11:
//detect MC0 if not define MC1_ONLY
    GET_MC1_ONLY
    bnez    a1, 12f
    nop
    //do auto probe DIMM
    PRINTSTR("\r\nProbing DDR MC0 SLOT: ");
    PRINTSTR("\r\nProbe MC0 slot 0.");
    dli     a1, 0xff
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
    dli     a1, 0xff
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
    GET_SDRAM_TYPE
    beqz    a1, 1f
    nop
    move    s1, t4
    GET_SDRAM_TYPE
    beqz    a1, 2f
    nop
    //both slot 0 and 1 has DIMM
    //step 1: compare the two DIMM type infor, if they differs, display errors
    xor     a1, t3, t4
    dli     a0, 0xfff08000
    and     a1, a1, a0
    bnez    a1, 9f
    nop
    //step 2: if the two DIMM types are equal, merge the CS_MAP and MC0_MEMSIZE
    //1. Calculate new CS_MAP
    move    s1, t4
    GET_MC_CS_MAP
    dsll    a0, a1, 2
    move    s1, t3
    GET_MC_CS_MAP
    or      t6, a0, a1
    dsll    t6, t6, MC_CS_MAP_OFFSET    //store new MC_MAP in t6, don't move to s1 imediately because when merge MC_MEMSIZE, s1 will be damaged
    //2. merge MC0_MEMSIZE
    move    s1, t3
    GET_DIMM_MEMSIZE
    move    a0, a1
    move    s1, t4
    GET_DIMM_MEMSIZE
    daddu   a1, a1, a0
    dli     a0, MC_MEMSIZE_MASK //make sure a1 not exceed its range
    and     a1, a1, a0
    dsll    a1, a1, MC0_MEMSIZE_OFFSET
    dli     a0, MC_MEMSIZE_MASK
    dsll    a0, a0, MC0_MEMSIZE_OFFSET
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
    dsll    a0, a0, MC_CS_MAP_OFFSET
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
    dsll    a1, a1, (MC_CS_MAP_OFFSET + 2)
    dli     a0, 0xf
    dsll    a0, a0, MC_CS_MAP_OFFSET
    not     a0, a0
    and     s1, s1, a0
    or      s1, s1, a1
    b       4f
    nop
4:  //move DIMM_MEMSIZE to MC0_MEMSIZE
    GET_DIMM_MEMSIZE
    dsll    a1, a1, MC0_MEMSIZE_OFFSET
    dli     a0, MC_MEMSIZE_MASK
    dsll    a0, a0, MC0_MEMSIZE_OFFSET
    not     a0, a0
    and     s1, s1, a0
    or      s1, s1, a1
    b       3f
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
//merge MC1 and MC0 DIMM infor to s1
    dli     a1, 0xffffff0000000000
    and     t0, t0, a1
    dli     a1, 0xffffffff
    and     s1, s1, a1
    or      s1, s1, t0
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
      in s1(CS_MAP at s1[MC_CS_MAP_OFFSET+2, MC_CS_MAP_OFFSET], 
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

#if 0
//probe SDRAM type, if SDRAM type error, repeat t5 times.----seems no benefit
    dli     t5, 1   //max probing times(t5)
1:
    daddiu  t5, t5, -1
#endif

    dli     a1, 2
    GET_I2C_NODE_ID_a2
    bal     i2cread
    nop
    //only bit[7:0] used
    andi    v0, v0, 0xff
    /* v0 should be 0xb or 0x8,else error DIMM type */
    dli     a1, 0x08
    beq     v0, a1, DDR2
    nop
    dli     a1, 0x0B
    beq     v0, a1, DDR3
    nop
#if 0
    //this time probe error
//delay some time
    dli     a1, 0x1000
2:
    daddiu  a1, a1, -1
    bnez    a1, 2b
    nop

    bnez    t5, 1b
    nop
#endif
    PRINTSTR("\r\nNO DIMM in this slot.\r\n")
    b       ERROR_TYPE
    nop
DDR2:
    dli     t5, 0x2
    dsll    t5, t5, SDRAM_TYPE_OFFSET
    or      s1, s1, t5
//probe DIMM_TYPE
    dli     a1, 20
    GET_I2C_NODE_ID_a2
    bal     i2cread
    nop
    //only bit[5:0] used
    andi    v0, v0, 0x3f
    //here just recognize RDIMM ,UDIMM and SODIMM
    dli     t5, 0x01
    beq     v0, t5, 1f
    nop
    dli     t5, 0x02
    beq     v0, t5, 2f
    nop
    dli     t5, 0x04    //SODIMM, deal as UDIMM
    beq     v0, t5, 2f
    nop
    PRINTSTR("\r\nERROR: DIMM type is not in support range(UDIMM or RDIMM).\r\n")
    b       ERROR_TYPE
    nop
1:  //RDIMM
    dli     t5, 0x1
    dsll    t5, t5, DIMM_TYPE_OFFSET
    or      s1, s1, t5
    b       3f
    nop
2:  //UDIMM
    b       3f
    nop
3:  
//probe DIMM WIDTH
    dli     a1, 6
    GET_I2C_NODE_ID_a2
    bal     i2cread
    nop
    //only bit[7:0] used
    andi    v0, v0, 0xff
    //simply deal here
    dli     t5, 0x40
    bge     v0, t5, 1f
    nop
    //reduc size
    dli     t5, 1
    dsll    t5, t5, DIMM_WIDTH_OFFSET
    or      s1, s1, t5
1:  //normal size, do nothing
//probe DIMM ECC
    dli     a1, 11
    GET_I2C_NODE_ID_a2
    bal     i2cread
    nop
    //only bit[1] used
    andi    t5, v0, 0x2
    dsrl    t5, t5, 1
    dsll    t5, t5, DIMM_ECC_OFFSET
    or      s1, s1, t5
//probe SDRAM_WIDTH
    dli     a1, 13
    GET_I2C_NODE_ID_a2
    bal     i2cread
    nop
    //only bit[7:0] used
    andi    v0, v0, 0xff
    //only check bit 4(here assuming x8 or x16 only)
    dsrl    t5, v0, 4
    and     t5, t5, 0x1
    dsll    t5, t5, SDRAM_WIDTH_OFFSET
    or      s1, s1, t5
//probe SDRAM_ROW_SIZE
    dli     a1, 3
    GET_I2C_NODE_ID_a2
    bal     i2cread
    nop
    //only bit[7:0] used
    andi    v0, v0, 0xff
    //v0 should <= MAX_ROW_SIZE
    andi    v0, v0, 0x1f
    dli     t5, MAX_ROW_SIZE
    dsubu   t5, t5, v0
    dsll    t5, t5, ROW_SIZE_OFFSET
    or      s1, s1, t5
//probe SDRAM_COL_SIZE
    dli     a1, 4
    GET_I2C_NODE_ID_a2
    bal     i2cread
    nop
    //only bit[7:0] used
    andi    v0, v0, 0xff
    //v0 should < 14
    andi    v0, v0, 0x0f
    dli     t5, 12
    dsubu   t5, t5, v0
    dsll    t5, t5, COL_SIZE_OFFSET
    or      s1, s1, t5
//probe SDRAM BANK number
    dli     a1, 17 
    GET_I2C_NODE_ID_a2
    bal     i2cread
    nop
    //bit[7:0] used
    andi    v0, v0, 0xff
    //here just recognize 4 banks or 8 banks
    dli     t5, 0x08
    beq     v0, t5, 1f
    nop
    dli     t5, 0x04
    beq     v0, t5, 2f
    nop
    PRINTSTR("\r\nERROR: SDRAM Banks number is not in support range(4 or 8).\r\n")
    b       ERROR_TYPE
    nop
1:  //8 banks
    dli     t5, 0x1
    dsll    t5, t5, EIGHT_BANK_OFFSET
    or      s1, s1, t5
    b       3f
    nop
2:  //4 banks
    //nothing need to do

3:  
//probe DIMM Ranks
    dli     a1, 5 
    GET_I2C_NODE_ID_a2
    bal     i2cread
    nop
    //only bit[2:0] used
    andi    v0, v0, 0x7
    //here just recognize 1 ranks or 2 ranks
    dli     t5, 0x0
    beq     v0, t5, 1f
    nop
    dli     t5, 0x1
    beq     v0, t5, 2f
    nop
    PRINTSTR("\r\nERROR: DIMM Ranks number is not in support range(1 or 2).\r\n")
    b       ERROR_TYPE
    nop
1:  //1 rank
    dli     t5, 0x1
    b       3f
    nop
2:  //2 ranks
    dli     t5, 0x3
3:  
    dsll    t5, t5, MC_CS_MAP_OFFSET
    or      s1, s1, t5
//probe DIMM Density
    dli     a1, 31
    GET_I2C_NODE_ID_a2
    bal     i2cread
    nop
    //only bit[7:0] used
    andi    v0, v0, 0xff
//currently only support 512M <= 1DIMM size <= 7.5G, else assume there is no dimm by clear SDRAM_TYPE
    move    t5, v0
    //we need to reorder t5 to normal order to represent 1 rank density(/128M)
    //because of the JEDEC SPD spec awkward defination.
    dli     a1, 0xe0
    and     a1, t5, a1
    beqz    a1, 1f
    nop
    //1 rank<= 512M
    dsrl    t5, t5, 5
    b       2f
    nop
1:  //1 rank >= 1G
    dsll    t5, t5, 3
2:
    //here, the 1 rank size is store in t5 in normal order, measured by 128M.
    //double the MEMSIZE if there are 2 ranks
    GET_MC_CS_MAP
    dsrl    a1, a1, 1   //test the cs 1
    dsll    t5, t5, a1  //double the size if a1 = 1
    dsrl    t5, t5, 2   //change measure unit to 512M
    andi    t5, t5, DIMM_MEMSIZE_MASK
    //check size is 0
    bnez    t5, 1f
    nop
//DIMM density < 512M or > 63.5G, errors(clear SDRAM_TYPE, assume there is NO DIMM in this slot)
    dli     t5, 0x3
    dsll    t5, t5, SDRAM_TYPE_OFFSET
    not     t5, t5
    and     s1, s1, t5
    PRINTSTR("\r\nERROR: DIMM size is not in support range(512M~63.5G).\r\n")
    b       ERROR_TYPE
    nop
1:
    dsll    t5, t5, DIMM_MEMSIZE_OFFSET
    or      s1, s1, t5
//DDR2 probe finished
    b       probe_dimm_end
    nop

DDR3:  //DDR3 SDRAM
    dli     t5, 0x3
    dsll    t5, t5, SDRAM_TYPE_OFFSET
    or      s1, s1, t5
//!!!!!!need to be completely tested
//probe DIMM_TYPE
    dli     a1, 0x3
    GET_I2C_NODE_ID_a2
    bal     i2cread
    nop
    //only bit[3:0] used
    andi    v0, v0, 0xf
    //here just recognize RDIMM ,UDIMM and SODIMM
    dli     t5, 0x01
    beq     v0, t5, 1f
    nop
    dli     t5, 0x02
    beq     v0, t5, 2f
    nop
    dli     t5, 0x03    //SODIMM, deal as UDIMM
    beq     v0, t5, 2f
    nop
    PRINTSTR("\r\nERROR: DIMM type is not in support range(UDIMM or RDIMM).\r\n")
    b       ERROR_TYPE
    nop
1:  //RDIMM
    dli     t5, 0x1
    dsll    t5, t5, DIMM_TYPE_OFFSET
    or      s1, s1, t5
    b       3f
    nop
2:  //UDIMM

    b       3f
    nop
3:  
//probe DIMM WIDTH and DIMM ECC
    dli     a1, 8
    GET_I2C_NODE_ID_a2
    bal     i2cread
    nop
    //bit[2:0]: DIMM_WIDTH, bit[3]: with ECC or not
    andi    v1, v0, 0x7
    //simply deal here
    dli     t5, 0x2
    bgt     v1, t5, 1f
    nop
    //reduc size
    dli     t5, 1
    dsll    t5, t5, DIMM_WIDTH_OFFSET
    or      s1, s1, t5
1:  //normal size, do nothing
    //set DIMM_ECC
    andi    t5, v0, 0x8
    dsrl    t5, t5, 3
    dsll    t5, t5, DIMM_ECC_OFFSET
    or      s1, s1, t5
//probe SDRAM_WIDTH
    dli     a1, 7
    GET_I2C_NODE_ID_a2
    bal     i2cread
    nop
    //only bit[2:0] used
    andi    v0, v0, 0x7
    //only check bit 1(here assuming x8 or x16 only)
    dsrl    t5, v0, 1
    and     t5, t5, 0x1
    dsll    t5, t5, SDRAM_WIDTH_OFFSET
    or      s1, s1, t5
//probe SDRAM_ROW_SIZE & SDRAM_COL_SIZE
    dli     a1, 5
    GET_I2C_NODE_ID_a2
    bal     i2cread
    nop
    //only bit[5:0] used
    andi    v0, v0, 0x3f
    //set row size
    andi    v1, v0, 0x38
    dsrl    v1, v1, 0x3
    //dli     t5, 3
    dli     t5, MAX_ROW_SIZE
    dsubu   t5, t5, 12
    ble     v1, t5, 1f
    nop
    PRINTSTR("\r\nERROR: The SDRAM Row size is too big(> MAX_ROW_SIZE).\r\n")
    b       ERROR_TYPE
    nop
1:
    dsubu   t5, t5, v1
    dsll    t5, t5, ROW_SIZE_OFFSET
    or      s1, s1, t5
    //set SDRAM_COL_SIZE
    andi    v0, v0, 0x3 //the bit 2 is reserved currently.
    dli     t5, 3
    dsubu   t5, t5, v0
    dsll    t5, t5, COL_SIZE_OFFSET
    or      s1, s1, t5
//probe SDRAM BANK number
    dli     a1, 4
    GET_I2C_NODE_ID_a2
    bal     i2cread
    nop
    //bit[6:0] used
    andi    v0, v0, 0x7f
    //here just support 8 banks
    andi    v1, v0, 0x70
    beqz    v1, 1f
    nop
    PRINTSTR("\r\nERROR: SDRAM Banks number is not supported(only support 8 banks now).\r\n")
    b       ERROR_TYPE
    nop
1:  //8 banks
    dli     t5, 0x1
    dsll    t5, t5, EIGHT_BANK_OFFSET
    or      s1, s1, t5

//probe DIMM Ranks
    dli     a1, 7 
    GET_I2C_NODE_ID_a2
    bal     i2cread
    nop
    //only bit[5:3] used
    andi    v0, v0, 0x38
    dsrl    v0, v0, 3
    //here just recognize 1 ranks or 2 ranks
    dli     t5, 0x0
    beq     v0, t5, 1f
    nop
    dli     t5, 0x1
    beq     v0, t5, 2f
    nop
    PRINTSTR("\r\nERROR: DIMM Ranks number is not in support range(1 or 2).\r\n")
    b       ERROR_TYPE
    nop
1:  //1 rank
    dli     t5, 0x1
    b       3f
    nop
2:  //2 ranks
    dli     t5, 0x3
3:  
    dsll    t5, t5, MC_CS_MAP_OFFSET
    or      s1, s1, t5
//probe DIMM Density
//DIMM Density = SDRAM Density / 8 * DIMM Width / SDRAM Width * Ranks
    dli     a1, 4
    GET_I2C_NODE_ID_a2
    bal     i2cread
    nop
    //only bit[3:0] used
    andi    v0, v0, 0x7 //bit 3 is reserved by JEDEC now
    dli     t5, 0x1
    dsll    t5, t5, v0
    //here t5 represent SDRAM Density in 256Mb
    dsll    t5, t5, 3   //*8 (64 bit width)
    GET_DIMM_WIDTH
    dsrl    t5, t5, a1  //if reduc=1, t5=t5 / 2
    //here t5 = SDRAM Density / 8 * DIMM Width
    //probe SDRAM Width
    dli     a1, 7
    GET_I2C_NODE_ID_a2
    bal     i2cread
    nop
    //only bit[2:0] used
    andi    v0, v0, 0x3 //bit 2 is reserved now
    dli     v1, 0x2
    daddu   v0, v0, v1
    dsrl    t5, t5, v0  //t5 /= SDRAM Width
    //here t5 = SDRAM Density / 8 * DIMM Width / SDRAM Width
    //here, the 1 rank size is store in t5 in normal order, measured by 256M.
    //double the MEMSIZE if there are 2 ranks
    GET_MC_CS_MAP
    dsrl    a1, a1, 1   //test the cs 1
    dsll    t5, t5, a1  //double the size if a1=1
    dsrl    t5, t5, 1   //change measure unit to 512M
    andi    t5, t5, DIMM_MEMSIZE_MASK
    //check size is 0
    bnez    t5, 1f
    nop
//DIMM density < 512M or > 63.5G, errors(clear SDRAM_TYPE, assume there is NO DIMM in this slot)
    dli     t5, 0x3
    dsll    t5, t5, SDRAM_TYPE_OFFSET
    not     t5, t5
    and     s1, s1, t5
    PRINTSTR("\r\nERROR: DIMM size is not in support range(512M~63.5G).\r\n")
    b       ERROR_TYPE
    nop
1:
    dsll    t5, t5, DIMM_MEMSIZE_OFFSET
    or      s1, s1, t5

    GET_DIMM_TYPE
    bnez    a1, 1f
    nop
//DDR3 UDIMM -- probe DIMM Address Mirroring
    dli     a1, 63
    GET_I2C_NODE_ID_a2
    bal     i2cread
    nop
    //only bit[0] used
    andi    v0, v0, 0x1
    move    t5, v0
    dsll    t5, t5, ADDR_MIRROR_OFFSET 
    or      s1, s1, t5
1:
//DDR3 probe finished
    b       probe_dimm_end
    nop

ERROR_TYPE:
//no DIMM or unrecognized DIMM in this slot
    dli     t5, 0x3
    dsll    t5, t5, SDRAM_TYPE_OFFSET
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
