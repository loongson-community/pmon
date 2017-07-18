#include <stdio.h>
#include <string.h>
#include <pmon.h>
#include <machine/types.h>
#include <sys/malloc.h>
#include <asm.h>

#define NAND_CMD       (*(volatile unsigned int *)0xbfee0000)
#define ADDR_C         (*(volatile unsigned int *)0xbfee0004)
#define ADDR_R         (*(volatile unsigned int *)0xbfee0008)
#define NAND_TIMING    (*(volatile unsigned int *)0xbfee000C)
#define ID_L           (*(volatile unsigned int *)0xbfee0010)
#define STATUS         (*(volatile unsigned int *)0Xbfee0014)
#define NAND_PARAMETER (*(volatile unsigned int *)0xbfee0018)
#define NAND_OP_NUM    (*(volatile unsigned int *)0xbfee001c)
#define CS_RDY_MAP     (*(volatile unsigned int *)0xbfee0020)
#define DMA_ACCESS     (*(volatile unsigned int *)0xbfee0040)

#if 1

static unsigned int wait_ready(void);
static unsigned int wait_all_ready(void);
static void udelay(int us)
{
	while (us--) ;
}

static unsigned int page_program(unsigned char *src_addr,
				 unsigned long nand_addr, unsigned int count)
{
	int i;
	unsigned int tmp_count, other_data;

	ADDR_C = nand_addr & 0xfff;
	ADDR_R = nand_addr >> 12;
	NAND_OP_NUM = count / 4;
	NAND_CMD = (0x1 << 8) | (0x1 << 2);
	NAND_CMD = (0x1 << 8) | (0x1 << 2) | 0x1;

	for (i = 0; i < count / 4; i++) {
		DMA_ACCESS = *((unsigned int *)src_addr + i);
	}
	if ((count % 4) != 0) {
		other_data = 0;
		tmp_count = (count - count % 4 - 1);
		for (i = 0; i < count % 4; i++) {
			other_data = (other_data << 8) | 
					(*(src_addr + tmp_count + i));
		}

		DMA_ACCESS = other_data;
	}
	wait_all_ready();
	return 0;
}

static unsigned int nand_read(unsigned char *dist_addr, unsigned long nand_addr,
			      unsigned int count)
{
	int i;
	unsigned int other_data, tmp_count;

	ADDR_C = nand_addr & 0xfff;
	ADDR_R = nand_addr >> 12;
	NAND_OP_NUM = count / 4;
	NAND_CMD = (0x1 << 8) | (0x1 << 1);
	NAND_CMD = (0x1 << 8) | (0x1 << 1) | 0x1;
	for (i = 0; i < count / 4; i++) {
		*((unsigned int *)dist_addr + i) = DMA_ACCESS;
	}
	if ((count % 4) != 0) {
		other_data = DMA_ACCESS;
		tmp_count = (count - count % 4 - 1);
		for (i = 0; i < count % 4; i++) {
			*(dist_addr + tmp_count + i) =
			    (other_data >> (i * 8)) & 0xff;
		}
	}

	wait_all_ready();
	return 0;

}

#define __ww(addr,val)  *((volatile unsigned int*)(addr)) = (val)
#define __rw(addr,val)  val =  *((volatile unsigned int*)(addr))

#if 1
#define VT_TO_PHY(x)      ((x) & 0x0fffffff| 0x40000000)

#define NAND_REG_BASE   0xbfee0000
#define NAND_DEV        0x1fee0040

#define DMA_DESP        0x84001000
#define DMA_DESP_ORDER  0x40800008	// bit3 :1 ,means dma start; otherwize,
#define DMA_ASK_ORDER   0x40800004

#define DDR_PHYADDR     0x42400000
#define DDR_PHY         0x42400000
#define DDR_ADDR        0x82400000

#define NAND_BASE           0xbfee0000
#define DMA_ACCESS_ADDR     0x1fee0040
#define ORDER_REG_ADDR      0xbfd00100
#endif

void nandread(unsigned char *dist_addr, unsigned int nand_addr,
	      unsigned int count)
{				/*cmd addr(L,page num) timing op_num(byte) */

	unsigned int cmd, val, pagenum, column;
	unsigned int buf;
	unsigned int addr;	// base on byte

	cmd = 0x103;

	/*      dma configure */
	__ww(DMA_DESP, 0);	// means next descriptor address, not used
	__ww(DMA_DESP + 0x4, VT_TO_PHY(((unsigned int)dist_addr)));
	__ww(DMA_DESP + 0x8, NAND_DEV);
	__ww(DMA_DESP + 0xc, (count) / 4);
	__ww(DMA_DESP + 0x10, 0x0);
	__ww(DMA_DESP + 0x14, 0x1);
	__ww(DMA_DESP + 0x18, 0x0000);	// bit 12 == 1'b1: read ddr write dev;bit 12 = 1'b0, read dev to write ddr
	__ww(ORDER_REG_ADDR, 0x10);
	udelay(1500);

	__ww(ORDER_REG_ADDR, (VT_TO_PHY(DMA_DESP) | 0x8));
	__ww(NAND_REG_BASE + 0x0c, 0x40c);	// set block number(erase unit) (204,188)
	__ww(NAND_REG_BASE + 0x1c, count);	// set block number(erase unit) (204,188)
	__ww(NAND_REG_BASE + 0x0, 0x0);
	__ww(NAND_REG_BASE + 0x0, 0x40);

	__ww(NAND_REG_BASE + 0x4, nand_addr % 2048);	// set address in a page; no use in erase operation
	__ww(NAND_REG_BASE + 0x8, nand_addr / 2048);	// set address index pages

	__ww(NAND_REG_BASE + 0x0, cmd & (~0xff));
	__ww(NAND_REG_BASE + 0x0, cmd);
	udelay(30000);
	buf = 0xaabbccdd;
	val = 0x0;

	__rw(NAND_REG_BASE + 0x0, val);
	while ((val & 0x400) != 0x400)	// operation done
	{
		udelay(150);
		__rw(NAND_REG_BASE, val);
	}
	__rw(NAND_REG_BASE, val);

}

void nandread_ecc(unsigned char *dist_addr, unsigned int nand_addr,
		  unsigned int count)
{				/*cmd addr(L,page num) timing op_num(byte) */

	unsigned int cmd, val, pagenum, column;
	unsigned int buf;
	unsigned int addr;	// base on byte
	int i;
	cmd = 0x903;

	/*      dma configure */
	__ww(DMA_DESP, 0);	// means next descriptor address, not used
	__ww(DMA_DESP + 0x4, VT_TO_PHY(((unsigned int)dist_addr)));
	__ww(DMA_DESP + 0x8, NAND_DEV);
	__ww(DMA_DESP + 0xc, (count) / 4);
	__ww(DMA_DESP + 0x10, 0x0);
	__ww(DMA_DESP + 0x14, 0x1);
	__ww(DMA_DESP + 0x18, 0x0000);	// bit 12 == 1'b1: read ddr write dev;bit 12 = 1'b0, read dev to write ddr
	__ww(ORDER_REG_ADDR, 0x10);
	udelay(1500);

	__ww(ORDER_REG_ADDR, (VT_TO_PHY(DMA_DESP) | 0x8));
	__ww(NAND_REG_BASE + 0x0c, 0x40b);	// set block number(erase unit) (204,188)
	__ww(NAND_REG_BASE + 0x1c, (count / 188) * 204);	// set block number(erase unit) (204,188)
	__ww(NAND_REG_BASE + 0x0, 0x0);
	__ww(NAND_REG_BASE + 0x0, 0x40);

	__ww(NAND_REG_BASE + 0x4, nand_addr % 2048);	// set address in a page; no use in erase operation
	__ww(NAND_REG_BASE + 0x8, nand_addr / 2048);	// set address index pages

	__ww(NAND_REG_BASE + 0x0, cmd & (~0xff));
	__ww(NAND_REG_BASE + 0x0, cmd);
	udelay(30000);
	buf = 0xaabbccdd;
	val = 0x0;

	__rw(NAND_REG_BASE + 0x0, val);
	while ((val & 0x400) != 0x400)	// operation done
	{
		udelay(150);
		__rw(NAND_REG_BASE, val);
	}

}

void nandread_spare(unsigned char *dist_addr, unsigned int nand_addr,
		    unsigned int count)
{				/*cmd addr(L,page num) timing op_num(byte) */

	unsigned int cmd, val, pagenum, column;
	unsigned int buf;
	unsigned int addr;	// base on byte
	int i = 0;
	cmd = 0x203;		//spare

	/*      dma configure */
	__ww(DMA_DESP, 0);	// means next descriptor address, not used
	__ww(DMA_DESP + 0x4, VT_TO_PHY(((unsigned int)dist_addr)));
	__ww(DMA_DESP + 0x8, NAND_DEV);
	__ww(DMA_DESP + 0xc, (count) / 4);
	__ww(DMA_DESP + 0x10, 0x0);
	__ww(DMA_DESP + 0x14, 0x1);
	__ww(DMA_DESP + 0x18, 0x0000);	// bit 12 == 1'b1: read ddr write dev;bit 12 = 1'b0, read dev to write ddr
	__ww(ORDER_REG_ADDR, 0x10);
	udelay(1500);

	__ww(ORDER_REG_ADDR, (VT_TO_PHY(DMA_DESP) | 0x8));

	__ww(NAND_REG_BASE + 0x0c, 0x40c);	// set block number(erase unit) (204,188)
	__ww(NAND_REG_BASE + 0x1c, count);	// set block number(erase unit) (204,188)
	__ww(NAND_REG_BASE + 0x0, 0x0);
	__ww(NAND_REG_BASE + 0x0, 0x40);

	__ww(NAND_REG_BASE + 0x4, (nand_addr % 2048) | (0x1 << 11));	// set address in a page; no use in erase operation
	__ww(NAND_REG_BASE + 0x8, nand_addr / 2048);	// set address index pages

	__ww(NAND_REG_BASE + 0x0, cmd & (~0xff));
	__ww(NAND_REG_BASE + 0x0, cmd);

	udelay(30000);
	buf = 0xaabbccdd;
	val = 0x0;

	__rw(NAND_REG_BASE + 0x0, val);
	while ((val & 0x400) != 0x400)	// operation done
	{
		udelay(150);
		__rw(NAND_REG_BASE, val);

	}

}

void nandwrite(unsigned char *src_addr, unsigned int nand_addr,
	       unsigned int count)
{				/*cmd addr(L,page num) timing op_num(byte) */

	unsigned int cmd, val, pagenum, column;
	unsigned int buf;
	unsigned int addr;	// base on byte

	cmd = 0x105;

	/*      dma configure */
	__ww(DMA_DESP, 0);	// means next descriptor address, not used
	__ww(DMA_DESP + 0x4, VT_TO_PHY(((unsigned int)src_addr)));
	__ww(DMA_DESP + 0x8, NAND_DEV);
	__ww(DMA_DESP + 0xc, (count) / 4);
	__ww(DMA_DESP + 0x10, 0x0);
	__ww(DMA_DESP + 0x14, 0x1);
	__ww(DMA_DESP + 0x18, 0x1000);	// bit 12 == 1'b1: read ddr write dev;bit 12 = 1'b0, read dev to write ddr
	__ww(ORDER_REG_ADDR, 0x10);
	udelay(30);

	__ww(ORDER_REG_ADDR, (VT_TO_PHY(DMA_DESP) | 0x8));
	__ww(NAND_REG_BASE + 0x1c, count);	// set block number(erase unit) (204,188)
	__ww(NAND_REG_BASE + 0x0, 0x0);
	__ww(NAND_REG_BASE + 0x0, 0x40);

	__ww(NAND_REG_BASE + 0x4, nand_addr % 2048);	// set address in a page; no use in erase operation
	__ww(NAND_REG_BASE + 0x8, nand_addr / 2048);	// set address index pages

	__ww(NAND_REG_BASE + 0x0, cmd & (~0xff));
	__ww(NAND_REG_BASE + 0x0, cmd);
	udelay(800);

	__rw(NAND_REG_BASE + 0x0, val);
	while ((val & 0x400) != 0x400)	// operation done
	{
		udelay(50);
		__rw(NAND_REG_BASE, val);
	}

}

void nandwrite_ecc(unsigned char *src_addr, unsigned int nand_addr,
		   unsigned int count)
{				/*cmd addr(L,page num) timing op_num(byte) */

	unsigned int cmd, val, pagenum, column;
	unsigned int buf;
	unsigned int addr;	// base on byte

	cmd = 0x1105;

	/*      dma configure */
	__ww(DMA_DESP, 0);	// means next descriptor address, not used
	__ww(DMA_DESP + 0x4, VT_TO_PHY(((unsigned int)src_addr)));
	__ww(DMA_DESP + 0x8, NAND_DEV);
	__ww(DMA_DESP + 0xc, (count) / 4);
	__ww(DMA_DESP + 0x10, 0x0);
	__ww(DMA_DESP + 0x14, 0x1);
	__ww(DMA_DESP + 0x18, 0x1000);	// bit 12 == 1'b1: read ddr write dev;bit 12 = 1'b0, read dev to write ddr
	__ww(ORDER_REG_ADDR, 0x10);
	udelay(30);

	__ww(ORDER_REG_ADDR, (VT_TO_PHY(DMA_DESP) | 0x8));
	__ww(NAND_REG_BASE + 0x1c, (count / 188) * 204);	// set block number(erase unit) (204,188)
	__ww(NAND_REG_BASE + 0x0, 0x0);
	__ww(NAND_REG_BASE + 0x0, 0x40);

	__ww(NAND_REG_BASE + 0x4, nand_addr % 2048);	// set address in a page; no use in erase operation
	__ww(NAND_REG_BASE + 0x8, nand_addr / 2048);	// set address index pages

	__ww(NAND_REG_BASE + 0x0, cmd & (~0xff));
	__ww(NAND_REG_BASE + 0x0, cmd);
	udelay(800);

	__rw(NAND_REG_BASE + 0x0, val);
	while ((val & 0x400) != 0x400)	// operation done
	{
		udelay(50);
		__rw(NAND_REG_BASE, val);
	}

}

//spare

void nandwrite_spare(unsigned char *src_addr, unsigned int nand_addr,
		     unsigned int count)
{				/*cmd addr(L,page num) timing op_num(byte) */

	unsigned int cmd, val, pagenum, column;
	unsigned int buf;
	unsigned int addr;	// base on byte

	cmd = 0x205;

	/*      dma configure */
	__ww(DMA_DESP, 0);	// means next descriptor address, not used
	__ww(DMA_DESP + 0x4, VT_TO_PHY(((unsigned int)src_addr)));
	__ww(DMA_DESP + 0x8, NAND_DEV);
	__ww(DMA_DESP + 0xc, (count) / 4);
	__ww(DMA_DESP + 0x10, 0x0);
	__ww(DMA_DESP + 0x14, 0x1);
	__ww(DMA_DESP + 0x18, 0x1000);	// bit 12 == 1'b1: read ddr write dev;bit 12 = 1'b0, read dev to write ddr
	__ww(ORDER_REG_ADDR, 0x10);
	udelay(30);

	__ww(ORDER_REG_ADDR, (VT_TO_PHY(DMA_DESP) | 0x8));
	__ww(NAND_REG_BASE + 0x1c, count);	// set block number(erase unit) (204,188)
	__ww(NAND_REG_BASE + 0x0, 0x0);
	__ww(NAND_REG_BASE + 0x0, 0x40);

	__ww(NAND_REG_BASE + 0x4, (nand_addr % 2048) | (1 << 11));	// set address in a page; no use in erase operation
	__ww(NAND_REG_BASE + 0x8, nand_addr / 2048);	// set address index pages

	__ww(NAND_REG_BASE + 0x0, cmd & (~0xff));
	__ww(NAND_REG_BASE + 0x0, cmd);
	udelay(800);

	__rw(NAND_REG_BASE + 0x0, val);
	while ((val & 0x400) != 0x400)	// operation done
	{
		udelay(50);
		__rw(NAND_REG_BASE, val);
	}

}

#if 0
static void nandwrite(unsigned char *src_addr, unsigned int nand_addr,
		      unsigned int count)
{				/*cmd addr(L,page num) timing op_num(byte) */
	int i;
	for (i = 0; i < count / 4; i++) {
		nandwrite_word(src_addr + i * 4, nand_addr + i * 4, 4);
	}
}

static void nandread(unsigned char *dist_addr, unsigned int nand_addr,
		     unsigned int count)
{				/*cmd addr(L,page num) timing op_num(byte) */
	int i;
	for (i = 0; i < count / 4; i++) {
		nandread_word(dist_addr + i * 4, nand_addr + i * 4, 4);
	}
}
#endif

int block_erase(unsigned long block_num, unsigned int count)
{
	unsigned int block_addr = 0;
	int ret;

	block_addr = block_num * (0x1 << 6);

	ADDR_R = block_addr;
	NAND_OP_NUM = count;
	NAND_CMD = ((0x1 << 4) | (0x1 << 3) | (0x1 << 8));
	NAND_CMD = ((0x1 << 8) | (0x1 << 4) | (0x1 << 3) | (0x1));

	ret = wait_all_ready();
	return ret;
}

#define SET_BADBLOCK_AREA ((unsigned char *)0x85100000)
#define BLOCK_SIZE 0x20000
#define BLOCK_SIZE_ECC (204*10*64)

unsigned int get_addr_jump_badblocks(unsigned int nand_addr,
				     unsigned char *badblock_flag)
{
	unsigned int cur_nand_addr, block_offset, ret_nand_addr;
	int read_count;
	int badblock_count;

	cur_nand_addr = 0;
	badblock_count = 0;

	while (1) {
		if (cur_nand_addr >= (nand_addr & (~(BLOCK_SIZE - 1)))) {
			break;
		}

		nandread_spare(badblock_flag, cur_nand_addr, 16);
		if (badblock_flag[0] != 0xff) {
			badblock_count++;
		}
		cur_nand_addr += BLOCK_SIZE;

	}

	ret_nand_addr = nand_addr + badblock_count * BLOCK_SIZE;
	return ret_nand_addr;
}

void nandwrite_set_badblock(unsigned char *src_addr, unsigned int nand_addr,
			    unsigned int count)
{

	int block_num;
	int state, is_badblock;
	int try_num, cur_count;
	int i;
	unsigned int cur_nand_addr, write_count;
	int block_offset;
	unsigned char *cur_src_addr;

	cur_src_addr = src_addr;
	cur_nand_addr = get_addr_jump_badblocks(nand_addr, SET_BADBLOCK_AREA);
	cur_count = count;
	is_badblock = 1;

	for (i = 1; i < 16; i++) {
		SET_BADBLOCK_AREA[i] = 0xff;
	}

	while (1) {
		try_num = 5;
		block_num = cur_nand_addr / (2048 * 64);
		block_offset =
		    cur_nand_addr - cur_nand_addr & (~(BLOCK_SIZE - 1));
		write_count = BLOCK_SIZE - block_offset;

		if (write_count > cur_count) {
			write_count = cur_count;
		}

		while (try_num--) {
			state = block_erase(block_num, 1);
			if (state >= 0) {
				is_badblock = 0;
				break;
			}
		}

		if (state < 0) {
			is_badblock = 1;
		}
		if (is_badblock == 1) {
			printf("badblock:%d\n", block_num);
			SET_BADBLOCK_AREA[0] = 0x00;
			nandwrite_spare(SET_BADBLOCK_AREA,
					cur_nand_addr & (~(BLOCK_SIZE - 1)),
					16);
			cur_nand_addr += BLOCK_SIZE;
		} else {
			nandwrite(cur_src_addr, cur_nand_addr, write_count);
			cur_src_addr += write_count;
			cur_nand_addr += write_count;
			cur_count -= write_count;
			if (cur_count == 0) {
				return 0;
			}
		}
	}

}

void nandread_check_badblock(unsigned char *dist_addr, unsigned int nand_addr,
			     unsigned int count)
{

	int block_num;
	int state, is_badblock;
	int try_num, cur_count;
	int i;
	unsigned int cur_nand_addr, read_count;
	int block_offset;
	unsigned char *cur_dist_addr;

	cur_dist_addr = dist_addr;

	cur_nand_addr = get_addr_jump_badblocks(nand_addr, dist_addr);
	cur_count = count;
	is_badblock = 1;

	while (1) {
		block_num = cur_nand_addr / (2048 * 64);
		block_offset =
		    cur_nand_addr - cur_nand_addr & (~(BLOCK_SIZE - 1));
		read_count = BLOCK_SIZE - block_offset;

		if (read_count > cur_count) {
			read_count = cur_count;
		}

		nandread_spare(cur_dist_addr,
			       cur_nand_addr & (~(BLOCK_SIZE - 1)), 16);
		if (cur_dist_addr[0] != 0xff) {
			is_badblock = 1;
		} else {
			is_badblock = 0;
		}
		if (is_badblock == 1) {
			printf("badblock:%d\n", block_num);
			cur_nand_addr += BLOCK_SIZE;
		} else {
			nandread(cur_dist_addr, cur_nand_addr, read_count);
			cur_dist_addr += read_count;
			cur_nand_addr += read_count;
			cur_count -= read_count;
			if (cur_count == 0) {
				return 0;
			}
		}
	}

}

void nandwrite_set_badblock_ecc(unsigned char *src_addr, unsigned int nand_addr,
				unsigned int count)
{

	int block_num;
	int state, is_badblock;
	int try_num, cur_count;
	int i;
	unsigned int cur_nand_addr, write_count;
	int block_offset;
	unsigned char *cur_src_addr;

	cur_src_addr = src_addr;
	cur_nand_addr = get_addr_jump_badblocks(nand_addr, SET_BADBLOCK_AREA);
	cur_count = count;
	is_badblock = 1;

	for (i = 1; i < 16; i++) {
		SET_BADBLOCK_AREA[i] = 0xff;
	}

	while (1) {
		try_num = 5;
		block_num = cur_nand_addr / (2048 * 64);
		block_offset = cur_nand_addr & (BLOCK_SIZE - 1);
		write_count =
		    ((BLOCK_SIZE_ECC - (block_offset / 2048) * 204 * 10 -
		      (block_offset % 2048)) / 204) * 188;

		if (write_count > cur_count) {
			write_count = cur_count;
		}

		while (try_num--) {
			state = block_erase(block_num, 1);
			if (state >= 0) {
				is_badblock = 0;
				break;
			}
		}

		if (state < 0) {
			is_badblock = 1;
		}
		if (is_badblock == 1) {
			printf("badblock:%d\n", block_num);
			SET_BADBLOCK_AREA[0] = 0x00;
			nandwrite_spare(SET_BADBLOCK_AREA,
					cur_nand_addr & (~(BLOCK_SIZE - 1)),
					16);
			cur_nand_addr += BLOCK_SIZE;
		} else {
			nandwrite_ecc(cur_src_addr, cur_nand_addr, write_count);
			cur_src_addr += write_count;
			cur_count -= write_count;

			cur_nand_addr +=
			    (write_count / (188 * 10)) * 2048 +
			    ((write_count % (188 * 10)) / 188) * 204;
			if (((block_offset % 2048) +
			     ((write_count % (188 * 10)) / 188) * 204) >=
			    (204 * 10)) {
				cur_nand_addr += 8;
			}

			if (cur_count == 0) {
				return 0;
			}
		}
	}

}

void nandread_check_badblock_ecc(unsigned char *dist_addr,
				 unsigned int nand_addr, unsigned int count)
{

	int block_num, test_count;

	int state, is_badblock;
	int try_num, cur_count;
	int i;
	unsigned int cur_nand_addr, read_count, cur_spare_addr;
	int block_offset;
	unsigned char *cur_dist_addr;

	cur_dist_addr = dist_addr;

	cur_nand_addr = get_addr_jump_badblocks(nand_addr, dist_addr);
	cur_count = count;
	is_badblock = 1;

	while (1) {
		block_num = cur_nand_addr / (2048 * 64);
		block_offset = cur_nand_addr & (BLOCK_SIZE - 1);
		read_count =
		    ((BLOCK_SIZE_ECC - (block_offset / 2048) * 204 * 10 -
		      (block_offset % 2048)) / 204) * 188;

		if (read_count > cur_count) {
			read_count = cur_count;
		}

		cur_spare_addr =
		    (((unsigned int)cur_dist_addr) & (0xfffffff0)) + 0x10;

		udelay(80000);

		nandread_spare(cur_spare_addr,
			       cur_nand_addr & (~(BLOCK_SIZE - 1)), 16);
		if (((unsigned char *)cur_spare_addr)[0] != 0xff) {
			is_badblock = 1;
		} else {
			is_badblock = 0;
		}

		if (is_badblock == 1) {
			printf("badblock:%d\n", block_num);
			cur_nand_addr += BLOCK_SIZE;
		} else {
			nandread_ecc(cur_dist_addr, cur_nand_addr, read_count);
			cur_dist_addr += read_count;
			cur_count -= read_count;

			cur_nand_addr +=
			    (read_count / (188 * 10)) * 2048 +
			    ((read_count % (188 * 10)) / 188) * 204;
			if (((block_offset % 2048) +
			     ((read_count % (188 * 10)) / 188) * 204) >=
			    (204 * 10)) {
				cur_nand_addr += 8;
			}

			if (cur_count == 0) {
				return 0;
			}
		}
	}

}

static unsigned int wait_all_ready(void)
{
	int i = 0;

	while (((NAND_CMD) & (0x1 << 10)) == 0) {
		udelay(500);
		i++;
		if (i > 500) {
			return -1;
		}
	}

	return 0;
}

static unsigned int wait_ready(void)
{
	while (((NAND_CMD) & (0x1 << 25)) == 0) ;

	return 0;
}

unsigned int init_nand(void)
{
	NAND_CMD = (0x1 << 6) | (0x1 << 0);	//reset , main
	wait_all_ready();

	NAND_CMD = 0x0;

	return 0;
}

static unsigned int read_id()
{

}

static int cmp_mem(unsigned char *src_addr, unsigned char *dist_addr, int count)
{
	int i;
	for (i = 0; i < count; i++) {

		if (*(src_addr + i) != *(dist_addr + i)) {
			printf("dist != src\n");
			return -1;
		}
	}
	printf("dist == src\n");

}

#define NAND_TEST_R_ADDR ((volatile unsigned char *)0x85000000)
#define NAND_TEST_W_ADDR ((volatile unsigned char *)0x86000000)
#define NAND_TEST_SHOW   ((volatile unsigned int  *)0x85000000)

static int nand_test_read_write(void)
{

#if 1
	printf("nand init ecc  0x%x\n", NAND_CMD);
	init_nand();

	printf("nand block erase 0x%x\n", NAND_CMD);
	//block_erase(0,4);

	printf("nand program  0x%x\n", NAND_CMD);
	//nandwrite(NAND_TEST_W_ADDR,0x0,0x80000);
	//nandwrite_set_badblock(NAND_TEST_W_ADDR,0x0,0x80000);
	nandwrite_set_badblock_ecc(NAND_TEST_W_ADDR, 0, 1880 * 64 * 4);

	printf("nand read 0x%x\n", NAND_CMD);
	//nandread(NAND_TEST_R_ADDR ,0x0,0x80000);
	//nandread_check_badblock(NAND_TEST_R_ADDR ,0x0,0x80000);
	nandread_check_badblock_ecc(NAND_TEST_R_ADDR, 0, 1880 * 64 * 4);

	printf("nand cmp\n");
	//cmp_mem(NAND_TEST_R_ADDR ,NAND_TEST_W_ADDR,0x80000);
	cmp_mem(NAND_TEST_R_ADDR, NAND_TEST_W_ADDR, 1880 * 64 * 4);
	printf("nand test end \n");
#endif

	return 0;
}

#endif
int cmd_myshow __P((int, char *[]));

int cmd_myshow(ac, av)
int ac;
char *av[];
{

	nand_test_read_write();
	return (1);
}

const Optdesc cmd_myshow_opts[] = {
	{"-a", "test nand opt"},
	{0},
};

static const Cmd Cmds[] = { {"test nand read write"},
{"test_nand_r_w", "[-a]", cmd_myshow_opts, "test nand read and write",
 cmd_myshow, 1, 99, 0},
{0, 0},

};

static void init_cmd __P((void)) __attribute__ ((constructor));

static void init_cmd()
{
	cmdlist_expand(Cmds, 1);
}
