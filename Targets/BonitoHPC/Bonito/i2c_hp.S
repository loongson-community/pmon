#if 0
//	b	211f
//#if 1
//	li	msize,0
//	PRINTSTR("DIMM read\r\n")

	/* only one memory slot, slave address is 1010000b */
/****************************************************/
#if	1
	PRINTSTR("\nST7FOXF1 TEST..........\n")
	li  	a1, 0x1
	li		a0, 0xa0
	bal     si2cread
	nop
	move	a0, v0
	bal		hexserial
	nop
	PRINTSTR("\r\nST7FOXF1 END..........\n\n")
#endif
/********************************************/

#if			0
#ifdef	DDR_TEST
	li	v0, 0x80;
#else
	li  	a1, 0x0
    li      a0,	0xa0
    bal     i2cread
	nop
#endif

	beq	v0,0xff,1f
	nop
	beq v0,0x80,1f
	nop
	move a0,v0
	bal hexserial
	nop
    PRINTSTR ("\r\nNo DIMM in slot 0 \r\n");
	b 2f
	nop
1:
//	or  	sdCfg, 0x1<<29
	nop
	li a0,0xa0
	bal ii2c_cfg
	nop
2:
	b	2f;	// liujl
	nop;
	
	li  	a1, 0x0
    li      a0,0xa2
    bal     si2cread
	nop
	li 	a1,0x0
	beq	v0,0xff,1f
	nop
	beq v0,0x80,1f
	nop
	move a0,v0
	bal hexserial
	nop
    PRINTSTR ("\r\nNo DIMM in slot 1 \r\n");
	b 2f
	nop
1:
	li a0,0xa2
	bal ii2c_cfg
	nop

	b 2f
	nop

2:
	b 211f
	nop
#endif

/*ic2 cfg
* a0=0xa1 for slot 0,a0=0xa3 for slot 1
* t5 used for save i2c addr a0,t6 save ra.
*/
LEAF(ii2c_cfg)
	move t6,ra
	move t5,a0

	# set some parameters for DDR333
	# rank number and DDR type field will be filled later
	# to check: fix TCAS?
       
	PRINTSTR("read DIMM number of rows\r\n") 

	/* read DIMM number of rows */
#ifndef	DDR_TEST
	move	a0,t5	/* #zgj-11-17 */
	li	a1,3
	bal	si2cread
	nop
#else
	li	v0, 0x0d
#endif

	move	s6,v0
#ifdef	DDR_TEST
	move	a0, v0;
	bal	hexserial;
	nop;
	PRINTSTR("\r\naaa\r\n");
	move	v0, s6;
#endif
	move	a0, v0
	subu	v0, 12
	bgtu	v0, 2,.nodimm1
	nop
	move	t1, v0
	PRINTSTR("read number of cols\r\n")

2:	/* read DIMM number of cols */

#ifndef DDR_TEST
    move      a0,t5   /* #zgj-11-17 */
	li	a1,4
	bal	i2cread
	nop
#else
	li	v0, 0x0a
#endif
#ifdef	DDR_TEST
	move	v1, v0;
	move	a0, v0;
	bal	hexserial;
	nop;
	PRINTSTR("\r\nbbb\r\n");
	move	v0, v1;
#endif
//////////////////////	
	move 	a0,v0
	dsll	a0,a0,32
	daddu	s6,a0
/////////////////////
	subu	v0, 8
	bgtu	v0, 4,.nodimm1
	nop
	
	bne	t1, 0, 10f
	nop
	bne	v0, 2, 20f
	nop
	li	v0, 0
	b	.ddrtype1
	nop

20:	bne	v0, 1, 21f
	nop
	li	v0, 1
	b	.ddrtype1
	nop
21:	bne	v0, 0, 22f
	nop
	
	li	v0, 2
	b	.ddrtype1
	nop
22:	bne	v0, 3, 33f
	nop
	li	v0, 3
	b	.ddrtype1
	nop
10:	bne	t1, 1, 11f
	nop
	bne	v0, 3, 20f
	nop
	li	v0, 4
	b	.ddrtype1
	nop
20:	bne	v0, 2, 21f
	nop
	li	v0, 5
	b	.ddrtype1
	nop
21:	bne	v0, 1, 22f
	nop
	li	v0, 6
	b	.ddrtype1
	nop
22:	bne	v0, 4, 33f
	nop
	li	v0, 7
	b	.ddrtype1
	nop
11:	bne	t1, 2, 33f
	nop
	bne	v0, 4, 20f
	nop
	li	v0, 8
	b	.ddrtype1
	nop
20:	bne	v0, 3, 21f
	nop
	li	v0, 9
	b	.ddrtype1
	nop
21:	bne	v0, 2, 33f
	nop
	li	v0, 10
	b	.ddrtype1
	nop
33:	PRINTSTR("DDR type not supported!\r\n");
34:	b	34b
	nop

.ddrtype1:
	#bit 25:22 is DDR type field
	sll	v0, 22 
	and	v0,0x03c00000
//	or	sdCfg,v0
	
	/* read DDR RATE*/
	move	a0,t5
	li	a1,23
	bal 	i2cread 
	nop
#ifdef	DDR_TEST
	move	v1, v0;
	move	a0, v1;
	bal	hexserial;
	nop;
	PRINTSTR("\r\nccc\r\n");
	move	v0, v1;
#endif
	beq	v0,0xa0,40f
	nop
	beq	v0,0x75,41f
	nop
	beq	v0,0x60,42f
	nop
	b	42f
	nop
	
	/* config sdCfg bits [ 9:0 ] */
40:	/* ddr200 */
//	or	sdCfg,0x0b1
	b	47f
	nop

41:     /* ddr266 */
//    or      sdCfg,0x3ae	
	b     47f
    nop

42:     /* ddr333 */
//    or      sdCfg,0x3df
	b     47f
    nop

#####################################################

 /* read DDR SDRAM Minimum Clock Cycle when CL is Derated by One Clock,config sdCfg [ 21:10 ]*/
47:
    move    a0,t5
    li      a1,25
    bal     i2cread
    nop
#ifdef		DDR_TEST
	move	v1, v0;
	move	a0, v1;
	bal	hexserial;
	nop;
	PRINTSTR("\r\nddd\r\n");
	move	v0, v1;
#endif
    beq     v0,0xa0,40f
    nop
    beq     v0,0x75,41f
    nop
    beq     v0,0x60,42f
    nop
    b     41f
	nop
	
40:     /* ddr200 */
 	move    a0,t5
    li      a1,12
    bal     i2cread
    nop
#ifdef	DDR_TEST
	move	v1, v0;
	move	a0, v1;
	bal	hexserial;
	nop;
	PRINTSTR("\r\neee\r\n");
	move	v0, v1;
#endif

    bne     v0,0x82,10f
    nop
//    or      sdCfg,780<<10
	b	45f
	nop
10:
    bne     v0,0x80,.nodimm1
    nop
//    or      sdCfg,1560<<10
    b     45f
    nop

41:     /* ddr266 */
    move    a0,t5
    li      a1,12
    bal     i2cread
    nop
#ifdef	DDR_TEST
	move	v1, v0;
	move	a0, v1;
	bal	hexserial;
	nop;
	PRINTSTR("\r\nfff\r\n");
	move	v0, v1;
#endif
    bne     v0,0x82,11f
    nop
//    or      sdCfg,1040<<10
	b	45f
	nop
11:
     bne     v0,0x80,.nodimm1
     nop
//     or      sdCfg,2080<<10
     b     45f
     nop

42:     /* ddr333 */
    move    a0,t5
    li      a1,12
    bal     i2cread
    nop
#ifdef	DDR_TEST
	move	v1, v0;
	move	a0, v1;
	bal	hexserial;
	nop;
	PRINTSTR("\r\nggg\r\n");
	move	v0, v1;
#endif

    bne     v0,0x82,12f
    nop
//    or      sdCfg,1300<<10
	b	45f
	nop
12:
     bne     v0,0x80,.nodimm1
     nop
//     or      sdCfg,2600<<10
     b     45f
     nop


#####################################################


	/* Minimum Ras to Cas Delay (tRCD) 
	move	a0,t5
	bal 	i2cread
	nop
#ifdef	DDR_TEST
	move	v1, v0;
	move	a0, v1;
	bal	hexserial;
	nop;
	PRINTSTR("\r\nhhh\r\n");
	move	v0, v1;
#endif

	li	a1,29
	or	*/
45:
2:
    move      a0,t5  
	li	a1,17
	bal	i2cread
	nop
#ifdef	DDR_TEST
	move	v1, v0;
	move	a0, v1;
	bal	hexserial;
	nop;
	PRINTSTR("\r\njjj\r\n");
	move	v0, v1;
#endif

	beq	v0,2,2f
	nop
	bne	v0,4,.nodimm1
	nop
	PRINTSTR("number of banks on sdram device\r\n");

2:	/* read DIMM number of sides (banks) */
    move      a0,t5   /* #zgj-11-17 */
	li	a1,5
	bal	i2cread
	nop
#ifdef	DDR_TEST
	move	v1, v0;
	move	a0, v1;
	bal	hexserial;
	nop;
	PRINTSTR("\r\niii\r\n");
	move	v0, v1;
#endif

//***********
	andi	v0,v0,0x7
//***********
	beq	v0,0,2f
	nop
	bne	v0,1,.nodimm1
	nop

//************
	bne 	t5,0xa0,123f
	nop
	ori	k1,k1,0x3
	b	124f

123:	ori	k1,k1,0xc

124:	b	124f
	nop


	PRINTSTR("number of ranks ,package and height\r\n") ;

2:	bne	t5,0xa0,123f
	nop
	ori	k1,k1,0x1
	b	124f
	nop

123:	ori	k1,k1,0x4
124:	nop

/* read DIMM width */
    move      a0,t5   
	li	a1,6
	bal	i2cread
	nop
#ifdef	DDR_TEST
	move	v1, v0;
	move	a0, v1;
	bal	hexserial;
	nop;
	PRINTSTR("\r\nkkk\r\n");
	move	v0, v1;
#endif

	bleu	v0,36,2f
	nop
	bgtu	v0,72,.nodimm1
	nop
	PRINTSTR("module data width\r\n") ;
2:
   move      a0,t5
   	li      a1,31
   	bal     i2cread
   	nop
#ifdef	DDR_TEST
	move	v1, v0;
	move	a0, v1;
	bal	hexserial;
	nop;
	PRINTSTR("\r\nttt\r\n");
	move	v0, v1;
#endif

  	beqz    v0,.nodimm1
   	nop
////////////////////
	beq	v0,0x80,1f
	nop
	beq	v0,0x40,2f
	nop
	beq	v0,0x20,3f
	nop
	beq	v0,0x1,4f
	nop
	li	tmpsize,2<<30
	b	100f
	nop

4:	li	tmpsize,1<<30
	b	100f
	nop
3:	li	tmpsize,128<<20
	b	100f
	nop
2:	li	tmpsize,256<<20
	b	100f
	nop
1:	li	tmpsize,512<<20

////////////////
100:   	addu	msize,tmpsize
   
   move      a0,t5 
	li	a1,5
	bal	i2cread
	nop
#ifdef	DDR_TEST
	move	v1, v0;
	move	a0, v1;
	bal	hexserial;
	nop;
	PRINTSTR("\r\nqqq\r\n");
	move	v0, v1;
#endif

	andi	v0,0x7
	beq	v0,0,1f
	nop
	addu	msize,tmpsize
	b 1f
	nop
	
.nodimm1:
	PRINTSTR ("\r\nNo DIMM in this slot ");
1:
	jr t6
	nop
END(ii2c_cfg)
#endif
/* data register bit[3:0] for output data
 * data register bit[19:16] for input data
 * gpio2 for data
 * gpio3 for clock
 */
#define GPIO_DIR	(0xbfe00100 | 0x20)
#define GPIO_DATA	(0xbfe00100 | 0x1c)
#define	G_OUTPUT	0
#define	G_INPUT		1
#define	SDA_NUM		2
#define	SCL_NUM		3
/*
 *can't use t1,t5,t6
 */
LEAF(_i2c_sleep)
//
//	li 	t0,0x300
	li 	t0,0x10
	sll	a0,t0,a0
	
1:	nop
	subu 	a0,1
	bnez	a0,1b
	nop
	
	jr ra
	nop
	
END(_i2c_sleep)

LEAF(_sda_dir)
	li	t0,GPIO_DIR
	lwu	t2,0(t0)
	nop
	
	beqz	a0,1f
	nop

	ori	t2, t2, (1 << SDA_NUM)
	b	2f
	nop
1:
	li	t3, ~(1 << SDA_NUM)
	and	t2, t2, t3
	nop

2:	sw	t2,0(t0)
	nop

	jr	ra
	nop
END(_sda_dir)


LEAF(_scl_dir)
	li	t0, GPIO_DIR
	lwu	t2, 0(t0)
	nop

	beqz	a0,1f
	nop

	ori	t2, t2, (1 << SCL_NUM)
	b	2f
	nop	
1:	li	t3, ~(1 << SCL_NUM)
	and t2, t2, t3
	nop

2:	sw	t2,0(t0)
	nop

	jr	ra
	nop
END(_scl_dir)

LEAF(_sda_bit)
	li	t0, GPIO_DATA
	lwu	t2, 0(t0)
	nop
	
	beqz	a0,1f
	nop

	ori	t2, t2, (1 << SDA_NUM)
	b	2f
	nop	
1:	li	t3, ~(1 << SDA_NUM)
	and t2, t2, t3
	nop

2:	sw	t2,0(t0)
	nop

	jr	ra
	nop
END(_sda_bit)

LEAF(_scl_bit)
	li	t0, GPIO_DATA
	lwu	t2, 0(t0)
	nop
	
	beqz	a0,1f
	nop

	ori	t2, t2, (1 << SCL_NUM)
	b	2f
	nop	
1:	li	t3, ~(1 << SCL_NUM)
	and t2, t2, t3
	nop

2:	sw	t2,0(t0)
	nop

	jr	ra
	nop
END(_sda_bit)


LEAF(_i2c_start)
	move	t7,ra

	li 	a0,G_OUTPUT
	bal	_sda_dir
	nop
	li	a0,G_OUTPUT
	bal	_scl_dir
	nop
	li	a0,0
	bal	_scl_bit
	nop
	li	a0,1
	bal	_i2c_sleep
	nop
	li	a0,1
	bal	_sda_bit
	nop

	li	a0,1
	bal	_i2c_sleep
	nop
	li	a0,1
	bal	_scl_bit
	nop
	li	a0,5
	bal	_i2c_sleep
	nop
	li 	a0,0
	bal _sda_bit
	nop
	li	a0,5
	bal	_i2c_sleep
	nop
	li	a0,0
	bal	_scl_bit
	nop
	li	a0,2
	bal	_i2c_sleep
	nop

	jr 	t7
	nop
	
END(_i2c_start)

LEAF(_i2c_stop)
	move 	t7,ra

	li	a0,G_OUTPUT
	bal	_sda_dir
	nop
	li	a0,G_OUTPUT
	bal	_scl_dir
	nop
	li	a0,0
	bal	_scl_bit
	nop
	li	a0,1
	bal	_i2c_sleep
	nop
	li	a0,0
	bal	_sda_bit
	nop
	li 	a0,1
	bal	_i2c_sleep
	nop
	li	a0,1
	bal 	_scl_bit
	nop
	li	a0,5
	bal	_i2c_sleep
	nop
	li	a0,1
	bal	_sda_bit
	nop
	li	a0,5
	bal	_i2c_sleep
	nop
	li 	a0,0
	bal	_scl_bit
	nop
	li	a0,2
	bal	_i2c_sleep
	nop
	
	jr	t7
	nop
END(_i2c_stop)
LEAF(_i2c_send_ack)

	move	t7,ra
	move 	t4,a0

	li	a0,G_OUTPUT
	bal	_sda_dir
	nop
	move a0,t4
	bal	_sda_bit
	nop
	li	a0,3
	bal	_i2c_sleep
	nop
	li 	a0,1
	bal	_scl_bit
	nop
	li	a0,5
	bal	_i2c_sleep
	nop
	li	a0,0
	bal	_scl_bit
	nop
	li	a0,2
	bal	_i2c_sleep
	nop
	
	jr	t7
	nop
END(_i2c_send_ack)

LEAF(_i2c_rec_ack)
	move 	t7,ra
	li	v0,1
	li	t4,10

	li	a0,G_INPUT
	bal	_sda_dir
	nop
	li	a0,3
	bal	_i2c_sleep
	nop
	li	a0,1
	bal	_scl_bit
	nop
	li	a0,5
	bal	_i2c_sleep
	nop
	li	t9,GPIO_DATA
	lwu	t9,0(t9)
	nop

	srl		t9, 16
	andi	t9, t9, (1 << SDA_NUM)

2:	beqz	t9,1f
	nop

	li	a0,1
	bal	_i2c_sleep
	nop
	subu	t4,t4,1
	
	bnez	t4,3f
	nop
	li	v0,0
	b	1f
	nop

3:	li	t9, GPIO_DATA
	lwu	t9, 0(t9)
	nop
	srl	t9, 16
	andi t9,t9, (1 << SDA_NUM)
	b	2b
	nop
	

1:	li	a0,0
	bal	_scl_bit
	nop
	li	a0,3
	bal	_i2c_sleep
	nop
	
	jr	t7
	nop	

END(_i2c_rec_ack)


LEAF(_i2c_rec)
	move 	t7,ra
	li	t9,0x7
	li	v0,0
	li	a0,G_INPUT
	bal	_sda_dir
	nop

2:	bltz	t9,1f
	nop

	li	a0,5
	bal	_i2c_sleep
	nop
	li	a0,1
	bal	_scl_bit
	nop
	li	a0,3
	bal	_i2c_sleep
	nop
	li	t4, GPIO_DATA
	lwu	t4, 0(t4)
	nop
	srl	t4, 16
	andi	t4,t4,(1 << SDA_NUM)

	beqz 	t4,3f
	nop
	li	t4,1

3:	sll	t4,t4,t9
	or	v0,v0,t4
	li	a0,3
	bal	_i2c_sleep
	nop
	li	a0,0
	bal	_scl_bit
	nop

	sub	t9,t9,1
	b	2b
	nop
	
1:	jr t7
	nop
	
END(_i2c_rec)

LEAF(_i2c_send)
	move	t7,ra
	move	t4,a0
	li	t9,0x7
	
	li	a0,G_OUTPUT
	bal	_sda_dir
	nop
	
2:	bltz	t9,1f
	nop

	move	a0,t4
	srl	a0,a0,t9
	andi	a0,a0,1
	bal	_sda_bit
	nop
	
	li	a0,1
	bal	_i2c_sleep
	nop
	li	a0,1
	bal	_scl_bit
	nop
	li	a0,5
	bal	_i2c_sleep
	nop
	li	a0,0
	bal	_scl_bit
	nop
	li	a0,1
	bal	_i2c_sleep
	nop
	
	sub	t9,t9,1
	b	2b
	nop
1:	li	a0,1
	bal	_sda_bit
	nop

	jr	t7
	nop

END(_i2c_send)

/*
a0,a2:slave device addr
a1,a3:sub addr
v0:recieve data
v1:show if sucess,0:sucess,1:failure
*/
LEAF(si2cread)
	move 	t8,ra
	nop
	move 	a2,a0
	move	a3,a1
	li	v0,0
	li	v1,0
	

	bal	_i2c_start
	nop

	move	a0,a2
	bal	_i2c_send
	nop

	bal	_i2c_rec_ack
	nop
	beqz	v0,1f
	nop

	move	a0,a3
	bal	_i2c_send
	nop
	beqz	v0,1f
	nop

	bal	_i2c_rec_ack
	nop
	beqz	v0,1f
	nop


	bal	_i2c_start
	nop

	move	a0,a2
	addu	a0,a0,1
	bal	_i2c_send
	nop
	beqz	v0,1f
	nop

	bal	_i2c_rec_ack
	nop
	beqz	v0,1f
	nop

	bal	_i2c_rec
	nop
	move	k0,v0
	
	li	a0,1
	bal	_i2c_send_ack
	nop
	bal	_i2c_stop
	nop

	li	v1,0
	move	v0,k0
	b	2f
	nop

1:	li	v1,1
2:	jr	t8 
	nop


END(si2cread)

//211:
