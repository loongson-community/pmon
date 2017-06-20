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



	/*WatchDog_Close*/
	bal wdt_close
	nop

	/*enable rs780e vGA*/
	li  t2, ~(1 << 14)
	lw  t1, 0x4(t0)
	and t1, t2
	sw  t1, 0x4(t0)
	lw  t1, 0x0(t0)
	and t1, t2
	sw  t1, 0x0(t0)

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
	beq	t0, a0, boot_entry
	nop

#ifdef MULTI_NODE_INIT_MEM
	li	a0, 0x3
	and	a0, t0, a0
	beqz	a0, MEM_INIT_BEGIN
	nop
#endif

	b	slave_main
	nop

boot_entry:
        bal     initserial
        nop
        bal     initserial1
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


	TTYDBG ("\r\n")
	TTYDBG ("MEM CLK SEL : ")
	dsrl t0, t1, 5
	andi a0, t0, 0x1f
	bal hexserial
	nop

	TTYDBG ("\r\n")

//Using S1 for passing the NODE ID

	dli	s1, 0x0000000000000000
#include "loongson3_clksetting.S"

##########################################

bal wdt_open
nop

#include "loongson3_fixup.S"

bal wdt_close
nop

//#define SHUT_SLAVES
#ifdef SHUT_SLAVES
	PRINTSTR("Shut down other cores\r\n")
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
	PRINTSTR("NOT Shut down other cores\r\n")
#endif

	dli	s1, 0x0000100000000000
#include "loongson3_clksetting.S"

	dli	s1, 0x0000200000000000
#include "loongson3_clksetting.S"

	dli	s1, 0x0000300000000000
#include "loongson3_clksetting.S"


//#include "loongson3_HT_init.S"

#if 1
       bal     beep_on
       nop
       li      a0,0x1000000
       1:
       addiu   a0,-1
       nop
       bnez    a0,1b
       nop
       bal     beep_off
       nop
#endif

##########################################


	PRINTSTR("NO TLB cache init ...\r\n")

#include "pcitlb.S" /* map 0x1000000-0x1700000 to 0x4000000 */
	
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

#include "loongson3_HT_init.S"

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
//#define DEBUG_DDR_NODE
//#define PRINT_DDR_LEVELING

    TTYDBG("\r\nStart Init Memory, wait a while......\r\n")
#ifdef MULTI_NODE_INIT_MEM
####################################
    b       MEM_INIT_BEGIN
    nop

MEM_INIT_DONE:
#else
####################################
    move    msize, $0
    move    s3, $0
//!!!!important--s1 must be correctly set

#if DEBUG_DDR_NODE
    PRINTSTR("\r\ntest node0?(0xf: skip): ")
    bal     inputaddress
    nop
    and     v0, v0, 0xf
    dli     a1, 0x1
    bgt     v0, a1, 330f
    nop
#endif

    	TTYDBG("NODE 0 MEMORY CONFIG BEGIN\r\n")

#ifdef  AUTO_DDR_CONFIG
    	dli     s1, 0xf1f00000  //set use MC1 or MC0 or MC1/0 and give All device id
#else
	//dli     s1, 0xc2e30400c2e30404
    	//dli     s1, 0xc1c30400c1c30404  // use MC0
    	//dli     s1, 0xc1a10400c1a10404  // use MC0, 2G SCS UDIMM
    	dli     s1, 0xf0a31000f0a31000  // use both, 8G SCS RDIMM
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

330:
#if 1

#if DEBUG_DDR_NODE
    PRINTSTR("\r\ntest node1?(0xf: skip): ")
    bal     inputaddress
    nop
    and     v0, v0, 0xf
    dli     a1, 0x1
    bgt     v0, a1, 330f
    nop
#endif

#if 1 //NODE1
    	TTYDBG("NODE 1 MEMORY CONFIG BEGIN\r\n")

#ifdef  AUTO_DDR_CONFIG
    	dli     s1, 0xf3f20001  //set use MC1 or MC0 or MC1/0 and give All device id
#else
	//dli     s1, 0xc2e30400c2e30405
    	//dli     s1, 0xc1a10400c1a10405  // use MC0, 2G SCS UDIMM
    	dli     s1, 0xf0a31001f0a31001  // use both, 8G SCS RDIMM
#endif
//#include "ddr_dir/loongson3_ddr2_config.S"
#include "ddr_dir/loongson3A2000_ddr2_config.S"


/* test memory */
//TEST if NODE1 has a memory
        dli     t0, 0x00007f0000007f00
        and	t0, s1, t0
        beqz    t0, 11f
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

#if 0
	PRINTSTR("\r\n======X2 cpu map windows:\r\n")
	li      t1, 23
	dli     t2, 0x900010003ff00000
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

330:
#if DEBUG_DDR_NODE
    PRINTSTR("\r\ntest node2?(0xf: skip): ")
    bal     inputaddress
    nop
    and     v0, v0, 0xf
    dli     a1, 0x1
    bgt     v0, a1, 330f
    nop
#endif

#if 1 //NODE2 //TODO
    	TTYDBG("NODE 2 MEMORY CONFIG BEGIN\r\n")

#ifdef  AUTO_DDR_CONFIG
	dli     s1, 0xf7f60002  //set use MC1 or MC0 or MC1/0 and give All device id
#else
	//dli     s1, 0xc2e30400c2e30405
    	//dli     s1, 0xc1a10400c1a10406  // use MC0, 2G SCS UDIMM
    	dli     s1, 0xf0a31002f0a31002  // use both, 8G SCS RDIMM
#endif
//#include "ddr_dir/loongson3_ddr2_config.S"
#include "ddr_dir/loongson3A2000_ddr2_config.S"


/* test memory */
//TEST if NODE1 has a memory
        dli     t0, 0x00007f0000007f00
        and	t0, s1, t0
        beqz    t0, 11f
        nop

        dli     t0, 0x9000200000000000
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
	dli     t5, 0x9000200000000000
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
	dli     t5, 0x9800200000000000
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

330:

#if DEBUG_DDR_NODE
    PRINTSTR("\r\ntest node3?(0xf: skip): ")
    bal     inputaddress
    nop
    and     v0, v0, 0xf
    dli     a1, 0x1
    bgt     v0, a1, 330f
    nop
#endif
#if 1 //NODE3
    	TTYDBG("NODE 3 MEMORY CONFIG BEGIN\r\n")

#ifdef  AUTO_DDR_CONFIG
	dli     s1, 0xf5f40003  //set use MC1 or MC0 or MC1/0 and give All device id
#else
	//dli     s1, 0xc2e30400c2e30405
    	//dli     s1, 0xc1a10400c1a10407  // use MC0, 2G SCS UDIMM
    	dli     s1, 0xf0a31003f0a31003  // use both, 8G SCS RDIMM
#endif
//#include "ddr_dir/loongson3_ddr2_config.S"
#include "ddr_dir/loongson3A2000_ddr2_config.S"


/* test memory */
//TEST if NODE1 has a memory
        dli     t0, 0x00007f0000007f00
        and	t0, s1, t0
        beqz    t0, 11f
        nop

        dli     t0, 0x9000300000000000
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
	dli     t5, 0x9000300000000000
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
	dli     t5, 0x9800300000000000
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

330:
#endif
#endif


	/*judge the node0 whether have memory*/
	and     a0, msize, 0xff
	beqz    a0, beep_on
	nop


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


#ifdef LS3_HT
//#include "loongson3_HT_init.S"
#endif

#include "3aserver_bridge_config.S"
##########################################
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

	PRINTSTR("\r\n======X1 ht map windows:\r\n")
	li      t1, 23
	dli     t2, 0x900000003ff02700
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

##########################################

#include "machine/newtest/newdebug.S"

##########################################

bootnow:
	//bal	spd_info_store
	nop

//#define CORE0_WAIT_ALL_OK
#ifdef CORE0_WAIT_ALL_OK //wait if all other core0 is cleared
	dli	t0, NODE3_CORE0_BUF0
1:
	lw	a0, FN_OFF(t0)
	bnez	a0, 1b
	nop
2:
	dli	t0, NODE2_CORE0_BUF0
1:
	lw	a0, FN_OFF(t0)
	bnez	a0, 1b
	nop
5:
	dli	t0, NODE1_CORE0_BUF0
1:
	lw	a0, FN_OFF(t0)
	bnez	a0, 1b
	nop
#endif


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

LEAF(spd_info_store)
	move    t8,ra

	TTYDBG("\r\n spd_info_store begain.\r\n")

	dli    t5, 0xffffffff8fffa000;

	dli    t7, 0xa1;
	dli    t6, 0xa9;        

4:
	move    a0, t7
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
	move    a1, t0;
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
	jr      ra
	nop
END(spd_info_store)

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
	bal	tgt_putchar
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

	jr	ra
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

	jr	ra
	nop	
END(tgt_putchar)
#endif
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

LEAF(wdt_open)
	/*WatchDog_Close*/
	li  t0, 0xbfe0011c
	li  t2, ~(0x81c)
	lw  t1, 0x4(t0)
	and t1, t2
	sw  t1, 0x4(t0)
	li  t2, ~(1 << 11)
	lw  t1, 0x0(t0)
	and t1, t2
	sw  t1, 0x0(t0)
	li  t2, 0x814
	lw  t1, 0x4(t0)
	or  t1, t2
	sw  t1, 0x4(t0)
	li  t2, (1 << 11)
	lw  t1, 0x0(t0)
	or  t1, t2
	sw  t1, 0x0(t0)
	nop
	jr	ra
	nop
END(wdt_open)

LEAF(wdt_close)
	/*WatchDog_Close*/
	li  t0, 0xbfe0011c
	li  t2, ~(1 << 11)
	lw  t1, 0x4(t0)
	and t1, t2
	sw  t1, 0x4(t0)
	lw  t1, 0x0(t0)
	and t1, t2
	sw  t1, 0x0(t0)
	nop
	jr	ra
	nop
END(wdt_close)

LEAF(beep_on)
	nop
	dli	t1,0x90000cfdfe00a080
	lbu	t0,0(t1)
	or	t0,0x04
	sb	t0,0(t1)
	nop
	jr	ra
	nop
END(beep_on)

LEAF(beep_off)
	nop
	dli	t1,0x90000cfdfe00a080
	lbu	t0,0(t1)
	and	t0,0xfb
	sb	t0,0(t1)
	nop
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
	la	v0, COM3_BASE_ADDR
	bne	v0, v1, 1b
	nop

	jr	ra
	nop
END(initserial)
#else
LEAF(initserial)
	li  a0, GS3_UART_BASE

 	li	t1,128
 	sb	t1,3(a0)
#ifdef BONITO_33M 
	li      t1,0x12      # divider, highest possible baud rate,for 33M crystal
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
#endif
LEAF(initserial1)
	li  a0, GS3_UART1_BASE

	li      t1,128
	sb      t1,3(a0)
	li      t1,0x12      # divider, highest possible baud rate
	sb      t1,0(a0)
	li      t1,0x0     # divider, highest possible baud rate
	sb      t1,1(a0)
	li      t1,3
	sb      t1,3(a0)

	li      t1,0
	sb      t1,1(a0)

	li      t1,71
	sb      t1,2(a0)
	jr      ra
	nop
END(initserial1)




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

	jr	ra
	nop
END(initserial_COM1)

LEAF(stringserial_COM1)
	move	a2, ra
	daddu	a1, a0, s0
	lbu	a0, 0(a1)
1:
	beqz	a0, 2f
	nop
	bal	tgt_putchar_COM1
	addiu	a1, 1
	b	1b
	lbu	a0, 0(a1)

2:
	move	ra, a2
	jr	ra
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
	daddu	v0, s0
	daddu	v0, a0
	bal	tgt_putchar_COM1
	lbu	a0, 0(v0)

	bnez	a3, 1b
	daddu	a3, -1

	move	ra, a2
	jr	ra
	nop
END(hexserial_COM1)

LEAF(tgt_putchar_COM1)
	la	v0, COM1_BASE_ADDR
1:
	lbu	v1, NSREG(NS16550_LSR)(v0)
	and	v1, LSR_TXRDY
	beqz	v1, 1b
	nop

	sb	a0, NSREG(NS16550_DATA)(v0)
	move	v1, v0
	la	v0, COM1_BASE_ADDR
	bne	v0, v1, 1b
	nop

	jr	ra
	nop	
END(tgt_putchar_COM1)


#include "i2c.S"
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


	

LEAF(nbmisc_read_index_mips)
	dli	t1, HT_CONFIG_ADDR
	or	t1, t1, a0
	sw	a1, NBMISC_INDEX(t1)
	lw	v0, 0x64(t1)
	jr	ra
	nop
END(nbmisc_read_index_mips)

LEAF(nbmisc_write_index_mips)
	dli	t1, HT_CONFIG_ADDR
	or	t1, t1, a0
	or	t2, a1, 0x80
	sw	t2, 0x60(t1)
	sw	a2, 0x64(t1)
	jr	ra
	nop
END(nbmisc_write_index_mips)

LEAF(post_code_mips)
	li	t0, CPU_POST_PORT
	sb	a0, 0x0(t0)
	jr	ra
	nop
END(post_code_mips)

LEAF(enable_rs780_dev8)
	move	t6, ra
	li	a0,  0x0
	li	a1,  0x0
	bal	nbmisc_read_index_mips
	nop
	move	v1, v0
	li	t0,  0xffffffbf      // ~(1 << 6)
	and	t1, v1, t0
	li	t0,  0x40  // (1 << 6)
	or	v1, t1, t0
	beq	v1, v0, 1f
	nop
	move	a2,  v1
	bal	nbmisc_write_index_mips
	nop
1:
	move	ra, t6
	jr	ra
	nop
END(enable_rs780_dev8)

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

LEAF(enable_lpcio_dev8)
	move t6, ra
	li   a0,20
	li	 a1,3
	sll  a0,11
	sll  a1,8
	bal    pci_read_config8
	nop
	move   v1, v0
	li     t0,  0x3
	or     t1, v1, t0
	move   a2,  t1
	bal   pci_write_config8
	nop
	move	ra, t6
	jr	ra
	nop
END(enable_lpcio_dev8)

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
#ifdef MULTI_NODE_INIT_MEM
#include "loongson3_ddr_init_4core.S"
#endif

#######################################

    .rdata
    .global ddr2_reg_data
    .global ddr3_reg_data

    .align  5
#include "loongson_mc2_param.S"

