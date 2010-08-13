#define BONITO_HTIO_BASE_VA 0x90000efdfc000000
#define CHIP1_BASE_ADDRESS	0x0000100000000000
#define CHIP2_BASE_ADDRESS	0x0000200000000000
#define CHIP3_BASE_ADDRESS	0x0000300000000000

#define SPD_NODEVICE  0xff
#define SPD_TYPEDDR2  0x8
#define SPD_TYPEDDR3  0xc

#define	DDRTYPE_MASK  0x7
#define	DDRPERSZ_512M 0x10
#define	DDRPERSZ_1G   0x20
#define	DDRPERSZ_2G   0x30

#define	SPD_MEMTYPE_OFFSET	0x02
#define	SPD_ROWADDR_OFFSET	0x03
#define	SPD_COLADDR_OFFSET	0x04
#define	SPD_DIMRANK_OFFSET	0x11
#define	SPD_RKDESITY_OFFSET	0x1f

#define	SPD_DDR2SIZE_512M	0x80
#define	SPD_DDR2SIZE_1G		0x01
#define	SPD_DDR2SIZE_2G		0x02

#define	DDR2SIZE_512M_MASK	0x10
#define	DDR2SIZE_1G_MASK	0x20
#define	DDR2SIZE_2G_MASK	0x30
#define	DDR2SIZE_MASK	    0x70


#ifndef  MULTI_CHIP



LEAF(i2cread)
	    ori a0,a0,1
        /* set device address */
        //li  v0, 0xbfd00000 + SMBUS_HOST_ADDRESS
        dli  v0, BONITO_HTIO_BASE_VA + SMBUS_HOST_ADDRESS

        sb  a0, 0(v0);

        /* store register offset */
        //li  v0, 0xbfd00000 + SMBUS_HOST_COMMAND
        dli  v0, BONITO_HTIO_BASE_VA + SMBUS_HOST_COMMAND
        sb  a1, 0(v0);

        /* read byte data protocol */
        li  v0, 0x08
        //li  v1, 0xbfd00000 + SMBUS_HOST_CONTROL
        dli  v1, BONITO_HTIO_BASE_VA + SMBUS_HOST_CONTROL
        sb  v0, 0(v1);

        /* make sure SMB host ready to start, important!--zfx */
        //li  v1, 0xbfd00000 + SMBUS_HOST_STATUS
        dli  v1, BONITO_HTIO_BASE_VA + SMBUS_HOST_STATUS
        lbu v0, 0(v1)
        andi v0,v0, 0x1f
        beqz  v0,1f
        nop
        sb  v0, 0(v1)
        lbu v0, 0(v1)   #flush the write
1:

        /* start */
        //li  v1, 0xbfd00000 + SMBUS_HOST_CONTROL
        dli  v1, BONITO_HTIO_BASE_VA + SMBUS_HOST_CONTROL
        lbu v0, 0(v1)
        ori v0, v0, 0x40
        sb  v0, 0(v1);

        /* wait */
        //li  v1, 0xbfd00000 + SMBUS_HOST_STATUS
        dli  v1, BONITO_HTIO_BASE_VA + SMBUS_HOST_STATUS
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

        //li  v1, 0xbfd00000 + SMBUS_HOST_STATUS
        dli  v1, BONITO_HTIO_BASE_VA + SMBUS_HOST_STATUS
        lbu v0, 0(v1)
        andi v0,v0, 0x1f
        beqz  v0,1f
        nop
        sb  v0, 0(v1)   #reset
        lbu v0, 0(v1)   #flush the write
1:

        //li  v1, 0xbfd00000 + SMBUS_HOST_DATA0
        dli  v1, BONITO_HTIO_BASE_VA + SMBUS_HOST_DATA0
        lbu  v0, 0(v1)

        jr      ra
        nop
END(i2cread)


LEAF(PROBE_DIMM) 
	move  a3,ra;
	move  s2,a0;  //store a0: i2C DEVICE id
	move  s3,a1;  //store a1: REG offset in i2c dev	

	li	a1, 2;
	bal	i2cread;
	nop;
	beq	v0,SPD_NODEVICE, out;
	nop;
	

	/* set DDR type @ s1[7:7] */
	/* assumed here v0 should be 0xc or 0x8 */
	//bne	v0,SPD_TYPEDDR3, ddr2  
	andi	v0,0x4;
	srl	v0,0x2;
	sll	v0,DDRTYPE_MASK ;
	or	s1,v0;


	/* set  CONTROLLER_SELECT@ s1[3:2] */
	/* a1 should set to be MC0_USED or MC1_USD */
	/* Firt check whether BOTH MC0 and MC1 used, if
       true,set s1[3:2] = 2b'00
	*/
	or	s1,s3;
	andi  t1,s1,0xc;
	bne	t1,0xc,10f;
	nop
	subu  s1,0xc

10:
	/* set SIZE_PER_CONTROLLE @ s1[6:4] */
	/* step 1: read out number of DIMMS Ranks(CS) */
	move a0, s2;  //store a1: REG offset in i2c dev	
	li	a1, 5;
	bal	i2cread;
	nop;
	andi  v0,0x3; // MASK for DIMMS Ranks(CS)
	move  t2,v0;

	//bez t2,out
	nop
	addi t2,0x1

	/* step 2: read out  DIMMS size */
	move  a0,s2;
	li	a1, 31;
	bal	i2cread;
	nop;
	bne	v0,SPD_DDR2SIZE_512M, 11f;
	nop;
	//ori	s1,DDR2SIZE_512M_MASK;	 
	li	v0, 0x20000000
	b 1f;
	nop;
11:
	bne	v0,SPD_DDR2SIZE_1G, 12f;
	nop;
	//ori	s1,DDR2SIZE_1G_MASK;
	li	v0, 0x40000000
	b 1f;
	nop;
12:
	bne	v0,SPD_DDR2SIZE_2G, 1f;
	nop;
	//ori	s1,DDR2SIZE_2G_MASK;	// v0: 0x80 means 1G
	li	v0, 0x80000000;
1:
	/* step 3: calculate each mem SIZE of one slot*/
	beq	t2,0x1,21f;
	nop
	sll	v0,0x1
	//multu  v0,t2;

21:
	bne	v0,0x20000000, 13f
	nop;

	/* check whther this channel is smaller than others
	 if smller: remove other bits,and set DDRPERSZ_512M 
	 else: do nothing, don't set PERSIZE_BIT
	// defalut setting 512M leaset MEM size  
	*/

	andi t1,s1,DDR2SIZE_MASK; 
	beqz  t1,211f;
	nop
	bleu  t1,DDR2SIZE_512M_MASK, 15f;
	nop
	li	t1,DDR2SIZE_MASK;
	not	t2,t1;
	and s1,t2;

211:
	ori	s1,DDRPERSZ_512M ;
	b 15f;
	nop
13:
	bne	v0,0x40000000, 14f
	nop

	/* check whther this channel is smaller than others
	 if smller: remove other bits,and set DDRPERSZ_1G 
	 else: do nothing, don't set PERSIZE_BIT
	*/
	andi t1,s1,DDR2SIZE_MASK; 
	beqz  t1,131f;
	nop
	bleu  t1,  DDR2SIZE_1G_MASK,15f
	nop
	li	t1,DDR2SIZE_MASK;
	not	t2,t1;
	and s1,t2;

131:
	ori	s1,DDRPERSZ_1G;
	b 15f;
	nop

14:/* only support 512M,1G,2G per DIMM now */
	bne	v0,0x80000000, 15f
	nop

	/* check whther this channel is smaller than others
	 if smller: remove other bits,and set DDRPERSZ_2G 
	 else: do nothing, don't set PERSIZE_BIT
	*/
	andi t1,s1,DDR2SIZE_MASK; 
	beqz  t1,141f;
	nop
	bleu  t1,  DDR2SIZE_2G_MASK,15f
	nop
	li	t1,DDR2SIZE_MASK;
	not	t2,t1;
	and s1,t2;

141:
	ori	s1,DDRPERSZ_2G;
	

15:
	/* check whether MC0 or MC1 used to set CS_MAP */
	bne	s3,	MC0_USED, 2f;
	nop;
	/* set DDR MC0_CS_MAP @s1[11:8] */
	ori	s1, 0x400 // at leaset one bit is selected
	move  a0,s2;
	li	a1, 5;
	bal	i2cread;
	nop;
	andi  v0,0x1;
	beq	v0,0x0,16f;
	nop;
	ori	s1,0x800;

16:
	/* set DDR MC0_COL_SIZE @s1[18:16] */
	move  a0,s2;
	li	a1, 4;
	bal	i2cread;
	nop;
	li	t0,14;
	sub	t0,v0;
	sll	t0,0x10;
	or	s1,t0;

	/* set MC0_EIGHT_BANK @s1[19] */
	move  a0,s2;
	li	a1, 17;
	bal	i2cread;
	nop;
	andi  v0,0x8;
	srl	v0,0x3;
	sll	v0,0x13;  //MC0_EIGHT_BANK shift
	or	s1,v0;

	/* set DDR MC0_ROW_SIZE @s1[22:20] */
	move  a0,s2;
	li	a1, 3;
	bal	i2cread;
	nop;
	li	t0,15;
	sub	t0,v0;
	sll	t0,0x14;
	or	s1,t0;

    /* set MC0_ECC bit @s1[32] */
	move  a0,s2;
	li	a1, 11;
	bal	i2cread;
	li	t0,0x2;
	andi  v0,0x2;
	srl	v0,0x1;
	dsll	v0,32;
	//dli v0,0x100000000 // for test
	or	s1,v0;

	/* set DIMM Type information @s1[33:33] */
	move  a0,s2;
	li	a1, 20;
	bal	i2cread;
	nop;
	andi  v0,0x1; // only to check whether in Register Dual In_line memory module
	dsll	v0,33;
	or	s1,v0;


	b out;
	nop;

2:	/* MC1_USED */
#if 1

	/* set DDR MC1_CS_MAP @s1[15:12] ?  */
	ori	s1, 0x4000  // at leaset one bit is selected
	move  a0,s2;
	li	a1, 5;
	bal	i2cread;
	nop;
	andi  v0,0x1;
	beq	v0,0x0, 26f
	nop
	ori	s1,0x8000
26:
	
	/* set DDR MC1_COL_SIZE @s1[26:24] */
	move  a0,s2;
	li	a1, 4;
	bal	i2cread;
	nop;
	li	t0,14;
	sub	t0,v0;
	sll	t0,0x18;
	or	s1,t0;

	/* set MC1_EIGHT_BANK @s1[27] ? */
	move  a0,s2
	li	a1, 17;
	bal	i2cread;
	nop;
	andi  v0,0x8;
	srl	v0,0x3;
	sll	v0,27;  //MC0_EIGHT_BANK shift
	or	s1,v0;

	/* set DDR MC1_ROW_SIZE @s1[30:28] */
	move  a0,s2;
	li	a1, 3;
	bal	i2cread;
	nop;
	li	t0,15;
	sub	t0,v0;
	sll	t0,0x1c;
	or	s1,t0;

	/* set MC1_ECC bit @s1[34] */
	move  a0,s2;
	li	a1, 11;
	bal	i2cread;
	li	t0,0x2;
	andi  v0,0x2;
	srl	v0,0x1;
	dsll	v0,34;
	or	s1,v0;

	/* set DIMM Type information @s1[35:35] */
	move  a0,s2;
	li	a1, 20;
	bal	i2cread;
	nop;
	and	v0,0x1;// only to check whether in Register Dual In_line memory module
	dsll	v0,35;
	or	s1,v0;

#endif

out:/* out of MC0_CS_MAP or MC1_CS_MAP */
	//jr	ra
	jr	a3
	nop
END(PROBE_DIMM)

#else
/****************************************************/
/*		for multi-chip mode							*/
/****************************************************/

/* input: a0,a1,a2
   a0: device ID
   a1: register offset
   a2: chip ID (0,1,2,3)
   changed registers: v0,v1,t3 
*/
LEAF(i2cread)
	    ori a0,a0,1

		/* added address offset according chip ID */
		beqz a2,88f
		nop
		beq	a2,0x1, 81f
		nop
		beq	a2,0x2, 82f
		nop

		dli	t3, CHIP3_BASE_ADDRESS
		b 88f

81:		dli	t3, CHIP1_BASE_ADDRESS
		b 88f
		nop
82:		dli	t3, CHIP2_BASE_ADDRESS
		b 88f
		nop
88:
        /* set device address */
        //li  v0, 0xbfd00000 + SMBUS_HOST_ADDRESS
        dli  v0, BONITO_HTIO_BASE_VA + SMBUS_HOST_ADDRESS
		dadd  v0,v0,t3
        sb  a0, 0(v0);

        /* store register offset */
        //li  v0, 0xbfd00000 + SMBUS_HOST_COMMAND
        dli  v0, BONITO_HTIO_BASE_VA + SMBUS_HOST_COMMAND
		dadd  v0,v0,t3
        sb  a1, 0(v0);

        /* read byte data protocol */
        li  v0, 0x08
        //li  v1, 0xbfd00000 + SMBUS_HOST_CONTROL
        dli  v1, BONITO_HTIO_BASE_VA + SMBUS_HOST_CONTROL
		dadd  v1,v1,t3
        sb  v0, 0(v1);

        /* make sure SMB host ready to start, important!--zfx */
        //li  v1, 0xbfd00000 + SMBUS_HOST_STATUS
        dli  v1, BONITO_HTIO_BASE_VA + SMBUS_HOST_STATUS
		dadd  v1,v1,t3
        lbu v0, 0(v1)
        andi v0,v0, 0x1f
        beqz  v0,1f
        nop
        sb  v0, 0(v1)
        lbu v0, 0(v1)   #flush the write
1:

        /* start */
        //li  v1, 0xbfd00000 + SMBUS_HOST_CONTROL
        dli  v1, BONITO_HTIO_BASE_VA + SMBUS_HOST_CONTROL
		dadd  v1,v1,t3
        lbu v0, 0(v1)
        ori v0, v0, 0x40
        sb  v0, 0(v1);

        /* wait */
        //li  v1, 0xbfd00000 + SMBUS_HOST_STATUS
        dli  v1, BONITO_HTIO_BASE_VA + SMBUS_HOST_STATUS
		dadd  v1,v1,t3
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

        //li  v1, 0xbfd00000 + SMBUS_HOST_STATUS
        dli  v1, BONITO_HTIO_BASE_VA + SMBUS_HOST_STATUS
		dadd  v1,v1,t3
        lbu v0, 0(v1)
        andi v0,v0, 0x1f
        beqz  v0,1f
        nop
        sb  v0, 0(v1)   #reset
        lbu v0, 0(v1)   #flush the write
1:

        //li  v1, 0xbfd00000 + SMBUS_HOST_DATA0
        dli  v1, BONITO_HTIO_BASE_VA + SMBUS_HOST_DATA0
		dadd  v1,v1,t3
        lbu  v0, 0(v1)

        jr      ra
        nop
END(i2cread)

/* input: a0,a1,a2
   a0: I2C device ID
   a1: register offset in the device
   a2: CHIP id (0,1,2,3)
   output: not set
*/
LEAF(PROBE_DIMM) 
	move  a3,ra;
	move  s2,a0;  //store a0: i2C DEVICE id
	move  s3,a1;  //store a1: REG offset in i2c dev	

	li	a1, 2;
	bal	i2cread;
	nop;
	beq	v0,SPD_NODEVICE, out;
	nop;
	

	/* set DDR type @ s1[7:7] */
	/* assumed here v0 should be 0xc or 0x8 */
	//bne	v0,SPD_TYPEDDR3, ddr2  
	andi	v0,0x4;
	srl	v0,0x2;
	sll	v0,DDRTYPE_MASK ;
	or	s1,v0;


	/* set  CONTROLLER_SELECT@ s1[3:2] */
	/* a1 should set to be MC0_USED or MC1_USD */
	/* Firt check whether BOTH MC0 and MC1 used, if
       true,set s1[3:2] = 2b'00
	*/
	or	s1,s3;
	andi  t1,s1,0xc;
	bne	t1,0xc,10f;
	nop
	subu  s1,0xc

10:
	/* set SIZE_PER_CONTROLLE @ s1[6:4] */
	/* step 1: read out number of DIMMS Ranks(CS) */
	move a0, s2;  //store a1: REG offset in i2c dev	
	li	a1, 5;
	bal	i2cread;
	nop;
	andi  v0,0x3; // MASK for DIMMS Ranks(CS)
	move  t2,v0;

	//bez t2,out
	nop
	addi t2,0x1

	/* step 2: read out  DIMMS size */
	move  a0,s2;
	li	a1, 31;
	bal	i2cread;
	nop;
	bne	v0,SPD_DDR2SIZE_512M, 11f;
	nop;
	//ori	s1,DDR2SIZE_512M_MASK;	 
	li	v0, 0x20000000
	b 1f;
	nop;
11:
	bne	v0,SPD_DDR2SIZE_1G, 12f;
	nop;
	//ori	s1,DDR2SIZE_1G_MASK;
	li	v0, 0x40000000
	b 1f;
	nop;
12:
	bne	v0,SPD_DDR2SIZE_2G, 1f;
	nop;
	//ori	s1,DDR2SIZE_2G_MASK;	// v0: 0x80 means 1G
	li	v0, 0x80000000;
1:
	/* step 3: calculate each mem SIZE of one slot*/
	beq	t2,0x1,21f;
	nop
	sll	v0,0x1
	//multu  v0,t2;

21:
	bne	v0,0x20000000, 13f
	nop;

	/* check whther this channel is smaller than others
	 if smller: remove other bits,and set DDRPERSZ_512M 
	 else: do nothing, don't set PERSIZE_BIT
	// defalut setting 512M leaset MEM size  
	*/

	andi t1,s1,DDR2SIZE_MASK; 
	beqz  t1,211f;
	nop
	bleu  t1,DDR2SIZE_512M_MASK, 15f;
	nop
	li	t1,DDR2SIZE_MASK;
	not	t2,t1;
	and s1,t2;

211:
	ori	s1,DDRPERSZ_512M ;
	b 15f;
	nop
13:
	bne	v0,0x40000000, 14f
	nop

	/* check whther this channel is smaller than others
	 if smller: remove other bits,and set DDRPERSZ_1G 
	 else: do nothing, don't set PERSIZE_BIT
	*/
	andi t1,s1,DDR2SIZE_MASK; 
	beqz  t1,131f;
	nop
	bleu  t1,  DDR2SIZE_1G_MASK,15f
	nop
	li	t1,DDR2SIZE_MASK;
	not	t2,t1;
	and s1,t2;

131:
	ori	s1,DDRPERSZ_1G;
	b 15f;
	nop

14:/* only support 512M,1G,2G per DIMM now */
	bne	v0,0x80000000, 15f
	nop

	/* check whther this channel is smaller than others
	 if smller: remove other bits,and set DDRPERSZ_2G 
	 else: do nothing, don't set PERSIZE_BIT
	*/
	andi t1,s1,DDR2SIZE_MASK; 
	beqz  t1,141f;
	nop
	bleu  t1,  DDR2SIZE_2G_MASK,15f
	nop
	li	t1,DDR2SIZE_MASK;
	not	t2,t1;
	and s1,t2;

141:
	ori	s1,DDRPERSZ_2G;
	

15:
	/* check whether MC0 or MC1 used to set CS_MAP */
	bne	s3,	MC0_USED, 2f;
	nop;
	/* set DDR MC0_CS_MAP @s1[11:8] */
	ori	s1, 0x400 // at leaset one bit is selected
	move  a0,s2;
	li	a1, 5;
	bal	i2cread;
	nop;
	andi  v0,0x1;
	beq	v0,0x0,16f;
	nop;
	ori	s1,0x800;

16:
	/* set DDR MC0_COL_SIZE @s1[18:16] */
	move  a0,s2;
	li	a1, 4;
	bal	i2cread;
	nop;
	li	t0,14;
	sub	t0,v0;
	sll	t0,0x10;
	or	s1,t0;

	/* set MC0_EIGHT_BANK @s1[19] */
	move  a0,s2;
	li	a1, 17;
	bal	i2cread;
	nop;
	andi  v0,0x8;
	srl	v0,0x3;
	sll	v0,0x13;  //MC0_EIGHT_BANK shift
	or	s1,v0;

	/* set DDR MC0_ROW_SIZE @s1[22:20] */
	move  a0,s2;
	li	a1, 3;
	bal	i2cread;
	nop;
	li	t0,15;
	sub	t0,v0;
	sll	t0,0x14;
	or	s1,t0;

    /* set MC0_ECC bit @s1[32] */
	move  a0,s2;
	li	a1, 11;
	bal	i2cread;
	li	t0,0x2;
	andi  v0,0x2;
	srl	v0,0x1;
	dsll	v0,32;
	//dli v0,0x100000000 // for test
	or	s1,v0;

	/* set DIMM Type information @s1[33:33] */
	move  a0,s2;
	li	a1, 20;
	bal	i2cread;
	nop;
	andi  v0,0x1; // only to check whether in Register Dual In_line memory module
	dsll	v0,33;
	or	s1,v0;


	b out;
	nop;

2:	/* MC1_USED */
#if 1

	/* set DDR MC1_CS_MAP @s1[15:12] ?  */
	ori	s1, 0x4000  // at leaset one bit is selected
	move  a0,s2;
	li	a1, 5;
	bal	i2cread;
	nop;
	andi  v0,0x1;
	beq	v0,0x0, 26f
	nop
	ori	s1,0x8000
26:
	
	/* set DDR MC1_COL_SIZE @s1[26:24] */
	move  a0,s2;
	li	a1, 4;
	bal	i2cread;
	nop;
	li	t0,14;
	sub	t0,v0;
	sll	t0,0x18;
	or	s1,t0;

	/* set MC1_EIGHT_BANK @s1[27] ? */
	move  a0,s2
	li	a1, 17;
	bal	i2cread;
	nop;
	andi  v0,0x8;
	srl	v0,0x3;
	sll	v0,27;  //MC0_EIGHT_BANK shift
	or	s1,v0;

	/* set DDR MC1_ROW_SIZE @s1[30:28] */
	move  a0,s2;
	li	a1, 3;
	bal	i2cread;
	nop;
	li	t0,15;
	sub	t0,v0;
	sll	t0,0x1c;
	or	s1,t0;

	/* set MC1_ECC bit @s1[34] */
	move  a0,s2;
	li	a1, 11;
	bal	i2cread;
	li	t0,0x2;
	andi  v0,0x2;
	srl	v0,0x1;
	dsll	v0,34;
	or	s1,v0;

	/* set DIMM Type information @s1[35:35] */
	move  a0,s2;
	li	a1, 20;
	bal	i2cread;
	nop;
	and	v0,0x1;// only to check whether in Register Dual In_line memory module
	dsll	v0,35;
	or	s1,v0;

#endif

out:/* out of MC0_CS_MAP or MC1_CS_MAP */
	//jr	ra
	jr	a3
	nop
END(PROBE_DIMM)

#endif