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
 /************************************************************************

 Copyright (C)
 File name:     sata_sil3114.c
 Author:  qianyl      Version:  ***      Date: ***
 Description:   
 Others:        
 Function List:
 
 Revision History:
 
 --------------------------------------------------------------------------------------------
  Date              Author           Activity ID            Activity Headline
  2008-05-12    QianYuli        PMON00000001    porting it from u-boot
*************************************************************************/
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

struct sil_sata_softc {
    /* General disk infos */
    struct device sc_dev;
    int bs,count;
};

#define vtophys(p)      _pci_dmamap((vm_offset_t)p, 1)

#define CFG_SATA_MAX_DEVICE 2
#define debug(fmt,args...)  printf(fmt ,##args)
static int fault_timeout;


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

extern void udelay(int usec);
extern sata_operation_t sata_op;
extern sata_device_t sata_dev[];

static u32 g_iobase[6] = { 0, 0, 0, 0, 0, 0};   /* PCI BAR registers for device */
sata_port_t sata_ports[CONFIG_SYS_SATA_MAX_DEVICE];
int curr_dev = -1;
block_dev_desc_t *p_stor_desc;
struct sil_sata_softc *p_sil_device;

sata_prd_t sil_prd_entry[CONFIG_SYS_SATA_MAX_DEVICE] =  {
    {0x0,0x81000000}, //size up to 16M
    {0x0,0x81000000}, //size up to 16M
    {0x0,0x81000000}, //size up to 16M
    {0x0,0x81000000}, //size up to 16M
};
sata_prd_t sata_prd_entry  __attribute__ ((aligned (32)));

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
    
    sil_sata_info_t *pinfo = aux;
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

    valid = sil_sata_initialize(pinfo->sata_reg_base,pinfo->flags);
    if(valid)
        self->dv_class = DV_DULL;

    sata_op.open = sil_open;
    sata_op.close = sil_close;
    sata_op.read = sil_read;
    sata_op.write = sil_write;    
}
static inline void mdelay(u32 msec)
{
    u32 i;
    for (i = 0; i < msec; i++)
        udelay(1000);
}

static inline void sdelay(u64 sec)
{
    u64 i;
    for (i = 0; i < sec; i++)
        mdelay(1000);
}

static void output_data (struct sata_ioports *ioaddr, u16 * sect_buf, int words)
{
    while (words--) {
        __raw_writew (*sect_buf++, (void *)ioaddr->data_addr);
    }
}

static int input_data (struct sata_ioports *ioaddr, u16 * sect_buf, int words)
{
    while (words--) {
        *sect_buf++ = __raw_readw ((void *)ioaddr->data_addr);
    }
    return 0;
}
static inline void sync(void)
{
    return;
}

static void sil_sata_port (struct sata_ioports *ioport)
{
    ioport->data_addr = ioport->cmd_addr + ATA_REG_DATA;
    ioport->error_addr = ioport->cmd_addr + ATA_REG_ERR;
    ioport->feature_addr = ioport->cmd_addr + ATA_REG_FEATURE;
    ioport->nsect_addr = ioport->cmd_addr + ATA_REG_NSECT;
    ioport->lbal_addr = ioport->cmd_addr + ATA_REG_LBAL;
    ioport->lbam_addr = ioport->cmd_addr + ATA_REG_LBAM;
    ioport->lbah_addr = ioport->cmd_addr + ATA_REG_LBAH;
    ioport->device_addr = ioport->cmd_addr + ATA_REG_DEVICE;
    ioport->status_addr = ioport->cmd_addr + ATA_REG_STATUS;
    ioport->command_addr = ioport->cmd_addr + ATA_REG_CMD;
}

/* Check if device is connected to port */
int sil_sata_bus_probe (int portno)
{
    u32 port = g_iobase[5];
    u32 val;
    switch (portno) {
    case 0:
        port += VND_SSTATUS_CH0;
        break;
    case 1:
        port += VND_SSTATUS_CH1;
        break;
    case 2:
        port += VND_SSTATUS_CH2;
        break;
    case 3:
        port += VND_SSTATUS_CH3;
        break;
    default:
        return 0;
    }
    val = readl (port);
    if ((val & SATA_DET_PRES) == SATA_DET_PRES) {
        return 1;
    } else {
        return 0;
    }
}

static u8 sil_sata_chk_status (struct sata_ioports *ioaddr, u8 usealtstatus)
{
    if (!usealtstatus) {
        return readb (ioaddr->status_addr);
    } else {
        return readb (ioaddr->altstatus_addr);
    }
}

static u8 sil_sata_busy_wait (struct sata_ioports *ioaddr, int bits,
              unsigned int max, u8 usealtstatus)
{
    u8 status;

    do {
        if (!((status = sil_sata_chk_status (ioaddr, usealtstatus)) & bits)) {
            //printf("sil_sata_busy_wait(): status:%x\n",status);
            break;
        }
        udelay (1000);
        max--;
    } while ((status & bits) && (max > 0));

    return status;
}

static int sil_sata_bus_softreset (int port_no)
{
    u8 status = 0;

    sata_ports[port_no].dev_mask = 1;

    sata_ports[port_no].ctl_reg = 0x08; /*Default value of control reg */
    writeb (sata_ports[port_no].ctl_reg, sata_ports[port_no].ioaddr.ctl_addr);
    udelay (10);
    writeb (sata_ports[port_no].ctl_reg | ATA_SRST, sata_ports[port_no].ioaddr.ctl_addr);
    udelay (10);
    writeb (sata_ports[port_no].ctl_reg, sata_ports[port_no].ioaddr.ctl_addr);

    /* spec mandates ">= 2ms" before checking status.
     * We wait 150ms, because that was the magic delay used for
     * ATAPI devices in Hale Landis's ATADRVR, for the period of time
     * between when the ATA command register is written, and then
     * status is checked.  Because waiting for "a while" before
     * checking status is fine, post SRST, we perform this magic
     * delay here as well.
     */
    mdelay(150);
    status = sil_sata_busy_wait (&sata_ports[port_no].ioaddr, ATA_BUSY, 300, 0);
    while ((status & ATA_BUSY)) {
        mdelay(150);
        status = sil_sata_busy_wait (&sata_ports[port_no].ioaddr, ATA_BUSY, 3, 0);
    }

    if (status & ATA_BUSY) {
        printf ("ata%u is slow to respond,plz be patient\n", sata_ports);
    }

    while ((status & ATA_BUSY)) {
        mdelay(100);
        status = sil_sata_chk_status (&sata_ports[port_no].ioaddr, 0);
    }

    if (status & ATA_BUSY) {
        printf ("ata%u failed to respond : ", sata_ports);
        printf ("bus reset failed\n");
        sata_ports[port_no].dev_mask = 0;
        return 1;
    }
    return 0;
}

int sil_sata_phy_reset (int port_no)
{
    u32 port = g_iobase[5];
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
        return 0;
    }
    val = readl (port);
    writel (val | SATA_SC_DET_RST, port);
    mdelay(150);
    writel (val & ~SATA_SC_DET_RST, port);
    return 0;
}

#if 1
u32 strnlen(const char* s, u32 count)
{
        const char *sc;
        for(sc = s;count -- && *sc != '\0';++sc)
        {
        }
        return (sc -s);
}
u64 ata_id_n_sectors(u16 *id)
{
        if (ata_id_has_lba(id)) {
                if (ata_id_has_lba48(id))
                        return ata_id_u64(id, ATA_ID_LBA48_SECTORS);
                else
                        return ata_id_u32(id, ATA_ID_LBA_SECTORS);
        } else {
                return 0;
        }
}

u32 ata_dev_classify(u32 sig)
{
        u8 lbam, lbah;

        lbam = (sig >> 16) & 0xff;
        lbah = (sig >> 24) & 0xff;

        if (((lbam == 0) && (lbah == 0)) ||
                ((lbam == 0x3c) && (lbah == 0xc3)))
                return ATA_DEV_ATA;

        if ((lbam == 0x14) && (lbah == 0xeb))
                return ATA_DEV_ATAPI;

        if ((lbam == 0x69) && (lbah == 0x96))
                return ATA_DEV_PMP;

        return ATA_DEV_UNKNOWN;
}

 void ata_id_string(const u16 *id, unsigned char *s, unsigned int ofs, unsigned int len)
{
        unsigned int c;

        while (len > 0) {
                c = id[ofs] >> 8;
                *s = c;
                s++;

                c = id[ofs] & 0xff;
                *s = c;
                s++;

                ofs++;
                len -= 2;
        }
}

void ata_id_c_string(const u16 *id, unsigned char *s,unsigned int ofs, unsigned int len)
{
        unsigned char *p;

        ata_id_string(id, s, ofs, len - 1);

        p = s + strnlen((char *)s, len - 1);
        while (p > s && p[-1] == ' ')
                p--;
        *p = '\0';
}

void ata_dump_id(u16 *id)
{
        unsigned char serial[ATA_ID_SERNO_LEN + 1];
        unsigned char firmware[ATA_ID_FW_REV_LEN + 1];
        unsigned char product[ATA_ID_PROD_LEN + 1];
        u64 n_sectors;

        /* Serial number */
        ata_id_c_string(id, serial, ATA_ID_SERNO, sizeof(serial));
        printf("S/N: %s\n\r", serial);

        /* Firmware version */
        ata_id_c_string(id, firmware, ATA_ID_FW_REV, sizeof(firmware));
        printf("Firmware version: %s\n\r", firmware);

        /* Product model */
        ata_id_c_string(id, product, ATA_ID_PROD, sizeof(product));
        printf("Product model number: %s\n\r", product);

        /* Total sectors of device  */
        n_sectors = ata_id_n_sectors(id);
        printf("Capablity: %d sectors\n\r", n_sectors);

        printf ("id[49]: capabilities = 0x%04x\n"
                "id[53]: field valid = 0x%04x\n"
                "id[63]: mwdma = 0x%04x\n"
                "id[64]: pio = 0x%04x\n"
                "id[75]: queue depth = 0x%04x\n",
                id[49],
                id[53],
                id[63],
                id[64],
                id[75]);

        printf ("id[76]: sata capablity = 0x%04x\n"
                "id[78]: sata features supported = 0x%04x\n"
                "id[79]: sata features enable = 0x%04x\n",
                id[76],
                id[78],
                id[79]);

        printf ("id[80]: major version = 0x%04x\n"
                "id[81]: minor version = 0x%04x\n"
                "id[82]: command set supported 1 = 0x%04x\n"
                "id[83]: command set supported 2 = 0x%04x\n"
                "id[84]: command set extension = 0x%04x\n",
                id[80],
                id[81],
                id[82],
                id[83],
                id[84]);
        printf ("id[85]: command set enable 1 = 0x%04x\n"
                "id[86]: command set enable 2 = 0x%04x\n"
                "id[87]: command set default = 0x%04x\n"
                "id[88]: udma = 0x%04x\n"
                "id[93]: hardware reset result = 0x%04x\n",
                id[85],
                id[86],
                id[87],
                id[88],
                id[93]);
}

void ata_swap_buf_le16(u16 *buf, unsigned int buf_words)
{
        unsigned int i;

        for (i = 0; i < buf_words; i++)
                buf[i] = (u16)buf[i];
}

#endif

void dump_print(u32 reg,u32 num)
{
    u32 i;
    u32 j = 0;
    printf("\n\n%08lx to %08lx shown below\n",reg,reg + num);
    for(i = reg;i < reg + num;i+=4)
    {
        if(j % 4 == 0)
            printf("\n%08lx\t",i);
        printf("%08lx\t",inl(i));
        j ++;
    }
}

/*
 *
 * Returns sectors read(DMA)
*/
static u32 sil_do_one_read(int device, ulong block, u16 blkcnt, u16 * buff,
               uchar lba48)
{

    u16 sr = blkcnt;
    u32 blknr =  block;
    u32 tmp, addr;

    unsigned char port_no = sata_dev[device].port_no;


    /*printf("sil_do_one_read:device:%x block:%x blkcnt:%x buff:%x uchar:%x port_no:%x blknr:%x\n",device,
           block, blkcnt, (unsigned int)buff, lba48,port_no,blknr);*/
    
    if (!(sil_sata_chk_status (&sata_ports[port_no].ioaddr, 0) & ATA_DRDY)) {
        printf ("Device%d not ready\n", device);
        return 0;
    }

    //Issue DMA read command
#ifdef CONFIG_LBA48
    if (lba48) {
        //printf("sil_do_one_read(): write high bits.\n");
        /* write high bits */
        writeb ((blkcnt >> 8) & 0xFF, sata_ports[port_no].ioaddr.nsect_addr);
        writeb ((blknr >> 24) & 0xFF, sata_ports[port_no].ioaddr.lbal_addr);
        //writeb ((blknr >> 32) & 0xFF, sata_ports[port_no].ioaddr.lbam_addr);
        //writeb ((blknr >> 40) & 0xFF, sata_ports[port_no].ioaddr.lbah_addr);
        writeb (0x0, sata_ports[port_no].ioaddr.lbam_addr);
        writeb (0x0, sata_ports[port_no].ioaddr.lbah_addr);
    }
#endif

    //printf("sil_do_one_read(): write low bits.\n");
    writeb ((blkcnt >> 0) & 0xFF, sata_ports[port_no].ioaddr.nsect_addr);
    writeb ((blknr >> 0) & 0xFF, sata_ports[port_no].ioaddr.lbal_addr);
    writeb ((blknr >> 8) & 0xFF, sata_ports[port_no].ioaddr.lbam_addr);
    writeb ((blknr >> 16) & 0xFF, sata_ports[port_no].ioaddr.lbah_addr);

#ifdef CONFIG_LBA48
    if (lba48) {
        //printf("sil_do_one_read(): issue DMA_READ_EXT.\n");
        writeb (0x40, sata_ports[port_no].ioaddr.device_addr);
        writeb (ATA_CMD_DMA_READ_EXT, sata_ports[port_no].ioaddr.command_addr);
    } else
#endif
    {
        //printf("sil_do_one_read(): issue DMA_READ.\n");
        writeb (0x40 | ((blknr >> 24) & 0xF),
            sata_ports[port_no].ioaddr.device_addr);
        writeb (ATA_CMD_DMA_READ, sata_ports[port_no].ioaddr.command_addr);
    }   

     //Clear bit 17 and bit 18 in the PCI Bus Master2
    addr = g_iobase[5] + sil_port[port_no].bmdma2;
    tmp = readl(addr);
    tmp |= 0x30000;
    writel(tmp, addr);
    tmp = readl(addr); 
   
    //Create PRD Table
     {
         unsigned long prdtable_base = (unsigned long)&sil_prd_entry[port_no];
         *(unsigned long *)(prdtable_base) = (unsigned long)vtophys((unsigned int)buff);

         addr = sata_ports[port_no].ioaddr.bmdma_addr + ATA_DMA_TABLE_OFS;
         writel((unsigned long)vtophys(prdtable_base), addr);
     }

    //need flush all cache,important here!! 
    flushcache();

    //Enable DMA transfer(read)
    addr = g_iobase[5] + sil_port[port_no].bmdma2;
    tmp = readl(addr);
    tmp |= 0x8;
    tmp |= 0x1;
    writel(tmp, addr);

    //Wait for PCI interrupt
    addr = g_iobase[5] + sil_port[port_no].bmdma2;
    while(1) {
        tmp = readl(addr);
        //printf("sil_do_one_read bmdma2(tmp):%x\n",tmp);
        tmp &= 0x70000;
        tmp >>=16;
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
    addr = g_iobase[5] + sil_port[port_no].bmdma2;
    tmp = readl(addr);
    tmp &= ~0x1;
    writel(tmp, addr); 
    //clear device interrupt
    
    //clear 18bit of PCI Bus Master2
    addr = g_iobase[5] + sil_port[port_no].bmdma2;
    tmp = readl(addr);
    tmp |= 0x40000;
    writel(tmp, addr);
    return sr;

read_err:
    printf("sil_do_one_read DMA read error!\n");
   
    return sr;
}

ulong sil_sata_read (int device, ulong block, lbaint_t blkcnt, void *buff)
{
    ulong n = 0;
    u16 sread;
    u16 *buffer = (u16 *) buff;
    u32 status = 0;
    u32 blknr = block;
    unsigned char lba48 = 0;
    int i = 0;

    p_stor_desc = &sata_dev[device].stor_desc;
#ifdef CONFIG_LBA48
        if (p_stor_desc->lba48) {
            lba48 = 1;
        } 
#endif

    while (blkcnt > 0) {
        if (lba48) {
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

        status = sil_do_one_read(device, blknr, sread, buffer, lba48);

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

//not finished,for not need till now
ulong sil_sata_write (int device, ulong block, lbaint_t blkcnt, const void *buff)
{    
    return 0;
}

static void sil_set_mode(int port_no)
{
    u32 tmp,addr = g_iobase[5] + sil_port[port_no].xfer_mode;

    tmp = readl(addr);
    tmp &= ~((1<<5) | (1<<4) | (1<<1) | (1<<0));
    tmp |=0x3;//device 0 udma 

    writel(tmp, addr);
    readl(addr); //flush
}

static void sil_sata_identify (int port_no,int dev_no)
{
    u8 cmd = 0, status = 0;
    u16 iobuf[ATA_SECTOR_WORDS];
    u64 n_sectors = 0;

    memset (iobuf, 0, sizeof (iobuf));

    if (!(sata_ports[port_no].dev_mask & 0x01)) {
        printf ("dev is not present on port#%d\n",  port_no);
        return;
    }

    p_stor_desc = &sata_dev[dev_no].stor_desc;

    status = 0;
    cmd = ATA_CMD_ID_ATA;   /*Device Identify Command */
    writeb (cmd, sata_ports[port_no].ioaddr.command_addr);
    readb (sata_ports[port_no].ioaddr.altstatus_addr);
    udelay (10);

    status = sil_sata_busy_wait (&sata_ports[port_no].ioaddr, ATA_BUSY, 1000, 0);
    if (status & ATA_ERR) {
        printf ("\ndevice not responding\n");
        sata_ports[port_no].dev_mask &= ~0x01;
        return;
    }

    input_data (&sata_ports[port_no].ioaddr, iobuf, ATA_SECTOR_WORDS);

    ata_swap_buf_le16 (iobuf, ATA_SECTOR_WORDS);

    //debug ("Specific config: %x\n", iobuf[2]);

    /* we require LBA and DMA support (bits 8 & 9 of word 49) */
    if (!ata_id_has_dma (iobuf) || !ata_id_has_lba (iobuf)) {
        debug ("ata%u: no dma/lba\n", sata_ports);
    }
#ifdef DEBUG
    ata_dump_id (iobuf);
#endif

    n_sectors = ata_id_n_sectors (iobuf);

    if (n_sectors == 0) {
        sata_ports[port_no].dev_mask &= ~0x01;
        return;
    }
    //to do
    ata_id_c_string (iobuf, (unsigned char *)p_stor_desc->revision,
             ATA_ID_FW_REV, sizeof (p_stor_desc->revision));
    ata_id_c_string (iobuf, (unsigned char *)p_stor_desc->vendor,
             ATA_ID_PROD, sizeof (p_stor_desc->vendor));
    ata_id_c_string (iobuf, (unsigned char *)p_stor_desc->product,
             ATA_ID_SERNO, sizeof (p_stor_desc->product));

    /* TODO - atm we asume harddisk ie not removable */
    p_stor_desc->removable = 0;

    p_stor_desc->lba = (u32) n_sectors;
    //debug ("lba=0x%x\n", p_stor_desc->lba);

#ifdef CONFIG_LBA48
    if (iobuf[83] & (1 << 10)) {
        p_stor_desc->lba48 = 1;
    } else {
        p_stor_desc->lba48 = 0;
    }
#endif

    /* assuming HD */
    p_stor_desc->type = DEV_TYPE_HARDDISK;
    p_stor_desc->blksz = ATA_SECT_SIZE;
    p_stor_desc->lun = 0;   /* just to fill something in... */
}

static void sil_set_feature_cmd (int port_no)//(int num, int dev)
{
    u8 status = 0;

    if (!(sata_ports[port_no].dev_mask & 0x01)) {
        debug ("dev is not present on port#%d\n", port_no);
        return;
    }

    writeb (SETFEATURES_XFER, sata_ports[port_no].ioaddr.feature_addr);
    //writeb (XFER_PIO_4, sata_ports[port_no].ioaddr.nsect_addr);
    //writeb (XFER_UDMA_4, sata_ports[port_no].ioaddr.nsect_addr);

    writeb (XFER_MW_DMA_2, sata_ports[port_no].ioaddr.nsect_addr);
    writeb (0, sata_ports[port_no].ioaddr.lbal_addr);
    writeb (0, sata_ports[port_no].ioaddr.lbam_addr);
    writeb (0, sata_ports[port_no].ioaddr.lbah_addr);

    writeb (ATA_DEVICE_OBS, sata_ports[port_no].ioaddr.device_addr);
    writeb (ATA_CMD_SET_FEATURES, sata_ports[port_no].ioaddr.command_addr);

    udelay (50);
    mdelay(150);

    status = sil_sata_busy_wait (&sata_ports[port_no].ioaddr, ATA_BUSY, 5000, 0);
    if ((status & (ATA_BUSY | ATA_ERR))) {
        printf ("Error  : status 0x%02x\n", status);
        sata_ports[port_no].dev_mask &= ~0x01;
    }
}

int sil_sata_scan (int port_no)
{
    u32 prd_base;
    /* A bit brain dead, but the code has a legacy */
    switch (port_no) {
    case 0:
        sata_ports[0].port_no = 0;
        sata_ports[0].ioaddr.cmd_addr = g_iobase[5] + VND_TF0_CH0;
        sata_ports[0].ioaddr.altstatus_addr = sata_ports[0].ioaddr.ctl_addr =
            (g_iobase[5] + VND_TF2_CH0) | ATA_PCI_CTL_OFS;
        sata_ports[0].ioaddr.bmdma_addr = g_iobase[5] + VND_BMDMA_CH0;
        break;
    case 1:
        sata_ports[1].port_no = 1;
        sata_ports[1].ioaddr.cmd_addr = g_iobase[5] + VND_TF0_CH1;
        sata_ports[1].ioaddr.altstatus_addr = sata_ports[1].ioaddr.ctl_addr =
            (g_iobase[5] + VND_TF2_CH1) | ATA_PCI_CTL_OFS;
        sata_ports[1].ioaddr.bmdma_addr = g_iobase[5] + VND_BMDMA_CH1;
        break;
    case 2:
        sata_ports[2].port_no = 2;
        sata_ports[2].ioaddr.cmd_addr = g_iobase[5] + VND_TF0_CH2;
        sata_ports[2].ioaddr.altstatus_addr = sata_ports[2].ioaddr.ctl_addr =
            (g_iobase[5] + VND_TF2_CH2) | ATA_PCI_CTL_OFS;
        sata_ports[2].ioaddr.bmdma_addr = g_iobase[5] + VND_BMDMA_CH2;
        break;
    case 3:
        sata_ports[3].port_no = 3;
        sata_ports[3].ioaddr.cmd_addr = g_iobase[5] + VND_TF0_CH3;
        sata_ports[3].ioaddr.altstatus_addr = sata_ports[3].ioaddr.ctl_addr =
            (g_iobase[5] + VND_TF2_CH3) | ATA_PCI_CTL_OFS;
        sata_ports[3].ioaddr.bmdma_addr =g_iobase[5] + VND_BMDMA_CH3;
        break;
    default:
        printf ("Tried to scan unknown port: ata%d\n", port_no);
        return 1;
    }

    /* Initialize other registers */
    sil_sata_port(&sata_ports[port_no].ioaddr);

    /* Check for attached device */
    if (!sil_sata_bus_probe (port_no)) {
        sata_ports[port_no].port_state = 0;
        //debug ("SATA#%d port is not present\n", port_no);
    } else {
        //debug ("SATA#%d port is present\n", port_no);
        if (sil_sata_bus_softreset (port_no)) {
            /* soft reset failed, try a hard one */
            sil_sata_phy_reset (port_no);
            if (sil_sata_bus_softreset (port_no)) {
                sata_ports[port_no].port_state = 0;
            } else {
                sata_ports[port_no].port_state = 1;
            }
        } else {
            sata_ports[port_no].port_state = 1;
        }
    }
    if (sata_ports[port_no].port_state == 1) {
        curr_dev++;
        prd_base =  (unsigned int)&sil_prd_entry[port_no];
        prd_base = (unsigned int) vtophys(prd_base);
        sata_ports[port_no].p_prd_table = prd_base;
        sata_dev[curr_dev].device_no = curr_dev;
        sata_dev[curr_dev].port_no = port_no;  
        p_stor_desc = &sata_dev[curr_dev].stor_desc;
        memset(p_stor_desc, 0, sizeof(struct block_dev_desc));
        p_stor_desc->if_type =  IF_TYPE_SATA;
        p_stor_desc->dev = curr_dev;
        p_stor_desc->part_type = PART_TYPE_UNKNOWN;
        p_stor_desc->type = DEV_TYPE_HARDDISK;
        p_stor_desc->lba = 0;
        p_stor_desc->blksz = 512;
        p_stor_desc->block_read = sil_sata_read;
        p_stor_desc->block_write = sil_sata_write;
        /* Probe device and set xfer mode */
        sil_sata_identify (port_no,curr_dev);
        sil_set_feature_cmd (port_no);
        sil_set_mode(port_no);
        config_found(p_sil_device, &sata_dev[curr_dev], NULL);
    }
    return 0;
}

 void sil_init_part (block_dev_desc_t* dev_desc)
{
    return;
}

int sil_sata_initialize(u32 reg,u32 flags)
{
    int rc;
    int i;

    for (i = 0; i < CONFIG_SYS_SATA_MAX_DEVICE; i++) {
        rc = sil_sata_scan(i);
        if ((p_stor_desc->lba > 0) && (p_stor_desc->blksz > 0))
            sil_init_part(p_stor_desc);        
    }
    return rc;
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

            printf("\nSATA read: device %d block # %ld, count %ld ... \n",
                curr_dev, blk, cnt);

            n = sil_sata_read(curr_dev, blk, cnt, (u32 *)addr);

            /* flush cache after read */
            //flush_cache(addr, cnt * sata_dev_desc[curr_device].blksz);

            printf("%ld blocks read: %s\n",
                n, (n==cnt) ? "OK" : "ERROR");
            return (n == cnt) ? 0 : 1;
        } else if (strcmp(argv[1], "write") == 0) {
            ulong addr = strtoul(argv[2], NULL, 16);
            ulong cnt = strtoul(argv[4], NULL, 16);
            ulong n;

            lbaint_t blk = strtoul(argv[3], NULL, 16);

            printf("\nSATA write: device %d block # %ld, count %ld ... ",
                curr_dev, blk, cnt);

            n = sil_sata_write(curr_dev, blk, cnt, (u32 *)addr);

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

