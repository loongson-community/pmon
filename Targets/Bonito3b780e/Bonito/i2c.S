#define BONITO_HTIO_BASE_VA 0x90000efdfc000000
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

#define SPD_NODEVICE  0xff
#define SPD_TYPEDDR2  0x8
#define SPD_TYPEDDR3  0xb
#define SPD_NODIMM	  0x0

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

/************************************************************
 * Input: a0, i2c device ID; a1, in MC0 or MC1 flags
 * Output: v0, equals orignal s1, included all DIMM info
************************************************************/
LEAF(PROBE_DIMM) 
	move  a2,ra;
	move  s2,a0;  //store a0: i2C DEVICE id, s2 can be only used before mszie set
	move  s3,a1;  //store a1: MC0 or MC1 used flags

	li	a1, 2;
	bal	i2cread;
	nop;
	beq	v0,SPD_TYPEDDR3, 4f;
	nop;
	beq	v0,SPD_TYPEDDR2, 4f;
	nop;

	xor	v0,v0
	jr	a2
	nop
	
4:
	li	s1,0x0
	/* set DDR type @ s1[7:7] */
	/* assumed here v0 should be 0xb or 0x8 */
	//bne	v0,SPD_TYPEDDR3, ddr2  
	move  t1,v0
	subu  t1, 0x8
	beqz  t1, 400f
	nop
	li	  v0,0x1
	b	  500f
	nop
400:
	li	v0, 0x0
500:
	sll	v0,DDRTYPE_MASK ;
	or	s1,v0;

	/* set  CONTROLLER_SELECT@ s1[3:2] */
	/* a1 should set to be MC0_USED or MC1_USD */
	/* Firt check whether BOTH MC0 and MC1 used, if
       true,set s1[3:2] = 2b'00
	*/
	andi  t1,s3,0xc
	or	s1,t1;
#if 0 /* left done in MERGE_CHANNEL */
	andi  t1,s1,0xc;
	bne	t1,0xc,10f;
	nop
	subu  s1,0xc
#endif

	/* added for DDR3 check */
	// if not ddr3,jump to 100f
	// else, directly set ddr3 size to be 2G
	li	a0, 0x80
	and	a0, s1,a0
	beqz  a0,100f //DDR2
	nop

#if 0 //NOTICE HERE!!!! changed to 1g mem
	ori	s1,DDRPERSZ_1G // DDR3, default size 2G
	b	  15f	
	nop
#else
    //DDR3
	//step 1. calulate SDRAM_CAPCITY (byte)
    move    a0, s2;  //store a1: REG offset in i2c dev  
    li      a1, 4;
    bal     i2cread;
    nop;
    andi    v0, v0, 0xf 
    addi    v0, v0, 25 // 2**25 = 256Mb/8
	move	t2, v0

	//step 2. multi (Primary bus width)
    move    a0, s2;  //store a1: REG offset in i2c dev  
    li      a1, 8;
    bal     i2cread;
    nop;
	andi	v0,0x7
	addi	v0,0x3	//2**3 = 8 bit
	daddu	t2,v0
  
	//step 3. divide (SDARM width)
    move    a0, s2;  //store a1: REG offset in i2c dev  
    li      a1, 7;
    bal     i2cread;
    nop;
	andi	v0,0x7
	addi	v0,0x2	//2**3 = 8 bit
	dsubu	t2,v0

	//step 4. mulip * (number of Ranks)
	li		a0,0x1
	dsll	t2,a0,t2
    move    a0, s2;  //store a1: REG offset in i2c dev  
    li      a1, 7;
    bal     i2cread;
    nop;
	andi	v0,0x38
	srl		v0,v0,0x3
	move	a0,t2
continue_add:
	beqz	v0, calculate_over
	nop
	daddu	t2,t2, a0
	subu	v0,1
	b		continue_add
	nop
calculate_over:

	// step 5. merge memsz t2 into s1

	//jr	ra
//	move  v0,t2
//	jr	a2
//	nop

	dli	t3,0x20000000
	beq	t2,t3, 311f;
	nop;
	dli	t3,0x40000000
	beq	t2,t3, 312f;
	nop;
	dli	t3,0x80000000
	beq	t2,t3, 313f;
	nop;
	dli	t3,0x100000000
	beq	t2,t3, 314f;
	nop;
	dli	t3,0x200000000
	beq	t2,t3, 315f;
	nop;

311:
	li		t2, 0x10
    b    out_probesize  // 512M 
	nop
312:
	li		t2, 0x20	// 1G
    b    out_probesize   
	nop
313:
	li		t2, 0x30	// 2G
    b    out_probesize   
	nop
314:
	li		t2, 0x40	// 4G
    b    out_probesize   
	nop
315:
	li		t2, 0x50	// 8G
    b    out_probesize   
	nop

out_probesize:
    or      s1, s1, t2
    nop 
    b       15f
    nop 
#endif
100:
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

#if 0
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
#endif // let compare size outside
	ori	s1,DDRPERSZ_512M ;
	b 15f;
	nop
13:
	bne	v0,0x40000000, 14f
	nop

#if 0
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
#endif // let compare size outside
	ori	s1,DDRPERSZ_1G;
	b 15f;
	nop

14:/* only support 512M,1G,2G per DIMM now */
	bne	v0,0x80000000, 15f
	nop

#if 0
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
#endif // let compare size outside
	ori	s1,DDRPERSZ_2G;
	

15:

	andi  a0, s1,0x80
	bnez  a0, 1601f	  // jump to ddr3 CS_MAP setting
	nop

	// below is  ddr2 CS_MAP setting
	/* check whether MC0 or MC1 used to set CS_MAP */
	move  a0,s3
	andi  a0,0xc
	bne	a0,	MC0_USED, 2f;
	nop;

	/* set DDR MC0_CS_MAP @s1[11:8] */

	// First,check whether MC0DIMM0 used 
	move  a0,s3
	andi  a0,0xf0
	bne	  a0,0x30,160f
	nop

	// MC0_DIMM0
	ori	s1, 0x100 // at leaset one bit is selected, CS_MAP[0]
	move  a0,s2;
	li	a1, 5;
	bal	i2cread;
	nop;
	andi  v0,0x1;
	beq	v0,0x0,16f;
	nop;
	ori	s1,0x200; // set CS_MAP[1] = 1'b1
	b	16f
	nop

160:
	// MC0_DIMM1
	ori	s1, 0x400 // at leaset one bit is selected, CS_MAP[2]
	move  a0,s2;
	li	a1, 5;
	bal	i2cread;
	nop;
	andi  v0,0x1;
	beq	v0,0x0,16f;
	nop;
	ori	s1,0x800;

16:
	

	/* set DDR2 MC0_COL_SIZE @s1[18:16] */
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
	b	1602f
	nop

1601: //DDR3

	// below is  ddr3 CS_MAP setting

	/* check whether MC0 or MC1 used to set CS_MAP */
	move  a0,s3
	andi  a0,0xc
	bne	a0,	MC0_USED, 2f;
	nop;

	/* set DDR MC0_CS_MAP @s1[11:8] */

	// First,check whether MC0DIMM0 used 
	move  a0,s3
	andi  a0,0xf0
	bne	  a0,0x30,170f
	nop

	// MC0_DIMM0
	ori	s1, 0x100 // at leaset one bit is selected, CS_MAP[0]
	move  a0,s2;
	li	a1, 7;
	bal	i2cread;
	nop;
	andi  v0,0x38;
	beq	v0,0x0,17f;
	nop;
	ori	s1,0x200; // set CS_MAP[1] = 1'b1
	b	17f
	nop

170:
	// MC0_DIMM1
	ori	s1, 0x400 // at leaset one bit is selected, CS_MAP[2]
	move  a0,s2;
	li	a1, 7;
	bal	i2cread;
	nop;
	andi  v0,0x38;
	beq	v0,0x0,17f;
	nop;
	ori	s1,0x800;


17:
	/* set DDR3 MC0_COL_SIZE @s1[18:16] */
	move  a0,s2;
	li	a1, 5;
	bal	i2cread;
	nop;
	andi  v0,0x7
	addi  v0,0x9
	li	t0,14;
	sub	t0,v0;
	sll	t0,0x10;
	or	s1,t0;

	/* set MC0_EIGHT_BANK @s1[19] */
	li	a0,0x80000
	or	s1,a0; // at leaset 8 banks

	/* set DDR MC0_ROW_SIZE @s1[22:20] */
	move  a0,s2;
	li	a1, 5;
	bal	i2cread;
	nop;
	andi  v0,0x38
	srl	  v0,0x3
	li	t0,3;
	sub	t0,v0;
	sll	t0,0x14;
	or	s1,t0;

#if 0
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
#endif

1602:

///////////////////
	b out;
	nop;

2:	/* MC1_USED */

	andi  a0, s1,0x80
	bnez  a0, 2601f
	nop

	// below begin to set DDR2 CS_MAP
	// First,check whether MC1DIMM0 used 
	move  a0,s3
	andi  a0,0xf0
	bne	  a0,0x30,260f
	nop

	/* set DDR MC1_CS_MAP @s1[15:12] ?  */
	//below check for MC1_DIMM0
	ori	s1, 0x1000 // at leaset one bit is selected, CS_MAP[0]
	move  a0,s2;
	li	a1, 5;
	bal	i2cread;
	nop;
	andi  v0,0x1;
	beq	v0,0x0,26f;
	nop;
	ori	s1,0x2000; // set CS_MAP[1] = 1'b1
	b	26f
	nop

260: // MC1_DIMM1
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
	and	v0,0x1;	 // only to check whether in Register Dual In_line memory module
	dsll	v0,35;
	or	s1,v0;
	b	2602f
	nop

2601:

	// below begin to set DDR3 CS_MAP

	// First,check whether MC1DIMM0 used 
	move  a0,s3
	andi  a0,0xf0
	bne	  a0,0x30,270f
	nop

	/* set DDR MC1_CS_MAP @s1[15:12] ?  */
	//below check for MC1_DIMM0
	ori	s1, 0x1000 // at leaset one bit is selected, CS_MAP[0]
	move  a0,s2;
	li	a1, 7;
	bal	i2cread;
	nop;
	andi  v0,0x38;
	beq	v0,0x0,27f;
	nop;
	ori	s1,0x2000; // set CS_MAP[1] = 1'b1
	b	27f
	nop

270: // MC1_DIMM1
	ori	s1, 0x4000  // at leaset one bit is selected
	move  a0,s2;
	li	a1, 7;
	bal	i2cread;
	nop;
	andi  v0,0x38;
	beq	v0,0x0, 27f
	nop
	ori	s1,0x8000
27:
	
	/* set DDR3 MC1_COL_SIZE @s1[26:24] */
	move  a0,s2;
	li	a1, 5;
	bal	i2cread;
	nop;
	andi  v0,0x7
	addi  v0,0x9
	li	t0,14;
	sub	t0,v0;
	sll	t0,0x18;
	or	s1,t0;

	/* set MC0_EIGHT_BANK @s1[19] */
	li	a0,0x8000000
	or	s1,a0 // at leaset 8 banks

	/* set DDR MC0_ROW_SIZE @s1[30:28] */
	move  a0,s2;
	li	a1, 5;
	bal	i2cread;
	nop;
	andi  v0,0x38
	srl	  v0,0x3
	li	t0,3;
	sub	t0,v0;
	sll	t0,0x1c;
	or	s1,t0;

#if 0
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
#endif

2602:

out: /* out of MC0_CS_MAP or MC1_CS_MAP */
	//jr	ra
	move  v0,s1
	jr	a2
	nop
END(PROBE_DIMM)

/*******************************************************************
 * input  a0: deteced info on DIMM bonded to CS[1:0], 0 means no dimm
 * input  a1: deteced info on DIMM bonded to CS[3:2],0 means no dimm
 * output v0: merged a0/a1 together,  0 means no mem 
 *******************************************************************/
	.global MERGE_DIMM 
	.ent    MERGE_DIMM
	.set    noreorder
	.set    mips3

MERGE_DIMM:
	/* check whether CS[1:0] and CS[3:2] tied to DIMM */
	bne	  a0,0x0,10f
	nop
	move  v0,a1	  /* no DIMM on CS[1:0], return a1 */
	jr	  ra
	nop
	
10:
	bne	  a1,0x0,12f
	nop
	move  v0,a0	  /* no DIMM on CS[3:2], return a0 */
	jr	  ra
	
	/* both CS[3:2] and CS[1:0] conncted to DIMM */
12:

	/* check whther both DD2 or DD3 */
	andi  t0,a0,0x80
	andi  t1,a1,0x80
	bne  t0,t1,err
	nop
	beq	a2, MC1_USED,1f
	nop

	############## first check MC0 ###########

	/* check whther both Register or Unbuffered */
	dli	  t2, 0x200000000
	and  t0,a0,t2
	and  t1,a1,t2
	bne  t0,t1, err
	nop

	/* check whther has same row */
	dli	  t2, 0x700000
	and  t0,a0,t2
	and  t1,a1,t2
	bne  t0,t1, err
	nop


	/* check whther has same bank */
	dli	  t2, 0x80000
	and  t0,a0,t2
	and  t1,a1,t2
	bne  t0,t1, err
	nop
	
	/* check whther has same col */
	dli	  t2, 0x70000
	and  t0,a0,t2
	and  t1,a1,t2
	bne  t0,t1, err
	nop
	
	b	2f
	nop

1:
  	############## now check MC1 ###########
	/* check whther both Register or Unbuffered */
#if 1
	dli	  t2, 0x800000000
	and  t0,a0,t2
	and  t1,a1,t2
	bne  t0,t1, err
	nop
#endif

	/* check whther has same row */
	dli	  t2, 0x70000000
	and  t0,a0,t2
	and  t1,a1,t2
	bne  t0,t1, err
	nop

	/* check whther has same bank */
	dli	  t2, 0x8000000
	and  t0,a0,t2
	and  t1,a1,t2
	bne  t0,t1, err
	nop
	
	/* check whther has same col */
	dli	  t2, 0x7000000
	and  t0,a0,t2
	and  t1,a1,t2
	bne  t0,t1, err
	nop
	
2: ## now merge mem size together of DIMM0 and DIMM1 on the same MC ##
	/* set v0 with a0 first */

	move  v0, a0

	########## set ddr autocalily decrease freq flag #############
	dli	  t3, 0x1000000000
	or	  v0,t3

	/* merge CS_MAP together */
	// shift right 2 bits, NOTICE here assume cs[3:2] connected DIMMB
	//andi  t0, a0, 0xc00 
	//srl	  t0,0x2
	//or	v0,t0

	beq	a2, MC0_USED,221f
	nop
	andi  t0, a1, 0xc000  // MC1
	b	222f
	nop
221:
	andi  t0, a1, 0xc00   // MC0
222:
	or	v0,t0

	/* merge MIMM mem size together */
	dli	  t3,0x1
	andi  t0,a0,0x70;
	srl	  t0,0x4
	addi  t0,0x1c
	dsll  t4, t3,t0  /* t4: DIMM0 size */
	
	andi  t1,a1,0x70;
	srl	  t1,0x4
	addi  t1,0x1c
	dsll  t5, t3,t1 /* t5: DIMM1 size */

	daddu t6,t4,t5	/* total mem size of DIMM0 and DMME1 */
	li	  t7,0x1c   /* 0x1 << 0x1c = 256M */
	dsrl  t6, t6,t7
	
	/* now calculate out SIZE_PER_CONTROLER */
	li	  t4,0x0  /* t4 =log2(memsize/256M)  */
4:
	dsrl  t6,0x1
	addu  t4,0x1
	bne	  t6,0x1,4b
	nop

	// first clean orignal SIZE_PER_CONTROLLER
	dli	  t5,0xffffffff8f
	and  v0,t5
	
	sll	  t4,0x4
	or	  v0, t4
	jr	  ra
	nop

err:
	li	v0,0x0;
	jr	  ra
	nop
    .end    MERGE_DIMM


/*******************************************************************
 * input  a0: deteced info on DIMM bonded to MC0, 0 means no dimm
 * input  a1: deteced info on DIMM bonded to MC1, 0 means no dimm
 * output v0: availabe mem size from compare a0/a1, passed to L2XBAR
 *******************************************************************/
LEAF(CALCULATE_MEMSZ)
	
	andi  t0,a0,0x70
	andi  t1,a1,0x70

	srl	  t0,0x4
	srl	  t1,0x4

	beq	  t0,$0,1f
	nop
	beq	  t1,$0,2f
	nop
	
	########## calculate and set memsz #################
	ble	  t0, t1, 3f
	nop
	move  t0,t1	  
3: /* t0 used to record smller size  */
	li	  t1, 0x1
	addu  t0, 29;/* both MC0 and CM1 used */	  
	dsll  t1, t1,t0	  
	dsubu  t1, 0x1000000 /* remain 16M according linux kernel */
	move  v0, t1

	b	  4f
	nop

1: // MC0 has no DIMM, only MC1 used
    ########## calculate and set memsz #################
	li	  t0, 0x1
	addu  t1, 28
	dsll  t0, t1	  
	dsubu  t0, 0x1000000 /* remain 16M according linux kernel */
	move  v0, t0
	b	  4f
	nop
	
	
2: // MC1 has no DIMM, only MC0 used
    ########## calculate and set memsz #################
	li	  t1, 0x1
	addu  t0, 28
	dsll  t1, t0	  
	dsubu  t1, 0x1000000 /* remain 16M according linux kernel */
	move  v0, t1
	b	  4f
	nop

4:
	jr	  ra
END(CALCULATE_MEMSZ)


/*******************************************************************
 * input  a0: deteced info on  MC0, 0 means no mem
 * input  a1: deteced info on  MC1, 0 means no mem
 * output v0: availabe mem size from compare a0/a1, passed to L2XBAR
 * The case when MC0 and MC1 not the same DDR2 or DDR3 not concerned
 *******************************************************************/

LEAF(MERGE_CHANNEL)
	#move  v0,a0
	or	  v0,a1,a0
	
	/* check whter both MCO and MC1 used, set s1[3:2]=2'b00 */
	andi  t0,v0,0xc
	bne  t0,0xc,3f
	nop
	li	  t0,0xfffffff3 /* set s1[3:2]=2'b00 */
	and  v0,v0,t0

3:
	andi  t0,a0,0x70
	andi  t1,a1,0x70

	beq	  t0,$0,1f
	nop
	beq	  t1,$0,2f
	nop
	
	########## calculate and merge channel memsz #################
	ble	  t0, t1, 5f
	nop
	move  t0,t1	  
5: /* t0 used to record smller size  */
	li	  t2,0xffffff8f
	and  v0,t2
	or	  v0,v0,t0

	b	  4f
	nop

1: // MC0 has no DIMM, only MC1 used
    ########## calculate and set memsz #################
	li	  t2,0xffffff8f
	and   v0,t2
	or	  v0,v0,t1
	b	  4f
	nop
	
	
2: // MC1 has no DIMM, only MC0 used
    ########## calculate and set memsz #################
	li	  t2,0xffffff8f
	and  v0,t2
	or	  v0,v0,t0
	b	  4f
	nop

4:
	jr	  ra
	nop
END(MERGE_CHANNEL)
