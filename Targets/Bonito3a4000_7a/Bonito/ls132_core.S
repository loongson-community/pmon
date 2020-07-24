/*ls132 core code*/

#define TURBO_CORE_NUM		4
#define NOT_TURBO_CORE_LEVEL	7
#define TURBO_FREQ		2000
#define LOW_LEVEL		3
#define HIGH_LEVEL		3

#define TURBO_2_CORES_SUB_FREQ	0
#define TURBO_3_CORES_SUB_FREQ	2
#define TURBO_4_CORES_SUB_FREQ	4

#define L1_LOOPC_TURBO0_FREQ    	(L1_REFC * TURBO_FREQ * L1_DIV / SYS_CLOCK)
#define L1_LOOPC_TURBO1_FREQ    	(L1_REFC * (TURBO_FREQ - (TURBO_2_CORES_SUB_FREQ * 25)) * L1_DIV / SYS_CLOCK)
#define L1_LOOPC_TURBO2_FREQ    	(L1_REFC * (TURBO_FREQ - (TURBO_3_CORES_SUB_FREQ * 25)) * L1_DIV / SYS_CLOCK)
#define L1_LOOPC_TURBO3_FREQ    	(L1_REFC * (TURBO_FREQ - (TURBO_4_CORES_SUB_FREQ * 25)) * L1_DIV / SYS_CLOCK)

#define LS132_BASE_REG	0xbfe00420

#define LS132_PUTC(x) \
	li a0,x;bal ls132_tgt_putchar;nop
#define COUNT_PERIOD 0x10000000
//#define LS132_UART 0xb0080000 //7a uart
#define LS132_UART 0xbfe001e8 //3a4000 uart 1
//#define NODE_CLK_CLOSE
//#define CPU_CLK_CLOSE
#define SET_SRAM
#define SRAM_LEVEL  6 //voltage > 1.1v
#define LS132_MAIL 0xbfe0051c
#define LS_FREQ_REG	0xbfe001d0
#define LS_NODE_FREQ_REG	0xbfe00184

#define SRAM_CTRL_VALUE_LV 0x1f156f
#define VBBn_VAL_LV 0
#define VBBp_VAL_LV 0

#define COMPLETE_STATUS		0x80000000
#define COMMAND_FIELD		0x7f000000
#define VOLTAGE_COMMAND		0x21
#define TURBO_MODE		0x80000
#define NORMAL_MODE		0x40000
#define CPU_ID_FIELD		0xf
#define FREQ_FIELD		0xf0
#define FREQ_FIELD_OFFSET	4
#define FEATURE_COMMAND		0x22
#define TURBO_INFO_COMMAND		0x23

/*for s3 used*/
#define VOLTAGE_FIELD		0xf0000
#define VOLTAGE_FIELD_OFFSET	16
#define TURBO_FLAG		0x1000000
#define VOLTAGE_DOWN		0x400000
#define VOLTAGE_UP		0x800000
#define VOLTAGE_DOWN_OFFSET	22
#define VOLTAGE_UP_OFFSET	23
#define FLAG_FIELD		0xff0000

#define LEVEL0		0
#define LEVEL1		1
#define LEVEL2		2
#define LEVEL3		3
#define LEVEL4		4
#define LEVEL5		5
#define LEVEL6		6
#define LEVEL7		7
#define LEVEL8		8
#define LEVEL9		9
#define LEVEL10		10
#define LEVEL11		11
#define LEVEL12		12
#define LEVEL13		13
#define LEVEL14		14
#define LEVEL15		15
#define LEVEL16		16

#ifndef MPS_V
#define V_LEVEL0	0x1600
#define V_LEVEL1	0x1600
#define V_LEVEL2	0x1a00
#define V_LEVEL3	0x1900 //1.25v
#define V_LEVEL4	0x1900
#define V_LEVEL5	0x1900
#define V_LEVEL6	0x1900
#define V_LEVEL7	0x1400 //1.00v
#define V_LEVEL8	0x1400
#define V_LEVEL9	0x1400 //1.00v
#define V_LEVEL10	0x1400 //1.00v
#define V_LEVEL11	0x1300
#define V_LEVEL12	0x1300
#define V_LEVEL13	0x1300
#define V_LEVEL14	0x1300
#define V_LEVEL15	0x1300
#define V_LEVEL16	0x1300

#define CHECK_LEVEL0	(V_LEVEL0  / 8 - 8)
#define CHECK_LEVEL1	(V_LEVEL1  / 8 - 8)
#define CHECK_LEVEL2	(V_LEVEL2  / 8 - 8)
#define CHECK_LEVEL3	(V_LEVEL3  / 8 - 8) //1.25v
#define CHECK_LEVEL4	(V_LEVEL4  / 8 - 8)
#define CHECK_LEVEL5	(V_LEVEL5  / 8 - 8)
#define CHECK_LEVEL6	(V_LEVEL6  / 8 - 8)
#define CHECK_LEVEL7	(V_LEVEL7  / 8 - 8) //1.1v
#define CHECK_LEVEL8	(V_LEVEL8  / 8 - 8)
#define CHECK_LEVEL9	(V_LEVEL9  / 8 - 8) //1.0v
#define CHECK_LEVEL10	(V_LEVEL10 / 8 - 8) //0.95v
#define CHECK_LEVEL11	(V_LEVEL11 / 8 - 8)
#define CHECK_LEVEL12	(V_LEVEL12 / 8 - 8)
#define CHECK_LEVEL13	(V_LEVEL13 / 8 - 8)
#define CHECK_LEVEL14	(V_LEVEL14 / 8 - 8)
#define CHECK_LEVEL15	(V_LEVEL15 / 8 - 8)
#define CHECK_LEVEL16	(V_LEVEL16 / 8 - 8)
#else
#define V_LEVEL0	76
#define V_LEVEL1	76
#define V_LEVEL2	81 //1,3
#define V_LEVEL3	76 //1.25v
#define V_LEVEL4	76
#define V_LEVEL5	76
#define V_LEVEL6	76
#define V_LEVEL7	51 //1.00v
#define V_LEVEL8	51
#define V_LEVEL9	51 //1.00v
#define V_LEVEL10	51 //1.00v
#define V_LEVEL11	46 //0.95
#define V_LEVEL12	46
#define V_LEVEL13	46
#define V_LEVEL14	46
#define V_LEVEL15	46
#define V_LEVEL16	46

#define CHECK_LEVEL0	(V_LEVEL0  - 2)
#define CHECK_LEVEL1	(V_LEVEL1  - 2)
#define CHECK_LEVEL2	(V_LEVEL2  - 2)
#define CHECK_LEVEL3	(V_LEVEL3  - 2) //1.25v
#define CHECK_LEVEL4	(V_LEVEL4  - 2)
#define CHECK_LEVEL5	(V_LEVEL5  - 2)
#define CHECK_LEVEL6	(V_LEVEL6  - 2)
#define CHECK_LEVEL7	(V_LEVEL7  - 2) //1.1v
#define CHECK_LEVEL8	(V_LEVEL8  - 2)
#define CHECK_LEVEL9	(V_LEVEL9  - 2) //1.0v
#define CHECK_LEVEL10	(V_LEVEL10 - 2) //0.95v
#define CHECK_LEVEL11	(V_LEVEL11 - 2)
#define CHECK_LEVEL12	(V_LEVEL12 - 2)
#define CHECK_LEVEL13	(V_LEVEL13 - 2)
#define CHECK_LEVEL14	(V_LEVEL14 - 2)
#define CHECK_LEVEL15	(V_LEVEL15 - 2)
#define CHECK_LEVEL16	(V_LEVEL16 - 2)
#endif
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
	li	v0, LS132_UART
1:
	lbu	v1, NSREG(NS16550_LSR)(v0)
	and	v1, LSR_TXRDY
	beqz	v1, 1b
	nop

	sb	a0, NSREG(NS16550_DATA)(v0)
	jr	ra
	nop
	.set pop
END(ls132_tgt_putchar)

LEAF(ls132_v_cmp)
/*
 * a0 give the need compare value
 * 0-3 core 0 level,4-7 core 1 level,8-11 core 2 level,
 * 12-15 core 3 level,16-19 last time highest level.
 * v0 give the result value
 * 0-3 highest level value, 4 bit reduce voltage, 5 bit increase voltage.
 * use register:
 * a0,a1,v0,v1
 */
	.set    push
	.set    noreorder
	.set    mips32

	and	v0, a0, 0xf

	srl	v1, a0, 4
	and	v1, 0xf
	
	bleu	v0, v1, 1f
	nop
	move	v0, v1
1:
	srl	v1, a0, 8
	and	v1, 0xf

	bleu	v0, v1, 1f
	nop
	move	v0, v1
1:
	srl	v1, a0, 12
	and	v1, 0xf

	bleu	v0, v1, 1f
	nop
	move	v0, v1
1:
	/*turbo mode not change voltage*/
	and	v1, a0, TURBO_FLAG
	bnez	v1, 2f
	nop

	srl	v1, a0, 16
	and	v1, 0xf

	bgeu	v0, v1, 1f
	nop
	ori	v0, (1 << (VOLTAGE_UP_OFFSET - 16))
	b	2f
	nop
1:
	beq	v0, v1, 2f
	nop
	ori	v0, (1 << (VOLTAGE_DOWN_OFFSET - 16))
2:
	jr	ra
	nop

	.set pop
END(ls132_v_cmp)

LEAF(turbo_core_pll)
/*
 * a0 give the need check value
 * 0-3 core 0 level,4-7 core 1 level,8-11 core 2 level,
 * 12-15 core 3 level.
 * v0 give the result value
 * use register:
 * a0,v0,v1
 */
	.set    push
	.set    noreorder
	.set    mips32

	move	v0, zero

	and	v1, a0, 0xf
	bne	v1, LEVEL3, 1f
	nop
	addiu	v0, 1
1:
	srl	v1, a0, 4
	and	v1, 0xf
	bne	v1, LEVEL3, 1f
	nop
	addiu	v0, 1
1:
	srl	v1, a0, 8
	and	v1, 0xf
	bne	v1, LEVEL3, 1f
	nop
	addiu	v0, 1
1:
	srl	v1, a0, 12
	and	v1, 0xf
	bne	v1, LEVEL3, 1f
	nop
	addiu	v0, 1
1:
	beq	v0, 4, 4f
	nop
	beq	v0, 3, 3f
	nop
	beq	v0, 2, 2f
	nop

	li	v0, (L1_LOOPC_TURBO0_FREQ << 24) | (L1_REFC << 18) | \
                    (L1_DIV << 10) | (L1_LOOPC_TURBO0_FREQ << 0)
	b	5f
	nop
2:
	li	v0, (L1_LOOPC_TURBO1_FREQ << 24) | (L1_REFC << 18) | \
                    (L1_DIV << 10) | (L1_LOOPC_TURBO1_FREQ << 0)
	b	5f
	nop
3:
	li	v0, (L1_LOOPC_TURBO2_FREQ << 24) | (L1_REFC << 18) | \
                    (L1_DIV << 10) | (L1_LOOPC_TURBO2_FREQ << 0)
	b	5f
	nop
4:
	li	v0, (L1_LOOPC_TURBO3_FREQ << 24) | (L1_REFC << 18) | \
                    (L1_DIV << 10) | (L1_LOOPC_TURBO3_FREQ << 0)
5:
	jr	ra
	nop

	.set pop
END(turbo_core_pll)

LEAF(ls132_config_pll)
/*
 * a0 give the pll value
 * use register:
 * a0,v0,v1
 * a0: PLL value
 */
	.set    push
	.set    noreorder
	.set    mips32

	//configure PLL
	li	v0, 0xbfe001b0
#if 0
	lw	v1, 0x0(v0)
	and	v1, ~1
	sw	v1, 0x0(v0)

	li	v1, (0x7 << 19) 	//power down pll L1 first
	sw	v1, 0x0(v0)
	li	v1, (L1_DIV << 0)
	sw	v1, 0x8(v0)
#endif

	sw	a0, 4(v0)
#if 0
	li	v1, (L1_REFC << 26) | (SYS_PD << 19) | \
		    (ST_PLL << 22) | (0x3 << 10) | (0x1 << 7)
	sw	v1, 0(v0)
	or	v1, PLL_L1_ENA
	sw	v1, 0x0(v0)
1:	//wait_locked_sys:
	lw	v1, 0x0(v0)
	and	v1, PLL_L1_LOCKED
	beqz	v1, 1b //wait_locked_sys
	nop

	lw	v1, 0x0(v0)
	ori	v1, PLL_CHANG_COMMIT
	sw	v1, 0x0(v0)
#endif

	jr	ra
	nop

	.set pop
END(ls132_config_pll)

LEAF(timer_handler)
	.set    push
	.set    noreorder
	.set    mips32

	//disable interrupt
	mfc0	v0, COP_0_STATUS_REG
	and	v0, ~(SR_INT_ENAB)
	mtc0	v0, COP_0_STATUS_REG

	mfc0	v0, COP_0_STATUS_REG
	mfc0	v1, COP_0_CAUSE_REG
	and	v0, v1
	and	v1, v0, (1 << 15)
	beqz	v1, 1f
	nop
#if 0
	LS132_PUTC('T')
	LS132_PUTC('i')
	LS132_PUTC('m')
	LS132_PUTC('e')
	LS132_PUTC('r')
	LS132_PUTC('\r')
	LS132_PUTC('\n')
#endif
	//set compare register
	mfc0	v0, COP_0_COUNT
	addu	v0, COUNT_PERIOD
	mtc0	v0, COP_0_COMPARE
#if 0
	move	a0,v0
	bal	ls132_hexserial
	nop
#endif
	mfc0	v0, COP_0_STATUS_REG
	mfc0	v1, COP_0_CAUSE_REG
	and	v0, v1
1:
	and     v1, v0, (1 << 14) //IP6
	beqz	v1, 2f //undefined interrupt
	nop
#ifdef V_DEBUG
	LS132_PUTC('C')
	LS132_PUTC('u')
	LS132_PUTC(':')
	mfc0	a0, COP_0_CAUSE_REG
	bal	ls132_hexserial
	nop

	LS132_PUTC('I')
	LS132_PUTC('P')
	LS132_PUTC('6')
	LS132_PUTC('\r')
	LS132_PUTC('\n')
	li	v0, LS132_BASE_REG
	lw	v1, 0x0(v0)
	move	a0, v1
	bal	ls132_hexserial
	nop
	LS132_PUTC('M')
	LS132_PUTC('A')
	LS132_PUTC('I')
	LS132_PUTC('L')
	LS132_PUTC(':')
	li	v0, LS132_MAIL
	lw	a0, 0x0(v0)
	bal	ls132_hexserial
	nop
#endif
	//clear ls132 irq
	li	v0, LS132_BASE_REG
	lw	v1, 0x0(v0)
	and	v1, ~(1 << 10)
	sw	v1, 0x0(v0)
#ifdef V_DEBUG
	LS132_PUTC('c')
	LS132_PUTC('u')
	LS132_PUTC(':')
	mfc0	a0, COP_0_CAUSE_REG
	bal	ls132_hexserial
	nop
#endif

	li	v0, LS132_MAIL
	lw	v1, 0x0(v0)
	move	s0, v1
	and	v0, v1, COMMAND_FIELD
	li	a0, (VOLTAGE_COMMAND << 24)
	bne	v0, a0, 3f
	nop

	and	v0, v1, (TURBO_MODE | NORMAL_MODE)
	beqz	v0, 1f
	nop

	li	a0, TURBO_MODE
	and	a0, s0
	beqz	a0, 22f //normal mode
	nop
	//jump to configure voltage
	li	v0, LEVEL2
	b	11f
	nop
12:

	//turbo mode: update value of s3
	move	v1, s0
	/*get cpu 4 cores level*/
	and	v1, 0xffff
	move	s3, v1
	or	s3, (0x3 << 16) //highest level
	/*set turbo mode flag*/
	or	s3, TURBO_FLAG

	/*get cpu 4 cores level*/
	move	v1, s0
	and	a0, v1, 0xf
	li	a1, 10
	sub	a0, a1, a0
	/*v0 used to store clk value*/
	move	v0, a0

	srl	a0, v1, 4
	and	a0, 0xf
	sub	a0, a1, a0
	sll	a0, 4
	or	v0, a0

	srl	a0, v1, 8
	and	a0, 0xf
	sub	a0, a1, a0
	sll	a0, 8
	or	v0, a0

	srl	a0, v1, 12
	and	a0, 0xf
	sub	a0, a1, a0
	sll	a0, 12
	or	v0, a0
	or	v0, 0x8888

	li	v1, LS_FREQ_REG
	sh	v0, 0x0(v1)

	move	a0, s3
	bal	turbo_core_pll
	nop
	move	a0, v0
	b	23f
	nop
22:
	/*clear turbo mode flag*/
	and	s3, ~TURBO_FLAG

	li	a0, (L1_LOOPC << 24) | (L1_REFC << 18) | \
                    (L1_DIV << 10) | (L1_LOOPC << 0)
23:
	bal	ls132_config_pll
	nop

	li	a0, NORMAL_MODE
	and	a0, s0
	beqz	a0, 6f //TURBO_MODE is complete
	nop
	//normal mode  jump to configure voltage
	li	v0, LEVEL3
	b	11f
	nop
1:
	/*update s3 value*/
	and	v0, v1, CPU_ID_FIELD
	mul	v0, 4
	li	a0, 0xf
	sll	a0, v0
	not	a0
	and	s3, a0
	and	a0, v1, FREQ_FIELD
	srl	a0, FREQ_FIELD_OFFSET
	sll	a0, v0
	or	s3, a0

	move	a0, s3
	bal	ls132_v_cmp
	nop

	and	s3, ~FLAG_FIELD
	sll	v0, VOLTAGE_FIELD_OFFSET
	or	s3, v0
	/*ending update*/

	/* turbo mode pll changing code */
	and	v1, s3, TURBO_FLAG
	beqz	v1, 32f
	nop
	move	a0, s3
	bal	turbo_core_pll
	nop
	move	a0, v0
	bal	ls132_config_pll
	nop
32:
	and	v0, s3, VOLTAGE_UP
	beqz	v0, 4f
	nop

	and	v0, s3, VOLTAGE_FIELD
	srl	v0, VOLTAGE_FIELD_OFFSET
11:
	move	t0, v0
#if 0
#if defined(CPU_CLK_CLOSE) || defined(SET_SRAM)
	/*node clk close cause timer count error,now close cpu clk*/
	/*close cpu clock EN*/
	li	a0, LS_FREQ_REG
	lh	a1, 0x0(a0)
	/*use t1 store 1d0 register value, sometimes kernel hotplug should change some enable bit*/
	move	t1, a1
	and	a1, ~(1 << 3 | 1 << 7 | 1 << 11 | 1 << 15)
	sh	a1, 0x0(a0)
#endif
#endif
	
#ifdef SET_SRAM
	/*set sram and bbgen begin*/
	/*in up mode if voltage little than 1.1V cpu sram with bbgen was configuration completed */
	li	a0, 0xbfe00430
	lw	a1, 0x0(a0)
	beq	a1, SRAM_CTRL_VALUE, 40f
	nop

	/*sram was changed*/
	/*level 8 voltage value is 1.1V it is the ture judgment condition*/
	bgt	v0, SRAM_LEVEL, 40f
	nop

	/*set sram value*/
	li	a0, 0xbfe00430
	li	a1, SRAM_CTRL_VALUE
	sw	a1, 0x0(a0)


#ifdef BBGEN
	/*set bbgen value*/
	li	a0, 0xbfe001a0
	li	a1, (0xff << 16) | (VBBn_VAL << 12) | (VBBp_VAL << 8) | (FEEDBACK << 4) | (0x0 << 1) | 0x1
	sw	a1, 0x0(a0)
#endif
	/*set sram and bbgen end*/
	
40:
#endif
#ifdef NODE_CLK_CLOSE
	li	a0, 0xbfe10000 //close node clk
	li	a1, 1
	sw	a1, 0x0(a0)
#endif

	bal	v_ctrl
	nop
	//there need a delay
	nop
	nop
	nop
	nop
	nop
	nop
	sync

#if 0
	//only voltage up need this function
	move	v0, t0
	bal	v_ctrl_check
	nop
#endif


#ifdef NODE_CLK_CLOSE
	li	a0, 0xbfe10000 //open node clk
	sw	zero, 0x0(a0)
#endif

#if 0
#if defined(CPU_CLK_CLOSE) || defined(SET_SRAM)
	/*sram need this code,if not close,open cpu clock EN it can not cause error*/
	li	a0, LS_FREQ_REG
	sh	t1, 0x0(a0)
#endif
#endif

	li	a0, TURBO_MODE
	and	a0, s0
	beqz	a0, 15f
	nop
	//voltage level 2 but node freq need level 3
	addiu	t0, 1
15:
	/*start configure node clock*/
	move	v0, t0
	//level 0-2 reserve,only support 3-10
	li	a0, LEVEL3
	bltu	v0, a0, 1f
	nop
	li	a0, 10
	sub	v0, a0, v0
	sll	v0, 0x8
	//3a4000 now only support node 0 so give the offset fixed value
	li	v1, 0x8
	li	a0, 0x7
	sll	a0, v1
	not	a0

	li	v1, LS_NODE_FREQ_REG
	lw	a1, 0x0(v1)
	and	a0, a1
	or	a0, v0
	sw	a0, 0x0(v1)
	/*configure node clock done*/
#ifdef V_DEBUG
	LS132_PUTC('n')
	LS132_PUTC('o')
	LS132_PUTC('d')
	LS132_PUTC('e')
	LS132_PUTC(' ')
	LS132_PUTC('u')
	LS132_PUTC('p')
	LS132_PUTC('!')
	LS132_PUTC('\r')
	LS132_PUTC('\n')
#endif
	//turbo mode or normal ctrl
	li	a0, TURBO_MODE
	and	a0, s0
	bnez	a0, 12b
	nop
	li	a0, NORMAL_MODE
	and	a0, s0
	bnez	a0, 6f
	nop

	b	4f
	nop

1:	//up voltage reserve

4:	//configure freq
#ifdef V_DEBUG
	LS132_PUTC('F')
	LS132_PUTC('r')
	LS132_PUTC('e')
	LS132_PUTC('q')
	LS132_PUTC('\r')
	LS132_PUTC('\n')
#endif
	move	v1, s0
	and	v0, v1, FREQ_FIELD
	srl	v0, FREQ_FIELD_OFFSET

	//level 0-2 reserve,only support 3-10
	li	a0, LEVEL3
	bltu	v0, a0, 1f
	nop
	li	a0, 10
	sub	v0, a0, v0
	or	v0, 8 //div freq enable bit
	li	a0, CPU_ID_FIELD
	and	v1, a0
	mul	v1, 4
	sll	v0, v1
	li	a0, 0x7
	sll	a0, v1
	not	a0

	li	v1, LS_FREQ_REG
	lh	a1, 0x0(v1)
	and	a0, a1
	or	a0, v0
	sh	a0, 0x0(v1)
#ifdef V_DEBUG
	bal	ls132_hexserial
	nop
#endif
	b	5f
	nop

1:	//up voltage reserve

5:	//wait freq complete configure

	and	v0, s3, VOLTAGE_DOWN
	beqz	v0, 6f
	nop

	and	v0, s3, VOLTAGE_FIELD
	srl	v0, VOLTAGE_FIELD_OFFSET
	move	t0, v0

	//level 0-2 reserve,only support 3-10
	li	a0, LEVEL3
	bltu	v0, a0, 1f
	nop
	li	a0, 10
	sub	v0, a0, v0
	sll	v0, 0x8
	//3a4000 now only support node 0 so give the offset value dirctly
	li	v1, 0x8
	li	a0, 0x7
	sll	a0, v1
	not	a0

	li	v1, LS_NODE_FREQ_REG
	lw	a1, 0x0(v1)
	and	a0, a1
	or	a0, v0
	sw	a0, 0x0(v1)
	b	7f
	nop

1:	//up voltage reserve

7:
#if 0
#if defined(CPU_CLK_CLOSE) || defined(SET_SRAM)
	/*node clk close cause timer count error,now close cpu clk*/
	/*close cpu clock EN*/
	li	a0, LS_FREQ_REG
	lh	a1, 0x0(a0)
	/*use t1 store 1d0 register value, sometimes kernel hotplug should change some enable bit*/
	move	t1, a1
	and	a1, ~(1 << 3 | 1 << 7 | 1 << 11 | 1 << 15)
	sh	a1, 0x0(a0)
#endif
#endif
	
#ifdef SET_SRAM
	/*set sram and bbgen begin*/
	/*level 8 voltage value is 1.1V it is the ture judgment condition*/
	move	v0, t0
	ble	v0, SRAM_LEVEL, 40f
	nop

	li	a0, 0xbfe00430
	lw	a1, 0x0(a0)
	beq	a1, SRAM_CTRL_VALUE_LV, 40f
	nop

	/*set sram value*/
	li	a0, 0xbfe00430
	li	a1, SRAM_CTRL_VALUE_LV
	sw	a1, 0x0(a0)

#ifdef BBGEN	
	/*set bbgen value*/
	li	a0, 0xbfe001a0
	li	a1, ((0xff << 16) | (VBBn_VAL_LV << 12) | (VBBp_VAL_LV << 8) | (FEEDBACK << 4) | (0x0 << 1) | 0x1)
	sw      a1, 0x0(a0)
#endif
	/*set sram and bbgen end*/
40:
#endif
#ifdef NODE_CLK_CLOSE
	li	a0, 0xbfe10000 //close node clk
	li	a1, 1
	sw	a1, 0x0(a0)
#endif

	move	v0, t0
	bal	v_ctrl
	nop

	//there need a delay
	nop
	nop
	nop
	nop
	nop
	nop
	sync

#ifdef NODE_CLK_CLOSE
	li	a0, 0xbfe10000 //open node clk
	sw	zero, 0x0(a0)
#endif
#if 0
#if defined(CPU_CLK_CLOSE) || defined(SET_SRAM)
	/*sram need this code,if not close,open cpu clock EN it can not cause error*/
	li	a0, LS_FREQ_REG
	sh	t1, 0x0(a0)
#endif
#endif

6:
	li	v0, LS132_MAIL
	li	v1, COMPLETE_STATUS
	sw	v1, 0x0(v0)
	b	2f
	nop
3:
	and	v0, s0, COMMAND_FIELD
	li	a0, (FEATURE_COMMAND << 24)
	bne	v0, a0, 31f
	nop
	li	a0, ((TURBO_CORE_NUM  << 20) | (NOT_TURBO_CORE_LEVEL << 16) | ((TURBO_FREQ / 25) << 8) | (LOW_LEVEL << 4) | HIGH_LEVEL)
	or	a0, COMPLETE_STATUS

	li	v0, LS132_MAIL
	sw	a0, 0x0(v0)

	b	2f
	nop
31:
	and	v0, s0, COMMAND_FIELD
	li	a0, (TURBO_INFO_COMMAND << 24)
	bne	v0, a0, 8f
	nop
	li	a0, ((TURBO_2_CORES_SUB_FREQ  << 0) | (TURBO_3_CORES_SUB_FREQ << 4) | (TURBO_4_CORES_SUB_FREQ << 8))
	or	a0, COMPLETE_STATUS

	li	v0, LS132_MAIL
	sw	a0, 0x0(v0)

	b	2f
	nop
8:
	LS132_PUTC('u')
	LS132_PUTC('n')
	LS132_PUTC('k')
	LS132_PUTC('n')
	LS132_PUTC('o')
	LS132_PUTC('w')
	LS132_PUTC(' ')
	LS132_PUTC('c')
	LS132_PUTC('m')
	LS132_PUTC('m')
	LS132_PUTC('a')
	LS132_PUTC('n')
	LS132_PUTC('d')
	LS132_PUTC('\r')
	LS132_PUTC('\n')
2:
	//enable interrupt
	mfc0	v0, COP_0_STATUS_REG
	and	v0, ~SR_EXL
	or	v0, SR_INT_ENAB
	mtc0	v0, COP_0_STATUS_REG
	nop

1:
	wait
	nop

	b	1b
	nop
	.set pop
END(timer_handler)
#if 1
	/*wake up the ls132 by gs464e core */
LEAF(ls132_core_main)
	.set    push
	.set    noreorder
	.set    mips32
	mtc0	zero, COP_0_STATUS_REG
	mtc0	zero, COP_0_CAUSE_REG
	li	t0, SR_BOOT_EXC_VEC	/* Exception to Boostrap Location */
	mtc0	t0, COP_0_STATUS_REG

	li	a0, LS132_UART
	bal	ls132_initserial
	nop

	bal	ls132_i2cinit
	nop

	li	t1,LS132_MAIL
	sw	zero, 0x0(t1)
	li	v0, COMPLETE_STATUS
	sw	v0, 0x0(t1)
	
	/*clear s2 used to store status*/
	move	s2, zero
	/*clear s3 used to store freq level*/
	move	s3, zero
	/*level 3 is default level*/
	li	s3, 0x33333 

	LS132_PUTC('C')
	LS132_PUTC('O')
	LS132_PUTC('R')
	LS132_PUTC('E')
	LS132_PUTC(' ')
	LS132_PUTC('i')
	LS132_PUTC('n')
	LS132_PUTC('i')
	LS132_PUTC('t')
	LS132_PUTC(' ')
	LS132_PUTC('D')
	LS132_PUTC('o')
	LS132_PUTC('n')
	LS132_PUTC('e')
	LS132_PUTC('\r')
	LS132_PUTC('\n')

#if 1 //interrupt mode code
	//enable IP7 IP6 interrupt
	mfc0	v0, COP_0_STATUS_REG
	//or	v0, ((3 << 14) | (1 << 22)) //IP6 IP7 BEV
	or	v0, ((1 << 14) | (1 << 22)) //IP6 BEV
	mtc0	v0, COP_0_STATUS_REG

	//set compare register
	mfc0	v0, COP_0_COUNT
	addu	v0, COUNT_PERIOD
	mtc0	v0, COP_0_COMPARE

	//enable interrupt
	mfc0	v0, COP_0_STATUS_REG
	or	v0, SR_INT_ENAB
	mtc0	v0, COP_0_STATUS_REG

#if 0 //debug
2:
	LS132_PUTC('c')
	LS132_PUTC('a')
	LS132_PUTC('u')
	LS132_PUTC('s')
	LS132_PUTC('e')
	LS132_PUTC(':')
	mfc0	a0, COP_0_CAUSE_REG
	bal	ls132_hexserial
	nop

	LS132_PUTC('s')
	LS132_PUTC('r')
	LS132_PUTC(':')
	mfc0	a0, COP_0_STATUS_REG
	bal	ls132_hexserial
	nop

	LS132_PUTC('c')
	LS132_PUTC('o')
	LS132_PUTC('u')
	LS132_PUTC('n')
	LS132_PUTC('t')
	LS132_PUTC(':')
	mfc0	a0, COP_0_COUNT
	bal	ls132_hexserial
	nop

	LS132_PUTC('c')
	LS132_PUTC('o')
	LS132_PUTC('m')
	LS132_PUTC('p')
	LS132_PUTC('a')
	LS132_PUTC('r')
	LS132_PUTC('e')
	LS132_PUTC(':')
	mfc0	a0, COP_0_COMPARE
	bal	ls132_hexserial
	nop

	b	2b
	nop
#endif
	wait
	nop
#else	//poll mode code
#endif
	.set    pop
END(ls132_core_main)
#endif
