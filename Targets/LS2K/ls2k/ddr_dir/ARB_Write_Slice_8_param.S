/*
 * Review the leveled result and adjust some value
 * Write Slice 8 phy and dll delay related setting
 * according to the nearest SDRAM chip
 * Author:  Chen Xinke
 * Date:    201209
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
    sync
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
#ifdef  USE_BIG_GF_POP_DELAY
    //adjust Gather FIFO pop delay: if all 8(4 at reduc mode) slices read pop delay
    //use 4, then adjust Gather FIFO pop delay to 7(instead of 6).
    dli     a2, 0x4
    GET_DIMM_WIDTH
    bnez    a1, 2f
    nop

    ld      a1, PHY_CTRL_0_7_ADDR(t7)
    dsrl    a1, a1, PHY_CTRL_0_7_OFFSET + PHY_CTRL_0_POP_DELAY_SHIFT
    and     a1, a1, PHY_CTRL_0_POP_DELAY_MASK
    blt     a1, a2, 1f
    nop
    ld      a1, PHY_CTRL_0_6_ADDR(t7)
    dsrl    a1, a1, PHY_CTRL_0_6_OFFSET + PHY_CTRL_0_POP_DELAY_SHIFT
    and     a1, a1, PHY_CTRL_0_POP_DELAY_MASK
    blt     a1, a2, 1f
    nop
    ld      a1, PHY_CTRL_0_5_ADDR(t7)
    dsrl    a1, a1, PHY_CTRL_0_5_OFFSET + PHY_CTRL_0_POP_DELAY_SHIFT
    and     a1, a1, PHY_CTRL_0_POP_DELAY_MASK
    blt     a1, a2, 1f
    nop
    ld      a1, PHY_CTRL_0_4_ADDR(t7)
    dsrl    a1, a1, PHY_CTRL_0_4_OFFSET + PHY_CTRL_0_POP_DELAY_SHIFT
    and     a1, a1, PHY_CTRL_0_POP_DELAY_MASK
    blt     a1, a2, 1f
    nop
2:
    ld      a1, PHY_CTRL_0_3_ADDR(t7)
    dsrl    a1, a1, PHY_CTRL_0_3_OFFSET + PHY_CTRL_0_POP_DELAY_SHIFT
    and     a1, a1, PHY_CTRL_0_POP_DELAY_MASK
    blt     a1, a2, 1f
    nop
    ld      a1, PHY_CTRL_0_2_ADDR(t7)
    dsrl    a1, a1, PHY_CTRL_0_2_OFFSET + PHY_CTRL_0_POP_DELAY_SHIFT
    and     a1, a1, PHY_CTRL_0_POP_DELAY_MASK
    blt     a1, a2, 1f
    nop
    ld      a1, PHY_CTRL_0_1_ADDR(t7)
    dsrl    a1, a1, PHY_CTRL_0_1_OFFSET + PHY_CTRL_0_POP_DELAY_SHIFT
    and     a1, a1, PHY_CTRL_0_POP_DELAY_MASK
    blt     a1, a2, 1f
    nop
    ld      a1, PHY_CTRL_0_0_ADDR(t7)
    dsrl    a1, a1, PHY_CTRL_0_0_OFFSET + PHY_CTRL_0_POP_DELAY_SHIFT
    and     a1, a1, PHY_CTRL_0_POP_DELAY_MASK
    blt     a1, a2, 1f
    nop

    //change Gather FIFO pop delay
    ld      a1, PHY_CTRL_2_ADDR(t7)
    dli     a2, PHY_CTRL_2_POP_DELAY_MASK
    dsll    a2, a2, PHY_CTRL_2_OFFSET + PHY_CTRL_2_POP_DELAY_SHIFT
    not     a2, a2
    and     a1, a1, a2
    dli     a2, 0x7
    dsll    a2, a2, PHY_CTRL_2_OFFSET + PHY_CTRL_2_POP_DELAY_SHIFT
    or      a1, a1, a2
    sd      a1, PHY_CTRL_2_ADDR(t7)
1:  
#endif

    //Set Slice 8 parameters
    //write PHY_0 cfg
#ifdef  DDR3_DIMM
    ld      a1, PHY_CTRL_0_4_ADDR(t7)
    dsrl    a1, a1, PHY_CTRL_0_4_OFFSET
#else
    ld      a1, PHY_CTRL_0_3_ADDR(t7)
    dsrl    a1, a1, PHY_CTRL_0_3_OFFSET
#endif
    dli     a2, PHY_CTRL_0_MASK
    and     a0, a1, a2
    ld      a1, PHY_CTRL_0_8_ADDR(t7)
    dli     a2, PHY_CTRL_0_MASK
    dsll    a2, a2, PHY_CTRL_0_8_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, a0, PHY_CTRL_0_8_OFFSET
    or      a1, a1, a2
    sd      a1, PHY_CTRL_0_8_ADDR(t7)

    //write PHY_1 cfg
#ifdef  DDR3_DIMM
    ld      a1, PHY_CTRL_1_4_ADDR(t7)
    dsrl    a1, a1, PHY_CTRL_1_4_OFFSET
#else
    ld      a1, PHY_CTRL_1_3_ADDR(t7)
    dsrl    a1, a1, PHY_CTRL_1_3_OFFSET
#endif
    dli     a2, PHY_CTRL_1_MASK
    and     a0, a1, a2
    ld      a1, PHY_CTRL_1_8_ADDR(t7)
    dli     a2, PHY_CTRL_1_MASK
    dsll    a2, a2, PHY_CTRL_1_8_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, a0, PHY_CTRL_1_8_OFFSET
    or      a1, a1, a2
    sd      a1, PHY_CTRL_1_8_ADDR(t7)

    //write rdlvl_gate_delay
#ifdef  DDR3_DIMM
    ld      a1, RDLVL_GATE_DELAY_4_ADDR(t7)
    dsrl    a1, a1, RDLVL_GATE_DELAY_4_OFFSET
#else
    ld      a1, RDLVL_GATE_DELAY_3_ADDR(t7)
    dsrl    a1, a1, RDLVL_GATE_DELAY_3_OFFSET
#endif
    dli     a2, RDLVL_GATE_DELAY_MASK
    and     a0, a1, a2
    ld      a1, RDLVL_GATE_DELAY_8_ADDR(t7)
    dli     a2, RDLVL_GATE_DELAY_MASK
    dsll    a2, a2, RDLVL_GATE_DELAY_8_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, a0, RDLVL_GATE_DELAY_8_OFFSET
    or      a1, a1, a2
    sd      a1, RDLVL_GATE_DELAY_8_ADDR(t7)

    //write rdlvl_delay_p
#ifdef  DDR3_DIMM
    ld      a1, RDLVL_DELAY_4_ADDR(t7)
    dsrl    a1, a1, RDLVL_DELAY_4_OFFSET
#else
    ld      a1, RDLVL_DELAY_3_ADDR(t7)
    dsrl    a1, a1, RDLVL_DELAY_3_OFFSET
#endif
    dli     a2, RDLVL_DELAY_MASK
    and     a0, a1, a2
    ld      a1, RDLVL_DELAY_8_ADDR(t7)
    dli     a2, RDLVL_DELAY_MASK
    dsll    a2, a2, RDLVL_DELAY_8_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, a0, RDLVL_DELAY_8_OFFSET
    or      a1, a1, a2
    sd      a1, RDLVL_DELAY_8_ADDR(t7)

    //write rdlvl_delay_n
#ifdef  DDR3_DIMM
    ld      a1, RDLVL_DQSN_DELAY_4_ADDR(t7)
    dsrl    a1, a1, RDLVL_DQSN_DELAY_4_OFFSET
#else
    ld      a1, RDLVL_DQSN_DELAY_3_ADDR(t7)
    dsrl    a1, a1, RDLVL_DQSN_DELAY_3_OFFSET
#endif
    dli     a2, RDLVL_DQSN_DELAY_MASK
    and     a0, a1, a2
    ld      a1, RDLVL_DQSN_DELAY_8_ADDR(t7)
    dli     a2, RDLVL_DQSN_DELAY_MASK
    dsll    a2, a2, RDLVL_DQSN_DELAY_8_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, a0, RDLVL_DQSN_DELAY_8_OFFSET
    or      a1, a1, a2
    sd      a1, RDLVL_DQSN_DELAY_8_ADDR(t7)

    //write wrlvl_delay
#ifdef  DDR3_DIMM
    ld      a1, WRLVL_DELAY_4_ADDR(t7)
    dsrl    a1, a1, WRLVL_DELAY_4_OFFSET
#else
    ld      a1, WRLVL_DELAY_3_ADDR(t7)
    dsrl    a1, a1, WRLVL_DELAY_3_OFFSET
#endif
    dli     a2, WRLVL_DELAY_MASK
    and     a0, a1, a2
    ld      a1, WRLVL_DELAY_8_ADDR(t7)
    dli     a2, WRLVL_DELAY_MASK
    dsll    a2, a2, WRLVL_DELAY_8_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, a0, WRLVL_DELAY_8_OFFSET
    or      a1, a1, a2
    sd      a1, WRLVL_DELAY_8_ADDR(t7)

    //write wrlvl_dq_delay
#ifdef  DDR3_DIMM
    ld      a1, WRLVL_DQ_DELAY_4_ADDR(t7)
    dsrl    a1, a1, WRLVL_DQ_DELAY_4_OFFSET
#else
    ld      a1, WRLVL_DQ_DELAY_3_ADDR(t7)
    dsrl    a1, a1, WRLVL_DQ_DELAY_3_OFFSET
#endif
    dli     a2, WRLVL_DQ_DELAY_MASK
    and     a0, a1, a2
    ld      a1, WRLVL_DQ_DELAY_8_ADDR(t7)
    dli     a2, WRLVL_DQ_DELAY_MASK
    dsll    a2, a2, WRLVL_DQ_DELAY_8_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, a0, WRLVL_DQ_DELAY_8_OFFSET
    or      a1, a1, a2
    sd      a1, WRLVL_DQ_DELAY_8_ADDR(t7)
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

    //resync DLL
    //delay some time
    dli     a2, 0x400
1:
    daddiu  a2, a2, -1
    bnez    a2, 1b
    nop
    //PRINTSTR("\r\nResync DLL.")
    dli     a2, 0x1
    dsll    a2, a2, MC_RESYNC_DLL_OFFSET
    ld      a1, MC_RESYNC_DLL_ADDR(t7)
    or      a1, a1, a2
    sd      a1, MC_RESYNC_DLL_ADDR(t7)
    sync

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
    sync
#endif

#ifdef  ADD_DELAY_AFTER_RESET_PHY
    //!!!!!!!!!!!!!!!!!!!!!!!!!!!
    //this delay can't be removed. wired!
    //delay some time, how long is proper?
    dli     a2, MC_RST_DELAY
1:
    daddiu  a2, a2, -1
    bnez    a2, 1b
    nop
#endif
