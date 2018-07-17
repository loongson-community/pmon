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
#include <include/stdarg.h>
 unsigned int mem_size = 0;

#include <include/stdio.h>
#include <include/string.h>
#include <include/file.h>
#include <include/termio.h>
#include <stdbool.h>
#include "target/sbd.h"

#include "../../../pmon/cmds/cmd_main/window.h"
#include "../../../pmon/cmds/cmd_main/cmd_main.h"

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

extern char * nvram_offs;

void	tgt_putchar (int);
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

#include <sys/param.h>
#include <sys/syslog.h>
#include <machine/endian.h>
#include <sys/device.h>
#include <machine/cpu.h>
#include <machine/pio.h>
#include <machine/intr.h>
#include <dev/pci/pcivar.h>
#include <sys/types.h>
#include <termio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>

#include <dev/ic/mc146818reg.h>
#include <linux/io.h>

#include <autoconf.h>

#include <machine/cpu.h>
#include <machine/pio.h>
#include "pflash.h"
#include "dev/pflash_tgt.h"

#include "target/bonito.h"
#include "target/ls2h.h"
#include "target/board.h"
#include <pmon/dev/gt64240reg.h>
#include <pmon/dev/ns16550.h>
#include "target/firewall.h"

#include <pmon.h>

#include "../pci/amd_780e.h"
#include "../pci/rs780_cmn.h"

#include "mod_x86emu_int10.h"
#include "mod_x86emu.h"
#include "mod_vgacon.h"
#include "mod_framebuffer.h"
#include "mod_smi712.h"
#include "mod_smi502.h"
#include "mod_sisfb.h"
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


int tgt_i2cread(int type,unsigned char *addr,int addrlen,unsigned char reg,unsigned char* buf ,int count);
int tgt_i2cwrite(int type,unsigned char *addr,int addrlen,unsigned char reg,unsigned char* buf ,int count);
extern struct trapframe DBGREG;
extern void *memset(void *, int, size_t);

int kbd_available;
int bios_available;
int usb_kbd_available;;
int vga_available;
int cmd_main_mutex = 0;
int bios_mutex = 0;
/* Board Version Number */
unsigned int board_ver_num;
unsigned int superio_base;

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

static int w83627_read(int dev,int addr)
{
	int data;
	/*enter*/
	outb(superio_base + 0x002e,0x87);
	outb(superio_base + 0x002e,0x87);
	/*select logic dev reg */
	outb(superio_base + 0x002e,0x7);
	outb(superio_base + 0x002f,dev);
	/*access reg */
	outb(superio_base + 0x002e,addr);
	data=inb(superio_base + 0x002f);
	/*exit*/
	outb(superio_base + 0x002e,0xaa);
	outb(superio_base + 0x002e,0xaa);
	return data;
}

static void w83627_write(int dev,int addr,int data)
{
	/*enter*/
	outb(superio_base + 0x002e,0x87);
	outb(superio_base + 0x002e,0x87);
	/*select logic dev reg */
	outb(superio_base + 0x002e,0x7);
	outb(superio_base + 0x002f,dev);
	/*access reg */
	outb(superio_base + 0x002e,addr);
	outb(superio_base + 0x002f,data);
	/*exit*/
	outb(superio_base + 0x002e,0xaa);
	outb(superio_base + 0x002e,0xaa);

}
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
	w83627_write(5,0xf0,0xc0); //KBC clock rate: 0xc0: 16MHZ, 0x80: 12MHZ

	/* add support for fan speed controler */
#define HM_OFF 0x0290
	w83627_write(0xb,0x60,HM_OFF >> 0x8); // set HM base address @0xbff00290
	w83627_write(0xb,0x61,HM_OFF & 0xff);
	/* enable HM  */
	w83627_write(0xb,0x30,0x1);

}
int afxIsReturnToPmon = 0;

  struct FackTermDev
  {
      int dev;
  };

#endif



ConfigEntry	ConfigTable[] =
{
	{ (char *)GS3_UART_BASE, 0, ns16550, 256, CONS_BAUD, NS16550HZ },
	{ (char *)GS3_UART1_BASE, 0, ns16550, 256, CONS_BAUD, NS16550HZ },
	{ (char *)1, 0, fbterm, 256, CONS_BAUD, NS16550HZ },
	{ 0 }
};

unsigned long _filebase;


extern unsigned long long  memorysize;
extern unsigned long long  memorysize_high;

extern char MipsException[], MipsExceptionEnd[];

unsigned char hwethadr[6];

void initmips(unsigned long long  raw_memsz);

void addr_tst1(void);
void addr_tst2(void);
void movinv1(int iter, ulong p1, ulong p2);

pcireg_t _pci_allocate_io(struct pci_device *dev, vm_size_t size);
static void superio_reinit();

extern char waitforinit[];
void
initmips(unsigned long long raw_memsz)
{
	int i;
	int* io_addr;
	unsigned long long memsz;
	//core1-3 run waitdorinit function in ram
	asm volatile(
	".set push;\n"
	".set noreorder\n"
        ".set mips64;\n"
	"dli $2,0x900000003ff01100;\n"
	"dli $3, 3;\n"
        "1:sd %0,0x20($2);"
	"daddiu $2,0x100;\n"
	"addiu $3,-1;\n"
	"bnez $3, 1b;\n"
	"nop;\n"
	".set reorder;\n"
        ".set mips0;\n"
	".set pop;\n"
         ::"r"(&waitforinit):"$2","$3");

	tgt_fpuenable();
	/*enable float*/
	tgt_fpuenable();
	//CPU_TLBClear();


	get_memorysize(raw_memsz);


	/*
	 *  Probe clock frequencys so delays will work properly.
	 */
	for (i = 0;i < 3;i++)
	{
		delay(0x200000);
		delay(0x200000);
	}
	ls2h_pcibios_init();

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
	SBD_DISPLAY("BEV1",0);
	bcopy(MipsException, (char *)TLB_MISS_EXC_VEC, MipsExceptionEnd - MipsException);
	bcopy(MipsException, (char *)XTLB_MISS_EXC_VEC, MipsExceptionEnd - MipsException);
	bcopy(MipsException, (char *)GEN_EXC_VEC, MipsExceptionEnd - MipsException);

	CPU_FlushCache();

#ifndef ROM_EXCEPTION
	CPU_SetSR(0, SR_BOOT_EXC_VEC);
#endif
	SBD_DISPLAY("BEV0",0);

	printf("BEV in SR set to zero.\n");

	if (board_ver_num == LS3A2H_BOARD_2_2)
		printf("3A2H Board Version is 2.2.\n");
	else
		printf("3A2H Board Version is 2.1 or 2.0.\n");

	/*
	 * Launch!
	 */
	//_pci_conf_write(_pci_make_tag(0,0,0),0x90,0xff800000); 

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
extern int init_win_device(void);
extern int fb_init(unsigned long,unsigned long);
extern int dc_init();


#define LS2H

#define u64 unsigned long long
extern u64 __raw__readq(u64 q);
extern u64 __raw__writeq(u64 addr, u64 val);
u64 ver_val;
#ifdef PCIE_GRAPHIC_CARD
extern bool is_pcie_vga_card();
#endif

void
tgt_devconfig()
{
	int ic, len;
	int count, i;
	char key;
	char copyright[9] ="REV_";
	char bootup[] = "Booting...";
	char *tmp_copy = NULL;
	char tmp_date[11];
	char * s;
	unsigned int cnt;
	u64 add, val;
#if NMOD_VGACON > 0
	int rc=1;
#if NMOD_FRAMEBUFFER > 0 
	unsigned long fbaddress,ioaddress;
	extern struct pci_device *vga_dev;
#endif
#endif
	outl(LS2H_GPIO_CFG_REG, 0xf << 24);
#if 1
	board_ver_num = LS3A2H_BOARD_2_2;
#else
	board_ver_num = (inl(LS2H_GPIO_IN_REG) >> 8) & 0xf;
#endif
	if(board_ver_num == LS3A2H_BOARD_2_2) {				// new 3A2H Board: lpc interface mount on 2H, old board lpc mount on 3A
		superio_base = 0xbbf00000;
	} else if(board_ver_num == LS3A2H_BOARD_OLD) {
		superio_base = 0xbff00000;
	}
	_pci_devinit(1);	/* PCI device initialization */
#if (NMOD_X86EMU_INT10 > 0)||(NMOD_X86EMU >0)
#ifdef PCIE_GRAPHIC_CARD
	if ( is_pcie_vga_card() ) {
		SBD_DISPLAY("VGAI", 0);

		// map 0x90000e50_00000000 --> 0x40000000
		// 2h pci mem used
		// 3a: 0x9000_0e50_0000_0000 ~ 0x9000_0e50_3fff_ffff
		// 2h: 0x4000_0000 ~ 7fff_ffff
		asm (".set mips3; \
			dli $2, 0x900000003ff02000; \
			dli $4, 0x900000003ff02700; \
			1: \
			dli $3, 0x000000001b000000 ; \
			sd  $3, 0x0($2); \
			dli $3, 0xffffffffff000000; \
			sd  $3, 0x40($2); \
			dli $3, 0x00000e001f0000f7; \
			sd  $3, 0x80($2); \
			dli $3, 0x0000000018000000; \
			sd  $3, 0x08($2); \
			dli $3, 0xffffffffff000000; \
			sd  $3, 0x48($2); \
			dli $3, 0x00000e00180000f7; \
			sd  $3, 0x88($2); \
			dli $3, 0x0000000010000000; \
			sd  $3, 0x10($2); \
			dli $3, 0xfffffffff8000000; \
			sd  $3, 0x50($2); \
			dli $3, 0x00000e00100000f7; \
			sd  $3, 0x90($2); \
			dli $3, 0x0000000040000000; \
			sd  $3, 0x18($2); \
			dli $3, 0xffffffffc0000000; \
			sd  $3, 0x58($2); \
			dli $3, 0x00000e50000000f7; \
			sd  $3, 0x98($2); \
			daddiu  $2, $2, 0x100 ; \
			bne     $2, $4, 1b; \
			nop; .set mips0" :::"$2", "$3", "$4");

		rc = vga_bios_init();
		printf("rc=%d\n", rc);
	}
#endif
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
	if (rc > 0) {

#ifdef LS2H
		add = 0x90000e000c000000;
		printf("wait for ls2h init ddr..\n");
		for (count = 0;count < 11;count++)
		{
			val = __raw__readq(add);
			val = val & 0xff;
			if(val == 0x55)
			{
				break;
			}
			delay(6000);
		}
		for(i=0,count=(200/8)+1;count>0;count--,i+=8)
		{
			ver_val = __raw__readq(0x90000e0008000000+i);
			__raw__writeq(0x9800000008000000+i, ver_val);
		}
		printf("begin dc_init\n");
		fbaddress = dc_init();
		fbaddress |= 0xc0000000; //NOTICE HERE: map to mem on ls2h
#endif

#if NMOD_SISFB
		fbaddress=sisfb_init_module();
		printf("fbaddress 0x%x\tioaddress 0x%x\n",fbaddress, ioaddress);
#endif

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

#ifdef PCIE_GRAPHIC_CARD
		if ( is_pcie_vga_card() ) {
			fbaddress = 0xc0000000;
		}
#endif
		printf("fbaddress = %08x\n", fbaddress);
		fb_init(fbaddress, ioaddress);

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
	//key_board init
	//#if ((NMOD_VGACON >0) &&(PCI_IDSEL_VIA686B !=0)|| (PCI_IDSEL_CS5536 !=0))
#if NMOD_VGACON >0
	if(getenv("nokbd")) rc=1;
	else {
		superio_reinit();
		rc=kbd_initialize();
	}
	printf("%s\n",kbd_error_msgs[rc]);
	if(!rc){ 
		kbd_available=1;
	}
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

	for(count = 0;count < 100;count ++)
	{
		ioctl(STDIN, FIONREAD, &cnt);
		if(cnt > 0 && strchr("[G\r",getchar()))
			break;
		delay(30);
	}

	vga_available = 1;
	video_set_color(0xf);

	for (ic = 0; ic < 64; ic++)
	{
		video_putchar1(2 + ic*8, REV_ROW_LINE, ' ');
		video_putchar1(2 + ic*8, INF_ROW_LINE, ' ');
	}

	vga_available = 0;

	if (count >= 100)
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


	printf("devconfig done.\n");

#endif



}
extern int test_icache_1(short *addr);
extern int test_icache_2(int addr);
extern int test_icache_3(int addr);
extern void godson1_cache_flush(void);
#define tgt_putchar_uc(x) (*(void (*)(char)) (((long)tgt_putchar)|0x20000000)) (x)

extern void cs5536_gpio_init(void);
extern void test_gpio_function(void);
extern void cs5536_pci_fixup(void);
#ifdef PCIE_GRAPHIC_CARD
extern int ls2h_pcibios_init(void);
#endif
extern int ls2h_pcibios_init(void);
extern int gmac_w18(void);




void
tgt_devinit()
{

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
	
    CPU_ConfigCache();

	_pci_businit(1);	/* PCI bus initialization */
}


void
tgt_poweroff()
{
	volatile unsigned int * pm_ctrl_reg = 0xbbef0014;
	volatile unsigned int * pm_statu_reg = 0xbbef000c;

	* pm_statu_reg = 0x100;	 // clear bit8: PWRBTN_STS
	delay(100);
	* pm_ctrl_reg = 0x3c00;  // sleep enable, and enter S5 state 
}

void
tgt_reboot(void)
{
	unsigned int GPIO_DATA_REG = 0xbfe0011c;
	unsigned int GPIO_EN_REG = 0xbfe00120;

#define ls_readl(x) (* (volatile unsigned int*)(x))	
	ls_readl(GPIO_DATA_REG) &= (~0x38);
	ls_readl(GPIO_EN_REG) &= (~(1 << 13));
	ls_readl(GPIO_DATA_REG) |= (1 << 13);
/*	volatile char * hard_reset_reg = 0xbbef0030;
	* hard_reset_reg = ( * hard_reset_reg) | 0x01; // watch dog hardreset
*/
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
	int year, month, date, hour, min, sec, val;

	val = (1 << 13) | (1 << 11) | (1 << 8);
	outl(LS2H_RTC_CTRL_REG, val);

	outl(LS2H_TOY_TRIM_REG, 0);
	outl(LS2H_RTC_TRIM_REG, 0);

	year	= inl(LS2H_TOY_READ1_REG);
	val	= inl(LS2H_TOY_READ0_REG);
	month	= ((val >> 26) & 0x3f);
	date	= (val >> 21) & 0x1f;
	hour	= (val >> 16) & 0x1f;
	min	= (val >> 10) & 0x3f;
	sec	= (val >> 4) & 0x3f;

	if ((year < 0 || year > 138) 
		|| (month < 1 || month > 12)
		|| (date < 1 || date > 31)
		|| (hour > 23) || (min > 59)
		|| (sec > 59)) {
		tgt_printf("RTC time invalid, reset to epoch.\n");
		/* 2000-01-01 00:00:00 */
		val = (2 << 26) | (1 << 21);
		outl(LS2H_TOY_WRITE1_REG, 0x64);
		outl(LS2H_TOY_WRITE0_REG, val);
	}
}

int word_addr;

static inline unsigned char CMOS_READ(unsigned char addr)
{
        unsigned char val;
	
	linux_outb_p(addr, 0x70);
        val = linux_inb_p(0x71);

        return val;
}
                                                                               
static inline void CMOS_WRITE(unsigned char val, unsigned char addr)
{

	linux_outb_p(addr, 0x70);
        linux_outb_p(val, 0x71);

}

static void
_probe_frequencies()
{
#ifdef HAVE_TOD
        int i, timeout, cur, sec, cnt;
#endif
                                                                    
        SBD_DISPLAY ("FREQ", CHKPNT_FREQ);

        md_pipefreq = 800000000;        /* NB FPGA*/
        md_cpufreq  =  60000000;

        clk_invalid = 1;
#ifdef HAVE_TOD
	init_legacy_rtc();

        SBD_DISPLAY ("FREI", CHKPNT_FREQ);

        /*
         * Do the next twice for two reasons. First make sure we run from
         * cache. Second make sure synched on second update. (Pun intended!)
         */
	for (i = 2; i != 0; i--) {
		timeout = 10000000;
		sec = (inl(LS2H_TOY_READ0_REG) >> 4) & 0x3f;
		do {
			cur = ((inl(LS2H_TOY_READ0_REG) >> 4) & 0x3f);
		} while (cur == sec);

		cnt = CPU_GetCOUNT();
		do {
			timeout--;
			sec = (inl(LS2H_TOY_READ0_REG) >> 4) & 0x3f;
		} while (timeout != 0 && (cur == sec));
		cnt = CPU_GetCOUNT() - cnt;
		if (timeout == 0) {
			tgt_printf("time out!\n");
			break;	/* Get out if clock is not running */
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
         tgt_printf("cpu freq %u\n",md_pipefreq);
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
        time_t t;
        int year, month, date, hour, min, sec;
	/*gx 2005-01-17 */
#ifdef HAVE_TOD

	month	= (inl(LS2H_TOY_READ0_REG) & 0xfc000000) >> 26;
	date	= (inl(LS2H_TOY_READ0_REG) & 0x3e00000) >> 21;
	hour	= (inl(LS2H_TOY_READ0_REG) & 0x1f0000) >> 16;
	min	= (inl(LS2H_TOY_READ0_REG) & 0xfc00) >> 10;
	sec	= (inl(LS2H_TOY_READ0_REG) & 0x3f0) >> 4;
	year	= inl(LS2H_TOY_READ1_REG);

	tm.tm_sec	= sec;
	tm.tm_min	= min;
	tm.tm_hour	= hour;
	tm.tm_mday	= date;
	tm.tm_mon	= month - 1;
	tm.tm_year	= year;

	tm.tm_isdst = tm.tm_gmtoff = 0;
	t = gmmktime(&tm);
#endif
        return(t);
}
/*
 *  Set the current time if a TOD clock is present
 */
void
tgt_settime(time_t t)
{
	struct tm *tm;
	int year, month, date, hour, min, sec;
	unsigned int time_value;
#ifdef HAVE_TOD
        tm = gmtime(&t);

	year = tm->tm_year;
	month = tm->tm_mon;
	date = tm->tm_mday;
	hour = tm->tm_hour;
	min = tm->tm_min;
	sec = tm->tm_sec;

	time_value = ((month + 1)<<26 | date<<21 | hour<<16 | min<<10 | sec<<4);
	outl(LS2H_TOY_WRITE0_REG, time_value);
	outl(LS2H_TOY_WRITE1_REG, year);
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
	printf("in envinit\n");
#ifdef NVRAM_IN_FLASH
	nvram = (char *)(tgt_flashmap())->fl_map_base + ((unsigned long)(&nvram_offs)-0x8f010000);
	printf("nvram=%08x\n",(unsigned int)nvram);
	if(fl_devident(nvram, NULL) == 0 ||
			cksum(nvram, NVRAM_SIZE, 0) != 0) {
#else
		nvram = (char *)malloc(512);
		nvram_get(nvram);
		if(cksum(nvram, NVRAM_SIZE, 0) != 0) {
#endif
			printf("NVRAM is invalid!\n");
			nvram_invalid = 1;
		}
		else {
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

		printf("NVRAM@%x\n",(u_int32_t)nvram);

		/*
		 *  Ethernet address for Galileo ethernet is stored in the last
		 *  six bytes of nvram storage. Set environment to it.
		 */
		bcopy(&nvram[ETHER_OFFS], hwethadr, 6);
		sprintf(env, "%02x:%02x:%02x:%02x:%02x:%02x", hwethadr[0], hwethadr[1],
				hwethadr[2], hwethadr[3], hwethadr[4], hwethadr[5]);
		(*func)("ethaddr", env);

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


		sprintf(env, "%d", md_pipefreq);
		(*func)("cpuclock", env);

		sprintf(env, "%d", md_cpufreq);
		(*func)("busclock", env);

		sprintf(env, "%d",VRAM_SIZE);
		(*func)("vramsize", env);

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
        nvram += ((unsigned long)(&nvram_offs)-0x8f010000);
	nvramsecbuf = (char *)malloc(NVRAM_SECSIZE);
	if(nvramsecbuf == 0) {
		printf("Warning! Unable to malloc nvrambuffer!\n");
		return(-1);
	}
        memcpy(nvramsecbuf, nvram, NVRAM_SECSIZE);
	nvrambuf = nvramsecbuf;
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
        nvram += ((unsigned long)(&nvram_offs)-0x8f010000);
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
		nvrambuf = nvramsecbuf;
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
	nvrambuf = nvramsecbuf;
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




extern struct efi_memory_map_loongson g_map;
extern unsigned long long memorysize;
extern unsigned long long memorysize_high;

#include "../../../pmon/cmds/bootparam.h"
#include "../../../pmon/common/smbios/smbios.h"
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

#ifndef UMA_VIDEO_RAM
	EMAP_ENTRY(i, 0, SYSTEM_RAM_LOW, 0x00200000, 0x0ee);
#else
	EMAP_ENTRY(i, 0, SYSTEM_RAM_LOW, 0x00200000, 0x0ee);
#endif

	/* for entry with mem_size < 1M, we set bit31 to 1 to indicate
	 * that the unit in mem_size is Byte not MBype */
	EMAP_ENTRY(i, 0, SMBIOS_TABLE, (SMBIOS_PHYSICAL_ADDRESS & 0x0fffffff),
			(SMBIOS_SIZE_LIMIT | 0x80000000));

#ifndef UMA_VIDEO_RAM
	EMAP_ENTRY(i, 0, SYSTEM_RAM_HIGH, 0x90000000, size >> 20);
#else
	/*add UMA_VIDEO_RAM area to reserved 0x100 MB memory for GPU vram*/

	EMAP_ENTRY(i, 0, UMA_VIDEO_RAM, 0x90000000, 0x100);
	EMAP_ENTRY(i, 0, SYSTEM_RAM_HIGH, 0xa0000000, (size >> 20) - 0x100);
//	EMAP_ENTRY(i, 0, UMA_VIDEO_RAM, 0x110000000, 0x100);
//	EMAP_ENTRY(i, 0, SYSTEM_RAM_HIGH, 0x120000000, (size >> 20) - 0x100);

#endif
	emap->vers = 1;
	emap->nr_map = i;

	return emap;
#undef	EMAP_ENTRY
}

#define HW_CONFIG 0xbfe00180
#define HW_SAMPLE 0xbfe00190
#define HT_MEM_PLL 0xbfe001c0

static inline int mem_is_hw_bypassed()
{
	int32_t hw_sample = readl(HW_SAMPLE + 0x4);
	return ((hw_sample >> 5) & 0x1f) == 0x1f;
}

static inline int mem_is_sw_setup()
{
	int32_t hw_sample = readl(HW_SAMPLE + 0x4);
	return ((hw_sample >> 5) & 0x1f) == 0x0f; 

}

static inline int mem_hw_freq_mul()
{
	int32_t hw_mul = (readl(HW_SAMPLE + 0x4) >> 5) & 0xf;

	return hw_mul + 30;
}

static inline int mem_hw_freq_div()
{
	int32_t hw_div = (readl(HW_SAMPLE + 0x4) >> 9) & 0x1;

	return hw_div + 3;
}

static inline int mem_sw_freq_mul()
{
	int32_t sw_mul = (readl(HT_MEM_PLL) >> 14) & 0x3ff;

	return sw_mul;
}

static inline int mem_sw_freq_div()
{
	int32_t sw_div = (readl(HT_MEM_PLL) >> 24) & 0x3f;

	return sw_div;
}

void print_mem_freq(void) 
{
	int mem_ref_clock = 33; /* int Mhz */

	if ( mem_is_hw_bypassed()) { 
		printf("hw bypassed! mem@ %dMhz\n", mem_ref_clock);
		return;
	}
	if (!mem_is_sw_setup()) 
		printf("hw selected! mem@ %dMhz\n", (mem_hw_freq_mul() * mem_ref_clock)/mem_hw_freq_div());
	else
		printf("sw selected! mem@ %dMhz\n", (mem_sw_freq_mul() * mem_ref_clock)/mem_sw_freq_div());
	
}

extern struct interface_info g_board;
struct board_devices *board_devices_info()
{

	struct board_devices *bd = &g_board;

	strcpy(bd->name,"Loongson-3A2000-2H-1w-V0.7-demo");
	bd->num_resources = 10;

	return bd;
}

void  print_cpu_info()
{
	int cycles1, cycles2;
	int bogo;
	int loops = 1 << 18;
	int freq = tgt_pipefreq() / 1000000;

	printf("Copyright 2000-2002, Opsycon AB, Sweden.\n");
	printf("Copyright 2005, ICT CAS.\n");
	printf("CPU %s ", md_cpuname());

	cycles1 = (int)read_c0_count();
	__loop_delay(loops);
	cycles2 = (int)read_c0_count();

	bogo = freq * loops / (cycles2 - cycles1);

	printf("BogoMIPS: %d\n", bogo);
}
