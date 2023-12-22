/* $Id: main.c,v 1.1.1.1 2006/09/14 01:59:08 root Exp $ */

/*
 * Copyright (c) 2001-2002 Opsycon AB  (www.opsycon.se / www.opsycon.com)
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
 *      This product includes software developed by Opsycon AB, Sweden.
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
 *  This code was created from code released to Public Domain
 *  by LSI Logic and Algorithmics UK.
 */ 

#include <stdio.h>
#include <string.h>
#include <machine/pio.h>
#include <pmon.h>
#include <termio.h>
#include <endian.h>
#include <string.h>
#include <signal.h>
#include <setjmp.h>
#include <ctype.h>
#include <unistd.h>
#include <stdlib.h>
#ifdef _KERNEL
#undef _KERNEL
#include <sys/ioctl.h>
#define _KERNEL
#else
#include <sys/ioctl.h>
#endif
#include <pmon.h>
#include <exec.h>
#include <file.h>

#include <sys/device.h>
#include "mod_debugger.h"
#include "mod_symbols.h"

#include "sd.h"
#include "wd.h"
#include "../cmds/cmd_main/window.h"
#include "../cmds/cmd_main/cmd_main.h"

#include <pflash.h>
#include <flash.h>
#include <dev/pflash_tgt.h>
#include <mod_usb_ohci.h>
extern void    *callvec;
unsigned int show_menu;

#include "cmd_hist.h"		/* Test if command history selected */
#include "cmd_more.h"		/* Test if more command is selected */

#include "smbios/smbios.h"
#include "../cmds/bootparam.h"

void print_mem_freq(void);
extern int bios_available;
extern int cmd_main_mutex;
extern int ohci_index;
extern int dl_ohci_kbd(void);

jmp_buf         jmpb;		/* non-local goto jump buffer */
char            line[LINESZ + 1];	/* input line */
struct termio	clntterm;	/* client terminal mode */
struct termio	consterm;	/* console terminal mode */
register_t	initial_sr;
unsigned long long             memorysize = 0;
unsigned long long             memorysize_high = 0;
#ifdef MULTI_CHIP
unsigned long long             memorysize_high_n1 = 0;
unsigned long long             memorysize_high_n2 = 0;
unsigned long long             memorysize_high_n3 = 0;
#endif
unsigned long long	       memorysize_total = 0;

char            prnbuf[LINESZ + 8];	/* commonly used print buffer */

int             repeating_cmd;
unsigned int  	moresz = 10;
#ifdef AUTOLOAD
static int autoload __P((char *));
#else
static void autorun __P((char *));
#endif
extern void __init __P((void));
extern void _exit (int retval);
extern void delay __P((int));

#ifdef INET
static void
pmon_intr (int dummy)
{
    sigsetmask (0);
    longjmp (jmpb, 1);
}
#endif

/*FILE *logfp; = stdout; */

#if NCMD_HIST == 0
void
get_line(char *line, int how)
{
	int i;

	i = read (STDIN, line, LINESZ);
	if(i > 0) {
		i--;
	}
	line[i] = '\0';
}
#endif

int check_user_password()
{
	char buf[50];
	struct termio tty;
	int i;
	char c;
	if(!pwd_exist()||!pwd_is_set("user"))
		return 0;

	for(i=0;i<2;i++)
	{
	ioctl(i,TCGETA,&tty);
	tty.c_lflag &= ~ ECHO;
	ioctl(i,TCSETAW,&tty);
	}


	printf("\nPlease input user password:");
loop0:
	for(i= 0;i<50;i++)
	{
		c=getchar();
		if(c!='\n'&&c!='\r'){	
			printf("*");
			buf[i] = c;
		}
		else
		{
			buf[i]='\0';
			break;
		}
	}
	
	if(!pwd_cmp("user",buf))
	{
		printf("\nPassword error!\n");
		printf("Please input user password:");
		goto loop0;
	}

	for(i=0;i<2;i++)
	{
	tty.c_lflag |=  ECHO;
	ioctl(i,TCSETAW,&tty);
	}
			
	return 0;
}

int check_admin_password()
{
	char buf[50];
	struct termio tty;
	int i;
	char c;
	if(!pwd_exist()||!pwd_is_set("admin"))
		return 0;

	for(i=0;i<2;i++)
	{
	ioctl(i,TCGETA,&tty);
	tty.c_lflag &= ~ ECHO;
	ioctl(i,TCSETAW,&tty);
	}


	printf("\nPlease input admin password:");
loop1:
	for(i= 0;i<50;i++)
	{
		c=getchar();
		if(c!='\n'&&c!='\r'){	
			printf("*");
			buf[i] = c;
		}
		else
		{
			buf[i]='\0';
			break;
		}
	}
	
	if(!pwd_cmp("admin",buf))
	{
		printf("\nPassword error!\n");
		printf("Please input admin password:");
		goto loop1;
	}


	for(i=0;i<2;i++)
	{
	tty.c_lflag |=  ECHO;
	ioctl(i,TCSETAW,&tty);
	}
	
	return 0;
}


int check_sys_password()
{
	char buf[50];
	struct termio tty;
	int i;
	char c;
	int count=0;
	if(!pwd_exist()||!pwd_is_set("sys"))
		return 0;

	for(i=0;i<6;i++)
	{
	ioctl(i,TCGETA,&tty);
	tty.c_lflag &= ~ ECHO;
	ioctl(i,TCSETAW,&tty);
	}


	printf("\nPlease input sys password:");
loop1:
	for(i= 0;i<50;i++)
	{
		c=getchar();
		if(c!='\n'&&c!='\r'){	
			printf("*");
			buf[i] = c;
		}
		else
		{
			buf[i]='\0';
			break;
		}
	}
	
	if(!pwd_cmp("sys",buf))
	{
		printf("\nPassword error!\n");
		printf("Please input sys password:");
		count++;
		if(count==3)
			return -1;
		goto loop1;
	}


	for(i=0;i<6;i++)
	{
	tty.c_lflag |=  ECHO;
	ioctl(i,TCSETAW,&tty);
	}
	
	return 0;
}

/*
 *  Main interactive command loop
 *  -----------------------------
 *
 *  Clean up removing breakpoints etc and enter main command loop.
 *  Read commands from the keyboard and execute. When executing a
 *  command this is where <crtl-c> takes us back.
 */
void __gccmain(void);
void __gccmain(void)
{
}

static void test_1a_led(void)
{//led:gpio56,57,58
	unsigned int *gpiocfg1 = (volatile unsigned int *)0xb2d010c4;//gpio632~gpio63 1:gpio   0:normal 
	unsigned int *gpiooe1 = (volatile unsigned int *)0xb2d010d4;//gpio632~gpio63  1:input  0:output
	unsigned int *gpiooutput1 = (volatile unsigned int *)0xb2d010f4;//gpio632~gpio63  1:high  0:low
	int i,j;

	*gpiocfg1 |= ((1<<24)|(1<<26)); 
	//*gpiocfg1 |= ((1<<24)|(1<<25)|(1<<26)); 
	*gpiooe1 &= (~((1<<24)|(1<<26)));
	//*gpiooe1 &= (~((1<<24)|(1<<25)|(1<<26)));
	//while(1){
	//	i = 0x5000000;
	//	j = 0x5000000;
	//	*gpiooutput1 |= ((1<<24)|(1<<25)|(1<<26));
	//	while(i--);
		*gpiooutput1 &= (~((1<<24)|(1<<26)));
		//*gpiooutput1 &= (~((1<<24)|(1<<25)|(1<<26)));
	//	while(j--);
		//printf("~~~~~~~~~~~~~~~~~~~~~~~~~ gpio test for 1a \n");
	//}
}

int
main()
{
	char prompt[32];
	int i;
#if defined(ARB_LEVEL) || defined(VREF_STORE)
	save_board_ddrparam(0);
#endif
#ifdef CONFIG_VIDEO_SPLASH
	video_enableoutput();
#endif
	if(cmd_main_mutex == 2)
		;
	else {
		unsigned char *bootmenu_envstr;
		if((bootmenu_envstr = getenv("ShowBootMenu")) && (strcmp("no",bootmenu_envstr) == 0))
			;
		else {
			bios_available = 1;//support usb_kbd in bios
			tty_flush();
			do_cmd("bootdev_sel");
			bios_available = 0;
		}
	}
	if (setjmp(jmpb)) {
		/* Bailing out, restore */
		closelst(0);
		ioctl(STDIN, TCSETAF, &consterm);
		printf(" break!\r\n");
	}

#ifdef INET
	signal (SIGINT, pmon_intr);
#else
	ioctl (STDIN, SETINTR, jmpb);
#endif

#if NMOD_DEBUGGER > 0
	rm_bpts();
#endif

	{
		static int run=0;
		char *s;
		int ret = -1;
		char buf[LINESZ];
		if(!run)
		{
			run=1;
#ifdef AUTOLOAD

			if(getenv("FR") == NULL)
			{
				setenv("FR","0");
				setenv("installdelay", "5");
				//setenv("autoinstall", "/dev/fs/iso9660@cd0/vmlinuxb");
				setenv("autoinstall", "/dev/fs/ext2@usb0/vmlinuxboot");
				//setenv("rd", "/sbin/init");
				//autoinstall("/dev/fs/iso9660@cd0/vmlinuxb");
				//autoinstall("/dev/fs/ext2@usb0/vmlinuxboot");
			}
			if (strcmp (getenv("FR"),"0") == 0) {
				unsetenv("al");
				unsetenv("al1");
				unsetenv("append");
				printf("==WARN: First Run\n==WARN: Setup the default boot configure\n");
				setenv("FR", "1");
			}
			//autoinstall("/dev/fs/iso9660@cd0/vmlinuxb");
			//autoinstall("/dev/fs/iso9660@cd0/vmlinuxb");
			//autoinstall("/dev/fs/ext2@usb0/vmlinuxboot");

			if(getenv("al") == NULL) /* CDROM autoload */
			{
				setenv("al","/dev/fs/iso9600@cd0/boot/vmlinux");
				setenv("append","console=tty root=/dev/sda1");
			}
			if (getenv("al1") == NULL) { /* HARDDISK autoload */
				setenv("al1","/dev/fs/ext2@wd0/boot/vmlinux");
				setenv("append","console=tty root=/dev/sda1");
			}

			//autoload("/dev/fs/ext2@wd0/boot/vmlinux");
			//autorun("g console=tty root=/dev/sda1 rw");
			//cmd_showwindows();
			//do_cmd("load /dev/fs/ext2@wd0/boot/vmlinux"); 
			//do_cmd("g console=tty root=/dev/sda1"); 
#if 1
			s = getenv ("al1");
			ret = autoload (s);
			if (ret == 1) {
				s = getenv("al");
				ret = autoload (s);
			}
#endif

#else
			s = getenv ("autoboot");
			autorun (s);
#endif
		}
	}

//#endif	
	while(1) {
#if 0
		while(1){char c;int i;
			i=term_read(0,&c,1);
			printf("haha:%d,%02x\n",i,c);
		}
#endif		
		strncpy (prompt, getenv ("prompt"), sizeof(prompt));

#if NCMD_HIST > 0
		if (strchr(prompt, '!') != 0) {
			char tmp[8], *p;
			p = strchr(prompt, '!');
			strdchr(p);	/* delete the bang */
			sprintf(tmp, "%d", histno);
			stristr(p, tmp);
		}
#endif

#ifndef	LOONGSON_2G5536
                if(cmd_main_mutex == 2) {
                        cmd_main_mutex = 1;
                        printf(" break!\r\n");
                }
#endif
		
		printf("%s", prompt);
#if NCMD_HIST > 0
		get_cmd(line);
#else
		get_line(line, 0);
#endif
		do_cmd(line);
		console_state(1);
	}
	DeviceRelease();
	return(0);
}

#ifdef AUTOLOAD
static int autoload(char *s)
{
	char buf[LINESZ];
	char *pa;
	char *rd;
	unsigned int dly, lastt;
	unsigned int cnt;
	struct termio sav;
	int ret = -1;
	int i;

	if(s != NULL  && strlen(s) != 0) {
		char *d = getenv ("bootdelay");
		if(!d || !atob (&dly, d, 10) || dly < 0 || dly > 99) {
			dly = 8;
		}

		SBD_DISPLAY ("AUTO", CHKPNT_AUTO);
		printf("Press <Enter> to execute loading image:%s\n",s);
		printf("Press any other key to abort.\n");
		ioctl (STDIN, CBREAK, &sav);
		lastt = 0;
		if (!dly) ioctl (STDIN, FIONREAD, &cnt);
		else
		do {
	//		delay(1000000);
			printf ("\b\b%02d", --dly);
			//printf (".", --dly);
			for(i = 0; i < 9000; i++){
				ioctl (STDIN, FIONREAD, &cnt);
				if(cnt)
				      break;
				delay(100);
			}
		} while (dly != 0 && cnt == 0);

		if(cnt > 0 && strchr("\n\r", getchar())) {
			cnt = 0;
		}

		ioctl (STDIN, TCSETAF, &sav);
		putchar ('\n');

		if(cnt == 0) {
	        if(getenv("autocmd"))
			{
			strcpy(buf,getenv("autocmd"));
			do_cmd(buf);
			}
			rd= getenv("rd");
			if (rd != 0){
				sprintf(buf, "initrd %s", rd);
				if(do_cmd(buf))return;
			}

			strcpy(buf,"load ");
			strcat(buf,s);
			if(do_cmd(buf))return;
			if((pa=getenv("append")))
			{
			sprintf(buf,"g %s",pa);
			}
			else if((pa=getenv("karg")))
			{
			sprintf(buf,"g %s",pa);
			}
			else
			{
			pa=getenv("dev");
			strcpy(buf,"g root=/dev/");
			if(pa != NULL  && strlen(pa) != 0) strcat(buf,pa);
			else strcat(buf,"hda1");
			//strcat(buf," console=tty");
			strcat(buf," console=ttyS0,115200 init=/bin/sh rw");
			}
			printf("%s\n",buf);
			delay(100000);
			ret = do_cmd (buf);
		}
	}

	return ret;
}

#else
/*
 *  Handle autoboot execution
 *  -------------------------
 *
 *  Autoboot variable set. Countdown bootdelay to allow manual
 *  intervention. If CR is pressed skip counting. If var bootdelay
 *  is set use the value othervise default to 15 seconds.
 */
static void
autorun(char *s)
{
	char buf[LINESZ];
	char *d;
	unsigned int dly, lastt;
	unsigned int cnt;
	struct termio sav;

	if(s != NULL  && strlen(s) != 0) {
		d = getenv ("bootdelay");
		if(!d || !atob (&dly, d, 10) || dly < 0 || dly > 99) {
			dly = 15;
		}

		SBD_DISPLAY ("AUTO", CHKPNT_AUTO);
		printf("Autoboot command: \"%.60s\"\n", s);
		printf("Press <Enter> to execute or any other key to abort.\n");
		ioctl (STDIN, CBREAK, &sav);
		lastt = 0;
		dly++;
		do {
#if defined(HAVE_TOD) && defined(DELAY_INACURATE)
			time_t t;
			t = tgt_gettime ();
			if(t != lastt) {
				printf ("\r%2d", --dly);
				lastt = t;
			}
#else
			delay(1000000);
			printf ("\r%2d", --dly);
#endif
			ioctl (STDIN, FIONREAD, &cnt);
		} while (dly != 0 && cnt == 0);

		if(cnt > 0 && strchr("\n\r", getchar())) {
			cnt = 0;
		}

		ioctl (STDIN, TCSETAF, &sav);
		putchar ('\n');

		if(cnt == 0) {
			strcpy (buf, s);
			do_cmd (buf);
		}
	}
}
#endif
static int
autoinstall(char *s)
{
	char buf[LINESZ];
	char *pa;
	char *rd;
	unsigned int dly, lastt;
	unsigned int cnt = 0;
	//unsigned int cnt ;
	struct termio sav;
	int ret = -1;
	char c;

	  if(s != NULL  && strlen(s) != 0) {
		char *d = getenv ("installdelay");
		if(!d || !atob (&dly, d, 10) || dly < 0 || dly > 99) {
			dly = 6;
		}

		SBD_DISPLAY ("AUTO", CHKPNT_AUTO);
		//printf("Press <F2> to execute system installing :%s\n",s);
		//printf("Press <Enter> to execute loading image:%s\n",s);
		printf("Press 'u(usb)' or 'c(cdrom)' to  install image from usb or cdrom:%s\n",s);
		printf("Press any other key to abort.\n");
		ioctl (STDIN, CBREAK, &sav);
		lastt = 0;
		do {
			delay(1000000);
			printf ("\b\b%02d", --dly);
			//printf (".", --dly);
			ioctl (STDIN, FIONREAD, &cnt);
		 } while (dly != 0 && cnt == 0);
		//} while (dly != 0);

		//if(cnt > 0! strchr("\0x71", getchar())) {
		//if(cnt > 0 && strchr("\0x71\0x72\0x73", getchar())) {
		//if( cnt > 0 ) {
		if(cnt > 0 && strchr("uUcC", c=getchar())) {
		//if(cnt > 0 && (c=getchar())) {
		//if(cnt > 0) {
		 // printf("cnt = %d: input = 0x%08x\n", cnt, getchar());
		  //printf("input = 0x%08x\n", c=getchar());
		  //putchar();
		  ioctl (STDIN, TCSETAF, &sav);
		  putchar ('\n');

#if 0
		  if(getenv("autoinstall"))
			  sprintf(buf, "load %s", getenv("autoinstall"));
		  else
			  sprintf(buf, "load /dev/fs/iso9660@cd0/vmlinuxb");
#else
		  if(c == 'u' || c == 'U')
		  {
			  //sprintf(buf, "load %s", getenv("autoinstall"));
			  //sprintf(buf, "load /dev/fs/ext2@usb0/vmlinuxboot");
			  sprintf(buf, "usbinstall");
			  ret = do_cmd(buf);
		  }
		  if(c == 'c' || c == 'C')
		  {
			  sprintf(buf, "cdinstall");
			  ret = do_cmd(buf);
		  }
#endif
	}
//	printf( "\n NO <F2> entered in 5 secs\n");
  }
  return ret;
}

extern void get_ec_version(void);
extern int nomsg_on_serial;
/*
 *  PMON2000 entrypoint. Called after initial setup.
 */
void
dbginit (char *adr)
{
	unsigned long long	memsize;
	char	*s;

/*	splhigh();*/

	memsize = memorysize;

	__init();	/* Do all constructor initialisation */

#ifdef CONFIG_VIDEO_SPLASH
	video_disableoutput();
#endif

	SBD_DISPLAY ("ENVI", CHKPNT_ENVI);
	envinit ();
	{
	char *s = getenv("NOMSG_ON_SERIAL");
	if(s && s[0]!='0') nomsg_on_serial=1;
	}

#if defined(SMP)
	/* Turn on caches unless opted out */
	if (!getenv("nocache"))
		md_cacheon();
#endif

	SBD_DISPLAY ("SBDD", CHKPNT_SBDD);
	tgt_devinit();
#if defined(LS3A7A_STR) || defined(LS3A2H_STR) || defined(LS2K_STR)
	check_str();
#endif
#ifdef INET
	SBD_DISPLAY ("NETI", CHKPNT_NETI);
	init_net (1);
#endif

#if NCMD_HIST > 0
	SBD_DISPLAY ("HSTI", CHKPNT_HSTI);
	histinit ();
#endif

#if NMOD_SYMBOLS > 0
	SBD_DISPLAY ("SYMI", CHKPNT_SYMI);
	syminit ();
#endif

#ifdef DEMO
	SBD_DISPLAY ("DEMO", CHKPNT_DEMO);
	demoinit ();
#endif

	SBD_DISPLAY ("SBDE", CHKPNT_SBDE);
	initial_sr |= tgt_enable (tgt_getmachtype ());

#ifdef SR_FR
	Status = initial_sr & ~SR_FR; /* don't confuse naive clients */
#endif
	/* Set up initial console terminal state */
	ioctl(STDIN, TCGETA, &consterm);

#ifdef HAVE_LOGO
	tgt_logo();
#else
	printf ("\n * PMON2000 Professional *");
#endif
	printf ("\nConfiguration [%s,%s", TARGETNAME,
			BYTE_ORDER == BIG_ENDIAN ? "EB" : "EL");
#ifdef INET
	printf (",NET");
#endif
#if NSD > 0
	printf (",SCSI");
#endif
#if NWD > 0
	printf (",IDE");
#endif
	printf ("]\nVersion: %s.\n", vers);
	printf ("Supported loaders [%s]\n", getExecString());
	printf ("Supported filesystems [%s]\n", getFSString());
	printf ("This software may be redistributed under the BSD copyright.\n");

		
	print_cpu_info();
	print_mem_freq();

	printf ("Memory size %lld MB .\n", memorysize_total);

	tgt_memprint();
#if defined(SMP)
	tgt_smpstartup();
#endif

	printf ("\n");
	loongson_smbios_init();	
	md_clreg(NULL);
	md_setpc(NULL, (int32_t) CLIENTPC);
	md_setsp(NULL, tgt_clienttos ());
	DevicesInit();
}

/*
 *  closelst(lst) -- Handle client state opens and terminal state.
 */
void
closelst(int lst)
{
	switch (lst) {
	case 0:
		/* XXX Close all LU's opened by client */
		break;

	case 1:
		break;

	case 2:
		/* reset client terminal state to consterm value */
		clntterm = consterm;
		break;
	}
}

/*
 *  console_state(lst) -- switches between PMON2000 and client tty setting.
 */
void
console_state(int lst)
{
	switch (lst) {
	case 1:
		/* save client terminal state and set PMON default */
		ioctl (STDIN, TCGETA, &clntterm);
		ioctl (STDIN, TCSETAW, &consterm);
		break;

	case 2:
		/* restore client terminal state */
		ioctl (STDIN, TCSETAF, &clntterm);
		break;
	}
}

/*************************************************************
 *  dotik(rate,use_ret)
 */
void
dotik (rate, use_ret)
	int             rate, use_ret;
{
static	int             tik_cnt;
static	const char      more_tiks[] = "|/-\\";
static	const char     *more_tik;

	tik_cnt -= rate;
#if NMOD_USB_OHCI
	if(ohci_index)
		dl_ohci_kbd();
#endif
	if (tik_cnt > 0) {
		return;
	}
	tik_cnt = 256000;
	if (more_tik == 0) {
		more_tik = more_tiks;
	}
	if (*more_tik == 0) {
		more_tik = more_tiks;
	}
	if (use_ret) {
		printf (" %c\r", *more_tik);
	} else {
		printf ("\b%c", *more_tik);
	}
	more_tik++;
}

#if NCMD_MORE == 0
/*
 *  Allow usage of more printout even if more is not compiled in.
 */
int
more (p, cnt, size)
     char           *p;
     int            *cnt, size;
{ 
	printf("%s\n", p);
	return(0);
}
#endif

/*
 *  Non direct command placeholder. Give info to user.
 */
int 
no_cmd(ac, av)
     int ac;
     char *av[];
{
    printf("Not a direct command! Use 'h %s' for more information.\n", av[0]);
    return (1);
}

/*
 *  Build argument area on 'clientstack' and set up clients
 *  argument registers in preparation for 'launch'.
 *  arg1 = argc, arg2 = argv, arg3 = envp, arg4 = callvector
 */

void
initstack (ac, av, addenv)
    int ac;
    char **av;
    int addenv;
{
	char	**vsp, *ssp;
	int	ec, stringlen, vectorlen, stacklen, i;
	register_t nsp;

	struct boot_params *bp;
	struct loongson_params  *lp;
	struct efi_memory_map_loongson *emap;
	struct efi_cpuinfo_loongson *ecpu;
	struct system_loongson *esys;
	struct irq_source_routing_table *eirq_source;
	struct interface_info *einter;
	struct board_devices *eboard;

	int param_len = 0;

	/*
	 *  Calculate the amount of stack space needed to build args.
	 */
	stringlen = 0;
	if (addenv) {
		envsize (&ec, &stringlen);
	}
	else {
		ec = 0;
	}
#ifdef BOOT_PARAM
	param_len = stringlen;
	param_len = ( param_len + 7 ) & ~7;
	stringlen = 0x0;
#endif
	for (i = 0; i < ac; i++) {
		stringlen += strlen(av[i]) + 1;
	}
	stringlen = (stringlen + 7) & ~7;	/* Round to words */

#ifndef BOOT_PARAM
	vectorlen = (ac + ec + 2) * sizeof (char *);
	stacklen = ((vectorlen + stringlen) + 7) & ~7;

#else
	vectorlen = (ac + 1) * sizeof (char *);
	vectorlen = ( vectorlen + 7 ) & ~7;
	stacklen = ((vectorlen + stringlen + param_len) + 7) & ~7;
#endif
	/*
	 *  Allocate stack and us md code to set args.
	 */
	nsp = md_adjstack(NULL, 0) - stacklen;

#ifndef BOOT_PARAM
	md_setargs(NULL, ac, nsp, nsp + (ac + 1) * sizeof(char *), (int)callvec);
	printf("ac = %08x, nsp @ %08x, env @ %08x, en @ %08x\n", ac, nsp, nsp + (ac + 1) * sizeof(char *), callvec);
#else
	md_setargs(NULL, ac, nsp, nsp + vectorlen + stringlen, (int)callvec);
	printf("ac = %08x, nsp @ %08x, env @ %08x, en @ %08x\n", ac, nsp, nsp + vectorlen + stringlen, callvec);
#endif

	/* put $sp below vectors, leaving 32 byte argsave */
	md_adjstack(NULL, nsp - 32);
	memset((void *)((int)nsp - 32), 0, 32);

	/*
	 * Build argument vector and strings on stack.
	 * Vectors start at nsp; strings after vectors.
	 */
	vsp = (char **)(int)nsp;
	ssp = (char *)((int)nsp + vectorlen);

	for (i = 0; i < ac; i++) {
		*vsp++ = ssp;
		strcpy (ssp, av[i]);
		ssp += strlen(av[i]) + 1;
	}

	ssp = ((int)ssp + 7) & ~7;	/* Round to words */

	*vsp++ = (char *)0;

	/* build environment vector on stack */
	if (ec) {
		printf("vsp = 08x%llx, ssp @ 08x%llx\n", (unsigned long long )vsp, (unsigned long long)ssp);
		if (ssp !=  (nsp + vectorlen + stringlen))
		{
			printf("!!! Error @@@: stack not meet \n");
		}
		envbuild (vsp, ssp);
#ifdef BOOT_PARAM
		bp = (struct boot_params *) ssp;
		lp = &(bp->efi.smbios.lp);
		emap = (struct efi_memory_map_loongson *)((unsigned long long)lp+lp->memory_offset);
		ecpu = (struct efi_cpuinfo_loongson *)((unsigned long long)lp + lp->cpu_offset);
		esys = (struct system_loongson *)((unsigned long long)lp+lp->system_offset);
		eirq_source = (struct irq_source_routing_table *)((unsigned long long)lp+lp->irq_offset);
		eboard = (struct board_devices *)((unsigned long long)lp+lp->boarddev_table_offset);

		printf("board_name:%s ---%p %d\n",&(eboard->name),eboard->name,eboard->num_resources);
		printf("Shutdown:%p reset:%p\n",bp->reset_system.Shutdown,bp->reset_system.ResetWarm);

#else
		printf("ssp:%lx line=%d\n",ssp,__LINE__);

#endif
	}
	else {
		*vsp++ = (char *)0;
	}

	/*
	 * Finally set the link register to catch returning programs.
	 */
#ifdef DTB
#include "target/load_dtb.h"
	if(!getenv("oldpmon"))
	esys->of_dtb_addr = setup_dtb(ac, av);
#endif
	md_setlr(NULL, (register_t)_exit);
}

void get_memorysize(unsigned long long raw_memsz) {
	unsigned long long memsz,mem_size;
	//tgt_printf("raw_memsz: 0x%llx\n", raw_memsz);
//3A4000 use 1GB unit to transfer memsize
	memsz = raw_memsz & 0xff;
#ifdef LOONGSON3A4000 
	memsz = memsz << 30;
#else
	memsz = memsz << 29;
#endif
	memsz = memsz - 0x1000000;
	memsz = memsz >> 20;
	/*
	 *	Set up memory address decoders to map entire memory.
	 *	But first move away bootrom map to high memory.
	 */
	memorysize = memsz > 240 ? 240 << 20 : memsz << 20;
	memorysize_high = memsz > 240 ? (((unsigned long long)memsz) - 240) << 20 : 0;
	mem_size = memsz;

#ifdef MULTI_CHIP
	memsz = raw_memsz & 0xff00;
	memsz = memsz >> 8;
#ifdef LOONGSON3A4000 
	memsz = memsz << 30;
#else
	memsz = memsz << 29;
#endif
    memorysize_high_n1 = (memsz == 0) ? 0 : (memsz - (256 << 20));

    memsz = raw_memsz & 0xff0000;
    memsz = memsz >> 16;
#ifdef LOONGSON3A4000 
	memsz = memsz << 30;
#else
	memsz = memsz << 29;
#endif
    memorysize_high_n2 = (memsz == 0) ? 0 : (memsz - (256 << 20));

	memsz = raw_memsz & 0xff000000;
	memsz = memsz >> 24;
#ifdef LOONGSON3A4000 
	memsz = memsz << 30;
#else
	memsz = memsz << 29;
#endif
    memorysize_high_n3 = (memsz == 0) ? 0 : (memsz - (256 << 20));
#endif
	memorysize_total =  ((memorysize  +  memorysize_high)  >> 20) + 16;
#ifdef MULTI_CHIP
	if(memorysize_high_n1 != 0)
	    memorysize_total += ((memorysize_high_n1 + (256 << 20)) >> 20);
	if(memorysize_high_n2 != 0)
		memorysize_total += ((memorysize_high_n2 + (256 << 20)) >> 20);
	if(memorysize_high_n3 != 0)
	    memorysize_total += ((memorysize_high_n3 + (256 << 20)) >> 20);
#endif
	tgt_printf("memorysize_high: 0x%llx\n", memorysize_high);
#ifdef MULTI_CHIP
	tgt_printf("memorysize_high_n1: 0x%llx\n", memorysize_high_n1);
	tgt_printf("memorysize_high_n2: 0x%llx\n", memorysize_high_n2);
	tgt_printf("memorysize_high_n3: 0x%llx\n", memorysize_high_n3);
#endif
}

/* default mem freq print, overrided by board print_mem_freq() */
void __attribute__((weak)) print_mem_freq(void)
{
#if defined(BOOT_PARAM)
#if defined(LOONGSON_2G1A) || defined(LOONGSON_2F1A)
#define MEM_BASE_CLK_2G	25
	int clk567, clk89;
#endif

	int memfreq,clk,clk30,clk4, clk20, clk34, mem_vco;
	unsigned short div_refc, div_loopc, div_out;

 	clk = get_mem_clk();
#if defined(LOONGSON_3BSINGLE) || defined (LOONGSON_3BSERVER)
	 /*for 3c/3b1500 ddr control*/
        if(clk != 0xf)
	{
#ifndef LOONGSON_3B1500
		clk20 = clk & 0x07;
		clk34 = (clk >> 3) & 0x03;
		mem_vco = 100 * (20 + clk20 * 2 + (clk20 == 7) * 2) / 3;
		memfreq = mem_vco / (1 << (clk34 + 1));
#else
		if ((clk & 0x8) == 0x0) { /* set ddr frequency by software */
			div_refc = ((*(volatile unsigned int *)(0xbfe001c0)) >> 8) & 0x3f;
			div_loopc = ((*(volatile unsigned int *)(0xbfe001c0)) >> 14 ) & 0x3ff;
			div_out = ((*(volatile unsigned int *)(0xbfe001c0)) >> 24) & 0x3f;
			memfreq = ((100 / div_refc) * div_loopc) / div_out / 3;
		} else { /* set ddr frequency by hareware */
			clk20 = clk & 0x07;
			clk34 = (clk >> 3) & 0x03;
			mem_vco = 100 * (20 + clk20 * 2 + (clk20 == 7) * 2) / 3;
			memfreq = mem_vco / (1 << (clk34 + 1));
		}
		printf("/ Bus @ %d MHz\n",memfreq);
#endif
	}
        else
		printf("/ Bus @ 33 MHz\n");
#elif defined(LOONGSON_2G1A) || defined(LOONGSON_2F1A)
	/* to calculate memory frequency.
	 * we can find this function in loongson 2G manual,
	 * memclk * (clksel[7:5] * 2 + 20 + clksel[7:5] == 7 ? 2 : 0)/((clksel[9:8] + 1) * 2)
	 */
        if(clk != 0x1f)
	{
		clk567 = clk & 0x07;
		clk89 = (clk >> 3) & 0x3;
		memfreq = MEM_BASE_CLK_2G *(clk567 * 2 + 20 + (clk567 == 7 ? 2 : 0))/(2 * (clk89 + 1));

		printf("/ Bus @ %d MHz\n", memfreq);
	}
        else
		printf("/ Bus @ %d MHz\n", MEM_BASE_CLK_2G);
#elif defined(LOONGSON_2K)
		printf("/ Bus @ %d MHz\n", read_ddrfreq());
	
#else  /* for 3a ddr controller */
        if(clk != 0x1f)
	{
		if ((*(volatile unsigned char*)0xbfe00187) == 0x37) {
			if ((clk & 0xf) == 0xf) { /* set ddr frequency by software */
				div_loopc = ((*(volatile unsigned int *)(0xbfe001c0)) >> 14 ) & 0x3ff;
				div_out = ((*(volatile unsigned int *)(0xbfe001c0)) >> 24) & 0x3f;
				memfreq = (100 * div_loopc) / div_out / 3;
			} else { /* set ddr frequency by hareware */
				clk30 = clk & 0x0f;
				clk4 = (clk >> 4) & 0x01;
				/* to calculate memory frequency.
				 * we can find this function in loongson 3A manual,
				 * memclk * (clksel[8:5] + 30)/(clksel[9] + 3)
				 */
				memfreq = 100*(clk30 + 30)/(clk4 + 3)/3;
			}
		} else {
		clk30 = clk & 0x0f;
		clk4 = (clk >> 4) & 0x01;
		/* to calculate memory frequency.
		 * we can find this function in loongson 3A manual,
		 * memclk * (clksel[8:5] + 30)/(clksel[9] + 3)
		 */
		memfreq = 100*(clk30 + 30)/(clk4 + 3)/3;
		}

		printf("/ Bus @ %d MHz\n",memfreq);
	}
        else
		printf("/ Bus @ 33 MHz\n");
#endif
#endif
}

void __attribute__((weak)) print_cpu_info(void)
{
	int  freq;
	char fs[10], *fp;

	tgt_machprint();

	freq = tgt_pipefreq ();
	sprintf(fs, "%d", freq);
	fp = fs + strlen(fs) - 6;
	fp[3] = '\0';
	fp[2] = fp[1];
	fp[1] = fp[0];
	fp[0] = '.';
	printf (" %s MHz", fs);
}

uint32_t read_c0_count()
{
	uint32_t ret;
	__asm__ __volatile__(
	"		.set mips64r2			\n"
	"		rdhwr	%0, $2			\n"
	"		.set mips0			\n"
	: "=r"(ret));
	
	return ret;
}


void __loop_delay(unsigned int loops)
{
	__asm__ __volatile__(
	"		.set noreorder			\n"
	"		.align 3			\n"
	"1:		bnez	%0, 1b			\n"
	"		addiu	%0, -1			\n"
	"		.set reorder			\n"
	: "=r"(loops)
	: "0"(loops));
}

