/* SPDX-License-Identifier: WTFPL */

#ifndef _BONITO_3AVIRT_QRMU_FW_CFG_H
#define _BONITO_3AVIRT_QRMU_FW_CFG_H

#include <linux/types.h>
#include <byteorder.h>

#define FW_CFG_REG_CTL (*(volatile u16 *)(0xb0080100 + 0x8))
#define FW_CFG_REG_DATA_16 (*(volatile u16 *)(0xb0080100))
#define FW_CFG_REG_DATA_32 (*(volatile u32 *)(0xb0080100))

#define FW_CFG_RAM_SIZE        0x03
#define FW_CFG_NB_CPUS         0x05

#define FW_CFG_ARCH_LOCAL      0x8000

#define FW_CFG_MACHINE_VERSION  (FW_CFG_ARCH_LOCAL + 0)
#define FW_CFG_CPU_FREQ         (FW_CFG_ARCH_LOCAL + 1)

static inline u16 fw_cfg_get_i16(u16 ctl)
{
    FW_CFG_REG_CTL = cpu_to_be16(ctl);
    return FW_CFG_REG_DATA_16;
}

static inline u32 fw_cfg_get_i32(u16 ctl)
{
    FW_CFG_REG_CTL = cpu_to_be16(ctl);
    return FW_CFG_REG_DATA_32;
}

static inline u32 fw_cfg_ram_size(void)
{
    return fw_cfg_get_i32(FW_CFG_RAM_SIZE);
}

static inline u16 fw_cfg_nb_cpus(void)
{
    return fw_cfg_get_i16(FW_CFG_NB_CPUS);
}

static inline u32 fw_cfg_machine_ver(void)
{
    return fw_cfg_get_i32(FW_CFG_MACHINE_VERSION);
}

static inline u32 fw_cfg_cpu_freq(void)
{
    return fw_cfg_get_i32(FW_CFG_CPU_FREQ);
}

#endif
