/*ls132 core code*/


#define LS132_BASE_REG	0xbfe00420

#define LS132_PUTC(x) \
	li a0,x;bal ls132_tgt_putchar;nop

#define	SMC_TTYDBG(x) \
	.rdata;98: .asciz x; .text; la a0, 98b; bal ls132_stringserial; nop

#define LS132_UART 0xbfe001e0 //3a4000 uart 0
#define LS132_MAIL 0xbfe0051c


LEAF(ls132_initserial)
	.set    push
	.set    noreorder
	.set    mips32
	//call this function must give the register addr to a0
	li	a1,128
	sb	a1,3(a0)
	li      a1,0x36		//100M ls3a4000 uart
	//li	a1,0x1b		//50M 7a uart
	sb	a1,0(a0)
	li	a1,0x0		//divider, highest possible baud rate
	sb	a1,1(a0)
	li	a1,3
	sb	a1,3(a0)
	#srl	a1,t1,0x8
	li	a1,0
	sb	a1,1(a0)
	#li	a1,1		//divider, highest possible baud rate
	li	a1,71
	sb	a1,2(a0)
	jr	ra
	nop
	.set pop
END(ls132_initserial)

LEAF(ls132_stringserial)
	.set    push
	.set    noreorder
	.set    mips32
	move	a2, ra
	addu	a1, a0, s0
	lbu	a0, 0(a1)
1:
	beqz	a0, 2f
	nop
	bal	ls132_tgt_putchar
	addiu	a1, 1
	b	1b
	lbu	a0, 0(a1)

2:
	j	a2
	nop
	.set	pop
END(ls132_stringserial)

LEAF(ls132_outstring)
	.set    push
	.set    noreorder
	.set    mips32
	move	a2, ra
	move	a1, a0
	lbu	a0, 0(a1)
1:
	beqz	a0, 2f
	nop
	bal     ls132_tgt_putchar
	addiu	a1, 1
	b	1b
	lbu	a0, 0(a1)

2:
	move	ra, a2
	jr	ra
	nop
	.set	pop
END(ls132_outstring)

LEAF(ls132_hexserial)
	.set    push
	.set    noreorder
	.set    mips32
	move	a2, ra
	move	a1, a0
	li	a3, 7
1:
	li	v0, 10 //it changed in tgt_putchar
	rol	a0, a1, 4
	move	a1, a0
	and	a0, 0xf

	bge	a0, v0, 2f
	nop
	addu	a0, 48
	b  3f
	nop
2:
	addu	a0, 87
3:
	bal	ls132_tgt_putchar
	nop

	bnez	a3, 1b
	addu	a3, -1

	LS132_PUTC('\r')
	LS132_PUTC('\n')

	move	ra, a2
	jr	ra
	nop
	.set pop
END(ls132_hexserial)

LEAF(ls132_tgt_putchar)
	.set    push
	.set    noreorder
	.set    mips32
	li      v0, LS132_UART
1:
	lbu     v1, NSREG(NS16550_LSR)(v0)
	and     v1, LSR_TXRDY
	beqz    v1, 1b
	nop

	sb      a0, NSREG(NS16550_DATA)(v0)
	jr      ra
	nop
	.set pop
END(ls132_tgt_putchar)


LEAF(ls132_core_main)
	.set    push
	.set    noreorder
	.set    mips32

	bal get_s0
	nop
	/* Get Current Offset of Address */
get_s0:
	la	t0, ls132_core_main
	subu	s0, ra, t0
	and	s0, 0xffff0000
	SMC_TTYDBG("\r\nSMC: GS132 UP PC: ")
	move	a0, ra
	bal	ls132_hexserial
	nop

	SMC_TTYDBG("SMC: CAUSE=")
	mfc0	a0, COP_0_CAUSE_REG
	bal	ls132_hexserial
	nop
	SMC_TTYDBG("SMC: STATUS=")
	mfc0	a0, COP_0_STATUS_REG
	bal	ls132_hexserial
	nop
	SMC_TTYDBG("SMC: ERRORPC=")
	mfc0	a0, COP_0_ERROR_PC
	bal	ls132_hexserial
	nop
	SMC_TTYDBG("SMC: EPC=")
	mfc0	a0, COP_0_EXC_PC
	bal	ls132_hexserial
	nop
	SMC_TTYDBG("SMC: BADADDR=")
	mfc0	a0, COP_0_BAD_VADDR
	bal	ls132_hexserial
	nop

	mtc0	zero, COP_0_STATUS_REG
	mtc0	zero, COP_0_CAUSE_REG
	li	t0, SR_BOOT_EXC_VEC	/* Exception to Boostrap Location */
	mtc0	t0, COP_0_STATUS_REG

	SMC_TTYDBG("\r\nSMC: GS132 Ebase:")
	mfc0	a0, $15, 1
	bal	ls132_hexserial
	nop
	SMC_TTYDBG("SMC: GS132 PRID:")
	mfc0	a0, $15, 0
	bal	ls132_hexserial

#if SMC_SET_KSEG1_CACHE
	mfc0	t6, CP0_CONFIG
	ori	t6, t6, 7
	xori	t6, t6, 4
	mtc0	t6, CP0_CONFIG
	SMC_TTYDBG("SMC: Set KSEG1 Cache\r\n")
#endif

#if SMC_FIND_RAM
	li	t0, 0x90000000
	li	t1, 0xdeadbeef
	li	t3, 0xa0000000
test_again:
	SMC_TTYDBG("SMC: Test at:")
	move	a0, t0
	bal	ls132_hexserial
	nop
	sw	t1, 0x0(t0)
	nop
	lw	t2, 0x0(t0)
	SMC_TTYDBG("SMC: Read back:")
	move	a0, t2
	bal	ls132_hexserial
	nop
	addiu	t0, t0, 0x100
	beq	t2, t1, test_end
	nop
	b	test_again
test_end:
	SMC_TTYDBG("SMC: SRAM Test Found END")
#endif

#if SMC_BOOT_FROM_GIVEN_PC
boot_test:
	li	t1, LS132_MAIL
	sw	zero, 0x0(t1)
retry:
	lw	t0, 0x0(t1)
	nop
	bnez	t0, boot
	nop
	b	retry
	nop
#endif

	li	t0, 0xbd600000 // KSEG0 Uncached

boot:
	SMC_TTYDBG("SMC: BOOT PC:")
	move	a0, t0
	bal	ls132_hexserial
	nop
	SMC_TTYDBG("SMC: Content at PC:")
	lw	a0, 0x0(t0)
	bal	ls132_hexserial
	nop	
	SMC_TTYDBG("\r\nSMC: Go\r\n")
	jr	t0
	nop

	.set    pop
END(ls132_core_main)
