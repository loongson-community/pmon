/*	$Id: tgt_machdep.c,v 1.6 2006/07/20 09:37:06 cpu Exp $ */

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
//#define USE_LEGACY_RTC

#include <include/stdarg.h>
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
#include <stdarg.h>
#include <dev/ic/mc146818reg.h>
#include <linux/io.h>
#include <autoconf.h>
#include <machine/cpu.h>
#include <machine/pio.h>
#include "pflash.h"
#include "dev/pflash_tgt.h"
#include "include/ATtiny44.h"
#include "include/bonito.h"
#include <pmon/dev/gt64240reg.h>
#include <pmon/dev/ns16550.h>
#include <pmon.h>
#include "mod_vgacon.h"
#include "mod_framebuffer.h"
#include "flash.h"

#define HAVE_RTC 1
#define FLASH_OFFS (tgt_flashmap()->fl_map_size - 0x1000)

#if (NMOD_FLASH_AMD + NMOD_FLASH_INTEL + NMOD_FLASH_SST + NMOD_FLASH_WINBOND) == 0
    #ifdef HAVE_FLASH
    #undef HAVE_FLASH
    #endif
#else
    #ifndef HAVE_FLASH
    #define HAVE_FLASH
    #endif
#endif

#if NMOD_X86EMU_INT10 != 0 || NMOD_X86EMU != 0
    #ifdef VGA_NO_ROM
    #include "vgarom.c"
    #endif
#endif
#define BCD2BIN(x) (((x&0xf0)>>4)*10+(x&0x0f))
#define BIN2BCD(x)  ((x/10)<<4)+(x%10)



extern void *memset(void *, int, size_t);
extern int fl_program(void *fl_base, void *data_base, int data_size, int verbose);
extern int vgaterm(int op, struct DevEntry * dev, unsigned long param, int data);
extern int fbterm(int op, struct DevEntry * dev, unsigned long param, int data);
extern void tgt_fpuenable(void);
extern int video_hw_init (void);
extern unsigned char rtc_i2c_rec_s(unsigned char,unsigned char,unsigned char*,int);
extern unsigned char rtc_i2c_send_s(unsigned char ,unsigned char,unsigned char *,int);
extern void rtc_i2c_start(void);
extern unsigned char rtc_i2c_send(unsigned char);
extern char rtc_i2c_rec_ack(void);
extern void rtc_i2c_stop(void);
extern unsigned char rtc_i2c_rec(void);
extern void rtc_i2c_send_ack(int);


extern unsigned int memorysize;
extern unsigned int memorysize_high;

extern char MipsException[], MipsExceptionEnd[];

extern struct trapframe DBGREG;



static int cksum(void *p, size_t s, int set);
static void _probe_frequencies(void);

#ifndef NVRAM_IN_FLASH
void nvram_get(char *);
void nvram_put(char *);
#endif
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
#if HAVE_RTC > 0
static void init_legacy_rtc(void);
#endif

/*
 * NOTE : we use COMMON_COM_BASE_ADDR and NS16550HZ instead the former. please see
 * the Targets/Bonito/include/bonito.h for detail.
 */
ConfigEntry	ConfigTable[] =
{
	{ (char *)COMMON_COM_BASE_ADDR, 0, ns16550, 256, CONS_BAUD, NS16550HZ }, 
	 /*{ (char *)COM2_BASE_ADDR, 0, ns16550, 256, CONS_BAUD, NS16550HZ }, */
#if NMOD_VGACON >0
#if NMOD_FRAMEBUFFER >0
	{ (char *)1, 0, fbterm, 256, CONS_BAUD, NS16550HZ },
#else
	{ (char *)1, 0, vgaterm, 256, CONS_BAUD, NS16550HZ },
#endif
#endif
	{ 0 }
};

volatile char * mmio;
unsigned long _filebase;
unsigned char hwethadr[6];

int kbd_available = 0;
int usb_kbd_available;
int vga_available = 0;

static int md_pipefreq = 0;
static int md_cpufreq = 0;
static int clk_invalid = 0;
static int nvram_invalid = 0;

void initmips(unsigned int memsz);
void addr_tst1(void);
void addr_tst2(void);
void movinv1(int iter, ulong p1, ulong p2);

void sd2068_write_enable(void);
void sd2068_write_disable(void);
void sd2068_write_date(struct tm *tm);
void sd2068_getdate(struct tm *tm);


unsigned char 
i2c_send_s(unsigned char slave_addr,unsigned char
				sub_addr,unsigned char * buf ,int count);
unsigned char 
i2c_send_s16(unsigned char slave_addr,unsigned char
				sub_addr,unsigned short * buf, int count);
unsigned char 
i2c_rec_s(unsigned char slave_addr,unsigned char 
                sub_addr,unsigned char* buf ,int count);


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

void fixup_sm502_rtc(void)
{
    unsigned char  data;
	unsigned int tmp;
	pcitag_t tag;

	tag=_pci_make_tag(0,14,0);
	
	_pci_conf_writen(tag, 0x14, 0x6000000, 4);
	tmp = _pci_conf_readn(tag, 0x04, 4);
	tgt_printf("cmd %x\n", tmp);
	_pci_conf_writen(tag, 0x04, tmp |0x02, 4);

	mmio =(volatile char *)_pci_conf_readn(tag,0x14,4);


	mmio =(volatile char *)((unsigned long)mmio|PTR_PAD(0xb0000000)); /*This is a global variable*/

	tgt_printf("mmio %llx\n", mmio);


	//enable gpio clock
	tmp = *(volatile unsigned int *)(mmio + 0x40);
	*(volatile unsigned int *)(mmio + 0x40) =tmp|0x40;


	data = 0x00;
	i2c_send_s((unsigned char)0xd0,0x0c,&data,1);

	i2c_rec_s((unsigned char)0xd0, 0x01, &data,1);
	data |= 0x80;
	i2c_send_s((unsigned char)0xd0,0x01,&data,1);
	data &= 0x7f;
	i2c_send_s((unsigned char)0xd0,0x01,&data,1);
}

void turn_fan(int on)
{
	unsigned char data8; 
	unsigned short data16;

	i2c_rec_s(0x90, 0x01, &data8, 1);

	/* 
	 * Reg 2,3 is the temparature limit, default is 0x4b00, 0x5000
	 * Reg 0 is current temp
	 * reg 1 is configuration --- valid level
	 */
	if(on) {
		//data16 = 0x320 << 4;
		//data16 = 0x260 << 4; /* 38 */
		data16 = 0x300 << 4; /* 48 */
		i2c_send_s16(0x90, 0x02, &data16, 1); /* Set Thyst*/
		i2c_send_s16(0x90, 0x03, &data16, 1); /* Set Tos */
		data8 &= ~0x04;
		i2c_rec_s(0x90, 0x01, &data8, 1);
	} else {
		data16 = 0x4b00;
		i2c_send_s16(0x90, 0x02, &data16, 1); /* Set Thyst*/

		data16 = 0x5000;
		i2c_send_s16(0x90, 0x03, &data16, 1); /* Set Tos */

		data8 &= ~0x04;
		i2c_send_s((unsigned char)0x90,0x01,&data8, 1);
	}
}
    

#define TEMP_GPIO_DIR_LOW    (volatile unsigned int *)(temp_mmio + 0x10008)
#define TEMP_GPIO_DATA_LOW   (volatile unsigned int *)(temp_mmio + 0x10000)

#define TEMP_GPIO_DIR_HIGH   (volatile unsigned int *)(temp_mmio + 0x1000c)
#define TEMP_GPIO_DATA_HIGH  (volatile unsigned int *)(temp_mmio + 0x10004)

void turnon_backlight(void)
{
    //unsigned int xiangy_tmp, temp_mmio;
    unsigned int tmp;
    unsigned long temp_mmio;
	pcitag_t tag=_pci_make_tag(0,14,0);

	_pci_conf_writen(tag, 0x14, 0x6000000, 4);
	temp_mmio = _pci_conf_readn(tag,0x14,4);
	temp_mmio =(unsigned long)temp_mmio|PTR_PAD(0xb0000000);

	/*gpio32 to LOW*/
        tmp = *TEMP_GPIO_DATA_HIGH;
        *TEMP_GPIO_DATA_HIGH = tmp & (~1);    
        tmp = *TEMP_GPIO_DIR_HIGH;
        *TEMP_GPIO_DIR_HIGH = tmp  | 1;
    
	/*gpio33 to HIGH*/
        tmp = *TEMP_GPIO_DATA_HIGH;
        *TEMP_GPIO_DATA_HIGH = tmp | 2;
        tmp = *TEMP_GPIO_DIR_HIGH;
        *TEMP_GPIO_DIR_HIGH = tmp  | 2;
}

void fix_audio(void)
{
	
    unsigned int tmp;
    unsigned long temp_mmio;
	pcitag_t tag=_pci_make_tag(0,14,0);

	_pci_conf_writen(tag, 0x14, 0x6000000, 4);
	temp_mmio = _pci_conf_readn(tag,0x14,4);
	temp_mmio =(unsigned long)temp_mmio|PTR_PAD(0xb0000000);

	//gpio24 output high
	tmp = *TEMP_GPIO_DIR_LOW;
	*TEMP_GPIO_DIR_LOW = tmp | (1 << 24);        
	tmp = *TEMP_GPIO_DATA_LOW;
	*TEMP_GPIO_DATA_LOW = tmp | (1 << 24);
}


void
initmips(unsigned int memsz)
{
    /*
	 *	Set up memory address decoders to map entire memory.
	 *	But first move away bootrom map to high memory.
	 */
#if 0
	GT_WRITE(BOOTCS_LOW_DECODE_ADDRESS, BOOT_BASE >> 20);
	GT_WRITE(BOOTCS_HIGH_DECODE_ADDRESS, (BOOT_BASE - 1 + BOOT_SIZE) >> 20);
#endif

	//fix IDT's problem
	fix_audio();
    tgt_fpuenable();

	/*
	 *	Set up memory address decoders to map entire memory.
	 *	But first move away bootrom map to high memory.
	 */
	memorysize = memsz > 256 ? 256 << 20 : memsz << 20;
	memorysize_high = memsz > 256 ? (memsz - 256) << 20 : 0;

#if defined(DEVBD2F_SM502)
	fixup_sm502_rtc();
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
extern void vt82c686_init(void);
extern void cs5536_init(void);
extern int fb_init(unsigned long,unsigned long);

#ifdef LOONGSON2F_ALLINONE
extern void i2c_write_single(int, int, char);
extern char i2c_read_single(int, int, char*);
#endif

void
tgt_devconfig(void)
{
	int rc=1;
	unsigned long fbaddress,ioaddress;
	extern struct pci_device *vga_dev;


    
	_pci_devinit(1);	/* PCI device initialization */

    
	if(!vga_dev) {
		printf("ERROR !!! VGA device is not found\n"); 
		rc = -1;
	}
	if (rc > 0) {
		fbaddress  =_pci_conf_read(vga_dev->pa.pa_tag,0x10);
		ioaddress  =_pci_conf_read(vga_dev->pa.pa_tag,0x18);

		fbaddress = fbaddress & PTR_PAD(0xffffff00); //laster 8 bit
		ioaddress = ioaddress & PTR_PAD(0xfffffff0); //laster 4 bit
		
        rc = video_hw_init ();
                fbaddress  =_pci_conf_read(vga_dev->pa.pa_tag,0x10);
                ioaddress  =_pci_conf_read(vga_dev->pa.pa_tag,0x14);
                fbaddress |= PTR_PAD(0xb0000000);
                ioaddress |= PTR_PAD(0xb0000000);
		
        delay(100000);
        turnon_backlight();
		fb_init(fbaddress, ioaddress);
		vga_available = 0;
	} else {
		printf("vga bios init failed, rc=%d\n",rc);
	}

    //zhangpin
    #ifdef SPREAD_SPECTRUM
    do_cmd("wriic 0xd2 1 12");
    #endif

    if (rc > 0) {
		if(!getenv("novga")) 
			vga_available=1;
		else 
			vga_available=0;
	} 
    config_init();
    configure();
}

extern int test_icache_1(short *addr);
extern int test_icache_2(int addr);
extern int test_icache_3(int addr);
extern void godson1_cache_flush(void);
#define tgt_putchar_uc(x) (*(void (*)(char)) (((long)tgt_putchar)|0x20000000)) (x)

extern void cs5536_gpio_init(void);
extern void test_gpio_function(void);
extern void cs5536_pci_fixup(void);

void
tgt_devinit(void)
{
	SBD_DISPLAY("SM502",0);

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

	return;
}


void
tgt_reboot(void)
{
	/* call ec do the reset */
    do_cmd("wriic 0xb6 1 2");
    while(1);
}

void
tgt_poweroff(void)
{
    /* call ec do the poweroff*/
    do_cmd("wriic 0xb6 1 1");
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
tgt_cmd_vers(void)
{
}

/*
 *  Display any target specific logo.
 */
void
tgt_logo(void)
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
    printf("[[[[[[[2005][[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[\n"); 
#endif
}

#if HAVE_RTC > 0
static void init_legacy_rtc(void)
{
    int tformat;
    struct tm my_tm,my_tm1;
  
    my_tm1.tm_sec = 0;
    my_tm1.tm_min = 0;
    my_tm1.tm_hour = 0;
    my_tm1.tm_wday = 5;
    my_tm1.tm_mday = 1;
    my_tm1.tm_mon = 1;
    my_tm1.tm_year = 10;

    tformat = CMOS_READ(0x2);//time format
    if(!(tformat & 0x80)){//not 24 hour format
        sd2068_write_enable();
        sd2068_write_date(&my_tm1);
        sd2068_write_disable();
    }
    sd2068_getdate(&my_tm); 
    my_tm.tm_sec = BCD2BIN(my_tm.tm_sec);
    my_tm.tm_min = BCD2BIN(my_tm.tm_min);
    my_tm.tm_hour =BCD2BIN(my_tm.tm_hour);
    my_tm.tm_wday = BCD2BIN(my_tm.tm_wday);
    my_tm.tm_mday = BCD2BIN(my_tm.tm_mday);
    my_tm.tm_mon = BCD2BIN(my_tm.tm_mon);
    my_tm.tm_year =BCD2BIN(my_tm.tm_year);
    
	if( ((my_tm.tm_year > 37) && ( my_tm.tm_year < 70)) || (my_tm.tm_year > 99) || (my_tm.tm_mon< 1 || my_tm.tm_mon > 12) ||
                (my_tm.tm_mday < 1 || my_tm.tm_mday > 31) || (my_tm.tm_hour > 23) || (my_tm.tm_min > 59) ||
                (my_tm.tm_sec > 59) ){
        sd2068_write_enable();
        sd2068_write_date(&my_tm1);
        sd2068_write_disable();
    }
}
#endif

void suppress_auto_start(void)
{
	/* suppress auto start when power pluged in */
	CMOS_WRITE(0x80, 0xd);
}

static inline unsigned char CMOS_READ(unsigned char addr)
{
	unsigned char val;
	volatile unsigned int tmp;
	
#ifndef DEVBD2F_SM502
	linux_outb_p(addr, 0x70);
        val = linux_inb_p(0x71);
#else

	pcitag_t tag;
	if(addr >= 32)
		return 0;
	switch(addr)
	{
	    case 0x2:
                addr = 0x02;
                break;
		case 0xf:
		    	addr = 0x0f;
			break;
		case 0x10:
		    	addr = 0x10;
			break;
		default:
			return 0;
	}

#if 1
	tag=_pci_make_tag(0,14,0);
	
	mmio = (volatile char *)_pci_conf_readn(tag,0x14,4);
    mmio =(volatile char *)((unsigned long )mmio |  PTR_PAD(0xb0000000));
	tmp = *(volatile unsigned int *)(mmio + 0x40);
	*(volatile unsigned int *)(mmio + 0x40) =tmp|0x40;
#endif
		
	rtc_i2c_rec_s((unsigned char)0x64, addr,&val,1);
#endif
	return val;
}
      
                                                                               
static inline void CMOS_WRITE(unsigned char val, unsigned char addr)
{
#ifndef DEVBD2F_SM502
	linux_outb_p(addr, 0x70);
        linux_outb_p(val, 0x71);
#else

	unsigned char tmp1,tmp2;
	volatile unsigned int tmp;
	tmp1 = (val/10)<<4;
	tmp2  = (val%10);
	val = tmp1|tmp2;
	if(addr >=32)
		return ;
	switch(addr)
	{
		case 0xf:
		    	addr = 0x0f;
			break;
		case 0x10:
		    	addr = 0x10;
			break;
		default:
			return;
	}

	{
		pcitag_t tag;
		tag=_pci_make_tag(0,14,0);
	
		mmio =(volatile char *)_pci_conf_readn(tag,0x14,4);
        mmio =(volatile char *)((unsigned long )mmio |  PTR_PAD(0xb0000000));
		tmp = *(volatile unsigned int *)(mmio + 0x40);
		*(volatile unsigned int *)(mmio + 0x40) =tmp|0x40;

		rtc_i2c_send_s((unsigned char)0x64,addr,&val,1);
	}
#endif
}


static void
_probe_frequencies(void)
{
    md_pipefreq = 800435000;
    md_cpufreq  =  60000000;
    init_legacy_rtc();
}
                                                                               

/*
 *   Returns the CPU pipelie clock frequency
 */
int
tgt_pipefreq(void)
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
tgt_cpufreq(void)
{
	if(md_cpufreq == 0) {
		_probe_frequencies();
	}
	return(md_cpufreq);
}

void sd2068_write_enable(void)
{
	CMOS_WRITE(80, 0x10);
	CMOS_WRITE(84, 0xf);
}
void sd2068_write_disable(void)
{
	CMOS_WRITE(0, 0xf);
	CMOS_WRITE(0, 0x10);
}
void sd2068_write_date(struct tm *tm)
{
        pcitag_t tag;
        int i;
        unsigned int tmp;
        unsigned char value[7];

        tag=_pci_make_tag(0,14,0);
        mmio =(volatile char *)_pci_conf_readn(tag,0x14,4);
        mmio =(volatile char *)((unsigned long)mmio | PTR_PAD(0xb0000000));
        tmp = *(volatile int *)(mmio + 0x40);
        *(volatile int *)(mmio + 0x40) =tmp|0x40;

        value[0] = tm->tm_sec;
        value[1] = tm->tm_min;
        value[2] = tm->tm_hour;
        value[3] = tm->tm_wday;
        value[4] = tm->tm_mday;
        value[5] = tm->tm_mon;
        value[6] = tm->tm_year;

        for(i=0; i < 7; i++)
                value[i]=BIN2BCD(value[i]);
        value[2] = value[2] | 0x80;

        rtc_i2c_start();
        rtc_i2c_send(0x64);
        if(!rtc_i2c_rec_ack()){
                printf("no_ack 1111\n");
                return ;
        }
        rtc_i2c_send(0);
        if(!rtc_i2c_rec_ack()){
                printf("no_ack 2222\n");
                return ;
        }

        for(i=0; i<7; i++) {
                rtc_i2c_send(value[i]);
                if(!rtc_i2c_rec_ack()){
                        printf("no_ack 2222\n");
                        return ;
                }
        }
        rtc_i2c_stop();
}

void sd2068_getdate(struct tm *tm)
{
        pcitag_t tag;
        int i;
        unsigned int tmp;
        unsigned char value[7];

        tag=_pci_make_tag(0,14,0);
        mmio = (volatile char *)_pci_conf_readn(tag,0x14,4);
        mmio =(volatile char *)((unsigned long )mmio |  PTR_PAD(0xb0000000));
        tmp = *(volatile int *)(mmio + 0x40);
        *(volatile int *)(mmio + 0x40) =tmp|0x40;

        //start signal
        rtc_i2c_start();
        //write slave_addr
        rtc_i2c_send(0x65);
        if(!rtc_i2c_rec_ack())
                return ;

        for(i=0; i<7; i++){
                //read data
                value[i]=rtc_i2c_rec();
                if(i != 6)
                        rtc_i2c_send_ack(0);//***add in***//
        }
        rtc_i2c_stop();

        tm->tm_sec = value[0];
        tm->tm_min = value[1];
        tm->tm_hour = value[2] & 0x7f;
        tm->tm_wday = value[3];
        tm->tm_mday = value[4];
        tm->tm_mon  = value[5];
        tm->tm_year = value[6];
}

time_t tgt_gettime(void)
{
	struct tm tm;
	time_t t;
                                                                            
	if(!clk_invalid) {
#if 0 //for nxp
		/*BCD code and mask some bits*/
		tm.tm_sec = CMOS_READ(DS_REG_SEC) & 0x7f;
		tm.tm_min = CMOS_READ(DS_REG_MIN) & 0x7f;
		tm.tm_hour = CMOS_READ(DS_REG_HOUR) & 0x3f;
		tm.tm_wday = CMOS_READ(DS_REG_WDAY) & 0x07;
		tm.tm_mday = CMOS_READ(DS_REG_DATE) & 0x3f;
		tm.tm_mon = CMOS_READ(DS_REG_MONTH) & 0x1f;
		tm.tm_year = CMOS_READ(DS_REG_YEAR);
#else //for sd2068
		sd2068_getdate(&tm);
#endif

        tm.tm_sec =  BCD2BIN(tm.tm_sec);
        tm.tm_min =  BCD2BIN(tm.tm_min);
        tm.tm_hour = BCD2BIN(tm.tm_hour);
        tm.tm_mon = BCD2BIN(tm.tm_mon);
        tm.tm_mday = BCD2BIN(tm.tm_mday);
        tm.tm_year = BCD2BIN(tm.tm_year);

		tm.tm_mon = tm.tm_mon - 1;
		if(tm.tm_year < 50)tm.tm_year += 100;

		tm.tm_isdst = tm.tm_gmtoff = 0;
		t = gmmktime(&tm);
	} else {
        t = 957960000;  /* Wed May 10 14:00:00 2000 :-) */
	}

	return(t);
}
                                                                               
/*
 *  Set the current time if a TOD clock is present
 */
void tgt_settime(time_t t)
{
        struct tm *tm;

        if(!clk_invalid) {
        	tm = gmtime(&t);
            tm->tm_year = tm->tm_year % 100;
	        tm->tm_mon = tm->tm_mon + 1;
                                                         
		    sd2068_write_enable();
		    sd2068_write_date(tm);
		    sd2068_write_disable();
        }
}

/*
 *  Print out any target specific memory information
 */
void tgt_memprint(void)
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

void tgt_machprint(void)
{
	printf("Copyright 2000-2002, Opsycon AB, Sweden.\n");
	printf("Copyright 2006, Lemote Corp. Ltd., ICT CAS.\n");
	printf("CPU %s @", md_cpuname());
} 

/*
 *  Return a suitable address for the client stack.
 *  Usually top of RAM memory.
 */

register_t tgt_clienttos(void)
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

struct fl_map tgt_fl_map_boot8[]={
	TARGET_FLASH_DEVICES_8
};


struct fl_map *tgt_flashmap(void)
{
	return tgt_fl_map_boot8;
}

void tgt_flashwrite_disable(void)
{
}

int tgt_flashwrite_enable(void)
{
	return(1);
}

void tgt_flashinfo(void *p, size_t *t)
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

int tgt_flashprogram(void *p, int size, void *s, int endian)
{
	printf("Programming flash %x:%x into %x\n", s, size, p);

	if( fl_program(p, s, size, TRUE) ){
		printf("Programming failed!\n");
	}
	
	return fl_verify_device(p, s, size, TRUE);
}
#endif /* PFLASH */

/*
 *  Network stuff.
 */
void tgt_netinit(void)
{
}

int tgt_ethaddr(char *p)
{
	bcopy((void *)&hwethadr, p, 6);
	return(0);
}

void tgt_netreset(void)
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
#if 0
	/*For 2F nas NOR flash*/
	if (BONITO_IODEVCFG & BONITO_IODEVCFG_WIS16BIT) {
		struct  fl_map *fl = tgt_flashmap();
		while(fl&&fl->fl_map_base !=0) {
			fl->fl_map_width = 2;
			fl->fl_map_bus = FL_BUS_16;
			fl ++;
		}
	}
#endif

#ifdef NVRAM_IN_FLASH
	nvram = (char *)(tgt_flashmap()->fl_map_base + FLASH_OFFS);
	printf("tgt_mapenv nvram %llx\n", nvram);
	if(fl_devident((void *)(tgt_flashmap()->fl_map_base), NULL) == 0 ||
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

	(*func)("systype", SYSTYPE);
	
}

int tgt_unsetenv(char *name)
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
	nvram = (char *)((tgt_flashmap())->fl_map_base + FLASH_OFFS);

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
#if 0
			if(fl_erase_device(nvram, NVRAM_SECSIZE, TRUE)) {
				status = -1;
				break;
			}

			if(fl_program_device(nvram, nvramsecbuf, NVRAM_SECSIZE, TRUE)) {
				status = -1;
				break;
			}
#endif
			fl_program(nvram, nvramsecbuf, NVRAM_SECSIZE, TRUE);
#else
			nvram_put(nvram);
#endif
			status = 1;
			break;
		} else if(*ep != '\0') {
			while(*ep++ != '\0');
		}
	}

	free(nvramsecbuf);
	return(status);
}

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
	nvram = (char *)((tgt_flashmap())->fl_map_base + FLASH_OFFS);
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
#if 0
		if(fl_erase_device(nvram, NVRAM_SECSIZE, TRUE)) {
			printf("Error! Nvram erase failed!\n");
			free(nvramsecbuf);
			return(-1);
		}
		if(fl_program_device(nvram, nvramsecbuf, NVRAM_SECSIZE, TRUE)) {
			printf("Error! Nvram init failed!\n");
			free(nvramsecbuf);
			return(-1);
		}
#endif
		fl_program(nvram, nvramsecbuf, NVRAM_SECSIZE, TRUE);
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

	bcopy(hwethadr, &nvramsecbuf[ETHER_OFFS], 6);
	cksum(nvrambuf, NVRAM_SIZE, 1);
#ifdef NVRAM_IN_FLASH
#if 0
	if(fl_erase_device(nvram, NVRAM_SECSIZE, TRUE)) {
		printf("Error! Nvram erase failed!\n");
		free(nvramsecbuf);
		return(0);
	}
	if(fl_program_device(nvram, nvramsecbuf, NVRAM_SECSIZE, TRUE)) {
		printf("Error! Nvram program failed!\n");
		free(nvramsecbuf);
		return(0);
	}
#endif
	fl_program(nvram, nvramsecbuf, NVRAM_SECSIZE, TRUE);
#else
	nvram_put(nvramsecbuf);
#endif
	free(nvramsecbuf);
	return(1);
}


/*
 *  Calculate checksum. If 'set' checksum is calculated and set.
 */
static int cksum(void *p, size_t s, int set)
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
void nvram_get(char *buffer)
{
	int i;
	for(i = 0; i < 114; i++) {
		linux_outb(i + RTC_NVRAM_BASE, RTC_INDEX_REG);	/* Address */
		buffer[i] = linux_inb(RTC_DATA_REG);
	}
}

void nvram_put(char *buffer)
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
void tgt_display(char *msg, int x)
{
	/* Have simple serial port driver */
    /*
	tgt_putchar(msg[0]);
	tgt_putchar(msg[1]);
	tgt_putchar(msg[2]);
	tgt_putchar(msg[3]);
	*/
	int msg_length = strlen(msg);
    int j = 0;
    for (j = 0; j < msg_length; j++)
        tgt_putchar(msg[j]);

	tgt_putchar('\r');
	tgt_putchar('\n');
}

static int putDebugChar(unsigned char byte)
{
	while ((linux_inb(0x3fd) & 0x20) == 0);
	linux_outb(byte,0x3f8);
	return 1;
}

static char buf[1024];
void prom_printf(char *fmt, ...)
{
	va_list args;
	int l;
	char *p, *buf_end;

	int putDebugChar(unsigned char);

	va_start(args, fmt);
	l = vsprintf(buf, fmt, args); /* hopefully i < sizeof(buf) */
	va_end(args);

	buf_end = buf + l;

	for (p = buf; p < buf_end; p++) {
		/* Crude cr/nl handling is better than none */
#if 0
		if(*p == '\n')putDebugChar('\r');
		putDebugChar(*p);
#endif
		if(*p == '\n')tgt_putchar('\r');
		tgt_putchar(*p);
	}
}

void clrhndlrs(void)
{
}

int tgt_getmachtype(void)
{
	return(md_cputype());
}

/* update the ec_firmware */
void tgt_ecprogram(void *s, int size)
{
	int ret;

	ret = attiny44_update_rom(s, size);
	if(ret)
		printf("ret is %d, ec update error!\n");	
}

/*
 *  Create stubs if network is not compiled in
 */
#ifdef INET
void tgt_netpoll(void)
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

int netopen (const char *, int);
int netread (int, void *, int);
int netwrite (int, const void *, int);
long netlseek (int, long, int);
int netioctl (int, int, void *);
int netclose (int);
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

#include "tgt_mycmd.c"
