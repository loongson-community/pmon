#define DEBUG_LOCORE



#ifdef DEBUG_LOCORE
#define	TTYDBG(x) \
	.rdata;98: .asciz x; .text; la a0, 98b; bal stringserial; nop
#define	TTYDBG_COM1(x) \
    .rdata;98: .asciz x; .text; la a0, 98b; bal stringserial_COM1; nop
#else
#define TTYDBG(x)
#endif

#define	PRINTSTR(x) \
    .rdata;98: .asciz x; .text; la a0, 98b; bal stringserial; nop

#ifdef DEVBD2F_SM502
#define GPIOLED_DIR  0xe
#else
#define GPIOLED_DIR  0xf
#endif

#undef USE_GPIO_SERIAL
#ifndef USE_GPIO_SERIAL
#if 0 //do not used
#define GPIOLED_SET(x) \
	li v0,0xbfe0011c; \
lw v1,4(v0); \
or v1,0xf; \
xor v1,GPIOLED_DIR; \
sw v1,4(v0); \
li v1,(~x)&0xf;\
sw v1,0(v0);\
li v1,0x1000;\
78: \
subu v1,1;\
bnez v1,78b;\
nop;
#endif
#else
#define GPIOLED_SET(x)
#endif

/*1 sel GPIO function,otherwise sel HT function*/
#ifdef MULTI_CHIP
#ifdef LS7A_2WAY_CONNECT
#define HT0_LO_DISABLE_UNUSED	(0 << 1)	//HT0_LO_RSTn
#define HT0_HI_DISABLE_UNUSED	(0 << 5)	//HT0_HI_RSTn
#define HT1_LO_DISABLE_UNUSED	(0 << 9)	//HT1_LO_RSTn
#define HT1_HI_DISABLE_UNUSED	(1 << 13)	//HT1_HI_RSTn
#else
#define HT0_LO_DISABLE_UNUSED	(0 << 1)
#define HT0_HI_DISABLE_UNUSED	(0 << 5)
#define HT1_LO_DISABLE_UNUSED	(0 << 9)
#define HT1_HI_DISABLE_UNUSED	(0 << 13)
#endif
#else
#define HT0_LO_DISABLE_UNUSED	(1 << 1)
#define HT0_HI_DISABLE_UNUSED	(0 << 5)
#define HT1_LO_DISABLE_UNUSED	(0 << 9)
#define HT1_HI_DISABLE_UNUSED	(0 << 13)
#endif

#define DISABLE_UNUSED_HT_PIN	(HT0_LO_DISABLE_UNUSED | HT0_HI_DISABLE_UNUSED | HT1_LO_DISABLE_UNUSED | HT1_HI_DISABLE_UNUSED)

#define SET_GPIO_FUNC_EN(node,x) \
	dli     t0, 0x900000001fe00500 | (node  << 44); \
	lw	t1, 0x4(t0); \
	or	t1, x; \
	sw	t1, 0x4(t0);

/*Finish it as soon as possible.*/
#define UNUESED_HT_PIN_TO_GPIO(x) \
	li      t0, 0xbfe00500; \
	li      t1, x ; \
	sh	t1,  0x6(t0);

#define STABLE_COUNTER_CLK_EN(node) \
dli	t0, 0x900000001fe00420 | (node << 44); \
lw	t1, 0x4(t0); \
or	t1, (1 << 15);/*clken_stable*/ \
sw	t1, 0x4(t0);

#define SRAM_CTRL_VALUE 0x2be127a
/*
0x1e1040
0x1f18e5
0x1e9440
0x1e0020
0x1e1020
0x1e5020
0x1e0040
*/
//sram ctrl
#define LS3A4000_SRAM_CTRL(node) \
	dli	t0, 0x900000001fe00430 | (node  << 44); \
	li	t1, SRAM_CTRL_VALUE; \
	sw	t1, 0x0(t0);

#define SET_NODEMASK(node, value) \
	dli     t0, 0x900000003ff00400 | (node<<44); \
	ld      t2, 0x0(t0); \
	dli     t1, ~(0x7<<4); \
	and     t2,t1,t2; \
	dli     t1, (value<<4); \
	or      t2, t2, t1; \
	sd      t2, 0x0(t0);

#define ENABLE_XLINK(node) \
	dli     t0, 0x900000003ff00400 | (node<<44); \
	ld      t2, 0x0(t0); \
	dli     t1, (0x1<<8); \
	or      t2, t2, t1; \
	sd      t2, 0x0(t0); 

#define SET_HT_REG_DISABLE(node, value) \
	dli     t0, 0x900000003ff00400 | (node<<44); \
	ld      t2, 0x0(t0); \
	dli     t1, ~(0xf<<44); \
	and     t2,t1,t2; \
	dli     t1, (value<<44); \
	or      t2, t2, t1; \
	sd      t2, 0x0(t0);

/* set GPIO as output
 * x : 0x1<<offset
 */
#define GPIO_SET_OUTPUT(x) \
dli             v0,             0x900000001fe00500; \
lw              v1,             0x8(v0); \
or              v1,             x; \
sw              v1,             0x8(v0); \
lw              v1,             0x0(v0); \
or              v1,             x; \
xor             v1,             x; \
sw              v1,             0x0(v0); \
lw              v1,             0x4(v0); \
or              v1,             x; \
xor             v1,             x; \
sw              v1,             0x4(v0); \
nop; \
nop;

/* clear GPIO as output
 *  * x : 0x1 <<offsest
 *   */
#define GPIO_CLEAR_OUTPUT(x) \
dli             v0,             0x900000001fe00500; \
lw              v1,             0x8(v0); \
or              v1,             x; \
xor             v1,             x; \
sw              v1,             0x8(v0); \
lw              v1,             0x0(v0); \
or              v1,             x; \
xor             v1,             x; \
sw              v1,             0x0(v0); \
lw              v1,             0x4(v0); \
or              v1,             x; \
xor             v1,             x; \
sw              v1,             0x4(v0); \
nop; \
nop;

#ifdef LEMOTE_WATCHDOG
#define WatchDog_Close \
GPIO_CLEAR_OUTPUT(0x1<<8); 

#define WatchDog_Enable \
GPIO_SET_OUTPUT(0x1<<14); \
GPIO_SET_OUTPUT(0x1<<8); \
GPIO_CLEAR_OUTPUT(0x1<<14); \
li v1,0x100;\
78:; \
subu v1,1; \
bnez v1,78b; \
nop; \
GPIO_SET_OUTPUT(0x1<<13);

#elif !defined(MULTI_CHIP)
/* WatchDog Close for chip MAX6369*/
#define WatchDog_Close \
GPIO_CLEAR_OUTPUT(0x1<<5); \
GPIO_SET_OUTPUT(0x1<<6|0x1<<4); \
GPIO_CLEAR_OUTPUT(0x1<<13); \

/* WatchDog Enable for chip MAX6369*/
#define WatchDog_Enable \
GPIO_CLEAR_OUTPUT(0x1<<13); \
GPIO_SET_OUTPUT(0x1<<14); \
GPIO_SET_OUTPUT(0x1<<5); \
GPIO_SET_OUTPUT(0x1<<4); \
GPIO_CLEAR_OUTPUT(0x1<<6); \
GPIO_CLEAR_OUTPUT(0x1<<14); \
li v1,0x100;\
78:; \
subu v1,1; \
bnez v1,78b; \
nop; \
GPIO_SET_OUTPUT(0x1<<13);
#else  //4 way watchdog uesed gpio was changed
/* WatchDog Close for chip MAX6369*/
#define WatchDog_Close \
GPIO_CLEAR_OUTPUT(0x1<<5); \
GPIO_SET_OUTPUT(0x1<<6|0x1<<4); \
GPIO_CLEAR_OUTPUT(0x1<<6); \

/* WatchDog Enable for chip MAX6369*/
#define WatchDog_Enable \
GPIO_CLEAR_OUTPUT(0x1<<6); \
GPIO_SET_OUTPUT(0x1<<14); \
GPIO_SET_OUTPUT(0x1<<5); \
GPIO_CLEAR_OUTPUT(0x1<<4); \
GPIO_SET_OUTPUT(0x1<<6); \
GPIO_CLEAR_OUTPUT(0x1<<14); \
li v1,0x100;\
78:; \
subu v1,1; \
bnez v1,78b; \
nop; \
GPIO_SET_OUTPUT(0x1<<6);
#endif

#define w83627write(x,y,z) \
li		v0,		0xb800002e; \
li		v1,		0x87; \
sb		v1,		0(v0); \
sb		v1,		0(v0); \
li		v1,		0x7; \
sb		v1,		0(v0); \
li		v1,		x; \
sb		v1,		1(v0); \
li		v1,		y; \
sb		v1,		0(v0); \
li		v1,		z; \
sb		v1,		1(v0); \
li		v1,		0xaa; \
sb		v1,		0(v0); \
sb		v1,		0(v0); \
nop; \
nop 

#define CONFIG_CACHE_64K_4WAY 1

#define tmpsize		s1
#define msize		s2
#define bonito		s4
#define dbg			s5
#define sdCfg		s6


/*
 * Coprocessor 0 register names
 */
#define CP0_INDEX $0
#define CP0_RANDOM $1
#define CP0_ENTRYLO0 $2
#define CP0_ENTRYLO1 $3
#define CP0_CONF $3
#define CP0_CONTEXT $4
#define CP0_PAGEMASK $5
#define CP0_WIRED $6
#define CP0_INFO $7
#define CP0_BADVADDR $8
#define CP0_COUNT $9
#define CP0_ENTRYHI $10
#define CP0_COMPARE $11
#define CP0_STATUS $12
#define CP0_CAUSE $13
#define CP0_EPC $14
#define CP0_PRID $15
#define CP0_CONFIG $16
#define CP0_LLADDR $17
#define CP0_WATCHLO $18
#define CP0_WATCHHI $19
#define CP0_XCONTEXT $20
#define CP0_FRAMEMASK $21
#define CP0_DIAGNOSTIC $22
#define CP0_PERFORMANCE $25
#define CP0_ECC $26
#define CP0_CACHEERR $27
#define CP0_TAGLO $28
#define CP0_TAGHI $29
#define CP0_ERROREPC $30

#define CP0_DEBUG  $23
#define CP0_DEPC   $24
#define CP0_DESAVE $31

#define NODE0_CORE0_BUF0 0x900000003ff01000
#define NODE1_CORE0_BUF0 0x900010003ff01000
#define NODE2_CORE0_BUF0 0x900020003ff01000
#define NODE3_CORE0_BUF0 0x900030003ff01000

#define HT_REMOTE_NODE  0x900010003ff01000


#define Index_Store_Tag_I			0x08
#define Index_Store_Tag_D			0x09
#define Index_Store_Tag_V			0x0a
#define Index_Invalidate_I			0x00
#define Index_Writeback_Inv_D			0x01
#define Index_Store_Tag_S			0x0b
#define Index_Writeback_Inv_S			0x03

#define FN_OFF 0x020
#define SP_OFF 0x028
#define GP_OFF 0x030
#define A1_OFF 0x038


#define L2_CACHE_OK                 0x1111
#define L2_CACHE_DONE               0x2222
#define TEST_HT                     0x3333
#define NODE_MEM_INIT_DONE          0x4444
#define ALL_CORE0_INIT_DONE         0x5555
#define NODE_SCACHE_ENABLED         0x6666
#define SYSTEM_INIT_OK              0x5a5a

#define MIKU_CFG_TAB_ADDR   0xbd600800 // KSEG1
#define MIKU_CFG_OFF_CCFREQ 0xc
#define MIKU_CFG_OFF_CFDM   0x10
#define MIKU_CFG_OFF_PLL	0x14
#define MIKU_CFG_PLL_DIV_B	0
#define MIKU_CFG_PLL_DIV_F		(0xff << MIKU_CFG_PLL_DIV_B)
#define MIKU_CFG_PLL_LOOPC_B	8
#define MIKU_CFG_PLL_LOOPC_F	(0xffff << MIKU_CFG_PLL_LOOPC_B)
#define MIKU_CFG_PLL_REFC_B	24
#define MIKU_CFG_PLL_REFC_F		(0xff << MIKU_CFG_PLL_REFC_B)

#if defined MIKU_SMC && !defined MIKU_MODEL_MAGIC
#error  "MIKU_MODEL_MAGIC not defined"
#endif
