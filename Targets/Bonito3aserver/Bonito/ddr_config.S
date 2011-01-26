/**********************************
    changed by cxk
    v1.0 support both 3A and 3A3 according macro defination--loongson3A3
    a0: input, Memory Controller config register base
**********************************/
#define REG_ADDRESS 0x0
#define CONFIG_BASE 0x900000000ff00000

        .global ddr2_config
        .ent    ddr2_config
        .set    noreorder
        .set    mips3
ddr2_config:
    GET_SDRAM_TYPE
    dli     t1, 0x2
    beq     t1, a1, 2f
    nop
    dli     t1, 0x3
    beq     t1, a1, 3f
    nop
    //not DDR2 and not DDR3, errors
    move    t1, ra
    PRINTSTR("\r\n!!!! NOT recognized DDR SDRAM TYPE. MC0 parameters will not be written!!!!\r\n");
    move    ra, t1
    b       ddr2_config_end
    nop
2:  //DDR2
    GET_DIMM_TYPE
    bnez    a1, 1f
    nop
    //UDIMM
    dla     a2, ddr2_reg_data
    b       4f
    nop
1:  //RDIMM
    dla     a2, ddr2_RDIMM_reg_data
    b       4f
    nop
3:  //DDR3
    GET_DIMM_TYPE
    bnez    a1, 1f
    nop
    //UDIMM
    dla     a2, ddr3_reg_data
    b       4f
    nop
1:  //RDIMM
    dla     a2, ddr3_RDIMM_reg_data
    b       4f
    nop
4:

    daddu   a2, a2, s0
#ifdef  loongson3A3
    dli     t1, 180
#else
    dli     t1, 152
#endif
    daddiu	v0, a0, 0x0

reg_write:
    ld      a1, 0x0(a2)
    sd      a1, REG_ADDRESS(v0)

    daddiu  t1, t1, -1
    daddiu  a2, a2, 0x8
    bne     t1, $0, reg_write
    daddiu  v0, v0, 0x10

    //set v0 to MC configure register base
    daddiu	v0, a0, 0x0
    //rewrite eight_bank_mode
    ld      a2, 0x10(v0)
    dli     a1, 0xffffff00ffffffff
    and     a2, a2, a1
    GET_EIGHT_BANK
    dsll    a1, a1, EIGHT_BANK_MODE_OFFSET
    or      a2, a2, a1
    sd      a2, 0x10(v0)
    //rewrite column_size and addr_pins
    ld      a2, 0x50(v0)
    dli     a1, 0xffffffff00ff00ff
    and     a2, a2, a1
    GET_ROW_SIZE
    dsll    a1, a1, ADDR_PINS_OFFSET
    or      a2, a2, a1
    GET_COL_SIZE
    dsll    a1, a1, COLUMN_SIZE_OFFSET
    or      a2, a2, a1
    sd      a2, 0x50(v0)
    //rewrite cs_map
    ld      a2, 0x70(v0)
    dli     a1, 0xffffffffff00ffff
    and     a2, a2, a1
    GET_MC_CS_MAP
    dsll    a1, a1, CS_MAP_OFFSET
    or      a2, a2, a1
    sd      a2, 0x70(v0)
    //rewrite ECC control ctrl_raw
    //not finished now

#ifdef MY_DEBUG_DDR   //debug
    /* store the a0,ra */
	move	t8, a0
	move	t1, ra

    PRINTSTR("\r\nChange some parameters of MC0:");
1:
	PRINTSTR("\r\nPlease input the register number you want to change!!!(0:jump out.): ");
	dli  	t6, 0x00
	bal	    inputaddress	#input value stored in v0
	nop
	move	t5, v0	#store v0 to t5
	
	beqz	t5, 2f	#if input 0,jump out
	nop
	dsll	t5, t5, 4	#t5 is the offset relative to a0
	daddu	t5, t5, t8

	PRINTSTR("\r\nPlease input the data-hex: ");
	dli	    t6, 0x00
	bal	    inputaddress	#input value stored in v0
	nop
	sd	    v0, 0x0(t5)	#v0 is the input value

    //print the new register value
    move    t6, t5
    PRINTSTR("\r\nRegister 0x")
    dsubu   t5, t5, t8
    dsrl    t5, t5, 4
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

	b	    1b
	nop
2:	
    /* recover the a0,ra */
	move	a0, t8
	move	ra, t1
#endif

    ############start##########
    /***** set start to 1,start to initialize SDRAM *****/
    daddiu	v0, a0, 0x30
    dli     a2, 0x0000010000000000
    ld      a1, 0x0(v0)
    or      a1, a1, a2
    sd      a1, 0x0(v0)

    daddiu  v0, a0, 0x960 //wait initialization complete 
1:
    ld      a1, 0x0(v0)
    andi    a1, a1, 0x100
    beqz    a1, 1b
    nop

//seems no need to do the following step
/** delay some time **/
    dli     a2, 0xfff
1:
    bnez    a2, 1b
    daddiu  a2, a2, -1
    nop

    daddiu  v0, a0, 0x40 //write SDRAM mode register
    dli     a2, 0x0000000001000000
    ld      a1, 0x0(v0)
    or      a1, a1, a2
    sd      a1, 0x0(v0)
    nop
    sync

/** delay some time **/
    dli     a2, 0xfff
1:
    bnez    a2, 1b
    daddiu  a2, a2, -1
    nop
    ###############################
ddr2_config_end:
    jr      ra
    nop
    .end    ddr2_config


        .global ddr2_config_mc1
        .ent    ddr2_config_mc1
        .set    noreorder
        .set    mips3
ddr2_config_mc1:
    GET_SDRAM_TYPE
    dli     t1, 0x2
    beq     t1, a1, 2f
    nop
    dli     t1, 0x3
    beq     t1, a1, 3f
    nop
    //not DDR2 and not DDR3, errors
    move    t1, ra
    PRINTSTR("\r\n!!!! NOT recognized DDR SDRAM TYPE. MC0 parameters will not be written!!!!\r\n");
    move    ra, t1
    b       ddr2_config_mc1_end
    nop
2:  //DDR2
    GET_DIMM_TYPE
    bnez    a1, 1f
    nop
    //UDIMM
    dla     a2, ddr2_reg_data_mc1
    b       4f
    nop
1:  //RDIMM
    dla     a2, ddr2_RDIMM_reg_data_mc1
    b       4f
    nop
3:  //DDR3
    GET_DIMM_TYPE
    bnez    a1, 1f
    nop
    //UDIMM
    dla     a2, ddr3_reg_data_mc1
    b       4f
    nop
1:  //RDIMM
    dla     a2, ddr3_RDIMM_reg_data_mc1
    b       4f
    nop
4:

    daddu   a2, a2, s0
#ifdef  loongson3A3
    dli     t1, 180
#else
    dli     t1, 152
#endif
    daddiu	v0, a0, 0x0

reg_write_mc1:

    ld      a1, 0x0(a2)
    sd      a1, REG_ADDRESS(v0)

    daddiu  t1, t1, -1
    daddiu  a2, a2, 0x8
    bne     t1, $0, reg_write_mc1
    daddiu  v0, v0, 0x10

    //set v0 to MC configure register base
    daddiu	v0, a0, 0x0
    //rewrite eight_bank_mode
    ld      a2, 0x10(v0)
    dli     a1, 0xffffff00ffffffff
    and     a2, a2, a1
    GET_EIGHT_BANK
    dsll    a1, a1, EIGHT_BANK_MODE_OFFSET
    or      a2, a2, a1
    sd      a2, 0x10(v0)
    //rewrite column_size and addr_pins
    ld      a2, 0x50(v0)
    dli     a1, 0xffffffff00ff00ff
    and     a2, a2, a1
    GET_ROW_SIZE
    dsll    a1, a1, ADDR_PINS_OFFSET
    or      a2, a2, a1
    GET_COL_SIZE
    dsll    a1, a1, COLUMN_SIZE_OFFSET
    or      a2, a2, a1
    sd      a2, 0x50(v0)
    //rewrite cs_map
    ld      a2, 0x70(v0)
    dli     a1, 0xffffffffff00ffff
    and     a2, a2, a1
    GET_MC_CS_MAP
    dsll    a1, a1, CS_MAP_OFFSET
    or      a2, a2, a1
    sd      a2, 0x70(v0)
    //rewrite ECC control ctrl_raw
    //not finished now

#ifdef MY_DEBUG_DDR   //debug
    /* store the a0,ra */
	move	t8, a0
	move	t1, ra

    PRINTSTR("\r\nChange some parameters of MC1:");
1:
	PRINTSTR("\r\nPlease input the register number you want to change!!!(0:jump out.): ");
	dli	    t6, 0x00
	bal	    inputaddress	#input value stored in v0
	nop
	move	t5, v0	#store v0 to t5
	
	beqz	t5, 2f	#if input 0,jump out
	nop
	dsll	t5, t5, 4	#t5 is the offset relative to a0
	daddu	t5, t5, t8

	PRINTSTR("\r\nPlease input the data-hex: ");
	dli 	t6, 0x00
	bal	    inputaddress	#input value stored in v0
	nop
	sd	    v0, 0x0(t5)	#v0 is the input value

    //print the new register value
    move    t6, t5
    PRINTSTR("\r\nRegister 0x")
    dsubu   t5, t5, t8
    dsrl    t5, t5, 4
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

	b	    1b
	nop
2:	
    /* recover the a0,ra */
	move	a0, t8
	move	ra, t1
#endif

    ############start##########
    /***** set start to 1,start to initialize SDRAM *****/
    daddiu	v0, a0, 0x30
    dli     a2, 0x0000010000000000
    ld      a1, 0x0(v0)
    or      a1, a1, a2
    sd      a1, 0x0(v0)

    daddiu  v0, a0, 0x960 //wait initialization complete 
1:
    ld      a1, 0x0(v0)
    andi    a1, a1, 0x100
    beqz    a1, 1b
    nop

//seems no need to do the following step
/** delay some time **/
    dli     a2, 0xfff
1:
    bnez    a2, 1b
    daddiu  a2, a2, -1
    nop

    daddiu  v0, a0, 0x40 //write SDRAM mode register
    dli     a2, 0x0000000001000000
    ld      a1, 0x0(v0)
    or      a1, a1, a2
    sd      a1, 0x0(v0)
    nop
    sync

/** delay some time **/
    dli     a2, 0xfff
1:
    bnez    a2, 1b
    daddiu  a2, a2, -1
    nop
    ###############################
ddr2_config_mc1_end:
    jr      ra
    nop
    .end    ddr2_config_mc1


	.rdata
	.align 5
    .global ddr2_reg_data

#ifdef  loongson3A3
#include "loongson3A3_ddr2_param.S"
#include "loongson3A3_ddr3_param.S"
#else
#include "loongson3A_ddr2_param.S"
#include "loongson3A_ddr3_param.S"
#endif
