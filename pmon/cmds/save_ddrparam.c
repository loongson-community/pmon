/*    $Id: save_ddrparam.c,v 1.1.1.1 2006/09/14 01:59:08 root Exp $ */

/*
 * Copyright (c) 2001 Opsycon AB  (www.opsycon.se)
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *    This product includes software developed by Opsycon AB, Sweden.
 * 4. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/device.h>
#include <sys/queue.h>

#include <pmon.h>

int cmd_save_ddrparam __P((int, char *[]));


#ifdef ARB_LEVEL

#define u64 unsigned long long
extern u64 __raw__readq(u64 addr);
extern u64 __raw__writeq(u64 addr, u64 val);

extern char _start;

#ifdef LOONGSON_2H
extern char c0_mc0_level_info;
#endif
#ifdef loongson3A3
extern char c0_mc0_level_info, c0_mc1_level_info;
#ifdef MULTI_CHIP
extern char c1_mc0_level_info, c1_mc1_level_info;
#endif
#endif
#ifdef LS3B
extern char c0_mc0_level_info;
#ifdef MULTI_CHIP
extern char c0_mc1_level_info;
#endif
#ifdef DUAL_3B
extern char c1_mc0_level_info;
extern char c1_mc1_level_info;
#endif
#endif

//#define DEBUG
#ifdef DEBUG
extern int do_cmd(char *);
#endif

#define LSMCD3_2
#ifdef  LSMCD3_2
#define DDR_PARAM_NUM     180
#else
#define DDR_PARAM_NUM     152
#endif
#define MC_CONFIG_ADDR 0x900000000ff00000ull

#ifdef  LOONGSON_2H
#define CPU_CONFIG_ADDR 0x900000001fd00200ull
#else
#define CPU_CONFIG_ADDR 0x900000001fe00180ull
#define CPU_L2XBAR_CONFIG_ADDR 0x900000003ff00000ull
#endif

void enable_ddrconfig(u64 node_id_shift44)
{
    unsigned long long val;

#ifdef LOONGSON_2K
	*(volatile int *)0xbfe10424 &= ~0x100;
#elif defined(loongson3A3)
    val = __raw__readq(CPU_CONFIG_ADDR | node_id_shift44);
#ifdef LSMC_2
    val &=0xfffffffffffffdefull;
#else
    val &=0xfffffffffffffeffull;
#endif
    __raw__writeq(CPU_CONFIG_ADDR | node_id_shift44, val);
#ifdef DEBUG
    printf("Enable sys config reg = %016llx", __raw__readq(CPU_CONFIG_ADDR | node_id_shift44));
#endif
    return;
#endif

#ifdef LS3B
    val = __raw__readq(CPU_CONFIG_ADDR | node_id_shift44 & 0xffffefffffffffffull);
    val &=0xfffffffffffffdefull;
    __raw__writeq(CPU_CONFIG_ADDR | node_id_shift44 & 0xffffefffffffffffull, val);
#ifdef DEBUG
    printf("Enable sys config reg = %016llx", __raw__readq(CPU_CONFIG_ADDR | node_id_shift44 & 0xffffefffffffffffull));
#endif
    return;
#endif

#ifdef  LOONGSON_2H
    val = __raw__readq(CPU_CONFIG_ADDR | node_id_shift44);
    val &=0xffffffffffffdfffull;
    __raw__writeq(CPU_CONFIG_ADDR | node_id_shift44, val);
#ifdef DEBUG
    printf("Enable sys config reg = %016llx", __raw__readq(CPU_CONFIG_ADDR | node_id_shift44 & 0xffffefffffffffffull));
#endif
    return;
#endif
}

void disable_ddrconfig(u64 node_id_shift44)
{
    unsigned long long val;

    /* Disable DDR access configure register */
#ifdef LOONGSON_2K
	*(volatile int *)0xbfe10424 |= 0x100;
#elif defined(loongson3A3)
    val = __raw__readq(CPU_CONFIG_ADDR | node_id_shift44);
#ifdef LSMC_2
    val |= 0x210;
#else
    val |= 0x100;
#endif
    __raw__writeq(CPU_CONFIG_ADDR | node_id_shift44, val);
#ifdef DEBUG
    printf("Disable sys config reg = %016llx\n", __raw__readq(CPU_CONFIG_ADDR | node_id_shift44));
#endif
    return;
#endif
#ifdef LS3B
    val = __raw__readq(CPU_CONFIG_ADDR | node_id_shift44 & 0xffffefffffffffffull);
    val |= 0x210;
    __raw__writeq(CPU_CONFIG_ADDR | node_id_shift44 & 0xffffefffffffffffull, val);
#ifdef DEBUG
    printf("Disable sys config reg = %016llx\n", __raw__readq(CPU_CONFIG_ADDR | node_id_shift44 & 0xffffefffffffffffull));
#endif
    return;
#endif

#ifdef  LOONGSON_2H
    val = __raw__readq(CPU_CONFIG_ADDR | node_id_shift44);
    val |= 0x2000ull;
    __raw__writeq(CPU_CONFIG_ADDR | node_id_shift44, val);
#ifdef DEBUG
    printf("Disable sys config reg = %016llx\n", __raw__readq(CPU_CONFIG_ADDR | node_id_shift44));
#endif
    return;
#endif
}


#ifdef loongson3A3
void enable_ddrcfgwindow(u64 node_id_shift44, int mc_selector, unsigned long long * buf)
    //use window 0 which is used to route 0x1fc00000 addr space, not used here.
{

    //printf("Now enable ddr config windows.\n");
#ifdef DEBUG
    printf("origin :: 0x00: %016llx  0x40: %016llx 0x80:  %016llx\n", __raw__readq((CPU_L2XBAR_CONFIG_ADDR | node_id_shift44)), __raw__readq((CPU_L2XBAR_CONFIG_ADDR | node_id_shift44) + 0x40), __raw__readq((CPU_L2XBAR_CONFIG_ADDR | node_id_shift44) + 0x80));
#endif
    buf[0] =  __raw__readq((CPU_L2XBAR_CONFIG_ADDR | node_id_shift44));
    buf[1] =  __raw__readq((CPU_L2XBAR_CONFIG_ADDR | node_id_shift44) + 0x40);
    buf[2] =  __raw__readq((CPU_L2XBAR_CONFIG_ADDR | node_id_shift44) + 0x80);

    __raw__writeq((CPU_L2XBAR_CONFIG_ADDR | node_id_shift44), (0x000000000ff00000ull + node_id_shift44));
    __raw__writeq((CPU_L2XBAR_CONFIG_ADDR | node_id_shift44) + 0x40, 0xfffffffffff00000ull);
    __raw__writeq((CPU_L2XBAR_CONFIG_ADDR | node_id_shift44) + 0x80, (0x000000000ff000f0ull + mc_selector));

#ifdef DEBUG
    printf("new  :: 0x00: %016llx  0x40: %016llx 0x80:  %016llx\n", __raw__readq((CPU_L2XBAR_CONFIG_ADDR | node_id_shift44)), __raw__readq((CPU_L2XBAR_CONFIG_ADDR | node_id_shift44) + 0x40), __raw__readq((CPU_L2XBAR_CONFIG_ADDR | node_id_shift44) + 0x80));

    printf("Now save L2X bar windows\n");
    printf("buf[0] = %llx, buf[1] = %llx, buf[2] = %llx\n", buf[0], buf[1], buf[2]);
#endif
}

//restore origin configure
void disable_ddrcfgwidow(u64 node_id_shift44, int mc_selector, unsigned long long * buf)
{
    //printf("Now disable ddr config windows.\n");
#ifdef DEBUG
    printf("Now retore L2X bar windows\n");
    printf("buf[0] = %llx, buf[1] = %llx, buf[2] = %llx\n", buf[0], buf[1], buf[2]);
#endif

    __raw__writeq((CPU_L2XBAR_CONFIG_ADDR | node_id_shift44) + 0x00, buf[0] );
    __raw__writeq((CPU_L2XBAR_CONFIG_ADDR | node_id_shift44) + 0x40, buf[1] );
    __raw__writeq((CPU_L2XBAR_CONFIG_ADDR | node_id_shift44) + 0x80, buf[2] );

#ifdef DEBUG
    printf("new  :: 0x00: %016llx  0x40: %016llx 0x80:  %016llx\n", __raw__readq((CPU_L2XBAR_CONFIG_ADDR | node_id_shift44)), __raw__readq((CPU_L2XBAR_CONFIG_ADDR | node_id_shift44) + 0x40), __raw__readq((CPU_L2XBAR_CONFIG_ADDR | node_id_shift44) + 0x80));
#endif
}
#endif

#define MC0 0x0
#define MC1 0x1

int read_ddr_param(u64 node_id_shift44, int mc_selector,  unsigned long long * base)
{
    int i;
    unsigned long long * val = base;
    unsigned long long buf[3];

#if defined(loongson3A3) && !defined(LOONGSON_2K)
    // step 1. Change The Primest window for MC0 or MC1 register space
    enable_ddrcfgwindow(node_id_shift44, mc_selector, buf);
#endif

    // step 2. Enabel access to MC0 or MC1 register space
    enable_ddrconfig(node_id_shift44);

    // step 3. Read out ddr config register to buffer
    printf("Now Read out DDR parameter from DDR MC%d controler after DDR training\n", mc_selector);
    for ( i = DDR_PARAM_NUM - 1; i >= 0; i--) // NOTICE HERE: it means system has DDR_PARAM_NUM double words
    {
#ifdef LSMC_2
        val[i] =  __raw__readq((MC_CONFIG_ADDR | node_id_shift44) + (0x8 * i));
#else
        val[i] =  __raw__readq((MC_CONFIG_ADDR | node_id_shift44) + (0x10 * i));
#endif

#ifdef DEBUG
        printf("< CFGREG >:val[%03d]  = %016llx \n", i, val[i]);
#endif
    }
    //clear param_start
#ifdef LSMC_2
    val[3]  &=  0xfffffffffffffffeull;
#else
    val[3]  &=  0xfffffeffffffffffull;
#endif

    // step 4. Disabel access to MC0 or MC1 register space
    disable_ddrconfig(node_id_shift44);

#if defined(loongson3A3) && !defined(LOONGSON_2K)
    // step 5. Restore The Primest window for accessing system memory
    disable_ddrcfgwidow(node_id_shift44, mc_selector, buf);
#endif

    //printf("Read out DDR MC%d config Done.\n", mc_selector);
    return 0;
}

void __attribute__((noinline)) save_ddrparam(u64 node_id_shift44, u64 *ddr_param_buf, int param_store_addr, int mc_selector)
{
#ifdef DEBUG
    int i;
    u64 tmp;
#endif

#ifdef DEBUG
    printf("\nnode_id_shift44=0x%016llx\n", node_id_shift44);
#endif
    // step 1.1 Read out DDR controler register from MC and save them in buffer
    read_ddr_param(node_id_shift44, mc_selector, &ddr_param_buf[6]);

    // step 1.2 Program buffers of MC0 register into FLASH
    tgt_flashprogram((int *)(0xbfc00000+(param_store_addr -(int)&_start)), (DDR_PARAM_NUM + 6) * 8, ddr_param_buf, TRUE);

#ifdef DEBUG
    for(i = 0; i < DDR_PARAM_NUM + 6; i++)
    {
        tmp =  __raw__readq(0x900000001fc00000ull + param_store_addr - (int)&_start + i * 8);
        if(ddr_param_buf[i] != tmp)
        {
            printf("\nMiscompare:i=%d, val=%016llx", i, tmp);
        }
        else
            printf("\nSame:i=%d, val=%016llx", i, tmp);
    }
#endif
}

#define DIMM_INFO_ADDR  0x980000000fff0000ull

#ifdef loongson3A3
int __attribute__((noinline)) save_board_ddrparam(int mandatory)
{
    unsigned long long flag;
    unsigned long long node_id;
    unsigned long long ddr_param_buf[DDR_PARAM_NUM + 6];
    if( (__raw__readq(DIMM_INFO_ADDR) == 0x2013011014413291ull) || mandatory ){
        printf("Token is correct!\n");
        flag    = __raw__readq(DIMM_INFO_ADDR + 0x8);
        printf("flag is 0x%016llx\n", flag);
        node_id = 0;
        //MC0
        if( ((flag >> 32) & 0x1) || mandatory ){
            printf("Store MC info of Node %lld MC 0\n", node_id);
            ddr_param_buf[0] = 0x1;
            ddr_param_buf[1] = __raw__readq(DIMM_INFO_ADDR + 0x10);
            ddr_param_buf[2] = __raw__readq(DIMM_INFO_ADDR + 0x18);
            ddr_param_buf[3] = __raw__readq(DIMM_INFO_ADDR + 0x20);
            ddr_param_buf[4] = __raw__readq(DIMM_INFO_ADDR + 0x28);
            ddr_param_buf[5] = __raw__readq(DIMM_INFO_ADDR + 0x90);
#ifdef DEBUG
            printf("mc level info is 0x%016llx\n", ddr_param_buf[0]);
            printf("dimm 0 info is 0x%016llx\n", ddr_param_buf[1]);
            printf("dimm 0 info is 0x%016llx\n", ddr_param_buf[2]);
            printf("dimm 1 info is 0x%016llx\n", ddr_param_buf[3]);
            printf("dimm 1 info is 0x%016llx\n", ddr_param_buf[4]);
            printf("clksel info is 0x%016llx\n", ddr_param_buf[5]);
#endif
            save_ddrparam(node_id << 44, &ddr_param_buf, (int)&c0_mc0_level_info, 0);
        }
        //MC1
        if( ((flag >> 33) & 0x1) || mandatory ){
            printf("Store MC info of Node %lld MC 1\n", node_id);
            ddr_param_buf[0] = 0x1;
            ddr_param_buf[1] = __raw__readq(DIMM_INFO_ADDR + 0x30);
            ddr_param_buf[2] = __raw__readq(DIMM_INFO_ADDR + 0x38);
            ddr_param_buf[3] = __raw__readq(DIMM_INFO_ADDR + 0x40);
            ddr_param_buf[4] = __raw__readq(DIMM_INFO_ADDR + 0x48);
            ddr_param_buf[5] = __raw__readq(DIMM_INFO_ADDR + 0x98);
#ifdef DEBUG
            printf("mc level info is 0x%016llx\n", ddr_param_buf[0]);
            printf("dimm 0 info is 0x%016llx\n", ddr_param_buf[1]);
            printf("dimm 0 info is 0x%016llx\n", ddr_param_buf[2]);
            printf("dimm 1 info is 0x%016llx\n", ddr_param_buf[3]);
            printf("dimm 1 info is 0x%016llx\n", ddr_param_buf[4]);
            printf("clksel info is 0x%016llx\n", ddr_param_buf[5]);
#endif
            save_ddrparam(node_id << 44, &ddr_param_buf, (int)&c0_mc1_level_info, 1);
        }
#ifdef MULTI_CHIP
        node_id = 1;
        //MC0
        if( ((flag >> 34) & 0x1) || mandatory ){
            printf("Store MC info of Node %lld MC 0\n", node_id);
            ddr_param_buf[0] = 0x1;
            ddr_param_buf[1] = __raw__readq(DIMM_INFO_ADDR + 0x50);
            ddr_param_buf[2] = __raw__readq(DIMM_INFO_ADDR + 0x58);
            ddr_param_buf[3] = __raw__readq(DIMM_INFO_ADDR + 0x60);
            ddr_param_buf[4] = __raw__readq(DIMM_INFO_ADDR + 0x68);
            ddr_param_buf[5] = __raw__readq(DIMM_INFO_ADDR + 0xa0);
#ifdef DEBUG
            printf("mc level info is 0x%016llx\n", ddr_param_buf[0]);
            printf("dimm 0 info is 0x%016llx\n", ddr_param_buf[1]);
            printf("dimm 0 info is 0x%016llx\n", ddr_param_buf[2]);
            printf("dimm 1 info is 0x%016llx\n", ddr_param_buf[3]);
            printf("dimm 1 info is 0x%016llx\n", ddr_param_buf[4]);
            printf("clksel info is 0x%016llx\n", ddr_param_buf[5]);
#endif
            save_ddrparam(node_id << 44, &ddr_param_buf, (int)&c1_mc0_level_info, 0);
        }
        //MC1
        if( ((flag >> 35) & 0x1) || mandatory ){
            printf("Store MC info of Node %lld MC 1\n", node_id);
            ddr_param_buf[0] = 0x1;
            ddr_param_buf[1] = __raw__readq(DIMM_INFO_ADDR + 0x70);
            ddr_param_buf[2] = __raw__readq(DIMM_INFO_ADDR + 0x78);
            ddr_param_buf[3] = __raw__readq(DIMM_INFO_ADDR + 0x80);
            ddr_param_buf[4] = __raw__readq(DIMM_INFO_ADDR + 0x88);
            ddr_param_buf[5] = __raw__readq(DIMM_INFO_ADDR + 0xa8);
#ifdef DEBUG
            printf("mc level info is 0x%016llx\n", ddr_param_buf[0]);
            printf("dimm 0 info is 0x%016llx\n", ddr_param_buf[1]);
            printf("dimm 0 info is 0x%016llx\n", ddr_param_buf[2]);
            printf("dimm 1 info is 0x%016llx\n", ddr_param_buf[3]);
            printf("dimm 1 info is 0x%016llx\n", ddr_param_buf[4]);
            printf("clksel info is 0x%016llx\n", ddr_param_buf[5]);
#endif
            save_ddrparam(node_id << 44, &ddr_param_buf, (int)&c1_mc1_level_info, 1);
        }
#endif
        return(1);
    }
    else{
        printf("Token is broken! Date is 0x%016llx\n", __raw__readq(DIMM_INFO_ADDR));
        return(0);
    }
}
#else
//for LS3B/LOONGSON_2H

int save_board_ddrparam(int mandatory)
{
    unsigned long long flag;
    unsigned long long node_id;
    unsigned long long ddr_param_buf[DDR_PARAM_NUM + 6];
    if( (__raw__readq(DIMM_INFO_ADDR) == 0x2013011014413291) || mandatory ){
        printf("Token is correct!\n");
        flag    = __raw__readq(DIMM_INFO_ADDR + 0x8);
        printf("flag is 0x%016llx\n", flag);
        //MC0
        node_id = 0;
        if( ((flag >> 32) & 0x1) || mandatory ){
            printf("Store MC info of Node %d\n", node_id);
            ddr_param_buf[0] = 0x1;
            ddr_param_buf[1] = __raw__readq(DIMM_INFO_ADDR + 0x10);
            ddr_param_buf[2] = __raw__readq(DIMM_INFO_ADDR + 0x18);
            ddr_param_buf[3] = __raw__readq(DIMM_INFO_ADDR + 0x20);
            ddr_param_buf[4] = __raw__readq(DIMM_INFO_ADDR + 0x28);
            ddr_param_buf[5] = __raw__readq(DIMM_INFO_ADDR + 0x90);
#ifdef DEBUG
            printf("mc level info is 0x%016llx\n", ddr_param_buf[0]);
            printf("dimm 0 info is 0x%016llx\n", ddr_param_buf[1]);
            printf("dimm 0 info is 0x%016llx\n", ddr_param_buf[2]);
            printf("dimm 1 info is 0x%016llx\n", ddr_param_buf[3]);
            printf("dimm 1 info is 0x%016llx\n", ddr_param_buf[4]);
            printf("clksel info is 0x%016llx\n", ddr_param_buf[5]);
#endif
            save_ddrparam(node_id << 44, &ddr_param_buf, (int)&c0_mc0_level_info, 0);
        }
#ifdef LS3B
#ifdef MULTI_CHIP
        //MC1
        node_id = 1;
        if( ((flag >> 33) & 0x1) || mandatory ){
            printf("Store MC info of Node %d\n", node_id);
            ddr_param_buf[0] = 0x1;
            ddr_param_buf[1] = __raw__readq(DIMM_INFO_ADDR + 0x30);
            ddr_param_buf[2] = __raw__readq(DIMM_INFO_ADDR + 0x38);
            ddr_param_buf[3] = __raw__readq(DIMM_INFO_ADDR + 0x40);
            ddr_param_buf[4] = __raw__readq(DIMM_INFO_ADDR + 0x48);
            ddr_param_buf[5] = __raw__readq(DIMM_INFO_ADDR + 0x98);
#ifdef DEBUG
            printf("mc level info is 0x%016llx\n", ddr_param_buf[0]);
            printf("dimm 0 info is 0x%016llx\n", ddr_param_buf[1]);
            printf("dimm 0 info is 0x%016llx\n", ddr_param_buf[2]);
            printf("dimm 1 info is 0x%016llx\n", ddr_param_buf[3]);
            printf("dimm 1 info is 0x%016llx\n", ddr_param_buf[4]);
            printf("clksel info is 0x%016llx\n", ddr_param_buf[5]);
#endif
            save_ddrparam(node_id << 44, &ddr_param_buf, (int)&c0_mc1_level_info, 0);
        }
#endif
#ifdef DUAL_3B
        //MC0
        node_id = 2;
        if( ((flag >> 34) & 0x1) || mandatory ){
            printf("Store MC info of Node %d\n", node_id);
            ddr_param_buf[0] = 0x1;
            ddr_param_buf[1] = __raw__readq(DIMM_INFO_ADDR + 0x50);
            ddr_param_buf[2] = __raw__readq(DIMM_INFO_ADDR + 0x58);
            ddr_param_buf[3] = __raw__readq(DIMM_INFO_ADDR + 0x60);
            ddr_param_buf[4] = __raw__readq(DIMM_INFO_ADDR + 0x68);
            ddr_param_buf[5] = __raw__readq(DIMM_INFO_ADDR + 0xa0);
#ifdef DEBUG
            printf("mc level info is 0x%016llx\n", ddr_param_buf[0]);
            printf("dimm 0 info is 0x%016llx\n", ddr_param_buf[1]);
            printf("dimm 0 info is 0x%016llx\n", ddr_param_buf[2]);
            printf("dimm 1 info is 0x%016llx\n", ddr_param_buf[3]);
            printf("dimm 1 info is 0x%016llx\n", ddr_param_buf[4]);
            printf("clksel info is 0x%016llx\n", ddr_param_buf[5]);
#endif
            save_ddrparam(node_id << 44, &ddr_param_buf, (int)&c1_mc0_level_info, 0);
        }
        //MC1
        node_id = 3;
        if( ((flag >> 35) & 0x1) || mandatory ){
            printf("Store MC info of Node %d\n", node_id);
            ddr_param_buf[0] = 0x1;
            ddr_param_buf[1] = __raw__readq(DIMM_INFO_ADDR + 0x70);
            ddr_param_buf[2] = __raw__readq(DIMM_INFO_ADDR + 0x78);
            ddr_param_buf[3] = __raw__readq(DIMM_INFO_ADDR + 0x80);
            ddr_param_buf[4] = __raw__readq(DIMM_INFO_ADDR + 0x88);
            ddr_param_buf[5] = __raw__readq(DIMM_INFO_ADDR + 0xa8);
#ifdef DEBUG
            printf("mc level info is 0x%016llx\n", ddr_param_buf[0]);
            printf("dimm 0 info is 0x%016llx\n", ddr_param_buf[1]);
            printf("dimm 0 info is 0x%016llx\n", ddr_param_buf[2]);
            printf("dimm 1 info is 0x%016llx\n", ddr_param_buf[3]);
            printf("dimm 1 info is 0x%016llx\n", ddr_param_buf[4]);
            printf("clksel info is 0x%016llx\n", ddr_param_buf[5]);
#endif
            save_ddrparam(node_id << 44, &ddr_param_buf, (int)&c1_mc1_level_info, 0);
        }
#endif
#endif
        return(1);
    }
    else{
        printf("Token is broken! Date is 0x%016llx\n", __raw__readq(DIMM_INFO_ADDR));
        return(0);
    }
}

#endif

int cmd_save_ddrparam(ac, av)
    int ac;
    char *av[];
{
    printf("start save_ddrparam\n");
    if(save_board_ddrparam(1) == 1)
    {
        printf("save_ddrparam done\n");
        return(1);
    }
    else
    {
        printf("save_ddrparam fail\n");
        return(0);
    }
};

#else

int cmd_save_ddrparam(ac, av)
    int ac;
    char *av[];
{
    return(1);
};

#endif

#ifdef ARB_LEVEL

int cmd_clear_level_mark(ac, av)
    int ac;
    char *av[];
{
    unsigned long long flag;
    flag = 0x0;
    if(ac == 1){
        //printf("ac = %d, av[0] == %s\n", ac, av[0]);
        //clear all 4 MC level mark
        printf("Clear all MC level mark!\n");
        tgt_flashprogram((int *)(0xbfc00000 + ((int)&c0_mc0_level_info - (int)&_start)), 8, &flag, TRUE);
#ifndef LOONGSON_2H
        tgt_flashprogram((int *)(0xbfc00000 + ((int)&c0_mc1_level_info - (int)&_start)), 8, &flag, TRUE);
#endif
#if ((loongson3A3 && MULTI_CHIP) || (LS3B && DUAL_3B))
        tgt_flashprogram((int *)(0xbfc00000 + ((int)&c1_mc0_level_info - (int)&_start)), 8, &flag, TRUE);
        tgt_flashprogram((int *)(0xbfc00000 + ((int)&c1_mc1_level_info - (int)&_start)), 8, &flag, TRUE);
#endif
    }
    else{
        //clear the user specified MC level mark
        //printf("ac = %d, av[0] == %s, av[1] == %s\n", ac, av[0], av[1]);
        if(atoi(av[1]) == 0){
            printf("Clear MC 0 level mark!\n");
            tgt_flashprogram((int *)(0xbfc00000 + ((int)&c0_mc0_level_info - (int)&_start)), 8, &flag, TRUE);
        }
#ifndef LOONGSON_2H
        else if(atoi(av[1]) == 1){
            printf("Clear MC 1 level mark!\n");
            tgt_flashprogram((int *)(0xbfc00000 + ((int)&c0_mc1_level_info - (int)&_start)), 8, &flag, TRUE);
        }
#endif
#if ((loongson3A3 && MULTI_CHIP) || (LS3B && DUAL_3B))
        else if(atoi(av[1]) == 2){
            printf("Clear MC 2 level mark!\n");
            tgt_flashprogram((int *)(0xbfc00000 + ((int)&c1_mc0_level_info - (int)&_start)), 8, &flag, TRUE);
        }
        else if(atoi(av[1]) == 3){
            printf("Clear MC 3 level mark!\n");
            tgt_flashprogram((int *)(0xbfc00000 + ((int)&c1_mc1_level_info - (int)&_start)), 8, &flag, TRUE);
        }
#endif
        else{
            printf("Unknown MC indentifier!!!");
        }
    }
    return(1);
};
#else
int cmd_clear_level_mark(ac, av)
    int ac;
    char *av[];
{
    return(1);
};
#endif

static const Cmd Cmd_clear_level_mark[] =
{
    {"Misc"},
    {"clear_level_mark", "[mc id]", 0, "clear leveled mark", cmd_clear_level_mark, 1, 99, 0},
    {0, 0}
};
/*
 *
 *  Command table registration
 *  ==========================
 */

static const Cmd Cmd_save_ddrparam[] =
{
    {"Misc"},
    {"save_ddrparam",    "", 0, "Save MC parameters into FALSH", cmd_save_ddrparam, 1, 99, 0},
    {0, 0}
};

static void init_cmd __P((void)) __attribute__ ((constructor));

void
init_cmd()
{
    cmdlist_expand(Cmd_save_ddrparam, 1);
    cmdlist_expand(Cmd_clear_level_mark, 1);
}
