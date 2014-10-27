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
/* #include "target/i82371eb.h"  */
#include "target/prid.h"
#include "target/sbd.h"
#include "target/bonito.h"
#include "target/cs5536.h"
#include "target/cs5536_pci.h"
#include "target/i8254.h"
/* #include "target/pc97307.h" */
#include "target/isapnpreg.h"
#define USE_CS5536_UART1
#define USE_CS5536_UART2
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
    //----------add by cxk
1:
    b   1b
    nop
    //----------
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

	/*
	 * pre-initial the sourthbridge superio module.
	 */
	 

//lxf moxify	bal 	superio_init
	nop
	bal	initserial
	nop

start_now:

//Read sys_clk_sel
//cxk
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


PRINTSTR("DIMM read\r\n")


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
	
	bnez s0,1f
	nop
	li a0,128
	la v0,initmips
	jr v0
	nop
1:

#define GS_2F_DDR2_CONFIG //cuckoo for 2f v3 board

#ifdef GS_2F_DDR2_CONFIG //cuckoo for 2f v3 board

gs_2f_v3_ddr2_cfg:


    ###enable the reg space###
#if 1
	TTYDBG	("\r\nenable register space of MEMORY\r\n")
        li  t2,0xbfe00180
        ld  a1,0x0(t2)
        li  a0,0xFFFFFEFF
        and a0,a0,a1
        #or  a1,a1,0x100
        sd  a0,0x0(t2)
#endif    
	li	msize,0x40000000
	PRINTSTR("DDR2 config begin_2\r\n")
	bal	ddr2_config
	nop
	PRINTSTR("DDR2 config end\r\n")
#endif //cuckoo for 2f v3 board

#if 1 //print registers
    li      t1, 151
    li      t2, 0xaff00000

reg_read:
    lw      a0, 0x4(t2)
    bal	    hexserial
    nop
    PRINTSTR("  ")

    lw      a0, 0x0(t2)
    bal	    hexserial
    nop
    PRINTSTR("\r\n")

    addiu   t2, t2, 16
    bnez    t1, reg_read
    addiu   t1, t1, -1
#endif
#if 0
        //CUCKOO PCI ARBITOR config
        li t2,0xbfe00168
        lw a1,0x0(t2)
        ori a1,a1,0x2
        sw a1,0x0(t2)
#endif 

    ###disable the reg space###
#if 1
	TTYDBG	("\r\ndisable register space of MEMORY\r\n")
        li  t2,0xbfe00180
        ld  a1,0x0(t2)
        or  a1,a1,0x100
        sd  a1,0x0(t2)
#endif    

do_caches:

#if 1 //by_css
	TTYDBG("godson2 caches found\r\n")
        bal     godson2_cache_init
        nop
	TTYDBG("scache init\r\n") 
        bal	scache_init // smh
        nop
#endif

//add by cxk--note: if Debug_Mem here, cache access has problem!!!!!!!!

#ifdef WITH_HT

#include "ht.S"

#endif


endtest:
	
/*strange: must disable cache here for 2gplus,otherwise will halt on pmon sometimes randomly
*/
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

//add by cxk--note: work ok here!!!
//#include "Test_Mem.S"

//#define GODSON3_SIM
#ifdef GODSON3_SIM
	TTYDBG("bypass Copy PMON to execute location...\r\n")
#else
	TTYDBG("Copy PMON to execute location...\r\n")
#define DEBUG_LOCORE
#ifdef DEBUG_LOCORE
#if 0
	TTYDBG("  start = 0x")
	la	a0, start
	move	t0, a0
	move	t1, t0
	move	a0, t1
	bal	hexserial
	nop
	TTYDBG("\r\n  copytoram = 0x")
	la	a0, copytoram
	addu	a0, s0
	bal	hexserial
	nop
	TTYDBG("\r\n  s0 = 0x")
	move	a0, s0
	bal	hexserial
	nop
	TTYDBG("\r\n")
#endif
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

	b mydebug_enter
	nop
bootnow:


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
	srl	a0,20
#if 0
        li      a0,256
#endif


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
  move t7,ra
  move a1,a0
  dsrl a0,32
  bal hexserial
  nop
  move a0,a1
  bal hexserial
  nop
  jr t7
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
 *	      			      CS5536					*
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

        la      v0, COM2_BASE_ADDR
		li		a0,(3686400 /2 / 16)/CONS_BAUD
		nop
		bal  1f
		nop

        la      v0, COM3_BASE_ADDR
		li		a0,(3686400 /2 / 16)/CONS_BAUD
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
    la  v0, COM2_BASE_ADDR
    bal 1f
    nop
    la  v0, COM3_BASE_ADDR
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
END(tgt_putchar)

#define DDR2_2f
#ifdef DDR2_2f

#include "ddr2_config_for_2gplus_32.S"

#else



#######################################
#define REG_ADDRESS 0x0
#define CONFIG_BASE 0xaff00000
    

        .global ddr2_config
        .ent    ddr2_config
        .set    noreorder
        .set    mips3
ddr2_config:
    la      t0, ddr2_reg_data
    addu    t0, t0, s0
    li      t1, 152 ##0x72
    li      t2, CONFIG_BASE

reg_write:

    ld      a1, 0x0(t0)
    sd      a1, REG_ADDRESS(t2)

/*
    ld      a0,0x0(t0)
    bal	    hexserial
    nop
    PRINTSTR("\r\n")

    ld      a0, REG_ADDRESS(t2)
    bal	    hexserial
    nop
    PRINTSTR("\r\n\r\n")
*/

    subu    t1, t1, 0x1
    addiu   t0, t0, 0x8
    bne     t1, $0, reg_write
    addiu   t2, t2, 0x10
    
    ############start##########
    li      t2, CONFIG_BASE
    la		t0,DDR2_CTL_start_DATA_LO
    #la		t0,ddr2_start_reg
    addu    	t0, t0, s0
    ld      a1, 0x0(t0)
    sd      a1, 0x30(t2)

    jr      ra
    nop
    .end    ddr2_config



	.rdata
	.align 5
ddr2_reg_data:
DENALI_CTL_000 : .dword 0x0000010000000101
DENALI_CTL_010 : .dword 0x0000010000000000
DENALI_CTL_020 : .dword 0x0100010100000000
DENALI_CTL_030 : .dword 0x0101010001010000
DENALI_CTL_040 : .dword 0x0100010100010101
DENALI_CTL_050 : .dword 0x0000000404050100
DENALI_CTL_060 : .dword 0x0a04040603040003
DENALI_CTL_070 : .dword 0x0f0e0200000f0a08
DENALI_CTL_080 : .dword 0x0102040801020408
DENALI_CTL_090 : .dword 0x0000050b00000000
DENALI_CTL_0a0 : .dword 0x0000003f3f140612
DENALI_CTL_0b0 : .dword 0x0000000000000000
DENALI_CTL_0c0 : .dword 0x00002c050f000000
DENALI_CTL_0d0 : .dword 0x0000000000000000
DENALI_CTL_0e0 : .dword 0x0000000000000000
DENALI_CTL_0f0 : .dword 0x0000000000000000
DENALI_CTL_100 : .dword 0x0000000000000000
DENALI_CTL_110 : .dword 0x0000000000000c2d
DENALI_CTL_120 : .dword 0x001c000000000000
DENALI_CTL_130 : .dword 0x6d56000302000000
DENALI_CTL_140 : .dword 0x0000204002000030
DENALI_CTL_150 : .dword 0x0000000000000004
DENALI_CTL_160 : .dword 0x0000000000000000
DENALI_CTL_170 : .dword 0x0000000000000000
DENALI_CTL_180 : .dword 0x0000000000000000
DENALI_CTL_190 : .dword 0x0000000000000000
DENALI_CTL_1a0 : .dword 0x0000000000000000
DENALI_CTL_1b0 : .dword 0x0000000000000000
DENALI_CTL_1c0 : .dword 0x0000000000000000
DENALI_CTL_1d0 : .dword 0x0203070400000101
DENALI_CTL_1e0 : .dword 0x0c2d0c2d0c2d0205
DENALI_CTL_1f0 : .dword 0x00208e8000000000
DENALI_CTL_200 : .dword 0x00208e8000208e80
DENALI_CTL_210 : .dword 0x00208e8000208e80
DENALI_CTL_220 : .dword 0x00208e8000208e80
DENALI_CTL_230 : .dword 0x00208e8000208e80//dll_ctrl_reg_0
DENALI_CTL_240 : .dword 0x00008e0000008e00//dll_ctrl_reg_1
DENALI_CTL_250 : .dword 0x00008e0000008e00
DENALI_CTL_260 : .dword 0x00008e0000008e00
DENALI_CTL_270 : .dword 0x00008e0000008e00
DENALI_CTL_280 : .dword 0x0000000000000e00
DENALI_CTL_290 : .dword 0x0000000000000000
DENALI_CTL_2a0 : .dword 0x0000000000000000
DENALI_CTL_2b0 : .dword 0x0000000000000000
DENALI_CTL_2c0 : .dword 0x0000000000000000
DENALI_CTL_2d0 : .dword 0xf30049470000019d
DENALI_CTL_2e0 : .dword 0xf3004947f3004947
DENALI_CTL_2f0 : .dword 0xf3004947f3004947
DENALI_CTL_300 : .dword 0xf3004947f3004947
DENALI_CTL_310 : .dword 0xf3004947f3004947
DENALI_CTL_320 : .dword 0x07c0000007c00000
DENALI_CTL_330 : .dword 0x07c0000007c00000
DENALI_CTL_340 : .dword 0x07c0000007c00000
DENALI_CTL_350 : .dword 0x07c0000007c00000
DENALI_CTL_360 : .dword 0x0800c00507c00000
DENALI_CTL_370 : .dword 0x0000000000000000
DENALI_CTL_380 : .dword 0x0000000000000000
DENALI_CTL_390 : .dword 0x0000000000000000
DENALI_CTL_3a0 : .dword 0x0000000000000000
DENALI_CTL_3b0 : .dword 0x0000000000000000
DENALI_CTL_3c0 : .dword 0x0000000000000000
DENALI_CTL_3d0 : .dword 0x0000000000000000
DENALI_CTL_3e0 : .dword 0x0000000000000000
DENALI_CTL_3f0 : .dword 0x0000000000000000
DENALI_CTL_400 : .dword 0x0000000000000000
DENALI_CTL_410 : .dword 0x0000000000000000
DENALI_CTL_420 : .dword 0x0000000000000000
DENALI_CTL_430 : .dword 0x0000000000000000
DENALI_CTL_440 : .dword 0x0000000000000000
DENALI_CTL_450 : .dword 0x0000000000000000
DENALI_CTL_460 : .dword 0x0000000000000000
DENALI_CTL_470 : .dword 0x0000000000000000
DENALI_CTL_480 : .dword 0x0000000000000000
DENALI_CTL_490 : .dword 0x0000000000000000
DENALI_CTL_4a0 : .dword 0x0000000000000000
DENALI_CTL_4b0 : .dword 0x0000000000000000
DENALI_CTL_4c0 : .dword 0x0000000000000000
DENALI_CTL_4d0 : .dword 0x0000000000000000
DENALI_CTL_4e0 : .dword 0x0000000000000000
DENALI_CTL_4f0 : .dword 0x0000000000000000
DENALI_CTL_500 : .dword 0x0000000000000000
DENALI_CTL_510 : .dword 0x0000000000000000
DENALI_CTL_520 : .dword 0x0000000000000000
DENALI_CTL_530 : .dword 0x0000000000000000
DENALI_CTL_540 : .dword 0x0000000000000000
DENALI_CTL_550 : .dword 0x0000000000000000
DENALI_CTL_560 : .dword 0x0000000000000000
DENALI_CTL_570 : .dword 0x0000000000000000
DENALI_CTL_580 : .dword 0x0000000000000000
DENALI_CTL_590 : .dword 0x0000000000000000
DENALI_CTL_5a0 : .dword 0x0000000000000000
DENALI_CTL_5b0 : .dword 0x0000000000000000
DENALI_CTL_5c0 : .dword 0x0000000000000000
DENALI_CTL_5d0 : .dword 0x0000000000000000
DENALI_CTL_5e0 : .dword 0x0000000000000000
DENALI_CTL_5f0 : .dword 0x0000000000000000
DENALI_CTL_600 : .dword 0x0000000000000000
DENALI_CTL_610 : .dword 0x0000000000000000
DENALI_CTL_620 : .dword 0x0000000000000000
DENALI_CTL_630 : .dword 0x0000000000000000
DENALI_CTL_640 : .dword 0x0000000000000000
DENALI_CTL_650 : .dword 0x0000000000000000
DENALI_CTL_660 : .dword 0x0000000000000000
DENALI_CTL_670 : .dword 0x0000000000000000
DENALI_CTL_680 : .dword 0x0000000000000000
DENALI_CTL_690 : .dword 0x0000000000000000
DENALI_CTL_6a0 : .dword 0x0000000000000000
DENALI_CTL_6b0 : .dword 0x0000000000000000
DENALI_CTL_6c0 : .dword 0x0000000000000000
DENALI_CTL_6d0 : .dword 0x0000000000000000
DENALI_CTL_6e0 : .dword 0x0000000000000000
DENALI_CTL_6f0 : .dword 0x0000000000000000
DENALI_CTL_700 : .dword 0x0000000000000000
DENALI_CTL_710 : .dword 0x0000000000000000
DENALI_CTL_720 : .dword 0x0000000000000000
DENALI_CTL_730 : .dword 0x0000000000000000
DENALI_CTL_740 : .dword 0x0100000000000000
DENALI_CTL_750 : .dword 0x0100000101020101
DENALI_CTL_760 : .dword 0x0303030000020001
DENALI_CTL_770 : .dword 0x0101010202020203
DENALI_CTL_780 : .dword 0x0102020400040c01
DENALI_CTL_790 : .dword 0x281900000f000303
DENALI_CTL_7a0 : .dword 0x00000000000000ff
DENALI_CTL_7b0 : .dword 0x0000000000000000
DENALI_CTL_7c0 : .dword 0x0000000000000000
DENALI_CTL_7d0 : .dword 0x0000000000000000
DENALI_CTL_7e0 : .dword 0x0000000000000000
DENALI_CTL_7f0 : .dword 0xff08000000000000
DENALI_CTL_800 : .dword 0x8e8e8e8e8e8e8e8e//rdlvl_midpoint_delay
DENALI_CTL_810 : .dword 0x000000000000000e
DENALI_CTL_820 : .dword 0x0420000c20400000
DENALI_CTL_830 : .dword 0x0000000000000c0a
DENALI_CTL_840 : .dword 0x0000640064000000
DENALI_CTL_850 : .dword 0x0000000000000064
DENALI_CTL_860 : .dword 0x0200004000000000
DENALI_CTL_870 : .dword 0x0002000200020002
DENALI_CTL_880 : .dword 0x0000000000000000
DENALI_CTL_890 : .dword 0x0a520a520a520a52
DENALI_CTL_8a0 : .dword 0x00000000001c001c
DENALI_CTL_8b0 : .dword 0x0000000000000000
DENALI_CTL_8c0 : .dword 0x0004000000000000
DENALI_CTL_8d0 : .dword 0x00000000c8000000
DENALI_CTL_8e0 : .dword 0x0000000000000050
DENALI_CTL_8f0 : .dword 0x0000000080808080//dll_ctrl_reg_2
DENALI_CTL_900 : .dword 0x0000000000000000
DENALI_CTL_910 : .dword 0x0000000000000000
DENALI_CTL_920 : .dword 0x0000000000000000
DENALI_CTL_930 : .dword 0x0000000000000000
DENALI_CTL_940 : .dword 0x0300000000050500
DENALI_CTL_950 : .dword 0x0000000000000a03
DENALI_CTL_960 : .dword 0x0503000000000000
DENALI_CTL_970 : .dword 0x000000000003e805
DDR2_CTL_start_DATA_LO: .word  0x01000000
//0000000_1 swap_port_rw_same_en 0000000_1 swap_en 0000000_0 start 0000000_0 srefresh
DDR2_CTL_start_DATA_HI: .word  0x01010100
#endif

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
	b 1f;
	nop;

i2cerr :
	nop;
	li	v0, 0x02;
	IO_WRITE_BYTE(SMB_BASE_ADDR | SMB_CTRL1);
	li  v0, 0x10;	
	IO_WRITE_BYTE(SMB_BASE_ADDR | SMB_STS);
1:
	jr	ra;
	nop;
END(i2cread)

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
	NORTHBRIDGE_INIT(0xb2d01114,0x238a1)    //1a_pci_map 1a pci mem map to 2g ddr 1a 0x10000000-0x18000000 -> pci addr 0x84xxxxxx-0x8cxxxxxx
	/******************************************************************
	0x18000000~0x1bffffff->0x8c000000~0x8fffffff (100011)
	0x14000000~0x17ffffff->0x88000000~0x8bffffff (100010)
	0x10000000~0x13ffffff->0x84000000~0x87ffffff (100001)

	******************************************************************/
	NORTHBRIDGE_INIT(0xb2d01130,0xffe00004) //1a_pci_hit0_lo 2MB gpu/dc space
	NORTHBRIDGE_INIT(0xb2d01134,0x7fffffff) //1a_pci_hit0_hi
	NORTHBRIDGE_INIT(0xb2d01140,0xff000004) //1a_pci_hit1_lo 16MB axi slave space
	NORTHBRIDGE_INIT(0xb2d01144,0x7fffffff) //1a_pci_hit1_hi
	NORTHBRIDGE_INIT(0xb2d01150,0xf0000004) //1a_pci_hit2_lo MEM space 256MB//1G->0xc0000004 256M->0xf0000004
	NORTHBRIDGE_INIT(0xb2d01154,0xffffffff) //1a_pci_hit2_hi
	NORTHBRIDGE_INIT(0xb2d0110c,0x000001c5) //1a_pci_pxarb_config   // SUMH for 1A2//lxf add

	PRINTSTR("setup 1a gpio\r\n");
	NORTHBRIDGE_INIT(0xb2e78030,0x8f8b)//config pll control reg ddr_dll = 10,cpu_dll = 4
	NORTHBRIDGE_INIT(0xb2d010c0,0x00000000) //1a_gpio_cfg0   normal mode// SUMH for 1A2
	NORTHBRIDGE_INIT(0xb2d010c4,0x10000000) //1a_gpio_cfg1   normal mode// SUMH for 1A2 ,gpio60 for reset
	NORTHBRIDGE_INIT(0xb2d010c8,0x00000000) //1a_gpio_cfg2   normal mode// SUMH for 1A2
	NORTHBRIDGE_INIT(0xb2d00420,0x0a000000) //1f_gpio_cfg2   // SUMH for 1A2 support nand gmac1 unspport LPC
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

#include "loongson3_fixup.S"//maybe can delete lxf test
	PRINTSTR("setup done\r\n");

	j	a3
	nop
END(init_ls2g_xbar_config)


//#undef COM1_BASE_ADDR
//#define COM1_BASE_ADDR COM2_BASE_ADDR
mydebug_enter:
#include "machine/newtest/mydebug.S"
