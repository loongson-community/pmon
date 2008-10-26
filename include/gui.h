#ifndef __PMON_LOONGSON_GUI_H_
#define __PMON_LOONGSON_GUI_H_


#include <list.h>

void gui_print(int row, int col, int width, int forecolor, int bgcolor, const char *str);


enum{
	EVT_KEY
};


struct widgetcb;
struct wincb;

typedef int (*win_ctor_t)(struct wincb *this);
typedef int (*win_process_t)(struct wincb *this, int message_type, int message);
typedef void (*win_dtor_t)(struct wincb *this);
typedef void (*widget_func)(struct widgetcb *this);
typedef int (*widget_process_t)(struct widgetcb *this, int message_type, int message);


/* widget control block */
struct widgetcb
{
	/* currently id doesn't need to be unique
	   you can use it for any purpose, now i use 
	   id for boot item for env indexing.
	*/
	int id;
	int row;
	int col;
	int cursor;
	int malloc_flag;
	const char *title;
	int refresh_cnt;
	widget_func ctor;
	widget_func refresh;
	widget_func get_focus;
	widget_func lose_focus;
	widget_func refresh_focus;
	widget_process_t process;
	struct list_head list;
};

/* window control block */
struct wincb
{
	win_ctor_t ctor;
	win_process_t process;
	win_dtor_t dtor;
	unsigned int widget_cnt;
	struct widgetcb *current_widget;
	struct list_head widget_list;
};


int change_current_win(struct wincb *new_win);

inline struct widgetcb *get_next_widget(
	struct wincb *win)
{
	struct list_head *next = win->current_widget->list.next;
	if (next != &win->widget_list)
		return list_entry(next, struct widgetcb, list);
	else
		return list_entry(next->next, struct widgetcb, list);
}

inline struct widgetcb *get_prev_widget(
	struct wincb *win)
{
	struct list_head *prev = win->current_widget->list.prev;
	if (prev != &win->widget_list)
		return list_entry(prev, struct widgetcb, list);
	else
		return list_entry(prev->prev, struct widgetcb, list);

}

#endif

