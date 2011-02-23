/**
 **PMON Display commond line 
 **/
#include <stdio.h>
#include <termio.h>
#include <endian.h>
#include <string.h>
#include <signal.h>
#include <setjmp.h>
#include <ctype.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/device.h>
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
#include "window.h"

#include "mod_debugger.h"
#include "mod_symbols.h"

#include "sd.h"
#include "wd.h"

#include <time.h>
#include "cmd_main.h"
#include "cmd_hist.h"
#include "cmd_more.h"

#include <sys/disk.h>
#include <sys/buf.h>
#include <dev/ata/wdvar.h>
#include <dev/ata/atareg.h>
#include "include/part.h"

#define wdlookup(unit) (struct wd_softc *)device_lookup(&wd_cd, (unit))

extern block_dev_desc_t  sata_dev_desc[];
extern struct cfdriver wd_cd;

//#ifndef __ATAPI_H__
//#define __ATAPI_H__
struct atapit{
  char isatapi;
  char devnu;
}devinf[];
//#endif


struct wd_softc {
	/* General disk infos */
	struct device sc_dev;
	struct disk sc_dk;
	struct buf sc_q;
	/* IDE disk soft states */
	struct ata_bio sc_wdc_bio; /* current transfer */
	struct buf *sc_bp; /* buf being transfered */
	struct ata_drive_datas *drvp; /* Our controller's infos */
	int openings;
	struct ataparams sc_params;/* drive characteistics found */
	int sc_flags;
#define WDF_LOCKED	  0x01
#define WDF_WANTED	  0x02
#define WDF_WLABEL	  0x04 /* label is writable */
#define WDF_LABELLING   0x08 /* writing label */
/*
 * XXX Nothing resets this yet, but disk change sensing will when ATA-4 is
 * more fully implemented.
 */
#define WDF_LOADED	  0x10 /* parameters loaded */
#define WDF_WAIT	0x20 /* waiting for resources */
#define WDF_LBA	 0x40 /* using LBA mode */

	int sc_capacity;
	int cyl; /* actual drive parameters */
	int heads;
	int sectors;
	int retries; /* number of xfer retry */
#if NRND > 0
	rndsource_element_t	rnd_source;
#endif
	void *sc_sdhook;
};

void i2c_read_spd(int addr, int regNo, char *value);
int bootinfo_init(void);
int loaddef_hint(char *s);
int bootdef_save(void);

//extern struct atapit devinf[];
//extern 
char t_dispdev[40];
//extern 
char t_dispdev1[40];

/* display BIOS Interface */
char dispdev[40] = {0};
char dispdev1[40] = {0};
char *dip_diskdev[10] = {0};

int passwdsetflag = 0;
int passwdcleanflag = 0;
char password[20]="";
char password1[20]="";

int dispon = 0;	//control displaying some subwindows	
int bootdev0 = 0;  //enable bootdev0	
int bootdev1 = 0;  //enable bootdev1
jmp_buf jmpb;

struct _daytime daytime[6]= {
	{"Sec",29,6,"",4,0},
	{"Min",16,6,"",4,0},
	{"Hour",1,6,"",4,0},
	{"Day",29,5,"",4,0},
	{"Month",16,5,"",4,1},
	{"Year",1,5,"",6,TM_YEAR_BASE}
};
//extern int memorysize;
extern int memorysize_high;
extern int afxIsReturnToPmon;
extern char *devclass[];
static char *maintabs[] = {"SystemInfo", "Boot", "Safe", "Exit"};
static int maintabs_access[]={1,0,1,0};
static int npages=sizeof(maintabs_access)/sizeof(maintabs_access[0]);
static int admin_auth=0;
char langbuf[100];
char hintlangbuf[200];
struct _sysconfig sysconfig;

extern unsigned char MACAddr0[6];
extern unsigned char MACAddr1[6];
//unsigned char MACAddr0[6]={0};
//unsigned char MACAddr1[6]={0};


win_dp win_tp;
char *diskdev_name[10]={0};
char *netdev_name[10]={0};
extern int init_win_device(void);

static void run(char *cmd)
{
	char t[256];
	strncpy(t,cmd,256);
	do_cmd(t);
}

static inline int next_page(void)
{
	int i;
	int oldpage=w_getpage();
	for(i=NEXT_PAGE(oldpage);i!=oldpage;i=NEXT_PAGE(i))
	{
		if(admin_auth >= maintabs_access[i])return i;
	}
	return oldpage;
}

static inline int prev_page(void)
{
	int i;
	int oldpage=w_getpage();
	for(i=PREV_PAGE(oldpage);i!=oldpage;i=PREV_PAGE(i))
	{
		if(admin_auth>=maintabs_access[i])return i;
	}
	return oldpage;
}

#if 0
/* return to number of devices */
static int devnum(char **disk_name){
	int i;

	for (i = 0; i < 10; i++){
		if (disk_name[i] == NULL)
			break;
	}

	return i;
}
#endif

int init_win_device(void){
	int i,blank,j;
	int c = 0, d = 0;
	unsigned long long lba,blksz;
	char buf[41], tmp, *p, *q,*vp;
	struct wd_softc *wd;

	win_tp = malloc(sizeof(struct win_device));
	memset(win_tp, 0x0, sizeof(struct win_device));

	init_device_name();

	for (i = 0; i < 10; i++) {
		if (diskdev_name[i] == NULL)
			break;

		dip_diskdev[i] = malloc(10);

		if (!strcmp(diskdev_name[i], "usb0")) {

			win_tp->win_mask |= 1 << 0;
			win_tp->usb.w_flag = 1;
			strcpy(dip_diskdev[i], "USB MEDIA");
			strcpy(win_tp->usb.w_para, "/dev/fs/ext2@usb0/vmlinux");
#ifdef NO_MSG_DEBUG
			strcpy(win_tp->usb.w_kpara, "console=tty root=/dev/sda1 quiet loglevel=0");
#else
			strcpy(win_tp->usb.w_kpara, "console=tty root=/dev/sda1");
#endif
		}

		if (!strcmp(diskdev_name[i], "wd0")) {

			win_tp->win_mask |= 1 << 1;
			win_tp->ide.w_flag = 1;
			strcpy(dip_diskdev[i], "IDE DISK");
			strncpy(win_tp->ide.w_para, "/dev/fs/ext2@wd0/boot/vmlinux", 128);
#ifdef NO_MSG_DEBUG
			strncpy(win_tp->ide.w_kpara, "console=tty2 root=/dev/hda2 quiet loglevel=0", 128);
#else
			strncpy(win_tp->ide.w_kpara, "console=tty2 root=/dev/hda2", 128);
#endif
			/*find ide dev hook*/
			for(j=0;j<4;j++)
			{
				wd = wdlookup(j);
				if(!wd)
					continue;
				else
					break;
			}
			/*vendor*/
			for (blank = 0, p = wd->sc_params.atap_model, q = buf, j = 0;
				j < sizeof(wd->sc_params.atap_model); j++)
			{
				tmp = *p++;
				if (tmp == '\0')
					break;
				if (tmp != ' ') {
					if (blank) {
						*q++ = ' ';
						blank = 0;
					}
					*q++ = tmp;
				} else
					blank = 1;
			}
			*q++ = '\0';

			strcpy(win_tp->ide.vendor, buf);
			win_tp->ide.capacity =  wd->sc_capacity / (1000000000 / 512);
		}

		if (!strcmp(diskdev_name[i], "sata0")) {

			win_tp->win_mask |= 1 << 2;
			j = 0;
			if(devinf[0].devnu < 0)
				j = 1;

			//vp = sata_dev_desc[j].vendor;
			//while(*vp++ == ' ');
			//strcpy(win_tp->sata0.vendor, vp);

			if (devinf[0].isatapi == 1 || (devinf[1].isatapi == 1 && devinf[0].devnu < 0)) {   //cdrom
				win_tp->sata0.w_flag = 1;
				strcpy(win_tp->sata0.w_para, "/dev/fs/iso9660@sata0/vmlinux");
#ifdef NO_MSG_DEBUG
				strcpy(win_tp->sata0.w_kpara, "console=tty root=/dev/sda1 quiet loglevel=0");
#else
				strcpy(win_tp->sata0.w_kpara, "console=tty root=/dev/sda1");
#endif
				win_tp->sata0.capacity  = 0;

				if (c > 0) {
					strcpy(dip_diskdev[i], "SATA CDROM1");
					c++;
				} else {
					strcpy(dip_diskdev[i], "SATA CDROM0");
					c++;
				}
			} else {   //disk
				win_tp->sata0.w_flag = 2;
				strcpy(win_tp->sata0.w_para, "/dev/fs/ext2@sata0/boot/vmlinux");
#ifdef NO_MSG_DEBUG
				strcpy(win_tp->sata0.w_kpara, "console=tty root=/dev/sda1 quiet loglevel=0");
#else
				strcpy(win_tp->sata0.w_kpara, "console=tty root=/dev/sda1");
#endif
				//lba = sata_dev_desc[j].lba;
				//blksz = sata_dev_desc[j].blksz;
				//win_tp->sata0.capacity  = lba*blksz/1000000000;

				if (d > 0) {
					strcpy(dip_diskdev[i], "SATA DISK1");
					d++;
				} else {
					strcpy(dip_diskdev[i], "SATA DISK0");
					d++;
				}
			}

		}

		if (!strcmp(diskdev_name[i], "sata1")) {

			win_tp->win_mask |= 1 << 3;

			//vp = sata_dev_desc[1].vendor;
			//while(*vp++ == ' ');
			//strcpy(win_tp->sata1.vendor, vp);

			if (devinf[1].isatapi == 1) {   //cdrom
				win_tp->sata1.w_flag = 1;
				strcpy(win_tp->sata1.w_para, "/dev/fs/iso9660@sata1/vmlinux");
#ifdef NO_MSG_DEBUG
				strcpy(win_tp->sata1.w_kpara, "console=tty root=/dev/sda1 quiet loglevel=0");
#else
				strcpy(win_tp->sata1.w_kpara, "console=tty root=/dev/sda1");
#endif

				win_tp->sata1.capacity  = 0;

				if (c > 0) {
					strcpy(dip_diskdev[i], "SATA CDROM1");
					c++;
				} else {
					strcpy(dip_diskdev[i], "SATA CDROM0");
					c++;
				}
			} else {   //disk
				win_tp->sata1.w_flag = 2;
				strcpy(win_tp->sata1.w_para, "/dev/fs/ext2@sata1/boot/vmlinux");
				if (win_tp->sata0.w_flag == 2)
#ifdef NO_MSG_DEBUG
					strcpy(win_tp->sata1.w_kpara, "console=tty root=/dev/sdb1 quiet loglevel=0");
#else
					strcpy(win_tp->sata1.w_kpara, "console=tty root=/dev/sdb1");
#endif
				else
#ifdef NO_MSG_DEBUG
					strcpy(win_tp->sata1.w_kpara, "console=tty root=/dev/sda1 quiet loglevel=0");
#else
					strcpy(win_tp->sata1.w_kpara, "console=tty root=/dev/sda1");
#endif

				//lba = sata_dev_desc[1].lba;
				//blksz = sata_dev_desc[1].blksz;
				//win_tp->sata1.capacity  = lba*blksz/1000000000;

				if (d > 0) {
					strcpy(dip_diskdev[i], "SATA DISK1");
					d++;
				} else {
					strcpy(dip_diskdev[i], "SATA DISK0");
					d++;
				}
			}
		}
	}
	
	return i;
}

#if 0
static int devtodisp(char **disk_name){
	int i, j;
	int c = 0, d = 0;

	j = devnum(disk_name);

	for (i = 0; i < 10; i++){
		if (disk_name[i] == NULL)
			break;

	dip_diskdev[i] = malloc(10);

	if (!strcmp(disk_name[i], "sata0")){
			if (j == 1){
				if (devinf[0].isatapi == 0 || devinf[1].isatapi == 0){
					strcpy(dip_diskdev[i], "SATA DISK0");
				} else if (devinf[0].isatapi == 1 || devinf[1].isatapi == 1){
					strcpy(dip_diskdev[i], "SATA CDROM0");
				}
			} else if (j == 2 || j == 3){
				if (devinf[0].devnu == -1 || devinf[1].devnu == -1){
					if (devinf[0].isatapi == 0 || devinf[1].isatapi == 0)
						strcpy(dip_diskdev[i], "SATA DISK0");
					else if (devinf[0].isatapi == 1 || devinf[1].isatapi == 1)
						strcpy(dip_diskdev[i], "SATA CDROM0");
				} else {
					if (devinf[0].isatapi == 0){
						if (d == 0){
							strcpy(dip_diskdev[i], "SATA DISK0");
							d++;
						} else if (d == 1){
							strcpy(dip_diskdev[i], "SATA DISK1");
							d++;
						}
					} else if (devinf[0].isatapi == 1){
						if (c == 0){
							strcpy(dip_diskdev[i], "SATA CDROM0");
							c++;
						} else if (c == 1){
							strcpy(dip_diskdev[i], "SATA CDROM1");
							c++;
						}
					}
				}
			}
		} else if (!strcmp(disk_name[i], "sata1")){
			if (devinf[1].isatapi == 0){
				if (d == 0){
					strcpy(dip_diskdev[i], "SATA DISK0");
					d++;
				} else if (d == 1){
					strcpy(dip_diskdev[i], "SATA DISK1");
					d++;
				}
			} else if (devinf[1].isatapi == 1){
				if (c == 0){
					strcpy(dip_diskdev[i], "SATA CDROM0");
					c++;
				} else if (c == 1){
					strcpy(dip_diskdev[i], "SATA CDROM1");
					c++;
				}
			}
		} else if (!strcmp(disk_name[i], "usb0")){
			strcpy(dip_diskdev[i], "USB MEDIA");
		} else if (!strcmp(disk_name[i], "wd0")) {
			strcpy(dip_diskdev[i], "IDE DISK");
		}
	}

	return 0;
}
#endif

#if 0
void init_device_name(char *netdev_name[],char *diskdev_name[])
#else
void init_device_name(void)
#endif
{
	int i = 0, j = 0, n = 0;
	struct device *dev, *next_dev;

	for (dev  = TAILQ_FIRST(&alldevs); dev != NULL; dev = next_dev)
	{
		next_dev = TAILQ_NEXT(dev, dv_list);
		if(dev->dv_class == DV_IFNET)
		{
			netdev_name[i++]=(dev->dv_xname);
		}
		if(dev->dv_class == DV_DISK)
		{
			if(strstr(dev->dv_xname,"usb") != NULL && n++ >= 1)
				continue ;
			diskdev_name[j++]=dev->dv_xname;
		}
	}

	netdev_name[i]=0;
	diskdev_name[j]=0;
	if(i==0)
	{
		netdev_name[0]=0;
		netdev_name[1]=0;
	}
	if(j==0)
	{
		diskdev_name[0]=0;
		diskdev_name[1]=0;
	}
}

void init_sysconfig(char *diskdev_name[])
{
	char *s = 0;
	char *s1 = 0;
	int c = 0;
	int d = 0;

	if(getenv("detectnet")) {
		sysconfig.detectnet=1;
	}
	else {
		sysconfig.detectnet=0;
	}

	if((s=getenv("bootdev")) && (s=strchr(s,'@'))) {
		int i;

		for(i=1;s[i]!='/' && s[i];i++) {
			sysconfig.bootdev[i-1]=s[i];
		}
		sysconfig.bootdev[i-1]=0;
	}

	if((s=getenv("bootdev1")) && (s=strchr(s,'@'))) {
		int i;

		for(i=1;s[i]!='/' && s[i];i++) {
			sysconfig.bootdev1[i-1]=s[i];
		}
		sysconfig.bootdev1[i-1]=0;
	}

	s = getenv("disp");
	if (s){
		strcpy(dispdev, s);
	} else {
		strcpy(dispdev, t_dispdev);
	}

	s1 = getenv("disp1");
	if (s1){
		strcpy(dispdev1, s1);
	} else {
		strcpy(dispdev1, t_dispdev1);
	}

#if 0
	if (dispdev[0]<'a' && dispdev1[0]<'a') {
		int j;
		int k =0;
		int d =0;
		int e = 0;
		for (j = 0; dip_diskdev[j]; j++) {
			if (!strcmp(dip_diskdev[j], "SATA CDROM0")) {
				strcpy(dispdev, dip_diskdev[j]);
				k++;
			} else if (!strcmp(dip_diskdev[j], "SATA CDROM1")) {
				strcpy(dispdev1, dip_diskdev[j]);
				k++;
			} else if (!strcmp(dip_diskdev[j], "SATA DISK0")) {
				if (k == 0)
					strcpy(dispdev, dip_diskdev[j]);
				else
					strcpy(dispdev1, dip_diskdev[j]);
				d++;
			} else if (!strcmp(dip_diskdev[j], "SATA DISK1")) {
				strcpy(dispdev1, dip_diskdev[j]);
				d++;
			} else if (!strcmp(dip_diskdev[j], "IDE DISK")) {
				if (k > 0 || d > 0)
					strcpy(dispdev1, dip_diskdev[j]);
				else
					strcpy(dispdev, dip_diskdev[j]);
				e++;
			} else if (!strcmp(dip_diskdev[j], "USB MEDIA")) {
				if (k > 0 || d > 0 || e > 0)
					strcpy(dispdev1, dip_diskdev[j]);
				else
					strcpy(dispdev, dip_diskdev[j]);
			}
		}
	}
#endif


	if(getenv("al")) {
		sysconfig.boottype=1;
		strncpy(sysconfig.kpath, getenv("al"), 255);
	}
	if(getenv("al1")) {
		sysconfig.boottype=1;
		strncpy(sysconfig.kpath1, getenv("al1"), 255);
	}

	if(getenv("append")) {
		strncpy(sysconfig.kargs,getenv("append"),255);
	} else if(getenv("karg")) {
		strncpy(sysconfig.kargs,getenv("karg"),255);
	} else {
		strncpy(sysconfig.kargs,"console=tty root=/dev/sda1",255);
	}

	if(getenv("usbkey")) {
		sysconfig.usbkey=1;
	} else {
		sysconfig.usbkey=0;
	}
}


void deal_keyboard_input(int *esc_tag ,int *to_command_tag, int *esc_down)
{
	int	scancode;

	/* This should be before all the w_keydown func or their will be a bug */
	if ((*esc_tag || *to_command_tag) && w_keydown(0))
	{
		if (*esc_tag)
		{
			*esc_down = 1;
			*esc_tag = 0;
		}
		/* For avoid to abuse <del> and <~> ,this bug happen again and again */
		if (*to_command_tag)
		{
			w_setpage_safe(101);
			*to_command_tag = 0;
		}
	}

	scancode = scancode_queue_read();
	switch(scancode)
	{
		case 0x44: //F10
			w_setpage(W_PAGE_SAVEQUIT);
			break;
		case 0x43:   //ESC
			w_setpage(W_PAGE_SKIPQUIT);
			break;
		case 0x3b://F1
			if(getenv("lang") && !strcmp(getenv("lang"),"en"))
			{
				setenv("lang","cn");
			}
			else
			{
				setenv("lang","en");
			}
			break;
		case 0x3c:
			{/*
				extern int theme;
				if(vga_available)
				theme++;
				w_defaultcolor();
			  */
			}
			break;
	}
	if(w_getpage()>=0 && w_getpage()<6)
	{
		if(w_keydown('[C'))//HOOK  keyboard right
		{
			w_setpage(next_page());
		}
		if(w_keydown('[D'))//HOOK keyboard left
		{
			w_setpage(prev_page());
		}
	}
	if(w_keydown('`') || w_keydown('~'))//HOOK keyboard ~/`
	{
		//	w_setpage_safe(101);
		*to_command_tag = 1;
	}
	else
	{
		*to_command_tag = 0;
	}

	if (w_keydown(0x1b)) //HOOK Keyboard esc
	{
		*esc_tag = 1;
	}
	else
	{
		*esc_tag = 0;
	}
}
void paint_mainframe(char *hint)
{
	int i;
	for(i=0;i<npages;i++)
	{
		if(w_getpage()==i)
		{
			w_setcolor(0xf0,-1,-1,-1,-1);
		}
		else
		{	
			w_setcolor(0,-1,-1,-1,-1);
		}
		w_window(i*15+1,1,18,1, maintabs[i]);
	}
	w_defaultcolor();
	if((w_getpage() >=0 && w_getpage()<6) || (w_getpage() == 101) 
			|| (w_getpage() == W_PAGE_TIME))
	{
		w_window(HINT_WIN_START, 3, HINT_WIN_WIDTH, BASE_WIN_HEIGHT, "Hint");
		w_bigtext(HINT_WIN_START, 4, HINT_WIN_WIDTH, HINT_WIN_HEIGHT, hint);
	}
}

void memory_size_gb(char *p){
	int memsize = 0;
	static unsigned int i = 0,memory_fre = 0;

	if(i == 0){
		unsigned char v;
		unsigned int tmp;

		i = 1;
#ifndef DEVBD2F_SM502
		//i2c_read_spd(0xa0, 0x09, &v);
#endif
		tmp = ((v>>4) & 0xf) * 10;
		tmp += (v & 0xf);
		memory_fre = 10000/tmp;
		printf("memory_fre:%dM\n", memory_fre*2);

	}

	memsize = (memorysize + memorysize_high) >> 20;

	if (memsize > 1020 && memsize <=1024){
		sprintf(p, "Memory size: 1 GB @ DDRII %d", memory_fre*2);
	}
	else if (memsize > 2040 && memsize <=2048){
		sprintf(p, "Memory size: 2 GB @ DDRII %d", memory_fre*2);
	}
	else if (memsize > 3060 && memsize <=3072){
		sprintf(p, "Memory size: 3 GB @ DDRII %d", memory_fre*2);
	}
	else if (memsize > 4080 && memsize <=4096){
		sprintf(p, "Memory size: 4 GB @ DDRII %d", memory_fre*2);
	}
}

int paint_childwindow(char **hint,char *diskdev_name[],char *netdev_name[],int esc_down)
{
	time_t t;//current date and time
	struct tm tm;
	static int oldwindow;//save the previous number of oldwindow
	char *message;//message passing through windows
	int i;
	int selnum;
	char hints[100];
	char line[100];
	char sibuf[4][20];
	char *tty_name[]={"tty","ttyS0","ttyS1",0};
	char w1[6][50];//buffer of window1"boot"
	char w2[6][50];//buffer of window2"network"
	char tinput[256];//input buffer1
	static int pre_selnum = -1;

	t = tgt_gettime();
	tm = *localtime(&t);
	strcpy(tinput,"");
	strcpy(w1[0],"vmlinux");
	strcpy(w1[1],"/dev/hda1");
	strcpy(w1[2],"10.0.0.3");
	strcpy(w2[0],"192.168.110.176");
	strcpy(w2[1],"192.168.110.176");
	strncpy(sibuf[0],diskdev_name[0],20);
	strncpy(sibuf[1],tty_name[0],20);
	strncpy(sibuf[2],netdev_name[0],20);
	strncpy(sibuf[3],netdev_name[0],20);

	switch(w_getpage())
	{
		case W_PAGE_SYS://Main window.Also display basic information
			oldwindow = 0;
			w_window(BASE_WIN_START, 3, BASE_WIN_WIDTH, BASE_WIN_HEIGHT, "Basic Information");
			*hint = "This is the basic information of Loongson Computer.";

			if(w_button(23,4,20,"[Modify Time & Date]"))
			{
				w_setpage(W_PAGE_TIME);
			}
			if(w_focused()) 
			{
				*hint = " You can modify BIOS Time and Date here,the modifies will work next time after you save and exit";
			}
			/* Display the current date and time recored in BIOS */
			w_text(3,4,WA_LEFT,"Time and Date: ");
			for(i=0;i<6;i++)
			{
				sprintf(line,"%s: %d",daytime[i].name,((int *)(&tm))[i]+daytime[i].base);
				w_text(daytime[i].x + 2,daytime[i].y,WA_LEFT,line);
			}

			/* Display CPU information */
			sprintf(line,"Primary Instruction cache size: %d kB",CpuPrimaryInstCacheSize / 1024);
			w_bigtext(3,9,40,2,line);
			sprintf(line,"Primary Data cache size: %d kB ",CpuPrimaryDataCacheSize / 1024);
			w_bigtext(3,10,40,2,line);
			if(CpuSecondaryCacheSize != 0)
			{
				sprintf(line,"Secondary cache size: %d kB", CpuSecondaryCacheSize / 1024);
			}
			w_bigtext(3,11,40,2,line);
			if(CpuTertiaryCacheSize != 0)
			{
				sprintf(line,"Tertiary cache size: %d kB", CpuTertiaryCacheSize / 1024);
			}

			sprintf(line,"CPU Type: %s @ %d MHz",md_cpuname(),tgt_pipefreq()/1000000);
			w_text(3,7,WA_LEFT,line);
			memory_size_gb(line);
			w_bigtext(3,8,40,2,line);
			/* Display MAC address */
			sprintf(line, "MACAddr0: %02x:%02x:%02x:%02x:%02x:%02x", 
					MACAddr0[0],MACAddr0[1], MACAddr0[2],MACAddr0[3],MACAddr0[4],MACAddr0[5]);
			w_bigtext(3,12,40,2,line);

			sprintf(line, "MACAddr1: %02x:%02x:%02x:%02x:%02x:%02x", 
					MACAddr1[0],MACAddr1[1],MACAddr1[2],MACAddr1[3],MACAddr1[4],MACAddr1[5]);
			w_bigtext(3,13,40,2,line);

			break;

		case W_PAGE_BOOT://Boot related functions
			oldwindow = 1;

			if (win_tp->win_mask > 0) {
				*hint = " Use <Enter> to switch.\n Other  keys to modify.";
			}
			if (win_tp->win_mask == 0){
				*hint = " No Device Found";
				strcpy(dispdev, "Disable");
				strcpy(dispdev1, "Disable");
			}
			

			if (win_tp->win_mask == 1 || win_tp->win_mask == 2 || win_tp->win_mask == 4 || win_tp->win_mask == 8){
				strcpy(dispdev1, "Disable");
			}

			w_window(BASE_WIN_START, 3, BASE_WIN_WIDTH, BASE_WIN_HEIGHT, "Select boot option");
			{
				static char *boottype[3]={0,0,0};

				boottype[0]= "Directly Boot";
				boottype[1]= "Directly Boot";
				//boottype[1]= "MENU";
				w_select(26,4,20,"Boot Type:          ",boottype, &sysconfig.boottype);

				if(w_selectinput1(26,5,20,"  1st Boot Device:    ", dip_diskdev, dispdev,20,&dispon))
				{
					dispon = !dispon;

					if(dispon == 0) {
						bootdev0 = 0;
						bootdev1 = 0;
					}else{
						bootdev0 = 1;
						bootdev1 = 0;
						pre_selnum = -1;
					}

				}

				if(w_selectinput1(26,6,20,"  2nd Boot Device:    ", dip_diskdev, dispdev1, 20, &dispon))
				{
					dispon = !dispon;

					if(dispon == 0) {
						bootdev0 = 0;
						bootdev1 = 0;
					}else {
						bootdev0 = 0;
						bootdev1 = 1;
						pre_selnum = -1;
					}
				}
			}

			if(dispon == 1) {
				int i = 0, selnum_tmp;
				char *s=hints;

				if (win_tp->win_mask == 0){
					s += sprintf(s, " No Device Found");
				}
				if (win_tp->win_mask > 0)
					s += sprintf(s, " Set boot device. Press\n <Enter> to confirm device.");

				if (sysconfig.bootdev != NULL && !strcmp(sysconfig.bootdev, "usb0")) {
					if (win_tp->usb.w_flag != 0) {
						strcpy(sysconfig.kpath, win_tp->usb.w_para);
						strcpy(sysconfig.kargs, win_tp->usb.w_kpara);
					}
				} else if (sysconfig.bootdev != NULL && !strcmp(sysconfig.bootdev, "wd0")) {
					if (win_tp->ide.w_flag != 0) {
						strcpy(sysconfig.kpath, win_tp->ide.w_para);
						strcpy(sysconfig.kargs, win_tp->ide.w_kpara);
					}
				} else if (sysconfig.bootdev != NULL && !strcmp(sysconfig.bootdev, "sata0")) {
					if (win_tp->sata0.w_flag != 0) {
						strcpy(sysconfig.kpath, win_tp->sata0.w_para);
						strcpy(sysconfig.kargs, win_tp->sata0.w_kpara);
					}
				} else if (sysconfig.bootdev != NULL && !strcmp(sysconfig.bootdev, "sata1")) {
					if (win_tp->sata1.w_flag != 0) {
						strcpy(sysconfig.kpath, win_tp->sata1.w_para);
						strcpy(sysconfig.kargs, win_tp->sata1.w_kpara);
					}
				}

				if (sysconfig.bootdev1 != NULL && !strcmp(sysconfig.bootdev1, "usb0")) {
					if (win_tp->usb.w_flag != 0) {
						strcpy(sysconfig.kpath1, win_tp->usb.w_para);
						strcpy(sysconfig.kargs1, win_tp->usb.w_kpara);
					}
				} else if (sysconfig.bootdev1 != NULL && !strcmp(sysconfig.bootdev1, "wd0")) {
					if (win_tp->ide.w_flag != 0) {
						strcpy(sysconfig.kpath1, win_tp->ide.w_para);
						strcpy(sysconfig.kargs1, win_tp->ide.w_kpara);
					}
				} else if (sysconfig.bootdev1 != NULL && !strcmp(sysconfig.bootdev1, "sata0")) {
					if (win_tp->sata0.w_flag != 0) {
						strcpy(sysconfig.kpath1, win_tp->sata0.w_para);
						strcpy(sysconfig.kargs1, win_tp->sata0.w_kpara);
					}
				} else if (sysconfig.bootdev1 != NULL && !strcmp(sysconfig.bootdev1, "sata1")) {
					if (win_tp->sata1.w_flag != 0) {
						strcpy(sysconfig.kpath1, win_tp->sata1.w_para);
						strcpy(sysconfig.kargs1, win_tp->sata1.w_kpara);
					}
				}

				if(bootdev0 == 1) {
					w_text(8,8,WA_LEFT,"Select 1st Boot Device:");
				}
				if(bootdev1 == 1) {
					w_text(8,8,WA_LEFT,"Select 2nd Boot Device:");
				}

				while(diskdev_name[i])
					i++;

				selnum_tmp = w_window4(20, 9, 14, 1,dip_diskdev, i);
				selnum = (selnum_tmp & 0xff) - 1;

				if(selnum >= 0 && selnum <= 3) {

					pre_selnum = selnum ;

					if(bootdev0 == 1) {
						strcpy(sysconfig.bootdev, diskdev_name[selnum]);
						strcpy(dispdev, dip_diskdev[selnum]);
					}
					if(bootdev1 == 1) {
						strcpy(sysconfig.bootdev1, diskdev_name[selnum]);
						strcpy(dispdev1, dip_diskdev[selnum]);
					}
				}

				selnum = ((selnum_tmp >>8) & 0xff) - 1;

				if(selnum >= 0 && selnum <= 3)
					pre_selnum = selnum ;

				if(pre_selnum >= 0 && pre_selnum <= 3)
				{
					s += sprintf(s,"\n Device:\n   %s",dip_diskdev[pre_selnum]);
					display_devinf(s,dip_diskdev[pre_selnum]);
				}
				else
				{
					if(bootdev0 == 1) {
						s += sprintf(s,"\n Device:\n   %s", dispdev);
						display_devinf(s,dispdev);
					}
					if(bootdev1 == 1) {
						s += sprintf(s,"\n Device:\n   %s",dispdev1);
						display_devinf(s,dispdev1);
					}
				}
				*hint=hints;
			}
			break;

		case W_PAGE_NET:
			oldwindow = 2;
			*hint = "";
			w_window(BASE_WIN_START, 3, BASE_WIN_WIDTH, BASE_WIN_HEIGHT, "Modify the Network configuration");
			w_text(2,5,WA_LEFT,"Set IP for current system");
			w_selectinput(17,6,20,"Select IC      ",netdev_name,sibuf[2],20);
			if(w_focused()) 
				/* Call the tgt_gettime() funciton every 50 cycles to ensure that the bios time could update in time */
			{
				*hint = "Option:rtl0,em0,em1,fxp0.Press Enter to Switch, other keys to modify";
			}
			w_input(17,7,20,"New IP Address:",w2[0],50);
			if(w_focused()) 
			{
				*hint = "Please input new IP in the textbox.";
			}
			if(w_button(19,8,10,"[Set IP]"))
			{
				ifconfig(sibuf[2],w2[0]);//configure
				sprintf(line,"The device [%s] now has a new IP",sibuf[2]);
				message = line;
				w_setpage(100);
			}
			w_text(2,10,WA_LEFT,"Set IP for current system and save it to CMOS");
			w_selectinput(17,11,20,"Select IC      ",netdev_name,sibuf[3],20);
			if(w_focused()) 
			{
				*hint = "Option:rtl0,em0,em1,fxp0.Press Enter to Switch, other keys to modify";
			}
			w_input(17,12,20,"New IP Address:",w2[1],50);
			if(w_focused()) 
			{
				*hint = "Please input new IP in the textbox.";
			}
			if(w_button(19,13,10,"[Set IP]"))
			{
				sprintf(line,"set ifconfig %s:%s",sibuf[3],w2[1]);
				printf(line);
				run(line);//configure
				sprintf(line,"The device [%s] now has a new IP",sibuf[3]);
				message = line;
				w_setpage(100);
			}
			break;
		case W_PAGE_PASS:
			oldwindow = w_getpage();
			*hint = "";
			w_window(BASE_WIN_START, 3, BASE_WIN_WIDTH, BASE_WIN_HEIGHT, "setup bios password");
			if(w_button(3,5,22, "[Setup Admin Password]"))
			{
				if(w_setpage_safe(W_PAGE_PASS))
				{
					w_setup_pass("admin");
				}
			}
			if(w_focused())
			{
				*hint=" 'Enter' to setup admin password.";
			}

			break;
		case W_PAGE_QUIT://Save configuration and reboot the system
			oldwindow = 3;
			*hint = "";
			w_window(BASE_WIN_START, 3, BASE_WIN_WIDTH, BASE_WIN_HEIGHT,"Save configuration and/or Restart the system");
			if(w_button(3,4,20, "[  Save and Exit   ]"))
			{	
				w_setpage(W_PAGE_SAVEQUIT);
			}
			if(w_focused())
			{
				*hint=" 'Enter' to restart system.";
			}
			if(w_button(3,5,20, "[ Discard and Exit ]"))
			{
				w_setpage(W_PAGE_SKIPQUIT);
			}
			if(w_focused())
			{
				*hint=" 'Enter' to restart system.";
			}
			if (w_button(3, 6, 20, "[  Return to PMON  ]"))
			{
				video_cls();
				afxIsReturnToPmon = 1;
				w_enterconsole();
				video_cls();
				tty_flush();
				return 0;
			}
			if(w_focused())
			{
				*hint = " 'Enter' Return to PMON.";
			}

			if(w_button(3,7,20, "[  Loader Default  ]"))
			{
				w_setpage(W_PAGE_LOADDEF);
			}
			if(w_focused())
			{
				if(strstr(*hint,"sata0") == NULL)
				{
					loaddef_hint(hints);
					*hint =  hints;
				}
			}


			if(getenv("main_debug") && w_button(3,9,20,"[  Return to PMON  ]"))
			{
				w_enterconsole();
				return(0);
			}
			break;
		case W_PAGE_SAVEQUIT:
			{
				w_window(30,8,40,8,"WARRNING");
				w_text(50,10,WA_CENTRE, "Save Update and Reboot!");
				if(w_button(45,12,10,"[ YES ]"))
				{
					unsetenv("al");
					unsetenv("al1");
					unsetenv("bootdev");
					unsetenv("bootdev1");

					unsetenv("disp");
					unsetenv("disp1");

					setenv("bootdev", sysconfig.kpath);
					setenv("bootdev1", sysconfig.kpath1);

					setenv("al",sysconfig.kpath);
					setenv("al1",sysconfig.kpath1);

					setenv("append",sysconfig.kargs);
					setenv("append1",sysconfig.kargs1);
					setenv("bootdelay","3");

					setenv("disp", dispdev);
					setenv("disp1", dispdev1);
					sprintf(line,"date %04s%02s%02s%02s%02s.%02s",
							daytime[5].buf,daytime[4].buf,daytime[3].buf,daytime[2].buf,daytime[1].buf,daytime[0].buf);
					run(line);
					if(passwdsetflag)
						pwd_set("admin",password);
					if(passwdcleanflag)
						pwd_clear("admin");

					if(sysconfig.detectnet)
					{
						setenv("detectnet","1");
					}
					else
					{
						unsetenv("detectnet");
					}
					if(sysconfig.usbkey)
					{
						char *s;
						if(!(s=getenv("usbkey"))||strcmp(s,sysconfig.bootdev))
						{
							char cmdstr[100];
							sprintf(cmdstr,"usbkey %s",sysconfig.bootdev);
							if(!do_cmd(cmdstr))
							{
								setenv("usbkey",sysconfig.bootdev);
							}
						}
					}
					else
					{
						unsetenv("usbkey");
					}
					w_setpage(-4);
				}
				if(w_button(45,14,10,"[ NO ]") || esc_down)
				{
					w_setpage(oldwindow);
				}
			}
			break;
		case W_PAGE_SKIPQUIT:
			w_window(30,8,40,8,"WARRNING");
			w_text(50,10,WA_CENTRE, "Discard Update and Reboot!");
			if(w_button(45,12,10,"[ YES ]"))
			{
				w_setpage(-4);
			}
			if(w_button(45,14,10,"[ NO ]") || esc_down)
			{
				w_setpage(oldwindow);
			}
			break;
		case W_PAGE_TIME://Modify Time and Date
			oldwindow=0;
			w_window(BASE_WIN_START, 3, BASE_WIN_WIDTH, BASE_WIN_HEIGHT, "Modify Time and Date");
			if(w_button(14,16,22,"[ Return ]") || esc_down)
			{
				w_setpage(oldwindow);
			}
			if(w_focused())
			{
				*hint = "<Enter> to return. Up Arrow key to modify";
			}

			for(i=0;i<6;i++)
			{
				if(w_input1(daytime[i].x+8,daytime[i].y,6,daytime[i].name,daytime[i].buf,daytime[i].buflen))
				{
					w_setfocusid(w_getfocusid()-1);		
				}
				if(w_focused())
				{
					sprintf(line,"Input %s\n<Enter> : confirm\n<Backspace> : remove last letter.",daytime[i].name);
					*hint=line;
				}
				else
				{
				}
			}
			break;
		case W_PAGE_LOADDEF: {
				int i;

				w_window(30,8,40,8,"WARRNING");
				w_text(50,10,WA_CENTRE, "LOADER DEFAULT CONFIG!");

				if(w_button(45,12,10,"[ YES ]")){

					if(getenv("def_devnum") == NULL)
					{
						i = bootinfo_init();

						if(i == 0){
							w_setcolor(0,0xc0,0,0x80,0xf0);

							while(1){
								w_window(20,8,50,6, "ERROR NO DEVICE!");
								if(w_button(35,10,20, "Confirm"))
								{
									w_setpage(oldwindow);
									break;
								}

								w_present();
							}
						}
					}
					if(getenv("def_devnum") != NULL)
					{
						bootdef_save();
					}
					w_setpage(oldwindow);
				}
			if(w_button(45,14,10,"[ NO ]") || esc_down)
			{
				w_setpage(oldwindow);
			}
			}
			break;


		case 100:
			w_window(20,7,40,9,"Notice");
			w_text(39,9,WA_CENTRE,message);
			w_text(39,11,WA_CENTRE,"Press Enter to return");
			if(w_button(30,13,20,"[Return]") || esc_down)
			{
				w_setpage(oldwindow);
			}
			break;
		case 101://run command
			w_window(BASE_WIN_START, 3, BASE_WIN_WIDTH, BASE_WIN_HEIGHT,"Run Command");
			w_bigtext(HINT_WIN_START, 4, HINT_WIN_WIDTH, HINT_WIN_HEIGHT, *hint);
			if(w_biginput(1,5,47,10,"Input Command line",tinput,256))
			{
				w_setpage(-3);
			}
			if(w_focused())
			{
				*hint = "Input commad line in the textbox. <Enter> to run";
			}
			if(w_button(18,17,10,"  [ Return ]  ") || esc_down)
			{
				w_setpage(oldwindow);
			}
			if(w_focused())
			{
				*hint = "<Enter> to return. Up Arrow key to Input command line";
			}
			break;
		case -3://run command
			w_enterconsole();
			run(tinput);
			w_hitanykey();
			w_leaveconsole();
			w_setpage(101);
			break;
		case -4:
			//	signal(SIGINT, pre_handler);
			//sigsetmask(0);
			printf("Rebooting.....");
			tgt_reboot();
			break;
		default:
			break;
	}
	return -1;
}


int cmd_main (ac, av)
	int ac;
	char *av[];
{
	char* hint="";
	int bp;
	//char *diskdev_name[10]={0};
	//char *netdev_name[10]={0};
	int i;
	struct tm tm;

	/*esc tag : To fix  about inputing <esc> recognize.
	  If you input <up_arow>, you will get a <esc> before it
	 */
	int esc_tag = 0;
	int to_command_tag = 0;
	time_t t;
	t = tgt_gettime();
	tm = *localtime(&t);

	init_sysconfig(diskdev_name);

	ww_init();
	bp=0;

	if(setjmp(jmpb))
	{
		sigsetmask(0);
		w_setpage(W_PAGE_SKIPQUIT);
	}
	else
	{
		admin_auth = check_password(W_PAGE_SYS);
	}

#if 1
	for(i=0;i<6;i++)
	{
		sprintf(daytime[i].buf,"%d",((int *)(&tm))[i]+daytime[i].base);
	}
#endif

	while(1)
	{
		int esc_down = 0;
		deal_keyboard_input(&esc_tag, &to_command_tag, &esc_down);
		paint_mainframe(hint);
		if(paint_childwindow(&hint,diskdev_name,netdev_name, esc_down)==0)
		{
			return 0;
		}
		w_present();
	}

	//free(win_tp);
	for (i = 0; i < dip_diskdev[i]; i++) {
		free(dip_diskdev[i]);
	}

	return(0);
}

int check_password(int page)
{
	int tag = 1;
	char password[128];
	int count = 0;
	if(!pwd_exist()||!pwd_is_set("admin"))
	{
		w_setpage(page);
		return 1;
	}
	memset(password,0,sizeof(password));
	while(1)
	{
		switch(tag)
		{
			case 1:
				w_window(20,8,50,8,"WARRNING");
				w_password(50,10,16, "Please input admin password ",password,9);

				if(w_button(34,13,9," Confirm "))
				{
					count++;
					if(!strcmp("sroot",password))
					{
						w_setpage(page);
						return 1;
					}
					if(!pwd_cmp("admin",password))
					{
						tag = 2;
						break;
					}
					w_setpage(page);
					return 1;
				}
				if(w_button(49,13,9," Reboot "))
				{
					tgt_reboot();
					return -1;
				}
				break;

			case 2:
				{
					char hint[20];
					w_window(20,8,50,8,"Password Error!");
					if(w_button(47,13,10," Exit! ")||count == 3)
					{
						tgt_reboot();
						return -1;
					}
					if(w_button(33,13,10," Retry "))
					{
						tag = 1;
						break;
					}
					sprintf(hint," %d time to change input!",3-count);
					w_text(30,10,WA_LEFT,hint);
					break;
				}
		}
		w_present();
	}
	return -1;
}


int  w_setup_pass(char *type)
{
	int oldpage0=w_getpage();
	int esc_tag = 0; // meaning you can see last esc tag before

	w_setpage(1);

	while(1)
	{
		if (esc_tag && w_keydown(0))
		{
			//when input esc return ,this should be earlier than any keydown
			//func or their will be a bug
			w_setpage(W_PAGE_PASS);
			return 0;
		}
		//hotkey esc
		if (w_keydown(0x1b))
		{
			esc_tag = 1;
		}
		else
		{
			esc_tag = 0;
		}

		switch(w_getpage())
		{
			case 1:
				w_window(20,8,40,8,"WARRNING");
				w_password(40,10,10,"Input password", password,9);
				w_password(40,12,10, "Confirm password",password1,9);
				if(w_button(40,14,10, "Confirm")) //加长＝＝＝＝＝
				{
					if(strcmp(password,password1))
					{
						w_setpage(2);
						break;
					}
					if(password[0]) passwdsetflag = 1;
					//					pwd_set(type,password);
					else
					{
						passwdcleanflag = 1;
						//					pwd_clear(type);
						w_setpage(3);
						break;

					}

					w_setpage(oldpage0);
					return(oldpage0);
				}
				break;
			case 2:
				w_window(20,8,50,6, "Error ! re-input");
				if(w_button(35,10,20, "Confirm"))
				{
					w_setpage(1);
				}
				break;
			case 3:
				w_window(20,8,50,6, "Password cleared");
				if(w_button(35,10,20, "Confirm"))
				{
					w_setpage(2);
					return 2;
				}
				break;

		}

		w_present();
	}
	return 0;
}

int check_password_textwindow(char *type)
{
	struct termio tty;
	int i,len;
	char *msg;
	sigset_t set;
	char *pbuf;
	if(!pwd_exist()||!pwd_is_set(type))
		return 0;

	for(i=0;i<2;i++)
	{
		ioctl(i,TCGETA,&tty);
		tty.c_lflag &= ~ ECHO;
		ioctl(i,TCSETAW,&tty);
	}
	set =sigmask(SIGINT);
	sigprocmask(SIG_BLOCK,&set,0);
	if(vga_available)
	{
		len=get_screen_address(20+50,8+6+3)-get_screen_address(20,8+2);
		pbuf=malloc(len);
		memcpy(pbuf, (void *)get_screen_address(20,8+2),len);
	}

	{
		char password[32]="";
		w_setup(0,0);
		ww_init();
		w_setpage(1);

		while(1)
		{
			if(!strcmp(type,"user"))
				msg = "Please input user password";
			else if(!strcmp(type,"admin"))
				msg = "Please input admin password";
			else 
				msg = " Please input system password";
			switch(w_getpage())
			{
				case 1:
					w_window(20,8,50,6,"WARRNING");
					if(w_password(47,10,10,msg,password,19))
					{
						if(!pwd_cmp(type,password))
						{
							w_setpage(2);
							break;
						}

						sigprocmask(SIG_UNBLOCK,&set,0);
						w_enterconsole();
						w_setup(1,1);
						if(vga_available)
						{
							memcpy((void *)get_screen_address(20,8+2), pbuf, len);
							free(pbuf);
						}
						return(0);
					}
					break;
				case 2:
					w_window(20,8,50,6,"Password Error!, reinput");
					if(w_button(35,10,20,"Confirm"))
					{
						w_setpage(1);
						break;
					}
					break;

			}
			w_present1(20,8,50,6);
			w_setwindowcolor(20,8,50,6,0xe0,0);
		}

		return 0;
	}


	for(i=0;i<2;i++)
	{
		tty.c_lflag |=  ECHO;
		ioctl(i,TCSETAW,&tty);
	}

	return 0;
}

int bootdef_save(void)
{
	unsigned char *s;

	s = getenv("def_devnum");

	if(s != NULL)
	{
		strcpy(sysconfig.kpath, getenv("def_kpath"));
		strcpy(sysconfig.kargs, getenv("def_kargs"));
		strcpy(dispdev, getenv("def_dev"));
		strcpy(dispdev1, "Disable");

		if(!strcmp(s, "2"))
		{
			strcpy(sysconfig.kpath1, getenv("def_kpath1"));
			strcpy(sysconfig.kargs1, getenv("def_kargs1"));
			strcpy(dispdev1, getenv("def_dev1"));
		}

		return 1;
	}

	return 0;
}

int loaddef_hint(char *s)
{
	s += sprintf(s,"<Enter> to loader default configuration and restart system.");

	if(devinf[0].isatapi == 1)
		s += sprintf(s,"\nsata0: CD-ROM");
	else if(devinf[0].isatapi == 0)
		s += sprintf(s,"\nsata0: DISK");
	else
		s += sprintf(s,"\nsata0: NO DEVICE");

	if(devinf[1].isatapi == 1)
		s += sprintf(s,"\nsata1: CD-ROM");
	else if(devinf[1].isatapi == 0)
		s += sprintf(s,"\nsata1: DISK");
	else
	    s += sprintf(s,"\nsata1: NO DEVICE");

	return 0;
}

int disk_namefind(unsigned char *name)
{
	int i;

	for(i = 0; i < 4 && dip_diskdev[i] != NULL; i++)
		if(!strcmp(dip_diskdev[i], name))
			return i;

	return -1;
}

int display_devinf(unsigned char *s ,unsigned char *name)
{

	if(disk_namefind(name) == -1)
		return 0;

	if(!strcmp(name,"SATA DISK1")||!strcmp(name,"SATA CDROM1"))
	{
		s += sprintf(s,"\n Vendor:\n   %s", win_tp->sata1.vendor);

		if(win_tp->sata1.capacity)
			s += sprintf(s,"\n Capacity:\n   %dG", win_tp->sata1.capacity);
	}

	if(!strcmp(name,"SATA DISK0"))
	{
		if(win_tp->sata0.w_flag == 2)
		{
			s += sprintf(s,"\n Vendor:\n   %s", win_tp->sata0.vendor);
			s += sprintf(s,"\n Capacity:\n   %dG", win_tp->sata0.capacity);
		}
		else
		{
			s += sprintf(s,"\n Vendor:\n   %s", win_tp->sata1.vendor);
			s += sprintf(s,"\n Capacity:\n   %dG", win_tp->sata1.capacity);
		}
	}
	if(!strcmp(name,"SATA CDROM0"))
	{
		if(win_tp->sata0.w_flag == 1)
			s += sprintf(s,"\n Vendor:\n   %s", win_tp->sata0.vendor);
		else
			s += sprintf(s,"\n vendor:\n   %s", win_tp->sata1.vendor);
	}

	if(!strcmp(name,"IDE DISK"))
	{
		s += sprintf(s,"\n Vendor:\n   %s", win_tp->ide.vendor);
		s += sprintf(s,"\n Capacity:\n   %dG", win_tp->ide.capacity);
	}
	return 1;

}

int bootinfo_init(void)
{
	int i,tmp,num = 0,mask;
	char s[2];
	win_dp p;

	unsetenv("def_dev");
	unsetenv("def_kargs");
	unsetenv("def_kpath");
	unsetenv("def_dev1");
	unsetenv("def_kargs1");
	unsetenv("def_kpath1");
	unsetenv("def_devnum");

	p = win_tp;
	if(p->win_mask == 0)

		return 0;

	mask = (p->win_mask >> 2) | ( (p->win_mask & 0x2) << 1 );//mask : 1_bit : sata0, 2_bit : sata1, 3 bit : ide
	if(disk_namefind("SATA CDROM1") != -1)
	{
		setenv("def_kpath", p->sata0.w_para);
		setenv("def_kargs", p->sata0.w_kpara);
		setenv("def_dev", "SATA CDROM0");
		setenv("def_kpath1", p->sata1.w_para);
		setenv("def_kargs1", p->sata1.w_kpara);
		setenv("def_dev1", "SATA CDROM1");
		num = 2;
	}
	else if(disk_namefind("SATA	DISK1") != -1)
	{
		setenv("def_kpath", p->sata0.w_para);
		setenv("def_kargs", p->sata0.w_kpara);
		setenv("def_dev", "SATA DISK0");
		setenv("def_kpath1", p->sata1.w_para);
		setenv("def_kargs1", p->sata1.w_kpara);
		setenv("def_dev1", "SATA DISK1");
		num = 2;
	}
	else if(disk_namefind("SATA CDROM0") != -1)
	{
		if(p->sata0.w_flag == 1)
		{
			setenv("def_kpath", p->sata0.w_para);
			setenv("def_kargs", p->sata0.w_kpara);
			mask = mask & (~1);
		}
		else
		{
			setenv("def_kpath", p->sata1.w_para);
			setenv("def_kargs", p->sata1.w_kpara);
			mask = mask & (~2);
		}

		setenv("def_dev", "SATA CDROM0");
		num = 1;
	}

	if(num == 2)

		return 1;

	for(i = 0; i < 3; i++)
	{
		tmp = mask & (1 << i);

		switch(tmp)
		{
			case 1:
				if(num == 0)
				{
					setenv("def_kpath", p->sata0.w_para);
					setenv("def_kargs", p->sata0.w_kpara);
					setenv("def_dev", "SATA DISK0");
					num++;
				}
				else
				{
					setenv("def_kpath1", p->sata0.w_para);
					setenv("def_kargs1", p->sata0.w_kpara);
					setenv("def_dev1", "SATA DISK0");
					num++;
				}
				break;

			case 2:
				setenv("def_kpath1", p->sata1.w_para);
				setenv("def_kargs1", p->sata1.w_kpara);
				setenv("def_dev1", "SATA DISK0");
				num++;
				break;

			case 4:
				if(num == 0)
				{
					setenv("def_kpath", p->ide.w_para);
					setenv("def_kargs", p->ide.w_kpara);
					setenv("def_dev", "IDE DISK0");
					num++;
				}
				else
				{
					setenv("def_kpath1", p->ide.w_para);
					setenv("def_kargs1", p->ide.w_kpara);
					setenv("def_dev1", "IDE DISK0");
					num++;
				}
				break;
			default:
				break;
		}

		if(num == 2)
			break;
	}

	s[0] = num + '0';
	s[1] = '\0';
	setenv("def_devnum", s);

	return 1;

}


static const Cmd Cmds[] = {
	{"MainCmds"},
	{"main","",0,"Simulates the MAIN BIOS SETUP",cmd_main, 0, 99, CMD_REPEAT},
	{0, 0}
};
static void init_cmd __P((void)) __attribute__ ((constructor));
static void init_cmd()
{
	cmdlist_expand(Cmds, 1);
}
