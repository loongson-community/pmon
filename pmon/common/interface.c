#include <stdio.h>
#include <fcntl.h>
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

#include "mod_debugger.h"
#include "mod_symbols.h"

#include "sd.h"
#include "wd.h"

#include "../../fb/bmp_pic.h"

#define USB_DISK "usb1"
#define COLOR 0x9c
#define U_F	0x1
#define V_F	0x2
#define Y_F	0x4
#define N_F	0x8
#define C_F	0x10
#define I_F	0x20
#define R_F	0x40
#define S_F	0x80
#define F_F	0x100

extern int video_display_bitmap(unsigned long bmp_image, int x, int y);
extern void cprintf(int y, int x, int width, char color, const char *fmt, ...);
extern void video_cls(void);
extern int getX(void);
extern int getY(void);
extern void setX(int x);
extern void setY(int y);

static int do_net_rescue(char *load, char *cmdline);
static int do_usb_rescue(char *load, char *cmdline);

static const char msg_server_ip[] = "Please Enter server IP address:";
static const char msg_client_ip[] = "Please Enter local IP address :";

void ui_select(char *load, char *cmdline)
{
	static int flag = 0;
	int x, y;
	int i;

	/*printf("Press <Enter> to execute loading image:%s\n", getenv("al"));
	printf("Press <del>  key to abort.\n");
	printf("Press <Tab>  key to recover system .\n");*/
	for (i = 0; i < 100; i++){
		cprintf(28, i, 0, 0," ");
		cprintf(29, i, 0, 0," ");
		cprintf(30, i, 0, 0," ");
		cprintf(31, i, 0, 0," ");
	}
	video_display_bitmap(SMALLBMP1_START_ADDR, (SMALLBMP1_X - 45), SMALLBMP1_Y);
	video_display_bitmap(SMALLBMP_START_ADDR_EN_02, SMALLBMP02_EN_X, SMALLBMP02_EN_Y);
	while ((flag & F_F) == 0) {
		char buf[200];
		switch (getchar()) {
		case 'c':
			if ((flag & U_F) && !(flag & C_F)) {
				flag |= C_F;
				for (i = 0; i < 100; i++){
					cprintf(23, i, 1, 0," ");
					cprintf(24, i, 1, 0," ");
					cprintf(25, i, 1, 0," ");
					cprintf(26, i, 1, 0," ");
				}
				video_display_bitmap(SMALLBMP3_START_ADDR,  SMALLBMP3_X, SMALLBMP3_Y);
				video_display_bitmap(SMALLBMP_START_ADDR_EN_04, SMALLBMP04_EN_X, SMALLBMP04_EN_Y);
				flag |= 0x1;
			}
			break;
		case 'i': /* Input IP address */
			if ((flag & V_F) && !(flag & I_F)) {
				char st[20], pi[20];

				flag |= (I_F | F_F);

				for (i=0;i<100;i++){
					cprintf(23, i, 1, 0, " ");
					cprintf(24, i, 1, 0, " ");
					cprintf(25, i, 1, 0, " ");
					cprintf(26, i, 1, 0, " ");
				}
				video_display_bitmap(SMALLBMP8_START_ADDR, SMALLBMP8_X, SMALLBMP8_Y);
				video_display_bitmap(SMALLBMP_START_ADDR_EN_09, SMALLBMP09_EN_X, SMALLBMP09_EN_Y);
				x = getX(); y = getY();
				setX(53); setY(24);
				gets(st);
				setX(x); setY(y);

				for (i=0;i<100;i++){
					cprintf(23, i, 1, 0, " ");
					cprintf(24, i, 1, 0, " ");
					cprintf(25, i, 1, 0, " ");
					cprintf(26, i, 1, 0, " ");
				}
				video_display_bitmap(SMALLBMP9_START_ADDR, SMALLBMP9_X, SMALLBMP9_Y);
				video_display_bitmap(SMALLBMP_START_ADDR_EN_10, SMALLBMP10_EN_X, SMALLBMP10_EN_Y);
				x = getX(); y = getY();
				setX(55); setY(24);
				gets(pi); 
				setX(x);  setY(y);

				vga_available = 0;
				sprintf(buf, "ifaddr rtl0 %s", st);
				do_cmd(buf);

				sprintf(buf, "set ifconfig rtl0:%s", st);
				do_cmd(buf);

				sprintf(buf, "set tftp tftp://%s/vmlinux", pi);
				do_cmd(buf);
				vga_available = 1;
				
				do_net_rescue(load, cmdline);
			}
		break;
		case 'n':
			if ((flag & U_F) && (flag & C_F) && !(flag & N_F)) {
				flag |= (N_F | F_F);
				vga_available = 1;
				video_cls();
				//cprintf(14, 27, 0, COLOR, "%s", "restart from local ide disk");
			}
			break;
		case 'r':
			if ((flag & (U_F | V_F))  && !(flag & R_F)) {
			/* If not want rescue then do default action. Action? */
				do_cmd("reboot");
			}
			break;
		case 's':
			if ((flag & V_F) && !(flag & S_F)) {
				flag |= (S_F | F_F);
				do_net_rescue(load, cmdline);
			}
			break;
		case 'u': /* From USB flag=0x1 */
			if (!(flag & U_F)) {
				flag |= U_F;
				for(i = 0; i < 100; i++){
					cprintf(23, i, 1, 0," ");
					cprintf(24, i, 1, 0," ");
					cprintf(25, i, 1, 0," ");
					cprintf(26, i, 1, 0," ");
				}
				video_display_bitmap(SMALLBMP2_START_ADDR, SMALLBMP2_X, SMALLBMP2_Y);
				video_display_bitmap(SMALLBMP_START_ADDR_EN_03, SMALLBMP03_EN_X, SMALLBMP03_EN_Y);
			}
			break;
		case 'v': /* From tftp flag=0x2*/
			if (!(flag & V_F)) {
				flag |= V_F;
				vga_available = 1;
				for (i=0;i<100;i++){
					cprintf(23, i, 1, 0, " ");
					cprintf(24, i, 1, 0, " ");
					cprintf(25, i, 1, 0, " ");
					cprintf(26, i, 1, 0, " ");
				}
				video_display_bitmap(SMALLBMP6_START_ADDR, SMALLBMP2_X, SMALLBMP2_Y);
				video_display_bitmap(SMALLBMP_START_ADDR_EN_07, SMALLBMP07_EN_X, SMALLBMP07_EN_Y);
			}
			break;
		case 'y':
			if ((flag & U_F) && (flag & C_F) && !(flag & Y_F)) {
				flag |= (Y_F | F_F);
				do_usb_rescue(load, cmdline);
			}
			break;
		case 'w':
			if(do_wd_rescue(load, cmdline) != EXIT_SUCCESS) {
				vga_available = 1;
				for (i=0;i<100;i++){
					cprintf(23, i, 1, 0, " ");
					cprintf(24, i, 1, 0, " ");
					cprintf(25, i, 1, 0, " ");
					cprintf(26, i, 1, 0, " ");
				}
				video_display_bitmap(SMALLBMP10_START_ADDR, SMALLBMP2_X, SMALLBMP2_Y);
				//cprintf(23, 20, 1, 0, "kernel not found");
				video_display_bitmap(SMALLBMP_START_ADDR_EN_11, SMALLBMP11_EN_X, SMALLBMP11_EN_Y);
			} else {
				flag |= F_F;
			}
			break;
		default:
			break;
		}
	}
	printf("out \n");
}

static int do_net_rescue(char *load, char *cmdline)
{
	int i;
	char *pb;
	for(i=0;i<100;i++){
		cprintf(23, i, 1, 0, " ");
		cprintf(24, i, 1, 0, " ");
		cprintf(25, i, 1, 0, " ");
		cprintf(26, i, 1, 0, " ");
	}
	video_display_bitmap(SMALLBMP7_START_ADDR,	SMALLBMP7_X, SMALLBMP7_Y);
	video_display_bitmap(SMALLBMP_START_ADDR_EN_08,	SMALLBMP08_EN_X, SMALLBMP08_EN_Y);
	vga_available = 0;

	pb = getenv("tftp");
	strcpy(load, "load ");
	strcat(load, pb);

	return 0;
}

static int do_wd_rescue(char *load, char *cmdline)
{
	int i;
	char *pb;
	char *filename = "/dev/fs/ext2@wd0b/vmlinux";
	int fd;

	for(i=0;i<100;i++){
		cprintf(23, i, 1, 0, " ");
		cprintf(24, i, 1, 0, " ");
		cprintf(25, i, 1, 0, " ");
		cprintf(26, i, 1, 0, " ");
	}

	if ((fd = open(filename, O_RDONLY | O_NONBLOCK)) < 0) {
		perror(filename);
		return EXIT_FAILURE;
	}

	video_display_bitmap(SMALLBMP7_START_ADDR,	SMALLBMP7_X, SMALLBMP7_Y);
	video_display_bitmap(SMALLBMP_START_ADDR_EN_08,	373, 390);

	strcpy(load, "load ");
	strcat(load, filename);

	return 0;
}


/* xuhua add 20080407 */
static int do_usb_rescue(char *load, char *cmdline)
{
	
	#include <sys/device.h>
	#include <fcntl.h>
	int i;
	static bootfd; /*xuhua 080902*/
	//static char path[256];/*080902*/
	static char *fat0="/dev/fat/disk@usb0/vmlinux";
	static char *fat1="/dev/fat/disk@usb1/vmlinux";
	static char *path0="/dev/fs/ext2@usb0/vmlinux";
	static char *path1="/dev/fs/ext2@usb1/vmlinux";

	char *dev_name = USB_DISK; /* FIXME: only match usb0 */
	
	struct device *dev, *next_dev;
	
	for (dev  = TAILQ_FIRST(&alldevs); dev != NULL; dev = next_dev) {
		next_dev = TAILQ_NEXT(dev, dv_list);
		if(dev->dv_class < DV_DISK) {
			continue;
		}
#if 0 /*080902*/
		if (strcmp(dev->dv_xname, dev_name) == 0) {
			for (i = 0; i < 100; i++){
				cprintf(23, i, 1, 0, " ");
				cprintf(24, i, 1, 0, " ");
			}
			video_display_bitmap(SMALLBMP4_START_ADDR, SMALLBMP4_X, SMALLBMP4_Y);
			vga_available = 0;
			strcpy(load, "load -n /dev/fs/ext2@usb1/vmlinux");
			break;
		}
#endif
		/*080902*/
		//printf("dev_name is :%s\n", dev->dv_xname);
		delay(3000000);
		if (strcmp(dev->dv_xname, "usb0") == 0) {
			if ((bootfd = open (path0, O_RDONLY | O_NONBLOCK)) >= 0){
				video_display_bitmap(SMALLBMP4_START_ADDR, SMALLBMP4_X, SMALLBMP4_Y);
				video_display_bitmap(SMALLBMP_START_ADDR_EN_05, SMALLBMP05_EN_X, SMALLBMP05_EN_Y);
				//vga_available = 1;
				strcpy(load, "load -n /dev/fs/ext2@usb0/vmlinux");
				break;
			}
			if ((bootfd = open (fat0, O_RDONLY | O_NONBLOCK)) >= 0){
				video_display_bitmap(SMALLBMP4_START_ADDR, SMALLBMP4_X, SMALLBMP4_Y);
				video_display_bitmap(SMALLBMP_START_ADDR_EN_05, SMALLBMP05_EN_X, SMALLBMP05_EN_Y);
				//vga_available = 1;
				strcpy(load, "load -n /dev/fat/disk@usb0/vmlinux");
				break;
			}
		}

		if (strcmp(dev->dv_xname, "usb1") == 0) {
			if ((bootfd = open (path1, O_RDONLY | O_NONBLOCK)) >= 0){
				video_display_bitmap(SMALLBMP4_START_ADDR, SMALLBMP4_X, SMALLBMP4_Y);
				video_display_bitmap(SMALLBMP_START_ADDR_EN_05, SMALLBMP05_EN_X, SMALLBMP05_EN_Y);
				//vga_available = 1;
				//printf("This is ext2 in usb1 fdisk\n");
				strcpy(load, "load -n /dev/fs/ext2@usb1/vmlinux");
				break;
			}
			if ((bootfd = open (fat1, O_RDONLY | O_NONBLOCK)) >= 0){
				video_display_bitmap(SMALLBMP4_START_ADDR, SMALLBMP4_X, SMALLBMP4_Y);
				video_display_bitmap(SMALLBMP_START_ADDR_EN_05, SMALLBMP05_EN_X, SMALLBMP05_EN_Y);
				//vga_available = 1;
				//printf("This is fat in usb1 fdisk\n");
				strcpy(load, "load -n /dev/fat/disk@usb1/vmlinux");
				break;
			}
		}
/*************************/
	}
	if (dev == NULL) {
		for (i = 0; i < 100; i++){
			cprintf(23, i, 1, 0, " ");
			cprintf(24, i, 1, 0, " ");
			cprintf(25, i, 1, 0, " ");
			cprintf(26, i, 1, 0, " ");
		}
		video_display_bitmap(SMALLBMP5_START_ADDR, SMALLBMP5_X, SMALLBMP5_Y);
		video_display_bitmap(SMALLBMP_START_ADDR_EN_06, SMALLBMP06_EN_X, SMALLBMP06_EN_Y);
	}
	return 0;
}
