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

#include <machine/pio.h>

#include <pmon.h>

#define INDEX_OFF	0x5
#define DATA_OFF	0x6

#define HM_OFF 0x0290

/* use CPUFANIN0 */
int cpufanspeed(void)
{
  int counter, div_b1_b0, div_b2, div, speed;

  outb(BONITO_PCIIO_BASE_VA + HM_OFF + INDEX_OFF, 0x29);
  counter = inb(BONITO_PCIIO_BASE_VA + HM_OFF + DATA_OFF);

  outb(BONITO_PCIIO_BASE_VA + HM_OFF + INDEX_OFF, 0x47);
  div_b1_b0 = inb(BONITO_PCIIO_BASE_VA + HM_OFF + DATA_OFF) >> 0x6;
  printf("Orignal div_b1_b0  : %08x\n", div_b1_b0);
  div_b1_b0 = 0x3 << 0x6;
  outb(BONITO_PCIIO_BASE_VA + HM_OFF + DATA_OFF, div_b1_b0);
  div_b1_b0 = inb(BONITO_PCIIO_BASE_VA + HM_OFF + DATA_OFF) >> 0x6;
  printf("Now div_b1_b0  : %08x\n", div_b1_b0);

  outb(BONITO_PCIIO_BASE_VA + HM_OFF + INDEX_OFF, 0x5D);
  div_b2 = (inb(BONITO_PCIIO_BASE_VA + HM_OFF + DATA_OFF) & 0x40) >> 0x4;
  printf("Orignal div_b2  : %08x\n", div_b2);
  div_b2 = 0x1 << 0x6;
  div_b2 = outb(BONITO_PCIIO_BASE_VA + HM_OFF + DATA_OFF, div_b2);
  div_b2 = (inb(BONITO_PCIIO_BASE_VA + HM_OFF + DATA_OFF) & 0x40) >> 0x4;
  printf("Now div_b2  : %08x\n", div_b2); // shoude be 0x4 //3'b111

  div = 1 << (div_b2 | div_b1_b0);


  speed = 1350000ull / counter / div; 

  return speed;
}

/* use  CPUFANIN0 */
unsigned int
cmd_cpufanspeed(ac, av)
	int ac;
	char *av[];
{
	unsigned int dev, reg, ret;

	if (ac > 1)
	  printf("Usage:cpufanspeed \n");

	ret = cpufanspeed();

	printf("CPU Fan speed: %dRPM\n", ret);

	return(1);
}


/* use SYSFANIN */
int sysfanspeed(void)
{
  int counter, div_b1_b0, div_b2, div, speed;

  outb(BONITO_PCIIO_BASE_VA + HM_OFF + INDEX_OFF, 0x28);
  counter = inb(BONITO_PCIIO_BASE_VA + HM_OFF + DATA_OFF);

#if 0 // SYSFAN counter limit reg
  outb(BONITO_PCIIO_BASE_VA + HM_OFF + INDEX_OFF, 0x28);
  counter = inb(BONITO_PCIIO_BASE_VA + HM_OFF + DATA_OFF);
#endif

  // index 47h: bit[5:4]
  outb(BONITO_PCIIO_BASE_VA + HM_OFF + INDEX_OFF, 0x47);
  div_b1_b0 = inb(BONITO_PCIIO_BASE_VA + HM_OFF + DATA_OFF) >> 0x4;
  printf("Orignal div_b1_b0  : %08x\n", div_b1_b0);
  div_b1_b0 = 0x3 << 0x4;
  outb(BONITO_PCIIO_BASE_VA + HM_OFF + DATA_OFF, div_b1_b0);
  div_b1_b0 = inb(BONITO_PCIIO_BASE_VA + HM_OFF + DATA_OFF) >> 0x4;
  printf("Now div_b1_b0  : %08x\n", div_b1_b0);

  // index 5D: bit bit[5]
  outb(BONITO_PCIIO_BASE_VA + HM_OFF + INDEX_OFF, 0x5D);
  div_b2 = (inb(BONITO_PCIIO_BASE_VA + HM_OFF + DATA_OFF) & 0x20) >> 0x3;
  printf("Orignal div_b2  : %08x\n", div_b2);
  div_b2 = 0x1 << 0x5;
  outb(BONITO_PCIIO_BASE_VA + HM_OFF + DATA_OFF, div_b2);
  div_b2 = (inb(BONITO_PCIIO_BASE_VA + HM_OFF + DATA_OFF) & 0x20) >> 0x3;
  printf("Now div_b2  : %08x\n", div_b2); // shoude be 0x4 //3'b111

  div = 1 << (div_b2 | div_b1_b0);


  speed = 1350000ull / counter / div; 

  return speed;
}

/* use SYSFANIN */
unsigned int
cmd_sysfanspeed(ac, av)
	int ac;
	char *av[];
{
	unsigned int dev, reg, ret;

	if (ac > 1)
	  printf("Usage:sysfanspeed \n");

	ret = sysfanspeed();

	printf("Sys Fan speed: %dRPM\n", ret);

	return(1);
}


/* use SYSFANOUT as PWM mode, test good */
int setfanpwm(int duty)
{
  int ret;

  // set SYSFANOUT PWM  mode
#if 1
  outb(BONITO_PCIIO_BASE_VA + HM_OFF + INDEX_OFF, 0x04);
  ret = inb(BONITO_PCIIO_BASE_VA + HM_OFF + DATA_OFF);
  printf("Original SYSFANOUT mode is: %d\n", ret);
  ret &= 0xfe; 
  outb(BONITO_PCIIO_BASE_VA + HM_OFF + DATA_OFF, ret);
  ret = inb(BONITO_PCIIO_BASE_VA + HM_OFF + DATA_OFF);
  printf("Now SYSFANOUT mode is: %d\n", ret);
#endif  // default PWM mode
  
  // set SYSFANOUT counter
  outb(BONITO_PCIIO_BASE_VA + HM_OFF + INDEX_OFF, 0x01);
  outb(BONITO_PCIIO_BASE_VA + HM_OFF + DATA_OFF, duty);
  ret = inb(BONITO_PCIIO_BASE_VA + HM_OFF + DATA_OFF);

  return ret;
  
}


int setfandc(int level)
{
  int counter, div_b1_b0, div_b2, div, speed;

  //set fan spped controled by DC not PWM

  // set CPUFANOUT0 and SYSFANOUT DC mode
  outb(BONITO_PCIIO_BASE_VA + HM_OFF + INDEX_OFF, 0x04);
  counter = inb(BONITO_PCIIO_BASE_VA + HM_OFF + DATA_OFF);
  printf("CPUFANOUT0 and SYSFANOUT0 mode: %d\n", counter);
  counter |= 0x3; 
  outb(BONITO_PCIIO_BASE_VA + HM_OFF + DATA_OFF, counter);

  // set CPUFANOUT1 DC mode
  outb(BONITO_PCIIO_BASE_VA + HM_OFF + INDEX_OFF, 0x62);
  counter = inb(BONITO_PCIIO_BASE_VA + HM_OFF + DATA_OFF);
  counter |= 0x1 << 6; 
  outb(BONITO_PCIIO_BASE_VA + HM_OFF + DATA_OFF, counter);

  // set SYSFANOUT counter
  outb(BONITO_PCIIO_BASE_VA + HM_OFF + INDEX_OFF, 0x01);
  outb(BONITO_PCIIO_BASE_VA + HM_OFF + DATA_OFF, level << 0x2);

  // set CPUFANOUT0 counter
  outb(BONITO_PCIIO_BASE_VA + HM_OFF + INDEX_OFF, 0x03);
  outb(BONITO_PCIIO_BASE_VA + HM_OFF + DATA_OFF, level << 0x2);

  // set CPUFANOUT1 counter
  outb(BONITO_PCIIO_BASE_VA + HM_OFF + INDEX_OFF, 0x61);
  outb(BONITO_PCIIO_BASE_VA + HM_OFF + DATA_OFF, level << 0x2);
  counter = inb(BONITO_PCIIO_BASE_VA + HM_OFF + DATA_OFF);
  counter >>= 0x2;


  return counter;
  
}

#define DEFAULT_LEVEL 64/2
#define DEFAULT_DUTY 0x8f

unsigned int
cmd_setfandc(ac, av)
	int ac;
	char *av[];
{
	unsigned int dev, reg, ret;
	unsigned level = DEFAULT_LEVEL;

	if (ac > 2)
	  printf("Usage:setfanspeed \n");
	if (ac == 2)
	  level = atoi(av[1]);

	printf("Set DC level: %d\n", level);
	ret = setfandc(level);

	printf("Set Fan speed with DC level: %dRPM\n", ret);

	return(1);
}

unsigned int
cmd_setfanpwm(ac, av)
	int ac;
	char *av[];
{
	unsigned int dev, reg, ret;
	unsigned duty = DEFAULT_LEVEL;

	if (ac > 2)
	  printf("Usage:setfanpwm  duty(0 ~255)\n");
	if (ac == 2)
	  duty = atoi(av[1]);

	printf("Set pwm duty : %d\n", duty);
	ret = setfanpwm(duty);

	printf("Set Fan speed with pwm mode: %d\n", ret);

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
	//{"cpufanspeed",	"", 0, "Get fan speed for board with W83527 chip", cmd_cpufanspeed, 1, 99, 0},
	{"sysfanspeed",	"", 0, "Get fan speed for board with W83527 chip", cmd_sysfanspeed, 1, 99, 0},
	{"setfanpwm",	"", 0, "Set fan DC for board with W83527 chip", cmd_setfanpwm, 1, 99, 0},
	//{"setfandc",	"", 0, "Set fan DC for board with W83527 chip", cmd_setfandc, 1, 99, 0},
	{0, 0}
};

static void init_cmd __P((void)) __attribute__ ((constructor));

void
init_cmd()
{
	cmdlist_expand(Cmds, 1);
}
