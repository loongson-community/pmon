/*	$Id: ifaddr.c,v 1.1.1.1 2006/06/29 06:43:25 cpu Exp $ */

/*
 * Copyright (c) 2002 Opsycon AB  (www.opsycon.se)
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

#include "boot_cfg.h"

//#include <../../x86emu/int10/vesa.h>


#define CDROM 0
#define IDE 1

extern int vga_available;
extern void video_console_print(int , int , unsigned char *);
extern void setY(int );
extern void setX(int );
#if 0
static char* asc_pic[] =
{" .--,       .--, ",
 "( (  \\.---./  ) ) ",
 " '.__/o   o\\__.' ",
 "    {=  ^  =} ",
 "     >  -  < ",
 "    /       \\ ",
 "   //       \\\\ ",
 "  //|   .   |\\\\ ",
 "  \"'\\       /'\"_.-~^`'-. ",
 "     \\  _  /--'         ` ",
 "   ___)( )(___ ",
 "   (((__) (__))) "};


static int asc_pic_line = 12;
#else
static char* asc_pic[] =
{"",
 "",
 "",
 "",
 "",
 "",
 "",
 "",
 "",
 "",
 "",
 ""};


static int asc_pic_line = 12;
#
#endif
/*
 * Prototypes
 */
static int testgui_cmd __P((int, char **av));

#define MAX_SCREEN_WIDTH 150
#define MAX_SCREEN_HEIGHT 80

void src_clr(void)
{
 	char tmp_str[MAX_SCREEN_WIDTH];
	int i;
	
	for (i = 0; i < MAX_SCREEN_WIDTH - 1; i++)
		tmp_str[i] = ' ';
	tmp_str[MAX_SCREEN_WIDTH - 1] = '\0';
	for (i = 0; i < MAX_SCREEN_HEIGHT; i++)
		 	video_console_print(0,i,tmp_str);	
}

int top_height = 0;
static int draw_top_copyright()
{	
 	video_console_print(0,top_height++,"2006-2008 (c) SUNWAH HI-TECH (www.sw-linux.com.cn)");
	video_console_print(0,top_height++,"2004-2006 (c) Lemote, Inc  (www.lemote.com)");
	video_console_print(0,top_height++,"2000-2002 (c) Opsycon AB  (www.opsycon.se)");
	return 0;
}

#define FRAME_WIDTH 50
int vesa_height = 25;
int frame_height = 14;
int mid_height = 0;
static int draw_mid_main(int sel, const char *path)
{
	int i;
	char str_line[81];
	char tmp[100];
	int selected = sel ;	
	const char* label = "Boot Menu List";

	mid_height = top_height;

	vesa_height = 600/16;
	memset(tmp, 0, sizeof(tmp));
	sprintf(tmp, "%s ", path);

    for(i = 0; i < 100; i++)
    {
        if (tmp[i]=='/')
        {
            tmp[i] = '\0'; 
            break;
        }
    }

    //First line of middle graph
	memset(str_line, (char)196, sizeof(str_line));
	str_line[FRAME_WIDTH] = '\0';
	str_line[0] = (char)218;
	str_line[FRAME_WIDTH - 1] = (char)191;
	video_console_print(0,mid_height++,str_line);	

    //Second line of middle graph
	memset(str_line, ' ', sizeof(str_line));
	str_line[FRAME_WIDTH] = '\0';
	str_line[0] = (char)179;
	sprintf(str_line + (FRAME_WIDTH - strlen(label) - strlen(tmp)) / 2, "%s %s ", label,tmp);
	str_line[strlen(str_line)] = ' ';
	str_line[FRAME_WIDTH - 1] = (char)179;
	video_console_print(0,mid_height++,str_line);

    //Third line of middle graph
    memset(str_line, (char)196, sizeof(str_line));
	str_line[FRAME_WIDTH] = '\0';
	str_line[0] = (char)195;
	str_line[FRAME_WIDTH - 1] = (char)180;
	video_console_print(0,mid_height++,str_line);
	
	/* print menu title */
    for (i = 0; i < frame_height - 3; i++)
	{
		memset(tmp, 0, sizeof(tmp));
		memset(str_line, ' ', sizeof(str_line));
		
		if (i < asc_pic_line)
		{
			memcpy(str_line + FRAME_WIDTH + 3, asc_pic[i], strlen(asc_pic[i]));
		}
		
		str_line[79] = '\0';
		str_line[0] = (char)179;
		if (i < menus_num && i < 9 )
		{
			#if 1
			if (selected == (i + 1))
			{
			
				sprintf(tmp, "%s %d %s","->", i + 1, menu_items[i].title);
			}
			else
			{
			
				sprintf(tmp, "%s %d %s","  " ,i + 1, menu_items[i].title);
			}
			#endif
			
			tmp[48] = '\0';

			memcpy(str_line + 2, tmp, strlen(tmp));
		}
		str_line[FRAME_WIDTH - 1] = (char)179;
		//printf("%s\n", str_line);
	    video_console_print(0,mid_height++,str_line);
		//printf("%s\n", menu_items[i].kernel);
	}
	
	memset(str_line, ' ', sizeof(str_line));
	str_line[FRAME_WIDTH] = '\0';
	str_line[0] = (char)179;
	memset(tmp, 0, sizeof(tmp));
	sprintf(tmp, "Please Select Boot Menu [%d]", selected);
	memcpy(str_line + 2, tmp, strlen(tmp));
	memset(str_line + strlen(str_line), ' ', FRAME_WIDTH - strlen(str_line));
	str_line[FRAME_WIDTH - 1] = (char)179;
	video_console_print(0,mid_height++,str_line);

	memset(str_line, (char)196, sizeof(str_line));
	str_line[FRAME_WIDTH] = '\0';
	str_line[0] = (char)192;
	str_line[FRAME_WIDTH - 1] = (char)217;
	video_console_print(0,mid_height++,str_line);
	return 0;
}


int bottom_height = 0;
static int draw_bottom_main()
{	
	bottom_height = top_height + mid_height;
	video_console_print(0,bottom_height++,"  Use number keys to navigate menu, press ENTER to");
	video_console_print(0,bottom_height++," boot selected OS and press 'b' to go back to PMON");
	video_console_print(0,bottom_height++," shell.");

	return 0;
}
static int draw_main(int sel, const char* path)
{

	draw_top_copyright();

	draw_mid_main(sel, path);

	draw_bottom_main();

	return 0;
}
static int show_main(int flag, const char* path)
{
	int i, j;
	unsigned int cnt;
	int dly ; 	
	int selected;
	
	char str_line[81];
	char tmp[100];
    char ch,ch_tab;
	
	if (load_list_menu(path))
	{
		printf("File:boot.cfg not found \n");
		return -1;
	}
	
	selected = atoi(get_option_value("default")) + 1;
	dly = atoi(get_option_value("timeout"));
	if (dly < 0)
	{
		dly = 5;
	}

	src_clr();
	draw_main(selected, path);
	
	if (dly > 0)
	{
		j = 1;
		ioctl(STDIN, FIONBIO, &j);
		ioctl(STDIN, FIOASYNC, &j);

		memset(tmp, 0, sizeof(tmp));
		memset(str_line, ' ', sizeof(str_line));
		sprintf(tmp, "Booting system in [%d] second(s)", dly);

		str_line[(sizeof(str_line))] =  '\0';
		memcpy(str_line + sizeof(str_line) - strlen(tmp) - 1, tmp, strlen(tmp));
		video_console_print(0,bottom_height + 1,str_line);
		j = 0;
		do {
			ioctl (STDIN, FIONREAD, &cnt);

            if (cnt != 0)
            {
                ch_tab = getchar();
            }
            
			if (j == 9)
			{
				memset(tmp, 0, sizeof(tmp));
				memset(str_line, ' ', sizeof(str_line));				
				sprintf(tmp, "Booting system in [%d] second(s)", --dly);
				str_line[(sizeof(str_line))] =  '\0';
				memcpy(str_line + sizeof(str_line) - strlen(tmp) - 1 , tmp, strlen(tmp));
				video_console_print(0,bottom_height + 1,str_line);
				j = 0;
			}
			delay(100000);
			j++;
		} while (dly != 0 && (cnt == 0 || ch_tab == 0x09));
	}
	
	if (cnt > 0)
	{
		while (1)
		{
			if (strchr("\r\n", ch) == NULL)
			{

				if ((ch >= '1' && ch <= '9' ) && (ch - '0' != selected ))
				{
					selected = ch - '0';  
					if (selected > menus_num)                        
					{
						continue;
					}
				
					draw_mid_main(selected, path);
				}
				for (i = 0; i < 80; i++)
				{
					str_line[i] = ' ';
				}
				str_line[80] = '\0';
				video_console_print(0,bottom_height + 1,str_line);
				
				ch = getchar();
            	if (98 == ch)//'b' pressed ,back to console
            	{
					//setX(cursor);
					setY(bottom_height + 2);
					//printf("\n");
					//printf("\n");
					SBD_DISPLAY("ESC0", 0);
                	return 0;
            	}
			}
			else
			{
				break;
			}
		}
	}
	setY(bottom_height + 2);
	do_cmd_boot_load(selected - 1, 0);
	return 0;
}


static int
cmd_menu_list (ac, av)
    int             ac;
    char           *av[];
{
	char path[256] = {0};
	int err;
	int dflag = -1;
	char c;
	int ret;
	struct termio sav;

	err = 0;
	optind = 0;
	optarg = NULL;

	while ((c = getopt(ac, av, "d:")) != EOF)
	{
		switch (c)
		{
		case 'd':
			if (strcmp(optarg, "cdrom") == 0)
			{
				dflag = CDROM;
			}
			else
			{
				dflag = IDE;
			}
			break;
		default:
			err++;
			break;
		}
	}

	if (err > 0)
	{
		return EXIT_FAILURE;
	}

	if (dflag == -1)
	{
		return EXIT_FAILURE;
	}

	strcpy(path, av[optind]);

	ioctl (STDIN, CBREAK, &sav);

	printf("Here before calling show_main().path:%s\n",path);
	show_main(dflag,path);
//	ret = do_cmd_menu_list(dflag, path);
	ioctl (STDIN, TCSETAF, &sav);
	return ret != 0 ? EXIT_FAILURE : 0;
	

}

static int cmd_load_flush(int ac, char *av[])
{
	char * flash_rom;
	char buf[LINESZ] = {0};
	if( ac == 2 )
	{
		sprintf(buf,"load -r -f bfc00000 %s",av[1]);
	}
	else
	{
		flash_rom=getenv("romfile");
		if(flash_rom)
			sprintf(buf,"load -r -f bfc00000 %s",flash_rom);
		else
			sprintf(buf,"load -r -f bfc00000 tftp://192.168.10.84/gzrom.bin");
	}
	do_cmd(buf);
	return 0;
}

static const Cmd Cmds[] =
{
	{"rays"},
	{"bl",	"-d cdrom/ide boot_config_file",0, "Load Boot menu from config file", cmd_menu_list, 2, 4, 0},
	{"fload", "firmware file", 0, "Update BIOS from file.", cmd_load_flush, 0, 2, CMD_ALIAS},
	{0, 0}
};


static void init_cmd __P((void)) __attribute__ ((constructor));

static void
init_cmd()
{
	cmdlist_expand(Cmds, 1);
}

