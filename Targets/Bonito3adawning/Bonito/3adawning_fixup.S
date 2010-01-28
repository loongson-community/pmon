
	TTYDBG("cww X2 fixup \r\n")
############ PCI Space
	dli	t2, 0x900000003ff00090
	dli	t0, 0x0000000000000000
	sd	t0, 0x0(t2)

	dli	t2, 0x900000003ff00050
	dli	t0, 0xffffffffc0000000
	sd	t0, 0x0(t2)

	dli	t2, 0x900000003ff00010
	dli	t0, 0x0000000040000000
	sd	t0, 0x0(t2)

	dli	t2, 0x900000003ff00090
	dli	t0, 0x00000000000000f0
	sd	t0, 0x0(t2)


