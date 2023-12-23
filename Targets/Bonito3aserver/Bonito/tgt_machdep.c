/*	$Id: tgt_machdep.c,v 1.1.1.1 2006/09/14 01:59:08 root Exp $ */

/*
 * Copyright (c) 2001 Opsycon AB  (www.opsycon.se)
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by Opsycon AB, Sweden.
 * 4. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */

#define STDIN ((kbd_available|usb_kbd_available)?3:0)

#include <include/stdarg.h>
unsigned int mem_size = 0;

#include <include/stdio.h>
#include <include/string.h>
#include <include/file.h>
#include <include/termio.h>
#include "target/sbd.h"

#include "../../../pmon/cmds/cmd_main/window.h"
#include "../../../pmon/cmds/cmd_main/cmd_main.h"
#include "../../../pmon/common/smbios/smbios.h"

// 
#include <sys/ioccom.h>

/* Generic file-descriptor ioctl's. */
#define	FIOCLEX		 _IO('f', 1)		/* set close on exec on fd */
#define	FIONCLEX	 _IO('f', 2)		/* remove close on exec */
#define	FIONREAD	_IOR('f', 127, int)	/* get # bytes to read */
#define	FIONBIO		_IOW('f', 126, int)	/* set/clear non-blocking i/o */
#define	FIOASYNC	_IOW('f', 125, int)	/* set/clear async i/o */
#define	FIOSETOWN	_IOW('f', 124, int)	/* set owner */
#define	FIOGETOWN	_IOR('f', 123, int)	/* get owner */




#define STDIN		((kbd_available|usb_kbd_available)?3:0)
//include "sys/sys/filio.h"
#include "loongson3_def.h"

extern char * nvram_offs;


void		tgt_putchar (int);
    int
tgt_printf (const char *fmt, ...)
{
    int  n;
    char buf[1024];
    char *p=buf;
    char c;
    va_list     ap;
    va_start(ap, fmt);
    n = vsprintf (buf, fmt, ap);
    va_end(ap);
    while((c=*p++))
    { 
	if(c=='\n')tgt_putchar('\r');
	tgt_putchar(c);
    }
    return (n);
}

#if 1
#include <sys/param.h>
#include <sys/syslog.h>
#include <machine/endian.h>
#include <sys/device.h>
#include <machine/cpu.h>
#include <machine/pio.h>
#include <machine/intr.h>
#include <dev/pci/pcivar.h>
#endif
#include <sys/types.h>
#include <termio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <include/file.h>

#include <dev/ic/mc146818reg.h>
#include <linux/io.h>

#include <autoconf.h>

#include <machine/cpu.h>
#include <machine/pio.h>
#include "pflash.h"
#include "dev/pflash_tgt.h"

#include "include/bonito.h"
#include <pmon/dev/gt64240reg.h>
#include <pmon/dev/ns16550.h>
#include "target/firewall.h"

#include <pmon.h>

#include "../pci/amd_780e.h"
#include "../pci/rs780_cmn.h"
#include "../pci/sb700.h"

#include "mod_x86emu_int10.h"
#include "mod_x86emu.h"
#include "mod_vgacon.h"
#include "mod_framebuffer.h"
#include "mod_smi712.h"
#include "mod_smi502.h"
#include "mod_sisfb.h"
#if PCI_IDSEL_CS5536 != 0
#include <include/cs5536.h>
#endif
#if (NMOD_X86EMU_INT10 > 0)||(NMOD_X86EMU >0)
extern int vga_bios_init(void);
#endif
extern int radeon_init(void);
extern int kbd_initialize(void);
extern int write_at_cursor(char val);
extern const char *kbd_error_msgs[];
#include "flash.h"
#if (NMOD_FLASH_AMD + NMOD_FLASH_INTEL + NMOD_FLASH_SST) == 0
#ifdef HAVE_FLASH
#undef HAVE_FLASH
#endif
#else
#define HAVE_FLASH
#endif

#if (NMOD_X86EMU_INT10 == 0)&&(NMOD_X86EMU == 0)
int vga_available=0;
#elif defined(VGAROM_IN_BIOS) && defined(USE_780E_VGA)
#include "vgarom.c"
#elif defined(VGAROM_IN_BIOS) && defined(USE_BMC)
#include "vgarom_bmc.c"
#endif
#include "timer_irq.h"

int tgt_i2cread(int type,unsigned char *addr,int addrlen,unsigned char reg,unsigned char* buf ,int count);
int tgt_i2cwrite(int type,unsigned char *addr,int addrlen,unsigned char reg,unsigned char* buf ,int count);
extern struct trapframe DBGREG;
extern void *memset(void *, int, size_t);

int kbd_available;
int bios_available;
int usb_kbd_available;
int vga_available;
int cmd_main_mutex = 0;
int bios_mutex = 0;

static int md_pipefreq = 0;
static int md_cpufreq = 0;
static int clk_invalid = 0;
static int nvram_invalid = 0;
static int cksum(void *p, size_t s, int set);
static void _probe_frequencies(void);

#ifndef NVRAM_IN_FLASH
void nvram_get(char *);
void nvram_put(char *);
#endif

extern int vgaterm(int op, struct DevEntry * dev, unsigned long param, int data);
extern int fbterm(int op, struct DevEntry * dev, unsigned long param, int data);
void error(unsigned long *adr, unsigned long good, unsigned long bad);
void modtst(int offset, int iter, unsigned long p1, unsigned long p2);
void do_tick(void);
void print_hdr(void);
void ad_err2(unsigned long *adr, unsigned long bad);
void ad_err1(unsigned long *adr1, unsigned long *adr2, unsigned long good, unsigned long bad);
void mv_error(unsigned long *adr, unsigned long good, unsigned long bad);

void print_err( unsigned long *adr, unsigned long good, unsigned long bad, unsigned long xor);
static inline unsigned char CMOS_READ(unsigned char addr);
static inline void CMOS_WRITE(unsigned char val, unsigned char addr);
static void init_legacy_rtc(void);

#ifdef INTERFACE_3A780E
#define REV_ROW_LINE 560
#define INF_ROW_LINE 576

int afxIsReturnToPmon = 0;

  struct FackTermDev
  {
      int dev;
  };

#endif
#ifdef USE_GPIO_SERIAL
//modified by zhanghualiang 
//this function is for parallel port to send and received  data
//via GPIO
//GPIO bit 2 for Receive clk   clk hight for request
//GPIO bit 3 for receive data 
#define DBG     
#define WAITSTART 10
#define START 20
#define HIBEGCLK 30
#define HINEXTCLK 40
#define LOWCLK 50
#define RXERROR 60
//static int zhlflg = 0;
    static int
ppns16550 (int op, struct DevEntry *dev, unsigned long param, int data)
{
    volatile ns16550dev *dp;  
    static int zhlflg = 0;
    int     crev,crevp,dat,datap,clk,clkp; //zhl begin
    int 	cnt = 0;
    int 	tick = 0;
    int	STAT = START;
    int	tmp;
    char    cget = 0;
    crev = 0;
    crevp = 0;
    dat = 0;
    datap = 0;	
    clk = 0;
    clkp = 0;	
    cget = 0;                                         //zhl end

    dp = (ns16550dev *) dev->sio;
    if(dp==-1)return 1;
    switch (op) {
	case OP_INIT:
	    return 1;

	case OP_XBAUD:
	case OP_BAUD:
	    return 0;

	case OP_TXRDY:
	    return 1;

	case OP_TX:
	    tgt_putchar(data);
	    break;

	case OP_RXRDY:
	    tmp = *(volatile int *)(0xbfe0011c);
	    //	tgt_printf("tmp=(%x)H, %d  ",tmp,tmp);
	    tmp = tmp >> 18;
	    tmp = tmp & 1;
	    if (tmp)
	    {
		//	tgt_printf("pmon pmon pom pmon pomn pmon get signal ");
		//	tgt_putchar('O');
		//	tgt_putchar('K');
		tgt_putchar(0x80);
		//	tgt_printf("\n");
	    }
	    return (tmp);

	case OP_RX:
	    cget = 0;
	    cnt = 0;
	    tick = 0;
	    while (cnt < 8)
	    {
		crevp = crev;
		tmp = *(volatile int *)(0xbfe0011c);
		//     tgt_printf("pmon tmp=%x,\n",tmp);
		tmp = tmp >> 18;
		tmp = tmp & 3;
		crev = tmp;

		clkp = clk;
		clk = crev & 1;
		datap = dat;
		dat = crev & 2;
		dat = dat >> 1;
		dat = dat & 1;
		//	if (clkp != clk)
		//	{
		tick++;
		if(tick>5000000)
		{	
		    //	tgt_putchar(0x80);
		    return 10;	
		}
		//		tgt_printf("%d,%d\n",clk,dat);
		//		}       
		//		continue;
		switch(STAT)
		{
		    case START:
			if (clk == 0)
			{
			    STAT = LOWCLK;		
			}
			break;

		    case LOWCLK:
			if(clk == 1)
			{	
			    STAT = HIBEGCLK;
			}
			break;

		    case HIBEGCLK:
			dat = dat << cnt;
			cget = cget | dat;
			cnt++;
			STAT = HINEXTCLK;
			//		tgt_printf("p c=%d, d=%d,\n",cnt,dat);
			if (cnt == 8 )
			{
			    do {
				tmp = *(volatile int *)(0xbfe0011c);
				tmp = tmp >> 18;
				tmp = tmp & 1;
			    }
			    while(tmp);
			    /*	{	sl_tgt_putchar('p');
				sl_tgt_putchar(' ');
				sl_tgt_putchar('c');
				sl_tgt_putchar('=');
				sl_tgt_putchar(cget);
				sl_tgt_putchar('\n');
				}	*/
			    //		tgt_printf(" ch = (%x)h,%d\n",cget,cget);
			    return cget;
			}
			break;

		    case HINEXTCLK:
			if (clk == 0)
			    STAT = LOWCLK;
			break;

		    case RXERROR:
			break;
		}
	    }



	case OP_RXSTOP:
	    return (0);
	    break;
    }
    return 0;
}
//end modification by zhanghualiang
#endif



ConfigEntry	ConfigTable[] =
{
#ifdef HAVE_NB_SERIAL
#ifdef DEVBD2F_FIREWALL
    { (char *)LS2F_COMA_ADDR, 0, ns16550, 256, CONS_BAUD, NS16550HZ/2 },
#else
#ifdef USE_LPC_UART
    { (char *)COM3_BASE_ADDR, 0, ns16550, 256, CONS_BAUD, NS16550HZ },
#else
#ifdef USE_SB700_LPC_UART
    { (char *)0xb80003f8, 0, ns16550, 256, CONS_BAUD, NS16550HZ },
#else
    { (char *)GS3_UART_BASE, 0, ns16550, 256, CONS_BAUD, NS16550HZ },
#endif
#endif
#endif
#elif defined(USE_SM502_UART0)
    { (char *)0xb6030000, 0, ns16550, 256, CONS_BAUD, NS16550HZ/2 },
#elif defined(USE_GPIO_SERIAL)
    { (char *)COM1_BASE_ADDR, 0,ppns16550, 256, CONS_BAUD, NS16550HZ }, 
#else
    { (char *)COM1_BASE_ADDR, 0, ns16550, 256, CONS_BAUD, NS16550HZ/2 }, 
#endif
#if NMOD_VGACON >0
#if NMOD_FRAMEBUFFER >0
    { (char *)1, 0, fbterm, 256, CONS_BAUD, NS16550HZ },
#else
    { (char *)1, 0, vgaterm, 256, CONS_BAUD, NS16550HZ },
#endif
#endif
#ifdef DEVBD2F_FIREWALL
    { (char *)LS2F_COMB_ADDR, 0, ns16550, 256, CONS_BAUD, NS16550HZ/2 ,1},
#endif
    { 0 }
};

unsigned long _filebase;

extern unsigned long long  memorysize;
extern unsigned long long  memorysize_high;
#ifdef MULTI_CHIP
extern unsigned long long  memorysize_high_n1;
extern unsigned long long  memorysize_high_n2;
extern unsigned long long  memorysize_high_n3;
#endif

extern char MipsException[], MipsExceptionEnd[];

unsigned char hwethadr[6];
static unsigned short wdt;

void initmips(unsigned int raw_memsz);

void addr_tst1(void);
void addr_tst2(void);
void movinv1(int iter, ulong p1, ulong p2);

pcireg_t _pci_allocate_io(struct pci_device *dev, vm_size_t size);
static void euperio_reinit();
int init_ecc();
volatile int init_ecc_finished;
int main();

    void
initmips(unsigned int raw_memsz)
{
	int i;
    int* io_addr;
    tgt_fpuenable();
    SBD_DISPLAY("BEV1",0);
    bcopy(MipsException, (char *)TLB_MISS_EXC_VEC, MipsExceptionEnd - MipsException);
    bcopy(MipsException, (char *)GEN_EXC_VEC, MipsExceptionEnd - MipsException);

    CPU_ConfigCache();
    CPU_FlushCache();

#ifndef ROM_EXCEPTION
    CPU_SetSR(0, SR_BOOT_EXC_VEC);
#endif
#if NTIMER_IRQ
    init_IRQ();
#endif
#ifdef DEVBD2F_SM502
    {
		/*set lio bus to 16 bit*/
		volatile int *p=0xbfe00108;
		*p=((*p)&~(0x1f<<8))|(0x8<<8) |(1<<13);
    }
#endif
    //tgt_printf("memsz %d\n",memsz);
    /*enable float*/
    tgt_fpuenable();
    //CPU_TLBClear(); /* Lc modify */

#if PCI_IDSEL_CS5536 != 0
    superio_reinit();
#endif
    get_memorysize(raw_memsz);
 
#if defined(FASTECC1)||defined(FASTECC2)
    asm volatile(".set mips64;dli $2,0x900000003ff01000;sw %0,0x20($2);sw %0,0x28($2);lw %0,0x20($2);.set mips0;"::"r"(init_ecc):"$2");
#endif

#if 0 /* whd : Disable gpu controller of MCP68 */
    //*(unsigned int *)0xbfe809e8 = 0x122380;
    //*(unsigned int *)0xbfe809e8 = 0x2280;
    *(unsigned int *)0xba0009e8 = 0x2280;
#endif

#if 0 /* whd : Enable IDE controller of MCP68 */
    *(unsigned int *)0xbfe83050 = (*(unsigned int *)0xbfe83050) | 0x2;

#endif

#if 0
    asm(\
	    "	 sd %1,0x18(%0);;\n" \
	    "	 sd %2,0x28(%0);;\n" \
	    "	 sd %3,0x20(%0);;\n" \
	    ::"r"(0xffffffffbff00000ULL),"r"(memorysize),"r"(memorysize_high),"r"(0x20000000)
	    :"$2"
       );
#endif

#if 0
    {
		int start = 0x80000000;
		int end = 0x80000000 + 16384;

		while (start < end) {
			__asm__ volatile (" cache 1,0(%0)\r\n"
				" cache 1,1(%0)\r\n"
				" cache 1,2(%0)\r\n"
				" cache 1,3(%0)\r\n"
				" cache 0,0(%0)\r\n"::"r"(start));
			start += 32;
		}

		__asm__ volatile ( " mfc0 $2,$16\r\n"
			" and  $2, $2, 0xfffffff8\r\n"
			" or   $2, $2, 2\r\n"
			" mtc0 $2, $16\r\n" :::"$2");
    }
#endif

    /*
     *  Probe clock frequencys so delays will work properly.
     */
    tgt_cpufreq();
    SBD_DISPLAY("DONE",0);
    /*
     *  Init PMON and debug
     */
    cpuinfotab[0] = &DBGREG;
    dbginit(NULL);

    /*
     *  Set up exception vectors.
     */
    SBD_DISPLAY("BEV0",0);

    printf("BEV in SR set to zero.\n");



#if PCI_IDSEL_VIA686B
    if(getenv("powermg"))
    {
		pcitag_t mytag;
		unsigned char data;
		unsigned int addr;
		mytag=_pci_make_tag(0,PCI_IDSEL_VIA686B,4);
		data=_pci_conf_readn(mytag,0x41,1);
		_pci_conf_writen(mytag,0x41,data|0x80,1);
		addr=_pci_allocate_io(_pci_head,256);
		printf("power management addr=%x\n",addr);
		_pci_conf_writen(mytag,0x48,addr|1,4);
    }
#endif

#if 0//HT BUS DEBUG whd
    printf("HT-PCI header scan:\n");
    io_addr = 0xbfe84000;
    for(i=0 ;i < 16;i++)
    {
		printf("io_addr : 0x%8X = 0x%8X\n",io_addr, *io_addr);
		io_addr = io_addr + 1;
    }

    //printf("BUS 9 scan:\n");
    //io_addr = 0xbe090000;
    //for(i=0 ;i < 16;i++)
    //{
    //	printf("io_addr : 0x%X = 0x%X\n",io_addr, *io_addr);
    //	io_addr = io_addr + (1<<9);
    //}

    printf("PCI bus header scan:\n");
    io_addr = 0xbe014800;
    for(i=0 ;i < 16;i++)
    {
		printf("io_addr : 0x%8X = 0x%8X\n",io_addr, *io_addr);
		io_addr = io_addr + 1;
    }

    //printf("write network IO space test\n");
    //io_addr = 0xba00ffc0;
    //for(i=0 ;i < 16;i++)
    //{
    //	printf("io_addr : 0x%X = 0x%X\n",io_addr, *io_addr);
    //	io_addr = io_addr + 1;
    //}

    //io_addr = 0xba00ffc0;
    //i = *io_addr;
    //	printf("io_addr : 0x%X = 0x%X\n",io_addr, i);

    //i = i | 0x1000000;
    //*io_addr = i;
    //	printf("io_addr : 0x%X = 0x%X\n",io_addr, *io_addr);
    //
    //printf("HT-SATA header scan:\n");
    //io_addr = 0xbfe84800;
    //for(i=0 ;i < 64;i++)
    //{
    //	printf("io_addr : 0x%8X = 0x%8X\n",io_addr, *io_addr);
    //	io_addr = io_addr + 1;
    //}
    //printf("HT Register\n");
    //io_addr = 0xbb000050;
    //for(i=0 ;i < 1;i++)
    //{
    //	printf("io_addr : 0x%X = 0x%X\n",io_addr, *io_addr);
    //	io_addr = io_addr + 1;
    //}
#endif

    /*
     * Launch!
     */
    _pci_conf_write(_pci_make_tag(0,0,0),0x90,0xff800000); 
#ifdef FASTECC
    init_ecc();
#endif
#if defined(FASTECC) || defined(FASTECC1) || defined(FASTECC2)
    init_ecc_end();
#endif
#if NTIMER_IRQ
    uninit_IRQ();
#endif
    main();
}

/*
 *  Put all machine dependent initialization here. This call
 *  is done after console has been initialized so it's safe
 *  to output configuration and debug information with printf.
 */
extern void	vt82c686_init(void);
int psaux_init(void);
extern int video_hw_init (void);

extern int fb_init(unsigned long,unsigned long);

extern unsigned long long uma_memory_base;
extern unsigned long long uma_memory_size;

void tgt_devconfig()
{
	int ic, len;
        int count = 0;
        char key;
        char copyright[9] ="REV_";
        char bootup[] = "Booting...";
        char *tmp_copy = NULL;
       	char * s;
#if NMOD_VGACON > 0
    int rc=1;
#if NMOD_FRAMEBUFFER > 0 
    unsigned long fbaddress,ioaddress;
    extern struct pci_device *vga_dev;
#ifdef USE_780E_VGA
#ifdef RS780E
    int test;
    int  i;
//    printf(" ====================  frame buffer test begin======================:%x \n" , test);
    for (i = 0;i < 0x100000;i += 4)
    {
        //printf(" i = %x \n" , i);
        *((volatile int *)(BONITO_PCILO_BASE_VA + i)) = i;
    }

    for (i = 0xffffc;i >= 0;i -= 4)
    {
        if (*((volatile int *)(BONITO_PCILO_BASE_VA + i)) != i)
        {
            //printf(" not equal ====  %x\n" ,i);
            break;
        }
    }

//    printf(" ====================  frame buffer test end======================:%x \n" , test);

    
#endif
#endif
#endif
#endif
	_pci_devinit(1);	/* PCI device initialization */
#if (NMOD_X86EMU_INT10 > 0)||(NMOD_X86EMU >0)
	SBD_DISPLAY("VGAI", 0);
	rc = vga_bios_init();
#endif
#if defined(VESAFB)
	SBD_DISPLAY("VESA", 0);
	if(rc > 0)
		vesafb_init();
#endif
#if (NMOD_X86EMU_INT10 == 0 && defined(RADEON7000))
	SBD_DISPLAY("VGAI", 0);
	rc = radeon_init();
#endif
#if NMOD_FRAMEBUFFER > 0
	vga_available=0;
	if(!vga_dev) {
		printf("ERROR !!! VGA device is not found\n"); 
		rc = -1;
	}
	if (rc > 0) {
		fbaddress  =_pci_conf_read(vga_dev->pa.pa_tag,0x10);
#ifdef USE_780E_VGA
		ioaddress  =_pci_conf_read(vga_dev->pa.pa_tag,0x18);
#elif defined(USE_BMC)
		ioaddress  =_pci_conf_read(vga_dev->pa.pa_tag,0x14);
#endif
		fbaddress = fbaddress &0xffffff00; //laster 8 bit
		ioaddress = ioaddress &0xfffffff0; //laster 4 bit

#ifdef USE_780E_VGA
#if NMOD_SISFB
		fbaddress=sisfb_init_module();
#endif
		printf("fbaddress 0x%x\tioaddress 0x%x\n",fbaddress, ioaddress);

#if NMOD_SMI712 > 0
		fbaddress |= 0xb0000000;
		//ioaddress |= 0xbfd00000;
		ioaddress |= BONITO_PCIIO_BASE_VA;
		smi712_init((unsigned char *)fbaddress,(unsigned char *)ioaddress);
#endif

#if NMOD_SMI502 > 0
		rc = video_hw_init ();
		fbaddress  =_pci_conf_read(vga_dev->pa.pa_tag,0x10);
		ioaddress  =_pci_conf_read(vga_dev->pa.pa_tag,0x14);
		fbaddress |= 0xb0000000;
		ioaddress |= 0xb0000000;
#endif

		/* lwg add.
		 * The address mapped from 0x10000000 to 0xf800000
		 * wouldn't work through tlb.
		 _*/
#ifdef CONFIG_GFXUMA
		fbaddress = 0x50000000; // virtual address mapped to the second 256M memory
#else
		fbaddress |= uma_memory_base | BONITO_PCILO_BASE_VA;
#endif
#elif defined(USE_BMC)
		fbaddress |= BONITO_PCILO_BASE_VA;
#endif
		printf("fbaddress = %08x\n", fbaddress);

		printf("begin fb_init\n");
		fb_init(fbaddress, ioaddress);
		printf("after fb_init\n");

	} else {
		printf("vga bios init failed, rc=%d\n",rc);
	}
#endif

#if (NMOD_FRAMEBUFFER > 0) || (NMOD_VGACON > 0 )
	if (rc > 0)
		if(!getenv("novga")) vga_available=1;
		else vga_available=0;
#endif
	config_init();
	configure();
	//#if ((NMOD_VGACON >0) &&(PCI_IDSEL_VIA686B !=0)|| (PCI_IDSEL_CS5536 !=0))
#if NMOD_VGACON >0
	if(getenv("nokbd")) rc=1;
	else {
		rc=kbd_initialize();
	}
	printf("%s\n",kbd_error_msgs[rc]);
	if(!rc){ 
		kbd_available=1;
	}
	//	psaux_init();
#endif
#ifdef INTERFACE_3A780E 

	vga_available = 1;
       	kbd_available=1;
        bios_available = 1; //support usb_kbd in bios
// Ask user whether to set bios menu
        printf("Press <Del> to set BIOS,waiting for 3 seconds here..... \n");

         video_set_color(0xf);

         init_win_device();

         vga_available = 0;          //lwg close printf output

	{
       	struct FackTermDev *devp;
        int fd = 0;
        DevEntry* p;
        Msg msg;   //==char msg
        int sign = 0;

        //get input without wait ===========   add by oldtai
        fd = ((FILE*)stdin)->fd;
        devp = (struct FackTermDev *)(_file[fd].data);
        p = &DevTable[devp->dev];

        for (count = 0;count < 10;count++)
        {
          	//get input without wait
                scandevs();

                while(!tgt_smplock());

                /* 'DEL' to BIOS Interface */
                if(p->rxq->count >= 3){
                  if ((p->rxq->dat[p->rxq->first + p->rxq->count-3] == 0x1b)
                          && (p->rxq->dat[p->rxq->first + p->rxq->count-2] == '[')
                          && (p->rxq->dat[p->rxq->first + p->rxq->count-1] == 'G')) {
                        sign = 1;
                        p->rxq->count -= 3;
                  }
                  if (sign == 1) {
                        tgt_smpunlock();
                        break;
                  }
                }
                tgt_smpunlock();

                //delay1(30);
/*If you want to Press <Del> to set BIOS open it(from 0 to 1)*/
#if 1
                delay1(300);
#endif
  	}

        vga_available = 1;
        video_set_color(0xf);

        for (ic = 0; ic < 64; ic++)
        {
         	video_putchar1(2 + ic*8, REV_ROW_LINE, ' ');
                video_putchar1(2 + ic*8, INF_ROW_LINE, ' ');
        }

        vga_available = 0;

        if (count >= 10)
        	goto run;
        else
                goto bios;

  bios:

         if(!(s = getenv("SHOW_DISPLAY")) || s[0] !='2')
          {
              char buf[10];
              video_set_color(0xf);
              video_set_color(0x8);
              tty_flush();
                          vga_available = 1;
              do_cmd("main");
              if (!afxIsReturnToPmon)
              {
                 vga_available = 0;
              }
          }

run:
        vga_available = 1;
        bios_available = 0;//support usb_kbd in bios
       	kbd_available = 1;

        len = strlen(bootup);
        for (ic = 0; ic < len; ic++)
        {
         	video_putchar1(2 + ic*8, INF_ROW_LINE,bootup[ic]);
        }
	}
#endif
	printf("devconfig done.\n");

	sb700_interrupt_fixup();
}

extern int test_icache_1(short *addr);
extern int test_icache_2(int addr);
extern int test_icache_3(int addr);
extern void godson1_cache_flush(void);
#define tgt_putchar_uc(x) (*(void (*)(char)) (((long)tgt_putchar)|0x20000000)) (x)

extern void cs5536_gpio_init(void);
extern void test_gpio_function(void);
extern void cs5536_pci_fixup(void);

#if PCI_IDSEL_SB700 != 0
static int w83627_read(int dev,int addr)
{
	int data;
#if 0
	/*enter*/
	outb(0xbfd0002e,0x87);
	outb(0xbfd0002e,0x87);
	/*select logic dev reg */
	outb(0xbfd0002e,0x7);
	outb(0xbfd0002f,dev);
	/*access reg */
	outb(0xbfd0002e,addr);
	data=inb(0xbfd0002f);
	/*exit*/
	outb(0xbfd0002e,0xaa);
	outb(0xbfd0002e,0xaa);
#endif
	/*enter*/
	outb(BONITO_PCIIO_BASE_VA + 0x002e,0x87);
	outb(BONITO_PCIIO_BASE_VA + 0x002e,0x87);
	/*select logic dev reg */
	outb(BONITO_PCIIO_BASE_VA + 0x002e,0x7);
	outb(BONITO_PCIIO_BASE_VA + 0x002f,dev);
	/*access reg */
	outb(BONITO_PCIIO_BASE_VA + 0x002e,addr);
	data=inb(BONITO_PCIIO_BASE_VA + 0x002f);
	/*exit*/
	outb(BONITO_PCIIO_BASE_VA + 0x002e,0xaa);
	outb(BONITO_PCIIO_BASE_VA + 0x002e,0xaa);
	return data;
}

static void w83627_write(int dev,int addr,int data)
{
#if 0
	/*enter*/
	outb(0xbfd0002e,0x87);
	outb(0xbfd0002e,0x87);
	/*select logic dev reg */
	outb(0xbfd0002e,0x7);
	outb(0xbfd0002f,dev);
	/*access reg */
	outb(0xbfd0002e,addr);
	outb(0xbfd0002f,data);
	/*exit*/
	outb(0xbfd0002e,0xaa);
	outb(0xbfd0002e,0xaa);
#endif
	/*enter*/
	outb(BONITO_PCIIO_BASE_VA + 0x002e,0x87);
	outb(BONITO_PCIIO_BASE_VA + 0x002e,0x87);
	/*select logic dev reg */
	outb(BONITO_PCIIO_BASE_VA + 0x002e,0x7);
	outb(BONITO_PCIIO_BASE_VA + 0x002f,dev);
	/*access reg */
	outb(BONITO_PCIIO_BASE_VA + 0x002e,addr);
	outb(BONITO_PCIIO_BASE_VA + 0x002f,data);
	/*exit*/
	outb(BONITO_PCIIO_BASE_VA + 0x002e,0xaa);
	outb(BONITO_PCIIO_BASE_VA + 0x002e,0xaa);
}
#endif

#if PCI_IDSEL_CS5536 != 0
static int w83627_read(int dev,int addr)
{
	int data;
#if 0
	/*enter*/
	outb(0xbfd0002e,0x87);
	outb(0xbfd0002e,0x87);
	/*select logic dev reg */
	outb(0xbfd0002e,0x7);
	outb(0xbfd0002f,dev);
	/*access reg */
	outb(0xbfd0002e,addr);
	data=inb(0xbfd0002f);
	/*exit*/
	outb(0xbfd0002e,0xaa);
	outb(0xbfd0002e,0xaa);
#endif
	/*enter*/
	outb(BONITO_PCIIO_BASE_VA + 0x002e,0x87);
	outb(BONITO_PCIIO_BASE_VA + 0x002e,0x87);
	/*select logic dev reg */
	outb(BONITO_PCIIO_BASE_VA + 0x002e,0x7);
	outb(BONITO_PCIIO_BASE_VA + 0x002f,dev);
	/*access reg */
	outb(BONITO_PCIIO_BASE_VA + 0x002e,addr);
	data=inb(BONITO_PCIIO_BASE_VA + 0x002f);
	/*exit*/
	outb(BONITO_PCIIO_BASE_VA + 0x002e,0xaa);
	outb(BONITO_PCIIO_BASE_VA + 0x002e,0xaa);
	return data;
}

static void w83627_write(int dev,int addr,int data)
{
#if 0
	/*enter*/
	outb(0xbfd0002e,0x87);
	outb(0xbfd0002e,0x87);
	/*select logic dev reg */
	outb(0xbfd0002e,0x7);
	outb(0xbfd0002f,dev);
	/*access reg */
	outb(0xbfd0002e,addr);
	outb(0xbfd0002f,data);
	/*exit*/
	outb(0xbfd0002e,0xaa);
	outb(0xbfd0002e,0xaa);
#endif
	/*enter*/
	outb(BONITO_PCIIO_BASE_VA + 0x002e,0x87);
	outb(BONITO_PCIIO_BASE_VA + 0x002e,0x87);
	/*select logic dev reg */
	outb(BONITO_PCIIO_BASE_VA + 0x002e,0x7);
	outb(BONITO_PCIIO_BASE_VA + 0x002f,dev);
	/*access reg */
	outb(BONITO_PCIIO_BASE_VA + 0x002e,addr);
	outb(BONITO_PCIIO_BASE_VA + 0x002f,data);
	/*exit*/
	outb(BONITO_PCIIO_BASE_VA + 0x002e,0xaa);
	outb(BONITO_PCIIO_BASE_VA + 0x002e,0xaa);
}
#endif
#if PCI_IDSEL_SB700 != 0
static void superio_reinit()
{
	w83627_write(0,0x24,0xc1);

	w83627_write(5,0x30,1);
	w83627_write(5,0x60,0);
	w83627_write(5,0x61,0x60);
	w83627_write(5,0x62,0);
	w83627_write(5,0x63,0x64);
	w83627_write(5,0x70,1);
	w83627_write(5,0x72,0xc);
	w83627_write(5,0xf0,0x80);

	//w83627_UART1
	w83627_write(2,0x30,0x01);
	w83627_write(2,0x60,0x03);
	w83627_write(2,0x61,0xf8);
	w83627_write(2,0x70,0x04);
	w83627_write(2,0xf0,0x00);

	//w83627_UART2
	w83627_write(3,0x30,0x01);
	w83627_write(3,0x60,0x02);
	w83627_write(3,0x61,0xf8);
	w83627_write(3,0x70,0x03);
	w83627_write(3,0xf0,0x00);

        ////w83627_PALLPort
        w83627_write(1,0x30,0x01);
        w83627_write(1,0x60,0x03);
        w83627_write(1,0x61,0x78);
        w83627_write(1,0x70,0x07);
        w83627_write(1,0x74,0x04);
        w83627_write(1,0xf0,0xF0);
}
#endif

#if PCI_IDSEL_CS5536 != 0
static void superio_reinit()
{
	w83627_write(0,0x24,0xc1);
	w83627_write(5,0x30,1);
	w83627_write(5,0x60,0);
	w83627_write(5,0x61,0x60);
	w83627_write(5,0x62,0);
	w83627_write(5,0x63,0x64);
	w83627_write(5,0x70,1);
	w83627_write(5,0x72,0xc);
	w83627_write(5,0xf0,0x80);
	_wrmsr(GET_MSR_ADDR(0x5140001F), 0, 0);//no keyboard emulation

#ifdef USE_CS5536_UART
	//w83627_UART1
	w83627_write(2,0x30,0x01);
	w83627_write(2,0x60,0x03);
	w83627_write(2,0x61,0xe8);
	w83627_write(2,0x70,0x04);
	w83627_write(2,0xf0,0x00);

	//w83627_UART2
	w83627_write(3,0x30,0x01);
	w83627_write(3,0x60,0x02);
	w83627_write(3,0x61,0xe8);
	w83627_write(3,0x70,0x03);
	w83627_write(3,0xf0,0x00);
#else
	//w83627_UART1
	w83627_write(2,0x30,0x01);
	w83627_write(2,0x60,0x03);
	w83627_write(2,0x61,0xf8);
	w83627_write(2,0x70,0x04);
	w83627_write(2,0xf0,0x00);

	//w83627_UART2
	w83627_write(3,0x30,0x01);
	w83627_write(3,0x60,0x02);
	w83627_write(3,0x61,0xf8);
	w83627_write(3,0x70,0x03);
	w83627_write(3,0xf0,0x00);
#endif

	////w83627_PALLPort
	w83627_write(1,0x30,0x01);
	w83627_write(1,0x60,0x03);
	w83627_write(1,0x61,0x78);
	w83627_write(1,0x70,0x07);
	w83627_write(1,0x74,0x04);
	w83627_write(1,0xf0,0xF0); 
	ConfigTable[0].flag=1;//reinit serial
}
#endif

	void
tgt_devinit()
{
	int value;

#if  (PCI_IDSEL_VIA686B != 0)
	SBD_DISPLAY("686I",0);

	vt82c686_init();
#endif

#if  (PCI_IDSEL_CS5536 != 0)
	SBD_DISPLAY("5536",0);
	cs5536_init();
#endif

	tgt_printf("rs780_early_setup\n");
	rs780_early_setup();

	tgt_printf("sb700_early_setup\n");
	sb700_early_setup();
	tgt_printf("rs780_before_pci_fixup\n");
	rs780_before_pci_fixup();
	tgt_printf("sb700_before_pci_fixup\n");
	sb700_before_pci_fixup();
	tgt_printf("rs780_enable\n");
	//vga test
	//rs780_enable(_pci_make_tag(0, 0, 0));
	//rs780_enable(_pci_make_tag(0, 1, 0));
	rs780_after_pci_fixup(); //name dug
	tgt_printf("sb700_enable\n");
	sb700_enable();

#if 1
	tgt_printf("disable bus0 device pcie bridges\n");
	//disable bus0 device 3 pci bridges (dev2 to dev7, dev9-dev10)
	//dev 2 and dev3 should not be open otherwise the vga could not work
	//==by oldtai
	set_nbmisc_enable_bits(_pci_make_tag(0,0,0), 0x0c,(1<<2|1<<3|1<<5|1<<6|1<<7|1<<17),
			(0<<2|0<<3|1<<5|1<<6|1<<7|0<<17));
#endif

#ifndef USE_780E_VGA 
	tgt_printf("disable internal graphics\n");
	//disable internal graphics
	set_nbcfg_enable_bits_8(_pci_make_tag(0,0,0), 0x7c, 1, 1);
#endif

#if 1
	//SBD_DISPLAY("disable OHCI and EHCI controller",0);
	//disable OHCI and EHCI controller
	tgt_printf("disable OHCI and EHCI controller\n");
	_pci_conf_write8(_pci_make_tag(0,0x14, 0), 0x68, 0x0);
#endif

#if 1
	/* usb enable,68h
	   1<<0:usb1 ohci0 enable
	   1<<1:usb1 ohci1 enable
	   1<<2:usb1 ehci enable
	   1<<3:reserved
	   1<<4:usb2 ohci0 enable
	   1<<5:usb2 ohci1 enable
	   1<<6:usb2 ehci enable
	   1<<7:usb3 ohci enable  */
	printf("enable OHCI controller\n");
	_pci_conf_write8(_pci_make_tag(0,0x14, 0), 0x68, (1<<0)|(1<<1)|(1<<2)|(1<<4)|(1<<5)|(1<<6)|(1<<7));
	//  _pci_conf_write8(_pci_make_tag(0,0x14, 0), 0x68, (1<<0));
#endif


#ifndef ENABLE_SATA 
	//SBD_DISPLAY("disable data",0);
	//disable sata
	tgt_printf("disable sata\n");
	value = _pci_conf_read32(_pci_make_tag(0, 0x14, 0), 0xac);
	value &= ~(1 << 8);
	_pci_conf_write32(_pci_make_tag(0, 0x14, 0), 0xac, value);
#endif


	/*
	 *  Gather info about and configure caches.
	 */
	if(getenv("ocache_off")) {
		CpuOnboardCacheOn = 0;
	}
	else {
		CpuOnboardCacheOn = 1;
	}
	if(getenv("ecache_off")) {
		CpuExternalCacheOn = 0;
	}
	else {
		CpuExternalCacheOn = 1;
	}

#if 1
	//CPU_ConfigCache();
#else
	{       
#define CTYPE_HAS_L2  0x100	
		CpuPrimaryInstCacheSize = 64<<10;
		CpuPrimaryInstCacheLSize = 32;
		CpuPrimaryInstSetSize = (64<<10)/32/4;
		CpuPrimaryDataCacheSize = 64<<10;
		CpuPrimaryDataCacheLSize = 32;;
		CpuPrimaryDataSetSize =  (64<<10)/32/4;
		//CpuCacheAliasMask ;
		CpuSecondaryCacheSize = ((1024*4)<<10);
		CpuTertiaryCacheSize = 0;
		CpuNWayCache = 4;
		CpuCacheType = CTYPE_HAS_L2 ;		/* R4K, R5K, RM7K */
		//CpuConfigRegister;
		//CpuStatusRegister;
		//CpuExternalCacheOn;	/* R5K, RM7K */
		//CpuOnboardCacheOn;	/* RM7K */

		//printf("whd:2\n");
		//CPU_FlushCache();
		//printf("whd:3\n");
		asm volatile("mfc0 $4, $16\n"
				"and    $4,0xfffffff8\n"
				"or     $4,0x3\n"
				"mtc0   $4,$16\n":::"a0");

		//printf("whd:4\n");
#if 0
		unsigned long config1, config2, lsize, linesz, sets, ways; 
		asm(".word 0x40028001\n":"=r"(config2));
		if ((lsize = ((config2 >> 4) & 15)))
			linesz = 2 << lsize;
		else
			linesz = lsize;

		sets = 64 << ((config2 >> 8) & 15);
		ways = 1 + ((config2  ) & 15);

		CPUSecondaryCacheSize = sets * ways * linesz;
#endif
	}
#endif
	superio_reinit();
	uart_init();

	_pci_businit(1);	/* PCI bus initialization */
#if defined(VIA686B_POWERFIXUP) && (PCI_IDSEL_VIA686B != 0)
	if(getenv("noautopower"))	vt82c686_powerfixup();
#endif

#if  (PCI_IDSEL_CS5536 != 0)
	cs5536_pci_fixup();
#endif
	tgt_printf("sb700_after_pci_fixup\n");
	sb700_after_pci_fixup();
}


void tgt_poweroff()
{
	volatile char * watch_dog_base = 0xb8000cd6;
	volatile char * watch_dog_config  = 0xba00a041;
	volatile unsigned int * watch_dog_mem = 0xbe010000;
	volatile unsigned char * reg_cf9 = (unsigned char *)0xb8000cf9;

	delay(100);
	*reg_cf9 = 4;

	/* enable WatchDogTimer */
	delay(100);
	* watch_dog_base  = 0x69;
	*(watch_dog_base + 1) = 0x0;

	/* set WatchDogTimer base address is 0x10000 */
	delay(100);
	* watch_dog_base = 0x6c;
	*(watch_dog_base + 1) = 0x0;

	delay(100);
	* watch_dog_base = 0x6d;
	*(watch_dog_base + 1) = 0x0;

	delay(100);
	* watch_dog_base = 0x6e;
	*(watch_dog_base + 1) = 0x1;

	delay(100);
	* watch_dog_base = 0x6f;
	*(watch_dog_base + 1) = 0x0;

	delay(100);
	* watch_dog_config = 0xff;

	/* set WatchDogTimer to starting */
	delay(100);
	* watch_dog_mem = 0x05;
	delay(100);
	*(watch_dog_mem + 1) = 0x1000;
	delay(100);
	* watch_dog_mem = 0x85;


}
void tgt_reboot(void)
{
	watchdog_enable();
	//delay(1000000);
	*(volatile char *)0xb8000cd6=0x85;
	*(volatile char *)0xb8000cd7=0xe;
	while(1);
}


/*
 *  This function makes inital HW setup for debugger and
 *  returns the apropriate setting for the status register.
 */
	register_t
tgt_enable(int machtype)
{
	/* XXX Do any HW specific setup */
	return(SR_COP_1_BIT|SR_FR_32|SR_EXL);
}


/*
 *  Target dependent version printout.
 *  Printout available target version information.
 */
	void
tgt_cmd_vers()
{
}

/*
 *  Display any target specific logo.
 */
	void
tgt_logo()
{
#if 0
	printf("\n");
	printf("[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[\n");
	printf("[[[            [[[[   [[[[[[[[[[   [[[[            [[[[   [[[[[[[  [[\n");
	printf("[[   [[[[[[[[   [[[    [[[[[[[[    [[[   [[[[[[[[   [[[    [[[[[[  [[\n");
	printf("[[  [[[[[[[[[[  [[[  [  [[[[[[  [  [[[  [[[[[[[[[[  [[[  [  [[[[[  [[\n");
	printf("[[  [[[[[[[[[[  [[[  [[  [[[[  [[  [[[  [[[[[[[[[[  [[[  [[  [[[[  [[\n");
	printf("[[   [[[[[[[[   [[[  [[[  [[  [[[  [[[  [[[[[[[[[[  [[[  [[[  [[[  [[\n");
	printf("[[             [[[[  [[[[    [[[[  [[[  [[[[[[[[[[  [[[  [[[[  [[  [[\n");
	printf("[[  [[[[[[[[[[[[[[[  [[[[[  [[[[[  [[[  [[[[[[[[[[  [[[  [[[[[  [  [[\n");
	printf("[[  [[[[[[[[[[[[[[[  [[[[[[[[[[[[  [[[  [[[[[[[[[[  [[[  [[[[[[    [[\n");
	printf("[[  [[[[[[[[[[[[[[[  [[[[[[[[[[[[  [[[   [[[[[[[[   [[[  [[[[[[[   [[\n");
	printf("[[  [[[[[[[[[[[[[[[  [[[[[[[[[[[[  [[[[            [[[[  [[[[[[[[  [[\n");
	printf("[[[[[[[2009 Loongson][[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[\n");
#endif

    printf("\n");
    printf("[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[\n");
    printf("[[  [[[[[[[[[       [[[[[       [[[[   [[[[[  [[[[[      [[[[[       [[[[[       [[[[   [[[[[  [[\n");
    printf("[[  [[[[[[[[   [[[[  [[[   [[[[  [[[    [[[[  [[[[  [[[[  [[[   [[[[  [[[   [[[[  [[[    [[[[  [[\n");
    printf("[[  [[[[[[[[  [[[[[[ [[[  [[[[[[ [[[  [  [[[  [[[  [[[[[[[[[[[[   [[[[[[[  [[[[[[ [[[  [  [[[  [[\n");
    printf("[[  [[[[[[[[  [[[[[[ [[[  [[[[[[ [[[  [[  [[  [[[  [[[    [[[[[[[    [[[[  [[[[[[ [[[  [[  [[  [[\n");
    printf("[[  [[[[[[[[  [[[[[[ [[[  [[[[[[ [[[  [[[  [  [[[  [[[[[  [[[[[[[[[[  [[[  [[[[[[ [[[  [[[  [  [[\n");
    printf("[[  [[[[[[[[   [[[[  [[[   [[[[  [[[  [[[[    [[[   [[[[  [[[   [[[  [[[[   [[[[  [[[  [[[[    [[\n");
    printf("[[       [[[[       [[[[[       [[[[  [[[[[   [[[[       [[[[[      [[[[[[       [[[[  [[[[[   [[\n");
    printf("[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[2011 Loongson][[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[\n");
 
}

static void init_legacy_rtc(void)
{
	int year, month, date, hour, min, sec;
	CMOS_WRITE(DS_CTLA_DV1, DS_REG_CTLA);
	CMOS_WRITE(DS_CTLB_24 | DS_CTLB_DM | DS_CTLB_SET, DS_REG_CTLB);
	CMOS_WRITE(0, DS_REG_CTLC);
	CMOS_WRITE(0, DS_REG_CTLD);
	year = CMOS_READ(DS_REG_YEAR);
	month = CMOS_READ(DS_REG_MONTH);
	date = CMOS_READ(DS_REG_DATE);
	hour = CMOS_READ(DS_REG_HOUR);
	min = CMOS_READ(DS_REG_MIN);
	sec = CMOS_READ(DS_REG_SEC);
	year = year%16 + year/16*10;
	month = month%16 + month/16*10;
	date = date%16 + date/16*10;
	hour = hour%16 + hour/16*10;
	min = min%16 + min/16*10;
	sec = sec%16 + sec/16*10;
	CMOS_WRITE(DS_CTLB_24 | DS_CTLB_DM, DS_REG_CTLB);
	tgt_printf("RTC: %02d-%02d-%02d %02d:%02d:%02d\n", year, month, date, hour, min, sec);

}

int word_addr;

static inline unsigned char CMOS_READ(unsigned char addr)
{
	unsigned char val;
	unsigned char tmp1,tmp2;
	volatile int tmp;

#if defined(DEVBD2F_VIA)||defined(DEVBD2F_CS5536)||defined(DEVBD2F_EVA)
	linux_outb_p(addr, 0x70);
	val = linux_inb_p(0x71);
#elif defined(DEVBD2F_SM502)

	pcitag_t tag;
	unsigned char value;
	char i2caddr[]={(unsigned char)0x64};
	if(addr >= 0x0a)
		return 0;
	switch(addr)
	{
		case 0:
			break;
		case 2:
			addr = 1;
			break;
		case 4:
			addr = 2;
			break;
		case 6:
			addr = 3;
			break;
		case 7:
			addr = 4;
			break;
		case 8:
			addr = 5;
			break;
		case 9:
			addr = 6;
			break;

	}

	tgt_i2cread(I2C_SINGLE,i2caddr,1,0xe<<4,&value,1);
	value = value|0x20;
	tgt_i2cwrite(I2C_SINGLE,i2caddr,1,0xe<<4,&value,1);
	tgt_i2cread(I2C_SINGLE,i2caddr,1,addr<<4,&val,1);
	tmp1 = ((val>>4)&0x0f)*10;
	tmp2  = val&0x0f;
	val = tmp1 + tmp2;

#elif defined(DEVBD2F_FIREWALL)
	char i2caddr[]={0xde,0};
	if(addr >= 0x0a)
		return 0;
	switch(addr)
	{
		case 0:
			addr= 0x30;
			break;
		case 2:
			addr = 0x31;
			break;
		case 4:
			addr = 0x32;
			break;
		case 6:
			addr = 0x36;
			break;
		case 7:
			addr = 0x33;
			break;
		case 8:
			addr = 0x34;
			break;
		case 9:
			addr = 0x35;
			break;

	}
#ifndef GPIO_I2C
	//atp8620_i2c_read(0xde,addr,&tmp,1);
#else	
	tgt_i2cread(I2C_SINGLE,i2caddr,2,addr,&tmp,1);
#endif
	if(addr == 0x32)
		tmp = tmp&0x7f;
	val = ((tmp>>4)&0x0f)*10;
	val = val + (tmp&0x0f);
#endif

	return val;
}

static inline void CMOS_WRITE(unsigned char val, unsigned char addr)
{

	char a;
#if defined(DEVBD2F_VIA)||defined(DEVBD2F_CS5536)||defined(DEVBD2F_EVA)
	linux_outb_p(addr, 0x70);
	linux_outb_p(val, 0x71);

#elif defined(DEVBD2F_SM502)

	unsigned char tmp1,tmp2;
	volatile int tmp;
	char i2caddr[]={(unsigned char)0x64};
	tmp1 = (val/10)<<4;
	tmp2  = (val%10);
	val = tmp1|tmp2;
	if(addr >=0x0a)
		return 0;
	switch(addr)
	{
		case 0:
			break;
		case 2:
			addr = 1;
			break;
		case 4:
			addr = 2;
			break;
		case 6:
			addr = 3;
			break;
		case 7:
			addr = 4;
			break;
		case 8:
			addr = 5;
			break;
		case 9:
			addr = 6;
			break;

	}
	{
		unsigned char value;

		tgt_i2cread(I2C_SINGLE,i2caddr,1,0xe<<4,&value,1);
		value = value|0x20;
		tgt_i2cwrite(I2C_SINGLE,i2caddr,1,0xe<<4,&value,1);
		tgt_i2cwrite(I2C_SINGLE,i2caddr,1,addr<<4,&val,1);
	}

#elif defined(DEVBD2F_FIREWALL)
	unsigned char tmp;
	char i2caddr[]={0xde,0,0};
	if(addr >= 0x0a)
		return 0;
	switch(addr)
	{
		case 0:
			addr= 0x30;
			break;
		case 2:
			addr = 0x31;
			break;
		case 4:
			addr = 0x32;
			break;
		case 6:
			addr = 0x36;
			break;
		case 7:
			addr = 0x33;
			break;
		case 8:
			addr = 0x34;
			break;
		case 9:
			addr = 0x35;
			break;

	}

	tmp = (val/10)<<4;
	val = tmp|(val%10);
#ifndef GPIO_I2C
	atp8620_i2c_write(0xde,addr,&val,1);
#else	
	a = 2;
	tgt_i2cwrite(I2C_SINGLE,i2caddr,2,0x3f,&a,1);
	a = 6;
	tgt_i2cwrite(I2C_SINGLE,i2caddr,2,0x3f,&a,1);
	tgt_i2cwrite(I2C_SINGLE,i2caddr,2,addr,&tmp,1);
#endif
#endif
}

	static void
_probe_frequencies()
{
#ifdef HAVE_TOD
	int i, timeout, cur, sec, cnt;
#endif

	SBD_DISPLAY ("FREQ", CHKPNT_FREQ);

#if 0
	md_pipefreq = 300000000;        /* Defaults */
	md_cpufreq  = 66000000;
#else
	md_pipefreq = 660000000;        /* NB FPGA*/
	md_cpufreq  =  60000000;
#endif

	clk_invalid = 1;
#ifdef HAVE_TOD
#ifdef DEVBD2F_FIREWALL 
	{
		extern void __main();
		char tmp;
		char i2caddr[]={0xde};
		tgt_i2cread(I2C_SINGLE,i2caddr,2,0x3f,&tmp,1);
		/*
		 * bit0:Battery is run out ,please replace the rtc battery
		 * bit4:Rtc oscillator is no operating,please reset the machine
		 */
		tgt_printf("0x3f value  %x\n",tmp);
		init_legacy_rtc();
		tgt_i2cread(I2C_SINGLE,i2caddr,2,0x14,&tmp,1);
		tgt_printf("0x14 value  %x\n",tmp);
	}
#else
	init_legacy_rtc();
#endif

	SBD_DISPLAY ("FREI", CHKPNT_FREQ);

	/*
	 * Do the next twice for two reasons. First make sure we run from
	 * cache. Second make sure synched on second update. (Pun intended!)
	 */
aa:
	for(i = 2;  i != 0; i--) {
		cnt = CPU_GetCOUNT();
		timeout = 10000000;
		while(CMOS_READ(DS_REG_CTLA) & DS_CTLA_UIP);

		sec = CMOS_READ(DS_REG_SEC);

		do {
			timeout--;

			while(CMOS_READ(DS_REG_CTLA) & DS_CTLA_UIP);
			cur = CMOS_READ(DS_REG_SEC);
			//	} while(timeout != 0 && ((cur == sec)||(cur !=((sec+1)%60))) && (CPU_GetCOUNT() - cnt<0x30000000));
	} while(timeout != 0 && (cur == sec));
	cnt = CPU_GetCOUNT() - cnt;
	if(timeout == 0) {
		tgt_printf("time out!\n");
		break;          /* Get out if clock is not running */
	}
}

/*
 *  Calculate the external bus clock frequency.
 */
if (timeout != 0) {
	clk_invalid = 0;
	md_pipefreq = cnt / 10000;
	md_pipefreq *= 20000;
	/* we have no simple way to read multiplier value
	 */
	md_cpufreq = 66000000;
}
tgt_printf("cpu fre %u\n",md_pipefreq);                                                                      
#endif /* HAVE_TOD */
}


/*
 *   Returns the CPU pipelie clock frequency
 */
	int
tgt_pipefreq()
{
	if(md_pipefreq == 0) {
		_probe_frequencies();
	}
	return(md_pipefreq);
}

/*
 *   Returns the external clock frequency, usually the bus clock
 */
	int
tgt_cpufreq()
{
	if(md_cpufreq == 0) {
		_probe_frequencies();
	}
	return(md_cpufreq);
}

	time_t
tgt_gettime()
{
	struct tm tm;
	int ctrlbsave;
	time_t t;
	int year, month, date, hour, min, sec, wday;
	/*gx 2005-01-17 */
	//return 0;

#ifdef HAVE_TOD
	if(!clk_invalid) {
		ctrlbsave = CMOS_READ(DS_REG_CTLB);
		CMOS_WRITE(ctrlbsave | DS_CTLB_SET, DS_REG_CTLB);
		year = CMOS_READ(DS_REG_YEAR);
		month = CMOS_READ(DS_REG_MONTH);
		month = CMOS_READ(DS_REG_MONTH);
		date = CMOS_READ(DS_REG_DATE);
		wday = CMOS_READ(DS_REG_WDAY);
		hour = CMOS_READ(DS_REG_HOUR);
		min = CMOS_READ(DS_REG_MIN);
		sec = CMOS_READ(DS_REG_SEC);

		year = year%16 + year/16*10;
		if(year < 50) year += 100;
		month = (month%16 + month/16*10) - 1;
		wday = wday%16 + wday/16*10;
		date = date%16 + date/16*10;
		hour = hour%16 + hour/16*10;
		min = min%16 + min/16*10;
		sec = sec%16 + sec/16*10;
		tm.tm_sec = sec;
		tm.tm_min = min;
		tm.tm_hour = hour;
		tm.tm_wday = wday;
		tm.tm_mday = date;
		tm.tm_mon = month;
		tm.tm_year = year;

		CMOS_WRITE(ctrlbsave & ~DS_CTLB_SET, DS_REG_CTLB);

		tm.tm_isdst = tm.tm_gmtoff = 0;
		t = gmmktime(&tm);
	}
	else 
#endif
	{
		t = 957960000;  /* Wed May 10 14:00:00 2000 :-) */
	}
	return(t);

}

char gpio_i2c_settime(struct tm *tm);

/*
 *  Set the current time if a TOD clock is present
 */
	void
tgt_settime(time_t t)
{
	struct tm *tm;
	int ctrlbsave;
#ifdef HAVE_TOD
	if(!clk_invalid) {
		tm = gmtime(&t);
#ifndef DEVBD2F_FIREWALL
		ctrlbsave = CMOS_READ(DS_REG_CTLB);
		CMOS_WRITE(ctrlbsave | DS_CTLB_SET, DS_REG_CTLB);

		CMOS_WRITE((((tm->tm_year) % 100)/10*16 + ((tm->tm_year) % 100)%10), DS_REG_YEAR);
		CMOS_WRITE((((tm->tm_mon) + 1)/10*16 + ((tm->tm_mon) + 1)%10), DS_REG_MONTH);
		CMOS_WRITE(tm->tm_mday/10*16 + tm->tm_mday%10, DS_REG_DATE);
		CMOS_WRITE(tm->tm_wday/10*16 + tm->tm_wday%10, DS_REG_WDAY);
		CMOS_WRITE(tm->tm_hour/10*16 + tm->tm_hour%10, DS_REG_HOUR);
		CMOS_WRITE(tm->tm_min/10*16 + tm->tm_min%10, DS_REG_MIN);
		CMOS_WRITE(tm->tm_sec/10*16 + tm->tm_sec%10, DS_REG_SEC);

		CMOS_WRITE(ctrlbsave & ~DS_CTLB_SET, DS_REG_CTLB);
#else
		gpio_i2c_settime(tm);	
#endif
	}
#endif

}


/*
 *  Print out any target specific memory information
 */
	void
tgt_memprint()
{
	printf("Primary Instruction cache size %dkb (%d line, %d way)\n",
			CpuPrimaryInstCacheSize / 1024, CpuPrimaryInstCacheLSize, CpuNWayCache);
	printf("Primary Data cache size %dkb (%d line, %d way)\n",
			CpuPrimaryDataCacheSize / 1024, CpuPrimaryDataCacheLSize, CpuNWayCache);
	if(CpuSecondaryCacheSize != 0) {
		printf("Secondary cache size %dkb\n", CpuSecondaryCacheSize / 1024);
	}
	if(CpuTertiaryCacheSize != 0) {
		printf("Tertiary cache size %dkb\n", CpuTertiaryCacheSize / 1024);
	}
}

	void
tgt_machprint()
{
	printf("Copyright 2000-2002, Opsycon AB, Sweden.\n");
	printf("Copyright 2005, ICT CAS.\n");
	printf("CPU %s @", md_cpuname());
} 

/*
 *  Return a suitable address for the client stack.
 *  Usually top of RAM memory.
 */

register_t
tgt_clienttos()
{
	extern char start[];
	return(register_t)(int)PHYS_TO_CACHED(start - 64);
}

#ifdef HAVE_FLASH
/*
 *  Flash programming support code.
 */

/*
 *  Table of flash devices on target. See pflash_tgt.h.
 */

struct fl_map tgt_fl_map_boot16[]={
	TARGET_FLASH_DEVICES_16
};


	struct fl_map *
tgt_flashmap()
{
	return tgt_fl_map_boot16;
}   
	void
tgt_flashwrite_disable()
{
}

	int
tgt_flashwrite_enable()
{
	return(1);
}

	void
tgt_flashinfo(void *p, size_t *t)
{
	struct fl_map *map;

	map = fl_find_map(p);
	if(map) {
		*t = map->fl_map_size;
	}
	else {
		*t = 0;
	}
}

	void
tgt_flashprogram(void *p, int size, void *s, int endian)
{
	printf("Programming flash %x:%x into %x\n", s, size, p);
	if(fl_erase_device(p, size, TRUE)) {
		printf("Erase failed!\n");
		return;
	}
	if(fl_program_device(p, s, size, TRUE)) {
		printf("Programming failed!\n");
	}
	fl_verify_device(p, s, size, TRUE);
}
#endif /* PFLASH */

/*
 *  Network stuff.
 */
	void
tgt_netinit()
{
}

	int
tgt_ethaddr(char *p)
{
	bcopy((void *)&hwethadr, p, 6);
	return(0);
}

	void
tgt_netreset()
{
}

/*************************************************************************/
/*
 *	Target dependent Non volatile memory support code
 *	=================================================
 *
 *
 *  On this target a part of the boot flash memory is used to store
 *  environment. See EV64260.h for mapping details. (offset and size).
 */

/*
 *  Read in environment from NV-ram and set.
 */
	void
tgt_mapenv(int (*func) __P((char *, char *)))
{
	char *ep;
	char env[512];
	char *nvram;
	int i;

	/*
	 *  Check integrity of the NVRAM env area. If not in order
	 *  initialize it to empty.
	 */
	tgt_printf("in envinit\n");
#ifdef NVRAM_IN_FLASH
	nvram = (char *)(tgt_flashmap())->fl_map_base;
	tgt_printf("nvram=%08x\n",(unsigned int)nvram);
	if(fl_devident(nvram, NULL) == 0 ||
			cksum(nvram + NVRAM_OFFS, NVRAM_SIZE, 0) != 0) {
#else
		nvram = (char *)malloc(512);
		nvram_get(nvram);
		if(cksum(nvram, NVRAM_SIZE, 0) != 0) {
#endif
			tgt_printf("NVRAM is invalid!\n");
			nvram_invalid = 1;
		}
		else {
			nvram += NVRAM_OFFS;
			ep = nvram+2;;

			while(*ep != 0) {
				char *val = 0, *p = env;
				i = 0;
				while((*p++ = *ep++) && (ep <= nvram + NVRAM_SIZE - 1) && i++ < 255) {
					if((*(p - 1) == '=') && (val == NULL)) {
						*(p - 1) = '\0';
						val = p;
					}
				}
				if(ep <= nvram + NVRAM_SIZE - 1 && i < 255) {
					(*func)(env, val);
				}
				else {
					nvram_invalid = 2;
					break;
				}
			}
		}

		tgt_printf("NVRAM@%x\n",(u_int32_t)nvram);

		/*
		 *  Ethernet address for Galileo ethernet is stored in the last
		 *  six bytes of nvram storage. Set environment to it.
		 */
		bcopy(&nvram[ETHER_OFFS], hwethadr, 6);
		sprintf(env, "%02x:%02x:%02x:%02x:%02x:%02x", hwethadr[0], hwethadr[1],
				hwethadr[2], hwethadr[3], hwethadr[4], hwethadr[5]);
		(*func)("ethaddr", env);

		if(nvram[NOMSG_OFFS]==0x5a)
		{
		extern int nomsg_on_serial;
		(*func)("NOMSG_ON_SERIAL","1");
		nomsg_on_serial=1;
		}

		bcopy(&nvram[WDT_OFFS], &wdt, 2);
		sprintf(env, "%d", (wdt==0||wdt==0xffff)?300:wdt);
		(*func)("wdt", env);

#ifndef NVRAM_IN_FLASH
		free(nvram);
#endif

#ifdef no_thank_you
		(*func)("vxWorks", env);
#endif


		sprintf(env, "%d", memorysize / (1024 * 1024));
		(*func)("memsize", env);

		sprintf(env, "%d", memorysize_high / (1024 * 1024));
		(*func)("highmemsize", env);

		sprintf(env, "%d", memorysize_high_n1 / (1024 * 1024));
		(*func)("memorysize_high_n1", env);

		sprintf(env, "%d", memorysize_high_n2 / (1024 * 1024));
		(*func)("memorysize_high_n2", env);

		sprintf(env, "%d", memorysize_high_n3 / (1024 * 1024));
		(*func)("memorysize_high_n3", env);

		sprintf(env, "%d", md_pipefreq);
		(*func)("cpuclock", env);

		sprintf(env, "%d", md_cpufreq);
		(*func)("busclock", env);

		sprintf(env, "%d",VRAM_SIZE);
		(*func)("vramsize", env);

#ifdef CONFIG_GFXUMA
	sprintf(env, "%d",1);
#else
	sprintf(env, "%d",0);
#endif
	(*func)("sharevram", env);


		(*func)("systype", SYSTYPE);

	}

	int
		tgt_unsetenv(char *name)
		{
			char *ep, *np, *sp;
			char *nvram;
			char *nvrambuf;
			char *nvramsecbuf;
			int status;

			if(nvram_invalid) {
				return(0);
			}

			/* Use first defined flash device (we probably have only one) */
#ifdef NVRAM_IN_FLASH
			nvram = (char *)(tgt_flashmap())->fl_map_base;

			/* Map. Deal with an entire sector even if we only use part of it */
			nvram += NVRAM_OFFS & ~(NVRAM_SECSIZE - 1);
			nvramsecbuf = (char *)malloc(NVRAM_SECSIZE);
			if(nvramsecbuf == 0) {
				printf("Warning! Unable to malloc nvrambuffer!\n");
				return(-1);
			}
			memcpy(nvramsecbuf, nvram, NVRAM_SECSIZE);
			nvrambuf = nvramsecbuf + (NVRAM_OFFS & (NVRAM_SECSIZE - 1));
#else
			nvramsecbuf = nvrambuf = nvram = (char *)malloc(512);
			nvram_get(nvram);
#endif

			ep = nvrambuf + 2;

			status = 0;
			while((*ep != '\0') && (ep <= nvrambuf + NVRAM_SIZE)) {
				np = name;
				sp = ep;

				while((*ep == *np) && (*ep != '=') && (*np != '\0')) {
					ep++;
					np++;
				}
				if((*np == '\0') && ((*ep == '\0') || (*ep == '='))) {
					while(*ep++);
					while(ep <= nvrambuf + NVRAM_SIZE) {
						*sp++ = *ep++;
					}
					if(nvrambuf[2] == '\0') {
						nvrambuf[3] = '\0';
					}
					cksum(nvrambuf, NVRAM_SIZE, 1);
#ifdef NVRAM_IN_FLASH
					if(fl_erase_device(nvram, NVRAM_SECSIZE, FALSE)) {
						status = -1;
						break;
					}

					if(fl_program_device(nvram, nvramsecbuf, NVRAM_SECSIZE, FALSE)) {
						status = -1;
						break;
					}
#else
					nvram_put(nvram);
#endif
					status = 1;
					break;
				}
				else if(*ep != '\0') {
					while(*ep++ != '\0');
				}
			}

			free(nvramsecbuf);
			return(status);
		}

	int
		tgt_setenv(char *name, char *value)
		{
			char *ep;
			int envlen;
			char *nvrambuf;
			char *nvramsecbuf;
#ifdef NVRAM_IN_FLASH
			char *nvram;
#endif

			/* Non permanent vars. */
			if(strcmp(EXPERT, name) == 0) {
				return(1);
			}

			/* Calculate total env mem size requiered */
			envlen = strlen(name);
			if(envlen == 0) {
				return(0);
			}
			if(value != NULL) {
				envlen += strlen(value);
			}
			envlen += 2;    /* '=' + null byte */
			if(envlen > 255) {
				return(0);      /* Are you crazy!? */
			}

			/* Use first defined flash device (we probably have only one) */
#ifdef NVRAM_IN_FLASH
			nvram = (char *)(tgt_flashmap())->fl_map_base;

			/* Deal with an entire sector even if we only use part of it */
			nvram += NVRAM_OFFS & ~(NVRAM_SECSIZE - 1);
#endif

			/* If NVRAM is found to be uninitialized, reinit it. */
			if(nvram_invalid) {
				nvramsecbuf = (char *)malloc(NVRAM_SECSIZE);
				if(nvramsecbuf == 0) {
					printf("Warning! Unable to malloc nvrambuffer!\n");
					return(-1);
				}
#ifdef NVRAM_IN_FLASH
				memcpy(nvramsecbuf, nvram, NVRAM_SECSIZE);
#endif
				nvrambuf = nvramsecbuf + (NVRAM_OFFS & (NVRAM_SECSIZE - 1));
				memset(nvrambuf, -1, NVRAM_SIZE);
				nvrambuf[2] = '\0';
				nvrambuf[3] = '\0';
				cksum((void *)nvrambuf, NVRAM_SIZE, 1);
				printf("Warning! NVRAM checksum fail. Reset!\n");
#ifdef NVRAM_IN_FLASH
				if(fl_erase_device(nvram, NVRAM_SECSIZE, FALSE)) {
					printf("Error! Nvram erase failed!\n");
					free(nvramsecbuf);
					return(-1);
				}
				if(fl_program_device(nvram, nvramsecbuf, NVRAM_SECSIZE, FALSE)) {
					printf("Error! Nvram init failed!\n");
					free(nvramsecbuf);
					return(-1);
				}
#else
				nvram_put(nvramsecbuf);
#endif
				nvram_invalid = 0;
				free(nvramsecbuf);
			}

			/* Remove any current setting */
			tgt_unsetenv(name);

			/* Find end of evironment strings */
			nvramsecbuf = (char *)malloc(NVRAM_SECSIZE);
			if(nvramsecbuf == 0) {
				printf("Warning! Unable to malloc nvrambuffer!\n");
				return(-1);
			}
#ifndef NVRAM_IN_FLASH
			nvram_get(nvramsecbuf);
#else
			memcpy(nvramsecbuf, nvram, NVRAM_SECSIZE);
#endif
			nvrambuf = nvramsecbuf + (NVRAM_OFFS & (NVRAM_SECSIZE - 1));
			/* Etheraddr is special case to save space */
			if (strcmp("ethaddr", name) == 0) {
				char *s = value;
				int i;
				int32_t v;
				for(i = 0; i < 6; i++) {
					gethex(&v, s, 2);
					hwethadr[i] = v;
					s += 3;         /* Don't get to fancy here :-) */
				} 
			}
			else if (strcmp("NOMSG_ON_SERIAL", name) == 0) {
			nvramsecbuf[NOMSG_OFFS]=(value[0]=='0')?0xff:0x5a;
			} else if (strcmp("wdt", name) == 0) {
				wdt = strtoul(value,0,0);
			} else {
				ep = nvrambuf+2;
				if(*ep != '\0') {
					do {
						while(*ep++ != '\0');
					} while(*ep++ != '\0');
					ep--;
				}
				if(((int)ep + NVRAM_SIZE - (int)ep) < (envlen + 1)) {
					free(nvramsecbuf);
					return(0);      /* Bummer! */
				}

				/*
				 *  Special case heaptop must always be first since it
				 *  can change how memory allocation works.
				 */
				if(strcmp("heaptop", name) == 0) {

					bcopy(nvrambuf+2, nvrambuf+2 + envlen,
							ep - nvrambuf+1);

					ep = nvrambuf+2;
					while(*name != '\0') {
						*ep++ = *name++;
					}
					if(value != NULL) {
						*ep++ = '=';
						while((*ep++ = *value++) != '\0');
					}
					else {
						*ep++ = '\0';
					}
				}
				else {
					while(*name != '\0') {
						*ep++ = *name++;
					}
					if(value != NULL) {
						*ep++ = '=';
						while((*ep++ = *value++) != '\0');
					}
					else {
						*ep++ = '\0';
					}
					*ep++ = '\0';   /* End of env strings */
				}
			}
			cksum(nvrambuf, NVRAM_SIZE, 1);

			bcopy(hwethadr, &nvramsecbuf[ETHER_OFFS], 6);
			bcopy(&wdt, &nvramsecbuf[WDT_OFFS], 2);
#ifdef NVRAM_IN_FLASH
			if(fl_erase_device(nvram, NVRAM_SECSIZE, FALSE)) {
				printf("Error! Nvram erase failed!\n");
				free(nvramsecbuf);
				return(0);
			}
			if(fl_program_device(nvram, nvramsecbuf, NVRAM_SECSIZE, FALSE)) {
				printf("Error! Nvram program failed!\n");
				free(nvramsecbuf);
				return(0);
			}
#else
			nvram_put(nvramsecbuf);
#endif
			free(nvramsecbuf);
			return(1);
		}



	/*
	 *  Calculate checksum. If 'set' checksum is calculated and set.
	 */
	static int
		cksum(void *p, size_t s, int set)
		{
			u_int16_t sum = 0;
			u_int8_t *sp = p;
			int sz = s / 2;

			if(set) {
				*sp = 0;	/* Clear checksum */
				*(sp+1) = 0;	/* Clear checksum */
			}
			while(sz--) {
				sum += (*sp++) << 8;
				sum += *sp++;
			}
			if(set) {
				sum = -sum;
				*(u_int8_t *)p = sum >> 8;
				*((u_int8_t *)p+1) = sum;
			}
			return(sum);
		}

#ifndef NVRAM_IN_FLASH

	/*
	 *  Read and write data into non volatile memory in clock chip.
	 */
	void
		nvram_get(char *buffer)
		{
			int i;
			for(i = 0; i < 114; i++) {
				linux_outb(i + RTC_NVRAM_BASE, RTC_INDEX_REG);	/* Address */
				buffer[i] = linux_inb(RTC_DATA_REG);
			}
		}

	void
		nvram_put(char *buffer)
		{
			int i;
			for(i = 0; i < 114; i++) {
				linux_outb(i+RTC_NVRAM_BASE, RTC_INDEX_REG);	/* Address */
				linux_outb(buffer[i],RTC_DATA_REG);
			}
		}

#endif

	/*
	 *  Simple display function to display a 4 char string or code.
	 *  Called during startup to display progress on any feasible
	 *  display before any serial port have been initialized.
	 */
	void
		tgt_display(char *msg, int x)
		{
			/* Have simple serial port driver */
			tgt_putchar(msg[0]);
			tgt_putchar(msg[1]);
			tgt_putchar(msg[2]);
			tgt_putchar(msg[3]);
			tgt_putchar('\r');
			tgt_putchar('\n');
		}

	void
		clrhndlrs()
		{
		}

	int
		tgt_getmachtype()
		{
			return(md_cputype());
		}

	/*
	 *  Create stubs if network is not compiled in
	 */
#ifdef INET
	void
		tgt_netpoll()
		{
			splx(splhigh());
		}

#else
	extern void longjmp(label_t *, int);
	void gsignal(label_t *jb, int sig);
	void
		gsignal(label_t *jb, int sig)
		{
			if(jb != NULL) {
				longjmp(jb, 1);
			}
		};

	int	netopen (const char *, int);
	int	netread (int, void *, int);
	int	netwrite (int, const void *, int);
	long	netlseek (int, long, int);
	int	netioctl (int, int, void *);
	int	netclose (int);
	int netopen(const char *p, int i)	{ return -1;}
	int netread(int i, void *p, int j)	{ return -1;}
	int netwrite(int i, const void *p, int j)	{ return -1;}
	int netclose(int i)	{ return -1;}
	long int netlseek(int i, long j, int k)	{ return -1;}
	int netioctl(int j, int i, void *p)	{ return -1;}
	void tgt_netpoll()	{};

#endif /*INET*/

#define SPINSZ		0x800000
#define DEFTESTS	7
#define MOD_SZ		20
#define BAILOUT		if (bail) goto skip_test;
#define BAILR		if (bail) return;

	/* memspeed operations */
#define MS_BCOPY	1
#define MS_COPY		2
#define MS_WRITE	3
#define MS_READ		4
#include "mycmd.c"

#ifdef DEVBD2F_FIREWALL
#include "i2c-sm502.c"
#elif defined(DEVBD2F_SM502)
#include "i2c-sm502.c"

#elif (PCI_IDSEL_CS5536 != 0)
#include "i2c-cs5536.c"
#else
#include "i2c-via.c"
#endif

char *tran_month(char *c, char *i)
 {
     switch (*c++){
     case  'J':
         if(*c++ == 'a')     /* Jan */
             i = "01";
         else if(*c++ == 'n')    /* June */
             i = "06";
         else                /* July */
             i = "07";
         break;
     case  'F':              /* Feb */
         i = "02";
         break;
     case  'M':
         c++;
         if(*c++ == 'r')     /* Mar */
             i = "03";
         else                /* May */
             i = "05";
         break;
     case  'A':
         if(*c++ == 'p')     /* Apr */
             i = "04";
         else                /* Aug */
             i = "08";
         break;
         case  'S':              /* Sept */
         i = "09";
         break;
     case  'O':              /* Oct */
         i = "10";
         break;
     case  'N':              /* Nov */
         i = "11";
         break;
     case  'D':              /* Dec */
         i = "12";
         break;
     default :
         i = NULL;
     }

     return i;
 }

int get_update(char *p)
 {
     int i=0;
     char *t,*mp,m[3];

     t  = strstr(vers, ":");
     strncpy(p, t+26, 4);     /* year */
     p[4] = '-';
     mp = tran_month(t+6, m);    /* month */
     strncpy(p+5,mp,2);
     p[7]='-';
     strncpy(p+8, t+10, 2);   /* day */
     p[10] = '\0';

     return 0;
 }

/********************************************
*     PCI/PCIE device interrupt talbe      **
*********************************************
BUS:DEV:FUN 		 interrupt line
01:05:00			    6
02:00:00			    3
03:00:00			    3
04:00:00			    3
05:00:00			    3
07:00:00			    5
0a:04:00			    3
0a:05:00			    3
*******************************************/

/*
﻿PCI IRQ Routing Index 
0 – INTA#	 
1 – INTB#	 
2 – INTC#	 
3 – INTD#	 
4 – SCI	   
5 – SMBus interrupt 
9 – INTE#	 
0Ah – INTF#	   
0Bh – INTG#	   
0Ch – INTH#	   

﻿0 ~ 15 : IRQ0 to IRQ15   
IRQ0, 2, 8, 13 are reserved	   
*/

#define SMBUS_IO_BASE	  0x0000  //ynn
#define PCI_BRADGE_TOTAL  0x0001  //ynn


#define DEBUG_FIXINTERRUPT
#ifdef DEBUG_FIXINTERRUPT
#define fixup_interrupt_printf(format, args...) printf(format, ##args)
#else
#define fixup_interrupt_printf(format, args...)	 __asm__ __volatile__("sync")
#endif

static char irqbus0[] = 
{	
  [1] = 2,
  [2] = 2,
  [3] = 3,
  [4] = 0,
  [5] = 1,
  [6] = 2,
  [7] = 3,
  [9] = 1,
  [10] = 2,
  [20] = 9, //inte
}; 

/*pci int route on develop board:
adxx: devno = xx - 16
ad19: inte
ad20: intf
*/
static int bus0dev20[16] = {
[0] = 10,
[1] = 11,
[2] = 12,
[3] = 9,
[4] = 10,
[5] = 11,
[6] = 12,
[7] = 9,
[8] = 10,
[9] = 11,
[10] = 12,
[11] = 9,
[12] = 9,
[13] = 10,
[14] = 11,
[15] = 12, //inth
};

char irqroute[16];

char val4d0;
char val4d1;

static void pci_fix_device_interrpt(struct pci_device *pd, int bus0tag)
{
	pcitag_t tag;
	int irq;
	volatile unsigned char * pic_index = 0xb8000c00 + SMBUS_IO_BASE; 
	volatile unsigned char * pic_data =  0xb8000c01 + SMBUS_IO_BASE;
	tag = pd->pa.pa_tag;

	if(pd->bridge.child)
	{
		for(pd = pd->bridge.child; pd; pd = pd->next) {
			pci_fix_device_interrpt(pd, bus0tag);
		}
	}
	else
	{
		int bus, dev , func, icr, pin;
		_pci_break_tag(tag, &bus, &dev, &func);
		icr = _pci_conf_read(tag, 0x3c);

		if((pin=((icr>>8)&0xff)))
		{
			int bus0, bus0dev, bus0func;
			_pci_break_tag(bus0tag, &bus0, &bus0dev, &bus0func);
                        if(bus0dev == 20)
                           irq = (bus0dev20[dev]+pin-1);
			else
                           irq = (irqbus0[bus0dev]+pin-1);
			if(!irqroute[irq])
			{
			  printf("nomap for busdev %d pin %d irqpin %d\n", bus0dev ,pin ,irq);
				irqroute[irq] = 3;
				*pic_index =  irq;
				*pic_data = irqroute[irq];
			}

			printf("fixup bus %d dev %d func %d pin %d irq %d new irq %d bus0dev %d\n", bus, dev, func, pin, icr&0xff, irqroute[irq], bus0dev);

			if(irqroute[irq]>=8)
			 val4d1 |= (1<<(irqroute[irq]-8));
			else val4d0 |= (1<<irqroute[irq]);

			_pci_conf_write(tag, 0x3c, (icr&~0xff)|irqroute[irq]);
		}

	}
}


void sb700_interrupt_fixup1(void)
{
	int i;
	struct pci_device *pd;
	int bus, dev , func;
	pcitag_t tag;
	extern struct pci_device *_pci_head;
	volatile unsigned char * pic_index = 0xb8000c00 + SMBUS_IO_BASE; 
	volatile unsigned char * pic_data =  0xb8000c01 + SMBUS_IO_BASE;
	//c00,c01
	char tmpirq = 8;

	for(i=0;i<16;i++)
	{
		* pic_index = i;
		irqroute[i] = * pic_data;
		printf("irqroute[%d]= %d\n", i, irqroute[i]);
	}


	printf("_pci_head=%p,child=%p,next=%p\n",_pci_head,_pci_head->bridge.child, _pci_head->next);
	for(i = 0, pd = _pci_head->bridge.child; pd; i++, pd = pd->next) {
		tag = pd->pa.pa_tag;
		if(pd->bridge.child)
		{
			pci_fix_device_interrpt(pd,tag);			
		}
	}

	
	*(volatile char *)(0xb80004d0) |= val4d0;
	*(volatile char *)(0xb80004d1) |= val4d1;
}


#define MULTY_FUNCTION (0x1 << 7)

void sb700_interrupt_fixup0(void)
{
	volatile unsigned char * pic_index = 0xb8000c00 + SMBUS_IO_BASE;
	volatile unsigned char * pic_data =  0xb8000c01 + SMBUS_IO_BASE;
	volatile unsigned short * intr_contrl =  0xb80004d0 + SMBUS_IO_BASE;
	unsigned short busnum, funnum;
	unsigned short origin_busnum;

	device_t dev,dev1;
	u32 val, tmp;
	u8 byte;

	//0.  smubs fixup
	fixup_interrupt_printf("\n-----------------godson3a_smbus_fixup---------------\n");

	/* Set SATA and PATA Controller to combined mode
	 * Port0-Port3 is SATA mode, Port4-Port5 is IDE mode */
	dev = _pci_make_tag(0, 0x14, 0x0);

	/*1. usb interrupt map smbus reg:0XBE  map usbint1map usbint3map(ohci use) to PCI_INTC#
	  map usbint2map usbint4map(ehci use) to PCI_INTC# */
	pci_write_config16(dev, 0xbe, ((2<<0)|(2 << 3)|(2 << 8)|(2 << 11)) );
	val = pci_read_config16(dev, 0xbe);
	fixup_interrupt_printf(" set smbus reg (0xbe) :%x (usb intr map)\n", val);

	/*2. sata interrupt map smbus reg:0Xaf map sataintmap to PCI_INTH#*/
	pci_write_config8(dev, 0xaf,  0x1c);
	byte = pci_read_config8(dev, 0xaf);
	fixup_interrupt_printf(" set smbus reg (0xaf) :%x (sata intr map)\n", byte);

#if 0 // Lc modify
	/* Set SATA and PATA Controller to combined mode
	 * Port0-Port3 is SATA mode, Port4-Port5 is IDE mode
	 */
	byte = pci_read_config8(dev, 0xad);
	byte |= 0x1<<3;
	byte &= ~(0x1<<4);
	pci_write_config8(dev, 0xad,  byte);

	/* Map the HDA interrupt to INTE */
	byte = pci_read_config8(dev, 0x63);
	byte &= 0xf8;
	pci_write_config8(dev, 0x63, byte|0x4);

	/* Set GPIO42, GPIO43, GPIO44, GPIO46 as HD function */
	pci_write_config16(dev, 0xf8, 0x0);
	//pci_write_config16(dev, 0xfc, 0x2<<0 | 0x2 << 2 | 0x2 << 4 | 0x2 << 6);
	pci_write_config16(dev, 0xfc, 0x2 << 0);
#endif

	//Begin to set SB700 interrupt PIC
	fixup_interrupt_printf("SB700 interrupt PIC set begin,  \n");
	/* bus4: INTA -->IRQ5 PCIE_slot(right) */
	*(pic_index) =  0x0;
	*(pic_data) =  0x5;
	if (*(pic_data) !=  0x5)
		fixup_interrupt_printf("set pic fail: read back %d,should be 0x5\n", *(pic_data));
	else
		fixup_interrupt_printf("set pic_5 pass\n");

	/* bus5: INTB -->IRQ5 NIC 82574 on borad  */
	*(pic_index) =  0x1;
	*(pic_data) =  0x5;
	if (*(pic_data) !=  0x5)
		fixup_interrupt_printf("set pic fail: read back %d,should be 0x5\n", *(pic_data));
	else
		fixup_interrupt_printf("set pic_5 pass\n");

	/* INTC -->IRQ6 NIC 82574, ohci0, ohci1, PCIE_slot(left) */
	*(pic_index) =  0x2;
	*(pic_data) =  0x6;
	if (*(pic_data) !=  0x6)
		fixup_interrupt_printf("set pic fail: read back %d,should be 0x6\n", *(pic_data));
	else
		fixup_interrupt_printf("set pic_6 pass\n");

	/* bus7: INTD -->IRQ5  PCIE_slot(middle) */
	*(pic_index) =  0x3;
	*(pic_data) =  0x5;
	if (*(pic_data) !=  0x5)
		fixup_interrupt_printf("set pic fail: read back %d,should be 0x5\n", *(pic_data));
	else
		fixup_interrupt_printf("set pic_5 pass\n");

	/* INTE -->IRQ5 PCI_slot(left) */
	*(pic_index) =  0x9;
	*(pic_data) =  0x5;
	if (*(pic_data) !=  0x5)
		fixup_interrupt_printf("set pic fail: read back %d,should be 0x5\n", *(pic_data));
	else
		fixup_interrupt_printf("set pic_5 pass\n");

	/* bus 10: dev 4: INTF -->IRQ5 PCI_slot(right) */
	*(pic_index) =  0xa;
	*(pic_data) =  0x5;
	if (*(pic_data) !=  0x5)
		fixup_interrupt_printf("set pic fail: read back %d,should be 0x5\n", *(pic_data));
	else
		fixup_interrupt_printf("set pic_a pass\n");

#if 1
	/* bus 7: dev 5: func 2: INTG -->IRQ3 PCI(right) */
	*(pic_index) =  0xb;
	*(pic_data) =  0x3;
	if (*(pic_data) !=  0x3)
		fixup_interrupt_printf("set pic fail: read back %d,should be 0x3\n", *(pic_data));
	else
		fixup_interrupt_printf("set pic_9 pass\n");
#endif

	/* INTH -->IRQ5 SATA */
	*(pic_index) =  0xc;
	*(pic_data) =  0x5;
	if (*(pic_data) !=  0x5)
		fixup_interrupt_printf("set pic fail: read back %d,should be 0x5\n", *(pic_data));
	else
		fixup_interrupt_printf("set pic_5 pass\n");

#ifndef USE_BMC
	/* 5. set int triggle model: level or edge */
	dev = _pci_make_tag(0, 0x14, 0x0);
	fixup_interrupt_printf("PIC control bit: %08x\n", pci_read_config16(dev, 0x64));

	fixup_interrupt_printf("original int mode: 0x%08x \n", (*intr_contrl));
	(*intr_contrl) = ((*intr_contrl)|(1<<6)|(1<<5)|(1<<4)|(1<<3));
	fixup_interrupt_printf("<1> now int mode: 0x%08x \n", (*intr_contrl));
	fixup_interrupt_printf("waiting....\n");
#if 1
	(*intr_contrl) = ((*intr_contrl)|(1<<6)|(1<<5)|(1<<4)|(1<<3));
	fixup_interrupt_printf("<1> now int mode: 0x%08x \n", (*intr_contrl));
#endif

	fixup_interrupt_printf("SB700 interrupt PIC set done \n");
#endif



	// Fix bonito interrupt according fixup_3a780e.c in kernel source code
	fixup_interrupt_printf("SB700 device interrupt route begin \n");

	// 1.fix up 82574:  
	/* NIC 82574 on board (left) bus5 dev0 fun 0*/
	fixup_interrupt_printf("\nrte0 fixup: em0 ---------------> int5\n");
	fixup_interrupt_printf("SB700 device  route em0: int5 \n");

	for( funnum = 0; funnum < 8; funnum++)
	{
		dev = _pci_make_tag(5, 0x0, funnum);
		val = pci_read_config32(dev, 0x00);
		if ( val != 0xffffffff) // device on the slot
			pci_write_config8(dev, 0x3c, 0x05);
	}

	/* NIC 82574 on board (right) bus6 dev0 fun0 */
	fixup_interrupt_printf("SB700 device  route em1: int6 \n");
	for( funnum = 0; funnum < 8; funnum++)
	{
		dev = _pci_make_tag(6, 0x0, funnum);
		val = pci_read_config32(dev, 0x00);
		if ( val != 0xffffffff) // device on the slot
			pci_write_config8(dev, 0x3c, 0x06);
	}

	//2.fixup sata int line
	fixup_interrupt_printf("\n godson3a_sata_fixup: sata ---------------> int5 \n"); /* Lc: use irq num 5, after shoud be 4*/

	/*2.1. enable the subcalss code register for setting sata controller mode*/
	dev = _pci_make_tag(0, 0x11, 0x0);
	val = pci_read_config8(dev, 0x40);
	(void) pci_write_config8(dev, 0x40, (val | 0x01) );

	/*2.2. set sata controller act as AHCI mode
	 *	 sata controller support IDE mode, AHCI mode, Raid mode*/
	(void) pci_write_config8(dev, 0x09, 0x01);
	(void) pci_write_config8(dev, 0x0a, 0x06);

	/*2.3. disable the subcalss code register*/
	val = pci_read_config8(dev, 0x40);
	(void) pci_write_config8(dev, 0x40, val & (~0x01));
	fixup_interrupt_printf("-----------------tset sata------------------\n");
	val = pci_read_config32(dev, 0x40);
	fixup_interrupt_printf("sata pci_config 0x40 (%x)\n", val);
	/*2.4. set interrput vector */
	pci_write_config8(dev, 0x3c, 0x5); 
	fixup_interrupt_printf("godson3a_sata: fix sata mode==:%d\n",pci_read_config8(dev,0x3c));

	/*3. ide fixup*/
	fixup_interrupt_printf("godson3a_ide_fixup: fix ide mode\n");
	dev = _pci_make_tag(0, 0x14, 1); 

	/* enable IDE DMA --Multi-Word DMA */
	pci_write_config32(dev, 0x44, 0x20000000);
	byte = pci_read_config8(dev, 0x54);
	byte |= 1 << 0;
	pci_write_config8(dev, 0x54, byte);

#if 0 
	/*set IDE ultra DMA enable as master and slalve device*/
	(void) pci_write_config8(dev, 0x54, 0xf);
	/*set ultral DAM mode 0~6  we use 6 as high speed !*/
	(void) pci_write_config16(dev, 0x56, (0x6 << 0)|(0x6 << 4)|(0x6 << 8)|(0x6 << 12));
	fixup_interrupt_printf("godson3a_ide_fixup: fix ide mode\n");
#endif

	/*4. usb fixup*/
	fixup_interrupt_printf("godson3a fixup: usb ------> int6 \n");
	dev = _pci_make_tag(0, 0x12, 0); //OHCI0
	pci_write_config8(dev, 0x3c, 0x6);
	fixup_interrupt_printf("godson3a fixup: usb ------> int6 \n");
	dev = _pci_make_tag(0, 0x12, 1); //OHCI1
	pci_write_config8(dev, 0x3c, 0x6);
	fixup_interrupt_printf("godson3a fixup: usb ------> int6 \n");
	dev = _pci_make_tag(0, 0x12, 2); //EHCI
	pci_write_config8(dev, 0x3c, 0x6);
	fixup_interrupt_printf("godson3a fixup: usb ------> int6 \n");
	dev = _pci_make_tag(0, 0x13, 0); //OHCI0
	pci_write_config8(dev, 0x3c, 0x6);
	fixup_interrupt_printf("godson3a fixup: usb ------> int6 \n");
	dev = _pci_make_tag(0, 0x13, 1); //OHCI1
	pci_write_config8(dev, 0x3c, 0x6);
	fixup_interrupt_printf("godson3a fixup: usb ------> int6 \n");
	dev = _pci_make_tag(0, 0x13, 2); //EHCI
	pci_write_config8(dev, 0x3c, 0x6);
	fixup_interrupt_printf("godson3a fixup: usb ------> int6 \n");
	dev = _pci_make_tag(0, 0x14, 5); //USB controller
	pci_write_config8(dev, 0x3c, 0x6);
	fixup_interrupt_printf("godson3a fixup: usb ------> int6 \n");

#if 0
	/*5. lpc fixup*/
	dev = _pci_make_tag(0, 0x14, 3);
	fixup_interrupt_printf("godson3a fixup: lpc ------> int6 \n");
	val = pci_read_config8(dev, 0x46);
	fixup_interrupt_printf("Fixup: lpc: 0x46 value is 0x%x\n",val);
	pci_write_config8(dev, 0x46, val|(0x3 << 6));
	val = pci_read_config8(dev, 0x46);
	fixup_interrupt_printf("Fixup: lpc: 0x47 value is 0x%x\n",val);

	val = pci_read_config8(dev, 0x47);
	fixup_interrupt_printf("Fixup: lpc: 0x47 value is 0x%x\n",val);
	pci_write_config8(dev, 0x47, val|0xff);
	val = pci_read_config8(dev, 0x47);
	fixup_interrupt_printf("Fixup: lpc: 0x47 value is 0x%x\n",val);

	val = pci_read_config8(dev, 0x48);
	fixup_interrupt_printf("Fixup: lpc: 0x48 value is 0x%x\n",val);
	pci_write_config8(dev, 0x48, val|0xff);
	val = pci_read_config8(dev, 0x48);
	fixup_interrupt_printf("Fixup: lpc: 0x48 value is 0x%x\n",val);
#endif

#if 0
	/*6. hda fixup*/
	fixup_interrupt_printf("godson3a fixup: HDA ------> int5 \n");
	dev = _pci_make_tag(0, 0x14, 2);
	pci_write_config8(dev,0x3c,0x05);

	fixup_interrupt_printf("godson3a fixup: HDA ------> int5 \n");
#endif

	/*7. VGA fixup*/
#ifdef USE_780E_VGA
	fixup_interrupt_printf("godson3a fixup: VGA ------> int6 \n");
	dev = _pci_make_tag(1, 0x5, 0);
	pci_write_config8(dev,0x3c,0x06);

	fixup_interrupt_printf("godson3a fixup: VGA ------> int6 \n");
#endif
	/*8. pci/pcie slot fixup */
#if 0
	//8.1. route  00:06:00 (pcie slot) INTA->INTC# -----------------> int6
	// First check if any device in the slot ( return -1 means no device, else there is device ) 
	dev = _pci_make_tag(6, 0x0, 0x0); //added to fixup pci bridge card
	val = pci_read_config32(dev, 0x00);
	if ( val != 0xffffffff) // device on the slot
		pci_write_config8(dev, 0x3c, 0x03);
#endif

	// 8.2.1 route  00:04:00 (pcie_slot(right)) INTA->INTB# -----------------> int5 
	// 8.2.2 route  00:04:01 (pcie_slot(right)) INTB->INTC# -----------------> int6 
	// 8.2.3 route  00:04:02 (pcie_slot(right)) INTC->INTD# -----------------> int5 
	// 8.2.4 route  00:04:03 (pcie_slot(right)) INTD->INTE# -----------------> int5 
	// First check if any device in the slot ( return -1 means no device, else there is device ) 
	dev = _pci_make_tag(4, 0x0, 0x00);
	val = pci_read_config32(dev, 0x00);
	if ( val != 0xffffffff)
		pci_write_config8(dev, 0x3c, 0x05);

	val = pci_read_config8(dev, 0x0e); //judge whether multi-function card

	if ( val & MULTY_FUNCTION) {
		dev = _pci_make_tag(4, 0x0, 0x01);
		val = pci_read_config32(dev, 0x00);
		if ( val != 0xffffffff)
			pci_write_config8(dev, 0x3c, 0x06);

		dev = _pci_make_tag(4, 0x0, 0x02);
		val = pci_read_config32(dev, 0x00);
		if ( val != 0xffffffff)
			pci_write_config8(dev, 0x3c, 0x05);

		dev = _pci_make_tag(4, 0x0, 0x03);
		val = pci_read_config32(dev, 0x00);
		if ( val != 0xffffffff)
			pci_write_config8(dev, 0x3c, 0x05);
	}
	// 8.3.1 route  00:03:00 (pcie_slot(middle)) INTA->INTA# -----------------> int5 
	// 8.3.2 route  00:03:01 (pcie_slot(middle)) INTB->INTB# -----------------> int5 
	// 8.3.3 route  00:03:02 (pcie_slot(middle)) INTC->INTC# -----------------> int6 
	// 8.3.4 route  00:03:03 (pcie_slot(middle)) INTD->INTD# -----------------> int5 
	// First check if any device in the slot ( return -1 means no device, else there is device ) 
	dev = _pci_make_tag(3, 0x0, 0x00);
	val = pci_read_config32(dev, 0x00);
	if ( val != 0xffffffff)
		pci_write_config8(dev, 0x3c, 0x05);

	val = pci_read_config8(dev, 0x0e);

	if ( val & MULTY_FUNCTION) {
		dev = _pci_make_tag(3, 0x0, 0x01);
		val = pci_read_config32(dev, 0x00);
		if ( val != 0xffffffff)
			pci_write_config8(dev, 0x3c, 0x05);

		dev = _pci_make_tag(3, 0x0, 0x02);
		val = pci_read_config32(dev, 0x00);
		if ( val != 0xffffffff)
			pci_write_config8(dev, 0x3c, 0x06);

		dev = _pci_make_tag(3, 0x0, 0x03);
		val = pci_read_config32(dev, 0x00);
		if ( val != 0xffffffff)
			pci_write_config8(dev, 0x3c, 0x05);
	}


	// 8.4.1 route  00:02:00 (pcie_slot(left)) INTA->INTC# -----------------> int6 
	// 8.4.2 route  00:02:01 (pcie_slot(left)) INTB->INTD# -----------------> int5 
	// 8.4.3 route  00:02:02 (pcie_slot(left)) INTC->INTE# -----------------> int5 
	// 8.4.4 route  00:02:03 (pcie_slot(left)) INTD->INTF# -----------------> int5 
	// First check if any device in the slot ( return -1 means no device, else there is device ) 
	dev = _pci_make_tag(2, 0x0, 0x00);
	val = pci_read_config32(dev, 0x00);
	if ( val != 0xffffffff)
		pci_write_config8(dev, 0x3c, 0x06);

	val = pci_read_config8(dev, 0x0e);

	if ( val & MULTY_FUNCTION) {
		dev = _pci_make_tag(2, 0x0, 0x01);
		val = pci_read_config32(dev, 0x00);
		if ( val != 0xffffffff)
			pci_write_config8(dev, 0x3c, 0x05);

		dev = _pci_make_tag(2, 0x0, 0x02);
		val = pci_read_config32(dev, 0x00);
		if ( val != 0xffffffff)
			pci_write_config8(dev, 0x3c, 0x05);

		dev = _pci_make_tag(2, 0x0, 0x03);
		val = pci_read_config32(dev, 0x00);
		if ( val != 0xffffffff)
			pci_write_config8(dev, 0x3c, 0x05);
	}

	// 9. route 07:0x:0x  (pci slot: con30 and con19)
	// NOTICE here: now assume dev 2, dev3 and dev 4 are all enable on x16 pcie slot, but 
	// in fact only one dev need to be enable. If only one device is enable, all code in this function
	// need to be update (that means bus number should minus 2, and interrupt need to be routed again),
	// But at this moment, don't care this "bug".

	// At most "PCI_BRADGE_TOTAL"  pci bridge is support before bus "origin_busnum" is scaned, 
	// now begin probing pci slot... 

	origin_busnum = 7;
	for ( busnum = origin_busnum; busnum <= PCI_BRADGE_TOTAL + origin_busnum ; busnum++)
	{

		// 9.1.1  route 07:05:00 (con38 with add_21) INTA->PCI_INTA --> INTE# ---------------------> int5
		// 9.1.2  route 07:05:01 (con38 with add_21) INTB->PCI_INTB --> INTF# ---------------------> int5
		// 9.1.3  route 07:05:02 (con38 with add_21) INTC->PCI_INTC --> INTG# ---------------------> int3
		// 9.1.4  route 07:05:03 (con38 with add_21) INTD->PCI_INTD --> INTH# ---------------------> int5
		dev = _pci_make_tag(busnum, 0x5, 0x0);
		val = pci_read_config32(dev, 0x00);
		if ( val != 0xffffffff) // device on the slot
			pci_write_config8(dev, 0x3c, 0x5);

		val = pci_read_config8(dev, 0x0e);

		if ( val & MULTY_FUNCTION) {
			dev = _pci_make_tag(busnum, 0x5, 0x1);
			val = pci_read_config32(dev, 0x00);
			if ( val != 0xffffffff) // device on the slot
				pci_write_config8(dev, 0x3c, 0x5);

			dev = _pci_make_tag(busnum, 0x5, 0x2);
			val = pci_read_config32(dev, 0x00);
			if ( val != 0xffffffff) // device on the slot
				pci_write_config8(dev, 0x3c, 0x3);

			dev = _pci_make_tag(busnum, 0x5, 0x3);
			val = pci_read_config32(dev, 0x00);
			if ( val != 0xffffffff) // device on the slot
				pci_write_config8(dev, 0x3c, 0x5);
		}
		// 9.2.1  route 07:06:00 (con45 with add_22) INTA->PCI_INTB --> INTF# ---------------------> int5
		// 9.2.2  route 07:06:01 (con45 with add_22) INTB->PCI_INTC --> INTG# ---------------------> int3
		// 9.2.3  route 07:06:02 (con45 with add_22) INTC->PCI_INTD --> INTH# ---------------------> int5
		// 9.2.4  route 07:06:03 (con45 with add_22) INTD->PCI_INTA --> INTE# ---------------------> int5
		dev = _pci_make_tag(busnum, 0x6, 0x0);
		val = pci_read_config32(dev, 0x00);
		if ( val != 0xffffffff) // device on the slot
			pci_write_config8(dev, 0x3c, 0x05);

		val = pci_read_config8(dev, 0x0e);

		if ( val & MULTY_FUNCTION) {
			dev = _pci_make_tag(busnum, 0x6, 0x1);
			val = pci_read_config32(dev, 0x00);
			if ( val != 0xffffffff) // device on the slot
				pci_write_config8(dev, 0x3c, 0x03);

			dev = _pci_make_tag(busnum, 0x6, 0x2);
			val = pci_read_config32(dev, 0x00);
			if ( val != 0xffffffff) // device on the slot
				pci_write_config8(dev, 0x3c, 0x05);

			dev = _pci_make_tag(busnum, 0x6, 0x3);
			val = pci_read_config32(dev, 0x00);
			if ( val != 0xffffffff) // device on the slot
				pci_write_config8(dev, 0x3c, 0x05);
		}
	}


	/*******************************************************/
	// below added to check pci/pcie interrupt line register
	/*******************************************************/
	// 10.1 check all pcie slot interrupt line register
	for ( tmp = 2; tmp < 7; tmp++)
	{
		dev = _pci_make_tag(tmp, 0x0, 0x0);
		val = pci_read_config32(dev, 0x00);
		if ( val != 0xffffffff) // device on the slot
		{
			val = pci_read_config8(dev, 0x3c);
			if ( val != 0x3)
				fixup_interrupt_printf("%02x:00:00 interrupt line : Error\n",tmp );
		}
	}

	// 10.2 check all pci slot interrupt line register
	for ( tmp = 0x4; tmp < 0x6; tmp++)
	{
		dev = _pci_make_tag(0xa, tmp, 0x0);
		val = pci_read_config32(dev, 0x00);
		if ( val != 0xffffffff) // device on the slot
		{
			val = pci_read_config8(dev, 0x3c);
			if ( val != 0x3)
				fixup_interrupt_printf("0a:%02x:00 interrupt line : Error\n",tmp );
		}
	}

	// 10.4 check RTE/CON30 interrupt line register
	dev = _pci_make_tag(0x7, 0x0, 0x0);
	val = pci_read_config32(dev, 0x00);
	if ( val != 0xffffffff) // device on the slot
	{
		val = pci_read_config8(dev, 0x3c);
		if ( val != 0x5)
			fixup_interrupt_printf("07:00:00 interrupt line : Error\n");
	}

	// 10.5 check  VGA interrupt line register
#ifdef USE_780E_VGA
	dev = _pci_make_tag(0x1, 0x5, 0x0);
	val = pci_read_config32(dev, 0x00);
	if ( val != 0xffffffff) // device on the slot
	{
		val = pci_read_config8(dev, 0x3c);
		if ( val != 0x6)
			fixup_interrupt_printf("01:05:00 interrupt line : Error\n");
	}
#endif
}

void sb700_interrupt_fixup(void)
{
 sb700_interrupt_fixup0();
 sb700_interrupt_fixup1();
}

extern struct efi_memory_map_loongson g_map;
extern unsigned long long memorysize;
extern unsigned long long memorysize_high;
extern unsigned long long memorysize_high_n1;

#include "../../../pmon/cmds/bootparam.h"
struct efi_memory_map_loongson * init_memory_map()
{
	struct efi_memory_map_loongson *emap = &g_map;
	int i = 0;
	unsigned long long size = memorysize_high;

#define EMAP_ENTRY(entry, node, type, start, size) \
	emap->map[(entry)].node_id = (node), \
	emap->map[(entry)].mem_type = (type), \
	emap->map[(entry)].mem_start = (start), \
	emap->map[(entry)].mem_size = (size), \
	(entry)++

 	EMAP_ENTRY(i, 0, SYSTEM_RAM_LOW, 0x00200000, 0x0ee);
 	 /* for entry with mem_size < 1M, we set bit31 to 1 to indicate
 	  * that the unit in mem_size is Byte not MBype*/
 	EMAP_ENTRY(i, 0, SMBIOS_TABLE, (SMBIOS_PHYSICAL_ADDRESS & 0x0fffffff),
 			(SMBIOS_SIZE_LIMIT|0x80000000));
	if(size < 0x6f000000)
		EMAP_ENTRY(i, 0, SYSTEM_RAM_HIGH, 0x90000000, size >> 20);
	/*we waste 16MB here, because 780e TOM is 0xff0000000*/
	else if (size > 0x70000000) {
 		EMAP_ENTRY(i, 0, SYSTEM_RAM_HIGH, 0x90000000, 0x6d0);
 		EMAP_ENTRY(i, 0, SYSTEM_RAM_HIGH, 0x100000000, (size - 0x70000000) >> 20);
	} else
 		EMAP_ENTRY(i, 0, SYSTEM_RAM_HIGH, 0x90000000, 0x6d0);

	if(memorysize_high_n1) {
 		EMAP_ENTRY(i, 1, SYSTEM_RAM_LOW, 0x00000000000L, 0x100);
 		EMAP_ENTRY(i, 1, SYSTEM_RAM_HIGH, 0x00000000000L + 0x90000000, memorysize_high_n1 >> 20);
	}


	emap->vers = 1;
	emap->nr_map = i;
	return emap;
#undef	EMAP_ENTRY
}

static inline unsigned int readl_addr64(unsigned long long addr)
{
unsigned long long a = addr;
unsigned int ret;
asm volatile( ".set mips64;ld $2,%1;lw %0,($2);.set mips0;\n":"=r"(ret):"m"(a):"$2")
;
return ret;
}


static inline void writel_addr64(unsigned long long addr, int v)
{
unsigned long long a = addr;
asm volatile( ".set mips64;ld $2,%1;sw %0,($2);.set mips0;\n"::"r"(v),"m"(a):"$2")
;
}

static inline unsigned long long readq_addr64(unsigned long long addr)
{
unsigned long long a = addr;
unsigned long long ret;
asm volatile( ".set mips64;ld $2,%0;ld $3,($2);sd $3,%1;.set mips0;\n"::"m"(a),"m"(ret):"$2","$3","memory")
;
return ret;
}


static inline void writeq_addr64(unsigned long long addr, unsigned long long v)
{
unsigned long long a = addr;
asm volatile( ".set mips64;ld $2,%0; ld $3,%1;sd $3,($2);.set mips0;\n"::"m"(a),"m"(v):"$2","$3","memory")
;
}

#define ECC_DISABLE_W_UC_ERR_ADDR   (0x10)
#define CTRL_RAW_ADDR            (0x40)
#define ECC_DISABLE_W_UC_ERR_OFFSET 24
#define CTRL_RAW_OFFSET         48 
#define DDR_CONFIG_DISABLE_OFFSET   8


static int ddr_ecc_werr(int cpu, int en)
{
	unsigned long long cpucfg = 0x900000001fe00180ULL+((unsigned long long)cpu<<44);
	unsigned long long ddrcfg = 0x900000000ff00000ULL+((unsigned long long)cpu<<44);
	unsigned long long data8;
	/*enable ddr cfg space*/
	writel_addr64(cpucfg, readl_addr64(cpucfg)& ~(1<<DDR_CONFIG_DISABLE_OFFSET));

	data8 = readq_addr64(ddrcfg+ECC_DISABLE_W_UC_ERR_ADDR);
	data8 = (data8&~(0xffULL<<ECC_DISABLE_W_UC_ERR_OFFSET));
	if(!en) data8 |= (1ULL<<ECC_DISABLE_W_UC_ERR_OFFSET);

	writeq_addr64(ddrcfg+ECC_DISABLE_W_UC_ERR_ADDR,data8);

        if(en)
	{
	data8 = readq_addr64(ddrcfg+CTRL_RAW_ADDR);
	data8 = (data8&~(0xffULL<<CTRL_RAW_OFFSET))|(3ULL<<CTRL_RAW_OFFSET);

	writeq_addr64(ddrcfg+CTRL_RAW_ADDR, data8);
        }
	/*disable ddr cfg space*/
	writel_addr64(cpucfg, readl_addr64(cpucfg)|(1<<DDR_CONFIG_DISABLE_OFFSET));
	return 0;
}

int init_ecc_one(int cpu, unsigned long long size)
{
	unsigned long long start, end, next;

	//ddr_ecc_werr(cpu, 1);

#ifdef  FASTECC2
	start = 0xb800000090000000ULL+ ((unsigned long long)cpu<<44);
#else
	start = 0x9800000090000000ULL+ ((unsigned long long)cpu<<44);
#endif
	end = start + size;
	for(;start<end;)
	{
	next=start+0x40000000;
	if(next>end) next=end;

#ifdef FASTECC
	printf("0x%llx 0x%llx\n",start, next);
#endif
	asm volatile( 
			".set mips64;"
			"ld $2,%0;"
			"ld $3,%1;"
			"1:\n"
#ifndef FASTECC2
			"sd $0,($2);"
			"sd $0,0x20($2);"
			"sd $0,0x40($2);"
			"sd $0,0x60($2);"
			"sd $0,0x80($2);"
			"sd $0,0xa0($2);"
			"sd $0,0xc0($2);"
			"sd $0,0xe0($2);"
#else
			"sd $0,($2);"
			"sd $0,8($2);"
			"sd $0,0x10($2);"
			"sd $0,0x18($2);"

			"sd $0,0x20($2);"
			"sd $0,0x28($2);"
			"sd $0,0x30($2);"
			"sd $0,0x38($2);"

			"sd $0,0x40($2);"
			"sd $0,0x48($2);"
			"sd $0,0x50($2);"
			"sd $0,0x58($2);"

			"sd $0,0x60($2);"
			"sd $0,0x68($2);"
			"sd $0,0x70($2);"
			"sd $0,0x78($2);"

			"sd $0,0x80($2);"
			"sd $0,0x88($2);"
			"sd $0,0x90($2);"
			"sd $0,0x98($2);"

			"sd $0,0xa0($2);"
			"sd $0,0xa8($2);"
			"sd $0,0xb0($2);"
			"sd $0,0xb8($2);"

			"sd $0,0xc0($2);"
			"sd $0,0xc8($2);"
			"sd $0,0xd0($2);"
			"sd $0,0xd8($2);"

			"sd $0,0xe0($2);"
			"sd $0,0xe8($2);"
			"sd $0,0xf0($2);"
			"sd $0,0xf8($2);"
#endif
			"daddiu $2,0x100;"
			"bne $2,$3,1b;"
			"nop;"
			".set mips0;\n"
			::"m"(start),"m"(next):"$2","$3"
				);
		start=next;
          }

#ifndef  FASTECC2

asm volatile(
"	li $2, 0x80000000;\n"
"	li $3, 0x400000;\n"
"       addu  $3,$2;\n"
"10:\n"
"	cache	3, 0x00($2);\n"
"	cache	3, 0x01($2);\n"
"	cache	3, 0x02($2);\n"
"	cache	3, 0x03($2);\n"
"	cache	3, 0x20($2);\n"
"	cache	3, 0x21($2);\n"
"	cache	3, 0x22($2);\n"
"	cache	3, 0x23($2);\n"
"	cache	3, 0x40($2);\n"
"	cache	3, 0x41($2);\n"
"	cache	3, 0x42($2);\n"
"	cache	3, 0x43($2);\n"
"	cache	3, 0x60($2);\n"
"	cache	3, 0x61($2);\n"
"	cache	3, 0x62($2);\n"
"	cache	3, 0x63($2);\n"
"	addu	$2, 0x80;\n"
"	bne	    $2,$3, 10b;\n"
"	nop;\n"
"sync;"
:::"$2","$3");

asm volatile(
".set mips64;\n"
"       dli $2, 0x9800100000000000;\n"
"	li $3, 0x400000;\n"
"   daddu  $3,$2;\n"
"10:\n"
"	cache	3, 0x00($2);\n"
"	cache	3, 0x01($2);\n"
"	cache	3, 0x02($2);\n"
"	cache	3, 0x03($2);\n"
"	cache	3, 0x20($2);\n"
"	cache	3, 0x21($2);\n"
"	cache	3, 0x22($2);\n"
"	cache	3, 0x23($2);\n"
"	cache	3, 0x40($2);\n"
"	cache	3, 0x41($2);\n"
"	cache	3, 0x42($2);\n"
"	cache	3, 0x43($2);\n"
"	cache	3, 0x60($2);\n"
"	cache	3, 0x61($2);\n"
"	cache	3, 0x62($2);\n"
"	cache	3, 0x63($2);\n"
"	daddiu	$2, 0x80;\n"
"	bne	    $2,$3, 10b;\n"
"	nop;\n"
"sync;"
".set mips0;\n"
:::"$2","$3");
#endif


	//ddr_ecc_werr(cpu, 0);



}
/*
make cfg  all tgt=rom CROSS_COMPILE=/opt/gcc-4.4-gnu/bin/mipsel-linux- DEBUG='-g -Wa,-mfix-ls2f-kernel'
*/
int init_ecc()
{
#ifdef FASTECC
	printf("init ecc, wait a little\n");
#endif

	if(memorysize_high)
	{
		init_ecc_one(0,memorysize_high);
	}

	if(memorysize_high_n1)
	{
		init_ecc_one(1, memorysize_high_n1);
	}

	init_ecc_finished = 1;
asm(".set nofix_ls2f_kernel");
return 0;
}

asm(".set fix_ls2f_kernel");

init_ecc_end()
{
	while(!init_ecc_finished);

	if(memorysize_high)
	{
		ddr_ecc_werr(0, 0);
	}

	if(memorysize_high_n1)
	{
		ddr_ecc_werr(1, 0);
	}

}
