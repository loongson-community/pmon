#define DDR_VREF_TRAIN_SLICE_NUM    0x3300
#define VREF_TRAINING_RANGE_FLAG    0x3301
#define PAGE_SIZE   0x0
#define PAGE_NUMBER 0x1
#define MAX_RTT_PARK_ONE_NO         0x3302
#define MAX_RTT_PARK_ODT            0x3308
#define ODT_AVALIBLE                0x3303
#define RTT_PARK_FOUND_FLAG         0x3304
#define CS_COVER          0x3310
#define CS_NUM            0x3311

#define DDR_VREF_DEBUG
//#define RTT_PRAK_TRAINING
    .text
    .set    noreorder
    .set    mips3

    .global ddr_vref_training
    .ent    ddr_vref_training

ddr_vref_training:
    move    t9, ra
#ifdef  DDR_VREF_DEBUG
    PRINTSTR("\r\n DDR vref training start!!!\r\n")
#endif
    GET_NODE_ID_a0
    dli     t8, 0x900000000ff00000
    or      t8, a0

    GET_CS_NUM_DDR4
    sb      v0, CS_NUM(t8)

#ifdef  RTT_PRAK_TRAINING
    /* init  rtt_park */
    dli     t1, 0
1:
    dsll    t3, t1, 4
    daddu   t3, t8
    dli     a0, 0x1
    bal     true_rtt_park_2_rtt_park_mrs
    nop
    li      mrs_num, 5
    lhu     mrs_cmd_a, DDR4_MR5_CS0_REG(t3)
    and     mrs_cmd_a, ~(7<<6)
    dsll    v0, 6
    or      mrs_cmd_a, v0
    sh      mrs_cmd_a, DDR4_MR5_CS0_REG(t3)
    move    mrs_cs, t1
    MRS_SEND(mrs_cmd_a,mrs_cs,mrs_num)
    daddu   t1, 1
    GET_CS_NUM_DDR4
    bltu    t1, v0, 1b
    nop

    dli     t1, 0
    sb      t1, MAX_RTT_PARK_ONE_NO(t8)
    sd      t1, MAX_RTT_PARK_ODT(t8)
    sb      t1, ODT_AVALIBLE(t8)
    sb      t1, RTT_PARK_FOUND_FLAG(t8)

rtt_park_loop:
#ifdef  DDR_VREF_DEBUG
    PRINTSTR("\r\n RTT_PARK=")
    lhu     a0, DDR4_MR5_REG(t8)
    srl     a0, 6
    and     a0, 0x7
    bal     rtt_park_mrs_2_true_rtt_park
    nop
    move    a0, v0
    bal     hexserial
    nop
#endif

#endif

    dli     t0, 1
    sb      t0, CS_COVER(t8)

ddr_vref_trainig_cs_loop:

    dli     t3, 0x2
    sb      t3, VREF_TRAINING_RANGE_FLAG(t8)     //vref training range select flag

ddr_vref_range_adjust:

    /*enable vref training mode*/
    li      mrs_num, 6
    lb      mrs_cs, CS_COVER(t8)
    subu    mrs_cs, 1
    dsll    t1, mrs_cs, 4
    daddu   t1, t8
    lhu     mrs_cmd_a, DDR4_MR6_CS0_REG(t1)
    and     mrs_cmd_a, ~(0xc0)
    lb      t3, VREF_TRAINING_RANGE_FLAG(t8)
    dsll    t3, 6
    or      mrs_cmd_a, t3       //select vref training range by t3 and start training
    sh      mrs_cmd_a, DDR4_MR6_CS0_REG(t1)
    MRS_SEND(mrs_cmd_a,mrs_cs,mrs_num)

//set Vref Value
    dli     t4, 0   //vref value
    dli     s5, 0   //effectvie vref value flag
ddr_vref_loop:

    /*set ddr vref value stored in t4*/
    li      mrs_num, 6
    lb      mrs_cs, CS_COVER(t8)
    subu    mrs_cs, 1
    dsll    t1, mrs_cs, 4
    daddu   t1, t8
    lhu     mrs_cmd_a, DDR4_MR6_CS0_REG(t1)
    and     mrs_cmd_a, ~(0x3f)
    or      mrs_cmd_a, t4
    MRS_SEND(mrs_cmd_a,mrs_cs,mrs_num)


//write and read, then compare
#if 0//def  DDR_VREF_DEBUG
    PRINTSTR("\r\nBASE addres is ")
#endif
    GET_DIMM_MEMSIZE_V1
    lb      t1, CS_NUM(t8)
    dsrl    t1, t1, 1
    dsrl    a1, a1, t1  //single cs memsize
    lb      t0, CS_COVER(t8)
    subu    t0, t0, 1
    dmulo   t0, a1, t0
    dsll    t0, t0, 30
    dsrl    t0, t0, 3   //test engin single size for each cs
    sd      t0, BASE_ADDR_OFFSET(t8)
#if 0//def  DDR_VREF_DEBUG
    dsrl    a0, t0, 32
    bal     hexserial
    nop
    move    a0, t0
    bal     hexserial
    nop
#endif
//set valid compare bits
    dli     t0, 0
    not     t1, t0
    sd      t1, VALID_BITS_OFFSET(t8)
    lb      t2, ECC_ENABLE_BIT(t8)
    and     t2, 0x1
    beqz    t2, 1f
    nop
    dli     t0, 0xff
1:
    sb      t0, VALID_BITS_ECC_OFFSET(t8)
    dli     t0, PAGE_SIZE     //page size
    sb      t0, PAGE_SIZE_OFFSET(t8)
    dli     t0, PAGE_NUMBER     //page number
    sw      t0, PAGE_NUM_OFFSET(t8)
    bal     test_engine
    nop

    dli     t1, 0x1
    dsll    t1, t4
    beqz    v0, 1f
    nop
//test success, set vref value flag 1 at its position
    or      s5, t1
    b       ddr_vref_loop_ctrl
    nop
//test failed, set vref value flag 0 at its position
1:
    not     t1
    and     s5, t1
ddr_vref_loop_ctrl:
//vref value start from 0 to 50
    daddu   t4, 1
    bleu    t4, 50, ddr_vref_loop
    nop
#ifdef  DDR_VREF_DEBUG
    PRINTSTR("\r\n DDR Vref in Range ")
    lb      t3, VREF_TRAINING_RANGE_FLAG(t8)
    dsubu   a0, t3, 1
    bal     hexserial
    nop
    PRINTSTR(" is: ")
    dsrl    a0, s5, 32
    bal     hexserial
    nop
    move    a0, s5
    bal     hexserial
    nop
#endif
//if s5=0, the corresponding vref range is not appropriate
//else set the vref value by mrs
#ifdef  RTT_PRAK_TRAINING
    bnez    s5, rtt_park_loop_ctrl
    nop
#else
    bnez    s5, ddr_vref_set
    nop
#endif
    dli     t7, 0
    b       exit_ddr_vref_training
    nop
//if range 1 is failed, restart training in range 2
vref_range_select:
    /*disable vref training mode*/
    li      mrs_num, 6
    lb      mrs_cs, CS_COVER(t8)
    subu    mrs_cs, 1
    dsll    t1, mrs_cs, 4
    daddu   t1, t8
    lhu     mrs_cmd_a, DDR4_MR6_CS0_REG(t1)
    and     mrs_cmd_a, ~(0xc0)
    MRS_SEND(mrs_cmd_a,mrs_cs,mrs_num)

    lb      t3, VREF_TRAINING_RANGE_FLAG(t8)
    daddu   t3, 1
    sb      t3, VREF_TRAINING_RANGE_FLAG(t8)
    beq     t3, 0x3, ddr_vref_range_adjust
    nop
#ifdef  RTT_PRAK_TRAINING
rtt_park_loop_ctrl:
    lb      t1, RTT_PARK_FOUND_FLAG(t8)
    bnez    t1, ddr_vref_set
    nop
    dli     t0, 0
    dli     t1, 0   //number of 1 in s5
1:
    dsrl    t2, s5, t0
    and     t2, 0x1
    daddu   t1, t2
    daddu   t0, 1
    bleu    t0, 51, 1b
    nop


    lhu     a0, DDR4_MR5_REG(t8)
    dsrl    a0, 6
    and     a0, 0x7             //a0 rtt_park value
    bal     rtt_park_mrs_2_true_rtt_park
    nop

    lb      t0, MAX_RTT_PARK_ONE_NO(t8)
    bgtu    t0, t1, 1f
    nop

    beq     t0, t1, 2f
    nop
    sb      t1, MAX_RTT_PARK_ONE_NO(t8)
    sd      $0, MAX_RTT_PARK_ODT(t8)    //clear stored rtt_park value
    sb      v0, MAX_RTT_PARK_ODT(t8)
    sb      $0, ODT_AVALIBLE(t8)
    b       1f
    nop
2:
    lb      t0, ODT_AVALIBLE(t8)
    daddu   t0, 1
    sb      t0, ODT_AVALIBLE(t8)
    ld      t1, MAX_RTT_PARK_ODT(t8)
    dsll    t2, t0, 0x3
    dsll    t3, v0, t2
    or      t1, t3
    sd      t1, MAX_RTT_PARK_ODT(t8)
1:
    daddu   v0, 1
    move    t0, v0

    /* set rtt_park */
    dli     t1, 0
1:
    dsll    t3, t1, 4
    daddu   t3, t8
    move    a0, t0
    and     a0, 0x7
    bal     true_rtt_park_2_rtt_park_mrs
    nop
    li      mrs_num, 5
    lhu     mrs_cmd_a, DDR4_MR5_CS0_REG(t3)
    and     mrs_cmd_a, ~(7<<6)
    dsll    v0, 6
    or      mrs_cmd_a, v0
    sh      mrs_cmd_a, DDR4_MR5_CS0_REG(t3)
    move    mrs_cs, t1
    MRS_SEND(mrs_cmd_a,mrs_cs,mrs_num)
    daddu   t1, 1
    GET_CS_NUM_DDR4
    bltu    t1, v0, 1b
    nop

    bleu    t0, 7, rtt_park_loop
    nop

    lb      t1, MAX_RTT_PARK_ONE_NO(t8)
    bnez    t1, rtt_park_set
    nop
#endif
//ddr vref training fail and set the default value
    PRINTSTR("\r\n DDR Vref training fail!!!")
    li      mrs_num, 6
    lb      mrs_cs, CS_COVER(t8)
    subu    mrs_cs, 1
    dsll    t1, mrs_cs, 4
    daddu   t1, t8
    lhu     mrs_cmd_a, DDR4_MR6_CS0_REG(t1)
    and     mrs_cmd_a, ~(0xc0)
    or      mrs_cmd_a, 0x80
    sh      mrs_cmd_a, DDR4_MR6_CS0_REG(t1)
    MRS_SEND(mrs_cmd_a,mrs_cs,mrs_num)

    dli     t4, 0x15
    b       ddr_vref_write
    nop
#ifdef  RTT_PRAK_TRAINING
rtt_park_set:
#ifdef  DDR_VREF_DEBUG
    PRINTSTR("\r\n Final result:")
#endif
    lb      t0, ODT_AVALIBLE(t8)
    ld      a0, MAX_RTT_PARK_ODT(t8)
    beqz    t0, 1f
    nop
    srl     t0, 1
    sll     t0, 3
    dsrl    a0, t0
    and     a0, 0xff
1:
    bal     true_rtt_park_2_rtt_park_mrs
    nop
    dsll    t2, v0, 6

    dli     t1, 0
1:
    dsll    t3, t1, 4
    daddu   t3, t8
    li      mrs_num, 5
    lhu     mrs_cmd_a, DDR4_MR5_CS0_REG(t3)
    and     mrs_cmd_a, ~(7<<6)
    or      mrs_cmd_a, t2
    sh      mrs_cmd_a, DDR4_MR5_CS0_REG(t3)
    move    mrs_cs, t1
    MRS_SEND(mrs_cmd_a,mrs_cs,mrs_num)
    daddu   t1, 1
    GET_CS_NUM_DDR4
    bltu    t1, v0, 1b
    nop

    li      t0, 1
    sb      t0, RTT_PARK_FOUND_FLAG(t8)
    b       rtt_park_loop
    nop
#endif


//define the vref value as the middle of the avalible values in s5
ddr_vref_set:
    dli     t2, 0
    dli     t0, 0xffff
    dli     t4, 35
//look for the start bit of continues 1
1:
    dsll    t3, t0, t2
    and     t1, s5, t3
    beq     t1, t3, 2f
    nop
    daddu   t2, 1
    bleu    t2, t4, 1b
    nop
    dsrl    t0, 1
    daddu   t4, 1
    dli     t1, 51
    dsubu   t1, t1, t4
    dli     t2, 0
    bnez    t1, 1b
    nop
2:
//count the number of continues 1
    move    t6, t2
    daddu   t6, 1
3:
    dsll    t3, t0, t6
    and     t1, s5, t3
    bne     t1, t3, 4f
    nop
    daddu   t6, 1
    bleu    t6, t4, 3b
    nop
4:
//set the middle of continues 1 as the vref value
    dsubu   t6, t6, t2
    dli     t1, 51
    dsubu   t1, t1, t4
    daddu   t1, t6
    dsrl    t1, 1
    daddu   t1, t2
    move    t4, t1
//set the vref value by sending mrs6 to sdram
ddr_vref_write:
#ifdef VREF_STORE
    GET_NODE_ID_a1
    mul     a1, DIMM_INFO_SIZE
    dli     t3, DIMM_INFO_IN_CACHE_OFFS
    daddu   t3, t3, a1
    mul     a1, k0, MC_INFO_SIZE
    daddu   t3, t3, a1
    lb      a1, CS_COVER(t8)
    subu    a1, a1, 1
    daddu   t3, t3, a1
    sb      t4, DIMM_OFFS_DDR_VREF(t3)
#endif

    /*set ddr vref value stored in t4*/
    li      mrs_num, 6
    lb      mrs_cs, CS_COVER(t8)
    subu    mrs_cs, 1
    dsll    t1, mrs_cs, 4
    daddu   t1, t8
    lhu     mrs_cmd_a, DDR4_MR6_CS0_REG(t1)
    and     mrs_cmd_a, ~(0x3f)
    or      mrs_cmd_a, t4
    sh      mrs_cmd_a, DDR4_MR6_CS0_REG(t1)
    MRS_SEND(mrs_cmd_a,mrs_cs,mrs_num)

    dli     t7, 0x1
//exit Vref training
exit_ddr_vref_training:
    /*disable vref training mode*/
    li      mrs_num, 6
    lb      mrs_cs, CS_COVER(t8)
    subu    mrs_cs, 1
    dsll    t1, mrs_cs, 4
    daddu   t1, t8
    lhu     mrs_cmd_a, DDR4_MR6_CS0_REG(t1)
    and     mrs_cmd_a, ~(0x80)
    sh      mrs_cmd_a, DDR4_MR6_CS0_REG(t1)
    MRS_SEND(mrs_cmd_a,mrs_cs,mrs_num)

    beqz    t7, vref_range_select
    nop
    nop
    nop
    nop
#ifdef  DDR_VREF_DEBUG
    PRINTSTR("\r\n CS ")
    lb      t0, CS_COVER(t8)
    subu    a0, t0, 1
    bal     hexserial
    nop
    PRINTSTR(" DDR vref is: ")
    move    a0, t4
    bal     hexserial
    nop
    PRINTSTR("\r\n DDR vref training done!!!")
#endif
//cs_loop_ctrl
    lb      t1, CS_NUM(t8)
    lb      t0, CS_COVER(t8)
    daddu   t0, t0, 1
    sb      t0, CS_COVER(t8)
    bleu    t0, t1, ddr_vref_trainig_cs_loop
    nop

    move    ra, t9
    jr      ra
    nop

    .end    ddr_vref_training
LEAF(rtt_park_mrs_2_true_rtt_park)
/*************************
translate rtt_park value in MRS to true rtt_park value
input:      a0--rtt_park value in MRS
output:     v0--true rtt_park value
************************/
    bne     a0, 1, 1f
    nop
    li      v0, 4
    b       2f
    nop
1:
    bne     a0, 2, 1f
    nop
    li      v0, 2
    b       2f
    nop
1:
    bne     a0, 3, 1f
    nop
    li      v0, 6
    b       2f
    nop
1:
    bne     a0, 4, 1f
    nop
    li      v0, 1
    b       2f
    nop
1:
    bne     a0, 5, 1f
    nop
    li      v0, 5
    b       2f
    nop
1:
    bne     a0, 6, 1f
    nop
    li      v0, 3
    b       2f
    nop
1:
    li      v0, 7
2:
    jr      ra
    nop
END(rtt_park_mrs_2_true_rtt_park)
LEAF(true_rtt_park_2_rtt_park_mrs)
    bne     a0, 1, 1f
    nop
    li      v0, 4
    b       2f
    nop
1:
    bne     a0, 2, 1f
    nop
    li      v0, 2
    b       2f
    nop
1:
    bne     a0, 3, 1f
    nop
    li      v0, 6
    b       2f
    nop
1:
    bne     a0, 4, 1f
    nop
    li      v0, 1
    b       2f
    nop
1:
    bne     a0, 5, 1f
    nop
    li      v0, 5
    b       2f
    nop
1:
    bne     a0, 6, 1f
    nop
    li      v0, 3
    b       2f
    nop
1:
    li      v0, 7
2:
    jr      ra
    nop
END(true_rtt_park_2_rtt_park_mrs)
