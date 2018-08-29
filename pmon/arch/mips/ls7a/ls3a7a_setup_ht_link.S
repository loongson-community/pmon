//************************************
// setup_ht_link
// author: chenxk
// date: 2017.11.14
// set up the link between CPU and PCH include two steps:
// 1. config_ht_link(set link parameters)
// 2. reset_ht_link(reset to make new configure effect)
//
// for CPU: LS3A2000/LS3A3000
// for PCH: LS7A1000
//************************************
#include "ht.h"

    .global config_ht_link
    .ent    config_ht_link
    .set    noreorder
    .set    mips3
//input:
//a0: HT address base(full address base, like: 0x90001e0000000000)
//a1: HT bus and hard freq mode related setting:
//[15:12]: 7A freq-0: 200M; 2: 400M; 5: 800M; 9: 1600M;
//[11: 8]: 3A freq-0: 200M; 2: 400M; 5: 800M; 9: 1600M;
//[ 7: 4]: GENx-1/3;
//[    1]: width-0: 8bit; 1: 16bits;
//[    0]: reconnect, 0: not reconnect; 1: reconnect
//a2: HT soft configure pll setting:
//[63:32]: 7A side ht_pllcfg[31:0]
//[31: 0]: 3A side ht_pllcfg[31:0]

//register usage:
//t0: 3A HT cntl register base address
//t1: 7A HT cntl register base address
//t5, t6: tmp variable
//t2: store a1
//t3: store a2
//s1: store ra

config_ht_link:

    move    s1, ra

    dli     t0, 0xfdfb000000
    dli     t1, 0xfdfe000000
    daddu   t0, t0, a0
    daddu   t1, t1, a0
    move    t2, a1
    move    t3, a2

#ifdef  PRINT_HT1_REG   //Print all HT registers
    TTYDBG("LS3A HT registers are:\r\n")
    move    t5, t0
    daddu   t6, t5, (LS3A_HT_REG_NUM * 4)
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
    daddu   t6, t5, (LS7A_HT_REG_NUM * 4)
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
#ifdef  DEBUG_HT1_PARAM
    PRINTSTR("\r\nChange some registers of 3A HT:");
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
    daddu   t5, t5, t0

    PRINTSTR("\r\nPlease input the data-hex(32bit): ");
    dli     t6, 0x00
    bal     inputaddress
    nop
    sw      v0, 0x0(t5)    #v0 is the input value

    //print the new register value
    move    t6, t5
    PRINTSTR("\r\nRegister 0x")
    dsubu   t5, t5, t0
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

#ifdef  DEBUG_HT1_PARAM
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
#ifdef  CHECK_HT_PLL_LOCK
    //check HT PLL lock
    move    t5, $0
    lw      a0, 0x178(t0)
    srl     a0, a0, 3
    and     a0, a0, 0x1
    bnez    a0, 2f
    nop
    PRINTSTR("\r\nError: After reconfigure, 3A HT PLL not locked!!!")
    add     t5, t5, 1
2:
#ifdef  CHECK_7A_HT_PLL_LOCK
    lw      a0, 0x1F4(t1)
    srl     a0, a0, 3
    and     a0, a0, 0x1
    bnez    a0, 2f
    nop
    PRINTSTR("\r\nError: After reconfigure, 7A HT PLL not locked!!!")
    add     t5, t5, 1
2:
#endif
    bnez    t5, 2b
    nop
    TTYDBG("\r\nPLL check success.")
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

#if HT1_RECONNECT
    and     a0, t2, 0x1
    beqz    a0, 8f
    nop

    TTYDBG("Set 7A side HT:\r\n")
    TTYDBG("Set width\r\n")
    lw      a0, 0x44(t1)
    li      a1, (0xff<<24)  
    not     a1, a1
    and     a0, a0, a1
    li      a1, HT_WIDTH_CTRL_8
    srl     t5, t2, 1
    and     t5, t5, 0x1
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

    TTYDBG("Set Freq\r\n")
    lw      a0, 0x4c(t1)
    li      a1, (0xf<<8)  
    not     a1, a1
    and     a0, a0, a1
    srl     a1, t2, 12
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

    srl     a1, t2, 4
    and     a1, a1, 0xf
    li      a0, 3
    bne     a0, a1, 4f
    nop
    TTYDBG("Set Gen3 mode\r\n")
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

    TTYDBG("Set retry mode\r\n")
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

    TTYDBG("Enable scrambling\r\n")
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
    TTYDBG("set buffer num\r\n")
    lw      a0, 0x1dc(t1)
    li      a1, 0xfffffff 
    or      a0, a0, a1
    sw      a0, 0x1dc(t1)

    lw      a0, 0x1dc(t1)
    bal     hexserial
    nop
    TTYDBG("\r\n")

#ifdef  DEBUG_HT1_PARAM
    PRINTSTR("\r\nChange some parameters of HT:");
1:
    PRINTSTR("\r\nPlease input the register address you want to change!!!(0xfff:jump out.): ");
    dli     t6, 0x00
    bal     inputaddress
    nop
    move    t5, v0

    dli     a1, (LS7A_HT_REG_NUM * 4)
    bge     t5, a1, 2f    #if input address offset exceed range,jump out
    nop
    and     t5, t5, 0xffc
    daddu   t5, t5, t1

    PRINTSTR("\r\nPlease input the data-hex: ");
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
2:
#endif

    //Set CPU side HT
    TTYDBG("Set CPU side HT:\r\n")
    //HT bus width
    TTYDBG("Set width\r\n")
    li      a1, HT_WIDTH_CTRL_8
    srl     t5, t2, 1
    and     t5, t5, 0x1
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
    TTYDBG("Set Freq\r\n")
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

    srl     a1, t2, 4
    and     a1, a1, 0xf
    li      a0, 3
    bne     a0, a1, 4f
    nop
    TTYDBG("Set GEN3 mode\r\n")
    li      a1, 0x88600000
    sw      a1, 0x110(t0)
    lw      a0, 0x110(t0)
    bal     hexserial
    nop
    TTYDBG("\r\n")

    TTYDBG("Set retry mode\r\n")
    li      a1, 0x81
    sb      a1, 0x118(t0)
    lw      a0, 0x118(t0)
    bal     hexserial
    nop
    TTYDBG("\r\n")

    TTYDBG("Enable scrambling\r\n")
    li      a1, 0x78
    sb      a1, 0x130(t0)
    lw      a0, 0x130(t0)
    bal     hexserial
    nop
    TTYDBG("\r\n")
4:
    TTYDBG("set buffer num\r\n")
    lw      a0, 0x5c(t0)
    li      a1, 0xfffffff 
    or      a0, a0, a1
    sw      a0, 0x5c(t0)

    lw      a0, 0x5c(t0)
    bal     hexserial
    nop
    TTYDBG("\r\n")

#ifdef  DEBUG_HT1_PARAM
    PRINTSTR("\r\nChange some parameters of HT:");
1:
    PRINTSTR("\r\nPlease input the register address you want to change!!!(0xfff:jump out.): ");
    dli     t6, 0x00
    bal     inputaddress
    nop
    move    t5, v0

    dli     a1, (LS3A_HT_REG_NUM * 4)
    bge     t5, a1, 2f    #if input address offset exceed range,jump out
    nop
    and     t5, t5, 0xffc
    daddu   t5, t5, t0

    PRINTSTR("\r\nPlease input the data-hex: ");
    dli     t6, 0x00
    bal     inputaddress
    nop
    sw      v0, 0x0(t5)    #v0 is the input value

    //print the new register value
    move    t6, t5
    PRINTSTR("\r\nRegister 0x")
    dsubu   t5, t5, t0
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
2:
#endif
8:
#endif

    move    ra, s1
    jr      ra
    nop
    .end    config_ht_link

######################################################
#define MAX_HT_CRC_ERR_COUNT    256

#define HT1_LINKUP_FAIL_OFFSET      0
#define HT1_LINKDOWN_FAIL_OFFSET    1
#define HT1_CPU_CRC_FAIL_OFFSET     4
#define HT1_CPU_CRC_ERR_OFFSET      5
#define HT1_SB_CRC_FAIL_OFFSET      6
#define HT1_SB_CRC_ERR_OFFSET       7

#define N1_OFFSET   8

    .global reset_ht_link
    .ent    reset_ht_link
    .set    noreorder
    .set    mips3

// support 1 way and 2 way connection.
// Because of the stupid bug, we have to keep all HT1 link reset together.
// 
// reset multi-times seems useless when linkup fail.

//input:
//a1: HT bus and hard freq mode related setting
//[    0]: reconnect, 0: not reconnect; 1: reconnect

//output:
//v0: setup_link status:
//0: success
//!0: fail

//register usage:
//t0: Node 0 LS3A HT1 cntl register base address
//t1: Node 0 LS7A HT1 cntl register base address
//t2: Node 1 LS3A HT1 cntl register base address
//t3: Node 1 LS7A HT1 cntl register base address
//t5, t6: tmp variable
//t8: store return value
//s1: store ra

reset_ht_link:

    move    s1, ra

    dli     t0, 0x90000efdfb000000
    dli     t1, 0x90000efdfe000000

    dli     t2, 0x90001efdfb000000
    dli     t3, 0x90001efdfe000000

    move    t8, $0

    and     a0, a1, 0x1
    beqz    a0, 88f
    nop

#ifdef  LS7A_2WAY_CONNECT
    TTYDBG("Reset Node 1 HT1 bus\r\n")
    lb      a0, 0x3e(t2)
    li      a1, 0x40
    or      a0, a0, a1
    sb      a0, 0x3e(t2)
    lw      a0, 0x3c(t2)
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
    //failed more than given time, set fail mark
    or      t8, t8, (0x1 << HT1_LINKDOWN_FAIL_OFFSET + N1_OFFSET)
2:
    lw      a0, 0x44(t2)
    li      a1, 0x20
    and     a0, a0, a1
    bnez    a0, 1b
    nop

    TTYDBG("\r\n")
    lw      a0, 0x44(t2)
    bal     hexserial
    nop
    TTYDBG("\r\n")
#endif

    TTYDBG("Reset Node 0 HT1 bus\r\n")
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
    //failed more than given time, set fail mark
    or      t8, t8, (0x1 << HT1_LINKDOWN_FAIL_OFFSET)
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

    TTYDBG("Dereset Node 0 HT1 bus\r\n")
    lb      a0, 0x3e(t0)
    li      a1, 0x40
    not     a1, a1
    and     a0, a0, a1
    sb      a0, 0x3e(t0)
    lw      a0, 0x3c(t0)
    bal     hexserial
    nop
    TTYDBG("\r\n")

#ifdef  LS7A_2WAY_CONNECT
    TTYDBG("Dereset Node 1 HT1 bus\r\n")
    lb      a0, 0x3e(t2)
    li      a1, 0x40
    not     a1, a1
    and     a0, a0, a1
    sb      a0, 0x3e(t2)
    lw      a0, 0x3c(t2)
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
    li      t5, 0x1f
    or      t8, t8, (0x1 << HT1_LINKUP_FAIL_OFFSET + N1_OFFSET)
2:
    lw      a0, 0x44(t2)
    li      a1, 0x20
    and     a0, a0, a1
    beqz    a0, 1b
    nop

    TTYDBG("\r\n")
    lw      a0, 0x44(t2)
    bal     hexserial
    nop
    TTYDBG("\r\n")
#endif

    //wait until HT link up
    TTYDBG("Wait HT bus up.")
    li      t5, 0x1f
1:
    TTYDBG(">")
    addi    t5, t5, -1
    bnez    t5, 2f
    nop
    TTYDBG("\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b=")
    li      t5, 0x1f
    or      t8, t8, (0x1 << HT1_LINKUP_FAIL_OFFSET)
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

#ifdef  PRINT_HT1_REG   //Print all HT registers
    TTYDBG("LS3A Node 0 HT1 registers are:\r\n")
    move    t5, t0
    daddu   t6, t5, (LS3A_HT_REG_NUM * 4)
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

#if 1
    TTYDBG("LS7A HT registers are:\r\n")
    move    t5, t1
    daddu   t6, t5, (LS7A_HT_REG_NUM * 4)
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
#ifdef  CHECK_HT_PLL_LOCK
    //check HT PLL lock
    move    t5, $0
    lw      a0, 0x178(t0)
    srl     a0, a0, 3
    and     a0, a0, 0x1
    bnez    a0, 2f
    nop
    PRINTSTR("\r\nError: After reconfigure, 3A HT PLL not locked!!!\r\n")
    add     t5, t5, 1
2:
#ifdef  CHECK_7A_HT_PLL_LOCK
    lw      a0, 0x1F4(t1)
    srl     a0, a0, 3
    and     a0, a0, 0x1
    bnez    a0, 2f
    nop
    PRINTSTR("\r\nError: After reconfigure, 7A HT PLL not locked!!!\r\n")
    add     t5, t5, 1
2:
#endif
    bnez    t5, 2b
    nop
    TTYDBG("\r\nAfter reconnect, PLL check success.\r\n")
#endif

#if 1//Check if CRC error bit set and reset it
    TTYDBG("Checking Node 0 HT1 CRC error.")
    li      t5, MAX_HT_CRC_ERR_COUNT
1:
    TTYDBG(">")
    addi    t5, t5, -1
    bnez    t5, 2f
    nop
    or      t8, t8, (0x1 << HT1_CPU_CRC_FAIL_OFFSET)
    li      t5, MAX_HT_CRC_ERR_COUNT
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
    TTYDBG("\r\nChecking Bridge HT CRC error bit.")
    li      t5, MAX_HT_CRC_ERR_COUNT
1:
    TTYDBG(">")
    addi    t5, t5, -1
    bnez    t5, 2f
    nop
    or      t8, t8, (0x1 << HT1_SB_CRC_FAIL_OFFSET)
    li      t5, MAX_HT_CRC_ERR_COUNT
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
    TTYDBG("\r\n")

#ifdef  LS7A_2WAY_CONNECT
    TTYDBG("Checking Node 1 HT1 CRC error.")
    li      t5, MAX_HT_CRC_ERR_COUNT
1:
    TTYDBG(">")
    addi    t5, t5, -1
    bnez    t5, 2f
    nop
    or      t8, t8, (0x1 << HT1_CPU_CRC_FAIL_OFFSET + N1_OFFSET)
    li      t5, MAX_HT_CRC_ERR_COUNT
2:
    lw      a0, 0x44(t2)
    li      a1, 0x300
    and     a0, a0, a1
    beqz    a0, 3f
    nop
    or      t8, t8, (0x1 << HT1_CPU_CRC_ERR_OFFSET + N1_OFFSET)
    TTYDBG("\r\nCRC error found\r\n")
    lw      a0, 0x44(t2)
    bal     hexserial
    nop

    lw      a0, 0x44(t2)
    li      a1, 0xfffffcff
    and     a0, a0, a1
    sw      a0, 0x44(t2)
    b       1b
    nop
3:
    TTYDBG("\r\nChecking Bridge HT CRC error bit.")
    li      t5, MAX_HT_CRC_ERR_COUNT
1:
    TTYDBG(">")
    addi    t5, t5, -1
    bnez    t5, 2f
    nop
    or      t8, t8, (0x1 << HT1_SB_CRC_FAIL_OFFSET + N1_OFFSET)
    li      t5, MAX_HT_CRC_ERR_COUNT
2:
    lw      a0, 0x44(t3)
    li      a1, 0x300
    and     a0, a0, a1
    beqz    a0, 3f
    nop
    or      t8, t8, (0x1 << HT1_SB_CRC_ERR_OFFSET + N1_OFFSET)
    TTYDBG("\r\nCRC error found\r\n")
    lw      a0, 0x44(t3)
    bal     hexserial
    nop

    lw      a0, 0x44(t3)
    li      a1, 0xfffffcff
    and     a0, a0, a1
    sw      a0, 0x44(t3)
    b       1b
    nop
3:
    TTYDBG("\r\n")
#endif
#endif

88:
#ifdef  DEBUG_HT1
    TTYDBG("\r\nNode 0 HT1 reg: 0x178: ")
    lw      a0, 0x178(t0)
    bal     hexserial
    nop
    TTYDBG("\r\nNode 0 LS7A HT1 reg: 0x1F4: ")
    lw      a0, 0x1F4(t1)
    bal     hexserial
    nop
    TTYDBG("\r\nNode 0 HT1 reg: 0x44: ")
    lw      a0, 0x44(t0)
    bal     hexserial
    nop
    TTYDBG("\r\nNode 0 HT1 reg: 0x54: ")
    lw      a0, 0x54(t0)
    bal     hexserial
    nop
    TTYDBG("\r\nNode 0 HT1 reg: 0x120: ")
    lw      a0, 0x120(t0)
    bal     hexserial
    nop
    TTYDBG("\r\nNode 0 HT1 reg: 0x128: ")
    lw      a0, 0x128(t0)
    bal     hexserial
    nop
    TTYDBG("\r\nNode 0 HT1 reg: 0x188: ")
    lw      a0, 0x188(t0)
    bal     hexserial
    nop
    TTYDBG("\r\nNode 0 HT1 reg: 0x18c: ")
    lw      a0, 0x18c(t0)
    bal     hexserial
    nop
    TTYDBG("\r\nNode 0 HT1 reg: 0x190: ")
    lw      a0, 0x190(t0)
    bal     hexserial
    nop
    TTYDBG("\r\n")
#endif

    move    v0, t8
    move    ra, s1
    jr      ra
    nop
    .end    reset_ht_link
