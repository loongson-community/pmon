/*******************
input param:
    t6: 1 ~ 6, param to be changed
    t2: change direction. 1: decrease; 0: increase;
output:
    t5: bit[63]: reach the maximum value.
        bit[3:0]: value after updating
*******************/
/**************************
t6:
//3 bit
 1: w2r_samecs_dly
 2: w2r_diffcs_dly
 3: w2w_samecs_dly
 4: w2w_diffcs_dly
 5: r2w_samecs_dly
 6: r2w_diffcs_dly
 7: r2r_samecs_dly
 8: r2r_diffcs_dly
 9: add_odt_clk_difftype_diffcs
10: add_odt_clk_sametype_diffcs
11: add_odt_clk_difftype_samecs
//4 bit
**************************/

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
#if 1
    //PRINTSTR("\r\nChange param value.");

//!!!!!note: don't change the switch order of the code bellow, because we use
//add instr to change a1 instead of dli instr to reduce code size.
    //set modify target to a2
    andi    a2, t6, 0xf

    dli     a1, 0x1
    beq     a2, a1, 1f;
    nop
    daddiu  a1, a1, 0x1
    beq     a2, a1, 2f;
    nop
    daddiu  a1, a1, 0x1
    beq     a2, a1, 3f;
    nop
    daddiu  a1, a1, 0x1
    beq     a2, a1, 4f;
    nop
    daddiu  a1, a1, 0x1
    beq     a2, a1, 5f;
    nop
    daddiu  a1, a1, 0x1
    beq     a2, a1, 6f;
    nop
    daddiu  a1, a1, 0x1
    beq     a2, a1, 7f;
    nop
    daddiu  a1, a1, 0x1
    beq     a2, a1, 8f;
    nop
    daddiu  a1, a1, 0x1
    beq     a2, a1, 9f;
    nop
    daddiu  a1, a1, 0x1
    beq     a2, a1, 10f;
    nop
    daddiu  a1, a1, 0x1
    beq     a2, a1, 11f;
    nop
    //PRINTSTR("\r\n--------Wrong selection: no parameter will be changed.");
    b       40f
    nop
1:
    ld      a1, W2R_SAMECS_DLY_ADDR(t7)
    dli     a2, ONE_BYTE_MASK
    dsll    a2, a2, W2R_SAMECS_DLY_OFFSET
    and     a2, a1, a2
    dsrl    t5, a2, W2R_SAMECS_DLY_OFFSET
    beqz    t2, 35f
    nop
    //decrease
    beqz    t5, 38f //reach zero
    nop
    daddu   t5, t5, -0x1
    b       38f
    nop
35: //increase
    dli     a2, THREE_BITS_MASK
    and     a2, a2, t5
    beq     a2, t5, 33f
    nop
    daddu   t5, t5, 0x1
    b       38f
    nop
33: //reach the maximum, set mark
    dli     a2, 0x1
    dsll    a2, a2, 63
    or      t5, t5, a2
38:
    dli     a2, ONE_BYTE_MASK
    dsll    a2, a2, W2R_SAMECS_DLY_OFFSET
    not     a2, a2
    and     a1, a1, a2
    and     a2, t5, 0xff
    dsll    a2, a2, W2R_SAMECS_DLY_OFFSET
    or      a1, a1, a2
    sd      a1, W2R_SAMECS_DLY_ADDR(t7)

    b       40f
    nop
2:
    ld      a1, W2R_DIFFCS_DLY_ADDR(t7)
    dli     a2, ONE_BYTE_MASK
    dsll    a2, a2, W2R_DIFFCS_DLY_OFFSET
    and     a2, a1, a2
    dsrl    t5, a2, W2R_DIFFCS_DLY_OFFSET
    beqz    t2, 35f
    nop
    //decrease
    beqz    t5, 38f //reach zero
    nop
    daddu   t5, t5, -0x1
    b       38f
    nop
35: //increase
    dli     a2, THREE_BITS_MASK
    and     a2, a2, t5
    beq     a2, t5, 33f
    nop
    daddu   t5, t5, 0x1
    b       38f
    nop
33: //reach the maximum, set mark
    dli     a2, 0x1
    dsll    a2, a2, 63
    or      t5, t5, a2
38:
    dli     a2, ONE_BYTE_MASK
    dsll    a2, a2, W2R_DIFFCS_DLY_OFFSET
    not     a2, a2
    and     a1, a1, a2
    and     a2, t5, 0xff
    dsll    a2, a2, W2R_DIFFCS_DLY_OFFSET
    or      a1, a1, a2
    sd      a1, W2R_DIFFCS_DLY_ADDR(t7)

    b       40f
    nop
3:
    ld      a1, W2W_SAMECS_DLY_ADDR(t7)
    dli     a2, ONE_BYTE_MASK
    dsll    a2, a2, W2W_SAMECS_DLY_OFFSET
    and     a2, a1, a2
    dsrl    t5, a2, W2W_SAMECS_DLY_OFFSET
    beqz    t2, 35f
    nop
    //decrease
    beqz    t5, 38f //reach zero
    nop
    daddu   t5, t5, -0x1
    b       38f
    nop
35: //increase
    dli     a2, THREE_BITS_MASK
    and     a2, a2, t5
    beq     a2, t5, 33f
    nop
    daddu   t5, t5, 0x1
    b       38f
    nop
33: //reach the maximum, set mark
    dli     a2, 0x1
    dsll    a2, a2, 63
    or      t5, t5, a2
38:
    dli     a2, ONE_BYTE_MASK
    dsll    a2, a2, W2W_SAMECS_DLY_OFFSET
    not     a2, a2
    and     a1, a1, a2
    and     a2, t5, 0xff
    dsll    a2, a2, W2W_SAMECS_DLY_OFFSET
    or      a1, a1, a2
    sd      a1, W2W_SAMECS_DLY_ADDR(t7)

    b       40f
    nop
4:
    ld      a1, W2W_DIFFCS_DLY_ADDR(t7)
    dli     a2, ONE_BYTE_MASK
    dsll    a2, a2, W2W_DIFFCS_DLY_OFFSET
    and     a2, a1, a2
    dsrl    t5, a2, W2W_DIFFCS_DLY_OFFSET
    beqz    t2, 35f
    nop
    //decrease
    beqz    t5, 38f //reach zero
    nop
    daddu   t5, t5, -0x1
    b       38f
    nop
35: //increase
    dli     a2, THREE_BITS_MASK
    and     a2, a2, t5
    beq     a2, t5, 33f
    nop
    daddu   t5, t5, 0x1
    b       38f
    nop
33: //reach the maximum, set mark
    dli     a2, 0x1
    dsll    a2, a2, 63
    or      t5, t5, a2
38:
    dli     a2, ONE_BYTE_MASK
    dsll    a2, a2, W2W_DIFFCS_DLY_OFFSET
    not     a2, a2
    and     a1, a1, a2
    and     a2, t5, 0xff
    dsll    a2, a2, W2W_DIFFCS_DLY_OFFSET
    or      a1, a1, a2
    sd      a1, W2W_DIFFCS_DLY_ADDR(t7)

    b       40f
    nop
5:
    ld      a1, R2W_SAMECS_DLY_ADDR(t7)
    dli     a2, ONE_BYTE_MASK
    dsll    a2, a2, R2W_SAMECS_DLY_OFFSET
    and     a2, a1, a2
    dsrl    t5, a2, R2W_SAMECS_DLY_OFFSET
    beqz    t2, 35f
    nop
    //decrease
    beqz    t5, 38f //reach zero
    nop
    daddu   t5, t5, -0x1
    b       38f
    nop
35: //increase
    dli     a2, THREE_BITS_MASK
    and     a2, a2, t5
    beq     a2, t5, 33f
    nop
    daddu   t5, t5, 0x1
    b       38f
    nop
33: //reach the maximum, set mark
    dli     a2, 0x1
    dsll    a2, a2, 63
    or      t5, t5, a2
38:
    dli     a2, ONE_BYTE_MASK
    dsll    a2, a2, R2W_SAMECS_DLY_OFFSET
    not     a2, a2
    and     a1, a1, a2
    and     a2, t5, 0xff
    dsll    a2, a2, R2W_SAMECS_DLY_OFFSET
    or      a1, a1, a2
    sd      a1, R2W_SAMECS_DLY_ADDR(t7)

    b       40f
    nop
6:
    ld      a1, R2W_DIFFCS_DLY_ADDR(t7)
    dli     a2, ONE_BYTE_MASK
    dsll    a2, a2, R2W_DIFFCS_DLY_OFFSET
    and     a2, a1, a2
    dsrl    t5, a2, R2W_DIFFCS_DLY_OFFSET
    beqz    t2, 35f
    nop
    //decrease
    beqz    t5, 38f //reach zero
    nop
    daddu   t5, t5, -0x1
    b       38f
    nop
35: //increase
    dli     a2, THREE_BITS_MASK
    and     a2, a2, t5
    beq     a2, t5, 33f
    nop
    daddu   t5, t5, 0x1
    b       38f
    nop
33: //reach the maximum, set mark
    dli     a2, 0x1
    dsll    a2, a2, 63
    or      t5, t5, a2
38:
    dli     a2, ONE_BYTE_MASK
    dsll    a2, a2, R2W_DIFFCS_DLY_OFFSET
    not     a2, a2
    and     a1, a1, a2
    and     a2, t5, 0xff
    dsll    a2, a2, R2W_DIFFCS_DLY_OFFSET
    or      a1, a1, a2
    sd      a1, R2W_DIFFCS_DLY_ADDR(t7)

    b       40f
    nop
7:
    ld      a1, R2R_SAMECS_DLY_ADDR(t7)
    dli     a2, ONE_BYTE_MASK
    dsll    a2, a2, R2R_SAMECS_DLY_OFFSET
    and     a2, a1, a2
    dsrl    t5, a2, R2R_SAMECS_DLY_OFFSET
    beqz    t2, 35f
    nop
    //decrease
    beqz    t5, 38f //reach zero
    nop
    daddu   t5, t5, -0x1
    b       38f
    nop
35: //increase
    dli     a2, THREE_BITS_MASK
    and     a2, a2, t5
    beq     a2, t5, 33f
    nop
    daddu   t5, t5, 0x1
    b       38f
    nop
33: //reach the maximum, set mark
    dli     a2, 0x1
    dsll    a2, a2, 63
    or      t5, t5, a2
38:
    dli     a2, ONE_BYTE_MASK
    dsll    a2, a2, R2R_SAMECS_DLY_OFFSET
    not     a2, a2
    and     a1, a1, a2
    and     a2, t5, 0xff
    dsll    a2, a2, R2R_SAMECS_DLY_OFFSET
    or      a1, a1, a2
    sd      a1, R2R_SAMECS_DLY_ADDR(t7)

    b       40f
    nop
8:
    ld      a1, R2R_DIFFCS_DLY_ADDR(t7)
    dli     a2, ONE_BYTE_MASK
    dsll    a2, a2, R2R_DIFFCS_DLY_OFFSET
    and     a2, a1, a2
    dsrl    t5, a2, R2R_DIFFCS_DLY_OFFSET
    beqz    t2, 35f
    nop
    //decrease
    beqz    t5, 38f //reach zero
    nop
    daddu   t5, t5, -0x1
    b       38f
    nop
35: //increase
    dli     a2, THREE_BITS_MASK
    and     a2, a2, t5
    beq     a2, t5, 33f
    nop
    daddu   t5, t5, 0x1
    b       38f
    nop
33: //reach the maximum, set mark
    dli     a2, 0x1
    dsll    a2, a2, 63
    or      t5, t5, a2
38:
    dli     a2, ONE_BYTE_MASK
    dsll    a2, a2, R2R_DIFFCS_DLY_OFFSET
    not     a2, a2
    and     a1, a1, a2
    and     a2, t5, 0xff
    dsll    a2, a2, R2R_DIFFCS_DLY_OFFSET
    or      a1, a1, a2
    sd      a1, R2R_DIFFCS_DLY_ADDR(t7)

    b       40f
    nop
9:
    ld      a1, ADD_ODT_CLK_DTDC_ADDR(t7)
    dli     a2, ONE_BYTE_MASK
    dsll    a2, a2, ADD_ODT_CLK_DTDC_OFFSET
    and     a2, a1, a2
    dsrl    t5, a2, ADD_ODT_CLK_DTDC_OFFSET
    beqz    t2, 35f
    nop
    //decrease
    beqz    t5, 38f //reach zero
    nop
    daddu   t5, t5, -0x1
    b       38f
    nop
35: //increase
    dli     a2, FOUR_BITS_MASK
    and     a2, a2, t5
    beq     a2, t5, 33f
    nop
    daddu   t5, t5, 0x1
    b       38f
    nop
33: //reach the maximum, set mark
    dli     a2, 0x1
    dsll    a2, a2, 63
    or      t5, t5, a2
38:
    dli     a2, ONE_BYTE_MASK
    dsll    a2, a2, ADD_ODT_CLK_DTDC_OFFSET
    not     a2, a2
    and     a1, a1, a2
    and     a2, t5, 0xff
    dsll    a2, a2, ADD_ODT_CLK_DTDC_OFFSET
    or      a1, a1, a2
    sd      a1, ADD_ODT_CLK_DTDC_ADDR(t7)

    b       40f
    nop
10:
    ld      a1, ADD_ODT_CLK_STDC_ADDR(t7)
    dli     a2, ONE_BYTE_MASK
    dsll    a2, a2, ADD_ODT_CLK_STDC_OFFSET
    and     a2, a1, a2
    dsrl    t5, a2, ADD_ODT_CLK_STDC_OFFSET
    beqz    t2, 35f
    nop
    //decrease
    beqz    t5, 38f //reach zero
    nop
    daddu   t5, t5, -0x1
    b       38f
    nop
35: //increase
    dli     a2, FOUR_BITS_MASK
    and     a2, a2, t5
    beq     a2, t5, 33f
    nop
    daddu   t5, t5, 0x1
    b       38f
    nop
33: //reach the maximum, set mark
    dli     a2, 0x1
    dsll    a2, a2, 63
    or      t5, t5, a2
38:
    dli     a2, ONE_BYTE_MASK
    dsll    a2, a2, ADD_ODT_CLK_STDC_OFFSET
    not     a2, a2
    and     a1, a1, a2
    and     a2, t5, 0xff
    dsll    a2, a2, ADD_ODT_CLK_STDC_OFFSET
    or      a1, a1, a2
    sd      a1, ADD_ODT_CLK_STDC_ADDR(t7)

    b       40f
    nop
11:
    ld      a1, ADD_ODT_CLK_DTSC_ADDR(t7)
    dli     a2, ONE_BYTE_MASK
    dsll    a2, a2, ADD_ODT_CLK_DTSC_OFFSET
    and     a2, a1, a2
    dsrl    t5, a2, ADD_ODT_CLK_DTSC_OFFSET
    beqz    t2, 35f
    nop
    //decrease
    beqz    t5, 38f //reach zero
    nop
    daddu   t5, t5, -0x1
    b       38f
    nop
35: //increase
    dli     a2, FOUR_BITS_MASK
    and     a2, a2, t5
    beq     a2, t5, 33f
    nop
    daddu   t5, t5, 0x1
    b       38f
    nop
33: //reach the maximum, set mark
    dli     a2, 0x1
    dsll    a2, a2, 63
    or      t5, t5, a2
38:
    dli     a2, ONE_BYTE_MASK
    dsll    a2, a2, ADD_ODT_CLK_DTSC_OFFSET
    not     a2, a2
    and     a1, a1, a2
    and     a2, t5, 0xff
    dsll    a2, a2, ADD_ODT_CLK_DTSC_OFFSET
    or      a1, a1, a2
    sd      a1, ADD_ODT_CLK_DTSC_ADDR(t7)

    b       40f
    nop
40:
    sync
#endif
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
    dli     a2, 0x4000
1:
    daddiu  a2, a2, -1
    bnez    a2, 1b
    nop
#endif
