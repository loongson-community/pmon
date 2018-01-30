// 7A chip configure
// author: chenxk
// 2017.8.15
// !notice: t0: used as global variable partial

    dli     t0, LS7A_CONFBUS_BASE_ADDR

#ifdef  LS7A_2WAY_CONNECT
    // support 2 nodes
    // route node0 dma to 7A HT Lo
    // route node1 dma to 7A HT Hi
    lw      t1, CONF_HT_ROUTE_OFFSET(t0)
    li      t2, (0xf | (0xf << 16))
    not     t2, t2
    and     t1, t1, t2
    li      t2, (0x1 | (0xa << 16))
    or      t1, t1, t2
    sw      t1, CONF_HT_ROUTE_OFFSET(t0)
#else
    // support 1 node
    lw      t1, CONF_HT_ROUTE_OFFSET(t0)
    li      t2, (0xf | (0xf << 16))
    not     t2, t2
    and     t1, t1, t2
    sw      t1, CONF_HT_ROUTE_OFFSET(t0)
#endif
    TTYDBG("config 7A dma route done.\r\n")

#ifndef LS7A_2WAY_CONNECT
    //shut down LS7A HT Hi
    lw      t1, CONF_HT_CLKEN_OFFSET(t0)
    li      t2, (0x1 << 1)
    not     t2, t2
    and     t1, t1, t2
    sw      t1, CONF_HT_CLKEN_OFFSET(t0)
    TTYDBG("7A HT Hi clock disabled.\r\n")
#endif

#if 0
    dli     t0, CONFBUS_HEADER_ADDR
    TTYDBG("\r\nLS7A confbus header 0-8:\r\n")
    lw      a0, 0x4(t0)
    bal     hexserial
    nop
    lw      a0, 0x0(t0)
    bal     hexserial
    nop
    TTYDBG("\r\n10-18:")
    lw      a0, 0x14(t0)
    bal     hexserial
    nop
    lw      a0, 0x10(t0)
    bal     hexserial
    nop

    //read confbus
    dli     t0, LS7A_CONFBUS_BASE_ADDR

    TTYDBG("\r\nconfbus 0x420:")
    lw      a0, 0x420(t0)
    bal     hexserial
    nop
    TTYDBG("\r\nconfbus 0x424:")
    lw      a0, 0x424(t0)
    bal     hexserial
    nop

    TTYDBG("\r\nconfbus 0x430:")
    lw      a0, 0x430(t0)
    bal     hexserial
    nop
    TTYDBG("\r\nconfbus 0x434:")
    lw      a0, 0x434(t0)
    bal     hexserial
    nop
#endif

    dli     t0, LS7A_CONFBUS_BASE_ADDR
#ifdef  LS7A_UC_ACC
    lw      t1, (CONF_NB_OFFSET+4)(t0)
    li      t2, (0x3f << 0)
    or      t1, t1, t2
    sw      t1, (CONF_NB_OFFSET+4)(t0)

    lw      t1, (CONF_SB_OFFSET+0)(t0)
    li      t2, (0xff << 24)
    or      t1, t1, t2
    sw      t1, (CONF_SB_OFFSET+0)(t0)
    TTYDBG("LS7A uncache accellerator enabled\r\n")
#endif

#if 1
//configure 7A pll
//LS7A_PLL_VALUE: (LOOPC, DIV2, DIV1, DIV0)

    //pcie, gmac, sata/usb
    daddu   a0, t0, CONF_PLL0_OFFSET
    li      a2, 0x4
    li      a1, LS7A_PLL_VALUE(80, 8, 16, 16)
    bal     ls7a_config_one_pll
    nop
    beqz    v0, 1f
    nop
    TTYDBG("!!!LS7A PLL0 soft configure fail.\r\n")
2:
    b       2b
    nop
1:

    //gpu, gmem, dc
    daddu   a0, t0, CONF_PLL1_OFFSET
    li      a2, 0x4
    li      a1, LS7A_PLL_VALUE(100, 5, 4, 10)
    bal     ls7a_config_one_pll
    nop
    beqz    v0, 1f
    nop
    TTYDBG("!!!LS7A PLL1 soft configure fail.\r\n")
2:
    b       2b
    nop
1:

    //flex, node, hda bitclk
    daddu   a0, t0, CONF_PLL2_OFFSET
    li      a2, 0x4
    li      a1, LS7A_PLL_VALUE(96, 72, 6, 100)
    bal     ls7a_config_one_pll
    nop
    beqz    v0, 1f
    nop
    TTYDBG("!!!LS7A PLL2 soft configure fail.\r\n")
2:
    b       2b
    nop
1:

    daddu   a0, t0, CONF_PLL3_OFFSET
    li      a2, 0x5
    li      a1, LS7A_PLL_VALUE(57, 30, 30, 30)
    bal     ls7a_config_one_pll
    nop
    beqz    v0, 1f
    nop
    TTYDBG("!!!LS7A PLL3 soft configure fail.\r\n")
2:
    b       2b
    nop
1:

    daddu   a0, t0, CONF_PLL4_OFFSET
    li      a2, 0x5
    li      a1, LS7A_PLL_VALUE(57, 30, 30, 30)
    bal     ls7a_config_one_pll
    nop
    beqz    v0, 1f
    nop
    TTYDBG("!!!LS7A PLL4 soft configure fail.\r\n")
2:
    b       2b
    nop
1:

    TTYDBG("LS7A pll configure done.\r\n")
#endif

#if 0   //set to use default routing(debug only)
    lw      t1, CONF_SB_OFFSET(t0)
    dli     t2, (0x1 << CONF_DEFAULT_ROUTE_SB_OFFSET)
    or      t1, t1, t2
    sw      t1, CONF_SB_OFFSET(t0)

    lw      t1, CONF_NB_OFFSET(t0)
    dli     t2, (0x1 << CONF_DEFAULT_ROUTE_NB_OFFSET)
    or      t1, t1, t2
    sw      t1, CONF_NB_OFFSET(t0)
#endif

    //init 7a hardware
#if 1   //configure to obey strict HT order
    lw      t1, 0x414(t0)
    li      t2, (0x7ffff << 0)
    or      t1, t1, t2
    sw      t1, 0x414(t0)
#endif

    //rewrite pci header default value
    or      a2, $0, 0x0100
    dli     t1, 0x0
    dli     t3, 0x480
1:
    daddu   t2, t0, t1
    daddu   t1, 0x40
    sb      $0, 0x300c(t2)  //clear CLS
    sw      a2, 0x303c(t2)  //rewrite INT_LN/PIN
    ble     t1, t3, 1b
    nop

    //rewrite HEADER TYPE to Multi-function
    or      a2, $0, 0x80
    sb      a2, 0x304e(t0)
    sb      a2, 0x308e(t0)

    sb      a2, 0x32ce(t0)
    sb      a2, 0x330e(t0)
    sb      a2, 0x334e(t0)

    //fix LPC class code
    li      a2, 0x06010000
    sw      a2, 0x3448(t0)

    //disable pci scan of MISC and confbus
    lw      a0, 0x3800(t0)
    or      a0, a0, 0xf
    sw      a0, 0x3800(t0)

    lw      a0, 0x3878(t0)
    or      a0, a0, 0xf
    sw      a0, 0x3878(t0)

    not     a0, $0
    sw      a0, 0x3000(t0)
    sw      a0, 0x33c0(t0)

    //disable write to RO bits of device header ---- start
    dli     a0, 0x10
    dli     a2, 0x10000000
    dli     t1, 0x0
    dli     t3, 0x90
1:
    daddu   t2, t0, t1
    daddu   t1, 0x8
    sh      a0, 0x3800(t2)
    sw      a2, 0x3804(t2)
    ble     t1, t3, 1b
    nop

    //special take care of GMEM BAR, clear to 4KB
    //if defined CFG_GMEM, this register will be reconfigured.
    li      a0, 0xfff
    sw      a0, 0x3838(t0)
    sw      $0, 0x383c(t0)
    //special take care of GPU-BAR3(64)
    lw      a0, 0x3844(t0)
    or      a0, 0xff
    sw      a0, 0x3844(t0)
    //special take care of LPC
    lw      a2, 0x388c(t0)
    or      a2, a2, 0xff
    sw      a2, 0x388c(t0)
    //disable write to part of header ---- end

    //change INT and HPET fix address
    li      a1, (INT_BASE_ADDR | 0x4)
    sw      a1, 0x460(t0)
    li      a1, (HPET_BASE_ADDR | 0x4)
    sw      a1, 0x464(t0)

    TTYDBG("LS7A hardware init done.\r\n")
//3. device configure 
    //enable access to device(PCIE/SATA/USB/LPC)
    dli     t0, LS7A_CONFBUS_BASE_ADDR
#ifndef USE_PCIE_PAD_REFCLK
    //power down phy
    lw      t1, 0x588(t0)
    li      t2, (1 << 24)
    or      t1, t1, t2
    sw      t1, 0x588(t0)

    lw      t1, 0x5a8(t0)
    li      t2, (1 << 24)
    or      t1, t1, t2
    sw      t1, 0x5a8(t0)

    lw      t1, 0x5c8(t0)
    li      t2, (1 << 24)
    or      t1, t1, t2
    sw      t1, 0x5c8(t0)

    lw      t1, 0x5e8(t0)
    li      t2, (1 << 24)
    or      t1, t1, t2
    sw      t1, 0x5e8(t0)

    lw      t1, 0x608(t0)
    li      t2, (1 << 24)
    or      t1, t1, t2
    sw      t1, 0x608(t0)

    //assert reset
    lw      t1, CONF_NB_OFFSET(t0)
    li      t2, ((1 << 28) | (1 << 24) | (1 << 20) | (1 << 16) | (1 << 8))
    or      t1, t1, t2
    sw      t1, CONF_NB_OFFSET(t0)

    //switch clk
    lw      t1, CONF_NB_OFFSET(t0)
    li      t2, (1 << 2)
    or      t1, t1, t2
    sw      t1, CONF_NB_OFFSET(t0)

    //delay a while
    dli     t1, 0x4000000
1:
    dsubu   t1, t1, 1
    bnez    t1, 1b
    nop

    //power up phy
    lw      t1, 0x588(t0)
    li      t2, (1 << 24)
    not     t2, t2
    and     t1, t1, t2
    sw      t1, 0x588(t0)

    lw      t1, 0x5a8(t0)
    li      t2, (1 << 24)
    not     t2, t2
    and     t1, t1, t2
    sw      t1, 0x5a8(t0)

    lw      t1, 0x5c8(t0)
    li      t2, (1 << 24)
    not     t2, t2
    and     t1, t1, t2
    sw      t1, 0x5c8(t0)

    lw      t1, 0x5e8(t0)
    li      t2, (1 << 24)
    not     t2, t2
    and     t1, t1, t2
    sw      t1, 0x5e8(t0)

    lw      t1, 0x608(t0)
    li      t2, (1 << 24)
    not     t2, t2
    and     t1, t1, t2
    sw      t1, 0x608(t0)

    //deassert reset
    lw      t1, CONF_NB_OFFSET(t0)
    li      t2, ((1 << 28) | (1 << 24) | (1 << 20) | (1 << 16) | (1 << 8))
    not     t2, t2
    and     t1, t1, t2
    sw      t1, CONF_NB_OFFSET(t0)

    //delay a while
    dli     t1, 0x4000000
1:
    dsubu   t1, t1, 1
    bnez    t1, 1b
    nop
#endif

    //enable access
    lw      t1, CONF_NB_OFFSET(t0)
    li      t2, ((1 << 29) | (1 << 25) | (1 << 21) | (1 << 17) | (1 << 9))
    or      t1, t1, t2
    sw      t1, CONF_NB_OFFSET(t0)
    TTYDBG("PCIE enabled\r\n")

    //re-configure used PHYs, else powerdown it
#if (!(LS7A_PCIE_F0_P0_DISABLE && LS7A_PCIE_F0_P1_DISABLE && LS7A_PCIE_F0_P2_DISABLE && LS7A_PCIE_F0_P3_DISABLE))
//DO not destroy a0, a1, for example, do not add print between these code
    daddu   a0, t0, 0x590
    li      a1, 0x403f1002
    bal     ls7a_phy_cfg_write
    nop
    addu    a1, a1, 0x100
    bal     ls7a_phy_cfg_write
    nop
    addu    a1, a1, 0x100
    bal     ls7a_phy_cfg_write
    nop
    addu    a1, a1, 0x100
    bal     ls7a_phy_cfg_write
    nop
#else
    lw      t1, 0x588(t0)
    li      t2, (1 << 24)
    or      t1, t1, t2
    sw      t1, 0x588(t0)
#endif

#if (!(LS7A_PCIE_F1_P0_DISABLE && LS7A_PCIE_F1_P1_DISABLE))
    daddu   a0, t0, 0x5b0
    li      a1, 0x403f1002
    bal     ls7a_phy_cfg_write
    nop
    addu    a1, a1, 0x100
    bal     ls7a_phy_cfg_write
    nop
    addu    a1, a1, 0x100
    bal     ls7a_phy_cfg_write
    nop
    addu    a1, a1, 0x100
    bal     ls7a_phy_cfg_write
    nop
#else
    lw      t1, 0x5a8(t0)
    li      t2, (1 << 24)
    or      t1, t1, t2
    sw      t1, 0x5a8(t0)
#endif

#if (!(LS7A_PCIE_G0_P0_DISABLE && LS7A_PCIE_G0_P1_DISABLE))
    daddu   a0, t0, 0x5d0
    li      a1, 0x403f1002
    bal     ls7a_phy_cfg_write
    nop
    addu    a1, a1, 0x100
    bal     ls7a_phy_cfg_write
    nop
    addu    a1, a1, 0x100
    bal     ls7a_phy_cfg_write
    nop
    addu    a1, a1, 0x100
    bal     ls7a_phy_cfg_write
    nop

    daddu   a0, t0, 0x5d8
    li      a1, 0x403f1002
    bal     ls7a_phy_cfg_write
    nop
    addu    a1, a1, 0x100
    bal     ls7a_phy_cfg_write
    nop
    addu    a1, a1, 0x100
    bal     ls7a_phy_cfg_write
    nop
    addu    a1, a1, 0x100
    bal     ls7a_phy_cfg_write
    nop
#else
    lw      t1, 0x5e8(t0)
    li      t2, (1 << 24)
    or      t1, t1, t2
    sw      t1, 0x5e8(t0)
#endif

#if (!(LS7A_PCIE_G1_P0_DISABLE && LS7A_PCIE_G1_P1_DISABLE))
    daddu   a0, t0, 0x5f0
    li      a1, 0x403f1002
    bal     ls7a_phy_cfg_write
    nop
    addu    a1, a1, 0x100
    bal     ls7a_phy_cfg_write
    nop
    addu    a1, a1, 0x100
    bal     ls7a_phy_cfg_write
    nop
    addu    a1, a1, 0x100
    bal     ls7a_phy_cfg_write
    nop

    daddu   a0, t0, 0x5f8
    li      a1, 0x403f1002
    bal     ls7a_phy_cfg_write
    nop
    addu    a1, a1, 0x100
    bal     ls7a_phy_cfg_write
    nop
    addu    a1, a1, 0x100
    bal     ls7a_phy_cfg_write
    nop
    addu    a1, a1, 0x100
    bal     ls7a_phy_cfg_write
    nop
#else
    lw      t1, 0x608(t0)
    li      t2, (1 << 24)
    or      t1, t1, t2
    sw      t1, 0x608(t0)
#endif

#if (!(LS7A_PCIE_H_P0_DISABLE && LS7A_PCIE_H_P1_DISABLE))
    daddu   a0, t0, 0x610
    li      a1, 0x403f1002
    bal     ls7a_phy_cfg_write
    nop
    addu    a1, a1, 0x100
    bal     ls7a_phy_cfg_write
    nop
    addu    a1, a1, 0x100
    bal     ls7a_phy_cfg_write
    nop
    addu    a1, a1, 0x100
    bal     ls7a_phy_cfg_write
    nop

    daddu   a0, t0, 0x618
    li      a1, 0x403f1002
    bal     ls7a_phy_cfg_write
    nop
    addu    a1, a1, 0x100
    bal     ls7a_phy_cfg_write
    nop
    addu    a1, a1, 0x100
    bal     ls7a_phy_cfg_write
    nop
    addu    a1, a1, 0x100
    bal     ls7a_phy_cfg_write
    nop
#else
    lw      t1, 0x5c8(t0)
    li      t2, (1 << 24)
    or      t1, t1, t2
    sw      t1, 0x5c8(t0)
#endif

#define PCIE_GEN_CFG 0x1

#if (!LS7A_PCIE_F0_P0_DISABLE)
    dli     t0, 0x90000efe08004800
    li      a0, 0xfff9ffff
    lw      t2, 0xc(t0)
    and     a0, a0, t2
    or      a0, (PCIE_GEN_CFG << 17)
    sw      a0, 0xc(t0)

    dli     t0, 0x90000efe00004800
    li      a0, 0x60000000
    sw      a0, 0x10(t0)

    dli     t1, 0x90000e0000000000
    li      a0, 0x60000000
    or      t1, t1, a0
    li      a0, 0xff204c
    sw      a0, 0x0(t1)

    sw      $0, 0x10(t0)
#endif

#if (!LS7A_PCIE_F0_P1_DISABLE)
    dli     t0, 0x90000efe08005000
    li      a0, 0xfff9ffff
    lw      t2, 0xc(t0)
    and     a0, a0, t2
    or      a0, (PCIE_GEN_CFG << 17)
    sw      a0, 0xc(t0)

    dli     t0, 0x90000efe00005000
    li      a0, 0x60100000
    sw      a0, 0x10(t0)

    dli     t1, 0x90000e0000000000
    li      a0, 0x60100000
    or      t1, t1, a0
    li      a0, 0xff204c
    sw      a0, 0x0(t1)

    sw      $0, 0x10(t0)
#endif

#if (!LS7A_PCIE_F0_P2_DISABLE)
    dli     t0, 0x90000efe08005800
    li      a0, 0xfff9ffff
    lw      t2, 0xc(t0)
    and     a0, a0, t2
    or      a0, (PCIE_GEN_CFG << 17)
    sw      a0, 0xc(t0)

    dli     t0, 0x90000efe00005800
    li      a0, 0x60200000
    sw      a0, 0x10(t0)

    dli     t1, 0x90000e0000000000
    li      a0, 0x60200000
    or      t1, t1, a0
    li      a0, 0xff204c
    sw      a0, 0x0(t1)

    sw      $0, 0x10(t0)
#endif

#if (!LS7A_PCIE_F0_P3_DISABLE)
    dli     t0, 0x90000efe08006000
    li      a0, 0xfff9ffff
    lw      t2, 0xc(t0)
    and     a0, a0, t2
    or      a0, (PCIE_GEN_CFG << 17)
    sw      a0, 0xc(t0)

    dli     t0, 0x90000efe00006000
    li      a0, 0x60300000
    sw      a0, 0x10(t0)

    dli     t1, 0x90000e0000000000
    li      a0, 0x60300000
    or      t1, t1, a0
    li      a0, 0xff204c
    sw      a0, 0x0(t1)

    sw      $0, 0x10(t0)
#endif

#if (!LS7A_PCIE_F1_P0_DISABLE)
    dli     t0, 0x90000efe08006800
    li      a0, 0xfff9ffff
    lw      t2, 0xc(t0)
    and     a0, a0, t2
    or      a0, (PCIE_GEN_CFG << 17)
    sw      a0, 0xc(t0)

    dli     t0, 0x90000efe00006800
    li      a0, 0x61000000
    sw      a0, 0x10(t0)

    dli     t1, 0x90000e0000000000
    li      a0, 0x61000000
    or      t1, t1, a0
    li      a0, 0xff204c
    sw      a0, 0x0(t1)

    sw      $0, 0x10(t0)
#endif

#if (!LS7A_PCIE_F1_P1_DISABLE)
    dli     t0, 0x90000efe08007000
    li      a0, 0xfff9ffff
    lw      t2, 0xc(t0)
    and     a0, a0, t2
    or      a0, (PCIE_GEN_CFG << 17)
    sw      a0, 0xc(t0)

    dli     t0, 0x90000efe00007000
    li      a0, 0x61100000
    sw      a0, 0x10(t0)

    dli     t1, 0x90000e0000000000
    li      a0, 0x61100000
    or      t1, t1, a0
    li      a0, 0xff204c
    sw      a0, 0x0(t1)

    sw      $0, 0x10(t0)
#endif

#if (!LS7A_PCIE_G0_P0_DISABLE)
    dli     t0, 0x90000efe08007800
    li      a0, 0xfff9ffff
    lw      t2, 0xc(t0)
    and     a0, a0, t2
    or      a0, (PCIE_GEN_CFG << 17)
    sw      a0, 0xc(t0)

    dli     t0, 0x90000efe00007800
    li      a0, 0x62000000
    sw      a0, 0x10(t0)

    dli     t1, 0x90000e0000000000
    li      a0, 0x62000000
    or      t1, t1, a0
    li      a0, 0xff204c
    sw      a0, 0x0(t1)

    sw      $0, 0x10(t0)
#endif

#if (!LS7A_PCIE_G0_P1_DISABLE)
    dli     t0, 0x90000efe08008000
    li      a0, 0xfff9ffff
    lw      t2, 0xc(t0)
    and     a0, a0, t2
    or      a0, (PCIE_GEN_CFG << 17)
    sw      a0, 0xc(t0)

    dli     t0, 0x90000efe00008000
    li      a0, 0x62100000
    sw      a0, 0x10(t0)

    dli     t1, 0x90000e0000000000
    li      a0, 0x62100000
    or      t1, t1, a0
    li      a0, 0xff204c
    sw      a0, 0x0(t1)

    sw      $0, 0x10(t0)
#endif

#if (!LS7A_PCIE_G1_P0_DISABLE)
    dli     t0, 0x90000efe08008800
    li      a0, 0xfff9ffff
    lw      t2, 0xc(t0)
    and     a0, a0, t2
    or      a0, (PCIE_GEN_CFG << 17)
    sw      a0, 0xc(t0)

    dli     t0, 0x90000efe00008800
    li      a0, 0x63000000
    sw      a0, 0x10(t0)

    dli     t1, 0x90000e0000000000
    li      a0, 0x63000000
    or      t1, t1, a0
    li      a0, 0xff204c
    sw      a0, 0x0(t1)

    sw      $0, 0x10(t0)
#endif

#if (!LS7A_PCIE_G1_P1_DISABLE)
    dli     t0, 0x90000efe08009000
    li      a0, 0xfff9ffff
    lw      t2, 0xc(t0)
    and     a0, a0, t2
    or      a0, (PCIE_GEN_CFG << 17)
    sw      a0, 0xc(t0)

    dli     t0, 0x90000efe00009000
    li      a0, 0x63100000
    sw      a0, 0x10(t0)

    dli     t1, 0x90000e0000000000
    li      a0, 0x63100000
    or      t1, t1, a0
    li      a0, 0xff204c
    sw      a0, 0x0(t1)

    sw      $0, 0x10(t0)
#endif

#if (!LS7A_PCIE_H_P0_DISABLE)
    dli     t0, 0x90000efe08009800
    li      a0, 0xfff9ffff
    lw      t2, 0xc(t0)
    and     a0, a0, t2
    or      a0, (PCIE_GEN_CFG << 17)
    sw      a0, 0xc(t0)

    dli     t0, 0x90000efe00009800
    li      a0, 0x64000000
    sw      a0, 0x10(t0)

    dli     t1, 0x90000e0000000000
    li      a0, 0x64000000
    or      t1, t1, a0
    li      a0, 0xff204c
    sw      a0, 0x0(t1)

    sw      $0, 0x10(t0)
#endif

#if (!LS7A_PCIE_H_P1_DISABLE)
    dli     t0, 0x90000efe0800a000
    li      a0, 0xfff9ffff
    lw      t2, 0xc(t0)
    and     a0, a0, t2
    or      a0, (PCIE_GEN_CFG << 17)
    sw      a0, 0xc(t0)

    dli     t0, 0x90000efe0000a000
    li      a0, 0x64100000
    sw      a0, 0x10(t0)

    dli     t1, 0x90000e0000000000
    li      a0, 0x64100000
    or      t1, t1, a0
    li      a0, 0xff204c
    sw      a0, 0x0(t1)

    sw      $0, 0x10(t0)
#endif

    //disable clock of unused PCIE ports
    dli     t0, LS7A_CONFBUS_BASE_ADDR
    lw      t1, CONF_NB_OFFSET(t0)
    li      t2, ((LS7A_PCIE_G1_P0_DISABLE << 30) | (LS7A_PCIE_G1_P1_DISABLE << 31) | (LS7A_PCIE_G0_P0_DISABLE << 26) | (LS7A_PCIE_G0_P1_DISABLE << 27) | (LS7A_PCIE_H_P0_DISABLE << 22) | (LS7A_PCIE_H_P1_DISABLE << 23) | (LS7A_PCIE_F1_P0_DISABLE << 18) | (LS7A_PCIE_F1_P1_DISABLE << 19) | (LS7A_PCIE_F0_P0_DISABLE << 10) | (LS7A_PCIE_F0_P1_DISABLE << 11) | (LS7A_PCIE_F0_P2_DISABLE << 12) | (LS7A_PCIE_F0_P3_DISABLE << 13))
    not     t2, t2
    and     t1, t1, t2
    sw      t1, CONF_NB_OFFSET(t0)
    TTYDBG("unused PCIE ports closed\r\n")

    //use t0 as global variable
    dli     t0, LS7A_CONFBUS_BASE_ADDR
#if (!LS7A_SATA0_DISABLE)
#ifndef USE_SATA_PAD_REFCLK
    //power down phy
    lw      t1, 0x744(t0)
    li      t2, (1 << 31)
    or      t1, t1, t2
    sw      t1, 0x744(t0)

    //assert phy reset
    lw      t1, 0x740(t0)
    li      t2, (1 << 2)
    or      t1, t1, t2
    sw      t1, 0x740(t0)

    //switch refclk
    lw      t1, 0x740(t0)
    li      t2, (1 << 1)
    not     t2, t2
    and     t1, t1, t2
    sw      t1, 0x740(t0)

    //delay a while
    li      t1, 0x1000
1:
    subu    t1, t1, 1
    bnez    t1, 1b
    nop

    //power up phy
    lw      t1, 0x744(t0)
    li      t2, (1 << 31)
    not     t2, t2
    and     t1, t1, t2
    sw      t1, 0x744(t0)

    //deassert phy reset
    lw      t1, 0x740(t0)
    li      t2, (1 << 2)
    not     t2, t2
    and     t1, t1, t2
    sw      t1, 0x740(t0)

    //delay a while
    li      t1, 0x1000000
1:
    subu    t1, t1, 1
    bnez    t1, 1b
    nop
#endif
    //deassert cntl reset
    lw      t1, (CONF_SB_OFFSET+4)(t0)
    li      t2, (1 << 8)
    not     t2, t2
    and     t1, t1, t2
    sw      t1, (CONF_SB_OFFSET+4)(t0)

    //sata en
    lw      t1, (CONF_SB_OFFSET+4)(t0)
    li      t2, (1 << 10)
    or      t1, t1, t2
    sw      t1, (CONF_SB_OFFSET+4)(t0)

    //configure phy
    daddu   a0, t0, 0x748
    li      a1, 0x403f1002
    bal     ls7a_phy_cfg_write
    nop
    TTYDBG("SATA0 enabled\r\n")
#else
    //powerdown phy
    lw      t1, 0x744(t0)
    li      t2, (1 << 31)
    or      t1, t1, t2
    sw      t1, 0x744(t0)

    //disable clock
    lw      t1, (CONF_SB_OFFSET+4)(t0)
    li      t2, (1 << 11)
    not     t2, t2
    and     t1, t1, t2
    sw      t1, (CONF_SB_OFFSET+4)(t0)
    TTYDBG("SATA0 closed\r\n")
#endif

#if (!LS7A_SATA1_DISABLE)
#ifndef USE_SATA_PAD_REFCLK
    //power down phy
    lw      t1, 0x754(t0)
    li      t2, (1 << 31)
    or      t1, t1, t2
    sw      t1, 0x754(t0)

    //assert phy reset
    lw      t1, 0x750(t0)
    li      t2, (1 << 2)
    or      t1, t1, t2
    sw      t1, 0x750(t0)

    //switch refclk
    lw      t1, 0x750(t0)
    li      t2, (1 << 1)
    not     t2, t2
    and     t1, t1, t2
    sw      t1, 0x750(t0)

    //delay a while
    li      t1, 0x1000
1:
    subu    t1, t1, 1
    bnez    t1, 1b
    nop

    //power up phy
    lw      t1, 0x754(t0)
    li      t2, (1 << 31)
    not     t2, t2
    and     t1, t1, t2
    sw      t1, 0x754(t0)

    //deassert phy reset
    lw      t1, 0x750(t0)
    li      t2, (1 << 2)
    not     t2, t2
    and     t1, t1, t2
    sw      t1, 0x750(t0)

    //delay a while
    li      t1, 0x1000000
1:
    subu    t1, t1, 1
    bnez    t1, 1b
    nop
#endif
    //deassert cntl reset
    lw      t1, (CONF_SB_OFFSET+4)(t0)
    li      t2, (1 << 12)
    not     t2, t2
    and     t1, t1, t2
    sw      t1, (CONF_SB_OFFSET+4)(t0)

    //sata en
    lw      t1, (CONF_SB_OFFSET+4)(t0)
    li      t2, (1 << 14)
    or      t1, t1, t2
    sw      t1, (CONF_SB_OFFSET+4)(t0)

    //configure phy
    daddu   a0, t0, 0x758
    li      a1, 0x403f1002
    bal     ls7a_phy_cfg_write
    nop
    TTYDBG("SATA1 enabled\r\n")
#else
    //powerdown phy
    lw      t1, 0x754(t0)
    li      t2, (1 << 31)
    or      t1, t1, t2
    sw      t1, 0x754(t0)

    //disable clock
    lw      t1, (CONF_SB_OFFSET+4)(t0)
    li      t2, (1 << 15)
    not     t2, t2
    and     t1, t1, t2
    sw      t1, (CONF_SB_OFFSET+4)(t0)
    TTYDBG("SATA1 closed\r\n")
#endif

#if (!LS7A_SATA2_DISABLE)
#ifndef USE_SATA_PAD_REFCLK
    //power down phy
    lw      t1, 0x764(t0)
    li      t2, (1 << 31)
    or      t1, t1, t2
    sw      t1, 0x764(t0)

    //assert phy reset
    lw      t1, 0x760(t0)
    li      t2, (1 << 2)
    or      t1, t1, t2
    sw      t1, 0x760(t0)

    //switch refclk
    lw      t1, 0x760(t0)
    li      t2, (1 << 1)
    not     t2, t2
    and     t1, t1, t2
    sw      t1, 0x760(t0)

    //delay a while
    li      t1, 0x1000
1:
    subu    t1, t1, 1
    bnez    t1, 1b
    nop

    //power up phy
    lw      t1, 0x764(t0)
    li      t2, (1 << 31)
    not     t2, t2
    and     t1, t1, t2
    sw      t1, 0x764(t0)

    //deassert phy reset
    lw      t1, 0x760(t0)
    li      t2, (1 << 2)
    not     t2, t2
    and     t1, t1, t2
    sw      t1, 0x760(t0)

    //delay a while
    li      t1, 0x1000000
1:
    subu    t1, t1, 1
    bnez    t1, 1b
    nop
#endif
    //deassert cntl reset
    lw      t1, (CONF_SB_OFFSET+4)(t0)
    li      t2, (1 << 16)
    not     t2, t2
    and     t1, t1, t2
    sw      t1, (CONF_SB_OFFSET+4)(t0)

    //sata en
    lw      t1, (CONF_SB_OFFSET+4)(t0)
    li      t2, (1 << 18)
    or      t1, t1, t2
    sw      t1, (CONF_SB_OFFSET+4)(t0)

    //configure phy
    daddu   a0, t0, 0x768
    li      a1, 0x403f1002
    bal     ls7a_phy_cfg_write
    nop
    TTYDBG("SATA2 enabled\r\n")
#else
    //powerdown phy
    lw      t1, 0x764(t0)
    li      t2, (1 << 31)
    or      t1, t1, t2
    sw      t1, 0x764(t0)

    //disable clock
    lw      t1, (CONF_SB_OFFSET+4)(t0)
    li      t2, (1 << 19)
    not     t2, t2
    and     t1, t1, t2
    sw      t1, (CONF_SB_OFFSET+4)(t0)
    TTYDBG("SATA2 closed\r\n")
#endif

#ifdef  USE_USB_SYS_REFCLK
    //switch refclk
    lw      t1, (CONF_SB_OFFSET+0)(t0)
    li      t2, (0x2 << 2)
    or      t1, t1, t2
    sw      t1, (CONF_SB_OFFSET+0)(t0)

    //delay a while
    li      t1, 0x1000
1:
    subu    t1, t1, 1
    bnez    t1, 1b
    nop
#endif

#if (!LS7A_USB0_DISABLE)
    //deassert phy reset
    lw      t1, (CONF_SB_OFFSET+0)(t0)
    li      t2, (1 << 9)
    not     t2, t2
    and     t1, t1, t2
    sw      t1, (CONF_SB_OFFSET+0)(t0)

    //delay a while
    li      t1, 0x1000000
1:
    subu    t1, t1, 1
    bnez    t1, 1b
    nop

    //deassert cntl reset
    lw      t1, (CONF_SB_OFFSET+0)(t0)
    li      t2, (1 << 8)
    not     t2, t2
    and     t1, t1, t2
    sw      t1, (CONF_SB_OFFSET+0)(t0)

    lw      t1, (CONF_SB_OFFSET+0)(t0)
    li      t2, (1 << 10)
    or      t1, t1, t2
    sw      t1, (CONF_SB_OFFSET+0)(t0)
    TTYDBG("USB0 enabled\r\n")
#else
    //disable clock
    lw      t1, (CONF_SB_OFFSET+0)(t0)
    li      t2, (1 << 11)
    not     t2, t2
    and     t1, t1, t2
    sw      t1, (CONF_SB_OFFSET+0)(t0)
    TTYDBG("USB0 closed\r\n")
#endif

#if (!LS7A_USB1_DISABLE)
    //deassert phy reset
    lw      t1, (CONF_SB_OFFSET+0)(t0)
    li      t2, (1 << 13)
    not     t2, t2
    and     t1, t1, t2
    sw      t1, (CONF_SB_OFFSET+0)(t0)

    //delay a while
    li      t1, 0x1000000
1:
    subu    t1, t1, 1
    bnez    t1, 1b
    nop

    //deassert cntl reset
    lw      t1, (CONF_SB_OFFSET+0)(t0)
    li      t2, (1 << 12)
    not     t2, t2
    and     t1, t1, t2
    sw      t1, (CONF_SB_OFFSET+0)(t0)

    lw      t1, (CONF_SB_OFFSET+0)(t0)
    li      t2, (1 << 14)
    or      t1, t1, t2
    sw      t1, (CONF_SB_OFFSET+0)(t0)
    TTYDBG("USB1 enabled\r\n")
#else
    //disable clock
    lw      t1, (CONF_SB_OFFSET+0)(t0)
    li      t2, (1 << 15)
    not     t2, t2
    and     t1, t1, t2
    sw      t1, (CONF_SB_OFFSET+0)(t0)
    TTYDBG("USB1 closed\r\n")
#endif

#if (!LS7A_LPC_DISABLE)
    lw      t1, (CONF_SB_OFFSET+4)(t0)
    li      t2, (1 << 0)
    or      t1, t1, t2
    sw      t1, (CONF_SB_OFFSET+4)(t0)
    TTYDBG("LPC enabled\r\n")
#endif

#if LS7A_GRAPHICS_DISABLE
    lw      t1, 0x420(t0)
    li      t2, (0x1 << 5)  //TODO, can not disable DC because the PMON assume there it is and as the GPU BAR will affect DC BAR's found, so GPU cannot be disabled also
    not     t2, t2
    and     t1, t1, t2
    sw      t1, 0x420(t0)
    TTYDBG("Graphics disabled.\r\n")
#endif

// GMEM configure -- put to Device initial code ?
#ifdef LS7A_GMEM_CFG
#ifdef  DEBUG_GMEM
    PRINTSTR("\r\nInitial GMEM?(0xf: skip): ")
    bal     inputaddress
    nop
    and     v0, v0, 0xf
    dli     a1, 0x1
    bgt     v0, a1, 8f
    nop
#endif

    TTYDBG("Gmem config begin\r\n")
    //set gmem bar for init gmem use
    dli     t0, GPU_HEADER_ADDR
    dli     a0, TEMP_GMEM_ADDR
    sw      a0, 0x18(t0)
    dsrl    a0, a0, 32
    sw      a0, 0x1c(t0)
    //mem space en
    li      a0, 0x2
    sw      a0, 0x4(t0)

    dli     s1, 0xc3a10404  //memsize: unit 32MB
    bal     ls7a_gmem_init
    nop

    //set gmem space bar mask
    dli     t0, LS7A_CONFBUS_BASE_ADDR
    GET_MC0_MEMSIZE
    dsll    a1, a1, 25
    dsub    a1, a1, 1
    sw      a1, 0x3838(t0)
    sw      $0, 0x383c(t0)

    //test gmem
#if 1
    dli     t0, LS7A_GMEM_TEMP_ADDR
    dli     a0, 0x5555555555555555
    sd      a0, 0x0(t0)
    dli     a0, 0xaaaaaaaaaaaaaaaa
    sd      a0, 0x8(t0)
    dli     a0, 0x3333333333333333
    sd      a0, 0x10(t0)
    dli     a0, 0xcccccccccccccccc
    sd      a0, 0x18(t0)
    dli     a0, 0x7777777777777777
    sd      a0, 0x20(t0)
    dli     a0, 0x8888888888888888
    sd      a0, 0x28(t0)
    dli     a0, 0x1111111111111111
    sd      a0, 0x30(t0)
    dli     a0, 0xeeeeeeeeeeeeeeee
    sd      a0, 0x38(t0)

    TTYDBG("The gmem data is:\r\n")
    dli     t1, 8
    move    t5, t0
1:
    ld      t6, 0x0(t5)
    move    a0, t5
    and     a0, a0, 0xfff
    bal     hexserial
    nop
    TTYDBG(":  ")
    dsrl    a0, t6, 32
    bal     hexserial
    nop
    move    a0, t6
    bal     hexserial
    nop
    TTYDBG("\r\n")

    daddiu  t1, t1, -1
    daddiu  t5, t5, 8
    bnez    t1, 1b
    nop

#ifdef  DEBUG_GMEM
#if 1
    PRINTSTR("\r\nTest GMEM?(0xf: skip): ")
    bal     inputaddress
    nop
    and     v0, v0, 0xf
    dli     a1, 0x1
    bgt     v0, a1, 3f
    nop
#endif

    GET_MC0_MEMSIZE
    dsrl    a1, a1, 2
    dsll    a1, a1, 48
    dli     s1, 0x00000e0000000000
    or      s1, s1, a1
    dli     a1, TEMP_GMEM_ADDR
    or      s1, s1, a1
#if 1
    PRINTSTR("\r\ndefault s1 = 0x");
    dsrl    a0, s1, 32
    bal     hexserial
    nop
    PRINTSTR("__")
    move    a0, s1
    bal     hexserial
    nop
    PRINTSTR("\r\nChange test param s1(0: skip)?: ")
    bal     inputaddress
    nop
    beqz    v0, 1f
    nop
    move    s1, v0
1:
#endif
1:
    dli     t1, 0x10010
    bal     test_mem
    nop
    move    t1, v0
    PRINTSTR("\r\n")
    dsrl    a0, t1, 32
    bal     hexserial
    nop
    move    a0, t1
    bal     hexserial
    nop
    beqz    t1, 2f
    nop
    PRINTSTR("  Error found!!\r\n")
2:
#if 0
    b       1b
    nop
#endif

3:
#endif
#endif

    //recover gpu bar
    dli     t0, GPU_HEADER_ADDR
    sw      $0, 0x4(t0)
    sw      $0, 0x18(t0)
    sw      $0, 0x1c(t0)
8:
#endif

#if 0 //debug, read confbus
    TTYDBG("\r\nconfbus content:\r\n")
    dli     t0, LS7A_CONFBUS_BASE_ADDR
    daddu   t2, t0, 0x3000
    daddu   t3, t2, 0x8a0
1:
    and     a0, t2, 0xffff
    bal     hexserial
    nop
    TTYDBG(": ")
    lw      a0, 0x0(t2)
    bal     hexserial
    nop
    TTYDBG("\r\n")

    daddi   t2, t2, 0x4
    bne     t2, t3, 1b
    nop

#if 0
    TTYDBG("\r\nnode1 7A confbus content:\r\n")
    dli     a0, (1<<44)
    daddu   t1, t0, a0

    daddu   t2, t1, 0x400
    daddu   t3, t2, 0x370
1:
    and     a0, t2, 0xffff
    bal     hexserial
    nop
    TTYDBG(": ")
    lw      a0, 0x0(t2)
    bal     hexserial
    nop
    TTYDBG("\r\n")

    daddi   t2, t2, 0x4
    bne     t2, t3, 1b
    nop
#endif
#endif

