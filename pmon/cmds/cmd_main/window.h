/*
    Loongson pmon graghic library 
 */

/* Text display relative position */
#define WA_LEFT 31
#define WA_CENTRE 1
#define WA_RIGHT 0

/* Font : 8*16*/
#define VIDEO_FONT_WIDTH	8
#define VIDEO_FONT_HEIGHT	16

#if defined(CENTERM)
/* Vga:1024x768 */
#define VIDEO_WIDTH		128
#define VIDEO_HEIGHT		48

#define REV_ROW_LINE		736
#define INF_ROW_LINE		752 
#elif defined(LOONGSON_2G5536)
/* Vga:800*600 */
#define VIDEO_WIDTH             80
#define VIDEO_HEIGHT		25

#define REV_ROW_LINE		560
#define INF_ROW_LINE		576
#else
/* Vga:800*600 */
#define VIDEO_WIDTH		100
#define VIDEO_HEIGHT		37

#define REV_ROW_LINE		560
#define INF_ROW_LINE		576
#endif

/* Vertical line compart position */
#define V_COMPART_LINE  (((VIDEO_WIDTH*3)/4) - 5)//70.91

/* Hint window macro */
#define HINT_WIN_START  (V_COMPART_LINE + 1)//71,92
#define HINT_WIN_WIDTH  (VIDEO_WIDTH - HINT_WIN_START - 1) //28,35

/* Base window macro */
#define BASE_WIN_START  1
#define BASE_WIN_WIDTH  (V_COMPART_LINE - 1)//69

/* Text height macro */
#define BASE_WIN_HEIGHT    (VIDEO_HEIGHT - 6)//31 <text background>
#define HINT_WIN_HEIGHT    (VIDEO_HEIGHT - 6)//31

void w_present(void);
void win_init(void);
void w_enterconsole(void);
void w_leaveconsole(void);

void w_setpage(int);
int w_getpage(void);

void w_window(int x,int y,int w,int h,char *text);
int w_window3(int x,int y,int w,int h,char **text,int n);
void w_window2(int x,int y,int w,int h,char *text);
int w_button(int x,int y,int w,char *caption);
void w_text(int x,int y,int xalign,char *ostr);
int w_select(int x,int y,int w,char *caption,char **options,int *current);
int w_switch(int x,int y,char *caption,int *base,int mask);
int w_input(int x,int y,int w,char *caption,char * text,int buflen);
void w_bigtext(int x,int y,int w,int h,char *text);
int w_selectinput(int x,int y,int w,char *caption,char **options,char *current,int buflen);
int w_biginput(int x,int y,int w,int h,char *caption,char * text,int buflen);

int w_focused(void);
int w_entered(void);
void w_setfocusid(int);
int w_getfocusid(void);

void w_setcolor(int windowcolor,int winbodycolor,int wintrans,int buttonunused,int buttonused);
void w_defaultcolor(void);

int w_keydown(int kin);
int w_hitanykey(void);
int w_setpage_safe(int i);
int w_password(int x,int y,int w,char *caption,char * text,int buflen);
void w_setup(int cls,int setcursor);
void w_present1(int x0,int y0,int w0,int h0);
void w_setwindowcolor(int x,int y,int w,int h,int color,int trans);
