/*whd : loongson3_fixup.S
        used to fix up the potential addressing miss
        caused by speculated execution
*/


#if 0
#set XBAR to route all the DMA request to Scache0
//#define SINGLE_SCACHE
#ifdef SINGLE_SCACHE
        dli       a0,0xf #using 37:36
#else
        dli       a0,0x2 #using 11:10
#endif
        dli       t0,0x900000003ff00400
        sd        a0,0x0(t0)

        PRINTSTR("Scache index setup done\r\n")
#endif


#if 1//config L1 xbar cpu port
    dli t2, 0x900000003ff02000
    dli t1, 0x900000003ff02400
	TTYDBG("Fix L1xbar illegal access \r\n")
1:

####### Unused HT0 port #########################
	dli	t0, 0x00000c0000000000
	sd	t0, 0x28(t2)
	dli	t0, 0xfffffe0000000000
	sd	t0, 0x68(t2)
	dli	t0, 0x00000c00000000f7
	sd	t0, 0xa8(t2)

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

    daddiu  t2, t2, 0x100
    bne     t2, t1, 1b
    nop

#endif


#if 1
############
	TTYDBG("Fix L2xbar illegal access \r\n")
############ PCI Space
	//dli	t2, 0x900000003ff00080
	//dli	t0, 0x0
	//sd	t0, 0x0(t2)

	dli	t2, 0x900000003ff00000
	dli	t0, 0x000000001fc00000
	sd	t0, 0x0(t2)

	dli	t2, 0x900000003ff00040
	dli	t0, 0xfffffffffff00000
	sd	t0, 0x0(t2)

	dli	t2, 0x900000003ff00080
	dli	t0, 0x000000001fc000f2
	sd	t0, 0x0(t2)

############ 0x10000000 Set to not allow Cache access #######
	dli	t2, 0x900000003ff00088
	dli	t0, 0x0000000010000082
	sd	t0, 0x0(t2)


#endif
