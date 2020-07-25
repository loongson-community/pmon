/*************************
 * this file i2c address use 64 bit access
 * this code used for control voltage
*************************/

#define MISC_BASE			0x90000e0010080000
#define LS7A_I2C0_REG_BASE		(MISC_BASE + 0x10000)
#define LS7A_I2C1_REG_BASE		(MISC_BASE + 0x10100)

//#define LS3A4000_I2C1_REG_BASE	0x900000001fe00130
#define LS3A4000_I2C0_REG_BASE		0x900000001fe00120

#define PRER_LO_REG			(0x0)
#define PRER_HI_REG			(0x1)
#define CTR_REG				(0x2)
#define TXR_REG				(0x3)
#define RXR_REG				(0x3)
#define CR_REG				(0x4)
#define SR_REG				(0x4)

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

#define PV3205_ADDR0			(16 << 1) //3A I2C0
#define PV3205_ADDR1			(17 << 1) //3A I2C0

#ifdef LOONGSON_BOARD
#define MPS_ADDR			(0x3b << 1)
#else //EASCS board
#define MPS_ADDR			(0x6b << 1)
#endif

#ifndef MULTI_CHIP
#define PV3205_VDDP  (33 << 1)
#endif

#define	UPI9212S_ADDR0	        0x4a    //3A I2C0

/*
 * use register:
 *	v0, v1, s1
 *	input: a3
 *	a3: i2c register base address
 *	s1: (node_id << 44) do not change it
 */
LEAF(ls_v_i2cinit)
	.set    push
	.set    noreorder
	.set    mips3
	//LPB clock_a,SCL clock_s,prescale = clock_a / (4 * clock_s);
	or	a3, s1
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
END(ls_v_i2cinit)

LEAF(ls_v_i2cwrite)
/*
 * use register:
 *	v0, v1, s1
 *	a0, a1, a3
 *	input: a0,a1,a2
 *	a0: device ID
 *	a1: register offset/command, [16-31] store write count now 1 mean 1 byte default 0/2  byte 2
 *	a2: configure value
 *	a3: i2c register base address
 *	v0: return value
 *	s1: (node_id << 44) do not change it
 */
	.set    push
	.set    noreorder
	.set    mips3
/*i2c_send_addr*/
	/* load device address */
	move	v1, a0
	or	a3, s1
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
END(ls_v_i2cwrite)

LEAF(ls_v_i2cread)
/*
 * use register:
 *	v0, v1
 *	a0, a1
 *	input: a0,a1
 *	a0: device ID
 *	a1: register offset
 *	v0: return value
 *
 */
    .set    push
    .set    noreorder
    .set    mips3
/*i2c_send_addr*/
/*send device addr*/
	/* load device address */
	move	v1, a0
	or	a3, s1
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
/*send device addr*/
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

/* i2c_read_word*/
/*send device addr*/
	/* load device address (dev_addr | 0x1/READ) */
	ori	v1, a0, 0x1
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
/*send device addr*/

	/* receive data from fifo */
	li	v1, CR_READ
	sb	v1, CR_REG(a3)

//	i2c_wait_tip
1:
	lbu	v1, SR_REG(a3)
	andi	v1, v1, SR_TIP
	bnez	v1, 1b
	nop

	/* read data from fifo */
	lbu	a1, RXR_REG(a3)

	/* receive data from fifo */
	li	v1, CR_READ | CR_ACK
	sb	v1, CR_REG(a3)

//	i2c_wait_tip
1:
	lbu	v1, SR_REG(a3)
	andi	v1, v1, SR_TIP
	bnez	v1, 1b
	nop

	/* read data from fifo */
	lbu	v1, RXR_REG(a3)
	sll	v1, v1,0x8
	addu a1, a1, v1
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

	move	v0, a1

	jr	ra
	nop
    .set pop
END(ls_v_i2cread)

LEAF(v_n_ctrl)
/*
 * v0 give the voltage level
 * use register:
 * a0,a1,a2,k0
 * a0: device ID
 * a1: command
 * a2: command value
 * k0: save ra
 * v0: return value
 */
	.set    push
	.set    noreorder
	.set    mips3
	move	k0, ra

	dli	a3, LS3A4000_I2C0_REG_BASE
	bal	ls_v_i2cinit
	nop
#if defined(LOONGSON_BOARD) && defined(MPS_V)
	li	a0, MPS_ADDR
	li	a2, 0x210
	li	a1, 0x5e
	dli	a3, LS3A4000_I2C0_REG_BASE
	bal	ls_v_i2cwrite
	nop
	li	v1, CR_STOP
	sb	v1, CR_REG(a3)
1:
	lbu	v1, SR_REG(a3)
	andi	v1, v1, SR_BUSY
	bnez	v1, 1b
	nop
#endif
#ifdef UPI9212S_V
	li  a0, UPI9212S_ADDR0
	li  a1, 0x39 | (1 << 16)   // unlock smbus, byte write
	li  a2, 0x94
	dli a3, LS3A4000_I2C0_REG_BASE
	bal ls_v_i2cwrite
	nop
	/* Wait until bus is not busy */
	li	v1, CR_STOP
	sb	v1, CR_REG(a3)
1:
	lbu	v1, SR_REG(a3)
	andi	v1, v1, SR_BUSY
	bnez	v1, 1b
	nop

	li  a0, UPI9212S_ADDR0
	li  a1, 0x3c | (1 << 16)  // enable latch VID, byte write
	li  a2, 0x1f
	dli a3, LS3A4000_I2C0_REG_BASE
	bal ls_v_i2cwrite
	nop
	/* Wait until bus is not busy */
	li	v1, CR_STOP
	sb	v1, CR_REG(a3)
1:
	lbu	v1, SR_REG(a3)
	andi	v1, v1, SR_BUSY
	bnez	v1, 1b
	nop
#endif
#ifdef INPUT_PARAM  //this code used for debug
	WatchDog_Close
#ifdef PV3205_V
	PRINTSTR("\r\nInput parameter 0.9v:0x1200, 0.95v:0x1300, 1.0v:0x1400, 1.05v:0x1500, 1.1v:0x1600, 1.15v:0x1700, 1.20v:0x1800")
	PRINTSTR("\r\nInput parameter 1.25v:0x1900, 1.3v:0x1a00")
#elif defined(MPS_V)
	PRINTSTR("\r\nInput parameter 0.9v:0x29, 0.95v:0x2e, 1.0v:0x33, 1.05v:0x38, 1.1v:0x3d, 1.15v:0x42, 1.20v:0x47")
	PRINTSTR("\r\nInput parameter 1.25v:0x4c, 1.3v:0x51")
#elif defined(UPI9212S_V)
	PRINTSTR("\r\nInput parameter 0.9v:0x69, 0.95v:0x70, 1.0v:0x78, 1.05v:0x7f, 1.1v:0x87, 1.15v:0x8f, 1.20v:0x96")
	PRINTSTR("\r\nInput parameter 1.25v:0x9e, 1.3v:0xa5")
#else
#error "PMIC not supported"
#endif
	bal	inputaddress
	nop
	move	a2, v0
#else
#ifdef PV3205_V
	//li	a2, 0x1200 //0.9v
	//li	a2, 0x1300 //0.95v
	//li	a2, 0x1400 //1.0v
	//li	a2, 0x1500 //1.05v
	//li	a2, 0x1600 //1.1v
	//li	a2, 0x1700 //1.15v
	//li	a2, 0x1800 //1.2v
	li	a2, 0x1900 //1.25v
	//li	a2, 0x1a00 //1.3v
#elif defined(MPS_V)
	//li	a2, 41 //0.9v
	//li	a2, 46 //0.95v
	//li	a2, 51 //1.0v
	//li	a2, 56 //1.05v
	//li	a2, 61 //1.1v
	//li	a2, 66 //1.15v
//	li	a2, 71 //1.2v
	li	a2, 76 //1.25v
	//li	a2, 81 //1.3v
#elif defined(UPI9212S_V)
	//li  a2, 0x69 //0.902v
	//li  a2, 0x70 //0.948v
	//li  a2, 0x78 //1.001v
	//li  a2, 0x7f //1.047v
	//li  a2, 0x87 //1.100v
	//li  a2, 0x8D //1.139v
	//li  a2, 0x8f //1.152v
	//li  a2, 0x96 //1.199v
	li  a2, 0x9e //1.251v
	//li  a2, 0xa5 //1.297v
#else
#error "PMIC not supported"
#endif
#endif
#ifdef PV3205_V
	li	a0, PV3205_ADDR0
	li	a1, 0x21 //VOUT_COMMAND
#elif defined(MPS_V)
	li	a0, MPS_ADDR
	li	a1, 0x21 //VOUT_COMMAND
#elif defined(UPI9212S_V)
	li	a0, UPI9212S_ADDR0
	li	a1, 0x30 | (1 << 16) //VOUT_COMMAND, byte write
#else
#error "PMIC not supported"
#endif
	dli	a3, LS3A4000_I2C0_REG_BASE

	bal	ls_v_i2cwrite
	nop
	beqz	v0, 2f
	nop

#ifdef PV3205_V
	li	a0, PV3205_ADDR1
	bal	ls_v_i2cwrite
	nop
	beqz	v0, 2f
	nop
#endif

	li	v1, CR_STOP
	sb	v1, CR_REG(a3) //if not change a3 == a3 or s1
1:
	lbu	v1, SR_REG(a3)
	andi	v1, v1, SR_BUSY
	bnez	v1, 1b
	nop

	b 3f
	nop
2:
	TTYDBG("v ctrl err\r\n")
	move	ra,k0
	jr	ra
	nop
3:
	TTYDBG("v ctrl end\r\n")
	move	ra,k0
	jr	ra
	nop
	.set pop
END(v_n_ctrl)

LEAF(v_n_ctrl_read)
/*
 * use register:
 * a0,a1,k0
 * a0: device ID
 * a1: command
 * k0: save ra
 * v0: return value
 */
	.set    push
	.set    noreorder
	.set    mips3

	move	k0, ra

	dli	a3, LS3A4000_I2C0_REG_BASE
	bal	ls_v_i2cinit
	nop
#ifdef PV3205_V
	li	a0, PV3205_ADDR0
#elif defined(MPS_V)
	li	a0, MPS_ADDR
#elif defined(UPI9212S_V)
	li	a0, UPI9212S_ADDR0
#else
#error "PMIC not supported"
#endif
#if defined(PV3205_V) || defined(MPS_V)
	li	a1, 0x8b //READ_VOUT
	bal	ls_v_i2cread
	nop
	move	a0, v0
	bal	hexserial
	nop
	TTYDBG("\r\n")
#elif defined(UPI9212S_V)
	li  a1, 0x30 //READ_VOUT
	bal	ls_v_i2cread
	nop
	mulou v0, 6587   // uv
	add   v0, 210000 // Vout = 0.006587 * (x) + 0.21
	move	a0, v0
	bal	hexserial
	nop
	TTYDBG("uV \r\n")
#else
#error "PMIC not supported"
#endif

	move	ra, k0
	jr	ra
	nop
	.set pop
END(v_n_ctrl_read)

#ifdef PV3205_V
LEAF(v_vo_ctrl)
/*
 * v0 give the voltage level
 * use register:
 * a0,a1,a2,k0
 * a0: device ID
 * a1: command
 * a2: command value
 * k0: save ra
 * v0: return value
 */
	.set    push
	.set    noreorder
	.set    mips3
	move	k0, ra

	dli	a3, LS3A4000_I2C0_REG_BASE
	bal	ls_v_i2cinit
	nop

	li	a0, PV3205_ADDR0
	li	a1, 0x40 //VOUT_OV_FAULT_LIMIT
	//li  a2, 0x1a00 //1.3v
	li	a2, 0x1c00 //1.4v overvoltage value
	bal	ls_v_i2cwrite
	nop
	beqz	v0, 2f
	nop

	li	a0, PV3205_ADDR1
	bal	ls_v_i2cwrite
	nop
	beqz	v0, 2f
	nop

	li	v1, CR_STOP
	sb	v1, CR_REG(a3)
1:
	lbu	v1, SR_REG(a3)
	andi	v1, v1, SR_BUSY
	bnez	v1, 1b
	nop

	b	3f
	nop
2:
	TTYDBG("ov ctrl err\r\n")
	move	ra,k0
	jr	ra
	nop
3:
	TTYDBG("ov ctrl end\r\n")
	move	ra,k0
	jr	ra
	nop
	.set pop
END(v_vo_ctrl)

LEAF(v_io_ctrl)
/*
 * v0 give the voltage level
 * use register:
 * a0,a1,a2,k0
 * a0: device ID
 * a1: command
 * a2: command value
 * k0: save ra
 * v0: return value
 */
	.set    push
	.set    noreorder
	.set    mips3
	move	k0, ra

	dli	a3, LS3A4000_I2C0_REG_BASE
	bal	ls_v_i2cinit
	nop

	li	a0, PV3205_ADDR0
	li	a1, 0x46 //IOUT_OC_FAULT_LIMIT
	li	a2, 0x400 //1.4v overvoltage value
	bal	ls_v_i2cwrite
	nop
	beqz	v0, 2f
	nop

	li	a0, PV3205_ADDR1
	bal	ls_v_i2cwrite
	nop
	beqz	v0, 2f
	nop

	li	v1, CR_STOP
	sb	v1, CR_REG(a3)
1:
	lbu	v1, SR_REG(a3)
	andi	v1, v1, SR_BUSY
	bnez	v1, 1b
	nop

	b	3f
	nop
2:
	TTYDBG("io ctrl err\r\n")
	move	ra,k0
	jr	ra
	nop
3:
	TTYDBG("io ctrl end\r\n")
	move	ra,k0
	jr	ra
	nop
	.set pop
END(v_io_ctrl)
#endif

LEAF(v_p_ctrl)
/*
 * v0 give the voltage level
 * use register:
 * a0,a1,a2,k0
 * a0: device ID
 * a1: command
 * a2: command value
 * k0: save ra
 * v0: return value
 */
	.set    push
	.set    noreorder
	.set    mips3
	move	k0, ra

#if defined(PV3205_V) || defined(MPS_V) // Other PMIC have only one VOUT ctrl
#ifdef PV3205_V
	dli	a3, LS7A_I2C1_REG_BASE
	bal	ls_v_i2cinit
	nop

	li	a0, PV3205_VDDP
	li	a1, 0x21 //VOUT_COMMAND
	//li	a2, 0x1b00 //1.35v
	li	a2, 0x1a00 //1.3v
	//li	a2, 0x1900 //1.25v
	//li	a2, 0x1800 //1.2v
	//li	a2, 0x1600 //1.1v

	bal	ls_v_i2cwrite
	nop
	beqz	v0, 2f
	nop

	li	v1, CR_STOP
	sb	v1, CR_REG(a3)
1:
	lbu	v1, SR_REG(a3)
	andi	v1, v1, SR_BUSY
	bnez	v1, 1b
	nop
#elif defined(MPS_V)
	dli	a3, LS3A4000_I2C0_REG_BASE
	bal	ls_v_i2cinit
	nop

	/*write one byte do not need completed i2c stop code*/
	li	a0, MPS_ADDR
	li	a1, 0 //page select command
	or	a1, (0x1 << 16) //write 1 byte
	li	a2, 1
	bal	ls_v_i2cwrite
	nop

	li	a0, MPS_ADDR
	li	a1, 0x21 //VOUT_COMMAND
	//li	a2, 61 //1.1v
	//li	a2, 71 //1.2v
	//li	a2, 76 //1.25v
	li	a2, 81 //1.3v
	bal	ls_v_i2cwrite
	nop

	li	v1, CR_STOP
	sb	v1, CR_REG(a3)
1:
	lbu	v1, SR_REG(a3)
	andi	v1, v1, SR_BUSY
	bnez	v1, 1b
	nop

	li	a0, MPS_ADDR
	li	a1, 0 //page select command
	or	a1, (0x1 << 16) //write 1 byte
	li	a2, 0
	bal	ls_v_i2cwrite
	nop
	/*mps chip control one byte do not check return value*/
#else
#error "PMIC not supported"
#endif

	TTYDBG("p ctrl end\r\n")
	move	ra,k0
	jr	ra
	nop
2:
	TTYDBG("p ctrl err\r\n")
#endif
	move	ra,k0
	jr	ra
	nop
	.set pop
END(v_p_ctrl)
