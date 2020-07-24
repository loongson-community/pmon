#if 1
/*
 * need configure slave device id 0 1 2 3 a b f
 * 0x1e000000 -> 0x90000e0000000000
 */
    dli     t0, 0x900000003ff02000
    daddu   t2, t0, 0xc00

1:
 //map HT: PCI IO : 0x90000e00_00000000 <-- 0x1e000000
    dli t1, 0x000000001e000000
    sd  t1, 0x18(t0)
    dli t1, 0xffffffffff000000
    sd  t1, 0x58(t0)
    dli t1, 0x00000e000000008e
    sd  t1, 0x98(t0)

    daddiu  t0, t0, 0x100
    dli     t3, 0x900000003ff02400
    bne     t0, t3, 2f
    nop
    daddiu  t0, t0, 0x600
2:
    bne     t0, t2, 1b
    nop

    dli t0, 0x900000003ff02000
    dli t1, 0x000000001e000000
    sd  t1, 0xf18(t0)
    dli t1, 0xffffffffff000000
    sd  t1, 0xf58(t0)
    dli t1, 0x00000e000000008e
    sd  t1, 0xf98(t0)
#endif

/*
 * need configure slave device id 0 1 2 3 a b c d e f
 */
 //map HT: PCI IO : 0x90000efd_fc000000 <-- 0x18000000
 //map              0x90000efd_fd000000 <-- 0x19000000
 //map HT: PCI CFG: 0x90000efd_fe000000 <-- 0x1a000000
 //map HT: PCI CFG: 0x90000efd_ff000000 <-- 0x1b000000
 //map HT: PCI MEM : 0x90000e00_10000000 <-- 0x10000000 ~ 0x17ffffff
 //map HT: PCI MEM : 0x90000e00_40000000 <-- 0x40000000 ~ 0x7fffffff
 //map HT: PCI MEM : 0x90001e00_00000000 <-- 0x90000e00_00000000 ~ 0x
#define SET_HT_ADDR_TRANS(node, destnode, destport) \
    dli     t0, 0x900000003ff02000 | node<<44; \
    daddu   t2, t0, 0x1000; \
	\
1:  \
	dli t1, 0x0000000018000000; \
	sd  t1, 0x0(t0); \
	dli t1, 0xfffffffffc000000; \
	sd  t1, 0x40(t0); \
	dli t1, 0x00000efdfc000080 | destnode<<44 | destport; \
	sd  t1, 0x80(t0); \
	\
	dli t1, 0x0000000010000000; \
	sd  t1, 0x8(t0); \
	dli t1, 0xfffffffff8000000; \
	sd  t1, 0x48(t0); \
	dli t1, 0x00000e0010000080 | destnode<<44 | destport; \
	sd  t1, 0x88(t0); \
	\
	dli t1, 0x0000000040000000; \
	sd  t1, 0x10(t0); \
	dli t1, 0xffffffffc0000000; \
	sd  t1, 0x50(t0); \
	dli t1, 0x00000e0040000080 | destnode<<44 | destport; \
	sd  t1, 0x90(t0); \
	\
	dli t1, 0x00000e0000000000; \
	sd  t1, 0x18(t0); \
	dli t1, 0xffffff0000000000; \
	sd  t1, 0x58(t0); \
	dli t1, 0x00000e0000000080 | destnode<<44 | destport; \
	sd  t1, 0x98(t0); \
	\
	daddiu  t0, t0, 0x100; \
	dli     t3, 0x900000003ff02400 | node<<44; \
	bne     t0, t3, 2f; \
	nop; \
	daddiu  t0, t0, 0x600; \
2: \
	bne     t0, t2, 1b; \
	nop;

#ifdef	MULTI_CHIP
// node 1 2/4 way mode
#ifdef	LS7A_2WAY_CONNECT
	SET_HT_ADDR_TRANS(1, 1, 0xe);
#else //!LS7A_2WAY_CONNECT
#ifdef  CHIP_4
	SET_HT_ADDR_TRANS(1, 0, 0xb);
#else
	SET_HT_ADDR_TRANS(1, 0, 0xa);
#endif
#endif

#ifdef CHIP_4
// node 2 4 way mode
	SET_HT_ADDR_TRANS(2, 0, 0xa);
#ifdef	LS7A_2WAY_CONNECT
	SET_HT_ADDR_TRANS(3, 1, 0xb);
#else //!LS7A_2WAY_CONNECT
#ifdef DISABLE_X_LINK
	SET_HT_ADDR_TRANS(3, 0, 0xa);
#else
	SET_HT_ADDR_TRANS(3, 0, 0xf);
#endif //DISABLE_X_LINK
#endif
#endif

#endif
