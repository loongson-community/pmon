/*
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of  the GNU General  Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.
 *
 * Copyright (C) 2007 Lemote, Inc. & Institute of Computing Technology
 * Author: Fuxin Zhang, zhangfx@lemote.com
 */

/* the following code is exported to kernel, we need to be sure that there are
 * safe to be called by 64bit code, so we use the reserved the area 0x8ffc0000
 * as the new stack */

/* this is leaf function exported by PMON to kernel */
#include <regdef.h>
#include "mipsregs.h"
	.text
	.global poweroff_kernel
	.ent	poweroff_kernel
poweroff_kernel:
	.set	noreorder
	mfc0	t0, CP0_STATUS
	li	t1, 0xfffffffe
	and	t0, t1, t0
	mtc0	t0, CP0_STATUS
	jal	tgt_poweroff
	lui	sp, 0x8ffc
1:	b	1b
	nop
	.end	poweroff_kernel

	.global reboot_kernel
	.ent	reboot_kernel
reboot_kernel:
	mfc0	t0, CP0_STATUS
	li	t1, 0xfffffffe
	and	t0, t1, t0
	mtc0	t0, CP0_STATUS
	jal	tgt_reboot
	lui	sp, 0x8ffc
1:	b	1b
	nop
	.end	reboot_kernel

	/* asm trick following o32 abi */

	/* u64  __raw__readq(u64 addr)
	 * a0, a1 hold low 32 and high 32
	 * v0, v1 hold low 32 and high 32 of ret
	 */
	.text
	.global __raw__readq
	.ent	__raw__readq
	.set	mips3
__raw__readq:

	dsll32	a1, a1, 0
	dsll32	a0, a0, 0
	dsrl32	a0, a0, 0
	or	a0, a1, a0

	ld	v0, 0(a0)
	dsra32  v1, v0, 0
	jr	ra
	sll	v0, v0, 0
	.set	mips0
	.end	__raw__readq

	/* u64 __raw__writeq(u64 addr, u64 val)
	 * a0, a1 hold low 32 and high 32 of addr,
	 * a2, a2 hold low 32 and high 32 of val,
	 * v0, v1 hold low 32 and high 32 of ret
	 */

	.global __raw__writeq
	.set	mips3
	.ent	__raw__writeq
__raw__writeq:

	dsll32	a1, a1, 0
	dsll32	a0, a0, 0
	dsrl32	a0, a0, 0
	or	a0, a1, a0

	dsll32	a3, a3, 0
	dsll32	a2, a2, 0
	dsrl32	a2, a2, 0
	or	a2, a2, a3

	sd	a2, 0(a0)
	ld	v0, 0(a0)
	dsra32  v1, v0, 0
	jr	ra
	sll	v0, v0, 0
	.set	mips0
	.end	__raw__writeq

	/* u64  __raw__readw(u64 addr)
	 * a0, a1 hold low 32 and high 32
	 * v0     hold     32 of ret
	 */
	.text
	.global __raw__readw
	.ent	__raw__readw
	.set	mips3
__raw__readw:

	dsll32	a1, a1, 0
	dsll32	a0, a0, 0
	dsrl32	a0, a0, 0
	or	a0, a1, a0

	lw	v0, 0(a0)
//	dsra32  v1, v0, 0
	jr	ra
	sll	v0, v0, 0
	.set	mips0
	.end	__raw__readw

	/* u64 __raw__writeq(u64 addr, u64 val)
	 * a0, a1 hold low 32 and high 32 of addr,
	 * a2, a2 hold low 32 and high 32 of val,
	 * v0, v1 hold low 32 and high 32 of ret
	 */


	/* u64 __raw__writew(u64 addr, u32 val)
	 * a0, a1 hold low 32 and high 32 of addr,
	 * a2     hold 32 of val,
	 * v0     hold 32 of ret
	 */

	.global __raw__writew
	.set	mips3
	.ent	__raw__writew
__raw__writew:

	dsll32	a1, a1, 0
	dsll32	a0, a0, 0
	dsrl32	a0, a0, 0
	or	a0, a1, a0

//	dsll32	a3, a3, 0
	dsll32	a2, a2, 0
	dsrl32	a2, a2, 0
//	or	a2, a2, a3

	sw	a2, 0(a0)
	lw	v0, 0(a0)
//	dsra32  v1, v0, 0
	jr	ra
	sll	v0, v0, 0
	.set	mips0
	.end	__raw__writew
