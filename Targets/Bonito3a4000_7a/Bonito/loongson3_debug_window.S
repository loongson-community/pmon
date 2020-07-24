/* 
 * input: 
 *      s1: node id
 */

    PRINTSTR("\r\n-----============")
    PRINTSTR("node ")
    dsrl    a0, s1, 44
    bal	    hexserial
	nop
	PRINTSTR("============-----")
#if 0
/*****************************************************/
	PRINTSTR("\r\n======X1 core0 map windows:\r\n")
	li      t1, 23
	dli     t2, 0x900000003ff02000
    or	    t2, t2, s1
1:
	move	a0, t2
	bal	    hexserial64
	nop
	PRINTSTR(": ")

	ld      a0, 0x0(t2)
	bal     hexserial64
	nop
	PRINTSTR("\r\n")

	daddiu  t2, t2, 8
	bnez    t1, 1b
	addiu   t1, t1, -1
/*****************************************************/
	PRINTSTR("\r\n======X1 ht0 map windows:\r\n")
	li      t1, 23
	dli     t2, 0x900000003ff02600    
	or	    t2, t2, s1
1:
	move	a0, t2
	bal     hexserial64
	nop
	PRINTSTR(": ")

	ld      a0, 0x0(t2)
	bal     hexserial64
	nop
	PRINTSTR("\r\n")

	daddiu  t2, t2, 8
	bnez    t1, 1b
	addiu   t1, t1, -1
/*****************************************************/
	PRINTSTR("\r\n======X1 ht1 map windows:\r\n")
	li      t1, 23
	dli     t2, 0x900000003ff02700    
	or	    t2, t2, s1
1:
	move	a0, t2
	bal     hexserial64
	nop
	PRINTSTR(": ")

	ld      a0, 0x0(t2)
	bal     hexserial64
	nop
	PRINTSTR("\r\n")

	daddiu  t2, t2, 8
	bnez    t1, 1b
	addiu   t1, t1, -1
/*****************************************************/
	PRINTSTR("\r\n======X2 cpu map windows:\r\n")
	li      t1, 23
	dli     t2, 0x900000003ff00000
	or	    t2, t2, s1
1:
	move	a0, t2
	bal     hexserial64
	nop
	PRINTSTR(": ")

	ld      a0, 0x0(t2)
	bal     hexserial64
	nop
	PRINTSTR("\r\n")

	daddiu  t2, t2, 8
	bnez    t1, 1b
	addiu   t1, t1, -1
/*****************************************************/
	PRINTSTR("\r\n======X2 pci map windows:\r\n")
	li      t1, 23
	dli     t2, 0x900000003ff00100
	or	    t2, t2, s1
	1:
	move	a0, t2
	bal     hexserial64
	nop
	PRINTSTR(": ")

	ld      a0, 0x0(t2)
	bal     hexserial64
	nop
	PRINTSTR("\r\n")

	daddiu  t2, t2, 8
	bnez    t1, 1b
	addiu   t1, t1, -1
#else
    /*****************************************************/
	PRINTSTR("\r\n======X1 core0 map windows:\r\n")
	li      t1, 3
	li      t3, 0
	dli     t2, 0x900000003ff02000
    or	    t2, t2, s1
1:
	move	a0, t2
	bal	    hexserial64
	nop
	PRINTSTR(": ")

	ld      a0, 0x0(t2)
	bal     hexserial64
	nop
	PRINTSTR("\r\n")

	daddiu  t2, t2, 0x40
	addiu   t1, t1, -1
	bnez    t1, 1b
	nop
	dsubu   t2, t2, 0xc0
	daddu   t2, t2, 0x8
	li      t1, 3
	add     t3, t3, 1
	blt    t3, 8, 1b
	nop	
/*****************************************************/
	PRINTSTR("\r\n======X1 ht0 map windows:\r\n")
	li      t1, 3
	li      t3, 0
	dli     t2, 0x900000003ff02600
    or	    t2, t2, s1
1:
	move	a0, t2
	bal	    hexserial64
	nop
	PRINTSTR(": ")

	ld      a0, 0x0(t2)
	bal     hexserial64
	nop
	PRINTSTR("\r\n")

	daddiu  t2, t2, 0x40
	addiu   t1, t1, -1
	bnez    t1, 1b
	nop
	dsubu   t2, t2, 0xc0
	daddu   t2, t2, 0x8
	li      t1, 3
	add     t3, t3, 1
	blt    t3, 8, 1b
	nop	
/*****************************************************/
	PRINTSTR("\r\n======X1 ht1 map windows:\r\n")
	li      t1, 3
	li      t3, 0
	dli     t2, 0x900000003ff02700
    or	    t2, t2, s1
1:
	move	a0, t2
	bal	    hexserial64
	nop
	PRINTSTR(": ")

	ld      a0, 0x0(t2)
	bal     hexserial64
	nop
	PRINTSTR("\r\n")

	daddiu  t2, t2, 0x40
	addiu   t1, t1, -1
	bnez    t1, 1b
	nop
	dsubu   t2, t2, 0xc0
	daddu   t2, t2, 0x8
	li      t1, 3
	add     t3, t3, 1
	blt    t3, 8, 1b
	nop	
/*****************************************************/
	PRINTSTR("\r\n======X2 cpu map windows:\r\n")
	li      t1, 3
	li      t3, 0
	dli     t2, 0x900000003ff00000
    or	    t2, t2, s1
1:
	move	a0, t2
	bal	    hexserial64
	nop
	PRINTSTR(": ")

	ld      a0, 0x0(t2)
	bal     hexserial64
	nop
	PRINTSTR("\r\n")

	daddiu  t2, t2, 0x40
	addiu   t1, t1, -1
	bnez    t1, 1b
	nop
	dsubu   t2, t2, 0xc0
	daddu   t2, t2, 0x8
	li      t1, 3
	add     t3, t3, 1
	blt    t3, 8, 1b
	nop	
/*****************************************************/
	PRINTSTR("\r\n======X2 pci map windows:\r\n")
	li      t1, 3
	li      t3, 0
	dli     t2, 0x900000003ff00100
    or	    t2, t2, s1
1:
	move	a0, t2
	bal	    hexserial64
	nop
	PRINTSTR(": ")

	ld      a0, 0x0(t2)
	bal     hexserial64
	nop
	PRINTSTR("\r\n")

	daddiu  t2, t2, 0x40
	addiu   t1, t1, -1
	bnez    t1, 1b
	nop
	dsubu   t2, t2, 0xc0
	daddu   t2, t2, 0x8
	li      t1, 3
	add     t3, t3, 1
	blt    t3, 8, 1b
	nop	
#endif
/*****************************************************/
	PRINTSTR("\r\n======read CPU HT config reg:\r\n")
	dli     t2, 0x90000efdfb000000
	or	    t2, t2, s1

	move	a0, t2
	bal     hexserial64
	nop
	PRINTSTR(": ")

	ld      a0, 0x0(t2)
	bal     hexserial64
	nop
	PRINTSTR("\r\n")

    //receive window
	daddiu	a0, t2, 0x60
	bal     hexserial64
	nop
	PRINTSTR(": ")

	ld      a0, 0x60(t2)
	bal     hexserial64
	nop
	PRINTSTR("\r\n")

	daddiu	a0, t2, 0x68
	bal     hexserial64
	nop
	PRINTSTR(": ")

	ld      a0, 0x68(t2)
	bal     hexserial64
	nop
	PRINTSTR("\r\n")

	daddiu	a0, t2, 0x70
	bal     hexserial64
	nop
	PRINTSTR(": ")

	ld      a0, 0x70(t2)
	bal     hexserial64
	nop    
	PRINTSTR("\r\n")
	PRINTSTR("\r\n")
	
    //uncache window
    daddiu	a0, t2, 0xf0
	bal     hexserial64
	nop
	PRINTSTR(": ")

	ld      a0, 0xf0(t2)
	bal     hexserial64
	nop    
	PRINTSTR("\r\n")

	daddiu	a0, t2, 0xf8
	bal     hexserial64
	nop
	PRINTSTR(": ")

	ld      a0, 0xf8(t2)
	bal     hexserial64
	nop    
	PRINTSTR("\r\n")

	daddiu	a0, t2, 0x168
	bal     hexserial64
	nop
	PRINTSTR(": ")

	ld      a0, 0x168(t2)
	bal     hexserial64
	nop    
	PRINTSTR("\r\n")

/*****************************************************/
	PRINTSTR("\r\n======read 7A HT config reg:\r\n")    //do not use "ld"
	dli     t2, 0x90000efdfe000000
	or	    t2, t2, s1

	move	a0, t2
	bal     hexserial64
	nop
	PRINTSTR(": ")

	lw      a0, 0x4(t2)
	bal     hexserial
	nop
	lw      a0, 0x0(t2)
	bal     hexserial
	nop
	PRINTSTR("\r\n")
	PRINTSTR("\r\n")

    //receive window
	daddiu	a0, t2, 0x140
	bal     hexserial64
	nop
	PRINTSTR(": ")

	lw      a0, 0x144(t2)
	bal     hexserial
	nop
	lw      a0, 0x140(t2)
	bal     hexserial
	nop
	PRINTSTR("\r\n")

	daddiu	a0, t2, 0x148
	bal     hexserial64
	nop
	PRINTSTR(": ")

	lw      a0, 0x14c(t2)
	bal     hexserial
	nop
	lw      a0, 0x148(t2)
	bal     hexserial
	nop
	PRINTSTR("\r\n")

	daddiu	a0, t2, 0x150
	bal     hexserial64
	nop
	PRINTSTR(": ")
	
	lw      a0, 0x154(t2)
	bal     hexserial
	nop
	lw      a0, 0x150(t2)
	bal     hexserial
	nop
	PRINTSTR("\r\n")
    
    daddiu	a0, t2, 0x158
	bal     hexserial64
	nop
	PRINTSTR(": ")

	lw      a0, 0x15c(t2)
	bal     hexserial
	nop
	lw      a0, 0x158(t2)
	bal     hexserial
	nop
	PRINTSTR("\r\n")

	daddiu	a0, t2, 0x160
	bal     hexserial64
	nop
	PRINTSTR(": ")

    lw      a0, 0x164(t2)
	bal     hexserial
	nop
	lw      a0, 0x160(t2)
	bal     hexserial
	nop
    PRINTSTR("\r\n")
    PRINTSTR("\r\n")
    
    //Transmit Post Window
	daddiu	a0, t2, 0x170
	bal     hexserial64
	nop
	PRINTSTR(": ")

	lw      a0, 0x174(t2)
	bal     hexserial
	nop
	lw      a0, 0x170(t2)
	bal     hexserial
	nop
	PRINTSTR("\r\n")

	daddiu	a0, t2, 0x178
	bal     hexserial64
	nop
	PRINTSTR(": ")

	lw      a0, 0x17c(t2)
	bal     hexserial
	nop
	lw      a0, 0x178(t2)
	bal     hexserial
	nop
	PRINTSTR("\r\n")
	PRINTSTR("\r\n")

	//Receive P2P Window
	daddiu	a0, t2, 0x1b0
	bal     hexserial64
	nop
	PRINTSTR(": ")

	lw      a0, 0x1b4(t2)
	bal     hexserial
	nop
	lw      a0, 0x1b0(t2)
	bal     hexserial
	nop
	PRINTSTR("\r\n")

	daddiu	a0, t2, 0x1b8
	bal     hexserial64
	nop
	PRINTSTR(": ")

	lw      a0, 0x1bc(t2)
	bal     hexserial
	nop
	lw      a0, 0x1b8(t2)
	bal     hexserial
	nop
    PRINTSTR("\r\n-----========================-----\r\n")
	
