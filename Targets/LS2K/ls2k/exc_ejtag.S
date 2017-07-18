#define CP0_HWRENA $7

#.align	7			/* bfc00480 */
////////////////////////////////////////////
/* only concern mem copy in bios now */
/* used below addres to simulate virtual EJTAG registers */
/* for DUMP ALL registers and restore context */
/* COMMON USED REGISTERS */
#define EJTAG_V0		0xff200000 	
#define EJTAG_V1		0xff200008 	
#define EJTAG_A0		0xff200010 	
#define EJTAG_A1		0xff200018 	
#define EJTAG_A2		0xff200020 	
#define EJTAG_A3		0xff200028 	
#define EJTAG_T0		0xff200030 	
#define EJTAG_T1		0xff200038 	
#define EJTAG_T2		0xff200040 	
#define EJTAG_T3		0xff200048 	
#define EJTAG_T4		0xff200050 	
#define EJTAG_T5		0xff200058 	
#define EJTAG_T6		0xff200060 	
#define EJTAG_T7		0xff200068 	
#define EJTAG_T8		0xff200070 	
#define EJTAG_T9		0xff200078 	
#define EJTAG_S0		0xff200080 	
#define EJTAG_S1		0xff200088 	
#define EJTAG_S2		0xff200090 	
#define EJTAG_S3		0xff200098 	
#define EJTAG_S4		0xff2000a0 	
#define EJTAG_S5		0xff2000a8 	
#define EJTAG_S6		0xff2000b0 	
#define EJTAG_S7		0xff2000b8 	
#define EJTAG_K0		0xff2000c0 	
#define EJTAG_K1		0xff2000c8 	
#define EJTAG_GP		0xff2000d0 	
#define EJTAG_SP		0xff2000d8 	
#define EJTAG_FP		0xff2000e0 	
#define EJTAG_RA		0xff2000e8 	

/* CP0 registers */
#define EJTAG_INDEX		0xff200100 	
#define EJTAG_RANDOM	0xff200108 	
#define EJTAG_ENTRYLO0  0xff200110 	
#define EJTAG_ENTRYLO1  0xff200118 	
#define EJTAG_CONTEXT	0xff200120 	
#define EJTAG_PAGEMASK  0xff200128 	
#define EJTAG_WIRED		0xff200130 	
#define EJTAG_HWRENA	0xff200138 	
#define EJTAG_BADVADDR  0xff200140 	
#define EJTAG_COUNT		0xff200148 	
#define EJTAG_ENTRYHI	0xff200150 	
#define EJTAG_COMPARE	0xff200158 	
#define EJTAG_STATUS	0xff200160 	
#define EJTAG_CAUSE		0xff200168 	
#define EJTAG_EPC		0xff200170 	
#define EJTAG_PRID		0xff200178 	
#define EJTAG_CONFIG	0xff200180 	
#define EJTAG_CONFIG1	0xff200188 	
#define EJTAG_CONFIG2	0xff200190 	
#define EJTAG_LLADDR	0xff200198 	
#define EJTAG_WATCHLO	0xff2001a0 	
#define EJTAG_WATCHHI	0xff2001a8 	
#define EJTAG_XCONTEXT  0xff2001b0 	
#define EJTAG_DEPC		0xff2001b8 	
#define EJTAG_ERROREPC  0xff2001c0 	
#define EJTAG_DESAVE	0xff2001c8 	

#define DUMPREGS_START  EJTAG_V0
#define DUMPREGS_END    EJTAG_DESAVE
#define CONTEXT_REGCNT  10
#define REGS_TATOAL     47
#define DUMP_CNT        (REGS_TATOAL - CONTEXT_REGCNT)


/* for memory access */
#define STATUSREG	0xff200200 	/* identify wheter begin to start bios copying */
#define ADDRREG		0xff200208      /* address to access memory or CP0 index */
#define DATAREG		0xff200210      /* value to sotre to mem or move to CP0 */
#define LENGREG		0xff200218      /* length of memory to access */
#define TYPEREG		0xff200220      /* type for access mem or CP0 */
#define DBGREG		0xff200228      /* get data during DBG modle */
#define TLBENREG	0xff200230      /* identify wheterh use TLB */
#define CMDCNTREG	0xff200238 	/* identify wheter begin to start bios copying */
#define EPCREG		0xff200240 	/* identify wheter begin to start bios copying */


#define CP0_DESAVE $31

//#define	CONTEXTREG	    0xff200000
#define	CONTEXTREG		EJTAG_T0 
#define STATUSREG       0xff200200      /* identify wheter begin to start bios copying */
#define ADDRREG         0xff200208      /* address to access memory or CP0 index */
#define DATAREG         0xff200210      /* value to sotre to mem or move to CP0 */
#define LENGREG         0xff200218      /* length of memory to access */
#define TYPEREG         0xff200220      /* type for access mem or CP0 */
#define DBGREG          0xff200228      /* type for access mem or CP0 */
#define TLBENREG		0xff200230      /* identify whether enabel TLB DUMP */
#define CMDCNTREG       0xff200238      /* cmd counter */
#define EPCREG			0xff200240      /* cmd counter */

//#define BIOS_ENTRY	0xa1000000      /* cmd counter */
#define BIOS_ENTRY		0x8f010000      /* cmd counter */

/********************************************/
// type
#define BYTE  0
#define HALF  1
#define WORD  2
#define DWORD 3 
// mem
#define CP0   1
#define MEM   0 
// dircetion 
#define WTOLS3    0
#define RFROMLS3  1 
// reserved
//#define NULL  0
#define TLB   0
#define CACHE 1

#define RESERVE_OFFSET  0x4
#define TYPE_MASK       0x3
#define MEM_MASK        0x4
#define DIR_MASK        0x8
#define MEM_OFFSET      0x2
#define DIR_OFFSET      0x3

#define IRET            0xf0

// below is operation TYPE
/* sd means to store a data to some address fixed which are loaded from X86 host */
#define SD  (DWORD | MEM  << MEM_OFFSET | WTOLS3 << DIR_OFFSET)
/* ld means to load a data to x86 host from some address which are loaded from X86 host */
#define LD  (DWORD | MEM  << MEM_OFFSET | RFROMLS3 << DIR_OFFSET)
/* sd means to store a data to some address fixed which are loaded from X86 host */
#define SW  (WORD | MEM  << MEM_OFFSET  | WTOLS3 << DIR_OFFSET)
/* lw means to load a data to x86 host from some address which are loaded from X86 host */
#define LW  (WORD | MEM  << MEM_OFFSET  | RFROMLS3 << DIR_OFFSET)
/* sw means to store a data to some address fixed which are loaded from X86 host */
#define SH  (HALF | MEM  << MEM_OFFSET  | WTOLS3 << DIR_OFFSET)
/* lh means to load a data to x86 host from some address which are loaded from X86 host */
#define LH  (HALF | MEM  << MEM_OFFSET  | RFROMLS3 << DIR_OFFSET)
/* sh means to store a data to some address fixed which are loaded from X86 host */
#define SB  (BYTE | MEM  << MEM_OFFSET  | WTOLS3 << DIR_OFFSET)
/* lb means to load a data to x86 host from some address which are loaded from X86 host */
#define LB  (BYTE | MEM  << MEM_OFFSET  | RFROMLS3 << DIR_OFFSET)
/* sb means to store a data to some address fixed which are loaded from X86 host */

#define LDDW      SD    
#define LDW       SW              
#define LDH       SH              
#define LDB       SB    
#define STDW      LD    
#define STW       LW         
#define STH       LH    
#define STB       LB    

/********************************************/

#define DUMPREGS		0x0f
#define	ERET			0xf0

#define DEBUG  //it seems has bug here, because cpu stop if not in DEBUG

    .set mips64
    // save context
    dmtc0   t0, CP0_DESAVE
	li		t0,	CONTEXTREG	     
    sd      t1, 0x08(t0)
    dmfc0   t1, CP0_DESAVE
	sd		t1, 0x00(t0)	// save t0
	li		t0,	CONTEXTREG	     
    sd      t2, 0x10(t0)
    sd      t3, 0x18(t0)
    sd      t4, 0x20(t0)
    sd      t5, 0x28(t0)
    sd      t6, 0x30(t0)
    sd      t7, 0x38(t0)
    sd      t8, 0x40(t0)
    sd      t9, 0x48(t0)
    dmfc0   t1, CP0_STATUS
	li		t0, EJTAG_STATUS
    sd      t1, 0x00(t0)
	li		t0, EJTAG_CONFIG
    dmfc0   t1, CP0_CONFIG
    sd      t1, 0x00(t0)
	li		t0, EJTAG_CAUSE
    dmfc0   t1, CP0_CAUSE
    sd      t1, 0x00(t0)

	li		t0, TLBENREG			
    ld	    t1, 0x0(t0)
    beqz    t1, tlbdump_end
    nop 
tlbdump_end:

	/* t6 used as cmd counter */
	li		t6,0x0

	/* t7: get cmds total number to execute */
	li		t0, STATUSREG
	ld		t7,	0x0(t0)
#ifdef DEBUG
	li		t0, DBGREG
	sd		t7, 0x0(t0)
#endif
	beqz	t7, 12f
	nop

11:
	/* get cmds type */
	li		t0, TYPEREG
	ld		t1,	0x0(t0)
#ifdef DEBUG
	li		t0, DBGREG
	sd		t1, 0x0(t0)
#endif

	/* check whether current cmds is "eret" */
	bne	    t1,ERET, 15f
	nop
	li		t0, EPCREG
	//lw		t2,	0x0(t0)
	ld		t2,	0x0(t0)
	//dmtc0	t2,CP0_DEXC_PC
	dmtc0	t2, CP0_DEPC

/* should avoid this because C client doesn't support */
##ifdef DEBUG
#	li		t0, DBGREG
#	sw		t2, 0x0(t0)
##endif
	#li		t2,BIOS_ENTRY  
// to check whether this cmds and below nop lead PC dead !!!

#if 1
	#.set	at
	#.set	push
	#.set	mips3 
	#eret	
	#.set	pop
	#.set	noreorder
	#.set	volatile
	nop
    // now not used jump
	li		t0,	CONTEXTREG
    ld      t1, 0x08(t0)
    ld      t2, 0x10(t0)
    ld      t3, 0x18(t0)
    ld      t4, 0x20(t0)
    ld      t5, 0x28(t0)
    ld      t6, 0x30(t0)
    ld      t7, 0x38(t0)
    ld      t8, 0x40(t0)
    ld      t9, 0x48(t0)
	#mtc0	t8, CP0_STATUS_REG
	#ld		t8, 0x48(t0)
	#mtc0	t8, CP0_CONFIG
	#ld		t8, 0x50(t0)
	#mtc0	t8, CP0_CAUSE_REG
    dmfc0   t0, CP0_DESAVE

	#mtc0	t2,CP0_EXC_PC
    .set mips64
	deret	
    .set mips3
	nop
	nop
#endif

	#b		12f		
	#nop

15:
	/* first check whether local instruction is to dump registers */
	bne	t1, DUMPREGS, 16f
	nop
	li	  t0, EJTAG_V0
	sd	  v0, 0x0(t0)
	li	  t0, EJTAG_V1
	sd	  v1, 0x0(t0)
	li	  t0, EJTAG_A0
	sd	  a0, 0x0(t0)
	li	  t0, EJTAG_A1
	sd	  a1, 0x0(t0)
	li	  t0, EJTAG_A2
	sd	  a2, 0x0(t0)
	li	  t0, EJTAG_A3
	sd	  a3, 0x0(t0)
	li	  t0, EJTAG_S0
	sd	  s0, 0x0(t0)
	li	  t0, EJTAG_S1
	sd	  s1, 0x0(t0)
	li	  t0, EJTAG_S2
	sd	  s2, 0x0(t0)
	li	  t0, EJTAG_S3
	sd	  s3, 0x0(t0)
	li	  t0, EJTAG_S4
	sd	  s4, 0x0(t0)
	li	  t0, EJTAG_S5
	sd	  s5, 0x0(t0)
	li	  t0, EJTAG_S6
	sd	  s6, 0x0(t0)
	li	  t0, EJTAG_S7
	sd	  s7, 0x0(t0)
	li	  t0, EJTAG_K0
	sd	  k0, 0x0(t0)
	li	  t0, EJTAG_K1
	sd	  k1, 0x0(t0)
	li	  t0, EJTAG_GP
	sd	  gp, 0x0(t0)
	li	  t0, EJTAG_SP
	sd	  sp, 0x0(t0)
	li	  t0, EJTAG_FP
	sd	  s8, 0x0(t0)
	li	  t0, EJTAG_RA
	sd	  ra, 0x0(t0)
	
	li	  t0, EJTAG_INDEX
    dmfc0 t1, CP0_INDEX
	sd	  t1, 0x0(t0)	
	li	  t0, EJTAG_RANDOM
    dmfc0 t1, CP0_RANDOM
	sd	  t1, 0x0(t0)	
	li	  t0, EJTAG_RANDOM
    dmfc0 t1, CP0_ENTRYLO0
	sd	  t1, 0x0(t0)	
	li	  t0, EJTAG_ENTRYLO1
    dmfc0 t1, CP0_ENTRYLO1
	sd	  t1, 0x0(t0)	
	li	  t0, EJTAG_CONTEXT
    dmfc0 t1, CP0_CONTEXT
	sd	  t1, 0x0(t0)	
	li	  t0, EJTAG_PAGEMASK
    dmfc0 t1, CP0_PAGEMASK
	sd	  t1, 0x0(t0)	
    li	  t0, EJTAG_WIRED
    dmfc0 t1, CP0_WIRED
	sd	  t1, 0x0(t0)	
	.set mips64
	li	  t0, EJTAG_HWRENA
    mfc0  t1, CP0_HWRENA,0
	.set mips3
	sd	  t1, 0x0(t0)	
    li	  t0, EJTAG_BADVADDR
    dmfc0 t1, CP0_BADVADDR
	sd	  t1, 0x0(t0)	
    li    t0, EJTAG_COUNT
    dmfc0 t1, CP0_COUNT
	sd	  t1, 0x0(t0)	
    li    t0, EJTAG_ENTRYHI
    dmfc0 t1, CP0_ENTRYHI
	sd	  t1, 0x0(t0)	
    li    t0, EJTAG_COMPARE
    dmfc0 t1, CP0_COMPARE
	sd	  t1, 0x0(t0)	
    li    t0, EJTAG_STATUS
    dmfc0 t1, CP0_STATUS
	sd	  t1, 0x0(t0)	
    li    t0, EJTAG_CAUSE
    dmfc0 t1, CP0_CAUSE
	sd	  t1, 0x0(t0)	
    li    t0, EJTAG_EPC
    dmfc0 t1, CP0_EPC
	sd	  t1, 0x0(t0)	
    li    t0, EJTAG_PRID
    dmfc0 t1, CP0_PRID
	sd	  t1, 0x0(t0)	
    li    t0, EJTAG_CONFIG
    dmfc0 t1, CP0_CONFIG
	sd	  t1, 0x0(t0)	
    li    t0, EJTAG_LLADDR
    dmfc0 t1, CP0_LLADDR
	sd	  t1, 0x0(t0)	
    li    t0, EJTAG_WATCHLO
    dmfc0 t1, CP0_WATCHLO
	sd	  t1, 0x0(t0)	
    li    t0, EJTAG_WATCHHI
    dmfc0 t1, CP0_WATCHHI
	sd	  t1, 0x0(t0)	
    li    t0, EJTAG_XCONTEXT
    dmfc0 t1, CP0_XCONTEXT
	sd	  t1, 0x0(t0)	
    li    t0, EJTAG_DEPC
    dmfc0 t1, CP0_DEPC
	sd	  t1, 0x0(t0)	
    li    t0, EJTAG_ERROREPC
    dmfc0 t1, CP0_ERROREPC
	sd	  t1, 0x0(t0)	
	/* get t0 from CP0_DESAVE and save it */
	li	  t0, EJTAG_DESAVE
    dmfc0 t1, CP0_DESAVE
	sd	  t1, 0x0(t0)	

	/* below dump TLB Registers */
	//#include "lookuptlb_ejtag.S" 
	#include "lookuptlb.S" 
	

	b	  18f
	nop

16:
	
	/* do sd/sw/ld/lw according value in TYPEREG */
	andi	t3, t1,0xf;	
	
	/* get accessing mem length */
	li		t0, LENGREG
	ld		t2,	0x0(t0)
#ifdef DEBUG
	li		t0, DBGREG
	sd		t2, 0x0(t0)
#endif
	#beqz	t2,11f
	beqz	t2,11b
	nop
14:
	/* t4: get address for a store or load instruction */
	li		t0,	ADDRREG
	ld		t4, 0x0(t0)
#ifdef DEBUG
	li		t0, DBGREG
	sd		t4, 0x0(t0)
#endif

	/* if ld/lw instruction, no need ld DATAREG */
	bge		t3,0x8, 110f
	nop

	/* t5: get data for a store or load insturction */
	li		t0,	DATAREG
	ld		t5, 0x0(t0) //WARING HERRE!!!! first ld, but later lw maybe
//#ifdef DEBUG
#if 1
	li		t0, DBGREG
	sd		t5, 0x0(t0)
#endif

110:
	beq		t3,	LDDW, 1110f
	nop
	beq		t3, LDW,  1111f
	nop
	beq		t3, LDH,  1114f
	nop
	beq		t3, LDB,  1116f
	nop

	beq		t3, STDW,  1112f
	nop
	beq		t3, STW,  1113f
	nop
	beq		t3, STH,  1115f
	nop
	beq		t3, STB,  1117f
	nop


 1110:
	sd		t5, 0x0(t4)
	b		113f		
	nop
 1111:
	/* why using delay slot like below fail ? */
	sw		t5, 0x0(t4)
	b		113f		
	nop
 1114:
	/* why using delay slot like below fail ? */
	sh		t5, 0x0(t4)
	b		113f		
	nop
 1116:
	/* why using delay slot like below fail ? */
	sb		t5, 0x0(t4)
	b		113f		
	nop

 1112:
	ld		t5, 0x0(t4)
##ifdef DEBUG
#	li		t0, DBGREG
#	sd		t5, 0x0(t0)
##endif
	li		t0, DATAREG
	sd		t5, 0x0(t0)
	b		113f		
	nop
 1113:
	lw		t5, 0x0(t4)
	li		t0, DATAREG
	sw		t5, 0x0(t0)
	b		113f
	nop
 1115:
	lh		t5, 0x0(t4)
	li		t0, DATAREG
	sh		t5, 0x0(t0)
	b		113f
	nop
 1117:
	lb		t5, 0x0(t4)
	li		t0, DATAREG
	sb		t5, 0x0(t0)
	b		113f
	nop

113:
	subu	t2, t2, 0x1
	bne		t2, $0, 14b
	nop

18:
 	/* added for cmd executed counter */
	addu	t6, t6,0x1;
	li		t0, CMDCNTREG
	sd		t6, 0x0(t0)
	/* to check whether all cmds is done */
	bne		t6, t7, 11b
	nop

    // restore context
12:
	li		t0,	CONTEXTREG
    ld      t1, 0x08(t0)
    ld      t2, 0x10(t0)
    ld      t3, 0x18(t0)
    ld      t4, 0x20(t0)
    ld      t5, 0x28(t0)
    ld      t6, 0x30(t0)
    ld      t7, 0x38(t0)
    ld      t8, 0x40(t0)
    ld      t9, 0x48(t0)
	#mtc0	t8, CP0_STATUS_REG
	#ld		t8, 0x48(t0)
	#mtc0	t8, CP0_CONFIG
	#ld		t8, 0x50(t0)
	#mtc0	t8, CP0_CAUSE_REG

    dmfc0   t0, CP0_DESAVE

    .set mips64
    deret
    .set mips3
////////////////////////////////////////////
