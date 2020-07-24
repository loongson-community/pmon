
    .text


#define ONE_SAMPLE_L64   0x3300
#define ONE_SAMPLE_H64   0x3308
#define ZERO_SAMPLE_L64  0x3310
#define ZERO_SAMPLE_H64  0x3318
#define SAMPLE0_L64      0x3320
#define SAMPLE0_H64      0x3328
#define SAMPLE1_L64      0x3330
#define SAMPLE1_H64      0x3338
#define SAMPLE2_L64      0x3340
#define SAMPLE2_H64      0x3348
#define SAMPLE3_L64      0x3350
#define SAMPLE3_H64      0x3358
#define SAMPLE4_L64      0x3360
#define SAMPLE4_H64      0x3368
#define REG_COUNT        0x3370
#define REG_SHIFT        0x3371
#define REG_VREF_COUNT   0x3372
#define SLICE_NUMBER     0x3373
#define RDDATA_DEFAULT   0x3374
#define VREF_FLAG        0x3375
#define RL_CURRENT       0x3376
#define RL_FLAG          0x3377
#define RL_1             0x3378
#define RL_2             0x3379
#define RL_3             0x337a
#define RL_4             0x337b
#define RL_5             0x337c
#define FIND_RL_FLAG     0x337d
#define VREF_RESULT_MAX  0x337e
#define VREF_VS_ODT_MAX  0x337f
#define RA_STORE         0x1160
#define RL_ORI           0x1060
#define ODT_LOOP_CTRL     0x8

#define CONT1_NUM 0x28
#define CONT0_NUM 0x28
#define WAIT_NUMBER     0x200

#define PRINT_VREF_TRAINIG_RESULT   \
    ld     s5, 0x1140(conf_base); \
    ld     s6, 0x1148(conf_base); \
	PRINTSTR("\r\nresult:          ");  \
	dsrl   a0, s6, 32;	\
	bal    hexserial;	\
	nop;	\
	dsrl   a0, s6, 0;	\
	bal    hexserial;	\
	nop;	\
	dsrl   a0, s5, 32;	\
	bal    hexserial;	\
	nop;	\
	dsrl   a0, s5, 0;	\
	bal    hexserial;	\
	nop



#define conf_base  t8
#define vref_value t7
#define dll_vref   t6
#define num_slice  t5
#define VREF_TRAINING_DEBUG
//#define PRINT_DLL_SAMPLE
//#define VREF_TRAINING
//#define ODT_DEBUG
#define DISABLE_DQ_ODT_TRAINING

    .global mc_vref_training
    .ent    mc_vref_training
	.set    noreorder
	.set    mips3

mc_vref_training:
    move   t9, ra

    GET_NODE_ID_a0
    dli    conf_base, 0x900000000ff00000
    or     conf_base, a0

	dli	   t0, 0xffffffffffffffff
    sd     t0, 0x1150(conf_base)
    sd     t0, 0x1158(conf_base)
//TODO, use mrs for ddr3 store s5/s6
#ifdef PRINT_DLL_SAMPLE
#define DLL_LOOP_COUNT  0x5
#define BIT_LOOP_CONTROL    0x1
#define VREF_LOOP_COUNT 0x1
#else
#define DLL_LOOP_COUNT  0x5
#define BIT_LOOP_CONTROL    0x8
#define VREF_LOOP_COUNT 0x2
#endif

#ifndef DDR3_DIMM
    /*enable MPR mode to side B for rdimm*/
    GET_LVL_CS_NUM
    move    mrs_cs, v0
    dsll    t1, v0, 4
    daddu   t1, t8
    lh      mrs_cmd_a, DDR4_MR3_CS0_REG(t1)
    or      mrs_cmd_a, (1<<2)
    and     mrs_cmd_a, ~(0x3 | 0x3<<11)
    li      mrs_num, 3
    MRS_SEND(mrs_cmd_a,mrs_cs,mrs_num)
#endif

    //set lvl_mode to gate leveling
    li     t0, 0x2
    sb     t0, LVL_MODE_OFFSET(conf_base)
    WAIT_FOR(20000)

    //wait lvl_ready
1:
    lb     t0, LVL_RDY_OFFSET(conf_base)
    beqz   t0, 1b
    nop

//define the training slice number
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
    dli     t0, 0x8
#ifndef DISABLE_DIMM_ECC
    GET_DIMM_ECC_V1
    beqz    a1, 1f
    nop
    daddu   t0, 1
#endif
1:
    sb      t0, SLICE_NUMBER(conf_base)

    dli     num_slice, 0x0 //loop num_slice
slice_loop:

    /* set mpr0 value for cs 4-8 for rdimm */
    GET_DIMM_TYPE_V1
    andi    a1, a1, 1
    beqz    a1, 4f
    nop
    bltu    num_slice, 4, 4f
    nop
    sb      zero, LVL_MODE_OFFSET(conf_base)
    WAIT_FOR(20000)
    /*enable MPR mode to side B for rdimm*/
    GET_LVL_CS_NUM
    move    mrs_cs, v0
    dsll    t1, v0, 4
    daddu   t1, t8
    lh      mrs_cmd_a, DDR4_MR3_CS0_REG(t1)
    or      mrs_cmd_a, (1<<2)
    and     mrs_cmd_a, ~(0x3 | 0x3<<11)
    li      mrs_num, 3
    MRS_SEND(mrs_cmd_a,mrs_cs,mrs_num)
    GET_LVL_CS_NUM
    move    a0, v0
    beq     num_slice, 8, 2f
    nop
    li      a1, 0xad
    b       3f
    nop
2:
    li      a1, 0x55
3:
    bal     mpr_write
    nop
    //set lvl_mode to gate leveling
    li     t0, 0x2
    sb     t0, LVL_MODE_OFFSET(conf_base)
    WAIT_FOR(20000)

    //wait lvl_ready
1:
    lb     t0, LVL_RDY_OFFSET(conf_base)
    beqz   t0, 1b
    nop
4:
//#ifdef VREF_TRAINING_DEBUG
	PRINTSTR("\r\nslice:")
	move    a0, num_slice
	bal     hexserial
	nop
//#endif
#ifndef DISABLE_DQ_ODT_TRAINING
//init  dataslice DQ ODT value
    dsll    t0, num_slice, 1
    daddu   t0, conf_base
    lhu     t1, DS0_ODT_OFFSET(t0)
    dli     t2, 0x7
    dsll    t2, 9
    not     t2
    and     t1, t2
    dli     t2, 0x1
    dsll    t2, 9
    or      t1, t2
    sh      t1, DS0_ODT_OFFSET(t0)
//init odt loop ctrl value
    dli     t0, 0
    sb      t0, VREF_RESULT_MAX(conf_base)
    sb      t0, VREF_VS_ODT_MAX(conf_base)
odt_loop:
    PRINTSTR(".")
#endif
    bal     vref_train_kernal
    nop
#ifndef DISABLE_DQ_ODT_TRAINING
#ifdef ODT_DEBUG
    PRINTSTR("\r\n MC_DQ_ODT value=")
    dsll    t0, num_slice, 1
    daddu   t0, conf_base
    lhu     t1, DS0_ODT_OFFSET(t0)
    dli     a0, 0x7
    dsll    a0, 9
    and     a0, t1
    dsrl    a0, 9
    bal     hexserial
    nop
    PRINT_VREF_TRAINIG_RESULT
#endif
//use the 3rd bit of VREF_VS_ODT_MAX as odt_loop complete flag
    lb      t0, VREF_VS_ODT_MAX(conf_base)
    xor     t0, t0, 0x8
    beqz    t0, odt_loop_end
    nop
//calculate the number of 1 in vref training result
    dli     t4, 0
    dli     t1, 0
    dli     t2, 0x1
1:
    ld      t0, 0x1140(conf_base)
    and     t3, t0, t2
    dsrl    t3, t1
    daddu   t4, t3
    ld      t0, 0x1148(conf_base)
    and     t3, t0, t2
    dsrl    t3, t1
    daddu   t4, t3
    daddu   t1, 1
    dsll    t2, 1
    bleu    t1, 63, 1b
    nop
#ifdef ODT_DEBUG
    PRINTSTR("\r\n the number of 1 is")
    move    a0, t4
    bal     hexserial
    nop
#endif
//compare the number of 1 with the max
//get current ODT value to t2
    dsll    t0, num_slice, 1
    daddu   t0, conf_base
    lhu     t1, DS0_ODT_OFFSET(t0)
    dli     t2, 0x7
    dsll    t2, 9
    and     t2, t1
    dsrl    t2, 9

    lb      t3, VREF_RESULT_MAX(conf_base)

    bleu    t4, t3, 1f
    nop
    sb      t4, VREF_RESULT_MAX(conf_base)
    sb      t2, VREF_VS_ODT_MAX(conf_base)
1:
    daddu   t2, 1
    move    t3, t2
    dsll    t2, 9
    dli     t4, 0x7
    dsll    t4, 9
    and     t2, t4
    not     t4
    and     t1, t4
    or      t1, t2
    sh      t1, DS0_ODT_OFFSET(t0)
    dsrl    t2, 9
    bltu    t3, ODT_LOOP_CTRL, odt_loop
    nop
//use the ODT value with max number of 1 in vref training result
    dsll    t0, num_slice, 1
    daddu   t0, conf_base
    lhu     t1, DS0_ODT_OFFSET(t0)
    dli     t2, 0x7
    dsll    t2, 9
    not     t2
    and     t1, t2
    lb      t2, VREF_VS_ODT_MAX(conf_base)
    andi    t2, 0x7
    dsll    t2, 9
    or      t1, t2
    sh      t1, DS0_ODT_OFFSET(t0)
    dli     t0, 0x8
    PRINTSTR("\r\nfinal MC_DQ_ODT is")
    lb      a0, VREF_VS_ODT_MAX(conf_base)
    bal     hexserial
    nop
    sb      t0, VREF_VS_ODT_MAX(conf_base)
    b       odt_loop
    nop
odt_loop_end:
#endif


//get the common vref value of all slices
    ld	t0, 0x1140(conf_base)
    ld	t1, 0x1150(conf_base)
    and	t1, t0
    sd	t1, 0x1150(conf_base)
    ld	t0, 0x1148(conf_base)
    ld	t1, 0x1158(conf_base)
    and	t1, t0
    sd	t1, 0x1158(conf_base)

vref_set_all:
#ifdef VREF_TRAINING_DEBUG
//print
    lb     t1, 0x0(conf_base)
    beq	   t1, 0x11, 1f
    nop

    lbu    t1, SLICE_NUMBER(conf_base)
    bltu   num_slice, t1, 1f
    nop
    PRINTSTR("\r\nfinal")
    ld     s5, 0x1150(conf_base)
    ld     s6, 0x1158(conf_base)
    b      2f
    nop
1:
    ld     s5, 0x1140(conf_base)
    ld     s6, 0x1148(conf_base)
2:
	PRINTSTR("\r\nresult:          ")
	dsrl   a0, s6, 32
	bal    hexserial
	nop

	dsrl   a0, s6, 0
	bal    hexserial
	nop

	dsrl   a0, s5, 32
	bal    hexserial
	nop

	dsrl   a0, s5, 0
	bal    hexserial
	nop

	PRINTSTR("\r\n")
#endif
	dli	    t0, 0xffff
    dli     t3, 111
vref_set:
    daddu   t3, 1
	dli	    t6, 0
    lb     t1, 0x0(conf_base)
    beq	   t1, 0x11, 2f
    nop
    lbu     t1, SLICE_NUMBER(conf_base)
    bltu    num_slice, t1, 2f
    nop
    ld      s5, 0x1150(conf_base)
    ld      s6, 0x1158(conf_base)
    b       1f
    nop
2:
    ld      s5, 0x1140(conf_base)
    ld      s6, 0x1148(conf_base)
1:
	and	t4, s5, t0
	beq	t4, t0, 1f
	nop
	and	t1, s5, 0x1
	dsrl	s5, 1

	and	t2, s6, 0x1
	dsrl	s6, 1
	daddu	t6, 1

	dsll	t1, 63
	dsll	t2, 63
	or	s5, t2
	or	s6, t1
	bleu	t6, t3, 1b
	nop
    dsrl    t0, 1
    bnez    t0, vref_set
#ifdef VREF_TRAINING_DEBUG
	PRINTSTR("Vref training failing!!!  Vref_value set as default value\r\n")
#endif
    dli vref_value, VREF
    dsrl    vref_value, 5
    b   vref_write
	nop
1:
    dli t7, 0
    dsubu   t6, t3, t6
2:
	and	t4, s5, t0
	bne	t4, t0, 1f
	nop

	and	t1, s5, 0x1
	dsrl	s5, 1

	and	t2, s6, 0x1
	dsrl	s6, 1

	dsll	t1, 63
	dsll	t2, 63
	or	s5, t2
	or	s6, t1
    dli     t1, 0x1
	daddu	t7, t7, t1

	bleu	t7, t6, 2b

1:
//sync
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop

    dli     t1, 127
    dsubu   t1, t1, t3
    daddu   t7, t7, t1
    dsrl    t7, 1
    dsubu   t6, t3, t6
    daddu   t7, t7, t6
    daddu   t7, t7, 1

#ifdef VREF_TRAINING_DEBUG
	PRINTSTR("Vref_value is :")
	dsrl	a0, vref_value, 32
	bal	hexserial
	nop
	move	a0, vref_value
	bal	hexserial
	nop
    PRINTSTR("\r\n")
#endif

vref_write:
    lb     t1, 0x0(conf_base)
    beq	   t1, 0x10, 2f
    nop

    dsll   t4, num_slice, 1
    daddu  t4, conf_base
    lhu    t1, 0x810(t4)
    li     t0, (0x7f<<5)
    not    t0, t0
    and    t1, t0
    ori    t1, 0x1
    dsll   t0, vref_value, 5
    or     t1, t0
    sh     t1, 0x810(t4)

#ifdef VREF_STORE
    GET_NODE_ID_a1
    mul     a1, DIMM_INFO_SIZE
    dli     t3, DIMM_INFO_IN_CACHE_OFFS
    daddu   t3, a1
    mul     a1, k0, MC_INFO_SIZE
    daddu   t3, a1
    daddu   t3, num_slice
    sb      vref_value, DIMM_OFFS_VREF(t3)
    b	   1f
    nop
#endif
2:

    lbu    t1, SLICE_NUMBER(conf_base)
    bltu   num_slice, t1, 1f
    nop
    b      vref_write_all_slice
    nop
1:

next_slice:

    addiu  num_slice, 0x1
    lbu    t1, SLICE_NUMBER(conf_base)//max num_slice
    bne    num_slice, t1, slice_loop
    nop
    lb     t1, 0x0(conf_base)
    beq	   t1, 0x11, 11f
    nop

    b      vref_set_all
    nop
vref_write_all_slice:
    move   t3, $0
    GET_NODE_ID_a0
    dli    t0, 0x900000003ff00180
    or     t0, a0
    lwu    t1, 0x0(t0)
    dsll   t3, t1, 32//store origin value

    li     t2, ((0x1<<4)|(0x1<<9))
    not    t2, t2
    and    t1, t2
    li     t2, ((0x1<<5)|(0x1<<10))
    or     t1, t2
    sw     t1, 0x0(t0)

    GET_NODE_ID_a0
    dli    t0, 0x900000003ff00400
    or     t0, a0
    lbu    t1, 0x3(t0)
    or     t3, t1 //store mc_en
    lbu    t1, 0x4(t0)
    dsll   t1, 0x8
    or     t3, t1 //store interleave_bit/en
    lwu    t1, 0x0(t0)
    li     t2, (0x3<<30)
    or     t1, t2
    sw     t1, 0x0(t0)

    dli    t4, 0x900004000ff00000
    lhu    t1, 0x810(t4)
    li     t0, (0x7f<<5)
    not    t0, t0
    and    t1, t0
    ori    t1, 0x1
    dsll   t0, vref_value, 5
    or     t1, t0

    sh     t1, 0x810(t4)
    sh     t1, 0x812(t4)
    sh     t1, 0x814(t4)
    sh     t1, 0x816(t4)
    sh     t1, 0x818(t4)
    sh     t1, 0x81a(t4)
    sh     t1, 0x81c(t4)
    sh     t1, 0x81e(t4)
    sh     t1, 0x820(t4)

    dli    t4, 0x900005000ff00000
    lhu    t1, 0x810(t4)
    li     t0, (0x7f<<5)
    not    t0, t0
    and    t1, t0
    ori    t1, 0x1
    dsll   t0, vref_value, 5
    or     t1, t0
    sh     t1, 0x810(t4)
    sh     t1, 0x812(t4)
    sh     t1, 0x814(t4)
    sh     t1, 0x816(t4)
    sh     t1, 0x818(t4)
    sh     t1, 0x81a(t4)
    sh     t1, 0x81c(t4)
    sh     t1, 0x81e(t4)
    sh     t1, 0x820(t4)

    GET_NODE_ID_a0
    dli    t0, 0x900000003ff00400
    or     t0, a0
    sb     t3, 0x3(t0)
    dsrl   t3, t3, 8
    sb     t3, 0x4(t0)
    GET_NODE_ID_a0
    dli    t0, 0x900000003ff00180
    or     t0, a0
    dsrl   t3, t3, 24
    sw     t3, 0x0(t0)
11:
    //exit lvl_mode
    li     t0, 0x0
    sb     t0, LVL_MODE_OFFSET(conf_base)
    WAIT_FOR(20000)
#ifndef DDR3_DIMM
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
    PRINTSTR("\r\n Vref training done!\r\n")

    move   ra, t9
    jr     ra
    nop

    .end mc_vref_training

LEAF(rl_mrs_send)
/*input: a0-RL*/
    move        a3, ra

    bleu        a0, 16, 1f
    nop
    beq         a0, 17, 2f
    nop
    beq         a0, 19, 3f
    nop
    beq         a0, 21, 4f
    nop
    beq         a0, 23, 5f
    nop
    bleu        a0, 24, 6f
    nop
    dsubu       a0, 25
    daddu       a0, 0x10
    b           7f
    nop
1:
    dsubu       a0, 9
    b           7f
    nop
2:
    dli         a0, 0xd
    b           7f
    nop
3:
    dli         a0, 0xe
    b           7f
    nop
4:
    dli         a0, 0xf
    b           7f
    nop
5:
    dli         a0, 0xc
    b           7f
    nop
6:
    dsubu       a0, 18
    dsrl        a0, 1
    daddu       a0, 8
7:
    andi        v0, a0, 0x1
    dsll        v0, 2
    andi        v1, a0, (0x7<<1)
    dsll        v1, 3
    or          v0, v1
    andi        v1, a0, (0x1<<4)
    dsll        v1, 8
    or          v0, v1
    lhu         mrs_cmd_a, DDR4_MR0_CS0_REG(t8)
    and         mrs_cmd_a, ~(0x1<<2|0x7<<4|0x1<<12)
    or          mrs_cmd_a, v0

    GET_LVL_CS_NUM
    move        mrs_cs, v0
    li          mrs_num, 0
    MRS_SEND(mrs_cmd_a,mrs_cs,mrs_num)

    move    ra, a3
    jr      ra
    nop
END(rl_mrs_send)

LEAF(vref_train_kernal)
/***************************************************************
output: vref training resutl stored in 0x1140(LSB) and 0x1148(HSB). Each bit represents the vref value , 1 means pass and 0 means fail.
***************************************************************/
    sd      ra, RA_STORE(conf_base)
//set vref_dly default value
    dli     t0, 0x5
    sb      t0, VREF_DLY_OFFSET(conf_base)
//set default value
    lb      t0, TPHY_RDDATA_OFFSET(conf_base)
    sb      t0, RDDATA_DEFAULT(conf_base)
    li      t0, 0
    sb      t0, VREF_FLAG(conf_base)
    sb      t0, FIND_RL_FLAG(conf_base)
    sb      t0, RL_1(conf_base)
    sb      t0, RL_2(conf_base)
    sb      t0, RL_3(conf_base)
    sb      t0, RL_4(conf_base)
    sb      t0, RL_5(conf_base)
    sb      t0, RL_FLAG(conf_base)
    sync
    lb      t0, RL_ORI(conf_base)
    sb      t0, RL_CURRENT(conf_base)

    b       vref_dly_loop
    nop

vref_dly_loop_ctrl:
//find vref_dly and delay 2 nck
    li      t0, 0x1
    sb      t0, VREF_FLAG(conf_base)
    lbu     t0, VREF_DLY_OFFSET(conf_base)
    daddu   t0, t0, 0x2
    sb      t0, VREF_DLY_OFFSET(conf_base)
vref_dly_loop:
/******sent MRS****/
    lb          t1, RL_CURRENT(conf_base)
    move    a0, t1
    bal     rl_mrs_send
    nop
//init 0x1140 and 0x1148 to store vref result
    dli     t0, 0
    not     t0
    sd     t0, 0x1140(conf_base)
    sd     t0, 0x1148(conf_base)
    sb     num_slice, VREF_NUM_OFFSET(conf_base)
    lb     t0, VREF_NUM_OFFSET(conf_base)
    lb     t0, VREF_NUM_OFFSET(conf_base)
    lb     t0, VREF_NUM_OFFSET(conf_base)
    lb     t0, VREF_NUM_OFFSET(conf_base)
    lb     t0, VREF_NUM_OFFSET(conf_base)
    lb     t0, VREF_NUM_OFFSET(conf_base)
	sb	$0, REG_VREF_COUNT(conf_base)
vref_loop:
#ifdef DISABLE_DQ_ODT_TRAINING
    PRINTSTR(".")
#endif
    dli    vref_value, 0x00 //loop vref_ctrl
    dli    s5, 0x0 //store vref_res low64
    dli    s6, 0x0 //store vref_res high64


vref_set_loop:
    dli    dll_vref, 0x0 //loop dll_vref
//clear tst_* value
    dli    t1, 0x1 //max num_slice
    move   t0, conf_base
    sd     $0, SAMPLE0_L64(t0)
    sd     $0, SAMPLE0_H64(t0)
    sd     $0, SAMPLE1_L64(t0)
    sd     $0, SAMPLE1_H64(t0)
    sd     $0, SAMPLE2_L64(t0)
    sd     $0, SAMPLE2_H64(t0)
    sd     $0, SAMPLE3_L64(t0)
    sd     $0, SAMPLE3_H64(t0)
    sd     $0, SAMPLE4_L64(t0)
    sd     $0, SAMPLE4_H64(t0)
1:
    sd     $0, ONE_SAMPLE_L64(t0)
    sd     $0, ONE_SAMPLE_H64(t0)
    daddiu t0, 0x10
    dsubu  t1, 0x1
    bnez   t1, 1b
    nop

    dli    t1, 0x1 //max num_slice
    move   t0, conf_base

1:
	dli	t3, 0xffffffffffffffff
    sd     t3, ZERO_SAMPLE_L64(t0)
    sd     t3, ZERO_SAMPLE_H64(t0)
    daddiu t0, 0x10
    dsubu  t1, 0x1
    bnez   t1, 1b
    nop
//TODO, temporary add here
    lb     t1, 0x0(conf_base)
    beq    t1, 0x11, single_vref_set
    nop

    move   t3, $0
    GET_NODE_ID_a0
    dli    t0, 0x900000003ff00180
    or     t0, a0
    lwu    t1, 0x0(t0)
    dsll   t3, t1, 32//store origin value

    li     t2, ((0x1<<4)|(0x1<<9))
    not    t2, t2
    and    t1, t2
    li     t2, ((0x1<<5)|(0x1<<10))
    or     t1, t2
    sw     t1, 0x0(t0)

    GET_NODE_ID_a0
    dli    t0, 0x900000003ff00400
    or     t0, a0
    lbu    t1, 0x3(t0)
    or     t3, t1 //store mc_en
    lbu    t1, 0x4(t0)
    dsll   t1, 0x8
    or     t3, t1 //store interleave_bit/en
    lwu    t1, 0x0(t0)
    li     t2, (0x3<<30)
    or     t1, t2
    sw     t1, 0x0(t0)

    dli    t4, 0x900004000ff00000
    lhu    t1, 0x810(t4)
    li     t0, (0x7f<<5)
    not    t0, t0
    and    t1, t0
    ori    t1, 0x1
    dsll   t0, vref_value, 5
    or     t1, t0

    sh     t1, 0x810(t4)
    sh     t1, 0x812(t4)
    sh     t1, 0x814(t4)
    sh     t1, 0x816(t4)
    sh     t1, 0x818(t4)
    sh     t1, 0x81a(t4)
    sh     t1, 0x81c(t4)
    sh     t1, 0x81e(t4)
    sh     t1, 0x820(t4)

    dli    t4, 0x900005000ff00000
    lhu    t1, 0x810(t4)
    li     t0, (0x7f<<5)
    not    t0, t0
    and    t1, t0
    ori    t1, 0x1
    dsll   t0, vref_value, 5
    or     t1, t0
    sh     t1, 0x810(t4)
    sh     t1, 0x812(t4)
    sh     t1, 0x814(t4)
    sh     t1, 0x816(t4)
    sh     t1, 0x818(t4)
    sh     t1, 0x81a(t4)
    sh     t1, 0x81c(t4)
    sh     t1, 0x81e(t4)

    GET_NODE_ID_a0
    dli    t0, 0x900000003ff00400
    or     t0, a0
    sb     t3, 0x3(t0)
    dsrl   t3, t3, 8
    sb     t3, 0x4(t0)
    GET_NODE_ID_a0
    dli    t0, 0x900000003ff00180
    or     t0, a0
    dsrl   t3, t3, 24
    sw     t3, 0x0(t0)
    b	   1f
    nop
single_vref_set:
    dsll   t4, num_slice, 1
    daddu  t4, conf_base
    lhu    t1, 0x810(t4)
    li     t0, (0x7f<<5)
    not    t0, t0
    and    t1, t0
    ori    t1, 0x1
    dsll   t0, vref_value, 5
    or     t1, t0
    sh     t1, 0x810(t4)
1:

//sync vref at start of cycle
    bnez   vref_value, 2f
    nop
    dli    t1, WAIT_NUMBER
1:
    dsubu  t1, 1
    bnez   t1, 1b
    nop
2:
//sync vref
    lh     t1, 0x832(t8)
    lh     t1, 0x832(t8)
    lh     t1, 0x832(t8)

	sb	$0, REG_COUNT(conf_base)
	sb	$0, REG_SHIFT(conf_base)
bit_loop:

	sb	$0, REG_COUNT(conf_base)
dll_vref_loop:
    dli    t0, 0x7f
    and    dll_vref, t0
    sb     dll_vref, DLL_VREF_OFFSET(conf_base)
    bnez   dll_vref, 2f
    nop
    dli    t0, WAIT_NUMBER
1:
    dsubu  t0, 1
    bnez   t0, 1b
    nop
2:

//sync dll
    lbu    t0, DLL_VREF_OFFSET(conf_base)
    lbu    t0, DLL_VREF_OFFSET(conf_base)

//set lvl_req
    li     t0, 0x1
    sb     t0, LVL_REQ_OFFSET(conf_base)
    sb     $0, LVL_REQ_OFFSET(conf_base)

//wait lvl_done
1:
    lbu    t0, LVL_DONE_OFFSET(conf_base)
    beqz   t0, 1b
    nop

    li     t0, 0x40
    bge    dll_vref, t0, set_h64
    nop
    move   t3, dll_vref

set_l64:
    lbu    t0, VREF_SAMPLE_OFFSET(conf_base)//TODO, bit8 not included
	lbu	t4, REG_COUNT(conf_base)
	beqz	t4, 1f
	nop
	beq	t4, 1, 2f
	nop
	beq	t4, 2, 3f
	nop
	beq	t4, 3, 4f
	nop
	beq	t4, 4, 5f
	nop

1:
//sample 0
    lbu		t4, REG_SHIFT(conf_base)
    li     t1, 0x1
    dsrl   t0, t4
    and    t1, t0
    dsll   t1, t3
    ld     t2, SAMPLE0_L64(conf_base)
    or     t2, t1
    sd     t2, SAMPLE0_L64(conf_base)
    b      10f
    nop

2:
//sample 1
    lbu		t4, REG_SHIFT(conf_base)
    li     t1, 0x1
    dsrl   t0, t4
    and    t1, t0
    dsll   t1, t3
    ld     t2, SAMPLE1_L64(conf_base)
    or     t2, t1
    sd     t2, SAMPLE1_L64(conf_base)
    b      10f
    nop

3:
//sample 2
    lbu		t4, REG_SHIFT(conf_base)
    li     t1, 0x1
    dsrl   t0, t4
    and    t1, t0
    dsll   t1, t3
    ld     t2, SAMPLE2_L64(conf_base)
    or     t2, t1
    sd     t2, SAMPLE2_L64(conf_base)
    b      10f
    nop

4:
//sample 3
    lbu		t4, REG_SHIFT(conf_base)
    li     t1, 0x1
    dsrl   t0, t4
    and    t1, t0
    dsll   t1, t3
    ld     t2, SAMPLE3_L64(conf_base)
    or     t2, t1
    sd     t2, SAMPLE3_L64(conf_base)
    b      10f
    nop

5:
//sample 4
    lbu		t4, REG_SHIFT(conf_base)
    li     t1, 0x1
    dsrl   t0, t4
    and    t1, t0
    dsll   t1, t3
    ld     t2, SAMPLE4_L64(conf_base)
    or     t2, t1
    sd     t2, SAMPLE4_L64(conf_base)


    b      10f
    nop

set_h64:

    li     t1, 0x40
    dsubu  t3, dll_vref, t1
    lbu    t0, VREF_SAMPLE_OFFSET(conf_base)

	lbu	t4, REG_COUNT(conf_base)
	beqz	t4, 1f
	nop
	beq	t4, 1, 2f
	nop
	beq	t4, 2, 3f
	nop
	beq	t4, 3, 4f
	nop
	beq	t4, 4, 5f
	nop
1:
//sample 0
    lbu		t4, REG_SHIFT(conf_base)
    li     t1, 0x1
    dsrl   t0, t4
    and    t1, t0
    dsll   t1, t3
    ld     t2, SAMPLE0_H64(conf_base)
    or     t2, t1
    sd     t2, SAMPLE0_H64(conf_base)
	b	10f
	nop

2:
//sample 1
    lbu		t4, REG_SHIFT(conf_base)
    li     t1, 0x1
    dsrl   t0, t4
    and    t1, t0
    dsll   t1, t3
    ld     t2, SAMPLE1_H64(conf_base)
    or     t2, t1
    sd     t2, SAMPLE1_H64(conf_base)
	b	10f
	nop

3:
//sample 2
    lbu		t4, REG_SHIFT(conf_base)
    li     t1, 0x1
    dsrl   t0, t4
    and    t1, t0
    dsll   t1, t3
    ld     t2, SAMPLE2_H64(conf_base)
    or     t2, t1
    sd     t2, SAMPLE2_H64(conf_base)
	b	10f
	nop

4:
//sample 3
    lbu		t4, REG_SHIFT(conf_base)
    li     t1, 0x1
    dsrl   t0, t4
    and    t1, t0
    dsll   t1, t3
    ld     t2, SAMPLE3_H64(conf_base)
    or     t2, t1
    sd     t2, SAMPLE3_H64(conf_base)
	b	10f
	nop

5:
//sample 4
    lbu		t4, REG_SHIFT(conf_base)
    li     t1, 0x1
    dsrl   t0, t4
    and    t1, t0
    dsll   t1, t3
    ld     t2, SAMPLE4_H64(conf_base)
    or     t2, t1
    sd     t2, SAMPLE4_H64(conf_base)

10:


//dll_vref_loop ctrl
    addiu  dll_vref, 0x1
    dli    t0, 0x7f//max dll_vref
    ble    dll_vref, t0, dll_vref_loop
    nop

	lbu	t4, REG_COUNT(conf_base)
	ld	t0, ONE_SAMPLE_L64(conf_base)
	ld	t1, ZERO_SAMPLE_L64(conf_base)
	beqz	t4, 1f
	nop
	beq	t4, 1, 2f
	nop
	beq	t4, 2, 3f
	nop
	beq	t4, 3, 4f
	nop
	beq	t4, 4, 5f
	nop
1:	ld	t2, SAMPLE0_L64(conf_base)
	b	6f
	nop
2:	ld	t2, SAMPLE1_L64(conf_base)
	b	6f
	nop
3:	ld	t2, SAMPLE2_L64(conf_base)
	b	6f
	nop
4:	ld	t2, SAMPLE3_L64(conf_base)
	b	6f
	nop
5:	ld	t2, SAMPLE4_L64(conf_base)
	nop
6:	or	t0, t0, t2
	and	t1, t1, t2
	sd	t0, ONE_SAMPLE_L64(conf_base)
	sd	t1, ZERO_SAMPLE_L64(conf_base)

	ld	t0, ONE_SAMPLE_H64(conf_base)
	ld	t1, ZERO_SAMPLE_H64(conf_base)
	beqz	t4, 1f
	nop
	beq	t4, 1, 2f
	nop
	beq	t4, 2, 3f
	nop
	beq	t4, 3, 4f
	nop
	beq	t4, 4, 5f
	nop
1:	ld	t2, SAMPLE0_H64(conf_base)
	b	6f
	nop
2:	ld	t2, SAMPLE1_H64(conf_base)
	b	6f
	nop
3:	ld	t2, SAMPLE2_H64(conf_base)
	b	6f
	nop
4:	ld	t2, SAMPLE3_H64(conf_base)
	b	6f
	nop
5:	ld	t2, SAMPLE4_H64(conf_base)
	nop
6:	or	t0, t0, t2
	and	t1, t1, t2
	sd	t0, ONE_SAMPLE_H64(conf_base)
	sd	t1, ZERO_SAMPLE_H64(conf_base)



#ifdef PRINT_DLL_SAMPLE
#ifdef VREF_TRAINING_DEBUG
	lbu	t4, REG_COUNT(conf_base)
	bnez	t4, 1f
	nop

	PRINTSTR("vref: ")
    move    t2, vref_value
	dsrl    a0, t2, 32
	bal     hexserial
	nop
	move    a0, t2
	bal     hexserial
	nop
	PRINTSTR("\r\n")




1:	PRINTSTR(" dll sample is: ")
	lbu	t4, REG_COUNT(conf_base)
	beqz	t4, 1f
	nop
	beq	t4, 1, 2f
	nop
	beq	t4, 2, 3f
	nop
	beq	t4, 3, 4f
	nop
	beq	t4, 4, 5f
	nop
1:
	ld      t2, SAMPLE0_H64(conf_base)
	dsrl    a0, t2, 32
	bal     hexserial
	nop
	move    a0, t2
	bal     hexserial
	nop
	PRINTSTR("_")
	ld      t2, SAMPLE0_L64(conf_base)
	dsrl    a0, t2, 32
	bal     hexserial
	nop
	move    a0, t2
	bal     hexserial
	nop

	PRINTSTR("\r\n")
	b	6f
	nop
2:
	ld      t2, SAMPLE1_H64(conf_base)
	dsrl    a0, t2, 32
	bal     hexserial
	nop
	move    a0, t2
	bal     hexserial
	nop
	PRINTSTR("_")
	ld      t2, SAMPLE1_L64(conf_base)
	dsrl    a0, t2, 32
	bal     hexserial
	nop
	move    a0, t2
	bal     hexserial
	nop

	PRINTSTR("\r\n")
	b	6f
	nop
3:
	ld      t2, SAMPLE2_H64(conf_base)
	dsrl    a0, t2, 32
	bal     hexserial
	nop
	move    a0, t2
	bal     hexserial
	nop
	PRINTSTR("_")
	ld      t2, SAMPLE2_L64(conf_base)
	dsrl    a0, t2, 32
	bal     hexserial
	nop
	move    a0, t2
	bal     hexserial
	nop

	PRINTSTR("\r\n")
	b	6f
	nop
4:
	ld      t2, SAMPLE3_H64(conf_base)
	dsrl    a0, t2, 32
	bal     hexserial
	nop
	move    a0, t2
	bal     hexserial
	nop
	PRINTSTR("_")
	ld      t2, SAMPLE3_L64(conf_base)
	dsrl    a0, t2, 32
	bal     hexserial
	nop
	move    a0, t2
	bal     hexserial
	nop

	PRINTSTR("\r\n")
	b	6f
	nop
5:
	ld      t2, SAMPLE4_H64(conf_base)
	dsrl    a0, t2, 32
	bal     hexserial
	nop
	move    a0, t2
	bal     hexserial
	nop
	PRINTSTR("_")
	ld      t2, SAMPLE4_L64(conf_base)
	dsrl    a0, t2, 32
	bal     hexserial
	nop
	move    a0, t2
	bal     hexserial
	nop

	PRINTSTR("\r\n")

6:


#endif
#endif

	dli	dll_vref, 0x0
	lbu	t4, REG_COUNT(conf_base)
	daddu	t4, 1
	sb	t4, REG_COUNT(conf_base)
	bltu	t4, DLL_LOOP_COUNT, dll_vref_loop
	nop

#ifdef PRINT_DLL_SAMPLE
#ifdef VREF_TRAINING_DEBUG
	PRINTSTR(" or  sample is: ")
	ld      t2, ONE_SAMPLE_H64(conf_base)
	dsrl    a0, t2, 32
	bal     hexserial
	nop
	move    a0, t2
	bal     hexserial
	nop
	PRINTSTR("_")
	ld      t2, ONE_SAMPLE_L64(conf_base)
	dsrl    a0, t2, 32
	bal     hexserial
	nop
	move    a0, t2
	bal     hexserial
	nop

	PRINTSTR("\r\n")
	PRINTSTR(" and sample is: ")
	ld      t2, ZERO_SAMPLE_H64(conf_base)
	dsrl    a0, t2, 32
	bal     hexserial
	nop
	move    a0, t2
	bal     hexserial
	nop
	PRINTSTR("_")
	ld      t2, ZERO_SAMPLE_L64(conf_base)
	dsrl    a0, t2, 32
	bal     hexserial
	nop
	move    a0, t2
	bal     hexserial
	nop

	PRINTSTR("\r\n")
#endif
#endif

correct_1_loop:
	dli	t0, 0x0
1:	ld	t1, ONE_SAMPLE_L64(conf_base)
	dsrl	t1, t0
	dli	t2, 0x1
	and	t2, t1
	beqz	t2, 14f
	nop
	move	t4, conf_base
	dli	t6, 0x1
	dli	t3, 0x0
11:	ld	t1, SAMPLE0_L64(t4)
	dsrl	t1, t0
	dli	t2, 0x1
	and	t2, t1
	daddiu	t6, 0x1
	daddiu	t4, 0x10
	beq	t2, 0x1, 12f
	nop
	bleu	t6, 0x5, 11b
	nop
	b	13f
	nop
12:	daddiu	t3, 0x1
	bleu	t6, 0x5, 11b
	nop
13:	bgeu	t3, 0x4, 14f
	nop
	dli	t2, 0x1
	dsll	t2, t0
	not	t2
	ld	t1, ONE_SAMPLE_L64(conf_base)
	and	t1, t2
	sd	t1, ONE_SAMPLE_L64(conf_base)
14:	daddiu	t0, 0x1
	bltu	t0, 0x40, 1b
	nop

	dli	t0, 0x0
1:	ld	t1, ONE_SAMPLE_H64(conf_base)
	dsrl	t1, t0
	dli	t2, 0x1
	and	t2, t1
	beqz	t2, 14f
	nop
	move	t4, conf_base
	dli	t6, 0x1
	dli	t3, 0x0
11:	ld	t1, SAMPLE0_H64(t4)
	dsrl	t1, t0
	dli	t2, 0x1
	and	t2, t1
	daddu	t6, 0x1
	daddu	t4, 0x10
	beq	t2, 0x1, 12f
	nop
	bleu	t6, 0x5, 11b
	nop
	b	13f
	nop
12:	daddu	t3, 0x1
	bleu	t6, 0x5, 11b
	nop
13:	bgeu	t3, 0x4, 14f
	nop
	dli	t2, 0x1
	dsll	t2, t0
	not	t2
	ld	t1, ONE_SAMPLE_H64(conf_base)
	and	t1, t2
	sd	t1, ONE_SAMPLE_H64(conf_base)
14:	daddu	t0, t0, 0x1
	bltu	t0, 0x40, 1b
	nop

correct_0_loop:
	dli	t0, 0x0
1:	ld	t1, ZERO_SAMPLE_L64(conf_base)
	dsrl	t1, t0
	dli	t2, 0x1
	and	t2, t1
	beq	t2, 0x1, 14f
	nop
	move	t4, conf_base
	dli	t6, 0x1
	dli	t3, 0x0
11:	ld	t1, SAMPLE0_L64(t4)
	dsrl	t1, t0
	dli	t2, 0x1
	and	t2, t1
	daddu	t6, 0x1
	daddu	t4, 0x10
	beqz	t2, 12f
	nop
	bleu	t6, 0x5, 11b
	nop
	b	13f
	nop
12:	daddu	t3, 0x1
	bleu	t6, 0x5, 11b
	nop
13:	bgeu	t3, 0x4, 14f
	nop
	dli	t2, 0x1
	dsll	t2, t0
	ld	t1, ZERO_SAMPLE_L64(conf_base)
	or	t1, t2
	sd	t1, ZERO_SAMPLE_L64(conf_base)
14:	daddu	t0, t0, 0x1
	bltu	t0, 0x40, 1b
	nop

	subu	t0, t0, 0x40
1:	ld	t1, ZERO_SAMPLE_H64(conf_base)
	dsrl	t1, t0
	dli	t2, 0x1
	and	t2, t1
	beq	t2, 0x1, 14f
	nop
	move	t4, conf_base
	dli	t6, 0x1
	dli	t3, 0x0
11:	ld	t1, SAMPLE0_H64(t4)
	dsrl	t1, t0
	dli	t2, 0x1
	and	t2, t1
	daddu	t6, 0x1
	daddu	t4, 0x10
	beqz	t2, 12f
	nop
	bleu	t6, 0x5, 11b
	nop
	b	13f
	nop
12:	daddu	t3, 0x1
	bleu	t6, 0x5, 11b
	nop
13:	bgeu	t3, 0x4, 14f
	nop
	dli	t2, 0x1
	dsll	t2, t0
	ld	t1, ZERO_SAMPLE_H64(conf_base)
	or	t1, t2
	sd	t1, ZERO_SAMPLE_H64(conf_base)
14:	daddu	t0, t0, 0x1
	bltu	t0, 0x40, 1b
	nop


#ifdef PRINT_DLL_SAMPLE
#ifdef VREF_TRAINING_DEBUG
	PRINTSTR(" or  correc is: ")
	ld      t2, ONE_SAMPLE_H64(conf_base)
	dsrl    a0, t2, 32
	bal     hexserial
	nop
	move    a0, t2
	bal     hexserial
	nop
	PRINTSTR("_")
	ld      t2, ONE_SAMPLE_L64(conf_base)
	dsrl    a0, t2, 32
	bal     hexserial
	nop
	move    a0, t2
	bal     hexserial
	nop

	PRINTSTR("\r\n")
	PRINTSTR(" and correc is: ")
	ld      t2, ZERO_SAMPLE_H64(conf_base)
	dsrl    a0, t2, 32
	bal     hexserial
	nop
	move    a0, t2
	bal     hexserial
	nop
	PRINTSTR("_")
	ld      t2, ZERO_SAMPLE_L64(conf_base)
	dsrl    a0, t2, 32
	bal     hexserial
	nop
	move    a0, t2
	bal     hexserial
	nop

	PRINTSTR("\r\n")
#endif
#endif

//compare duty cycle for each bit

bit_check_loop:
11:
    ld     t4, ONE_SAMPLE_L64(conf_base)
    ld     s7, ONE_SAMPLE_H64(conf_base)
    li     t6, 127//loop times


find_cont1:
    beqz   t6, chk_fail
    nop

    move   t3, t4

    li     t1, 0x1
    and    t1, t4
    li     t0, 63
    dsll   t1, t0

    li     t2, 0x1
    and    t2, s7
    li     t0, 63
    dsll   t2, t0

    dsrl   t4, 0x1
    or     t4, t2
    dsrl   s7, 0x1
    or     s7, t1

    dli    t0, (0x1<<CONT1_NUM)
    dsubu  t0, 0x1
    and    t3, t0
    bne    t3, t0, find_cont1
    subu   t6, 0x1

    ld     t4, ZERO_SAMPLE_L64(conf_base)
    ld     s7, ZERO_SAMPLE_H64(conf_base)
    li     t6, 127//loop times

find_cont0:
    beqz   t6, chk_fail
    nop

	move   t3, t4
    li     t1, 0x1
    and    t1, t4
    li     t0, 63
    dsll   t1, t0

    li     t2, 0x1
    and    t2, s7
    li     t0, 63
    dsll   t2, t0

    dsrl   t4, 0x1
    or     t4, t2
    dsrl   s7, 0x1
    or     s7, t1

    dli    t0, (0x1<<CONT0_NUM)
    dsubu  t0, 0x1
    and    t3, t0
    bnez   t3, find_cont0
    subu   t6, 0x1

chk_pass:
    li     t0, 0x40
    blt    vref_value, t0, 1f
    nop
    subu   t0, vref_value, t0
    li     t1, 0x1
    dsll   t1, t0
    or     s6, t1

    b      2f
    nop

1:
    li     t0, 0x1
    dsll   t0, vref_value
    or     s5, t0

2:
//print
//	PRINTSTR("pass\r\n")
#if 1
next_bit_check:
	sd	$0, ONE_SAMPLE_L64(conf_base)
	sd	$0, ONE_SAMPLE_H64(conf_base)
	sd	$0, SAMPLE0_L64(conf_base)
	sd	$0, SAMPLE0_H64(conf_base)
	sd	$0, SAMPLE1_L64(conf_base)
	sd	$0, SAMPLE1_H64(conf_base)
	sd	$0, SAMPLE2_L64(conf_base)
	sd	$0, SAMPLE2_H64(conf_base)
	sd	$0, SAMPLE3_L64(conf_base)
	sd	$0, SAMPLE3_H64(conf_base)
	sd	$0, SAMPLE4_L64(conf_base)
	sd	$0, SAMPLE4_H64(conf_base)
	dli	t0, 0xffffffffffffffff
	sd	t0, ZERO_SAMPLE_L64(conf_base)
	sd	t0, ZERO_SAMPLE_H64(conf_base)
	lbu	t4, REG_SHIFT(conf_base)
	daddu	t4, 0x1
	dli	dll_vref, 0x0
	sb	t4, REG_SHIFT(conf_base)
	bltu	t4, BIT_LOOP_CONTROL, bit_loop
	nop
#endif


    b      slice_finish
    nop

//1bit in a slice fail, indicate this vref_value is not right
chk_fail:
    li     t0, 0x40
    blt    vref_value, t0, 1f
    nop
    subu   t0, vref_value, t0
    li     t1, 0x1
    dsll   t1, t0
    not    t1, t1
    and    s6, t1

    b      2f
    nop

1:
    li     t0, 0x1
    dsll   t0, vref_value
    not    t0, t0
    and    s5, t0

2:

//print
//	PRINTSTR("fail\r\n")

slice_finish:

next_vref_set:
    daddiu vref_value, 0x1
    dli    t0, 0x7f
    ble    vref_value, t0, vref_set_loop
    nop



#if 0
#ifdef VREF_TRAINING_DEBUG
//print
	PRINTSTR("\r\nNO.")
	lbu	t4, REG_VREF_COUNT(conf_base)
    move    a0, t4
    bal     hexserial
    nop
	PRINTSTR("      ")

	dsrl   a0, s6, 32
	bal    hexserial
	nop

	dsrl   a0, s6, 0
	bal    hexserial
	nop

	dsrl   a0, s5, 32
	bal    hexserial
	nop

	dsrl   a0, s5, 0
	bal    hexserial
	nop

#endif
#endif


//TODO, use mrs for ddr3 store s5/s6 for each slice
    ld     t0, 0x1140(conf_base)
    and     t0, s5
    sd     t0, 0x1140(conf_base)

    ld     t0, 0x1148(conf_base)
    and     t0, s6
    sd     t0, 0x1148(conf_base)

//optimize vref_dly
    dli     t4, 0
    dli     t1, 0
    dli     t2, 0x1
1:
    ld      t0, 0x1140(conf_base)
    and     t3, t0, t2
    dsrl    t3, t1
    daddu   t4, t3
    ld      t0, 0x1148(conf_base)
    and     t3, t0, t2
    dsrl    t3, t1
    daddu   t4, t3
    daddu   t1, 1
    dsll    t2, 1
    bleu    t1, 63, 1b
    nop

    ld     s5, 0x1140(conf_base)
    ld     s6, 0x1148(conf_base)
#ifdef  VREF_TRAINING
    PRINTSTR("\r\nthis loop result:     ")
	dsrl   a0, s6, 32
	bal    hexserial
	nop

	dsrl   a0, s6, 0
	bal    hexserial
	nop

	dsrl   a0, s5, 32
	bal    hexserial
	nop

	dsrl   a0, s5, 0
	bal    hexserial
	nop
#endif

#ifdef VREF_TRAINING
	PRINTSTR("\r\nthis loop vref_dly:   ")
    lb      t0, VREF_DLY_OFFSET(conf_base)
    move    a0, t0
    bal     hexserial
    nop
    PRINTSTR("\r\n")
#endif
//t4 means the number of 1 in vref training result

    lb      t0, RL_FLAG(conf_base)
    beq     t0, 0x1, out_vref_dly_loop2
    nop
    lb      t0, VREF_FLAG(conf_base)
    beq     t0, 0x1,begin1
    nop
    bgtu    t4, 0xa, vref_dly_loop_ctrl
    nop

#ifdef VREF_TRAINING
    PRINTSTR("vref_dly training RL: ")
    lb      t0, RL_CURRENT(conf_base)
    move    a0, t0
    bal     hexserial
    nop
    PRINTSTR("\r\n")
#endif
    lb      t0, VREF_DLY_OFFSET(conf_base)
    daddu   t0, 1
    bgeu    t0, 0x20, 1f
    nop
    sb      t0, VREF_DLY_OFFSET(conf_base)
    b       vref_dly_loop
    nop
1:
#ifdef VREF_TRAINING
    PRINTSTR("\r\nVref_dly training ERROR!!!!!!!!!!!!!!!!!!***********************************\n")
#endif
    b       out_vref_dly_loop3
    nop

begin1:

#ifdef VREF_TRAINING
    PRINTSTR("tRL training-------RL:")
    lb      t1, RL_CURRENT(conf_base)
    move    a0, t1
    bal     hexserial
    nop
    PRINTSTR("\r\n.\n")
#endif

    lb      t1, FIND_RL_FLAG(conf_base)
    addu    t1, t1,0x1
    sb      t1, FIND_RL_FLAG(conf_base)
    lb      t0, TPHY_RDDATA_OFFSET(conf_base)
    addu    t0, t0, 0x1
    sb      t0, TPHY_RDDATA_OFFSET(conf_base)
    lb      t0, RL_CURRENT(conf_base)
    addu    t0, t0, 0x1
    sb      t0, RL_CURRENT(conf_base)
    bgtu    t0, 0x21 ,1f
    nop
    bgtu    t1, 0x4, 9f
    nop
    bgtu    t1, 0x3, 8f
    nop
    bgtu    t1, 0x2, 7f
    nop
    bgtu    t1, 0x1, 6f
    nop
    sb      t4, RL_1(conf_base)
    b       vref_dly_loop
    nop
6:
    sb      t4, RL_2(conf_base)
    b       vref_dly_loop
    nop
7:
    sb      t4, RL_3(conf_base)
    b       vref_dly_loop
    nop
8:
    sb      t4, RL_4(conf_base)
    b       vref_dly_loop
    nop
9:
    sb      t4, RL_5(conf_base)
    lb      t0, RL_1(conf_base)
    bgtu    t4, t0, 10f
    nop
//    lb      t0, RL_CURRENT(conf_base)
//    bgtu    t0, 0x1c,1f
//    nop
    lb      t0,RL_4(conf_base)
    bltu    t0, 0xa, 1f
    nop
    b       2f
    nop
1:
    PRINTSTR("Training RL ERROR!!!!!***********************************************************************\r\n")
    b       out_vref_dly_loop2
    nop

2:
    lb      t0, TPHY_RDDATA_OFFSET(conf_base)
    subu    t0, t0, 0x3
    sb      t0, TPHY_RDDATA_OFFSET(conf_base)
    lb      t0, RL_CURRENT(conf_base)
    subu    t0, t0, 0x3
    sb      t0, RL_CURRENT(conf_base)
#ifdef VREF_TRAINING
    PRINTSTR("\r\n")
    PRINTSTR("find RL2:  ")
    move    a0, t0
    bal     hexserial
    nop
    subu    t0, t0, 0x1
    PRINTSTR("   and RL1:  ")
    move    a0, t0
    bal     hexserial
    nop
    PRINTSTR("\r\n")
#endif
//sum=RL_1+RL_2+...+RL5
    lb      t0, RL_1(conf_base)
    lb      t1, RL_2(conf_base)
    addu    t1, t1, t0
    lb      t0, RL_3(conf_base)
    addu    t1, t1, t0
    lb      t0, RL_4(conf_base)
    addu    t1, t1,t0
    lb      t0, RL_5(conf_base)
    addu    t1, t1,t0
    bgtu    t1, 0x32,out_vref_dly_loop        //for 8GB 1Rx8 PC-2666u-19-19-19  26v :10*5=50=0x32 is fine!!!!!!!!!(vref_dly+2)
    nop                                       //for 16GB 1Rx4 PC4-3200AA-RC2-12     :25*4=100=0x64,when training RL,some slice has no data(vref_dly+1)
10:
    //exchange RL
    lb      t0, RL_2(conf_base)
    sb      t0, RL_1(conf_base)
    lb      t0, RL_3(conf_base)
    sb      t0, RL_2(conf_base)
    lb      t0, RL_4(conf_base)
    sb      t0, RL_3(conf_base)
    lb      t0, RL_5(conf_base)
    sb      t0, RL_4(conf_base)
    lb      t0, FIND_RL_FLAG(conf_base)
    subu    t0, t0, 0x1
    sb      t0, FIND_RL_FLAG(conf_base)
    b       vref_dly_loop
    nop
/*****************************************************************************/
out_vref_dly_loop:
    li          t1, 0x1
    sb          t1, RL_FLAG(conf_base)
    b           vref_dly_loop
    nop

//*********sent MRS command to set rl as BIG_RL******/
out_vref_dly_loop2:
    lb      t1, TPHY_RDDATA_OFFSET(conf_base)
    subu    t1, t1, 0x1
    sb      t1, TPHY_RDDATA_OFFSET(conf_base)
    lb      t1, RL_CURRENT(conf_base)
#ifdef VREF_TRAINING
    PRINTSTR("last training RL:     ")
    move    a0, t1
    bal     hexserial
    nop
    PRINTSTR("\r\n\n")
#endif
    subu    t1, t1,0x1
    sb      t1, RL_CURRENT(conf_base)
    move    a0, t1
    bal     rl_mrs_send
    nop
	lbu	    t4, REG_VREF_COUNT(conf_base)
	daddu	t4, 1
	sb  	t4, REG_VREF_COUNT(conf_base)
	bltu	t4, VREF_LOOP_COUNT, vref_loop
	nop
out_vref_dly_loop3:
//reset rddata value
    lb      t1, RDDATA_DEFAULT(conf_base)
    sb      t1, TPHY_RDDATA_OFFSET(conf_base)

/****sent MRS ****/
    lb      t1, RL_ORI(conf_base)
#ifdef VREF_TRAINING
    PRINTSTR("\r\nreset RL as default value")
#endif
    move    a0, t1
    bal     rl_mrs_send
    nop
    ld      ra, RA_STORE(conf_base)
    jr      ra
    nop
END(vref_train_kernal)
