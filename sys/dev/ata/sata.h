/************************************************************************

 Copyright (C)
 File name:     sata.h
 Author:  qianyl      Version:  ***      Date: ***
 Description:   
 Others:        
 Function List:
 
 Revision History:
 
 --------------------------------------------------------------------------
  Date          Author          Activity ID     Activity Headline
  2008-05-12    QianYuli        PMON00000001    Creat it
*************************************************************************/
#ifndef SATA_H
#define SATA_H
#include <sys/param.h>
#include <ctype.h>
#include <sys/device.h>
#include <machine/intr.h>
#include <machine/bus.h>
#include <dev/ata/atavar.h>
#include <dev/ata/atareg.h>
#include <dev/ic/wdcreg.h>
#include <dev/ic/wdcvar.h>
#include <linux/libata.h>
#include <part.h>
#include <pmon.h>

#define PCI_VENDOR_SATA_ATP8620  0x1191
#define PCI_DEVICE_SATA_ATP8620  0x000d

#define PCI_VENDOR_SATA_SIL3114  0x1095
#define PCI_DEVICE_SATA_SIL3114  0x3114

#define CONFIG_SYS_SATA_MAXBUS         2       /*Max Sata buses supported */
#define CONFIG_SYS_SATA_DEVS_PER_BUS   2      /*Max no. of devices per bus/port */
#define CONFIG_SYS_SATA_MAX_DEVICE     (CONFIG_SYS_SATA_MAXBUS* CONFIG_SYS_SATA_DEVS_PER_BUS)
#define CONFIG_ATA_PIIX     1       /*Supports ata_piix driver */

typedef struct sata_prd {
    unsigned int addr;
    unsigned int flag_len;
}sata_prd_t, *p_sata_prd_t;

typedef struct pci_sata_adapter_id {
    unsigned int vendor_id;
    unsigned int device_id;
} pci_sata_adapter_id_t;

typedef struct sata_operation {
    int (*open) (dev_t dev, int flag, int fmt, struct proc *p);
    int (*read) (dev_t dev, struct uio *uio, int flag);
    int (*write) (dev_t dev,struct uio *uio, int flag);
    int (*close)(dev_t dev, int flag, int fmt, struct proc *p);
} sata_operation_t;


typedef struct sata_ioports {
    unsigned long cmd_addr;
    unsigned long data_addr;
    unsigned long error_addr;
    unsigned long feature_addr;
    unsigned long nsect_addr;
    unsigned long lbal_addr;
    unsigned long lbam_addr;
    unsigned long lbah_addr;
    unsigned long device_addr;
    unsigned long status_addr;
    unsigned long command_addr;
    unsigned long altstatus_addr;
    unsigned long ctl_addr;
    unsigned long bmdma_addr;
    unsigned long scr_addr;
} sata_ioports_t;

typedef struct sata_port {
    unsigned char port_no;  /* primary=0, secondary=1       */
    struct sata_ioports ioaddr; /* ATA cmd/ctl/dma reg blks     */
    unsigned int p_prd_table;
    unsigned char ctl_reg;
    unsigned char last_ctl;
    unsigned char port_state;   /* 1-port is available and      */
    /* 0-port is not available      */
    unsigned char dev_mask;
}sata_port_t;

typedef struct sata_device {
    struct device r_dev;
    block_dev_desc_t stor_desc;
    unsigned char port_no; /*which port the device connect to*/
    unsigned char device_no;/*tht number assigned to device*/
} sata_device_t;
#endif
