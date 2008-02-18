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
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/net/if.h>
void route_init();

#define rm9000_tlb_hazard(...)
#define CONFIG_PAGE_SIZE_64KB
#include "mipsregs.h"
int cmd_mycfg __P((int, char *[]));
extern char  *heaptop;

#include <errno.h>
#define ULONGLONG_MAX 0xffffffffffffffffUL
unsigned long long
strtoull(const char *nptr,char **endptr,int base)
{
    int c;
    unsigned long long  result = 0L;
    unsigned long long limit;
    int negative = 0;
    int overflow = 0;
    int digit;

    while ((c = *nptr) && isspace(c)) /* skip leading white space */
      nptr++;

    if ((c = *nptr) == '+' || c == '-') { /* handle signs */
	negative = (c == '-');
	nptr++;
    }

    if (base == 0) {		/* determine base if unknown */
	base = 10;
	if (*nptr == '0') {
	    base = 8;
	    nptr++;
	    if ((c = *nptr) == 'x' || c == 'X') {
		base = 16;
		nptr++;
	    }
	}
    } else if (base == 16 && *nptr == '0') {	
	/* discard 0x/0X prefix if hex */
	nptr++;
	if ((c = *nptr == 'x') || c == 'X')
	  nptr++;
    }

    limit = ULONGLONG_MAX / base;	/* ensure no overflow */

    nptr--;			/* convert the number */
    while ((c = *++nptr) != 0) {
	if (isdigit(c))
	  digit = c - '0';
	else if(isalpha(c))
	  digit = c - (isupper(c) ? 'A' : 'a') + 10;
	else
	  break;
	if (digit < 0 || digit >= base)
	  break;
	if (result > limit)
	  overflow = 1;
	if (!overflow) {
	    result = base * result;
	    if (digit > ULONGLONG_MAX - result)
	      overflow = 1;
	    else	
	      result += digit;
	}
    }
    if (negative && !overflow)	/* BIZARRE, but ANSI says we should do this! */
      result = 0L - result;
    if (overflow) {
	extern int errno;
	errno = ERANGE;
	result = ULONGLONG_MAX;
    }

    if (endptr != NULL)		/* point at tail */
      *endptr = (char *)nptr;
    return result;
}

static union commondata{
		unsigned char data1;
		unsigned short data2;
		unsigned int data4;
		unsigned int data8[2];
		unsigned char c[8];
}mydata,*pmydata;

unsigned int syscall_addrtype=0;
static int __syscall1(int type,long long addr,union commondata *mydata);
static int __syscall2(int type,long long addr,union commondata *mydata);
int (*syscall1)(int type,long long addr,union commondata *mydata)=(void *)&__syscall1;
int (*syscall2)(int type,long long addr,union commondata *mydata)=(void *)&__syscall2;
static char *str2addmsg[]={"32 bit cpu address","64 bit cpu address","64 bit cached phyiscal address","64 bit uncached phyiscal address"};
static unsigned long long
str2addr(const char *nptr,char **endptr,int base)
{
unsigned long long result;
if(syscall_addrtype%4==0)
{
result=(long)strtoul(nptr,endptr,base);
}
else
{
result=strtoull(nptr,endptr,base);
if(syscall_addrtype%4==3)result|=0x9000000000000000UL;
else if(syscall_addrtype%4==2)result|=0x9800000000000000UL;
}
return result;
}
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
#define nr_strtoll strtoull
#define nr_str2addr str2addr

static	pcitag_t mytag=0;

static char diskname[0x40];

static int __disksyscall1(int type,unsigned int addr,union commondata *mydata)
{
	char fname[0x40];
	FILE *fp;
	if(strncmp(diskname,"/dev/",5)) sprintf(fname,"/dev/disk/%s",diskname);
	else strcpy(fname,diskname);
	fp=fopen(fname,"r+");
	if(!fp){printf("open %s error!\n",fname);return -1;}
	fseek(fp,addr,SEEK_SET);
switch(type)
{
case 1:fread(&mydata->data1,1,1,fp);break;
case 2:fread(&mydata->data2,2,1,fp);break;
case 4:fread(&mydata->data4,4,1,fp);break;
case 8:fread(&mydata->data8,8,1,fp);break;
}
	fclose(fp);
return 0;
}

static int __disksyscall2(int type,unsigned int addr,union commondata *mydata)
{
	char fname[0x40];
	FILE *fp;
	sprintf(fname,"/dev/disk/%s",diskname);
	fp=fopen(fname,"r+");
	if(!fp){printf("open %s error!\n",fname);return -1;}
	fseek(fp,addr,SEEK_SET);
switch(type)
{
case 1:fwrite(&mydata->data1,1,1,fp);break;
case 2:fwrite(&mydata->data2,2,1,fp);break;
case 4:fwrite(&mydata->data4,4,1,fp);break;
case 8:fwrite(&mydata->data8,8,1,fp);break;
}
	fclose(fp);
return 0;
}

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

static int __syscall1(int type,long long addr,union commondata *mydata)
{
switch(type)
{
case 1:
	  //mydata->data1=*(volatile char *)addr;break;
	  asm("lbu $2,(%1);
		   sb $2,(%0)
		   "
		  ::"r"(&mydata->data1),"r"(addr)
		  :"$2"
		 );
	   break;
case 2:
	  //mydata->data2=*(volatile short *)addr;break;
	  asm("lhu $2,(%1);
		   sh $2,(%0)
		   "
		  ::"r"(&mydata->data2),"r"(addr)
		  :"$2"
		 );
	   break;
case 4:
	  //mydata->data4=*(volatile int *)addr;break;
	  asm("lwu $2,(%1);
		   sw $2,(%0)
		   "
		  ::"r"(&mydata->data4),"r"(addr)
		  :"$2"
		 );
	   break;
case 8:
	  // mydata->data8[0]=*(volatile int *)addr;mydata->data8[1]=*(volatile int *)(addr+4);
	  //*(long long *)mydata->data8=*(volatile long long *)addr;
	  asm("ld $2,(%1);
		   sd $2,(%0)
		   "
		  ::"r"(mydata->data8),"r"(addr)
		  :"$2"
		 );
	   break;
}
return 0;
}

static int __syscall2(int type,long long addr,union commondata *mydata)
{
switch(type)
{
case 1:
	 //*(volatile char *)addr=mydata->data1;break;
	  asm("lbu $2,(%0);
		   sb $2,(%1)
		   "
		  ::"r"(&mydata->data1),"r"(addr)
		  :"$2"
		 );
	   break;
case 2:
	   //*(volatile short *)addr=mydata->data2;break;
	  asm("lhu $2,(%0);
		   sh $2,(%1)
		   "
		  ::"r"(&mydata->data2),"r"(addr)
		  :"$2"
		 );
	  break;
case 4:
	  //*(volatile int *)addr=mydata->data4;break;
	  asm("lwu $2,(%0);
		   sw $2,(%1)
		   "
		  ::"r"(&mydata->data4),"r"(addr)
		  :"$2"
		 );
	    break;
case 8:
	  asm("ld $2,(%0);
		   sd $2,(%1)
		   "
		  ::"r"(mydata->data8),"r"(addr)
		  :"$2"
		 );
	   //*(volatile int *)addr=mydata->data8[0];*(volatile int *)(addr+4)=mydata->data8[1];
	   //*(volatile long long *)addr=*(volatile long long *)mydata->data8;
	   break;
}
return 0;
}


static int mydump(char type,unsigned long long addr,unsigned count)
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
    int tmp;

if(ac==4)
{
	bus=nr_strtol(av[1],0,0);
	dev=nr_strtol(av[2],0,0);
	func=nr_strtol(av[3],0,0);
	mytag=_pci_make_tag(bus,dev,func);
	syscall1=(void *)__pcisyscall1;
	syscall2=(void *)__pcisyscall2;
}
else if(ac==2)
{
	syscall_addrtype=nr_strtol(av[1],0,0);
	syscall1=__syscall1;
	syscall2=__syscall2;
	mytag=-1;
   printf("select normal memory access (%s)\n",str2addmsg[syscall_addrtype%4]);
}
else if(ac==1)
{
int i;
for(i=0;i>-4;i--)
printf("pcs %d : select select normal memory access %s\n",i,str2addmsg[(unsigned)i%4]);
printf("pcs bus dev func : select pci configuration space access with bus dev func\n");
if(mytag!=-1)
{
	_pci_break_tag(mytag,&bus,&dev,&func);
	printf("pci select bus=%d,dev=%d,func=%d\n",bus,dev,func);
}
}

	return (0);
}

int mydisks(int ac,char **av)
{
if(ac>2)return -1;
if(ac==2)
{
if(nr_strtol(av[1],0,0)==-1)
{
	syscall1=__syscall1;
	syscall2=__syscall2;
	diskname[0]=0;
}
else{ strncpy(diskname,av[1],0x40);diskname[0x3f]=0;}
}

if(diskname[0])
{
	syscall1=(void *)__disksyscall1;
	syscall2=(void *)__disksyscall2;
	printf("disk select %s\n",diskname);
}
else printf("select normal memory access\n");

	return (0);
}

static unsigned long long lastaddr=0;
static int dump(int argc,char **argv)
{
		char type=4;
unsigned long long  addr;
static int count=1;
//		char opts[]="bhwd";
		if(argc>3){nr_printf("d{b/h/w/d} adress count\n");return -1;}

		switch(argv[0][1])
		{
				case '1':	type=1;break;
				case '2':	type=2;break;
				case '4':	type=4;break;
				case '8':	type=8;break;
		}

		if(argc>1)addr=nr_str2addr(argv[1],0,0);
		else addr=lastaddr;
		if(argc>2)count=nr_strtol(argv[2],0,0);
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
		unsigned long long addr;
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
		addr=nr_str2addr(argv[1],0,0);
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
//----------------------------------------------------------------------
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

static int setkbd(int argc,char **argv)
{
	if(argc>1)
	{
	if(argv[1][0]=='1')kbd_available=1;
	else if(argv[1][0]=='2')usb_kbd_available=1;
	else {kbd_available=0;usb_kbd_available=0;}
	}
	printf("kbd_available=%d,usb_kbd_available=%d\n",kbd_available,usb_kbd_available);
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
return 0;
}

static int tlbtest(int argc,char **argv)
{
	int idx, pid;
	unsigned int phyaddr,viraddr;
	int i;
	int eflag=0;
	unsigned int lo0,lo1,hi;
	
	
	if(argc!=3)return -1;

	viraddr=strtoul(argv[1],0,0);
	phyaddr=strtoul(argv[2],0,0);

for(i=0;i<64;i++)
{

	write_c0_pagemask(PM_DEFAULT_MASK);
	pid = read_c0_entryhi() & ASID_MASK;
	viraddr &= (PAGE_MASK << 1);
	write_c0_entryhi(viraddr | (pid));
	idx = i;
	write_c0_index(i);
	write_c0_entrylo0((phyaddr >> 6)|0x1f);
	write_c0_entrylo1(((phyaddr+PAGE_SIZE) >> 6)|0x1f);
	write_c0_entryhi(viraddr | (pid));
	if(idx < 0) {
		tlb_write_random();
	} else {
		tlb_write_indexed();
	}
	write_c0_entryhi(pid);
	tlb_read();

	lo0=read_c0_entrylo0();
	lo1=read_c0_entrylo1();
	hi=read_c0_entryhi();
	if(lo0!=((phyaddr >> 6)|0x1f))printf("idx %d:lo0 %x!=%x\n",i,lo0,(phyaddr >> 6)|0x1f);
	if(lo1!=(((phyaddr+PAGE_SIZE) >> 6)|0x1f))printf("idx %d:lo0 %x!=%x\n",i,lo1,((phyaddr+PAGE_SIZE) >> 6)|0x1f);
	if(hi!=(viraddr | (pid)))printf("idx %d:vadd %lx!=%lx\n",i,hi,viraddr | (pid));
	phyaddr=phyaddr+PAGE_SIZE*2;
	viraddr=viraddr+PAGE_SIZE*2;
}

return 0;
}

extern void CPU_TLBClear();
extern void CPU_TLBInit(unsigned int address,unsigned int steps);
static int tlbclear(int argc,char **argv)
{
CPU_TLBClear();
return 0;
}

static int tlbinit(int argc,char **argv)
{
	unsigned int addr,size;
	if(argc!=3)return -1;

	addr=strtoul(argv[1],0,0);
	size=strtoul(argv[2],0,0);
CPU_TLBInit(addr,size);
return 0;
}

extern int kbd_initialize(void);
static int initkbd(int argc,char **argv)
{
 return kbd_initialize();
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
	}
return 0;
}

void mycacheflush(long long addrin,unsigned int size,unsigned int rw)
{
unsigned int status;
unsigned long long addr;
addr=addrin&~0x1fULL;
size=(addrin-addr+size+0x1f)&~0x1fUL;
asm("
		#define COP_0_STATUS_REG	$12
		#define SR_DIAG_DE		0x00010000
		mfc0	%0, $12		# Save the status register.
		li	$2, 0x00010000
		mtc0	$2, $12		# Disable interrupts
		":"=r"(status)
		::"$2");
if(rw)
{
	asm("
		#define HitWBInvalidate_S   0x17
		#define HitWBInvalidate_D   0x15
		.set noreorder
		1:	
		sync
		cache   0x17, 0(%0)
		daddiu %0,32
		addiu %1,-32
		bnez %1,1b
		nop
		.set reorder
			"::"r"(addr),"r"(size));
}
else
{
	asm("
	#define HitInvalidate_S     0x13
	#define HitInvalidate_D     0x11
	.set noreorder
	1:	
	sync
	cache	0x13, 0(%0)
	daddiu %0,32
	addiu %1,-32
	bnez %1,1b
	nop
	.set reorder
		"::"r"(addr),"r"(size));
}

asm("
	#define COP_0_STATUS_REG	$12
	mtc0	%0, $12		# Restore the status register.
		"::"r"(status));

}

static int cmd_cacheflush(int argc,char **argv)
{
unsigned long long addr;
unsigned int size,rw;
if(argc!=4)return -1;
addr=strtoull(argv[1],0,0);
size=strtoul(argv[2],0,0);
rw=strtoul(argv[3],0,0);

mycacheflush(addr,size,rw);
return 0;
}

static int cmd_cflush(int argc,char **argv)
{
unsigned long addr;
unsigned int size,rw;
if(argc!=4)return -1;
addr=strtoul(argv[1],0,0);
size=strtoul(argv[2],0,0);
rw=strtoul(argv[3],0,0);

pci_sync_cache(0,addr,size,rw);
return 0;
}

#if NMOD_FLASH_ST
#include <include/pflash.h>
static int erase(int argc,char **argv)
{
struct fl_map *map;
int offset;
if(argc!=2)return -1;
map=tgt_flashmap();
map++;
printf("%x,%d,%d,",map,map->fl_map_width,map->fl_map_bus);
offset=strtoul(argv[1],0,0);
printf(",%x\n",offset);
fl_erase_sector_st(map,0,offset);
return 0;
}

static int program(int argc,char **argv)
{
struct fl_map *map;
int offset;
int i;
if(argc!=3)return -1;
map=tgt_flashmap();
map++;
printf("%x,%d,%d,",map,map->fl_map_width,map->fl_map_bus);
offset=strtoul(argv[1],0,0);
for(i=0;i<strlen(argv[2]);i=i+2)
fl_program_st(map,0,offset+i,&argv[2][i]);
return 0;
}
#endif
//------------------------------------------------------------------------------------------
static int loopcmd(int ac,char **av)
{
static char buf[0x100];
int count,i;
int n=1;
	if(ac<3)return -1;
	count=strtol(av[1],0,0);
	while(count--)
	{
	buf[0]=0;
		for(i=2;i<ac;i++)
		{
		strcat(buf,av[i]);
		strcat(buf," ");
		}
	if(av[0][0]=='l')printf("NO %d\n",n++);
	do_cmd(buf);
	}
	return 0;
}

static int checksum(int ac,char **argv)
{
	unsigned long addr,size;
	unsigned long sum = 0;
	unsigned long *p;
	int i = 0;

	if (ac!=3)
		return -1;
    addr = strtoul(argv[1],0,0);
    size = strtoul(argv[2],0,0);

	if (addr < heaptop || (addr + size) >= 0x90000000) {
		printf("Invalid addr,size <%lx,%lx>\n",addr,size);
	}

	printf("checksuming addr %lx,size=%lx\n",addr,size);
	p = (unsigned long *)addr;

	while (i<size) {
		sum += *p++;
		i = i + sizeof(long);
	}

	printf("total %d words, checksum=0x%x\n",i/4,sum);

	return 0;
}

static int testide(int ac,char **av)
{
	FILE *fp;
	char *buf;
	long bufsize;
	unsigned long count,x,tmp;
	bufsize=(ac==1)?0x100000:strtoul(av[1],0,0);
	buf=heaptop;
	x=count=0;
	tmp=bufsize>>24;
	fp=fopen("/dev/disk/wd0","r");
	if(!fp){printf("open error!\n");return -1;}
	while(!feof(fp))
	{
		fread(buf,bufsize,1,fp);
		x +=bufsize;
		if((x&0xffffff)==0){count+=tmp?tmp:16;printf("%d\n",count);}
	}
	fclose(fp);
	return 0;
}
//------------------------------------------------------------------------------------------
static struct parttype
{char *name;
unsigned char type;
} known_parttype[]=
 {
	"FAT12",1,
	"FAT16",4,
	"W95 FAT32",0xb,
	"W95 FAT32 (LBA)",0xc,
	"W95 FAT16 (LBA)",0xe,
	"W95 Ext'd (LBA)",0xf,
	"Linux",0x83,
	"Linux Ext'd" , 0x85,
	"Dos Ext'd" , 5,
	"Linux swap / Solaris" , 0x82,
	"Linux raid" , 0xfd,	/* autodetect RAID partition */
	"Freebsd" , 0xa5,    /* FreeBSD Partition ID */
	"Openbsd" , 0xa6,    /* OpenBSD Partition ID */
	"Netbsd" , 0xa9,   /* NetBSD Partition ID */
	"Minix" , 0x81,  /* Minix Partition ID */
};
struct partition {
    unsigned char boot_ind;     /* 0x80 - active */
    unsigned char head;     /* starting head */
    unsigned char sector;       /* starting sector */
    unsigned char cyl;      /* starting cylinder */
    unsigned char sys_ind;      /* What partition type */
    unsigned char end_head;     /* end head */
    unsigned char end_sector;   /* end sector */
    unsigned char end_cyl;      /* end cylinder */
    unsigned int start_sect;    /* starting sector counting from 0 */
    unsigned int nr_sects;      /* nr of sectors in partition */
} __attribute__((packed));


static int fdisk(int argc,char **argv)
{
int j,n,type_counts;
struct partition *p0;
FILE *fp;
char device[0x40];
int buf[0x10];
sprintf(device,"/dev/disk/%s",(argc==1)?"wd0":argv[1]);
type_counts=sizeof(known_parttype)/sizeof(struct parttype);
fp=fopen(device,"rb");
if(!fp)return -1;
fseek(fp,446,0);
fread(buf,0x40,1,fp);
fclose(fp);

printf("Device Boot %-16s%-16s%-16sId System\n","Start","End","Sectors");
for(n=0,p0=(void *)buf;n<4;n++,p0++)
{
if(!p0->sys_ind)continue;

for(j=0;j<type_counts;j++)
if(known_parttype[j].type==p0->sys_ind)break;

printf("%-6d %-4c %-16d%-16d%-16d%x %s\n",n,(p0->boot_ind==0x80)?'*':' ',p0->start_sect,p0->start_sect+p0->nr_sects,p0->nr_sects,\
p0->sys_ind,j<type_counts?known_parttype[j].name:"unknown");
}
return 0;
}
#include <sys/netinet/in.h>
#include <sys/netinet/in_var.h>
#define SIN(x) ((struct sockaddr_in *)&(x))
extern char activeif_name[];

static void
setsin (struct sockaddr_in *sa, int family, u_long addr)
{
    bzero (sa, sizeof (*sa));
    sa->sin_len = sizeof (*sa);
    sa->sin_family = family;
    sa->sin_addr.s_addr = addr;
}

static int del_if_rt(char *ifname);
static int cmd_ifconfig(int argc,char **argv)
{
struct ifreq *ifr;
struct in_aliasreq *ifra;
struct in_aliasreq data;
int s = socket (AF_INET, SOCK_DGRAM, 0);
	if (s < 0) {
		perror("ifconfig: socket");
		return -1;
	}
ifra=(void *)&data;
ifr=(void *)&data;
bzero (ifra, sizeof(*ifra));
strcpy(ifr->ifr_name,argv[1]);
if(argc==2)
{
(void) ioctl(s, SIOCGIFADDR, ifr);
printf("ip:%s\n",inet_ntoa(satosin(&ifr->ifr_addr)->sin_addr));
(void) ioctl(s,SIOCGIFNETMASK, ifr);
printf("netmask:%s\n",inet_ntoa(satosin(&ifr->ifr_addr)->sin_addr));
(void) ioctl(s,SIOCGIFFLAGS,ifr);
printf("status:%s %s\n",ifr->ifr_flags&IFF_UP?"up":"down",ifr->ifr_flags&IFF_RUNNING?"running":"stoped");
}
else if(argc>=3)
{
	if(argv[2][0]=='d')
	{
	(void) ioctl(s,SIOCGIFFLAGS,ifr);
	ifr->ifr_flags &=~IFF_UP;
	(void) ioctl(s,SIOCSIFFLAGS,ifr);
	}
	else if(argv[2][0]=='u')
	{
	(void) ioctl(s,SIOCGIFFLAGS,ifr);
	ifr->ifr_flags |=IFF_UP;
	(void) ioctl(s,SIOCSIFFLAGS,ifr);
	}
	else if(argv[2][0]=='r')
	{
	(void) ioctl(s,SIOCGIFFLAGS,ifr);
	ifr->ifr_flags &=~IFF_UP;
	(void) ioctl(s,SIOCSIFFLAGS,ifr);
	(void) ioctl(s, SIOCGIFADDR, ifra);
	(void) ioctl(s, SIOCDIFADDR, ifr);
	del_if_rt(argv[1]);
	}
	else if(argv[2][0]=='s')
	{
	register struct ifnet *ifp;
	ifp = ifunit(argv[1]);
	if(!ifp){printf("can not find dev %s.\n",argv[1]);return -1;}
    printf("RX packets:%d,TX packets:%d,collisions:%d\n" \
		   "RX errors:%d,TX errors:%d\n" \
		   "RX bytes:%d TX bytes:%d\n" ,
	ifp->if_ipackets, 
	ifp->if_opackets, 
	ifp->if_collisions,
	ifp->if_ierrors, 
	ifp->if_oerrors,  
	ifp->if_ibytes, 
	ifp->if_obytes 
	);
	if(ifp->if_baudrate)printf("link speed up to %d Mbps\n",ifp->if_baudrate);
	}
	else
	{
	(void) ioctl(s, SIOCGIFADDR, ifra);
	(void) ioctl(s, SIOCDIFADDR, ifra);
	setsin (SIN(ifra->ifra_addr), AF_INET, inet_addr(argv[2]));
	(void) ioctl(s, SIOCAIFADDR, ifra);
	if(argc>=4)
	 {
	 setsin (SIN(ifra->ifra_addr), AF_INET, inet_addr(argv[3]));
	 (void) ioctl(s,SIOCSIFNETMASK, ifra);
	 }
	}
}
close(s);
return 0;
}

static int cmd_ifdown(int argc,char **argv)
{
struct ifreq ifr;
int s = socket (AF_INET, SOCK_DGRAM, 0);
	if (s < 0) {
		perror("ifconfig: socket");
		return -1;
	}
bzero (&ifr, sizeof(ifr));
strcpy(ifr.ifr_name,argv[1]);
(void) ioctl(s, SIOCGIFADDR, &ifr);
printf("%s",inet_ntoa(satosin(&ifr.ifr_addr)->sin_addr));
(void) ioctl(s, SIOCDIFADDR, &ifr);
ifr.ifr_flags=0;
(void) ioctl(s,SIOCSIFFLAGS,(void *)&ifr);
close(s);
return 0;
}

static int cmd_ifup(int argc,char **argv)
{
struct ifreq ifr;
int s = socket (AF_INET, SOCK_DGRAM, 0);
	if (s < 0) {
		perror("ifconfig: socket");
		return -1;
	}
bzero (&ifr, sizeof(ifr));
strcpy(ifr.ifr_name,argv[1]);
ifr.ifr_flags=IFF_UP;
(void) ioctl(s,SIOCSIFFLAGS,(void *)&ifr);
close(s);
return 0;
}
static int  cmd_rtlist(int argc,char **argv)
{
	db_show_arptab();
return 0;
}
#include <sys/net/route.h>
static int cmd_rtdel(int argc,char **argv)
{
	struct sockaddr dst;
	bzero(&dst,sizeof(dst));
	setsin (SIN(dst), AF_INET, inet_addr("10.0.0.3"));
	rtrequest(RTM_DELETE, &dst, 0, 0, 0,0);
	return 0;
}

static int mydelrt(rn, w)
	struct radix_node *rn;
	void *w;
{
	struct rtentry *rt = (struct rtentry *)rn;
	register struct ifaddr *ifa;

	if (rt->rt_ifp)
	{
		if(!strcmp(rt->rt_ifp->if_xname,(char *)w))
		{
	rtrequest(RTM_DELETE, rt_key(rt), 0, 0, 0,0);
		}
	}

	return (0);
}

/*
 * Function to print all the route trees.
 * Use this from ddb:  "call db_show_arptab"
 */
static int del_if_rt(char *ifname)
{
	struct radix_node_head *rnh;
	rnh = rt_tables[AF_INET];
	rn_walktree(rnh, mydelrt, ifname);
	if (rnh == NULL) {
		printf(" (not initialized)\n");
		return (0);
	}
	return (0);
}



static int cmd_sleep(int argc,char **argv)
{
	unsigned long long microseconds;
	unsigned long long total, start;
	int i;

	microseconds=strtoul(argv[1],0,0);
//	printf("%d\n",microseconds);
	for(i=0;i<microseconds;i++)
	{
	delay(1000);
	}
	return 0;
}

static int cmd_sleep1(int argc,char **argv)
{
	unsigned long long microseconds;
	unsigned long long total, start;
	int i;

	microseconds=strtoul(argv[1],0,0);
//	printf("%d\n",microseconds);
	for(i=0;i<microseconds;i++)
	{
	delay1(1000);
	}
	return 0;
}

static void cmd_led(int argc,char **argv)
{
	int led;
	led=strtoul(argv[1],0,0);
	pckbd_leds(led);
}


static int cmd_testcpu(int argc,char **argv)
{
int count=strtoul(argv[1],0,0);
asm("
.set noreorder
move $4,%0
li $2,0x80000000
1:
.rept 300
ld $3,($2)
add.s $f4,$f2,$f0
mul.s $f10,$f8,$f6
srl $5,$4,10
.endr
bnez $4,1b
addiu $4,-1
.set reorder
"
:
:"r"(count)
);
return 0;
}


int highmemcpy(long long dst,long long src,long long count);

int highmemcpy(long long dst,long long src,long long count)
{
asm("
.set noreorder
1:
beqz %2,2f
nop
lb $2,(%0)
sb $2,(%1)
daddiu %0,1
daddiu %1,1
b 1b
daddiu %2,-1
2:
.set reorder
"
::"r"(src),"r"(dst),"r"(count)
:"$2"
);
return 0;
}

int highmemset(long long addr,char c,long long count)
{
asm("
.set noreorder
1:
beqz %2,2f
nop
sb %1,(%0)
daddiu %0,1
b 1b
daddiu %2,-1
2:
.set reorder
"
::"r"(addr),"r"(c),"r"(count)
:"$2"
);
return 0;
}

static cmd_mymemcpy(int argc,char **argv)
{
	long long src,dst,count;
	int ret;
	if(argc!=4)return -1;
	src=str2addr(argv[1],0,0);
	dst=str2addr(argv[2],0,0);
	count=nr_strtoll(argv[3],0,0);
	ret=highmemcpy(dst,src,count);	
	return ret;	
}
//------------------------------------------------------------------------------------------
static int memcmp(const void * cs,const void * ct,size_t count)
{
	const unsigned char *su1, *su2;
	signed char res = 0;

	for( su1 = cs, su2 = ct; 0 < count; ++su1, ++su2, count--)
		if ((res = *su1 - *su2) != 0)
			break;
	return res;
}

#if defined(NMOD_FLASH_ST)&&defined(FLASH_ST_DEBUG)
#include "c2311.c"
int cmd_testst(void) {
ParameterType fp; /* Contains all Flash Parameters */
ReturnType rRetVal; /* Return Type Enum */
Flash(ReadManufacturerCode, &fp);
printf("Manufacturer Code: %08Xh\r\n",
fp.ReadManufacturerCode.ucManufacturerCode);
Flash(ReadDeviceId, &fp);
printf("Device Code: %08Xh\r\n",
fp.ReadDeviceId.ucDeviceId);
fp.BlockErase.ublBlockNr = 10; /* block number 10 will be erased
*/
rRetVal = Flash(BlockErase, &fp); /* function execution */
return rRetVal;
} /* EndFunction Main*/
#endif

//-------------------------------------------------------------------------------------------
static	unsigned long long lrdata;
static int lwl(int argc,char **argv)
{
	unsigned long long addr;
	if(argc!=2)return -1;
	addr=nr_str2addr(argv[1],0,0);
	  asm("ld  $2,(%0);
		   lwl $2,(%1);
		   sw $2,(%0)
		   "
		  ::"r"(&lrdata),"r"(addr)
		  :"$2"
		 );
	  nr_printf("data=%016llx\n",lrdata);
}
static int lwr(int argc,char **argv)
{
	unsigned long long addr;
	if(argc!=2)return -1;
	addr=nr_str2addr(argv[1],0,0);
	  asm("ld $2,(%0);
		   lwr $2,(%1);
		   sw $2,(%0)
		   "
		  ::"r"(&lrdata),"r"(addr)
		  :"$2"
		 );
	  nr_printf("data=%016llx\n",lrdata);
}
static int ldl(int argc,char **argv)
{
	unsigned long long addr;
	if(argc!=2)return -1;
	addr=nr_str2addr(argv[1],0,0);
	  asm("ld $2,(%0);
		   ldl $2,(%1);
		   sd $2,(%0)
		   "
		  ::"r"(&lrdata),"r"(addr)
		  :"$2"
		 );
	  nr_printf("data=%016llx\n",lrdata);
}
static int ldr(int argc,char **argv)
{
	unsigned long long addr;
	if(argc!=2)return -1;
	addr=nr_str2addr(argv[1],0,0);
	  asm("ld $2,(%0);
		   ldr $2,(%1);
		   sd $2,(%0)
		   "
		  ::"r"(&lrdata),"r"(addr)
		  :"$2"
		 );
	  nr_printf("data=%016llx\n",lrdata);
}

static int linit(int argc,char **argv)
{
	lrdata=0;
	return 0;
}
extern char *allocp1;
#if 1
double sin(double);
static void testfloat()
{
volatile static double  x=1.12,y=1.34,z;
z=sin(x);
printf("sin(1.12)=%d\n",(int)(z*1000));
printf("sin(1.12)=%f\n",z);
z=sin(x)*y;
printf("sin(1.12)*1.34=%d\n",(int)(z*1000));
z=x*y;
printf("1.12*1.34=%d\n",(int)(z*1000));
}
#endif
static int mytest(int argc,char **argv)
{
#if 1
tgt_fpuenable();
testfloat();
#endif

	return 0;
}

static int mycmp(int argc,char **argv)
{
	unsigned char *s1,*s2;
	int length,left;
	if(argc!=4)return -1;
	s1=strtoul(argv[1],0,0);
	s2=strtoul(argv[2],0,0);
	length=strtoul(argv[3],0,0);
	while(left=bcmp(s1,s2,length))
	{
		s1=s1+length-left;
		s2=s2+length-left;
		length=left;
		printf("[%p]!=[%p](0x%02x!=0x%02x)\n",s1,s2,*s1,*s2);
		s1++;
		s2++;
		length--;
	}
	return 0;
}

static int (*oldwrite)(int fd,char *buf,int len)=0;
static char *buffer;
static int total;
extern void (*__msgbox)(int yy,int xx,int height,int width,char *msg);
void *restdout(int  (*newwrite) (int fd, const void *buf, size_t n));
static int newwrite(int fd,char *buf,int len)
{
//if(stdout->fd==fd)
{
memcpy(buffer+total,buf,len);
total+=len;
}
oldwrite(fd,buf,len);
return len;
}


static int mymore(int ac,char **av)
{
int i;
char *myline;
if(ac<2)return -1;
myline=heaptop;
total=0;
buffer=heaptop+0x100000;
oldwrite=restdout(newwrite);
myline[0]=0;
for(i=1;i<ac;i++)
{
strcat(myline,av[i]);
strcat(myline," ");
}
do_cmd(myline);
restdout(oldwrite);
buffer[total]='\n';
buffer[total+1]=0;
__console_alloc();
__msgbox(0,0,24,80,buffer);
__console_free();
return 0;
}


//----------------------------------
static const Cmd Cmds[] =
{
	{"MyCmds"},
	{"pcs",	"bus dev func", 0, "select pci dev function", mypcs, 0, 99, CMD_REPEAT},
	{"disks",	"disk", 0, "select disk", mydisks, 0, 99, CMD_REPEAT},
	{"d1",	"[addr] [count]", 0, "dump address byte", dump, 0, 99, CMD_REPEAT},
	{"d2",	"[addr] [count]", 0, "dump address half world", dump, 0, 99, CMD_REPEAT},
	{"d4",	"[addr] [count]", 0, "dump address world", dump, 0, 99, CMD_REPEAT},
	{"d8",	"[addr] [count]", 0, "dump address double word", dump, 0, 99, CMD_REPEAT},
	{"m1",	"addr [data]", 0, "modify address byte", modify, 0, 99, CMD_REPEAT},
	{"m2",	"addr [data]", 0, "mofify address half world", modify, 0, 99, CMD_REPEAT},
	{"m4",	"addr [data]", 0, "modify address world", modify, 0, 99, CMD_REPEAT},
	{"m8",	"addr [data]", 0, "modify address double word",modify, 0, 99, CMD_REPEAT},
	{"setvga","[0|1]",0,"set vga_available",setvga,0,99,CMD_REPEAT},
	{"setkbd","[0|1]",0,"set kbd_available",setkbd,0,99,CMD_REPEAT},
	{"initkbd","",0,"kbd_initialize",initkbd,0,99,CMD_REPEAT},
	{"tlbset","viraddr phyaddr [-x]",0,"tlbset viraddr phyaddr [-x]",tlbset,0,99,CMD_REPEAT},
	{"tlbtest","viraddr phyaddr ",0,"tlbset viraddr phyaddr ",tlbtest,0,99,CMD_REPEAT},
	{"tlbclear","",0,"tlbclear",tlbclear,0,99,CMD_REPEAT},
	{"tlbinit","addr size",0,"tlbinit phaddr=vaddr",tlbinit,0,99,CMD_REPEAT},
	{"cache","[0 1]",0,"cache [0 1]",setcache,0,99,CMD_REPEAT},
	{"loop","count cmd...",0,"loopcmd count cmd...",loopcmd,0,99,CMD_REPEAT},
	{"Loop","count cmd...",0,"loopcmd count cmd...",loopcmd,0,99,CMD_REPEAT},
	{"testide","[onecesize]",0,"test ide dma",testide,0,99,CMD_REPEAT},
	{"checksum","start size",0,"calculate checksum for a memory section",checksum,0,99,CMD_REPEAT},
	{"fdisk","diskname",0,"dump disk partation",fdisk,0,99,CMD_REPEAT},
#if NMOD_FLASH_ST
	{"erase","[0 1]",0,"cache [0 1]",erase,0,99,CMD_REPEAT},
	{"program","[0 1]",0,"cache [0 1]",program,0,99,CMD_REPEAT},
#ifdef FLASH_ST_DEBUG
	{"testst","n",0,"",cmd_testst,0,99,CMD_REPEAT},
#endif
#endif
	{"ifconfig","ifname",0,"ifconig fxp0",cmd_ifconfig,2,99,CMD_REPEAT},
	{"ifup","ifname",0,"ifup fxp0",cmd_ifup,2,99,CMD_REPEAT},
	{"ifdown","ifname",0,"ifdown fxp0",cmd_ifdown,2,99,CMD_REPEAT},
	{"rtlist","",0,"rtlist",cmd_rtlist,0,99,CMD_REPEAT},
	{"rtdel","",0,"rtdel",cmd_rtdel,0,99,CMD_REPEAT},
	{"sleep","ms",0,"sleep ms",cmd_sleep,2,2,CMD_REPEAT},
	{"sleep1","ms",0,"sleep1 s",cmd_sleep1,2,2,CMD_REPEAT},
	{"cacheflush","addr size rw",0,"cacheflush addr size rw",cmd_cacheflush,0,99,CMD_REPEAT},
	{"cflush","addr size rw",0,"cflush addr size rw",cmd_cflush,0,99,CMD_REPEAT},
	{"memcpy","src dst count",0,"mymemcpy src dst count",cmd_mymemcpy,0,99,CMD_REPEAT},
	{"testcpu","",0,"testcpu",cmd_testcpu,2,2,CMD_REPEAT},
	{"led","n",0,"led n",cmd_led,2,2,CMD_REPEAT},
	{"lwl","n",0,"lwl n",lwl,2,2,CMD_REPEAT},
	{"lwr","n",0,"lwr n",lwr,2,2,CMD_REPEAT},
	{"ldl","n",0,"ldl n",ldl,2,2,CMD_REPEAT},
	{"ldr","n",0,"ldr n",ldr,2,2,CMD_REPEAT},
	{"linit","",0,"linit",linit,1,1,CMD_REPEAT},
	{"mytest","",0,"mytest",mytest,1,1,CMD_REPEAT},
	{"mycmp","s1 s2 len",0,"mecmp s1 s2 len",mycmp,4,4,CMD_REPEAT},
	{"mymore","",0,"mymore",mymore,1,99,CMD_REPEAT},
	{0, 0}
};


static void init_cmd __P((void)) __attribute__ ((constructor));

static void
init_cmd()
{
	cmdlist_expand(Cmds, 1);
}


