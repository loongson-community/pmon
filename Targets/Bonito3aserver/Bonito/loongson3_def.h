#define DEBUG_LOCORE
#undef USE_GPIO_SERIAL

#ifdef DDR3_DIMM
#define USE_SB_I2C
#else
#define USE_GPIO_I2C
#define MULTI_I2C_BUS
#endif

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

#ifndef USE_GPIO_SERIAL
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
#else
#define GPIOLED_SET(x)
#endif
/* set GPIO as output
 * x : 0x1<<offset
 */
#define GPIO_SET_OUTPUT(x) \
li		v0,		0xbfe0011c; \
lw		v1,		0(v0); \
or		v1,		x&0xffff; \
xor		v1,		0x0; \
sw		v1,		0(v0); \
lw		v1,		4(v0); \
or		v1,		x&0xffff; \
xor		v1,		x; \
sw		v1,		4(v0); \
nop; \
nop;

/* clear GPIO as output
 * x : 0x1 <<offsest
 */
#define GPIO_CLEAR_OUTPUT(x) \
li		v0,		0xbfe0011c; \
lw		v1,		0(v0); \
or		v1,		x&0xffff; \
xor		v1,		x; \
sw		v1,		0(v0); \
lw		v1,		4(v0); \
or		v1,		x&0xffff; \
xor		v1,		x; \
sw		v1,		4(v0); \
nop; \
nop;

#ifdef DDR3_DIMM
/* WatchDog Close for chip MAX6369*/
#define WatchDog_Close \
GPIO_CLEAR_OUTPUT(0x1<<13); \
GPIO_CLEAR_OUTPUT(0x1<<14); \
GPIO_CLEAR_OUTPUT(0x1<<6 | 0x1<<5|0x1<<4); \
GPIO_SET_OUTPUT(0x1<<3);

#if 0
li v1,0x1000;\
78:; \
subu v1,1; \
bnez v1,78b; \
nop; \
GPIO_CLEAR_OUTPUT(0x1<<14);
#endif 

/* WatchDog Enable for chip MAX6369*/
#define WatchDog_Enable \
GPIO_SET_OUTPUT(0x1<<4 | 0x1<<5 | 0x1 << 6 | 0x1<<13); \
GPIO_CLEAR_OUTPUT(0x1<<13); \
li v1,0x100;\
78:; \
subu v1,1; \
bnez v1,78b; \
nop; \
GPIO_SET_OUTPUT(0x1<<14); \
li v1,0x1000;\
78:; \
subu v1,1; \
bnez v1,78b; \
nop; \
GPIO_CLEAR_OUTPUT(0x1<<14);

#else

  /* WatchDog Close for chip MAX6369*/
  #define WatchDog_Close \
  GPIO_CLEAR_OUTPUT(0x1<<13); \
  GPIO_SET_OUTPUT(0x1<<14); \
  GPIO_CLEAR_OUTPUT(0x1<<5); \
  GPIO_SET_OUTPUT(0x1<<3|0x1<<4); \
  li v1,0x1000;\
  78:; \
  subu v1,1; \
  bnez v1,78b; \
  nop; \
  GPIO_CLEAR_OUTPUT(0x1<<14);

  /* WatchDog Enable for chip MAX6369*/
  #define WatchDog_Enable \
  GPIO_SET_OUTPUT(0x1<<14); \
  GPIO_SET_OUTPUT(0x1<<5); \
  GPIO_CLEAR_OUTPUT(0x1<<4); \
  GPIO_SET_OUTPUT(0x1<<3); \
  li v1,0x100;\
  78:; \
  subu v1,1; \
  bnez v1,78b; \
  nop; \
  GPIO_CLEAR_OUTPUT(0x1<<14); \
  li v1,0x1000;\
  78:; \
  subu v1,1; \
  bnez v1,78b; \
  nop; \
  GPIO_SET_OUTPUT(0x1<<13); 

#endif

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

/*BOOTCORE_ID at [0, 3]*/
#define	BOOTCORE_ID 0

#define Index_Store_Tag_I			0x08
#define Index_Store_Tag_D			0x09
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

