#ifndef __RS780_CMN_H__
#define __RS780_CMN_H__

#include "rs780.h"

#define pci_read_config32(t, r)   _pci_conf_read32((t), (r))
#define pci_read_config16(t, r)   _pci_conf_read16((t), (r))
#define pci_read_config8(t, r)    _pci_conf_read8((t), (r))

#define pci_write_config32(t, r, d)  _pci_conf_write32((t), (r), (d))
#define pci_write_config16(t, r, d)  _pci_conf_write16((t), (r), (d))
#define pci_write_config8(t, r, d)   _pci_conf_write8((t), (r), (d))

extern void delay(int usec);

#define udelay(m) delay(m)

u32 pci_read_type0_config32(u32 dev, u32 func, u32 reg);
void pci_write_type0_config32(u32 dev, u32 func, u32 reg, u32 val);
u32 pci_read_type1_config32(u32 bus, u32 dev, u32 func, u32 reg);
void pci_write_type1_config32(u32 bus, u32 dev, u32 func, u32 reg, u32 val);

u32 _pci_conf_read(device_t tag,int reg);
u32 _pci_conf_read32(device_t tag,int reg);
u16 _pci_conf_read16(device_t tag,int reg);
u8 _pci_conf_read8(device_t tag,int reg);
u32 _pci_conf_readn(device_t tag, int reg, int width);

void _pci_conf_write(device_t tag, int reg, u32 data);
void _pci_conf_write32(device_t tag, int reg, u32 data);
void _pci_conf_write16(device_t tag, int reg, u16 data);
void _pci_conf_write8(device_t tag, int reg, u8 data);
void _pci_conf_writen(device_t tag, int reg, u32 data,int width);



extern int printf (const char *fmt, ...);

#define printk_emerg(fmt, arg...)   printf(fmt, ##arg)
#define printk_alert(fmt, arg...)   printf(fmt, ##arg)
#define printk_crit(fmt, arg...)    printf(fmt, ##arg)
#define printk_err(fmt, arg...)     printf(fmt, ##arg)
#define printk_warning(fmt, arg...) printf(fmt, ##arg)
#define printk_notice(fmt, arg...)  printf(fmt, ##arg)
#define printk_info(fmt, arg...)    printf(fmt, ##arg)
#define printk_debug(fmt, arg...)   printf(fmt, ##arg)
#define printk_spew(fmt, arg...)    printf(fmt, ##arg)
#define printk(fmt, arg...)         printf(fmt, ##arg)

#endif
	
