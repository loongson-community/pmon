################## DMA memory-to-memory test #####################
/*
 *  This test is used for detecting HT controller's rx_buff bug.
 *  This bug may cause system hang when the first DMA & IRQ request emit.
 *  DMA read & write can detect this bug if operation not complete.
 *
 *  ch5 & ch6 are ch0 & ch1 in DMAC2 indeed.
 *
 *  Fixme: DMAC1 is not work in this platform, and channel4 in DMAC2 either.
 *         It semms memory-to-memory mode must use the channel4,5 if use DMAC2.
 *         Now I have to use channel5,6 to emulate memory-to-memory operation,
 *         Although the result is wrong, DMA read & write commands are done.
 */

#define DMA2_CMD_REG		0xD0 /* w */
#define DMA2_STATUS_REG		0xD0 /* r */
#define DMA2_REQ_REG		0xD2
#define DMA2_MASK_REG		0xD4
#define DMA2_MODE_REG		0xD6
#define DMA2_CLEAR_REG		0xD8
#define DMA2_MASK_ALL_REG	0xDE

#define CH5_PAGE_REG	0x8B
#define CH5_ADDR_REG	0xC4
#define CH5_COUNT_REG	0xC6

#define CH6_PAGE_REG	0x89
#define CH6_ADDR_REG	0xC8
#define CH6_COUNT_REG	0xCA

/*
 * This function is same as linux_outb_p in C code
 * Notice:
 *	    port ---  should be limited in 16 bits
 *	    val  ---  8 bits
 *          this code use macro BONITO_PCIIO_BASE_VA defined in bonito.h
 *	
 *	    v0 & v1 will be change if after function executing
 */
#define mips_outb_p(val, port)\
	li	v0, (val);\
	li	v1, BONITO_PCIIO_BASE_VA + (port);\
	sb	v0, 0(v1);\
	nop;\
	nop;\
	nop;\
	nop;\
	nop;\
	nop;\
	nop

#define mips_inb_p(port)\
	li	v1, BONITO_PCIIO_BASE_VA + (port);\
	lb	v0, 0(v1);\
	nop;\
	nop;\
	nop;\
	nop;\
	nop;\
	nop;\
	nop

#define mips_outw_p(val, port)\
	li	v0, (val);\
	li	v1, BONITO_PCIIO_BASE_VA + (port);\
	sw	v0, 0(v1);\
	nop;\
	nop;\
	nop;\
	nop;\
	nop;\
	nop;\
	nop

#define mips_inw_p(port)\
	li	v1, BONITO_PCIIO_BASE_VA + (port);\
	lw	v0, 0(v1);\
	nop;\
	nop;\
	nop;\
	nop;\
	nop;\
	nop;\
	nop


/* some code I don't know, but needed by DMA */

#define AX_INDXC  0
#define AX_INDXP  1
#define AXCFG     2
#define ABCFG     3
       
#define AB_INDX	0xCD8 
#define AB_DATA	0xCDC

#define alink_ab_indx(type, reg, mask, val)\
	mips_outw_p((type & 3) << 30 | reg, AB_INDX);\
	mips_inw_p(AB_DATA);\
	li	a0, ~mask;\
	li	a1, val;\
	and	v0, v0, a0;\
	or	a0, v0, a1;\
	mips_outw_p(((type & 3) << 30 | reg), AB_INDX);\
	li	v1, BONITO_PCIIO_BASE_VA + AB_DATA;\
	sb	a0, 0(v1);\
	nop

#define alink_ax_indx(type, reg, mask, val)\
	mips_outw_p(type << 30 | type << 3 | 0x30, AB_INDX);\
	mips_outw_p(reg, AB_INDX);\
	mips_outw_p(type << 30 | type << 3 | 0x34, AB_INDX);\
	mips_inw_p(AB_DATA);\
	li	a0, ~mask;\
	li	a1, val;\
	and	v0, v0, a0;\
	or	a0, v0, a1;\
	mips_outw_p(type << 30 | type << 3 | 0x30, AB_INDX);\
	mips_outw_p(reg, AB_INDX);\
	mips_outw_p(type << 30 | type << 3 | 0x34, AB_INDX);\
	li	v1, BONITO_PCIIO_BASE_VA + AB_DATA;\
	sb	a0, 0(v1);\
	nop

#define CH0		0
#define CH1		1
#define CH2		2
#define CH3		3

#define SINGLE_MODE	1
#define BLOCK_MODE	2
#define WRITE_MODE	1
#define READ_MODE	2

#define MODE_SHIFT	6
#define TRANSFER_TYPE_SHIFT	2
#define CHANNEL_SELECT_SHIFT	0

#define CH5_MODE	((BLOCK_MODE << MODE_SHIFT) | \
			 (WRITE_MODE << TRANSFER_TYPE_SHIFT) | \
			 (CH1 << CHANNEL_SELECT_SHIFT))

#define CH6_MODE	((BLOCK_MODE << MODE_SHIFT) | \
			 (READ_MODE << TRANSFER_TYPE_SHIFT) | \
			 (CH2 << CHANNEL_SELECT_SHIFT))

#define SRC_ADDR	0x00000000 //(bus address, 0x8000_0000 in cpu's eyes)
#define DST_ADDR	0x00000040 //(bus address, 0x8000_0040 in cpu's eyes)
#define TRANSFER_LEN	0x10

/* this procedure only use 5 regs(a0, a1, v0, v1, ra)*/
dma_test:
	############ prepare for DMA testing ###########
	/* set A-link bridge register address base which I can't understand */
	dli	a1, 0x90000efdfe00a0f0
	li	a0, 0xcd8
	sw	a0, 0(a1)

	dli	a1, 0x90000efdfe000004
	lw	a0, 0(a1)
	ori	a0, a0, 7
	sw	a0, 0(a1)

	dli	a1, 0x90000efdfe000090
	li	a0, 0xffffffff
	sw	a0, 0(a1)

	/* I can't find any descrition about 0xcd8 & 0xcdc in AMD documents */
	alink_ab_indx(AXCFG, 0x4, (1<<2), (1<<2))
	alink_ax_indx(AX_INDXC, 0x21, 0xff, 0)

	############ begin to test DMA ###########
	/* enable all channel on DMA2 */
	mips_outb_p(0, DMA2_MASK_ALL_REG)

	/* mask ch5 & ch6 before initialize */
	mips_outb_p(4|1, DMA2_MASK_REG)
	mips_outb_p(4|2, DMA2_MASK_REG)

	/* clear any transfer which are currently executing */
	mips_outb_p(0, DMA2_CLEAR_REG)
	mips_outb_p(0, DMA2_CLEAR_REG)

	/* setting ch5 & ch6 mode */
	mips_outb_p(CH5_MODE, DMA2_MODE_REG)
	mips_outb_p(CH6_MODE, DMA2_MODE_REG)

	/* Notice: DMAC2 transfer is base on 2-bytes, 
	 * address and length is not in unit of byte */

	/* setting transfer read address*/
	mips_outb_p((SRC_ADDR>>1) & 0xff, CH6_ADDR_REG)
	mips_outb_p((SRC_ADDR>>9) & 0xff, CH6_ADDR_REG)
	mips_outb_p((SRC_ADDR>>17) & 0xff, CH6_PAGE_REG)

	/* setting transfer write address */
	mips_outb_p((DST_ADDR>>1) & 0xff, CH5_ADDR_REG)
	mips_outb_p((DST_ADDR>>9) & 0xff, CH5_ADDR_REG)
	mips_outb_p((DST_ADDR>>17) & 0xff, CH5_PAGE_REG)

	/* send the tranfer length, assume length is not great than 256 bytes */
	mips_outb_p((TRANSFER_LEN >> 1) - 1, CH5_COUNT_REG)
	mips_outb_p(0, CH5_COUNT_REG)
	mips_outb_p((TRANSFER_LEN >> 1) - 1, CH6_COUNT_REG)
	mips_outb_p(0, CH6_COUNT_REG)

	/* enable ch5 & ch6*/
	mips_outb_p(CH2, DMA2_MASK_REG)
	mips_outb_p(CH1, DMA2_MASK_REG)

	/* send the request */
	mips_outb_p((4 | CH2), DMA2_REQ_REG)
	mips_outb_p((4 | CH1), DMA2_REQ_REG)

	/* start this transfer*/
	mips_outb_p(7, DMA2_CMD_REG)

	/* wait until DMA done or HT bus hang */
1:
	mips_inb_p(DMA2_STATUS_REG)
	li	a0, ((1 << CH1) | (1 << CH2))
	bne	v0, a0, 1b
	nop

	/* check bus is hang or not */
	mips_inb_p(0x70)

	/* jr	ra */
	nop
