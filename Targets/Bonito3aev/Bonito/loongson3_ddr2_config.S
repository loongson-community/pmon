/*whd : loongson3_ddr2_config.S
        used to set up all ddr controllers
        and set up the memory space on L2 Xbar
*/

#######################################################
/* Undefine the two to enable both */
#define MC0_ONLY
//#define MC1_ONLY

/* Size of each DDR controller */
//#define DDR_512
#define DDR_1G
//#define DDR_2G

/* Interleave pattern when both controller enabled */
//#define INTERLEAVE_27
//#define INTERLEAVE_13
//#define INTERLEAVE_12
#define INTERLEAVE_11
#######################################################

/******************************************************
 s1: [1:0]: NODE ID
******************************************************/
#define GET_NODE_ID_a0 dli a0, 0x3; and a0, s1, a0; dsll a0, 44;

        
        GET_NODE_ID_a0;
        bnez    a0, 1f
        nop
#ifdef DDR_2G
	    li	msize, 0x7f000000
#else
#ifdef DDR_1G
#ifdef MC0_ONLY
	    li	msize, 0x3f000000
#else
#ifdef MC1_ONLY
	    li	msize, 0x3f000000
#else
	    li	msize, 0x7f000000
#endif
#endif
#else
#ifdef DDR_512
	    li	msize, 0x1f000000
#else
	    li	msize, 0x0f000000
#endif
#endif
#endif

1:

        dli     t2, 0x900000001fe00180
        dli     t0, 0x900000003ff00000
        or      t2, t2, a0
        or      t0, t0, a0


###Enable the reg space###
#if 1
	TTYDBG	("Enable register space of MEMORY\r\n")
        #li  t2, 0xbfe00180
        #dli t2, 0x900000001fe00180
        lw  a1, 0x0(t2)
        li  a0, 0xfffffeff
        and a1, a1,a0
        sw  a1, 0x0(t2)
#endif    


#ifndef MC1_ONLY
        GET_NODE_ID_a0;

        dli     t1, 0x00000000
        or      t1, t1, a0
        sd      t1, 0x10(t0)

        dli     t1, 0xfffffffff0000000
        sd      t1, 0x50(t0)

        dli     t1, 0x000000f0
        sd      t1, 0x90(t0)
        sync

        dli     a1, 0x900000000ff00000
        or      a0, a0, a1
	    bal	    ddr2_config
	    nop

	    PRINTSTR("MC0 Config DONE\r\n")
#endif

#ifndef MC0_ONLY
        GET_NODE_ID_a0;

        dli     t1, 0x00000000
        or      t1, t1, a0
        sd      t1, 0x10(t0)

        dli     t1, 0xfffffffff0000000
        sd      t1, 0x50(t0)

        dli     t1, 0x000000f1
        sd      t1, 0x90(t0)
        sync

        dli     a1, 0x900000000ff00000
        or      a0, a0, a1
	    bal	    ddr2_config_mc1
	    nop

	    PRINTSTR("MC1 Config DONE\r\n")
#endif

//cww
#if 1
#ifdef MC0_ONLY
	    PRINTSTR("MC0 space open : 0x00000000 - 0x0FFFFFFF\r\n")
        #dli     t0, 0x900000003ff00000  #base
        GET_NODE_ID_a0;
        dli     t1, 0x00000000
        or      t1, t1, a0
        sd      t1, 0x10(t0)

        dli     t1, 0xfffffffff0000000
        sd      t1, 0x50(t0)

        dli     t1, 0x000000f0
        sd      t1, 0x90(t0)
	    PRINTSTR("MC0 space open : 0x00000000 - 0x0FFFFFFF : done\r\n")

#else
#ifdef MC1_ONLY
	    PRINTSTR("MC1 space open : 0x00000000 - 0x0FFFFFFF\r\n")
        #dli     t0, 0x900000003ff00000  #base
        GET_NODE_ID_a0;
        dli     t1, 0x00000000
        or      t1, t1, a0
        sd      t1, 0x10(t0)

        dli     t1, 0xfffffffff0000000
        sd      t1, 0x50(t0)

        dli     t1, 0x000000f1
        sd      t1, 0x90(t0)

#else
#ifdef INTERLEAVE_27
	    PRINTSTR("DDR Interleave space open : 0x00000000 - 0x0FFFFFFF\r\n")
	    PRINTSTR("DDR Interleave using Bit 27\r\n")
        GET_NODE_ID_a0;
        dli     t1, 0x00000000
        or      t1, t1, a0
        sd      t1, 0x10(t0)

        dli     t1, 0xfffffffff8000000
        sd      t1, 0x50(t0)

        dli     t1, 0x000000f0
        sd      t1, 0x90(t0)

        dli     t1, 0x08000000
        or      t1, t1, a0
        sd      t1, 0x18(t0)

        dli     t1, 0xfffffffff8000000
        sd      t1, 0x58(t0)

        dli     t1, 0x000000f1
        sd      t1, 0x98(t0)
#else
#ifdef INTERLEAVE_13
	    PRINTSTR("DDR Interleave space open : 0x00000000 - 0x0FFFFFFF\r\n")
	    PRINTSTR("DDR Interleave using Bit 13\r\n")
        GET_NODE_ID_a0;
        dli     t1, 0x0000000000000000
        or      t1, t1, a0
        sd      t1, 0x10(t0)

        dli     t1, 0xfffffffff0002000
        sd      t1, 0x50(t0)

        dli     t1, 0x00000000000000f0
        sd      t1, 0x90(t0)

        dli     t1, 0x0000000000002000
        or      t1, t1, a0
        sd      t1, 0x18(t0)

        dli     t1, 0xfffffffff0002000
        sd      t1, 0x58(t0)

        dli     t1, 0x00000000000000f1
        sd      t1, 0x98(t0)
#else
#ifdef INTERLEAVE_12
	    PRINTSTR("DDR Interleave space open : 0x00000000 - 0x0FFFFFFF\r\n")
	    PRINTSTR("DDR Interleave using Bit 12\r\n")
        GET_NODE_ID_a0;
        dli     t1, 0x0000000000000000
        or      t1, t1, a0
        sd      t1, 0x10(t0)

        dli     t1, 0xfffffffff0001000
        sd      t1, 0x50(t0)

        dli     t1, 0x00000000000000f0
        sd      t1, 0x90(t0)

        dli     t1, 0x0000000000001000
        or      t1, t1, a0
        sd      t1, 0x18(t0)

        dli     t1, 0xfffffffff0001000
        sd      t1, 0x58(t0)

        dli     t1, 0x00000000000000f1
        sd      t1, 0x98(t0)
#else
#ifdef INTERLEAVE_11
	    PRINTSTR("DDR Interleave space open : 0x00000000 - 0x0FFFFFFF\r\n")
	    PRINTSTR("DDR Interleave using Bit 11\r\n")
        GET_NODE_ID_a0;
        dli     t1, 0x0000000000000000
        or      t1, t1, a0
        sd      t1, 0x10(t0)

        dli     t1, 0xfffffffff0000800
        sd      t1, 0x50(t0)

        dli     t1, 0x00000000000000f0
        sd      t1, 0x90(t0)

        dli     t1, 0x0000000000000800
        or      t1, t1, a0
        sd      t1, 0x18(t0)

        dli     t1, 0xfffffffff0000800
        sd      t1, 0x58(t0)

        dli     t1, 0x00000000000000f1
        sd      t1, 0x98(t0)
#else
	    PRINTSTR("DDR Interleave space open : 0x00000000 - 0x0FFFFFFF\r\n")
        GET_NODE_ID_a0;
        dli     t1, 0x0000000000000000
        or      t1, t1, a0
        sd      t1, 0x10(t0)

        dli     t1, 0xfffffffff0000400
        sd      t1, 0x50(t0)

        dli     t1, 0x00000000000000f0
        sd      t1, 0x90(t0)

        dli     t1, 0x0000000000000400
        or      t1, t1, a0
        sd      t1, 0x18(t0)

        dli     t1, 0xfffffffff0000400
        sd      t1, 0x58(t0)

        dli     t1, 0x00000000000000f1
        sd      t1, 0x98(t0)
#endif
#endif
#endif
#endif
#endif
#endif


#ifdef MC0_ONLY
#ifdef DDR_512
	    PRINTSTR("MC0 pace open : 0x20000000 - 0x2FFFFFFF\r\n")
        GET_NODE_ID_a0;
        dli     t1, 0x0000000020000000
        or      t1, t1, a0
        sd      t1, 0x20(t0)

        dli     t1, 0xfffffffff0000000
        sd      t1, 0x60(t0)

        dli     t1, 0x00000000100000f0
        sd      t1, 0xa0(t0)

        dli     t1, 0x0000000090000000  #zxj added
        or      t1, t1, a0
        sd      t1, 0x28(t0)

        dli     t1, 0xfffffffff0000000
        sd      t1, 0x68(t0)

        dli     t1, 0x00000000100000f0
        sd      t1, 0xa8(t0)
#else
#ifdef DDR_1G
	    PRINTSTR("MC0 space open : 0x40000000 - 0x7FFFFFFF\r\n")
        #dli     t0, 0x900000003ff00000  #base
        GET_NODE_ID_a0;
        dli     t1, 0x0000000040000000
        or      t1, t1, a0
        sd      t1, 0x18(t0)

        dli     t1, 0xffffffffc0000000
        sd      t1, 0x58(t0)

        dli     t1, 0x00000000000000f0
        sd      t1, 0x98(t0)

        //dli     t1, 0x0000000090000000  #zxj added
        dli     t1, 0x0000000070000000  #zxj added
        or      t1, t1, a0
        sd      t1, 0x20(t0)

        dli     t1, 0xfffffffff0000000
        sd      t1, 0x60(t0)

        dli     t1, 0x00000000100000f0
        sd      t1, 0xa0(t0)
	PRINTSTR("MC0 space open : 0x40000000 - 0x7FFFFFFF: done\r\n")

#else
#ifdef DDR_2G
	    PRINTSTR("    MC0 space open : 0x80000000 - 0xbFFFFFFF\r\n")
	    PRINTSTR("and MC1 space open : 0xc0000000 - 0xFFFFFFFF\r\n")
        #dli     t0, 0x900000003ff00000  #base
        GET_NODE_ID_a0;
        dli     t1, 0x0000000080000000
        or      t1, t1, a0
        sd      t1, 0x18(t0)

        dli     t1, 0xffffffffc0000000
        sd      t1, 0x58(t0)

        dli     t1, 0x00000000000000f0
        sd      t1, 0x98(t0)
	PRINTSTR("    MC0 space open : 0x80000000 - 0xbFFFFFFF : done\r\n")

        dli     t1, 0x00000000c0000000
        or      t1, t1, a0
        sd      t1, 0x20(t0)

        dli     t1, 0xffffffffc0000000
        sd      t1, 0x60(t0)

        dli     t1, 0x00000000000000f1
        sd      t1, 0xa0(t0)
	PRINTSTR("and MC1 space open : 0xc0000000 - 0xFFFFFFFF : done\r\n")

#endif
#endif
#endif

#else
#ifdef MC1_ONLY
#ifdef DDR_512
	    PRINTSTR("MC1 space open : 0x20000000 - 0x2FFFFFFF\r\n")
        GET_NODE_ID_a0;
        dli     t1, 0x0000000020000000
        or      t1, t1, a0
        sd      t1, 0x20(t0)

        dli     t1, 0xfffffffff0000000
        sd      t1, 0x60(t0)

        dli     t1, 0x00000000100000f1
        sd      t1, 0xa0(t0)

        dli     t1, 0x0000000090000000  #zxj
        or      t1, t1, a0
        sd      t1, 0x28(t0)

        dli     t1, 0xfffffffff0000000
        sd      t1, 0x68(t0)

        dli     t1, 0x00000000100000f1
        sd      t1, 0xa8(t0)
#else
#ifdef DDR_1G
	    PRINTSTR("MC1 space open : 0x40000000 - 0x7FFFFFFF\r\n")
        GET_NODE_ID_a0;
        dli     t1, 0x0000000040000000
        or      t1, t1, a0
        sd      t1, 0x20(t0)

        dli     t1, 0xffffffffc0000000
        sd      t1, 0x60(t0)

        dli     t1, 0x00000000000000f1
        sd      t1, 0xa0(t0)

        dli     t1, 0x0000000090000000  #zxj
        or      t1, t1, a0
        sd      t1, 0x28(t0)

        dli     t1, 0xfffffffff0000000
        sd      t1, 0x68(t0)

        dli     t1, 0x00000000100000f1
        sd      t1, 0xa8(t0)
#else
#ifdef DDR_2G
	    PRINTSTR("MC1 space open : 0x80000000 - 0xFFFFFFFF\r\n")
        GET_NODE_ID_a0;
        dli     t1, 0x0000000080000000
        or      t1, t1, a0
        sd      t1, 0x20(t0)

        dli     t1, 0xffffffff80000000
        sd      t1, 0x60(t0)

        dli     t1, 0x00000000000000f1
        sd      t1, 0xa0(t0)

#endif
#endif
	    PRINTSTR("PCI DMA SPACE reassigned to MC1\r\n")
        dli     t1, 0x00000000000000f1
        sd      t1, 0x180(t0)

#endif

#else
#ifdef INTERLEAVE_27
	    PRINTSTR("DDR Interleave space open : 0x80000000 - 0xFFFFFFFF\r\n")
	    PRINTSTR("DDR Interleave using Bit 27\r\n")

        GET_NODE_ID_a0;
        dli     t1, 0x0000000080000000
        or      t1, t1, a0
        sd      t1, 0x20(t0)
        dli     t1, 0xFFFFFFFFC8000000
        sd      t1, 0x60(t0)
        dli     t1, 0x00000000000000F0
        sd      t1, 0xa0(t0)

        dli     t1, 0x0000000088000000
        or      t1, t1, a0
        sd      t1, 0x28(t0)
        dli     t1, 0xFFFFFFFFC8000000
        sd      t1, 0x68(t0)
        dli     t1, 0x00000000000000F1
        sd      t1, 0xa8(t0)

        dli     t1, 0x00000000C0000000
        or      t1, t1, a0
        sd      t1, 0x30(t0)
        dli     t1, 0xFFFFFFFFC8000000
        sd      t1, 0x70(t0)
        dli     t1, 0x00000000080000F0
        sd      t1, 0xb0(t0)

        dli     t1, 0x00000000C8000000
        or      t1, t1, a0
        sd      t1, 0x38(t0)
        dli     t1, 0xFFFFFFFFC8000000
        sd      t1, 0x78(t0)
        dli     t1, 0x00000000080000F1
        sd      t1, 0xb8(t0)

#else
#ifdef INTERLEAVE_13
	    PRINTSTR("DDR Interleave space open : 0x80000000 - 0xFFFFFFFF\r\n")
	    PRINTSTR("DDR Interleave using Bit 13\r\n")

        GET_NODE_ID_a0;
        dli     t1, 0x0000000080000000
        or      t1, t1, a0
        sd      t1, 0x20(t0)
        dli     t1, 0xFFFFFFFFC0002000
        sd      t1, 0x60(t0)
        dli     t1, 0x00000000000000F0
        sd      t1, 0xa0(t0)

        dli     t1, 0x0000000080002000
        or      t1, t1, a0
        sd      t1, 0x28(t0)
        dli     t1, 0xFFFFFFFFC0002000
        sd      t1, 0x68(t0)
        dli     t1, 0x00000000000000F1
        sd      t1, 0xa8(t0)

        dli     t1, 0x00000000C0000000
        or      t1, t1, a0
        sd      t1, 0x30(t0)
        dli     t1, 0xFFFFFFFFC0002000
        sd      t1, 0x70(t0)
        dli     t1, 0x00000000000020F0
        sd      t1, 0xb0(t0)

        dli     t1, 0x00000000C0002000
        or      t1, t1, a0
        sd      t1, 0x38(t0)
        dli     t1, 0xFFFFFFFFC0002000
        sd      t1, 0x78(t0)
        dli     t1, 0x00000000000020F1
        sd      t1, 0xb8(t0)

#else
#ifdef INTERLEAVE_12
	    PRINTSTR("DDR Interleave space open : 0x80000000 - 0xFFFFFFFF\r\n")
	    PRINTSTR("DDR Interleave using Bit 12\r\n")

        GET_NODE_ID_a0;
        dli     t1, 0x0000000080000000
        or      t1, t1, a0
        sd      t1, 0x20(t0)
        dli     t1, 0xFFFFFFFFC0001000
        sd      t1, 0x60(t0)
        dli     t1, 0x00000000000000F0
        sd      t1, 0xa0(t0)

        dli     t1, 0x0000000080001000
        or      t1, t1, a0
        sd      t1, 0x28(t0)
        dli     t1, 0xFFFFFFFFC0001000
        sd      t1, 0x68(t0)
        dli     t1, 0x00000000000000F1
        sd      t1, 0xa8(t0)

        dli     t1, 0x00000000C0000000
        or      t1, t1, a0
        sd      t1, 0x30(t0)
        dli     t1, 0xFFFFFFFFC0001000
        sd      t1, 0x70(t0)
        dli     t1, 0x00000000000010F0
        sd      t1, 0xb0(t0)

        dli     t1, 0x00000000C0001000
        or      t1, t1, a0
        sd      t1, 0x38(t0)
        dli     t1, 0xFFFFFFFFC0001000
        sd      t1, 0x78(t0)
        dli     t1, 0x00000000000010F1
        sd      t1, 0xb8(t0)

#else
#ifdef INTERLEAVE_11
	    PRINTSTR("DDR Interleave space open : 0x80000000 - 0xFFFFFFFF\r\n")
	    PRINTSTR("DDR Interleave using Bit 11\r\n")

        GET_NODE_ID_a0;
        dli     t1, 0x0000000080000000
        or      t1, t1, a0
        sd      t1, 0x20(t0)
        dli     t1, 0xFFFFFFFFC0000800
        sd      t1, 0x60(t0)
        dli     t1, 0x00000000000000F0
        sd      t1, 0xa0(t0)

        dli     t1, 0x0000000080000800
        or      t1, t1, a0
        sd      t1, 0x28(t0)
        dli     t1, 0xFFFFFFFFC0000800
        sd      t1, 0x68(t0)
        dli     t1, 0x00000000000000F1
        sd      t1, 0xa8(t0)

        dli     t1, 0x00000000C0000000
        or      t1, t1, a0
        sd      t1, 0x30(t0)
        dli     t1, 0xFFFFFFFFC0000800
        sd      t1, 0x70(t0)
        dli     t1, 0x00000000000008F0
        sd      t1, 0xb0(t0)

        dli     t1, 0x00000000C0000800
        or      t1, t1, a0
        sd      t1, 0x38(t0)
        dli     t1, 0xFFFFFFFFC0000800
        sd      t1, 0x78(t0)
        dli     t1, 0x00000000000008F1
        sd      t1, 0xb8(t0)

#else
	    PRINTSTR("DDR Interleave space open : 0x80000000 - 0xFFFFFFFF\r\n")

        GET_NODE_ID_a0;
        dli     t1, 0x0000000080000000
        or      t1, t1, a0
        sd      t1, 0x20(t0)
        dli     t1, 0xFFFFFFFFC0000400
        sd      t1, 0x60(t0)
        dli     t1, 0x00000000000000F0
        sd      t1, 0xa0(t0)

        dli     t1, 0x0000000080000400
        or      t1, t1, a0
        sd      t1, 0x28(t0)
        dli     t1, 0xFFFFFFFFC0000400
        sd      t1, 0x68(t0)
        dli     t1, 0x00000000000000F1
        sd      t1, 0xa8(t0)

        dli     t1, 0x00000000C0000000
        or      t1, t1, a0
        sd      t1, 0x30(t0)
        dli     t1, 0xFFFFFFFFC0000400
        sd      t1, 0x70(t0)
        dli     t1, 0x00000000000004F0
        sd      t1, 0xb0(t0)

        dli     t1, 0x00000000C0000400
        or      t1, t1, a0
        sd      t1, 0x38(t0)
        dli     t1, 0xFFFFFFFFC0000400
        sd      t1, 0x78(t0)
        dli     t1, 0x00000000000004F1
        sd      t1, 0xb8(t0)

#endif
#endif
#endif
#endif
#endif
#endif
#endif

        sync

#if 0 //print registers
    li      t1, 152 ##0x72
    li      t2, 0xaff00000

reg_read:
    ld      a0, 0x0(t2)

    bal	    hexserial
    nop
    PRINTSTR("\r\n")

    addiu   t2, t2, 16
    bnez    t1, reg_read
    addiu   t1, t1, -1

#endif


#if 0 /* read ddr2 registers */
/* No use in Loongson 3A */
        li	t0, 0xaff00000
        
not_locked:
        ld	t1, 0x10(t0)
        andi    t1, 0x01
        beqz    t1, not_locked
        nop

        PRINTSTR("DDR2 DLL locked\r\n")
        
        ld	    t1, 0xf0(t0)
        move    a0, t1
        bal     hexserial
        nop

#endif
        
###disable the reg space###
#if 1
	    TTYDBG("Disable register space of MEMORY\r\n")
        #li  t2,0xbfe00180
        #dli t2,0x900000001fe00180
        lw  a1,0x0(t2)
        or  a1,a1,0x100
        sw  a1,0x0(t2)
#endif

#if 0 // AdonWang disable ddr3 readbuff
/*      May Affect the Performance     */
       TTYDBG("Disable read buffer\r\n")
       #dli     t2, 0x900000001fe00180
       lw      t1, 0x4(t2)
       li      a0, 0x18
       or      t1, t1, a0
       sw      t1, 0x4(t2)
#endif

#if 1 // AdonWang disable cpu buffered read
/* !!!!!!!!!!!!! IMPORTANT !!!!!!!!!!!! */
       TTYDBG("Disable cpu buffered read\r\n")
       #dli     t2, 0x900000001fe00180 
       lw      t1, 0x0(t2)
       li      a0, 0xfffffdff
       and     t1, t1, a0
       sw      t1, 0x0(t2)
#endif

