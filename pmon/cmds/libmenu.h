/* SPDX-License-Identifier: BSD-2-Clause */

/*
 * PMON Generic Menu Select Library
 *     Author: Jiaxun Yang <jiaxun.yang@flygoat.com>
 */

#ifndef __LIBMENU_H__
#define __LIBMENU_H__

#include <pmon.h>
#include <exec.h>

#define MAX_ITEMS   32

#define MAX_TIME_OUT 1000

#define OPTION_LEN	50
#define VALUE_LEN	1024
#define GLOBAL_VALUE_LEN 256

#define MENU_TITLE_BUF_LEN 79

typedef int (*libmenu_cb)(int, void *);

typedef struct libmenu_item
{
	char *title;	//Title of menu item, display on screen.
	char *bottom_info[5]; // Information lines to be displayed at bottom
    libmenu_cb cb;
    void *cb_data;
} libmenu_item;

typedef struct libmenu_obj
{
    char nemu_name[OPTION_LEN];
    int default_item;
    int timeout;
    int num_items;
    libmenu_item items[MAX_ITEMS];
} libmenu_obj;

int libmenu_show(libmenu_obj *obj);

#endif
