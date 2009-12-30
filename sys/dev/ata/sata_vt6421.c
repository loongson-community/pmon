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
 File name:     sata_vt6421.c
 Author:  qianyl      Version:  ***      Date: ***
 Description:   
 Others:        
 Function List:
 
 Revision History:
 
 -----------------------------------------------------------------------------------------------------------
  Date          Author          Activity ID     Activity Headline
  2008-05-12    QianYuli        PMON00000001    porting it from u-boot and linux
  2008-06-26    QianYuli        PMON00000002    move some function to libata_***.c
  2009-12-24    QianYuli        PMON00001224    porting it from sata_sil3114.c
************************************************************************************/
/*
Note to myself:
Port  index number does not  equal to device
index number.
for example:vt6421 has 4 ports, the index number is 0,1,2,3, and if only one sata device
(for example hard disk) connects to port 2.Then the device's dev_no is 0,but the device's
port_no is 2!And if two sata devices connect to port 1 and port 2,then the first device's
dev_no is 0 and its port_no is 1,the second device's dev_no is 1 and its port_no is 2.
The rest may be deduced by analogy.
 */

#include "sata_vt6421.h"
#include <stdio.h>
#include <dev/pci/pcireg.h>
#include <dev/pci/pcivar.h>
#include <fcntl.h>
#include <file.h>
#include <sys/buf.h>
#include <sys/malloc.h>
#include <ramfile.h>
#include <sys/unistd.h>
#undef _KERNEL
#include <errno.h>

/* Convert sectorsize to wordsize */
#define ATA_SECTOR_WORDS (ATA_SECT_SIZE/2)
#define vtophys(p)      _pci_dmamap((vm_offset_t)p, 1)
#define CFG_SATA_MAX_DEVICE 2

static int vt_match (struct device *, void *, void *);
static void vt_attach (struct device *, struct device *, void *);

#define myudelay delay
struct cfattach vt_ca = {
    sizeof(vt_sata_t),vt_match,vt_attach,
};

struct cfdriver vt_cd = {
        NULL, "vt", DV_DULL, 
};

int vt_open(dev_t dev,int flag, int fmt,struct proc *p);
int vt_close(dev_t dev,int flag, int fmt,struct proc *p);
int vt_read(dev_t dev,struct uio *uio,int flags);
int vt_write(dev_t dev,struct uio *uio,int flags);

static void vt_bmdma_setup(struct ata_port *p_ap);
static void vt_bmdma_start(struct ata_port *p_ap);
static void vt_bmdma_stop(struct ata_port *p_ap);
unsigned int vt_devchk (struct ata_port *p_ap);
int vt_sata_phy_reset (struct ata_port *p_ap);
static void vt_irq_clear(struct ata_port *p_ap);
static u8 vt_bmdma_status(struct ata_port *p_ap);

extern void udelay(int usec);

extern sata_operation_t sata_op;
static unsigned long g_iobase[6] = { 0, 0, 0, 0, 0, 0};   /* PCI BAR registers for device */
ata_port_t vt_ata_ports[CONFIG_SYS_SATA_MAXPORTS];
ata_device_t vt_ata_dev[CONFIG_SYS_SATA_MAXPORTS];
ata_host_t vt_host;

p_ata_port_t p_vt_ata_ports[CONFIG_SYS_SATA_MAXPORTS] = {
        &vt_ata_ports[0],
        &vt_ata_ports[1],
};

p_ata_device_t p_vt_ata_dev[CONFIG_SYS_SATA_MAXPORTS] = {
        &vt_ata_dev[0],
        &vt_ata_dev[1],
};

static int fault_timeout;
struct vt_sata_softc *p_vt_device;

extern const struct ata_port_operations_t ata_bmdma_port_ops;
static struct ata_port_operations vt_ops = {
    .inherits       = &ata_bmdma_port_ops,   
    .bmdma_setup            = vt_bmdma_setup,
    .bmdma_start            = vt_bmdma_start,
    .bmdma_stop     = vt_bmdma_stop,
    .bmdma_status = vt_bmdma_status,
    .devchk           = vt_devchk,
    .hardreset       = vt_sata_phy_reset,
    .sff_irq_clear    = vt_irq_clear,
 };

//seems ok
static void vt_irq_clear(struct ata_port *p_ap)
{
    void  *mmio_base4 = p_ap->p_host->iomap[4];

    iowrite8(ioread8(mmio_base4 + ATA_DMA_STATUS + p_ap->port_no*8), mmio_base4 + ATA_DMA_STATUS + p_ap->port_no*8);
}


//seems ok
static void vt_bmdma_setup(struct ata_port *p_ap)
{
    void  *mmio_base4 = p_ap->p_host->iomap[4];
    //u32  tmp,addr = bmdma + 0x10;//bmdma2

    /* load PRD table addr. */
    iowrite32(p_ap->prd_dma, mmio_base4 + ATA_DMA_TABLE_OFS + p_ap->port_no*8);

    /* clear bit 1 and bit 2 in the PCI Bus Master */
    if (p_ap&&p_ap->p_ops&&p_ap->p_ops->sff_irq_clear) {
        p_ap->p_ops->sff_irq_clear(p_ap);
    }
 
    /* issue r/w command */
    ata_tf_to_host(p_ap);
}


//seems ok
static void vt_bmdma_start(struct ata_port *p_ap)
{
    unsigned int rw = (p_ap->p_tf->flags & ATA_TFLAG_WRITE);
    void  *mmio_base4 = p_ap->p_host->iomap[4];
    u8 dmactl = ATA_DMA_START;

    /* set transfer direction, start host DMA transaction
       Note: For Large Block Transfer to work, the DMA must be started
       using the bmdma2 register. */
    if (!rw)
        dmactl |= ATA_DMA_WR;
    iowrite8(dmactl, mmio_base4 + ATA_DMA_CMD + p_ap->port_no*8);
}


//seems ok
static void vt_bmdma_stop(struct ata_port *p_ap)
{
    void  *mmio_base4 = p_ap->p_host->iomap[4];
    u8 val;

    /*clear bit 2 of PCI Bus Master*/
    if (p_ap&&p_ap->p_ops&&p_ap->p_ops->sff_irq_clear) {
        p_ap->p_ops->sff_irq_clear(p_ap);
    }
    
    /* clear start/stop bit  */
    val = ioread8(mmio_base4 + ATA_DMA_CMD + p_ap->port_no*8);
    val &= ~ATA_DMA_START;
    iowrite8(val, mmio_base4 + ATA_DMA_CMD + p_ap->port_no*8); 
}


//seems ok
static u8 vt_bmdma_status(struct ata_port *p_ap)
{
    void  *mmio_base4 = p_ap->p_host->iomap[4];

    return ioread8(mmio_base4 + ATA_DMA_STATUS + p_ap->port_no*8);
}

//seems ok
static int vt_match(
                struct device *parent,
                void   *match,
                void * aux
                )
{
    struct pci_attach_args *pa = aux;
    if(PCI_VENDOR(pa->pa_id) == PCI_VENDOR_SATA_VT6421&&
       PCI_PRODUCT(pa->pa_id) == PCI_DEVICE_SATA_VT6421){
        return 1;
    }
    return 0;
}


//seems ok
static void vt_attach(struct device * parent,struct device * self,void *aux)
{
    struct pci_attach_args *pa = (struct pci_attach_args *)aux;
    pci_chipset_tag_t pc = pa->pa_pc;
    bus_space_tag_t memt = pa->pa_memt;
	bus_space_tag_t iot = pa->pa_iot;
    bus_addr_t membase,iobase; 
    bus_size_t memsize,iosize; 
    u16 cmd = 0;
    int cachable;
    int i = 0; 
    
    p_vt_device =(struct vt_sata_softc *)self;
    
    for (i = 0; i < 6; i++){    
        if (pci_io_find(pc, pa->pa_tag, 0x10+i*4, &iobase, &iosize)){
            printf ("Error no base addr for SATA controller(sbs'600 sata controller). i = %d\n",i);
            return;
        }

        if (bus_space_map(iot, iobase, iosize, 0, &g_iobase[i])) {
            printf(": Can't map io space,i = %d\n",i);
            return;
        }
    }    

    
    /* Enable operation */
    cmd = _pci_conf_readn(pa->pa_tag,PCI_COMMAND,2);
    cmd |= PCI_COMMAND_MASTER | PCI_COMMAND_IO | PCI_COMMAND_MEMORY;
     _pci_conf_writen(pa->pa_tag, PCI_COMMAND, cmd,2);

    //do the necessary init 
    vt_host.p_ops = &vt_ops;
    vt_host.pp_dev = p_vt_ata_dev;
    vt_host.pp_ports = p_vt_ata_ports;
    vt_host.iomap    = (void *)&g_iobase;
    vt_host.n_ports = CONFIG_SYS_SATA_MAXPORTS;
     
    for (i = 0; i <CONFIG_SYS_SATA_MAXPORTS; i++ ) {
         p_ata_port_t  p_ap = vt_host.pp_ports[i];
         p_ata_ioports_t  p_ioaddr = &p_ap->ioaddr;
       
         vt_ata_dev[i].p_host = &vt_host;
         vt_ata_ports[i].p_host = &vt_host;
         vt_ata_dev[i].port_no = -1; //no port attached from init
         vt_ata_dev[i].dev_no = i;
         vt_ata_ports[i].port_no = i;
         vt_ata_ports[i].dev_no = -1;//no device attached from init
         vt_ata_ports[i].p_ops = &vt_ops;
         p_ioaddr->cmd_addr = g_iobase[i];
         p_ioaddr->altstatus_addr = 
         p_ioaddr->ctl_addr = g_iobase[i] + 0xa;
         printf("vt_attach p_ioaddr->ctl_addr:%x\n",(unsigned long)p_ioaddr->ctl_addr);

         ata_sff_std_ports(p_ioaddr);
     }

    if(ata_host_activate(&vt_host))
    {
        printf("vt_attach: ata_host_activate() failed.\n");
    }

    for (i = 0; i <CONFIG_SYS_SATA_MAX_DEVICE; i++ ) {
        if(vt_ata_dev[i].port_no == -1) {
            continue;
        }
        //printf("before config_found:port_no:%d device_no:%d\n",sbs_ata_dev[i].port_no,sbs_ata_dev[i].dev_no);
        config_found(p_vt_device, &vt_ata_dev[i], NULL);
    }

    sata_op.open = vt_open;
    sata_op.close = vt_close;
    sata_op.read = vt_read;
    sata_op.write = vt_write;    
}

//seems ok
/* Check if sata device is connected to port */
unsigned int vt_devchk (struct ata_port *p_ap)
{
    unsigned int port_no,val;
    unsigned long port_addr;
    p_ata_host_t  p_host = p_ap->p_host;

    port_addr =(unsigned long) p_host->iomap[5];
    port_no = p_ap->port_no;

    printf("vt_devchk port_addr:%x\n",(unsigned long)port_addr);

    //find SSTATUS reg
    port_addr +=(port_no*0x40); 
    val = readl (port_addr);

    printf("vt_devchk val:%x\n",val);
    
    if ((val & SATA_DET_PRES) == SATA_DET_PRES) {
        return 0;
    } else {
        return 1;
    }
}

//seems ok
int vt_sata_phy_reset (struct ata_port *p_ap)
{
    unsigned long port_addr =(unsigned long) p_ap->p_host->iomap[5];
    int port_no = p_ap->port_no;
    u32 val;

    printf("vt_sata_phy_reset port_addr:%x\n",(unsigned long)port_addr);

    //find SCONTROL reg
    port_addr +=(port_no*0x40 + 8); 
    val = readl (port_addr);
    printf("vt_sata_phy_reset val:%x\n",val);

    writel (val | SATA_SC_DET_RST, port_addr);
    mdelay(150);
    writel (val & ~SATA_SC_DET_RST, port_addr);
    return 0;
}


//seems ok
void setup_prd_table(struct ata_port *p_ap,void *buff,u32 size)
{
    u32 prd_entrys = size / VT_PRD_BUFF_SIZE + 1;
    ata_prd_t *prd_entry = NULL;
    int i;

	prd_entry =(ata_prd_t *)malloc(prd_entrys * sizeof(ata_prd_t), M_DEVBUF, M_NOWAIT);

    for(i = 0; i < prd_entrys - 1; i++){
        prd_entry[i].addr= (u32)buff + VT_PRD_BUFF_SIZE * i;
    }
    
    prd_entry[i].addr= (u32)buff + VT_PRD_BUFF_SIZE * i;
    prd_entry[i].flag_len = (size - VT_PRD_BUFF_SIZE *(prd_entrys - 1))|0x10000000;//EOT set

    p_ap->prd_dma =(unsigned long)vtophys(prd_entry);   
    
    return;
}

//seems ok
void del_prd_table(struct ata_port *p_ap)
{
    u8 *prd_buff = (u8 *)PHYS_TO_UNCACHED(p_ap->prd_dma);
    free(prd_buff,M_DEVBUF);
    return;
}
 
//seems ok
/*
 *
 * Returns sectors read(DMA)
 */
static u32 vt_do_one_read(p_ata_port_t p_ap, ulong block, u16 blkcnt, u16 * buff)
{

    u16 sr = blkcnt;
    u32 tmp;
    u32 tf_flags,tag;
    p_ata_port_operations_t p_ops = p_ap->p_ops;
    int rv = 0;
    ata_taskfile_t tf;


    memset(&tf,0,sizeof(ata_taskfile_t));

    //printf("vt_do_one_read p_ap:%x \n",(unsigned long)p_ap);


    tf.flags &= ~ATA_TFLAG_WRITE;//Read
    p_ap->p_tf =&tf;
    tf_flags = 0;
    tag = 0;    

    /*printf("sbs_do_one_read: block:%x blkcnt:%x buff:%x  blknr:%x\n",
           block, blkcnt, (unsigned int)buff, blknr);*/
    

    if (!(p_ops->sff_check_status(p_ap,0)&ATA_DRDY)) {
        printf ("Port%d's Device%d not ready\n", p_ap->port_no,p_ap->dev_no);
        return 0;
    }
    
    //build cmd for DMA transfer
    if(rv = ata_build_rw_tf(p_ap,block,blkcnt,tf_flags,tag)) {
        printf("--vt_do_one_read--   ata_build_rw_tf failed.  rv = %d\n ",rv);
        return 0;
    }

    setup_prd_table(p_ap,buff,blkcnt* ATA_SECT_SIZE);


    p_ops->bmdma_setup(p_ap);

    //printf("--sbs_do_one_read-- p_ap->prd_dma:%x addr_base:%x\n",p_ap->prd_dma, *(u32 *)0xa5000000);

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
read_err:
    printf("vt_do_one_read DMA read error!\n");   
    
    //stop DMA
stop_dma:
    p_ops->bmdma_stop(p_ap);
    del_prd_table(p_ap);
    p_ap->p_tf = NULL;
    return sr;


 }


//seems ok
ulong vt_sata_read (int device, ulong block, lbaint_t blkcnt, void *buff)
{
    ulong n = 0;
    u16 sread;
    u16 *buffer = (u16 *) buff;
    u32 status = 0;
    u32 blknr = block;
    int port_no =  vt_ata_dev[device].port_no;

    //printf("vt_sata_read port.no:%x \n",port_no);

    while (blkcnt > 0) {
        if (vt_ata_ports[port_no].dflags&ATA_DFLAG_LBA48) {
            if (blkcnt > 65535) {
                sread = 65535;
            } else {
                sread = blkcnt;
            }
        }else {               
            if (blkcnt > 255) {
                 sread = 255;
             } else {
                 sread = blkcnt;
             }
        }

        status = vt_do_one_read(&vt_ata_ports[port_no], blknr, sread, buffer);       

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

//not finished yet,for not need till now
ulong vt_sata_write (int device, ulong block, lbaint_t blkcnt, const void *buff)
{
    return 0;
}


#define vt_sata_lookup(dev) (struct vt_sata_softc *)device_lookup(&vt_cd, minor(dev))

//seems ok
void vt_sata_strategy(struct buf *bp)
{
        struct vt_sata_softc *priv;
        unsigned int blkno, blkcnt;
        int ret ;

        priv=vt_sata_lookup(bp->b_dev);

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
            ret=vt_sata_read(minor(bp->b_dev),blkno,blkcnt,bp->b_data); 
            if(ret != blkcnt||fault_timeout)
                bp->b_flags |= B_ERROR;
            dotik(30000, 0);
        }
        else
        {
            fault_timeout = 0;
            ret=vt_sata_write(minor(bp->b_dev),blkno,blkcnt,bp->b_data);
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


//seems ok
int vt_open(
    dev_t dev,
    int flag, int fmt,
    struct proc *p)
{
    struct vt_sata_softc *priv;
    priv=vt_sata_lookup(dev);
    if(!priv)return -1;
    priv->bs=ATA_SECT_SIZE;
    priv->count=-1;
    return 0;
}

//seems ok
int vt_close( dev_t dev,
        int flag, int fmt,
        struct proc *p)
{
    return 0;
}

//seems ok
int vt_read(
    dev_t dev,
    struct uio *uio,
    int flags)
{
    return physio(vt_sata_strategy, NULL, dev, B_READ, minphys, uio);
}

int vt_write(
    dev_t dev,
    struct uio *uio,
    int flags)
{
    return (physio(vt_sata_strategy, NULL, dev, B_WRITE, minphys, uio));
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

            n = vt_sata_read(0, blk, cnt, (u32 *)addr);

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

            n = vt_sata_write(0, blk, cnt, (u32 *)addr);

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
