/* SPDX-License-Identifier: BSD-2-Clause */

/*
 * PMON Generic Menu Select Library
 *     Author: Jiaxun Yang <jiaxun.yang@flygoat.com>
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

#include "libmenu.h"

#define PREV	CNTRL('P')
#define NEXT	CNTRL('N')
#define FORW	CNTRL('F')
#define BACK	CNTRL('B')
#define BEGIN	CNTRL('A')
#define END	CNTRL('E')
#define DELETE	CNTRL('D')
#define DELLINE	CNTRL('K')
#define MARK	CNTRL(' ')
#define KILL	CNTRL('W')
#define CLEAN	CNTRL('L')
#define TIMER   0x102

int getch (void);
#define COLOR 0x7

extern int vga_available;
extern void setY(int );
extern void setX(int );
extern void video_console_print(int console_col, int console_row, unsigned char *s);

#define DEBUG

#if 0
static char* asc_pic[] = {
'       :\     /;               _  ',
'      ;  \___/  ;             ; ; ',
'     ,:-"    `"-:.            / ; ',
'_   /,---.   ,---.\   _     _; /  ',
'_:>((  |  ) (  |  ))<:_ ,-""_,"   ',
'    \`````   `````/""""",-""      ',
'     `-.._ v _..-`      )         ',
'       / ___   ____,..  \         ',
'      / /   | |   | ( \. \        ',
'ctr  / /    | |    | |  \ \       ',
'     `"     `"     `"    `"       ',
'                                  '
};
#else
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
#endif

static int asc_pic_line = 12;

/*
 * Prototypes
 */

#define FRAME_WIDTH 50
static int top_height = 0;
static int vesa_height = 25;
static int frame_height = 12;
static int mid_height = 0;
static int bottom_height = 0;
extern void (*__cprint)(int y, int x,int width,char color, const char *text);
extern void (*__set_cursor)(unsigned char x,unsigned char y);
extern (*__popup)(int y, int x,int height,int width);
extern void (*__scr_clear)();

static int draw_top_copyright(void)	
{	
	int top_level = 0;
	__cprint(top_level++,0,0,COLOR,"                                            ");
	__cprint(top_level++,0,0,COLOR,"                                            ");;
	__cprint(top_level++,0,0,COLOR,"                                            ");;
	top_height = top_level;
	return 0;
}

static int draw_mid_main(int sel, libmenu_obj *obj)
{
	int i;
	char str_line[81];
	char tmp[100];
	int selected = sel ;	
	const char* label = &obj->nemu_name;

	mid_height = top_height;

	vesa_height = 600/16;
	memset(tmp, 0, sizeof(tmp));

    //First line of middle graph
	memset(str_line, (char)196, sizeof(str_line));
	str_line[FRAME_WIDTH] = '\0';
	str_line[0] = (char)218;
	str_line[FRAME_WIDTH - 1] = (char)191;
	__cprint(mid_height++,0,0,COLOR,str_line);;	

    //Second line of middle graph
	memset(str_line, ' ', sizeof(str_line));
	str_line[FRAME_WIDTH] = '\0';
	str_line[0] = (char)179;
	sprintf(str_line + (FRAME_WIDTH - strlen(label) - strlen(tmp)) / 2, "%s %s ", label,tmp);
	str_line[strlen(str_line)] = ' ';
	str_line[FRAME_WIDTH - 1] = (char)179;
	__cprint(mid_height++,0,0,COLOR,str_line);;

    //Third line of middle graph
    memset(str_line, (char)196, sizeof(str_line));
	str_line[FRAME_WIDTH] = '\0';
	str_line[0] = (char)195;
	str_line[FRAME_WIDTH - 1] = (char)180;
	__cprint(mid_height++,0,0,COLOR,str_line);;
	
	/* print menu title */
    for (i = 0; i < frame_height - 3; i++)
	{
        int j = (selected<9)?0:(selected-9+1);
        /* i means actual frameline, j means the first item of current frame  */

		memset(tmp, 0, sizeof(tmp));
		memset(str_line, ' ', sizeof(str_line));
		
		if (i < asc_pic_line)
		{
			memcpy(str_line + FRAME_WIDTH + 3, asc_pic[i], strlen(asc_pic[i]));
		}
		
		str_line[79] = '\0';
		str_line[0] = (char)179;
		if (j + i < obj->num_items && i < 9)
		{
			if (selected == (j + i))
			{	
				sprintf(tmp, "%s %d %s","->", j + i, obj->items[j+i].title);
			} else {
				sprintf(tmp, "%s %d %s","  ", j + i, obj->items[j+i].title);
			}
			
			tmp[48] = '\0';

			memcpy(str_line + 2, tmp, strlen(tmp));
		}
		str_line[FRAME_WIDTH - 1] = (char)179;
	    __cprint(mid_height++, 0, 0, COLOR, str_line);;
	}
	
	memset(str_line, ' ', sizeof(str_line));
	str_line[FRAME_WIDTH] = '\0';
	str_line[0] = (char)179;
	memset(tmp, 0, sizeof(tmp));
	sprintf(tmp, "Please Select %s [%d]", obj->nemu_name, selected);
	memcpy(str_line + 2, tmp, strlen(tmp));
	memset(str_line + strlen(str_line), ' ', FRAME_WIDTH - strlen(str_line));
	str_line[FRAME_WIDTH - 1] = (char)179;
	__cprint(mid_height++,0,0,COLOR,str_line);;

	memset(str_line, (char)196, sizeof(str_line));
	str_line[FRAME_WIDTH] = '\0';
	str_line[0] = (char)192;
	str_line[FRAME_WIDTH - 1] = (char)217;
	__cprint(mid_height++,0,0,COLOR,str_line);;
	return 0;
}

static int draw_bottom_main(void)
{	
	bottom_height = top_height + mid_height - 2;
	__cprint(bottom_height++,0,0,COLOR,"Use the UP and DOWN keys to select the entry.");;
	__cprint(bottom_height++,0,0,COLOR,"Press ENTER to enter the selected entry.");;
	__cprint(bottom_height++,0,0,COLOR,"Press 'c' to command-line.");;

	return 0;
}
static int draw_main(int sel, libmenu_obj *obj)
{

	draw_top_copyright();

	draw_mid_main(sel, obj);

	draw_bottom_main();

	return 0;
}

int libmenu_show(libmenu_obj *obj)
{
	int i, j;
	unsigned int cnt;
	int dly ; 	
	int retid;
    struct termio sav;

	char str_line[81];
	char tmp[100];
    int ch;
	int not_delay = FALSE;
	int not_erased = TRUE;
	int selected_menu_num;
	
	selected_menu_num = obj->default_item;

    if (selected_menu_num >= obj->num_items)
        selected_menu_num = 0;

    top_height = 0;
    vesa_height = 25;
    frame_height = 12;
    mid_height = 0;
    bottom_height = 0;

	dly = obj->timeout;
	if (dly < 0)
	{
		dly = 5;
	}

	__console_init();
    ioctl (STDIN, CBREAK, &sav);
	ioctl (STDIN, FIONREAD, &cnt);
	while (cnt !=0 ) //Avoid the pre Pressed TAB to escape the Delay showing
	{
		getchar();
		ioctl (STDIN, FIONREAD, &cnt);
	}
	__scr_clear();
	draw_main(selected_menu_num, obj);
	
	i = 1;
	ioctl(STDIN, FIONBIO, &j);
	ioctl(STDIN, FIOASYNC, &j);

	memset(tmp, 0, sizeof(tmp));
	memset(str_line, ' ', sizeof(str_line));
	sprintf(tmp, "                Countinue in [%d] second(s)", dly);

	str_line[(sizeof(str_line))] =  '\0';
	memcpy(str_line + sizeof(str_line) - strlen(tmp) - 1, tmp, strlen(tmp));
	__cprint(bottom_height + 2,0,0,COLOR,str_line);;

	while (1) {
            ch = getch();			
			if (ch!=TIMER)
			    not_delay = TRUE;

			if (strchr("\r\n", ch) != NULL)
			{
				__set_cursor(0,0);
				break;
			}
            else if (99 == ch) //'c' pressed ,back to console
            {
				__scr_clear();
				__set_cursor(0,0);
                return 0;
            }			
			else if (ch  == PREV) //UP key pressed
            { 
                --selected_menu_num;
                if (selected_menu_num >= 0)
                    draw_mid_main(selected_menu_num, obj);
                else
                    ++selected_menu_num;
            }
            else if (ch == NEXT)//DOWN key pressed
            { 
                ++selected_menu_num;
                if (selected_menu_num < obj->num_items)
                    draw_mid_main(selected_menu_num, obj);
                else 
                    --selected_menu_num;
            }	

            if (not_delay != TRUE)
            {
                    memset(tmp, 0, sizeof(tmp));
                    memset(str_line, ' ', sizeof(str_line));				
                    sprintf(tmp, "Continue in [%d] second(s)", --dly);
                    str_line[(sizeof(str_line))] =  '\0';
                    memcpy(str_line + sizeof(str_line) - strlen(tmp) - 1 , tmp, strlen(tmp));
                    __cprint(bottom_height + 2,0,0,COLOR,str_line);;
                if (dly == 0)
                    break;
            }
            else if (not_erased)
            {
                int bottom;

                for (bottom = 0; bottom < 5; bottom++) {
#if 0
                    // FIXME: May fail
                    if (obj->items[selected_menu_num].bottom_info[bottom])
                        __cprint(bottom_height + 1,0,80,COLOR,obj->items[selected_menu_num].bottom_info[bottom]);
#endif
                }
            }
	}

JUST_BOOT:
	__scr_clear();
	__set_cursor(0,0);
    ioctl (STDIN, TCSETAF, &sav);
	return obj->items[selected_menu_num].cb(selected_menu_num, obj->items[selected_menu_num].cb_data);
}

#ifdef DEBUG

static int cmd_menutest(int ac, char *av[])
{
    libmenu_obj obj;
    char pwroff[] = "Powwer off";
    char reboot[] = "Reboot";

    sprintf(obj.nemu_name, "Menu test");
    obj.default_item = 0;
    obj.num_items = 2;
    obj.timeout = 50;

    obj.items[0].title = pwroff;
    obj.items[0].bottom_info[0] = pwroff;
    obj.items[0].cb = tgt_poweroff;

    obj.items[1].title = reboot;
    obj.items[1].bottom_info[0] = reboot;
    obj.items[1].cb = tgt_reboot;

    libmenu_show(&obj);
    
    return 0;
}

static const Cmd Cmds[] =
{
	{"flygoat"},
	{"menutest", "", 0, "Test libmenu", cmd_menutest, 0, 99, 0},
	{0, 0}
};

static void init_cmd __P((void)) __attribute__ ((constructor));

static void
init_cmd()
{
	cmdlist_expand(Cmds, 1);
}

#endif