
#define SOFT_CLKSEL

#ifdef SOFT_CLKSEL

//#define DDR_LOOPC	 48//200MHz
//#define DDR_LOOPC	 72//300MHz
//#define DDR_LOOPC  96 //400MHz
//#define DDR_LOOPC  28 //466MHz
//#define DDR_LOOPC  60 //500MHz
#define DDR_LOOPC  80 //533MHz
//#define DDR_LOOPC  34 //566MHz
//#define DDR_LOOPC  72 //600MHz
//#define DDR_LOOPC  76 //633MHz
//#define DDR_LOOPC  80 //667MHz
#define DDR_REFC   1
#define DDR_DIV    5

// L1_* define both CPU and Node freq simutanleously
//#define L1_LOOPC    80//1000@25MHz
#define L1_LOOPC    64//800@25MHz
//#define L1_LOOPC    49//808@33MHz
//#define L1_LOOPC    48//600
#define L1_DIV      2

#define BYPASS_CORE 0x0
#define BYPASS_NODE 0x0
#define BYPASS_L1   0x0

#define PLL_CHANG_COMMIT 0x1

#define BYPASS_REFIN 		(0x1 << 0)
#define CORE_CLKSEL		0x1c
#define CORE_HSEL		0x0c
#define PLL_L1_LOCKED (0x1 << 16)
#define PLL_L1_ENA		(0x1 << 2)

#define MEM_CLKSEL (0x01f << 5)
#define MEM_HSEL (0x0f << 5)
#define PLL_MEM_ENA		(0x1 << 1)
#define PLL_MEM_LOCKED (01 << 16)

#define HT_HSEL	(0x1 << 15)



	TTYDBG ("Soft CLK SEL adjust begin\r\n")

	dli     t0, 0x900000001fe00194
	or	t0, t0, s1
	lw      a0, 0x0(t0)
	li      a1, CORE_CLKSEL
	and     a0, a0, a1
	li	a1, CORE_HSEL
	bne	a0, a1, n1_soft_mem
	nop

n1_soft_sys:
	TTYDBG ("CORE & NODE:")

	dli     t0, 0x900000001fe001b0
	or	t0, t0, s1
	li	t1, (0x1 << 19) 	//power down pll L1 first
	sd	t1, 0x0(t0)
	dli	t1, (L1_LOOPC << 32) | (L1_DIV << 42) | (0x3 << 10) | (0x1 << 7)
	sd	t1, 0(t0)
	ori	t1, PLL_L1_ENA
	sd      t1, 0x0(t0)

n1_wait_locked_sys:
	ld      a0, 0x0(t0)
	li      a1, PLL_L1_LOCKED
	and     a0, a1, a0
	beqz    a0, n1_wait_locked_sys
	nop

	ld      a0, 0x0(t0)
	ori     a0, a0, PLL_CHANG_COMMIT
	sd      a0, 0x0(t0)

	bal     hexserial
	nop

n1_soft_mem:
	dli     t0, 0x900000001fe00194
	or	    t0, t0, s1
	lw      a0, 0x0(t0)
	li      a1, MEM_CLKSEL
	and     a0, a0, a1
	li	    a1, MEM_HSEL
	bne	    a0, a1, n1_soft_ht
	nop

	TTYDBG ("\r\nMEM        :")

	dli      t0, 0x900000001fe001c0
	or	t0, t0, s1
	dli     a0, (DDR_DIV << 24) | (DDR_LOOPC << 14) | (0x3 << 4) | (0x1 << 3) | PLL_MEM_ENA
	sw	a0, 0x0(t0)
n1_wait_locked_ddr:
	lw      a0, 0x0(t0)
	li      a1, 0x00000040
	and     a0, a0, a1
	beqz    a0, n1_wait_locked_ddr
	nop

	lw      a0, 0x0(t0)
	ori     a0, a0, 0x1
	sw      a0, 0x0(t0)

	bal     hexserial
	nop

n1_soft_ht:
	TTYDBG ("\r\nHT         :")

	dli     t0, 0x900000001fe001b0
	or	t0, t0, s1
	lw      a0, 0x14(t0)
	bal     hexserial
	nop

//soft_out:

	TTYDBG ("\r\n")
#endif
