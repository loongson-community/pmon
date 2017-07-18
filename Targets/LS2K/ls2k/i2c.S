/*************************
 * ls2k
*************************/


#define	i2c_wait_tip				\
		li	v0, LS2K_I2C0_SR_REG;	\
1:						\	
		lb	v1, 0x0(v0);		\
		andi	v1, v1, SR_TIP;		\
		bnez	v1, 1b;			\
		nop	

#define	i2c_wait_ack				\
		li	v0, LS2K_I2C0_SR_REG;	\
1:						\	
		lb	v1, 0x0(v0);		\
		and	v1, v1, SR_NOACK;	\
		bnez	v1, 1b;			\
		nop	
LEAF(i2cinit)
		//LPB clock_a,SCL clock_s,prescale = clock_a / (4 * clock_s);
		li	  v1, 0
		li	  v0, LS2K_I2C0_CTR_REG
		sb	  v1, 0x0(v0)

		li	  v1, 0x71
		li	  v0, LS2K_I2C0_PRER_LO_REG
		sb	  v1, 0x0(v0)

		li	  v1, 0x2
		li	  v0, LS2K_I2C0_PRER_HI_REG
		sb	  v1, 0x0(v0)

		li	  v1, 0x80
		li	  v0, LS2K_I2C0_CTR_REG
		sb	  v1, 0x0(v0)

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
	li	v0, LS2K_I2C0_TXR_REG	
	sb	v1, 0x0(v0)		

	/* send start frame */
	li	v1, CR_START | CR_WRITE	
	li	v0, LS2K_I2C0_CR_REG		
	sb	v1, 0x0(v0)		

	/* waite send finished */
//	i2c_wait_tip			
	li	v0, LS2K_I2C0_SR_REG	
1:						
	lb	v1, 0x0(v0)		
	andi	v1, v1, SR_TIP		
	bnez	v1, 1b			
	nop	
	
	/* load data to be send */
	move	v1, a1			
	li	v0, LS2K_I2C0_TXR_REG	
	sb	v1, 0x0(v0)		

	/* send data frame */
	li	v1, CR_WRITE		
	li	v0, LS2K_I2C0_CR_REG		
	sb	v1, 0x0(v0)		

	/* waite send finished */
//	i2c_wait_tip			
	li	v0, LS2K_I2C0_SR_REG	
1:						
	lb	v1, 0x0(v0)		
	andi	v1, v1, SR_TIP		
	bnez	v1, 1b			
	nop	

/* i2c_read_b */			       
	/* load device address */
	ori	v1, a0, 0x1
	li	v0, LS2K_I2C0_TXR_REG	
	sb	v1, 0x0(v0)		
	
	/* send start frame */
	li	v1, CR_START | CR_WRITE	
	li	v0, LS2K_I2C0_CR_REG		
	sb	v1, 0x0(v0)		

	/* waite send finished */
//	i2c_wait_tip			
	li	v0, LS2K_I2C0_SR_REG	
1:						
	lb	v1, 0x0(v0)		
	andi	v1, v1, SR_TIP		
	bnez	v1, 1b			
	nop	
	
	/* receive data to fifo */
	li	v1, CR_READ | CR_ACK	
	li	v0, LS2K_I2C0_CR_REG		
	sb	v1, 0x0(v0)		

//	i2c_wait_tip			
	li	v0, LS2K_I2C0_SR_REG	
1:						
	lb	v1, 0x0(v0)		
	andi	v1, v1, SR_TIP		
	bnez	v1, 1b			
	nop	

	/* read data from fifo */
	li	v0, LS2K_I2C0_RXR_REG	
	lb	a1, 0x0(v0)

/* i2c_stop */				
	/* free i2c bus */
	li	v0, LS2K_I2C0_CR_REG		
	li	v1, CR_STOP		
	sb	v1, 0x0(v0)		
1:					
	li	v0, LS2K_I2C0_SR_REG		
	lb	v1, 0x0(v0)		
	andi	v1, v1, SR_BUSY		
	bnez	v1, 1b			
	nop				
	
	move	v0, a1

	jr	ra
	nop
END(i2cread)
