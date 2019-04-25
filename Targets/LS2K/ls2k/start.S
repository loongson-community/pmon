/*	$Id: start.S,v 1.3 2004/05/17 10:39:22 wlin Exp $ */

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
#include "target/bonito.h"
#include "target/ls2k.h"

#include "target/cacheops.h"	//mtf

#define TTYDEBUG
#ifdef TTYDEBUG
#define	TTYDBG(x) \
	.rdata;98: .asciz x; .text; la a0, 98b; bal stringserial; nop
#else
#define TTYDBG(x)
#endif
#define HAVE_TARGET_GETCHAR
#define	PRINTSTR(x) \
	.rdata;98: .asciz x; .text; la a0, 98b; bal stringserial; nop

#define CONFIG_CACHE_64K_4WAY 1 

#define tmpsize		s1
#define msize		s2
#define sdShape		s3
#define bonito		s4
#define dbg		s5
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

#ifdef ACPI_S3_MODE

#define S3_REG_SAVE_AREA        0xffffffffa00f0000
#define GEN_RTC_1_REG        	0xffffffffbfef0050
#define PM1_CNT                 0xffffffffbfef0014
#define INTIEN0                 0xbfd00044
#define INTIEN1                 0xbfd0005c
#define INTIEN2                 0xbfd00074
#define INTIEN3                 0xbfd0008c
#define INTIEN4                 0xbfd000a4

#endif




/*
 *   Register usage:
 *
 *	s0	link versus load offset, used to relocate absolute adresses.
 *	s1	free
 *	s2	memory size.
 *	s3	sdShape.
 *	s4	Bonito base address.
 *	s5	dbg.
 *	s6	sdCfg.
 *	s7	rasave.
 *	s8	L3 Cache size.
 */


#ifdef BOOT_FROM_NAND

#define Index_Store_Tag_D			0x09
#define Index_Store_Tag_S			0x0B 
#define Index_Store_Tag_I                       0x08        


#define CONFREG_BASE 0xbfd00000
#define APB_BASE 0xbfe80000
#define NAND_BASE 0x60000
#define NAND_DMA_OFFSET 0x40
#define REMAP_REG 0xbfd80088
//#define REMAP_REG 0x1fd80088
#define REMAP_DDR 0x00e000f0 //0x80e00000 
#define REMAP_DDR_DMA 0x00e00000 //0x80e00000 

#endif

	.set	noreorder
	.globl	_start
	.globl	start
	.globl	__main
_start:
start:
	.globl	stack
stack = start - 0x4000		/* Place PMON stack below PMON start in RAM */

#if 1 //fix the hardware poweroff error.

	bal	ls2k_version
	nop
	bnez	v0,2f
	nop

	.set    mips32
	mfc0    t0, $15, 1      #EBASE
	.set    mips3
	andi    t0, t0, 0x3ff
	bnez    t0, 2f
	nop

	lui	t0, 0xba00
	lui	t1, 0x1fe0
	sw	t1, 0x1010(t0) /* config bar for APB */
	lw	t2, 0x1004(t0)
	ori	t2, t2, 0x2
	sw	t2, 0x1004(t0)

	li t0,0xbfe0700c
	lw t1,0x0(t0)
	and t2,t1,(1 << 11)
	beqz  t2,2f
	nop

	li t0,0xbfe0700c
	lw t1, 0x0(t0)
	sw t1,0x0(t0)
	li t2,0x3c00
	li t0,0xbfe07014
	sw t2,0x0(t0)
2:
#endif
	bal watchdog_close
	nop

	/* init processor state at first*/
/* NOTE!! Not more that 16 instructions here!!! Right now it's FULL! */
	mtc0	zero, COP_0_STATUS_REG //cuckoo
	mtc0	zero, COP_0_CAUSE_REG
	li	t0, SR_BOOT_EXC_VEC	/* Exception to Boostrap Location */
	mtc0	t0, COP_0_STATUS_REG //cuckoo

        bal     initregs
        nop

	.set	mips32
	mfc0	t0, $16, 6		#Store fill
	.set	mips3
	li	t1, 0xfffffeff
	and	t0, t1, t0
	.set	mips32
	mtc0	t0, $16, 6		#Store fill
	.set	mips3

	/* spi speedup */
	li  t0, 0xbfff0220
	li  t1, 0x47
	sb  t1, 0x4(t0)

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
	move	k0, ra		#save ra
	la	a0, v200_msg
	bal	stringserial
	nop
	b	exc_common

	.align	7			/* bfc00280 */
	move	k0, ra	#save ra
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
	move	k0, ra		#save ra
	la	a0, v380_msg
	bal	stringserial
	nop
	b	exc_common
	
	.align	8			/* bfc00400 */
	move	k0, ra		#save ra
	la	a0, v400_msg
	bal	stringserial
	nop

#if 1
	b	exc_common
	nop

	/* Debug exception */
	.align  7           /* bfc00480 */
	#include "exc_ejtag.S"
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
    	b	1b
    	nop
	PRINTSTR("\r\nDERR0=")
	cfc0	a0, COP_0_DERR_0
	bal	hexserial
	nop
	PRINTSTR("\r\nDERR1=")
	cfc0	a0, COP_0_DERR_1
	bal	hexserial
	nop

//	b	ext_map_and_reboot
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

#define SUPPORT_PWRUP_FROM_SELFREF
#ifdef  SUPPORT_PWRUP_FROM_SELFREF
#define GET_PWRUP_FROM_SREFRESH \
	move    a1, $0;
#endif

initregs:
        ## init registers
        move    $1, $0
        move    $2, $0
        move    $3, $0
        move    $4, $0
        move    $5, $0
        move    $6, $0
        move    $7, $0
        move    $8, $0
        move    $9, $0
        move    $10, $0
        move    $11, $0
        move    $12, $0
        move    $13, $0
        move    $14, $0
        move    $15, $0
        move    $16, $0
        move    $17, $0
        move    $18, $0
        move    $19, $0
        move    $20, $0
        move    $21, $0
        move    $22, $0
        move    $23, $0
        move    $24, $0
        move    $25, $0
        move    $26, $0
        move    $27, $0
        move    $28, $0
        move    $29, $0
        move    $30, $0

	/* now begin initial float CP1 register */
        ## float point

        mfc0    t0, CP0_STATUS
        li	t1, 0x64000000|SR_KX|SR_SX|SR_UX|SR_BOOT_EXC_VEC
        or      t0, t0, t1
        mtc0    t0, CP0_STATUS
	la	sp, stack
	la	gp, _gp
	jr	ra
	nop

/*
 *  We get here from executing a bal to get the PC value of the current execute
 *  location into ra. Check to see if we run from ROM or if this is ramloaded.
 */
locate:

	la	s0, start
	subu	s0, ra, s0
	and	s0, 0xffff0000

	mfc0    t0, CP0_STATUS
	li	t1, 0x64000000|SR_KX|SR_SX|SR_UX|SR_BOOT_EXC_VEC      # {cu3,cu2,cu1,cu0}<={0110, status_fr<=1,0xe0 to enable 64bit space
	or	t0, t0, t1
	mtc0	t0, CP0_STATUS
	mtc0	zero, COP_0_CAUSE_REG

cp0_main:
        .set    mips32
        mfc0    t0, $15, 1      #EBASE
        .set    mips3
        andi    t0, t0, 0x3ff
        bnez    t0, wait_for_smp
        nop

	bal	beep_on
	nop

	li	a0, 0x8000
1:
	addiu	a0, -1
	nop
	bnez	a0, 1b
	nop
	bal	beep_off//mtf
	nop

	lui	t0, 0xba00
	lui	t1, 0x1fe0
	sw	t1, 0x1010(t0) /* config bar for APB */
	lw	t2, 0x1004(t0)
	ori	t2, t2, 0x2
	sw	t2, 0x1004(t0)

#if 1	//xwr pcie signal test
        li      t0, 0xbfe10000

	li	t1, 0xc2492331
	sw	t1, 0x580(t0)
	sw	t1, 0x5a0(t0)

	li	t1, 0xff3ff0a8
	sw	t1, 0x584(t0)
	sw	t1, 0x5a4(t0)

	li	t1, 0x27fff
	sw	t1, 0x588(t0)
	sw	t1, 0x5a8(t0)

#endif
/* mtf add for cfg pcie */
	
        li      t0, 0xbfe10590
        dli     t1, 0x14fff1002
        sd      t1, 0x0(t0)
        sd      t1, 0x20(t0)

        dli     t1, 0x14fff1102
        sd      t1, 0x0(t0)
        sd      t1, 0x20(t0)
	
        dli     t1, 0x14fff1202
        sd      t1, 0x0(t0)
        sd      t1, 0x20(t0)
	
        dli     t1, 0x14fff1302
        sd      t1, 0x0(t0)
        sd      t1, 0x20(t0)
	
	li	t0, 0xbfe10430
	lw	t1, 0x0(t0)
	or	t1, t1, 0x30000	//pcie enable
	sw	t1, 0x0(t0)

//pcie1 port0
	dli	t0, 0x900000fe0800680c
	li	t1, 0xfff9ffff
        lw      t2, 0x0(t0)
        and     t1, t1, t2
	or	t1, 0x20000
	sw	t1, 0x0(t0)

	dli	t0, 0x900000fe0700681c
    lw  t2, 0x0(t0)
    li  t1, (0x1 << 26)
    or  t2, t1
    sw  t2, 0x0(t0)


	dli	t0, 0x900000fe00006800
	li	t1, 0x10000000
	sw	t1, 0x10(t0)

	dli	t0, 0x9000000000000000
	li      t1, 0x10000000
	or	t0, t0, t1

    li  t1, (0x7<<18)|(0x7<<2)
    not t1, t1
    lw  t2, 0x54(t0)
    and t2, t2, t1
    sw  t2, 0x54(t0)

    lw  t2, 0x58(t0)
    and t2, t2, t1
    sw  t2, 0x58(t0)

	dli	t1, 0xff204f
        sw      t1, 0x0(t0)
  
//pcie1 port1
	dli	t0, 0x900000fe0800700c
	li	t1, 0xfff9ffff
        lw      t2, 0x0(t0)
        and     t1, t1, t2
	or	t1, 0x20000
	sw	t1, 0x0(t0)

	dli	t0, 0x900000fe0700701c
    lw  t2, 0x0(t0)
    li  t1, (0x1 << 26)
    or  t2, t1
    sw  t2, 0x0(t0)

	dli	t0, 0x900000fe00007000
	li	t1, 0x10100000
	sw	t1, 0x10(t0)

	dli	t0, 0x9000000000000000
	li      t1, 0x10100000
	or	t0, t0, t1

    li  t1, (0x7<<18)|(0x7<<2)
    not t1, t1
    lw  t2, 0x54(t0)
    and t2, t2, t1
    sw  t2, 0x54(t0)

    lw  t2, 0x58(t0)
    and t2, t2, t1
    sw  t2, 0x58(t0)

	dli	t1, 0xff204c
        sw      t1, 0x0(t0)
  
//pcie0 port0
	dli	t0, 0x900000fe0800480c	//other pcie controller
	li	t1, 0xfff9ffff
        lw      t2, 0x0(t0)
        and     t1, t1, t2
	or	t1, 0x20000
	sw	t1, 0x0(t0)

	dli	t0, 0x900000fe0700481c
    lw  t2, 0x0(t0)
    li  t1, (0x1 << 26)
    or  t2, t1
    sw  t2, 0x0(t0)

	dli	t0, 0x900000fe00004800
	li	t1, 0x11000000
	sw	t1, 0x10(t0)

	dli	t0, 0x9000000000000000
	li      t1, 0x11000000
	or	t0, t0, t1

    li  t1, (0x7<<18)|(0x7<<2)
    not t1, t1
    lw  t2, 0x54(t0)
    and t2, t2, t1
    sw  t2, 0x54(t0)

    lw  t2, 0x58(t0)
    and t2, t2, t1
    sw  t2, 0x58(t0)

	dli	t1, 0xff204c
        sw      t1, 0x0(t0)
  
//pcie0 port1
	dli	t0, 0x900000fe0800500c	//other pcie controller
	li	t1, 0xfff9ffff
        lw      t2, 0x0(t0)
        and     t1, t1, t2
	or	t1, 0x20000
	sw	t1, 0x0(t0)

	dli	t0, 0x900000fe0700501c
    lw  t2, 0x0(t0)
    li  t1, (0x1 << 26)
    or  t2, t1
    sw  t2, 0x0(t0)

	dli	t0, 0x900000fe00005000
	li	t1, 0x11100000
	sw	t1, 0x10(t0)

	dli	t0, 0x9000000000000000
	li      t1, 0x11100000
	or	t0, t0, t1

    li  t1, (0x7<<18)|(0x7<<2)
    not t1, t1
    lw  t2, 0x54(t0)
    and t2, t2, t1
    sw  t2, 0x54(t0)

    lw  t2, 0x58(t0)
    and t2, t2, t1
    sw  t2, 0x58(t0)

	dli	t1, 0xff204c
        sw      t1, 0x0(t0)
  
//pcie0 port2
	dli	t0, 0x900000fe0800580c	//other pcie controller
	li	t1, 0xfff9ffff
        lw      t2, 0x0(t0)
        and     t1, t1, t2
	or	t1, 0x20000
	sw	t1, 0x0(t0)

	dli	t0, 0x900000fe00005800
	li	t1, 0x11200000
	sw	t1, 0x10(t0)

	dli	t0, 0x9000000000000000
	li      t1, 0x11200000
	or	t0, t0, t1

	dli	t1, 0xff204c
        sw      t1, 0x0(t0)
  
//pcie0 port3
	dli	t0, 0x900000fe0800600c	//other pcie controller
	li	t1, 0xfff9ffff
        lw      t2, 0x0(t0)
        and     t1, t1, t2
	or	t1, 0x20000
	sw	t1, 0x0(t0)

	dli	t0, 0x900000fe00006000
	li	t1, 0x11300000
	sw	t1, 0x10(t0)

	dli	t0, 0x9000000000000000
	li      t1, 0x11300000
	or	t0, t0, t1

	dli	t1, 0xff204c
        sw      t1, 0x0(t0)

	bal	initserial
	nop

	PRINTSTR("\r\ninitserial good ^_^...\r\n")
	nop

	bal	ls2k_version
	nop
	bnez	v0,2f
	nop
	//gmac1 phy reset
	li t1,0xbfe10500
	li t2,(1 << 2) //34 - 2
	not t2,t2
	lw t3,0x4(t1)
	and t2,t3
	sw t2,0x4(t1)

	li t2,(1 << 2)//34 -2
	lw t3,0x14(t1)
	not t2,t2
	and t2,t3 
	sw t2,0x14(t1)
2:

#include "loongson3_clksetting.S"

	bal 	initserial_later
	nop

	bal	ls2k_version
	nop
	bnez	v0,2f
	nop
	//gmac1 phy reset
	li t1,0xbfe10500
	li t2,(1 << 2) //34 - 2
	lw t3,0x14(t1)
	or t2,t3 
	sw t2,0x14(t1)
2:

#if 1 /* Config SATA : use internel clock */
	li	    t0, 0xbfe10000

    li      t1, 0x30c31cf9
    sw      t1, 0x454(t0)
    li      t1, 0xf300040f
    sw      t1, 0x450(t0)

#if 1
	PRINTSTR("\r\nUSE internel SATA ref clock\r\n")
	li	t1, 0xbfe10450
	ld	a0, 0x0(t1)
	li	a1, 0x2
	not	a1, a1
	and	a0, a0, a1
	sd	a0, 0x0(t1)
	sync

	ld	a0, 0x0(t1)
	li	a1, 0x4
    or  a0, a1
	sd	a0, 0x0(t1)
	sync

	ld	a0, 0x0(t1)
	li	a1, 0x8
    or  a0, a1
	sd	a0, 0x0(t1)
	sync

	ld	a0, 0x0(t1)
	bal	hexserial64
	nop
#endif

#if 0
    li      t0, 0xbfe10000

    //assert p0 reset
    lw      t1, 0x450(t0)
    li      t2, (1 << 3)
    not     t2, t2
    and     t1, t1, t2
    sw      t1, 0x450(t0)

    //assert phy reset
    lw      t1, 0x450(t0)
    li      t2, (1 << 2)
    not     t2, t2
    and     t1, t1, t2
    sw      t1, 0x450(t0)

    //power down phy
    lw      t1, 0x454(t0)
    li      t2, (1 << 31)
    or      t1, t1, t2
    sw      t1, 0x454(t0)

    //switch refclk
    lw      t1, 0x450(t0)
    li      t2, (1 << 1)
    not     t2, t2
    and     t1, t1, t2
    sw      t1, 0x450(t0)

    //delay a while
    li      t1, 0x1000
1:
    subu    t1, t1, 1
    bnez    t1, 1b
    nop


    //power up phy
    lw      t1, 0x454(t0)
    li      t2, (1 << 31)
    not     t2, t2
    and     t1, t1, t2
    sw      t1, 0x454(t0)

    //deassert phy reset
    lw      t1, 0x450(t0)
    li      t2, (1 << 2)
    or      t1, t1, t2
    sw      t1, 0x450(t0)
	
    //deassert p0 reset
    lw      t1, 0x450(t0)
    li      t2, (1 << 3)
    or      t1, t1, t2
    sw      t1, 0x450(t0)

    //delay a while
    li      t1, 0x10000
1:
    subu    t1, t1, 1
    bnez    t1, 1b
    nop
#endif
#endif

#if 0 /* Config SATA TX signal*/
        li      t0, 0xbfe10458
        dli     t1, 0x1403f1002
        sd      t1, 0x0(t0)
#endif


#if 1 // Fix the Gmac0  multi-func to enable Gmac1
	li	t0, 0xbfe13800
	dli	a0, 0xffffff0000ffffff
	sd	a0, 0x08(t0)

	li	t0, 0xba001800
	li	a0, 0x0080ff08
	sw	a0, 0x0c(t0)
#endif

#if 1 // Set the invalid BAR to read only
	li	t0, 0xbfe13800
	dli	a0, 0xff00ff0000fffff0
	sd	a0, 0x00(t0)
	sd	a0, 0x08(t0)
	sd	a0, 0x10(t0)
	sd	a0, 0x18(t0)
	sd	a0, 0x20(t0)
	sd	a0, 0x28(t0)
	sd	a0, 0x30(t0)
	sd	a0, 0x38(t0)
	sd	a0, 0x40(t0)
	sd	a0, 0x48(t0)
	sd	a0, 0x50(t0)
#endif

start_now:

	PRINTSTR("\r\nPMON2000 MIPS Initializing. Standby...\r\n")

	bnez	s0, 1f
	nop
	li	a0, 128
	la	v0, initmips
	jr	v0
	nop

1:



        mfc0	a0, COP_0_CONFIG		/* enable kseg0 cachability */
        ori     a0, a0, 0x3           // ENABLE
        mtc0   a0, COP_0_CONFIG


#include "pcitlb.S" /* map 0x4000000-0x7fffffff to 0xc0000000 */

/* jmp to 0x9fc... */
        lui     t0, 0xdfff ####################### go to 9fc
        ori     t0, t0, 0xffff
        bal     1f
        nop     
1:
	and     ra, ra, t0
	addiu   ra, ra, 16
	jr      ra      
	nop 
	PRINTSTR("cache enable done\r\n")

	li	t0, 0xbfe10430
	lw 	a2, 0x0(t0)
	// pcie0 and pcie1
	lui	t1, 0x3
	// enable dvo0 and dvo1 pin output 
    	ori     t1, t1, 0x12
	or	a2, a2, t1
   	sw      a2, 0x0(t0)
	

	li	t0, 0xbfe10420
	//sdio
	lw	t2, 0x0(t0)
	lui	t1, 0x10
	//enable pwm0, pwm1, i2c0, i2c1, nand, sata, i2s, gmac1
	//no hda, no ac97
	ori	t1, t1, 0x3f48
	or	t2, t2, t1
	sw	t2, 0x0(t0)
	

//##########################################
//DDR config start
//cxk
#include "ddr_dir/lsmc_ddr_param_define.h"
#include "ddr_dir/ddr_config_define.h"
//#define DDR_DLL_BYPASS
#define DISABLE_DIMM_ECC
#define PRINT_MSG
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
//#define  DISABLE_DDR_A15
//#define DEBUG_DDR
//#define DEBUG_DDR_PARAM
//#define PRINT_DDR_LEVELING
//#define DLL_DELAY_LOOP
//#define NO_AUTO_TRFC   //adjust TRFC param manually if defined
    TTYDBG("\r\nStart Init Memory, wait a while......\r\n")
####################################
    move    msize, $0
    move    s3, $0
//!!!!important--s1 must be correctly set

    TTYDBG("NODE 0 MEMORY CONFIG BEGIN\r\n")
#ifdef  AUTO_DDR_CONFIG
    dli     s1, 0xff100004  //set use MC1 or MC0 or MC1/0 and give All device id
#else
	//dli     s1, 0xc2e30400c2e30404
	dli     s1, 0xc1a10404
#endif
#include "ddr_dir/loongson3_ddr2_config.S"

	/*judge the node0 whether have memory*/
	and     a0, msize, 0xff
//	beqz    a0, beep_on
//	nop


//close default internal mapping in ddr controller
	li      t0, 0xbfe10424
	lb      a0, 0x1(t0)
	and     a0, a0, 0xfd
	sb      a0, 0x1(t0)
	sync

	li      t0, 0xbfe10420
	ld      a0, 0x0(t0)
	move    t6, a0
	dsrl    a0, t6, 32
	bal     hexserial
	nop
	move    a0, t6
	bal     hexserial
	nop
	PRINTSTR("\r\n")

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
	PRINTSTR("\r\nDo test?(0xf: skip): ")
	bal     inputaddress
	nop
	and     v0, v0, 0xf
	dli     a1, 0x1
	bgt     v0, a1, 2f
	nop
	dli     s1, 0x0010000080000000
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
#endif

#ifdef  AUTO_ARB_LEVEL
#include "ddr_dir/store_auto_arb_level_info.S"
#endif

#include "machine/newtest/newdebug.S"

bootnow:

	TTYDBG("  start = 0x")
	la	a0, start
	bal	hexserial
	nop
	TTYDBG("\r\n  s0 = 0x")
	move	a0, s0
	bal	hexserial
	nop
	TTYDBG("\r\n  _edata = 0x");
	la	a0, _edata
	bal	hexserial
	nop
	TTYDBG("\r\n  _end = 0x");
	la	a0, _end
	bal	hexserial
	nop
#if 0 //ls2h old code
	TTYDBG("\r\n")
        bal  spd_info_store
	nop
#endif
	la	a0, start
	li	a1, 0xbfc00000
	la	a2, _edata

	move	t0, a0
	move	t1, a1
	move	t2, a2

	/* copy text section */
	
1:	and	t3, t0, 0x0000ffff
	bnez	t3, 2f
	nop
	move	a0, t0
	bal	hexserial
	nop
	li	a0, '\r'
	bal 	tgt_putchar
	nop

2:	lw	t3, 0(t1)
	sw	t3, 0(t0)

	addu	t0, 4
	addu	t1, 4

	blt	    t0, t2, 1b
	nop
	PRINTSTR("\ncopy text section done.\r\n")

	.set noreorder
	/* Clear BSS */
	la	a0, _edata
	la	a2, _end
2:	sw	zero, 0(a0)
	bne	a2, a0, 2b
	addu	a0, 4
	TTYDBG("\nClear BSS done.\r\n")

	PRINTSTR("Copy PMON to execute location done.\r\n")

/* cxk */
/*******************/

	move	a0, msize

	la	v0, initmips
	jalr	v0  
	nop

#if __mips64
#define MTC0 dmtc0
#else 
#define MTC0 mtc0
#endif

.global wait_for_smp;
.global wait_for_smp_call;
wait_for_smp:
	mfc0	t1, CP0_CONFIG
	ori	t1, t1, 0x3
	mtc0	t1, CP0_CONFIG
	la	t1, 1f
	addu	t1, s0
	li	v0, 0x9fffffff
	and	t1, v0
	jr	t1
	nop
1:
	li	t0, 0xbfe11120
	sd	zero, 0(t0)
	sd	t1, 8(t0)

wait_for_smp_call:
1:
	ld	t1, 0(t0)
	beqz	t1, 1b
	ld	sp, 8(t0)
	sd	zero,0(t0)
	ld	gp, 16(t0)

	jr	t1
	nop


LEAF(CPU_TLBClear)
	li	a3, 0			# First TLB index.

	li	a2, PG_SIZE_4K
	MTC0	a2, COP_0_TLB_PG_MASK   # Whatever...

1:
	MTC0	zero, COP_0_TLB_HI	# Clear entry high.
	MTC0	zero, COP_0_TLB_LO0	# Clear entry low0.
	MTC0	zero, COP_0_TLB_LO1	# Clear entry low1.

	mtc0	a3, COP_0_TLB_INDEX	# Set the index.
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
	MTC0	a2, COP_0_TLB_PG_MASK   # All pages are 16Mb.

1:
	and	a2, a0, PG_SVPN
	MTC0	a2, COP_0_TLB_HI	# Set up entry high.

	move	a2, a0
	srl	a2, a0, PG_SHIFT 
	and	a2, a2, PG_FRAME
	ori	a2, PG_IOPAGE
	MTC0	a2, COP_0_TLB_LO0	# Set up entry low0.
	addu	a2, (0x01000000 >> PG_SHIFT)
	MTC0	a2, COP_0_TLB_LO1	# Set up entry low1.

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

/*
 * Simple character printing routine used before full initialization
 */

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
	la	v0, hexchar
	addu	v0, s0
	addu	v0, a0
#ifdef BOOT_FROM_NAND
#ifdef NAND_ECC_MODE
	addiu   v0, v0,0x400 
#endif
#endif
	bal	tgt_putchar
	lbu	a0, 0(v0)

	bnez	a3, 1b
	addu	a3, -1

	j	a2
	nop
END(hexserial)


LEAF(ls2k_version)
	.set    mips32
	mfc0    a0, COP_0_PRID
	.set    mips3
	andi    a0, a0, 0xf
	li	a1, 0x3
	bne     a0, a1, 2f
	li	v0, 0
	li	v0, 0x1
2:
	j	ra
	nop
END(outstring)
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

#define Index_Store_Tag_D			0x09
#define Index_Invalidate_I			0x00
#define Index_Writeback_Inv_D			0x01
#define Index_Writeback_Inv_S			0x01//delete scache  
#define Index_Store_Tag_S			0x0B 

LEAF(nullfunction)
	jr ra
	nop
END(nullfunction)

#if 1   //mtf add
LEAF(pcache_init)
	.set mips32
	mfc0    v0, $16, 1      #Read Config1
	.set mips3
	li      t0, 1
	srl     t1, v0, 16   
	andi    t1, t1, 7       #I-Cache way - 1
	srl     t2, v0, 19
	andi    t2, t2, 7
	addiu   t2, t2, 1
	sllv    t2, t0, t2      #I-Cache Line Size
	srl     t3, v0, 22
	andi    t3, t3, 7
	addiu   t3, t3, 6
	sllv    t3, t0, t3      #I-Cache sets per way
	addiu   t3, t3, -1

	la      a0, 0x80000000
	mtc0    $0, CP0_TAGLO
	mtc0    $0, CP0_TAGHI
	mtc0    $0, CP0_ECC
icache_loop_set:
	move    t4, t1
icache_loop_way:
	or	a1, a0, t4
	cache   Index_Store_Tag_I, 0x0(a1)
	/* flush icache data bank */
	cache   Index_Store_Data_I, 0x00(a1)
	cache   Index_Store_Data_I, 0x08(a1)
	cache   Index_Store_Data_I, 0x10(a1)
	cache   Index_Store_Data_I, 0x18(a1)
	cache   Index_Store_Data_I, 0x20(a1)
	cache   Index_Store_Data_I, 0x28(a1)
	cache   Index_Store_Data_I, 0x30(a1)
	cache   Index_Store_Data_I, 0x38(a1)
	bnez    t4, icache_loop_way
	addiu   t4, t4, -1
	addu    a0, a0, t2
	bnez    t3, icache_loop_set
	addiu   t3, t3, -1

	.set mips32
	mfc0    v0, $16, 1      #Read Config1
	.set mips3
	li      t0, 1
	srl     t1, v0, 7   
	andi    t1, t1, 7       #D-Cache way - 1
	srl     t2, v0, 10
	andi    t2, t2, 7
	addiu   t2, t2, 1
	sllv    t2, t0, t2      #D-Cache Line Size
	srl     t3, v0, 13
	andi    t3, t3, 7
	addiu   t3, t3, 6
	sllv    t3, t0, t3      #D-Cache sets per way
	addiu   t3, t3, -1

	la      a0, 0x80000000
	li      t4, 0x22
	mtc0    $0, CP0_TAGLO
	mtc0    $0, CP0_TAGHI
	mtc0    t4, CP0_ECC
dcache_loop_set:
	move    t4, t1
dcache_loop_way:
	or      a1, a0, t4
	cache   Index_Store_Tag_D, 0x0(a1)
	/* flush dcache data bank */
	cache   Index_Store_Data_D, 0x00(a1)
	cache   Index_Store_Data_D, 0x08(a1)
	cache   Index_Store_Data_D, 0x10(a1)
	cache   Index_Store_Data_D, 0x18(a1)
	cache   Index_Store_Data_D, 0x20(a1)
	cache   Index_Store_Data_D, 0x28(a1)
	cache   Index_Store_Data_D, 0x30(a1)
	cache   Index_Store_Data_D, 0x38(a1)
	bnez    t4, dcache_loop_way
	addiu   t4, t4, -1
	addu    a0, a0, t2
	bnez    t3, dcache_loop_set
	addiu   t3, t3, -1

	jr      ra
	nop
END(pcache_init)
#endif

#define CP0_ECC  $26

LEAF(scache_init)

	.set mips32
	mfc0    v0, $16, 2      #Read Config2
	.set mips3
	li      t0, 1
	andi    t1, v0, 0xf     #S-Cache way - 1
	srl     t2, v0, 4
	andi    t2, t2, 0xf
	addiu   t2, t2, 1
	sllv    t2, t0, t2      #S-Cache Line Size
	srl     t3, v0, 8
	andi    t3, t3, 0xf
	addiu   t3, t3, 6
	sllv    t3, t0, t3      #S-Cache sets per way
	addiu   t3, t3, -1

	la      a0, 0x80000000
	li      t4, 0x22
	mtc0    $0, CP0_TAGLO
	mtc0    $0, CP0_TAGHI
	mtc0    t4, CP0_ECC
scache_loop_set:
	move    t4, t1
scache_loop_way:
	or      a1, a0, t4
	cache   Index_Store_Tag_S, 0x0(a1)
	/* flush scache data bank */
	cache   Index_Store_Data_S, 0x00(a1)
	cache   Index_Store_Data_S, 0x10(a1)
	cache   Index_Store_Data_S, 0x20(a1)
	cache   Index_Store_Data_S, 0x30(a1)
	bnez    t4, scache_loop_way
	addiu   t4, t4, -1
	addu    a0, a0, t2
	bnez    t3, scache_loop_set
	addiu   t3, t3, -1

	jr      ra
	nop
END(scache_init)

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
END(tlb_init)
###############################
LEAF(hexserial64)
	move	t7, ra
	move	t6, a0
	dsrl	a0, 32
	bal	hexserial
	nop
	move	a0, t6
	bal	hexserial
	nop
	jr	t7
	nop
END(hexserial64)

LEAF(smh_flush_dcache)
	li	a0, 0x80000000
	li	a1, (1<<14)
        addu    v0, $0, a0
        addu    v1, a0, a1
1:      slt     a3, v0, v1
        beq     a3, $0, 1f
        nop
        cache   0x01, 0x0(v0)   // Index_Writeback_Inv_D
        cache   0x01, 0x1(v0)
        cache   0x01, 0x2(v0)
        cache   0x01, 0x3(v0)
        beq     $0, $0, 1b
        addiu   v0, v0, 0x20
	jr	ra
	nop
END(smh_flush_dcache)

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

LEAF(get_userenv)
	jr	ra
	nop
END(get_userenv)
LEAF(initserial)
	.set noat
	move 	AT,ra

#if 1   //mtf add
	li	a0, COM1_BASE_ADDR
	li	t1, CFCR_DLAB
	sb	t1, NSREG(NS16550_CFCR)(a0)

	li	t1, 0
	sb	t1, 1(a0)
	li	t1, 0x36	# divider, highest possible baud rate
	sb	t1, 0(a0)

1:
	lb	a1, 0(a0)
	bne	a1, t1, 1b
	nop

	li	t1, 3	#CFCR_8BITS
	sb	t1, 3(a0)	#NSREG(NS16550_CFCR)(v0)

	li	t1, 71
	sb	t1, 2(a0)
#endif
 
        j       ra
        nop
	.set at
END(initserial)

LEAF(initserial_later)
	.set noat
	move 	AT,ra

#if 1   //mtf add
	li	a0, COM1_BASE_ADDR
	li	t1, CFCR_DLAB
	sb	t1, NSREG(NS16550_CFCR)(a0)

	li	t1, 0
	sb	t1, 1(a0)
	li	t1, 68	# divider, highest possible baud rate
	sb	t1, 0(a0)

1:
	lb	a1, 0(a0)
	bne	a1, t1, 1b
	nop

	li	t1, 3	#CFCR_8BITS
	sb	t1, 3(a0)	#NSREG(NS16550_CFCR)(v0)

	li	t1, 71
	sb	t1, 2(a0)
#endif
 
        j       ra
        nop
	.set at
END(initserial_later)

LEAF(tgt_putchar)
	.set noat
	move	AT, ra
	la	v0, COM1_BASE_ADDR
	bal	1f
	nop
	jr	AT
	nop
  
1:
	lbu	v1, NSREG(NS16550_LSR)(v0)
	and	v1, LSR_TXRDY
	beqz	v1, 1b
	nop
						 
	sb	a0, NSREG(NS16550_DATA)(v0)
	j	ra
	nop
	.set at
END(tgt_putchar)

LEAF(tgt_testchar)
	.set noat
	move	AT, ra
        la      v0, COM1_BASE_ADDR
	bal	1f
	nop
	jr	AT
	nop
1:
        lbu     v1, NSREG(NS16550_LSR)(v0)
        and     v0, v1, LSR_RXRDY
        jr	ra
        nop
	.set	at
END(tgt_testchar)

LEAF(tgt_getchar)
	.set	noat
	move	AT, ra
	la	v0, COM1_BASE_ADDR
	bal	1f
	nop
	jr	AT
	nop
1:
	lbu	v1, NSREG(NS16550_LSR)(v0)
	and	v1, LSR_RXRDY
	beqz	v1, 1b
	nop
	lb	v0, NSREG(NS16550_DATA)(v0)
	jr ra
	nop
	.set at
END(tgt_getchar)

LEAF(beep_on)
	/* enable gpio35 output */
	li	t1, 0xbfe10504
	lw	t0, 0(t1)
	and	t0, 0xfffffff7
	sw	t0, 0(t1)
	/* set gpio35 high */
	li	t1, 0xbfe10514
	lw	t0, 0(t1)
	or	t0, 0x0008
	sw	t0, 0(t1)
	
	jr	ra
	nop
END(beep_on)
	
LEAF(beep_off)
	/* enable gpio0 output */
	li	t1, 0xbfe10504
	lw	t0, 0(t1)
	or	t0, 0xffffffff
	sw	t0, 0(t1)
	/* set gpio0 low */
	li	t1, 0xbfe10514
	lw	t0, 0(t1)
	and	t0, 0xfff7
	sw	t0, 0(t1)

	jr	ra
	nop
END(beep_off)

#if 1 //used i2c read ddr
#include "i2c.S"
#endif

LEAF(spd_info_store)
	move    t8, ra

	TTYDBG("\r\n spd_info_store begin.\r\n")

	dli    t5, 0xffffffff8fffa000;

	dli    t7, 0xa9
	dli    t6, 0xad

	move	a0, t5
	daddiu	a1, a0, 0x200
1:
	sb	zero, 0(a0)
	daddiu	a0, 1
	bltu	a0, a1, 1b
	nop

4:
	move    a0, t7
	dli     a1, 0x2
	//GET_I2C_NODE_ID_a2
	bal     i2cread
	nop
	dli     t3, 0x80
	bltu    v0, t3, 2f
	nop
	move    t3, t5
	daddiu  t3, 0x100;
	move    t4, t5
1:
	sb      zero, 0(t4)
	daddiu  t4, 0x1
	bltu    t4, t3, 1b
	nop
	b       3f
	nop
2:
	move    t4, t5
	dli     t0, 0x0 //used as counter

1:
	move    a0, t7
	move    a1, t0
	//GET_I2C_NODE_ID_a2
	bal     i2cread
	nop
	sb      v0, 0(t4)
	dli     a1, 0x100
	daddiu  t4, 0x1
	daddiu  t0, 0x1
	bne     t0, a1, 1b
	nop
3:
	daddiu  t5, 0x100
	daddiu  t7, 0x2
	bltu    t7, t6, 4b
	nop

	TTYDBG("\r\n spd_info_store done.\r\n")
	jr      t8
	nop
END(spd_info_store)

#######################################
#ifdef  AUTO_DDR_CONFIG
//#include "ddr_dir/detect_node_dimm.S"
#include "ddr_dir/detect_node_dimm_all.S"
#endif

#######################################
#include "ddr_dir/ls3A8_ddr_config.S"
#ifdef DDR3_DIMM
#include "ddr_dir/loongson3C_ddr3_leveling.S"
#endif
#ifdef ARB_LEVEL
#include "ddr_dir/ARB_level_new.S"
#endif
#ifdef  DEBUG_DDR
#include "ddr_dir/Test_Mem.S"
#endif

LEAF(watchdog_close)
	//disable watch DOG.
	/*gpio 3 output zero*/
	li t1,0xbfe10500

	li t2,(1 << 3)
	not t2,t2
	lw t3,0x0(t1)
	and t2,t3
	sw t2,0x0(t1)

	li t2,(1 << 3)
	lw t3,0x10(t1)
	not t2,t2
	and t2,t3
	sw t2,0x10(t1)
	nop

	jr		ra
	nop
END(watchdog_close)

	.text
	.global  nvram_offs
	.align 12
nvram_offs:
	.dword 0x0
	.align 12
#######################################

    .rdata
    .global ddr2_reg_data
    .global ddr3_reg_data

    .align  5
#include "loongson_mc2_param.S"
