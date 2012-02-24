/*	$Id: start.S,v 1.1.1.1 2006/09/14 01:59:08 root Exp $ */

/*
 * Copyright (c) 2001 Opsycon AB  (www.opsycon.se)
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by Opsycon AB, Sweden.
 * 4. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */

#ifndef _KERNEL
#define _KERNEL
#endif

#include <asm.h>
#include <regnum.h>
#include <cpu.h>
#include <pte.h>

#include "pmon/dev/ns16550.h"
#include "target/prid.h"
#include "target/sbd.h"
#include "target/bonito.h"
#include "target/via686b.h"
#include "target/i8254.h"
#include "target/isapnpreg.h"
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

/* Beep ON/OFF Function */
#define BEEP_ON \
	dli t1, 0x90000cfdfe00a080; \
	lbu t0, 0x0(t1); \
	or  t0, 0x4; \
	sb  t0, 0x0(t1); \
	nop; \
	nop;
#define BEEP_OFF \
	dli t1, 0x90000cfdfe00a080; \
	lbu t0, 0x0(t1); \
	and t0, 0xfb; \
	sb  t0, 0x0(t1); \
	nop; \
	nop;

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

/* WatchDog Close for chip MAX6369*/
#define WatchDog_Close \
GPIO_CLEAR_OUTPUT(0x1<<5); \
GPIO_SET_OUTPUT(0x1<<3|0x1<<4); \
GPIO_CLEAR_OUTPUT(0x1<<13); \

/* WatchDog Enable for chip MAX6369*/
#define WatchDog_Enable \
GPIO_CLEAR_OUTPUT(0x1<<13); \
GPIO_SET_OUTPUT(0x1<<14); \
GPIO_SET_OUTPUT(0x1<<5); \
GPIO_CLEAR_OUTPUT(0x1<<4); \
GPIO_SET_OUTPUT(0x1<<3); \
GPIO_CLEAR_OUTPUT(0x1<<14); \
li v1,0x100;\
78:; \
subu v1,1; \
bnez v1,78b; \
nop; \
GPIO_SET_OUTPUT(0x1<<13); 

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
#define msize		s2	//NOTICE : s2 used to as temperature
#define msize_node1	s3	//NOTICE : s3 used to as temperature
#define bonito		s4
#define dbg			s5
#define sdCfg		s6

#define CP0_CONFIG $16
#define CP0_TAGLO  $28
#define CP0_TAGHI  $29

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

/*
 *   Register usage:
 *
 *	s0	link versus load offset, used to relocate absolute adresses.
 *	s1	free
 *	s2	memory size.
 *	s3	free.
 *	s4	Bonito base address.
 *	s5	dbg.
 *	s6	sdCfg.
 *	s7	rasave.
 *	s8	L3 Cache size.
 */


.set	noreorder
.globl	_start
.globl	start
.globl	__main
_start:
start:
.globl	stack
stack = start - 0x4000		/* Place PMON stack below PMON start in RAM */



/////////////////////////////////////////////////////////////

#if 0
		.set 	mips64
		mfc0	t0, $15, 1
		.set 	mips3

		andi	t0, 0x3ff

		li		t1, 0x0
1:
		//bne		t0, t1, 1b ;
		b	  1b;
		nop	 


#if 0
	li	t1,0xaff00040
	li	t2,0x2
    sd  t2,0x0(t1)
	sync
	nop
	nop
	nop

#endif
	li	t1,0xaff00060
	li	t2,0x2
    sd  t2,0x0(t1)
	sync
	nop
	nop
	nop
1:
#endif

/* NOTE!! Not more that 16 instructions here!!! Right now it's FULL! */
mtc0	zero, COP_0_STATUS_REG
mtc0	zero, COP_0_CAUSE_REG
li	t0, SR_BOOT_EXC_VEC	/* Exception to Boostrap Location */
mtc0	t0, COP_0_STATUS_REG
la	sp, stack
la	gp, _gp

//	bal	uncached		/* Switch to uncached address space */
	nop

	/* WatchDog chip MAX6369 disable work */
	WatchDog_Close

	bal	locate			/* Get current execute address */
	nop

	uncached:
	or	ra, UNCACHED_MEMORY_ADDR
	j	ra
	nop

	/*
	 *  Reboot vector usable from outside pmon.
	 */
	.align	8
	ext_map_and_reboot:
	bal	CPU_TLBClear
	nop

	li	a0, 0xc0000000
	li	a1, 0x40000000
	bal	CPU_TLBInit
	nop
	la	v0, tgt_reboot
	la	v1, start
	subu	v0, v1
	lui	v1, 0xffc0
	addu	v0, v1
	jr	v0
	nop

	/*
	 *  Exception vectors here for rom, before we are up and running. Catch
	 *  whatever comes up before we have a fully fledged exception handler.
	 */
	.align	9			/* bfc00200 */
	la	a0, v200_msg
	bal	stringserial
	nop
	b	exc_common

	.align	7			/* bfc00280 */
	la	a0, v280_msg
	bal	stringserial
	nop
	b	exc_common

	/* Cache error */
	.align	8			/* bfc00300 */
	PRINTSTR("\r\nPANIC! Unexpected Cache Error exception! ")
	mfc0	a0, COP_0_CACHE_ERR
	bal	hexserial
	nop
	b	exc_common

	/* General exception */
	.align	7			/* bfc00380 */
	la	a0, v380_msg
	bal	stringserial
	nop
	b	exc_common

	.align	8			/* bfc00400 */
	la	a0, v400_msg
	bal	stringserial
	nop
#if 1
	b	exc_common
	nop

/* Debug exception */
	.align	7			/* bfc00480 */
////////////////////////////////////////////
#if 0
1:
	mfc0  a0, COP_0_STATUS_REG
	b	  1b
	nop
#endif
#include "exc_ejtag.S"
////////////////////////////////////////////
#endif
	
exc_common:
	PRINTSTR("\r\nCAUSE=")
	mfc0	a0, COP_0_CAUSE_REG
	bal	hexserial
	nop
	PRINTSTR("\r\nSTATUS=")
	mfc0	a0, COP_0_STATUS_REG
	bal	hexserial
	nop
	PRINTSTR("\r\nERRORPC=")
	mfc0	a0, COP_0_ERROR_PC
	bal	hexserial
	nop
	PRINTSTR("\r\nEPC=")
	mfc0	a0, COP_0_EXC_PC
	bal	hexserial
	nop

1:
	b 1b
	nop

//#ifndef ROM_EXCEPTION
#if 0
	PRINTSTR("\r\nDERR0=")
	cfc0	a0, COP_0_DERR_0
	bal	hexserial
	nop
	PRINTSTR("\r\nDERR1=")
	cfc0	a0, COP_0_DERR_1
	bal	hexserial
	nop
##else
##1:
##	b 1b
##	mfc0	$0, COP_0_STATUS_REG
#endif

	//b	ext_map_and_reboot
1:
	/* added below lines to let it response to 0XBFC00480 interrupt if in neted loops */
	b 1b
	mfc0	$0, COP_0_STATUS_REG


	.align 8
	nop
	.align 8
	.word read
	.word write
	.word open
	.word close
	.word nullfunction
	.word printf
	.word vsprintf
	.word nullfunction
	.word nullfunction
	.word getenv
	.word nullfunction
	.word nullfunction
	.word nullfunction
	.word nullfunction


	/*
	 *  We get here from executing a bal to get the PC value of the current execute
	 *  location into ra. Check to see if we run from ROM or if this is ramloaded.
	 */
	locate:

	li	s0, 0xbfe00198;	// temperature register
	ld	s2, 0x0(s0); //save it in s2

	la	s0,start
	subu	s0,ra,s0
	and	s0,0xffff0000

	li	t0,SR_BOOT_EXC_VEC
	mtc0	t0,COP_0_STATUS_REG
	mtc0    zero,COP_0_CAUSE_REG
	.set noreorder

li	bonito,PHYS_TO_UNCACHED(BONITO_REG_BASE)
	lpc_write_test:

loop_here:

		//Open 64-bit address space
		mfc0    t0, CP0_STATUS
		li      t1, 0x00e0      # {cu3,cu2,cu1,cu0}<={0110, status_fr<=1
	  	or      t0, t0, t1
		mtc0    t0, CP0_STATUS

/**********************************************************************/
/* below added for test whether every core can always fetch code right
/**********************************************************************/
/* used t0: core number
 * used t1: counter for wait
 /**********************************************************************/
#define BOOT_LOOP 0x100
#define CORE0_BOOT_LOOP 0x200
#define CORE1_BOOT_LOOP 0x800
#define CORE2_BOOT_LOOP 0x400

#if 0
	// 0xd is not bad
    dli a0,0xd #using 32:31
    dli t0,0x900000003ff00400
    sd  a0,0x0(t0)
    dli t0,0x900010003ff04400
    sd  a0,0x0(t0)

#endif


#if 1
	.set 	mips64
	mfc0	t0, $15, 1
	.set 	mips3
	andi	t0, 0x7

	beq		t0, 0x0, core0_loop
	nop

	
	beq		t0, 0x1, core1_loop
	nop

	beq		t0, 0x2, core2_loop
	nop

	b		out_bootloop
	nop

#if 0
	beq		t0, 0x3, core3_loop
	nop

	beq		t0, 0x4, core4_loop
	nop
	beq		t0, 0x5, core5_loop
	nop
	beq		t0, 0x6, core6_loop
	nop
	beq		t0, 0x7, core7_loop
	nop
#endif

core0_loop:
	li     t1, CORE0_BOOT_LOOP 
1:
    subu   t1, 1
    bnez   t1, 1b
    nop
	b		out_bootloop
	nop

core1_loop:
	li     t1, CORE1_BOOT_LOOP
1:
    subu   t1, 1
    bnez   t1, 1b
    nop
	b		out_bootloop
	nop

core2_loop:
	li     t1, CORE2_BOOT_LOOP
1:
    subu   t1, 1
    bnez   t1, 1b
    nop
	b		out_bootloop
	nop

core3_loop:
	li     t1, BOOT_LOOP 
1:
    //subu   t1, 1
    bnez   t1, 1b
    nop

#if 0
core4_loop:
	li     t1, BOOT_LOOP 
1:
    subu   t1, 1
    bnez   t1, 1b
    nop

core5_loop:
	li     t1, BOOT_LOOP 
1:
    subu   t1, 1
    bnez   t1, 1b
    nop

core6_loop:
	li     t1, BOOT_LOOP 
1:
    subu   t1, 1
    bnez   t1, 1b
    nop

core7_loop:
	li     t1, BOOT_LOOP 
1:
    subu   t1, 1
    bnez   t1, 1b
    nop
#endif
out_bootloop:
#endif
#if 1
	li	t1,0xaff00000
	li	t2,0x2
    sd  t2,0x0(t1)
	sync
	nop
	nop
	nop


	li	t1,0xaff00020
	li	t2,0x2
    sd  t2,0x0(t1)
	sync
	nop
	nop
	nop


	li	t1,0xaff00040
	li	t2,0x2
    sd  t2,0x0(t1)
	sync
	nop
	nop
	nop

	li	t1,0xaff00060
	li	t2,0x2
    sd  t2,0x0(t1)
	sync
	nop
	nop
	nop

#endif
		mfc0    t0, CP0_STATUS
		li      t1, 0x00e0      # {cu3,cu2,cu1,cu0}<={0110, status_fr<=1
	  	or      t0, t0, t1
		mtc0    t0, CP0_STATUS

#if 0
	dli	t1,0x90000efdfb000100
	li	t2,0x2
    sd  t2,0x0(t1)
	sync
	nop
	nop
	nop

	dli	t1,0x90000efdfb000120
    sd  t2,0x0(t1)
	sync
	nop
	nop
	nop


	dli	t1,0x90000efdfb000140
    sd  t2,0x0(t1)
	sync
	nop
	nop
	nop

	dli	t1,0x90000efdfb000160
    sd  t2,0x0(t1)
	sync
	nop
	nop
	nop

	dli	t1,0x90000efdfb000180
	li	t2,0x2
    sd  t2,0x0(t1)
	sync
	nop
	nop
	nop

	dli	t1,0x90000efdfb0001a0
    sd  t2,0x0(t1)
	sync
	nop
	nop
	nop


	dli	t1,0x90000efdfb0001c0
    sd  t2,0x0(t1)
	sync
	nop
	nop
	nop

	dli	t1,0x90000efdfb0001e0
    sd  t2,0x0(t1)
	sync
	nop
	nop
	nop

#endif
/**************************************************/
#define NODE1_BOOT 1
//#define SHUTDOWN_CORE

#define NODE0_CORE0_BUF0 0x900000003ff01000   
#define NODE1_CORE0_BUF0 0x900010003ff05000   
#define NODE2_CORE0_BUF0 0x900020003ff09000   
#define NODE3_CORE0_BUF0 0x900030003ff0d000   

#define FN_OFF 0x020
#define SP_OFF 0x028
#define GP_OFF 0x030
#define A1_OFF 0x038
/**************************************************/

/**************************************************/
/* Clear Mail BOX of every core                   */
/**************************************************/
		.set 	mips64
		mfc0	t0, $15, 1
		.set 	mips3

		andi	t0, 0x3ff
		andi    t3, t0,0xc

		beq		t3, 0x0, node0;
		nop	 

		beq		t3, 0x4, node1;
		nop	 

		beq		t3, 0x8, node2;
		nop	 

		beq		t3, 0xc, node3;
		nop	 

node0:
        dli     t1, NODE0_CORE0_BUF0;
		b		common;
		nop

node1:
        dli     t1, NODE1_CORE0_BUF0;
		b		common;
		nop

node2:
        dli     t1, NODE2_CORE0_BUF0;
		b		common;
		nop

node3:
        dli     t1, NODE3_CORE0_BUF0;
		b		common;
		nop

common:
        andi    t3, t0, 0x3  #local cpuid
        dsll    t3, 8
        or      t1, t1, t3

        dli     a0, 0x0
        sd      a0,FN_OFF(t1)

/*******************************************************/
// below test added for multi-core inst fetching
/*******************************************************/
#if 0 
	  all_wait:
		  b	all_wait;
		  nop
#endif
/*******************************************************/
// above test added for multi-core inst fetching
/*******************************************************/
#ifdef NODE1_BOOT
        dli     a0, 0x4 
#else
        dli     a0, 0x0;
#endif
        bne     t0, a0, slave_main
		nop


// shut down slave core of 3B
#ifdef  SHUTDOWN_CORE
        li	  a0, 0xbfe00184
#ifdef NODE1_BOOT
	    li	  t1, 0x0010ffff
#else
	    li	  t1, 0x0001ffff
#endif
		sw	  t1, 0x0(a0)
#endif

/*******************************************************/
// below added for test scache interleave
/*******************************************************/
#if 0
	// 0xd is not bad
    dli a0,0xd #using 11:10
    dli t0,0x900000003ff00400
    sd  a0,0x0(t0)
    dli t0,0x900010003ff04400
    sd  a0,0x0(t0)

#endif


/*******************************************************/
// below added for test which scache routing has bug
/*******************************************************/
#if 0
    dli a0,0x0 #using 6:5
    dli t0,0x900000003ff00400
    sd  a0,0x0(t0)
	sync
	nop
	nop
	nop

	dli	t1,0x900000000ff00000
	dli	t2,0x0
    sd  t2,0x0(t1)
	sync
	nop
	nop
	nop

	dli	t1,0x900000000ff00020
	dli	t2,0x1
    sd  t2,0x0(t1)
	sync
	nop
	nop
	nop

	
	dli	t1,0x900000000ff00040
	dli	t2,0x2
    sd  t2,0x0(t1)
	sync
	nop
	nop
	nop

	
	dli	t1,0x900000000ff00060
	dli	t2,0x2
    sd  t2,0x0(t1)
	sync
	nop
	nop
	nop


    dli a0,0x1 #using 9:8
    dli t0,0x900000003ff00400
    sd  a0,0x0(t0)
	sync
	nop
	nop
	nop



	dli	t1,0x900000000ff00000
	dli	t2,0x0
    sd  t2,0x0(t1)
	sync
	nop
	nop
	nop

	dli	t1,0x900000000ff00100
	dli	t2,0x1
    sd  t2,0x0(t1)
	sync
	nop
	nop
	nop
	
	dli	t1,0x900000000ff00200
	dli	t2,0x2
    sd  t2,0x0(t1)
	sync
	nop
	nop
	nop
	
	dli	t1,0x900000000ff00300
	dli	t2,0x2
    sd  t2,0x0(t1)
	sync
	nop
	nop
	nop


    dli a0,0x2 #using 11:10
    dli t0,0x900000003ff00400
    sd  a0,0x0(t0)
	sync
	nop
	nop
	nop

	dli	t1,0x900000000ff00000
	dli	t2,0x0
    sd  t2,0x0(t1)
	sync
	nop
	nop
	nop

	dli	t1,0x900000000ff00400
	dli	t2,0x1
    sd  t2,0x0(t1)
	sync
	nop
	nop
	nop
	
	dli	t1,0x900000000ff00800
	dli	t2,0x2
    sd  t2,0x0(t1)
	sync
	nop
	nop
	nop
	
	dli	t1,0x900000000ff00c00
	dli	t2,0x2
    sd  t2,0x0(t1)
	sync
	nop
	nop
	nop

    dli a0,0x3 #using 13:12
    dli t0,0x900000003ff00400
    sd  a0,0x0(t0)

	dli	t1,0x900000000ff00000
	dli	t2,0x0
    sd  t2,0x0(t1)
	sync
	nop
	nop
	nop

	dli	t1,0x900000000ff01000
	dli	t2,0x1
    sd  t2,0x0(t1)
	sync
	nop
	nop
	nop
	
	dli	t1,0x900000000ff02000
	dli	t2,0x2
    sd  t2,0x0(t1)
	sync
	nop
	nop
	nop
	
	dli	t1,0x900000000ff03000
	dli	t2,0x2
    sd  t2,0x0(t1)
	sync
	nop
	nop
	nop

#endif
/*******************************************************/
// above added for test which scache has bug
/*******************************************************/

/**************************************************/

//#define DEBUG_LS3
#ifdef DEBUG_LS3
		dli     a0, 0xfff
		and     t1, t1, a0
		dsll    t1, t1, 10
		1:
		bnez    t1, 1b
		daddi   t1, t1, -1

		bal	initserial
		nop

		PRINTSTR("CP0_STATUS:")
		mfc0    a0, CP0_STATUS
		bal	    hexserial
		nop
		PRINTSTR("\r\n\r\n")


		sync
		sync
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		PRINTSTR("sync ok\r\n")
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		1:
		bnez    t0, 1b
		nop
#endif


#if 0 /* Niuxie Tube */
		li      a0,0xbff00080
		li      t0,0xcc
		sb      t0,0x0(a0)
#endif

#ifdef DEBUG_LS3
#else
		bal	initserial
		nop
#endif

#if 0
		dli	t2, 0x900000003ff00000
		dli	t1, 0x900000003ff08000
1:
		ld	a0, 0x0(t2)
		bal	hexserial64
		nop
		
		daddi  t2,0x100
		bne	  t2,t1,1b;
		nop

#endif

#ifdef DEBUG_LS3
		dli     t2, 0x9800000000000000
		cache   21, 0x0(t2)
		PRINTSTR("Hit WB Invalidate_D 0 \r\n")
		dli     t2, 0x9800000000000020
		cache   21, 0x0(t2)
		PRINTSTR("Hit WB Invalidate_D 2 \r\n")
		dli     t2, 0x9800000000000040
		cache   21, 0x0(t2)
		PRINTSTR("Hit WB Invalidate_D 4 \r\n")
		dli     t2, 0x9800000000000060
		cache   21, 0x0(t2)
		PRINTSTR("Hit WB Invalidate_D 6 \r\n")

		dli     t2, 0x9800000000000000
		ld      a0, 0x0(t2)
		PRINTSTR("Read 0 \r\n")
		dli     t2, 0x9800000000000020
		ld      a0, 0x0(t2)
		PRINTSTR("Read 2 \r\n")
		dli     t2, 0x9800000000000040
		ld      a0, 0x0(t2)
		PRINTSTR("Read 4 \r\n")
		dli     t2, 0x9800000000000060
		ld      a0, 0x0(t2)
		PRINTSTR("Read 6 \r\n")

		dli     t1, 0x9800000000000000
		dli     t2, 0x9800000000400000
		1:
		ld      a0, 0x0(t1)
		daddiu  t1, t1, 0x20
		bne     t1, t2, 1b
		nop
		PRINTSTR("Scache scan done \r\n")

		1:
		b       1b
		nop

#endif


#if 0
		PRINTSTR("Shut down other cores\r\n")
		li      a0, 0xbfe00180
		lw      a1, 0x0(a0)
		li      t1, 0xffff1fff
		and     a1, a1, t1
		sw      a1, 0x0(a0)

#endif

		li      a0,0x900000001ff00080
		li      t0,0x99
		sb      t0,0x0(a0)

#ifdef LS3_HT
		b       core0_start
		nop
#endif

#define MOD_MASK	0x00000003
#define MOD_B		0x00000000 /* byte "modifier" */
#define MOD_H		0x00000001 /* halfword "modifier" */
#define MOD_W		0x00000002 /* word "modifier" */
#if __mips64
#define MOD_D		0x00000003 /* doubleword "modifier" */
#endif

#define OP_MASK		0x000000fc
#define	OP_EXIT		0x00000000 /* exit (status) */
#define OP_DELAY	0x00000008 /* delay (cycles) */
#define OP_RD		0x00000010 /* read (addr) */
#define OP_WR		0x00000014 /* write (addr, val) */
#define OP_RMW		0x00000018 /* read-modify-write (addr, and, or) */
#define OP_WAIT		0x00000020 /* wait (addr, mask, value) */

#define WR_INIT(mod,addr,val) \
		.word	OP_WR|mod,PHYS_TO_UNCACHED(addr);\
		.word	(val),0

#define RD_INIT(mod,addr) \
		.word	OP_RD|mod,PHYS_TO_UNCACHED(addr);\
		.word	0,0

#define RMW_INIT(mod,addr,and,or) \
		.word	OP_RMW|mod,PHYS_TO_UNCACHED(addr);\
		.word	(and),(or)

#define WAIT_INIT(mod,addr,and,or) \
		.word	OP_WAIT|mod,PHYS_TO_UNCACHED(addr);\
		.word	(mask),(val)

#define DELAY_INIT(cycles) \
		.word	OP_DELAY,(cycles);\
		.word	0,0

#define EXIT_INIT(status) \
		.word	OP_EXIT,(status);\
		.word	0,0

#define BONITO_INIT(r,v) WR_INIT(MOD_W,BONITO_BASE+/**/r,v)
#define BONITO_BIS(r,b) RMW_INIT(MOD_W,BONITO_BASE+(r),~0,b)
#define BONITO_BIC(r,b) RMW_INIT(MOD_W,BONITO_BASE+(r),~(b),0)
#define BONITO_RMW(r,c,s) RMW_INIT(MOD_W,BONITO_BASE+(r),~(c),s)

#define CFGADDR(idsel,function,reg) ((1<<(11+(idsel)))+((function)<<8)+(reg))
#define _ISABWR_INIT(mod,function,isabreg,val) \
		WR_INIT(MOD_W,BONITO_BASE+BONITO_PCIMAP_CFG,CFGADDR(PCI_IDSEL_VIA686B,function,isabreg)>>16) ; \
		RD_INIT(MOD_W,BONITO_BASE+BONITO_PCIMAP_CFG) ; \
		WR_INIT(mod,PCI_CFG_SPACE+(CFGADDR(PCI_IDSEL_VIA686B,function,isabreg)&0xffff),val)

#define _ISABRD_INIT(mod,function,isabreg) \
		WR_INIT(MOD_W,BONITO_BASE+BONITO_PCIMAP_CFG,CFGADDR(PCI_IDSEL_VIA686B,function,isabreg)>>16) ; \
		RD_INIT(MOD_W,BONITO_BASE+BONITO_PCIMAP_CFG) ; \
		RD_INIT(mod,PCI_CFG_SPACE+(CFGADDR(PCI_IDSEL_VIA686B,function,isabreg)&0xffff))


#define _ISAWR_INIT(isareg,val) \
		WR_INIT(MOD_B,PCI_IO_SPACE+(isareg),val)

#define _ISARD_INIT(isareg) \
		RD_INIT(MOD_B,PCI_IO_SPACE+(isareg))


#define ISABBWR_INIT(function,isabreg,val) \
		_ISABWR_INIT(MOD_B,function,(isabreg),val)
#define ISABHWR_INIT(function,isabreg,val) \
		_ISABWR_INIT(MOD_H,function,(isabreg),val)
#define ISABWWR_INIT(function,isabreg,val) \
		_ISABWR_INIT(MOD_W,function,isabreg,val)
#define ISAWR_INIT(isareg,val) \
		_ISAWR_INIT(isareg,val)
#define ISARD_INIT(isareg) \
		_ISARD_INIT(isareg)

		bal	1f
		nop

		/* bonito endianess */
		BONITO_BIC(BONITO_BONPONCFG,BONITO_BONPONCFG_CPUBIGEND)
		BONITO_BIC(BONITO_BONGENCFG,BONITO_BONGENCFG_BYTESWAP|BONITO_BONGENCFG_MSTRBYTESWAP)
		BONITO_BIS(BONITO_BONPONCFG, BONITO_BONPONCFG_IS_ARBITER)

		/*
		 * In certain situations it is possible for the Bonito ASIC
		 * to come up with the PCI registers uninitialised, so do them here
		 */
#define PCI_CLASS_BRIDGE		0x06
#define PCI_CLASS_SHIFT			24
#define PCI_SUBCLASS_BRIDGE_HOST	0x00
#define PCI_SUBCLASS_SHIFT		16
#define PCI_COMMAND_IO_ENABLE		0x00000001
#define PCI_COMMAND_MEM_ENABLE		0x00000002
#define PCI_COMMAND_MASTER_ENABLE	0x00000004
#define PCI_COMMAND_STATUS_REG		0x04
#define PCI_MAP_IO			0X00000001
#define PCI_CFG_SPACE			BONITO_PCICFG_BASE

		BONITO_INIT(BONITO_PCICLASS,(PCI_CLASS_BRIDGE << PCI_CLASS_SHIFT) | (PCI_SUBCLASS_BRIDGE_HOST << PCI_SUBCLASS_SHIFT))
		BONITO_INIT(BONITO_PCICMD, BONITO_PCICMD_PERR_CLR|BONITO_PCICMD_SERR_CLR|BONITO_PCICMD_MABORT_CLR|BONITO_PCICMD_MTABORT_CLR|BONITO_PCICMD_TABORT_CLR|BONITO_PCICMD_MPERR_CLR)
		//BONITO_INIT(BONITO_PCILTIMER, 0)
		BONITO_INIT(BONITO_PCILTIMER, 255)
		BONITO_INIT(BONITO_PCIBASE0, 0)
		BONITO_INIT(BONITO_PCIBASE1, 0)
		BONITO_INIT(BONITO_PCIBASE2, 0)
		BONITO_INIT(BONITO_PCIEXPRBASE, 0)
		BONITO_INIT(BONITO_PCIINT, 0)

		BONITO_INIT(0x150,0x8000000c)
		BONITO_INIT(0x154,0xffffffff)

		BONITO_BIS(BONITO_PCICMD, BONITO_PCICMD_PERRRESPEN)

		BONITO_BIS(BONITO_PCICMD, PCI_COMMAND_IO_ENABLE|PCI_COMMAND_MEM_ENABLE|PCI_COMMAND_MASTER_ENABLE)

		BONITO_BIC(BONITO_BONGENCFG, 0x80)

#BONITO_BIS(BONITO_BONGENCFG, BONITO_BONGENCFG_BUSERREN)

		/* Set debug mode */
		BONITO_BIS(BONITO_BONGENCFG, BONITO_BONGENCFG_DEBUGMODE)

		/******** added to init southbridge*/

#if  (PCI_IDSEL_VIA686B != 0)
		/* Set the SMB base address */
		ISABWWR_INIT(4, SMBUS_IO_BASE_ADDR, SMBUS_IO_BASE_VALUE | 0x1)
		/* enable the host controller */
		ISABHWR_INIT(4, SMBUS_HOST_CONFIG_ADDR, SMBUS_HOST_CONFIG_ENABLE_BIT)
		/* enable the SMB IO ports */
		ISABBWR_INIT(4, PCI_COMMAND_STATUS_REG, PCI_COMMAND_IO_ENABLE)

		/* 15us ISA bus refresh clock */
#define ISAREFRESH (PT_CRYSTAL/(1000000/15))
		ISARD_INIT(CTC_PORT+PT_CONTROL)

		/* program i8254 ISA refresh counter */
		ISAWR_INIT(CTC_PORT+PT_CONTROL,PTCW_SC(PT_REFRESH)|PTCW_16B|PTCW_MODE(MODE_RG))
		ISAWR_INIT(CTC_PORT+PT_REFRESH, ISAREFRESH & 0xff)
		ISAWR_INIT(CTC_PORT+PT_REFRESH, ISAREFRESH >> 8)
#endif

		EXIT_INIT(0)


#define	Init_Op	0
#define	Init_A0	4
#define	Init_A1	8
#define	Init_A2	12
#define	Init_Size	16

		1:	move a0,ra
		reginit:			/* local name */
		lw	t3, Init_Op(a0)
		lw	t0, Init_A0(a0)
		and	t4,t3,OP_MASK


		/*
		 * EXIT(STATUS)
		 */
		bne	t4, OP_EXIT, 8f
		nop
		move	v0,t0
		b	.done
		nop

		/*
		 * DELAY(CYCLES)
		 */
		8:	bne	t4, OP_DELAY, 8f
		nop
		1:	/////bnez	t0,1b
		subu	t0,1
		b	.next
		nop
		/*
		 * READ(ADDR)
		 */
		8:	bne	t4,OP_RD,8f
		nop
		and	t4,t3,MOD_MASK

		bne	t4,MOD_B,1f
		nop
		lbu	t5,0(t0)
		b	.next
		nop
		1:	bne	t4,MOD_H,1f
		nop
		lhu	t5,0(t0)
		b	.next
		nop
		1:	bne	t4,MOD_W,1f
		nop
#if __mips64
		lwu	t5,0(t0)
#else
		lw	t5,0(t0)
#endif
		b	.next
		nop
		1:
#if __mips64
		lw	t5,0(t0)
		b	.next
		nop
#else
		b	.fatal
		nop
#endif

		/*
		 * WRITE(ADDR,VAL)
		 */
		8:	bne	t4,OP_WR,8f
		nop
		lw	t1,Init_A1(a0)
		and	t4,t3,MOD_MASK

		bne	t4,MOD_B,1f
		nop
		sb	t1,0(t0)
		b	.next
		nop
		1:	bne	t4,MOD_H,1f
		nop
		sh	t1,0(t0)
		b	.next
		nop
		1:	bne	t4,MOD_W,1f
		nop
		sw	t1,0(t0)
		b	.next
		nop

		1:
#if __mips64
		sd	t1,0(t0)
		b	.next
		nop
#else
		b	.fatal
		nop
#endif


		/*
		 * RMW(ADDR,AND,OR)
		 */
		8:	bne	t4,OP_RMW,8f
		nop
		lw	t1,Init_A1(a0)
		lw	t2,Init_A2(a0)
		and	t4,t3,MOD_MASK

		bne	t4,MOD_B,1f
		nop
		lbu	t4,0(t0)
		and	t4,t1
		or	t4,t2
		sb	t4,0(t0)
		b	.next
		nop
		1:	bne	t4,MOD_H,1f
		nop
		lhu	t4,0(t0)
		and	t4,t1
		or	t4,t2
		sh	t4,0(t0)
		b	.next
		nop
		1:	bne	t4,MOD_W,1f
		nop
		lw	t4,0(t0)
		and	t4,t1
		or	t4,t2
		sw	t4,0(t0)
		b	.next
		nop

		1:
#if __mips64
		ld	t4,0(t0)
		and	t4,t1
		or	t4,t2
		sd	t4,0(t0)
		b	.next
		nop
#else
		b	.fatal
		nop
#endif


		/*
		 * WAIT(ADDR,MASK,VAL)
		 */
		8:	bne	t4,OP_WAIT,8f
		nop
		lw	t1,Init_A1(a0)
		lw	t2,Init_A2(a0)
		and	t4,t3,MOD_MASK

		bne	t4,MOD_B,1f
		nop
		3:	lbu	t4,0(t0)
		and	t4,t1
		bne	t4,t2,3b
		nop
		b	.next
		nop
		1:	bne	t4,MOD_H,1f
		nop
		3:	lhu	t4,0(t0)
		and	t4,t1
		bne	t4,t2,3b
		nop
		b	.next
		nop
		1:	bne	t4,MOD_W,1f
		nop
		3:	lw	t4,0(t0)
		and	t4,t1
		bne	t4,t2,3b
		nop
		b	.next
		nop
		1:
#if __mips64
3:	ld	t4,0(t0)
	and	t4,t1
	bne	t4,t2,3b
	nop
	b	.next
	nop
#else
	b	.fatal
	nop
#endif


	.next:	addu	a0,Init_Size
	b	reginit	
	nop	

	8:
	.fatal:	b .done
	nop
	bal 	stuck
	nop
	.done:	

	/*
	   GPIOLED_SET(5)
	   bal 	superio_init
	   nop

	   GPIOLED_SET(6)
	   bal	initserial
	   nop
	   GPIOLED_SET(7)
	 */

	core0_start:
	PRINTSTR("\r\nPMON2000 MIPS Initializing. Standby...\r\n")
	/*
	   PRINTSTR("ERRORPC=")
	   mfc0	a0, COP_0_ERROR_PC
	   bal	hexserial
	   nop

	   PRINTSTR(" CONFIG=")
	   mfc0	a0, COP_0_CONFIG
	   bal	hexserial
	   nop
	   PRINTSTR("\r\n")

	   PRINTSTR(" PRID=")
	   mfc0	a0, COP_0_PRID
	   bal	hexserial
	   nop
	   PRINTSTR("\r\n")
	 */

	bnez s0,1f
	nop

	li a0,128
	la v0,initmips
	jr v0
	nop
	1:

	/* 
	 * Now determine DRAM configuration and size by
	 * reading the I2C EEROM on the DIMMS
	 */

##############################################

	/* 
	 * now, we just write ddr2 parameters directly. 
	 * we should use i2c for memory auto detecting. 
	 */
gs_2f_v3_ddr2_cfg:

	//Read sys_clk_sel
#if 1
	TTYDBG ("\r\n0xbfe00180  : ")
	li  t2,0xbfe00180
	ld  t1, 0x0(t2)
	dsrl a0, t1, 32
	bal hexserial
	nop
	move    a0, t1
	bal hexserial
	nop
	TTYDBG ("\r\n0xbfe00190  : ")
	li  t2,0xbfe00190
	ld  t1, 0x0(t2)
	dsrl a0, t1, 32
	bal hexserial
	nop
	move    a0, t1
	bal hexserial
	nop
	TTYDBG ("\r\nCPU CLK SEL : ")
	dsrl t1, t1, 32
	andi a0, t1, 0x1f
	bal hexserial
	nop

#if 1 /* TLB init */
	    TTYDBG("Init htpcitlb...\r\n")
#include "pcitlb.S"
#endif

	TTYDBG ("\r\nCPU clk frequency = SYSCLK x 0x")
	andi  t0, t1, 0x1f
	li  a0, 0x1f
	bne t0, a0, 1f
	nop
	TTYDBG ("1\r\n")
	b   2f
	nop
	1:
	andi    t0, t1, 0x1f
	andi    a0, t0, 0xf
	addi    a0, a0, 0x1e
	bal     hexserial
	nop
	TTYDBG (" / ")
	srl     a0, t0, 4
	beqz    a0, 3f
	nop
	TTYDBG (" 2\r\n")
	b       2f
	3:        
	nop
	TTYDBG (" 1\r\n")
	2:      
	TTYDBG ("MEM CLK SEL : ")
	dsrl t0, t1, 5
	andi a0, t0, 0x1f
	bal hexserial
	nop

	TTYDBG ("\r\nDDR clk frequency = MEMCLK x 0x")
	dsrl t0, t1, 5
	andi    t0, t0, 0x1f
	li  a0, 0x1f
	bne t0, a0, 1f
	nop
	TTYDBG ("1\r\n")
	b   2f
	nop
	1:
	dsrl t0, t1, 5
	andi t0, t0, 0x1f
	andi    a0, t0, 0xf
	addi    a0, a0, 0x1e
	bal     hexserial
	nop
	TTYDBG (" / ")
	srl     a0, t0, 4
	beqz    a0, 3f
	nop
	TTYDBG (" 4\r\n")
	b       2f
	nop
	3:
	TTYDBG (" 3\r\n")
	2:      

  
#endif    

#ifdef MULTI_CHIP
	TTYDBG("CPU 1 READ TEST...\r\n")
	dli      t0, 0x900010003ff00040
	lw       a0, 0x0(t0)
	TTYDBG("CPU 1 READ TEST OK.\r\n")
#endif
##########################################

	WatchDog_Enable
#include "loongson3_fixup.S"

//begin :Beep on -> delay -> Beep off
        bal     beep_on
        nop
        li      a0,0x1000
        1:
        addiu   a0,-1
        nop
        bnez    a0,1b
        nop
        bal     beep_off
        nop
//end
	WatchDog_Close

//#define EJTAG_TEST 
#ifdef EJTAG_TEST

        TTYDBG("Wait test by EJTAG\r\n")
1:
		
		PRINTSTR("*")
		nop
		nop
		nop

		mfc0	$0, COP_0_STATUS_REG
		nop
		b 1b
		nop

#endif


##########################################
# Now begin memory detection on board
##########################################

	//##########################################
	//TTYDBG("NODE 0 MEMORY CONFIG BEGIN\r\n")
	//       li      s1, 0x0

##########################################
	/***********************************************************
	  s1: 
	  |[30:28]| MC1_ROW            | MC0_ROW | 15 - MC1_ROW_SIZE |
	  |[27:27]| MC1_EIGHT_BANK     | 1'b0    | FOUR  BANKS       |
	  |       |                    | 1'b1    | EIGHT BANKS       |
	  |[26:24]| MC1_COL            | MC0_COL | 14 - MC1_COL_SIZE |
	  |[22:20]| MC0_ROW            | MC0_ROW | 15 - MC0_ROW_SIZE |
	  |[19:19]| MC0_EIGHT_BANK     | 1'b0    | FOUR  BANKS       |
	  |       |                    | 1'b1    | EIGHT BANKS       |
	  |[18:16]| MC0_COL_SIZE       | MC0_COL | 14 - COL_SIZE     |
	  |[15:12]| MC1_CS_MAP         |         |                   |
	  |[11: 8]| MC0_CS_MAP         |         |                   |
	  |[ 7: 7]| DIMM_TYPE          | 1'b0    | DDR2              |
	  |       |                    | 1'b1    | DDR3              |
	  |[ 6: 4]| SIZE_PER_CONTROLLER| 3'b001  | 512MB             |
	  |       |                    | 3'b010  | 1G                |
	  |       |                    | 3'b011  | 2G                |
	  |[ 3: 2]| CONTROLLER_SELECT  | 2'b00   | USE BOTH          |
	  |       |                    | 2'b01   | MC0_ONLY          |
	  |       |                    | 2'b10   | MC1_ONLY          |
	  |[ 1: 0]| NODE ID            |         |                   |
	 ***********************************************************/
#define MC0DIMM0_USED 0x34 /* 0x3 ---> CS[1:0]=2'b11, 4 ----> MC0 */ 
#define MC0DIMM1_USED 0xc4 /* 0xc ---> CS[3:2]=2'b11, 4 ----> MC0 */ 
#define MC1DIMM0_USED 0x38 /* 0x3 ---> CS[1:0]=2'b11, 8 ----> MC1 */ 
#define MC1DIMM1_USED 0xc8 /* 0xc ---> CS[3:2]=2'b11, 8 ----> MC1 */ 
#define MC0_USED	  0x04 /* 0x4 ----> MC0 */ 
#define MC1_USED	  0x08 /* 0x8 ----> MC1 */ 


#ifdef AUTO_DDR_CONFIG
#define MC0_USED 0x4
#define MC1_USED 0x8

		WatchDog_Enable;

	/* Open the SMBUS controller */
	dli	    a1,0x90000efdfe00a090 ; #b:d:f:r=0:14:0:90 set tmp config address
	li	    a0,SMBUS_IO_BASE_VALUE | 0x1
	sw	    a0, 0x0(a1);

	/* enable the host controller */
	dli	    a1,0x90000efdfe00a0d0 ; #b:d:f:r=0:14:0:d2 bit0=1
	lw	    a0,0x0(a1);
	li	    t1, 0x10000;
	or 	    a0, t1
	sw	    a0, 0x0(a1);

		WatchDog_Close;
/*************************************************************
 * NOTICE HERE: scan dimm of MC0 first, then dimm of MC1     * 
 * below define need to be CHANGED according different board *
*************************************************************/
#define MC0DIMM0  0xa1  /* SA2 = 0, SA1 = 0, SA0 = 0 <--->  MAP_CS 0011 */ //CON36
#define MC0DIMM1  0xa3  /* SA2 = 0, SA1 = 0, SA0 = 1 <--->  MAP_CS 1100 */ //CON37
#define MC1DIMM0  0xa5  /* SA2 = 0, SA1 = 1, SA0 = 0 <--->  MAP_CS 0011 */ //CON38
#define MC1DIMM1  0xa7  /* SA2 = 0, SA1 = 1, SA0 = 1 <--->  MAP_CS 1100 */ //CON39
        PRINTSTR("\r\nProbing DDR SLOT: ");
        PRINTSTR("\r\nProbe NODE0_MC0DIMM0: ");
        li      s1,0x0
        li      v0,0x0
        li      a0,MC0DIMM0;
        li      a1,MC0DIMM0_USED;
        bal     PROBE_DIMM;
        nop;
        move	s4,v0;
        move	a0,v0;
        bal hexserial;
        nop;
	  
        PRINTSTR("\r\nProbe NODE0_MC0DIMM1: ");
        li      s1,0x0
        li      v0,0x0
        li      a0,MC0DIMM1;
        li      a1,MC0DIMM1_USED;
        bal     PROBE_DIMM;
        nop;
		move    s5,v0
        move	a0,v0;
        bal hexserial;
        nop;

		/* check whether DDR in the same MC controler has same BANK/ROW/COL/TYP */
        li    v0,0x0
		move  a0,s4
		move  a1,s5
		li	  a2,MC0_USED 
		bal	MERGE_DIMM
		nop
		move  s6, v0  /* s6 used to save detected infomation from MC0 */
		beqz  v0, probe_node1_mc0
		nop

		/* show value of 64 bit s1 of MC0 */
        PRINTSTR("\r\nNow Merged MC0 on NODE0: ");
        move	a0,s6;
		dsrl	a0,32;
        bal     hexserial;
        nop;
        PRINTSTR("_");
        move	a0,s6;
        bal     hexserial;
        nop;
        PRINTSTR("\r\n");

probe_node1_mc0:

        PRINTSTR("\r\nProbe NODE1_MC0DIMM0: ");
        li      s1,0x0
        li      v0,0x0
        li      a0,MC1DIMM0;
        li      a1,MC0DIMM0_USED;
        bal     PROBE_DIMM;
        nop;
        move	s4,v0;
        move	a0,v0;
        bal hexserial;
        nop;

        PRINTSTR("\r\nProbe NODE1_MC0DIMM1: ");
        li      s1,0x0
        li      v0,0x0
        li      a0,MC1DIMM1;
        li      a1,MC0DIMM1_USED;
        bal     PROBE_DIMM;
        nop;
	move  s5,v0
        move  a0,v0;
        bal hexserial;
        nop;

	 /* now check whether DDR in the same MC controler has same BANK/ROW/COL/TYPE */
        li      v0,0x0
		move  a0,s4
		move  a1,s5
		li	  a2,MC0_USED 
		bal	  MERGE_DIMM
		nop
		move  s7, v0  

		/* show value of 64 bit s1 of MC1 */
        PRINTSTR("\r\nNow Merged MC0 on NODE1: ");
        move	a0,s7;
	dsrl	a0,32;
        bal     hexserial;
        nop;
        PRINTSTR("_");
        move	a0,s7;
        bal     hexserial;
        nop;
        PRINTSTR("\r\n");
#else
        li      s6, 0x001c03a4 /* s6 used to save detected infomation from MC0*/
 	li      s7, 0x001c03a4 /* s7 used to save detected infomation from MC1*/
//        li      s7, 0x0 /* s7 used to save detected infomation from MC1 */
#endif

		/* s6 should not be zeror, otherwize system won't boot successfully */
		bnez s6,get_node0_size
		nop
no_mem:
        PRINTSTR("NO MEMORY FOUND ON MC0/NODE0\r\n");
		bal beep_on
		nop
2:
		bal 2b
		nop

get_node0_size:
	/* calculate memory size */
	move	a0,s6
	li		a1,0x0
	bal		CALCULATE_MEMSZ
	nop
	move	msize,v0

        PRINTSTR("Node0 Total Memory Size:");
	/* show value of 64 bit memsize */
        move	a0,msize;
	dsrl	a0,32;
        bal     hexserial;
        nop;
        PRINTSTR("_");
        move	a0,msize;
        bal     hexserial;
        nop;
        PRINTSTR("\r\n");

	bnez s7,get_node1_size
	nop
	b	out_mem;
	nop

get_node1_size:
	/* calculate memory size */
	move	a0,s7
	li		a1,0x0
	bal		CALCULATE_MEMSZ
	nop
	move	msize_node1,v0

        PRINTSTR("Node1 Total Memory Size: ");
	/* show value of 64 bit memsize */
        move	a0,msize_node1;
	dsrl	a0,32;
        bal     hexserial;
        nop;
        PRINTSTR("_");
        move	a0,msize_node1;
        bal     hexserial;
        nop;
        PRINTSTR("\r\n");

out_mem:

##########################################
# Now begin initialize MC0 and MC1
##########################################

		/* First intitialize MC0 on NODE0 */
		TTYDBG("NODE 0 MEMORY CONFIG BEGIN\r\n")
		move  s1, s6
#include "loongson3_ddr2_config.S"

		TTYDBG("NODE 1 MEMORY CONFIG BEGIN\r\n")
		beqz	s7,mc_config_out
		nop
		ori	s1, s7,0x01;	// to set NODE number
#include "loongson3_ddr2_config.S"
mc_config_out:

//#include "loongson3_showwindows.S"

	/* Used to show memory controler configuration over */
	//GPIO_CLEAR_OUTPUT(0x1<<2)
	GPIO_SET_OUTPUT(0x1<<15)

##########################################
# Now begin TLB clean and initializing  
##########################################

		bal CPU_TLBClear
		nop
		/*
		 *  Reset and initialize caches to a known state.
		 */

		and	t1, t3, 0x10
		addu	s6, t1, 16		/* s6 = D cache line size */
		TTYDBG("Init tlb...\r\n")
		bal     tlb_init
		nop

#if 1 /* TLB init */
	    TTYDBG("Init htpcitlb...\r\n")
#include "pcitlb.S"
#endif
	
		//TTYDBG("Init caches...\r\n")

#if 1
		//mfc0    a0, COP_0_PRID
		//li      a1, 0x6301
		//bne     a0,a1,1f
		//nop
		TTYDBG("godson2 caches found\r\n")
		bal     godson2_cache_init
		nop
		TTYDBG("scache init node 0\r\n")
		//bal	scache_init // smh
		dli a0, 0x9800000000000000
		bal	scache_init_64 // whd
		nop
#if 0
		dli a0, 0x9800000100000000
		bal	scache_init_64 // whd
		nop
		dli a0, 0x9800000200000000
		bal	scache_init_64 // whd
		nop
		dli a0, 0x9800000300000000
		bal	scache_init_64 // whd
		nop
#endif
		nop
#if	1 //def MULTI_CHIP
		TTYDBG("scache init node 1\r\n")
		dli a0, 0x9800100000000000
		bal	scache_init_64 // whd
		nop

#if 0
		bal	scache_init_64 // whd
		dli a0, 0x9800100100000000
		bal	scache_init_64 // whd
		nop
		dli a0, 0x9800100200000000
		bal	scache_init_64 // whd
		nop
		dli a0, 0x9800100300000000
		bal	scache_init_64 // whd
		nop
#endif

		nop
#endif
#endif

#if 0
	bal lock_scache
        nop
	TTYDBG("cache lock done\r\n")
	nop
#endif



#if 1
		PRINTSTR("Jump to 9fc\r\n")
		lui     t0, 0xdfff ####################### go to 9fc
		ori     t0, t0, 0xffff
		bal     1f
		nop
		1:
		and     ra, ra, t0
		addiu   ra, ra, 16
		jr      ra
		nop

#endif

#if 1
		mfc0   $4, $16
		and    $4,0xfffffff8
		or     $4,0x3
		mtc0   $4,$16

		TTYDBG("cache enable done\r\n")
		nop
#endif

#if 0
	TTYDBG("Save system temperature:(* ") // save  temp
    la  a1,temperature
	move  a0,a1
    bal hexserial
    nop  
    TTYDBG(") == ")
    move a0,s2
    bal hexserial
    nop  
    TTYDBG("\r\n")
//  sb  s2, 0x0(a1)

	TTYDBG("Saving node1 memory size:(* ") 
    la  a1,memsz_node1
	move  a0,a1
    bal hexserial
    nop  
    TTYDBG(") == ")
    move a0,msize_node1
    bal hexserial
    nop  
    TTYDBG("\r\n")
    sd  msize_node1, 0x0(a1)
#endif

		WatchDog_Enable;
##########################################

#ifdef LS3_HT
#include "loongson3_HT_init.S"
/***************************************/
#endif
		WatchDog_Close;

#include "3aserver_bridge_config.S"
##########################################

		//#include "loongson3_ddr_debug.S"

##########################################
/***************************************/

##########################################
//cxk
//#include "Test_Mem_N.S"

#include "machine/newtest/mydebug.S"

##########################################

	bootnow:
		TTYDBG("Copy PMON to execute location...\r\n")
#ifdef DEBUG_LOCORE
		TTYDBG("  start = 0x")
		la	a0, start
		bal	hexserial
		nop
		TTYDBG("\r\n  s0 = 0x")
		move	a0, s0
		bal	hexserial
		nop
		TTYDBG("\r\n")
#endif
		la	a0, start
		li	a1, 0xbfc00000
		la	a2, _edata
		#or      a0, 0xa0000000
		#or      a2, 0xa0000000
		subu	t1, a2, a0
		srl	t1, t1, 2

		move	t0, a0
		move	t1, a1
		move	t2, a2

		/* copy text section */

		1:	and	t3,t0,0x0000ffff
		bnez	t3,2f
		nop
#if 0
		move	a0,t0
		bal	hexserial
		nop
		li	a0,'\r'
		bal 	tgt_putchar
		nop
#endif
		2:	lw	t3, 0(t1)
		nop
		sw	t3, 0(t0)
		addu	t0, 4
		addu	t1, 4
		bne	t2, t0, 1b
		nop

	//	PRINTSTR("\ncopy text section done.\r\n")

		/* Clear BSS */
		la	a0, _edata
		la	a2, _end
		2:	sw	zero, 0(a0)
		bne	a2, a0, 2b
		addu	a0, 4


		TTYDBG("Copy PMON to execute location done.\r\n")
		/* zhb */
#if 0
		zhb:
		TTYDBG("Testing...\r\n")
		la	a0, start
		li	a1, 0xbfc00000
		la	a2, _edata
		#or      a0, 0xa0000000
		#or      a2, 0xa0000000
		/*	subu	s6, a2, a0*/
		/*	srl	s6, s6, 2*/

		move	t0, a0
		move	t1, a1
		move	t2, a2
		/* copy text section */

		1:	lw	t4, 0(t1)
		nop
		lw	t5, 0(t0)
		addu	t0, 4
		addu	t1, 4
		beq	t4, t5, 2f
		nop
		move	a0, t0
		subu	a0, 4
		bal	hexserial
		nop
		TTYDBG	(" ")
		move	a0, t4
		bal	hexserial
		nop
		TTYDBG	(" ")
		move	a0, t5
		bal	hexserial
		nop
		TTYDBG	("\r\n")
		2:	bne	t2, t0, 1b
		nop
		TTYDBG	("test ok!\r\n")
		/*
		   3:	beqz	zero, 3b
		   nop
		 */
#endif


		TTYDBG("sp=");
	move a0, sp
		bal	hexserial
		nop

		li	a0, 4096*1024
		sw	a0, CpuTertiaryCacheSize /* Set L3 cache size */

#if 0
		mfc0   a0,COP_0_CONFIG
		and    a0,a0,0xfffffff8
		or     a0,a0,0x3
		mtc0   a0,COP_0_CONFIG
#endif
		PRINTSTR("\r\n")


		/* pass pointer to kseg1 tgt_putchar */
		la  a1, tgt_putchar
		addu a1,a1,s0

		la  a2, stringserial
		addu a2,a2,s0

		srl	msize,20
		move	a0,msize


#ifdef NODE1_BOOT
		dli     t0,NODE1_CORE0_BUF0  #buf of cpu0
#else
		dli     t0,NODE0_CORE0_BUF0  #buf of cpu0
#endif

		li      t1, 0x5a5a
		sw      t1, FN_OFF(t0)
		nop

#if 1

1:
		lw      t1, FN_OFF(t0)
		li		t2, 0x5a5a
		bne     t2, t1,1b
		nop
#endif

# below added to test vector-dma operation

//		jalr  fft_1kp

		la	v0, initmips
		jalr	v0
		nop

		stuck:
#ifdef DEBUG_LOCORE
		TTYDBG("Dumping GT64240 setup.\r\n")
		TTYDBG("offset----data------------------------.\r\n")
		li	s3, 0
		1:
		move	a0, s3
		bal	hexserial
		nop
		TTYDBG(": ")
		2:
		add	a0, s3, bonito
		lw	a0, 0(a0)
		bal	hexserial
		addiu	s3, 4
		TTYDBG(" ")
		li	a0, 0xfff
		and	a0, s3
		beqz	a0, 3f
		li	a0, 0x01f
		and	a0, s3
		bnez	a0, 2b
		TTYDBG("\r\n")
		b	1b
		nop
		3:
		b	3b
		nop

#else
		b	stuck
		nop
#endif
		/*
		 *  Clear the TLB. Normally called from start.S.
		 */
#if __mips64
#define MTC0 dmtc0
#else 
#define MTC0 mtc0
#endif
	       	LEAF(get_mem_clk)

               	li t0,0xbfe00190
               	ld t1,0x0(t0)
               	dsrl t2,t1,37
               	andi t2,t2,0x0000001f
               	move v0,t2

               	nop
               	jr ra
               	nop
       		END(get_mem_clk)

		LEAF(CPU_TLBClear)
		li	a3, 0			# First TLB index.

		li	a2, PG_SIZE_4K
		MTC0   a2, COP_0_TLB_PG_MASK   # Whatever...

		1:
		MTC0   zero, COP_0_TLB_HI	# Clear entry high.
		MTC0   zero, COP_0_TLB_LO0	# Clear entry low0.
		MTC0   zero, COP_0_TLB_LO1	# Clear entry low1.

		mtc0    a3, COP_0_TLB_INDEX	# Set the index.
		addiu	a3, 1
		li	a2, 64
		nop
		nop
		tlbwi				# Write the TLB

		bne	a3, a2, 1b
		nop

		jr	ra
		nop
		END(CPU_TLBClear)

		/*
		 *  Set up the TLB. Normally called from start.S.
		 */
		LEAF(CPU_TLBInit)
		li	a3, 0			# First TLB index.

		li	a2, PG_SIZE_16M
		MTC0   a2, COP_0_TLB_PG_MASK   # All pages are 16Mb.

		1:
		and	a2, a0, PG_SVPN
		MTC0   a2, COP_0_TLB_HI	# Set up entry high.

		move	a2, a0
		srl	a2, a0, PG_SHIFT 
		and	a2, a2, PG_FRAME
		ori	a2, PG_IOPAGE
		MTC0   a2, COP_0_TLB_LO0	# Set up entry low0.
		addu	a2, (0x01000000 >> PG_SHIFT)
		MTC0   a2, COP_0_TLB_LO1	# Set up entry low1.

		mtc0    a3, COP_0_TLB_INDEX	# Set the index.
		addiu	a3, 1
		li	a2, 0x02000000
		subu	a1, a2
		nop
		tlbwi				# Write the TLB

		bgtz	a1, 1b
		addu	a0, a2			# Step address 32Mb.

		jr	ra
		nop
		END(CPU_TLBInit)

		LEAF(stringserial)
		move	a2, ra
#ifdef ROM_EXCEPTION
		li a1,0x3ec00000
		addu	a1, a0, a1
#else
		addu	a1, a0, s0
#endif
		lbu	a0, 0(a1)
		1:
		beqz	a0, 2f
		nop
		bal	tgt_putchar
		addiu	a1, 1
		b	1b
		lbu	a0, 0(a1)

		2:
		j	a2
		nop
		END(stringserial)

		LEAF(outstring)
		move	a2, ra
		move	a1, a0
		lbu	a0, 0(a1)
		1:
		beqz	a0, 2f
		nop
		bal	tgt_putchar
		addiu	a1, 1
		b	1b
		lbu	a0, 0(a1)

		2:
		j	a2
		nop
		END(outstring)

		LEAF(hexserial)
		move	a2, ra
		move	a1, a0
		li	a3, 7
		1:
		rol	a0, a1, 4
		move	a1, a0
		and	a0, 0xf
#ifdef ROM_EXCEPTION
		la	v0, (hexchar+0x3ec00000)
#else
		la	v0, hexchar
		addu	v0, s0
#endif
		addu	v0, a0
		bal	tgt_putchar
		lbu	a0, 0(v0)

		bnez	a3, 1b
		addu	a3, -1

		j	a2
		nop
		END(hexserial)

//#include "fft_1k2g_v1.2.2.S"

		//#define USE_LPC_UART # defined in pmon_cfg
#ifdef USE_LPC_UART
		LEAF(tgt_putchar)
#	la	v0, COM1_BASE_ADDR
		la	v0, COM3_BASE_ADDR
		1:
		lbu	v1, NSREG(NS16550_LSR)(v0)
		and	v1, LSR_TXRDY
#	li	v1, 1
		beqz	v1, 1b
		nop

		sb	a0, NSREG(NS16550_DATA)(v0)
		move	v1, v0
#	la	v0, COM1_BASE_ADDR
		la	v0, COM3_BASE_ADDR
		bne	v0, v1, 1b
		nop

		j	ra
		nop	
		END(tgt_putchar)
#else

		LEAF(tgt_putchar)
		la	v0,GS3_UART_BASE 
		1:
		lbu	v1, NSREG(NS16550_LSR)(v0)
		and	v1, LSR_TXRDY
#	li	v1, 1
		beqz	v1, 1b
		nop

		sb	a0, NSREG(NS16550_DATA)(v0)
		move	v1, v0
		la	v0, GS3_UART_BASE
		bne	v0, v1, 1b
		nop

		j	ra
		nop	
		END(tgt_putchar)
#endif
		LEAF(beep_on)
                nop
                dli     t1,0x90000cfdfe00a080
                lbu     t0,0(t1)
                or      t0,0x04
                sb      t0,0(t1)
                nop
                jr      ra
                nop
                END(beep_on)
                LEAF(beep_off)
                nop
                dli     t1,0x90000cfdfe00a080
                lbu     t0,0(t1)
                and     t0,0xfd
		sb      t0,0(t1)
                nop
                jr      ra
                nop
                END(beep_off)
		/* baud rate definitions, matching include/termios.h */
#define B0      0
#define B50     50      
#define B75     75
#define B110    110
#define B134    134
#define B150    150
#define B200    200
#define B300    300
#define B600    600
#define B1200   1200
#define B1800   1800
#define B2400   2400
#define B4800   4800
#define B9600   9600
#define B19200  19200
#define B38400  38400
#define B57600  57600
#define B115200 115200


		LEAF(initserial_uart)
		li  a0, GS3_UART_BASE

		li	t1,128
#	addiu	a2,a0,3
		sb	t1,3(a0)
		li	t1,0x12      # divider, highest possible baud rate,33M
		#li	t1,0x0e      # divider, highest possible baud rate,25M
		#li	t1,0x1b      # divider, highest possible baud rate, for 50M
		#sb	t1,0(a0)
		li	t1,0x0     # divider, highest possible baud rate
		sb	t1,1(a0)
		li	t1,3
		sb	t1,3(a0)

#srl	t1,t1,0x8
		li	t1,0
		sb	t1,1(a0)
#li	t1,1      # divider, highest possible baud rate


		li	t1,71
		sb	t1,2(a0)
		jr	ra
		nop
		END(initserial_uart)

#ifdef USE_LPC_UART
		LEAF(initserial)
#	la	v0, COM1_BASE_ADDR
		la	v0, COM3_BASE_ADDR
		1:
#set UART FIFO
		li	v1, FIFO_ENABLE|FIFO_RCV_RST|FIFO_XMT_RST|FIFO_TRIGGER_4
		sb	v1, NSREG(NS16550_FIFO)(v0)

#set THR/RDR to BRDL mode
		li	v1, CFCR_DLAB                  #DLAB
		sb	v1, NSREG(NS16550_CFCR)(v0)    

#if 0
		1:
		li      v1, 0x78
		sb	v1, 0x7 (v0)    

#        li      v1,12
#        li      a0,0xbff00080
#        sb      v1,0x0(a0)

		lb	v1, 0x7 (v0)    
		lb	v1, 0x7 (v0)    
#       li      a0,0xbff00080
#        sb      v1,0x0(a0)
		b 1b
#endif

#set Baud rate low byte
		li	v1, NS16550HZ/(16*CONS_BAUD)   #set BRDL
		sb	v1, NSREG(NS16550_DATA)(v0)

#set Baud rate high byte
		srl	v1, 8
		sb	v1, NSREG(NS16550_IER)(v0)     #set BRDH

#set word length to 8bit
		li	v1, CFCR_8BITS                 #8bit
		sb	v1, NSREG(NS16550_CFCR)(v0)

#set DTR and RTS valid
		li	v1, MCR_DTR|MCR_RTS
		sb	v1, NSREG(NS16550_MCR)(v0)

#disable all interrupt
		li	v1, 0x0
		sb	v1, NSREG(NS16550_IER)(v0)

		move	v1, v0
#	la	v0, COM1_BASE_ADDR
		la	v0, COM3_BASE_ADDR
		bne	v0, v1, 1b
		nop

		j	ra
		nop
		END(initserial)
#else
		LEAF(initserial)
		li  a0, GS3_UART_BASE

		li	t1,128
#	addiu	a2,a0,3
		sb	t1,3(a0)
		li	t1,0x12      # divider, highest possible baud rate, for 33M
		#li	t1,0x0e      # divider, highest possible baud rate, for 25M
		#li	t1,0x1b      # divider, highest possible baud rate, for 50M
		sb	t1,0(a0)
		li	t1,0x0     # divider, highest possible baud rate
		sb	t1,1(a0)
		li	t1,3
		sb	t1,3(a0)

#srl	t1,t1,0x8
		li	t1,0
		sb	t1,1(a0)
#li	t1,1      # divider, highest possible baud rate


		li	t1,71
		sb	t1,2(a0)
		jr	ra
		nop
		END(initserial)
#endif

		LEAF(initserial_COM1)
		la	v0, COM1_BASE_ADDR
#la	v0, 0xba0003f8
		1:
#set UART FIFO
		li	v1, FIFO_ENABLE|FIFO_RCV_RST|FIFO_XMT_RST|FIFO_TRIGGER_4
		sb	v1, NSREG(NS16550_FIFO)(v0)

#set THR/RDR to BRDL mode
		li	v1, CFCR_DLAB                  #DLAB
		sb	v1, NSREG(NS16550_CFCR)(v0)    


#set Baud rate low byte
		li	v1, NS16550HZ/(16*CONS_BAUD)   #set BRDL
		//li	v1, 1843200/(16*CONS_BAUD)   #set BRDL
		sb	v1, NSREG(NS16550_DATA)(v0)

#set Baud rate high byte
		srl	v1, 8
		sb	v1, NSREG(NS16550_IER)(v0)     #set BRDH

#set word length to 8bit
		li	v1, CFCR_8BITS                 #8bit
		sb	v1, NSREG(NS16550_CFCR)(v0)

#set DTR and RTS valid
		li	v1, MCR_DTR|MCR_RTS
		sb	v1, NSREG(NS16550_MCR)(v0)

#disable all interrupt
		li	v1, 0x0
		sb	v1, NSREG(NS16550_IER)(v0)

		move	v1, v0
		la	v0, COM1_BASE_ADDR
#la	v0, 0xba0002f8
		bne	v0, v1, 1b
		nop

		j	ra
		nop
		END(initserial_COM1)
		LEAF(stringserial_COM1)
		move	a2, ra
		addu	a1, a0, s0
		lbu	a0, 0(a1)
		1:
		beqz	a0, 2f
		nop
		bal	tgt_putchar_COM1
		addiu	a1, 1
		b	1b
		lbu	a0, 0(a1)

		2:
		j	a2
		nop
		END(stringserial_COM1)
		LEAF(hexserial_COM1)
		move	a2, ra
		move	a1, a0
		li	a3, 7
		1:
		rol	a0, a1, 4
		move	a1, a0
		and	a0, 0xf
		la	v0, hexchar
		addu	v0, s0
		addu	v0, a0
		bal	tgt_putchar_COM1
		lbu	a0, 0(v0)

		bnez	a3, 1b
		addu	a3, -1

		j	a2
		nop
		END(hexserial_COM1)

		LEAF(tgt_putchar_COM1)
		la	v0, COM1_BASE_ADDR
#la	v0, 0xba0002f8
		1:
		lbu	v1, NSREG(NS16550_LSR)(v0)
		and	v1, LSR_TXRDY
#	li	v1, 1
		beqz	v1, 1b
		nop

		sb	a0, NSREG(NS16550_DATA)(v0)
		move	v1, v0
		la	v0, COM1_BASE_ADDR
#la	v0, 0xba0002f8
		bne	v0, v1, 1b
		nop

		j	ra
		nop	
		END(tgt_putchar_COM1)


#include "i2c.S"
		__main:
		j	ra
		nop


		.rdata
		transmit_pat_msg:
		.asciz	"\r\nInvalid transmit pattern.  Must be DDDD or DDxDDx\r\n"
		v200_msg:
		.asciz	"\r\nPANIC! Unexpected TLB refill exception!\r\n"
		v280_msg:
		.asciz	"\r\nPANIC! Unexpected XTLB refill exception!\r\n"
		v380_msg:
		.asciz	"\r\nPANIC! Unexpected General exception!\r\n"
		v400_msg:
		.asciz	"\r\nPANIC! Unexpected Interrupt exception!\r\n"
		hexchar:
		.ascii	"0123456789abcdef"

		.text
		.align	2
		/*
		 *   I2C Functions used in early startup code to get SPD info from
		 *   SDRAM modules. This code must be entirely PIC and RAM independent.
		 */

		/* Delay macro */
#define	DELAY(count)	\
		li v0, count;	\
		99:			\
		bnz	vo, 99b;\
		addiu	v0, -1


#define I2C_INT_ENABLE	0x80
#define I2C_ENABLE	0x40
#define I2C_ACK		0x04
#define I2C_INT_FLAG	0x08
#define I2C_STOP_BIT	0x10
#define I2C_START_BIT	0x20

#define	I2C_AMOD_RD	0x01

#define	BUS_ERROR				0x00
#define	START_CONDITION_TRA			0x08
#define	RSTART_CONDITION_TRA			0x10
#define	ADDR_AND_WRITE_BIT_TRA_ACK_REC		0x18
#define	ADDR_AND_READ_BIT_TRA_ACK_REC		0x40
#define	SLAVE_REC_WRITE_DATA_ACK_TRA		0x28
#define	MAS_REC_READ_DATA_ACK_NOT_TRA		0x58

#define Index_Store_Tag_D			0x09
#define Index_Invalidate_I			0x00
#define Index_Writeback_Inv_D			0x01
#define Index_Store_Tag_S			0x0b
#define Index_Writeback_Inv_S			0x03

		LEAF(nullfunction)
		jr ra
		nop
		END(nullfunction)

#define CP0_ECC  $26
		LEAF(scache_init)
#        daddi   sp, sp, 0xfff8
#        sd      ra, 0(sp)
		move	t7, ra
#if 0 /* gx 2G */
		.word 0x40028001 #mfc0    v0,c0_config1
		and     v0, 0xf 
		beqz    v0, 1f  
		nop     
		jr      ra      
		nop     
		1:
#endif

		lui     a0, 0x8000
		lui     a2, 0x0010      #4M/4way
#lui     a2, 0x0002      #512k/4way
#lui     a2, 0x0004      #1M/4way
		scache_init_4way:
#a0=0x80000000, a2=scache_size
#a3, v0 and v1 used as local registers
		li      t0, 0x22
		mtc0    t0, CP0_ECC
		mtc0    $0, CP0_TAGHI
		mtc0    $0, CP0_TAGLO
		addu    v0, $0, a0
		addu    v1, a0, a2
		1:      slt     a3, v0, v1
		beq     a3, $0, 1f
		nop
		cache   Index_Store_Tag_S, 0x0(v0)
		cache   Index_Store_Tag_S, 0x1(v0)
		cache   Index_Store_Tag_S, 0x2(v0)
		cache   Index_Store_Tag_S, 0x3(v0)
		beq     $0, $0, 1b
		addiu   v0, v0, 0x20
		1:
		/*
scache_flush_4way:
addu    v0, $0, a0
addu    v1, a0, a2
1:      slt     a3, v0, v1
beq     a3, $0, 1f
nop
cache   Index_Writeback_Inv_S, 0x0(v0)
cache   Index_Writeback_Inv_S, 0x1(v0)
cache   Index_Writeback_Inv_S, 0x2(v0)
cache   Index_Writeback_Inv_S, 0x3(v0)
beq     $0, $0, 1b
addiu   v0, v0, 0x20
1:
		 */
scache_init_finish:
#	TTYDBG	("\r\nscache init ok\r\n")

#        ld      ra, 0(sp)
		jr      ra
		nop
		nop
#        daddiu  sp, sp, 8
		scache_init_panic:
		TTYDBG	("\r\nscache init panic\r\n")
		1:      b       1b
		nop
		END(scache_init)
		LEAF(scache_init_64)
		move	t7, ra

#lui     a0, 0x8000
#lui     a2, 0x0010      #4M/4way
		dli     a2, 0x00100000   #4M/4way
#lui     a2, 0x0002      #512k/4way
#lui     a2, 0x0004      #1M/4way
		scache_init_4way_64:
#a0=0x80000000, a2=scache_size
#a3, v0 and v1 used as local registers
		li      t0, 0x22
		mtc0    t0, CP0_ECC
		mtc0    $0, CP0_TAGHI
		mtc0    $0, CP0_TAGLO
		daddu    v0, $0, a0
		daddu    v1, a0, a2
		1:      //dslt     a3, v0, v1
		//beq     a3, $0, 1f
		beq     v0, v1, 1f
		nop
		cache   Index_Store_Tag_S, 0x0(v0)
		cache   Index_Store_Tag_S, 0x1(v0)
		cache   Index_Store_Tag_S, 0x2(v0)
		cache   Index_Store_Tag_S, 0x3(v0)
		beq     $0, $0, 1b
		daddiu   v0, v0, 0x20
		1:

		jr      ra
		nop

		1:      b       1b
		nop
		END(scache_init_64)

		LEAF(tlb_init)
		mtc0    $0, CP0_WIRED
		mtc0    $0, CP0_PAGEMASK
		tlb_flush_all:
		lui     a0, 0x8000
		addiu   a1, $0, 64
#a0=KSEG0,a1 = tlbsize, v0, v1, a3 used as local registers
		mtc0    $0, CP0_ENTRYLO0
		mtc0    $0, CP0_ENTRYLO1
		mfc0    v0, CP0_WIRED
		addu    v1, $0, a0
		1:      sltu    a3, v0, a1
		beq     a3, $0, 1f
		nop
		mtc0    v1, CP0_ENTRYHI
		mtc0    v0, CP0_INDEX
		tlbwi
		addiu   v1, v1, 0x2000
		beq     $0, $0, 1b
		addiu   v0, v0, 1
		1:
###tlb_init finish####
		tlbp
		jr      ra
		nop
		END(tlb_init)
###############################
		LEAF(hexserial64)
		move t7,ra
		move t6,a0
		dsrl a0,32
		bal hexserial
		nop
		move a0,t6
		bal hexserial
		nop
		jr t7
		END(hexserial64)

		LEAF(godson2_cache_init)
####part 2####
		cache_detect_4way:
		mfc0    t4, CP0_CONFIG
		andi    t5, t4, 0x0e00
		srl     t5, t5, 9
		andi    t6, t4, 0x01c0
		srl     t6, t6, 6
		addiu   t6, t6, 10      #4way
		addiu   t5, t5, 10      #4way
		addiu   t4, $0, 1
		sllv    t6, t4, t6
		sllv    t5, t4, t5
		addiu   t7, $0, 4
####part 3####
		lui     a0, 0x8000
#addu    a1, $0, t5
#addu    a2, $0, t6
		li      a1, (1<<14) #64k/4way
		li      a2, (1<<14)
		cache_init_d4way:
#a0=0x80000000, a1=icache_size, a2=dcache_size
#a3, v0 and v1 used as local registers
		mtc0    $0, CP0_TAGHI
		li      t0, 0x22
		mtc0    t0, CP0_ECC
		addu    v0, $0, a0
		addu    v1, a0, a2
		1:      slt     a3, v0, v1
		beq     a3, $0, 1f
		nop
		mtc0    $0, CP0_TAGLO
		cache   Index_Store_Tag_D, 0x0(v0)
		cache   Index_Store_Tag_D, 0x1(v0)
		cache   Index_Store_Tag_D, 0x2(v0)
		cache   Index_Store_Tag_D, 0x3(v0)
		beq     $0, $0, 1b
		addiu   v0, v0, 0x20
		1:
cache_flush_i4way:
		addu    v0, $0, a0
		addu    v1, a0, a1
		mtc0    $0, CP0_TAGLO
		mtc0    $0, CP0_TAGHI
		mtc0    $0, CP0_ECC
		1:      slt     a3, v0, v1
		beq     a3, $0, 1f
		nop
		cache   0x08, 0x0(v0)/*Index_Store_Tag_I*/
		cache   0x08, 0x1(v0)/*Index_Store_Tag_I*/
		cache   0x08, 0x2(v0)/*Index_Store_Tag_I*/
		cache   0x08, 0x3(v0)/*Index_Store_Tag_I*/
		beq     $0, $0, 1b
		addiu   v0, v0, 0x20
		1:
cache_init_finish:
		//TTYDBG	("\r\ncache init ok\r\n")

		jr      ra
		nop
		cache_init_panic:
		TTYDBG	("\r\ncache init panic\r\n")
		1:      b       1b
		nop
		.end	godson2_cache_init

LEAF(lock_scache)
       //addi sp, sp, -64  
       addi sp, sp, -56  
       sd   t0, 0x00(sp)
       sd   t1, 0x08(sp)
       sd   t2, 0x10(sp)
       sd   t3, 0x18(sp)
       sd   t4, 0x20(sp)
       sd   t5, 0x28(sp)
       sd   t6, 0x30(sp)
       move t7, ra

        dli  t0, 0x900000003ff00000
        dli  t1, 0xfffffffffff00000
        sd   t1, 0x240(t0)
        dli  t1, 0x8000000078000000 
        sd   t1, 0x200(t0)
        
        dli  t0, 0x0
        dli  t4, 0x4
 
 1:     dli  t1, 0x0 
        dsll t3, t0, 5
        dli  t5, 0x9800000078000000 
        dli  t6, 0x98000000780fff80 
        or   t5, t5, t3
        or   t6, t6, t3
 
 2:     dli  t2, 0x9800000000000000
        dsll t3, t1, 7
        or   t2, t2, t3
        dsll t3, t0, 5
        or   t2, t2, t3
 
        cache  0x3, 0x3(t2)
        dli    t2, 0x1400
        dsll   t3, t5, 28
        dsrl   t3, t3, 45
        dsll   t3, t3, 13
        or     t2, t2, t3
        mtc0   t2, CP0_TAGLO
 
        dli    t2, 0x0 
        dsll   t3, t5, 16
        dsrl   t3, t3, 52
        or     t2, t2, t3
        mtc0   t2, CP0_TAGHI
 
        cache   0xb, 0x3(t5)

        dli    t2, 0x0
        mtc0   t2, CP0_TAGLO
        dsrl   t2, t2, 32
        mtc0   t2, CP0_TAGHI

        cache   0x1f, 0x3(t5)
        cache   0x1f, 0xb(t5)
        cache   0x1f, 0x13(t5)
        cache   0x1f, 0x1b(t5)
        daddi   t5, t5, 0x80
        daddi   t1, t1, 0x1
        bne     t5, t6, 2b
        nop
        nop
	TTYDBG("lock_scache_cycle1 \r\n")
        daddi   t0, t0, 0x1
        bne     t0, t4, 1b
        nop
	TTYDBG("lock_scache_cycle2 \r\n")
        nop

       ld   t0, 0x00(sp)
       ld   t1, 0x08(sp)
       ld   t2, 0x10(sp)
       ld   t3, 0x18(sp)
       ld   t4, 0x20(sp)
       ld   t5, 0x28(sp)
       ld   t6, 0x30(sp)
       addi sp, sp, 56  
       jr    t7
       // jr    ra
        nop
END(lock_scache)

		//lycheng
		LEAF(nbmisc_read_index_mips)
		dli   t1, HT_CONFIG_ADDR
		or    t1, t1, a0
		sw    a1, NBMISC_INDEX(t1)
		lw    v0, 0x64(t1)
		j  ra
		nop
		END(nbmisc_read_index_mips)
		LEAF(nbmisc_write_index_mips)
		dli   t1, HT_CONFIG_ADDR
		or    t1, t1, a0
		or    t2, a1, 0x80
		sw    t2, 0x60(t1)
		sw    a2, 0x64(t1)
		j  ra
		nop
		END(nbmisc_write_index_mips)
		LEAF(post_code_mips)
		li t0, CPU_POST_PORT
		sb a0, 0x0(t0)
		j ra
		nop
		END(post_code_mips)
		LEAF(enable_rs780_dev8)
		move t6, ra
		li   a0,  0x0
		li   a1,  0x0
		bal    nbmisc_read_index_mips
		nop
		move   v1, v0
		li     t0,  0xffffffbf      // ~(1 << 6)
		and   t1, v1, t0
		li     t0,  0x40  // (1 << 6)
		or    v1, t1, t0
		beq    v1, v0, 1f
		nop
		//or  a1, a1, 0x80
		move a2,  v1
		bal   nbmisc_write_index_mips
		nop
		1:
		j  t6
		nop
		END(enable_rs780_dev8)
		LEAF(pci_read_config32_mips)
		dli t1, HT_CONFIG_ADDR
		or  t2, t1, a0
		or  t1, t2, a1
		lw  v0, 0x0(t1)
		j  ra
		nop
		END(pci_read_config32_mips)
		LEAF(pci_write_config32_mips)
		dli t1, HT_CONFIG_ADDR
		or  t2, t1, a0
		or  t1, t2, a1
		sw  a2, 0x0(t1)
		j  ra
		nop
		END(pci_write_config32_mips)
		LEAF(pci_read_config8_mips)
		dli t1, HT_CONFIG_ADDR
		or  t2, t1, a0
		or  t1, t2, a1
		lb  v0, 0x0(t1)
		j  ra
		nop
		END(pci_read_config8_mips)
		LEAF(pci_write_config8_mips)
		dli t1, HT_CONFIG_ADDR
		or  t2, t1, a0
		or  t1, t2, a1
		sb  a2, 0x0(t1)
		j  ra
		nop
		END(pci_write_config8_mips)


/*********************************************************************/
/* slave core jump to here to begin get ready for entering kernel    */
/*********************************************************************/

		.ent	slave_main
		slave_main:

#if 1
	mfc0	t0, CP0_STATUS
        li      t1, 0x00e0      # {cu3,cu2,cu1,cu0}<={0110, status_fr<=1
        or      t0, t0, t1
	  mtc0    t0, CP0_STATUS
	mfc0    t0, CP0_STATUS

	#lui     t1, 0x10
	#and     t0, t0, t1      #sr 
	lui     t1, 0x40        #bev
	or      t0, t0, t1
	mtc0    t0, CP0_STATUS
	mtc0    zero, CP0_CAUSE
#endif


#ifdef NODE1_BOOT
    dli     t0, NODE1_CORE0_BUF0 #buf of cpu1
#else
    dli     t0, NODE0_CORE0_BUF0 #buf of cpu0
#endif

myidle:
#if 1
    li      a0, 0x1000
1:
    subu   a0, 1
    bnez    a0, 1b
    nop
#endif
    lw      a0, FN_OFF(t0)
    beqz    a0, myidle
    nop


#if 1 /* gx */

		bal CPU_TLBClear
		nop
		bal     tlb_init
		nop
        bal     godson2_cache_init
        nop


#if 1
		mfc0	t0, CP0_CONFIG
		ori	    t0, t0, 7
  		xori	t0, t0, 4
		mtc0	t0, CP0_CONFIG
#endif

#if 1
		la	t0, next
#if 1
	dli	t1, 0xfffff
	and     t0, t1
	dli	t1, 0xffffffff9fc00000
	or	t0, t1
#endif	
	jr	t0
	nop
next:
#endif

#if 1
        ## enable kseg0 cachablilty####
        mfc0    t0, CP0_CONFIG
        lui     t1, 0xffff
        ori     t1, t1, 0xfff8
        and     t0, t0, t1
        ori     t0, t0, 0x3
        mtc0    t0, CP0_CONFIG
        ################################
#endif

#endif

#if 0
############
        mtc0    $0, CP0_WIRED
        mtc0    $0, CP0_PAGEMASK
tlb_flush_all_slave:
        lui     a0, 0x8000
        addiu   a1, $0, 64
#a0=KSEG0,a1 = tlbsize, v0, v1, a3 used as local registers
        mtc0    $0, CP0_ENTRYLO0
        mtc0    $0, CP0_ENTRYLO1
        mfc0    v0, CP0_WIRED
        addu    v1, $0, a0
1:      sltu    a3, v0, a1
        beq     a3, $0, 1f
        nop
        mtc0    v1, CP0_ENTRYHI
        mtc0    v0, CP0_INDEX
        tlbwi
        addiu   v1, v1, 0x2000
        beq     $0, $0, 1b
        addiu   v0, v0, 1
1:
############
#endif

/******************************************************************/
/* Read Mail BOX to judge whether current core can jump to kernel */
/******************************************************************/
/**********************
t0: core ID
t1: core mailbox base address
t2: jump address
t3: temp
************************/
		.set 	mips64
		mfc0	t0, $15, 1
		.set 	mips3

		andi	t0, 0x3ff
		andi    t3, t0,0xc

		beq		t3, 0x0, node00;
		nop	 

		beq		t3, 0x4, node01;
		nop	 

		beq		t3, 0x8, node02;
		nop	 

		beq		t3, 0xc, node03;
		nop	 

node00:
        dli     t1, NODE0_CORE0_BUF0;
		b		core_x
		nop

node01:
        dli     t1, NODE1_CORE0_BUF0;
		b		core_x
		nop

node02:
        dli     t1, NODE2_CORE0_BUF0;
		b		core_x
		nop

node03:
        dli     t1, NODE3_CORE0_BUF0;
		b		core_x
		nop

core_x:
		//add cores offset of every node
		andi	t3, t0, 0x3
		dsll	t3, t3, 8
		or		t1, t1, t3
waitforinit:

	li      a0, 0x1000000
idle1000:    
    addiu   a0, -1
    bnez    a0, idle1000
    nop

    lw      t2, FN_OFF(t1)
    beqz    t2, waitforinit
    nop


    dli      t3, 0xffffffff00000000 
    or       t2, t3

    dli      t3, 0x9800000000000000 
    ld      sp, SP_OFF(t1)
    or      sp, t3
    ld      gp, GP_OFF(t1)
    or      gp, t3
    ld      a1, A1_OFF(t1)

    jalr    t2  # slave core jump to kernel, byebye
    nop

	.end	slave_main

//*********************************************/
//* input :
// * output: v0
// * used:t0, origin value from cp0
// * used:t1, tmp
// * used:t2, tmp
// * function: get smp base address from current core id
// ********************************************/

	.global get_nodebase
	.ent    get_nodebase
	.set    noreorder
	.set    mips3
	
		.set 	mips64
		mfc0	t0, $15, 1
		.set 	mips3
		andi	t0, 0x3ff

		andi	t1,t0,0xc
		andi	t2,t0,0x3

		
	jr      ra

	.end    get_nodebase
/*********************************************/

/*********************************************************************/
	#define loongson3A3
//#include "loongson3a_ddr3_leveling.S"
#######################################
	.global ddr2_config
	.ent    ddr2_config
	.set    noreorder
	.set    mips3


ddr2_config:
#ifdef loongson3A3
#define MODIFY_PARAM_MC0 180
#else
#define MODIFY_PARAM_MC0 152
#endif

	GET_DDR_TYPE
	la      a2, ddr2_reg_data
	beqz    a1, 1f
	nop
	la      a2, ddr3_reg_data
1:

	addu    a2, a2, s0
	daddiu	v0, a0, 0x0
######################################################
	ld      a1, 0x0(a2)
	sd      a1, 0x0(v0)

#####Eight_bank_mode #################################
	ld      t1, 0x8(a2)
	dli     a1, 0xffffff00ffffffff
	and     t1, t1, a1
	GET_MC0_EIGHT
	or      t1, t1, a1
	sd      t1, 0x10(v0)

######################################################
	ld      a1, 0x10(a2)
	sd      a1, 0x20(v0)
	ld      a1, 0x18(a2)
	sd      a1, 0x30(v0)
	ld      a1, 0x20(a2)
	sd      a1, 0x40(v0)

#####COL_SIZE / ROW_SIZE #############################
	ld      t1, 0x28(a2)
	dli     a1, 0xffffffff00ff00ff
	and     t1, t1, a1
	GET_MC0_ROW
	or      t1, t1, a1
	GET_MC0_COL
	or      t1, t1, a1
	sd      t1, 0x50(v0)

######################################################
	ld      a1, 0x30(a2)
	sd      a1, 0x60(v0)

#####CS_MAP ##########################################
	ld      t1, 0x38(a2)
	dli     a1, 0xffffffffff00ffff
	and     t1, t1, a1
	GET_MC0_CS_MAP
	or      t1, t1, a1
	sd      t1, 0x70(v0)

######################################################

	li      t1, MODIFY_PARAM_MC0-8
	daddiu	v0, a0, 0x80

reg_write:
	ld      a1, 0x40(a2)
	sd      a1, 0x0(v0)

	subu    t1, t1, 0x1
	addiu   a2, a2, 0x8
	bne     t1, $0, reg_write
	daddiu   v0, v0, 0x10

#if	0 //def MY_DEBUG_DDR   //debug
    /* store the a0,ra */
	move	t8, a0
	move	t1, ra

    PRINTSTR("\r\nChange some parameters of MC0:");
1:
	PRINTSTR("\r\nPlease input the register number you want to change!!!(0:jump out.): ");
	bal	    inputaddress	#input value stored in v0
	nop
	move	t5, v0	#store v0 to t5
	
	beqz	t5, 2f	#if input 0,jump out
	nop
	dsll	t5, t5, 4	#t5 is the offset relative to a0
	daddu	t5, t5, t8

	PRINTSTR("\r\nPlease input the data-hex: ");
	bal	    inputaddress	#input value stored in v0
	nop
	sd	    v0, 0x0(t5)	#v0 is the input value

    //print the new register value
    move    t6, t5
    PRINTSTR("\r\nRegister 0x")
    dsubu   t5, t5, t8
    dsrl    t5, t5, 4
    move    a0, t5
    bal     hexserial
    nop
    PRINTSTR(": ")
    ld      t6, 0x0(t6)
    dsrl    a0, t6, 32
    bal     hexserial
    nop
    move    a0, t6
    bal     hexserial
    nop

	b	    1b
	nop
2:	
    /* recover the a0,ra */
	move	a0, t8
	move	ra, t1
#endif
	sync

############start##########
	daddiu	v0, a0, 0x0
	ld      a1, 0x30(v0)
	dli     a2, 0x0000010000000000
	or      a1, a1, a2
	sd      a1, 0x30(v0)
	sync
###########################
  //delay
	dli     v0, 0x1000
1:
	bnez    v0, 1b
	daddi   v0, v0, -1
	nop

	daddiu  v0, a0, 0x960 //wait int status 
1:
	ld      a1, 0x0(v0)
	andi    a1, a1, 0x100
	beqz    a1, 1b
	nop

	jr      ra
	nop
	.end    ddr2_config


	.rdata
	.align 5
	.global ddr2_reg_data
	.global ddr3_reg_data

#include "loongson3B_ddr2_param.S"
#include "ldp.S"
//#include "loongson3a_ddr3_param.S"

		.text
		.global temperature
		.global memsz_node1
	temperature:		.dword	  2
	memsz_node1:		.dword	  0


	.global watchdog_enable
	.ent    watchdog_enable
	.set    noreorder
	.set    mips3
watchdog_enable:
	WatchDog_Enable
	jr		ra
	.end watchdog_enable
