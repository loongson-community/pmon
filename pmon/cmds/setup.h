#define TYPE_NONE 0
#define TYPE_CMD  1
#define TYPE_MENU 2
#define TYPE_MSG  3
#define TYPE_EDITMENU 4
#define TYPE_CHOICE 5

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

