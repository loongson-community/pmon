/*
 *	loongson3_fixup.S
 *
 *	Multi chip HT Link configure
 *
 */

#ifdef MULTI_CHIP
//for 2/4 ways
/*ATTENTION: NO 16BIT mode when using HT1*/

#define HT0_RECONNECT
//#define DISABLE_X_LINK
//#define INTERCONNECTION_HT1
#define HT0_16BIT //NO 16Bit mode when HT1.0

//modify this macro for ht frequency change if using soft config pll
#ifdef BONITO_100M
//#define HT0_SOFT_FREQ       3600
#define HT0_SOFT_FREQ       3200
//#define HT0_SOFT_FREQ       2400
//#define HT0_SOFT_FREQ       1600
#elif  BONITO_25M
#define HT0_SOFT_FREQ       3175 //Max frequency when using 25MHz ref
#else
#define HT0_SOFT_FREQ       "undefined"
#endif

#define HT_HARD_FREQ 		HT_HARD_800M

//!!DO NOT CHANGE BELOW!!
#ifdef BONITO_100M
#if    (HT0_SOFT_FREQ <= 400)
#define HT0_PHY_DIV			8
#elsif (HT0_SOFT_FREQ <= 800)
#define HT0_PHY_DIV      	4
#elsif (HT0_SOFT_FREQ <= 1600)
#define HT0_PHY_DIV      	2
#else
#define HT0_PHY_DIV      	1
#endif

//#define HT0_DIV_REFC		1
#define HT0_DIV_REFC		2
#elif  BONITO_25M
#if    (HT0_SOFT_FREQ <= 600)
#define HT0_PHY_DIV			4
#elsif (HT0_SOFT_FREQ <= 1200)
#define HT0_PHY_DIV			2
#else
#define HT0_PHY_DIV			1
#endif

#define HT0_DIV_REFC		1
#else
#define HT0_PHY_DIV			"undefined"
#define HT0_DIV_REFC		"undefined"
#endif

#define HT0_PHY_LO_DIV		HT0_PHY_DIV
#define HT0_PHY_HI_DIV		HT0_PHY_DIV
#define HT0_DIV_LOOPC		(HT0_DIV_REFC*HT0_SOFT_FREQ*HT0_PHY_DIV/SYS_CLOCK)
#define HT0_CORE_DIV		8 //no use in 3A4000
#ifdef  INTERCONNECTION_HT1
#define HT0_PLL_CONFIG		0x00464083
#else
#define HT0_PLL_CONFIG		(HT0_PHY_LO_DIV << 22) | (HT0_PHY_HI_DIV << 18) | (HT0_DIV_REFC << 16) | (HT0_DIV_LOOPC << 9) | (HT0_CORE_DIV << 5) | 0x3;
#endif

#define NODE0				0x0
#define NODE1				0x1
#define NODE2				0x2
#define NODE3				0x3

#define HT0_Lo				0xa
#define HT0_Hi				0xb
#define HT1_Lo				0xe
#define HT1_Hi				0xf

#define HT_HARD_3200M		0xf
#define HT_HARD_2400M		0xd
#define HT_HARD_2200M		0xc
#define HT_HARD_2000M		0xb
#define HT_HARD_1800M		0xa
#define HT_HARD_1600M		0x9
#define HT_HARD_1200M		0x9
#define HT_HARD_800M		0x5
#define HT_HARD_200M		0x0

#define HT_LINK_8B			0
#define HT_LINK_16B			1
//!!DO NOT CHANGE ABOVE!!

#define RESET_HT(node,ht) \
	dli	t0, 0x900000fdfb000000 | node << 44 | ht << 40; \
	TTYDBG("Reset HT bus\r\n");   \
	lb      a0, 0x3e(t0);         \
	or      a0, 0x40;             \
	sb      a0, 0x3e(t0);         \
	lw      a0, 0x3c(t0);         \
	bal     hexserial;            \
	nop;                          \
	TTYDBG("\r\n");

#define DERESET_HT(node,ht)       \
	dli	t0, 0x900000fdfb000000 | node << 44 | ht << 40; \
	TTYDBG("Dereset HT bus\r\n"); \
	lb      a0, 0x3e(t0);         \
	and     a0, ~(0x40);          \
	sb      a0, 0x3e(t0);         \
	lw      a0, 0x3c(t0);         \
	bal     hexserial;            \
	nop;                          \
	TTYDBG("\r\n");

//Check the 0x44 for 0x1000 times if no error found
#define CHECK_HT(node,ht)         \
3:                                \
	dli	t0, 0x900000fdfb000000 | node << 44 | ht << 40; \
	lw      a0, 0x44(t0);         \
	and     t1, a0, 0xfffffcff;   \
	sw      t1, 0x44(t0);         \
	sync;				\
	li      a1, 0x1000;           \
1:                                \
	sub	a1, a1, 1;                \
	beqz	a1, 2f;               \
	nop;                          \
	lw      a0, 0x44(t0);         \
	and     t1, a0, 0x300;        \
	bnez    t1, 1f;               \
	nop;                          \
	and     a0, 0x20;             \
	beqz    a0, 1b;               \
	nop;                          \
	TTYDBG("\r\n");               \
	lw      a0, 0x44(t0);         \
	bal     hexserial;            \
	nop;                          \
	TTYDBG("\r\n");               \
	b	2f;                       \
	nop;                          \
1:                                \
	TTYDBG("CRC ERROR\r\n");      \
	RESET_HT(node,ht);            \
	dli	t0, 0x900000fdfb000000 | node << 44 | ht << 40; \
	lw      a0, 0x44(t0);         \
	sync;				\
	DERESET_HT(node,ht);          \
	b	3b;                       \
	nop;                          \
2:;

#define CHECK_HT_NOCRC(node,ht)   \
3:                                \
	dli	t0, 0x900000fdfb000000 | node << 44 | ht << 40; \
	li      a1, 0x1000;           \
1:                                \
	sub	a1, a1, 1;                \
	beqz	a1, 1f;               \
	nop;                          \
	lw      a0, 0x44(t0);         \
	and     a0, 0x20;             \
	beqz    a0, 1b;               \
	nop;                          \
	TTYDBG("\r\n");               \
	lw      a0, 0x44(t0);         \
	bal     hexserial;            \
	nop;                          \
	sw      zero, 0x44(t0);       \
	RESET_HT(node,ht);            \
	DERESET_HT(node,ht);          \
	lw      a0, 0x44(t0);         \
	bal     hexserial;            \
	nop;                          \
	TTYDBG("\r\n");               \
	b	2f;                       \
	nop;                          \
1:                                \
	RESET_HT(node,ht);            \
	sw      zero, 0x44(t0);       \
	DERESET_HT(node,ht);          \
	b	3b;                       \
	nop;                          \
2:;

#define SET_CH_INTERLEAVE(node, ht) \
	dli	t0, 0x900000fdfb0001d8 | node << 44 | ht << 40; \
	lw	a0, 0x00(t0);   \
	li	a1, 0xc0000000; \
	or	a0, a1;         \
	sw	a0, 0x00(t0);

#define SET_SOFT_FREQ(node, ht, pll_config) \
	li	t0, pll_config; \
	dli	t2, 0x900000fdfb000000 | node << 44 | ht << 40; \
	sw	t0, 0x1f4(t2); 

#define SET_HARD_FREQ(node, ht, freq) \
	li	t0, freq; \
	dli	t2, 0x900000fdfb000000 | node << 44 | ht << 40; \
	sb	t0, 0x49(t2);

#define SET_GEN3_MODE(node, ht) \
	dli	t2, 0x900000fdfb000000 | node << 44 | ht << 40; \
	li	t0, 0x88600000; \
	sw	t0, 0x6c(t2); 
//#define SET_GEN3_MODE(node, ht) \
//	dli	t2, 0x900000fdfb000000 | node << 44 | ht << 40; \
//	lw	t0, 0x6c(t2);    \
//	li	t1, ~(0xff<<16); \
//	and t0, t0, t1;      \
//	li	t1, 0x60<<16;    \
//	or  t0, t0, t1;      \
//	sw	t0, 0x6c(t2); 

#define SET_RETRY_MODE(node, ht) \
	dli	t2, 0x900000fdfb000000 | node << 44 | ht << 40; \
	li	t0, 0x81;     \
	sb	t0, 0x64(t2); 

#define SET_SCRAMBLING(node, ht) \
	dli	t2, 0x900000fdfb000000 | node << 44 | ht << 40; \
	lw	a0, 0xd0(t2); \
	li	t0, 1<<3;     \
	or	t0, t0, a0;   \
	sb	t0, 0xd0(t2); 

#define SET_8B10B(node, ht) \
	dli	t2, 0x900000fdfb000000 | node << 44 | ht << 40; \
	lw	a0, 0xd0(t2); \
	li	t0, 1<<2;     \
	or	t0, t0, a0;   \
	sb	t0, 0xd0(t2); 

#define SET_COHERENT_MODE(node, ht) \
	dli	t2, 0x900000fdfb000000 | node << 44 | ht << 40; \
	lw	t0, 0x1c0(t2); \
	or	t0, 1<<22;     \
	sw	t0, 0x1c0(t2); 

#define SET_HT_LINK_8B_16B(node, ht, mode) \
	dli	a0, 0x900000fdfb000000 | node << 44 | ht << 40; \
	lw  a1, 0x44(a0);            \
	li  a2, 0x88fffff;           \
	and a1, a1, a2;              \
	li  a2, mode<<24 | mode<<28; \
	or  a1, a1, a2;              \
	sw  a1, 0x44(a0);

#define HT0_WAIT_LINK_STATUS(node, ht, bit, label) \
	dli	t0, 0x900000fdfb000000 | node << 44 | ht << 40; \
	li	t2, 0x10;       \
	li	t1, 0x1f;       \
1:	                    \
	beqz	t2, label;  \
	nop;                \
	beqz	t1,2f;	    \
	TTYDBG(">");	    \
	addi	t1, t1, -1; \
	b	3f;             \
	nop;	            \
2:	                    \
	TTYDBG("\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b=");\
	li	t1, 0x1f;       \
	addi	t2, t2, -1; \
3:	                    \
	lw	a0, 0x44(t0);   \
	and	a0, 0x20;       \
	xor	a0, bit;        \
	beqz	a0, 1b;     \
	nop;	            \
	lw	t1, 0x68(t0);   \
	li  t3, 0x1000;     \
4:	addi	t3, t3, -1; \
	bnez    t3, 4b;     \
	nop;	            \
	lw	t2, 0x68(t0);   \
	bne t1, t2, label;  \
	nop;	            \
	TTYDBG("\r\n");	    \
	lw	a0, 0x44(t0);   \
	bal	hexserial;      \
	nop;	            \
	TTYDBG("\r\n");

#define SET_TX_PREENMP(node, ht) \
	dli   t0, 0x900000fdfb000000 | node << 44 | ht << 40; \
	lw    a0, 0x1fc(t0); \
	and   a0, a0,0xf0fe0fff; \
	li    a1, (0xc << 24 | 0x01 << 12); \
	or    a0, a0, a1; \
	sw    a0, 0x1fc(t0);

#define SET_SCANIN(node, ht) \
	dli   t0, 0x900000fdfb000000 | node << 44 | ht << 40; \
	li    a0, 0xef0ff000; \
	sw    a0, 0x1f8(t0);
#define HT_WAIT_LINK_UP(node, ht, label) HT0_WAIT_LINK_STATUS(node, ht, 0, label)
#define HT_WAIT_LINK_DOWN(node, ht, label) HT0_WAIT_LINK_STATUS(node, ht, 0x20, label)

/*
//Only A need to reset
//Reset HT links to connect multi chips
//Only reset ht0-lo of node0, because ht0 reset signal of all chips is tied
	TTYDBG("\r\nCheck node 0 HT0 bus up.")
	RESET_HT(NODE0,HT0_Lo)
	dli	t0, 0x90000afdfb000000
	sw      zero, 0x44(t0)
	DERESET_HT(NODE0,HT0_Lo)
	CHECK_HT_NOCRC(NODE0,HT0_Lo)

//2W fixup. When power up, node0_ht0_8x2 is 0, node1_ht0_8x2 is 1.
//After connected, change node1_ht0_8x2 to 0 and reset node1 cpus
//to make node1 requests can be routed correctly.
#ifndef CHIP_4
	GPIO_CLEAR_OUTPUT(1 << 1)

	dli t0, 0x900010001fe001d8
	li t1, 0x55
	sw	t1, 0(t0)

	li t2, 0x100
1:
	addu t2, t2, -1
	bnez t2, 1b
	nop

	li t1, 0x00
	sw	t1, 0(t0)

	li t2, 0x100
1:
	addu t2, t2, -1
	bnez t2, 1b
	nop

	li t1, 0x55
	sw	t1, 0(t0)

	li t2, 0x100
1:
	addu t2, t2, -1
	bnez t2, 1b
	nop

	li t1, 0xff
	sw	t1, 0(t0)
#endif
*/

	TTYDBG("READ CSR in NODE 1\r\n")
	dli	t2, 0x900010003ff00000
	lw  	a0, 0x10(t2)
	bal	hexserial
	nop
	TTYDBG("\r\n")
#ifdef CHIP_4
	TTYDBG("READ CSR in NODE 2\r\n")
	dli	t2, 0x900020003ff00000
	lw  	a0, 0x10(t2)
	bal	hexserial
	nop
	TTYDBG("\r\n")
	TTYDBG("READ CSR in NODE 3\r\n")
	dli	t2, 0x900030003ff00000
	lw  	a0, 0x10(t2)
	bal	hexserial
	nop
	TTYDBG("\r\n")
#endif

#ifdef SHUT_SLAVES
	TTYDBG("Shut down CPU1\r\n")
	dli	a0, 0x900010001fe001d0
	li	a1, 0x0
	sw	a1, 0x0(a0)

#ifdef CHIP_4
	TTYDBG("Shut down CPU2\r\n")
	dli	a0, 0x900020001fe001d0
	li	a1, 0x0
	sw	a1, 0x0(a0)

	TTYDBG("Shut down CPU3\r\n")
	dli	a0, 0x900030001fe001d0
	li	a1, 0x0
	sw	a1, 0x0(a0)
#endif	
#endif	

/* SET HT connection interleave between two LS3A chipses */ 
	TTYDBG("Setting HT b interleave and nop interleave\r\n")
	SET_CH_INTERLEAVE(NODE0, HT0_Lo);
	SET_CH_INTERLEAVE(NODE1, HT0_Lo);

#ifdef CHIP_4
	SET_CH_INTERLEAVE(NODE0, HT0_Hi);

	SET_CH_INTERLEAVE(NODE1, HT0_Hi);

	SET_CH_INTERLEAVE(NODE2, HT0_Lo);
	SET_CH_INTERLEAVE(NODE2, HT0_Hi);

	SET_CH_INTERLEAVE(NODE3, HT0_Lo);
	SET_CH_INTERLEAVE(NODE3, HT0_Hi);

#ifndef DISABLE_X_LINK
	SET_CH_INTERLEAVE(NODE0, HT1_Hi);
	SET_CH_INTERLEAVE(NODE1, HT1_Hi);
	SET_CH_INTERLEAVE(NODE2, HT1_Hi);
	SET_CH_INTERLEAVE(NODE3, HT1_Hi);
#endif	
#endif	

#ifdef HT0_16BIT
	TTYDBG("Setting HT link to 16 bit mode\r\n")
	SET_HT_LINK_8B_16B(NODE0, HT0_Lo, HT_LINK_16B);
	SET_HT_LINK_8B_16B(NODE1, HT0_Lo, HT_LINK_16B);
#else
	TTYDBG("Setting HT link to 8 bit mode\r\n")
	SET_HT_LINK_8B_16B(NODE0, HT0_Lo, HT_LINK_8B);
	SET_HT_LINK_8B_16B(NODE1, HT0_Lo, HT_LINK_8B);
#endif

#ifdef CHIP_4
	SET_HT_LINK_8B_16B(NODE0, HT0_Hi, HT_LINK_8B);

	SET_HT_LINK_8B_16B(NODE1, HT0_Hi, HT_LINK_8B);

	SET_HT_LINK_8B_16B(NODE2, HT0_Lo, HT_LINK_8B);
	SET_HT_LINK_8B_16B(NODE2, HT0_Hi, HT_LINK_8B);

	SET_HT_LINK_8B_16B(NODE3, HT0_Lo, HT_LINK_8B);
	SET_HT_LINK_8B_16B(NODE3, HT0_Hi, HT_LINK_8B);

#ifndef DISABLE_X_LINK
	SET_HT_LINK_8B_16B(NODE0, HT1_Hi, HT_LINK_8B);
	SET_HT_LINK_8B_16B(NODE1, HT1_Hi, HT_LINK_8B);
	SET_HT_LINK_8B_16B(NODE2, HT1_Hi, HT_LINK_8B);
	SET_HT_LINK_8B_16B(NODE3, HT1_Hi, HT_LINK_8B);
#endif	
#endif	

#ifdef CHIP_4
#ifndef DISABLE_X_LINK
	TTYDBG("Setting HT1_Hi coherent mode\r\n")
	SET_COHERENT_MODE(NODE0, HT1_Hi);
	SET_COHERENT_MODE(NODE1, HT1_Hi);
	SET_COHERENT_MODE(NODE2, HT1_Hi);
	SET_COHERENT_MODE(NODE3, HT1_Hi);
#endif	
#endif	

#ifdef HT0_RECONNECT

#########TEST CLKSEL[9]
	li	t2, 0xbfe00194
	lw	t1, 0x0(t2)
	andi	t1, 0x8000   //3a3000/3a4000 ht frequency (software)
	bnez	t1, no_softconfig_ht
	nop

	TTYDBG("Setting HyperTransport bus frequency by SOFT config\r\n")
	TTYDBG("SOFT Set HT0 freq\r\n")
	li      a0, HT0_SOFT_FREQ
	bal     hexserial;
	nop;
	TTYDBG("\r\n")

	SET_SOFT_FREQ(NODE0, HT0_Lo, HT0_PLL_CONFIG);
	SET_SOFT_FREQ(NODE1, HT0_Lo, HT0_PLL_CONFIG);

#ifdef CHIP_4
	SET_SOFT_FREQ(NODE0, HT0_Hi, HT0_PLL_CONFIG);
	SET_SOFT_FREQ(NODE1, HT0_Hi, HT0_PLL_CONFIG);

	SET_SOFT_FREQ(NODE2, HT0_Lo, HT0_PLL_CONFIG);
	SET_SOFT_FREQ(NODE2, HT0_Hi, HT0_PLL_CONFIG);

	SET_SOFT_FREQ(NODE3, HT0_Lo, HT0_PLL_CONFIG);
	SET_SOFT_FREQ(NODE3, HT0_Hi, HT0_PLL_CONFIG);
#endif	
	b	config_others	
	nop

/*software use the follow code for read some debug info.*/
no_softconfig_ht:

	TTYDBG("Setting HyperTransport bus frequency by HARD config\r\n")
	TTYDBG("HARD Set HT0 freq\r\n")
	li      a0, HT_HARD_FREQ
	bal     hexserial;
	nop;
	TTYDBG("\r\n")
	SET_HARD_FREQ(NODE0, HT0_Lo, HT_HARD_FREQ);
	SET_HARD_FREQ(NODE1, HT0_Lo, HT_HARD_FREQ);

#ifdef CHIP_4
	SET_HARD_FREQ(NODE0, HT0_Hi, HT_HARD_FREQ);
	SET_HARD_FREQ(NODE1, HT0_Hi, HT_HARD_FREQ);

	SET_HARD_FREQ(NODE2, HT0_Lo, HT_HARD_FREQ);
	SET_HARD_FREQ(NODE2, HT0_Hi, HT_HARD_FREQ);

	SET_HARD_FREQ(NODE3, HT0_Lo, HT_HARD_FREQ);
	SET_HARD_FREQ(NODE3, HT0_Hi, HT_HARD_FREQ);
#ifndef DISABLE_X_LINK
	SET_HARD_FREQ(NODE0, HT1_Hi, HT_HARD_FREQ);
	SET_HARD_FREQ(NODE1, HT1_Hi, HT_HARD_FREQ);
	SET_HARD_FREQ(NODE2, HT1_Hi, HT_HARD_FREQ);
	SET_HARD_FREQ(NODE3, HT1_Hi, HT_HARD_FREQ);
#endif	
#endif	

config_others:

#ifdef INTERCONNECTION_HT1
	TTYDBG("Enable HyperTransport Controller GEN1 mode\r\n"); 

#else
	TTYDBG("Enable HyperTransport Controller GEN3 mode\r\n");
	SET_GEN3_MODE(NODE0, HT0_Lo);
	SET_GEN3_MODE(NODE1, HT0_Lo);

#ifdef CHIP_4
	SET_GEN3_MODE(NODE0, HT0_Hi);

	SET_GEN3_MODE(NODE1, HT0_Hi);

	SET_GEN3_MODE(NODE2, HT0_Lo);
	SET_GEN3_MODE(NODE2, HT0_Hi);

	SET_GEN3_MODE(NODE3, HT0_Lo);
	SET_GEN3_MODE(NODE3, HT0_Hi);
#ifndef DISABLE_X_LINK
	SET_GEN3_MODE(NODE0, HT1_Hi);
	SET_GEN3_MODE(NODE1, HT1_Hi);
	SET_GEN3_MODE(NODE2, HT1_Hi);
	SET_GEN3_MODE(NODE3, HT1_Hi);
#endif	
#endif

	TTYDBG("Setting HyperTransport Controller retry mode\r\n")
	SET_RETRY_MODE(NODE0, HT0_Lo);
	SET_RETRY_MODE(NODE1, HT0_Lo);

#ifdef CHIP_4
	SET_RETRY_MODE(NODE0, HT0_Hi);

	SET_RETRY_MODE(NODE1, HT0_Hi);

	SET_RETRY_MODE(NODE2, HT0_Lo);
	SET_RETRY_MODE(NODE2, HT0_Hi);

	SET_RETRY_MODE(NODE3, HT0_Lo);
	SET_RETRY_MODE(NODE3, HT0_Hi);
#ifndef DISABLE_X_LINK
	SET_RETRY_MODE(NODE0, HT1_Hi);
	SET_RETRY_MODE(NODE1, HT1_Hi);
	SET_RETRY_MODE(NODE2, HT1_Hi);
	SET_RETRY_MODE(NODE3, HT1_Hi);
#endif	
#endif

//#define HT_8B10B
#ifdef HT_8B10B
	TTYDBG("Enable HyperTransport Controller 8B/10B\r\n")
	SET_8B10B(NODE0, HT0_Lo);
	SET_8B10B(NODE1, HT0_Lo);

#ifdef CHIP_4
	SET_8B10B(NODE0, HT0_Hi);

	SET_8B10B(NODE1, HT0_Hi);

	SET_8B10B(NODE2, HT0_Lo);
	SET_8B10B(NODE2, HT0_Hi);

	SET_8B10B(NODE3, HT0_Lo);
	SET_8B10B(NODE3, HT0_Hi);
#ifndef DISABLE_X_LINK
	SET_8B10B(NODE0, HT1_Hi);
	SET_8B10B(NODE1, HT1_Hi);
	SET_8B10B(NODE2, HT1_Hi);
	SET_8B10B(NODE3, HT1_Hi);
#endif	
#endif
#endif

	TTYDBG("Enable HyperTransport Controller scrambling\r\n")
	SET_SCRAMBLING(NODE0, HT0_Lo);
	SET_SCRAMBLING(NODE1, HT0_Lo);

#ifdef CHIP_4
	SET_SCRAMBLING(NODE0, HT0_Hi);

	SET_SCRAMBLING(NODE1, HT0_Hi);

	SET_SCRAMBLING(NODE2, HT0_Lo);
	SET_SCRAMBLING(NODE2, HT0_Hi);

	SET_SCRAMBLING(NODE3, HT0_Lo);
	SET_SCRAMBLING(NODE3, HT0_Hi);
#ifndef DISABLE_X_LINK
	SET_SCRAMBLING(NODE0, HT1_Hi);
	SET_SCRAMBLING(NODE1, HT1_Hi);
	SET_SCRAMBLING(NODE2, HT1_Hi);
	SET_SCRAMBLING(NODE3, HT1_Hi);
#endif	
#endif

#endif //HT 3.0

#ifdef CHIP_4
	//Optimize the signal for HT(CPU-CPU)
	SET_TX_PREENMP(NODE0, HT0_Lo);
	SET_TX_PREENMP(NODE0, HT0_Hi);
	SET_TX_PREENMP(NODE0, HT1_Lo);
	SET_TX_PREENMP(NODE0, HT1_Hi);

	SET_TX_PREENMP(NODE1, HT0_Lo);
	SET_TX_PREENMP(NODE1, HT0_Hi);
	SET_TX_PREENMP(NODE1, HT1_Lo);
	SET_TX_PREENMP(NODE1, HT1_Hi);

	SET_TX_PREENMP(NODE2, HT0_Lo);
	SET_TX_PREENMP(NODE2, HT0_Hi);
	SET_TX_PREENMP(NODE2, HT1_Lo);
	SET_TX_PREENMP(NODE2, HT1_Hi);

	SET_TX_PREENMP(NODE3, HT0_Lo);
	SET_TX_PREENMP(NODE3, HT0_Hi);
	SET_TX_PREENMP(NODE3, HT1_Lo);
	SET_TX_PREENMP(NODE3, HT1_Hi);

	SET_SCANIN(NODE0, HT0_Lo);
	SET_SCANIN(NODE0, HT0_Hi);
	SET_SCANIN(NODE0, HT1_Lo);
	SET_SCANIN(NODE0, HT1_Hi);

	SET_SCANIN(NODE1, HT0_Lo);
	SET_SCANIN(NODE1, HT0_Hi);
	SET_SCANIN(NODE1, HT1_Lo);
	SET_SCANIN(NODE1, HT1_Hi);

	SET_SCANIN(NODE2, HT0_Lo);
	SET_SCANIN(NODE2, HT0_Hi);
	SET_SCANIN(NODE2, HT1_Lo);
	SET_SCANIN(NODE2, HT1_Hi);

	SET_SCANIN(NODE3, HT0_Lo);
	SET_SCANIN(NODE3, HT0_Hi);
	SET_SCANIN(NODE3, HT1_Lo);
	SET_SCANIN(NODE3, HT1_Hi);
#endif

reset_ht0:
	RESET_HT(NODE0,HT0_Lo)
	DERESET_HT(NODE0,HT0_Lo)
	CHECK_HT(NODE0,HT0_Lo); //reset HT0 links for all nodes

	//wait until CPU1 HT link up
	TTYDBG("Waiting CPU0 HT0_Lo HyperTransport bus to be up.")
	HT_WAIT_LINK_UP(NODE0, HT0_Lo, reset_ht0);
	TTYDBG("Waiting CPU1 HT0_Lo HyperTransport bus to be up.")
	HT_WAIT_LINK_UP(NODE1, HT0_Lo, reset_ht0);

#ifdef CHIP_4
	TTYDBG("Waiting CPU0 HT0_Hi HyperTransport bus to be up.")
	HT_WAIT_LINK_UP(NODE0, HT0_Hi, reset_ht0);
	TTYDBG("Waiting CPU1 HT0_Hi HyperTransport bus to be up.")
	HT_WAIT_LINK_UP(NODE1, HT0_Hi, reset_ht0);
	TTYDBG("Waiting CPU2 HT0_Lo HyperTransport bus to be up.")
	HT_WAIT_LINK_UP(NODE2, HT0_Lo, reset_ht0);
	TTYDBG("Waiting CPU2 HT0_Hi HyperTransport bus to be up.")
	HT_WAIT_LINK_UP(NODE2, HT0_Hi, reset_ht0);
	TTYDBG("Waiting CPU3 HT0_Lo HyperTransport bus to be up.")
	HT_WAIT_LINK_UP(NODE2, HT0_Lo, reset_ht0);
	TTYDBG("Waiting CPU3 HT0_Hi HyperTransport bus to be up.")
	HT_WAIT_LINK_UP(NODE2, HT0_Hi, reset_ht0);
#endif

#ifdef CHIP_4
#ifndef DISABLE_X_LINK
//	/* 
//	  This config only set freq for node2->node1 x-link and node3->node0 x-link. 
//	  For node1->node2 and node0->node3 x-link, the freq is controlled by 
//	  node0/1 HT1_Lo, which is used by 3A-7A HT link, so for these links, 
//	  freq is according to 3A-7A HT link freq.
//	*/
#define HT1_LO_SETTING 0x93c10a
	SET_SOFT_FREQ(NODE2, HT1_Lo, HT0_PLL_CONFIG); // Lo actually controls the PLL
	SET_SOFT_FREQ(NODE3, HT1_Lo, HT0_PLL_CONFIG); // Lo actually controls the PLL
	//SET_SOFT_FREQ(NODE2, HT1_Lo, HT1_LO_SETTING); // Lo actually controls the PLL
	//SET_SOFT_FREQ(NODE3, HT1_Lo, HT1_LO_SETTING); // Lo actually controls the PLL

//reset release of HT1 hi are moved to the HT1 init, for HT1 lo is reset there
	TTYDBG("Reset Node 0 HT1-hi bus\r\n")
	dli     t5, 0x90000ffdfb000000
	lb      a0, 0x3e(t5)
	li      a1, 0x40
	or      a0, a0, a1
	sb      a0, 0x3e(t5)
	lw      a0, 0x3c(t5)
	bal     hexserial
	nop
	TTYDBG("Wait HT bus down...")
1:
	lw      a0, 0x44(t5)
	li      a1, 0x20
	and     a0, a0, a1
	bnez    a0, 1b
	nop
	TTYDBG("link down!\r\n")
	
	TTYDBG("Reset Node 1 HT1-hi bus\r\n")
	dli     t5, 0x90001ffdfb000000
	lb      a0, 0x3e(t5)
	li      a1, 0x40
	or      a0, a0, a1
	sb      a0, 0x3e(t5)
	lw      a0, 0x3c(t5)
	bal     hexserial
	nop
	TTYDBG("Wait HT bus down...")
1:
	lw      a0, 0x44(t5)
	li      a1, 0x20
	and     a0, a0, a1
	bnez    a0, 1b
	nop
	TTYDBG("link down!\r\n")

	TTYDBG("Reset node2 HT1_Lo\r\n")
	RESET_HT(NODE2,HT1_Lo); //to make node2 ht1 pll work
	TTYDBG("Reset node3 HT1_Lo\r\n")
	RESET_HT(NODE3,HT1_Lo); //to make node3 ht1 pll work

//No need to release HT1-lo of NODE 2/3, for these two are not connected
	//TTYDBG("De Reset node2 HT1_Lo\r\n")
	//DERESET_HT(NODE2,HT1_Lo); //to make node2 ht1 pll work
	//TTYDBG("De Reset node3 HT1_Lo\r\n")
	//DERESET_HT(NODE3,HT1_Lo); //to make node3 ht1 pll work

#endif
#endif


##################################################
#endif //HT0_RECONNECT
#endif
