/*	$Id: devls.c,v 1.1.1.1 2006/09/14 01:59:08 root Exp $ */

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

/*
 *  List known devices.
 */

void lxf_1a_mem_test(void)
{
     unsigned char *ls1a_mem_base = (unsigned char *)0xb4000000;
     int i,j;
     
     for(j = 0;j < 1000;j++){
	     for(i = 0;i < 10000000;i++){
		     *(ls1a_mem_base + i) = i;
	     }

	     for(i = 0;i < 10000000;i++){
		     if(*(ls1a_mem_base + i) != i){
			     printf("test error !!!! i:%d,date:%d\n",i,*(ls1a_mem_base + i));
			     return;
		     }
	     }

	     for(i = 0;i < 10000000;i++){
		     *(ls1a_mem_base + i) = 0x55;
	     }
     }
}



int cmd_lxftest __P((int, char *[]));


int
cmd_lxftest(ac, av)
	int ac;
	char *av[];
{
	lxf_1a_mem_test();
	return(1);
}

/*
 *  Command table registration
 *  ==========================
 */

const Optdesc cmd_lxftest_opts[] = {
	{ "-a",	"show all device types" },
	{ "-n",	"show network devices" },
	{ 0 }
};

static const Cmd Cmds[] =
{
	{"Misc"},
	{"lxftest",	"[-an]",
			cmd_lxftest_opts,
			"list devices",
			cmd_lxftest, 1, 99,
			0},
	{0, 0}
};

static void init_cmd __P((void)) __attribute__ ((constructor));

void
init_cmd()
{
	cmdlist_expand(Cmds, 1);
}
