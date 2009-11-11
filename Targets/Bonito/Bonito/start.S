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
 * MODIFY 1:
 *	Author : jlliu Date : 07-10-10
 *	1, ignored the former via686b sourthbridge init which just after the northbridge
 *	init process.
 *	2, write out a new superio_init instead, use the new sourthbridge and its
 *	relative functions. initialized the uart MSR and smb MSR.
 *	3, write out a new i2cread instead for spd read
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
#include "target/cs5536.h"
#include "target/cs5536_pci.h"
#include "target/i8254.h"
#include "target/isapnpreg.h"
#define DEBUG_LOCORE

#include "cs5536.h"


/***********************************MAIN FLOW****************************************/

#ifdef DEBUG_LOCORE
#define	TTYDBG(x) \
	.rdata;98: .asciz x; .text; la a0, 98b; bal stringserial; nop
#else
#define TTYDBG(x)
#endif

#define	PRINTSTR(x) \
	.rdata;98: .asciz x; .text; la a0, 98b; bal stringserial; nop

#define CONFIG_CACHE_64K_4WAY 1

#define tmpsize		s1
#define msize		s2
#define bonito		s4
#define dbg		s5
#define sdCfg		s6

#define CP0_CONFIG $16
#define CP0_TAGLO  $28
#define CP0_TAGHI  $29

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

	bal	uncached		/* Switch to uncached address space */
	nop

	bal	locate			/* Get current execute address */
	nop

uncached:
	or	ra, UNCACHED_MEMORY_ADDR
	j	ra
	nop
	
/***********************************EXC VECTOR***************************************/

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
exc_tlb:
	move	k0, ra		#save ra
	la	a0, v200_msg
	bal	stringserial
	nop
	b	exc_common
	nop
	
	.align	8
	nop

/* General exception */
	.align	7			/* bfc00380 */
exc_general:
	move	k0, ra		#save ra
	la	a0, v380_msg
	bal	stringserial
	nop
	    
	b	exc_common
	nop
	
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
	PRINTSTR("\r\nDERR0=")
	cfc0	a0, COP_0_DERR_0
	bal	hexserial
	nop
	PRINTSTR("\r\nDERR1=")
	cfc0	a0, COP_0_DERR_1
	bal	hexserial
	nop
stop:
	b	stop
//	b	ext_map_and_reboot
	nop

/****************************************LOCATE*********************************/

/*
 *  We get here from executing a bal to get the PC value of the current execute
 *  location into ra. Check to see if we run from ROM or if this is ramloaded.
 */
locate:
	la	s0,start
	subu	s0,ra,s0
	and	s0,0xffff0000

	li	t0,SR_BOOT_EXC_VEC
	mtc0	t0,COP_0_STATUS_REG
	mtc0    zero,COP_0_CAUSE_REG
	.set noreorder

 	li	bonito,PHYS_TO_UNCACHED(BONITO_REG_BASE)

	/* 
	 * we use the godson2f cpu internal uart for debugging,
	 * so there is no need for sourthbridge uart initial.
	 */
#ifdef USE_GODSON2F_UART

	bal	initserial
	nop
	PRINTSTR("\r\n godson2f cpu internal uart test...\r\n")
	nop
	
#endif


#define MOD_MASK	0x00000003
#define MOD_B		0x00000000 /* byte "modifier" */
#define MOD_H		0x00000001 /* halfword "modifier" */
#define MOD_W		0x00000002 /* word "modifier" */
#if __mips64
#	define MOD_D		0x00000003 /* doubleword "modifier" */
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

#ifdef	USE_GODSON2F_UART
        move    t5,a0  
	
	PRINTSTR("a0: ");
	move	a0,t5
	bal	hexserial
        nop

	PRINTSTR("   t3: ");
        move    a0,t3
	bal	hexserial
        nop

	PRINTSTR("   t0: ");
        move    a0,t0
	bal	hexserial
        nop

	PRINTSTR("\r\n");
        move    a0,t5
#endif
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
1:	bnez	t0,1b
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
	 * pre-initial the sourthbridge superio module.
	 */
	 
#ifdef	USE_GODSON2F_UART

	PRINTSTR("\r\njust before superio init...\r\n")

#ifdef	TEST_CS5536_ID

#define	TEST_PCICONF_READW(dev, func, reg) \
	li	a0, CFGADDR(dev, func, reg); \
	li	a1, PHYS_TO_UNCACHED(PCI_CFG_SPACE); \
	and	a2, a0, 0xffff; \
	or	a1, a2; \
	srl	a0, 16; \
	li	a2, BONITO_BASE+BONITO_PCIMAP_CFG; \
	sw	a0, BONITO_PCIMAP_CFG(bonito); \
	lw	zero, BONITO_PCIMAP_CFG(bonito); \
	lw	a0, (a1);

	PRINTSTR("\r\njust before superio init...\r\n")
11:
	
	li	v0, 0xbfd00000;
	li	v1, 0x02000000;
	sw	v1, 0(v0);

	TEST_PCICONF_READW(14, 0, 0x00);
	move	v0, a0;
	
	li	a1, 0x208f1022;
	beq	a0, a1, 55f;
	nop;

	move	a0, v0;
	bal hexserial;
	nop;
	TTYDBG("\r\n");
	
	li v1, 0x10000
100:
        bnez    v1,100b
        addiu   v1, -1


	b	11b;
	nop;

55:
	PRINTSTR("\r\nend of cs5536 id test...\r\n");
#endif	/* TEST_CS5536_ID */

#endif	/* USE_GODSON2F_UART */

#if	NCS5536 > 0
	bal 	superio_init
	nop
#endif
	
#ifndef	USE_GODSON2F_UART

	bal	initserial
	nop
#endif

/*****************************SMB*******************************/

#ifdef TEST_CS5536_SMB
/* 
 * t4 : the start register number of smb
 * t1 : the end register number of smb
 */
#define	SLAVE_SMB_ADDR		0xa0
#define	SLAVE_SMB_REG_COUNT	0x100
	PRINTSTR("\r\n cs5536 SMB test...\r\n");
	
	move	t4, zero;
    li		t1, SLAVE_SMB_REG_COUNT;
1:
	move	a0, t4;
	bal	hexserial;
	nop;
	PRINTSTR("    ");
	nop;
	
	move	a1, t4;
	li	a0, SLAVE_SMB_ADDR | 0x01; // read op
	bal	i2cread;
	nop;

	move	a0, v0;
	bal	hexserial;
	nop;
	PRINTSTR("\r\n");

	bne	t4, t1, 1b;
	addi	t4, 1;

	PRINTSTR("\r\n cs5536 SMB test end....\r\n");
#endif

/***************************************************************/


/**********************************DDR2****************************************/

	PRINTSTR("\r\nPMON2000 MIPS Initializing. Standby...\r\n")
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

	
##############################################

/* 
 * now, we just write ddr2 parameters directly. 
 * we should use i2c for memory auto detecting. 
 */ 
gs_2f_v3_ddr2_cfg:
	TTYDBG	("\r\nenable register space of MEMORY\r\n")
	
        li	t2, 0xbfe00180
        ld	a1, 0x0(t2)
	/* disable ddr_buffer_cpu according to beijing suggestion. */
	and	a1, a1, 0x4ff
	/* this is the default setting. */
	#and	a1, a1, 0x6ff
	/* disable ddr2 config, just for debugging. */
        #or	a1, a1, 0x100
        sd	a1, 0x0(t2)
	
	li	msize, 0x0f000000
	PRINTSTR("DDR2 config begin...\r\n")
	bal	ddr2_config
	nop
	PRINTSTR("DDR2 config end\r\n")

#if 1 /* read ddr2 registers for debugging */
        li	t0, 0xaffffe00
        
not_locked:
        ld	t1, 0x10(t0)
        andi    t1, 0x01
        beqz    t1, not_locked
        nop

        PRINTSTR("DDR2 DLL locked\r\n")
        
        ld	t1, 0xf0(t0)
        move    a0, t1
        bal     hexserial
        nop

#endif
        
#if 0
	TTYDBG	("\r\ndisable register space of MEMORY\r\n")
        li	t2, 0xbfe00180
        ld	a1, 0x0(t2)
        or	a1, a1,0x100
        sd	a1, 0x0(t2)
#endif    

#if	NCS5536 > 0

        PRINTSTR("MEMORY TYPE AND SIZE PROBE.\r\n");
	
	/* set the default memory size : 256MB */
	li	msize, 0x10000000;

	/* read DIMM memory size per side */
	li	a0, 0xa1;
	li	a1, 31;
	bal	i2cread;
	nop;
	beqz	v0, .nodimm;
	nop;
	sll a0, v0, 2
	la  a1, bigmem
	addu a1, a1, s0
	addu a1, a1, a0
	lw  tmpsize, (a1)
	li  a0, 0x10
	blt v0, a0, 2f
	nop
	sll	tmpsize, v0, 22;		// multiply by 4M

2:	/* read DIMM number of sides (banks) */
	li	a0, 0xa1;
	li	a1, 5
	bal	i2cread
	nop
	bltu	v0, 0x60, .nodimm;
	nop;
	subu	v0, 0x60;
	sll	tmpsize, v0;
	nop;

	move	msize, tmpsize;
        PRINTSTR("type : ddr2, size = 0x");
	move	a0, msize;
	srl	a0, 20;
	bal	hexserial;
	nop;
	PRINTSTR("MB\r\n");
	b	1f;
	nop;
	
.nodimm :
	PRINTSTR("No ddr2 type memory found.\r\n");
	PRINTSTR("Check i2c circuit\r\n");
	PRINTSTR("now, we use default set\r\n");
	b	1f;
	nop;
	
1:
	nop;
	
#endif

// FOR rom access speed update.
#if	1
        li	t2, 0xbfe00108;
	lw	t1, 0(t2);
	li	a0, 0xffffff83;
	and	t1, a0;
	ori	t1, 0xc;
	sw	t1, 0(t2);
	nop;
	
	li	t2, 0xbfe00108;
	lw	t1, 0(t2);
	li	a0, 0xffffe0ff;
	and	t1, a0;
	ori	t1, 0x0f00;
	sw	t1, 0(t2);

	PRINTSTR("jlliu : rom speed reg : 0x");
	lw	t1, 0(t2);
	move	a0, t1;
	bal	hexserial;
	nop;
	PRINTSTR("\r\n");
#endif


######################skipdimm####################

	
skipdimm:

	li	t1,0		# accumulate pcimembasecfg settings
		
	/* set bar0 mask and translation to point to SDRAM */
	//sub	t0,msize,1
	//Felix 2008-10-06
	subu t0,msize,1
	not	t0
	srl	t0,BONITO_PCIMEMBASECFG_ASHIFT-BONITO_PCIMEMBASECFG_MEMBASE0_MASK_SHIFT
	and	t0,BONITO_PCIMEMBASECFG_MEMBASE0_MASK
	or	t1,t0
	
	li	t0,0x00000000
	srl	t0,BONITO_PCIMEMBASECFG_ASHIFT-BONITO_PCIMEMBASECFG_MEMBASE0_TRANS_SHIFT
	and	t0,BONITO_PCIMEMBASECFG_MEMBASE0_TRANS
	or	t1,t0
	or	t1,BONITO_PCIMEMBASECFG_MEMBASE0_CACHED

	/* set bar1 to minimum size to conserve PCI space */
	li	t0, ~0
	srl	t0,BONITO_PCIMEMBASECFG_ASHIFT-BONITO_PCIMEMBASECFG_MEMBASE1_MASK_SHIFT
	and	t0,BONITO_PCIMEMBASECFG_MEMBASE1_MASK
	or	t1,t0
	
	li	t0,0x00000000
	srl	t0,BONITO_PCIMEMBASECFG_ASHIFT-BONITO_PCIMEMBASECFG_MEMBASE1_TRANS_SHIFT
	and	t0,BONITO_PCIMEMBASECFG_MEMBASE1_TRANS
	or	t1,t0
	or	t1,BONITO_PCIMEMBASECFG_MEMBASE1_CACHED

	sw	t1,BONITO_PCIMEMBASECFG(bonito)

	/* enable configuration cycles now */
	lw	t0,BONITO_BONPONCFG(bonito)
	and	t0,~BONITO_BONPONCFG_CONFIG_DIS
	sw	t0,BONITO_BONPONCFG(bonito)

	PRINTSTR("Init SDRAM Done!\r\n");


/**************************************CACHE*****************************/

/*
 *  Reset and initialize caches to a known state.
 */
#define IndexStoreTagI	0x08
#define IndexStoreTagD	0x09
#define IndexStoreTagS	0x0b
#define IndexStoreTagT	0x0a
#define FillI		0x14

/*
 *  RM7000 config register bits.
 */
#define CF_7_SE         (1 << 3)        /* Secondary cache enable */
#define CF_7_SC         (1 << 31)       /* Secondary cache not present */
#define CF_7_TE         (1 << 12)       /* Tertiary cache enable */
#define CF_7_TC         (1 << 17)       /* Tertiary cache not present */
#define CF_7_TS         (3 << 20)       /* Tertiary cache size */
#define CF_7_TS_AL      20              /* Shift to align */
#define NOP8 nop;nop;nop;nop;nop;nop;nop;nop
do_caches:
	TTYDBG("Sizing caches...\r\n");

	mfc0	t3, COP_0_CONFIG	/* t3 = original config */
	and	t3, 0xffffeff0		/* Make sure coherency is OK */

	and	t3, ~(CF_7_TE|CF_7_SE|CF_7_TC|CF_7_SC)  /* disable L2/L3 cache */
	mtc0    t3, COP_0_CONFIG

	li	t2, 4096

	srl	t1, t3, 9
	and	t1, 3
	sllv	s3, t2, t1		/* s3 = I cache size */

#ifdef CONFIG_CACHE_64K_4WAY 
        sll     s3,2
#endif

	and	t1, t3, 0x20
	srl	t1, t1, 1
	addu	s4, t1, 16		/* s4 = I cache line size */

	srl	t1, t3, 6
	and	t1, 3
	sllv	s5, t2, t1		/* s5 = D cache size */

#ifdef CONFIG_CACHE_64K_4WAY
        sll     s5,2
#endif

	and	t1, t3, 0x10
	addu	s6, t1, 16		/* s6 = D cache line size */
	TTYDBG("Init caches...\r\n")

	li	s7, 0                   /* no L2 cache */
	li	s8, 0                   /* no L3 cache */

#if 0
        mfc0    a0, COP_0_PRID
        li      a1, 0x6303	// for godson2f id
        bne     a0,a1,1f
        nop
#endif
	TTYDBG("godson2 TLB clear");
	bal CPU_TLBClear
	nop
	TTYDBG("godson2 caches found\r\n")
        bal     godson2_cache_init
        nop

	/* close L2 cache */
	li      a0, 0xbfe00164
        sw      zero, 0(a0);

        mfc0   a0,COP_0_CONFIG
        and    a0,a0,~((1<<12) | 3)
	or     a0,a0,2
        mtc0   a0,COP_0_CONFIG

#ifdef DEBUG_LOCORE
	TTYDBG("Init caches done, cfg = ")
	mfc0	a0, COP_0_CONFIG
	bal	hexserial
	nop
	TTYDBG("\r\n\r\n")
#endif

/***********************MEMORY DEBUGGING AND COPY SELF TO RAM***********************/

/* zhb */
#ifdef	TEST_DDR_BEFORE_COPY	
/*	li	t0, 0		*/
/*	li	t1, 0x90000	*/
/*1:	addiu	t0, 1		*/
/*	bne	t0, t1, 1b	*/
	nop

	TTYDBG("Testing memory...\r\n")
	
	li	t7, 3
tmem:
	li	t0, 0xa0000000+1*1024*1024
	li	t1, 0xa0000000
	li	t2, 0xffffffff
1:
	sw	t2, 0(t1)
	lw	t3, 0(t1)
	bne	t3, t2, 1f
	nop
	not	t2
	sw	t2, 0(t1)
	lw	t3, 0(t1)
	bne	t3, t2, 1f
	nop
	not	t2
	subu	t2, 1
	addu	t1, 4
	beq	t1, t0, 2f
	nop
	and	t4, t1, 0x000fffff
	bnez	t4, skipdot
	li	a0, '.'
	bal	tgt_putchar
	nop
skipdot:
	b	1b
	nop
1:
	TTYDBG("Memory test failed at ");
	move	a0,	t1
	bal	hexserial
	nop
	TTYDBG("\r\nWrite=");
	move	a0, t2
	bal	hexserial
	nop
	TTYDBG("\r\nRead=");
	move	a0, t3
	bal	hexserial
	nop
1:
	b	1b
	nop
2:
	TTYDBG("Testing ok...\r\n");
	sub	t7,1
	beqz	t7, 1f
	nop
	b	tmem
	nop
//1:	
//	b	1b
//	nop
#endif

//#include "machine/newtest/mydebug.S"
1:
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
#if	1
	TTYDBG("TEST _EDATA...\r\n");
	la	a0, _edata;
	bal hexserial;
	nop;
#endif


#endif
	la	a0, start
	li	a1, 0xbfc00000
	la	a2, _edata
        or      a0, 0xa0000000
        or      a2, 0xa0000000
	subu	t1, a2, a0
	srl	t1, t1, 2

	move	t0, a0
	move	t1, a1
	move	t2, a2
	
	/* copy text section */
	
1:	and	t3,t0,0x0000ffff
	bnez	t3,2f
	nop
	move	a0,t0
	bal	hexserial
	nop
	li	a0,'\r'
	bal 	tgt_putchar
	nop
2:	lw	t3, 0(t1)
	nop
	sw	t3, 0(t0)
	addu	t0, 4
	addu	t1, 4
	bne	t2, t0, 1b
	nop

	PRINTSTR("\ncopy text section done.\r\n")
	
	/* Clear BSS */
	la	a0, _edata
	la	a2, _end
2:	sw	zero, 0(a0)
	bne	a2, a0, 2b
	addu	a0, 4


	TTYDBG("Copy PMON to execute location done.\r\n")
/* zhb */
#ifdef	TEST_DDR_AFTER_COPY
zhb:
	TTYDBG("Testing...\r\n")
	la	a0, start
	li	a1, 0xbfc00000
	la	a2, _edata
        or      a0, 0xa0000000
        or      a2, 0xa0000000
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

/*********************************TLB*****************************************/

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

	move	a0,msize
	srl	a0,20

    /* pass pointer to kseg1 tgt_putchar */
    la  a1, tgt_putchar
	addu a1,a1,s0

    la  a2, stringserial
	addu a2,a2,s0

	/*TTYDBG("\r\nenter initmips...\r\n");*/
	
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
LEAF(CPU_TLBClear)
	li	a3, 0			# First TLB index.

//	li	a2, PG_SIZE_4K
	MTC0  $0,$6     #clear wired register
	li     a2,PG_SIZE_16K
	MTC0   a2, COP_0_TLB_PG_MASK   # Whatever...
	lui    a0,0x8000

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

/***********************************FUNCTIONS**********************************/

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
	bal	tgt_putchar
	lbu	a0, 0(v0)

	bnez	a3, 1b
	addu	a3, -1

	j	a2
	nop
END(hexserial)

LEAF(tgt_putchar)
	la	v0, COMMON_COM_BASE_ADDR
1:
	lbu	v1, NSREG(NS16550_LSR)(v0)
	and	v1, LSR_TXRDY
	beqz	v1, 1b
	nop

	sb	a0, NSREG(NS16550_DATA)(v0)
	move v1,v0
	la	v0, COMMON_COM_BASE_ADDR
	bne v0,v1,1b
	nop
	j	ra
	nop	
END(tgt_putchar)

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
        la      v0, COMMON_COM_BASE_ADDR
1:
        li      v1, FIFO_ENABLE|FIFO_RCV_RST|FIFO_XMT_RST|FIFO_TRIGGER_1
        sb      v1, NSREG(NS16550_FIFO)(v0)
        li      v1, CFCR_DLAB                  #DLAB
        sb      v1, NSREG(NS16550_CFCR)(v0)
        li      v1, NS16550HZ/(16*CONS_BAUD)   #set BRDL
        sb      v1, NSREG(NS16550_DATA)(v0)
        srl     v1, 8
        sb      v1, NSREG(NS16550_IER)(v0)     #set BRDH
        li      v1, CFCR_8BITS                 #8bit
        sb      v1, NSREG(NS16550_CFCR)(v0)
        li      v1, MCR_DTR|MCR_RTS
        sb      v1, NSREG(NS16550_MCR)(v0)
        li      v1, 0x0
        sb      v1, NSREG(NS16550_IER)(v0)

        move    v1, v0
        la      v0, COMMON_COM_BASE_ADDR
        bne     v0, v1, 1b
        nop

        j       ra
        nop
END(initserial)



__main:
	j	ra
	nop


	.rdata
v200_msg:
	.asciz	"\r\nPANIC! Unexpected TLB refill exception!\r\n"
v380_msg:
	.asciz	"\r\nPANIC! Unexpected General exception!\r\n"
hexchar:
	.ascii	"0123456789abcdef"
bigmem:
    .word  0
	.word  0x40000000
	.word  0x80000000
	.word  0x0
	.word  0x0 /*4G fixme*/

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

#define Index_Store_Tag_D			0x05
#define Index_Invalidate_I			0x00
#define Index_Writeback_Inv_D			0x01
#define Index_Store_Tag_S			0x0b
#define Index_Writeback_Inv_S			0x03

LEAF(nullfunction)
	jr ra
	nop
END(nullfunction)

LEAF(godson2_cache_init)
####part 2####
cache_detect_2way:
	mfc0	t4, CP0_CONFIG
	andi	t5, t4, 0x0e00
	srl	t5, t5, 9
	andi	t6, t4, 0x01c0
	srl	t6, t6, 6
	addiu	t6, t6, 11
	addiu	t5, t5, 11
	addiu	t4, $0, 1
	sllv	t6, t4, t6
	srl	t6,1
	sllv	t5, t4, t5
	srl	t5,1
	addiu	t7, $0, 2
####part 3####
	lui	a0, 0x8000
	addu	a1, $0, t5
	addu	a2, $0, t6
cache_init_d2way:
#a0=0x80000000, a1=icache_size, a2=dcache_size
#a3, v0 and v1 used as local registers
	mtc0	$0, CP0_TAGHI
	addu	v0, $0, a0
	addu	v1, a0, a2
1:	slt	a3, v0, v1
	beq	a3, $0, 1f
	nop
	mtc0	$0, CP0_TAGLO
	cache	Index_Store_Tag_D, 0x0(v0)
	mtc0	$0, CP0_TAGLO
	cache	Index_Store_Tag_D, 0x1(v0)
	mtc0	$0, CP0_TAGLO
	cache   Index_Store_Tag_D, 0x2(v0)
	mtc0	$0, CP0_TAGLO
	cache   Index_Store_Tag_D, 0x3(v0)
	beq	$0, $0, 1b
	addiu	v0, v0, 0x20

#if 1
1:
cache_init_l24way:
        mtc0    $0, CP0_TAGHI
        addu    v0, $0, a0
        addu    v1, a0, 128*1024
1:      slt     a3, v0, v1
        beq     a3, $0, 1f
        nop
        mtc0    $0, CP0_TAGLO
        cache   Index_Store_Tag_S, 0x0(v0)
        mtc0    $0, CP0_TAGLO
        cache   Index_Store_Tag_S, 0x1(v0)
        mtc0    $0, CP0_TAGLO
        cache   Index_Store_Tag_S, 0x2(v0)
        mtc0    $0, CP0_TAGLO
        cache   Index_Store_Tag_S, 0x3(v0)
        beq     $0, $0, 1b
        addiu   v0, v0, 0x20


1:
cache_flush_4way:
	addu	v0, $0, a0
	addu	v1, a0, 128*1024
1:	slt	a3, v0, v1
	beq	a3, $0, 1f
	nop
	cache	Index_Writeback_Inv_S, 0x0(v0)
	cache	Index_Writeback_Inv_S, 0x1(v0)
	cache	Index_Writeback_Inv_S, 0x2(v0)
	cache	Index_Writeback_Inv_S, 0x3(v0)
	beq	$0, $0, 1b
	addiu	v0, v0, 0x20
# endif

1:
cache_flush_i2way:
	addu	v0, $0, a0
	addu	v1, a0, a1
1:	slt	a3, v0, v1
	beq	a3, $0, 1f
	nop
	cache	Index_Invalidate_I, 0x0(v0)
#	cache	Index_Invalidate_I, 0x1(v0)
#	cache	Index_Invalidate_I, 0x2(v0)
#	cache	Index_Invalidate_I, 0x3(v0)
	beq	$0, $0, 1b
	addiu	v0, v0, 0x20
1:
cache_flush_d2way:
	addu	v0, $0, a0
	addu	v1, a0, a2
1:	slt	a3, v0, v1
	beq	a3, $0, 1f
	nop
	cache	Index_Writeback_Inv_D, 0x0(v0)
	cache	Index_Writeback_Inv_D, 0x1(v0)
	cache	Index_Writeback_Inv_D, 0x2(v0)
	cache	Index_Writeback_Inv_D, 0x3(v0)
	beq	$0, $0, 1b
	addiu	v0, v0, 0x20
1:
cache_init_finish:
	nop
	jr	ra
	nop

cache_init_panic:
	TTYDBG("cache init panic\r\n");
1:	b	1b
	nop
	.end	godson2_cache_init

#define PCICONF_WRITEB(dev,func,reg,data) \
	li      a0,CFGADDR(dev,func,reg); \
	li      a1,PHYS_TO_UNCACHED(PCI_CFG_SPACE); \
        and     a2,a0,0xffff; \
	or      a1,a2; \
	srl     a0,16; \
	li      a2,BONITO_BASE+BONITO_PCIMAP_CFG; \
    	sw      a0,BONITO_PCIMAP_CFG(bonito); \
    	lw      zero,BONITO_PCIMAP_CFG(bonito); \
    	or	a0,zero,data; \
	sb      a0,(a1);

#if	0
#define PCICONF_WRITEW(dev,func,reg,data) \
	li      a0,CFGADDR(dev,func,reg); \
	li      a1,PHYS_TO_UNCACHED(PCI_CFG_SPACE); \
        and     a2,a0,0xffff; \
	or      a1,a2; \
	srl     a0,16; \
	li      a2,BONITO_BASE+BONITO_PCIMAP_CFG; \
    	sw      a0,BONITO_PCIMAP_CFG(bonito); \
    	lw      zero,BONITO_PCIMAP_CFG(bonito); \
    	or	a0,zero,data; \
	sw      a0,(a1);
#endif

#define PCICONF_ORB(dev,func,reg,data) \
	li      a0,CFGADDR(dev,func,reg); \
        li      a1,PHYS_TO_UNCACHED(PCI_CFG_SPACE); \
        and     a2,a0,0xffff; \
        or      a1,a2; \
        srl     a0,16; \
        li      a2,BONITO_BASE+BONITO_PCIMAP_CFG; \
        sw      a0,BONITO_PCIMAP_CFG(bonito); \
        lw      zero,BONITO_PCIMAP_CFG(bonito); \
    	lbu	a2,(a1); \
    	ori	a2,data; \
        sw      a0,BONITO_PCIMAP_CFG(bonito); \
        lw      zero,BONITO_PCIMAP_CFG(bonito); \
    	sb	a2,(a1);
#define SUPERIO_WR(idx,data) \
        li 	v0,BONITO_PCIIO_BASE_VA+0x3f0; \
        or      v1,zero,idx; \
        sb      v1,(v0); \
        or 	v1,zero,data; \
        sb      v1,1(v0);


/********************************************************************************
 *										*
 *	      			      CS5536					*
 *										*
 ********************************************************************************/

#define PCICONF_WRITEW(dev,func,reg,data) \
        li      a0,CFGADDR(dev,func,reg); \
	li      a1,PHYS_TO_UNCACHED(PCI_CFG_SPACE); \
	and     a2,a0,0xffff; \
	or      a1,a2; \
	srl     a0,16; \
	li      a2,BONITO_BASE+BONITO_PCIMAP_CFG; \
    	sw      a0,BONITO_PCIMAP_CFG(bonito); \
    	lw      zero,BONITO_PCIMAP_CFG(bonito); \
    	or	a0,zero,data; \
	sw      a0,(a1);

#define	PCICONF_READW(dev, func, reg) \
	li	a0, CFGADDR(dev, func, reg); \
	li	a1, PHYS_TO_UNCACHED(PCI_CFG_SPACE); \
	and	a2, a0, 0xffff; \
	or	a1, a2; \
	srl	a0, 16; \
	li	a2, BONITO_BASE+BONITO_PCIMAP_CFG; \
	sw	a0, BONITO_PCIMAP_CFG(bonito); \
	lw	zero, BONITO_PCIMAP_CFG(bonito); \
	lw	a0, (a1);

#define	CS5536_MSR_WRITE(reg, lo, hi) \
	PCICONF_WRITEW(PCI_IDSEL_CS5536, 0, 0xF4, reg); \
	PCICONF_WRITEW(PCI_IDSEL_CS5536, 0, 0xF8, lo); \
	PCICONF_WRITEW(PCI_IDSEL_CS5536, 0, 0xFC, hi);
	
#define	CS5536_MSR_READ(reg) \
	PCICONF_WRITEW(PCI_IDSEL_CS5536, 0, 0xF4, reg); \
	PCICONF_READW(PCI_IDSEL_CS5536, 0, 0xF8); \
	PCICONF_READW(PCI_IDSEL_CS5536, 0, 0xFC);

/* 
 * gpio bit setting
 */
#define	GPIO_HI_BIT(bit, reg) \
	lui	v1, 0xbfd0; \
	ori	v1, reg;    \
	lw	v0, 0(v1);  \
	li	a0, 0x0001; \
	sll	a0, bit;    \
	or	v0, a0;     \
	sll	a0, 16;     \
	not	a0;         \
	and	v0, a0;     \
	sw	v0, 0(v1);
	
#define	GPIO_LO_BIT(bit, reg) \
	lui	v1, 0xbfd0; \
	ori	v1, reg;    \
	lw	v0, 0(v1);  \
	li	a0, 0x0001; \
	sll	a0, (bit + 16); \
	or	v0, a0;     \
	srl	a0, 16;     \
	not	a0;         \
	and	v0, a0;     \
	sw	v0, 0(v1);

/*
 * io space byte access
 */
#define	IO_READ_BYTE(reg) \
	lui	v1, 0xbfd0; \
	ori	v1, reg;    \
	lbu	v0, 0(v1);

#define	IO_WRITE_BYTE(reg) \
	lui	v1, 0xbfd0; \
	ori	v1, reg;    \
	sb	v0, 0(v1);

#define	IO_READ_DWORD(reg) \
	lui	v1, 0xbfd0; \
	ori	v1, reg;    \
	lw	v0, 0(v1);

#define	IO_WRITE_DWORD(reg) \
	lui	v1, 0xbfd0; \
	ori	v1, reg;    \
	sw	v0, 0(v1);

/**********************************************************************/

#define	HW_POWER_OFF_TIME	0x18000		// Hardware power off delay time is 3s
#define	UART1_TX	8
#define	UART1_RX	9
#define	UART2_TX	4
#define	UART2_RX	3
#define	I8259_IRQ	12
#define	MIDE_IRQ	2
#define	GPIO_5		5
#ifdef LOONGSON2F_ALLINONE
#define GPIO_6      6
#define GPIO_7      7
#define GPIO_8      8
#define GPIO_11     11
#endif
#define	SMB_CLK		14
#define	SMB_DATA	15
#define	PWR_BUT		(28 - 16)	// WE USE HIGH GPIO BANK FOR 28

/*
 * you should change the LBAR if you want to match the pciscan dispatched address.
 */
#define	DIVIL_BASE_ADDR		0xB000
#define	SMB_BASE_ADDR		(DIVIL_BASE_ADDR | 0x320)
#define	GPIO_BASE_ADDR		(DIVIL_BASE_ADDR | 0x000)
#define	MFGPT_BASE_ADDR		(DIVIL_BASE_ADDR | 0x280)
#define	PMS_BASE_ADDR		(DIVIL_BASE_ADDR | 0x200)
#define	ACPI_BASE_ADDR		(DIVIL_BASE_ADDR | 0x2c0)

LEAF(superio_init)	

	// set the id select
	li	v0, 0xbfd00000;
	li	v1, PCI_CFG_BASE;
	sw	v1, 0(v0);
	
	// read the DEVICE ID and VENDOR ID for debug, correct is 0x208f1022.
2:
	PCICONF_READW(PCI_IDSEL_CS5536, 0, 0x00);
	
	li	a1, 0x208f1022;
	beq	a0, a1, 55f;
	nop;
	
	b	2b;
	nop;
	
55:

	// set the msr enable
	PCICONF_WRITEW(PCI_IDSEL_CS5536, 0, 0xf0, 0x01);
	
	//
	// MSR ACCESS
	//
	
	// active all the ports
	CS5536_MSR_WRITE((CS5536_GLIU_MSR_BASE | 0x81), 0x0000ffff, 0x0);
	
#ifdef	TEST_CS5536_MSR_ACCESS
	PCICONF_WRITEW(PCI_IDSEL_CS5536, 0, 0xF4, (CS5536_GLIU_MSR_BASE | 0x81)); 
	PCICONF_READW(PCI_IDSEL_CS5536, 0, 0xF8); 
	PCICONF_READW(PCI_IDSEL_CS5536, 0, 0xFC);
#endif

	// GLOBAL CONTROL setting
	CS5536_MSR_WRITE((CS5536_SB_MSR_BASE | 0x10), 0x00000003, 0x44000030);

	//DD LBAR function checking.
	CS5536_MSR_WRITE((CS5536_DIVIL_MSR_BASE | 0x0b), SMB_BASE_ADDR, 0xf001);
	CS5536_MSR_WRITE((CS5536_DIVIL_MSR_BASE | 0x0c), GPIO_BASE_ADDR, 0xf001);
	//CS5536_MSR_WRITE((CS5536_DIVIL_MSR_BASE | 0x0d), MFGPT_BASE_ADDR, 0xf001);
	//CS5536_MSR_WRITE((CS5536_DIVIL_MSR_BASE | 0x0e), ACPI_BASE_ADDR, 0xf001);
	CS5536_MSR_WRITE((CS5536_DIVIL_MSR_BASE | 0x0f), PMS_BASE_ADDR, 0xf001);
	
////////////////////////////////////////////////////////////////////////////////

	//
	// IO SPACE ACCESS
	// 
	
	// enable com1 and place legacy IO to 0x3f8
	// hi : bit+16 = 0 and bit = 1
	// lo : bit+16 = 1 and bit = 0

#if defined(USE_CS5536_UART2) || defined(USE_CS5536_UART1)
	/* config uart1 as 0x3f8, uart2 as 0x2f8 */
	CS5536_MSR_WRITE((CS5536_DIVIL_MSR_BASE | 0x14), 0x04570003, 0x0);
	
	/* config uart1 legacy configuration */
	CS5536_MSR_WRITE((CS5536_DIVIL_MSR_BASE | 0x3a), 0x2, 0x0);
	
	/* config uart1 gpio */
	GPIO_HI_BIT(UART1_TX, GPIO_BASE_ADDR | GPIOL_OUT_EN);
	GPIO_HI_BIT(UART1_TX, GPIO_BASE_ADDR | GPIOL_OUT_AUX1_SEL);
	GPIO_HI_BIT(UART1_TX, GPIO_BASE_ADDR | GPIOL_PU_EN);
	GPIO_LO_BIT(UART1_TX, GPIO_BASE_ADDR | GPIOL_IN_EN);
	GPIO_LO_BIT(UART1_TX, GPIO_BASE_ADDR | GPIOL_OUT_AUX2_SEL);
	GPIO_LO_BIT(UART1_TX, GPIO_BASE_ADDR | GPIOL_IN_AUX1_SEL);

	GPIO_HI_BIT(UART1_RX, GPIO_BASE_ADDR | GPIOL_PU_EN);
	GPIO_HI_BIT(UART1_RX, GPIO_BASE_ADDR | GPIOL_IN_EN);
	GPIO_HI_BIT(UART1_RX, GPIO_BASE_ADDR | GPIOL_IN_AUX1_SEL);
	GPIO_LO_BIT(UART1_RX, GPIO_BASE_ADDR | GPIOL_OUT_EN);
	GPIO_LO_BIT(UART1_RX, GPIO_BASE_ADDR | GPIOL_OUT_AUX2_SEL);
	GPIO_LO_BIT(UART1_RX, GPIO_BASE_ADDR | GPIOL_OUT_AUX1_SEL);

	/* disable the uart2 pin as input for nas board debug. */
	GPIO_HI_BIT(UART2_TX, GPIO_BASE_ADDR | GPIOL_IN_EN);
	GPIO_HI_BIT(UART2_RX, GPIO_BASE_ADDR | GPIOL_IN_EN);
#if 0
	/* config uart2 as 0x3f8 */
	CS5536_MSR_WRITE((CS5536_DIVIL_MSR_BASE | 0x14), 0x04700003, 0x0);
#endif
	/* config uart2 legacy configuration */
	CS5536_MSR_WRITE((CS5536_DIVIL_MSR_BASE | 0x3e), 0x2, 0x0);
	
	/* config uart2 gpio */
	GPIO_HI_BIT(UART2_TX, GPIO_BASE_ADDR | GPIOL_OUT_EN);
	GPIO_LO_BIT(UART2_TX, GPIO_BASE_ADDR | GPIOL_OUT_OD_EN);
	GPIO_HI_BIT(UART2_TX, GPIO_BASE_ADDR | GPIOL_OUT_AUX1_SEL);
	GPIO_LO_BIT(UART2_TX, GPIO_BASE_ADDR | GPIOL_OUT_AUX2_SEL);
//	GPIO_LO_BIT(UART2_TX, GPIO_BASE_ADDR | GPIOL_PU_EN);
//	GPIO_LO_BIT(UART2_TX, GPIO_BASE_ADDR | GPIOL_PD_EN);
	GPIO_LO_BIT(UART2_TX, GPIO_BASE_ADDR | GPIOL_IN_EN);
	GPIO_LO_BIT(UART2_TX, GPIO_BASE_ADDR | GPIOL_IN_AUX1_SEL);

//	GPIO_HI_BIT(UART2_RX, GPIO_BASE_ADDR | GPIOL_PU_EN);
	GPIO_HI_BIT(UART2_RX, GPIO_BASE_ADDR | GPIOL_IN_EN);
	GPIO_HI_BIT(UART2_RX, GPIO_BASE_ADDR | GPIOL_IN_AUX1_SEL);
	GPIO_LO_BIT(UART2_RX, GPIO_BASE_ADDR | GPIOL_OUT_EN);
	GPIO_LO_BIT(UART2_RX, GPIO_BASE_ADDR | GPIOL_OUT_AUX2_SEL);
	GPIO_LO_BIT(UART2_RX, GPIO_BASE_ADDR | GPIOL_OUT_AUX1_SEL);

#endif

	/* fixup the ide and i8259 irq line. */
	/* IDE irq init */
	GPIO_HI_BIT(MIDE_IRQ,	GPIO_BASE_ADDR | GPIOL_IN_EN);
	GPIO_HI_BIT(MIDE_IRQ,	GPIO_BASE_ADDR | GPIOL_IN_AUX1_SEL);
	GPIO_LO_BIT(MIDE_IRQ,	GPIO_BASE_ADDR | GPIOL_OUT_EN);
	GPIO_LO_BIT(MIDE_IRQ,	GPIO_BASE_ADDR | GPIOL_OUT_AUX1_SEL);
	GPIO_LO_BIT(MIDE_IRQ,	GPIO_BASE_ADDR | GPIOL_OUT_AUX2_SEL);

	/* I8259 irq init*/
	GPIO_HI_BIT(I8259_IRQ,	GPIO_BASE_ADDR | GPIOL_OUT_EN);
	GPIO_HI_BIT(I8259_IRQ,	GPIO_BASE_ADDR | GPIOL_OUT_AUX1_SEL);
	GPIO_LO_BIT(I8259_IRQ,	GPIO_BASE_ADDR | GPIOL_OUT_OD_EN);
	GPIO_LO_BIT(I8259_IRQ,	GPIO_BASE_ADDR | GPIOL_OUT_AUX2_SEL);
	GPIO_LO_BIT(I8259_IRQ,	GPIO_BASE_ADDR | GPIOL_IN_EN);
	GPIO_LO_BIT(I8259_IRQ,	GPIO_BASE_ADDR | GPIOL_IN_AUX1_SEL);

	/* PWN_BUT gpio init */
	GPIO_HI_BIT(PWR_BUT,	GPIO_BASE_ADDR | GPIOH_IN_EN);
	GPIO_HI_BIT(PWR_BUT,	GPIO_BASE_ADDR | GPIOH_IN_AUX1_SEL);
	GPIO_LO_BIT(PWR_BUT,	GPIO_BASE_ADDR | GPIOH_OUT_EN);
	GPIO_LO_BIT(PWR_BUT,	GPIO_BASE_ADDR | GPIOH_OUT_AUX1_SEL);
	GPIO_LO_BIT(PWR_BUT,	GPIO_BASE_ADDR | GPIOH_OUT_AUX2_SEL);

	li	v0, 0x00;
	li	v1, (HW_POWER_OFF_TIME << 0);
	or	v0, v1;
	IO_WRITE_DWORD(PMS_BASE_ADDR | 0x40);
	IO_READ_DWORD(PMS_BASE_ADDR | 0x40);
	li	v1, (1 << 30);
	or	v0, v1;
	IO_WRITE_DWORD(PMS_BASE_ADDR | 0x40);
	IO_READ_DWORD(PMS_BASE_ADDR | 0x40);
	li	v1, (1 << 31);
	or	v0, v1;
	IO_WRITE_DWORD(PMS_BASE_ADDR | 0x40);

	/* SMB GPIO init, there are no internal pull-up and pull-down */
	GPIO_HI_BIT(SMB_CLK, GPIO_BASE_ADDR | GPIOL_IN_EN);
	GPIO_HI_BIT(SMB_CLK, GPIO_BASE_ADDR | GPIOL_OUT_EN);
	GPIO_HI_BIT(SMB_CLK, GPIO_BASE_ADDR | GPIOL_IN_AUX1_SEL);
	GPIO_HI_BIT(SMB_CLK, GPIO_BASE_ADDR | GPIOL_OUT_AUX1_SEL);
	
	GPIO_HI_BIT(SMB_DATA, GPIO_BASE_ADDR | GPIOL_IN_EN);
	GPIO_HI_BIT(SMB_DATA, GPIO_BASE_ADDR | GPIOL_OUT_EN);
	GPIO_HI_BIT(SMB_DATA, GPIO_BASE_ADDR | GPIOL_IN_AUX1_SEL);
	GPIO_HI_BIT(SMB_DATA, GPIO_BASE_ADDR | GPIOL_OUT_AUX1_SEL);

	/* SMB initial sequence. */
	/* disable device and config the bus clock */
	li	v0, ((0x3c << 1) & 0xfe);
	IO_WRITE_BYTE(SMB_BASE_ADDR | SMB_CTRL2);

	/* polling mode */
	li	v0, 0x00;
	IO_WRITE_BYTE(SMB_BASE_ADDR | SMB_CTRL1);

	/* Disable slave address, disable slave mode */
	li	v0, 0x0;
	IO_WRITE_BYTE(SMB_BASE_ADDR | SMB_ADDR);

	/* Enable the bus master device */
	IO_READ_BYTE(SMB_BASE_ADDR | SMB_CTRL2);
	ori	v0, SMB_ENABLE;
	IO_WRITE_BYTE(SMB_BASE_ADDR | SMB_CTRL2);

	/* Free STALL after START */
	IO_READ_BYTE(SMB_BASE_ADDR | SMB_CTRL1);
	and	v0, ~(SMB_CTRL1_STASTRE | SMB_CTRL1_NMINTE);
	IO_WRITE_BYTE(SMB_BASE_ADDR | SMB_CTRL1);

	/* Send a STOP */
	IO_READ_BYTE(SMB_BASE_ADDR | SMB_CTRL1);
	ori	v0, SMB_CTRL1_STOP;
	IO_WRITE_BYTE(SMB_BASE_ADDR | SMB_CTRL1);

	/* Clear BER, NEGACK and STASTR bits */
	li	v0, (SMB_STS_BER | SMB_STS_NEGACK | SMB_STS_STASTR);
	IO_WRITE_BYTE(SMB_BASE_ADDR | SMB_STS);

	/* Clear BB bit */
	IO_READ_BYTE(SMB_BASE_ADDR | SMB_CTRL_STS);
	ori	v0, SMB_CSTS_BB;
	IO_WRITE_BYTE(SMB_BASE_ADDR | SMB_CTRL_STS);

#ifdef	TEST_CS5536_GPIO
	GPIO_HI_BIT(UART1_TX, GPIO_BASE_ADDR | GPIOL_OUT_EN);
	GPIO_HI_BIT(UART1_TX, GPIO_BASE_ADDR | GPIOL_PU_EN);
	GPIO_LO_BIT(UART1_TX, GPIO_BASE_ADDR | GPIOL_IN_EN);
	GPIO_LO_BIT(UART1_TX, GPIO_BASE_ADDR | GPIOL_OUT_AUX1_SEL);
	GPIO_LO_BIT(UART1_TX, GPIO_BASE_ADDR | GPIOL_OUT_AUX2_SEL);
	GPIO_LO_BIT(UART1_TX, GPIO_BASE_ADDR | GPIOL_IN_AUX1_SEL);

	GPIO_HI_BIT(UART1_RX, GPIO_BASE_ADDR | GPIOL_OUT_EN);
	GPIO_LO_BIT(UART1_RX, GPIO_BASE_ADDR | GPIOL_IN_EN);
	GPIO_LO_BIT(UART1_RX, GPIO_BASE_ADDR | GPIOL_OUT_AUX1_SEL);
	GPIO_LO_BIT(UART1_RX, GPIO_BASE_ADDR | GPIOL_OUT_AUX2_SEL);
	GPIO_LO_BIT(UART1_RX, GPIO_BASE_ADDR | GPIOL_IN_AUX1_SEL);
	GPIO_HI_BIT(UART1_RX, GPIO_BASE_ADDR | GPIOL_PU_EN);

	GPIO_HI_BIT(UART2_TX, GPIO_BASE_ADDR | GPIOL_OUT_EN);
	GPIO_LO_BIT(UART2_TX, GPIO_BASE_ADDR | GPIOL_IN_EN);
	GPIO_LO_BIT(UART2_TX, GPIO_BASE_ADDR | GPIOL_OUT_AUX1_SEL);
	GPIO_LO_BIT(UART2_TX, GPIO_BASE_ADDR | GPIOL_OUT_AUX2_SEL);
	GPIO_LO_BIT(UART2_TX, GPIO_BASE_ADDR | GPIOL_IN_AUX1_SEL);
	GPIO_HI_BIT(UART2_TX, GPIO_BASE_ADDR | GPIOL_PU_EN);

	GPIO_HI_BIT(UART2_RX, GPIO_BASE_ADDR | GPIOL_OUT_EN);
	GPIO_LO_BIT(UART2_RX, GPIO_BASE_ADDR | GPIOL_IN_EN);
	GPIO_LO_BIT(UART2_RX, GPIO_BASE_ADDR | GPIOL_OUT_AUX1_SEL);
	GPIO_LO_BIT(UART2_RX, GPIO_BASE_ADDR | GPIOL_OUT_AUX2_SEL);
	GPIO_LO_BIT(UART2_RX, GPIO_BASE_ADDR | GPIOL_IN_AUX1_SEL);
	GPIO_HI_BIT(UART2_RX, GPIO_BASE_ADDR | GPIOL_PU_EN);

	GPIO_HI_BIT(I8259_IRQ, GPIO_BASE_ADDR | GPIOL_OUT_EN);
	GPIO_LO_BIT(I8259_IRQ, GPIO_BASE_ADDR | GPIOL_IN_EN);
	GPIO_LO_BIT(I8259_IRQ, GPIO_BASE_ADDR | GPIOL_OUT_AUX1_SEL);
	GPIO_LO_BIT(I8259_IRQ, GPIO_BASE_ADDR | GPIOL_OUT_AUX2_SEL);
	GPIO_LO_BIT(I8259_IRQ, GPIO_BASE_ADDR | GPIOL_IN_AUX1_SEL);
	GPIO_HI_BIT(I8259_IRQ, GPIO_BASE_ADDR | GPIOL_PU_EN);

	GPIO_HI_BIT(GPIO_5, GPIO_BASE_ADDR | GPIOL_OUT_EN);
	GPIO_LO_BIT(GPIO_5, GPIO_BASE_ADDR | GPIOL_IN_EN);
	GPIO_LO_BIT(GPIO_5, GPIO_BASE_ADDR | GPIOL_OUT_AUX1_SEL);
	GPIO_LO_BIT(GPIO_5, GPIO_BASE_ADDR | GPIOL_OUT_AUX2_SEL);
	GPIO_LO_BIT(GPIO_5, GPIO_BASE_ADDR | GPIOL_IN_AUX1_SEL);
	GPIO_HI_BIT(GPIO_5, GPIO_BASE_ADDR | GPIOL_PU_EN);

		GPIO_HI_BIT(MIDE_IRQ, GPIO_BASE_ADDR | GPIOL_OUT_EN);
	GPIO_HI_BIT(MIDE_IRQ, GPIO_BASE_ADDR | GPIOL_PU_EN);
	GPIO_LO_BIT(MIDE_IRQ, GPIO_BASE_ADDR | GPIOL_IN_EN);
	GPIO_LO_BIT(MIDE_IRQ, GPIO_BASE_ADDR | GPIOL_OUT_AUX1_SEL);
	GPIO_LO_BIT(MIDE_IRQ, GPIO_BASE_ADDR | GPIOL_OUT_AUX2_SEL);
	GPIO_LO_BIT(MIDE_IRQ, GPIO_BASE_ADDR | GPIOL_IN_AUX1_SEL);

	GPIO_HI_BIT(SMB_CLK, GPIO_BASE_ADDR | GPIOL_OUT_EN);
	GPIO_LO_BIT(SMB_CLK, GPIO_BASE_ADDR | GPIOL_IN_EN);
	GPIO_LO_BIT(SMB_CLK, GPIO_BASE_ADDR | GPIOL_OUT_AUX1_SEL);
	GPIO_LO_BIT(SMB_CLK, GPIO_BASE_ADDR | GPIOL_OUT_AUX2_SEL);
	GPIO_LO_BIT(SMB_CLK, GPIO_BASE_ADDR | GPIOL_IN_AUX1_SEL);
	GPIO_HI_BIT(SMB_CLK, GPIO_BASE_ADDR | GPIOL_PU_EN);
	
	GPIO_HI_BIT(SMB_DATA, GPIO_BASE_ADDR | GPIOL_OUT_EN);
	GPIO_HI_BIT(SMB_DATA, GPIO_BASE_ADDR | GPIOL_PU_EN);
	GPIO_LO_BIT(SMB_DATA, GPIO_BASE_ADDR | GPIOL_IN_EN);
	GPIO_LO_BIT(SMB_DATA, GPIO_BASE_ADDR | GPIOL_OUT_AUX1_SEL);
	GPIO_LO_BIT(SMB_DATA, GPIO_BASE_ADDR | GPIOL_OUT_AUX2_SEL);
	GPIO_LO_BIT(SMB_DATA, GPIO_BASE_ADDR | GPIOL_IN_AUX1_SEL);


1:
	GPIO_HI_BIT(UART1_TX, GPIO_BASE_ADDR | GPIOL_OUT_VAL);
	GPIO_HI_BIT(UART1_RX, GPIO_BASE_ADDR | GPIOL_OUT_VAL);
	GPIO_HI_BIT(UART2_TX, GPIO_BASE_ADDR | GPIOL_OUT_VAL);
	GPIO_HI_BIT(UART2_RX, GPIO_BASE_ADDR | GPIOL_OUT_VAL);
	GPIO_HI_BIT(I8259_IRQ,  GPIO_BASE_ADDR | GPIOL_OUT_VAL);
	GPIO_HI_BIT(GPIO_5,   GPIO_BASE_ADDR | GPIOL_OUT_VAL);
	GPIO_HI_BIT(MIDE_IRQ,   GPIO_BASE_ADDR | GPIOL_OUT_VAL);
	GPIO_HI_BIT(SMB_CLK,   GPIO_BASE_ADDR | GPIOL_OUT_VAL);
	GPIO_HI_BIT(SMB_DATA,   GPIO_BASE_ADDR | GPIOL_OUT_VAL);
	
	GPIO_LO_BIT(UART1_TX, GPIO_BASE_ADDR | GPIOL_OUT_VAL);
	GPIO_LO_BIT(UART1_RX, GPIO_BASE_ADDR | GPIOL_OUT_VAL);
	GPIO_LO_BIT(UART2_TX, GPIO_BASE_ADDR | GPIOL_OUT_VAL);
	GPIO_LO_BIT(UART2_RX, GPIO_BASE_ADDR | GPIOL_OUT_VAL);
	GPIO_LO_BIT(I8259_IRQ,  GPIO_BASE_ADDR | GPIOL_OUT_VAL);
	GPIO_LO_BIT(GPIO_5,   GPIO_BASE_ADDR | GPIOL_OUT_VAL);
	GPIO_LO_BIT(MIDE_IRQ,   GPIO_BASE_ADDR | GPIOL_OUT_VAL);
	GPIO_LO_BIT(SMB_CLK,   GPIO_BASE_ADDR | GPIOL_OUT_VAL);
	GPIO_LO_BIT(SMB_DATA,   GPIO_BASE_ADDR | GPIOL_OUT_VAL);

	b	1b;
	nop;

#endif

#ifdef	TEST_CS5536_FLASH	// flash test....
	CS5536_MSR_WRITE( (CS5536_DIVIL_MSR_BASE | 0x15), 0x00, 0x00 );
	CS5536_MSR_WRITE( (CS5536_DIVIL_MSR_BASE | 0x10), 0xfffff005, 0x06050000 );
	lui	v1, 0xb605;
	ori	v1, 0x00;
	lw	v0, 0(v1);
	lw	zero, 0(v1);
	nop;
#endif

#ifdef	TEST_CS5536_USB		// USB test...
	CS5536_MSR_WRITE( (CS5536_GLIU_MSR_BASE | 0x21), 0x000fffff, 0x40000005 );
	CS5536_MSR_WRITE( (CS5536_USB_MSR_BASE | 0x0b), 0x05000000, 0x02);
	lui	v1, 0xb500;
	ori	v1, 0x00;
	lw	v0, 0(v1);
	lw	zero, 0(v1);
	nop;
#endif

#ifdef LOONGSON2F_ALLINONE
	GPIO_LO_BIT(GPIO_6, GPIO_BASE_ADDR | GPIOL_OUT_VAL);
	GPIO_HI_BIT(GPIO_6, GPIO_BASE_ADDR | GPIOL_OUT_EN);
	GPIO_LO_BIT(GPIO_6, GPIO_BASE_ADDR | GPIOL_IN_EN);
	GPIO_LO_BIT(GPIO_6, GPIO_BASE_ADDR | GPIOL_OUT_AUX1_SEL);
	GPIO_LO_BIT(GPIO_6, GPIO_BASE_ADDR | GPIOL_OUT_AUX2_SEL);
	GPIO_LO_BIT(GPIO_6, GPIO_BASE_ADDR | GPIOL_IN_AUX1_SEL);
	GPIO_HI_BIT(GPIO_6, GPIO_BASE_ADDR | GPIOL_PU_EN);

	GPIO_HI_BIT(GPIO_7, GPIO_BASE_ADDR | GPIOL_OUT_VAL);
	GPIO_HI_BIT(GPIO_7, GPIO_BASE_ADDR | GPIOL_OUT_EN);
	GPIO_LO_BIT(GPIO_7, GPIO_BASE_ADDR | GPIOL_IN_EN);
	GPIO_LO_BIT(GPIO_7, GPIO_BASE_ADDR | GPIOL_OUT_AUX1_SEL);
	GPIO_LO_BIT(GPIO_7, GPIO_BASE_ADDR | GPIOL_OUT_AUX2_SEL);
	GPIO_LO_BIT(GPIO_7, GPIO_BASE_ADDR | GPIOL_IN_AUX1_SEL);
	GPIO_HI_BIT(GPIO_7, GPIO_BASE_ADDR | GPIOL_PU_EN);

	GPIO_LO_BIT(GPIO_8, GPIO_BASE_ADDR | GPIOL_OUT_VAL);
	GPIO_HI_BIT(GPIO_8, GPIO_BASE_ADDR | GPIOL_OUT_EN);
	GPIO_LO_BIT(GPIO_8, GPIO_BASE_ADDR | GPIOL_IN_EN);
	GPIO_LO_BIT(GPIO_8, GPIO_BASE_ADDR | GPIOL_OUT_AUX1_SEL);
	GPIO_LO_BIT(GPIO_8, GPIO_BASE_ADDR | GPIOL_OUT_AUX2_SEL);
	GPIO_LO_BIT(GPIO_8, GPIO_BASE_ADDR | GPIOL_IN_AUX1_SEL);
	GPIO_HI_BIT(GPIO_8, GPIO_BASE_ADDR | GPIOL_PU_EN);

	GPIO_LO_BIT(GPIO_11, GPIO_BASE_ADDR | GPIOL_OUT_VAL);
	GPIO_HI_BIT(GPIO_11, GPIO_BASE_ADDR | GPIOL_OUT_EN);
	GPIO_LO_BIT(GPIO_11, GPIO_BASE_ADDR | GPIOL_IN_EN);
	GPIO_LO_BIT(GPIO_11, GPIO_BASE_ADDR | GPIOL_OUT_AUX1_SEL);
	GPIO_LO_BIT(GPIO_11, GPIO_BASE_ADDR | GPIOL_OUT_AUX2_SEL);
	GPIO_LO_BIT(GPIO_11, GPIO_BASE_ADDR | GPIOL_IN_AUX1_SEL);
	GPIO_HI_BIT(GPIO_11, GPIO_BASE_ADDR | GPIOL_PU_EN);
#endif

	jr	ra;
	nop;
	
END(superio_init)
	
/************************************************************************
 *									*
 *				SMB					*	
 *									*
 ************************************************************************/
#define	SMBUS_WAIT \
	li	a2, 1000; \
1: \
	li	a3, 0x100; \
2: \
	bnez	a3, 2b;    \
	addiu	a3, -1;	   \
	lui	a0, 0xbfd0;\
	ori	a0, (SMB_BASE_ADDR | SMB_STS); \
	lbu	a1, 0(a0); \
	andi	a1, SMB_STS_SDAST;  \
	li	a3, 0x0;   \
	bnez	a1, 3f;    \
	nop;		   \
	lbu	a1, 0(a0); \
	andi	a1, (SMB_STS_BER | SMB_STS_NEGACK); \
	li	a3, 0x01;  \
	bnez	a1, 3f;    \
	nop;		   \
	addiu	a2, -1;    \
	bnez	a2, 1b;    \
	nop;               \
	li	a3, 0x01;  \
3: \
	nop;
	

/*
 * a0 : slave address, 0xa0 for spd
 * a1 : slave device register index
 * v0 : the returned register value.
 */

LEAF(i2cread)
	andi	a0, 0xfe;
	move	t2, a0;
	move	t3, a1;
	
	/* start condition */
	IO_READ_BYTE(SMB_BASE_ADDR | SMB_CTRL1);
	ori	v0, SMB_CTRL1_START;
	IO_WRITE_BYTE(SMB_BASE_ADDR | SMB_CTRL1);
	IO_READ_BYTE(SMB_BASE_ADDR | SMB_STS);
	andi	v0, SMB_STS_BER;
	bnez	v0, i2cerr;
	nop;
	SMBUS_WAIT;
	bnez	a3, i2cerr;
	nop;
	
	/* send slave address */
	move	v0, t2;
	IO_WRITE_BYTE(SMB_BASE_ADDR | SMB_SDA);
	IO_READ_BYTE(SMB_BASE_ADDR | SMB_STS);
	andi	v0, (SMB_STS_BER | SMB_STS_NEGACK);
	bnez	v0, i2cerr;
	nop;
	SMBUS_WAIT;
	bnez	a3, i2cerr;
	nop;
		
	/* acknowledge smbus */
	IO_READ_BYTE(SMB_BASE_ADDR | SMB_CTRL1);
	ori	v0, (SMB_CTRL1_ACK);
	IO_WRITE_BYTE(SMB_BASE_ADDR | SMB_CTRL1);
	
	/* send command */
	move	v0, t3;
	IO_WRITE_BYTE(SMB_BASE_ADDR | SMB_SDA);
	IO_READ_BYTE(SMB_BASE_ADDR | SMB_STS);
	andi	v0, (SMB_STS_BER | SMB_STS_NEGACK);
	bnez	v0, i2cerr;
	nop;
	SMBUS_WAIT;
	bnez	a3, i2cerr;
	nop;
		
	/* start condition again */
	IO_READ_BYTE(SMB_BASE_ADDR | SMB_CTRL1);
	ori	v0, SMB_CTRL1_START;
	IO_WRITE_BYTE(SMB_BASE_ADDR | SMB_CTRL1);
	IO_READ_BYTE(SMB_BASE_ADDR | SMB_STS);
	andi	v0, SMB_STS_BER;
	bnez	v0, i2cerr;
	nop;
	SMBUS_WAIT;
	bnez	a3, i2cerr;
	nop;
	
	/* send salve address again */
	move	v0, t2;
	ori	v0, 0x01;
	IO_WRITE_BYTE(SMB_BASE_ADDR | SMB_SDA);
	IO_READ_BYTE(SMB_BASE_ADDR | SMB_STS);
	andi	v0, (SMB_STS_BER | SMB_STS_NEGACK);
	bnez	v0, i2cerr;
	nop;
	SMBUS_WAIT;
	bnez	a3, i2cerr;
	nop;
		
	/* stop condition */
	li	v0, SMB_CTRL1_STOP;
	IO_WRITE_BYTE(SMB_BASE_ADDR | SMB_CTRL1);
	SMBUS_WAIT;
	bnez	a3, i2cerr;
	nop;
	
	/* read data */
	IO_READ_BYTE(SMB_BASE_ADDR | SMB_SDA);

i2cerr :
	nop;
	
	jr	ra;
	nop;
END(i2cread)

/*************************************************************************
 *									 *
 *				DDR2					 *
 *									 *
 *************************************************************************/

#define REG_ADDRESS 0x0
#define CONFIG_BASE 0xaffffe00
    

        .global ddr2_config
        .ent    ddr2_config
        .set    noreorder
        .set    mips3
ddr2_config:
    move s1, ra
#if 1
	la      t0, ddr2_reg_data
#endif
1:	
	addu    t0, t0, s0
	li      t1, 0x1d
	li      t2, CONFIG_BASE

reg_write:

	ld      a1, 0x0(t0)
	sd	a1, REG_ADDRESS(t2)
	subu	t1, t1, 0x1
	addiu	t0, t0, 0x8
	bne	t1, $0, reg_write
	addiu	t2, t2, 0x10

    
    //Felix-2008-10-13
    //Read the DDR2 spd regs
    #define QYL_SPD_DEBUG
QYL_NEXT2:
    #ifndef QYL_SPD_DEBUG
    PRINTSTR("Read the DDR2 spd regs:\r\n")
    li  a1, 0x0
    li  s3, 0x80
spd_reg_read:
    li  a0, 0xa0
    move t8, a1
    bal i2cread
    nop 
    move a0, v0
    bal hexserial
    nop 
    li  a0, ' '
    bal tgt_putchar
    nop 
    addu t8,t8,0x1
    move a1, t8
    subu s3,s3,0x1
    bne s3,$0,spd_reg_read
    nop    
    #endif

    
#if 1
	move k0, ra
	/* read spd to initialize memory controller */
	li a0, 0xa0
	li a1, 3    //Row Address
	bal i2cread
	nop
	li v1, 15
	sub v1, v0
	and v1, 0x7
	sll v1, 0x8
	li  v0, 0xaffffe50
	lw  a1, (v0)
	and a1, 0xfffff8ff
	or  a1, v1
	sw  a1, (v0)

	li  a0, 0xa0
	li  a1, 0x04 // Column Address
	bal i2cread
	nop
	li v1, 14
	sub v1, v0
	sll v1, 24
	li  v0, 0xaffffe50
	lw  a1, (v0)
	and a1, 0xffffff
	or  a1, v1
	sw  a1, (v0)

	/* number of CS */
	li a0, 0xa0
	li a1, 5
	bal i2cread
	nop
	li v1, 0x7
	and v1, v0
	add v1, 1
	li  a0, 1
	sll a0, v1
	sub a0, 1
	li v0, 0xaffffe70
	lw v1, 0(v0)
	nop 
	sll a0, a0, 16
	li  t0, 0xfff0ffff
	and v1, v1, t0
	or  v1, v1, a0
	sw  v1, 0(v0)
	nop

	//Config CS* ODT 
	li a0,0xa0
	li a1, 5
	bal i2cread 
	nop 
	li v1, 0x7
	and v1, v0
	bgtz v1, 1f   // have 1 CS(default cs0)
	nop
	li v0, 0xaffffe80
	li t0, 0x00000001
	sw t0, 0(v0)
	nop 
	sw t0, 4(v0)
	nop 
	b 5f
	nop
1:  sub v1, 1   // have 2 CS(default cs0 cs1)
	bgtz v1, 2f
	nop
	li v0, 0xaffffe80
	li t0, 0x00000102
	sw t0, 0(v0)
	nop 
	sw t0, 4(v0)
	nop	
	b 5f
	nop
2:  sub v1, 1 // have 3 CS (default cs0 cs 1 cs 2)
	bgtz v1, 3f
	nop
	li v0, 0xaffffe80
	li t0, 0x00010402
	sw t0, 0(v0)
	nop 
	sw t0, 4(v0)
	nop	
	b 5f
	nop
3:  sub v1, 1 // have 4 CS (default cs0 cs 1 cs 2 cs 3)
	bgtz v1, 4f
	nop
	li v0, 0xaffffe80
	li t0, 0x01080402
	sw t0, 0(v0)
	nop 
	sw t0, 4(v0)
	nop	
	b 5f
	nop
4:	li v0, 0xaffffe80 //More than 4 CS, not supported till now, give a default value
	li t0, 0x00000000
	sw t0, 0(v0)
	nop 
	sw t0, 4(v0)
	nop	          
5:
    PRINTSTR("Read the DDR2 No8 reg:\r\n")
	li v0, 0xaffffe80
	lw a0, 0(v0)
	bal hexserial
	nop
    li v0, 0xaffffe80
    lw a0, 4(v0)
    bal hexserial
    nop
	//Config Unbuffered bit *fix me*
	//Note:during debuging,we found that if there are more than 8 (exam. 16) chips on a module,
	//Set Unbuffered bit will make it work,if not set, the module even can't work.
	li a0, 0xa0
	li a1, 5
	bal i2cread 
	nop 
	li s3, 0x7
	and s3, v0
	li a0, 0xa0
	li a1, 13
	bal i2cread
	nop
	move t0, v0
	bgtz s3, 1f // 1 cs, only (64/4*1=16 need to set)
	nop 
	sub t0, 4
	bgtz t0, NOT_SET_UNBUFFERED
	nop
	b SET_UNBUFFERED
	nop
1:  sub s3, 1 // 2cs, (64/4*2=32, 64/8*2=16 need to set)
	bgtz s3, 2f 
	nop 	
	sub t0, 8
    bgtz t0, NOT_SET_UNBUFFERED
    nop
    b SET_UNBUFFERED
    nop
2:  sub s3, 1 // 3cs, (64/4*3=48, 64/8*3=24 64/16*3=12 need to set)
	bgtz s3, 3f 
	nop 	
	sub t0, 16
    bgtz t0, NOT_SET_UNBUFFERED
    nop
    b SET_UNBUFFERED
    nop
3:  sub s3, 1 // 4cs, (64/4*4=64, 64/8*4=32 64/16*4=16 need to set,in fact,it  can be integrated to the pre condition)
	bgtz s3, NOT_SET_UNBUFFERED //More than 4cs ,not suported, give a default value,not set
	nop 	
	sub t0, 16
    bgtz t0, NOT_SET_UNBUFFERED
    nop
    //b SET_UNBUFFERED
SET_UNBUFFERED:
	li v0, 0xafffffc0
	li t0, 0x00000001
	sw t0, 4(v0)
	nop
	b 5f
	nop
NOT_SET_UNBUFFERED:
	li v0, 0xafffffc0
	li t0, 0x00000000
	sw t0, 4(v0)
	nop
5:   
    PRINTSTR("Read the DDR2 No28 reg:\r\n")
	li v0, 0xafffffc0
	lw a0, 4(v0)
	bal hexserial
	nop
	
    //Felix-2008-10-27
    li  v0, 0xaffffe70
    lw  a0, 0(v0)
    nop
    bal hexserial
    nop
    
	/* Number of banks */
	li  a0, 0xa0 
	li  a1, 0x11 //Bank 
	bal i2cread
	nop 
	//li  v1, 0x4
	//beq v1, v0, 1f
	li  v1, 0x8
	bne v1, v0, 1f	
	nop
	li 	v1, 0x1
	li  v0, 0xaffffe10
	sw  v1, 4(v0) 
	nop


1:	
	//move t7,k0
    #ifndef QYL_DDR2_DEBUG
    PRINTSTR("Read the DDR2 regs:\r\n")
    li  t7, 0xaffffe00
    li  s3, 29
ddr2_reg_read:
    lw  a0, 0(t7)
    nop
    bal hexserial
    nop 
    li  a0, ' '
    bal tgt_putchar
    nop 
    lw  a0, 4(t7)
    nop
    bal hexserial
    nop 
    li  a0, ' '
    bal tgt_putchar
    nop     
    addu t7,t7,0x10
    subu s3,s3,0x1
    bne s3,$0,ddr2_reg_read
    nop    
    #endif

	/*nop
    PRINTSTR("Read the DDR2 No28 reg:\r\n")*/
	//move k0, t7
	move ra, k0
#endif

    ############start##########
	li      t2, CONFIG_BASE
	la	t0,DDR2_CTL_start_DATA_LO
	#la	t0,ddr2_start_reg
	addu   	t0, t0, s0
	ld      a1, 0x0(t0)
	sd      a1, 0x30(t2)


	move    ra, s1
	jr      ra
	nop
    .set    mips0
    .end    ddr2_config



/* expanation of "old" is the reserved configuration data for 2f motherboard. */
	.rdata
	.align 5
ddr2_reg_data:
DDR2_CTL_00_DATA_LO: .word 0x00000101
DDR2_CTL_00_DATA_HI: .word 0x01000100 #no_concurrentap
DDR2_CTL_01_DATA_LO: .word 0x00010000
DDR2_CTL_01_DATA_HI: .word 0x00000000
DDR2_CTL_02_DATA_LO: .word 0x00000000
DDR2_CTL_02_DATA_HI: .word 0x01000101
DDR2_CTL_03_DATA_LO: .word 0x01000000
DDR2_CTL_03_DATA_HI: .word 0x01010000
//Felix-2008-1020
DDR2_CTL_04_DATA_LO: .word 0x01010101
//DDR2_CTL_04_DATA_LO: .word 0x00010101
DDR2_CTL_04_DATA_HI: .word 0x01000202
DDR2_CTL_05_DATA_LO: .word 0x04050202 
DDR2_CTL_05_DATA_HI: .word 0x00000000
DDR2_CTL_06_DATA_LO: .word 0x03050203 #800
DDR2_CTL_06_DATA_HI: .word 0x0a040306 #800
DDR2_CTL_07_DATA_LO: .word 0x00030a0b #CL=5//cs_map to cs0-cs3 
DDR2_CTL_07_DATA_HI: .word 0x00000400 #800
//Felix-2008-10-27
//DDR2_CTL_08_DATA_LO: .word 0x01020408
//DDR2_CTL_08_DATA_HI: .word 0x01020408
//DDR2_CTL_08_DATA_LO: .word 0x08040201
//DDR2_CTL_08_DATA_HI: .word 0x08040201
//Felix-2008-12-11
//DDR2_CTL_08_DATA_LO: .word 0x00000102
//DDR2_CTL_08_DATA_HI: .word 0x00000102
DDR2_CTL_08_DATA_LO: .word 0x00000001
DDR2_CTL_08_DATA_HI: .word 0x00000001
DDR2_CTL_09_DATA_LO: .word 0x00000000
DDR2_CTL_09_DATA_HI: .word 0x0000060c #800
//Felix-2008-10-20
DDR2_CTL_10_DATA_LO: .word 0x3f1f0200 #800
//DDR2_CTL_10_DATA_LO: .word 0x3f1a021b #800
//Felix-2008-10-28
//Felix-2008-12-11
DDR2_CTL_10_DATA_HI: .word 0x2323233f //yanhua
DDR2_CTL_11_DATA_LO: .word 0x23232323 //yanhua
DDR2_CTL_11_DATA_HI: .word 0x5f7f2323 //yanhua
//DDR2_CTL_10_DATA_HI: .word 0x1818183f //yanhua
//DDR2_CTL_11_DATA_LO: .word 0x18181818 //yanhua
//DDR2_CTL_11_DATA_HI: .word 0x5f7f1818 //yanhua
//DDR2_CTL_10_DATA_HI: .word 0x1515153f //yanhua
//DDR2_CTL_11_DATA_LO: .word 0x15151515 //yanhua
//DDR2_CTL_11_DATA_HI: .word 0x5f7f1515 //yanhua
//Felix-2008-10-20
DDR2_CTL_12_DATA_LO: .word 0x15000000 #800
//DDR2_CTL_12_DATA_LO: .word 0x25000000 #800
DDR2_CTL_12_DATA_HI: .word 0x002a3c06 #800
DDR2_CTL_13_DATA_LO: .word 0x002a002a
DDR2_CTL_13_DATA_HI: .word 0x002a002a
DDR2_CTL_14_DATA_LO: .word 0x002a002a
DDR2_CTL_14_DATA_HI: .word 0x002a002a
DDR2_CTL_15_DATA_LO: .word 0x00000004
DDR2_CTL_15_DATA_HI: .word 0x00b40020
DDR2_CTL_16_DATA_LO: .word 0x00000087
DDR2_CTL_16_DATA_HI: .word 0x000007ff #no_interrupt
//Felix-2008-10-20
//Felix-2008-12-11
//DDR2_CTL_17_DATA_LO: .word 0x0016101b #800
DDR2_CTL_17_DATA_LO: .word 0x0016101f #800
//DDR2_CTL_17_DATA_LO: .word 0x0004101b #800
DDR2_CTL_17_DATA_HI: .word 0x00000000
DDR2_CTL_18_DATA_LO: .word 0x00000000
DDR2_CTL_18_DATA_HI: .word 0x001c0000
DDR2_CTL_19_DATA_LO: .word 0x00c8006b
//Felix-2008-10-20
DDR2_CTL_19_DATA_HI: .word 0x28e10002 #800
//DDR2_CTL_19_DATA_HI: .word 0x68e10002 #800
DDR2_CTL_20_DATA_LO: .word 0x00c8002f #800
DDR2_CTL_20_DATA_HI: .word 0x00000000
DDR2_CTL_21_DATA_LO: .word 0x00030d40 #real
DDR2_CTL_21_DATA_HI: .word 0x00000000
DDR2_CTL_22_DATA_LO: .word 0x00000000
DDR2_CTL_22_DATA_HI: .word 0x00000000
DDR2_CTL_23_DATA_LO: .word 0x00000000
DDR2_CTL_23_DATA_HI: .word 0x00000000
DDR2_CTL_24_DATA_LO: .word 0x00000000
DDR2_CTL_24_DATA_HI: .word 0x00000000
DDR2_CTL_25_DATA_LO: .word 0x00000000
DDR2_CTL_25_DATA_HI: .word 0x00000000
DDR2_CTL_26_DATA_LO: .word 0x00000000
DDR2_CTL_26_DATA_HI: .word 0x00000000
DDR2_CTL_27_DATA_LO: .word 0x00000000
DDR2_CTL_27_DATA_HI: .word 0x00000000
DDR2_CTL_28_DATA_LO: .word 0x00000000
DDR2_CTL_28_DATA_HI: .word 0x00000000
DDR2_CTL_start_DATA_LO: .word 0x01000000
DDR2_CTL_start_DATA_HI: .word 0x01010100

