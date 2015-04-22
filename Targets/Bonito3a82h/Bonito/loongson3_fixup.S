/*whd : loongson3_fixup.S
        used to fix up the potential addressing miss
        caused by speculated execution
*/

	sync

	dli	t2, 0x900000003ff02000
	dli	t1, 0x900000003ff02800
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
	dli	t0, 0x0000000010000000
	sd	t0, 0x08(t2)
	dli	t0, 0xfffffffff0000000
	sd	t0, 0x48(t2)
	dli	t0, 0x0000000010000082
	sd	t0, 0x88(t2)

	sd	zero, 0x90(t2)

	sync

