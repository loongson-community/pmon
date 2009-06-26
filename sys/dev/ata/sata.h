/************************************************************************
 *

 Copyright (C)
 File name:     sata.h
 Author:  qianyl      Version:  ***      Date: ***
 Description:   
 Others:        
 Function List:
 
 Revision History:
 
 ----------------------------------------------------------------------------------------------------------
  Date              Author           Activity ID             Activity Headline
  2008-05-12    QianYuli        PMON00000001    Creat it
  2008-06-26    QianYuli        PMON00000002    Remove some sata*** like struct
***********************************************************************************/
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

typedef struct sata_operation {
    int (*open) (dev_t dev, int flag, int fmt, struct proc *p);
    int (*read) (dev_t dev, struct uio *uio, int flag);
    int (*write) (dev_t dev,struct uio *uio, int flag);
    int (*close)(dev_t dev, int flag, int fmt, struct proc *p);
} sata_operation_t;
#endif
