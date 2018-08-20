
//func: subroutine for LS7A related ops
//author: Chen Xinke
//date: 2017.8.17

    .global ls7a_config_one_pll
    .ent    ls7a_config_one_pll
    .set    noreorder
    .set    mips3
ls7a_config_one_pll:
//input parameters:
//a0: pll address
//a1: pll value
//a2: div_refc
//output value:
//v0: 0: success; 1: fail.


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
    li      v1, 0x1000
    move    v0, $0
    li      t2, (0x1 << LS7A_PLL_LOCK_OFFSET)
1:
    lw      t1, 0x4(a0)
    and     t1, t1, t2
    subu    v1, v1, 1
    beqz    v1, 1f
    nop
    beqz    t1, 1b
    nop

    //select pll out
    lw      t1, 0x4(a0)
    li      t2, (0x7 << LS7A_PLL_SEL0_OFFSET)
    or      t1, t1, t2
    sw      t1, 0x4(a0)
    b       2f
    nop
1:  //PLL lock fail
    ori     v0, v0, 1 

2:
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

    //score calculation algorithm:
    //for value: 3/c: -1; 1/4/5: +1; others: +0;
    //repeat N times, each time get 2 values, so highest score is 2N,
    //lowest score is -2N.
    //add score offset 2N to make score keep positive
    //record number of value 3 and value c seperately too, for dll adjustment.
    //t1: count of value 3; indicate clk 1 early
    //t2: count of value c; indicate clk 0 early
    //t3: current configure score

    .global ls7a_get_pcie_dll_score
    .ent    ls7a_get_pcie_dll_score
    .set    noreorder
    .set    mips3
ls7a_get_pcie_dll_score:
//input:
//t4: confreg address
//return: t1, t2, t3
//used register: a0-a2, v0-v1, t1-t5

    move    t1, $0
    move    t2, $0
    li      t3, (PCIE_PD_LOOP * 2)

    li      a1, PCIE_PD_LOOP
1:
    lw      a0, 0x0(t4)
    srl     a2, a0, 24
    and     a2, a2, 0xf
    move    t5, $0  //control check 2 clk
check_1clk:
    li      v0, 0x3
    beq     a2, v0, pd_3
    nop
    li      v0, 0xc
    beq     a2, v0, pd_c
    nop
    li      v0, 0x1
    beq     a2, v0, pd_145
    nop
    li      v0, 0x4
    beq     a2, v0, pd_145
    nop
    li      v0, 0x5
    beq     a2, v0, pd_145
    nop
    b       2f
    nop

pd_3:
    li      v1, (1 << 0)
    daddu   t1, t1, v1
    subu    t3, t3, 1
    b       2f
    nop
pd_c:
    li      v1, (1 << 0)
    daddu   t2, t2, v1
    subu    t3, t3, 1
    b       2f
    nop

pd_145:
    addu    t3, t3, 1
2:
    bnez    t5, 3f
    nop
    addu    t5, t5, 1
    srl     a2, a0, 28
    and     a2, a2, 0xf
    b       check_1clk
    nop
3:
    subu    a1, a1, 1
    bnez    a1, 1b
    nop
    
    jr      ra
    nop
    .end    ls7a_get_pcie_dll_score
