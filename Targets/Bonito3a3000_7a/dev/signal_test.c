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


#define pt32(x) (*(volatile unsigned int *)(x))
#define pt8(x) (*(volatile unsigned char *)(x))

unsigned char
cmd_usbtest(ac, av)
	int ac;
	char *av[];
{
	unsigned int tmp;
	unsigned int base;
	unsigned int cntl, port, test_mode;
	if (ac != 3){
		printf("Usage:usbtest <port > <Test mode> \n");
		printf("<Test mode> 1: J_STATE\n");
		printf("<Test mode> 2: K_STATE\n");
		printf("<Test mode> 3: SE0_NAK\n");
		printf("<Test mode> 4: Packet\n");
		printf("<Test mode> 5: FORCE_ENABLE\n");
		printf("For example:usbtest 1 1 \n");
		return 0;
	}

	port = (unsigned int)atoi(av[1]);
	test_mode = (unsigned int)atoi(av[2]);

	if(port < 3){
		cntl = 0;
	}else if(port < 6){
		cntl = 1;
		port -= 3;
	}else{
		printf("Error: port number exceed valid value(5)\n");
		return 0;
	}
	if(test_mode < 1 || test_mode > 5){
		printf("Error: test mode exceed valid value[1,5]\n");
		return 0;
	}
	printf("[debug]: USB cntl %d port %d\n", cntl, port);

	//get cntl base address, func 1: EHCI
	base = *(volatile unsigned int *)(0xba000000 | (0 << 16) | ((4+cntl) << 11) | (1 << 8) | 0x10);
	base &= 0xfffffff0;
	base |= 0x80000000;

	base += 0x10; //HC operational register base
	printf("[debug]: USB operational register base is: 0x%x\n", base);

	//reset USB to stop last test
	pt32(base) = 0x2;

	//set test mode
	pt32(base + 0x44 + port * 4) = ((test_mode << 16) | 0x3084);

	if (test_mode == 5){
		pt32(base) = 0x1;
	}
	printf("USB test ready and start...\n");

	return 1;
}

cmd_satatest(ac, av)
	int ac;
	char *av[];
{
	unsigned int port, gen;
	unsigned int base;
	unsigned int test_mode;
	unsigned int tmp;
	if (ac != 4){
		printf("Usage:satatest <port > <gen> <test_mode>\n");
		printf("<gen> 1: SATA 1.0\n");
		printf("<gen> 2: SATA 2.0\n");
		printf("<test_mode> 0: SSOP( Simultaneous switching outputs pattern)\n");
		printf("<test_mode> 1: HTDP( High transition density pattern)       \n");
		printf("<test_mode> 2: LTDP( Low transition density pattern)        \n");
		printf("<test_mode> 3: LFSCP( Low frequency spectral component pattern)\n");
		printf("<test_mode> 4: COMP( Composite pattern)      \n");
		printf("<test_mode> 5: LBP( Lone bit pattern)        \n");
		printf("<test_mode> 6: MFTP( Mid frequency test pattern)\n");
		printf("<test_mode> 7: HFTP( High frequency test pattern)\n");
		printf("<test_mode> 8: LFTP( Low frequency test pattern)\n");
		return 0;
	}

	//port actual mean cntl
	port = (unsigned int)atoi(av[1]);
	gen = (unsigned int)atoi(av[2]);
	test_mode =(unsigned int)atoi(av[3]);

	if(port > 2){
		printf("Error: port exceed max value(2)\n");
		return 0;
	}
	if(gen != 1 && gen != 2){
		printf("Error: Gen is not a valid value[1,2]\n");
		return 0;
	}
	if(test_mode > 8){
		printf("Error: test mode exceed valid value[0,8]\n");
		return 0;
	}
	//get cntl base address
	base = *(volatile unsigned int *)(0xba000000 | (0 << 16) | (8 << 11) | (port << 8) | 0x10);
	base &= 0xfffffff0;
	base |= 0x80000000;

	printf("[debug]: request test: port: %d, gen: %d, test mode: %d\n", port, gen, test_mode);
	printf("[debug]: SATA ctrl register base is: 0x%x\n", base);

	//TODO PHY cfg, GEN sel
	//if (gen == 1)
	//	pt8(base + 0x8012) = 0x0;
	//else if (gen == 2)
	//	pt8(base + 0x8012) = 0x9;
	//pt8(base + 0x8010) = 0x1;

	//set port to 0
	pt32(base + 0xf4) = 0x0;
	//
	tmp = pt32(base + 0xa4);
	//transmit only
	pt32(base + 0xa4) = ((1 << 18) | (tmp & ~0xf & ~(0x7 << 8)) | test_mode);

	return(1);
}

//TODO
unsigned char
cmd_pcietest(ac, av)
	int ac;
	char *av[];
{
	unsigned int header, cntl, port, gen;
	unsigned int base,test_mode;
	unsigned int pcie_clock_source;

	if (ac < 2){
		printf("usage: pcietest <port name> <gen> [test mode for gen2]\n");
		printf("port name: F0_P0 \n");
		printf("gen2_test_mode: 1 ->0xf052, -3.5db De-emphasis                                      \n");
		printf("gen2_test_mode: 2 ->0xf012, -6db De-emphasis                                        \n");
		printf("gen2_test_mode: 3 ->0xf452, -3.5db De-emphasis, modified compliance                 \n");
		printf("gen2_test_mode: 4 ->0xf412, -6db De-emphasis, modified compliance                   \n");
		printf("gen2_test_mode: 5 ->0xfc52, -3.5db De-emphasis, modified compliance, compliance SOS \n");
		printf("gen2_test_mode: 6 ->0xfc12, -6db De-emphasis, modified compliance, compliance       \n");
		printf("For example:pcietest 2 1 \n");
		return 0;
	}

	gen = (unsigned int)atoi(av[2]);
	if (gen == 2) {
		test_mode = (unsigned int)atoi(av[3]);

		pt32(header + 0x7c) = 0x533c42;// the low 4 bit must be 2.
	}

	pt32(header + 0x80c) = 0x2040f;

	//TODO PHY cfg, override GEN mode
	//for (port = 0;port < 4;port++) {
	//	pt8(base | (port * 0x100) | 0x11) = 0x21;
	//	pt8(base | (port * 0x100) | 0x10) = 0xb;
	//}

	//if (gen == 2) {
	//	for (port = 0;port < 4;port++)
	//		pt8(base | (port * 0x100) | 0x12) = 0xa;
	//}


	pt32(cntl) = 0xff204c;

	if (gen == 0x1) {
		pt32(header + 0xa0) = 0xfc51;
	} else if (gen == 0x2){
		switch (test_mode) {
			case 1:
				pt32(header + 0xa0) = 0xf052;
				break;
			case 2:
				pt32(header + 0xa0) = 0xf012;
				break;
			case 3:
				pt32(header + 0xa0) = 0xf452;
				break;
			case 4:
				pt32(header + 0xa0) = 0xf412;
				break;
			case 5:
				pt32(header + 0xa0) = 0xfc52;
				break;
			case 6:
				pt32(header + 0xa0) = 0xfc12;
				break;
			default:
				break;
		}
	}
	pt32(header + 0x708) = 0x7028004;

	return(1);
}

/*
 *
 *  Command table registration
 *  ==========================
 */

static const Cmd Cmds[] =
{
	{"Misc"},
	{"usbtest",	 "", 0, "7A usbtest : usbtest  ", cmd_usbtest, 1, 99, 0},
	//{"pcietest", "", 0, "7A pcietest: pcietest ", cmd_pcietest, 1, 99, 0},
	{"satatest", "", 0, "7A satatest: satatest ", cmd_satatest, 1, 99, 0},
	{0, 0}
};

static void init_cmd __P((void)) __attribute__ ((constructor));

void
init_cmd()
{
	cmdlist_expand(Cmds, 1);
}
