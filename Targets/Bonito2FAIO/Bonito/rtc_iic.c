#include <pmon.h>
#include <stdio.h>
#include <string.h>

#ifdef DEVBD2F_SM502
#define	SM502_USE_LOWC
extern volatile char *mmio;
#define SM502_I2C_COUNT 0x010040
#define SM502_I2C_CTRL 0x010041
#define SM502_I2C_STAT 0x010042
#define SM502_I2C_ADDR 0x010043      //0:write,1:read
#define SM502_I2C_DATA 0x010044      //44-53

#define GPIO_DIR_LOW 		(volatile unsigned int *)(mmio + 0x10008)
#define GPIO_DATA_LOW		(volatile unsigned int *)(mmio + 0x10000)

#define GPIO_DIR_HIGH 		(volatile unsigned int *)(mmio + 0x1000c)
#define GPIO_DATA_HIGH		(volatile unsigned int *)(mmio + 0x10004)
#define G_OUTPUT		1
#define G_INPUT			0
#define	DAT_PIN			21//13	
#define	CLK_PIN			20//6

typedef unsigned int pcitag_t;

extern volatile char *mmio;
extern pcitag_t _pci_make_tag(int, int, int);
extern pcitag_t _pci_conf_readn(pcitag_t, int, int);
extern void _pci_conf_writen(pcitag_t, int, pcitag_t, int);

void rtc_i2c_sleep(int ntime)
{
	int i,j=0;

	for(i=0; i<320*ntime; i++)		// 100KHz on 900MHz
	{
		j=i;
		j+=i;
	}
}

#ifdef	SM502_USE_LOWC
static void rtc_sda_dir(int ivalue)
{
	int tmp;
	tmp = *GPIO_DIR_LOW;
	if(ivalue == 1)
		*GPIO_DIR_LOW = tmp|(0x1<<DAT_PIN);
	else
		*GPIO_DIR_LOW = tmp&(~(0x1<<DAT_PIN));
}
static void rtc_scl_dir(int ivalue)
{
	int tmp;
	tmp = *GPIO_DIR_LOW;
	if(ivalue == 1)
		*GPIO_DIR_LOW = tmp|(0x1<<CLK_PIN);
	else
		*GPIO_DIR_LOW = tmp&(~(0x1<<CLK_PIN));
}

static void rtc_sda_bit(int ivalue)
{
	int tmp;
	tmp = *GPIO_DATA_LOW;
	if(ivalue == 1)
		*GPIO_DATA_LOW = tmp|(0x1<<DAT_PIN);
	else
		*GPIO_DATA_LOW = tmp&(~(0x1<<DAT_PIN));
}
static void rtc_scl_bit(int ivalue)
{
	int tmp;
	tmp = *GPIO_DATA_LOW;
	if(ivalue == 1)
		*GPIO_DATA_LOW = tmp|(0x1<<CLK_PIN);
	else
		*GPIO_DATA_LOW = tmp&(~(0x1<<CLK_PIN));
}

#else
static void rtc_sda_dir(int ivalue)
{
	int tmp;
	tmp = *GPIO_DIR_HIGH;
	if(ivalue == 1)
		*GPIO_DIR_HIGH = tmp|(0x1<<15);
	else
		*GPIO_DIR_HIGH = tmp&(~(0x1<<15));
}

static void rtc_scl_dir(int ivalue)
{
	int tmp;
	tmp = *GPIO_DIR_HIGH;
	if(ivalue == 1)
		*GPIO_DIR_HIGH = tmp|(0x1<<14);
	else
		*GPIO_DIR_HIGH = tmp&(~(0x1<<14));
}

static void rtc_sda_bit(int ivalue)
{
	int tmp;
	tmp = *GPIO_DATA_HIGH;
	if(ivalue == 1)
		*GPIO_DATA_HIGH = tmp|(0x1<<15);
	else
		*GPIO_DATA_HIGH = tmp&(~(0x1<<15));
}
static void rtc_scl_bit(int ivalue)
{
	int tmp;
	tmp = *GPIO_DATA_HIGH;
	if(ivalue == 1)
		*GPIO_DATA_HIGH = tmp|(0x1<<14);
	else
		*GPIO_DATA_HIGH = tmp&(~(0x1<<14));
}
#endif

void rtc_i2c_start(void)
{
	rtc_sda_dir(G_OUTPUT);
	rtc_scl_dir(G_OUTPUT);
	rtc_scl_bit(0);
	rtc_i2c_sleep(1);
	rtc_sda_bit(1);
	rtc_i2c_sleep(1);
	rtc_scl_bit(1);
	rtc_i2c_sleep(5);
	rtc_sda_bit(0);
	rtc_i2c_sleep(5);
	rtc_scl_bit(0);
	rtc_i2c_sleep(2);
	
}
void rtc_i2c_stop(void)
{
	rtc_sda_dir(G_OUTPUT);
	rtc_scl_dir(G_OUTPUT);
	rtc_scl_bit(0);
	rtc_i2c_sleep(1);
	rtc_sda_bit(0);
	rtc_i2c_sleep(1);
	rtc_scl_bit(1);
	rtc_i2c_sleep(5);
	rtc_sda_bit(1);
	rtc_i2c_sleep(5);
	rtc_scl_bit(0);
	rtc_i2c_sleep(2);
}

void rtc_i2c_send_ack(int ack)
{
	rtc_sda_dir(G_OUTPUT);
	rtc_sda_bit(ack);
	rtc_i2c_sleep(3);
	rtc_scl_bit(1);
	rtc_i2c_sleep(5);
	rtc_scl_bit(0);
	rtc_i2c_sleep(2);
}

char rtc_i2c_rec_ack(void)
{
        char res = 1;
        int num=10;
	int tmp;
        rtc_sda_dir(G_INPUT);
        rtc_i2c_sleep(3);
        rtc_scl_bit(1);
        rtc_i2c_sleep(5);
#ifdef	SM502_USE_LOWC
	tmp = ((*GPIO_DATA_LOW)&(1 << DAT_PIN) );
#else
	tmp = ((*GPIO_DATA_HIGH)&0x8000);
#endif

        //wait for a ack signal from slave

        while(tmp)
        {
                rtc_i2c_sleep(1);
                num--;
                if(!num)
                {
                        res = 0;
                        break;
                }
#ifdef	SM502_USE_LOWC
		tmp = ((*GPIO_DATA_LOW)&(1 << DAT_PIN));
#else
		tmp = ((*GPIO_DATA_HIGH)&0x8000);
#endif
        }
        rtc_scl_bit(0);
        rtc_i2c_sleep(3);
        return res;
}

unsigned char rtc_i2c_rec(void)
{
	int i;
	int tmp;
	unsigned char or_char;
	unsigned char value = 0x00;
	rtc_sda_dir(G_INPUT);
	for(i=7;i>=0;i--)
	{
//		rtc_i2c_sleep(6);
		rtc_i2c_sleep(5);
		rtc_scl_bit(1);
		rtc_i2c_sleep(3);
#ifdef	SM502_USE_LOWC
		tmp = ((*GPIO_DATA_LOW)&(1 << DAT_PIN));
#else
		tmp = ((*GPIO_DATA_HIGH)&0x8000);
#endif

		if(tmp)
			or_char=0x1;
		else
			or_char=0x0;
		or_char<<=i;
		value|=or_char;
		rtc_i2c_sleep(3);
		rtc_scl_bit(0);
	}
	return value;
}

unsigned char rtc_i2c_send(unsigned char value)
{//we assume that now scl is 0
	int i;
	unsigned char and_char;
	rtc_sda_dir(G_OUTPUT);
	for(i=7;i>=0;i--)
	{
		and_char = value;
		and_char>>=i;
		and_char&=0x1;
		if(and_char)
			rtc_sda_bit(1);
		else
			rtc_sda_bit(0);
		rtc_i2c_sleep(1);
		rtc_scl_bit(1);
		rtc_i2c_sleep(5);
		rtc_scl_bit(0);
//		rtc_i2c_sleep(3);
		rtc_i2c_sleep(1);
	}
	rtc_sda_bit(1);	
	return 1;
}

static unsigned short rtc_i2c_rec16(void)
{
	int i;
	int tmp;
	unsigned short or_char;
	unsigned short value = 0x00;

	rtc_sda_dir(G_INPUT);
	for(i=15;i>=0;i--)
	{
//		rtc_i2c_sleep(6);
		rtc_i2c_sleep(5);
		rtc_scl_bit(1);
		rtc_i2c_sleep(4);
#ifdef	SM502_USE_LOWC
		tmp = ((*GPIO_DATA_LOW)&(1 << DAT_PIN));
#else
		tmp = ((*GPIO_DATA_HIGH)&0x8000);
#endif

		if(tmp)
			or_char=0x1;
		else
			or_char=0x0;
		or_char<<=i;
		value|=or_char;
		rtc_i2c_sleep(3);
		rtc_scl_bit(0);
	}
	return value;
}

static unsigned char rtc_i2c_send16(unsigned short value)
{//we assume that now scl is 0
	int i;
	unsigned char and_char;

	rtc_sda_dir(G_OUTPUT);
	for(i=15;i>=0;i--)
	{
		and_char = value;
		and_char>>=i;
		and_char&=0x1;
		if(and_char)
			rtc_sda_bit(1);
		else
			rtc_sda_bit(0);
		rtc_i2c_sleep(1);
		rtc_scl_bit(1);
		rtc_i2c_sleep(5);
		rtc_scl_bit(0);
//		rtc_i2c_sleep(3);
		rtc_i2c_sleep(1);
	}
	rtc_sda_bit(1);	
	return 1;
}


unsigned char rtc_i2c_rec_s(unsigned char slave_addr,unsigned char sub_addr,unsigned char* buf ,int count)
{
	
	unsigned char value;
	while(count)
	{
		//start signal
		rtc_i2c_start();
		//write slave_addr
		rtc_i2c_send(slave_addr);
		if(!rtc_i2c_rec_ack())
			return 0;
		//write sub_addr
		rtc_i2c_send(sub_addr);
		if(!rtc_i2c_rec_ack())
			return 0;

		//repeat start
		rtc_i2c_start();
		//write slave_addr+1
		rtc_i2c_send(slave_addr+0x1);
		if(!rtc_i2c_rec_ack())
			return 0;
		//read data
		value=rtc_i2c_rec();	
		rtc_i2c_send_ack(1);//***add in***//
		rtc_i2c_stop();
		
		//deal the data
		*buf=value;
		buf++;	
		count--;
		sub_addr++;
	}

	return 1;
}

unsigned char rtc_i2c_send_s(unsigned char slave_addr,unsigned char sub_addr,unsigned char * buf ,int count)
{
	while(count)
	{	
		rtc_i2c_start();	
		rtc_i2c_send(slave_addr);
		if(!rtc_i2c_rec_ack()){
			printf("no_ack 1111\n");
			return 0;
		}
		rtc_i2c_send(sub_addr);
		if(!rtc_i2c_rec_ack()){
			printf("no_ack 2222\n");
			return 0;
		}
		rtc_i2c_send(*buf);
		if(!rtc_i2c_rec_ack()){
			printf("no_ack 3333\n");
			return 0;
		}
		rtc_i2c_stop();
		//deal the data
		buf++;
		count--;
		sub_addr++;
	}
	return 1;
}

unsigned short rtc_i2c_rec_s16(unsigned char slave_addr,unsigned char sub_addr,unsigned short* buf ,int count)
{
	
	unsigned short value;
	while(count)
	{
		//start signal
		rtc_i2c_start();
		//write slave_addr
		rtc_i2c_send(slave_addr);
		if(!rtc_i2c_rec_ack())
			return 0;
		//write sub_addr
		rtc_i2c_send(sub_addr);
		if(!rtc_i2c_rec_ack())
			return 0;

		//repeat start
		rtc_i2c_start();
		//write slave_addr+1
		rtc_i2c_send(slave_addr+0x1);
		if(!rtc_i2c_rec_ack())
			return 0;
		//read data
		value=rtc_i2c_rec();
		rtc_i2c_send_ack(0);//***add in***//
		value=(value << 8) | rtc_i2c_rec();

		rtc_i2c_stop();
		//deal the data
		*buf=value;
		buf++;	
		count--;
		sub_addr++;
	}

	return 1;
}

unsigned char rtc_i2c_send_s16(unsigned char slave_addr,unsigned char sub_addr,unsigned short* buf ,int count)
{
	unsigned char *data8 = buf;

	while(count)
	{	
		rtc_i2c_start();
		rtc_i2c_send(slave_addr);
		if(!rtc_i2c_rec_ack()){
			printf("no_ack 1111\n");
			return 0;
		}
		rtc_i2c_send(sub_addr);
		if(!rtc_i2c_rec_ack()){
			printf("no_ack 2222\n");
			return 0;
		}
		rtc_i2c_send(data8[1]);
		if(!rtc_i2c_rec_ack()){
			printf("no_ack 3333\n");
			return 0;
		}
		rtc_i2c_send(data8[0]);
		if(!rtc_i2c_rec_ack()){
			printf("no_ack 4444\n");
			return 0;
		}
		rtc_i2c_stop();
		//deal the data
		buf++;
		count--;
		sub_addr++;
	}
	return 1;
}

#endif

int wriic2(int argc,char **argv)
{
	pcitag_t tag;
	unsigned long temp;
	int ret = 0;
	unsigned char slave_addr, index_addr;
	unsigned char value;

	tag  = _pci_make_tag(0, 14, 0);
	mmio = _pci_conf_readn(tag, 0x14, 4);
	mmio = (int)mmio | 0xb0000000;
	temp = _pci_conf_readn(tag, 0x04, 4);
	_pci_conf_writen(tag, 0x04, temp | 0x07, 0x04);
	temp = _pci_conf_readn(tag, 0x04, 4);

	rtc_sda_dir(G_OUTPUT);
	rtc_scl_dir(G_OUTPUT);
	rtc_sda_bit(1);
	rtc_scl_bit(1);
	rtc_i2c_sleep(1000);

	slave_addr = strtoul(argv[1], 0, 16);
	index_addr = strtoul(argv[2], 0, 16);
	value 	   = strtoul(argv[3], 0, 16);
	printf("slave_addr 0x%x, index_addr 0x%x, value\n", slave_addr, index_addr, value);


    //start signal
    rtc_i2c_start();
    //write slave_addr
    rtc_i2c_send(slave_addr);
    if(!rtc_i2c_rec_ack()){
		ret = 1;
		goto out;
	}       
	//write sub_addr
	rtc_i2c_send(index_addr);
	if(!rtc_i2c_rec_ack()){
	    ret = 2;
	    goto out;
	}
	// write data	
	rtc_i2c_send(value);
	if(!rtc_i2c_rec_ack()){
		ret = 3;
		goto out;
	}

	// stop
	rtc_i2c_stop();

out :
    if(ret){
        printf("slave_addr : 0x%x, index_addr : 0x%x, value 0x%x, error =         %d\n", slave_addr, index_addr, value, ret);
    }else{
        printf("slave_addr : 0x%x, index_addr : 0x%x, value 0x%x\n",
             slave_addr, index_addr, value);
    }
	
	return 0;
}

int rdiic2(int argc,char **argv)
{
	pcitag_t tag;
	unsigned long temp;
	int ret = 0;
	unsigned char slave_addr, index_addr;
	unsigned char value;

	slave_addr = strtoul(argv[1], 0, 16);
	index_addr = strtoul(argv[2], 0, 16);
	printf("slave_addr 0x%x, index_addr 0x%x\n", slave_addr, index_addr);

    //start signal
    rtc_i2c_start();
    //write slave_addr
    rtc_i2c_send(slave_addr);
    if(!rtc_i2c_rec_ack()){
		ret = 1;
		goto out;
	}       
	//write sub_addr
	rtc_i2c_send(index_addr);
	if(!rtc_i2c_rec_ack()){
	    ret = 2;
	    goto out;
	}
	//repeat start
	rtc_i2c_start();
	//write slave_addr+1
	rtc_i2c_send(slave_addr+0x1);
	if(!rtc_i2c_rec_ack()){
		ret = 3;
		goto out;
	}
	// read data
	value = rtc_i2c_rec();
	rtc_i2c_send_ack(1);//***add in***//
	rtc_i2c_stop();

out :
    if(ret){
        printf("slave_addr : 0x%x, index_addr : 0x%x, value 0x%x, error =         %d\n", slave_addr, index_addr, value, ret);
    }else{
        printf("slave_addr : 0x%x, index_addr : 0x%x, value 0x%x\n",
             slave_addr, index_addr, value);
    }
	
	return 0;
}

static const Cmd Cmds[] =
{
	{"MyCmds"},
	{"rdiic2","", 0, "read rtc i2c slave registers", rdiic2, 3, 3, 0},
	{"wriic2","", 0, "write rtc i2c slave registers", wriic2, 4, 4, 0},
	{0, 0}
};

static void init_cmd __P((void)) __attribute__ ((constructor));

void init_cmd()
{
	cmdlist_expand(Cmds, 1);
}
