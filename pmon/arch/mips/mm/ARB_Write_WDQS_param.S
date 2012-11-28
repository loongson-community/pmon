/*
 * write wrlvl_delay store in t2
 *
 * Author:  Chen Xinke
 * Date:    201209
 *
 * input param:
 *   t2: param value of 8 byte lane
 *   use register: t7
*/

#ifdef  CONTROL_L2XBAR_DDR_WINDOW
    //PRINTSTR("\r\nDisable DDR access window.")
    sync
    sync
    sync
    sync
    sync
    sync
    sync
    sync
    //Disable L2XBAR_WINDOW
    dli     t7, L2XBAR_CONFIG_BASE_ADDR
#ifdef LS3B
    GET_ARB_LEVEL_NODE_ID
    dsll    a1, a1, 14
    daddu   t7, t7, a1
#endif
    GET_ARB_LEVEL_NODE_ID
    dsll    a1, a1, 44
    or      t7, t7, a1
    daddu   t7, t7, ARB_TEMP_L2WINDOW_OFFSET
    ld      a0, 0x80(t7)
    and     a0, a0, 0xf
    sd      a0, 0x80(t7)
#endif

    //PRINTSTR("\r\nEnable DDR MC config space.");
    bal     enable_ddr_confspace
    nop
    
    dli     t7, DDR_MC_CONFIG_BASE
    GET_ARB_LEVEL_NODE_ID
    dsll    a1, a1, 44
    or      t7, t7, a1
    
#ifdef  ARBLVL_PUT_DRAM_SREF
    //put memory into self-refresh
    ld      a1, SREFRESH_ADDR(t7)
    dli     a2, 0x1
    dsll    a2, a2, SREFRESH_OFFSET
    or      a1, a1, a2
    sd      a1, SREFRESH_ADDR(t7)
    sync

    //delay some time
    dli     a2, 0x400
1:
    daddiu  a2, a2, -1
    bnez    a2, 1b
    nop
#endif
    
    //clear param_start
    //PRINTSTR("\r\nClear param_start.")
    dli     a2, 0xff
    dsll    a2, a2, START_OFFSET
    not     a2, a2
    ld      a1, START_ADDR(t7)
    and     a1, a1, a2
    sd      a1, START_ADDR(t7)

    //reset Gather FIFO
    ld      a1, PHY_CTRL_2_ADDR(t7)
    dli     a2, 0x1
    dsll    a2, a2, PHY_CTRL_2_OFFSET + RESET_GFIFO_SHIFT
    or      a1, a1, a2
    sd      a1, PHY_CTRL_2_ADDR(t7)

#if 1
    //PRINTSTR("\r\nChange param value.");

    ld      a1, WRLVL_DELAY_7_ADDR(t7)
    dli     a2, WRLVL_DELAY_MASK
    dsll    a2, a2, WRLVL_DELAY_7_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsrl    a2, t2, 0x38
    and     a2, a2, 0x7f
    dsll    a2, a2, WRLVL_DELAY_7_OFFSET
    or      a1, a1, a2
    sd      a1, WRLVL_DELAY_7_ADDR(t7)

    ld      a1, WRLVL_DELAY_6_ADDR(t7)
    dli     a2, WRLVL_DELAY_MASK
    dsll    a2, a2, WRLVL_DELAY_6_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsrl    a2, t2, 0x30
    and     a2, a2, 0x7f
    dsll    a2, a2, WRLVL_DELAY_6_OFFSET
    or      a1, a1, a2
    sd      a1, WRLVL_DELAY_6_ADDR(t7)

    ld      a1, WRLVL_DELAY_5_ADDR(t7)
    dli     a2, WRLVL_DELAY_MASK
    dsll    a2, a2, WRLVL_DELAY_5_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsrl    a2, t2, 0x28
    and     a2, a2, 0x7f
    dsll    a2, a2, WRLVL_DELAY_5_OFFSET
    or      a1, a1, a2
    sd      a1, WRLVL_DELAY_5_ADDR(t7)

    ld      a1, WRLVL_DELAY_4_ADDR(t7)
    dli     a2, WRLVL_DELAY_MASK
    dsll    a2, a2, WRLVL_DELAY_4_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsrl    a2, t2, 0x20
    and     a2, a2, 0x7f
    dsll    a2, a2, WRLVL_DELAY_4_OFFSET
    or      a1, a1, a2
    sd      a1, WRLVL_DELAY_4_ADDR(t7)

    ld      a1, WRLVL_DELAY_3_ADDR(t7)
    dli     a2, WRLVL_DELAY_MASK
    dsll    a2, a2, WRLVL_DELAY_3_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsrl    a2, t2, 0x18
    and     a2, a2, 0x7f
    dsll    a2, a2, WRLVL_DELAY_3_OFFSET
    or      a1, a1, a2
    sd      a1, WRLVL_DELAY_3_ADDR(t7)

    ld      a1, WRLVL_DELAY_2_ADDR(t7)
    dli     a2, WRLVL_DELAY_MASK
    dsll    a2, a2, WRLVL_DELAY_2_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsrl    a2, t2, 0x10
    and     a2, a2, 0x7f
    dsll    a2, a2, WRLVL_DELAY_2_OFFSET
    or      a1, a1, a2
    sd      a1, WRLVL_DELAY_2_ADDR(t7)

    ld      a1, WRLVL_DELAY_1_ADDR(t7)
    dli     a2, WRLVL_DELAY_MASK
    dsll    a2, a2, WRLVL_DELAY_1_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsrl    a2, t2, 0x8
    and     a2, a2, 0x7f
    dsll    a2, a2, WRLVL_DELAY_1_OFFSET
    or      a1, a1, a2
    sd      a1, WRLVL_DELAY_1_ADDR(t7)

    ld      a1, WRLVL_DELAY_0_ADDR(t7)
    dli     a2, WRLVL_DELAY_MASK
    dsll    a2, a2, WRLVL_DELAY_0_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsrl    a2, t2, 0x0
    and     a2, a2, 0x7f
    dsll    a2, a2, WRLVL_DELAY_0_OFFSET
    or      a1, a1, a2
    sd      a1, WRLVL_DELAY_0_ADDR(t7)

#ifdef  CLEAR_HALF_CLK_SHIFT
    //reset phy_ctrl_reg_0[17:16]--add_half_clk_shift
    //byte_7
    dli     a1, WRLVL_HALF_CLK_VALUE
    dsrl    a2, t2, 0x38
    and     a2, a2, 0x7f
    blt     a2, a1, 51f
    nop
    //set add_half_clk_shift
    ld      a1, PHY_CTRL_0_7_ADDR(t7)
    dli     a2, 0x3
    dsll    a2, a2, PHY_CTRL_0_7_OFFSET + PHY_CTRL_0_ADDWLDLY_SHIFT
    or      a1, a1, a2
    sd      a1, PHY_CTRL_0_7_ADDR(t7)
    b       52f
    nop
51: //clear add_half_clk_shift
    ld      a1, PHY_CTRL_0_7_ADDR(t7)
    dli     a2, 0x3
    dsll    a2, a2, PHY_CTRL_0_7_OFFSET + PHY_CTRL_0_ADDWLDLY_SHIFT
    not     a2, a2
    and     a1, a1, a2
    sd      a1, PHY_CTRL_0_7_ADDR(t7)
52:
    //byte_6
    dli     a1, WRLVL_HALF_CLK_VALUE
    dsrl    a2, t2, 0x30
    and     a2, a2, 0x7f
    blt     a2, a1, 51f
    nop
    //set add_half_clk_shift
    ld      a1, PHY_CTRL_0_6_ADDR(t7)
    dli     a2, 0x3
    dsll    a2, a2, PHY_CTRL_0_6_OFFSET + PHY_CTRL_0_ADDWLDLY_SHIFT
    or      a1, a1, a2
    sd      a1, PHY_CTRL_0_6_ADDR(t7)
    b       52f
    nop
51: //clear add_half_clk_shift
    ld      a1, PHY_CTRL_0_6_ADDR(t7)
    dli     a2, 0x3
    dsll    a2, a2, PHY_CTRL_0_6_OFFSET + PHY_CTRL_0_ADDWLDLY_SHIFT
    not     a2, a2
    and     a1, a1, a2
    sd      a1, PHY_CTRL_0_6_ADDR(t7)
52:
    //byte_5
    dli     a1, WRLVL_HALF_CLK_VALUE
    dsrl    a2, t2, 0x28
    and     a2, a2, 0x7f
    blt     a2, a1, 51f
    nop
    //set add_half_clk_shift
    ld      a1, PHY_CTRL_0_5_ADDR(t7)
    dli     a2, 0x3
    dsll    a2, a2, PHY_CTRL_0_5_OFFSET + PHY_CTRL_0_ADDWLDLY_SHIFT
    or      a1, a1, a2
    sd      a1, PHY_CTRL_0_5_ADDR(t7)
    b       52f
    nop
51: //clear add_half_clk_shift
    ld      a1, PHY_CTRL_0_5_ADDR(t7)
    dli     a2, 0x3
    dsll    a2, a2, PHY_CTRL_0_5_OFFSET + PHY_CTRL_0_ADDWLDLY_SHIFT
    not     a2, a2
    and     a1, a1, a2
    sd      a1, PHY_CTRL_0_5_ADDR(t7)
52:
    //byte_4
    dli     a1, WRLVL_HALF_CLK_VALUE
    dsrl    a2, t2, 0x20
    and     a2, a2, 0x7f
    blt     a2, a1, 51f
    nop
    //set add_half_clk_shift
    ld      a1, PHY_CTRL_0_4_ADDR(t7)
    dli     a2, 0x3
    dsll    a2, a2, PHY_CTRL_0_4_OFFSET + PHY_CTRL_0_ADDWLDLY_SHIFT
    or      a1, a1, a2
    sd      a1, PHY_CTRL_0_4_ADDR(t7)
    b       52f
    nop
51: //clear add_half_clk_shift
    ld      a1, PHY_CTRL_0_4_ADDR(t7)
    dli     a2, 0x3
    dsll    a2, a2, PHY_CTRL_0_4_OFFSET + PHY_CTRL_0_ADDWLDLY_SHIFT
    not     a2, a2
    and     a1, a1, a2
    sd      a1, PHY_CTRL_0_4_ADDR(t7)
52:
    //byte_3
    dli     a1, WRLVL_HALF_CLK_VALUE
    dsrl    a2, t2, 0x18
    and     a2, a2, 0x7f
    blt     a2, a1, 51f
    nop
    //set add_half_clk_shift
    ld      a1, PHY_CTRL_0_3_ADDR(t7)
    dli     a2, 0x3
    dsll    a2, a2, PHY_CTRL_0_3_OFFSET + PHY_CTRL_0_ADDWLDLY_SHIFT
    or      a1, a1, a2
    sd      a1, PHY_CTRL_0_3_ADDR(t7)
    b       52f
    nop
51: //clear add_half_clk_shift
    ld      a1, PHY_CTRL_0_3_ADDR(t7)
    dli     a2, 0x3
    dsll    a2, a2, PHY_CTRL_0_3_OFFSET + PHY_CTRL_0_ADDWLDLY_SHIFT
    not     a2, a2
    and     a1, a1, a2
    sd      a1, PHY_CTRL_0_3_ADDR(t7)
52:
    //byte_2
    dli     a1, WRLVL_HALF_CLK_VALUE
    dsrl    a2, t2, 0x10
    and     a2, a2, 0x7f
    blt     a2, a1, 51f
    nop
    //set add_half_clk_shift
    ld      a1, PHY_CTRL_0_2_ADDR(t7)
    dli     a2, 0x3
    dsll    a2, a2, PHY_CTRL_0_2_OFFSET + PHY_CTRL_0_ADDWLDLY_SHIFT
    or      a1, a1, a2
    sd      a1, PHY_CTRL_0_2_ADDR(t7)
    b       52f
    nop
51: //clear add_half_clk_shift
    ld      a1, PHY_CTRL_0_2_ADDR(t7)
    dli     a2, 0x3
    dsll    a2, a2, PHY_CTRL_0_2_OFFSET + PHY_CTRL_0_ADDWLDLY_SHIFT
    not     a2, a2
    and     a1, a1, a2
    sd      a1, PHY_CTRL_0_2_ADDR(t7)
52:
    //byte_1
    dli     a1, WRLVL_HALF_CLK_VALUE
    dsrl    a2, t2, 0x8
    and     a2, a2, 0x7f
    blt     a2, a1, 51f
    nop
    //set add_half_clk_shift
    ld      a1, PHY_CTRL_0_1_ADDR(t7)
    dli     a2, 0x3
    dsll    a2, a2, PHY_CTRL_0_1_OFFSET + PHY_CTRL_0_ADDWLDLY_SHIFT
    or      a1, a1, a2
    sd      a1, PHY_CTRL_0_1_ADDR(t7)
    b       52f
    nop
51: //clear add_half_clk_shift
    ld      a1, PHY_CTRL_0_1_ADDR(t7)
    dli     a2, 0x3
    dsll    a2, a2, PHY_CTRL_0_1_OFFSET + PHY_CTRL_0_ADDWLDLY_SHIFT
    not     a2, a2
    and     a1, a1, a2
    sd      a1, PHY_CTRL_0_1_ADDR(t7)
52:
    //byte_0
    dli     a1, WRLVL_HALF_CLK_VALUE
    dsrl    a2, t2, 0x0
    and     a2, a2, 0x7f
    blt     a2, a1, 51f
    nop
    //set add_half_clk_shift
    ld      a1, PHY_CTRL_0_0_ADDR(t7)
    dli     a2, 0x3
    dsll    a2, a2, PHY_CTRL_0_0_OFFSET + PHY_CTRL_0_ADDWLDLY_SHIFT
    or      a1, a1, a2
    sd      a1, PHY_CTRL_0_0_ADDR(t7)
    b       52f
    nop
51: //clear add_half_clk_shift
    ld      a1, PHY_CTRL_0_0_ADDR(t7)
    dli     a2, 0x3
    dsll    a2, a2, PHY_CTRL_0_0_OFFSET + PHY_CTRL_0_ADDWLDLY_SHIFT
    not     a2, a2
    and     a1, a1, a2
    sd      a1, PHY_CTRL_0_0_ADDR(t7)
52:
#endif

    sync
#endif

    //enable Gather FIFO
    ld      a1, PHY_CTRL_2_ADDR(t7)
    dli     a2, 0x1
    dsll    a2, a2, PHY_CTRL_2_OFFSET + RESET_GFIFO_SHIFT
    not     a2, a2
    and     a1, a1, a2
    sd      a1, PHY_CTRL_2_ADDR(t7)

    //set start to 1
    //PRINTSTR("\r\nSet param_start 1.");
    dli     a2, 0x1
    dsll    a2, a2, START_OFFSET
    ld      a1, START_ADDR(t7)
    or      a1, a1, a2
    sd      a1, START_ADDR(t7)
    sync

    //poll until DLL locked.
    dli     a2, 0x1
1:
    ld      a1, DLLLOCKREG_ADDR(t7)
    and     a1, a1, a2
    beqz    a1, 1b
    nop

#ifdef  ARBLVL_PUT_DRAM_SREF
    //pull memory out of self-refresh
    ld      a1, SREFRESH_ADDR(t7)
    dli     a2, 0x1
    dsll    a2, a2, SREFRESH_OFFSET
    not     a2, a2
    and     a1, a1, a2
    sd      a1, SREFRESH_ADDR(t7)
    sync

    //delay some time
    dli     a2, 0x400
1:
    daddiu  a2, a2, -1
    bnez    a2, 1b
    nop
#endif
    
    //PRINTSTR("\r\nDisable DDR MC config space.\r\n");
    bal     disable_ddr_confspace
    nop

#ifdef  CONTROL_L2XBAR_DDR_WINDOW
    //PRINTSTR("\r\nEnable DDR access window.")
    sync
    sync
    sync
    sync
    sync
    sync
    sync
    sync
    //Enable L2XBAR_WINDOW
    dli     t7, L2XBAR_CONFIG_BASE_ADDR
#ifdef LS3B
    GET_ARB_LEVEL_NODE_ID
    dsll    a1, a1, 14
    daddu   t7, t7, a1
#endif
    GET_ARB_LEVEL_NODE_ID
    dsll    a1, a1, 44
    or      t7, t7, a1
    daddu   t7, t7, ARB_TEMP_L2WINDOW_OFFSET
    ld      a0, 0x80(t7)
    or      a0, a0, 0xf0
    sd      a0, 0x80(t7)
#endif

#ifdef  ADD_DELAY_AFTER_RESET_PHY
    //this delay can't be removed. wired!
    //delay some time, how long is proper?
    dli     a2, MC_RST_DELAY
1:
    daddiu  a2, a2, -1
    bnez    a2, 1b
    nop
#endif
