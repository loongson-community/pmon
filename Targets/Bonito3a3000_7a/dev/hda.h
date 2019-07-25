#ifndef _HDA_H_
#define _HDA_H_
/*
 * registers
 */
#define ICH6_REG_GCAP			0x00
#define ICH6_GCAP_64OK		(1 << 0)	/* 64bit address support */
#define ICH6_GCAP_NSDO		(3 << 1	)	/* # of serial data out signals */
#define ICH6_GCAP_BSS		(31 << 3)	/* # of bidirectional streams */
#define ICH6_GCAP_ISS		(15 << 8)	/* # of input streams */
#define ICH6_GCAP_OSS		(15 << 12)	/* # of output streams */
#define ICH6_REG_VMIN			0x02
#define ICH6_REG_VMAJ			0x03
#define ICH6_REG_OUTPAY			0x04
#define ICH6_REG_INPAY			0x06
#define ICH6_REG_GCTL			0x08
#define ICH6_GCTL_RESET		(1 << 0)	/* controller reset */
#define ICH6_GCTL_FCNTRL	(1 << 1)	/* flush control */
#define ICH6_GCTL_UNSOL		(1 << 8)	/* accept unsol. response enable */
#define ICH6_REG_WAKEEN			0x0c
#define ICH6_REG_STATESTS		0x0e
#define ICH6_REG_GSTS			0x10
#define ICH6_REG_OUTSTRMPAY		0x18
#define ICH6_REG_INSTRMPAY		0x1a
#define ICH6_GSTS_FSTS		(1 << 1)	/* flush status */
#define ICH6_REG_INTCTL			0x20
#define ICH6_REG_INTSTS			0x24
#define ICH6_REG_WALLCLK		0x30	/* 24Mhz source */
#define ICH6_REG_SYNC			0x38
#define ICH6_REG_CORBLBASE		0x40
#define ICH6_REG_CORBUBASE		0x44
#define ICH6_REG_CORBWP			0x48
#define ICH6_REG_CORBRP			0x4a
#define ICH6_CORBRP_RST		(1 << 15)	/* read pointer reset */
#define ICH6_REG_CORBCTL		0x4c
#define ICH6_CORBCTL_RUN	(1 << 1)	/* enable DMA */
#define ICH6_CORBCTL_CMEIE	(1 << 0)	/* enable memory error irq */
#define ICH6_REG_CORBSTS		0x4d
#define ICH6_CORBSTS_CMEI	(1 << 0)	/* memory error indication */
#define ICH6_REG_CORBSIZE		0x4e

#define ICH6_REG_RIRBLBASE		0x50
#define ICH6_REG_RIRBUBASE		0x54
#define ICH6_REG_RIRBWP			0x58
#define ICH6_RIRBWP_RST		(1 << 15)	/* write pointer reset */
#define ICH6_REG_RINTCNT		0x5a
#define ICH6_REG_RIRBCTL		0x5c
#define ICH6_RBCTL_IRQ_EN	(1 << 0)	/* enable IRQ */
#define ICH6_RBCTL_DMA_EN	(1 << 1)	/* enable DMA */
#define ICH6_RBCTL_OVERRUN_EN	(1 << 2)	/* enable overrun irq */
#define ICH6_REG_RIRBSTS		0x5d
#define ICH6_RBSTS_IRQ		(1 << 0)	/* response irq */
#define ICH6_RBSTS_OVERRUN	(1 << 2)	/* overrun irq */
#define ICH6_REG_RIRBSIZE		0x5e

#define ICH6_REG_IC			0x60
#define ICH6_REG_IR			0x64
#define ICH6_REG_IRS			0x68
#define ICH6_IRS_VALID		(1<<1)
#define ICH6_IRS_BUSY		(1<<0)

#define ICH6_REG_DPLBASE		0x70
#define ICH6_REG_DPUBASE		0x74
#define ICH6_DPLBASE_ENABLE		0x1	/* Enable position buffer */

/* SD offset: SDI0=0x80, SDI1=0xa0, ... SDO3=0x160 */
enum { SDI0, SDI1, SDI2, SDI3, SDO0, SDO1, SDO2, SDO3 };

/* stream register offsets from stream base */
#define ICH6_REG_SD_CTL			0x00
#define ICH6_REG_SD_STS			0x03
#define ICH6_REG_SD_LPIB		0x04
#define ICH6_REG_SD_CBL			0x08
#define ICH6_REG_SD_LVI			0x0c
#define ICH6_REG_SD_FIFOSIZE		0x10
#define ICH6_REG_SD_FORMAT		0x12
#define ICH6_REG_SD_BDLPL		0x18
#define ICH6_REG_SD_BDLPU		0x1c


#define hda_ls_writel(addr,val) *(volatile unsigned int*)(addr) = (val)
#define hda_ls_readl(addr) *(volatile unsigned int*)(addr)
#define ls_readl(addr) *(volatile unsigned int*)(addr)
#define ls_writel(addr,val) *(volatile unsigned int*)(addr) = (val)


#define hda_writel(base,reg,val) *(volatile unsigned int*)(base + ICH6_REG_##reg) = (val)
#define hda_writew(base,reg,val) *(volatile unsigned short*)(base + ICH6_REG_##reg) = (val)
#define hda_writeb(base,reg,val) *(volatile unsigned char*)(base + ICH6_REG_##reg) = (val)
#define hda_readl(base,reg) (*(volatile unsigned int*)(base + ICH6_REG_##reg))
#define hda_readw(base,reg) (*(volatile unsigned short*)(base + ICH6_REG_##reg))
#define hda_readb(base,reg) (*(volatile unsigned char*)(base + ICH6_REG_##reg))

#endif /*_HDA_H_*/
