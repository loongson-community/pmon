/*
 * Now the function on 2g_+690e platform couldn't work
 */

	li	t0, 0x20000000
	.set mips64
	dmfc0	t1, COP_0_TLB_PG_MASK, 1
	or	t1, t1, t0
	dmtc0	t1, COP_0_TLB_PG_MASK, 1

	dmfc0	t0, COP_0_CONFIG, 3
	ori	t0, t0, 0x80
	dmtc0	t0, COP_0_CONFIG, 3
	.set mips3

	li	t0, 0xfff000
	mtc0	t0, COP_0_TLB_PG_MASK    # 16MB page

	li	t0, 15

	li	t3,  0xd0000000   # entry_hi
	#dli	t4, (0x00000e0010000000 >> 6)|0x17   # entry_lo, uncached, valid, dirty, global
	li	t4, 0x0e001000
	#li	t4, 0x0efdf000
	.set mips64
	dsll	t4, t4, 10
	.set mips3
	ori	t4, t4, 0x17

	li	t5, (0x1000000>>6)            # 16M stride
	li	t6, 0x2000000             # VPN2 32M stride

	.set mips64
1:
	dmtc0	t3, COP_0_TLB_HI
	daddu	t3, t3, t6

	dmtc0	t4, COP_0_TLB_LO0
	daddu	t4, t4, t5
	dmtc0	t4, COP_0_TLB_LO1
	daddu	t4, t4, t5

	.set mips3

	mtc0	t0, COP_0_TLB_INDEX           # 16MB page
	nop
	nop
	nop
	nop
	nop
	tlbwi                             # random++

	bnez	t0, 1b
	addiu	t0, t0, -1

###################################### Video RAM mapping ##########

	li	t0, 15

	li	t3,  0xf0000000   # entry_hi
	#dli	t4, (0x0000000070000000 >> 6)|0x1f   # entry_lo, cached, valid, dirty, global

#if 1
    li      a0, 0x3f000000
    bleu    msize, a0, 1f
    nop
	li	t4, 0x0000f000
    b   2f
    nop
#endif
1:
	li	t4, 0x00007000
2:
	.set mips64
	dsll	t4, t4, 10
	.set mips3
	ori	t4, t4, 0x1f

	li	t5, (0x1000000>>6)            # 16M stride
	li	t6, 0x2000000             # VPN2 32M stride

	.set mips64
1:
	dmtc0	t3, COP_0_TLB_HI
	daddu	t3, t3, t6

	dmtc0	t4, COP_0_TLB_LO0
	daddu	t4, t4, t5
	dmtc0	t4, COP_0_TLB_LO1
	daddu	t4, t4, t5

	.set mips3

	addiu	t1, t0, 16
	mtc0	t1, COP_0_TLB_INDEX           # 16MB page
	nop
	nop
	nop
	nop
	nop
	tlbwi

	bnez	t0, 1b
	addiu	t0, t0, -1



