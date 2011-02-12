/*	$Id: memcmds.c,v 1.1.1.1 2006/09/14 01:59:08 root Exp $ */

/*
 * Copyright (c) 2000-2001 Opsycon AB  (www.opsycon.se)
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

#include <pmon.h>


//int	cmd_install  __P((void)); 

void cmd_cdinstall(void)
{

	char buf[100];

	sprintf(buf, "load /dev/fs/iso9660@cd0/vmlinuxb");
	//buf = "load /dev/fs/iso9660@cd0/vmlinuxb";
	do_cmd(buf);
	sprintf(buf, "g console=tty rdinit=/sbin/init video=vfb:1");
	do_cmd(buf);
}
void cmd_usbcdinstall(void)
{

	char buf[100];

	sprintf(buf, "load /dev/fs/iso9660@usb0/vmlinuxboot");
	do_cmd(buf);
	sprintf(buf, "g console=tty rdinit=/sbin/init video=vfb:1");
	do_cmd(buf);
}

void cmd_usbinstall(void)
{

	char buf[100];

	sprintf(buf, "load /dev/fs/ext2@usb0/vmlinuxboot");
	do_cmd(buf);
	sprintf(buf, "g console=tty rdinit=/sbin/init video=vfb:1");
	do_cmd(buf);
}

static const Cmd Cmds[] =
{
	{"Misc"},
	{"cdinstall", "",0,"install Linux system from CD-ROM", cmd_cdinstall, 1, 99, 0},
	{"usbinstall", "",0,"install Linux system from USB DISK", cmd_usbinstall, 1, 99, 0},
	{"usbcdinstall", "",0,"install Linux system from USB CD-ROM", cmd_usbcdinstall, 1, 99, 0},
	{0, 0}
};

static void init_cmd __P((void)) __attribute__ ((constructor));

static void
init_cmd()
{
	cmdlist_expand(Cmds, 1);
}
