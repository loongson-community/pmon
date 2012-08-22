/*************************
 * just suport ls3a2h
*************************/
// below added for ls3a+2h i2c

#define CR_START	0x80
#define CR_STOP		0x40
#define CR_RD		0x20
#define CR_WR		0x10
#define CR_ACK		0x8
#define CR_IACK		0x1
#define SR_TIP		0x2
#define SR_BUSY		0x40
#define SR_RXACK	0x80
#

#define I2C_START 0x80
#define I2C_STOP 0x40
#define I2C_READ 0x20
#define I2C_WRITE 0x10
#define I2C_WACK 0x8
#define I2C_IACK 0x1
#define I2C_RUN 0x2
#define I2C_BUSY 0x40
#define I2C_RACK 0x80

#define LS2H_I2C_BASE    0xbbe90000
#define LS2H_I2C_PRER_LO (LS2H_I2C_BASE + 0x0)
#define LS2H_I2C_PRER_HI (LS2H_I2C_BASE + 0x1)
#define LS2H_I2C_CTR     (LS2H_I2C_BASE + 0x2)
#define LS2H_I2C_TXR     (LS2H_I2C_BASE + 0x3)
#define LS2H_I2C_RXR     (LS2H_I2C_BASE + 0x3)
#define LS2H_I2C_CR      (LS2H_I2C_BASE + 0x4)
#define LS2H_I2C_SR      (LS2H_I2C_BASE + 0x4)


#define i2c_wait  \
		li	  v0, LS2H_I2C_CR	\
11:	\
		lb	  v1, 0x0(v0);	\
		li	  v1, CR_TIP  \
		bne	  v1, 11b;	\
		nop

LEAF(i2cinit)
		li	  v1, 0x64
		li	  v0, LS2H_I2C_PRER_LO
		sb	  v1, 0x0(v0);

		li	  v1, 0x00
		li	  v0, LS2H_I2C_PRER_HI
		sb	  v1, 0x0(v0);

		li	  v1, 0x80
		li	  v0, LS2H_I2C_CTR
		sb	  v1, 0x0(v0);

        jr      ra
        nop
END(i2cinit)
	  
#if 0
LEAF(i2cread)
/***************
use register:
v0, v1
a0, a1
input: a0,a1
   a0: device ID
   a1: register offset
***************/

		/* store device base address */
		andi  v1, a0, 0xfe
		li	  v0, LS2H_I2C_TXR 
		sb	  v1, 0x0(v0);

		/* start i2c */
		li	  v1, CR_START|CR_WRITE;
		li	  v0, LS2H_I2C_CR
		sb	  v1, 0x0(v0);

		i2c_wait;
		
		/* store reg offset */
		move  v1, a1
		li	  v0, LS2H_I2C_TXR 
		sb	  v1, 0x0(v0);

  		/* stop i2c */
		li	  v1, CR_STOP|CR_WRITE;
		li	  v0, LS2H_I2C_CR
		sb	  v1, 0x0(v0);

		i2c_wait;

		/* now choose register offset to read out */
		ori   v1, a0, 0x1
		li	  v0, LS2H_I2C_TXR 
		sb	  v1, 0x0(v0);

		/* start i2c */
		li	  v1, CR_START|CR_WRITE; /* start on first addr */
		li	  v0, LS2H_I2C_CR
		sb	  v1, 0x0(v0);

		i2c_wait;
		
		li	  v1, CR_READ|I2C_WACK;  /* last read not send ack */
		li	  v0, LS2H_I2C_CR
		sb	  v1, 0x0(v0);

		i2c_wait;

		li	  v0, LS2H_I2C_TXR
		lb	  a0, 0x0(v0);   /* get data */

		li	  a1, CR_STOP;   /* stop */
		li	  v0, LS2H_I2C_CR
		sb	  a1, 0x0(v0);
		
		/* wait i2c free */
		li	  v0, LS2H_I2C_SR
		lb	  a1, 0x0(v0);


		li	  v0, LS2H_I2C_SR
112:
		lb	  a1, 0x0(v0);
		andi  a1, a1, SR_BUSY
		bne	  a1, 112b;
		nop

		move  v0, a0;
        jr      ra
        nop
END(i2cread)
#else
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
	li	v0, LS2H_I2C_TXR	
	sb	v1, 0x0(v0)		

	/* send start frame */
	li	v1, CR_START | CR_WR	
	li	v0, LS2H_I2C_CR		
	sb	v1, 0x0(v0)		

	/* waite send finished */
//	i2c_wait_tip			
	li	v0, LS2H_I2C_SR;	
1:						
	lb	v1, 0x0(v0);		
	andi	v1, v1, SR_TIP;		
	bnez	v1, 1b;			
	nop	
	
	/* load data to be send */
	move	v1, a1			
	li	v0, LS2H_I2C_TXR	
	sb	v1, 0x0(v0)		

	/* send data frame */
	li	v1, CR_WR		
	li	v0, LS2H_I2C_CR		
	sb	v1, 0x0(v0)		

	/* waite send finished */
//	i2c_wait_tip			
	li	v0, LS2H_I2C_SR;	
1:						
	lb	v1, 0x0(v0);		
	andi	v1, v1, SR_TIP;		
	bnez	v1, 1b;			
	nop	

/* i2c_read_b */			       
	/* load device address */
	ori	v1, a0, 0x1
	li	v0, LS2H_I2C_TXR	
	sb	v1, 0x0(v0)		
	
	/* send start frame */
	li	v1, CR_START | CR_WR	
	li	v0, LS2H_I2C_CR		
	sb	v1, 0x0(v0)		

	/* waite send finished */
//	i2c_wait_tip			
	li	v0, LS2H_I2C_SR;	
1:						
	lb	v1, 0x0(v0);		
	andi	v1, v1, SR_TIP;		
	bnez	v1, 1b;			
	nop	
	
	/* receive data to fifo */
	li	v1, CR_RD | CR_ACK	
	li	v0, LS2H_I2C_CR		
	sb	v1, 0x0(v0)		

//	i2c_wait_tip			
	li	v0, LS2H_I2C_SR;	
1:						
	lb	v1, 0x0(v0);		
	andi	v1, v1, SR_TIP;		
	bnez	v1, 1b;			
	nop	

	/* read data from fifo */
	li	v0, LS2H_I2C_RXR	
	lb	a1, 0x0(v0)

/* i2c_stop */				
	/* free i2c bus */
	li	v0, LS2H_I2C_CR		
	li	v1, CR_STOP		
	sb	v1, 0x0(v0)		
1:					
	li	v0, LS2H_I2C_SR		
	lb	v1, 0x0(v0)		
	andi	v1, v1, SR_BUSY		
	bnez	v1, 1b			
	nop				
	
	move	v0, a1

	jr	ra
	nop
END(i2cread)
#endif
