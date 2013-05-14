/*whd : loongson3_ddr2_config.S
        used to set up all ddr controllers
        and set up the memory space on L2 Xbar
*/

/* below options means only one memory controler is used */
//#define MC0_ONLY		  
//#define MC1_ONLY
#define MC1X2_ONLY

/* below options indicated both memory controler is used defaulty */

//#define INTERLEAVE_27
//#define INTERLEAVE_13
//#define INTERLEAVE_12
#define INTERLEAVE_11
//#define INTERLEAVE_10
//#define NO_INTERLEAVE

/* Size of each DDR controller */
//#define DDR_256
//#define DDR_512
//#define DDR_1G
#define DDR_2G

/* Micro for printf address X */
/* This micro will use: a0/a1/a2/a3 v0/s0 */
#define PPADDR(x)   \
        li	a0, ((x)>>32); \
        bal	hexserial; nop; \
        li	a0, ((x)); \
        bal	hexserial; nop; \
        TTYDBG("-"); \
        dli	a0, (x); lw     a0, 0x4(a0) ; bal	hexserial; nop; \
        dli	a0, (x); lw     a0, 0x0(a0) ; bal	hexserial; nop; \
        TTYDBG(" "); 

/* Micro for printf address X: 0x0, 0x40, 0x80. used for printf */
/* This micro will use: a0/a1/a2/a3 v0/s0, because it only call PPADDR */
#define PPWIN(x)   \
	PPADDR(x+0x0)  ; PPADDR(x+0x40) ; PPADDR(x+0x80); TTYDBG("\r\n"); \
	PPADDR(x+0x8)  ; PPADDR(x+0x48) ; PPADDR(x+0x88); TTYDBG("\r\n"); \
	PPADDR(x+0x10) ; PPADDR(x+0x50) ; PPADDR(x+0x90); TTYDBG("\r\n"); \
	PPADDR(x+0x18) ; PPADDR(x+0x58) ; PPADDR(x+0x98); TTYDBG("\r\n"); \
	PPADDR(x+0x20) ; PPADDR(x+0x60) ; PPADDR(x+0xa0); TTYDBG("\r\n"); \
	PPADDR(x+0x28) ; PPADDR(x+0x68) ; PPADDR(x+0xa8); TTYDBG("\r\n"); \
	PPADDR(x+0x30) ; PPADDR(x+0x70) ; PPADDR(x+0xb0); TTYDBG("\r\n"); \
	PPADDR(x+0x38) ; PPADDR(x+0x78) ; PPADDR(x+0xb8); TTYDBG("\r\n"); 

	//dli t1, 0x900000003ff02000
	// show L1X BAR
	PPWIN(0x900000003ff02000);
	PPWIN(0x900000003ff02100);
	PPWIN(0x900000003ff02200);
	PPWIN(0x900000003ff02300);
	PPWIN(0x900000003ff02400);
	PPWIN(0x900000003ff02700);
	// show L2X BAR
	PPWIN(0x900000003ff00000);
	PPWIN(0x900000003ff00100);

#if 0
	//dli t1, 0x900010003ff06000
	// show L1X BAR
	PPWIN(0x900010003ff06000);
	PPWIN(0x900010003ff06100);
	PPWIN(0x900010003ff06200);
	PPWIN(0x900010003ff06300);
	PPWIN(0x900010003ff06400);
	PPWIN(0x900010003ff06700);
	// show L2X BAR
	PPWIN(0x900010003ff04000);
#endif


