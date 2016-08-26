/*	$Id: fan.c,v 1.1.1.1 2006/09/14 01:59:08 xqch Exp $ */
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
 *	This product includes software developed by Opsycon AB, Sweden.
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

#include "gmac.h"

#define read32(x) (*(volatile unsigned int *)(x))
#define read8(x) (*(volatile unsigned char *)(x))

unsigned char
cmd_usbtest(ac, av)
	int ac;
	char *av[];
{
	unsigned int base = 0xbbe00000;
	int port,test_mode;
	if (ac != 3){
	  printf("Usage:usbtest <port > <T> \n");
	  printf("port0: 0\n");
	  printf("port1: 1\n");
	  printf("port2: 2\n");
	  printf("port3: 3\n");
	  printf("port4: 4\n");
	  printf("port5: 5\n");
	  printf("<T> 1: J_STATE\n");
	  printf("<T> 2: K_STATE\n");
	  printf("<T> 3: SE0_NAK\n");
	  printf("<T> 4: Packet\n");
	  printf("<T> 5: FORCE_ENABLE\n");
	  printf("For example:usbtest 1 1 \n");
	  return 0;
	}

	port = atoi(av[1]);
	test_mode = atoi(av[2]);

	read32(0xbbe00010) = 0x2;
	switch (port) {
	case 0:
		read32(base | 0x54) = ((test_mode << 16) | 0x3084);
		break;
	case 1:
		read32(base | 0x58) = ((test_mode << 16) | 0x3084);
		break;
	case 2:
		read32(base | 0x5c) = ((test_mode << 16) | 0x3084);
		break;
	case 3:
		read32(base | 0x60) = ((test_mode << 16) | 0x3084);
		break;
	case 4:
		read32(base | 0x64) = ((test_mode << 16) | 0x3084);
		break;
	case 5:
		read32(base | 0x68) = ((test_mode << 16) | 0x3084);
		break;
	default:
		printf("the port number is error!\n");
		break;
	}
	
	if (test_mode == 5)
		read8(0xbBe00010) = 0x1;

	return(1);
}

cmd_satatest(ac, av)
	int ac;
	char *av[];
{
	
	unsigned int port, gen;
	unsigned int base;
	int test_mode;
	if (ac != 4){
	  printf("Usage:satatest <port > <gen> <test_mode>\n");
	  printf("port0: 0\n");
	  printf("port1: 1\n");
	  printf("gen1: 1\n");
	  printf("gen2: 2\n");
	  printf("test_mode: 0x0	SSOP( Simultaneous switching outputs pattern)\n");
	  printf("test_mode: 0x1	HTDP( High transition density pattern)       \n");
	  printf("test_mode: 0x2	LTDP( Low transition density pattern)        \n");
	  printf("test_mode: 0x3	LFSCP( Low frequency spectral component pattern)\n");
	  printf("test_mode: 0x4	COMP( Composite pattern)      \n");
	  printf("test_mode: 0x5	LBP( Lone bit pattern)        \n");
	  printf("test_mode: 0x6	MFTP( Mid frequency test pattern)\n");
	  printf("test_mode: 0x7	HFTP( High frequency test pattern)\n");
	  printf("test_mode: 0x8	LFTP( Low frequency test pattern)\n");
	
	  return 0;
	}

	port = atoi(av[1]);
	gen = atoi(av[2]);
	test_mode = atoi(av[3]);

	base = (0xbbe38000 + port * 0x100);
//	printf(" -> 0x%x\n", (gen == 1 ? 0x0 : 0x9));
	if (gen == 1)
		read8(base | 0x12) = 0x0;
	else if (gen == 2)
		read8(base | 0x12) = 0x9;

	read8(base | 0x10) = 0x1;
	read32(0xbbe300f4) = port * 0x10000;
	read32(0xbbe300a4) = (0x10000 | test_mode);

	return(1);

}

unsigned char
cmd_pcietest(ac, av)
	int ac;
	char *av[];
{
	unsigned int port, gen;
	unsigned int base,test_mode;
	unsigned int pcie_clock_source;

	if (ac < 2){

	  printf("if test gen1:pcietest  <gen1>\n");
	  printf("if test gen2:pcietest  <gen2> <gen2_test_mode> \n");
	  printf("gen1: 1\n");
	  printf("gen2: 2\n");
	  printf("gen2_test_mode: 1 ->0xf052, -3.5db De-emphasis                                      \n");
	  printf("gen2_test_mode: 2 ->0xf012, -6db De-emphasis                                        \n");
	  printf("gen2_test_mode: 3 ->0xf452, -3.5db De-emphasis, modified compliance                 \n");
	  printf("gen2_test_mode: 4 ->0xf412, -6db De-emphasis, modified compliance                   \n");
	  printf("gen2_test_mode: 5 ->0xfc12, -6db De-emphasis, modified compliance, compliance       \n");
	  printf("gen2_test_mode: 6 ->0xfc52, -3.5db De-emphasis, modified compliance, compliance SOS \n");
	  printf("For example:pcietest 2 1 \n");
	  return 0;
	}
	gen = atoi(av[1]);
	if (gen == 2) {
		test_mode = atoi(av[2]);
		read32(0xb811407c) = 0x533c42;// the low 4 bit must be 2.
	}

	base = 0xb8110000;
	read32(base | 0x480c) = 0x2040f;
	for (port = 0;port < 4;port++) {
		read8(base | (port * 0x100) | 0x11) = 0x21;
		read8(base | (port * 0x100) | 0x10) = 0xb;
	}

	if (gen == 2) {
		for (port = 0;port < 4;port++)
			read8(base | (port * 0x100) | 0x12) = 0xa;
	}

	read32(base | 0x8000) = 0xff204c;
	if (gen == 0x1) {
		read32(base | 0x40a0) = 0xfc51;
	} else if (gen == 0x2){
		switch (test_mode) {
			case 1:
				read32(base | 0x40a0) = 0xf052;
				break;
			case 2:
				read32(base | 0x40a0) = 0xf012;
				break;
			case 3:
				read32(base | 0x40a0) = 0xf452;
				break;
			case 4:
				read32(base | 0x40a0) = 0xf412;
				break;
			case 5:
				read32(base | 0x40a0) = 0xfc52;
				break;
			case 6:
				read32(base | 0x40a0) = 0xfc12;
				break;
			default:
	 			printf("The test mode is error!\n");
				break;
		}
	 	printf("test_mode = 0x%lx\n",test_mode);
	}
		
	read32(base | 0x4708) = 0x7028004;

	return(1);
}

#define u64 unsigned long
#define u32 unsigned int
#define u16 unsigned short
#define u8 unsigned char
#define GMAC0_MAC_REG_ADDR	0xffffffffbbe10000
#define GMAC0_DMA_REG_ADDR	0xffffffffbbe11000
#define GMAC1_MAC_REG_ADDR	0xffffffffbbe18000
#define GMAC1_DMA_REG_ADDR	0xffffffffbbe19000

#define PHY_REG20 20
#define PHY_REG0 0
#define PHY_REG16 16
#define PHY_LOOPBACK		(1 << 14)
#define PHY_SPEED0		(1 << 13)
#define PHY_DUMPLEX_FULL	(1 << 8)
#define PHY_SPEED1		(1 << 6)
#define PHY_MODE_100M		(PHY_SPEED0 & ~PHY_SPEED1)

static u32 gmac_read(u64 base, u32 reg)
{
	u64 addr;
	u32 data;

	addr = base + (u64)reg;
	data = ls_readl(addr);
	return data;
}

static void gmac_write(u64 base, u32 reg, u32 data)
{
	u64 addr;

	addr = base + (u64)reg;
	ls_readl(addr) = data;
	return;
}

static signed int gmac_phy_read(u64 base,u32 PhyBase, u32 reg, u16 * data )
{
	u32 addr;
	u32 loop_variable;
	addr = ((PhyBase << GmiiDevShift) & GmiiDevMask) | ((reg << GmiiRegShift) & GmiiRegMask) | GmiiCsrClk3;
	addr = addr | GmiiBusy ;

	gmac_write(base,GmacGmiiAddr,addr);

        for(loop_variable = 0; loop_variable < DEFAULT_LOOP_VARIABLE; loop_variable++){
                if (!(gmac_read(base,GmacGmiiAddr) & GmiiBusy)){
			break;
                }
		int i = DEFAULT_DELAY_VARIABLE;
		while (i--);
        }
        if(loop_variable < DEFAULT_LOOP_VARIABLE)
               * data = (u16)(gmac_read(base,GmacGmiiData) & 0xFFFF);
        else{
		tgt_printf("\rError::: PHY not responding Busy bit didnot get cleared !!!!!!\n");
		return -ESYNOPGMACPHYERR;
        }

	return -ESYNOPGMACNOERR;
}

static signed int gmac_phy_write(u64 base, u32 PhyBase, u32 reg, u16 data)
{
	u32 addr;
	u32 loop_variable;
	gmac_write(base,GmacGmiiData,data);

	addr = ((PhyBase << GmiiDevShift) & GmiiDevMask) | ((reg << GmiiRegShift) & GmiiRegMask) | GmiiWrite | GmiiCsrClk3;

	addr = addr | GmiiBusy ;
	gmac_write(base,GmacGmiiAddr,addr);
        for(loop_variable = 0; loop_variable < DEFAULT_LOOP_VARIABLE; loop_variable++){
                if (!(gmac_read(base,GmacGmiiAddr) & GmiiBusy)){
			break;
                }
		int i = DEFAULT_DELAY_VARIABLE;
		while (i--);
        }

        if(loop_variable < DEFAULT_LOOP_VARIABLE){
		return -ESYNOPGMACNOERR;
	}
        else{
	        tgt_printf("\rError::: PHY not responding Busy bit didnot get cleared !!!!!!\n");
	return -ESYNOPGMACPHYERR;
        }
}

unsigned char
cmd_lantest(ac, av)
	int ac;
	char *av[];
{
	unsigned int base,test_mode,id;
	unsigned short data;	
	u64 mac_base;
	if (ac < 2){
	  printf("lantest: lantest  <testmode>\n");
	  printf("testmode1: 1\n");
	  printf("testmode2: 2\n");
	  printf("testmode3: 3\n");
	  printf("testmode4: 4\n");
	  printf("For example:lantest 1 \n");
	  return 0;
	}
	test_mode = atoi(av[1]);

	for (id = 0;id < 2;id++) {
		if (id == 0)
			mac_base = GMAC0_MAC_REG_ADDR;
		else if (id == 1)
			mac_base = GMAC1_MAC_REG_ADDR;
        
		switch (test_mode) {
			case 1:
				gmac_phy_read(mac_base,16,9,&data);
				printf("phy 9 register value = 0x%lx\n",data);
				gmac_phy_read(mac_base,16,10,&data);
				printf("phy 10 register value = 0x%lx\n",data);
				gmac_phy_write(mac_base,16,9,(1 << 13));
				gmac_phy_read(mac_base,16,9,&data);
				printf("changed phy 9 register value = 0x%lx\n",data);
				gmac_phy_read(mac_base,16,10,&data);
				printf("changed phy 10 register value = 0x%lx\n",data);
				break;
			case 2:
				gmac_phy_read(mac_base,16,9,&data);
				printf("phy 9 register value = 0x%lx\n",data);
				gmac_phy_read(mac_base,16,10,&data);
				printf("phy 10 register value = 0x%lx\n",data);
				gmac_phy_write(mac_base,16,9, (0x2 << 13));
				gmac_phy_read(mac_base,16,9,&data);
				printf("changed phy 9 register value = 0x%lx\n",data);
				gmac_phy_read(mac_base,16,10,&data);
				printf("changed phy 10 register value = 0x%lx\n",data);
				break;
			case 3:
				gmac_phy_read(mac_base,16,9,&data);
				printf("phy 9 register value = 0x%lx\n",data);
				gmac_phy_read(mac_base,16,10,&data);
				printf("phy 10 register value = 0x%lx\n",data);
				gmac_phy_write(mac_base,16,9,(0x3 << 13));
				gmac_phy_read(mac_base,16,9,&data);
				printf("changed phy 9 register value = 0x%lx\n",data);
				gmac_phy_read(mac_base,16,10,&data);
				printf("changed phy 10 register value = 0x%lx\n",data);
				break;
			case 4:
				gmac_phy_read(mac_base,16,9,&data);
				printf("phy 9 register value = 0x%lx\n",data);
				gmac_phy_read(mac_base,16,10,&data);
				printf("phy 10 register value = 0x%lx\n",data);
				gmac_phy_write(mac_base,16,9, (0x4 << 13));
				gmac_phy_read(mac_base,16,9,&data);
				printf("changed phy 9 register value = 0x%lx\n",data);
				gmac_phy_read(mac_base,16,10,&data);
				printf("changed phy 10 register value = 0x%lx\n",data);
				break;
			default:
				printf("Test mode is error!\n");
				break;
		}
	}

	return(1);
}
int gmac_w18(void)
{
	unsigned char offset;
	unsigned char id;
	unsigned long long mac_base;
	unsigned short data;
	for (id = 0;id < 2;id++) {
		if (id == 0) {
			mac_base = GMAC0_MAC_REG_ADDR;
		} else if (id == 1){
			mac_base = GMAC1_MAC_REG_ADDR;
		}
/*
		gmac_phy_read(mac_base,16,16,&data);
		data |= 0x300;
		gmac_phy_write(mac_base,16,16,data);
		gmac_phy_read(mac_base,16,16,&data);
*/
		gmac_phy_read(mac_base,16,19,&data);
		data = 0x10;
		gmac_phy_write(mac_base,16,18,data);
		gmac_phy_read(mac_base,16,18,&data);

		gmac_phy_read(mac_base,16,19,&data);
		
		gmac_phy_read(mac_base,16,24,&data);
		data = data | 0x800;
		gmac_phy_write(mac_base,16,24,data);

		gmac_phy_read(mac_base,16,27,&data);
		data = data & (~(1 << 10));
		gmac_phy_write(mac_base,16,27,data);

		gmac_phy_read(mac_base,16,0,&data);
		data = data | 0x8000;
		gmac_phy_write(mac_base,16,0,data);
        }
	return 0;
}
int gmac_w(ac, av)
	int ac;
	char *av[];
{
	unsigned char offset;
	unsigned char id;
	unsigned long long mac_base;
	unsigned short data;
	if (ac == 3) {
	data = 0x1e1;
	printf("data  0x%lx\n",data);
	for (id = 0;id < 2;id++) {
		if (id == 0) {
			mac_base = GMAC0_MAC_REG_ADDR;
			gmac_phy_write(mac_base,16,4,data);
			gmac_phy_read(mac_base,16,4,&data);
			printf("gmac0 4 0x%lx\n",data);
			gmac_phy_read(mac_base,16,0x00,&data);
			gmac_phy_write(mac_base,16,0x00,data);
		} else if (id == 1){
			data = 0xffff;
			mac_base = GMAC1_MAC_REG_ADDR;
			gmac_phy_write(mac_base,16,4,data);
			gmac_phy_read(mac_base,16,4,&data);
			printf("gmac1 4 0x%lx\n",data);
			gmac_phy_read(mac_base,16,0x00,&data);
			data = data | 0x8000;
			gmac_phy_write(mac_base,16,0x00,data);
		}
        
	}
	}
	if (ac == 1) {
	printf("data  0x%lx\n",data);
	for (id = 0;id < 2;id++) {
		if (id == 0) {
			data = 0x10;
			mac_base = GMAC0_MAC_REG_ADDR;
			gmac_phy_write(mac_base,16,18,data);
			gmac_phy_read(mac_base,16,18,&data);
			printf("gmac0 4 0x%lx\n",data);
			gmac_phy_read(mac_base,16,0,&data);
			data = data | 0x8000;
			gmac_phy_write(mac_base,16,0,data);
		} else if (id == 1){
			data = 0x10;
			mac_base = GMAC1_MAC_REG_ADDR;
			gmac_phy_write(mac_base,16,18,data);
			gmac_phy_read(mac_base,16,18,&data);
			printf("gmac1 4 0x%lx\n",data);
			gmac_phy_read(mac_base,16,0,&data);
			data = data | 0x8000;
			gmac_phy_write(mac_base,16,0,data);
		}
        
	}
	}
	if (ac == 2) {
        for (id = 0;id < 2;id++) {
                if (id == 0)
                        mac_base = GMAC0_MAC_REG_ADDR;
                else if (id == 1)
                        mac_base = GMAC1_MAC_REG_ADDR;
        
		data = 0x40;
                gmac_phy_write(mac_base,16,18,data);
                gmac_phy_read(mac_base,16,0x00,&data);
                data = data | 0x8000;
                gmac_phy_write(mac_base,16,0x00,data);
        }
        }
	return 0;
}
int gmacphy_write(ac, av)
	int ac;
	char *av[];
{
	unsigned char offset;
	if (ac != 3) {
		printf("the parameters is error!\n");
		printf("gmacphy_write  <offset> <data>\n");
		return -1;
	}
	
	offset = atoi(av[1]);

	unsigned char id;
	unsigned long long mac_base;
	unsigned short data;
	data = atoi(av[2]);
	printf("data  0x%lx\n",data);
	for (id = 0;id < 2;id++) {
		if (id == 0)
			mac_base = GMAC0_MAC_REG_ADDR;
		else if (id == 1)
			mac_base = GMAC1_MAC_REG_ADDR;
        
		gmac_phy_write(mac_base,16,offset,data);
	}
	return 0;
}
int gmacphy_read(ac, av)
	int ac;
	char *av[];
{
	unsigned char offset;
	unsigned char id;
	unsigned long long mac_base;
	unsigned short data;
	if (ac == 1) {
		for (id = 0;id < 2;id++) {
			if (id == 0)
				mac_base = GMAC0_MAC_REG_ADDR;
			else if (id == 1)
				mac_base = GMAC1_MAC_REG_ADDR;
                
			for (offset = 0;offset < 32;offset++) {
				gmac_phy_read(mac_base,16,offset,&data);
				printf("gmac%d reg(%d) %lx\n",id,offset,data);
			}
		}

		return -1;
	}
	
	offset = atoi(av[1]);

	for (id = 0;id < 2;id++) {
		if (id == 0)
			mac_base = GMAC0_MAC_REG_ADDR;
		else if (id == 1)
			mac_base = GMAC1_MAC_REG_ADDR;
        
		gmac_phy_read(mac_base,16,offset,&data);
		printf("gmac%d reg(%d) %lx\n",id,offset,data);
	}
	return 0;
}
/*
 *
 *  Command table registration
 *  ==========================
 */

static const Cmd Cmds[] =
{
	{"Misc"},
	{"usbtest",	"", 0, "3a2h usbtest : usbtest  ", cmd_usbtest, 1, 99, 0},
	{"gmacphy_read",	"", 0, "read gmac phy reg", gmacphy_read, 1, 99, 0},
	{"gmacphy_write",	"", 0, "read gmac phy reg", gmacphy_write, 1, 99, 0},
	{"gmac_w",	"", 0, "read gmac phy reg", gmac_w, 1, 99, 0},
	{"lantest",	"", 0, "3a2h lantest : lantest  ", cmd_lantest, 1, 99, 0},
	{"pcietest",	"", 0, "3a2h pcietest: pcietest ", cmd_pcietest, 1, 99, 0},
	{"satatest",	"", 0, "3a2h satatest: satatest ", cmd_satatest, 1, 99, 0},
	{0, 0}
};

static void init_cmd __P((void)) __attribute__ ((constructor));

void
init_cmd()
{
	cmdlist_expand(Cmds, 1);
}
