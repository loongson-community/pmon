/*******************
input param:
    t2: current test param value
*******************/

#if 0
    //delay to wait no rw op to Mem
    dli     a2, 0x400000
1:
    daddiu  a2, a2, -1
    bnez    a2, 1b
    nop
#endif
#ifndef LS3B
    //enable DDR MC register config space
    //PRINTSTR("\r\nEnable DDR MC config space.");
    dli     t7, 0x900000001fe00180
    GET_ARB_LEVEL_NODE_ID
    dsll    a1, a1, 44
    or      t7, t7, a1
    lw      a1, 0x0(t7)
    li      a2, 0xfffffeff
    and     a1, a1, a2
    sw      a1, 0x0(t7)
    sync
#endif
    
    //clear param_start
    //PRINTSTR("\r\nClear param_start.");
    dli     t7, DDR_MC_CONFIG_BASE
    GET_ARB_LEVEL_NODE_ID
    dsll    a1, a1, 44
    or      t7, t7, a1
    dli     a2, 0xff
    dsll    a2, a2, START_OFFSET
    not     a2, a2
    ld      a1, START_ADDR(t7)
    and     a1, a1, a2
    sd      a1, START_ADDR(t7)
    sync

#if 1
    //delay some time
    dli     a2, 0x400
1:
    daddiu  a2, a2, -1
    bnez    a2, 1b
    nop
#endif

    //change config param
    and     t2, t2, 0xff

    ld      a1, PAD_CTRL_REG_ADDR(t7)
    dli     a2, PAD_CTRL_COMP_MASK
    dsll    a2, a2, PAD_CTRL_COMP_OFFSET
    not     a2, a2
    and     a1, a1, a2
    dsll    a2, t2, PAD_CTRL_COMP_OFFSET
    or      a1, a1, a2
    sd      a1, PAD_CTRL_REG_ADDR(t7)

    sync

    //set start to 1
    //PRINTSTR("\r\nSet param_start 1.");
    dli     a2, 0x1
    dsll    a2, a2, START_OFFSET
    ld      a1, START_ADDR(t7)
    or      a1, a1, a2
    sd      a1, START_ADDR(t7)
    sync

    //poll until init completed
    dli     a2, 0x100
1:
    ld      a1, INT_STATUS_ADDR(t7)
    and     a1, a1, a2
    beqz    a1, 1b
    nop

#ifndef LS3B
    //disable DDR MC config reg space
    dli     t7, 0x900000001fe00180
    GET_ARB_LEVEL_NODE_ID
    dsll    a1, a1, 44
    or      t7, t7, a1
    lw      a1, 0x0(t7)
    or      a1, a1, 0x100
    sw      a1, 0x0(t7)
    sync        //this doesn't matter
    //PRINTSTR("\r\nDisable DDR MC config space.\r\n");
#endif
#if 1
    //!!!!!!!!!!!!!!!!!!!!!!!!!!!
    //this delay can't be removed. wired!
    //delay some time, how long is proper?
    //dli     a2, 0x4000000     //work good.
    //dli     a2, 0x1000000   //seems not good.
    dli     a2, 0x4000000
1:
    daddiu  a2, a2, -1
    bnez    a2, 1b
    nop
#endif
