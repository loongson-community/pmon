// 7A chip configure
// author: chenxk
// 2017.8.15
// !notice: t0: used as global variable partial
// DO NOT touch this file!

    dli     t0, LS7A_CONFBUS_BASE_ADDR

    lw      t1, CONF_HT_ROUTE_OFFSET(t0)
    li      t2, (0xf | (0xf << 16))
    not     t2, t2
    and     t1, t1, t2

#ifdef MULTI_CHIP
    move    t3, zero
#ifdef  LS7A_2WAY_CONNECT
    // support 2 nodes
    // route node0 dma to 7A HT Lo
    // route node1 dma to 7A HT Hi
    /*dma_dest_ht set to support 2 node*/
    li      t3, (0xa << 16)
#endif
    ori     t2, t3, 0x1
#ifdef CHIP_4
    ori     t2, t3, 0x3
#endif
    or      t1, t1, t2
#endif

#ifdef LS7A_PHYS48_TO_HT40
    /*support 40 bit DMA device otherwise it lost node id*/
    and     t1, ~(0xff << 8)
    or      t1, (0x2 << 8) | (0x6 << 13)
#endif

    //default support 1 node
    sw      t1, CONF_HT_ROUTE_OFFSET(t0)
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

#if 1
//configure 7A pll
//LS7A_PLL_VALUE: (LOOPC, DIV2, DIV1, DIV0)

    //pcie, gmac, sata/usb
    daddu   a0, t0, CONF_PLL0_OFFSET
    li      a2, 0x4
    li      a1, LS7A_PLL_VALUE(80, 8, 16, 12)
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
    li      a1, LS7A_PLL_VALUE(127, 8, 6, 12)
    bal     ls7a_version
    nop
    beqz    v0, 1f
    nop
    li      a1, LS7A_PLL_VALUE(120, 5, 5, 12)
1:
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
    bal     ls7a_version
    nop
    beqz    v0, 1f
    nop
    li      a1, LS7A_PLL_VALUE(96, 72, 4, 100)
1:
    bal     ls7a_config_one_pll
    nop
    beqz    v0, 1f
    nop
    TTYDBG("!!!LS7A PLL2 soft configure fail.\r\n")
2:
    b       2b
    nop
1:

    //PIX0, default 38.2MHz for x800x600
    daddu   a0, t0, CONF_PLL3_OFFSET
    li      a2, 0x4
    li      a1, LS7A_PLL_VALUE(104, 68, 68, 68)
    bal     ls7a_config_one_pll
    nop
    beqz    v0, 1f
    nop
    TTYDBG("!!!LS7A PLL3 soft configure fail.\r\n")
2:
    b       2b
    nop
1:

    //PIX1, default 38.2MHz for x800x600
    daddu   a0, t0, CONF_PLL4_OFFSET
    li      a2, 0x4
    li      a1, LS7A_PLL_VALUE(104, 68, 68, 68)
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
    sb      a2, 0x304e(t0) //gmac0
    sb      a2, 0x308e(t0) //gmac1

    sb      a2, 0x32ce(t0) //sata0
    sb      a2, 0x330e(t0) //sata1
    sb      a2, 0x334e(t0) //sata2
    //fix GPU/DC header
    li      a2, 0x0400
    sh      a2, 0x31ca(t0)
    li      a2, 0x0380
    sh      a2, 0x320a(t0)

    //fix LPC header
    li      a2, 0x0601
    sh      a2, 0x344a(t0)
    li      a2, 0xfc000000
    sw      a2, 0x3460(t0)
    li      a2, 0xfd
    sw      a2, 0x3464(t0)
    li      a2, LPC_CNTL_BASE_ADDR
    sw      a2, 0x3450(t0)
    li      a2, LPC_MEM_BASE_ADDR
    sw      a2, 0x3458(t0)

    lw      a2, 0x3444(t0)
    or      a2, a2, 0x3
    sw      a2, 0x3444(t0)

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
    or      a0, 0xfc
    sw      a0, 0x3844(t0)
    //special take care of LPC
    li      a0, 0x0
    sw      a0, 0x3888(t0)
    //disable write to part of header ---- end

    //change INT and HPET fix address
    li      a1, (INT_BASE_ADDR | 0x4)
    sw      a1, 0x460(t0)
    li      a1, (HPET_BASE_ADDR | 0x4)
    sw      a1, 0x464(t0)

    TTYDBG("LS7A chip init done.\r\n")
//3. device configure 

    //init PCIE -- v0.4
    dli     t0, LS7A_CONFBUS_BASE_ADDR

    //configure phy parameter
    li      t1, 0xc2492331
    sw      t1, 0x580(t0)
    sw      t1, 0x5a0(t0)
    sw      t1, 0x5c0(t0)
    sw      t1, 0x5e0(t0)
    sw      t1, 0x600(t0)

    li      t1, 0x73e70b0
    sw      t1, 0x584(t0)
    sw      t1, 0x5a4(t0)
    sw      t1, 0x5c4(t0)
    sw      t1, 0x5e4(t0)
    sw      t1, 0x604(t0)

    li      t1, 0x20000
    sw      t1, 0x588(t0)
    sw      t1, 0x5a8(t0)
    sw      t1, 0x5c8(t0)
    sw      t1, 0x5e8(t0)
    sw      t1, 0x608(t0)

    //reset PCIE begin
    //assert reset
    lw      t1, CONF_NB_OFFSET(t0)
    li      t2, ((1 << 28) | (1 << 24) | (1 << 20) | (1 << 16) | (1 << 8))
    or      t1, t1, t2
    sw      t1, CONF_NB_OFFSET(t0)

    //enable all ports
    lw      t1, 0x588(t0)
    li      t2, (1 << 27)
    or      t1, t1, t2
    sw      t1, 0x588(t0)

    lw      t1, 0x5a8(t0)
    li      t2, (1 << 27)
    or      t1, t1, t2
    sw      t1, 0x5a8(t0)

    lw      t1, 0x5c8(t0)
    li      t2, (3 << 26)
    or      t1, t1, t2
    sw      t1, 0x5c8(t0)

    lw      t1, 0x5e8(t0)
    li      t2, (3 << 26)
    or      t1, t1, t2
    sw      t1, 0x5e8(t0)

    lw      t1, 0x608(t0)
    li      t2, (3 << 26)
    or      t1, t1, t2
    sw      t1, 0x608(t0)

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

    //switch to inside ref clk
    lw      t1, CONF_NB_OFFSET(t0)
    li      t2, (1 << 2)
    or      t1, t1, t2
    sw      t1, CONF_NB_OFFSET(t0)

    //delay 10ms
    dli     t1, 0x1000000
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

    //TTYDBG("PCIE clock status: \r\n")
    //lw      a0, 0x424(t0)
    //bal     hexserial
    //nop

    //make sure all ports clock are ready
1:
    lw      a0, 0x424(t0)
    srl     a0, a0, 8
    li      a1, 0xfff
    and     a0, a0, a1
    bne     a0, a1, 1b
    nop

    //delay at least 100ms
    dli     t1, 0x10000000
1:
    dsubu   t1, t1, 1
    bnez    t1, 1b
    nop

    //unless specified, recover to use prsnt pin to decide device existence.
#ifndef FORCE_ENABLE_PCIE_F0_P123
    lw      t1, 0x588(t0)
    li      t2, (1 << 27)
    not     t2, t2
    and     t1, t1, t2
    sw      t1, 0x588(t0)
#endif

#ifndef FORCE_ENABLE_PCIE_F1_P1
    lw      t1, 0x5a8(t0)
    li      t2, (1 << 27)
    not     t2, t2
    and     t1, t1, t2
    sw      t1, 0x5a8(t0)
#endif

#ifndef FORCE_ENABLE_PCIE_H_P1
    lw      t1, 0x5c8(t0)
    li      t2, (1 << 27)
    not     t2, t2
    and     t1, t1, t2
    sw      t1, 0x5c8(t0)
#endif

#ifndef FORCE_ENABLE_PCIE_G0_P1
    lw      t1, 0x5e8(t0)
    li      t2, (1 << 27)
    not     t2, t2
    and     t1, t1, t2
    sw      t1, 0x5e8(t0)
#endif

#ifndef FORCE_ENABLE_PCIE_G1_P1
    lw      t1, 0x608(t0)
    li      t2, (1 << 27)
    not     t2, t2
    and     t1, t1, t2
    sw      t1, 0x608(t0)
#endif

    //assert reset
    lw      t1, CONF_NB_OFFSET(t0)
    li      t2, ((1 << 28) | (1 << 24) | (1 << 20) | (1 << 16) | (1 << 8))
    or      t1, t1, t2
    sw      t1, CONF_NB_OFFSET(t0)

#ifdef USE_PCIE_PAD_REFCLK
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

    //switch to use outside ref clk
    lw      t1, CONF_NB_OFFSET(t0)
    li      t2, (1 << 2)
    not     t2, t2
    and     t1, t1, t2
    sw      t1, CONF_NB_OFFSET(t0)

    //delay 10ms
    dli     t1, 0x1000000
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
#else
    //delay 10ms
    dli     t1, 0x1000000
1:
    dsubu   t1, t1, 1
    bnez    t1, 1b
    nop
#endif

    //deassert reset
    lw      t1, CONF_NB_OFFSET(t0)
    li      t2, ((1 << 28) | (1 << 24) | (1 << 20) | (1 << 16) | (1 << 8))
    not     t2, t2
    and     t1, t1, t2
    sw      t1, CONF_NB_OFFSET(t0)

#ifndef DISABLE_PCIEX8_CAL
#define PCIE_PD_LOOP    20
    //for PCIE_H/G0/G1

    //t7: adjust dir(dll delay offset, 0: add delay at dll 0; 8: add deley at dll 1)
    //t8: old score
    dli     a0, 0x5cc
    daddu   t4, t0, a0

cal_one_pcie_x8:
    bal     ls7a_get_pcie_dll_score
    nop
#ifdef  DEBUG_PCIEX8_CAL
    TTYDBG("\r\n\r\ninitial config is: 0x")
    lw      a0, 0x0(t4)
    bal     hexserial
    nop
    TTYDBG("\r\ninitial score is: 0x")
    move    a0, t3
    bal     hexserial
    nop
    TTYDBG("\r\ninitial count of 3 is: 0x")
    move    a0, t1
    bal     hexserial
    nop
    TTYDBG("\r\ninitial count of c is: 0x")
    move    a0, t2
    bal     hexserial
    nop
#endif
    //find max of value 3 and c and determine the calibration direction
    bgt     t1, t2, 1f
    nop
    //t2 > t1
    dsubu   a0, t2, t1
    b       2f
    nop
1:
    //t1 > t2
    dsubu   a0, t1, t2
2:
    //when the two number has remarkable difference, start dll calibration
    dli     a1, (PCIE_PD_LOOP / 2)
    blt     a0, a1, 8f
    nop
    move    t7, $0
    ble     t1, t2, 1f
    nop
    daddu   t7, $0, 8
1:
    //calibration begin
    //store old score
    move    t8, t3

    li      a1, 0xff
    lw      a0, 0x0(t4)
    srl     a0, a0, t7
    and     a0, a0, a1
    beq     a0, a1, 8f  //reach max adjust value
    nop
    sll     a0, a0, 1
    or      a0, a0, 1
    and     a0, a0, a1
    sll     a0, a0, t7
    sw      a0, 0x0(t4)
    bal     ls7a_get_pcie_dll_score
    nop
#ifdef  DEBUG_PCIEX8_CAL
    TTYDBG("\r\ncurrent config is: 0x")
    lw      a0, 0x0(t4)
    bal     hexserial
    nop
    TTYDBG("\r\ncurrent score is: 0x")
    move    a0, t3
    bal     hexserial
    nop
    TTYDBG("\r\ncurrent count of 3 is: 0x")
    move    a0, t1
    bal     hexserial
    nop
    TTYDBG("\r\ncurrent count of c is: 0x")
    move    a0, t2
    bal     hexserial
    nop
#endif
    dsub    a0, t3, t8
    dli     a1, -2   //if this calibration has not make it remarkable worse, continue
    bgt     a0, a1, 1b
    nop
    //make it worse a lot, scroll back
    lw      a0, 0x0(t4)
    srl     a0, a0, t7
    and     a0, a0, 0xff
    srl     a0, a0, 1
    sll     a0, a0, t7
    sw      a0, 0x0(t4)
8:  //end of calibration
#ifdef  DEBUG_PCIEX8_CAL
    bal     ls7a_get_pcie_dll_score
    nop
    TTYDBG("\r\nfinal config is: 0x")
    lw      a0, 0x0(t4)
    bal     hexserial
    nop
    TTYDBG("\r\nfinal score is: 0x")
    move    a0, t3
    bal     hexserial
    nop
    TTYDBG("\r\nfinal count of 3 is: 0x")
    move    a0, t1
    bal     hexserial
    nop
    TTYDBG("\r\nfinal count of c is: 0x")
    move    a0, t2
    bal     hexserial
    nop
#endif
    daddu   t4, t4, 0x20
    dsubu   a0, t4, t0
    li      a1, 0x60c
    dsubu   a0, a0, a1
    blez    a0, cal_one_pcie_x8
    nop
#endif
    
    //init SATA
    //use t0 as global variable
    dli     t0, LS7A_CONFBUS_BASE_ADDR
    //configure sata phy parameter
    li      t1, 0x30c31cf9
    sw      t1, 0x744(t0)
    sw      t1, 0x754(t0)
    sw      t1, 0x764(t0)
    li      t1, 0xf3000403
    sw      t1, 0x740(t0)
    sw      t1, 0x750(t0)
    sw      t1, 0x760(t0)

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

#ifdef  OVRD_SATA_PHY
    //configure phy
    daddu   a0, t0, 0x748
    li      a1, 0x403f1002
    bal     ls7a_phy_cfg_write
    nop
#endif
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
    TTYDBG("SATA0 disabled.\r\n")
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

#ifdef  OVRD_SATA_PHY
    //configure phy
    daddu   a0, t0, 0x758
    li      a1, 0x403f1002
    bal     ls7a_phy_cfg_write
    nop
#endif
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
    TTYDBG("SATA1 disabled.\r\n")
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

#ifdef  OVRD_SATA_PHY
    //configure phy
    daddu   a0, t0, 0x768
    li      a1, 0x403f1002
    bal     ls7a_phy_cfg_write
    nop
#endif
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
    TTYDBG("SATA2 disabled.\r\n")
#endif

    //init USB
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

    //delay at least 1ms
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

	//增加usb 驱动能力
    lw      t1, (0x700)(t0)
    li      t2, (0x7 << 4) | 0x9
    or      t1, t1, t2
    sw      t1, (0x700)(t0)

    lw      t1, (0x704)(t0)
    li      t2, (0x7 << 4) | 0x9
    or      t1, t1, t2
    sw      t1, (0x704)(t0)

    lw      t1, (0x708)(t0)
    li      t2, (0x7 << 4) | 0x9
    or      t1, t1, t2
    sw      t1, (0x708)(t0)
    TTYDBG("USB0 enabled\r\n")
#else
    //disable clock
    lw      t1, (CONF_SB_OFFSET+0)(t0)
    li      t2, (1 << 11)
    not     t2, t2
    and     t1, t1, t2
    sw      t1, (CONF_SB_OFFSET+0)(t0)
    TTYDBG("USB0 disabled.\r\n")
#endif

#if (!LS7A_USB1_DISABLE)
    //deassert phy reset
    lw      t1, (CONF_SB_OFFSET+0)(t0)
    li      t2, (1 << 13)
    not     t2, t2
    and     t1, t1, t2
    sw      t1, (CONF_SB_OFFSET+0)(t0)

    //delay at least 1ms
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

	//增加usb 驱动能力
    lw      t1, (0x710)(t0)
    li      t2, (0x7 << 4) | 0x9
    or      t1, t1, t2
    sw      t1, (0x710)(t0)

    lw      t1, (0x714)(t0)
    li      t2, (0x7 << 4) | 0x9
    or      t1, t1, t2
    sw      t1, (0x714)(t0)

    lw      t1, (0x718)(t0)
    li      t2, (0x7 << 4) | 0x9
    or      t1, t1, t2
    sw      t1, (0x718)(t0)
    TTYDBG("USB1 enabled\r\n")
#else
    //disable clock
    lw      t1, (CONF_SB_OFFSET+0)(t0)
    li      t2, (1 << 15)
    not     t2, t2
    and     t1, t1, t2
    sw      t1, (CONF_SB_OFFSET+0)(t0)
    TTYDBG("USB1 disabled.\r\n")
#endif

#if (LS7A_GMAC0_DISABLE)
    lw      t1, (CONF_SB_OFFSET+0)(t0)
    li      t2, (1 << 5)
    not     t2, t2
    and     t1, t1, t2
    sw      t1, (CONF_SB_OFFSET+0)(t0)
    TTYDBG("GMAC0 disabled.\r\n")
#endif

#if (LS7A_GMAC1_DISABLE)
    lw      t1, (CONF_SB_OFFSET+0)(t0)
    li      t2, (1 << 7)
    not     t2, t2
    and     t1, t1, t2
    sw      t1, (CONF_SB_OFFSET+0)(t0)
    TTYDBG("GMAC1 disabled.\r\n")
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

#define PCIE_TRY_GEN2 1
#define PCIE_TX_FULL_SWING 0
#define PCIE_STAT_CHECK_TIMES 100   //if some device link fail, maybe you can increase this value for try

    //put PCIE device detect later, else you need to add more delay
    //delay at least 200ms
#if 1
    //delay at least 100ms
    dli     t1, 0x10000000
1:
    dsubu   t1, t1, 1
    bnez    t1, 1b
    nop
#endif
#if (!LS7A_PCIE_F0_DISABLE)
    //enable access
    lw      t1, CONF_NB_OFFSET(t0)
    li      t2, (1 << 9)
    or      t1, t1, t2
    sw      t1, CONF_NB_OFFSET(t0)
    TTYDBG("PCIE F0 enabled\r\n")

    //PCIE F0
    //re-configure PCIE PHYs
    //DO not destroy a0, a1, for example, do not add print between these code
    daddu   a0, t0, 0x590
    lui     a1, 0x4fff
    add     a1, a1, 0x1002
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

    move    t4, $0
    //Port 0
    dli     t3, 0x90000efe08004800
    li      a0, 0xfff9ffff
    lw      a1, 0xc(t3)
    and     a0, a0, a1
    or      a0, ((PCIE_TRY_GEN2 << 17) | (PCIE_TX_FULL_SWING << 18))
    sw      a0, 0xc(t3)

    dli     t3, 0x90000efe0700481c
    lw      a1, 0x0(t3)
    li      a0, (0x1 << 26)
    or      a1, a0
    sw      a1, 0x0(t3)

    dli     t3, 0x90000efe00004800
#if defined(FORCE_PCIE_GEN1) &&  FORCE_PCIE_GEN1 & 1
//pcie capability link control 2, target link speed(bit 3:0)
    lw 	    a0, 0xa0(t3)
    or	    a0, 3
    xor     a0, 2
    sw      a0, 0xa0(t3)
#endif
    li      a0, 0x60000000
    sw      a0, 0x10(t3)

    dli     t1, 0x90000e0060000000
    li      a1, (0x7<<18)|(0x7<<2)
    not     a1, a1
    lw      a0, 0x54(t1)
    and     a0, a0, a1
    sw      a0, 0x54(t1)

    lw      a0, 0x58(t1)
    and     a0, a0, a1
    sw      a0, 0x58(t1)


    dli     t1, 0x90000e0060000000
    li      a0, 0xff204c
    sw      a0, 0x0(t1)

    //read link state
    sync
    li      a1, PCIE_STAT_CHECK_TIMES
1:
    lw      a0, 0xc(t1)
    and     a0, a0, 0x3f
    bnez    a0, 2f
    nop
    subu    a1, a1, 1
    bnez    a1, 1b
    nop
    or      t4, t4, 0x1
2:
    //read x4_mode to decide whether P1~P3 is valid
    lw      a0, 0x28(t1)
    srl     a0, a0, 26
    and     a0, a0, 1
    beqz    a0, 1f
    nop
    //x4 mode
    or      t4, t4, 0xe
1:
    sw      $0, 0x10(t3)

    li      a0, 0x1
    bgt     t4, a0, 7f
    nop
    //Port 1
    dli     t3, 0x90000efe08005000
    li      a0, 0xfff9ffff
    lw      a2, 0xc(t3)
    and     a0, a0, a2
    or      a0, ((PCIE_TRY_GEN2 << 17) | (PCIE_TX_FULL_SWING << 18))
    sw      a0, 0xc(t3)

    dli     t3, 0x90000efe0700501c
    lw      a1, 0x0(t3)
    li      a0, (0x1 << 26)
    or      a1, a0
    sw      a1, 0x0(t3)

    dli     t3, 0x90000efe00005000
#if defined(FORCE_PCIE_GEN1) &&  FORCE_PCIE_GEN1 & 2
//pcie capability link control 2, target link speed(bit 3:0)
    lw 	    a0, 0xa0(t3)
    or	    a0, 3
    xor     a0, 2
    sw      a0, 0xa0(t3)
#endif
    li      a0, 0x60100000
    sw      a0, 0x10(t3)

    dli     t1, 0x90000e0060100000
    li      a1, (0x7<<18)|(0x7<<2)
    not     a1, a1
    lw      a0, 0x54(t1)
    and     a0, a0, a1
    sw      a0, 0x54(t1)

    lw      a0, 0x58(t1)
    and     a0, a0, a1
    sw      a0, 0x58(t1)

    dli     t1, 0x90000e0060100000
    li      a0, 0xff204c
    sw      a0, 0x0(t1)

    //read link state
    sync
    li      a1, PCIE_STAT_CHECK_TIMES
1:
    lw      a0, 0xc(t1)
    and     a0, a0, 0x3f
    bnez    a0, 2f
    nop
    subu    a1, a1, 1
    bnez    a1, 1b
    nop
    or      t4, t4, 0x2
2:
    sw      $0, 0x10(t3)

    //Port 2
    dli     t3, 0x90000efe08005800
    li      a0, 0xfff9ffff
    lw      a2, 0xc(t3)
    and     a0, a0, a2
    or      a0, ((PCIE_TRY_GEN2 << 17) | (PCIE_TX_FULL_SWING << 18))
    sw      a0, 0xc(t3)

    dli     t3, 0x90000efe0700581c
    lw      a1, 0x0(t3)
    li      a0, (0x1 << 26)
    or      a1, a0
    sw      a1, 0x0(t3)

    dli     t3, 0x90000efe00005800
#if defined(FORCE_PCIE_GEN1) &&  FORCE_PCIE_GEN1 & 4
//pcie capability link control 2, target link speed(bit 3:0)
    lw 	    a0, 0xa0(t3)
    or	    a0, 3
    xor     a0, 2
    sw      a0, 0xa0(t3)
#endif
    li      a0, 0x60200000
    sw      a0, 0x10(t3)

    dli     t1, 0x90000e0060200000
    li      a1, (0x7<<18)|(0x7<<2)
    not     a1, a1
    lw      a0, 0x54(t1)
    and     a0, a0, a1
    sw      a0, 0x54(t1)

    lw      a0, 0x58(t1)
    and     a0, a0, a1
    sw      a0, 0x58(t1)

    dli     t1, 0x90000e0060200000
    li      a0, 0xff204c
    sw      a0, 0x0(t1)

    //read link state
    sync
    li      a1, PCIE_STAT_CHECK_TIMES
1:
    lw      a0, 0xc(t1)
    and     a0, a0, 0x3f
    bnez    a0, 2f
    nop
    subu    a1, a1, 1
    bnez    a1, 1b
    nop
    or      t4, t4, 0x4
2:
    sw      $0, 0x10(t3)

    //Port 3
    dli     t3, 0x90000efe08006000
    li      a0, 0xfff9ffff
    lw      a2, 0xc(t3)
    and     a0, a0, a2
    or      a0, ((PCIE_TRY_GEN2 << 17) | (PCIE_TX_FULL_SWING << 18))
    sw      a0, 0xc(t3)

    dli     t3, 0x90000efe0700601c
    lw      a1, 0x0(t3)
    li      a0, (0x1 << 26)
    or      a1, a0
    sw      a1, 0x0(t3)

    dli     t3, 0x90000efe00006000
#if defined(FORCE_PCIE_GEN1) &&  FORCE_PCIE_GEN1 & 8
//pcie capability link control 2, target link speed(bit 3:0)
    lw 	    a0, 0xa0(t3)
    or	    a0, 3
    xor     a0, 2
    sw      a0, 0xa0(t3)
#endif
    li      a0, 0x60300000
    sw      a0, 0x10(t3)

    dli     t1, 0x90000e0060300000
    li      a1, (0x7<<18)|(0x7<<2)
    not     a1, a1
    lw      a0, 0x54(t1)
    and     a0, a0, a1
    sw      a0, 0x54(t1)

    lw      a0, 0x58(t1)
    and     a0, a0, a1
    sw      a0, 0x58(t1)

    dli     t1, 0x90000e0060300000
    li      a0, 0xff204c
    sw      a0, 0x0(t1)

    //read link state
    sync
    li      a1, PCIE_STAT_CHECK_TIMES
1:
    lw      a0, 0xc(t1)
    and     a0, a0, 0x3f
    bnez    a0, 2f
    nop
    subu    a1, a1, 1
    bnez    a1, 1b
    nop
    or      t4, t4, 0x8
2:
    sw      $0, 0x10(t3)
7:
#ifdef  LS7A_PCIE_NO_POWERDOWN
    move    t4, $0
#endif
#else   //disable this PCIE
    li      t4, 0xf
#endif
    li      a0, 0xf
    bne     t4, a0, 8f
    nop
    //powerdown phy
    lw      t1, 0x588(t0)
    li      t2, (1 << 24)
    or      t1, t1, t2
    sw      t1, 0x588(t0)

    lw      t1, CONF_NB_OFFSET(t0)
    li      t2, (1 << 9)
    not     t2, t2
    and     t1, t1, t2
    sw      t1, CONF_NB_OFFSET(t0)
    TTYDBG("Powerdown PCIE F0 PHY and disable all Ports.\r\n")
8:
    //disable clock of unused PCIE ports
    lw      t1, CONF_NB_OFFSET(t0)
    sll     t4, t4, 10
    not     t4, t4
    and     t1, t1, t4
    sw      t1, CONF_NB_OFFSET(t0)
    TTYDBG("unused PCIE F0 ports clock disabled.\r\n")

#if (!LS7A_PCIE_F1_DISABLE)
    //enable access
    lw      t1, CONF_NB_OFFSET(t0)
    li      t2, (1 << 17)
    or      t1, t1, t2
    sw      t1, CONF_NB_OFFSET(t0)
    TTYDBG("PCIE F1 enabled\r\n")

    //re-configure PHYs
    daddu   a0, t0, 0x5b0
    lui     a1, 0x4fff
    add     a1, a1, 0x1002
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

    move    t4, $0
    //Port 0
    dli     t3, 0x90000efe08006800
    li      a0, 0xfff9ffff
    lw      a1, 0xc(t3)
    and     a0, a0, a1
    or      a0, ((PCIE_TRY_GEN2 << 17) | (PCIE_TX_FULL_SWING << 18))
    sw      a0, 0xc(t3)

    dli     t3, 0x90000efe0700681c
    lw      a1, 0x0(t3)
    li      a0, (0x1 << 26)
    or      a1, a0
    sw      a1, 0x0(t3)

    dli     t3, 0x90000efe00006800
#if defined(FORCE_PCIE_GEN1) &&  FORCE_PCIE_GEN1 & 0x10
//pcie capability link control 2, target link speed(bit 3:0)
    lw 	    a0, 0xa0(t3)
    or	    a0, 3
    xor     a0, 2
    sw      a0, 0xa0(t3)
#endif
    li      a0, 0x60000000
    sw      a0, 0x10(t3)

    dli     t1, 0x90000e0060000000
    li      a1, (0x7<<18)|(0x7<<2)
    not     a1, a1
    lw      a0, 0x54(t1)
    and     a0, a0, a1
    sw      a0, 0x54(t1)

    lw      a0, 0x58(t1)
    and     a0, a0, a1
    sw      a0, 0x58(t1)

    dli     t1, 0x90000e0060000000
    li      a0, 0xff204c
    sw      a0, 0x0(t1)

    //read link state
    sync
    li      a1, PCIE_STAT_CHECK_TIMES
1:
    lw      a0, 0xc(t1)
    and     a0, a0, 0x3f
    bnez    a0, 2f
    nop
    subu    a1, a1, 1
    bnez    a1, 1b
    nop
    or      t4, t4, 0x1
2:
    //read x4_mode to decide whether P1 is valid
    lw      a0, 0x28(t1)
    srl     a0, a0, 26
    and     a0, a0, 1
    beqz    a0, 1f
    nop
    //x4 mode
    or      t4, t4, 0x2
1:
    sw      $0, 0x10(t3)

    li      a0, 0x1
    bgt     t4, a0, 7f
    nop
    //Port 1
    dli     t3, 0x90000efe08007000
    li      a0, 0xfff9ffff
    lw      a2, 0xc(t3)
    and     a0, a0, a2
    or      a0, ((PCIE_TRY_GEN2 << 17) | (PCIE_TX_FULL_SWING << 18))
    sw      a0, 0xc(t3)

    dli     t3, 0x90000efe0700701c
    lw      a1, 0x0(t3)
    li      a0, (0x1 << 26)
    or      a1, a0
    sw      a1, 0x0(t3)

    dli     t3, 0x90000efe00007000
#if defined(FORCE_PCIE_GEN1) &&  FORCE_PCIE_GEN1 & 0x20
//pcie capability link control 2, target link speed(bit 3:0)
    lw 	    a0, 0xa0(t3)
    or	    a0, 3
    xor     a0, 2
    sw      a0, 0xa0(t3)
#endif
    li      a0, 0x60100000
    sw      a0, 0x10(t3)

    dli     t1, 0x90000e0060100000
    li      a1, (0x7<<18)|(0x7<<2)
    not     a1, a1
    lw      a0, 0x54(t1)
    and     a0, a0, a1
    sw      a0, 0x54(t1)

    lw      a0, 0x58(t1)
    and     a0, a0, a1
    sw      a0, 0x58(t1)

    dli     t1, 0x90000e0060100000
    li      a0, 0xff204c
    sw      a0, 0x0(t1)

    //read link state
    sync
    li      a1, PCIE_STAT_CHECK_TIMES
1:
    lw      a0, 0xc(t1)
    and     a0, a0, 0x3f
    bnez    a0, 2f
    nop
    subu    a1, a1, 1
    bnez    a1, 1b
    nop
    or      t4, t4, 0x2
2:
    sw      $0, 0x10(t3)
7:
#ifdef  LS7A_PCIE_NO_POWERDOWN
    move    t4, $0
#endif
#else   //disable this PCIE
    li      t4, 0x3
#endif
    li      a0, 0x3
    bne     t4, a0, 8f
    nop
    //powerdown phy
    lw      t1, 0x5a8(t0)
    li      t2, (1 << 24)
    or      t1, t1, t2
    sw      t1, 0x5a8(t0)

    lw      t1, CONF_NB_OFFSET(t0)
    li      t2, (1 << 17)
    not     t2, t2
    and     t1, t1, t2
    sw      t1, CONF_NB_OFFSET(t0)
    TTYDBG("Powerdown PCIE F1 PHY and disable all Ports.\r\n")
8:
    //disable clock of unused PCIE ports
    lw      t1, CONF_NB_OFFSET(t0)
    sll     t4, t4, 18
    not     t4, t4
    and     t1, t1, t4
    sw      t1, CONF_NB_OFFSET(t0)
    TTYDBG("unused PCIE F1 ports clock disabled.\r\n")

#if (!LS7A_PCIE_H_DISABLE)
    //enable access
    lw      t1, CONF_NB_OFFSET(t0)
    li      t2, (1 << 21)
    or      t1, t1, t2
    sw      t1, CONF_NB_OFFSET(t0)
    TTYDBG("PCIE H enabled\r\n")

    //re-configure PHYs
    daddu   a0, t0, 0x5d0
    lui     a1, 0x4fff
    add     a1, a1, 0x1002
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
    lui     a1, 0x4fff
    add     a1, a1, 0x1002
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

    move    t4, $0
    //Port 0
    dli     t3, 0x90000efe08009800
    li      a0, 0xfff9ffff
    lw      a1, 0xc(t3)
    and     a0, a0, a1
    or      a0, ((PCIE_TRY_GEN2 << 17) | (PCIE_TX_FULL_SWING << 18))
    sw      a0, 0xc(t3)

    dli     t3, 0x90000efe0700981c
    lw      a1, 0x0(t3)
    li      a0, (0x1 << 26)
    or      a1, a0
    sw      a1, 0x0(t3)

    dli     t3, 0x90000efe00009800
#if defined(FORCE_PCIE_GEN1) &&  FORCE_PCIE_GEN1 & 0x400
//pcie capability link control 2, target link speed(bit 3:0)
    lw 	    a0, 0xa0(t3)
    or	    a0, 3
    xor     a0, 2
    sw      a0, 0xa0(t3)
#endif
    li      a0, 0x60000000
    sw      a0, 0x10(t3)

    dli     t1, 0x90000e0060000000
    li      a1, (0x7<<18)|(0x7<<2)
    not     a1, a1
    lw      a0, 0x54(t1)
    and     a0, a0, a1
    sw      a0, 0x54(t1)

    lw      a0, 0x58(t1)
    and     a0, a0, a1
    sw      a0, 0x58(t1)

    dli     t1, 0x90000e0060000000
    li      a0, 0xff204c
    sw      a0, 0x0(t1)

    //read link state
    sync
    li      a1, PCIE_STAT_CHECK_TIMES
1:
    lw      a0, 0xc(t1)
    and     a0, a0, 0x3f
    bnez    a0, 2f
    nop
    subu    a1, a1, 1
    bnez    a1, 1b
    nop
    or      t4, t4, 0x1
2:
    //read x4_mode to decide whether P1 is valid
    lw      a0, 0x28(t1)
    srl     a0, a0, 26
    and     a0, a0, 1
    bnez    a0, 1f
    nop
    //x8 mode
    or      t4, t4, 0x2
1:
    sw      $0, 0x10(t3)

    li      a0, 0x1
    bgt     t4, a0, 7f
    nop
    //Port 1
    dli     t3, 0x90000efe0800a000
    li      a0, 0xfff9ffff
    lw      a2, 0xc(t3)
    and     a0, a0, a2
    or      a0, ((PCIE_TRY_GEN2 << 17) | (PCIE_TX_FULL_SWING << 18))
    sw      a0, 0xc(t3)

    dli     t3, 0x90000efe0700a01c
    lw      a1, 0x0(t3)
    li      a0, (0x1 << 26)
    or      a1, a0
    sw      a1, 0x0(t3)

    dli     t3, 0x90000efe0000a000
#if defined(FORCE_PCIE_GEN1) &&  FORCE_PCIE_GEN1 & 0x800
//pcie capability link control 2, target link speed(bit 3:0)
    lw 	    a0, 0xa0(t3)
    or	    a0, 3
    xor     a0, 2
    sw      a0, 0xa0(t3)
#endif
    li      a0, 0x60100000
    sw      a0, 0x10(t3)

    dli     t1, 0x90000e0060100000
    li      a1, (0x7<<18)|(0x7<<2)
    not     a1, a1
    lw      a0, 0x54(t1)
    and     a0, a0, a1
    sw      a0, 0x54(t1)

    lw      a0, 0x58(t1)
    and     a0, a0, a1
    sw      a0, 0x58(t1)

    dli     t1, 0x90000e0060100000
    li      a0, 0xff204c
    sw      a0, 0x0(t1)

    //read link state
    sync
    li      a1, PCIE_STAT_CHECK_TIMES
1:
    lw      a0, 0xc(t1)
    and     a0, a0, 0x3f
    bnez    a0, 2f
    nop
    subu    a1, a1, 1
    bnez    a1, 1b
    nop
    or      t4, t4, 0x2
2:
    sw      $0, 0x10(t3)
7:
#ifdef  LS7A_PCIE_NO_POWERDOWN
    move    t4, $0
#endif
#else   //disable this PCIE
    li      t4, 0x3
#endif
    li      a0, 0x3
    bne     t4, a0, 8f
    nop
    //powerdown phy
    lw      t1, 0x5c8(t0)
    li      t2, (3 << 24)
    or      t1, t1, t2
    sw      t1, 0x5c8(t0)

    lw      t1, CONF_NB_OFFSET(t0)
    li      t2, (1 << 21)
    not     t2, t2
    and     t1, t1, t2
    sw      t1, CONF_NB_OFFSET(t0)
    TTYDBG("Powerdown PCIE H PHY and disable all Ports.\r\n")
8:
    //disable clock of unused PCIE ports
    lw      t1, CONF_NB_OFFSET(t0)
    sll     t4, t4, 22
    not     t4, t4
    and     t1, t1, t4
    sw      t1, CONF_NB_OFFSET(t0)
    TTYDBG("unused PCIE H ports clock disabled.\r\n")

#if (!LS7A_PCIE_G0_DISABLE)
    //enable access
    lw      t1, CONF_NB_OFFSET(t0)
    li      t2, (1 << 25)
    or      t1, t1, t2
    sw      t1, CONF_NB_OFFSET(t0)
    TTYDBG("PCIE G0 enabled\r\n")

    //re-configure PHYs
    daddu   a0, t0, 0x5f0
    lui     a1, 0x4fff
    add     a1, a1, 0x1002
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
    lui     a1, 0x4fff
    add     a1, a1, 0x1002
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

    move    t4, $0
    //Port 0
    dli     t3, 0x90000efe08007800
    li      a0, 0xfff9ffff
    lw      a1, 0xc(t3)
    and     a0, a0, a1
    or      a0, ((PCIE_TRY_GEN2 << 17) | (PCIE_TX_FULL_SWING << 18))
    sw      a0, 0xc(t3)

    dli     t3, 0x90000efe0700781c
    lw      a1, 0x0(t3)
    li      a0, (0x1 << 26)
    or      a1, a0
    sw      a1, 0x0(t3)

    dli     t3, 0x90000efe00007800
#if defined(FORCE_PCIE_GEN1) &&  FORCE_PCIE_GEN1 & 0x40
//pcie capability link control 2, target link speed(bit 3:0)
    lw 	    a0, 0xa0(t3)
    or	    a0, 3
    xor     a0, 2
    sw      a0, 0xa0(t3)
#endif
    li      a0, 0x60000000
    sw      a0, 0x10(t3)

    dli     t1, 0x90000e0060000000
    li      a1, (0x7<<18)|(0x7<<2)
    not     a1, a1
    lw      a0, 0x54(t1)
    and     a0, a0, a1
    sw      a0, 0x54(t1)

    lw      a0, 0x58(t1)
    and     a0, a0, a1
    sw      a0, 0x58(t1)


    dli     t1, 0x90000e0060000000
    li      a0, 0xff204c
    sw      a0, 0x0(t1)

    //read link state
    sync
    li      a1, PCIE_STAT_CHECK_TIMES
1:
    lw      a0, 0xc(t1)
    and     a0, a0, 0x3f
    bnez    a0, 2f
    nop
    subu    a1, a1, 1
    bnez    a1, 1b
    nop
    or      t4, t4, 0x1
2:
    //read x4_mode to decide whether P1 is valid
    lw      a0, 0x28(t1)
    srl     a0, a0, 26
    and     a0, a0, 1
    bnez    a0, 1f
    nop
    //x8 mode
    or      t4, t4, 0x2
1:
    sw      $0, 0x10(t3)

    li      a0, 0x1
    bgt     t4, a0, 7f
    nop
    //Port 1
    dli     t3, 0x90000efe08008000
    li      a0, 0xfff9ffff
    lw      a2, 0xc(t3)
    and     a0, a0, a2
    or      a0, ((PCIE_TRY_GEN2 << 17) | (PCIE_TX_FULL_SWING << 18))
    sw      a0, 0xc(t3)

    dli     t3, 0x90000efe0700801c
    lw      a1, 0x0(t3)
    li      a0, (0x1 << 26)
    or      a1, a0
    sw      a1, 0x0(t3)

    dli     t3, 0x90000efe00008000
#if defined(FORCE_PCIE_GEN1) &&  FORCE_PCIE_GEN1 & 0x80
//pcie capability link control 2, target link speed(bit 3:0)
    lw 	    a0, 0xa0(t3)
    or	    a0, 3
    xor     a0, 2
    sw      a0, 0xa0(t3)
#endif
    li      a0, 0x60100000
    sw      a0, 0x10(t3)

    dli     t1, 0x90000e0060100000
    li      a1, (0x7<<18)|(0x7<<2)
    not     a1, a1
    lw      a0, 0x54(t1)
    and     a0, a0, a1
    sw      a0, 0x54(t1)

    lw      a0, 0x58(t1)
    and     a0, a0, a1
    sw      a0, 0x58(t1)

    dli     t1, 0x90000e0060100000
    li      a0, 0xff204c
    sw      a0, 0x0(t1)

    //read link state
    sync
    li      a1, PCIE_STAT_CHECK_TIMES
1:
    lw      a0, 0xc(t1)
    and     a0, a0, 0x3f
    bnez    a0, 2f
    nop
    subu    a1, a1, 1
    bnez    a1, 1b
    nop
    or      t4, t4, 0x2
2:
    sw      $0, 0x10(t3)
7:
#ifdef  LS7A_PCIE_NO_POWERDOWN
    move    t4, $0
#endif
#else   //disable this PCIE
    li      t4, 0x3
#endif
    li      a0, 0x3
    bne     t4, a0, 8f
    nop
    //powerdown phy
    lw      t1, 0x5e8(t0)
    li      t2, (3 << 24)
    or      t1, t1, t2
    sw      t1, 0x5e8(t0)

    lw      t1, CONF_NB_OFFSET(t0)
    li      t2, (1 << 25)
    not     t2, t2
    and     t1, t1, t2
    sw      t1, CONF_NB_OFFSET(t0)
    TTYDBG("Powerdown PCIE G0 PHY and disable all Ports.\r\n")
8:
    //disable clock of unused PCIE ports
    lw      t1, CONF_NB_OFFSET(t0)
    sll     t4, t4, 26
    not     t4, t4
    and     t1, t1, t4
    sw      t1, CONF_NB_OFFSET(t0)
    TTYDBG("unused PCIE G0 ports clock disabled.\r\n")

#if (!LS7A_PCIE_G1_DISABLE)
    //enable access
    lw      t1, CONF_NB_OFFSET(t0)
    li      t2, (1 << 29)
    or      t1, t1, t2
    sw      t1, CONF_NB_OFFSET(t0)
    TTYDBG("PCIE G1 enabled\r\n")

    //re-configure PHYs
    daddu   a0, t0, 0x610
    lui     a1, 0x4fff
    add     a1, a1, 0x1002
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
    lui     a1, 0x4fff
    add     a1, a1, 0x1002
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

    move    t4, $0
    //Port 0
    dli     t3, 0x90000efe08008800
    li      a0, 0xfff9ffff
    lw      a1, 0xc(t3)
    and     a0, a0, a1
    or      a0, ((PCIE_TRY_GEN2 << 17) | (PCIE_TX_FULL_SWING << 18))
    sw      a0, 0xc(t3)

    dli     t3, 0x90000efe0700881c
    lw      a1, 0x0(t3)
    li      a0, (0x1 << 26)
    or      a1, a0
    sw      a1, 0x0(t3)

    dli     t3, 0x90000efe00008800
#if defined(FORCE_PCIE_GEN1) &&  FORCE_PCIE_GEN1 & 0x100
//pcie capability link control 2, target link speed(bit 3:0)
    lw 	    a0, 0xa0(t3)
    or	    a0, 3
    xor     a0, 2
    sw      a0, 0xa0(t3)
#endif
    li      a0, 0x60000000
    sw      a0, 0x10(t3)

    dli     t1, 0x90000e0060000000
    li      a1, (0x7<<18)|(0x7<<2)
    not     a1, a1
    lw      a0, 0x54(t1)
    and     a0, a0, a1
    sw      a0, 0x54(t1)

    lw      a0, 0x58(t1)
    and     a0, a0, a1
    sw      a0, 0x58(t1)

    dli     t1, 0x90000e0060000000
    li      a0, 0xff204c
    sw      a0, 0x0(t1)

    //read link state
    sync
    li      a1, PCIE_STAT_CHECK_TIMES
1:
    lw      a0, 0xc(t1)
    and     a0, a0, 0x3f
    bnez    a0, 2f
    nop
    subu    a1, a1, 1
    bnez    a1, 1b
    nop
    or      t4, t4, 0x1
2:
    //read x4_mode to decide whether P1 is valid
    lw      a0, 0x28(t1)
    srl     a0, a0, 26
    and     a0, a0, 1
    bnez    a0, 1f
    nop
    //x8 mode
    or      t4, t4, 0x2
1:
    sw      $0, 0x10(t3)

    li      a0, 0x1
    bgt     t4, a0, 7f
    nop
    //Port 1
    dli     t3, 0x90000efe08009000
    li      a0, 0xfff9ffff
    lw      a2, 0xc(t3)
    and     a0, a0, a2
    or      a0, ((PCIE_TRY_GEN2 << 17) | (PCIE_TX_FULL_SWING << 18))
    sw      a0, 0xc(t3)

    dli     t3, 0x90000efe0700901c
    lw      a1, 0x0(t3)
    li      a0, (0x1 << 26)
    or      a1, a0
    sw      a1, 0x0(t3)

    dli     t3, 0x90000efe00009000
#if defined(FORCE_PCIE_GEN1) &&  FORCE_PCIE_GEN1 & 0x200
//pcie capability link control 2, target link speed(bit 3:0)
    lw 	    a0, 0xa0(t3)
    or	    a0, 3
    xor     a0, 2
    sw      a0, 0xa0(t3)
#endif
    li      a0, 0x60100000
    sw      a0, 0x10(t3)

    dli     t1, 0x90000e0060100000
    li      a1, (0x7<<18)|(0x7<<2)
    not     a1, a1
    lw      a0, 0x54(t1)
    and     a0, a0, a1
    sw      a0, 0x54(t1)

    lw      a0, 0x58(t1)
    and     a0, a0, a1
    sw      a0, 0x58(t1)

    dli     t1, 0x90000e0060100000
    li      a0, 0xff204c
    sw      a0, 0x0(t1)

    //read link state
    sync
    li      a1, PCIE_STAT_CHECK_TIMES
1:
    lw      a0, 0xc(t1)
    and     a0, a0, 0x3f
    bnez    a0, 2f
    nop
    subu    a1, a1, 1
    bnez    a1, 1b
    nop
    or      t4, t4, 0x2
2:
    sw      $0, 0x10(t3)
7:
#ifdef  LS7A_PCIE_NO_POWERDOWN
    move    t4, $0
#endif
#else   //disable this PCIE
    li      t4, 0x3
#endif
    li      a0, 0x3
    bne     t4, a0, 8f
    nop
    //powerdown phy
    lw      t1, 0x608(t0)
    li      t2, (3 << 24)
    or      t1, t1, t2
    sw      t1, 0x608(t0)

    lw      t1, CONF_NB_OFFSET(t0)
    li      t2, (1 << 29)
    not     t2, t2
    and     t1, t1, t2
    sw      t1, CONF_NB_OFFSET(t0)
    TTYDBG("Powerdown PCIE G1 PHY and disable all Ports.\r\n")
8:
    //disable clock of unused PCIE ports
    lw      t1, CONF_NB_OFFSET(t0)
    sll     t4, t4, 30
    not     t4, t4
    and     t1, t1, t4
    sw      t1, CONF_NB_OFFSET(t0)
    TTYDBG("unused PCIE G1 ports clock disabled.\r\n")

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

#if 0
#ifdef  DEBUG_GMEM
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
#ifdef  DEBUG_GMEM
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
    dli     t1, 0x40010
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
#if 1
    bal     beep_on
    nop
1:
    b       1b
    nop
#endif
2:

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

    dli     t0, LS7A_CONFBUS_BASE_ADDR
#ifdef  LS7A_UC_ACC
    lw      t1, (CONF_NB_OFFSET+4)(t0)
    li      t2, (0x3f << 0)
    or      t1, t1, t2
    sw      t1, (CONF_NB_OFFSET+4)(t0)

    lw      t1, (CONF_SB_OFFSET+0)(t0)
    li      t2, (0xef << 24)
    or      t1, t1, t2
    sw      t1, (CONF_SB_OFFSET+0)(t0)
    TTYDBG("LS7A uncache accellerator enabled\r\n")
#endif

