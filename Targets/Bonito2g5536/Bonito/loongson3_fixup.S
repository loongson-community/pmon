/*whd : loongson3_fixup.S
        used to fix up the potential addressing miss
        caused by speculated execution
*/


#if 1
#set XBAR to route all the DMA request to Scache0
#define SINGLE_SCACHE
#ifdef SINGLE_SCACHE
    dli a0,0xf #using 37:36
#else
    //dli a0,0x2 #using 11:10
    dli a0,0xb #using 29:28
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


#if 1//config L1 xbar cpu port
    dli t2, 0x900000003ff02000
    dli t1, 0x900000003ff02400
	TTYDBG("Fix L1xbar illegal access \r\n")
1:


#ifndef MULTI_CHIP
####### address space to other nodes ############
//	dli	t0, 0x0000200000000000
//	sd	t0, 0x30(t2)
//	dli	t0, 0x0000200000000000
//	sd	t0, 0x70(t2)
//	dli	t0, 0x00002000000000f7
//	sd	t0, 0xb0(t2)
//
//	dli	t0, 0x0000100000000000
//	sd	t0, 0x38(t2)
//	dli	t0, 0x0000300000000000
//	sd	t0, 0x78(t2)
//	dli	t0, 0x00001000000000f7
//	sd	t0, 0xb8(t2)

	dli	t0, 0x0000000000000000
	sd	t0, 0x38(t2)
	dli	t0, 0x0000000000000000
	sd	t0, 0x78(t2)
	dli	t0, 0x00000000000000f0
	sd	t0, 0xb8(t2)
    daddiu  t2, t2, 0x100
    bne     t2, t1, 1b
    nop
#endif

#endif


