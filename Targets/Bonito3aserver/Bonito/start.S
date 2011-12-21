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
#undef USE_GPIO_SERIAL

#ifdef DDR3_DIMM
#define USE_SB_I2C
#else
#define USE_GPIO_I2C
#define MULTI_I2C_BUS
#endif

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
/* set GPIO as output
 * x : 0x1<<offset
 */
#define GPIO_SET_OUTPUT(x) \
li		v0,		0xbfe0011c; \
lw		v1,		0(v0); \
or		v1,		x&0xffff; \
xor		v1,		0x0; \
sw		v1,		0(v0); \
lw		v1,		4(v0); \
or		v1,		x&0xffff; \
xor		v1,		x; \
sw		v1,		4(v0); \
nop; \
nop;

/* clear GPIO as output
 * x : 0x1 <<offsest
 */
#define GPIO_CLEAR_OUTPUT(x) \
li		v0,		0xbfe0011c; \
lw		v1,		0(v0); \
or		v1,		x&0xffff; \
xor		v1,		x; \
sw		v1,		0(v0); \
lw		v1,		4(v0); \
or		v1,		x&0xffff; \
xor		v1,		x; \
sw		v1,		4(v0); \
nop; \
nop;

#ifdef DDR3_DIMM
/* WatchDog Close for chip MAX6369*/
#define WatchDog_Close \
GPIO_CLEAR_OUTPUT(0x1<<13); \
GPIO_CLEAR_OUTPUT(0x1<<14); \
GPIO_CLEAR_OUTPUT(0x1<<6 | 0x1<<5|0x1<<4); \
GPIO_SET_OUTPUT(0x1<<3);

#if 0
li v1,0x1000;\
78:; \
subu v1,1; \
bnez v1,78b; \
nop; \
GPIO_CLEAR_OUTPUT(0x1<<14);
#endif 

/* WatchDog Enable for chip MAX6369*/
#define WatchDog_Enable \
GPIO_SET_OUTPUT(0x1<<4 | 0x1<<5 | 0x1 << 6 | 0x1<<13); \
GPIO_CLEAR_OUTPUT(0x1<<13); \
li v1,0x100;\
78:; \
subu v1,1; \
bnez v1,78b; \
nop; \
GPIO_SET_OUTPUT(0x1<<14); \
li v1,0x1000;\
78:; \
subu v1,1; \
bnez v1,78b; \
nop; \
GPIO_CLEAR_OUTPUT(0x1<<14);

#else

  /* WatchDog Close for chip MAX6369*/
  #define WatchDog_Close \
  GPIO_CLEAR_OUTPUT(0x1<<13); \
  GPIO_SET_OUTPUT(0x1<<14); \
  GPIO_CLEAR_OUTPUT(0x1<<5); \
  GPIO_SET_OUTPUT(0x1<<3|0x1<<4); \
  li v1,0x1000;\
  78:; \
  subu v1,1; \
  bnez v1,78b; \
  nop; \
  GPIO_CLEAR_OUTPUT(0x1<<14);

  /* WatchDog Enable for chip MAX6369*/
  #define WatchDog_Enable \
  GPIO_SET_OUTPUT(0x1<<14); \
  GPIO_SET_OUTPUT(0x1<<5); \
  GPIO_CLEAR_OUTPUT(0x1<<4); \
  GPIO_SET_OUTPUT(0x1<<3); \
  li v1,0x100;\
  78:; \
  subu v1,1; \
  bnez v1,78b; \
  nop; \
  GPIO_CLEAR_OUTPUT(0x1<<14); \
  li v1,0x1000;\
  78:; \
  subu v1,1; \
  bnez v1,78b; \
  nop; \
  GPIO_SET_OUTPUT(0x1<<13); 

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

#ifdef DDR3_DIMM
    GPIO_CLEAR_OUTPUT(0x1<<2);  
#else
    GPIOLED_SET(0xd)
#endif

    WatchDog_Close

    bal	locate			/* Get current execute address */
    nop

uncached:
    or	ra, UNCACHED_MEMORY_ADDR
    j	ra
    nop

/*
 *  Reboot vector usable from outside pmon.
 */
    .align	8   /* bfc00200 */
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

#if 1
    b	exc_common
    nop

    /* Debug exception */
    .align	7 /* bfc00480 */
#include "exc_ejtag.S"
#endif


    
exc_common:
    .set     mips64
    mfc0    t0, $15, 1
    .set     mips3
    PRINTSTR("\r\nNODE ID=")
    move    a0, t0
    bal    hexserial
    nop

    PRINTSTR("\r\nCAUSE=")
    mfc0    a0, COP_0_CAUSE_REG
    bal    hexserial
    nop
    PRINTSTR("\r\nSTATUS=")
    mfc0    a0, COP_0_STATUS_REG
    bal    hexserial
    nop
    PRINTSTR("\r\nERRORPC=")
    mfc0    a0, COP_0_ERROR_PC
    bal    hexserial
    nop
    PRINTSTR("\r\nEPC=")
    mfc0    a0, COP_0_EXC_PC
    bal    hexserial
    nop
1:
    b 1b
    nop
#ifndef ROM_EXCEPTION
    PRINTSTR("\r\nDERR0=")
    cfc0    a0, COP_0_DERR_0
    bal    hexserial
    nop
    PRINTSTR("\r\nDERR1=")
    cfc0    a0, COP_0_DERR_1
    bal    hexserial
    nop
#else
1:
    b 1b
    nop
#endif

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

//Open 64-bit address space
    mfc0    t0, CP0_STATUS
    li      t1, 0x00e0      # {cu3,cu2,cu1,cu0}<={0110, status_fr<=1
    or      t0, t0, t1
    mtc0    t0, CP0_STATUS
 
    mfc0    t0, CP0_STATUS
    lui     t1, 0x40        #bev
    or      t0, t0, t1
    mtc0    t0, CP0_STATUS
    mtc0    zero, CP0_CAUSE

//#defien NODE1_BOOT
#define NODE0_CORE0_BUF0 0x900000003ff01000
#define NODE1_CORE0_BUF0 0x900010003ff01000
#define NODE2_CORE0_BUF0 0x900020003ff01000
#define NODE3_CORE0_BUF0 0x900030003ff01000

#define FN_OFF 0x020
#define SP_OFF 0x028
#define GP_OFF 0x030
#define A1_OFF 0x038

#define MAIN_CORE_PRE_TLB_INIT_OK   0x1111
#define NODE_MEM_INIT_DONE 0x6666
#define ALL_CORE0_INIT_DONE 0x8888
#define NODE_SCACHE_ENABLED   0x2222
#define SYSTEM_INIT_OK  0x5a5a

/**************************************************/
/* Clear Mail BOX of every core                   */
/**************************************************/
    .set    mips64
    mfc0    t0, $15, 1
    .set    mips3
    andi    t0, 0x3ff

    andi    t3, t0,0xc

    beq     t3, 0x0, node0;
    nop     

    beq     t3, 0x4, node1;
    nop     

    beq     t3, 0x8, node2;
    nop     

    beq     t3, 0xc, node3;
    nop     

node0:
    dli     t1, NODE0_CORE0_BUF0;
    b       common;
    nop

node1:
    dli     t1, NODE1_CORE0_BUF0;
    b       common;
    nop

node2:
    dli     t1, NODE2_CORE0_BUF0;
    b       common;
    nop

node3:
    dli     t1, NODE3_CORE0_BUF0;
    b       common;
    nop

common:
    andi    t3, t0, 0x3  #local cpuid
    dsll    t3, 8
    or      t1, t1, t3

    sd      $0, FN_OFF(t1)
    sd      $0, SP_OFF(t1)
    sd      $0, GP_OFF(t1)
    sd      $0, A1_OFF(t1)

#ifdef NODE1_BOOT
    dli     a0, 0x4 
#else
    dli     a0, 0x0;
#endif
    bne     t0, a0, slave_main
    nop

//#define DEBUG_LS3
#ifdef DEBUG_LS3
    dli     a0, 0xfff
    and     t1, t1, a0
    dsll    t1, t1, 10
1:
    bnez    t1, 1b
    daddi   t1, t1, -1

    bal	initserial
    nop

    PRINTSTR("CP0_STATUS:")
    mfc0    a0, CP0_STATUS
    bal	    hexserial
    nop
    PRINTSTR("\r\n\r\n")


    sync
    sync
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    PRINTSTR("sync ok\r\n")
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
1:
    bnez    t0, 1b
    nop
#endif

#define SHUTDOWN_CORE
#ifdef  SHUTDOWN_CORE
    li	  a0, 0xbfe00184
    li	  t1, 0x0001ffff
    sw	  t1, 0x0(a0)
    li	  a0, 0x900010001fe00184
    li	  t1, 0x0000ffff
    sw	  t1, 0x0(a0)
#endif



#if 0 /* Niuxie Tube */
    li      a0,0xbff00080
    li      t0,0xcc
    sb      t0,0x0(a0)
#endif

#ifdef DEBUG_LS3
#else
    bal	initserial
    nop
    //bal	initserial_uart
    //nop
#endif

#ifdef DEBUG_LS3
    dli     t2, 0x9800000000000000
    cache   21, 0x0(t2)
    PRINTSTR("Hit WB Invalidate_D 0 \r\n")
    dli     t2, 0x9800000000000020
    cache   21, 0x0(t2)
    PRINTSTR("Hit WB Invalidate_D 2 \r\n")
    dli     t2, 0x9800000000000040
    cache   21, 0x0(t2)
    PRINTSTR("Hit WB Invalidate_D 4 \r\n")
    dli     t2, 0x9800000000000060
    cache   21, 0x0(t2)
    PRINTSTR("Hit WB Invalidate_D 6 \r\n")

    dli     t2, 0x9800000000000000
    ld      a0, 0x0(t2)
    PRINTSTR("Read 0 \r\n")
    dli     t2, 0x9800000000000020
    ld      a0, 0x0(t2)
    PRINTSTR("Read 2 \r\n")
    dli     t2, 0x9800000000000040
    ld      a0, 0x0(t2)
    PRINTSTR("Read 4 \r\n")
    dli     t2, 0x9800000000000060
    ld      a0, 0x0(t2)
    PRINTSTR("Read 6 \r\n")

    dli     t1, 0x9800000000000000
    dli     t2, 0x9800000000400000
1:
    ld      a0, 0x0(t1)
    daddiu  t1, t1, 0x20
    bne     t1, t2, 1b
    nop
    PRINTSTR("Scache scan done \r\n")

1:
    b       1b
    nop

#endif


#if 0
    PRINTSTR("Shut down other cores\r\n")
    li      a0, 0xbfe00180
    lw      a1, 0x0(a0)
    li      t1, 0xffff1fff
    and     a1, a1, t1
    sw      a1, 0x0(a0)

#endif

    li      a0,0x900000001ff00080
    li      t0,0x99
    sb      t0,0x0(a0)

#ifdef LS3_HT
    b       core0_start
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
#ifdef DDR3_DIMM
#else
     GPIOLED_SET(4)
#endif

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
1:	/////bnez	t0,1b
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
    PRINTSTR("\r\n\r\nPMON2000 MIPS Initializing. Standby...\r\n")
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

#ifdef MULTI_CHIP
    WatchDog_Enable;
    TTYDBG("CPU 1 READ TEST...\r\n")
    dli      t0, 0x900010003ff00040
    lw       a0, 0x0(t0)
    TTYDBG("CPU 1 READ TEST OK.\r\n")
    WatchDog_Close;
#endif
##########################################

    WatchDog_Enable;
#include "loongson3_fixup.S"
    WatchDog_Close;

##########################################
//let other cores going
#ifdef NODE1_BOOT
    dli     t0,NODE1_CORE0_BUF0  #buf of cpu0
#else
    dli     t0,NODE0_CORE0_BUF0  #buf of cpu0
#endif
    li      t1, MAIN_CORE_PRE_TLB_INIT_OK
    sw      t1, FN_OFF(t0)
    sync
    

    TTYDBG("Init tlb...\r\n")
    bal     CPU_TLBClear
    nop
    bal     tlb_init
    nop

#if 1 /* TLB init */
    TTYDBG("Init htpcitlb...\r\n")
#include "pcitlb.S" /* map 0x1000000-0x1700000 to 0x4000000 */
#endif
/*
 *  Reset and initialize caches to a known state.
 */

#if 1
    TTYDBG("godson2 caches found\r\n")
    bal     godson2_cache_init
    nop
    //TTYDBG("scache init\r\n")
    .set     mips64
    mfc0    t0, $15, 1
    .set     mips3
    andi    t0, 0xc
    dsll    a1, t0, 42
    dli     a0, 0x9800000000000000
    or      a0, a0, a1
    bal     scache_init_64
    nop
#endif

#if 1
    TTYDBG("Jump to 9fc\r\n")
    lui     t0, 0xdfff
    ori     t0, t0, 0xffff
    bal     1f
    nop
1:
    and     ra, ra, t0
    addiu   ra, ra, 16
    jr      ra
    nop

    sync
    nop
    nop
    nop
    nop
    ## enable kseg0 cachablilty####
    mfc0   $4, $16
    and    $4,0xfffffff8
    or     $4,0x3
    mtc0   $4,$16

    TTYDBG("scache init done\r\n")

    .set    mips64
    mfc0    t1, $15, 1
    .set    mips3
    andi    a1, t1, 0xc
    dsll    a1, a1, 42
    dli     t0, NODE0_CORE0_BUF0
    or      t0, t0, a1
#ifdef LS3B
    andi    a1, t1, 0xc
    dsll    a1, a1, 12
    daddu   t0, t0, a1
#endif
    li      a0, NODE_SCACHE_ENABLED
    sw      a0, SP_OFF(t0)
    sync
#endif

##########################################
//cxk
#define MULTI_NODE_INIT_MEM

#include "ddr_param_define.h"
#include "ddr_config_define.h"
#define DISABLE_DIMM_ECC
#define PRINT_MSG
#define FIX_DDR_PARAM
//#define DEBUG_DDR
//#define DEBUG_DDR_PARAM

    TTYDBG("\r\nStart Init Memory, wait a while......\r\n")
    GPIO_SET_OUTPUT(0x1<<8)
#ifdef MULTI_NODE_INIT_MEM
####################################
    b       MEM_INIT_BEGIN
    nop

MEM_INIT_DONE:
####################################
#else
####################################
    //wait the NODE scache enabled
    dli     t0, NODE0_CORE0_BUF0
    li      a1, NODE_SCACHE_ENABLED
1:
    lw      a0, SP_OFF(t0)
    bne     a0, a1, 1b
    nop

    dli     msize, 0

//!!!!important--s1 must be correctly set

    TTYDBG("NODE 0 MEMORY CONFIG BEGIN\r\n")

#ifdef DDR3_DIMM
#ifdef  AUTO_DDR_CONFIG
    dli     s1, 0x32100000
#else
    dli     s1, 0xc1c30400c1c30404  // use MC0
#endif
#else
#ifdef  AUTO_DDR_CONFIG
    dli     s1, 0x32100000
#else
    dli     s1, 0xb0c10400b0c10404
#endif
#endif

#include "loongson3_ddr2_config.S"

#if 0
    PRINTSTR("Skip other core:")
    dli     t6, 0x00
    bal     inputaddress    #input value stored in v0
    nop
    bnez    v0, SYS_MEM_INIT_END
    nop
#endif

#ifdef MULTI_CHIP
    //wait the NODE scache enabled
    dli     t0, NODE1_CORE0_BUF0
1:
    li      a1, NODE_SCACHE_ENABLED
    lw      a0, SP_OFF(t0)
    bne     a0, a1, 1b
    nop

    TTYDBG("NODE 1 MEMORY CONFIG BEGIN\r\n")
#ifdef DDR3_DIMM
#ifdef  AUTO_DDR_CONFIG
    dli     s1, 0x76540001
#else
    dli     s1, 0xc1c30400c1c30405  // use MC0
#endif
#else
#ifdef  AUTO_DDR_CONFIG
    dli     s1, 0x32100011
#else
    dli     s1, 0xb0c10400b0c10405
#endif
#endif
#include "loongson3_ddr2_config.S"
#endif

#endif
    GPIO_SET_OUTPUT(0x1<<9)
    TTYDBG("Init Memory done.\r\n")

#ifdef ARB_LEVEL
    b       ARB_level_over
    nop
#include "ARB_level.S"
ARB_level_over:
#endif
##########################################

#if 1 //cww_X2
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
#endif
##########################################
#ifdef  DEBUG_DDR
    b       TM_over
    nop
#include "Test_Mem.S"
TM_over:

#if 1
    PRINTSTR("\r\nDo test?(0xf: skip): ")
    bal     inputaddress
    nop
    and     v0, v0, 0xf
    dli     a1, 0x1
    bgt     v0, a1, 3f
    nop
#endif

#if 0
#if 0
    PRINTSTR("\r\nStart other core test?(0xcccc: start): ")
    bal     inputaddress
    nop
    move    t1, v0
#else
    li      t1, 0xcccc
#endif

#ifdef NODE1_BOOT
    dli     t0, NODE1_CORE0_BUF0 #buf of cpu1
#else
    dli     t0, NODE0_CORE0_BUF0 #buf of cpu0
#endif
    sw      t1, FN_OFF(t0)
    nop
#endif

#if 0
    PRINTSTR("\r\nStart simple_test_mem......\r\n")
1:
    dli     t1, 0x0010
    dli     s1, 0x0004000080000000  //NODE 0, start from 0x80000000
    dli     t0, 0xaaaaaaaaaaaaaaaa
    bal     simple_test_mem
    nop
    b       1b
    nop
#endif

    dli     s1, 0x0010000080000000  //NODE 0, start from 0x80000000
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

3:
#endif
#########################################

#ifdef LS3_HT
WatchDog_Enable;
#include "loongson3_HT_init.S"
WatchDog_Close;
#endif

    PRINTSTR("\r\n======This is cww's world:1\r\n")
#include "3aserver_bridge_config.S"
    PRINTSTR("\r\n======This is cww's world:2\r\n")
##########################################

//#include "loongson3_ddr_debug.S"

##########################################

#if 1 //cww_X2
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
#endif

##########################################

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
    TTYDBG("\r\n")
#endif
    la	a0, start
    li	a1, 0xbfc00000
    la	a2, _edata
    //or      a0, 0xa0000000
    //or      a2, 0xa0000000
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
    //or      a0, 0xa0000000
    //or      a2, 0xa0000000

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

//	srl	msize,20
    move	a0,msize
    
#ifdef NODE1_BOOT
    dli     t0,NODE1_CORE0_BUF0  #buf of cpu0
#else
    dli     t0,NODE0_CORE0_BUF0  #buf of cpu0
#endif
    li      t1, SYSTEM_INIT_OK
    sw      t1, FN_OFF(t0)
    nop

#if 1
1:
    lw      t1, FN_OFF(t0)
    li      t2, SYSTEM_INIT_OK
    bne     t2, t1, 1b
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
//#define USE_LPC_UART # defined in pmon_cfg
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


LEAF(initserial_uart)
    li  a0, GS3_UART_BASE

 	li	t1,128
 	sb	t1,3(a0)
#ifdef DDR3_DIMM
    li	t1,0x12      # divider, highest possible baud rate
#else
    li	t1,0x0e     # divider, highest possible baud rate
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
END(initserial_uart)

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
 	sb	t1,3(a0)
#ifdef DDR3_DIMM
    li	t1,0x12      # divider, highest possible baud rate
#else
    li	t1,0x0e     # divider, highest possible baud rate
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
    la	v0, COM1_BASE_ADDR
    #la	v0, 0xba0002f8
1:
    lbu	v1, NSREG(NS16550_LSR)(v0)
    and	v1, LSR_TXRDY
#	li	v1, 1
    beqz	v1, 1b
    nop

    sb	a0, NSREG(NS16550_DATA)(v0)
    move	v1, v0
    la	v0, COM1_BASE_ADDR
    #la	v0, 0xba0002f8
    bne	v0, v1, 1b
    nop

    j	ra
    nop	
END(tgt_putchar_COM1)


#include "i2c.S"
#ifdef  AUTO_DDR_CONFIG
#include "detect_node_dimm.S"
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
LEAF(scache_init_64)
    move	t7, ra

    #lui     a0, 0x8000
    #lui     a2, 0x0010      #4M/4way
    dli     a2, 0x00100000   #4M/4way
    #lui     a2, 0x0002      #512k/4way
    #lui     a2, 0x0004      #1M/4way
scache_init_4way_64:
#a0=0x80000000, a2=scache_size
#a3, v0 and v1 used as local registers
    li      t0, 0x22
    mtc0    t0, CP0_ECC
    mtc0    $0, CP0_TAGHI
    mtc0    $0, CP0_TAGLO
    daddu    v0, $0, a0
    daddu    v1, a0, a2
1:      //dslt     a3, v0, v1
    //beq     a3, $0, 1f
    beq     v0, v1, 1f
    nop
    cache   Index_Store_Tag_S, 0x0(v0)
    cache   Index_Store_Tag_S, 0x1(v0)
    cache   Index_Store_Tag_S, 0x2(v0)
    cache   Index_Store_Tag_S, 0x3(v0)
    beq     $0, $0, 1b
    daddiu   v0, v0, 0x20
1:

    jr      ra
    nop

1:      b       1b
    nop
END(scache_init_64)
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

//lycheng
LEAF(nbmisc_read_index_mips)
    dli   t1, HT_CONFIG_ADDR
    or    t1, t1, a0
    sw    a1, NBMISC_INDEX(t1)
    lw    v0, 0x64(t1)
    j  ra
    nop
END(nbmisc_read_index_mips)
LEAF(nbmisc_write_index_mips)
    dli   t1, HT_CONFIG_ADDR
    or    t1, t1, a0
    or    t2, a1, 0x80
    sw    t2, 0x60(t1)
    sw    a2, 0x64(t1)
    j  ra
    nop
END(nbmisc_write_index_mips)
LEAF(post_code_mips)
    li t0, CPU_POST_PORT
    sb a0, 0x0(t0)
    j ra
    nop
END(post_code_mips)
LEAF(enable_rs780_dev8)
    move t6, ra
    li   a0,  0x0
    li   a1,  0x0
    bal    nbmisc_read_index_mips
    nop
    move   v1, v0
    li     t0,  0xffffffbf      // ~(1 << 6)
    and   t1, v1, t0
    li     t0,  0x40  // (1 << 6)
    or    v1, t1, t0
    beq    v1, v0, 1f
    nop
    //or  a1, a1, 0x80
    move a2,  v1
    bal   nbmisc_write_index_mips
    nop
1:
    j  t6
    nop
END(enable_rs780_dev8)
LEAF(pci_read_config32_mips)
    dli t1, HT_CONFIG_ADDR
    or  t2, t1, a0
    or  t1, t2, a1
    lw  v0, 0x0(t1)
    j  ra
    nop
END(pci_read_config32_mips)
LEAF(pci_write_config32_mips)
    dli t1, HT_CONFIG_ADDR
    or  t2, t1, a0
    or  t1, t2, a1
    sw  a2, 0x0(t1)
    j  ra
    nop
END(pci_write_config32_mips)
LEAF(pci_read_config8_mips)
    dli t1, HT_CONFIG_ADDR
    or  t2, t1, a0
    or  t1, t2, a1
    lb  v0, 0x0(t1)
    j  ra
    nop
END(pci_read_config8_mips)
LEAF(pci_write_config8_mips)
    dli t1, HT_CONFIG_ADDR
    or  t2, t1, a0
    or  t1, t2, a1
    sb  a2, 0x0(t1)
    j  ra
    nop
END(pci_write_config8_mips)

.global watchdog_enable
.ent    watchdog_enable
.set    noreorder
.set    mips3
watchdog_enable:
    WatchDog_Enable
    jr	ra
.end watchdog_enable

#if  1 /* slave main */

    .ent    slave_main
slave_main:

#if 1       //cxk, has been done already
//Open 64-bit address space
    mfc0    t0, CP0_STATUS
    li      t1, 0x00e0      # {cu3,cu2,cu1,cu0}<={0110, status_fr<=1
    or      t0, t0, t1
    mtc0    t0, CP0_STATUS

    mfc0    t0, CP0_STATUS
    lui     t1, 0x40        #bev
    or      t0, t0, t1
    mtc0    t0, CP0_STATUS
    mtc0    zero, CP0_CAUSE
#endif

//cxk, not core 0, jump to slave_core
    .set     mips64
    mfc0    t0, $15, 1
    .set     mips3
    andi    t0, 0x3ff
    andi    a1, t0, 0x3
    bnez    a1, slave_core
    nop

//core0 of each NODE
#ifdef NODE1_BOOT
    dli     t0, NODE1_CORE0_BUF0 #buf of cpu1
#else
    dli     t0, NODE0_CORE0_BUF0 #buf of cpu0
#endif
    li      a1, MAIN_CORE_PRE_TLB_INIT_OK
1:
    lw      a0, FN_OFF(t0)
    bne     a1, a0, 1b
    nop

//cxk
//serial init ok
    bal     CPU_TLBClear
    nop
    bal     tlb_init
    nop
    bal     godson2_cache_init
    nop

    //TTYDBG("scache init\r\n")
    .set     mips64
    mfc0    t0, $15, 1
    .set     mips3
    andi    t0, 0xc
    dsll    a1, t0, 42
    dli     a0, 0x9800000000000000
    or      a0, a0, a1
    bal     scache_init_64 // whd
    nop

#if 1
    //PRINTSTR("Jump to 9fc\r\n")
    lui     t0, 0xdfff
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
    //TTYDBG("cache enable done\r\n")
#endif
    .set     mips64
    mfc0    t1, $15, 1
    .set     mips3
    andi    a1, t1, 0xc
    dsll    a1, a1, 42
    dli     t0, NODE0_CORE0_BUF0
    or      t0, t0, a1
#ifdef LS3B
    andi    a1, t1, 0xc
    dsll    a1, a1, 12
    daddu   t0, t0, a1
#endif
    li      a0, NODE_SCACHE_ENABLED
    sw      a0, SP_OFF(t0)
    sync
    //PRINTSTR("Slave node scache init done.\r\n")

#ifdef MULTI_NODE_INIT_MEM
//cxk
#define  SEQ_INIT_MEM   //Concurrent init not work now.
#ifndef  SEQ_INIT_MEM
#define  PRINTSTR(x)
#endif
MEM_INIT_BEGIN:
//init mem on order(NODE 0>1>2>3)
    .set     mips64
    mfc0    t0, $15, 1
    .set     mips3
    andi    t0, 0xc
    dsrl    t0, t0, 2
    dli     a0, 0x0
    beq     t0, a0, NODE0_INIT_MEM
    nop
    daddu   a0, a0, 0x1
    beq     t0, a0, NODE1_INIT_MEM
    nop
    daddu   a0, a0, 0x1
    beq     t0, a0, NODE2_INIT_MEM
    nop
    daddu   a0, a0, 0x1
    beq     t0, a0, NODE3_INIT_MEM
    nop
    b       ALL_NODE_MEM_INIT_DONE
    nop

NODE0_INIT_MEM:
    //wait the NODE scache enabled
    dli     t0, NODE0_CORE0_BUF0
    li      a1, NODE_SCACHE_ENABLED
1:
    lw      a0, SP_OFF(t0)
    bne     a0, a1, 1b
    nop

    PRINTSTR("NODE 0 MEMORY CONFIG BEGIN\r\n")
    dli     msize, 0
#ifdef DDR3_DIMM
#ifdef  AUTO_DDR_CONFIG
    dli     s1, 0x32100000
#else
    dli     s1, 0xc1c30400c1c30404  // use MC0
#endif
#else
#ifdef  AUTO_DDR_CONFIG
    dli     s1, 0x32100000
#else
    dli     s1, 0xb0c10400b0c10404
#endif
#endif
#include "loongson3_ddr2_config.S"

    dli     t0, NODE0_CORE0_BUF0
    sw      msize, SP_OFF(t0)
#ifdef  SEQ_INIT_MEM
    li      a1, NODE_MEM_INIT_DONE
    sw      a1, FN_OFF(t0)
#endif
    sync
    b       ALL_NODE_MEM_INIT_DONE
    nop


NODE1_INIT_MEM:
    //wait the NODE scache enabled
    dli     t0, NODE1_CORE0_BUF0
1:
    li      a1, NODE_SCACHE_ENABLED
    lw      a0, SP_OFF(t0)
    bne     a0, a1, 1b
    nop

#ifdef  SEQ_INIT_MEM
    dli     t0, NODE0_CORE0_BUF0
    li      a1, NODE_MEM_INIT_DONE
1:
#if 1
    dli     a2, 0x1000
2:
    daddiu  a2, a2, -0x1
    bnez    a2, 2b
    nop
#endif
    lw      a0, FN_OFF(t0)
    bne     a0, a1, 1b
    nop
#endif

    PRINTSTR("NODE 1 MEMORY CONFIG BEGIN\r\n")
    dli     msize, 0
#ifdef DDR3_DIMM
#ifdef  AUTO_DDR_CONFIG
    dli     s1, 0x76540001
#else
    dli     s1, 0xc1c30400c1c30405  // use MC0
#endif
#else
#ifdef  AUTO_DDR_CONFIG
    dli     s1, 0x32100011
#else
    dli     s1, 0xb0c10400b0c10405
#endif
#endif
#include "loongson3_ddr2_config.S"

    dli     t0, NODE1_CORE0_BUF0
    sw      msize, SP_OFF(t0)
#ifdef  SEQ_INIT_MEM
    li      a1, NODE_MEM_INIT_DONE
    sw      a1, FN_OFF(t0)
#endif
    sync
    b       ALL_NODE_MEM_INIT_DONE
    nop

NODE2_INIT_MEM:
    //wait the NODE scache enabled
    dli     t0, NODE2_CORE0_BUF0
1:
    li      a1, NODE_SCACHE_ENABLED
    lw      a0, SP_OFF(t0)
    bne     a0, a1, 1b
    nop

#ifdef  SEQ_INIT_MEM
    dli     t0, NODE1_CORE0_BUF0
    li      a1, NODE_MEM_INIT_DONE
1:
#if 1
    dli     a2, 0x1000
2:
    daddiu  a2, a2, -0x1
    bnez    a2, 2b
    nop
#endif
    lw      a0, FN_OFF(t0)
    bne     a0, a1, 1b
    nop
#endif

    PRINTSTR("NODE 2 MEMORY CONFIG BEGIN\r\n")
    dli     msize, 0
#ifdef  AUTO_DDR_CONFIG
    dli     s1, 0xff540002
#else
    dli     s1, 0xc1c30402
#endif
#include "loongson3_ddr2_config.S"

    dli     t0, NODE2_CORE0_BUF0
    sw      msize, SP_OFF(t0)
#ifdef  SEQ_INIT_MEM
    li      a1, NODE_MEM_INIT_DONE
    sw      a1, FN_OFF(t0)
#endif
    sync
    b       ALL_NODE_MEM_INIT_DONE
    nop


NODE3_INIT_MEM:
    //wait the NODE scache enabled
    dli     t0, NODE3_CORE0_BUF0
1:
    li      a1, NODE_SCACHE_ENABLED
    lw      a0, SP_OFF(t0)
    bne     a0, a1, 1b
    nop

#ifdef  SEQ_INIT_MEM
    dli     t0, NODE2_CORE0_BUF0
    li      a1, NODE_MEM_INIT_DONE
1:
#if 1
    dli     a2, 0x1000
2:
    daddiu  a2, a2, -0x1
    bnez    a2, 2b
    nop
#endif
    lw      a0, FN_OFF(t0)
    bne     a0, a1, 1b
    nop
#endif

    PRINTSTR("NODE 3 MEMORY CONFIG BEGIN\r\n")
    dli     msize, 0
#ifdef  AUTO_DDR_CONFIG
    dli     s1, 0xff760003
#else
    dli     s1, 0xc1c30403
#endif
#include "loongson3_ddr2_config.S"

    dli     t0, NODE3_CORE0_BUF0
    sw      msize, SP_OFF(t0)
#ifdef  SEQ_INIT_MEM
    li      a1, NODE_MEM_INIT_DONE
    sw      a1, FN_OFF(t0)
#endif
    sync
    b       ALL_NODE_MEM_INIT_DONE
    nop

ALL_NODE_MEM_INIT_DONE:
    .set     mips64
    mfc0    t0, $15, 1
    .set     mips3
    andi    t0, 0xc
#ifdef NODE1_BOOT
    dli     a0, 4
#else
    dli     a0, 0
#endif
    beq     t0, a0, 2f
    nop
    //non boot core, wait all NODE mem init done, then clear its mailbox, and wait to jump to kernel
#ifdef NODE1_BOOT
    dli     t0, NODE1_CORE0_BUF0 #buf of cpu1
#else
    dli     t0, NODE0_CORE0_BUF0 #buf of cpu0
#endif
    li      a1, ALL_CORE0_INIT_DONE
1:
    lw      a0, FN_OFF(t0)
    bne     a1, a0, 1b
    nop

    //clear self mailbox
    .set     mips64
    mfc0    t0, $15, 1
    .set     mips3
    andi    t0, t0, 0xc
    beq     t0, 0x0, 10f
    nop     
    beq     t0, 0x4, 11f;
    nop     
    beq     t0, 0x8, 12f;
    nop     
    beq     t0, 0xc, 13f;
    nop     
    b       wait_to_jump_kernel
    nop
10:
    dli     t1, NODE0_CORE0_BUF0;
    sw      $0, FN_OFF(t1)
    sw      $0, SP_OFF(t1)
    sync
    b       wait_to_jump_kernel
    nop
11:
    dli     t1, NODE1_CORE0_BUF0;
    sw      $0, FN_OFF(t1)
    sw      $0, SP_OFF(t1)
    sync
    b       wait_to_jump_kernel
    nop
12:
    dli     t1, NODE2_CORE0_BUF0;
    sw      $0, FN_OFF(t1)
    sw      $0, SP_OFF(t1)
    sync
    b       wait_to_jump_kernel
    nop
13:
    dli     t1, NODE3_CORE0_BUF0;
    sw      $0, FN_OFF(t1)
    sw      $0, SP_OFF(t1)
    sync
    b       wait_to_jump_kernel
    nop

2:
    //boot core, wait all core0 mem init done
    li      a1, NODE_MEM_INIT_DONE

    dli     t0, NODE0_CORE0_BUF0
1:
    lw      a0, FN_OFF(t0)
    bne     a0, a1, 1b
    nop

#ifdef  MULTI_CHIP
    dli     t0, NODE1_CORE0_BUF0
1:
    lw      a0, FN_OFF(t0)
    bne     a0, a1, 1b
    nop
#endif

#ifdef  DUAL_3B
    dli     t0, NODE2_CORE0_BUF0
1:
    lw      a0, FN_OFF(t0)
    bne     a0, a1, 1b
    nop

    dli     t0, NODE3_CORE0_BUF0
1:
    lw      a0, FN_OFF(t0)
    bne     a0, a1, 1b
    nop
#endif
    //all core0 mem init done
    //load msize from ALL NODEs
    dli     t1, 0x0
    dli     t0, NODE0_CORE0_BUF0
    lw      a0, SP_OFF(t0)
    or      t1, t1, a0
#ifdef  MULTI_CHIP
    dli     t0, NODE1_CORE0_BUF0
    lw      a0, SP_OFF(t0)
    or      t1, t1, a0
#endif
#ifdef  DUAL_3B
    dli     t0, NODE2_CORE0_BUF0
    lw      a0, SP_OFF(t0)
    or      t1, t1, a0
    dli     t0, NODE3_CORE0_BUF0
    lw      a0, SP_OFF(t0)
    or      t1, t1, a0
#endif

    move    msize, t1

    PRINTSTR("\r\nsystem msize= 0x")
    move    a0, msize
    bal     hexserial
    nop
    PRINTSTR("\r\n")

    //set all core0 init done signal
#ifdef NODE1_BOOT
    dli     t0, NODE1_CORE0_BUF0
#else
    dli     t0, NODE0_CORE0_BUF0
#endif
    sw      $0, SP_OFF(t0)
    li      a1, ALL_CORE0_INIT_DONE
    sw      a1, FN_OFF(t0)
    sync
    b       MEM_INIT_DONE
    nop
#endif


    b       wait_to_jump_kernel
    nop
slave_core:
    //wait core0 of this node scache enabled
    .set     mips64
    mfc0    t1, $15, 1
    .set     mips3
    andi    a1, t1, 0xc
    dsll    a1, a1, 42
    dli     t0, NODE0_CORE0_BUF0
    or      t0, t0, a1
#ifdef LS3B
    andi    a1, t1, 0xc
    dsll    a1, a1, 12
    daddu   t0, t0, a1
#endif
    li      a1, NODE_SCACHE_ENABLED
1:
    lw      a0, SP_OFF(t0)
    bne     a1, a0, 1b
    nop

    bal     CPU_TLBClear
    nop
    bal     tlb_init
    nop
    bal     godson2_cache_init
    nop

#if 1
//jump to 0x9fcxxxxx
    la      t0, next
    dli     t1, 0xfffff
    and     t0, t1
    dli     t1, 0xffffffff9fc00000
    or      t0, t1
    jr      t0
    nop
next:

    sync
    nop
    nop
    nop
    nop
    ## enable kseg0 cachablilty####
    mfc0    t0, CP0_CONFIG
    lui     t1, 0xffff
    ori     t1, t1, 0xfff8
    and     t0, t0, t1
    ori     t0, t0, 0x3
    mtc0    t0, CP0_CONFIG
    ################################
#endif

wait_to_jump_kernel:

//cxk test mem
#ifdef DEBUG_DDR_MT
#ifdef NODE1_BOOT
    dli     t0, NODE1_CORE0_BUF0 #buf of cpu1
#else
    dli     t0, NODE0_CORE0_BUF0 #buf of cpu0
#endif
    li      a1, 0xcccc
1:
    lw      a0, FN_OFF(t0)
    bne     a0, a1, 1b
    nop
//code for test mem using multi-core

    .set mips64
    mfc0    t2, $15, 1
    .set mips3
    andi    t2, 0x3ff
//Test mem
    //node 1 cores jump to 5f
    and     a1, t2, 0xc
    bnez    a1, 5f
    nop
    and     a1, t2, 0x3
    dli     a2, 0x0
    beq     a1, a2, 10f
    nop
    daddu   a2, a2, 0x1
    beq     a1, a2, 1f
    nop
    daddu   a2, a2, 0x1
    beq     a1, a2, 2f
    nop
    daddu   a2, a2, 0x1
    beq     a1, a2, 3f
    nop
    b       4f
    nop
10:
    b       10b
    nop
    dli     s1, 0x0004000080000000
    dli     t1, 0x1010
    bal     test_mem_1_silence
    nop
    b       10b
    nop
    b       4f
    nop
1:
    dli     s1, 0x10040000a0000000
    dli     t1, 0x1010
    bal     test_mem_1_silence
    nop
    b       1b
    nop
    b       4f
    nop
2:
    dli     s1, 0x20040000c0000000
    dli     t1, 0x1010
    bal     test_mem_1_silence
    //dli     t0, 0x5aa55a5aa55aa5a5
    //bal     simple_test_mem
    nop
    b       2b
    nop
    b       4f
    nop
3:
    dli     s1, 0x30040000e0000000
    dli     t1, 0x1010
    bal     test_mem_1_silence
    nop
    b       3b
    nop

    b       4f
    nop

5:  //NODE 1 cores
    daddiu  a1, a1, -4
    bnez    a1, 5f
    nop
    and     a1, t2, 0x3
    dli     a2, 0x0
    beq     a1, a2, 10f
    nop
    daddu   a2, a2, 0x1
    beq     a1, a2, 1f
    nop
    daddu   a2, a2, 0x1
    beq     a1, a2, 2f
    nop
    daddu   a2, a2, 0x1
    beq     a1, a2, 3f
    nop
    b       4f
    nop
10:
    b       10b
    nop
    dli     s1, 0x4004000080000000
    dli     t1, 0x1010
    bal     test_mem_1_silence
    nop
    b       10b
    nop
    b       4f
    nop
1:
    dli     s1, 0x50040000a0000000
    dli     t1, 0x1010
    bal     test_mem_1_silence
    nop
    b       1b
    nop
    b       4f
    nop
2:
    dli     s1, 0x60040000c0000000
    dli     t1, 0x1010
    bal     test_mem_1_silence
    //dli     t0, 0x4bb44b4bb44bb4b4
    //bal     simple_test_mem
    nop
    b       2b
    nop
    b       4f
    nop
3:
    dli     s1, 0x70040000e0000000
    dli     t1, 0x1010
    bal     test_mem_1_silence
    nop
    b       3b
    nop

    b       4f
    nop

5:  //NODE 2 cores
    daddiu  a1, a1, -4
    bnez    a1, 5f
    nop
    and     a1, t2, 0x3
    dli     a2, 0x0
    beq     a1, a2, 10f
    nop
    daddu   a2, a2, 0x1
    beq     a1, a2, 1f
    nop
    daddu   a2, a2, 0x1
    beq     a1, a2, 2f
    nop
    daddu   a2, a2, 0x1
    beq     a1, a2, 3f
    nop
    b       4f
    nop
10:
    b       10b
    nop
    dli     s1, 0x8004000080000000
    dli     t1, 0x1010
    bal     test_mem_1_silence
    nop
    b       10b
    nop
    b       4f
    nop
1:
    dli     s1, 0x90040000a0000000
    dli     t1, 0x1010
    bal     test_mem_1_silence
    nop
    b       1b
    nop
    b       4f
    nop
2:
    dli     s1, 0xa0040000c0000000
    dli     t1, 0x1010
    bal     test_mem_1_silence
    //dli     t0, 0x4bb44b4bb44bb4b4
    //bal     simple_test_mem
    nop
    b       2b
    nop
    b       4f
    nop
3:
    dli     s1, 0xb0040000e0000000
    dli     t1, 0x1010
    bal     test_mem_1_silence
    nop
    b       3b
    nop

    b       4f
    nop
5:  //NODE 3 cores

4:
1:
    b       1b
    nop
#endif


/******************************************************************/
/* Read Mail BOX to judge whether current core can jump to kernel */
/******************************************************************/
/**********************
t0: core ID
t1: core mailbox base address
t2: jump address
t3: temp
************************/
    .set    mips64
    mfc0    t0, $15, 1
    .set    mips3

    andi    t0, 0x3ff
    andi    t3, t0,0xc

    beq     t3, 0x0, node00;
    nop     

    beq     t3, 0x4, node01;
    nop     

    beq     t3, 0x8, node02;
    nop     

    beq     t3, 0xc, node03;
    nop     

node00:
    dli     t1, NODE0_CORE0_BUF0;
    b       core_x
    nop

node01:
    dli     t1, NODE1_CORE0_BUF0;
    b       core_x
    nop

node02:
    dli     t1, NODE2_CORE0_BUF0;
    b       core_x
    nop

node03:
    dli     t1, NODE3_CORE0_BUF0;
    b       core_x
    nop

core_x:
    //add cores offset of every node
    andi    t3, t0, 0x3
    dsll    t3, t3, 8
    or      t1, t1, t3

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

#endif

#ifndef DDR3_DIMM
//#define MULTI_NODE_DDR_PARAM
#endif
#include "ddr_config.S"

    .text
    .global ddr2_reg_data
    .global ddr3_reg_data
#ifdef  ARB_LEVEL
    .global ddr2_leveled_mark
    .global ddr2_reg_data_mc0_leveled
    .global ddr2_reg_data_mc1_leveled
#ifdef  MULTI_CHIP
    .global n1_ddr2_reg_data_mc0_leveled
    .global n1_ddr2_reg_data_mc1_leveled
#endif
#endif

    .align  5
#include "loongson3A3_ddr_param.S"
#ifdef  MULTI_NODE_DDR_PARAM
#include "loongson3A3_ddr_param_c1.S"
#endif

#ifdef  ARB_LEVEL
#include "loongson3A3_ddr_param.lvled.S"
#ifdef  MULTI_CHIP
#include "loongson3A3_ddr_param_c1.lvled.S"
#endif

    .align  12
ddr2_leveled_mark:
.dword 0x0
    .align  12
#else
#ifdef FIX_DDR_PARAM
//#include "loongson3A3_ddr_param.fix.S"
#include "loongson3A3_ddr_param.fix.S.UDIMM-2G"
#endif
#endif
