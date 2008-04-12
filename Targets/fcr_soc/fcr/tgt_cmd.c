#include <pmon.h>
union commondata{
		unsigned char data1;
		unsigned short data2;
		unsigned int data4;
		unsigned int data8[2];
		unsigned char c[8];
};
extern unsigned int syscall_addrtype;
extern int (*syscall1)(int type,long long addr,union commondata *mydata);
extern int (*syscall2)(int type,long long addr,union commondata *mydata);
unsigned char i2c_read(unsigned char addr,int v);
int i2c_write(unsigned char data, unsigned char addr,int v);

static int __rtcsyscall1(int type,long long addr,union commondata *mydata)
{
switch(type)
{
case 1:mydata->data1=i2c_read(addr,0);break;
default:break;
}
return 0;
}

static int __rtcsyscall2(int type,long long addr,union commondata *mydata)
{
switch(type)
{
case 1: i2c_write(addr,mydata->data1,0);return 0;
default:break;
}
}

static int rtcs(int ac,char **av)
{
	int bus;
	int dev;
	int func;
    int tmp;
	syscall1=__rtcsyscall1;
	syscall2=__rtcsyscall2;
    printf("select rtc\n");

	return 0;
}
static void fcrtest(int argc,char *argv[])
{
int i;
char buf[100];
printf("test spi");
sprintf(buf,"spi_read_w25x_id");
do_cmd(buf);
sprintf(buf,"spi_read_w25x_id");
do_cmd(buf);
printf("test rtc");
sprintf(buf,"rtcs");
do_cmd(buf);
for(i=0;i<5;i++)
{
delay(1000);
sprintf(buf,"d1 0 10");
do_cmd(buf);
}
}
extern void spi_read_w25x_id();
static const Cmd Cmds[] =
{
	{"MyCmds"},
	{"rtcs","", 0, "test rtc", rtcs, 0, 99, CMD_REPEAT},
	{"fcrtest","", 0, "fcrtest", fcrtest, 0, 99, CMD_REPEAT},
	{"spi_read_w25x_id","",0,"spi_read_w25x_id",spi_read_w25x_id,0,99,CMD_REPEAT},
	{0, 0}
};

static void init_cmd __P((void)) __attribute__ ((constructor));

static void
init_cmd()
{
	cmdlist_expand(Cmds, 1);
}

