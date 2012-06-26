#define EJTAG_TLB_INDEX	EJTAG_INDEX
#define EJTAG_TLB_HIGH	EJTAG_ENTRYHI
#define EJTAG_TLB_LO0	EJTAG_ENTRYLO0
#define EJTAG_TLB_LO1	EJTAG_ENTRYLO1

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
	li	t0, -1
	mtc0	t0, COP_0_TLB_PG_MASK    # 16MB page

	li	t0, 63
1:
	mtc0	t0, COP_0_TLB_INDEX

    #PRINTSTR("index:")
	#move	a0, t0
    #	bal	hexserial64
	#nop
    #	PRINTSTR("\r\n")
	li		t2,EJTAG_TLB_INDEX
	sd		t0, 0x0(t2)

	tlbr
	nop
	nop
	nop
	nop

	.set mips64
	dmfc0	t1, COP_0_TLB_HI
	#PRINTSTR("tlb hi:")
	#move	a0, t3
    #	bal	hexserial64
	#nop
    #	PRINTSTR("\r\n")
	li		t2,EJTAG_TLB_HIGH
	sd		t1, 0x0(t2)


	dmfc0	t1, COP_0_TLB_LO0
	#PRINTSTR("tlb lo0:")
	#move	a0, t4
    #	bal	hexserial64
	#nop
    #	PRINTSTR("\r\n")
	li		t2,EJTAG_TLB_LO0
	sd		t1, 0x0(t2)


	dmfc0	t1, COP_0_TLB_LO1
	#PRINTSTR("tlb lo1:")
	#move	a0, t5
    #	bal	hexserial64
	#nop
    #	PRINTSTR("\r\n")
	li		t2,EJTAG_TLB_LO1
	sd		t1, 0x0(t2)

	.set mips3

	bnez	t0, 1b
	addiu	t0, -1
