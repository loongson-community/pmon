/**********************************
    Author: chenxinke
    Date:   20150707
    mc_init for 3A4000
    v1.0
    input:
    t7(option ARB_LEVEL)--do arb level, 0--not level; 1--do level;
    t3--MC select: 0--MC0; 1--MC1
**********************************/
//#include "lsmc_config_param.S"
#define WAIT_ITEM   0x60
#define SPD_REGS_INFO(x,slotid)	GET_SPD_SLOT(0x148,slotid);dsll v0, 56; move x, v0;	\
				GET_SPD_SLOT(0x147,slotid);dsll v0, 48; or x, v0;	\
				GET_SPD_SLOT(0x146,slotid);dsll v0, 40; or x, v0;	\
				GET_SPD_SLOT(0x145,slotid);dsll v0, 32; or x, v0;	\
				GET_SPD_SLOT(0x144,slotid);dsll v0, 24; or x, v0;	\
				GET_SPD_SLOT(0x143,slotid);dsll v0, 16; or x, v0;	\
				GET_SPD_SLOT(0x141,slotid);dsll v0,  8; or x, v0;	\
				GET_SPD_SLOT(0x140,slotid);dsll v0,  0; or x, v0;

#define GET_REG_B(x,y) \
    dsrl    v0, x, (y<<3);\
    and     v0, 0xff

#define STORE_REG_B(x,y,z) \
    and     x, ~(0xff<<(y<<3));\
    and     z, 0xff;\
    dsll    z, (y<<3);\
    or      x, z

#define DDR_TTYBIT \
	la	v0, hexchar;\
	daddu	v0, s0;\
	daddu	v0, a0;\
    dsubu   v0, 0x2000000;\
	bal	tgt_putchar;\
	lbu	a0, 0(v0)

#define DDR_TTYSTRING(x) \
    .rdata;\
98:;\
    .asciz x;\
    .text;\
    la      a0, 98b;\
	daddu	a1, a0, s0;\
    dsubu   a1, 0x20000000;\
	lbu	    a0, 0(a1);\
97:;\
	beqz	a0, 96f;\
	nop;\
	bal	tgt_putchar;\
	addiu	a1, 1;\
	b	97b;\
	lbu	a0, 0(a1);\
96:

#define WAIT_FOR(x) \
    dli     v0, x;\
82:;\
    dsubu   v0, 1;\
    bnez    v0, 82b;\
    nop

#define GET_CS_NUM_DDR4 \
    lbu     v0, DDR4_CS_ENABLE_OFFSET(t8);\
    .set    mips32;\
    clz     v0, v0;\
    .set    mips3;\
    subu    v0, 32;\
    abs     v0, v0

#define GET_LVL_CS_NUM \
    lbu     v0, LVL_CS_OFFSET(t8);\
    .set    mips32;\
    clz     v0, v0;\
    .set    mips3;\
    subu    v0, 32;\
    abs     v0, v0;\
    subu    v0, 1

#define GET_PARAM_STORE_FLASH_BASE_v0 \
    GET_NODE_ID_a1;\
    mul     a1, DIMM_INFO_SIZE;\
    dli     v0, (0xffffffff00000000 | DIMM_INFO_IN_FLASH_OFFS);\
    daddu   v0, a1;\
    mul     v1, k0, MC_INFO_SIZE;\
    daddu   v0, v1

#define mrs_cmd_a   a0
#define mrs_cs      a1
#define mrs_num     a2

#define MRS_SEND(mrs_send_mrs_cmd_a,mrs_send_mrs_cs,mrs_send_mrs_num)  \
    and     mrs_send_mrs_cmd_a, 0x3ffff;\
    and     mrs_send_mrs_num, 0x7;\
    dsll    mrs_send_mrs_num, 20;\
    or      a0, mrs_send_mrs_cmd_a, mrs_send_mrs_num;\
    and     mrs_send_mrs_cs, 0x7;\
    dsll    mrs_send_mrs_cs, 24;\
    or      a0, mrs_send_mrs_cs;\
    GET_SDRAM_TYPE_V1;\
    beq     a1, 3, 80f;\
    nop;\
    GET_DIMM_TYPE_V1;\
    andi    a1, a1, 1;\
    beqz    a1, 80f;\
    nop;\
    bal     rdimm_mrs_send;\
    nop;\
    b       81f;\
    nop;\
80:;\
    bal     mrs_send;\
    nop;\
81:

#define MR7_SEND(mr7_send_mrs_cmd_a,mr7_send_mrs_cs)  \
    and     mr7_send_mrs_cmd_a, 0x3ffff;\
    or      a0, mr7_send_mrs_cmd_a, (0x7<<20);\
    and     mr7_send_mrs_cs, 0x7;\
    dsll    mr7_send_mrs_cs, 24;\
    or      a0, mr7_send_mrs_cs;\
    bal     mrs_send;\
    nop

#define RDIMM_MR7_SEND(rdimm_mr7_mrs_cmd_a) \
    move    a3, rdimm_mr7_mrs_cmd_a;\
    bal     get_mrs7_cs;\
    nop;\
    move    a2, v0;\
    beq     v1, 0xf, 83f;\
    nop;\
    move    mrs_cs, v1;\
    move    mrs_cmd_a, a3;\
    MR7_SEND(mrs_cmd_a, mrs_cs);\
83:;\
    beq     a2, 0xf, 83f;\
    move    mrs_cs, a2;\
    move    mrs_cmd_a, a3;\
    MR7_SEND(mrs_cmd_a, mrs_cs);\
83:

        .global mc_init
        .ent    mc_init
        .set    noreorder
        .set    novolatile
        .set    mips3
mc_init:
    move    s4, ra
#ifndef DDR3_DIMM
#define ENABLE_DDR_VREF_TRAINING
#define ENABLE_BIT_TRAINING
#endif
#define TPHY_WR_TRAINING
//#define TPHY_WRLAT_DEBUG
    sync
    nop
    nop
    nop
    nop

    move    k0, t3
    MC_ENABLE(MC0_ONLY_EN)
    beqz    t3, 1f
    nop
    MC_ENABLE(MC1_ONLY_EN)

1:

    sync
    nop
    nop
    nop
    nop

#if 1
    TTYDBG("\r\nEnable register space of MEMORY\r\n")
    move    a1, k0
    bal     enable_mc_regs_default
    nop

    move    a1, k0
    bal     enable_mc_conf_space
    nop

    GET_NODE_ID_a0;
    dli     t8, DDR_MC_CONFIG_BASE
    or      t8, t8, a0
    GET_SDRAM_TYPE_V1
    dli     t1, 0x3
    beq     t1, a1, 3f
    nop
    dli     t1, 0x4
    beq     t1, a1, 4f
    nop
    //not DDR3 and not DDR4, errors
    PRINTSTR("\r\n!!! ERROR: NOT recognized DDR SDRAM TYPE. !!!\r\n");
1:
    b       1b
    nop
3:  //DDR3
    GET_DIMM_TYPE_V1
    andi    a1, a1, 1
    bnez    a1, 1f
    nop
    //UDIMM
    dla     a2, ddr3_reg_data
    beqz    t3, 21f
    nop
    dla     a2, ddr3_reg_data_mc1
21:
#ifdef  MULTI_NODE_DDR_PARAM
    GET_NODE_ID_a1
    beqz    a1, 4f
    nop
    dla     a2, n1_ddr3_reg_data
    beqz    t3, 21f
    nop
    dla     a2, n1_ddr3_reg_data_mc1
21:
#endif
    b       9f
    nop
1:  //RDIMM
    dla     a2, ddr3_RDIMM_reg_data
    beqz    t3, 21f
    nop
    dla     a2, ddr3_RDIMM_reg_data_mc1
21:
#ifdef  MULTI_NODE_DDR_PARAM
    GET_NODE_ID_a1
    beqz    a1, 4f
    nop
    dla     a2, n1_ddr3_RDIMM_reg_data
    beqz    t3, 21f
    nop
    dla     a2, n1_ddr3_RDIMM_reg_data_mc1
21:
#endif
    b       9f
    nop

4:  //DDR4
    GET_DIMM_TYPE_V1
    and     a1, a1, 1
    bnez    a1, 1f
    nop
    //UDIMM or sodimm
    dla     a2, ddr4_reg_data
    beqz    t3, 21f
    nop
    dla     a2, ddr4_reg_data_mc1
21:
#ifdef  MULTI_NODE_DDR_PARAM
    GET_NODE_ID_a1
    beqz    a1, 4f
    nop
    dla     a2, n1_ddr4_reg_data
    beqz    t3, 21f
    nop
    dla     a2, n1_ddr4_reg_data_mc1
21:
#endif
    b       9f
    nop
1:  //RDIMM or LRDIMM
    dla     a2, ddr4_RDIMM_reg_data
    beqz    t3, 21f
    nop
    dla     a2, ddr4_RDIMM_reg_data_mc1
21:
#ifdef  MULTI_NODE_DDR_PARAM
    GET_NODE_ID_a1
    beqz    a1, 4f
    nop
    dla     a2, n1_ddr4_RDIMM_reg_data
    beqz    t3, 21f
    nop
    dla     a2, n1_ddr4_RDIMM_reg_data_mc1
21:
#endif
    b       9f
    nop
9:

    bal     mc_config
    nop

#ifdef  PRINT_DDR_LEVELING   //print registers
    PRINTSTR("\r\nThe MC param is:\r\n")
    PRINTSTR("PHY:\r\n")
    dli     t1, 290
    GET_NODE_ID_a0
    dli     t5, DDR_MC_CONFIG_BASE
    or      t5, t5, a0
1:
    ld      t6, 0x0(t5)
    move    a0, t5
    and     a0, a0, 0xffff
    bal     hexserial
    nop
    PRINTSTR(":  ")
    dsrl    a0, t6, 32
    bal     hexserial
    nop
    //PRINTSTR("  ")
    move    a0, t6
    bal     hexserial
    nop
    PRINTSTR("\r\n")

    daddiu  t1, t1, -1
    daddiu  t5, t5, 8
    bnez    t1, 1b
    nop

    PRINTSTR("\r\nCTRL:\r\n")
    dli     t1, 231
    GET_NODE_ID_a0
    dli     t5, DDR_MC_CONFIG_BASE
    or      t5, t5, a0
    ori     t5, 0x1000

1:
    ld      t6, 0x0(t5)
    move    a0, t5
    and     a0, a0, 0xffff
    bal     hexserial
    nop
    PRINTSTR(":  ")
    dsrl    a0, t6, 32
    bal     hexserial
    nop
    //PRINTSTR("  ")
    move    a0, t6
    bal     hexserial
    nop
    PRINTSTR("\r\n")

    daddiu  t1, t1, -1
    daddiu  t5, t5, 8
    bnez    t1, 1b
    nop
#endif

    move    a1, k0
    bal     disable_mc_conf_space
    nop
#endif

#ifndef DISABLE_HARD_LEVELING
    //DDR3 SDRAM, do hard leveling
    PRINTSTR("\r\nStart Hard Leveling...")
    TTYDBG("\r\nEnable register space of MEMORY\r\n")
    move    a1, k0
    bal     enable_mc_conf_space
    nop


4:

wait_dram_init_done_new:
    ld      t0, (CTL_ADDRESS + 0x100)(t8)
    andi    t1, t0, 0xff     //cs_enable
    ld      t0, (PHY_ADDRESS + DRAM_INIT)(t8)
    andi    t0, t0, 0xff00
    dsrl    t0, t0, 0x8
    bne     t0, t1, wait_dram_init_done_new
    nop

    GET_DIMM_TYPE_V1
    andi    a1, a1, 1
    beqz    a1, leveling
    nop

    PRINTSTR("\r\nStart to config RCD")

#ifndef DDR3_DIMM
//RDIMM
//set mirror and RDIMM snoop in F0RC0D in RCD
    GET_ADDR_MIRROR_V1
    beqz    a1, 1f
    nop

    li      mrs_cmd_a, 0xdc
    b       2f
    nop
1:
    li      mrs_cmd_a, 0xd4
2:
    RDIMM_MR7_SEND(mrs_cmd_a)

//set CA CS drive strength in F0RC03 in RCD
    GET_SPD(0x89)
    dsrl    mrs_cmd_a, v0, 4
    or      mrs_cmd_a, 0x30
    RDIMM_MR7_SEND(mrs_cmd_a)

//set ODT CKE drive strength in F0RC04 in RCD
    GET_SPD(0x89)
    and     mrs_cmd_a, v0, 3
    sll     mrs_cmd_a, 2
    srl     v0, 2
    andi    v0, 0x3
    or      mrs_cmd_a, v0
    or      mrs_cmd_a, 0x40
    RDIMM_MR7_SEND(mrs_cmd_a)

//set Clock drive strength in F0RC05 in RCD
    GET_SPD(0x8a)
    and     mrs_cmd_a, v0, 3
    sll     mrs_cmd_a, 2
    srl     v0, 2
    andi    v0, 0x3
    or      mrs_cmd_a, v0
    or      mrs_cmd_a, 0x50
    RDIMM_MR7_SEND(mrs_cmd_a)

//set RDIMM operate speed in F0RC0A in RCD
    dli     a0, DDR_FREQ
    dsll    a0, 2
    bgtu    a0, 1600, 1f
    nop
    dli     a0, 0
    b       2f
    nop
1:
    bgtu    a0, 1867, 1f
    nop
    dli     a0, 1
    b       2f
    nop
1:
    bgtu    a0, 2134, 1f
    nop
    dli     a0, 2
    b       2f
    nop
1:
    bgtu    a0, 2400, 1f
    nop
    dli     a0, 3
    b       2f
    nop
1:
    bgtu    a0, 2667, 1f
    nop
    dli     a0, 4
    b       2f
    nop
1:
    dli     a0, 0x5
2:
    or      mrs_cmd_a, a0, 0xa0
    RDIMM_MR7_SEND(mrs_cmd_a)

//set RDIMM fine granularity of RDIMM operating speed in RC3x in RCD
    dli     a0, DDR_FREQ
    dsll    a0, 2
    bgtu    a0, 1240, 1f
    nop
    dli     a0, 0
    b       2f
    nop
1:
    dsubu   a0, 1240
    ddivu   a0, a0, 20
    mfhi    t0
    bnez    t0, 2f
    nop
    dsubu   a0, 1
2:
    or      mrs_cmd_a, a0, 0x300
    RDIMM_MR7_SEND(mrs_cmd_a)

    /* power saving setting */
    /* F0RC09*/
    GET_CS_NUM_DDR4
    bgtu    v0, 1, 1f
    nop
    dli     mrs_cmd_a, 0x93
    RDIMM_MR7_SEND(mrs_cmd_a)
1:
    /* F0RC08*/
    dli     mrs_cmd_a, 0x84
    /* disable QC[2:0] */
    dli     a1, 0x3
    GET_CS_NUM_DDR4
    bne     v0, 4, 1f
    nop
    and     a1, 0x1
1:
    or      mrs_cmd_a, a1
    /* disable DA17 */
    GET_ROW_SIZE_V1
    beqz    a1, 1f
    nop
    or      mrs_cmd_a, 0x8
1:
    RDIMM_MR7_SEND(mrs_cmd_a)
    /* F0RC02 */
    dli     mrs_cmd_a, 0x22
    GET_ROW_SIZE_V1
    beqz    a1, 1f
    nop
    or      mrs_cmd_a, 0x1
1:
    RDIMM_MR7_SEND(mrs_cmd_a)

    PRINTSTR("\r\nConfig RCD done\r\n")

    /*send mrs to side B for rdimm*/
    /*get cs number in t1 */
    GET_CS_NUM_DDR4
    move    t1, v0

    /*mrs send for different cs loop*/
    li      t0, 0       //cs number ctrl
5:
    /*mrs send for different mrs loop, mrs_num followring thr ddr init sequence:3,6,5,4,2,1,0*/
    li      t2, 0       //mrs number ctrl
2:
    dsll    t3, t0, 4
    daddu   t3, t8
    /*translate mrs_number ctrl t2 to mrs_num */
    bnez    t2, 1f
    nop
    li      mrs_num, 3
    b       3f
    nop
1:
    subu    mrs_num, t2, 7
    abs     mrs_num
    bleu    t2, 3, 3f
    nop
    subu    mrs_num, 1
3:
    dsll    t4, mrs_num, 1
    daddu   t3, t4
    lh      mrs_cmd_a, DDR4_MR0_CS0_REG(t3)
    move    mrs_cs, t0
    MRS_SEND(mrs_cmd_a,mrs_cs,mrs_num)
    /*mrs send for different mrs loop ctrl*/
    daddu   t2, 1
    bleu    t2, 6, 2b
    nop

    /*mrs send for different cs loop ctrl */
    daddu   t0, 1
    bltu    t0, t1, 5b
    nop

    //sync time
    ld      t3, (PHY_ADDRESS + 0x010)(t8)
    ld      t3, (PHY_ADDRESS + 0x010)(t8)
    ld      t3, (PHY_ADDRESS + 0x010)(t8)

#endif

leveling:
    /*disable dll reset in MR REG*/
    li      t1, 0
    move    t2, t8
1:
    lh      t0, DDR4_MR0_CS0_REG(t2)
    and     t0, t0, ~(0x1<<8)
    sh      t0, DDR4_MR0_CS0_REG(t2)
    daddu   t1, t1, 1
    daddu   t2, t2, 0x10
    bleu    t1, 7, 1b
    nop
//load vref store
#ifndef DDR3_DIMM
#ifdef VREF_STORE
#if 1//if it's old CPU, skip
    GET_NODE_ID_a0
    or      a1, a0, 0x900004000ff00000
    dsll    t4, k0, 40
    daddu   t4, a1
    lb      a1, 0x0(t4)
    /*if version is 0x10,don't need training*/
    beq     a1, 0x10, 4f
    nop
#endif
/* t1:save flash base; t2:save slot 1 SPD info; t4:save slot 2 SPD info; */
    GET_NODE_ID_a1
    mul     a1, DIMM_INFO_SIZE
    dli     t1, (0xffffffff00000000 | DIMM_INFO_IN_FLASH_OFFS)
    daddu   t1, a1

    mul     a0, k0, MC_INFO_SIZE	//if k0==1, add mc1 offset
    daddu   t1, a0

#ifdef  AUTO_DDR_CONFIG
    /* read SPD info */
    SPD_REGS_INFO(t2,0);
    SPD_REGS_INFO(t4,1);
#endif

    /*check DDR freq */
    li      a1, DDR_FREQ
    lw      a0, DIMM_OFFS_CLK(t1)
    bne     a1, a0, 4f
    nop

#ifdef  AUTO_DDR_CONFIG
    /*mc0:k0=0; mc1:k0=1*/
    /* check slot0 SPD info */
    ld      a0, DIMM_OFFS_SLOT0_SPD(t1)
    bne     t2, a0, 4f
    nop
    /* check slot1 SPD info */
    ld      a0, DIMM_OFFS_SLOT1_SPD(t1)
    bne     t4, a0, 4f
    nop
#endif

#ifdef ENABLE_MC_VREF_TRAINING
    /*Using configuration in flash*/
    daddu   t4, t8, VREF_CTRL_DS0_OFFSET
    /* store mc vref param */
    GET_PARAM_STORE_FLASH_BASE_v0
    daddiu  a0, v0, DIMM_OFFS_VREF
    dli     a1, 0
3:
    lb      t0, 0x0(a0)
    sll     t0, 0x5
    ori     t0, 0x1
    sh      t0, 0x0(t4)

    daddiu  a0, 0x1
    daddiu  t4, 0x2
    daddiu  a1, 1

    bltu   a1, DIMM_VREF_DATA_NUM, 3b
    nop
#endif

#ifdef ENABLE_DDR_VREF_TRAINING
    GET_PARAM_STORE_FLASH_BASE_v0
    daddiu  t3, v0, DIMM_OFFS_DDR_VREF      //flash base in t3
    GET_CS_NUM_DDR4
    move    t4, v0      //cs number in t4
    /*vref set cs loop */
    dli     t0, 0
1:
    dsll    t1, t0, 4
    daddu   t1, t8
    /*enable vref training mode*/
    lhu     mrs_cmd_a, DDR4_MR6_CS0_REG(t1)
    and     mrs_cmd_a, ~(0x7f)
    or      mrs_cmd_a, 0x80
    move    mrs_cs, t0
    li      mrs_num, 6
    MRS_SEND(mrs_cmd_a,mrs_cs,mrs_num)

    /*set ddr vref*/
    lhu     mrs_cmd_a, DDR4_MR6_CS0_REG(t1)
    and     mrs_cmd_a, ~(0x7f)
    or      mrs_cmd_a, 0x80
    lb      t2, 0x0(t3)
    or      mrs_cmd_a, t2
    move    mrs_cs, t0
    li      mrs_num, 6
    MRS_SEND(mrs_cmd_a,mrs_cs,mrs_num)

    /*disable vref training mode*/
    lhu     mrs_cmd_a, DDR4_MR6_CS0_REG(t1)
    and     mrs_cmd_a, ~(0xff)
    move    mrs_cs, t0
    li      mrs_num, 6
    MRS_SEND(mrs_cmd_a,mrs_cs,mrs_num)

    daddu   t0, 1
    daddu   t3, 1
    bltu    t0, t4, 1b
    nop
#endif

#ifdef ENABLE_BIT_TRAINING
    /*Using configuration in flash*/
    /* store vref param */
    GET_PARAM_STORE_FLASH_BASE_v0
    daddiu  a0, v0, DIMM_OFFS_BIT_TRAIN     //flash base in t3
    dli     t0, 0
3:
    dsll    t1, t0, 7
    daddu   t3, t8, t1

    lb      t1, 0x0(a0)
    sb      t1, DLL_WRDQS_OFFSET(t3)
    daddu   a0, 2
    lh      t1, 0x0(a0)
    sh      t1, DLL_RDDQS0_OFFSET(t3)
    daddu   a0, 2
    ld      t1, 0x0(a0)
    sd      t1, WRDQ_BDLY00_OFFSET(t3)
    daddu   a0, 8
    ld      t1, 0x0(a0)
    sd      t1, RDQSP_BDLY00_OFFSET(t3)
    daddu   a0, 8
    ld      t1, 0x0(a0)
    sd      t1, RDQSN_BDLY00_OFFSET(t3)
    daddu   a0, 8

    daddu   t0, 1
    bltu    t0, DIMM_VREF_DATA_NUM, 3b
    nop
#endif

4:

#endif
#endif

#ifdef  DEBUG_DDR_PARAM   //print registers
    PRINTSTR("The MC param before leveling is:\r\n")
    PRINTSTR("PHY:\r\n")
    dli     t1, 290
    GET_NODE_ID_a0
    dli     t5, DDR_MC_CONFIG_BASE
    or      t5, t5, a0
1:
    ld      t6, 0x0(t5)
    move    a0, t5
    and     a0, a0, 0xffff
    bal     hexserial
    nop
    PRINTSTR(":  ")
    dsrl    a0, t6, 32
    bal     hexserial
    nop
    //PRINTSTR("  ")
    move    a0, t6
    bal     hexserial
    nop
    PRINTSTR("\r\n")

    daddiu  t1, t1, -1
    daddiu  t5, t5, 8
    bnez    t1, 1b
    nop

    PRINTSTR("\r\nCTRL:\r\n")
    dli     t1, 231
    GET_NODE_ID_a0
    dli     t5, DDR_MC_CONFIG_BASE
    or      t5, t5, a0
    ori     t5, 0x1000

1:
    ld      t6, 0x0(t5)
    move    a0, t5
    and     a0, a0, 0xffff
    bal     hexserial
    nop
    PRINTSTR(":  ")
    dsrl    a0, t6, 32
    bal     hexserial
    nop
    //PRINTSTR("  ")
    move    a0, t6
    bal     hexserial
    nop
    PRINTSTR("\r\n")

    daddiu  t1, t1, -1
    daddiu  t5, t5, 8
    bnez    t1, 1b
    nop
//#endif
//#ifdef  DEBUG_DDR_PARAM   //Change parameters of MC
    GET_NODE_ID_a0;
    dli     a1, DDR_MC_CONFIG_BASE
    or      t8, a0, a1

    PRINTSTR("\r\nChange some parameters of MC:");
1:
    PRINTSTR("\r\nPlease input the register number you want to change!!!(0xffff:jump out.): ");
    dli     t6, 0x00
    bal     inputaddress
    nop
    move    t5, v0

    dli     a1, 0xffff
    bge     t5, a1, 2f    #if input address offset exceed range,jump out
    nop
    and     t5, t5, 0xfff8
    daddu   t5, t5, t8

    PRINTSTR("\r\nPlease input the data-hex: ");
    dli     t6, 0x00
    bal     inputaddress
    nop
    sd      v0, 0x0(t5)    #v0 is the input value

    //print the new register value
    move    t6, t5
    PRINTSTR("\r\nRegister 0x")
    dsubu   t5, t5, t8
    move    a0, t5
    bal     hexserial
    nop
    PRINTSTR(": ")
    ld      t6, 0x0(t6)
    dsrl    a0, t6, 32
    bal     hexserial
    nop
    move    a0, t6
    bal     hexserial
    nop

    b        1b
    nop
2:
#endif
    bal     ddr4_leveling
    nop

    /* now dram read is accessable */
    move    a1, k0
    bal     disable_mc_regs_default
    nop

#ifndef DDR3_DIMM
#ifdef VREF_STORE
/* t1:save flash base; t2:save SPD info; t3:save sdram base; */
    GET_NODE_ID_a1
    mul     a1, DIMM_INFO_SIZE
    dli     t3, DIMM_INFO_IN_CACHE_OFFS
    daddu   t3, a1
    dli     t1, (0xffffffff00000000 | DIMM_INFO_IN_FLASH_OFFS)
    daddu   t1, a1

    mul     a0, k0, MC_INFO_SIZE	//if k0==1, add mc1 offset
    daddu   t3, a0
    daddu   t1, a0

#ifdef  AUTO_DDR_CONFIG
    /* read SPD info */
    SPD_REGS_INFO(t2,0);
    SPD_REGS_INFO(t4,1);
#endif

    /*check DDR freq */
    li      a1, DDR_FREQ
    lw      a0, DIMM_OFFS_CLK(t1)
    bne     a1, a0, 4f
    nop

#ifdef  AUTO_DDR_CONFIG
    /*mc0:k0=0; mc1:k0=1*/
    /* check slot0 SPD info */
    ld      a0, DIMM_OFFS_SLOT0_SPD(t1)
    bne     t2, a0, 4f
    nop
    /* check slot1 SPD info */
    ld      a0, DIMM_OFFS_SLOT1_SPD(t1)
    bne     t4, a0, 4f
    nop
#endif

    b       vref_end
    nop

4:
#endif
#ifdef ENABLE_MC_VREF_TRAINING
    bal     mc_vref_training
    nop
#endif
#ifdef TPHY_WR_TRAINING
    /*enter vref training mode*/
    lhu     mrs_cmd_a, DDR4_MR6_CS0_REG(t8)
    and     mrs_cmd_a, ~(0x7f)
    or      mrs_cmd_a, 0x80
    li      mrs_cs, 0
    li      mrs_num, 6
    MRS_SEND(mrs_cmd_a,mrs_cs,mrs_num)

    /*set ddr vref*/
    lhu     mrs_cmd_a, DDR4_MR6_CS0_REG(t8)
    and     mrs_cmd_a, ~(0x7f)
    or      mrs_cmd_a, 0xa5
    li      mrs_cs, 0
    li      mrs_num, 6
    MRS_SEND(mrs_cmd_a,mrs_cs,mrs_num)

    /*exit vref training mode*/
    lhu     mrs_cmd_a, DDR4_MR6_CS0_REG(t8)
    and     mrs_cmd_a, ~(0xff)
    li      mrs_cs, 0
    li      mrs_num, 6
    MRS_SEND(mrs_cmd_a,mrs_cs,mrs_num)
#endif

vref_end:
#endif
#ifdef TPHY_WR_TRAINING

#ifdef TPHY_WR_MODE0
//tPHY_WRDATA training, to compare the data to decide the value of tPHY_WRDATA
    PRINTSTR("\r\nstart training of tPHY_WR\r\n")

        dli     t3, 0x0000f00000000000
	and	t3, t3, t8//get node id
	dli     t1, 0xb800000000000000
	or	t1, t1, t3
	dli     t2, 0x6666666666666666
	sd      t2, 0x0(t1)
	dli     t2, 0x5555555555555555
	sd      t2, 0x8(t1)
	dli     t2, 0x4444444444444444
	sd      t2, 0x10(t1)
	dli     t2, 0x3333333333333333
	sd      t2, 0x18(t1)
	dli     t2, 0x2222222222222222
	sd      t2, 0x20(t1)
	dli     t2, 0x1111111111111111
	sd      t2, 0x28(t1)
	dli     t2, 0x0000000000000000
	sd      t2, 0x30(t1)
	dli     t2, 0x8888888888888888
	sd      t2, 0x38(t1)

    dli     t4, 0
wr_preamble_check:
	dli	t2, 0x0000ffffffffffff
	and	t1, t1, t2
	dli	t2, 0x9000000000000000
	or	t1, t1, t2
	ld	t2, 0x0(t1)
    PRINTSTR("\r\n 0x0:    ")
    dsrl    a0, t2, 32
    bal     hexserial
    nop
    move    a0, t2
    bal     hexserial
    nop
    dli     t3, 0x6666666666666666
    bne     t2, t3, 1f
    nop
    dli     t3, 0
	b	    set_value
    nop
1:
	//test if get 2222, we need to delay for two cycles
//	dli	t3, 0x2222222222222222
    lbu t2, 0x0(t1)
    dli t3, 0x22
	bne	t2, t3, 1f
	nop

	dli	t3, 0x0000020000000000
	b	set_value
	nop

1:
	//test if get 4444, we need to delay for one cycles
//	dli	t3, 0x4444444444444444
    dli t3, 0x44
	bne	t2, t3, 1f
	nop

	dli	t3, 0x0000010000000000
	b	set_value
	nop

1:
	//test if get 6666 at 0x10, we need to take one cycles forward
	lbu	t2, 0x10(t1)
    PRINTSTR("\r\n 0x10:    ")
    dsrl    a0, t2, 32
    bal     hexserial
    nop
    move    a0, t2
    bal     hexserial
    nop
//	dli	t3, 0x6666666666666666
    dli t3, 0x66
	bne	t2, t3, 1f
	nop

	dli	t3, 0xffffff0000000000//-1
	b	set_value
	nop

1:
	//test if get 6666 at 0x20, we need to take two cycles forward
	lbu	t2, 0x20(t1)
    PRINTSTR("\r\n 0x20:    ")
    dsrl    a0, t2, 32
    bal     hexserial
    nop
    move    a0, t2
    bal     hexserial
    nop
//	dli	t3, 0x6666666666666666
    dli t3, 0x66
	bne	t2, t3, 1f
	nop

	dli	t3, 0xfffffe0000000000//-1
	b	set_value
	nop

1:
	//Nothing to do, very good
    dsubu   a0,t1, t4
    ld  t2, 0x0(a0)
    PRINTSTR("\r\n 0x0:    ")
    dsrl    a0, t2, 32
    bal     hexserial
    nop
    move    a0, t2
    bal     hexserial
    nop
    dli t3, 0x6666666666666666
    beq t2, t3, 1f
    nop
    daddu   t4, 1
    daddu   t1, 1
    bleu    t4, 7, wr_preamble_check
    nop

1:
	dli	t3, 0x0

set_value:
    dli     t0, (CTL_ADDRESS + 0x60)
    or      t0, t0, t8
    ld      t2, (0x0)(t0)
    dadd    t2, t2, t3
    sd	    t2, 0x0(t0)
//for RDIMM
    GET_DIMM_TYPE_V1
    andi    a1, a1, 1
    beqz    a1, 10f
    nop

    ld      t2, 0x10(t1)
    dli     t4, 0x6666666644444444
    bne     t4, t2, 2f
    nop
//low 32 and ecc dly_2x + 1
    lb      t2, 0x1065(t8)
    subu    t2, 1
    sb      t2, 0x1065(t8)
    li      t0, 0
1:
    dsll    t2, t0, 7
    daddu   t2, t2, t8
    lb      t3, DLY_2X_OFFSET(t2)
    dadd    t3, 1
    sb      t3, DLY_2X_OFFSET(t2)
    dadd    t0, 1
    bleu    t0, 3, 1b
    nop
    dadd    t0, 4
    beq     t0, 8, 1b
    nop
    b       10f
    nop
2:
//high 32 dly2x + 1
    dli     t4, 0x2222222244444444
    bne     t4, t2, 10f
    nop
    li      t0, 4
1:
    dsll    t2, t0, 7
    daddu   t2, t2, t8
    lb      t3, DLY_2X_OFFSET(t2)
    dadd    t3, 1
    sb      t3, DLY_2X_OFFSET(t2)
    dadd    t0, 1
    bleu    t0, 7, 1b
    nop
10:
#else

//tPHY_WRDATA training, to compare the data to decide the value of tPHY_WRDATA
#define     DDR4_TPHY_WRLAT_STORE    0x3300
    PRINTSTR("\r\nstart training of tPHY_WR\r\n")
tphy_wrlat_train_start:

    /*init dly_2x */
    dli     t0, 0
1:
    dsll    t1, t0, 7
    daddu   t1, t8
    lb      t2, DLY_2X_OFFSET(t1)
    andi    t2, 0x3c
    sb      t2, DLY_2X_OFFSET(t1)
    daddu   t0, 1
    bleu    t0, 8, 1b
    nop

    lb      t0, DDR4_CHANNEL_WIDTH_OFFSET(t8)
    li      t1, 1
    dsll    t1, t1, t0
    subu    t0, t1, 1

    sd      zero, DDR4_TPHY_WRLAT_STORE(t8)
    sd      zero, (DDR4_TPHY_WRLAT_STORE+8)(t8)

    li      k1, 0
    lb      t1, ECC_ENABLE_BIT(t8)
    beqz    t1, 1f
    nop
    move    k1, t1
    //disable ECC interrupt
    ld      a2, ECC_INT_ENABLE_ADDR(t8)
    dli     a1, 0x3
    dsll    a1, a1, ECC_INT_ENABLE_OFFSET
    not     a1
    and     a2, a2, a1
    sd      a2, ECC_INT_ENABLE_ADDR(t8)

    //enable ECC function but without reporting error
    ld      a2, ECC_ENABLE_ADDR(t8)
    dli     a1, 0x7
    dsll    a1, a1, ECC_ENABLE_OFFSET
    not     a1, a1
    and     a2, a2, a1
    sd      a2, ECC_ENABLE_ADDR(t8)
1:
    dli     t4, 0
    dli     t7, 0xff   //store min tphy_wrlat

tphy_wrlat_train:
#ifdef TPHY_WRLAT_DEBUG
    PRINTSTR("\r\n\r\nSlice No. ")
    move    a0, t4
    bal     hexserial
    nop
#endif
    dli     t3, 3
    lb      t2, DDR4_2T_OFFSET(t8)
    beqz    t2, 1f
    nop
    daddu   t3, t3, 1
1:
    sb      t3, DDR4_TPHY_WRLAT_OFFSET(t8)
    ld      t3, 0x0(t8)     //for sync
    ld      t3, 0x0(t8)     //for sync
    ld      t3, 0x0(t8)     //for sync
    ld      t3, 0x0(t8)     //for sync
    ld      t3, 0x0(t8)     //for sync
    sync
11:
    dli     t3, 0x0000f00000000000
	and	    t3, t3, t8//get node id
	dli     t1, 0xb800000000000000
	or	    t1, t1, t3

	dli     t2, 0x6666666666666666
	sd      t2, 0x0(t1)
	dli     t2, 0x5555555555555555
	sd      t2, 0x8(t1)
	dli     t2, 0x4444444444444444
	sd      t2, 0x10(t1)
	dli     t2, 0x3333333333333333
	sd      t2, 0x18(t1)
	dli     t2, 0x2222222222222222
	sd      t2, 0x20(t1)
	dli     t2, 0x1111111111111111
	sd      t2, 0x28(t1)
	dli     t2, 0x0000000000000000
	sd      t2, 0x30(t1)
	dli     t2, 0x8888888888888888
	sd      t2, 0x38(t1)

    sync
    ld      t3, 0x0(t8)     //for sync
    ld      t3, 0x0(t8)     //for sync
    ld      t3, 0x0(t8)     //for sync
    ld      t3, 0x0(t8)     //for sync
    ld      t3, 0x0(t8)     //for sync
#ifdef TPHY_WRLAT_DEBUG
    PRINTSTR("\r\nWrite_address=")
    dsrl    a0, t1, 32
    bal	    hexserial
    nop
    move    a0, t1
    bal     hexserial
    nop
#endif

	dli     t3, 0x00ffffffffffffff
	and	    t1, t1, t3
	dli     t3, 0x9000000000000000
    or      t1, t1, t3

#ifdef TPHY_WRLAT_DEBUG
    PRINTSTR("\r\nRead_out: ")
    ld      t2, 0x0(t1)
    dsrl    a0, t2, 32
    bal     hexserial
    nop
    move    a0, t2
    bal     hexserial
    nop
#endif

    dli     t2, 0xff
    dsll    t3, t4, 3
    ld      t5, 0x0(t1)
    sync
    dsll    t2, t2, t3
    and     t5, t5, t2
    dsrl    t5, t5, t3
#ifdef TPHY_WRLAT_DEBUG
    PRINTSTR("\r\nResult=")
    move    a0, t5
    bal     hexserial
    nop
#endif
    lb      t6, DDR4_TPHY_WRLAT_OFFSET(t8)
    dli     t2, 3
    lb      t3, DDR4_2T_OFFSET(t8)
    beqz    t3, 1f
    nop
    daddu   t2, t2, 1
1:
    ld      t3, 0x0(t8)     //for sync
    ld      t3, 0x0(t8)     //for sync
    ld      t3, 0x0(t8)     //for sync
    ld      t3, 0x0(t8)     //for sync
    ld      t3, 0x0(t8)     //for sync
    sync
    sb      t2, DDR4_TPHY_WRLAT_OFFSET(t8)
#if 0
    PRINTSTR("\r\n tPHY_WRLAT=")
    move    a0, t2
    bal     hexserial
    nop
    PRINTSTR("\r\n clear address=")
    dsrl    a0, t1, 32
    bal     hexserial
    nop
    move    a0, t1
    bal     hexserial
    nop
#endif

	dli     t3, 0x00ffffffffffffff
	and	    t1, t1, t3
	dli     t3, 0xb800000000000000
    or      t1, t1, t3

    dli     t3, 0xffffffffffffffff
    sd      t3, 0x0(t1)
    dli     t3, 0xffffffffffffffff
    sd      t3, 0x8(t1)
    dli     t3, 0xffffffffffffffff
    sd      t3, 0x10(t1)
    dli     t3, 0xffffffffffffffff
    sd      t3, 0x18(t1)
    dli     t3, 0xffffffffffffffff
    sd      t3, 0x20(t1)
    dli     t3, 0xffffffffffffffff
    sd      t3, 0x28(t1)
    dli     t3, 0xffffffffffffffff
    sd      t3, 0x30(t1)
    dli     t3, 0xffffffffffffffff
    sd      t3, 0x38(t1)
    sync
    ld      t3, 0x0(t8)     //for sync
    ld      t3, 0x0(t8)     //for sync
    ld      t3, 0x0(t8)     //for sync
    ld      t3, 0x0(t8)     //for sync
    ld      t3, 0x0(t8)     //for sync
    /*slot ddr need this code otherwise need the follow print*/
	dli     t3, 0x00ffffffffffffff
	and	    t1, t1, t3
	dli     t3, 0x9000000000000000
    or      t1, t1, t3
    ld      t3, 0x0(t1)     //for sync
    ld      t3, 0x0(t8)     //for sync
    ld      t3, 0x0(t8)     //for sync
    ld      t3, 0x0(t8)     //for sync
    ld      t3, 0x0(t8)     //for sync
    ld      t3, 0x0(t8)     //for sync
    PRINTSTR("")
#if 0
    PRINTSTR("\r\n 0x0:")
    ld      t3, 0x0(t1)
    dsrl    a0, t3, 32
    bal     hexserial
    nop
    move    a0, t3
    bal     hexserial
    nop
#endif
    daddu   t2, t2, 1
    bleu    t2, 0x10, 1b
    nop
    sb      t6, DDR4_TPHY_WRLAT_OFFSET(t8)
    dli     t6, 0x66
    beq     t5, t6, 2f
    nop
#if 0
    PRINTSTR("\r\nafter compare t1=")
    dsrl    a0, t1, 32
    bal	    hexserial
    nop
    move    a0, t1
    bal     hexserial
    nop
#endif

    lb      t5, DDR4_TPHY_WRLAT_OFFSET(t8)
#ifdef TPHY_WRLAT_DEBUG
    PRINTSTR("\r\n tPHY=")
    move    a0, t5
    bal     hexserial
    nop
#endif
    daddu   t5, t5, 1
    bleu    t5, 0x10, 1f
    nop
    PRINTSTR("\r\nError in tPHY_WR training")
    dli     t5, 4
    sb      t5, DDR4_TPHY_WRLAT_OFFSET(t8)
    b       2f
    nop
1:
    sb      t5, DDR4_TPHY_WRLAT_OFFSET(t8)
    ld      t3, 0x0(t8)     //for sync
    ld      t3, 0x0(t8)     //for sync
    ld      t3, 0x0(t8)     //for sync
    ld      t3, 0x0(t8)     //for sync
    ld      t3, 0x0(t8)     //for sync
    b       11b
    nop
2:
#ifdef TPHY_WRLAT_DEBUG
    PRINTSTR("\r\n tphy_wrlat=")
    lb      a0, DDR4_TPHY_WRLAT_OFFSET(t8)
    bal     hexserial
    nop
#endif
    dli     t2, 0xff
    dsll    t3, t4, 3
    dsll    t2, t2, t3
    not     t2, t2
    ld      t5, DDR4_TPHY_WRLAT_STORE(t8)
    and     t5, t5, t2
    lb      t6, DDR4_TPHY_WRLAT_OFFSET(t8)
    dsll    t6, t6, t3
    or      t5, t5, t6
    sd      t5, DDR4_TPHY_WRLAT_STORE(t8)
#if 0
    PRINTSTR("\r\n tphy_store=")
    dsrl    a0, t5, 32
    bal     hexserial
    nop
    move    a0, t5
    bal     hexserial
    nop
#endif
    lb      t6, DDR4_TPHY_WRLAT_OFFSET(t8)
    bgeu    t6, t7, 1f
    nop
    move    t7, t6
1:
    daddu   t4, t4, 1

    bleu    t4, t0, tphy_wrlat_train
    nop

#ifdef TPHY_WRLAT_DEBUG
    PRINTSTR("\r\n tphy_min=")
    move    a0, t7
    bal     hexserial
    nop
    PRINTSTR("\r\n tphy_store")
    ld	    t4, DDR4_TPHY_WRLAT_STORE(t8)
    dsrl    a0, t4, 32
    bal	    hexserial
    nop
    move    a0, t4
    bal     hexserial
    nop
#endif
//ecc train
    beqz    k1, ddr4_tphy_wrlat_set
    nop

#ifdef  CLK_FLY_BY_ORDER
    dli     a0, CLK_FLY_BY_ORDER
//first find ds8 position
    dli     t4, 8
1:
    dsll    t2, t4, 2
    dsrl    t3, a0, t2
    and     t3, t3, 0xf
    beq     t3, 8, 1f
    nop
    dsubu   t4, t4, 1
    bgez    t4, 1b
    nop
1:
    beqz    t4, 1f
    nop
//inc_order
    dsubu   t4, t4, 1
    dsll    t2, t4, 2
    dsrl    t2, a0, t2
    and     t2, t2, 0xf     //t2->next ds
    dsll    t5, t2, 7
    daddu   t5, t5, t8
    lb      t3, DDR4_DLL_WRDQ_OFFSET(t5)
    lb      t6, DDR4_DLL_WRDQ8_OFFSET(t8)
    daddu   t1, t8, t2
    lb      t2, DDR4_TPHY_WRLAT_STORE(t1)
    b       ecc_inc_order
    nop

1:
    dli     t4, 1
    dsll    t2, t4, 2
    dsrl    t2, a0, t2
    and     t2, t2, 0xf     //t2->next ds
    dsll    t5, t2, 7
    daddu   t5, t5, t8
    lb      t6, DDR4_DLL_WRDQ_OFFSET(t5)
    lb      t3, DDR4_DLL_WRDQ8_OFFSET(t8)
    daddu   t1, t8, t2
    lb      t2, DDR4_TPHY_WRLAT_STORE(t1)
    b       ecc_dec_order
    nop

#else
    GET_DIMM_TYPE_V1
    andi    a1, a1, 1
    beqz    a1, udimm_ecc_train
    nop
//rdimm ecc train
//first compare ecc
    lb      t6, DDR4_DLL_WRDQ8_OFFSET(t8)
    lb      t3, DDR4_DLL_WRDQ3_OFFSET(t8)
    lb      t2, (DDR4_TPHY_WRLAT_STORE + 3)(t8)
    b       ecc_inc_order
    nop

udimm_ecc_train:
    GET_SPD(DDR4_SPD_REFERENCE_RAW_CARDS_OFFSET)
    dsrl    t1, v0, 7
    andi    t1, t1, 1
    bnez    t1, raw_cards_error
    nop

    and     t1, v0, 0x1f    //t1->raw cards No.
//check dimm type for compare raw cards
    GET_DIMM_TYPE_V1
    bnez    a1, 1f
    nop

//now support udimm D0/1/2 raw cards
    beq     t1, 3, order0
    nop
    b       raw_cards_error
    nop

//now support sodimm D0/1,F0,G0/1,H0 raw cards
1:
    beq     t1, 5, order0
    nop
    beq     t1, 6, order0
    nop
    beq     t1, 7, order0
    nop
    beq     t1, 3, 1f
    nop
    b       raw_cards_error
    nop
1:
    dsrl    t1, v0, 5
    andi    t1, t1, 0x3
    beqz    t1, order0
    nop
    beq     t1, 1, order1
    nop
    b       raw_cards_error
    nop

order0:     //clk fly by order is 38
    lb      t6, DDR4_DLL_WRDQ3_OFFSET(t8)
    lb      t2, (DDR4_TPHY_WRLAT_STORE + 3)(t8)
    b       1f
    nop

order1:     //clk fly by order is 28
    lb      t6, DDR4_DLL_WRDQ2_OFFSET(t8)
    lb      t2, (DDR4_TPHY_WRLAT_STORE + 2)(t8)
1:
    lb      t3, DDR4_DLL_WRDQ8_OFFSET(t8)

    b       ecc_dec_order
    nop

raw_cards_error:
    PRINTSTR("\r\nError: not supported DIMM raw card, pleas use manule defined order")
1:
    b       1b
    nop
#endif
ecc_dec_order:
    bgeu    t3, t6, 3f      //cur dll_wrdq > last dll_wrdq
    nop
    dli     t1, 0x50
    bltu    t1, t6, 2f
    nop
    dsubu   t1, t6, t3
    bleu    t1, 0x10, 3f
    nop
2:
    dli     t1, 0x3e
    bltu    t3, t1, 2f
    nop
    dsubu   t1, t6, t3
    bleu    t1, 0x10, 3f
    nop
2:
    addu    t2, t2, 1
3:
    sb      t2, (DDR4_TPHY_WRLAT_STORE + 8)(t8)

    b       exit_ecc_tphy_wrlat_train
    nop

ecc_inc_order:
    bgeu    t3, t6, 3f      //cur dll_wrdq > last dll_wrdq
    nop
    dli     t1, 0x50
    bltu    t1, t6, 2f
    nop
    dsubu   t1, t6, t3
    bleu    t1, 0x10, 3f
    nop
2:
    dli     t1, 0x3e
    bltu    t3, t1, 2f
    nop
    dsubu   t1, t6, t3
    bleu    t1, 0x10, 3f
    nop
2:
    subu    t2, t2, 1
//compare min with ecc tphy wrlat
    bgeu    t2, t7, 3f
    nop
    bgeu    t2, 3, 1f   //3 is supported min of tphy wrlat
    nop
    daddu   t2, t2, 1
    dli     t1, 0x0101010101010101
    ld      t3, DDR4_TPHY_WRLAT_STORE(t8)
    daddu   t3, t3, t1
    sd      t3, DDR4_TPHY_WRLAT_STORE(t8)
1:
    move    t7, t2
3:
    sb      t2, (DDR4_TPHY_WRLAT_STORE + 8)(t8)

exit_ecc_tphy_wrlat_train:
    //enable ECC function with reporting error
    ld      a2, ECC_ENABLE_ADDR(t8)
    dli     a1, 0x7
    dsll    a1, a1, ECC_ENABLE_OFFSET
    not     a1, a1
    and     a2, a2, a1
    dli     a1, 0x7
    dsll    a1, a1, ECC_ENABLE_OFFSET
    or      a2, a2, a1
    sd      a2, ECC_ENABLE_ADDR(t8)

    //enable ECC interrupt
    ld      a2, ECC_INT_ENABLE_ADDR(t8)
    dli     a1, 0x3
    dsll    a1, a1, ECC_INT_ENABLE_OFFSET
    or      a2, a2, a1
    sd      a2, ECC_INT_ENABLE_ADDR(t8)

ddr4_tphy_wrlat_set:
    lb      t4, ECC_ENABLE_BIT(t8)
    beqz    t4, 1f
    nop
    daddu   t0, t0, 1
1:

    sb      t7, DDR4_TPHY_WRLAT_OFFSET(t8)
    dli     t4, 0
1:
    dsll    t3, t4, 7
    daddu   t3, t3, t8
    daddu   t2, t8, t4
    lb      t5, DDR4_TPHY_WRLAT_STORE(t2)
    subu    t5, t5, t7
    andi    t5, t5, 0x3
    lb      t2, DLY_2X_OFFSET(t3)
    andi    t2, t2, 0x3c
    or      t2, t2, t5
    sb      t2, DLY_2X_OFFSET(t3)
    daddu   t4, t4, 1
    bleu    t4, t0, 1b
    nop

    dli     t3, 0x0000f00000000000
	and	    t3, t3, t8//get node id
	dli     t1, 0xb800000000000000
	or	    t1, t1, t3
	dli     t2, 0x5555555555555555
	sd      t2, 0x0(t1)
	dli     t2, 0xaaaaaaaaaaaaaaaa
	sd      t2, 0x8(t1)
	dli     t2, 0x3333333333333333
	sd      t2, 0x10(t1)
	dli     t2, 0xcccccccccccccccc
	sd      t2, 0x18(t1)
	dli     t2, 0x7777777777777777
	sd      t2, 0x20(t1)
	dli     t2, 0x8888888888888888
	sd      t2, 0x28(t1)
	dli     t2, 0x1111111111111111
	sd      t2, 0x30(t1)
	dli     t2, 0xeeeeeeeeeeeeeeee
	sd      t2, 0x38(t1)

	dli     t3, 0x00ffffffffffffff
	and	    t1, t1, t3
	dli     t3, 0x9000000000000000
    or      t1, t1, t3

	dli     t2, 0x5555555555555555
    ld      t3, 0x0(t1)
    bne     t2, t3, tphy_wrlat_train_start
    nop
	dli     t2, 0xaaaaaaaaaaaaaaaa
    ld      t3, 0x8(t1)
    bne     t2, t3, tphy_wrlat_train_start
    nop
	dli     t2, 0x3333333333333333
    ld      t3, 0x10(t1)
    bne     t2, t3, tphy_wrlat_train_start
    nop
	dli     t2, 0xcccccccccccccccc
    ld      t3, 0x18(t1)
    bne     t2, t3, tphy_wrlat_train_start
    nop
	dli     t2, 0x7777777777777777
    ld      t3, 0x20(t1)
    bne     t2, t3, tphy_wrlat_train_start
    nop
	dli     t2, 0x8888888888888888
    ld      t3, 0x28(t1)
    bne     t2, t3, tphy_wrlat_train_start
    nop
	dli     t2, 0x1111111111111111
    ld      t3, 0x30(t1)
    bne     t2, t3, tphy_wrlat_train_start
    nop
	dli     t2, 0xeeeeeeeeeeeeeeee
    ld      t3, 0x38(t1)
    bne     t2, t3, tphy_wrlat_train_start
    nop

    PRINTSTR("\r\ntPHY_WRLAT training success")
#ifdef TPHY_WRLAT_DEBUG
    PRINTSTR("\r\n tphy_store")
    ld	    t4, DDR4_TPHY_WRLAT_STORE(t8)
    dsrl    a0, t4, 32
    bal	    hexserial
    nop
    move    a0, t4
    bal     hexserial
    nop
    PRINTSTR("\r\n")
#endif
#endif
#endif

#ifndef DDR3_DIMM
#ifdef VREF_STORE
/* t1:save flash base; t2:save SPD info; t3:save sdram base; */
    GET_NODE_ID_a1
    mul     a1, DIMM_INFO_SIZE
    dli     t3, DIMM_INFO_IN_CACHE_OFFS
    daddu   t3, a1
    dli     t1, (0xffffffff00000000 | DIMM_INFO_IN_FLASH_OFFS)
    daddu   t1, a1

/*first init, write freq into mc0 info to mark it has been done*/
    li      a2, DDR_FREQ
    lw      a0, DIMM_OFFS_CLK(t3)
    /*If it's not initialized, it won't be equal*/
    beq     a2, a0, 2f
    nop

    GET_NODE_ID_a1
    bnez     a1, 2f
    nop

    sw      a2, DIMM_OFFS_CLK(t3)
    daddiu  a1, t3, DIMM_OFFS_SLOT0_SPD
    sd      zero, 0x0(a1)
    sd      zero, 0x8(a1)
    daddiu  a1, a1, MC_INFO_SIZE
    sd      zero, 0x0(a1)
    sd      zero, 0x8(a1)

#ifdef	MULTI_CHIP
    daddiu  a1, a1, MC_INFO_SIZE
    sd      zero, 0x0(a1)
    sd      zero, 0x8(a1)
    daddiu  a1, a1, MC_INFO_SIZE
    sd      zero, 0x0(a1)
    sd      zero, 0x8(a1)
#ifdef CHIP_4
    daddiu  a1, a1, MC_INFO_SIZE
    sd      zero, 0x0(a1)
    sd      zero, 0x8(a1)
    daddiu  a1, a1, MC_INFO_SIZE
    sd      zero, 0x0(a1)
    sd      zero, 0x8(a1)
    daddiu  a1, a1, MC_INFO_SIZE
    sd      zero, 0x0(a1)
    sd      zero, 0x8(a1)
    daddiu  a1, a1, MC_INFO_SIZE
    sd      zero, 0x0(a1)
    sd      zero, 0x8(a1)
#endif
#endif
2:
    mul     a0, k0, MC_INFO_SIZE	//if k0==1, add mc1 offset
    daddu   t3, a0
    daddu   t1, a0

#ifdef  AUTO_DDR_CONFIG
    /* read SPD info */
    SPD_REGS_INFO(t2,0);
    SPD_REGS_INFO(t4,1);
#endif

    /*check DDR freq */
    li      a1, DDR_FREQ
    sw      a1, DIMM_OFFS_CLK(t3)
    lw      a0, DIMM_OFFS_CLK(t1)
    bne     a1, a0, 4f
    nop

#ifdef  AUTO_DDR_CONFIG
    /*mc0:k0=0; mc1:k0=1*/
    /* check slot0 SPD info */
    ld      a0, DIMM_OFFS_SLOT0_SPD(t1)
    bne     t2, a0, 4f
    nop
    /* check slot1 SPD info */
    ld      a0, DIMM_OFFS_SLOT1_SPD(t1)
    bne     t4, a0, 4f
    nop
#endif

    b       bit_training_end
    nop

4:
#ifdef  AUTO_DDR_CONFIG
    /* save SPD info */
    sd      t2, DIMM_OFFS_SLOT0_SPD(t3)
    sd      t4, DIMM_OFFS_SLOT1_SPD(t3)
#else
    /* SPD information is set to 1 to judge whether flash is burned or not */
    li      t2, 0x1
    sd      t2, DIMM_OFFS_SLOT0_SPD(t3)
    li      t4, 0x1
    sd      t4, DIMM_OFFS_SLOT1_SPD(t3)

#endif
5:
#endif
#ifdef ENABLE_DDR_VREF_TRAINING
    bal     ddr_vref_training
    nop
#endif
#ifdef ENABLE_BIT_TRAINING
    bal	    rd_bit_training
    nop
    bal	    wr_bit_training
    nop
#endif
#ifdef VREF_STORE
//store training result
    GET_NODE_ID_a1
    mul     a1, DIMM_INFO_SIZE
    dli     t3, DIMM_INFO_IN_CACHE_OFFS
    daddu   t3, a1
    mul     a1, k0, MC_INFO_SIZE
    daddu   t3, a1
    GET_NODE_ID_a0;
    dli     t8, DDR_MC_CONFIG_BASE
    or      t8, t8, a0
    dli     t0, 0
1:
    dsll    t1, t0, 7
    daddu   t4, t8, t1

    lb      t1, DLL_WRDQS_OFFSET(t4)
    sb      t1, DIMM_OFFS_BIT_TRAIN(t3)
    daddu   t3, 2
    lh      t1, DLL_RDDQS0_OFFSET(t4)
    sh      t1, DIMM_OFFS_BIT_TRAIN(t3)
    daddu   t3, 2
    ld      t1, WRDQ_BDLY00_OFFSET(t4)
    sd      t1, DIMM_OFFS_BIT_TRAIN(t3)
    daddu   t3, 8
    ld      t1, RDQSP_BDLY00_OFFSET(t4)
    sd      t1, DIMM_OFFS_BIT_TRAIN(t3)
    daddu   t3, 8
    ld      t1, RDQSN_BDLY00_OFFSET(t4)
    sd      t1, DIMM_OFFS_BIT_TRAIN(t3)
    daddu   t3, 8

    daddu   t0, 1
    bleu    t0, 8, 1b
    nop

bit_training_end:
#endif
#endif

//#define DLL_BYPASS
#ifdef DLL_BYPASS
    li      t2, 0x8//slice num
    dli     a0, 0x900000000ff00000
    lbu     t0, 0x36(a0)

1:
//dll_wrdq
    lbu     t1, 0x100(a0)
    dmulou  t1, t1, t0
    dsrl    t1, t1, 0x7
    ori     t1, t1, 0x80
    sb      t1, 0x100(a0)

//dll_wrdqs
    lbu     t1, 0x101(a0)
    dmulou  t1, t1, t0
    dsrl    t1, t1, 0x7
    ori     t1, t1, 0x80
    sb      t1, 0x101(a0)

//dll_clk1x
    lbu     t1, 0x103(a0)
    dmulou  t1, t1, t0
    dsrl    t1, t1, 0x7
    ori     t1, t1, 0x80
    sb      t1, 0x103(a0)

//dll_rddqs0
    lbu     t1, 0x108(a0)
    dmulou  t1, t1, t0
    dsrl    t1, t1, 0x7
    ori     t1, t1, 0x80
    sb      t1, 0x108(a0)

//dll_rddqs1
    lbu     t1, 0x109(a0)
    dmulou  t1, t1, t0
    dsrl    t1, t1, 0x7
    ori     t1, t1, 0x80
    sb      t1, 0x109(a0)

//dll_gate
    lbu     t1, 0x10a(a0)
    dmulou  t1, t1, t0
    dsrl    t1, t1, 0x7
    ori     t1, t1, 0x80
    sb      t1, 0x10a(a0)

    daddiu  a0, 0x80
    subu    t2, 0x1
    bnez    t2, 1b
    nop

#endif
#if 1 //def  PRINT_DDR_LEVELING   //print registers
    PRINTSTR("The MC param after leveling is:\r\n")
    PRINTSTR("PHY:\r\n")
    dli     t1, 290
    GET_NODE_ID_a0
    dli     t5, DDR_MC_CONFIG_BASE
    or      t5, t5, a0
1:
    ld      t6, 0x0(t5)
    move    a0, t5
    and     a0, a0, 0xffff
    bal     hexserial
    nop
    PRINTSTR(":  ")
    dsrl    a0, t6, 32
    bal     hexserial
    nop
    //PRINTSTR("  ")
    move    a0, t6
    bal     hexserial
    nop
    PRINTSTR("\r\n")

    daddiu  t1, t1, -1
    daddiu  t5, t5, 8
    bnez    t1, 1b
    nop

    PRINTSTR("\r\nCTRL:\r\n")
    dli     t1, 231
    GET_NODE_ID_a0
    dli     t5, DDR_MC_CONFIG_BASE
    or      t5, t5, a0
    ori     t5, 0x1000

1:
    ld      t6, 0x0(t5)
    move    a0, t5
    and     a0, a0, 0xffff
    bal     hexserial
    nop
    PRINTSTR(":  ")
    dsrl    a0, t6, 32
    bal     hexserial
    nop
    //PRINTSTR("  ")
    move    a0, t6
    bal     hexserial
    nop
    PRINTSTR("\r\n")

    daddiu  t1, t1, -1
    daddiu  t5, t5, 8
    bnez    t1, 1b
    nop
#endif

    move    t3, k0

    //Re-set t0&t2 because mc_init(ddr3_leveling) will change t0~t9
    GET_NODE_ID_a0
    dli     t2, 0x900000001fe00180
    dli     t0, 0x900000003ff02000 #3A4000 win base change to 0x2000
    or      t2, t2, a0
    or      t0, t0, a0

#ifdef  PRINT_DDR_LEVELING   //print registers
    PRINTSTR("The MC param after leveling is:\r\n")
    PRINTSTR("PHY:\r\n")
    dli     t1, 290
    GET_NODE_ID_a0
    dli     t5, DDR_MC_CONFIG_BASE
    or      t5, t5, a0
1:
    ld      t6, 0x0(t5)
    move    a0, t5
    and     a0, a0, 0xffff
    bal     hexserial
    nop
    PRINTSTR(":  ")
    dsrl    a0, t6, 32
    bal     hexserial
    nop
    //PRINTSTR("  ")
    move    a0, t6
    bal     hexserial
    nop
    PRINTSTR("\r\n")

    daddiu  t1, t1, -1
    daddiu  t5, t5, 8
    bnez    t1, 1b
    nop

    PRINTSTR("\r\nCTRL:\r\n")
    dli     t1, 231
    GET_NODE_ID_a0
    dli     t5, DDR_MC_CONFIG_BASE
    or      t5, t5, a0
    ori     t5, 0x1000

1:
    ld      t6, 0x0(t5)
    move    a0, t5
    and     a0, a0, 0xffff
    bal     hexserial
    nop
    PRINTSTR(":  ")
    dsrl    a0, t6, 32
    bal     hexserial
    nop
    //PRINTSTR("  ")
    move    a0, t6
    bal     hexserial
    nop
    PRINTSTR("\r\n")

    daddiu  t1, t1, -1
    daddiu  t5, t5, 8
    bnez    t1, 1b
    nop
#endif
#ifdef  DEBUG_DDR_PARAM   //Change parameters of MC
    GET_NODE_ID_a0;
    dli     a1, DDR_MC_CONFIG_BASE
    or      t8, a0, a1

    PRINTSTR("\r\nChange some parameters of MC:");
1:
    PRINTSTR("\r\nPlease input the register number you want to change!!!(0xffff:jump out.): ");
    dli     t6, 0x00
    bal     inputaddress
    nop
    move    t5, v0

    dli     a1, 0xffff
    bge     t5, a1, 2f    #if input address offset exceed range,jump out
    nop
    and     t5, t5, 0xfff8
    daddu   t5, t5, t8

    PRINTSTR("\r\nPlease input the data-hex: ");
    dli     t6, 0x00
    bal     inputaddress
    nop
    sd      v0, 0x0(t5)    #v0 is the input value

    //print the new register value
    move    t6, t5
    PRINTSTR("\r\nRegister 0x")
    dsubu   t5, t5, t8
    move    a0, t5
    bal     hexserial
    nop
    PRINTSTR(": ")
    ld      t6, 0x0(t6)
    dsrl    a0, t6, 32
    bal     hexserial
    nop
    move    a0, t6
    bal     hexserial
    nop

    b        1b
    nop
2:
#endif

    //TTYDBG("Disable register space of MEMORY\r\n")
    move    a1, k0
    bal     disable_mc_conf_space
    nop
88:
#endif

#ifndef DISABLE_DRAM_CRC
#define CMD_PREALL_OFFSET 0x1124
#define CMD_PREDONE_OFFSET 0x1125
#define CMD_PDA_OFFSET    0x1130
#define CMD_DQ0_OFFSET    0x1138

    PRINTSTR("\r\nEnable CRC")

    GET_NODE_ID_a0
    dli     t2, 0x900000001fe00180
    or      t2, t2, a0
    move    a1, k0
    bal     enable_mc_conf_space
    nop

    GET_NODE_ID_a0;
    dli     t8, DDR_MC_CONFIG_BASE
    or      t8, t8, a0
    ld      t1, 0x1450(t8)
    dli     a1, 0xff
    not     a1
    and     t1, a1
    dli     a1, 0x1
    or      t1, a1
    sd      t1, 0x1450(t8)

    ld      t1, 0x1458(t8)
    dli     a1, 0xff
    not     a1
    and     t1, a1
    dli     a1, 0x28
    or      t1, a1
    sd      t1, 0x1458(t8)

    ld      t1, 0x1280(t8)
    dli     a1, 0x1
    dsll    a1, 4
    not     a1
    and     t1, a1
    dli     a1, 0x1
    dsll    a1, 4
    or      t1, a1
    sd      t1, 0x1280(t8)

    dli     t1, 0x100
    sh      t1, 0x1480(t8)

    /*get cs number in t1 */
    GET_CS_NUM_DDR4
    move    t1, v0

    /*mrs send for different cs loop*/
    li      t0, 0       //cs number ctrl
1:
    dsll    t3, t0, 4
    daddu   t3, t8
    /*enable crc mode*/
    lhu     mrs_cmd_a, DDR4_MR2_CS0_REG(t3)
    or      mrs_cmd_a, (1<<12)
    move    mrs_cs, t0
    li      mrs_num, 2
    MRS_SEND(mrs_cmd_a,mrs_cs,mrs_num)

    /*change the write latency in mr3*/
    GET_SDRAM_WIDTH_V1
    beqz    a1, 4f      //if x4 or DM disable, don't change
    nop
	dli    	t2, DDR_FREQ
    dsll    t2, 2
    bleu    t2, 1600, 1f
    nop
    bleu    t2, 2666, 2f
    nop
    dli     t2, 0x2
    b       3f
    nop
1:
    dli     t2, 0x0
    b       3f
    nop
2:
    dli     t2, 0x1
3:
    lhu     mrs_cmd_a, DDR4_MR3_CS0_REG(t3)
    and     mrs_cmd_a, ~(0x3<<9)
    dsll    t2, 9
    or      mrs_cmd_a, t2
    move    mrs_cs, t0
    li      mrs_num, 3
    MRS_SEND(mrs_cmd_a,mrs_cs,mrs_num)
4:
    /*change CCD_L in DDR and MC*/
    lbu     t2, 0x1050(t8)
    bne     t2, 4, 1f
    nop
    daddu   t2, 1
    sb      t2, 0x1050(t8)
1:
    dsubu   t2, 4
    dsll    t2, 10
    lhu     mrs_cmd_a, DDR4_MR6_CS0_REG(t3)
    and     mrs_cmd_a, ~(0x7<<10)
    or      mrs_cmd_a, t2
    move    mrs_cs, t0
    li      mrs_num, 6
    MRS_SEND(mrs_cmd_a,mrs_cs,mrs_num)

    /*mrs send for different cs loop ctrl */
    daddu   t0, 1
    bltu    t0, t1, 1b
    nop

//set bitmap according to SPD reg
    dli     t1, 0x1460
    or      t1, t8
    dli     t0, 0x3c
1:
    GET_SPD(t0)
    andi    v0, 0x3f
    sb      v0, 0x0(t1)
    daddu   t1, 0x1
    daddu   t0, 0x1
    bleu    t0, 0x43, 1b
    nop

    dli     t1, 0x1468
    or      t1, t8
    dli     t0, 0x46
1:
    GET_SPD(t0)
    andi    v0, 0x3f
    sb      v0, 0x0(t1)
    daddu   t1, 0x1
    daddu   t0, 0x1
    bleu    t0, 0x4d, 1b
    nop

    dli     t1, 0x1470
    or      t1, t8
    dli     t0, 0x44
1:
    GET_SPD(t0)
    andi    v0, 0x3f
    sb      v0, 0x0(t1)
    daddu   t1, 0x1
    daddu   t0, 0x1
    bleu    t0, 0x45, 1b
    nop

    dli     t1, 0xaa
    sb      t1, 0x1478(t8)

//change CCD_S in MC
    lbu     t0, 0x1051(t8)
    bne     t0, 4, 1f
    nop
    daddu   t0, 1
    sb      t0, 0x1051(t8)
1:
//add 1nCK in odt_wr_length
    lbu     t1, 0x13c8(t8)
    daddu   t1, 1
    sb      t1, 0x13c8(t8)
    lbu     t1, 0x13c9(t8)
    daddu   t1, 1
    sb      t1, 0x13c9(t8)

    PRINTSTR("\r\nEnable CRC done\r\n")
    move    a1, k0
    bal     disable_mc_conf_space
    nop
#endif

#ifndef  DISABLE_DIMM_ECC
    //Init ECC according to DIMM ECC info
    GET_DIMM_ECC_V1
    beqz    a1, 4f
    nop
    TTYDBG("ECC init start(maybe take 1 minute or so)....\r\n")

    //TTYDBG("Enable register space of MEMORY\r\n")
    GET_NODE_ID_a0
    dli     t2, 0x900000001fe00180
    or      t2, t2, a0
    move    a1, k0
    bal     enable_mc_conf_space
    nop

    GET_NODE_ID_a0;
    dli     t8, DDR_MC_CONFIG_BASE
    or      t8, t8, a0

    //disable ECC interrupt
    ld      a2, ECC_INT_ENABLE_ADDR(t8)
    dli     a1, 0x3
    dsll    a1, a1, ECC_INT_ENABLE_OFFSET
    not     a1
    and     a2, a2, a1
    sd      a2, ECC_INT_ENABLE_ADDR(t8)

    //enable ECC function but without reporting error
    ld      a2, ECC_ENABLE_ADDR(t8)
    dli     a1, 0x7
    dsll    a1, a1, ECC_ENABLE_OFFSET
    not     a1, a1
    and     a2, a2, a1
    dli     a1, 0x1
    dsll    a1, a1, ECC_ENABLE_OFFSET
    or      a2, a2, a1
    sd      a2, ECC_ENABLE_ADDR(t8)

	//add node_id on default winbase
    ld      t1, 0x1500(t8)
    or      t1, t1, a0
    sd      t1, 0x1500(t8)
    ld      t1, 0x1508(t8)
    or      t1, t1, a0
    sd      t1, 0x1508(t8)
    ld      t1, 0x1510(t8)
    or      t1, t1, a0
    sd      t1, 0x1510(t8)

    //TTYDBG("Disable register space of MEMORY\r\n")
    move    a1, k0
    bal     disable_mc_conf_space
    nop
    move    a1, k0
    bal     disable_mc_regs_default
    nop

#ifdef MCC
    dli     t1, 0x900000001fe01514
1:
    GET_NODE_ID_a0
    or      t1, t1, a0
    dli     t0, 1
    sw      t0, 0(t1)
#endif

    //use 4T/5T addr for ECC init
    sync
    nop
    nop
    nop
    nop
//init mem to all 0
    dli     t1, 0x9800040000000000
    beqz    t3, 1f
    nop
    dli     t1, 0x9800050000000000
1:
    GET_NODE_ID_a0
    or      t1, t1, a0
	GET_DIMM_MEMSIZE_V1
1:
    dsll    a1, a1, 30   //a1*1G
    daddu   t5, t1, a1
//write memory
1:
    bgeu    t1, t5, 1f
    nop

    sd      $0, 0x0(t1)
    sd      $0, 0x8(t1)
    sd      $0, 0x10(t1)
    sd      $0, 0x18(t1)
    sd      $0, 0x20(t1)
    sd      $0, 0x28(t1)
    sd      $0, 0x30(t1)
    sd      $0, 0x38(t1)
    daddu   t1, t1, 0x40
    b       1b
    nop
1:
#if 0
#use load to flush out ucacc queue
    dli     t1, 0x9000000080000000
    GET_NODE_ID_a0
    or      t1, t1, a0
    daddu   t4, t1, t4
    daddu   t5, -0x10000
1:
    bgeu    t5, t4, 1f
    nop
    ld      t1, 0x00(t5)
    ld      t1, 0x08(t5)
    ld      t1, 0x10(t5)
    ld      t1, 0x18(t5)
    ld      t1, 0x20(t5)
    ld      t1, 0x28(t5)
    ld      t1, 0x30(t5)
    ld      t1, 0x38(t5)
    daddu   t5, t5, 0x40
    b       1b
    nop
1:
#endif
//  flush out L3 cache using addtional 32MB load.
    dli     t1, 0x9800040000000000
    beqz    t3, 1f
    nop
    dli     t1, 0x9800050000000000
1:
    GET_NODE_ID_a0
    or      t1, t1, a0
    li      a1, 0x2000000
    daddu   t5, t1, a1

//flush out  memory
1:
	ld		$0, 0x0(t1)
	ld		$0, 0x40(t1)
	ld		$0, 0x80(t1)
	ld		$0, 0xc0(t1)
	daddiu	t1, t1, 0x100
	bltu	t1, t5, 1b
	nop

#ifdef MCC
    dli     t1, 0x900000001fe01514
1:
    GET_NODE_ID_a0
    or      t1, t1, a0
    sw      zero, 0(t1)
#endif

    //TTYDBG("Enable register space of MEMORY\r\n")
    move    a1, k0
    bal     enable_mc_conf_space
    nop

    //enable ECC function with reporting error
    ld      a2, ECC_ENABLE_ADDR(t8)
    dli     a1, 0x7
    dsll    a1, a1, ECC_ENABLE_OFFSET
    not     a1, a1
    and     a2, a2, a1
    dli     a1, 0x7
    dsll    a1, a1, ECC_ENABLE_OFFSET
    or      a2, a2, a1
    sd      a2, ECC_ENABLE_ADDR(t8)

    //enable ECC interrupt
    ld      a2, ECC_INT_ENABLE_ADDR(t8)
    dli     a1, 0x3
    dsll    a1, a1, ECC_INT_ENABLE_OFFSET
    or      a2, a2, a1
    sd      a2, ECC_INT_ENABLE_ADDR(t8)

    //PRINTSTR("\r\nDisable register space of MEMORY\r\n")
    move    a1, k0
    bal     disable_mc_conf_space
    nop

    TTYDBG("MC ECC init done.\r\n")
4:
#endif

    move    ra, s4
    jr      ra
    nop
    .end    mc_init

//for 3A4000
#[5:4]=reg_default, reg_disable,
#[10:9]=reg_default, reg_disable,
LEAF(enable_mc_conf_space)
/*********************
reg used v0, v1, a1
input: a1 - MC select
*********************/
	sync
    /* MC0 use bit[4], MC1 use bit[9] */
    dli     v1, 0x1
    dsll    v1, a1
    dsll    v1, 4
    dsll    a1, 2
    dsll    v1, a1
    not     v1
    /* chip configure register address */
    GET_NODE_ID_a1
    dsll    a1, 44
    or      a1, 0x900000003ff00180
    lw      v0, 0x0(a1)
    and     v1, v0
    sw      v1, 0x0(a1)
    sync

    jr      ra
    nop
END(enable_mc_conf_space)

LEAF(disable_mc_conf_space)
/*********************
reg used v0, v1, a1
input: a1 - MC select
*********************/
	sync
    /* MC0 use bit[4], MC1 use bit[9] */
    dli     v1, 0x1
    dsll    v1, a1
    dsll    v1, 4
    dsll    a1, 2
    dsll    v1, a1
    /* chip configure register address */
    GET_NODE_ID_a1
    dsll    a1, 44
    or      a1, 0x900000003ff00180
    lw      v0, 0x0(a1)
    or      v1, v0
    sw      v1, 0x0(a1)
	sync

	jr      ra
	nop
END(disable_mc_conf_space)

LEAF(enable_mc_regs_default)
/*********************
reg used v0, v1, a1
input: a1 - MC select
*********************/
    sync
    /* MC0 use bit[5], MC1 use bit[10] */
    dli     v1, 0x1
    dsll    v1, a1
    dsll    v1, 5
    dsll    a1, 2
    dsll    v1, a1
    /* chip configure register address */
    GET_NODE_ID_a1
    dsll    a1, 44
    or      a1, 0x900000003ff00180
    lw      v0, 0x0(a1)
    or      v1, v0
    sw      v1, 0x0(a1)
	sync

	jr      ra
	nop
END(enable_mc_regs_default)

LEAF(disable_mc_regs_default)
/*********************
reg used v0, v1, a1
input: a1 - MC select
*********************/
    sync
    /* MC0 use bit[5], MC1 use bit[10] */
    dli     v1, 0x1
    dsll    v1, a1
    dsll    v1, 5
    dsll    a1, 2
    dsll    v1, a1
    not     v1
    /* chip configure register address */
    GET_NODE_ID_a1
    dsll    a1, 44
    or      a1, 0x900000003ff00180
    lw      v0, 0x0(a1)
    and     v1, v0
    sw      v1, 0x0(a1)
	sync

	jr      ra
	nop
END(disable_mc_regs_default)

LEAF(enable_mc_read_buffer)

    jr      ra
    nop
END(enable_mc_read_buffer)

LEAF(disable_mc_read_buffer)

    jr      ra
    nop
END(disable_mc_read_buffer)

LEAF(disable_cpu_buffer_read)

    jr      ra
    nop
END(disable_cpu_buffer_read)

LEAF(winconf_mc_one)
##############CONFIGURE WINDOWS FOR ONE MC###################
#memory size a1x512MB, window doubled every loop 2G->4G->8G...
#a1: memory size
winconf_mc_one:

    //cpu 0 need 0~0x10000000 window
    dli     t0, 0x0
    dli     t1, 0x0
    dli     t2, ~0xfffffff
    dli     t3, 0x80
    MC_WIN_CONFIG_NODE(t0, t1, t2, t3)

    dli     t0, 1 #index
    dli     t1, DDR_WIN_BASE_ADDR #start phy addr
    #t2, mask, compute later
    dli     t3, 0x80 #start mc addr with win en
    dli     t4, (DDR_WIN_BASE_ADDR >> 30)   #base win size, doubled every iteration
    move    t6, a1

winconf_mc_one_loop:
    beqz    t6, winconf_mc_one_finished
    nop
    bgeu    t6, t4, winconf_mc_one_full_window
    nop
winconf_mc_one_part_window: #last window
    dsll    t2, t6, 30
    move    t3, t2
    ori     t3, 0x80
    dsubu   t2, t2, 1
    not     t2, t2
    move    t6, zero
    b       1f
    nop
winconf_mc_one_full_window:
    dsll    t2, t4, 30
    dsubu   t2, t2, 1
    not     t2, t2
    dsubu   t6, t6, t4

1:
    MC_WIN_CONFIG_NODE(t0, t1, t2, t3)
    beqz   t6, winconf_mc_one_finished
    nop
    #inc base,mmap
    dsll    t5, t4, 30 #512M
    daddu   t1, t1, t5
    move    t3, t1
    ori     t3, 0x80

    dsll    t4, t4, 1 #double window size

    daddiu  t0, t0, 1
    b       winconf_mc_one_loop
    nop
winconf_mc_one_finished:

    jr      ra
    nop
END(winconf_mc_one)

LEAF(get_mem_clk)
/*********************
    v0: current ddr freq setting
*********************/
    li t0,0xbfe00190
    ld t1,0x0(t0)
    dsrl t2,t1,37
    andi t2,t2,0x0000001f
    move v0,t2

    nop
    jr ra
    nop
END(get_mem_clk)

#ifdef VDDP_CTR
LEAF(v_p_ctrl_in_mc)
/*
 * v0 give the voltage level
 * use register:
 * a0,a1,a2,t3, t0
 * t0: input ( the value of vddp )
 * a0: device ID
 * a1: command
 * a2: command value
 * t3: save ra
 * v0: return value
 */
	.set    push
	.set    noreorder
	.set    mips3
	move t3, ra

#ifdef PV3205_V
	dli a3, LS7A_I2C1_REG_BASE
	bal ls_v_i2cinit
	nop

	li  a0, PV3205_VDDP
	li  a1, 0x21 //VOUT_COMMAND
	//li  a2, 0x1800 //1.2v
	li  a2, 0x1600 //1.1v

	bal ls_v_i2cwrite_in_mc
	nop
	beqz v0, 2f
	nop

	li	v1, CR_STOP
	sb	v1, CR_REG(a3)
1:
	lbu	v1, SR_REG(a3)
	andi	v1, v1, SR_BUSY
	bnez	v1, 1b
	nop
#elif defined(MPS_V)
	dli a3, LS3A4000_I2C0_REG_BASE
	bal ls_v_i2cinit_in_mc
	nop

	/*write one byte do not need completed i2c stop code*/
	li  a0, MPS_ADDR
	li  a1, 0 //page select command
	or  a1, (0x1 << 16) //write 1 byte
	li  a2, 1
	bal ls_v_i2cwrite_in_mc
	nop

	li  a0, MPS_ADDR
	li  a1, 0x21 //VOUT_COMMAND
    move    a2, t0
//  li  a2, 91 //1.4v
//  li  a2, 81 //1.3v
//	li  a2, 71 //1.2v
//	li  a2, 61 //1.1v
	bal ls_v_i2cwrite_in_mc
	nop

	li	v1, CR_STOP
	sb	v1, CR_REG(a3)
1:
	lbu	v1, SR_REG(a3)
	andi	v1, v1, SR_BUSY
	bnez	v1, 1b
	nop

	li  a0, MPS_ADDR
	li  a1, 0 //page select command
	or  a1, (0x1 << 16) //write 1 byte
	li  a2, 0
	bal ls_v_i2cwrite_in_mc
	nop
	/*mps chip control one byte do not check return value*/
#else
#error "ls_v_i2cinit_in_mc: PMIC not supported"
#endif

	TTYDBG("\r\nio ctrl end")
	move	ra,t3
	jr      ra
	nop
2:
	TTYDBG("\r\nio ctrl err")
	move	ra,t3
	jr      ra
	nop
	.set pop
END(v_p_ctrl_in_mc)
LEAF(ls_v_i2cinit_in_mc)
	.set    push
	.set    noreorder
	.set    mips3
	//LPB clock_a,SCL clock_s,prescale = clock_a / (4 * clock_s);
    GET_NODE_ID_a0
	or	a3, a0
	lb	v1, CTR_REG(a3)
	and	v1, ~(1 << 7)
	sb	v1, CTR_REG(a3)

#ifdef BONITO_100M
	li	v1, 0x53 //100M
#else
	li	v1, 0x63 //25M
#endif
	sb	v1, PRER_LO_REG(a3)

#ifdef BONITO_100M
	li	v1, 0x2 //
#else
	li	v1, 0x0 //25M
#endif
	sb	v1, 0x1(a3)

	lb	v1, CTR_REG(a3)
	or	v1, (1 << 7)
	sb	v1, CTR_REG(a3)

	jr      ra
	nop
	.set pop
END(ls_v_i2cinit_in_mc)
LEAF(ls_v_i2cwrite_in_mc)
/*
 * use register:
 *	v0, v1
 *	a0, a1, a3
 *	input: a0,a1,a2
 *	a0: device ID
 *	a1: register offset/command, [16-31] store write count now 1 mean 1 byte default 0/2  byte 2
 *	a2: configure value
 *	a3: i2c register base address
 *	v0: return value
 *	a0: (node_id << 44) do not change it
 */
	.set    push
	.set    noreorder
	.set    mips3
/*i2c_send_addr*/
	/* load device address */
	move	v1, a0
    GET_NODE_ID_a0
	or	a3, a0
	sb	v1, TXR_REG(a3)

	/* send start frame */
	li	v1, CR_START | CR_WRITE
	sb	v1, CR_REG(a3)

	/* wait send finished */
//	i2c_wait_tip
1:
	lbu	v1, SR_REG(a3)
	andi	v1, v1, SR_TIP
	bnez	v1, 1b
	nop

	//check ACK
	lbu	v1, SR_REG(a3)
	andi	v1, v1, SR_NOACK
	bnez	v1, 3f
	nop

	/* load data(offset/command) to be send */
	move	v1, a1
	sb	v1, TXR_REG(a3)

	/* send data frame */
	li	v1, CR_WRITE
	sb	v1, CR_REG(a3)

	/* wait send finished */
//	i2c_wait_tip
1:
	lbu	v1, SR_REG(a3)
	andi	v1, v1, SR_TIP
	bnez	v1, 1b
	nop

	//check ACK
	lbu	v1, SR_REG(a3)
	andi	v1, v1, SR_NOACK
	bnez	v1, 3f
	nop
/*i2c_send_addr*/

/* i2c write max data is word*/
/*i2c tx byte*/
	/* load configure value */
	move	v1, a2
	sb	v1, TXR_REG(a3)

	/* send start frame */
	li	v1, CR_WRITE
	sb	v1, CR_REG(a3)

	/* wait send finished */
//	i2c_wait_tip
1:
	lbu	v1, SR_REG(a3)
	andi	v1, v1, SR_TIP
	bnez	v1, 1b
	nop

	//check ACK
	lbu	v1, SR_REG(a3)
	andi	v1, v1, SR_NOACK
	bnez	v1, 3f
	nop
	/*get count*/
	and v1, a1, (1 << 16)
	bnez v1, 3f
	nop
/*i2c tx byte*/
/*i2c tx byte*/
	/* load configure value */
	move	v1, a2
	srl	v1, v1, 8
	sb	v1, TXR_REG(a3)

	/* send start frame */
	li	v1, CR_WRITE
	sb	v1, CR_REG(a3)

	/* wait send finished */
//	i2c_wait_tip
1:
	lbu	v1, SR_REG(a3)
	andi	v1, v1, SR_TIP
	bnez	v1, 1b
	nop

	//check ACK
	lbu	v1, SR_REG(a3)
	andi	v1, v1, SR_NOACK
	bnez	v1, 3f
	nop

	b	4f
	nop
/*i2c tx byte*/
3:
/* i2c_stop */
	/* free i2c bus */
	li	v1, CR_STOP
	sb	v1, CR_REG(a3)
1:
	lbu	v1, SR_REG(a3)
	andi	v1, v1, SR_BUSY
	bnez	v1, 1b
	nop
	li	v0, 0 //return value check this function
4:
	jr	ra
	nop
	.set pop
END(ls_v_i2cwrite_in_mc)
#endif

LEAF(get_mrs7_cs)
/***********
reg used:
    a0, a1, v0, v1
output:
v0: logical cs number-> physical cs4
v1: logical cs number-> physical cs0
***********/
    dli     a1, 0x0
1:
    lw      a0, DDR4_CS_MAP_OFFSET(t8)
    dsll    v0, a1, 2
    dsrl    a0, v0
    andi    a0, 0xf
    beqz    a0, 1f
    nop
    daddu   a1, a1, 1
    GET_CS_NUM_DDR4
    bltu    a1, v0, 1b
    nop
    li      v1, 0xf //0xf->no physcial cs0
    b       2f
    nop
1:
    move    v1, a1  //store physical cs0 to v1
2:

    dli     a1, 0x0
1:
    lw      a0, DDR4_CS_MAP_OFFSET(t8)
    dsll    v0, a1, 2
    dsrl    a0, v0
    andi    a0, 0xf
    beq     a0, 4, 1f
    nop
    daddu   a1, a1, 1
    GET_CS_NUM_DDR4
    bleu    a1, v0, 1b
    nop
    li      v0, 0xf //0xf->no physcial cs4
    b       2f
    nop
1:
    move    v0, a1  //store physical cs4 to v0
2:

    jr      ra
    nop
END(get_mrs7_cs)
LEAF(mrs_send)
/*************
input:
a0: [17: 0] cmd_a
    [23:20] mrs_number
    [27:24] cmd_cs
reg used:
    a0, v0, v1
*************/
//enter command mode
    li      v0, 0x1
    sb      v0, CMD_OFFSET(t8)

1:
    lbu     v0, CMD_STATUS_OFFSET(t8)
    beqz    v0, 1b
    nop

//set cmd_*

    dsrl    v1, a0, 24
    and     v1, 0xf
    li      v0, 1
    dsll    v0, v0, v1
    not     v0
    andi    v0, 0xff
    sb      v0, CMD_CS_OFFSET(t8)

    li      v0, 0x0
    sb      v0, CMD_C_OFFSET(t8)

    li      v0, 0x8//mrs
    sb      v0, CMD_CMD_OFFSET(t8)

    and     v0, a0, 0x3ffff
    sw      v0, CMD_A_OFFSET(t8)

    li      v0, 0xff
    sb      v0, CMD_CKE_OFFSET(t8)

    //select MR
    dsrl    a0, 20
    and     a0, 0xf
    andi    v0, a0, 0x3
    sb      v0, CMD_BA_OFFSET(t8)
    dsrl    v0, a0, 2
    sb      v0, CMD_BG_OFFSET(t8)

    li      v0, 0x1
    sb      v0, CMD_REQ_OFFSET(t8)

    WAIT_FOR(0x30000)

//exit cmd mode
    li      v0, 0x0
    sb      v0, CMD_REQ_OFFSET(t8)

//exit cmd mode
    li      v0, 0x0
    sb      v0, CMD_OFFSET(t8)

    jr      ra
    nop
END(mrs_send)
LEAF(rdimm_mrs_send)
/*************
input:
a0: [17: 0] cmd_a
    [23:20] mrs_number
    [27:24] cmd_cs
reg used:
    a0, v0, v1
*************/
//enter command mode
    li      v0, 0x1
    sb      v0, CMD_OFFSET(t8)

1:
    lbu     v0, CMD_STATUS_OFFSET(t8)
    beqz    v0, 1b
    nop

//set cmd_*
    dsrl    v1, a0, 24
    and     v1, 0xf
    li      v0, 1
    dsll    v0, v0, v1
    not     v0
    andi    v0, 0xff
    sb      v0, CMD_CS_OFFSET(t8)

    li      v0, 0x0
    sb      v0, CMD_C_OFFSET(t8)

    li      v0, 0x8//mrs
    sb      v0, CMD_CMD_OFFSET(t8)

    //select MR
    dsrl    v1, a0, 20
    and     v1, 0xf
    andi    v0, v1, 0x3
    sb      v0, CMD_BA_OFFSET(t8)
    dsrl    v0, v1, 2
    sb      v0, CMD_BG_OFFSET(t8)

    and     a0, 0x3ffff
    dli     v0, (1<<14)
    and     v0, a0
    and     a0, ~(0x7<<14)
    dsll    v0, 3
    or      v0, a0
    sw      v0, CMD_A_OFFSET(t8)

    li      v0, 0xff
    sb      v0, CMD_CKE_OFFSET(t8)

    li      v0, 0x1
    sb      v0, CMD_REQ_OFFSET(t8)

    dli     v0, 0x60
1:
    subu    v0, 1
    bnez    v0, 1b
    nop

    li      v0, 0x0
    sb      v0, CMD_REQ_OFFSET(t8)

//invert address and BG/BA, repeat send mrs for B side
    lw      v1, CMD_A_OFFSET(t8)
    and     v1, 0x3ffff
    not     v0, v1
    and     v0, (0x7f<<3 | 0x1<<11 | 0x1<<13 | 0x1<<17)
    and     v1, ~(0x7f<<3 | 0x1<<11 | 0x1<<13 | 0x1<<17)
    or      v1, v0
    sw      v1, CMD_A_OFFSET(t8)

    li      v0, 0xff
    sb      v0, CMD_CKE_OFFSET(t8)

    lb      v0, CMD_BA_OFFSET(t8)
    not     v0
    andi    v0, 0x3
    sb      v0, CMD_BA_OFFSET(t8)
    lb      v0, CMD_BG_OFFSET(t8)
    not     v0
    andi    v0, 0x3
    sb      v0, CMD_BG_OFFSET(t8)

    li      v0, 0x1
    sb      v0, CMD_REQ_OFFSET(t8)

    WAIT_FOR(0x30000)

    li      v0, 0x0
    sb      v0, CMD_REQ_OFFSET(t8)

//exit cmd mode
    li      v0, 0x0
    sb      v0, CMD_OFFSET(t8)

    jr      ra
    nop
END(rdimm_mrs_send)
LEAF(mpr_write)
/*********************
input:
a0: cs_num
a1: cmd_a
*********************/
//enter command mode
    li      v0, 0x1
    sb      v0, CMD_OFFSET(t8)

1:
    lbu     v0, CMD_STATUS_OFFSET(t8)
    beqz    v0, 1b
    nop

//set cmd_*
    li      v0, 0xff
    sb      v0, CMD_CKE_OFFSET(t8)

    li      v0, 1
    dsll    v0, v0, a0
    not     v0
    andi    v0, 0xff
    sb      v0, CMD_CS_OFFSET(t8)

    li      v0, 0x0
    sb      v0, CMD_C_OFFSET(t8)

    li      v0, 0xc//wr
    sb      v0, CMD_CMD_OFFSET(t8)

    sh      a1, CMD_A_OFFSET(t8)

    li      v0, 0x0
    sb      v0, CMD_BG_OFFSET(t8)
    li      v0, 0x0
    sb      v0, CMD_BA_OFFSET(t8)

    li      v0, 0x1
    sb      v0, CMD_REQ_OFFSET(t8)

    WAIT_FOR(0x30000)

//exit cmd mode
    li      v0, 0x0
    sb      v0, CMD_OFFSET(t8)

    jr      ra
    nop
END(mpr_write)
