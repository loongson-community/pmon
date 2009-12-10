#include <stdio.h>
#include <termio.h>
#include <string.h>
#include <setjmp.h>
#include <sys/endian.h>
#include <ctype.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#ifdef _KERNEL
#undef _KERNEL
#include <sys/ioctl.h>
#define _KERNEL
#else
#include <sys/ioctl.h>
#endif

#include <machine/cpu.h>

#include <pmon.h>
#include <dev/pci/pcivar.h>
#include <dev/pci/pcidevs.h>
#include <flash.h>

#define rm9000_tlb_hazard(...)
#define CONFIG_PAGE_SIZE_64KB
#include "mipsregs.h"
int cmd_mycfg __P((int, char *[]));

/*
 *  Execute the 'call' command
 *  ==========================
 *
 *  The call command invokes a function in the same way as
 *  it would be when called from a program. Arguments passed
 *  can be given as values or strings. On return the value
 *  returned is printed out.
 */
#define nr_printf printf
#define nr_gets gets
#define nr_strtol strtoul
#define X16 16
#define X10 10
#define X0 0
static union commondata{
		unsigned char data1;
		unsigned short data2;
		unsigned long data4;
		unsigned long data8[2];
		unsigned char c[8];
}mydata,*pmydata;

static	pcitag_t mytag=0;

static int __pcisyscall1(int type,unsigned int addr,union commondata *mydata)
{
switch(type)
{
case 1:mydata->data1=_pci_conf_readn(mytag,addr,1);break;
case 2:mydata->data2=_pci_conf_readn(mytag,addr,2);break;
case 4:mydata->data4=_pci_conf_readn(mytag,addr,4);break;
case 8:mydata->data8[0]=_pci_conf_readn(mytag,addr,4);
	   mydata->data8[1]=_pci_conf_readn(mytag,addr+4,4);break;
}
return 0;
}

static int __pcisyscall2(int type,unsigned int addr,union commondata *mydata)
{
switch(type)
{
case 1: _pci_conf_writen(mytag,addr,mydata->data1,1);return 0;
case 2: _pci_conf_writen(mytag,addr,mydata->data2,2);return 0;
case 4: _pci_conf_writen(mytag,addr,mydata->data4,4);return 0;
case 8:break;
}
return -1;
}

static int __syscall1(int type,unsigned int addr,union commondata *mydata)
{
switch(type)
{
case 1:mydata->data1=*(volatile char *)addr;break;
case 2:mydata->data2=*(volatile short *)addr;break;
case 4:mydata->data4=*(volatile long *)addr;break;
case 8:mydata->data8[0]=*(volatile long *)addr;mydata->data8[1]=*(volatile long *)(addr+4);break;
}
return 0;
}

static int __syscall2(int type,unsigned int addr,union commondata *mydata)
{
switch(type)
{
case 1:*(volatile char *)addr=mydata->data1;break;
case 2:*(volatile short *)addr=mydata->data2;break;
case 4:*(volatile long *)addr=mydata->data4;break;
case 8:*(volatile long *)addr=mydata->data8[0];*(volatile long *)(addr+4)=mydata->data8[1];break;
}
return 0;
}

static int (*syscall1)(int type,unsigned addr,union commondata *mydata)=&__pcisyscall1;
static int (*syscall2)(int type,unsigned addr,union commondata *mydata)=&__pcisyscall2;

static int mydump(char type,unsigned addr,unsigned count)
{
		int i,j,k;
		char memdata[16];
		for(j=0;j<count;j=j+16/type,addr=addr+16)
		{
		nr_printf("%08x: ",addr);

		pmydata=(void *)memdata;
		for(i=0;type*i<16;i++)
		{
		if(syscall1(type,addr+i*type,pmydata)<0){nr_printf("read address %p error\n",addr+i*type);return -1;}
		pmydata=(void *)((char *)pmydata+type);
		if(j+i+1>=count)break;
		}
		
		pmydata=(void *)memdata;
		for(i=0;type*i<16;i++)
		{
		switch(type)
		{
		case 1:	nr_printf("%02x ",pmydata->data1);break;
		case 2: nr_printf("%04x ",pmydata->data2);break;
		case 4: nr_printf("%08x ",pmydata->data4);break;
		case 8: nr_printf("%08x%08x ",pmydata->data8[1],pmydata->data8[0]);break;
		}
		if(j+i+1>=count){int k;for(i=i+1;type*i<16;i++){for(k=0;k<type;k++)nr_printf("  ");nr_printf(" ");}break;}
		pmydata=(void *)((char *)pmydata+type);
		}
		
		pmydata=(void *)memdata;
		#define CPMYDATA ((char *)pmydata)
		for(k=0;k<16;k++)
		{
		nr_printf("%c",(CPMYDATA[k]<0x20 || CPMYDATA[k]>0x7e)?'.':CPMYDATA[k]);
		if(j+(k+1)/type>=count)break;
		}
		nr_printf("\n");
		}
		return 0;
}

int mypcs(int ac,char **av)
{
	int bus;
	int dev;
	int func;

if(ac==4)
{
	bus=nr_strtol(av[1],0,0);
	dev=nr_strtol(av[2],0,0);
	func=nr_strtol(av[3],0,0);
	mytag=_pci_make_tag(bus,dev,func);
	syscall1=__pcisyscall1;
	syscall2=__pcisyscall2;
}
else if((ac==2) && (nr_strtol(av[1],0,0)==-1))
{
	syscall1=__syscall1;
	syscall2=__syscall2;
	mytag=-1;
}

if(mytag!=-1)
{
	_pci_break_tag(mytag,&bus,&dev,&func);
	printf("pci select bus=%d,dev=%d,func=%d\n",bus,dev,func);
}
else printf("select normal memory access\n");

	return (0);
}

static unsigned lastaddr=0;
static int dump(int argc,char **argv)
{
		char type=4;
static	unsigned int addr,count=1;
//		char opts[]="bhwd";
		if(argc>3){nr_printf("d{b/h/w/d} adress count\n");return -1;}

		switch(argv[0][1])
		{
				case '1':	type=1;break;
				case '2':	type=2;break;
				case '4':	type=4;break;
				case '8':	type=8;break;
		}

		if(argc>1)addr=nr_strtol(argv[1],0,X0);
		else addr=lastaddr;
		if(argc>2)count=nr_strtol(argv[2],0,X0);
		else if(count<=0||count>=1024) count=1;
		mydump(type,addr,count);
		lastaddr=addr+count*type;
		return 0;
}

static int getdata(char *str)
{
	static char buf[17];
	char *pstr;
	int sign=1;
	int radix=10;
	pstr=strtok(str," \t\x0a\x0d");

		if(pstr)
		{
		if(pstr[0]=='q')return -1;
		memset(buf,'0',16); buf[17]=0;
		if(pstr[0]=='-')
		{
		sign=-1;
		pstr++;
		}
		else if(pstr[0]=='+')
		{
			pstr++;
		}

		if(pstr[0]!='0'){radix=10;}
		else if(pstr[1]=='x'){radix=16;pstr=pstr+2;}
		else {radix=8;pstr++;}
			
		memcpy(buf+16-strlen(pstr),pstr,strlen(pstr));
		pstr=buf;
		pstr[16]=pstr[8];pstr[8]=0;
		mydata.data8[1]=nr_strtol(pstr,0,radix);
		pstr[8]=pstr[16];pstr[16]=0;
		mydata.data8[0]=nr_strtol(&pstr[8],0,radix);
		if(sign==-1)
		{
		long x=mydata.data8[0];
			mydata.data8[0]=-mydata.data8[0];
			if(x<0)
			mydata.data8[1]=-mydata.data8[1];
			else mydata.data8[1]=~mydata.data8[1];
			
		}
		return 1;
		}
		return 0;

}

static int modify(int argc,char **argv)
{
		char type=4;
		unsigned addr;
//		char opts[]="bhwd";
		char str[100];
		int i;

		if(argc<2){nr_printf("m{b/h/w/d} adress [data]\n");return -1;}

		switch(argv[0][1])
		{
				case '1':	type=1;break;
				case '2':	type=2;break;
				case '4':	type=4;break;
				case '8':	type=8;break;
		}
		addr=nr_strtol(argv[1],0,X0);
		if(argc>2)
		{
		 i=2;
	          while(i<argc)
		 {
	       	   getdata(argv[i]);
		   if(syscall2(type,addr,&mydata)<0)
		   {nr_printf("write address %p error\n",addr);return -1;};
		   addr=addr+type;
		 i++;
		 }
		  return 0;
		}


		while(1)
		{
		if(syscall1(type,addr,&mydata)<0){nr_printf("read address %p error\n",addr);return -1;};
		nr_printf("%08x:",addr);
		switch(type)
		{
		case 1:	nr_printf("%02x ",mydata.data1);break;
		case 2: nr_printf("%04x ",mydata.data2);break;
		case 4: nr_printf("%08x ",mydata.data4);break;
		case 8: nr_printf("%08x%08x ",mydata.data8[1],mydata.data8[0]);break;
		}
		memset(str,0,100);
		nr_gets(str);
	        i=getdata(str);
		if(i<0)break;	
		else if(i>0) 
		{
		if(syscall2(type,addr,&mydata)<0)
		{nr_printf("write address %p error\n",addr);return -1;};
		}
	addr=addr+type;
		}	
		lastaddr=addr;	
		return 0;
}
//-------------------------------------------PNP------------------------------------------
// MB PnP configuration register

#define PNP_KEY_ADDR (PCI_IO_SPACE_BASE+0x3f0)
#define PNP_DATA_ADDR (PCI_IO_SPACE_BASE+0x3f1)


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


static int PnpRead(int argc,char **argv)
{
	unsigned char Index,data;
		if(argc!=2){return -1;}
		
		Index=nr_strtol(argv[1],0,X0);
data=PNPGetConfig(Index);
nr_printf("pnpread index=0x%02x,value=0x%02x\n",Index,data);
return 0;
}

static int PnpWrite(int argc,char **argv)
{
        unsigned char Index,data;
        if(argc!=4){nr_printf("pnpw Index data\n");return -1;}
		Index=nr_strtol(argv[1],0,X0);
		data=nr_strtol(argv[2],0,X0);
PNPSetConfig(Index,data);
nr_printf("pnpwrite index=0x%02x,value=0x%02x,",Index,data);
data=PNPGetConfig(Index);
nr_printf("result=0x%02x\n",data);
return 0;
}

extern int novga;
static int setvga(int argc,char **argv)
{
	if(argc>1)
	{
	if(argv[1][0]=='1')vga_available=1;
	else vga_available=0;
	novga=!vga_available;
	}
	printf("vga_available=%d\n",vga_available);
	return 0;
}

static int writefb(int argc,char **argv)
{
	int i;
	unsigned long base,value,size = 800*600;
	if(argc>1)
	{
		base = strtoul(argv[1],0,0);
		value= strtoul(argv[2],0,0);
		if (argc>3) size = strtoul(argv[3],0,0);

		printf("base=%x,value=%x,size=%x\n",base,value,size);

		for (i=0;i<size;i++) {
			*(volatile short *)(base + 2*i) = value;
		}
	}
	return 0;
}

static int setkbd(int argc,char **argv)
{
//#if NMOD_VGACON > 0
	if(argc>1)
	{
	if(argv[1][0]=='1')kbd_available=1;
	else kbd_available=0;
	}
	printf("kbd_available=%d\n",kbd_available);
//#endif
	return 0;
}

//-------------------------------------------------------------------------------------------

#define ASID_INC	0x1
#define ASID_MASK	0xff

/*
 * PAGE_SHIFT determines the page size
 */
#ifdef CONFIG_PAGE_SIZE_4KB
#define PAGE_SHIFT	12
#endif
#ifdef CONFIG_PAGE_SIZE_16KB
#define PAGE_SHIFT	14
#endif
#ifdef CONFIG_PAGE_SIZE_64KB
#define PAGE_SHIFT	16
#endif
#define PAGE_SIZE	(1UL << PAGE_SHIFT)
#define PAGE_MASK	(~(PAGE_SIZE-1))


static int tlbset(int argc,char **argv)
{
	int idx, pid;
	unsigned int phyaddr,viraddr;
	int eflag=0;
	
	if(argc==4)
	{
		if(!strcmp("-x",argv[3]))eflag=1;
		else return -1;
	}
	else if(argc!=3)return -1;
	
	viraddr=strtoul(argv[1],0,0);
	phyaddr=strtoul(argv[2],0,0);
	write_c0_pagemask(PM_DEFAULT_MASK);


	pid = read_c0_entryhi() & ASID_MASK;
	

	viraddr &= (PAGE_MASK << 1);
	write_c0_entryhi(viraddr | (pid));
	tlb_probe();
	idx = read_c0_index();
	printf("viraddr=%08x,phyaddr=%08x,pid=%x,idx=%x\n",viraddr,phyaddr,pid,idx);
	write_c0_entrylo0((phyaddr >> 6)|0x1f);
	write_c0_entrylo1(((phyaddr+PAGE_SIZE) >> 6)|0x1f);
	write_c0_entryhi(viraddr | (pid));

	if(idx < 0) {
		tlb_write_random();
	} else {
		tlb_write_indexed();
	}
	write_c0_entryhi(pid);
    if(eflag)    __asm__ __volatile__ ("mtc0 %0,$22;"::"r"(0x4));
}

#if NMOD_VGACON > 0
extern int kbd_initialize(void);
#endif
static int initkbd(int argc,char **argv)
{
#if NMOD_VGACON > 0
 return kbd_initialize();
#endif
}

static int setcache(int argc,char **argv)
{
	if(argc==2)
	{
		if(argv[1][0]=='1')
		{
		cacheflush();
	    __asm__ volatile(
		".set mips3;\r\n"
        "mfc0   $4,$16;\r\n"
        "and    $4,$4,0xfffffff8;\r\n"
        "or     $4,$4,0x3;\r\n"
        "mtc0   $4,$16;\r\n"
		".set mips0;"
		::
		:"$4"
		);
		}
		else
		{
		cacheflush();
		__asm__ volatile(
		 ".set mips3;\r\n"
		 "mfc0   $4,$16;\r\n"
        "and    $4,$4,0xfffffff8;\r\n"
        "or     $4,$4,0x2;\r\n"
        "mtc0   $4,$16;\r\n"
		".set mips0;\r\n"
		::
		: "$4"
		);
		}
	}else{
		unsigned long a;
		__asm__ volatile(
		 ".set mips3;\r\n"
		 "mfc0  %0,$16;\r\n"
		 ".set mips0;\r\n"
		 :"=r"(a)
		 );
		printf("CONFIG is %x\n",a);
	}
return 0;
}
#if NMOD_FLASH_SST
static int erase(int argc,char **argv)
{
struct fl_map *map;
int offset;
if(argc!=2)return -1;
map=tgt_flashmap();
offset=strtoul(argv[1],0,0);
fl_erase_sector_sst(map,0,offset);
}

static int program(int argc,char **argv)
{
struct fl_map *map;
int offset;
char i;
if(argc!=3)return -1;
map=tgt_flashmap();
offset=strtoul(argv[1],0,0);
for(i=0;i<strlen(argv[1]);i++)
fl_program_sst(map,0,offset,&argv[1][i]);
	
}
#endif
       
//-------------------------------------------------------------------------------------------
static const Cmd Cmds[] =
{
	{"MyCmds"},
	{"pnpr",	"LDN index", 0, "pnpr LDN(logic device NO) index", PnpRead, 0, 99, CMD_REPEAT},
	{"pnpw",	"LDN index value", 0, "pnpw LDN(logic device NO) index value", PnpWrite, 0, 99, CMD_REPEAT},
	{"pcs",	"bus dev func", 0, "select pci dev function", mypcs, 0, 99, CMD_REPEAT},
	{"d1",	"[addr] [count]", 0, "dump address byte", dump, 0, 99, CMD_REPEAT},
	{"d2",	"[addr] [count]", 0, "dump address half world", dump, 0, 99, CMD_REPEAT},
	{"d4",	"[addr] [count]", 0, "dump address world", dump, 0, 99, CMD_REPEAT},
	{"d8",	"[addr] [count]", 0, "dump address double word", dump, 0, 99, CMD_REPEAT},
	{"m1",	"addr [data]", 0, "modify address byte", modify, 0, 99, CMD_REPEAT},
	{"m2",	"addr [data]", 0, "mofify address half world", modify, 0, 99, CMD_REPEAT},
	{"m4",	"addr [data]", 0, "modify address world", modify, 0, 99, CMD_REPEAT},
	{"m8",	"addr [data]", 0, "modify address double word",modify, 0, 99, CMD_REPEAT},
	{"setvga","[0|1]",0,"set vga_available",setvga,0,99,CMD_REPEAT},
	{"writefb","base value size",0,"write fb",writefb,0,99,CMD_REPEAT},
	{"setkbd","[0|1]",0,"set kbd_available",setkbd,0,99,CMD_REPEAT},
	{"initkbd","",0,"kbd_initialize",initkbd,0,99,CMD_REPEAT},
	{"tlbset","viraddr phyaddr [-x]",0,"tlbset viraddr phyaddr [-x]",tlbset,0,99,CMD_REPEAT},
	{"cache","[0 1]",0,"cache [0 1]",setcache,0,99,CMD_REPEAT},

#if NMOD_FLASH_SST
	{"erase","[0 1]",0,"cache [0 1]",erase,0,99,CMD_REPEAT},
	{"program","[0 1]",0,"cache [0 1]",program,0,99,CMD_REPEAT},
#endif
	{0, 0}
};


static void init_cmd __P((void)) __attribute__ ((constructor));

static void
init_cmd()
{
	cmdlist_expand(Cmds, 1);
}


