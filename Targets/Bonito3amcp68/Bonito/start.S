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
#define DEBUG_LOCORE



#ifdef DEBUG_LOCORE
#define	TTYDBG(x) \
	.rdata;98: .asciz x; .text; la a0, 98b; bal stringserial; nop
#define	TTYDBG_COM1(x) \
	.rdata;98: .asciz x; .text; la a0, 98b; bal stringserial_COM1; nop
#else
#define TTYDBG(x)
#endif

#define	PRINTSTR(x) \
	.rdata;98: .asciz x; .text; la a0, 98b; bal stringserial; nop

#ifdef DEVBD2F_SM502
#define GPIOLED_DIR  0xe
#else
#define GPIOLED_DIR  0xf
#endif

#ifndef USE_GPIO_SERIAL
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
#else
#define GPIOLED_SET(x) 
#endif


#define CONFIG_CACHE_64K_4WAY 1

#define tmpsize		s1
#define msize		s2
#define bonito		s4
#define dbg			s5
#define sdCfg		s6

#define CP0_CONFIG $16
#define CP0_TAGLO  $28
#define CP0_TAGHI  $29

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

#define DDR_DQ 0xbff00030             /*  zgj  */

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

//	bal	uncached		/* Switch to uncached address space */
	nop
	GPIOLED_SET(2)

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
#ifndef ROM_EXCEPTION
	PRINTSTR("\r\nDERR0=")
	cfc0	a0, COP_0_DERR_0
	bal	hexserial
	nop
	PRINTSTR("\r\nDERR1=")
	cfc0	a0, COP_0_DERR_1
	bal	hexserial
	nop
#else
1:
	b 1b
	nop
#endif

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
lpc_write_test: 

loop_here:
		.set 	mips64
		mfc0	t0, $15, 1
		.set 	mips3
		andi	t0, 0x3ff
		//bnez 	t0, loop_here
        //li      a0, 0x3
		//bne 	t0, a0, slave_main
		bnez 	t0, slave_main
		nop

        li      a0,0xbff00080
        li      t0,0xcc
        sb      t0,0x0(a0)

	bal	initserial //cuckoo for 2f v3
	nop

#if 0
        PRINTSTR("Shut down other cores\r\n")
        li      a0, 0xbfe00180
        lw      a1, 0x0(a0)
        li      t1, 0xffff1fff
        and     a1, a1, t1
        sw      a1, 0x0(a0)

#endif


        mfc0    t0, CP0_STATUS

        PRINTSTR("CP0_STATUS:")
        move    a0,t0
        bal	    hexserial
        nop
        PRINTSTR("\r\n\r\n")

        li      a0,0xbff00080
        li      t0,0xbb
        sb      t0,0x0(a0)

        li      t1, 0x00e0      # {cu3,cu2,cu1,cu0}<={0110, status_fr<=1
        or      t0, t0, t1

        PRINTSTR("CP0_STATUS:")
        move    a0,t0
        bal	    hexserial
        nop
        PRINTSTR("\r\n\r\n")

        mtc0    t0, CP0_STATUS

        PRINTSTR("CP0_STATUS:")
        mfc0    a0, CP0_STATUS
        bal	    hexserial
        nop
        PRINTSTR("\r\n\r\n")


#if 0
#set XBAR to route all the DMA request to Scache0
        dli       a0,0xffffffffffffffff
        dli       t0,0x900000003ff00400
        sd        a0,0x0(t0)

PRINTSTR("Scache index setup done\r\n")
#endif


        li      a0,0x900000001ff00080
        li      t0,0x99
        sb      t0,0x0(a0)

        b       core0_start
        nop

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
#define _ISABWR_INIT(mod,function,isabreg,val) \
	WR_INIT(MOD_W,BONITO_BASE+BONITO_PCIMAP_CFG,CFGADDR(PCI_IDSEL_VIA686B,function,isabreg)>>16) ; \
        RD_INIT(MOD_W,BONITO_BASE+BONITO_PCIMAP_CFG) ; \
	WR_INIT(mod,PCI_CFG_SPACE+(CFGADDR(PCI_IDSEL_VIA686B,function,isabreg)&0xffff),val)
	
#define _ISABRD_INIT(mod,function,isabreg) \
	WR_INIT(MOD_W,BONITO_BASE+BONITO_PCIMAP_CFG,CFGADDR(PCI_IDSEL_VIA686B,function,isabreg)>>16) ; \
        RD_INIT(MOD_W,BONITO_BASE+BONITO_PCIMAP_CFG) ; \
	RD_INIT(mod,PCI_CFG_SPACE+(CFGADDR(PCI_IDSEL_VIA686B,function,isabreg)&0xffff))


#define _ISAWR_INIT(isareg,val) \
	WR_INIT(MOD_B,PCI_IO_SPACE+(isareg),val)
	
#define _ISARD_INIT(isareg) \
	RD_INIT(MOD_B,PCI_IO_SPACE+(isareg))
	

#define ISABBWR_INIT(function,isabreg,val) \
	_ISABWR_INIT(MOD_B,function,(isabreg),val)
#define ISABHWR_INIT(function,isabreg,val) \
	_ISABWR_INIT(MOD_H,function,(isabreg),val)
#define ISABWWR_INIT(function,isabreg,val) \
	_ISABWR_INIT(MOD_W,function,isabreg,val)
#define ISAWR_INIT(isareg,val) \
	_ISAWR_INIT(isareg,val)
#define ISARD_INIT(isareg) \
	_ISARD_INIT(isareg)
	GPIOLED_SET(4)

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
	
    BONITO_BIC(BONITO_BONGENCFG, 0x80)  #½û?¹iobc

	#BONITO_BIS(BONITO_BONGENCFG, BONITO_BONGENCFG_BUSERREN)

	/* Set debug mode */
	BONITO_BIS(BONITO_BONGENCFG, BONITO_BONGENCFG_DEBUGMODE)

    /******** added to init southbridge*/
	
#if  (PCI_IDSEL_VIA686B != 0)
	/* Set the SMB base address */
	ISABWWR_INIT(4, SMBUS_IO_BASE_ADDR, SMBUS_IO_BASE_VALUE | 0x1)
	/* enable the host controller */
	ISABHWR_INIT(4, SMBUS_HOST_CONFIG_ADDR, SMBUS_HOST_CONFIG_ENABLE_BIT)
	/* enable the SMB IO ports */
	ISABBWR_INIT(4, PCI_COMMAND_STATUS_REG, PCI_COMMAND_IO_ENABLE)
	
/* 15us ISA bus refresh clock */
#define ISAREFRESH (PT_CRYSTAL/(1000000/15))
	ISARD_INIT(CTC_PORT+PT_CONTROL)
	
	/* program i8254 ISA refresh counter */
	ISAWR_INIT(CTC_PORT+PT_CONTROL,PTCW_SC(PT_REFRESH)|PTCW_16B|PTCW_MODE(MODE_RG))
	ISAWR_INIT(CTC_PORT+PT_REFRESH, ISAREFRESH & 0xff)
	ISAWR_INIT(CTC_PORT+PT_REFRESH, ISAREFRESH >> 8)
#endif
                                                                        
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
	GPIOLED_SET(5)
	bal 	superio_init
	nop
	
	GPIOLED_SET(6)
	bal	initserial
	nop
	GPIOLED_SET(7)
    */
	
core0_start:
	PRINTSTR("\r\nPMON2000 MIPS Initializing. Standby...\r\n")
    /*
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
    */

	bnez s0,1f
	nop

	li a0,128
	la v0,initmips
	jr v0
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
#if 1
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
        andi t0, t1, 1f
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
    TTYDBG ("\r\nMEM CLK SEL : ")
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

##########################################

#include "loongson3_ddr2_config.S"


##########################################

	
skipdimm:
	PRINTSTR("Init SDRAM Done!\r\n");

		bal CPU_TLBClear
		nop



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
#if 0
	TTYDBG("Sizing caches...\r\n");

	mfc0	t3, COP_0_CONFIG	/* t3 = original config */
	and	t3, 0xffffeff0		/* Make sure coherency is OK */

	and	t3, ~(CF_7_TE|CF_7_SE|CF_7_TC|CF_7_SC)  /* disable L2/L3 cache */
	mtc0    t3, COP_0_CONFIG

	li	t2, 4096

	srl	t1, t3, 9
	and	t1, 7
	sllv	s3, t2, t1		/* s3 = I cache size */

#ifdef CONFIG_CACHE_64K_4WAY 
        sll     s3,2
#endif

	and	t1, t3, 0x20
	srl	t1, t1, 1
	addu	s4, t1, 16		/* s4 = I cache line size */

	srl	t1, t3, 6
	and	t1, 7
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
        li      a1, 0x6301
        bne     a0,a1,1f
        nop
#endif
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
#endif

	and	t1, t3, 0x10
	addu	s6, t1, 16		/* s6 = D cache line size */
	TTYDBG("Init tlb...\r\n")
        bal     tlb_init
        nop
	TTYDBG("\r\nInit caches...\r\n")

//	li	s7, 0                   /* no L2 cache */
//	li	s8, 0                   /* no L3 cache */
//

#if 1
        //mfc0    a0, COP_0_PRID
        //li      a1, 0x6301
        //bne     a0,a1,1f
        //nop
	TTYDBG("godson2 caches found\r\n")
        bal     godson2_cache_init
        nop
	TTYDBG("scache init\r\n")
        bal	scache_init // smh
        nop
#endif

#if 1
PRINTSTR("Jump to 9fc\r\n")
        lui     t0, 0xdfff ####################### go to 9fc
        ori     t0, t0, 0xffff
        bal     1f
        nop
1:
        and     ra, ra, t0
        addiu   ra, ra, 16
        jr      ra
        nop

#endif

#if 1
        mfc0   $4, $16
        and    $4,0xfffffff8
        or     $4,0x3
        mtc0   $4,$16

	TTYDBG("scache init done\r\n")
	nop
#endif

#if 0 //by AdonWang disable L2 cache
	lw	a1,0xbfe00180
	or	a1,a1,0x08 		/*disable L2 Cache*/
	sw	a1,0xbfe00180
	TTYDBG("Disable L2 Cache done\r\n")
#endif


#if 1//config L1 xbar cpu port
//Core 0
	TTYDBG("Fix L1xbar illegal access \r\n")
	dli	t2, 0x900000003ff02028
	dli	t0, 0x00000e0000000000
	sd	t0, 0x0(t2)

	dli	t2, 0x900000003ff02068
	dli	t0, 0xffffff0000000000
	sd	t0, 0x0(t2)

	dli	t2, 0x900000003ff020a8
	dli	t0, 0x87
	sd	t0, 0x0(t2)

	dli	t2, 0x900000003ff02030
	dli	t0, 0x00000e0000000000
	sd	t0, 0x0(t2)

	dli	t2, 0x900000003ff02070
	dli	t0, 0xffffff0000000000
	sd	t0, 0x0(t2)

	dli	t2, 0x900000003ff020b0
	dli	t0, 0xf0
	sd	t0, 0x0(t2)

//Core 1
	TTYDBG("Fix L1xbar illegal access \r\n")
	dli	t2, 0x900000003ff02128
	dli	t0, 0x00000e0000000000
	sd	t0, 0x0(t2)

	dli	t2, 0x900000003ff02168
	dli	t0, 0xffffff0000000000
	sd	t0, 0x0(t2)

	dli	t2, 0x900000003ff021a8
	dli	t0, 0x87
	sd	t0, 0x0(t2)

	dli	t2, 0x900000003ff02130
	dli	t0, 0x00000e0000000000
	sd	t0, 0x0(t2)

	dli	t2, 0x900000003ff02170
	dli	t0, 0xffffff0000000000
	sd	t0, 0x0(t2)

	dli	t2, 0x900000003ff021b0
	dli	t0, 0xf0
	sd	t0, 0x0(t2)

//Core 2
	TTYDBG("Fix L1xbar illegal access \r\n")
	dli	t2, 0x900000003ff02228
	dli	t0, 0x00000e0000000000
	sd	t0, 0x0(t2)

	dli	t2, 0x900000003ff02268
	dli	t0, 0xffffff0000000000
	sd	t0, 0x0(t2)

	dli	t2, 0x900000003ff022a8
	dli	t0, 0x87
	sd	t0, 0x0(t2)

	dli	t2, 0x900000003ff02230
	dli	t0, 0x00000e0000000000
	sd	t0, 0x0(t2)

	dli	t2, 0x900000003ff02270
	dli	t0, 0xffffff0000000000
	sd	t0, 0x0(t2)

	dli	t2, 0x900000003ff022b0
	dli	t0, 0xf0
	sd	t0, 0x0(t2)

//Core 3
	TTYDBG("Fix L1xbar illegal access \r\n")
	dli	t2, 0x900000003ff02328
	dli	t0, 0x00000e0000000000
	sd	t0, 0x0(t2)

	dli	t2, 0x900000003ff02368
	dli	t0, 0xffffff0000000000
	sd	t0, 0x0(t2)

	dli	t2, 0x900000003ff023a8
	dli	t0, 0x87
	sd	t0, 0x0(t2)

	dli	t2, 0x900000003ff02330
	dli	t0, 0x00000e0000000000
	sd	t0, 0x0(t2)

	dli	t2, 0x900000003ff02370
	dli	t0, 0xffffff0000000000
	sd	t0, 0x0(t2)

	dli	t2, 0x900000003ff023b0
	dli	t0, 0xf0
	sd	t0, 0x0(t2)

############
#if 0
	TTYDBG("Fix L1xbar illegal access \r\n")
	dli	t2, 0x900000003ff02030
	dli	t0, 0x0
	sd	t0, 0x0(t2)

	dli	t2, 0x900000003ff02070
	dli	t0, 0xffffff0000000000
	sd	t0, 0x0(t2)

	dli	t2, 0x900000003ff020b0
	dli	t0, 0xf0
	sd	t0, 0x0(t2)

############
	TTYDBG("Fix L2xbar illegal access \r\n")
	dli	t2, 0x900000003ff00010
	dli	t0, 0x000000001fc00000
	sd	t0, 0x0(t2)

	dli	t2, 0x900000003ff00050
	dli	t0, 0xfffffffffff00000
	sd	t0, 0x0(t2)

	dli	t2, 0x900000003ff00090
	dli	t0, 0xf2
	sd	t0, 0x0(t2)

############
	TTYDBG("Fix L2xbar illegal access \r\n")
	dli	t2, 0x900000003ff00018
	dli	t0, 0x000000001fe00000
	sd	t0, 0x0(t2)

	dli	t2, 0x900000003ff00058
	dli	t0, 0xfffffffffff00000
	sd	t0, 0x0(t2)

	dli	t2, 0x900000003ff00098
	dli	t0, 0xf2
	sd	t0, 0x0(t2)

#### disable default
	dli	t2, 0x900000003ff00088
	dli	t0, 0x0
	sd	t0, 0x0(t2)
#endif

#endif


####################################

#include "loongson3_HT_init.S"

####################################

//#include "loongson3_ddr_debug.s"

####################################

#include "machine/newtest/mydebug.S"

####################################


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
#if 0
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
	PRINTSTR("\r\n")


    /* pass pointer to kseg1 tgt_putchar */
    la  a1, tgt_putchar
	addu a1,a1,s0

    la  a2, stringserial
	addu a2,a2,s0

	srl	msize,20
	move	a0,msize
	
#if 1 
#define CORE0_BUF0 0x900000003ff01020   
#define CORE1_BUF0 0x900000003ff01120   
#define CORE2_BUF0 0x900000003ff01220   
#define CORE3_BUF0 0x900000003ff01320   

    dli     t0, CORE0_BUF0 #buf of cpu0
    li      t1, 0x5a5a
    sw      t1, 0(t0)
    nop
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

LEAF(stringserial)
	move	a2, ra
#ifdef ROM_EXCEPTION
	li a1,0x3ec00000
	addu	a1, a0, a1
#else
	addu	a1, a0, s0
#endif
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
#ifdef ROM_EXCEPTION
	la	v0, (hexchar+0x3ec00000)
#else
	la	v0, hexchar
	addu	v0, s0
#endif
	addu	v0, a0
	bal	tgt_putchar
	lbu	a0, 0(v0)

	bnez	a3, 1b
	addu	a3, -1

	j	a2
	nop
END(hexserial)
#define USE_LPC_UART
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
        #define GS3_UART0_BASE 0xbfe001e0
        #define GS3_UART1_BASE 0xbfe001e8
        #define GS3_UART_BASE GS3_UART0_BASE

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

	j	ra
	nop	
END(tgt_putchar)
#endif
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

#if 0
1:
        li      v1, 0x78
        sb	v1, 0x7 (v0)    

#        li      v1,12
#        li      a0,0xbff00080
#        sb      v1,0x0(a0)
 
        lb	v1, 0x7 (v0)    
        lb	v1, 0x7 (v0)    
 #       li      a0,0xbff00080
#        sb      v1,0x0(a0)
        b 1b
#endif

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
#	la	v0, COM1_BASE_ADDR
	la	v0, COM3_BASE_ADDR
	bne	v0, v1, 1b
	nop

	j	ra
	nop
END(initserial)
#else
LEAF(initserial)
        li  a0, GS3_UART_BASE

 	li	t1,128
 #	addiu	a2,a0,3
 	sb	t1,3(a0)
 	li	t1,0x12      # divider, highest possible baud rate
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

LEAF(initserial_COM1)
	#la	v0, COM1_BASE_ADDR
	la	v0, 0xba0003f8
	#la	v0, 0xba0002f8
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
	#la	v0, COM1_BASE_ADDR
	la	v0, 0xba0003f8
	#la	v0, 0xba0002f8
	bne	v0, v1, 1b
	nop

	j	ra
	nop
END(initserial_COM1)
LEAF(stringserial_COM1)
	move	a2, ra
	addu	a1, a0, s0
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
	addu	v0, s0
	addu	v0, a0
	bal	tgt_putchar_COM1
	lbu	a0, 0(v0)

	bnez	a3, 1b
	addu	a3, -1

	j	a2
	nop
END(hexserial_COM1)

LEAF(tgt_putchar_COM1)
	#la	v0, COM1_BASE_ADDR
	la	v0, 0xba0003f8
	#la	v0, 0xba0002f8
1:
	lbu	v1, NSREG(NS16550_LSR)(v0)
	and	v1, LSR_TXRDY
#	li	v1, 1
	beqz	v1, 1b
	nop

	sb	a0, NSREG(NS16550_DATA)(v0)
	move	v1, v0
	#la	v0, COM1_BASE_ADDR
	la	v0, 0xba0003f8
	#la	v0, 0xba0002f8
	bne	v0, v1, 1b
	nop

	j	ra
	nop	
END(tgt_putchar_COM1)


#include "i2c.S"
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

#define Index_Store_Tag_D			0x09
#define Index_Invalidate_I			0x00
#define Index_Writeback_Inv_D			0x01
#define Index_Store_Tag_S			0x0b
#define Index_Writeback_Inv_S			0x03

LEAF(nullfunction)
	jr ra
	nop
END(nullfunction)

#define CP0_ECC  $26
LEAF(scache_init)
#        daddi   sp, sp, 0xfff8
#        sd      ra, 0(sp)
	move	t7, ra
#if 0 /* gx 2G */
        .word 0x40028001 #mfc0    v0,c0_config1
        and     v0, 0xf 
        beqz    v0, 1f  
        nop     
        jr      ra      
        nop     
1:
#endif

        lui     a0, 0x8000
        lui     a2, 0x0010      #4M/4way
        #lui     a2, 0x0002      #512k/4way
        #lui     a2, 0x0004      #1M/4way
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
/*
scache_flush_4way:
        addu    v0, $0, a0
        addu    v1, a0, a2
1:      slt     a3, v0, v1
        beq     a3, $0, 1f
        nop
        cache   Index_Writeback_Inv_S, 0x0(v0)
        cache   Index_Writeback_Inv_S, 0x1(v0)
        cache   Index_Writeback_Inv_S, 0x2(v0)
        cache   Index_Writeback_Inv_S, 0x3(v0)
        beq     $0, $0, 1b
        addiu   v0, v0, 0x20
1:
*/
scache_init_finish:
#	TTYDBG	("\r\nscache init ok\r\n")

#        ld      ra, 0(sp)
        jr      ra
        nop
nop
#        daddiu  sp, sp, 8
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

#if  1 /* slave main */

	.ent	slave_main
slave_main:

#if 1
	mfc0	t0, CP0_STATUS
        li      t1, 0x00e0      # {cu3,cu2,cu1,cu0}<={0110, status_fr<=1
        or      t0, t0, t1

        mtc0    t0, CP0_STATUS

	mfc0    t0, CP0_STATUS
	#lui     t1, 0x10
	#and     t0, t0, t1      #sr 
	lui     t1, 0x40        #bev
	or      t0, t0, t1
	mtc0    t0, CP0_STATUS
	mtc0    zero, CP0_CAUSE

        #PRINTSTR("CP0_STATUS:")
        #mfc0    a0, CP0_STATUS
        #bal	    hexserial
        #nop
        #PRINTSTR("\r\n\r\n")

        #PRINTSTR("hello world!!\r\n");


#endif

#if 1 

    dli     t0, CORE0_BUF0 #buf of cpu0
myidle:
    lw      a0, 0(t0)
    beqz    a0, myidle
    nop
#endif

#if 0
    li      a0, 0x100000
myidle:    
    addiu   a0, -1
    bnez    a0, myidle
    nop
#endif

//PRINTSTR("$$$$$$$$!!\r\n");

#if 1 /* gx */
        bal     godson2_cache_init
        nop

	mfc0	t0, CP0_CONFIG
	ori	    t0, t0, 7
	xori	t0, t0, 4
	mtc0	t0, CP0_CONFIG

	la	t0, next
#if 1
	dli	t1, 0xfffff
	and     t0, t1
	dli	t1, 0xffffffff9fc00000
	or	t0, t1
#endif	
	
	jr	t0
	nop
next:

        ## enable kseg0 cachablilty####
        mfc0    t0, CP0_CONFIG
        lui     t1, 0xffff
        ori     t1, t1, 0xfff8
        and     t0, t0, t1
        ori     t0, t0, 0x3
        mtc0    t0, CP0_CONFIG
        ################################
#endif

#if 0
############
        mtc0    $0, CP0_WIRED
        mtc0    $0, CP0_PAGEMASK
tlb_flush_all_slave:
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
############
#endif

    .set mips64
    mfc0    t2, $15, 1
    andi    t2, 0x3ff
    .set mips3
########
#define FN_OFF 0x020
#define SP_OFF 0x028
#define GP_OFF 0x030
#define A1_OFF 0x038

    dli     t0, 0x900000003ff01000 
    andi    t3, t2, 0x3  #local cpuid
    sll     t3, 8
    or      t0, t0, t3

    andi    t4, t2, 0xc  #node id
    dsll     t4, 42
    or      t0, t0, t4


waitforinit:   

#if 0 /* gx */
    .set mips64
    mfc0    s1, $15, 1
    andi    s1, 0x3ff
    .set mips3
	move	a0, s1
	bal	hexserial
	nop
	//move	a0, v0
	//bal	hexserial
	//nop
	//move	a0, s1
	//bal	hexserial
	//nop
#endif	
    li      a0, 0x1000000
idle1000:    
    addiu   a0, -1
    bnez    a0, idle1000
    nop

    lw      v0, FN_OFF(t0)
    beqz    v0, waitforinit
    nop

#if 0 /* gx */
    move	a0, v0
    bal	        hexserial
    nop
#endif

    dli      t1, 0xffffffff00000000 
    or       v0, t1

    dli      t1, 0x9800000000000000 
    lw      sp, SP_OFF(t0)
    or      sp, t1
    lw      gp, GP_OFF(t0)
    or      gp, t1
    lw      a1, A1_OFF(t0)

//PRINTSTR("&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&& !!\r\n");

    jalr    v0  #byebye 
    nop

#######
1:  b   1b
    nop

	.end	slave_main


#endif


#######################################
#define REG_ADDRESS 0x0
//#define CONFIG_BASE 0xaff00000
//#define CONFIG_BASE 0x900000002ff00000
#define CONFIG_BASE 0x900000000ff00000
    

        .global ddr2_config
        .ent    ddr2_config
        .set    noreorder
        .set    mips3
ddr2_config:
    la      t0, ddr2_reg_data
    addu    t0, t0, s0
    li      t1, 152 ##0x72
    #dli      t2, CONFIG_BASE
    #move    t2, a0         #old:li      t2, CONFIG_BASE
    daddiu	t2, a0, 0x0

reg_write:

    ld      a1, 0x0(t0)
    sd      a1, REG_ADDRESS(t2)

    subu    t1, t1, 0x1
    addiu   t0, t0, 0x8
    bne     t1, $0, reg_write
    daddiu   t2, t2, 0x10
    
    li      t2, 0xfff
1:
    bnez    t2, 1b
    addi    t2, t2, -1
    nop
    sync
    ############start##########
    #dli      t2, CONFIG_BASE
    #move    t2, a0         #old:li      t2, CONFIG_BASE
    daddiu	t2, a0, 0x0
    la		t0,MC0_CTL_start_DATA_LO
    #la		t0,ddr2_start_reg
    addu    	t0, t0, s0
    ld      a1, 0x0(t0)
    sd      a1, 0x30(t2)


    daddiu  t2, a0, 0x960 //wait int status 
1:
    ld      a1, 0x0(t2)
    andi    a1, a1, 0x100
    beqz    a1, 1b
    nop

    daddiu  t2, a0, 0x30 //Set to srefresh
    dli     a1, 0x0101010100000000
    sd      a1, 0x0(t2)

    li      t2, 0xfff
1:
    bnez    t2, 1b
    addi    t2, t2, -1
    nop
    sync

    daddiu  t2, a0, 0x30 //Out of srefresh
    dli     a1, 0x0101010000000000
    sd      a1, 0x0(t2)

    li      t2, 0xfff
1:
    bnez    t2, 1b
    addi    t2, t2, -1
    nop
    sync

    daddiu  t2, a0, 0x40 //Write mode regs
    dli     a1, 0x0100010101010101
    sd      a1, 0x0(t2)


    jr      ra
    nop
    .end    ddr2_config


        .global ddr2_config_mc1
        .ent    ddr2_config_mc1
        .set    noreorder
        .set    mips3
ddr2_config_mc1:
    la      t0, ddr2_reg_data_mc1
    addu    t0, t0, s0
    li      t1, 152 ##0x72
    #dli      t2, CONFIG_BASE
    #move    t2, a0         #old:li      t2, CONFIG_BASE
    daddiu	t2, a0, 0x0

reg_write_mc1:

    ld      a1, 0x0(t0)
    sd      a1, REG_ADDRESS(t2)

    subu    t1, t1, 0x1
    addiu   t0, t0, 0x8
    bne     t1, $0, reg_write_mc1
    daddiu   t2, t2, 0x10
    
    li      t2, 0xfff
1:
    bnez    t2, 1b
    addi    t2, t2, -1
    nop
    sync
    ############start##########
    #dli      t2, CONFIG_BASE
    #move    t2, a0         #old:li      t2, CONFIG_BASE
    daddiu	t2, a0, 0x0
    la		t0,MC1_CTL_start_DATA_LO
    #la		t0,ddr2_start_reg
    addu    	t0, t0, s0
    ld      a1, 0x0(t0)
    sd      a1, 0x30(t2)

    daddiu  t2, a0, 0x960 //wait int status 
1:
    ld      a1, 0x0(t2)
    andi    a1, a1, 0x100
    beqz    a1, 1b
    nop

    daddiu  t2, a0, 0x30 //Set to srefresh
    dli     a1, 0x0101010100000000
    sd      a1, 0x0(t2)

    li      t2, 0xfff
1:
    bnez    t2, 1b
    addi    t2, t2, -1
    nop
    sync

    daddiu  t2, a0, 0x30 //Out of srefresh
    dli     a1, 0x0101010000000000
    sd      a1, 0x0(t2)

    li      t2, 0xfff
1:
    bnez    t2, 1b
    addi    t2, t2, -1
    nop
    sync

    daddiu  t2, a0, 0x40 //Write mode regs
    dli     a1, 0x0100010101010101
    sd      a1, 0x0(t2)

    jr      ra
    nop
    .end    ddr2_config_mc1


	.data
	.align	5
data_aaa0:
        .dword 0x0

	.rdata
	.align 5
    .global ddr2_reg_data
ddr2_reg_data:
MC0_CTL_000 : .dword 0x0000000000000101
//000000000000000_0 concurrentap(RW) 0000000_1 bank_split_en(RW) 0000000_0 auto_refresh_mode(RW) 0000000_0 arefresh(WR) 0000000_0 ap(RW) 0000000_1 addr_cmp_en(RW) 0000000_1 active_aging(RW) 
MC0_CTL_010 : .dword 0x0000010100010000
//0000000_0 fwc(WR) 0000000_0 fast_write(RW) 0000000_1 enable_quick_srefresh(RW) 0000000_1 eight_bank_mode(RW) 0000000_0 ecc_disable_w_uc_err(RW) 0000000_1 dqs_n_en(RW) 0000000_0 dll_bypass_mode(RW) 0000000_0 dlllockreg(RD) 
MC0_CTL_020 : .dword 0x0100010100000000
//0000000_1 priority_en(RW) 0000000_0 power_down(RW) 0000000_1 placement_en(RW) 0000000_1 odt_add_turn_clk_en(RW) 0000000_0 no_cmd_init(RW) 0000000_0 intrptwritea(RW) 0000000_0 intrptreada(RW) 0000000_0 intrptapburst(RW) 
MC0_CTL_030 : .dword 0x0101000001000000
//0000000_1 swap_port_rw_same_en(RW) 0000000_1 swap_en(RW) 0000000_0 start(RW) 0000000_0 srefresh(RW+) 0000000_1 rw_same_en(RW) 0000000_0 reg_dimm_enable(RW) 0000000_0 reduc(RW) 0000000_0 pwrup_srefresh_exit(RW) 
MC0_CTL_040 : .dword 0x0100010100010101
//000000_01 rtt_0(RW) 000000_11 ctrl_raw(RW) 000000_01 axi0_w_priority(RW) 000000_01 axi0_r_priority(RW) 0000000_0 write_modereg(WR) 0000000_1 writeinterp(RW) 0000000_1 tref_enable(RW) 0000000_1 tras_lockout(RW) 
MC0_CTL_050 : .dword 0x0000000404050100
//00000_000 q_fullness(RW) 00000_000 port_data_error_type(RD) 000_00000 out_of_range_type(RD) 00000_000 max_cs_reg(RD) 00000_100 column_size(RW) 0000_0101 caslat(RW) 00000_010 addr_pins(RW) 00000000
MC0_CTL_060 : .dword 0x0a04040603040003
//0000_1010 aprebit(RW) 0000_0101 wrlat(RW) 0000_0100 twtr(RW) 000_00110 twr_int(RW) 00000_011 trtp(RW) 00000_100 trrd(RW) 0000000000000_011 tcke(RW) 
MC0_CTL_070 : .dword 0x0f0e020000040a08
//0000_0000 max_row_reg(RD) 0000_0000 max_col_reg(RD) 0000_0000 initaref(RW) 00000000000000000000_1111 cs_map(RW) 000_01010 caslat_lin_gate(RW) 000_01010 caslat_lin(RW) 
MC0_CTL_080 : .dword 0x0104040101040401
//0000_0001 odt_wr_map_cs3(RW) 0000_0010 odt_wr_map_cs2(RW) 0000_0100 odt_wr_map_cs1(RW) 0000_1000 odt_wr_map_cs0(RW) 0000_0001 odt_rd_map_cs3(RW) 0000_0010 odt_rd_map_cs2(RW) 0000_0100 odt_rd_map_cs1(RW) 0000_1000 odt_rd_map_cs0(RW) 
MC0_CTL_090 : .dword 0x0000050b00000000
//000_00000 ocd_adjust_pup_cs_0(RW) 000_00000 ocd_adjust_pdn_cs_0(RW) 0000_0101 trp(RW) 000_01011 tdal(RW) 000000000000_0000 port_cmd_error_type(RD) 0000000000000000
//MC0_CTL_0a0 : .dword 0x0000003f3f140612
MC0_CTL_0a0 : .dword 0x0000003f3f14021b
//00000000000000000000000000_111111 command_age_count(RW) 00_111111 age_count(RW) 000_10100 trc(RW) 000_00110 tmrd(RW) 000_10010 tfaw(RW) 
MC0_CTL_0b0 : .dword 0x0000000000000000
MC0_CTL_0c0 : .dword 0x00002c050f000000
//000000000000000000101100 trfc(RW) 00000101 trcd_int(RW) 00001111 tras_min(RW) 00000000 out_of_range_length(RD) 00000000 ecc_u_synd(RD) 00000000 ecc_c_synd(RD) 
MC0_CTL_0d0 : .dword 0x0000000000000000
MC0_CTL_0e0 : .dword 0x0000000000000000
MC0_CTL_0f0 : .dword 0x0000000000000000
MC0_CTL_100 : .dword 0x0000000000000000
MC0_CTL_110 : .dword 0x0000000000000c2d
//0_000000000000000 emrs2_data_1(RW) 0_000000000000000 emrs2_data_0(RW) 000000000000000000_00110000101101 tref(RW) 
MC0_CTL_120 : .dword 0x001c000000000000
//0000000000011100 axi0_en_size_lt_width_instr(RW) 00000000000000000_000000000000000 emrs2_data_3(RW) 0_000000000000000 emrs2_data_2(RW) 
MC0_CTL_130 : .dword 0x0d56000302000000
//0110110101010110 tras_max(RW) 0000000000000011 tpdex(RW) 0000001000000000 tdll(RW) 0000000000000000 tcpd(RW) 
MC0_CTL_140 : .dword 0x0000204002000030
//0000000000000000 xor_check_bits(RW) 0000000000000000 version(RD) 0000001000000000 txsr(RW) 0000000000110000 txsnr(RW) 
MC0_CTL_150 : .dword 0x0000000000300004
//000_0000000000000000000000000000000000000 ecc_c_addr(RD) 000000000000000000011011 tinit(RW) 
MC0_CTL_160 : .dword 0x0000000000000000
//000000000000000000000000000_0000000000000000000000000000000000000 ecc_u_addr(RD) 
MC0_CTL_170 : .dword 0x0000000000000000
//000000000000000000000000000_0000000000000000000000000000000000000 out_of_range_addr(RD) 
MC0_CTL_180 : .dword 0x0000000000000000
//000000000000000000000000000_0000000000000000000000000000000000000 port_cmd_error_addr(RD) 
MC0_CTL_190 : .dword 0x0000000000000000
//0000000000000000000000000000000000000000000000000000000000000000 ecc_c_data(RD) 
MC0_CTL_1a0 : .dword 0x0000000000000000
//0000000000000000000000000000000000000000000000000000000000000000 ecc_u_data(RD) 
MC0_CTL_1b0 : .dword 0x0000000000000000
//0000000000000000000000000000000000000000000000000000000000000_000 cke_delay(RW) 
MC0_CTL_1c0 : .dword 0x0000000000000000
MC0_CTL_1d0 : .dword 0x0203070400000101
//0000_0010 tdfi_phy_wrlat_base(RW) 0000_0000 tdfi_phy_wrlat(RD) 000_00111 tdfi_phy_rdlat(RW) 0000_0000 tdfi_ctrlupd_min(RD) 000000000000_0000 dram_clk_disable(RW) 0000000_1 odt_alt_en(RW) 0000000_1 drive_dq_dqs(RW) 
MC0_CTL_1e0 : .dword 0x0c2d0c2d0c2d0205
//00_00000000000000 tdfi_phyupd_type0(RD) 00_00000000000000 tdfi_phyupd_resp(RD) 00_00000000000000 tdfi_ctrlupd_max(RD) 000_00000 tdfi_rddata_en_base(RW) 000_00000 tdfi_rddata_en(RD) 
#ifdef DDR_DEBUG
//MC0_CTL_1f0 : .dword 0x00120e8000000000
//MC0_CTL_200 : .dword 0x00160e8000160e80
//MC0_CTL_210 : .dword 0x00100e8000130e80
////00000000001000000000111010000000 dll_ctrl_reg_0_4(RW) 00000000001000000000111010000000 dll_ctrl_reg_0_3(RW) 
//MC0_CTL_220 : .dword 0x00130e8000010e80
////00000000001000000000111010000000 dll_ctrl_reg_0_6(RW) 00000000001000000000111010000000 dll_ctrl_reg_0_5(RW) 
//MC0_CTL_230 : .dword 0x00200e8000100e80

#debug write
MC0_CTL_1f0 : .dword 0x00000e8000000000
MC0_CTL_200 : .dword 0x00000e8000000e80
MC0_CTL_210 : .dword 0x00000e8000000e80
//00000000001000000000111010000000 dll_ctrl_reg_0_4(RW) 00000000001000000000111010000000 dll_ctrl_reg_0_3(RW) 
MC0_CTL_220 : .dword 0x00000e8000000e80
//00000000001000000000111010000000 dll_ctrl_reg_0_6(RW) 00000000001000000000111010000000 dll_ctrl_reg_0_5(RW) 
MC0_CTL_230 : .dword 0x00200e8000000e80

#debug read
//MC0_CTL_1f0 : .dword 0x00120e8000000000
//MC0_CTL_200 : .dword 0x00160e8000160e80
//MC0_CTL_210 : .dword 0x00100e8000140e80
////00000000001000000000111010000000 dll_ctrl_reg_0_4(RW) 00000000001000000000111010000000 dll_ctrl_reg_0_3(RW) 
//MC0_CTL_220 : .dword 0x00100e8000120e80
////00000000001000000000111010000000 dll_ctrl_reg_0_6(RW) 00000000001000000000111010000000 dll_ctrl_reg_0_5(RW) 
//MC0_CTL_230 : .dword 0x00200e8000140e80

#else
MC0_CTL_1f0 : .dword 0x00200e8000000000
MC0_CTL_200 : .dword 0x00200e8000200e80
MC0_CTL_210 : .dword 0x00200e8000200e80
//00000000001000000000111510000000 dll_ctrl_reg_0_4(RW) 00000000001000000000111010000000 dll_ctrl_reg_0_3(RW) 
MC0_CTL_220 : .dword 0x00200e8000200e80
//00000000001000000000111510000000 dll_ctrl_reg_0_6(RW) 00000000001000000000111010000000 dll_ctrl_reg_0_5(RW) 
MC0_CTL_230 : .dword 0x00200e8000200e80

//MC0_CTL_1f0 : .dword 0x00130e8000000000
//MC0_CTL_200 : .dword 0x00130e8000130e80
//MC0_CTL_210 : .dword 0x00130e8000130e80
////00000000001000000000111510000000 dll_ctrl_reg_0_4(RW) 00000000001000000000111010000000 dll_ctrl_reg_0_3(RW) 
//MC0_CTL_220 : .dword 0x00100e8000130e80
////00000000001000000000111510000000 dll_ctrl_reg_0_6(RW) 00000000001000000000111010000000 dll_ctrl_reg_0_5(RW) 
//MC0_CTL_230 : .dword 0x00150e8000120e80

//MC0_CTL_1f0 : .dword 0x00230eff00000000
//MC0_CTL_200 : .dword 0x00230eff00230eff
//MC0_CTL_210 : .dword 0x00230eff00230eff
////00000000001000000000111510000000 dll_ctrl_reg_0_4(RW) 00000000001000000000111010000000 dll_ctrl_reg_0_3(RW) 
//MC0_CTL_220 : .dword 0x00200eff00230eff
////00000000001000000000111510000000 dll_ctrl_reg_0_6(RW) 00000000001000000000111010000000 dll_ctrl_reg_0_5(RW) 
//MC0_CTL_230 : .dword 0x00150eff00200eff

//MC0_CTL_1f0 : .dword 0x00120e8000000000
//MC0_CTL_200 : .dword 0x00160e8000160e80
//MC0_CTL_210 : .dword 0x00100e8000180e80
////00000000001000000000111010000000 dll_ctrl_reg_0_4(RW) 00000000001000000000111010000000 dll_ctrl_reg_0_3(RW) 
//MC0_CTL_220 : .dword 0x000c0e8000120e80
////00000000001000000000111010000000 dll_ctrl_reg_0_6(RW) 00000000001000000000111010000000 dll_ctrl_reg_0_5(RW) 
//MC0_CTL_230 : .dword 0x00200e8000100e80

#150M MC0
//MC0_CTL_1f0 : .dword 0x001a0e8000000000
//MC0_CTL_200 : .dword 0x00120e8000120e80
//MC0_CTL_210 : .dword 0x00100e8000120e80
////00000000001000000000111010000000 dll_ctrl_reg_0_4(RW) 00000000001000000000111010000000 dll_ctrl_reg_0_3(RW) 
//MC0_CTL_220 : .dword 0x00100e8000120e80
////00000000001000000000111010000000 dll_ctrl_reg_0_6(RW) 00000000001000000000111010000000 dll_ctrl_reg_0_5(RW) 
//MC0_CTL_230 : .dword 0x00200e8000120e80

#150M MC0
//MC0_CTL_1f0 : .dword 0x00160e8000000000
//MC0_CTL_200 : .dword 0x00100e8000100e80
//MC0_CTL_210 : .dword 0x00100e8000160e80
////00000000001000000000111010000000 dll_ctrl_reg_0_4(RW) 00000000001000000000111010000000 dll_ctrl_reg_0_3(RW) 
//MC0_CTL_220 : .dword 0x00160e8000120e80
////00000000001000000000111010000000 dll_ctrl_reg_0_6(RW) 00000000001000000000111010000000 dll_ctrl_reg_0_5(RW) 
//MC0_CTL_230 : .dword 0x00200e8000100e80
#endif
MC0_CTL_240 : .dword 0x00000e0000000e00
//00000000000000000000111000000000 dll_ctrl_reg_1_1(RW) 00000000000000000000111000000000 dll_ctrl_reg_1_0(RW) 
MC0_CTL_250 : .dword 0x00000e0000000e00
MC0_CTL_260 : .dword 0x00000e0000000e00
MC0_CTL_270 : .dword 0x00000e0000000e00
MC0_CTL_280 : .dword 0x0000000000000e00
MC0_CTL_290 : .dword 0x0000000000000000
MC0_CTL_2a0 : .dword 0x0000000000000000
MC0_CTL_2b0 : .dword 0x0000000000000000
MC0_CTL_2c0 : .dword 0x0000000000000000
MC0_CTL_2d0 : .dword 0xf3004a480000019d
//11110100000000000011101100100111 phy_ctrl_reg_0_0(RD) 000000_00000000000000000110011101 pad_ctrl_reg_0(RW) 
MC0_CTL_2e0 : .dword 0xf3004a48f3004a48
MC0_CTL_2f0 : .dword 0xf3004a48f3004a48
MC0_CTL_300 : .dword 0xf3004a48f3004a48
MC0_CTL_310 : .dword 0xf3004a48f3004a48
//MC0_CTL_2d0 : .dword 0xf3005a470000019d
////11110100000000000011101100100111 phy_ctrl_reg_0_0(RD) 000000_00000000000000000110011101 pad_ctrl_reg_0(RW) 
//MC0_CTL_2e0 : .dword 0xf3005a47f3005a47
//MC0_CTL_2f0 : .dword 0xf3005a47f3005a47
//MC0_CTL_300 : .dword 0xf3005a47f3005a47
//MC0_CTL_310 : .dword 0xf3005a47f3005a47
MC0_CTL_320 : .dword 0x07c0000007c00000
MC0_CTL_330 : .dword 0x07c0000007c00000
MC0_CTL_340 : .dword 0x07c0000007c00000
MC0_CTL_350 : .dword 0x07c0000007c00000
MC0_CTL_360 : .dword 0x0800c00507c00000
//00000000000000001100000000000101 phy_ctrl_reg_2(RW) 00000111110000000000001100000001 phy_ctrl_reg_1_8(RD) 
MC0_CTL_370 : .dword 0x0000000000000000
MC0_CTL_380 : .dword 0x0000000000000000
MC0_CTL_390 : .dword 0x0000000000000000
MC0_CTL_3a0 : .dword 0x0000000000000000
MC0_CTL_3b0 : .dword 0x0000000000000000
MC0_CTL_3c0 : .dword 0x0000000000000000
MC0_CTL_3d0 : .dword 0x0000000000000000
MC0_CTL_3e0 : .dword 0x0000000000000000
MC0_CTL_3f0 : .dword 0x0000000000000000
MC0_CTL_400 : .dword 0x0000000000000000
MC0_CTL_410 : .dword 0x0000000000000000
MC0_CTL_420 : .dword 0x0000000000000000
MC0_CTL_430 : .dword 0x0000000000000000
MC0_CTL_440 : .dword 0x0000000000000000
MC0_CTL_450 : .dword 0x0000000000000000
MC0_CTL_460 : .dword 0x0000000000000000
MC0_CTL_470 : .dword 0x0000000000000000
MC0_CTL_480 : .dword 0x0000000000000000
MC0_CTL_490 : .dword 0x0000000000000000
MC0_CTL_4a0 : .dword 0x0000000000000000
MC0_CTL_4b0 : .dword 0x0000000000000000
MC0_CTL_4c0 : .dword 0x0000000000000000
MC0_CTL_4d0 : .dword 0x0000000000000000
MC0_CTL_4e0 : .dword 0x0000000000000000
MC0_CTL_4f0 : .dword 0x0000000000000000
MC0_CTL_500 : .dword 0x0000000000000000
MC0_CTL_510 : .dword 0x0000000000000000
MC0_CTL_520 : .dword 0x0000000000000000
MC0_CTL_530 : .dword 0x0000000000000000
MC0_CTL_540 : .dword 0x0000000000000000
MC0_CTL_550 : .dword 0x0000000000000000
MC0_CTL_560 : .dword 0x0000000000000000
MC0_CTL_570 : .dword 0x0000000000000000
MC0_CTL_580 : .dword 0x0000000000000000
MC0_CTL_590 : .dword 0x0000000000000000
MC0_CTL_5a0 : .dword 0x0000000000000000
MC0_CTL_5b0 : .dword 0x0000000000000000
MC0_CTL_5c0 : .dword 0x0000000000000000
MC0_CTL_5d0 : .dword 0x0000000000000000
MC0_CTL_5e0 : .dword 0x0000000000000000
MC0_CTL_5f0 : .dword 0x0000000000000000
MC0_CTL_600 : .dword 0x0000000000000000
MC0_CTL_610 : .dword 0x0000000000000000
MC0_CTL_620 : .dword 0x0000000000000000
MC0_CTL_630 : .dword 0x0000000000000000
MC0_CTL_640 : .dword 0x0000000000000000
MC0_CTL_650 : .dword 0x0000000000000000
MC0_CTL_660 : .dword 0x0000000000000000
MC0_CTL_670 : .dword 0x0000000000000000
MC0_CTL_680 : .dword 0x0000000000000000
MC0_CTL_690 : .dword 0x0000000000000000
MC0_CTL_6a0 : .dword 0x0000000000000000
MC0_CTL_6b0 : .dword 0x0000000000000000
MC0_CTL_6c0 : .dword 0x0000000000000000
MC0_CTL_6d0 : .dword 0x0000000000000000
MC0_CTL_6e0 : .dword 0x0000000000000000
MC0_CTL_6f0 : .dword 0x0000000000000000
MC0_CTL_700 : .dword 0x0000000000000000
MC0_CTL_710 : .dword 0x0000000000000000
MC0_CTL_720 : .dword 0x0000000000000000
MC0_CTL_730 : .dword 0x0000000000000000
MC0_CTL_740 : .dword 0x0100000000000000
//MC0_CTL_750 : .dword 0x0100000101020101
MC0_CTL_750 : .dword 0x0101000101020101
//000000_01 wrlvl_cs(RW) 000000_00 sw_leveling_mode(RW) 000000_00 rdlvl_cs(RW) 000000_01 axi2_w_priority(RW) 000000_01 axi2_r_priority(RW) 000000_10 axi2_port_ordering(RW) 000000_01 axi1_w_priority(RW) 000000_01 axi1_r_priority(RW) 
MC0_CTL_760 : .dword 0x0303030000020001
//0000_0011 axi0_priority2_relative_priority(RW) 0000_0011 axi0_priority1_relative_priority(RW) 0000_0011 axi0_priority0_relative_priority(RW) 0000_0000 address_mirroring(RW) 00000_000 tdfi_dram_clk_disable(RW) 00000_010 bstlen(RW) 000000_00 zq_req(WR) 000000_01 zq_on_sref_exit(RW) 
MC0_CTL_770 : .dword 0x0101010202020203
//0000_0001 axi2_priority2_relative_priority(RW) 0000_0001 axi2_priority1_relative_priority(RW) 0000_0001 axi2_priority0_relative_priority(RW) 0000_0010 axi1_priority3_relative_priority(RW) 0000_0010 axi1_priority2_relative_priority(RW) 0000_0010 axi1_priority1_relative_priority(RW) 0000_0010 axi1_priority0_relative_priority(RW) 0000_0011 axi0_priority3_relative_priority(RW) 
MC0_CTL_780 : .dword 0x0102020400040001
//0000_0001 tdfi_dram_clk_enable(RW) 0000_0010 tdfi_ctrl_delay(RW) 0000_0010 rdlvl_gate_dq_zero_count(RW) 0000_0100 rdlvl_dq_zero_count(RW) 0000_0000 lowpower_refresh_enable(RW) 0000_0110 dram_class(RW) 0000_1100 burst_on_fly_bit(RW) 0000_0001 axi2_priority3_relative_priority(RW) 
MC0_CTL_790 : .dword 0x281900000f000303
//00_101000 wlmrd(RW) 00_011001 wldqsen(RW) 000_00000 lowpower_control(RW) 000_00000 lowpower_auto_enable(RW) 0000_1111 zqcs_chip(RW) 0000_0000 wrr_param_value_err(RD) 0000_0011 tdfi_wrlvl_dll(RW) 0000_0011 tdfi_rdlvl_dll(RW) 
MC0_CTL_7a0 : .dword 0x00000000000000ff
MC0_CTL_7b0 : .dword 0x0000000000000000
MC0_CTL_7c0 : .dword 0x0000000000000000
MC0_CTL_7d0 : .dword 0x0000000000000000
MC0_CTL_7e0 : .dword 0x0000000000000000
//00000000 rdlvl_gate_delay_2(RD) 00000000 rdlvl_gate_delay_1(RD) 00000000 rdlvl_gate_delay_0(RD) 00000000 rdlvl_gate_clk_adjust_8(RW) 00000000 rdlvl_gate_clk_adjust_7(RW) 00000000 rdlvl_gate_clk_adjust_6(RW) 00000000 rdlvl_gate_clk_adjust_5(RW) 00000000 rdlvl_gate_clk_adjust_4(RW) 
MC0_CTL_7f0 : .dword 0xff08000000000000
//11111111 rdlvl_max_delay(RW) 00001000 rdlvl_gate_max_delay(RW) 00000000 rdlvl_gate_delay_8(RD) 00000000 rdlvl_gate_delay_7(RD) 00000000 rdlvl_gate_delay_6(RD) 00000000 rdlvl_gate_delay_5(RD) 00000000 rdlvl_gate_delay_4(RD) 00000000 rdlvl_gate_delay_3(RD) 
MC0_CTL_800 : .dword 0x0000000000000000
//00000000 rdlvl_midpoint_delay_7(RD) 00000000 rdlvl_midpoint_delay_6(RD) 00000000 rdlvl_midpoint_delay_5(RD) 00000000 rdlvl_midpoint_delay_4(RD) 00000000 rdlvl_midpoint_delay_3(RD) 00000000 rdlvl_midpoint_delay_2(RD) 00000000 rdlvl_midpoint_delay_1(RD) 00000000 rdlvl_midpoint_delay_0(RD) 
MC0_CTL_810 : .dword 0x000000000000000e
//00000000 rdlvl_offset_delay_6(RW) 00000000 rdlvl_offset_delay_5(RW) 00000000 rdlvl_offset_delay_4(RW) 00000000 rdlvl_offset_delay_3(RW) 00000000 rdlvl_offset_delay_2(RW) 00000000 rdlvl_offset_delay_1(RW) 00000000 rdlvl_offset_delay_0(RW) 00000000 rdlvl_midpoint_delay_8(RD) 
MC0_CTL_820 : .dword 0x0420000c20400000
//00000100 tdfi_wrlvl_resplat(RW) 00000000 tdfi_wrlvl_resp(RD) 00000000 tdfi_rdlvl_rr(RW) 00001100 tdfi_rdlvl_resplat(RW) 00000000 tdfi_rdlvl_resp(RD) 01000000 refresh_per_zq(RW) 00000000 rdlvl_offset_delay_8(RW) 00000000 rdlvl_offset_delay_7(RW) 
//MC0_CTL_830 : .dword 0x0000000000000c0a
MC0_CTL_830 : .dword 0x1313131313130c0a
//00000000 wrlvl_delay_5(RD) 00000000 wrlvl_delay_4(RD) 00000000 wrlvl_delay_3(RD) 00000000 wrlvl_delay_2(RD) 00000000 wrlvl_delay_1(RD) 00000000 wrlvl_delay_0(RD) 00000010 tmod(RW) 00001010 tdfi_wrlvl_ww(RW) 
MC0_CTL_840 : .dword 0x0000640064001313
//00000000000000_0001100100 axi1_priority_relax(RW) 000000_0001100100 axi0_priority_relax(RW) 00000000 wrlvl_delay_8(RD) 00000000 wrlvl_delay_7(RD) 00000000 wrlvl_delay_6(RD) 
MC0_CTL_850 : .dword 0x0000000000000064
MC0_CTL_860 : .dword 0x0200004000000000
MC0_CTL_870 : .dword 0x0044004400440044
//0_000000000000010 emrs1_data_3(RW) 0_000000000000010 emrs1_data_2(RW) 0_000000000000010 emrs1_data_1(RW) 0_000000000000010 emrs1_data_0(RW) 
MC0_CTL_880 : .dword 0x0000000000000000
MC0_CTL_890 : .dword 0x0a520a520a520a52
//0_000010000010000 mrs_data_3(RW) 0_000010000010000 mrs_data_2(RW) 0_000010000010000 mrs_data_1(RW) 0_000010000010000 mrs_data_0(RW) 
MC0_CTL_8a0 : .dword 0x00000000001c001c
MC0_CTL_8b0 : .dword 0x0000000000000000
MC0_CTL_8c0 : .dword 0x0004000000000000
MC0_CTL_8d0 : .dword 0x00000000c8000000
MC0_CTL_8e0 : .dword 0x0000000000000050
MC0_CTL_8f0 : .dword 0x0000000020202080
//MC0_CTL_8f0 : .dword 0x0000000040404080
//0000000000000000000000000111100_000000000000000000000000001111000 dll_ctrl_reg_2(RW) 
MC0_CTL_900 : .dword 0x0000000000000000
MC0_CTL_910 : .dword 0x0000000000000000
MC0_CTL_920 : .dword 0x0000000000000000
MC0_CTL_930 : .dword 0x0000000000000000
MC0_CTL_940 : .dword 0x0300000000050500
MC0_CTL_950 : .dword 0x0000000000000a03
MC0_CTL_960 : .dword 0x0604000100000000
//000_00101 rdlat_adj(RW) 0000_0100 wrlat_adj(RW) 0000000_0 swlvl_start(WR) 0000000_0 swlvl_load(WR) 0000000_0 swlvl_exit(WR) 000000_000000000000000000 int_status(RD) 
MC0_CTL_970 : .dword 0x000000000003e805
MC0_CTL_start_DATA_LO: .word  0x00000000
//0000000_1 rw_same_en 0000000_0 reg_dimm_enable 0000000_0 reduc 0000000_0 pwrup_srefresh_exit
MC0_CTL_start_DATA_HI: .word  0x01010100
//0000000_1 swap_port_rw_same_en 0000000_1 swap_en 0000000_0 start 0000000_0 srefresh

ddr2_reg_data_mc1:
MC1_CTL_000 : .dword 0x0000000000000101
//000000000000000_0 concurrentap(RW) 0000000_1 bank_split_en(RW) 0000000_0 auto_refresh_mode(RW) 0000000_0 arefresh(WR) 0000000_0 ap(RW) 0000000_1 addr_cmp_en(RW) 0000000_1 active_aging(RW) 
MC1_CTL_010 : .dword 0x0000010100010000
//0000000_0 fwc(WR) 0000000_0 fast_write(RW) 0000000_1 enable_quick_srefresh(RW) 0000000_1 eight_bank_mode(RW) 0000000_0 ecc_disable_w_uc_err(RW) 0000000_1 dqs_n_en(RW) 0000000_0 dll_bypass_mode(RW) 0000000_0 dlllockreg(RD) 
MC1_CTL_020 : .dword 0x0100010100000000
//0000000_1 priority_en(RW) 0000000_0 power_down(RW) 0000000_1 placement_en(RW) 0000000_1 odt_add_turn_clk_en(RW) 0000000_0 no_cmd_init(RW) 0000000_0 intrptwritea(RW) 0000000_0 intrptreada(RW) 0000000_0 intrptapburst(RW) 
MC1_CTL_030 : .dword 0x0101000001000000
//0000000_1 swap_port_rw_same_en(RW) 0000000_1 swap_en(RW) 0000000_0 start(RW) 0000000_0 srefresh(RW+) 0000000_1 rw_same_en(RW) 0000000_0 reg_dimm_enable(RW) 0000000_0 reduc(RW) 0000000_0 pwrup_srefresh_exit(RW) 
MC1_CTL_040 : .dword 0x0101010100010100
//000000_01 rtt_0(RW) 000000_11 ctrl_raw(RW) 000000_01 axi0_w_priority(RW) 000000_01 axi0_r_priority(RW) 0000000_0 write_modereg(WR) 0000000_1 writeinterp(RW) 0000000_1 tref_enable(RW) 0000000_1 tras_lockout(RW) 
MC1_CTL_050 : .dword 0x0000000404050100
//00000_000 q_fullness(RW) 00000_000 port_data_error_type(RD) 000_00000 out_of_range_type(RD) 00000_000 max_cs_reg(RD) 00000_100 column_size(RW) 0000_0101 caslat(RW) 00000_010 addr_pins(RW) 00000000
MC1_CTL_060 : .dword 0x0a04040603040003
//0000_1010 aprebit(RW) 0000_0101 wrlat(RW) 0000_0100 twtr(RW) 000_00110 twr_int(RW) 00000_011 trtp(RW) 00000_100 trrd(RW) 0000000000000_011 tcke(RW) 
MC1_CTL_070 : .dword 0x0f0e020000010a08
//0000_0000 max_row_reg(RD) 0000_0000 max_col_reg(RD) 0000_0000 initaref(RW) 00000000000000000000_1111 cs_map(RW) 000_01010 caslat_lin_gate(RW) 000_01010 caslat_lin(RW) 
MC1_CTL_080 : .dword 0x0104040101000400
//0000_0001 odt_wr_map_cs3(RW) 0000_0010 odt_wr_map_cs2(RW) 0000_0100 odt_wr_map_cs1(RW) 0000_1000 odt_wr_map_cs0(RW) 0000_0001 odt_rd_map_cs3(RW) 0000_0010 odt_rd_map_cs2(RW) 0000_0100 odt_rd_map_cs1(RW) 0000_1000 odt_rd_map_cs0(RW) 
MC1_CTL_090 : .dword 0x0000050b00000000
//000_00000 ocd_adjust_pup_cs_0(RW) 000_00000 ocd_adjust_pdn_cs_0(RW) 0000_0101 trp(RW) 000_01011 tdal(RW) 000000000000_0000 port_cmd_error_type(RD) 0000000000000000
MC1_CTL_0a0 : .dword 0x0000003f3f140612
//00000000000000000000000000_111111 command_age_count(RW) 00_111111 age_count(RW) 000_10100 trc(RW) 000_00110 tmrd(RW) 000_10010 tfaw(RW) 
MC1_CTL_0b0 : .dword 0x0000000000000000
MC1_CTL_0c0 : .dword 0x00002c050f000000
//000000000000000000101100 trfc(RW) 00000101 trcd_int(RW) 00001111 tras_min(RW) 00000000 out_of_range_length(RD) 00000000 ecc_u_synd(RD) 00000000 ecc_c_synd(RD) 
MC1_CTL_0d0 : .dword 0x0000000000000000
MC1_CTL_0e0 : .dword 0x0000000000000000
MC1_CTL_0f0 : .dword 0x0000000000000000
MC1_CTL_100 : .dword 0x0000000000000000
MC1_CTL_110 : .dword 0x0000000000000c2d
//0_000000000000000 emrs2_data_1(RW) 0_000000000000000 emrs2_data_0(RW) 000000000000000000_00110000101101 tref(RW) 
MC1_CTL_120 : .dword 0xffff000000000000
//0000000000011100 axi0_en_size_lt_width_instr(RW) 00000000000000000_000000000000000 emrs2_data_3(RW) 0_000000000000000 emrs2_data_2(RW) 
MC1_CTL_130 : .dword 0x0d56000302000000
//0110110101010110 tras_max(RW) 0000000000000011 tpdex(RW) 0000001000000000 tdll(RW) 0000000000000000 tcpd(RW) 
MC1_CTL_140 : .dword 0x0000204002000030
//0000000000000000 xor_check_bits(RW) 0000000000000000 version(RD) 0000001000000000 txsr(RW) 0000000000110000 txsnr(RW) 
MC1_CTL_150 : .dword 0x0000000011000004
//000_0000000000000000000000000000000000000 ecc_c_addr(RD) 000000000000000000011011 tinit(RW) 
MC1_CTL_160 : .dword 0x0000000000000000
//000000000000000000000000000_0000000000000000000000000000000000000 ecc_u_addr(RD) 
MC1_CTL_170 : .dword 0x0000000000000000
//000000000000000000000000000_0000000000000000000000000000000000000 out_of_range_addr(RD) 
MC1_CTL_180 : .dword 0x0000000000000000
//000000000000000000000000000_0000000000000000000000000000000000000 port_cmd_error_addr(RD) 
MC1_CTL_190 : .dword 0x0000000000000000
//0000000000000000000000000000000000000000000000000000000000000000 ecc_c_data(RD) 
MC1_CTL_1a0 : .dword 0x0000000000000000
//0000000000000000000000000000000000000000000000000000000000000000 ecc_u_data(RD) 
MC1_CTL_1b0 : .dword 0x0000000000000000
//0000000000000000000000000000000000000000000000000000000000000_000 cke_delay(RW) 
MC1_CTL_1c0 : .dword 0x0000000000000000
MC1_CTL_1d0 : .dword 0x0203070400000101
//0000_0010 tdfi_phy_wrlat_base(RW) 0000_0000 tdfi_phy_wrlat(RD) 000_00111 tdfi_phy_rdlat(RW) 0000_0000 tdfi_ctrlupd_min(RD) 000000000000_0000 dram_clk_disable(RW) 0000000_1 odt_alt_en(RW) 0000000_1 drive_dq_dqs(RW) 
MC1_CTL_1e0 : .dword 0x0c2d0c2d0c2d0205
//00_00000000000000 tdfi_phyupd_type0(RD) 00_00000000000000 tdfi_phyupd_resp(RD) 00_00000000000000 tdfi_ctrlupd_max(RD) 000_00000 tdfi_rddata_en_base(RW) 000_00000 tdfi_rddata_en(RD) 
#ifdef DDR_DEBUG
//MC1_CTL_1f0 : .dword 0x00120e8000000000
//MC1_CTL_200 : .dword 0x00160e8000160e80
//MC1_CTL_210 : .dword 0x00100e8000130e80
////00000000001000000000111010000000 dll_ctrl_reg_0_4(RW) 00000000001000000000111010000000 dll_ctrl_reg_0_3(RW) 
//MC1_CTL_220 : .dword 0x00130e8000010e80
////00000000001000000000111010000000 dll_ctrl_reg_0_6(RW) 00000000001000000000111010000000 dll_ctrl_reg_0_5(RW) 
//MC1_CTL_230 : .dword 0x00200e8000100e80

//MC1_CTL_1f0 : .dword 0x00000e8000000000
//MC1_CTL_200 : .dword 0x00200e8000200e80
//MC1_CTL_210 : .dword 0x00200e8000200e80
////00000000001000000000111510000000 dll_ctrl_reg_0_4(RW) 00000000001000000000111010000000 dll_ctrl_reg_0_3(RW) 
//MC1_CTL_220 : .dword 0x00200e8000200e80
////00000000001000000000111510000000 dll_ctrl_reg_0_6(RW) 00000000001000000000111010000000 dll_ctrl_reg_0_5(RW) 
//MC1_CTL_230 : .dword 0x00200e8000200e80

#debug write
MC1_CTL_1f0 : .dword 0x00000e8000000000
MC1_CTL_200 : .dword 0x00000e8000000e80
MC1_CTL_210 : .dword 0x00000e8000000e80
//00000000001000000000111010000000 dll_ctrl_reg_0_4(RW) 00000000001000000000111010000000 dll_ctrl_reg_0_3(RW) 
MC1_CTL_220 : .dword 0x00000e8000000e80
//00000000001000000000111010000000 dll_ctrl_reg_0_6(RW) 00000000001000000000111010000000 dll_ctrl_reg_0_5(RW) 
MC1_CTL_230 : .dword 0x00200e8000000e80

#debug read
//MC1_CTL_1f0 : .dword 0x00120e8000000000
//MC1_CTL_200 : .dword 0x00160e8000160e80
//MC1_CTL_210 : .dword 0x00100e8000140e80
////00000000001000000000111010000000 dll_ctrl_reg_0_4(RW) 00000000001000000000111010000000 dll_ctrl_reg_0_3(RW) 
//MC1_CTL_220 : .dword 0x00100e8000120e80
////00000000001000000000111010000000 dll_ctrl_reg_0_6(RW) 00000000001000000000111010000000 dll_ctrl_reg_0_5(RW) 
//MC1_CTL_230 : .dword 0x00200e8000140e80

#else
MC1_CTL_1f0 : .dword 0x00200e8000000000
MC1_CTL_200 : .dword 0x00200e8000200e80
MC1_CTL_210 : .dword 0x00200e8000200e80
//00000000001000000000111510000000 dll_ctrl_reg_0_4(RW) 00000000001000000000111010000000 dll_ctrl_reg_0_3(RW) 
MC1_CTL_220 : .dword 0x00200e8000200e80
//00000000001000000000111510000000 dll_ctrl_reg_0_6(RW) 00000000001000000000111010000000 dll_ctrl_reg_0_5(RW) 
MC1_CTL_230 : .dword 0x00200e8000200e80

//MC1_CTL_1f0 : .dword 0x00000e8000000000
//MC1_CTL_200 : .dword 0x00000e8000000e80
//MC1_CTL_210 : .dword 0x00000e8000000e80
////00000000001000000000111510000000 dll_ctrl_reg_0_4(RW) 00000000001000000000111010000000 dll_ctrl_reg_0_3(RW) 
//MC1_CTL_220 : .dword 0x00000e8000000e80
////00000000001000000000111510000000 dll_ctrl_reg_0_6(RW) 00000000001000000000111010000000 dll_ctrl_reg_0_5(RW) 
//MC1_CTL_230 : .dword 0x00000e8000000e80

//MC1_CTL_1f0 : .dword 0x00230eff00000000
//MC1_CTL_200 : .dword 0x00230eff00230eff
//MC1_CTL_210 : .dword 0x00230eff00230eff
////00000000001000000000111510000000 dll_ctrl_reg_0_4(RW) 00000000001000000000111010000000 dll_ctrl_reg_0_3(RW) 
//MC1_CTL_220 : .dword 0x00200eff00230eff
////00000000001000000000111510000000 dll_ctrl_reg_0_6(RW) 00000000001000000000111010000000 dll_ctrl_reg_0_5(RW) 
//MC1_CTL_230 : .dword 0x00150eff00200eff

//MC1_CTL_1f0 : .dword 0x00120e8000000000
//MC1_CTL_200 : .dword 0x00160e8000160e80
//MC1_CTL_210 : .dword 0x00100e8000180e80
////00000000001000000000111010000000 dll_ctrl_reg_0_4(RW) 00000000001000000000111010000000 dll_ctrl_reg_0_3(RW) 
//MC1_CTL_220 : .dword 0x000c0e8000120e80
////00000000001000000000111010000000 dll_ctrl_reg_0_6(RW) 00000000001000000000111010000000 dll_ctrl_reg_0_5(RW) 
//MC1_CTL_230 : .dword 0x00200e8000100e80

#150M MC1
//MC1_CTL_1f0 : .dword 0x001a0e8000000000
//MC1_CTL_200 : .dword 0x00120e8000120e80
//MC1_CTL_210 : .dword 0x00100e8000120e80
////00000000001000000000111010000000 dll_ctrl_reg_0_4(RW) 00000000001000000000111010000000 dll_ctrl_reg_0_3(RW) 
//MC1_CTL_220 : .dword 0x00100e8000120e80
////00000000001000000000111010000000 dll_ctrl_reg_0_6(RW) 00000000001000000000111010000000 dll_ctrl_reg_0_5(RW) 
//MC1_CTL_230 : .dword 0x00200e8000120e80

#150M MC1
//MC1_CTL_1f0 : .dword 0x00160e8000000000
//MC1_CTL_200 : .dword 0x00100e8000100e80
//MC1_CTL_210 : .dword 0x00100e8000160e80
////00000000001000000000111010000000 dll_ctrl_reg_0_4(RW) 00000000001000000000111010000000 dll_ctrl_reg_0_3(RW) 
//MC1_CTL_220 : .dword 0x00160e8000120e80
////00000000001000000000111010000000 dll_ctrl_reg_0_6(RW) 00000000001000000000111010000000 dll_ctrl_reg_0_5(RW) 
//MC1_CTL_230 : .dword 0x00200e8000100e80
#endif
MC1_CTL_240 : .dword 0x00000e0000000e00
//00000000000000000000111000000000 dll_ctrl_reg_1_1(RW) 00000000000000000000111000000000 dll_ctrl_reg_1_0(RW) 
MC1_CTL_250 : .dword 0x00000e0000000e00
MC1_CTL_260 : .dword 0x00000e0000000e00
MC1_CTL_270 : .dword 0x00000e0000000e00
MC1_CTL_280 : .dword 0x0000000000000e00
MC1_CTL_290 : .dword 0x0000000000000000
MC1_CTL_2a0 : .dword 0x0000000000000000
MC1_CTL_2b0 : .dword 0x0000000000000000
MC1_CTL_2c0 : .dword 0x0000000000000000
MC1_CTL_2d0 : .dword 0xf300494603c0019d
//11110100000000000011101100100111 phy_ctrl_reg_0_0(RD) 000000_00000000000000000110011101 pad_ctrl_reg_0(RW) 
MC1_CTL_2e0 : .dword 0xf3004946f3004946
MC1_CTL_2f0 : .dword 0xf3004946f3004946
MC1_CTL_300 : .dword 0xf3004946f3004946
MC1_CTL_310 : .dword 0xf3004946f3004946
//MC1_CTL_2d0 : .dword 0xf3005a470000019d
////11110100000000000011101100100111 phy_ctrl_reg_0_0(RD) 000000_00000000000000000110011101 pad_ctrl_reg_0(RW) 
//MC1_CTL_2e0 : .dword 0xf3005a47f3005a47
//MC1_CTL_2f0 : .dword 0xf3005a47f3005a47
//MC1_CTL_300 : .dword 0xf3005a47f3005a47
//MC1_CTL_310 : .dword 0xf3005a47f3005a47
MC1_CTL_320 : .dword 0x07c0000007c00000
MC1_CTL_330 : .dword 0x07c0000007c00000
MC1_CTL_340 : .dword 0x07c0000007c00000
MC1_CTL_350 : .dword 0x07c0000007c00000
MC1_CTL_360 : .dword 0x0800c00507c00000
//00000000000000001100000000000101 phy_ctrl_reg_2(RW) 00000111110000000000001100000001 phy_ctrl_reg_1_8(RD) 
MC1_CTL_370 : .dword 0x0000000000000000
MC1_CTL_380 : .dword 0x0000000000000000
MC1_CTL_390 : .dword 0x0000000000000000
MC1_CTL_3a0 : .dword 0x0000000000000000
MC1_CTL_3b0 : .dword 0x0000000000000000
MC1_CTL_3c0 : .dword 0x0000000000000000
MC1_CTL_3d0 : .dword 0x0000000000000000
MC1_CTL_3e0 : .dword 0x0000000000000000
MC1_CTL_3f0 : .dword 0x0000000000000000
MC1_CTL_400 : .dword 0x0000000000000000
MC1_CTL_410 : .dword 0x0000000000000000
MC1_CTL_420 : .dword 0x0000000000000000
MC1_CTL_430 : .dword 0x0000000000000000
MC1_CTL_440 : .dword 0x0000000000000000
MC1_CTL_450 : .dword 0x0000000000000000
MC1_CTL_460 : .dword 0x0000000000000000
MC1_CTL_470 : .dword 0x0000000000000000
MC1_CTL_480 : .dword 0x0000000000000000
MC1_CTL_490 : .dword 0x0000000000000000
MC1_CTL_4a0 : .dword 0x0000000000000000
MC1_CTL_4b0 : .dword 0x0000000000000000
MC1_CTL_4c0 : .dword 0x0000000000000000
MC1_CTL_4d0 : .dword 0x0000000000000000
MC1_CTL_4e0 : .dword 0x0000000000000000
MC1_CTL_4f0 : .dword 0x0000000000000000
MC1_CTL_500 : .dword 0x0000000000000000
MC1_CTL_510 : .dword 0x0000000000000000
MC1_CTL_520 : .dword 0x0000000000000000
MC1_CTL_530 : .dword 0x0000000000000000
MC1_CTL_540 : .dword 0x0000000000000000
MC1_CTL_550 : .dword 0x0000000000000000
MC1_CTL_560 : .dword 0x0000000000000000
MC1_CTL_570 : .dword 0x0000000000000000
MC1_CTL_580 : .dword 0x0000000000000000
MC1_CTL_590 : .dword 0x0000000000000000
MC1_CTL_5a0 : .dword 0x0000000000000000
MC1_CTL_5b0 : .dword 0x0000000000000000
MC1_CTL_5c0 : .dword 0x0000000000000000
MC1_CTL_5d0 : .dword 0x0000000000000000
MC1_CTL_5e0 : .dword 0x0000000000000000
MC1_CTL_5f0 : .dword 0x0000000000000000
MC1_CTL_600 : .dword 0x0000000000000000
MC1_CTL_610 : .dword 0x0000000000000000
MC1_CTL_620 : .dword 0x0000000000000000
MC1_CTL_630 : .dword 0x0000000000000000
MC1_CTL_640 : .dword 0x0000000000000000
MC1_CTL_650 : .dword 0x0000000000000000
MC1_CTL_660 : .dword 0x0000000000000000
MC1_CTL_670 : .dword 0x0000000000000000
MC1_CTL_680 : .dword 0x0000000000000000
MC1_CTL_690 : .dword 0x0000000000000000
MC1_CTL_6a0 : .dword 0x0000000000000000
MC1_CTL_6b0 : .dword 0x0000000000000000
MC1_CTL_6c0 : .dword 0x0000000000000000
MC1_CTL_6d0 : .dword 0x0000000000000000
MC1_CTL_6e0 : .dword 0x0000000000000000
MC1_CTL_6f0 : .dword 0x0000000000000000
MC1_CTL_700 : .dword 0x0000000000000000
MC1_CTL_710 : .dword 0x0000000000000000
MC1_CTL_720 : .dword 0x0000000000000000
MC1_CTL_730 : .dword 0x0000000000000000
MC1_CTL_740 : .dword 0x0100000000000000
//MC1_CTL_750 : .dword 0x0100000101020101
MC1_CTL_750 : .dword 0x0101000101020101
//000000_01 wrlvl_cs(RW) 000000_00 sw_leveling_mode(RW) 000000_00 rdlvl_cs(RW) 000000_01 axi2_w_priority(RW) 000000_01 axi2_r_priority(RW) 000000_10 axi2_port_ordering(RW) 000000_01 axi1_w_priority(RW) 000000_01 axi1_r_priority(RW) 
MC1_CTL_760 : .dword 0x0303030000020001
//0000_0011 axi0_priority2_relative_priority(RW) 0000_0011 axi0_priority1_relative_priority(RW) 0000_0011 axi0_priority0_relative_priority(RW) 0000_0000 address_mirroring(RW) 00000_000 tdfi_dram_clk_disable(RW) 00000_010 bstlen(RW) 000000_00 zq_req(WR) 000000_01 zq_on_sref_exit(RW) 
MC1_CTL_770 : .dword 0x0101010202020203
//0000_0001 axi2_priority2_relative_priority(RW) 0000_0001 axi2_priority1_relative_priority(RW) 0000_0001 axi2_priority0_relative_priority(RW) 0000_0010 axi1_priority3_relative_priority(RW) 0000_0010 axi1_priority2_relative_priority(RW) 0000_0010 axi1_priority1_relative_priority(RW) 0000_0010 axi1_priority0_relative_priority(RW) 0000_0011 axi0_priority3_relative_priority(RW) 
MC1_CTL_780 : .dword 0x0102020400040001
//0000_0001 tdfi_dram_clk_enable(RW) 0000_0010 tdfi_ctrl_delay(RW) 0000_0010 rdlvl_gate_dq_zero_count(RW) 0000_0100 rdlvl_dq_zero_count(RW) 0000_0000 lowpower_refresh_enable(RW) 0000_0110 dram_class(RW) 0000_1100 burst_on_fly_bit(RW) 0000_0001 axi2_priority3_relative_priority(RW) 
MC1_CTL_790 : .dword 0x281900000f000303
//00_101000 wlmrd(RW) 00_011001 wldqsen(RW) 000_00000 lowpower_control(RW) 000_00000 lowpower_auto_enable(RW) 0000_1111 zqcs_chip(RW) 0000_0000 wrr_param_value_err(RD) 0000_0011 tdfi_wrlvl_dll(RW) 0000_0011 tdfi_rdlvl_dll(RW) 
MC1_CTL_7a0 : .dword 0x00000000000000ff
MC1_CTL_7b0 : .dword 0x0000000000000000
MC1_CTL_7c0 : .dword 0x0000000000000000
MC1_CTL_7d0 : .dword 0x0000000000000000
MC1_CTL_7e0 : .dword 0x0000000000000000
//00000000 rdlvl_gate_delay_2(RD) 00000000 rdlvl_gate_delay_1(RD) 00000000 rdlvl_gate_delay_0(RD) 00000000 rdlvl_gate_clk_adjust_8(RW) 00000000 rdlvl_gate_clk_adjust_7(RW) 00000000 rdlvl_gate_clk_adjust_6(RW) 00000000 rdlvl_gate_clk_adjust_5(RW) 00000000 rdlvl_gate_clk_adjust_4(RW) 
MC1_CTL_7f0 : .dword 0xff08000000000000
//11111111 rdlvl_max_delay(RW) 00001000 rdlvl_gate_max_delay(RW) 00000000 rdlvl_gate_delay_8(RD) 00000000 rdlvl_gate_delay_7(RD) 00000000 rdlvl_gate_delay_6(RD) 00000000 rdlvl_gate_delay_5(RD) 00000000 rdlvl_gate_delay_4(RD) 00000000 rdlvl_gate_delay_3(RD) 
MC1_CTL_800 : .dword 0x0000000000000000
//00000000 rdlvl_midpoint_delay_7(RD) 00000000 rdlvl_midpoint_delay_6(RD) 00000000 rdlvl_midpoint_delay_5(RD) 00000000 rdlvl_midpoint_delay_4(RD) 00000000 rdlvl_midpoint_delay_3(RD) 00000000 rdlvl_midpoint_delay_2(RD) 00000000 rdlvl_midpoint_delay_1(RD) 00000000 rdlvl_midpoint_delay_0(RD) 
MC1_CTL_810 : .dword 0x000000000000000e
//00000000 rdlvl_offset_delay_6(RW) 00000000 rdlvl_offset_delay_5(RW) 00000000 rdlvl_offset_delay_4(RW) 00000000 rdlvl_offset_delay_3(RW) 00000000 rdlvl_offset_delay_2(RW) 00000000 rdlvl_offset_delay_1(RW) 00000000 rdlvl_offset_delay_0(RW) 00000000 rdlvl_midpoint_delay_8(RD) 
MC1_CTL_820 : .dword 0x0420000c20400000
//00000100 tdfi_wrlvl_resplat(RW) 00000000 tdfi_wrlvl_resp(RD) 00000000 tdfi_rdlvl_rr(RW) 00001100 tdfi_rdlvl_resplat(RW) 00000000 tdfi_rdlvl_resp(RD) 01000000 refresh_per_zq(RW) 00000000 rdlvl_offset_delay_8(RW) 00000000 rdlvl_offset_delay_7(RW) 
//MC1_CTL_830 : .dword 0x0000000000000c0a
MC1_CTL_830 : .dword 0x1313131313130c0a
//00000000 wrlvl_delay_5(RD) 00000000 wrlvl_delay_4(RD) 00000000 wrlvl_delay_3(RD) 00000000 wrlvl_delay_2(RD) 00000000 wrlvl_delay_1(RD) 00000000 wrlvl_delay_0(RD) 00000010 tmod(RW) 00001010 tdfi_wrlvl_ww(RW) 
MC1_CTL_840 : .dword 0x0000640064001313
//00000000000000_0001100100 axi1_priority_relax(RW) 000000_0001100100 axi0_priority_relax(RW) 00000000 wrlvl_delay_8(RD) 00000000 wrlvl_delay_7(RD) 00000000 wrlvl_delay_6(RD) 
MC1_CTL_850 : .dword 0x0000000000000064
MC1_CTL_860 : .dword 0x0200004000000000
MC1_CTL_870 : .dword 0x0000000000000044
//0_000000000000010 emrs1_data_3(RW) 0_000000000000010 emrs1_data_2(RW) 0_000000000000010 emrs1_data_1(RW) 0_000000000000010 emrs1_data_0(RW) 
MC1_CTL_880 : .dword 0x0000000000000000
//MC1_CTL_890 : .dword 0x0a520a520a520a52
MC1_CTL_890 : .dword 0x0a520a520a520a5a
//0_000010000010000 mrs_data_3(RW) 0_000010000010000 mrs_data_2(RW) 0_000010000010000 mrs_data_1(RW) 0_000010000010000 mrs_data_0(RW) 
MC1_CTL_8a0 : .dword 0x00000000001c001c
MC1_CTL_8b0 : .dword 0x0000000000000000
MC1_CTL_8c0 : .dword 0x0004000000000000
MC1_CTL_8d0 : .dword 0x00000000c8000000
MC1_CTL_8e0 : .dword 0x0000000000000050
//MC1_CTL_8f0 : .dword 0x000000000a150080 //clk skew cleared
//MC1_CTL_8f0 : .dword 0x000000000a100080 //clk skew cleared
MC1_CTL_8f0 : .dword 0x000000002b352180
//MC1_CTL_8f0 : .dword 0x000000001b251180
//MC1_CTL_8f0 : .dword 0x0000000040404080
//0000000000000000000000000111100_000000000000000000000000001111000 dll_ctrl_reg_2(RW) 
MC1_CTL_900 : .dword 0x0000000000000000
MC1_CTL_910 : .dword 0x0000000000000000
MC1_CTL_920 : .dword 0x0000000000000000
MC1_CTL_930 : .dword 0x0000000000000000
MC1_CTL_940 : .dword 0x0300000000050500
MC1_CTL_950 : .dword 0x0000000000000a03
MC1_CTL_960 : .dword 0x0604000100000000
//000_00101 rdlat_adj(RW) 0000_0100 wrlat_adj(RW) 0000000_0 swlvl_start(WR) 0000000_0 swlvl_load(WR) 0000000_0 swlvl_exit(WR) 000000_000000000000000000 int_status(RD) 
MC1_CTL_970 : .dword 0x000000000003e805
MC1_CTL_start_DATA_LO: .word  0x00000000
//0000000_1 rw_same_en 0000000_0 reg_dimm_enable 0000000_0 reduc 0000000_0 pwrup_srefresh_exit
MC1_CTL_start_DATA_HI: .word  0x01010100
//0000000_1 swap_port_rw_same_en 0000000_1 swap_en 0000000_0 start 0000000_0 srefresh
