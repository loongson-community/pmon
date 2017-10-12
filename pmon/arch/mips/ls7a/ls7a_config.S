
//func: subroutine for LS7A related ops
//author: Chen Xinke
//date: 2017.8.17

    .global ls7a_config_one_pll
    .ent    ls7a_config_one_pll
    .set    noreorder
    .set    mips3
ls7a_config_one_pll:
//a0: pll address
//a1: pll value
//a2: div_refc

    move    t7, ra

    //switch to backup clk
    lw      t1, 0x4(a0)
    li      t2, (0x7 << LS7A_PLL_SEL0_OFFSET)
    not     t2, t2
    and     t1, t1, t2
    sw      t1, 0x4(a0)

    //power down pll
    lw      t1, 0x4(a0)
    li      t2, (1 << LS7A_PLL_PD_OFFSET)
    or      t1, t1, t2
    sw      t1, 0x4(a0)

    //disable pll configure
    lw      t1, 0x4(a0)
    li      t2, (1 << LS7A_PLL_SET_OFFSET)
    not     t2, t2
    and     t1, t1, t2
    sw      t1, 0x4(a0)

    //configure pll parameters
    sw      a1, 0x0(a0)
    //set div_refc
    lw      t1, 0x4(a0)
    li      t2, (0x3f << LS7A_PLL_DIV_REFC_OFFSET)
    not     t2, t2
    and     t1, t1, t2
    or      t1, t1, a2
    sw      t1, 0x4(a0)

    //enable pll configure
    lw      t1, 0x4(a0)
    li      t2, (1 << LS7A_PLL_SET_OFFSET)
    or      t1, t1, t2
    sw      t1, 0x4(a0)

    //not bypass pll
    lw      t1, 0x4(a0)
    li      t2, (0x1 << LS7A_PLL_BYPASS_OFFSET)
    not     t2, t2
    and     t1, t1, t2
    sw      t1, 0x4(a0)

    //power up pll
    lw      t1, 0x4(a0)
    li      t2, (0x1 << LS7A_PLL_PD_OFFSET)
    not     t2, t2
    and     t1, t1, t2
    sw      t1, 0x4(a0)

    //poll lock signal
    li      t2, (0x1 << LS7A_PLL_LOCK_OFFSET)
1:
    lw      t1, 0x4(a0)
    and     t1, t1, t2
    beqz    t1, 1b
    nop

    //select pll out
    lw      t1, 0x4(a0)
    li      t2, (0x7 << LS7A_PLL_SEL0_OFFSET)
    or      t1, t1, t2
    sw      t1, 0x4(a0)

    move    ra, t7
    jr      ra
    nop
    .end    ls7a_config_one_pll

    .global ls7a_phy_cfg_write
    .ent    ls7a_phy_cfg_write
    .set    noreorder
    .set    mips3
ls7a_phy_cfg_write:
//a0: confreg address
//a1: [phy_cfg_data, phy_cfg_addr]

    //wait cfg ready first
1:
    lw      v0, 0x4(a0)
    and     v0, v0, 0x4
    beqz    v0, 1b
    nop

    sw      a1, 0x0(a0)
    li      v0, 0x1
    sw      v0, 0x4(a0)

    jr      ra
    nop
    .end    ls7a_phy_cfg_write

    .global ls7a_phy_cfg_read
    .ent    ls7a_phy_cfg_read
    .set    noreorder
    .set    mips3
ls7a_phy_cfg_read:
//a0: confreg address
//a1: [phy_cfg_addr]

    //wait cfg ready first
1:
    lw      v0, 0x4(a0)
    and     v0, v0, 0x4
    beqz    v0, 1b
    nop

    sw      a1, 0x0(a0)
    sw      $0, 0x4(a0)

    //wait read data ready
1:
    lw      v0, 0x4(a0)
    and     v0, v0, 0x4
    beqz    v0, 1b
    nop

    lw      v0, 0x0(a0)

    jr      ra
    nop
    .end    ls7a_phy_cfg_read
