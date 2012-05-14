//Modify for LS1X 32 bit system
//!!!!note: inputaddress and Inputhex have been changed by cxk!!!!
//note: inputaddress will damage t9
    b       22f
    nop
inputaddress:
	  move t9,ra
	  PRINTSTR("8'h");
	  li a0,8
	  bal Inputhex
	  nop
	  jr t9
	  nop

LEAF(Inputhex)
/*********
 * v0: return value
 * v1: volatile
 * a0: input--loop times, then used for tgt_putchar input param
 * a1: loop counter
 * a2: temp store of inputted value
 * a3: store ra
**********/
	move a3,ra
	move a1,a0
	move a2,zero
1:
	bal tgt_getchar
	nop
	li v1,'q'
	beq v0,v1,3f
	nop
	li v1,0x20	//space
	beq v1,v0,3f
	nop
	li v1,0xd	//CR
	beq v1,v0,3f
	nop
	li v1,0x8	//backspace
	beq v1,v0,4f
	nop
	li v1,'x'
	beq v1,v0,4f
	nop

	slt v1,v0,'0'
	bnez v1,1b
	nop
	slt v1,v0,'9'+1
	bnez v1,2f
	move v1,v0
	slt v1,v0,'a'
	bnez v1,1b
	nop
	slt v1,v0,'f'+1
	beqz v1,1b
	move v1,v0
	addi v0,10-'a'+'0'
2:	
	addi v0,0-'0'
	dsll a2,4
	or a2,v0,a2	
	move a0,v1
	bal tgt_putchar
	nop
	addi a1,-1
	bnez a1,1b
	nop
	li v1,0
3:
	move v0,a2
	jr a3
	nop
4:
	dsrl a2,4
	li	v1, 16
	bgeu a1, v1, 1b
	nop
	addu a1,1
	li a0,'\b'
	bal tgt_putchar
	nop
	li a0,0x20	  //space
	bal tgt_putchar
	nop
	li a0,'\b'
	bal tgt_putchar
	nop
	b 1b
	nop
END(Inputhex)

22:
