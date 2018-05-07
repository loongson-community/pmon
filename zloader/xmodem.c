//#include <asm/context.h>
//#include <asm/cacheops.h>
//#include <asm/mipsregs.h>
//#include <asm/r4kcache.h>
#define tgt_puts tgt_puts1
#undef XMODEM_DEBUG
static inline __attribute__((always_inline))  int now()
{
int count;
asm volatile("mfc0 %0,$9":"=r"(count)); 
return count;
}

static inline __attribute__((always_inline))  void *memcpy(char *s1, const char *s2, int n)
{
while(n--)
  *s1++=*s2++;
}

static __attribute__ ((section (".data")))  void * memset(void * s,int c, int count)
{
	char *xs = (char *) s;

	while (count--)
		*xs++ = c;

	return s;
}

static __attribute__ ((section (".data")))  unsigned short _crc_xmodem_update (unsigned short crc, unsigned char data)
{
    int i;
    crc = crc ^ ((unsigned short)data << 8);
    for (i=0; i<8; i++)
    {
        if (crc & 0x8000)
            crc = (crc << 1) ^ 0x1021;
        else
            crc <<= 1;
    }
    return crc;
}

static __attribute__ ((section (".data"))) int tgt_puts(char *str)
{
	while(*str)
	tgt_putchar1(*str++);
	return 0;
}



//常数定义
#define BLOCKSIZE       128            //M16的一个Flash页为128字节(64字)
#define DATA_BUFFER_SIZE    BLOCKSIZE   //定义接收缓冲区长度
//#define F_CPU            7372800         //系统时钟7.3728MHz

//定义Xmoden控制字符
#define XMODEM_NUL          0x00
#define XMODEM_SOH          0x01
#define XMODEM_STX          0x02
#define XMODEM_EOT          0x04
#define XMODEM_ACK          0x06
#define XMODEM_NAK          0x15
#define XMODEM_CAN          0x18
#define XMODEM_EOF          0x1A
#define XMODEM_WAIT_CHAR    'C'

//定义全局变量
struct str_XMODEM
{
    unsigned char SOH;                  //起始字节
    unsigned char BlockNo;               //数据块编号
    unsigned char nBlockNo;               //数据块编号反码
    unsigned char Xdata[BLOCKSIZE];            //数据128字节
    unsigned char CRC16hi;               //CRC16校验数据高位
    unsigned char CRC16lo;               //CRC16校验数据低位
} strXMODEM ;                           //XMODEM的接收数据结构

/*   GCC里面地址使用32位长度，适应所有AVR的容量*/


#define ST_WAIT_START       0x00         //等待启动
#define ST_BLOCK_OK       0x01         //接收一个数据块成功
#define ST_BLOCK_FAIL       0x02         //接收一个数据块失败
#define ST_OK             0x03         //完成


#ifdef XMODEM_DEBUG
#define dbg_printf tgt_puts
#else
#define dbg_printf(...)
#endif

static __attribute__ ((section (".data")))  int testchar()
{
	int count=2;
	int total, start;
	start = now();

	while(1)
	{
    if(tgt_testchar1())
	return 100;
	if(!count)break;
	if((now()-start>0x3000000 )){
	start = now();
	count--;
	}
	}

	   return 0;
}

static __attribute__ ((section (".data")))  int get_data(unsigned char *ptr,unsigned int len,unsigned int timeout)
{
	int i=0;
	volatile int count=1;
	while(i<len)
	{
		if(testchar()>0)
		ptr[i++]=tgt_getchar();
		else {
		if(!count)break;
		count--;
		}
	}
    return i;
}



//计算CRC16
static __attribute__ ((section (".data")))  unsigned int calcrc(unsigned char *ptr, unsigned int count)
{
    unsigned int crc = 0;
    while (count--)
    {
        crc =_crc_xmodem_update(crc,*ptr++);
    }
    return crc;
}

static int __attribute__ ((section (".data"))) program1(char *p,int off, int size)
{
	static int first = 1;

	if(first) 
	{
		erase();
		first = 0;
	}

		 program(p,off,size);
}

int __attribute__ ((section (".data"))) xmodem_transfer(char *base)
{
    unsigned char c;
    unsigned int i;
    unsigned int crc;
    unsigned int filesize=0;
    unsigned char BlockCount=1;               //数据块累计(仅8位，无须考虑溢出)
    unsigned char STATUS;                  //运行状态

	spi_init();

    //向PC机发送开始提示信息
        STATUS=ST_WAIT_START;               //并且数据='d'或'D',进入XMODEM
    c=0;
	while(1)
	{
		tgt_putchar1(XMODEM_WAIT_CHAR);
		if(testchar()>0)break;

	}
    while(STATUS!=ST_OK)                  //循环接收，直到全部发完
    {
	
        i=get_data(&strXMODEM.SOH,BLOCKSIZE+5,1000);   //限时1秒，接收133字节数据
        if(i)
        {
            //分析数据包的第一个数据 SOH/EOT/CAN
            switch(strXMODEM.SOH)
            {
            case XMODEM_SOH:               //收到开始符SOH
                if (i>=(BLOCKSIZE+5))
                {
                    STATUS=ST_BLOCK_OK;
                }
                else
                {
                    STATUS=ST_BLOCK_FAIL;      //如果数据不足，要求重发当前数据块
                    tgt_putchar1(XMODEM_NAK);
                }
                break;
            case XMODEM_EOT:               //收到结束符EOT
                //tgt_putchar1(XMODEM_ACK);            //通知PC机全部收到
                //STATUS=ST_OK;
                tgt_putchar1(XMODEM_NAK);            //要求重发当前数据块
                STATUS=ST_BLOCK_FAIL;
            dbg_printf("transfer succeed!\r\n");
                break;
            case XMODEM_CAN:               //收到取消符CAN
                //tgt_putchar1(XMODEM_ACK);            //回应PC机
                //STATUS=ST_OK;
                tgt_putchar1(XMODEM_NAK);            //要求重发当前数据块
                STATUS=ST_BLOCK_FAIL;
            dbg_printf("Warning:user cancelled!\r\n");
                break;
            default:                     //起始字节错误
                tgt_putchar1(XMODEM_NAK);            //要求重发当前数据块
                STATUS=ST_BLOCK_FAIL;
                break;
            }
        }
		else 
		{
		dbg_printf("time out!\n");
		break;
		}
        if (STATUS==ST_BLOCK_OK)            //接收133字节OK，且起始字节正确
        {
            if (BlockCount != strXMODEM.BlockNo)//核对数据块编号正确
            {
                tgt_putchar1(XMODEM_NAK);            //数据块编号错误，要求重发当前数据块
                continue;
            }
            if (BlockCount !=(unsigned char)(~strXMODEM.nBlockNo))
            {
                tgt_putchar1(XMODEM_NAK);            //数据块编号反码错误，要求重发当前数据块
                continue;
            }
            crc=strXMODEM.CRC16hi<<8;
            crc+=strXMODEM.CRC16lo;
            //AVR的16位整数是低位在先，XMODEM的CRC16是高位在先
            if(calcrc(&strXMODEM.Xdata[0],BLOCKSIZE)!=crc)
            {
                tgt_putchar1(XMODEM_NAK);              //CRC错误，要求重发当前数据块
				dbg_printf("crc error\n");
                continue;
            }


            //正确接收128个字节数据，刚好是M16的一页
            memcpy(base+(filesize&0xfff),strXMODEM.Xdata,128);
            filesize+=128;
            tgt_putchar1(XMODEM_ACK);                 //回应已正确收到一个数据块
            BlockCount++;                       //数据块累计加1
	    if((filesize&0xfff)==0)
		 program1(base,filesize-0x1000,0x1000);
        }
    }
	    if((filesize&0xfff))
		 program1(base,(filesize&~0xfff),0x1000);

    //退出Bootloader程序，从0x0000处执行应用程序
    tgt_puts("xmodem finished\r\n");
    return filesize;
}


static char blockdata[0x1000];

int xmodem()
{
int size;
int offset;
char mode;
//clean char buf
tgt_puts("xmodem now\r\n");
while(tgt_getchar()!='x') tgt_puts("press x to tranfser\r\n");

//tgt_puts("haha\r\n");

xmodem_transfer(blockdata);

while(1);

return 0;
}


