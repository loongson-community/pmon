        .global get_dimmsize_a1
        .ent    get_dimmsize_a1
get_dimmsize_a1:
    move    s5, ra

    dli     t5, 0x0
    dli     t6, 0x0
    move    t6, a0
    dsll    t6, t6, 32
    GET_MC0_SLOT0_ID
    beq     a0, t5, 11f
    nop
    GET_MC1_SLOT0_ID
11:
    dli     a0, 0x8
    bgeu    a1, a0, 1f  //invalidate device id
    nop
    dsll    a1, a1, 1
    ori     a0, a1, 0xa1
    bal     ddr4_get_capasity
    nop
    daddu   t6, a1, t6
1:
    move    a0, t6
    dsrl    a0, a0, 32
    dli     t5, 0x0
    GET_MC0_SLOT0_ID
    beq     a0, t5, 12f
    nop
    GET_MC1_SLOT1_ID
12:
    dli     a0, 0x8
    bgeu    a1, a0, 2f  //invalidate device id
    nop
    dsll    a1, a1, 1
    ori     a0, a1, 0xa1
    bal     ddr4_get_capasity
    nop
    dli     a0, 0xffffffff
    and     t6, t6, a0
    daddu   a1, a1, t6
2:
    jr      s5
    .end    get_dimmsize_a1


LEAF(ddr4_get_capasity)
    move    
//probe DIMM Density
//DIMM Density = SDRAM Density / 8 * DIMM Width / SDRAM Width * Ranks
    dli     a1, 4
//    GET_I2C_NODE_ID_a2
    bal     i2cread
    nop
    //only bit[3:0] used
    andi    v0, v0, 0xf //bit 3 is reserved by JEDEC now
    dli     a0, 0x1000
    bne     a0, v0, 24f
    nop
    dli     t5, 0x30
    b       2f
    nop
24:
    dli     a0, 0x1001
    bne     a0, v0, 1f
    nop
    dli     t5, 0x60
    b       2f
    nop
1:
    dli     t5, 0x1
    dsll    t5, t5, v0
2:
    dsrl    t5, t5, 0x1
    //here t5 represent SDRAM Density in 512Mb
    dsll    t5, t5, 3   //*8 (64 bit width)

    dli     a1, 12   //GET_DIMM_WIDTH
//    GET_I2C_NODE_ID_a2
    bal     i2cread
    nop
    andi    v0, v0, 0x2
    dsrl    v0, v0, 0x1
    dsrl    t5, t5, 3
    dsrl    t5, t5, v0
    //here t5 = SDRAM Density / 8 * DIMM Width
    //probe SDRAM Width
    dli     a1, 13
    //GET_I2C_NODE_ID_a2
    bal     i2cread
    nop
    //only bit[2:0] used
    andi    v0, v0, 0x1 //bit 2 is reserved now
    dsll    t5, t5, 5  //t5 /= SDRAM Width
    dsll    t5, t5, v0  //
    //here t5 = SDRAM Density / 8 * DIMM Width / SDRAM Width
    //here, the 1 rank size is store in t5 in normal order, measured by 256M.
    //double the MEMSIZE if there are 2 ranks
    dli     a1, 12   //GET_DIMM_WIDTH
//    GET_I2C_NODE_ID_a2
    bal     i2cread
    nop
    andi    v0, v0, 0x38
    dsrl    v0, v0, 0x3
    daddu,  v0, v0, 0x1
    dmultu  t5, t5, v0

    dli     a1, 6   //
//    GET_I2C_NODE_ID_a2
    bal     i2cread
    nop
    andi    v0, v0, 0x3 
    dli     a1, 0x2
    bne     v0, a1, not3ds
    nop
    dli     a1, 6   //
//    GET_I2C_NODE_ID_a2
    bal     i2cread
    nop
    andi    v0, v0, 0x70
    dsrl    v0, v0, 0x4
    daddu,  v0, v0, 0x1
    dmultu  t5, t5, v0
not3ds:
    dli     a1, 0xffff
    and     t5, t5, a1
    move    a1, t5

    jr      ra
END(ddr4_get_capasity)
