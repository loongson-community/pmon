#define c0_badvaddr "$8"
#define c0_status "$12"
#define c0_cause "$13"
#define c0_epc "$14"
#define c0_debug "$23"
#define c0_depc "$24"
#define c0_desave "$31"

#include <ri.h>
#include <asm/mipsregs.h>

//typedef unsigned long long u64;
u64  __raw__readq(u64 addr);
u64 __raw__writeq(u64 addr, u64 val);

int ejtag_stack[1024];
extern char ejtag_exception[], ejtag_exception_end[];
asm(
"ejtag_exception:;\n"
".global ejtag_exception;\n"
".global ejtag_exception_end;\n"
".set mips64;\n"
".set noreorder;\n"
".set noat\n"
"mtc0 $k0, $31;\n"
"mfc0 $k0," c0_debug "\n"
"srl $k0,20;\n"
"andi $k0,1;\n"
"bnez $k0,2f;\n"
"nop;\n"
"la $k0, ejtag_stack;\n"
"b 3f;\n"
"nop;\n"
"2:\n"
"move $k0, $sp;\n"
"3:\n"
"subu $k0,$k0," STR(PT_SIZE) ";\n"
"sw $1," STR(PT_R1) "($k0);\n"
"sw $2," STR(PT_R2) "($k0);\n"
"sw $3," STR(PT_R3) "($k0);\n"
"sw $4," STR(PT_R4) "($k0);\n"
"sw $5," STR(PT_R5) "($k0);\n"
"sw $6," STR(PT_R6) "($k0);\n"
"sw $7," STR(PT_R7) "($k0);\n"
"sw $8," STR(PT_R8) "($k0);\n"
"sw $9," STR(PT_R9) "($k0);\n"
"sw $10," STR(PT_R10) "($k0);\n"
"sw $11," STR(PT_R11) "($k0);\n"
"sw $12," STR(PT_R12) "($k0);\n"
"sw $13," STR(PT_R13) "($k0);\n"
"sw $14," STR(PT_R14) "($k0);\n"
"sw $15," STR(PT_R15) "($k0);\n"
"sw $16," STR(PT_R16) "($k0);\n"
"sw $17," STR(PT_R17) "($k0);\n"
"sw $18," STR(PT_R18) "($k0);\n"
"sw $19," STR(PT_R19) "($k0);\n"
"sw $20," STR(PT_R20) "($k0);\n"
"sw $21," STR(PT_R21) "($k0);\n"
"sw $22," STR(PT_R22) "($k0);\n"
"sw $23," STR(PT_R23) "($k0);\n"
"sw $24," STR(PT_R24) "($k0);\n"
"sw $25," STR(PT_R25) "($k0);\n"
"sw $27," STR(PT_R27) "($k0);\n"
"sw $28," STR(PT_R28) "($k0);\n"
"sw $29," STR(PT_R29) "($k0);\n"
"sw $30," STR(PT_R30) "($k0);\n"
"sw $31," STR(PT_R31) "($k0);\n"
"mfc0 $v0," c0_status ";\n"
"sw $v0," STR(PT_STATUS) "($k0);\n"
"mflo $v0;\n"
"sw $v0," STR(PT_LO) "($k0);\n"
"mfhi $v0;\n"
"sw $v0," STR(PT_HI) "($k0);\n"
"mfc0 $v0," c0_badvaddr ";\n"
"sw $v0," STR(PT_BVADDR) "($k0);\n"
"mfc0 $v0," c0_cause ";\n"
"sw $v0," STR(PT_CAUSE) "($k0);\n"
"mfc0 $v0," c0_depc ";\n"
"sw $v0," STR(PT_EPC) "($k0);\n"
"move $a0,$k0;\n"
"addiu $sp,$k0,-32;\n"
"dla $v0, ejtag_enter;\n"
"jalr $v0;\n"
"nop;\n"
"1:\n"
"addiu $k0,$sp,32;\n"
"lw $v0," STR(PT_EPC) "($k0);\n"
"mtc0 $v0," c0_depc ";\n"
"lw $v0," STR(PT_CAUSE) "($k0);\n"
"mtc0 $v0," c0_cause ";\n"
"lw $v0," STR(PT_BVADDR) "($k0);\n"
"mtc0 $v0," c0_badvaddr ";\n"
"lw $v0," STR(PT_HI) "($k0);\n"
"mthi $v0;\n"
"lw $v0," STR(PT_LO) "($k0);\n"
"mtlo $v0;\n"
"lw $v0," STR(PT_STATUS) "($k0);\n"
"mtc0 $v0," c0_status ";\n"
"lw $1," STR(PT_R1) "($k0);\n"
"lw $2," STR(PT_R2) "($k0);\n"
"lw $3," STR(PT_R3) "($k0);\n"
"lw $4," STR(PT_R4) "($k0);\n"
"lw $5," STR(PT_R5) "($k0);\n"
"lw $6," STR(PT_R6) "($k0);\n"
"lw $7," STR(PT_R7) "($k0);\n"
"lw $8," STR(PT_R8) "($k0);\n"
"lw $9," STR(PT_R9) "($k0);\n"
"lw $10," STR(PT_R10) "($k0);\n"
"lw $11," STR(PT_R11) "($k0);\n"
"lw $12," STR(PT_R12) "($k0);\n"
"lw $13," STR(PT_R13) "($k0);\n"
"lw $14," STR(PT_R14) "($k0);\n"
"lw $15," STR(PT_R15) "($k0);\n"
"lw $16," STR(PT_R16) "($k0);\n"
"lw $17," STR(PT_R17) "($k0);\n"
"lw $18," STR(PT_R18) "($k0);\n"
"lw $19," STR(PT_R19) "($k0);\n"
"lw $20," STR(PT_R20) "($k0);\n"
"lw $21," STR(PT_R21) "($k0);\n"
"lw $22," STR(PT_R22) "($k0);\n"
"lw $23," STR(PT_R23) "($k0);\n"
"lw $24," STR(PT_R24) "($k0);\n"
"lw $25," STR(PT_R25) "($k0);\n"
"lw $27," STR(PT_R27) "($k0);\n"
"lw $28," STR(PT_R28) "($k0);\n"
"lw $29," STR(PT_R29) "($k0);\n"
"lw $30," STR(PT_R30) "($k0);\n"
"lw $31," STR(PT_R31) "($k0);\n"
"mfc0 $k0, $31;\n"
"sync\n"
"deret;\n"
"ejtag_exception_end:\n"
"nop;\n"
".set mips0;\n"
);

/* These the fields of 32 bit mips instructions */
#define mips32_op(x) (x >> 26)
#define itype_op(x) (x >> 26)
#define itype_rs(x) ((x >> 21) & 0x1f)
#define itype_rt(x) ((x >> 16) & 0x1f)
#define itype_immediate(x) (x & 0xffff)

#define jtype_op(x) (x >> 26)
#define jtype_target(x) (x & 0x03ffffff)

#define rtype_op(x) (x >> 26)
#define rtype_rs(x) ((x >> 21) & 0x1f)
#define rtype_rt(x) ((x >> 16) & 0x1f)
#define rtype_rd(x) ((x >> 11) & 0x1f)
#define rtype_shamt(x) ((x >> 6) & 0x1f)
#define rtype_funct(x) (x & 0x3f)

long get_frame_register_signed(struct pt_regs *frame,int r)
{
	if(r<38)
		return frame->regs[r];
	else
		return 0;
}

static long long
mips32_relative_offset (unsigned long long inst)
{
  return ((itype_immediate (inst) ^ 0x8000) - 0x8000) << 2;
}

/* Determine where to set a single step breakpoint while considering
   branch prediction.  */
unsigned long long
mips32_next_pc (struct pt_regs *frame, unsigned long inst, unsigned long long pc)
{
  int op;
  if(inst==0x42000018) return __read_ulong_c0_register($14, 0);

  if ((inst & 0xe0000000) != 0)	/* Not a special, jump or branch instruction */
    {
      if (itype_op (inst) >> 2 == 5)
	/* BEQL, BNEL, BLEZL, BGTZL: bits 0101xx */
	{
	  op = (itype_op (inst) & 0x03);
	  switch (op)
	    {
	    case 0:		/* BEQL */
	      goto equal_branch;
	    case 1:		/* BNEL */
	      goto neq_branch;
	    case 2:		/* BLEZL */
	      goto less_branch;
	    case 3:		/* BGTZL */
	      goto greater_branch;
	    default:
	      pc += 4;
	    }
	}
      else if (itype_op (inst) == 17 && itype_rs (inst) == 8)
	/* BC1F, BC1FL, BC1T, BC1TL: 010001 01000 */
	{
	  int tf = itype_rt (inst) & 0x01;
	  int cnum = itype_rt (inst) >> 2;
	  int fcrcs =
	    get_frame_register_signed (frame, 38+33);
	  int cond = ((fcrcs >> 24) & 0x0e) | ((fcrcs >> 23) & 0x01);

	  if (((cond >> cnum) & 0x01) == tf)
	    pc += mips32_relative_offset (inst) + 4;
	  else
	    pc += 8;
	}
      else
	pc += 4;		/* Not a branch, next instruction is easy */
    }
  else
    {				/* This gets way messy */

      /* Further subdivide into SPECIAL, REGIMM and other */
      switch (op = itype_op (inst) & 0x07)	/* extract bits 28,27,26 */
	{
	case 0:		/* SPECIAL */
	  op = rtype_funct (inst);
	  switch (op)
	    {
	    case 8:		/* JR */
	    case 9:		/* JALR */
	      /* Set PC to that address */
	      pc = get_frame_register_signed (frame, rtype_rs (inst));
	      break;
	    default:
	      pc += 4;
	    }

	  break;		/* end SPECIAL */
	case 1:		/* REGIMM */
	  {
	    op = itype_rt (inst);	/* branch condition */
	    switch (op)
	      {
	      case 0:		/* BLTZ */
	      case 2:		/* BLTZL */
	      case 16:		/* BLTZAL */
	      case 18:		/* BLTZALL */
	      less_branch:
		if (get_frame_register_signed (frame, itype_rs (inst)) < 0)
		  pc += mips32_relative_offset (inst) + 4;
		else
		  pc += 8;	/* after the delay slot */
		break;
	      case 1:		/* BGEZ */
	      case 3:		/* BGEZL */
	      case 17:		/* BGEZAL */
	      case 19:		/* BGEZALL */
		if (get_frame_register_signed (frame, itype_rs (inst)) >= 0)
		  pc += mips32_relative_offset (inst) + 4;
		else
		  pc += 8;	/* after the delay slot */
		break;
		/* All of the other instructions in the REGIMM category */
	      default:
		pc += 4;
	      }
	  }
	  break;		/* end REGIMM */
	case 2:		/* J */
	case 3:		/* JAL */
	  {
	    unsigned long reg;
	    reg = jtype_target (inst) << 2;
	    /* Upper four bits get never changed... */
	    pc = reg + ((pc + 4) & ~(unsigned long long) 0x0fffffff);
	  }
	  break;
	  /* FIXME case JALX : */
	  {
	    unsigned long reg;
	    reg = jtype_target (inst) << 2;
	    pc = reg + ((pc + 4) & ~(unsigned long long) 0x0fffffff) + 1;	/* yes, +1 */
	    /* Add 1 to indicate 16 bit mode - Invert ISA mode */
	  }
	  break;		/* The new PC will be alternate mode */
	case 4:		/* BEQ, BEQL */
	equal_branch:
	  if (get_frame_register_signed (frame, itype_rs (inst)) ==
	      get_frame_register_signed (frame, itype_rt (inst)))
	    pc += mips32_relative_offset (inst) + 4;
	  else
	    pc += 8;
	  break;
	case 5:		/* BNE, BNEL */
	neq_branch:
	  if (get_frame_register_signed (frame, itype_rs (inst)) !=
	      get_frame_register_signed (frame, itype_rt (inst)))
	    pc += mips32_relative_offset (inst) + 4;
	  else
	    pc += 8;
	  break;
	case 6:		/* BLEZ, BLEZL */
	  if (get_frame_register_signed (frame, itype_rs (inst)) <= 0)
	    pc += mips32_relative_offset (inst) + 4;
	  else
	    pc += 8;
	  break;
	case 7:
	default:
	greater_branch:	/* BGTZ, BGTZL */
	  if (get_frame_register_signed (frame, itype_rs (inst)) > 0)
	    pc += mips32_relative_offset (inst) + 4;
	  else
	    pc += 8;
	  break;
	}			/* switch */
    }				/* else */
  return pc;
}				/* mips32_next_pc */

#define dMIPS32_DEBUG_IN_DEBUG               0x40000000
#define dMIPS32_DEBUG_SW_BKPT                0x00000002
#define dMIPS32_DEBUG_HW_INST_BKPT           0x00000010
#define dMIPS32_DEBUG_HW_DATA_LD_BKPT        0x00000004
#define dMIPS32_DEBUG_HW_DATA_ST_BKPT        0x00000008
#define dMIPS32_DEBUG_HW_DATA_LD_IMP_BKPT    0x00040000
#define dMIPS32_DEBUG_HW_DATA_ST_IMP_BKPT    0x00080000
#define dMIPS32_DEBUG_SINGLE_STEP_BKPT       0x00000001
#define dMIPS32_DEBUG_JTAG_BKPT              0x00000020

#define dMIPS32_DATA_BKPT_STATUS       0xFFFFFFFFFF302000LL
#define dMIPS32_DATA_BKPT_ADDR(X)      (0xFFFFFFFFFF302100LL + (X * 0x100))
#define dMIPS32_DATA_BKPT_ADDR_MSK(X)  (0xFFFFFFFFFF302108LL + (X * 0x100))
#define dMIPS32_DATA_BKPT_ASID(X)      (0xFFFFFFFFFF302110LL + (X * 0x100))
#define dMIPS32_DATA_BKPT_CONTROL(X)   (0xFFFFFFFFFF302118LL + (X * 0x100))
#define dMIPS32_DATA_BKPT_VALUE(X)     (0xFFFFFFFFFF302120LL + (X * 0x100))

#define dMIPS32_INST_BKPT_STATUS       0xFFFFFFFFFF301000LL
#define dMIPS32_INST_BKPT_ADDR(X)      0xFFFFFFFFFF301100LL + (X * 0x100)
#define dMIPS32_INST_BKPT_ADDR_MSK(X)  0xFFFFFFFFFF301108LL + (X * 0x100)
#define dMIPS32_INST_BKPT_ASID(X)      0xFFFFFFFFFF301110LL + (X * 0x100)
#define dMIPS32_INST_BKPT_CONTROL(X)   0xFFFFFFFFFF301118LL + (X * 0x100)
#define ejtag_readq  __raw__readq
#define ejtag_writeq(d, a) __raw__writeq(a, d)
#define ejtag_writel(l,addr) ((*(volatile unsigned int *)(addr)) = (l))
#define ejtag_readl(addr) (*(volatile unsigned int *)(addr))
static int config_gdbserver_nolb = 1;
static int config_gdbserver_nosb = 1;

enum kgdb_bptype {
	BP_BREAKPOINT = 0,
	BP_HARDWARE_BREAKPOINT,
	BP_WRITE_WATCHPOINT,
	BP_READ_WATCHPOINT,
	BP_ACCESS_WATCHPOINT
};

extern void *mclfree;

static long long watch_address;

static int set_inst_breakpoint(unsigned long long ui_address,unsigned long long ibm)
{

	unsigned int i;
	unsigned int ibc;
	int index = -1;
	int max_inst_breakpoints = (ejtag_readl(dMIPS32_INST_BKPT_STATUS)>>24) & 0xf;
	unsigned int tmpdbcn,value;

	// Set breakpoint - First check to make sure this address doesn't already
	// have a software breakpoint.  We only allow 1 breakpoint per address.
	for(i=0; i<max_inst_breakpoints; i++)
	{
		// Breakpoint is enabled, check to see the address

		
		if ((ejtag_readq(dMIPS32_INST_BKPT_CONTROL(i)) & 1) && ejtag_readq(dMIPS32_INST_BKPT_ADDR(i)) == ui_address)
		{
			index = i;
			// Bad bad bad!  Already have a breakpoint at this address!
			break;
		}
		else if(index == -1) index = i;
	}

	if(index == -1)
	{
		// No open slots!
		return -ENOMEM;
	}
	i = index;

	// Ok - This is an open slot - Mark enabled - Read old value - 
	// Write new value - Store address

	ibc = 1;

	ejtag_writeq(ui_address, dMIPS32_INST_BKPT_ADDR(i));
	ejtag_writeq(ibm, dMIPS32_INST_BKPT_ADDR_MSK(i));
	//ejtag_writeq(asid, dMIPS32_DATA_BKPT_ASID(i));
	ejtag_writeq(ibc, dMIPS32_INST_BKPT_CONTROL(i));

	return 0;
}

static int unset_inst_breakpoint(unsigned long long ui_address)
{
	unsigned int i;
	int max_inst_breakpoints = (ejtag_readl(dMIPS32_INST_BKPT_STATUS)>>24) & 0xf;

	// Search for the breakpoint
	for(i=0; i<max_inst_breakpoints; i++)
	{
		if (ejtag_readq(dMIPS32_INST_BKPT_CONTROL(i))&1)
		{
			if (ejtag_readq(dMIPS32_INST_BKPT_ADDR(i)) == ui_address)
			{

				ejtag_writeq(0, dMIPS32_INST_BKPT_CONTROL(i));
				return(1);
			}
		}
	}

	return -ENOENT;
}

static int set_data_breakpoint(unsigned long long ui_address,unsigned long long dbm,unsigned int ui_addrsize,int cmpdata, unsigned long long data_value,enum kgdb_bptype action)
{

	unsigned int i;
	int index = -1;
	int max_data_breakpoints = (ejtag_readl(dMIPS32_DATA_BKPT_STATUS)>>24) & 0xf;
	unsigned int tmpdbcn,value;

	// Set breakpoint - First check to make sure this address doesn't already
	// have a software breakpoint.  We only allow 1 breakpoint per address.
	for(i=0; i<max_data_breakpoints; i++)
	{
		// Breakpoint is enabled, check to see the address
		if (ejtag_readq(dMIPS32_DATA_BKPT_ADDR(i)) == ui_address)
		{
			index = i;
			// Bad bad bad!  Already have a breakpoint at this address!
			break;
		}
		else if(index == -1) index = i;
	}

	if(index == -1)
	{
		// No open slots!
		return -ENOMEM;
	}
	i = index;

	// Ok - This is an open slot - Mark enabled - Read old value - 
	// Write new value - Store address

	value = data_value;


	tmpdbcn=1;	//set BE
	switch (ui_addrsize) {
		case 8:
			break;
		case 4:
			if(1) 
			{
				tmpdbcn=tmpdbcn|((0xff^(0xf<<(ui_address&4)))<<14);	//BAI
				tmpdbcn=tmpdbcn|((0xff^(0xf<<(ui_address&4)))<<4);	//BLM
			}
			break;
		case 2:
			tmpdbcn=tmpdbcn|((0xff^(0x3<<(ui_address&6)))<<14);	//BAI
			tmpdbcn=tmpdbcn|((0xff^(0x3<<(ui_address&6)))<<4);	//BLM
			break;
		case 1:
			tmpdbcn=tmpdbcn|((0xff^(0x1<<(ui_address&7)))<<14);	//BAI
			tmpdbcn=tmpdbcn|((0xff^(0x1<<(ui_address&7)))<<4);	//BLM
			break;
		default:
			break;
	}

	if(!cmpdata)
		tmpdbcn |= (0xff<<4); //BAI, byte access ignore data bus
	if(action == BP_WRITE_WATCHPOINT && config_gdbserver_nosb)
		tmpdbcn |= (1<<12); //NOLB
	else if(action == BP_READ_WATCHPOINT && config_gdbserver_nolb)
		tmpdbcn |= (1<<13); //NOSB

	ejtag_writeq(ui_address, dMIPS32_DATA_BKPT_ADDR(i));
	ejtag_writeq(dbm, dMIPS32_DATA_BKPT_ADDR_MSK(i));
	//ejtag_writeq(asid, dMIPS32_DATA_BKPT_ASID(i));
	if(cmpdata)
		ejtag_writeq(value, dMIPS32_DATA_BKPT_VALUE(i));
	ejtag_writeq(tmpdbcn, dMIPS32_DATA_BKPT_CONTROL(i));

	return 0;
}

static int unset_data_breakpoint(unsigned long long ui_address)
{
	unsigned int i;
	int max_data_breakpoints = (ejtag_readl(dMIPS32_DATA_BKPT_STATUS)>>24) & 0xf;

	// Search for the breakpoint
	for(i=0; i<max_data_breakpoints; i++)
	{
		if (ejtag_readq(dMIPS32_DATA_BKPT_CONTROL(i))&1)
		{
			if (ejtag_readq(dMIPS32_DATA_BKPT_ADDR(i)) == ui_address)
			{

				ejtag_writeq(0, dMIPS32_DATA_BKPT_CONTROL(i));
				return(1);
			}
		}
	}

	return -ENOENT;
}

int ejtag_enter(struct pt_regs *ejtag_frame)
{
	int debug = read_c0_debug();
	if(!(debug & 0x3f))
	{
	tgt_printf("ejtag debug exception debug=0x%x, epc=0x%x cause=0x%x, badaddr=0x%x\n", debug, ejtag_frame->cp0_epc, (debug>>10)&0x1f, ejtag_frame->cp0_badvaddr);
	}
	
	if(debug & dMIPS32_DEBUG_SW_BKPT)
	{
	  int cmd_set = ejtag_frame->regs[2];
	  long address = ejtag_frame->regs[4];
	  long length = ejtag_frame->regs[5];
	  enum kgdb_bptype action = ejtag_frame->regs[6];
	  
	  if(cmd_set)	 
	    set_data_breakpoint(address, 0, length , 0, 0, action);
	  else
	    unset_data_breakpoint(address);
	  ejtag_frame->cp0_epc += 4;

	}
	else if(debug & (dMIPS32_DEBUG_HW_DATA_LD_BKPT|dMIPS32_DEBUG_HW_DATA_ST_BKPT|dMIPS32_DEBUG_HW_DATA_LD_IMP_BKPT|dMIPS32_DEBUG_HW_DATA_ST_IMP_BKPT))
	{
		ejtag_writel(0, dMIPS32_DATA_BKPT_STATUS);
		if(!data_breakpoint_handler(ejtag_frame))
			unset_data_breakpoint(watch_address);
		else
		{
			*(int *)0xa0100480 = 0x4200001f;
		 
		}
		{
			int epc = ejtag_frame->cp0_epc;
			unsigned int inst = *(unsigned int *)epc;
			int npc; 
			npc = mips32_next_pc(ejtag_frame, inst, epc);
			set_inst_breakpoint(npc , 0);
		}
	}
	else if(debug & dMIPS32_DEBUG_HW_INST_BKPT)
	{
		int epc = ejtag_frame->cp0_epc;
	
		ejtag_writel(0, dMIPS32_INST_BKPT_STATUS);
		unset_inst_breakpoint(epc);
	    	set_data_breakpoint(watch_address, 0, 4 , 0, 0, BP_WRITE_WATCHPOINT);
	}
	return 0;
}

static int arch_set_hw_breakpoint(long  address,int length, enum kgdb_bptype type)
{
	register unsigned long __a0 asm("$4") = (unsigned long) address; 
	register unsigned long __a1 asm("$5") = (unsigned long) length;
	register unsigned long __a2 asm("$6") = (unsigned long) type;
	register unsigned long __v0 asm("$2") = (unsigned long) 1;
	if(read_c0_debug() & dMIPS32_DEBUG_IN_DEBUG)
	    set_data_breakpoint(address, 0, length , 0, 0, type);
	else

	asm volatile(
			".set mips64;\n"
			"sdbbp;\n"
			".set mips0;\n"
			::"r"(__a0),"r"(__a1),"r"(__a2),"r"(__v0):"memory"
		    );
	return 0;
}

static int arch_remove_hw_breakpoint(long  address,int length, enum kgdb_bptype type)
{
	register unsigned long __a0 asm("$4") = (unsigned long) address; 
	register unsigned long __a1 asm("$5") = (unsigned long) length;
	register unsigned long __a2 asm("$6") = (unsigned long) type;
	register unsigned long __v0 asm("$2") = (unsigned long) 0;
	if(read_c0_debug() & dMIPS32_DEBUG_IN_DEBUG)
	    unset_data_breakpoint(address);
	else
	asm volatile(
			".set mips64;\n"
			"sdbbp;\n"
			".set mips0;\n"
			::"r"(__a0),"r"(__a1),"r"(__a2),"r"(__v0)
		    );
	return 0;
}

int data_breakpoint_handler(struct pt_regs *ejtag_frame)
{
	int epc = ejtag_frame->cp0_epc;
	unsigned int inst = *(unsigned int *)epc; 
	int rt = (inst>>16)&0x1f;
	int rb = (inst>>21)&0x1f;
	int offs = (short)(inst&0xffff);
	int a = ejtag_frame->regs[rb] + offs;
	int v = ejtag_frame->regs[rt];
	if(v == -1)
	{
		tgt_printf("0x%08x:m4 0x%x 0x%x\n", ejtag_frame->cp0_epc, a, v);
		return 1;
	}
	return 0;
}

static int ejtag_init()
{
long address = &mclfree;
  memcpy(0xa0100000, 0x9fc00000, 0x100000);
  memcpy((void *)(int)0x80100480, ejtag_exception,  ejtag_exception_end-ejtag_exception);
  memcpy((void *)(int)0xa0100480, ejtag_exception,  ejtag_exception_end-ejtag_exception);
 watch_address = address;
  arch_set_hw_breakpoint(watch_address, 4, BP_WRITE_WATCHPOINT);

  return 0;
}


