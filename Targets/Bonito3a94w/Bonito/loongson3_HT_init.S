/*whd : loongson3_HT_init.S
        used to set up the HyperTransport interface
        and the South bridge
        mainly for MCP68
*/

######################################################
#define HT_32bit_TRANS
#define WITH_HT
//#define HT3_0
//#define HT_2400M
//#define HT_2000M
//#define HT_1800M
//#define HT_1600M
//#define HT_1200M
//#define HT_1000M
#define HT_800M
//#define HT_200M
//#define HT_16bit
#define HT_RECONNECT
//#define HT_REG_TRANS
#define HTMEM_ACC
#define DMA_64BIT
#define HT_SOFT_PLL
######################################################

#ifdef HT_32bit_TRANS //PCI CFG : TYPE 0: 
	TTYDBG("32 bit PCI space translate to 64 bit HT space\r\n")

    dli t0, 0x900000003ff02000
    dli t2, 0x900000003ff02800

1:
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
/* has been set in fixup.S
    dli t1, 0x00000e0000000000
    sd  t1, 0x8(t0)
    dli t1, 0xffffff0000000000
    sd  t1, 0x48(t0)
    dli t1, 0x00000e00000000f7
    sd  t1, 0x88(t0)
*/
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

    daddiu  t0, t0, 0x100
    bne     t0, t2, 1b
    nop



#endif


#if 0 //NOT use for 3A2000, use HT RX windows
 // for dma up to 128GB,extract bit37~38 move it to bit44~bit45
 // and route int Node1 by HT0
	dli	t0,	0x900000003ff02700
	dli	t1,	0x0000002000000000
	sd	t1,	0x00(t0)
	dli	t1,	0xffffffe000000000
	sd	t1,	0x40(t0)
	dli	t1,	0x00001000000000a6
	sd	t1,	0x80(t0)
#endif

//#define DEBUG_HTBUS
#ifdef DEBUG_HTBUS
	dli	t2, 0x90000efdfb000000
	li	a0, 0x00000001
	sw	a0, 0x54(t2)
	sync
	//li	a0, 0x00000000
	//sw	a0, 0x54(t2)
#endif
	

#if 0//Print all HT registers
	TTYDBG("Print all HT registers\r\n")
	dli	t2, 0x90000efdfb000000
    	dli     t3, 0x90000efdfb000180
1:
    	lw      a0, 0x00(t2)
	bal	    hexserial
    	nop
	TTYDBG("\r\n")

    	daddi   t2, t2, 0x4
    	bne     t2, t3, 1b
    	nop
#endif


#if 1//wait until HT link up
    TTYDBG("Waiting HyperTransport bus to be up.")
	dli	t0, 0x90000efdfb000000
	li	t1, 0x1f
1:
	lw	a0, 0x44(t0)
	#bal	hexserial
	nop
	beqz	t1,2f
	nop
	TTYDBG(">")
	addi	t1, t1, -1
	b	3f
	nop
2:
	TTYDBG("\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b=")
	li	t1, 0x1f

3:
	lw      a0, 0x44(t0)
	li	a1, 0x20
	and	a0, a0, a1

	beqz	a0,	1b
	nop

	TTYDBG("\r\n")
	lw      a0, 0x44(t0)
	bal	hexserial
	nop
	TTYDBG("\r\n")
#endif

#if	0
	TTYDBG("HT bus 0 scanning with deviceID and vendorID\r\n")

        dli	t0, 0x90000efdfe000000
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
	dli	t2, 0x90000efdfb000060
	li	t0, 0xc0000000
	sw	t0, 0x0(t2)
	li	t0, 0x0080fff0
	sw	t0, 0x4(t2)
	TTYDBG("HT RX DMA address ENABLE done 1, 0x8000_0000\r\n")


#ifdef DMA_64BIT
	li	t0, 0xc0000000
	sw	t0, 0x8(t2)
	li	t0, 0x0000e000
	sw	t0, 0xc(t2)
	TTYDBG("HT RX DMA address ENABLE done 2, DMA route to NODE 0\r\n")

	li	t0, 0xc0100000
	sw	t0, 0x10(t2)
	li	t0, 0x2000e000
	sw	t0, 0x14(t2)
	TTYDBG("HT RX DMA address ENABLE done 3, DMA route to NODE 1\r\n")

//0x148
	li	t0, 0xc0200000
	sw	t0, 0xe8(t2)
	li	t0, 0x4000e000
	sw	t0, 0xec(t2)
	TTYDBG("HT RX DMA address ENABLE done 4, DMA route to NODE 2\r\n")

//0x150
	li	t0, 0xc0300000
	sw	t0, 0xf0(t2)
	li	t0, 0x6000e000
	sw	t0, 0xf4(t2)
	TTYDBG("HT RX DMA address ENABLE done 5, DMA route to NODE 3\r\n")

#else
	li	t0, 0xc0000000
	sw	t0, 0x8(t2)
	li	t0, 0x00008000
	sw	t0, 0xc(t2)
	TTYDBG("HT RX DMA address ENABLE done 2\r\n")

#endif
#endif

#ifdef HTMEM_ACC//Set Mem space post
	dli	t2, 0x90000efdfb000000
	TTYDBG("Set HT Memory space all post\r\n")
	li	t0, 0x80000000
	sw	t0, 0xd0(t2)
	li	t0, 0x0000ff00
	sw	t0, 0xd4(t2)

1:
#endif


#ifdef HT_RECONNECT
#ifdef HT_16bit//Set HT bridge to be 16-bit width
	TTYDBG("Setting HyperTransport Controller to be 16-bit width\r\n")
	dli	t2, 0x90000efdfb000000
	#li	t0, 0x10  //RECEIVER 16bit
	li	t0, 0x11
	sb	t0, 0x47(t2)
	lw      a0, 0x44(t2)
	bal	hexserial
	nop
	TTYDBG("\r\n")
#else
	TTYDBG("Setting HyperTransport Controller to be 8-bit width\r\n")
	dli	t2, 0x90000efdfb000000
	li	t0, 0x00
	sb	t0, 0x47(t2)
	lw      a0, 0x44(t2)
	bal	hexserial
	nop
	TTYDBG("\r\n")
#endif

#ifdef HT_SOFT_PLL

#ifdef HT_2400M
#define PHY_LO_DIV 2
#define PHY_HI_DIV 2
#define DIV_REFC   1
#define DIV_LOOPC  48
#define CORE_DIV   6
#else
#ifdef HT_2000M
#define PHY_LO_DIV 2
#define PHY_HI_DIV 2
#define DIV_REFC   1
#define DIV_LOOPC  40
#define CORE_DIV   10
#else
#ifdef HT_1800M
#define PHY_LO_DIV 2
//#define PHY_HI_DIV 2
#define DIV_REFC   1
#define DIV_LOOPC  36
#define CORE_DIV   8
#else
#ifdef HT_1600M
#define PHY_LO_DIV 4
//#define PHY_HI_DIV 4
#define DIV_REFC   1
#define DIV_LOOPC  64
#define CORE_DIV   8
#else
#ifdef HT_1200M
#define PHY_LO_DIV 4
//#define PHY_HI_DIV 4
#define DIV_REFC   1
#define DIV_LOOPC  48
#define CORE_DIV   12
#else
#ifdef HT_1000M
#define PHY_LO_DIV 4
//#define PHY_HI_DIV 4
#define DIV_REFC   1
#define DIV_LOOPC  40
#define CORE_DIV   10
#else
#ifdef HT_800M
#define PHY_LO_DIV 4
//#define PHY_HI_DIV 4
#define DIV_REFC   1
#define DIV_LOOPC  64
#define CORE_DIV   8
#else
#define PHY_LO_DIV 8
//#define PHY_HI_DIV 8
#define DIV_REFC   1
#define DIV_LOOPC  32
#define CORE_DIV   8
#endif
#endif
#endif
#endif
#endif
#endif
#endif

        TTYDBG("Setting CPU0 HyperTransport Controller to be soft config\r\n")
        dli     t2, 0x90000efdfb000000
        li      t0, (PHY_LO_DIV << 22) | (HT1_X_PHY_HI_DIV << 18) | (DIV_REFC << 16) | (DIV_LOOPC << 9) | (CORE_DIV << 5) | 0x3
        sw      t0, 0x178(t2)
        lw      a0, 0x178(t2)
        bal     hexserial
        nop
        TTYDBG("\r\n")
#endif


#ifdef HT_2400M
	TTYDBG("Setting HyperTransport Controller to be 2400Mhz\r\n")
	dli	t2, 0x90000efdfb000000
	li	t0, 0xd
#else
#ifdef HT_2000M
	TTYDBG("Setting HyperTransport Controller to be 2000Mhz\r\n")
	dli	t2, 0x90000efdfb000000
	li	t0, 0xb
#else
#ifdef HT_1800M
	TTYDBG("Setting HyperTransport Controller to be 1800Mhz\r\n")
	dli	t2, 0x90000efdfb000000
	li	t0, 0xa
#else
#ifdef HT_1600M//Set HT bridge to be 1600Mhz
	TTYDBG("Setting HyperTransport Controller to be 1600Mhz\r\n")
	dli	t2, 0x90000efdfb000000
	li	t0, 0x9
#else
#ifdef HT_1200M
	TTYDBG("Setting HyperTransport Controller to be 1200Mhz\r\n")
	dli	t2, 0x90000efdfb000000
	li	t0, 0x7
#else
#ifdef HT_1000M
	TTYDBG("Setting HyperTransport Controller to be 1000Mhz\r\n")
	dli	t2, 0x90000efdfb000000
	li	t0, 0x6
#else
#ifdef HT_800M//Set HT bridge to be 800Mhz
	TTYDBG("Setting HyperTransport Controller to be 800Mhz\r\n")
	dli	t2, 0x90000efdfb000000
	li	t0, 0x5
#endif
#endif
#endif
#endif
#endif
#endif
#endif
	sb	t0, 0x49(t2)
	lw      a0, 0x48(t2)
	bal	    hexserial
	nop
	TTYDBG("\r\n")

#ifdef HT3_0
	TTYDBG("Setting HyperTransport Controller to be GEN3 mode\r\n")
	dli	t2, 0x90000efdfb000000
	li	t0, 0x88600000
	sw	t0, 0x110(t2)
	lw      a0, 0x110(t2)
	bal	    hexserial
	nop
	TTYDBG("\r\n")

	TTYDBG("Setting HyperTransport Controller to be retry mode\r\n")
	dli	t2, 0x90000efdfb000000
	li	t0, 0x81
	sb	t0, 0x118(t2)
	lw      a0, 0x118(t2)
	bal	hexserial
	nop
	TTYDBG("\r\n")

	TTYDBG("Enable HyperTransport Controller scrambling\r\n")
	dli	t2, 0x90000efdfb000000
	li	t0, 0x78
	sb	t0, 0x130(t2)
	lw      a0, 0x130(t2)
	bal	hexserial
	nop
	TTYDBG("\r\n")
#endif

#endif



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
	dli	t2, 0x90000efdfb000040
	lw	a0, 0x0(t2)

	bal	hexserial
	nop

	li	t1, 0xfbffffff
	and	a0, a0, t1
	sw	a0, 0x0(t2)
	lw	a0, 0x0(t2)

	bal	hexserial
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
	dli	t0, 0x90000efdfe000000
	li	t1, 0x11
	sb	t1, 0xcb(t0)
	lw      a0, 0xc8(t0)
	bal	hexserial
	nop
	TTYDBG("\r\n")
#else
	TTYDBG("Setting HyperTransport Southbridge to be 8-bit width\r\n")
	dli	t0, 0x90000efdfe000000
	li	t1, 0x00
	sync
	sb	t1, 0xcb(t0)
	sync
	lw      a0, 0xc8(t0)
	bal	hexserial
	nop
	TTYDBG("\r\n")

#endif

#ifdef HT_2400M
	TTYDBG("Setting HyperTransport Southbridge to be 2400M\r\n")
	dli	t0, 0x90000efdfe000000
	li	t1, 0xd
#else
#ifdef HT_2000M
	TTYDBG("Setting HyperTransport Southbridge to be 2000M\r\n")
	dli	t0, 0x90000efdfe000000
	li	t1, 0xb
#else
#ifdef HT_1800M
	TTYDBG("Setting HyperTransport Southbridge to be 1800M\r\n")
	dli	t0, 0x90000efdfe000000
	li	t1, 0xa
#else
#ifdef HT_1600M
	TTYDBG("Setting HyperTransport Southbridge to be 1600M\r\n")
	dli	t0, 0x90000efdfe000000
	li	t1, 0x9
#else
#ifdef HT_1200M
	TTYDBG("Setting HyperTransport Southbridge to be 1200M\r\n")
	dli	t0, 0x90000efdfe000000
	li	t1, 0x7
#else
#ifdef HT_1000M
	TTYDBG("Setting HyperTransport Southbridge to be 1000M\r\n")
	dli	t0, 0x90000efdfe000000
	li	t1, 0x6
#else
#ifdef HT_800M //Write Southbridge to 800Mhz
	TTYDBG("Setting HyperTransport Southbridge to be 800M\r\n")
    	dli	t0, 0x90000efdfe000000
	li	t1, 0x5
#endif
#endif
#endif
#endif
#endif
#endif
#endif
	sync
	sb	t1, 0xd1(t0)
	sync
	lw      a0, 0xd0(t0)
	bal	hexserial
	nop
	TTYDBG("\r\n")


#ifdef HT3_0
	TTYDBG("Setting HyperTransport Southbridge to be Gen3 mode\r\n")
	dli	t0, 0x90000efdfe000000
	li	t1, 0x00
	sync
	sb	t1, 0x9e(t0)
	sync
	lw      a0, 0x9c(t0)
	bal	hexserial
	nop
	TTYDBG("\r\n")

	dli	t0, 0x90000efdfe000000
	li	t1, 0x4c
	sync
	sb	t1, 0xd8(t0)
	sync
	lw      a0, 0xd8(t0)
	bal	hexserial
	nop
	TTYDBG("\r\n")

	TTYDBG("Setting HyperTransport Southbridge Gen3 function\r\n")
	dli	t0, 0x90000efdfe000000
	li	t1, 0x1
	sync
	sb	t1, 0x44(t0)
	sync
	lw      a0, 0x44(t0)
	bal	hexserial
	nop
	TTYDBG("\r\n")
	dli	t0, 0x90000efdfe000000
	li	t1, 0x8
	sync
	sb	t1, 0xac(t0)
	sync
	lw      a0, 0xac(t0)
	bal	hexserial
	nop
	TTYDBG("\r\n")
	dli	t0, 0x90000efdfe000000
	li	t1, 0x22
	sync
	sb	t1, 0xa0(t0)
	sync
	lw      a0, 0xa0(t0)
	bal	hexserial
	nop
	TTYDBG("\r\n")

#endif


#if 1 //Watch dog Trying
	TTYDBG("Setting Watch Dog to make a WARM RESET\r\n")
    li      t1, 10
//#define WD_DEBUG
	TTYDBG("Watch dog Enable\r\n")
	dli	t0, 0x90000efdfc000cd6
	li	a0, 0x00000069
	sync
	sb	a0, 0(t0)
	sync
	li	a0, 0x00000000
	sb	a0, 1(t0)
	sync
	lb	a0, 1(t0)
	sync
	//bal	hexserial
	nop
	TTYDBG("\r\n")
	nop
	dli	t0, 0x90000efdfc000cd6
	li	a0, 0x0000006c
	sb	a0, 0(t0)
	li	a0, 0x00000000
	sb	a0, 1(t0)
	nop
	li	a0, 0x0000006d
	sb	a0, 0(t0)
	li	a0, 0x00000000
	sb	a0, 1(t0)
	nop
	li	a0, 0x0000006e
	sb	a0, 0(t0)
	li	a0, 0x00000001
	sb	a0, 1(t0)
	nop
	li	a0, 0x0000006f
	sb	a0, 0(t0)
	li	a0, 0x00000000
	sb	a0, 1(t0)
	nop
#ifdef WD_DEBUG
	lb      a0, 1(t0)
	bal	hexserial
	nop
	TTYDBG("\r\n")
#endif

#ifdef WD_DEBUG
	lb	a0, 1(t0)
	bal	hexserial
	nop	
	TTYDBG("\r\n")

	TTYDBG("Watch dog base value\r\n")
	li	a0, 0x00000069
	sb	a0, 0(t0)
	lb      a0, 1(t0)
	bal	hexserial
	nop	
	TTYDBG("\r\n")
	li	a0, 0x0000006c
	sb	a0, 0(t0)
	lb      a0, 1(t0)
	bal	hexserial
	nop	
	TTYDBG("\r\n")
	li	a0, 0x0000006d
	sb	a0, 0(t0)
	lb      a0, 1(t0)
	bal	hexserial
	nop	
	TTYDBG("\r\n")
	li	a0, 0x0000006e
	sb	a0, 0(t0)
	lb      a0, 1(t0)
	bal	hexserial
	nop	
	TTYDBG("\r\n")
	li	a0, 0x0000006f
	sb	a0, 0(t0)
	lb      a0, 1(t0)
	bal	hexserial
	nop	
	TTYDBG("\r\n")
#endif
	TTYDBG("Watch dog decode enable\r\n")
	dli	t0, 0x90000efdfe00a041
	li      a0, 0xff
	sb      a0, 0(t0)
	lb      a0, 0(t0)
	bal	hexserial
	nop	
	TTYDBG("\r\n")

	TTYDBG("Watch dog control value\r\n")
	dli	t0, 0x90000e0000010000
	sync
	lw	a0, 0(t0)
	bal	hexserial
	nop	
	TTYDBG("\r\n")
	sync
	lw	a0, 4(t0)
	bal	hexserial
	nop	
	TTYDBG("\r\n")
	TTYDBG("Set Watch dog control value\r\n")
	li      a0, 0x15
	sw	a0, 4(t0)
	nop	
	li      a0, 0x01
	sw	a0, 0(t0)
	nop	
	li      a0, 0x81
	sw	a0, 0(t0)
	nop	

#endif
	
#if 1//wait until HT link down
	TTYDBG("Waiting HyperTransport bus to be down.")
	dli     t0, 0x90000efdfb000000
	li	t1, 0x1f
1:
	lw      a0, 0x44(t0)
	#bal	hexserial
	nop
	beqz	t1,2f
	TTYDBG(">")
	addi	t1, t1, -1
	b	3f
	nop
2:
	TTYDBG("\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b=")
	li	t1, 0x1f

3:
	lw      a0, 0x44(t0)
	li	a1, 0x20
	and	a0, a0, a1

	bnez	a0, 1b
	nop

	TTYDBG("\r\n")
	lw      a0, 0x44(t0)
	bal	hexserial
	nop
	TTYDBG("\r\n")
#endif

#if 1//wait until HT link up
	TTYDBG("Waiting HyperTransport bus to be up.")
	dli     t0, 0x90000efdfb000000
	li	t1, 0x1f
1:
	lw      a0, 0x44(t0)
	#bal	hexserial
	nop
	beqz	t1,2f
	TTYDBG(">")
	addi	t1, t1, -1
	b	3f
	nop
2:
	TTYDBG("\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b=")
	li	t1, 0x1f

3:
	lw      a0, 0x44(t0)
	li	a1, 0x20
	and	a0, a0, a1

	beqz	a0,	1b
	nop

	TTYDBG("\r\n")
	lw      a0, 0x44(t0)
	bal	hexserial
	nop
	TTYDBG("\r\n")
#endif

#if 1
	TTYDBG("Setting HyperTransport Southbridge back to be 8-bit width and 200Mhz for next RESET\r\n")
	dli	t0, 0x90000efdfe000000
	li	t1, 0x00
	sync
	sb	t1, 0xd1(t0)
	sync
	lw      a0, 0xd0(t0)
	sync
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
	li	a1, 0x300
	and	a0, a0, a1

	beqz	a0, 1f
	nop

	lw      a0, 0x44(t0)
	bal	hexserial
	nop
	TTYDBG("\r\nReset the Controller errror CRC bit\r\n")
 	nop

	lw      a0, 0x44(t0)
	li	a1, 0xfffffcff
	and	a0, a0, a1

	sw	a0, 0x44(t0)
	nop

#ifdef RESET_CRC
	b	2b
	nop
#endif

1:
	TTYDBG("Checking HyperTransport SouthBridge CRC error bit.\r\n")
	dli	t0, 0x90000efdfe000000
2:
	lw      a0, 0x48(t0)
	li	a1, 0x300
	and	a0, a0, a1

	beqz	a0,	1f
	nop

	lw      a0, 0x48(t0)
	bal	hexserial
	nop
	TTYDBG("\r\nReset the Bridge errror CRC bit\r\n")
 	nop

	lw      a0, 0x48(t0)
	li	a1, 0xfffffcff
	and	a0, a0, a1

	sw	a0, 0x48(t0)
	nop

#ifdef RESET_CRC
	b	2b
	nop
#endif


1:
	TTYDBG("Done\r\n")

#endif

#if 1//Read HT channel priority
	TTYDBG("Read HT Channel priority\r\n")
	dli	t2, 0x90000efdfb000000
	lw      a0, 0x50(t2)
	bal	hexserial
	nop
	TTYDBG("\r\n")
#endif


