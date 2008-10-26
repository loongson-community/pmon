#include <stdio.h>
#include <termio.h>
#include <endian.h>
#include <string.h>
#include <signal.h>
#include <setjmp.h>
#include <ctype.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdarg.h>
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
#include <fb.h>
#include <gui.h>
#include <list.h>

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
#define UPDATE  0x100
#define UPDATE1  0x101
#define TIMER   0x102
#define MBEGIN 0x103
#define MEND 	0x104
#define RUN -1

#define SIZE_OF_EXPLINE 512
static char cvtab[] = { PREV, NEXT, FORW, BACK, 'E', END, 'G', BEGIN };
enum esc_state { NONE, HAVE_ESC, HAVE_LB };

extern int vga_available;
void gui_trace(const char *fmt, ...)
{
	int len;
	va_list ap;
	int old_vga_available = vga_available;
	vga_available = 0;
	va_start(ap, fmt);
	len = vfprintf (stdout, fmt, ap);
	va_end(ap);
	vga_available = old_vga_available;
	return ;
}

/* widget func */
void null_refresh(struct widgetcb *this);
void caption_focus_in(struct widgetcb *this);
void caption_lose_focus(struct widgetcb *this);
void caption_refresh_focus(struct widgetcb *this);
int boot_process(struct widgetcb *this, int message_type, int message);


struct widgetcb def_boot = {
	0, 1, 40, 0, 0, "hello1", 0x8000,
	null_refresh,
	null_refresh, 
	caption_focus_in, 
	caption_lose_focus,
	caption_refresh_focus,
	boot_process,
	LIST_HEAD_INIT(def_boot.list)
};

#define BOOT_ROW 5
struct widgetcb *mk_boot_widget(int id, const char *file_path, const char *title)
{
	struct widgetcb *boot = (struct widgetcb *)0;

	if (file_path ==0) return (struct widgetcb *)0;
	
	//printf("id %i, file_path %s\n", id, file_path);
	
	boot = malloc(sizeof(struct widgetcb));
	if (!boot){
		printf("[mk_boot_widget]malloc assertion!!!!]");
		return boot;
	}
	
	*boot = def_boot;

	boot->id = id;
	boot->row = BOOT_ROW + id*2;
	boot->col = 5;
	boot->malloc_flag = 1;
	boot->title = title;
	if (boot->title == 0) boot->title = file_path;
	INIT_LIST_HEAD(&boot->list);
	
	return boot;
	
}

#if 0
struct widgetcb def_editor = {
	0, 1, 40, 0, "", 0x8000,
	null_refresh,
	null_refresh, 
	caption_focus_in, 
	caption_lose_focus,
	caption_refresh_focus,
	widget_boot,
	LIST_HEAD_INIT(def_boot.list)
};

struct widgetcb *mk_boot_editor(int id)
{
	struct widgetcb *editor = (struct widgetcb *)0;

	editor = malloc(sizeof(struct widgetcb));
	if (!editor){
		printf("[mk_editor_widget]malloc assertion!!!!]");
		return (struct widgetcb *)0;
	}
	
	*editor = def_editor;

	editor->id = id;
	editor->row = BOOT_ROW + id*2;
	editor->col = 5;
	editor->malloc_flag = 1;
	editor->title = title;
	if (editor->title == 0) editor->title = file_path;
	INIT_LIST_HEAD(&editor->list);
	
	return editor;
}
#endif
void null_refresh(struct widgetcb *this)
{
	//gui_trace("null_refresh for id  %d\n", this->id);
}

void caption_focus_in(struct widgetcb *this)
{
	gui_print(this->row, this->col, 10, 6, 0, this->title);
	gui_trace("id %d\n", this->id);
}

void caption_lose_focus(struct widgetcb *this)
{
	gui_print(this->row, this->col, 10, 7, 0, this->title);
	gui_trace("id %d\n", this->id);
}

void caption_refresh_focus(struct widgetcb *this)
{
	static unsigned int i = 0;
	
	/*gui_trace("caption_refresh_focus for %d\n", this->id);*/
	
	if (i++ % 2)
		gui_print(this->row, this->col, 10, 7, 0, this->title);
	else	
		gui_print(this->row, this->col, 10, 6, 0, this->title);

	gui_trace("id %d\n", this->id);
}

static char cmd_buf[0x100];
void boot_on_id(int id)
{
	char load_str[] = "load $al0";
	char karg_str[] = "karg0";
	
	const char *karg;

	if (id == 0 && getenv("al") != 0){
		load_str[8] = 0;
		karg_str[4] = 0;
	}else{
		load_str[8] += (unsigned char)id;
		karg_str[4] += (unsigned char)id;
	}
	
	memset(cmd_buf, 0, sizeof(cmd_buf));	
	do_cmd(load_str);

	karg = getenv(karg_str);
	strcpy(cmd_buf,"g ");
	if(karg != NULL  && strlen(karg) != 0){
		strcat(cmd_buf, karg);
	}else{
		strcat(cmd_buf," -S root=/dev/hda1 console=tty");
	}
	do_cmd(cmd_buf);
}

int boot_process(struct widgetcb *this, int message_type, int message)
{
	int ret = -1;
	switch(message)
	{
	/* ENTER */
	case 0xa:
		boot_on_id(this->id);
		ret = 0;
		break;
	default:
		break;
	}
	return ret;

}

/* win func */
int main_win_ctor(struct wincb *this);
void main_win_dtor(struct wincb *this);
int main_win_process(struct wincb *this, int message_type, int message);

struct wincb main_win = 
{
	main_win_ctor,
	main_win_process,
	main_win_dtor,
	(struct widgetcb *)0,
	LIST_HEAD_INIT(main_win.widget_list)

};

int main_win_ctor(struct wincb *this)
{
	int id = 0;
	char al_str[] = "al0";
	char title_str[] = "title0";
	struct list_head *p;
	struct widgetcb *widget;
	struct list_head *last_widget = &this->widget_list;

	video_cls();

	INIT_LIST_HEAD(&this->widget_list);
	widget = mk_boot_widget(0, getenv("al"), getenv("title"));
	if (widget){
		list_add(&widget->list, last_widget);
		last_widget = &widget->list;
		id++;
	}
	
	//construct widgets_list
	while(1){
		al_str[2] = '0' + (unsigned char)id;
		title_str[5] = '0' + (unsigned char)id;
		widget = mk_boot_widget(id, getenv(al_str), getenv(title_str));
		if (!widget) break;
		list_add(&widget->list, last_widget);
		last_widget = &widget->list;
		id++;
	}

	//default boot item
	this->current_widget = list_entry(this->widget_list.next, struct widgetcb, list);

	list_for_each(p, &this->widget_list){
		widget = list_entry(p, struct widgetcb, list);
		if (widget == this->current_widget){
			widget->get_focus(widget);
		}else{
			widget->lose_focus(widget);
		}
	}
	if (id == 0) return -1;
	return 0;
}

void main_win_dtor(struct wincb *this)
{
	struct list_head *p, *n;
	struct widgetcb *widget;

	video_cls();
	
	list_for_each_safe(p, n, &this->widget_list){
		list_del_init(p);
		widget = list_entry(p, struct widgetcb, list);
		if (widget->malloc_flag != 0) free(widget);
	}
	this->current_widget = (struct widgetcb *)0;
	this->widget_cnt = 0;

}

void change_focus(struct wincb *win, struct widgetcb *old_widget, struct widgetcb *new_widget)
{
	win->current_widget = new_widget;
	old_widget->lose_focus(old_widget);
	new_widget->get_focus(new_widget);
}

int main_win_process(struct wincb *this, int message_type, int message)
{
	struct widgetcb *current = this->current_widget;
	
	if (current->process(current, message_type, message) == 0)
		return 0;
		
	
	switch(message)
	{
		//UP KEY
		case PREV:
			change_focus(this, current, get_prev_widget(this));
			break;

		//DOWN KEY
		case NEXT:
			change_focus(this, current, get_next_widget(this));
			break;
		case 'r':
		case 'R':
			tgt_reboot();
			break;
		case 'q':
		case 'Q':
		case 0x1b:
			return 1;
			break;

	}
	return 0;
}

static int get_input(void)
{
	int cnt;
	enum esc_state esc_state = NONE;
	int c, oc;
	struct termio tbuf;

	ioctl (STDIN, SETNCNE, &tbuf);

	for(;;) {
		ioctl (STDIN, FIONREAD, &cnt);
		if(cnt == 0) return 0;

		c = getchar ();
		oc=c;
		gui_trace("key: 0x%x\n", (unsigned char)c);
		switch (esc_state) {
			case NONE:
				if (c == 0x1b) {
					esc_state = HAVE_ESC;
					continue;
				}
				break;

			case HAVE_ESC:
				if (c == '[') {
					esc_state = HAVE_LB;
					continue;
				}
				esc_state = NONE;
				break;

			case HAVE_LB:
				if (c >= 'A' && c <= 'H' ) {
					c = cvtab[c - 'A'];
				}
				esc_state = NONE;
				break;

			default:
				esc_state = NONE;
				break;
		}
		break;
	}
	ioctl (STDIN, TCSETAW, &tbuf);
	return c;
}


struct wincb *current_win;

int change_current_win(struct wincb *new_win)
{
	if (new_win){
		if (current_win)
			current_win->dtor(current_win);
		current_win = new_win;
		return current_win->ctor(current_win);
	}
	return -1;
}

/* main logic */
int cmd_gui(int ac, char *av[])
{
	int c;
	unsigned int cnt = 0;
	int ret;
	struct list_head *p;
	struct widgetcb *widget;

	current_win = 0;
	if (change_current_win(&main_win) != 0){
		video_cls();
		vga_available = 1;
		printf("gui init failed\n");
		return -1;
	}

	while(1){
		/* check sanity */
		if (!current_win || !(current_win->process)){
			printf("[PMON_GUI]win process assertion");
			return -1;
		}

		/* key event */
		if (c = get_input()){
			ret = current_win->process(current_win, EVT_KEY, c);
			if (ret == 1){
				video_cls();
				vga_available = 1;
				return 0;
			}else if (ret < 0){
				video_cls();
				vga_available = 1;
				printf("[gui]fantal error occur when processing key event");
				return -1;
			}
		}

		/* self refresh & focus refresh event */
		list_for_each(p, &current_win->widget_list){

			widget = list_entry(p, struct widgetcb, list);
			if (cnt % widget->refresh_cnt != 0) continue;

			/* self refresh */
			if (widget && widget->refresh){
				widget->refresh(widget);
			}else{
				printf("widget refresh assertion");
			}

			/* focus refresh */
			if (widget == current_win->current_widget
				&& widget->refresh_focus){
				widget->refresh_focus(widget);
			}

		}
		cnt++;
	}
	
	if (current_win){
		current_win->dtor(current_win);
	}
	return 0;
}


/*
 *  Command table registration
 *  ==========================
 */

static const Cmd GUICmd[] =
{
	{"GUI"},
	{"gui",		"no arg",
		0,
		"start gui process",
		cmd_gui, 1, 1, CMD_REPEAT},
	{0, 0}
};

static void init_cmd __P((void)) __attribute__ ((constructor));

	static void
init_cmd()
{
	cmdlist_expand(GUICmd, 1);
}

