/*************************
 * ls7a
*************************/

#ifdef  MULTI_I2C_BUS
#define ADD_NODE_ID_a2 or v0, a2;
#else
#define ADD_NODE_ID_a2 ;
#endif

#if 0 //if use the ls7a i2c0
#define MISC_BASE				0x90000e0010080000
#define LS7A_I2C0_REG_BASE			(MISC_BASE + 0x10000)
#define LS_I2C_REG_BASE				LS7A_I2C0_REG_BASE
#endif

#define LS3A4000_I2C1_REG_BASE			0x900000001fe00130
#define LS3A4000_I2C0_REG_BASE			0x900000001fe00120

#if defined(MULTI_CHIP) && !defined(CHIP_4)
#define LS_I2C_REG_BASE				LS3A4000_I2C0_REG_BASE
#else
#define LS_I2C_REG_BASE				LS3A4000_I2C0_REG_BASE //new board
//#define LS_I2C_REG_BASE			LS3A4000_I2C1_REG_BASE
#endif

#define PRER_LO_REG				(0x0)
#define PRER_HI_REG				(0x1)
#define CTR_REG					(0x2)
#define TXR_REG					(0x3)
#define RXR_REG					(0x3)
#define CR_REG					(0x4)
#define SR_REG					(0x4)

#define CR_START				0x80
#define CR_STOP					0x40
#define CR_READ					0x20
#define CR_WRITE				0x10
#define CR_ACK					0x8
#define CR_IACK					0x1

#define SR_NOACK				0x80
#define SR_BUSY					0x40
#define SR_AL					0x20
#define SR_TIP					0x2
#define SR_IF					0x1

#define SWP0 0x62
#define SWP1 0x68
#define SWP2 0x6a
#define SWP3 0x60
#define CWP  0x66
#define RPS0 0x63
#define RPS1 0x69
#define RPS2 0x6b
#define RPS3 0x61
#define SPA0 0x6c
#define SPA1 0x6e
#define RPA  0x6d

LEAF(i2cinit)
	//LPB clock_a,SCL clock_s,prescale = clock_a / (4 * clock_s);
	dli	v0, LS_I2C_REG_BASE
	ADD_NODE_ID_a2

	lb	v1, CTR_REG(v0)
	and	v1, ~(1 << 7)
	sb	v1, CTR_REG(v0)

//	li	v1, 0x71 //33M
	li	v1, 0x53 //100M
	sb	v1, PRER_LO_REG(v0)

//	li	v1, 0x2 //33M
	li	v1, 0x7 //100M
	sb	v1, PRER_HI_REG(v0)

	lb	v1, CTR_REG(v0)
	or	v1, (1 << 7)
	sb	v1, CTR_REG(v0)

	jr	ra
	nop
END(i2cinit)

LEAF(i2c_spd_ee_ops)
/*DDR4 eeprom device select code*/
/*
 * use register:
 *	v0, v1
 *	a0, a1
 *	input: a0
 *	   a0: device select code
 *	   v0: return value
 * ADD_NODE_ID_a2 use a2 for store node_id addr
 */
	move	v1, a0
	/*use v0 store  i2c register base address*/
	dli	v0, LS_I2C_REG_BASE
	ADD_NODE_ID_a2

	sb	v1, TXR_REG(v0)

	/*send device select code*/
	li	v1, CR_START | CR_WRITE
	sb	v1, CR_REG(v0)

	/* wait send finished */
1:
	lb	v1, SR_REG(v0)
	andi	v1, v1, SR_TIP
	bnez	v1, 1b
	nop

	/* if no ack return 1, else return 0 */
	lb	v1, SR_REG(v0)
	and	v1, v1, SR_NOACK
	srl	a1, v1, 7

	/* i2c_stop */
	li	v1, CR_STOP
	sb	v1, CR_REG(v0)
1:
	lb	v1, SR_REG(v0)
	andi	v1, v1, SR_BUSY
	bnez	v1, 1b
	nop

	move	v0, a1

	jr	ra
	nop
END(i2c_spd_ee_ops)

LEAF(i2cread)
/*
 * use register:
 *	v0, v1
 *	a0, a1
 *	input:	a0,a1
 *	a0: device ID
 *	a1: register offset
 *	v0: return value
 *
 * ADD_NODE_ID_a2 use a2 for store node_id addr
 */

	/*use v0 store  i2c register base address*/
	dli	v0, LS_I2C_REG_BASE
	ADD_NODE_ID_a2

	/* if addr large than 0xff set to page1 */
	andi	v1, a1, 0xff00
	beqz	v1, 2f
	nop
/*set page to 1*/
	li	v1, SPA1
	sb	v1, TXR_REG(v0)

	/*send device select code*/
	li	v1, CR_START | CR_WRITE
	sb	v1, CR_REG(v0)

	/* wait send finished */
1:
	lb	v1, SR_REG(v0)
	andi	v1, v1, SR_TIP
	bnez	v1, 1b
	nop

	/* i2c_stop */
	li	v1, CR_STOP
	sb	v1, CR_REG(v0)
1:
	lb	v1, SR_REG(v0)
	andi	v1, v1, SR_BUSY
	bnez	v1, 1b
	nop
/*set page to 1 end*/

2:
	/* load device address */
	andi	v1, a0, 0xfe
	sb	v1, TXR_REG(v0)

	/* send start frame */
	li	v1, CR_START | CR_WRITE
	sb	v1, CR_REG(v0)

	/* wait send finished */
//	i2c_wait_tip
1:
	lb	v1, SR_REG(v0)
	andi	v1, v1, SR_TIP
	bnez	v1, 1b
	nop

	/* load data to be send */
	move	v1, a1
	sb	v1, TXR_REG(v0)

	/* send data frame */
	li	v1, CR_WRITE
	sb	v1, CR_REG(v0)

	/* wait send finished */
//	i2c_wait_tip
1:
	lb	v1, SR_REG(v0)
	andi	v1, v1, SR_TIP
	bnez	v1, 1b
	nop

/* i2c_read_b */
	/* load device address */
	ori	v1, a0, 0x1
	sb	v1, TXR_REG(v0)
	
	/* send start frame */
	li	v1, CR_START | CR_WRITE
	sb	v1, CR_REG(v0)

	/* wait send finished */
//	i2c_wait_tip
1:
	lb	v1, SR_REG(v0)
	andi	v1, v1, SR_TIP
	bnez	v1, 1b
	nop

	/* receive data to fifo */
	li	v1, CR_READ | CR_ACK
	sb	v1, CR_REG(v0)

//	i2c_wait_tip
1:
	lb	v1, SR_REG(v0)
	andi	v1, v1, SR_TIP
	bnez	v1, 1b
	nop

	/* read data from fifo */
	lb	v1, RXR_REG(v0)
	/*store a1 high bit for judge whether need config to page 0*/
	and	a1, ~0xff
	or	a1, v1

/* i2c_stop */
	/* free i2c bus */
	li	v1, CR_STOP
	sb	v1, CR_REG(v0)
1:
	lb	v1, SR_REG(v0)
	andi	v1, v1, SR_BUSY
	bnez	v1, 1b
	nop

	/* if addr large than 0xff set to page0 as default status */
	andi	v1, a1, 0xff00
	beqz	v1, 2f
	nop
/*set page to 0*/
	li	v1, SPA0
	sb	v1, TXR_REG(v0)

	li	v1, CR_START | CR_WRITE
	sb	v1, CR_REG(v0)

	/* wait send finished */
1:
	lb	v1, SR_REG(v0)
	andi	v1, v1, SR_TIP
	bnez	v1, 1b
	nop

	/* i2c_stop */
	li	v1, CR_STOP
	sb	v1, CR_REG(v0)
1:
	lb	v1, SR_REG(v0)
	andi	v1, v1, SR_BUSY
	bnez	v1, 1b
	nop
/*set page to 0 end*/
2:
	and	v0, a1, 0xff

	jr	ra
	nop
END(i2cread)
