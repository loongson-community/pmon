
    dli     t0, 0x900000003ff02000
    daddu   t2, t0, 0x800

1:
 //map HT: PCI IO : 0x90000efd_fc000000 <-- 0x18000000
 //map              0x90000efd_fd000000 <-- 0x19000000
 //map HT: PCI CFG: 0x90000efd_fe000000 <-- 0x1a000000
 //map HT: PCI CFG: 0x90000efd_ff000000 <-- 0x1b000000
    dli t1, 0x0000000018000000
    sd  t1, 0x0(t0)
    dli t1, 0xfffffffffc000000
    sd  t1, 0x40(t0)
    dli t1, 0x00000efdfc0000f7
    sd  t1, 0x80(t0)

 //map HT: PCI MEM : 0x90000e00_10000000 <-- 0x10000000 ~ 0x10ffffff
    dli t1, 0x0000000010000000
    sd  t1, 0x8(t0)
    dli t1, 0xffffffffff000000
    sd  t1, 0x48(t0)
    dli t1, 0x00000e00100000f7
    sd  t1, 0x88(t0)

 //map HT: PCI MEM : 0x90000e00_40000000 <-- 0x40000000 ~ 0x7fffffff
    dli t1, 0x0000000040000000
    sd  t1, 0x10(t0)
    dli t1, 0xffffffffc0000000
    sd  t1, 0x50(t0)
    dli t1, 0x00000e00400000f7
    sd  t1, 0x90(t0)

    daddiu  t0, t0, 0x100
    bne     t0, t2, 1b
    nop

