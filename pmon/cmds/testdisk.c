#include <stdio.h>
//#include "pmon/dev/gt64420reg.h"
#include <termio.h>
#include <string.h>
#include <setjmp.h>
#include <sys/endian.h>
#include <ctype.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#ifdef _KERNEL
#undef _KERNEL
#include <sys/ioctl.h>
#define _KERNEL
#else
#include <sys/ioctl.h>
#endif

#include <machine/cpu.h>

#include <pmon.h>
#include <dev/pci/pcivar.h>
#include <machine/pio.h>
#include <dev/pci/pcidevs.h>
#include <flash.h>
//#include <target/ev64420.h>

#undef	PCI_CACHE_LINE_SIZE
#define PCI_CACHE_LINE_SIZE 0x0c    /* 8 bits */

#define PCI_LATENCY_TIMER   0x0d    /* 8 bits */
#define PCI_HEADER_TYPE     0x0e    /* 8 bits */

#undef  PCI_COMMAND
#define PCI_COMMAND     0x04    /* 16 bits */

#define  PCI_COMMAND_IO     0x1 /* Enable response in I/O space */
#define  PCI_COMMAND_MEMORY 0x2 /* Enable response in Memory space */
#define  PCI_COMMAND_MASTER 0x4 /* Enable bus mastering */


#define IDE0_PIO_BASE		(0xbfd00000 + 0x1f0)
#define IDE_DATA_REG		(IDE0_PIO_BASE + 0)
#define IDE_ERROR_REG		(IDE0_PIO_BASE + 1)
#define IDE_NSECTOR_REG		(IDE0_PIO_BASE + 2)
#define IDE_SECTOR_REG		(IDE0_PIO_BASE + 3)
#define IDE_LCYL_REG		(IDE0_PIO_BASE + 4)
#define IDE_HCYL_REG		(IDE0_PIO_BASE + 5)
#define IDE_SELECT_REG		(IDE0_PIO_BASE + 6)
#define IDE_STATUS_REG		(IDE0_PIO_BASE + 7)
#define IDE_CONTROL_REG		(IDE0_PIO_BASE + 8)
#define IDE_IRQ_REG			(IDE0_PIO_BASE + 9)

#define IDE_FEATURE_REG     IDE_ERROR_REG
#define IDE_COMMAND_REG     IDE_STATUS_REG
#define IDE_ALTSTATUS_REG   IDE_CONTROL_REG
#define IDE_IREASON_REG     IDE_NSECTOR_REG
#define IDE_BCOUNTL_REG     IDE_LCYL_REG
#define IDE_BCOUNTH_REG     IDE_HCYL_REG

#define IDE0_DMA_BASE		(0xbfd00000 + 0xcc0)		/* fixme */
#define DMA_COMMAND			(IDE0_DMA_BASE)
#define DMA_VENDOR1			(IDE0_DMA_BASE + 1)
#define DMA_STATUS			(IDE0_DMA_BASE + 2)
#define DMA_VENDOR3			(IDE0_DMA_BASE + 3)
#define DMA_PRDTABLE		(IDE0_DMA_BASE + 4)

#define TEST_SECT			400
#define TRANS_SECT			1
#define LBA_MODE			0xe0
#define WIN_READDMA_EXT		0x25 /* 48-Bit */
#define WIN_WRITEDMA_EXT	0x35 /* 48-Bit */
#define WIN_READDMA         0xC8 /* read sectors using DMA transfers */
#define WIN_WRITEDMA        0xCA /* write sectors using DMA transfers */
#define PRDTABLE_BASE		(0xa3000000)
/*#define PRDTABLE_BASE_DMA	(PRDTABLE_BASE & 0x0fffffff)*/
#define PRDTABLE_BASE_DMA	(0x83000000)
#define BUF_BASE			(0xa5000000)
/*#define BUF_BASE_DMA		(BUF_BASE & 0x0fffffff)*/
#define BUF_BASE_DMA		(0x85000000)

#define ERR_STAT		0x01
#define INDEX_STAT		0x02
#define ECC_STAT		0x04	/* Corrected error */
#define DRQ_STAT		0x08
#define SEEK_STAT		0x10
#define SRV_STAT		0x10
#define WRERR_STAT		0x20
#define READY_STAT		0x40
#define BUSY_STAT		0x80

#define DRIVE_READY		(READY_STAT  | SEEK_STAT)

static int testdisk_init = 0;
static int sect_per_trans = 0;
typedef union {
	unsigned short all		:16;
	struct {
		unsigned low		:8;
		unsigned high		:8;
	} b;
} ata_nsector_t;

void initialize(void) 
{
	unsigned char val = 0;
	unsigned long tag;

#if 1 /* for longmeng 2006 */
	tag = _pci_make_tag(0, 5, 1);
    _pci_conf_writen(tag, PCI_CACHE_LINE_SIZE, 0x20, 1);
    _pci_conf_writen(tag, PCI_LATENCY_TIMER, 0x20, 1);
    _pci_conf_writen(tag, PCI_COMMAND, PCI_COMMAND_IO|PCI_COMMAND_MEMORY|PCI_COMMAND_MASTER, 1);
    _pci_conf_writen(tag, 0x40, 0x0b, 1);   /* FIXME: Might depend on drives connected */
    val = _pci_conf_readn(tag,  0x4, 1);   /* FIXME: Might depend on drives connected */
    _pci_conf_writen(tag, 0x4, val | 5, 1);
    val = _pci_conf_readn(tag,  0x4, 1);   /* FIXME: Might depend on drives connected */
    _pci_conf_writen(tag, 0x41, 0x02, 1);
    /* legacy mode */
    _pci_conf_writen(tag, 0x42, 0x09, 1);
    _pci_conf_writen(tag, 0x43, 0x05, 1);
    _pci_conf_writen(tag, 0x44, 0x0, 1);
    _pci_conf_writen(tag, 0x45, 0x0, 1);
    //_pci_conf_writen(tag, 0x4e, 0x22, 1);   /* FIXME: Not documented, but set in PC bios */
    //_pci_conf_writen(tag, 0x4f, 0x20, 1);   /* FIXME: Not documented */
#else /* for nb2005 */
	unsigned long temp, i;
	tag = _pci_make_tag(0, 17, 1);  /* fixme */
    /* set IO_ENABLE */
    temp = _pci_conf_read(tag, PCI_COMMAND);
	if (temp & 0x1) {
	  temp &= ~0x1;
	  _pci_conf_write(tag, PCI_COMMAND, temp);
	}
    /* set IDE timing registers */
    _pci_conf_write(tag, 0x40, /*0x80778077);*/ 0xa377a377);
    /* set bus master base address to 0xcc00 */
    _pci_conf_write(tag, 0x20, 0x0000cc00);
	/*_pci_conf_writen(tag, 0x48, 0x03, 1);*/
	temp = inb(0xbfd0cc02);
	temp |= 0x20;
	outb(0xbfd0cc02, temp);
	temp = inb(0xbfd0cc02);
	printf("bfd0cc02 = %x\n", temp);

    temp |= PCI_COMMAND_IO|PCI_COMMAND_MEMORY|PCI_COMMAND_MASTER;
    _pci_conf_write(tag, PCI_COMMAND, temp);
    temp = _pci_conf_read(tag, PCI_COMMAND);
    printf("New PCI_COMMAND = 0x%04x\n", temp);
	temp = _pci_conf_readn(tag, 0x48, 1);
    printf("New 0x48 = 0x%04x\n", temp);

#endif
	testdisk_init = 1;
}

static int check_content(void) 
{
	unsigned long *buf_addr = (unsigned long *)BUF_BASE;
	int i, ret = 0;
	for (i = 0; (unsigned long)buf_addr < (BUF_BASE + sect_per_trans * 512); buf_addr++, i++) {
		if (*buf_addr != i) {
			printf("err@%08x: %08x %08x\n", buf_addr, *buf_addr, i);
			ret = 1;
		}
	}
	return ret;
}

static void prepare_content(void)
{
	unsigned long *buf_addr = (unsigned long *)BUF_BASE;
	int i;
	for (i = 0; (unsigned long)buf_addr < (BUF_BASE + sect_per_trans * 512); buf_addr++, i++) 
		*buf_addr = i;
}

static int ide_dma_end (void)
{
	unsigned char dma_stat = 0, dma_cmd = 0;

	/* get dma_command mode */
	dma_cmd = inb(DMA_COMMAND);
	/* stop DMA */
	outb(DMA_COMMAND, dma_cmd&~1);
	/* get DMA status */
	dma_stat = inb(DMA_STATUS);
	/* clear the INTR & ERROR bits */
	outb(DMA_STATUS, dma_stat|6);
	/* purge DMA mappings */
	/* verify good DMA status */
	return (dma_stat & 7) != 4 ? (0x10 | dma_stat) : 0;
}

static int prepare(int block)
{
	ata_nsector_t nsectors;
	unsigned char tasklets[10];
	nsectors.all = sect_per_trans;
#if 0
	tasklets[0] = 0;
	tasklets[1] = 0;
	tasklets[2] = nsectors.b.low;
	tasklets[3] = nsectors.b.high;
	tasklets[4] = (unsigned char) block;
	tasklets[5] = (unsigned char) (block>>8);
	tasklets[6] = (unsigned char) (block>>16);
	tasklets[7] = (unsigned char) (block>>24);
	tasklets[8] = (unsigned char) 0;
	tasklets[9] = (unsigned char) 0;
//	tasklets[8] = (unsigned char) (block>>32);
//	tasklets[9] = (unsigned char) (block>>40);
//
	outb(IDE_FEATURE_REG, tasklets[1]);
	outb(IDE_NSECTOR_REG, tasklets[3]);
	outb(IDE_SECTOR_REG, tasklets[7]);
	outb(IDE_LCYL_REG, tasklets[8]);
	outb(IDE_HCYL_REG, tasklets[9]);

	outb(IDE_FEATURE_REG, tasklets[0]);
	outb(IDE_NSECTOR_REG, tasklets[2]);
	outb(IDE_SECTOR_REG, tasklets[4]);
	outb(IDE_LCYL_REG, tasklets[5]);
	outb(IDE_HCYL_REG, tasklets[6]);
	outb(IDE_SELECT_REG, 0x00|LBA_MODE);
#else
    outb(IDE_FEATURE_REG, 0x00);
    outb(IDE_NSECTOR_REG, nsectors.b.low);
    outb(IDE_SECTOR_REG, block);
    outb(IDE_LCYL_REG, block>>=8);
    outb(IDE_HCYL_REG, block>>=8);
    outb(IDE_SELECT_REG, ((block>>8)&0x0f)|LBA_MODE);
#endif
	return 0;
}

static int ide_build_dmatable(void)
{
	unsigned long prdtable_base = PRDTABLE_BASE;
	*(unsigned long *)(prdtable_base) = (unsigned long)BUF_BASE_DMA;
	*(unsigned long *)(prdtable_base + 4) = 0x80000000 | (sect_per_trans * 512);
	return 0;
}

static int readide(int argc, char **argv)
{
	unsigned char dma_stat, dma_command;
	int total_sect = (argc > 1) ? strtoul(argv[1], 0, 0) :  TEST_SECT;
	int block, done, stat, activetimes;

	if (!testdisk_init)
		initialize();

	sect_per_trans = (argc >= 3) ? strtoul(argv[2], 0, 0) : TRANS_SECT;
	printf("Reading %d sectors, %d sectorts per trans...\n", total_sect, sect_per_trans);
	ide_build_dmatable();
	for (block = 0; block < total_sect; block += sect_per_trans){
		activetimes = 0;
		done = 0;
		prepare(block);
		/* PRD table */
		outl(DMA_PRDTABLE, PRDTABLE_BASE_DMA);
		/* specify r/w */
		outb(DMA_COMMAND, 1 << 3);
		/* read dma_status for INTR & ERROR flags */
		dma_stat = inb(DMA_STATUS);
		/* clear INTR & ERROR flags */
		outb(DMA_STATUS, dma_stat|6);
		outb(IDE_COMMAND_REG, WIN_READDMA);
		delay(400);
		dma_command = inb(DMA_COMMAND);
		outb(DMA_COMMAND, dma_command|1);
		delay(400);
		while(!done){
			delay(20);
			dma_stat = inb(DMA_STATUS);
			switch(dma_stat & 0x05) {
				case 1: /* active, still processing */
					activetimes++;
					continue;
				case 0:
					printf("Error occured, %d %d\n", block, activetimes);
					return 1;
				case 4:
					break;
				case 5:
					printf("wired: memory > transbuf?\n");
					return 1;
			}
			dma_stat = ide_dma_end();
			stat = inb(IDE_STATUS_REG);	/* get drive status */
			if ((stat & (DRIVE_READY|DRQ_STAT)) == DRIVE_READY && !dma_stat) {
				if (check_content()) {
					printf("dma_stat=0x%x\n", dma_stat);
					return 1;
				} else
					done = 1;
			}
		} 
		if (block && (block % 20 == 0))
			printf(".");
		if (block && (block % 1000 == 0))
			printf("\n");
	}
	printf("\n");
	return 0;
}
	
static int writeide(int argc, char **argv)
{
	unsigned char dma_stat, dma_command;
	int total_sect = (argc > 1) ? strtoul(argv[1], 0, 0) :  TEST_SECT;
	int block, done, stat;

	if (!testdisk_init)
		initialize();

	sect_per_trans = (argc >= 3) ? strtoul(argv[2], 0, 0) : TRANS_SECT;
	printf("Writing %d sectors, %d sectors per trans...\n", total_sect, sect_per_trans);
	ide_build_dmatable();
	prepare_content();
	for (block = 0; block < total_sect; block += sect_per_trans) {
		done = 0;
		prepare(block);
		/* PRD table */
		outl(DMA_PRDTABLE, PRDTABLE_BASE_DMA);
		/* specify r/w */
		outb(DMA_COMMAND, 0);
		/* read dma_status for INTR & ERROR flags */
		dma_stat = inb(DMA_STATUS);
		/* clear INTR & ERROR flags */
		outb(DMA_STATUS, dma_stat|6);
		outb(IDE_COMMAND_REG, WIN_WRITEDMA);
		delay(2000);/*strtoul(argv[3], 0, 0));   6000 recommanded at 8 sect per trans */
		dma_command = inb(DMA_COMMAND);
		outb(DMA_COMMAND, dma_command|1);
		delay(2000);/*strtoul(argv[3], 0, 0));*/
		while (!done) {
			delay(20);
			dma_stat = inb(DMA_STATUS);
			switch(dma_stat & 0x05) {
				case 1: /* active, still processing */
					continue;
				case 0:
					printf("Error occured\n");
					return 1;
				case 4:
					break;
				case 5:
					printf("wired: memory > transbuf?\n");
					return 1;
			}
			dma_stat = ide_dma_end();
			stat = inb(IDE_STATUS_REG);	/* get drive status */
			if (((stat & (DRIVE_READY|DRQ_STAT)) == DRIVE_READY) && !dma_stat) 
				done = 1;
			else {
				printf("write error@block: %d", block);
				return 1;
			}
		}
		if (block && (block % 20 == 0))
			printf(".");
		if (block && (block % 1000 == 0))
			printf("\n");
	}
	printf("\n");
	return 0;
}
	
static int comptest(int argc, char** argv)
{
  while(1) {
	writeide(argc, argv);
	readide(argc, argv);
  }
}

static const Cmd Cmds[] = 
{
	{"TestDiskCmds"},
	{"writeide", "sects", 0, "write ide using DMA", writeide, 0, 99, CMD_REPEAT},
	{"readide", "sects", 0, "read ide using DMA", readide, 0, 99, CMD_REPEAT},
	{"comptest", "sects", 0, "compound test", comptest, 0, 99, CMD_REPEAT},
	{0, 0}
};
static void init_cmd __P((void)) __attribute__ ((constructor));

static void
init_cmd()
{
	cmdlist_expand(Cmds, 1);
}
