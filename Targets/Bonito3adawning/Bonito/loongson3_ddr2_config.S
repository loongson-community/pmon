/**********************************
    loongson3_ddr2_config.S
        used to set up all ddr controllers
        and set up the memory space on L2 Xbar
    original: whd
    rewrite by cxk on 09/27/2010
    1: reorder the program
    2: add AUTO_DDR_CONFIG function
    3: msize is set according to s1[MC1&0_MEMSIZE]
    note: config L2 Xbar still need to be finished,currently only support limited MEMSIZE.
************************************/
#######################################################
/* Only DDR_1G CAN USE INTERLEAVE by now           */
/* Interleave pattern when both controller enabled */
#define NO_INTERLEAVE
//#define INTERLEAVE_27
//#define INTERLEAVE_13
//#define INTERLEAVE_12
//#define INTERLEAVE_11

#define GET_NODE_ID_a0  dli a0, 0x00000003; and a0, s1, a0; dsll a0, 44;
#define GET_MC0_ONLY    dli a0, 0x00000004; and a0, s1, a0;
#define GET_MC1_ONLY    dli a0, 0x00000008; and a0, s1, a0;
#define GET_DEBUG_MEM   dli a0, 0x10; and a0, s1, a0;
#define XBAR_CONFIG_NODE_a0(OFFSET, BASE, MASK, MMAP) \
						daddi   v0, t0, OFFSET;       \
                        dli     t1, BASE;             \
                        or      t1, t1, a0;           \
                        sd      t1, 0x00(v0);         \
                        dli     t1, MASK;             \
                        sd      t1, 0x40(v0);         \
                        dli     t1, MMAP;             \
                        sd      t1, 0x80(v0);
/*************************
PROBE_DIMM:
function: probe the given slot, if there is no DIMM in this slot, clear SDRAM_TYPE to 0,
          else read the DIMM infor from the SPD and store the infor in s1(CS_MAP at [17:16], 
          MEMSIZE at [11:8]).
*************************/
#ifndef LOONGSON3_DDR2_CONFIG
#define LOONGSON3_DDR2_CONFIG
//------------------------
//define for ddr configure register param location
#define EIGHT_BANK_MODE_OFFSET  32
#define COLUMN_SIZE_OFFSET      24
#define ADDR_PINS_OFFSET        8
#define CS_MAP_OFFSET           16
//------------------------
#define SDRAM_TYPE_OFFSET   30
#define DIMM_TYPE_OFFSET    28
#define DIMM_ECC_OFFSET     27
#define ROW_SIZE_OFFSET     24
#define EIGHT_BANK_OFFSET   23
#define COL_SIZE_OFFSET     20
#define MC_CS_MAP_OFFSET    16
#define MC1_MEMSIZE_OFFSET  12
#define MC0_MEMSIZE_OFFSET  8
#define DIMM_MEMSIZE_OFFSET 32
#define GET_SDRAM_TYPE      \
dli     a1, 0x3;\
dsll    a1, a1, SDRAM_TYPE_OFFSET;\
and     a1, s1, a1;\
dsrl    a1, a1, SDRAM_TYPE_OFFSET;
#define GET_DIMM_TYPE      \
dli     a1, 0x1;\
dsll    a1, a1, DIMM_TYPE_OFFSET;\
and     a1, s1, a1;\
dsrl    a1, DIMM_TYPE_OFFSET;
#define GET_DIMM_ECC        \
dli     a1, 0x1;\
dsll    a1, a1, DIMM_ECC_OFFSET;\
and     a1, s1, a1;\
dsrl    a1, a1, DIMM_ECC_OFFSET;
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
#define GET_COL_SIZE      \
dli     a1, 0x7;\
dsll    a1, a1, COL_SIZE_OFFSET;\
and     a1, s1, a1;\
dsrl    a1, a1, COL_SIZE_OFFSET;
#define GET_MC_CS_MAP      \
dli     a1, 0xf;\
dsll    a1, a1, MC_CS_MAP_OFFSET;\
and     a1, s1, a1;\
dsrl    a1, a1, MC_CS_MAP_OFFSET;
#define GET_DIMM_MEMSIZE      \
dli     a1, 0xf;\
dsll    a1, a1, DIMM_MEMSIZE_OFFSET;\
and     a1, s1, a1;\
dsrl    a1, a1, DIMM_MEMSIZE_OFFSET;
#define GET_MC1_MEMSIZE      \
dli     a1, 0xf;\
dsll    a1, a1, MC1_MEMSIZE_OFFSET;\
and     a1, s1, a1;\
dsrl    a1, a1, MC1_MEMSIZE_OFFSET;
#define GET_MC0_MEMSIZE      \
dli     a1, 0xf;\
dsll    a1, a1, MC0_MEMSIZE_OFFSET;\
and     a1, s1, a1;\
dsrl    a1, a1, MC0_MEMSIZE_OFFSET;
#endif

#######################################################
/************************************
1. get DIMM slot information, store to s1
***********************************/
/***********************************************************
 s1: new map
------------------------------------------------------------
not affected by PROBE_DIMM
|[15:12]| MC1_MEMSIZE        | 4'b0000 | 0M                |
|       |                    | 4'b0001 | 512M              |
|       |                    | 4'b0010 | 1G                |
|       |                    | 4'b0011 | 1.5G              |
|       |                    | 4'hX    | X*512M            | 
|[11: 8]| MC0_MEMSIZE        | 4'b0000 | 0M                |
|       |                    | 4'b0001 | 512M              |
|       |                    | 4'b0010 | 1G                |
|       |                    | 4'b0011 | 1.5G              |
|       |                    | 4'hX    | X*512M            | 
------------------------------------------------------------
not affected by AUTO_DDR_CONFIG
|[ 7: 5]|                    | 3'b0    | RESERVED          |
|[ 4: 4]| DMEM_SELECT        | 1'b0    | Skip Debug_Mem    |
|       |                    | 1'b1    | Do Debug_Mem      |
|[ 3: 2]| CONTROLLER_SELECT  | 2'b00   | USE BOTH          |
|       |                    | 2'b01   | MC0_ONLY          |
|       |                    | 2'b10   | MC1_ONLY          |
|[ 1: 0]| NODE ID            |         |                   |
------------------------------------------------------------
DIMM infor:
|[31:30]| SDRAM_TYPE         | 2'b00   | NO_DIMM           |
|       |                    | 2'b10   | DDR2              |
|       |                    | 2'b11   | DDR3              |
|[29:29]|                    | 1'b0    | RESERVED          |
|[28:28]| DIMM_TYPE          | 1'b1    | Registered DIMM   |	
|       |                    | 1'b0    | Unbuffered DIMM   |
|[27:27]| DIMM_ECC           | 1'b1    | WITH DATA ECC     |	
|       |                    | 1'b0    | NO	  DATA ECC     |
|[26:24]| SDRAM_ROW_SIZE     | MC_ROW  | 15 - ROW_SIZE     |
|[23:23]| SDRAM_EIGHT_BANK   | 1'b0    | FOUR  BANKS       |
|       |                    | 1'b1    | EIGHT BANKS       |
|[22:20]| SDRAM_COL_SIZE     | MC_COL  | 14 - COL_SIZE     |
------------------------------------------------------------
|[19:16]| MC_CS_MAP          |         |                   |
temparary used in PROBE_DIMM
|[35:32]| DIMM_MEMSIZE       | 4'b0000 | 0M                |
|       |                    | 4'b0001 | 512M              |
|       |                    | 4'b0010 | 1G                |
|       |                    | 4'b0011 | 1.5G              |
|       |                    | 4'hX    | X*512M            | 
***********************************************************/
/******************************************
s1: [ 4: 4]: DEBUG_MEM
    [ 3: 3]: MC1_ONLY
    [ 2: 2]: MC0_ONLY
    [ 1: 0]: NODE ID
    10: MC1_ONLY
    01: MC0_ONLY
    00: USE MC0&MC1
******************************************/
//set MC0&MC1 MEMSIZE in s1, s1 should be clear to 0 except s1[1:0].
        dli     t5, 0x4400
        or      s1, s1, t5

#define USE_SB_I2C
//#define DEBUG_DDR
//#define MY_DEBUG_DDR
//set use which MC: 01-MC0; 10-MC1; 00-MC0&MC1
#ifdef  MY_DEBUG_DDR
	    PRINTSTR("Please input DDR SELLECT :\r\n[  4]:Test_Mem\r\n[  3]:MC1_ONLY\r\n[  2]:MC0_ONLY\r\n");
	    li	    t6, 0x00
	    bal	    inputaddress	#input value stored in v0
	    nop
        dli     t6, 0xfc
        and     v0, v0, t6
        or      s1, s1, v0
#else
    //use both controllers
        dli     t6, 0x0     //can only be: 0,4,8
        or      s1, s1, t6
#endif

#ifdef AUTO_DDR_CONFIG
#ifdef USE_SB_I2C
	PRINTSTR("\r\nOpen SMBUS controller\r\n");
    /* Open the SMBUS controller */
	dli	    a1,0x90000efdfe00a090 ; #b:d:f:r=0:14:0:90 set tmp config address
	li	    a0,SMBUS_IO_BASE_VALUE | 0x1
	sw	    a0, 0x0(a1);

	/* enable the host controller */
	dli	    a1,0x90000efdfe00a0d0 ; #b:d:f:r=0:14:0:d2 bit0=1
	lw	    a0,0x0(a1);
	li	    t1, 0x10000;
	or 	    a0, t1
	sw	    a0, 0x0(a1);
#endif
#if 0  //for debug
//display DIMM SPD values
	PRINTSTR("\r\nSPD register dump\r\n");
	li	a0, 0xa1;
	li	a1,0x0;
	bal	i2cread;
	nop;
	move  t5, v0;
	move  a0, t5
	bal	hexserial
	nop
	PRINTSTR("\r\n");

    li  t5,0x80
	li	t6,0x0; //used as counter
	
1:
	move a0,t6;
	bal	hexserial
	nop
	PRINTSTR(": ");
	li	a0, 0xa1;
	move a1,t6;
	bal	i2cread;
	nop;
	move a0, v0
	bal	hexserial
	nop
	PRINTSTR("\r\n");
	nop
	
	addiu  t6,0x1;
	bne	   t6,t5, 1b;
	nop

	PRINTSTR("\r\n ^^^^^^^^^^^^^^^^^^^^^^^^^^^^:\r\n");
#endif 
#endif
//***************************************

/************************
2. config Memory Controller.
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
        lw  a1, 0x0(t2)
        li  a0, 0xfffffeff
        and a1, a1,a0
        sw  a1, 0x0(t2)
#endif    

//config MC0 if not define MC1_ONLY
//-------------------------------------
config_MC0:
        GET_MC1_ONLY
        bnez    a0, config_MC1
        nop
#ifdef AUTO_DDR_CONFIG
    //do auto probe DIMM
        PRINTSTR("\r\nProbing DDR MC0 SLOT: ");
        PRINTSTR("\r\nProbe MC0 slot 0: ");
        dli     t5, 0xf0ff
        and     s1, s1, t5
        li      a0,0xa1;
        bal     PROBE_DIMM;
        nop;
#ifdef  DEBUG_DDR
		/* show value of s1 */
        PRINTSTR(" s1 = ");
        dsrl    a0, s1, 32
        bal     hexserial
        nop
        PRINTSTR("__")
        move	a0, s1
        bal     hexserial
        nop;
        PRINTSTR("\r\n")
#endif
        //store slot 0 DIMM infor in t3
        move    t3, s1

        PRINTSTR("\r\nProbe MC0 slot 1: ");
        dli     t5, 0xf0ff
        and     s1, s1, t5
        li      a0,0xa3;
        bal     PROBE_DIMM;
        nop;
#ifdef  DEBUG_DDR
		/* show value of s1 */
        PRINTSTR(" s1 = ");
        dsrl    a0, s1, 32
        bal     hexserial
        nop
        PRINTSTR("__")
        move	a0, s1
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
        dli     a0, 0xd3f00000  //don't care ECC type now
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
        dsll    t6, t6, MC_CS_MAP_OFFSET    //store new MC_MAP in t6
#ifdef  DEBUG_DDR
		/* show value of s1 */
        PRINTSTR("\r\n T2 s1 = ");
        dsrl    a0, s1, 32
        bal     hexserial
        nop
        PRINTSTR("__")
        move	a0, s1
        bal     hexserial
        nop;
        PRINTSTR("\r\n")
#endif
        //2. merge MC0_MEMSIZE
        move    s1, t3
        GET_DIMM_MEMSIZE
        move    a0, a1
        move    s1, t4
        GET_DIMM_MEMSIZE
        daddu   a1, a1, a0
        dli     t5, 0xf //make sure a1 not exceed its range
        and     a1, a1, t5
        dsll    a1, a1, MC0_MEMSIZE_OFFSET
        dli     a0, 0xf
        dsll    a0, a0, MC0_MEMSIZE_OFFSET
        not     a0, a0
        and     s1, s1, a0
        or      s1, s1, a1
#ifdef  DEBUG_DDR
		/* show value of s1 */
        PRINTSTR("\r\n T3 s1 = ");
        dsrl    a0, s1, 32
        bal     hexserial
        nop
        PRINTSTR("__")
        move	a0, s1
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
2:  //only slot 0 has DIMM
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
        dli     a0, 0xf
        dsll    a0, a0, MC0_MEMSIZE_OFFSET
        not     a0, a0
        and     s1, s1, a0
        or      s1, s1, a1
        b       3f
        nop
1:  //no DIMM in slot 0 and 1   
        PRINTSTR("\r\nNO DIMM in MC0 slot. MC0 will not be configured!!\r\n");
        b       config_MC1
        nop
3:
#ifdef  DEBUG_DDR
		/* show value of s1 */
        PRINTSTR("\r\n T5 s1 = ");
        dsrl    a0, s1, 32
        bal     hexserial
        nop
        PRINTSTR("__")
        move	a0, s1
        bal     hexserial
        nop;
        PRINTSTR("\r\n")
#endif
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
#ifdef  DEBUG_DDR   //print registers
    PRINTSTR("The MC0 configuration is:\r\n")
    li      t1, 151
    li      t2, 0xaff00000
1:
    lw      a0, 0x4(t2)
    bal	    hexserial
    nop
    PRINTSTR("  ")

    lw      a0, 0x0(t2)
    bal	    hexserial
    nop
    PRINTSTR("\r\n")

    addiu   t2, t2, 16
    bnez    t1, 1b
    addiu   t1, t1, -1
#endif
//-------------------------------------
//config MC1 if not define MC0_ONLY
config_MC1:
        GET_MC0_ONLY
        bnez    a0, MC_config_end
        nop
#ifdef AUTO_DDR_CONFIG
    //do auto probe DIMM
        PRINTSTR("\r\nProbing DDR MC1 SLOT: ");
        PRINTSTR("\r\nProbe MC1 slot 0: ");
        dli     t5, 0x0fff
        and     s1, s1, t5
        li      a0,0xa5;
        bal     PROBE_DIMM;
        nop;
#ifdef  DEBUG_DDR
		/* show value of s1 */
        PRINTSTR(" s1 = ");
        dsrl    a0, s1, 32
        bal     hexserial
        nop
        PRINTSTR("__")
        move	a0, s1
        bal     hexserial
        nop;
        PRINTSTR("\r\n")
#endif
        //store slot 0 DIMM infor in t3
        move    t3, s1

        PRINTSTR("\r\nProbe MC1 slot 1: ");
        dli     t5, 0x0fff
        and     s1, s1, t5
        li      a0,0xa7;
        bal     PROBE_DIMM;
        nop;
#ifdef  DEBUG_DDR
		/* show value of s1 */
        PRINTSTR(" s1 = ");
        dsrl    a0, s1, 32
        bal     hexserial
        nop
        PRINTSTR("__")
        move	a0, s1
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
        dli     a0, 0xd3f00000  //don't care ECC type now
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
        dli     t5, 0xf //make sure a1 not exceed its range
        and     a1, a1, t5
        dsll    a1, a1, MC1_MEMSIZE_OFFSET
        dli     a0, 0xf
        dsll    a0, a0, MC1_MEMSIZE_OFFSET
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
9:      //two slot have different type DIMM, give ERROR message and use slot 0 only
        PRINTSTR("MC0 has two different DIMM, please use same DIMM!!\r\n")
        PRINTSTR("Currently system will use only slot 0!!\r\n")
        b       2f
        nop
2:  //only slot 0 has DIMM
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
4:  //move DIMM_MEMSIZE to MC1_MEMSIZE
        GET_DIMM_MEMSIZE
        dsll    a1, a1, MC1_MEMSIZE_OFFSET
        dli     a0, 0xf
        dsll    a0, a0, MC1_MEMSIZE_OFFSET
        not     a0, a0
        and     s1, s1, a0
        or      s1, s1, a1
        b       3f
        nop
1:  //no DIMM in slot 0 and 1   
        PRINTSTR("\r\nNO DIMM in MC1 slot. MC1 will not be configured!!\r\n");
        b       MC_config_end
        nop
3:
#ifdef  DEBUG_DDR
		/* show value of s1 */
        PRINTSTR("\r\n T5 s1 = ");
        dsrl    a0, s1, 32
        bal     hexserial
        nop
        PRINTSTR("__")
        move	a0, s1
        bal     hexserial
        nop;
        PRINTSTR("\r\n")
#endif
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
#ifdef  DEBUG_DDR   //print registers
    PRINTSTR("The MC1 configuration is:\r\n")
    li      t1, 151
    li      t2, 0xaff00000
1:
    lw      a0, 0x4(t2)
    bal	    hexserial
    nop
    PRINTSTR("  ")

    lw      a0, 0x0(t2)
    bal	    hexserial
    nop
    PRINTSTR("\r\n")

    addiu   t2, t2, 16
    bnez    t1, 1b
    addiu   t1, t1, -1
#endif
//-------------------------------------
MC_config_end:

###disable the reg space###
#if 1
	    TTYDBG("Disable register space of MEMORY\r\n")
        lw  a1,0x0(t2)
        or  a1,a1,0x100
        sw  a1,0x0(t2)
#endif

#if 0 // AdonWang disable ddr3 readbuff
/*      May Affect the Performance     */
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
/*****************************
 3. set msize for system
******************************/
//only NODE 0 uses msize
        dli     msize, 0x0
        GET_NODE_ID_a0;
        bnez    a0, 3f
        nop

        GET_MC0_ONLY
        beqz    a0, 1f
        nop
//use MC0 only
        GET_MC0_MEMSIZE
        move    t5, a1
        dsll    msize, t5, 29   //t5*512M
        b       2f
        nop
1:
        GET_MC1_ONLY
        beqz    a0, 1f
        nop
//use MC1 only
        GET_MC1_MEMSIZE
        move    t5, a1
        dsll    msize, t5, 29   //t5*512M
        b       2f
        nop
1:
//use MC0&MC1
        GET_MC0_MEMSIZE
        move    t5, a1
        GET_MC1_MEMSIZE
        daddu   t5, t5, a1
        dsll    msize, t5, 29   //t5*512M
2:
        //check msize > 0
        bnez    msize, 2f
        nop
        //msize = 0
        PRINTSTR("\r\nERROR: No DIMM is used in this system, cpu will be stalled!\r\n")
        PRINTSTR("-------------------------------------------")
1:      
        b       1b
        nop
2:      
        //check msize <= 4G
        dli     t5, 0x100000000
        bleu    msize, t5, 2f
        nop
        //msize > 4G
        PRINTSTR("\r\nERROR: Memory size > 4G, cpu will be stalled!\r\n")
        PRINTSTR("-------------------------------------------")
1:      
        b       1b
        nop
2:
        dli     t5, 0x1000000   //16M
        dsubu   msize, msize, t5//msize-=16M
3:
        PRINTSTR("\r\nmsize = 0x")
        move    a0, msize
        bal     hexserial
        nop
        PRINTSTR("\r\n")
/**************************
* when use AUTO_DDR_CONFIG, one MC may have no DIMM while the other has, in this case,
* the program should set MC0_ONLY or MC1_ONLY in s1 automatically, because the code of 
* configuring L2-Xbar will use this message.
* when program runs to here, the msize must > 0, so at least one MC is used
**************************/
        GET_MC0_MEMSIZE
        bnez    a1, 1f
        nop
        //MC0_MEMSIZE=0, set use MC1_ONLY
        dli     a1, 0x8
        or      s1, a1
1:        
        GET_MC1_MEMSIZE
        bnez    a1, 1f
        nop
        //MC1_MEMSIZE=0, set use MC0_ONLY
        dli     a1, 0x4
        or      s1, a1
1:        
/*******************************
 4. config L2 X-bar
    note: currently,when use only 1 MC,support memory size: 512M, 1G, 2G, 3G, 4G;
          when use MC0&MC1 both, only support 1G or 2G Memory size of each Controller.
*******************************/
/* MC0_ONLY */
        GET_MC0_ONLY
        beqz    a0, 1f
        nop
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
        b       2f
        nop
1:
/* MC1_ONLY */
        GET_MC1_ONLY
        beqz    a0, 1f
        nop
        GET_NODE_ID_a0;
        XBAR_CONFIG_NODE_a0(0x10, \
                            0x0000000000000000, \
                            0xFFFFFFFFF0000000, \
                            0x00000000000000F1)
	    PRINTSTR("MC1 space open : 0x00000000 - 0x0FFFFFFF\r\n")
        GET_NODE_ID_a0;
        XBAR_CONFIG_NODE_a0(0x100, \
                            0x0000000080000000, \
                            0xFFFFFFFFF0000000, \
                            0x00000000000000F1)
	    PRINTSTR("PCI space open to MC1: 0x80000000 - 0x8FFFFFFF\r\n")
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
#else   //Interleave_10
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

        GET_MC0_ONLY
        beqz    a0, 1f
        nop
//use MC0 only, currently only support 512M, 1G, 2G, 3G, 4G
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
        //temp code, MEM size >= 4G, use 4G only
        bgeu    a1, t5, 80f
        nop
        b       4f  //error condition
        nop
10:     //ddr_512MB_MC0:
        //is this window right?
        GET_NODE_ID_a0;
        XBAR_CONFIG_NODE_a0(0x20, \
                            0x0000000020000000, \
                            0xFFFFFFFFE0000000, \
                            0x00000000000000F0)
	    PRINTSTR("MC0 pace open : 0x20000000 - 0x3FFFFFFF\r\n")
        GET_NODE_ID_a0;
        XBAR_CONFIG_NODE_a0(0x120, \
                            0x0000000080000000, \
                            0xFFFFFFFFE0000000, \
                            0x00000000000000F0)
	    PRINTSTR("PCI space open to MC0: 0x80000000 - 0x9FFFFFFF\r\n")
        b       2f
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
        b       2f
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
        XBAR_CONFIG_NODE_a0(0x38, \
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
        b       2f
        nop
1:

        GET_MC1_ONLY
        beqz    a0, 1f
        nop
//use MC1 only, currently only support 512M, 1G, 2G, 3G, 4G
        GET_MC1_MEMSIZE
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
        b       4f
        nop
10:     //ddr_512MB_MC1:
        GET_NODE_ID_a0;
        XBAR_CONFIG_NODE_a0(0x20, \
                            0x0000000020000000, \
                            0xFFFFFFFFE0000000, \
                            0x00000000000000F1)
	    PRINTSTR("MC1 pace open : 0x20000000 - 0x3FFFFFFF\r\n")
        GET_NODE_ID_a0;
        XBAR_CONFIG_NODE_a0(0x120, \
                            0x0000000080000000, \
                            0xFFFFFFFFE0000000, \
                            0x00000000000000F1)
	    PRINTSTR("PCI space open to MC1: 0x80000000 - 0x9FFFFFFF\r\n")
        b       2f
        nop
20:     //ddr_1GB_MC1:
        GET_NODE_ID_a0;
        XBAR_CONFIG_NODE_a0(0x20, \
                            0x0000000040000000, \
                            0xFFFFFFFFC0000000, \
                            0x00000000000000F1)
	    PRINTSTR("MC1 space open : 0x40000000 - 0x7FFFFFFF\r\n")
        GET_NODE_ID_a0;
        XBAR_CONFIG_NODE_a0(0x120, \
                            0x0000000080000000, \
                            0xFFFFFFFFC0000000, \
                            0x00000000000000F1)
	    PRINTSTR("PCI space open to MC1: 0x80000000 - 0xBFFFFFFF\r\n")
        b       2f
        nop
40:     //ddr_2GB_MC1:
        GET_NODE_ID_a0;
        XBAR_CONFIG_NODE_a0(0x20, \
                            0x0000000080000000, \
                            0xFFFFFFFF80000000, \
                            0x00000000000000F1)
	    PRINTSTR("MC1 space open : 0x80000000 - 0xFFFFFFFF\r\n")
        b       3f
        nop
60:     //ddr_3GB_MC0:
        GET_NODE_ID_a0;
        XBAR_CONFIG_NODE_a0(0x20, \
                            0x00000000C0000000, \
                            0xFFFFFFFFC0000000, \
                            0x00000000000000F1)
        GET_NODE_ID_a0;
        XBAR_CONFIG_NODE_a0(0x38, \
                            0x0000000100000000, \
                            0xFFFFFFFF80000000, \
                            0x00000000000000F1)
	    PRINTSTR("MC1 space open : 0x0C0000000 - 0x17FFFFFFF\r\n")
        b       3f
        nop
80:     //ddr_4GB_MC1:
        GET_NODE_ID_a0;
        XBAR_CONFIG_NODE_a0(0x20, \
                            0x0000000100000000, \
                            0xFFFFFFFF00000000, \
                            0x00000000000000F1)
	    PRINTSTR("MC1 space open : 0x100000000 - 0x1FFFFFFFF\r\n")
        b       3f
        nop
3:
        //memsize >= 2G, PCIDMA still use 2G.
        GET_NODE_ID_a0;
        XBAR_CONFIG_NODE_a0(0x120, \
                            0x0000000080000000, \
                            0xFFFFFFFF80000000, \
                            0x00000000000000F1)
	    PRINTSTR("PCI space open to MC1: 0x80000000 - 0xFFFFFFFF\r\n")
        b       2f
        nop
1:  

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
//only support MC0 & MC1 1G or 2G.
        GET_MC0_MEMSIZE
        dli     t5, 0x2
        beq     a1, t5, 200f
        nop
        dli     t5, 0x4
        //temp code. >= 2G, use 2G only
        bgeu    a1, t5, 400f
        nop
        b       4f
        nop
200:    //MC0_SIZE_1G
        GET_MC1_MEMSIZE
        dli     t5, 0x2
        beq     a1, t5, 201f
        nop
        dli     t5, 0x4
        //temp code. >= 2G, use 2G only
        bgeu    a1, t5, 202f
        nop
        b       4f
        nop
201:    //MC1_SIZE_1G
//2G space: MC0: 8~bf; MC1: c~ff
        GET_NODE_ID_a0;
        XBAR_CONFIG_NODE_a0(0x20, \
                            0x0000000080000000, \
                            0xFFFFFFFFC0000000, \
                            0x00000000000000F0)
	    PRINTSTR("MC0 space open : 0x80000000 - 0xBFFFFFFF\r\n")
        GET_NODE_ID_a0;
        XBAR_CONFIG_NODE_a0(0x38, \
                            0x00000000C0000000, \
                            0xFFFFFFFFC0000000, \
                            0x00000000000000F1)
	    PRINTSTR("MC1 space open : 0xC0000000 - 0xFFFFFFFF\r\n")
	    PRINTSTR("DDR space open : 0x80000000 - 0xFFFFFFFF\r\n")
        b       2f
        nop
202:    //MC1_SIZE_2G
//3G space: MC0: c~ff; MC1: 10~17f
        GET_NODE_ID_a0;
        XBAR_CONFIG_NODE_a0(0x20, \
                            0x00000000C0000000, \
                            0xFFFFFFFFC0000000, \
                            0x00000000000000F0)
	    PRINTSTR("MC0 space open : 0xC0000000 - 0xFFFFFFFF\r\n")
        GET_NODE_ID_a0;
        XBAR_CONFIG_NODE_a0(0x38, \
                            0x0000000100000000, \
                            0xFFFFFFFF80000000, \
                            0x00000000000000F1)
	    PRINTSTR("MC1 space open : 0x100000000 - 0x17FFFFFFF\r\n")
	    PRINTSTR("DDR space open : 0x0C0000000 - 0x17FFFFFFF\r\n")
        b       2f
        nop
400:    //MC0_SIZE_2G
        GET_MC1_MEMSIZE
        dli     t5, 0x2
        beq     a1, t5, 401f
        nop
        dli     t5, 0x4
        //temp code. >= 2G, use 2G only
        bgeu    a1, t5, 402f
        nop
        b       4f
        nop
401:    //MC1_SIZE_1G
//3G space: MC1: c~ff; MC0: 10~17f   !!!!note: here use MC1 first for simple.
        GET_NODE_ID_a0;
        XBAR_CONFIG_NODE_a0(0x20, \
                            0x00000000C0000000, \
                            0xFFFFFFFFC0000000, \
                            0x00000000000000F1)
	    PRINTSTR("MC1 space open : 0xC0000000 - 0xFFFFFFFF\r\n")
        GET_NODE_ID_a0;
        XBAR_CONFIG_NODE_a0(0x38, \
                            0x0000000100000000, \
                            0xFFFFFFFF80000000, \
                            0x00000000000000F0)
	    PRINTSTR("MC0 space open : 0x100000000 - 0x17FFFFFFF\r\n")
	    PRINTSTR("DDR space open : 0x0C0000000 - 0x17FFFFFFF\r\n")
        b       2f
        nop
402:    //MC1_SIZE_2G
//4G space: MC0: 10~17f; MC1: 18~1ff
        GET_NODE_ID_a0;
        XBAR_CONFIG_NODE_a0(0x20, \
                            0x0000000100000000, \
                            0xFFFFFFFF80000000, \
                            0x00000000000000F0)
	    PRINTSTR("MC0 space open : 0x100000000 - 0x17FFFFFFF\r\n")
        GET_NODE_ID_a0;
        XBAR_CONFIG_NODE_a0(0x38, \
                            0x0000000180000000, \
                            0xFFFFFFFF80000000, \
                            0x00000000000000F1)
	    PRINTSTR("MC1 space open : 0x180000000 - 0x1FFFFFFFF\r\n")
	    PRINTSTR("DDR space open : 0x100000000 - 0x1FFFFFFFF\r\n")
        b       2f
        nop
#else   //INTERLEAVE MODE
/* ONLY 1GB or 2GB has INTERLEAVE mode now */
        GET_MC0_MEMSIZE
        move    t5, a1
        GET_MC1_MEMSIZE
        bne     a1, t5, 3f
        nop
        dli     t5, 0x2
        beq     a1, t5, 200f
        nop
        dli     t5, 0x4
        //temp code. >= 2G, use 2G only
        bgeu    a1, t5, 400f
        nop
        b       4f
        nop
200:
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
	    PRINTSTR("DDR Interleave space open : 0x80000000 - 0xFFFFFFFF\r\n")
        b       2f
        nop
400:    //2G in each MC
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
	    PRINTSTR("DDR Interleave space open : 0x100000000 - 0x1FFFFFFFF\r\n")
        b       2f
        nop

3:      //error: MC0_MEMSIZE != MC1_MEMSIZE
        PRINTSTR("When use MC0&MC1 interleave mode,the two MC_MEMSIZE must be equal, cpu will be stalled!\r\n")
        PRINTSTR("-------------------------------------------")
1:      
        b       1b
        nop
4:      //error not in support range
        PRINTSTR("The MEMSIZE is not supported, cpu will be stalled!\r\n")
        PRINTSTR("-------------------------------------------")
1:      
        b       1b
        nop
#endif
2:
