/*
    PMON Graphic Library
    Designed by Zhouhe(周赫) USTC 04011
*/
#define WA_LEFT 0
#define WA_CENTRE 1
#define WA_RIGHT 31
//#define CHINESE     //中文界面

#define MAIN_TAB_ID 0
#define BOOT_TAB_ID 1
#define NET_TAB_ID 2
#define EXIT_TAB_ID 3

#define DATETIME_WINDOW_ID 5
#define RESTART_WARN_WINDOW_ID 31
#define SHUTDOWN_WARN_WINDOW_ID 32
#define NOTE_WINDOW_ID 100
#define COMMAND_WINDOW_ID 101


#define RUN_COMMAND_ID -3
#define REBOOT_ID -4
#define SHUTDOWN_ID -5

#define UP_KEY_CODE      '[A'       //0x5b41
#define DOWN_KEY_CODE '[B'     //0x5b42
#define LEFT_KEY_CODE    '[D'      //0x5b44
#define RIGHT_KEY_CODE  '[C'     //0x5b43
#define TAB_KEY_CODE      9
#define COMMAND_KEY_CODE  '`'  //0x60

#define COLOR_GRAY          0xe0
#define COLOR_BLUE 0x30
#define COLOR_WHITE 0xffff
#define COLOR_BLACK 0x00

void w_present(void);
void w_init(void);
void w_enterconsole(void);
void w_leaveconsole(void);

void w_setpage(int);
int w_getpage(void);

void w_window(int x,int y,int w,int h,char *text);
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

void w_setcolor(char windowcolor,char buttonunused,char buttonused);
//void w_defaultcolor(void);

int w_keydown(int kin);
int w_hitanykey(void);
