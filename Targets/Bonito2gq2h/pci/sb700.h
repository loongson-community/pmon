#ifndef _SB700_H_
#define _SB700_H_

#include "sb700_chip.h"

//#define NULL (void*)0

typedef unsigned long device_t;

#if 1
#define INB(addr) (*(volatile unsigned char *) (addr))
#define INW(addr) (*(volatile unsigned short *) (addr))
#define INL(addr) (*(volatile unsigned int *) (addr))

#define OUTB(b,addr) (*(volatile unsigned char *) (addr) = (b))
#define OUTW(b,addr) (*(volatile unsigned short *) (addr) = (b))
#define OUTL(b,addr) (*(volatile unsigned int *) (addr) = (b))

#define WRITEB(val, addr) (*(volatile u8*)(addr) = (val))
#define WRITEW(val, addr) (*(volatile u16*)(addr) = (val))
#define WRITEL(val, addr) (*(volatile u32*)(addr) = (val))
#define READB(addr) (*(volatile u8*)(addr))
#define READW(addr) (*(volatile u16*)(addr))
#define READL(addr) (*(volatile u32*)(addr))

#endif

//typedef unsigned long pcitag_t;

extern device_t _pci_make_tag(int, int, int);
extern void     _pci_break_tag(device_t, int *, int *, int *);

extern void pm_iowrite(u8 reg, u8 value);
extern u8 pm_ioread(u8 reg);
extern void pm2_iowrite(u8 reg, u8 value);
extern u8 pm2_ioread(u8 reg);
extern void set_sm_enable_bits(device_t sm_dev, u32 reg_pos, u32 mask, u32 val);

extern void sb700_enable();


#endif
