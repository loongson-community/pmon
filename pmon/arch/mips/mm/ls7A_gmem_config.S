#include "lsmc7a_config_param.S"

        .global ls7a_gmem_init
        .ent    ls7a_gmem_init
        .set    noreorder
        .set    mips3
ls7a_gmem_init:

    move    t7, ra

    TTYDBG("Enable gmem config space\r\n")
    dli     t0, LS7A_CONFBUS_BASE_ADDR
    lw      t1, 0x424(t0)
    li      t2, (0x1 << 24)
    not     t2, t2
    and     t1, t1, t2
    sw      t1, 0x424(t0)
  
#if 0 //def  DEBUG_GMEM_PARAM
    dli     t0, LS7A_GMEM_TEMP_ADDR

    TTYDBG("Gmem parameter is:\r\n")
    dli     t4, DDR_PARAM_NUM
1:
    ld      t1, 0x0(t0)

    and     a0, t0, 0xfff
    bal     hexserial
    nop
    TTYDBG(":  ")
    dsrl    a0, t1, 32
    bal     hexserial
    nop
    move    a0, t1
    bal     hexserial
    nop
    TTYDBG("\r\n")

    daddiu  t4, t4, -1
    daddiu  t0, t0, 8
    bnez    t4, 1b
    nop
#endif

    dla     a2, gmem_reg_data
    dli     t8, LS7A_GMEM_TEMP_ADDR
    bal     ls7a_ddr2_config
    nop

#ifndef DISABLE_HARD_LEVELING
    dli     t8, LS7A_GMEM_TEMP_ADDR
    bal     ddr3_leveling
    nop
#endif

#ifdef  DEBUG_GMEM_PARAM
    TTYDBG("Gmem parameter after config is:\r\n")

    dli     t0, LS7A_GMEM_TEMP_ADDR

    dli     t4, DDR_PARAM_NUM
1:
    ld      t1, 0x0(t0)

    move    a0, t0
    and     a0, a0, 0xfff
    bal     hexserial
    nop
    TTYDBG(":  ")
    dsrl    a0, t1, 32
    bal     hexserial
    nop
    move    a0, t1
    bal     hexserial
    nop
    TTYDBG("\r\n")

    daddiu  t4, t4, -1
    daddiu  t0, t0, 8
    bnez    t4, 1b
    nop
#endif

    TTYDBG("Disable gmem config space\r\n")
    dli     t0, LS7A_CONFBUS_BASE_ADDR
    lw      t1, 0x424(t0)
    li      t2, (0x1 << 24)
    or      t1, t1, t2
    sw      t1, 0x424(t0)
  
    move    ra, t7
    jr      ra
    nop
    .end    ls7a_gmem_init
