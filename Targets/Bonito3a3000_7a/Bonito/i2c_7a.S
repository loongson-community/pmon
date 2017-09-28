/*************************
 * ls7a
*************************/

#define MISC_BASE	0x90000e0010080000
#define LS7A_I2C0_REG_BASE				    (MISC_BASE + 0x10000)
#define LS7A_I2C0_PRER_LO_REG				(LS7A_I2C0_REG_BASE + 0x0)
#define LS7A_I2C0_PRER_HI_REG				(LS7A_I2C0_REG_BASE + 0x1)
#define LS7A_I2C0_CTR_REG   				(LS7A_I2C0_REG_BASE + 0x2)
#define LS7A_I2C0_TXR_REG   				(LS7A_I2C0_REG_BASE + 0x3)
#define LS7A_I2C0_RXR_REG    				(LS7A_I2C0_REG_BASE + 0x3)
#define LS7A_I2C0_CR_REG     				(LS7A_I2C0_REG_BASE + 0x4)
#define LS7A_I2C0_SR_REG     				(LS7A_I2C0_REG_BASE + 0x4)

#define CR_START					0x80
#define CR_STOP						0x40
#define CR_READ						0x20
#define CR_WRITE					0x10
#define CR_ACK						0x8
#define CR_IACK						0x1

#define SR_NOACK					0x80
#define SR_BUSY						0x40
#define SR_AL						0x20
#define SR_TIP						0x2
#define	SR_IF						0x1

#define	i2c_wait_tip				\
		dli	v0, LS7A_I2C0_SR_REG;	\
1:						\	
		lb	v1, 0x0(v0);		\
		andi	v1, v1, SR_TIP;		\
		bnez	v1, 1b;			\
		nop	

#define	i2c_wait_ack				\
		dli	v0, LS7A_I2C0_SR_REG;	\
1:						\	
		lb	v1, 0x0(v0);		\
		and	v1, v1, SR_NOACK;	\
		bnez	v1, 1b;			\
		nop	
LEAF(i2cinit)
#if 0
		//i2c0 sel
		dli     v1, 0x90000e005ff00440
		lb	v0, 0x0(v1)
		or	v0, (1 << 4)
		sb	v0, 0x0(v1)
#endif
		//LPB clock_a,SCL clock_s,prescale = clock_a / (4 * clock_s);
		li	v1, 0
		dli	v0, LS7A_I2C0_CTR_REG
		sb	v1, 0x0(v0)

		li	v1, 0x71
		dli	v0, LS7A_I2C0_PRER_LO_REG
		sb	v1, 0x0(v0)

		li	v1, 0x2
		dli	v0, LS7A_I2C0_PRER_HI_REG
		sb	v1, 0x0(v0)

		li	v1, 0x80
		dli	v0, LS7A_I2C0_CTR_REG
		sb	v1, 0x0(v0)

		jr      ra
		nop
END(i2cinit)
	  
LEAF(i2cread)
/*
 * use register:
 *	v0, v1
 *	a0, a1
 *	input: a0,a1
 *	   a0: device ID
 *	   a1: register offset
 *	   v0: return value
 *
 */

/*i2c_send_b*/				
	/* load device address */
	andi	v1, a0, 0xfe		
	dli	v0, LS7A_I2C0_TXR_REG	
	sb	v1, 0x0(v0)		

	/* send start frame */
	li	v1, CR_START | CR_WRITE	
	dli	v0, LS7A_I2C0_CR_REG		
	sb	v1, 0x0(v0)		

	/* waite send finished */
//	i2c_wait_tip			
	dli	v0, LS7A_I2C0_SR_REG	
1:						
	lb	v1, 0x0(v0)		
	andi	v1, v1, SR_TIP		
	bnez	v1, 1b			
	nop	
	
	/* load data to be send */
	move	v1, a1			
	dli	v0, LS7A_I2C0_TXR_REG	
	sb	v1, 0x0(v0)		

	/* send data frame */
	li	v1, CR_WRITE		
	dli	v0, LS7A_I2C0_CR_REG		
	sb	v1, 0x0(v0)		

	/* waite send finished */
//	i2c_wait_tip			
	dli	v0, LS7A_I2C0_SR_REG	
1:						
	lb	v1, 0x0(v0)		
	andi	v1, v1, SR_TIP		
	bnez	v1, 1b			
	nop	

/* i2c_read_b */			       
	/* load device address */
	ori	v1, a0, 0x1
	dli	v0, LS7A_I2C0_TXR_REG	
	sb	v1, 0x0(v0)		
	
	/* send start frame */
	li	v1, CR_START | CR_WRITE	
	dli	v0, LS7A_I2C0_CR_REG		
	sb	v1, 0x0(v0)		

	/* waite send finished */
//	i2c_wait_tip			
	dli	v0, LS7A_I2C0_SR_REG	
1:						
	lb	v1, 0x0(v0)		
	andi	v1, v1, SR_TIP		
	bnez	v1, 1b			
	nop	
	
	/* receive data to fifo */
	li	v1, CR_READ | CR_ACK	
	dli	v0, LS7A_I2C0_CR_REG		
	sb	v1, 0x0(v0)		

//	i2c_wait_tip			
	dli	v0, LS7A_I2C0_SR_REG	
1:						
	lb	v1, 0x0(v0)		
	andi	v1, v1, SR_TIP		
	bnez	v1, 1b			
	nop	

	/* read data from fifo */
	dli	v0, LS7A_I2C0_RXR_REG	
	lb	a1, 0x0(v0)

/* i2c_stop */				
	/* free i2c bus */
	dli	v0, LS7A_I2C0_CR_REG		
	li	v1, CR_STOP		
	sb	v1, 0x0(v0)		
1:					
	dli	v0, LS7A_I2C0_SR_REG		
	lb	v1, 0x0(v0)		
	andi	v1, v1, SR_BUSY		
	bnez	v1, 1b			
	nop				
	
	move	v0, a1

	jr	ra
	nop
END(i2cread)
