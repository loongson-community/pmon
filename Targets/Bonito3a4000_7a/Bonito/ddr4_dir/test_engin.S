/**********************************
    Author: dongji
    Date:   20191025
    ddr_test_engin for 3A4000
    v1.0
    input:
    0x1140: base address
    0x1148: valid bits
    0x1150: page size(byte)
    0x1154: page number(word)
    output:
    v0--Success flag: 0--fail; 1--success
**********************************/
//#define  DEBUG_TEST_ENGINE

//#define  PAGE_SIZE        0x1 //3bit
#define  BUS_WIDTH        0x3 //2bit: 0-8bit;1-16bit;2-32bit;3-64bit
#define  FIX_PATTEN_INDEX 0x9 //5bit: bit4(0/1-addr inc/dec) bit3:0-pattern sel
//#define  BASE_ADDR        0x900000000ff00000 //64bit, here only use low 32bit
#define  USR_PATTERN      0x5555555555555555//64bit
//#define  VALID_BITS       0xffffffffffffffff//64bit
#define  START            0x1
#define  SKIP_INIT        0x0
#define  STOP_WR          0x0
#define  STOP_RD          0x0
#define  STOP_WHEN_FINISH 0x1
#define  CLOSE_COMPARE    0x0
#define  INV_USR_PATTERN  0x1
#define  FIX_PATTERN_EN   0x1   //0-random 1-fixed
#define  STOP_WHEN_ERR    0x1
#define  AUTO_START       0x0
#define  RD_WORKER_NUM    0x0   //0-1threde 1-2threde 2-3threds 3-4thredes
#define  WR_WORKER_NUM    0x0   //0-1threde 1-2threde 2-3threds 3-4thredes
//#define  PAGE_NUM         0x800//32bit, actuall page num should *256
#define  READ_INTERVAL    0x0f0f0f0f
#define  WRITE_INTERVAL   0x0f0f0f0f

		.text
        .global test_engine
        .ent    test_engine
test_engine:

    move    s7, ra

#ifdef DEBUG_TEST_ENGINE
    PRINTSTR("\r\nddr test engin start\r\n")
#endif
    GET_NODE_ID_a0
    dli     t8, 0x900000000ff00000
    or      t8, a0
    sd      $0, 0x3100(t8)
    sd      $0, 0x3120(t8)
    sd      $0, 0x3130(t8)
    sd      $0, 0x3140(t8)
    sd      $0, 0x3148(t8)
    sd      $0, 0x3150(t8)
    sd      $0, 0x3158(t8)
    ld      t0, 0x3150(t8)
    ld      t0, 0x3150(t8)
    ld      t0, 0x3150(t8)

    dli     t0, 0x0
    dli     t1, 1
    or      t0, t1
    lb      t1, PAGE_SIZE_OFFSET(t8)
//    li      t1, PAGE_SIZE
    sll     t1, t1, 8
    or      t0, t0, t1
    li      t1, BUS_WIDTH
    sll     t1, t1, 16
    or      t0, t0, t1
    li      t1, FIX_PATTEN_INDEX
    sll     t1, t1, 24
    or      t0, t0, t1
    sw      t0, 0x3100(t8)
//    dli     t0, BASE_ADDR
    ld      t0, BASE_ADDR_OFFSET(t8)
    sd      t0, 0x3120(t8)
    dli     t0, USR_PATTERN
    sd      t0, 0x3130(t8)
//    dli     t0, VALID_BITS
    ld      t0, VALID_BITS_OFFSET(t8)
    sd      t0, 0x3140(t8)
    lb      t0, VALID_BITS_ECC_OFFSET(t8)
    sb      t0, 0x3148(t8)
    ld      t0, 0x1230(t8)
    sd      t0, 0x3110(t8)
//    li      t0, PAGE_NUM
    lw      t0, PAGE_NUM_OFFSET(t8)
    sw      t0, 0x3154(t8)
    li      t0, READ_INTERVAL
    sw      t0, 0x3158(t8)
    li      t0, WRITE_INTERVAL
    sw      t0, 0x315c(t8)

    li      t0, 0x0
    li      t1, SKIP_INIT
    sll     t1, t1, 1
    or      t0, t0, t1
    li      t1, STOP_WR
    sll     t1, t1, 2
    or      t0, t0, t1
    li      t1, STOP_RD
    sll     t1, t1, 3
    or      t0, t0, t1
    li      t1, STOP_WHEN_FINISH
    sll     t1, t1, 4
    or      t0, t0, t1
    li      t1, CLOSE_COMPARE
    sll     t1, t1, 5
    or      t0, t0, t1
    li      t1, INV_USR_PATTERN
    sll     t1, t1, 6
    or      t0, t0, t1
    li      t1, FIX_PATTERN_EN
    sll     t1, t1, 7
    or      t0, t0, t1
    li      t1, STOP_WHEN_ERR
    sll     t1, t1, 8
    or      t0, t0, t1
    li      t1, AUTO_START
    sll     t1, t1, 9
    or      t0, t0, t1
    li      t1, RD_WORKER_NUM
    sll     t1, t1, 16
    or      t0, t0, t1
    li      t1, WR_WORKER_NUM
    sll     t1, t1, 20
    or      t0, t0, t1
    li      t1, 1
    not     t1
    and     t0, t1
    sw      t0, 0x3150(t8)
    sync


#if 0//#ifdef DEBUG_TEST_ENGINE
    PRINTSTR("\r\n")
    dli     t1, 0x1000
1:
    daddu   t6, t8, t1
    ld      t0, 0x0(t6)
    move    a0, t1
    bal     hexserial
    nop
    PRINTSTR(":")
    dsrl    a0, t0, 32
    bal     hexserial
    nop
    PRINTSTR("_")
    move    a0, t0
    bal     hexserial
    nop
    PRINTSTR("\r\n")
    daddu   t1, 8
    bleu    t1, 0x1068, 1b
    nop

    PRINTSTR("\r\n")
    dli     t1, 0x1200
1:
    daddu   t6, t8, t1
    ld      t0, 0x0(t6)
    move    a0, t1
    bal     hexserial
    nop
    PRINTSTR(":")
    dsrl    a0, t0, 32
    bal     hexserial
    nop
    PRINTSTR("_")
    move    a0, t0
    bal     hexserial
    nop
    PRINTSTR("\r\n")
    daddu   t1, 8
    bleu    t1, 0x1250, 1b
    nop


    PRINTSTR("\r\n")
    dli     t1, 0x3100
1:
    daddu   t6, t8, t1
    ld      t0, 0x0(t6)
    move    a0, t1
    bal     hexserial
    nop
    PRINTSTR(":")
    dsrl    a0, t0, 32
    bal     hexserial
    nop
    PRINTSTR("_")
    move    a0, t0
    bal     hexserial
    nop
    PRINTSTR("\r\n")
    daddu   t1, 8
    bleu    t1, 0x3160, 1b
    nop
#endif

    li      t1, 1
    not     t1
    and     t0, t1
    li      t1, START
    sll     t1, t1, 0
    lw      t0, 0x3150(t8)
    or      t0, t0, t1
    sw      t0, 0x3150(t8)

#ifdef DEBUG_TEST_ENGINE
    PRINTSTR("\r\n")
    dli t7, 0
#endif
test_loop:
#if 0//#ifdef DEBUG_TEST_ENGINE
    dli     t0, 1000000
    ddivu   t1, t7, t0
    sync
    mfhi    a0
    move    a0, t7
    bal     hexserial
    nop
    PRINTSTR("\r\n")
    dli     t1, 0x3100
1:
    daddu   t6, t8, t1
    ld      t0, 0x0(t6)
    move    a0, t1
    bal     hexserial
    nop
    PRINTSTR(":")
    dsrl    a0, t0, 32
    bal     hexserial
    nop
    PRINTSTR("_")
    move    a0, t0
    bal     hexserial
    nop
    PRINTSTR("\r\n")
    daddu   t1, 8
    bleu    t1, 0x3160, 1b
    nop
#endif
    ld      t0, 0x3160(t8)
    li      t1, 0x6
    and     a1, t1, t0
    bne     a1, t1, test_loop
    nop
    andi    t1, t0, 0x1
    beqz    t1, test_pass
    nop
    PRINTSTR("")        //for bug
#ifdef DEBUG_TEST_ENGINE
    PRINTSTR("\r\ntest engine fail\r\n")
#endif
    li      v0, 0
    b       test_engine_end
    nop
test_pass:
    PRINTSTR("")        //for bug
#ifdef DEBUG_TEST_ENGINE
    PRINTSTR("\r\ntest engine pass\r\n")
#endif
    li      v0, 1
test_engine_end:
    dli     t1, 0x1000
1:
    dsubu   t1, 1
    bnez    t1, 1b
    nop
    sd      $0, 0x3150(t8)
    sd      $0, 0x3100(t8)
    jr      s7
    nop
    .end    test_engine
