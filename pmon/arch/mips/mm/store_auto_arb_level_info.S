//store whether save ddr param info to memory
//at least 512MB memory and DIMM_INFO_ADDR has been routed to memory
#define DIMM_INFO_ADDR  0x980000000fff0000
#define DIMM_INFO_TOKEN 0x2013011014413291
/***************
store info organize in memory:
0x0: token
0x8: s3
DIMM identifier:
0x10~0x30: Chip 0 MC0, slot 0, slot 1
0x30~0x50: Chip 0 MC1, slot 0, slot 1
0x50~0x70: Chip 1 MC0, slot 0, slot 1
0x70~0x90: Chip 1 MC1, slot 0, slot 1
MC clksel:
    0: soft freq config/hardware freq config
63: 1: configure setting(soft clksel, hard clksel)
0x90:   Chip 0 MC0
0x98:   Chip 1 MC1
0xa0:   Chip 1 MC0
0xa8:   Chip 1 MC1

s3:
|[52:48]|                    | 4'bx    | chip 1 ddr clksel |--obselete
|[44:40]|                    | 4'bx    | chip 0 ddr clksel |--obselete
|[39:36]|                    | 4'bx    | Reserved          |
|[35:32]|                    | 4'bx    | level status      |
|35:    |                                       Chip 1 MC1 |
|34:    |                                       Chip 1 MC0 |
|33:    |                                       Chip 0 MC1 |
|32:    |                                       Chip 0 MC0 |
Chip 1(3A node 1,3B node 2/3):
|[31:28]|                    | 4'bx    | MC1_SLOT1 I2C ADDR|
|[27:24]|                    | 4'bx    | MC1_SLOT0 I2C ADDR|
|[23:20]|                    | 4'bx    | MC0_SLOT1 I2C ADDR|
|[19:16]|                    | 4'bx    | MC0_SLOT0 I2C ADDR|
Chip 0(3A node 0,3B node 0/1):
|[15:12]|                    | 4'bx    | MC1_SLOT1 I2C ADDR|
|[11: 8]|                    | 4'bx    | MC1_SLOT0 I2C ADDR|
|[ 7: 4]|                    | 4'bx    | MC0_SLOT1 I2C ADDR|
|[ 3: 0]|                    | 4'bx    | MC0_SLOT0 I2C ADDR|
***************/
    //set token to detect whether this area info is demaged
    dli     t0, DIMM_INFO_ADDR
    dli     a0, DIMM_INFO_TOKEN
    sd      a0, 0x0(t0)
    //store s3
    sd      s3, 0x8(t0)

#ifdef  CHECK_ARB_LEVEL_DIMM
    PRINTSTR("\r\nstore DIMM info of NODE0\r\n")
    //deal N0 MC0
    dsrl    a0, s3, 32
    and     a0, a0, 0x1
    beqz    a0, 1f
    nop
    //store N0-MC0 info
    dsrl    a0, s3, 0
    and     a0, a0, 0xf
    move    a2, $0
    bal     READ_DIMM_IDENTIFIER
    nop
    sd      t4, 0x10(t0)
    sd      t5, 0x18(t0)

    dsrl    a0, s3, 4
    and     a0, a0, 0xf
    move    a2, $0
    bal     READ_DIMM_IDENTIFIER
    nop
    sd      t4, 0x20(t0)
    sd      t5, 0x28(t0)
1:
    //deal N0 MC1
    dsrl    a0, s3, 33
    and     a0, a0, 0x1
    beqz    a0, 1f
    nop
    //store N0-MC1 info
    dsrl    a0, s3, 8
    and     a0, a0, 0xf
    move    a2, $0
    bal     READ_DIMM_IDENTIFIER
    nop
    sd      t4, 0x30(t0)
    sd      t5, 0x38(t0)

    dsrl    a0, s3, 12
    and     a0, a0, 0xf
    move    a2, $0
    bal     READ_DIMM_IDENTIFIER
    nop
    sd      t4, 0x40(t0)
    sd      t5, 0x48(t0)
1:
#if ((loongson3A3 && MULTI_CHIP) || (LS3B && DUAL_3B))
    PRINTSTR("\r\nstore DIMM info of NODE1\r\n")
    //deal N1 MC0
    dsrl    a0, s3, 34
    and     a0, a0, 0x1
    beqz    a0, 1f
    nop
    //store N0-MC0 info
    dsrl    a0, s3, 16
    and     a0, a0, 0xf
    move    a2, $0
    bal     READ_DIMM_IDENTIFIER
    nop
    sd      t4, 0x50(t0)
    sd      t5, 0x58(t0)

    dsrl    a0, s3, 20
    and     a0, a0, 0xf
    move    a2, $0
    bal     READ_DIMM_IDENTIFIER
    nop
    sd      t4, 0x60(t0)
    sd      t5, 0x68(t0)
1:
    //deal N1 MC1
    dsrl    a0, s3, 35
    and     a0, a0, 0x1
    beqz    a0, 1f
    nop
    //store N0-MC0 info
    dsrl    a0, s3, 24
    and     a0, a0, 0xf
    move    a2, $0
    bal     READ_DIMM_IDENTIFIER
    nop
    sd      t4, 0x70(t0)
    sd      t5, 0x78(t0)

    dsrl    a0, s3, 28
    and     a0, a0, 0xf
    move    a2, $0
    bal     READ_DIMM_IDENTIFIER
    nop
    sd      t4, 0x80(t0)
    sd      t5, 0x88(t0)
1:
#endif
#endif

#ifdef  CHECK_ARB_LEVEL_FREQ
    dsrl    a0, s3, 32
    and     a0, a0, 0x1
    beqz    a0, 1f
    nop
    move    a1, $0
    bal     GET_DDR_FREQ_CONFIG
    nop
    sd      v0, 0x90(t0)
1:
    dsrl    a0, s3, 33
    and     a0, a0, 0x1
    beqz    a0, 1f
    nop
    move    a1, $0
    bal     GET_DDR_FREQ_CONFIG
    nop
    sd      v0, 0x98(t0)
1:
#if ((loongson3A3 && MULTI_CHIP) || (LS3B && DUAL_3B))
    //deal N1 MC0
    dsrl    a0, s3, 34
    and     a0, a0, 0x1
    beqz    a0, 1f
    nop
    add     a1, $0, 1
    bal     GET_DDR_FREQ_CONFIG
    nop
    sd      v0, 0xa0(t0)
1:
    dsrl    a0, s3, 35
    and     a0, a0, 0x1
    beqz    a0, 1f
    nop
    add     a1, $0, 1
    bal     GET_DDR_FREQ_CONFIG
    nop
    sd      v0, 0xa8(t0)
1:
#endif
#endif

#ifdef  DEBUG_AUTO_ARB_LEVEL
    //print stored memory
    dli     t0, DIMM_INFO_ADDR
    daddu   t3, t0, 0xb0
    PRINTSTR("\r\nthe stored info is:\r\n")
1:
    dsrl    a0, t0, 32
    bal     hexserial
    nop
    move    a0, t0
    bal     hexserial
    nop
    PRINTSTR(":  0x")
    ld      t1, 0x0(t0)
    dsrl    a0, t1, 32
    bal     hexserial
    nop
    move    a0, t1
    bal     hexserial
    nop
    PRINTSTR("\r\n")
    
    daddu   t0, t0, 0x8
    blt     t0, t3, 1b
    nop
#endif
