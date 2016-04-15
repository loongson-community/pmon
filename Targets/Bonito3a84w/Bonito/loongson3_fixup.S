/*whd : loongson3_fixup.S
	used to fix up the potential addressing miss
	caused by speculated execution

ATTENTION: NO 16BIT mode when using HT1

*/
#define HT0_RECONNECT
//#define INTERCONNECTION_HT0_1_0 //USE HT1.0 on Ring
//#define HT0_3200M
//#define HT0_2400M
//#define HT0_2200M
//#define HT0_2000M
//#define HT0_1800M
#define HT0_1600M
//#define HT0_200M

#define ENABLE_X
//#define INTERCONNECTION_X_HT1_1_0 //USE HT1.0 on X
#define HT1_X_1600M
#define HT1_X_800M //Only in HT1.0 mode

//#define HT0_16BIT //NO 16Bit mode in 4 chip interconnection
#define USE_HT_RESET
#define RESET_AGAIN_WHEN_FAIL
#define ENABLE_RDinterleave


#if 1 //Fix L2XBAR
	TTYDBG("Fix L2xbar in NODE 1\r\n")
	dli	t2, 0x900010003ff00000
//keep consistent to NODE 0.
//windows 1 used to route 0x10000000~ access
	dli 	t0, 0x0000100010000000
	sd  	t0, 0x08(t2)
	dli 	t0, 0xfffffffff0000000
	sd  	t0, 0x48(t2)
	dli 	t0, 0x0000000010000082
	sd  	t0, 0x88(t2)
//disable window 0
	sd  	$0, 0x80(t2)
	sd  	$0, 0x90(t2)

	TTYDBG("Fix L2xbar in NODE 2\r\n")
	dli	t2, 0x900020003ff00000
//keep consistent to NODE 0.
//windows 1 used to route 0x10000000~ access
	dli 	t0, 0x0000200010000000
	sd  	t0, 0x08(t2)
	dli 	t0, 0xfffffffff0000000
	sd  	t0, 0x48(t2)
	dli 	t0, 0x0000000010000082
	sd  	t0, 0x88(t2)
//disable window 0
	sd  	$0, 0x80(t2)
	sd  	$0, 0x90(t2)

	TTYDBG("Fix L2xbar in NODE 3\r\n")
	dli	t2, 0x900030003ff00000
//keep consistent to NODE 0.
//windows 1 used to route 0x10000000~ access
	dli 	t0, 0x0000300010000000
	sd  	t0, 0x08(t2)
	dli 	t0, 0xfffffffff0000000
	sd  	t0, 0x48(t2)
	dli 	t0, 0x0000000010000082
	sd  	t0, 0x88(t2)
//disable window 0
	sd  	$0, 0x80(t2)
	sd  	$0, 0x90(t2)

#endif

#if 1 //Check if HT1 is OK
	TTYDBG("Check if HT1 is OK\r\n")
	dli	t2, 0x90000ffdfe000000
	lw	a0, 0x50(t2)
	bal	hexserial
	nop
	dli	t2, 0x90001ffdfe000000
	lw	a0, 0x50(t2)
	bal	hexserial
	nop
	dli	t2, 0x90003ffdfe000000
	lw	a0, 0x50(t2)
	bal	hexserial
	nop
	dli	t2, 0x90002ffdfe000000
	lw	a0, 0x50(t2)
	bal	hexserial
	nop
#endif

#if 1
	TTYDBG("Shut down CPU1\r\n")
	dli	a0, 0x900010001fe001d0
	li	a1, 0x0
	sw	a1, 0x0(a0)

	TTYDBG("Shut down CPU2\r\n")
	dli	a0, 0x900020001fe001d0
	li	a1, 0x0
	sw	a1, 0x0(a0)

	TTYDBG("Shut down CPU3\r\n")
	dli	a0, 0x900030001fe001d0
	li	a1, 0x0
	sw	a1, 0x0(a0)
#endif


#ifdef ENABLE_X
	TTYDBG("Begin to enable X routing\r\n")

//setup ICCC_EN in HT1_HI
	dli	t2, 0x90000ffdfb000000
	lw	t1, 0x50(t2)
	li	t0, 0x00400000
	or	t1, t1, t0
	sw	t1, 0x50(t2)
	dli	t2, 0x90001ffdfb000000
	lw	t1, 0x50(t2)
	li	t0, 0x00400000
	or	t1, t1, t0
	sw	t1, 0x50(t2)
	dli	t2, 0x90002ffdfb000000
	lw	t1, 0x50(t2)
	li	t0, 0x00400000
	or	t1, t1, t0
	sw	t1, 0x50(t2)
	dli	t2, 0x90003ffdfb000000
	lw	t1, 0x50(t2)
	li	t0, 0x00400000
	or	t1, t1, t0
	sw	t1, 0x50(t2)

/* SET HT connection interleave between two LS3A chipses */ 
	dli 	t0, 0x90000ffdfb000108;
	lw  	a0, 0x00(t0);
	li  	a1, 0xc0000000; 
	or  	a0, a1;
	sw  	a0, 0x00(t0);

	dli 	t0, 0x90001ffdfb000108;
	lw  	a0, 0x00(t0);
	li  	a1, 0xc0000000; 
	or  	a0, a1;
	sw  	a0, 0x00(t0);

	dli 	t0, 0x90002ffdfb000108;
	lw  	a0, 0x00(t0);
	li  	a1, 0xc0000000; 
	or  	a0, a1;
	sw  	a0, 0x00(t0);

	dli 	t0, 0x90003ffdfb000108;
	lw  	a0, 0x00(t0);
	li  	a1, 0xc0000000; 
	or  	a0, a1;
	sw  	a0, 0x00(t0);

#ifdef INTERCONNECTION_X_HT1_1_0
	TTYDBG("X HT1.0 used \r\n")
#########TEST CLKSEL[15]
	li	t2, 0xbfe00194
	lw	t1, 0x0(t2)
	andi	t1, 0x8000
	bnez	t1, no_softconfig_x_ht1
	nop

	TTYDBG("Setting CPU1 HyperTransport Controller to be soft config\r\n")
	dli	t2, 0x90001ffdfb000000
	li	t0, 0x00464083
	sw	t0, 0x178(t2)
	lw      a0, 0x178(t2)
	bal	hexserial
	nop
	TTYDBG("\r\n")

	TTYDBG("Setting CPU0 HyperTransport Controller to be soft config\r\n")
	dli	t2, 0x90000ffdfb000000
	li	t0, 0x00464083
	sw	t0, 0x178(t2)
	lw      a0, 0x178(t2)
	bal	hexserial
	nop
	TTYDBG("\r\n")

	TTYDBG("Setting CPU2 HyperTransport Controller to be soft config\r\n")
	dli	t2, 0x90002ffdfb000000
	li	t0, 0x00464083
	sw	t0, 0x178(t2)
	lw      a0, 0x178(t2)
	bal	hexserial
	nop
	TTYDBG("\r\n")

	TTYDBG("Setting CPU3 HyperTransport Controller to be soft config\r\n")
	dli	t2, 0x90003ffdfb000000
	li	t0, 0x00464083
	sw	t0, 0x178(t2)
	lw      a0, 0x178(t2)
	bal	hexserial
	nop
	TTYDBG("\r\n")

no_softconfig_x_ht1:
###################### HT_HI@CPU1
	dli	a0, 0x90001ffdfb000000
#ifdef HT1_X_800M
	//set 800 Mhz HT HOST
	lw  	a1, 0x48(a0)
	li  	a2, 0x500 ##800Mhz
	or  	a1, a1, a2  
	sw  	a1, 0x48(a0)
#endif

	//set 8 bit HT HOST
	lw  	a1, 0x44(a0)
	li  	a2, 0x88ffffff        ##8bit mode
	and 	a1, a1, a2            ##set to 8 bit mode
	sw  	a1, 0x44(a0)

###################### HT_HI@CPU0
	dli	a0, 0x90000ffdfb000000
#ifdef HT1_X_800M
	//set 800 Mhz HT HOST
	lw  	a1, 0x48(a0)
	li  	a2, 0x500 ##800Mhz
	or  	a1, a1, a2  
	sw  	a1, 0x48(a0)
#endif

	//set 8 bit HT HOST
	lw  	a1, 0x44(a0)
	li  	a2, 0x88ffffff        ##8bit mode
	and 	a1, a1, a2            ##set to 8 bit mode
	sw  	a1, 0x44(a0)

###################### HT_HI@CPU3
	dli	a0, 0x90003ffdfb000000
#ifdef HT1_X_800M
	//set 800 Mhz HT HOST
	lw  	a1, 0x48(a0)
	li  	a2, 0x500 ##800Mhz
	or  	a1, a1, a2  
	sw  	a1, 0x48(a0)
#endif

	//set 8 bit HT HOST
	lw  	a1, 0x44(a0)
	li  	a2, 0x88ffffff        ##8bit mode
	and 	a1, a1, a2            ##set to 8 bit mode
	sw  	a1, 0x44(a0)

###################### HT_HI@CPU2
	dli	a0, 0x90002ffdfb000000
#ifdef HT1_X_800M
	//set 800 Mhz HT HOST
	lw  	a1, 0x48(a0)
	li  	a2, 0x500 ##800Mhz
	or  	a1, a1, a2  
	sw  	a1, 0x48(a0)
#endif

	//set 8 bit HT HOST
	lw  	a1, 0x44(a0)
	li  	a2, 0x88ffffff        ##8bit mode
	and 	a1, a1, a2            ##set to 8 bit mode
	sw  	a1, 0x44(a0)

#else
	TTYDBG("HT3.0 used on X \r\n")

#####HT3.0 reconnection

###################### HT_HI@CPU0
	dli 	a0, 0x90000ffdfb000000
	//set 8 bit HT HOST
	lw  	a1, 0x44(a0)
	li  	a2, 0x88ffffff        ##8bit mode
	and 	a1, a1, a2            ##set to 8 bit mode
	sw  	a1, 0x44(a0)

###################### HT_HI@CPU1
	dli 	a0, 0x90001ffdfb000000
	//set 8 bit HT HOST
	lw  	a1, 0x44(a0)
	li  	a2, 0x88ffffff        ##8bit mode
	and 	a1, a1, a2            ##set to 8 bit mode
	sw  	a1, 0x44(a0)

###################### HT_HI@CPU2
	dli 	a0, 0x90002ffdfb000000
	//set 8 bit HT HOST
	lw  	a1, 0x44(a0)
	li  	a2, 0x88ffffff        ##8bit mode
	and 	a1, a1, a2            ##set to 8 bit mode
	sw  	a1, 0x44(a0)

###################### HT_HI@CPU3
	dli 	a0, 0x90003ffdfb000000
	//set 8 bit HT HOST
	lw  	a1, 0x44(a0)
	li  	a2, 0x88ffffff        ##8bit mode
	and 	a1, a1, a2            ##set to 8 bit mode
	sw  	a1, 0x44(a0)


#########TEST CLKSEL[15]
	li	t2,0xbfe00194
	lw	t1, 0x0(t2)
	andi	t1, 0x8000
	bnez	t1, no_softconfig_x_ht
	nop


#ifdef HT1_X_2400M
	TTYDBG("Setting CPU0/1/2/3 HyperTransport Controller to be soft config 2400\r\n")
	li	t0, 0x00466083
#else
#ifdef HT1_X_2200M
	TTYDBG("Setting CPU0/1/2/3 HyperTransport Controller to be soft config 2200\r\n")
	li	t0, 0x00465883
#else
#ifdef HT1_X_2000M
	TTYDBG("Setting CPU0/1/2/3 HyperTransport Controller to be soft config 2000\r\n")
	li	t0, 0x00465083
#else
#ifdef HT1_X_1800M
	TTYDBG("Setting CPU0/1/2/3 HyperTransport Controller to be soft config 1800\r\n")
	li	t0, 0x00464883
#else
#ifdef HT1_X_1600M
	TTYDBG("Setting CPU0/1/2/3 HyperTransport Controller to be soft config 1600\r\n")
	li	t0, 0x00464083
#else
#ifdef HT1_X_1200M
	TTYDBG("Setting CPU0/1/2/3 HyperTransport Controller to be soft config 1200\r\n")
	li	t0, 0x00463083
#else
#ifdef HT1_X_200M
	TTYDBG("Setting CPU0/1/2/3 HyperTransport Controller to be soft config 200\r\n")
	li	t0, 0x00460883
#endif
#endif
#endif
#endif
#endif
#endif
#endif

	dli	t2, 0x90000ffdfb000000
	sw 	t0, 0x178(t2)
	dli	t2, 0x90001ffdfb000000
	sw 	t0, 0x178(t2)
	dli	t2, 0x90002ffdfb000000
	sw 	t0, 0x178(t2)
	dli	t2, 0x90003ffdfb000000
	sw 	t0, 0x178(t2)


no_softconfig_x_ht:

#ifdef HT1_X_3200M
	TTYDBG("Setting CPU0/1 HyperTransport Controller to be 3200Mhz\r\n")
	li	t0, 0xf //Frequency: 2400 Mhz
#else
#ifdef HT1_X_2400M
	TTYDBG("Setting CPU0/1 HyperTransport Controller to be 2400Mhz\r\n")
	li	t0, 0xd //Frequency: 2400 Mhz
#else
#ifdef HT1_X_2200M
	TTYDBG("Setting CPU0/1 HyperTransport Controller to be 2200Mhz\r\n")
	li	t0, 0xc //Frequency: 2200 Mhz
#else
#ifdef HT1_X_2000M
	TTYDBG("Setting CPU0/1 HyperTransport Controller to be 2000Mhz\r\n")
	li	t0, 0xb //Frequency: 2000 Mhz
#else
#ifdef HT1_X_1800M
	TTYDBG("Setting CPU0/1 HyperTransport Controller to be 1800Mhz\r\n")
	li	t0, 0xa //Frequency: 1800 Mhz
#else
#ifdef HT1_X_1600M
	TTYDBG("Setting CPU0/1 HyperTransport Controller to be 1600Mhz\r\n")
	li	t0, 0x9 //Frequency: 1600 Mhz
#else
#ifdef HT1_X_1200M
	TTYDBG("Setting CPU0/1 HyperTransport Controller to be 1200Mhz\r\n")
	li	t0, 0x7 //Frequency: 1200 Mhz
#else
#ifdef HT1_X_200M
	TTYDBG("Setting CPU0/1 HyperTransport Controller to be 200Mhz\r\n")
	#li	t0, 0x2 //Frequency: 400 Mhz
	#li	t0, 0x0 //Frequency: 1200 Mhz
	li	t0, 0x1 //Frequency: 200 Mhz
#endif
#endif
#endif
#endif
#endif
#endif
#endif
#endif

	dli	t2, 0x90000ffdfb000000
	sb 	t0, 0x49(t2)
	dli	t2, 0x90001ffdfb000000
	sb 	t0, 0x49(t2)
	dli	t2, 0x90002ffdfb000000
	sb 	t0, 0x49(t2)
	dli	t2, 0x90003ffdfb000000
	sb 	t0, 0x49(t2)

	TTYDBG("Setting CPU0 HT1 HI HyperTransport Controller to be GEN3 mode\r\n")
	dli	t2, 0x90000ffdfb000000
	li 	t0, 0x88600000
	sw 	t0, 0x110(t2)
	lw      a0, 0x110(t2)
	bal	hexserial
	nop
	TTYDBG("\r\n")

	TTYDBG("Setting CPU0 HT1 HI HyperTransport Controller to be retry mode\r\n")
	dli	t2, 0x90000ffdfb000000
	li 	t0, 0x81
	sb 	t0, 0x118(t2)
	lw      a0, 0x118(t2)
	bal	hexserial
	nop
	TTYDBG("\r\n")

	TTYDBG("Enable CPU0 HT1 HI HyperTransport Controller scrambling\r\n")
	dli	t2, 0x90000ffdfb000000
	li 	t0, 0x78
	sb 	t0, 0x130(t2)
	lw      a0, 0x130(t2)
	bal	hexserial
	nop
	TTYDBG("\r\n")

	TTYDBG("Setting CPU1 HT1 HI HyperTransport Controller to be GEN3 mode\r\n")
	dli	t2, 0x90001ffdfb000000
	li 	t0, 0x88600000
	sw 	t0, 0x110(t2)
	lw      a0, 0x110(t2)
	bal	hexserial
	nop
	TTYDBG("\r\n")

	TTYDBG("Setting CPU1 HT1 HI HyperTransport Controller to be retry mode\r\n")
	dli	t2, 0x90001ffdfb000000
	li 	t0, 0x81
	sb 	t0, 0x118(t2)
	lw      a0, 0x118(t2)
	bal	hexserial
	nop
	TTYDBG("\r\n")

	TTYDBG("Enable CPU1 HT1 HI HyperTransport Controller scrambling\r\n")
	dli	t2, 0x90001ffdfb000000
	li 	t0, 0x78
	sb 	t0, 0x130(t2)
	lw      a0, 0x130(t2)
	bal	hexserial
	nop
	TTYDBG("\r\n")

	TTYDBG("Setting CPU2 HT1 HI HyperTransport Controller to be GEN3 mode\r\n")
	dli	t2, 0x90002ffdfb000000
	li 	t0, 0x88600000
	sw 	t0, 0x110(t2)
	lw      a0, 0x110(t2)
	bal	hexserial
	nop
	TTYDBG("\r\n")

	TTYDBG("Setting CPU2 HT1 HI HyperTransport Controller to be retry mode\r\n")
	dli	t2, 0x90002ffdfb000000
	li 	t0, 0x81
	sb 	t0, 0x118(t2)
	lw      a0, 0x118(t2)
	bal	hexserial
	nop
	TTYDBG("\r\n")

	TTYDBG("Enable CPU2 HT1 HI HyperTransport Controller scrambling\r\n")
	dli	t2, 0x90002ffdfb000000
	li 	t0, 0x78
	sb 	t0, 0x130(t2)
	lw      a0, 0x130(t2)
	bal	hexserial
	nop
	TTYDBG("\r\n")

	TTYDBG("Setting CPU3 HT1 HI HyperTransport Controller to be GEN3 mode\r\n")
	dli	t2, 0x90003ffdfb000000
	li 	t0, 0x88600000
	sw 	t0, 0x110(t2)
	lw      a0, 0x110(t2)
	bal	hexserial
	nop
	TTYDBG("\r\n")

	TTYDBG("Setting CPU3 HT1 HI HyperTransport Controller to be retry mode\r\n")
	dli	t2, 0x90003ffdfb000000
	li 	t0, 0x81
	sb 	t0, 0x118(t2)
	lw      a0, 0x118(t2)
	bal	hexserial
	nop
	TTYDBG("\r\n")

	TTYDBG("Enable CPU3 HT1 HI HyperTransport Controller scrambling\r\n")
	dli	t2, 0x90003ffdfb000000
	li 	t0, 0x78
	sb 	t0, 0x130(t2)
	lw      a0, 0x130(t2)
	bal	hexserial
	nop
	TTYDBG("\r\n")

#endif

//reset links
	TTYDBG("Start reset X link 0-3 \r\n")
	dli	t2, 0x90000ffdfb000000
	lw	t1, 0x3c(t2)
	li	t0, 0x00400000
	or	t1, t1, t0
	sw	t1, 0x3c(t2)
1:
	lw	t0, 0x3c(t2)
	bne	t1, t0, 1b
	nop

//check if link down
1:
	lw      a0, 0x44(t2)
	li	a1, 0x20
	and	a0, a0, a1

	bnez	a0, 1b
	nop

	dli	t2, 0x90003ffdfb000000
1:
	lw      a0, 0x44(t2)
	li	a1, 0x20
	and	a0, a0, a1

	bnez	a0, 1b
	nop

	dli	t2, 0x90000ffdfb000000
	lw	t1, 0x3c(t2)
	li	t0, 0xffbfffff
	and	t1, t1, t0
	sw	t1, 0x3c(t2)

	TTYDBG("Start reset X link 1-2 \r\n")
	dli	t2, 0x90001ffdfb000000
	lw	t1, 0x3c(t2)
	li	t0, 0x00400000
	or	t1, t1, t0
	sw	t1, 0x3c(t2)
1:
	lw	t0, 0x3c(t2)
	bne	t1, t0, 1b
	nop

//check if link down
1:
	lw      a0, 0x44(t2)
	li	a1, 0x20
	and	a0, a0, a1

	bnez	a0, 1b
	nop

	dli	t2, 0x90002ffdfb000000
1:
	lw      a0, 0x44(t2)
	li	a1, 0x20
	and	a0, a0, a1

	bnez	a0, 1b
	nop

	dli	t2, 0x90001ffdfb000000
	lw	t1, 0x3c(t2)
	li	t0, 0xffbfffff
	and	t1, t1, t0
	sw	t1, 0x3c(t2)

#if 1//wait until HT link up
    TTYDBG("Waiting HyperTransport bus to be up.")
    dli     t0, 0x90000ffdfb000000
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

#if 1//wait until HT link up
    TTYDBG("Waiting HyperTransport bus to be up.")
    dli     t0, 0x90003ffdfb000000
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



#if 1//wait until HT link up
    TTYDBG("Waiting HyperTransport bus to be up.")
    dli     t0, 0x90001ffdfb000000
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

#if 1//wait until HT link up
    TTYDBG("Waiting HyperTransport bus to be up.")
    dli     t0, 0x90002ffdfb000000
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



//Enable X
	TTYDBG("Enable X routing\r\n")

	dli	t2, 0x900000003ff00400
	lw	t1, 0x0(t2)
	li	t0, 0x00000100
	or	t1, t1, t0
	sw	t1, 0x0(t2)
	dli	t2, 0x900010003ff00400
	lw	t1, 0x0(t2)
	li	t0, 0x00000100
	or	t1, t1, t0
	sw	t1, 0x0(t2)
	dli	t2, 0x900020003ff00400
	lw	t1, 0x0(t2)
	li	t0, 0x00000100
	or	t1, t1, t0
	sw	t1, 0x0(t2)
	dli	t2, 0x900030003ff00400
	lw	t1, 0x0(t2)
	li	t0, 0x00000100
	or	t1, t1, t0
	sw	t1, 0x0(t2)

	TTYDBG("X routing enabled\r\n")
#endif

#if 1
/* SET HT connection interleave between two LS3A chipses */ 
	dli 	t0, 0x90000cfdfb000108;
	lw  	a0, 0x00(t0);
	li  	a1, 0xc0000000; 
	or  	a0, a1;
	sw  	a0, 0x00(t0);

	dli 	t0, 0x90001dfdfb000108;
	lw  	a0, 0x00(t0);
	li  	a1, 0xc0000000; 
	or  	a0, a1;
	sw  	a0, 0x00(t0);

	dli 	t0, 0x90000dfdfb000108;
	lw  	a0, 0x00(t0);
	li  	a1, 0xc0000000; 
	or  	a0, a1;
	sw  	a0, 0x00(t0);

	dli 	t0, 0x90002cfdfb000108;
	lw  	a0, 0x00(t0);
	li  	a1, 0xc0000000; 
	or  	a0, a1;
	sw  	a0, 0x00(t0);

	dli 	t0, 0x90001cfdfb000108;
	lw  	a0, 0x00(t0);
	li  	a1, 0xc0000000; 
	or  	a0, a1;
	sw  	a0, 0x00(t0);

	dli 	t0, 0x90003dfdfb000108;
	lw  	a0, 0x00(t0);
	li  	a1, 0xc0000000; 
	or  	a0, a1;
	sw  	a0, 0x00(t0);

	dli 	t0, 0x90002dfdfb000108;
	lw  	a0, 0x00(t0);
	li  	a1, 0xc0000000; 
	or  	a0, a1;
	sw  	a0, 0x00(t0);

	dli 	t0, 0x90003cfdfb000108;
	lw  	a0, 0x00(t0);
	li  	a1, 0xc0000000; 
	or  	a0, a1;
	sw  	a0, 0x00(t0);

#ifdef HT0_RECONNECT
	TTYDBG("HT0 frequency reconfig \r\n")

#ifdef INTERCONNECTION_HT0_1_0
	TTYDBG("HT1.0 used \r\n")

#########TEST CLKSEL[15]
	li	t2,0xbfe00194
	lw	t1, 0x0(t2)
	andi	t1, 0x8000
	bnez	t1, no_softconfig_ht1
	nop

	TTYDBG("Setting CPU1 HyperTransport Controller to be soft config\r\n")
	dli	t2, 0x90001cfdfb000000
	li	t0, 0x00464083
	sw	t0, 0x178(t2)
	lw      a0, 0x178(t2)
	bal	hexserial
	nop
	TTYDBG("\r\n")

	TTYDBG("Setting CPU0 HyperTransport Controller to be soft config\r\n")
	dli	t2, 0x90000cfdfb000000
	li	t0, 0x00464083
	sw	t0, 0x178(t2)
	lw      a0, 0x178(t2)
	bal	hexserial
	nop
	TTYDBG("\r\n")

	TTYDBG("Setting CPU2 HyperTransport Controller to be soft config\r\n")
	dli	t2, 0x90002cfdfb000000
	li	t0, 0x00464083
	sw	t0, 0x178(t2)
	lw      a0, 0x178(t2)
	bal	hexserial
	nop
	TTYDBG("\r\n")

	TTYDBG("Setting CPU3 HyperTransport Controller to be soft config\r\n")
	dli	t2, 0x90003cfdfb000000
	li	t0, 0x00464083
	sw	t0, 0x178(t2)
	lw      a0, 0x178(t2)
	bal	hexserial
	nop
	TTYDBG("\r\n")

no_softconfig_ht1:
###################### HT_HI@CPU1
	dli	a0, 0x90001dfdfb000000
	//set 800 Mhz HT HOST
	lw  	a1, 0x48(a0)
	li  	a2, 0x500 ##800Mhz
	or  	a1, a1, a2  
	sw  	a1, 0x48(a0)

	//set 8 bit HT HOST
	lw  	a1, 0x44(a0)
	li  	a2, 0x88ffffff        ##8bit mode
	and 	a1, a1, a2            ##set to 8 bit mode
	sw  	a1, 0x44(a0)

###################### HT_LO@CPU0
	dli 	a0, 0x90000cfdfb000000
	//set 800 Mhz HT HOST
	lw  	a1, 0x48(a0)
	li  	a2, 0x500 ##800Mhz
	or  	a1, a1, a2 ##
	sw  	a1, 0x48(a0)

	//set 8 bit HT HOST
	lw  	a1, 0x44(a0)
	li  	a2, 0x88ffffff        ##8bit mode
	and 	a1, a1, a2            ##set to 8 bit mode
	sw  	a1, 0x44(a0)

###################### HT_HI@CPU0
	dli	a0, 0x90000dfdfb000000
	//set 800 Mhz HT HOST
	lw  	a1, 0x48(a0)
	li  	a2, 0x500 ##800Mhz
	or  	a1, a1, a2  
	sw  	a1, 0x48(a0)

	//set 8 bit HT HOST
	lw  	a1, 0x44(a0)
	li  	a2, 0x88ffffff        ##8bit mode
	and 	a1, a1, a2            ##set to 8 bit mode
	sw  	a1, 0x44(a0)

###################### HT_LO@CPU2
	dli 	a0, 0x90002cfdfb000000
	//set 800 Mhz HT HOST
	lw  	a1, 0x48(a0)
	li  	a2, 0x500 ##800Mhz
	or  	a1, a1, a2 ##
	sw  	a1, 0x48(a0)

	//set 8 bit HT HOST
	lw  	a1, 0x44(a0)
	li  	a2, 0x88ffffff        ##8bit mode
	and 	a1, a1, a2            ##set to 8 bit mode
	sw  	a1, 0x44(a0)

###################### HT_HI@CPU3
	dli	a0, 0x90003dfdfb000000
	//set 800 Mhz HT HOST
	lw  	a1, 0x48(a0)
	li  	a2, 0x500 ##800Mhz
	or  	a1, a1, a2  
	sw  	a1, 0x48(a0)

	//set 8 bit HT HOST
	lw  	a1, 0x44(a0)
	li  	a2, 0x88ffffff        ##8bit mode
	and 	a1, a1, a2            ##set to 8 bit mode
	sw  	a1, 0x44(a0)

###################### HT_LO@CPU1
	dli 	a0, 0x90001cfdfb000000
	//set 800 Mhz HT HOST
	lw  	a1, 0x48(a0)
	li  	a2, 0x500 ##800Mhz
	or  	a1, a1, a2 ##
	sw  	a1, 0x48(a0)

	//set 8 bit HT HOST
	lw  	a1, 0x44(a0)
	li  	a2, 0x88ffffff        ##8bit mode
	and 	a1, a1, a2            ##set to 8 bit mode
	sw  	a1, 0x44(a0)

###################### HT_HI@CPU2
	dli	a0, 0x90002dfdfb000000
	//set 800 Mhz HT HOST
	lw  	a1, 0x48(a0)
	li  	a2, 0x500 ##800Mhz
	or  	a1, a1, a2  
	sw  	a1, 0x48(a0)

	//set 8 bit HT HOST
	lw  	a1, 0x44(a0)
	li  	a2, 0x88ffffff        ##8bit mode
	and 	a1, a1, a2            ##set to 8 bit mode
	sw  	a1, 0x44(a0)

###################### HT_LO@CPU3
	dli 	a0, 0x90003cfdfb000000
	//set 800 Mhz HT HOST
	lw  	a1, 0x48(a0)
	li  	a2, 0x500 ##800Mhz
	or  	a1, a1, a2 ##
	sw  	a1, 0x48(a0)

	//set 8 bit HT HOST
	lw  	a1, 0x44(a0)
	li  	a2, 0x88ffffff        ##8bit mode
	and 	a1, a1, a2            ##set to 8 bit mode
	sw  	a1, 0x44(a0)

#else

	TTYDBG("HT3.0 used \r\n")
#if 0
	TTYDBG("Shut down CPU1\r\n")
	dli	a0, 0x900010001fe001d0
	li	a1, 0x0
	sw	a1, 0x0(a0)

	TTYDBG("Shut down CPU2\r\n")
	dli	a0, 0x900020001fe001d0
	li	a1, 0x0
	sw	a1, 0x0(a0)

	TTYDBG("Shut down CPU3\r\n")
	dli	a0, 0x900030001fe001d0
	li	a1, 0x0
	sw	a1, 0x0(a0)
#endif
#####HT3.0 reconnection

###################### HT_HI@CPU1
	dli 	a0, 0x90001dfdfb000000
	//set 8 bit HT HOST
	lw  	a1, 0x44(a0)
	li  	a2, 0x88ffffff        ##8bit mode
	and 	a1, a1, a2            ##set to 8 bit mode
	li  	a2, 0x11000000        ##16bit
	sw  a1, 0x44(a0)

###################### HT_LO@CPU0
	dli 	a0, 0x90000cfdfb000000
	//set 8 bit HT HOST
	lw  	a1, 0x44(a0)
	li  	a2, 0x88ffffff        ##8bit mode
	and 	a1, a1, a2            ##set to 8 bit mode
	li  	a2, 0x11000000        ##16bit
	sw  	a1, 0x44(a0)

###################### HT_HI@CPU0
	dli 	a0, 0x90000dfdfb000000
	//set 8 bit HT HOST
	lw  	a1, 0x44(a0)
	li  	a2, 0x88ffffff        ##8bit mode
	and 	a1, a1, a2            ##set to 8 bit mode
	li  	a2, 0x11000000        ##16bit
	sw  	a1, 0x44(a0)

###################### HT_LO@CPU2
	dli 	a0, 0x90002cfdfb000000
	//set 8 bit HT HOST
	lw  	a1, 0x44(a0)
	li  	a2, 0x88ffffff        ##8bit mode
	and 	a1, a1, a2            ##set to 8 bit mode
	li  	a2, 0x11000000        ##16bit
	sw  	a1, 0x44(a0)

###################### HT_HI@CPU3
	dli 	a0, 0x90003dfdfb000000
	//set 8 bit HT HOST
	lw  	a1, 0x44(a0)
	li  	a2, 0x88ffffff        ##8bit mode
	and 	a1, a1, a2            ##set to 8 bit mode
	li  	a2, 0x11000000        ##16bit
	sw  	a1, 0x44(a0)

###################### HT_LO@CPU1
	dli 	a0, 0x90001cfdfb000000
	//set 8 bit HT HOST
	lw  	a1, 0x44(a0)
	li  	a2, 0x88ffffff        ##8bit mode
	and 	a1, a1, a2            ##set to 8 bit mode
	li  	a2, 0x11000000        ##16bit
	sw  	a1, 0x44(a0)

###################### HT_HI@CPU2
	dli 	a0, 0x90002dfdfb000000
	//set 8 bit HT HOST
	lw  	a1, 0x44(a0)
	li  	a2, 0x88ffffff        ##8bit mode
	and 	a1, a1, a2            ##set to 8 bit mode
	li  	a2, 0x11000000        ##16bit
	sw  	a1, 0x44(a0)

###################### HT_LO@CPU3
	dli 	a0, 0x90003cfdfb000000
	//set 8 bit HT HOST
	lw  	a1, 0x44(a0)
	li  	a2, 0x88ffffff        ##8bit mode
	and 	a1, a1, a2            ##set to 8 bit mode
	li  	a2, 0x11000000        ##16bit
	sw  	a1, 0x44(a0)


#########TEST CLKSEL[15]
	li	t2,0xbfe00194
	lw	t1, 0x0(t2)
	andi	t1, 0x8000
	bnez	t1, no_softconfig_ht
	nop


#ifdef HT0_2400M
	TTYDBG("Setting CPU0/1/2/3 HyperTransport Controller to be soft config 2400\r\n")
	li	t0, 0x00466083
#else
#ifdef HT0_2200M
	TTYDBG("Setting CPU0/1/2/3 HyperTransport Controller to be soft config 2200\r\n")
	li	t0, 0x00465883
#else
#ifdef HT0_2000M
	TTYDBG("Setting CPU0/1/2/3 HyperTransport Controller to be soft config 2000\r\n")
	li	t0, 0x00465083
#else
#ifdef HT0_1800M
	TTYDBG("Setting CPU0/1/2/3 HyperTransport Controller to be soft config 1800\r\n")
	li	t0, 0x00464883
#else
#ifdef HT0_1600M
	TTYDBG("Setting CPU0/1/2/3 HyperTransport Controller to be soft config 1600\r\n")
	li	t0, 0x00464083
#else
#ifdef HT0_1200M
	TTYDBG("Setting CPU0/1/2/3 HyperTransport Controller to be soft config 1200\r\n")
	li	t0, 0x00463083
#else
#ifdef HT0_200M
	TTYDBG("Setting CPU0/1/2/3 HyperTransport Controller to be soft config 200\r\n")
	li	t0, 0x00460883
#endif
#endif
#endif
#endif
#endif
#endif
#endif

	dli	t2, 0x90000cfdfb000000
	sw 	t0, 0x178(t2)
	dli	t2, 0x90001dfdfb000000
	sw 	t0, 0x178(t2)
	dli	t2, 0x90002cfdfb000000
	sw 	t0, 0x178(t2)
	dli	t2, 0x90000dfdfb000000
	sw 	t0, 0x178(t2)
	dli	t2, 0x90001cfdfb000000
	sw 	t0, 0x178(t2)
	dli	t2, 0x90003dfdfb000000
	sw 	t0, 0x178(t2)
	dli	t2, 0x90003cfdfb000000
	sw 	t0, 0x178(t2)
	dli	t2, 0x90002dfdfb000000
	sw 	t0, 0x178(t2)

	dli	t2, 0x90000cfdfb000000
	lw 	a0, 0x178(t2)
	bal	hexserial
	nop
	TTYDBG("\r\n")

	dli	t2, 0x90001dfdfb000000
	lw 	a0, 0x178(t2)
	bal	hexserial
	nop
	TTYDBG("\r\n")

	dli	t2, 0x90002cfdfb000000
	lw 	a0, 0x178(t2)
	bal	hexserial
	nop
	TTYDBG("\r\n")

	dli	t2, 0x90000dfdfb000000
	lw 	a0, 0x178(t2)
	bal	hexserial
	nop
	TTYDBG("\r\n")

	dli	t2, 0x90001cfdfb000000
	lw 	a0, 0x178(t2)
	bal	hexserial
	nop
	TTYDBG("\r\n")

	dli	t2, 0x90003dfdfb000000
	lw 	a0, 0x178(t2)
	bal	hexserial
	nop
	TTYDBG("\r\n")

	dli	t2, 0x90003cfdfb000000
	lw 	a0, 0x178(t2)
	bal	hexserial
	nop
	TTYDBG("\r\n")

	dli	t2, 0x90002dfdfb000000
	lw 	a0, 0x178(t2)
	bal	hexserial
	nop
	TTYDBG("\r\n")


no_softconfig_ht:

#ifdef HT0_3200M
	TTYDBG("Setting CPU0/1 HyperTransport Controller to be 3200Mhz\r\n")
	li	t0, 0xf //Frequency: 2400 Mhz
#else
#ifdef HT0_2400M
	TTYDBG("Setting CPU0/1 HyperTransport Controller to be 2400Mhz\r\n")
	li	t0, 0xd //Frequency: 2400 Mhz
#else
#ifdef HT0_2200M
	TTYDBG("Setting CPU0/1 HyperTransport Controller to be 2200Mhz\r\n")
	li	t0, 0xc //Frequency: 2200 Mhz
#else
#ifdef HT0_2000M
	TTYDBG("Setting CPU0/1 HyperTransport Controller to be 2000Mhz\r\n")
	li	t0, 0xb //Frequency: 2000 Mhz
#else
#ifdef HT0_1800M
	TTYDBG("Setting CPU0/1 HyperTransport Controller to be 1800Mhz\r\n")
	li	t0, 0xa //Frequency: 1800 Mhz
#else
#ifdef HT0_1600M
	TTYDBG("Setting CPU0/1 HyperTransport Controller to be 1600Mhz\r\n")
	li	t0, 0x9 //Frequency: 1600 Mhz
#else
#ifdef HT0_1200M
	TTYDBG("Setting CPU0/1 HyperTransport Controller to be 1200Mhz\r\n")
	li	t0, 0x7 //Frequency: 1200 Mhz
#else
#ifdef HT0_200M
	TTYDBG("Setting CPU0/1 HyperTransport Controller to be 200Mhz\r\n")
	#li	t0, 0x2 //Frequency: 400 Mhz
	#li	t0, 0x0 //Frequency: 1200 Mhz
	li	t0, 0x1 //Frequency: 200 Mhz
#endif
#endif
#endif
#endif
#endif
#endif
#endif
#endif

	dli	t2, 0x90000cfdfb000000
	sb 	t0, 0x49(t2)
	dli	t2, 0x90001dfdfb000000
	sb 	t0, 0x49(t2)
	dli	t2, 0x90002cfdfb000000
	sb 	t0, 0x49(t2)
	dli	t2, 0x90000dfdfb000000
	sb 	t0, 0x49(t2)
	dli	t2, 0x90001cfdfb000000
	sb 	t0, 0x49(t2)
	dli	t2, 0x90003dfdfb000000
	sb 	t0, 0x49(t2)
	dli	t2, 0x90003cfdfb000000
	sb 	t0, 0x49(t2)
	dli	t2, 0x90002dfdfb000000
	sb 	t0, 0x49(t2)

	dli	t2, 0x90000cfdfb000000
	lw 	a0, 0x48(t2)
	bal	hexserial
	nop
	TTYDBG("\r\n")
	dli	t2, 0x90001dfdfb000000
	lw 	a0, 0x48(t2)
	bal	hexserial
	nop
	TTYDBG("\r\n")
	dli	t2, 0x90002cfdfb000000
	lw 	a0, 0x48(t2)
	bal	hexserial
	nop
	TTYDBG("\r\n")
	dli	t2, 0x90000dfdfb000000
	lw 	a0, 0x48(t2)
	bal	hexserial
	nop
	TTYDBG("\r\n")
	dli	t2, 0x90001cfdfb000000
	lw 	a0, 0x48(t2)
	bal	hexserial
	nop
	TTYDBG("\r\n")
	dli	t2, 0x90003dfdfb000000
	lw 	a0, 0x48(t2)
	bal	hexserial
	nop
	TTYDBG("\r\n")
	dli	t2, 0x90003cfdfb000000
	lw 	a0, 0x48(t2)
	bal	hexserial
	nop
	TTYDBG("\r\n")
	dli	t2, 0x90002dfdfb000000
	lw 	a0, 0x48(t2)
	bal	hexserial
	nop
	TTYDBG("\r\n")


config_others:

/*
	dli	    t2, 0x90000cfdfb000000
	li          t1, 0x30
	lb          a0, 0x17f(t2)
	or          a0, a0, t1
	sb	    t0, 0x17f(t2)
	lw          a0, 0x17c(t2)
	bal	    hexserial
	nop
	TTYDBG("\r\n")
	dli	    t2, 0x90001cfdfb000000
	li          t1, 0x30
	lb          a0, 0x17f(t2)
	or          a0, a0, t1
	sb	    t0, 0x17f(t2)
	lw          a0, 0x17c(t2)
	bal	    hexserial
	nop
	TTYDBG("\r\n")


	dli	    t2, 0x90000cfdfb000000
	li	    t0, 0x8fffffff
	sw	    t0, 0x134(t2)
	lw          a0, 0x134(t2)
	bal	    hexserial
	nop
	TTYDBG("\r\n")
	dli	    t2, 0x90000cfdfb000000
	li	    t0, 0xffffffff
	sw	    t0, 0x138(t2)
	lw          a0, 0x138(t2)
	bal	    hexserial
	nop
	TTYDBG("\r\n")
	dli	    t2, 0x90000cfdfb000000
	li	    t0, 0xffffffff
	sw	    t0, 0x13c(t2)
	lw          a0, 0x13c(t2)
	bal	    hexserial
	nop
	TTYDBG("\r\n")
	dli	    t2, 0x90000cfdfb000000
	li	    t0, 0xffffffff
	sw	    t0, 0x140(t2)
	lw          a0, 0x140(t2)
	bal	    hexserial
	nop
	TTYDBG("\r\n")
	dli	    t2, 0x90000cfdfb000000
	li	    t0, 0xffffffff
	sw	    t0, 0x144(t2)
	lw          a0, 0x144(t2)
	bal	    hexserial
	nop
	TTYDBG("\r\n")

	dli	    t2, 0x90001cfdfb000000
	li	    t0, 0x8fffffff
	sw	    t0, 0x134(t2)
	lw      a0, 0x134(t2)
	bal	    hexserial
	nop
	TTYDBG("\r\n")
	dli	    t2, 0x90001cfdfb000000
	li	    t0, 0xffffffff
	sw	    t0, 0x138(t2)
	lw      a0, 0x138(t2)
	bal	    hexserial
	nop
	TTYDBG("\r\n")
	dli	    t2, 0x90001cfdfb000000
	li	    t0, 0xffffffff
	sw	    t0, 0x13c(t2)
	lw          a0, 0x13c(t2)
	bal	    hexserial
	nop
	TTYDBG("\r\n")
	dli	    t2, 0x90001cfdfb000000
	li	    t0, 0xffffffff
	sw	    t0, 0x140(t2)
	lw          a0, 0x140(t2)
	bal	    hexserial
	nop
	TTYDBG("\r\n")
	dli	    t2, 0x90001cfdfb000000
	li	    t0, 0xffffffff
	sw	    t0, 0x144(t2)
	lw          a0, 0x144(t2)
	bal	    hexserial
	nop
	TTYDBG("\r\n")
*/


/*
	TTYDBG("Setting CPU0 HyperTransport Controller to skip cdr lock\r\n")
	dli	    t2, 0x90000cfdfb000000
	li	    t0, 0xff
	sw	    t0, 0x180(t2)
	lw      a0, 0x180(t2)
	bal	    hexserial
	nop
	TTYDBG("\r\n")

	TTYDBG("Setting CPU1 HyperTransport Controller to skip cdr lock\r\n")
	dli	    t2, 0x90001cfdfb000000
	li	    t0, 0xff
	sw	    t0, 0x180(t2)
	lw      a0, 0x180(t2)
	bal	    hexserial
	nop
	TTYDBG("\r\n")
*/


	TTYDBG("Setting CPU0 LO HyperTransport Controller to be GEN3 mode\r\n")
	dli	t2, 0x90000cfdfb000000
	li 	t0, 0x88600000
	sw 	t0, 0x110(t2)
	lw      a0, 0x110(t2)
	bal	hexserial
	nop
	TTYDBG("\r\n")

	TTYDBG("Setting CPU0 LO HyperTransport Controller to be retry mode\r\n")
	dli	t2, 0x90000cfdfb000000
	li 	t0, 0x81
	sb 	t0, 0x118(t2)
	lw      a0, 0x118(t2)
	bal	hexserial
	nop
	TTYDBG("\r\n")

	TTYDBG("Enable CPU0 LO HyperTransport Controller scrambling\r\n")
	dli	t2, 0x90000cfdfb000000
	li 	t0, 0x78
	sb 	t0, 0x130(t2)
	lw      a0, 0x130(t2)
	bal	hexserial
	nop
	TTYDBG("\r\n")

	TTYDBG("Setting CPU1 HI HyperTransport Controller to be GEN3 mode\r\n")
	dli	t2, 0x90001dfdfb000000
	li 	t0, 0x88600000
	sw 	t0, 0x110(t2)
	lw      a0, 0x110(t2)
	bal	hexserial
	nop
	TTYDBG("\r\n")

	TTYDBG("Setting CPU1 HI HyperTransport Controller to be retry mode\r\n")
	dli	t2, 0x90001dfdfb000000
	li 	t0, 0x81
	sb 	t0, 0x118(t2)
	lw      a0, 0x118(t2)
	bal	hexserial
	nop
	TTYDBG("\r\n")

	TTYDBG("Enable CPU1 HI HyperTransport Controller scrambling\r\n")
	dli	t2, 0x90001dfdfb000000
	li 	t0, 0x78
	sb 	t0, 0x130(t2)
	lw      a0, 0x130(t2)
	bal	hexserial
	nop
	TTYDBG("\r\n")

	TTYDBG("Setting CPU2 LO HyperTransport Controller to be GEN3 mode\r\n")
	dli	t2, 0x90002cfdfb000000
	li 	t0, 0x88600000
	sw 	t0, 0x110(t2)
	lw      a0, 0x110(t2)
	bal	hexserial
	nop
	TTYDBG("\r\n")

	TTYDBG("Setting CPU2 LO HyperTransport Controller to be retry mode\r\n")
	dli	t2, 0x90002cfdfb000000
	li 	t0, 0x81
	sb 	t0, 0x118(t2)
	lw      a0, 0x118(t2)
	bal	hexserial
	nop
	TTYDBG("\r\n")

	TTYDBG("Enable CPU2 LO HyperTransport Controller scrambling\r\n")
	dli	t2, 0x90002cfdfb000000
	li 	t0, 0x78
	sb 	t0, 0x130(t2)
	lw      a0, 0x130(t2)
	bal	hexserial
	nop
	TTYDBG("\r\n")

	TTYDBG("Setting CPU0 HI HyperTransport Controller to be GEN3 mode\r\n")
	dli	t2, 0x90000dfdfb000000
	li 	t0, 0x88600000
	sw 	t0, 0x110(t2)
	lw      a0, 0x110(t2)
	bal	hexserial
	nop
	TTYDBG("\r\n")

	TTYDBG("Setting CPU0 HI HyperTransport Controller to be retry mode\r\n")
	dli	t2, 0x90000dfdfb000000
	li 	t0, 0x81
	sb 	t0, 0x118(t2)
	lw      a0, 0x118(t2)
	bal	hexserial
	nop
	TTYDBG("\r\n")

	TTYDBG("Enable CPU0 HI HyperTransport Controller scrambling\r\n")
	dli	t2, 0x90000dfdfb000000
	li 	t0, 0x78
	sb 	t0, 0x130(t2)
	lw      a0, 0x130(t2)
	bal	hexserial
	nop
	TTYDBG("\r\n")

	TTYDBG("Setting CPU1 LO HyperTransport Controller to be GEN3 mode\r\n")
	dli	t2, 0x90001cfdfb000000
	li 	t0, 0x88600000
	sw 	t0, 0x110(t2)
	lw      a0, 0x110(t2)
	bal	hexserial
	nop
	TTYDBG("\r\n")

	TTYDBG("Setting CPU1 LO HyperTransport Controller to be retry mode\r\n")
	dli	t2, 0x90001cfdfb000000
	li 	t0, 0x81
	sb 	t0, 0x118(t2)
	lw      a0, 0x118(t2)
	bal	hexserial
	nop
	TTYDBG("\r\n")

	TTYDBG("Enable CPU1 LO HyperTransport Controller scrambling\r\n")
	dli	t2, 0x90001cfdfb000000
	li 	t0, 0x78
	sb 	t0, 0x130(t2)
	lw      a0, 0x130(t2)
	bal	hexserial
	nop
	TTYDBG("\r\n")

	TTYDBG("Setting CPU3 HI HyperTransport Controller to be GEN3 mode\r\n")
	dli	t2, 0x90003dfdfb000000
	li 	t0, 0x88600000
	sw 	t0, 0x110(t2)
	lw      a0, 0x110(t2)
	bal	hexserial
	nop
	TTYDBG("\r\n")

	TTYDBG("Setting CPU3 HI HyperTransport Controller to be retry mode\r\n")
	dli	t2, 0x90003dfdfb000000
	li 	t0, 0x81
	sb 	t0, 0x118(t2)
	lw      a0, 0x118(t2)
	bal	hexserial
	nop
	TTYDBG("\r\n")

	TTYDBG("Enable CPU3 HI HyperTransport Controller scrambling\r\n")
	dli	t2, 0x90003dfdfb000000
	li 	t0, 0x78
	sb 	t0, 0x130(t2)
	lw      a0, 0x130(t2)
	bal	hexserial
	nop
	TTYDBG("\r\n")

	TTYDBG("Setting CPU3 LO HyperTransport Controller to be GEN3 mode\r\n")
	dli	t2, 0x90003cfdfb000000
	li 	t0, 0x88600000
	sw 	t0, 0x110(t2)
	lw      a0, 0x110(t2)
	bal	hexserial
	nop
	TTYDBG("\r\n")

	TTYDBG("Setting CPU3 LO HyperTransport Controller to be retry mode\r\n")
	dli	t2, 0x90003cfdfb000000
	li 	t0, 0x81
	sb 	t0, 0x118(t2)
	lw      a0, 0x118(t2)
	bal	hexserial
	nop
	TTYDBG("\r\n")

	TTYDBG("Enable CPU3 LO HyperTransport Controller scrambling\r\n")
	dli	t2, 0x90003cfdfb000000
	li 	t0, 0x78
	sb 	t0, 0x130(t2)
	lw      a0, 0x130(t2)
	bal	hexserial
	nop
	TTYDBG("\r\n")

	TTYDBG("Setting CPU2 HI HyperTransport Controller to be GEN3 mode\r\n")
	dli	t2, 0x90002dfdfb000000
	li 	t0, 0x88600000
	sw 	t0, 0x110(t2)
	lw      a0, 0x110(t2)
	bal	hexserial
	nop
	TTYDBG("\r\n")

	TTYDBG("Setting CPU2 HI HyperTransport Controller to be retry mode\r\n")
	dli	t2, 0x90002dfdfb000000
	li 	t0, 0x81
	sb 	t0, 0x118(t2)
	lw      a0, 0x118(t2)
	bal	hexserial
	nop
	TTYDBG("\r\n")

	TTYDBG("Enable CPU2 HI HyperTransport Controller scrambling\r\n")
	dli	t2, 0x90002dfdfb000000
	li 	t0, 0x78
	sb 	t0, 0x130(t2)
	lw      a0, 0x130(t2)
	bal	hexserial
	nop
	TTYDBG("\r\n")
#endif

#ifndef USE_HT_RESET
###################### Disconnect
	dli 	a0, 0x90000cfdfb000000
	//Disconnect HT BUS 
	lw  	a1, 0x50(a0)
	li  	a2, 0x40000000
	or  	a1, a1, a2
	sw  	a1, 0x50(a0)

	dli 	a0, 0x90001cfdfb000000
	//Disconnect HT BUS 
	lw  	a1, 0x50(a0)
	li  	a2, 0x40000000
	or  	a1, a1, a2
	sw  	a1, 0x50(a0)

	dli 	a0, 0x90002cfdfb000000
	//Disconnect HT BUS 
	lw  	a1, 0x50(a0)
	li  	a2, 0x40000000
	or  	a1, a1, a2
	sw  	a1, 0x50(a0)

	dli 	a0, 0x90003cfdfb000000
	//Disconnect HT BUS 
	lw  	a1, 0x50(a0)
	li  	a2, 0x40000000
	or  	a1, a1, a2
	sw  	a1, 0x50(a0)

#else
#if 1//reset 0-1 HT
reset_ht0:
	TTYDBG("Reset CPU0 LO HyperTransport bus\r\n")
	dli	t0, 0x90000cfdfb000000
	lb      a0, 0x3e(t0)
	li      a1, 0x40
	or      a0, a0, a1
	sb      a0, 0x3e(t0)
	lw      a0, 0x3c(t0)
	bal	hexserial
	nop
	TTYDBG("\r\n")
#endif


#if 1//wait until CPU0 HT link down
	TTYDBG("Waiting CPU0 HyperTransport bus to be down.")
	dli     t0, 0x90000cfdfb000000
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
	li	    t1, 0x1f

3:
	lw      a0, 0x44(t0)
	li	a1, 0x20
	and	a0, a0, a1

	bnez	a0, 1b
	nop

	TTYDBG("\r\n")
	lw      a0, 0x44(t0)
	bal	    hexserial
	nop
	TTYDBG("\r\n")
#endif

#if 1//release cpu0 HT
	TTYDBG("Release HyperTransport bus\r\n")
	dli	t0, 0x90000cfdfb000000
	lb      a0, 0x3e(t0)
	li      a1, 0xbf
	and     a0, a0, a1
	sb      a0, 0x3e(t0)
	lw      a0, 0x3c(t0)
	bal	hexserial
	nop
	TTYDBG("\r\n")
#endif

#if 1//wait until CPU0 HT link up
	TTYDBG("Waiting CPU0 HyperTransport bus to be up.")
	dli     t0, 0x90000cfdfb000000
#ifdef RESET_AGAIN_WHEN_FAIL
	li	t2, 0x3f
#endif
	li	t1, 0x1f
1:
#ifdef RESET_AGAIN_WHEN_FAIL
	beqz	t2, reset_ht0
	nop
#endif
	lw      a0, 0x11c(t0)
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
#ifdef RESET_AGAIN_WHEN_FAIL
	addi	t2, t2, -1
#endif

3:
	lw      a0, 0x44(t0)
	li	a1, 0x20
	and	a0, a0, a1

	beqz	a0, 1b
	nop

	TTYDBG("\r\n")
	lw      a0, 0x44(t0)
	bal	hexserial
	nop
	TTYDBG("\r\n")
#endif

#if 1//wait until CPU1 HT link up
	TTYDBG("Waiting CPU1 HI HyperTransport bus to be up.")
	dli     t0, 0x90001dfdfb000000
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
	li	    t1, 0x1f

3:
	lw      a0, 0x44(t0)
	li	a1, 0x20
	and	a0, a0, a1

	beqz	a0, 1b
	nop

	TTYDBG("\r\n")
	lw      a0, 0x44(t0)
	bal	hexserial
	nop
	TTYDBG("\r\n")
#endif

#if 1//reset 0-2 HT
reset_ht1:
	TTYDBG("Reset CPU0 HI HyperTransport bus\r\n")
	dli	t0, 0x90000dfdfb000000
	lb      a0, 0x3e(t0)
	li      a1, 0x40
	or      a0, a0, a1
	sb      a0, 0x3e(t0)
	lw      a0, 0x3c(t0)
	bal	hexserial
	nop
	TTYDBG("\r\n")
#endif


#if 1//wait until CPU0 HT link down
	TTYDBG("Waiting CPU0 HyperTransport bus to be down.")
	dli     t0, 0x90000dfdfb000000
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

#if 1//release cpu0 HT
	TTYDBG("Release HyperTransport bus\r\n")
	dli	t0, 0x90000dfdfb000000
	lb      a0, 0x3e(t0)
	li      a1, 0xbf
	and     a0, a0, a1
	sb      a0, 0x3e(t0)
	lw      a0, 0x3c(t0)
	bal	hexserial
	nop
	TTYDBG("\r\n")
#endif

#if 1//wait until CPU0 HT link up
	TTYDBG("Waiting CPU0 HI HyperTransport bus to be up.")
	dli     t0, 0x90000dfdfb000000
#ifdef RESET_AGAIN_WHEN_FAIL
	li	t2, 0x3f
#endif
	li	t1, 0x1f
1:
#ifdef RESET_AGAIN_WHEN_FAIL
	beqz	t2, reset_ht1
	nop
#endif
	lw      a0, 0x11c(t0)
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
#ifdef RESET_AGAIN_WHEN_FAIL
	addi	t2, t2, -1
#endif

3:
	lw      a0, 0x44(t0)
	li	a1, 0x20
	and	a0, a0, a1

	beqz	a0, 1b
	nop

	TTYDBG("\r\n")
	lw      a0, 0x44(t0)
	bal	hexserial
	nop
	TTYDBG("\r\n")
#endif

#if 1//wait until CPU1 HT link up
	TTYDBG("Waiting CPU2 LO HyperTransport bus to be up.")
	dli     t0, 0x90002cfdfb000000
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

	beqz	a0, 1b
	nop

	TTYDBG("\r\n")
	lw      a0, 0x44(t0)
	bal	hexserial
	nop
	TTYDBG("\r\n")
#endif

#if 1//reset 1-3 HT
reset_ht2:
	TTYDBG("Reset CPU1 LO HyperTransport bus\r\n")
	dli	t0, 0x90001cfdfb000000
	lb      a0, 0x3e(t0)
	li      a1, 0x40
	or      a0, a0, a1
	sb      a0, 0x3e(t0)
	lw      a0, 0x3c(t0)
	bal	hexserial
	nop
	TTYDBG("\r\n")
#endif


#if 1//wait until CPU0 HT link down
	TTYDBG("Waiting CPU0 HyperTransport bus to be down.")
	dli     t0, 0x90001cfdfb000000
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

#if 1//release cpu0 HT
	TTYDBG("Release HyperTransport bus\r\n")
	dli	t0, 0x90001cfdfb000000
	lb      a0, 0x3e(t0)
	li      a1, 0xbf
	and     a0, a0, a1
	sb      a0, 0x3e(t0)
	lw      a0, 0x3c(t0)
	bal	hexserial
	nop
	TTYDBG("\r\n")
#endif

#if 1//wait until CPU0 HT link up
	TTYDBG("Waiting CPU1 LO HyperTransport bus to be up.")
	dli     t0, 0x90001cfdfb000000
#ifdef RESET_AGAIN_WHEN_FAIL
	li	t2, 0x3f
#endif
	li	t1, 0x1f
1:
#ifdef RESET_AGAIN_WHEN_FAIL
	beqz	t2, reset_ht2
	nop
#endif
	lw      a0, 0x11c(t0)
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
#ifdef RESET_AGAIN_WHEN_FAIL
	addi	t2, t2, -1
#endif

3:
	lw      a0, 0x44(t0)
	li	a1, 0x20
	and	a0, a0, a1

	beqz	a0, 1b
	nop

	TTYDBG("\r\n")
	lw      a0, 0x44(t0)
	bal	hexserial
	nop
	TTYDBG("\r\n")
#endif

#if 1//wait until CPU1 HT link up
	TTYDBG("Waiting CPU3 HI HyperTransport bus to be up.")
	dli     t0, 0x90003dfdfb000000
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

	beqz	a0, 1b
	nop

	TTYDBG("\r\n")
	lw      a0, 0x44(t0)
	bal	hexserial
	nop
	TTYDBG("\r\n")
#endif

#if 1//reset 2-3 HT
reset_ht3:
	TTYDBG("Reset CPU2 HI HyperTransport bus\r\n")
	dli	t0, 0x90002dfdfb000000
	lb      a0, 0x3e(t0)
	li      a1, 0x40
	or      a0, a0, a1
	sb      a0, 0x3e(t0)
	lw      a0, 0x3c(t0)
	bal	hexserial
	nop
	TTYDBG("\r\n")
#endif


#if 1//wait until CPU0 HT link down
	TTYDBG("Waiting CPU2 HyperTransport bus to be down.")
	dli     t0, 0x90002dfdfb000000
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

#if 1//release cpu0 HT
	TTYDBG("Release HyperTransport bus\r\n")
	dli	t0, 0x90002dfdfb000000
	lb      a0, 0x3e(t0)
	li      a1, 0xbf
	and     a0, a0, a1
	sb      a0, 0x3e(t0)
	lw      a0, 0x3c(t0)
	bal	hexserial
	nop
	TTYDBG("\r\n")
#endif

#if 1//wait until CPU0 HT link up
	TTYDBG("Waiting CPU2 HI HyperTransport bus to be up.")
	dli     t0, 0x90002dfdfb000000
#ifdef RESET_AGAIN_WHEN_FAIL
	li	t2, 0x3f
#endif
	li	t1, 0x1f
1:
#ifdef RESET_AGAIN_WHEN_FAIL
	beqz	t2, reset_ht3
	nop
#endif
	lw      a0, 0x11c(t0)
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
#ifdef RESET_AGAIN_WHEN_FAIL
	addi	t2, t2, -1
#endif

3:
	lw      a0, 0x44(t0)
	li	a1, 0x20
	and	a0, a0, a1

	beqz	a0, 1b
	nop

	TTYDBG("\r\n")
	lw      a0, 0x44(t0)
	bal	hexserial
	nop
	TTYDBG("\r\n")
#endif

#if 1//wait until CPU1 HT link up
	TTYDBG("Waiting CPU3 LO HyperTransport bus to be up.")
	dli     t0, 0x90003cfdfb000000
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

	beqz	a0, 1b
	nop

	TTYDBG("\r\n")
	lw      a0, 0x44(t0)
	bal	hexserial
	nop
	TTYDBG("\r\n")
#endif

#if 0
//////Enable others
	TTYDBG("Enable CPU1\r\n")
	dli	a0, 0x900010001fe001d0
	li	a1, 0xffffffff
	sw	a1, 0x0(a0)
	TTYDBG("Enable CPU2\r\n")
	dli	a0, 0x900020001fe001d0
	li	a1, 0xffffffff
	sw	a1, 0x0(a0)
	TTYDBG("Enable CPU3\r\n")
	dli	a0, 0x900030001fe001d0
	li	a1, 0xffffffff
	sw	a1, 0x0(a0)
#endif
#endif


##################################################
#endif
#endif
	dli 	t2, 0x900000003ff02000
	dli 	t1, 0x900000003ff02800
	TTYDBG("Fix L1xbar illegal access at NODE 0\r\n")
1:

#if 0
####### Unused HT0 port #########################
	dli	t0, 0x00000c0000000000
	sd	t0, 0x28(t2)
	dli	t0, 0x00000c0000000000
	sd	t0, 0x68(t2)
	dli	t0, 0x00000c00000000f7
	sd	t0, 0xa8(t2)

	dli	t0, 0x00000d0000000000
	sd	t0, 0x30(t2)
	dli	t0, 0x00000d0000000000
	sd	t0, 0x70(t2)
	dli	t0, 0x00000c00000000f7
	sd	t0, 0xb0(t2)
#else
	dli	t0, 0x00000c0000000000
	sd	t0, 0x30(t2)
	dli	t0, 0x0000fd0000000000
	sd	t0, 0x70(t2)
	dli	t0, 0x00000c00000000f7
	sd	t0, 0xb0(t2)

	dli	t0, 0x0000000000000000
	sd	t0, 0x38(t2)
	dli	t0, 0x0000300000000000
	sd	t0, 0x78(t2)
	dli	t0, 0x00000000000000f0
	sd	t0, 0xb8(t2)
#endif

	daddiu  t2, t2, 0x100
	bne     t2, t1, 1b
	nop


	dli 	t2, 0x900010003ff02000
	dli 	t1, 0x900010003ff02800
	TTYDBG("Fix L1xbar illegal access at NODE 1\r\n")
1:

#if 0
####### Unused HT0 port #########################
	dli	t0, 0x00001c0000000000
	sd	t0, 0x28(t2)
	dli	t0, 0x00001c0000000000
	sd	t0, 0x68(t2)
	dli	t0, 0x00001c00000000f7
	sd	t0, 0xa8(t2)

	dli	t0, 0x00001d0000000000
	sd	t0, 0x30(t2)
	dli	t0, 0x00001d0000000000
	sd	t0, 0x70(t2)
	dli	t0, 0x00001c00000000f7
	sd	t0, 0xb0(t2)
#else
	dli	t0, 0x0000100000000000
	sd	t0, 0x38(t2)
	dli	t0, 0x0000300000000000
	sd	t0, 0x78(t2)
	dli	t0, 0x00001000000000f0
	sd	t0, 0xb8(t2)
#endif

	daddiu  t2, t2, 0x100
	bne     t2, t1, 1b
	nop


	dli 	t2, 0x900020003ff02000
	dli 	t1, 0x900020003ff02800
	TTYDBG("Fix L1xbar illegal access at NODE 2\r\n")
1:

#if 0
####### Unused HT0 port #########################
	dli	t0, 0x00002c0000000000
	sd	t0, 0x28(t2)
	dli	t0, 0x00002c0000000000
	sd	t0, 0x68(t2)
	dli	t0, 0x00002c00000000f7
	sd	t0, 0xa8(t2)

	dli	t0, 0x00002d0000000000
	sd	t0, 0x30(t2)
	dli	t0, 0x00002d0000000000
	sd	t0, 0x70(t2)
	dli	t0, 0x00002c00000000f7
	sd	t0, 0xb0(t2)
#else
	dli	t0, 0x0000200000000000
	sd	t0, 0x38(t2)
	dli	t0, 0x0000300000000000
	sd	t0, 0x78(t2)
	dli	t0, 0x00002000000000f0
	sd	t0, 0xb8(t2)
#endif

	daddiu  t2, t2, 0x100
	bne     t2, t1, 1b
	nop


	dli 	t2, 0x900030003ff02000
	dli 	t1, 0x900030003ff02800
	TTYDBG("Fix L1xbar illegal access at NODE 3\r\n")
1:

#if 0
####### Unused HT0 port #########################
	dli	t0, 0x00003c0000000000
	sd	t0, 0x28(t2)
	dli	t0, 0x00003c0000000000
	sd	t0, 0x68(t2)
	dli	t0, 0x00003c00000000f7
	sd	t0, 0xa8(t2)

	dli	t0, 0x00003c0000000000
	sd	t0, 0x30(t2)
	dli	t0, 0x00003d0000000000
	sd	t0, 0x70(t2)
	dli	t0, 0x00003c00000000f7
	sd	t0, 0xb0(t2)
#else
	dli	t0, 0x0000300000000000
	sd	t0, 0x38(t2)
	dli	t0, 0x0000300000000000
	sd	t0, 0x78(t2)
	dli	t0, 0x00003000000000f0
	sd	t0, 0xb8(t2)
#endif

	daddiu  t2, t2, 0x100
	bne     t2, t1, 1b
	nop



############
	TTYDBG("Fix L2xbar in NODE 0\r\n")
//order cann't be changed.
	dli	t2, 0x900000003ff00000

	dli	t0, 0xfffffffffff00000
	sd	t0, 0x40(t2)

	dli	t0, 0x000000001fc000f2
	sd	t0, 0x80(t2)

	dli	t0, 0x000000001fc00000
	sd	t0, 0x0(t2)

############ 0x10000000 Set to not allow Cache access #######
	dli 	t0, 0x0000000010000000
	sd  	t0, 0x08(t2)
	dli 	t0, 0xfffffffff0000000
	sd  	t0, 0x48(t2)
	dli	t0, 0x0000000010000082
	sd	t0, 0x88(t2)

	sd  	$0, 0x90(t2)

#if 1
//////Enable others
	TTYDBG("Enable CPU1\r\n")
	dli	a0, 0x900010001fe001d0
	li	a1, 0xffffffff
	sw	a1, 0x0(a0)
	TTYDBG("Enable CPU2\r\n")
	dli	a0, 0x900020001fe001d0
	li	a1, 0xffffffff
	sw	a1, 0x0(a0)
	TTYDBG("Enable CPU3\r\n")
	dli	a0, 0x900030001fe001d0
	li	a1, 0xffffffff
	sw	a1, 0x0(a0)
#endif


#ifdef ENABLE_RDinterleave
	TTYDBG("Enable MC read interleave\r\n")

	dli	t2, 0x900000003ff00400
	lw	t1, 0x0(t2)
	li	t0, 0x00000010
	or	t1, t1, t0
	sw	t1, 0x0(t2)
	dli	t2, 0x900010003ff00400
	lw	t1, 0x0(t2)
	li	t0, 0x00000010
	or	t1, t1, t0
	sw	t1, 0x0(t2)
	dli	t2, 0x900020003ff00400
	lw	t1, 0x0(t2)
	li	t0, 0x00000010
	or	t1, t1, t0
	sw	t1, 0x0(t2)
	dli	t2, 0x900030003ff00400
	lw	t1, 0x0(t2)
	li	t0, 0x00000010
	or	t1, t1, t0
	sw	t1, 0x0(t2)
#endif
