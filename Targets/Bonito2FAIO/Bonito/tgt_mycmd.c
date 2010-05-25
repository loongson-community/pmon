#define nr_printf printf
#define nr_gets gets
#define nr_strtol strtoul
//-------------------------------------------PNP------------------------------------------
// MB PnP configuration register

#define PNP_KEY_ADDR (0xbfd00000+0x3f0)
#define PNP_DATA_ADDR (0xbfd00000+0x3f1)



static unsigned char slave_addr;

void PNPSetConfig(char Index, char data);
char PNPGetConfig(char Index);

#define SUPERIO_CFG_REG 0x85

void EnterMBPnP(void)
{
	pcitag_t tag;
	char confval;
	tag=_pci_make_tag(VTSB_BUS,VTSB_DEV, VTSB_ISA_FUNC);
	confval=_pci_conf_readn(tag,SUPERIO_CFG_REG,1);
	_pci_conf_writen(tag,SUPERIO_CFG_REG,confval|2,1);	
}

void ExitMBPnP(void)
{
	pcitag_t tag;
	char confval,val;
	tag=_pci_make_tag(VTSB_BUS,VTSB_DEV, VTSB_ISA_FUNC);
	confval=_pci_conf_readn(tag,SUPERIO_CFG_REG,1);
	_pci_conf_writen(tag,SUPERIO_CFG_REG,confval&~2,1);	
}

void PNPSetConfig(char Index, char data)
{
        EnterMBPnP();                                // Enter IT8712 MB PnP mode
        outb(PNP_KEY_ADDR,Index);
        outb(PNP_DATA_ADDR,data);
        ExitMBPnP();
}

char PNPGetConfig(char Index)
{
        char rtn;

        EnterMBPnP();                                // Enter IT8712 MB PnP mode
        outb(PNP_KEY_ADDR,Index);
        rtn = inb(PNP_DATA_ADDR);
        ExitMBPnP();
        return rtn;
}



int dumpsis(int argc,char **argv)
{
int i;
volatile unsigned char *p=0xbfd003c4;
unsigned char c;
for(i=0;i<0x15;i++)
{
p[0]=i;
c=p[1];
printf("sr%x=0x%02x\n",i,c);
}
p[0]=5;
p[1]=0x86;
printf("after set 0x86 to sr5\n");
for(i=0;i<0x15;i++)
{
p[0]=i;
c=p[1];
printf("sr%x=0x%02x\n",i,c);
}
return 0;
}

unsigned char i2cread(char slot,char offset);


union commondata{
		unsigned char data1;
		unsigned short data2;
		unsigned int data4;
		unsigned int data8[2];
		unsigned char c[8];
};

#include "target/via686b.h"
static int i2cslot=0;

#ifndef DEVBD2F_SM502

static int DimmRead(int type,long long addr,union commondata *mydata)
{
char c;
switch(type)
{
case 1:
linux_outb((i2cslot<<1)+0xa1,SMBUS_HOST_ADDRESS);
linux_outb(addr,SMBUS_HOST_COMMAND);
linux_outb(0x8,SMBUS_HOST_CONTROL); 
if((c=linux_inb(SMBUS_HOST_STATUS))&0x1f)
{
linux_outb(c,SMBUS_HOST_STATUS);
}

linux_outb(linux_inb(SMBUS_HOST_CONTROL)|0x40,SMBUS_HOST_CONTROL);

while(linux_inb(SMBUS_HOST_STATUS)&SMBUS_HOST_STATUS_BUSY);

if((c=linux_inb(SMBUS_HOST_STATUS))&0x1f)
{
linux_outb(c,SMBUS_HOST_STATUS);
}

mydata->data1=linux_inb(SMBUS_HOST_DATA0);
break;

default: return -1;break;
}
return 0;
}

#endif
static int DimmWrite(int type,long long addr,union commondata *mydata)
{
return -1;
}


static int Ics950220Read(int type,long long addr,union commondata *mydata)
{
char c;
switch(type)
{
case 1:
linux_outb(0xd3,SMBUS_HOST_ADDRESS); //0xd3
linux_outb(addr,SMBUS_HOST_COMMAND);
linux_outb(1,SMBUS_HOST_DATA0);
linux_outb(0x14,SMBUS_HOST_CONTROL); //0x14
if((c=linux_inb(SMBUS_HOST_STATUS))&0x1f)
{
linux_outb(c,SMBUS_HOST_STATUS);
}

linux_outb(linux_inb(SMBUS_HOST_CONTROL)|0x40,SMBUS_HOST_CONTROL);

while(linux_inb(SMBUS_HOST_STATUS)&SMBUS_HOST_STATUS_BUSY);

if((c=linux_inb(SMBUS_HOST_STATUS))&0x1f)
{
linux_outb(c,SMBUS_HOST_STATUS);
}

mydata->data1=linux_inb(SMBUS_HOST_DATA1+1);
break;

default: return -1;break;
}
return 0;
}

static int rom_ddr_reg_read(int type,long long addr,union commondata *mydata)
{
    char *nvrambuf;
	extern char ddr2_reg_data,_start;
	nvrambuf = 0xbfc00000+((int)&ddr2_reg_data -(int)&_start)+addr;
	//printf("ddr2_reg_data=%x\nbuf=%x,ddr=%x\n",&ddr2_reg_data,nvrambuf,addr);
	switch(type)
	{
		case 1:memcpy(&mydata->data1,nvrambuf,1);break;
		case 2:memcpy(&mydata->data2,nvrambuf,2);break;
		case 4:memcpy(&mydata->data4,nvrambuf,4);break;
		case 8:memcpy(&mydata->data8,nvrambuf,8);break;
	}
	return 0;
}

static int rom_ddr_reg_write(int type,long long addr,union commondata *mydata)
{
	char *nvrambuf;
	char *nvramsecbuf;
	char *nvram;
	int offs;
	extern char ddr2_reg_data,_start;
	struct fl_device *dev=fl_devident(0xbfc00000,0);
	int nvram_size=dev->fl_secsize;

	nvram = 0xbfc00000+((int)&ddr2_reg_data -(int)&_start);
	offs=(int)nvram &(nvram_size - 1);
	nvram  =(int)nvram & ~(nvram_size - 1);

	/* Deal with an entire sector even if we only use part of it */

	/* If NVRAM is found to be uninitialized, reinit it. */

	/* Find end of evironment strings */
	nvramsecbuf = (char *)malloc(nvram_size);

	if(nvramsecbuf == 0) {
		printf("Warning! Unable to malloc nvrambuffer!\n");
		return(-1);
	}

	memcpy(nvramsecbuf, nvram, nvram_size);
	if(fl_erase_device(nvram, nvram_size, FALSE)) {
			printf("Error! Nvram erase failed!\n");
			free(nvramsecbuf);
			return(0);
	}

	nvrambuf = nvramsecbuf + offs;
	switch(type)
	{
		case 1:memcpy(nvrambuf+addr,&mydata->data1,1);break;
		case 2:memcpy(nvrambuf+addr,&mydata->data2,2);break;
		case 4:memcpy(nvrambuf+addr,&mydata->data4,4);break;
		case 8:memcpy(nvrambuf+addr,&mydata->data8,8);break;
	}

	if(fl_program_device(nvram, nvramsecbuf, nvram_size, FALSE)) {
			printf("Error! Nvram program failed!\n");
			free(nvramsecbuf);
			return(0);
	}
	free(nvramsecbuf);
	return 0;
}

#ifdef DEVBD2F_SM502
//#define	SM502_USE_LOWC
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
#define	DAT_PIN			47//13	
#define	CLK_PIN			46//6

static void i2c_sleep(int ntime)
{
	int i,j=0;
	// 700~5K in v3
	//for(i=0; i<700*ntime; i++)	// 20KHz
	//for(i=0; i<140*ntime; i++)	// 50
	//for(i=0; i<280*ntime; i++)		// 50KHz
//	for(i=0; i<493*ntime; i++)		// 50KHz on 618MHz
	//for(i=0; i<986*ntime; i++)		// 100KHz on 618MHz
	//for(i=0; i<1640*ntime; i++)		// 25KHz on 900MHz
	for(i=0; i<3280*ntime; i++)		// 25KHz on 900MHz
//	for(i=0; i<175*ntime; i++)		// 50KHz
	{
		j=i;
		j+=i;
	}
}

#ifdef	SM502_USE_LOWC
void sda_dir(int ivalue)
{
	int tmp;
	tmp = *GPIO_DIR_LOW;
	if(ivalue == 1)
		*GPIO_DIR_LOW = tmp|(0x1<<DAT_PIN);
	else
		*GPIO_DIR_LOW = tmp&(~(0x1<<DAT_PIN));
}
void scl_dir(int ivalue)
{
	int tmp;
	tmp = *GPIO_DIR_LOW;
	if(ivalue == 1)
		*GPIO_DIR_LOW = tmp|(0x1<<CLK_PIN);
	else
		*GPIO_DIR_LOW = tmp&(~(0x1<<CLK_PIN));
}

void sda_bit(int ivalue)
{
	int tmp;
	tmp = *GPIO_DATA_LOW;
	if(ivalue == 1)
		*GPIO_DATA_LOW = tmp|(0x1<<DAT_PIN);
	else
		*GPIO_DATA_LOW = tmp&(~(0x1<<DAT_PIN));
}
void scl_bit(int ivalue)
{
	int tmp;
	tmp = *GPIO_DATA_LOW;
	if(ivalue == 1)
		*GPIO_DATA_LOW = tmp|(0x1<<CLK_PIN);
	else
		*GPIO_DATA_LOW = tmp&(~(0x1<<CLK_PIN));
}

#else
void sda_dir(int ivalue)
{
	int tmp;
	tmp = *GPIO_DIR_HIGH;
	if(ivalue == 1)
		*GPIO_DIR_HIGH = tmp|(0x1<<15);
	else
		*GPIO_DIR_HIGH = tmp&(~(0x1<<15));
}
void scl_dir(int ivalue)
{
	int tmp;
	tmp = *GPIO_DIR_HIGH;
	if(ivalue == 1)
		*GPIO_DIR_HIGH = tmp|(0x1<<14);
	else
		*GPIO_DIR_HIGH = tmp&(~(0x1<<14));
}

void sda_bit(int ivalue)
{
	int tmp;
	tmp = *GPIO_DATA_HIGH;
	if(ivalue == 1)
		*GPIO_DATA_HIGH = tmp|(0x1<<15);
	else
		*GPIO_DATA_HIGH = tmp&(~(0x1<<15));
}
void scl_bit(int ivalue)
{
	int tmp;
	tmp = *GPIO_DATA_HIGH;
	if(ivalue == 1)
		*GPIO_DATA_HIGH = tmp|(0x1<<14);
	else
		*GPIO_DATA_HIGH = tmp&(~(0x1<<14));
}
#endif

static void i2c_start(void)
{
	sda_dir(G_OUTPUT);
	scl_dir(G_OUTPUT);
	scl_bit(0);
	i2c_sleep(1);
	sda_bit(1);
	i2c_sleep(1);
	scl_bit(1);
	i2c_sleep(5);
	sda_bit(0);
	i2c_sleep(5);
	scl_bit(0);
	i2c_sleep(2);
	
}
static void i2c_stop(void)
{
	sda_dir(G_OUTPUT);
	scl_dir(G_OUTPUT);
	scl_bit(0);
	i2c_sleep(1);
	sda_bit(0);
	i2c_sleep(1);
	scl_bit(1);
	i2c_sleep(5);
	sda_bit(1);
	i2c_sleep(5);
	scl_bit(0);
	i2c_sleep(2);
	
		
}

static void i2c_send_ack(int ack)
{
	sda_dir(G_OUTPUT);
	sda_bit(ack);
	i2c_sleep(3);
	scl_bit(1);
	i2c_sleep(5);
	scl_bit(0);
	i2c_sleep(2);
}

static char i2c_rec_ack(void)
{
        char res = 1;
        int num=10;
	int tmp;
        sda_dir(G_INPUT);
        i2c_sleep(3);
        scl_bit(1);
        i2c_sleep(5);
#ifdef	SM502_USE_LOWC
	tmp = ((*GPIO_DATA_LOW)&(1 << DAT_PIN) );
#else
	tmp = ((*GPIO_DATA_HIGH)&0x8000);
#endif

        //wait for a ack signal from slave

        while(tmp)
        {
                i2c_sleep(1);
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
        scl_bit(0);
        i2c_sleep(3);
        return res;
}

static unsigned char i2c_rec(void)
{
	int i;
	int tmp;
	unsigned char or_char;
	unsigned char value = 0x00;
	sda_dir(G_INPUT);
	for(i=7;i>=0;i--)
	{
//		i2c_sleep(6);
		i2c_sleep(7);
		scl_bit(1);
		i2c_sleep(2);
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
		i2c_sleep(2);
		scl_bit(0);
	}
	return value;
}

static unsigned char i2c_send(unsigned char value)
{//we assume that now scl is 0
	int i;
	unsigned char and_char;
	sda_dir(G_OUTPUT);
	for(i=7;i>=0;i--)
	{
		and_char = value;
		and_char>>=i;
		and_char&=0x1;
		if(and_char)
			sda_bit(1);
		else
			sda_bit(0);
		i2c_sleep(1);
		scl_bit(1);
		i2c_sleep(4);
		scl_bit(0);
//		i2c_sleep(3);
		i2c_sleep(2);
	}
	sda_bit(1);	
	return 1;
}

static unsigned short i2c_rec16(void)
{
	int i;
	int tmp;
	unsigned short or_char;
	unsigned short value = 0x00;
	sda_dir(G_INPUT);
	for(i=15;i>=0;i--)
	{
//		i2c_sleep(6);
		i2c_sleep(5);
		scl_bit(1);
		i2c_sleep(4);
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
		i2c_sleep(3);
		scl_bit(0);
	}
	return value;
}

static unsigned char i2c_send16(unsigned short value)
{//we assume that now scl is 0
	int i;
	unsigned char and_char;
	sda_dir(G_OUTPUT);
	for(i=15;i>=0;i--)
	{
		and_char = value;
		and_char>>=i;
		and_char&=0x1;
		if(and_char)
			sda_bit(1);
		else
			sda_bit(0);
		i2c_sleep(1);
		scl_bit(1);
		i2c_sleep(5);
		scl_bit(0);
//		i2c_sleep(3);
		i2c_sleep(1);
	}
	sda_bit(1);	
	return 1;
}


unsigned char i2c_rec_s(unsigned char slave_addr,unsigned char sub_addr,unsigned char* buf ,int count)
{
	
	unsigned char value;
	while(count)
	{
		//start signal
		i2c_start();
		//write slave_addr
		i2c_send(slave_addr);
		if(!i2c_rec_ack())
			return 0;
		//write sub_addr
		i2c_send(sub_addr);
		if(!i2c_rec_ack())
			return 0;

		//repeat start
		i2c_start();
		//write slave_addr+1
		i2c_send(slave_addr+0x1);
		if(!i2c_rec_ack())
			return 0;
		//read data
		value=i2c_rec();	
		i2c_send_ack(1);//***add in***//
		i2c_stop();
		
		//deal the data
		*buf=value;
		buf++;	
		count--;
		sub_addr++;
	}

	return 1;
}

unsigned char i2c_send_s(unsigned char slave_addr,unsigned char sub_addr,unsigned char * buf ,int count)
{
	while(count)
	{	
		i2c_start();	
		i2c_send(slave_addr);
		if(!i2c_rec_ack()){
			tgt_printf("no_ack 1111\n");
			return 0;
		}
		i2c_send(sub_addr);
		if(!i2c_rec_ack()){
			tgt_printf("no_ack 2222\n");
			return 0;
		}
		i2c_send(*buf);
		if(!i2c_rec_ack()){
			tgt_printf("no_ack 3333\n");
			return 0;
		}
		i2c_stop();
		//deal the data
		buf++;
		count--;
		sub_addr++;
	}
	return 1;
}

unsigned short i2c_rec_s16(unsigned char slave_addr,unsigned char sub_addr,unsigned short* buf ,int count)
{
	
	unsigned short value;
	while(count)
	{
		//start signal
		i2c_start();
		//write slave_addr
		i2c_send(slave_addr);
		if(!i2c_rec_ack())
			return 0;
		//write sub_addr
		i2c_send(sub_addr);
		if(!i2c_rec_ack())
			return 0;

		//repeat start
		i2c_start();
		//write slave_addr+1
		i2c_send(slave_addr+0x1);
		if(!i2c_rec_ack())
			return 0;
		//read data
		value=i2c_rec();
		i2c_send_ack(0);//***add in***//
		value=(value << 8) | i2c_rec();

		i2c_stop();
		//deal the data
		*buf=value;
		buf++;	
		count--;
		sub_addr++;
	}

	return 1;
}

unsigned char i2c_send_s16(unsigned char slave_addr,unsigned char sub_addr,unsigned short* buf ,int count)
{
	unsigned char *data8 = buf;

	while(count)
	{	
		i2c_start();
		i2c_send(slave_addr);
		if(!i2c_rec_ack()){
			tgt_printf("no_ack 1111\n");
			return 0;
		}
		i2c_send(sub_addr);
		if(!i2c_rec_ack()){
			tgt_printf("no_ack 2222\n");
			return 0;
		}
		i2c_send(data8[1]);
		if(!i2c_rec_ack()){
			tgt_printf("no_ack 3333\n");
			return 0;
		}
		i2c_send(data8[0]);
		if(!i2c_rec_ack()){
			tgt_printf("no_ack 4444\n");
			return 0;
		}
		i2c_stop();
		//deal the data
		buf++;
		count--;
		sub_addr++;
	}
	return 1;
}

static int sm502SPDRead(int type,long long addr,union commondata *mydata)
{
	char c;

//	printf("mmio =%x\n",mmio);
//	printf("%x\n",addr);
	switch(type)
	{
	case 1:
		if(i2c_rec_s((unsigned char)slave_addr,addr,&c,1))
			memcpy(&mydata->data1,&c,1);
		else
			printf("rec data error!\n");
		break;
	default:
		return -1;
	}
	return 0;
}
static int sm502SPDWrite(int type,long long addr,union commondata *mydata)
{
	char c;

	switch(type)
	{
	case 1:	
	
		memcpy(&c,&mydata->data1,1);
		if(i2c_send_s((unsigned char)slave_addr,addr,&c,1))
			;
		else
			printf("send data error\n");
		break;
	default:
		return -1;
	}
	return 0;
}
static int sm502RtcRead(int type,long long addr,union commondata *mydata)
{
	char c;

//	printf("mmio =%x\n",mmio);
//	printf("%x\n",addr);
	switch(type)
	{
	case 1:
		if(i2c_rec_s((unsigned char)0x64,addr<<4,&c,1))
			memcpy(&mydata->data1,&c,1);
		else
			printf("rec data error!\n");
		break;
	default:
		return -1;
	}
	return 0;
}
static int sm502RtcWrite(int type,long long addr,union commondata *mydata)
{
	char c;

	switch(type)
	{
	case 1:	
	
		memcpy(&c,&mydata->data1,1);
		if(i2c_send_s((unsigned char)0x64,addr<<4,&c,1))
			;
		else
			printf("send data error\n");
		break;
	default:
		return -1;
	}
	return 0;
}


static int DimmRead(int type,long long addr,union commondata *mydata)
{
switch(type)
{
case 1:
		i2c_start();	
		i2c_send(0xa0|(i2cslot<<1));
		if(!i2c_rec_ack())
			return -1;
		i2c_send(addr);
		if(!i2c_rec_ack())
			return -1;
		i2c_start();
		i2c_send(0xa1|(i2cslot<<1));
		if(!i2c_rec_ack())
			return -1;
		mydata->data1=i2c_rec();
		i2c_stop();
		break;
default:
		return -1;
}
		return 0;
}





void i2c_test()
{
	unsigned int  rst,suc;
	asm volatile(
		".extern newi2cread \t\n"	\
		"li	$4,0xa0 \t\n"		\
		"li	$5,0x0 \t\n"		\
		"jal	newi2cread \t\n"	\
		"nop	\t\n"			\
		"move	%0,$2 \t\n"		\
		"move	%1,$3 \t\n"
		:"=r"(rst),"=r"(suc)
		:
		:"$4","$5"
		);
	printf("rst  %x  suc  %x \n",rst,suc);


	asm volatile(
		".extern newi2cread \t\n"	\
		"li	$4,0xa0 \t\n"		\
		"li	$5,0x7 \t\n"		\
		"jal	newi2cread \t\n"	\
		"nop	\t\n"			\
		"move	%0,$2 \t\n"		\
		"move	%1,$3 \t\n"
		:"=r"(rst),"=r"(suc)
		:
		:"$4","$5"
		);
	printf("rst  %x suc %x \n",rst,suc);

	asm volatile(
		".extern newi2cread \t\n"	\
		"li	$4,0xa0 \t\n"		\
		"li	$5,0x8 \t\n"		\
		"jal	newi2cread \t\n"	\
		"nop	\t\n"			\
		"move	%0,$2 \t\n"		\
		"move	%1,$3 \t\n"
		:"=r"(rst),"=r"(suc)
		:
		:"$4","$5"
		);
	printf("rst  %x suc %x \n",rst,suc);
}
#endif

int wriic(int argc,char **argv)
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


	sda_dir(G_OUTPUT);
	scl_dir(G_OUTPUT);
	sda_bit(1);
	scl_bit(1);
	i2c_sleep(1000);

	slave_addr = strtoul(argv[1], 0, 16);
	index_addr = strtoul(argv[2], 0, 16);
	value 	   = strtoul(argv[3], 0, 16);
	printf("slave_addr 0x%x, index_addr 0x%x, value\n", slave_addr, index_addr, value);


    //start signal
    i2c_start();
    //write slave_addr
    i2c_send(slave_addr);
    if(!i2c_rec_ack()){
		ret = 1;
		goto out;
	}       
	//write sub_addr
	i2c_send(index_addr);
	if(!i2c_rec_ack()){
	    ret = 2;
	    goto out;
	}
	// write data	
	i2c_send(value);
	if(!i2c_rec_ack()){
		ret = 3;
		goto out;
	}

	// stop
	i2c_stop();

out :
    if(ret){
        printf("slave_addr : 0x%x, index_addr : 0x%x, value 0x%x, error =         %d\n", slave_addr, index_addr, value, ret);
    }else{
        printf("slave_addr : 0x%x, index_addr : 0x%x, value 0x%x\n",
             slave_addr, index_addr, value);
    }
	
	return 0;
}

int rdiic(int argc,char **argv)
{
	pcitag_t tag;
	unsigned long temp;
	int ret = 0;
	unsigned char slave_addr, index_addr;
	unsigned char value;
#if 0
	tag  = _pci_make_tag(0, 14, 0);
	mmio = (volatile char *)_pci_conf_readn(tag, 0x14, 4);
	mmio = (volatile char *)((int)mmio | 0xb0000000);
	temp = _pci_conf_readn(tag, 0x04, 4);
	_pci_conf_writen(tag, 0x04, temp | 0x07, 0x04);
	temp = _pci_conf_readn(tag, 0x04, 4);


	sda_dir(G_OUTPUT);
	scl_dir(G_OUTPUT);
	sda_bit(1);
	scl_bit(1);
	i2c_sleep(1000);
#endif

	slave_addr = strtoul(argv[1], 0, 16);
	index_addr = strtoul(argv[2], 0, 16);
	printf("slave_addr 0x%x, index_addr 0x%x\n", slave_addr, index_addr);


    //start signal
    i2c_start();
    //write slave_addr
    i2c_send(slave_addr);
    if(!i2c_rec_ack()){
		ret = 1;
		goto out;
	}       
	//write sub_addr
	i2c_send(index_addr);
	if(!i2c_rec_ack()){
	    ret = 2;
	    goto out;
	}
	//repeat start
	i2c_start();
	//write slave_addr+1
	i2c_send(slave_addr+0x1);
	if(!i2c_rec_ack()){
		ret = 3;
		goto out;
	}
	// read data
	value = i2c_rec();
	i2c_send_ack(1);//***add in***//
	i2c_stop();

out :
    if(ret){
        printf("slave_addr : 0x%x, index_addr : 0x%x, value 0x%x, error =         %d\n", slave_addr, index_addr, value, ret);
    }else{
        printf("slave_addr : 0x%x, index_addr : 0x%x, value 0x%x\n",
             slave_addr, index_addr, value);
    }
	
	return 0;
}

int flash_send(unsigned char slave_addr, unsigned char index_addr, unsigned char value)
{
	unsigned char ret = 0;
	
	sda_dir(G_OUTPUT);
	scl_dir(G_OUTPUT);
	sda_bit(1);
	scl_bit(1);
	i2c_sleep(1000);

	//start signal
	i2c_start();
	//write slave_addr
	i2c_send(slave_addr);
    	if(!i2c_rec_ack()){
		ret = 1;
		goto out;
	}       
	//write sub_addr
	i2c_send(index_addr);
	if(!i2c_rec_ack()){
	    ret = 2;
	    goto out;
	}
	// write data	
	i2c_send(value);
	if(!i2c_rec_ack()){
		ret = 3;
		goto out;
	}

	// stop
	i2c_stop();

out :
#if	0
	if(ret){
        	printf("slave_addr : 0x%x, index_addr : 0x%x, value 0x%x, error =         %d\n", slave_addr, index_addr, value, ret);
	}else{
       		 printf("slave_addr : 0x%x, index_addr : 0x%x, value 0x%x\n",
             slave_addr, index_addr, value);
	}
#endif

	return ret;
}


int flash_recv(unsigned char slave_addr, unsigned char index_addr, unsigned char *value)
{
	unsigned char ret = 0;
	
	sda_dir(G_OUTPUT);
	scl_dir(G_OUTPUT);
	sda_bit(1);
	scl_bit(1);
	i2c_sleep(1000);

        //start signal
        i2c_start();
        //write slave_addr
        i2c_send(slave_addr);
        if(!i2c_rec_ack()){
		ret = 1;
		goto out;
	}       
	//write sub_addr
	i2c_send(index_addr);
	if(!i2c_rec_ack()){
	    ret = 2;
	    goto out;
	}
	//repeat start
	i2c_start();
	//write slave_addr+1
	i2c_send(slave_addr+0x1);
	if(!i2c_rec_ack()){
		ret = 3;
		goto out;
	}
	// read data
	*value = i2c_rec();
	i2c_send_ack(1);//***add in***//
	i2c_stop();
	
out : 
#if	0
    if(ret){
        printf("slave_addr : 0x%x, index_addr : 0x%x, value 0x%x, error =         %d\n", slave_addr, index_addr, *value, ret);
    }else{
        printf("slave_addr : 0x%x, index_addr : 0x%x, value 0x%x\n",
             slave_addr, index_addr, *value);
    }
#endif
	return ret;
}

#define	FLASH_ID_0		0x47	// 'G'
#define	FLASH_ID_1		0x44	// 'D'
#define	FLASH_ID_2		0x49	// 'I'
#define	FLASH_ID_3		0x55	// 'U'
#define	FLASH_ID_4		0x4D	// 'M'
#define	FLASH_ID_S		0x53	// 'S'	// for start program
#define	FLASH_ID_E		0x45	// 'E'	// for end   program
#define	FLASH_ACK_ID		0xbb	// for answer master for on

#define	IIC_BLOCK_SIZE		0x20
#define	MAX_DATA_SIZE		0x100
unsigned char iic_data[MAX_DATA_SIZE] = {
	0x56, 0x67, 0x78, 0x89, 0x90, 0x01, 0xaa, 0xbb,
	0x66, 0x67, 0x78, 0x89, 0x90, 0x01, 0xaa, 0xbb,
	0x76, 0x67, 0x78, 0x89, 0x90, 0x01, 0xaa, 0xbb,
	0x86, 0x67, 0x78, 0x89, 0x90, 0x01, 0xaa, 0xbb,
	0xaa, 0x67, 0x78, 0x89, 0x90, 0x01, 0xaa, 0xbb,
	0xbb, 0x67, 0x78, 0x89, 0x90, 0x01, 0xaa, 0xbb,
	0xcc, 0x67, 0x78, 0x89, 0x90, 0x01, 0xaa, 0xbb,
	0xdd, 0x67, 0x78, 0x89, 0x90, 0x01, 0xaa, 0xbb,

	0x56, 0x67, 0x78, 0x89, 0x90, 0x01, 0xaa, 0xbb,
	0x66, 0x67, 0x78, 0x89, 0x90, 0x01, 0xaa, 0xbb,
	0x76, 0x67, 0x78, 0x89, 0x90, 0x01, 0xaa, 0xbb,
	0x86, 0x67, 0x78, 0x89, 0x90, 0x01, 0xaa, 0xbb,
	0x56, 0x67, 0x78, 0x89, 0x90, 0x01, 0xaa, 0xbb,
	0x66, 0x67, 0x78, 0x89, 0x90, 0x01, 0xaa, 0xbb,
	0x76, 0x67, 0x78, 0x89, 0x90, 0x01, 0xaa, 0xbb,
	0x86, 0x67, 0x78, 0x89, 0x90, 0x01, 0xaa, 0xbb,

	0x56, 0x67, 0x78, 0x89, 0x90, 0x01, 0xaa, 0xbb,
	0x66, 0x67, 0x78, 0x89, 0x90, 0x01, 0xaa, 0xbb,
	0x76, 0x67, 0x78, 0x89, 0x90, 0x01, 0xaa, 0xbb,
	0x86, 0x67, 0x78, 0x89, 0x90, 0x01, 0xaa, 0xbb,
	0x56, 0x67, 0x78, 0x89, 0x90, 0x01, 0xaa, 0xbb,
	0x66, 0x67, 0x78, 0x89, 0x90, 0x01, 0xaa, 0xbb,
	0x76, 0x67, 0x78, 0x89, 0x90, 0x01, 0xaa, 0xbb,
	0x86, 0x67, 0x78, 0x89, 0x90, 0x01, 0xaa, 0xbb,

	0x56, 0x67, 0x78, 0x89, 0x90, 0x01, 0xaa, 0xbb,
	0x66, 0x67, 0x78, 0x89, 0x90, 0x01, 0xaa, 0xbb,
	0x76, 0x67, 0x78, 0x89, 0x90, 0x01, 0xaa, 0xbb,
	0x86, 0x67, 0x78, 0x89, 0x90, 0x01, 0xaa, 0xbb,
	0x56, 0x67, 0x78, 0x89, 0x90, 0x01, 0xaa, 0xbb,
	0x66, 0x67, 0x78, 0x89, 0x90, 0x01, 0xaa, 0xbb,
	0x76, 0x67, 0x78, 0x89, 0x90, 0x01, 0xaa, 0xbb,
	0x86, 0x67, 0x78, 0x89, 0x90, 0x01, 0xaa, 0xbb,
};

int flashiic(int argc,char **argv)
{
	pcitag_t tag;
	unsigned long temp;
	int ret = 0;
	unsigned char slave_addr, index_addr;
	unsigned char s_e;
	unsigned char whole_size;
	tag  = _pci_make_tag(0, 14, 0);
	mmio = (volatile char *)_pci_conf_readn(tag, 0x14, 4);
	mmio = (volatile char *)((int)mmio | 0xb0000000);
	temp = _pci_conf_readn(tag, 0x04, 4);
	_pci_conf_writen(tag, 0x04, temp | 0x07, 0x04);
	temp = _pci_conf_readn(tag, 0x04, 4);

	slave_addr = strtoul(argv[1], 0, 16);
	s_e = strtoul(argv[2], 0, 16);
	if(s_e)
		whole_size = strtoul(argv[3], 0, 16);

	ret = flash_send(slave_addr, 0, FLASH_ID_0);
	if(ret){
		return 0;
	}
//	delay(2000000);
	ret = flash_send(slave_addr, 1, FLASH_ID_1);
	if(ret){
		return 0;
	}
//	delay(2000000);
	ret = flash_send(slave_addr, 2, FLASH_ID_2);
	if(ret){
		return 0;
	}
//	delay(2000000);
	ret = flash_send(slave_addr, 3, FLASH_ID_3);
	if(ret){
		return 0;
	}
//	delay(2000000);
	ret = flash_send(slave_addr, 4, FLASH_ID_4);
	if(ret){
		return 0;
	}
//	delay(2000000);
	if(s_e){
		ret = flash_send(slave_addr, 5, FLASH_ID_S);
		if(ret){
			return 0;
		}
	}else{
		ret = flash_send(slave_addr, 5, FLASH_ID_E);
		if(ret){
			return 0;
		}

	}

	if(s_e){
//	delay(2000000);
		ret = flash_send(slave_addr, 6, whole_size);
		if(ret){
			return 0;
		}
	}

//	ret = flash_recv(slave_addr, IIC_BLOCK_SIZE + 1, &value);
//	if(ret){
//		return 0;
//	}

out :
	return 0;
}

int dataiic(int argc,char **argv)
{
	pcitag_t tag;
	unsigned long temp;
	int ret = 0;
	unsigned char slave_addr;
	unsigned char counter, data_size;
	int i;
	tag  = _pci_make_tag(0, 14, 0);
	mmio = (volatile char *)_pci_conf_readn(tag, 0x14, 4);
	mmio = (volatile char *)((int)mmio | 0xb0000000);
	temp = _pci_conf_readn(tag, 0x04, 4);
	_pci_conf_writen(tag, 0x04, temp | 0x07, 0x04);
	temp = _pci_conf_readn(tag, 0x04, 4);

	slave_addr = strtoul(argv[1], 0, 16);
	counter = strtoul(argv[2], 0, 16);
	data_size = strtoul(argv[3], 0, 16);
	
	for(i = 0; i < data_size; i++){
		ret = flash_send(slave_addr, i, iic_data[i + counter * 32]);
		if(ret)
			return 0;
	}

//	ret = flash_recv(slave_addr, 32, &value);
//	if(ret){
//		return 0;
//	}

out :
	return 0;
}

int startiic(int argc,char **argv)
{
	pcitag_t tag;
	unsigned long temp;
	int ret = 0;
	unsigned char slave_addr, index_addr;
	tag  = _pci_make_tag(0, 14, 0);
	mmio = (volatile char *)_pci_conf_readn(tag, 0x14, 4);
	mmio = (volatile char *)((int)mmio | 0xb0000000);
	temp = _pci_conf_readn(tag, 0x04, 4);
	_pci_conf_writen(tag, 0x04, temp | 0x07, 0x04);
	temp = _pci_conf_readn(tag, 0x04, 4);

	slave_addr = strtoul(argv[1], 0, 16);

	ret = flash_send(slave_addr, 0, 0x4e);
	if(ret){
		return 0;
	}
//	delay(2000000);
	ret = flash_send(slave_addr, 1, 0x4f);
	if(ret){
		return 0;
	}
//	delay(2000000);
	ret = flash_send(slave_addr, 2, 0x52);
	if(ret){
		return 0;
	}
#if	0
//	delay(2000000);
	ret = flash_send(slave_addr, 3, 0x4d);
	if(ret){
		return 0;
	}
//	delay(2000000);
	ret = flash_send(slave_addr, 4, 0x41);
	if(ret){
		return 0;
	}
//	delay(2000000);
	ret = flash_send(slave_addr, 5, 0x4c);
	if(ret){
		return 0;
	}
#endif

	printf("st7 : start.\n");
	return 0;
}

int rdadt(int ac,char **argv)
{
	int reg;
	unsigned char d8;
	unsigned short d16;

	if(ac == 1) {
		i2c_rec_s16(0x90, 0, &d16, 1); 
		printf("reg[0]= %04x\n", d16);

		i2c_rec_s(0x90, 1, &d8, 1); 
		printf("reg[1]= %02x\n", d8);

		i2c_rec_s16(0x90, 2, &d16, 1); 
		printf("reg[2]= %04x\n", d16);

		i2c_rec_s16(0x90, 3, &d16, 1); 
		printf("reg[3]= %04x\n", d16);
	}

	return 0;
}

char *pcbv_string[]={
    "PCBA=V0.1",
    "PCBA=V0.2",
    "PCBA=V0.3",
    "PCBA=V0.4",
    "PCBA=V0.5",
    "PCBA=V0.6",
    "PCBA=V0.7",
    "PCBA=V0.8",
    "PCBA=V0.9",
    "PCBA=V0.A",
    "PCBA=V0.B",
    "PCBA=V0.C",
    "PCBA=V0.D",
    "PCBA=V0.E",
    "PCBA=V0.F",
    "PCBA=V0.G",
};

int rdpcbv(int ac,char **argv)
{
    unsigned int tmp, pcb_version = 0;
    unsigned long temp_mmio;
	pcitag_t tag=_pci_make_tag(0,14,0);
    #define SM502_GPIO_DIR_LOW    (volatile unsigned int *)(temp_mmio + 0x10008)
    #define SM502_GPIO_DATA_LOW   (volatile unsigned int *)(temp_mmio + 0x10000)

    #define SM502_GPIO_DIR_HIGH   (volatile unsigned int *)(temp_mmio + 0x1000c)
    #define SM502_GPIO_DATA_HIGH  (volatile unsigned int *)(temp_mmio + 0x10004)

	_pci_conf_writen(tag, 0x14, 0x6000000, 4);
	temp_mmio = _pci_conf_readn(tag,0x14,4);
	temp_mmio =(unsigned long)temp_mmio|PTR_PAD(0xb0000000);

    //read GPIO51~GPIO48's status
    
    tmp = *SM502_GPIO_DIR_HIGH;
    *SM502_GPIO_DIR_HIGH = tmp & 0xfff0ffff;

    tmp = *SM502_GPIO_DATA_HIGH;
    pcb_version = (tmp & 0x000f0000)>>16;

    printf("GPIO(51-48)=0x%x %s\n",pcb_version,pcbv_string[pcb_version]);
    return 0;
}


static const Cmd Cmds[] =
{
	{"MyCmds"},
	{"dumpsis",	"", 0, "dump sis registers", dumpsis, 0, 99, CMD_REPEAT},
	{"rdiic",	"", 0, "read dexxon board i2c slave registers", rdiic, 0, 99, CMD_REPEAT},
	{"wriic",	"", 0, "write dexxon board i2c slave registers", wriic, 0, 99, CMD_REPEAT},
	{"flashiic",	"", 0, "flash dexxon board with sector1 data", flashiic, 0, 99, CMD_REPEAT},
	{"dataiic",	"", 0, "flash dexxon board with sector1 data", dataiic, 0, 99, CMD_REPEAT},
	{"startiic",	"", 0, "flash dexxon board with sector1 data start ", startiic, 0, 99, CMD_REPEAT},
	{"rdadt", "", 0, "read adt registers", rdadt, 0, 99, CMD_REPEAT},
	{"rdpcbv", "", 0, "read PCB version", rdpcbv, 0, 99, CMD_REPEAT},
	{0, 0}
};

#ifdef DEVBD2F_SM502
int power_button_poll(void *unused)
{
/*
	int cause;
	asm("mfc0 %0,$13":"=r"(cause));
	if(cause&(1<<10))
		tgt_poweroff();
*/
	return 0;
}
#endif

static void init_cmd __P((void)) __attribute__ ((constructor));

static void
init_cmd()
{
#ifdef DEVBD2F_SM502
	tgt_poll_register(1, power_button_poll, 0);
#endif
	cmdlist_expand(Cmds, 1);
}

