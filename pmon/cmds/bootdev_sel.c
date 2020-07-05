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

#include "boot_cfg.h"
#include "libmenu.h"

static int load_cmd_cb(int num, void *path)
{
    char load[256];

    sprintf(load, "bl -d ide %s", path);
    return do_cmd(load);
}

int cmd_bootdev_sel(int ac, char *av[])
{
    libmenu_obj obj;
    struct device *dev, *next_dev;
    int num_path = 0;
    char boot_path[10][256];
    char dir[256];
    int i;

    memset(boot_path, 0, 10 * 128);
    memset(dir, 0, 256);

    // Generate menu
    for (dev  = TAILQ_FIRST(&alldevs); dev != NULL; dev = next_dev) {
            next_dev = TAILQ_NEXT(dev, dv_list);
            if(dev->dv_class != DV_DISK) {
                    continue;
            }

            sprintf(dir, "(%s,0)/boot/boot.cfg", dev->dv_xname);
            if (check_config(dir) == 1) {
                memcpy(boot_path[num_path], dir, 256);
                num_path++;
            }

            sprintf(dir, "(%s,0)/boot.cfg", dev->dv_xname);
            if (check_config(dir) == 1) {
                memcpy(boot_path[num_path], dir, 256);
                num_path++;
            }

            if (num_path >= 9)
                break;
    }

    if (num_path == 0) {
        printf("No avilable boot device\n");
        return -1;
    }

    if (num_path == 1) {
        printf("Only one boot device: %s", boot_path[0]);
        return load_cmd_cb(0, boot_path[0]);
    }

    sprintf(obj.nemu_name, "Boot Device");
    obj.default_item = 0;
    obj.num_items = num_path;
    obj.timeout = 10;

    for (i = 0; i < num_path; i++) {
        obj.items[i].title = boot_path[i];
        obj.items[i].bottom_info[0] = boot_path[i];
        obj.items[i].cb = load_cmd_cb;
        obj.items[i].cb_data = boot_path[i];
    }

    libmenu_show(&obj);
    
    return 0;
}

static const Cmd Cmds[] =
{
	{"flygoat"},
	{"bootdev_sel", "", 0, "Boot device selection menu", cmd_bootdev_sel, 0, 99, 0},
	{0, 0}
};

static void init_cmd __P((void)) __attribute__ ((constructor));

static void
init_cmd()
{
	cmdlist_expand(Cmds, 1);
}