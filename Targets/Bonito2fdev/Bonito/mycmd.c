#define nr_printf printf
#define nr_gets gets
#define nr_strtol strtoul
//-------------------------------------------PNP------------------------------------------
// MB PnP configuration register

#define PNP_KEY_ADDR (0xbfd00000+0x3f0)
#define PNP_DATA_ADDR (0xbfd00000+0x3f1)


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
extern unsigned int syscall_addrtype;
extern int (*syscall1)(int type,long long addr,union commondata *mydata);
extern int (*syscall2)(int type,long long addr,union commondata *mydata);

static int PnpRead(int type,long long addr,union commondata *mydata)
{
switch(type)
{
case 1:mydata->data1=PNPGetConfig(addr);break;
default: return -1;break;
}
return 0;
}

static int PnpWrite(int type,long long addr,union commondata *mydata)
{
switch(type)
{
case 1:PNPSetConfig(addr,mydata->data1);break;
default: return -1;break;
}
return 0;
}

#if PCI_IDSEL_CS5536 != 0

static int logicdev=0;
static int PnpRead_w83627(int type,long long addr,union commondata *mydata)
{
switch(type)
{
case 1:
mydata->data1=w83627_read(logicdev,addr);
break;
default: return -1;break;
}
return 0;
}

static int PnpWrite_w83627(int type,long long addr,union commondata *mydata)
{
switch(type)
{
case 1:
w83627_write(logicdev,addr,mydata->data1);
break;
default: return -1;break;
}
return 0;
}

#endif


static int pnps(int argc,char **argv)
{
#if PCI_IDSEL_CS5536 != 0
logicdev=strtoul(argv[1],0,0);
syscall1=(void*)PnpRead_w83627;
syscall2=(void*)PnpWrite_w83627;
#else
syscall1=(void*)PnpRead;
syscall2=(void*)PnpWrite;
#endif
syscall_addrtype=0;
return 0;
}

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

static int Ics950220Write(int type,long long addr,union commondata *mydata)
{
char c;
switch(type)
{
case 1:
linux_outb(0xd2,SMBUS_HOST_ADDRESS); //0xd3
linux_outb(addr,SMBUS_HOST_COMMAND);
linux_outb(1,SMBUS_HOST_DATA0);
linux_outb(0x14,SMBUS_HOST_CONTROL); //0x14
if((c=linux_inb(SMBUS_HOST_STATUS))&0x1f)
{
linux_outb(c,SMBUS_HOST_STATUS);
}

c=linux_inb(SMBUS_HOST_CONTROL);
linux_outb(mydata->data1,SMBUS_HOST_DATA1+1);
linux_outb(c|0x40,SMBUS_HOST_CONTROL);

while(linux_inb(SMBUS_HOST_STATUS)&SMBUS_HOST_STATUS_BUSY);

if((c=linux_inb(SMBUS_HOST_STATUS))&0x1f)
{
linux_outb(c,SMBUS_HOST_STATUS);
}

break;

default: return -1;break;
}
return 0;
return -1;
}

static int rom_ddr_reg_read(int type,long long addr,union commondata *mydata)
{
	    char *nvrambuf;
		extern char ddr2_reg_data,_start;
        nvrambuf = 0xbfc00000+((int)&ddr2_reg_data -(int)&_start)+addr;
//		printf("ddr2_reg_data=%x\nbuf=%x,ddr=%x\n",&ddr2_reg_data,nvrambuf,addr);
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
static volatile char *mmio = 0;
#define SM502_I2C_COUNT 0x010040
#define SM502_I2C_CTRL 0x010041
#define SM502_I2C_STAT 0x010042
#define SM502_I2C_ADDR 0x010043      //0:write,1:read
#define SM502_I2C_DATA 0x010044      //44-53
#define GPIO_DIR_HIGH 		(volatile unsigned int *)(mmio + 0x1000c)
#define GPIO_DATA_HIGH		(volatile unsigned int *)(mmio + 0x10004)
#define G_OUTPUT		1
#define G_INPUT			0


static void i2c_sleep(int ntime)
{
	int i,j=0;
	for(i=0; i<300*ntime; i++)
	{
		j=i;
		j+=i;
	}
}

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

static char i2c_rec_ack()
{
        char res = 1;
        int num=10;
	int tmp;
        sda_dir(G_INPUT);
        i2c_sleep(3);
        scl_bit(1);
        i2c_sleep(5);
	tmp = ((*GPIO_DATA_HIGH)&0x8000);

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
		tmp = ((*GPIO_DATA_HIGH)&0x8000);
        }
        scl_bit(0);
        i2c_sleep(3);
        return res;
}


static unsigned char i2c_rec()
{
	int i;
	int tmp;
	unsigned char or_char;
	unsigned char value = 0x00;
	sda_dir(G_INPUT);
	for(i=7;i>=0;i--)
	{
		i2c_sleep(5);
		scl_bit(1);
		i2c_sleep(3);
		tmp = ((*GPIO_DATA_HIGH)&0x8000);

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
		i2c_sleep(5);
		scl_bit(0);
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
		if(!i2c_rec_ack())
			return 0;
		i2c_send(sub_addr);
		if(!i2c_rec_ack())
			return 0;
		i2c_send(*buf);
		if(!i2c_rec_ack())
			return 0;
		i2c_stop();
		//deal the data
		buf++;
		count--;
		sub_addr++;
	}
	return 1;
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
char c;
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
#endif
static int i2cs(int argc,char **argv)
{
	pcitag_t tag;
	volatile int tmp;
if(argc!=2) return -1;

i2cslot=strtoul(argv[1],0,0);

switch(i2cslot)
{
case 0:
case 1:
 syscall1=(void*)DimmRead;
 syscall2=(void*)DimmWrite;
break;
case 2:
 syscall1=(void*)Ics950220Read;
 syscall2=(void*)Ics950220Write;
case 3:
 syscall1=(void *)rom_ddr_reg_read;
 syscall2=(void *)rom_ddr_reg_write;
break;
#ifdef DEVBD2F_SM502
case 4:

	tag=_pci_make_tag(0,14,0);

	mmio=_pci_conf_readn(tag,0x14,4);
	mmio =(int)mmio|(0xb0000000);
//	printf("mmio =%x\n",mmio);	
	
	syscall1 = (void *)sm502RtcRead;
 	syscall2 = (void *)sm502RtcWrite;
	tmp = *(volatile int *)(mmio + 0x40);
	*(volatile int *)(mmio + 0x40) =tmp|0x40;
 break;
#endif
default:
 return -1;
break;
}

syscall_addrtype=0;

return 0;
}


static const Cmd Cmds[] =
{
	{"MyCmds"},
	{"pnps",	"", 0, "select pnp ops for d1,m1 ", pnps, 0, 99, CMD_REPEAT},
	{"dumpsis",	"", 0, "dump sis registers", dumpsis, 0, 99, CMD_REPEAT},
	{"i2cs","slotno #slot 0-1 for dimm,slot 2 for ics95220,3 for ddrcfg", 0, "select i2c ops for d1,m1", i2cs, 0, 99, CMD_REPEAT},
	{0, 0}
};

#ifdef DEVBD2F_SM502
int power_button_poll(void *unused)
{
	int cause;
	volatile int *p=0xbfe0011c;
	asm("mfc0 %0,$13":"=r"(cause));
	if(cause&(1<<10))tgt_poweroff();
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

