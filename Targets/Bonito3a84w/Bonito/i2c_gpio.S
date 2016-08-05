#define GPIO_DIR_HIGH	(0xbfe00120)
#define GPIO_DATA_HIGH	(0xbfe0011c)
#define	G_OUTPUT	0
#define	G_INPUT		1

#define GPIO_SDA_DIR_SHIFT	1
#define	GPIO_SCL_DIR_SHIFT	5
#define GPIO_SDA_DATA_SHIFT	1
#define GPIO_SCL_DATA_SHIFT	5

/*
Just use t2,t4,t6,t9, k0,k1,a0,a1,a2,a3,v0,v1
*/

/*
 * used to sleep some nop for ddr_frenquency,
 * here it can support 600M-1000M 
 */

LEAF(_i2c_sleep)
	li 	t2,0x10
//	li 	t2,0x3
	sll	a0,t2,a0
	
1:	nop
	subu 	a0,1
	sync
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
	li	a1,GPIO_DIR_HIGH
	lwu	t2,0(a1)
	nop
	
	beqz	a0,1f
	nop
	ori	t2,t2,0x1<<GPIO_SDA_DIR_SHIFT
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
	li	a1,GPIO_DIR_HIGH
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
 * v1: tmp value
 */

LEAF(_sda_bit)
	li	a1,GPIO_DATA_HIGH
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
 * v1: tmp value
 */

LEAF(_scl_bit)
	li	a1,GPIO_DATA_HIGH
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
	move	k1,ra

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

	move	ra, k1
	jr 	ra
	nop
	
END(_i2c_start)

LEAF(_i2c_stop)
	move 	k1,ra

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
	
	move	ra, k1
	jr 	ra
	nop
END(_i2c_stop)
LEAF(_i2c_send_ack)
	move	k1,ra
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
	
	move	ra, k1
	jr 	ra
	nop
END(_i2c_send_ack)



LEAF(_i2c_rec_ack)
	move 	k1,ra
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

	li	t9,GPIO_DATA_HIGH
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

3:	li	t9,GPIO_DATA_HIGH
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
	
	move	ra, k1
	jr 	ra
	nop

END(_i2c_rec_ack)


LEAF(_i2c_rec)
	move 	k1,ra
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

	li	t4,GPIO_DATA_HIGH
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
	
1:
	move	ra, k1
	jr 	ra
	nop
	
END(_i2c_rec)

LEAF(_i2c_send)
	move	k1,ra
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

	move	ra, k1
	jr 	ra
	nop

END(_i2c_send)

/*
a0,a2:slave device addr
a1,a3:sub addr
v0:recieve data
v1:show if sucess,0:sucess,1:failure
  here no use v1,for detect_dimm use many
  registers,so,I can`t have enough register
  to store sucess or failure.
*/
LEAF(i2cread)
	move 	t6,ra
	nop
	move 	a2,a0
	move	a3,a1
	li	v0,0

	
	bal	_i2c_start
	nop
	sync

	and	a0, a2,0xfe
	bal	_i2c_send
	nop
	sync

	bal	_i2c_rec_ack
	nop
	sync
	
	move	a0,a3
	bal	_i2c_send
	nop
	sync

	bal	_i2c_rec_ack
	nop
	sync
	
	bal	_i2c_start
	nop
	sync

	move	a0,a2
	bal	_i2c_send
	nop
	sync
	bal	_i2c_rec_ack
	nop
	sync


	bal	_i2c_rec
	nop
	sync
	
	move	k0,v0

	li	a0,1
	bal	_i2c_send_ack
	nop
	sync
	bal	_i2c_stop
	nop
	sync

	move	v0,k0
	move 	a0,a2

	move	ra, t6
	jr 	ra
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

	move	ra, t6
	jr 	ra
	nop

END(i2cwrite)

