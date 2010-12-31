/*
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
 *	This product includes software developed by
 *	Opsycon Open System Consulting AB, Sweden.
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
#include <termio.h>
#include <string.h>
#include <setjmp.h>
#include <sys/endian.h>
#include <ctype.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#ifdef _KERNEL
#undef _KERNEL
#include <sys/ioctl.h>
#define _KERNEL
#else
#include <sys/ioctl.h>
#endif

#include <machine/cpu.h>
#include <machine/bus.h>

#include <pmon.h>

#include <dev/pci/pcivar.h>
#include <dev/pci/pcidevs.h>
#include <Targets/Bonito3a780e/pci/sb700.h>
#include <Targets/Bonito3a780e/pci/rs780_cmn.h>
/*
 * Prototypes
 */
int cmd_msqt_sata __P((int, char *[]));
/*
 * Motherboard Signal Quality Test for ATP SATA Controller
 */

	int
cmd_msqt_sata(int ac, char *av[])
{
	u16 word,P0TxTEST;
	u16 output;
	device_t dev = _pci_make_tag(0, 0x11, 0);
	int addr,addr1;
	/* enable sata0,sata1,sata2,sata3,sata4,sata5 phyctrl*/
	if (!strcmp(*(av + 1), "port0")){
		printf("Enable port0\n");
		addr = 0xb0;
		if(!strcmp(*(av + 3), "1.5G")) {
			addr1 = 0x88;
		}else{
			addr1 = 0xa0;
		}
		word = pci_read_config16(dev, 0xb0);
		word |= 0x01;
		pci_write_config16(dev, 0xb0, word);
	}else if (!strcmp(*(av + 1), "port1")){
		printf("Enable port1\n");
		addr = 0xb8;
		if(!strcmp(*(av + 3), "1.5G")) {
			addr1 = 0x8c;
		}else{
			addr1 = 0xa2;
		}
		word = pci_read_config16(dev, 0xb8);
		word |= 0x01;
		pci_write_config16(dev, 0xb8, word);
	}else if (!strcmp(*(av + 1), "port2")){
		printf("Enable port2\n");
		addr = 0xc0;
		if(!strcmp(*(av + 3), "1.5G")) {
			addr1 = 0x90;
		}else{
			addr1 = 0xa4;
		}
		word = pci_read_config16(dev, 0xc0);
		word |= 0x01;
		pci_write_config16(dev, 0xc0, word);
	}else if (!strcmp(*(av + 1), "port3")){
		printf("Enable port3\n");
		addr = 0xc8;
		if(!strcmp(*(av + 3), "1.5G")) {
			addr1 = 0x94;
		}else{
			addr1 = 0xa6;
		}
		word = pci_read_config16(dev, 0xc8);
		word |= 0x01;
		pci_write_config16(dev, 0xc8, word);
	}else if (!strcmp(*(av + 1), "port4")){
		printf("Enable port4\n");
		addr = 0xd0;
		if(!strcmp(*(av + 3), "1.5G")) {
			addr1 = 0x98;
		}else{
			addr1 = 0xa8;
		}
		word = pci_read_config16(dev, 0xd0);
		word |= 0x01;
		pci_write_config16(dev, 0xd0, word);
	}else if (!strcmp(*(av + 1), "port5")){
		printf("Enable port5\n");
		addr = 0xd8;
		if(!strcmp(*(av + 3), "1.5G")) {
			addr1 = 0x9c;
		}else{
			addr1 = 0xaa;
		}
		word = pci_read_config16(dev, 0xd8);
		word |= 0x01;
		pci_write_config16(dev, 0xd8, word);
	}else
		printf("Please choose correct test port\n");


	/* set the tx test pattern  */
	P0TxTEST = pci_read_config16(dev, addr);
	P0TxTEST = ~(0xf << 2) & P0TxTEST;

	if(!strcmp(*(av + 2), "align")) {
		printf("TX test pattern is align.\n");
		P0TxTEST |= 0x0 << 2 ; /* align */
	}
	else if(!strcmp(*(av + 2), "D10.2")) {
		printf("TX test pattern is D10.2.\n");
		P0TxTEST |= 0x1 << 2 ; /* D10.2 */
	}
	else if(!strcmp(*(av + 2), "sync")) {
		printf("TX test pattern is sync.\n");
		P0TxTEST |= 0x2 << 2 ; /* sync */
	}
	else if(!strcmp(*(av + 2), "lbp")) {
		printf("TX test pattern is lbp.\n");
		P0TxTEST |= 0x3 << 2 ; /* lbp */
	}
	else if(!strcmp(*(av + 2), "mftp")) {
		printf("TX test pattern is mftp.\n");
		P0TxTEST |= 0x4 << 2 ; /* mftp */
	}
	else if(!strcmp(*(av + 2), "20bit")) {
		printf("TX test pattern is 20bit.\n");
		P0TxTEST |= 0x5 << 2 ; /* 20bit*/
	}
	else if(!strcmp(*(av + 2), "fferlb")) {
		printf("TX test pattern is fferlb.\n");
		P0TxTEST |= 0x6 << 2 ; /* fferlb */
	}
	else if(!strcmp(*(av + 2), "Tmode")) {
		printf("TX test pattern is Tmode.\n");
		P0TxTEST |= 0x7 << 2 ; /* Tmode */
	}
	else
		printf("Please choose correct test pattern: align/D10.2/sync/lbp/mftp/20bit/fferlb/Tmode.\n");

	P0TxTEST &= ~(0x1 << 1);

	if(!strcmp(*(av + 3), "1.5G")) {
		printf("speed is 1.5G.\n");
		P0TxTEST &= ~(0x1 << 1) ; /* 1.5G gen1 */
	}else if(!strcmp(*(av + 3), "3G")) {
		printf("speed is 3G.\n");
		P0TxTEST |= (0x1 << 1) ; /* 3G gen11 */
	}else
		printf("Please choose correct speed : 1.5G/3G\n");

	output = pci_read_config16(dev, addr1);
	output &= ~(0x1f << 0);

	if(!strcmp(*(av + 4), "1")) {
		printf("Driver Nominal Output 400mv.\n");
		output |= (0x10 << 0) ; /* 400mv */
	}else if(!strcmp(*(av + 4), "2")) {
		printf("Driver Nominal Output 450mv.\n");
		output |= (0x12 << 0) ; /* 450mv */
	}else if(!strcmp(*(av + 4), "3")) {
		printf("Driver Nominal Output 500mv.\n");
		output |= (0x14 << 0) ; /* 500mv */
	}else if(!strcmp(*(av + 4), "4")) {
		printf("Driver Nominal Output 550mv.\n");
		output |= (0x16 << 0) ; /* 550mv */
	}else if(!strcmp(*(av + 4), "5")) {
		printf("Driver Nominal Output 600mv.\n");
		output |= (0x18 << 0) ; /* 600mv */
	}else if(!strcmp(*(av + 4), "6")) {
		printf("Driver Nominal Output 650mv.\n");
		output |= (0x1a << 0) ; /* 650mv */
	}else if(!strcmp(*(av + 4), "7")) {
		printf("Driver Nominal Output 700mv.\n");
		output |= (0x1c << 0) ; /* 700mv */
	}else if(!strcmp(*(av + 4), "8")) {
		printf("Driver Nominal Output 750mv.\n");
		output |= (0x1e << 0) ; /* 750mv */
	}else
		printf("Please choose correct Nominal Output [ 1/2/3/4/5/6/7/8]\n");

	pci_write_config16(dev, addr, P0TxTEST);
	pci_write_config16(dev, addr1, output);

	printf("High frequency pattern.\n");

	return 0;
}

#if 1
	int
cmd_msqt_sata1(int ac, char *av[])
{

	u16 word;
	u32 output;
	device_t dev = _pci_make_tag(0, 0x11, 0);
	int addr;
	/* enable sata0,sata1,sata2,sata3,sata4,sata5 phyctrl*/
	if (!strcmp(*(av + 1), "port0")){
		printf("Enable port0 TX pre-emphasis driver swing\n");
		if(!strcmp(*(av + 2), "1.5G")) {
			addr = 0x88;
		}else{
			addr = 0xa0;
		}
	}else if (!strcmp(*(av + 1), "port1")){
		printf("Enable port1 TX pre-emphasis driver swing\n");
		if(!strcmp(*(av + 2), "1.5G")) {
			addr = 0x8c;
		}else{
			addr = 0xa2;
		}
	}else if (!strcmp(*(av + 1), "port2")){
		printf("Enable port2 TX pre-emphasis driver swing\n");
		if(!strcmp(*(av + 2), "1.5G")) {
			addr = 0x90;
		}else{
			addr = 0xa4;
		}
	}else if (!strcmp(*(av + 1), "port3")){
		printf("Enable port3 TX pre-emphasis driver swing\n");
		if(!strcmp(*(av + 2), "1.5G")) {
			addr = 0x94;
		}else{
			addr = 0xa6;
		}
	}else if (!strcmp(*(av + 1), "port4")){
		printf("Enable port4 TX pre-emphasis driver swing\n");
		if(!strcmp(*(av + 2), "1.5G")) {
			addr = 0x98;
		}else{
			addr = 0xa8;
		}
	}else if (!strcmp(*(av + 1), "port5")){
		printf("Enable port5 TX pre-emphasis driver swing\n");
		if(!strcmp(*(av + 2), "1.5G")) {
			addr = 0x9c;
		}else{
			addr = 0xaa;
		}
	}else
		printf("Please choose correct test port\n");

	word = pci_read_config16(dev, addr);
	word |= 0x01 << 13;
	pci_write_config16(dev, addr, word);

	output = pci_read_config16(dev, addr);
	output &= ~(0x7 << 5);

	if(!strcmp(*(av + 3), "1")) {
		printf("Driver Nominal Output 0mv.\n");
		output |= (0x0 << 5) ; /* 0mv */
	}else if(!strcmp(*(av + 3), "2")) {
		printf("Driver Nominal Output 25mv.\n");
		output |= (0x1 << 5) ; /* 25mv */
	}else if(!strcmp(*(av + 3), "3")) {
		printf("Driver Nominal Output 50mv.\n");
		output |= (0x2 << 5) ; /* 50mv */
	}else if(!strcmp(*(av + 3), "4")) {
		printf("Driver Nominal Output 75mv.\n");
		output |= (0x3 << 5) ; /* 75mv */
	}else if(!strcmp(*(av + 3), "5")) {
		printf("Driver Nominal Output 100mv.\n");
		output |= (0x4 << 5) ; /* 100mv */
	}else if(!strcmp(*(av + 3), "6")) {
		printf("Driver Nominal Output 125mv.\n");
		output |= (0x5 << 5) ; /* 125mv */
	}else if(!strcmp(*(av + 3), "7")) {
		printf("Driver Nominal Output 150mv.\n");
		output |= (0x6 << 5) ; /* 150mv */
	}else if(!strcmp(*(av + 3), "8")) {
		printf("Driver Nominal Output 175mv.\n");
		output |= (0x7 << 5) ; /* 175mv */
	}else
		printf("Please choose correct Nominal Output [ 1/2/3/4/5/6/7/8]\n");

	pci_write_config16(dev, addr, output);

	return 0;

}
#endif

static const Cmd Cmds[] =
{
	{"Motherboard Signal Quality Test (msqt)"},
	{"msqt_sata",	"port[0/1/2/3/4/5] pattern[align/D10.2/sync/lbp/mftp/20bit/fferlb/Tmode] speed[ 1.5G/3G ] Nominal Output[ 1/2/3/4/5/6/7/8 ]", 0,
		"Motherboard Signal Quality Test for SATA", cmd_msqt_sata, 5, 5, 0},
	{"msqt_sata1",	"port[0/1/2/3/4/5]  speed[ 1.5G/3G ] Nominal Output[ 1/2/3/4/5/6/7/8 ]", 0,
		"Motherboard Signal Quality Test for SATA TX pre-emphasis driver swing", cmd_msqt_sata1, 4, 4, 0},
	{0, 0}
};


static void init_cmd __P((void)) __attribute__ ((constructor));

	static void
init_cmd()
{
	cmdlist_expand(Cmds, 1);
}

