/*
 * Copyright (c) 2003-2013 Broadcom Corporation
 *
 * Copyright (c) 2009-2010 Micron Technology, Inc.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */
#include<pmon.h>
#include<asm.h>
#include<machine/types.h>
#include<linux/mtd/mtd.h>
#include<linux/mtd/nand.h>
#include<linux/mtd/partitions.h>
#include<sys/malloc.h>
#include <sys/mbuf.h>
#include <linux/mtd/spinand.h>
#include <linux/spi.h>
#include <sys/time.h>
#include <linux/mtd/nand_bch.h>
#include <nand_bch.h>
#include <linux/bitops.h>

#define REG_STRENGTH		0xd0
#define dev_err(dev,msg...) printf(msg)
#define cond_resched tgt_clkpoll
#define jiffies ticks

#define devm_kzalloc(dev, size, flags) \
	({ \
          void  *info  = malloc(size,M_DEVBUF, M_DONTWAIT ); \
          if(info) memset(info, 0, size); \
          info; \
         })
#define dev_set_drvdata(...)

#define BUFSIZE (10 * 64 * 2048)
#define NAND_CMD_PARAM		0xec

/*
 * OOB area specification layout:  Total 32 available free bytes.
 */
#ifdef NOUSED_MTD_SPINAND_ONDIEECC
static int enable_hw_ecc;
static int enable_read_hw_ecc;
#endif
static struct nand_ecclayout spinand_oob_64 = {
	.eccbytes = 24,
	.eccpos = {
		   40, 41, 42, 43, 44, 45, 46, 47,
		   48, 49, 50, 51, 52, 53, 54, 55,
		   56, 57, 58, 59, 60, 61, 62, 63},
	.oobfree = {
		{.offset = 2,
		 .length = 38}}
};

static struct nand_ecclayout spinand_oob_128 = {
	.eccbytes = 48,
	.eccpos = {
		   80, 81, 82, 83, 84, 85, 86, 87,
		   88, 89, 90, 91, 92, 93, 94, 95,
		   96, 97, 98, 99, 100, 101, 102, 103,
		   104, 105, 106, 107, 108, 109, 110, 111,
		   112, 113, 114, 115, 116, 117, 118, 119,
		   120, 121, 122, 123, 124, 125, 126, 127},
	.oobfree = {
		{.offset = 2,
		 .length = 78}}
};

/*
 * spinand_cmd - to process a command to send to the SPI Nand
 * Description:
 *    Set up the command buffer to send to the SPI controller.
 *    The command buffer has to initized to 0
 */

int spinand_cmd(struct spi_device *spi, struct spinand_cmd *cmd)
{
	struct spi_message message;
	struct spi_transfer x[4];
	char cmdbuf[16];
	u8 dummy = 0xff;
	int ret;

	spi_message_init(&message);
	memset(x, 0, sizeof(x));

	x[0].len = 1;
	x[0].tx_buf = cmdbuf;
	cmdbuf[0] = cmd->cmd;

	if (cmd->n_addr) {
		x[0].len += cmd->n_addr;
		memcpy(&cmdbuf[1], cmd->addr, cmd->n_addr);
	}
	spi_message_add_tail(&x[0], &message);

	if (cmd->n_dummy) {
		x[2].len = cmd->n_dummy;
		x[2].tx_buf = &dummy;
		spi_message_add_tail(&x[2], &message);
	}

	if (cmd->n_tx) {
		x[3].len = cmd->n_tx;
		x[3].tx_buf = cmd->tx_buf;
		spi_message_add_tail(&x[3], &message);
	}

	if (cmd->n_rx) {
		x[3].len = cmd->n_rx;
		x[3].rx_buf = cmd->rx_buf;
		spi_message_add_tail(&x[3], &message);
	}

	ret = spi_sync(spi, &message);

	return ret;
}

/*
 * spinand_read_id- Read SPI Nand ID
 * Description:
 *    Read ID: read two ID bytes from the SPI Nand device
 */
static int spinand_read_id(struct spinand_info *info, u8 *id)
{
	int retval;
	u8 nand_id[3];
	struct spinand_cmd cmd = {0};

	cmd.cmd = CMD_READ_ID;
	cmd.n_rx = 3;
	cmd.rx_buf = &nand_id[0];

	retval = spinand_cmd(info->spi, &cmd);
	if (retval != 0) {
		printk("error %d reading id\n", retval);
		return retval;
	}

	printf("nand id is 0x%x 0x%x 0x%x\n", nand_id[0], nand_id[1],nand_id[2]);

	if(nand_id[0] == 0xc8 && nand_id[1] == 0xb4) {
		info->gd_ctype = 1;
		id[0] = nand_id[0];
		id[1] = nand_id[1];
	} else {
		id[0] = nand_id[1];
		id[1] = nand_id[2];
	}

	return 0;
}

/*
 * spinand_read_status- send command 0xf to the SPI Nand status register
 * Description:
 *    After read, write, or erase, the Nand device is expected to set the
 *    busy status.
 *    This function is to allow reading the status of the command: read,
 *    write, and erase.
 *    Once the status turns to be ready, the other status bits also are
 *    valid status bits.
 */
static int spinand_read_status(struct spi_device *spi_nand, uint8_t *status)
{
	struct spinand_cmd cmd = {0};
	int ret;

	cmd.cmd = CMD_READ_REG;
	cmd.n_addr = 1;
	cmd.addr[0] = REG_STATUS;
	cmd.n_rx = 1;
	cmd.rx_buf = status;

	ret = spinand_cmd(spi_nand, &cmd);
	if (ret != 0) {
		dev_err(&spi_nand->dev, "err: %d read status register\n", ret);
		return ret;
	}

	return 0;
}


#define time_after_eq(a,b)		((long)(a) - (long)(b) >= 0)
#define time_after(a,b)		((long)(b) - (long)(a) < 0)
#define time_before(a,b)	time_after(b,a)
#define MAX_WAIT_JIFFIES  (40 * HZ)
static int wait_till_ready(struct spi_device *spi_nand)
{
	unsigned long deadline;
	int retval;
	u8 stat = 0;

	deadline = jiffies + MAX_WAIT_JIFFIES;
	do {
		retval = spinand_read_status(spi_nand, &stat);
		if (retval < 0)
			return -1;
		else if (!(stat & 0x1))
			break;

		cond_resched();
	} while (!time_after_eq(jiffies, deadline));

	if ((stat & 0x1) == 0)
		return 0;

	return -1;
}
/**
 * spinand_get_otp- send command 0xf to read the SPI Nand OTP register
 * Description:
 *   There is one bit( bit 0x10 ) to set or to clear the internal ECC.
 *   Enable chip internal ECC, set the bit to 1
 *   Disable chip internal ECC, clear the bit to 0
 */
static int spinand_get_otp(struct spi_device *spi_nand, u8 *otp)
{
	struct spinand_cmd cmd = {0};
	int retval;

	cmd.cmd = CMD_READ_REG;
	cmd.n_addr = 1;
	cmd.addr[0] = REG_OTP;
	cmd.n_rx = 1;
	cmd.rx_buf = otp;

	retval = spinand_cmd(spi_nand, &cmd);
	if (retval != 0) {
		dev_err(&spi_nand->dev, "error %d get otp\n", retval);
		return retval;
	}
	return 0;
}

/**
 * spinand_set_otp- send command 0x1f to write the SPI Nand OTP register
 * Description:
 *   There is one bit( bit 0x10 ) to set or to clear the internal ECC.
 *   Enable chip internal ECC, set the bit to 1
 *   Disable chip internal ECC, clear the bit to 0
 */
static int spinand_set_otp(struct spi_device *spi_nand, u8 *otp)
{
	int retval;
	struct spinand_cmd cmd = {0};

	cmd.cmd = CMD_WRITE_REG,
	cmd.n_addr = 1,
	cmd.addr[0] = REG_OTP,
	cmd.n_tx = 1,
	cmd.tx_buf = otp,

	retval = spinand_cmd(spi_nand, &cmd);
	if (retval != 0) {
		dev_err(&spi_nand->dev, "error %d set otp\n", retval);
		return retval;
	}
	return 0;
}

/**
 * spinand_enable_ecc- send command 0x1f to write the SPI Nand OTP register
 * Description:
 *   There is one bit( bit 0x10 ) to set or to clear the internal ECC.
 *   Enable chip internal ECC, set the bit to 1
 *   Disable chip internal ECC, clear the bit to 0
 */
static int spinand_enable_ecc(struct spi_device *spi_nand)
{
	int retval;
	u8 otp = 0;

	retval = spinand_get_otp(spi_nand, &otp);

	if ((otp & OTP_ECC_MASK) == OTP_ECC_MASK) {
		return 0;
	} else {
		otp |= OTP_ECC_MASK;
		retval = spinand_set_otp(spi_nand, &otp);
		retval = spinand_get_otp(spi_nand, &otp);
		return retval;
	}
}

static int spinand_disable_ecc(struct spi_device *spi_nand)
{
	int retval;
	u8 otp = 0;

	retval = spinand_get_otp(spi_nand, &otp);

	if ((otp & OTP_ECC_MASK) == OTP_ECC_MASK) {
		otp &= ~OTP_ECC_MASK;
		retval = spinand_set_otp(spi_nand, &otp);
		retval = spinand_get_otp(spi_nand, &otp);
		return retval;
	} else
		return 0;
}

static inline int spinand_driver_strength(struct spi_device *spi_nand)
{
	struct spinand_cmd cmd = {0};
	int ret;
	u8 otp = 0,lock = 0x60;

	ret = spinand_get_otp(spi_nand, &otp);

	cmd.cmd = CMD_WRITE_REG;
	cmd.n_addr = 1;
	cmd.addr[0] = REG_STRENGTH;
	cmd.n_tx = 1;
	cmd.tx_buf = &lock;
	
	ret = spinand_cmd(spi_nand, &cmd);
	if (ret != 0) {
		printk("error %d driver strength\n", ret);
		return ret;
	}
	return ret;
}

/**
 * spinand_write_enable- send command 0x06 to enable write or erase the
 * Nand cells
 * Description:
 *   Before write and erase the Nand cells, the write enable has to be set.
 *   After the write or erase, the write enable bit is automatically
 *   cleared (status register bit 2)
 *   Set the bit 2 of the status register has the same effect
 */
static int spinand_write_enable(struct spi_device *spi_nand)
{
	struct spinand_cmd cmd = {0};

	cmd.cmd = CMD_WR_ENABLE;
	return spinand_cmd(spi_nand, &cmd);
}

static int spinand_read_page_to_cache(struct spi_device *spi_nand, int page_id)
{
	struct spinand_cmd cmd = {0};
	int row;

	row = page_id;
	cmd.cmd = CMD_READ;
	cmd.n_addr = 3;
	cmd.addr[0] = (u8)((row & 0xff0000) >> 16);
	cmd.addr[1] = (u8)((row & 0xff00) >> 8);
	cmd.addr[2] = (u8)(row & 0x00ff);

	return spinand_cmd(spi_nand, &cmd);
}

/*
 * spinand_read_from_cache- send command 0x03 to read out the data from the
 * cache register(2112 bytes max)
 * Description:
 *   The read can specify 1 to 2112 bytes of data read at the coresponded
 *   locations.
 *   No tRd delay.
 */
static int spinand_read_from_cache(struct spinand_info *info, u16 byte_id,
		u16 len, u8 *rbuf)
{
	struct spinand_cmd cmd = {0};
	u16 column;

	column = byte_id;
	cmd.cmd = CMD_READ_RDM;
	cmd.n_addr = 3;
	if(info->gd_ctype == 1) {
		cmd.addr[0] = (u8)(0xff);
		cmd.addr[1] = (u8)((column & 0xff00) >> 8);
		cmd.addr[2] = (u8)(column & 0x00fe);
	} else {
		if(byte_id > 0)
		{
			column |= 0x8000;
		}

		cmd.addr[0] = (u8)((column & 0xff00) >> 8);
		cmd.addr[1] = (u8)(column & 0x00ff);
		cmd.addr[2] = (u8)(0xff);
	}
	cmd.n_dummy = 0;
	cmd.n_rx = len;
	cmd.rx_buf = rbuf;

	return spinand_cmd(info->spi, &cmd);
}

/*
 * spinand_read_page-to read a page with:
 * @page_id: the physical page number
 * @offset:  the location from 0 to 2111
 * @len:     number of bytes to read
 * @rbuf:    read buffer to hold @len bytes
 *
 * Description:
 *   The read icludes two commands to the Nand: 0x13 and 0x03 commands
 *   Poll to read status to wait for tRD time.
 */
static int spinand_read_page(struct spinand_info *info, int page_id,
		u16 offset, u16 len, u8 *rbuf)
{
	int ret;
	u8 status = 0;

#ifdef NOUSED_MTD_SPINAND_ONDIEECC
	if (enable_read_hw_ecc) {
		if (spinand_enable_ecc(info->spi))
			dev_err(&info->spi->dev, "enable HW ECC failed!");
	}
#endif
	ret = spinand_read_page_to_cache(info->spi, page_id);
	if (wait_till_ready(info->spi))
		dev_err(&info->spi->dev, "WAIT timedout!!!\n");

	while (1) {
		ret = spinand_read_status(info->spi, &status);
		if (ret < 0) {
			dev_err(&info->spi->dev,
					"err %d read status register\n", ret);
			memset(rbuf,0,len);
			return ret;
		}

		if ((status & STATUS_OIP_MASK) == STATUS_READY) {
			if ((status & STATUS_ECC_MASK) == STATUS_ECC_ERROR) {
				dev_err(&info->spi->dev, "ecc error, page=%d\n",
						page_id);
				ret = spinand_disable_ecc(info->spi);
				memset(rbuf,0,len);
				return 0;
			}
			break;
		}
	}

	ret = spinand_read_from_cache(info, offset, len, rbuf);
	if (ret != 0)
		dev_err(&info->spi->dev, "read from cache failed!!\n");

#ifdef NOUSED_MTD_SPINAND_ONDIEECC
	if (enable_read_hw_ecc) {
		ret = spinand_disable_ecc(info->spi);
		enable_read_hw_ecc = 0;
	}
#endif
	return 0;
}

/*
 * spinand_program_data_to_cache--to write a page to cache with:
 * @byte_id: the location to write to the cache
 * @len:     number of bytes to write
 * @rbuf:    read buffer to hold @len bytes
 *
 * Description:
 *   The write command used here is 0x84--indicating that the cache is
 *   not cleared first.
 *   Since it is writing the data to cache, there is no tPROG time.
 */
static int spinand_program_data_to_cache(struct spi_device *spi_nand,
		u16 byte_id, u16 len, u8 *wbuf)
{
	struct spinand_cmd cmd = {0};
	u16 column;

	column = byte_id;
	cmd.cmd = CMD_PROG_PAGE_CLRCACHE;
	cmd.n_addr = 2;
	cmd.addr[0] = (u8)((column & 0xff00) >> 8);
	cmd.addr[1] = (u8)(column & 0x00ff);
	cmd.n_tx = len;
	cmd.tx_buf = wbuf;

	return spinand_cmd(spi_nand, &cmd);
}

/**
 * spinand_program_execute--to write a page from cache to the Nand array with
 * @page_id: the physical page location to write the page.
 *
 * Description:
 *   The write command used here is 0x10--indicating the cache is writing to
 *   the Nand array.
 *   Need to wait for tPROG time to finish the transaction.
 */
static int spinand_program_execute(struct spi_device *spi_nand, int page_id)
{
	struct spinand_cmd cmd = {0};
	int row;

	row = page_id;
	cmd.cmd = CMD_PROG_PAGE_EXC;
	cmd.n_addr = 3;
	cmd.addr[0] = (u8)((row & 0xff0000) >> 16);
	cmd.addr[1] = (u8)((row & 0xff00) >> 8);
	cmd.addr[2] = (u8)(row & 0x00ff);

	return spinand_cmd(spi_nand, &cmd);
}

/**
 * spinand_program_page--to write a page with:
 * @page_id: the physical page location to write the page.
 * @offset:  the location from the cache starting from 0 to 2111
 * @len:     the number of bytes to write
 * @wbuf:    the buffer to hold the number of bytes
 *
 * Description:
 *   The commands used here are 0x06, 0x84, and 0x10--indicating that
 *   the write enable is first
 *   sent, the write cache command, and the write execute command
 *   Poll to wait for the tPROG time to finish the transaction.
 */
static int spinand_program_page(struct spinand_info *info,
		int page_id, u16 offset, u16 len, u8 *buf)
{
	int retval;
	u8 status = 0;
	uint8_t *wbuf;
#ifdef NOUSED_MTD_SPINAND_ONDIEECC
	unsigned int i, j;

	enable_read_hw_ecc = 0;
	wbuf = devm_kzalloc(&info->spi->dev, 2112, GFP_KERNEL);
	spinand_read_page(info, page_id, 0, 2112, wbuf);

	for (i = offset, j = 0; i < len; i++, j++)
		wbuf[i] &= buf[j];

	if (enable_hw_ecc)
		retval = spinand_enable_ecc(info->spi);
#else
	wbuf = buf;
#endif
	retval = spinand_write_enable(info->spi);
	if (wait_till_ready(info->spi))
		dev_err(&info->spi->dev, "wait timedout!!!\n");

	retval = spinand_program_data_to_cache(info->spi, offset, len, wbuf);
	retval = spinand_program_execute(info->spi, page_id);
	while (1) {
		retval = spinand_read_status(info->spi, &status);
		if (retval < 0) {
			dev_err(&info->spi->dev,
					"error %d reading status register\n",
					retval);
			return retval;
		}

		if ((status & STATUS_OIP_MASK) == STATUS_READY) {
			if ((status & STATUS_P_FAIL_MASK) == STATUS_P_FAIL) {
				dev_err(&info->spi->dev,
					"program error, page %d\n", page_id);
				return -1;
			} else
				break;
		}
	}
#ifdef NOUSED_MTD_SPINAND_ONDIEECC
	if (enable_hw_ecc) {
		retval = spinand_disable_ecc(info->spi);
		enable_hw_ecc = 0;
	}
#endif

	return 0;
}

/**
 * spinand_erase_block_erase--to erase a page with:
 * @block_id: the physical block location to erase.
 *
 * Description:
 *   The command used here is 0xd8--indicating an erase command to erase
 *   one block--64 pages
 *   Need to wait for tERS.
 */
static int spinand_erase_block_erase(struct spi_device *spi_nand, int block_id)
{
	struct spinand_cmd cmd = {0};
	int row;

	row = block_id;
	cmd.cmd = CMD_ERASE_BLK;
	cmd.n_addr = 3;
	cmd.addr[0] = (u8)((row & 0xff0000) >> 16);
	cmd.addr[1] = (u8)((row & 0xff00) >> 8);
	cmd.addr[2] = (u8)(row & 0x00ff);

	return spinand_cmd(spi_nand, &cmd);
}

/**
 * spinand_erase_block--to erase a page with:
 * @block_id: the physical block location to erase.
 *
 * Description:
 *   The commands used here are 0x06 and 0xd8--indicating an erase
 *   command to erase one block--64 pages
 *   It will first to enable the write enable bit (0x06 command),
 *   and then send the 0xd8 erase command
 *   Poll to wait for the tERS time to complete the tranaction.
 */
static int spinand_erase_block(struct spi_device *spi_nand, int block_id, struct mtd_info *mtd)
{
	int retval;
	u8 status = 0;

	retval = spinand_write_enable(spi_nand);
	if (wait_till_ready(spi_nand))
		dev_err(&spi_nand->dev, "wait timedout!!!\n");

	retval = spinand_erase_block_erase(spi_nand, block_id);
	while (1) {
		retval = spinand_read_status(spi_nand, &status);
		if (retval < 0) {
			dev_err(&spi_nand->dev,
					"error %d reading status register\n",
					(int) retval);
			return retval;
		}

		if ((status & STATUS_OIP_MASK) == STATUS_READY) {
			if ((status & STATUS_E_FAIL_MASK) == STATUS_E_FAIL) {
				dev_err(&spi_nand->dev,
					"erase error, block %d\n", block_id);
				mtd->block_markbad(mtd, block_id*mtd->writesize);
				return -1;
			} else
				break;
		}
	}
	return 0;
}

#ifdef NOUSED_MTD_SPINAND_ONDIEECC
static int spinand_write_page_hwecc(struct mtd_info *mtd,
		struct nand_chip *chip, const uint8_t *buf, int oob_required)
{
	const uint8_t *p = buf;
	int eccsize = chip->ecc.size;
	int eccsteps = chip->ecc.steps;

	enable_hw_ecc = 1;
	chip->write_buf(mtd, p, eccsize * eccsteps);
	return 0;
}

static int spinand_read_page_hwecc(struct mtd_info *mtd, struct nand_chip *chip,
		uint8_t *buf, int oob_required, int page)
{
	u8 retval, status;
	uint8_t *p = buf;
	int eccsize = chip->ecc.size;
	int eccsteps = chip->ecc.steps;
	struct spinand_info *info = (struct spinand_info *)chip->priv;

	enable_read_hw_ecc = 1;

	chip->read_buf(mtd, p, eccsize * eccsteps);
	if (oob_required)
		chip->read_buf(mtd, chip->oob_poi, mtd->oobsize);

	while (1) {
		retval = spinand_read_status(info->spi, &status);
		if ((status & STATUS_OIP_MASK) == STATUS_READY) {
			if ((status & STATUS_ECC_MASK) == STATUS_ECC_ERROR) {
				pr_info("spinand: ECC error\n");
				mtd->ecc_stats.failed++;
			} else if ((status & STATUS_ECC_MASK) ==
					STATUS_ECC_1BIT_CORRECTED)
				mtd->ecc_stats.corrected++;
			break;
		}
	}
	return 0;

}
#endif

static void spinand_select_chip(struct mtd_info *mtd, int dev)
{
}

static uint8_t spinand_read_byte(struct mtd_info *mtd)
{
	struct nand_chip *chip = (struct nand_chip *)mtd->priv;
	struct spinand_info *info = (struct spinand_info *)chip->priv;
	struct nand_state *state = (struct nand_state *)info->priv;
	u8 data;

	data = state->buf[state->buf_ptr];
	state->buf_ptr++;
	return data;
}

static int spinand_wait(struct mtd_info *mtd, struct nand_chip *chip)
{
	struct spinand_info *info = (struct spinand_info *)chip->priv;

	unsigned long timeo = jiffies;
	int retval, state = chip->state;
	u8 status;

	if (state == FL_ERASING)
		timeo += (HZ * 400) / 1000;
	else
		timeo += (HZ * 20) / 1000;

	while (time_before(jiffies, timeo)) {
		retval = spinand_read_status(info->spi, &status);
		if ((status & STATUS_OIP_MASK) == STATUS_READY)
			return 0;

		cond_resched();
	}
	return 0;
}

static void spinand_write_buf(struct mtd_info *mtd, const uint8_t *buf, int len)
{
	struct nand_chip *chip = (struct nand_chip *)mtd->priv;
	struct spinand_info *info = (struct spinand_info *)chip->priv;
	struct nand_state *state = (struct nand_state *)info->priv;

	memcpy(state->buf+state->buf_ptr, buf, len);
	state->buf_ptr += len;
}

static void spinand_read_buf(struct mtd_info *mtd, uint8_t *buf, int len)
{
	struct nand_chip *chip = (struct nand_chip *)mtd->priv;
	struct spinand_info *info = (struct spinand_info *)chip->priv;
	struct nand_state *state = (struct nand_state *)info->priv;

	memcpy(buf, state->buf+state->buf_ptr, len);
	state->buf_ptr += len;
}

static void spinand_cmdfunc(struct mtd_info *mtd, unsigned int command,
		int column, int page)
{
	struct nand_chip *chip = (struct nand_chip *)mtd->priv;
	struct spinand_info *info = (struct spinand_info *)chip->priv;
	struct nand_state *state = (struct nand_state *)info->priv;

	switch (command) {
	/*
	 * READ0 - read in first  0x800 bytes
	 */
	case NAND_CMD_READ1:
	case NAND_CMD_READ0:
		state->buf_ptr = 0;
//		spinand_read_page(info, page, 0x0, 0x840, state->buf);
		spinand_read_page(info, page, 0x0, mtd->oobsize + mtd->writesize, state->buf);
		break;
	/* READOOB reads only the OOB because no ECC is performed. */
	case NAND_CMD_READOOB:
		state->buf_ptr = 0;
		if(info->gd_ctype == 0)
		{
			if(page == 0xffc0 || page == 0x6440)
			{
				memset(state->buf,0,mtd->oobsize);
				break;
			}
		}
		spinand_read_page(info, page, mtd->writesize, mtd->oobsize, state->buf);
		break;
	case NAND_CMD_RNDOUT:
		state->buf_ptr = column;
		break;
	case NAND_CMD_READID:
		state->buf_ptr = 0;
		spinand_read_id(info, (u8 *)state->buf);
		break;
	case NAND_CMD_PARAM:
		state->buf_ptr = 0;
		break;
	/* ERASE1 stores the block and page address */
	case NAND_CMD_ERASE1:
		spinand_erase_block(info->spi, page, mtd);
		break;
	/* ERASE2 uses the block and page address from ERASE1 */
	case NAND_CMD_ERASE2:
		break;
	/* SEQIN sets up the addr buffer and all registers except the length */
	case NAND_CMD_SEQIN:
		state->col = column;
		state->row = page;
		state->buf_ptr = 0;
		break;
	/* PAGEPROG reuses all of the setup from SEQIN and adds the length */
	case NAND_CMD_PAGEPROG:
		spinand_program_page(info, state->row, state->col,
				state->buf_ptr, state->buf);
		break;
	case NAND_CMD_STATUS:
		spinand_get_otp(info->spi, state->buf);
		if (!(state->buf[0] & 0x80))
			state->buf[0] = 0x80;
		state->buf_ptr = 0;
		break;
	/* RESET command */
	case NAND_CMD_RESET:
		break;
	default:
		dev_err(&mtd->dev, "Unknown CMD: 0x%x\n", command);
	}
}

/**
 * spinand_lock_block- send write register 0x1f command to the Nand device
 *
 * Description:
 *    After power up, all the Nand blocks are locked.  This function allows
 *    one to unlock the blocks, and so it can be wriiten or erased.
 */
static int spinand_lock_block(struct spi_device *spi_nand, u8 lock)
{
	struct spinand_cmd cmd = {0};
	int ret;
	u8 otp = 0;

	ret = spinand_get_otp(spi_nand, &otp);

	cmd.cmd = CMD_WRITE_REG;
	cmd.n_addr = 1;
	cmd.addr[0] = REG_BLOCK_LOCK;
	cmd.n_tx = 1;
	cmd.tx_buf = &lock;

	ret = spinand_cmd(spi_nand, &cmd);
	if (ret != 0) {
		dev_err(&spi_nand->dev, "error %d lock block\n", ret);
		return ret;
	}
	return 0;
}


/*
 * spinand_probe - [spinand Interface]
 * @spi_nand: registered device driver.
 *
 * Description:
 *   To set up the device driver parameters to make the device available.
 */
int spinand_probe(struct spi_device *spi_nand)
{
	struct mtd_info *mtd;
	struct nand_chip *chip;
	struct spinand_info *info;
	struct nand_state *state;
	u8 spi_flash_id[3];

	int ret;


	info  = devm_kzalloc(&spi_nand->dev, sizeof(struct spinand_info),
			GFP_KERNEL);
	if (!info)
		return -ENOMEM;

	info->spi = spi_nand;

	spinand_lock_block(spi_nand, BL_ALL_UNLOCKED);

	state = devm_kzalloc(&spi_nand->dev, sizeof(struct nand_state),
			GFP_KERNEL);
	if (!state)
		return -ENOMEM;

	info->priv	= state;
	state->buf_ptr	= 0;
	state->buf	= devm_kzalloc(&spi_nand->dev, BUFSIZE, GFP_KERNEL);
	if (!state->buf)
		return -ENOMEM;

	chip = devm_kzalloc(&spi_nand->dev, sizeof(struct nand_chip),
			GFP_KERNEL);
	if (!chip)
		return -ENOMEM;
	memset(chip, 0, sizeof(struct nand_chip));

	chip->priv	= info;
	chip->read_buf	= spinand_read_buf;
	chip->write_buf	= spinand_write_buf;
	chip->read_byte	= spinand_read_byte;
	chip->cmdfunc	= spinand_cmdfunc;
	chip->waitfunc	= spinand_wait;
	chip->options	|= NAND_CACHEPRG;
	chip->select_chip = spinand_select_chip;

	mtd = devm_kzalloc(&spi_nand->dev, sizeof(struct mtd_info), GFP_KERNEL);
	if (!mtd)
		return -ENOMEM;

	dev_set_drvdata(&spi_nand->dev, mtd);

	mtd->priv = chip;
	mtd->name = "spinand_flash";

	spinand_read_id(info, spi_flash_id);

	ret = nand_scan_ident(mtd, 1);
	if (ret)
		return -ENOMEM;

	if(info->gd_ctype == 1) {
		spinand_driver_strength(info->spi);
	}

#ifdef NOUSED_MTD_SPINAND_ONDIEECC
	chip->ecc.mode	= NAND_ECC_HW;
	chip->ecc.size	= 0x200;
	chip->ecc.bytes	= 0x6;
	chip->ecc.steps	= 0x4;

#if 0
	chip->ecc.strength = 1;
#endif
	chip->ecc.total	= chip->ecc.steps * chip->ecc.bytes;
	chip->ecc.layout = &spinand_oob_64;
	chip->ecc.read_page = spinand_read_page_hwecc;
	chip->ecc.write_page = spinand_write_page_hwecc;
#elif defined(CONFIG_SPINAND_SOFTBCH)
#if !NNAND_BCH
    #error("need select nand_bch");
#endif
#define BCH_BUG(a...) printf(a);while(1);
	{
		int bch = 4;
		if (!mtd_nand_has_bch()) {
			BCH_BUG("BCH ECC support is disabled\n");
		}
		/* use 512-byte ecc blocks */
		//eccsteps = writesize/512;
		//eccbytes = (bch*13+7)/8;
		chip->ecc.size = 512;
		chip->ecc.strength = bch;
		chip->ecc.bytes = DIV_ROUND_UP(
				chip->ecc.strength * fls(8 * chip->ecc.size), 8);
		/* do not bother supporting small page devices */
		if (mtd->oobsize < 64) {
			BCH_BUG("bch not available on small page devices\n");
			return -EINVAL;
		}
		chip->ecc.mode = NAND_ECC_SOFT_BCH;
		printf("using %u-bit/%u bytes BCH ECC\n", bch, chip->ecc.size);
	}
#elif defined(CONFIG_SPINAND_SOFTECC)
	chip->ecc.mode		= NAND_ECC_SOFT;
	chip->ecc.size		= 256;
	chip->ecc.bytes		= 3;
#else
	chip->ecc.mode	= NAND_ECC_NONE;
#endif

#ifdef CONFIG_MTD_SPINAND_NO_HWECC
	spinand_write_enable(info->spi);
	ret = spinand_disable_ecc(info->spi);
#else
	spinand_write_enable(info->spi);
	ret = spinand_enable_ecc(info->spi);
#endif

	if (nand_scan_tail(mtd))
		return -1;

	if ((chip->ecc.bytes*mtd->writesize/chip->ecc.size) > chip->ecc.layout->eccbytes) {
		printf("invalid ecc or bch value \n");
		return -EINVAL;
	}




        if(!nand_flash_add_parts(mtd,0)){
                //add_mtd_device(mtd,0,0,"total");
                add_mtd_device(mtd,0,0x01400000,"kernel");
                add_mtd_device(mtd,0x01400000,0x0,"os");
        }
	return 0;
}
