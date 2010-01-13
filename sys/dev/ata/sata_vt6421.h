/*
 * Copyright (C) Excito Elektronik i Skåne AB, All rights reserved.
 * Author: Tor Krill <tor@excito.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */
/************************************************************************

 Copyright (C)
 File name:     sata_sil3114.h
 Author:  qianyl      Version:  ***      Date: ***
 Description:   
 Others:        
 Function List:
 
 Revision History:
 
 ---------------------------------------------------------------------------------------------
  Date          Author          Activity ID     Activity Headline
  2008-05-12    QianYuli        PMON00000001    porting it from u-boot
  2009-12-24    QianYuli        PMON00001224    porting it from sata_sil3114.h
*************************************************************************/
#ifndef SATA_VT6421_H
#define SATA_VT6421_H
#include "sata.h"

#define CONFIG_SYS_SATA_MAXPORTS 0x2

/* Missing ata defines */
#define ATA_CMD_STANDBY         0xE2
#define ATA_CMD_STANDBYNOW1     0xE0
#define ATA_CMD_IDLE            0xE3
#define ATA_CMD_IDLEIMMEDIATE   0xE1

/* Defines for SIL3114 chip */

/* PCI defines */
#define VT_VEND_ID     0x1106
#define VT6421_DEVICE_ID   0x3249

/* some vendor specific registers */
#define VND_SYSCONFSTAT 0x88    /* System Configuration Status and Command */
#define VND_SYSCONFSTAT_CHN_0_INTBLOCK (1<<22)
#define VND_SYSCONFSTAT_CHN_1_INTBLOCK (1<<23)
#define VND_SYSCONFSTAT_CHN_2_INTBLOCK (1<<24)
#define VND_SYSCONFSTAT_CHN_3_INTBLOCK (1<<25)

/* internal registers mapped by BAR5 */
/* SATA Control*/
#define VND_SCONTROL_CH0    0x100
#define VND_SCONTROL_CH1    0x180
#define VND_SCONTROL_CH2    0x300
#define VND_SCONTROL_CH3    0x380

#define SATA_SC_IPM_T2P     (1<<16)
#define SATA_SC_IPM_T2S     (2<<16)
#define SATA_SC_SPD_1_5     (1<<4)
#define SATA_SC_SPD_3_0     (2<<4)
#define SATA_SC_DET_RST     (1) /* ATA Reset sequence */
#define SATA_SC_DET_PDIS    (4) /* PHY Disable */

/* SATA Status */
#define VND_SSTATUS_CH0     0x104
#define VND_SSTATUS_CH1     0x184
#define VND_SSTATUS_CH2     0x304
#define VND_SSTATUS_CH3     0x384

#define SATA_SS_IPM_ACTIVE  (1<<8)
#define SATA_SS_IPM_PARTIAL (2<<8)
#define SATA_SS_IPM_SLUMBER (6<<8)
#define SATA_SS_SPD_1_5     (1<<4)
#define SATA_SS_SPD_3_0     (2<<4)
#define SATA_DET_P_NOPHY    (1) /* Device presence but no PHY connection established */
#define SATA_DET_PRES       (3) /* Device presence and active PHY */
#define SATA_DET_OFFLINE    (4) /* Device offline or in loopback mode */

/* Task file registers in BAR5 mapping */
#define VND_TF0_CH0         0x80
#define VND_TF0_CH1         0xc0
#define VND_TF0_CH2         0x280
#define VND_TF0_CH3         0x2c0
#define VND_TF1_CH0         0x88
#define VND_TF1_CH1         0xc8
#define VND_TF1_CH2         0x288
#define VND_TF1_CH3         0x2c8
#define VND_TF2_CH0         0x88
#define VND_TF2_CH1         0xc8
#define VND_TF2_CH2         0x288
#define VND_TF2_CH3         0x2c8

#define VND_BMDMA_CH0       0x00
#define VND_BMDMA_CH1       0x08
#define VND_BMDMA_CH2       0x200
#define VND_BMDMA_CH3       0x208
#define VND_BMDMA2_CH0      0x10
#define VND_BMDMA2_CH1      0x18
#define VND_BMDMA2_CH2      0x210
#define VND_BMDMA2_CH3      0x218

/* FIFO control */
#define VND_FIFOCFG_CH0     0x40
#define VND_FIFOCFG_CH1     0x44
#define VND_FIFOCFG_CH2     0x240
#define VND_FIFOCFG_CH3     0x244

/* Task File configuration and status */
#define VND_TF_CNST_CH0     0xa0
#define VND_TF_CNST_CH1     0xe0
#define VND_TF_CNST_CH2     0x2a0
#define VND_TF_CNST_CH3     0x2e0

#define VND_TF_CNST_BFCMD   (1<<1)
#define VND_TF_CNST_CHNRST  (1<<2)
#define VND_TF_CNST_VDMA    (1<<10)
#define VND_TF_CNST_INTST   (1<<11)
#define VND_TF_CNST_WDTO    (1<<12)
#define VND_TF_CNST_WDEN    (1<<13)
#define VND_TF_CNST_WDIEN   (1<<14)

/* for testing */
#define VND_SSDR            0x04c   /* System Software Data Register */
#define VND_FMACS           0x050   /* Flash Memory Address control and status */

#define SATA_HC_MAX_NUM     4 /* Max host controller numbers */
#define SATA_HC_MAX_CMD     16 /* Max command queue depth per host controller */
#define SATA_HC_MAX_PORT    16 /* Max port number per host controller */
#define PCI_IO_BASE         0xbfd00000

#define READ_CMD    0
#define WRITE_CMD   1
#define PCI_VENDOR_ID       0x00    /* 16 bits */
#define PCI_DEVICE_ID       0x02    /* 16 bits */
#define PCI_COMMAND     0x04    /* 16 bits */

#define PCI_CACHE_LINE_SIZE_INDEX   0x0c    /* 8 bits */
#define PCI_LATENCY_TIMER   0x0d    /* 8 bits */
#define PCI_HEADER_TYPE     0x0e    /* 8 bits */

#define PCI_BASE_ADDRESS_0  0x10    /* 32 bits */
#define PCI_BASE_ADDRESS_1  0x14    /* 32 bits [htype 0,1 only] */
#define PCI_BASE_ADDRESS_2  0x18    /* 32 bits [htype 0 only] */
#define PCI_BASE_ADDRESS_3  0x1c    /* 32 bits */
#define PCI_BASE_ADDRESS_4  0x20    /* 32 bits */
#define PCI_BASE_ADDRESS_5  0x24    /* 32 bits */

#define  PCI_COMMAND_IO     0x1 /* Enable response in I/O space */
#define  PCI_COMMAND_MEMORY 0x2 /* Enable response in Memory space */
#define  PCI_COMMAND_MASTER 0x4 /* Enable bus mastering */
#define  PCI_COMMAND_SPECIAL    0x8 /* Enable response to special cycles */
#define  PCI_COMMAND_INVALIDATE 0x10    /* Use memory write and invalidate */
#define  PCI_COMMAND_VGA_PALETTE 0x20   /* Enable palette snooping */
#define  PCI_COMMAND_PARITY 0x40    /* Enable parity checking */
#define  PCI_COMMAND_WAIT   0x80    /* Enable address/data stepping */
#define  PCI_COMMAND_SERR   0x100   /* Enable SERR */
#define  PCI_COMMAND_FAST_BACK  0x200   /* Enable back-to-back writes */

#define VT_PRD_BUFF_SIZE 0x10000   /* 64 KB*/ 

/*
 * SATA device driver struct
 * Here SATA device attached to BUS by Sil3114-Sata-Adapter
 */
 
typedef struct vt_sata {
    struct wdc_softc    sc_wdcdev;  /* common wdc definitions */
    struct channel_softc *wdc_chanarray[1];
    struct channel_softc wdc_channel; /* generic part */
    char        name[12];
    u32     reg_base;       /* the base address of controller register */
} vt_sata_t;

struct vt_sata_softc {
    /* General disk infos */
    struct device sc_dev;
    //int sata_disk_exsit;
    int bs,count;
};
#endif
