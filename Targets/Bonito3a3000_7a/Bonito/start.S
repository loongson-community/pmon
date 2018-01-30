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
	//lui	t1, 0x8000
	or	t0, t1, t0
	mtc0	t0, $22
	.set	pop

	mtc0	zero, COP_0_STATUS_REG
	mtc0	zero, COP_0_CAUSE_REG
	li	t0, SR_BOOT_EXC_VEC	/* Exception to Boostrap Location */
	mtc0	t0, COP_0_STATUS_REG
	la	sp, stack
	la	gp, _gp


	WatchDog_Close


	/* spi speedup */
	li  t0, 0xbfe00220
	li  t1, 0x07
	sb  t1, 0x4(t0)

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

	li      a0, GS3_UART_BASE
    bal     initserial
    nop
	li      a0, GS3_UART1_BASE
    bal     initserial
    nop

//#define SHUT_SLAVES
#ifdef SHUT_SLAVES
	PRINTSTR("Shut down slave cores\r\n")
	li      a0, 0xbfe001d0
	li	a1, BOOTCORE_ID
	sll	a1, 2
	li      t1, 0xf
	sll	a1, t1, a1
	li	t1, 0x88888888
	or	t1, a1, t1
	sw      t1, 0x0(a0)
	li      t1, 0x00000000
	or	t1, a1, t1
	sw      t1, 0x0(a0)
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
	//Read sys_clk_sel
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
	TTYDBG ("\r\nMEM CLK SEL : ")
	dsrl t0, t1, 5
	andi a0, t0, 0x1f
	bal hexserial
	nop
	TTYDBG ("\r\nHT CLK SEL : ")
	dsrl t0, t1, 10
	andi a0, t0, 0x3f
	bal hexserial
	nop
	TTYDBG ("\r\n")

//USING S1 FOR PASSING THE NODE ID
	dli	s1, 0X0000000000000000
#include "loongson3_clksetting.S"

##########################################

#include "loongson3_fixup.S"

#ifdef MULTI_CHIP
//USING S1 FOR PASSING THE NODE ID
	dli	s1, 0x0000100000000000
#include "loongson3_clksetting.S"
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
	lui     t0, 0xdfff 
	ori     t0, t0, 0xffff
	bal     1f
	nop
1:
	and     ra, ra, t0
	addiu   ra, ra, 16
	jr      ra
	nop

    TTYDBG("32 bit PCI space translate to 64 bit HT space\r\n")
#include "loongson3_ht1_32addr_trans.S"

    //config fix address bar for Misc devices block
    dli     t0, MISC_HEADER_ADDR
    li      t1, MISC_BASE_ADDR
    sw      t1, 0x10(t0)
    lw      t2, 0x4(t0)
    ori     t2, t2, 0x2
    sw      t2, 0x4(t0)
    //change confbus base address
    dli     t0, CONFBUS_HEADER_ADDR
    li      t1, CONFBUS_BASE_ADDR
    sw      t1, 0x10(t0)
    TTYDBG("set LS7A MISC and confbus base address done.\r\n")

    //set PWM output 1
    dli     t0, (LS7A_MISC_BASE_ADDR | PWM_BASE_ADDR_OFFSET)
    li      a0, 0x100
    li      a1, (1<<0)
    sw      $0, 0x4(t0)
    sw      a0, 0x8(t0)
    sw      a1, 0xc(t0)
    sw      $0, 0x104(t0)
    sw      a0, 0x108(t0)
    sw      a1, 0x10c(t0)
    sw      $0, 0x204(t0)
    sw      a0, 0x208(t0)
    sw      a1, 0x20c(t0)
    sw      $0, 0x304(t0)
    sw      a0, 0x308(t0)
    sw      a1, 0x30c(t0)

    bal  beep_off
    nop

//setup LS3A - 7A HT link start...
    //check 3A clksel setting
    li      t0, 0xbfe00190
    lw      a0, 0x4(t0)
    srl     a0, a0, 15
    beqz    a0, 3f
    nop
#ifdef  CHECK_HT_PLL_MODE
    TTYDBG("Warning: 3A HT in hard freq mode, please modify clksel[7].\r\n")
    dli     a0, 0x4000000
1:
    dsub    a0, a0, 1
    bnez    a0, 1b
    nop
#endif
    b       2f
    nop
3:
    TTYDBG("3A HT in soft freq cfg mode...ok\r\n")
2:

    //check 7A clksel setting
    dli     t0, (LS7A_MISC_BASE_ADDR + 0x60000)
    lb      a0, (0xa00+53)(t0)
    beqz    a0, 3f
    nop
#ifdef  CHECK_HT_PLL_MODE
    TTYDBG("Warning: 7A HT in hard freq mode, please modify clksel[7].\r\n")
    dli     a0, 0x4000000
1:
    dsub    a0, a0, 1
    bnez    a0, 1b
    nop
#endif
    b       2f
    nop
3:
    TTYDBG("7A HT in soft freq cfg mode...ok\r\n")
2:

    li      t2, ((HT1_HARD_FREQ_CFG << 12) | (HT1_HARD_FREQ_CFG << 8) | (HT1_GEN_CFG << 4) | (HT1_WIDTH_CFG << 1) | (HT1_RECONNECT << 0))

    li      t8, LS7A_HT1_SOFT_FREQ_CFG
    dsll    t3, t8, 32
    li      t8, LS3A_HT1_SOFT_FREQ_CFG
    or      t3, t3, t8

#ifdef  DEBUG_HT1
    PRINTSTR("HT1 default setting: \r\na1: 0x")
    move    a0, t2
    bal     hexserial
    nop
    PRINTSTR("\r\na2: 0x")
    dsrl    a0, t3, 32
    bal     hexserial
    nop
    move    a0, t3
    bal     hexserial
    nop
    PRINTSTR("\r\nInput parameter a1: ([15:12]: 7A freq-0/2/5/9; [11:8]: 3A freq-0/2/5/9; [7:4]: GENx-1/3; [1]: width-0/1; [0]: reconnect-0/1): ")
    bal     inputaddress
    nop
    beqz    v0, 1f
    nop
    move    t2, v0
1:
    PRINTSTR("\r\nInput parameter a2: ([3:0]: ht pll soft cfg sel. 0: 200M; 5: 800M; 6: 1000M; 7: 1200M; 9: 1600M; b: 2000M; c: 2200M; d: 2400M; e: 2600M; f: 3200M;): ")
    bal     inputaddress
    nop
    move    t1, v0
    PRINTSTR("\r\n")

    move    a0, $0
    dli     t3, ((LS7A_HT_PLL_200M | 0x2) << 32) | (LS3A_HT_PLL_200M | 0x2)    //0
    beq     t1, a0, 8f
    nop
    daddu   a0, a0, 5
    dli     t3, ((LS7A_HT_PLL_800M | 0x2) << 32) | (LS3A_HT_PLL_800M | 0x2)    //5
    beq     t1, a0, 8f
    nop
    daddu   a0, a0, 1
    dli     t3, ((LS7A_HT_PLL_1000M | 0x2) << 32) | (LS3A_HT_PLL_1000M | 0x2)   //6
    beq     t1, a0, 8f
    nop
    daddu   a0, a0, 1
    dli     t3, ((LS7A_HT_PLL_1200M | 0x2) << 32) | (LS3A_HT_PLL_1200M | 0x2)   //7
    beq     t1, a0, 8f
    nop
    daddu   a0, a0, 2
    dli     t3, ((LS7A_HT_PLL_1600M | 0x2) << 32) | (LS3A_HT_PLL_1600M | 0x2)   //9
    beq     t1, a0, 8f
    nop
    daddu   a0, a0, 2
    dli     t3, ((LS7A_HT_PLL_2000M | 0x2) << 32) | (LS3A_HT_PLL_2000M | 0x2)   //b
    beq     t1, a0, 8f
    nop
    daddu   a0, a0, 1
    dli     t3, ((LS7A_HT_PLL_2200M | 0x2) << 32) | (LS3A_HT_PLL_2200M | 0x2)   //c
    beq     t1, a0, 8f
    nop
    daddu   a0, a0, 1
    dli     t3, ((LS7A_HT_PLL_2400M | 0x2) << 32) | (LS3A_HT_PLL_2400M | 0x2)   //d
    beq     t1, a0, 8f
    nop
    daddu   a0, a0, 1
    dli     t3, ((LS7A_HT_PLL_2600M | 0x2) << 32) | (LS3A_HT_PLL_2600M | 0x2)   //e
    beq     t1, a0, 8f
    nop
    daddu   a0, a0, 1
    dli     t3, ((LS7A_HT_PLL_3200M | 0x2) << 32) | (LS3A_HT_PLL_3200M | 0x2)   //f
    beq     t1, a0, 8f
    nop
    bgt     t1, a0, 2f
    nop

    PRINTSTR("Error: freq select illegle, use default 800M.")
    dli     t3, ((LS7A_HT_PLL_800M | 0x2) << 32) | (LS3A_HT_PLL_800M | 0x2)    //5
    b       8f
    nop
2:
    move    t3, t1
8:
#endif
    dli     a0, 0x90000e0000000000
    move    a1, t2
    move    a2, t3
    bal     config_ht_link
    nop
#ifdef  LS7A_2WAY_CONNECT
    dli     a0, 0x90001e0000000000
    move    a1, t2
    move    a2, t3
    bal     config_ht_link
    nop
#endif

    move    a1, t2
    bal     reset_ht_link
    nop
    li      a0, 0x0303
    and     a0, a0, v0
    beqz    a0, 8f
    nop
    move    t8, v0
    TTYDBG("!!!LS3A-7A link fail. fail status: ")
    move    a0, t8
    bal     hexserial
    nop
1:
    b       1b
    nop
8:
    TTYDBG("LS3A-7A linkup.")
//setup LS3A - 7A HT link done.

//#define TEST_REBOOT
#ifdef TEST_REBOOT
	bal	tgt_testchar
	nop
	bnez	v0, no_reboot
	nop

	WatchDog_Enable

no_reboot:

#endif

//##########################################
//DDR config start
//cxk
####################################
#include "ddr_dir/lsmc_ddr_param_define.h"
#include "ddr_dir/ddr_config_define.h"
//#define DDR_DLL_BYPASS
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
//#define DISABLE_DDR_A15
//#define DEBUG_DDR
//#define DEBUG_DDR_PARAM


    TTYDBG("\r\nStart Init Memory, wait a while......\r\n")
####################################
    move    msize, $0
    move    s3, $0
//!!!!important--s1 must be correctly set

    TTYDBG("NODE 0 MEMORY CONFIG BEGIN\r\n")

#ifdef  AUTO_DDR_CONFIG
    dli     s1, 0xf1f00000  //set use MC1 or MC0 or MC1/0 and give All device id
#else
    	dli     s1, 0xf0a31000f0a31004  // use both, 8G SCS RDIMM
#endif
//#include "ddr_dir/loongson3_ddr2_config.S"
#include "ddr_dir/loongson3A2000_ddr2_config.S"

/* test memory */
        dli     t0, 0x9000000000000000
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

#ifdef	MULTI_CHIP
    TTYDBG("NODE 1 MEMORY CONFIG BEGIN\r\n")

#ifdef  AUTO_DDR_CONFIG
    dli     s1, 0xf3f20001  //set use MC1 or MC0 or MC1/0 and give All device id
#else
//	dli     s1, 0xc2e30400c2e30405
   	dli     s1, 0xf0a31000f0a31001  // use both, 8G SCS RDIMM
#endif
#include "ddr_dir/loongson3A2000_ddr2_config.S"


/* test memory */
//TEST if NODE1 has a memory
        dli     t0, 0x9000100000000008
        lw      a0, 0x0(t0)
        li      a1, 0x10000000
        beq     a0, a1, 11f
        nop

        dli     t0, 0x9000100000000000
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
	dli     t5, 0x9000100000000000
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
	dli     t5, 0x9800100000000000
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

11:
#endif

	/*judge the node1 whether have memory*/
	and     a0, msize, 0xff
//	beqz    a0, beep_on
//	nop


    TTYDBG("Init Memory done.\r\n")

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

#ifdef LOCK_SCACHE
	bal lock_scache
	nop
	TTYDBG("cache lock done\r\n")
	nop
#endif
##########################################

//Initialize LS7A here, cxk
#if 1
    //check chip ID
    dli     t0, LS7A_CONFBUS_BASE_ADDR
    lw      a0, 0x3ff8(t0)
    li      a1, 0x7A000000
    xor     a0, a0, a1
    srl     a0, a0, 24  //check 7A only
    beqz    a0, 2f
    nop
    bal     hexserial
    nop
    TTYDBG("\r\nbridge CHIP ID check failed!!!")
1:
    b       1b
    nop
2:
#endif

//HT1 window and configurations
    dli     a0, 0x90000e0000000000
    bal     ls3a7a_ht_init
    nop
    TTYDBG("Node 0 LS3A-7A init done.\r\n")
#ifdef  LS7A_2WAY_CONNECT
    dli     a0, 0x90001e0000000000
    bal     ls3a7a_ht_init
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
    dli     s1, 0x0000000000000000
#include "loongson3_debug_window.S"

#ifdef MULTI_CHIP
    dli     s1, 0x0000100000000000
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

#ifdef SHUT_SLAVES
	PRINTSTR("Wake up other cores\r\n")

        /* Set clock low for a safe shift */
        li      t0, 0xbfe001b0
        lw      a0, 0x0(t0)
        li      a1, 0xfffffffe
        and     a0, a0, a1
        sw      a0, 0x0(t0)

	li      a0, 0xbfe001d0
	li	a1, BOOTCORE_ID
	sll	a1, 2
	li	t1, 0xf
	sll	a1, t1, a1
	li      t1, 0x88888888
	or	t1, a1, t1
	sw      t1, 0x0(a0)

        li      t0, 0xbfe001b0
        lw      a0, 0x0(t0)
        ori     a0, a0, 0x1
        sw      a0, 0x0(t0)

	li      a0, 0xbfe001d0
	li      t1, 0xffffffff
	sw      t1, 0x0(a0)
#else
	PRINTSTR("NOT Wake up other cores\r\n")

#endif


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
#ifdef ROM_EXCEPTION
	li a1,0x3ec00000
	daddu	a1, a0, a1
#else
	daddu	a1, a0, s0
#endif
	lbu	a0, 0(a1)
1:
	beqz	a0, 2f
	nop
	bal     tgt_putchar
	nop
	bal     tgt_putchar1
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
	bal     tgt_putchar
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
	move	v1, v0
	la	v0, GS3_UART_BASE
	bne	v0, v1, 1b
	nop

	jr	ra
	nop	
END(tgt_putchar)

LEAF(tgt_putchar1)
	la      v0,GS3_UART1_BASE
1:
	lbu     v1, NSREG(NS16550_LSR)(v0)
	and     v1, LSR_TXRDY
	beqz    v1, 1b
	nop

	sb      a0, NSREG(NS16550_DATA)(v0)
	move    v1, v0
	la      v0, GS3_UART1_BASE
	bne     v0, v1, 1b
	nop

	jr       ra
	nop
END(tgt_putchar1)

LEAF(beep_on)
	nop
    //set GPIO0 output 1 to open beep
    dli t0, (LS7A_MISC_BASE_ADDR | GPIO_BASE_ADDR_OFFSET)
    li  t1,1
    sb  t1, 0x900(t0)
	jr	ra
	nop
END(beep_on)

LEAF(beep_off)
	nop
    //set GPIO0 output 0 to close beep
    dli t0, (LS7A_MISC_BASE_ADDR | GPIO_BASE_ADDR_OFFSET)
    sb  $0, 0x900(t0)
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
//call this function must give the register addr to a0
 	li	t1,128
 	sb	t1,3(a0)
#ifdef  BONITO_33M 
	li      t1,0x12      # divider, highest possible baud rate,for 33M crystal
#elif   BONITO_25M 
	li      t1,0x0e      # divider, highest possible baud rate,for 25M crystal
#else   BONITO_50M 
	li      t1,0x1b      # divider, highest possible baud rate,for 50M crystal
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

#include "i2c_7a.S"
#ifdef AUTO_DDR_CONFIG
#include "ddr_dir/detect_node_dimm_all.S"
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
	move ra, t7
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

LEAF(lock_scache)

#if 1
	dli  t0, 0x900010003ff04000
	dli  t1, 0xffffffffffe00000
	sd   t1, 0x240(t0)
	dli  t1, 0x8000100090000000 
	sd   t1, 0x200(t0)
#if 0   //1M   
	dli  t0, 0x900010003ff04000
	dli  t1, 0xfffffffffff00000
	sd   t1, 0x248(t0)
	dli  t1, 0x8000100090200000 
	sd   t1, 0x208(t0)
#endif

	dli  t0, 0x900000003ff00000
	dli  t1, 0xffffffffffe00000
	sd   t1, 0x240(t0)
	dli  t1, 0x8000000090000000 
	sd   t1, 0x200(t0)
#if 0   //1M 
	dli  t0, 0x900000003ff00000
	dli  t1, 0xfffffffffff00000
	sd   t1, 0x248(t0)
	dli  t1, 0x8000000090200000 
	sd   t1, 0x208(t0)
#endif
	jr   ra
	nop
#endif
END(lock_scache)

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


#######################################
#include "../../../pmon/arch/mips/ls7a/ls3a7a_setup_ht_link.S"
#include "../../../pmon/arch/mips/ls7a/ls3a7a_ht_init.S"
#include "../../../pmon/arch/mips/ls7a/ls7a_config.S"
#include "ddr_dir/ls3A8_ddr_config.S"
#ifdef  LS7A_GMEM_CFG
#include "ddr_dir/ls7A_gmem_config.S"
#endif
#ifdef DDR3_DIMM
#include "loongson3C_ddr3_leveling.S"
#endif
#ifdef ARB_LEVEL
#include "ddr_dir/ARB_level_new.S"
#endif
#if  (defined(DEBUG_DDR) || defined(DEBUG_GMEM))
#include "ddr_dir/Test_Mem.S"
#endif

	.global watchdog_enable
	.ent    watchdog_enable
	.set    noreorder
	.set    mips3
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
    .global ddr2_reg_data
    .global ddr3_reg_data
    .global gmem_reg_data

    .align  5
#include "loongson_mc2_param.S"
#ifdef  LS7A_GMEM_CFG
#include "loongson7A_gmem_param.S"
#endif

