/*whd : loongson3_HT_init.S
        used to set up the HyperTransport interface
        and the South bridge
        mainly for MCP68
*/

######################################################
#define HT_32bit_TRANS
//#define HT_800M
#define HT_RECONNECT
//#define HT_REG_TRANS
######################################################

#ifdef HT_32bit_TRANS //PCI CFG : TYPE 0: 
	TTYDBG("32 bit PCI space translate to 64 bit HT space\r\n")

    dli t0, 0x900000003ff02000
    dli t2, 0x900000003ff02700

1:
 //map HT: PCI IO : 0x90000efd_fc000000 --> 0x18000000
 //map              0x90000efd_fd000000 --> 0x19000000
 //map HT: PCI CFG: 0x90000efd_fe000000 --> 0x1a000000
 //map HT: PCI CFG: 0x90000efd_ff000000 --> 0x1b000000
    dli t1, 0x000000001b000000
    sd  t1, 0x0(t0)
    dli t1, 0xffffffffff000000
    sd  t1, 0x40(t0)
    dli t1, 0x00000e001f0000f7
    sd  t1, 0x80(t0)

#if 1 // why it stop here if this code enabled?; just for pcie
// map PCIE space to 0x10000000
    dli t1, 0x0000000018000000
    sd  t1, 0x08(t0)
    dli t1, 0xffffffffff000000
    sd  t1, 0x48(t0)
    dli t1, 0x00000e00180000f7
    sd  t1, 0x88(t0)

    dli t1, 0x0000000010000000
    sd  t1, 0x10(t0)
    dli t1, 0xfffffffff8000000
    sd  t1, 0x50(t0)
    dli t1, 0x00000e00100000f7
    sd  t1, 0x90(t0)

#endif

// below added for ls2h dc frame buffer
  
 //map 0x90000e00_00000000 --> 0x40000000
    dli t1, 0x0000000040000000
    sd  t1, 0x18(t0)
    dli t1, 0xffffffffc0000000
    sd  t1, 0x58(t0)
    dli t1, 0x00000e00000000f7
    sd  t1, 0x98(t0)

#if 0
 //HT Space enable
 //map 0x90000e00_00000000 --> 0x90000e00_00000000
    dli t1, 0x00000e0000000000
    sd  t1, 0x8(t0)
    dli t1, 0xffffff0000000000
    sd  t1, 0x48(t0)
    dli t1, 0x00000e00000000f7
    sd  t1, 0x88(t0)
#if 0
 //HT: PCI LO BASE
 //map 0x90000e00_00000000 --> 0x10000000
    dli t1, 0x0000000010000000
    sd  t1, 0x10(t0)
    dli t1, 0xfffffffffc000000
    sd  t1, 0x50(t0)
    dli t1, 0x00000e00000000f7
    sd  t1, 0x90(t0)
#endif

 //HT: PCI HI BASE
 //map 0x90000e00_10000000 --> 0x10000000
 //map 0x90000e00_40000000 --> 0x40000000
    dli t1, 0x0000000040000000
    sd  t1, 0x18(t0)
    dli t1, 0xffffffffc0000000
    sd  t1, 0x58(t0)
    dli t1, 0x00000e00400000f7
    sd  t1, 0x98(t0)

#ifdef HT_REG_TRANS
 //HT REG BASE
 //map 0x90000efd_fb000000 --> 0x1e000000
    dli t1, 0x000000001e000000
    sd  t1, 0x20(t0)
    dli t1, 0xffffffffff000000
    sd  t1, 0x60(t0)
    dli t1, 0x00000efdfb0000f7
    sd  t1, 0xa0(t0)
#else
 //map 0x90000e00_00000000 --> 0x1e000000
    dli t1, 0x000000001e000000
    sd  t1, 0x20(t0)
    dli t1, 0xffffffffff000000
    sd  t1, 0x60(t0)
    dli t1, 0x00000e00000000f7
    sd  t1, 0xa0(t0)
#endif
#endif

    daddiu  t0, t0, 0x100
    bne     t0, t2, 1b
    nop

#endif



//////////////////
up_here:


#if 1//wait until HT link up
    TTYDBG("Waiting HyperTransport bus to be up.")
    dli     t0, 0x90000efdfb000000
	li	    t1, 0x1f
1:
    lw      a0, 0x44(t0)
	#bal	hexserial
    nop
	beqz	t1,2f
    nop
	TTYDBG(">")
	addi	t1, t1, -1
	b	    3f
    nop
2:
	TTYDBG("\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b=")
	li	    t1, 0x1f

3:
    lw      a0, 0x44(t0)
	li	    a1, 0x20
	and	    a0, a0, a1

    beqz	a0,	1b
	nop

	TTYDBG("\r\n")
    lw      a0, 0x44(t0)
	bal	    hexserial
    nop
	TTYDBG("\r\n")
#endif

// xqch
#if 1	// below added to fix ls2gq2h HT connection fail bugs 
    dli     t0, 0x90000efdfb000000
    lw      a0, 0x44(t0)
	andi	a0, 0x100
	beqz	a0, check_b12_0
	nop
	b		reset_htbus_0
	nop
check_b12_0:
    lw      a0, 0x48(t0)
	andi	a0, 0x1000
	beqz	a0, config_over_0
	nop
	b		reset_htbus_0
	nop
config_over_0:
    //lw      a0, 0x3c(t0)
    //dli     t0, 0x90000efdfe000000
	//li		a0, 0x01
	//sw		a0, 0x3c(t0)
	b		fix2gq2h_over_0
	nop
reset_htbus_0:

#if 1
	TTYDBG("RESET HT bus\r\n")
	dli	    t2, 0x90000efdfb000000
    dli     t3, 0x00000000
    sw      t3, 0x3c(t2)
    dli     t3, 0x00400000
    sw      t3, 0x3c(t2)
#endif

#if 1//wait until HT link down
    TTYDBG("Waiting HyperTransport bus to be down.")
    dli     t0, 0x90000efdfb000000
	li	    t1, 0x1f
1:
    lw      a0, 0x44(t0)
	#bal	hexserial
    nop
	beqz	t1,2f
	TTYDBG(">")
	addi	t1, t1, -1
	b	    3f
    nop
2:
	TTYDBG("\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b=")
	li	    t1, 0x1f

3:
    lw      a0, 0x44(t0)
	li	    a1, 0x20
	and	    a0, a0, a1

    bnez	a0,	1b
	nop

	TTYDBG("\r\n")
    lw      a0, 0x44(t0)
	bal	    hexserial
    nop
	TTYDBG("\r\n")
#endif

	li		a0, 0xfffffeff
	lw		a1, 0x44(t0)
	and		a0, a0, a1
	sw		a0, 0x44(t0)

	li		a0, 0xffffefff
	lw		a1, 0x48(t0)
	and		a0, a0, a1
	sw		a0, 0x48(t0)

    dli     t3, 0x00000000
    sw      t3, 0x3c(t0)

    b       up_here
    nop

	
fix2gq2h_over_0:

#endif

/////////////////////
#if 1 //FIX 3A rx buffer problem
	TTYDBG("FIX 3A rx buffer problem\r\n")
	dli	    t2, 0x90000efdfe000100
    li      t3, 0x11111111
    sw      t3, 0x8(t2)
    li      t3, 0x0
    sw      t3, 0x8(t2)

    lw      a0, 0x0(t2)
    li      t3, 0x01090909
    //li      t3, 0x020a0a0a
    beq     a0, t3, data_buffer_check
    nop

	TTYDBG("rx buffer ERROR found in cmd buffer!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\r\n")
    lw      a0, 0x0(t2)
    bal     hexserial
    nop

    dli     a0, 0x1000
1:
    daddi   a0, a0, -1
    bnez    a0, 1b
    nop

	dli	    t2, 0x90000efdfe000100
    lw      a0, 0x0(t2)
    nop

#if 1
//PC CMD
    li      t3, 0x00000009
    li      a1, 0x000000ff
    and     a2, a1, a0
    beq     a2, t3, 1f
    nop

    sub     a1, t3, a2
    bgtz    a1, 2f
    nop

    li      t3, 0x11111110
    sw      t3, 0x8(t2)
    li      t3, 0x0
    sw      t3, 0x8(t2)

    li      t3, 0x0000000a
    sub     a1, a2, t3
    li      t3, 0x30000000
    or      t3, t3, a1
    sw      t3, 0x8(t2)
    li      t3, 0x0
    sw      t3, 0x8(t2)

    b       1f
    nop

2:
    li      t3, 0x10000000
    or      t3, t3, a1
    sw      t3, 0x8(t2)
    li      t3, 0x0
    sw      t3, 0x8(t2)


#if 1
1:
//NPC CMD
    li      t3, 0x00000900
    li      a1, 0x0000ff00
    and     a2, a1, a0
    beq     a2, t3, 1f
    nop

    sub     a1, t3, a2
    bgtz    a1, 2f
    nop

    li      t3, 0x11111101
    sw      t3, 0x8(t2)
    li      t3, 0x0
    sw      t3, 0x8(t2)

    li      t3, 0x00000a00
    sub     a1, a2, t3
    li      t3, 0x30000000
    srl     a1, 4
    or      t3, t3, a1
    sw      t3, 0x8(t2)
    li      t3, 0x0
    sw      t3, 0x8(t2)

    b       1f
    nop

2:
    li      t3, 0x10000000
    srl     a1, 4
    or      t3, t3, a1
    sw      t3, 0x8(t2)
    li      t3, 0x0
    sw      t3, 0x8(t2)
#endif    

1:
//R CMD
    li      t3, 0x00090000
    li      a1, 0x00ff0000
    and     a2, a1, a0
    beq     a2, t3, 1f
    nop

    sub     a1, t3, a2
    bgtz    a1, 2f
    nop

    li      t3, 0x11111011
    sw      t3, 0x8(t2)
    li      t3, 0x0
    sw      t3, 0x8(t2)

    li      t3, 0x000a0000
    sub     a1, a2, t3
    li      t3, 0x30000000
    srl     a1, 8
    or      t3, t3, a1
    sw      t3, 0x8(t2)
    li      t3, 0x0
    sw      t3, 0x8(t2)

    b       1f
    nop

2:
    li      t3, 0x10000000
    srl     a1, 8
    or      t3, t3, a1
    sw      t3, 0x8(t2)
    li      t3, 0x0
    sw      t3, 0x8(t2)
    

1:
#else
    TTYDBG("\r\nERROR detected in cmd buffer\r\n")
#endif


data_buffer_check:

    lw      a0, 0x4(t2)
    li      t3, 0x00080808
    //li      t3, 0x00090909
    beq     a0, t3, buffer_check_out
    nop

#if 1
	TTYDBG("\r\nrx buffer ERROR found in data buffer!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\r\n")
    lw      a0, 0x4(t2)
    bal     hexserial
    nop
    lw      a0, 0x4(t2)

//PC DATA
    li      t3, 0x00000008
    li      a1, 0x000000ff
    and     a2, a1, a0
    beq     a2, t3, 1f
    nop

    sub     a1, t3, a2
    bgtz    a1, 2f
    nop

    li      t3, 0x11101111
    sw      t3, 0x8(t2)
    li      t3, 0x0
    sw      t3, 0x8(t2)

    li      t3, 0x00000009
    sub     a1, a2, t3
    li      t3, 0x30000000
    sll     a1, 16
    or      t3, t3, a1
    sw      t3, 0x8(t2)
    li      t3, 0x0
    sw      t3, 0x8(t2)

    b       1f
    nop

2:
    li      t3, 0x10000000
    sll     a1, 16
    or      t3, t3, a1
    sw      t3, 0x8(t2)
    li      t3, 0x0
    sw      t3, 0x8(t2)
    

#if 0 //Should NOT be turn on for BUGs
1:
//NPC DATA
    li      t3, 0x00000800
    li      a1, 0x0000ff00
    and     a2, a1, a0
    beq     a2, t3, 1f
    nop

    sub     a1, t3, a2
    bgtz    a1, 2f
    nop

    li      t3, 0x11011111
    sw      t3, 0x8(t2)
    li      t3, 0x0
    sw      t3, 0x8(t2)

    li      t3, 0x00000900
    sub     a1, a2, t3
    li      t3, 0x30000000
    sll     a1, 12
    or      t3, t3, a1
    sw      t3, 0x8(t2)
    li      t3, 0x0
    sw      t3, 0x8(t2)

    b       1f
    nop

2:
    li      t3, 0x10000000
    sll     a1, 12
    or      t3, t3, a1
    sw      t3, 0x8(t2)
    li      t3, 0x0
    sw      t3, 0x8(t2)
#endif

1:
//R DATA
    li      t3, 0x00080000
    li      a1, 0x00ff0000
    and     a2, a1, a0
    beq     a2, t3, 1f
    nop

    sub     a1, t3, a2
    bgtz    a1, 2f
    nop

    li      t3, 0x10111111
    sw      t3, 0x8(t2)
    li      t3, 0x0
    sw      t3, 0x8(t2)

    li      t3, 0x00090000
    sub     a1, a2, t3
    li      t3, 0x30000000
    sll     a1, 8
    or      t3, t3, a1
    sw      t3, 0x8(t2)
    li      t3, 0x0
    sw      t3, 0x8(t2)

    b       1f
    nop

2:
    li      t3, 0x10000000
    sll     a1, 8
    or      t3, t3, a1
    sw      t3, 0x8(t2)
    li      t3, 0x0
    sw      t3, 0x8(t2)
    

1:
#else
    TTYDBG("ERROR detected in data buffer\r\n")
#endif

buffer_check_out:

	dli	    t2, 0x90000efdfe000100
    li      t3, 0x30000000
    sw      t3, 0x8(t2)
    li      t3, 0x0
    sw      t3, 0x8(t2)

#endif


/////////////////////
#if 1//Print 2H rx buffer number
	TTYDBG("\r\nPrint 2H HT tx buffer(which is sent by 3A)\r\n")
	dli	    t2, 0x90000efdfe000100
    lw      a0, 0x0(t2)
    bal     hexserial
    nop
	TTYDBG("\r\n")
    nop
    lw      a0, 0x4(t2)
    bal     hexserial
    nop
	TTYDBG("\r\n")
    nop

#endif

#if 1 //Set 2H receive space
#if 1
	TTYDBG("Set 2H HT receive space\r\n")
	dli	    t2, 0x90000efdfe000000
    dli     t3, 0x80000000
    sw      t3, 0x60(t2)
    dli     t3, 0x00008000
    sw      t3, 0x64(t2)
#else
	TTYDBG("Set 2H HT receive space 0x80000000~0x8fffffff\r\n")
	dli	    t2, 0x90000efdfe000000
    dli     t3, 0x80000000
    sw      t3, 0x60(t2)
    dli     t3, 0x0080fff0
    sw      t3, 0x64(t2)

	TTYDBG("Set 2H HT receive space 0x0000000000~0x7fffffffff\r\n")
	dli	    t2, 0x90000efdfe000000
    dli     t3, 0x80000000
    sw      t3, 0x68(t2)
    dli     t3, 0x00008000
    sw      t3, 0x6c(t2)

#endif

	TTYDBG("Set some space in 2H HT receive space UNCACHED: 0x00000000 - 0x2000000\r\n")
	dli	    t2, 0x90000efdfe000000
    dli     t3, 0x80000000
    sw      t3, 0xf0(t2)
    //dli     t3, 0x0010fff0
    dli     t3, 0x0000ffe0
    sw      t3, 0xf4(t2)

#endif

#if 1//Enable 2H DMA cached address
	TTYDBG("Enable 2H DMA cached address\r\n")
	dli	    t2, 0x90000efdfe000000
    dli     t3, 0x80000000
    sw      t3, 0xe0(t2)
    dli     t3, 0x00008000
    sw      t3, 0xe4(t2)
#endif


#if 1//OPEN RX SPACE in 3A
	TTYDBG("HT RX DMA address ENABLE\r\n")
	dli	    t2, 0x90000efdfb000060
	li	    t0, 0xc0000000
	sw	    t0, 0x0(t2)
	li	    t0, 0x0000fff0
	sw	    t0, 0x4(t2)
	TTYDBG("HT RX DMA address ENABLE done 1\r\n")

#if 1
	li	t0, 0xc0000080
	sw	t0, 0x08(t2)
	li	t0, 0x0000ffc0
	sw	t0, 0x0c(t2)
	TTYDBG("HT RX DMA address ENABLE done 2\r\n")

	li	t0, 0x80000000
	sw	t0, 0x10(t2)
	li	t0, 0x00008000
	sw	t0, 0x14(t2)
	TTYDBG("HT RX DMA address ENABLE done 3\r\n")
#endif


#if 0//Set Mem space post
	TTYDBG("Set HT Memory space all post\r\n")
	dli	    t2, 0x90000efdfb000000
	li	    t0, 0x80000010
	sw	    t0, 0xd0(t2)
	li	    t0, 0x0010fff8
	sw	    t0, 0xd4(t2)

#endif

#endif


#if 1//SET 2H DMA write POST
	TTYDBG("HT RX DMA write SET to POST\r\n")
	dli	    t2, 0x90000efdfb0000d0
	li	    t0, 0x80000080
	sw	    t0, 0x0(t2)
	li	    t0, 0x0080ff80
	sw	    t0, 0x4(t2)

	//li	t0, 0x80000000
	//sw	t0, 0x8(t2)
	//li	t0, 0x0000ffc0
	//sw	t0, 0xc(t2)
#endif



#ifdef HT_RECONNECT

#ifdef HT_800M//Set HT bridge to be 800Mhz
	TTYDBG("Setting HyperTransport Controller to be 800Mhz\r\n")
	dli	    t2, 0x90000efdfb000000
	#li	    t0, 0x2 //Frequency: 400 Mhz
	li	    t0, 0x5 //Frequency: 800 Mhz
	sb	    t0, 0x49(t2)
    lw      a0, 0x48(t2)
	bal	    hexserial
    nop
	TTYDBG("\r\n")
#endif


#ifdef HT_800M //Write Southbridge to 800Mhz
	TTYDBG("Setting HyperTransport Southbridge to be 800M\r\n")
    dli	    t2, 0x90000efdfe000000
	#li	    t0, 0x2 //Frequency : 400Mhz
	li	    t0, 0x5 //Frequency: 800 Mhz
	sb	    t0, 0x49(t2)
    lw      a0, 0x48(t2)
	bal	    hexserial
    nop
	TTYDBG("\r\n")
#endif

#if 1 //RESET HT BUS

#if 1
	TTYDBG("RESET HT bus\r\n")
	dli	    t2, 0x90000efdfb000000
    dli     t3, 0x00000000
    sw      t3, 0x3c(t2)
    dli     t3, 0x00400000
    sw      t3, 0x3c(t2)
#endif

#if 1//wait until HT link down
    TTYDBG("Waiting HyperTransport bus to be down.")
    dli     t0, 0x90000efdfb000000
	li	    t1, 0x1f
1:
    lw      a0, 0x44(t0)
	#bal	hexserial
    nop
	beqz	t1,2f
	TTYDBG(">")
	addi	t1, t1, -1
	b	    3f
    nop
2:
	TTYDBG("\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b=")
	li	    t1, 0x1f

3:
    lw      a0, 0x44(t0)
	li	    a1, 0x20
	and	    a0, a0, a1

    bnez	a0,	1b
	nop

	TTYDBG("\r\n")
    lw      a0, 0x44(t0)
	bal	    hexserial
    nop
	TTYDBG("\r\n")
#endif

    dli     t3, 0x00000000
    sw      t3, 0x3c(t2)

#if 0 //RESET forever
    b       up_here
    nop
#endif

#endif

	
wait_up:

#if 1//wait until HT link up
    TTYDBG("Waiting HyperTransport bus to be up.")
    dli     t0, 0x90000efdfb000000
	li	    t1, 0x1f
1:
    lw      a0, 0x44(t0)
	#bal	hexserial
    nop
	beqz	t1,2f
	TTYDBG(">")
	addi	t1, t1, -1
	b	    3f
    nop
2:
	TTYDBG("\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b=")
	li	    t1, 0x1f

3:
    lw      a0, 0x44(t0)
	li	    a1, 0x20
	and	    a0, a0, a1

    beqz	a0,	1b
	nop

	TTYDBG("\r\n")
    lw      a0, 0x44(t0)
	bal	    hexserial
    nop
	TTYDBG("\r\n")
#endif

// xqch
#if 1	// below added to fix ls2gq2h HT connection fail bugs 
    dli     t0, 0x90000efdfb000000
    lw      a0, 0x44(t0)
	andi	a0, 0x100
	beqz	a0, check_b12
	nop
	b		reset_htbus
	nop
check_b12:
    lw      a0, 0x48(t0)
	andi	a0, 0x1000
	beqz	a0, config_over
	nop
	b		reset_htbus
	nop
config_over:
    //lw      a0, 0x3c(t0)
	b		fix2gq2h_over
	nop
reset_htbus:

#if 1
	TTYDBG("RESET HT bus\r\n")
	dli	    t2, 0x90000efdfb000000
    dli     t3, 0x00000000
    sw      t3, 0x3c(t2)
    dli     t3, 0x00400000
    sw      t3, 0x3c(t2)
#endif

#if 1//wait until HT link down
    TTYDBG("Waiting HyperTransport bus to be down.")
    dli     t0, 0x90000efdfb000000
	li	    t1, 0x1f
1:
    lw      a0, 0x44(t0)
	#bal	hexserial
    nop
	beqz	t1,2f
	TTYDBG(">")
	addi	t1, t1, -1
	b	    3f
    nop
2:
	TTYDBG("\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b=")
	li	    t1, 0x1f

3:
    lw      a0, 0x44(t0)
	li	    a1, 0x20
	and	    a0, a0, a1

    bnez	a0,	1b
	nop

	TTYDBG("\r\n")
    lw      a0, 0x44(t0)
	bal	    hexserial
    nop
	TTYDBG("\r\n")
#endif

	li		a0, 0xfffffeff
	lw		a1, 0x44(t0)
	and		a0, a0, a1
	sw		a0, 0x44(t0)

	li		a0, 0xffffefff
	lw		a1, 0x48(t0)
	and		a0, a0, a1
	sw		a0, 0x48(t0)

    dli     t3, 0x00000000
    sw      t3, 0x3c(t0)

#if 0 //RESET forever
    b       up_here
    nop
#endif
	b		wait_up
	nop

	
fix2gq2h_over:

#endif


/////////////////////
#if 1 //FIX 3A rx buffer problem
	TTYDBG("FIX 3A rx buffer problem\r\n")
	dli	    t2, 0x90000efdfe000100
    li      t3, 0x11111111
    sw      t3, 0x8(t2)
    li      t3, 0x0
    sw      t3, 0x8(t2)

    lw      a0, 0x0(t2)
    li      t3, 0x01090909
    //li      t3, 0x020a0a0a
    beq     a0, t3, data_buffer_check1
    nop

	TTYDBG("rx buffer ERROR found in cmd buffer!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\r\n")
    lw      a0, 0x0(t2)
    bal     hexserial
    nop

    dli     a0, 0x1000
1:
    daddi   a0, a0, -1
    bnez    a0, 1b
    nop

	dli	    t2, 0x90000efdfe000100
    lw      a0, 0x0(t2)
    nop

#if 1
//PC CMD
    li      t3, 0x00000009
    li      a1, 0x000000ff
    and     a2, a1, a0
    beq     a2, t3, 1f
    nop

    sub     a1, t3, a2
    bgtz    a1, 2f
    nop

    li      t3, 0x11111110
    sw      t3, 0x8(t2)
    li      t3, 0x0
    sw      t3, 0x8(t2)

    li      t3, 0x0000000a
    sub     a1, a2, t3
    li      t3, 0x30000000
    or      t3, t3, a1
    sw      t3, 0x8(t2)
    li      t3, 0x0
    sw      t3, 0x8(t2)

    b       1f
    nop

2:
    li      t3, 0x10000000
    or      t3, t3, a1
    sw      t3, 0x8(t2)
    li      t3, 0x0
    sw      t3, 0x8(t2)


#if 1
1:
//NPC CMD
    li      t3, 0x00000900
    li      a1, 0x0000ff00
    and     a2, a1, a0
    beq     a2, t3, 1f
    nop

    sub     a1, t3, a2
    bgtz    a1, 2f
    nop

    li      t3, 0x11111101
    sw      t3, 0x8(t2)
    li      t3, 0x0
    sw      t3, 0x8(t2)

    li      t3, 0x00000a00
    sub     a1, a2, t3
    li      t3, 0x30000000
    srl     a1, 4
    or      t3, t3, a1
    sw      t3, 0x8(t2)
    li      t3, 0x0
    sw      t3, 0x8(t2)

    b       1f
    nop

2:
    li      t3, 0x10000000
    srl     a1, 4
    or      t3, t3, a1
    sw      t3, 0x8(t2)
    li      t3, 0x0
    sw      t3, 0x8(t2)
#endif    

1:
//R CMD
    li      t3, 0x00090000
    li      a1, 0x00ff0000
    and     a2, a1, a0
    beq     a2, t3, 1f
    nop

    sub     a1, t3, a2
    bgtz    a1, 2f
    nop

    li      t3, 0x11111011
    sw      t3, 0x8(t2)
    li      t3, 0x0
    sw      t3, 0x8(t2)

    li      t3, 0x000a0000
    sub     a1, a2, t3
    li      t3, 0x30000000
    srl     a1, 8
    or      t3, t3, a1
    sw      t3, 0x8(t2)
    li      t3, 0x0
    sw      t3, 0x8(t2)

    b       1f
    nop

2:
    li      t3, 0x10000000
    srl     a1, 8
    or      t3, t3, a1
    sw      t3, 0x8(t2)
    li      t3, 0x0
    sw      t3, 0x8(t2)
    

1:
#else
    TTYDBG("\r\nERROR detected in cmd buffer\r\n")
#endif


data_buffer_check1:

    lw      a0, 0x4(t2)
    li      t3, 0x00080808
    //li      t3, 0x00090909
    beq     a0, t3, buffer_check_out1
    nop

#if 1
	TTYDBG("\r\nrx buffer ERROR found in data buffer!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\r\n")
    lw      a0, 0x4(t2)
    bal     hexserial
    nop
    lw      a0, 0x4(t2)

//PC DATA
    li      t3, 0x00000008
    li      a1, 0x000000ff
    and     a2, a1, a0
    beq     a2, t3, 1f
    nop

    sub     a1, t3, a2
    bgtz    a1, 2f
    nop

    li      t3, 0x11101111
    sw      t3, 0x8(t2)
    li      t3, 0x0
    sw      t3, 0x8(t2)

    li      t3, 0x00000009
    sub     a1, a2, t3
    li      t3, 0x30000000
    sll     a1, 16
    or      t3, t3, a1
    sw      t3, 0x8(t2)
    li      t3, 0x0
    sw      t3, 0x8(t2)

    b       1f
    nop

2:
    li      t3, 0x10000000
    sll     a1, 16
    or      t3, t3, a1
    sw      t3, 0x8(t2)
    li      t3, 0x0
    sw      t3, 0x8(t2)
    

#if 0 //Should NOT be turn on for BUGs
1:
//NPC DATA
    li      t3, 0x00000800
    li      a1, 0x0000ff00
    and     a2, a1, a0
    beq     a2, t3, 1f
    nop

    sub     a1, t3, a2
    bgtz    a1, 2f
    nop

    li      t3, 0x11011111
    sw      t3, 0x8(t2)
    li      t3, 0x0
    sw      t3, 0x8(t2)

    li      t3, 0x00000900
    sub     a1, a2, t3
    li      t3, 0x30000000
    sll     a1, 12
    or      t3, t3, a1
    sw      t3, 0x8(t2)
    li      t3, 0x0
    sw      t3, 0x8(t2)

    b       1f
    nop

2:
    li      t3, 0x10000000
    sll     a1, 12
    or      t3, t3, a1
    sw      t3, 0x8(t2)
    li      t3, 0x0
    sw      t3, 0x8(t2)
#endif

1:
//R DATA
    li      t3, 0x00080000
    li      a1, 0x00ff0000
    and     a2, a1, a0
    beq     a2, t3, 1f
    nop

    sub     a1, t3, a2
    bgtz    a1, 2f
    nop

    li      t3, 0x10111111
    sw      t3, 0x8(t2)
    li      t3, 0x0
    sw      t3, 0x8(t2)

    li      t3, 0x00090000
    sub     a1, a2, t3
    li      t3, 0x30000000
    sll     a1, 8
    or      t3, t3, a1
    sw      t3, 0x8(t2)
    li      t3, 0x0
    sw      t3, 0x8(t2)

    b       1f
    nop

2:
    li      t3, 0x10000000
    sll     a1, 8
    or      t3, t3, a1
    sw      t3, 0x8(t2)
    li      t3, 0x0
    sw      t3, 0x8(t2)
    

1:
#else
    TTYDBG("ERROR detected in data buffer\r\n")
#endif

buffer_check_out1:

	dli	    t2, 0x90000efdfe000100
    li      t3, 0x30000000
    sw      t3, 0x8(t2)
    li      t3, 0x0
    sw      t3, 0x8(t2)

#endif


/////////////////////
#if 1//Print 2H rx buffer number
	TTYDBG("\r\nPrint 2H HT tx buffer(which is sent by 3A)\r\n")
	dli	    t2, 0x90000efdfe000100
    lw      a0, 0x0(t2)
    bal     hexserial
    nop
	TTYDBG("\r\n")
    nop
    lw      a0, 0x4(t2)
    bal     hexserial
    nop
	TTYDBG("\r\n")
    nop

#endif

#if 1
	TTYDBG("Setting HyperTransport Southbridge back to be 8-bit width and 200Mhz for next RESET\r\n")
    dli	    t2, 0x90000efdfb000000
	li	    t0, 0x0 //Frequency: 200 Mhz
	sb	    t0, 0x49(t2)
    lw      a0, 0x48(t2)
	bal	    hexserial
    nop
	TTYDBG("\r\n")
    dli	    t2, 0x90000efdfe000000
	li	    t0, 0x0 //Frequency: 200 Mhz
	sb	    t0, 0x49(t2)
    lw      a0, 0x48(t2)
	bal	    hexserial
    nop
	TTYDBG("\r\n")
#endif
#endif

#if 1//Check if CRC error bit set and reset it
#define RESET_CRC
crc_checking:
    TTYDBG("Checking HyperTransport bus CRC error bit.\r\n")
    dli     t0, 0x90000efdfb000000

2:
    lw      a0, 0x44(t0)
	li	    a1, 0x300
	and	    a0, a0, a1

    beqz	a0,	1f
	nop

    lw      a0, 0x44(t0)
	bal	    hexserial
	nop
	TTYDBG("\r\nReset the Controller errror CRC bit\r\n")
 	nop

    lw      a0, 0x44(t0)
	li	    a1, 0xfffffcff
	and	    a0, a0, a1

	sw	    a0, 0x44(t0)
	nop

#ifdef RESET_CRC
	b	    2b
	nop
#endif

1:
    TTYDBG("Checking HyperTransport SouthBridge CRC error bit.\r\n")
	//li	t0, 0xba000000
    dli	    t0, 0x90000efdfe000000
2:
    lw      a0, 0x44(t0)
	li	    a1, 0x300
	and	    a0, a0, a1

    beqz	a0,	1f
	nop

    lw      a0, 0x44(t0)
	bal	    hexserial
	nop
	TTYDBG("\r\nReset the Bridge errror CRC bit\r\n")
 	nop

    lw      a0, 0x44(t0)
	li	    a1, 0xfffffcff
	and	    a0, a0, a1

	sw	    a0, 0x44(t0)
	nop

#ifdef RESET_CRC
	b	    2b
	nop
#endif


1:
	TTYDBG("Done\r\n")

#endif

#if 1//Set  HT channel byte write for LS2H 
	TTYDBG("Set HT channel byte write for LS2H\r\n")
	dli	    t2, 0x90000efdfe000000
	li	    t0, 0x4321
	sw	    t0, 0x50(t2)
	sync
    lw      a0, 0x50(t2)
	bal	    hexserial
    nop
#endif
#if 1//Set  HT channel in order for LS2H DMA
	TTYDBG("Set HT channel in order for LS2H DMA\r\n")
	dli	    t2, 0x90000efdfe000000
	lw	    t0, 0x50(t2)
	li		t1, 0x00010000
	or		t0, t0, t1
	sw		t0, 0x50(t2)
	sync
    lw      a0, 0x50(t2)
	bal	    hexserial
    nop
#endif

#define set_L2_win(mas_i, win_i, base, mask, mmap) \
	li	t0, 0xbbd80000 ;\
	daddiu	t0, t0, mas_i*0x100 ;\
	daddiu	t0, t0, win_i*0x8 ;\
	dli	t1, base ;\
	sd	t1, 0x0(t0) ;\
	dli	t1, mask ;\
	sd	t1, 0x40(t0) ;\
	dli	t1, mmap ;\
	sd	t1, 0x80(t0)

        ## disable window before changing
#define set_L1_mas6(win_i, base, mask, mmap) \
	li	t0, 0xbbd82600 ;\
	daddiu	t0, t0, win_i*0x8 ;\
	sd	$0, 0x80(t0) ;\
	dli	t1, base ;\
	sd	t1, 0x0(t0) ;\
	dli	t1, mask ;\
	sd	t1, 0x40(t0) ;\
	dli	t1, mmap ;\
	sd	t1, 0x80(t0)

#### IODMA 4G ####
	set_L2_win(1, 0, 0x0000000000000000, 0xffffffffc0000000, 0x00000010800000f3) # 0~1G
	set_L2_win(1, 1, 0x0000000040000000, 0xffffffffc0000000, 0x00000020800000f3) # 1~2G
	set_L2_win(1, 2, 0x0000000080000000, 0xffffffffc0000000, 0x00000030800000f3) # 2~3G
	set_L2_win(1, 3, 0x00000000c0000000, 0xffffffffc0000000, 0x00000040800000f3) # 3~4G
	set_L2_win(1, 4, 0xffffffff80000000, 0xffffffffc0000000, 0x00000030800000f3) # 2~3G 
	set_L2_win(1, 5, 0xffffffffc0000000, 0xffffffffc0000000, 0x00000040800000f3) # 3~4G

	#PCIE window
	set_L2_win(4, 0, 0x0000000000000000, 0xffffffffc0000000, 0x00000010800000f3) # 0~1G
	set_L2_win(4, 1, 0x0000000040000000, 0xffffffffc0000000, 0x00000020800000f3) # 1~2G
	set_L2_win(4, 2, 0x0000000080000000, 0xffffffffc0000000, 0x00000030800000f3) # 2~3G
	set_L2_win(4, 3, 0x00000000c0000000, 0xffffffffc0000000, 0x00000040800000f3) # 3~4G
	set_L2_win(4, 4, 0xffffffff80000000, 0xffffffffc0000000, 0x00000030800000f3) # 2~3G
	set_L2_win(4, 5, 0xffffffffc0000000, 0xffffffffc0000000, 0x00000040800000f3) # 3~4G

	set_L1_mas6(5, 0x0000000000000000, 0x0000000000000000, 0x00000000000000f0) # others, all to L2$
	set_L1_mas6(0, 0x0000001080000000, 0xfffffffff0000000, 0x00000000000000f6)
	set_L1_mas6(1, 0x0000001080000000, 0xffffffffc0000000, 0x00000001000000f6)
	set_L1_mas6(2, 0x0000002080000000, 0xffffffffc0000000, 0x00000001400000f6)
	set_L1_mas6(3, 0x0000003080000000, 0xffffffffc0000000, 0x00000001800000f6)
	set_L1_mas6(4, 0x0000004080000000, 0xffffffffc0000000, 0x00000001c00000f6)

	// set ls3a-2h dma cached coherence by hardware
//	li	t2, 0x90001EFDFB0000F0; li t1, 0x0; sw t1, 0x00(t2);

	
	// support 1000M phy
	li	t1, 0x800f0000; li t2, 0xbbd0020c; sw t1, 0x00(t2); 

    dli     t0, 0x90000efdfe000000
	li		a0, 0x01
	sw		a0, 0x30(t0)
