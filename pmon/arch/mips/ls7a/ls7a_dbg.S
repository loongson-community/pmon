
    //cache DMA
    dli     t0, 0x980000000f000000
    dli     t1, 0x900000000f000000
    dli     t6, 0x5a5aa5a516161616
    sd      t6, 0x0(t0)
    sd      t6, 0x0(t1)

    move    t4, $0
    PRINTSTR("Read local memory..\r\n")
    ld      a1, 0x0(t0)
    beq     t6, a1, 1f
    nop
    PRINTSTR("local cache compare fail!\r\n")
    daddu   t4, t4, 1
1:
    PRINTSTR("Read node0 7A times: 0x10000 ..\r\n")

    dli     t1, 0x90000e000f000000
    dli     t7, 0x10000
2:
    ld      a1, 0x0(t1)
    beq     t6, a1, 1f
    nop
    PRINTSTR("LS7A > node0 compare fail!\r\n")
    daddu   t4, t4, 1
1:
    dsub    t7, t7, 1
    bnez    t7, 2b
    nop

#ifdef  LS7A_2WAY_CONNECT
    PRINTSTR("Read node1 7A ..\r\n")

    dli     t1, 0x90001e000f000000
    ld      a1, 0x0(t1)
    beq     t6, a1, 1f
    nop
    PRINTSTR("node1 > LS7A > node0 compare fail!\r\n")
    daddu   t4, t4, 1
1:
#if 1
    dli     t0, LS7A_CONFBUS_BASE_ADDR
    //store default setting
    lw      t5, CONF_HT_ROUTE_OFFSET(t0)
    // reverse 7A HT routing
    // route node0 dma to 7A HT Hi
    // route node1 dma to 7A HT Lo
    lw      t1, CONF_HT_ROUTE_OFFSET(t0)
    li      t2, (0xf | (0xf << 16))
    not     t2, t2
    and     t1, t1, t2
    li      t2, (0x1 | (0x1 << 16))
    or      t1, t1, t2
    sw      t1, CONF_HT_ROUTE_OFFSET(t0)

    PRINTSTR("Read 7A node1 node0 ..\r\n")
    dli     t1, 0x90000e000f000000
    ld      a1, 0x0(t1)
    beq     t6, a1, 1f
    nop
    PRINTSTR("LS7A>node1>node0 compare fail!\r\n")
    daddu   t4, t4, 1
1:
    // recover default setting
    sw      t5, CONF_HT_ROUTE_OFFSET(t0)
#endif
#endif

    beqz    t4, 8f
    nop
    PRINTSTR("LS3A -- 7A connection test fail!\r\n")
1:
    b       1b
    nop
8:
