/*
 *  libata-sff.c - helper library for PCI IDE BMDMA
 *
 *  Maintained by:  Jeff Garzik <jgarzik@pobox.com>
 *              Please ALWAYS copy linux-ide@vger.kernel.org
 *          on emails.
 *
 *  Copyright 2003-2006 Red Hat, Inc.  All rights reserved.
 *  Copyright 2003-2006 Jeff Garzik
 *
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *
 *  libata documentation is available via 'make {ps|pdf}docs',
 *  as Documentation/DocBook/libata.*
 *
 *  Hardware documentation available from http://www.t13.org/ and
 *  http://www.sata-io.org/
 *
 */
 /************************************************************************

 Copyright (C)
 File name:     libata_sff.c
 Author:  qianyl      Version:  ***      Date: ***
 Description:   
 Others:        
 Function List:
 
 Revision History:
 
 ----------------------------------------------------------------------------------------------
  Date              Author           Activity ID            Activity Headline
  2008-06-26    QianYuli        PMON00000002   porting it from linux
*************************************************************************/

#include <linux/libata.h>
#include <stdio.h>

extern int ata_port_start(struct ata_port *);


int ata_sff_softreset(struct ata_port *p_ap);
int sata_sff_hardreset(struct ata_port *p_ap);
u8 ata_sff_check_status(struct ata_port *p_ap, u8 usealtstatus);
void ata_sff_tf_load(struct ata_port *p_ap);
void ata_sff_tf_read(struct ata_port *p_ap, struct ata_taskfile *p_tf);
void ata_sff_exec_command(struct ata_port *p_ap);
int ata_sff_port_start(struct ata_port *p_ap);

void ata_bmdma_setup(struct ata_port *p_ap);
void ata_bmdma_start(struct ata_port *p_ap);
void ata_bmdma_stop(struct ata_port *p_ap);
u8 ata_bmdma_status(struct ata_port *p_ap);

extern struct ata_port_operations ata_base_port_ops;
struct ata_port_operations ata_sff_port_ops = {
    .inherits       = &ata_base_port_ops,
    .softreset      = ata_sff_softreset,
    .hardreset      = sata_sff_hardreset,
    .sff_check_status   = ata_sff_check_status,
    .sff_tf_load        = ata_sff_tf_load,
    .sff_tf_read        = ata_sff_tf_read,
    .sff_exec_command   = ata_sff_exec_command,
    .port_start     = ata_sff_port_start,
};

struct ata_port_operations ata_bmdma_port_ops = {
    .inherits       = &ata_sff_port_ops,
    .bmdma_setup        = ata_bmdma_setup,
    .bmdma_start        = ata_bmdma_start,
    .bmdma_stop     = ata_bmdma_stop,
    .bmdma_status       = ata_bmdma_status,
};

u8 ata_sff_check_status(struct ata_port *p_ap, u8 usealtstatus)
{
    if(!usealtstatus) {
         return ioread8(p_ap->ioaddr.status_addr);
    }else {
        return ioread8(p_ap->ioaddr.altstatus_addr);
    }
   
}

static u8 ata_sff_altstatus(struct ata_port *p_ap)
{
    if (p_ap->p_ops->sff_check_altstatus)
        return p_ap->p_ops->sff_check_altstatus(p_ap);

    return ioread8(p_ap->ioaddr.altstatus_addr);
}

static u8 ata_sff_irq_status(struct ata_port *p_ap)
{
    u8 status;

    status = ata_sff_check_status(p_ap,1);
    /* Not us: We are busy */
    if (status & ATA_BUSY)
            return status;
 
    /* Clear INTRQ latch */
    if(p_ap&&p_ap->p_ops) {
        status = p_ap->p_ops->sff_check_status(p_ap,0);
    }

    return status;
}

static void ata_sff_sync(struct ata_port *p_ap)
{
    if (p_ap->p_ops->sff_check_status)
        p_ap->p_ops->sff_check_status(p_ap,1);
    else if (p_ap->ioaddr.altstatus_addr)
        ioread8(p_ap->ioaddr.altstatus_addr);
}


void ata_sff_pause(struct ata_port *p_ap)
{
    ata_sff_sync(p_ap);
    udelay(1);
}

void ata_sff_dma_pause(struct ata_port *p_ap)
{
    if(p_ap&&p_ap->p_ops&&p_ap->p_ops->sff_check_status) {
        p_ap->p_ops->sff_check_status(p_ap,1);
        return;
    }
}

void ata_sff_irq_clear(struct ata_port *p_ap)
{
    void  *mmio = p_ap->ioaddr.bmdma_addr;

    if (!mmio)
        return;

    iowrite8(ioread8(mmio + ATA_DMA_STATUS), mmio + ATA_DMA_STATUS);
}

void ata_sff_tf_load(struct ata_port *p_ap)
{
    struct ata_taskfile *p_tf = p_ap->p_tf;
    struct ata_ioports *p_ioaddr = &p_ap->ioaddr;
    unsigned int is_addr = p_tf->flags & ATA_TFLAG_ISADDR;

    if (p_ioaddr->ctl_addr){
            iowrite8(p_tf->ctl, p_ioaddr->ctl_addr);
    }

    if (is_addr && (p_tf->flags & ATA_TFLAG_LBA48)) {
        iowrite8(p_tf->hob_feature, p_ioaddr->feature_addr);
        iowrite8(p_tf->hob_nsect, p_ioaddr->nsect_addr);
        iowrite8(p_tf->hob_lbal, p_ioaddr->lbal_addr);
        iowrite8(p_tf->hob_lbam, p_ioaddr->lbam_addr);
        iowrite8(p_tf->hob_lbah, p_ioaddr->lbah_addr);
    }

    if (is_addr) {
        iowrite8(p_tf->feature, p_ioaddr->feature_addr);
        iowrite8(p_tf->nsect, p_ioaddr->nsect_addr);
        iowrite8(p_tf->lbal, p_ioaddr->lbal_addr);
        iowrite8(p_tf->lbam, p_ioaddr->lbam_addr);
        iowrite8(p_tf->lbah, p_ioaddr->lbah_addr);
    }

    if (p_tf->flags & ATA_TFLAG_DEVICE) {
        iowrite8(p_tf->device, p_ioaddr->device_addr);   
    } 
}

void ata_sff_tf_read(struct ata_port *p_ap, struct ata_taskfile *p_tf)
{
    struct ata_ioports *p_ioaddr = &p_ap->ioaddr;

    p_tf->command = ata_sff_check_status(p_ap,0);
    p_tf->feature = ioread8(p_ioaddr->error_addr);
    p_tf->nsect = ioread8(p_ioaddr->nsect_addr);
    p_tf->lbal = ioread8(p_ioaddr->lbal_addr);
    p_tf->lbam = ioread8(p_ioaddr->lbam_addr);
    p_tf->lbah = ioread8(p_ioaddr->lbah_addr);
    p_tf->device = ioread8(p_ioaddr->device_addr);

    if (p_tf->flags & ATA_TFLAG_LBA48) {
        if (p_ioaddr->ctl_addr) {
            iowrite8(p_tf->ctl | ATA_HOB, p_ioaddr->ctl_addr);
            p_tf->hob_feature = ioread8(p_ioaddr->error_addr);
            p_tf->hob_nsect = ioread8(p_ioaddr->nsect_addr);
            p_tf->hob_lbal = ioread8(p_ioaddr->lbal_addr);
            p_tf->hob_lbam = ioread8(p_ioaddr->lbam_addr);
            p_tf->hob_lbah = ioread8(p_ioaddr->lbah_addr);
            iowrite8(p_tf->ctl, p_ioaddr->ctl_addr);
        } 
    }
    return;
}

void ata_sff_exec_command(struct ata_port *p_ap)
{
    struct ata_taskfile  *p_tf = p_ap->p_tf;
    iowrite8(p_tf->command, p_ap->ioaddr.command_addr);
    ata_sff_pause(p_ap);
}

void ata_tf_to_host(struct ata_port *p_ap)
{
    p_ap->p_ops->sff_tf_load(p_ap);
    p_ap->p_ops->sff_exec_command(p_ap);
}

unsigned int ata_devchk(struct ata_port *p_ap)
{
    struct ata_ioports *p_ioaddr = &p_ap->ioaddr;
    u8 nsect, lbal;


    iowrite8(0x55, p_ioaddr->nsect_addr);
    iowrite8(0xaa, p_ioaddr->lbal_addr);

    iowrite8(0xaa, p_ioaddr->nsect_addr);
    iowrite8(0x55, p_ioaddr->lbal_addr);

    iowrite8(0x55, p_ioaddr->nsect_addr);
    iowrite8(0xaa, p_ioaddr->lbal_addr);

    nsect = ioread8(p_ioaddr->nsect_addr);
    lbal = ioread8(p_ioaddr->lbal_addr);

    if ((nsect == 0x55) && (lbal == 0xaa))
        return 0;   /* we found a device */

    return 1;       /* nothing found */
}

unsigned int ata_sff_dev_classify(p_ata_port_t p_ap)
{ 
    struct ata_taskfile tf;
    unsigned int class;
    u8 err = 0;

    memset(&tf, 0, sizeof(tf));

    p_ap->p_ops->sff_tf_read(p_ap, &tf);
 

    /* determine  device's type*/
    class = ata_dev_classify(&tf);

    if (class == ATA_DEV_UNKNOWN) {
        if ((p_ap->port_state & ATA_PORT_DEVICE_ATTACHED) == ATA_PORT_DEVICE_ATTACHED)
            class = ATA_DEV_ATA;
        else
            class = ATA_DEV_NONE;
    } else if ((class == ATA_DEV_ATA) &&
           (p_ap->p_ops->sff_check_status(p_ap,0) == 0))
        class = ATA_DEV_NONE;

    p_ap->dclass = class;

    return err;
}

u8 ata_sff_busy_wait (struct ata_port *p_ap, int bits,
              unsigned int max, u8 usealtstatus)
{
    u8 status;

    do {
        if (!((status = ata_sff_check_status (p_ap, usealtstatus)) & bits)) {
            //printf("ata_sff_busy_wait(): status:%x\n",status);
            break;
        }
        udelay (1000);
        max--;
    } while ((status & bits) && (max > 0));

    return status;
}

int ata_sff_softreset(struct ata_port *p_ap)
{

    u8 status = 0;
    int timeout = 20;

    p_ap->ctl =0x08;    //Default value of control reg
    /* software reset.  causes dev0 to be selected */
    iowrite8(p_ap->ctl, p_ap->ioaddr.ctl_addr);
    udelay(20); /* FIXME: flush */
    iowrite8(p_ap->ctl | ATA_SRST, p_ap->ioaddr.ctl_addr);
    udelay(20); /* FIXME: flush */
    iowrite8(p_ap->ctl, p_ap->ioaddr.ctl_addr);

    mdelay(150);
    status = ata_sff_busy_wait(p_ap,ATA_BUSY,300,0);
    while((status & ATA_BUSY)&&timeout) {
        mdelay(150);
        status = ata_sff_busy_wait(p_ap, ATA_BUSY,3,0);
        timeout--;
    }

    if(status&ATA_BUSY) {
        printf("ata port%d's device%d is slow to respond,plz be patient.\n",p_ap->port_no,p_ap->dev_no);
    }
    /*
    while(status&ATA_BUSY) {
        mdelay(100);
        status = ata_sff_check_status(p_ap,0);
    }
    */
    if(status&ATA_BUSY) {
        printf("ata port%d's device%d failed to respond: ",p_ap->port_no,p_ap->dev_no);
        printf("bus reset failed\n");
        return 1;        
    }
    return 0;
}


int sata_sff_hardreset(struct ata_port *p_ap)
{
    if (p_ap&&p_ap->p_ops&&p_ap->p_ops->hardreset) {
        return p_ap->p_ops->hardreset(p_ap);
    }
    else
        return 2;
}

int ata_sff_port_start(struct ata_port *p_ap)
{
    if (p_ap->ioaddr.bmdma_addr)
        return ata_port_start(p_ap);
    return 0;
}

int ata_sff_port_start32(struct ata_port *p_ap)
{
    p_ap->pflags |= ATA_PFLAG_PIO32 | ATA_PFLAG_PIO32CHANGE;
    if (p_ap->ioaddr.bmdma_addr)
        return ata_port_start(p_ap);
    return 0;
}

void ata_sff_std_ports(struct ata_ioports *p_ioaddr)
{
    p_ioaddr->data_addr = p_ioaddr->cmd_addr + ATA_REG_DATA;
    p_ioaddr->error_addr = p_ioaddr->cmd_addr + ATA_REG_ERR;
    p_ioaddr->feature_addr = p_ioaddr->cmd_addr + ATA_REG_FEATURE;
    p_ioaddr->nsect_addr = p_ioaddr->cmd_addr + ATA_REG_NSECT;
    p_ioaddr->lbal_addr = p_ioaddr->cmd_addr + ATA_REG_LBAL;
    p_ioaddr->lbam_addr = p_ioaddr->cmd_addr + ATA_REG_LBAM;
    p_ioaddr->lbah_addr = p_ioaddr->cmd_addr + ATA_REG_LBAH;
    p_ioaddr->device_addr = p_ioaddr->cmd_addr + ATA_REG_DEVICE;
    p_ioaddr->status_addr = p_ioaddr->cmd_addr + ATA_REG_STATUS;
    p_ioaddr->command_addr = p_ioaddr->cmd_addr + ATA_REG_CMD;
}

void ata_bmdma_setup(struct ata_port *p_ap)
{
    unsigned int rw = (p_ap->p_tf->flags & ATA_TFLAG_WRITE);
    u8 dmactl;

    if(p_ap&&p_ap->p_tf) {
        rw = p_ap->p_tf->flags & ATA_TFLAG_WRITE;
    }
    /* load PRD table addr. */
    iowrite32(p_ap->prd_dma, p_ap->ioaddr.bmdma_addr + ATA_DMA_TABLE_OFS);

    /* specify data direction, triple-check start bit is clear */
    dmactl = ioread8(p_ap->ioaddr.bmdma_addr + ATA_DMA_CMD);
    dmactl &= ~(ATA_DMA_WR | ATA_DMA_START);
    if (!rw)
        dmactl |= ATA_DMA_WR;
    iowrite8(dmactl, p_ap->ioaddr.bmdma_addr + ATA_DMA_CMD);

    /* issue r/w command */
   ata_tf_to_host(p_ap);
}

void ata_bmdma_start(struct ata_port *p_ap)
{
    u8 dmactl;

    /* start host DMA transaction */
    dmactl = ioread8(p_ap->ioaddr.bmdma_addr + ATA_DMA_CMD);
    iowrite8(dmactl | ATA_DMA_START, p_ap->ioaddr.bmdma_addr + ATA_DMA_CMD);
}

void ata_bmdma_stop(struct ata_port *p_ap)
{
    void  *mmio = p_ap->ioaddr.bmdma_addr;

    /* clear start/stop bit */
    iowrite8(ioread8(mmio + ATA_DMA_CMD) & ~ATA_DMA_START,
         mmio + ATA_DMA_CMD);

    /* one-PIO-cycle guaranteed wait, per spec, for HDMA1:0 transition */
    ata_sff_dma_pause(p_ap);
}

u8 ata_bmdma_status(struct ata_port *ap)
{
    return ioread8(ap->ioaddr.bmdma_addr + ATA_DMA_STATUS);
}



