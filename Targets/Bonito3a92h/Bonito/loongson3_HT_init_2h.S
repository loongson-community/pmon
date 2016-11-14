/*
   HT initialization for 2h
*/

######################################################
#define HT_32bit_TRANS
#define HT_800M
#define HT_RECONNECT
#define UMA_VRAM_MODE
######################################################

#ifdef HT_32bit_TRANS //PCI CFG : TYPE 0: 
	TTYDBG("32 bit PCI space translate to 64 bit HT space\r\n")

	dli	t0, 0x900000003ff02000
	dli	t2, 0x900000003ff02700

1:
 //map HT: PCI IO : 0x90000efd_fc000000 --> 0x18000000
 //map              0x90000efd_fd000000 --> 0x19000000
 //map HT: PCI CFG: 0x90000efd_fe000000 --> 0x1a000000
 //map HT: PCI CFG: 0x90000efd_ff000000 --> 0x1b000000
	dli	t1, 0x000000001b000000
	sd 	t1, 0x0(t0)
	dli	t1, 0xffffffffff000000
	sd	t1, 0x40(t0)
	dli	t1, 0x00000e001f0000f7
	sd	t1, 0x80(t0)

// map PCIE space to 0x10000000
	dli	t1, 0x0000000018000000
	sd	t1, 0x08(t0)
	dli	t1, 0xffffffffff000000
	sd	t1, 0x48(t0)
	dli	t1, 0x00000e00180000f7
	sd	t1, 0x88(t0)

	dli	t1, 0x0000000010000000
	sd	t1, 0x10(t0)
	dli	t1, 0xfffffffff8000000
	sd	t1, 0x50(t0)
	dli	t1, 0x00000e00100000f7
	sd	t1, 0x90(t0)


	// below added for ls2h dc frame buffer

	//map 0x90000e00_00000000 --> 0x40000000
	dli	t1, 0x0000000040000000
	sd	t1, 0x18(t0)
	dli	t1, 0xffffffffc0000000
	sd	t1, 0x58(t0)
	dli	t1, 0x00000e00000000f7
	sd	t1, 0x98(t0)

	daddiu  t0, t0, 0x100
	bne     t0, t2, 1b
	nop

#endif

#if 1
	//wait until HT link up
	TTYDBG("\r\nWaiting HyperTransport bus to be up.\r\n")
	dli     t0, 0x90000efdfb000000
1:
	lw      a0, 0x44(t0)
	andi	a0, a0, 0x20
	beqz	a0, 1b
	nop

	TTYDBG("HyperTransport bus up\r\n")
#endif

#define DEBUG_HTBUS1
#ifdef DEBUG_HTBUS1
	dli	t2, 0x90000efdfb000000
	li	a0, 0x00000001
	sw	a0, 0x54(t2)
	sync
	//li	a0, 0x00004444
	//sw	a0, 0x50(t2)
	sync
	//li	a0, 0x00000000
	//sw	a0, 0x54(t2)

#if 1//Print all HT registers
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
#endif

#if 1 //reset the PLL config and reset the bus
/*
#define PHY_LO_DIV 8
#define PHY_HI_DIV 8
#define DIV_REFC   1
#define DIV_LOOPC  24
#define CORE_DIV   8
*/
#ifdef HT_800M
#define PHY_LO_DIV 2
#define PHY_HI_DIV 2
#define DIV_REFC   1
#define DIV_LOOPC  32
#define CORE_DIV   8
#else
#define PHY_LO_DIV 8
#define PHY_HI_DIV 8
#define DIV_REFC   1
#define DIV_LOOPC  32
#define CORE_DIV   8
#endif
        TTYDBG("Setting CPU0 HyperTransport Controller to be soft config\r\n")
        dli         t2, 0x90000efdfb000000
        //li          t0, 0x00464083
        li          t0, (PHY_LO_DIV << 22) | (PHY_HI_DIV << 18) | (DIV_REFC << 16) | (DIV_LOOPC << 9) | (CORE_DIV << 5) | 0x3
        sw          t0, 0x178(t2)
        lw          a0, 0x178(t2)
        bal         hexserial
        nop
        TTYDBG("\r\n")

#if 1
	TTYDBG("SET HT bus 8 bits\r\n")
	dli	t2, 0x90000efdfb000000
	dli     t3, 0x00000000
	sb      t3, 0x47(t2)
#endif

#if 1
	TTYDBG("RESET HT bus\r\n")
	dli	t2, 0x90000efdfb000000
	dli     t3, 0x00000000
	sw      t3, 0x3c(t2)
	dli     t3, 0x00400000
	sw      t3, 0x3c(t2)

	TTYDBG("Waiting HyperTransport bus to be down.\r\n")
	li	a1, 100
1:
	dli	t2, 0x90000efdfb000000
	lw      a0, 0x44(t2)
	andi	a0, a0, 0x20
	//addi	a1, a1, -1
	bnez	a0, 1b
	nop

#define DEBUG_HTBUS
#ifdef DEBUG_HTBUS
	dli	t2, 0x90000efdfb000000
	li	a0, 0x00000001
	sw	a0, 0x54(t2)
	sync
	//li	a0, 0x00000000
	//sw	a0, 0x54(t2)

#if 1//Print all HT registers
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
#endif


//#define POWERDOWN
#ifdef POWERDOWN
	li	a0, 0xbfe0011C
	li	a1, 0x73fb
	sw	a1, 0x4(a0)//OE

	li	t0, 0xfffffffb
	lw	a1, 0x0(a0)
	and	a1, a1, t0
	sw	a1, 0x0(a0)

	TTYDBG("reset 2H HT\r\n")

#endif

	//bal     inputaddress
	nop
	//TTYDBG("HyperTransport bus down\r\n")

	dli	t2, 0x90000efdfb000000
	sw	zero, 0x3c(t2)

#ifdef POWERDOWN
	li	a0, 0xbfe0011C
	lw	a1, 0x0(a0)
	ori	a1, 0x4//bit-2 : LED 2
	sw	a1, 0x0(a0)
#endif

	TTYDBG("Waiting HyperTransport bus to be up again.\r\n")
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

	TTYDBG("HyperTransport bus up again\r\n")

#endif


#endif

#if 0 //reset the PLL config and reset the bus

#if 1
	TTYDBG("RESET HT bus\r\n")
	dli	t2, 0x90000efdfb000000
	dli     t3, 0x00000000
	sw      t3, 0x3c(t2)
	dli     t3, 0x00400000
	sw      t3, 0x3c(t2)

	TTYDBG("Waiting HyperTransport bus to be down.\r\n")
	li	a1, 100
1:
	dli	t2, 0x90000efdfb000000
	lw      a0, 0x44(t2)
	andi	a0, a0, 0x20
	//addi	a1, a1, -1
	bnez	a0, 1b
	nop

#define DEBUG_HTBUS
#ifdef DEBUG_HTBUS
	dli	t2, 0x90000efdfb000000
	li	a0, 0x00000001
	sw	a0, 0x54(t2)
	sync
	//li	a0, 0x00000000
	//sw	a0, 0x54(t2)

#if 1//Print all HT registers
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
#endif


#define POWERDOWN
#ifdef POWERDOWN
	li	a0, 0xbfe0011C
	li	a1, 0x73fb
	sw	a1, 0x4(a0)//OE

	li	t0, 0xfffffffb
	lw	a1, 0x0(a0)
	and	a1, a1, t0
	sw	a1, 0x0(a0)

	TTYDBG("reset 2H HT\r\n")

#endif

	bal     inputaddress
	nop
	//TTYDBG("HyperTransport bus down\r\n")

	dli	t2, 0x90000efdfb000000
	sw	zero, 0x3c(t2)

#ifdef POWERDOWN
	li	a0, 0xbfe0011C
	lw	a1, 0x0(a0)
	ori	a1, 0x4//bit-2 : LED 2
	sw	a1, 0x0(a0)
#endif

	TTYDBG("Waiting HyperTransport bus to be up again.\r\n")
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

	TTYDBG("HyperTransport bus up again\r\n")

#endif


#endif

#if 0
//SET 2H DMA write POST
	TTYDBG("HT RX DMA write SET to POST\r\n")
	dli	t2, 0x90000efdfb0000d0
	li	t0, 0x80000000
	sw	t0, 0x0(t2)
	li	t0, 0x0000ff00
	sw	t0, 0x4(t2)

	li	t2, 0xbb000000
	sw	a0, 0x0(t2)
#endif

#define DEBUG_HTBUS
#ifdef DEBUG_HTBUS
	dli	t2, 0x90000efdfb000000
	li	a0, 0x00000001
	sw	a0, 0x54(t2)
	sync
	//li	a0, 0x00000000
	//sw	a0, 0x54(t2)

#if 1//Print all HT registers
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
#endif


	//Set 2H receive space
#if 1
	TTYDBG("Set 2H HT receive space\r\n")
	dli	t2, 0x90000efdfe000000
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
	dli	t2, 0x90000efdfe000000
	dli     t3, 0x80000000
	sw      t3, 0xf0(t2)
	//li     t3, 0x0010fff0
	dli     t3, 0x0000ffe0
	sw      t3, 0xf4(t2)

#ifdef PCIE_GRAPHIC_CARD
	// for pcie graphic card
	TTYDBG("Set some space in 2H HT receive space UNCACHED: 0x0000000000~0x7fffffffff\r\n")
	dli     t2, 0x90000efdfe000000
	dli     t3, 0x80000000
	sw      t3, 0xf8(t2)
	dli     t3, 0x5000f000
	sw      t3, 0xfc(t2)
#endif

//Enable 2H DMA cached address
	TTYDBG("Enable 2H DMA cached address\r\n")
	dli	t2, 0x90000efdfe000000
	dli     t3, 0x80000000
	sw      t3, 0xe0(t2)
	dli     t3, 0x00008000
	sw      t3, 0xe4(t2)


//OPEN RX SPACE in 3A
	TTYDBG("HT RX DMA address ENABLE\r\n")
	dli	t2, 0x90000efdfb000060
	li	t0, 0xc0000000
	sw	t0, 0x0(t2)
	li	t0, 0x0000fff0
	sw	t0, 0x4(t2)
	TTYDBG("HT RX DMA address ENABLE done 1\r\n")

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


//SET 2H DMA write POST
	TTYDBG("HT RX DMA write SET to POST\r\n")
	dli	t2, 0x90000efdfb0000d0
	li	t0, 0x80000080
	sw	t0, 0x0(t2)
	li	t0, 0x0080ff80
	sw	t0, 0x4(t2)

#ifdef HT_800M//Set HT bridge to be 800Mhz
	TTYDBG("Setting HyperTransport Controller to be 800Mhz\r\n")
	dli	t2, 0x90000efdfb000000
	li	t0, 0x5 //Frequency: 0x5:800 Mhz 0x4:400 Mhz
	sb	t0, 0x49(t2)
	lw      a0, 0x48(t2)
	bal	hexserial
	nop
	TTYDBG("\r\n")
#endif

#ifdef HT_RECONNECT
	TTYDBG("RESET HT bus\r\n")
	dli	t2, 0x90000efdfb000000
	dli     t3, 0x00000000
	sw      t3, 0x3c(t2)
	dli     t3, 0x00400000
	sw      t3, 0x3c(t2)

	TTYDBG("Waiting HyperTransport bus to be down.\r\n")
1:
	dli	t2, 0x90000efdfb000000
	lw      a0, 0x44(t2)
	andi	a0, a0, 0x20
	bnez	a0, 1b
	nop

	TTYDBG("HyperTransport bus down\r\n")

	dli	t2, 0x90000efdfb000000
	sw	zero, 0x3c(t2)

	TTYDBG("Waiting HyperTransport bus to be up again.\r\n")
1:
	dli	t2, 0x90000efdfb000000
	lw      a0, 0x44(t2)
	andi	a0, a0, 0x20
	beqz	a0, 1b
	nop

	TTYDBG("HyperTransport bus up again\r\n")

	/* we reconfigure the ht freq to 200Mhz "WITHOUT" resetting the ht link */

	TTYDBG("Setting HyperTransport Southbridge back to be 8-bit width and 200Mhz for next RESET\r\n")
	dli	t2, 0x90000efdfb000000
	li	t0, 0x0 //Frequency: 200 Mhz
	sb	t0, 0x49(t2)
	lw      a0, 0x48(t2)
	bal	hexserial
	nop
	TTYDBG("\r\n")
#endif


	//Set  HT channel byte write for LS2H 
	TTYDBG("Set HT channel byte write for LS2H\r\n")
	dli	t2, 0x90000efdfe000000
	li	t0, 0x4321
	sw	t0, 0x50(t2)
	sync
	lw      a0, 0x50(t2)
	bal	hexserial
	nop
	//Set  HT channel in order for LS2H DMA
	TTYDBG("Set HT channel in order for LS2H DMA\r\n")
	dli	t2, 0x90000efdfe000000
	lw	t0, 0x50(t2)
	li	t1, 0x00010000
	or	t0, t0, t1
	sw	t0, 0x50(t2)
	sync
	lw      a0, 0x50(t2)
	bal	hexserial
	nop

#define set_Xbar_win(xbarbase, mas_i, win_i, base, mask, mmap) \
	li	t0, xbarbase ;\
	daddiu	t0, t0, mas_i*0x100 ;\
	daddiu	t0, t0, win_i*0x8 ;\
	dli	t1, base ;\
	sd	t1, 0x0(t0) ;\
	dli	t1, mask ;\
	sd	t1, 0x40(t0) ;\
	dli	t1, mmap ;\
	sd	t1, 0x80(t0)

#### IODMA 4G ####
	set_Xbar_win(0xbbd80000, 1, 0, 0x0000000000000000, 0xffffffffc0000000, 0x00000010800000f3) # 0~1G
	set_Xbar_win(0xbbd80000, 1, 1, 0x0000000040000000, 0xffffffffc0000000, 0x00000020800000f3) # 1~2G
	set_Xbar_win(0xbbd80000, 1, 2, 0x0000000080000000, 0xffffffffc0000000, 0x00000030800000f3) # 2~3G
	set_Xbar_win(0xbbd80000, 1, 3, 0x00000000c0000000, 0xffffffffc0000000, 0x00000040800000f3) # 3~4G
	set_Xbar_win(0xbbd80000, 1, 4, 0xffffffff80000000, 0xffffffffc0000000, 0x00000030800000f3) # 2~3G 
	set_Xbar_win(0xbbd80000, 1, 5, 0xffffffffc0000000, 0xffffffffc0000000, 0x00000040800000f3) # 3~4G

#PCIE window
	set_Xbar_win(0xbbd80000, 4, 0, 0x0000000000000000, 0xffffffffc0000000, 0x00000010800000f3) # 0~1G
	set_Xbar_win(0xbbd80000, 4, 1, 0x0000000040000000, 0xffffffffc0000000, 0x00000020800000f3) # 1~2G
	set_Xbar_win(0xbbd80000, 4, 2, 0x0000000080000000, 0xffffffffc0000000, 0x00000030800000f3) # 2~3G
	set_Xbar_win(0xbbd80000, 4, 3, 0x00000000c0000000, 0xffffffffc0000000, 0x00000040800000f3) # 3~4G
	set_Xbar_win(0xbbd80000, 4, 4, 0xffffffff80000000, 0xffffffffc0000000, 0x00000030800000f3) # 2~3G
	set_Xbar_win(0xbbd80000, 4, 5, 0xffffffffc0000000, 0xffffffffc0000000, 0x00000040800000f3) # 3~4G

#ifdef UMA_VRAM_MODE
	set_Xbar_win(0xbbd82000, 6, 7, 0x0000000000000000, 0x0000000000000000, 0x00000000000000f0) # others, all to L2$
	set_Xbar_win(0xbbd82000, 6, 0, 0x0000001080000000, 0xfffffffff0000000, 0x00000000000000f6)
	set_Xbar_win(0xbbd82000, 6, 1, 0x0000001080000000, 0xffffffffc0000000, 0x00000000800000f6)
	set_Xbar_win(0xbbd82000, 6, 2, 0x0000002080000000, 0xffffffffc0000000, 0x00000000c00000f6)
	set_Xbar_win(0xbbd82000, 6, 3, 0x0000003080000000, 0xffffffffc0000000, 0x00000001000000f6)
	set_Xbar_win(0xbbd82000, 6, 4, 0x0000004080000000, 0xffffffffc0000000, 0x00000001400000f6)
#ifdef PCIE_GRAPHIC_CARD
	set_Xbar_win(0xbbd82000, 6, 5, 0x0000005000000000, 0xffffffffc0000000, 0x00000000400000f0) # to ht int // mj
#else
	set_Xbar_win(0xbbd82000, 6, 5, 0x000000f080000000, 0xffffffffff000000, 0x000000fdf80000f6) # to ht int
#endif
	set_Xbar_win(0xbbd82000, 6, 6, 0x0000000040000000, 0xffffffffc0000000, 0x00000000000000f0) # gpu dma 0x4... -> 0x0...
#else
	set_Xbar_win(0xbbd82000, 6, 6, 0x0000000000000000, 0x0000000000000000, 0x00000000000000f0) # others, all to L2$
	set_Xbar_win(0xbbd82000, 6, 0, 0x0000001080000000, 0xfffffffff0000000, 0x00000000000000f6)
	set_Xbar_win(0xbbd82000, 6, 1, 0x0000001080000000, 0xffffffffc0000000, 0x00000000800000f6)
	set_Xbar_win(0xbbd82000, 6, 2, 0x0000002080000000, 0xffffffffc0000000, 0x00000000c00000f6)
	set_Xbar_win(0xbbd82000, 6, 3, 0x0000003080000000, 0xffffffffc0000000, 0x00000001000000f6)
	set_Xbar_win(0xbbd82000, 6, 4, 0x0000004080000000, 0xffffffffc0000000, 0x00000001400000f6)
	set_Xbar_win(0xbbd82000, 6, 5, 0x000000f080000000, 0xffffffffff000000, 0x000000fdf80000f6) # to ht int
#endif

	set_Xbar_win(0xbbd80000, 0, 6, 0x0000000018000000, 0xfffffffff8000000, 0x0000000018000081) # copy of sb win
	set_Xbar_win(0xbbd80000, 0, 4, 0x0000000019000000, 0xffffffffff000000, 0x000000f080000083) # to ht int(l1)

#ifdef UMA_VRAM_MODE
#GPU window
	set_Xbar_win(0xbbd80000, 2, 0, 0x0000000040000000, 0xffffffffc0000000, 0x0000001080000003) # 1~2G -> iodma -> 3a
	set_Xbar_win(0xbbd80000, 2, 1, 0x0000000040000000, 0xffffffffc0000008, 0x00000010800000f3) # 1~2G -> iodma -> 3a
	//set_Xbar_win(0xbbd80000, 2, 2, 0x0000000000000000, 0xffffffffc0000000, 0x00000000400000f3) # 0~1G -> iodma -> L2$ 0x40000000 -> ddr 256M
	set_Xbar_win(0xbbd80000, 2, 2, 0x0000000000000000, 0xffffffffc0000000, 0x00000000000000f0) # 0~1G to ddr

	//set_Xbar_win(0xbbd80000, 2, 0, 0x0000000000000000, 0xffffffffc0000000, 0x00000000400000f3) # 0~1G -> iodma -> L2$
#endif

	// set ls3a-2h dma cached coherence by hardware
	//li	t2, 0x90001EFDFB0000F0; li t1, 0x0; sw t1, 0x00(t2);
	//support 1000M phy
	li	t1, 0x800f0000; li t2, 0xbbd0020c; sw t1, 0x00(t2); 
	dli     t0, 0x90000efdfe000000
	li	a0, 0x01
	sw	a0, 0x30(t0)
