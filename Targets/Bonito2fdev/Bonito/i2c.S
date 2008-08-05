#define i2cread newi2cread
	li	k0,0
	li	k1,0
	li	msize,0
	PRINTSTR("DIMM read\r\n")

	li  	a1, 0x0
    li      a0,0xa1
    bal     i2cread
	nop
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
	li a0,0xa1
	bal ii2c_cfg
	nop
2:
	li  	a1, 0x0
    li      a0,0xa3
    bal     i2cread
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
	li a0,0xa3
	bal ii2c_cfg
	nop

	b 2f
	nop

2:
	b 211f
	nop
/*ic2 cfg
* a0=0xa1 for slot 0,a0=0xa3 for slot 1
* t5 used for save i2c addr a0,t6 save ra.
*/
LEAF(ii2c_cfg)
	move t6,ra
	move t5,a0

#ifdef I2C_DEBUG
	li t1,0
1:
	move a1,t1
	move a0,t5
	bal	i2cread
	nop

	#print
	move a0, v0
	bal  hexserial
	nop
	PRINTSTR("\r\n")
	addiu t1,t1,1
	li   v0, 0x20
	bleu  t1, v0, 1b
	nop
#endif

	# set some parameters for DDR333
	# rank number and DDR type field will be filled later
	# to check: fix TCAS?

       
	PRINTSTR("read memory type\r\n") 

	/* read DIMM number of rows */
	move	a0,t5	/* #zgj-11-17 */
	li	a1,3
	bal	i2cread
	nop	
	move	s6,v0
	move	a0, v0
	subu	v0, 12
	bgtu	v0, 2,.nodimm1
	nop
	move	t1, v0
	PRINTSTR("read number of rows\r\n")
	move 	a0,s6
	bal	hexserial
	nop

2:	/* read DIMM number of cols */

    move      a0,t5   /* #zgj-11-17 */
	li	a1,4
	bal	i2cread
	nop
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
	
	/* read DDR RATE*/
	move	a0,t5
	li	a1,23
	bal 	i2cread 
	nop
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
	b	47f
	nop

41:     /* ddr266 */
	b     47f
    nop

42:     /* ddr333 */
	b     47f
    nop

#####################################################

 /* read DDR SDRAM Minimum Clock Cycle when CL is Derated by One Clock,config sdCfg [ 21:10 ]*/
47:
    move    a0,t5
    li      a1,25
    bal     i2cread
	nop
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
    bne     v0,0x82,10f
    nop
	b	45f
	nop
10:
    bne     v0,0x80,.nodimm1
    nop
    b     45f
    nop

41:     /* ddr266 */
    move    a0,t5
    li      a1,12
    bal     i2cread
    nop
    bne     v0,0x82,11f
    nop
	b	45f
	nop
11:
     bne     v0,0x80,.nodimm1
     nop
     b     45f
     nop

42:     /* ddr333 */
    move    a0,t5
    li      a1,12
    bal     i2cread
    nop
    bne     v0,0x82,12f
    nop
	b	45f
	nop
12:
     bne     v0,0x80,.nodimm1
     nop
     b     45f
     nop


#####################################################

45:
2:
    move      a0,t5 
	li	a1,17
	bal	i2cread
	nop
	beq	v0,4,2f
	nop
	bne	v0,8,.nodimm1
	li	k0,1
	nop
2:	

	PRINTSTR("number of ranks ,package and height\r\n") ;
	PRINTSTR("k1 to save cs_map filed valule\r\n") ;
    move    a0,t5
	li	a1,5
	bal	i2cread
	nop
//***********
	andi	v0,v0,0x7
//***********
	beq	v0,0,2f
	nop
	bne	v0,1,.nodimm1
	nop

//************
	bne 	t5,0xa1,123f
	nop
	ori	k1,k1,0x3
	b	124f
	nop

123:	ori	k1,k1,0xc

124:	b	124f
	nop



2:	bne	t5,0xa1,123f
	nop
	ori	k1,k1,0x1
	b	124f
	nop

123:	ori	k1,k1,0x4
124:	nop

//************



/* read DIMM width */
    move      a0,t5   
	li	a1,6

	bal	i2cread
	nop
	bleu	v0,36,2f
	nop
	bgtu	v0,72,.nodimm1
	nop
	PRINTSTR("read width\r\n") ;
2:
	PRINTSTR("module rank density\r\n") ;
   	move    a0,t5
   	li      a1,31
   	bal     i2cread
  	nop
  	beqz    v0,.nodimm1
   	nop
////////////////////
	beq	v0,0x80,1f
	nop
	beq	v0,0x40,2f
	nop
	beq	v0,0x20,3f
	nop
	beq	v0,0x10,4f
	nop	
	beq	v0,0x08,5f
	nop
	beq	v0,0x04,6f
	nop
	beq	v0,0x02,7f
	nop	
	li	tmpsize,1<<10
	b	100f
	nop

7:	li	tmpsize,2<<10
	b	100f
	nop
6:	li 	tmpsize,4<<10
	b 	100f
	nop
5:	li      tmpsize,8<<10
	b	100f
	nop
4:	li	tmpsize,16<<10
	b	100f
	nop
3:	li	tmpsize,128
	b	100f
	nop
2:	li	tmpsize,256
	b	100f
	nop
1:	li	tmpsize,512

////////////////
100:   	addu	msize,tmpsize
  
  
	PRINTSTR("sizing slot memory size\r\n") ;
   	move      a0,t5 
	li	a1,5
	bal	i2cread
	nop
	andi	v0,0x7
2:	beq	v0,0,1f
	nop
	addu	msize,tmpsize
	subu	v0,v0,1

	b 2b
	nop
	
.nodimm1:
	PRINTSTR ("\r\nNo DIMM in this slot ");
1:
	jr t6
	nop
END(ii2c_cfg)

LEAF(i2cread)
        /* set device address */
        li  v0, 0xbfd00000 + SMBUS_HOST_ADDRESS

        sb  a0, 0(v0);

        /* store register offset */
        li  v0, 0xbfd00000 + SMBUS_HOST_COMMAND
        sb  a1, 0(v0);

        /* read byte data protocol */
        li  v0, 0x08
        li  v1, 0xbfd00000 + SMBUS_HOST_CONTROL
        sb  v0, 0(v1);

        /* make sure SMB host ready to start, important!--zfx */
        li  v1, 0xbfd00000 + SMBUS_HOST_STATUS
        lbu v0, 0(v1)
        andi v0,v0, 0x1f
        beqz  v0,1f
        nop
        sb  v0, 0(v1)
        lbu v0, 0(v1)   #flush the write
1:

        /* start */
        li  v1, 0xbfd00000 + SMBUS_HOST_CONTROL
        lbu v0, 0(v1)
        ori v0, v0, 0x40
        sb  v0, 0(v1);

        /* wait */
        li  v1, 0xbfd00000 + SMBUS_HOST_STATUS
1:

#if 0
        /* delay */
        li a0, 0x1000
2:
        bnez    a0,2b
        addiu   a0, -1
#endif

        lbu  v0, 0(v1)
        andi v0, SMBUS_HOST_STATUS_BUSY
        bnez  v0, 1b  #IDEL ?
        nop

        li  v1, 0xbfd00000 + SMBUS_HOST_STATUS
        lbu v0, 0(v1)
        andi v0,v0, 0x1f
        beqz  v0,1f
        nop
        sb  v0, 0(v1)   #reset
        lbu v0, 0(v1)   #flush the write
1:

        li  v1, 0xbfd00000 + SMBUS_HOST_DATA0
        lbu  v0, 0(v1)

        jr      ra
        nop
END(i2cread)
211:
	beqz msize,212f
	nop
	move	a0,k1
	
	li	v0,0xaffffe70
	lw	v1,0(v0)
	nop
	sll	a0,a0,16
	li	t0,0xfff0ffff
	and	v1,v1,t0
	or	v1,v1,a0
	sw	v1,0(v0)
	nop
	
	bal 	hexserial
	nop


PRINTSTR("\r\n")
PRINTSTR("config bank if bank 8\r\n")
	li	v0,0xaffffe10
	ld	v1,0(v0)
	nop
	move 	a0,v1
	dsrl	a0,a0,32
	or	a0,a0,k0
	dsll	a0,a0,32
	or	v1,a0,v1
	sd	v1,0(v0)
	nop
	move	a0,v1
	dsrl	a0,a0,32
	bal	hexserial
	nop

PRINTSTR("cols rows:\r\n")
	
	//row addr numbers
	li	v0,0xffffffff
	and	a0,s6,v0
	li	v0,15
	subu	a0,v0,a0

	//col addr numbers
	dsrl	s6,s6,20
	dsrl	s6,s6,12
	li	v0,14
	
	move	a1,s6
	subu	a1,v0,a1

	sll	a1,a1,24
	sll	a0,a0,8

	li 	v0,0xaffffe50
	lw	v1,0(v0)
	nop
	li	t0,0xf8fff8ff
	and	v1,v1,t0
	or	v1,v1,a0
	or	v1,v1,a1
	sw	v1,0(v0)
	nop
//	PRINTSTR("cols rows:\r\n")
	move	a0,v1
	bal	hexserial
	nop
	
	PRINTSTR("\r\n")
	sll msize,20
	b 213f
	nop
212:	
.nodimm:
	li	msize,0x10000000
	PRINTSTR ("\r\nNo DIMM in all slots,use default configure\r\n")
213:
#undef i2cread
