/*	$Id: tgt_machdep.c,v 1.2 2006/06/30 22:57:38 cpu Exp $ */

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
#undef DEEP_DEBUG

#if 0
#ifdef USE_LEGACY_RTC
#	undef NVRAM_IN_FLASH 
#else
#	define NVRAM_IN_FLASH 1
#endif
#endif

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

#include <linux/io.h>
#ifdef USE_LEGACY_RTC
#include <dev/ic/mc146818reg.h>
#else
#include <dev/ic/ds15x1reg.h>
#endif

#include <autoconf.h>

#include <machine/cpu.h>
#include <machine/pio.h>
#include "pflash.h"
#include "dev/pflash_tgt.h"

#include "include/ev64420.h"
#include <pmon/dev/gt64420reg.h>
#include <pmon/dev/ns16550.h>

#include <pmon.h>
#include <sys/ioctl.h>

#include "mod_x86emu.h"
#include "mod_x86emu_int10.h"
#include "mod_vgacon.h"
extern void vga_bios_init(void);
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

#if NMOD_X86EMU_INT10 == 0
int vga_available=0;
#else
#include "vgarom.c"
#endif

extern int boot_i2c_read(int addr);

extern struct trapframe DBGREG;

extern void *memset(void *, int, size_t);

int kbd_available;

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

int mpscdebug (int op, struct DevEntry *dev, unsigned long param, int data);
ConfigEntry	ConfigTable[] =
{
#if 1
	{ (char *)1,0,mpscdebug,256, CONS_BAUD, NS16550HZ },
#endif
	//{ (char *)COM1_BASE_ADDR, 0, ns16550, 256, CONS_BAUD, NS16550HZ },
	//{ (char *)COM2_BASE_ADDR, 0, ns16550, 256, CONS_BAUD, NS16550HZ },
#if NMOD_VGACON > 0
	{ (char *)1, 0, vgaterm, 256, CONS_BAUD, NS16550HZ },
#endif
	{ 0 }
};

unsigned long _filebase;

extern int memorysize;
extern int memorysize_high;

extern char MipsException[], MipsExceptionEnd[];

unsigned char hwethadr[6];

void initmips(unsigned int memsz);

void addr_tst1(void);
void addr_tst2(void);
void movinv1(int iter, ulong p1, ulong p2);
void test_regs(void);

void
initmips(unsigned int memsz)
{
	memorysize=(memsz&0x0000ffff) << 20;
	memorysize_high=((memsz&0xffff0000)>>16) << 20;

	SBD_DISPLAY("STAR",0);
	/*
	 *  Probe clock frequencys so delays will work properly.
	 */
#ifdef USE_SUPERIO_UART
#define	ISAREFRESH (14318180/(100000/15))
	linux_outb(0x7c,0x43);
	linux_outb(ISAREFRESH&&0xff,0x41);
	linux_outb(ISAREFRESH>>8,0x41);
#endif

	tgt_cpufreq();

#if 0
	tgt_display("memtest with cache on\n",0);

	/* memtest */
	addr_tst1();
	addr_tst2();
	movinv1(2,0,~0);
	movinv1(2,0xaa5555aa,~0xaa5555aa);
	tgt_display("memtest done\n",0);

	/*
	__asm__ volatile (
			"mfc0    $2,$16\r\n"
			"and     $2,$2,~0x7\r\n"
			"or      $2,$2,0x2\r\n"
			"mtc0    $2,$16\r\n":::"$2");
	*/

#endif

	SBD_DISPLAY("DBGI",0);
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
	bcopy(MipsException, (char *)GEN_EXC_VEC, MipsExceptionEnd - MipsException);

	CPU_FlushCache();

	CPU_SetSR(0, SR_BOOT_EXC_VEC);
	SBD_DISPLAY("BEV0",0);
	
	printf("BEV in SR set to zero.\n");

	
	/*
	 * Launch!
	 */
	main();
}
/*
 *  Put all machine dependent initialization here. This call
 *  is done after console has been initialized so it's safe
 *  to output configuration and debug information with printf.
 */

extern void	vt82c686_init(void);
void
tgt_devconfig()
{

#if NMOD_VGACON >0
	int rc;
#endif

	
	/* Set the PHY addresses for the GT ethernet */
	GT_WRITE(ETH_PHY_ADDR_REG, 0x18a4);
	GT_WRITE(MAIN_ROUTING_REGISTER, 0x007ffe38);
#if defined(LINUX_PC)
	GT_WRITE(SERIAL_PORT_MULTIPLEX, 0x00001101);
#else
	GT_WRITE(SERIAL_PORT_MULTIPLEX, 0x00001102);
#endif
	
#if defined(LINUX_PC)
	/* Route Multi Purpose Pins */
	GT_WRITE(MPP_CONTROL0, 0x77777777);
	GT_WRITE(MPP_CONTROL1, 0x00000000);
	GT_WRITE(MPP_CONTROL2, 0x00888888);
	GT_WRITE(MPP_CONTROL3, 0x00000066);
#elif defined(GODSONEV2A) 
	/* Route Multi Purpose Pins */
	GT_WRITE(MPP_CONTROL0, 0x77777777);
	GT_WRITE(MPP_CONTROL1, 0x00000000);
	GT_WRITE(MPP_CONTROL2, 0x00888888);
	GT_WRITE(MPP_CONTROL3, 0x00000066);
#elif defined(LONGMENG)
/*
 * MPP8			MPP9		MPP18	MPP19	MPP20	MPP21	MPP22	MPP23	MPP24	MPP25	
 * DBG_UART_TX DBG_UART_RX  GNT0# 	REQ0# 	GNT1#	REQ1#	GNT#2	REQ#2	GNT#3	REQ#3
 * MPP28      MPP29     MPP30     MPP31
 * PCI_INTA#  PCI_INTB# PCI_INTC# PCI_INTD#		
 */
#define DBG_LED0	(1<<4)     /*MPP4 as led0*/
#define DBG_LED1	(1<<5)     /*MPP5 as led1*/
#define DBG_LED2	(1<<6)	   /*MPP6 as led2*/
#define MPP_UART_TX	(1<<0)
#define MP_INTA	(1<<24)
#define MP_INTB	(1<<25)
#define MP_INTC	(1<<26)
#define MP_INTD	(1<<27)

 	GT_WRITE(GPP_IO_CONTROL,MPP_UART_TX|DBG_LED0|DBG_LED1|DBG_LED2);
 	GT_WRITE(GPP_LEVEL_CONTROL,MP_INTA|MP_INTB|MP_INTC|MP_INTD);
 	GT_WRITE(GPP_INTERRUPT_MASK,MP_INTA|MP_INTB|MP_INTC|MP_INTD);

	/* Route Multi Purpose Pins */
	GT_WRITE(MPP_CONTROL0, 0x00000322);	//MPP0,MPP1 as S0_TXD,S0_RXD,MPP2 clk24M,others set to gpio pin
	GT_WRITE(MPP_CONTROL1, 0x00000000);	
	GT_WRITE(MPP_CONTROL2, 0x11111111);	//MPP16~23 as PCI REQ or GNT.
	GT_WRITE(MPP_CONTROL3, 0x00400000); //MPP24~27 as PCI INTA-INTD,MPP29 as clk14M if nessary,others gpio. 

#else
	/* Route Multi Purpose Pins */
	GT_WRITE(MPP_CONTROL0, 0x53547777);
	GT_WRITE(MPP_CONTROL1, 0x44009911);
	GT_WRITE(MPP_CONTROL2, 0x40098888);
	GT_WRITE(MPP_CONTROL3, 0x00090066);
#endif	
	_pci_devinit(1);	/* PCI device initialization */
	SBD_DISPLAY("VGAI", 0);
#if (NMOD_X86EMU_INT10 > 0) || (NMOD_X86EMU >0)
	vga_bios_init();
#endif
	config_init();
	SBD_DISPLAY("CONF", 0);
	configure();
#if  NMOD_VGACON >0
	rc=kbd_initialize();
	printf("%s\n",kbd_error_msgs[rc]);
	if(!rc){ 
		kbd_available=1;
	}
#endif
	SBD_DISPLAY("DEVD", 0);
}

extern void godson2_cache_flush(void);
#ifdef DEEP_DEBUG
void tgt_test_memory(void)
{
	register unsigned i,j;
        register volatile char * volatile cmem;
        register volatile unsigned * volatile imem;
        register volatile unsigned * volatile imem_uc;
  //      long long * volatile imem;

	printf("%s:%d executing...\n",__FUNCTION__,__LINE__);
	j = 0;
	for (i=0;i<100000;i++) {
		i++;
	}
	tgt_putchar('1');
	if (i!=j) { tgt_putchar('2'); } 
        else { tgt_putchar('('); }

        cmem = (char*)0x80400000L;
        imem = (unsigned *)0x80400000L;
        imem_uc = (unsigned *)0xa0400000L;

        for (i=0;i<64*1024/sizeof(*cmem);i++) {
          cmem[i] = 0x5a;
          if (cmem[i]!=0x5a) {
	   tgt_putchar('4');
          }else{
	   //tgt_putchar('3');
          }
        }
	puts("char access test passed");

        for (i=0;i<64*1024/sizeof(*imem);i++) {
          imem[i] = 0xaa55aa55;
          if (imem[i]!=0xaa55aa55) {
	   tgt_putchar('6');
          }else{
	   //tgt_putchar('5');
          }
        }

	puts("word access test passed");

	printf("executing...\n");
        for (i=0,j=0x87654321;i<64*1024/sizeof(*imem);i++,j+=0x01010101) {
	  imem[i] = j;
	  if (imem[i] != j) printf("error1(imem[i]=0x%x,j=0x%x.\n", imem[i],j);
	}
	CPU_FlushDCache((int)imem, 64*1024);
        for (i=0,j=0x87654321;i<64*1024/sizeof(*imem);i++,j+=0x01010101) {
	  if (imem_uc[i] != j) printf("%s:%d error(imem_uc[i]=0x%x,j=0x%x.\n", __FUNCTION__,__LINE__,imem_uc[i],j);
	}
        for (i=0,j=0x87654321;i<64*1024/sizeof(*imem);i++,j+=0x01010101) {
	  if (imem[i] != j) printf("%s:%d error(imem[i]=0x%x,j=0x%x.\n", __FUNCTION__,__LINE__,imem[i],j);
	}

	printf("%s:%d executing...\n",__FUNCTION__,__LINE__);
        for (i=0,j=0x12345678;i<64*1024/sizeof(*imem);i++,j+=0x01010101) {
	  imem_uc[i] = j;
	  if (imem_uc[i] != j) printf("%s:%d error(imem_uc[i]=0x%x,j=0x%x.\n", __FUNCTION__,__LINE__,imem_uc[i],j);
	}
	CPU_HitInvalidateDCache((int)imem, 64*1024);
        for (i=0,j=0x12345678;i<64*1024/sizeof(*imem);i++,j+=0x01010101) {
	  if (imem[i] != j) printf("%s:%d error(imem[i]=0x%x,j=0x%x.\n", __FUNCTION__,__LINE__,imem[i],j);
	}
        for (i=0,j=0x12345678;i<64*1024/sizeof(*imem);i++,j+=0x01010101) {
	  if (imem_uc[i] != j) printf("%s:%d error(imem_uc[i]=0x%x,j=0x%x.\n", __FUNCTION__,__LINE__,imem_uc[i],j);
	}

}
#endif
extern int novga;
void
tgt_devinit()
{
int cnt;
//ioctl (STDIN, FIONREAD, &cnt);
//if(cnt){novga=1;}

	SBD_DISPLAY("686I",0);
	
	vt82c686_init();
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
	
	SBD_DISPLAY("CACH",0);
	CPU_ConfigCache();
	SBD_DISPLAY("CAC2",0);
	SBD_DISPLAY("CAC3",0);

	//godson2_cache_flush();

	_pci_businit(1);	/* PCI bus initialization */
}


void
tgt_reboot()
{

	void (*longreach) __P((void));
	
	longreach = (void *)0xbfc00000;
	(*longreach)();
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
	printf("Board revision level: %c.\n", in8(PLD_BAR) + 'A');
	printf("PLD revision levels: %d.%d and %d.%d.\n",
					in8(PLD_ID1) >> 4, in8(PLD_ID1) & 15,
					in8(PLD_ID2) >> 4, in8(PLD_ID2) & 15);
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
    printf("[[[[[[[2000][[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[\n"); 
#endif
}
#ifdef USE_LEGACY_RTC
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

static void init_legacy_rtc(void)
{
	int year, month, date, hour, min, sec;
	char buf[128];
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
	if( (year > 99) || (month < 1 || month > 12) || 
		(date < 1 || date > 31) || (hour > 23) || (min > 59) ||
		(sec > 59) ){
		/*
		printf("RTC time invalid, reset to epoch.\n");*/
		CMOS_WRITE(3, DS_REG_YEAR);
		CMOS_WRITE(1, DS_REG_MONTH);
		CMOS_WRITE(1, DS_REG_DATE);
		CMOS_WRITE(0, DS_REG_HOUR);
		CMOS_WRITE(0, DS_REG_MIN);
		CMOS_WRITE(0, DS_REG_SEC);
	}
	CMOS_WRITE(DS_CTLB_24 | DS_CTLB_DM, DS_REG_CTLB);

	sprintf(buf,"RTC: %02d-%02d-%02d %02d:%02d:%02d\n",
			year, month, date, hour, min, sec);
	tgt_display(buf,0);
}
#endif

static void
_probe_frequencies()
{
#ifdef HAVE_TOD
	int i, timeout, cur, sec, cnt;
#endif
	extern void tgt_setpar100mhz __P((void));

	SBD_DISPLAY ("FREQ", CHKPNT_FREQ);


	md_pipefreq = 300000000;	/* Defaults */
	md_cpufreq  = 100000000;
	clk_invalid = 1;
#ifdef HAVE_TOD
#ifdef USE_LEGACY_RTC
	init_legacy_rtc();
	/*
	 * Do the next twice for two reasons. First make sure we run from
	 * cache. Second make sure synched on second update. (Pun intended!)
	 */
	for(i = 2;  i != 0; i--) {
		cnt = CPU_GetCOUNT();
		timeout = 10000000;
		
		while(CMOS_READ(DS_REG_CTLA) & DS_CTLA_UIP);

		sec = CMOS_READ(DS_REG_SEC);

		do {
			timeout--;
			while(CMOS_READ(DS_REG_CTLA) & DS_CTLA_UIP);

			cur = CMOS_READ(DS_REG_SEC);
		} while(timeout != 0 && cur == sec);

		cnt = CPU_GetCOUNT() - cnt;
		if(timeout == 0) {
			tgt_display("RTC clock is not running!\r\n",0);
			break;		/* Get out if clock is not running */
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
		md_cpufreq = 660000000;
	}

	/*
	 * Hack to change timing of GT64420 if not 125Mhz FSB.
	 */

	if(md_cpufreq < 110000000) {
		//tgt_setpar100mhz();
	}

#endif /* USE_LEGACY_RTC */
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

#ifdef HAVE_TOD
/*
 *  Returns the current time if a TOD clock is present or 0
 *  This code is defunct after 2088... (out of bits)
 */
#define FROMBCD(x)      (((x) >> 4) * 10 + ((x) & 0xf))
#define TOBCD(x)        (((x) / 10 * 16) + ((x) % 10)) 
#define YEARDAYS(year)  ((((year) % 4) == 0 && \
                         ((year % 100) != 0 || (year % 400) == 0)) ? 366 : 365)
#define SECMIN  (60)            /* seconds per minute */
#define SECHOUR (60*SECMIN)     /* seconds per hour */
#define SECDAY  (24*SECHOUR)    /* seconds per day */
#define SECYR   (365*SECDAY)    /* seconds per common year */

static const short dayyr[12] =
    { 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334 };

time_t
tgt_gettime()
{
	struct tm tm;
	int ctrlbsave;
	time_t t;

	if(!clk_invalid) {
#ifdef USE_LEGACY_RTC
		ctrlbsave = CMOS_READ(DS_REG_CTLB);
		CMOS_WRITE(ctrlbsave | DS_CTLB_SET, DS_REG_CTLB);
		
		tm.tm_sec = CMOS_READ(DS_REG_SEC); 
		tm.tm_min = CMOS_READ(DS_REG_MIN);
		tm.tm_hour = CMOS_READ(DS_REG_HOUR);
		tm.tm_wday = CMOS_READ(DS_REG_WDAY);
		tm.tm_mday = CMOS_READ(DS_REG_DATE);
		tm.tm_mon = CMOS_READ(DS_REG_MONTH) - 1;
		tm.tm_year = CMOS_READ(DS_REG_YEAR);
		if(tm.tm_year < 50)tm.tm_year += 100;
		
		CMOS_WRITE(ctrlbsave & ~DS_CTLB_SET, DS_REG_CTLB);
#else
		/* Freeze readout data regs */
		ctrlbsave =  inb(RTC_BASE + DS_REG_CTLB);
		outb(RTC_BASE + DS_REG_CTLB, ctrlbsave & ~DS_CTLB_TE);

		tm.tm_sec = FROMBCD(inb(RTC_BASE + DS_REG_SEC));
		tm.tm_min = FROMBCD(inb(RTC_BASE + DS_REG_MIN));
		tm.tm_hour = FROMBCD(inb(RTC_BASE + DS_REG_HOUR));
		tm.tm_wday = FROMBCD(inb(RTC_BASE + DS_REG_WDAY));
		tm.tm_mday = FROMBCD(inb(RTC_BASE + DS_REG_DATE));
		tm.tm_mon = FROMBCD(inb(RTC_BASE + DS_REG_MONTH) & 0x1f) - 1;
		tm.tm_year = FROMBCD(inb(RTC_BASE + DS_REG_YEAR));
		tm.tm_year += 100 * (FROMBCD(inb(RTC_BASE + DS_REG_CENT)) - 19);

		outb(RTC_BASE + DS_REG_CTLB, ctrlbsave | DS_CTLB_TE);
#endif /* USE_LEGACY_RTC */

		tm.tm_isdst = tm.tm_gmtoff = 0;
		t = gmmktime(&tm);
	}
	else {
		t = 957960000;  /* Wed May 10 14:00:00 2000 :-) */
	}
	return(t);
}

/*
 *  Set the current time if a TOD clock is present
 */
void
tgt_settime(time_t t)
{
	struct tm *tm;
	int ctrlbsave;

	if(!clk_invalid) {
		tm = gmtime(&t);
#ifdef USE_LEGACY_RTC
		ctrlbsave = CMOS_READ(DS_REG_CTLB);
		CMOS_WRITE(ctrlbsave | DS_CTLB_SET, DS_REG_CTLB);

		CMOS_WRITE(tm->tm_year % 100, DS_REG_YEAR);
		CMOS_WRITE(tm->tm_mon + 1, DS_REG_MONTH);
		CMOS_WRITE(tm->tm_mday, DS_REG_DATE);
		CMOS_WRITE(tm->tm_wday, DS_REG_WDAY);
		CMOS_WRITE(tm->tm_hour, DS_REG_HOUR);
		CMOS_WRITE(tm->tm_min, DS_REG_MIN);
		CMOS_WRITE(tm->tm_sec, DS_REG_SEC);

		CMOS_WRITE(ctrlbsave & ~DS_CTLB_SET, DS_REG_CTLB);
#else
		/* Enable register writing */
		ctrlbsave =  inb(RTC_BASE + DS_REG_CTLB);
		outb(RTC_BASE + DS_REG_CTLB, ctrlbsave & ~DS_CTLB_TE);

		outb(RTC_BASE + DS_REG_CENT, TOBCD(tm->tm_year / 100 + 19));
		outb(RTC_BASE + DS_REG_YEAR, TOBCD(tm->tm_year % 100));
		outb(RTC_BASE + DS_REG_MONTH,TOBCD(tm->tm_mon + 1));
		outb(RTC_BASE + DS_REG_DATE, TOBCD(tm->tm_mday));
		outb(RTC_BASE + DS_REG_WDAY, TOBCD(tm->tm_wday));
		outb(RTC_BASE + DS_REG_HOUR, TOBCD(tm->tm_hour));
		outb(RTC_BASE + DS_REG_MIN,  TOBCD(tm->tm_min));
		outb(RTC_BASE + DS_REG_SEC,  TOBCD(tm->tm_sec));

		/* Transfer new time to counters */
		outb(RTC_BASE + DS_REG_CTLB, ctrlbsave | DS_CTLB_TE);
#endif /* USE_LEGACY_RTC */
	}
}

#else

time_t
tgt_gettime()
{
	return(957960000);  /* Wed May 10 14:00:00 2000 :-) */
}

#endif /* HAVE_TOD */


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
	printf("Copyright 2003, Michael and Pine, ICT CAS.\n");
	printf("CPU %s @", md_cpuname());
} 

/*
 *  Return a suitable address for the client stack.
 *  Usually top of RAM memory.
 */

register_t
tgt_clienttos()
{
	return((register_t)(int)PHYS_TO_UNCACHED(memorysize & ~7) - 64);
}

#ifdef HAVE_FLASH
/*
 *  Flash programming support code.
 */

/*
 *  Table of flash devices on target. See pflash_tgt.h.
 */
struct fl_map tgt_fl_map_boot8[] = {
        TARGET_FLASH_DEVICES_8
};
 
struct fl_map tgt_fl_map_boot32[] = {
        TARGET_FLASH_DEVICES_32
};

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
	if(in8(PLD_BSTAT) & 0x40) {
		return(1);
	}
	else {
		return(1);	/* can't enable. jumper selected! */
	}
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
#ifdef NVRAM_IN_FLASH
	nvram = (char *)(tgt_flashmap())->fl_map_base;
	if(fl_devident(nvram, NULL) == 0 ||
			cksum(nvram + NVRAM_OFFS, NVRAM_SIZE, 0) != 0) {
#else
	nvram = (char *)malloc(512);
	nvram_get(nvram);
	if(cksum(nvram, NVRAM_SIZE, 0) != 0) {
#endif
		printf("NVRAM is invalid!\n");
		nvram_invalid = 1;
	} else {
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

	printf("NVRAM@%x\n",(u_int32_t)nvram);

	/*
	 *  Ethernet address for Galileo ethernet is stored in the last
	 *  six bytes of nvram storage. Set environment to it.
	 */
	bcopy(&nvram[ETHER_OFFS], hwethadr, 6);
	{int i;
	 for(i=0;i<6;i++)
		 if(hwethadr[i]!=0)break;
	 if(i==6){
		 for(i=0;i<6;i++)
			 hwethadr[i]=i;
	 }
	}
	sprintf(env, "%02x:%02x:%02x:%02x:%02x:%02x", hwethadr[0], hwethadr[1],
	    hwethadr[2], hwethadr[3], hwethadr[4], hwethadr[5]);
	(*func)("ethaddr", env);

#ifndef NVRAM_IN_FLASH
	free(nvram);
#endif

	sprintf(env, "%d", memorysize / (1024 * 1024));
	(*func)("memsize", env);

	sprintf(env, "%d", md_pipefreq);
	(*func)("cpuclock", env);

	sprintf(env, "%d", md_cpufreq);
	(*func)("busclock", env);

	(*func)("systype", SYSTYPE);

	sprintf(env, "%d", CpuTertiaryCacheSize / (1024*1024));
	(*func)("l3cache", env);

	sprintf(env, "%x", GT_BASE_ADDR);
	(*func)("gtbase", env);
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

	bcopy(hwethadr, &nvrambuf[ETHER_OFFS], 6);
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
	//printf("nvram_get:\n");
#ifdef USE_LEGACY_RTC
	for(i = 0; i < 114; i++) {
		linux_outb(i + RTC_NVRAM_BASE, RTC_INDEX_REG);	/* Address */
		buffer[i] = linux_inb(RTC_DATA_REG);
	}
#else
	buffer[0] = '\0';
#endif
	/*
	for(i = 0; i < 114; i++) {
		printf("%x ", buffer[i]);
	}
	printf("\n");
	*/
}

void
nvram_put(char *buffer)
{
#ifdef USE_LEGACY_RTC
	int i;
	for(i = 0; i < 114; i++) {
		linux_outb(i+RTC_NVRAM_BASE, RTC_INDEX_REG);	/* Address */
		linux_outb(buffer[i],RTC_DATA_REG);
	}
#endif
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
	char *p = msg;

	while (*p != 0) {
		tgt_putchar(*p++);
		delay(500);
	}
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
