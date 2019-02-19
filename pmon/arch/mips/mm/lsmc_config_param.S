/**********************************
    Author: chenxinke
    Date:   20150708
    v1.0
        support lsmc(3A8, 3B5~)
    input param:
    a2: mc param address
    t7(option ARB_LEVEL)--do arb level, 0--not level; 1--do level;
    t8: input, Memory Controller config register base
    t3: controller select
        0: MC0
        1: MC1
**********************************/

        .global ddr2_config
        .ent    ddr2_config
        .set    noreorder
        .set    mips3
ddr2_config:
    daddu   a2, a2, s0
    dli     t1, DDR_PARAM_NUM
    daddiu  v0, t8, 0x0
//write param registers
1:
    ld      a1, 0x0(a2)
    sd      a1, 0x0(v0)
    daddiu  t1, t1, -1
    daddiu  a2, a2, 0x8
    daddiu  v0, v0, 0x8
    bnez    t1, 1b
    nop

#ifdef FIX_3A82H_DDR
	li	t4, 0xf00
	and	t4, t4, s1
	bne	t4, 0x4, 1f
	nop
	dli	t4, 0x18
	or	t4, t4, t8
	dli	a0, 0x3030303016100000
	sd	a0, 0x0(t4)
1:
#endif
#if 0 
//autoset param registers
set_tMOD:		
	dli		t1, FREQ
	dli		t3, 15
	dmulou	t3, t3, t1
	divu	t3, t3, 2000
	daddu	t3, t3, 1
	bge		t3, 12, 1f
	nop
	li		t3, 0xc
1:
	dli		t1,	0x1c0
	or 		t1, t1, t8
	sb		t3, 0x4(t1)

set_tFAW:
	dli		t1, FREQ
	dli		t3, 0x1e
	dmulou	t3, t3, t1
	divu	t3, t3, 2000
	daddu	t3, t3, 1

	dli		t1, 0x1c8
	or		t1, t1, t8
	sb		t3, 0x7(t1)

set_tRRD:	
	dli		t1, FREQ
	dli		t3, 0x6
	dmulou	t3, t3, t1
	divu	t3, t3, 2000
	daddu	t3, t3, 1
	bge		t3, 4, 1f
	nop
	li		t3, 0x4
1:
	dli		t1, 0x1c8
	or		t1, t1, t8
	sb		t3, 0x6(t1)

/*set_tRCD:	
	dli		t1, FREQ
	dli		t3, 0xb
	dmulou	t3, t3, t1
	divu	t3, t3, 1600
	daddu	t3, t3, 1

	dli		t1, 0x1c8
	or		t1, t1, t8
	sb		t3, 0x5(t1)
	nop

set_tRP:	
	dli		t1, FREQ
	dli		t3, 0xb
	dmulou	t3, t3, t1
	divu	t3, t3, 1600
	daddu	t3, t3, 1

	dli		t1, 0x1c8
	or		t1, t1, t8
	sb		t3, 0x4(t1)
	nop
*/
set_tREFI:
	dli		t1, FREQ
	divu	t1, t1, 10
	dli		t3, 78
	dmulou	t3, t3, t1
	dsrl	t3, t3, 9
	daddu	t3, t3, 1

	dli		t1, 0x1c8
	or		t1, t1, t8
	sb		t3, 0x3(t1)

set_tRFC:
    GET_MC0_MEMSIZE
    move	t1, a1
//	move	a0, a1
//	bal		hexserial
	nop
	GET_MC_CS_MAP
//	move	a0, a1
//	bal		hexserial
	nop
	dli 	t5, 0x4
	dli		t3, 0x0
cal_memsize:
	dsubu	t5, t5, 1
	beqz	t5, 1f
	nop
	and		t4, a1, 0x1
	dsrl	a1, a1, 0x1	
	beqz	t4, cal_memsize
	nop
	daddu	t3, t3, 1
	b		cal_memsize
	nop

1:
	divu	a2, t1, t3

	dli		t1, 0x1
	blt		a2, t1, 21f
	dli		t3, 0x9

	dli		t1, 0x2
	blt		a2, t1, 21f
	dli		t3, 0xb

	dli		t1, 0x4
	blt		a2, t1, 21f
	dli		t3, 0x10

	dli		t1, 0x8
	blt		a2, t1, 21f
	dli		t3, 0x1a

	dli		t1, 0x10
	blt		a2, t1, 21f
	dli		t3, 0x22

	PRINTSTR("\r\n memsize wrong \r\n")
	b 		2f
	nop

21:

	dli		t1, FREQ
	dmulou	t3, t3, t1
	divu	t3, t3, 200
	
	dli		t1, 0x1c8
	or		t1, t1, t8
	sb		t3, 0x2(t1)
2:
	
set_ZQCS:	
	dli		t1, FREQ
	dli		t3, 0x50
	dmulou	t3, t3, t1
	divu	t3, t3, 2000
	daddu	t3, t3, 1
	bge		t3, 64, 1f
	nop
	dli		t3, 64
1:
	dli		t1, 0x1c8
	or		t1, t1, t8

	/*lb		t4, 0x3(t1)
	divu	t3, t3, t4
	daddu	t3, t3, 1*/

	sb		t3, 0x1(t1)

set_tXPDLL:	
	dli		t1, FREQ
	dli		t3, 0x18
	dmulou	t3, t3, t1
	divu	t3, t3, 2000
	daddu	t3, t3, 1
	bge		t3, 10, 1f
	nop
	dli		t3, 10
1:
	dli		t1, 0x1d8
	or		t1, t1, t8
	sb		t3, 0x7(t1)

set_tXP:
	dli		t1, FREQ
	dli		t3, 0x6
	dmulou	t3, t3, t1
	divu	t3, t3, 2000
	daddu	t3, t3, 1
	bge		t3, 3, 1f
	nop
	dli		t3, 3
1:
	dli		t1, 0x1d8
	or		t1, t1, t8
	sb		t3, 0x6(t1)

set_tWR:	
	dli		t1, FREQ
	dli		t3, 0x15
	dmulou	t3, t3, t1
	divu	t3, t3, 2000
	daddu	t3, t3, 1
1:
	dli		t1, 0x1d8
	or		t1, t1, t8
	sb		t3, 0x5(t1)


set_tRTP:
	dli		t1, FREQ
	dli		t3, 0x8
	dmulou	t3, t3, t1
	divu	t3, t3, 2000
	daddu	t3, t3, 1
	bge		t3, 4, 1f
	nop
	dli		t3, 4
1:
	dli		t1, 0x1d8
	or		t1, t1, t8
	sb		t3, 0x4(t1)


/*set_tRL:
	dli		t1, FREQ
	dli		t3, 0xb
	dmulou	t3, t3, t1
	divu	t3, t3, 1600
	daddu	t3, t3, 1

	dli		t1, 0x1d8
	or		t1, t1, t8
	sb		t3, 0x3(t1)
	
	dsubu	t3, t3, 2
	dli		t1, 0x1c0
	or 		t1, t1, t8
	sb		t3, 0x0(t1)

	dli		t1, 0x1a0
	or 		t1, t1, t8
	lb		t4, 0x0(t1)
	li		t5, 0x8b//10001011
	and		t4, t4, t5
	dsubu	t3, t3, 2
	li		t5, 0x7
	and		t3, t3, t5
	dsll	t3, t3, 4
	or		t4, t4, t3
	
	dli		t1, FREQ
	dli		t3, 0xb
	dmulou	t3, t3, t1
	divu	t3, t3, 1600
	daddu	t3, t3, 1
	dsubu	t3, t3, 4
	dli		t5, 0x8
	and		t3, t3, t5
	dsrl	t3, t3, 1
	or		t4, t4, t3
	
	dli		t1, 0x1a0
	or		t1, t1, t8
	sb		t4, 0x0(t1)	
	sb		t4, 0x8(t1)
	sb		t4, 0x10(t1)
	sb		t4, 0x18(t1)
*/
#endif
set_tREFI:
	dli		t1, DDR_FREQ
	divu	t1, t1, 10
	dli		a2, 78
	dmulou	a2, a2, t1
#ifndef TEMP_EXTREME
	dsrl	a2, a2, 8
#else
	dsrl	a2, a2, 9
#endif
//	daddu	a2, a2, 1
	dli		t1, 0x1c8
	or		t1, t1, t8
	sb		a2, 0x3(t1)

#ifndef  NO_AUTO_TRFC 
set_tRFC:
    GET_MC0_MEMSIZE
    beqz    t3, 1f
    nop
    GET_MC1_MEMSIZE
1:
    move	t1, a1
	GET_MC_CS_MAP
    beqz    t3, 1f
    nop
	GET_MC1_CS_MAP
1:
	dli 	t5, 0x4
	dli		a2, 0x0
cal_memsize:
	dsubu	t5, t5, 1
	beqz	t5, 1f
	nop
	and		t4, a1, 0x1
	dsrl	a1, a1, 0x1	
	beqz	t4, cal_memsize
	nop
	daddu	a2, a2, 1
	b		cal_memsize
	nop
1:
	divu	a2, t1, a2

    GET_SDRAM_WIDTH
    beqz    t3, 1f
    nop
    GET_MC1_SDRAM_WIDTH
1:
	dli     t5, 0x2
    beqz    a1, x8
    nop
	dli     t5, 0x1
x8:
	divu	a2, a2, t5

	dli		t1, 0x1
	blt		a2, t1, 21f
	dli		t5, 0x9

	dli		t1, 0x2
	blt		a2, t1, 21f
	dli		t5, 0xb

	dli		t1, 0x4
	blt		a2, t1, 21f
	dli		t5, 0x10

	dli		t1, 0x8
	blt		a2, t1, 21f
	dli		t5, 0x1a

	dli		t1, 0x10
	blt		a2, t1, 21f
	dli		t5, 0x23

	PRINTSTR("\r\n memsize wrong \r\n")
	b 		2f
	nop
21:

	dli		t1, DDR_FREQ
	dmulou	t5, t5, t1
	divu	t5, t5, 100
	
	dli		t1, 0x1c8
	or		t1, t1, t8
	sb		t5, 0x2(t1)
 
2:
#endif

    //for UDIMM 4cs,open 2T mode
    GET_DIMM_TYPE
    bnez    a1, 1f
    nop
    //UDIMM
    GET_MC_CS_MAP
    dli     a2, 0xf
    bne     a1, a2, 1f
    nop
    //add cmd_timing ,trddata and tphy_wrlat by one
    ld      a2, CMD_TIMING(t8)
    dli     a1, 0x1
    dsll    a1, a1, CMD_TIMING_OFFSET
    daddu   a2, a2, a1
    sd      a2, CMD_TIMING(t8)

    ld      a2, TRDDATA(t8)
    dli     a1, 0x1
    dsll    a1, a1, TRDDATA_OFFSET
    daddu   a2, a2, a1
    sd      a2, TRDDATA(t8)

    ld      a2, TPHY_WRLAT(t8)
    dli     a1, 0x1
    dsll    a1, a1, TPHY_WRLAT_OFFSET
    daddu   a2, a2, a1
    sd      a2, TPHY_WRLAT(t8)
1:
    //for UDIMM 4cs,open 2T mode
    GET_DIMM_TYPE
    bnez    a1, 1f
    nop
    //UDIMM
    GET_MC_CS_MAP
    dli     a2, 0xf
    bne     a1, a2, 1f
    nop
    //add cmd_timing ,trddata and tphy_wrlat by one
    ld      a2, CMD_TIMING(t8)
    dli     a1, 0x1
    dsll    a1, a1, CMD_TIMING_OFFSET
    daddu   a2, a2, a1
    sd      a2, CMD_TIMING(t8)

    ld      a2, TRDDATA(t8)
    dli     a1, 0x1
    dsll    a1, a1, TRDDATA_OFFSET
    daddu   a2, a2, a1
    sd      a2, TRDDATA(t8)

    ld      a2, TPHY_WRLAT(t8)
    dli     a1, 0x1
    dsll    a1, a1, TPHY_WRLAT_OFFSET
    daddu   a2, a2, a1
    sd      a2, TPHY_WRLAT(t8)

1:
    //rewrite eight_bank_mode
    //rewrite pm_bank_diff_0 and pm_bank
    ld      a2, EIGHT_BANK_MODE_ADDR(t8)
    dli     a1, 0x3
    dsll    a1, a1, EIGHT_BANK_MODE_OFFSET
    not     a1, a1
    and     a2, a2, a1
    GET_EIGHT_BANK
    xor     a1, a1, 0x1
    dsll    a1, a1, EIGHT_BANK_MODE_OFFSET
    or      a2, a2, a1
    sd      a2, EIGHT_BANK_MODE_ADDR(t8)

    //for version 3, pm_bank not used, changed to cs_resync
    lw      a2, 0x0(t8)
    dli     a1, 0x3
    beq     a2, a1, 1f
    nop
    //rewrite pm_bank
    ld      a2, BANK_NUM_ADDR(t8)
    dli     a1, 0x7
    dsll    a1, a1, BANK_NUM_OFFSET
    not     a1, a1
    and     a2, a2, a1
    dli     v0, 0x7
    GET_EIGHT_BANK
    xor     a1, a1, 0x1
    dsrl    a1, v0, a1
    dsll    a1, a1, BANK_NUM_OFFSET
    or      a2, a2, a1
    sd      a2, BANK_NUM_ADDR(t8)

#ifndef MC_MULTI_CHANNEL
    //rewrite pm_addr_win
    ld      a2, ADDR_WIN_BANK_NUM_ADDR(t8)
    dli     a1, 0x3
    dsll    a1, a1, ADDR_WIN_BANK_NUM_OFFSET
    not     a1, a1
    and     a2, a2, a1
    dli     v0, 0x2
    GET_EIGHT_BANK
    or      a1, v0, a1
    dsll    a1, a1, ADDR_WIN_BANK_NUM_OFFSET
    or      a2, a2, a1
    sd      a2, ADDR_WIN_BANK_NUM_ADDR(t8)
#endif
1:

    //rewrite row_diff and column_diff
    ld      a2, ROW_DIFF_ADDR(t8)
    dli     a1, 0x7
    dsll    a1, a1, ROW_DIFF_OFFSET
    not     a1, a1
    and     a2, a2, a1
    GET_ROW_SIZE
    dsll    a1, a1, ROW_DIFF_OFFSET
    or      a2, a2, a1
    sd      a2, ROW_DIFF_ADDR(t8)

    ld      a2, COLUMN_DIFF_ADDR(t8)
    dli     a1, 0x7
    dsll    a1, a1, COLUMN_DIFF_OFFSET
    not     a1, a1
    and     a2, a2, a1
    GET_COL_SIZE
    daddu   a1, a1, 0x4
    dsll    a1, a1, COLUMN_DIFF_OFFSET
    or      a2, a2, a1
    sd      a2, COLUMN_DIFF_ADDR(t8)

    //rewrite cs_diff
    ld      a2, CS_DIFF_ADDR(t8)
    dli     a1, 0x3
    dsll    a1, a1, CS_DIFF_OFFSET
    not     a1, a1
    and     a2, a2, a1
    //count cs num to a3
    GET_MC_CS_MAP
    and     a0, a1, 0x1
    move    a3, a0
    dsrl    a1, a1, 1
    and     a0, a1, 0x1
    daddu   a3, a3, a0
    dsrl    a1, a1, 1
    and     a0, a1, 0x1
    daddu   a3, a3, a0
    dsrl    a1, a1, 1
    and     a0, a1, 0x1
    daddu   a3, a3, a0

    move    a1, $0
    //dli   a0, 0x2
    daddu   a0, $0, 0x1
    daddu   a0, a0, 0x1
    bgt     a3, a0, 1f  //4 or 3 ranks
    nop
    daddu   a1, a1, 0x1
    beq     a3, a0, 1f  //2 ranks
    nop
    daddu   a1, a1, 0x1 //1 or 0 rank
1:
    dsll    a1, a1, CS_DIFF_OFFSET
    or      a2, a2, a1
    sd      a2, CS_DIFF_ADDR(t8)

    and     a0, a3, 0x1
    beqz    a0, 1f
    nop
    //if there are 1 or 3 ranks, disable cs_place
    ld      a2, CS_PLACE_ADDR(t8)
    dli     a1, 0x1
    dsll    a1, a1, CS_PLACE_OFFSET
    not     a1, a1
    and     a2, a2, a1
    sd      a2, CS_PLACE_ADDR(t8)
1:
    //rewrite cs_enable cs_mrs and cs_zq
    ld      a2, CS_ENABLE_ADDR(t8)
    dli     a1, 0xf
    dsll    a1, a1, CS_ENABLE_OFFSET
    not     a1, a1
    and     a2, a2, a1
    //GET_MC_CS_MAP
    dli     a1, 0xf
    dsll    a1, a1, CS_ENABLE_OFFSET
    or      a2, a2, a1
    sd      a2, CS_ENABLE_ADDR(t8)

    ld      a2, CS_MRS_ADDR(t8)
    dli     a1, 0xf
    dsll    a1, a1, CS_MRS_OFFSET
    not     a1, a1
    and     a2, a2, a1
    GET_MC_CS_MAP
    dsll    a1, a1, CS_MRS_OFFSET
    or      a2, a2, a1
    sd      a2, CS_MRS_ADDR(t8)

    ld      a2, CS_ZQ_ADDR(t8)
    dli     a1, 0xf
    dsll    a1, a1, CS_ZQ_OFFSET
    not     a1, a1
    and     a2, a2, a1
    GET_MC_CS_MAP
    dsll    a1, a1, CS_ZQ_OFFSET
    or      a2, a2, a1
    sd      a2, CS_ZQ_ADDR(t8)

    //for version 3, pm_bank not used, changed to cs_resync
    lw      a2, 0x0(t8)
    dli     a1, 0x3
    bne     a2, a1, 1f
    nop
    ld      a2, BANK_NUM_ADDR(t8)
    dli     a1, 0xf
    dsll    a1, a1, BANK_NUM_OFFSET
    not     a1, a1
    and     a2, a2, a1
    GET_MC_CS_MAP
    dsll    a1, a1, BANK_NUM_OFFSET
    or      a2, a2, a1
    sd      a2, BANK_NUM_ADDR(t8)
1:
    //reconfigure pm_cs_map
    ld      a2, CS_MAP_ADDR(t8)
    dli     a1, 0xff
    dsll    a1, a1, CS_MAP_OFFSET
    not     a1, a1
    and     a2, a2, a1

    //a0: logic cs; a3: pm_cs_map; v0: temp; v1: DDR_CS pin;
    move    a0, $0
    move    a3, $0
    move    v1, $0
    GET_MC_CS_MAP
2:
    and     v0, a1, 0x1
    beqz    v0, 1f
    nop
    dsll    v0, a0, 1   //v0 = a0 * 2
    dsll    v0, v1, v0
    or      a3, a3, v0
    daddu   a0, a0, 0x1
1:
    daddu   v1, v1, 0x1
    dsrl    a1, a1, 1
    dli     v0, 0x4
    blt     v1, v0, 2b
    nop

    dsll    a3, a3, CS_MAP_OFFSET
    or      a2, a2, a3
    sd      a2, CS_MAP_ADDR(t8)

    //rewrite lvl_cs
    ld      a2, LEVEL_CS_ADDR(t8)
    dli     a1, 0xf
    dsll    a1, a1, LEVEL_CS_OFFSET
    not     a1, a1
    and     a2, a2, a1
    GET_MC_CS_MAP
    beqz    a1, 4f
    nop
    daddu   v1, $0, 0x1
1:
    and     v0, a1, v1
    bnez    v0, 2f
    nop
    dsll    v1, v1, 1
    b       1b
    nop
2:
    dsll    v1, v1, LEVEL_CS_OFFSET
    or      a2, a2, v1
    sd      a2, LEVEL_CS_ADDR(t8)
4:

    //reconfig address_mirroring
    ld      a2, ADDRESS_MIRROR_ADDR(t8)
    dli     a1, 0xf
    dsll    a1, a1, ADDRESS_MIRROR_OFFSET
    not     a1, a1
    and     a2, a2, a1
    //When use RDIMM, ignore address mirror bit
    GET_DIMM_TYPE
    bnez    a1, 1f
    nop
    GET_ADDR_MIRROR
    beqz    a1, 1f
    nop
    dli     a1, 0xa
    b       2f
    nop
1:
    dli     a1, 0x0
2:
    dsll    a1, a1, ADDRESS_MIRROR_OFFSET
    or      a2, a2, a1
    sd      a2, ADDRESS_MIRROR_ADDR(t8)
#ifndef MANAUL_ODT_MAP
    //reconfig ODT map
    //set default first
    //clear map first
    dli     a1, 0x0000ffff0000ffff
    ld      a2, ODT_MAP_CS_ADDR(t8)
    and     a2, a2, a1
    sd      a2, ODT_MAP_CS_ADDR(t8)

    dli     a2, 0x8421000000000000  //DDR3
    GET_SDRAM_TYPE
    dli     a0, 0x3
    beq     a0, a1, 2f
    nop

    dli     a2, 0x8421000084210000  //DDR2
2:
    ld      a1, ODT_MAP_CS_ADDR(t8)
    or      a2, a2, a1
    sd      a2, ODT_MAP_CS_ADDR(t8)

    //v0 store cs map
    GET_MC_CS_MAP
    move    v0, a1
    //step 1: swap open wr odt if it's a Dual Rank DIMM
    //check cs_map[3]
    dsrl    a2, v0, 3
    beqz    a2, 1f
    nop
    //slot 1 is a DR DIMM
    ld      a2, ODT_MAP_CS_ADDR(t8)
    dli     a1, 0x00ffffffffffffff
    and     a2, a2, a1
    dli     a1, 0x4800000000000000
    or      a2, a2, a1
    sd      a2, ODT_MAP_CS_ADDR(t8)
1:
    //check cs_map[1]
    dsrl    a2, v0, 1
    and     a2, a2, 0x1
    beqz    a2, 1f
    nop
    //slot 0 is a DR DIMM
    ld      a2, ODT_MAP_CS_ADDR(t8)
    dli     a1, 0xff00ffffffffffff
    and     a2, a2, a1
    dli     a1, 0x0012000000000000
    or      a2, a2, a1
    sd      a2, ODT_MAP_CS_ADDR(t8)
1:
    //step 2: open extra RD/WR ODT CS if there is 2 DIMM
    //check CS[0] and CS[2]
    dsrl    a2, v0, 2
    xor     a2, v0, a2
    and     a2, a2, 0x1
    bnez    a2, 1f
    nop
    //2 DIMM: open the first rank of the non-target DIMM
    ld      a2, ODT_MAP_CS_ADDR(t8)
    dli     a1, 0x1144000011440000
    or      a2, a2, a1
    sd      a2, ODT_MAP_CS_ADDR(t8)
1:  //1 DIMM
#if 0
    //if its DDR3_DIMM, enable dynamic ODT and reset ODT value
    GET_SDRAM_TYPE
    dli     a2, 0x3
    bne     a2, a1, 1f
    nop
    //DDR3 DIMM, enable RTT_wr
    ld      a2, MR2_DATA_0_ADDR(t8)
    dli     a1, 0x0400
    dsll    a1, a1, MR2_DATA_0_OFFSET
    or      a2, a2, a1
    sd      a2, MR2_DATA_0_ADDR(t8)

    ld      a2, MR2_DATA_1_ADDR(t8)
    dli     a1, 0x0400
    dsll    a1, a1, MR2_DATA_1_OFFSET
    or      a2, a2, a1
    sd      a2, MR2_DATA_1_ADDR(t8)

    ld      a2, MR2_DATA_2_ADDR(t8)
    dli     a1, 0x0400
    dsll    a1, a1, MR2_DATA_2_OFFSET
    or      a2, a2, a1
    sd      a2, MR2_DATA_2_ADDR(t8)

    ld      a2, MR2_DATA_3_ADDR(t8)
    dli     a1, 0x0400
    dsll    a1, a1, MR2_DATA_3_OFFSET
    or      a2, a2, a1
    sd      a2, MR2_DATA_3_ADDR(t8)
1:
#endif
#endif
#if 0
    //set data bus width
    ld      a2, DATA_WIDTH_32_ADDR(t8)
    dli     a1, 0x1
    dsll    a1, a1, DATA_WIDTH_32_OFFSET
    not     a1, a1
    and     a2, a2, a1
    GET_DIMM_WIDTH
    dsll    a1, a1, DATA_WIDTH_32_OFFSET
    or      a2, a2, a1
    sd      a2, DATA_WIDTH_32_ADDR(t8)
#ifndef MC_MULTI_CHANNEL
    //rewrite multi_channel mode
    ld      a2, MC_MULTI_CHANNEL_ADDR(t8)
    dli     a1, 0x1
    dsll    a1, a1, MC_MULTI_CHANNEL_OFFSET
    not     a1, a1
    and     a2, a2, a1
    GET_DIMM_WIDTH
    dsll    a1, a1, MC_MULTI_CHANNEL_OFFSET
    or      a2, a2, a1
    sd      a2, MC_MULTI_CHANNEL_ADDR(t8)
#endif
#ifndef MC_MULTI_CHANNEL
    //rewrite pm_addr_win(data width)
    ld      a2, ADDR_WIN_DATA_WIDTH_ADDR(t8)
    dli     a1, 0x3
    dsll    a1, a1, ADDR_WIN_DATA_WIDTH_OFFSET
    not     a1, a1
    and     a2, a2, a1
    dli     v0, 0x3
    GET_DIMM_WIDTH
    xor     a1, v0, a1
    dsll    a1, a1, ADDR_WIN_DATA_WIDTH_OFFSET
    or      a2, a2, a1
    sd      a2, ADDR_WIN_DATA_WIDTH_ADDR(t8)
#endif
#endif
    //disable ECC module here for leveling, ECC will be enabled later
    ld      a2, ECC_ENABLE_ADDR(t8)
    dli     a1, 0x7
    dsll    a1, a1, ECC_ENABLE_OFFSET
    not     a1, a1
    and     a2, a2, a1
#ifndef DISABLE_DIMM_ECC
    GET_DIMM_ECC
    beqz    a1, 1f
    nop
    dsll    a1, a1, ECC_ENABLE_OFFSET
    or      a2, a2, a1
1:
#endif
    sd      a2, ECC_ENABLE_ADDR(t8)

    //reconfigure CS_1/2/3 addr info as CS_0
    ld      a2, ADDR_INFO_CS_0_ADDR(t8)
    sd      a2, ADDR_INFO_CS_1_ADDR(t8)
    sd      a2, ADDR_INFO_CS_2_ADDR(t8)
    sd      a2, ADDR_INFO_CS_3_ADDR(t8)

#ifdef DEBUG_DDR_PARAM   //debug
//input once, change all byte lanes parameters.
    /* store the ra */
    move    t1, ra

41:
    PRINTSTR("\r\nChange parameters:\r\n0--skip;1--dll_clock;2--dll_rddqs_gate;3--dll_rddqs_p;4--dll_rddqs_n;5--dll_wrdqs;6--dll_wrdq;\r\n7--rd_oe_end/start-edge_stop/start;8--wr_dqs_oe_end/start-edge_stop/start;9--wr_dq_oe_end/start-edge_stop/start;a--wr_odt_oe_end/start-edge_stop/start;\r\nb--wrdq_clk_delay;c--rddata_delay;d--rddqs_lt_half;e--wrdqs_lt_half;f--wrdq_lt_half;\r\n");
    dli     t6, 0x00
    bal     inputaddress
    nop
    beqz    v0, 90f;
    nop
    move    t5, v0
    PRINTSTR("\r\nPlease input the data-hex: ");
    dli     t6, 0x00
    bal     inputaddress
    nop
    move    a2, t5
    dli     a1, 0xffffffff
    and     t5, v0, a1
/*****************
a2: change select
t5: value
*****************/

//!!!!!note: don't change the switch order of the code bellow, because we use
//add instr to change a1 instead of dli instr to reduce code size.
    dli     a1, 0x1
    beq     a2, a1, 1f;
    nop
    daddiu  a1, a1, 0x1
    beq     a2, a1, 2f;
    nop
    daddiu  a1, a1, 0x1
    beq     a2, a1, 3f;
    nop
    daddiu  a1, a1, 0x1
    beq     a2, a1, 4f;
    nop
    daddiu  a1, a1, 0x1
    beq     a2, a1, 5f;
    nop
    daddiu  a1, a1, 0x1
    beq     a2, a1, 6f;
    nop
    daddiu  a1, a1, 0x1
    beq     a2, a1, 7f;
    nop
    daddiu  a1, a1, 0x1
    beq     a2, a1, 8f;
    nop
    daddiu  a1, a1, 0x1
    beq     a2, a1, 9f;
    nop
    daddiu  a1, a1, 0x1
    beq     a2, a1, 10f;
    nop
    daddiu  a1, a1, 0x1
    beq     a2, a1, 11f;
    nop
    daddiu  a1, a1, 0x1
    beq     a2, a1, 12f;
    nop
    daddiu  a1, a1, 0x1
    beq     a2, a1, 13f;
    nop
    daddiu  a1, a1, 0x1
    beq     a2, a1, 14f;
    nop
    daddiu  a1, a1, 0x1
    beq     a2, a1, 15f;
    nop
    PRINTSTR("\r\n--------Wrong selection: no parameter will be changed.");
    b       40f
    nop
1:
    and     t5, t5, CLKLVL_DELAY_MASK

    ld      a1, CLKLVL_DELAY_2_ADDR(t8)
    dli     a2, CLKLVL_DELAY_MASK
    dsll    a2, a2, CLKLVL_DELAY_2_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, CLKLVL_DELAY_2_OFFSET
    or      a1, a1, a2
    sd      a1, CLKLVL_DELAY_2_ADDR(t8)

    ld      a1, CLKLVL_DELAY_1_ADDR(t8)
    dli     a2, CLKLVL_DELAY_MASK
    dsll    a2, a2, CLKLVL_DELAY_1_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, CLKLVL_DELAY_1_OFFSET
    or      a1, a1, a2
    sd      a1, CLKLVL_DELAY_1_ADDR(t8)

    ld      a1, CLKLVL_DELAY_0_ADDR(t8)
    dli     a2, CLKLVL_DELAY_MASK
    dsll    a2, a2, CLKLVL_DELAY_0_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, CLKLVL_DELAY_0_OFFSET
    or      a1, a1, a2
    sd      a1, CLKLVL_DELAY_0_ADDR(t8)
    b       40f
    nop
2:
    and     t5, t5, RDLVL_GATE_DELAY_MASK

    ld      a1, RDLVL_GATE_DELAY_8_ADDR(t8)
    dli     a2, RDLVL_GATE_DELAY_MASK
    dsll    a2, a2, RDLVL_GATE_DELAY_8_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, RDLVL_GATE_DELAY_8_OFFSET
    or      a1, a1, a2
    sd      a1, RDLVL_GATE_DELAY_8_ADDR(t8)

    ld      a1, RDLVL_GATE_DELAY_7_ADDR(t8)
    dli     a2, RDLVL_GATE_DELAY_MASK
    dsll    a2, a2, RDLVL_GATE_DELAY_7_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, RDLVL_GATE_DELAY_7_OFFSET
    or      a1, a1, a2
    sd      a1, RDLVL_GATE_DELAY_7_ADDR(t8)

    ld      a1, RDLVL_GATE_DELAY_6_ADDR(t8)
    dli     a2, RDLVL_GATE_DELAY_MASK
    dsll    a2, a2, RDLVL_GATE_DELAY_6_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, RDLVL_GATE_DELAY_6_OFFSET
    or      a1, a1, a2
    sd      a1, RDLVL_GATE_DELAY_6_ADDR(t8)

    ld      a1, RDLVL_GATE_DELAY_5_ADDR(t8)
    dli     a2, RDLVL_GATE_DELAY_MASK
    dsll    a2, a2, RDLVL_GATE_DELAY_5_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, RDLVL_GATE_DELAY_5_OFFSET
    or      a1, a1, a2
    sd      a1, RDLVL_GATE_DELAY_5_ADDR(t8)

    ld      a1, RDLVL_GATE_DELAY_4_ADDR(t8)
    dli     a2, RDLVL_GATE_DELAY_MASK
    dsll    a2, a2, RDLVL_GATE_DELAY_4_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, RDLVL_GATE_DELAY_4_OFFSET
    or      a1, a1, a2
    sd      a1, RDLVL_GATE_DELAY_4_ADDR(t8)

    ld      a1, RDLVL_GATE_DELAY_3_ADDR(t8)
    dli     a2, RDLVL_GATE_DELAY_MASK
    dsll    a2, a2, RDLVL_GATE_DELAY_3_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, RDLVL_GATE_DELAY_3_OFFSET
    or      a1, a1, a2
    sd      a1, RDLVL_GATE_DELAY_3_ADDR(t8)

    ld      a1, RDLVL_GATE_DELAY_2_ADDR(t8)
    dli     a2, RDLVL_GATE_DELAY_MASK
    dsll    a2, a2, RDLVL_GATE_DELAY_2_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, RDLVL_GATE_DELAY_2_OFFSET
    or      a1, a1, a2
    sd      a1, RDLVL_GATE_DELAY_2_ADDR(t8)

    ld      a1, RDLVL_GATE_DELAY_1_ADDR(t8)
    dli     a2, RDLVL_GATE_DELAY_MASK
    dsll    a2, a2, RDLVL_GATE_DELAY_1_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, RDLVL_GATE_DELAY_1_OFFSET
    or      a1, a1, a2
    sd      a1, RDLVL_GATE_DELAY_1_ADDR(t8)

    ld      a1, RDLVL_GATE_DELAY_0_ADDR(t8)
    dli     a2, RDLVL_GATE_DELAY_MASK
    dsll    a2, a2, RDLVL_GATE_DELAY_0_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, RDLVL_GATE_DELAY_0_OFFSET
    or      a1, a1, a2
    sd      a1, RDLVL_GATE_DELAY_0_ADDR(t8)
    b       40f
    nop
3:
    and     t5, t5, RDLVL_DELAY_MASK

    ld      a1, RDLVL_DELAY_8_ADDR(t8)
    dli     a2, RDLVL_DELAY_MASK
    dsll    a2, a2, RDLVL_DELAY_8_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, RDLVL_DELAY_8_OFFSET
    or      a1, a1, a2
    sd      a1, RDLVL_DELAY_8_ADDR(t8)

    ld      a1, RDLVL_DELAY_7_ADDR(t8)
    dli     a2, RDLVL_DELAY_MASK
    dsll    a2, a2, RDLVL_DELAY_7_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, RDLVL_DELAY_7_OFFSET
    or      a1, a1, a2
    sd      a1, RDLVL_DELAY_7_ADDR(t8)

    ld      a1, RDLVL_DELAY_6_ADDR(t8)
    dli     a2, RDLVL_DELAY_MASK
    dsll    a2, a2, RDLVL_DELAY_6_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, RDLVL_DELAY_6_OFFSET
    or      a1, a1, a2
    sd      a1, RDLVL_DELAY_6_ADDR(t8)

    ld      a1, RDLVL_DELAY_5_ADDR(t8)
    dli     a2, RDLVL_DELAY_MASK
    dsll    a2, a2, RDLVL_DELAY_5_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, RDLVL_DELAY_5_OFFSET
    or      a1, a1, a2
    sd      a1, RDLVL_DELAY_5_ADDR(t8)

    ld      a1, RDLVL_DELAY_4_ADDR(t8)
    dli     a2, RDLVL_DELAY_MASK
    dsll    a2, a2, RDLVL_DELAY_4_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, RDLVL_DELAY_4_OFFSET
    or      a1, a1, a2
    sd      a1, RDLVL_DELAY_4_ADDR(t8)

    ld      a1, RDLVL_DELAY_3_ADDR(t8)
    dli     a2, RDLVL_DELAY_MASK
    dsll    a2, a2, RDLVL_DELAY_3_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, RDLVL_DELAY_3_OFFSET
    or      a1, a1, a2
    sd      a1, RDLVL_DELAY_3_ADDR(t8)

    ld      a1, RDLVL_DELAY_2_ADDR(t8)
    dli     a2, RDLVL_DELAY_MASK
    dsll    a2, a2, RDLVL_DELAY_2_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, RDLVL_DELAY_2_OFFSET
    or      a1, a1, a2
    sd      a1, RDLVL_DELAY_2_ADDR(t8)

    ld      a1, RDLVL_DELAY_1_ADDR(t8)
    dli     a2, RDLVL_DELAY_MASK
    dsll    a2, a2, RDLVL_DELAY_1_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, RDLVL_DELAY_1_OFFSET
    or      a1, a1, a2
    sd      a1, RDLVL_DELAY_1_ADDR(t8)

    ld      a1, RDLVL_DELAY_0_ADDR(t8)
    dli     a2, RDLVL_DELAY_MASK
    dsll    a2, a2, RDLVL_DELAY_0_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, RDLVL_DELAY_0_OFFSET
    or      a1, a1, a2
    sd      a1, RDLVL_DELAY_0_ADDR(t8)
    b       40f
    nop
4:
    and     t5, t5, RDLVL_DQSN_DELAY_MASK

    ld      a1, RDLVL_DQSN_DELAY_8_ADDR(t8)
    dli     a2, RDLVL_DQSN_DELAY_MASK
    dsll    a2, a2, RDLVL_DQSN_DELAY_8_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, RDLVL_DQSN_DELAY_8_OFFSET
    or      a1, a1, a2
    sd      a1, RDLVL_DQSN_DELAY_8_ADDR(t8)

    ld      a1, RDLVL_DQSN_DELAY_7_ADDR(t8)
    dli     a2, RDLVL_DQSN_DELAY_MASK
    dsll    a2, a2, RDLVL_DQSN_DELAY_7_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, RDLVL_DQSN_DELAY_7_OFFSET
    or      a1, a1, a2
    sd      a1, RDLVL_DQSN_DELAY_7_ADDR(t8)

    ld      a1, RDLVL_DQSN_DELAY_6_ADDR(t8)
    dli     a2, RDLVL_DQSN_DELAY_MASK
    dsll    a2, a2, RDLVL_DQSN_DELAY_6_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, RDLVL_DQSN_DELAY_6_OFFSET
    or      a1, a1, a2
    sd      a1, RDLVL_DQSN_DELAY_6_ADDR(t8)

    ld      a1, RDLVL_DQSN_DELAY_5_ADDR(t8)
    dli     a2, RDLVL_DQSN_DELAY_MASK
    dsll    a2, a2, RDLVL_DQSN_DELAY_5_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, RDLVL_DQSN_DELAY_5_OFFSET
    or      a1, a1, a2
    sd      a1, RDLVL_DQSN_DELAY_5_ADDR(t8)

    ld      a1, RDLVL_DQSN_DELAY_4_ADDR(t8)
    dli     a2, RDLVL_DQSN_DELAY_MASK
    dsll    a2, a2, RDLVL_DQSN_DELAY_4_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, RDLVL_DQSN_DELAY_4_OFFSET
    or      a1, a1, a2
    sd      a1, RDLVL_DQSN_DELAY_4_ADDR(t8)

    ld      a1, RDLVL_DQSN_DELAY_3_ADDR(t8)
    dli     a2, RDLVL_DQSN_DELAY_MASK
    dsll    a2, a2, RDLVL_DQSN_DELAY_3_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, RDLVL_DQSN_DELAY_3_OFFSET
    or      a1, a1, a2
    sd      a1, RDLVL_DQSN_DELAY_3_ADDR(t8)

    ld      a1, RDLVL_DQSN_DELAY_2_ADDR(t8)
    dli     a2, RDLVL_DQSN_DELAY_MASK
    dsll    a2, a2, RDLVL_DQSN_DELAY_2_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, RDLVL_DQSN_DELAY_2_OFFSET
    or      a1, a1, a2
    sd      a1, RDLVL_DQSN_DELAY_2_ADDR(t8)

    ld      a1, RDLVL_DQSN_DELAY_1_ADDR(t8)
    dli     a2, RDLVL_DQSN_DELAY_MASK
    dsll    a2, a2, RDLVL_DQSN_DELAY_1_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, RDLVL_DQSN_DELAY_1_OFFSET
    or      a1, a1, a2
    sd      a1, RDLVL_DQSN_DELAY_1_ADDR(t8)

    ld      a1, RDLVL_DQSN_DELAY_0_ADDR(t8)
    dli     a2, RDLVL_DQSN_DELAY_MASK
    dsll    a2, a2, RDLVL_DQSN_DELAY_0_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, RDLVL_DQSN_DELAY_0_OFFSET
    or      a1, a1, a2
    sd      a1, RDLVL_DQSN_DELAY_0_ADDR(t8)
    b       40f
    nop
5:
    and     t5, t5, WRLVL_DELAY_MASK

    ld      a1, WRLVL_DELAY_8_ADDR(t8)
    dli     a2, WRLVL_DELAY_MASK
    dsll    a2, a2, WRLVL_DELAY_8_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, WRLVL_DELAY_8_OFFSET
    or      a1, a1, a2
    sd      a1, WRLVL_DELAY_8_ADDR(t8)

    ld      a1, WRLVL_DELAY_7_ADDR(t8)
    dli     a2, WRLVL_DELAY_MASK
    dsll    a2, a2, WRLVL_DELAY_7_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, WRLVL_DELAY_7_OFFSET
    or      a1, a1, a2
    sd      a1, WRLVL_DELAY_7_ADDR(t8)

    ld      a1, WRLVL_DELAY_6_ADDR(t8)
    dli     a2, WRLVL_DELAY_MASK
    dsll    a2, a2, WRLVL_DELAY_6_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, WRLVL_DELAY_6_OFFSET
    or      a1, a1, a2
    sd      a1, WRLVL_DELAY_6_ADDR(t8)

    ld      a1, WRLVL_DELAY_5_ADDR(t8)
    dli     a2, WRLVL_DELAY_MASK
    dsll    a2, a2, WRLVL_DELAY_5_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, WRLVL_DELAY_5_OFFSET
    or      a1, a1, a2
    sd      a1, WRLVL_DELAY_5_ADDR(t8)

    ld      a1, WRLVL_DELAY_4_ADDR(t8)
    dli     a2, WRLVL_DELAY_MASK
    dsll    a2, a2, WRLVL_DELAY_4_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, WRLVL_DELAY_4_OFFSET
    or      a1, a1, a2
    sd      a1, WRLVL_DELAY_4_ADDR(t8)

    ld      a1, WRLVL_DELAY_3_ADDR(t8)
    dli     a2, WRLVL_DELAY_MASK
    dsll    a2, a2, WRLVL_DELAY_3_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, WRLVL_DELAY_3_OFFSET
    or      a1, a1, a2
    sd      a1, WRLVL_DELAY_3_ADDR(t8)

    ld      a1, WRLVL_DELAY_2_ADDR(t8)
    dli     a2, WRLVL_DELAY_MASK
    dsll    a2, a2, WRLVL_DELAY_2_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, WRLVL_DELAY_2_OFFSET
    or      a1, a1, a2
    sd      a1, WRLVL_DELAY_2_ADDR(t8)

    ld      a1, WRLVL_DELAY_1_ADDR(t8)
    dli     a2, WRLVL_DELAY_MASK
    dsll    a2, a2, WRLVL_DELAY_1_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, WRLVL_DELAY_1_OFFSET
    or      a1, a1, a2
    sd      a1, WRLVL_DELAY_1_ADDR(t8)

    ld      a1, WRLVL_DELAY_0_ADDR(t8)
    dli     a2, WRLVL_DELAY_MASK
    dsll    a2, a2, WRLVL_DELAY_0_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, WRLVL_DELAY_0_OFFSET
    or      a1, a1, a2
    sd      a1, WRLVL_DELAY_0_ADDR(t8)
    b       40f
    nop
6:
    and     t5, t5, WRLVL_DQ_DELAY_MASK

    ld      a1, WRLVL_DQ_DELAY_8_ADDR(t8)
    dli     a2, WRLVL_DQ_DELAY_MASK
    dsll    a2, a2, WRLVL_DQ_DELAY_8_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, WRLVL_DQ_DELAY_8_OFFSET
    or      a1, a1, a2
    sd      a1, WRLVL_DQ_DELAY_8_ADDR(t8)

    ld      a1, WRLVL_DQ_DELAY_7_ADDR(t8)
    dli     a2, WRLVL_DQ_DELAY_MASK
    dsll    a2, a2, WRLVL_DQ_DELAY_7_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, WRLVL_DQ_DELAY_7_OFFSET
    or      a1, a1, a2
    sd      a1, WRLVL_DQ_DELAY_7_ADDR(t8)

    ld      a1, WRLVL_DQ_DELAY_6_ADDR(t8)
    dli     a2, WRLVL_DQ_DELAY_MASK
    dsll    a2, a2, WRLVL_DQ_DELAY_6_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, WRLVL_DQ_DELAY_6_OFFSET
    or      a1, a1, a2
    sd      a1, WRLVL_DQ_DELAY_6_ADDR(t8)

    ld      a1, WRLVL_DQ_DELAY_5_ADDR(t8)
    dli     a2, WRLVL_DQ_DELAY_MASK
    dsll    a2, a2, WRLVL_DQ_DELAY_5_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, WRLVL_DQ_DELAY_5_OFFSET
    or      a1, a1, a2
    sd      a1, WRLVL_DQ_DELAY_5_ADDR(t8)

    ld      a1, WRLVL_DQ_DELAY_4_ADDR(t8)
    dli     a2, WRLVL_DQ_DELAY_MASK
    dsll    a2, a2, WRLVL_DQ_DELAY_4_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, WRLVL_DQ_DELAY_4_OFFSET
    or      a1, a1, a2
    sd      a1, WRLVL_DQ_DELAY_4_ADDR(t8)

    ld      a1, WRLVL_DQ_DELAY_3_ADDR(t8)
    dli     a2, WRLVL_DQ_DELAY_MASK
    dsll    a2, a2, WRLVL_DQ_DELAY_3_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, WRLVL_DQ_DELAY_3_OFFSET
    or      a1, a1, a2
    sd      a1, WRLVL_DQ_DELAY_3_ADDR(t8)

    ld      a1, WRLVL_DQ_DELAY_2_ADDR(t8)
    dli     a2, WRLVL_DQ_DELAY_MASK
    dsll    a2, a2, WRLVL_DQ_DELAY_2_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, WRLVL_DQ_DELAY_2_OFFSET
    or      a1, a1, a2
    sd      a1, WRLVL_DQ_DELAY_2_ADDR(t8)

    ld      a1, WRLVL_DQ_DELAY_1_ADDR(t8)
    dli     a2, WRLVL_DQ_DELAY_MASK
    dsll    a2, a2, WRLVL_DQ_DELAY_1_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, WRLVL_DQ_DELAY_1_OFFSET
    or      a1, a1, a2
    sd      a1, WRLVL_DQ_DELAY_1_ADDR(t8)

    ld      a1, WRLVL_DQ_DELAY_0_ADDR(t8)
    dli     a2, WRLVL_DQ_DELAY_MASK
    dsll    a2, a2, WRLVL_DQ_DELAY_0_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, WRLVL_DQ_DELAY_0_OFFSET
    or      a1, a1, a2
    sd      a1, WRLVL_DQ_DELAY_0_ADDR(t8)
    b       40f
    nop
7:
    and     t5, t5, RD_OE_EDGE_MASK

    ld      a1, RD_OE_EDGE_8_ADDR(t8)
    dli     a2, RD_OE_EDGE_MASK
    dsll    a2, a2, RD_OE_EDGE_8_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, RD_OE_EDGE_8_OFFSET
    or      a1, a1, a2
    sd      a1, RD_OE_EDGE_8_ADDR(t8)

    ld      a1, RD_OE_EDGE_7_ADDR(t8)
    dli     a2, RD_OE_EDGE_MASK
    dsll    a2, a2, RD_OE_EDGE_7_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, RD_OE_EDGE_7_OFFSET
    or      a1, a1, a2
    sd      a1, RD_OE_EDGE_7_ADDR(t8)

    ld      a1, RD_OE_EDGE_6_ADDR(t8)
    dli     a2, RD_OE_EDGE_MASK
    dsll    a2, a2, RD_OE_EDGE_6_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, RD_OE_EDGE_6_OFFSET
    or      a1, a1, a2
    sd      a1, RD_OE_EDGE_6_ADDR(t8)

    ld      a1, RD_OE_EDGE_5_ADDR(t8)
    dli     a2, RD_OE_EDGE_MASK
    dsll    a2, a2, RD_OE_EDGE_5_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, RD_OE_EDGE_5_OFFSET
    or      a1, a1, a2
    sd      a1, RD_OE_EDGE_5_ADDR(t8)

    ld      a1, RD_OE_EDGE_4_ADDR(t8)
    dli     a2, RD_OE_EDGE_MASK
    dsll    a2, a2, RD_OE_EDGE_4_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, RD_OE_EDGE_4_OFFSET
    or      a1, a1, a2
    sd      a1, RD_OE_EDGE_4_ADDR(t8)

    ld      a1, RD_OE_EDGE_3_ADDR(t8)
    dli     a2, RD_OE_EDGE_MASK
    dsll    a2, a2, RD_OE_EDGE_3_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, RD_OE_EDGE_3_OFFSET
    or      a1, a1, a2
    sd      a1, RD_OE_EDGE_3_ADDR(t8)

    ld      a1, RD_OE_EDGE_2_ADDR(t8)
    dli     a2, RD_OE_EDGE_MASK
    dsll    a2, a2, RD_OE_EDGE_2_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, RD_OE_EDGE_2_OFFSET
    or      a1, a1, a2
    sd      a1, RD_OE_EDGE_2_ADDR(t8)

    ld      a1, RD_OE_EDGE_1_ADDR(t8)
    dli     a2, RD_OE_EDGE_MASK
    dsll    a2, a2, RD_OE_EDGE_1_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, RD_OE_EDGE_1_OFFSET
    or      a1, a1, a2
    sd      a1, RD_OE_EDGE_1_ADDR(t8)

    ld      a1, RD_OE_EDGE_0_ADDR(t8)
    dli     a2, RD_OE_EDGE_MASK
    dsll    a2, a2, RD_OE_EDGE_0_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, RD_OE_EDGE_0_OFFSET
    or      a1, a1, a2
    sd      a1, RD_OE_EDGE_0_ADDR(t8)
    b       40f
    nop
8:
    and     t5, t5, WR_DQS_OE_EDGE_MASK

    ld      a1, WR_DQS_OE_EDGE_8_ADDR(t8)
    dli     a2, WR_DQS_OE_EDGE_MASK
    dsll    a2, a2, WR_DQS_OE_EDGE_8_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, WR_DQS_OE_EDGE_8_OFFSET
    or      a1, a1, a2
    sd      a1, WR_DQS_OE_EDGE_8_ADDR(t8)

    ld      a1, WR_DQS_OE_EDGE_7_ADDR(t8)
    dli     a2, WR_DQS_OE_EDGE_MASK
    dsll    a2, a2, WR_DQS_OE_EDGE_7_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, WR_DQS_OE_EDGE_7_OFFSET
    or      a1, a1, a2
    sd      a1, WR_DQS_OE_EDGE_7_ADDR(t8)

    ld      a1, WR_DQS_OE_EDGE_6_ADDR(t8)
    dli     a2, WR_DQS_OE_EDGE_MASK
    dsll    a2, a2, WR_DQS_OE_EDGE_6_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, WR_DQS_OE_EDGE_6_OFFSET
    or      a1, a1, a2
    sd      a1, WR_DQS_OE_EDGE_6_ADDR(t8)

    ld      a1, WR_DQS_OE_EDGE_5_ADDR(t8)
    dli     a2, WR_DQS_OE_EDGE_MASK
    dsll    a2, a2, WR_DQS_OE_EDGE_5_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, WR_DQS_OE_EDGE_5_OFFSET
    or      a1, a1, a2
    sd      a1, WR_DQS_OE_EDGE_5_ADDR(t8)

    ld      a1, WR_DQS_OE_EDGE_4_ADDR(t8)
    dli     a2, WR_DQS_OE_EDGE_MASK
    dsll    a2, a2, WR_DQS_OE_EDGE_4_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, WR_DQS_OE_EDGE_4_OFFSET
    or      a1, a1, a2
    sd      a1, WR_DQS_OE_EDGE_4_ADDR(t8)

    ld      a1, WR_DQS_OE_EDGE_3_ADDR(t8)
    dli     a2, WR_DQS_OE_EDGE_MASK
    dsll    a2, a2, WR_DQS_OE_EDGE_3_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, WR_DQS_OE_EDGE_3_OFFSET
    or      a1, a1, a2
    sd      a1, WR_DQS_OE_EDGE_3_ADDR(t8)

    ld      a1, WR_DQS_OE_EDGE_2_ADDR(t8)
    dli     a2, WR_DQS_OE_EDGE_MASK
    dsll    a2, a2, WR_DQS_OE_EDGE_2_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, WR_DQS_OE_EDGE_2_OFFSET
    or      a1, a1, a2
    sd      a1, WR_DQS_OE_EDGE_2_ADDR(t8)

    ld      a1, WR_DQS_OE_EDGE_1_ADDR(t8)
    dli     a2, WR_DQS_OE_EDGE_MASK
    dsll    a2, a2, WR_DQS_OE_EDGE_1_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, WR_DQS_OE_EDGE_1_OFFSET
    or      a1, a1, a2
    sd      a1, WR_DQS_OE_EDGE_1_ADDR(t8)

    ld      a1, WR_DQS_OE_EDGE_0_ADDR(t8)
    dli     a2, WR_DQS_OE_EDGE_MASK
    dsll    a2, a2, WR_DQS_OE_EDGE_0_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, WR_DQS_OE_EDGE_0_OFFSET
    or      a1, a1, a2
    sd      a1, WR_DQS_OE_EDGE_0_ADDR(t8)
    b       40f
    nop
9:
    and     t5, t5, WR_DQ_OE_EDGE_MASK

    ld      a1, WR_DQ_OE_EDGE_8_ADDR(t8)
    dli     a2, WR_DQ_OE_EDGE_MASK
    dsll    a2, a2, WR_DQ_OE_EDGE_8_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, WR_DQ_OE_EDGE_8_OFFSET
    or      a1, a1, a2
    sd      a1, WR_DQ_OE_EDGE_8_ADDR(t8)

    ld      a1, WR_DQ_OE_EDGE_7_ADDR(t8)
    dli     a2, WR_DQ_OE_EDGE_MASK
    dsll    a2, a2, WR_DQ_OE_EDGE_7_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, WR_DQ_OE_EDGE_7_OFFSET
    or      a1, a1, a2
    sd      a1, WR_DQ_OE_EDGE_7_ADDR(t8)

    ld      a1, WR_DQ_OE_EDGE_6_ADDR(t8)
    dli     a2, WR_DQ_OE_EDGE_MASK
    dsll    a2, a2, WR_DQ_OE_EDGE_6_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, WR_DQ_OE_EDGE_6_OFFSET
    or      a1, a1, a2
    sd      a1, WR_DQ_OE_EDGE_6_ADDR(t8)

    ld      a1, WR_DQ_OE_EDGE_5_ADDR(t8)
    dli     a2, WR_DQ_OE_EDGE_MASK
    dsll    a2, a2, WR_DQ_OE_EDGE_5_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, WR_DQ_OE_EDGE_5_OFFSET
    or      a1, a1, a2
    sd      a1, WR_DQ_OE_EDGE_5_ADDR(t8)

    ld      a1, WR_DQ_OE_EDGE_4_ADDR(t8)
    dli     a2, WR_DQ_OE_EDGE_MASK
    dsll    a2, a2, WR_DQ_OE_EDGE_4_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, WR_DQ_OE_EDGE_4_OFFSET
    or      a1, a1, a2
    sd      a1, WR_DQ_OE_EDGE_4_ADDR(t8)

    ld      a1, WR_DQ_OE_EDGE_3_ADDR(t8)
    dli     a2, WR_DQ_OE_EDGE_MASK
    dsll    a2, a2, WR_DQ_OE_EDGE_3_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, WR_DQ_OE_EDGE_3_OFFSET
    or      a1, a1, a2
    sd      a1, WR_DQ_OE_EDGE_3_ADDR(t8)

    ld      a1, WR_DQ_OE_EDGE_2_ADDR(t8)
    dli     a2, WR_DQ_OE_EDGE_MASK
    dsll    a2, a2, WR_DQ_OE_EDGE_2_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, WR_DQ_OE_EDGE_2_OFFSET
    or      a1, a1, a2
    sd      a1, WR_DQ_OE_EDGE_2_ADDR(t8)

    ld      a1, WR_DQ_OE_EDGE_1_ADDR(t8)
    dli     a2, WR_DQ_OE_EDGE_MASK
    dsll    a2, a2, WR_DQ_OE_EDGE_1_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, WR_DQ_OE_EDGE_1_OFFSET
    or      a1, a1, a2
    sd      a1, WR_DQ_OE_EDGE_1_ADDR(t8)

    ld      a1, WR_DQ_OE_EDGE_0_ADDR(t8)
    dli     a2, WR_DQ_OE_EDGE_MASK
    dsll    a2, a2, WR_DQ_OE_EDGE_0_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, WR_DQ_OE_EDGE_0_OFFSET
    or      a1, a1, a2
    sd      a1, WR_DQ_OE_EDGE_0_ADDR(t8)
    b       40f
    nop
10:
    and     t5, t5, WR_ODT_OE_EDGE_MASK

    ld      a1, WR_ODT_OE_EDGE_8_ADDR(t8)
    dli     a2, WR_ODT_OE_EDGE_MASK
    dsll    a2, a2, WR_ODT_OE_EDGE_8_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, WR_ODT_OE_EDGE_8_OFFSET
    or      a1, a1, a2
    sd      a1, WR_ODT_OE_EDGE_8_ADDR(t8)

    ld      a1, WR_ODT_OE_EDGE_7_ADDR(t8)
    dli     a2, WR_ODT_OE_EDGE_MASK
    dsll    a2, a2, WR_ODT_OE_EDGE_7_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, WR_ODT_OE_EDGE_7_OFFSET
    or      a1, a1, a2
    sd      a1, WR_ODT_OE_EDGE_7_ADDR(t8)

    ld      a1, WR_ODT_OE_EDGE_6_ADDR(t8)
    dli     a2, WR_ODT_OE_EDGE_MASK
    dsll    a2, a2, WR_ODT_OE_EDGE_6_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, WR_ODT_OE_EDGE_6_OFFSET
    or      a1, a1, a2
    sd      a1, WR_ODT_OE_EDGE_6_ADDR(t8)

    ld      a1, WR_ODT_OE_EDGE_5_ADDR(t8)
    dli     a2, WR_ODT_OE_EDGE_MASK
    dsll    a2, a2, WR_ODT_OE_EDGE_5_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, WR_ODT_OE_EDGE_5_OFFSET
    or      a1, a1, a2
    sd      a1, WR_ODT_OE_EDGE_5_ADDR(t8)

    ld      a1, WR_ODT_OE_EDGE_4_ADDR(t8)
    dli     a2, WR_ODT_OE_EDGE_MASK
    dsll    a2, a2, WR_ODT_OE_EDGE_4_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, WR_ODT_OE_EDGE_4_OFFSET
    or      a1, a1, a2
    sd      a1, WR_ODT_OE_EDGE_4_ADDR(t8)

    ld      a1, WR_ODT_OE_EDGE_3_ADDR(t8)
    dli     a2, WR_ODT_OE_EDGE_MASK
    dsll    a2, a2, WR_ODT_OE_EDGE_3_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, WR_ODT_OE_EDGE_3_OFFSET
    or      a1, a1, a2
    sd      a1, WR_ODT_OE_EDGE_3_ADDR(t8)

    ld      a1, WR_ODT_OE_EDGE_2_ADDR(t8)
    dli     a2, WR_ODT_OE_EDGE_MASK
    dsll    a2, a2, WR_ODT_OE_EDGE_2_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, WR_ODT_OE_EDGE_2_OFFSET
    or      a1, a1, a2
    sd      a1, WR_ODT_OE_EDGE_2_ADDR(t8)

    ld      a1, WR_ODT_OE_EDGE_1_ADDR(t8)
    dli     a2, WR_ODT_OE_EDGE_MASK
    dsll    a2, a2, WR_ODT_OE_EDGE_1_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, WR_ODT_OE_EDGE_1_OFFSET
    or      a1, a1, a2
    sd      a1, WR_ODT_OE_EDGE_1_ADDR(t8)

    ld      a1, WR_ODT_OE_EDGE_0_ADDR(t8)
    dli     a2, WR_ODT_OE_EDGE_MASK
    dsll    a2, a2, WR_ODT_OE_EDGE_0_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, WR_ODT_OE_EDGE_0_OFFSET
    or      a1, a1, a2
    sd      a1, WR_ODT_OE_EDGE_0_ADDR(t8)
    b       40f
    nop
11:
    and     t5, t5, WRDQ_CLK_DELAY_MASK

    ld      a1, WRDQ_CLK_DELAY_8_ADDR(t8)
    dli     a2, WRDQ_CLK_DELAY_MASK
    dsll    a2, a2, WRDQ_CLK_DELAY_8_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, WRDQ_CLK_DELAY_8_OFFSET
    or      a1, a1, a2
    sd      a1, WRDQ_CLK_DELAY_8_ADDR(t8)

    ld      a1, WRDQ_CLK_DELAY_7_ADDR(t8)
    dli     a2, WRDQ_CLK_DELAY_MASK
    dsll    a2, a2, WRDQ_CLK_DELAY_7_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, WRDQ_CLK_DELAY_7_OFFSET
    or      a1, a1, a2
    sd      a1, WRDQ_CLK_DELAY_7_ADDR(t8)

    ld      a1, WRDQ_CLK_DELAY_6_ADDR(t8)
    dli     a2, WRDQ_CLK_DELAY_MASK
    dsll    a2, a2, WRDQ_CLK_DELAY_6_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, WRDQ_CLK_DELAY_6_OFFSET
    or      a1, a1, a2
    sd      a1, WRDQ_CLK_DELAY_6_ADDR(t8)

    ld      a1, WRDQ_CLK_DELAY_5_ADDR(t8)
    dli     a2, WRDQ_CLK_DELAY_MASK
    dsll    a2, a2, WRDQ_CLK_DELAY_5_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, WRDQ_CLK_DELAY_5_OFFSET
    or      a1, a1, a2
    sd      a1, WRDQ_CLK_DELAY_5_ADDR(t8)

    ld      a1, WRDQ_CLK_DELAY_4_ADDR(t8)
    dli     a2, WRDQ_CLK_DELAY_MASK
    dsll    a2, a2, WRDQ_CLK_DELAY_4_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, WRDQ_CLK_DELAY_4_OFFSET
    or      a1, a1, a2
    sd      a1, WRDQ_CLK_DELAY_4_ADDR(t8)

    ld      a1, WRDQ_CLK_DELAY_3_ADDR(t8)
    dli     a2, WRDQ_CLK_DELAY_MASK
    dsll    a2, a2, WRDQ_CLK_DELAY_3_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, WRDQ_CLK_DELAY_3_OFFSET
    or      a1, a1, a2
    sd      a1, WRDQ_CLK_DELAY_3_ADDR(t8)

    ld      a1, WRDQ_CLK_DELAY_2_ADDR(t8)
    dli     a2, WRDQ_CLK_DELAY_MASK
    dsll    a2, a2, WRDQ_CLK_DELAY_2_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, WRDQ_CLK_DELAY_2_OFFSET
    or      a1, a1, a2
    sd      a1, WRDQ_CLK_DELAY_2_ADDR(t8)

    ld      a1, WRDQ_CLK_DELAY_1_ADDR(t8)
    dli     a2, WRDQ_CLK_DELAY_MASK
    dsll    a2, a2, WRDQ_CLK_DELAY_1_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, WRDQ_CLK_DELAY_1_OFFSET
    or      a1, a1, a2
    sd      a1, WRDQ_CLK_DELAY_1_ADDR(t8)

    ld      a1, WRDQ_CLK_DELAY_0_ADDR(t8)
    dli     a2, WRDQ_CLK_DELAY_MASK
    dsll    a2, a2, WRDQ_CLK_DELAY_0_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, WRDQ_CLK_DELAY_0_OFFSET
    or      a1, a1, a2
    sd      a1, WRDQ_CLK_DELAY_0_ADDR(t8)
    b       40f
    nop
12:
    and     t5, t5, RDDATA_DELAY_MASK

    ld      a1, RDDATA_DELAY_8_ADDR(t8)
    dli     a2, RDDATA_DELAY_MASK
    dsll    a2, a2, RDDATA_DELAY_8_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, RDDATA_DELAY_8_OFFSET
    or      a1, a1, a2
    sd      a1, RDDATA_DELAY_8_ADDR(t8)

    ld      a1, RDDATA_DELAY_7_ADDR(t8)
    dli     a2, RDDATA_DELAY_MASK
    dsll    a2, a2, RDDATA_DELAY_7_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, RDDATA_DELAY_7_OFFSET
    or      a1, a1, a2
    sd      a1, RDDATA_DELAY_7_ADDR(t8)

    ld      a1, RDDATA_DELAY_6_ADDR(t8)
    dli     a2, RDDATA_DELAY_MASK
    dsll    a2, a2, RDDATA_DELAY_6_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, RDDATA_DELAY_6_OFFSET
    or      a1, a1, a2
    sd      a1, RDDATA_DELAY_6_ADDR(t8)

    ld      a1, RDDATA_DELAY_5_ADDR(t8)
    dli     a2, RDDATA_DELAY_MASK
    dsll    a2, a2, RDDATA_DELAY_5_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, RDDATA_DELAY_5_OFFSET
    or      a1, a1, a2
    sd      a1, RDDATA_DELAY_5_ADDR(t8)

    ld      a1, RDDATA_DELAY_4_ADDR(t8)
    dli     a2, RDDATA_DELAY_MASK
    dsll    a2, a2, RDDATA_DELAY_4_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, RDDATA_DELAY_4_OFFSET
    or      a1, a1, a2
    sd      a1, RDDATA_DELAY_4_ADDR(t8)

    ld      a1, RDDATA_DELAY_3_ADDR(t8)
    dli     a2, RDDATA_DELAY_MASK
    dsll    a2, a2, RDDATA_DELAY_3_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, RDDATA_DELAY_3_OFFSET
    or      a1, a1, a2
    sd      a1, RDDATA_DELAY_3_ADDR(t8)

    ld      a1, RDDATA_DELAY_2_ADDR(t8)
    dli     a2, RDDATA_DELAY_MASK
    dsll    a2, a2, RDDATA_DELAY_2_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, RDDATA_DELAY_2_OFFSET
    or      a1, a1, a2
    sd      a1, RDDATA_DELAY_2_ADDR(t8)

    ld      a1, RDDATA_DELAY_1_ADDR(t8)
    dli     a2, RDDATA_DELAY_MASK
    dsll    a2, a2, RDDATA_DELAY_1_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, RDDATA_DELAY_1_OFFSET
    or      a1, a1, a2
    sd      a1, RDDATA_DELAY_1_ADDR(t8)

    ld      a1, RDDATA_DELAY_0_ADDR(t8)
    dli     a2, RDDATA_DELAY_MASK
    dsll    a2, a2, RDDATA_DELAY_0_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, RDDATA_DELAY_0_OFFSET
    or      a1, a1, a2
    sd      a1, RDDATA_DELAY_0_ADDR(t8)
    b       40f
    nop
13:
    and     t5, t5, RDDQS_LT_HALF_MASK

    ld      a1, RDDQS_LT_HALF_8_ADDR(t8)
    dli     a2, RDDQS_LT_HALF_MASK
    dsll    a2, a2, RDDQS_LT_HALF_8_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, RDDQS_LT_HALF_8_OFFSET
    or      a1, a1, a2
    sd      a1, RDDQS_LT_HALF_8_ADDR(t8)

    ld      a1, RDDQS_LT_HALF_7_ADDR(t8)
    dli     a2, RDDQS_LT_HALF_MASK
    dsll    a2, a2, RDDQS_LT_HALF_7_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, RDDQS_LT_HALF_7_OFFSET
    or      a1, a1, a2
    sd      a1, RDDQS_LT_HALF_7_ADDR(t8)

    ld      a1, RDDQS_LT_HALF_6_ADDR(t8)
    dli     a2, RDDQS_LT_HALF_MASK
    dsll    a2, a2, RDDQS_LT_HALF_6_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, RDDQS_LT_HALF_6_OFFSET
    or      a1, a1, a2
    sd      a1, RDDQS_LT_HALF_6_ADDR(t8)

    ld      a1, RDDQS_LT_HALF_5_ADDR(t8)
    dli     a2, RDDQS_LT_HALF_MASK
    dsll    a2, a2, RDDQS_LT_HALF_5_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, RDDQS_LT_HALF_5_OFFSET
    or      a1, a1, a2
    sd      a1, RDDQS_LT_HALF_5_ADDR(t8)

    ld      a1, RDDQS_LT_HALF_4_ADDR(t8)
    dli     a2, RDDQS_LT_HALF_MASK
    dsll    a2, a2, RDDQS_LT_HALF_4_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, RDDQS_LT_HALF_4_OFFSET
    or      a1, a1, a2
    sd      a1, RDDQS_LT_HALF_4_ADDR(t8)

    ld      a1, RDDQS_LT_HALF_3_ADDR(t8)
    dli     a2, RDDQS_LT_HALF_MASK
    dsll    a2, a2, RDDQS_LT_HALF_3_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, RDDQS_LT_HALF_3_OFFSET
    or      a1, a1, a2
    sd      a1, RDDQS_LT_HALF_3_ADDR(t8)

    ld      a1, RDDQS_LT_HALF_2_ADDR(t8)
    dli     a2, RDDQS_LT_HALF_MASK
    dsll    a2, a2, RDDQS_LT_HALF_2_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, RDDQS_LT_HALF_2_OFFSET
    or      a1, a1, a2
    sd      a1, RDDQS_LT_HALF_2_ADDR(t8)

    ld      a1, RDDQS_LT_HALF_1_ADDR(t8)
    dli     a2, RDDQS_LT_HALF_MASK
    dsll    a2, a2, RDDQS_LT_HALF_1_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, RDDQS_LT_HALF_1_OFFSET
    or      a1, a1, a2
    sd      a1, RDDQS_LT_HALF_1_ADDR(t8)

    ld      a1, RDDQS_LT_HALF_0_ADDR(t8)
    dli     a2, RDDQS_LT_HALF_MASK
    dsll    a2, a2, RDDQS_LT_HALF_0_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, RDDQS_LT_HALF_0_OFFSET
    or      a1, a1, a2
    sd      a1, RDDQS_LT_HALF_0_ADDR(t8)
    b       40f
    nop
14:
    and     t5, t5, WRDQS_LT_HALF_MASK

    ld      a1, WRDQS_LT_HALF_8_ADDR(t8)
    dli     a2, WRDQS_LT_HALF_MASK
    dsll    a2, a2, WRDQS_LT_HALF_8_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, WRDQS_LT_HALF_8_OFFSET
    or      a1, a1, a2
    sd      a1, WRDQS_LT_HALF_8_ADDR(t8)

    ld      a1, WRDQS_LT_HALF_7_ADDR(t8)
    dli     a2, WRDQS_LT_HALF_MASK
    dsll    a2, a2, WRDQS_LT_HALF_7_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, WRDQS_LT_HALF_7_OFFSET
    or      a1, a1, a2
    sd      a1, WRDQS_LT_HALF_7_ADDR(t8)

    ld      a1, WRDQS_LT_HALF_6_ADDR(t8)
    dli     a2, WRDQS_LT_HALF_MASK
    dsll    a2, a2, WRDQS_LT_HALF_6_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, WRDQS_LT_HALF_6_OFFSET
    or      a1, a1, a2
    sd      a1, WRDQS_LT_HALF_6_ADDR(t8)

    ld      a1, WRDQS_LT_HALF_5_ADDR(t8)
    dli     a2, WRDQS_LT_HALF_MASK
    dsll    a2, a2, WRDQS_LT_HALF_5_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, WRDQS_LT_HALF_5_OFFSET
    or      a1, a1, a2
    sd      a1, WRDQS_LT_HALF_5_ADDR(t8)

    ld      a1, WRDQS_LT_HALF_4_ADDR(t8)
    dli     a2, WRDQS_LT_HALF_MASK
    dsll    a2, a2, WRDQS_LT_HALF_4_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, WRDQS_LT_HALF_4_OFFSET
    or      a1, a1, a2
    sd      a1, WRDQS_LT_HALF_4_ADDR(t8)

    ld      a1, WRDQS_LT_HALF_3_ADDR(t8)
    dli     a2, WRDQS_LT_HALF_MASK
    dsll    a2, a2, WRDQS_LT_HALF_3_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, WRDQS_LT_HALF_3_OFFSET
    or      a1, a1, a2
    sd      a1, WRDQS_LT_HALF_3_ADDR(t8)

    ld      a1, WRDQS_LT_HALF_2_ADDR(t8)
    dli     a2, WRDQS_LT_HALF_MASK
    dsll    a2, a2, WRDQS_LT_HALF_2_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, WRDQS_LT_HALF_2_OFFSET
    or      a1, a1, a2
    sd      a1, WRDQS_LT_HALF_2_ADDR(t8)

    ld      a1, WRDQS_LT_HALF_1_ADDR(t8)
    dli     a2, WRDQS_LT_HALF_MASK
    dsll    a2, a2, WRDQS_LT_HALF_1_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, WRDQS_LT_HALF_1_OFFSET
    or      a1, a1, a2
    sd      a1, WRDQS_LT_HALF_1_ADDR(t8)

    ld      a1, WRDQS_LT_HALF_0_ADDR(t8)
    dli     a2, WRDQS_LT_HALF_MASK
    dsll    a2, a2, WRDQS_LT_HALF_0_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, WRDQS_LT_HALF_0_OFFSET
    or      a1, a1, a2
    sd      a1, WRDQS_LT_HALF_0_ADDR(t8)
    b       40f
    nop
15:
    and     t5, t5, WRDQ_LT_HALF_MASK

    ld      a1, WRDQ_LT_HALF_8_ADDR(t8)
    dli     a2, WRDQ_LT_HALF_MASK
    dsll    a2, a2, WRDQ_LT_HALF_8_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, WRDQ_LT_HALF_8_OFFSET
    or      a1, a1, a2
    sd      a1, WRDQ_LT_HALF_8_ADDR(t8)

    ld      a1, WRDQ_LT_HALF_7_ADDR(t8)
    dli     a2, WRDQ_LT_HALF_MASK
    dsll    a2, a2, WRDQ_LT_HALF_7_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, WRDQ_LT_HALF_7_OFFSET
    or      a1, a1, a2
    sd      a1, WRDQ_LT_HALF_7_ADDR(t8)

    ld      a1, WRDQ_LT_HALF_6_ADDR(t8)
    dli     a2, WRDQ_LT_HALF_MASK
    dsll    a2, a2, WRDQ_LT_HALF_6_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, WRDQ_LT_HALF_6_OFFSET
    or      a1, a1, a2
    sd      a1, WRDQ_LT_HALF_6_ADDR(t8)

    ld      a1, WRDQ_LT_HALF_5_ADDR(t8)
    dli     a2, WRDQ_LT_HALF_MASK
    dsll    a2, a2, WRDQ_LT_HALF_5_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, WRDQ_LT_HALF_5_OFFSET
    or      a1, a1, a2
    sd      a1, WRDQ_LT_HALF_5_ADDR(t8)

    ld      a1, WRDQ_LT_HALF_4_ADDR(t8)
    dli     a2, WRDQ_LT_HALF_MASK
    dsll    a2, a2, WRDQ_LT_HALF_4_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, WRDQ_LT_HALF_4_OFFSET
    or      a1, a1, a2
    sd      a1, WRDQ_LT_HALF_4_ADDR(t8)

    ld      a1, WRDQ_LT_HALF_3_ADDR(t8)
    dli     a2, WRDQ_LT_HALF_MASK
    dsll    a2, a2, WRDQ_LT_HALF_3_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, WRDQ_LT_HALF_3_OFFSET
    or      a1, a1, a2
    sd      a1, WRDQ_LT_HALF_3_ADDR(t8)

    ld      a1, WRDQ_LT_HALF_2_ADDR(t8)
    dli     a2, WRDQ_LT_HALF_MASK
    dsll    a2, a2, WRDQ_LT_HALF_2_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, WRDQ_LT_HALF_2_OFFSET
    or      a1, a1, a2
    sd      a1, WRDQ_LT_HALF_2_ADDR(t8)

    ld      a1, WRDQ_LT_HALF_1_ADDR(t8)
    dli     a2, WRDQ_LT_HALF_MASK
    dsll    a2, a2, WRDQ_LT_HALF_1_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, WRDQ_LT_HALF_1_OFFSET
    or      a1, a1, a2
    sd      a1, WRDQ_LT_HALF_1_ADDR(t8)

    ld      a1, WRDQ_LT_HALF_0_ADDR(t8)
    dli     a2, WRDQ_LT_HALF_MASK
    dsll    a2, a2, WRDQ_LT_HALF_0_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t5, WRDQ_LT_HALF_0_OFFSET
    or      a1, a1, a2
    sd      a1, WRDQ_LT_HALF_0_ADDR(t8)
    b       40f
    nop
40:
    sync
    b        41b
    nop

90:

    PRINTSTR("\r\nChange some parameters of MC:");
1:
    PRINTSTR("\r\nPlease input the register number you want to change!!!(0xfff:jump out.): ");
    dli     t6, 0x00
    bal     inputaddress
    nop
    move    t5, v0
    
    dli     a1, DDR_PARAM_NUM
    dsll    a1, a1, 3
    bge     t5, a1, 2f    #if input address offset exceed range,jump out
    nop
    and     t5, t5, 0xff8
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

    b       1b
    nop
2:    
    /* recover the ra */
    move    ra, t1
#endif
    sync

    ############start##########
    /***** set start to 1,start to initialize SDRAM *****/
    daddiu  v0, t8, START_ADDR
    dli     a2, 0x1
    dsll    a2, a2, START_OFFSET
    ld      a1, 0x0(v0)
    or      a1, a1, a2
    sd      a1, 0x0(v0)
    sync

wait_dram_init:
    //wait initialization complete 
    //delay
    dli     v0, 0x100
1:
    daddi   v0, v0, -1
    bnez    v0, 1b
    nop  

#if 1
    /* store the ra */
    move    t1, ra

    PRINTSTR("run to wait dram init ok!3\r\n")

    /* recover the ra */
    move    ra, t1
#endif

    daddiu  v0, t8, DRAM_INIT_ADDR
    GET_MC_CS_MAP
1:
    ld      a2, 0x0(v0)
    dsrl    a2, a2, DRAM_INIT_OFFSET
    and     a2, a2, a1
    //bne     a1, a2, 1b
    bne     a1, a2, wait_dram_init
    nop

ddr2_config_end:
    jr      ra
    nop
    .end    ddr2_config
