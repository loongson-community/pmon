/* $Id: main.c,v 1.2 2006/08/03 14:47:48 cpu Exp $ */

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

extern void    *callvec;

#include "cmd_hist.h"       /* Test if command history selected */
#include "cmd_more.h"       /* Test if more command is selected */

#include "cs5536.h"
#include "vt82c686.h"

extern void DevicesInit(void);
extern void DeviceRelease(void);
extern int check_config (const char * file);

jmp_buf         jmpb;       /* non-local goto jump buffer */
char            line[LINESZ + 1];   /* input line */
struct termio   clntterm;   /* client terminal mode */
struct termio   consterm;   /* console terminal mode */
register_t  initial_sr;
unsigned int             memorysize;
unsigned int             memorysize_high;
char            prnbuf[LINESZ + 8]; /* commonly used print buffer */

unsigned int    show_menu;
int             repeating_cmd;
unsigned int    moresz = 10;
#ifdef AUTOLOAD
static void autoload __P((char *));
#else
static void autorun __P((char *));
#endif
extern void __init __P((void));
extern void _exit (int retval);
extern void delay __P((int));
extern void suppress_auto_start(void);
/*one key recovery support*/
extern void ui_select(char *, char *);
extern void video_cls(void);

extern void cprintf(int y, int x, int width, char color, const char *fmt,...); 
int video_display_bitmap_pic(ulong, int, int);
static int load_menu_list __P(());

#include "fb/bmp_layout.h"
int xcount;
bmp_image_t *bmp;
bmp_color_table_entry_t cte;


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
int
main()
{
    char prompt[32];

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
        
    md_setsr(NULL, initial_sr); /* XXX does this belong here? */

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
            strdchr(p); /* delete the bang */
            sprintf(tmp, "%d", histno);
            stristr(p, tmp);
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
static int load_menu_list()
{
    unsigned int dly, lastt;
    unsigned int cnt;
    struct termio sav;
    int i = 0;
    char* prg = "-\\|/";
    char* rootdev = NULL;
    char* path = NULL;
    int retid;

    dly = 200;
    printf("booting: ");
    /*ioctl (STDIN, CBREAK, &sav);
    lastt = 0;
    do {
        ioctl (STDIN, FIONREAD, &cnt);
        printf("\b%c", prg[i++ % 4]);
        delay(10000);
    } while (dly-- != 0 && cnt == 0);

    if(cnt > 0 && strchr("\n\r", getchar())) {
        cnt = 0;
    }


    ioctl (STDIN, TCSETAF, &sav);
    //        video_cls();*/
    printf("\n");
    cnt = 0;

    if(cnt == 0) {
        show_menu=1;
        if (path == NULL)
        {
            path = malloc(512);
            if (path == NULL)
            {
                return 0;
            }
        }

        memset(path, 0, 512);
        rootdev = getenv("bootdev");
        if (rootdev == NULL)
        {
            rootdev = "(wd0,0)";
        }
        //sprintf(path, "%s/boot.cfg", rootdev);
        sprintf(path, "bl -d ide %s/boot.cfg", rootdev);
        retid = do_cmd(path);
        if (retid == 0)
        {
            show_menu = 0;
            //                  video_cls();
            free(path);
            path = NULL;
            return 1;
        }
        //sprintf(path, "%s/boot/boot.cfg", rootdev);
        sprintf(path, "bl -d ide %s/boot/boot.cfg", rootdev);
        retid = do_cmd(path);
        if (retid == 0)
        {
            show_menu = 0;
            //                  video_cls();
            free(path);
            path = NULL;
            return 1;
        }   
        #if 0
        sprintf(path, "%s/boot.cfg", rootdev);
        if (check_config(path) == 1)
        {
            sprintf(path, "bl -d ide %s/boot.cfg", rootdev);
            if (do_cmd(path) == 0)
            {
                show_menu = 0;
                //                  video_cls();
                free(path);
                path = NULL;
                return 1;
            }
        }
        else
        {
            sprintf(path, "%s/boot/boot.cfg", rootdev);
            if (check_config(path) == 1)
            {
                sprintf(path, "bl -d ide %s/boot/boot.cfg", rootdev);
                if (do_cmd(path) == 0)
                {
                    show_menu = 0;
                    //                      video_cls();
                    free(path);
                    path = NULL;
                    return 1;
                }
            }
        }
        #endif
#if 0           
        if( check_ide() == 1 )// GOT IDE
        {
            if( do_cmd ("bl -d ide /dev/fs/ext2@wd0/boot.cfg") ==0 )
            {
                show_menu=0;
                video_cls();
                return 1;
            }
        }
        else if( check_cdrom () == 1 ) // GOT CDROM
        {
            if( do_cmd ("bl -d cdrom /dev/fs/ext2@wd0/boot.cfg") ==0 )
            {
                show_menu=0;
                video_cls();
                return 1;
            }
        }
#endif
        if (retid == 1)
            printf ("The selected kernel entry is wrong! System will try default entry from al.\n ");
        else
            printf ("The boot.cfg not existed!System will try default entry from al.\n");
        delay(1000000);
        free(path);
        path = NULL;
        //          video_cls();
        show_menu=0;
        return 0;
    }
    //show_menu=0;
    //return 1;

}

#define NO_KEY      0
#define ENTER_KEY   1
#define DEL_KEY     2
#define TAB_KEY     3
#define ESC_KEY     4
#define U_KEY   5
#define B_KEY    6
extern int vga_available;
int get_boot_selection(void)
{
    int flag = 1;
    int c;
    unsigned int cnt;
    unsigned int dly;
    struct termio sav;
    dly = 128;

    
    ioctl(STDIN, CBREAK, &sav);
    do {
        delay(10000);
        //if (dly % 128 == 0) printf (".");

        ioctl (STDIN, FIONREAD, &cnt);

//#ifdef LOONGSON2F_7INCH 
        if(cnt == 0) {
            flag = NO_KEY;
            continue;
        }
        c = getchar();

        if (cnt > 0 && c == 0x0a){
            flag = ENTER_KEY;
            break;
        }

        if (cnt >0 && c == 0x75){
            flag = U_KEY;
            break;
        }
        if(cnt >0 && c == 0x62) {
            flag = B_KEY;
            break;
        }
        if (cnt > 0 && c == 0x1b){ 
            ioctl (STDIN, FIONREAD, &cnt);
            if (cnt > 0 && getchar() == 0x5b){
                ioctl (STDIN, FIONREAD, &cnt);
                if (cnt > 0 && getchar() == 0x47){ /*Del pressed*/
                    vga_available = 1;
                    flag = DEL_KEY;
                    break;
                }
            } else {
                flag = ESC_KEY;
            }
        }
        if (cnt > 0 && c == 0x09){ /*Tab key pressed*/
            flag = TAB_KEY;
            break;
        }
/*      
#else
        if (cnt == 0) continue;
        c = getchar();
        if (cnt > 0 && c == 10){
            flag = 1;
            break;
        }
#ifdef ALL_OTHER_KEY_TO_STOP_BOOT
        if (cnt > 0){
            flag = 0;
            break;
        }
#elif defined(ESC_KEY_STOP_BOOT)
        if (cnt >0 && c == 0x1b){
            flag = 0;
            break;
        }
#else
        if (cnt > 0 && c == 0x1b){
            ioctl (STDIN, FIONREAD, &cnt);
            if (cnt > 0 && getchar() == 0x5b){
                ioctl (STDIN, FIONREAD, &cnt);
                if (cnt > 0 && getchar() == 0x47){
                    vga_available = 1;
                    flag = 0;
                    break;
                }
            }
        }
#endif
*/
//#endif //LOONGSON2F_7INCH Recovery key Pressed
    } while (dly-- != 0);

    ioctl (STDIN, TCSETAF, &sav);
    putchar ('\n');
    return flag;

}

#ifdef AUTOLOAD
static void
autoload(char *s)
{
    char buf[LINESZ] = {0};
    char *pa = NULL;
    char *rd;
    //char *Version;

    //char cmdline[256] = "console=tty"; /*Modified by usb rescue or tftp .*/

    if(s != NULL  && strlen(s) != 0) {
        SBD_DISPLAY ("AUTO", CHKPNT_AUTO);
        #if 0 //11-18
        printf("Press <Enter> to execute loading image:%s\n",s);
        printf("Press <del> other key to abort.\n");

        switch (get_boot_selection()){
        case 0:
            /*
             video_cls()
             vga_available = 1;
             */
            break;
#ifdef LOONGSON2F_7INCH 
        case 2: 
            pa = cmdline;
            ui_select(buf, pa);
            /*fall through*/
#endif
        case 1:
        default:
#ifdef CLEAR_WHEN_BOOT
            //video_cls();
            //vga_available = 1;
#endif
        #endif
            vga_available = 0;
            rd= getenv("rd");
            if (rd != 0){
                sprintf(buf, "initrd %s", rd);
                do_cmd(buf);
            }

            if (buf[0] == 0) {
                strcpy(buf,"load ");
                strcat(buf,s);
            }
            do_cmd(buf);

            if (pa == NULL || pa[0] == '\0') 
                pa=getenv("karg");

            strcpy(buf,"g ");
            if(pa != NULL  && strlen(pa) != 0) strcat(buf,pa);
            else strcat(buf," -S root=/dev/hda1 console=tty");
            //else strcat(buf," root=/dev/hda1 console=tty");
            
            //PMON_VER will get in 'g' command.
            /*if((Version = getenv("Version")) == NULL) 
                Version="undefined";
            
            strcat(buf, " PMON_VER=");
            strcat(buf, Version);*/

            delay(10000);
            do_cmd (buf);
            vga_available = 1;
    }
            /*break;
        }
    } else {
        //video_cls();
        //  vga_available = 1;
    }*/
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
#define RESCUE_MEDIA "usb"

static int rescue(void)
{
    //int i;
    struct device *dev, *next_dev;
    char load[256];
    int retid;
    memset(load, 0, 256);

    for (dev  = TAILQ_FIRST(&alldevs); dev != NULL; dev = next_dev) {
        next_dev = TAILQ_NEXT(dev, dv_list);
        if(dev->dv_class < DV_DISK) {
            continue;
        }

        if (strncmp(dev->dv_xname, RESCUE_MEDIA, 3) == 0) {
            sprintf(load, "bl -d ide (%s,0)/boot.cfg", dev->dv_xname);
            retid = do_cmd(load);
            if (retid == 0)
                return 1;
            if (retid == 1)
                printf ("The selected kernel entry is wrong! System will try default entry from al.\n ");
            else
                printf ("The boot.cfg not existed!System will try default entry from al.\n");
            return 0;
        }
    }
    if (dev == NULL) {
        printf("Can't Find rescue disk!!!!!!System will try default entry from al.\n");
        return 0;
    }
    return 0;
}


static int recover(void)
{
    char buf[LINESZ] = {0};
    char *pa = NULL;
    char *rd;
    char *Version;
    char *s;
    char cmdline[256] = "console=tty"; /*Modified by usb rescue or tftp .*/
    int ret;

    //if(s != NULL  && strlen(s) != 0)
#ifdef LOONGSON2F_7INCH
    {
        pa = cmdline;
        ui_select(buf, pa);

        rd= getenv("rd");
        if (rd != 0){
            sprintf(buf, "initrd %s", rd);
            do_cmd(buf);
            buf[0] = 0;
        }

        if (buf[0] == 0) {
            s = getenv ("al");
            if (s == NULL)
                return -1;
            strcpy(buf,"load ");
            strcat(buf,s);
        }

        if ((ret = do_cmd(buf)) != 0) {
                return -1;
        }
        
        if (pa == NULL || pa[0] == '\0') 
            pa=getenv("karg");
        
        strcpy(buf,"g ");
        
        if(pa != NULL  && strlen(pa) != 0) 
            strcat(buf,pa);
        else 
            strcat(buf," -S root=/dev/hda1 console=tty");

        delay(10000);
        do_cmd (buf);
        return -1;
    }
    return 0;
#else
    return 0;
#endif
    
}


/*
 *  PMON2000 entrypoint. Called after initial setup.
 */
void
dbginit (char *adr)
{
    int memsize, freq;
    char    fs[10], *fp;
    char    *s;

/*  splhigh();*/

    memsize = memorysize;

    __init();   /* Do all constructor initialisation */

    SBD_DISPLAY ("ENVI", CHKPNT_ENVI);
    envinit ();

#if NCS5536 > 0 || NVT82C686 > 0
    s = getenv("autopower");
    if (s == 0 || strcmp(s, "1") != 0) {
        suppress_auto_start();
    }
#endif

#if defined(SMP)
    /* Turn on caches unless opted out */
    if (!getenv("nocache"))
        md_cacheon();
#endif

    SBD_DISPLAY ("SBDD", CHKPNT_SBDD);
    tgt_devinit();

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

    tgt_machprint();

    freq = tgt_pipefreq ();
    sprintf(fs, "%d", freq);
    fp = fs + strlen(fs) - 6;
    fp[3] = '\0';
    fp[2] = fp[1];
    fp[1] = fp[0];
    fp[0] = '.';
    printf (" %s MHz", fs);

    freq = tgt_cpufreq ();
    sprintf(fs, "%d", freq);
    fp = fs + strlen(fs) - 6;
    fp[3] = '\0';
    fp[2] = fp[1];
    fp[1] = fp[0];
    fp[0] = '.';
    printf (" / Bus @ %s MHz\n", fs);

    printf ("Memory size %3ld MB (%3d MB Low memory, %3d MB High memory) .\n", (memsize+memorysize_high)>>20,
        (memsize>>20), (memorysize_high>>20));

    tgt_memprint();
#if defined(SMP)
    tgt_smpstartup();
#endif

    printf ("\n");

    md_clreg(NULL);
    md_setpc(NULL, (int32_t) CLIENTPC);
    md_setsp(NULL, tgt_clienttos ());
    DevicesInit();

    /*printf("Press <DEL> key to enter pmon console.\n");
    printf("Press <TAB> key to recover system .\n");
    printf("Press <ENTER> key to boot selection .\n");*/
    {
        unsigned char *envstr;
        if((envstr = getenv("ShowBootMenu"))&&!strcmp("yes", envstr))
        {
            vga_available = 1;
        }
        else
        {
            vga_available = 0;
        }
    }
    switch (get_boot_selection()){
        case TAB_KEY: 
            //#ifdef LOONGSON2F_7INCH 

            //s = getenv ("al");
            //if (s != 0){
            vga_available = 0;
            recover();
            vga_available = 1;
            //#endif
            //}
            break;
            
        case U_KEY:
            vga_available = 1;
            if(!rescue())
            {
                /* second try autoload env */
                s = getenv ("al");
                if (s != 0){
                    autoload (s);
                } else {
                    vga_available = 1;
                    printf("[auto load error]you haven't set the kernel path!\n");
                }           
            }               
            break;

        case NO_KEY:
        case ENTER_KEY:
#ifdef AUTOLOAD
#if defined(LOONGSON2F_FULOONG) || defined(LOONGSON2F_7INCH)||defined(LOONGSON2F_ALLINONE)

            if (!load_menu_list())
            {
                /* second try autoload env */
                s = getenv ("al");
                if (s != 0){
                    autoload (s);
                } else {
                    vga_available = 1;
                    printf("[auto load error]you haven't set the kernel path!\n");
                }           
            }
#else
                s = getenv ("al");
                if (s != 0){
                    autoload (s);
                } else {
                    printf("[auto load error]you haven't set the kernel path!\n");
                }           
#endif
#else
            s = getenv ("autoboot");
            autorun (s);
#endif
            break;

#if defined(LOONGSON2F_7INCH)||defined(LOONGSON2F_FULOONG)||defined(LOONGSON2F_ALLINONE)
    case B_KEY:
            vga_available = 1;
            _set_font_color();
            do_cmd("main");
            break;
#endif
        case DEL_KEY:
            vga_available = 1;
        case ESC_KEY:
#if defined(LOONGSON2F_7INCH)||defined(LOONGSON2F_FULOONG)||defined(LOONGSON2F_ALLINONE)
            _set_font_color();
#endif
            break;
    }
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
static  int             tik_cnt;
static  const char      more_tiks[] = "|/-\\";
static  const char     *more_tik;

    tik_cnt -= rate;
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
    char    **vsp, *ssp;
    int ec, stringlen, vectorlen, stacklen, i;
    register_t nsp;

    /*
     *  Calculate the amount of stack space needed to build args.
     */
    stringlen = 0;
    if (addenv) {
        envsize (&ec, &stringlen);
    }else {
        ec = 0;
    }
    for (i = 0; i < ac; i++) {
        stringlen += strlen(av[i]) + 1;
    }
    stringlen = (stringlen + 3) & ~3;   /* Round to words */
    vectorlen = (ac + ec + 2) * sizeof (char *);
    stacklen = ((vectorlen + stringlen) + 7) & ~7;

    /*
     *  Allocate stack and us md code to set args.
     */
    nsp = md_adjstack(NULL, 0) - stacklen;
    md_setargs(NULL, ac, nsp, nsp + (ac + 1) * sizeof(char *), (int)callvec);

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
    *vsp++ = (char *)0;

    /* build environment vector on stack */
    if (ec) {
        envbuild (vsp, ssp);
    }
    else {
        *vsp++ = (char *)0;
    }
    /*
     * Finally set the link register to catch returning programs.
     */
    md_setlr(NULL, (register_t)_exit);
}

