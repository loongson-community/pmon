//#include "regdef.h"
//#include "mipsregs.h"
//#include "cacheops.h"

#define WAIT_ITEM   0x60
#define CLK_PHASE 0x38
#define DBL_CFG     0x40
#define RTT_240     0x4
#define RTT_120     0x2
#define RTT_80      0x6
#define RTT_60      0x1
#define RTT_48      0x5
#define RTT_40      0x3
#define RTT_34      0x7
#define RTT_DISABLE 0x0

#define IDLE_TIMES  0x10
//#define PARAM_SET_FROM_SPD_DEBUG
#define SET_BY_PROTOCOL
#define REFRESH_1X
#ifdef AUTO_DDR_CONFIG
//#define SPD_ONLY
#endif

//#define PRINTSTR(x) \
//    .rdata;98: .asciz x; .text; la a0, 98b; bal outputstring; nop


        .text
        .set    noreorder
        .set    mips3

        .global mc_config
        .ent    mc_config

mc_config:
    move    t7, ra
    move    k1, a2
mc_reset:
    daddu   a2, a2, s0
    nop
    dli     t1, 744
    daddiu  v0, t8, 0x0
//write param registers
1:
    ld      a1, 0x0(a2)
    sd      a1, 0x0(v0)
    daddiu  t1, t1, -1
    daddiu  a2, a2, 0x8
    daddiu  v0, v0, 0x8
    bnez    t1, 1b
    nop
#if (SEL_DBL == 1)
#ifdef  MULTI_CHIP
#define NODE0_MC0_DBL   0x2c00000030
#define NODE0_MC1_DBL   0x2800000030
#define NODE1_MC0_DBL   0x000000002e
#define NODE1_MC1_DBL   0x0000000030
#define NODE2_MC0_DBL   0x0000000030
#define NODE2_MC1_DBL   0x0000000031
#define NODE3_MC0_DBL   0x0000000030
#define NODE3_MC1_DBL   0x0000000032
#define NODE0_MC0_CLK   0x144
#define NODE0_MC1_CLK   0x144
#define NODE1_MC0_CLK   0x144
#define NODE1_MC1_CLK   0x144
#define NODE2_MC0_CLK   0x144
#define NODE2_MC1_CLK   0x144
#define NODE3_MC0_CLK   0x144
#define NODE3_MC1_CLK   0x144

    GET_NODE_ID_a1
    bnez    a1, 2f
    nop
    bnez    k0, 1f
    nop
    dli     t1, NODE0_MC0_DBL
    dli     t0, NODE0_MC0_CLK
    b       10f
    nop
1:
    dli     t1, NODE0_MC1_DBL
    dli     t0, NODE0_MC1_CLK
    b       10f
    nop
2:
    bne     a1, 1, 2f
    nop
    bnez    k0, 1f
    nop
    dli     t1, NODE1_MC0_DBL
    dli     t0, NODE1_MC0_CLK
    b       10f
    nop
1:
    dli     t1, NODE1_MC1_DBL
    dli     t0, NODE1_MC1_CLK
    b       10f
    nop
2:
    bne     a1, 2, 2f
    nop
    bnez    k0, 1f
    nop
    dli     t1, NODE2_MC0_DBL
    dli     t0, NODE2_MC0_CLK
    b       10f
    nop
1:
    dli     t1, NODE2_MC1_DBL
    dli     t0, NODE2_MC1_CLK
    b       10f
    nop
2:
    bnez    k0, 1f
    nop
    dli     t1, NODE3_MC0_DBL
    dli     t0, NODE3_MC0_CLK
    b       10f
    nop
1:
    dli     t1, NODE3_MC1_DBL
    dli     t0, NODE3_MC1_CLK
10:
    sd      t1, DBL_CFG(t8)
    sb      t0, CLK_PHASE(t8)
#endif
#endif



//reconfig *_diff

#if 1

#if 1 //fix the NODE windows hit
    GET_NODE_ID_a0;
    ld		a1, 0x1500(t8)
    or		a1, a1, a0
    sd		a1, 0x1500(t8)

    ld		a1, 0x1508(t8)
    or		a1, a1, a0
    sd		a1, 0x1508(t8)

    ld      a1, 0x1510(t8)
    or      a1, a1, a0
    sd      a1, 0x1510(t8)

#endif

#ifdef  TWO_T_MODE_ENABLE
    dli     t0, 1
    sb      t0, DDR4_2T_OFFSET(t8)
#else
    sb      zero, DDR4_2T_OFFSET(t8)
#endif

#ifdef DISABLE_DIMM_ECC
    sb      zero, DDR4_ECC_EN_OFFSET(t8)
#else
    GET_DIMM_ECC_V1
    sb      a1, DDR4_ECC_EN_OFFSET(t8)
#endif

#ifndef DISABLE_DRAM_CRC
//add tW2R_*_adj
    ld      a1, 0x1070(t8)
    dli     t0, (0x10101<<24)
    daddu   a1, t0
    and     a1, 0x3f3f3fffffff
    sd      a1, 0x1070(t8)

    ld      a1, 0x1078(t8)
    dli     t0, (0x10101<<24)
    daddu   a1, t0
    and     a1, 0x3f3f3fffffff
    sd      a1, 0x1078(t8)

    ld      a1, 0x1080(t8)
    dli     t0, (0x10101<<24)
    daddu   a1, t0
    and     a1, 0x3f3f3fffffff
    sd      a1, 0x1080(t8)

    ld      a1, 0x1090(t8)
    dli     t0, (0x10101<<24)
    daddu   a1, t0
    and     a1, 0x3f3f3fffffff
    sd      a1, 0x1090(t8)

    ld      a1, 0x1098(t8)
    dli     t0, (0x10101<<24)
    daddu   a1, t0
    and     a1, 0x3f3f3fffffff
    sd      a1, 0x1098(t8)
//add tR2R_samec_adj to fixup the problem of unable to boot kernal with enable CRC
    lbu     a1, 0x1080(t8)
    daddu   a1, 1
    and     a1, 0x3f
    sb      a1, 0x1080(t8)
#endif

#ifndef DISABLE_READ_DBI
    ld      t0, 0x1450(t8)
    dli     a1, 0x1
    dsll    a1, 17
    or      t0, a1
    sd      t0, 0x1450(t8)
    lhu     t0, 0x118a(t8)
    ori     t0, (0x1<<12)
    sh      t0, 0x118a(t8)
    sh      t0, 0x119a(t8)
    sh      t0, 0x11aa(t8)
    sh      t0, 0x11ba(t8)
    sh      t0, 0x11ca(t8)
    sh      t0, 0x11da(t8)
    sh      t0, 0x11ea(t8)
    sh      t0, 0x11fa(t8)
#endif

#ifndef DISABLE_WRITE_DBI
    GET_SDRAM_WIDTH_V1
    beqz    a1, 1f
    nop
//disable DM and enable WR_DBI in MRS
    lhu     t0, 0x118a(t8)
    dli     t1, 0x3
    dsll    t1, 10
    not     t1
    and     t0, t1
    dli     t1, 0x2
    dsll    t1, 10
    or      t0, t1
    sh      t0, 0x118a(t8)
    sh      t0, 0x119a(t8)
    sh      t0, 0x11aa(t8)
    sh      t0, 0x11ba(t8)
    sh      t0, 0x11ca(t8)
    sh      t0, 0x11da(t8)
    sh      t0, 0x11ea(t8)
    sh      t0, 0x11fa(t8)
//enable rd before wr
    ld      t0, 0x1280(t8)
    dli     t1, 0xff
    dsll    t1, 40
    not     t1
    and     t0, t1
    dli     t1, 0x1
    dsll    t1, 40
    or      t0, t1
    sd      t0, 0x1280(t8)
//enable wr dbi in MC controller
    ld      t0, 0x1450(t8)
    dli     a1, 0x1
    dsll    a1, 16
    or      t0, a1
    sd      t0, 0x1450(t8)
1:
#endif

//config ddr3
    GET_SDRAM_TYPE_V1
    beq     a1, 0x4, 1f
    nop
    li      t0, 0x1
    sb      t0, 0xc(t8)
1:

    GET_COL_SIZE_V1
    sb      a1, DDR4_COL_DIFF_OFFSET(t8)

    GET_ROW_SIZE_V1
    sb      a1, DDR4_ROW_DIFF_OFFSET(t8)

    GET_BA_NUM_V1
    not     a1, a1
    andi    a1, 0x1
    sb      a1, DDR4_BA_DIFF_OFFSET(t8)

    GET_BG_NUM_V1
    dli     a0, 0x2
    dsubu   a1, a0, a1
    sb      a1, DDR4_BG_DIFF_OFFSET(t8)

    GET_CID_NUM_V1
    dli     a0, 3
    dsubu   a0, a1
    sb      a0, DDR4_C_DIFF_OFFSET(t8)

    GET_ADDR_MIRROR_V1
    dli     t0, 0xff
    not     t0
    ld      t1, 0x1208(t8)
    and     t1, t0
    beqz    a1, 1f
    nop
    dli     t0, 0xaa
    or      t1, t0
1:
    sd      t1, 0x1208(t8)
//config the DIMM width parameter
    ld      t0, 0x1200(t8)
    dli     t1, 0xffff
    dsll    t1, 24
    not     t1
    and     t0, t1
    GET_DIMM_WIDTH_V1
    beq     a1, 0x1, 1f
    nop
    beq     a1, 0x2, 2f
    nop
    beq     a1, 0x3, 3f
    nop
1:
    dli     t1, 0x0301
    b       4f
    nop
2:
    dli     t1, 0x0502
    b       4f
    nop
3:
    dli     t1, 0x0003
4:
    dsll    t1, 24
    or      t0, t1
    sd      t0, 0x1200(t8)

//if x4
    GET_SDRAM_WIDTH_V1
    bnez    a1, 1f
    nop
    ld      t0, 0x8(t8)
    dli     t1, 0xff
    dsll    t1, 40
    not     t1
    and     t0, t1
    dli     t1, 0x3
    dsll    t1, 40
    or      t0, t1
    sd      t0, 0x8(t8)
//disable DM in MRS
    lhu     t0, 0x118a(t8)
    dli     t1, 0x1
    dsll    t1, 10
    not     t1
    and     t0, t1
    sh      t0, 0x118a(t8)
    sh      t0, 0x119a(t8)
    sh      t0, 0x11aa(t8)
    sh      t0, 0x11ba(t8)
    sh      t0, 0x11ca(t8)
    sh      t0, 0x11da(t8)
    sh      t0, 0x11ea(t8)
    sh      t0, 0x11fa(t8)
    ld      t0, 0x1280(t8)
    dli     t1, 0xff
    dsll    t1, 40
    not     t1
    and     t0, t1
    dli     t1, 0x1
    dsll    t1, 40
    or      t0, t1
    sd      t0, 0x1280(t8)
1:
//change according to Burst length
    ld      t0, 0x1280(t8)
    dli     t1, 0xff
    dsll    t1, 56
    not     t1
    and     t0, t1
    lb      t2, 0x1440(t8)
    beq     t2, 0x3, 1f
    nop
    dli     t1, 0x1
    dsll    t1, 56
    or      t0, t1
1:
    sd      t0, 0x1280(t8)

    dli     t0, 0x0
    dli     t1, 0x8
    GET_MC_CS_MAP_V1
1:
    andi    a0, a1, 0x1
    daddu   t0, a0
    dsrl    a1, 0x1
    dsubu   t1, 0x1
    bnez    t1, 1b
    nop

    beq     t0, 0x1, 1f
    nop
    beq     t0, 0x2, 2f
    nop
    beq     t0, 0x4, 4f
    nop
    dli     a0, 0x0 //8CS
    sb      a0, DDR4_CS_DIFF_OFFSET(t8)
    dli     a0, 0xff
    sb      a0, DDR4_CS_ENABLE_OFFSET(t8)
    sb      a0, DDR4_CS_ZQ_OFFSET(t8)
    sb      a0, DDR4_CS_ZQCL_OFFSET(t8)
    sb      a0, DDR4_CS_RESYNC_OFFSET(t8)
    sb      a0, DDR4_CS_REF_OFFSET(t8)
    b       3f
    nop


1://1CS
    dli     a0, 0x3
    sb      a0, DDR4_CS_DIFF_OFFSET(t8)
    dli     a0, 0x1
    sb      a0, DDR4_CS_ENABLE_OFFSET(t8)
    sb      a0, DDR4_CS_ZQ_OFFSET(t8)
    sb      a0, DDR4_CS_ZQCL_OFFSET(t8)
    sb      a0, DDR4_CS_RESYNC_OFFSET(t8)
    sb      a0, DDR4_CS_REF_OFFSET(t8)
    b       3f
    nop

2://2CS
    dli     a0, 0x2
    sb      a0, DDR4_CS_DIFF_OFFSET(t8)
    dli     a0, 0x3
    sb      a0, DDR4_CS_ENABLE_OFFSET(t8)
    sb      a0, DDR4_CS_ZQ_OFFSET(t8)
    sb      a0, DDR4_CS_ZQCL_OFFSET(t8)
    sb      a0, DDR4_CS_RESYNC_OFFSET(t8)
    sb      a0, DDR4_CS_REF_OFFSET(t8)
    b       3f
    nop

4://4CS
    dli     a0, 0x1
    sb      a0, DDR4_CS_DIFF_OFFSET(t8)
    dli     a0, 0xf
    sb      a0, DDR4_CS_ENABLE_OFFSET(t8)
    sb      a0, DDR4_CS_ZQ_OFFSET(t8)
    sb      a0, DDR4_CS_ZQCL_OFFSET(t8)
    sb      a0, DDR4_CS_RESYNC_OFFSET(t8)
    sb      a0, DDR4_CS_REF_OFFSET(t8)
    b       3f
    nop

3:
    /*config cs_mrs, according to DIMM type */
    lb      a0, DDR4_CS_ENABLE_OFFSET(t8)
    GET_DIMM_TYPE_V1
    andi    a1, 1
    beqz    a1, 1f
    nop
    dli     a0, 0
1:
    sb      a0, DDR4_CS_MRS_OFFSET(t8)

//cs_map

    GET_MC_CS_MAP_V1
    dli     a0, 0x01
    beq     a0, a1, 1f
    nop
    dli     a0, 0x10
    beq     a0, a1, 2f
    nop
    dli     a0, 0x11
    beq     a0, a1, 3f
    nop
    dli     a0, 0x03
    beq     a0, a1, 4f
    nop
    dli     a0, 0x30
    beq     a0, a1, 5f
    nop
    dli     a0, 0x33
    beq     a0, a1, 6f
    nop
    PRINTSTR("\r\nError: The CS MAP is not supported!!!!!!!")
10:
    b       10b
    nop

1:
    dli     a0, 0x0
    sw      a0, DDR4_CS_MAP_OFFSET(t8)
    dli     a0, 0x1
    sw      a0, DDR4_CKE_MAP_OFFSET(t8)
    dli     a0, 0x1
    sw      a0, DDR4_WRODT_MAP_OFFSET(t8)
    b       9f
    nop

2:
    dli     a0, 0x4
    sw      a0, DDR4_CS_MAP_OFFSET(t8)
    dli     a0, 0x4
    sw      a0, DDR4_CKE_MAP_OFFSET(t8)
    dli     a0, 0x4
    sw      a0, DDR4_WRODT_MAP_OFFSET(t8)
    b       9f
    nop

3:
    dli     a0, 0x40
    sw      a0, DDR4_CS_MAP_OFFSET(t8)
    dli     a0, 0x41
    sw      a0, DDR4_CKE_MAP_OFFSET(t8)
    dli     a0, 0x41
    sw      a0, DDR4_WRODT_MAP_OFFSET(t8)
    b       9f
    nop

4:
    dli     a0, 0x10
    sw      a0, DDR4_CS_MAP_OFFSET(t8)
    dli     a0, 0x21
    sw      a0, DDR4_CKE_MAP_OFFSET(t8)
    dli     a0, 0x21
    sw      a0, DDR4_WRODT_MAP_OFFSET(t8)
//set rtt_normal 120ohm
    li      a0, 0x7
    dsll    a0, a0, 8
    not     a0, a0
    lh      a1, DDR4_MR1_CS0_REG(t8)
    and     a1, a1, a0
    li      a0, RTT_DISABLE
    dsll    a0, a0, 8
    or      a1, a1, a0
    sh      a1, DDR4_MR1_CS0_REG(t8)

    li      a0, 0x7
    dsll    a0, a0, 8
    not     a0, a0
    lh      a1, DDR4_MR1_CS1_REG(t8)
    and     a1, a1, a0
    li      a0, RTT_DISABLE
    dsll    a0, a0, 8
    or      a1, a1, a0
    sh      a1, DDR4_MR1_CS1_REG(t8)
//set rtt_park 80ohm
    li      a0, 0x7
    dsll    a0, a0, 6
    not     a0, a0
    lh      a1, DDR4_MR5_CS0_REG(t8)
    and     a1, a1, a0
    li      a0, RTT_60
    dsll    a0, a0, 6
    or      a1, a1, a0
    sh      a1, DDR4_MR5_CS0_REG(t8)

    li      a0, 0x7
    dsll    a0, a0, 6
    not     a0, a0
    lh      a1, DDR4_MR5_CS1_REG(t8)
    and     a1, a1, a0
    li      a0, RTT_60
    dsll    a0, a0, 6
    or      a1, a1, a0
    sh      a1, DDR4_MR5_CS1_REG(t8)

    b       9f
    nop

5:
    dli     a0, 0x54
    sw      a0, DDR4_CS_MAP_OFFSET(t8)
    dli     a0, 0x84
    sw      a0, DDR4_CKE_MAP_OFFSET(t8)
    dli     a0, 0x84
    sw      a0, DDR4_WRODT_MAP_OFFSET(t8)
//set rtt_normal 120ohm
    li      a0, 0x7
    dsll    a0, a0, 8
    not     a0, a0
    lh      a1, DDR4_MR1_CS0_REG(t8)
    and     a1, a1, a0
    li      a0, 0x2
    dsll    a0, a0, 8
    or      a1, a1, a0
    sh      a1, DDR4_MR1_CS0_REG(t8)

    li      a0, 0x7
    dsll    a0, a0, 8
    not     a0, a0
    lh      a1, DDR4_MR1_CS1_REG(t8)
    and     a1, a1, a0
    li      a0, 0x2
    dsll    a0, a0, 8
    or      a1, a1, a0
    sh      a1, DDR4_MR1_CS1_REG(t8)
//set rtt_park 80ohm
    li      a0, 0x7
    dsll    a0, a0, 6
    not     a0, a0
    lh      a1, DDR4_MR5_CS0_REG(t8)
    and     a1, a1, a0
    li      a0, 0x6
    dsll    a0, a0, 6
    or      a1, a1, a0
    sh      a1, DDR4_MR5_CS0_REG(t8)

    li      a0, 0x7
    dsll    a0, a0, 6
    not     a0, a0
    lh      a1, DDR4_MR5_CS1_REG(t8)
    and     a1, a1, a0
    li      a0, 0x6
    dsll    a0, a0, 6
    or      a1, a1, a0
    sh      a1, DDR4_MR5_CS1_REG(t8)

    b       9f
    nop

6:
    dli     a0, 0x5410
    sw      a0, DDR4_CS_MAP_OFFSET(t8)
    dli     a0, 0x8421
    sw      a0, DDR4_CKE_MAP_OFFSET(t8)
    dli     a0, 0x8421
    sw      a0, DDR4_WRODT_MAP_OFFSET(t8)
    b       9f
    nop
9:

//set lvl_cs
/*
1 DIMM : lvl_cs use cs0
2 DIMMs: lvl_cs use rank0 of slot1
*/
    GET_MC_CS_MAP_V1
    dli     a0, 0x01
    beq     a0, a1, 1f
    nop
    dli     a0, 0x10
    beq     a0, a1, 1f
    nop
    dli     a0, 0x11
    beq     a0, a1, 2f
    nop
    dli     a0, 0x03
    beq     a0, a1, 1f
    nop
    dli     a0, 0x30
    beq     a0, a1, 1f
    nop
    dli     a0, 0x33
    beq     a0, a1, 3f
    nop
1:
    li      a0, 0x1
    b       4f
    nop
2:
    li      a0, 0x2
    b       4f
    nop
3:
    li      a0, 0x4
    b       4f
    nop
4:
    sb      a0, LVL_CS_OFFSET(t8)
#endif

    lb      t1, 0x0(t8)
    beq     t1, 0x10, ddr_init  //MC controller version 1.0 does not support auto config
    nop

#ifdef AUTO_DDR_CONFIG
set_speed_bin:
//calculate tCKAVGmin from SPD
#ifndef DDR3_DIMM
    GET_SPD(0x12)
    move            t1, v0
    GET_SPD(0x7d)
    move            t2, v0
	dmulou			t1, t1, 125
	dsrl			t4, t2, 7
	and		    	t4, t4, 0x1
	beqz			t4, 1f
	nop
    dli             t3, 0xff
    not             t3
	or		    	t2, t2, t3
1:	daddu			t3, t1, t2
#else
    GET_SPD(0xc)
    move            t1, v0
    bal             get_ddr3_mtb
    nop
	dmulou			t3, t1, v0
#endif
#ifdef  PARAM_SET_FROM_SPD_DEBUG
	PRINTSTR("\r\n tCKmin=")
	move			a0, t3
	bal		    	hexserial
	nop
#endif
#endif
//calculate tCKAVG and store in t0
	dli	    	t0, DDR_FREQ
    dsll        t0, t0, 1
    dli         t1, 1000000
    dmulou      t0, t0, t1
	dli	    	t2, 0xe8d4a51000
	ddivu		t0, t2, t0
#ifdef  PARAM_SET_FROM_SPD_DEBUG
    PRINTSTR("\r\n tCKAVG=")
    move        a0, t0
    bal         hexserial
    nop
#endif

#ifdef AUTO_DDR_CONFIG
//define speed grade by t6
    //report error!
    bgeu        t0, t3, 2f
    nop
8:
    PRINTSTR("\r\nError!!! The DDR speed is not suported for such DRAM or DIMM!!!")
1:
    b           1b
    nop
2:
#endif
#ifdef  DDR3_DIMM
    bgeu        t0, 1874, 8b    #1066MHz
    nop
#endif
    bgeu        t0, 1500, 1f    #1333MHz
    nop
	bgeu		t0, 1250, 2f	#1600|#1333MHz
	nop
	bgeu		t0, 1071, 3f	#1866|#1600MHz
	nop
	bgeu		t0, 937, 4f	    #2133|#1866MHz
	nop
	bgeu		t0, 833, 5f	    #2400|#2133MHz
	nop
	bgeu		t0, 750, 6f	    #2666|#2400MHz
	nop
	bgeu		t0, 682, 7f	    #2933|#2666MHz
	nop
	dli			t6, 3200
	b			10f
	nop
1:	dli			t6, 1333
	b			10f
	nop
2:	dli			t6, 1600
	b			10f
	nop
3:	dli			t6, 1866
	b			10f
	nop
4:	dli			t6, 2133
	b			10f
	nop
5:	dli			t6, 2400
	b			10f
	nop
6:	dli			t6, 2666
	b			10f
	nop
7:	dli			t6, 2933
10:

set_tPL:
#ifndef DDR3_DIMM
#ifdef  PL_ENABLE
	bleu			t6, 2133, 2f
	nop
	bleu			t6, 2666, 3f
	nop
	dli			t1, 6
	b			4f
	nop
2:	dli			t1, 4
	b			4f
	nop
3:	dli			t1, 5
4:
#else
    dli         t1, 0
#endif
#else
    li          t1, 0
#endif
    sb          t1, 0x1068(t8)
#ifdef  PARAM_SET_FROM_SPD_DEBUG
	PRINTSTR("\r\n tPL		=	")
	move			a0, t1
	bal			hexserial
	nop
#endif

#if 1
set_tRL:
/*******************************
for no SPD
________________________________________________________________________
Speed	|1600	|1866	|2133	|2400	|2666	|2933	|3200	|MHz	|
________|_______|_______|_______|_______|_______|_______|_______|_______|
tRL 	|12  	|14  	|16  	|18 	|20 	|20 	|24 	|nCK	|
________|_______|_______|_______|_______|_______|_______|_______|_______|
if DLL_on:
tRL=AL+CL+PL
if DLL_off:
CA Parity mode is not allowed
CL only equals 10
CWL only equals 9
RL=AL+CL-1
ifdef AL0:AL=0
ifdef AL1:AL=CL-1
ifdef AL2:AL=CL-2
*******************************/
#ifndef DDR3_DIMM
#ifdef AUTO_DDR_CONFIG
//get supported CL from SPD
//#ifdef DLL_on
    GET_SPD(0x14)
    move        t1, v0
    GET_SPD(0x15)
    move        t2, v0
    GET_SPD(0x16)
    move        t4, v0
    GET_SPD(0x17)
    move        t5, v0
	dsll	    t5, t5, 24
	or			t1, t1, t5
	dsll		t4, t4, 16
	or			t1, t1, t4
	dsll		t2, t2, 8
	or			t5, t1, t2
//calculate tAAmin to t1
    GET_SPD(0x18)
    move        t1, v0
    GET_SPD(0x7b)
    move        t2, v0
	dmulou		t1, t1, 125
	dsrl		t4, t2, 7
	and			t4, t4, 0x1
	beqz		t4, 1f
	nop
    dli         t3, 0xff
    not         t3
	or			t2, t2, t3
1:
    dadd		t1, t1, t2
    move        a1, t1
    move        a0, t0
	bal	int_rounding
	nop
    move        t1, v0
#ifdef SET_RL_MANUALY
    dli         t1, SET_RL_MANUALY
#endif
#ifndef DISABLE_READ_DBI
    bgtu        t6, 1866, 1f
    nop
    daddu       t1, t1, 2
    b           2f
    nop
1:
    bgtu        t6, 2666, 1f
    nop
    daddu       t1, t1, 3
    b           2f
    nop
1:
    daddu       t1, t1, 4
2:
#endif
//judge the CL range by the 31st bit of t2, "1" means large CL range, "0" means small CL range
	dsrl		t2, t5, 31
	and			t2, t2, 0x1
	beqz		t2, 1f
	nop
//large CL range
	dli			t2, 23
	b			2f
	nop
//small CL range
1:	dli			t2, 7
//get the lowest bit position of "1" in t2
2:	beq			t1, t2, 5f
	nop
	dsubu		t1, t1, t2
	dsrl		t5, t5, t1
	and			t4, t5, 0x1
	beq			t4,  1, 4f
	nop
3:	dsrl		t5, t5, 1
	daddu		t1, t1, 1		#t1 is tempory the number of t4 right shifting
	and			t4, t5, 0x1
	beqz		t4, 3b
	nop
4:	daddu		t1, t1, t2		#CL=t1
//#endif
#else
    bne         t6, 1600, 1f
    nop
    dli         t1, 12
    b           2f
1:
    bne         t6, 1866, 1f
    nop
    dli         t1, 14
    b           2f
1:
    bne         t6, 2133, 1f
    nop
    dli         t1, 16
    b           2f
    nop
1:
    bne         t6, 2400, 1f
    nop
    dli         t1, 18
    b           2f
    nop
1:
    bne         t6, 2666, 1f
    nop
    dli         t1, 20
    b           2f
    nop
1:
    bne         t6, 2933, 1f
    nop
    dli         t1, 22
    b           2f
    nop
1:
    dli         t1, 24
2:
#ifdef SET_RL_MANUALY
    dli         t1, SET_RL_MANUALY
#endif
#ifndef DISABLE_READ_DBI
    bgtu        t6, 1866, 1f
    nop
    daddu       t1, t1, 2
    b           2f
    nop
1:
    bgtu        t6, 2666, 1f
    nop
    daddu       t1, t1, 3
    b           2f
    nop
1:
    daddu       t1, t1, 4
2:
#endif
#endif
#ifdef PL_ENABLE
	lb			t4, 0x1068(t8)		#t4 means PL
#else
	dli			t4, 0
#endif
	dli			t2, 0			#AL=0, t2 means AL, default disable
#ifdef	DLL_off
	dli			t1, 10
	dli			t4, 0
#endif
#ifdef AL_ENABLE
#if	(AL==1)
	dsubu		t2, t1, 1
#elif	(AL==2)
	dsubu		t2, t1, 2
#endif
#endif
#ifdef	DLL_off
	dsubu		t1, t1, 1
#endif
5:	daddu		t1, t1, t4
	daddu		t1, t1, t2
	sb			t1, 0x1060(t8)
//set RL in MRS
    bleu        t1, 16, 1f
    nop
    beq         t1, 17, 2f
    nop
    beq         t1, 19, 3f
    nop
    beq         t1, 21, 4f
    nop
    beq         t1, 23, 5f
    nop
    bleu        t1, 24, 6f
    nop
    dsubu       t1, 25
    daddu       t1, 0x10
    b           7f
    nop
1:
    dsubu       t1, 9
    b           7f
    nop
2:
    dli         t1, 0xd
    b           7f
    nop
3:
    dli         t1, 0xe
    b           7f
    nop
4:
    dli         t1, 0xf
    b           7f
    nop
5:
    dli         t1, 0xc
    b           7f
    nop
6:
    dsubu       t1, 18
    dsrl        t1, 1
    daddu       t1, 8
7:
    andi        t2, t1, 0x1
    dsll        t2, 2
    andi        t4, t1, (0x7<<1)
    dsll        t4, 3
    or          t2, t4
    andi        t4, t1, (0x1<<4)
    dsll        t4, 8
    or          t2, t4
    dli         t4, (0x1<<2|0x7<<4|0x1<<12)
    not         t4
    lhu         t1, 0x1180(t8)
    and         t1, t4
    or          t1, t2
    sh          t1, 0x1180(t8)
    sh          t1, 0x1190(t8)
    sh          t1, 0x11a0(t8)
    sh          t1, 0x11b0(t8)
    sh          t1, 0x11c0(t8)
    sh          t1, 0x11d0(t8)
    sh          t1, 0x11e0(t8)
    sh          t1, 0x11f0(t8)
#else
    GET_SPD(0xe)
    move    t1, v0
    GET_SPD(0xf)
    dsll    v0, v0, 8
    or      t1, t1, v0
    GET_SPD(0x10)
    move    t2, v0
    bal     get_ddr3_mtb
    nop
    dmulou  t2, v0, t2
    move    a0, t0
    move    a1, t2
	bal	    int_rounding
	nop
    dsubu   v0, v0, 4
1:
    dsrl    t2, t1, v0
    andi    t2, t2, 0x1
    bnez    t2, 1f
    nop
    daddu   v0, v0 ,1
    b       1b
    nop
1:
    daddu   v0, v0, 4
    sb      v0, 0x1060(t8)
    bleu    v0, 11, 1f
    nop
    dsubu   v0, v0, 12
    dsll    v0, v0, 1
    daddu   v0, v0, 1
    b       2f
    nop
1:
    dsubu   v0, 4
    dsll    v0, v0, 1
2:
    andi    t1, v0, 0x1
    dsll    t1, 2
    andi    t2, v0, (0x7<<1)
    dsll    t2, 3
    or      t1, t2
    dli     t2, (0x1<<2|0x7<<4)
    not     t2
    lhu     v0, 0x1140(t8)
    and     v0, t2
    or      v0, t1
    sh      v0, 0x1140(t8)
    sh      v0, 0x1148(t8)
    sh      v0, 0x1150(t8)
    sh      v0, 0x1158(t8)
    sh      v0, 0x1160(t8)
    sh      v0, 0x1168(t8)
    sh      v0, 0x1170(t8)
    sh      v0, 0x1178(t8)
#endif
#ifdef  PARAM_SET_FROM_SPD_DEBUG
	PRINTSTR("\r\n tRL		=	")
    lb          a0, 0x1060(t8)
	bal			hexserial
	nop
#endif
#endif

#ifdef SET_BY_PROTOCOL
set_tRESET:
	dli		t1, 200000000
	dsll		t0, t0, 12

    move        a1, t1
    move        a0, t0
	bal	int_rounding
	nop
    move        t1, v0
	dsrl		t0, t0, 12
	sb		t1, 0x1000(t8)
#ifdef  PARAM_SET_FROM_SPD_DEBUG
	PRINTSTR("\r\n tRESET		=	")
	move		a0, t1
	bal		hexserial
	nop
#endif

set_tCKE:
	dli		t1, 500000000
	dsll		t0, t0, 12
    move        a1, t1
    move        a0, t0
	bal	int_rounding
	nop
    move        t1, v0
	dsrl		t0, t0, 12
	sb		t1, 0x1001(t8)
#ifdef  PARAM_SET_FROM_SPD_DEBUG
	PRINTSTR("\r\n tCKE		=	")
	move		a0, t1
	bal		hexserial
	nop
#endif
#endif

set_tRFC_auto:
/************************
tRFC
________________________________________________________________________________
|Refresh Mode   |2Gb(0011)      |4Gb(0100)      |8Gb(0101)      |16Gb(0110)|   |
|_______________|_______________|_______________|_______________|__________|___|
|x1             |160            |260            |350            |550	   |ns |
|_______________|_______________|_______________|_______________|__________|___|
|x2             |110            |160            |260            |350       |ns |
|_______________|_______________|_______________|_______________|__________|___|
|x4             |90             |110            |160            |260       |ns |
|_______________|_______________|_______________|_______________|__________|___|
size of SDRAM=2^(BG+(2+BA)+(18-ROW)+(12-COL)+(2+SDRAM_WIDTH))=2^(34+BG+BA+SDRAM_WIDTH-ROW-COL) the parameter comes from s1
************************/
#ifndef DDR3_DIMM
#ifdef AUTO_DDR_CONFIG
    GET_SPD(0x4)
    move    t2, v0
    and     t2, t2, 0xf
#ifdef	        REFRESH_1X
    GET_SPD(0x1f)
#elif defined   REFRESH_2X
    GET_SPD(0x21)
#else
    GET_SPD(0x23)
#endif
    move    t1, v0
#ifdef	        REFRESH_1X
    GET_SPD(0x1e)
#elif defined   REFRESH_2X
    GET_SPD(0X20)
#else
    GET_SPD(0x22)
#endif
    move    t4, v0
	dsll	t1, t1, 8
	or	t1, t1, t4
	dmulou	t1, t1, 125
#endif
#ifndef  SPD_ONLY
    GET_BG_NUM_V1
    daddu   t2, a1, 34
    GET_BA_NUM_V1
    daddu   t2, t2, a1
    GET_SDRAM_WIDTH_V1
    daddu   t2, t2, a1
    GET_ROW_SIZE_V1
    dsubu   t2, t2, a1
    GET_COL_SIZE_V1
    dsubu   t2, t2, a1
    bne     t2, 31, 1f
    nop
    dli     t2, 0x3
    b       2f
    nop
1:
    bne     t2, 32, 1f
    nop
    dli     t2, 0x4
    b       2f
    nop
1:
    bne     t2, 33, 1f
    nop
    dli     t2, 0x5
    b       2f
    nop
1:
    dli     t2, 0x6
2:
	beq	t2, 0x3, 1f
	nop
	beq	t2, 0x4, 2f
	nop
	beq	t2, 0x5, 3f
	nop
	beq	t2, 0x6, 4f
	nop
	PRINTSTR("\r\nError: memsize wrong \r\n")
	b 		6f
	nop
1:
#ifdef	        REFRESH_1X
	dli	t2, 160000
#elif defined   REFRESH_2X
	dli	t2, 110000
#else
	dli	t2, 90000
#endif
	b	5f
	nop
2:
#ifdef	        REFRESH_1X
	dli	t2, 260000
#elif defined   REFRESH_2X
	dli	t2, 160000
#else
	dli	t2, 110000
#endif
	b	5f
	nop
3:
#ifdef	        REFRESH_1X
	dli	t2, 350000
#elif defined   REFRESH_2X
	dli	t2, 260000
#else
	dli	t2, 160000
#endif
	b	5f
	nop
4:
#ifdef	        REFRESH_1X
	dli	t2, 550000
#elif defined   REFRESH_2X
	dli	t2, 350000
#else
	dli	t2, 260000
#endif
5:
#ifdef AUTO_DDR_CONFIG
	bgeu	t1, t2, 6f
	nop
	move	t1, t2
#else
    move    t1, t2
#endif
6:
#endif
#else
    GET_SPD(0x19)
    move    t1, v0
    GET_SPD(0x18)
    dsll    t1, t1, 8
    or      t1, t1, v0
    bal     get_ddr3_mtb
    nop
    dmulou  t1, t1, v0
#endif
    move        a1, t1
    move        a0, t0
    bal	int_rounding
	nop
    move        t1, v0
	sh	t1, 0x1012(t8)
#ifdef  PARAM_SET_FROM_SPD_DEBUG
	PRINTSTR("\r\n tRFC		=	")
	move	a0, t1
	bal	hexserial
	nop
#endif

/*************************
tXPR=max(5nCK,tRFC+10ns)          =>    DDR4 Spec
tRFC=[Byte 0x1f,Byte 0x1e]	    =>	  SPD
*************************/

#ifdef SET_BY_PROTOCOL
set_tXPR:
	lhu		    t1, 0x1012(t8)
    dmulou      t1, t1, t0
	daddu		t1, t1, 10000
	dmulou		t2, t0, 5
	bltu		t2, t1, 1f
	nop
	dli	    	t1, 5
	b	    	2f
	nop

1:
    move        a1, t1
    move        a0, t0
    bal	int_rounding
	nop
    move        t1, v0
2:
	sh		t1, 0x1002(t8)

#ifdef  PARAM_SET_FROM_SPD_DEBUG
	PRINTSTR("\r\n tXPR		=	")
	move		a0, t1
	bal		hexserial
	nop
#endif


/*************************
tMOD=max(24nCK,15ns)          =>    DDR4 Spec
*************************/


set_tMOD:
#ifndef DDR3_DIMM
	dmulou	t1, t0, 24
#else
	dmulou	t1, t0, 12
#endif
	dli		t2, 15000
	bltu	t1, t2, 1f
	nop
	dli		t1, 24
	b		2f
	nop

1:	dli		t1, 15000
    move        a1, t1
    move        a0, t0
	bal	int_rounding
	nop
    move        t1, v0
2:
	sb		t1, 0x1004(t8)

#ifdef  PARAM_SET_FROM_SPD_DEBUG
	PRINTSTR("\r\n tMOD		=	")
	move		a0, t1
	bal		hexserial
	nop
#endif
#endif

set_tWL:						#use set1
#ifndef DDR3_DIMM
//#ifdef	PREAMBLE1
	bleu	t6, 1600, 1f
	nop
	bleu	t6, 1866, 2f
	nop
	bleu	t6, 2133, 3f
	nop
	bleu	t6, 2400, 4f
	nop
	bleu	t6, 2666, 5f
	nop
	dli	t1, 16
	b	10f
	nop
1:	dli	t1, 9
	b	10f
	nop
2:	dli	t1, 10
	b	10f
	nop
3:	dli	t1, 11
	b	10f
	nop
4:	dli	t1, 12
	b	10f
	nop
5:	dli	t1, 14
//#endif
#ifdef	PREAMBLE2
    bleu    t6, 2400, 1f
    nop
    bleu    t6, 2666, 2f
    nop
	dli	t1, 18
	b	10f
	nop
1:	dli	t1, 14
	b	10f
	nop
2:	dli	t1, 16
	b	10f
	nop
#endif
10:
	sb	t1, 0x1064(t8)
//set CWL in MRS
    bleu    t1, 11, 1f
    nop
    dsubu   t1, 12
    dsrl    t1, 1
    daddu   t1, 3
    b       2f
    nop
1:
    dsubu   t1, 9
2:
    dsll    t1, 3
    dli     t2, (0x7<<3)
    not     t2
    lh      t4, 0x1184(t8)
    and     t4, t2
    or      t4, t1
    sh      t4, 0x1184(t8)
    sh      t4, 0x1194(t8)
    sh      t4, 0x11a4(t8)
    sh      t4, 0x11b4(t8)
    sh      t4, 0x11c4(t8)
    sh      t4, 0x11d4(t8)
    sh      t4, 0x11e4(t8)
    sh      t4, 0x11f4(t8)
#else
#if 0
    bgtu    t6, 1333, 1f
    nop
    li      t1, 7
    b       2f
    nop
1:
    bgtu    t6, 1600, 1f
    nop
    li      t1, 8
    b       2f
    nop
1:
    PRINTSTR("\r\nError: DDR freq is not supported!")
1:
    b       1b
    nop
2:
#endif
    li      t1, 0x9
    sb      t1, 0x1064(t8)
    dsubu   t1, t1, 5
    dsll    t1, t1, 3
    lh      t2, 0x1144(t8)
    li      v0, (0x7<<3)
    not     v0, v0
    and     t2, t2, v0
    or      t2, t2, t1
    sh      t2, 0x1144(t8)
    sh      t2, 0x114c(t8)
    sh      t2, 0x1154(t8)
    sh      t2, 0x115c(t8)
    sh      t2, 0x1164(t8)
    sh      t2, 0x116c(t8)
    sh      t2, 0x1174(t8)
    sh      t2, 0x117c(t8)
#endif
#ifdef  PARAM_SET_FROM_SPD_DEBUG
	PRINTSTR("\r\n tWL		=	")
    lb          a0, 0x1064(t8)
	bal			hexserial
	nop
#endif


/************************
if ODT on:
	tWLDQSEN>tMOD+ODTLon+tADC	(DLL Enable)
	tWLDQSEN>tMOD+tAONAS		(DLL Disable)

tMOD=max(24nCK,15ns)
tAONAS (1ns,9ns)
tADC (0.3tCK,0.7tCK)
ODTLon:
if preamble==1	ODTLon=WL-2
if preamble==2	ODTLon=WL-3
************************/

#ifdef SET_BY_PROTOCOL
set_tWLDQSEN:
#ifndef DDR3_DIMM
    lb      t1, 0x1004(t8)  //tMOD
    lb      t2, 0x1064(t8)
    sub     t2, t2, 2   //tDODTLon=WL-2
    add     t1, t2
    add     t1, 1
#else
    li      t1, 25
#endif
	sb		t1, 0x1005(t8)	#tWLDQSEN varies depending on DLL Mode or ODT

#ifdef  PARAM_SET_FROM_SPD_DEBUG
	PRINTSTR("\r\n tWLDQSEN	=	")
	move		a0, t1
	bal		hexserial
	nop
#endif
#endif

/***************************
tRP=(Byte 0x1a)*125ps+(Byte 0x79)*1ps   => SPD
***************************/
set_tRP:
#ifndef DDR3_DIMM
#ifdef AUTO_DDR_CONFIG
    GET_SPD(0x1a)
    move    t1, v0
    GET_SPD(0x79)
    move    t2, v0
	dmulou		t1, t1, 125
	dsrl			t4, t2, 7
	and			t4, t4, 0x1
	beqz			t4, 1f
	nop
    dli         t3, 0xff
    not         t3
	or			t2, t2, t3
1:
    dadd		t1, t1, t2
    move        a1, t1
    move        a0, t0
	bal	int_rounding
	nop
    move        t1, v0
#else
    lb          t1, 0x1060(t8)
#endif
#else
    GET_SPD(0x14)
    move    t1, v0
    bal     get_ddr3_mtb
    nop
    dmulou  a1, t1, v0
    move    a0, t0
	bal	    int_rounding
	nop
    move        t1, v0
#endif
	sb		t1, 0x1006(t8)
#ifdef  PARAM_SET_FROM_SPD_DEBUG
	PRINTSTR("\r\n tRP		=	")
	move		a0, t1
	bal		hexserial
	nop
#endif

#ifdef SET_BY_PROTOCOL
set_tREF:
#ifdef	            REFRESH_1X
	dli		t1, 7800000
#elif defined		REFRESH_2X
	dli		t1, 3900000
#else
	dli		t1, 1950000
#endif
    move        a1, t1
    move        a0, t0
	bal	int_rounding
	nop
    move        t1, v0
	dli		t2, 16			#tREF unit=16nCK
	ddivu		t1, t1, t2
	mfhi		t2
	beqz		t2, 1f
	nop
	daddu		t1, t1, 1
1:
	sh		t1, 0x1010(t8)
#ifdef  PARAM_SET_FROM_SPD_DEBUG
	PRINTSTR("\r\n tREF		=	")
	move 		a0, t1
	bal		hexserial
	nop
#endif


set_tREFretention:				#tREFretention=32ms/tCKAVG
	dli	t1, 32000000000
	ddivu	t1, t1, t0
	nop
	mfhi		t2
	beqz		t2, 1f
	nop
	daddu		t1, t1, 1
1:
	sw	t1, 0x1014(t8)
#ifdef  PARAM_SET_FROM_SPD_DEBUG
	PRINTSTR("\r\n tREFretention	=	")
	move	a0, t1
	bal	hexserial
	nop
#endif
#endif

set_tRFC_dlr:
/************************
tRFC_dlr
________________________________________________________________________________
|Refresh Mode	|2Gb(0011)      |4Gb(0100)      |8Gb(0101)      |16Gb(0110)|   |
|_______________|_______________|_______________|_______________|__________|___|
|x1             |--             |90             |120            |TBD       |ns |
|_______________|_______________|_______________|_______________|__________|___|
|x2             |--             |55             |90             |TBD       |ns |
|_______________|_______________|_______________|_______________|__________|___|
|x4             |--             |40             |55             |TBD       |ns |
|_______________|_______________|_______________|_______________|__________|___|
************************/
#ifdef AUTO_DDR_CONFIG
//judge 3DS
    GET_SPD(0x6)
    move        t1, v0
    and         t1, t1, 0x3
	beq			t1, 2, 1f
	nop
    dli     t1, 0
    b       7f
    nop
1:
    GET_SPD(0x4)
    move    t2, v0
    and     t2, t2, 0xf
	beq	t2, 0x4, 2f
	nop
	beq	t2, 0x5, 3f
	nop
	PRINTSTR("\r\n memsize wrong \r\n")
	b 		6f
	nop
#else
    dli     t1, 0
    b       7f
    nop
#endif
2:
#ifdef	        REFRESH_1X
	dli	t1, 90000
#elif defined	REFRESH_2X
	dli	t1, 55000
#else
	dli	t1, 40000
#endif
	b	6f
	nop
3:
#ifdef	        REFRESH_1X
    dli	t1, 120000
#elif defined	REFRESH_2X
	dli	t1, 90000
#else
	dli	t1, 55000
#endif
6:
    move        a1, t1
    move        a0, t0
    bal	int_rounding
	nop
    move        t1, v0
7:
	sh	t1, 0x101a(t8)
#ifdef  PARAM_SET_FROM_SPD_DEBUG
	PRINTSTR("\r\n tRFC_dlr	=	")
	move	a0, t1
	bal	hexserial
	nop
#endif

#ifdef SET_BY_PROTOCOL
set_tXS:			#tXS=tRFC+10ns
	lhu	t1, 0x1012(t8)
    dmulou  t1, t1, t0
	daddu	t1, t1, 10000
    move        a1, t1
    move        a0, t0
	bal	int_rounding
	nop
#ifdef DDR3_DIMM
    bgeu    v0, 5, 1f
    nop
    li      v0, 5
1:
#endif
    move        t1, v0
	sh	t1, 0x101c(t8)
#ifdef  PARAM_SET_FROM_SPD_DEBUG
	PRINTSTR("\r\n tXS		=	")
	move	a0, t1
	bal	hexserial
	nop
#endif

set_tXSRD:
/************************
tXSRD=tXSDLL=tDLLK
________________________________________________________________________
Speed	|1600	|1866	|2133	|2400	|2666	|2933	|3200	|MHz	|
________|_______|_______|_______|_______|_______|_______|_______|_______|
tDLLK	|597	|597	|768	|768	|854	|940	|1024	|nCK	|
________|_______|_______|_______|_______|_______|_______|_______|_______|
************************/
#ifndef DDR3_DIMM
//choose speed bin
	bleu			t6, 1866, 1f
	nop
	bleu			t6, 2400, 2f
	nop
	beq			t6,  2666, 3f
	nop
	beq			t6,  2933, 4f
	nop
	dli			t1, 1024
	b			5f
	nop
1:	dli			t1, 597
	b			5f
	nop
2:	dli			t1, 768
	b			5f
	nop
3:	dli			t1, 854
	b			5f
	nop
4:	dli			t1, 940
#else
    dli         t1, 512
#endif
5:	ddivu			t1, t1, 256
	mfhi			t2
	beqz			t2, 6f
	nop
	daddu			t1, t1, 1
6:
	sb			t1, 0x101e(t8)
#ifdef  PARAM_SET_FROM_SPD_DEBUG
	PRINTSTR("\r\n tXSRD		=	")
	move			a0, t1
	bal			hexserial
	nop
#endif

set_tCKESR:					#tCKESR=tCKE+1nCK	tCKE=max(3nCK, 5ns)
//calculate tCKE
	dmulou			t1, t0, 3
#ifndef DDR3_DIMM
	dli			t2, 5000
#else
    dli         t2, 5625
#endif
	bgeu			t1, t2, 1f
	nop
    move            t1, t2
1:	daddu			t1, t1, t0
    move        a1, t1
    move        a0, t0
	bal	int_rounding
	nop
    move        t1, v0
	sb			t1, 0x101f(t8)
#ifdef  PARAM_SET_FROM_SPD_DEBUG
	PRINTSTR("\r\n tCKESR		=	")
	move			a0, t1
	bal			hexserial
	nop
#endif

set_tXP:					#tXP=max(4nCK,6ns)
	dli			t2, 6000
    dsll            t1, t0, 2
	bgeu			t1, t2, 1f
	nop
	move			t1, t2
1:
    move        a1, t1
    move        a0, t0
    bal	int_rounding
	nop
    move        t1, v0
	sb			t1, 0x1020(t8)
#ifdef  PARAM_SET_FROM_SPD_DEBUG
	PRINTSTR("\r\n tXP		=	")
	move			a0, t1
	bal			hexserial
	nop
#endif

set_tCPDED:
#ifndef DDR3_DIMM
	dli			t1, 4
#else
    dli         t1, 1
#endif
	sb			t1, 0x1022(t8)
#ifdef  PARAM_SET_FROM_SPD_DEBUG
	PRINTSTR("\r\n tCPDED		=	")
	move			a0, t1
	bal			hexserial
	nop
#endif

set_tRDPDEN:					#tRDPDEN=RL+4+1
	lb			t1, 0x1060(t8)
	daddu			t1, t1, 5
	sb			t1, 0x1023(t8)
#ifdef  PARAM_SET_FROM_SPD_DEBUG
	PRINTSTR("\r\n tRDPDEN	=	")
	move			a0, t1
	bal			hexserial
	nop
#endif

set_tZQCS:
#ifndef DDR3_DIMM
	dli			t1, 128
#else
    dli         a1, 80000
    move        a0, t0
    bal	int_rounding
	nop
    move        t1, v0
    bgeu        t1, 64, 1f
    nop
    li          t1, 64
1:
#endif
	sb			t1, 0x1029(t8)
#ifdef  PARAM_SET_FROM_SPD_DEBUG
	PRINTSTR("\r\n tZQCS		=	")
	move			a0, t1
	bal			hexserial
	nop
#endif

set_tZQCL:
	dli			t1, 2
	sb			t1, 0x102a(t8)
#ifdef  PARAM_SET_FROM_SPD_DEBUG
	PRINTSTR("\r\n tZQCL		=	")
	move			a0, t1
	bal			hexserial
	nop
#endif

set_tZQperiod:
	dli			t1, 128000000000
	dsll			t0, t0, 20
    move        a1, t1
    move        a0, t0
	bal	int_rounding
	nop
    move        t1, v0
	dsrl			t0, t0, 20
	sb			t1, 0x102b(t8)
#ifdef  PARAM_SET_FROM_SPD_DEBUG
	PRINTSTR("\r\n tZQperiod	=	")
	move			a0, t1
	bal			hexserial
	nop
#endif
#endif


set_tRASmin:
/************************
if speed==1600MHz tRASmin=35ns
if speed==1866MHz tRASmin=34ns
if speed==2133MHz tRASmin=33ns
if speed>2133MHz  tRASmin=32ns
min:max(4nCK,tRAS)
************************/
#ifndef DDR3_DIMM
#ifdef AUTO_DDR_CONFIG
    GET_SPD(0x1b)
    move        t2, v0
	dsrl		t2, t2, 4
	and			t2, t2, 0xf
    GET_SPD(0x1c)
    move        t4, v0
	dsll			t2, t2, 8
	or			t2, t2, t4
	dmulou			a1, t2, 125
#else
    bgtu        t6, 1600, 1f
    nop
    dli         a1, 35000
    b           2f
    nop
1:
    bne         t6, 1866, 1f
    nop
    dli         a1, 34000
    b           2f
    nop
1:
    bne         t6, 2133, 1f
    nop
    dli         a1, 33000
    b           2f
    nop
1:
    dli         a1, 32000
2:
#endif
#else
    GET_SPD(0x15)
    andi        t1, v0, 0xf
    dsll        t1, t1, 8
    GET_SPD(0x16)
    andi        v0, v0, 0xff
    or          t1, t1, v0
    bal         get_ddr3_mtb
    nop
    dmulou      a1, t1, v0
#endif
    move        a0, t0
    bal	int_rounding
	nop
    move        t1, v0
	nop
	sb			t1, 0x1040(t8)
#ifdef  PARAM_SET_FROM_SPD_DEBUG
	PRINTSTR("\r\n tRASmin	=	")
	move			a0, t1
	bal			hexserial
	nop
#endif

set_tRRD_dlr:					#if 3DS tRRD_dlr=4nCK, else it is tRRD_S
/************************
tRRD_S define
__________________________________________________________________________________________________________________________________________________
page_size| 1600(1.25ns)	 |  1866(1.071ns)  |   2133(0.938ns)  | 2400(0.833ns)   |     2666(0.75ns)    |     2933(0.682ns)     |   3200(0.625ns)  |
_________|_______________|_________________|__________________|_________________|_____________________|_______________________|__________________|
2K       |max(4nCK,6ns)  |max(4nCK,5.3ns)  |<--               |<--              |<--                  |<--                    |<--               |
_________|_______________|_________________|__________________|_________________|_____________________|_______________________|__________________|
1K       |max(4nCK,5ns)  |max(4nCK,4.2ns)  |max(4nCK,3.7ns)   |max(4nCK,3.3ns)  |max(4nCK,3ns)        |max(4nCK,2.7ns)        |max(4nCK,2.5ns)   |
_________|_______________|_________________|__________________|_________________|_____________________|_______________________|__________________|
0.5K	 |max(4nCK,5ns)  |max(4nCK,4.2ns)  |max(4nCK,3.7ns)   |max(4nCK,3.3ns)  |max(4nCK,3ns)	      |max(4nck,2.7ns)	      |max(4nCK,2.5ns)   |
_________|_______________|_________________|__________________|_________________|_____________________|_______________________|__________________|
page_size=2^{(Byte 0x5 bit 2:0)+9+(Byte 0xc bit 2:0)+2-3}
************************/
#ifndef DDR3_DIMM
#ifdef AUTO_DDR_CONFIG
//judge 3DS
    GET_SPD(0x6)
    move        t1, v0
    and         t1, t1, 0x3
	beq			t1, 2, 12f
	nop

//get tRRD_S from SPD: t1
    GET_SPD(0x26)
    move        t1, v0
    GET_SPD(0x77)
    move        t2, v0
	dmulou			t1, t1, 125
	dsrl			t4, t2, 7
	and			t4, t4, 0x1
	beqz			t4, 1f
	nop
    dli         t3, 0xff
    not         t3
	or			t2, t2, t3
1:
    dadd		t1, t1, t2
#endif
#ifndef SPD_ONLY
/*********************************************
calculate the power number of page size: t2
t2= 9 :page sizeis 0.5K
t2=10 :page size is   1K
t2=11 :page size is   2K
*********************************************/
    GET_SDRAM_WIDTH_V1
    daddu       t2, a1, 11
    GET_COL_SIZE_V1
    dsubu       t2, t2, a1
//choose speed bin
	bleu		t6, 1600, 3f
	nop
	beq			t6, 1866, 4f
	nop
	beq			t6, 2133, 5f
	nop
	beq			t6, 2400, 6f
	nop
	beq			t6, 2666, 7f
	nop
	beq			t6, 2933, 8f
	nop
	beq			t2, 11, 21f
	nop
	bltu		t2, 11, 22f
	nop
	PRINTSTR("\r\n pagesize wrong \r\n")
	b 		    10f
	nop
21:	dli			t2, 5300
	b			9f
	nop
22:	dli			t2, 2500
	b			9f
	nop
3:
	beq			t2, 11, 31f
	nop
	bltu		t2, 11, 32f
	nop
	PRINTSTR("\r\n pagesize wrong \r\n")
	b   		10f
	nop
31:	dli			t2, 6000
	b			9f
	nop
32:	dli			t2, 5000
	b			9f
	nop
4:
	beq			t2, 11, 41f
	nop
	bltu		t2, 11, 42f
	nop
	PRINTSTR("\r\n pagesize wrong \r\n")
	b   		10f
	nop
41:	dli			t2, 5300
	b			9f
	nop
42:	dli			t2, 4200
	b			9f
	nop
5:
	beq			t2, 11, 51f
	nop
	bltu		t2, 11, 52f
	nop
	PRINTSTR("\r\n pagesize wrong \r\n")
	b 		    10f
	nop
51:	dli			t2, 5300
	b			9f
	nop
52:	dli			t2, 3700
	b			9f
	nop
6:
	beq			t2, 11, 61f
	nop
	bltu		t2, 11, 62f
	nop
	PRINTSTR("\r\n pagesize wrong \r\n")
	b 		    10f
	nop
61:	dli			t2, 5300
	b			9f
	nop
62:	dli			t2, 3300
	b			9f
	nop
7:
	beq			t2, 11, 71f
	nop
	bltu		t2, 11, 72f
	nop
	PRINTSTR("\r\n pagesize wrong \r\n")
	b 	    	10f
	nop
71:	dli			t2, 5300
	b			9f
	nop
72:	dli			t2, 3000
	b			9f
	nop
8:
	beq			t2, 11, 81f
	nop
	bltu		t2, 11, 82f
	nop
	PRINTSTR("\r\n pagesize wrong \r\n")
	b 		    10f
	nop
81:	dli			t2, 5300
	b			9f
	nop
82:	dli			t2, 2700
9:
#ifdef AUTO_DDR_CONFIG
#ifndef SPD_ONLY
//max(tRRD_S_SPD, tRRD_S_Spec)
    bgeu		t1, t2, 10f
	nop
	move		t1, t2
#endif
#else
    move        t1, t2
#endif
10:
#endif
    move        a1, t1
    move        a0, t0
    bal	int_rounding
	nop
    move        t1, v0
//max(tRRD_S, 4nCK)
	dli			t2, 4
	bgeu		t1, t2, 11f
	nop
	move		t1, t2
11:
#ifdef  PARAM_SET_FROM_SPD_DEBUG
	PRINTSTR("\r\n tRRD_S		=	")
#endif
	b			13f
	nop
12:
    dli			t1, 4
#ifdef  PARAM_SET_FROM_SPD_DEBUG
	PRINTSTR("\r\n tRRD_dlr	=	")
#endif
13:
#else
    GET_SPD(0x13)
    move        t1, v0
    bal         get_ddr3_mtb
    nop
    dmulou      a1, t1, v0
    move        a0, t0
    bal	int_rounding
	nop
    move        t1, v0
#ifdef  PARAM_SET_FROM_SPD_DEBUG
	PRINTSTR("\r\n tRRD    	=	")
#endif
#endif
	sb			t1, 0x1044(t8)
#ifdef  PARAM_SET_FROM_SPD_DEBUG
    move		a0, t1
	bal			hexserial
	nop
#endif

set_tRRD_S_slr:// now it is for monolithic
/************************
tRRD_S define
__________________________________________________________________________________________________________________________________________________
page_size| 1600(1.25ns)	 |  1866(1.071ns)  |   2133(0.938ns)  | 2400(0.833ns)   |     2666(0.75ns)    |     2933(0.682ns)     |   3200(0.625ns)  |
_________|_______________|_________________|__________________|_________________|_____________________|_______________________|__________________|
2K       |max(4nCK,6ns)  |max(4nCK,5.3ns)  |<--               |<--              |<--                  |<--                    |<--               |
_________|_______________|_________________|__________________|_________________|_____________________|_______________________|__________________|
1K       |max(4nCK,5ns)  |max(4nCK,4.2ns)  |max(4nCK,3.7ns)   |max(4nCK,3.3ns)  |max(4nCK,3ns)        |max(4nCK,2.7ns)        |max(4nCK,2.5ns)   |
_________|_______________|_________________|__________________|_________________|_____________________|_______________________|__________________|
0.5K	 |max(4nCK,5ns)  |max(4nCK,4.2ns)  |max(4nCK,3.7ns)   |max(4nCK,3.3ns)  |max(4nCK,3ns)	      |max(4nck,2.7ns)	      |max(4nCK,2.5ns)   |
_________|_______________|_________________|__________________|_________________|_____________________|_______________________|__________________|
page_size=2^{(Byte 0x5 bit 2:0)+9+(Byte 0xc bit 2:0)+2-3}
************************/
#ifdef AUTO_DDR_CONFIG
    GET_SPD(0x26)
    move        t1, v0
    GET_SPD(0x77)
    move        t2, v0
//get tRRD_S from SPD: t1
	dmulou		t1, t1, 125
	dsrl		t4, t2, 7
	and			t4, t4, 0x1
	beqz		t4, 1f
	nop
    dli         t3, 0xff
    not         t3
	or			t2, t2, t3
1:
    dadd		t1, t1, t2
#endif
#ifndef SPD_ONLY
//calculate the power number of page size: t2
    GET_SDRAM_WIDTH_V1
    daddu       t2, a1, 11
    GET_COL_SIZE_V1
    dsubu       t2, t2, a1
//choose speed bin
	bleu		t6, 1600, 3f
	nop
	beq			t6, 1866, 4f
	nop
	beq			t6, 2133, 5f
	nop
	beq			t6, 2400, 6f
	nop
	beq			t6, 2666, 7f
	nop
	beq			t6, 2933, 8f
	nop
	dli			t5, 11
	beq			t2, t5, 21f
	nop
	bltu			t2, t5, 22f
	nop
	PRINTSTR("\r\n pagesize wrong \r\n")
	b 		10f
	nop

21:	dli			t2, 5300
	b			9f
	nop
22:	dli			t2, 2500
	b			9f
	nop

3:	dli			t5, 11
	beq			t2, t5, 31f
	nop
	bltu			t2, t5, 32f
	nop
	PRINTSTR("\r\n pagesize wrong \r\n")
	b 		10f
	nop

31:	dli			t2, 6000
	b			9f
	nop
32:	dli			t2, 5000
	b			9f
	nop
4:	dli			t5, 11
	beq			t2, t5, 41f
	nop
	bltu			t2, t5, 42f
	nop
	PRINTSTR("\r\n pagesize wrong \r\n")
	b 		10f
	nop
41:	dli			t2, 5300
	b			9f
	nop
42:	dli			t2, 4200
	b			9f
	nop
5:	dli			t5, 11
	beq			t2, t5, 51f
	nop
	bltu			t2, t5, 52f
	nop
	PRINTSTR("\r\n pagesize wrong \r\n")
	b 		10f
	nop
51:	dli			t2, 5300
	b			9f
	nop
52:	dli			t2, 3700
	b			9f
	nop
6:	dli			t5, 11
	beq			t2, t5, 61f
	nop
	bltu			t2, t5, 62f
	nop
	PRINTSTR("\r\n pagesize wrong \r\n")
	b 		10f
	nop
61:	dli			t2, 5300
	b			9f
	nop
62:	dli			t2, 3300
	b			9f
	nop
7:	dli			t5, 11
	beq			t2, t5, 71f
	nop
	bltu			t2, t5, 72f
	nop
	PRINTSTR("\r\n pagesize wrong \r\n")
	b 		10f
	nop
71:	dli			t2, 5300
	b			9f
	nop
72:	dli			t2, 3000
	b			9f
	nop
8:	dli			t5, 11
	beq			t2, t5, 81f
	nop
	bltu			t2, t5, 82f
	nop
	PRINTSTR("\r\n pagesize wrong \r\n")
	b 		10f
	nop
81:	dli			t2, 5300
	b			9f
	nop
82:	dli			t2, 2700
9:
#endif

#ifdef AUTO_DDR_CONFIG
#ifndef SPD_ONLY
//max(tRRD_S_SPD, tRRD_S_Spec)
    bgeu			t1, t2, 10f
	nop
	move			t1, t2
10:
#endif
#else
    move        t1, t2
#endif
    move        a1, t1
    move        a0, t0
    bal	int_rounding
	nop
    move        t1, v0
	dli			t2, 4
//max(tRRD_S, 4nCK)
	bgeu			t1, t2, 11f
	nop
	move			t1, t2
11:
	sb			t1, 0x1046(t8)
#ifdef  PARAM_SET_FROM_SPD_DEBUG
	PRINTSTR("\r\n tRRD_S_slr	=	")
	move			a0, t1
	bal			hexserial
	nop
#endif

set_RRD_L_slr: //now only for monolithic
/************************
tRRD_L define
__________________________________________________________________________________________________________________________________________________
page_size| 1600(1.25ns)	 |  1866(1.071ns)  |   2133(0.937ns)  | 2400(0.833ns)   |     2666(0.75ns)    |     2933(0.682ns)     |   3200(0.625ns)  |
_________|_______________|_________________|__________________|_________________|_____________________|_______________________|__________________|
2K       |max(4nCK,7.5ns)|max(4nCK,6.4ns)  |<--               |<--              |<--                  |<--                    |<--               |
_________|_______________|_________________|__________________|_________________|_____________________|_______________________|__________________|
1K       |max(4nCK,6ns)  |max(4nCK,5.3ns)  |<--               |max(4nCK,4.9ns)  |<--                  |<--                    |<--               |
_________|_______________|_________________|__________________|_________________|_____________________|_______________________|__________________|
0.5K     |max(4nCK,6ns)  |max(4nCK,5.3ns)  |<--               |max(4nCK,4.9ns)  |<--                  |<--                    |<--               |
_________|_______________|_________________|__________________|_________________|_____________________|_______________________|__________________|
page_size=2^{(Byte 0x5 bit 2:0)+9+(Byte 0xc bit 2:0)+2-3}
tFAW=max(tFAWmin,tFAW_page)
************************/
#ifdef AUTO_DDR_CONFIG
//get tRRD_L from SPD: t1
    GET_SPD(0x27)
    move        t1, v0
    GET_SPD(0x76)
    move        t5, v0
	dmulou		t1, t1, 125
	dsrl		t4, t5, 7
	and			t4, t4, 0x1
	beqz		t4, 1f
	nop
    dli         t3, 0xff
    not         t3
	or			t5, t5, t3
1:
    dadd		t1, t1, t5
#endif
#ifndef SPD_ONLY
/*********************************************
calculate the power number of page size: t2
t2= 9 :page size is 0.5K
t2=10 :page size is   1K
t2=11 :page size is   2K
*********************************************/
    GET_SDRAM_WIDTH_V1
    daddu       t2, a1, 11
    GET_COL_SIZE_V1
    dsubu       t2, t2, a1
//choose speed bin
	bleu		t6, 1600, 3f
	nop
	bgeu		t6, 2400, 4f
	nop
	beq			t2, 11, 21f
	nop
	bltu		t2, 11, 22f
	nop
	PRINTSTR("\r\n pagesize wrong \r\n")
	b    		10f
	nop
21:	dli			t2, 6400
	b			9f
	nop
22:	dli			t2, 5300
	b			9f
	nop

3:	beq			t2, 11, 31f
	nop
	bltu		t2, 11, 32f
	nop
	PRINTSTR("\r\n pagesize wrong \r\n")
	b 	    	10f
	nop
31:	dli			t2, 7500
	b			9f
	nop
32:	dli			t2, 6000
	b			9f
	nop
4:	beq			t2, 11, 41f
	nop
	bltu		t2, 11, 42f
	nop
	PRINTSTR("\r\n pagesize wrong \r\n")
	b 	    	10f
	nop
41:	dli			t2, 6400
	b			9f
	nop
42:	dli			t2, 4900
9:
#endif
#ifdef AUTO_DDR_CONFIG
#ifndef SPD_ONLY
//max(tRRD_L_SPD, tRRD_L_Spec)
    bgeu		t1, t2, 10f
	nop
	move		t1, t2
	b			10f
	nop
#endif
#else
    move        t1, t2
#endif
10:
    move        a1, t1
    move        a0, t0
    bal	int_rounding
	nop
    move        t1, v0
	nop
12:
//max(tRRD_L_slr, 4nCK)
    dli			t2, 4
	bgeu		t1, t2, 11f
	nop
	move		t1, t2
11:
	sb			t1, 0x1045(t8)
#ifdef  PARAM_SET_FROM_SPD_DEBUG
	PRINTSTR("\r\n tRRD_L_slr	=	")
	move		a0, t1
	bal			hexserial
	nop
#endif

set_tRCD:
#ifndef DDR3_DIMM
#ifdef AUTO_DDR_CONFIG
//get tRCD from SPD: t1
    GET_SPD(0x19)
    move        t1, v0
    GET_SPD(0x7a)
    move        t2, v0
	dmulou		t1, t1, 125
	dsrl		t4, t2, 7
	and			t4, t4, 0x1
	beqz		t4, 1f
	nop
    dli         t3, 0xff
    not         t3
	or			t2, t2, t3
1:
    dadd		t1, t1, t2
    move        a1, t1
    move        a0, t0
	bal	        int_rounding
	nop
    move        t1, v0
#else
    lb          t1, 0x1060(t8)
#endif
#ifdef AL_ENABLE
#if	(AL==1)
	daddu		t2, t2, 1
	dsrl		t2, t2, 1
#elif	(AL==2)
	daddu		t2, t2, 2
	dsrl		t2, t2, 1
#endif
#endif
#else
    GET_SPD(0x12)
    move        t1, v0
    bal         get_ddr3_mtb
    nop
    dmulou      a1, v0, t1
    move        a0, t0
	bal	        int_rounding
	nop
    move        t1, v0
#endif
	sb			t1, 0x1047(t8)
#ifdef  PARAM_SET_FROM_SPD_DEBUG
	PRINTSTR("\r\n tRCD		=	")
	move		a0, t1
	bal			hexserial
	nop
#endif

set_tFAW:
/************************
tFAW_page define
____________________________________________________________________________________________________________________________________________________
page_size| 1600(1.25ns)	   |  1866(1.071ns)  |   2133(0.938ns)	| 2400(0.833ns)	  |     2666(0.75ns)	|     2933(0.682ns)     |   3200(0.625ns)  |
_________|_________________|_________________|__________________|_________________|_____________________|_______________________|__________________|
2K       |max(28nCK,35ns)  |max(28nCK,30ns)  |<--               |<--              |<--                  |<--                    |<--               |
_________|_________________|_________________|__________________|_________________|_____________________|_______________________|__________________|
1K       |max(20nCK,25ns)  |max(20nCK,23ns)  |max(20nCK,21ns)   |<--              |<--                  |<--                    |<--               |
_________|_________________|_________________|__________________|_________________|_____________________|_______________________|__________________|
0.5K     |max(16nCK,20ns)  |max(16nCK,17ns)  |max(16nCK,15ns)   |max(16nCK,13ns)  |max(16nCK,12ns)      |max(16nck,10.875ns)    |max(16nCK,10ns)   |
_________|_________________|_________________|__________________|_________________|_____________________|_______________________|__________________|
page_size=2^{(Byte 0x5 bit 2:0)+9+(Byte 0xc bit 2:0)+2-3}
tFAW=max(tFAW_SPD,tFAW_spec)
************************/
#ifndef DDR3_DIMM
#ifndef SPD_ONLY
/*********************************************
calculate the power number of page size: t2
t2= 9 :page size is 0.5K
t2=10 :page size is   1K
t2=11 :page size is   2K
*********************************************/
    GET_SDRAM_WIDTH_V1
    daddu       t2, a1, 11
    GET_COL_SIZE_V1
    dsubu       t2, t2, a1
//choose page size
	dli			t5, 11
	beq			t2, t5, 3f
	nop
	dli			t5, 10
	beq			t2, t5, 4f
	nop
	dli			t5,  9
	beq			t2, t5, 5f
	nop
	PRINTSTR("\r\n memsize wrong \r\n")
	b           10f
	nop

//calculate tFAW_spec:t2

//page_size=2K
3:
	dmulou		t2, t0, 28
//choose speed bin
	bleu		t6, 1600, 32f
	nop
//max(28nCK, 30ns)
	dli			t1, 30000
	bgeu		t2, t1, 7f
	nop
	dli			t2, 30000
	b			7f
	nop
//max(28nCK, 35ns)
32:	dli			t1, 35000
	bgeu		t2, t1, 7f
	nop
	dli			t2, 35000
	b			7f
	nop

//page_size=1K
4:
	dmulou		t2, t0, 20
//choose speed bin
	bleu		t6, 1600, 42f
	nop
	beq			t4, 1866, 44f
	nop
//max(20nCK, 21ns)
	dli			t1, 21000
	bgeu		t2, t1, 7f
	nop
	dli			t2, 21000
	b			7f
	nop

//max(20nCK, 25ns)
42:	dli			t1, 25000
	bgeu		t2, t1, 7f
	nop
	dli			t2, 25000
	b			7f
	nop

//max(20nCK, 23ns)
44:	dli			t1, 23000
	bgeu		t2, t1, 7f
	nop
	dli			t2, 23000
	b			7f
	nop

//page_size=0.5K
5:
	dmulou		t2, t0, 16
//choose speed bin
	bleu		t6, 1600, 52f
	nop
	beq			t6, 1866, 54f
	nop
	beq			t6, 2133, 56f
	nop
	beq			t6, 2400, 58f
	nop
	beq			t6, 2666, 60f
	nop
	beq			t6, 2933, 62f
	nop
//max(16nCK, 10ns)
	dli			t1, 10000
	bgeu		t2, t1, 7f
	nop
	dli			t2, 10000
	b			7f
	nop
//max(16nCK, 20ns)
52:	dli			t1, 20000
	bgeu		t2, t1, 7f
	nop
	dli			t2, 20000
	b			7f
	nop
//max(16nCK, 17ns)
54:	dli			t1, 17000
	bgeu		t2, t1, 7f
	nop
	dli			t2, 17000
	b			7f
	nop
//max(16nCK, 15ns)
56:	dli			t1, 15000
	bgeu		t2, t1, 7f
	nop
	dli			t2, 15000
	b			7f
	nop
//max(16nCK, 13ns)
58:	dli			t1, 13000
	bgeu		t2, t1, 7f
	nop
	dli			t2, 13000
	b			7f
	nop
//max(16nCK, 12ns)
60:	dli			t1, 12000
	bgeu		t2, t1, 7f
	nop
	dli			t2, 12000
	b			7f
	nop
//max(16nCK, 10.875ns)
62:	dli			t1, 10875
	bgeu		t2, t1, 7f
	nop
	dli			t2, 10875
//calculate tFAWmin from SPD
7:
#endif
#ifdef AUTO_DDR_CONFIG
    GET_SPD(0x24)
    move        t1, v0
    and         t1, t1, 0xf
    GET_SPD(0x25)
    move        t4, v0
	dsll 		t1, t1, 8
	or			t1, t1, t4
	dmulou		t1, t1, 125
#ifndef SPD_ONLY
//max(tFAWmin, tFAW_page)
	bgeu		t1, t2, 8f
	nop
	move		t1, t2
8:
#endif
#else
    move        t1, t2
#endif
#else
    GET_SPD(0x1c)
    andi        t1, v0, 0xf
    GET_SPD(0x1d)
    andi        v0, 0xff
    dsll        t1, t1, 8
    or          t1, t1, v0
    bal         get_ddr3_mtb
    nop
    dmulou      t1, t1, v0
#endif
    move        a1, t1
    move        a0, t0
    bal     	int_rounding
	nop
    move        t1, v0
9:
	sb			t1, 0x1048(t8)
#ifdef  PARAM_SET_FROM_SPD_DEBUG
	PRINTSTR("\r\n tFAW		=	")
	move		a0, t1
	bal			hexserial
	nop
#endif
10:

set_tFAW_slr:
#ifdef AUTO_DDR_CONFIG
//judge 3DS
    GET_SPD(0x6)
    move        t1, v0
    andi        t1, t1, 0x3
	beq			t1, 2, 1f
	nop
	dli			t1, 0x22
	b			9f
	nop
1:
    GET_SPD(0xc)
    move        t1, v0
    and         t1, t1, 0x7
	beq			t1, 0x0, 3f
	nop
	beq			t6, 1600, 21f
	nop
	beq			t6, 1866, 22f
	nop
	dli			t1, 21000
	b			8f
	nop
21:	dli			t1, 25000
	b			8f
	nop
22:	dli			t1, 23000
	b			8f
	nop
3:
	beq			t6, 1600, 31f
	nop
	beq			t6, 1866, 32f
	nop
	beq			t6, 2133, 33f
	nop
	dli			t1, 13000
	b			8f
	nop
31:	dli			t1, 20000
	b			8f
	nop
32:	dli			t1, 17000
	b			8f
	nop
33:	dli			t1, 15000
	b			8f
	nop
8:
    move        a1, t1
    move        a0, t0
    bal	int_rounding
	nop
    move        t1, v0
#else
    dli         t1, 0x22
#endif
9:
	sb			t1, 0x1049(t8)
#ifdef  PARAM_SET_FROM_SPD_DEBUG
	PRINTSTR("\r\n tFAW_slr	=	")
	move			a0, t1
	bal			hexserial
	nop
#endif

set_tWR:        //max(tWR_SPD,15ns)
#ifndef DDR3_DIMM
#ifdef AUTO_DDR_CONFIG
    GET_SPD(0x29)
    move        t1, v0
    and         t1, t1, 0xf
    GET_SPD(0x2a)
    move        t2, v0
	dsll		t1, 8
	or			t1, t1, t2
	dmulou		t1, t1, 125
#ifndef SPD_ONLY
    bgeu        t1, 15000, 1f
    nop
    dli         t1, 15000
1:
#endif
#else
    dli         t1, 15000
#endif
    move        a1, t1
    move        a0, t0
	bal	        int_rounding
	nop
    move        t1, v0
	nop
//should be bigger than that in MRS
	dli			t2, 10
	bleu		t1, t2, 3f
	nop
	dli			t2, 12
	bleu		t1, t2, 3f
	nop
	dli			t2, 14
	bleu		t1, t2, 3f
	nop
	dli			t2, 16
	bleu		t1, t2, 3f
	nop
	dli			t2, 18
	bleu		t1, t2, 3f
	nop
	dli			t2, 20
	bleu		t1, t2, 3f
	nop
	dli			t2, 22
	bleu		t1, t2, 3f
	nop
	dli			t2, 24
	bleu		t1, t2, 3f
	nop
	dli			t2, 26
	bleu		t1, t2, 3f
	nop
	PRINTSTR("\r\n write recovery time wrong \r\n")
3:	move			t1, t2
	sb			t1, 0x104a(t8)
//set WR in MRS
    bleu        t1, 20, 1f
    nop
    beq         t1, 24, 2f
    nop
    beq         t1, 22, 3f
    nop
    dli         t1, 0x8
    b           4f
    nop
1:
    dsubu       t1, 10
    dsrl        t1, 1
    b           4f
    nop
2:
    dli         t1, 6
    b           4f
    nop
3:
    dli         t1, 7
4:
    andi        t2, t1, 0x7
    dsll        t2, 9
    andi        t4, t1, 0x8
    dsll        t4, 10
    or          t2, t4
    lh          t1, 0x1180(t8)
    dli         t4, (0x1<<13|0x7<<9)
    not         t4
    and         t1, t4
    or          t1, t2
    sh          t1, 0x1180(t8)
    sh          t1, 0x1190(t8)
    sh          t1, 0x11a0(t8)
    sh          t1, 0x11b0(t8)
    sh          t1, 0x11c0(t8)
    sh          t1, 0x11d0(t8)
    sh          t1, 0x11e0(t8)
    sh          t1, 0x11f0(t8)
#else
    GET_SPD(0x11)
    move        t1, v0
    bal         get_ddr3_mtb
    nop
    dmulou      a1, t1, v0
    move        a0, t0
    bal         int_rounding
    nop
//tWRmin should > that in MRS
    bgeu        v0, 5, 1f
    nop
    li          v0, 5
    b           2f
    nop
1:
    bleu        v0, 8, 2f
    nop
    bleu        v0, 16, 1f
    nop
    li          v0, 16
    b           2f
    nop
1:
    andi        t1, v0, 0x1
    beqz        t1, 2f
    nop
    daddu       v0, v0, 1
2:
	sb			v0, 0x104a(t8)
//set WR in MRS
    bgtu        v0, 8, 1f
    nop
    dsubu       v0, v0, 5
    daddu       v0, v0, 1
    b           2f
    nop
1:
    bne         v0, 16, 1f
    nop
    li          v0, 0
    b           2f
    nop
1:
    dsubu       v0, v0 ,10
    dsrl        v0, v0, 1
    daddu       v0, v0, 5
2:
    lh          t1, 0x1140(t8)
    dli         t2, (0x7<<9)
    not         t2, t2
    and         t1, t1, t2
    dsll        v0, v0, 9
    or          t1, t1, v0
    sh          t1, 0x1140(t8)
    sh          t1, 0x1148(t8)
    sh          t1, 0x1150(t8)
    sh          t1, 0x1158(t8)
    sh          t1, 0x1160(t8)
    sh          t1, 0x1168(t8)
    sh          t1, 0x1170(t8)
    sh          t1, 0x1178(t8)
#endif
#ifdef  PARAM_SET_FROM_SPD_DEBUG
	PRINTSTR("\r\n tWR		=	")
    lb          a0, 0x104a(t8)
	bal			hexserial
	nop
#endif

#ifdef SET_BY_PROTOCOL
set_tWR_CRC_DM:     //this value replaces tWR if CRC and DM are both enbaled
/*********************************
 1600:   tWR+max(4nCK,3.75ns)
>1600:   tWR+max(5nCK,3.75ns)
*********************************/
	dsll		t3, t0, 2
	bleu		t6, 1600, 1f
	nop
	daddu		t3, t3, t0
1:
	bgeu		t3, 3750, 2f
	nop
    dli         t3, 3750
2:
#ifdef AUTO_DDR_CONFIG
//get tWR
    GET_SPD(0x29)
    move        t1, v0
    and         t1, t1, 0xf
    GET_SPD(0x2a)
    move        t2, v0
	dsll		t1, 8
	or			t1, t1, t2
	dmulou		t1, t1, 125
#ifndef SPD_ONLY
    bgeu        t1, 15000, 1f
    nop
    dli         t1, 15000
1:
#endif
#else
    dli         t1, 15000
#endif
    daddu       a1, t3, t1
    move        a0, t0
	bal	        int_rounding
	nop
    move        t1, v0
	sb			v0, 0x104b(t8)
#ifdef  PARAM_SET_FROM_SPD_DEBUG
	PRINTSTR("\r\n tWR_CRC_DM	=	")
	move		a0, t1
	bal			hexserial
	nop
#endif

set_tRTP:					#tRTP=max(4nCK,7.5ns)
#ifndef DDR3_DIMM
	dli			t1, 7500
	dsll		t2, t0, 2
	bgeu		t2, t1, 1f
	nop
    move        a1, t1
    move        a0, t0
	bal	int_rounding
	nop
    move        t1, v0
	b			2f
	nop
1:	dli			t1, 4
2:
#else
    GET_SPD(0x1b)
    move        t1, v0
    bal         get_ddr3_mtb
    nop
    dmulou      a1, t1, v0
    move        a0, t0
	bal	        int_rounding
	nop
    move        t1, v0
#endif
	sb			t1, 0x104c(t8)
#ifdef  PARAM_SET_FROM_SPD_DEBUG
	PRINTSTR("\r\n tRTP		=	")
	move		a0, t1
	bal			hexserial
	nop
#endif
#endif
set_tCCD_L:
/************************************
1600        max(5nCK,6.25ns)
[1866,2133] max(5nCK,5.355ns)
>2133       max(5nCK,5ns)
************************************/
#ifndef SPD_ONLY
	dli			t1, 5
	dmulou		t2, t0, t1
	bleu		t6, 1600, 3f
	nop
	bleu		t6, 2133, 4f
	nop
	dli			t1, 5000
	bgeu		t2, t1, 6f
	nop
	b			5f
	nop
3:	dli			t1, 6250
	bgeu		t2, t1, 6f
	nop
	b			5f
	nop
4:	dli			t1, 5355
	bgeu		t2, t1, 6f
	nop
5:	move		t2, t1
6:
#endif
#ifdef AUTO_DDR_CONFIG
//calculate the tCCD_Lmin: t5
    GET_SPD(0x28)
    move        t4, v0
    GET_SPD(0X75)
    move        t5, v0
	dmulou		t4, t4, 125
	dsrl		t1, t5, 7
	and			t1, t1, 0x1
	beqz		t1, 7f
	nop
    dli         t3, 0xff
    not         t3
	or			t1, t5, t3
7:
    dadd		t1, t1, t4
#ifndef SPD_ONLY
	bgeu		t1, t2, 8f
	nop
	move		t1, t2
8:
#endif
#else
    move        t1, t2
#endif
    move        a1, t1
    move        a0, t0
    bal	int_rounding
	nop
    move        t1, v0
#ifndef SPD_ONLY
    bleu        t6, 1333, 9f
    nop
    bleu        t6, 1866, 10f
    nop
    dli         t2, 6
    b           11f
    nop
9:  dli         t2, 4
    b           11f
    nop
10: dli         t2, 5
11: bgeu        t1, t2, 12f
    nop
    move        t1, t2
12:
#endif
	sb			t1, 0x1050(t8)
//set CCD_L in MRS
    dsubu       t1, 4
    dsll        t1, 10
    lh          t2, 0x118c(t8)
    dli         t4, (0x7<<10)
    not         t4
    and         t2, t4
    or          t2, t1
    sh          t2, 0x118c(t8)
    sh          t2, 0x119c(t8)
    sh          t2, 0x11ac(t8)
    sh          t2, 0x11bc(t8)
    sh          t2, 0x11cc(t8)
    sh          t2, 0x11dc(t8)
    sh          t2, 0x11ec(t8)
    sh          t2, 0x11fc(t8)
#ifdef  PARAM_SET_FROM_SPD_DEBUG
	PRINTSTR("\r\n tCCD_L		=	")
    lb          a0, 0x1050(t8)
	bal			hexserial
	nop
#endif

#ifdef SET_BY_PROTOCOL
set_tCCD_S:
	dli			t1, 4
	sb			t1, 0x1051(t8)
#ifdef  PARAM_SET_FROM_SPD_DEBUG
	PRINTSTR("\r\n tCCD_S		=	")
	move		a0, t1
	bal			hexserial
	nop
#endif
#endif

set_tWTR:				#tWTR_L=max(4nCK,7.5ns)
#ifndef DDR3_DIMM
#ifdef AUTO_DDR_CONFIG
    GET_SPD(0x2b)
    move        t1, v0
    dsrl        t1, t1, 4
    and         t1, t1, 0xf
    GET_SPD(0x2d)
    move        t5, v0
	dsll		t1, 8
	or			t1, t1, t5
	dli			t5, 125
	dmulou		t1, t1, t5
#endif
#ifndef SPD_ONLY
	dsll		t2, t0, 2
	bgeu		t2, 7500, 1f
	nop
	dli			t2, 7500
1:
#endif
#ifdef AUTO_DDR_CONFIG
#ifndef SPD_ONLY
	bgeu		t1, t2, 2f
	nop
	move 		t1, t2
2:
#endif
#else
    move        t1, t2
#endif
#else
    GET_SPD(0x1a)
    move        t1, v0
    bal         get_ddr3_mtb
    nop
    dmulou      t1, t1, v0
#endif
    move        a1, t1
    move        a0, t0
    bal     	int_rounding
	nop
    move        t1, v0
	sb			v0, 0x1054(t8)
#ifdef  PARAM_SET_FROM_SPD_DEBUG
	PRINTSTR("\r\n tWTR		=	")
	move		a0, t1
	bal			hexserial
	nop
#endif

set_tWTR_S:				#tWTR_s=max(2nCK,2.5ns)
#ifdef AUTO_DDR_CONFIG
    GET_SPD(0x2b)
    move        t1, v0
    and         t1, t1, 0xf
    GET_SPD(0x2c)
    move        t5, v0
	dsll		t1, 8
	or			t1, t1, t5
	dli			t5, 125
	dmulou		t1, t1, t5
#endif
#ifndef SPD_ONLY
	dsll		t2, t0, 1
	bgeu		t2, 2500, 1f
	nop
	dli			t2, 2500
1:
#endif

#ifdef AUTO_DDR_CONFIG
#ifndef SPD_ONLY
	bgeu		t1, t2, 2f
	nop
	move 		t1, t2
2:
#endif
#else
    move        t1, t2
#endif
    move        a1, t1
    move        a0, t0
    bal	int_rounding
	nop
    move        t1, v0
	sb			v0, 0x1055(t8)
#ifdef  PARAM_SET_FROM_SPD_DEBUG
	PRINTSTR("\r\n tWTR_S		=	")
	move		a0, t1
	bal			hexserial
	nop
#endif

#ifdef SET_BY_PROTOCOL
set_tWTR_L_CRC_DM:			#tWTR_L_CRC_DM=tWTR_L+max(4nCK,3.75ns)-->1600MHz	tWTR_L_CRC_DM=tWTR_L+max(5nCK,3.75ns)-->not1600
#ifdef AUTO_DDR_CONFIG
//get WTR_L to t1
    GET_SPD(0x2b)
    move        t1, v0
    dsrl        t1, t1, 4
    and         t1, t1, 0xf
    GET_SPD(0x2d)
    move        t5, v0
	dsll		t1, 8
	or			t1, t1, t5
	dli			t5, 125
	dmulou		t1, t1, t5
#endif
#ifndef SPD_ONLY
	dsll		t2, t0, 2
	bgeu		t2, 7500, 1f
	nop
	dli			t2, 7500
1:
#endif
#ifdef AUTO_DDR_CONFIG
#ifndef SPD_ONLY
	bgeu		t1, t2, 2f
	nop
	move 		t1, t2
2:
#endif
#else
    move        t1, t2
#endif
//max(4nCK,3.75ns) or max(5nCK,3.75ns)
	dsll		t2, t0, 2
	bleu		t6, 1600, 3f
	nop
	daddu		t2, t2, t0
3:	bgeu		t2, 3750, 4f
	nop
	dli			t2, 3750
4:
    daddu       a1, t1, t2
    move        a0, t0
    bal	        int_rounding
	nop
    move        t1, v0
	sb			v0, 0x1056(t8)
#ifdef  PARAM_SET_FROM_SPD_DEBUG
	PRINTSTR("\r\n tWTR_L_CRC_DM	=	")
	move		a0, t1
	bal			hexserial
	nop
#endif

set_tWTR_S_CRC_DM:			#tWTR_S_CRC_DM=tWTR_S+max(4nCK,3.75ns)-->1600MHz	tWTR_S_CRC_DM=tWTR_S+max(5nCK,3.75ns)-->not1600
#ifdef AUTO_DDR_CONFIG
//get tWTR_S to t1
    GET_SPD(0x2b)
    move        t1, v0
    and         t1, t1, 0xf
    GET_SPD(0x2c)
    move        t5, v0
	dsll		t1, 8
	or			t1, t1, t5
	dli			t5, 125
	dmulou		t1, t1, t5
#endif
#ifndef SPD_ONLY
	dsll		t2, t0, 1
	bgeu		t2, 2500, 1f
	nop
	dli			t2, 2500
1:
#endif
#ifdef AUTO_DDR_CONFIG
#ifndef SPD_ONLY
	bgeu		t1, t2, 2f
	nop
	move 		t1, t2
2:
#endif
#else
    move        t1, t2
#endif
//max(4nCK,3.75ns) or max(5nCK,3.75ns)
	dsll		t2, t0, 2
	bleu		t6, 1600, 3f
	nop
	daddu		t2, t2, t0
3:	bgeu		t2, 3750, 4f
	nop
	dli			t2, 3750
4:
    daddu       a1, t1, t2
    move        a0, t0
    bal	        int_rounding
	nop
    move        t1, v0
	sb			v0, 0x1057(t8)
#ifdef  PARAM_SET_FROM_SPD_DEBUG
	PRINTSTR("\r\n tWTR_S_CRC_DM	=	")
	move		a0, t1
	bal			hexserial
	nop
#endif
#endif

set_tPHY_RDLAT:
	lb			t1, 0x1060(t8)
	daddu		t1, t1, 0x7
	sb			t1, 0x1061(t8)
#ifdef  PARAM_SET_FROM_SPD_DEBUG
	PRINTSTR("\r\n tPHY_RDLAT	=	")
	move		a0, t1
	bal			hexserial
	nop
#endif

#if 0
set_tPHY_WRLAT:
    lbu         t1, 0x1448(t8)
	lb			t2, 0x1064(t8)
    dsubu       t2, t2, 9
    daddu       t1, t1, t2
    daddu       t1, t1, 5
    GET_DIMM_TYPE_V1
    beqz        a1, 1f
    nop
    daddu       t1, 1
1:
    sb          t1, 0x1065(t8)
#ifdef  PARAM_SET_FROM_SPD_DEBUG
	PRINTSTR("\r\n tPHY_WRLAT	=	")
	move		a0, t1
	bal			hexserial
	nop
#endif
#endif

#if 0
set_tCAL:					#tCAL=max(3nCK,3.748ns)
#ifdef  CAL_ENABLE
	dli			t1, 3748
	dli			t4, 3
	dmulou		t2, t0, t4
	bgeu		t1, t2, 1f
	nop
	dli			t1, 3
	b			2f
	nop
1:
    move        a1, t1
    move        a0, t0
    bal	int_rounding
	nop
    move        t1, v0
2:
#else
    dli         t1, 0
#endif
	sb			t1, 0x106c(t8)
#ifdef  PARAM_SET_FROM_SPD_DEBUG
	PRINTSTR("\r\n tCAL		=	")
	move			a0, t1
	bal			hexserial
	nop
#endif
#endif

#ifdef DEBUG_DDR_PARAM
    move    s7, t8
    move    t4, ra

    bal     tgt_testchar
    nop
    beqz    v0, 3f
    nop
1:
    PRINTSTR("\r\nChange parameters?(0xffff: skip): ")
    bal     inputaddress
    nop
    dli     a1, 0xffff
    beq     v0, a1, 3f
    nop
    and     v0, v0, 0xfff8
    daddu   s6, v0, s7

    PRINTSTR("\r\nInput data: ")
    bal     inputaddress
    nop
    sd      v0, 0x0(s6)

    PRINTSTR("\r\nThe parameter just write is: ")
    ld      t6, 0x0(s6)
    move    a0, s6
    and     a0, a0, 0xffff
    bal     hexserial
    nop
    PRINTSTR(":  ")
    dsrl    a0, t6, 32
    bal     hexserial
    nop
    //PRINTSTR("  ")
    move    a0, t6
    bal     hexserial
    nop
    PRINTSTR("\r\n")
    b       1b
    nop

3:
    dsrl    a0, s7, 32
    bal     hexserial
    nop
    //PRINTSTR("  ")
    move    a0, s7
    bal     hexserial
    nop
    PRINTSTR("\r\n")

    move    a0, s7
    move    ra, t4

#endif

##START
ddr_init:
#ifdef VDDP_CTR
//change vddp to 1.1v
    PRINTSTR("\r\nchanging vddp to ")
    li      t1, (FIRST_VDDP - 41)
    li      t2, 10
    div     t0, t1, t2
    li      t1, 100
    mul     t0, t0, t1
    addu    a0, t0, 900
    bal     hexserial
    nop
    PRINTSTR(" mV")
    li      t0, FIRST_VDDP
    bal     v_p_ctrl_in_mc
    nop
#endif

//choose clk_2x of ckca according to MC version
    lhu     t3, (PHY_ADDRESS)(t8)
    bne     t3, 0x11, 1f
    nop
#ifdef CLK2X_CKCA_SEL
//ds2 clk_2x choose clk_2x of ckca
    lbu     t3, (DLL_CTRL + 1)(t8)
    ori     t3, (0x1 << 5)
    sb      t3, (DLL_CTRL + 1)(t8)
//ds6 clk_2x choose clk_2x of ckca
    lbu     t3, (DLL_CTRL + 2)(t8)
    ori     t3, (0x1 << 5)
    sb      t3, (DLL_CTRL + 2)(t8)

//ds2 and ds6 choose pll to pd
    lb      t3, 0x78(t8)
    li      t4, ~(0x1)
    and     t3, t4
    sb      t3, 0x78(t8)
    lb      t3, 0xb8(t8)
    li      t4, ~(0x1)
    and     t3, t4
    sb      t3, 0xb8(t8)
#else
//ds2 clk_2x chosse itself
    lbu     t3, (DLL_CTRL + 1)(t8)
    dli     t4, (0x1 << 5)
    not     t4
    and     t3, t4
    sb      t3, (DLL_CTRL + 1)(t8)
//ds6 clk_2x choose itself
    lbu     t3, (DLL_CTRL + 2)(t8)
    dli     t4, (0x1 << 5)
    not     t4
    and     t3, t4
    sb      t3, (DLL_CTRL + 2)(t8)

#endif

//config clk2x dll bypass
    lw      t3, (DLL_CTRL)(t8)
    dli     t4, 0x101010
#ifndef CLK2X_DLL_BYPASS
    not     t4, t4
    and     t3, t3, t4
#else
    or      t3, t3, t4
#endif
    sw      t3, (DLL_CTRL)(t8)

//config clk2x predly bypass
    lw      t3, (DLL_CTRL)(t8)
    dli     t4, 0x010101
#ifndef DLL_PREDELAY_BYPASS
    not     t4, t4
    and     t3, t3, t4
#else
    or      t3, t3, t4
#endif
    sw      t3, (DLL_CTRL)(t8)

1:
//pd pll of ckca ds2 and ds6
    lb      t3, 0x4f(t8)
    ori     t3, 0x1
    sb      t3, 0x4f(t8)

    lb      t3, 0x7f(t8)
    ori     t3, 0x1
    sb      t3, 0x7f(t8)

    lb      t3, 0xbf(t8)
    ori     t3, 0x1
    sb      t3, 0xbf(t8)

//enable dbl dcc of ckca ds2 and ds6
#ifdef DCC_CTR
    lw      t3, 0x44(t8)
    dli     t4, (0x1 << 14)
    or      t3, t4
    sw      t3, 0x44(t8)

    lw      t3, 0x74(t8)
    dli     t4, 0x1<<14
    or      t3, t3, t4
    sw      t3, 0x74(t8)

    lw      t3, 0xb4(t8)
    dli     t4, 0x1<<14
    or      t3, t3, t4
    sw      t3, 0xb4(t8)
#endif

//set soft rest_en
    lhu     t3, (PHY_ADDRESS)(t8)
    bne     t3, 0x11, 1f
    nop

    lw      t3, (DLL_CTRL)(t8)
    dli     t4, 0x404040
    or      t3, t3, t4
    sw      t3, (DLL_CTRL)(t8)
1:

    lb      t3, (DLL_CTRL)(t8)
    lb      t3, (DLL_CTRL)(t8)
    lb      t3, (DLL_CTRL)(t8)
    lb      t3, (DLL_CTRL)(t8)
    lb      t3, (DLL_CTRL)(t8)

    ld      t3, (PHY_ADDRESS + 0x010)(t8)
    ori     t3, (0x1 << 1)     //phy_init_start
    sd      t3, (PHY_ADDRESS + 0x010)(t8)

    PRINTSTR("\r\ninit start")

//dll init done check
1:
    ld      t3, (PHY_ADDRESS + 0x030)(t8)
    dli     t4, (0x1 << 40)
    and     t3, t3, t4
    beqz    t3, 1b
    nop

#if (SEL_DBL == 0)
//get MC version
    lhu     t3, (PHY_ADDRESS)(t8)
    bne     t3, 0x11, 3f
    nop

#ifndef CLK2X_CKCA_SEL
//ds2 pll pd and on
    lw      t3, 0x7c(t8)
    li      t4, 0x1<<24
    or      t3, t3, t4
    sw      t3, 0x7c(t8)

    xor     t3, t4, t3
    sw      t3, 0x7c(t8)
1:
    lb      t3, 0x7b(t8)
    andi    t3, t3, 1
    beqz    t3, 1b
    nop
//ds6 pll pd and on
    lw      t3, 0xbc(t8)
    li      t4, 0x1<<24
    or      t3, t3, t4
    sw      t3, 0xbc(t8)

    xor     t3, t4, t3
    sw      t3, 0xbc(t8)
1:
    lb      t3, 0xbb(t8)
    andi    t3, t3, 1
    beqz    t3, 1b
    nop
#endif
    b       2f
    nop

3:
//ds2 pll pd and on
    lw      t3, 0x7c(t8)
    li      t4, 0x1<<24
    or      t3, t3, t4
    sw      t3, 0x7c(t8)

    xor     t3, t4, t3
    sw      t3, 0x7c(t8)
1:
    lb      t3, 0x7b(t8)
    andi    t3, t3, 1
    beqz    t3, 1b
    nop
//ds2 pll pd and on
    lw      t3, 0xbc(t8)
    li      t4, 0x1<<24
    or      t3, t3, t4
    sw      t3, 0xbc(t8)

    xor     t3, t4, t3
    sw      t3, 0xbc(t8)
1:
    lb      t3, 0xbb(t8)
    andi    t3, t3, 1
    beqz    t3, 1b
    nop
2:
//ckca pll pd
    lw      t3, 0x4c(t8)
    li      t4, 0x1<<24
    or      t3, t4
    sw      t3, 0x4c(t8)

    xor     t3, t4, t3
    sw      t3, 0x4c(t8)
1:
    lb      t3, 0x4b(t8)
    andi    t3, t3, 1
    beqz    t3, 1b
    nop
#endif

//get MC version
    lhu     t3, (PHY_ADDRESS)(t8)
    bne     t3, 0x11, dll_lock_check
    nop

#ifndef CLK2X_DLL_BYPASS
#ifndef DLL_PREDELAY_BYPASS
/*t2:   0   ckca
        1   ds2
        2   ds6 */
    dli     t1, 1
    dli     t2, 0
    move    t0, t8
    move    t5, t8
dll_lock_or_overflow_check:
    ld      t3, 0x48(t0)
    dli     t4, (0x1 << 16)
    and     t3, t3, t4
    lbu     t4, (DLL_OUT)(t5)
    andi    t4, (0x1 << 6)
    or      t3, t4
    beqz    t3, dll_lock_or_overflow_check
    nop
//check overflow of predelay
    lbu     t4, (DLL_OUT)(t5)
    andi    t4, (0x1 << 6)
    beqz    t4, predly_loop_ctrl
    nop
//add predelay
//dll resetn
    dli     t4, (0x1 << 7)
    not     t4
    and     t3, t4
    sb      t3, (DLL_CTRL)(t5)
//add predelay 3'b1 as thermometro code
    dli     t4, (0x7 << 1)
    not     t4
    and     t3, t4
    dli     t4, 0x2
    dsll    t4, t1
    dsubu   t4, 0x2
    or      t3, t4
    sb      t3, (DLL_CTRL)(t5)

    dli     t4, WAIT_ITEM
1:
    dsubu   t4, 1
    bnez    t4, 1b
    nop
//clear dll resetn
    lbu     t3, (DLL_CTRL)(t5)
    ori     t3, (0x1 << 7)
    sb      t3, (DLL_CTRL)(t5)

    lbu     t3, (DLL_CTRL)(t5)
    lbu     t3, (DLL_CTRL)(t5)
    lbu     t3, (DLL_CTRL)(t5)
    lbu     t3, (DLL_CTRL)(t5)
    lbu     t3, (DLL_CTRL)(t5)

    daddu   t1, 1
    bleu    t1, 4, dll_lock_or_overflow_check
    nop
    PRINTSTR("\r\nError: predly lock failed")
1:
    b       1b
    nop

//loop ctrl to select ds2 or ds6
predly_loop_ctrl:
    dli     t1, 1
    daddu   t2, 1
    daddu   t5, 1
    beq     t2, 1, 1f
    nop
    beq     t2, 2, 2f
    nop
    b       dll_lock_check
    nop
1:
    daddu   t0, t8, 0x30
    b       dll_lock_or_overflow_check
    nop
2:
    daddu   t0, t8, 0x70
    b       dll_lock_or_overflow_check
    nop
#endif
#endif

dll_lock_check:
//get MC version
    lhu     t3, (PHY_ADDRESS)(t8)
    bne     t3, 0x11, 2f
    nop

//dll resetn
    lw      t3, (DLL_CTRL)(t8)
    dli     t4, 0x7f7f7f
    and     t3, t3, t4
    sw      t3, (DLL_CTRL)(t8)

    dli     t4, WAIT_ITEM
1:
    dsubu   t4, 1
    bnez    t4, 1b
    nop
//clear dll resetn
    lw      t3, (DLL_CTRL)(t8)
    dli     t4, 0x808080
    or      t3, t3, t4
    sw      t3, (DLL_CTRL)(t8)

    lbu     t3, (DLL_CTRL)(t8)
    lbu     t3, (DLL_CTRL)(t8)
    lbu     t3, (DLL_CTRL)(t8)
    lbu     t3, (DLL_CTRL)(t8)
    lbu     t3, (DLL_CTRL)(t8)

2:
#ifdef CLK2X_DLL_BYPASS
//get MC version
    lhu     t3, (PHY_ADDRESS)(t8)
    bne     t3, 0x10, set_clk_en
    nop
#endif
    li      t6,0x100000
1:
    lb      t3, 0x4a(t8)
    andi    t3, t3, 0x1
    lb      t1, 0x7a(t8)
    andi    t1, t1, 0x1
    lb      t2, 0xba(t8)
    andi    t2, t2, 0x1
    and     t1, t1, t2
    and     t3, t1, t3
    subu    t6, 1
    beqz    t6, 1f
    nop

    beqz    t3, 1b
    nop

1:
    beqz    t3, dll_lock_check
    nop

    ld      t1, 0x908(t8)
    PRINTSTR("\r\n908:")
    dsrl    a0, t1, 32
    bal     hexserial
    nop
    move    a0, t1
    bal     hexserial
    nop
#if (SEL_DBL==0)//select small dll
#ifdef SELECT_SMALL_DLL
    lb      t3, DLL_OUT(t8)
    bgtu    t3, DLL_SELECT, 1f
    nop
    lb      t3, (DLL_OUT+1)(t8)
    bgtu    t3, DLL_SELECT, 1f
    nop
    lb      t3, (DLL_OUT+2)(t8)
    bgtu    t3, DLL_SELECT, 1f
    nop
    b       2f
    nop
1:      //total reset
    GET_NODE_ID_a0
    dli     t1, 0x900000001fe00180
    or      t1, t1, a0
    ld      t0, 0x0(t1)
    dli     t2, 0x1<<7
    beqz    k0, 1f
    nop
    dsll    t2, t2, 5
1:
    not     t2, t2
    and     t0, t0, t2
    sd      t0, 0x0(t1)

    dli     t3, 0x1000
1:
    dsubu   t3, t3, 1
    bnez    t3, 1b
    nop

    not     t2, t2
    or      t0, t0, t2
    sd      t0, 0x0(t1)

    dli     t3, 0x1000
1:
    dsubu   t3, t3, 1
    bnez    t3, 1b
    nop

    PRINTSTR("\r\n begin Reset MC ")
    move    a2, k1
    b       mc_reset
    nop
2:
#endif
#endif

set_clk_en:
#ifdef DCC_CTR
    PRINTSTR("\r\nbefore change REG0x44: ")
    lw      a0, 0x44(t8)
    bal     hexserial
    nop
//bypass clk2x dcc
    lw      t3, 0x44(t8)
    dli     t4, (0x1 << 14)
    not     t4
    and     t3, t4
    sw      t3, 0x44(t8)

    PRINTSTR("\r\nafter change REG0x44: ")
    lw      a0, 0x44(t8)
    bal     hexserial
    nop

    PRINTSTR("\r\nbefore change REG0x74: ")
    lw      a0, 0x74(t8)
    bal     hexserial
    nop
//bypass clk2x dcc
    lw      t3, 0x74(t8)
    dli     t4, (0x1 << 14)
    not     t4
    and     t3, t4
    sw      t3, 0x74(t8)

    PRINTSTR("\r\nafter change REG0x74: ")
    lw      a0, 0x74(t8)
    bal     hexserial
    nop

    PRINTSTR("\r\nbefore change REG0xb4: ")
    lw      a0, 0xb4(t8)
    bal     hexserial
    nop
//bypass clk2x dcc
    lw      t3, 0xb4(t8)
    dli     t4, (0x1 << 14)
    not     t4
    and     t3, t4
    sw      t3, 0xb4(t8)

    PRINTSTR("\r\nafter change REG0xb4: ")
    lw      a0, 0xb4(t8)
    bal     hexserial
    nop

    dli     t4, WAIT_ITEM
1:
    dsubu   t4, 1
    bnez    t4, 1b
    nop
#endif

//clk2x enable
    ld      t3, 0x48(t8)
    ori     t3, t3, (1 << 8)
    sd      t3, 0x48(t8)
    ld      t3, 0x78(t8)
    ori     t3, t3, (1 << 8)
    sd      t3, 0x78(t8)
    ld      t3, 0xb8(t8)
    ori     t3, t3, (1 << 8)
    sd      t3, 0xb8(t8)

//pm_init_start
    ld      t3, (PHY_ADDRESS + 0x010)(t8)
    ori     t3, t3, 0x1         //init_start
    sd      t3, (PHY_ADDRESS + 0x010)(t8)

//dram_init_chk
1:
    ld      t3, (CTL_ADDRESS + 0x100)(t8)
    andi    t1, t3, 0xff     //cs_enable
    ld      t3, (PHY_ADDRESS + DRAM_INIT)(t8)
    andi    t3, t3, 0xff00
    dsrl    t3, t3, 0x8
    bne     t3, t1, 1b
    nop

#ifdef VDDP_CTR
    PRINTSTR("\r\nnow vddp is ")
    li      t1, (FIRST_VDDP - 41)
    li      t2, 10
    div     t0, t1, t2
    li      t1, 100
    mul     t0, t0, t1
    addu    a0, t0, 900
    bal     hexserial
    nop
    PRINTSTR(" mV\r\n908:")
    ld      t3, 0x908(t8)
    dsrl    a0, t3, 32
    bal     hexserial
    nop
    move    a0, t3
    bal     hexserial
    nop

//change vddp to 1.3v
    PRINTSTR("\r\nchanging vddp to ")
    li      t1, (SECOND_VDDP - 41)
    li      t2, 10
    div     t0, t1, t2
    li      t1, 100
    mul     t0, t0, t1
    addu    a0, t0, 900
    bal     hexserial
    nop
    PRINTSTR(" mV")
    li      t0, SECOND_VDDP
    bal     v_p_ctrl_in_mc
    nop
    PRINTSTR("\r\nnow vddp is ")
    li      t1, (SECOND_VDDP - 41)
    li      t2, 10
    div     t0, t1, t2
    li      t1, 100
    mul     t0, t0, t1
    addu    a0, t0, 900
    bal     hexserial
    nop
    PRINTSTR(" mV\r\n908:")
    ld      t3, 0x908(t8)
    dsrl    a0, t3, 32
    bal     hexserial
    nop
    move    a0, t3
    bal     hexserial
    nop
#endif
    PRINTSTR("\r\ninit done")

#ifdef CAL_ENABLE
//calculate tCKAVG and store in t0
	dli	    	t0, DDR_FREQ
    dsll        t0, t0, 1
    dli         t1, 1000000
    dmulou      t0, t0, t1
	dli	    	t2, 0xe8d4a51000
	ddivu		t0, t2, t0

	dli			t1, 3748
	dli			t4, 3
	dmulou		t2, t0, t4
	bgeu		t1, t2, 1f
	nop
	dli			t2, 3
	b			2f
	nop
1:
    move        a1, t1
    move        a0, t0
    bal	int_rounding
	nop
    move        t2, v0      //t2 means tCAL
2:
//first enable CAL by mrs
//enter command mode
    li      t1, 0x1
    sb      t1, CMD_OFFSET(t8)

    li      t1, 0x0
    sb      t1, CMD_REQ_OFFSET(t8)

1:
    lbu     t1, CMD_STATUS_OFFSET(t8)
    beqz    t1, 1b
    nop

//set mrs_cmd
    li      t1, 0xff
    sb      t1, CMD_CKE_OFFSET(t8)

    li      t1, 0x0
    sb      t1, CMD_C_OFFSET(t8)

    li      t1, 0x8//mrs
    sb      t1, CMD_CMD_OFFSET(t8)

    lh      t1, DDR4_MR4_CS0_REG(t8)
    move    t0, t2
    bleu    t2, 6, 1f
    nop
    dsubu   t2, t2, 1
1:
    dsubu   t2, t2, 2
    andi    t2, 0x7
    dsll    t2, t2, 6
    dli     t3, ~(0x7<<6)
    and     t1, t1, t3
    or      t1, t1, t2
    sh      t1, CMD_A_OFFSET(t8)

    //select MRS4
    li      t1, 0x0
    sb      t1, CMD_BA_OFFSET(t8)
    li      t1, 0x1
    sb      t1, CMD_BG_OFFSET(t8)

    li      t1, 0xfe
    sb      t1, CMD_CS_OFFSET(t8)//TODO, now only support cs0

//send cmd
    li      t1, 0x1
    sb      t1, CMD_REQ_OFFSET(t8)

    dli     t1, 0x60
1:
    subu    t1, 1
    bnez    t1, 1b
    nop

    li      t1, 0x0
    sb      t1, CMD_REQ_OFFSET(t8)

    li      t1, 0x0
    sb      t1, CMD_OFFSET(t8)

	sb			t0, 0x106c(t8)

    lb      t0, 0x808(t8)
    dli     t1, ~(0x1)
    and     t0, t0, t1
    sb      t0, 0x808(t8)
#endif

    move    ra, t7
    jr      ra
    nop
    .end    mc_config

LEAF(int_rounding)
/*******************************
nCK=truncate[{(parameter_in_ps*1000)/application_tCK_in_ps+974}/1000]  =>  DDR4 Spec
input:
	a1 -> timing parameter
	a0 -> tCKAVG
output: v0 -> the number of nCK
*******************************/
	dmulou		a1, a1, 1000
#ifndef DDR3_DIMM
	bgtu		a0, 1600, 1f
	nop
#endif
	bgeu		a0, 1500, 2f	#tCK<1.6nS   & tCK>=1.5ns	f=1333MHz
	nop
	bgeu		a0, 1250, 3f	#tCK<1.5ns   & tCK>=1.25ns 	f=1600MHz
	nop
	bgeu		a0, 1071, 4f	#tCK<1.25ns  & tCK>=1.071ns	f=1866MHz
	nop
	bgeu		a0,  937, 5f	#tCK<1.071ns & tCK>=0.938ns	f=2133MHz
	nop
	bgeu		a0,  833, 6f	#tCK<0.938ns & tCK>=0.833ns	f=2400MHz
	nop
	bgeu		a0,  750, 7f	#tCK<0.833ns & tCK>=0.750ns	f=2666MHz
	nop
	bgeu		a0,  682, 8f	#tCK<0.750ns & tCK>=0.682ns	f=2933MHz
	nop
	bgeu		a0,  625, 9f	#tCK<0.682ns & tCK>=0.625ns	f=3200MHz
	nop
1:	ddivu		a1, a1, a0
	b		10f
	nop
2:	ddivu		a1, a1, 1500
	b		10f
	nop
3:	ddivu		a1, a1, 1250
	b		10f
	nop
4:	ddivu		a1, a1, 1071
	b		10f
	nop
5:	ddivu		a1, a1, 937
	b		10f
	nop
6:	ddivu		a1, a1, 833
	b		10f
	nop
7:	ddivu		a1, a1, 750
	b		10f
	nop
8:	ddivu		a1, a1, 682
	b		10f
	nop
9:	ddivu		a1, a1, 625
10:	daddu		a1, a1, 974
	ddivu		a1, a1, 1000
    move        v0, a1
	jr		ra
	nop
END(int_rounding)
LEAF(get_ddr3_mtb)
//output v0(ps)
    move    t4, ra
    GET_SPD(0xa)
    li      t5, 1000
    dmulou  t5, v0, t5
    GET_SPD(0xb)
    ddivu   v0, t5, v0
    jr      t4
    nop
END(get_ddr3_mtb)
