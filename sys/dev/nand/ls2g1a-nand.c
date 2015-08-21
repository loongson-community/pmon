/*
 * Author:mengtianfang
 * email:mengtianfang@loongson.cn
 * nandinit, nanderase, nandreadid, nandread, nandwrite, nand_load functions can be used.
 * These functions are modified according to the source file sys/dev/nand/ls1g-annd.c.
 */

#include<pmon.h>
#include<asm.h>
#include<machine/types.h>
#include<linux/mtd/mtd.h>
#include<linux/mtd/nand.h>
#include<linux/mtd/partitions.h>
#include<sys/malloc.h>

#include <unistd.h>
#include <fcntl.h>

#ifndef __iomem
#define __iomem
#endif

#define DMA_ACCESS_ADDR		0x40
#define ORDER_REG_ADDR		0xb2d01160
#define DDR_PHYADDR		0xad000000
#define NAND_PAGE_SIZE		2048
#define NAND_PAGE_PER_BLOCK	0x40

#define nand_write(val, addr)	(*(volatile unsigned int*)(addr) = (val))
#define nand_read(addr)		(*(volatile unsigned int*)(addr))

#define ALIGN_DMA(x)		(((x) + 3)/4)
#define STATUS_TIME_LOOP_R	1000
#define STATUS_TIME_LOOP_E	1000

/* add for 2g1a */
#define NAND_REG_BASE		0xb2e78000

#define NAND_REG_CMD		(NAND_REG_BASE + 0x00)
#define NAND_REG_ADDR_C		(NAND_REG_BASE + 0x04)
#define NAND_REG_ADDR_R		(NAND_REG_BASE + 0x08)
#define NAND_REG_TIMING		(NAND_REG_BASE + 0x0c)
#define NAND_REG_IDL		(NAND_REG_BASE + 0x10)
#define NAND_REG_STATUS		(NAND_REG_BASE + 0x14)
#define NAND_REG_IDH		(NAND_REG_BASE + 0x14)
#define NAND_REG_PARAMETER	(NAND_REG_BASE + 0x18)
#define NAND_REG_OP_NUM		(NAND_REG_BASE + 0x1c)
#define NAND_REG_CS_RDY_MAP	(NAND_REG_BASE + 0x20)
#define NAND_REG_DMA_AC_ADDR	(NAND_REG_BASE + 0x40)

#define NAND_CMD_VALID		(1 << 0)
#define NAND_CMD_OP_RD		(1 << 1)
#define NAND_CMD_OP_WR		(1 << 2)
#define NAND_CMD_OP_ER		(1 << 3)
#define NAND_CMD_BLK_ER		(1 << 4)
#define	NAND_CMD_RD_ID		(1 << 5)
#define NAND_CMD_OP_RST		(1 << 6)
#define NAND_CMD_RD_STATUS	(1 << 7)
#define NAND_CMD_MAIN		(1 << 8)
#define NAND_CMD_SPARE		(1 << 9)
#define NAND_CMD_DONE		(1 << 10)

enum{
	STATE_READY = 0,
	STATE_BUSY  ,
};

struct ls2g1a_nand_cmdset {
        uint32_t    cmd_valid:1;
	uint32_t    read:1;
	uint32_t    write:1;
	uint32_t    erase_one:1;
	uint32_t    erase_con:1;
	uint32_t    read_id:1;
	uint32_t    reset:1;
	uint32_t    read_sr:1;
	uint32_t    op_main:1;
	uint32_t    op_spare:1;
	uint32_t    done:1;
        uint32_t    resv1:5;		//11-15 reserved
        uint32_t    nand_rdy:4;		//16-19
        uint32_t    nand_ce:4;		//20-23
        uint32_t    resv2:8;		//24-32 reserved
};

struct ls2g1a_nand_dma_desc{
        uint32_t    orderad;
        uint32_t    saddr;
        uint32_t    daddr;
        uint32_t    length;
        uint32_t    step_length;
        uint32_t    step_times;
        uint32_t    cmd;
};

struct ls2g1a_nand_dma_cmd{
        uint32_t    dma_int_mask:1;
        uint32_t    dma_int:1;
        uint32_t    dma_sl_tran_over:1;
        uint32_t    dma_tran_over:1;
        uint32_t    dma_r_state:4;
        uint32_t    dma_w_state:4;
        uint32_t    dma_r_w:1;
        uint32_t    dma_cmd:2;
        uint32_t    revl:17;
};

struct ls2g1a_nand_desc{
        uint32_t    cmd;
        uint32_t    addrl;
        uint32_t    addrh;
        uint32_t    timing;
        uint32_t    idl;		//readonly
        uint32_t    status_idh;		//readonly
        uint32_t    param;
        uint32_t    op_num;
        uint32_t    cs_rdy_map;
};

struct ls2g1a_nand_info {
	struct nand_chip	nand_chip;

//	struct platform_device	    *pdev;
        /* MTD data control*/
	unsigned int 		buf_start;
	unsigned int		buf_count;
        /* NAND registers*/
	void __iomem		*mmio_base;
        struct ls2g1a_nand_desc   nand_regs;
        unsigned int            nand_addrl;
        unsigned int            nand_addrh;
        unsigned int            nand_timing;
        unsigned int            nand_op_num;
        unsigned int            nand_cs_rdy_map;
        unsigned int            nand_cmd;

	/* DMA information */

        struct ls2g1a_nand_dma_desc  dma_regs;
        unsigned int            order_reg_addr;  
        unsigned int            dma_orderad;
        unsigned int            dma_saddr;
        unsigned int            dma_daddr;
        unsigned int            dma_length;
        unsigned int            dma_step_length;
        unsigned int            dma_step_times;
        unsigned int            dma_cmd;
        unsigned int		drcmr_dat;//dma descriptor address;
	unsigned int 		drcmr_dat_phys;
        size_t                  drcmr_dat_size;
	unsigned char		*data_buff;//dma data buffer;
	unsigned int 		data_buff_phys;
	size_t			data_buff_size;
        unsigned int            data_ask;
        unsigned int            data_ask_phys;
        unsigned int            data_length;
        unsigned int            cac_size;
        unsigned int            size;
        unsigned int            num;
        
	/* relate to the command */
	unsigned int		state;
//	int			use_ecc;	/* use HW ECC ? */
	size_t			data_size;	/* data size in FIFO */
        unsigned int            cmd;
        unsigned int            cmd_prev;
        unsigned int            page_addr;
//	struct completion 	cmd_complete;
        unsigned int            seqin_column;
        unsigned int            seqin_page_addr;
};

struct ls2g1a_nand_ask_regs{
        unsigned int dma_order_addr;
        unsigned int dma_mem_addr;
        unsigned int dma_dev_addr;
        unsigned int dma_length;
        unsigned int dma_step_length;
        unsigned int dma_step_times;
        unsigned int dma_state_tmp;
};

static struct mtd_info *ls2g1a_soc_mtd = NULL;
static struct nand_ecclayout hw_largepage_ecclayout = {
	.eccbytes = 24,
	.eccpos = {
		40, 41, 42, 43, 44, 45, 46, 47,
		48, 49, 50, 51, 52, 53, 54, 55,
		56, 57, 58, 59, 60, 61, 62, 63},
	.oobfree = { {2, 38} }
};

static int ls2g1a_nand_ecc_calculate(struct mtd_info *mtd,
		const uint8_t *dat, uint8_t *ecc_code)
{
	return 0;
}
static int ls2g1a_nand_ecc_correct(struct mtd_info *mtd,
		uint8_t *dat, uint8_t *read_ecc, uint8_t *calc_ecc)
{
	/*
	 * Any error include ERR_SEND_CMD, ERR_DBERR, ERR_BUSERR, we
	 * consider it as a ecc error which will tell the caller the
	 * read fail We have distinguish all the errors, but the
	 * nand_read_ecc only check this function return value
	 */
	return 0;
}

static void ls2g1a_nand_ecc_hwctl(struct mtd_info *mtd, int mode)
{
	return;
}

static int ls2g1a_nand_waitfunc(struct mtd_info *mtd, struct nand_chip *this)
{
	udelay(50);
	return 0;
}

static void ls2g1a_nand_select_chip(struct mtd_info *mtd, int chip)
{
	return;
}

static int ls2g1a_nand_dev_ready(struct mtd_info *mtd)
{
	return 1;
}

static void ls2g1a_nand_read_buf(struct mtd_info *mtd, uint8_t *buf, int len)
{
	struct ls2g1a_nand_info *info = mtd->priv;
	int real_len = min_t(size_t, len, info->buf_count - info->buf_start);

	memcpy(buf, info->data_buff + info->buf_start, real_len);
	info->buf_start += real_len;
}

static u16 ls2g1a_nand_read_word(struct mtd_info *mtd)
{
	struct ls2g1a_nand_info *info = mtd->priv;
	u16 retval = 0xFFFF;
	if(!(info->buf_start & 0x1) && info->buf_start < info->buf_count){
		retval = *(u16 *)(info->data_buff + info->buf_start);
	}
	info->buf_start += 2;
	return retval;
}

static uint8_t ls2g1a_nand_read_byte(struct mtd_info *mtd)
{
	struct ls2g1a_nand_info *info = mtd->priv;
	char retval = 0xFF;

	if (info->buf_start < info->buf_count)
	/* Has just send a new command? */
		retval = info->data_buff[(info->buf_start)++];
	return retval;
}

static void ls2g1a_nand_write_buf(struct mtd_info *mtd,const uint8_t *buf, int len)
{
	struct ls2g1a_nand_info *info = mtd->priv;
	int real_len = min_t(size_t, len, info->buf_count - info->buf_start);

	memcpy(info->data_buff + info->buf_start, buf, real_len);
	info->buf_start += real_len;
}

static int ls2g1a_nand_verify_buf(struct mtd_info *mtd,const uint8_t *buf, int len)
{
	int i = 0; 
	while(len--){
		if(buf[i++] != ls2g1a_nand_read_byte(mtd) ){
			printf("verify error...\n\n");
			return -1;
		}
	}
	return 0;
}

static int nand_read_id(void);
static void ls2g1a_nand_cmdfunc(struct mtd_info *mtd, unsigned command,int column, int page_addr);
static void ls2g1a_nand_init_mtd(struct mtd_info *mtd,struct ls2g1a_nand_info *info);

int ls2g1a_nand_init(struct mtd_info *mtd)
{
	int ret = 0;
	ret = ls2g1a_nand_pmon_info_init((struct ls2g1a_nand_info *)(mtd->priv), mtd);//data buff, desc buff, reg
	ls2g1a_nand_init_mtd(mtd, (struct ls2g1a_nand_info *)(mtd->priv));//func
	return ret;
}

static void ls2g1a_nand_init_mtd(struct mtd_info *mtd, struct ls2g1a_nand_info *info)
{
	struct nand_chip *this = &info->nand_chip;

	this->options = 8;

	this->waitfunc		= ls2g1a_nand_waitfunc;		/*等待设备准备好 硬件相关函数*/
	this->select_chip	= ls2g1a_nand_select_chip;	/*控制CE信号*/
	this->dev_ready		= ls2g1a_nand_dev_ready;	/*板特定的设备ready/busy信息*/
	this->cmdfunc		= ls2g1a_nand_cmdfunc;		/*命令处理函数*/
	this->read_word		= ls2g1a_nand_read_word;	/*从芯片读一个字*/
	this->read_byte		= ls2g1a_nand_read_byte;	/*从芯片读一个字节*/
	this->read_buf		= ls2g1a_nand_read_buf;		/*将芯片数据读到缓冲区*/
	this->write_buf		= ls2g1a_nand_write_buf;	/*将缓冲区内容写入芯片*/
	this->verify_buf	= ls2g1a_nand_verify_buf;	/*验证芯片和写入缓冲区中的数据*/

//	this->ecc.mode		= NAND_ECC_NONE;
	this->ecc.mode		= NAND_ECC_SOFT;	/*ECC模式 这里是软件模式*/
//	this->ecc.hwctl		= ls2g1a_nand_ecc_hwctl;
//	this->ecc.calculate	= ls2g1a_nand_ecc_calculate;
//	this->ecc.correct		= ls2g1a_nand_ecc_correct;
//	this->ecc.size		= 2048;
//	this->ecc.bytes		= 24;

//	this->ecc.layout		= &hw_largepage_ecclayout;
//	mtd->owner				= THIS_MODULE;
}

static void wait_nand_done(struct ls2g1a_nand_info *info, int timeout)
{
	int t, status_times = timeout;

	do {
		t = nand_read(NAND_REG_CMD) & NAND_CMD_DONE;
		if (!(status_times--)) {
			printf("nand time out\n");
			nand_write(0x0, NAND_REG_CMD);
			nand_write(0x0, NAND_REG_CMD);
			nand_write(NAND_CMD_OP_RST | NAND_CMD_VALID, NAND_REG_CMD);
			break;
		}
		udelay(50);
	} while(t == 0);

	nand_write(0x0, NAND_REG_CMD);
}

static void dma_setup(struct ls2g1a_nand_info *info, int dma_cmd, int dma_cnt)
{
	int t;
	struct ls2g1a_nand_dma_desc *dma_base = (volatile struct ls2g1a_nand_dma_desc *)(info->drcmr_dat);

	dma_base->orderad = 0;
	dma_base->saddr = info->data_buff_phys;
	dma_base->daddr = DMA_ACCESS_ADDR;
	dma_base->length = dma_cnt;
	dma_base->step_length = 0;
	dma_base->step_times = 0x1;
	dma_base->cmd = dma_cmd;

	/* dma start */
	*(volatile unsigned int *)(info->order_reg_addr) = ((unsigned int )info->drcmr_dat_phys) | (0x1 << 3);
	*(volatile unsigned int *)(info->order_reg_addr) = (0x1 << 2) | (info->data_ask_phys);
	t = STATUS_TIME_LOOP_R;
	while (*(volatile unsigned int *)(info->order_reg_addr) & (0x1 << 3) && t) {
		t--;
		udelay(50);
	}
	if (t == 0)
		printf("dma timeout!\n\n");
	wait_nand_done(info, STATUS_TIME_LOOP_R);
}

static void nand_setup(unsigned int cmd, int addr_c, int addr_r, int op_num)
{
	nand_write(op_num, NAND_REG_OP_NUM);
	nand_write(addr_c, NAND_REG_ADDR_C);
	nand_write(addr_r, NAND_REG_ADDR_R);
	nand_write(0, NAND_REG_CMD);
	nand_write(0, NAND_REG_CMD);
	nand_write(cmd, NAND_REG_CMD);
}

static void ls2g1a_nand_cmdfunc(struct mtd_info *mtd, unsigned command,int column, int page_addr)
{
	struct ls2g1a_nand_info *info = mtd->priv;

	unsigned int cmd = 0;
	int addrc, addrr, op_num;
	int dma_cmd, dma_cnt;

	info->cmd = command;
	info->page_addr = page_addr;
	switch(command){
		case NAND_CMD_READOOB:
			if(info->state == STATE_BUSY){
				printf("read oob nandflash chip if busy...\n");
				return;
			}
			info->state = STATE_BUSY;
			info->buf_count = mtd->oobsize;
			info->buf_start = 0;
			info->cac_size = info->buf_count;
			if(info->buf_count <=0 )
				break;
			/* nand regs set */
			addrr = page_addr;
			addrc = mtd->writesize;
			op_num = info->buf_count;
			cmd = (NAND_CMD_OP_RD | NAND_CMD_SPARE | NAND_CMD_VALID);
			nand_setup(cmd, addrc, addrr, op_num);

			/* dma regs config */
			dma_cnt = ALIGN_DMA(info->buf_count);
			dma_cmd = 1;//dma_int_mask
			dma_setup(info, dma_cmd, dma_cnt);
		break;
		case NAND_CMD_READ0:
			if(info->state == STATE_BUSY){
				printf("nandflash chip if busy...\n");
				return;
			}
			info->state = STATE_BUSY;

			cmd = NAND_CMD_MAIN | NAND_CMD_SPARE | NAND_CMD_OP_RD | NAND_CMD_VALID;
			addrr = page_addr;
			addrc = 0;
			op_num = mtd->oobsize + mtd->writesize;

			info->buf_count = op_num;
			info->buf_start = 0;
			info->cac_size = info->buf_count;

			nand_setup(cmd, addrc, addrr, op_num);
			/* dma regs config */
			dma_cmd = 0x1;
			dma_cnt = ALIGN_DMA(op_num);
			dma_setup(info, dma_cmd, dma_cnt);
		break;
		case NAND_CMD_SEQIN:
			if(info->state == STATE_BUSY){
				printf("nandflash chip if busy...\n");
				return;
			}
			info->state = STATE_BUSY;
			info->buf_count = mtd->oobsize + mtd->writesize - column;
			info->buf_start = 0;
			info->seqin_column = column;
			info->seqin_page_addr = page_addr;
		break;
		case NAND_CMD_PAGEPROG:
			if(info->state == STATE_BUSY){
				printf("nandflash chip if busy...\n");
				return;
			}
			info->state = STATE_BUSY;
			if(info->buf_count <= 0 )
				break;

			if(((info->num)++) % 512 == 0){
				printf("nand have write : %d M\n",(info->size)++); 
			}

			addrc = info->seqin_column;
			addrr = info->seqin_page_addr;
			op_num = info->buf_start;
			cmd = NAND_CMD_SPARE | NAND_CMD_OP_WR | NAND_CMD_VALID;
			if (addrc < mtd->writesize)
				cmd |= NAND_CMD_MAIN;
			nand_setup(cmd, addrc, addrr, op_num);

			dma_cmd = 0x1 | (1 << 12);
			dma_cnt = ALIGN_DMA(op_num);
			dma_setup(info, dma_cmd, dma_cnt);
		break;
		case NAND_CMD_RESET:
			info->state = STATE_BUSY;
			cmd = (NAND_CMD_OP_RST | NAND_CMD_VALID);
			nand_setup(cmd, 0, 0, 0);
			wait_nand_done(info, 30);
			info->state = STATE_READY;
		break;
		case NAND_CMD_ERASE1:
			info->state = STATE_BUSY;
			/* nand regs set */
			addrr = page_addr;
			addrc = 0x0;
			op_num = 0x0;
			cmd = NAND_CMD_OP_ER | NAND_CMD_VALID;
			nand_setup(cmd, addrc, addrr, op_num);
			wait_nand_done(info, STATUS_TIME_LOOP_E);
			info->state = STATE_READY;
		break;
		case NAND_CMD_STATUS:
			info->buf_count = 0x1;
			info->buf_start = 0x0;
			*(unsigned char *)info->data_buff =
				(nand_read(NAND_REG_CMD) & (NAND_CMD_DONE)) | (NAND_CMD_RD_STATUS);
		break;
		case NAND_CMD_READID:
			if(info->state == STATE_BUSY){
				printf(" read id nandflash chip if busy...\n");
				return;
			}
			info->state = STATE_BUSY;
			info->buf_count = 0x4;
			info->buf_start = 0;
			nand_read_id();

		break;
		case NAND_CMD_ERASE2:
		case NAND_CMD_READ1:
		break;
		default :
			printf("non-supported command.\n");
		break;
	}

	info->state = STATE_READY;
}

int ls2g1a_nand_detect(struct mtd_info *mtd)
{
        printf("NANDFlash info:\nerasesize\t%d B\nwritesize\t%d B\noobsize  \t%d B\n",
			mtd->erasesize, mtd->writesize,mtd->oobsize );
        return (mtd->erasesize != 1<<17 || mtd->writesize != 1<<11 || mtd->oobsize != 1<<6);

}
static void ls2g1a_nand_init_info(struct ls2g1a_nand_info *info)
{
	info->num=0;
	info->size=0;
	info->cac_size = 0; 
	info->state = STATE_READY;

	info->cmd_prev = -1;
	info->page_addr = -1;

	nand_write(0x412, NAND_REG_TIMING);
	nand_write(0, NAND_REG_CS_RDY_MAP);

	info->dma_orderad = 0x0;
	info->dma_saddr = info->data_buff_phys;
	info->dma_daddr = DMA_ACCESS_ADDR;
	info->dma_length = 0x0;
	info->dma_step_length = 0x0;
	info->dma_step_times = 0x1;
	info->dma_cmd = 0x0;

	info->order_reg_addr = ORDER_REG_ADDR;
}

int ls2g1a_nand_pmon_info_init(struct ls2g1a_nand_info *info, struct mtd_info *mtd)
{
	info->drcmr_dat = 0xa2000000;	//DMA desc_addr a2000000
	info->drcmr_dat = (unsigned int)((info->drcmr_dat + 15) & ~0xff);
	info->drcmr_dat_phys = ((info->drcmr_dat) & 0x1fffffff) | 0x80000000;
	printf("desc addr 0x%x\n", info->drcmr_dat);
	printf("desc addr phys 0x%x\n", info->drcmr_dat_phys);

	info->mmio_base = NAND_REG_BASE;
	printf("nand reg base 0x%x\n", info->mmio_base);

	info->data_buff = (unsigned char *)DDR_PHYADDR;	//for saddr//ad000000
	memset(info->data_buff, 0, sizeof(unsigned int));
	info->data_buff_phys = ((unsigned int)(info->data_buff) & 0x1fffffff) | 0x80000000;
	printf("data_buff==0x%08x\n",info->data_buff);
	printf("data_buff_phys==0x%08x\n",info->data_buff_phys);

	info->data_ask = 0xa2000000;
	if(info->data_ask == NULL)
		return -1;
	info->data_ask_phys = ( info->data_ask & 0x1fffffff) |0x80000000;

	ls2g1a_nand_init_info(info);//nand reg, dma reg
	return 0;
}

static int nandwrite(int argc,char **argv)
{
	unsigned int cmd, val, addrr, addrc, op_num;
	struct ls2g1a_nand_info *info;
	unsigned int dma_cmd, dma_cnt;
	unsigned int addr; // base on byte

	if (argc != 3) {
		printf("nandwrite nand_addr val\n");
		return -1;
	}
	addr = strtoul(argv[1], 0, 0);
	val = strtoul(argv[2], 0, 0);
	info = ls2g1a_soc_mtd->priv;
	nand_write(val, info->data_buff);

	cmd = NAND_CMD_MAIN | NAND_CMD_SPARE | NAND_CMD_OP_WR | NAND_CMD_VALID;
	addrr = addr / NAND_PAGE_SIZE;
	addrc = addr % NAND_PAGE_SIZE;
	op_num = 4;
	nand_setup(cmd, addrc, addrr, op_num);

	/* dma configure */
	dma_cmd = 0x1 | (0x1 << 12);
	dma_cnt = ALIGN_DMA(op_num);
	dma_setup(info, dma_cmd, dma_cnt);

	printf("nand address: %08lx @ page %08x: %08x, op_num 0x%x\n",
				addr, addrr, addrc, op_num);
	return 0;
}

static int nandread(int argc,char **argv)
{
	unsigned int cmd, addrr, addrc;
	struct ls2g1a_nand_info *info;
	unsigned int buf;
	unsigned int addr;
	int op_num, dma_cmd, dma_cnt;

	if (argc != 2) {
		printf("nandread nand_addr\n");
		return -1;
	}
	memset((void *)DDR_PHYADDR, 0, sizeof(unsigned int));
	addr = strtoul(argv[1], 0, 0);

	cmd = NAND_CMD_OP_RD | NAND_CMD_VALID;
	addrr = addr / NAND_PAGE_SIZE;
	addrc = addr % NAND_PAGE_SIZE;
	op_num = ls2g1a_soc_mtd->oobsize + ls2g1a_soc_mtd->writesize;
	nand_setup(cmd, addrc, addrr, op_num);

	/* dma configure */
	info = ls2g1a_soc_mtd->priv;
	dma_cmd = 0x1;
	dma_cnt = ALIGN_DMA(op_num);
	dma_setup(info, dma_cmd, dma_cnt);

	buf = nand_read(DDR_PHYADDR);
	printf("nand address: %08lx @ page %08x: %08x ======  %08x\n",
			addr, addrr, addrc, buf);
	return 0;
}

static void nanderase(void)
{
	int i=0;
	
	nand_write(0x0, NAND_REG_CMD);
	nand_write(0x0, NAND_REG_CMD);
	nand_write(NAND_CMD_OP_RST | NAND_CMD_VALID, NAND_REG_CMD);
	nand_write(0x0, NAND_REG_ADDR_C);

	for(i = 0; i < 1024; i++){	// 1024 blocks
		printf("erase blockaddr: 0x%08x\n", i << (11 + 6));
		nand_write(0x0, NAND_REG_ADDR_C);   // 128K/block
		nand_write(i << 6, NAND_REG_ADDR_R);   //128K
		nand_write(NAND_CMD_OP_ER, NAND_REG_CMD);
		nand_write(NAND_CMD_OP_ER | NAND_CMD_VALID, NAND_REG_CMD);
		while((nand_read(NAND_REG_CMD) & NAND_CMD_DONE) == 0){
			udelay(80);
		}
		nand_write(0x0, NAND_REG_CMD);
	}
	printf("\nerase all nandflash ok...\n");
}

static int nand_read_id(void)
{
	unsigned char *data;
	unsigned int id_val_l = 0;
	unsigned int id_val_h = 0;
	int i, dev_id, maf_idx, busw;
	struct mtd_info *mtd = ls2g1a_soc_mtd;
	struct nand_chip *chip = mtd->priv;
	struct nand_flash_dev *type = NULL;
	unsigned int timing = 0;

	timing = nand_read(NAND_REG_TIMING);
	nand_write(0x30f0, NAND_REG_TIMING);
	nand_write(NAND_CMD_RD_ID | NAND_CMD_VALID, NAND_REG_CMD);
#define    IDL  *((volatile unsigned int*)(NAND_REG_IDL))  
#define    IDH  *((volatile unsigned int*)(NAND_REG_IDH))  
	while(1){
		while(((id_val_l |= IDL) & 0xff) == 0){
			id_val_h = IDH;
		}
		while (((id_val_h = IDH) & 0xff) == 0);
		break;
	}

	dev_id = (id_val_l >> 24);

	/* Lookup the flash id */
	for (i = 0; nand_flash_ids[i].name != NULL; i++) {
		if (dev_id == nand_flash_ids[i].id) {
			type =  &nand_flash_ids[i];
			break;
		}
	}

	if (!type){
		printf("No NAND device found!!!\n");
		return 1;
	}

	/* Try to identify manufacturer */
	for (maf_idx = 0; nand_manuf_ids[maf_idx].id != 0x0; maf_idx++) {
		if (nand_manuf_ids[maf_idx].id == (unsigned char)id_val_h)
			break;
	}

	busw = type->options & NAND_BUSWIDTH_16;

	printf("readid, NAND device: Manufacturer ID:"
			" 0x%02x, Chip ID: 0x%02x (%s %s)\n", id_val_h,
			(id_val_l >> 24), nand_manuf_ids[maf_idx].name, mtd->name);
	printf("readid, NAND bus width %d instead %d bit\n",
			(chip->options & NAND_BUSWIDTH_16) ? 16 : 8,
			busw ? 16 : 8);

	data = (unsigned char *)(((struct ls2g1a_nand_info *)(mtd->priv))->data_buff);
	nand_write(timing, NAND_REG_TIMING);
	data[0]  = (id_val_h & 0xff);
	data[1]  = (id_val_l & 0xff000000)>>24;
	data[2]  = (id_val_l & 0x00ff0000)>>16;
	data[3]  = (id_val_l & 0x0000ff00)>>8;
	return 0;
}

int ls2g1a_soc_nand_init(void)
{
	struct ls2g1a_nand_info *info;
	struct nand_chip *this;

	ls2g1a_soc_mtd = malloc(sizeof(struct mtd_info) + sizeof(struct ls2g1a_nand_info), M_DEVBUF, M_WAITOK);
	if (!ls2g1a_soc_mtd) {
		printf("Unable to allocate fcr_soc NAND MTD device structure.\n");
		return -ENOMEM;
	}
	memset(ls2g1a_soc_mtd, 0, sizeof(struct mtd_info) + sizeof(struct ls2g1a_nand_info));
	info = (struct ls2g1a_nand_info *)(&ls2g1a_soc_mtd[1]);

	this = &info->nand_chip;
	memset(this, 0, sizeof(struct nand_chip));

	ls2g1a_soc_mtd->priv = info;

	/* 15 us command delay time 从数据手册获知命令延迟时间 */
	this->chip_delay = 15;

	if(ls2g1a_nand_init(ls2g1a_soc_mtd)){//init mtd func, init desc, init mmio, reg
		printf("\n\nerror: PMON nandflash driver have some error!\n\n");
		return -ENXIO;
	}

	if (nand_scan(ls2g1a_soc_mtd, 1)) {
		free(ls2g1a_soc_mtd, M_DEVBUF);
		return -ENXIO;
	}
	if(ls2g1a_nand_detect(ls2g1a_soc_mtd)){
		printf("error: PMON driver don't support the NANDFlash!\n ");
		return -ENXIO;
	}

	/* Register the partitions */
	add_mtd_device(ls2g1a_soc_mtd, 0, 0x02000000, "kernel");
	add_mtd_device(ls2g1a_soc_mtd, 0x02000000, 0x06000000, "os");
	printf("init nand ok !\n");
	return 0;
}

static int nand_load(int argc, char *av[])
{
	int fd_r, fd_w, ret_r, ret_w, count;
	unsigned char *load_buf;

	if (argc != 2) {
		printf("input error ! must be : nand_load xxx.xxx.xxx.xxx/xxx (load file)\n");
		return -1;
	}
	
	load_buf = malloc(NAND_PAGE_PER_BLOCK * NAND_PAGE_SIZE, M_DEVBUF, M_WAITOK);
	if(load_buf == NULL){
		printf("malloc error !\n");
		return -1;
	}

	fd_r = open(av[1], O_RDONLY);
	if(fd_r < 0){
		printf("src open error !\n");
		return -1;
	}

	fd_w = open("/dev/fs/yaffs2@mtd0/vmlinux1", O_RDWR | O_CREAT | O_TRUNC);
	if(fd_w < 0){
		close(fd_r);	
		printf("dist vmlinux1 open error !\n");
		return -1;
	}

	printf("open write ok\n");
	count = 0;

	while(1){
		ret_r = read(fd_r, load_buf, NAND_PAGE_PER_BLOCK * NAND_PAGE_SIZE);
		if(ret_r < 0){
			printf("read error !\n");
			close(fd_r);	
			close(fd_w);
			return -1;
		}

		ret_w = write(fd_w, load_buf, ret_r);
		if(ret_w < 0){
			printf("write vmlinux1 error !\n");
			close(fd_r);	
			close(fd_w);
			return -1;
		}
		
		count += ret_r;
		printf("%d", count);

		if(ret_r < NAND_PAGE_PER_BLOCK * NAND_PAGE_SIZE){
			break;
		}
	}
	printf("\n");
	free(load_buf, M_DEVBUF);
	close(fd_r);	
	close(fd_w);

	return 0;
}

static const Cmd Cmds[] =
{
	{"MyCmds"},
	{"nandread", "val", 0, "nand read addr", nandread, 0, 99, CMD_REPEAT},
	{"nandwrite", "val", 0, "nand write addr val", nandwrite, 0, 99, CMD_REPEAT},
	{"nanderase", "val", 0, "hardware test", nanderase, 0, 99, CMD_REPEAT},
	{"nandreadid", "val", 0, "hardware test", nand_read_id, 0, 99, CMD_REPEAT},
	{"nandinit", "val", 0, "hardware test", ls2g1a_soc_nand_init, 0, 99, CMD_REPEAT},
	{"nand_load", "path", 0, "spi_nand_load from ", nand_load, 1, 99, 0},
	{0, 0}
};

static void init_cmd __P((void)) __attribute__ ((constructor));
static void init_cmd()
{
	cmdlist_expand(Cmds, 1);
}
