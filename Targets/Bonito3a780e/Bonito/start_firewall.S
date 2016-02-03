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

#include "loongson3firewall_def.h"


#ifdef USE_GPIO_AS_PCIRESET
#define GPIOLED_DIR 0x2
#define GPIO_PCIRESET(x) \
li v0,0xbfe0011c; \
lw v1,4(v0); \
or v1,0xffff; \
xor v1,GPIOLED_DIR; \
sw v1,4(v0); \
li v1,x;\
sw v1,0(v0);\
li v1,0x1000;\
78: \
subu v1,1;\
bnez v1,78b;\
nop;
#else
#define GPIO_PCIRESET(x) 
#endif

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
	mtc0	zero, COP_0_STATUS_REG
	mtc0	zero, COP_0_CAUSE_REG
	li	t0, SR_BOOT_EXC_VEC	/* Exception to Boostrap Location */
	mtc0	t0, COP_0_STATUS_REG
	la	sp, stack
	la	gp, _gp

	nop
	
	WatchDog_Close

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
	move 	k1, ra
	la	a0,_start
	li	s0,0xbfc00000
	subu	s0,a0
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
        move    k1, ra
	la	a0,_start
	li	s0,0xbfc00000
	subu	s0,a0
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
	PRINTSTR("\r\nra=")
	move a0, k1
	bal	hexserial
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

	GPIO_PCIRESET(2)
	GPIO_PCIRESET(2)
	GPIO_PCIRESET(2)
	GPIO_PCIRESET(0)
	GPIO_PCIRESET(0)
	GPIO_PCIRESET(0)

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

	bal	scache_init_64_fast
	dli	t2, NODE0_CORE0_BUF0

	or	t1, t2, t1
	dli	t3, L2_CACHE_OK
	sw	t3, FN_OFF(t1)




	dli     a0, BOOTCORE_ID
	bne     t0, a0, slave_main
	nop

	bal	initserial
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

	/*Read sys_clk_sel*/
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

##########################################

	WatchDog_Enable;
#include "loongson3_fixup.S"

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
	WatchDog_Close;

##########################################
//configure DDR freq by software
#if 1
    li      t5, 0x17
#if 0
    PRINTSTR("Please input DDR frequency SELLECT[4:0](default: 0x1e;0x1f: skip software setting):\r\n");
    dli     t6, 0x00
    bal     inputaddress #input value stored in v0
    nop
    dli     t6, 0x1f
    and     v0, v0, t6
    beq     v0, t6, 88f
    nop
    move    t5, v0
#endif

    TTYDBG  ("\r\nSet DDR frequency by software\r\n")
    move    a2, $0
    bal     config_mc_frequency
    nop

88:
#endif


	PRINTSTR("Init TLB...\r\n")
	bal     tlb_init
	nop

	PRINTSTR("L1 caches init\r\n")
	bal     godson2_cache_init
	nop
	PRINTSTR("Init htpcitlb...\r\n")
#include "pcitlb.S" /* map 0x1000000-0x1700000 to 0x4000000 */
	
/*
 *  Reset and initialize l1 caches to a known state.
 */

	/*
	 * intialize l2 cache 
	 * t0 ebase[0:9] aka boot cpu id
	 * t1 mailbox base
	 * t2 i, starting from 0
	 * t3 nr of cores, aka 4 for 3a780e
	 * t4 mailbox base refering to t2
	 */
	.set     mips64
	mfc0    t0, $15, 1
	.set     mips3
	andi    t0, t0, 0x3ff
	dli	t1, NODE0_CORE0_BUF0
	move	t2, zero
	dli	t3, 4


next_core:
	andi	t5, t2, 0x3
	dsll	t5, 8
	or	t4, t5, t1
	andi	t5, t2, 0xc
	dsll	t5, 42
	or	t4, t4, t5

	li	t6, L2_CACHE_OK
wait_l2_ok:
	lw	t5, FN_OFF(t4)
	bne	t5, t6, wait_l2_ok
	nop
	sw	zero, FN_OFF(t4)

	daddiu	t2, t2, 1
	blt	t2, t3, next_core
	nop
	
	
	andi	t5, t0, 0x3
	dsll	t5, 8
	or	t4, t5, t1
	andi	t5, t0, 0xc
	dsll	t5, 42
	or	t4, t4, t5
	li	t6, L2_CACHE_DONE
	sw	t6, FN_OFF(t4)
	

	PRINTSTR("scache init done\r\n")

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



	/* cxk */
####################################
#include "ddr_dir/ddr_param_define.h"
#include "ddr_dir/ddr_config_define.h"
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
	dli     s1, 0x32100000
#elif 0
	//dli     s1, 0xc1e30400c1e30400
    //set MC1 dimm infor
    dli     a0, ( MC_SDRAM_TYPE_DDR3    /* sdram type: DDR3/DDR2 */ \
                | MC_DIMM_ECC_NO        /* dimm ECC: YES/NO */ \
                | MC_DIMM_BUF_REG_NO    /* dimm buffer register: YES/NO, for RDIMM use YES, all else use NO*/ \
                | MC_DIMM_WIDTH_64      /* memory data width: 64/32 */ \
                | MC_SDRAM_ROW_14       /* sdram row address number: 15~11 */ \
                | MC_SDRAM_COL_10       /* sdram column address number: 12~9 */ \
                | MC_SDRAM_BANK_8       /* sdram bank number: 8/4 */ \
                | MC_ADDR_MIRROR_YES    /* for standard DDR3 UDIMM, use YES else use NO */ \
                | MC_SDRAM_WIDTH_X8     /* SDRAM device data width: 8/16 */ \
                | MC_USE_CS_0_1         /* the CS pins the sdram connected on(split by '_', from small to big) */ \
                | MC_MEMSIZE_(4)        /* MC memory size, unit: 512MB */ \
                )
    dsll    s1, a0, 32
    //set MC0 dimm infor
    dli     a0, ( MC_SDRAM_TYPE_DDR3    /* sdram type: DDR3/DDR2 */ \
                | MC_DIMM_ECC_NO        /* dimm ECC: YES/NO */ \
                | MC_DIMM_BUF_REG_NO    /* dimm buffer register: YES/NO, for RDIMM use YES, all else use NO*/ \
                | MC_DIMM_WIDTH_64      /* memory data width: 64/32 */ \
                | MC_SDRAM_ROW_14       /* sdram row address number: 15~11 */ \
                | MC_SDRAM_COL_10       /* sdram column address number: 12~9 */ \
                | MC_SDRAM_BANK_8       /* sdram bank number: 8/4 */ \
                | MC_ADDR_MIRROR_YES    /* for standard DDR3 UDIMM, use YES, else use NO */ \
                | MC_SDRAM_WIDTH_X8     /* SDRAM device data width: 8/16 */ \
                | MC_USE_CS_0_1         /* the CS pins the sdram connected on(split by '_', from small to big) */ \
                | MC_MEMSIZE_(4)        /* MC memory size, unit: 512MB */ \
                )
    or      s1, s1, a0
    //set used MC and NODE ID
    or      s1, s1, USE_MC_0            /* used memory controller: _0/_1/_0_1 */
    or      s1, s1, MC_NODE_ID_0        /* node ID: 0/1/2/3 */
#else
	//dli     s1, 0xc1ec0400c1e30400  // use MC0
        //dli     s1,0xc9e3040000000000
        dli     s1,0xc0a10400c0a10400
#ifdef DDR_S1
	dli	s1, DDR_S1
#endif
#endif

#include "ddr_dir/loongson3_ddr2_config.S"
	/*judge the node0 whether have memory*/
	and     a0, msize, 0xff
	beqz    a0, beep_on
	nop

	TTYDBG("Init Memory done.\r\n")
##########################################
#ifdef  DEBUG_DDR
	PRINTSTR("\r\nDo test?(0xf: skip): ")
	bal     inputaddress
	nop
	and     v0, v0, 0xf
	dli     a1, 0x1
	bgt     v0, a1, 3f
	nop



	dli     s1, 0x0010000080000000  //NODE 0, start from 0x80000000
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

3:
#endif
##########################################

#ifdef  AUTO_ARB_LEVEL
#include "ddr_dir/store_auto_arb_level_info.S"
#endif
#########################################

	//dump L1-L2-HT config windows
	PRINTSTR("\r\n======X1 core0 map windows:\r\n")
	li      t1, 23
	dli     t2, 0x900000003ff02000
1:
	move    a0, t2
	bal    hexserial64
	nop
	PRINTSTR(": ")

	ld      a0, 0x0(t2)
	bal    hexserial64
	nop
	PRINTSTR("\r\n")

	daddiu  t2, t2, 8
	bnez    t1, 1b
	addiu   t1, t1, -1


	PRINTSTR("\r\n======X2 cpu map windows:\r\n")
	li      t1, 23
	dli     t2, 0x900000003ff00000
1:
	move    a0, t2
	bal    hexserial64
	nop
	PRINTSTR(": ")

	ld      a0, 0x0(t2)
	bal    hexserial64
	nop
	PRINTSTR("\r\n")

	daddiu  t2, t2, 8
	bnez    t1, 1b
	addiu   t1, t1, -1

	PRINTSTR("\r\n======X2 pci map windows:\r\n")
	li      t1, 23
	dli     t2, 0x900000003ff00100
1:
	move    a0, t2
	bal    hexserial64
	nop
	PRINTSTR(": ")

	ld      a0, 0x0(t2)
	bal    hexserial64
	nop
	PRINTSTR("\r\n")

	daddiu  t2, t2, 8
	bnez    t1, 1b
	addiu   t1, t1, -1

	PRINTSTR("\r\n======read HT config reg:\r\n")
	dli     t2, 0x90000efdfb000000

	move    a0, t2
	bal    hexserial64
	nop
	PRINTSTR(": ")

	ld      a0, 0x0(t2)
	bal    hexserial64
	nop
	PRINTSTR("\r\n")

	daddiu    a0, t2, 0x60
	bal    hexserial64
	nop
	PRINTSTR(": ")

	ld      a0, 0x60(t2)
	bal    hexserial64
	nop
	PRINTSTR("\r\n")

	daddiu    a0, t2, 0x68
	bal    hexserial64
	nop
	PRINTSTR(": ")

	ld      a0, 0x68(t2)
	bal    hexserial64
	nop
	PRINTSTR("\r\n")

	daddiu    a0, t2, 0x70
	bal    hexserial64
	nop
	PRINTSTR(": ")

	ld      a0, 0x70(t2)
	bal    hexserial64
	nop
	PRINTSTR("\r\n")

#ifdef MULTI_CHIP 
	PRINTSTR("\r\nNode 1 Xbar map windows:\r\n")
	PRINTSTR("\r\n======X1 core0 map windows:\r\n")
	li      t1, 23
	dli     t2, 0x900010003ff02000
#ifdef LS3B
	daddu   t2, t2, 0x4000
#endif
1:
	move    a0, t2
	bal    hexserial64
	nop
	PRINTSTR(": ")

	ld      a0, 0x0(t2)
	bal    hexserial64
	nop
	PRINTSTR("\r\n")

	daddiu  t2, t2, 8
	bnez    t1, 1b
	addiu   t1, t1, -1


	PRINTSTR("\r\n======X2 cpu map windows:\r\n")
	li      t1, 23
	dli     t2, 0x900010003ff00000
#ifdef LS3B
	daddu   t2, t2, 0x4000
#endif
1:
	move    a0, t2
	bal    hexserial64
	nop
	PRINTSTR(": ")

	ld      a0, 0x0(t2)
	bal    hexserial64
	nop
	PRINTSTR("\r\n")

	daddiu  t2, t2, 8
	bnez    t1, 1b
	addiu   t1, t1, -1

	PRINTSTR("\r\n======X2 pci map windows:\r\n")
	li      t1, 23
	dli     t2, 0x900010003ff00100
#ifdef LS3B
	daddu   t2, t2, 0x4000
#endif
1:
	move    a0, t2
	bal    hexserial64
	nop
	PRINTSTR(": ")

	ld      a0, 0x0(t2)
	bal    hexserial64
	nop
	PRINTSTR("\r\n")

	daddiu  t2, t2, 8
	bnez    t1, 1b
	addiu   t1, t1, -1

	PRINTSTR("\r\n======read HT config reg:\r\n")
	dli     t2, 0x90001efdfb000000
#ifdef LS3B
	daddu   t2, t2, 0x4000
#endif

	move    a0, t2
	bal    hexserial64
	nop
	PRINTSTR(": ")

	ld      a0, 0x0(t2)
	bal    hexserial64
	nop
	PRINTSTR("\r\n")

	daddiu    a0, t2, 0x60
	bal    hexserial64
	nop
	PRINTSTR(": ")

	ld      a0, 0x60(t2)
	bal    hexserial64
	nop
	PRINTSTR("\r\n")

	daddiu    a0, t2, 0x68
	bal    hexserial64
	nop
	PRINTSTR(": ")

	ld      a0, 0x68(t2)
	bal    hexserial64
	nop
	PRINTSTR("\r\n")

	daddiu    a0, t2, 0x70
	bal    hexserial64
	nop
	PRINTSTR(": ")

	ld      a0, 0x70(t2)
	bal    hexserial64
	nop
	PRINTSTR("\r\n")
#endif
##########################################


#ifdef LS3_HT
	WatchDog_Enable;
#include "loongson3_HT_init.S"
	WatchDog_Close;
#endif

	WatchDog_Enable;
#include "dma_test.S"
	WatchDog_Close;

#include "3aserver_bridge_config.S"
##########################################

		/* enable power led */
	bal	 enable_lpcio_dev8  /* enable Super IO config port 2e-2h, 4e-4f */
	nop
	
#if DEBUG_LS3
	PRINTSTR("\r\n======read HT config reg:\r\n")
	dli     t2, 0x90000efdfb000000

	move    a0, t2
	bal    hexserial64
	nop
	PRINTSTR(": ")

	ld      a0, 0x0(t2)
	bal    hexserial64
	nop
	PRINTSTR("\r\n")

	daddiu    a0, t2, 0x60
	bal    hexserial64
	nop
	PRINTSTR(": ")

	ld      a0, 0x60(t2)
	bal    hexserial64
	nop
	PRINTSTR("\r\n")

	daddiu    a0, t2, 0x68
	bal    hexserial64
	nop
	PRINTSTR(": ")

	ld      a0, 0x68(t2)
	bal    hexserial64
	nop
	PRINTSTR("\r\n")

	daddiu    a0, t2, 0x70
	bal    hexserial64
	nop
	PRINTSTR(": ")

	ld      a0, 0x70(t2)
	bal    hexserial64
	nop
	PRINTSTR("\r\n")

#endif

##########################################

#include "machine/newtest/newdebug.S"

##########################################

bootnow:
#ifdef AUTO_DDR_CONFIG
	bal  spd_info_store
	nop;
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


	TTYDBG("sp=");
	move a0, sp
	bal	hexserial
	nop

	li	a0, 4096*1024
	sw	a0, CpuTertiaryCacheSize /* Set L3 cache size */

	PRINTSTR("\r\n")


	/* pass pointer to kseg1 tgt_putchar */
	la  a1, tgt_putchar
	daddu a1,a1,s0

	la  a2, stringserial
	daddu a2,a2,s0

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

	 jr      t8
	 nop
END(spd_info_store)

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
	daddu	v0, s0
	daddu	v0, a0
	bal	tgt_putchar
	lbu	a0, 0(v0)

	bnez	a3, 1b
	daddu	a3, -1

	j	a2
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

	j	ra
	nop	
END(tgt_putchar)
#else

LEAF(tgt_putchar)
.set push
.set noat
	move AT, ra
	la	v0,GS3_UART_BASE 
	bal 1f
	nop
	jr AT
	nop
1:
	lbu	v1, NSREG(NS16550_LSR)(v0)
	and	v1, LSR_TXRDY
#	li	v1, 1
	beqz	v1, 1b
	nop
	sb	a0, NSREG(NS16550_DATA)(v0)
	j	ra
	nop	
.set pop
END(tgt_putchar)
#endif

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

	j	ra
	nop
END(initserial)
#else
LEAF(initserial)
	li  a0, GS3_UART_BASE

 	li	t1, 128
	sb	t1, 3(a0)
#ifdef BONITO_33M 
	li      t1, 0x12      # divider, highest possible baud rate,for 33M crystal
#else
#ifdef BONITO_25M 
	li      t1, 0x0d      # divider, highest possible baud rate,for 25M crystal
#else
#ifdef BONITO_50M 
	li      t1, 0x1b      # divider, highest possible baud rate,for 50M crystal
#endif
#endif
#endif
 	sb	t1, 0(a0)
 	li	t1, 0x0     # divider, highest possible baud rate
 	sb	t1, 1(a0)
 	li	t1, 3
 	sb	t1, 3(a0)

 	#srl	t1, t1, 0x8
 	li	t1, 0
 	sb	t1, 1(a0)
 	#li	t1, 1      # divider, highest possible baud rate


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
	daddu	v0, s0
	daddu	v0, a0
	bal	tgt_putchar_COM1
	lbu	a0, 0(v0)

	bnez	a3, 1b
	daddu	a3, -1

	j	a2
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

	j	ra
	nop	
END(tgt_putchar_COM1)


#include "i2c.S"
#ifdef  AUTO_DDR_CONFIG
#include "ddr_dir/detect_node_dimm.S"
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

/*
  loogson 3a has 4 x l2 cache body, each of which is 4 way, 32Byte per line,
  1 MBype size by defaut, use phys addr bit[5:6] to select which body, and 
  phys addr [0:1] to select which way, this function expect para a0 is a 
  0x40000-aligned xphys address
*/
LEAF(scache_init_64_fast)

	dli     a2, 0x00040000   #cache init 1M/4way
	li      a3, 0x22
	mtc0    a3, CP0_ECC
	mtc0    zero, CP0_TAGHI
	mtc0    zero, CP0_TAGLO
	daddu    v0, zero, a0
	daddu    v1, a0, a2
1:      
	daddiu   v0, v0, 0x80
	 /* body 0*/ 
	cache   Index_Store_Tag_S, -0x80(v0)
	cache   Index_Store_Tag_S, -0x7f(v0)
	cache   Index_Store_Tag_S, -0x7e(v0)
	cache   Index_Store_Tag_S, -0x7d(v0)

	 /* body 1*/
	cache   Index_Store_Tag_S, -0x60(v0)
	cache   Index_Store_Tag_S, -0x5f(v0)
	cache   Index_Store_Tag_S, -0x5e(v0)
	cache   Index_Store_Tag_S, -0x5d(v0)

	 /* body 2*/
	cache   Index_Store_Tag_S, -0x40(v0)
	cache   Index_Store_Tag_S, -0x3f(v0)
	cache   Index_Store_Tag_S, -0x3e(v0)
	cache   Index_Store_Tag_S, -0x3d(v0)

	 /* body 3*/
	cache   Index_Store_Tag_S, -0x20(v0)
	cache   Index_Store_Tag_S, -0x1f(v0)
	cache   Index_Store_Tag_S, -0x1e(v0)
	bne     v0, v1, 1b
	cache   Index_Store_Tag_S, -0x1d(v0)


	jr      ra
	nop

END(scache_init_64_fast)
/*  use 4KB pagesize to initialize all 64 entry tlb */
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
	jr t7
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
	dli	t2, NODE0_CORE0_BUF0
	or	t1, t1, t2
	sd	zero, FN_OFF(t1)
	sd	zero, SP_OFF(t1)
	sd	zero, GP_OFF(t1)
	sd	zero, A1_OFF(t1)


	jr	ra
	nop
END(clear_mailbox)


	

	/* initialize L1 D and I cache 
	 loongson3a has 64KB 4 way, 32Byte/line L1 Dcache
	 loongson3a has 64KB 4 way, 32Byte/line L1 Icache
	 loongson3a has 64KB 4 way, 32Byte/line L1 Icache
	 loongson use address[0:1] to select different way 
	*/
LEAF(godson2_cache_init)
	lui     a0, 0x8000
	li      a2, (1<<14) #64k/4way

	mtc0    $0, CP0_TAGHI
	mtc0    $0, CP0_TAGLO
	li      a1, 0x22
	addu    v0, $0, a0
	addu    v1, a0, a2
1:
	slt     a3, v0, v1
	beq     a3, $0, 2f
	nop
	mtc0    a1, CP0_ECC
	cache   Index_Store_Tag_D, 0x0(v0)
	cache   Index_Store_Tag_D, 0x1(v0)
	cache   Index_Store_Tag_D, 0x2(v0)
	cache   Index_Store_Tag_D, 0x3(v0)

	mtc0    zero, CP0_ECC

	cache   Index_Store_Tag_I, 0x0(v0)
	cache   Index_Store_Tag_I, 0x1(v0)
	cache   Index_Store_Tag_I, 0x2(v0)
	cache   Index_Store_Tag_I, 0x3(v0)
	b	1b
	daddiu   v0, v0, 0x20
2:
	jr      ra
	nop
END(godson2_cache_init)
//lycheng
LEAF(nbmisc_read_index_mips)
	dli	t1, HT_CONFIG_ADDR
	or	t1, t1, a0
	sw	a1, NBMISC_INDEX(t1)
	lw	v0, 0x64(t1)
	j	ra
	nop
END(nbmisc_read_index_mips)

LEAF(nbmisc_write_index_mips)
	dli	t1, HT_CONFIG_ADDR
	or	t1, t1, a0
	or	t2, a1, 0x80
	sw	t2, 0x60(t1)
	sw	a2, 0x64(t1)
	j	ra
	nop
END(nbmisc_write_index_mips)

LEAF(post_code_mips)
	li	t0, CPU_POST_PORT
	sb	a0, 0x0(t0)
	j	ra
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
	j	t6
	nop
END(enable_rs780_dev8)

LEAF(pci_read_config32_mips)
	dli	t1, HT_CONFIG_ADDR
	or	t2, t1, a0
	or	t1, t2, a1
	lw	v0, 0x0(t1)
	j	ra
	nop
END(pci_read_config32_mips)

LEAF(pci_write_config32_mips)
	dli	t1, HT_CONFIG_ADDR
	or	t2, t1, a0
	or	t1, t2, a1
	sw	a2, 0x0(t1)
	j	ra
	nop
END(pci_write_config32_mips)

LEAF(pci_read_config8_mips)
	dli	t1, HT_CONFIG_ADDR
	or	t2, t1, a0
	or	t1, t2, a1
	lb	v0, 0x0(t1)
	j	ra
	nop
END(pci_read_config8_mips)

LEAF(pci_write_config8_mips)
	dli	t1, HT_CONFIG_ADDR
	or	t2, t1, a0
	or	t1, t2, a1
	sb	a2, 0x0(t1)
	j	ra
	nop
END(pci_write_config8_mips)

	LEAF(pci_read_config8)
	dli t1, HT_CONFIG_ADDR
	or  t2, t1, a0
	or  t1, t2, a1
	lb  v0, 0x48(t1)
	j  ra
	nop
	END(pci_read_config8)
	LEAF(pci_write_config8)
	dli t1, HT_CONFIG_ADDR
	or  t2, t1, a0
	or  t1, t2, a1
	sb  a2, 0x48(t1)
	j  ra
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
	j  t6
	nop
	END(enable_lpcio_dev8)


	.ent    slave_main
slave_main:

	bal 	tlb_init
	nop

	bal     godson2_cache_init
	nop

	dli     t2, NODE0_CORE0_BUF0
	dli	t3, BOOTCORE_ID
	dsll    t3, 8
	or      t2, t2, t3

wait_scache_allover:
	lw	t4, FN_OFF(t2)
	dli	t5, L2_CACHE_DONE
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


wait_to_jump_kernel:

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

	jalr    t2  # slave core jump to kernel, byebye
	nop

	.end    slave_main


#######################################
#include "ddr_dir/ddr_config.S"
#ifdef ARB_LEVEL
//#define KEEP_GATE_INIT_DLY_FIXED
//#define KEEP_DQ_DLY_FIXED
//#define USE_SPECIAL_WRLVL_DQ_DELAY
#ifdef  USE_SPECIAL_WRLVL_DQ_DELAY
#define WRLVL_DQ_SPECIAL_DLY 0x1c1a1c1c1a1c1c1a //specific the wrlvl_dq_delay for each byte lane
#endif
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

#######################################

    .rdata
    .global ddr2_reg_data
    .global ddr3_reg_data

    .align  5
#include "loongson3A3_ddr_param.S"

#ifdef  ARB_LEVEL
    .text
    .global c0_mc0_level_info 
    .global c0_mc1_level_info 

#include "ddr_dir/loongson3A3_ddr_param.lvled.S"
#else
#ifdef FIX_DDR_PARAM
#include "loongson3A3_ddr_param.fix.S"
#endif
#endif
