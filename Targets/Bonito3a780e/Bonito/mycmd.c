#include "time.h"
#include "include/wpce775x.h"

#define nr_printf printf
#define nr_gets gets
#define nr_strtol strtoul

//-------------------------------------------PNP------------------------------------------
// MB PnP configuration register

//#define PNP_KEY_ADDR (0xbfd00000+0x3f0)
//#define PNP_DATA_ADDR (0xbfd00000+0x3f1)

#define PNP_KEY_ADDR (BONITO_PCIIO_BASE_VA+0x3f0)
#define PNP_DATA_ADDR (BONITO_PCIIO_BASE_VA+0x3f1)

#define ACPI_STSCMD_ADDR (BONITO_PCIIO_BASE_VA + 0x66)
#define ACPI_DATA_ADDR (BONITO_PCIIO_BASE_VA + 0x62)

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
//volatile unsigned char *p=0xbfd003c4;
volatile unsigned char *p=(BONITO_PCIIO_BASE_VA + 0x3c4);
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


static int DimmRead(int type,long long addr,union commondata *mydata)
{
char i2caddr[]={(i2cslot<<1)+0xa0};
switch(type)
{
case 1:
tgt_i2cread(I2C_SINGLE,i2caddr,1,addr,&mydata->data1,1);
break;

default: return -1;break;
}
return 0;
}

static int DimmWrite(int type,long long addr,union commondata *mydata)
{
return -1;
}


static int Ics950220Read(int type,long long addr,union commondata *mydata)
{
char c;
char i2caddr[]={0xd2};
switch(type)
{
case 1:
tgt_i2cread(I2C_SMB_BLOCK,i2caddr,1,addr,&mydata->data1,1);

break;

default: return -1;break;
}
return 0;
}

static int Ics950220Write(int type,long long addr,union commondata *mydata)
{
char c;
char i2caddr[]={0xd2};
switch(type)
{
case 1:
tgt_i2cwrite(I2C_SMB_BLOCK,i2caddr,1,addr,&mydata->data1,1);

break;

default: return -1;break;
}
return 0;
return -1;
}

static int rom_ddr_reg_read(int type,long long addr,union commondata *mydata)
{
	    char *nvrambuf;
		extern char ddr3_reg_data,_start;
        nvrambuf = 0xbfc00000+((int)&ddr3_reg_data -(int)&_start)+addr;
//		printf("ddr3_reg_data=%x\nbuf=%x,ddr=%x\n",&ddr3_reg_data,nvrambuf,addr);
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
		extern char ddr3_reg_data,_start;
		struct fl_device *dev=fl_devident(0xbfc00000,0);
		int nvram_size=dev->fl_secsize;

        nvram = 0xbfc00000+((int)&ddr3_reg_data -(int)&_start);
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


#if defined(DEVBD2F_SM502)||defined(DEVBD2F_FIREWALL)


#ifndef BCD_TO_BIN
#define BCD_TO_BIN(val) ((val)=((val)&15) + ((val)>>4)*10)
#endif

#ifndef BIN_TO_BCD
#define BIN_TO_BCD(val) ((val)=(((val)/10)<<4) + (val)%10)
#endif

void tm_binary_to_bcd(struct tm *tm)
{
	BIN_TO_BCD(tm->tm_sec);	
	BIN_TO_BCD(tm->tm_min);	
	BIN_TO_BCD(tm->tm_hour);	
	tm->tm_hour = tm->tm_hour|0x80;
	BIN_TO_BCD(tm->tm_mday);	
	BIN_TO_BCD(tm->tm_mon);	
	BIN_TO_BCD(tm->tm_year);	
	BIN_TO_BCD(tm->tm_wday);	

}

/*
 *isl 12027
 * */
char gpio_i2c_settime(struct tm *tm)
{
struct 
{
char tm_sec;
char tm_min;
char tm_hour;
char tm_mday;
char tm_mon;
char tm_year;
char tm_wday;
char tm_year_hi;
}  rtcvar;
char i2caddr[]={0xde,0};

	char a ;
	word_addr = 1;
	tm->tm_mon = tm->tm_mon + 1;
	tm_binary_to_bcd(tm);

//when rtc stop,can't set it ,follow 5 lines to resolve it
	a = 2;
	tgt_i2cwrite(I2C_SINGLE,i2caddr,2,0x3f,&a,1);
	a = 6;
	tgt_i2cwrite(I2C_SINGLE,i2caddr,2,0x3f,&a,1);
	tgt_i2cwrite(I2C_SINGLE,i2caddr,2,0x30,&a,1);
	
	a = 2;
	tgt_i2cwrite(I2C_SINGLE,i2caddr,2,0x3f,&a,1);
	a = 6;
	tgt_i2cwrite(I2C_SINGLE,i2caddr,2,0x3f,&a,1);

//begin set

rtcvar.tm_sec=tm->tm_sec;
rtcvar.tm_min=tm->tm_min;
rtcvar.tm_hour=tm->tm_hour;
rtcvar.tm_mday=tm->tm_mday;
rtcvar.tm_mon=tm->tm_mon;
rtcvar.tm_wday=tm->tm_wday;
if(tm->tm_year>=0xa0)
{
	rtcvar.tm_year = tm->tm_year - 0xa0;
	rtcvar.tm_year_hi=20;

}
else
{
	rtcvar.tm_year = tm->tm_year;
	rtcvar.tm_year_hi=19;
}
	tgt_i2cwrite(I2C_BLOCK,i2caddr,2,0x30,&rtcvar,sizeof(rtcvar));
	
	return 1;

}

/*
 * sm502: rx8025
 * fire:isl12027
 */




#endif

//----------------------------------------

static int syscall_i2c_type,syscall_i2c_addrlen;
static char syscall_i2c_addr[2];

static int i2c_read_syscall(int type,long long addr,union commondata *mydata)
{
char c;
switch(type)
{
case 1:
tgt_i2cread(syscall_i2c_type,syscall_i2c_addr,syscall_i2c_addrlen,addr,&mydata->data1,1);

break;

default: return -1;break;
}
return 0;
}

static int i2c_write_syscall(int type,long long addr,union commondata *mydata)
{
char c;
switch(type)
{
case 1:
tgt_i2cwrite(syscall_i2c_type,syscall_i2c_addr,syscall_i2c_addrlen,addr,&mydata->data1,1);

break;

default: return -1;break;
}
return 0;
return -1;
}
//----------------------------------------

static int i2cs(int argc,char **argv)
{
	pcitag_t tag;
	volatile int i;
	if(argc<2) 
		return -1;

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
			 break;
		case 3:
			 syscall1=(void *)rom_ddr_reg_read;
			 syscall2=(void *)rom_ddr_reg_write;
			 if(argc==3 && !strcmp(argv[2][2],"revert"))
			 {
				 extern char ddr3_reg_data,_start;
				 extern char ddr3_reg_data1;
				 printf("revert to default ddr setting\n");
				// tgt_flashprogram(0xbfc00000+((int)&ddr3_reg_data -(int)&_start),30*8,&ddr3_reg_data1,TRUE);
			 }
			break;
		case -1:
			if(argc<4)return -1;
			syscall_i2c_type=strtoul(argv[2],0,0);
			syscall_i2c_addrlen=argc-3;
			for(i=3;i<argc;i++)syscall_i2c_addr[i-3]=strtoul(argv[i],0,0);
			 syscall1=(void*)i2c_read_syscall;
			 syscall2=(void*)i2c_write_syscall;
			break;
		default:
			 return -1;
	}

	syscall_addrtype=0;

	return 0;
}

//not implement yet.
int pcbver(void)
{
    return 0;
}

#ifdef LOONGSON3A_3A780E 
/* Write EC port */
static int cmd_wrport(int ac, char *av[])
{
	u8 port;
	u8 value = 0;

	if(ac < 3){
		if(strcmp(av[1], "-h") == 0){
			printf("Write PM and KM channel port for wpce775l.\n");
			printf("usage : %s port value\n", av[0]);
			printf(" Note : port contain 64h/60h or 66h/62h etc.\n");
			printf("  e.g.: %s 66 80 (write command 80h to port 66h)\n", av[0]);
			printf("        %s 62 5a (write index 5ah to port 62h)\n", av[0]);
			return 0;
		}else{
			printf("ERROR : Too few parameters.\n");
			return -1;
		}
	}
	

	port = strtoul(av[1], NULL, 16);
	value = strtoul(av[2], NULL, 16);
	printf("Write EC port : port 0x%x, value 0x%x\n", port, value);
	/* Write value to EC port */
	write_port(port, value);

	return 0;
} 

/* Read EC port */
static int cmd_rdport(int ac, char *av[])
{
	u8 port;
	u8 val = 0;
	
	if(ac < 2){
		printf("ERROR : Too few parameters.\n");
		return -1;
	}
	
	if(strcmp(av[1], "-h") == 0){
		printf("Read PM and KM channel port for wpce775l.\n");
		printf("usage : %s port\n", av[0]);
		printf(" Note : port contain 64h/60h, 66h/62h etc.\n");
		printf("  e.g.: %s 62 (read io port 62h)\n", av[0]);
		return 0;
	}
	
	port = strtoul(av[1], NULL, 16);

	printf("Read EC port : port 0x%x", port);

	val = read_port(port);
	printf(" value 0x%x\n", val);

	return 0;
}

static int cmd_wr775(int ac, char *av[])
{
	u8 index;
	u8 cmd;
	u8 value = 0;

	if(ac < 3){
		if(strcmp(av[1], "-h") == 0){
			printf("Write EC space for wpce775l.\n");
			printf("usage : %s cmd [index] value\n", av[0]);
			printf("  e.g.: %s 81 5A 1 (adjust brightness level to 1)\n", av[0]);
			printf("        %s 49 0/1 (close/open backlight)\n", av[0]);
			return 0;
		}else{
			printf("ERROR : Too few parameters.\n");
			return -1;
		}
	}
	
	cmd = strtoul(av[1], NULL, 16);

	if(ac == 4){
		index = strtoul(av[2], NULL, 16);
		value = strtoul(av[3], NULL, 16);
		printf("Write EC 775 : command 0x%x, index 0x%x, data 0x%x \n", cmd, index, value);
		/* Write index value to EC 775 space */
		ec_write(cmd, index, value);
	}else{
		value = strtoul(av[2], NULL, 16);
		printf("Write EC 775 : command 0x%x, data 0x%x \n", cmd, value);
		/* Write index value to EC 775 space */
		ec_wr_noindex(cmd, value);
	}

	return 0;
} 

/*
 * rd775 : read a space of EC 775.
 * usage : rd775 cmd index size (hex)
 *  e.g. : rd775 4f [0] [21] (read EC version)
 * return: value (hex, dec, char)
 * daway added 2010-01-13
 */
static int cmd_rd775(int ac, char *av[])
{
	u8 i;
	u8 cmd, index;
	u8 size = 1, val = 0;
	
	if(ac < 2){
		printf("ERROR : Too few parameters.\n");
		return -1;
	}
	
	if(strcmp(av[1], "-h") == 0){
		printf("Read EC space for wpce775l.\n");
		printf("usage : %s {-h | cmd [index] [size]}\n", av[0]);
		printf("  e.g.: %s -h      (this help)\n", av[0]);
		printf("        %s 80 5A 1 (read command 80h, index 5A, size 1 data)\n", av[0]);
		printf("        %s 46 1    (read command 46h, size 1 data)\n", av[0]);
		printf("        %s 46      (read command 46h, i.e., read device status)\n", av[0]);
		return 0;
	}
	
	cmd = strtoul(av[1], NULL, 16);

	if(ac == 4){
		index = strtoul(av[2], NULL, 16);
		if(!get_rsa(&size, av[3])){
			printf("ERROR : arguments error!\n");
			return -1;
		}
	}

	if(ac == 3){
		if(!get_rsa(&size, av[2])){
			printf("ERROR : arguments error!\n");
			return -1;
		}
	}

	if(ac == 2){
		size = 1;
	}

	printf("Read EC 775 : command 0x%x, size %d\n", cmd, size);

	for(i = 0; i < size; i++){
		/* Read EC 775 space value */
		if(ac == 4){
			val = ec_read(cmd, index);
			printf("index 0x%x, value 0x%x %d '%c'\n", index, val, val, val);
			index++;
		}else{
			val = ec_rd_noindex(cmd + i);
			printf("value 0x%x %d '%c'\n", val, val, val);
		}
	}

	return 0;
}

static int cmd_rdecver(int ac, char *av[])
{
	printf("Read EC Firmware version: %s\n", get_ecver());

	return 0;
}

static int cmd_test_sci(int ac, char *av[])
{
    u32 i = 0;
	u8 sci_num;
 
	while(1){
		delay(100000);
		while( (read_port(EC_STS_PORT) & (EC_SCI_EVT | EC_OBF)) == 0 );
		if(!ec_query_seq(CMD_QUERY)){
			sci_num = recv_ec_data();
		}
		printf("sci number (%d): 0x%x\n", ++i, sci_num);
	}

	return 0;
}


#if 1	// test update flash function, daway 2011-02-15
#include "include/flupdate.h"
static int cmd_rdstsreg(int ac, char *av[])
{
    u8 val = 0;
 
    //set_interface_cfg(0);
	Init_Flash_Command(CMD_READ_DEV_ID);

	val = Flash_read_status_register();

	printf("Read EC FLASH status register: 0x%x\n", val);

	return 0;
}

static int cmd_wrstsreg(int ac, char *av[])
{
    u8 data;

	if(ac < 2){
		printf("ERROR : Too few parameters.\n");
		return -1;
	}
	if(strcmp(av[1], "-h") == 0){
		printf("Write EC flash status register for wpce775l.\n");
        printf("usage : %s status_value\n", av[0]);
		printf("  e.g.: %s 18 (write protect all blocks option to EC flash status register.)\n", av[0]);
		return 0;
	}
 
	data = strtoul(av[1], NULL, 16);

	printf("Write EC FLASH status register: data 0x%x\n", data);
    //set_interface_cfg(0);
	Init_Flash_Command(CMD_READ_DEV_ID);
	Flash_write_status_register(data);

	return 0;
}

static int cmd_rdids(int ac, char *av[])
{
    u32 temp_val = 0, ec_ids = 0;
	u8 mf_id = 0;
	u16 dev_id = 0;

    //set_interface_cfg(0);
	Init_Flash_Command(CMD_READ_DEV_ID);
	ec_ids = Read_Flash_IDs();

	if((ec_ids & 0x000000FF) == EC_ROM_PRODUCT_ID_WINBOND){
		Init_Flash_Command(CMD_READ_JEDEC_ID);
		temp_val = Read_Flash_IDs();
		ec_ids = (ec_ids & 0x0) | ((temp_val & 0x00FF0000) >> 8) |
			((temp_val & 0x0000FF00) << 8) | ((temp_val & 0x000000FF));
		dev_id = (u16) ((ec_ids & 0x00FFFF00) >> 8);
	}else{
		dev_id = (u16) ((ec_ids & 0x0000FF00) >> 8);
	}
	//printf("Read wpce775l flash ID: 0x%x\n", ec_ids);
	mf_id = ec_ids & 0x000000FF;
	printf("Manufacture ID 0x%x, Device ID : 0x%x\n", mf_id, dev_id);

    printf("EC ROM manufacturer: ");
    switch(mf_id){
        case EC_ROM_PRODUCT_ID_SPANSION :
            printf("SPANSION.\n");
            break;
        case EC_ROM_PRODUCT_ID_MXIC :
            printf("MXIC.\n");
            break;
        case EC_ROM_PRODUCT_ID_AMIC :
            printf("AMIC.\n");
            break;
        case EC_ROM_PRODUCT_ID_EONIC :
            printf("EONIC.\n");
            break;
        case EC_ROM_PRODUCT_ID_WINBOND :
			printf("WINBOND, Device: ");
			if(dev_id == EC_ROM_PRODUCT_ID_WBW25x80A){
				printf("W25x80.");
			}
			else if(dev_id == EC_ROM_PRODUCT_ID_WBW25Q80BV){
				printf("W25Q80BV.");
			}
            printf("\n");
            break;
 
		default :
            printf("Unknown chip type.\n");
            break;
    }

	return 0;
}
         
static int cmd_wrwcb(int ac, char *av[])
{
    unsigned long wcb_addr;
	u8 data;
 
	if(ac < 3){
		if(strcmp(av[1], "-h") == 0){
			printf("Write WCB for wpce775l.\n");
			printf("usage : %s address data\n", av[0]);
			printf("  e.g.: %s bbf00002 55 (write data to WCB.)\n", av[0]);
			return 0;
		}else{
			printf("ERROR : Too few parameters.\n");
			return -1;
		}
	}

	wcb_addr = strtoul(av[1], NULL, 16);
	data = strtoul(av[2], NULL, 16);

	printf("Write EC775 WCB : address 0x%x, data 0x%x\n", wcb_addr, data);

    //set_interface_cfg(0);
	wrwcb(wcb_addr, data);

	return 0;
}

static int cmd_rdwcb(int ac, char *av[])
{
    unsigned long wcb_addr;
	u8 size = 1;
    u8 val = 0;
	int i;
 
    if(ac < 2){
		printf("ERROR : Too few parameters.\n");
		return -1;
	}
	if(strcmp(av[1], "-h") == 0){
        printf("Read WCB for wpce775l.\n");
        printf("usage : %s address [size]\n", av[0]);
		printf("  e.g.: %s bbf00000 10 (read 16 bytes data from WCB.)\n", av[0]);
		printf("  e.g.: %s bbf00000 (read 1 byte data from WCB.)\n", av[0]);
		return 0;
    }

	wcb_addr = strtoul(av[1], NULL, 16);

	if(ac == 3){
		size = strtoul(av[2], NULL, 16);
	}else{
		size = 1;
	}

	if(size > 16){
		printf("ERROR : out of size range.\n");
		return -1;
	}

	printf("Read EC775 WCB : start address 0x%x, size %d\n", wcb_addr, size);

    //set_interface_cfg(0);
	for(i = 0; i < size; i++){
		val = rdwcb(wcb_addr + i);
		printf("WCB address 0x%x, data 0x%x\n", wcb_addr + i, val);
	}

	return 0;
}


static int cmd_rdrom(int ac, char *av[])
{
    u16 i, val;
	unsigned long start_addr;
	u32 size;

    if(ac < 2){
		printf("ERROR : Too few parameters.\n");
        return -1;
    }

	if(!(strcmp(av[1], "-h")) || (!strcmp(av[1], "--help"))){
        printf("Read data from EC flash for wpce775l.\n");
        printf("usage : %s start_address size\n", av[0]);
        printf(" Note : start_address is from 0 to (1MB - 1) range.\n");
        printf("        size is up to (1MB - start_address).\n");
        printf(" e.g. : %s 0x20000 10 (Read 16 bytes data from 0x20000 in EC flash.)\n", av[0]);
		return 0;
	}


	start_addr = (strtoul(av[1], NULL, 16) | FLASH_WIN_BASE_ADDR);
	size = strtoul(av[2], NULL, 16);

	if(size > (1024 * 1024 - start_addr)){
		printf("Warnning: size out of range, reset size is 1.\n");
		size = 1;
	}

	printf("In EC ROM read data from address 0x%x, size %d:\n", start_addr, size);
	printf("Address ");
	for(i = 0; i < 15; i++){
		if(((i % 8 ) == 0) && (i != 0)){
			printf(" -");
		}
		printf(" 0%x", i);
	}
	printf(" 0%x\n%5x", i, start_addr);
    //set_interface_cfg(1);
	for(i = 0; i < size; i++){
		val = PcMemReadB(start_addr + i);
		if(((i % 8 ) == 0) && ((i % 16) != 0)){
			printf(" -");
		}
		if((val >> 4) == 0){
			printf(" 0%x", val);
		}else{
			printf(" %x", val);
		}
		if((i + 1) % 16 == 0){
			printf("\n");
			printf("%5x", (start_addr + i + 1));
		}
	}
	printf("\n");
    //set_interface_cfg(0);

	return 0;
}

static int cmd_wrrom(int ac, char *av[])
{
	unsigned long start_addr;
	u8 *data;
	u32 size;

    if(ac < 2){
		printf("ERROR : Too few parameters.\n");
        return -1;
    }

	if(!(strcmp(av[1], "-h")) || (!strcmp(av[1], "--help"))){
        printf("Write data string to EC flash for wpce775l.\n");
        printf("usage : %s start_address data_string\n", av[0]);
        printf(" e.g. : %s 0x20000 aaaaa (Write \"aaaaa\" to address 0x20000 of EC flash.)\n", av[0]);
		return 0;
    }

	start_addr = strtoul(av[1], NULL, 16);
	data = av[2];
	size = strlen(data);

	printf("Progarm data: start address 0x%x, data %s, size %d\n", start_addr, data, size);

	Flash_program_data(start_addr, data, size);

	return 0;
}
#endif // end if 1

/*
 * rdbat : read a register of battery.
 */
static int cmd_rdbat(int ac, char *av[])
{
	u8 bat_present;
	u8 bat_capacity;
	u16 bat_voltage;
	u16 bat_current;
	u16 bat_temperature;
	char current_sign;
	
	/* read battery status */
	bat_present = ec_read(CMD_READ_EC, INDEX_POWER_STATUS) & BIT_POWER_BATPRES;

	/* read battery voltage */
	bat_voltage = (u16) ec_read(CMD_READ_EC, INDEX_BATTERY_VOL_LOW);
	bat_voltage |= (((u16) ec_read(CMD_READ_EC, INDEX_BATTERY_VOL_HIGH)) << 8);
	bat_voltage = bat_voltage * 2;

	/* read battery current */
	bat_current = (u16) ec_read(CMD_READ_EC, INDEX_BATTERY_AI_LOW);
	bat_current |= (((u16) ec_read(CMD_READ_EC, INDEX_BATTERY_AI_HIGH)) << 8);
	bat_current = bat_current * 357 / 2000;
	if( (ec_read(CMD_READ_EC, INDEX_BATTERY_FLAG) & BIT_BATTERY_CURRENT_PN) != 0 ){
		current_sign = '-';
	}else{
		current_sign = ' ';
	}

	/* read battery capacity % */
	bat_capacity = ec_read(CMD_READ_EC, INDEX_BATTERY_CAPACITY);

	/* read battery temperature */
	bat_temperature = (u16) ec_read(CMD_READ_EC, INDEX_BATTERY_TEMP_LOW);
	bat_temperature |= (((u16) ec_read(CMD_READ_EC, INDEX_BATTERY_TEMP_HIGH)) << 8);
	if(bat_present){	// && bat_temperature <= 0x5D4){	/* temp <= 100 (0x5D4 = 1492(d)) */
		bat_temperature = bat_temperature / 4 - 273;
	}else{
		bat_temperature = 0;
	}

    printf("Read battery current information:\n");
	printf("Voltage %dmV, Current %c%dmA, Capacity %d%%, Temperature %d\n",
			bat_voltage, current_sign, bat_current, bat_capacity, bat_temperature);

	return 0;
}

/* 
 * rdfan : read a register of temperature IC. 
 */
static int cmd_rdfan(int ac, char *av[])
{
    u16 speed = 0;
	u8 temperature;
	char sign = ' ';

	speed = (u16)ec_read(CMD_READ_EC, INDEX_FAN_SPEED_LOW);
    speed |= (((u16)ec_read(CMD_READ_EC, INDEX_FAN_SPEED_HIGH)) << 8); 

	temperature = ec_read(CMD_READ_EC, INDEX_TEMPERATURE_VALUE);
	if(temperature & BIT_TEMPERATURE_PN){
		temperature = (temperature & TEMPERATURE_VALUE_MASK) - 128;
		sign = '-';
	}

    printf("Fan speed: %dRPM, CPU temperature: %c%d.\n", speed, sign, temperature);

	return 0;
}

#endif // end ifdef LOONGSON3A_3A780E


static const Cmd Cmds[] =
{
	{"MyCmds"},
	{"pnps",	"", 0, "select pnp ops for d1,m1 ", pnps, 0, 99, CMD_REPEAT},
	{"dumpsis",	"", 0, "dump sis registers", dumpsis, 0, 99, CMD_REPEAT},
	{"i2cs","slotno #slot 0-1 for dimm,slot 2 for ics95220,3 for ddrcfg,3 revert for revert to default ddr setting", 0, "select i2c ops for d1,m1", i2cs, 0, 99, CMD_REPEAT},

#ifdef LOONGSON3A_3A780E	
	{"wrport", "reg", NULL, "WPCE775L write port test", cmd_wrport, 2, 99, CMD_REPEAT},
	{"rdport", "reg", NULL, "WPCE775L read port test", cmd_rdport, 2, 99, CMD_REPEAT},

	{"wrwcb", "reg", NULL, "WPCE775L WCB(Write Command Buffer) write test", cmd_wrwcb, 2, 99, CMD_REPEAT},
	{"rdwcb", "reg", NULL, "WPCE775L WCB(Write Command Buffer) read test", cmd_rdwcb, 2, 99, CMD_REPEAT},
	{"wrrom", "reg", NULL, "WPCE775L write flash test", cmd_wrrom, 2, 99, CMD_REPEAT},
	{"rdrom", "reg", NULL, "WPCE775L read flash test", cmd_rdrom, 2, 99, CMD_REPEAT},
	{"rdstsreg", "", NULL, "WPCE775L EC ID read test", cmd_rdstsreg, 0, 99, CMD_REPEAT},
	{"wrstsreg", "reg", NULL, "WPCE775L read flash test", cmd_wrstsreg, 2, 99, CMD_REPEAT},

	{"tsci", "", NULL, "WPCE775L EC ID read test", cmd_test_sci, 0, 99, CMD_REPEAT},
	{"wr775", "reg", NULL, "WPCE775L Space write test", cmd_wr775, 2, 99, CMD_REPEAT},
	{"rd775", "reg", NULL, "WPCE775L Space read test", cmd_rd775, 2, 99, CMD_REPEAT},
	{"rdids", "", NULL, "WPCE775L EC ID read test", cmd_rdids, 0, 99, CMD_REPEAT},
	{"rdecver", "", NULL, "EC F/W version for LS3ANB read test", cmd_rdecver, 0, 99, CMD_REPEAT},
	{"rdbat", "", NULL, "WPCE775L battery reg read test", cmd_rdbat, 0, 99, CMD_REPEAT},
	{"rdfan", "", NULL, "WPCE775L CPU temperature and fan reg read test", cmd_rdfan, 0, 99, CMD_REPEAT},
#endif

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

