/*whd : loongson3_clksetting.S
	change the PLL settings of each core

ATTENTION:

         Using S1 for passing the NODE ID

	PLL
	40M > refclk/L1_div_ref >20M
	3.2G > refclk/ L1_div_ref*L1_loopc > 1.2G
	node_clock=refclk/L1_div_ref*L1_loopc/L1_divout/L2_divout
*/
#define SOFT_CLKSEL


#ifndef DDR_FREQ
/* MEM @ 500Mhz */
//#define DDR_FREQ   500   //this param must change with DDR freq together,ether soft or hard freq modified!!!
/* MEM @ 400Mhz */
#define DDR_FREQ   400
#endif
#ifdef SOFT_CLKSEL

#define DDR_REFC   4
#define DDR_DIV    1
#define DDR_DIV_L2    4
#define DDR_LOOPC  (DDR_FREQ*DDR_REFC*DDR_DIV*DDR_DIV_L2/100)

#ifndef CORE_FREQ
/* CPU @ 800Mhz */
#define CORE_FREQ   800
#endif

#define L1_LOOPC    64
#define L1_REFC     4
#define L1_DIV      1
#define L2_DIV      2
#define CORE_LOOPC  (CORE_FREQ*L1_DIV*L2_DIV*L1_REFC/100)

#if 1
/* GPU @ 400Mhz */
#define GPU_DIV_L2    7
#define HDA_DIV_L2    100
#endif
#if 0
/* GPU @ 400Mhz */
#define GPU_DIV_L2    5
#endif
#if 0
/* CPU @ 1000Mhz */
#define L1_LOOPC    80
#define L1_REFC     4
#define L1_DIV      1
#define L2_DIV      2
#endif
/* DC @ 200Mhz */
/* GMAC @ 125Mhz */
#define DC_LOOPC    80
#define DC_REFC     4
#define DC_DIV      1
#define DC_DIV_L2   8
#define GMAC_DIV    16

#define PIX0_LOOPC	109
#define PIX0_REFC	5
#define PIX0_DIV	1
#define PIX0_DIV_L2	20

#define PIX1_LOOPC	109
#define PIX1_REFC	5
#define PIX1_DIV	1
#define PIX1_DIV_L2	20
#define BYPASS_CORE 0x0
#define BYPASS_NODE 0x0
#define BYPASS_L1   0x0

#define PLL_CHANG_COMMIT 0x1
/*
#define BYPASS_REFIN 		(0x1 << 0)
*/
#define PLL_L1_LOCKED 		(0x1 << 16)
#define PLL_L1_ENA		(0x1 << 2)

#define PLL_MEM_ENA		(0x1 << 2)//chg
#define PLL_MEM_LOCKED 		(01 << 16)

#define HT_HSEL			(0x1 << 15)

	TTYDBG ("Soft CLK SEL adjust begin\r\n")

	li     	t0, 0xbfe10480
	li	t1, (0x1 << 19) 	//power down pll L1 first
	sd	t1, 0x0(t0)
	dli	t1, (L1_LOOPC << 32) | (L1_DIV << 42) | (L1_REFC << 26) | (0x3 << 10) | (0x1 << 7)
	dli	t2, L2_DIV
	sd	t1, 0(t0)
	sd	t2, 8(t0)
	ori	t1, PLL_L1_ENA
	sd      t1, 0x0(t0)
	
11:
	ld      a0, 0x0(t0)
	li      a1, PLL_L1_LOCKED
	and     a0, a1, a0
	beqz    a0, 11b //wait_locked_sys
	nop

	ld      a0, 0x0(t0)
	ori     a0, a0, PLL_CHANG_COMMIT
	sd      a0, 0x0(t0)

	bal     hexserial
	nop

	TTYDBG ("\r\nMEM        :")

	li      t0, 0xbfe10490
	li	t1, (0x1 << 19) 	//power down pll  first
	sd	t1, 0x0(t0)
	dli     t1, (DDR_DIV << 42) | (DDR_REFC << 26) | (DDR_LOOPC << 32) | (0x3 << 10) | (0x1 << 7)
	dli	t2, (GPU_DIV_L2 << 22) | (DDR_DIV_L2) | (HDA_DIV_L2 << 44)
	sd	t1, 0x0(t0)
	sd	t2, 0x8(t0)
	ori	t1, PLL_L1_ENA
	sd      t1, 0x0(t0)

21:
	lw      a0, 0x0(t0)
	li      a1, PLL_MEM_LOCKED
	and     a0, a0, a1
	beqz    a0, 21b //wait_locked_ddr
	nop

	lw      a0, 0x0(t0)
	ori     a0, a0, 0x3
	sw      a0, 0x0(t0)

	bal     hexserial
	nop

	TTYDBG ("\r\nDC        :")

	li      t0, 0xbfe104a0
	li	t1, (0x1 << 19) 	//power down pll  first
	sd	t1, 0x0(t0)
	dli     t1, (DC_DIV << 42) | (DC_REFC << 26) | (DC_LOOPC << 32) | (0x3 << 10) | (0x1 << 7)
	dli	t2, (GMAC_DIV << 22) | (DC_DIV_L2)
	sd	t1, 0x0(t0)
	sd	t2, 0x8(t0)
	ori	t1, PLL_L1_ENA
	sd      t1, 0x0(t0)

21:
	lw      a0, 0x0(t0)
	li      a1, PLL_MEM_LOCKED
	and     a0, a0, a1
	beqz    a0, 21b
	nop

	lw      a0, 0x0(t0)
	ori     a0, a0, 0x3
	sw      a0, 0x0(t0)

	bal     hexserial
	nop

	TTYDBG ("\r\nPIX0        :")

	li      t0, 0xbfe104b0		//pll_pix0
	li	t1, (0x1 << 19) 	//power down pll  first
	sd	t1, 0x0(t0)
	dli     t1, (PIX0_DIV << 42) | (PIX0_REFC << 26) | (PIX0_LOOPC << 32) | (0x3 << 10) | (0x1 << 7)
	dli	t2, PIX0_DIV_L2
	sd	t1, 0x0(t0)
	sd	t2, 0x8(t0)
	ori	t1, PLL_L1_ENA
	sd      t1, 0x0(t0)

21:
	lw      a0, 0x0(t0)
	li      a1, PLL_MEM_LOCKED
	and     a0, a0, a1
	beqz    a0, 21b
	nop

	lw      a0, 0x0(t0)
	ori     a0, a0, 0x1
	sw      a0, 0x0(t0)

	bal     hexserial
	nop

	TTYDBG ("\r\nPIX1        :")

	li      t0, 0xbfe104c0		//pll_pix1
	li	t1, (0x1 << 19) 	//power down pll  first
	sd	t1, 0x0(t0)
	dli     t1, (PIX1_DIV << 42) | (PIX1_REFC << 26) | (PIX1_LOOPC << 32) | (0x3 << 10) | (0x1 << 7)
	dli	t2, PIX1_DIV_L2
	sd	t1, 0x0(t0)
	sd	t2, 0x8(t0)
	ori	t1, PLL_L1_ENA
	sd      t1, 0x0(t0)

21:
	lw      a0, 0x0(t0)
	li      a1, PLL_MEM_LOCKED
	and     a0, a0, a1
	beqz    a0, 21b
	nop

	lw      a0, 0x0(t0)
	ori     a0, a0, 0x1
	sw      a0, 0x0(t0)

	bal     hexserial
	nop


#endif
