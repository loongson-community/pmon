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
#include "target/prid.h"
#include "target/sbd.h"
#include "target/bonito.h"
#include "target/i8254.h"

#include "target/ls1a.h"

#include "target/isapnpreg.h"
#define DEBUG_LOCORE
#undef	DEBUG_DIMM_SPD
#ifdef DEBUG_LOCORE
#define	TTYDBG(x) \
	.rdata;98: .asciz x; .text; la a0, 98b; bal stringserial; nop
#else
#define TTYDBG(x)
#endif
#define	PRINTSTR(x) \
	.rdata;98: .asciz x; .text; la a0, 98b; bal stringserial; nop

#define GPIOLED_DIR  0x1
#if 0 
#define GPIO_RESET_DELAY(x,y) \
li v0,0xbfe0011c; \
lw v1,4(v0); \
or v1,0x1; \
xor v1,GPIOLED_DIR; \
sw v1,4(v0); \
lw v1,0(v0); \
and v1,~(0x1); \
or v1,x;\
sw v1,0(v0);\
li v1,y ;\
78: \
bnez v1,78b;\
subu v1,1;\
nop;
#else
#define GPIO_RESET_DELAY(x,y) \
li v0,0xbfe0011c; \
lw v1,0(v0); \
and v1,~(0x1); \
or v1,x;\
sw v1,0(v0);\
lw v1,4(v0); \
or v1,0x1; \
xor v1,GPIOLED_DIR; \
sw v1,4(v0); \
li v1,y ;\
78: \
bnez v1,78b;\
subu v1,1;\
nop;
#endif
#define	GPIOLED_DIR1 0x6
#if 0 
#define GPIO_RESET_DELAY1(x,y) \
li v0,0xbfe0011c; \
lw v1,4(v0); \
or v1,0x6; \
xor v1,GPIOLED_DIR1; \
sw v1,4(v0); \
lw v1,0(v0); \
and v1,~(0x6); \
or v1,x;\
sw v1,0(v0);\
li v1,y ;\
78: \
bnez v1,78b;\
subu v1,1;\
nop;
#else
#define GPIO_RESET_DELAY1(x,y) \
li v0,0xbfe0011c; \
lw v1,0(v0); \
and v1,~(0x6); \
or v1,x;\
sw v1,0(v0);\
lw v1,4(v0); \
or v1,0x6; \
xor v1,GPIOLED_DIR1; \
sw v1,4(v0); \
li v1,y ;\
78: \
bnez v1,78b;\
subu v1,1;\
nop;
#endif
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

#define	DDR100 0x1d441091 
/* 
#define	DDR100 0x0c011091*/
#define	DDR266 0x0410435e
#define DDR300 0x041453df

#define PCI_IDSEL_1A    9

#define NORTHBRIDGE_INIT(r,v) \
	li      t0,r;\
	li      t1,v;\
	sw      t1,0x0(t0);
#define SOUTHBRIDGE_INIT(r,v) NORTHBRIDGE_INIT(r,v)

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


	.set	noreorder
	.globl	_start
	.globl	start
	.globl	__main
_start:
start:
	.globl	stack
stack = start - 0x4000		/* Place PMON stack below PMON start in RAM */

	/* init processor state at first*/
/* NOTE!! Not more that 16 instructions here!!! Right now it's FULL! */
	mtc0	zero, COP_0_STATUS_REG //cuckoo
	mtc0	zero, COP_0_CAUSE_REG
	li	t0, SR_BOOT_EXC_VEC	/* Exception to Boostrap Location */
	mtc0	t0, COP_0_STATUS_REG //cuckoo
#if	0
	la	sp, stack
	la	gp, _gp
#else
        bal     initregs
        nop
#endif

//	bal	uncached		/* Switch to uncached address space */
	nop

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

/* Debug exception */
	.align	7
	#include "exc_ejtag.S"
	
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
    b   1b
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
        #move    $31, $0 // ra

	/* now begin initial float CP1 register */
        ## float point

        mfc0    t0, CP0_STATUS
        li     t1, 0x64000000|SR_KX|SR_SX|SR_UX|SR_BOOT_EXC_VEC      # {cu3,cu2,cu1,cu0}<={0110, status_fr<=1,0xe0 to enable 64bit space
        or      t0, t0, t1
        mtc0    t0, CP0_STATUS
	la       sp, stack
	la       gp, _gp
        jr       ra
        nop

/*
 *  We get here from executing a bal to get the PC value of the current execute
 *  location into ra. Check to see if we run from ROM or if this is ramloaded.
 */
locate:

	la	s0,start
	subu	s0,ra,s0
	and	s0,0xffff0000


reset2:  GPIO_RESET_DELAY(0,0x40000) //set pci_rest to 0 200ms
reset3:  GPIO_RESET_DELAY(1,0x100)  //set pci_reset to 1 100ms

        mfc0    t0, CP0_STATUS
        li     t1, 0x64000000|SR_KX|SR_SX|SR_UX|SR_BOOT_EXC_VEC      # {cu3,cu2,cu1,cu0}<={0110, status_fr<=1,0xe0 to enable 64bit space
        or      t0, t0, t1
        mtc0    t0, CP0_STATUS
        mtc0    zero,COP_0_CAUSE_REG
	.set noreorder /* ? what it means? tell compiler not reorder these instruction*/

 	li	bonito,PHYS_TO_UNCACHED(BONITO_REG_BASE)


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
#define	PCI_DEV_I82371			17
#define PCI_CFG_SPACE			BONITO_PCICFG_BASE

        BONITO_INIT(BONITO_PCICLASS,(PCI_CLASS_BRIDGE << PCI_CLASS_SHIFT) | (PCI_SUBCLASS_BRIDGE_HOST << PCI_SUBCLASS_SHIFT))
	BONITO_INIT(BONITO_PCICMD, BONITO_PCICMD_PERR_CLR|BONITO_PCICMD_SERR_CLR|BONITO_PCICMD_MABORT_CLR|BONITO_PCICMD_MTABORT_CLR|BONITO_PCICMD_TABORT_CLR|BONITO_PCICMD_MPERR_CLR)
	BONITO_INIT(BONITO_PCILTIMER, 0)
	BONITO_INIT(BONITO_PCIBASE0, 0)
	BONITO_INIT(BONITO_PCIBASE1, 0)
	BONITO_INIT(BONITO_PCIBASE2, 0)
	BONITO_INIT(BONITO_PCIEXPRBASE, 0)
	BONITO_INIT(BONITO_PCIINT, 0)
   
        BONITO_INIT(0x150,0x8000000c)
        BONITO_INIT(0x154,0xffffffff)
	
	BONITO_BIS(BONITO_PCICMD, BONITO_PCICMD_PERRRESPEN)
	
	BONITO_BIS(BONITO_PCICMD, PCI_COMMAND_IO_ENABLE|PCI_COMMAND_MEM_ENABLE|PCI_COMMAND_MASTER_ENABLE)
	
                                                                        
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

	bal	initserial
	nop

//Read sys_clk_sel
#if 0
	TTYDBG ("\r\n0xbfe00180  : ")
        li   t2,0xbfe00180
        ld   t1, 0x0(t2)
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
#endif    

	PRINTSTR("\r\nPMON2000 MIPS Initializing. Standby...\r\n")

#if 0
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
#endif

.set mips3
	bal init_ls2g_xbar_config
	nop
PRINTSTR("init ls2g xbar config  done\r\n")

	bal init_south_bridge
	nop
PRINTSTR("init ls2g south bridge done\r\n")
	
	bal ddr2_config_1A
	nop
PRINTSTR("init ddr2 config for 1a  done\r\n")

SOUTHBRIDGE_INIT(0xb2d00190,0x1c2000f2) //change back  ddrcofig ->GPU/DC

	nop
	bal	initserial_1a
	nop

	bnez s0,1f
	nop
	li a0,128
	la v0,initmips
	jr v0
	nop
1:

#if 0
        //CUCKOO PCI ARBITOR config
        li t2,0xbfe00168
        lw a1,0x0(t2)
        ori a1,a1,0x2
        sw a1,0x0(t2)
#endif 

#if 1 //by_css
	TTYDBG("godson2 caches found\r\n")
        bal     godson2_cache_init
        nop
	TTYDBG("scache init\r\n") 
        bal	scache_init // smh
        nop
#endif

        mfc0   a0,COP_0_CONFIG     /* enable kseg0 cachability */
#if 1
        and    a0,a0,0xfffffff8
        or     a0,a0,0x3           // ENABLE
#else
        and    a0,a0,0xfffffff8
        or     a0,a0,0x2           // DISABLE
#endif
        mtc0   a0,COP_0_CONFIG

#if 1 /* jmp to 0x9fc... */
        lui     t0, 0xdfff ####################### go to 9fc
        //lui     t0, 0xffff                   // don't go to...
        ori     t0, t0, 0xffff
        bal     1f
        nop     
1:
        and     ra, ra, t0
        addiu   ra, ra, 16
        jr      ra      
        nop 
#endif

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
	dli     s1, 0xff100004
#else
	dli     s1, 0x82a48104  //MC0 only
#endif
#include "ddr_dir/loongson2G_ddr2_config.S"

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

	dli     s1, 0x0008000080000000  //NODE 0, start from 0x80000000
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


##########################################

#include "machine/newtest/newdebug.S"

##########################################
bootnow:

//#define GODSON3_SIM
#ifdef GODSON3_SIM
	TTYDBG("bypass Copy PMON to execute location...\r\n")
#else
	TTYDBG("Copy PMON to execute location...\r\n")
//#define DEBUG_LOCORE
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
        //SMH or      a0, 0xa0000000	//by_css
        //SMH or      a2, 0xa0000000
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


3:

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

#endif /* sim bypass copy */
/* cxk */
#if 0
	TTYDBG("Debug Testing...\r\n")
	la  	a0, start
	li	    a1, 0xbfc00000
	la  	a2, _edata
    //or      a0, 0xa0000000
    //or      a2, 0xa0000000

	move	t0, a0
	move	t1, a1
	move	t2, a2
	/* copy text section */
	
    #addu	t0, 64  
	#addu	t1, 64 
    li      t3, 128  //loop counter

1:	
    lw	    t4, 0(t1)
	nop
	lw	    t5, 0(t0)
	nop
    
	TTYDBG	("\r\n")
	move	a0, t0
	bal	    hexserial
	nop
	TTYDBG	(" ")
	move	a0, t4
	bal	    hexserial
	nop
	TTYDBG	(" ")
	move	a0, t5
	bal	    hexserial
	nop
	beq	    t4, t5, 2f
    nop
    /* reread the wrong byte*/
    TTYDBG  (" ")
    lw      t4, 0(t0)
    nop
    move    a0, t4
    bal     hexserial
    nop
    /* if the reread value differs the first read, print mark */
    beq     t4, t5, 2f
    nop
    TTYDBG  ("  DDD")
2:	
	addu	t0, 4
	addu	t1, 4
    subu    t3, 1
#if 0
    li      v0, 0xff
3:
    daddi   v0, -1
    bnez    v0, 3b
    nop
#endif
    bnez 	t3, 1b
	nop
	TTYDBG	("\r\nDebug Testing done!\r\n")
#endif
/*******************/

	move	a0,msize

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
	bal	tgt_putchar
	lbu	a0, 0(v0)

	bnez	a3, 1b
	addu	a3, -1

	j	a2
	nop
END(hexserial)



__main:
	j	ra
	nop
#include "ddr2_configs_for_1a.S"

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
//!!!!!!!!!SMH#define Index_Store_Tag_S			0x09 
#define Index_Writeback_Inv_S			0x01//delete scache  

#define Index_Store_Tag_S			0x0B 

LEAF(nullfunction)
	jr ra
	nop
END(nullfunction)

#define CP0_ECC  $26
LEAF(scache_init)
#        daddi   sp, sp, 0xfff8
#        sd      ra, 0(sp)
	move	t7, ra
#if 0 /* gx 2G */  //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! SMH
        .word 0x40028001 #mfc0    v0,c0_config1
        and     v0, 0xf 
        beqz    v0, 1f  
        nop     
        jr      ra      
        nop     
1:
#endif

        lui     a0, 0x8000
        #lui     a2, 0x0002      #512k/4way
        lui     a2, 0x0004      #1M/4way
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
scache_init_finish:
	TTYDBG	("\r\nscache init ok\r\n")
        jr      t7
	nop
scache_init_panic:
	TTYDBG	("\r\nscache init panic\r\n")
1:      b       1b
        nop
END(scache_init)
###############################
LEAF(hexserial64)
  move t7,ra
  move a1,a0
  dsrl a0,32
  bal hexserial
  nop
  move a0,a1
  bal hexserial
  nop
  jr t7
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

/********************************************************************************
 *										*
 *	      			      PCI OPTION				*
 *										*
 ********************************************************************************/
#define PCICONF_WRITEB(dev,func,reg,data) \
	li      a0,CFGADDR(dev,func,reg); \
	li      a1,PHYS_TO_UNCACHED(PCI_CFG_SPACE); \
	and     a2,a0,0xffff; \
	or      a1,a2; \
	srl     a0,16; \
	li      a2,BONITO_BASE+BONITO_PCIMAP_CFG; \
	sw      a0,BONITO_PCIMAP_CFG(bonito); \
	lw      zero,BONITO_PCIMAP_CFG(bonito); \
	or      a0,zero,data; \
	sb      a0,(a1);

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
#ifdef NVRAM_IN_FLASH
    li a1,(0xbfc00000+NVRAM_OFFS+3)
    add a1,a0
    addu a0,a1,-3
    move v1,zero
	1:  
    lbu v0,(a1)
    sll v1,8
    or v1,v0
    bne a0,a1,1b
    addu a1,-1
#else
    li a1,RTC_NVRAM_BASE+3
    add a1,a0
	addu a0,a1,-3
	move v1,zero
	1:
	sb a1,(PHYS_TO_UNCACHED(PCI_IO_SPACE+RTC_INDEX_REG))
	lbu v0,(PHYS_TO_UNCACHED(PCI_IO_SPACE+RTC_DATA_REG))
	sll v1,v1,8
	or v1,v0
	bne a1,a0,1b
	addiu a1,-1
#endif
	move v0,v1
	jr ra
	nop
END(get_userenv)

LEAF(initserial)
		.set noat
		move AT,ra

        la      v0, COM1_BASE_ADDR
		li		a0,(33300000/ 16)/CONS_BAUD
		nop
		bal  1f
		nop

		jr AT
		nop
1:
        li      v1, FIFO_ENABLE|FIFO_RCV_RST|FIFO_XMT_RST|FIFO_TRIGGER_4
        sb      v1, NSREG(NS16550_FIFO)(v0)
        li      v1, CFCR_DLAB                  #DLAB
        sb      v1, NSREG(NS16550_CFCR)(v0)
        sb      a0, NSREG(NS16550_DATA)(v0)
        srl     a0, 8
        sb      a0, NSREG(NS16550_IER)(v0)     #set BRDH
        li      v1, CFCR_8BITS                 #8bit
        sb      v1, NSREG(NS16550_CFCR)(v0)
        li      v1, MCR_DTR|MCR_RTS
        sb      v1, NSREG(NS16550_MCR)(v0)
        li      v1, 0x0
        sb      v1, NSREG(NS16550_IER)(v0)

        j       ra
        nop
		.set at
END(initserial)

LEAF(initserial_1a)
	.set noat
	move AT,ra
	la	v0, COM0_1A_BASE_ADDR
	li	v1, FIFO_ENABLE|FIFO_RCV_RST|FIFO_XMT_RST|FIFO_TRIGGER_4
	sb	v1, NSREG(NS16550_FIFO_1A)(v0)
	li	v1, CFCR_DLAB
	sb	v1, NSREG(NS16550_CFCR_1A)(v0)
	li	v1, ((APB_CLK*5)/(2*16*CONS_BAUD))
	sb	v1, NSREG(NS16550_DATA_1A)(v0)
	srl	v1, 8
	sb	v1, NSREG(NS16550_IER_1A)(v0)
	li	v1, CFCR_8BITS
	sb	v1, NSREG(NS16550_CFCR_1A)(v0)
	li	v1, MCR_DTR|MCR_RTS
	sb	v1, NSREG(NS16550_MCR_1A)(v0)
	li	v1, 0x0
	sb	v1, NSREG(NS16550_IER_1A)(v0)
	li	v1, 0x0
	sb	v1, NSREG(NS16550_IER_1A)(v0)
	j	ra
	nop
	.set at
END(initserial_1a)

LEAF(_w83627_write)
	li	v0,0xbfd0002e
	li	v1,0xbfd0002f
	li	t0,0x87
	sb	t0,0(v0)
	nop
	sb	t0,0(v0)
	nop
	li	t0,0x7
	sb	t0,0(v0)
	nop
	move 	t0,a0
	sb	t0,0(v1)
	nop
	move	t0,a1
	sb	t0,0(v0)
	nop
	move 	t0,a2
	sb	t0,0(v1)
	nop
	li	t0,0xaa
	sb	t0,0(v0)
	nop
	sb	t0,0(v0)
	nop

	jr	ra
	nop
	
END(_w83627_write)

LEAF(tgt_putchar)
	.set noat
    move AT,ra
    la  v0, COM1_BASE_ADDR
    bal 1f
    nop

    jr AT
    nop
  
1:
    lbu v1, NSREG(NS16550_LSR)(v0)
    and v1, LSR_TXRDY
    beqz    v1, 1b
    nop
						 
    sb  a0, NSREG(NS16550_DATA)(v0)
    j   ra
    nop
	.set at
END(tgt_putchar)

.text


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

LEAF(init_south_bridge)
.set at
	move a3,ra	//save ra
	/* first config north bridge*/
	li      t0, CFGADDR(PCI_IDSEL_1A,0,0)
	srl     t0, 16
	sw      t0, PHYS_TO_UNCACHED(BONITO_BASE+BONITO_PCIMAP_CFG)
	PRINTSTR("setup ls2g pci controller window\r\n");
	NORTHBRIDGE_INIT(0xbfe00004,0x7)        //command
	NORTHBRIDGE_INIT(0xbfe00010,0x80000000) //bar0 pci bus space 2G-4G
	NORTHBRIDGE_INIT(0xbfe00014,0x00000000) //bar1
	NORTHBRIDGE_INIT(0xbfe00018,0x800000)   //bar2 pci bus space 8M-16M maybe can delete
	NORTHBRIDGE_INIT(0xbfe0001c,0x0)        //bar3
	NORTHBRIDGE_INIT(0xbfe00110,0x11404)    //pci_map 0~128M 0x40000000~0x47ffffff     010001 010000 000100
	NORTHBRIDGE_INIT(0xbfe00150,0x8000000c) //pci_hit0_lo 2G
	NORTHBRIDGE_INIT(0xbfe00154,0xffffffff) //pci_hit0_hi
	NORTHBRIDGE_INIT(0xbfe00158,0xff80000c) //pci_hit1_lo 8M
	NORTHBRIDGE_INIT(0xbfe0015c,0xffffffff) //pci_hit1_hi
	NORTHBRIDGE_INIT(0xbfe00160,0x00000006) //pci_hit2_lo
	NORTHBRIDGE_INIT(0xbfe00164,0x00000000) //pci_hit2_hi
	NORTHBRIDGE_INIT(0xbfe00168,0xfe01c5)   //pxarb_config
	PRINTSTR("setup 1a south controller window\r\n");
	/*then config south bridge*/
	PCICONF_WRITEB(PCI_IDSEL_1A,0,0x04,7)           //pci_command
	PCICONF_WRITEW(PCI_IDSEL_1A,0,0x10,0xffffffff)  //base address reg0 
	PCICONF_WRITEW(PCI_IDSEL_1A,0,0x18,0xffffffff)  //base address reg2 
	PCICONF_WRITEW(PCI_IDSEL_1A,0,0x20,0xffffffff)  //base address reg4 
	PCICONF_WRITEW(PCI_IDSEL_1A,0,0x4c,0x82000001)  //reg isr_4ch

	PCICONF_WRITEW(PCI_IDSEL_1A,0,0x10,0x10200000)  //base address reg0 GPU/DC 2M
	PCICONF_WRITEW(PCI_IDSEL_1A,0,0x18,0x12000000)  //base address reg2 AXI MUX SLAVE 16M
	PCICONF_WRITEW(PCI_IDSEL_1A,0,0x20,0x40000000)  //base address reg4 MEM
	PRINTSTR("setup 1a south controller window done\r\n");

	NORTHBRIDGE_INIT(0xb2d001a0,0x000000f0)
	/*make pci's GPU/DC space map to ddr's config space,then change back*/
	NORTHBRIDGE_INIT(0xb2d00190,0x0fe000f0)
	NORTHBRIDGE_INIT(0xb2d0110c,0x1)        //?????1f_pci_oxarb_config
	/******************************************************************/
	NORTHBRIDGE_INIT(0xb2d01130,0xffe00004) //1a_pci_hit0_lo 2MB gpu/dc space
	NORTHBRIDGE_INIT(0xb2d01134,0x7fffffff) //1a_pci_hit0_hi
	NORTHBRIDGE_INIT(0xb2d01140,0xff000004) //1a_pci_hit1_lo 16MB axi slave space
	NORTHBRIDGE_INIT(0xb2d01144,0x7fffffff) //1a_pci_hit1_hi
	NORTHBRIDGE_INIT(0xb2d01150,0xfc000004) //1a_pci_hit2_lo MEM space 64MB//1G->0xc0000004 256M->0xf0000004
	NORTHBRIDGE_INIT(0xb2d01154,0xffffffff) //1a_pci_hit2_hi
	NORTHBRIDGE_INIT(0xb2d0110c,0x000001c5) //1a_pci_pxarb_config   // SUMH for 1A2
	PRINTSTR("setup 1a pci dma window\r\n");

	// 2g ddr 0x80000000-0xffffffff -> pci addr 0x80000000-0xffffffff
	NORTHBRIDGE_INIT(0xb2d00308,0x80000000) //1a_pci_pxarb_config   // SUMH for 1A2
	NORTHBRIDGE_INIT(0xb2d0030c,0x00000000) //1a_pci_pxarb_config   // SUMH for 1A2
	NORTHBRIDGE_INIT(0xb2d00348,0x80000000) //1a_pci_pxarb_config   // SUMH for 1A2
	NORTHBRIDGE_INIT(0xb2d0034c,0xffffffff) //1a_pci_pxarb_config   // SUMH for 1A2
	NORTHBRIDGE_INIT(0xb2d00388,0x800000f1) //1a_pci_pxarb_config   // SUMH for 1A2
	NORTHBRIDGE_INIT(0xb2d0038c,0x00000000) //1a_pci_pxarb_config   // SUMH for 1A2
	NORTHBRIDGE_INIT(0xb2d00328,0x80000000) //1a_pci_pxarb_config   // SUMH for 1A2
	NORTHBRIDGE_INIT(0xb2d0032c,0xffffffff) //1a_pci_pxarb_config   // SUMH for 1A2
	NORTHBRIDGE_INIT(0xb2d00368,0x80000000) //1a_pci_pxarb_config   // SUMH for 1A2
	NORTHBRIDGE_INIT(0xb2d0036c,0xffffffff) //1a_pci_pxarb_config   // SUMH for 1A2
	NORTHBRIDGE_INIT(0xb2d003a8,0x800000f1) //1a_pci_pxarb_config   // SUMH for 1A2
	NORTHBRIDGE_INIT(0xb2d003ac,0x00000000) //1a_pci_pxarb_config   // SUMH for 1A2

	PRINTSTR("setup 1a gpio\r\n");
	NORTHBRIDGE_INIT(0xb2e78030,0x8a8b)//config pll control reg ddr_dll = 5,cpu_dll = 4 
	NORTHBRIDGE_INIT(0xb2d010c0,0x00000000) //1a_gpio_cfg0   normal mode// SUMH for 1A2
	NORTHBRIDGE_INIT(0xb2d010c4,0x10000000) //1a_gpio_cfg1   normal mode// SUMH for 1A2 ,gpio60 for reset
	NORTHBRIDGE_INIT(0xb2d010c4,0x10000000) //1a_gpio_cfg1   normal mode// SUMH for 1A2 ,gpio60 for reset
	NORTHBRIDGE_INIT(0xb2d010c8,0x00000000) //1a_gpio_cfg2   normal mode// SUMH for 1A2
	NORTHBRIDGE_INIT(0xb2d00420,0x34000000) //1f_gpio_cfg2   // SUMH for 1A2 support nand gmac1 unspport LPC
	PRINTSTR("setup 1a done\r\n");

	j       a3
	nop
.set noat
END(init_south_bridge)

LEAF(init_ls2g_xbar_config)

	move a3,ra//save ra

TTYDBG("2G plus GMAC Address Config\r\n")
/*
gmac1: 0x0c0000000000
gmac2: 0x0d0000000000
gmac dma 0-> ddr 0
*/

/*
gmac1: 0x1e000000
gmac0: 0x1f000000
*/
//chengguipeng for display l1 xbar win5 ht0 0x00000000,1e000000->0x00000100,00000000 size 8k
	dli     t0, 0x900000003ff02000 # set ht0 since implicit routing has been disabled
	dli     t1, 0x000000001e000000 # ht0 base
	sd      t1, 0x28(t0) # win5
	dli     t1, 0xffffffffffffe000 # ht0 mask
	sd      t1, 0x68(t0) # win5
	dli     t1, 0x00000100000000f6 # ht0 mmap
	sd      t1, 0xa8(t0) # win5

//l1 xbar win3 ht0 0x00000000,1f000000->0x0000000,00000000 size 8k
	dli     t0, 0x900000003ff02000 # set ht0 since implicit routing has been disabled
	dli     t1, 0x000000001f000000 # ht0 base
	sd      t1, 0x18(t0) # win3
	dli     t1, 0xffffffffffffe000 # ht0 mask
	sd      t1, 0x58(t0) # win3
	dli     t1, 0x00000000000000f6 # ht0 mmap
	sd      t1, 0x98(t0) # win3

//l1 xbar win6 ht0 0x00000c00,00000000->0x00000000,00000000 size 8k
	dli     t0, 0x900000003ff02000 # set ht0 since implicit routing has been disabled
	dli     t1, 0x00000c0000000000 # ht0 base
	sd      t1, 0x30(t0) # win6
	dli     t1, 0xfffffeffffffe000 # ht0 mask
	sd      t1, 0x70(t0) # win6
	dli     t1, 0x00000000000000f6 # ht0 mmap
	sd      t1, 0xb0(t0) # win6

//l1 xbar gmac win 0x00000000,00000000->0x00000000,00000000 size 4GB
	#setup gmac DMA address window
	dli     t0, 0x900000003ff02600 # set gamc master address window
	dli     t1, 0x0000000000000000 # gmac0 base
	sd      t1, 0x00(t0) # win6
	dli     t1, 0xffffffff00000000 # gmac0 mask
	sd      t1, 0x40(t0) # win6
	dli     t1, 0x00000000000000f0 # gmac0 mmap
	sd      t1, 0x80(t0) # win6

	PRINTSTR("setup bigpci\r\n");

/*big pci window: cpu 1G->PCI 1G*/
	//l2 xbar win4 pci 0x00000000,40000000 ->0x00000000,40000000 1GB
	dli     t0, 0x900000003ff00000 # set ht0 since implicit routing has been disabled
	dli     t1, 0x0000000040000000 # cpu win4 base
	sd      t1, 0x20(t0) # win4
	dli     t1, 0xffffffffc0000000 # cpu win4 mask
	sd      t1, 0x60(t0) # win4
	dli     t1, 0x0000000040000082 # cpu win4 mmap
	sd      t1, 0xa0(t0) # win4


/*pci window*/
	PRINTSTR("setup pci\r\n");

/*setup bootrom cached*/
//l2 xbar win2 lpc boot 0x00000000,1fc00000->0x00000000,1fc00000 size 1MB
	dli     t1, 0x000000001fc00000 # lpc boot base
	sd      t1, 0x10(t0) # win2
	dli     t1, 0xfffffffffff00000 # lpc boot mask
	sd      t1, 0x50(t0) # win2
	dli     t1, 0x000000001fc000f2 # lpc mmap
	sd      t1, 0x90(t0) # win2

/*set other pci space uncached */
	dli     t1, 0x0000000010000082 # l2 xbar win1 mmap
	sd      t1, 0x88(t0) # win1

	PRINTSTR("setup memory\r\n");
/*memory*/
/*
cpu 0-256M -> ddr 0
*/
//l2 xbar win0 ddr window 0x00000000,00000000->0x00000000,00000000 size 256MB
	dli     t1, 0x0000000000000000 # ddr base
	sd      t1, 0x0(t0) # win0
	dli     t1, 0xfffffffff0000000 # ddr mask
	sd      t1, 0x40(t0) # win0
	dli     t1, 0x00000000000000f0 # ddr mmap
	sd      t1, 0x80(t0) # win0

/*
cpu 2G-4G -> ddr 0
*/
//l2 xbar win3 ddr window 0x00000000,80000000->0x00000000,00000000 size 2GB
	dli     t1, 0x0000000080000000 # ddr base
	sd      t1, 0x18(t0) # win3
	dli     t1, 0xffffffff80000000 # ddr mask
	sd      t1, 0x58(t0) # win3
	dli     t1, 0x00000000000000f0 # ddr mmap
	sd      t1, 0x98(t0) # win3

/*dma map*/
/*
pci 2G -> ddr 0
will set on pci_machdep.c
*/
//l2 xbar pci win0 0x00000000,80000000->0x00000000,00000000 size 2GB
	dli     t1, 0x0000000080000000 # pci win0 base
	sd      t1, 0x100(t0) # win6
	dli     t1, 0xffffffff80000000 # pci win0 mask
	sd      t1, 0x140(t0) # win6
	dli     t1, 0x00000000000000f0 # pci win0 mmap
	sd      t1, 0x180(t0) # win6

	PRINTSTR("other fixup\r\n");

#include "loongson3_fixup.S"
	PRINTSTR("setup done\r\n");

	j	a3
	nop
END(init_ls2g_xbar_config)

	.set at

#include "i2c.S"
#ifdef  AUTO_DDR_CONFIG
#include "ddr_dir/detect_node_dimm.S"
#endif

#######################################
#include "ddr_dir/2G_ddr_config.S"
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
#include "loongson2G_ddr_param.S"

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
