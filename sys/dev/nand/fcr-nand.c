/*
 *  drivers/mtd/nand/fcr_soc.c
 */

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/nand.h>
#include <linux/mtd/partitions.h>
#include <asm/io.h>

/*
 * MTD structure for fcr_soc board
 */
static struct mtd_info *fcr_soc_mtd = NULL;

MODULE_PARM(fcr_soc_pedr, "i");
MODULE_PARM(fcr_soc_peddr, "i");
/*
 * Define partitions for flash device
 */
static const struct mtd_partition partition_info[] = {
	{
	 .name = "fcr_soc flash partition 1",
	 .offset = 12*1024*1024,
	 .size = 32 * 1024 * 1024}
};

#define NUM_PARTITIONS 1

static void fcr_soc_hwcontrol(struct mtd_info *mtd, int dat,unsigned int ctrl)
{
	struct nand_chip *chip = mtd->priv;

if((ctrl & NAND_CTRL_ALE)==NAND_CTRL_ALE)
		*(volatile unsigned int *)(0xbf000014) = dat;
if ((ctrl & NAND_CTRL_CLE)==NAND_CTRL_CLE)
		*(volatile unsigned int *)(0xbf000010) = dat;
}

/*
 * Main initialization routine
 */
static int __init fcr_soc_init(void)
{
	struct nand_chip *this;

	/* Allocate memory for MTD device structure and private data */
	fcr_soc_mtd = kmalloc(sizeof(struct mtd_info) + sizeof(struct nand_chip), GFP_KERNEL);
	if (!fcr_soc_mtd) {
		printk("Unable to allocate fcr_soc NAND MTD device structure.\n");
		return -ENOMEM;
	}

	/* Get pointer to private data */
	this = (struct nand_chip *)(&fcr_soc_mtd[1]);

	/* Initialize structures */
	memset(fcr_soc_mtd, 0, sizeof(struct mtd_info));
	memset(this, 0, sizeof(struct nand_chip));

	/* Link the private data with the MTD structure */
	fcr_soc_mtd->priv = this;


	/* Set address of NAND IO lines */
	this->IO_ADDR_R = (void  *)0xbf000040;
	this->IO_ADDR_W = (void  *)0xbf000040;
	/* Set address of hardware control function */
	this->cmd_ctrl = fcr_soc_hwcontrol;
	/* 15 us command delay time */
	this->chip_delay = 15;
	this->ecc.mode = NAND_ECC_SOFT;

	/* Scan to find existence of the device */
	if (nand_scan(fcr_soc_mtd, 1)) {
		kfree(fcr_soc_mtd);
		return -ENXIO;
	}

	/* Register the partitions */
	add_mtd_partitions(fcr_soc_mtd, partition_info, NUM_PARTITIONS);
	add_mtd_device(fcr_soc_mtd);

	/* Return happy */
	return 0;
}

module_init(fcr_soc_init);

/*
 * Clean up routine
 */
static void __exit fcr_soc_cleanup(void)
{
	/* Release resources, unregister device */
	nand_release(fcr_soc_mtd);

	/* Free the MTD device structure */
	kfree(fcr_soc_mtd);
}

module_exit(fcr_soc_cleanup);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("qiao chong <qiaochong@ict.ac.cn");
MODULE_DESCRIPTION("Board-specific glue layer for NAND flash on fcr_soc board");
