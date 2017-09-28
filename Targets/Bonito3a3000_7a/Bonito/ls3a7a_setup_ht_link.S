// setup_ht_link
// set up the link between CPU and Bridge.
// for LS3A3000 + LS7A1000

######################################################
#define HT_WIDTH_CTRL_8     (0x00)
#define HT_WIDTH_CTRL_16    (0x11)

######################################################
    .global setup_ht_link
    .ent    setup_ht_link
    .set    noreorder
    .set    mips3
//input:
//a0: HT address base(full address base, like: 0x90001e0000000000)
//a1: HT bus related setting:
//[16]: reset ht when wait linkup timeout;
//[15:12]: GENx-1/3;
//[11: 8]: freq-0: 200M; 2: 400M; 5: 800M; 9: 1600M;
//[ 7: 4]: width-0: 8bit; 1: 16bits;
//[    0]: reconnect, 0: not reconnect; 1: reconnect
//reset when linkup fail seems useless.

//register usage:
//t0: 3A HT cntl register base address
//t1: 7A HT cntl register base address
//t5, t6: tmp variable
//t7: store a1
//s1: store ra

setup_ht_link:

    move    s1, ra

    dli     t0, 0xfdfb000000
    dli     t1, 0xfdfe000000
    daddu   t0, t0, a0
    daddu   t1, t1, a0
    move    t7, a1

#ifdef  DEBUG_HT1   //Print all HT registers
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
    TTYDBG("Wait HT bus up.")
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
    and     a0, t7, 0x1
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
    srl     t5, t7, 4
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
    srl     a1, t7, 8
    and     a1, a1, 0xf
    sll     a1, a1, 8
    or      a0, a0, a1
    sw      a0, 0x4c(t1)

    lw      a0, 0x4c(t1)
    bal     hexserial
    nop
    TTYDBG("\r\n")

    //TODO check these setting
    //TTYDBG("Set HT to soft config 1600\r\n")
    //li      a1, 0x00464083
    //sw      a1, 0x1F4(t1)
    //lw      a0, 0x1F4(t1)
    //bal     hexserial
    //nop
    //TTYDBG("\r\n")

    srl     a1, t7, 12
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
    srl     t5, t7, 4
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
    srl     a1, t7, 8
    and     a1, a1, 0xf
    sb      a1, 0x49(t0)
    lw      a0, 0x48(t0)
    bal     hexserial
    nop
    TTYDBG("\r\n")

    //TTYDBG("Set HT to soft config 1600\r\n")
    //li      a1, 0x00464083
    //sw      a1, 0x178(t0)
    //lw      a0, 0x178(t0)
    //bal     hexserial
    //nop
    //TTYDBG("\r\n")

    srl     a1, t7, 12
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

reset_ht1:
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
    li      t5, 0x1f
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
    lw      a0, 0x54(t0)
    bal     hexserial
    nop
    TTYDBG(">")
    lw      a0, 0x44(t0)
    bal     hexserial
    nop
    TTYDBG("%")
    lw      a0, 0x120(t0)
    bal     hexserial
    nop
    TTYDBG("#")
    addi    t5, t5, -1
    bnez    t5, 2f
    nop
    TTYDBG("\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b=")
    srl     a1, t7, 16
    and     a1, a1, 0x1
    bnez    a1, reset_ht1
    nop

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
8:
#endif

#ifdef  DEBUG_HT1   //Print all HT registers
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

#if 1//Check if CRC error bit set and reset it
crc_checking:
    TTYDBG("Checking CPU HT CRC error.\r\n")

2:
    lw      a0, 0x44(t0)
    li      a1, 0x300
    and     a0, a0, a1
    beqz    a0, 1f
    nop

    TTYDBG("\r\nCRC error found\r\n")
    lw      a0, 0x44(t0)
    bal     hexserial
    nop

    lw      a0, 0x44(t0)
    li      a1, 0xfffffcff
    and     a0, a0, a1
    sw      a0, 0x44(t0)

    b       2b
    nop

1:
    TTYDBG("Checking Bridge HT CRC error bit.\r\n")
2:
    lw      a0, 0x44(t1)
    li      a1, 0x300
    and     a0, a0, a1
    beqz    a0, 1f
    nop

    TTYDBG("\r\nCRC error found\r\n")
    lw      a0, 0x44(t1)
    bal     hexserial
    nop

    lw      a0, 0x44(t1)
    li      a1, 0xfffffcff
    and     a0, a0, a1
    sw      a0, 0x44(t1)

    b       2b
    nop

1:
    TTYDBG("Done\r\n")
#endif

    lw      a0, 0x54(t0)
    bal     hexserial
    nop
    TTYDBG(">")
    lw      a0, 0x44(t0)
    bal     hexserial
    nop
    TTYDBG("%")
    lw      a0, 0x120(t0)
    bal     hexserial
    nop
    TTYDBG("#")

#if 0//Read HT channel priority
    TTYDBG("Read HT Channel priority\r\n")
    lw      a0, 0x50(t0)
    bal     hexserial
    nop
    TTYDBG("\r\n")
#endif

    move    ra, s1
    jr      ra
    nop
    .end    setup_ht_link
