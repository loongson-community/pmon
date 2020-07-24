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
#include "target/bonito.h"

#include "loongson3_def.h"
#include "ls7a_config.h"

#include "../../../pmon/arch/mips/ls7a/ls7a_define.h"
#include "../../../pmon/arch/mips/ls7a/ht.h"
#include "ddr4_dir/ls3a4000_reg_def.h"

/*
 *	Register usage:
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

#define	CORE_FREQ	1800 /*stable timer and clksetting need it*/

	.set	noreorder
	.globl	_start
	.globl	start
	.globl	__main
_start:
start:
	.globl	stack
stack = start - 0x4000		/* Place PMON stack below PMON start in RAM */

/* NOTE!! Not more that 16 instructions here!!! Right now it's FULL! */
#ifdef	LS132_CORE
#define	LS132_CORE_ID 0x8000
	.set	push
	.set	mips32
	mfc0	t0, $15, 0
	.set mips3
	andi	t0, t0, 0xe000
	li	t2, LS132_CORE_ID
	beq	t0, t2, ls132_core_main
	nop
#endif
	.set	push
	.set	mips64

	/*This code is not required if the hardware is connected to HT reset*/
	UNUESED_HT_PIN_TO_GPIO(~DISABLE_UNUSED_HT_PIN)

	/* no sw combine */
	mfc0	t0, $16,  6
	ori	t0, 0x200
	and	t0, ~(1 << 16) //bit 16 for llexcen
	mtc0	t0, $16,  6

	mtc0	zero, COP_0_CAUSE_REG
	/* Exception to Boostrap Location */
	li	t0, SR_BOOT_EXC_VEC | SR_KX | SR_SX | SR_UX
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
#ifdef	LS132_CORE
	.set	push
	.set	noreorder
	.set	mips32
	//ls132 etc exception entry
	mfc0	t0, $15, 0
	andi	t0, t0, 0xe000
	li	t2, LS132_CORE_ID
	bne	t0, t2, 1f
	nop
	bal	timer_handler
	nop
	.set pop
1:
#endif
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


#ifdef	LS3A7A_STR
/*
 * 3A7A STR config start
 */
	.align  8			/* bfc00500*/
	.set mips64
	.set noreorder

	/*************************************************************************
	/* This Code Must Be Execute Before Memory SelfRefresh Begain,
	/* Because Once We Enter SelfRefresh Mode,Memory Can't Be Accessed Any More
	/* We Leave Poweroff Op Later(After Enter SelfRefresh Mode)
	**************************************************************************/
	/* store ra and sp to memory */
	dli	t0, 0x900000000faaa040
	sd	a0, 0x0(t0) //store ra
	sd	a1, 0x8(t0) //store sp
	dli	t1, 0x5a5a5a5a5a5a5a5a
	sd	t1, 0x10(t0) //store str flag

	la	s0, start
	li	a0, 0xbfc00000
	subu	s0, a0, s0
	and	s0, 0xffff0000

	li	a0, GS3_UART_BASE
	bal	initserial
	nop

1:	lui	t0, 0xbfe0  /* Enable DDR control register  */
	lw	t1, 0x0180(t0)
	li	t2, 0xfffffdef
	and	t1, t1, t2
	li	t3, 0x00002000
	or	t1, t1, t3
	sw	t1, 0x0180(t0)
	sync
	.set mips64

	/* Set interleave_en is 0 */
	dli	t0, 0x900000001fe00400
	lw	t1, 0x4(t0)
	li	t2, ~(1 << 7)
	and	t1, t1, t2
	sw	t1, 0x4(t0)
	sync
	
	dli	t0, 0x900004000ff00000
	ld	t1, 0x1308(t0)
	dli	t2, 0xff
	or	t1, t1, t2
	sd	t1, 0x1308(t0)
	sync
	
	dli	t0, 0x900005000ff00000
	ld	t1, 0x1308(t0)
	dli	t2, 0xff
	or	t1, t1, t2
	sd	t1, 0x1308(t0)
	sync

	/* info ec */
#ifdef	CETC32S
	li	t0,0x1
	li	t1,0xb80000b0
	sw	t0,0x0(t1)
#endif

	/* delay */
	li	t0, 0x1000
1:
	subu	t0, t0, 0x1
	bnez	t0, 1b
	nop

	li	a0,'S'
	bal	tgt_putchar
	nop
	li	a0,'3'
	bal	tgt_putchar
	nop

	dli	t0, (LS7A_MISC_BASE_ADDR | ACPI_BASE_ADDR_OFFSET)
	/* set key,usb wakeup of reg GPE0_EN */
	lw	t1, 0x2c(t0)
	li	t3, (0x1 << 8) | (0x3f << 10)
	or	t1, t1, t3
	sw	t1, 0x2c(t0)

	/* clear 0-15 of reg GPE0_STS */
	lw	t1, 0x28(t0)
	li	t3, 0x0000ffff
	sw	t3, 0x28(t0)

	/* clear 0-15 of reg PM1_STS */
	lw	t1, 0x0c(t0)
	li	t3, 0x0000ffff
	sw	t3, 0x0c(t0)

	/* set vsb_gat_delay */
	lw	t1, 0x4(t0)
	li	t3, ((0x5 << 11) | (0x1 << 7))
	or	t1, t1, t3
	andi	t1, t1, 0xefff
	sw	t1, 0x4(t0)

	/* cmos signed as s3 mode*/
	li	t3, 0x12345678
	sw	t3, 0x50(t0)

	/* set reg PM1_CNT to get into S3*/
	li	t3, 0x00003400
	sw	t3, 0x14(t0)

	/* delay */
	li	t0, 0x40000
1:
	subu	t0, t0, 0x1
	bnez	t0, 1b
	nop
1:
	b	1b
	nop
#endif	/*str end*/

	/* Debug exception */
	.align  7			/* bfc00480 */
#include "exc_ejtag.S"

exc_common:
	.set mips64
	mfc0	t0, $15, 1
	.set mips3
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
	b	1b
	nop
#ifndef	ROM_EXCEPTION
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
	//pmon gcc4.4 do not support csr instruction
	#RDCSR	(0x32<<26 | 0x18 | 0x4<<6 | 0x0<<16 | T0<<21 | T1<<11)
	#WRCSR	(0x32<<26 | 0x18 | 0x4<<6 | 0x1<<16 | T0<<21 | T1<<11)
	#DRDCSR	(0x32<<26 | 0x18 | 0x4<<6 | 0x2<<16 | T0<<21 | T1<<11)
	#DWRCSR	(0x32<<26 | 0x18 | 0x4<<6 | 0x3<<16 | T0<<21 | T1<<11)
	/*cfg csr addr = (((register num & 0x3fff) << 2) + 0xfffe0000)*/
	.set	mips64

#if 0
	/*use csr configure cpu prid*/
	li	t0, 0xfffe0000
	li	t1, 0x0014630d //write this ID as 3a3000
	.word (0x32<<26 | 0x18 | 0x4<<6 | 0x1<<16 | T0<<21 | T1<<11)
	/*end*/
#endif
	/*set hardware non alignment access enable*/
	li	t0, 0xfffe0004
	.word (0xc8000118 | (T0 << 21) | (T1 << 11))
	and	t1, ~((1 << 21) | (1 << 22))
	or	t1, ((1 << 25) | (1 << 26))
	.word (0x32<<26 | 0x18 | 0x4<<6 | 0x1<<16 | T0<<21 | T1<<11)

	mfc0	t0, $16, 6
	or	t0, (1 << 10)
	mtc0	t0, $16, 6
	/*end*/

	/*lpixu lpixnu*/
	li	t0, 0xfffe0008
	.word (0xc8000118 | (T0 << 21) | (T1 << 11))
	or	t1, ((1 << 3) | (1 << 4))
	.word (0x32<<26 | 0x18 | 0x4<<6 | 0x1<<16 | T0<<21 | T1<<11)
	/*end*/

#if 0
	/*set position independent enable*/
	li	t0, 0xfffe0008
	.word (0xc8000118 | (T0 << 21) | (T1 << 11))
	or	t1, ((1 << 13) | (1 << 14))
	.word (0x32<<26 | 0x18 | 0x4<<6 | 0x1<<16 | T0<<21 | T1<<11)
	/*end*/
#endif
	/*set stable counter clk (CCFreq * CFM / CFD) = clk*/
	li	t0, 0xfffe0010
	li	t1, 100000000 //100M Hz
	.word (0x32<<26 | 0x18 | 0x4<<6 | 0x1<<16 | T0<<21 | T1<<11)
	li	t0, 0xfffe0014
	li	t1, (CORE_FREQ / 100) | (1 << 16) /* [31:16]->CDF | [15:0]->CFM */
	.word (0x32<<26 | 0x18 | 0x4<<6 | 0x1<<16 | T0<<21 | T1<<11)
	/*end*/

	/*UCAWINP / UCAWINP | GCCAEQRP*/
	mfc0	t0, $15, 0
	andi	t0, 0x1fff
	li	t1, 0x3
	li	t2, 0x2
	movz	t1, t2, t0

	li	t0, 0xfffe001c
	.word (0x32<<26 | 0x18 | 0x4<<6 | 0x1<<16 | T0<<21 | T1<<11)
	/*end*/

	//dissable mcsr write function
	dli	t0, 0xffffffffffff0000
	li	t1, 0
	.word (0x32<<26 | 0x18 | 0x4<<6 | 0x1<<16 | T0<<21 | T1<<11)

	.set mips64
	/*low power consumption config*/
	mfc0	t0, $16, 7
	ori	t0, 0x5
	mtc0	t0, $16, 7
//#define	MCC
#ifndef	MCC
	/*mca clock*/
	li	t0, 0xbfe00180
	lw	t1, 0x0(t0)
	and	t1, ~((1 << 6) | (1 << 11))
	sw	t1, 0x0(t0)
#endif

	/*disable temperature feature,0x420 Disable_ID should not be 1*/
	li	t0, 0xbfe00008
	lw	t1, 0x0(t0)
	and	t1, ~(1 << 0)
	sw	t1, 0x0(t0)

	/* spi speedup */
	li	t0, 0xbfe001f0 //address was changed
#ifdef	BONITO_100M
	li	t1, 0x47
	sb	t1, 0x4(t0)
#elif	BONITO_25M
	li	t1, 0x7
	sb	t1, 0x4(t0)
#endif

#ifdef	SPI_QUAD_IO
	//open this code should be ensure the spi support this mode
	/* spi quad_io */
	li	t1, 0xb
	sb	t1, 0x6(t0)
1:
	lbu	a0, 0x6(t0)
	bne	a0, t1, 1b
	nop
#endif
	la	s0, start
	subu	s0, ra, s0
	and	s0, 0xffff0000

	.set noreorder

	li	bonito,PHYS_TO_UNCACHED(BONITO_REG_BASE)

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
	dli	a0, BOOTCORE_ID
	bne	t0, a0, slave_main
	nop

	li	a0, GS3_UART_BASE
	bal	initserial
	nop

#ifndef	MULTI_CHIP
	//configure uart1 pin enable
	dli	a0, 0x900000001fe00500
	lw	a1, 0x4(a0)
	ori	a1, 0x3fc
	sw	a1, 0x4(a0)

	li	a0, GS3_UART1_BASE
	bal	initserial
	nop
#endif

#define	SHUT_SLAVES
#ifdef	SHUT_SLAVES
	PRINTSTR("Shut down slave cores\r\n")
	li	a0, 0xbfe001d0
	li	a1, BOOTCORE_ID
	sll	a1, 2
	li	t1, 0xf
	sll	a1, t1, a1
	sw	a1, 0x0(a0)

#else
	PRINTSTR("\r\nNOT Shut down slave cores\r\n")
#endif

bsp_start:
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
/*
	li	t0, 0xbfe00420
	lw	a0, 0x0(t0)
	li	t1, 0x30000000
	not	t1, t1
	and	a0, t1
	sw	a0, 0x0(t0)
*/
/* configure voltage chip
 * we should use this code set node voltage
*/
	WatchDog_Close

//#define INPUT_PARAM
#define VOLTIGE_CTRL
#define LOONGSON_BOARD
#define MPS_V
#ifdef VOLTIGE_CTRL
	//node 0
	move	s1, zero
	TTYDBG("\r\nnode 0 N Voltage  write :\r\n")
	bal	v_n_ctrl
	nop
#ifdef PV3205_V
	bal	v_io_ctrl
	nop
#endif
	TTYDBG("\r\nnode 0 N Voltage  read :\r\n")
	bal	v_n_ctrl_read
	nop

	//node 0
	move	s1, zero
	TTYDBG("\r\nnode 0 P Voltage write :\r\n")
	bal	v_p_ctrl
	nop
#ifdef MULTI_CHIP
	//node 1
	dli	s1, 0x0000100000000000
	TTYDBG("\r\nnode 1 N Voltage write :\r\n")
	bal	v_n_ctrl
	nop
#ifdef PV3205_V
	bal	v_io_ctrl
	nop
#endif
	TTYDBG("\r\nnode 1 N Voltage read :\r\n")
	bal	v_n_ctrl_read
	nop

	TTYDBG("\r\nnode 1 P Voltage write :\r\n")
	bal	v_p_ctrl
	nop
#ifdef CHIP_4
	//node 2
	dli	s1, 0x0000200000000000
	TTYDBG("\r\nnode 2 N Voltage write :\r\n")
	bal	v_n_ctrl
	nop
#ifdef PV3205_V
	bal	v_io_ctrl
	nop
#endif
	TTYDBG("\r\nnode 2 N Voltage read :\r\n")
	bal	v_n_ctrl_read
	nop
	TTYDBG("\r\nnode 2 P Voltage write :\r\n")
	bal	v_p_ctrl
	nop

	//node 3
	dli	s1, 0x0000300000000000
	TTYDBG("\r\nnode 3 N Voltage write :\r\n")
	bal	v_n_ctrl
	nop
#ifdef PV3205_V
	bal	v_io_ctrl
	nop
#endif
	TTYDBG("\r\nnode 3 N Voltage read :\r\n")
	bal	v_n_ctrl_read
	nop
	TTYDBG("\r\nnode 3 P Voltage write :\r\n")
	bal	v_p_ctrl
	nop
#endif
#endif
#endif

	//Read sys_clk_sel
	TTYDBG	("\r\n0xbfe00190  : ")
	li	t2,0xbfe00190
	ld	t1, 0x0(t2)
	dsrl	a0, t1, 32
	bal	hexserial
	nop
	move	a0, t1
	bal	hexserial
	nop
	TTYDBG	("\r\nCPU CLK SEL : ")
	dsrl	t1, t1, 32
	andi	a0, t1, 0x1f
	bal	hexserial
	nop
	TTYDBG	("\r\nMEM CLK SEL : ")
	dsrl	t0, t1, 5
	andi	a0, t0, 0x1f
	bal	hexserial
	nop
	TTYDBG	("\r\nHT CLK SEL : ")
	dsrl	t0, t1, 10
	andi	a0, t0, 0x3f
	bal	hexserial
	nop
	TTYDBG ("\r\n")

//USING S1 FOR PASSING THE NODE ID
	dli	s1, 0X0000000000000000
#include "loongson3_clksetting.S"
#ifndef MULTI_CHIP//SRF
//first init mc0 & mc1, no matter wheather there is DIMM
//enable mc0 and mc1, and interleave disable
#if 0
    dli     t2, 0x900000001fe00400
    ld      t3, 0x0(t2)
    dli     t1, (3<<30)
    or      t3, t3, t1
    dli     t1, ~(1<<39)
    and     t3, t3, t1
    sd      t3, 0x0(t2)
//enbale mc config space
    dli     t2, 0x900000001fe00180
    ld      t3, 0x0(t2)
    dli     t1, (1<<5 | 1<<10)
    or      t3, t3, t1
    dli     t1, ~(1<<4 | 1<<9)
    and     t3, t3, t1
    sd      t3, 0x0(t2)
#endif

    dli     t0, 0x900004000ff00000
    dli     t1, 0x900005000ff00000
//set mirror
    dli     t2, 0xaa
    sb      t2, 0x1208(t0)
    sb      t2, 0x1208(t1)
//set cs_infor for 2cs
    dli     a0, 0x2
    sb      a0, DDR4_CS_DIFF_OFFSET(t0)
    sb      a0, DDR4_CS_DIFF_OFFSET(t1)

    dli     a0, 0x3
    sb      a0, DDR4_CS_ENABLE_OFFSET(t0)
    sb      a0, DDR4_CS_MRS_OFFSET(t0)
    sb      a0, DDR4_CS_ZQ_OFFSET(t0)
    sb      a0, DDR4_CS_ZQCL_OFFSET(t0)
    sb      a0, DDR4_CS_RESYNC_OFFSET(t0)
    sb      a0, DDR4_CS_REF_OFFSET(t0)

    sb      a0, DDR4_CS_ENABLE_OFFSET(t1)
    sb      a0, DDR4_CS_MRS_OFFSET(t1)
    sb      a0, DDR4_CS_ZQ_OFFSET(t1)
    sb      a0, DDR4_CS_ZQCL_OFFSET(t1)
    sb      a0, DDR4_CS_RESYNC_OFFSET(t1)
    sb      a0, DDR4_CS_REF_OFFSET(t1)

    dli     a0, 0x10
    sw      a0, DDR4_CS_MAP_OFFSET(t0)
    sw      a0, DDR4_CS_MAP_OFFSET(t1)
    dli     a0, 0x21
    sw      a0, DDR4_CKE_MAP_OFFSET(t0)
    sw      a0, DDR4_CKE_MAP_OFFSET(t1)

    sw      a0, DDR4_WRODT_MAP_OFFSET(t0)
    sw      a0, DDR4_WRODT_MAP_OFFSET(t1)

//config clk 2x
    dli     a0, 0xe1e1c1
    sw      a0, DLL_CTRL(t0)
    sw      a0, DLL_CTRL(t1)

//use DBL
    dli     a0, 0x2a
    sd      a0, 0x40(t0)
    sd      a0, 0x40(t1)

    dli     a0, 0x140
    sd      a0, 0x38(t0)
    sd      a0, 0x38(t1)
//pad set
    dli     a0, 0x0506647f0026d2df
    sd      a0, 0x800(t0)
    sd      a0, 0x800(t1)

    dli     a0, 0x000101010003ed79
    sd      a0, 0x808(t0)
    sd      a0, 0x808(t1)

    dli     a0, 0x0a010a010a010a01
    sd      a0, 0x810(t0)
    sd      a0, 0x818(t0)
    sd      a0, 0x810(t1)
    sd      a0, 0x818(t1)

    dli     a0, 0x89bf89bf89bf89bf
    sd      a0, 0x840(t0)
    sd      a0, 0x848(t0)
    sd      a0, 0x840(t1)
    sd      a0, 0x848(t1)

    dli     a0, 0xb42
    sd      a0, 0x830(t0)
    sd      a0, 0x830(t1)
//phy init
    lb      a0, 0x10(t0)
    ori     a0, 0x2
    sb      a0, 0x10(t0)

    lb      a0, 0x10(t1)
    ori     a0, 0x2
    sb      a0, 0x10(t1)

//dll init done check
1:
    ld      a0, 0x030(t0)
    dli     a1, (0x1 << 40)
    and     a0, a0, a1
    beqz    a0, 1b
    nop

1:
    ld      a0, 0x030(t1)
    dli     a1, (0x1 << 40)
    and     a0, a0, a1
    beqz    a0, 1b
    nop
//dll lock check
//mc0
2:
//dll resetn
    lw      a0, (DLL_CTRL)(t0)
    dli     a1, 0x7f7f7f
    and     a0, a0, a1
    sw      a0, (DLL_CTRL)(t0)

    dli     a0, 0x60
1:
    dsubu   a0, 1
    bnez    a0, 1b
    nop
//clear dll resetn
    lw      a0, (DLL_CTRL)(t0)
    dli     a1, 0x808080
    or      a0, a0, a1
    sw      a0, (DLL_CTRL)(t0)

    lbu     a0, (DLL_CTRL)(t0)
    lbu     a0, (DLL_CTRL)(t0)
    lbu     a0, (DLL_CTRL)(t0)
    lbu     a0, (DLL_CTRL)(t0)
    lbu     a0, (DLL_CTRL)(t0)

    li      t3,0x100000
1:
    lb      a0, 0x4a(t0)
    andi    a0, a0, 0x1
    lb      a1, 0x7a(t0)
    andi    a1, a1, 0x1
    lb      a2, 0xba(t0)
    andi    a2, a2, 0x1
    and     a1, a1, a2
    and     a0, a1, a0
    subu    t3, 1
    beqz    t3, 1f
    nop

    beqz    a0, 1b
    nop

1:
    beqz    a0, 2b
    nop

//mc1
2:
//dll resetn
    lw      a0, (DLL_CTRL)(t1)
    dli     a1, 0x7f7f7f
    and     a0, a0, a1
    sw      a0, (DLL_CTRL)(t1)

    dli     a0, 0x60
1:
    dsubu   a0, 1
    bnez    a0, 1b
    nop
//clear dll resetn
    lw      a0, (DLL_CTRL)(t1)
    dli     a1, 0x808080
    or      a0, a0, a1
    sw      a0, (DLL_CTRL)(t1)

    lbu     a0, (DLL_CTRL)(t1)
    lbu     a0, (DLL_CTRL)(t1)
    lbu     a0, (DLL_CTRL)(t1)
    lbu     a0, (DLL_CTRL)(t1)
    lbu     a0, (DLL_CTRL)(t1)

    li      t3,0x100000
1:
    lb      a0, 0x4a(t1)
    andi    a0, a0, 0x1
    lb      a1, 0x7a(t1)
    andi    a1, a1, 0x1
    lb      a2, 0xba(t1)
    andi    a2, a2, 0x1
    and     a1, a1, a2
    and     a0, a1, a0
    subu    t3, 1
    beqz    t3, 1f
    nop

    beqz    a0, 1b
    nop

1:
    beqz    a0, 2b
    nop

//clk2x enable
    ld      a0, 0x48(t0)
    ori     a0, a0, (1 << 8)
    sd      a0, 0x48(t0)
    ld      a0, 0x78(t0)
    ori     a0, a0, (1 << 8)
    sd      a0, 0x78(t0)
    ld      a0, 0xb8(t0)
    ori     a0, a0, (1 << 8)
    sd      a0, 0xb8(t0)

    ld      a0, 0x48(t1)
    ori     a0, a0, (1 << 8)
    sd      a0, 0x48(t1)
    ld      a0, 0x78(t1)
    ori     a0, a0, (1 << 8)
    sd      a0, 0x78(t1)
    ld      a0, 0xb8(t1)
    ori     a0, a0, (1 << 8)
    sd      a0, 0xb8(t1)

//mc init
    lb      a0, 0x10(t0)
    ori     a0, a0, 0x1
    sb      a0, 0x10(t0)

    lb      a0, 0x10(t1)
    ori     a0, a0, 0x1
    sb      a0, 0x10(t1)

    dli     a1, 0x3 //cs_enable
1:
    lb      a0, 0x11(t0)
    bne     a0, a1, 1b
    nop

1:
    lb      a0, 0x11(t1)
    bne     a0, a1, 1b
    nop

/*set memory controller selfrefresh*/
    dli     a1, 0x00000000000000ff
    sd      a0, 0x1308(t0)
    sd      a0, 0x1308(t1)

    dli     a0, 0x100
1:
    dsubu   a0, a0, 1
    bnez    a0, 1b
    nop

//enbale mc config space
    dli     t0, 0x900000001fe00180
    ld      t3, 0x0(t0)
    dli     t1, (1<<7 | 1<<12)
    not     t1, t1
    and     t3, t3, t1
    sd      t3, 0x0(t0)

    dli     a0, 0x1000
1:
    dsubu   a0, a0, 1
    bnez    a0, 1b
    nop

    dli     t1, (1<<7 | 1<<12)
    or      t3, t3, t1
    sd      t3, 0x0(t0)
#endif
#ifdef MULTI_CHIP
//USING S1 FOR PASSING THE NODE ID
	dli	s1, 0x0000100000000000
#include "loongson3_clksetting.S"
#ifdef CHIP_4
	dli	s1, 0x0000200000000000
#include "loongson3_clksetting.S"
	dli	s1, 0x0000300000000000
#include "loongson3_clksetting.S"
#endif
#endif

##########################################
#include "loongson3_fixup.S"

	/*sram ctrl*/
	LS3A4000_SRAM_CTRL(0)
	STABLE_COUNTER_CLK_EN(0)
#ifdef MULTI_CHIP
	SET_GPIO_FUNC_EN(0,(1 << 13))
	SET_GPIO_FUNC_EN(1,(1 << 13))
	/*sram ctrl*/
	LS3A4000_SRAM_CTRL(1)
	STABLE_COUNTER_CLK_EN(1)
#ifdef	CHIP_4
	SET_GPIO_FUNC_EN(2,(1 << 13))
	SET_GPIO_FUNC_EN(3,(1 << 13))
	/*sram ctrl*/
	LS3A4000_SRAM_CTRL(2)
	LS3A4000_SRAM_CTRL(3)

	STABLE_COUNTER_CLK_EN(2)
	STABLE_COUNTER_CLK_EN(3)
#endif
#endif

#ifdef MULTI_CHIP
	/*sync stable counter*/
	GPIO_CLEAR_OUTPUT(1 << 12)
	GPIO_SET_OUTPUT(1 << 12)
#else
	/*stable_reset*/
	dli	t0, 0x900000001fe00420
	lw	t1, 0x0(t0)
	or	t1, (1 << 21)
	sw	t1, 0x0(t0)
	lw	t1, 0x0(t0)
	xor	t1, (1 << 21)
	sw	t1, 0x0(t0)
#endif

#ifdef MULTI_CHIP
#ifdef CHIP_4
	TTYDBG("\r\nSet 4-way nodemask.\r\n")
	SET_NODEMASK(0, 0x3)
	SET_NODEMASK(1, 0x3)
	SET_NODEMASK(2, 0x3)
	SET_NODEMASK(3, 0x3)
#else
	TTYDBG("\r\nSet 2-way nodemask.\r\n")
	SET_NODEMASK(0, 0x1)
	SET_NODEMASK(1, 0x1)
#endif
#else
	SET_NODEMASK(0, 0x0)
#endif

#ifdef MCC
#set mcc_en
	dli	t0, 0x900000003ff00400
	ld	t2, 0x0(t0)
	dli	t1, (0x1<<12)
	or	t2, t2, t1
	sd	t2, 0x0(t0)
#ifdef MULTI_CHIP
//node 1
	dli	t0, 0x900010003ff00400
	ld	t2, 0x0(t0)
	dli	t1, (0x1<<12)
	or	t2, t2, t1
	sd	t2, 0x0(t0)
#ifdef CHIP_4
//node 2
	dli	t0, 0x900020003ff00400
	ld	t2, 0x0(t0)
	dli	t1, (0x1<<12)
	or	t2, t2, t1
	sd	t2, 0x0(t0)
//node 3
	dli	t0, 0x900030003ff00400
	ld	t2, 0x0(t0)
	dli	t1, (0x1<<12)
	or	t2, t2, t1
	sd	t2, 0x0(t0)
#endif
#endif
#endif

##########################################
	PRINTSTR("NO TLB cache init ...\r\n")

#include "pcitlb.S" /* map 0x4000000-0x7fffffff to 0xc0000000 */

/*
 *  Reset and initialize l1 caches to a known state.
 */
	## enable kseg0 cachablilty####
	mfc0	t6, CP0_CONFIG
	ori	t6, t6, 7
	xori	t6, t6, 4
	mtc0	t6, CP0_CONFIG

	#jump to cached kseg0 address
	PRINTSTR("Jump to 9fc\r\n")
	lui	t0, 0xdfff 
	ori	t0, t0, 0xffff
	bal	1f
	nop
1:
	and	ra, ra, t0
	addiu	ra, ra, 16
	jr	ra
	nop
	/*now pc was jumped to 0x9fcxxxxx*/

	TTYDBG("32 bit PCI space translate to 64 bit HT space\r\n")
#include "loongson3_ht1_32addr_trans.S"
#if 1 //disable 7a link

check_ht1:
	TTYDBG("\r\nCheck HT bus up.")
	dli	t0, 0x90000efdfb000000
	li	t5, 0x1fffff
1:
	sub	t5, t5, 1
	beqz	t5, 1f
	nop
	lw	a0, 0x44(t0)
	li	a1, 0x20
	and	a0, a0, a1
	beqz	a0, 1b
	nop

	TTYDBG("\r\n")
	lw	a0, 0x44(t0)
	bal	hexserial
	nop
	TTYDBG("\r\n")
1:
	lw	a0, 0x44(t0)
	li	a1, 0x20
	and	a0, a0, a1
	bnez	a0, 1f
	nop

	TTYDBG("Reset Node 0 HT1 bus\r\n")
	lb	a0, 0x3e(t0)
	li	a1, 0x40
	or	a0, a0, a1
	sb	a0, 0x3e(t0)
	lw	a0, 0x3c(t0)
	bal	hexserial
	nop
	TTYDBG("\r\n")

	TTYDBG("Dereset Node 0 HT1 bus\r\n")
	lb	a0, 0x3e(t0)
	li	a1, 0x40
	not	a1, a1
	and	a0, a0, a1
	sb	a0, 0x3e(t0)
	lw	a0, 0x3c(t0)
	bal	hexserial
	nop
	TTYDBG("\r\n")

	b	check_ht1
	nop
1:
	//config fix address bar for Misc devices block
	dli	t0, MISC_HEADER_ADDR
	li	t1, MISC_BASE_ADDR
	sw	t1, 0x10(t0)
	lw	t2, 0x4(t0)
	ori	t2, t2, 0x2
	sw	t2, 0x4(t0)
	//change confbus base address
	dli	t0, CONFBUS_HEADER_ADDR
	li	t1, CONFBUS_BASE_ADDR
	sw	t1, 0x10(t0)
	lw	t2, 0x4(t0)
	ori	t2, t2, 0x2
	sw	t2, 0x4(t0)
	TTYDBG("set LS7A MISC and confbus base address done.\r\n")

	bal	beep_on
	nop


	//set PWM output 1
	dli	t0, (LS7A_MISC_BASE_ADDR | PWM_BASE_ADDR_OFFSET)
	li	a0, 0x100
	li	a1, (1<<0)
	sw	zero, 0x4(t0)
	sw	a0, 0x8(t0)
	sw	a1, 0xc(t0)
	sw	zero, 0x104(t0)
	sw	a0, 0x108(t0)
	sw	a1, 0x10c(t0)
	sw	zero, 0x204(t0)
	sw	a0, 0x208(t0)
	sw	a1, 0x20c(t0)
	sw	zero, 0x304(t0)
	sw	a0, 0x308(t0)
	sw	a1, 0x30c(t0)

	bal	beep_off
	nop

	//setup LS3A - 7A HT link start...
	//check 3A clksel setting
	li	t0, 0xbfe00190
	lw	a0, 0x4(t0)
	srl	a0, a0, 15
	beqz	a0, 3f
	nop
#ifdef	CHECK_HT_PLL_MODE
	TTYDBG("Warning: 3A HT in hard freq mode, please modify clksel[7].\r\n")
	dli	a0, 0x4000000
1:
	dsub	a0, a0, 1
	bnez	a0, 1b
	nop
#endif
	b	2f
	nop
3:
	TTYDBG("3A HT in soft freq cfg mode...ok\r\n")
2:

	//check 7A clksel setting
	dli	t0, (LS7A_MISC_BASE_ADDR + 0x60000)
	lb	a0, (0xa00+53)(t0)
	beqz	a0, 3f
	nop
#ifdef	CHECK_HT_PLL_MODE
	TTYDBG("Warning: 7A HT in hard freq mode, please modify clksel[7].\r\n")
	dli	a0, 0x4000000
1:
	dsub	a0, a0, 1
	bnez	a0, 1b
	nop
#endif
	b	2f
	nop
3:
	TTYDBG("7A HT in soft freq cfg mode...ok\r\n")
2:

	li	t2, ((HT1_HARD_FREQ_CFG << 12) | (HT1_HARD_FREQ_CFG << 8) | (HT1_GEN_CFG << 4) | (HT1_WIDTH_CFG << 1) | (HT1_RECONNECT << 0))

	li	t8, LS7A_HT1_SOFT_FREQ_CFG
	dsll	t3, t8, 32
	li	t8, LS3A_HT1_SOFT_FREQ_CFG
	or	t3, t3, t8

	bal     ls7a_version
	nop
	beqz    v0, 1f
	nop
	li      t8, LS7A_HT1_SOFT_FREQ_CFG_C
	dsll    t3, t8, 32
	li      t8, LS3A_HT1_SOFT_FREQ_CFG
	or      t3, t3, t8
1:

#ifdef	DEBUG_HT1
	PRINTSTR("HT1 default setting: \r\na1: 0x")
	move	a0, t2
	bal	hexserial
	nop
	PRINTSTR("\r\na2: 0x")
	dsrl	a0, t3, 32
	bal	hexserial
	nop
	move	a0, t3
	bal	hexserial
	nop
	PRINTSTR("\r\nInput parameter a1: ([15:12]: 7A freq-0/2/5/9; [11:8]: 3A freq-0/2/5/9; [7:4]: GENx-1/3; [1]: width-0/1; [0]: reconnect-0/1): ")
	bal	inputaddress
	nop
	beqz	v0, 1f
	nop
	move	t2, v0
1:
	PRINTSTR("\r\nInput parameter a2: ([3:0]: ht pll soft cfg sel. 0: 200M; 2: 400M; 5: 800M; 6: 1000M; 7: 1200M; 9: 1600M; b: 2000M; c: 2200M; d: 2400M; e: 2600M; f: 3200M;): ")
	bal	inputaddress
	nop
	move	t1, v0
	PRINTSTR("\r\n")

	move	a0, zero
	dli	t3, ((LS7A_HT_PLL_200M | 0x2) << 32) | (LS3A_HT_PLL_200M | 0x2)    //0
	beq	t1, a0, 8f
	nop
	daddu	a0, a0, 2
	dli	t3, ((LS7A_HT_PLL_400M | 0x2) << 32) | (LS3A_HT_PLL_400M | 0x2)    //2
	beq	t1, a0, 8f
	nop
	daddu	a0, a0, 3
	dli	t3, ((LS7A_HT_PLL_800M | 0x2) << 32) | (LS3A_HT_PLL_800M | 0x2)    //5
	beq	t1, a0, 8f
	nop
	daddu	a0, a0, 1
	dli	t3, ((LS7A_HT_PLL_1000M | 0x2) << 32) | (LS3A_HT_PLL_1000M | 0x2)   //6
	beq	t1, a0, 8f
	nop
	daddu	a0, a0, 1
	dli	t3, ((LS7A_HT_PLL_1200M | 0x2) << 32) | (LS3A_HT_PLL_1200M | 0x2)   //7
	beq	t1, a0, 8f
	nop
	daddu	a0, a0, 2
	dli	t3, ((LS7A_HT_PLL_1600M | 0x2) << 32) | (LS3A_HT_PLL_1600M | 0x2)   //9
	beq	t1, a0, 8f
	nop
	daddu	a0, a0, 2
	dli	t3, ((LS7A_HT_PLL_2000M | 0x2) << 32) | (LS3A_HT_PLL_2000M | 0x2)   //b
	beq	t1, a0, 8f
	nop
	daddu	a0, a0, 1
	dli	t3, ((LS7A_HT_PLL_2200M | 0x2) << 32) | (LS3A_HT_PLL_2200M | 0x2)   //c
	beq	t1, a0, 8f
	nop
	daddu	a0, a0, 1
	dli	t3, ((LS7A_HT_PLL_2400M | 0x2) << 32) | (LS3A_HT_PLL_2400M | 0x2)   //d
	beq	t1, a0, 8f
	nop
	daddu	a0, a0, 1
	dli	t3, ((LS7A_HT_PLL_2600M | 0x2) << 32) | (LS3A_HT_PLL_2600M | 0x2)   //e
	beq	t1, a0, 8f
	nop
	daddu	a0, a0, 1
	dli	t3, ((LS7A_HT_PLL_3200M | 0x2) << 32) | (LS3A_HT_PLL_3200M | 0x2)   //f
	beq	t1, a0, 8f
	nop
	bgt	t1, a0, 2f
	nop

	PRINTSTR("Error: freq select illegle, use default 800M.")
	dli	t3, ((LS7A_HT_PLL_800M | 0x2) << 32) | (LS3A_HT_PLL_800M | 0x2)    //5
	b	8f
	nop
2:
	move	t3, t1
8:
#endif
	dli	a0, 0x90000e0000000000
	move	a1, t2
	move	a2, t3
	bal	config_ht_link
	nop
#ifdef	LS7A_2WAY_CONNECT
	dli	a0, 0x90001e0000000000
	move	a1, t2
	move	a2, t3
	bal	config_ht_link
	nop
#endif

	//WatchDog_Enable

	move	a1, t2
	bal	reset_ht_link
	nop
	li	a0, 0xf3f3
	and	a0, a0, v0
	beqz	a0, 8f
	nop
	move	t8, v0
	TTYDBG("!!!LS3A-7A link error occur. Error status: ")
	move	a0, t8
	bal	hexserial
	nop
8:
	TTYDBG("LS3A-7A linkup.")
	/*setup LS3A - 7A HT link done.*/

	/* not close wdt until here to prevent HT reset dead */
	WatchDog_Close

/*add x route code
*finally...
*/

#if 1
#disable ht regs
	TTYDBG("\r\nDisable ht regs.\r\n")
	SET_HT_REG_DISABLE(0,0xb)
#ifdef	MULTI_CHIP
//node 1
	SET_HT_REG_DISABLE(1,0xb)
#ifdef	CHIP_4
//node 2,3
	SET_HT_REG_DISABLE(2,0xf)
	SET_HT_REG_DISABLE(3,0xf)
#endif
#endif
#endif

#if	defined(MULTI_CHIP) && defined(CHIP_4) && !defined(DISABLE_X_LINK)
	TTYDBG("\r\nEnable 4-way X-route.\r\n")
	ENABLE_XLINK(0)
	ENABLE_XLINK(1)
	ENABLE_XLINK(2)
	ENABLE_XLINK(3)
#endif
	bal	beep_off
	nop
#endif
	//WatchDog_Close

//#define	TEST_REBOOT
#ifdef	TEST_REBOOT
	bal	tgt_testchar
	nop
	bnez	v0, no_reboot
	nop
	TTYDBG("\r\nenable watchdog.")

	WatchDog_Enable
no_reboot:

#endif

//##########################################
//DDR config start
####################################
//#define ONE_CS_PER_MC
#include "ddr_dir/lsmc_ddr_param_define.h"
#include "ddr4_dir/ddr_config_define.h"
#include "ddr4_dir/ddr_config_define_v1.h"
#include "ddr4_dir/mc_vref_set.S"
//#define CLK_FLY_BY_ORDER 0x012384567   //clk go from MSB to LSB (ds8 is ecc)
//#define DDR_DLL_BYPASS
#define DISABLE_DRAM_CRC
//#define TWO_T_MODE_ENABLE
//#define DISABLE_DIMM_ECC
//#define SET_RL_MANUALY  12
#define DISABLE_READ_DBI
#define DISABLE_WRITE_DBI
#define PRINT_MSG
//#define TPHY_WR_MODE0
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
#define SELECT_SMALL_DLL
#ifdef SELECT_SMALL_DLL
#define DLL_SELECT 0xa
#endif
#if (DDR_FREQ>400)
#if (SEL_DBL == 1)
#define DCC_CTR
#endif
//#define VDDP_CTR
#define FIRST_VDDP 51
#define SECOND_VDDP 76
#endif
#define CLK2X_CKCA_SEL
#define DLL_PREDELAY_BYPASS
//#define CLK2X_DLL_BYPASS
//#define CHANGE_DIMM_WIDTH
//#define DEBUG_DDR
//#define DEBUG_DDR_PARAM
//#define DLL_DELAY_LOOP
//#define PRINT_DDR_LEVELING
//#define DLL_CK_DELAY_DEBUG
//#define NO_AUTO_TRFC  //adjust TRFC param manually if defined

	TTYDBG("\r\nStart Init Memory, wait a while......\r\n")
####################################
	move	msize, zero
	move	s3, zero
//!!!!important--s1 must be correctly set

	TTYDBG("NODE 0 MEMORY CONFIG BEGIN\r\n")


#ifdef  AUTO_DDR_CONFIG
#define N0_MC0_SLOT0_I2C_ADDR 0x0
#if !defined(CHIP_4)
#define N0_MC0_SLOT1_I2C_ADDR 0xf
#ifndef DDR3_DIMM
#define N0_MC1_SLOT0_I2C_ADDR 0x2
#else
#define N0_MC1_SLOT0_I2C_ADDR 0x1
#endif
#define N0_MC1_SLOT1_I2C_ADDR 0xf
#else
#define N0_MC0_SLOT1_I2C_ADDR 0xf
#define N0_MC1_SLOT0_I2C_ADDR 0x2
#define N0_MC1_SLOT1_I2C_ADDR 0xf
#endif
#define N0_MC0_ENABLE	0x1
#define N0_MC1_ENABLE	0x1
#define N0_NODE_ID		0x0
#define MEM_S1_VALUE(x) (N##x##_MC1_SLOT1_I2C_ADDR << 28) | \
			(N##x##_MC1_SLOT0_I2C_ADDR << 24) | \
			(N##x##_MC0_SLOT1_I2C_ADDR << 20) | \
			(N##x##_MC0_SLOT0_I2C_ADDR << 16) | \
			(N##x##_MC1_ENABLE         << 9 ) | \
			(N##x##_MC0_ENABLE         << 8 ) | \
			(N##x##_NODE_ID)
	dli	s1, MEM_S1_VALUE(0) 
#ifdef  CHANGE_DIMM_WIDTH
#define DIMM_WIDTH		0x3 /*DIMM width 16bit-1, 32bit-2, 64bit-3*/
	ori	s1, (DIMM_WIDTH<<4)
#endif
#else
	dli	s1, S1_VALUE | 0x0//MC0
	dli	s3, S3_VALUE | 0x0//MC1,TODO, maybe demage by ddr3_leveling.S and ddr4 vref_training.S
#endif
#ifndef DDR3_DIMM
#define VREF 0x0801
#else
#define VREF 0x0c01
#endif
	VREF_SET(VREF)

#if	defined(ENABLE_MC_VREF_TRAINING) && defined(VREF_STORE)
	PRINTSTR("\r\nLock Scache\r\n")
	dli	a2, LOCK_SCACHE_CONFIG_BASE_ADDR
	dli	a3, 0xfffffffffffff000
	sd	a3, 0x48(a2)
	dli	a3, 0x8000000010000000
	sd	a3, 0x8(a2)
	sync
	PRINTSTR("Lock Scache Done.\r\n")
#endif

#include "ddr_dir/loongson3A4000_ddr4_config.S"
	//TEST_MEMORY(0);

#ifdef	MULTI_CHIP
	TTYDBG("\r\nNODE 1 MEMORY CONFIG BEGIN\r\n")

#ifdef  AUTO_DDR_CONFIG
#if !defined(CHIP_4)
#define N1_MC0_SLOT0_I2C_ADDR 0x4
#define N1_MC0_SLOT1_I2C_ADDR 0x5
#define N1_MC1_SLOT0_I2C_ADDR 0x6
#define N1_MC1_SLOT1_I2C_ADDR 0x7
#else
#define N1_MC0_SLOT0_I2C_ADDR 0x0
#define N1_MC0_SLOT1_I2C_ADDR 0xf
#define N1_MC1_SLOT0_I2C_ADDR 0x2
#define N1_MC1_SLOT1_I2C_ADDR 0xf
#endif
#define N1_MC0_ENABLE		0x1
#define N1_MC1_ENABLE		0x1
#define N1_NODE_ID		0x1
	dli	s1, MEM_S1_VALUE(1) 
#ifdef  CHANGE_DIMM_WIDTH
#define N1_DIMM_WIDTH		0x3 /*DIMM width 16bit-1, 32bit-2, 64bit-3*/
	ori	s1, (N1_DIMM_WIDTH<<4)
#endif
#else
	dli	s1, S1_VALUE | 0x1//MC0
	dli	s3, S3_VALUE | 0x1//MC1,TODO, maybe demage by ddr3_leveling.S and ddr4 vref_training.S
#endif

	VREF_SET(VREF)
#include "ddr_dir/loongson3A4000_ddr4_config.S"
	//TEST_MEMORY(1);

#ifdef CHIP_4
	TTYDBG("\r\nNODE 2 MEMORY CONFIG BEGIN\r\n")

#ifdef  AUTO_DDR_CONFIG
#define N2_MC0_SLOT0_I2C_ADDR 0x0
#define N2_MC0_SLOT1_I2C_ADDR 0xf
#define N2_MC1_SLOT0_I2C_ADDR 0x2
#define N2_MC1_SLOT1_I2C_ADDR 0xf
#define N2_MC0_ENABLE		0x1
#define N2_MC1_ENABLE		0x1
#define N2_NODE_ID		0x2
	dli	s1, MEM_S1_VALUE(2) 
#ifdef  CHANGE_DIMM_WIDTH
#define N2_DIMM_WIDTH		0x3 /*DIMM width 16bit-1, 32bit-2, 64bit-3*/
	ori	s1, (N2_DIMM_WIDTH<<4)
#endif
#else
	dli	s1, S1_VALUE | 0x2//MC0
	dli	s3, S3_VALUE | 0x2//MC1,TODO, maybe demage by ddr3_leveling.S and ddr4 vref_training.S
#endif

	VREF_SET(VREF)
#include "ddr_dir/loongson3A4000_ddr4_config.S"
	//TEST_MEMORY(2);

	TTYDBG("\r\nNODE 3 MEMORY CONFIG BEGIN\r\n")

#ifdef  AUTO_DDR_CONFIG
#define N3_MC0_SLOT0_I2C_ADDR 0x0
#define N3_MC0_SLOT1_I2C_ADDR 0xf
#define N3_MC1_SLOT0_I2C_ADDR 0x2
#define N3_MC1_SLOT1_I2C_ADDR 0xf
#define N3_MC0_ENABLE		0x1
#define N3_MC1_ENABLE		0x1
#define N3_NODE_ID		0x3
	dli	s1, MEM_S1_VALUE(3) 
#ifdef  CHANGE_DIMM_WIDTH
#define N3_DIMM_WIDTH		0x3 /*DIMM width 16bit-1, 32bit-2, 64bit-3*/
	ori	s1, (N3_DIMM_WIDTH<<4)
#endif
#else
	dli	s1, S1_VALUE | 0x3//MC0
	dli	s3, S3_VALUE | 0x3//MC1,TODO, maybe demage by ddr3_leveling.S and ddr4 vref_training.S
#endif
	VREF_SET(VREF)
#include "ddr_dir/loongson3A4000_ddr4_config.S"
	//TEST_MEMORY(3)
#endif
#endif
	TTYDBG("Init Memory done.\r\n")

#if	defined(ENABLE_MC_VREF_TRAINING) && defined(VREF_STORE)
//copy spd info
	li	a1, 0x1
#ifdef	MULTI_CHIP
	li	a1, 0x2
#ifdef CHIP_4
	li	a1, 0x4
#endif
#endif
	mul	a1, DIMM_INFO_SIZE
	dli	t0, DIMM_INFO_IN_CACHE_OFFS
	dli	t1, (0xffffffff00000000 | DIMM_INFO_IN_SDRAM_OFFS)
	daddu	t2, t1, a1
		
1:
	ld	a3, 0x0(t0)
	sd	a3, 0x0(t1)
	daddiu	t0, 0x8
	daddiu	t1, 0x8
	blt	t1, t2, 1b
	nop

	PRINTSTR("\r\nUnlock Scache Node\r\n")
	dli	a2, LOCK_SCACHE_CONFIG_BASE_ADDR
	sd	zero, 0x8(a2)
	sd	zero, 0x48(a2)
	sync

//invalid scache
	//dli	a0, 0x9800000010000000
	dli	a0, 0xffffffff90000000
	li	a1, 0x1000
	//bal	CPU_HitInvalidateSCache
	bal	LS3A4000_HitInvalidateSCache
	nop
#endif

//#define TEST_MEM
#define REBOOT_ERROR_FOUND
#ifdef TEST_MEM
	//test node 0 memory
	li	a0, 0
	bal	test_node_mem
	nop
#ifdef MULTI_CHIP
	//test node 1 memory
	li	a0, 1
	bal	test_node_mem
	nop
#ifdef CHIP_4
	//test node 2 memory
	li	a0, 2
	bal	test_node_mem
	nop
	//test node 3 memory
	li	a0, 3
	bal	test_node_mem
	nop
#endif
#endif
#endif
##########################################
#ifdef  DEBUG_DDR
#if 1
	PRINTSTR("\r\nDo test?(0xf: skip): ")
	bal	inputaddress
	nop
	and	v0, v0, 0xf
	dli	a1, 0x1
	bgt	v0, a1, 3f
	nop
#endif

	dli	s1, 0x0010000090000000
#if 1
	PRINTSTR("\r\ndefault s1 = 0x");
	dsrl	a0, s1, 32
	bal	hexserial
	nop
	PRINTSTR("__")
	move	a0, s1
	bal	hexserial
	nop
	PRINTSTR("\r\nChange test param s1(0: skip)?: ")
	bal	inputaddress
	nop
	beqz	v0, 1f
	nop
	move	s1, v0
1:
#endif
1:
	dli	t1, 0x0010
	bal	test_mem
	nop
	move	t1, v0
	PRINTSTR("\r\n")
	dsrl	a0, t1, 32
	bal	hexserial
	nop
	move	a0, t1
	bal	hexserial
	nop
	beqz	t1, 2f
	nop
	PRINTSTR("  Error found!!\r\n")
2:
#if 0
	b	 1b
	nop
#endif

3:
#endif


#ifdef  AUTO_ARB_LEVEL
#include "ddr_dir/store_auto_arb_level_info.S"
#endif

#ifdef LOCK_SCACHE
	bal lock_scache
	nop
	TTYDBG("cache lock done\r\n")
	nop
#endif
##########################################

//Initialize LS7A here
#if 1
	TTYDBG("\r\nbridge CHIP ID: 0x")
	dli	t0, LS7A_CONFBUS_BASE_ADDR
	lw	a0, 0x3ff8(t0)
	bal	hexserial
	nop
	TTYDBG("revision: 0x")
	lw	a0, 0x3ffc(t0)
	srl	a0, a0, 24
	bal	hexserial
	nop
	TTYDBG("\r\n")

	//check chip ID
	lw	a0, 0x3ff8(t0)
	li	a1, 0x7A000000
	xor	a0, a0, a1
	srl	a0, a0, 24  //check 7A only
	beqz	a0, 2f
	nop
	bal	hexserial
	nop
	TTYDBG("\r\nbridge CHIP ID check failed!!!")
1:
	b	 1b
	nop
2:
#endif

//HT1 window and configurations
	dli	a0, 0x90000e0000000000
	bal	ls3a7a_ht_init
	nop
	TTYDBG("Node 0 LS3A-7A init done.\r\n")
#ifdef  LS7A_2WAY_CONNECT
	dli	a0, 0x90001e0000000000
	bal	ls3a7a_ht_init
	nop
	TTYDBG("Node 1 LS3A-7A init done.\r\n")
#endif
#if 1
#include "../../../pmon/arch/mips/ls7a/ls7a_dbg.S"
#endif
#include "../../../pmon/arch/mips/ls7a/ls7a_init.S"
	TTYDBG("\r\nLS7A init done.\r\n")

##########################################
#if 0
	dli	s1, 0x0000000000000000
#include "loongson3_debug_window.S"

#ifdef MULTI_CHIP
	dli	s1, 0x0000100000000000
#include "loongson3_debug_window.S"
#endif
#endif
###########################################

#include "machine/newtest/newdebug.S"

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

	/*clken_percore enable*/
	li	a0, 0xbfe00420
	lw	a1, 0x0(a0)
	or	a1, (1 << 23)
	sw	a1, 0x0(a0)

#ifdef MULTI_CHIP
	/*node 1 clken_percore enable*/
	dli	a0, 0x900010001fe00420
	lw	a1, 0x0(a0)
	or	a1, (1 << 23)
	sw	a1, 0x0(a0)
#ifdef CHIP_4
	/*node 2 clken_percore enable*/
	dli	a0, 0x900020001fe00420
	lw	a1, 0x0(a0)
	or	a1, (1 << 23)
	sw	a1, 0x0(a0)

	/*node 3 clken_percore enable*/
	dli	a0, 0x900030001fe00420
	lw	a1, 0x0(a0)
	or	a1, (1 << 23)
	sw	a1, 0x0(a0)
#endif
#endif

#ifdef SHUT_SLAVES
	PRINTSTR("Wake up other cores\r\n")

	li	a0, 0xbfe001d0
	li	t1, 0xffffffff
	sw	t1, 0x0(a0)

#ifdef MULTI_CHIP
	TTYDBG("Enable CPU1\r\n")
	dli	a0, 0x900010001fe001d0
	li	a1, 0xffffffff
	sw	a1, 0x0(a0)
#ifdef CHIP_4
	TTYDBG("Enable CPU2\r\n")
	dli	a0, 0x900020001fe001d0
	li	a1, 0xffffffff
	sw	a1, 0x0(a0)

	TTYDBG("Enable CPU3\r\n")
	dli	a0, 0x900030001fe001d0
	li	a1, 0xffffffff
	sw	a1, 0x0(a0)
#endif
#endif
#else
	PRINTSTR("NOT Wake up other cores\r\n")

#endif


	TTYDBG("sp=");
	move	a0, sp
	bal	hexserial
	nop

	li	a0, 4096*1024
	sw	a0, CpuTertiaryCacheSize /* Set L3 cache size */ //useless this code

	PRINTSTR("\r\n")

	/* pass pointer to kseg1 tgt_putchar */
	la	a1, tgt_putchar
	daddu	a1, a1, s0

	la	a2, stringserial
	daddu	a2, a2, s0

	move	a0,msize

	dli	t0, NODE0_CORE0_BUF0  #buf of cpu0 we need bootcore_id
	dli	t3, BOOTCORE_ID
	dsll	t3, 8
	or	t0, t0, t3
	li	t1, SYSTEM_INIT_OK
	sw	t1, FN_OFF(t0)
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
	mtc0	a2, COP_0_TLB_PG_MASK   # Whatever...

1:
	mtc0	zero, COP_0_TLB_HI	# Clear entry high.
	mtc0	zero, COP_0_TLB_LO0	# Clear entry low0.
	mtc0	zero, COP_0_TLB_LO1	# Clear entry low1.

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
	daddu	a2, (0x01000000 >> PG_SHIFT)
	MTC0	a2, COP_0_TLB_LO1	# Set up entry low1.

	mtc0	a3, COP_0_TLB_INDEX	# Set the index.
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

/*
 * input: a0
 * a0 is node number
 * k0 store ra
 */
LEAF(test_node_mem)
	move	k0, ra

	move	t0, msize
	li	t1, 0xff
	sll	a0, 3
	sll	t1, a0
	and	t0, t1
	beqz	t0, 2f
	nop
	dli	s1, 0x0010000090000000
	dsll	t1, a0, 59
	dli	t1, 0x0010
	bal	test_mem
	nop
	move	t1, v0
	PRINTSTR("\r\n")
	dsrl	a0, t1, 32
	bal	hexserial
	nop
	move	a0, t1
	bal	hexserial
	nop
	beqz	t1, 2f
	nop
	PRINTSTR("  Error found!!\r\n")

//	bal	beep_on
	nop
1:
#ifdef	REBOOT_ERROR_FOUND
	li	t0, 0xb00d0030
	li	t1, 1
	sw	t1, 0x0(t0)
#endif
	b	1b
	nop
2:
	move	ra, k0
	jr	ra
	nop
END(test_node_mem)

END(CPU_TLBInit)

LEAF(stringserial)
	move	a2, ra
#ifdef	ROM_EXCEPTION
	li	a1,0x3ec00000
	daddu	a1, a0, a1
#else
	daddu	a1, a0, s0
#endif
	lbu	a0, 0(a1)
1:
	beqz	a0, 2f
	nop
	bal	tgt_putchar
//	nop
//	bal	tgt_putchar1
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
#ifdef	ROM_EXCEPTION
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
	move	v1, v0
	la	v0, GS3_UART_BASE
	bne	v0, v1, 1b
	nop

	jr	ra
	nop	
END(tgt_putchar)

LEAF(tgt_putchar1)
	la	v0,GS3_UART1_BASE
1:
	lbu	v1, NSREG(NS16550_LSR)(v0)
	and	v1, LSR_TXRDY
	beqz	v1, 1b
	nop

	sb	a0, NSREG(NS16550_DATA)(v0)
	move	v1, v0
	la	v0, GS3_UART1_BASE
	bne	v0, v1, 1b
	nop

	jr	 ra
	nop
END(tgt_putchar1)

LEAF(beep_off)
	nop
	//set GPIO0 output 1 to close beep
	dli	t0, (LS7A_MISC_BASE_ADDR | GPIO_BASE_ADDR_OFFSET)
	li	t1, 1
	sb	t1, 0x900(t0)
	jr	ra
	nop
END(beep_off)

LEAF(beep_on)
	nop
	//set GPIO0 output 0 to open beep
	dli	t0, (LS7A_MISC_BASE_ADDR | GPIO_BASE_ADDR_OFFSET)
	sb	zero, 0x900(t0)
	jr	ra
	nop
END(beep_on)

/* baud rate definitions, matching include/termios.h */
#define B0	0
#define B50	50
#define B75	75
#define B110	110
#define B134	134
#define B150	150
#define B200	200
#define B300	300
#define B600	600
#define B1200	1200
#define B1800	1800
#define B2400	2400
#define B4800	4800
#define B9600	9600
#define B19200	19200
#define B38400	38400
#define B57600	57600
#define B115200	115200

LEAF(initserial)
//call this function must give the register addr to a0
	li	t1,128
	sb	t1,3(a0)
#ifdef	BONITO_33M
	li	t1,0x12	# divider, highest possible baud rate,for 33M crystal
#elif	BONITO_25M
	li	t1,0x0d	# divider, highest possible baud rate,for 25M crystal
#elif	BONITO_50M
	li	t1,0x1b	# divider, highest possible baud rate,for 50M crystal
#else	BONITO_100M //as a default
	li	t1,0x36	# divider, highest possible baud rate,for 100M crystal
#endif
	sb	t1,0(a0)
	li	t1,0x0	# divider, highest possible baud rate
	sb	t1,1(a0)
	li	t1,3
	sb	t1,3(a0)

	#srl	t1,t1,0x8
	li	t1,0
	sb	t1,1(a0)
	#li	t1,1	# divider, highest possible baud rate


	li	t1,71
	sb	t1,2(a0)
	jr	ra
	nop
END(initserial)

#include "i2c_ls.S"
#ifdef AUTO_DDR_CONFIG
#include "ddr4_dir/detect_channel_dimm.S"
#endif

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

LEAF(nullfunction)
	jr	ra
	nop
END(nullfunction)

###############################
LEAF(hexserial64)
	move	t7,ra
	move	t6,a0
	dsrl	a0,32
	bal	hexserial
	nop
	move	a0,t6
	bal	hexserial
	nop
	move	ra, t7
	jr	ra
	nop
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
	#dsrl	t2, 30		  /* for 3b/3c */
	#or	t1, t2, t1
	dli	t2, NODE0_CORE0_BUF0
	or	t1, t1, t2
	sd	zero, FN_OFF(t1)
	sd	zero, SP_OFF(t1)
	sd	zero, GP_OFF(t1)
	sd	zero, A1_OFF(t1)

	jr	ra
	nop
END(clear_mailbox)

LEAF(lock_scache)
#if 1
	dli	t0, 0x900010003ff04000
	dli	t1, 0xffffffffffe00000
	sd	t1, 0x240(t0)
	dli	t1, 0x8000100090000000
	sd	t1, 0x200(t0)
#if 0	//1M
	dli	t0, 0x900010003ff04000
	dli	t1, 0xfffffffffff00000
	sd	t1, 0x248(t0)
	dli	t1, 0x8000100090200000
	sd	t1, 0x208(t0)
#endif
	dli	t0, 0x900000003ff00000
	dli	t1, 0xffffffffffe00000
	sd	t1, 0x240(t0)
	dli	t1, 0x8000000090000000
	sd	t1, 0x200(t0)
#if 0	//1M 
	dli	t0, 0x900000003ff00000
	dli	t1, 0xfffffffffff00000
	sd	t1, 0x248(t0)
	dli	t1, 0x8000000090200000
	sd	t1, 0x208(t0)
#endif
	jr	ra
	nop
#endif
END(lock_scache)

#if	defined(ENABLE_MC_VREF_TRAINING) && defined(VREF_STORE)
LEAF(LS3A4000_HitInvalidateSCache)
	mfc0	v1, COP_0_STATUS_REG		# Save the status register.
	li	v0, SR_DIAG_DE
	mtc0	v0, COP_0_STATUS_REG		# Disable interrupts

	beq	a1, zero, 3f			# size is zero!
	addu	a1, 127				# Round up
	addu	a1, a1, a0			# Add in extra from align
	and	a0, a0, -128			# Align address
	subu	a1, a1, a0
	srl	a1, a1, 7			# Compute number of cache lines
1:
	addu	a1, -1
	
	cache	0x13, 0(a0)
	cache	0x13, 32(a0)
	cache	0x13, 64(a0)
	cache	0x13, 96(a0)

	bne	a1, zero, 1b
	addu	a0, 128

3:
	mtc0	v1, COP_0_STATUS_REG	# Restore the status register.
	nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;
	j	ra
	nop
END(LS3A4000_HitInvalidateSCache)
#endif

#ifdef	LS132_CORE
#include "ls132_core.S"
#include "ls132_i2c.S"
#endif
#include "ls3a4000_vctrl.S"
	.ent slave_main
slave_main:

	dli	t2, NODE0_CORE0_BUF0
	dli	t3, BOOTCORE_ID
	dsll	t3, 8
	or	t2, t2, t3

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
	lui	t6, 0xdfff 
	ori	t6, t6, 0xffff
	bal	1f
	nop
1:
	and	ra, ra, t6
	daddiu	ra, ra, 16
	jr	ra
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

	li	a0, 0x1000
idle1000:
	addiu	a0, -1
	bnez	a0, idle1000
	nop

	lw	t2, FN_OFF(t1)
	beqz	t2, waitforinit
	nop

	dli	t3, 0xffffffff00000000 
	or	t2, t3

	dli	t3, 0x9800000000000000 
	ld	sp, SP_OFF(t1)
	or	sp, t3
	ld	gp, GP_OFF(t1)
	or	gp, t3
	ld	a1, A1_OFF(t1)

	move	ra, t2
	jr	ra  # slave core jump to kernel, byebye
	nop

	.end	slave_main


#######################################
#include "../../../pmon/arch/mips/ls7a/ls3a7a_setup_ht_link.S"
#include "../../../pmon/arch/mips/ls7a/ls3a7a_ht_init.S"
#include "../../../pmon/arch/mips/ls7a/ls7a_config.S"
#include "ddr_dir/loongson3A4000_mc_init.S"
#ifdef  LS7A_GMEM_CFG
#include "ddr_dir/ls7A_gmem_config.S"
#endif
//#ifdef DDR3_DIMM
#include "../../../pmon/arch/mips/mm/loongson3C_ddr3_leveling.S"
//#else
#include "ddr4_dir/ddr4_leveling.S"
#include "ddr4_dir/mc_vref_training.S"
#include "ddr4_dir/mc_config.S"
//#endif
#ifdef ARB_LEVEL
#include "ddr_dir/ARB_level_new.S"
#endif
#if 1	// (defined(DEBUG_DDR) || defined(DEBUG_GMEM))
#include "ddr_dir/Test_Mem.S"
#endif
#include "ddr4_dir/test_engin.S"
#include "ddr4_dir/wr_bit_training.S"
#include "ddr4_dir/rd_bit_training.S"
#include "ddr4_dir/ddr_vref_training.S"

	.global	watchdog_enable
	.ent	watchdog_enable
	.set	noreorder
	.set	mips3
watchdog_enable:
	WatchDog_Enable
	jr		ra
	nop
	.end watchdog_enable

	.text
	.global  nvram_offs
	.align 12
nvram_offs:
	.dword 0x0
	.align 12

#######################################

	.rdata
	.global	ddr2_reg_data
	.global	ddr3_reg_data
	.global	gmem_reg_data

	.align  5
//#ifndef DDR3_DIMM
//#include "ddr4_dir/ddr4_register_param.S"
//#else
#include "loongson_mc_param.S"
//#endif
#ifdef  LS7A_GMEM_CFG
#include "loongson7A_gmem_param.S"
#endif
