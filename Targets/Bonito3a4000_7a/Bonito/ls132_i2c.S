/*************************
 * ls7a
*************************/

//#define LS3A4000_I2C1_REG_BASE	0xbfe00130
#define LS3A4000_I2C0_REG_BASE		0xbfe00120
#define LS132_I2C_REG_BASE		LS3A4000_I2C0_REG_BASE

#define LS132_I2C_PRER_LO_REG		(LS132_I2C_REG_BASE + 0x0)
#define LS132_I2C_PRER_HI_REG		(LS132_I2C_REG_BASE + 0x1)
#define LS132_I2C_CTR_REG   		(LS132_I2C_REG_BASE + 0x2)
#define LS132_I2C_TXR_REG   		(LS132_I2C_REG_BASE + 0x3)
#define LS132_I2C_RXR_REG    		(LS132_I2C_REG_BASE + 0x3)
#define LS132_I2C_CR_REG     		(LS132_I2C_REG_BASE + 0x4)
#define LS132_I2C_SR_REG     		(LS132_I2C_REG_BASE + 0x4)

#define CR_START			0x80
#define CR_STOP				0x40
#define CR_READ				0x20
#define CR_WRITE			0x10
#define CR_ACK				0x8
#define CR_IACK				0x1

#define SR_NOACK			0x80
#define SR_BUSY				0x40
#define SR_AL				0x20
#define SR_TIP				0x2
#define SR_IF				0x1

#define PV3205_ADDR0			(16 << 1)
#define PV3205_ADDR1			(17 << 1)

#ifdef LOONGSON_BOARD
#define MPS_ADDR			(0x3b << 1)
#else //EASCS board
#define MPS_ADDR			(0x6b << 1)
#endif

LEAF(ls132_i2cinit)
	.set    push
	.set    noreorder
	.set    mips32
	//LPB clock_a,SCL clock_s,prescale = clock_a / (4 * clock_s);
	li	v0, LS132_I2C_CTR_REG
	lb	v1, 0x0(v0)
	and	v1, ~(1 << 7)
	sb	v1, 0x0(v0)

	//the pv3205 device required i2c rate at least 10k
	li	v1, 0x53
	li	v0, LS132_I2C_PRER_LO_REG
	sb	v1, 0x0(v0)

	li	v1, 0x2
	li	v0, LS132_I2C_PRER_HI_REG
	sb	v1, 0x0(v0)

	li	v0, LS132_I2C_CTR_REG
	lb	v1, 0x0(v0)
	or	v1, (1 << 7)
	sb	v1, 0x0(v0)

	jr	ra
	nop
	.set pop
END(ls132_i2cinit)

LEAF(ls132_i2cwrite)
/*
 * use register:
 *	v0, v1
 *	a0, a1
 *	input: a0,a1,a2
 *	a0: device ID
 *	a1: register offset/command
 *	a2: configure value
 *	v0: return value
 */
	.set    push
	.set    noreorder
	.set    mips32
/*i2c_send_addr*/
	/* load device address */
	move	v1, a0
	li	v0, LS132_I2C_TXR_REG
	sb	v1, 0x0(v0)

	/* send start frame */
	li	v1, CR_START | CR_WRITE
	li	v0, LS132_I2C_CR_REG
	sb	v1, 0x0(v0)

	/* wait send finished */
//	i2c_wait_tip
	li	v0, LS132_I2C_SR_REG
1:
	lbu	v1, 0x0(v0)
	andi	v1, v1, SR_TIP
	bnez	v1, 1b
	nop

	//check ACK
	lbu	v1, 0x0(v0)
	andi	v1, v1, SR_NOACK
	bnez	v1, 3f
	nop

	/* load data(offset/command) to be send */
	move	v1, a1
	li	v0, LS132_I2C_TXR_REG
	sb	v1, 0x0(v0)

	/* send data frame */
	li	v1, CR_WRITE
	li	v0, LS132_I2C_CR_REG
	sb	v1, 0x0(v0)

	/* wait send finished */
//	i2c_wait_tip
	li	v0, LS132_I2C_SR_REG
1:
	lbu	v1, 0x0(v0)
	andi	v1, v1, SR_TIP
	bnez	v1, 1b
	nop

	//check ACK
	lbu	v1, 0x0(v0)
	andi	v1, v1, SR_NOACK
	bnez	v1, 3f
	nop
/*i2c_send_addr*/

/* i2c_write_word*/
/*i2c tx byte*/
	/* load configure value */
	move	v1, a2
	li	v0, LS132_I2C_TXR_REG
	sb	v1, 0x0(v0)

	/* send start frame */
	li	v1, CR_WRITE
	li	v0, LS132_I2C_CR_REG
	sb	v1, 0x0(v0)

	/* wait send finished */
//	i2c_wait_tip
	li	v0, LS132_I2C_SR_REG
1:
	lbu	v1, 0x0(v0)
	andi	v1, v1, SR_TIP
	bnez	v1, 1b
	nop

	//check ACK
	lbu	v1, 0x0(v0)
	andi	v1, v1, SR_NOACK
	bnez	v1, 3f
	nop
/*i2c tx byte*/
/*i2c tx byte*/
	/* load configure value */
	move	v1, a2
	li	v0, LS132_I2C_TXR_REG
	srl	v1, v1, 8
	sb	v1, 0x0(v0)

	/* send start frame */
	li	v1, CR_WRITE
	li	v0, LS132_I2C_CR_REG
	sb	v1, 0x0(v0)

	/* wait send finished */
//	i2c_wait_tip
	li	v0, LS132_I2C_SR_REG
1:
	lbu	v1, 0x0(v0)
	andi	v1, v1, SR_TIP
	bnez	v1, 1b
	nop

	//check ACK
	lbu	v1, 0x0(v0)
	andi	v1, v1, SR_NOACK
	bnez	v1, 3f
	nop

	b	4f
	nop
/*i2c tx byte*/
3:
//if this code error we should call i2c stop
/* i2c_stop */
	/* free i2c bus */
	li	v0, LS132_I2C_CR_REG
	li	v1, CR_STOP
	sb	v1, 0x0(v0)
1:
	li	v0, LS132_I2C_SR_REG
	lbu	v1, 0x0(v0)
	andi	v1, v1, SR_BUSY
	bnez	v1, 1b
	nop
	li	v0, 0 //return value check this function
4:
	jr	ra
	nop
	.set pop
END(ls132_i2cwrite)

LEAF(ls132_i2cread)
/*
 * use register:
 *	v0, v1
 *	a0, a1
 *	input: a0,a1
 *	a0: device ID
 *	a1: register offset
 *	v0: return value has 2 bytes data
 *
 */
    .set    push
    .set    noreorder
    .set    mips32
/*i2c_send_addr*/
/*send device addr*/
	/* load device address */
	move	v1, a0
	li	v0, LS132_I2C_TXR_REG
	sb	v1, 0x0(v0)

	/* send start frame */
	li	v1, CR_START | CR_WRITE
	li	v0, LS132_I2C_CR_REG
	sb	v1, 0x0(v0)

	/* wait send finished */
//	i2c_wait_tip
	li	v0, LS132_I2C_SR_REG
1:
	lbu	v1, 0x0(v0)
	andi	v1, v1, SR_TIP
	bnez	v1, 1b
	nop

	//check ACK
	lbu	v1, 0x0(v0)
	andi	v1, v1, SR_NOACK
	bnez	v1, 3f
	nop
/*send device addr*/
	/* load data(offset/command) to be send */
	move	v1, a1
	li	v0, LS132_I2C_TXR_REG
	sb	v1, 0x0(v0)

	/* send data frame */
	li	v1, CR_WRITE
	li	v0, LS132_I2C_CR_REG
	sb	v1, 0x0(v0)

	/* wait send finished */
//	i2c_wait_tip
	li	v0, LS132_I2C_SR_REG
1:
	lbu	v1, 0x0(v0)
	andi	v1, v1, SR_TIP
	bnez	v1, 1b
	nop

	//check ACK
	lbu	v1, 0x0(v0)
	andi	v1, v1, SR_NOACK
	bnez	v1, 3f
	nop
/*i2c_send_addr*/

/* i2c_read_word*/
/*send device addr*/
	/* load device address (dev_addr | 0x1/READ) */
	ori	v1, a0, 0x1
	li	v0, LS132_I2C_TXR_REG
	sb	v1, 0x0(v0)

	/* send start frame */
	li	v1, CR_START | CR_WRITE
	li	v0, LS132_I2C_CR_REG
	sb	v1, 0x0(v0)

	/* wait send finished */
//	i2c_wait_tip
	li	v0, LS132_I2C_SR_REG
1:
	lbu	v1, 0x0(v0)
	andi	v1, v1, SR_TIP
	bnez	v1, 1b
	nop

	 //check ACK
	lbu	v1, 0x0(v0)
	andi	v1, v1, SR_NOACK
	bnez	v1, 3f
	nop
/*send device addr*/

	/* receive data from fifo */
	li	v1, CR_READ
	li	v0, LS132_I2C_CR_REG
	sb	v1, 0x0(v0)

//	i2c_wait_tip
	li	v0, LS132_I2C_SR_REG
1:
	lbu	v1, 0x0(v0)
	andi	v1, v1, SR_TIP
	bnez	v1, 1b
	nop

	/* read data from fifo */
	li	v0, LS132_I2C_RXR_REG
	lbu	a1, 0x0(v0)

	/* receive data from fifo */
	li	v1, CR_READ | CR_ACK
	li	v0, LS132_I2C_CR_REG
	sb	v1, 0x0(v0)

//	i2c_wait_tip
	li	v0, LS132_I2C_SR_REG
1:
	lbu	v1, 0x0(v0)
	andi	v1, v1, SR_TIP
	bnez	v1, 1b
	nop

	/* read data from fifo */
	li	v0, LS132_I2C_RXR_REG
	lbu	v1, 0x0(v0)
	sll	v1, v1,0x8
	addu a1, a1, v1
3:
/* i2c_stop */
	/* free i2c bus */
	li	v0, LS132_I2C_CR_REG
	li	v1, CR_STOP
	sb	v1, 0x0(v0)
1:
	li	v0, LS132_I2C_SR_REG
	lbu	v1, 0x0(v0)
	andi	v1, v1, SR_BUSY
	bnez	v1, 1b
	nop

	move	v0, a1

	jr	ra
	nop
    .set pop
END(ls132_i2cread)

LEAF(i_ctrl_read)
/*
 * use register:
 * a0,a1,k0
 * a0: device ID
 * a1: command
 * k0: save ra
 * v0: return value
 *
 */
	.set    push
	.set    noreorder
	.set    mips32

	/*the read operation only need control one device*/
#ifndef MPS_V
	li	a0, PV3205_ADDR0
#else
	li	a0, MPS_ADDR
#endif
	//li  a1, 0x8b //READ_VOUT
	li	a1, 0x8c //READ_IOUT
	bal	ls132_i2cread
	nop

	move	ra, k0
	jr	ra
	nop

	.set pop
END(i_ctrl_read)
LEAF(v_ctrl_read)
/*
 * use register:
 * a0,a1,k0
 * a0: device ID
 * a1: command
 * k0: save ra
 * v0: return value
 *
 */
	.set    push
	.set    noreorder
	.set    mips32
	move	k0, ra
	bal	ls132_i2cinit
	nop

	LS132_PUTC('V')
	LS132_PUTC(':')

#ifndef MPS_V
	li	a0, PV3205_ADDR0
#else
	li	a0, MPS_ADDR
#endif
	li	a1, 0x8b //READ_VOUT
//	li	a1, 0x8c //READ_IOUT
	bal	ls132_i2cread
	nop
	move	a0, v0
	bal	ls132_hexserial
	nop
#ifndef MPS_V
	li	a0, PV3205_ADDR1
	li	a1, 0x8b //READ_VOUT
//	li	a1, 0x8c //READ_IOUT
	bal	ls132_i2cread
	nop
	move	a0, v0
	bal	ls132_hexserial
	nop
#endif
	move	ra, k0
	jr	ra
	nop
	.set pop
END(v_ctrl_read)

LEAF(v_ctrl)
/*
 * v0 give the voltage level
 * use register:
 * a0,a1,a2,k0
 * a0: device ID
 * a1: command
 * a2: command value
 * k0: save ra
 * v0: return value
 *
 */
	.set    push
	.set    noreorder
	.set    mips32
	move	k0, ra

	li	a0, LEVEL0
	bne	a0, v0, 1f
	nop

	li	a2, V_LEVEL0  //a2 store Voltage configure value
	b	4f
	nop

1:
	li	a0, LEVEL1
	bne	a0, v0, 1f
	nop

	li	a2, V_LEVEL1  //a2 store Voltage configure value
	b	4f
	nop
1:
	li	a0, LEVEL2
	bne	a0, v0, 1f
	nop

	li	a2, V_LEVEL2  //a2 store Voltage configure value
	b	4f
	nop
1:
	li	a0, LEVEL3
	bne	a0, v0, 1f
	nop

	li	a2, V_LEVEL3  //a2 store Voltage configure value
	b	4f
	nop
1:
	li	a0, LEVEL4
	bne	a0, v0, 1f
	nop

	li	a2, V_LEVEL4  //a2 store Voltage configure value
	b	4f
	nop
1:
	li	a0, LEVEL5
	bne	a0, v0, 1f
	nop

	li	a2, V_LEVEL5  //a2 store Voltage configure value
	b	4f
	nop
1:
	li	a0, LEVEL6
	bne	a0, v0, 1f
	nop

	li	a2, V_LEVEL6  //a2 store Voltage configure value
	b	4f
	nop
1:
	li	a0, LEVEL7
	bne	a0, v0, 1f
	nop

	li	a2, V_LEVEL7  //a2 store Voltage configure value
	b	4f
	nop
1:
	li	a0, LEVEL8
	bne	a0, v0, 1f
	nop

	li	a2, V_LEVEL8  //a2 store Voltage configure value
	b	4f
	nop
1:
	li	a0, LEVEL9
	bne	a0, v0, 1f
	nop

	li	a2, V_LEVEL9  //a2 store Voltage configure value
	b	4f
	nop
1:
	li	a0, LEVEL10
	bne	a0, v0, 1f
	nop

	li	a2, V_LEVEL10  //a2 store Voltage configure value
	b	4f
	nop
1:
	li	a0, LEVEL11
	bne	a0, v0, 1f
	nop

	li	a2, V_LEVEL11  //a2 store Voltage configure value
	b	4f
	nop
1:
	li	a0, LEVEL12
	bne	a0, v0, 1f
	nop

	li	a2, V_LEVEL12  //a2 store Voltage configure value
	b	4f
	nop
1:
	li	a0, LEVEL13
	bne	a0, v0, 1f
	nop

	li	a2, V_LEVEL13  //a2 store Voltage configure value
	b	4f
	nop
1:

	li	a0, LEVEL14
	bne	a0, v0, 1f
	nop

	li	a2, V_LEVEL14  //a2 store Voltage configure value
	b	4f
	nop
1:
	li	a2, V_LEVEL15  //a2 store Voltage configure value

4:	//configure voltage
	//a2 was got value
#ifndef MPS_V
	li	a0, PV3205_ADDR0
#else
	li	a0, MPS_ADDR
#endif
	li	a1, 0x21 //VOUT_COMMAND
	bal	ls132_i2cwrite
	nop
	beqz	v0, 2f
	nop

#ifndef MPS_V
	li	a0, PV3205_ADDR1
	bal	ls132_i2cwrite
	nop
	beqz	v0, 2f
	nop
#endif

	li	v0, LS132_I2C_CR_REG
	li	v1, CR_STOP
	sb	v1, 0x0(v0)
1:
	li	v0, LS132_I2C_SR_REG
	lbu	v1, 0x0(v0)
	andi	v1, v1, SR_BUSY
	bnez	v1, 1b
	nop

	b 3f
	nop
2:
#if 0
	LS132_PUTC('V')
	LS132_PUTC('c')
	LS132_PUTC('t')
	LS132_PUTC('l')
	LS132_PUTC(' ')
	LS132_PUTC('E')
	LS132_PUTC('r')
	LS132_PUTC('r')
	LS132_PUTC('o')
	LS132_PUTC('\r')
	LS132_PUTC('\n')
#endif
	move	ra,k0
	jr	ra
	nop
3:
#if 0
	LS132_PUTC('V')
	LS132_PUTC('c')
	LS132_PUTC('t')
	LS132_PUTC('r')
	LS132_PUTC('l')
	LS132_PUTC(' ')
	LS132_PUTC('E')
	LS132_PUTC('N')
	LS132_PUTC('D')
	LS132_PUTC('\r')
	LS132_PUTC('\n')
#endif
	move	ra,k0
	jr	ra
	nop
	.set pop
END(v_ctrl)
LEAF(v_ctrl_check)
/*
 * v0 give the voltage level
 * use register:
 * a0,a1,a2,k0
 * a0: device ID
 * a1: command
 * a2: command value
 * k0: save ra
 * v0: return value
 *
 */
	.set    push
	.set    noreorder
	.set    mips32
	move	k0, ra

	li	a0, LEVEL0
	bne	a0, v0, 1f
	nop

	li	a2, CHECK_LEVEL0  //a2 store Voltage configure value
	b	4f
	nop

1:
	li	a0, LEVEL1
	bne	a0, v0, 1f
	nop

	li	a2, CHECK_LEVEL1  //a2 store Voltage configure value
	b	4f
	nop
1:
	li	a0, LEVEL2
	bne	a0, v0, 1f
	nop

	li	a2, CHECK_LEVEL2  //a2 store Voltage configure value
	b	4f
	nop
1:
	li	a0, LEVEL3
	bne	a0, v0, 1f
	nop

	li	a2, CHECK_LEVEL3  //a2 store Voltage configure value
	b	4f
	nop
1:
	li	a0, LEVEL4
	bne	a0, v0, 1f
	nop

	li	a2, CHECK_LEVEL4  //a2 store Voltage configure value
	b	4f
	nop
1:
	li	a0, LEVEL5
	bne	a0, v0, 1f
	nop

	li	a2, CHECK_LEVEL5  //a2 store Voltage configure value
	b	4f
	nop
1:
	li	a0, LEVEL6
	bne	a0, v0, 1f
	nop

	li	a2, CHECK_LEVEL6  //a2 store Voltage configure value
	b	4f
	nop
1:
	li	a0, LEVEL7
	bne	a0, v0, 1f
	nop

	li	a2, CHECK_LEVEL7  //a2 store Voltage configure value
	b	4f
	nop
1:
	li	a0, LEVEL8
	bne	a0, v0, 1f
	nop

	li	a2, CHECK_LEVEL8  //a2 store Voltage configure value
	b	4f
	nop
1:
	li	a0, LEVEL9
	bne	a0, v0, 1f
	nop

	li	a2, CHECK_LEVEL9  //a2 store Voltage configure value
	b	4f
	nop
1:
	li	a0, LEVEL10
	bne	a0, v0, 1f
	nop

	li	a2, CHECK_LEVEL10  //a2 store Voltage configure value
	b	4f
	nop
1:
	li	a0, LEVEL11
	bne	a0, v0, 1f
	nop

	li	a2, CHECK_LEVEL11  //a2 store Voltage configure value
	b	4f
	nop
1:
	li	a0, LEVEL12
	bne	a0, v0, 1f
	nop

	li	a2, CHECK_LEVEL12  //a2 store Voltage configure value
	b	4f
	nop
1:
	li	a0, LEVEL13
	bne	a0, v0, 1f
	nop

	li	a2, CHECK_LEVEL13  //a2 store Voltage configure value
	b	4f
	nop
1:

	li	a0, LEVEL14
	bne	a0, v0, 1f
	nop

	li	a2, CHECK_LEVEL14  //a2 store Voltage configure value
	b	4f
	nop
1:
	li	a2, CHECK_LEVEL15  //a2 store Voltage configure value

4:	//check voltage
	//a2 was got check value
#ifndef MPS_V
	li	a0, PV3205_ADDR1
#else
	li	a0, MPS_ADDR
#endif
	li	a1, 0x8b //READ_VOUT
	bal	ls132_i2cread
	nop
	blt	v0, a2, 4b
	nop
	move	ra, k0
	jr	ra
	nop
	.set pop
END(v_ctrl_check)
