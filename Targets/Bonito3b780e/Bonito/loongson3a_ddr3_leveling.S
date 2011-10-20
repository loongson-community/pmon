#######################################
	.global ddr3_leveling
	.ent    ddr3_leveling
	.set    noreorder
	.set    mips3
	ddr3_leveling:

#define INT_STATUS  0x960
#define LVL_STATUS  0x940
#define EN_WR_LVL   0x710
#define SW_LVL_MODE 0x750
#define SWLVL_START 0x960
#define SWLVL_LOAD  0x960
//#define WRLVL_DELAY 0x830
#define WRLVL_DELAY 0xaf0 //3A3
#define SWLVL_RESP  0x7a0
#define SWLVL_EXIT  0x960
#define WRLVL_REQ   0x740
#define SWLVL_DONE  0x720

    move    t8, ra

    daddi   v0, a0, WRLVL_REQ
    dli     a1, 0x0000010000000000
    sd      a1, 0x0(v0)
    nop

    daddi   t1, a0, INT_STATUS
1:
    ld      a1, 0x0(t1)
    dli     a2, 0x2000
    //move    a0, a1
    //bal     hexserial64
    //nop
    and     a1, a1, a2
    beqz    a1, 1b
    nop

    daddi   v0, a0, LVL_STATUS
    ld      a1, 0x0(v0)
    dli     a2, 0x100000000
    and     a1, a1, a2
    beqz    a1, 2f
    nop

    daddi   v0, a0, EN_WR_LVL
    dli     a2, 0x000000000
    sd      a2, 0x0(v0)
    nop

    daddi   v0, a0, SW_LVL_MODE
    dli     a2, 0x0001000101020101
    sd      a2, 0x0(v0)
    nop

    daddi   v0, a0, SWLVL_LOAD
    ld      a2, 0x0(v0)
    dli     a1, 0xffffff00ffffffff
    and     a2, a2, a1
    sd      a2, 0x0(v0)
    nop

    daddi   v0, a0, SWLVL_START
    ld      a2, 0x0(v0)
    dli     a1, 0x0000010000000000
    or      a2, a2, a1
    sd      a2, 0x0(v0)
    nop

slice_0:
    //daddi   v0, a0, INT_STATUS
    daddi   v0, a0, SWLVL_DONE
1:
    ld      a1, 0x0(v0)
    dli     a2, 0x100
    and     a1, a1, a2
    beqz    a1, 1b
    nop

    daddi   v0, a0, WRLVL_DELAY
    ld      a2, 0x0(v0)
    dli     a1, 0x00000000ffffffff
    and     a2, a2, a1
    sd      a2, 0x0(v0)
    nop
3:

    daddi   v0, a0, WRLVL_DELAY
    ld      a2, 0x0(v0)
    dli     a1, 0x0000000100000000
    dadd    a2, a2, a1
    sd      a2, 0x0(v0)
    nop

    daddi   v0, a0, SWLVL_LOAD
    ld      a2, 0x0(v0)
    dli     a1, 0x0000000100000000
    or      a2, a2, a1
    sd      a2, 0x0(v0)
    nop

    //daddi   v0, a0, INT_STATUS
    daddi   v0, a0, SWLVL_DONE
1:
    ld      a1, 0x0(v0)
    dli     a2, 0x100
    and     a1, a1, a2
    beqz    a1, 1b
    nop

    daddi   v0, a0, SWLVL_RESP
    ld      a1, 0x0(v0)
    dli     a2, 0xff00
    and     a1, a1, a2
    beqz    a1, 3b
    nop

slice_1:
    //daddi   v0, a0, INT_STATUS
    daddi   v0, a0, SWLVL_DONE
1:
    ld      a1, 0x0(v0)
    dli     a2, 0x100
    and     a1, a1, a2
    beqz    a1, 1b
    nop

    daddi   v0, a0, WRLVL_DELAY
    ld      a2, 0x0(v0)
    dli     a1, 0x0000ffffffffffff
    and     a2, a2, a1
    sd      a2, 0x0(v0)
    nop
3:

    daddi   v0, a0, WRLVL_DELAY
    ld      a2, 0x0(v0)
    dli     a1, 0x0001000000000000
    dadd    a2, a2, a1
    sd      a2, 0x0(v0)
    nop

    daddi   v0, a0, SWLVL_LOAD
    ld      a2, 0x0(v0)
    dli     a1, 0x0000000100000000
    or      a2, a2, a1
    sd      a2, 0x0(v0)
    nop

    //daddi   v0, a0, INT_STATUS
    daddi   v0, a0, SWLVL_DONE
1:
    ld      a1, 0x0(v0)
    dli     a2, 0x100
    and     a1, a1, a2
    beqz    a1, 1b
    nop

    daddi   v0, a0, SWLVL_RESP
    ld      a1, 0x0(v0)
    dli     a2, 0xff0000
    and     a1, a1, a2
    beqz    a1, 3b
    nop

slice_2:
    //daddi   v0, a0, INT_STATUS
    daddi   v0, a0, SWLVL_DONE
1:
    ld      a1, 0x0(v0)
    dli     a2, 0x100
    and     a1, a1, a2
    beqz    a1, 1b
    nop

    daddi   v0, a0, WRLVL_DELAY
    ld      a2, 0x10(v0)
    dli     a1, 0x0000000000000000
    and     a2, a2, a1
    sd      a2, 0x10(v0)
    nop
3:

    daddi   v0, a0, WRLVL_DELAY
    ld      a2, 0x10(v0)
    dli     a1, 0x0000000000000001
    dadd    a2, a2, a1
    sd      a2, 0x10(v0)
    nop

    daddi   v0, a0, SWLVL_LOAD
    ld      a2, 0x0(v0)
    dli     a1, 0x0000000100000000
    or      a2, a2, a1
    sd      a2, 0x0(v0)
    nop

    //daddi   v0, a0, INT_STATUS
    daddi   v0, a0, SWLVL_DONE
1:
    ld      a1, 0x0(v0)
    dli     a2, 0x100
    and     a1, a1, a2
    beqz    a1, 1b
    nop

    daddi   v0, a0, SWLVL_RESP
    ld      a1, 0x0(v0)
    dli     a2, 0xff000000
    and     a1, a1, a2
    beqz    a1, 3b
    nop

slice_3:
    //daddi   v0, a0, INT_STATUS
    daddi   v0, a0, SWLVL_DONE
1:
    ld      a1, 0x0(v0)
    dli     a2, 0x100
    and     a1, a1, a2
    beqz    a1, 1b
    nop

    daddi   v0, a0, WRLVL_DELAY
    ld      a2, 0x10(v0)
    dli     a1, 0x000000000000ffff
    and     a2, a2, a1
    sd      a2, 0x10(v0)
    nop
3:

    daddi   v0, a0, WRLVL_DELAY
    ld      a2, 0x10(v0)
    dli     a1, 0x0000000000010000
    dadd    a2, a2, a1
    sd      a2, 0x10(v0)
    nop

    daddi   v0, a0, SWLVL_LOAD
    ld      a2, 0x0(v0)
    dli     a1, 0x0000000100000000
    or      a2, a2, a1
    sd      a2, 0x0(v0)
    nop

    //daddi   v0, a0, INT_STATUS
    daddi   v0, a0, SWLVL_DONE
1:
    ld      a1, 0x0(v0)
    dli     a2, 0x100
    and     a1, a1, a2
    beqz    a1, 1b
    nop

    daddi   v0, a0, SWLVL_RESP
    ld      a1, 0x0(v0)
    dli     a2, 0xff00000000
    and     a1, a1, a2
    beqz    a1, 3b
    nop

slice_4:
    //daddi   v0, a0, INT_STATUS
    daddi   v0, a0, SWLVL_DONE
1:
    ld      a1, 0x0(v0)
    dli     a2, 0x100
    and     a1, a1, a2
    beqz    a1, 1b
    nop

    daddi   v0, a0, WRLVL_DELAY
    ld      a2, 0x10(v0)
    dli     a1, 0x00000000ffffffff
    and     a2, a2, a1
    sd      a2, 0x10(v0)
    nop
3:

    daddi   v0, a0, WRLVL_DELAY
    ld      a2, 0x10(v0)
    dli     a1, 0x0000000100000000
    dadd    a2, a2, a1
    sd      a2, 0x10(v0)
    nop

    daddi   v0, a0, SWLVL_LOAD
    ld      a2, 0x0(v0)
    dli     a1, 0x0000000100000000
    or      a2, a2, a1
    sd      a2, 0x0(v0)
    nop

    //daddi   v0, a0, INT_STATUS
    daddi   v0, a0, SWLVL_DONE
1:
    ld      a1, 0x0(v0)
    dli     a2, 0x100
    and     a1, a1, a2
    beqz    a1, 1b
    nop

    daddi   v0, a0, SWLVL_RESP
    ld      a1, 0x0(v0)
    dli     a2, 0xff0000000000
    and     a1, a1, a2
    beqz    a1, 3b
    nop

slice_5:
    //daddi   v0, a0, INT_STATUS
    daddi   v0, a0, SWLVL_DONE
1:
    ld      a1, 0x0(v0)
    dli     a2, 0x100
    and     a1, a1, a2
    beqz    a1, 1b
    nop

    daddi   v0, a0, WRLVL_DELAY
    ld      a2, 0x10(v0)
    dli     a1, 0x0000ffffffffffff
    and     a2, a2, a1
    sd      a2, 0x10(v0)
    nop
3:

    daddi   v0, a0, WRLVL_DELAY
    ld      a2, 0x10(v0)
    dli     a1, 0x0001000000000000
    dadd    a2, a2, a1
    sd      a2, 0x10(v0)
    nop

    daddi   v0, a0, SWLVL_LOAD
    ld      a2, 0x0(v0)
    dli     a1, 0x0000000100000000
    or      a2, a2, a1
    sd      a2, 0x0(v0)
    nop

    //daddi   v0, a0, INT_STATUS
    daddi   v0, a0, SWLVL_DONE
1:
    ld      a1, 0x0(v0)
    dli     a2, 0x100
    and     a1, a1, a2
    beqz    a1, 1b
    nop

    daddi   v0, a0, SWLVL_RESP
    ld      a1, 0x0(v0)
    dli     a2, 0xff000000000000
    and     a1, a1, a2
    beqz    a1, 3b
    nop

slice_6:
    //daddi   v0, a0, INT_STATUS
    daddi   v0, a0, SWLVL_DONE
1:
    ld      a1, 0x0(v0)
    dli     a2, 0x100
    and     a1, a1, a2
    beqz    a1, 1b
    nop

    daddi   v0, a0, WRLVL_DELAY
    ld      a2, 0x20(v0)
    dli     a1, 0xffff000000000000
    and     a2, a2, a1
    sd      a2, 0x20(v0)
    nop
3:

    daddi   v0, a0, WRLVL_DELAY
    ld      a2, 0x20(v0)
    dli     a1, 0x0000000000000001
    dadd    a2, a2, a1
    sd      a2, 0x20(v0)
    nop

    daddi   v0, a0, SWLVL_LOAD
    ld      a2, 0x0(v0)
    dli     a1, 0x0000000100000000
    or      a2, a2, a1
    sd      a2, 0x0(v0)
    nop

    //daddi   v0, a0, INT_STATUS
    daddi   v0, a0, SWLVL_DONE
1:
    ld      a1, 0x0(v0)
    dli     a2, 0x100
    and     a1, a1, a2
    beqz    a1, 1b
    nop

    daddi   v0, a0, SWLVL_RESP
    ld      a1, 0x0(v0)
    dli     a2, 0xff00000000000000
    and     a1, a1, a2
    beqz    a1, 3b
    nop

slice_7:
    //daddi   v0, a0, INT_STATUS
    daddi   v0, a0, SWLVL_DONE
1:
    ld      a1, 0x0(v0)
    dli     a2, 0x100
    and     a1, a1, a2
    beqz    a1, 1b
    nop

    daddi   v0, a0, WRLVL_DELAY
    ld      a2, 0x20(v0)
    dli     a1, 0x000000000000ffff
    and     a2, a2, a1
    sd      a2, 0x20(v0)
    nop
3:

    daddi   v0, a0, WRLVL_DELAY
    ld      a2, 0x20(v0)
    dli     a1, 0x0000000000010000
    dadd    a2, a2, a1
    sd      a2, 0x20(v0)
    nop

    daddi   v0, a0, SWLVL_LOAD
    ld      a2, 0x0(v0)
    dli     a1, 0x0000000100000000
    or      a2, a2, a1
    sd      a2, 0x0(v0)
    nop

    //daddi   v0, a0, INT_STATUS
    daddi   v0, a0, SWLVL_DONE
1:
    ld      a1, 0x0(v0)
    dli     a2, 0x100
    and     a1, a1, a2
    beqz    a1, 1b
    nop

    daddi   v0, a0, SWLVL_RESP
    ld      a1, 0x10(v0)
    dli     a2, 0xff
    and     a1, a1, a2
    beqz    a1, 3b
    nop

    daddi   v0, a0, SWLVL_EXIT
    ld      a2, 0x0(v0)
    dli     a1, 0x0000000001000000
    or      a2, a2, a1
    sd      a2, 0x0(v0)
    nop

    daddi   v0, a0, INT_STATUS
1:
    ld      a1, 0x0(v0)
    dli     a2, 0x4000
    and     a1, a1, a2
    beqz    a1, 1b
    nop

2:

#define GATE_REQ    0x720
#define GATE_EN     0x720
#define PHY_REG_0   0x2d0
#define PHY_REG_1   0x360
#define GATE_DELAY  0xa70

#if 0
gate_training:

    daddi   v0, a0, GATE_REQ
    dli     a1, 0x0100000000000000
    sd      a1, 0x0(v0)
    nop

    daddi   v0, a0, LVL_STATUS
    ld      a1, 0x0(v0)
    dli     a2, 0x400000000
    and     a1, a1, a2
    beqz    a1, 2f
    nop

    daddi   v0, a0, GATE_EN
    dli     a1, 0x0000000000000000
    sd      a1, 0x0(v0)
    nop

#if 1
    daddi   v0, a0, PHY_REG_0
    ld      a1, 0x0(v0)
    dli     a2, 0x0001000000000000
    or      a1, a1, a2
    sd      a1, 0x0(v0)
    ld      a1, 0x10(v0)
    dli     a2, 0x0001000000010000
    or      a1, a1, a2
    sd      a1, 0x10(v0)
    ld      a1, 0x20(v0)
    dli     a2, 0x0001000000010000
    or      a1, a1, a2
    sd      a1, 0x20(v0)
    ld      a1, 0x30(v0)
    dli     a2, 0x0001000000010000
    or      a1, a1, a2
    sd      a1, 0x30(v0)
    ld      a1, 0x40(v0)
    dli     a2, 0x0001000000010000
    or      a1, a1, a2
    sd      a1, 0x40(v0)
    nop
#else
    daddi   v0, a0, PHY_REG_0
    ld      a1, 0x0(v0)
    dli     a2, 0xfffeffffffffffff
    and     a1, a1, a2
    sd      a1, 0x0(v0)
    ld      a1, 0x10(v0)
    dli     a2, 0xfffefffffffeffff
    and     a1, a1, a2
    sd      a1, 0x10(v0)
    ld      a1, 0x20(v0)
    dli     a2, 0xfffefffffffeffff
    and     a1, a1, a2
    sd      a1, 0x20(v0)
    ld      a1, 0x30(v0)
    dli     a2, 0xfffefffffffeffff
    and     a1, a1, a2
    sd      a1, 0x30(v0)
    ld      a1, 0x40(v0)
    dli     a2, 0xfffefffffffeffff
    and     a1, a1, a2
    sd      a1, 0x40(v0)
    nop
#endif

    daddi   v0, a0, SW_LVL_MODE
    dli     a2, 0x0003000101020101
    sd      a2, 0x0(v0)
    nop

    daddi   v0, a0, SWLVL_START
    ld      a2, 0x0(v0)
    dli     a1, 0x0000010000000000
    or      a2, a2, a1
    sd      a2, 0x0(v0)
    nop

gate_slice_0:
    daddi   v0, a0, SWLVL_DONE
1:
    ld      a1, 0x0(v0)
    dli     a2, 0x100
    and     a1, a1, a2
    beqz    a1, 1b
    nop

    daddi   v0, a0, GATE_DELAY
    ld      a2, 0x0(v0)
    dli     a1, 0x0000ffffffffffff
    and     a2, a2, a1
    sd      a2, 0x0(v0)
    nop

    //TTYDBG("HHHH111\r\n")
    //dli     a0, 0x900000000ff00000
3:

    daddi   v0, a0, GATE_DELAY
    ld      a2, 0x0(v0)
    dli     a1, 0x0001000000000000
    dadd    a2, a2, a1
    sd      a2, 0x0(v0)
    nop

    daddi   v0, a0, SWLVL_LOAD
    ld      a2, 0x0(v0)
    dli     a1, 0x0000000100000000
    or      a2, a2, a1
    sd      a2, 0x0(v0)
    nop

    daddi   v0, a0, SWLVL_DONE
1:
    ld      a1, 0x0(v0)
    dli     a2, 0x100
    and     a1, a1, a2
    beqz    a1, 1b
    nop

    daddi   v0, a0, SWLVL_RESP
    ld      a1, 0x0(v0)
    nop

    daddi   v0, a0, SWLVL_RESP
    ld      a1, 0x0(v0)
    dli     a2, 0x0100
    and     a1, a1, a2
    beqz    a1, 3b
    //bnez    a1, 3b
    nop
    
#if 1
gate_slice_1:
    daddi   v0, a0, SWLVL_DONE
1:
    ld      a1, 0x0(v0)
    dli     a2, 0x100
    and     a1, a1, a2
    beqz    a1, 1b
    nop

    daddi   v0, a0, GATE_DELAY
    ld      a2, 0x10(v0)
    dli     a1, 0x0000000000000000
    and     a2, a2, a1
    sd      a2, 0x10(v0)
    nop

3:

    daddi   v0, a0, GATE_DELAY
    ld      a2, 0x10(v0)
    dli     a1, 0x0000000000000001
    dadd    a2, a2, a1
    sd      a2, 0x10(v0)
    nop

    daddi   v0, a0, SWLVL_LOAD
    ld      a2, 0x0(v0)
    dli     a1, 0x0000000100000000
    or      a2, a2, a1
    sd      a2, 0x0(v0)
    nop

    daddi   v0, a0, SWLVL_DONE
1:
    ld      a1, 0x0(v0)
    dli     a2, 0x100
    and     a1, a1, a2
    beqz    a1, 1b
    nop

    daddi   v0, a0, SWLVL_RESP
    ld      a1, 0x0(v0)
    nop

    daddi   v0, a0, SWLVL_RESP
    ld      a1, 0x0(v0)
    dli     a2, 0xff0000
    and     a1, a1, a2
    beqz    a1, 3b
    nop

gate_slice_2:
    daddi   v0, a0, SWLVL_DONE
1:
    ld      a1, 0x0(v0)
    dli     a2, 0x100
    and     a1, a1, a2
    beqz    a1, 1b
    nop

    daddi   v0, a0, GATE_DELAY
    ld      a2, 0x10(v0)
    dli     a1, 0x000000000000ffff
    and     a2, a2, a1
    sd      a2, 0x10(v0)
    nop

3:

    daddi   v0, a0, GATE_DELAY
    ld      a2, 0x10(v0)
    dli     a1, 0x0000000000010000
    dadd    a2, a2, a1
    sd      a2, 0x10(v0)
    nop

    daddi   v0, a0, SWLVL_LOAD
    ld      a2, 0x0(v0)
    dli     a1, 0x0000000100000000
    or      a2, a2, a1
    sd      a2, 0x0(v0)
    nop

    daddi   v0, a0, SWLVL_DONE
1:
    ld      a1, 0x0(v0)
    dli     a2, 0x100
    and     a1, a1, a2
    beqz    a1, 1b
    nop

    daddi   v0, a0, SWLVL_RESP
    ld      a1, 0x0(v0)
    nop

    daddi   v0, a0, SWLVL_RESP
    ld      a1, 0x0(v0)
    dli     a2, 0xff000000
    and     a1, a1, a2
    beqz    a1, 3b
    nop

gate_slice_3:
    daddi   v0, a0, SWLVL_DONE
1:
    ld      a1, 0x0(v0)
    dli     a2, 0x100
    and     a1, a1, a2
    beqz    a1, 1b
    nop

    daddi   v0, a0, GATE_DELAY
    ld      a2, 0x10(v0)
    dli     a1, 0x00000000ffffffff
    and     a2, a2, a1
    sd      a2, 0x10(v0)
    nop

3:

    daddi   v0, a0, GATE_DELAY
    ld      a2, 0x10(v0)
    dli     a1, 0x0000000100000000
    dadd    a2, a2, a1
    sd      a2, 0x10(v0)
    nop

    daddi   v0, a0, SWLVL_LOAD
    ld      a2, 0x0(v0)
    dli     a1, 0x0000000100000000
    or      a2, a2, a1
    sd      a2, 0x0(v0)
    nop

    daddi   v0, a0, SWLVL_DONE
1:
    ld      a1, 0x0(v0)
    dli     a2, 0x100
    and     a1, a1, a2
    beqz    a1, 1b
    nop

    daddi   v0, a0, SWLVL_RESP
    ld      a1, 0x0(v0)
    nop

    daddi   v0, a0, SWLVL_RESP
    ld      a1, 0x0(v0)
    dli     a2, 0xff00000000
    and     a1, a1, a2
    beqz    a1, 3b
    nop

gate_slice_4:
    daddi   v0, a0, SWLVL_DONE
1:
    ld      a1, 0x0(v0)
    dli     a2, 0x100
    and     a1, a1, a2
    beqz    a1, 1b
    nop

    daddi   v0, a0, GATE_DELAY
    ld      a2, 0x10(v0)
    dli     a1, 0x0000ffffffffffff
    and     a2, a2, a1
    sd      a2, 0x10(v0)
    nop

3:

    daddi   v0, a0, GATE_DELAY
    ld      a2, 0x10(v0)
    dli     a1, 0x0001000000000000
    dadd    a2, a2, a1
    sd      a2, 0x10(v0)
    nop

    daddi   v0, a0, SWLVL_LOAD
    ld      a2, 0x0(v0)
    dli     a1, 0x0000000100000000
    or      a2, a2, a1
    sd      a2, 0x0(v0)
    nop

    daddi   v0, a0, SWLVL_DONE
1:
    ld      a1, 0x0(v0)
    dli     a2, 0x100
    and     a1, a1, a2
    beqz    a1, 1b
    nop

    daddi   v0, a0, SWLVL_RESP
    ld      a1, 0x0(v0)
    nop

    daddi   v0, a0, SWLVL_RESP
    ld      a1, 0x0(v0)
    dli     a2, 0xff0000000000
    and     a1, a1, a2
    beqz    a1, 3b
    nop

gate_slice_5:
    daddi   v0, a0, SWLVL_DONE
1:
    ld      a1, 0x0(v0)
    dli     a2, 0x100
    and     a1, a1, a2
    beqz    a1, 1b
    nop

    daddi   v0, a0, GATE_DELAY
    ld      a2, 0x20(v0)
    dli     a1, 0x0000000000000000
    and     a2, a2, a1
    sd      a2, 0x20(v0)
    nop

3:

    daddi   v0, a0, GATE_DELAY
    ld      a2, 0x20(v0)
    dli     a1, 0x0000000000000001
    dadd    a2, a2, a1
    sd      a2, 0x20(v0)
    nop

    daddi   v0, a0, SWLVL_LOAD
    ld      a2, 0x0(v0)
    dli     a1, 0x0000000100000000
    or      a2, a2, a1
    sd      a2, 0x0(v0)
    nop

    daddi   v0, a0, SWLVL_DONE
1:
    ld      a1, 0x0(v0)
    dli     a2, 0x100
    and     a1, a1, a2
    beqz    a1, 1b
    nop

    daddi   v0, a0, SWLVL_RESP
    ld      a1, 0x0(v0)
    nop

    daddi   v0, a0, SWLVL_RESP
    ld      a1, 0x0(v0)
    dli     a2, 0xff000000000000
    and     a1, a1, a2
    beqz    a1, 3b
    nop

gate_slice_6:
    daddi   v0, a0, SWLVL_DONE
1:
    ld      a1, 0x0(v0)
    dli     a2, 0x100
    and     a1, a1, a2
    beqz    a1, 1b
    nop

    daddi   v0, a0, GATE_DELAY
    ld      a2, 0x20(v0)
    dli     a1, 0x000000000000ffff
    and     a2, a2, a1
    sd      a2, 0x20(v0)
    nop

3:

    daddi   v0, a0, GATE_DELAY
    ld      a2, 0x20(v0)
    dli     a1, 0x0000000000010000
    dadd    a2, a2, a1
    sd      a2, 0x20(v0)
    nop

    daddi   v0, a0, SWLVL_LOAD
    ld      a2, 0x0(v0)
    dli     a1, 0x0000000100000000
    or      a2, a2, a1
    sd      a2, 0x0(v0)
    nop

    daddi   v0, a0, SWLVL_DONE
1:
    ld      a1, 0x0(v0)
    dli     a2, 0x100
    and     a1, a1, a2
    beqz    a1, 1b
    nop

    daddi   v0, a0, SWLVL_RESP
    ld      a1, 0x0(v0)
    nop

    daddi   v0, a0, SWLVL_RESP
    ld      a1, 0x0(v0)
    dli     a2, 0xff00000000000000
    and     a1, a1, a2
    beqz    a1, 3b
    nop

gate_slice_7:
    daddi   v0, a0, SWLVL_DONE
1:
    ld      a1, 0x0(v0)
    dli     a2, 0x100
    and     a1, a1, a2
    beqz    a1, 1b
    nop

    daddi   v0, a0, GATE_DELAY
    ld      a2, 0x20(v0)
    dli     a1, 0x00000000ffffffff
    and     a2, a2, a1
    sd      a2, 0x20(v0)
    nop

3:

    daddi   v0, a0, GATE_DELAY
    ld      a2, 0x20(v0)
    dli     a1, 0x0000000100000000
    dadd    a2, a2, a1
    sd      a2, 0x20(v0)
    nop

    daddi   v0, a0, SWLVL_LOAD
    ld      a2, 0x0(v0)
    dli     a1, 0x0000000100000000
    or      a2, a2, a1
    sd      a2, 0x0(v0)
    nop

    daddi   v0, a0, SWLVL_DONE
1:
    ld      a1, 0x0(v0)
    dli     a2, 0x100
    and     a1, a1, a2
    beqz    a1, 1b
    nop

    daddi   v0, a0, SWLVL_RESP
    ld      a1, 0x0(v0)
    nop

    daddi   v0, a0, SWLVL_RESP
    ld      a1, 0x10(v0)
    dli     a2, 0xff
    and     a1, a1, a2
    beqz    a1, 3b
    nop
#endif


    daddi   v0, a0, PHY_REG_0
    ld      a1, 0x0(v0)
    dli     a2, 0xfffeffffffffffff
    and     a1, a1, a2
    sd      a1, 0x0(v0)
    ld      a1, 0x10(v0)
    dli     a2, 0xfffefffffffeffff
    and     a1, a1, a2
    sd      a1, 0x10(v0)
    ld      a1, 0x20(v0)
    dli     a2, 0xfffefffffffeffff
    and     a1, a1, a2
    sd      a1, 0x20(v0)
    ld      a1, 0x30(v0)
    dli     a2, 0xfffefffffffeffff
    and     a1, a1, a2
    sd      a1, 0x30(v0)
    ld      a1, 0x40(v0)
    dli     a2, 0xfffefffffffeffff
    and     a1, a1, a2
    sd      a1, 0x40(v0)
    nop

    daddi   v0, a0, SWLVL_EXIT
    ld      a2, 0x0(v0)
    dli     a1, 0x0000000001000000
    or      a2, a2, a1
    sd      a2, 0x0(v0)
    nop

    daddi   v0, a0, SWLVL_DONE
1:
    ld      a1, 0x0(v0)
    dli     a2, 0x100
    and     a1, a1, a2
    beqz    a1, 1b
    nop


2:
#endif

#if 0
read_leveling:

#define RDLVL_REQ   0x740
#define RDLVL_EN    0x720
#define RDLVL_EDGE  0x940
#define RDLVL_DELAY 0xa30
#define RDLVL_FALL  0x240

    daddi   v0, a0, RDLVL_REQ
    dli     a1, 0x0000000000000100
    sd      a1, 0x0(v0)
    nop

    daddi   v0, a0, LVL_STATUS
    ld      a1, 0x0(v0)
    dli     a2, 0x200000000
    and     a1, a1, a2
    beqz    a1, 2f
    nop

    daddi   v0, a0, RDLVL_EN
    dli     a1, 0x0000000000000000
    sd      a1, 0x0(v0)
    nop

#ifdef FALL_EDGE
    daddi   v0, a0, RDLVL_EDGE
    ld      a1, 0x0(v0)
    dli     a2, 0x0000000001000000 //fall edge
    or      a1, a1, a2
    sd      a1, 0x0(v0)
    nop


    daddi   v0, a0, PHY_REG_0
    ld      a1, 0x0(v0)
    dli     a2, 0x0020000000000000
    or      a1, a1, a2
    sd      a1, 0x0(v0)
    ld      a1, 0x10(v0)
    dli     a2, 0x0020000000020000
    or      a1, a1, a2
    sd      a1, 0x10(v0)
    ld      a1, 0x20(v0)
    dli     a2, 0x0020000000020000
    or      a1, a1, a2
    sd      a1, 0x20(v0)
    ld      a1, 0x30(v0)
    dli     a2, 0x0020000000020000
    or      a1, a1, a2
    sd      a1, 0x30(v0)
    ld      a1, 0x40(v0)
    dli     a2, 0x0020000000020000
    or      a1, a1, a2
    sd      a1, 0x40(v0)
    nop
#endif

    daddi   v0, a0, SW_LVL_MODE
    dli     a2, 0x0002000101020101
    sd      a2, 0x0(v0)
    nop

    daddi   v0, a0, SWLVL_START
    ld      a2, 0x0(v0)
    dli     a1, 0x0000010000000000
    or      a2, a2, a1
    sd      a2, 0x0(v0)
    nop

read_slice_0:
    daddi   v0, a0, SWLVL_DONE
1:
    ld      a1, 0x0(v0)
    dli     a2, 0x100
    and     a1, a1, a2
    beqz    a1, 1b
    nop

#ifndef FALL_EDGE
    daddi   v0, a0, RDLVL_DELAY
    ld      a2, 0x0(v0)
    //dli     a1, 0xffff00ffffffffff
    dli     a1, 0xffffffff0000ffff
    and     a2, a2, a1
    dli     a1, 0x0000000000050000
    or      a2, a2, a1
    sd      a2, 0x0(v0)
    nop

3:

    daddi   v0, a0, RDLVL_DELAY
    ld      a2, 0x0(v0)
    dli     a1, 0x0000000000010000
    dadd    a2, a2, a1
    sd      a2, 0x0(v0)
    nop
#else
    daddi   v0, a0, RDLVL_FALL
    ld      a2, 0x0(v0)
    //dli     a1, 0xffff00ffffffffff
    dli     a1, 0xffffffffffff00ff
    and     a2, a2, a1
    sd      a2, 0x0(v0)
    nop

3:

    daddi   v0, a0, RDLVL_FALL
    ld      a2, 0x0(v0)
    dli     a1, 0x0000000000000100
    dadd    a2, a2, a1
    sd      a2, 0x0(v0)
    nop
#endif

    daddi   v0, a0, SWLVL_LOAD
    ld      a2, 0x0(v0)
    dli     a1, 0x0000000100000000
    or      a2, a2, a1
    sd      a2, 0x0(v0)
    nop

    daddi   v0, a0, SWLVL_DONE
1:
    ld      a1, 0x0(v0)
    dli     a2, 0x100
    and     a1, a1, a2
    beqz    a1, 1b
    nop

    daddi   v0, a0, SWLVL_RESP
    ld      a1, 0x0(v0)
    dli     a2, 0x0100
    and     a1, a1, a2
    beqz    a1, 3b
    //bnez    a1, 3b
    nop
    
read_slice_1:
    daddi   v0, a0, SWLVL_DONE
1:
    ld      a1, 0x0(v0)
    dli     a2, 0x100
    and     a1, a1, a2
    beqz    a1, 1b
    nop

#ifndef FALL_EDGE
    daddi   v0, a0, RDLVL_DELAY
    ld      a2, 0x0(v0)
    //dli     a1, 0xffff00ffffffffff
    dli     a1, 0xffff0000ffffffff
    and     a2, a2, a1
    dli     a1, 0x0000000500000000
    or      a2, a2, a1
    sd      a2, 0x0(v0)
    nop

3:

    daddi   v0, a0, RDLVL_DELAY
    ld      a2, 0x0(v0)
    dli     a1, 0x0000000100000000
    dadd    a2, a2, a1
    sd      a2, 0x0(v0)
    nop
#else
    daddi   v0, a0, RDLVL_FALL
    ld      a2, 0x0(v0)
    //dli     a1, 0xffff00ffffffffff
    dli     a1, 0xffff00ffffffffff
    and     a2, a2, a1
    sd      a2, 0x0(v0)
    nop

3:

    daddi   v0, a0, RDLVL_FALL
    ld      a2, 0x0(v0)
    dli     a1, 0x0000010000000000
    dadd    a2, a2, a1
    sd      a2, 0x0(v0)
    nop
#endif

    daddi   v0, a0, SWLVL_LOAD
    ld      a2, 0x0(v0)
    dli     a1, 0x0000000100000000
    or      a2, a2, a1
    sd      a2, 0x0(v0)
    nop

    daddi   v0, a0, SWLVL_DONE
1:
    ld      a1, 0x0(v0)
    dli     a2, 0x100
    and     a1, a1, a2
    beqz    a1, 1b
    nop

    daddi   v0, a0, SWLVL_RESP
    ld      a1, 0x0(v0)
    dli     a2, 0x010000
    and     a1, a1, a2
    beqz    a1, 3b
    //bnez    a1, 3b
    nop
    
read_slice_2:
    daddi   v0, a0, SWLVL_DONE
1:
    ld      a1, 0x0(v0)
    dli     a2, 0x100
    and     a1, a1, a2
    beqz    a1, 1b
    nop

    daddi   v0, a0, RDLVL_DELAY
    ld      a2, 0x0(v0)
    //dli     a1, 0xffff00ffffffffff
    dli     a1, 0x0000ffffffffffff
    and     a2, a2, a1
    dli     a1, 0x0005000000000000
    or      a2, a2, a1
    sd      a2, 0x0(v0)
    nop

3:

    daddi   v0, a0, RDLVL_DELAY
    ld      a2, 0x0(v0)
    dli     a1, 0x0001000000000000
    dadd    a2, a2, a1
    sd      a2, 0x0(v0)
    nop
/*
    daddi   v0, a0, RDLVL_FALL
    ld      a2, 0x0(v0)
    //dli     a1, 0xffff00ffffffffff
    dli     a1, 0xffffffffffff00ff
    and     a2, a2, a1
    sd      a2, 0x0(v0)
    nop

3:

    daddi   v0, a0, RDLVL_FALL
    ld      a2, 0x0(v0)
    dli     a1, 0x0000000000000100
    dadd    a2, a2, a1
    sd      a2, 0x0(v0)
    nop
    */

    daddi   v0, a0, SWLVL_LOAD
    ld      a2, 0x0(v0)
    dli     a1, 0x0000000100000000
    or      a2, a2, a1
    sd      a2, 0x0(v0)
    nop

    daddi   v0, a0, SWLVL_DONE
1:
    ld      a1, 0x0(v0)
    dli     a2, 0x100
    and     a1, a1, a2
    beqz    a1, 1b
    nop

    daddi   v0, a0, SWLVL_RESP
    ld      a1, 0x0(v0)
    dli     a2, 0x01000000
    and     a1, a1, a2
    beqz    a1, 3b
    //bnez    a1, 3b
    nop
    
read_slice_3:
    daddi   v0, a0, SWLVL_DONE
1:
    ld      a1, 0x0(v0)
    dli     a2, 0x100
    and     a1, a1, a2
    beqz    a1, 1b
    nop

    daddi   v0, a0, RDLVL_DELAY
    ld      a2, 0x10(v0)
    //dli     a1, 0xffff00ffffffffff
    dli     a1, 0xffffffffffff0000
    and     a2, a2, a1
    dli     a1, 0x0000000000000005
    or      a2, a2, a1
    sd      a2, 0x10(v0)
    nop

3:

    daddi   v0, a0, RDLVL_DELAY
    ld      a2, 0x10(v0)
    dli     a1, 0x0000000000000001
    dadd    a2, a2, a1
    sd      a2, 0x10(v0)
    nop
/*
    daddi   v0, a0, RDLVL_FALL
    ld      a2, 0x0(v0)
    //dli     a1, 0xffff00ffffffffff
    dli     a1, 0xffffffffffff00ff
    and     a2, a2, a1
    sd      a2, 0x0(v0)
    nop

3:

    daddi   v0, a0, RDLVL_FALL
    ld      a2, 0x0(v0)
    dli     a1, 0x0000000000000100
    dadd    a2, a2, a1
    sd      a2, 0x0(v0)
    nop
    */

    daddi   v0, a0, SWLVL_LOAD
    ld      a2, 0x0(v0)
    dli     a1, 0x0000000100000000
    or      a2, a2, a1
    sd      a2, 0x0(v0)
    nop

    daddi   v0, a0, SWLVL_DONE
1:
    ld      a1, 0x0(v0)
    dli     a2, 0x100
    and     a1, a1, a2
    beqz    a1, 1b
    nop

    daddi   v0, a0, SWLVL_RESP
    ld      a1, 0x0(v0)
    dli     a2, 0x0100000000
    and     a1, a1, a2
    beqz    a1, 3b
    //bnez    a1, 3b
    nop
    
read_slice_4:
    daddi   v0, a0, SWLVL_DONE
1:
    ld      a1, 0x0(v0)
    dli     a2, 0x100
    and     a1, a1, a2
    beqz    a1, 1b
    nop

    daddi   v0, a0, RDLVL_DELAY
    ld      a2, 0x10(v0)
    //dli     a1, 0xffff00ffffffffff
    dli     a1, 0xffffffff0000ffff
    and     a2, a2, a1
    dli     a1, 0x0000000000050000
    or      a2, a2, a1
    sd      a2, 0x10(v0)
    nop

3:

    daddi   v0, a0, RDLVL_DELAY
    ld      a2, 0x10(v0)
    dli     a1, 0x0000000000010000
    dadd    a2, a2, a1
    sd      a2, 0x10(v0)
    nop
/*
    daddi   v0, a0, RDLVL_FALL
    ld      a2, 0x0(v0)
    //dli     a1, 0xffff00ffffffffff
    dli     a1, 0xffffffffffff00ff
    and     a2, a2, a1
    sd      a2, 0x0(v0)
    nop

3:

    daddi   v0, a0, RDLVL_FALL
    ld      a2, 0x0(v0)
    dli     a1, 0x0000000000000100
    dadd    a2, a2, a1
    sd      a2, 0x0(v0)
    nop
    */

    daddi   v0, a0, SWLVL_LOAD
    ld      a2, 0x0(v0)
    dli     a1, 0x0000000100000000
    or      a2, a2, a1
    sd      a2, 0x0(v0)
    nop

    daddi   v0, a0, SWLVL_DONE
1:
    ld      a1, 0x0(v0)
    dli     a2, 0x100
    and     a1, a1, a2
    beqz    a1, 1b
    nop

    daddi   v0, a0, SWLVL_RESP
    ld      a1, 0x0(v0)
    dli     a2, 0x010000000000
    and     a1, a1, a2
    beqz    a1, 3b
    //bnez    a1, 3b
    nop
    
read_slice_5:
    daddi   v0, a0, SWLVL_DONE
1:
    ld      a1, 0x0(v0)
    dli     a2, 0x100
    and     a1, a1, a2
    beqz    a1, 1b
    nop

    daddi   v0, a0, RDLVL_DELAY
    ld      a2, 0x10(v0)
    //dli     a1, 0xffff00ffffffffff
    dli     a1, 0xffff0000ffffffff
    and     a2, a2, a1
    dli     a1, 0x0000000500000000
    or      a2, a2, a1
    sd      a2, 0x10(v0)
    nop

3:

    daddi   v0, a0, RDLVL_DELAY
    ld      a2, 0x10(v0)
    dli     a1, 0x0000000100000000
    dadd    a2, a2, a1
    sd      a2, 0x10(v0)
    nop
/*
    daddi   v0, a0, RDLVL_FALL
    ld      a2, 0x0(v0)
    //dli     a1, 0xffff00ffffffffff
    dli     a1, 0xffffffffffff00ff
    and     a2, a2, a1
    sd      a2, 0x0(v0)
    nop

3:

    daddi   v0, a0, RDLVL_FALL
    ld      a2, 0x0(v0)
    dli     a1, 0x0000000000000100
    dadd    a2, a2, a1
    sd      a2, 0x0(v0)
    nop
    */

    daddi   v0, a0, SWLVL_LOAD
    ld      a2, 0x0(v0)
    dli     a1, 0x0000000100000000
    or      a2, a2, a1
    sd      a2, 0x0(v0)
    nop

    daddi   v0, a0, SWLVL_DONE
1:
    ld      a1, 0x0(v0)
    dli     a2, 0x100
    and     a1, a1, a2
    beqz    a1, 1b
    nop

    daddi   v0, a0, SWLVL_RESP
    ld      a1, 0x0(v0)
    dli     a2, 0x01000000000000
    and     a1, a1, a2
    beqz    a1, 3b
    //bnez    a1, 3b
    nop
    
read_slice_6:
    daddi   v0, a0, SWLVL_DONE
1:
    ld      a1, 0x0(v0)
    dli     a2, 0x100
    and     a1, a1, a2
    beqz    a1, 1b
    nop

    daddi   v0, a0, RDLVL_DELAY
    ld      a2, 0x10(v0)
    //dli     a1, 0xffff00ffffffffff
    dli     a1, 0x0000ffffffffffff
    and     a2, a2, a1
    dli     a1, 0x0005000000000000
    or      a2, a2, a1
    sd      a2, 0x10(v0)
    nop

3:

    daddi   v0, a0, RDLVL_DELAY
    ld      a2, 0x10(v0)
    dli     a1, 0x0001000000000000
    dadd    a2, a2, a1
    sd      a2, 0x10(v0)
    nop
/*
    daddi   v0, a0, RDLVL_FALL
    ld      a2, 0x0(v0)
    //dli     a1, 0xffff00ffffffffff
    dli     a1, 0xffffffffffff00ff
    and     a2, a2, a1
    sd      a2, 0x0(v0)
    nop

3:

    daddi   v0, a0, RDLVL_FALL
    ld      a2, 0x0(v0)
    dli     a1, 0x0000000000000100
    dadd    a2, a2, a1
    sd      a2, 0x0(v0)
    nop
    */

    daddi   v0, a0, SWLVL_LOAD
    ld      a2, 0x0(v0)
    dli     a1, 0x0000000100000000
    or      a2, a2, a1
    sd      a2, 0x0(v0)
    nop

    daddi   v0, a0, SWLVL_DONE
1:
    ld      a1, 0x0(v0)
    dli     a2, 0x100
    and     a1, a1, a2
    beqz    a1, 1b
    nop

    daddi   v0, a0, SWLVL_RESP
    ld      a1, 0x0(v0)
    dli     a2, 0x0100000000000000
    and     a1, a1, a2
    beqz    a1, 3b
    //bnez    a1, 3b
    nop
    
read_slice_7:
    daddi   v0, a0, SWLVL_DONE
1:
    ld      a1, 0x0(v0)
    dli     a2, 0x100
    and     a1, a1, a2
    beqz    a1, 1b
    nop

    daddi   v0, a0, RDLVL_DELAY
    ld      a2, 0x20(v0)
    //dli     a1, 0xffff00ffffffffff
    dli     a1, 0xffffffffffff0000
    and     a2, a2, a1
    dli     a1, 0x0000000000000005
    or      a2, a2, a1
    sd      a2, 0x20(v0)
    nop

3:

    daddi   v0, a0, RDLVL_DELAY
    ld      a2, 0x20(v0)
    dli     a1, 0x0000000000000001
    dadd    a2, a2, a1
    sd      a2, 0x20(v0)
    nop
/*
    daddi   v0, a0, RDLVL_FALL
    ld      a2, 0x0(v0)
    //dli     a1, 0xffff00ffffffffff
    dli     a1, 0xffffffffffff00ff
    and     a2, a2, a1
    sd      a2, 0x0(v0)
    nop

3:

    daddi   v0, a0, RDLVL_FALL
    ld      a2, 0x0(v0)
    dli     a1, 0x0000000000000100
    dadd    a2, a2, a1
    sd      a2, 0x0(v0)
    nop
    */

    daddi   v0, a0, SWLVL_LOAD
    ld      a2, 0x0(v0)
    dli     a1, 0x0000000100000000
    or      a2, a2, a1
    sd      a2, 0x0(v0)
    nop

    daddi   v0, a0, SWLVL_DONE
1:
    ld      a1, 0x0(v0)
    dli     a2, 0x100
    and     a1, a1, a2
    beqz    a1, 1b
    nop

    daddi   v0, a0, SWLVL_RESP
    ld      a1, 0x10(v0)
    dli     a2, 0x01
    and     a1, a1, a2
    beqz    a1, 3b
    //bnez    a1, 3b
    nop
    
    daddi   v0, a0, SWLVL_EXIT
    ld      a2, 0x0(v0)
    dli     a1, 0x0000000001000000
    or      a2, a2, a1
    sd      a2, 0x0(v0)
    nop

    daddi   v0, a0, SWLVL_DONE
1:
    ld      a1, 0x0(v0)
    dli     a2, 0x100
    and     a1, a1, a2
    beqz    a1, 1b
    nop


2:
#endif

    move    ra, t8
	jr      ra
	nop
    .end    ddr3_leveling
