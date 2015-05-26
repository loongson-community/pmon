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
#include "target/ls2h.h"

#include "loongson3_def.h"
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


	.text	
	.set	noreorder
	.globl	_start
	.globl	start
_start:
start:
	.globl	stack
stack = start - 0x4000		/* Place PMON stack below PMON start in RAM */
	.set	mips64

	mfc0	t0, $16, 6
	or	t0, 0x100 
	xori	t0, 0x100
	mtc0	t0, $16, 6

	mfc0	t0, $22
	//lui	t1, 0x8000
	lui	t1, 0x0000
	or	t0, t1, t0
	mtc0	t0, $22
	.set	mips0

	/* missing test for status */
	lui     t1, 0x40    //set bev to 1'b1
	ori	t1, 0x00e0  //set {UX,SX,KX} to 3'b111
	mtc0    t1, CP0_STATUS
	mtc0    zero, CP0_CAUSE

	la	sp, stack
	la	gp, _gp

	/* WatchDog chip MAX6369 disable work */
	WatchDog_Close

	bal	locate			/* Get current execute address */
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

	b	exc_common
	nop

	/* Debug exception */
	.align  7           /* bfc00480 */
	#include "exc_ejtag.S"

exc_common:
	.set 	mips64
	mfc0	t0, $15, 1
	.set 	mips3
	PRINTSTR("\r\nCPU ID=")
	move	a0, t0
	bal	hexserial
	nop
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
	b	1b
	nop

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
	la	s0, start
	subu	s0, ra, s0
	and	s0, 0xffff0000

	li	bonito,PHYS_TO_UNCACHED(BONITO_REG_BASE)


	.set 	mips64
	mfc0	t0, $15, 1
	.set 	mips3
	andi	t0, 0x3ff

	li	t3, RESERVED_COREMASK
	andi	t3, 0xf
	li	t1, 0x1
	sllv	t1, t1, t0
	and	t3, t1, t3
	bnez	t3, wait_to_be_killed
	nop
	li	t2, BOOTCORE_ID
	bne	t0, t2, 1f
	nop
	lui	v0, 0xbfe0
	addiu	v0, 0x01d0
	lw	t2, 0x0(v0)
	xori	t2, SHUTDOWN_MASK
	sw	t2, 0x0(v0)
	
	b	1f
	nop

wait_to_be_killed:

	b	wait_to_be_killed
	nop
1:

	dli     a0, BOOTCORE_ID
	bne     t0, a0, slave_main
	nop

	bal	initserial
	nop
	// temp workarount
	bal	initserial_uart1
	nop
	bal	clear_mailbox
	nop
//	speed up spi flash read
	dli     a0,0x900000001ff00080
	li      t0,0x99
	sb      t0,0x0(a0)

core0_start:
	PRINTSTR("\r\nPMON2000 MIPS Initializing. Standby...\r\n")
	bnez	s0, 1f
	nop

	li	a0, 128
	la	v0, initmips
	jr	v0
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
	TTYDBG ("\r\n0xbfe00190  : ")
	li	t2,0xbfe00190
	ld	t1, 0x0(t2)
	dsrl	a0, t1, 32
	bal	hexserial
	nop
	move    a0, t1
	bal	hexserial
	nop
	TTYDBG ("\r\nCPU CLK SEL : ")
	dsrl	t1, t1, 32
	andi	a0, t1, 0x1f
	bal	hexserial
	nop


	TTYDBG ("\r\nCPU clk frequency = SYSCLK x 0x")
	andi	t0, t1, 0x1f
	li	a0, 0x1f
	bne	t0, a0, 1f
	nop
	TTYDBG ("1\r\n")
	b	2f
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
	dsrl	t0, t1, 5
	andi	a0, t0, 0x1f
	bal	hexserial
	nop

	TTYDBG ("\r\nDDR clk frequency = MEMCLK x 0x")
	dsrl	t0, t1, 5
	andi    t0, t0, 0x1f
	li 	a0, 0x1f
	bne	t0, a0, 1f
	nop
	TTYDBG ("1\r\n")
	b	2f
	nop
1:
	dsrl	t0, t1, 5
	andi	t0, t0, 0x1f
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

#define SOFT_CLKSEL
#ifdef SOFT_CLKSEL

//#define DDR_LOOPC    64 //264MHz
//#define DDR_LOOPC    72 //300MHz
//#define DDR_LOOPC    80 //330MHz
//#define DDR_LOOPC  48 //400MHz
//#define DDR_LOOPC  28 //466MHz
//#define DDR_LOOPC  60 //500MHz
//#define DDR_LOOPC  32 //533MHz
//#define DDR_LOOPC  34 //566MHz
#define DDR_LOOPC  72 //600MHz
//#define DDR_LOOPC  38 //633MHz
#define DDR_REFC   1
//#define DDR_DIV    8
#define DDR_DIV    4

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

#define MEM_CLKSEL (0x01f << 5)
#define MEM_HSEL (0x0f << 5)
#define PLL_MEM_ENA		(0x1 << 1)
#define PLL_MEM_LOCKED (01 << 16)

#define HT_HSEL	(0x1 << 15)

	TTYDBG ("Soft CLK SEL adjust begin\r\n")

	li      t0, 0xbfe00194
	lw      a0, 0x0(t0)
	li      a1, CORE_CLKSEL
	and     a0, a0, a1
	li	a1, CORE_HSEL
	bne	a0, a1, soft_mem
	nop

soft_sys:
	TTYDBG ("CORE & NODE:")

	li      t0, 0xbfe001b0
	dli	t1, (L1_LOOPC << 32) | (L1_DIV << 42) | (0x3 << 10) | (0x1 << 7)
	sd	t1, 0(t0)
	ori	t1, PLL_L1_ENA
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
	li      t0, 0xbfe00194
	lw      a0, 0x0(t0)
	li      a1, MEM_CLKSEL
	and     a0, a0, a1
	li	a1, MEM_HSEL
	bne	a0, a1, soft_ht
	nop

	TTYDBG ("\r\nMEM        :")

	li      t0, 0xbfe001c0
	dli     a0, (DDR_DIV << 24) | (DDR_LOOPC << 14) | (0x3 << 4) | (0x1 << 3) | PLL_MEM_ENA
	sw	a0, 0x0(t0)
wait_locked_ddr:
	lw      a0, 0x0(t0)
	li      a1, 0x00000040
	and     a0, a0, a1
	beqz    a0, wait_locked_ddr
	nop

	lw      a0, 0x0(t0)
	ori     a0, a0, 0x1
	sw      a0, 0x0(t0)

	bal     hexserial
	nop

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

#include "loongson3_fixup.S"

        bal     tgt_testchar
        nop  
        bnez    v0, after_ht
        nop  


#include "loongson3_HT_init_2h.S" 
after_ht:
	bal	beep_on
	nop
	li	a0, 0x800
1:
	addiu	a0, a0, -1
	nop
	bnez	a0, 1b
	nop
	bal	beep_off
	nop

	TTYDBG("Init htpcitlb...\r\n")
#include "pcitlb.S"

	mfc0	t0, CP0_CONFIG
	or	t0, 0x7
	xori	t0, 0x3
	mtc0	t0, CP0_CONFIG

	PRINTSTR("Jump to 9fc\r\n")
	lui     t0, 0xdfff
	ori     t0, t0, 0xffff
	bal     1f
	nop
1:
	and     ra, ra, t0
	addiu   ra, ra, 16
	jr      ra
	nop

	TTYDBG("s0 = 0x")
        move    a0, s0
        bal     hexserial64
        nop
	TTYDBG("\r\n")

//##########################################
//DDR config start
//cxk
####################################
#include "ddr_dir/lsmc_ddr_param_define.h"
#include "ddr_dir/ddr_config_define.h"
#define DDR_DLL_BYPASS
#define DISABLE_DIMM_ECC
#define PRINT_MSG
#ifndef ARB_LEVEL
//#define FIX_DDR_PARAM
#endif
#ifdef  ARB_LEVEL
#define AUTO_ARB_LEVEL
#endif
#ifdef  AUTO_ARB_LEVEL
#define CHECK_ARB_LEVEL_FREQ
#ifdef  AUTO_DDR_CONFIG
#define CHECK_ARB_LEVEL_DIMM
#endif
//#define DEBUG_AUTO_ARB_LEVEL
#endif
//#define DEBUG_DDR
//#define DEBUG_DDR_PARAM

	TTYDBG("\r\nStart Init Memory, wait a while......\r\n")
####################################
	move    msize, $0
	move    s3, $0
	//!!!!important--s1 must be correctly set

	TTYDBG("NODE 0 MEMORY CONFIG BEGIN\r\n")

#ifdef  AUTO_DDR_CONFIG
        dli     s1, 0x32170000  //set use MC1 or MC0 or MC1/0 and give All device id
#else
	dli     s1, 0xc3e30200c3e30204
#endif
#include "ddr_dir/loongson3a2h_ddr_config.S"

	TTYDBG("Init Memory done.\r\n")

/*judge the node0 whether have memory*/
	and     a0, msize, 0xff
	//beqz    a0, beep_on
	nop

/* test memory */
        li      t0, 0xa0000000
        dli     a0, 0x5555555555555555
        sd      a0, 0x0(t0)
        dli     a0, 0xaaaaaaaaaaaaaaaa
        sd      a0, 0x8(t0)
        dli     a0, 0x3333333333333333
        sd      a0, 0x10(t0)
        dli     a0, 0xcccccccccccccccc
        sd      a0, 0x18(t0)
        dli     a0, 0x7777777777777777
        sd      a0, 0x20(t0)
        dli     a0, 0x8888888888888888
        sd      a0, 0x28(t0)
        dli     a0, 0x1111111111111111
        sd      a0, 0x30(t0)
        dli     a0, 0xeeeeeeeeeeeeeeee
        sd      a0, 0x38(t0)


	PRINTSTR("The uncache data is:\r\n")
	dli     t1, 8
	dli     t5, 0x9000000000000000
1:
	ld      t6, 0x0(t5)
	move    a0, t5
	and     a0, a0, 0xfff
	bal     hexserial
	nop
	PRINTSTR(":  ")
	dsrl    a0, t6, 32
	bal     hexserial
	nop
	move    a0, t6
	bal     hexserial
	nop
	PRINTSTR("\r\n")

	daddiu  t1, t1, -1
	daddiu  t5, t5, 8
	bnez    t1, 1b
	nop

	PRINTSTR("The cached  data is:\r\n")
	dli     t1, 8
	dli     t5, 0x9800000000000000
1:
	ld      t6, 0x0(t5)
	move    a0, t5
	and     a0, a0, 0xfff
	bal     hexserial
	nop
	PRINTSTR(":  ")
	dsrl    a0, t6, 32
	bal     hexserial
	nop
	move    a0, t6
	bal     hexserial
	nop
	PRINTSTR("\r\n")

	daddiu  t1, t1, -1
	daddiu  t5, t5, 8
	bnez    t1, 1b
	nop

##########################################
#ifdef  DEBUG_DDR
#if 1
	PRINTSTR("\r\nDo test?(0xf: skip): ")
	bal     inputaddress
	nop
	and     v0, v0, 0xf
	dli     a1, 0x1
	bgt     v0, a1, 3f
	nop
#endif

	dli     s1, 0x0010000100000000
#if 1
	PRINTSTR("\r\ndefault s1 = 0x");
	dsrl    a0, s1, 32
	bal     hexserial
	nop
	PRINTSTR("__")
	move    a0, s1
	bal     hexserial
	nop
	PRINTSTR("\r\nChange test param s1(0: skip)?: ")
	bal     inputaddress
	nop
	beqz    v0, 1f
	nop
	move    s1, v0
1:
#endif
1:
	dli     t1, 0x0010
	bal     test_mem
	nop
	move    t1, v0
	PRINTSTR("\r\n")
	dsrl    a0, t1, 32
	bal     hexserial
	nop
	move    a0, t1
	bal     hexserial
	nop
	beqz    t1, 2f
	nop
	PRINTSTR("  Error found!!\r\n")
2:
#if 0
	b       1b
	nop
#endif

3:
#endif

#ifdef  AUTO_ARB_LEVEL
#include "ddr_dir/store_auto_arb_level_info.S"
#endif


#if 0 //cww_X2

	PRINTSTR("\r\n======X1 core0 map windows:\r\n")
	li      t1, 23
	dli     t2, 0x900000003ff02000
1:
	move	a0, t2
	bal	hexserial64
	nop
	PRINTSTR(": ")

	ld      a0, 0x0(t2)
	bal	hexserial64
	nop
	PRINTSTR("\r\n")

	daddiu  t2, t2, 8
	bnez    t1, 1b
	addiu   t1, t1, -1

	PRINTSTR("\r\n======X2 cpu map windows:\r\n")
	li      t1, 23
	dli     t2, 0x900000003ff00000
1:
	move	a0, t2
	bal	hexserial64
	nop
	PRINTSTR(": ")

	ld      a0, 0x0(t2)
	bal	hexserial64
	nop
	PRINTSTR("\r\n")

	daddiu  t2, t2, 8
	bnez    t1, 1b
	addiu   t1, t1, -1

	PRINTSTR("\r\n======X2 pci map windows:\r\n")
	li      t1, 23
	dli     t2, 0x900000003ff00100
1:
	move	a0, t2
	bal	hexserial64
	nop
	PRINTSTR(": ")

	ld      a0, 0x0(t2)
	bal	hexserial64
	nop
	PRINTSTR("\r\n")

	daddiu  t2, t2, 8
	bnez    t1, 1b
	addiu   t1, t1, -1
#endif

#if 0
	PRINTSTR("\r\n======read HT config reg:\r\n")
	dli     t2, 0x90000efdfb000000

	move	a0, t2
	bal	hexserial64
	nop
	PRINTSTR(": ")

	ld      a0, 0x0(t2)
	bal	hexserial64
	nop
	PRINTSTR("\r\n")

	daddiu	a0, t2, 0x60
	bal	hexserial64
	nop
	PRINTSTR(": ")

	ld      a0, 0x60(t2)
	bal	hexserial64
	nop
	PRINTSTR("\r\n")

	daddiu	a0, t2, 0x68
	bal	hexserial64
	nop
	PRINTSTR(": ")

	ld      a0, 0x68(t2)
	bal	hexserial64
	nop
	PRINTSTR("\r\n")

	daddiu	a0, t2, 0x70
	bal	hexserial64
	nop
	PRINTSTR(": ")

	ld      a0, 0x70(t2)
	bal	hexserial64
	nop
	PRINTSTR("\r\n")
#endif


##########################################

#include "machine/newtest/newdebug.S"

##########################################

bootnow:
	bal	spd_info_store
	nop
		
	TTYDBG("Copy PMON to execute location...\r\n")
	la	a0, start
	li	a1, 0x9fc00000
	la	a2, _edata
	/* copy text section */

1:	
	lw	a3, 0(a1)
	sw	a3, 0(a0)
	daddiu	a0, a0, 4
	bne	a2, a0, 1b
	daddiu	a1, a1, 4

	PRINTSTR("copy text section done.\r\n")

	/* Clear BSS */
	la	a0, _edata
	la	a2, _end
2:	
	sw	zero, 0(a0)
	daddiu	a0, a0, 4
	bne	a2, a0, 2b
	nop

	TTYDBG("Copy PMON to execute location done.\r\n")

	li	a0, 4096*1024
	sw	a0, CpuTertiaryCacheSize /* Set L3 cache size */

	/* pass pointer to kseg1 tgt_putchar */
	la	a1, tgt_putchar
	addu	a1, a1, s0

	la	a2, stringserial
	addu	a2, a2, s0

	move	a0, msize
	dli     t0, NODE0_CORE0_BUF0  #buf of cpu0 we need bootcore_id
	dli	t3, BOOTCORE_ID
	dsll    t3, t3, 8
	or      t0, t0, t3
	li      t1, SYSTEM_INIT_OK
	sw      t1, FN_OFF(t0)
	la	v0, initmips
	jalr	v0
	nop
stuck:
	b	stuck
	nop

LEAF(clear_mailbox)
	.set mips64
	mfc0	t0, $15, 1
	.set mips3
	andi	t0, t0, 0x3ff
	andi	t1, t0, 0x3
	dsll	t1, 8
	andi	t2, t0, 0xc
	dsll	t2, 42
	or	t1, t2, t1
	dsrl	t2, 30              /* for 3b/3c */
	or	t1, t2, t1
	dli	t2, NODE0_CORE0_BUF0
	or	t1, t1, t2
	sd	zero, FN_OFF(t1)
	sd	zero, SP_OFF(t1)
	sd	zero, GP_OFF(t1)

	jr	ra
	sd	zero, A1_OFF(t1)
END(clear_mailbox)

LEAF(get_mem_clk)

	li	t0, 0xbfe00190
	ld	t1, 0x0(t0)
	dsrl	t2, t1, 37
	andi	t2, t2, 0x0000001f
	jr ra
	move	v0, t2
END(get_mem_clk)

		
LEAF(spd_info_store)
	 move    t8,ra

	 TTYDBG("\r\n spd_info_store begain.\r\n")

	 dli    t5, 0xffffffff8fffa000;

	 dli    t7, 0xa1;
	 dli    t6, 0xa9; //slot1 slot2 slot3       

4:
	 move    a0, t7
	 bne     a0, 0xa1, 5f
	 nop 
	 dli     a0, 0xaf; //slot0
5:       
	dli     a1, 0x2;
 //GET_I2C_NODE_ID_a2
	 bal     i2cread;
	 nop;

	 dli     t3, 0x80
	 bltu    v0, t3, 2f
	 nop;
	 move    t3, t5;
	 daddiu  t3, 0x100;
	 move    t4, t5;
1:
	 sb      zero,0(t4);
	 daddiu  t4, 0x1;
	 bltu    t4, t3, 1b
	 nop;

	 b       3f
	 nop;

2:
	move    t4, t5;
	dli     t0, 0x0; //used as counter
1:
	move    a0, t7;
	bne     a0, 0xa1, 6f
	nop
	dli     a0, 0xaf
6:      move    a1, t0;
//GET_I2C_NODE_ID_a2
	 bal     i2cread;
	 nop;

	sb      v0, 0(t4);

	dli     a1, 0x100
	daddiu  t4, 0x1;
	daddiu  t0, 0x1;
	bne     t0, a1, 1b;
	nop
3:
	daddiu  t5, 0x100;
	daddiu  t7, 0x2;

	bltu    t7, t6, 4b
	nop

	TTYDBG("\r\n spd_info_store done.\r\n")

	move	ra, t8
	jr	ra
	nop
END(spd_info_store)

LEAF(stringserial)
	move	a2, ra
	addu	a1, a0, s0
	lbu	a0, 0(a1)
1:
	beqz	a0, 2f
	nop
	bal	tgt_putchar
	addiu	a1, 1
	b	1b
	lbu	a0, 0(a1)

2:
	move	ra, a2
	j	ra
	nop
END(stringserial)

LEAF(hexserial)
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
	bal	tgt_putchar
	lbu	a0, 0(v0)

	bnez	a3, 1b
	addu	a3, -1

	move	ra, a2
	j	ra
	nop
END(hexserial)

LEAF(tgt_putchar)
	la	v0, GS3_UART_BASE 
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
	jr	ra
	nop	
END(tgt_putchar)
LEAF(beep_on)

	/* enable gpio0 output */
	//li	t1,0xbbd000c4
	dli	t1,0x90000e001fd000c4
	lh	t0,0(t1)
	and	t0,0xfffe
	sh	t0,0(t1)
	/* set gpio0 high */
	//li	t1,0xbbd000cc
	dli	t1,0x90000e001fd000cc
	lh	t0,0(t1)
	or	t0,0x0001
	sh	t0,0(t1)

	nop
	jr	ra
	nop
END(beep_on)

LEAF(beep_off)

	/* enable gpio0 output */
	//li	t1,0xbbd000c4
	dli	t1,0x90000e001fd000c4
	lh	t0,0(t1)
	and	t0,0xfffe
	sh	t0,0(t1)
	/* set gpio0 low */
	//li	t1,0xbbd000cc
	dli	t1,0x90000e001fd000cc
	lh	t0,0(t1)
	and	t0,0xfffe
	sh	t0,0(t1)

	jr	ra
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


LEAF(initserial)
	li	a0, GS3_UART_BASE

	li	t1, 128
	sb	t1, 3(a0)
#ifdef	BONITO_33M 
	li	t1, 0x12      # divider, highest possible baud rate,for 33M crystal
#endif
#ifdef BONITO_25M 
	li	t1, 0x0e      # divider, highest possible baud rate,for 25M crystal
#endif
#ifdef BONITO_50M 
	li	t1, 0x1b      # divider, highest possible baud rate,for 50M crystal
#endif
	sb	t1, 0(a0)
	li	t1, 0x0     # divider, highest possible baud rate
	sb	t1, 1(a0)
	li	t1, 3
	sb	t1, 3(a0)
	li	t1, 0
	sb	t1, 1(a0)
	li	t1, 71
	sb	t1, 2(a0)
	jr	ra
	nop
END(initserial)


LEAF(initserial_uart1)
	li 	a0, GS3_UART1_BASE

	li	t1, 128
	sb	t1, 3(a0)
#ifdef	BONITO_33M 
	li	t1, 0x12      # divider, highest possible baud rate,for 33M crystal
#endif
#ifdef BONITO_25M 
	li	t1, 0x0e      # divider, highest possible baud rate,for 25M crystal
#endif
#ifdef BONITO_50M 
	li	t1, 0x1b      # divider, highest possible baud rate,for 50M crystal
#endif
	sb	t1, 0(a0)
	li	t1, 0x0     # divider, highest possible baud rate
	sb	t1, 1(a0)
	li	t1, 3
	sb	t1, 3(a0)
	li	t1, 0
	sb	t1, 1(a0)
	li	t1, 71
	sb	t1, 2(a0)
	jr	ra
	nop
END(initserial_uart1)



#include "i2c.S"
#ifdef AUTO_DDR_CONFIG  // for no ls2h cpu on board, disable AUTO_DDR_CONFIG
#include "ddr_dir/detect_node_dimm_all.S"
#endif

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

LEAF(nullfunction)
	jr ra
	nop
END(nullfunction)

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
	move ra,t7
	jr ra
END(hexserial64)

	.ent    slave_main
slave_main:
	dli     t2, NODE0_CORE0_BUF0
	dli	t3, BOOTCORE_ID
	dsll    t3, 8
	or      t2, t2, t3

wait_scache_allover:
	lw	t4, FN_OFF(t2)
	/* since bsp be paused, then resumed after mem initialised
	 * we need to SYSTEM_INIT_OK instead of L2_CACHE_DONE
	 */
	dli	t5, SYSTEM_INIT_OK
	bne	t4, t5, wait_scache_allover
	nop
	/**********************************************/

	## enable kseg0 cachablilty####
	mfc0	t6, CP0_CONFIG
	ori	t6, t6, 7
	xori	t6, t6, 4
	mtc0	t6, CP0_CONFIG


	#jump to cached kseg0 address
	lui     t6, 0xdfff 
	ori     t6, t6, 0xffff
	bal     1f
	nop
1:
	and     ra, ra, t6
	daddiu	ra, ra, 16
	jr      ra
	nop

/******************************************************************/
/* Read Mail BOX to judge whether current core can jump to kernel 
 * the cpu spin till FN_OFF is NOT zero
 
/******************************************************************/
	/**********************
	 * t0: core ID
	 * t1: core mailbox base address
	 * t2: jump address
	 * t3: temp
	 ************************/

	bal	clear_mailbox
	nop
waitforinit:

	li      a0, 0x1000
idle1000:    
	addiu   a0, -1
	bnez    a0, idle1000
	nop

	lw      t2, FN_OFF(t1)
	beqz    t2, waitforinit
	nop

	dli     t3, 0xffffffff00000000 
	or      t2, t3

	dli     t3, 0x9800000000000000 
	ld      sp, SP_OFF(t1)
	or      sp, t3
	ld      gp, GP_OFF(t1)
	or      gp, t3
	ld      a1, A1_OFF(t1)

	move	ra, t2
	jr	ra  # slave core jump to kernel, byebye
	nop

	.end    slave_main

    .global watchdog_enable
    .ent    watchdog_enable
    .set    noreorder
    .set    mips3
watchdog_enable:
    WatchDog_Enable
    jr		ra
    nop
    .end watchdog_enable
#######################################
#include "ddr_dir/lsmc_ddr_config.S"
#ifdef DDR3_DIMM
#include "ddr_dir/loongson3C_ddr3_leveling.S"
#endif
#ifdef ARB_LEVEL
#include "ddr_dir/ARB_level_new.S"
#endif
#ifdef  DEBUG_DDR
#include "ddr_dir/Test_Mem.S"
#endif

	.rdata
	.global ddr2_reg_data
	.global ddr3_reg_data

	.align  5
#include "loongson_mc2_param.S"
	.text
	.global  nvram_offs
	.align 12
nvram_offs:
	.dword 0x0
	.align 12
