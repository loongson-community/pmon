/************************
    function PROBE_DIMM changed by cxk on 09/26/2010
    Single CHIP & USE_SB_I2C mode & DDR2 DIMM can work normal
    DDR3 SDRAM need to be finished!!
************************/

#define BONITO_HTIO_BASE_VA 0x90000efdfc000000
#define CHIP1_BASE_ADDRESS	0x0000100000000000
#define CHIP2_BASE_ADDRESS	0x0000200000000000
#define CHIP3_BASE_ADDRESS	0x0000300000000000

#ifndef  MULTI_CHIP

#ifdef  USE_SB_I2C
LEAF(i2cread)
/***************
use register:
v0, v1
a0, a1
***************/
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
#else

#ifdef  USE_GPIO_I2C
#include "i2c_firewall.S"
#endif

#endif

/*************************
PROBE_DIMM:
function: probe the given slot(I2C device id is given in a0),
          if there is no DIMM in this slot, clear SDRAM_TYPE to 0,
          else read the DIMM infor from the SPD and store the infor in s1(CS_MAP at [17:16], 
          MEMSIZE at [11:8]).
use register:
a0,a1,a2,a3
v0, v1
t5, t6

t7: store ra
a0: input, i2c device id(don't change it).
a1: register offset of i2c device	
*************************/
#if 0   //debug code, used in PROBE_DIMM, after read i2c, print v0
    //debug------------
    move    t5, a0
    move    t6, v0
    TTYDBG("\r\nBanks: ")
    move    a0, t6
    bal     hexserial
    nop
    TTYDBG("\r\n")
    move    a0, t5
    move    v0, t6
    //------------debug

    //Test whether i2cread will dead loop
    move    t5, a0
    TTYDBG("\r\nIn Probe_DIMM, before i2cread!")
    move    a0, t5
    dli     a1, 0
    bal     i2cread
    nop
    move    t5, a0
    TTYDBG("\r\nIn Probe_DIMM, after i2cread!")
    move    a0, t5
#endif
LEAF(PROBE_DIMM) 
    move    t7, ra

//read the i2c spd for learn,read data is abandon
    dli     a1, 0
    bal     i2cread
    nop

//probe SDRAM type, if SDRAM type error, repeat t6 times.
    dli     t6, 8   //max probing times(t6)
1:
//delay some time
    dli     t5, 0xfff
2:
    daddiu  t5, t5, -1
    bnez    t5, 2b
    nop

    daddiu  t6, t6, -1

    dli	    a1, 2
	bal     i2cread
	nop
    //only bit[7:0] used
    andi    v0, v0, 0xff
	/* v0 should be 0xb or 0x8,else error DIMM type */
    dli     t5, 0x08
    beq     v0, t5, DDR2
    nop
    dli     t5, 0x0B
    beq     v0, t5, DDR3
    nop
    //this time probe error
    bnez    t6, 1b
    nop
3:
    TTYDBG("\r\nNO DIMM in this slot.\r\n")
    b       ERROR_TYPE
    nop
DDR2:
    dli     t5, 0x2
    dsll    t5, t5, SDRAM_TYPE_OFFSET
    or      s1, s1, t5
//probe DIMM_TYPE
    dli     a1, 20
    bal     i2cread
    nop
    //only bit[5:0] used
    andi    v0, v0, 0x3f
    //here just recognize RDIMM and UDIMM
    dli     t5, 0x01
    beq     v0, t5, 1f
    nop
    dli     t5, 0x02
    beq     v0, t5, 2f
    nop
    TTYDBG("\r\nERROR: DIMM type is not in support range(UDIMM or RDIMM).\r\n")
    b       ERROR_TYPE
    nop
1:  //RDIMM
    dli     t5, 0x1
    dsll    t5, t5, DIMM_TYPE_OFFSET
    or      s1, s1, t5
    b       3f
    nop
2:  //UDIMM

3:  
//probe DIMM ECC
    dli     a1, 11
    bal     i2cread
    nop
    //only bit[2:0] used
    andi    v0, v0, 0x07
    //here just recognize ECC or not
    andi    t5, v0, 0x2
    dsrl    t5, t5, 1
    dsll    t5, t5, DIMM_ECC_OFFSET
    or      s1, s1, t5
//probe SDRAM_ROW_SIZE
    dli     a1, 3
    bal     i2cread
    nop
    //only bit[7:0] used
    andi    v0, v0, 0xff
    //v0 should < 15
    andi    v0, v0, 0x0f
    dli     t5, 15
    subu    t5, t5, v0
    dsll    t5, t5, ROW_SIZE_OFFSET
    or      s1, s1, t5
//probe SDRAM_COL_SIZE
    dli     a1, 4
    bal     i2cread
    nop
    //only bit[7:0] used
    andi    v0, v0, 0xff
    //v0 should < 14
    andi    v0, v0, 0x0f
    dli     t5, 14
    subu    t5, t5, v0
    dsll    t5, t5, COL_SIZE_OFFSET
    or      s1, s1, t5
//probe SDRAM BANK number
    dli     a1, 17 
    bal     i2cread
    nop
    //bit[7:0] used
    andi    v0, v0, 0xff
    //here just recognize 4 banks or 8 banks
    dli     t5, 0x08
    beq     v0, t5, 1f
    nop
    dli     t5, 0x04
    beq     v0, t5, 2f
    nop
    TTYDBG("\r\nERROR: SDRAM Banks number is not in support range(4 or 8).\r\n")
    b       ERROR_TYPE
    nop
1:  //8 banks
    dli     t5, 0x1
    dsll    t5, t5, EIGHT_BANK_OFFSET
    or      s1, s1, t5
    b       3f
    nop
2:  //4 banks

3:  
//probe DIMM Ranks
    dli     a1, 5 
    bal     i2cread
    nop
    //only bit[2:0] used
    andi    v0, v0, 0x7
    //here just recognize 1 ranks or 2 ranks
    dli     t5, 0x0
    beq     v0, t5, 1f
    nop
    dli     t5, 0x1
    beq     v0, t5, 2f
    nop
    TTYDBG("\r\nERROR: DIMM Ranks number is not in support range(1 or 2).\r\n")
    b       ERROR_TYPE
    nop
1:  //1 rank
    dli     t5, 0x1
    b       3f
    nop
2:  //2 ranks
    dli     t5, 0x3
3:  
    dsll    t5, t5, MC_CS_MAP_OFFSET
    or      s1, s1, t5
//probe DIMM Memory SIZE
    dli     a1, 31
    bal     i2cread
    nop
    //only bit[7:0] used
    andi    v0, v0, 0xff
//currently only support 1 rank >= 512M & 2 ranks <= 4G, else assume there is NO DIMM
    move    t5, v0
    dli     t6, 0xe0
    and     t5, t5, t6
    beqz    t5, 1f
    nop
    //1 rank<= 512M
    dsrl    t5, t5, 7
    beqz    t5, 2f  //1 rank < 512M, error
    nop 
    //1 rank = 512M, double the MEMSIZE if there are 2 ranks
    GET_MC_CS_MAP
    dsrl    a1, a1, 1   //test the cs 1
    beqz    a1, 5f
    nop
    //double the size
    dsll    t5, t5, 1
5:    
    dsll    t5, t5, DIMM_MEMSIZE_OFFSET
    or      s1, s1, t5
    b       3f
    nop
1:  //1 rank >= 1G
    //double the MEMSIZE if there are 2 ranks
    GET_MC_CS_MAP
    dsrl    a1, a1, 1   //test the cs 1
    beqz    a1, 5f
    nop
    //double the size
    dsll    v0, v0, 1
5:
    dli     t5, 0x7
    and     t5, t5, v0
    beqz    t5, 2f  //DIMM SIZE > 4G
    nop
    dsll    t5, v0, 1   //MEMSIZE is 512M/unit, v0 is 1G/unit
    dsll    t5, t5, DIMM_MEMSIZE_OFFSET
    or      s1, s1, t5
    b       3f
    nop
2:
//1 rank < 512M or DIMM SIZE > 4G, errors(clear SDRAM_TYPE, assume there is NO DIMM in this slot)
    dli     t5, 0x3
    dsll    t5, t5, SDRAM_TYPE_OFFSET
    not     t5, t5
    and     s1, s1, t5
    TTYDBG("\r\nERROR: DIMM size is not in support range(512M~4G).\r\n")
    b       ERROR_TYPE
    nop
3: 
//DDR2 probe finished
    b       end
    nop
DDR3:  //DDR3 SDRAM
    dli     t5, 0x3
    dsll    t5, t5, SDRAM_TYPE_OFFSET
    or      s1, s1, t5
//!!!!!!need to be completed    
    b       end
    nop

ERROR_TYPE:
//no DIMM or unrecognized DIMM in this slot
    dli     t5, 0x3
    dsll    t5, t5, SDRAM_TYPE_OFFSET
    not     t5, t5
    and     s1, s1, t5
end:
	jr	t7
	nop
END(PROBE_DIMM)

#else   //Multi chips
//not checked now!!!!!!!!!!1
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
