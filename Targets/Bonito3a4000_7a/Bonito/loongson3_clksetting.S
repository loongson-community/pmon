/*loongson3_clksetting.S
	change the PLL settings of each core
NOTE:
    Using S1 for passing the NODE ID
*/
#define SOFT_CLKSEL
#define BBGEN

#ifdef  BBGEN
#define VBBn_VAL	0x6
#define VBBp_VAL	0x4
#define FEEDBACK	0x3
#endif

#ifdef SOFT_CLKSEL

#define CORE_STEP	25

#define USE_LS_PLL	1
#define DDR_SEL_NODE	0
#define DDR_NODE_DIV	4

#ifdef BONITO_100M
#define SYS_CLOCK	100  //MUST BE 100 or 25, depend on the osillator
#elif BONITO_25M
#define SYS_CLOCK	25   //MUST BE 100 or 25, depend on the osillator SYS_CLOCK IS NOT GOOD FOR HIGH FREQUENCY
#endif
//########THE CORE FREQ is MOVED to start.S
//#define CORE_FREQ	1600 //RECOMMENDED TO BE MULTIPLE OF 25, can be others depending on L1_DIV
#define DDR_FREQ	400  //RECOMMENDED TO BE MULTIPLE OF 25, can be multiple of 12.5/6.25 depending on DDR_DIV

#ifdef BONITO_100M
#define DDR_REFC	SYS_CLOCK/100 //(clk_ref/div_refc)->(100M) clk_ref = 100M/100M
#elif BONITO_25M
#define DDR_REFC	SYS_CLOCK/25 //(clk_ref/div_refc)->(25m-50M) clk_ref = 100M/25M
#endif

#if (DDR_FREQ < 800)
//#define DDR_DIV	4 //(clk_ref/div_refc * div_loopc)->(1.2GHz-3.2GHz)
#if ((DDR_FREQ == 533)||(DDR_FREQ == 400)||(DDR_FREQ == 466))
#define DDR_DIV		9 //(clk_ref/div_refc * div_loopc)->(1.2GHz-3.2GHz)
#elif   (DDR_FREQ == 425 || DDR_FREQ == 333 )
#define DDR_DIV		12
#else
#define DDR_DIV		8
#endif
#else
#define DDR_DIV		2
#endif
#if (DDR_FREQ == 533)
#define DDR_LOOPC	48
#elif (DDR_FREQ == 466)
#define DDR_LOOPC	42
#elif (DDR_FREQ == 333)
#define DDR_LOOPC	40
#else
#define DDR_LOOPC	(DDR_REFC*DDR_FREQ*DDR_DIV/SYS_CLOCK)
#endif

// L1_* define core frequency
#define LS_PLL		USE_LS_PLL
#define ST_PLL		(1 - USE_LS_PLL)

#define L1_REFC		1//SYS_CLOCK/25
//#define L1_REFC		SYS_CLOCK/25
#if (CORE_FREQ < 1200)
#define L1_DIV		2
#else
#define L1_DIV		1
#endif
#define L1_LOOPC	(L1_REFC*CORE_FREQ*L1_DIV/SYS_CLOCK)

#define BYPASS_CORE	0x0
#define BYPASS_NODE	0x0
#define BYPASS_L1	0x0

#define SYS_PD			(((1 - ST_PLL) << 1) | (1 - LS_PLL))
#define PLL_L1_LOCKED 		((ST_PLL << 17) | (LS_PLL << 16))
#define PLL_CHANG_COMMIT 	0x1

#define BYPASS_REFIN 		(0x1 << 0)
#define CORE_CLKSEL		0xc0
#define CORE_HSEL		0x80
#define PLL_L1_ENA		(0x1 << 2)

#define MEM_CLKSEL 		(0x3 << 8)
#define MEM_HSEL		(0x2 << 8)
#define PLL_MEM_ENA		(0x1 << 1)
#define HT_HSEL			(0x1 << 15)

#ifdef BBGEN
	//config dcdl
	dli	t0, 0x900000001fe00440
//	li	a1, 0x1111
	li	a1, 0
	sw	a1, 0x0(t0)
	sw	zero, 0x8(t0)
#endif
#ifdef MULTI_CHIP
	TTYDBG ("Change the scale of HT0 clock\r\n")
	dli	t0, 0x900000001fe00180
	or	t0, t0, s1
	lw	a0, 0x0(t0)
	li	a1, 0xf0ffffff
	and	a0, a0, a1
	li	a1, 0x0b000000 // 4/8 of node clock
	or	a0, a0, a1
	sw	a0, 0x0(t0)
#else
	TTYDBG ("Disable HT0 clock\r\n")
	dli	t0, 0x900000001fe00180
	or	t0, t0, s1
	lw	a0, 0x0(t0)
	li	a1, 0xf0ffffff
	and	a0, a0, a1
	li	a1, 0x00000000 // Disable
	or	a0, a0, a1
	sw	a0, 0x0(t0)
#endif

	TTYDBG ("Change the scale of HT1 clock\r\n")
	dli	t0, 0x900000001fe00180
	or	t0, t0, s1
	lw	a0, 0x0(t0)
	li	a1, 0x0fffffff
	and	a0, a0, a1
	li	a1, 0xb0000000 // 4/8 of node clock
	or	a0, a0, a1
	sw	a0, 0x0(t0)

	TTYDBG ("Change the scale of LS132 clock\r\n")
	dli	t0, 0x900000001fe00420
	or	t0, t0, s1
	lw	a0, 0x0(t0)
	li	a1, 0xfff0ffff
	and	a0, a0, a1
	li	a1, 0x00080000 // 1/8 of node clock
	or	a0, a0, a1
	sw	a0, 0x0(t0)


#ifdef BBGEN
	TTYDBG ("\r\nBBGEN start  :")
	dli     t0, 0x900000001fe00478
	or      t0, t0, s1
	lw      t1, 0x0(t0)
	and     t1, (0xf << 25)
	srl     t1, 25
	move    t3, zero
	move    t4, t1
	beqz    t1, 1f
	nop
	and     t2, t1, (1 << 0)
	sll     t3, t2, 3
	and     t2, t1, (1 << 1)
	sll     t2, t2, 1
	or      t3, t2
	and     t2, t1, (1 << 2)
	srl     t2, t2, 1
	or      t3, t2
	and     t2, t1, (1 << 3)
	srl     t2, t2, 3
	or      t3, t2
	blt	t3, 3, 1f
	nop
	bgt	t3, 6, 3f
	nop
	sub	t3, 2

	li	t2, VBBn_VAL
	sub     t2, t3
	sll     t2, 4
	ori     t2, VBBp_VAL
	sub     t2, t3
	sll     t2, 8
	b       2f
	nop
1:
	li      t2, (VBBn_VAL << 12) | (VBBp_VAL << 8)
2:

	dli	t0, 0x900000001fe001a0
	or	t0, t0, s1
	li	t1, (0xff << 16) | (FEEDBACK << 4) | (0x0 << 1) | 0x1
	or	t1, t2
	sw	t1, 0x0(t0)

	TTYDBG ("\r\nBBGEN config value  :")
	dli	t0, 0x900000001fe001a0
	or	t0, t0, s1
	lw	a0, 0x0(t0)

	bal	hexserial
	nop
	TTYDBG ("\r\n")
3:
#endif


	TTYDBG ("Soft CLK SEL adjust begin\r\n")

	dli	t0, 0x900000001fe00194
	or	t0, t0, s1
	lw	a0, 0x0(t0)
	li	a1, CORE_CLKSEL
	and	a0, a0, a1
	li	a1, CORE_HSEL
	bne	a0, a1, 20f //soft_mem
	nop

//soft_sys:
	TTYDBG ("CORE & NODE:")

	move	t6, zero // t3 = 0, no software modification
#ifdef INPUT_PARAM  //this code used for debug
	PRINTSTR("\r\nBase freq 800M + (25M * input value) = target freq")
	bal	inputaddress
	nop

	//compute core freq in t4
	move	t2, v0
	li	t3, CORE_STEP
	li	t4, 800
	mulou	t2, t2, t3
	addu	t4, t2

	//compute L1_DIV int t5
	li	t5, 1
	bgeu	t4, 1200, L1_DIV_2
	nop
	li	t5, 2
L1_DIV_2:
	//compute L1_LOOPC in t4
	li	t3, L1_REFC
	mulou	t4, t3, t4
	divu	t4, t4, SYS_CLOCK

	// Put L1_REFC in t3
	li	t3, L1_REFC

	// soft modification
	li	t6, 0x1
#endif

#ifdef MIKU_SMC
	/* Only use values in Miku Config table if model magic matches */
	li	t2, MIKU_CFG_TAB_ADDR
	lw	t0, 0x0(t2)
	li	t3, MIKU_MODEL_MAGIC
	beq	t0, t3, pll_sel_from_table
	nop
	PRINTSTR("\r\nMiku MAGIC Mismtach\r\n")
	b  pll_mismatch_cont
	nop
	/* Actual values in config table */
pll_sel_from_table:
	PRINTSTR("\r\nMiku freq sel\r\n")
	lw	t2, MIKU_CFG_OFF_PLL(t2)

	and		t3, t2, MIKU_CFG_PLL_REFC_F
	dsrl	t3, t3, MIKU_CFG_PLL_REFC_B

	and		t4, t2, MIKU_CFG_PLL_LOOPC_F
	dsrl	t4, t4, MIKU_CFG_PLL_LOOPC_B

	and		t5, t2, MIKU_CFG_PLL_DIV_F
	dsrl	t5, t5, MIKU_CFG_PLL_DIV_B
	// soft modification
	li	t6, 0x1

pll_mismatch_cont:
#endif

	dli	t0, 0x900000001fe001b0
	or	t0, t0, s1
	li	t1, (0x7 << 19) 	//power down pll L1 first
	sd	t1, 0x0(t0)
	dli	t1, (L1_DIV << 0)
	sd	t1, 0x8(t0)
	dli	t1, (L1_LOOPC << 54) | (L1_REFC  << 48) | \
			  (L1_DIV   << 42) | (L1_LOOPC << 32) | (L1_REFC << 26) | \
			  (SYS_PD   << 19) | (ST_PLL   << 22) | (0x3 << 10) | (0x1 << 7)

	beqz	t6, pll_no_soft_mod
	nop
	and		t1, ~((0x1ff << 32) | (0x3f << 42) | (0xf << 26))
	dsll	t5, t5, 42
	dsll	t4, t4, 32
	dsll	t3, t3, 26
	or	t4, t4, t5
	or	t4, t3, t4
	or	t1, t4, t1
pll_no_soft_mod:

	sd	t1, 0(t0)
	ori	t1, PLL_L1_ENA
	sd	t1, 0x0(t0)

11: //wait_locked_sys:
	ld	a0, 0x0(t0)
	li	a1, PLL_L1_LOCKED
	and	a0, a1, a0
	beqz	a0, 11b //wait_locked_sys
	nop

	ld	a0, 0x0(t0)
	ori	a0, a0, PLL_CHANG_COMMIT
	sd	a0, 0x0(t0)

	bal	hexserial
	nop

20: //soft_mem:
#if DDR_SEL_NODE
	dli	t0, 0x900000001fe001c0
	or	t0, t0, s1
    	dli	a0, (DDR_DIV << 24) | (DDR_LOOPC << 14) | (DDR_NODE_DIV << 8) | (DDR_SEL_NODE << 30) | (0x3 << 4) | (0x1 << 3) | PLL_MEM_ENA | 0x1
    	sw	a0, 0x0(t0)

#else

	dli	t0, 0x900000001fe001c0
	or	t0, t0, s1
	li	t1, (0x1 << 7) 	//power down all pll first
	sw	t1, 0x0(t0)


	dli	t0, 0x900000001fe00194
	or	t0, t0, s1
	lw	a0, 0x0(t0)
	li	a1, MEM_CLKSEL
	and	a0, a0, a1
	li	a1, MEM_HSEL
	bne	a0, a1, 30f
	nop

	TTYDBG ("\r\nMEM	  :")

	dli	t0, 0x900000001fe001c0
	or	t0, t0, s1
	dli	a0, (DDR_DIV << 24) | (DDR_LOOPC << 14) | (DDR_REFC << 8) | (0x3 << 4) | (0x1 << 3) | PLL_MEM_ENA
	sw	a0, 0x0(t0)
21:
	lw	a0, 0x0(t0)
	li	a1, 0x00000040
	and	a0, a0, a1
	beqz	a0, 21b
	nop

	lw	a0, 0x0(t0)
	ori	a0, a0, 0x1
	sw	a0, 0x0(t0)

	bal	hexserial
	nop

	TTYDBG ("\r\nfdcoefficient  :")
	dli	t0, 0x900000001fe001c0
	or	t0, t0, s1
	ld	t1,0x0(t0)
	dsrl	a0,t1,8
	and	a0,a0,63

	dsrl	a1,t1,14
	and	a1,a1,1023

	dmul	a0,a0,a1
	dsrl	a1,t1,24
	and	a1,a1,63

	ddiv	a0,a0,a1
	bal	hexserial
	nop

#endif


30: //soft_ht:
	TTYDBG ("\r\nHT    :")

	dli	t0, 0x900000001fe001b0
	or	t0, t0, s1
	lw	a0, 0x14(t0)
	bal	hexserial
	nop


//soft_out:

	TTYDBG ("\r\n")
	TTYDBG ("SYS_LOOPC:")

#ifdef INPUT_PARAM
	move	a0, t4
#else
	li	a0, L1_LOOPC
#endif
	bal	hexserial
	nop
	TTYDBG ("\r\n")

	TTYDBG ("\r\n")
	TTYDBG ("DDR_LOOPC:")
	li	a0, DDR_LOOPC
	bal	hexserial
	nop
	TTYDBG ("\r\n")

#endif

