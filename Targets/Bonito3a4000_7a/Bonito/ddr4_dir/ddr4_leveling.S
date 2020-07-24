//#include "regdef.h"
//#include "mipsregs.h"
//#include "cacheops.h"

#define CHECKCOUNT          0x20
#define CHECKCOUNT_G        0x20
#define IDLE_TIMES          0x1000
#define WAIT_ITEM           60
#define DLL_GATE_STORE      0x3300
#define DLY_2X_STORE        0x3310
#define RDDATA_STORE        0x3319
#define RDDATA_FIND_FLAG    0x3330
#define GATE_POSITION_STORE 0x3300
#define CONTINUE_VALUE      0x20
#define FLUCTUATION_MARGIN_VALUE 0x10
#define GATE_MARGIN_VALUE   0x20
#define GATE_RDDATA_START   0x6
//#define LVL_DEBUG

//#define PRINTSTR(x) \
//    .rdata;98: .asciz x; .text; la a0, 98b; bal outputstring; nop


        .text
        .set    noreorder
        .set    mips3


####### leveling start ##################
        .global ddr4_leveling
        .ent    ddr4_leveling
ddr4_leveling:
    move    t9, ra

#ifdef LVL_DEBUG
    PRINTSTR("\r\nBegin leveling\r\n")
#endif

write_leveling_new:
#ifdef LVL_DEBUG
    PRINTSTR("\r\nBegin write leveling\r\n")
#endif
//note: reg usage
//t0 - number of dataslice
//t1 - addr of reg
//t2 - value of reg
//t3 - elastic reg
//t4 - count of dataslice (for i++)
//t5 - elastic reg
//t6 - elastic reg
//t7 - elastic reg
//t9 - elastic reg
//t8 - a0, 0x900000000ff00000
//s7 - dll_wrdq_*_add done, 0 - done, 1 - doing
//s6 - for addtional lvl req   //not used now
//s5 - lvl_resp, 0 - resp is 1, find 0; resp is 0, find 1

    GET_CS_NUM_DDR4
    move    t2, v0      //cs number in t2
    GET_LVL_CS_NUM
    move    t3, v0      //lvl cs in t3

    /*set DQ off for all cs other than lvl cs loop */
    dli     t0, 0
1:
    beq     t0, t3, 2f
    nop
    /*set DQ off*/
#ifndef DDR3_DIMM
    dsll    t1, t0, 4
    daddu   t1, t8
    lh      mrs_cmd_a, DDR4_MR1_CS0_REG(t1)
#else
    dsll    t1, t0, 3
    daddu   t1, t8
    lh      mrs_cmd_a, DDR3_MR1_CS0_REG(t1)
#endif
    or      mrs_cmd_a, (1<<12)
    move    mrs_cs, t0
    li      mrs_num, 1
    MRS_SEND(mrs_cmd_a,mrs_cs,mrs_num)
2:
    daddu   t0, 1
    bltu    t0, t2, 1b
    nop

#ifdef LVL_DEBUG
    PRINTSTR("\r\nall dll init value set 0\r\n")
#endif

/** 3. set leveling mode to be WRITE LEVELING **/
#ifndef DDR3_DIMM
    /*enable write lvl to side B for rdimm*/
    GET_LVL_CS_NUM
    move    mrs_cs, v0
    dsll    t1, v0, 4
    daddu   t1, t8
    lh      mrs_cmd_a, DDR4_MR1_CS0_REG(t1)
    or      mrs_cmd_a, (1<<7)
    li      mrs_num, 1
    MRS_SEND(mrs_cmd_a,mrs_cs,mrs_num)
#endif
lvl_mode:
    sb      zero, LVL_MODE_OFFSET(t8)
    WAIT_FOR(20000)
    li      t2, 1
    sb      t2, LVL_MODE_OFFSET(t8)

#ifdef LVL_DEBUG
    PRINTSTR("\r\nset leveling mode to be WRITE LEVELING\r\n")
#endif

/** 4. check whether to start leveling **/
lvl_ready:
    ld      t2, (PHY_ADDRESS + 0x708)(t8)
    and     t2, t2, 0xff
    beqz    t2, lvl_ready
    nop

#ifdef LVL_DEBUG
    PRINTSTR("\r\nwrite leveling ready\r\n")
#endif

/** 5. set leveling req for write leveling **/
    //s5 is used to indicate whether all dataslices got 0
    dli     s5, 0x0

check_bit_count_reset:
    dli     s7, 0x0
lvl_req_set:
    ld      t2, (PHY_ADDRESS + 0x700)(t8)
    dli     t3, 0xffffffffffff00ff
    and     t2, t2, t3
    ori     t2, t2, (0x1 << 8)
    sd      t2, (PHY_ADDRESS + 0x700)(t8)

#ifdef LVL_DEBUG
    PRINTSTR("\r\nwrite leveling req\r\n")
#endif

/** 6. check whether write leveling req done **/
lvl_done:
    ld      t2, (PHY_ADDRESS + 0x708)(t8)
    andi    t2, t2, (0x1 << 8)
    beqz    t2, lvl_done
    nop

#ifdef LVL_DEBUG
    PRINTSTR("\r\nwrite leveling done\r\n")
#endif

    andi    a0, s5, 0x10
    bnez    a0, lvl_resp_set_one
    nop

/** 7. check each dataslice's response to adjust the write dll **/
/** 7.1. ensure all dataslices got a 0 first **/
lvl_resp_set_zero:      //rdata[0+i*8]
    dli     t4, 0x0    //resp offset for each dataslice
//define the num of dataslice according to DIMM width
    GET_DIMM_WIDTH_V1
    beq     a1, 0x1, 1f
    nop
    beq     a1, 0x2, 2f
    nop
    beq     a1, 0x3, 3f
    nop
1:
    dli     t0, 0x2
    b       1f
    nop
2:
    dli     t0, 0x4
    b       1f
    nop
3:
    //identify whether there is ecc dataslice
    dli     t0, 0x8      //num of dataslice without ecc
    dli     t1, (CTL_ADDRESS + 0x280)
    or      t1, t1, t8
    lb      t2, 0x4(t1)
    beqz    t2, 1f
    nop
    daddiu  t0, t0, 0x1  //num of dataslice with ecc

1:
    andi    s5, s5, 0x1e

lvl_resp_set_0:
    dli     t1, (PHY_ADDRESS + 0x710)
    or      t1, t1, t8
    dli     t5, 0x01
    move    t6, t0
    beq     t4, t6, 4f  //all over
    nop
    ld      t2, (0x0)(t1)
    nop
    dsll    t6, t4, 0x3   //t6 = t4 *8
    dsll    t5, t5, t6
    and     t2, t2, t5
    dli     a0, 0x8
    bne     t4, a0, 8f
    nop
    ld      t2, (0x8)(t1)
    andi    t2, t2, 0x1

8:
    bnez    t2, wr_needadd1_0   //rdata[0+i*8] is 1
    nop
    b       wr_check0_ok
    nop
wr_needadd1_0:        //first clear check bit count
//    dsll    a0, t4, 0x2  //a0 = t4 *4
    dmulou  a0, t4, 0x7    //a0 = t4 *7
    dli     v0, 0x7f
    dsll    a0, v0, a0
    not     a0, a0
    and     s7, s7, a0
    b       dll_wrdq_add_0
    nop

wr_check0_ok:
//    dsll    a0, t4, 0x2  //a0 = t4 *4
    dmulou  a0, t4, 0x7    //a0 = t4 *7
    dli     v0, 0x1
    dsll    v0, v0, a0
    daddu   s7, s7, v0
    dsrl    v1, s7, a0
    andi    v1, v1, 0x7f
    dli     a0, CHECKCOUNT
    bne     a0, v1, 3f
    nop
    dsubu   s7, s7, v0
    b       2f
    nop
3:
    b       dll_wrdq_add_0
    nop


2:
    daddiu  t4, t4, 0x1
    bne     t4, t0, lvl_resp_set_0
    nop
4:
    andi    a0, s5, 0x1
    bnez    a0, lvl_req_set            //all dataslice finished dll_wrdq_add_1
    nop

    dli     t4, 0x0
1:
    dli     t1, (PHY_ADDRESS + 0x100)
    or      t1, t1, t8
    dsll    t5, t4, 0x7   //t5 = t4 * 0x80
    dadd    t1, t1, t5
    lb      t2, (0x0)(t1)
    andi    t6, t2, 0x7f      //dll_wrdq, the same as dll_1xdly
    dli     a0, CHECKCOUNT
    dsubu   a0, a0, 0x1
    dsubu   t6, t6, a0
    andi    t6, t6, 0x7f
    sb      t6, 0x0(t1)
    sb      t6, 0x3(t1)
    move    a0, t0
    daddiu  t4, t4, 0x1
    bne     t4, a0, 1b
    nop
    dli     t1, IDLE_TIMES    //wait when dll_wrdq is 0
1:
    daddiu  t1, t1, -0x1
    bnez    t1, 1b
    nop

//next step
    ori     s5, s5, 0x10
    b       check_bit_count_reset            //~rdata[0+i*8]
    nop

/** 7.2. start from all slice got 0, until all 1 found **/
lvl_resp_set_one:      //rdata[0+i*8]
    dli     t4, 0x0    //resp offset for each dataslice
//define the num of dataslice according to DIMM width
    GET_DIMM_WIDTH_V1
    beq     a1, 0x1, 1f
    nop
    beq     a1, 0x2, 2f
    nop
    beq     a1, 0x3, 3f
    nop
1:
    dli     t0, 0x2
    b       1f
    nop
2:
    dli     t0, 0x4
    b       1f
    nop
3:
    //identify whether there is ecc dataslice
    dli     t0, 0x8      //num of dataslice without ecc
    dli     t1, (CTL_ADDRESS + 0x280)
    or      t1, t1, t8
    lb      t2, 0x4(t1)
    beqz    t2, 1f
    nop
    daddiu  t0, t0, 0x1  //num of dataslice with ecc

1:
    andi    s5, s5, 0x1e

lvl_resp_set_1:
    dli     t1, (PHY_ADDRESS + 0x710)
    or      t1, t1, t8
    dli     t5, 0x01
    move    t6, t0
    beq     t4, t6, 4f  //all over
    nop
    ld      t2, (0x0)(t1)
    nop
    dsll    t6, t4, 0x3   //t6 = t4 * 8
    dsll    t5, t5, t6
    and     t2, t2, t5
    dli     a0, 0x8
    bne     t4, a0, 8f
    nop
    ld      t2, (0x8)(t1)
    andi    t2, t2, 0x1

8:
    beqz    t2, wr_needadd1_1   //rdata[0+i*8] is 1
    nop
    b       wr_check1_ok
    nop
wr_needadd1_1:        //first clear check bit count
    dmulou  a0, t4, 0x7    //a0 = t4 *7
    dli     v0, 0x7f
    dsll    a0, v0, a0
    not     a0, a0
    and     s7, s7, a0
    b       dll_wrdq_add_1
    nop

wr_check1_ok:
    dmulou  a0, t4, 0x7    //a0 = t4 *7
    dli     v0, 0x1
    dsll    v0, v0, a0
    daddu   s7, s7, v0
    dsrl    v1, s7, a0
    andi    v1, v1, 0x7f
    dli     a0, CHECKCOUNT
    bne     a0, v1, 3f
    nop
    dsubu   s7, s7, v0
    b       2f
    nop
3:
    b       dll_wrdq_add_1
    nop


2:
    daddiu  t4, t4, 0x1
    bne     t4, t0, lvl_resp_set_1
    nop
4:
    andi    a0, s5, 0x1
    bnez    a0, lvl_req_set            //all dataslice finished dll_wrdq_add_1
    nop

    dli     t4, 0x0
1:
    dli     t1, (PHY_ADDRESS + 0x100)
    or      t1, t1, t8
    dsll    t5, t4, 0x7   //t5 = t4 * 0x80
    dadd    t1, t1, t5
    lb      t2, (0x0)(t1)
    andi    t6, t2, 0x7f      //dll_wrdq, the same as dll_1xdly
    dli     a0, CHECKCOUNT
    dsubu   a0, a0, 0x1
    dsubu   t6, t6, a0
    andi    t6, t6, 0x7f
    sb      t6, 0x0(t1)
    sb      t6, 0x3(t1)
    move    a0, t0
    daddiu  t4, t4, 0x1
    bne     t4, a0, 1b
    nop
    dli     t1, IDLE_TIMES    //wait when dll_wrdq is 0
1:
    daddiu  t1, t1, -0x1
    bnez    t1, 1b
    nop

/** 8. All 1 found, set params according to wrdqs **/
/** 8.1. set dll_wrdq/dll_1xdly **/
    dli     t4, 0x0    //resp offset for each dataslice
//define the num of dataslice according to DIMM width
    GET_DIMM_WIDTH_V1
    beq     a1, 0x1, 1f
    nop
    beq     a1, 0x2, 2f
    nop
    beq     a1, 0x3, 3f
    nop
1:
    dli     t0, 0x2
    b       1f
    nop
2:
    dli     t0, 0x4
    b       1f
    nop
3:
    //identify whether there is ecc dataslice
    dli     t0, 0x8      //num of dataslice without ecc
    dli     t1, (CTL_ADDRESS + 0x280)
    or      t1, t1, t8
    lb      t2, 0x4(t1)
    beqz    t2, 1f
    nop
    daddiu  t0, t0, 0x1  //num of dataslice with ecc

1:
set_dll_wrdq_1xdly:
    dli     t1, (PHY_ADDRESS + 0x100)
    or      t1, t1, t8
    dsll    t5, t4, 0x7  //t5 = t4 * 0x80
    dadd    t1, t1, t5

    lb      t2, (0x0)(t1)
    dli     t6, 0x40
    subu    t2, t6
    dli     t6, 0x7f
    and     t2, t6
    sb      t2, 0x0(t1)
    sb      t2, 0x3(t1)

    daddiu  t4, t4, 0x1
    bne     t4, t0, set_dll_wrdq_1xdly
    nop

/** 8.2. set dly_2x **/
#ifdef TPHY_WR_MODE0
set_dly_2x_0_after_wrlvl:

#ifndef  AUTO_DDR_CONFIG
    dli     a0, CLK_FLY_BY_ORDER
#else
    dli     a0, 0x01234567
#endif

    dli     s5, 0x0
    GET_DIMM_TYPE_V1
    bnez    a1, rdimm_order
    nop
    dsubu   t1, t0, 1
set_dly_2x_after_wrlvl:
    dsll    t5, t1, 2
    dsrl    t5, a0, t5
    andi    t5, t5, 0xf     //t5 last slice num
    dsll    t5, t5, 7
    daddu   t5, t5, DDR4_DLL_WRDQ_OFFSET
    or      t5, t5, t8

    dsubu   t1, t1, 1
    dsll    t4, t1, 2
    dsrl    t4, a0, t4
    andi    t4, t4, 0xf     //t4 cur slice num
    dsll    t4, t4, 7
    daddu   t4, t4, DDR4_DLL_WRDQ_OFFSET
    or      t4, t4, t8

    lb      t6, 0x0(t5)
    lb      t3, 0x0(t4)
    ld      t2, (0x18)(t5)

    bgeu    t3, t6, 1f      //cur dll_wrdq > last dll_wrdq
    nop
    dli     t7, 0x50
    bltu    t7, t6, 2f
    nop
    dsubu   t7, t6, t3
    bleu    t7, 0x10, 1f
    nop
2:
    dli     t7, 0x3e
    bltu    t3, t7, 2f
    nop
    dsubu   t7, t6, t3
    bleu    t7, 0x10, 1f
    nop
2:
    dli     t7, 0x030000
    and     t6, t2, t7     //dly_2x[1:0]
    dli     t7, 0x010000
    dadd    t6, t6, t7
    dli     t7, 0x030000
    and     t6, t6, t7
    dli     t7, 0xfffffffffffcffff
    and     t2, t2, t7
    or      t2, t2, t6
    dli     t7, 0x0c0000
    and     t6, t2, t7     //dly_2x[3:2]
    dli     t7, 0x040000
    dadd    t6, t6, t7
    dli     t7, 0x0c0000
    and     t6, t6, t7
    dli     t7, 0xfffffffffff3ffff
    and     t2, t2, t7
    or      t2, t2, t6
    dli     t7, 0x300000
    and     t6, t2, t7    //dly_2x[5:4]
    dli     t7, 0x100000
    dadd    t6, t6, t7
    dli     t7, 0x300000
    and     t6, t6, t7
    dli     t7, 0xffffffffffcfffff
    and     t2, t2, t7
    or      t2, t2, t6

    dli     s5, 0x1
1:
    sd      t2, (0x18)(t4)

    bnez    t1, set_dly_2x_after_wrlvl
    nop
    b       over_cfg_2xdly
    nop

rdimm_order:
    dli     t4, 0x0
    dli     a0, 0x9
    beq     a0, t0, 1f
    nop
    dli     t4, 0x1    //resp offset for each dataslice
1:

set_dly_2x_after_wrlvl_r:
    dli     t1, (PHY_ADDRESS + 0x100)
    or      t1, t1, t8
    move    v1, t4
    bal     table_check_ds
    nop
    dsll    t5, v0, 0x7   //t5 = v0 * 0x80
    dadd    t1, t1, t5
    ld      t2, (0x0 )(t1)       //last dll_wrdq
    dli     t7, 0xff
    and     t6, t2, t7

    dli     t1, (PHY_ADDRESS + 0x100)
    or      t1, t1, t8
    daddiu  v1, t4, 0x1
    bal     table_check_ds
    nop
    dsll    t5, v0, 0x7   //t5 = v0 * 0x80
    dadd    t1, t1, t5
    ld      t3, (0x0 )(t1)
    and     t3, t3, t7

    dli     t1, (PHY_ADDRESS + 0x100)
    or      t1, t1, t8
    move    v1, t4
    bal     table_check_ds
    nop
    dsll    t5, v0, 0x7   //t5 = v0 * 0x80
    dadd    t1, t1, t5
    ld      t2, (0x18)(t1)
    bgeu    t3, t6, 1f      //cur dll_wrdq > last dll_wrdq
    nop
    dli     t7, 0x50
    bltu    t7, t6, 2f
    nop
    dsubu   t7, t6, t3
    bleu    t7, 0x10, 1f
    nop
2:
    dli     t7, 0x3e
    bltu    t3, t7, 2f
    nop
    dsubu   t7, t6, t3
    bleu    t7, 0x10, 1f
    nop
2:

add_dly_2x:
    dli     t7, 0x030000
    and     t6, t2, t7     //dly_2x[1:0]
    dli     t7, 0x010000
    dadd    t6, t6, t7
    dli     t7, 0x030000
    and     t6, t6, t7
    dli     t7, 0xfffffffffffcffff
    and     t2, t2, t7
    or      t2, t2, t6
    dli     t7, 0x0c0000
    and     t6, t2, t7     //dly_2x[3:2]
    dli     t7, 0x040000
    dadd    t6, t6, t7
    dli     t7, 0x0c0000
    and     t6, t6, t7
    dli     t7, 0xfffffffffff3ffff
    and     t2, t2, t7
    or      t2, t2, t6
    dli     t7, 0x300000
    and     t6, t2, t7    //dly_2x[5:4]
    dli     t7, 0x100000
    dadd    t6, t6, t7
    dli     t7, 0x300000
    and     t6, t6, t7
    dli     t7, 0xffffffffffcfffff
    and     t2, t2, t7
    or      t2, t2, t6

    dli     s5, 0x1
1:
    dli     t1, (PHY_ADDRESS + 0x100)
    or      t1, t1, t8
    daddiu  v1, t4, 0x1
    bal     table_check_ds
    nop
    dsll    t5, v0, 0x7   //t5 = v0 * 0x80
    dadd    t1, t1, t5
    sd      t2, (0x18)(t1)

    daddiu  t4, t4, 0x1
    dli     a0, 0x4
    bne     t4, a0, 4f
    nop
    daddiu  t4, t4, 0x1
4:
    dli     a0, 0x8
    bne     t4, a0, set_dly_2x_after_wrlvl_r
    nop

over_cfg_2xdly:
    beqz    s5, 1f
    nop
    lb      t0, 0x1062(t8)
    subu    t0, 0x1
    sb      t0, 0x1062(t8)
    lb      t0, 0x1065(t8)
    subu    t0, 0x1
    sb      t0, 0x1065(t8)

1:
#endif

/** exit **/
    b       write_leveling_exit_new
    nop

dll_wrdq_add_0:
#ifdef LVL_DEBUG
    PRINTSTR("\r\nSlice ")
    move    a0, t4
    bal     hexserial
    nop
    PRINTSTR(": for all write resp got a 0, slice add\r\n")
#endif
    dli     t1, (PHY_ADDRESS + 0x100)
    or      t1, t1, t8
    dsll    t5, t4, 0x7   //t5 = t4 * 0x80
    dadd    t1, t1, t5
    ld      t2, (0x0)(t1)
    andi    t6, t2, 0x7f      //dll_wrdq
    bnez    t6, 2f
    nop

    dli     t7, IDLE_TIMES    //wait when dll_wrdq is 0
1:
    daddiu  t7, t7, -0x1
    bnez    t7, 1b
    nop
2:
    dli     t7, 0x1
    dadd    t6, t6, t7
    dli     t5, 0x7f
    and     t6, t6, t5
    sb      t6, 0x0(t1)
    sb      t6, 0x3(t1)
    daddiu  t4, t4, 0x1           //next dataslice
    bne     t4, t0, lvl_resp_set_0
    ori     s5, s5, 0x1
    b       lvl_req_set
    nop

dll_wrdq_add_1:
#ifdef LVL_DEBUG
    PRINTSTR("\r\nSlice ")
    move    a0, t4
    bal     hexserial
    nop
    PRINTSTR(": for all write resp got a 1, slice add\r\n")
#endif
    dli     t1, (PHY_ADDRESS + 0x100)
    or      t1, t1, t8
    dsll    t5, t4, 0x7   //t5 = t4 * 0x80
    dadd    t1, t1, t5
    ld      t2, (0x0)(t1)
    andi    t6, t2, 0x7f      //dll_wrdq, the same as dll_1xdly
    bnez    t6, 2f
    nop

    dli     t5, IDLE_TIMES    //wait when dll_wrdq is 0
1:
    daddiu  t5, t5, -0x1
    bnez    t5, 1b
    nop
2:
    dli     t7, 0x1
    dadd    t6, t6, t7
    dli     t5, 0x7f
    and     t6, t6, t5
    sb      t6, 0x0(t1)
    sb      t6, 0x3(t1)
    daddiu  t4, t4, 0x1           //next dataslice
    bne     t4, t0, lvl_resp_set_1
    ori     s5, s5, 0x1
    b       lvl_req_set
    nop

write_leveling_exit_new:
    sb      zero, LVL_MODE_OFFSET(t8)
#ifndef DDR3_DIMM
    /*disable write lvl to side B for rdimm*/
    GET_LVL_CS_NUM
    move    mrs_cs, v0
    dsll    t1, v0, 4
    daddu   t1, t8
    lh      mrs_cmd_a, DDR4_MR1_CS0_REG(t1)
    and     mrs_cmd_a, ~(1<<7)
    li      mrs_num, 1
    MRS_SEND(mrs_cmd_a,mrs_cs,mrs_num)
#endif
    WAIT_FOR(20000)

    GET_CS_NUM_DDR4
    move    t2, v0      //cs number in t2

    /*set DQ on for all cs other than lvl cs loop */
    dli     t0, 0
1:
    /*set DQ on*/
#ifndef DDR3_DIMM
    dsll    t1, t0, 4
    daddu   t1, t8
    lh      mrs_cmd_a, DDR4_MR1_CS0_REG(t1)
#else
    dsll    t1, t0, 3
    daddu   t1, t8
    lh      mrs_cmd_a, DDR3_MR1_CS0_REG(t1)
#endif
    and     mrs_cmd_a, ~(1<<12)
    move    mrs_cs, t0
    li      mrs_num, 1
    MRS_SEND(mrs_cmd_a,mrs_cs,mrs_num)
    daddu   t0, 1
    bltu    t0, t2, 1b
    nop

gate_leveling_new:
//init rddata
    dli     t0, 0x3
    lb      t2, DDR4_2T_OFFSET(t8)
    beqz    t2, 1f
    nop
    daddu   t0, t0, 1
1:
    sb      t0, TPHY_RDDATA_OFFSET(t8)

    li      t1, 0
    sd      $0, RDDATA_STORE(t8)
    sb      $0, (RDDATA_STORE+8)(t8)
    sh      t1, RDDATA_FIND_FLAG(t8)
    dli     s7, 0
    dli     s5, 0
#ifdef LVL_DEBUG
    PRINTSTR("\r\nwrite leveling finish and gate leveling begin\r\n")
#endif

#ifndef DDR3_DIMM
    /*enable MPR mode to side B for rdimm*/
    GET_LVL_CS_NUM
    move    mrs_cs, v0
    dsll    t1, v0, 4
    daddu   t1, t8
    lh      mrs_cmd_a, DDR4_MR3_CS0_REG(t1)
    or      mrs_cmd_a, (1<<2)
    and     mrs_cmd_a, ~(0x3)
    li      mrs_num, 3
    MRS_SEND(mrs_cmd_a,mrs_cs,mrs_num)

    /* enable read_preamble_training mode*/
    GET_LVL_CS_NUM
    move    mrs_cs, v0
    dsll    t1, v0, 4
    daddu   t1, t8
    lh      mrs_cmd_a, DDR4_MR4_CS0_REG(t1)
    or      mrs_cmd_a, (1<<10)
    li      mrs_num, 4
    MRS_SEND(mrs_cmd_a,mrs_cs,mrs_num)

#endif
//set gate leveling modes
    sb      zero, LVL_MODE_OFFSET(t8)
    WAIT_FOR(20000)
    li      t2, 2
    sb      t2, LVL_MODE_OFFSET(t8)

glvl_ready:
    ld      t2, (PHY_ADDRESS + 0x708)(t8)
    and     t2, t2, 0xff
    beqz    t2, glvl_ready
    nop

//ddr3 skip send mr4
    GET_SDRAM_TYPE_V1
    beq     a1, 3, ddr3_gate_leveling
    nop

2:
//init dll gate and dly_2x
    move    t1, t8
    dli     t2, 0
    dli     t4, 9
1:
    sb      t2, DLL_GATE_OFFSET(t1)
    lb      t3, DLY_2X_OFFSET(t1)
    dli     t5, 0x3c
    not     t5
    and     t3, t5
    sb      t3, DLY_2X_OFFSET(t1)
    daddu   t1, 0x80
    dsubu   t4, 1
    bnez    t4, 1b
    nop
#ifdef LVL_DEBUG
    bal     print_dqs
    nop
#endif
glvl_req_set_rddata:
    lh      t1, RDDATA_FIND_FLAG(t8)
    GET_DIMM_WIDTH_V1
    beq     a1, 0x1, 1f
    nop
    beq     a1, 0x2, 2f
    nop
    beq     a1, 0x3, 3f
    nop
1:
    dli     t0, 0x3
    b       1f
    nop
2:
    dli     t0, 0xf
    b       1f
    nop
3:
    dli     t0, 0xff
    lb      t2, 0x1284(t8)
    beqz    t2, 1f
    nop
    dli     t0, 0x1ff
1:
    beq     t1, t0, exit_rddata_find
    nop

    ld      t2, (PHY_ADDRESS + 0x700)(t8)
    and     t2, t2, 0xffffffffffff00ff
    ori     t2, t2, (0x1 << 8)
    sd      t2, (PHY_ADDRESS + 0x700)(t8)

glvl_done_rddata:
    ld      t2, (PHY_ADDRESS + 0x708)(t8)
    and     t2, t2, (0x1 << 8)
    beqz    t2, glvl_done_rddata
    nop

    dli     t4, 0
//define the num of dataslice according to DIMM width
    GET_DIMM_WIDTH_V1
    beq     a1, 0x1, 1f
    nop
    beq     a1, 0x2, 2f
    nop
    beq     a1, 0x3, 3f
    nop
1:
    dli     t0, 0x2
    b       1f
    nop
2:
    dli     t0, 0x4
    b       1f
    nop
3:
    //identify whether there is ecc dataslice
    dli     t0, 0x8      //num of dataslice without ecc
    dli     t1, (CTL_ADDRESS + 0x280)
    or      t1, t1, t8
    lb      t2, 0x4(t1)
    beqz    t2, 1f
    nop
    daddiu  t0, t0, 0x1  //num of dataslice with ecc

1:
    andi    s5, s5, 0x1e
glvl_resp_set_1_rddata:
    lh      t1, RDDATA_FIND_FLAG(t8)
    li      t2, 1
    dsll    t2, t4
    and     t1, t2
    beqz    t1, 1f
    nop
    b       dll_gate_add_1_done_rddata
    nop
1:
    dli     t1, (PHY_ADDRESS + 0x710)
    or      t1, t1, t8
    dli     t5, 0x01
    beq     t4, t0, glvl_req_set_rddata  //all over
    nop
    ld      t2, (0x0)(t1)
    nop
    dsll    t6, t4, 0x3   //t6 = t4 *8
    dsll    t5, t5, t6
    and     t2, t2, t5
    dli     a0, 0x8
    bne     t4, a0, 8f
    nop
    ld      t2, (0x8)(t1)
    and     t2, t2, 0x1
8:
    beqz    t2, rd_needadd1_1_rddata   //rdata[0+i*8] is 1
    nop
    b       rd_check1_ok_rddata
    nop
rd_needadd1_1_rddata:        //first clear check bit count
    dmulou  a0, t4, 0x7    //a0 = t4 *7
    dli     v0, 0x7f
    dsll    a0, v0, a0
    not     a0, a0
    and     s7, s7, a0
    b       dll_gate_add_1_rddata
    nop

rd_check1_ok_rddata:
    dmulou  a0, t4, 0x7    //a0 = t4 *7
    dli     v0, 0x1
    dsll    v0, v0, a0
    daddu   s7, s7, v0
    dsrl    t5, s7, a0
    andi    t5, t5, 0x7f
    bne     t5, 0x20, dll_gate_add_1_rddata    //found 1/4 period of continue 1
    nop
    lb      t2, TPHY_RDDATA_OFFSET(t8)
    daddu   t1, t4, t8
    sb      t2, RDDATA_STORE(t1)    //store rddata for every slice
    li      t2, 1
    dsll    t2, t4
    lh      t5, RDDATA_FIND_FLAG(t8)//set flag for rddata found of every slice
    or      t5, t2
    sh      t5, RDDATA_FIND_FLAG(t8)
    b       dll_gate_add_1_done_rddata
    nop

dll_gate_add_1_rddata:
#ifdef LVL_DEBUG
    PRINTSTR("\r\nfor find rddata, tRDDATA=")
    lb      a0, TPHY_RDDATA_OFFSET(t8)
    bal     hexserial
    nop
    PRINTSTR(" slice_No.=")
    move    a0, t4
    bal     hexserial
    nop
    PRINTSTR(" dll_gate=")
    dli     t1, (PHY_ADDRESS + 0x108)
    or      t1, t1, t8
    dsll    t5, t4, 0x7   //t5 = t4 * 0x80
    dadd    t1, t1, t5
    ld      t2, (0x0)(t1)
    dli     t7, 0x7f0000
    and     t6, t2, t7      //dll_gate
    dsrl    a0, t6, 16
    bal     hexserial
    nop
    PRINTSTR(" s7=")
    move    a0, s7
    bal     hexserial64
    nop
#endif
    dli     t1, (PHY_ADDRESS + 0x108)
    or      t1, t1, t8
    dsll    t5, t4, 0x7   //t5 = t4 * 0x80
    dadd    t1, t1, t5
    ld      t2, (0x0)(t1)
    dli     t7, 0x7f0000
    and     t6, t2, t7      //dll_gate
    dli     t7, (0x1 << 16)
    dadd    t6, t6, t7
    dli     t5, 0x00000000007f0000
    and     t6, t6, t5
    dli     t5, 0xffffffffff00ffff
    and     t2, t2, t5
    or      t2, t2, t6
    sd      t2, (0x0)(t1)

    dli     t7, 0x7f0000
    and     t6, t2, t7
    bnez    t6, dll_gate_add_1_done_rddata
    nop

    dli     t2, IDLE_TIMES    //wait when dll_wrdq is 0
1:
    daddiu  t2, t2, -0x1
    bnez    t2, 1b
    nop

    ori     s5, 0x1

    dli     t3, 128
sync_dll_code_1_rddata:
    dli     t1, (PHY_ADDRESS + 0x700)
    or      t1, t1, t8
    ld      t2, (0x0)(t1)
    dli     t7, 0xffffffffffff00ff
    and     t2, t2, t7
    dli     t7, (0x1 << 8)
    or      t2, t2, t7
    sd      t2, (0x0)(t1)
glvl_done_sync_1_rddata:
    ld      t2, (0x8)(t1)
    dli     t7, (0x1 << 8)
    and     t2, t2, t7
    beqz    t2, glvl_done_sync_1_rddata
    nop
    daddiu  t3, t3, -0x1
    bnez    t3, sync_dll_code_1_rddata
    nop
dll_gate_add_1_done_rddata:
    daddiu  t4, t4, 0x1           //next dataslice
    bne     t4, t0, glvl_resp_set_1_rddata
    nop
    andi    t1, s5, 0x1
    beqz    t1, 1f
    nop
    lb      t2, TPHY_RDDATA_OFFSET(t8)
    daddu   t2, 1
    sb      t2, TPHY_RDDATA_OFFSET(t8)
1:
    b       glvl_req_set_rddata
    nop
//sub 0x40
exit_rddata_find:
/** 14. All 1 found, set params according to wrdqs **/
/** 14.1. set dll_gate and dly_2x[3:2] **/
    dli     t4, 0x0    //resp offset for each dataslice
//define the num of dataslice according to DIMM width
    GET_DIMM_WIDTH_V1
    beq     a1, 0x1, 1f
    nop
    beq     a1, 0x2, 2f
    nop
    beq     a1, 0x3, 3f
    nop
1:
    dli     t0, 0x2
    b       1f
    nop
2:
    dli     t0, 0x4
    b       1f
    nop
3:
    //identify whether there is ecc dataslice
    dli     t0, 0x8      //num of dataslice without ecc
    dli     t1, (CTL_ADDRESS + 0x280)
    or      t1, t1, t8
    lb      t2, 0x4(t1)
    beqz    t2, 1f
    nop
    daddiu  t0, t0, 0x1  //num of dataslice with ecc
1:

    lb      t2, RDDATA_STORE(t8)
    sb      t2, TPHY_RDDATA_OFFSET(t8)
set_dll_gate_dly2x:
    dli     t1, (PHY_ADDRESS + 0x108)
    or      t1, t1, t8
    dsll    t5, t4, 0x7   //t5 = t4 * 0x80
    daddu   t1, t1, t5
    ld      t2, (0x0)(t1)
    dli     t3, 0xff0000
    and     t6, t2, t3
    dli     t3, 0x400000
    dsubu   t6, t6, t3
    dli     t3, 0x00000000007f0000
    and     t6, t6, t3
    dli     t3, 0xffffffffff00ffff
    and     t2, t2, t3
    or      t2, t2, t6
    sd      t2, (0x0)(t1)   //keep dll_gate
    dli     t7, 0x7f0000
    and     t2, t2, t7
    dli     t3, 0x400000
    daddu   t1, t4, t8
    lb      t5, RDDATA_STORE(t1)
    bltu    t2, t3, 1f      //keep TPHY_RDDATA_OFFSET
    nop
    subu    t5, 1
    sb      t5, RDDATA_STORE(t1)
1:
    lb      t2, TPHY_RDDATA_OFFSET(t8)
    bleu    t2, t5, 1f
    nop
    sb      t5, TPHY_RDDATA_OFFSET(t8)
1:
    daddiu  t4, t4, 0x1
    bltu    t4, t0, set_dll_gate_dly2x
    nop

//set dly_2x
    GET_DIMM_WIDTH_V1
    beq     a1, 0x1, 1f
    nop
    beq     a1, 0x2, 2f
    nop
    beq     a1, 0x3, 3f
    nop
1:
    dli     t0, 0x2
    b       1f
    nop
2:
    dli     t0, 0x4
    b       1f
    nop
3:
    //identify whether there is ecc dataslice
    dli     t0, 0x8      //num of dataslice without ecc
    dli     t1, (CTL_ADDRESS + 0x280)
    or      t1, t1, t8
    lb      t2, 0x4(t1)
    beqz    t2, 1f
    nop
    daddiu  t0, t0, 0x1  //num of dataslice with ecc
1:

    lb      t3, TPHY_RDDATA_OFFSET(t8)
    dli     t4, 0
set_dly2x:
    daddu   t1, t4, t8
    lb      t5, RDDATA_STORE(t1)
    subu    t5, t5, t3
    dsll    t6, t4, 7   //t4*0x80
    daddu   t6, t6, t8
    lb      t2, DLY_2X_OFFSET(t6)
    and     t2, 0xc3
    dsll    t1, t5, 2
    or      t5, t5, t1
    dsll    t5, t5, 2
    or      t2, t5
    sb      t2, DLY_2X_OFFSET(t6)

    daddiu  t4, 1
    bltu    t4, t0, set_dly2x
    nop

    b       gate_leveling_exit_new
    nop

ddr3_gate_leveling:
/************************
t0:    dataslice number
t1-t3: elastic reg
t4:    dataslice under training
t5:    dqs count;smallest rddata value
t6:    count number for continues 0/1
t7:    0-find continues 1
       1-find continues 0
DLL_GATE_POSITION_STORE use 18 Bytes.
each dataslice use 2 bytes.
one for rddata and another foe dll_gate
DLL_GATE_POSITION_STORE use reg 0x3300,0x3308,0x3310.
************************/
#ifdef LVL_DEBUG
    bal     print_dqs
    nop
#endif
    GET_DIMM_WIDTH_V1
    beq     a1, 0x1, 1f
    nop
    beq     a1, 0x2, 2f
    nop
    beq     a1, 0x3, 3f
    nop
1:
    dli     t0, 0x2
    b       1f
    nop
2:
    dli     t0, 0x4
    b       1f
    nop
3:
    //identify whether there is ecc dataslice
    dli     t0, 0x8      //num of dataslice without ecc
    lb      t1, DDR4_ECC_EN_OFFSET(t8)
    beqz    t1, 1f
    nop
    daddiu  t0, t0, 0x1  //num of dataslice with ecc
1:

    li      t4, 0
ddr3_gate_lvl_slice:
//init rddata and dll_gate and dly_2x
    dli     t1, GATE_RDDATA_START
    lb      t2, DDR4_2T_OFFSET(t8)
    beqz    t2, 1f
    nop
    daddu   t1, t1, 1
1:
    sb      t1, TPHY_RDDATA_OFFSET(t8)

    dsll    t2, t4, 0x7   //t1 = t4 * 0x80
    dadd    t2, t2, t8
    sb      zero, DLL_GATE_OFFSET(t2)

    lb      t1, DLY_2X_OFFSET(t2)
    andi    t1, 0x3
    sb      t1, DLY_2X_OFFSET(t2)

#ifdef LVL_DEBUG
    PRINTSTR("\r\ndataslice No")
    move    a0, t4
    bal     hexserial
    nop
#endif
    dli     t5, 0
    dli     t7, 0
    dli     t6, 0
ddr3_gate_lvl_req_set:
//set lvl req
    li      t1, 0
    sb      t1, LVL_REQ_OFFSET(t8)
    li      t1, 1
    sb      t1, LVL_REQ_OFFSET(t8)
//check lvl done
1:
    lb      t1, LVL_DONE_OFFSET(t8)
    and     t1, t1, 0x1
    beqz    t1, 1b
    nop

    daddu   t2, t8, t4
    lb      t1, LVL_RESP_OFFSET(t2)
    andi    t1, t1, 0x1
    bnez    t7, ddr3_gate_dll_find_0
    nop
//ddr3_gate_dll_find_1
    bnez    t1, 1f
    nop
    li      t6, 0
    li      t5, 0
    b       ddr3_gate_dll_add_1
    nop
1:
    bnez    t5, 1f
    nop
    bnez    t6, 1f
    nop
ddr3_store_gate_position:
//store gate position DLL_GATE_POSITION_STORE only in first found 1
    dsll    t1, t4, 0x7   //t1 = t4 * 0x80
    dadd    t2, t1, t8
    lb      t1, DLL_GATE_OFFSET(t2)
    andi    t1, 0xff
    lb      t2, TPHY_RDDATA_OFFSET(t8)
    andi    t2, 0xff
    dsll    t2, t2, 8
    or      t1, t1, t2
    dsll    t2, t4, 1
    dadd    t2, t2, t8
    sh      t1, GATE_POSITION_STORE(t2)
#ifdef LVL_DEBUG
    PRINTSTR("\r\nfirst position store")
    PRINTSTR("\r\n0x3300:")
    ld      a0, 0x3300(t8)
    dsrl    a0, 32
    bal     hexserial
    nop
    ld      a0, 0x3300(t8)
    bal     hexserial
    nop
    PRINTSTR("\r\n0x3308:")
    ld      a0, 0x3308(t8)
    dsrl    a0, 32
    bal     hexserial
    nop
    ld      a0, 0x3308(t8)
    bal     hexserial
    nop
#endif
1:
//find continus 1
ddr3_find_1:
    daddu   t6, t6, 1
#ifdef LVL_DEBUG
    PRINTSTR("\r\nFIND 1 count =")
    move    a0, t6
    bal     hexserial
    nop
    PRINTSTR(", dqs count =")
    move    a0, t5
    bal     hexserial
    nop
    PRINTSTR(", RDDAATA=")
    lb      a0, TPHY_RDDATA_OFFSET(t8)
    bal     hexserial
    nop
    PRINTSTR(", dll_gate=")
    dsll    t3, t4, 0x7   //t1 = t4 * 0x80
    dadd    t2, t3, t8
    lb      a0, DLL_GATE_OFFSET(t2)
    bal     hexserial
    nop
#endif
    bleu    t6, CONTINUE_VALUE, ddr3_gate_dll_add_1
    nop
//next find continues 0 and first set gate to expected start position of continues 0
    dsll    t2, t4, 1
    daddu   t2, t2, t8
//set rddata
    lb      t1, GATE_POSITION_STORE(t2)
    li      t3, 0
    bltu    t1, (0x40-FLUCTUATION_MARGIN_VALUE), 1f     //t1<0x80-(0x40+FLUCTUATION_MARGIN_VALUE), then rddata + 1
    nop
    daddu   t3, t3, 1
1:
    daddu   t2, t2, 1
    lb      t1, GATE_POSITION_STORE(t2)
    daddu   t1, t1, t5
    daddu   t1, t1, t3
    sb      t1, TPHY_RDDATA_OFFSET(t8)
//set dll_gate
    dsubu   t2, t2, 1
    lb      t1, GATE_POSITION_STORE(t2)
    daddu   t1, t1, (0x40+FLUCTUATION_MARGIN_VALUE)
    andi    t1, 0x7f
    dsll    t3, t4, 0x7   //t1 = t4 * 0x80
    dadd    t2, t3, t8
    sb      t1, DLL_GATE_OFFSET(t2)

    bnez    t1, 3f
    nop

    dli     a0, IDLE_TIMES    //wait when dll_wrdq is 0
1:
    daddiu  a0, a0, -0x1
    bnez    a0, 1b
    nop

    dli     a1, 128
//sync_dll_code_1
1:
//set lvl req
    li      a0, 0
    sb      a0, LVL_REQ_OFFSET(t8)
    li      a0, 1
    sb      a0, LVL_REQ_OFFSET(t8)
//check lvl done
2:
    lb      a0, LVL_DONE_OFFSET(t8)
    and     a0, a0, 0x1
    beqz    a0, 2b
    nop
    daddiu  a1, a1, -0x1
    bnez    a1, 1b
    nop
3:

    dli     t6, 0
    dli     t7, 0x1
//for sync
    lb      a0, TPHY_RDDATA_OFFSET(t8)
    lb      a0, DLL_GATE_OFFSET(t2)
    lb      a0, TPHY_RDDATA_OFFSET(t8)
    lb      a0, DLL_GATE_OFFSET(t2)
    lb      a0, TPHY_RDDATA_OFFSET(t8)
    lb      a0, DLL_GATE_OFFSET(t2)
#ifdef LVL_DEBUG
    PRINTSTR("\r\nnow start find 0")
    PRINTSTR(", RDDAATA=")
    lb      a0, TPHY_RDDATA_OFFSET(t8)
    bal     hexserial
    nop
    PRINTSTR(", dll_gate=")
    dsll    t3, t4, 0x7   //t1 = t4 * 0x80
    dadd    t2, t3, t8
    lb      a0, DLL_GATE_OFFSET(t2)
    bal     hexserial
    nop
#endif
    b       ddr3_gate_lvl_req_set
    nop

ddr3_gate_dll_find_0:
    beqz    t1, 1f
    nop
    dli     t5, 0
    dli     t6, 0
    dli     t7, 0
    b       ddr3_store_gate_position
    nop
1:
ddr3_find_0:
    daddu   t6, t6, 1
#ifdef LVL_DEBUG
    PRINTSTR("\r\nFIND 0 count =")
    move    a0, t6
    bal     hexserial
    nop
    PRINTSTR("  dqs count =")
    move    a0, t5
    bal     hexserial
    nop
    PRINTSTR(", RDDAATA=")
    lb      a0, TPHY_RDDATA_OFFSET(t8)
    bal     hexserial
    nop
    PRINTSTR(", dll_gate=")
    dsll    t3, t4, 0x7   //t1 = t4 * 0x80
    dadd    t2, t3, t8
    lb      a0, DLL_GATE_OFFSET(t2)
    bal     hexserial
    nop
#endif
    bleu    t6, CONTINUE_VALUE, ddr3_gate_dll_add_1
    nop
//found continues 1 and 0, counter + 1 and set next expected start position of continues 1
    daddu   t5, t5, 1
    dsll    t2, t4, 1
    daddu   t2, t2, t8
//set rddata
    lb      t1, GATE_POSITION_STORE(t2)
    li      t3, 0
    bltu    t1, (0x80-FLUCTUATION_MARGIN_VALUE), 1f     //t1<0x80-FLUCTUATION_MARGIN_VALUE, then rddata + 1
    nop
    daddu   t3, t3, 1
1:
    daddu   t2, t2, 1
    lb      t1, GATE_POSITION_STORE(t2)
    daddu   t1, t1, t5
    daddu   t1, t1, t3
    sb      t1, TPHY_RDDATA_OFFSET(t8)
//set dll_gate
    dsubu   t2, t2, 1
    lb      t1, GATE_POSITION_STORE(t2)
    daddu   t1, t1, FLUCTUATION_MARGIN_VALUE
    andi    t1, 0x7f
    dsll    t3, t4, 0x7   //t1 = t4 * 0x80
    dadd    t2, t3, t8
    sb      t1, DLL_GATE_OFFSET(t2)

    bnez    t1, 3f
    nop

    dli     a0, IDLE_TIMES    //wait when dll_wrdq is 0
1:
    daddiu  a0, a0, -0x1
    bnez    a0, 1b
    nop

    dli     a1, 128
//sync_dll_code_1
1:
//set lvl req
    li      a0, 0
    sb      a0, LVL_REQ_OFFSET(t8)
    li      a0, 1
    sb      a0, LVL_REQ_OFFSET(t8)
//check lvl done
2:
    lb      a0, LVL_DONE_OFFSET(t8)
    and     a0, a0, 0x1
    beqz    a0, 2b
    nop
    daddiu  a1, a1, -0x1
    bnez    a1, 1b
    nop
3:

//for sync
    lb      a0, TPHY_RDDATA_OFFSET(t8)
    lb      a0, DLL_GATE_OFFSET(t2)
    lb      a0, TPHY_RDDATA_OFFSET(t8)
    lb      a0, DLL_GATE_OFFSET(t2)
    lb      a0, TPHY_RDDATA_OFFSET(t8)
    lb      a0, DLL_GATE_OFFSET(t2)
#ifdef LVL_DEBUG
    PRINTSTR("\r\nnow start next find 1")
    PRINTSTR(", dqs count =")
    move    a0, t5
    bal     hexserial
    nop
    PRINTSTR(", RDDAATA=")
    lb      a0, TPHY_RDDATA_OFFSET(t8)
    bal     hexserial
    nop
    PRINTSTR(", dll_gate=")
    dsll    t3, t4, 0x7   //t1 = t4 * 0x80
    dadd    t2, t3, t8
    lb      a0, DLL_GATE_OFFSET(t2)
    bal     hexserial
    nop
#endif

    dli     t6, 0
    dli     t7, 0
    bltu    t5, 4, ddr3_gate_lvl_req_set
    nop
//dataslice control
    daddu   t4, t4, 1
    bltu    t4, t0, ddr3_gate_lvl_slice
    nop
#ifdef LVL_DEBUG
    PRINTSTR("\r\n0x3300:")
    ld      a0, 0x3300(t8)
    dsrl    a0, 32
    bal     hexserial
    nop
    ld      a0, 0x3300(t8)
    bal     hexserial
    nop
    PRINTSTR("\r\n0x3308:")
    ld      a0, 0x3308(t8)
    dsrl    a0, 32
    bal     hexserial
    nop
    ld      a0, 0x3308(t8)
    bal     hexserial
    nop
#endif

//subu 0x20 for gate and find smallest rddata t5 and set finally dll_gate
    dli     t4, 0
    dli     t5, 0x3f
ddr3_gate_margin:
//store dll_gate
    dsll    t2, t4, 1
    daddu   t2, t2, t8
    lb      t1, GATE_POSITION_STORE(t2)
    dsubu   t3, t1, GATE_MARGIN_VALUE
    andi    t3, 0x7f
    dsll    t6, t4, 0x7   //t1 = t4 * 0x80
    dadd    t6, t6, t8
    sb      t3, DLL_GATE_OFFSET(t6)

    bnez    t1, 3f
    nop

    dli     a0, IDLE_TIMES    //wait when dll_wrdq is 0
1:
    daddiu  a0, a0, -0x1
    bnez    a0, 1b
    nop

    dli     a1, 128
//sync_dll_code_1
1:
//set lvl req
    li      a0, 0
    sb      a0, LVL_REQ_OFFSET(t8)
    li      a0, 1
    sb      a0, LVL_REQ_OFFSET(t8)
//check lvl done
2:
    lb      a0, LVL_DONE_OFFSET(t8)
    and     a0, a0, 0x1
    beqz    a0, 2b
    nop
    daddiu  a1, a1, -0x1
    bnez    a1, 1b
    nop
3:

//store rddata and find smallest rddata
    li      t3, 0
    bgeu    t1, GATE_MARGIN_VALUE, 1f     //t1<GATE_MARGIN_VALUE, then rddata - 1
    nop
    daddu   t3, t3, 1
1:
    daddu   t2, t2, 1
    lb      t1, GATE_POSITION_STORE(t2)
    dsubu   t1, t1, t3
    bgeu    t1, t5, 1f
    nop
    move    t5, t1
#ifdef LVL_DEBUG
    PRINTSTR("\r\n smallest rddata=")
    move    a0, t5
    bal     hexserial
    nop
#endif
1:
    sb      t1, GATE_POSITION_STORE(t2)
//dataslice control
    daddu   t4, t4, 1
    bltu    t4, t0, ddr3_gate_margin
    nop

#ifdef LVL_DEBUG
    PRINTSTR("\r\nafter gate margin")
    PRINTSTR("\r\n smallest rddata=")
    move    a0, t5
    bal     hexserial
    nop
    PRINTSTR("\r\n0x3300:")
    ld      a0, 0x3300(t8)
    dsrl    a0, 32
    bal     hexserial
    nop
    ld      a0, 0x3300(t8)
    bal     hexserial
    nop
    PRINTSTR("\r\n0x3308:")
    ld      a0, 0x3308(t8)
    dsrl    a0, 32
    bal     hexserial
    nop
    ld      a0, 0x3308(t8)
    bal     hexserial
    nop
#endif
//set finally dly_2x, rddata
    sb      t5, TPHY_RDDATA_OFFSET(t8)
    dli     t4, 0
1:
    dsll    t2, t4, 1
    daddu   t2, t2, t8
    daddu   t2, t2, 1
    lb      t1, GATE_POSITION_STORE(t2)
    dsubu   t1, t1, t5

    dsll    t2, t4, 7   //t4*0x80
    daddu   t2, t2, t8
    lb      t3, DLY_2X_OFFSET(t2)
    and     t3, 0xc3
    dsll    t6, t1, 2
    or      t1, t1, t6
    dsll    t1, t1, 2
    and     t1, 0x3c
    or      t3, t1
    sb      t3, DLY_2X_OFFSET(t2)

    daddiu  t4, 1
    bltu    t4, t0, 1b
    nop

    b       gate_leveling_exit_new
    nop

ddr3_gate_dll_add_1:
    dsll    t1, t4, 0x7   //t1 = t4 * 0x80
    dadd    t2, t1, t8
    lb      t1, DLL_GATE_OFFSET(t2)
    and     t1, t1, 0x7f     //dll_gate
    dadd    t1, t1, 1
    and     t1, t1, 0x7f     //dll_gate
    sb      t1, DLL_GATE_OFFSET(t2)

    bnez    t1, ddr3_gate_lvl_req_set
    nop

    dli     t1, IDLE_TIMES    //wait when dll_wrdq is 0
1:
    daddiu  t1, t1, -0x1
    bnez    t1, 1b
    nop

    dli     t2, 128
//sync_dll_code_1
1:
//set lvl req
    li      t1, 0
    sb      t1, LVL_REQ_OFFSET(t8)
    li      t1, 1
    sb      t1, LVL_REQ_OFFSET(t8)
//check lvl done
2:
    lb      t1, LVL_DONE_OFFSET(t8)
    and     t1, t1, 0x1
    beqz    t1, 2b
    nop
    daddiu  t2, t2, -0x1
    bnez    t2, 1b
    nop
//rddata +1
    lb      t1, TPHY_RDDATA_OFFSET(t8)
    daddu   t1, 1
    sb      t1, TPHY_RDDATA_OFFSET(t8)

    b       ddr3_gate_lvl_req_set
    nop

gate_leveling_exit_new:
#ifdef  LVL_DEBUG
    bal     print_dqs
    nop
#endif

    sb      zero, LVL_MODE_OFFSET(t8)
    sb      zero, LVL_REQ_OFFSET(t8)
    WAIT_FOR(20000)

#ifndef DDR3_DIMM
    /* disable read_preamble_training mode*/
    GET_LVL_CS_NUM
    move    mrs_cs, v0
    dsll    t1, v0, 4
    daddu   t1, t8
    lh      mrs_cmd_a, DDR4_MR4_CS0_REG(t1)
    and     mrs_cmd_a, ~(1<<10)
    li      mrs_num, 4
    MRS_SEND(mrs_cmd_a,mrs_cs,mrs_num)

    /*disable MPR mode to side B for rdimm*/
    GET_LVL_CS_NUM
    move    mrs_cs, v0
    dsll    t1, v0, 4
    daddu   t1, t8
    lh      mrs_cmd_a, DDR4_MR3_CS0_REG(t1)
    and     mrs_cmd_a, ~(1<<2)
    li      mrs_num, 3
    MRS_SEND(mrs_cmd_a,mrs_cs,mrs_num)
#endif

    move    ra, t9
    jr      ra
    nop

    .end    ddr4_leveling

LEAF(table_check_ds)
    dli     a0, 0x0
    bne     v1, a0, 1f
    nop
    dli     v0, 0x8
    b       inquired
    nop
1:
    dli     a0, 0x4
    bgtu    v1, a0, 2f
    nop
    dsubu   v0, a0, v1
    b       inquired
    nop
2:
    sub     v0, v1, 1

inquired:
    jr      ra
    nop
END(table_check_ds)
LEAF(print_dqs)
/**********************************
reg used:
t0: ra
t1: print_reg
t2: slice number
0x3300-0x3308: DLL_GATE_STRORE
0x3310-0x3318: DLY_2X_STORE
0x3319: RDDATA_STORE
**********************************/
//#define DISABLE_PRIT_DQS
//scan read dqs
    move    t0, ra
    sd      $0, DLL_GATE_STORE(t8)
    sd      $0, DLY_2X_STORE(t8)
    sd      $0, (DLL_GATE_STORE+8)(t8)
    sd      $0, (DLY_2X_STORE+8)(t8)
    sb      $0, RDDATA_STORE(t8)
    move    a1, t8
    move    a2, t8
    dli     a3, 9
1:
    lb      a0, DLL_GATE_OFFSET(a1)
    sb      a0, DLL_GATE_STORE(a2)
    lb      a0, DLY_2X_OFFSET(a1)
    sb      a0, DLY_2X_STORE(a2)
    daddu   a1, 0x80
    daddu   a2, 0x1
    dsubu   a3, 1
    bnez    a3, 1b
    nop

    lb      a0, TPHY_RDDATA_OFFSET(t8)
    sb      a0, RDDATA_STORE(t8)
#ifndef DISABLE_PRIT_DQS
    PRINTSTR("\r\ntRDDATA = ")
    lb      a0, TPHY_RDDATA_OFFSET(t8)
    sb      a0, RDDATA_STORE(t8)
    bal     hexserial
    nop
    PRINTSTR("\r\n")
#endif

    dli     a2, 0x0  //ds num
1:
    dsll    a1, a2, 7
    daddu   a1, t8
    lb      a0, DLY_2X_OFFSET(a1)
    andi    a0, a0, 0x3
    sb      a0, DLY_2X_OFFSET(a1)
    lb      a0, DLL_GATE_OFFSET(a1)
    andi    a0, a0, 0x80
    sb      a0, DLL_GATE_OFFSET(a1)
    daddiu  a2, a2, 0x1
    bleu    a2, 8, 1b
    nop

    dli      t2, 0x0    //ds num
nxt_ds:

#ifndef DISABLE_PRIT_DQS
    PRINTSTR("\r\nds num ")
    move     a0, t2
    bal      hexserial
    nop
    PRINTSTR(":      dly_2x=")
    daddu   a0, t2, t8
    lb      a0, DLY_2X_STORE(a0)
    bal     hexserial
    nop
    PRINTSTR("      dll_gate=")
    daddu   a0, t2, t8
    lb      a0, DLL_GATE_STORE(a0)
    bal     hexserial
    nop
#endif

#ifndef DDR3_DIMM
    dli     v0, 0x9   //init read delay
#else
    dli     v0, 0x2
#endif
    sb      v0, TPHY_RDDATA_OFFSET(t8)
nxt_cycle:
    dsll    a0, t2, 7
    daddu   a0, t8
    sb      $0, DLL_GATE_OFFSET(a0)

#ifndef DISABLE_PRIT_DQS
    PRINTSTR("\r\nrddata is ")
    lb      a0, TPHY_RDDATA_OFFSET(t8)
    bal     hexserial
    nop
    PRINTSTR(":")
#endif

    dli     t1, 0x0    //scan vector
next_req:
    li      v0, 1
    sb      v0, LVL_REQ_OFFSET(t8)

/** 12. check whether gate leveling req done **/
1:
    lb      v0, LVL_DONE_OFFSET(t8)
    beqz    v0, 1b
    nop

    /* print gate lvl serial */
    daddu   v0, t2, t8
    lb      v0, LVL_RESP_OFFSET(v0)
    andi    v0, 0x1
    dsll    a0, t2, 7
    daddu   a0, t8
    lb      v1, DLL_GATE_OFFSET(a0)
    bgeu    v1, 0x40, 1f
    nop
    dli     a0, 0x3f
    b       2f
    nop
1:
    dli     a0, 0x7f
2:
    dsubu   v1, a0, v1
    dsll    v0, v0, v1
    or      t1, v0, t1
    bnez    v1, 1f
    nop
#ifndef DISABLE_PRIT_DQS
    dsrl    a0, t1, 32
    bal     hexserial
    nop
    move    a0, t1
    bal     hexserial
    nop
#endif
    dli     t1, 0
1:
    dsll    a0, t2, 7
    daddu   a0, t8
    lb      v0, DLL_GATE_OFFSET(a0)
    daddiu  v0, v0, 0x1
    andi    v0, v0, 0x7f
    sb      v0, DLL_GATE_OFFSET(a0)
    bnez    v0, next_req
    nop
    lb      v0, TPHY_RDDATA_OFFSET(t8)
    daddiu  v0, v0, 0x1
    sb      v0, TPHY_RDDATA_OFFSET(t8)
    bne     v0, 0x1b, nxt_cycle
    nop
    daddu   t2, 1
    bleu    t2, 8, nxt_ds
    nop

end_scan_dqs:
#ifndef DISABLE_PRIT_DQS
    PRINTSTR("\r\nscan test over")
#endif

    move    a1, t8
    move    a2, t8
    dli     a3, 9
1:
    lb      a0, DLL_GATE_STORE(a2)
    sb      a0, DLL_GATE_OFFSET(a1)
    lb      a0, DLY_2X_STORE(a2)
    sb      a0, DLY_2X_OFFSET(a1)
    daddu   a1, 0x80
    daddu   a2, 0x1
    dsubu   a3, 1
    bnez    a3, 1b
    nop

    lb      v0, RDDATA_STORE(t8)
    sb      v0, TPHY_RDDATA_OFFSET(t8)
    sd      $0, DLL_GATE_STORE(t8)
    sd      $0, DLY_2X_STORE(t8)
    sd      $0, (DLL_GATE_STORE+8)(t8)
    sd      $0, (DLY_2X_STORE+8)(t8)
    sb      $0, RDDATA_STORE(t8)

    move    ra, t0
    jr      ra
    nop
END(print_dqs)
LEAF(wrlvl_print)
/***************************
reg used: a0, a1, a2, v0, v1, t8
a2: [ 7: 0] dll value
    [15: 8] dataslice
    [23:16] DQ/DQS select 0-DQ 1-DQS
a3: ra store
v0, v1, a0, a1 elastic regs
do not change t8
***************************/
#define WRLVL_PRINT_DATASLICE_NUM   8
#define WRLVL_PRINT_DLL_LEVEL_NUM   256
#define WRLVL_PRINT_DQS_ADD
    move    a3, ra

    /* init dll value of DQ/DQS */
    dli     a0, 0
1:
    dsll    a1, a0, 7
    daddu   a1, t8
    sd      zero, DDR4_DLL_WRDQ_OFFSET(a1)
    daddu   a0, 1
    bltu    a0, WRLVL_PRINT_DATASLICE_NUM, 1b
    nop

    /* disable wrlvl mode */
    dli     v0, 1
    sb      v0, LVL_MODE_OFFSET(t8)
1:
    lb      v0, LVL_RDY_OFFSET(t8)
    beqz    v0, 1b
    nop

    /* print dll level */
    dli     a2, 100
1:
    beq     a2, 1, 2f
    nop
    DDR_TTYSTRING("\r\n       ")
    b       3f
    nop
2:
    DDR_TTYSTRING("\r\n dll : ")
3:
    dli     a1, 0
4:
    ddivu   a0, a1, a2
    dremu   a0, 10
    DDR_TTYBIT
    daddu   a1, 1
    bltu    a1, WRLVL_PRINT_DLL_LEVEL_NUM, 4b
    nop
    ddivu   a2, 10
    bnez    a2, 1b
    nop
    DDR_TTYSTRING("\r\n")

    /* print wrlvl result loop */
    dli     a2, 0
11:
    DDR_TTYSTRING("\r\nDS ")
    GET_REG_B(a2,1)
    move    a0, v0
    DDR_TTYBIT
    DDR_TTYSTRING(" : ")

    dli     v0, 0x80
    STORE_REG_B(a2,0,v0)
10:
    GET_REG_B(a2,1)
    dsll    v1, v0, 7
    daddu   v1, t8
    GET_REG_B(a2,0)
    and     a1, v0, 0xff
    move    a3, ra
#ifdef WRLVL_PRINT_DQS_ADD
    GET_REG_B(a2,2)
    bnez    v0, 1f
    nop
#endif
    sb      a1, DDR4_DLL_WRDQ_OFFSET(v1)
    sb      a1, DLL_1XDLY_OFFSET(v1)
#ifdef WRLVL_PRINT_DQS_ADD
    b       2f
    nop
1:
    sb      a1, DLL_WRDQS_OFFSET(v1)
2:
#endif
    WAIT_FOR(0x3000)
    dli     v0, 1
    sb      v0, LVL_REQ_OFFSET(t8)

1:
    lb      v0, LVL_DONE_OFFSET(t8)
    beqz    v0, 1b
    nop

    GET_REG_B(a2,1)
    daddu   v1, v0, t8
    lb      a0, LVL_RESP_OFFSET(v1)
    and     a0, 1
    DDR_TTYBIT

    /* dll loop ctrl */
    GET_REG_B(a2,0)
    daddu   v0, 1
    STORE_REG_B(a2,0,v0)
    GET_REG_B(a2,0)
    bnez    v0, 10b
    nop

    /* DQ/DQS loop ctrl */
#ifdef WRLVL_PRINT_DQS_ADD
    GET_REG_B(a2,2)
    daddu   v0, 1
    STORE_REG_B(a2,2,v0)
    dli     v0, 0x80
    STORE_REG_B(a2,0,v0)
    GET_REG_B(a2,2)
    bleu    v0, 1, 10b
    nop
#endif

    /* slice loop ctrl */
    GET_REG_B(a2,1)
    daddu   v0, 1
    STORE_REG_B(a2,1,v0)
#ifdef WRLVL_PRINT_DQS_ADD
    dli     v0, 0
    STORE_REG_B(a2,2,v0)
#endif
    GET_REG_B(a2,1)
    bltu    v0, WRLVL_PRINT_DATASLICE_NUM, 11b
    nop

    /* disable wrlvl mode */
    dli     v0, 0
    sb      v0, LVL_MODE_OFFSET(t8)
    sb      v0, LVL_REQ_OFFSET(t8)

    move    ra, a3
    jr      ra
    nop
END(wrlvl_print)
