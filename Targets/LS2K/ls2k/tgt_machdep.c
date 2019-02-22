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
#include <sys/linux/types.h>
#include <include/stdarg.h>
unsigned int mem_size = 0;

#include "../../../pmon/common/smbios/smbios.h"

void tgt_putchar(int);
int tgt_printf(const char *fmt, ...)
{
	int n;
	char buf[1024];
	char *p = buf;
	char c;
	va_list ap;
	va_start(ap, fmt);
	n = vsprintf(buf, fmt, ap);
	va_end(ap);
	while ((c = *p++)) {
		if (c == '\n')
			tgt_putchar('\r');
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

#include <dev/ic/mc146818reg.h>
#include <linux/io.h>

#include <autoconf.h>

#include "pflash.h"
#include "dev/pflash_tgt.h"

#include "target/bonito.h"
#include "target/ls2k.h"
#include "target/board.h"
#include <pmon/dev/gt64240reg.h>
#include <pmon/dev/ns16550.h>

#include <pmon.h>

#include "mod_x86emu_int10.h"
#include "mod_x86emu.h"
#include "mod_vgacon.h"
#include "mod_framebuffer.h"
#include "mod_smi712.h"
#include "mod_smi502.h"
#include "mod_sisfb.h"
#include "sii9022a.h"
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
int vga_available = 0;
#elif defined(VGAROM_IN_BIOS)
#include "vgarom.c"
#endif
#include "nand.h"
#include "spinand_mt29f.h"
#include "spinand_lld.h"
#include "m25p80.h"

int tgt_i2cread(int type, unsigned char *addr, int addrlen, unsigned char reg,
		unsigned char *buf, int count);
int tgt_i2cwrite(int type, unsigned char *addr, int addrlen, unsigned char reg,
		 unsigned char *buf, int count);
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

extern int vgaterm(int op, struct DevEntry *dev, unsigned long param, int data);
extern int fbterm(int op, struct DevEntry *dev, unsigned long param, int data);
void error(unsigned long *adr, unsigned long good, unsigned long bad);
void modtst(int offset, int iter, unsigned long p1, unsigned long p2);
void do_tick(void);
void print_hdr(void);
void ad_err2(unsigned long *adr, unsigned long bad);
void ad_err1(unsigned long *adr1, unsigned long *adr2, unsigned long good,
	     unsigned long bad);
void mv_error(unsigned long *adr, unsigned long good, unsigned long bad);

void print_err(unsigned long *adr, unsigned long good, unsigned long bad,
	       unsigned long xor);
static void init_legacy_rtc(void);

ConfigEntry ConfigTable[] = {
	{(char *)COM1_BASE_ADDR, 0, ns16550, 256, CONS_BAUD, NS16550HZ},
#if NMOD_VGACON >0
#if NMOD_FRAMEBUFFER >0
	{(char *)1, 0, fbterm, 256, CONS_BAUD, NS16550HZ},
#else
	{(char *)1, 0, vgaterm, 256, CONS_BAUD, NS16550HZ},
#endif
#endif
	{0}
};

int afxIsReturnToPmon = 0;
unsigned char activecom = 0x3;
unsigned char em_enable = 0x3;
unsigned long _filebase;

extern unsigned long long memorysize;
extern unsigned long long memorysize_high;

extern char MipsException[], MipsExceptionEnd[];

unsigned char hwethadr[6];

void initmips(unsigned long long  raw_memsz);

void addr_tst1(void);
void addr_tst2(void);
void movinv1(int iter, ulong p1, ulong p2);

pcireg_t _pci_allocate_io(struct pci_device *dev, vm_size_t size);
static void superio_reinit();
extern char wait_for_smp_call[];
extern char ls2k_version();
#ifdef SLT
extern void slt_test();
#endif

void initmips(unsigned long long  raw_memsz)
{
	unsigned int hi;
	unsigned long long memsz;
	unsigned short i;
	//core1 run wait_for_smp_call function in ram
	asm volatile(".set mips64;sd %1,(%0);.set mips0;"::"r"(0xbfe11120),"r"(&wait_for_smp_call));
#ifdef CONFIG_UART0_SPLIT
	*(volatile int *)0xbfe10428 |= 0xe;
#endif

	tgt_fpuenable();

	get_memorysize(raw_memsz);

	/*enable float */
	tgt_fpuenable();
//	CPU_TLBClear();

	/*
	 *  Probe clock frequencys so delays will work properly.
	 */

	tgt_cpufreq();
	SBD_DISPLAY("DONE", 0);

	cpuinfotab[0] = &DBGREG;
	/*
	 *  Init PMON and debug
	 */
	CPU_ConfigCache();

	dbginit(NULL);

#ifndef ROM_EXCEPTION
	CPU_SetSR(0, SR_BOOT_EXC_VEC);
#endif
	/*
	 *  Set up exception vectors.
	 */
	SBD_DISPLAY("BEV1", 0);
	bcopy(MipsException, (char *)XTLB_MISS_EXC_VEC,
			MipsExceptionEnd - MipsException);
	bcopy(MipsException, (char *)GEN_EXC_VEC,
			MipsExceptionEnd - MipsException);
	SBD_DISPLAY("BEV0", 0);
	printf("BEV in SR set to zero.\n");
	/*disable spi instruct fetch before enter spi io mode*/
	*(volatile int *)0xbfe10080 = 0x1fc00082;
#if NNAND
#ifdef CONFIG_LS2K_NAND
	*(volatile int *)0xbfe10420 |= (1<<9) ;
	ls2k_nand_init();
#else
	/*nand pin as gpio*/
	*(volatile int *)0xbfe10420 &= ~(1<<9) ;
#endif
#if NSPINAND_MT29F || NSPINAND_LLD
	ls2h_spi_nand_probe();
#endif
#if NM25P80
	ls2h_m25p_probe();
#endif
#endif
 /*set pwm1,2 to gpio, gpio21, gpio22 to 1*/
 *(volatile int *)0xbfe10420 &= ~(0x6<<12);
 *(volatile int *)0xbfe10500 &= ~(0x3<<21);
 *(volatile int *)0xbfe10510 |= 0x3<<21;
#ifdef DTB
	verify_dtb();
#endif

	/*
	 * Launch!
	 */
	main();
}

#define STR_STORE_BASE 0xafaaa000
#define PM_REG_BASE    0xbfe07000
/*
 *  Put all machine dependent initialization here. This call
 *  is done after console has been initialized so it's safe
 *  to output configuration and debug information with printf.
 */
extern void vt82c686_init(void);
int psaux_init(void);
extern int video_hw_init(void);

extern int fb_init(unsigned long, unsigned long);
extern int dc_init();



static void init_pcidev(void)
{
	unsigned int val;
#if NMOD_VGACON > 0
	int rc = 1;
#if NMOD_FRAMEBUFFER > 0
	unsigned long fbaddress, ioaddress;
	extern struct pci_device *pcie_dev;
#endif
#endif
	*(volatile unsigned int *)0xbfe10428 &= ~(1<<19); /*disable usb prefetch*/
	val = *(unsigned int *)0xbfe10420;
	*(unsigned int *)0xbfe10420 = (val | 0xc000);//mtf, enable I2C1

	_pci_devinit(1);	/* PCI device initialization */
#if (NMOD_X86EMU_INT10 > 0)||(NMOD_X86EMU >0)
	if(pcie_dev != NULL){
		SBD_DISPLAY("VGAI", 0);
		rc = vga_bios_init();
	}
#endif
#if NMOD_FRAMEBUFFER > 0
	if (rc > 0) {
		if(pcie_dev == NULL){
			printf("begin fb_init\n");
			fbaddress = dc_init();
			printf("dc_init done\n");
			//this parameters for 1280*1024 VGA
		} else {
			fbaddress  = _pci_conf_read(pcie_dev->pa.pa_tag,0x10);
			fbaddress = fbaddress &0xffffff00; //laster 8 bit
			fbaddress |= 0x80000000;
		}
		printf("fbaddress = %08x\n", fbaddress);
		fb_init(fbaddress, 0);
		printf("fb_init done\n");
	} else {
		printf("vga bios init failed, rc=%d\n",rc);
	}
#if NSII9022A
	config_sii9022a();
#endif
#endif

#if (NMOD_FRAMEBUFFER > 0)
	if (rc > 0)
		if (!getenv("novga"))
			vga_available = 1;
		else
			vga_available = 0;
#endif

	return;
}

void tgt_devconfig()
{

	/* Enable pci device and init VGA device */
	init_pcidev();
	/*
	 * if found syn1,then set io multiplexing
	 * gmac1 use UART0,UART1
	 */
	{
		int i;
		extern struct cfdata cfdata[];
		for(i=0;cfdata[i].cf_driver;i++)
		{
			if(strcmp(cfdata[i].cf_driver->cd_name,"syn") == 0 && cfdata[i].cf_unit == 1)
			{

				*(volatile int *)0xbfe10420 |= 0x108;
				break;
			}

		}
	}

	config_init();
	configure();

#if 0//(NMOD_VGACON > 0)//mtf
//	superio_reinit();//mtf
	if (getenv("nokbd"))
		rc = 1;
	else
		rc = kbd_initialize();
	printf("%s\n", kbd_error_msgs[rc]);
	if (!rc) {
		kbd_available = 1;
	}
	psaux_init();
#endif
#ifdef SLT
	slt_test();
#endif
	printf("devconfig done.\n");

}

uint64_t cmos_read64(unsigned long addr)
{
	unsigned char bytes[8];
	int i;

	for (i = 0; i < 8; i++)
		bytes[i] = *((unsigned char *)(STR_STORE_BASE + addr + i));
	return *(uint64_t *) bytes;
}

void cmos_write64(uint64_t data, unsigned long addr)
{
	int i;
	unsigned char *bytes = (unsigned char *) &data;

	for (i = 0; i < 8; i++)
		*((unsigned char *)(STR_STORE_BASE + addr + i)) = bytes[i];
}

void check_str()
{
	uint64_t s3_ra,s3_flag;
	long long s3_sp;
	unsigned int sp_h,sp_l;
	unsigned int gpe0_stat;

	s3_ra = cmos_read64(0x40);
	s3_sp = cmos_read64(0x48);
	s3_flag = cmos_read64(0x50);

	sp_h = s3_sp >> 32;
	sp_l = s3_sp;

	if ((s3_sp < 0x9800000000000000) || (s3_ra < 0xffffffff80000000)
			|| (s3_flag != 0x5a5a5a5a5a5a5a5a)) {
		printf("S3 status no exit %llx\n", s3_flag);
		return;
	}
	/* clean s3 wake flag */
	cmos_write64(0x0, 0x40);
	cmos_write64(0x0, 0x48);
	cmos_write64(0x0, 0x50);

	/*clean s3 wake status*/
	gpe0_stat = *(volatile unsigned int *)(PM_REG_BASE + 0x28);
	gpe0_stat |= 0xfff0;
	*(volatile unsigned int *)(PM_REG_BASE + 0x28) = gpe0_stat;

	/* Enable pci device and init VGA device */
	init_pcidev();

	/* fixup pcie config */
	ls_pcie_config_set();

	/* jump to kernel... */
	__asm__ __volatile__(
			".set   noat            \n"
			".set   mips64          \n"
			"move   $t0, %0         \n"
			"move   $t1, %1         \n"
			"dli    $t2, 0x00000000ffffffff \n"
			"and    $t1,$t2         \n"
			"dsll   $t0,32          \n"
			"or $sp, $t0,$t1        \n"
			"jr %2          \n"
			"nop                \n"
			".set   at          \n"
			: /* No outputs */
			:"r"(sp_h), "r"(sp_l),"r"(s3_ra)
			);
}

extern int test_icache_1(short *addr);
extern int test_icache_2(int addr);
extern int test_icache_3(int addr);
extern void godson1_cache_flush(void);
#define tgt_putchar_uc(x) (*(void (*)(char)) (((long)tgt_putchar)|0x20000000)) (x)

extern void test_gpio_function(void);
//#ifdef LOONGSON_2K//mtf
#if 0
static int w83627_read(int dev, int addr)//mtf ??
{
	int data;
	/*enter */
	outb(0xbff00000 + 0x002e, 0x87);
	outb(0xbff00000 + 0x002e, 0x87);
	/*select logic dev reg */
	outb(0xbff00000 + 0x002e, 0x7);
	outb(0xbff00000 + 0x002f, dev);
	/*access reg */
	outb(0xbff00000 + 0x002e, addr);
	data = inb(0xbff00000 + 0x002f);
	/*exit */
	outb(0xbff00000 + 0x002e, 0xaa);
	outb(0xbff00000 + 0x002e, 0xaa);
	return data;
}

static void w83627_write(int dev, int addr, int data)
{
	/*enter */
	outb(0xbff00000 + 0x002e, 0x87);
	outb(0xbff00000 + 0x002e, 0x87);
	/*select logic dev reg */
	outb(0xbff00000 + 0x002e, 0x7);
	outb(0xbff00000 + 0x002f, dev);
	/*access reg */
	outb(0xbff00000 + 0x002e, addr);
	outb(0xbff00000 + 0x002f, data);
	/*exit */
	outb(0xbff00000 + 0x002e, 0xaa);
	outb(0xbff00000 + 0x002e, 0xaa);
}
//#endif

static void superio_reinit()
{
	w83627_write(0, 0x24, 0xc1);
	w83627_write(5, 0x30, 1);
	w83627_write(5, 0x60, 0);
	w83627_write(5, 0x61, 0x60);
	w83627_write(5, 0x62, 0);
	w83627_write(5, 0x63, 0x64);
	w83627_write(5, 0x70, 1);
	w83627_write(5, 0x72, 0xc);
	w83627_write(5, 0xf0, 0xc0);
}
#endif
void tgt_devinit()
{
	/*
	 *  Gather info about and configure caches.
	 */
	if (getenv("ocache_off")) {
		CpuOnboardCacheOn = 0;
	} else {
		CpuOnboardCacheOn = 1;
	}
	if (getenv("ecache_off")) {
		CpuExternalCacheOn = 0;
	} else {
		CpuExternalCacheOn = 1;
	}


	_pci_businit(1);        /* PCI bus initialization */
}

void tgt_reboot()//mtf
{
	*(volatile unsigned int *)0xbfe07030 = 1;
}

void tgt_poweroff()//mtf
{
	*(volatile unsigned int *)0xbfe0700c &= 0xffffffff;
	*(volatile unsigned int *)0xbfe07014 = 0x3c00;
}

/*
 *  This function makes inital HW setup for debugger and
 *  returns the apropriate setting for the status register.
 */
register_t tgt_enable(int machtype)
{
	/* XXX Do any HW specific setup */
	return (SR_COP_1_BIT | SR_FR_32 | SR_EXL);
}

/*
 *  Target dependent version printout.
 *  Printout available target version information.
 */
void tgt_cmd_vers()
{
}

/*
 *  Display any target specific logo.
 */
void tgt_logo()
{
#if 0
	printf("\n");
	printf
	    ("[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[\n");
	printf
	    ("[[[            [[[[   [[[[[[[[[[   [[[[            [[[[   [[[[[[[  [[\n");
	printf
	    ("[[   [[[[[[[[   [[[    [[[[[[[[    [[[   [[[[[[[[   [[[    [[[[[[  [[\n");
	printf
	    ("[[  [[[[[[[[[[  [[[  [  [[[[[[  [  [[[  [[[[[[[[[[  [[[  [  [[[[[  [[\n");
	printf
	    ("[[  [[[[[[[[[[  [[[  [[  [[[[  [[  [[[  [[[[[[[[[[  [[[  [[  [[[[  [[\n");
	printf
	    ("[[   [[[[[[[[   [[[  [[[  [[  [[[  [[[  [[[[[[[[[[  [[[  [[[  [[[  [[\n");
	printf
	    ("[[             [[[[  [[[[    [[[[  [[[  [[[[[[[[[[  [[[  [[[[  [[  [[\n");
	printf
	    ("[[  [[[[[[[[[[[[[[[  [[[[[  [[[[[  [[[  [[[[[[[[[[  [[[  [[[[[  [  [[\n");
	printf
	    ("[[  [[[[[[[[[[[[[[[  [[[[[[[[[[[[  [[[  [[[[[[[[[[  [[[  [[[[[[    [[\n");
	printf
	    ("[[  [[[[[[[[[[[[[[[  [[[[[[[[[[[[  [[[   [[[[[[[[   [[[  [[[[[[[   [[\n");
	printf
	    ("[[  [[[[[[[[[[[[[[[  [[[[[[[[[[[[  [[[[            [[[[  [[[[[[[[  [[\n");
	printf
	    ("[[[[[[[2005][[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[\n");
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
	int year, month, date, hour, min, sec, val;
	val = (1 << 13) | (1 << 11) | (1 << 8);
	
	outl(LS2H_RTC_CTRL_REG, val);
	outl(LS2H_TOY_TRIM_REG, 0);
	outl(LS2H_RTC_TRIM_REG, 0);

	val = inl(LS2H_TOY_READ0_REG);

	year = inl(LS2H_TOY_READ1_REG);
	month = (val >> 26) & 0x3f;
	date = (val >> 21) & 0x1f; 
	hour = (val >> 16) & 0x1f; 
	min = (val >> 10) & 0x3f; 
	sec = (val >> 4) & 0x3f; 
	if ((year < 0 || year > 138)
		|| (month < 1 || month > 12)
		|| (date < 1 || date > 31)
		|| (hour > 23) || (min > 59)
		|| (sec > 59)) {

		tgt_printf("RTC time invalid, reset to epoch.\n");
		/* 2000-01-01 00:00:00 */
		val = (1 << 26) | (1 << 21);
		outl(LS2H_TOY_WRITE1_REG, 0x64);
		outl(LS2H_TOY_WRITE0_REG, val);
	}
}

#ifdef EXTERNAL_RTC
extern int rtc_get_time(unsigned char *);
extern int rtc_set_time(unsigned char *);
extern int rtc_get_sec(void);
#endif

static void _probe_frequencies()
{
#ifdef HAVE_TOD
	int i, timeout, cur, sec, cnt;
#endif

	SBD_DISPLAY("FREQ", CHKPNT_FREQ);

	md_pipefreq = 660000000;
	md_cpufreq = 60000000;

	clk_invalid = 1;
#ifdef HAVE_TOD
#ifdef EXTERNAL_RTC
	for (i = 2; i != 0; i--) {
		timeout = 10000000;
		sec = rtc_get_sec();
		do {
			//wait 1 min.
			cur = rtc_get_sec();
		} while (cur == sec);

		cnt = CPU_GetCOUNT();
		do {
			timeout--;
			sec = rtc_get_sec();
		} while (timeout != 0 && (cur == sec));
		cnt = CPU_GetCOUNT() - cnt;
		if (timeout == 0) {
			tgt_printf("time out!\n");
			break;	/* Get out if clock is not running */
		}
	}
#elif defined(INTERNAL_RTC)
	init_legacy_rtc();

	SBD_DISPLAY("FREI", CHKPNT_FREQ);

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
#endif

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
	tgt_printf("cpu freq %u\n", md_pipefreq);
#endif /* HAVE_TOD */
}

/*
 *   Returns the CPU pipelie clock frequency
 */
int tgt_pipefreq()
{
	if (md_pipefreq == 0) 
		_probe_frequencies();

	return (md_pipefreq);
}

/*
 *   Returns the external clock frequency, usually the bus clock
 */
int tgt_cpufreq()
{
	if (md_cpufreq == 0) 
		_probe_frequencies();

	return (md_cpufreq);
}

unsigned int
read_ddrfreq()
{
#define CTRL0_LDF_MASK 0xff 
#define CTRL0_ODF_MASK 0x3 
#define CTRL0_IDF_MASK 0x7
#define CTRL0_LDF_SHIFT 24
#define CTRL0_ODF_SHIFT 22
#define CTRL0_IDF_SHIFT 19
#define SAMP_SHIFT 16
#define BOOTCFG_NAND3_4_SHIFT 7

        unsigned int ldf, odf, idf, fin, val_clock_ctrl, fref, fvco;
        unsigned int val_chip_samp, bootcfg, hw_freq, nand_d3_d4;
        val_chip_samp = inl(LS2H_CHIP_SAMP0_REG);
        bootcfg = val_chip_samp >> SAMP_SHIFT;
        if (bootcfg & 0x0200)
        {
                nand_d3_d4 = (bootcfg & 0x180) >> BOOTCFG_NAND3_4_SHIFT;
                if(nand_d3_d4 & 0x2)
                {
                        if(nand_d3_d4 & 0x1)
                                hw_freq = 100;
                        else
                                hw_freq = 100*10/3;
                }
                else
                {
                        if(nand_d3_d4 & 0x1)
                                hw_freq = 100*8/3;
                        else
                                hw_freq = 100*5/3;
                }
        }
        else
        {
                val_clock_ctrl = inl(LS2H_CLOCK_CTRL0_REG);
                ldf = ((val_clock_ctrl >> CTRL0_LDF_SHIFT) & CTRL0_LDF_MASK);
                odf = ((val_clock_ctrl >> CTRL0_ODF_SHIFT) & CTRL0_ODF_MASK);
                idf = ((val_clock_ctrl >> CTRL0_IDF_SHIFT) & CTRL0_IDF_MASK);

                fin = 100;
                fref = (fin/idf);
                fvco = (fref*2*ldf);
                hw_freq = (fvco/(1 << odf));
        }
        return (hw_freq);
}

time_t tgt_gettime()
{
	struct tm tm;
	time_t t;

#ifdef HAVE_TOD
#ifdef EXTERNAL_RTC
	unsigned char buf[7] = {0};

	t = rtc_get_time(buf);

	if (t) {
		tm.tm_sec = buf[0];
		tm.tm_min = buf[1];
		tm.tm_hour = buf[2];
		tm.tm_mday = buf[4];
		tm.tm_mon = buf[5];
		tm.tm_year = buf[6];
		if (tm.tm_year < 50)
			tm.tm_year += 100;
		tm.tm_isdst = tm.tm_gmtoff = 0;
		t = gmmktime(&tm);
	} else
#elif defined(INTERNAL_RTC)
	unsigned int val;

	if (!clk_invalid) {
		val = inl(LS2H_TOY_READ0_REG);
		tm.tm_sec = (val >> 4) & 0x3f;
		tm.tm_min = (val >> 10) & 0x3f;
		tm.tm_hour = (val >> 16) & 0x1f;
		tm.tm_mday = (val >> 21) & 0x1f;
		tm.tm_mon = ((val >> 26) & 0x3f) - 1;
		tm.tm_year = inl(LS2H_TOY_READ1_REG);
		if (tm.tm_year < 50)
			tm.tm_year += 100;
		tm.tm_isdst = tm.tm_gmtoff = 0;
		t = gmmktime(&tm);
	} else
#endif
#endif
	{
		t = 957960000;	/* Wed May 10 14:00:00 2000 :-) */
	}
	return (t);
}

/*
 *  Set the current time if a TOD clock is present
 */
void tgt_settime(time_t t)
{
	struct tm *tm;

#ifdef HAVE_TOD
#ifdef EXTERNAL_RTC
	unsigned char buf[7] = {0};
	tm = gmtime(&t);
	buf[0] = tm->tm_sec;
	buf[1] = tm->tm_min;
	buf[2] = tm->tm_hour;
	buf[4] = tm->tm_mday;
	buf[5] = (tm->tm_mon + 1);
	buf[6] = tm->tm_year;

	rtc_set_time(buf);
#elif defined(INTERNAL_RTC)
	unsigned int val;

	if (!clk_invalid) {
		tm = gmtime(&t);
		val = ((tm->tm_mon + 1) << 26) | (tm->tm_mday << 21) |
			(tm->tm_hour << 16) | (tm->tm_min << 10) |
			(tm->tm_sec << 4);
		outl(LS2H_TOY_WRITE0_REG, val);
		outl(LS2H_TOY_WRITE1_REG, tm->tm_year);
	}
#endif
#endif
}

/*
 *  Print out any target specific memory information
 */
void tgt_memprint()
{
	printf("Primary Instruction cache size %dkb (%d line, %d way)\n",
	       CpuPrimaryInstCacheSize / 1024, CpuPrimaryInstCacheLSize,
	       CpuNWayCache);
	printf("Primary Data cache size %dkb (%d line, %d way)\n",
	       CpuPrimaryDataCacheSize / 1024, CpuPrimaryDataCacheLSize,
	       CpuNWayCache);
	if (CpuSecondaryCacheSize != 0) {
		printf("Secondary cache size %dkb\n",
		       CpuSecondaryCacheSize / 1024);
	}
	if (CpuTertiaryCacheSize != 0) {
		printf("Tertiary cache size %dkb\n",
		       CpuTertiaryCacheSize / 1024);
	}
}

void tgt_machprint()
{
	printf("Copyright 2000-2002, Opsycon AB, Sweden.\n");
	printf("Copyright 2005, ICT CAS.\n");
	printf("CPU %s @", md_cpuname());
}

/*
 *  Return a suitable address for the client stack.
 *  Usually top of RAM memory.
 */

register_t tgt_clienttos()
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

struct fl_map tgt_fl_map_boot16[] = {
	TARGET_FLASH_DEVICES_16
};

struct fl_map *tgt_flashmap()
{
	return tgt_fl_map_boot16;
}

void tgt_flashwrite_disable()
{
}

int tgt_flashwrite_enable()
{
	return (1);
}

void tgt_flashinfo(void *p, size_t * t)
{
	struct fl_map *map;

	map = fl_find_map(p);
	if (map) {
		*t = map->fl_map_size;
	} else {
		*t = 0;
	}
}

void tgt_update_pmon_to_nand(unsigned char *src_addr, int size)
{
	printf("update pmom in normal mode\n");
	if ((size % 16) != 0) {
		size = (size & 0xfffffff0) + 0x10;	//16 byte allign              
	}
	nandwrite_set_badblock(src_addr, 0, size);
}

void tgt_update_pmon_to_nand_ecc(unsigned char *src_addr, int size)
{
	printf("update pmom in ecc mode\n");
	if ((size % 1880) != 0) {
		size = size - (size % 1880) + 1880;
	}
	nandwrite_set_badblock_ecc(src_addr, 0, size);
}

#ifdef  BOOT_FROM_NAND

int update_env_to_nand(char *nvram_addr, char *update_buff, int size);
int update_rom_to_nand_1block(char *nvram_addr, char *nvram_start_addr);
void nandwrite_set_badblock(unsigned char *src_addr, unsigned int nand_addr,
			    unsigned int count);

void tgt_flashprogram(void *p, int size, void *s, int endian)
{
	printf("Programming flash %x:%x into %x\n", s, size, p);

	if (p != 0xbfc00000) {
		memcpy(p, s, size);
		update_rom_to_nand_1block(p,
					  (char *)(tgt_flashmap())->
					  fl_map_base);

	} else {
		tgt_update_pmon_to_nand(s, size);
	}
}
#else
void tgt_flashprogram(void *p, int size, void *s, int endian)
{
	printf("Programming flash %x:%x into %x\n", s, size, p);
	if (fl_erase_device(p, size, TRUE)) {
		printf("Erase failed!\n");
		return;
	}
	if (fl_program_device(p, s, size, TRUE)) {
		printf("Programming failed!\n");
	}
	fl_verify_device(p, s, size, TRUE);
}

#endif
#endif /* PFLASH */

/*
 *  Network stuff.
 */
void tgt_netinit()
{
}

int tgt_ethaddr(char *p)
{
	bcopy((void *)&hwethadr, p, 6);
	return (0);
}

void tgt_netreset()
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
void tgt_mapenv(int (*func) __P((char *, char *)))
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
	nvram = (char *)(tgt_flashmap())->fl_map_base;
	printf("nvram=%08x\n", (unsigned int)nvram);
	if (fl_devident(nvram, NULL) == 0 ||
	    cksum(nvram + NVRAM_OFFS, NVRAM_SIZE, 0) != 0) {
#else
	nvram = (char *)malloc(512);
	nvram_get(nvram);
	if (cksum(nvram, NVRAM_SIZE, 0) != 0) {
#endif
		printf("NVRAM is invalid!\n");
		nvram_invalid = 1;
	} else {
#ifdef NVRAM_IN_FLASH
		nvram += NVRAM_OFFS;
		ep = nvram + 2;;

		while (*ep != 0) {
			char *val = 0, *p = env;
			i = 0;
			while ((*p++ = *ep++) && (ep <= nvram + NVRAM_SIZE - 1)
			       && i++ < 255) {
				if ((*(p - 1) == '=') && (val == NULL)) {
					*(p - 1) = '\0';
					val = p;
				}
			}
			if (ep <= nvram + NVRAM_SIZE - 1 && i < 255) {
				(*func) (env, val);
			} else {
				nvram_invalid = 2;
				break;
			}
		}
#endif
	}

	printf("NVRAM@%x\n", (u_int32_t) nvram);
#ifdef NVRAM_IN_FLASH
	printf("ACTIVECOM_OFFS = %d, = 0x%x\n", ACTIVECOM_OFFS, ACTIVECOM_OFFS);
	printf("MASTER_BRIDGE_OFFS = %d, = 0x%x\n", MASTER_BRIDGE_OFFS,
	       MASTER_BRIDGE_OFFS);
	printf("before :activecom = %d. em_enable = %d\n", activecom,
	       em_enable);
//      printf("nuram[MASTER_BRIDGE_OFFS] = %d.\n" nvram[MASTER_BRIDGE_OFFS]);
	if (!nvram_invalid)
		bcopy(&nvram[ACTIVECOM_OFFS], &activecom, 1);
	else
		activecom = 3 /*1 */ ;
	sprintf(env, "0x%02x", activecom);
	(*func) ("activecom", env);	/*tangyt */

	if (!nvram_invalid)
		bcopy(&nvram[MASTER_BRIDGE_OFFS], &em_enable, 1);
	else
		em_enable = 3 /*1 */ ;
	sprintf(env, "0x%02x", em_enable);
	(*func) ("em_enable", env);	/*tangyt */

	printf("activecom = %d.   em_enable = %d.\n", activecom, em_enable);
#endif
	/*
	 *  Ethernet address for Galileo ethernet is stored in the last
	 *  six bytes of nvram storage. Set environment to it.
	 */

	bcopy(&nvram[ETHER_OFFS], hwethadr, 6);
	sprintf(env, "%02x:%02x:%02x:%02x:%02x:%02x", hwethadr[0], hwethadr[1],
		hwethadr[2], hwethadr[3], hwethadr[4], hwethadr[5]);
	(*func) ("ethaddr", env);

#ifndef NVRAM_IN_FLASH
	free(nvram);
#endif

#ifdef no_thank_you
	(*func) ("vxWorks", env);
#endif

	sprintf(env, "%d", memorysize / (1024 * 1024));
	(*func) ("memsize", env);

	sprintf(env, "%d", memorysize_high / (1024 * 1024));
	(*func) ("highmemsize", env);

	sprintf(env, "%d", md_pipefreq);
	(*func) ("cpuclock", env);

	sprintf(env, "%d", md_cpufreq);
	(*func) ("busclock", env);

	(*func) ("systype", SYSTYPE);

}

int tgt_unsetenv(char *name)
{
	char *ep, *np, *sp;
	char *nvram;
	char *nvrambuf;
	char *nvramsecbuf;
	int status;

	if (nvram_invalid) {
		return (0);
	}

	/* Use first defined flash device (we probably have only one) */
#ifdef NVRAM_IN_FLASH
	nvram = (char *)(tgt_flashmap())->fl_map_base;

	/* Map. Deal with an entire sector even if we only use part of it */
	nvram += NVRAM_OFFS & ~(NVRAM_SECSIZE - 1);
	nvramsecbuf = (char *)malloc(NVRAM_SECSIZE);
	if (nvramsecbuf == 0) {
		printf("Warning! Unable to malloc nvrambuffer!\n");
		return (-1);
	}
	memcpy(nvramsecbuf, nvram, NVRAM_SECSIZE);
	nvrambuf = nvramsecbuf + (NVRAM_OFFS & (NVRAM_SECSIZE - 1));
#else
	nvramsecbuf = nvrambuf = nvram = (char *)malloc(512);
	nvram_get(nvram);
#endif

	ep = nvrambuf + 2;

	status = 0;
#ifdef NVRAM_IN_FLASH
	while ((*ep != '\0') && (ep <= nvrambuf + NVRAM_SIZE)) {
		np = name;
		sp = ep;

		while ((*ep == *np) && (*ep != '=') && (*np != '\0')) {
			ep++;
			np++;
		}
		if ((*np == '\0') && ((*ep == '\0') || (*ep == '='))) {
			while (*ep++) ;
			while (ep <= nvrambuf + NVRAM_SIZE) {
				*sp++ = *ep++;
			}
			if (nvrambuf[2] == '\0') {
				nvrambuf[3] = '\0';
			}
			cksum(nvrambuf, NVRAM_SIZE, 1);
#ifdef NVRAM_IN_FLASH

#ifdef BOOT_FROM_NAND
			memcpy(nvram, nvramsecbuf, NVRAM_SECSIZE);
			//update_env_to_nand( nvram, nvramsecbuf,NVRAM_SECSIZE);
			update_rom_to_nand_1block(nvram,
						  (char *)(tgt_flashmap())->
						  fl_map_base);

#else
			if (fl_erase_device(nvram, NVRAM_SECSIZE, FALSE)) {
				status = -1;
				break;
			}

			if (fl_program_device
			    (nvram, nvramsecbuf, NVRAM_SECSIZE, FALSE)) {
				status = -1;
				break;
			}
#endif

#else
			nvram_put(nvram);
#endif
			status = 1;
			break;
		} else if (*ep != '\0') {
			while (*ep++ != '\0') ;
		}
	}
#endif
	free(nvramsecbuf);
	return (status);
}

#ifdef   BOOT_FROM_NAND

int cmp_test_mem(char *dist, char *src, int size)
{
	int i, j, count;
	j = 0;
	count = 0;
	for (i = 0; i < size; i++) {

		if (*dist != *src) {
			count++;
			if (j <= 10) {
				printf("dist != num.addr is :0x%x\n\n", src);

			}
			j++;
		}
		dist++;
		src++;
	}
	if (count == 0) {
		printf("dist == num.\n");
	} else {

		printf("dist != num. count:%d\n", count);
	}
	return 0;
}

#define NAND_TEMP_ADDR ((volatile unsigned char *)0x86200000)

int update_env_to_nand(char *nvram_addr, char *update_buff, int size)
{
	int block_num;
	char *block_addr;
	int block_offset;
	char *nand_update_ramaddr;
	char *test_addr;

	block_addr =
	    (char *)(((unsigned int)NVRAM_OFFS + 0x400) & (~(0x20000 - 1)));
	// block_addr = ((unsigned int )(nvram_addr) &(~(0x20000 - 1)));
	nandread(NAND_TEMP_ADDR, block_addr, 0x20000);	//read 1 block to ram

	// block_offset = nvram_addr - block_addr; 
	block_offset = NVRAM_OFFS + 0x400 - (unsigned int)block_addr;
	nand_update_ramaddr = NAND_TEMP_ADDR + block_offset;

	memcpy(nand_update_ramaddr, update_buff, size);	//update env in ram

	block_num = (unsigned int)block_addr / (2048 * 64);
	block_erase(block_num, 1);

	nandwrite(NAND_TEMP_ADDR, block_addr, 0x20000);	//write ram to nand for 1 block

	return 0;
}

#define REMAP_DDR_DMA 0x00e00000
#define ROM_OFFSET    0x400
#define ROM_OFFSET_ECC    0xeb0
#define NANDBLOCK_SIZE 0x20000
#define NANDBLOCK_SIZE_ECC (188*10*64)
#define NAND_PAGE_SIZE 2048
#define NAND_PAGE_SIZE_ECC 1880
char *romaddr_to_ramaddr(char *romaddr)
{
	char *ramaddr;

	ramaddr = (((unsigned int)romaddr) & (~0xfff00000));
	ramaddr = ((unsigned int)ramaddr) | REMAP_DDR_DMA;
	return ramaddr;
}

#ifndef NAND_ECC_MODE

int update_rom_to_nand_1block(char *nvram_addr, char *nvram_start_addr)	//normal mode
{
	int block_num;
	char *nand_block_addr, *rom_block_addr;
	rom_block_addr =
	    (char
	     *)((((unsigned int)nvram_addr +
		  ROM_OFFSET) & (~(NANDBLOCK_SIZE - 1))) - ROM_OFFSET);
	rom_block_addr = romaddr_to_ramaddr(rom_block_addr);
	nand_block_addr =
	    (nvram_addr - nvram_start_addr +
	     ROM_OFFSET) & (~(NANDBLOCK_SIZE - 1));
	block_num = (unsigned int)nand_block_addr / NANDBLOCK_SIZE;

	nandwrite_set_badblock(rom_block_addr, nand_block_addr, NANDBLOCK_SIZE);	//write ram to nand for 1 block

	return 0;
}
#else

int update_rom_to_nand_1block(char *nvram_addr, char *nvram_start_addr)	//ecc mode
{
	int block_num, nand_count;
	char *nand_block_addr, *rom_block_addr;
	rom_block_addr =
	    ((unsigned int)nvram_addr) - ((unsigned int)nvram_start_addr) +
	    ROM_OFFSET_ECC;
	rom_block_addr =
	    ((unsigned int)rom_block_addr) -
	    (((unsigned int)rom_block_addr) % NANDBLOCK_SIZE_ECC) -
	    ROM_OFFSET_ECC;
	rom_block_addr = romaddr_to_ramaddr(rom_block_addr);
	nand_count = nvram_addr - nvram_start_addr + ROM_OFFSET_ECC;
	nand_block_addr =
	    (nand_count / NAND_PAGE_SIZE_ECC) * NAND_PAGE_SIZE +
	    ((nand_count % NAND_PAGE_SIZE_ECC) / 188) * 204;
	nand_block_addr =
	    ((unsigned int)nand_block_addr) & (~(NANDBLOCK_SIZE - 1));

	nandwrite_set_badblock_ecc(rom_block_addr, nand_block_addr, NANDBLOCK_SIZE_ECC);	//write ram to nand for 1 block

	return 0;
}

#endif

int check_bad_block_pt(void)
{
	int i;
	for (i = 0; i < 16; i++) {
		NAND_TEMP_ADDR[i] = 0;
	}
	nandwrite_spare(NAND_TEMP_ADDR, 333 * 2048 * 64, 16);
	nandwrite_spare(NAND_TEMP_ADDR, 444 * 2048 * 64, 16);
	nandwrite_spare(NAND_TEMP_ADDR, 555 * 2048 * 64, 16);
	for (i = 0; i < 1024; i++) {
		nandread_spare(NAND_TEMP_ADDR, i * 2048 * 64, 16);
		if (NAND_TEMP_ADDR[0] != 0xff) {
			printf("bad block addr: 0x%x,num: %d\n", i * 2048 * 64,
			       i);
		}
	}
}

void tgt_nand_test_print(void)
{
	int i, j, k;
	int block_num;
	//nandread_check_badblock_ecc(0x86000000,0 ,188*10*64*4 );//write ram to nand for 1 block

	//cmp_test_mem(0x85000000  ,0x86000000 + 1880*2 ,188*10*64*4  - 1880*2 );
}
#endif

int tgt_setenv(char *name, char *value)
{
	char *ep;
	int envlen;
	char *nvrambuf;
	char *nvramsecbuf;
#ifdef NVRAM_IN_FLASH
	char *nvram;
#endif

	/* Non permanent vars. */
	if (strcmp(EXPERT, name) == 0) {
		return (1);
	}

	/* Calculate total env mem size requiered */
	envlen = strlen(name);
	if (envlen == 0) {
		return (0);
	}
	if (value != NULL) {
		envlen += strlen(value);
	}
	envlen += 2;		/* '=' + null byte */
	if (envlen > 255) {
		return (0);	/* Are you crazy!? */
	}

	/* Use first defined flash device (we probably have only one) */
#ifdef NVRAM_IN_FLASH
	nvram = (char *)(tgt_flashmap())->fl_map_base;

	/* Deal with an entire sector even if we only use part of it */
	nvram += NVRAM_OFFS & ~(NVRAM_SECSIZE - 1);
#endif

	/* If NVRAM is found to be uninitialized, reinit it. */
	if (nvram_invalid) {
		nvramsecbuf = (char *)malloc(NVRAM_SECSIZE);
		if (nvramsecbuf == 0) {
			printf("Warning! Unable to malloc nvrambuffer!\n");
			return (-1);
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

#ifdef BOOT_FROM_NAND
		memcpy(nvram, nvramsecbuf, NVRAM_SECSIZE);
		//update_env_to_nand( nvram, nvramsecbuf,NVRAM_SECSIZE);
		update_rom_to_nand_1block(nvram,
					  (char *)(tgt_flashmap())->
					  fl_map_base);
#else

		if (fl_erase_device(nvram, NVRAM_SECSIZE, FALSE)) {
			printf("Error! Nvram erase failed!\n");
			free(nvramsecbuf);
			return (-1);
		}
		if (fl_program_device(nvram, nvramsecbuf, NVRAM_SECSIZE, FALSE)) {
			printf("Error! Nvram init failed!\n");
			free(nvramsecbuf);
			return (-1);
		}
#endif

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
	if (nvramsecbuf == 0) {
		printf("Warning! Unable to malloc nvrambuffer!\n");
		return (-1);
	}
#ifndef NVRAM_IN_FLASH
	nvram_get(nvramsecbuf);
#else
	memcpy(nvramsecbuf, nvram, NVRAM_SECSIZE);
#endif
	nvrambuf = nvramsecbuf + (NVRAM_OFFS & (NVRAM_SECSIZE - 1));
	/* Etheraddr is special case to save space */
#if 1				/*added by tangyt */
	if (strcmp("activecom", name) == 0) {
		activecom = strtoul(value, 0, 0);
		printf("set activecom to com %d\n", activecom);
	} else if (strcmp("em_enable", name) == 0) {
		em_enable = strtoul(value, 0, 0);
		printf("set em_enable to com %d\n", em_enable);
	} else
#endif
	if (strcmp("ethaddr", name) == 0) {
		char *s = value;
		int i;
		int32_t v;
		for (i = 0; i < 6; i++) {
			gethex(&v, s, 2);
			hwethadr[i] = v;
			s += 3;	/* Don't get to fancy here :-) */
		}
	} else {
		ep = nvrambuf + 2;
		if (*ep != '\0') {
			do {
				while (*ep++ != '\0') ;
			} while (*ep++ != '\0');
			ep--;
		}
		if (((int)ep + NVRAM_SIZE - (int)ep) < (envlen + 1)) {
			free(nvramsecbuf);
			return (0);	/* Bummer! */
		}

		/*
		 *  Special case heaptop must always be first since it
		 *  can change how memory allocation works.
		 */
		if (strcmp("heaptop", name) == 0) {

			bcopy(nvrambuf + 2, nvrambuf + 2 + envlen,
			      ep - nvrambuf + 1);

			ep = nvrambuf + 2;
			while (*name != '\0') {
				*ep++ = *name++;
			}
			if (value != NULL) {
				*ep++ = '=';
				while ((*ep++ = *value++) != '\0') ;
			} else {
				*ep++ = '\0';
			}
		} else {
			while (*name != '\0') {
				*ep++ = *name++;
			}
			if (value != NULL) {
				*ep++ = '=';
				while ((*ep++ = *value++) != '\0') ;
			} else {
				*ep++ = '\0';
			}
			*ep++ = '\0';	/* End of env strings */
		}
	}
	cksum(nvrambuf, NVRAM_SIZE, 1);
#ifdef NVRAM_IN_FLASH
	bcopy(&activecom, &nvrambuf[ACTIVECOM_OFFS], 1);
	bcopy(&em_enable, &nvrambuf[MASTER_BRIDGE_OFFS], 1);
#endif
	bcopy(hwethadr, &nvramsecbuf[ETHER_OFFS], 6);
#ifdef NVRAM_IN_FLASH

#ifdef BOOT_FROM_NAND

	memcpy(nvram, nvramsecbuf, NVRAM_SECSIZE);
	//update_env_to_nand( nvram, nvramsecbuf,NVRAM_SECSIZE);
	update_rom_to_nand_1block(nvram, (char *)(tgt_flashmap())->fl_map_base);
#else

	if (fl_erase_device(nvram, NVRAM_SECSIZE, FALSE)) {
		printf("Error! Nvram erase failed!\n");
		free(nvramsecbuf);
		return (0);
	}
	if (fl_program_device(nvram, nvramsecbuf, NVRAM_SECSIZE, FALSE)) {
		printf("Error! Nvram program failed!\n");
		free(nvramsecbuf);
		return (0);
	}
#endif

#else
	nvram_put(nvramsecbuf);
#endif
	free(nvramsecbuf);
	return (1);
}

/*
 *  Calculate checksum. If 'set' checksum is calculated and set.
 */
static int cksum(void *p, size_t s, int set)
{
	u_int16_t sum = 0;
	u_int8_t *sp = p;
	int sz = s / 2;

	if (set) {
		*sp = 0;	/* Clear checksum */
		*(sp + 1) = 0;	/* Clear checksum */
	}
	while (sz--) {
		sum += (*sp++) << 8;
		sum += *sp++;
	}
	if (set) {
		sum = -sum;
		*(u_int8_t *) p = sum >> 8;
		*((u_int8_t *) p + 1) = sum;
	}
	return (sum);
}

#ifndef NVRAM_IN_FLASH

/*
 *  Read and write data into non volatile memory in clock chip.
 */
void nvram_get(char *buffer)
{
	int i;
	for (i = 0; i < 114; i++) {
		linux_outb(i + RTC_NVRAM_BASE, RTC_INDEX_REG);	/* Address */
		buffer[i] = linux_inb(RTC_DATA_REG);
	}
}

void nvram_put(char *buffer)
{
	int i;
	for (i = 0; i < 114; i++) {
		linux_outb(i + RTC_NVRAM_BASE, RTC_INDEX_REG);	/* Address */
		linux_outb(buffer[i], RTC_DATA_REG);
	}
}

#endif

/*
 *  Simple display function to display a 4 char string or code.
 *  Called during startup to display progress on any feasible
 *  display before any serial port have been initialized.
 */
void tgt_display(char *msg, int x)
{
	/* Have simple serial port driver */
	tgt_putchar(msg[0]);
	tgt_putchar(msg[1]);
	tgt_putchar(msg[2]);
	tgt_putchar(msg[3]);
	tgt_putchar('\r');
	tgt_putchar('\n');
}

void clrhndlrs()
{
}

int tgt_getmachtype()
{
	return (md_cputype());
}

/*
 *  Create stubs if network is not compiled in
 */
#ifdef INET
void tgt_netpoll()
{
	splx(splhigh());
}

#else
extern void longjmp(label_t *, int);
void gsignal(label_t * jb, int sig);
void gsignal(label_t * jb, int sig)
{
	if (jb != NULL) {
		longjmp(jb, 1);
	}
};

int netopen(const char *, int);
int netread(int, void *, int);
int netwrite(int, const void *, int);
long netlseek(int, long, int);
int netioctl(int, int, void *);
int netclose(int);
int netopen(const char *p, int i)
{
	return -1;
}

int netread(int i, void *p, int j)
{
	return -1;
}

int netwrite(int i, const void *p, int j)
{
	return -1;
}

int netclose(int i)
{
	return -1;
}

long int netlseek(int i, long j, int k)
{
	return -1;
}

int netioctl(int j, int i, void *p)
{
	return -1;
}

void tgt_netpoll()
{
};

#endif /*INET*/

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
#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

struct pci_config_data pci_config_array[] = {
			/*		APB		*/
[0] = {
.bus = 0, .dev = 0x2, .func = 0, .interrupt = 0, .primary = 0, .secondary = 0,
.subordinate = 0, .mem_start = 0x1fe00000, .mem_end = 0x1fe0ffff, .type = PCI_DEV,
},
			/*		GMAC0	*/
[1] = {
.bus = 0, .dev = 0x3, .func = 0, .interrupt = 20, .primary = 0, .secondary = 0,
.subordinate = 0, .mem_start = 0x40040000, .mem_end = 0x4004ffff, .type = PCI_DEV,
},
			/*		GMAC1	*/
[2] = {
.bus = 0, .dev = 0x3, .func = 1, .interrupt = 22, .primary = 0, .secondary = 0,
.subordinate = 0, .mem_start = 0x40050000, .mem_end = 0x4005ffff, .type = PCI_DEV,
},
			/*		OTG		*/
[3] = {
.bus = 0, .dev = 0x4, .func = 0, .interrupt = 57, .primary = 0, .secondary = 0,
.subordinate = 0, .mem_start = 0x40000000, .mem_end = 0x4003ffff, .type = PCI_DEV,
},
			/*		EHCI	*/
[4] = {
.bus = 0, .dev = 0x4, .func = 1, .interrupt = 58, .primary = 0, .secondary = 0,
.subordinate = 0, .mem_start = 0x40060000, .mem_end = 0x4006ffff, .type = PCI_DEV,
},
			/*		OHCI	*/
[5] = {
.bus = 0, .dev = 0x4, .func = 2, .interrupt = 59, .primary = 0, .secondary = 0,
.subordinate = 0, .mem_start = 0x40070000, .mem_end = 0x4007ffff, .type = PCI_DEV,
},
			/*		GPU		*/
[6] = {
.bus = 0, .dev = 0x5, .func = 0, .interrupt = 37, .primary = 0, .secondary = 0,
.subordinate = 0, .mem_start = 0x40080000, .mem_end = 0x400bffff, .type = PCI_DEV,
},
			/*		DC		*/
[7] = {
.bus = 0, .dev = 0x6, .func = 0, .interrupt = 36, .primary = 0, .secondary = 0,
.subordinate = 0, .mem_start = 0x400c0000, .mem_end = 0x400cffff, .type = PCI_DEV,
},
			/*		HDA		*/
[8] = {
.bus = 0, .dev = 0x7, .func = 0, .interrupt = 12, .primary = 0, .secondary = 0,
.subordinate = 0, .mem_start = 0x400d0000, .mem_end = 0x400dffff, .type = PCI_DEV,
},
			/*		SATA	*/
[9] = {
.bus = 0, .dev = 0x8, .func = 0, .interrupt = 27, .primary = 0, .secondary = 0,
.subordinate = 0, .mem_start = 0x400e0000, .mem_end = 0x400effff, .type = PCI_DEV,
},
			/*	PCIE0-PORT0	*/
[10] = {
.bus = 0, .dev = 0x9, .func = 0, .interrupt = 40, .primary = 0, .secondary = 1,
.subordinate = 1, .mem_start = 0x40100000, .mem_end = 0x4fffffff, .type = PCI_BRIDGE,
.io_start = 0x18000000, .io_end = 0x180fffff,
},
			/*	PCIE0-PORT1	*/
[11] = {
.bus = 0, .dev = 0xa, .func = 0, .interrupt = 41, .primary = 0, .secondary = 4,
.subordinate = 4, .mem_start = 0x50000000, .mem_end = 0x53ffffff, .type = PCI_BRIDGE,
.io_start = 0x18100000, .io_end = 0x181fffff,
},
			/*	PCIE0-PORT2	*/
[12] = {
.bus = 0, .dev = 0xb, .func = 0, .interrupt = 42, .primary = 0, .secondary = 8,
.subordinate = 8, .mem_start = 0x54000000, .mem_end = 0x57ffffff, .type = PCI_BRIDGE,
.io_start = 0x18200000, .io_end = 0x182fffff,
},
			/*	PCIE0-PORT3	*/
[13] = {
.bus = 0, .dev = 0xc, .func = 0, .interrupt = 43, .primary = 0, .secondary = 0xc,
.subordinate = 0xc, .mem_start = 0x58000000, .mem_end = 0x5fffffff, .type = PCI_BRIDGE,
.io_start = 0x18300000, .io_end = 0x183fffff,
},
			/*	PCIE1-PORT0	*/
[14] = {
.bus = 0, .dev = 0xd, .func = 0, .interrupt = 44, .primary = 0, .secondary = 0x10,
.subordinate = 0x10, .mem_start = 0x60000000, .mem_end = 0x77ffffff, .type = PCI_BRIDGE,
.io_start = 0x18400000, .io_end = 0x184fffff,
},
			/*	PCIE1-PORT1	*/
[15] = {
.bus = 0, .dev = 0xe, .func = 0, .interrupt = 45, .primary = 0, .secondary = 0x14,
.subordinate = 0x14, .mem_start = 0x78000000, .mem_end = 0x7fffffff, .type = PCI_BRIDGE,
.io_start = 0x18500000, .io_end = 0x185fffff,
},

};

int pci_config_array_size = ARRAY_SIZE(pci_config_array);
//typedef unsigned long long u64;
u64  __raw__readq(u64 addr);
u64 __raw__writeq(u64 addr, u64 val);
void ls_pcie_config_set(void)
{
	int i;
	if(getenv("oldpmon")) return;

	for(i = 0;i < ARRAY_SIZE(pci_config_array);i++){
			//ls_pcie_mem_fixup(pci_config_array + i);
			ls_pcie_interrupt_fixup(pci_config_array + i);
			//ls_pcie_busnr_fixup(pci_config_array + i);
			ls_pcie_payload_fixup(pci_config_array + i);
	}
	ls_pci_msi_window_config();

	if(!ls2k_version())
		ls_set_io_noncoherent();
	else
		map_gpu_addr();
}

extern unsigned long long memorysize_total;
void map_gpu_addr(void)
{
    flushcache();

	if (memorysize_total == 0x800) {
		__raw__writeq(0x900000001fe10038 , 0x20000000);
		__raw__writeq(0x900000001fe10078 , 0xffffffffe0000000);
		__raw__writeq(0x900000001fe100b8 , 0x00000000600000f0);
	} else if (memorysize_total == 0x1000) {
		__raw__writeq(0x900000001fe10038 , 0x20000000);
		__raw__writeq(0x900000001fe10078 , 0xffffffffe0000000);
		__raw__writeq(0x900000001fe100b8 , 0x00000000e00000f0);
	} else if (memorysize_total == 0x2000) {
		__raw__writeq(0x900000001fe10038 , 0x20000000);
		__raw__writeq(0x900000001fe10078 , 0xffffffffe0000000);
		__raw__writeq(0x900000001fe100b8 , 0x00000001e00000f0);
	} else {
		tgt_printf ("Now this Memory size %lld MB is not support mapping GPU address.\n", memorysize_total);
	}
}

void ls_set_io_noncoherent(void)
{
		u64 val;

		val = __raw__readq(0x900000001fe10420);
		val &= 0xffffff8fffffffeULL; //pcie, usb, hda, gmac
		__raw__writeq(0x900000001fe10420 , val);

		val = __raw__readq(0x900000001fe10430);
		val &= 0xffffffffffffff3ULL; //dc, gpu
		__raw__writeq(0x900000001fe10430 , val);

		val = __raw__readq(0x900000001fe10450);
		val &= 0xffffffffffffbffULL; //sata
		__raw__writeq(0x900000001fe10450 , val);

		val = __raw__readq(0x900000001fe10c00);
		val |= 0x2; //apbdma0
		__raw__writeq(0x900000001fe10c00 , val);

		val = __raw__readq(0x900000001fe10c10);
		val |= 0x2; //apbdma1
		__raw__writeq(0x900000001fe10c10 , val);

		val = __raw__readq(0x900000001fe10c20);
		val |= 0x2; //apbdma2
		__raw__writeq(0x900000001fe10c20 , val);

		val = __raw__readq(0x900000001fe10c30);
		val |= 0x2; //apbdma3
		__raw__writeq(0x900000001fe10c30 , val);

		val = __raw__readq(0x900000001fe10c40);
		val |= 0x2; //apbdma4
		__raw__writeq(0x900000001fe10c40 , val);
}

void ls_pci_msi_window_config(void)
{
	/*config msi window*/
	__raw__writeq(0x900000001fe12500 , 0x000000001fe10000ULL);
	__raw__writeq(0x900000001fe12540 , 0xffffffffffff0000ULL);
	__raw__writeq(0x900000001fe12580 , 0x000000001fe10081ULL);
}


#define LS2K_PCI_IO_MASK 0x1ffffff

void ls_pcie_mem_fixup(struct pci_config_data *pdata)
{
	unsigned int dev;
	unsigned int val;
	unsigned int io_start;
	unsigned int io_end;

	dev = _pci_make_tag(pdata->bus, pdata->dev, pdata->func);
	val = _pci_conf_read32(dev, 0x00);
	/*	device on the slot	*/
	if ( val != 0xffffffff){
			if(pdata->type == PCI_DEV){
					/*write bar*/
					_pci_conf_write32(dev, 0x10, pdata->mem_start);
			}else{
					_pci_conf_write32(dev, 0x10, 0x0);
					/*write memory base and memory limit*/
					val = ((pdata->mem_start >> 16)&0xfff0)|(pdata->mem_end&0xfff00000);
					_pci_conf_write32(dev, 0x20, val);
					_pci_conf_write32(dev, 0x24, val);

					io_start = pdata->io_start & LS2K_PCI_IO_MASK;
					io_end = pdata->io_end & LS2K_PCI_IO_MASK;
					/*write io upper 16bit base and io upper 16bit limit*/
					val = ((io_start >> 16)&0xffff)|(io_end&0xffff0000);
					_pci_conf_write32(dev, 0x30, val);
					/*write io base and io limit*/
					val = ((io_start >> 8)&0xf0)|(io_end & 0xf000);
					val|= 0x1 | (0x1 << 8);
					_pci_conf_write16(dev, 0x1c, val);
			}
	}
}

void ls_pcie_busnr_fixup(struct pci_config_data *pdata)
{
	unsigned int dev;
	unsigned int val;

	dev = _pci_make_tag(pdata->bus, pdata->dev, pdata->func);
	val = _pci_conf_read32(dev, 0x00);
	/*	device on the slot	*/
	if ( val != 0xffffffff){
			if(pdata->type == PCI_BRIDGE){
					/*write primary ,secondary and subordinate*/
					val = pdata->primary |(pdata->secondary << 8)|(pdata->subordinate << 16);
					_pci_conf_write32(dev, 0x18, val);
			}
	}
}

void ls_pcie_interrupt_fixup(struct pci_config_data *pdata)
{
	unsigned int dev;
	unsigned int val;

	dev = _pci_make_tag(pdata->bus, pdata->dev, pdata->func);
	val = _pci_conf_read32(dev, 0x00);
	/*	device on the slot	*/
	if ( val != 0xffffffff)
			_pci_conf_write16(dev, 0x3c, pdata->interrupt|0x100);

	//mask the unused device
#if 0
#define PCICFG30_RECFG	0xbfe13808 /*GMAC0*/
#define PCICFG31_RECFG	0xbfe13810 /*GMAC1*/
#define PCICFG40_RECFG	0xbfe13818 /*OTG*/
#define PCICFG41_RECFG	0xbfe13820 /*EHCI*/
#define PCICFG42_RECFG	0xbfe13828 /*OHCI*/
#define PCICFG5_RECFG	0xbfe13830 /*GPU*/
#define PCICFG6_RECFG	0xbfe13838 /*DC*/
#define PCICFG7_RECFG	0xbfe13840 /*HDA*/
#define PCICFG8_RECFG	0xbfe13848 /*SATA*/
#define PCICFGf_RECFG	0xbfe13850 /*DMA*/
	//pcicfg31_recfg for gmac1
	inl(PCICFG31_RECFG) |= 0xf;
	dev = _pci_make_tag(0, 3, 1);
	_pci_conf_write32(dev, 0, 0xffffffff);
	inl(PCICFG31_RECFG) &= 0xfffffff0;
#endif
}
#define  PCI_EXP_DEVCTL_READRQ  0x7000	/* Max_Read_Request_Size */
#define  PCI_EXP_DEVCTL_PAYLOAD 0x00e0  /* Max_Payload_Size */

void ls_pcie_payload_fixup(struct pci_config_data *pdata)
{
	unsigned int dev;
	unsigned int val;
	u16 max_payload_spt, control;

	dev = _pci_make_tag(pdata->bus, pdata->dev, pdata->func);
	val = _pci_conf_read32(dev, 0x00);
	/*	device on the slot	*/
	if ( val != 0xffffffff){
			if(pdata->type == PCI_BRIDGE){
					/*set Max_Payload_Size & Max_Read_Request_Size*/
					max_payload_spt = 1;
					control = _pci_conf_read16(dev, 0x78);
					control &= (~PCI_EXP_DEVCTL_PAYLOAD & ~PCI_EXP_DEVCTL_READRQ);
					control |= ((max_payload_spt << 5) | (max_payload_spt << 12));
					_pci_conf_write16(dev, 0x78, control);
			}
	}
}

extern struct efi_memory_map_loongson g_map;
extern unsigned long long memorysize;
extern unsigned long long memorysize_high;

#include "../../../pmon/cmds/bootparam.h"
struct efi_memory_map_loongson * init_memory_map()
{
#if 0
	struct efi_memory_map_loongson *emap = &g_map;
	int i = 0;
	unsigned long long size = memorysize_high;

	emap->nr_map = 7;
	emap->mem_freq = 266000000; //266Mhz
#define EMAP_ENTRY(entry, node, type, start, size) \
	emap->map[(entry)].node_id = (node), \
	emap->map[(entry)].mem_type = (type), \
	emap->map[(entry)].mem_start = (start), \
	emap->map[(entry)].mem_size = (size), \
	(entry)++

 	EMAP_ENTRY(i, 0, SYSTEM_RAM_LOW, 0x00200000, 0x98);
 	 /* for entry with mem_size < 1M, we set bit31 to 1 to indicate
 	  * that the unit in mem_size is Byte not MBype*/
 	EMAP_ENTRY(i, 0, SYSTEM_RAM_HIGH, 0x110000000, atoi(getenv("ihighmemsize")) - VRAM_SIZE);
 	EMAP_ENTRY(i, 0, SMBIOS_TABLE, (SMBIOS_PHYSICAL_ADDRESS & 0x0fffffff),
 			(SMBIOS_SIZE_LIMIT >> 20));
 	EMAP_ENTRY(i, 0, UMA_VIDEO_RAM, 0x110000000, VRAM_SIZE);

	emap->vers = 1;
#else
	struct efi_memory_map_loongson *emap = &g_map;
	
	emap->nr_map = 7;
	emap->mem_freq = 266000000; //266Mhz
	
	//SYSTEM_RAM_LOW
	emap->map[0].node_id = 0;
	emap->map[0].mem_type = 1;
	emap->map[0].mem_start = 0x00200000;
	//gpu and frame buffer address 0x0a00_0000~0x0f00_0000
	emap->map[0].mem_size = 0x98;
	
	//SYSTEM_RAM_HIGH
	emap->map[1].node_id = 0;
	emap->map[1].mem_type = 2;
	emap->map[1].mem_start = 0x110000000 + (VRAM_SIZE << 20);
	emap->map[1].mem_size = atoi(getenv("highmemsize")) - VRAM_SIZE;
	
	//SMBIOS_TABLE
	emap->map[2].node_id = 0;
	emap->map[2].mem_type = 10;
	emap->map[2].mem_start = SMBIOS_PHYSICAL_ADDRESS & 0x0fffffff;
	emap->map[2].mem_size = SMBIOS_SIZE_LIMIT >> 20;
	
	//UMA_VIDEO_RAM
	emap->map[6].node_id = 0;
	emap->map[6].mem_type = 11;
	emap->map[6].mem_start = 0x110000000;
	emap->map[6].mem_size = VRAM_SIZE;
#endif
	return emap;
}
