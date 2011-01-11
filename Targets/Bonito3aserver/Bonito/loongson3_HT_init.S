/*whd : loongson3_HT_init.S
        used to set up the HyperTransport interface
        and the South bridge
        mainly for MCP68
*/

######################################################
#define HT_32bit_TRANS
#define WITH_HT
#define HT_800M
//#define HT_16bit
#define HT_RECONNECT
//#define HT_REG_TRANS
######################################################

#ifdef HT_32bit_TRANS //PCI CFG : TYPE 0: 
	TTYDBG("32 bit PCI space translate to 64 bit HT space\r\n")

    dli t0, 0x900000003ff02000
    dli t2, 0x900000003ff02800

1:
#if 1
 //map HT: PCI IO : 0x90000efd_fc000000 --> 0x18000000
 //map              0x90000efd_fd000000 --> 0x19000000
 //map HT: PCI CFG: 0x90000efd_fe000000 --> 0x1a000000
 //map HT: PCI CFG: 0x90000efd_ff000000 --> 0x1b000000
    dli t1, 0x0000000018000000
    sd  t1, 0x0(t0)
    dli t1, 0xfffffffffc000000
    sd  t1, 0x40(t0)
    dli t1, 0x00000efdfc0000f7
    sd  t1, 0x80(t0)

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
    dli t1, 0x0000000010000000
    sd  t1, 0x18(t0)
    dli t1, 0xfffffffff8000000
    sd  t1, 0x58(t0)
    dli t1, 0x00000e00100000f7
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

    daddiu  t0, t0, 0x100
    bne     t0, t2, 1b
    nop

#else
/******************************************/
##################################### LEVEL-1 XBAR : CPU #########################################
###HT CFG : TYPE 0: 
###map 0x90000efd_fe000000 --> 0x1fe80000, size 0x00080000 
	dli t1, 0x000000001fe80000
	sd  t1, 0x0(t0)
	dli t1, 0xfffffffffff80000
	sd  t1, 0x40(t0)
	dli t1, 0x00000efdfe0000f7
	sd  t1, 0x80(t0)
###HT CFG : TYPE 1:
###map 0x90000efd_ff000000 --> 0x1e000000, size 0x01000000
	dli t1, 0x000000001e000000
	sd  t1, 0x8(t0)
	dli t1, 0xffffffffff000000
	sd  t1, 0x48(t0)
	dli t1, 0x00000efdff0000f7
	sd  t1, 0x88(t0)

###HT LOW MEM
###map 0x90000e00_00000000 --> 0x10000000, size 0x04000000
	dli t1, 0x0000000010000000
	sd  t1, 0x10(t0)
	dli t1, 0xfffffffffc000000
	sd  t1, 0x50(t0)
	dli t1, 0x00000e00000000f7
	sd  t1, 0x90(t0)

###HT HIGH MEM
###map 0x90000e00_10000000 --> 0x14000000, size 0x04000000
	dli t1, 0x0000000014000000
	sd  t1, 0x18(t0)
	dli t1, 0xfffffffffc000000
	sd  t1, 0x58(t0)
	dli t1, 0x00000e00140000f7
	sd  t1, 0x98(t0)

###HT IO 
###map 0x90000efd_fc000000 --> 0x1fd00000, size 0x01000000
	dli t1, 0x000000001fd00000
	sd  t1, 0x20(t0)
	dli t1, 0xfffffffffff00000
	sd  t1, 0x60(t0)
	dli t1, 0x00000efdfc0000f7
	sd  t1, 0xa0(t0)

###HT BRIDGE CFG
###map 0x90000efd_fb000000 --> 0x1b000000, size 0x01000000
	dli t1, 0x000000001b000000
	sd  t1, 0x28(t0)
	dli t1, 0xffffffffff000000
	sd  t1, 0x68(t0)
	dli t1, 0x00000efdfb0000f7
	sd  t1, 0xa8(t0)

### ANY ELSE DEFAULY MEMORY 
### Mapping 0x0 ---> 0x0, size 0x00001000_00000000

	daddiu	t0, t0, 0x100
	bne	t0, t2, 1b
	nop


/******************************************/
#endif
#endif



//////////////////
#if	0
	TTYDBG("DEBUG......\r\n")
	dli	t0, 0x900000003ff02000
	dli	t1, 0x900000003ff02400
1 :
	ld	a0, 0(t0)
    dsrl    a0, 32
	bal	hexserial
	nop
	ld	a0, 0(t0)
	bal	hexserial
	nop
	TTYDBG("\r\n")
	daddiu	t0, t0, 8
	nop
	bne	t0, t1, 1b
	nop
	TTYDBG("DEBUG END......\r\n")
#endif

#if 0//Print all HT registers
	TTYDBG("Print all HT registers\r\n")
	dli	    t2, 0x90000efdfb000000
    dli     t3, 0x90000efdfb000100
1:
    lw      a0, 0x00(t2)
	bal	    hexserial
    nop
	TTYDBG("\r\n")

    daddi   t2, t2, 0x4
    bne     t2, t3, 1b
    nop
#endif


#ifdef WITH_HT
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

#if	0
	TTYDBG("HT bus 0 scanning with deviceID and vendorID\r\n")

        dli	t0, 0x90000efdfe000000
//        li	t0, 0xbfe80000
	li	t3, 32
1 :
	li	t2, 8

2 :
//	lw	a0, 0x0(t0)
//	li	a1, 0xffffffff
//	beq	a0, a1, ht_next_id
//	nop

	TTYDBG("Device(")
	nop
	li	a0, 32
	sub	a0, a0, t3
	bal	hexserial
	nop
	TTYDBG(")---")
	nop
	TTYDBG("Function(")
	nop
	li	a0, 8
	sub	a0, a0, t2
	bal	hexserial
	nop
	TTYDBG(") : ")
	nop

	lw	a0, 0x0(t0)
	bal	hexserial
        nop
	TTYDBG("\t")
        nop

	TTYDBG("CAPABILITY POINTER : ")
	lw	a0, 0x34(t0)
	bal	hexserial
	nop
	TTYDBG("\r\n")

ht_next_id :
	daddi	t0, t0, 0x100
	addi	t2, t2, -1
	bnez	t2, 2b
	nop

	addi	t3, t3, -1
	bnez	t3, 1b
	nop
	
	TTYDBG("END HT bus 0 scan\r\n")
        nop
#endif


#if 0//Print all HT registers
	TTYDBG("Print all HT registers\r\n")
	dli	    t2, 0x90000efdfb000000
    dli     t3, 0x90000efdfb000100
1:
    lw      a0, 0x00(t2)
	bal	    hexserial
    nop
	TTYDBG("\r\n")

    daddi   t2, t2, 0x4
    bne     t2, t3, 1b
    nop
#endif

#if 0//Set HT channel priority
	TTYDBG("Set HT Channel priority\r\n")
	dli	    t2, 0x90000efdfb000000
	li	    t0, 0x4f04
	//li	t0, 0x4f14
	//li	t0, 0x4f44 #45S
	sh	    t0, 0x50(t2)
    lw      a0, 0x50(t2)
	bal	    hexserial
    nop
	TTYDBG("\r\n")
#endif

#if 0//Set HT Seq
	TTYDBG("Don't care about HT Channel seq\r\n")
	dli	    t2, 0x90000efdfb000000
	lw	    t0, 0x50(t2)
    li      a0, 0x00200000
    or      t0, t0, a0
	sw	    t0, 0x50(t2)
    lw      a0, 0x50(t2)
	bal	    hexserial
    nop
	TTYDBG("\r\n")
#endif

#if 0//Set CPU2HT access with seq
	TTYDBG("Set CPU2HT Channel with seq\r\n")
	dli	    t2, 0x90000efdfb000000
	lw	    t0, 0x50(t2)
    li      a0, 0x00110000
    or      t0, t0, a0
	sw	    t0, 0x50(t2)
    lw      a0, 0x50(t2)
	bal	    hexserial
    nop
	TTYDBG("\r\n")
#endif

#if 1//OPEN RX SPACE in HOST
	TTYDBG("HT RX DMA address ENABLE\r\n")
	dli	    t2, 0x90000efdfb000060
	li	    t0, 0xc0000000
	sw	    t0, 0x0(t2)
	li	    t0, 0x0080fff0
	sw	    t0, 0x4(t2)
	TTYDBG("HT RX DMA address ENABLE done 1\r\n")


	li	t0, 0xc0000000
	sw	t0, 0x8(t2)
	li	t0, 0x00008000
	sw	t0, 0xc(t2)
	TTYDBG("HT RX DMA address ENABLE done 2\r\n")

	//li	t0, 0xc0000000
	//sw	t0, 0x10(t2)
	//li	t0, 0xfdfcfffc
	//sw	t0, 0x14(t2)
	////TTYDBG("HT RX DMA address ENABLE done 3\r\n")

#endif

#if 0//Set Mem space post
	TTYDBG("Set HT Memory space all post\r\n")
	dli	    t2, 0x90000efdfb000000
	li	    t0, 0x80000010
	sw	    t0, 0xd0(t2)
	li	    t0, 0x0010fff8
	sw	    t0, 0xd4(t2)

#endif

//lycheng
#if 0
        TTYDBG("-------------------HT HOST mode enable\r\n")
        li      t2, 0xbb000040
        lw      a0, 0x0(t2)
        bal     hexserial
        nop

        li      t1, 0xfbffffff
        and     a0, a0, t1
        sw      a0, 0x0(t2)

        lw      a0, 0x0(t2)
        bal     hexserial
        nop
        TTYDBG("\r\n")
        TTYDBG("-------------------HT HOST mode enable done\r\n")
#endif

#ifdef HT_16bit//Set HT bridge to be 16-bit width
	TTYDBG("Setting HyperTransport Controller to be 16-bit width\r\n")
	dli	    t2, 0x90000efdfb000000
	#li	    t0, 0x10  //RECEIVER 16bit
	li	    t0, 0x11
	sb	    t0, 0x47(t2)
    lw      a0, 0x44(t2)
	bal	    hexserial
    nop
	TTYDBG("\r\n")
#else
	TTYDBG("Setting HyperTransport Controller to be 8-bit width\r\n")
	dli	    t2, 0x90000efdfb000000
	li	    t0, 0x00
	sb	    t0, 0x47(t2)
    lw      a0, 0x44(t2)
	bal	    hexserial
    nop
	TTYDBG("\r\n")
#endif

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

#endif


#ifdef WITH_HT

#if 0//SET RX SPACE UNCACHE
	TTYDBG("HT RX DMA SET to UNCACHE\r\n")
	dli	    t2, 0x90000efdfb0000f0
	li	    t0, 0xc0000000
	sw	    t0, 0x0(t2)
	li	    t0, 0x0080ff80
	sw	    t0, 0x4(t2)

	//li	t0, 0x80000000
	//sw	t0, 0x8(t2)
	//li	t0, 0x0000ffc0
	//sw	t0, 0xc(t2)
#endif

#if 1 //SET HT as HOST
	TTYDBG("SET HT as HOST\r\n")
	dli	    t2, 0x90000efdfb000040
	lw	    a0, 0x0(t2)

	bal	    hexserial
    nop

	li	    t1, 0xfbffffff
	and	    a0, a0, t1
	sw	    a0, 0x0(t2)
	lw	    a0, 0x0(t2)

	bal	    hexserial
    nop
	TTYDBG("\r\n")
#endif



#if 0
    li	t1, 31
	TTYDBG("start HT bus scan\r\n")

    //li	t0, 0xba000000
    dli	    t0, 0x90000efdfe000000
1:
	move	a0, t0
	bal	    hexserial
	nop
	TTYDBG("  :  ")

	lw	    a0, 0x0(t0)
	bal	    hexserial
    nop
	TTYDBG("\r\n")
    nop

	//add	t0, t0, 0x800
	dadd	t0, t0, 0x800

	bnez	t1, 1b
	sub	    t1, t1, 1
       
	TTYDBG("END HT bus scan\r\n")
    nop
#endif

#if 0 //Find capability in DEVICE 0
	TTYDBG("Find capability in DEVICE 1\r\n")
	TTYDBG("function 0\r\n")

	//li	t0, 0xba000000
    dli	    t0, 0x90000efdfe000000

	lb	    t1, 0x34(t0)
	
1:

	TTYDBG("Address of Capability : ")

	move	a0, t1
	bal	    hexserial
	nop
	TTYDBG("\r\n")

	TTYDBG("First Line of Capability : ")
	dadd	t1, t1, t0
	lw	    a0, 0(t1)

	bal	    hexserial
    nop
	TTYDBG("\r\n")

	#TTYDBG("Second Line of Capability : ")
	#add	t1, t1, t0
	#lw	    a0, 4(t1)

	#bal	hexserial
    #nop
	#TTYDBG("\r\n")

	lb	    t1, 0x1(t1)
	andi	t1, t1, 0xff

	bnez	t1, 1b	
    nop

/*
	TTYDBG("function 1\r\n")

	//li	t0, 0xba000900
    dli	    t0, 0x90000efdfe000900
	lb	    t1, 0x34(t0)
1:

	TTYDBG("Address of Capability : ")

	move	a0, t1
	bal	    hexserial
	nop
	TTYDBG("\r\n")

	TTYDBG("First Line of Capability : ")
	dadd	t1, t1, t0
	lw	    a0, 0(t1)

	bal	    hexserial
    nop
	TTYDBG("\r\n")

	lb	    t1, 0x1(t1)
	bnez	t1, 1b	
    nop

	TTYDBG("function 2\r\n")

	//li	t0, 0xba000a00
    dli	    t0, 0x90000efdfe000a00

	lb	    t1, 0x34(t0)
	
1:

	TTYDBG("Address of Capability : ")

	move	a0, t1
	bal	    hexserial
	nop
	TTYDBG("\r\n")

	TTYDBG("First Line of Capability : ")
	dadd	t1, t1, t0
	lw	    a0, 0(t1)

	bal	    hexserial
    nop
	TTYDBG("\r\n")

	lb	    t1, 0x1(t1)
	bnez	t1, 1b	
    nop

	TTYDBG("function 3\r\n")

	//li	t0, 0xba000b00
    dli	    t0, 0x90000efdfe000b00

	lb	    t1, 0x34(t0)
	
1:

	TTYDBG("Address of Capability : ")

	move	a0, t1
	bal	    hexserial
	nop
	TTYDBG("\r\n")

	TTYDBG("First Line of Capability : ")
	add	    t1, t1, t0
	lw	    a0, 0(t1)

	bal	    hexserial
    nop
	TTYDBG("\r\n")

	lb	    t1, 0x1(t1)

	bnez	t1, 1b	
    nop
*/



	TTYDBG("End of Capability\r\n")
	
	
    li	    t1, 6
	TTYDBG("start Capability of HT scan\r\n")

        //li	t0, 0xba000044
    dli	    t0, 0x90000efdfe000044
1:
	move	a0, t0
	bal	    hexserial
	nop
	TTYDBG("  :  ")

	lw	    a0, 0x0(t0)
	bal	    hexserial
    nop
	TTYDBG("\r\n")
    nop


	dadd	t0, t0, 0x4

	bnez	t1, 1b
	sub	    t1, t1, 1
       
	TTYDBG("END HT bus scan\r\n")
    nop
#endif


#if 0//RESET Southbridge
	dli	    t0, 0x90000efdfc000cf9
	//li	    t0, 0xb8000cf9
	lb	    a0, 0x0(t0)
	li	    a0, 0x4
	sb	    a0, 0x0(t0)

	TTYDBG("RESETing HyperTransport\r\n")
	nop
#endif

#ifdef HT_RECONNECT
#ifdef HT_16bit //Write Southbridge to 16 bit width
	TTYDBG("Setting HyperTransport Southbridge to be 16-bit width\r\n")
	//li	t0, 0xba000000
    dli	    t0, 0x90000efdfe000000
	#li	    t1, 0x01 //DRIVER 16bit
	li	    t1, 0x11
	sb	    t1, 0xcb(t0)
    lw      a0, 0xc8(t0)
	bal	    hexserial
    nop
	TTYDBG("\r\n")
#else
	TTYDBG("Setting HyperTransport Southbridge to be 8-bit width\r\n")
	//li	t0, 0xba000000
    dli	    t0, 0x90000efdfe000000
	li	    t1, 0x00
	sb	    t1, 0xcb(t0)
    lw      a0, 0xc8(t0)
	bal	    hexserial
    nop
	TTYDBG("\r\n")

#endif

#ifdef HT_800M //Write Southbridge to 800Mhz
	TTYDBG("Setting HyperTransport Southbridge to be 800M\r\n")
	//li	t0, 0xba000000
    dli	    t0, 0x90000efdfe000000
	#li	    t1, 0x2 //Frequency : 400Mhz
	li	    t1, 0x5 //Frequency : 800Mhz
	sb	    t1, 0xd1(t0)
    lw      a0, 0xd0(t0)
	bal	    hexserial
    nop
	TTYDBG("\r\n")
#endif
#if 1 //Watch dog Trying
	TTYDBG("Setting Watch Dog to make a WARM RESET\r\n")
    li      t1, 10
//#define WD_DEBUG
	TTYDBG("Watch dog Enable\r\n")
    dli	    t0, 0x90000efdfc000cd6
	li	    a0, 0x00000069
    sync
	sb	    a0, 0(t0)
    sync
	li	    a0, 0x00000000
	sb	    a0, 1(t0)
    sync
	lb	    a0, 1(t0)
    sync
	//bal	    hexserial
    nop
	TTYDBG("\r\n")
	nop
    dli	    t0, 0x90000efdfc000cd6
	li	    a0, 0x0000006c
	sb	    a0, 0(t0)
	li	    a0, 0x00000000
	sb	    a0, 1(t0)
	nop
	li	    a0, 0x0000006d
	sb	    a0, 0(t0)
	li	    a0, 0x00000000
	sb	    a0, 1(t0)
	nop
	li	    a0, 0x0000006e
	sb	    a0, 0(t0)
	li	    a0, 0x00000001
	sb	    a0, 1(t0)
	nop
	li	    a0, 0x0000006f
	sb	    a0, 0(t0)
	li	    a0, 0x00000000
	sb	    a0, 1(t0)
	nop
#ifdef WD_DEBUG
    lb      a0, 1(t0)
	bal	    hexserial
    nop
	TTYDBG("\r\n")
#endif

#ifdef WD_DEBUG
	lb	    a0, 1(t0)
	bal	    hexserial
	nop	
	TTYDBG("\r\n")

	TTYDBG("Watch dog base value\r\n")
	li	    a0, 0x00000069
	sb	    a0, 0(t0)
    lb      a0, 1(t0)
	bal	    hexserial
	nop	
	TTYDBG("\r\n")
	li	    a0, 0x0000006c
	sb	    a0, 0(t0)
    lb      a0, 1(t0)
	bal	    hexserial
	nop	
	TTYDBG("\r\n")
	li	    a0, 0x0000006d
	sb	    a0, 0(t0)
    lb      a0, 1(t0)
	bal	    hexserial
	nop	
	TTYDBG("\r\n")
	li	    a0, 0x0000006e
	sb	    a0, 0(t0)
    lb      a0, 1(t0)
	bal	    hexserial
	nop	
	TTYDBG("\r\n")
	li	    a0, 0x0000006f
	sb	    a0, 0(t0)
    lb      a0, 1(t0)
	bal	    hexserial
	nop	
	TTYDBG("\r\n")
#endif
	TTYDBG("Watch dog decode enable\r\n")
    dli	    t0, 0x90000efdfe00a041
    li      a0, 0xff
    sb      a0, 0(t0)
    lb      a0, 0(t0)
	bal	    hexserial
	nop	
	TTYDBG("\r\n")

	TTYDBG("Watch dog control value\r\n")
    dli	    t0, 0x90000e0000010000
    sync
	lw	    a0, 0(t0)
	bal	    hexserial
	nop	
	TTYDBG("\r\n")
    sync
	lw	    a0, 4(t0)
	bal	    hexserial
	nop	
	TTYDBG("\r\n")
	TTYDBG("Set Watch dog control value\r\n")
    li      a0, 0x15
	sw	    a0, 4(t0)
	nop	
    li      a0, 0x01
	sw	    a0, 0(t0)
	nop	
    li      a0, 0x81
	sw	    a0, 0(t0)
	nop	

#if 0
1:
    lw      a0, 4(t0)
	bal	    hexserial
	nop	
	TTYDBG("\r\n")
    lb      a0, 4(t0)
	bne	    a0, t1, 1b
	nop
#endif

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

#if 1
	TTYDBG("Setting HyperTransport Southbridge back to be 8-bit width and 200Mhz for next RESET\r\n")
	//li	t0, 0xba000000
    dli	    t0, 0x90000efdfe000000
	li	    t1, 0x00
	sb	    t1, 0xd1(t0)
    lw      a0, 0xd0(t0)
	bal	    hexserial
    nop
	TTYDBG("\r\n")
	//li	t1, 0x0
	//sb	t1, 0x4d(t0)
    //lw    a0, 0x4c(t0)
	//bal	hexserial
    //nop
	//TTYDBG("\r\n")
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
    lw      a0, 0xc8(t0)
	li	    a1, 0x300
	and	    a0, a0, a1

    beqz	a0,	1f
	nop

    lw      a0, 0xc8(t0)
	bal	    hexserial
	nop
	TTYDBG("\r\nReset the Bridge errror CRC bit\r\n")
 	nop

    lw      a0, 0xc8(t0)
	li	    a1, 0xfffffcff
	and	    a0, a0, a1

	sw	    a0, 0x48(t0)
	nop

#ifdef RESET_CRC
	b	    2b
	nop
#endif


1:
	TTYDBG("Done\r\n")

    //b       crc_checking
    nop
#endif

#if 1//Read HT channel priority
	TTYDBG("Read HT Channel priority\r\n")
	dli	    t2, 0x90000efdfb000000
    lw      a0, 0x50(t2)
	bal	    hexserial
    nop
	TTYDBG("\r\n")
#endif


#if 0//LPC header scan
    li	t1, 16
	TTYDBG("start HT-LPC header scan\r\n")

    //li	t0, 0xba000800
    dli	    t0, 0x90000efdfe000800
1:
	move	a0, t0
	bal	    hexserial
	nop
	TTYDBG("  :  ")

	lw	    a0, 0x0(t0)
	bal	    hexserial
    nop
	TTYDBG("\r\n")
    nop

	dadd	t0, t0, 0x4

	bnez	t1, 1b
	sub	    t1, t1, 1
       
	TTYDBG("END HT-LPC header scan\r\n")
    nop

    li	    t1, 6
    //li	t0, 0xba000810
    dli	    t0, 0x90000efdfe000810
1:
	li	    a0, 0xffffffff

	sw	    a0, 0x0(t0)
    nop

	dadd	t0, t0, 0x4

	bnez	t1, 1b
	sub	    t1, t1, 1
       

    li	    t1, 16
	TTYDBG("start HT-LPC header scan\r\n")

    //li	t0, 0xba000800
    dli	    t0, 0x90000efdfe000800
1:
	move	a0, t0
	bal	    hexserial
	nop
	TTYDBG("  :  ")

	lw	    a0, 0x0(t0)
	bal	    hexserial
    nop
	TTYDBG("\r\n")
    nop

	dadd	t0, t0, 0x4

	bnez	t1, 1b
	sub	    t1, t1, 1
       
	TTYDBG("END HT-LPC header scan\r\n")
    nop


#endif

#if 0//Enable the LPC on the Southbridge, including SuperIO, LED....
	TTYDBG("Check the LPC on the Southbridge\r\n")
	//li	t0, 0xba000800
    dli	    t0, 0x90000efdfe000800
	lw	    a0, 0x78(t0)
	bal	    hexserial
    nop
	TTYDBG("\r\n")

	li	    a0, 0x19
	sb	    a0, 0x7b(t0)
	lw	    a0, 0x78(t0)
	bal	    hexserial
    nop
	TTYDBG("\r\n")

	lw	    a0, 0xa4(t0)
	bal	    hexserial
    nop
	TTYDBG("\r\n")

	li	    a1, 0x30000
	sw	    a1, 0xa4(t0)

	nop
	nop
	nop

	lw	    a0, 0xa4(t0)
	bal	    hexserial
    nop
	TTYDBG("\r\n")

    li      a0,0xb8000080
    li      t0,0x0c
    sb      t0,0x0(a0)
    sb      t0,0x4(a0)

    //li      a0,0xb0000080
    //li      t0,0x0c
    //sb      t0,0x0(a0)
    //sb      t0,0x4(a0)

    //li      t0,0xb8000080
    //li      a0,0x0c
    //sb      a0,0x0(t0)

	//lb	a0, 0x00(t0)
	//bal	hexserial
    //nop
	//TTYDBG("\r\n")

//OUTPUT by COM1
    //li	t0, 0xba000800
    dli	    t0, 0x90000efdfe000800
	lw	    a0, 0xa0(t0)
	bal	    hexserial
    nop
	TTYDBG("\r\n")

	li	    a0, 0xffffffff
	sw	    a0, 0xa0(t0)
	nop
	lw	    a0, 0xa0(t0)
	bal	    hexserial
    nop
	TTYDBG("\r\n")

//UART A
	li	    a0, 0x03ff03f8
	sw	    a0, 0xa8(t0)
	nop
	lw	    a0, 0xa8(t0)
	bal	    hexserial
    nop
	TTYDBG("\r\n")

//PORT 80 for DEBUG LED
	li	    a0, 0x00840080
	sw	    a0, 0xac(t0)
	nop
	lw	    a0, 0xac(t0)
	bal	    hexserial
    nop
	TTYDBG("\r\n")

#if 1
//PORT 60 for KeyBoard
	li	    a0, 0x007f0060
	sw	    a0, 0xb0(t0)
	nop
	lw	    a0, 0xb0(t0)
	bal	    hexserial
    nop
	TTYDBG("\r\n")
#endif

//set Southbridge Super IO to be 24Mhz input
	TTYDBG("Setting Southbridge Super IO to be 24Mhz input\r\n")
	
	li	    a0, 0xffff0000
	sw	    a0, 0xb4(t0)
	nop
	lw	    a0, 0xb4(t0)
	bal	    hexserial
    nop
	TTYDBG("\r\n")

//Start Super IO configuration KEYWORDS
    li      t0,0xb8000000
	li	    a0, 0x87
	sb	    a0, 0x2e(t0)
	nop
	sb	    a0, 0x2e(t0)
	nop
	li	    a0, 0x24
	sb	    a0, 0x2e(t0)
	nop
	
	lb	    a0, 0x2f(t0)
	bal	    hexserial
    nop
	TTYDBG("\r\n")

	lb	    a0, 0x2f(t0)
	andi	a0, a0, 0xbf
	sb	    a0, 0x2f(t0)
	nop
	lb	    a0, 0x2f(t0)
	bal	    hexserial
    nop
	TTYDBG("\r\n")

	li	    a0, 0xaa
	sb	    a0, 0x2e(t0)

//Write Space 4 of LPC to NULL
    //li	t0, 0xba000800
    dli	    t0, 0x90000efdfe000800
	li	    a0, 0x0000
	sw	    a0, 0xb4(t0)
	nop
	lw	    a0, 0xb4(t0)
	bal	    hexserial
    nop
	TTYDBG("\r\n")
#####endhere

	bal	    initserial_COM1
	nop
	TTYDBG("Check out output of Southbridge UART port\r\n")

1:
	TTYDBG_COM1("Loongson 3 PMON output from Southbridge\r\n")
	nop

    //li	t0, 0xba000800
    dli	    t0, 0x90000efdfe000800
	lw	    a0, 0x78(t0)
	bal	    hexserial_COM1
    nop
	TTYDBG_COM1("\r\n")

	lw	    a0, 0xa4(t0)
	bal	    hexserial_COM1
    nop
	TTYDBG_COM1("\r\n")

	lw	    a0, 0xa0(t0)
	bal	    hexserial
    nop
	TTYDBG("\r\n")

    li      t0,0xb8000080
    li      a0,0x0E
    sb      a0,0x0(t0)
    sb      a0,0x4(t0)

#endif

#if 0 //DISABLE all other devices
	TTYDBG("DISABLE all other HT device\r\n")
	//li	t0, 0xba000900
    dli	    t0, 0x90000efdfe000900
	lw	    a0, 0xe4(t0)
	bal	    hexserial
	nop
	
	TTYDBG("\r\n")
    nop
	
	li	    t1, 0x7f7
	or	    a0, a0, t1
	sw	    a0, 0xe4(t0)
	
	lw	    a0, 0xe4(t0)
	bal	    hexserial
	nop
	
	TTYDBG("\r\n")
    nop
	
	lw	    a0, 0xe8(t0)
	bal	    hexserial
	nop
	
	TTYDBG("\r\n")
    nop
	
	//li	t1, 0x53fd27
	li	    t1, 0x537d23//DMA,P2P enable
	or	    a0, a0, t1
	sw	    a0, 0xe8(t0)
	
	lw	    a0, 0xe8(t0)
	bal	    hexserial
	nop
	
	TTYDBG("\r\n")
    nop

	//li	t0, 0xba000800
    dli	    t0, 0x90000efdfe000800
	lw	    a0, 0xa4(t0)
	bal	    hexserial
	nop
	
	TTYDBG("\r\n")
    nop

	li	    t1, 0x10000
	or	    a0, a0, t1
	sw	    a0, 0xa4(t0)
	
	lw	    a0, 0xa4(t0)
	bal	    hexserial
	nop
	
	TTYDBG("\r\nD1,F0, 0x88\r\n")
    nop
	
	//li	t0, 0xba000800
    dli	    t0, 0x90000efdfe000800
	lw	    a0, 0x88(t0)
	bal	    hexserial
	nop

	TTYDBG("\r\nD1,F0, 0x78\r\n")

	lw	    a0, 0x78(t0)
	bal	    hexserial
	nop
	
	TTYDBG("\r\nD1,F1, 0x60\r\n")

	//li	t0, 0xba000900
    dli	    t0, 0x90000efdfe000900
	lw	    a0, 0x60(t0)
	bal	    hexserial
	nop

	TTYDBG("\r\nD1,F1, 0x64\r\n")

	lw	    a0, 0x64(t0)
	bal	    hexserial
	nop

#endif

#if 0//read HT-PCI bridge header

    li	    t1, 16
	TTYDBG("start HT-PCI header scan\r\n")

    //li	t0, 0xba004000
    dli	    t0, 0x90000efdfe004000
1:
	move	a0, t0
	bal	    hexserial
	nop
	TTYDBG("  :  ")

	lw	    a0, 0x0(t0)
	bal	    hexserial
    nop
	TTYDBG("\r\n")
    nop

	dadd	t0, t0, 0x4

	bnez	t1, 1b
	sub	t1, t1, 1
       
	TTYDBG("END HT-PCI header scan\r\n")
    nop
#endif

#endif
