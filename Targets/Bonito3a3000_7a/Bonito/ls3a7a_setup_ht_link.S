// setup_ht_link
// set up the link between CPU and Bridge.
// for LS3A3000 + LS7A1000
// reset when linkup fail seems useless.

######################################################
#define HT_WIDTH_CTRL_8     (0x00)
#define HT_WIDTH_CTRL_16    (0x11)

#define HT1_LINKUP_FAIL_OFFSET      0
#define HT1_LINKDOWN_FAIL_OFFSET    1
#define HT1_CPU_CRC_FAIL_OFFSET     4
#define HT1_CPU_CRC_ERR_OFFSET      5
#define HT1_SB_CRC_FAIL_OFFSET      6
#define HT1_SB_CRC_ERR_OFFSET       7
######################################################
    .global setup_ht_link
    .ent    setup_ht_link
    .set    noreorder
    .set    mips3
//input:
//a0: HT address base(full address base, like: 0x90001e0000000000)
//a1: HT bus and hard freq mode related setting:
//[15:12]: GENx-1/3;
//[11: 8]: freq-0: 200M; 2: 400M; 5: 800M; 9: 1600M;
//[ 7: 4]: width-0: 8bit; 1: 16bits;
//[    0]: reconnect, 0: not reconnect; 1: reconnect
//a2: HT soft configure pll setting:
//[63:32]: 7A side ht_pllcfg[31:0]
//[31: 0]: 3A side ht_pllcfg[31:0]

//output:
//v0: setup_link status:
//0: success
//!0: fail

//register usage:
//t0: 3A HT cntl register base address
//t1: 7A HT cntl register base address
//t5, t6: tmp variable
//t2: store a1
//t3: store a2
//t8: store return value
//s1: store ra

setup_ht_link:

    move    s1, ra

    dli     t0, 0xfdfb000000
    dli     t1, 0xfdfe000000
    daddu   t0, t0, a0
    daddu   t1, t1, a0
    move    t2, a1
    move    t8, $0

#ifdef  PRINT_HT1_REG   //Print all HT registers
    TTYDBG("LS3A HT registers are:\r\n")
    move    t5, t0
    daddu   t6, t5, 0x188
1:
    and     a0, t5, 0x1ff
    bal     hexserial
    nop
    TTYDBG(": ")
    lw      a0, 0x00(t5)
    bal     hexserial
    nop
    TTYDBG("\r\n")

    daddi   t5, t5, 0x4
    bne     t5, t6, 1b
    nop

#if 0
    li      a1, 0x1
    sw      a1, 0x54(t0)

    TTYDBG("after debug enable, LS3A HT registers\r\n")
    move    t5, t0
    daddu   t6, t5, 0x180
1:
    and     a0, t5, 0x1ff
    bal     hexserial
    nop
    TTYDBG(": ")
    lw      a0, 0x0(t5)
    bal     hexserial
    nop
    TTYDBG("\r\n")

    daddi   t5, t5, 0x4
    bne     t5, t6, 1b
    nop
#endif

    TTYDBG("LS7A HT registers are:\r\n")
    move    t5, t1
    daddu   t6, t5, 0x268
1:
    and     a0, t5, 0x3ff
    bal     hexserial
    nop
    TTYDBG(": ")
    lw      a0, 0x00(t5)
    bal     hexserial
    nop
    TTYDBG("\r\n")

    daddi   t5, t5, 0x4
    bne     t5, t6, 1b
    nop
#endif

    //wait until HT link up
    TTYDBG("\r\nWait HT bus up.")
    li      t5, 0x1f
1:
    lw      a0, 0x44(t0)
    bal     hexserial
    nop
    TTYDBG(">")
    addi    t5, t5, -1
    bnez    t5, 2f
    nop
    TTYDBG("\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b=")
    li      t5, 0x1f
2:
    lw      a0, 0x44(t0)
    li      a1, 0x20
    and     a0, a0, a1
    beqz    a0, 1b
    nop

    TTYDBG("\r\n")
    lw      a0, 0x44(t0)
    bal     hexserial
    nop
    TTYDBG("\r\n")

#if 1   //HT1_RECONNECT
    and     a0, t2, 0x1
    beqz    a0, 8f
    nop
    //set 7A side HT
    //TTYDBG("Set HT to 1.0 mode\r\n")
    //lw      a0, 0x4c(t1)
    //li      a1, (0xff<<0)
    //not     a1, a1
    //and     a0, a0, a1
    //li      a1, (0x20<<0)
    //or      a0, a0, a1
    //sw      a0, 0x4c(t1)

    //lw      a0, 0x4c(t1)
    //bal     hexserial
    //nop
    //TTYDBG("\r\n")

    TTYDBG("Set 7A HT width\r\n")
    lw      a0, 0x44(t1)
    li      a1, (0xff<<24)  
    not     a1, a1
    and     a0, a0, a1
    li      a1, HT_WIDTH_CTRL_8
    srl     t5, t2, 4
    and     t5, t5, 0xf
    beqz    t5, 1f
    nop
    li      a1, HT_WIDTH_CTRL_16
1:
    sll     a1, a1, 24
    or      a0, a0, a1
    sw      a0, 0x44(t1)

    lw      a0, 0x44(t1)
    bal     hexserial
    nop
    TTYDBG("\r\n")

    TTYDBG("Set 7A HT Freq\r\n")
    lw      a0, 0x4c(t1)
    li      a1, (0xf<<8)  
    not     a1, a1
    and     a0, a0, a1
    srl     a1, t2, 8
    and     a1, a1, 0xf
    sll     a1, a1, 8
    or      a0, a0, a1
    sw      a0, 0x4c(t1)

    lw      a0, 0x4c(t1)
    bal     hexserial
    nop
    TTYDBG("\r\n")

    TTYDBG("Set soft config\r\n")
    dsrl    a1, t3, 32
    sw      a1, 0x1F4(t1)
    lw      a0, 0x1F4(t1)
    bal     hexserial
    nop
    TTYDBG("\r\n")

    srl     a1, t2, 12
    and     a1, a1, 0xf
    li      a0, 3
    bne     a0, a1, 4f
    nop
    TTYDBG("Set HT to Gen3 mode\r\n")
    lw      a0, 0x6c(t1)
    li      a1, (0xff<<16)  
    not     a1, a1
    and     a0, a0, a1
    li      a1, (0x60<<16)
    or      a0, a0, a1
    sw      a0, 0x6c(t1)

    lw      a0, 0x6c(t1)
    bal     hexserial
    nop
    TTYDBG("\r\n")

    TTYDBG("Set HT to retry mode\r\n")
    lw      a0, 0x64(t1)
    li      a1, (0xc1<<0)  
    not     a1, a1
    and     a0, a0, a1
    li      a1, (0x81<<0)
    or      a0, a0, a1
    sw      a0, 0x64(t1)

    lw      a0, 0x64(t1)
    bal     hexserial
    nop
    TTYDBG("\r\n")

    TTYDBG("Enable HT scrambling\r\n")
    lw      a0, 0xd0(t1)
    li      a1, (0x78<<0)  
    not     a1, a1
    and     a0, a0, a1
    li      a1, (0x78<<0)
    or      a0, a0, a1
    sw      a0, 0xd0(t1)

    lw      a0, 0xd0(t1)
    bal     hexserial
    nop
    TTYDBG("\r\n")
4:

    //Set CPU side HT
    //HT bus width
    TTYDBG("Set CPU HT width\r\n")
    li      a1, HT_WIDTH_CTRL_8
    srl     t5, t2, 4
    and     t5, t5, 0xf
    beqz    t5, 1f
    nop
    li      a1, HT_WIDTH_CTRL_16
1:
    sb      a1, 0x47(t0)
    lw      a0, 0x44(t0)
    bal     hexserial
    nop
    TTYDBG("\r\n")

    //Set HT bus frequency
    TTYDBG("Set CPU HT Freq\r\n")
    srl     a1, t2, 8
    and     a1, a1, 0xf
    sb      a1, 0x49(t0)
    lw      a0, 0x48(t0)
    bal     hexserial
    nop
    TTYDBG("\r\n")

    TTYDBG("Set soft config\r\n")
    move    a1, t3
    sw      a1, 0x178(t0)
    lw      a0, 0x178(t0)
    bal     hexserial
    nop
    TTYDBG("\r\n")

    srl     a1, t2, 12
    and     a1, a1, 0xf
    li      a0, 3
    bne     a0, a1, 4f
    nop
    TTYDBG("Set HT to GEN3 mode\r\n")
    li      a1, 0x88600000
    sw      a1, 0x110(t0)
    lw      a0, 0x110(t0)
    bal     hexserial
    nop
    TTYDBG("\r\n")

    TTYDBG("Set HT to retry mode\r\n")
    li      a1, 0x81
    sb      a1, 0x118(t0)
    lw      a0, 0x118(t0)
    bal     hexserial
    nop
    TTYDBG("\r\n")

    TTYDBG("Enable HT scrambling\r\n")
    li      a1, 0x78
    sb      a1, 0x130(t0)
    lw      a0, 0x130(t0)
    bal     hexserial
    nop
    TTYDBG("\r\n")
4:

    TTYDBG("Reset HT bus\r\n")
    lb      a0, 0x3e(t0)
    li      a1, 0x40
    or      a0, a0, a1
    sb      a0, 0x3e(t0)
    lw      a0, 0x3c(t0)
    bal     hexserial
    nop
    TTYDBG("\r\n")

    //wait until HT link down
    TTYDBG("Wait HT bus down.")
    li      t5, 0x1f
1:
    TTYDBG(">")
    addi    t5, t5, -1
    bnez    t5, 2f
    nop
    TTYDBG("\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b=")
    TTYDBG("Wait HT bus down fail!\r\n")
    //failed more than given time, set fail mark and break
    or      t8, t8, (0x1 << HT1_LINKDOWN_FAIL_OFFSET)
    b       88f
    nop
2:
    lw      a0, 0x44(t0)
    li      a1, 0x20
    and     a0, a0, a1
    bnez    a0, 1b
    nop

    TTYDBG("\r\n")
    lw      a0, 0x44(t0)
    bal     hexserial
    nop
    TTYDBG("\r\n")

    TTYDBG("Dereset HT bus\r\n")
    lb      a0, 0x3e(t0)
    li      a1, 0x40
    not     a1, a1
    and     a0, a0, a1
    sb      a0, 0x3e(t0)
    lw      a0, 0x3c(t0)
    bal     hexserial
    nop
    TTYDBG("\r\n")

    //wait until HT link up
    TTYDBG("Wait HT bus up.")
    li      t5, 0x1f
1:
    TTYDBG(">")
    addi    t5, t5, -1
    bnez    t5, 2f
    nop
    TTYDBG("\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b=")
    TTYDBG("Wait HT bus up fail!\r\n")
    or      t8, t8, (0x1 << HT1_LINKUP_FAIL_OFFSET)
    b       88f
    nop
2:
    lw      a0, 0x44(t0)
    li      a1, 0x20
    and     a0, a0, a1
    beqz    a0, 1b
    nop

    TTYDBG("\r\n")
    lw      a0, 0x44(t0)
    bal     hexserial
    nop
    TTYDBG("\r\n")
8:
#endif

#ifdef  PRINT_HT1_REG   //Print all HT registers
    TTYDBG("LS3A HT registers are:\r\n")
    move    t5, t0
    daddu   t6, t5, 0x188
1:
    and     a0, t5, 0x1ff
    bal     hexserial
    nop
    TTYDBG(": ")
    lw      a0, 0x00(t5)
    bal     hexserial
    nop
    TTYDBG("\r\n")

    daddi   t5, t5, 0x4
    bne     t5, t6, 1b
    nop

#if 0
    TTYDBG("LS7A HT registers are:\r\n")
    move    t5, t1
    daddu   t6, t5, 0x268
1:
    and     a0, t5, 0x3ff
    bal     hexserial
    nop
    TTYDBG(": ")
    lw      a0, 0x00(t5)
    bal     hexserial
    nop
    TTYDBG("\r\n")

    daddi   t5, t5, 0x4
    bne     t5, t6, 1b
    nop
#endif
#endif

#if 1//Check if CRC error bit set and reset it
    TTYDBG("Checking CPU HT CRC error.\r\n")
    li      t5, 0x1f
1:
    TTYDBG(">")
    addi    t5, t5, -1
    bnez    t5, 2f
    nop
    TTYDBG("\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b=")
    or      t8, t8, (0x1 << HT1_CPU_CRC_FAIL_OFFSET)
    b       88f
    nop
2:
    lw      a0, 0x44(t0)
    li      a1, 0x300
    and     a0, a0, a1
    beqz    a0, 3f
    nop
    or      t8, t8, (0x1 << HT1_CPU_CRC_ERR_OFFSET)
    TTYDBG("\r\nCRC error found\r\n")
    lw      a0, 0x44(t0)
    bal     hexserial
    nop

    lw      a0, 0x44(t0)
    li      a1, 0xfffffcff
    and     a0, a0, a1
    sw      a0, 0x44(t0)
    b       1b
    nop
3:
    TTYDBG("Checking Bridge HT CRC error bit.\r\n")
    li      t5, 0x1f
1:
    TTYDBG(">")
    addi    t5, t5, -1
    bnez    t5, 2f
    nop
    TTYDBG("\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b=")
    or      t8, t8, (0x1 << HT1_SB_CRC_FAIL_OFFSET)
    b       88f
    nop
2:
    lw      a0, 0x44(t1)
    li      a1, 0x300
    and     a0, a0, a1
    beqz    a0, 3f
    nop
    or      t8, t8, (0x1 << HT1_SB_CRC_ERR_OFFSET)
    TTYDBG("\r\nCRC error found\r\n")
    lw      a0, 0x44(t1)
    bal     hexserial
    nop

    lw      a0, 0x44(t1)
    li      a1, 0xfffffcff
    and     a0, a0, a1
    sw      a0, 0x44(t1)
    b       1b
    nop
3:
#endif

88:
#ifdef  DEBUG_HT1
    TTYDBG("\r\nCPU HT reg: 0x44: ")
    lw      a0, 0x44(t0)
    bal     hexserial
    nop
    TTYDBG("\r\nCPU HT reg: 0x54: ")
    lw      a0, 0x54(t0)
    bal     hexserial
    nop
    TTYDBG("\r\nCPU HT reg: 0x120: ")
    lw      a0, 0x120(t0)
    bal     hexserial
    nop
    TTYDBG("\r\nCPU HT reg: 0x128: ")
    lw      a0, 0x128(t0)
    bal     hexserial
    nop
    TTYDBG("\r\nCPU HT reg: 0x188: ")
    lw      a0, 0x188(t0)
    bal     hexserial
    nop
    TTYDBG("\r\nCPU HT reg: 0x18c: ")
    lw      a0, 0x18c(t0)
    bal     hexserial
    nop
    TTYDBG("\r\nCPU HT reg: 0x190: ")
    lw      a0, 0x190(t0)
    bal     hexserial
    nop
    TTYDBG("\r\n")
#endif

    move    v0, t8
    move    ra, s1
    jr      ra
    nop
    .end    setup_ht_link
