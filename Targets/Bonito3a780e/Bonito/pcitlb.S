/*smh : pcitlb.S
        used to map address accessing for kuseg to address accessing for system memory beyond low 256M memory
        used to map address accessing for kseg2 to address accessing for pci memory which are from 0x40000000
*/

/***********************************************************************************************************/
/**************************************** MAP KSEG2 ********************************************************/
/** MAP virtual address: 0xc0000000~ 0xffffffff to physical address:  0x40000000~ 0x7fffffff for PCI-MEM  **/
/***********************************************************************************************************/

	li	t0, 0x40000000

	.set mips64
	dmfc0	t1, COP_0_TLB_PG_MASK, 1
	or	t1, t1, t0
	dmtc0	t1, COP_0_TLB_PG_MASK, 1

	dmfc0	t0, COP_0_CONFIG, 3
	ori	t0, t0, 0x80
	dmtc0	t0, COP_0_CONFIG, 3
	.set mips3

	li	t0, 0x01ffe000
	mtc0	t0, COP_0_TLB_PG_MASK    # 16MB page

	li	t0, 31

	li	t3,  0xc0000000   # entry_hi
	#dli	t4, (0x00000e0040000000 >> 6)|0x17   # entry_lo, uncached, valid, dirty, global
	#li	t4, 0x0e004000

	li	t4, 0x00004000
	#li	t4, 0x0efdf000

	.set mips64
	dsll	t4, t4, 10
	.set mips3
	ori	t4, t4, 0x17

	li	t5, (0x1000000>>6)            # 16M stride
	li	t6, 0x2000000             # VPN2 32M stride

	.set mips64
1:
	dmtc0	t3, COP_0_TLB_HI
	daddu	t3, t3, t6

	dmtc0	t4, COP_0_TLB_LO0
	daddu	t4, t4, t5
	dmtc0	t4, COP_0_TLB_LO1
	daddu	t4, t4, t5

	.set mips3

	mtc0	t0, COP_0_TLB_INDEX           # 16MB page
	nop
	nop
	nop
	nop
	nop
	tlbwi                             # random++

	bgez	t0, 1b
	addiu	t0, t0, -1


	li	t0, 32
	mtc0	t0, COP_0_TLB_WIRED    # 16MB page



/***********************************************************************************************************/
/**************************************** MAP KUSEG ********************************************************/
/** MAP virtual address: 0x40000000~ 0x7fffffff to physical address:  0x80000000~ 0xbfffffff for SYS-MEM  **/
/***********************************************************************************************************/

#ifdef CONFIG_GFXUMA /* for UMA mode. */

	li	t0, 0x80000000

	.set mips64
	dmfc0	t1, COP_0_TLB_PG_MASK, 1
	or	t1, t1, t0
	dmtc0	t1, COP_0_TLB_PG_MASK, 1

	dmfc0	t0, COP_0_CONFIG, 3
	ori	t0, t0, 0x80
	dmtc0	t0, COP_0_CONFIG, 3
	.set mips3

	li	t0, 0x01ffe000
	mtc0	t0, COP_0_TLB_PG_MASK    # 16MB page

	li	t0, 63

	li	t3,  0x40000000   # entry_hi
	#dli	t4, (0x00000e0040000000 >> 6)|0x17   # entry_lo, uncached, valid, dirty, global
	#li	t4, 0x0e004000

	li	t4, 0x00008000

	#li	t4, 0x0efdf000

	.set mips64
	dsll	t4, t4, 10
	.set mips3
	ori	t4, t4, 0x17

	li	t5, (0x1000000>>6)            # 16M stride
	li	t6, 0x2000000             # VPN2 32M stride

	.set mips64
	li	t7, 32
1:
	dmtc0	t3, COP_0_TLB_HI
	daddu	t3, t3, t6

	dmtc0	t4, COP_0_TLB_LO0
	daddu	t4, t4, t5
	dmtc0	t4, COP_0_TLB_LO1
	daddu	t4, t4, t5

	.set mips3

	mtc0	t0, COP_0_TLB_INDEX           # 16MB page
	nop
	nop
	nop
	nop
	nop
	tlbwi                             # random++

	#bnez	t0, 1b
	addiu	t0, t0, -1
	bge		t0, t7, 1b
	nop


	li	t0, 64
	mtc0	t0, COP_0_TLB_WIRED    # 16MB page

#endif
