#define     WRDQS_DLL_RESULT_BIT0   0x3300
#define     WRDQS_BDLY_RESULT_BIT0  0x3308
#define     WRDQS_DLL_RESULT_OR     0x1160
#define     WRDQS_BDLY_RESULT_OR    0x1168
#define     FIND_CONTINUE_MAX       0x1158
#define     FIND_CONTINUE_LSB       0x1159
#define     CYCLE_COUNT             0x115a
#define     MID_STORE_BIT0          0x1170

#define     WR_BIT_TRAINING_DEBUG
#define     REPEAT_TIMES            0x4
#define     PAGE_SIZE               0x0
#define     PAGE_NUMBER             0x1
#define     CORRECT_PARAM           0x9
#define     WATI_INIT_TIME          0x200

	.text
    .global wr_bit_training
    .ent    wr_bit_training
wr_bit_training:
    move    t9, ra

    dli     s6, 0       //slice number
wr_bit_slice_loop:
//init reg
    dli     t0, 0
    move    t1, t8
1:
    sd      $0, (WRDQS_DLL_RESULT_BIT0)(t1)
    daddu   t1, 8
    daddu   t0, 1
    bleu    t0, 16, 1b
    nop
//#ifdef WR_BIT_TRAINING_DEBUG
    PRINTSTR("\r\nSlice No.")
    move    a0, s6
    bal     hexserial
    nop
    PRINTSTR(" start")
//#endif

    dli     s5, 0       //s5--bit number
wr_training_bit_loop:
#ifdef WR_BIT_TRAINING_DEBUG
    PRINTSTR("\r\nBit No.")
    move    a0, s5
    bal     hexserial
    nop
#endif
    PRINTSTR(".")

    dli     t0, 0
    sb      t0, CYCLE_COUNT(t8)
//start cycle loop
cycle_loop:
//start wrdqs_dll loop
    dli     t5, 0
wrdqs_dll_loop:
#ifdef WR_BIT_TRAINING_DETAIL_DEBUG
    PRINTSTR("\r\nwrdqs_dll is ")
    move    a0, t5
    bal     hexserial
    nop
#endif
    or      t1, t5, 0x80    //bypass mode
    dsll    t0, s6, 7   //t0=s6*0x80
    daddu   t0, t0, t8
    sb      t1, DLL_WRDQS_OFFSET(t0)
    dli     t0, 0       //base address
    sd      t0, BASE_ADDR_OFFSET(t8)
    dli     t0, 0xff
    beq     s6, 8, 1f
    nop
    dsll    t1, s6, 3
    dsll    t0, t1      //valid bits
    sd      t0, VALID_BITS_OFFSET(t8)
    dli     t0, 0
    sb      t0, VALID_BITS_ECC_OFFSET(t8)
    b       2f
    nop
1:
    sb      t0, VALID_BITS_ECC_OFFSET(t8)
    sd      $0, VALID_BITS_OFFSET(t8)
2:
    dli     t0, PAGE_SIZE     //page size
    sb      t0, PAGE_SIZE_OFFSET(t8)
    dli     t0, PAGE_NUMBER     //page number
    sw      t0, PAGE_NUM_OFFSET(t8)
    bal     test_engine
    nop
    bnez    v0, wrdqs_test_pass     //test sucess
    nop

//test with error
    beq     s6, 8, 1f
    nop
    ld      t1, 0x3180(t8)
    ld      t2, 0x3188(t8)
    or      t1, t2
    ld      t2, 0x31a0(t8)
    or      t1, t2
    ld      t2, 0x31a8(t8)
    or      t1, t2
    not     t1          //t1 store error bits and reverse, 1 represent right
    dsll    t0, s6, 3
    dsrl    t1, t0

    b       wr_dqs_result_loop
    nop
1:
    lb      t1, 0x316e(t8)
    lb      t2, 0x316f(t8)
    or      t1, t2
    lb      t2, 0x31b2(t8)
    or      t1, t2
    lb      t2, 0x31b3(t8)
    or      t1, t2
    not     t1          //t1 store ECC error bits and reverse, 1 represent right
    b       wr_dqs_result_loop
    nop
wrdqs_test_pass:
    dli     t1, 0
    not     t1
wr_dqs_result_loop:
//bit loop
//result store in different cycle
    lb      t2, CYCLE_COUNT(t8)
    dsll    t6, t2, 4   //a1=t2*0x10
    daddu   t0, t6, t8

    dsrl    t6, t1, s5
    and     t6, 0x1
    dsll    t6, t6, t5
    ld      t2, WRDQS_DLL_RESULT_BIT0(t0)
    or      t2, t6
    sd      t2, WRDQS_DLL_RESULT_BIT0(t0)

    daddu   t5, 1
    lb      t0, DLL_VALUE_OFFSET(t8)
    bleu    t0, 63, 1f
    nop
    dli     t0, 63
1:
    bleu    t5, t0, wrdqs_dll_loop
    nop

//if DLL value<63, skip bdly dll
    lb      t0, DLL_VALUE_OFFSET(t8)
    bleu    t0, 63, skip_bdly_dll
    nop
//start wr_bdly_dll loop
    dli     t5, 0
wr_bdly_dll_loop:
#ifdef WR_BIT_TRAINING_DETAIL_DEBUG
    PRINTSTR("\r\nbdly_dll is ")
    move    a0, t5
    bal     hexserial
    nop
#endif
    dsll    t0, s6, 7   //t0=s6*0x80
    daddu   t0, t0, t8
    sb      t5, WRDQS0_BDLY_OFFSET(t0)
    dli     t0, 0       //base address
    sd      t0, BASE_ADDR_OFFSET(t8)
    dli     t0, 0xff
    beq     s6, 8, 1f
    nop
    dsll    t1, s6, 3
    dsll    t0, t1      //valid bits
    sd      t0, VALID_BITS_OFFSET(t8)
    dli     t0, 0
    sb      t0, VALID_BITS_ECC_OFFSET(t8)
    b       2f
    nop
1:
    sb      t0, VALID_BITS_ECC_OFFSET(t8)
    sd      $0, VALID_BITS_OFFSET(t8)
2:
    dli     t0, PAGE_SIZE     //page size
    sb      t0, PAGE_SIZE_OFFSET(t8)
    dli     t0, PAGE_NUMBER     //page number
    sw      t0, PAGE_NUM_OFFSET(t8)
    bal     test_engine
    nop
    bnez    v0, bdly_test_pass     //test sucess
    nop
//test with error
    beq     s6, 8, 1f
    nop
    ld      t1, 0x3180(t8)
    ld      t2, 0x3188(t8)
    or      t1, t2
    ld      t2, 0x31a0(t8)
    or      t1, t2
    ld      t2, 0x31a8(t8)
    or      t1, t2
    not     t1          //t1 store error bits and reverse, 1 represent right
    dsll    t0, s6, 3
    dsrl    t1, t0
    b       wr_bdly_result_loop
    nop
1:
    lb      t1, 0x316e(t8)
    lb      t2, 0x316f(t8)
    or      t1, t2
    lb      t2, 0x31b2(t8)
    or      t1, t2
    lb      t2, 0x31b3(t8)
    or      t1, t2
    not     t1          //t1 store ECC error bits and reverse, 1 represent right
    b       wr_bdly_result_loop
    nop
bdly_test_pass:
    dli     t1, 0
    not     t1

wr_bdly_result_loop:
//result store in different cycle
    lb      t2, CYCLE_COUNT(t8)
    dsll    t6, t2, 4   //t6=t2*16
    daddu   t0, t6, t8

    dsrl    t6, t1, s5
    and     t6, 0x1
    dsll    t6, t5
    ld      t3, WRDQS_BDLY_RESULT_BIT0(t0)
    or      t3, t6
    sd      t3, WRDQS_BDLY_RESULT_BIT0(t0)


    daddu   t5, 1
    lb      t0, DLL_VALUE_OFFSET(t8)
    dsubu   t0, 63
    bleu    t5, t0, wr_bdly_dll_loop
    nop

skip_bdly_dll:

    dli     t5, 0
    dsll    t0, s6, 7   //t0=s6*0x80
    daddu   t0, t0, t8
    sb      t5, WRDQS0_BDLY_OFFSET(t0)

    dli     t5, 0
    or      t1, t5, 0x80    //bypass mode
    dsll    t0, s6, 7   //t0=s6*0x80
    daddu   t0, t0, t8
    sb      t1, DLL_WRDQS_OFFSET(t0)

    dli     t0, 0
1:
    bleu    t0, WATI_INIT_TIME, 1b
    daddu   t0, 1
    nop

    lb      t0, CYCLE_COUNT(t8)
    daddu   t0, 1
    sb      t0, CYCLE_COUNT(t8)
    bltu    t0, REPEAT_TIMES, cycle_loop
    nop

//or wrdqs dll result
    dli     t3, 0
1:
    dsll    t5, t3, 4
    daddu   t2, t5, t8
    ld      t1, WRDQS_DLL_RESULT_OR(t8)
    ld      t0, WRDQS_DLL_RESULT_BIT0(t2)
    or      t1, t0
    sd      t1, WRDQS_DLL_RESULT_OR(t8)
    daddu   t3, 1
    bltu    t3, REPEAT_TIMES, 1b
    nop

//or wrdqs bdly result
    dli     t3, 0
1:
    dsll    t5, t3, 4
    daddu   t2, t5, t8
    ld      t1, WRDQS_BDLY_RESULT_OR(t8)
    ld      t0, WRDQS_BDLY_RESULT_BIT0(t2)
    or      t1, t0
    sd      t1, WRDQS_BDLY_RESULT_OR(t8)
    daddu   t3, 1
    bltu    t3, REPEAT_TIMES, 1b
    nop

#ifdef WR_BIT_TRAINING_DEBUG
    dli     t5, 0
1:
    dsll    t1, t5, 4
    daddu   t1, t8
    PRINTSTR("\r\n No. ")
    move    a0, t5
    bal     hexserial
    nop
    PRINTSTR(" result is: ")
    ld      a0, WRDQS_BDLY_RESULT_BIT0(t1)
    bal     hexserial
    nop
    PRINTSTR("--")
    ld      t3, WRDQS_DLL_RESULT_BIT0(t1)
    dsrl    a0, t3, 32
    bal     hexserial
    nop
    PRINTSTR("_")
    move    a0, t3
    bal     hexserial
    nop
    daddu   t5, 1
    bltu    t5, REPEAT_TIMES, 1b
    nop
    PRINTSTR("\r\n")
#endif


//correct every dll result according to the number of 1
    dli     t0, 0
wrdqs_dll_or_loop:
    ld      t1, WRDQS_DLL_RESULT_OR(t8)
    dsrl    t1, t0
    and     t1, 0x1
    beqz    t1, wrdqs_dll_or_loop_ctrl
    nop

//count the number of 1 at some wrdqs dll
    dli     t2, 0
    dli     t6, 0
wrdqs_dll_correct_loop:
    dsll    t3, t2, 4
    daddu   t3, t8
    ld      t5, WRDQS_DLL_RESULT_BIT0(t3)
    dsrl    t5, t0
    and     t5, 0x1
    beqz    t5, wrdqs_dll_correct_loop_ctrl
    nop
    daddu   t6, 1
wrdqs_dll_correct_loop_ctrl:
    daddu   t2, 1
    bltu    t2, REPEAT_TIMES, wrdqs_dll_correct_loop
    nop

//correct 1 to 0 if count number t6 is less than REPEAT_TIMES-1
    bgeu    t6, (REPEAT_TIMES-1), wrdqs_dll_or_loop_ctrl
    nop
    ld      t1, WRDQS_DLL_RESULT_OR(t8)
    dli     t2, 0x1
    dsll    t2, t0
    not     t2
    and     t1, t2
    sd      t1, WRDQS_DLL_RESULT_OR(t8)

wrdqs_dll_or_loop_ctrl:
    daddu   t0, 1
    bleu    t0, 63, wrdqs_dll_or_loop
    nop

//if there is 0 in the middle of result, recorrect
    dli     t6, CORRECT_PARAM
    dli     t2, 0x1
    dsll    t2, t6
    dsubu   t2, 1
    dsrl    t1, t6, 1
    dli     t3, 0x1
    dsll    t3, t1
    not     t3
    and     t1, t2, t3
    dli     t0, 0
1:
    ld      t3, WRDQS_DLL_RESULT_OR(t8)
    dsrl    t5, t3, t0
    and     t5, t2
    bne     t5, t1, 2f
    nop
    move    t5, t2
    dsll    t5, t0
    or      t3, t5
    sd      t3, WRDQS_DLL_RESULT_OR(t8)
2:
    daddu   t0, 1
    bleu    t0, (63-CORRECT_PARAM), 1b
    nop

//correct every bdly dll result according to the number of 1
    dli     t0, 0
wrdqs_bdly_or_loop:
    ld      t1, WRDQS_BDLY_RESULT_OR(t8)
    dsrl    t1, t0
    and     t1, 0x1
    beqz    t1, wrdqs_bdly_or_loop_ctrl
    nop

//count the number of 1 at some wrdqs dll
    dli     t2, 0
    dli     t6, 0
wrdqs_bdly_correct_loop:
    dsll    t3, t2, 4
    daddu   t3, t8
    ld      t5, WRDQS_BDLY_RESULT_BIT0(t3)
    dsrl    t5, t0
    and     t5, 0x1
    beqz    t5, wrdqs_bdly_correct_loop_ctrl
    nop
    daddu   t6, 1
wrdqs_bdly_correct_loop_ctrl:
    daddu   t2, 1
    bltu    t2, REPEAT_TIMES, wrdqs_bdly_correct_loop
    nop

//correct 1 to 0 if count number t6 is less than REPEAT_TIMES-1
    bgeu    t6, (REPEAT_TIMES-1), wrdqs_bdly_or_loop_ctrl
    nop
    ld      t1, WRDQS_BDLY_RESULT_OR(t8)
    dli     t2, 0x1
    dsll    t2, t0
    not     t2
    and     t1, t2
    sd      t1, WRDQS_BDLY_RESULT_OR(t8)

wrdqs_bdly_or_loop_ctrl:
    daddu   t0, 1
    bleu    t0, 15, wrdqs_bdly_or_loop
    nop

//if there is 0 in the middle of result, recorrect
    dli     t6, CORRECT_PARAM
    dli     t2, 0x1
    dsll    t2, t6
    dsubu   t2, 1
    dsrl    t1, t6, 1
    dli     t3, 0x1
    dsll    t3, t1
    not     t3
    and     t1, t2, t3
    dli     t0, 0
1:
    ld      t3, WRDQS_DLL_RESULT_OR(t8)
    dsrl    t5, t3, t0
    and     t5, t2
    bne     t5, t1, 2f
    nop
    move    t5, t2
    dsll    t5, t0
    or      t3, t5
    sd      t3, WRDQS_DLL_RESULT_OR(t8)
2:
    daddu   t0, 1
    bleu    t0, (15-CORRECT_PARAM), 1b
    nop

#ifdef WR_BIT_TRAINING_DEBUG
    PRINTSTR("      correct result is: ")
    ld      a0, WRDQS_BDLY_RESULT_OR(t8)
    bal     hexserial
    nop
    PRINTSTR("--")
    ld      t3, WRDQS_DLL_RESULT_OR(t8)
    dsrl    a0, t3, 32
    bal     hexserial
    nop
    PRINTSTR("_")
    move    a0, t3
    bal     hexserial
    nop
    PRINTSTR("\r\n")
#endif

//find  the mid of continue 1 and the max mid as t7
find_wr_mid_loop:
    dli     t7, 0
    dli     t0, 0
    dli     t6, 0       //count for 1
    sb      t6, FIND_CONTINUE_MAX(t8)       //max number of continue 1
    sb      t6, FIND_CONTINUE_LSB(t8)       //lowest position of 0
1:
    ld      t3, WRDQS_DLL_RESULT_OR(t8)
    and     t2, t3, 0x1
    dsrl    t3, 1
    ld      t7, WRDQS_BDLY_RESULT_OR(t8)
    and     t5, t7, 0x1
    dsrl    t7, 1
    dsll    t5, 63
    or      t3, t5
    sd      t3, WRDQS_DLL_RESULT_OR(t8)
    dsll    t2, 15
    or      t7, t2
    sd      t7, WRDQS_BDLY_RESULT_OR(t8)

    beqz    t2, 2f
    nop
    daddu   t6, 1
    b       4f
    nop
2:
    lb      t3, FIND_CONTINUE_MAX(t8)
    bgeu    t3, t6, 3f
    nop
    move    t3, t6
    sb      t3, FIND_CONTINUE_MAX(t8)
    dsubu   t2, t0, t3
    sb      t2, FIND_CONTINUE_LSB(t8)
3:
    dli     t6, 0
4:
    daddu   t0, 1
    bleu    t0, 79, 1b
    nop

#ifdef WR_BIT_TRAINING_DEBUG
    PRINTSTR("\r\nmax=")
    lb      a0, FIND_CONTINUE_MAX(t8)
    bal     hexserial
    nop
    PRINTSTR("\r\nlsb=")
    lb      a0, FIND_CONTINUE_LSB(t8)
    bal     hexserial
    nop
#endif
//store the mid
    lb      t3, FIND_CONTINUE_MAX(t8)
    dsrl    t3, 1
    lb      t2, FIND_CONTINUE_LSB(t8)
    daddu   t3, t2
    daddu   t3, 1

#ifdef WR_BIT_TRAINING_DEBUG
    PRINTSTR("\r\nmid=")
    move    a0, t3
    bal     hexserial
    nop
    PRINTSTR("\r\n")
#endif

    daddu   t1, s5, t8
    sb      t3, MID_STORE_BIT0(t1)

    dli     t0, 0
1:
    dsrl    t1, t0, 4
    daddu   t1, t8
    sd      $0, WRDQS_DLL_RESULT_BIT0(t1)
    sd      $0, WRDQS_BDLY_RESULT_BIT0(t1)
    daddu   t0, 1
    bltu    t0, REPEAT_TIMES, 1b
    nop

    daddu   s5, 1
    bleu    s5, 7, wr_training_bit_loop
    nop


    dli     s5, 0
    dli     t7, 0
find_max_wr_mid:
    daddu   t1, s5, t8
    lb      t3, MID_STORE_BIT0(t1)

    bleu    t3, t7, 1f
    nop
    move    t7, t3
1:
    daddu   s5, 1
    bleu    s5, 7, find_max_wr_mid
    nop

#ifdef WR_BIT_TRAINING_DEBUG
    PRINTSTR("\r\nmax mid is")
    move    a0, t7
    bal     hexserial
    nop
#endif

    lb      t0, DLL_VALUE_OFFSET(t8)
    dsll    t1, t7, 7   //t1=t7*128
    ddivu   t0, t1, t0
    dsll    t1, s6, 7   //t1=s6*0x80
    daddu   t1, t1, t8
    sb      t0, DLL_WRDQS_OFFSET(t1)
    dli     t2, 0
    sb      t2, WRDQS0_BDLY_OFFSET(t1)

//wrdq set bit loop
    dli     t0, 0
    dsll    t3, s6, 7   //t3=s6*0x80
    daddu   t3, t3, t8
wrdq_set_loop:
    daddu   t1, t0, t8
    lb      t2, MID_STORE_BIT0(t1)
    dsubu   t2, t7, t2
    bgeu    t2, 0x10, 1f
    nop
    daddu   t1, t3, t0
    sb      t2, (WRDQ_BDLY00_OFFSET)(t1)
    b       2f
    nop
1:
    PRINTSTR("\r\nERROR: bit dly value for wrdq ")
    move    a0, t0
    bal     hexserial
    nop
    PRINTSTR("\r\n exceed max value\r\n")
2:
    daddu   t0, 1
    bleu    t0, 7, wrdq_set_loop
    nop

    daddu   s6, 1
    GET_DIMM_WIDTH_V1
    bgtu    a1, 1, 1f
    nop
    dli     t0, 2
    b       2f
    nop
1:
    bgtu    a1, 2, 1f
    nop
    dli     t0, 4
    b       2f
    nop
1:
    dli     t0, 8
    lb      t2, 0x1284(t8)
    beqz    t2, 2f
    nop
    daddiu  t0, t0, 0x1  //num of dataslice with ecc
2:
    bltu    s6, t0, wr_bit_slice_loop
    nop
    PRINTSTR("\r\n")

    jr      t9
    .end    wr_bit_training
