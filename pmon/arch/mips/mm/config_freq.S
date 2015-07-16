/***********************************
NOT finished yet!
***********************************/

#define SOFT_CLKSEL
#ifdef SOFT_CLKSEL


//#define L1_LOOPC    68//850
#define L1_LOOPC    64//800
//#define L1_LOOPC    48//600
#define L1_DIV      2


#define PLL_CHANG_COMMIT 0x1

#define BYPASS_REFIN 		(0x1 << 0)
#define CORE_CLKSEL		0x1c
#define CORE_HSEL		0x0c
#define PLL_L1_LOCKED (0x1 << 16)
#define PLL_L1_ENA		(0x1 << 2)

#define PLL_MEM_ENA		(0x1 << 1)
#define PLL_MEM_LOCKED (01 << 16)

#define HT_HSEL	(0x1 << 15)

	TTYDBG ("Soft CLK SEL adjust begin\r\n")

	li      t0, 0xbfe00194
	lw      a0, 0x0(t0)
	li      a1, CORE_CLKSEL
	and     a0, a0, a1
	li	    a1, CORE_HSEL
	bne	    a0, a1, soft_mem
	nop

soft_sys:
	TTYDBG ("CORE & NODE:")

	li      t0, 0xbfe001b0
	dli	    t1, (L1_LOOPC << 32) | (L1_DIV << 42) | (0x3 << 10) | (0x1 << 7)
	sd	    t1, 0(t0)
	ori	    t1, PLL_L1_ENA
	sd      t1, 0x0(t0)

wait_locked_sys:
	ld      a0, 0x0(t0)
	li      a1, PLL_L1_LOCKED
	and     a0, a1, a0
	beqz    a0, wait_locked_sys
	nop

	ld      a0, 0x0(t0)
	ori     a0, a0, PLL_CHANG_COMMIT
	sd      a0, 0x0(t0)

	bal     hexserial
	nop

soft_mem:

soft_fdcoefficient:
	TTYDBG ("\r\nfdcoefficient  :")
	li      t0, 0xbfe001c0
	ld      t1, 0x0(t0)
	dsrl    a0, t1, 8
	and     a0, a0, 63

	dsrl    a1, t1, 14
	and     a1, a1, 1023

	dmul    a0, a0, a1
	dsrl    a1, t1, 24
	and     a1, a1, 63

	ddiv    a0, a0, a1
	bal     hexserial
	nop

soft_ht:
	TTYDBG ("\r\nHT         :")

	li      t0, 0xbfe001b0
	lw      a0, 0x14(t0)
	bal     hexserial
	nop


soft_out:
	TTYDBG ("\r\n")

#endif

//#define DDR_LOOPC    64 //264MHz
//#define DDR_LOOPC    72 //300MHz
//#define DDR_LOOPC    80 //330MHz
//#define DDR_LOOPC  48 //400MHz
//#define DDR_LOOPC  60 //500MHz
#define DDR_LOOPC  72 //600MHz
//#define DDR_LOOPC  28 //466MHz
//#define DDR_LOOPC  32 //533MHz
//#define DDR_LOOPC  34 //566MHz
//#define DDR_LOOPC  38 //633MHz


#define DDR_REFC   1
//#define DDR_DIV    8
#define DDR_DIV    4

#if     (DDR_FREQ == 264)
#define DDR_LOOPC  64
#define DDR_DIV    8
#elif   (DDR_FREQ == 300)
#define DDR_DIV    8
#elif   (DDR_FREQ == 330)
#define DDR_DIV    8
#elif   (DDR_FREQ == 400)
#elif   (DDR_FREQ == 466)
#elif   (DDR_FREQ == 500)
#elif   (DDR_FREQ == 533)
#elif   (DDR_FREQ == 566)
#elif   (DDR_FREQ == 600)
#elif   (DDR_FREQ == 633)
#endif

#define MEM_CLKSEL (0x01f << 5)
#define MEM_HSEL (0x0f << 5)

LEAF(config_mc_freq)

	li      t0, 0xbfe00194
	lw      a0, 0x0(t0)
	li      a1, MEM_CLKSEL
	and     a0, a0, a1
	li	    a1, MEM_HSEL
	bne	    a0, a1, mc_freq_hard_mode
	nop

	li      t0, 0xbfe001c0
	dli     a0, (DDR_DIV << 24) | (DDR_LOOPC << 14) | (0x3 << 4) | (0x1 << 3) | PLL_MEM_ENA
	sw	    a0, 0x0(t0)
wait_locked_ddr:
	lw      a0, 0x0(t0)
	li      a1, 0x00000040
	and     a0, a0, a1
	beqz    a0, wait_locked_ddr
	nop

	lw      a0, 0x0(t0)
	ori     a0, a0, 0x1
	sw      a0, 0x0(t0)

mc_freq_hard_mode:
    jr      ra
    nop

END(config_mc_freq)
