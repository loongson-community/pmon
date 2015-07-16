
#if 0   //def  AUTO_DDR_CONFIG     //hot fix for board misconnection--temp code
    GET_ROW_SIZE
    dli     a0, 0x2
    bne     a1, a0, 1f
    nop
    //a1 == 2, row num is 14, decrease one(!!!a1+1), half the density
    daddu   a1, a1, 0x1
    dli     a0, 0x7
    dsll    a0, a0, ROW_SIZE_OFFSET
    not     a0, a0
    and     s1, s1, a0
    dsll    a1, a1, ROW_SIZE_OFFSET
    or      s1, s1, a1

    //half the density
    GET_MC0_MEMSIZE
    dsrl    a1, a1, 0x1
    dli     a0, MC_MEMSIZE_MASK
    dsll    a0, a0, MC0_MEMSIZE_OFFSET
    not     a0, a0
    and     s1, s1, a0
    dsll    a1, a1, MC0_MEMSIZE_OFFSET
    or      s1, s1, a1
1:
#if 1
    /* show value of s1 */
    PRINTSTR("\r\nnew s1 = 0x");
    dsrl    a0, s1, 32
    bal     hexserial
    nop
    PRINTSTR("__")
    move    a0, s1
    bal     hexserial
    nop
    PRINTSTR("\r\n")
#endif
#endif

