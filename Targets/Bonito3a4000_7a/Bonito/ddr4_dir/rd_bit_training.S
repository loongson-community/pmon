#define _RDDQS_DLL_NUM 64

#define BIT0_TMP_REG_LO    0x3300
#define BIT1_TMP_REG_LO    0x3308
#define BIT2_TMP_REG_LO    0x3310
#define BIT3_TMP_REG_LO    0x3318
#define BIT4_TMP_REG_LO    0x3320
#define BIT5_TMP_REG_LO    0x3328
#define BIT6_TMP_REG_LO    0x3330
#define BIT7_TMP_REG_LO    0x3338
#define BIT8_TMP_REG_LO    0x3340
#define BIT0_TMP_REG_HI    0x3304
#define BIT1_TMP_REG_HI    0x330c
#define BIT2_TMP_REG_HI    0x3314
#define BIT3_TMP_REG_HI    0x331c
#define BIT4_TMP_REG_HI    0x3324
#define BIT5_TMP_REG_HI    0x332c
#define BIT6_TMP_REG_HI    0x3334
#define BIT7_TMP_REG_HI    0x333c
#define BIT8_TMP_REG_HI    0x3344

#define COUNT_REG          0x3350
#define TMP_REG0           0x3360
#define TMP_REG1           0x3368
#define TMP_REG2           0x3370
#define TMP_REG3           0x3378

#define WAITING_TIME    128

#define     PAGE_SIZE               0x0
#define     PAGE_NUMBER             0x1
        .text
        .set    noreorder
        .set    mips3


####### start read training ##################
        .global rd_bit_training
        .ent    rd_bit_training
rd_bit_training:
     move   a0, ra
     sd     a0, TMP_REG2(t8)

     dli    a0, 0x0
     sd     a0, TMP_REG0(t8)
     sd     a0, TMP_REG1(t8)
     dli    t4, 0x0
next_ds:


PRINTSTR("\r\nnow train dataslice:")
move a0, t4
bal hexserial
nop
PRINTSTR("...")

     dli    a0, 0x0
     sd     a0, TMP_REG0(t8)
     sd     a0, TMP_REG1(t8)
     dsll   t5, t4, 7     //data slice sel
     dadd   t5, t5, t8
     dli    s5, 0x1ff     //first test dqs_p:[7:0]min,[8]p\n index
     lb     a0, 0xd(t8)
     bne    a0, 3, 1f
     nop
     dli    s6, 0xff
1:
retest_for_n:
     dli    a0, 0x0
     sd     a0, BIT0_TMP_REG_LO(t8)
     sd     a0, BIT1_TMP_REG_LO(t8)
     sd     a0, BIT2_TMP_REG_LO(t8)
     sd     a0, BIT3_TMP_REG_LO(t8)
     sd     a0, BIT4_TMP_REG_LO(t8)
     sd     a0, BIT5_TMP_REG_LO(t8)
     sd     a0, BIT6_TMP_REG_LO(t8)
     sd     a0, BIT7_TMP_REG_LO(t8)
     sd     a0, BIT8_TMP_REG_LO(t8)
     sb     $0, RDQSP_BDLY00_OFFSET(t5)
     sb     $0, RDQSP_BDLY01_OFFSET(t5)
     sb     $0, RDQSP_BDLY02_OFFSET(t5)
     sb     $0, RDQSP_BDLY03_OFFSET(t5)
     sb     $0, RDQSP_BDLY04_OFFSET(t5)
     sb     $0, RDQSP_BDLY05_OFFSET(t5)
     sb     $0, RDQSP_BDLY06_OFFSET(t5)
     sb     $0, RDQSP_BDLY07_OFFSET(t5)
     sb     $0, RDQSN_BDLY00_OFFSET(t5)
     sb     $0, RDQSN_BDLY01_OFFSET(t5)
     sb     $0, RDQSN_BDLY02_OFFSET(t5)
     sb     $0, RDQSN_BDLY03_OFFSET(t5)
     sb     $0, RDQSN_BDLY04_OFFSET(t5)
     sb     $0, RDQSN_BDLY05_OFFSET(t5)
     sb     $0, RDQSN_BDLY06_OFFSET(t5)
     sb     $0, RDQSN_BDLY07_OFFSET(t5)
     dli    t9, 0x0
rddqs_dll_num_loop:
     dli    a0, 0x80     //bypass mode 0x80
     move   a1, t9
     or     a0, a1, a0
     sb     a0, DLL_RDDQS0_OFFSET(t5)
     lb     a1, 0xd(t8)
     bne    a1, 3, 1f
     nop
     sb     a0, DLL_RDDQS1_OFFSET(t5)
1:

//wait
     dli    t6, WAITING_TIME
1:
     daddiu t6, t6, -0x1
     bnez   t6, 1b
     nop

     sw     $0, COUNT_REG(t8)
     dli    t7, 0x0
retest_stable1:
     dli     t0, 0       //base address
     sd      t0, BASE_ADDR_OFFSET(t8)
     dli     t0, 0xff
     beq     t4, 8, 1f
     nop
     dsll    t1, t4, 3
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

     dli     v1, 0x0
     bnez    v0, rddqs_test_pass     //test sucess
     nop
     dli     a0, 8
     beq     t4, a0, ecc_conf0
     nop
     dsll    t6, t4, 3
     ld      t0, 0x3180(t8)
     dsrl    t0, t0, t6
     andi    t0, t0, 0xff
     or      v1, v1, t0
     ld      t0, 0x3188(t8)
     dsrl    t0, t0, t6
     andi    t0, t0, 0xff
     or      v1, v1, t0
     ld      t0, 0x31a0(t8)
     dsrl    t0, t0, t6
     andi    t0, t0, 0xff
     or      v1, v1, t0
     ld      t0, 0x31a8(t8)
     dsrl    t0, t0, t6
     andi    t0, t0, 0xff
     or      v1, v1, t0

     ld      t0, 0x3180(t8)
     ld      t1, 0x3188(t8)
     dsrl    t0, t0, t6
     andi    t0, t0, 0xff
     dsrl    t1, t1, t6
     andi    t1, t1, 0xff
     or      t0, t1, t0
     dsll    t0, t0, 8
     or      v1, t0, v1

     ld      t0, 0x31a0(t8)
     ld      t1, 0x31a8(t8)
     dsrl    t0, t0, t6
     andi    t0, t0, 0xff
     dsrl    t1, t1, t6
     andi    t1, t1, 0xff
     or      t0, t1, t0
     dsll    t0, t0, 16
     or      v1, t0, v1
     b       1f
     nop

ecc_conf0:
     lb      t1, 0x316e(t8)
     lb      t2, 0x316f(t8)
     or      t1, t2
     lb      t2, 0x31b2(t8)
     or      t1, t2
     lb      t2, 0x31b3(t8)
     or      t1, t2
     andi    t1, t1, 0xff
     or      v1, v1, t1

     lb      t1, 0x316e(t8)
     lb      t2, 0x316f(t8)
     or      t1, t2, t1
     andi    t1, t1, 0xff
     dsll    t1, t1, 8
     or      v1, t1, v1
     lb      t1, 0x31b2(t8)
     lb      t2, 0x31b3(t8)
     or      t1, t2, t1
     andi    t1, t1, 0xff
     dsll    t1, t1, 16
     or      v1, t1, v1
1:
rddqs_test_pass:
     not    v1, v1

     dli    v0, 0x100
     and    t0, s5, v0
     dli    t1, 8
     beq    v0, t0, 3f
     nop
     dli    t1, 16
3:

     dsrl   t0, v1, t1
     dli    t6, 0x0
2:
     dsrl   t1, t0, t6
     andi   t1, t1, 0x1
     beqz   t1, 1f
     nop
     dsll   a0, t6, 3
     dsll   t1, t1, a0
     daddu  t7, t7, t1
1:
     daddiu t6, t6, 0x1
     dli    a0, 0x8
     bne    t6, a0, 2b
     nop
     lw     a1, COUNT_REG(t8)
     daddiu a1, a1, 0x1
     sw     a1, COUNT_REG(t8)
     andi   a1, a1, 0xff
     dli    a0, 10     //trianing times
     bne    a0, a1, retest_stable1
     nop

     dli    v1, 0x0
     dli    t6, 0x0
bit_reorder:
     move   t0, t7
     andi   t0, t0, 0xff
     dli    a0, 8     //bit threshold
     bltu   t0, a0, 1f
     nop
     dli    a0, 0x1
     dsll   a0, a0, t6
     or     v1, v1, a0
1:
     dsrl   t7, t7, 0x8
     daddiu t6, t6, 0x1
     dli    a0, 0x8
     bne    a0, t6, bit_reorder
     nop

     move   v0, v1
     andi   v0, v0, 0x1
     dli    t6, 63
     dsubu  t6, t6, t9
     dsll   v0, v0, t6
     ld     t6, BIT0_TMP_REG_LO(t8)
     or     t6, t6, v0
     sd     t6, BIT0_TMP_REG_LO(t8)
     dsrl   v1, v1, 0x1

     move   v0, v1
     andi   v0, v0, 0x1
     dli    t6, 63
     dsubu  t6, t6, t9
     dsll   v0, v0, t6
     ld     t6, BIT1_TMP_REG_LO(t8)
     or     t6, t6, v0
     sd     t6, BIT1_TMP_REG_LO(t8)
     dsrl   v1, v1, 0x1

     move   v0, v1
     andi   v0, v0, 0x1
     dli    t6, 63
     dsubu  t6, t6, t9
     dsll   v0, v0, t6
     ld     t6, BIT2_TMP_REG_LO(t8)
     or     t6, t6, v0
     sd     t6, BIT2_TMP_REG_LO(t8)
     dsrl   v1, v1, 0x1

     move   v0, v1
     andi   v0, v0, 0x1
     dli    t6, 63
     dsubu  t6, t6, t9
     dsll   v0, v0, t6
     ld     t6, BIT3_TMP_REG_LO(t8)
     or     t6, t6, v0
     sd     t6, BIT3_TMP_REG_LO(t8)
     dsrl   v1, v1, 0x1

     move   v0, v1
     andi   v0, v0, 0x1
     dli    t6, 63
     dsubu  t6, t6, t9
     dsll   v0, v0, t6
     ld     t6, BIT4_TMP_REG_LO(t8)
     or     t6, t6, v0
     sd     t6, BIT4_TMP_REG_LO(t8)
     dsrl   v1, v1, 0x1

     move   v0, v1
     andi   v0, v0, 0x1
     dli    t6, 63
     dsubu  t6, t6, t9
     dsll   v0, v0, t6
     ld     t6, BIT5_TMP_REG_LO(t8)
     or     t6, t6, v0
     sd     t6, BIT5_TMP_REG_LO(t8)
     dsrl   v1, v1, 0x1

     move   v0, v1
     andi   v0, v0, 0x1
     dli    t6, 63
     dsubu  t6, t6, t9
     dsll   v0, v0, t6
     ld     t6, BIT6_TMP_REG_LO(t8)
     or     t6, t6, v0
     sd     t6, BIT6_TMP_REG_LO(t8)
     dsrl   v1, v1, 0x1

     move   v0, v1
     andi   v0, v0, 0x1
     dli    t6, 63
     dsubu  t6, t6, t9
     dsll   v0, v0, t6
     ld     t6, BIT7_TMP_REG_LO(t8)
     or     t6, t6, v0
     sd     t6, BIT7_TMP_REG_LO(t8)

     daddiu t9, t9, 0x1
     dli    a0, 64
     bne    a0, t9, rddqs_dll_num_loop
     nop

     PRINTSTR("\r\ntest rddqs_dll vector:\r\n")
     lw     a0, BIT0_TMP_REG_HI(t8)
     bal    hexserial
     nop
     lw     a0, BIT0_TMP_REG_LO(t8)
     bal    hexserial
     nop
     PRINTSTR("\r\n")
     lw     a0, BIT1_TMP_REG_HI(t8)
     bal    hexserial
     nop
     lw     a0, BIT1_TMP_REG_LO(t8)
     bal    hexserial
     nop
     PRINTSTR("\r\n")
     lw     a0, BIT2_TMP_REG_HI(t8)
     bal    hexserial
     nop
     lw     a0, BIT2_TMP_REG_LO(t8)
     bal    hexserial
     nop
     PRINTSTR("\r\n")
     lw     a0, BIT3_TMP_REG_HI(t8)
     bal    hexserial
     nop
     lw     a0, BIT3_TMP_REG_LO(t8)
     bal    hexserial
     nop
     PRINTSTR("\r\n")
     lw     a0, BIT4_TMP_REG_HI(t8)
     bal    hexserial
     nop
     lw     a0, BIT4_TMP_REG_LO(t8)
     bal    hexserial
     nop
     PRINTSTR("\r\n")
     lw     a0, BIT5_TMP_REG_HI(t8)
     bal    hexserial
     nop
     lw     a0, BIT5_TMP_REG_LO(t8)
     bal    hexserial
     nop
     PRINTSTR("\r\n")
     lw     a0, BIT6_TMP_REG_HI(t8)
     bal    hexserial
     nop
     lw     a0, BIT6_TMP_REG_LO(t8)
     bal    hexserial
     nop
     PRINTSTR("\r\n")
     lw     a0, BIT7_TMP_REG_HI(t8)
     bal    hexserial
     nop
     lw     a0, BIT7_TMP_REG_LO(t8)
     bal    hexserial
     nop

//compress
     move   t1, t8
     dli    t2, 0x0
nxt_bit_cpr32:
     ld     t6, BIT0_TMP_REG_LO(t1)
     sd     $0, BIT0_TMP_REG_LO(t1)
     dli    t7, 0x0
     dli    t0, 0x0
1:
     dsll   a0, t7, 0x1
     dsrl   a0, t6, a0
     dsrl   a1, a0, 0x1
     and    a1, a0, a1
     andi   a1, a1, 0x1
     beqz   a1, 2f
     nop
     dsll   a1, a1, t7
     or     t0, t0, a1
2:
     daddiu t7, t7, 0x1
     dli    a0, 32
     bne    t7, a0, 1b
     nop
     sw     t0, BIT0_TMP_REG_LO(t1)

     daddiu t2, t2, 0x1
     daddu  t1, t1, 0x8
     dli    a0, 8
     bne    t2, a0, nxt_bit_cpr32
     nop
//end cpr

     dli    t9, 0x0
rddqs_bdly_num_loop:
     sb     t9, RDQSP_BDLY00_OFFSET(t5)
     sb     t9, RDQSP_BDLY01_OFFSET(t5)
     sb     t9, RDQSP_BDLY02_OFFSET(t5)
     sb     t9, RDQSP_BDLY03_OFFSET(t5)
     sb     t9, RDQSP_BDLY04_OFFSET(t5)
     sb     t9, RDQSP_BDLY05_OFFSET(t5)
     sb     t9, RDQSP_BDLY06_OFFSET(t5)
     sb     t9, RDQSP_BDLY07_OFFSET(t5)
     sb     t9, RDQSN_BDLY00_OFFSET(t5)
     sb     t9, RDQSN_BDLY01_OFFSET(t5)
     sb     t9, RDQSN_BDLY02_OFFSET(t5)
     sb     t9, RDQSN_BDLY03_OFFSET(t5)
     sb     t9, RDQSN_BDLY04_OFFSET(t5)
     sb     t9, RDQSN_BDLY05_OFFSET(t5)
     sb     t9, RDQSN_BDLY06_OFFSET(t5)
     sb     t9, RDQSN_BDLY07_OFFSET(t5)

     dli    a0, WAITING_TIME
1:
     daddiu a0, a0, -0x1
     bnez   a0, 1b
     nop

     dli     t0, 0       //base address
     sd      t0, BASE_ADDR_OFFSET(t8)
     dli     t0, 0xff
     beq     t4, 8, 1f
     nop
     dsll    t1, t4, 3
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
     dli     v1, 0x0
     bnez    v0, rddqs_test_pass1     //test sucess
     nop

     beq     t4, 8, ecc_conf1
     nop
     dsll    t6, t4, 3
     ld      t0, 0x3180(t8)
     dsrl    t0, t0, t6
     andi    t0, t0, 0xff
     or      v1, v1, t0
     ld      t0, 0x3188(t8)
     dsrl    t0, t0, t6
     andi    t0, t0, 0xff
     or      v1, v1, t0
     ld      t0, 0x31a0(t8)
     dsrl    t0, t0, t6
     andi    t0, t0, 0xff
     or      v1, v1, t0
     ld      t0, 0x31a8(t8)
     dsrl    t0, t0, t6
     andi    t0, t0, 0xff
     or      v1, v1, t0

     ld      t0, 0x3180(t8)
     ld      t0, 0x3188(t8)
     dsrl    t0, t0, t6
     andi    t0, t0, 0xff
     dsrl    t1, t1, t6
     andi    t1, t1, 0xff
     or      t0, t1, t0
     dsll    t0, t0, 8
     or      v1, t0, v1

     ld      t0, 0x31a0(t8)
     ld      t1, 0x31a8(t8)
     dsrl    t0, t0, t6
     andi    t0, t0, 0xff
     dsrl    t1, t1, t6
     andi    t1, t1, 0xff
     or      t0, t1, t0
     dsll    t0, t0, 16
     or      v1, t0, v1
     b       1f
     nop

ecc_conf1:
     lb      t1, 0x316e(t8)
     lb      t2, 0x316f(t8)
     or      t1, t2
     lb      t2, 0x31b2(t8)
     or      t1, t2
     lb      t2, 0x31b3(t8)
     or      t1, t2
     andi    t1, t1, 0xff
     or      v1, v1, t1

     lb      t1, 0x316e(t8)
     lb      t2, 0x316f(t8)
     or      t1, t2, t1
     andi    t1, t1, 0xff
     dsll    t1, t1, 8
     or      v1, t1, v1
     lb      t1, 0x31b2(t8)
     lb      t2, 0x31b3(t8)
     or      t1, t2, t1
     andi    t1, t1, 0xff
     dsll    t1, t1, 16
     or      v1, t1, v1
1:
rddqs_test_pass1:
     not    v1, v1

     dli    v0, 0x100
     and    t0, s5, v0
     dli    t2, 8
     beq    v0, t0, 1f
     nop
     dli    t2, 16
1:
     dsrl   v0, v1, t2
     andi   v0, v0, 0x1
     dli    t6, 31
     dsubu  t6, t6, t9
     dsll   v0, v0, t6
     lw     t6, BIT0_TMP_REG_HI(t8)
     or     t6, t6, v0
     sw     t6, BIT0_TMP_REG_HI(t8)
     daddiu t2, t2, 1

     dsrl   v0, v1, t2
     andi   v0, v0, 0x1
     dli    t6, 31
     dsubu  t6, t6, t9
     dsll   v0, v0, t6
     lw     t6, BIT1_TMP_REG_HI(t8)
     or     t6, t6, v0
     sw     t6, BIT1_TMP_REG_HI(t8)
     daddiu t2, t2, 1

     dsrl   v0, v1, t2
     andi   v0, v0, 0x1
     dli    t6, 31
     dsubu  t6, t6, t9
     dsll   v0, v0, t6
     lw     t6, BIT2_TMP_REG_HI(t8)
     or     t6, t6, v0
     sw     t6, BIT2_TMP_REG_HI(t8)
     daddiu t2, t2, 1

     dsrl   v0, v1, t2
     andi   v0, v0, 0x1
     dli    t6, 31
     dsubu  t6, t6, t9
     dsll   v0, v0, t6
     lw     t6, BIT3_TMP_REG_HI(t8)
     or     t6, t6, v0
     sw     t6, BIT3_TMP_REG_HI(t8)
     daddiu t2, t2, 1

     dsrl   v0, v1, t2
     andi   v0, v0, 0x1
     dli    t6, 31
     dsubu  t6, t6, t9
     dsll   v0, v0, t6
     lw     t6, BIT4_TMP_REG_HI(t8)
     or     t6, t6, v0
     sw     t6, BIT4_TMP_REG_HI(t8)
     daddiu t2, t2, 1

     dsrl   v0, v1, t2
     andi   v0, v0, 0x1
     dli    t6, 31
     dsubu  t6, t6, t9
     dsll   v0, v0, t6
     lw     t6, BIT5_TMP_REG_HI(t8)
     or     t6, t6, v0
     sw     t6, BIT5_TMP_REG_HI(t8)
     daddiu t2, t2, 1

     dsrl   v0, v1, t2
     andi   v0, v0, 0x1
     dli    t6, 31
     dsubu  t6, t6, t9
     dsll   v0, v0, t6
     lw     t6, BIT6_TMP_REG_HI(t8)
     or     t6, t6, v0
     sw     t6, BIT6_TMP_REG_HI(t8)
     daddiu t2, t2, 1

     dsrl   v0, v1, t2
     andi   v0, v0, 0x1
     dli    t6, 31
     dsubu  t6, t6, t9
     dsll   v0, v0, t6
     lw     t6, BIT7_TMP_REG_HI(t8)
     or     t6, t6, v0
     sw     t6, BIT7_TMP_REG_HI(t8)

     daddiu t9, t9, 0x1
     dli    a0, 16
     bne    a0, t9, rddqs_bdly_num_loop
     nop

     PRINTSTR("\r\ntest bly_dll vector:\r\n")
     lw     a0, BIT0_TMP_REG_HI(t8)
     bal    hexserial
     nop
     PRINTSTR("\r\n")
     lw     a0, BIT1_TMP_REG_HI(t8)
     bal    hexserial
     nop
     PRINTSTR("\r\n")
     lw     a0, BIT2_TMP_REG_HI(t8)
     bal    hexserial
     nop
     PRINTSTR("\r\n")
     lw     a0, BIT3_TMP_REG_HI(t8)
     bal    hexserial
     nop
     PRINTSTR("\r\n")
     lw     a0, BIT4_TMP_REG_HI(t8)
     bal    hexserial
     nop
     PRINTSTR("\r\n")
     lw     a0, BIT5_TMP_REG_HI(t8)
     bal    hexserial
     nop
     PRINTSTR("\r\n")
     lw     a0, BIT6_TMP_REG_HI(t8)
     bal    hexserial
     nop
     PRINTSTR("\r\n")
     lw     a0, BIT7_TMP_REG_HI(t8)
     bal    hexserial
     nop

//compress
     move   t1, t8
     dli    t2, 0x0
nxt_bit_cpr8:
     lw     t6, BIT0_TMP_REG_HI(t1)
     sw     $0, BIT0_TMP_REG_HI(t1)
     dli    t7, 0x0
     dli    t0, 0x0
1:
     dsll   a0, t7, 0x1
     dsrl   a0, t6, a0
     dsrl   a1, a0, 0x1
     and    a1, a0, a1
     andi   a1, a1, 0x1
     beqz   a1, 2f
     nop
     dsll   a1, a1, t7
     or     t0, t0, a1
2:
     daddiu t7, t7, 0x1
     dli    a0, 16
     bne    t7, a0, 1b
     nop
     dsll   t0, t0, 16
     sw     t0, BIT0_TMP_REG_HI(t1)
     daddiu t2, t2, 0x1
     daddiu t1, t1, 0x8
     dli    a0, 0x8
     bne    t2, a0, nxt_bit_cpr8
     nop
//end cpr

     dli    t9, 0x0
bit_one_by_one:
     dsll   t0, t9, 3
     dadd   t0, t0, t8
     ld     v0, BIT0_TMP_REG_LO(t0)
     dli    a0, 0xffffffff   //bao
     and    v0, v0, a0

     bal    find_continues_1
     nop
     dsrl   v0, v0, 16
     andi   v0, v0, 0xff
     dsll   v0, v0, 0x1  //uncompress

     dsll   t6, t9, 3
     dsll   t6, v0, t6

     dli    a0, 0x100
     and    t0, s5, a0
     bne    a0, t0, record_n_param
     nop
     ld     a0, TMP_REG1(t8)
     or     a0, a0, t6
     sd     a0, TMP_REG1(t8)
     b      1f
     nop
record_n_param:
     ld     a0, TMP_REG0(t8)
     or     a0, a0, t6
     sd     a0, TMP_REG0(t8)
1:
     lb     a0, 0xd(t8)
     bne    a0, 3, 2f
     nop
     bleu   t9, 3, 2f
     nop
     andi   t0, s6, 0xff
     bleu   t0, v0, 1f
     nop
     dli    t0, 0xff
     not    t0, t0
     and    s6, s6, t0
     or     s6, s6, v0
1:
     b      1f
     nop
2:
     andi   t0, s5, 0xff
     bleu   t0, v0, 1f
     nop
     dli    t0, 0xff
     not    t0, t0
     and    s5, s5, t0
     or     s5, s5, v0
1:
     daddiu t9, t9, 0x1
     dli    a0, 0x8
     bne    t9, a0, bit_one_by_one
     nop

     dli    a0, 0x100
     and    t0, s5, a0
     bne    a0, t0, 1f
     nop
     not    a0, a0
     and    s5, s5, a0
     b      retest_for_n
     nop
1:
     andi   s5, 0xff
     lb     t6, DLL_VALUE_OFFSET(t8)
     dsll   a0, s5, 7
     divu   t6, a0, t6
     sb     t6, DLL_RDDQS0_OFFSET(t5)
     lb     a0, 0xd(t8)
     bne    a0, 3, 1f
     nop
     andi   s6, 0xff
     lb     t6, DLL_VALUE_OFFSET(t8)
     dsll   a0, s6, 7
     divu   t6, a0, t6
     sb     t6, DLL_RDDQS1_OFFSET(t5)
1:
     dli    t9, 0x0
bit_one_by_one1:
     dadd   t0, t5, t9
     dsll   t6, t9, 3
     ld     a0, TMP_REG1(t8)
     dsrl   t6, a0, t6
     andi   t6, t6, 0xff
     lb     a0, 0xd(t8)
     bne    a0, 3, 1f
     nop
     bleu   t9, 3, 1f
     nop
     dsubu  t6, t6, s6
     b      2f
     nop
1:
     dsubu  t6, t6, s5
2:
     dli    a0, 0x10
     bgeu   t6, a0, bdly_error
     nop
     sb     t6, RDQSP_BDLY00_OFFSET(t0)
     daddiu t9, t9, 0x1
     dli    a0, 8
     bne    t9, a0, bit_one_by_one1
     nop

     dli    t9, 0x0
bit_one_by_one2:
     dadd   t0, t5, t9
     dsll   t6, t9, 3
     ld     a0, TMP_REG0(t8)
     dsrl   t6, a0, t6
     andi   t6, t6, 0xff
     lb     a0, 0xd(t8)
     bne    a0, 3, 1f
     nop
     bleu   t9, 3, 1f
     nop
     dsubu  t6, t6, s6
     b      2f
     nop
1:
     dsubu  t6, t6, s5
2:
     dli    a0, 0x10
     bgeu   t6, a0, bdly_error
     nop
     sb     t6, RDQSN_BDLY00_OFFSET(t0)
     daddiu t9, t9, 0x1
     dli    a0, 8
     bne    t9, a0, bit_one_by_one2
     nop

     daddiu  t4, t4, 0x1
     dli     t0, 8
     lb      t2, 0x1284(t8)
     beqz    t2, 2f     //no ecc
     nop
     daddiu  t0, t0, 0x1  //num of dataslice with ecc
2:
     bltu    t4, t0, next_ds
     nop

     b      bdly_config_end
     nop
bdly_error:
     PRINTSTR("ERROR: bit dly value for rddqs exceed max value\r\n")

bdly_config_end:

     ld     a0, TMP_REG2(t8)
     move   ra, a0
     jr     ra
     nop
     .end   rd_bit_training

/**********************************
    input:
    v0--error bits
    t3--MC select: 0--MC0; 1--MC1
    t4--dataslice num
    output:
    v0--return mid num
**********************************/
LEAF(find_continues_1)
    dli     t7, 0x0    //[7:0]cnt,[15:8]max[23:16]lsb
    dli     t6, 0x0    //check current bit
    dli     t2, 0x0   //[31:0]tmp check bits [32]hi,lo bits sel
    dli     a0, 0xffffffff
    and     a0, a0, v0
    or      t2, t2, a0
    b       next_vectorbit
    nop
check_hidata:
    dli     a0, 0x1
    dsll    a0, a0, 32
    or      t2, t2, a0
    dli     a0, 0x100000000
    and     t2, t2, a0
    dsrl    a0, v0, 32
    or      t2, t2, a0
    dli     t6, 0x0
next_vectorbit:
    dli     a0, 31
    dsubu   a1, a0, t6
    dsrl    v1, t2, a1
    dli     a1, 0x1
    and     v1, v1, a1
    bne     v1, a1, 3f
    nop
    daddu   t7, t7, 0x1
    b       4f
    nop
3:
    andi    v1, t7, 0xff
    dsrl    a1, t7, 8
    and     a1, a1, 0xff
    bleu    v1, a1, 2f     //v1:cnt a1:max
    nop
    dli     a1, 0xffff00ff   //conf max
    and     t7, a1, t7
    dsll    v1, v1, 8
    or      t7, v1, t7
    dsrl    v1, v1, 8
    move    a0, t6
    dsrl    a1, t2, 32
    andi    a1, a1, 0x1
    beqz    a1, lodata
    nop
    daddu   a0, a0, 32
lodata:
    dsubu   v1, a0, v1
    dli     a0, 0xff00ffff
    and     t7, t7, a0
    dsll    v1, v1, 16
    or      t7, v1, t7
2:
    dli     a0, 0xffffff00
    and     t7, t7, a0

4:
    daddiu  t6, t6, 0x1
    dli     a0, 32
    bne     t6, a0, next_vectorbit
    nop
    dsrl    a0, t2, 32
    andi    a0, a0, 0x1
    beqz    a0, check_hidata
    nop

//repeat
    dli     t6, 0x0
    dli     t2, 0x0   //[31:0]tmp check bits [32]hi,lo bits sel
    dli     a0, 0xffffffff
    and     a0, a0, v0
    or      t2, t2, a0
    b       next_vectorbit1
    nop
check_hidata1:
    dli     a0, 0x1
    dsll    a0, a0, 32
    or      t2, t2, a0
    dli     a0, 0x100000000
    and     t2, t2, a0
    dsrl    a0, v0, 32
    or      t2, t2, a0
    dli     t6, 0x0
next_vectorbit1:
    dli     a0, 31
    dsubu   a1, a0, t6
    dsrl    v1, t2, a1
    dli     a1, 0x1
    and     v1, v1, a1
    bne     v1, a1, 3f
    nop
    daddiu  t7, t7, 0x1
    b       4f
    nop
3:
    andi    v1, t7, 0xff
    dsrl    a1, t7, 8
    and     a1, a1, 0xff
    bleu    v1, a1, 2f     //v1:cnt a1:max
    nop
    dli     a1, 0xffff00ff   //conf max
    and     t7, a1, t7
    dsll    v1, v1, 8
    or      t7, v1, t7
    dsrl    v1, v1, 8
    move    a0, t6
    dsrl    a1, t2, 32
    andi    a1, a1, 0x1
    beqz    a1, lodata1
    nop
    daddu   a0, a0, 32
lodata1:
    dsubu   v1, a0, v1
    dli     a0, 0xff00ffff
    and     t7, t7, a0
    dsll    v1, v1, 16
    or      t7, v1, t7
2:
    dli     a0, 0xffffff00
    and     t7, t7, a0

4:
    daddiu  t6, t6, 0x1
    dli     a0, 32
    bne     t6, a0, next_vectorbit1
    nop

    dsrl    a0, t2, 32
    andi    a0, a0, 0x1
    beqz    a0, check_hidata1
    nop

    dli     a1, 0x80
    dli     a0, 0xff
    and     a0, t7, a0
    bne     a0, a1, 5f
    nop
    dli     t7, 0x0
    dsll    t7, a1, 8
5:
    dsrl    t7, t7, 8
    dli     v1, 0xff
    and     a1, t7, v1
    dli     v0, 0x0
    or      v0, a1, v0
    dsrl    t7, t7, 8
    and     v1, t7, v1
    dsrl    a1, a1, 1
    daddu   a1, a1, v1
    dsll    a1, a1, 16
    dsll    v1, v1, 8
    or      v0, v1, v0
    or      v0, a1, v0  //[7:0]max,[15:8]lsb,[23:16]mid

    jr      ra
    nop
END(find_continues_1)

/*********************
input:
    v0--loop times
    a0--base addr
*********************/
LEAF(mem_set)
1:
    dli     t2, 6
    dsll    t2, v0, t2
    dadd    a0, t2, a0
    sd      $0, 0x00(a0)
    sd      $0, 0x08(a0)
    sd      $0, 0x10(a0)
    sd      $0, 0x18(a0)
    sd      $0, 0x20(a0)
    sd      $0, 0x28(a0)
    sd      $0, 0x30(a0)
    sd      $0, 0x38(a0)
    daddiu  v0, v0, -0x1
    bnez    v0, 1b
    nop
    jr      ra
    nop
END(mem_set)

/*********************
input:
    v0--loop times
    a0--base addr
output:
    v1--[32:16]err bit N;[15:8]err bit P;[7:0]all
*********************/
LEAF(mem_test)
    dli     t7, 0x3
    dsll    t7, t4, t7
    dli     t6, 0x0
    dli     v1, 0x0

repeat_test:
    dli     t2, 6
    dsll    t2, t6, t2
    dadd    a0, t2, a0
    dli     t2, 0x5555555555555555
    sd      t2, 0x00(a0)
    dli     t2, 0xaaaaaaaaaaaaaaaa
    sd      t2, 0x08(a0)
    dli     t2, 0x3333333333333333
    sd      t2, 0x10(a0)
    dli     t2, 0xcccccccccccccccc
    sd      t2, 0x18(a0)
    dli     t2, 0x7777777777777777
    sd      t2, 0x20(a0)
    dli     t2, 0x8888888888888888
    sd      t2, 0x28(a0)
    dli     t2, 0x1111111111111111
    sd      t2, 0x30(a0)
    dli     t2, 0xeeeeeeeeeeeeeeee
    sd      t2, 0x38(a0)

    ld      t2, 0x00(a0)
    dsrl    t2, t2, t7
    andi    t2, t2, 0xff
    dli     t0, 0x55
    xor     t2, t2, t0
    or      v1, v1, t2
    dsll    t2, t2, 0x8
    or      v1, v1, t2

    ld      t2, 0x10(a0)
    dsrl    t2, t2, t7
    andi    t2, t2, 0xff
    dli     t0, 0x33
    xor     t2, t2, t0
    or      v1, v1, t2
    dsll    t2, t2, 0x8
    or      v1, v1, t2

    ld      t2, 0x20(a0)
    dsrl    t2, t2, t7
    andi    t2, t2, 0xff
    dli     t0, 0x77
    xor     t2, t2, t0
    or      v1, v1, t2
    dsll    t2, t2, 0x8
    or      v1, v1, t2

    ld      t2, 0x30(a0)
    dsrl    t2, t2, t7
    andi    t2, t2, 0xff
    dli     t0, 0x11
    xor     t2, t2, t0
    or      v1, v1, t2
    dsll    t2, t2, 0x8
    or      v1, v1, t2

    ld      t2, 0x08(a0)
    dsrl    t2, t2, t7
    andi    t2, t2, 0xff
    dli     t0, 0xaa
    xor     t2, t2, t0
    or      v1, v1, t2
    dsll    t2, t2, 0x10
    or      v1, v1, t2

    ld      t2, 0x18(a0)
    dsrl    t2, t2, t7
    andi    t2, t2, 0xff
    dli     t0, 0xcc
    xor     t2, t2, t0
    or      v1, v1, t2
    dsll    t2, t2, 0x10
    or      v1, v1, t2

    ld      t2, 0x28(a0)
    dsrl    t2, t2, t7
    andi    t2, t2, 0xff
    dli     t0, 0x88
    xor     t2, t2, t0
    or      v1, v1, t2
    dsll    t2, t2, 0x10
    or      v1, v1, t2

    ld      t2, 0x38(a0)
    dsrl    t2, t2, t7
    andi    t2, t2, 0xff
    dli     t0, 0xee
    xor     t2, t2, t0
    or      v1, v1, t2
    dsll    t2, t2, 0x10
    or      v1, v1, t2

    daddiu  t6, t6, 1
    bne     t6, v0, repeat_test
    nop

    jr      ra
    nop
END(mem_test)
