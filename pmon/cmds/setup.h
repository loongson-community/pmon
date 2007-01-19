#include "mod_framebuffer.h"
#define TYPE_NONE 0
#define TYPE_CMD  1
#define TYPE_MENU 2
#define TYPE_MSG  3
#define TYPE_EDITMENU 4
#define TYPE_CHOICE 5

#if NMOD_FRAMEBUFFER ==0
#define POP_W	30
#define POP_H	15
#define POP_X	16
#define POP_Y	8
#define MSG_W	70
#define MSG_H	15
#define MSG_X	5
#define MSG_Y	8
#define INFO_Y  24
#define INFO_W  80
#else
#define POP_W	30
#define POP_H	15
#define POP_X	16
#define POP_Y	5
#define MSG_W	70
#define MSG_H	15
#define MSG_X	5
#define MSG_Y	5
#define INFO_Y  21
#define INFO_W  80
#endif

struct setupMenuitem{
char y;
char x;
char dnext;
char rnext;
char type;
char *msg;
int (*action)(int msg);
void *arg;
};

struct setupMenu{
int (*action)(int msg);
int width,height;
struct setupMenuitem *items;
};

void __console_alloc();
void __console_free();
void do_menu(struct setupMenu *newmenu);
