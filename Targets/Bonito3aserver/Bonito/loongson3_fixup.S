/*whd : loongson3_fixup.S
        used to fix up the potential addressing miss
        caused by speculated execution
*/

    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    sync
    sync
    sync
    sync
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop

#if 0
#set XBAR to route all the DMA request to Scache0
/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
   THIS CONFIGURATION WILL AFFECT the Scache initilization,
   The Scache init MUST BE rewrite when reconfiguared
   !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */
#define SINGLE_SCACHE
#ifdef SINGLE_SCACHE
    dli a0,0xf #using 37:36
#else
    dli a0,0x2 #using 11:10
#endif
    dli t0,0x900000003ff00400
    sd  a0,0x0(t0)

#if 0//config L1 xbar cpu port
    dli t2, 0x900000003ff02000
    dli t1, 0x900000003ff02400
	TTYDBG("Using Scache 3 \r\n")
1:
	dli	t0, 0x0000000000000000
	sd	t0, 0x00(t2)
	dli	t0, 0x0000000000000000
	sd	t0, 0x40(t2)
	dli	t0, 0x00000000000000f3
	sd	t0, 0x80(t2)
#endif

    PRINTSTR("Scache index setup done\r\n")
#endif

#ifdef MULTI_CHIP
#if 1
    TTYDBG("HT0 frequency reconfig \r\n")
###################### HT@CPU1
    dli a0, 0x90001cfdfb000000
    //set 800 Mhz HT HOST
    lw  a1, 0x48(a0)
    li  a2, 0x500 ##800Mhz
    or  a1, a1, a2  
    sw  a1, 0x48(a0)

    //set 8 bit HT HOST
    lw  a1, 0x44(a0)
    li  a2, 0x88ffffff        ##8bit mode
    and a1, a1, a2            ##set to 8 bit mode
    li  a2, 0x11000000        ##16bit
    or  a1, a1, a2
    sw  a1, 0x44(a0)

###################### HT@CPU0
    dli a0, 0x90000cfdfb000000
    //set 800 Mhz HT HOST
    lw  a1, 0x48(a0)
    li  a2, 0x500 ##800Mhz
    or  a1, a1, a2 ##
    sw  a1, 0x48(a0)

    //set 8 bit HT HOST
    lw  a1, 0x44(a0)
    li  a2, 0x88ffffff        ##8bit mode
    and a1, a1, a2            ##set to 8 bit mode
    li  a2, 0x11000000        ##16bit
    or  a1, a1, a2
    sw  a1, 0x44(a0)

###################### Disconnect
    dli a0, 0x90000cfdfb000000
    //Disconnect HT BUS 
    lw  a1, 0x50(a0)
    li  a2, 0x40000000
    or  a1, a1, a2
    sw  a1, 0x50(a0)

##################################################
#endif
#endif


#if 1//config L1 xbar cpu port
    dli t2, 0x900000003ff02000
    dli t1, 0x900000003ff02800
	TTYDBG("Fix L1xbar illegal access at NODE 0\r\n")
1:

####### Unused HT0 port #########################
	dli	t0, 0x00000c0000000000
	sd	t0, 0x28(t2)
	dli	t0, 0x00000c0000000000
	sd	t0, 0x68(t2)
	dli	t0, 0x00000c00000000f7
	sd	t0, 0xa8(t2)

	dli	t0, 0x0000200000000000
	sd	t0, 0x30(t2)
	dli	t0, 0x0000200000000000
	sd	t0, 0x70(t2)
	dli	t0, 0x00002000000000f7
	sd	t0, 0xb0(t2)

	dli	t0, 0x000000fdfe000000
	sd	t0, 0x38(t2)
	dli	t0, 0x000000fffe000000
	sd	t0, 0x78(t2)
	dli	t0, 0x000000fdfe0000f7
	sd	t0, 0xb8(t2)

#ifndef MULTI_CHIP
####### address space to other nodes ############
	dli	t0, 0x0000200000000000
	sd	t0, 0x30(t2)
	dli	t0, 0x0000200000000000
	sd	t0, 0x70(t2)
	dli	t0, 0x00002000000000f7
	sd	t0, 0xb0(t2)

	dli	t0, 0x0000100000000000
	sd	t0, 0x38(t2)
	dli	t0, 0x0000300000000000
	sd	t0, 0x78(t2)
	dli	t0, 0x00001000000000f7
	sd	t0, 0xb8(t2)
#endif

    daddiu  t2, t2, 0x100
    bne     t2, t1, 1b
    nop

#endif

#ifdef  MULTI_CHIP 
    dli t2, 0x900010003ff02000
    dli t1, 0x900010003ff02800
	TTYDBG("Fix L1xbar illegal access at NODE 1\r\n")
1:

####### Unused HT0 port #########################
	dli	t0, 0x00001c0000000000
	sd	t0, 0x28(t2)
	dli	t0, 0x00001c0000000000
	sd	t0, 0x68(t2)
	dli	t0, 0x00001c00000000f7
	sd	t0, 0xa8(t2)

	dli	t0, 0x0000200000000000
	sd	t0, 0x30(t2)
	dli	t0, 0x0000200000000000
	sd	t0, 0x70(t2)
	dli	t0, 0x00002000000000f7
	sd	t0, 0xb0(t2)

	dli	t0, 0x00000efdfe000000
	sd	t0, 0x38(t2)
	dli	t0, 0x0000fffffe000000
	sd	t0, 0x78(t2)
	dli	t0, 0x000000fdfe0000f6
	sd	t0, 0xb8(t2)

    daddiu  t2, t2, 0x100
    bne     t2, t1, 1b
    nop

#endif

#if 1
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
    dli t0, 0x0000000010000000
    sd  t0, 0x08(t2)
    dli t0, 0xfffffffff0000000
    sd  t0, 0x48(t2)
	dli	t0, 0x0000000010000082
	sd	t0, 0x88(t2)

    sd  $0, 0x90(t2)
#endif

#ifdef MULTI_CHIP
	TTYDBG("Fix L2xbar in NODE 1\r\n")
	dli	t2, 0x900010003ff00000
//keep consistent to NODE 0.
//windows 1 used to route 0x10000000~ access
    dli t0, 0x0000100010000000
    sd  t0, 0x08(t2)
    dli t0, 0xfffffffff0000000
    sd  t0, 0x48(t2)
    dli t0, 0x0000000010000082
    sd  t0, 0x88(t2)
//disable window 0
    sd  $0, 0x80(t2)
    sd  $0, 0x90(t2)
#endif

    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    sync
    sync
    sync
    sync
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
