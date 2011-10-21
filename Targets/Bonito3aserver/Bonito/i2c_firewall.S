#define GPIO_DIR_HIGH	(0x900000001fe00120)  /* GPIO Direction Register */
#define GPIO_DATA_HIGH	(0x900000001fe0011c)  /* GPIO Data Register */
#define	G_OUTPUT	0
#define	G_INPUT		1

#define GPIO_SDA_DIR_SHIFT	1  /* GPIO1 used as SDA signal, R102 */
#define	GPIO_SCL_DIR_SHIFT	0  /* GPIO0 used as SCL signal, R94  */
#define GPIO_SDA_DATA_SHIFT	1
#define GPIO_SCL_DATA_SHIFT	0

/*
Just use t2,t4,t5,t6,t9, k0,k1,a0,a1,a2,a3,v0,v1
*/

/*
 * used to sleep some nop for ddr_frenquency,
 * here it can support 600M-1000M 
 */

LEAF(_i2c_sleep)
	li 	t2,0x10
	sll	a0,t2,a0
	
1:	nop
	subu 	a0,1
	bnez	a0,1b
	nop
	
	jr ra
	nop
	
END(_i2c_sleep)


/*
 * used to set SDA signal output or input
 * input: a0, 0 means set gpio output, while 1 means input
 * used : a1,t2,v1
 * a1: store the SDA address
 * t2: store SDA value
 * v1: tmp value
 */

LEAF(_sda_dir)
	dli	a1,GPIO_DIR_HIGH
	or a1, a1, k1
	lwu	t2,0(a1)
	nop
	
	beqz	a0,1f
	nop
	ori	t2,t2,0x1<<GPIO_SDA_DIR_SHIFT  //get the gpio1,for gpio used for i2c_sda
	nop
	b	2f
	nop
	
1:	li	v1,~(0x1<<GPIO_SDA_DIR_SHIFT)
	and 	t2,t2,v1
2:	sw	t2,0(a1)

	nop
	jr	ra
	nop
	
END(_sda_dir)


/*
 * used to set SCL signal input or output
 * input: a0, 0 means set gpio output, while 1 means input
 * used : a1,t2,v1
 * a1: store the SCL address
 * t2: store SCL value
 * v1: tmp value
 */

LEAF(_scl_dir)
	dli	a1,GPIO_DIR_HIGH
	or a1, a1, k1
	lwu	t2,0(a1)
	nop

	beqz	a0,1f
	nop
	ori	t2,t2,0x1<<GPIO_SCL_DIR_SHIFT
	b	2f
	nop
	
1:	li	v1,~(0x1<<GPIO_SCL_DIR_SHIFT)
	and 	t2,t2,v1
2:	sw	t2,0(a1)

	nop
	jr	ra
	nop

END(_scl_dir)


/*
 * used to set SDA signal high or low
 * input: a0, 0 means set gpio low, while 1 means high
 * used : a1,t2,k1,v1
 * a1: store the SDA address
 * t2: store SDA value
 * k1: store NODE_ID
 * v1: tmp value
 */

LEAF(_sda_bit)
	dli	a1,GPIO_DATA_HIGH
	or a1, a1, k1
	lwu	t2,0(a1)
	nop
	
	beqz	a0,1f
	nop
	ori	t2,t2,0x1<<GPIO_SDA_DATA_SHIFT
	b	2f
	nop
	
1:	li	v1,~(0x1<<GPIO_SDA_DATA_SHIFT)
	and 	t2,t2,v1
2:	sw	t2,0(a1)

	nop
	jr	ra
	nop
END(_sda_bit)


/*
 * used to set SCL signal high or low
 * input: a0, 0 means set gpio low, while 1 means high
 * used : a1,t2,k1,v1
 * a1: store the SCL address
 * t2: store SCL value
 * k1: store NODE_ID
 * v1: tmp value
 */

LEAF(_scl_bit)
	dli	a1,GPIO_DATA_HIGH
	or a1, a1, k1
	lwu	t2,0(a1)
	nop
	
	beqz	a0,1f
	nop
	ori	t2,t2,0x1<<GPIO_SCL_DATA_SHIFT
	b	2f
	nop
	
1:	li	v1,~(0x1<<GPIO_SCL_DATA_SHIFT)
	and 	t2,t2,v1
2:	sw	t2,0(a1)

	nop
	jr	ra
	nop
END(_scl_bit)

/*
* start the i2c,SCL holds high level and SDA turn down from high
* to low. And the matser is on OUTPUT module.
*/
LEAF(_i2c_start)
	move	t5,ra

	li 	a0,G_OUTPUT
	bal	_sda_dir
	nop
	li	a0,G_OUTPUT
	bal	_scl_dir
	nop
	li	a0,0
	bal	_scl_bit
	nop
	li	a0,1
	bal	_i2c_sleep
	nop
	li	a0,1
	bal	_sda_bit
	nop

	li	a0,1
	bal	_i2c_sleep
	nop
	li	a0,1
	bal	_scl_bit
	nop
	li	a0,5
	bal	_i2c_sleep
	nop
	li 	a0,0
	bal 	_sda_bit
	nop
	li	a0,5
	bal	_i2c_sleep
	nop
	li	a0,0
	bal	_scl_bit
	nop
	li	a0,2
	bal	_i2c_sleep
	nop

	jr 	t5
	nop
	
END(_i2c_start)

LEAF(_i2c_stop)
	move 	t5,ra

	li	a0,G_OUTPUT
	bal	_sda_dir
	nop
	li	a0,G_OUTPUT
	bal	_scl_dir
	nop
	li	a0,0
	bal	_scl_bit
	nop
	li	a0,1
	bal	_i2c_sleep
	nop
	li	a0,0
	bal	_sda_bit
	nop
	li 	a0,1
	bal	_i2c_sleep
	nop
	li	a0,1
	bal 	_scl_bit
	nop
	li	a0,5
	bal	_i2c_sleep
	nop
	li	a0,1
	bal	_sda_bit
	nop
	li	a0,5
	bal	_i2c_sleep
	nop

	li 	a0,0
	bal	_scl_bit
	nop
	li	a0,2
	bal	_i2c_sleep
	nop

	
	jr	t5
	nop
END(_i2c_stop)


LEAF(_i2c_send_ack)
	move	t5,ra
	move 	t4,a0

	li	a0,G_OUTPUT
	bal	_sda_dir
	nop

	move	a0,t4
	bal	_sda_bit
	nop

	li	a0,3
	bal	_i2c_sleep
	nop

	li 	a0,1
	bal	_scl_bit
	nop
	li	a0,5
	bal	_i2c_sleep
	nop
	li	a0,0
	bal	_scl_bit
	nop
	li	a0,2
	bal	_i2c_sleep
	nop
	
	jr	t5
	nop
END(_i2c_send_ack)



LEAF(_i2c_rec_ack)
	move 	t5,ra
	li	v0,1
	li	t4,10

	li	a0,G_INPUT
	bal	_sda_dir
	nop
	li	a0,3
	bal	_i2c_sleep
	nop
	li	a0,1
	bal	_scl_bit
	nop
	li	a0,5
	bal	_i2c_sleep
	nop

	dli	t9,GPIO_DATA_HIGH
	or  t9, t9, k1
	lwu	t9,0(t9)
	nop
	srl 	t9,t9,16
	andi	t9,t9,0x1<<(GPIO_SDA_DATA_SHIFT)

2:	beqz	t9,1f
	nop
	li	a0,1
	bal	_i2c_sleep
	nop
	subu	t4,t4,1
	
	bnez	t4,3f
	nop
	li	v0,0
	b	1f
	nop

3:	dli	t9,GPIO_DATA_HIGH
	or  t9, t9, k1
	lwu	t9,0(t9)
	nop
	srl 	t9,t9,16
	andi	t9,t9,0x1<<(GPIO_SDA_DATA_SHIFT)
	b	2b
	nop
	

1:	li	a0,0
	bal	_scl_bit
	nop
	li	a0,3
	bal	_i2c_sleep
	nop
	
	jr	t5
	nop	

END(_i2c_rec_ack)


LEAF(_i2c_rec)
	move 	t5,ra
	li	t9,0x7
	li	v0,0
	li	a0,G_INPUT
	bal	_sda_dir
	nop

2:	bltz	t9,1f
	nop


	li	a0,5
	bal	_i2c_sleep
	nop
	li	a0,1
	bal	_scl_bit
	nop
	li	a0,3
	bal	_i2c_sleep
	nop

	dli	t4,GPIO_DATA_HIGH
	or  t4, t4, k1
	lwu	t4,0(t4)
	nop
	srl 	t4,t4,16
	andi	t4,t4,0x1<<(GPIO_SDA_DATA_SHIFT)

	beqz 	t4,3f
	nop
	li	t4,1

3:	sll	t4,t4,t9   
	or	v0,v0,t4  
	li	a0,3
	bal	_i2c_sleep
	nop
	li	a0,0
	bal	_scl_bit
	nop

	sub	t9,t9,1
	b	2b
	nop
	
1:	jr t5
	nop
	
END(_i2c_rec)

LEAF(_i2c_send)
	move	t5,ra
	move	t4,a0
	li	t9,0x7
	
	li	a0,G_OUTPUT
	bal	_sda_dir
	nop
	
2:	bltz	t9,1f
	nop

	move	a0,t4
	srl	a0,a0,t9
	andi	a0,a0,1
	bal	_sda_bit
	nop
	
	li	a0,1
	bal	_i2c_sleep
	nop
	li	a0,1
	bal	_scl_bit
	nop
	li	a0,5
	bal	_i2c_sleep
	nop
	li	a0,0
	bal	_scl_bit
	nop
	li	a0,1
	bal	_i2c_sleep
	nop
	
	sub	t9,t9,1
	b	2b
	nop
1:	li	a0,1
	bal	_sda_bit
	nop

	jr	t5
	nop

END(_i2c_send)

/*
a0,a2:slave device addr
a1,a3:sub addr
k1: NODE_ID
v0:recieve data
v1:show if sucess,0:sucess,1:failure
  here no use v1,for detect_dimm use many
  registers,so,I can`t have enough register
  to store sucess or failure.
*/
LEAF(i2cread)
	move 	t6,ra
	nop
	dsll	k1,a2,44
	nop
  	move	a2,a0
	move	a3,a1
	li	v0,0
//	li	v1,0
	
	bal	_i2c_start
	nop

	andi	a0, a2,0xfe
	bal	_i2c_send
	nop

	bal	_i2c_rec_ack
	nop
	
	move	a0,a3
	bal	_i2c_send
	nop

	bal	_i2c_rec_ack
	nop
	
	bal	_i2c_start
	nop

	move	a0,a2
	bal	_i2c_send
	nop
	bal	_i2c_rec_ack
	nop

	
	bal	_i2c_rec
	nop

	
	move	k0,v0
	
	li	a0,1
	bal	_i2c_send_ack
	nop
	

	bal	_i2c_stop
	nop

//	li	v1,0
	move	v0,k0
	move 	a0,a2
	b	2f
	nop

//1:	li	v1,1
2:	jr	t6
	nop

END(i2cread)


/*
 *a0: slave_addr
 *a1: sub_addr
 *a2: value
 *write is also good for i2c,
 *you can use it derectly in need.
 */
LEAF(i2cwrite)
	move 	t6,ra
	nop
	move	a3,a0
	move	a2,a1

	bal	_i2c_start
	nop

	move	a0,a3
	bal	_i2c_send
	nop

	bal	_i2c_rec_ack
	nop
//	beqz	v0,1f
//	nop

	//move	a0,a1
	move	a0,a2
	bal	_i2c_send
	nop


	bal	_i2c_rec_ack
	nop
//	beqz	v0,1f
//	nop
	
	move	a0,a2
	bal	_i2c_send
	nop

	bal	_i2c_rec_ack
	nop
//	beqz	v0,1f
//	nop

	bal	_i2c_stop
	nop

	b	2f
	nop

//1:	li	v1,1
2:	jr	t6
	nop

END(i2cwrite)


/*
* this function isn`t checked.
* The correct of this function is not sure.
*/
LEAF(isl12027_watchdog)
	move	t1,ra	

	li	a0,0xde
	li	a1,0x3f
	li	a2,0x2
	bal	i2cwrite
	nop

//PRINTSTR("setp1\r\n")
	li	a0,0xde
	li	a1,0x3f
	li	a2,0x6
	bal	i2cwrite
	nop

//PRINTSTR("setp2\r\n")
	li	a0,0xde
	li	a1,0x14
	li	a2,0x84
	bal	i2cwrite
	nop

//PRINTSTR("setp3\r\n")


	li	a0,0xde
	li	a1,0x3f
	li	a2,0x2
	bal	i2cwrite
	nop

//PRINTSTR("setp1\r\n")
	li	a0,0xde
	li	a1,0x3f
	li	a2,0x6
	bal	i2cwrite
	nop

//PRINTSTR("setp2\r\n")
	li	a0,0xde
	li	a1,0x10
	li	a2,0x18
	bal	i2cwrite
	nop

//PRINTSTR("setp3\r\n")
	jr	t1
	nop
	
END(isl12027_watchdog)

