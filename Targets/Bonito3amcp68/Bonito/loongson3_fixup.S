/*whd : loongson3_fixup.S
        used to fix up the potential addressing miss
        caused by execute speculated
*/

#define SINGLE_SCACHE

#if 0
#set XBAR to route all the DMA request to Scache0
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

	dli	t0, 0x00000c0000000000
	sd	t0, 0x30(t2)
	dli	t0, 0xfffffe0000000000
	sd	t0, 0x70(t2)
	dli	t0, 0x00000c00000000f7
	sd	t0, 0xb0(t2)

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
	dli	t2, 0x900000003ff00080
	dli	t0, 0x0
	sd	t0, 0x0(t2)

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

#if 0 /* map 0xexxxx to HT1 */
      //map 0x90000e00_00000000
    dli t0, 0x900000003ff02000
    dli t1, 0x00000e0000000000
    sd  t1, 0x30(t0)
    dli t1, 0xffffff0000000000
    sd  t1, 0x70(t0)
    dli t1, 0x00000000000000f7
    sd  t1, 0xb0(t0)

      //map 0x90000e00_00000000
    dli t0, 0x900000003ff02100
    dli t1, 0x00000e0000000000
    sd  t1, 0x30(t0)
    dli t1, 0xffffff0000000000
    sd  t1, 0x70(t0)
    dli t1, 0x00000000000000f7
    sd  t1, 0xb0(t0)

      //map 0x90000e00_00000000
    dli t0, 0x900000003ff02200
    dli t1, 0x00000e0000000000
    sd  t1, 0x30(t0)
    dli t1, 0xffffff0000000000
    sd  t1, 0x70(t0)
    dli t1, 0x00000000000000f7
    sd  t1, 0xb0(t0)

      //map 0x90000e00_00000000
    dli t0, 0x900000003ff02300
    dli t1, 0x00000e0000000000
    sd  t1, 0x30(t0)
    dli t1, 0xffffff0000000000
    sd  t1, 0x70(t0)
    dli t1, 0x00000000000000f7
    sd  t1, 0xb0(t0)


      /* all address using CACHE 1 */

      //map 0x00000000_00000000
    dli t0, 0x900000003ff02000
    dli t1, 0x0000000000000000
    sd  t1, 0x38(t0)
    dli t1, 0x0000000000000000
    sd  t1, 0x78(t0)
    dli t1, 0x00000000000000f1
    sd  t1, 0xb8(t0)
    
      //map 0x00000000_00000000
    dli t0, 0x900000003ff02100
    dli t1, 0x0000000000000000
    sd  t1, 0x38(t0)
    dli t1, 0x0000000000000000
    sd  t1, 0x78(t0)
    dli t1, 0x00000000000000f1
    sd  t1, 0xb8(t0)
    
      //map 0x00000000_00000000
    dli t0, 0x900000003ff02200
    dli t1, 0x0000000000000000
    sd  t1, 0x38(t0)
    dli t1, 0x0000000000000000
    sd  t1, 0x78(t0)
    dli t1, 0x00000000000000f1
    sd  t1, 0xb8(t0)
    
      //map 0x00000000_00000000
    dli t0, 0x900000003ff02300
    dli t1, 0x0000000000000000
    sd  t1, 0x38(t0)
    dli t1, 0x0000000000000000
    sd  t1, 0x78(t0)
    dli t1, 0x00000000000000f1
    sd  t1, 0xb8(t0)

	TTYDBG("HT RX DMA address TRANSLATE to Scache n\r\n")
	dli	t2, 0x900000003ff02700
	dli	t0, 0x0000000000000000
	sd	t0, 0x0(t2)

	dli	t2, 0x900000003ff02740
	dli	t0, 0xffffffff00000000
	sd	t0, 0x0(t2)

	dli	t2, 0x900000003ff02780
	dli	t0, 0xf1
	sd	t0, 0x0(t2)


    sync
    
#endif


/*
//Core 1
	TTYDBG("Fix L1xbar illegal access \r\n")
	dli	t2, 0x900000003ff02128
	dli	t0, 0x00000e0000000000
	sd	t0, 0x0(t2)

	dli	t2, 0x900000003ff02168
	dli	t0, 0xffffff0000000000
	sd	t0, 0x0(t2)

	dli	t2, 0x900000003ff021a8
	dli	t0, 0x87
	sd	t0, 0x0(t2)

	dli	t2, 0x900000003ff02130
	dli	t0, 0x00000e0000000000
	sd	t0, 0x0(t2)

	dli	t2, 0x900000003ff02170
	dli	t0, 0xffffff0000000000
	sd	t0, 0x0(t2)

	dli	t2, 0x900000003ff021b0
	dli	t0, 0xf0
	sd	t0, 0x0(t2)

//Core 2
	TTYDBG("Fix L1xbar illegal access \r\n")
	dli	t2, 0x900000003ff02228
	dli	t0, 0x00000e0000000000
	sd	t0, 0x0(t2)

	dli	t2, 0x900000003ff02268
	dli	t0, 0xffffff0000000000
	sd	t0, 0x0(t2)

	dli	t2, 0x900000003ff022a8
	dli	t0, 0x87
	sd	t0, 0x0(t2)

	dli	t2, 0x900000003ff02230
	dli	t0, 0x00000e0000000000
	sd	t0, 0x0(t2)

	dli	t2, 0x900000003ff02270
	dli	t0, 0xffffff0000000000
	sd	t0, 0x0(t2)

	dli	t2, 0x900000003ff022b0
	dli	t0, 0xf0
	sd	t0, 0x0(t2)

//Core 3
	TTYDBG("Fix L1xbar illegal access \r\n")
	dli	t2, 0x900000003ff02328
	dli	t0, 0x00000e0000000000
	sd	t0, 0x0(t2)

	dli	t2, 0x900000003ff02368
	dli	t0, 0xffffff0000000000
	sd	t0, 0x0(t2)

	dli	t2, 0x900000003ff023a8
	dli	t0, 0x87
	sd	t0, 0x0(t2)

	dli	t2, 0x900000003ff02330
	dli	t0, 0x00000e0000000000
	sd	t0, 0x0(t2)

	dli	t2, 0x900000003ff02370
	dli	t0, 0xffffff0000000000
	sd	t0, 0x0(t2)

	dli	t2, 0x900000003ff023b0
	dli	t0, 0xf0
	sd	t0, 0x0(t2)
    */
