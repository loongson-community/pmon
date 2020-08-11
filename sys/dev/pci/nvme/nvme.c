// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2017 NXP Semiconductors
 * Copyright (C) 2017 Bin Meng <bmeng.cn@gmail.com>
 */
#include "bpfilter.h"
#include <sys/param.h>
#include <sys/systm.h>
#include <sys/mbuf.h>
#include <sys/malloc.h>
#include <sys/kernel.h>
#include <sys/socket.h>
#include <sys/syslog.h>
#include <autoconf.h>
#include <sys/malloc.h>
#include <sys/buf.h>

#include <ctype.h>

#if defined(__NetBSD__) || defined(__OpenBSD__)

#include <sys/ioctl.h>
#include <sys/errno.h>
#include <sys/device.h>

#include <vm/vm.h>

#include <machine/cpu.h>
#include <machine/bus.h>
#include <machine/intr.h>

#include <dev/pci/pcivar.h>
#include <dev/pci/pcireg.h>
#include <dev/pci/pcidevs.h>

#include <dev/pci/if_fxpreg.h>

#else /* __FreeBSD__ */

#include <sys/sockio.h>

#include <machine/clock.h>	/* for DELAY */

#include <pci/pcivar.h>

#endif /* __NetBSD__ || __OpenBSD__ */

#include <machine/intr.h>
#include <machine/bus.h>

#include <dev/ata/atareg.h>
#include <dev/ic/wdcreg.h>

#include <linux/libata.h>
#include <part.h>
#include <pmon.h>

/* Maximum timeouts for each event */
#define WAIT_MS_SPINUP	20000
#define WAIT_MS_DATAIO	10000
#define WAIT_MS_FLUSH	5000
#define WAIT_MS_LINKUP	200
#define debug printf
#include <linux/list.h>

#include <linux/compat.h>
#include <sys/linux/types.h>
#define	ETIME		62	/* Timer expired */
#define readb(addr)     (*(volatile unsigned char *)(addr))
#define readw(addr)     (*(volatile unsigned short *)(addr))
#define readl(addr)     (*(volatile unsigned int *)(addr))

#define writeb(b,addr) ((*(volatile unsigned char *)(addr)) = (b))
#define writew(w,addr) ((*(volatile unsigned short *)(addr)) = (w))
#define writel(l,addr) ((*(volatile unsigned int *)(addr)) = (l))

#define __iomem
#define DIV_ROUND_UP(n,d)       (((n) + (d) - 1) / (d))
#define cpu_to_le64(x) (x)
#define cpu_to_le32(x)		(x)
#define cpu_to_le16(x) (x)
#define le64_to_cpu(x)		(x)
#define le32_to_cpu(x)		(x)
#define le16_to_cpu(x)		(x)
typedef unsigned char __u8;
typedef unsigned short __u16;
typedef unsigned __u32;
typedef unsigned long long  __u64;
typedef __u16 __le16; 
typedef __u32 __le32;
typedef __u64 __le64;
typedef int bool;
#define true  1
#define false 0
typedef unsigned long dma_addr_t;
typedef void* wait_queue_head_t;
unsigned long timer_get_us();
#define upper_32_bits(n) ((u32)(((n) >> 16) >> 16))
#define lower_32_bits(n) ((u32)(n))
#define flush_dcache_range(start, stop) pci_sync_cache(NULL, start, stop - start + 1, SYNC_W)
#define invalidate_dcache_range(start, stop) pci_sync_cache(NULL, start, stop - start + 1, SYNC_R)
#undef malloc
#undef free
#define memalign(a,size) kern_malloc(((size-1+a)&~(a-1)), M_DEVBUF, 0)
#define malloc(size) kern_malloc(size, M_DEVBUF, 0)
#define free(a)  kern_free(a, M_DEVBUF)
#include "nvme.h"
#define BLK_VEN_SIZE		40
#define BLK_PRD_SIZE		20
#define BLK_REV_SIZE		8
struct blk_desc {
	/*
	 * TODO: With driver model we should be able to use the parent
	 * device's uclass instead.
	 */
	int		devnum;		/* device number */
	unsigned char	part_type;	/* partition type */
	unsigned char	target;		/* target SCSI ID */
	unsigned char	lun;		/* target LUN */
	unsigned char	hwpart;		/* HW partition, e.g. for eMMC */
	unsigned char	type;		/* device type */
	unsigned char	removable;	/* removable device */
#ifdef CONFIG_LBA48
	/* device can use 48bit addr (ATA/ATAPI v7) */
	unsigned char	lba48;
#endif
	lbaint_t	lba;		/* number of blocks */
	unsigned long	blksz;		/* block size */
	int		log2blksz;	/* for convenience: log2(blksz) */
	char		vendor[BLK_VEN_SIZE + 1]; /* device vendor string */
	char		product[BLK_PRD_SIZE + 1]; /* device product number */
	char		revision[BLK_REV_SIZE + 1]; /* firmware revision */
	void	*bdev;
};

typedef struct nvme_t {
	struct device sc_dev;	/* General device infos */
	struct nvme_dev ndev;
	struct nvme_ns ns;
	struct pci_attach_args pa;
	//struct blk_desc *desc;
} nvme_t;

typedef struct nvme_blk_t {
	struct device sc_dev;	/* General device infos */
	nvme_t *parent;
	int no;
	struct nvme_ns ns;
	struct blk_desc desc;
	int bs;
} nvme_blk_t;


#define NVME_Q_DEPTH		2
#define NVME_AQ_DEPTH		2
#define NVME_SQ_SIZE(depth)	(depth * sizeof(struct nvme_command))
#define NVME_CQ_SIZE(depth)	(depth * sizeof(struct nvme_completion))
#define ADMIN_TIMEOUT		60
#define IO_TIMEOUT		30
#define MAX_PRP_POOL		512

enum nvme_queue_id {
	NVME_ADMIN_Q,
	NVME_IO_Q,
	NVME_Q_NUM,
};

/*
 * An NVM Express queue. Each device has at least two (one for admin
 * commands and one for I/O commands).
 */
struct nvme_queue {
	struct nvme_dev *dev;
	struct nvme_command *sq_cmds;
	struct nvme_completion *cqes;
	wait_queue_head_t sq_full;
	u32 __iomem *q_db;
	u16 q_depth;
	s16 cq_vector;
	u16 sq_head;
	u16 sq_tail;
	u16 cq_head;
	u16 qid;
	u8 cq_phase;
	u8 cqe_seen;
	unsigned long cmdid_data[];
};

static int nvme_wait_ready(struct nvme_dev *dev, bool enabled)
{
	u32 bit = enabled ? NVME_CSTS_RDY : 0;
	int timeout;
	ulong start;

	/* Timeout field in the CAP register is in 500 millisecond units */
	timeout = NVME_CAP_TIMEOUT(dev->cap) * 500;

	start = get_timer(0);
	while (get_timer(start) < timeout) {
		if ((readl(&dev->bar->csts) & NVME_CSTS_RDY) == bit)
			return 0;
	}

	return -ETIME;
}

static int nvme_setup_prps(struct nvme_dev *dev, u64 *prp2,
			   int total_len, u64 dma_addr)
{
	u32 page_size = dev->page_size;
	int offset = dma_addr & (page_size - 1);
	u64 *prp_pool;
	int length = total_len;
	int i, nprps;
	u32 prps_per_page = (page_size >> 3) - 1;
	u32 num_pages;

	length -= (page_size - offset);

	if (length <= 0) {
		*prp2 = 0;
		return 0;
	}

	if (length)
		dma_addr += (page_size - offset);

	if (length <= page_size) {
		*prp2 = dma_addr;
		return 0;
	}

	nprps = DIV_ROUND_UP(length, page_size);
	num_pages = DIV_ROUND_UP(nprps, prps_per_page);

	if (nprps > dev->prp_entry_num) {
		free(dev->prp_pool);
		/*
		 * Always increase in increments of pages.  It doesn't waste
		 * much memory and reduces the number of allocations.
		 */
		dev->prp_pool = memalign(page_size, num_pages * page_size);
		if (!dev->prp_pool) {
			printf("Error: malloc prp_pool fail\n");
			return -ENOMEM;
		}
		dev->prp_entry_num = prps_per_page * num_pages;
	}

	prp_pool = dev->prp_pool;
	i = 0;
	while (nprps) {
		if (i == ((page_size >> 3) - 1)) {
			*(prp_pool + i) = vtophys(cpu_to_le64((ulong)prp_pool +
					page_size));
			i = 0;
			prp_pool += page_size;
		}
		*(prp_pool + i++) = cpu_to_le64(dma_addr);
		dma_addr += page_size;
		nprps--;
	}
	*prp2 = vtophys((ulong)dev->prp_pool);

	flush_dcache_range((ulong)dev->prp_pool, (ulong)dev->prp_pool +
			   dev->prp_entry_num * sizeof(u64));

	return 0;
}

static __le16 nvme_get_cmd_id(void)
{
	static unsigned short cmdid;

	return cpu_to_le16((cmdid < USHRT_MAX) ? cmdid++ : 0);
}

static u16 nvme_read_completion_status(struct nvme_queue *nvmeq, u16 index)
{
	u64 start = (ulong)&nvmeq->cqes[index];
	u64 stop = start + sizeof(struct nvme_completion);

	invalidate_dcache_range(start, stop);

	return le16_to_cpu(readw(&(nvmeq->cqes[index].status)));
}

/**
 * nvme_submit_cmd() - copy a command into a queue and ring the doorbell
 *
 * @nvmeq:	The queue to use
 * @cmd:	The command to send
 */
static void nvme_submit_cmd(struct nvme_queue *nvmeq, struct nvme_command *cmd)
{
	u16 tail = nvmeq->sq_tail;

	memcpy(&nvmeq->sq_cmds[tail], cmd, sizeof(*cmd));
	flush_dcache_range((ulong)&nvmeq->sq_cmds[tail],
			   (ulong)&nvmeq->sq_cmds[tail] + sizeof(*cmd));

	if (++tail == nvmeq->q_depth)
		tail = 0;
	writel(tail, nvmeq->q_db);
	nvmeq->sq_tail = tail;
}

static int nvme_submit_sync_cmd(struct nvme_queue *nvmeq,
				struct nvme_command *cmd,
				u32 *result, unsigned timeout)
{
	u16 head = nvmeq->cq_head;
	u16 phase = nvmeq->cq_phase;
	u16 status;
	ulong start_time;
	ulong timeout_us = timeout * 100000;

	cmd->common.command_id = nvme_get_cmd_id();
	nvme_submit_cmd(nvmeq, cmd);

	start_time = timer_get_us();

	for (;;) {
		status = nvme_read_completion_status(nvmeq, head);
		if ((status & 0x01) == phase)
			break;
		if (timeout_us > 0 && (timer_get_us() - start_time)
		    >= timeout_us)
			return -ETIMEDOUT;
	}

	status >>= 1;
	if (status) {
		printf("ERROR: status = %x, phase = %d, head = %d\n",
		       status, phase, head);
		status = 0;
		if (++head == nvmeq->q_depth) {
			head = 0;
			phase = !phase;
		}
		writel(head, nvmeq->q_db + nvmeq->dev->db_stride);
		nvmeq->cq_head = head;
		nvmeq->cq_phase = phase;

		return -EIO;
	}

	if (result)
		*result = le32_to_cpu(readl(&(nvmeq->cqes[head].result)));

	if (++head == nvmeq->q_depth) {
		head = 0;
		phase = !phase;
	}
	writel(head, nvmeq->q_db + nvmeq->dev->db_stride);
	nvmeq->cq_head = head;
	nvmeq->cq_phase = phase;

	return status;
}

static int nvme_submit_admin_cmd(struct nvme_dev *dev, struct nvme_command *cmd,
				 u32 *result)
{
	return nvme_submit_sync_cmd(dev->queues[NVME_ADMIN_Q], cmd,
				    result, ADMIN_TIMEOUT);
}

static struct nvme_queue *nvme_alloc_queue(struct nvme_dev *dev,
					   int qid, int depth)
{
	struct nvme_queue *nvmeq = malloc(sizeof(*nvmeq));
	if (!nvmeq)
		return NULL;
	memset(nvmeq, 0, sizeof(*nvmeq));

	nvmeq->cqes = (void *)memalign(4096, NVME_CQ_SIZE(depth));
	if (!nvmeq->cqes)
		goto free_nvmeq;
	memset((void *)nvmeq->cqes, 0, NVME_CQ_SIZE(depth));

	nvmeq->sq_cmds = (void *)memalign(4096, NVME_SQ_SIZE(depth));
	if (!nvmeq->sq_cmds)
		goto free_queue;
	memset((void *)nvmeq->sq_cmds, 0, NVME_SQ_SIZE(depth));

	nvmeq->dev = dev;

	nvmeq->cq_head = 0;
	nvmeq->cq_phase = 1;
	nvmeq->q_db = &dev->dbs[qid * 2 * dev->db_stride];
	nvmeq->q_depth = depth;
	nvmeq->qid = qid;
	dev->queue_count++;
	dev->queues[qid] = nvmeq;

	return nvmeq;

 free_queue:
	free((void *)nvmeq->cqes);
 free_nvmeq:
	free(nvmeq);

	return NULL;
}

static int nvme_delete_queue(struct nvme_dev *dev, u8 opcode, u16 id)
{
	struct nvme_command c;

	memset(&c, 0, sizeof(c));
	c.delete_queue.opcode = opcode;
	c.delete_queue.qid = cpu_to_le16(id);

	return nvme_submit_admin_cmd(dev, &c, NULL);
}

static int nvme_delete_sq(struct nvme_dev *dev, u16 sqid)
{
	return nvme_delete_queue(dev, nvme_admin_delete_sq, sqid);
}

static int nvme_delete_cq(struct nvme_dev *dev, u16 cqid)
{
	return nvme_delete_queue(dev, nvme_admin_delete_cq, cqid);
}

static int nvme_enable_ctrl(struct nvme_dev *dev)
{
	dev->ctrl_config &= ~NVME_CC_SHN_MASK;
	dev->ctrl_config |= NVME_CC_ENABLE;
	writel(cpu_to_le32(dev->ctrl_config), &dev->bar->cc);

	return nvme_wait_ready(dev, true);
}

static int nvme_disable_ctrl(struct nvme_dev *dev)
{
	dev->ctrl_config &= ~NVME_CC_SHN_MASK;
	dev->ctrl_config &= ~NVME_CC_ENABLE;
	writel(cpu_to_le32(dev->ctrl_config), &dev->bar->cc);

	return nvme_wait_ready(dev, false);
}

static void nvme_free_queue(struct nvme_queue *nvmeq)
{
	free((void *)nvmeq->cqes);
	free(nvmeq->sq_cmds);
	free(nvmeq);
}

static void nvme_free_queues(struct nvme_dev *dev, int lowest)
{
	int i;

	for (i = dev->queue_count - 1; i >= lowest; i--) {
		struct nvme_queue *nvmeq = dev->queues[i];
		dev->queue_count--;
		dev->queues[i] = NULL;
		nvme_free_queue(nvmeq);
	}
}

static void nvme_init_queue(struct nvme_queue *nvmeq, u16 qid)
{
	struct nvme_dev *dev = nvmeq->dev;

	nvmeq->sq_tail = 0;
	nvmeq->cq_head = 0;
	nvmeq->cq_phase = 1;
	nvmeq->q_db = &dev->dbs[qid * 2 * dev->db_stride];
	memset((void *)nvmeq->cqes, 0, NVME_CQ_SIZE(nvmeq->q_depth));
	flush_dcache_range((ulong)nvmeq->cqes,
			   (ulong)nvmeq->cqes + NVME_CQ_SIZE(nvmeq->q_depth));
	dev->online_queues++;
}

static int nvme_configure_admin_queue(struct nvme_dev *dev)
{
	int result;
	u32 aqa;
	u64 cap = dev->cap;
	struct nvme_queue *nvmeq;
	/* most architectures use 4KB as the page size */
	unsigned page_shift = 12;
	unsigned dev_page_min = NVME_CAP_MPSMIN(cap) + 12;
	unsigned dev_page_max = NVME_CAP_MPSMAX(cap) + 12;

	if (page_shift < dev_page_min) {
		debug("Device minimum page size (%u) too large for host (%u)\n",
		      1 << dev_page_min, 1 << page_shift);
		return -ENODEV;
	}

	if (page_shift > dev_page_max) {
		debug("Device maximum page size (%u) smaller than host (%u)\n",
		      1 << dev_page_max, 1 << page_shift);
		page_shift = dev_page_max;
	}

	result = nvme_disable_ctrl(dev);
	if (result < 0)
		return result;

	nvmeq = dev->queues[NVME_ADMIN_Q];
	if (!nvmeq) {
		nvmeq = nvme_alloc_queue(dev, 0, NVME_AQ_DEPTH);
		if (!nvmeq)
			return -ENOMEM;
	}

	aqa = nvmeq->q_depth - 1;
	aqa |= aqa << 16;
	aqa |= aqa << 16;

	dev->page_size = 1 << page_shift;

	dev->ctrl_config = NVME_CC_CSS_NVM;
	dev->ctrl_config |= (page_shift - 12) << NVME_CC_MPS_SHIFT;
	dev->ctrl_config |= NVME_CC_ARB_RR | NVME_CC_SHN_NONE;
	dev->ctrl_config |= NVME_CC_IOSQES | NVME_CC_IOCQES;

	writel(aqa, &dev->bar->aqa);
	nvme_writeq(vtophys((ulong)nvmeq->sq_cmds), &dev->bar->asq);
	nvme_writeq(vtophys((ulong)nvmeq->cqes), &dev->bar->acq);

	result = nvme_enable_ctrl(dev);
	if (result)
		goto free_nvmeq;

	nvmeq->cq_vector = 0;

	nvme_init_queue(dev->queues[NVME_ADMIN_Q], 0);

	return result;

 free_nvmeq:
	nvme_free_queues(dev, 0);

	return result;
}

static int nvme_alloc_cq(struct nvme_dev *dev, u16 qid,
			    struct nvme_queue *nvmeq)
{
	struct nvme_command c;
	int flags = NVME_QUEUE_PHYS_CONTIG | NVME_CQ_IRQ_ENABLED;

	memset(&c, 0, sizeof(c));
	c.create_cq.opcode = nvme_admin_create_cq;
	c.create_cq.prp1 = vtophys(cpu_to_le64((ulong)nvmeq->cqes));
	c.create_cq.cqid = cpu_to_le16(qid);
	c.create_cq.qsize = cpu_to_le16(nvmeq->q_depth - 1);
	c.create_cq.cq_flags = cpu_to_le16(flags);
	c.create_cq.irq_vector = cpu_to_le16(nvmeq->cq_vector);

	return nvme_submit_admin_cmd(dev, &c, NULL);
}

static int nvme_alloc_sq(struct nvme_dev *dev, u16 qid,
			    struct nvme_queue *nvmeq)
{
	struct nvme_command c;
	int flags = NVME_QUEUE_PHYS_CONTIG | NVME_SQ_PRIO_MEDIUM;

	memset(&c, 0, sizeof(c));
	c.create_sq.opcode = nvme_admin_create_sq;
	c.create_sq.prp1 = vtophys(cpu_to_le64((ulong)nvmeq->sq_cmds));
	c.create_sq.sqid = cpu_to_le16(qid);
	c.create_sq.qsize = cpu_to_le16(nvmeq->q_depth - 1);
	c.create_sq.sq_flags = cpu_to_le16(flags);
	c.create_sq.cqid = cpu_to_le16(qid);

	return nvme_submit_admin_cmd(dev, &c, NULL);
}

int nvme_identify(struct nvme_dev *dev, unsigned nsid,
		  unsigned cns, dma_addr_t dma_addr)
{
	struct nvme_command c;
	u32 page_size = dev->page_size;
	int offset = dma_addr & (page_size - 1);
	int length = sizeof(struct nvme_id_ctrl);
	int ret;

	memset(&c, 0, sizeof(c));
	c.identify.opcode = nvme_admin_identify;
	c.identify.nsid = cpu_to_le32(nsid);
	c.identify.prp1 = cpu_to_le64(dma_addr);

	length -= (page_size - offset);
	if (length <= 0) {
		c.identify.prp2 = 0;
	} else {
		dma_addr += (page_size - offset);
		c.identify.prp2 = cpu_to_le64(dma_addr);
	}

	c.identify.cns = cpu_to_le32(cns);

	ret = nvme_submit_admin_cmd(dev, &c, NULL);
	if (!ret)
		invalidate_dcache_range(_pci_cpumap(dma_addr, 0),
					_pci_cpumap(dma_addr + sizeof(struct nvme_id_ctrl), 0));

	return ret;
}

int nvme_get_features(struct nvme_dev *dev, unsigned fid, unsigned nsid,
		      dma_addr_t dma_addr, u32 *result)
{
	struct nvme_command c;

	memset(&c, 0, sizeof(c));
	c.features.opcode = nvme_admin_get_features;
	c.features.nsid = cpu_to_le32(nsid);
	c.features.prp1 = cpu_to_le64(dma_addr);
	c.features.fid = cpu_to_le32(fid);

	/*
	 * TODO: add cache invalidate operation when the size of
	 * the DMA buffer is known
	 */

	return nvme_submit_admin_cmd(dev, &c, result);
}

int nvme_set_features(struct nvme_dev *dev, unsigned fid, unsigned dword11,
		      dma_addr_t dma_addr, u32 *result)
{
	struct nvme_command c;

	memset(&c, 0, sizeof(c));
	c.features.opcode = nvme_admin_set_features;
	c.features.prp1 = cpu_to_le64(dma_addr);
	c.features.fid = cpu_to_le32(fid);
	c.features.dword11 = cpu_to_le32(dword11);

	/*
	 * TODO: add cache flush operation when the size of
	 * the DMA buffer is known
	 */

	return nvme_submit_admin_cmd(dev, &c, result);
}

static int nvme_create_queue(struct nvme_queue *nvmeq, int qid)
{
	struct nvme_dev *dev = nvmeq->dev;
	int result;

	nvmeq->cq_vector = qid - 1;
	result = nvme_alloc_cq(dev, qid, nvmeq);
	if (result < 0)
		goto release_cq;

	result = nvme_alloc_sq(dev, qid, nvmeq);
	if (result < 0)
		goto release_sq;

	nvme_init_queue(nvmeq, qid);

	return result;

 release_sq:
	nvme_delete_sq(dev, qid);
 release_cq:
	nvme_delete_cq(dev, qid);

	return result;
}

static int nvme_set_queue_count(struct nvme_dev *dev, int count)
{
	int status;
	u32 result;
	u32 q_count = (count - 1) | ((count - 1) << 16);

	status = nvme_set_features(dev, NVME_FEAT_NUM_QUEUES,
			q_count, 0, &result);

	if (status < 0)
		return status;
	if (status > 1)
		return 0;

	return min(result & 0xffff, result >> 16) + 1;
}

static void nvme_create_io_queues(struct nvme_dev *dev)
{
	unsigned int i;

	for (i = dev->queue_count; i <= dev->max_qid; i++)
		if (!nvme_alloc_queue(dev, i, dev->q_depth))
			break;

	for (i = dev->online_queues; i <= dev->queue_count - 1; i++)
		if (nvme_create_queue(dev->queues[i], i))
			break;
}

static int nvme_setup_io_queues(struct nvme_dev *dev)
{
	int nr_io_queues;
	int result;

	nr_io_queues = 1;
	result = nvme_set_queue_count(dev, nr_io_queues);
	if (result <= 0)
		return result;

	dev->max_qid = nr_io_queues;

	/* Free previously allocated queues */
	nvme_free_queues(dev, nr_io_queues + 1);
	nvme_create_io_queues(dev);

	return 0;
}

static int nvme_get_info_from_identify(struct nvme_dev *dev)
{
	struct nvme_id_ctrl *ctrl;
	int ret;
	int shift = NVME_CAP_MPSMIN(dev->cap) + 12;

	ctrl = memalign(dev->page_size, sizeof(struct nvme_id_ctrl));
	if (!ctrl)
		return -ENOMEM;

	ret = nvme_identify(dev, 0, 1, (dma_addr_t)vtophys((long)ctrl));
	if (ret) {
		free(ctrl);
		return -EIO;
	}

	dev->nn = le32_to_cpu(ctrl->nn);
	dev->vwc = ctrl->vwc;
	memcpy(dev->serial, ctrl->sn, sizeof(ctrl->sn));
	memcpy(dev->model, ctrl->mn, sizeof(ctrl->mn));
	memcpy(dev->firmware_rev, ctrl->fr, sizeof(ctrl->fr));
	if (ctrl->mdts)
		dev->max_transfer_shift = (ctrl->mdts + shift);
	else {
		/*
		 * Maximum Data Transfer Size (MDTS) field indicates the maximum
		 * data transfer size between the host and the controller. The
		 * host should not submit a command that exceeds this transfer
		 * size. The value is in units of the minimum memory page size
		 * and is reported as a power of two (2^n).
		 *
		 * The spec also says: a value of 0h indicates no restrictions
		 * on transfer size. But in nvme_blk_read/write() below we have
		 * the following algorithm for maximum number of logic blocks
		 * per transfer:
		 *
		 * u16 lbas = 1 << (dev->max_transfer_shift - ns->lba_shift);
		 *
		 * In order for lbas not to overflow, the maximum number is 15
		 * which means dev->max_transfer_shift = 15 + 9 (ns->lba_shift).
		 * Let's use 20 which provides 1MB size.
		 */
		dev->max_transfer_shift = 20;
	}

	free(ctrl);
	return 0;
}


static int nvme_blk_probe(struct nvme_blk_t *udev)
{
	struct nvme_dev *ndev = &udev->parent->ndev;
	struct blk_desc *desc = &udev->desc;
	struct nvme_ns *ns = &udev->ns;
	u8 flbas;
	struct nvme_id_ns *id;

	id = memalign(ndev->page_size, sizeof(struct nvme_id_ns));
	if (!id)
		return -ENOMEM;

	memset(ns, 0, sizeof(*ns));
	ns->dev = ndev;
	/* extract the namespace id from the block device name */
	ns->ns_id = udev->sc_dev.dv_unit + 1;
	if (nvme_identify(ndev, ns->ns_id, 0, (dma_addr_t)vtophys((long)id))) {
		free(id);
		return -EIO;
	}

	memcpy(&ns->eui64, &id->eui64, sizeof(id->eui64));
	flbas = id->flbas & NVME_NS_FLBAS_LBA_MASK;
	ns->flbas = flbas;
	ns->lba_shift = id->lbaf[flbas].ds;
	ns->mode_select_num_blocks = le64_to_cpu(id->nsze);
	ns->mode_select_block_len = 1 << ns->lba_shift;
	list_add(&ns->list, &ndev->namespaces);

	desc->lba = ns->mode_select_num_blocks;
	desc->log2blksz = ns->lba_shift;
	desc->blksz = 1 << ns->lba_shift;
	desc->bdev = udev;
	sprintf(desc->vendor, "0x%.4x", PCI_VENDOR(udev->parent->pa.pa_id));
	memcpy(desc->product, ndev->serial, sizeof(ndev->serial));
	memcpy(desc->revision, ndev->firmware_rev, sizeof(ndev->firmware_rev));
	udev->bs = 1 << desc->log2blksz;

	free(id);
	return 0;
}

static ulong nvme_blk_rw(struct nvme_blk_t *udev, lbaint_t blknr,
			 lbaint_t blkcnt, void *buffer, bool read)
{
	struct nvme_ns *ns = &udev->ns;
	struct nvme_dev *dev = ns->dev;
	struct nvme_command c;
	struct blk_desc *desc = &udev->desc;
	int status;
	u64 prp2;
	u64 total_len = blkcnt << desc->log2blksz;
	u64 temp_len = total_len;

	u64 slba = blknr;
	u16 lbas = 1 << (dev->max_transfer_shift - ns->lba_shift);
	u64 total_lbas = blkcnt;

	flush_dcache_range((unsigned long)buffer,
			   (unsigned long)buffer + total_len);

	c.rw.opcode = read ? nvme_cmd_read : nvme_cmd_write;
	c.rw.flags = 0;
	c.rw.nsid = cpu_to_le32(ns->ns_id);
	c.rw.control = 0;
	c.rw.dsmgmt = 0;
	c.rw.reftag = 0;
	c.rw.apptag = 0;
	c.rw.appmask = 0;
	c.rw.metadata = 0;

	while (total_lbas) {
		if (total_lbas < lbas) {
			lbas = (u16)total_lbas;
			total_lbas = 0;
		} else {
			total_lbas -= lbas;
		}

		if (nvme_setup_prps(dev, &prp2,
				    lbas << ns->lba_shift, vtophys((ulong)buffer)))
			return -EIO;
		c.rw.slba = cpu_to_le64(slba);
		slba += lbas;
		c.rw.length = cpu_to_le16(lbas - 1);
		c.rw.prp1 = vtophys(cpu_to_le64((ulong)buffer));
		c.rw.prp2 = cpu_to_le64(prp2);
		status = nvme_submit_sync_cmd(dev->queues[NVME_IO_Q],
				&c, NULL, IO_TIMEOUT);
		if (status)
			break;
		temp_len -= (u32)lbas << ns->lba_shift;
		buffer += lbas << ns->lba_shift;
	}

	if (read)
		invalidate_dcache_range((unsigned long)buffer,
					(unsigned long)buffer + total_len);

	return (total_len - temp_len) >> desc->log2blksz;
}

static ulong nvme_blk_read(struct nvme_blk_t *udev, lbaint_t blknr,
			   lbaint_t blkcnt, void *buffer)
{
	return nvme_blk_rw(udev, blknr, blkcnt, buffer, true);
}

static ulong nvme_blk_write(struct nvme_blk_t *udev, lbaint_t blknr,
			    lbaint_t blkcnt, const void *buffer)
{
	return nvme_blk_rw(udev, blknr, blkcnt, (void *)buffer, false);
}

static int nvme_match(struct device *parent, void *match, void *aux)
{
	return 1;
}

static void nvme_attach(struct device *parent,
			   struct device *self, void *aux)
{
	int err;
	struct nvme_blk_t *blkdev = (struct nvme_t *)self;
	struct nvme_t *udev = parent;
	int no = *(int *)aux;
	blkdev->no = no;
        blkdev->parent = parent;

	nvme_blk_probe(blkdev);
}

struct cfattach nvme_ca = {
	sizeof(struct nvme_blk_t), nvme_match, nvme_attach,
};

struct cfdriver nvme_cd = {
	NULL, "nvme", DV_DISK
};


static int nvme_probe(struct nvme_t *udev)
{
	int i, ret;
	struct nvme_dev *ndev = &udev->ndev;

	ndev->instance = udev->sc_dev.dv_unit;

	INIT_LIST_HEAD(&ndev->namespaces);
	//ndev->bar = dm_pci_map_bar(udev, PCI_BASE_ADDRESS_0,
	//		PCI_REGION_MEM);
	if (readl(&ndev->bar->csts) == -1) {
		ret = -ENODEV;
		printf("Error: %s: Out of memory!\n", udev->sc_dev.dv_xname);
		goto free_nvme;
	}

	ndev->queues = malloc(NVME_Q_NUM * sizeof(struct nvme_queue *));
	if (!ndev->queues) {
		ret = -ENOMEM;
		printf("Error: %s: Out of memory!\n", udev->sc_dev.dv_xname);
		goto free_nvme;
	}
	memset(ndev->queues, 0, NVME_Q_NUM * sizeof(struct nvme_queue *));

	ndev->cap = nvme_readq(&ndev->bar->cap);
	ndev->q_depth = min_t(int, NVME_CAP_MQES(ndev->cap) + 1, NVME_Q_DEPTH);
	ndev->db_stride = 1 << NVME_CAP_STRIDE(ndev->cap);
	ndev->dbs = ((void __iomem *)ndev->bar) + 4096;

	ret = nvme_configure_admin_queue(ndev);
	if (ret)
		goto free_queue;

	/* Allocate after the page size is known */
	ndev->prp_pool = memalign(ndev->page_size, MAX_PRP_POOL);
	if (!ndev->prp_pool) {
		ret = -ENOMEM;
		printf("Error: %s: Out of memory!\n", udev->sc_dev.dv_xname);
		goto free_nvme;
	}
	ndev->prp_entry_num = MAX_PRP_POOL >> 3;

	ret = nvme_setup_io_queues(ndev);
	if (ret)
		goto free_queue;

	nvme_get_info_from_identify(ndev);

	/* Create a blk device for each namespace */
	for (i = 0; i < ndev->nn; i++) {
		/*
		 * Encode the namespace id to the device name so that
		 * we can extract it when doing the probe.
		 */

		/* The real blksz and size will be set by nvme_blk_probe() */
		config_found(udev, (void *)&i, NULL);
	}

	return 0;

free_queue:
	free((void *)ndev->queues);
free_nvme:
	return ret;
}



#define PCI_CLASS_STORAGE_EXPRESS	0x010802
static int pcinvme_match(struct device *parent, void *match, void *aux)
{
	struct pci_attach_args *pa = aux;

	if((pa->pa_class >> 8) == PCI_CLASS_STORAGE_EXPRESS)
		return 1;

	return 0;
}

static void pcinvme_attach(struct device *parent,
			   struct device *self, void *aux)
{
	int err;
	struct nvme_t *udev = (struct nvme_t *)self;
	struct pci_attach_args *pa = aux;
	bus_space_tag_t memt = pa->pa_memt;
	bus_addr_t membasep;
	bus_size_t memsizep;

	if (pci_mem_find(NULL, pa->pa_tag, 0x10, &membasep, &memsizep, NULL)) {
		printf(" Can't find mem space\n");
		return;
	}
	printf("Found memory space: memt->bus_base=0x%x, baseaddr=0x%x"
	       "size=0x%x\n", memt->bus_base, (u32) (membasep),
	       (u32) (memsizep));

	udev->pa = *pa;
	udev->ndev.bar = memt->bus_base | membasep;
	nvme_probe(udev);
}

struct cfattach pcinvme_ca = {
	sizeof(struct nvme_t), pcinvme_match, pcinvme_attach,
};

struct cfdriver pcinvme_cd = {
	NULL, "pcinvme", DV_DULL
};

#define nvme_lookup(cd, dev)	\
	(struct nvme_blk_t *)device_lookup(&cd, minor(dev))

void nvme_strategy(struct buf *bp)
{
	struct nvme_blk_t *priv;
	unsigned int blkno, blkcnt;
	int ret;
	priv = nvme_lookup(nvme_cd, bp->b_dev);
	if (!priv) {
		printf("%s<%d>: no found device\n", __func__, __LINE__);
		return;
	}

	blkno = bp->b_blkno;

	blkno = blkno / (priv->bs / DEV_BSIZE);
	blkcnt = howmany(bp->b_bcount, priv->bs);

	/* Valid request ? */
	if (bp->b_blkno < 0 ||
	    (bp->b_bcount % priv->bs) != 0 ||
	    (bp->b_bcount / priv->bs) >= (1 << NBBY)) {
		bp->b_error = EINVAL;
		printf("Invalid request\n");
		goto bad;
	}

	/* If it's a null transfer, return immediately. */
	if (bp->b_bcount == 0)
		goto done;

	if (bp->b_flags & B_READ) {
		ret = nvme_blk_read(priv, blkno, blkcnt, bp->b_data);
		if (ret != blkcnt)
			bp->b_flags |= B_ERROR;
		dotik(30000, 0);
	} else {
		ret = nvme_blk_write(priv, blkno, blkcnt, bp->b_data);
		if (ret != blkcnt)
			bp->b_flags |= B_ERROR;
		dotik(30000, 0);
	}
done:
	biodone(bp);
	return;
bad:
	bp->b_flags |= B_ERROR;
	biodone(bp);
}

int nvme_open(dev_t dev, int flag, int fmt, struct proc *p)
{
	return 0;
}

int nvme_close(dev_t dev, int flag, int fmt, struct proc *p)
{
	return 0;
}

int nvme_read(dev_t dev, struct uio *uio, int flags)
{
	return physio(nvme_strategy, NULL, dev, B_READ, minphys, uio);
}

int nvme_write(dev_t dev, struct uio *uio, int flags)
{
	return (physio(nvme_strategy, NULL, dev, B_WRITE, minphys, uio));
}
