/*
    PMON Graphic Library
    Designed by Zhouhe(周赫) USTC 04011
*/

/*****************************************************************************************************************

 Copyright (C)
 File name:     window.c
 Author:  ***      Version:  ***      Date: ***
 Description:   
 Others:        
 Function List:
 
 Revision History:
 
 ----------------------------------------------------------------------------------------------------------------------------
  Date                Author           Activity ID                Activity Headline
  2008-05-17    Zhouhe          PMON20080517    Create it.
  2009-07-17    QianYuli         PMON20090202    Modified for porting to Fuloong & 8089 & allinone platform.
*************************************************************************************************************/
#include <termio.h>
#include <pmon.h>
#include <stdio.h>
#include <linux/types.h>
#include "window.h"
#include <sys/ioctl.h>
#include <stdio.h>
#include "mod_framebuffer.h"
#include <linux/io.h>
#ifdef VESAFB
#include <x86emu/int10/vesa.h>
#endif

#define VIDEO_FONT_WIDTH    8
#define VIDEO_FONT_HEIGHT   16
#define VIDEO_FONT_CN_WIDTH 16
#define W_MAX_SCREEN_WIDTH  171
#define W_MAX_SCREEN_HEIGHT  48

int screen_width = 128;//count of fonts in a row of the specific platform
int screen_height = 37;//count of fonts in a column the specific platform
int y_start = 0;
typedef struct {
    char text;
    unsigned char color;    
}scr;

scr finalbuf[W_MAX_SCREEN_HEIGHT][W_MAX_SCREEN_WIDTH];
scr foreground[W_MAX_SCREEN_HEIGHT][W_MAX_SCREEN_WIDTH];
int newpage=0,oldpage=0;
int currentid=0,enterid=0,overid=1,winit=1;
unsigned char bcolor=0xe0;
unsigned char btncolor[3];
int charin=0,cn=0;
unsigned int pal[32] = {
    0x0,                       //black                      0x0           theme1
    0x100010,           // blue                        0x1
    0x0,                      //black                       0x2
    0x0,                      //black                       0x3
    0x0,                      //black                       0x4
    0x0,                     //black                        0x5
    0xc61fc61f,        //gray                         0x6
    0x6140614,       //dark green              0x7
    0xffffffff,             //white                        0x8           theme2
    0x6140614,        //dark green             0x9
    0x0,                      //black                       0xa
    0x0,                      //black                       0xb
    0x0,                      //black                       0xc
    0x0,                     //black                        0xd
    0x0,                     //black                        0xe
    0x100010,           //blue                         0xf
    0xffffffff,               //white                       0x10          theme3
    0xc61fc61f,         //gray                         0x11
    0x0,                      //black                       0x12
    0x0,                      //black                       0x13
    0x0,                      //black                       0x14
    0x0,                     //black                        0x15
    0x100010,          //blue                          0x16
    0x6140614,        //dark green              0x17
    0xc61fc61f,         //gray                         0x18         theme4
    0x6140614,         //dark green             0x19
    0x0,                      //black                       0x1a
    0x0,                      //black                       0x1b
    0x0,                      //black                       0x1c
    0x0,                     //black                        0x1d
    0xffffffff,              //white                        0x1e
    0x100010,          //blue                          0x1f
};

unsigned int *p_pal = pal;
#if NMOD_FRAMEBUFFER
extern unsigned int eorx, fgx, bgx;
extern void video_disableoutput(void);
extern void video_enableoutput(void);
#else
extern unsigned char* vgabh;
#endif
extern int com_counts;
extern int vga_available;

extern void video_drawchars (int xx, int yy, unsigned char *s, int count);
extern void vedio_drawCNchar(int xx, int yy, unsigned char *s);

void w_setpage(int i)
{
    newpage=i;
}

int w_getpage(void)
{
    return oldpage;
}

int w_focused(void)
{
    return currentid==overid;
}

int w_entered(void)
{
    return currentid==enterid;
}

void w_setfocusid(int n)
{
    overid=n;
}

int w_getfocusid(void)
{
    return overid;
}

static int inside(int x,int y)
{
    return (x<screen_width && x>=0 && y<screen_height && y>=0);
}

void w_enterconsole()
{
#if  NMOD_FRAMEBUFFER
    if(vga_available) {
        video_enableoutput();
        eorx=fgx=0xffffffff;
        bgx=0;
    }
#endif
    video_cls();
}
void w_leaveconsole()
{
    video_cls();
#if  NMOD_FRAMEBUFFER
    if(vga_available){
        video_disableoutput();
    }
#endif
}

void w_init(void)
{
    struct termio tbuf;

    video_cls();
    screen_width = get_scr_width();
    screen_height = get_scr_height();
    w_setcolor(0x10,0x10,0x60);
    ioctl (STDIN, SETNCNE, &tbuf);  //fixme
}
static void w_copy(void *src,void *dest)//瀵归?锛?!!
{
    int i;
    //for(i=0;i<80*25*2;i+=8)
    for(i=0;i<screen_width*screen_height*2;i+=8){
        *((long long *)(dest+i))=*((long long *)(src+i));
    }
}
static void w_background(void)
{
    int i,j,y,y1;
        
    //line 0 line screen_height-2 and line screen_height-3 dark green background(case theme1)
    for(j=0; j<screen_width; j++){  
        foreground[0][j].color = 0x70;  
        foreground[screen_height-2][j].color = 0x70;
        foreground[screen_height-1][j].color = 0x70;     
    }

    //line 1 blue background(case theme1)
    for(i=0; i < screen_width;i++) {
        foreground[1][i].color = 0x10;
    }

    //line 2 gray background(case theme1)
    for(i=0; i<screen_width; i++){
        foreground[2][i].color = 0x60;
    }
  
    for (i = 0; i < screen_width; i++) {
        foreground[3][i].color = 0;
    }

    //line 4 to line screen_height-4 blue background(case theme1)
    for (j = 4; j < screen_height-3; j++)
        for (i = 0; i < screen_width; i++) {
            foreground[j][i].text = 0;
            foreground[j][i].color = 0x10;
        }

        //line screen_width-3 gray background(case theme1)
        for(i=0;i<screen_width;i++) {
            foreground[screen_height-3][i].color = 0x60;
        }
        //row 0 and row screen_width-1 and row screen_width*5/8 gray background(case theme1)
        for (j=3;j<screen_height-2;j++) {
            foreground[j][0].color = 0x60;
            foreground[j][screen_width-1].color = 0x60;
            foreground[j][screen_width * 5/8+1].color = 0x60;
        }

    for(i=0;i<=screen_width-1;i++){
        foreground[2][i].text=foreground[screen_height-3][i].text=' ';
    }

#ifdef  CHINESE
        w_text(screen_width/2,0,WA_CENTRE,"龙芯BIOS设置");
        w_bigtext(0,screen_height-2,screen_width,2,"方向键←&→: 切换页面    <TAB>:切换选项    <回车>:确认和切换   <~>:切换主题");
#else
        w_text(screen_width/2,0,WA_CENTRE,"LOONGSON BIOS SETUP");   
        w_bigtext(0,screen_height-2,screen_width,2,"<-&->: Switch Page     TAB: Switch Item     Enter: Confirm and Switch   `:Change Theme");
#endif  
}

int isdifferent(scr* x_scr, scr* y_scr,int j)
{
    int i;

    for(i=0;i<screen_width;i++){
        if(*(short *)(&(finalbuf[j][i]))!=*(short *)(&(foreground[j][i])))
            return 1;
    }
    return 0;
}

void w_updatescreen()
{
    int i,j,x,y;
    char s[2];

    video_enableoutput();
    for(j=0,y=VIDEO_FONT_HEIGHT*y_start;j<screen_height;j++,y+=VIDEO_FONT_HEIGHT){
        if(isdifferent(finalbuf, foreground, j)){
            for(i=0,x=0;i<screen_width;i++){  
                fgx=p_pal[foreground[j][i].color&0xf];
                bgx=p_pal[foreground[j][i].color>>4];
                eorx=fgx^bgx;
                if( (foreground[j][i].text & 128) !=0 && (foreground[j][i+1].text & 128) !=0 ) {   
                    s[0]=foreground[j][i].text;
                    s[1]=foreground[j][i+1].text;
                    vedio_drawCNchar(x,y,s);
                    i++;
                    x+=VIDEO_FONT_CN_WIDTH;
                } else {                       
                    video_drawchars (x, y,&(foreground[j][i].text),1);
                    x+=VIDEO_FONT_WIDTH;
                }
            }                
        }
    }
    video_disableoutput();
    for (j = 0; j < screen_height; j++)
        for (i = 0; i < screen_width; i++) {
            finalbuf[j][i].text = foreground[j][i].text;
            finalbuf[j][i].color = foreground[j][i].color;
        }
}

void changetheme(void)
{
    int ij,ik;
    //every theme has 8 colors 
    p_pal+=8;
    if(p_pal == &pal[32]) {
        p_pal=pal;
    }
    for(ij=0;ij<screen_height;ij++)
        for(ik=0;ik<screen_width;ik++) {
            finalbuf[ij][ik].text = ~foreground[ij][ik].text;
            finalbuf[ij][ik].color = ~foreground[ij][ik].color;
        }
}

void w_present(void)
{
    char c,c0,c1;    
    int cnt;
    int ij,ik;

    if(vga_available) {
#if NMOD_FRAMEBUFFER    
        w_updatescreen();
#else
        for (ij = 0; ij < screen_height; ij++)
            for (ik = 0; ik < screen_width; ik++) {
                vgabh[2*(ij*screen_width+ik)] = foreground[ij][ik].text;
                vgabh[2*(ij*screen_width+ik)+1] = foreground[ij][ik].color;
            }
#endif
    }    
 
    winit=0;
    w_background();
    if(newpage!=oldpage){
        overid=1;
        oldpage=newpage;
        winit=1;
    }
    charin=0;
    enterid=-100;

    switch(cn)
    {
        case '\n':
                enterid=overid;
                break;
        case UP_KEY_CODE://UP key
        case LEFT_KEY_CODE://LEFT key
        case RIGHT_KEY_CODE://RIGHT key
        case DOWN_KEY_CODE://DOWN key
                break;
        case TAB_KEY_CODE://TAB key
                overid = overid%com_counts;
                overid++;
                break;
        case '`':
                changetheme();
                break;
        default:
            charin=cn;
    }

    ioctl (STDIN, FIONREAD, &cnt);
    cn=0;
    if(cnt) {
        do {
            cn=(cn<<8)+(c=getchar());
        }while(c=='[');

#if NMOD_FRAMEBUFFER
        set_cursor_fb(0,0);
#else
        set_cursor(0,0);
#endif
    }
    currentid=0;
}
static void w_text2(int x,int y,int xalign,char *ostr,int len)
{
    int i,t;

    if(y<0 || y> screen_height-1)
        return;

    t = len;
    x -=  t >> xalign ;
    for(i=0;i<t;i++)
        if(ostr[i]!=' ' && ostr[i]!='\n' && inside(x+i,y)) {
            foreground[y][x+i].text=ostr[i];
            foreground[y][x+i].color&=0xf0;
            foreground[y][x+i].color|=((~(foreground[y][x+i].color>>4))&0x7);
        }
}
void w_text(int x,int y,int xalign,char *ostr)
{
    w_text2(x,y,xalign,ostr,strlen(ostr));
}
void w_window(int x,int y,int w,int h,char *text)
{
    int x1,y1,i,lpitch;
    scr *tscr;  

    x1=x+w;
    y1=y+h;

    if(x1<1 || x>screen_width-1 || y1<1 || y>screen_height-1)
        return;
    
    if(x<0)
        x=0;
    if(y<0)
        y=0;
    if(x1>screen_width)
        x1=screen_width;
    if(y1>screen_height)
        y1=screen_height;
    
    tscr=&(foreground[y][x]);
    lpitch = W_MAX_SCREEN_WIDTH - (x1 - x);
    for(i=x;i<x1;i++,tscr++) {
        tscr->text=' ';
        tscr->color=bcolor;
    }
    w_text((x+x1)>>1,y,WA_CENTRE,text);
    for(y++;y<y1;y++) {
        tscr+=lpitch;
        for(i=x;i<x1;i++,tscr++)
            tscr->color = btncolor[1];
    }
}

static void w_window2(int x,int y,int w,int h,char *text)
{
    currentid++;
    if(currentid==overid) {
        bcolor=btncolor[2];
    }
    else  {       
        bcolor=btncolor[1];
    }
    w_window(x,y,w,h,text);
    bcolor=btncolor[0];
}
int w_button(int x,int y,int w,char *caption)
{
    w_window2(x,y,w,1,caption);
    return currentid==enterid;
}
int w_select(int x,int y,int w,char *caption,char **options,int *current)
{
    w_text(x,y,WA_LEFT,caption);
    w_window2(x,y,w,1,options[*current]);
    if(currentid==enterid) {
        (*current)++;
        if(options[*current] == 0)
            *current=0;
    }
    return currentid==enterid;
}
int w_switch(int x,int y,char *caption,int *base,int mask)
{
    w_window2(x,y,1,1,(((*base)&mask)?"X":" "));
    if(currentid == enterid)    
        (*base)^=mask;
    w_text(x+1,y,WA_RIGHT,caption);
    return currentid==enterid;
}
int w_input(int x,int y,int w,char *caption,char * text,int buflen)
{
    int l=strlen(text); 

    w_text(x,y,WA_LEFT,caption);
    w_window2(x,y,w,1,((w>l)?text:text+l-w));
    if(currentid==overid)  {
        if(charin==127 || charin==7 || charin =='\b')  {//backspace key
            if(l){
                text[l-1]=0;
            }
        } else if(charin>=32 && l<buflen-1) {
            text[l]=charin;
            text[l+1]=0;
        }
    }
    return currentid==enterid;
}
void w_bigtext(int x,int y,int w,int h,char *text)
{
    int i,j,k;
    h+=y;
    #ifdef CHINESE//Chinese font's width is two times of english font's width
    if ((w%2)!=0) {
        x++;
        w--;
    }
    #endif
    for(k=y;k<h;k++) {
        for(i=0;i<w+1;i++)
            switch(text[i])
            {
                case 0:
                    j=i;
                    w_text(x,k,WA_RIGHT,text);
                    return;
                case '\n':
                    j=i+1;
                    goto end;
                case ' ':
                    j=i+1;
                    break;
                default:
                    j = w;
            }
end:
        w_text2(x,k,WA_RIGHT,text,j);
        text+=j;
    }
}
void w_setcolor(char windowcolor,char buttonunused,char buttonused)
{
    bcolor=btncolor[0]=windowcolor;
    btncolor[1]=buttonunused;
    btncolor[2]=buttonused;
}

int w_keydown(int kin)
{   
    if(kin==cn) {
        cn=0;
        return 1;
    }
    return 0;
}
int w_selectinput(int x,int y,int w,char *caption,char **options,char *current,int buflen)
{
    if(winit) {
        for(x=0;options[x];x++)
            if((unsigned char)(current[buflen-1])>=x)
                current[buflen-1]=0;
        for(x=0;x<buflen-1 && current[x];x++);
        if(x==0 || x==buflen-1)
            strcpy(current,options[current[buflen-1]]);
        return 0;
    }
    if(w_input(x,y,w,caption,current,buflen-1)) {
        current[buflen-1]++;
        if(options[current[buflen-1]] == 0)
            current[buflen-1]=0;
        strcpy(current,options[current[buflen-1]]);
    }
    return currentid==enterid;
}
int w_biginput(int x,int y,int w,int h,char *caption,char * text,int buflen)
{
    int l=strlen(text); 
    w_window2(x,y,w,h,caption);
    if(currentid==overid)  {
        text[l]=219;
        text[l+1]=0;
        w_bigtext(x,y+1,w,h-1,text);
        text[l]=0;

        if(charin==127)  {
            if(l){
                text[l-1]=0;
            }
        } else if(charin>=32 && l<buflen-2)  {
            text[l]=charin;
            text[l+1]=0;
        }
    } else{ 
        w_bigtext(x,y+1,w,h-1,text);
    }
    return currentid==enterid;
}
int w_hitanykey()
{
    int cnt;      
    while(cnt==0){
        ioctl (STDIN, FIONREAD, &cnt);
    }
    return getchar();
}

