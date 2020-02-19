/*******************************************************************************

  Intel(R) Gigabit Ethernet Linux driver
  Copyright(c) 2007-2014 Intel Corporation.

  This program is free software; you can redistribute it and/or modify it
  under the terms and conditions of the GNU General Public License,
  version 2, as published by the Free Software Foundation.

  This program is distributed in the hope it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
  more details.

  The full GNU General Public License is included in this distribution in
  the file called "COPYING".

  Contact Information:
  Linux NICS <linux.nics@intel.com>
  e1000-devel Mailing List <e1000-devel@lists.sourceforge.net>
  Intel Corporation, 5200 N.E. Elam Young Parkway, Hillsboro, OR 97124-6497

*******************************************************************************/

/* glue for the OS independent part of e1000
 * includes register access macros
 */

#ifndef _E1000_OSDEP_H_
#define _E1000_OSDEP_H_
#ifdef PMON
#include <pmon.h>
#include <machine/cpu.h>
#define udelay(us) delay(us)
typedef unsigned int bool;
typedef unsigned long long  __u64;
//typedef unsigned long long  u64;
typedef unsigned int __u32;
typedef unsigned short __u16;
typedef signed int __s32;
typedef signed short __s16;
typedef long long __s64;

typedef __u16 __le16; 
typedef __u16 __be16;
typedef __u32 __le32;
typedef __u32 __be32;

typedef __u64 __le64;
typedef __u64 __be64;
#define CONFIG_IGB_DISABLE_PACKET_SPLIT
#define IGB_NO_LRO
//#define DEBUG
//#define DEBUG_FUNC
#include <linux/types.h>
#define DEFINE_DMA_UNMAP_ADDR(ADDR_NAME)	dma_addr_t ADDR_NAME
#define DEFINE_DMA_UNMAP_LEN(LEN_NAME)		unsigned int LEN_NAME
struct  rcu_head
{
};

#define ____cacheline_internodealigned_in_smp

#define __init
#define __iomem
#define true 1
#define false 0
#define ETH_ALEN    6       /* Octets in one ethernet addr   */
#define ETH_HLEN    14      /* Total octets in header.   */
#define ETH_ZLEN    60      /* Min. octets in frame sans FCS */
#define ETH_DATA_LEN    1500        /* Max. octets in payload    */
#define ETH_FRAME_LEN   1514        /* Max. octets in frame sans FCS */
#define ETH_FCS_LEN 4  
#define NULL 0
#define unlikely(x) (x)
#define likely(x) (x)

#ifndef ACCESS_ONCE
#define ACCESS_ONCE(x) (*(volatile typeof(x) *)&(x))
#endif
#define readb(addr)     (*(volatile unsigned char *)(addr))
#define readw(addr)     (*(volatile unsigned short *)(addr))
#define readl(addr)     (*(volatile unsigned int *)(addr))

#define __raw_readb(addr)   (*(volatile unsigned char *)(addr))
#define __raw_readw(addr)   (*(volatile unsigned short *)(addr))
#define __raw_readl(addr)   (*(volatile unsigned int *)(addr))

#define writeb(b,addr) ((*(volatile unsigned char *)(addr)) = (b))
#define writew(w,addr) ((*(volatile unsigned short *)(addr)) = (w))
#define writel(l,addr) ((*(volatile unsigned int *)(addr)) = (l))

#define __raw_writeb(b,addr)    ((*(volatile unsigned char *)(addr)) = (b))
#define __raw_writew(w,addr)    ((*(volatile unsigned short *)(addr)) = (w))
#define __raw_writel(l,addr)    ((*(volatile unsigned int *)(addr)) = (l))
#define usec_delay(x) delay(x)
#define usec_delay_irq(x) delay(x)
#define msec_delay(x) delay(x*1000)

/* Some workarounds require millisecond delays and are run during interrupt
 * context.  Most notably, when establishing link, the phy may need tweaking
 * but cannot process phy register reads/writes faster than millisecond
 * intervals...and we establish link due to a "link status change" interrupt.
 */
#define msec_delay_irq(x) delay(x*1000)
#define le16_to_cpus(x) do {} while (0)
#define cpu_to_le16s(x) do {} while (0)
#else

#include <linux/pci.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/if_ether.h>
#include <linux/sched.h>
#include "kcompat.h"
#define usec_delay(x) udelay(x)
#define usec_delay_irq(x) udelay(x)
#ifndef msec_delay
#define msec_delay(x) do { \
	/* Don't mdelay in interrupt context! */ \
	if (in_interrupt()) \
		BUG(); \
	else \
		msleep(x); \
} while (0)

/* Some workarounds require millisecond delays and are run during interrupt
 * context.  Most notably, when establishing link, the phy may need tweaking
 * but cannot process phy register reads/writes faster than millisecond
 * intervals...and we establish link due to a "link status change" interrupt.
 */
#define msec_delay_irq(x) mdelay(x)

#endif

#endif
#define E1000_READ_REG(x, y) e1000_read_reg(x, y)

#define PCI_COMMAND_REGISTER   PCI_COMMAND
#define CMD_MEM_WRT_INVALIDATE PCI_COMMAND_INVALIDATE
#define ETH_ADDR_LEN           ETH_ALEN

#ifdef __BIG_ENDIAN
#define E1000_BIG_ENDIAN __BIG_ENDIAN
#endif


#ifdef DEBUG
#define DEBUGOUT(S) do { printf("%s:%d, ", __FUNCTION__, __LINE__); printf(S); } while(0)
#define DEBUGOUT1(S, A...) printf(S, ## A)
#else
#define DEBUGOUT(S)
#define DEBUGOUT1(S, A...)
#endif

#ifdef DEBUG_FUNC
#define DEBUGFUNC(F) DEBUGOUT(F "\n")
#else
#define DEBUGFUNC(F)
#endif
#define DEBUGOUT2 DEBUGOUT1
#define DEBUGOUT3 DEBUGOUT2
#define DEBUGOUT7 DEBUGOUT3

#define E1000_REGISTER(a, reg) reg

/* forward declaration */
struct e1000_hw;

/* write operations, indexed using DWORDS */
#define E1000_WRITE_REG(hw, reg, val) \
do { \
	u8 __iomem *hw_addr = ACCESS_ONCE((hw)->hw_addr); \
	if (!E1000_REMOVED(hw_addr)) \
		writel((val), &hw_addr[(reg)]); \
} while (0)

u32 e1000_read_reg(struct e1000_hw *hw, u32 reg);

#define E1000_WRITE_REG_ARRAY(hw, reg, idx, val) \
	E1000_WRITE_REG((hw), (reg) + ((idx) << 2), (val))

#define E1000_READ_REG_ARRAY(hw, reg, idx) ( \
		e1000_read_reg((hw), (reg) + ((idx) << 2)))

#define E1000_READ_REG_ARRAY_DWORD E1000_READ_REG_ARRAY
#define E1000_WRITE_REG_ARRAY_DWORD E1000_WRITE_REG_ARRAY

#define E1000_WRITE_REG_ARRAY_WORD(a, reg, offset, value) ( \
	writew((value), ((a)->hw_addr + E1000_REGISTER(a, reg) + \
	((offset) << 1))))

#define E1000_READ_REG_ARRAY_WORD(a, reg, offset) ( \
	readw((a)->hw_addr + E1000_REGISTER(a, reg) + ((offset) << 1)))

#define E1000_WRITE_REG_ARRAY_BYTE(a, reg, offset, value) ( \
	writeb((value), ((a)->hw_addr + E1000_REGISTER(a, reg) + (offset))))

#define E1000_READ_REG_ARRAY_BYTE(a, reg, offset) ( \
	readb((a)->hw_addr + E1000_REGISTER(a, reg) + (offset)))

#define E1000_WRITE_REG_IO(a, reg, offset) do { \
	outl(reg, ((a)->io_base));                  \
	outl(offset, ((a)->io_base + 4)); \
	} while (0)

#define E1000_WRITE_FLUSH(a) E1000_READ_REG(a, E1000_STATUS)

#define E1000_WRITE_FLASH_REG(a, reg, value) ( \
	writel((value), ((a)->flash_address + reg)))

#define E1000_WRITE_FLASH_REG16(a, reg, value) ( \
	writew((value), ((a)->flash_address + reg)))

#define E1000_READ_FLASH_REG(a, reg) (readl((a)->flash_address + reg))

#define E1000_READ_FLASH_REG16(a, reg) (readw((a)->flash_address + reg))

#define E1000_REMOVED(h) unlikely(!(h))

#endif /* _E1000_OSDEP_H_ */
