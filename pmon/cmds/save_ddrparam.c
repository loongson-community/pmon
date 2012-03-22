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

u64 __raw_readq_sp(u64 q)
{
  u64 ret;

  asm volatile(
      ".set mips3;\r\n"
      "move    $8,%1;\r\n"
      "ld    %0,0($8);\r\n"
      :"=r"(ret)
      :"r" (q)
      :"$8");

    return ret;
}

u64 __raw_writeq_sp(u64 addr, u64 val)
{
  
  u64 ret;

  asm volatile(
      ".set mips3;\r\n"
      "move    $8,%1;\r\n"
      "move    $9,%2;\r\n"
      "sd    $9,0($8);\r\n"
      "ld    %0,0($8);\r\n"
      :"=r"(ret)
      :"r" (addr), "r" (val)
      :"$8", "$9");

    return ret;
}


#define sd  __raw_writeq_sp
#define ld  __raw_readq_sp

extern char _start;
extern char ddr2_leveled_mark;

#ifndef LS3B
extern char ddr2_reg_data_mc0_leveled, ddr2_reg_data_mc1_leveled;
#ifdef MULTI_CHIP
extern char n1_ddr2_reg_data_mc0_leveled, n1_ddr2_reg_data_mc1_leveled;
#endif
#else
extern char ddr2_reg_data_leveled;
#ifdef MULTI_CHIP
extern char n1_ddr2_reg_data_leveled;
#endif
#ifdef DUAL_3B
extern char n2_ddr2_reg_data_leveled;
extern char n3_ddr2_reg_data_leveled;
#endif
#endif

//#define DEBUG
#ifdef DEBUG
extern int do_cmd(char *);
extern void dump_l2xbar(int node);
#endif

#define LSMCD3_2
#ifdef  LSMCD3_2
#define DDR_PARAM_NUM     180
#else
#define DDR_PARAM_NUM     152
#endif
#define CPU_CONFIG_ADDR 0x900000001fe00180ull
#define CPU_L2XBAR_CONFIG_ADDR 0x900000003ff00000ull
#define MC_CONFIG_ADDR 0x900000000ff00000ull

static void disable_ddrconfig(u64 node_id_shift44)
{
    unsigned long long val; 

    /* Disable DDR access configure register */
    val = ld(CPU_CONFIG_ADDR | node_id_shift44);
    val |= 0x100;
    sd(CPU_CONFIG_ADDR | node_id_shift44, val);

#ifdef DEBUG
    printf("Disable sys config reg:0x1fe00180 = %016llx\n", ld(CPU_CONFIG_ADDR | node_id_shift44));
#endif
}

static void enable_ddrconfig(u64 node_id_shift44)
{
    unsigned long long val; 

    val = ld(CPU_CONFIG_ADDR | node_id_shift44);
    val &=0xfffffffffffffeffull;
    sd(CPU_CONFIG_ADDR | node_id_shift44, val);

#ifdef DEBUG
    printf("Enable sys config reg:0x1fe00180 = %016llx", ld(CPU_CONFIG_ADDR | node_id_shift44));
#endif

}


void enable_ddrcfgwindow(u64 node_id_shift44, int mc_selector, unsigned long long * buf)
    //use window 0 which is used to route 0x1fc00000 addr space, not used here.
{

    printf("Now enable ddr config windows \n");
#ifdef DEBUG
    printf("origin :: 0x00: %016llx  0x40: %016llx 0x80:  %016llx\n", ld((CPU_L2XBAR_CONFIG_ADDR | node_id_shift44)), ld((CPU_L2XBAR_CONFIG_ADDR | node_id_shift44) + 0x40), ld((CPU_L2XBAR_CONFIG_ADDR | node_id_shift44) + 0x80));
#endif
    buf[0] =  ld((CPU_L2XBAR_CONFIG_ADDR | node_id_shift44));
    buf[1] =  ld((CPU_L2XBAR_CONFIG_ADDR | node_id_shift44) + 0x40);
    buf[2] =  ld((CPU_L2XBAR_CONFIG_ADDR | node_id_shift44) + 0x80);

    sd((CPU_L2XBAR_CONFIG_ADDR | node_id_shift44), (0x000000000ff00000ull | node_id_shift44));
    sd((CPU_L2XBAR_CONFIG_ADDR | node_id_shift44) + 0x40, 0xfffffffffff00000ull);
    sd((CPU_L2XBAR_CONFIG_ADDR | node_id_shift44) + 0x80, 0x000000000ff000f0ull | mc_selector);

#ifdef DEBUG
    printf("new  :: 0x00: %016llx  0x40: %016llx 0x80:  %016llx\n", ld((CPU_L2XBAR_CONFIG_ADDR | node_id_shift44)), ld((CPU_L2XBAR_CONFIG_ADDR | node_id_shift44) + 0x40), ld((CPU_L2XBAR_CONFIG_ADDR | node_id_shift44) + 0x80));

    printf("Now save L2X bar windows\n");
    printf("buf[0] = %llx, buf[1] = %llx, buf[2] = %llx\n", buf[0], buf[1], buf[2]); 

    printf("Now dump L2X bar windows\n");
    dump_l2xbar(1);
#endif

}

    
//restore origin configure
void disable_ddrcfgwidow(u64 node_id_shift44, int mc_selector, unsigned long long * buf)
{
#ifdef DEBUG
    printf("Now retore L2X bar windows\n");
    printf("buf[0] = %llx, buf[1] = %llx, buf[2] = %llx\n", buf[0], buf[1], buf[2]); 
#endif

    sd((CPU_L2XBAR_CONFIG_ADDR | node_id_shift44) + 0x00, buf[0] );
    sd((CPU_L2XBAR_CONFIG_ADDR | node_id_shift44) + 0x40, buf[1] );
    sd((CPU_L2XBAR_CONFIG_ADDR | node_id_shift44) + 0x80, buf[2] );

#ifdef DEBUG
    printf("new  :: 0x00: %016llx  0x40: %016llx 0x80:  %016llx\n", ld((CPU_L2XBAR_CONFIG_ADDR | node_id_shift44)), ld((CPU_L2XBAR_CONFIG_ADDR | node_id_shift44) + 0x40), ld((CPU_L2XBAR_CONFIG_ADDR | node_id_shift44) + 0x80));
#endif
}

#define MC0 0x0
#define MC1 0x1

int read_ddr_param(u64 node_id_shift44, int mc_selector,  unsigned long long * base)
{
    int i;
    unsigned long long * val = base;
    unsigned long long buf[3];

    //do_cmd("showwindows");
    //dump_l2xbar(1);

#ifndef LS3B
    // step 1. Change The Primest window for MC0 or MC1 register space 
    enable_ddrcfgwindow(node_id_shift44, mc_selector, buf);

    //do_cmd("showwindows");
    //dump_l2xbar(1);

    // step 2. Enabel access to MC0 or MC1 register space 
    enable_ddrconfig(node_id_shift44);
#endif
    // step 3. Read out ddr config register to buffer
    printf("\nNow Read out DDR parameter from DDR MC%d controler after DDR training\n", mc_selector);
    for ( i = DDR_PARAM_NUM - 1; i >= 0; i--) // NOTICE HERE: it means system has DDR_PARAM_NUM double words
    {
        val[i] =  ld((MC_CONFIG_ADDR | node_id_shift44) + (0x10 * i)); 

#ifdef DEBUG
        printf("< CFGREG >:val[%03d]  = %016llx \n", i, val[i]); 
#endif
    }
    //clear param_start
    val[3]  &=  0xfffffeffffffffff;

#ifndef LS3B
    // step 4. Disabel access to MC0 or MC1 register space 
    disable_ddrconfig(node_id_shift44);

    // step 5. Restore The Primest window for accessing system memory
    disable_ddrcfgwidow(node_id_shift44, mc_selector, buf);
#endif

    printf("Read out DDR MC%d config Done.\n", mc_selector);
    return 0;
}

#ifndef LS3B

void save_ddrparam(u64 node_id_shift44, int mc0_param_store_addr, int mc1_param_store_addr)
{

  unsigned long long ddr_param_buf[DDR_PARAM_NUM + 1];
  unsigned long long tmp1, tmp2, tmp3, tmp4;

#ifdef DEBUG
  int   i;
  unsigned long long tmp;
#endif

#ifdef DEBUG
  printf("\nnode_id_shift44=0x%016llx\n", node_id_shift44);
#endif
  /********************************************************/
  /************************/ // End of flash 
  /*      DDRPTOVF          */ // End - 8 (byte) (1M-8)
  /* -------------------- */ //
  /*                      */ // 
  /*      .......          */
  /*      .......          */
  /*      .......          */ // $ddr3_data
  /*      .......          */
  /*                      */ 
  /************************/ // Base of flash: offset 0x00
  /********************************************************/

  // step 1. Read out DDR controler register values and save them in buffers

  //according to low 256M route manner to decide NODE MC enable state.  not ok now!
  //according to high memory route manner to decide NODE MC enable state.
    tmp1 = ld((CPU_L2XBAR_CONFIG_ADDR | node_id_shift44) + 0xa0);
    tmp2 = ld((CPU_L2XBAR_CONFIG_ADDR | node_id_shift44) + 0xa8);
    tmp3 = ld((CPU_L2XBAR_CONFIG_ADDR | node_id_shift44) + 0xb0);
    tmp4 = ld((CPU_L2XBAR_CONFIG_ADDR | node_id_shift44) + 0xb8);
#ifdef DEBUG
  printf("tmp1=0x%016llx\n", tmp1);
  printf("tmp2=0x%016llx\n", tmp2);
#endif
    //note, only check the last 4-bit is not ok!
    tmp1 &= 0xff;
    tmp2 &= 0xff;
    tmp3 &= 0xff;
    tmp4 &= 0xff;
    if ((tmp1 == 0xf0) || (tmp2 == 0xf0) || (tmp3 == 0xf0) || (tmp4 == 0xf0))
    {
        // step 1.1 Read out DDR controler register from MC0 and save them in buffer0
        read_ddr_param(node_id_shift44, MC0, ddr_param_buf);

        //ddr_param_buf[DDR_PARAM_NUM] = 0x0;
        // step 1.2 Program buffers of MC0 register into FLASH 
        tgt_flashprogram((int *)(0xbfc00000+(mc0_param_store_addr -(int)&_start)), DDR_PARAM_NUM*8, ddr_param_buf,TRUE);

#ifdef DEBUG
  for(i = 0; i< DDR_PARAM_NUM; i++)
    {
        tmp =  ld(0x900000001fc00000 + mc0_param_store_addr - (int)&_start + i * 8);
        if(ddr_param_buf[i] != tmp)
        {
            printf("\nMiscompare:i=%d, val=%016llx", i, tmp);
        }
        else
            printf("\nSame:i=%d, val=%016llx", i, tmp);
    }
#endif
    }

    if ((tmp1 == 0xf1) || (tmp2 == 0xf1) || (tmp3 == 0xf1) || (tmp4 == 0xf1))
    {
        // step 2.2 Read out DDR controler register from MC1 and save them in buffer1
        read_ddr_param(node_id_shift44, MC1, ddr_param_buf);

        // step 2.2 Program buffers of MC1 register into FLASH 
        tgt_flashprogram((int *)(0xbfc00000+(mc1_param_store_addr -(int)&_start)), DDR_PARAM_NUM*8, ddr_param_buf,TRUE);

#ifdef DEBUG
  for(i = 0; i< DDR_PARAM_NUM; i++)
    {
        tmp =  ld(0x900000001fc00000 + mc1_param_store_addr - (int)&_start + i * 8);
        if(ddr_param_buf[i] != tmp)
        {
            printf("\nMiscompare:i=%d, val=%016llx", i, tmp);
        }
        else
            printf("\nSame:i=%d, val=%016llx", i, tmp);
    }
#endif
    }

}

int save_board_ddrparam()
{
    unsigned long long flag;
    unsigned long long node_id;
    if(ld(0x900000001fc00000 + (int) &ddr2_leveled_mark - (int)&_start) == 0)
    {
        node_id = 0;
        save_ddrparam(node_id << 44, (int)&ddr2_reg_data_mc0_leveled, (int)&ddr2_reg_data_mc1_leveled);
#ifdef MULTI_CHIP
        node_id = 1;
        save_ddrparam(node_id << 44, (int)&n1_ddr2_reg_data_mc0_leveled, (int)&n1_ddr2_reg_data_mc1_leveled);
#endif
        flag = 0x1;
        tgt_flashprogram((int *)(0xbfc00000 + ((int)&ddr2_leveled_mark - (int)&_start)), 8, &flag, TRUE);
    }
    return(1);
}
#else

void save_ddrparam(u64 node_id_shift44, int mc0_param_store_addr)
{

  unsigned long long ddr_param_buf[DDR_PARAM_NUM + 1];

#ifdef DEBUG
  int   i;
  unsigned long long tmp;
#endif

#ifdef DEBUG
  printf("\nnode_id_shift44=0x%016llx\n", node_id_shift44);
#endif
  /********************************************************/
  /************************/ // End of flash 
  /*      DDRPTOVF          */ // End - 8 (byte) (1M-8)
  /* -------------------- */ //
  /*                      */ // 
  /*      .......          */
  /*      .......          */
  /*      .......          */ // $ddr3_data
  /*      .......          */
  /*                      */ 
  /************************/ // Base of flash: offset 0x00
  /********************************************************/

  // step 1. Read out DDR controler register values and save them in buffers

  // step 1.1 Read out DDR controler register from MC0 and save them in buffer0
  read_ddr_param(node_id_shift44, MC0, ddr_param_buf);

  //ddr_param_buf[DDR_PARAM_NUM] = 0x0;
  // step 1.2 Program buffers of MC0 register into FLASH 
  tgt_flashprogram((int *)(0xbfc00000+(mc0_param_store_addr -(int)&_start)), DDR_PARAM_NUM*8, ddr_param_buf,TRUE);

#ifdef DEBUG
  for(i = 0; i< DDR_PARAM_NUM; i++)
    {
        tmp =  ld(0x900000001fc00000 + mc0_param_store_addr - (int)&_start + i * 8);
        if(ddr_param_buf[i] != tmp)
        {
            printf("\nMiscompare:i=%d, val=%016llx", i, tmp);
        }
        else
            printf("\nSame:i=%d, val=%016llx", i, tmp);
    }
#endif

}

int save_board_ddrparam()
{
    unsigned long long flag;
    unsigned long long node_id;
    if(ld(0x900000001fc00000 + (int) &ddr2_leveled_mark - (int)&_start) == 0)
    {
        node_id = 0;
        save_ddrparam(node_id << 44, (int)&ddr2_reg_data_leveled);
#ifdef MULTI_CHIP
        node_id = 1;
        save_ddrparam(node_id << 44, (int)&n1_ddr2_reg_data_leveled);
#endif
#ifdef DUAL_3B
        node_id = 2;
        save_ddrparam(node_id << 44, (int)&n2_ddr2_reg_data_leveled);
        node_id = 3;
        save_ddrparam(node_id << 44, (int)&n3_ddr2_reg_data_leveled);
#endif
        flag = 0x1;
        tgt_flashprogram((int *)(0xbfc00000 + ((int)&ddr2_leveled_mark - (int)&_start)), 8, &flag, TRUE);
    }
    return(1);
}

#endif

int cmd_save_ddrparam(ac, av)
    int ac;
    char *av[];
{
    printf("start save_ddrparam\n");
    if(save_board_ddrparam() == 1)
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
    printf("clear level mark\n");
    flag = 0x0;
    tgt_flashprogram((int *)(0xbfc00000 + ((int)&ddr2_leveled_mark - (int)&_start)), 8, &flag, TRUE);
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
    {"clear_level_mark",    "", 0, "clear leveled mark", cmd_clear_level_mark, 1, 99, 0},
    {0, 0}
};
/*
 *
 *  Command table registration
 *  ==========================
 */

static const Cmd Cmds[] =
{
    {"Misc"},
    {"save_ddrparam",    "", 0, "Save MC parameters into FALSH", cmd_save_ddrparam, 1, 99, 0},
    {0, 0}
};

static void init_cmd __P((void)) __attribute__ ((constructor));

void
init_cmd()
{
    cmdlist_expand(Cmds, 1);
    cmdlist_expand(Cmd_clear_level_mark, 1);
}
