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
char *elf_addr = 0x80205c00; //huang
char *myversion = "2.6.36";//huang
int flag = 0; //huang
int judge_kernel_version(char *src, char *dest)
{
	char *p = src;
	char *tmp = "version ";
	char *q = tmp;
	int i = 20;
	char *ver;
	while(*tmp != '\0')
	{
		if(*src++ != *tmp++)
		{
			src = ++p;
			tmp = q;	
		}
	}
	q = ver;
	while(i-- >= 0)
	*ver++ = *p++;	
	*ver = '\0';
	ver  = strstr(q, dest);
	if(ver == NULL)
	return 0;

	return 1;	

}
void cmd_cdinstall(void)
{

	char buf[100];
	int ret = 0;
	char *rd;
	int flag = 0; //huang
	sprintf(buf, "load /dev/fs/iso9660@cd0/vmlinuxb");
	//buf = "load /dev/fs/iso9660@cd0/vmlinuxb";
	ret = do_cmd(buf);
	
	if (ret != 0)
		return;

	flag = judge_kernel_version(elf_addr, myversion);

	if(flag == 1)
{
	rd= getenv("rdinit");
        if (rd != 0) // set rd /sbin/init
          sprintf(buf, "g console=tty rdinit=%s ", rd);
        else
          sprintf(buf, "g console=tty rdinit=/sbin/init ");

        do_cmd(buf);
}
	else
{	
	rd= getenv("rdinit");
	if (rd != 0) // set rd /sbin/init
	  sprintf(buf, "g console=tty rdinit=%s video=vfb:1 ", rd);
	else
	  sprintf(buf, "g console=tty rdinit=/sbin/init video=vfb:1 ");

	do_cmd(buf);
}
}

void cmd_usbcdinstall(void)
{

	char buf[100];
	char *rd;
	int ret = 0;

	sprintf(buf, "load /dev/fs/iso9660@usb0/vmlinuxboot");
	ret = do_cmd(buf);
	if (ret != 0)
		return;
 flag = judge_kernel_version(elf_addr, myversion);

        if(flag == 1)
{
        rd= getenv("rdinit");
        if (rd != 0) // set rd /sbin/init
          sprintf(buf, "g console=tty rdinit=%s ", rd);
        else
          sprintf(buf, "g console=tty rdinit=/sbin/init ");

        do_cmd(buf);
}
        else
{
        rd= getenv("rdinit");
        if (rd != 0) // set rd /sbin/init
          sprintf(buf, "g console=tty rdinit=%s video=vfb:1 ", rd);
        else
          sprintf(buf, "g console=tty rdinit=/sbin/init video=vfb:1 ");

        do_cmd(buf);
}
}

void cmd_usbinstall(void)
{

	char buf[100];
	int ret = 0;
	char *rd;

	/* first try ext2 file system format */
	sprintf(buf, "load /dev/fs/ext2@usb0/vmlinuxboot");
	ret = do_cmd(buf);
	if (ret != 0)
	{
	  /* then try fat file system format */
	  sprintf(buf, "load /dev/fs/fat@usb0/vmlinuxboot");
	  ret = do_cmd(buf);
	}
	if (ret != 0)
		return;
 flag = judge_kernel_version(elf_addr, myversion);

        if(flag == 1)
{
        rd= getenv("rdinit");
        if (rd != 0) // set rd /sbin/init
          sprintf(buf, "g console=tty rdinit=%s ", rd);
        else
          sprintf(buf, "g console=tty rdinit=/sbin/init ");

        do_cmd(buf);
}
        else
{
        rd= getenv("rdinit");
        if (rd != 0) // set rd /sbin/init
          sprintf(buf, "g console=tty rdinit=%s video=vfb:1 ", rd);
        else
          sprintf(buf, "g console=tty rdinit=/sbin/init video=vfb:1 ");

        do_cmd(buf);
}
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
