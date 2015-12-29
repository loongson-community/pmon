
#define W_PAGE_SYS 0
#define W_PAGE_BOOT 1
#define W_PAGE_NET 222
#define W_PAGE_PASS 2
#define W_PAGE_QUIT 3
#define W_PAGE_TIME 105
#define W_PAGE_SAVEQUIT 31
#define W_PAGE_SKIPQUIT 32
#define W_PAGE_LOADDEF 33
#define TM_YEAR_BASE 1900

#define NEXT_PAGE(x) (x+1>=npages?0:x+1)
#define PREV_PAGE(x) (x==0?npages-1:x-1)

#if	defined(LOONGSON_2G5536)||defined(LOONGSON_2G1A) || defined(LOONGSON_2F1A)
#include "../../../x86emu/src/biosemu/biosemui.h"
RMREGS	*vga_reg;
RMSREGS	*vga_sreg;
#define LOONGSON_2G5536_VGA_REG		vga_reg
#define LOONGSON_2G5536_VGA_SREG	vga_sreg
#endif

struct _daytime
{
	char *name;
	short x,y;
	char buf[8];
	short buflen,base;
};

static struct _sysconfig{
	char kpath[256];
	char kpath1[256];		/* path for boot device 2 */
	char kargs[256];
	char kargs1[256];
	int detectnet;
	int boottype;
	int usbkey;
	char bootdev[40];		/* 1st boot device */
	char bootdev1[40];		/* 2st boot device */
};

typedef struct 
{
	char text;
	unsigned char color;	
}scr;

struct win_para {
	int w_flag;
	char w_para[128];
	char w_kpara[128];
	char vendor[40+1];
	int capacity;  //unit GB
	char dev_name[5]; /* Lc add */
	int w_used; /* Lc add */
};
/* discripte find device
 * win_mask : 1_bit :  usb0, 2_bit : wd0, 3_bit : sata0, 4_bit : sata1
 */
typedef struct win_device *win_dp;
struct win_device {
	char win_mask;
	struct win_para sata0;
	struct win_para sata1;
	struct win_para sata2; /* Lc add */
    struct win_para sata3; /* Lc add */
	struct win_para ide;
	struct win_para ide0; /* Lc add */
    struct win_para ide1; /* Lc add */
	struct win_para usb;
};

int strcmp(const char *dest, const char *source);
unsigned int scancode_queue_read(void);
int	cmd_main __P((int, char *[]));
int w_setup_pass(char *type);
static void run(char *cmd);
static inline int next_page(void);
static inline int prev_page(void);
//void init_device_name(char *netdev_name[],char *diskdev_name[]);
void init_device_name(void);
void init_sysconfig(char *diskdev_name[]);
void deal_keyboard_input(int *esc_tag, int *to_command_tag, int *esc_down);
void paint_mainframe(char *hint);
int paint_childwindow(char **hint,char *diskdev_name[],char *netdev_name[],int esc_down);
int check_password_textwindow(char *type);
int check_password(int page);
int ifconfig (char *ifname, char *ipaddr);
void tty_flush(void);
int snprintf (char *buf, size_t maxlen, const char *fmt, ...);
int pwd_set(char *user,char* npwd);
int pwd_clear(char *user);
int pwd_exist(void);
int pwd_is_set(char *user);
long get_screen_address(int xx,int yy);
int pwd_cmp(char *user,char* npwd);
int w_selectinput1(int x,int y,int w,char *caption,char **options,
char *current,int buflen,int * dispon);
extern void video_cls1(void);
int init_win_deivice(void);
