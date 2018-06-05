//************************************
// HT initialization
// author: chenxk
// date: 2017.8.15
// rewrite by chenxk, 2017.09.03

// include both CPU and PCH side HT window configure and others
// for CPU: LS3A2000/LS3A3000
// for PCH: LS7A1000
// for board: 1way/2way
//************************************
#include "ht.h"

######################################################

    .global ls3a7a_ht_init
    .ent    ls3a7a_ht_init
    .set    noreorder
    .set    mips3

//input:
//a0: HT address base(full address base, like: 0x90001e0000000000)
//register usage:
//t0: 3A HT cntl register base address
//t1: 7A HT cntl register base address
//t5, t6: tmp variable
//s1: store ra
ls3a7a_ht_init:

    move    s1, ra

    dli     t0, 0xfdfb000000
    dli	    t1, 0xfdfe000000
    daddu   t0, t0, a0
    daddu   t1, t1, a0

//1. 3A side HT init
//!!!note: use t0 store HT controller address as global variable
#ifdef  PRINT_HT1_REG
    TTYDBG("LS3A HT registers\r\n")
    move    t5, t0
    daddu   t6, t5, (LS3A_HT_REG_NUM * 4)
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

    TTYDBG("LS7A HT registers\r\n")
    move    t5, t1
    daddu   t6, t5, (LS7A_HT_REG_NUM * 4)
1:
    and     a0, t5, 0x3ff
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

#if 0   //enable HT sample for debug
    li      a1, 0x1
    sw      a1, 0x54(t0)
#endif

    //OPEN RX SPACE in CPU    //window0 00_0000_0000-1f_ffff_ffff
    //cache DMA and uncache DMA are mutual exclusion or co-work together???

    TTYDBG("Open 3A HT RX space.\r\n")
    //translate 0x80000000 ~ 0x90000000 to 0x0 ~ 0x10000000 for legacy kernel
    li      a0, 0x0080fff0
    sw      a0, (LS3A_HT_RX_CACHE_WIN0_OFFSET+4)(t0)
    li      a0, 0xc0000000
    sw      a0, (LS3A_HT_RX_CACHE_WIN0_OFFSET+0)(t0)

    move    a1, msize
    li      a0, 0xff
    and     a1, a0
    sll     a1, 5

    //translate 0x20000000 ~ 0x40000000 to (high mem - 512M)
    //mem start address is 0x80000000
    li      a0, 0x0020ffe0
    sw      a0, (LS3A_HT_RX_CACHE_WIN1_OFFSET+4)(t0)
    li      a0, 0xc0000060 //0x80000000 - 0x20000000
    addu    a0, a1
    sw      a0, (LS3A_HT_RX_CACHE_WIN1_OFFSET+0)(t0)

    //open RX window: 0x0 ~ 0x7f_ffff_ffff
    li      a0, 0x0000e000
    sw      a0, (LS3A_HT_RX_CACHE_WIN2_OFFSET+4)(t0)
    li      a0, 0x80000000
    sw      a0, (LS3A_HT_RX_CACHE_WIN2_OFFSET+0)(t0)

#ifdef MULTI_CHIP
    //related to system chip number, not the 3A - 7A connection
    //recover node 1 access address (bit 37: node id)
    li      a0, 0x2000e000
    sw      a0, (LS3A_HT_RX_CACHE_WIN3_OFFSET+4)(t0)
    li      a0, 0xc0100000
    sw      a0, (LS3A_HT_RX_CACHE_WIN3_OFFSET+0)(t0)
#endif

#if 0   //uncache DMA   currently, configured by kernel
    //translate 0x80000000 ~ 0x90000000 to 0x0 ~ 0x10000000 for legacy kernel
    li      a0, 0x0080fff0
    sw      a0, (LS3A_HT_RX_UNCACHE_WIN0_OFFSET+4)(t0)
    li      a0, 0xc0000000
    sw      a0, (LS3A_HT_RX_UNCACHE_WIN0_OFFSET+0)(t0)

    li      a0, 0x0000e000
    sw      a0, (LS3A_HT_RX_UNCACHE_WIN1_OFFSET+4)(t0)
    li      a0, 0x80000000
    sw      a0, (LS3A_HT_RX_UNCACHE_WIN1_OFFSET+0)(t0)

#ifdef MULTI_CHIP
    //recover node 1 access address (bit 37: node id)
    li      a0, 0x2000e000
    sw      a0, (LS3A_HT_RX_UNCACHE_WIN2_OFFSET+4)(t0)
    li      a0, 0xc0100000
    sw      a0, (LS3A_HT_RX_UNCACHE_WIN2_OFFSET+0)(t0)
#endif
#endif
    //set csr_dw_write to 1'b0 to transfer write mask infomation when write data less than 32Byte
    TTYDBG("Disable 3A HT dw_write.\r\n")
    lw      a0, 0x50(t0)
    li      a1, 0xff7fffff
    and     a0, a0,a1
    sw      a0, 0x50(t0)

//2. 7A side HT configure begin
//!!!note: use t1 store 7A side HT controller address as global variable
    //open RX space
    TTYDBG("Open 7A HT RX space.\r\n")
    //window0 00_0000_0000-7f_ffff_ffff
    li      a1, 0x00008000
    sw      a1, (LS7A_HT_RX_WIN0_OFFSET+4)(t1)
    li      a1, 0x80000000
    sw      a1, (LS7A_HT_RX_WIN0_OFFSET+0)(t1)

    //window1 fd_fc00_0000-fd_fcff_ffff
    li      a1, 0xfdfcffff
    sw      a1, (LS7A_HT_RX_WIN1_OFFSET+4)(t1)
    li      a1, 0x80000000
    sw      a1, (LS7A_HT_RX_WIN1_OFFSET+0)(t1)

#if 0   //enable DMA post write
    TTYDBG("Enable 7A HT Post space.\r\n")
    //window0 00_0000_0000-7f_ffff_ffff
    li      a1, 0x00008000
    sw      a1, (LS7A_HT_TX_POST_WIN0_OFFSET+4)(t1)
    li      a1, 0x80000000 
    sw      a1, (LS7A_HT_TX_POST_WIN0_OFFSET+0)(t1)
#endif
    
    //set csr_dw_write to 1'b0 to transfer write mask infomation when write data less than 32Byte
    TTYDBG("Disable 7A HT dw_write.\r\n")
    lw      a0, 0x1c0(t1)
    li      a1, 0xfbffffff
    and     a0, a0,a1
    sw      a0, 0x1c0(t1)

    //enable HT configure access
    TTYDBG("Enable 7A HT configure access.\r\n")
    li      a0, 0xc000fe00
    sw      a0, 0x168(t1)

#if 0   //def  DEBUG_HT1
    PRINTSTR("\r\nChange some registers of 7A HT:");
1:
    PRINTSTR("\r\ninput the HT register offset you want to change!!!(0xfff:jump out.): ");
    dli     t6, 0x00
    bal     inputaddress
    nop
    move    t5, v0

    li      a1, (LS7A_HT_REG_NUM * 4)
    bge     t5, a1, 4f    #if input address offset exceed range,jump out
    nop

    and     t5, t5, 0x3fc
    daddu   t5, t5, t1

    PRINTSTR("\r\nPlease input the data-hex(32bit): ");
    dli     t6, 0x00
    bal     inputaddress
    nop
    sw      v0, 0x0(t5)    #v0 is the input value

    //print the new register value
    move    t6, t5
    PRINTSTR("\r\nRegister 0x")
    dsubu   t5, t5, t1
    move    a0, t5
    bal     hexserial
    nop
    PRINTSTR(": ")
    lw      t6, 0x0(t6)
    move    a0, t6
    bal     hexserial
    nop

    b       1b
    nop
4:
#endif

#ifdef  PRINT_HT1_REG
    TTYDBG("LS3A HT registers\r\n")
    move    t5, t0
    daddu   t6, t5, (LS7A_HT_REG_NUM * 4)
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

    TTYDBG("LS7A HT registers\r\n")
    move    t5, t1
    daddu   t6, t5, (LS7A_HT_REG_NUM * 4)
1:
    and     a0, t5, 0x3ff
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
    
    move    ra, s1
    jr      ra
    nop
    .end    ls3a7a_ht_init
