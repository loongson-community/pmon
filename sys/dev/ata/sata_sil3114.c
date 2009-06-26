/*
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */
 /*********************************************************************************

 Copyright (C)
 File name:     sata_sil3114.c
 Author:  qianyl      Version:  ***      Date: ***
 Description:   
 Others:        
 Function List:
 
 Revision History:
 
 -----------------------------------------------------------------------------------------------------------
  Date               Author          Activity ID             Activity Headline
  2008-05-12    QianYuli        PMON00000001    porting it from u-boot and linux
  2008-06-26    QianYuli        PMON00000002    move some function to libata_***.c
************************************************************************************/
/*
Note to myself:
Port (or in sil3114 datasheet called channel) index number does not  equal to device
index number.
for example:sil3114 has 4 ports, the index number is 0,1,2,3, and if only one sata device
(for example hard disk) connects to port 2.Then the device's dev_no is 0,but the device's
port_no is 2!And if two sata devices connect to port 1 and port 2,then the first device's
dev_no is 0 and its port_no is 1,the second device's dev_no is 1 and its port_no is 2.
The rest may be deduced by analogy.
 */

#include "sata_sil3114.h"
#include <stdio.h>
#include <dev/pci/pcireg.h>
#include <dev/pci/pcivar.h>
#include <fcntl.h>
#include <file.h>
#include <sys/buf.h>
#include <ramfile.h>
#include <sys/unistd.h>
#undef _KERNEL
#include <errno.h>

/* Convert sectorsize to wordsize */
#define ATA_SECTOR_WORDS (ATA_SECT_SIZE/2)
#define vtophys(p)      _pci_dmamap((vm_offset_t)p, 1)
#define CFG_SATA_MAX_DEVICE 2

static int sil_match (struct device *, void *, void *);
static void sil_attach (struct device *, struct device *, void *);

#define myudelay delay
struct cfattach sil_ca = {
    sizeof(sil_sata_t),sil_match,sil_attach,
};

struct cfdriver sil_cd = {
        NULL, "sil", DV_DULL, 
};

int sil_sata_initialize(u32 reg,u32 flags);
int sil_open(dev_t dev,int flag, int fmt,struct proc *p);
int sil_close(dev_t dev,int flag, int fmt,struct proc *p);
int sil_read(dev_t dev,struct uio *uio,int flags);
int sil_write(dev_t dev,struct uio *uio,int flags);

static int sil_set_mode(struct ata_port *p_ap);
static void sil_bmdma_setup(struct ata_port *p_ap);
static void sil_bmdma_start(struct ata_port *p_ap);
static void sil_bmdma_stop(struct ata_port *p_ap);
unsigned int sil_devchk (struct ata_port *p_ap);
int sil_sata_phy_reset (struct ata_port *p_ap);
static void sil_irq_clear(struct ata_port *p_ap);
static u8 sil_bmdma_status(struct ata_port *p_ap);

extern void udelay(int usec);

extern sata_operation_t sata_op;
static u32 g_iobase[6] = { 0, 0, 0, 0, 0, 0};   /* PCI BAR registers for device */
ata_port_t sil_ata_ports[CONFIG_SYS_SATA_MAX_DEVICE];
ata_device_t sil_ata_dev[CONFIG_SYS_SATA_MAX_DEVICE];
ata_host_t sil_host;

p_ata_port_t p_sil_ata_ports[CONFIG_SYS_SATA_MAX_DEVICE] = {
        &sil_ata_ports[0],
        &sil_ata_ports[1],
        &sil_ata_ports[2],
        &sil_ata_ports[3],  
};

p_ata_device_t p_sil_ata_dev[CONFIG_SYS_SATA_MAX_DEVICE] = {
        &sil_ata_dev[0],
        &sil_ata_dev[1],
        &sil_ata_dev[2],
        &sil_ata_dev[3],
};

static int fault_timeout;
struct sil_sata_softc *p_sil_device;

ata_prd_t sil_prd_entry[CONFIG_SYS_SATA_MAX_DEVICE] =  {
    {0x0,0x81000000}, //size up to 16M
    {0x0,0x81000000}, //size up to 16M
    {0x0,0x81000000}, //size up to 16M
    {0x0,0x81000000}, //size up to 16M
};

extern const struct ata_port_operations_t ata_bmdma_port_ops;
static struct ata_port_operations sil_ops = {
    .inherits       = &ata_bmdma_port_ops,   
    .set_mode       = sil_set_mode,
    .bmdma_setup            = sil_bmdma_setup,
    .bmdma_start            = sil_bmdma_start,
    .bmdma_stop     = sil_bmdma_stop,
    .bmdma_status = sil_bmdma_status,
    .devchk           = sil_devchk,
    .hardreset       = sil_sata_phy_reset,
    .sff_irq_clear    = sil_irq_clear,
 };

/* per-port register offsets */
/* TODO: we can probably calculate rather than use a table */
static const struct {
    unsigned long tf;   /* ATA taskfile register block */
    unsigned long ctl;  /* ATA control/altstatus register block */
    unsigned long bmdma;    /* DMA register block */
    unsigned long bmdma2;   /* DMA register block #2 */
    unsigned long fifo_cfg; /* FIFO Valid Byte Count and Control */
    unsigned long scr;  /* SATA control register block */
    unsigned long sien; /* SATA Interrupt Enable register */
    unsigned long xfer_mode;/* data transfer mode register */
    unsigned long sfis_cfg; /* SATA FIS reception config register */
} sil_port[] = {
    /* port 0 ... */
    /*   tf    ctl  bmdma  bmdma2  fifo    scr   sien   mode   sfis */
    {  0x80,  0x8A,   0x0,  0x10,  0x40, 0x100, 0x148,  0xb4, 0x14c },
    {  0xC0,  0xCA,   0x8,  0x18,  0x44, 0x180, 0x1c8,  0xf4, 0x1cc },
    { 0x280, 0x28A, 0x200, 0x210, 0x240, 0x300, 0x348, 0x2b4, 0x34c },
    { 0x2C0, 0x2CA, 0x208, 0x218, 0x244, 0x380, 0x3c8, 0x2f4, 0x3cc },
    /* ... port 3 */
};

static void sil_irq_clear(struct ata_port *p_ap)
{
    void  *mmio_base = p_ap->p_host->iomap[5];
    void  *bmdma2 = mmio_base + sil_port[p_ap->port_no].bmdma2;

    if (!bmdma2)
        return;

    iowrite8(ioread8(bmdma2 + ATA_DMA_STATUS), bmdma2 + ATA_DMA_STATUS);
}

static void sil_bmdma_setup(struct ata_port *p_ap)
{
    void  *bmdma = p_ap->ioaddr.bmdma_addr;
    u32  tmp,addr = bmdma + 0x10;//bmdma2

    /* load PRD table addr. */
    iowrite32(p_ap->prd_dma, bmdma + ATA_DMA_TABLE_OFS);

    //Clear bit 17 and bit 18 in the PCI Bus Master2
    if (p_ap&&p_ap->p_ops&&p_ap->p_ops->sff_irq_clear) {
        p_ap->p_ops->sff_irq_clear(p_ap);
    }

    /* issue r/w command */
    ata_tf_to_host(p_ap);
}

static void sil_bmdma_start(struct ata_port *p_ap)
{
    unsigned int rw = (p_ap->p_tf->flags & ATA_TFLAG_WRITE);
    void  *mmio_base = p_ap->p_host->iomap[5];
    void  *bmdma2 = mmio_base + sil_port[p_ap->port_no].bmdma2;
    u8 dmactl = ATA_DMA_START;

    /* set transfer direction, start host DMA transaction
       Note: For Large Block Transfer to work, the DMA must be started
       using the bmdma2 register. */
    if (!rw)
        dmactl |= ATA_DMA_WR;
    iowrite8(dmactl, bmdma2);
}

static void sil_bmdma_stop(struct ata_port *p_ap)
{
    void  *mmio_base = p_ap->p_host->iomap[5];
    void  *bmdma2 = mmio_base + sil_port[p_ap->port_no].bmdma2;

    //clear 18bit of PCI Bus Master2
    if (p_ap&&p_ap->p_ops&&p_ap->p_ops->sff_irq_clear) {
        p_ap->p_ops->sff_irq_clear(p_ap);
    }

    /* clear start/stop bit - can safely always write 0 */
    iowrite8(0, bmdma2); 
}

static u8 sil_bmdma_status(struct ata_port *p_ap)
{
    void  *mmio_base = p_ap->p_host->iomap[5];
    void  *bmdma2 = mmio_base + sil_port[p_ap->port_no].bmdma2;

    return ioread8(bmdma2 + ATA_DMA_STATUS);
}

/* Driver implementation */
static u8 sil_get_device_cache_line (struct pci_attach_args *pa)
{
    u8 cache_line = 0;
    cache_line = _pci_conf_readn(pa->pa_tag,PCI_CACHE_LINE_SIZE_INDEX,1);       
    return cache_line;
}

static int sil_match(
                struct device *parent,
                void   *match,
                void * aux
                )
{
    struct pci_attach_args *pa = aux;
    if(PCI_VENDOR(pa->pa_id) == PCI_VENDOR_SATA_SIL3114&&
       PCI_PRODUCT(pa->pa_id) == PCI_DEVICE_SATA_SIL3114){
        return 1;
    }
    return 0;
}

static void sil_attach(struct device * parent,struct device * self,void *aux)
{
    int valid;
    struct pci_attach_args *pa = (struct pci_attach_args *)aux;
    pci_chipset_tag_t pc = pa->pa_pc;
    bus_space_tag_t memt = pa->pa_memt;
    bus_addr_t membase; 
    bus_size_t memsize; 
    u8 cls = 0;
    u16 cmd = 0;
    u32 sconf = 0;
    int cachable;
    int i = 0;    

    p_sil_device =(struct sil_sata_softc *)self;
    
    //we only use MMIO from BAR5
    if (pci_mem_find(pc, pa->pa_tag, PCI_BASE_ADDRESS_5, &membase, &memsize,&cachable)){
        printf ("Error no base addr for SATA controller(Sil3114).\n");
        return;
    }

    if (bus_space_map(memt, membase, memsize, 0, &g_iobase[5])) {
        printf(": Can't map mem space\n");
        return;
    }

    /* from sata_sil in Linux kernel */
    cls = sil_get_device_cache_line (pa);
    if (cls) {
        cls >>= 3;
        cls++;      /* cls = (line_size/8)+1 */
        writel (cls << 8 | cls, g_iobase[5] + VND_FIFOCFG_CH0);
        writel (cls << 8 | cls, g_iobase[5] + VND_FIFOCFG_CH1);
        writel (cls << 8 | cls, g_iobase[5] + VND_FIFOCFG_CH2);
        writel (cls << 8 | cls, g_iobase[5] + VND_FIFOCFG_CH3);
    } else {
        printf ("Cache line not set. Driver may not function\n");
    }   

    /* Enable operation */
    cmd = _pci_conf_readn(pa->pa_tag,PCI_COMMAND,2);
    cmd |= PCI_COMMAND_MASTER | PCI_COMMAND_IO | PCI_COMMAND_MEMORY;
     _pci_conf_writen(pa->pa_tag, PCI_COMMAND, cmd,2);

    //do the necessary init 
     sil_host.p_ops = &sil_ops;
     sil_host.pp_dev = p_sil_ata_dev;
     sil_host.pp_ports = p_sil_ata_ports;
     sil_host.iomap    = (void *)&g_iobase;
     sil_host.n_ports = CONFIG_SYS_SATA_MAX_DEVICE;
     
     for (i = 0; i <CONFIG_SYS_SATA_MAX_DEVICE; i++ ) {
         p_ata_port_t  p_ap = sil_host.pp_ports[i];
         p_ata_ioports_t  p_ioaddr = &p_ap->ioaddr;
         unsigned long prdtable_base = (unsigned long)&sil_prd_entry[i];

         p_ap->prd_dma =(unsigned long)vtophys(prdtable_base);
         sil_ata_dev[i].p_host = &sil_host;
         sil_ata_ports[i].p_host = &sil_host;
         sil_ata_dev[i].port_no = -1; //no port attached from init
         sil_ata_dev[i].dev_no = i;
         sil_ata_ports[i].port_no = i;
         sil_ata_ports[i].dev_no = -1;//no device attached from init
         sil_ata_ports[i].p_ops = &sil_ops;
         p_ioaddr->cmd_addr = g_iobase[5]  + sil_port[i].tf;
         p_ioaddr->altstatus_addr = 
         p_ioaddr->ctl_addr = g_iobase[5] + sil_port[i].ctl;
         p_ioaddr->bmdma_addr = g_iobase[5] + sil_port[i].bmdma;
         p_ioaddr->scr_addr = g_iobase[5] + sil_port[i].scr;

         ata_sff_std_ports(p_ioaddr);
     }

    if(ata_host_activate(&sil_host))
    {
        printf("sil_attach: ata_host_activate() failed.\n");
    }

    for (i = 0; i <CONFIG_SYS_SATA_MAX_DEVICE; i++ ) {
        if(sil_ata_dev[i].port_no == -1) {
            continue;
        }
        //printf("before config_found:port_no:%d device_no:%d\n",sil_ata_dev[i].port_no,sil_ata_dev[i].dev_no);
        config_found(p_sil_device, &sil_ata_dev[i], NULL);
    }

    sata_op.open = sil_open;
    sata_op.close = sil_close;
    sata_op.read = sil_read;
    sata_op.write = sil_write;    
}

/* Check if sata device is connected to port */
unsigned int sil_devchk (struct ata_port *p_ap)
{
    unsigned int port_no,port_addr,val;
    p_ata_host_t  p_host = p_ap->p_host;

    port_addr =(unsigned int) p_host->iomap[5];
    port_no = p_ap->port_no;

    switch (port_no) {
    case 0:
        port_addr += VND_SSTATUS_CH0;
        break;
    case 1:
        port_addr += VND_SSTATUS_CH1;
        break;
    case 2:
        port_addr += VND_SSTATUS_CH2;
        break;
    case 3:
        port_addr += VND_SSTATUS_CH3;
        break;
    default:
        return 1;
    }
    val = readl (port_addr);
    if ((val & SATA_DET_PRES) == SATA_DET_PRES) {
        return 0;
    } else {
        return 1;
    }
}

int sil_sata_phy_reset (struct ata_port *p_ap)
{
    u32 port =(u32) p_ap->p_host->iomap[5];
    int port_no = p_ap->port_no;
    u32 val;
    switch (port_no) {
    case 0:
        port += VND_SCONTROL_CH0;
        break;
    case 1:
        port += VND_SCONTROL_CH1;
        break;
    case 2:
        port += VND_SCONTROL_CH2; 
        break;
    case 3:
        port += VND_SCONTROL_CH3;
        break;
    default:
        return 1;
    }
    val = readl (port);
    writel (val | SATA_SC_DET_RST, port);
    mdelay(150);
    writel (val & ~SATA_SC_DET_RST, port);
    return 0;
}

static int sil_set_mode(struct ata_port *p_ap)
{
    u32 tmp,mmio_base  =(u32) p_ap->p_host->iomap[5];
    u32 addr = mmio_base + sil_port[p_ap->port_no].xfer_mode;

    ata_do_set_mode(p_ap);  

    p_ap->trans_mode = ATA_TRANS_UDMA;
    tmp = readl(addr);
    tmp &= ~((1<<5) | (1<<4) | (1<<1) | (1<<0));  
    tmp |= 0x3;      //UDMA
    writel(tmp, addr);
    readl(addr);    /* flush */
    return 0;
}    int i = 0;
    unsigned int tmp = 0 ; 

/*
 *
 * Returns sectors read(DMA)
*/
static u32 sil_do_one_read(p_ata_port_t p_ap, ulong block, u16 blkcnt, u16 * buff)
{

    u16 sr = blkcnt;
    u32 tmp;
    u32 tf_flags,tag;
    p_ata_port_operations_t p_ops = p_ap->p_ops;
    int rv = 0;
    ata_taskfile_t tf;

    memset(&tf,0,sizeof(ata_taskfile_t));

    tf.flags &= ~ATA_TFLAG_WRITE;//Read
    p_ap->p_tf =&tf;
    tf_flags = 0;
    tag = 0;    

    /*printf("sil_do_one_read: block:%x blkcnt:%x buff:%x  blknr:%x\n",
           block, blkcnt, (unsigned int)buff, blknr);*/
    

    if (!(p_ops->sff_check_status(p_ap,0)&ATA_DRDY)) {
        printf ("Port%d's Device%d not ready\n", p_ap->port_no,p_ap->dev_no);
        return 0;
    }
    
    //build cmd for DMA transfer
    if(rv = ata_build_rw_tf(p_ap,block,blkcnt,tf_flags,tag)) {
        printf("--sil_do_one_read--   ata_build_rw_tf failed.  rv = %d\n ",rv);
        return 0;
    }
    
   {
       unsigned long buff_base = (unsigned long)buff;
       sil_prd_entry[p_ap->port_no].addr =(unsigned long)vtophys(buff_base);
   }
   
   p_ops->bmdma_setup(p_ap);

   //printf("--sil_do_one_read-- p_ap->prd_dma:%x addr_base:%x\n",p_ap->prd_dma, *(u32 *)0xa5000000);

   //need flush all cache,important here!!
   flushcache();

    p_ops->bmdma_start(p_ap);
  
     /*wait for dma interrupt here*/
   
    while(1) {
         tmp = p_ops->bmdma_status(p_ap);
         tmp &=0x7;
         switch (tmp) {
            case DMA_ERROR:
                goto  read_err;
            case PRD_SIZE_SAMLLER:
            case NORMAL_COMPLETE:
            case PRD_SIZE_LARGER:
                goto stop_dma;
            case DMA_IN_PROGRESS:
            default:   
                udelay(100);
            break;
        }
    }

    //stop DMA
stop_dma:
    p_ops->bmdma_stop(p_ap);
    p_ap->p_tf = NULL;
    return sr;

read_err:
    printf("sil_do_one_read DMA read error!\n");   
    p_ap->p_tf = NULL;
    return sr;
 }

ulong sil_sata_read (int device, ulong block, lbaint_t blkcnt, void *buff)
{
    ulong n = 0;
    u16 sread;
    u16 *buffer = (u16 *) buff;
    u32 status = 0;
    u32 blknr = block;
    int port_no =  sil_ata_dev[device].port_no;

    while (blkcnt > 0) {
        if (sil_ata_ports[port_no].dflags&ATA_DFLAG_LBA48) {
            if (blkcnt > 65535) {
                sread = 65535;
            } else {
                sread = blkcnt;
            }
       }
        else 
        {       
             if (blkcnt > 255) {
                 sread = 255;
             } else {
                 sread = blkcnt;
             }
        }

        status = sil_do_one_read(&sil_ata_ports[port_no], blknr, sread, buffer);       

        if (status != sread) {
            printf ("Read failed\n");
            return n;
        }

        blkcnt -= sread;
        blknr += sread;
        n += sread;
        buffer += sread * ATA_SECTOR_WORDS;
    }
    return n;
}

//not finished yet,not need till now
ulong sil_sata_write (int device, ulong block, lbaint_t blkcnt, const void *buff)
{
    return 0;
}


#define sil_sata_lookup(dev) (struct sil_sata_softc *)device_lookup(&sil_cd, minor(dev))

void sil_sata_strategy(struct buf *bp)
{
        struct sil_sata_softc *priv;
        unsigned int blkno, blkcnt;
        int ret ;

        priv=sil_sata_lookup(bp->b_dev);

        blkno = bp->b_blkno;

        blkno = blkno /(priv->bs/DEV_BSIZE);
        blkcnt = howmany(bp->b_bcount, priv->bs);

        /* Valid request?  */
        if (bp->b_blkno < 0 ||
            (bp->b_bcount % priv->bs) != 0 ||
            (bp->b_bcount / priv->bs) >= (1 << NBBY)) {
                bp->b_error = EINVAL;
                printf("Invalid request \n");
                goto bad;
        }

        /* If it's a null transfer, return immediately. */
        if (bp->b_bcount == 0)
                goto done;

        if(bp->b_flags & B_READ){
            fault_timeout = 0;
            ret=sil_sata_read(minor(bp->b_dev),blkno,blkcnt,bp->b_data); 
            if(ret != blkcnt||fault_timeout)
                bp->b_flags |= B_ERROR;
            dotik(30000, 0);
        }
        else
        {
            fault_timeout = 0;
            ret=sil_sata_write(minor(bp->b_dev),blkno,blkcnt,bp->b_data);
            if(ret != blkcnt||fault_timeout)
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

int sil_open(
    dev_t dev,
    int flag, int fmt,
    struct proc *p)
{
    struct sil_sata_softc *priv;
    priv=sil_sata_lookup(dev);
    if(!priv)return -1;
    priv->bs=ATA_SECT_SIZE;
    priv->count=-1;
    return 0;
}

int sil_close( dev_t dev,
        int flag, int fmt,
        struct proc *p)
{
    return 0;
}

int sil_read(
    dev_t dev,
    struct uio *uio,
    int flags)
{
    return physio(sil_sata_strategy, NULL, dev, B_READ, minphys, uio);
}

int sil_write(
    dev_t dev,
    struct uio *uio,
    int flags)
{
    return (physio(sil_sata_strategy, NULL, dev, B_WRITE, minphys, uio));
}

#define DEBUGSATA
#ifdef DEBUGSATA

static int do_sata (int argc, char *argv[])
{
    int rc = 0;
    
    switch (argc) {
    case 0:
    case 1:
    case 2:
    case 3:
        return 1;

    default: /* at least 4 args */
        if (strcmp(argv[1], "read") == 0) {
            ulong addr = strtoul(argv[2], NULL, 16);
            ulong cnt = strtoul(argv[4], NULL, 16);
            ulong n;
            lbaint_t blk = strtoul(argv[3], NULL, 16);

            printf("\nSATA read:  block # %ld, count %ld ... \n",
                blk, cnt);

            n = sil_sata_read(0, blk, cnt, (u32 *)addr);

            printf("%ld blocks read: %s\n",
                n, (n==cnt) ? "OK" : "ERROR");
            return (n == cnt) ? 0 : 1;
        } else if (strcmp(argv[1], "write") == 0) {
            ulong addr = strtoul(argv[2], NULL, 16);
            ulong cnt = strtoul(argv[4], NULL, 16);
            ulong n;

            lbaint_t blk = strtoul(argv[3], NULL, 16);

            printf("\nSATA write:  block # %ld, count %ld ... ",
                 blk, cnt);

            n = sil_sata_write(0, blk, cnt, (u32 *)addr);

            printf("%ld blocks written: %s\n",
                n, (n == cnt) ? "OK" : "ERROR");
            return (n == cnt) ? 0 : 1;
        } 
        
    }
    return rc;
}

static const Cmd Cmds[] = {
    { "Sata commands"},
    { "sata", " ", NULL, "general sata commands", do_sata, 1, 5, 0},
    { 0, 0}
};  

static void init_cmd(void) __attribute__((constructor));

static void init_cmd(void)
{
    cmdlist_expand(Cmds, 1);
}

#endif

