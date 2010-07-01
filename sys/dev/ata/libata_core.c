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
 File name:     libata-core.c
 Author:  qianyl      Version:  ***      Date: ***
 Description:   
 Others:        
 Function List:
 
 Revision History:
 
 ---------------------------------------------------------------------------------------------
  Date              Author           Activity ID             Activity Headline
  2008-06-26    QianYuli        PMON00000002    porting it from linux
*************************************************************************/
/*
 * libata-core.c - helper library for ATA
 */

#include <linux/libata.h>
#include <stdio.h>
//tmp
#define ERANGE      34  /* Math result not representable */
#define EINVAL      22  /* Invalid argument */
#ifndef NULL
#define NULL    0
#endif
extern void udelay(int usec);

extern unsigned int ata_sff_dev_classify(p_ata_port_t);
extern u8 ata_sff_busy_wait (struct ata_port *,int ,unsigned int ,u8);

struct ata_port_operations ata_base_port_ops = {
    .devchk              = ata_devchk,
    .port_identify     = ata_identify,
};

static void output_data (struct ata_ioports *p_ioaddr, u16 *sect_buf, int words)
{
    while (words--) {
        __raw_writew (*sect_buf++, (void *)p_ioaddr->data_addr);
    }
}

static int input_data (struct ata_ioports *p_ioaddr, u32 *sect_buf, int dwords)
{
    while (dwords--) {
        *sect_buf++ = __raw_readl ((void *)p_ioaddr->data_addr);
    }
    return 0;
}

void ata_port (struct ata_ioports *p_ioport)
{
    p_ioport->data_addr = p_ioport->cmd_addr + ATA_REG_DATA;
    p_ioport->error_addr = p_ioport->cmd_addr + ATA_REG_ERR;
    p_ioport->feature_addr = p_ioport->cmd_addr + ATA_REG_FEATURE;
    p_ioport->nsect_addr = p_ioport->cmd_addr + ATA_REG_NSECT;
    p_ioport->lbal_addr = p_ioport->cmd_addr + ATA_REG_LBAL;
    p_ioport->lbam_addr = p_ioport->cmd_addr + ATA_REG_LBAM;
    p_ioport->lbah_addr = p_ioport->cmd_addr + ATA_REG_LBAH;
    p_ioport->device_addr = p_ioport->cmd_addr + ATA_REG_DEVICE;
    p_ioport->status_addr = p_ioport->cmd_addr + ATA_REG_STATUS;
    p_ioport->command_addr = p_ioport->cmd_addr + ATA_REG_CMD;
}

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

unsigned int ata_dev_classify(const struct ata_taskfile *p_tf)
{

        if((p_tf->lbam == 0) && (p_tf->lbah == 0)) {
            return ATA_DEV_ATA;
        }
        
        if((p_tf->lbam == 0x14) && (p_tf->lbah == 0xeb)) {
            return ATA_DEV_ATAPI;
        }

        if ((p_tf->lbam == 0x69) && (p_tf->lbah == 0x96)) {
            return ATA_DEV_PMP;
        }

        if((p_tf->lbam == 0x3c) && (p_tf->lbah == 0xc3)) {
            return ATA_DEV_SEMB;
        }

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

static const u8 ata_rw_cmds[] = {
    /* pio multi */
    ATA_CMD_READ_MULTI,
    ATA_CMD_WRITE_MULTI,
    ATA_CMD_READ_MULTI_EXT,
    ATA_CMD_WRITE_MULTI_EXT,
    0,
    0,
    0,
    ATA_CMD_WRITE_MULTI_FUA_EXT,
    /* pio */
    ATA_CMD_PIO_READ,
    ATA_CMD_PIO_WRITE,
    ATA_CMD_PIO_READ_EXT,
    ATA_CMD_PIO_WRITE_EXT,
    0,
    0,
    0,
    0,
    /* dma */
    ATA_CMD_READ,
    ATA_CMD_WRITE,
    ATA_CMD_READ_EXT,
    ATA_CMD_WRITE_EXT,
    0,
    0,
    0,
    ATA_CMD_WRITE_FUA_EXT
};

static int ata_rwcmd_protocol(struct ata_taskfile *tf, struct ata_port *p_ap)
{
    u8 cmd;

    int index, fua, lba48, write;

    fua = (tf->flags & ATA_TFLAG_FUA) ? 4 : 0;
    lba48 = (tf->flags & ATA_TFLAG_LBA48) ? 2 : 0;
    write = (tf->flags & ATA_TFLAG_WRITE) ? 1 : 0;

    if (p_ap->dflags & ATA_DFLAG_PIO) {
        tf->protocol = ATA_PROT_PIO;
        index = p_ap->multi_count ? 0 : 8;
    } else if (lba48 && (p_ap->flags & ATA_FLAG_PIO_LBA48)) {
        /* Unable to use DMA due to host limitation */
        tf->protocol = ATA_PROT_PIO;
        index = p_ap->multi_count ? 0 : 8;
    } else {
        tf->protocol = ATA_PROT_DMA;
        index = 16;
    }

    cmd = ata_rw_cmds[index + fua + lba48 + write];
    if (cmd) {
        tf->command = cmd;
        return 0;
    }
    return -1;
}

int ata_build_rw_tf(struct ata_port *p_ap,u64 block, u32 n_block, unsigned int tf_flags,
            unsigned int tag)
{
    struct ata_taskfile *p_tf = p_ap->p_tf;
    p_tf->flags |= ATA_TFLAG_ISADDR | ATA_TFLAG_DEVICE;
    p_tf->flags |= tf_flags;

    //printf("--ata_build_rw_tf--  block:%x n_block:%x\n",block,n_block);

    if (p_ap->dflags & ATA_DFLAG_LBA) {
        p_tf->flags |= ATA_TFLAG_LBA;

        if (lba_28_ok(block, n_block)) {
            /* use LBA28 */
            p_tf->device |= (block >> 24) & 0xf;
        } else if (lba_48_ok(block, n_block)) {
            if (!(p_ap->dflags & ATA_DFLAG_LBA48)){
                printf("p_ap->dflags & ATA_DFLAG_LBA48 failed.\n");
                return -ERANGE;
            }

            /* use LBA48 */
            p_tf->flags |= ATA_TFLAG_LBA48;

            p_tf->hob_nsect = (n_block >> 8) & 0xff;

            p_tf->hob_lbah = (block >> 40) & 0xff;
            p_tf->hob_lbam = (block >> 32) & 0xff;
            p_tf->hob_lbal = (block >> 24) & 0xff;
        } else{
            /* request too large even for LBA48 */
            printf("--ata_build_rw_tf-- request too large even for LBA48.\n");
            return -ERANGE;
        }

        if (ata_rwcmd_protocol(p_tf, p_ap) < 0)
            return -EINVAL;

        p_tf->nsect = n_block & 0xff;

        p_tf->lbah = (block >> 16) & 0xff;
        p_tf->lbam = (block >> 8) & 0xff;
        p_tf->lbal = block & 0xff;

        p_tf->device |= ATA_LBA;
    } else {
        /* CHS */
        //not supported now,for it is too old        
    }

    return 0;
}

int ata_dev_configure(struct ata_port *p_ap,u16 *p_id)
{
    int sr = 1;
    if (ata_id_has_dma(p_id)){
        p_ap->pflags |= ATA_PROT_FLAG_DMA;
        sr =0;
    }

    if (ata_id_has_lba(p_id)) {
        p_ap->dflags |= ATA_DFLAG_LBA;
        sr = 0;
    }

    if (ata_id_has_lba48(p_id)) {
        p_ap->dflags |= ATA_DFLAG_LBA48;
        sr = 0;
    }

    //printf("--ata_dev_configure-- pid[88]:%x pid[63]:%x  pid[64]:%x \n",p_id[88],p_id[63],p_id[64]);

    p_ap->udma = p_id[88] & 0xff;
    p_ap->mwdma = p_id[63] & 0xff;
    p_ap->pio = p_id[64] & 0xff;
    return sr;
}

int ata_identify (struct ata_port  *p_ap)
{
    u8 cmd = 0, status = 0;
    u16 iobuf[ATA_SECTOR_WORDS];
    u32 *p_iobuf = (u32 *)iobuf;

    memset (iobuf, 0, sizeof (iobuf));
    
    ata_sff_dev_classify(p_ap);
    switch(p_ap->dclass) {
    case ATA_DEV_ATA:
        status =0;
        cmd = ATA_CMD_ID_ATA;
        writeb(cmd, p_ap->ioaddr.command_addr);
        readb(p_ap->ioaddr.altstatus_addr);
        udelay(10);
        status = ata_sff_busy_wait(p_ap,ATA_BUSY,1000,0);
        if (status & ATA_ERR) {
            printf ("\ndevice not responding\n");
            return 1;
        }
        input_data (&p_ap->ioaddr, p_iobuf, ATA_SECTOR_DWORDS);
        ata_swap_buf_le16 (iobuf, ATA_SECTOR_WORDS);
        ata_dev_configure(p_ap, iobuf);
        break;
    case ATA_DEV_ATAPI:
    case ATA_DEV_PMP:
    case ATA_DEV_SEMB:    
    default:
            break;
    }
    return 0;
}
//empty yet
int ata_port_start(struct ata_port *ap)
{
    return 0;
}

static void ata_finalize_port_ops(p_ata_port_operations_t  p_ops)
{
    p_ata_port_operations_t  p_cur;
    void **begin = (void **)p_ops;
    void **end = (void **)&p_ops->inherits;
    void **pp;

    if (!p_ops || !p_ops->inherits) {
        return;
    }
    for ( p_cur = p_ops->inherits; p_cur; p_cur = p_cur->inherits) {
        void **inherits = (void **)p_cur;
        for( pp = begin; pp < end; pp++, inherits++) {
            if (!*pp) {
                *pp = *inherits;
            }
        }
    }

    p_ops->inherits = NULL;
}

int ata_set_mode_cmd(struct ata_port *p_ap, u8 mode)
{
    u8 status = 0;

    //printf("ata_set_mod_cmd: mode:%x\n",mode);
    writeb (SETFEATURES_XFER, p_ap->ioaddr.feature_addr);
    writeb (mode, p_ap->ioaddr.nsect_addr);
    writeb (0, p_ap->ioaddr.lbal_addr);
    writeb (0, p_ap->ioaddr.lbam_addr);
    writeb (0, p_ap->ioaddr.lbah_addr);

    writeb (ATA_DEVICE_OBS, p_ap->ioaddr.device_addr);
    writeb (ATA_CMD_SET_FEATURES, p_ap->ioaddr.command_addr);

    udelay (50);
    mdelay(150);
 
    status = ata_sff_busy_wait(p_ap,ATA_BUSY,5000,0);
    if ((status & (ATA_BUSY | ATA_ERR))) {
        printf ("Error  : status 0x%02x\n", status); 
        return 1;
    }
    return 0;
}

int ata_get_highest_mode(u8 a_mode)
{
    int i = 7;
        if(a_mode == 0) {
            return -1;
        }
        while(!(a_mode & 0x80)&&(i>0))
        {     
            a_mode <<= 1;
            i--;
        }
        return i;
}
int ata_do_set_mode(struct ata_port *p_ap)
{
    u8 mode = 0;
    if (p_ap->udma) {
        mode = 0x40 | ata_get_highest_mode(p_ap->udma&0xff);
        p_ap->trans_mode = ATA_TRANS_UDMA;
    }
    else if (p_ap->mwdma) {
        mode = 0x20 | ata_get_highest_mode(p_ap->mwdma&0xff);
        p_ap->trans_mode = ATA_TRANS_MWDMA;
    }
    else if (p_ap->pio) {
        mode = 0x4 | ata_get_highest_mode(p_ap->pio&0xff);
        p_ap->trans_mode = ATA_TRANS_PIO;
    }
    return mode?ata_set_mode_cmd(p_ap, mode):1;
}

int ata_host_start(p_ata_host_t p_host)
{
    int i,rc;
    ata_finalize_port_ops(p_host->p_ops);

    for (i = 0; i < p_host->n_ports; i++) {
        p_ata_port_t  p_ap = p_host->pp_ports[i];
        ata_finalize_port_ops(p_ap->p_ops);
    }

    for (i = 0; i < p_host->n_ports; i++) {
        p_ata_port_t p_ap = p_host->pp_ports[i];
        if ( p_ap->p_ops->port_start) {
            rc = p_ap->p_ops->port_start(p_ap);
            if (rc) {
                goto err_out;
            }
        }
    }
   
    p_host->flags |= ATA_HOST_STARTED;
    return 0;

 err_out:
     while (--i >= 0) {
         p_ata_port_t  p_ap = p_host->pp_ports[i];
         if (p_ap->p_ops->port_stop) {
             p_ap->p_ops->port_stop(p_ap);
         }
     }
     return rc;
}

//not finished yet
int ata_host_register(p_ata_host_t  p_host)
{
    int i;
    static int dev_no = 0;
    //Host must have been started
    if(!(p_host->flags & ATA_HOST_STARTED)) {
        printf("Trying to register unstarted host");
        return -EINVAL;
    }
     
    for (i = 0; i < p_host->n_ports; i++) {
        p_ata_port_t  p_ap = p_host->pp_ports[i];
        p_ata_port_operations_t p_apo = p_host->p_ops;
        /*to do list 1*/
        //probe whether device attached to the specified port
        if (p_apo && p_apo->devchk) {
            if (p_apo->devchk(p_ap)) {
                p_ap->port_state &= ~ATA_PORT_DEVICE_ATTACHED; 
                continue;
            }
        } else{
            continue;
        }
        
        /*to do list 2*/
        //reset device if it exists
        if(p_apo->softreset) {
            if(p_apo->softreset(p_ap)) {
                if(p_apo->hardreset) {
                    p_apo->hardreset(p_ap);                 
                }
                if(p_apo->softreset(p_ap)) {
                    p_ap->port_state &= ~ATA_PORT_DEVICE_ATTACHED;
                    continue;
                }else{
                    p_ap->port_state |= ATA_PORT_DEVICE_ATTACHED;
                }
            } else{
                p_ap->port_state |= ATA_PORT_DEVICE_ATTACHED;
            }
        }else{
            p_ap->port_state &= ~ATA_PORT_DEVICE_ATTACHED;
            continue;
        }

        p_host->pp_dev[dev_no]->port_no = i;
        p_ap->dev_no = dev_no++;

        /*to do list 3*/
        //identify device
        if(p_apo->port_identify) {
            if(p_apo->port_identify(p_ap)){
                continue;
            }
        }

        /*to do list 4*/
        //set device transfer mode
        if (p_apo->set_mode) {
            if(p_apo->set_mode(p_ap) ){
                printf("ata_host_register(): set_mode failed,port_no:%d",p_ap->port_no);
                //set mode failed
            }
        }
    }
    return 0;
}

int ata_host_activate(p_ata_host_t  p_host)
{
    int rc;
    rc = ata_host_start(p_host);
    if (rc) {
        return rc;
    }

    return ata_host_register(p_host);
}
