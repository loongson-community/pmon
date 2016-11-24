/*whd : loongson3_ddr_init_4core.S
        used to speed up the ddr leveling on 4way system
        from the KD90 boot codes, first writing by chenxk
*/

//#define  SEQ_INIT_MEM   //Concurrent init not work now.
#define  CORE0_WAIT_ALL  //make core0 wait other cpus finishing the ddr leveling
//#ifndef  SEQ_INIT_MEM
//#define  PRINTSTR(x)
//#else
//#define  CORE0_WAIT_ALL
//#endif


MEM_INIT_BEGIN:

#if 1 //enable cache
	## enable kseg0 cachablilty####
	mfc0	t6, CP0_CONFIG
	ori	t6, t6, 7
	xori	t6, t6, 4
	mtc0	t6, CP0_CONFIG


	#jump to cached kseg0 address
	lui     t6, 0xdfff 
	ori     t6, t6, 0xffff
	bal     1f
	nop
1:
	and     ra, ra, t6
	daddiu	ra, ra, 16
	jr      ra
	nop
#endif


//init mem on order(NODE 0>1>2>3)
    .set     mips64
    mfc0    t0, $15, 1
    .set     mips3
    andi    t0, 0xc
    dsrl    t0, t0, 2
    dli     a0, 0x0
    beq     t0, a0, NODE0_INIT_MEM
    nop

	/* NODE 1 */
    daddu   a0, a0, 0x1
    beq     t0, a0, NODE1_INIT_MEM
    nop

	/* NODE 2 */
    daddu   a0, a0, 0x1
    beq     t0, a0, NODE2_INIT_MEM
    nop

	/* NODE 3 */
    daddu   a0, a0, 0x1
    beq     t0, a0, NODE3_INIT_MEM
    nop

//NO one arrive here
    b       ALL_NODE_MEM_INIT_DONE
    nop



NODE0_INIT_MEM:
#if 0
    //ASK all to start, put after I2C read for S1
    dli     t0, NODE0_CORE0_BUF0
    li      a1, NODE_SCACHE_ENABLED
1:
    sw      a1, SP_OFF(t0)
    //bne     a0, a1, 1b
    nop
#endif

    PRINTSTR("NODE 0 MEMORY CONFIG BEGIN\r\n")
    move    msize, $0
    move    s3, $0
#ifdef  AUTO_DDR_CONFIG
    dli     s1, 0xf1f00000
#else
    //dli     s1, 0xc1a10400c1a10404  // use MC0, 2G SCS UDIMM
    dli     s1, 0xf0a31000f0a31000  // use both, 8G SCS RDIMM
#endif
#include "ddr_dir/loongson3A2000_ddr2_config.S"

    dli     t0, NODE0_CORE0_BUF0
    sw      msize, SP_OFF(t0)
    sd      s3, GP_OFF(t0)
#ifdef  CORE0_WAIT_ALL
    li      a1, NODE_MEM_INIT_DONE
    sw      a1, FN_OFF(t0)
#endif
    sync
    b       ALL_NODE_MEM_INIT_DONE
    nop

NODE1_INIT_MEM:
#if 1
    //wait Core0 for I2C release
    dli     t0, NODE0_CORE0_BUF0
1:
    li      a1, NODE_SCACHE_ENABLED
    lw      a0, SP_OFF(t0)
    bne     a0, a1, 1b
    nop
#endif

#ifdef  SEQ_INIT_MEM
    dli     t0, NODE0_CORE0_BUF0
    li      a1, NODE_MEM_INIT_DONE
1:
#if 1
    dli     a2, 0x1000
2:
    daddiu  a2, a2, -0x1
    bnez    a2, 2b
    nop
#endif
    lw      a0, FN_OFF(t0)
    bne     a0, a1, 1b
    nop
#endif

    PRINTSTR("NODE 1 MEMORY CONFIG BEGIN\r\n")
    move    msize, $0
    move    s3, $0
#ifdef  AUTO_DDR_CONFIG
    dli     s1, 0xf3f20001
#else
    //dli     s1, 0xc1a10400c1a10405  // use MC0, 2G SCS UDIMM
    dli     s1, 0xf0a31001f0a31001  // use both, 8G SCS RDIMM
#endif
#include "ddr_dir/loongson3A2000_ddr2_config.S"

    dli     t0, NODE1_CORE0_BUF0
    sw      msize, SP_OFF(t0)
    sd      s3, GP_OFF(t0)
#ifdef  CORE0_WAIT_ALL
    li      a1, NODE_MEM_INIT_DONE
    sw      a1, FN_OFF(t0)
#endif
    sync
    b       ALL_NODE_MEM_INIT_DONE
    nop

NODE2_INIT_MEM:
#if 1
    //wait Core4 for I2C release
    dli     t0, NODE1_CORE0_BUF0
1:
    li      a1, NODE_SCACHE_ENABLED
    lw      a0, SP_OFF(t0)
    bne     a0, a1, 1b
    nop
#endif

#ifdef  SEQ_INIT_MEM
    dli     t0, NODE1_CORE0_BUF0
    li      a1, NODE_MEM_INIT_DONE
1:
#if 1
    dli     a2, 0x1000
2:
    daddiu  a2, a2, -0x1
    bnez    a2, 2b
    nop
#endif
    lw      a0, FN_OFF(t0)
    bne     a0, a1, 1b
    nop
#endif

    PRINTSTR("NODE 2 MEMORY CONFIG BEGIN\r\n")
    move    msize, $0
    move    s3, $0
#ifdef  AUTO_DDR_CONFIG
    dli     s1, 0xf5f40002
#else
    //dli     s1, 0xc1a10400c1a10406  // use MC0, 2G SCS UDIMM
    dli     s1, 0xf0a31002f0a31002  // use both, 8G SCS RDIMM
#endif
#include "ddr_dir/loongson3A2000_ddr2_config.S"

    dli     t0, NODE2_CORE0_BUF0
    sw      msize, SP_OFF(t0)
    sd      s3, GP_OFF(t0)
#ifdef  CORE0_WAIT_ALL
    li      a1, NODE_MEM_INIT_DONE
    sw      a1, FN_OFF(t0)
#endif
    sync
    b       ALL_NODE_MEM_INIT_DONE
    nop

NODE3_INIT_MEM:
#if 1
    //wait Core0 for I2C release
    dli     t0, NODE2_CORE0_BUF0
1:
    li      a1, NODE_SCACHE_ENABLED
    lw      a0, SP_OFF(t0)
    bne     a0, a1, 1b
    nop
#endif

#ifdef  SEQ_INIT_MEM
    dli     t0, NODE2_CORE0_BUF0
    li      a1, NODE_MEM_INIT_DONE
1:
#if 1
    dli     a2, 0x1000
2:
    daddiu  a2, a2, -0x1
    bnez    a2, 2b
    nop
#endif
    lw      a0, FN_OFF(t0)
    bne     a0, a1, 1b
    nop
#endif

    PRINTSTR("NODE 3 MEMORY CONFIG BEGIN\r\n")
    move    msize, $0
    move    s3, $0
#ifdef  AUTO_DDR_CONFIG
    dli     s1, 0xf7f60003
#else
    //dli     s1, 0xc1a10400c1a10407  // use MC0, 2G SCS UDIMM
    dli     s1, 0xf0a31003f0a31003  // use both, 8G SCS RDIMM
#endif
#include "ddr_dir/loongson3A2000_ddr2_config.S"

    dli     t0, NODE3_CORE0_BUF0
    sw      msize, SP_OFF(t0)
    sd      s3, GP_OFF(t0)
#ifdef  CORE0_WAIT_ALL
    li      a1, NODE_MEM_INIT_DONE
    sw      a1, FN_OFF(t0)
#endif
    sync
    b       ALL_NODE_MEM_INIT_DONE
    nop




ALL_NODE_MEM_INIT_DONE:

    .set     mips64
    mfc0    t0, $15, 1
    .set     mips3
    andi    t0, 0xc
    dli     a0, 0
    beq     t0, a0, 2f
    nop
    //non boot core, wait all NODE mem init done, then clear its mailbox, and wait to jump to kernel
    dli     t0, NODE0_CORE0_BUF0 #buf of cpu0
    li      a1, ALL_CORE0_INIT_DONE
1:
    lw      a0, FN_OFF(t0)
    bne     a1, a0, 1b
    nop

    //clear self mailbox
    .set     mips64
    mfc0    t0, $15, 1
    .set     mips3
    andi    t0, t0, 0xc
    beq     t0, 0x0, 10f
    nop     
    beq     t0, 0x4, 11f;
    nop     
    beq     t0, 0x8, 12f;
    nop     
    beq     t0, 0xc, 13f;
    nop     
//no one arrive here
    //b       wait_to_jump_kernel
    b       waitforinit
    nop
10:
    dli     t1, NODE0_CORE0_BUF0;
    sw      $0, FN_OFF(t1)
    sw      $0, SP_OFF(t1)
    sd      $0, GP_OFF(t1)
    sync
    b       waitforinit
    nop
11:
    dli     t1, NODE1_CORE0_BUF0;
    sw      $0, FN_OFF(t1)
    sw      $0, SP_OFF(t1)
    sd      $0, GP_OFF(t1)
    sync
    b       waitforinit
    nop
12:
    dli     t1, NODE2_CORE0_BUF0;
    sw      $0, FN_OFF(t1)
    sw      $0, SP_OFF(t1)
    sd      $0, GP_OFF(t1)
    sync
    b       waitforinit
    nop
13:
    dli     t1, NODE3_CORE0_BUF0;
    sw      $0, FN_OFF(t1)
    sw      $0, SP_OFF(t1)
    sd      $0, GP_OFF(t1)
    sync
    b       waitforinit
    nop

2:
    //boot core, wait all core0 mem init done
    li      a1, NODE_MEM_INIT_DONE

    dli     t0, NODE0_CORE0_BUF0
1:
    lw      a0, FN_OFF(t0)
    bne     a0, a1, 1b
    nop




	/* check node 3 */
3:
    dli     t0, NODE3_CORE0_BUF0
1:
    lw      a0, FN_OFF(t0)
    bne     a0, a1, 1b
    nop

	/* check node 2 */
2:
    dli     t0, NODE2_CORE0_BUF0
1:
    lw      a0, FN_OFF(t0)
    bne     a0, a1, 1b
    nop

	/* check node 1 */
5:
    dli     t0, NODE1_CORE0_BUF0
1:
    lw      a0, FN_OFF(t0)
    bne     a0, a1, 1b
    nop
4:

    //all core0 mem init done
    //load msize and s3 from ALL 3 NODEs
    move    msize, $0
    move    s3, $0
    dli     t0, NODE0_CORE0_BUF0
    lw      a0, SP_OFF(t0)
    or      msize, msize, a0
    ld      a0, GP_OFF(t0)
    or      s3, s3, a0

	/* get node3 msize */
3:
    dli     t0, NODE3_CORE0_BUF0
    lw      a0, SP_OFF(t0)
    or      msize, msize, a0
    ld      a0, GP_OFF(t0)
    or      s3, s3, a0

	/* get node2 msize */
2:
    dli     t0, NODE2_CORE0_BUF0
    lw      a0, SP_OFF(t0)
    or      msize, msize, a0
    ld      a0, GP_OFF(t0)
    or      s3, s3, a0

	/* get node1 msize */
1:
    dli     t0, NODE1_CORE0_BUF0
    lw      a0, SP_OFF(t0)
    or      msize, msize, a0
    ld      a0, GP_OFF(t0)
    or      s3, s3, a0
4:
    PRINTSTR("\r\nsystem msize = 0x")
    move    a0, msize
    bal     hexserial
    nop
    PRINTSTR("\r\nsystem s3 = 0x")
    dsrl    a0, s3, 32
    bal     hexserial
    nop
    move    a0, s3
    bal     hexserial
    nop
    PRINTSTR("\r\n")

    //set all core0 init done signal
    dli     t0, NODE0_CORE0_BUF0
    sw      $0, SP_OFF(t0)
    sd      $0, GP_OFF(t0)
    li      a1, ALL_CORE0_INIT_DONE
    sw      a1, FN_OFF(t0)
    sync
    b       MEM_INIT_DONE
    nop
