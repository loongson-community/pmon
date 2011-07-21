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
#include "lvds_reg.h"
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

#include "mod_x86emu_int10.h"
#include "mod_x86emu.h"
#include "mod_vgacon.h"
#include "mod_framebuffer.h"
#include "mod_smi712.h"
#include "mod_smi502.h"
#include "mod_sisfb.h"

#include "mycmd.c"

#if (NMOD_X86EMU_INT10 > 0)||(NMOD_X86EMU >0)
extern int vga_bios_init(void);
#endif

#if NMOD_FRAMEBUFFER > 0
extern struct pci_device *vga_dev;
#endif
extern int kbd_initialize(void);
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
#elif defined(VGAROM_IN_BIOS)
#include "vgarom.c"
#endif

extern struct trapframe DBGREG;
extern void *memset(void *, int, size_t);

int kbd_available;
int usb_kbd_available;;
int vga_available;

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

ConfigEntry	ConfigTable[] =
{
#ifdef HAVE_NB_SERIAL
	#ifdef DEVBD2F_FIREWALL
	 { (char *)LS2F_COMA_ADDR, 0, ns16550, 256, CONS_BAUD, NS16550HZ/2 },
	#else
    #ifdef USE_LPC_UART
	 { (char *)COM3_BASE_ADDR, 0, ns16550, 256, CONS_BAUD, NS16550HZ },
    #else
	 //{ (char *)0xbfe001e0, 0, ns16550, 256, CONS_BAUD, NS16550HZ },
	 { (char *)GS3_UART_BASE, 0, ns16550, 256, CONS_BAUD, NS16550HZ },
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

//extern unsigned long long  memorysize;
//extern unsigned long long  memorysize_high;
extern unsigned int  memorysize;
extern unsigned int  memorysize_high;

extern char MipsException[], MipsExceptionEnd[];

unsigned char hwethadr[6];

void initmips(unsigned int memsz);

void addr_tst1(void);
void addr_tst2(void);
void movinv1(int iter, ulong p1, ulong p2);

pcireg_t _pci_allocate_io(struct pci_device *dev, vm_size_t size);
static void superio_reinit();
void lvds_reg_init()
{
    int i=0;
    unsigned long ioaddress;

	ioaddress  =_pci_conf_read(vga_dev->pa.pa_tag,0x18);
    ioaddress = ioaddress &0xfffffff0; //laster 4 bit
    ioaddress |= ioaddress | 0xb0000000;
    printf("ioaddress:%x for LVDS patch\n ",ioaddress);

    while(lvds_reg[i].reg) {
        *(volatile unsigned int *)(ioaddress + lvds_reg[i].reg*4)  = lvds_reg[i].val;
        delay(1000);
        i++;
    }
}

void
initmips(unsigned int memsz)
{
	int i;
	int* io_addr;
    unsigned int tmp_memsz = 0;
    
    tgt_fpuenable();
    tgt_printf("memsz %d\n",memsz);
    /*enable float*/
    tgt_fpuenable();
    CPU_TLBClear();

	/*
	 *	Set up memory address decoders to map entire memory.
	 *	But first move away bootrom map to high memory.
	 */
#if 0
	GT_WRITE(BOOTCS_LOW_DECODE_ADDRESS, BOOT_BASE >> 20);
	GT_WRITE(BOOTCS_HIGH_DECODE_ADDRESS, (BOOT_BASE - 1 + BOOT_SIZE) >> 20);
#endif
    /*
      tmp_memsz = memsz * 256M
    */
    tmp_memsz = memsz;
    tmp_memsz = tmp_memsz << 28;
    tmp_memsz = tmp_memsz >> 20;
    tmp_memsz = tmp_memsz - 16;
    tgt_printf("tmp_memsz : %d\n",tmp_memsz);
#ifdef CONFIG_GFXUMA
	memorysize = tmp_memsz > 128 ? 128 << 20 : tmp_memsz << 20;
#else
	memorysize = tmp_memsz > 240 ? 240 << 20 : tmp_memsz << 20;
#endif
	memorysize_high = tmp_memsz > 240 ? (tmp_memsz - 240) << 20 : 0;

    tgt_printf("memorysize : %d memorysize_high : %d\n",memorysize,memorysize_high);

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

#ifndef ROM_EXCEPTION
	CPU_SetSR(0, SR_BOOT_EXC_VEC);
#endif
	SBD_DISPLAY("BEV0",0);
	
	printf("BEV in SR set to zero.\n");

	/*
	 * Launch!
	 */
	_pci_conf_write(_pci_make_tag(0,0,0),0x90,0xff800000); 
	main();
}

/*
 *  Put all machine dependent initialization here. This call
 *  is done after console has been initialized so it's safe
 *  to output configuration and debug information with printf.
 */
int psaux_init(void);
extern int video_hw_init (void);

extern int fb_init(unsigned long,unsigned long);
void
tgt_devconfig()
{
#if NMOD_VGACON > 0
	int rc=1;
#if NMOD_FRAMEBUFFER > 0 
	unsigned long fbaddress,ioaddress;
	//extern struct pci_device *vga_dev;
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
lvds_reg_init();
#endif
#if NMOD_FRAMEBUFFER > 0
	vga_available=0;
	if(!vga_dev) {
		printf("ERROR !!! VGA device is not found\n"); 
		rc = -1;
	}
	if (rc > 0) {
		fbaddress  =_pci_conf_read(vga_dev->pa.pa_tag,0x10);
		ioaddress  =_pci_conf_read(vga_dev->pa.pa_tag,0x18);

		fbaddress = fbaddress &0xffffff00; //laster 8 bit
		ioaddress = ioaddress &0xfffffff0; //laster 4 bit

		printf("fbaddress 0x%x\tioaddress 0x%x\n",fbaddress, ioaddress);


#if (SHARED_VRAM == 128)
		fbaddress = 0xf8000000;//64M graph memory
#elif (SHARED_VRAM == 64)
		fbaddress = 0xf8000000;//64M graph memory
#elif (SHARED_VRAM == 32)
		fbaddress = 0xfe000000;//32 graph memory
#endif

		/* lwg add.
		 * The address mapped from 0x10000000 to 0xf800000
		 * wouldn't work through tlb.
		_*/
#ifdef CONFIG_GFXUMA
		fbaddress = 0x88000000; /* FIXME */
#else
		fbaddress = 0xb0000000; /* FIXME */
#endif

		printf("begin fb_init\n");
		fb_init(fbaddress, ioaddress);
		printf("after fb_init\n");

	} else {
		printf("vga bios init failed, rc=%d\n",rc);
	}
#endif

/*
#if (NMOD_FRAMEBUFFER > 0) || (NMOD_VGACON > 0 )
    if (rc > 0)
	 if(!getenv("novga")) vga_available=1;
	 else vga_available=0;
#endif
*/
    config_init();
    printf("before calling configure()\n");
    configure();
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
//	psaux_init();
#endif
   clock_spread();
   printf("devconfig done.\n");
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
}
#endif


void
tgt_devinit()
{
	int value;

	printf("rs780_early_setup\n");
	rs780_early_setup();

	printf("sb700_early_setup\n");
	sb700_early_setup();
	printf("rs780_before_pci_fixup\n");
	rs780_before_pci_fixup();
	printf("sb700_before_pci_fixup\n");
	sb700_before_pci_fixup();
	printf("rs780_enable\n");
	//vga test
	//rs780_enable(_pci_make_tag(0, 0, 0));
	//rs780_enable(_pci_make_tag(0, 1, 0));
	rs780_after_pci_fixup(); //name dug
	printf("sb700_enable\n");
	sb700_enable();

#if 1
	printf("disable bus0 device pcie bridges\n");
	//disable bus0 device 3 pci bridges (dev2 to dev7, dev9-dev10)
	//dev 2 and dev3 should not be open otherwise the vga could not work
	//==by oldtai
	set_nbmisc_enable_bits(_pci_make_tag(0,0,0), 0x0c,(1<<2|1<<3|1<<4|1<<5|1<<6|1<<7|1<<16|1<<17),
			(1<<2|1<<3|0<<4|0<<5|0<<6|0<<7|0<<16|0<<17));
#endif

#ifndef USE_780E_VGA 
	printf("disable internal graphics\n");
	//disable internal graphics
	set_nbcfg_enable_bits_8(_pci_make_tag(0,0,0), 0x7c, 1, 1);
#endif

#if 1
	//SBD_DISPLAY("disable OHCI and EHCI controller",0);
	//disable OHCI and EHCI controller
	printf("disable OHCI and EHCI controller\n");
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
	_pci_conf_write8(_pci_make_tag(0,0x14, 0), 0x68, (1<<0)|(1<<1)|(1<<4)|(1<<5)|(1<<7));
	//  _pci_conf_write8(_pci_make_tag(0,0x14, 0), 0x68, (1<<0));
#endif


#ifndef ENABLE_SATA 
	//SBD_DISPLAY("disable data",0);
	//disable sata
	printf("disable sata\n");
	value = _pci_conf_read32(_pci_make_tag(0, 0x14, 0), 0xac);
	value &= ~(1 << 8);
	_pci_conf_write32(_pci_make_tag(0, 0x14, 0), 0xac, value);
#endif

    //Add codec PCI Subsystem vendor&Subsystem id
    _pci_conf_write32(_pci_make_tag(0,0x14,2), 0x2c, 0x20111c06);
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

	printf("sb700_after_pci_fixup\n");
	sb700_after_pci_fixup();
}


void
tgt_reboot()
{
	/* Send reboot command */
	ec_wr_noindex(CMD_RESET, INDEX_RESET_ON);

	while(1);
}

void
tgt_poweroff()
{
	ec_wr_noindex(CMD_RESET, INDEX_PWROFF_ON);
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
}

static void init_legacy_rtc(void)
{
		int year, month, date, hour, min, sec;
        unsigned char ctlsave;
        
        CMOS_WRITE(DS_CTLA_DV1|0xd, DS_REG_CTLA);
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
    unsigned char ctlval;

    SBD_DISPLAY ("FREQ", CHKPNT_FREQ);

    md_pipefreq = 660000000;        /* NB FPGA*/
    md_cpufreq  =  60000000;


    clk_invalid = 1;
#ifdef HAVE_TOD
    init_legacy_rtc();
    SBD_DISPLAY ("FREI", CHKPNT_FREQ);

    /*
     * Do the next twice for two reasons. First make sure we run from
     * cache. Second make sure synched on second update. (Pun intended!)
     */
aa:
    for(i = 2;  i != 0; i--) { 
        while((CMOS_READ(DS_REG_CTLC) & DS_CTLC_PF) == 0x0);
        cnt = CPU_GetCOUNT();
        while((CMOS_READ(DS_REG_CTLC) & DS_CTLC_PF) == 0x0);
        cnt = CPU_GetCOUNT() - cnt;

    }
    clk_invalid = 0;
    md_pipefreq = cnt * 16;
    /* we have no simple way to read multiplier value
	 */
    md_cpufreq = 66000000;
    tgt_printf("cpu fre %u\n",md_pipefreq);
    tgt_printf("pipe fre %u\n",md_cpufreq);

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
	return((register_t)(int)PHYS_TO_CACHED(memorysize & ~7) - 64);
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

int
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
	return fl_verify_device(p, s, size, TRUE);
}
#endif /* PFLASH */

#ifdef LOONGSON3A_3A780E
/* update the ec_firmware */
void tgt_ecprogram(void *s, int size)
{
	ec_update_rom(s, size);
 	return;
}
#endif /* ifdef LOONGSON3A_3A780E, update the ec_firmware */

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
    nvram = (char *)(tgt_flashmap())->fl_map_base;
	printf("nvram=%08x\n",(unsigned int)nvram);
	if(fl_devident(nvram, NULL) == 0 ||
           cksum(nvram + NVRAM_OFFS, NVRAM_SIZE, 0) != 0) {
#else
    nvram = (char *)malloc(512);
	nvram_get(nvram);
	if(cksum(nvram, NVRAM_SIZE, 0) != 0) {
#endif
		        printf("NVRAM is invalid!\n");
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
//#include "mycmd.c"
#include "i2c-via.c"
/*
#ifdef DEVBD2F_FIREWALL
#include "i2c-sm502.c"
#elif defined(DEVBD2F_SM502)
#include "i2c-sm502.c"

#elif (PCI_IDSEL_CS5536 != 0)
#include "i2c-cs5536.c"
#else
#include "i2c-via.c"
#endif
*/

