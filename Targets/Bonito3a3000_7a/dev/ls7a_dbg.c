/* $Id: ls7a_dbg.c,v 1.1.1.1 2006/09/14 01:59:08 root Exp $ */
/*
 * Copyright (c) 2002 Opsycon AB  (www.opsycon.se)
 * Copyright (c) 2002 Patrik Lindergren  (www.lindergren.com)
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
 *	This product includes software developed by Opsycon AB.
 *	This product includes software developed by Patrik Lindergren.
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
/*  This code is based on code made freely available by Algorithmics, UK */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/errno.h>
#include <sys/endian.h>

#include <pmon.h>

#ifdef __mips__
#include <machine/cpu.h>
#endif

#define u64 unsigned long long
#define u32 unsigned int

int cmd_nphy_cfg __P((int, char *[]));
static int nphy_cfg __P((int, char **));

unsigned long long strtoull(const char *nptr,char **endptr,int base);

#define write_reg(addr, val) (*(volatile u32*)(addr) = (val))
#define read_reg(addr)       (*(volatile u32*)(addr))

static int
nphy_cfg (argc, argv)
	int argc;
	char **argv;
{
    int flag_wr = 0;
    unsigned int *reg_ptr;
    unsigned int reg_addr;
    unsigned int cfg_addr;
    unsigned int tmp;
    unsigned int data=0;

    if(argc < 3){
        printf("Error: please specify cfg reg address(offset) and phy cfg address!\n");
        return EXIT_FAILURE;
    }else{
        reg_addr = strtoull(argv[1], 0, 0) & 0xffff;
        cfg_addr = strtoull(argv[2], 0, 0) & 0xffff;
    }
    if(argc > 3){
        data = strtoull(argv[3], 0, 0) & 0xffff;
        flag_wr = 1;
    }
    reg_addr |= 0xb0010000;

    printf("debug: get parameter:reg_addr=0x%x, cfg_addr=0x%x, data=0x%x\n",reg_addr, cfg_addr, data);

    if(flag_wr){
        //cfg write
        //printf("cfg wr.\n");
        while(!(read_reg(reg_addr+4) & 0x4)) ;
        write_reg(reg_addr, (data << 16) | cfg_addr);
        write_reg(reg_addr+4, 1);
    }else{
        //cfg read
        //printf("cfg rd.\n");
        while(!(read_reg(reg_addr+4) & 0x4)) ;
        write_reg(reg_addr, cfg_addr);
        write_reg(reg_addr+4, 0);

        while(!(read_reg(reg_addr+4) & 0x4)) ;
        tmp = read_reg(reg_addr);
        printf("cfg data is: 0x%x\n", tmp);
    }

	return EXIT_SUCCESS;
}

int
cmd_nphy_cfg (argc, argv)
    int argc; char **argv;
{
    int ret;
    ret = spawn ("phy_cfg", nphy_cfg, argc, argv);
    return (ret & ~0xff) ? 1 : (signed char)ret;
}

/*
 *  Command table registration
 *  ==========================
 */
static const Cmd Cmds[] =
{
   {"Misc"},
   {"phy_cfg",	"", 0,
   "ls7a pcie/sata phy_cfg",
   cmd_nphy_cfg, 1, 4, 0},
   {0, 0}
};

static void init_cmd __P((void)) __attribute__ ((constructor));

static void
   init_cmd()
{
   cmdlist_expand(Cmds, 1);
}
