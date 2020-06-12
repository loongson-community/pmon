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

#include "loongson3_def.h"

#define HT_CONFIG_ADDR 0xba000000

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

/* NOTE!! Not more that 16 instructions here!!! Right now it's FULL! */
	.set	push
	.set	mips64
	mfc0	t0, $16, 6
	or	t0, 0x100
	xori	t0, 0x100
	mtc0	t0, $16, 6

        /* no sw combine */
        mfc0    t0, $16,  6
        ori     t0, 0x200
        mtc0    t0, $16,  6

	mfc0	t0, $22
	lui	t1, 0x0000
//	lui	t1, 0x8000
	or	t0, t1, t0
	mtc0	t0, $22
	.set	pop

	mtc0	zero, COP_0_STATUS_REG
	mtc0	zero, COP_0_CAUSE_REG
	li	t0, SR_BOOT_EXC_VEC	/* Exception to Boostrap Location */
	mtc0	t0, COP_0_STATUS_REG
	la	sp, stack
	la	gp, _gp


	bal	locate			/* Get current execute address */
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
	daddu	v0, v1
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
	PRINTSTR("\r\nBADADDR=")
	mfc0	a0, COP_0_BAD_VADDR
	bal	hexserial
	nop
1:
	b 1b
	nop
#ifndef ROM_EXCEPTION
	PRINTSTR("\r\nDERR0=")
	mfc0	a0, COP_0_DERR_0
	bal	hexserial
	nop
	PRINTSTR("\r\nDERR1=")
	mfc0	a0, COP_0_DERR_1
	bal	hexserial
	nop
#endif
1:
	b 1b
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

	li	t0,SR_BOOT_EXC_VEC
	mtc0	t0,COP_0_STATUS_REG
	mtc0    zero,COP_0_CAUSE_REG
	.set noreorder

	li	bonito,PHYS_TO_UNCACHED(BONITO_REG_BASE)


	mfc0    t0, CP0_STATUS
	li      t1, 0x00e0      # {cu3,cu2,cu1,cu0}<={0110, status_fr<=1
	or      t0, t0, t1
	mtc0    t0, CP0_STATUS

	/* here we get l2 cache initialized */
	.set mips64
	mfc0	t0, $15, 1
	.set mips3
	andi	t0, t0, 0x3ff
	dli	a0, 0x9800000000000000
	andi	t1, t0, 0x3		/* core id */
	dsll	t2, t1, 18
	or	a0, t2, a0		/* 256KB offset for the each core */
	andi	t2, t0, 0xc		/* node id */
	dsll	t2, 42
	or	a0, t2, a0		/* get the L2 cache address */


	dsll	t1, t1, 8
	or	t1, t2, t1

	dli	t2, NODE0_CORE0_BUF0
	or	t1, t2, t1

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

        bal     initserial
        nop


bsp_start:
	PRINTSTR("\r\nPMON2000 MIPS Initializing. Standby...\r\n")
	bnez	s0, 1f
	nop

	li	a0, 128
	la	v0, initmips
	jr	v0
	nop
1:
	dli     a0, BOOTCORE_ID
    bal     hexserial
    nop


#include "pcitlb.S" /* map 0x1000000-0x1700000 to 0x4000000 */


	## enable kseg0 cachablilty####
	mfc0	t6, CP0_CONFIG
	ori	t6, t6, 7
	xori	t6, t6, 4
	mtc0	t6, CP0_CONFIG


	#jump to cached kseg0 address
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



    TTYDBG("Init Memory done.\r\n")


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

	TTYDBG("\r\n  _edata = 0x")
	la	a0, _edata
	bal	hexserial
	nop

	TTYDBG("\r\n  _end = 0x")
	la	a0, _end
	bal	hexserial
	nop

#endif
	la	a0, start
	li	a1, 0x9fc00000
	la	a2, _edata
	/* copy text section */

1:
	lw	a3, 0(a1)
	sw	a3, 0(a0)
	daddu	a0, 4
	bne	a2, a0, 1b
	daddu	a1, 4

	PRINTSTR("copy text section done.\r\n")

	/* Clear BSS */
	la	a0, _edata
	la	a2, _end
2:
	sw	zero, 0(a0)
	daddu	a0, 4
	bne	a2, a0, 2b
	nop


	TTYDBG("Copy PMON to execute location done.\r\n")

	TTYDBG("sp=");
	move	a0, sp
	bal	hexserial
	nop

	li	a0, 4096*1024
	sw	a0, CpuTertiaryCacheSize /* Set L3 cache size */

	PRINTSTR("\r\n")

	/* pass pointer to kseg1 tgt_putchar */
	la	a1, tgt_putchar
	daddu	a1, a1, s0

	la	a2, stringserial
	daddu	a2, a2, s0

	move	a0,msize

	dli     t0, NODE0_CORE0_BUF0  #buf of cpu0 we need bootcore_id
	dli	t3, BOOTCORE_ID
	dsll    t3, 8
	or      t0, t0, t3
	li      t1, SYSTEM_INIT_OK
	sw      t1, FN_OFF(t0)
	nop

	la	v0, initmips
	jalr	v0
	nop
stuck:
	b	stuck
	nop


/* end of man start.S */

/*
 *  Clear the TLB. Normally called from start.S.
 */
#if __mips64
#define MTC0 dmtc0
#else
#define MTC0 mtc0
#endif

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
	daddu	a2, (0x01000000 >> PG_SHIFT)
	MTC0   a2, COP_0_TLB_LO1	# Set up entry low1.

	mtc0    a3, COP_0_TLB_INDEX	# Set the index.
	addiu	a3, 1
	li	a2, 0x02000000
	subu	a1, a2
	nop
	tlbwi				# Write the TLB

	bgtz	a1, 1b
	daddu	a0, a2			# Step address 32Mb.

	jr	ra
	nop
END(CPU_TLBInit)

LEAF(stringserial)
	move	a2, ra
	daddu	a1, a0, s0
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
	jr	ra
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
	move	ra, a2
	jr	ra
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
	daddu	v0, s0
#endif
	daddu	v0, a0
	bal	tgt_putchar
	lbu	a0, 0(v0)

	bnez	a3, 1b
	daddu	a3, -1

	move	ra, a2
	jr	ra
	nop
END(hexserial)


LEAF(tgt_putchar)
	la	v0,GS3_UART_BASE
1:
	lbu	v1, NSREG(NS16550_LSR)(v0)
	and	v1, LSR_TXRDY
#	li	v1, 1
	beqz	v1, 1b
	nop

	sb	a0, NSREG(NS16550_DATA)(v0)
	sync
	move	v1, v0
	la	v0, GS3_UART_BASE
	bne	v0, v1, 1b
	nop

	jr	ra
	nop
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

	li	t1,128
	sb	t1,3(a0)
#ifdef BONITO_33M
	li      t1,0x12      # divider, highest possible baud rate,for 33M crystal
	//li      t1,0x36      # divider, highest possible baud rate,for 33M crystal
#else
#ifdef BONITO_25M
	li      t1,0x0e      # divider, highest possible baud rate,for 25M crystal
#else
#ifdef BONITO_50M
	li      t1,0x1b      # divider, highest possible baud rate,for 50M crystal
#endif
#endif
#endif
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


__main:
	jr	ra
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





LEAF(nullfunction)
	jr ra
	nop
END(nullfunction)


LEAF(tlb_init)
	mtc0    zero, CP0_PAGEMASK
	lui     a0, 0x8000
	li	a1, 64
	dmtc0    zero, CP0_ENTRYLO0
	dmtc0    zero, CP0_ENTRYLO1
	move	v0, zero
1:
	dmtc0    a0, CP0_ENTRYHI
	mtc0    v0, CP0_INDEX
	tlbwi
	daddiu   v0, v0, 1
	bne     v0, a1, 1b
	daddiu   a0, a0, 0x2000
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
	move ra, t7
	jr	ra
END(hexserial64)

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
	sd	zero, A1_OFF(t1)


	jr	ra
	nop
END(clear_mailbox)



LEAF(pci_read_config32_mips)
	dli	t1, HT_CONFIG_ADDR
	or	t2, t1, a0
	or	t1, t2, a1
	lw	v0, 0x0(t1)
	jr	ra
	nop
END(pci_read_config32_mips)

LEAF(pci_write_config32_mips)
	dli	t1, HT_CONFIG_ADDR
	or	t2, t1, a0
	or	t1, t2, a1
	sw	a2, 0x0(t1)
	jr	ra
	nop
END(pci_write_config32_mips)

LEAF(pci_read_config8_mips)
	dli	t1, HT_CONFIG_ADDR
	or	t2, t1, a0
	or	t1, t2, a1
	lb	v0, 0x0(t1)
	jr	ra
	nop
END(pci_read_config8_mips)

LEAF(pci_write_config8_mips)
	dli	t1, HT_CONFIG_ADDR
	or	t2, t1, a0
	or	t1, t2, a1
	sb	a2, 0x0(t1)
	jr	ra
	nop
END(pci_write_config8_mips)

LEAF(pci_read_config8)
	dli t1, HT_CONFIG_ADDR
	or  t2, t1, a0
	or  t1, t2, a1
	lb  v0, 0x48(t1)
	jr  ra
	nop
END(pci_read_config8)

LEAF(pci_write_config8)
	dli t1, HT_CONFIG_ADDR
	or  t2, t1, a0
	or  t1, t2, a1
	sb  a2, 0x48(t1)
	jr  ra
	nop
END(pci_write_config8)


	.ent    slave_main
slave_main:

	dli     t2, NODE0_CORE0_BUF0
	dli	t3, BOOTCORE_ID
	dsll    t3, 8
	or      t2, t2, t3

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


#######################################
	.text
	.global  nvram_offs
	.align 12
nvram_offs:
	.dword 0x0
	.align 12

#######################################

