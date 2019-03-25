#include <pmon.h>
#include <errno.h>
#include <asm/mipsregs.h>
#include "include/bonito.h"
#define ST0_IM  0x0000ff00
#define CAUSEF_IP7      ( 1 << 15)
#define CAUSEF_IP6      ( 1 << 14)
#define CAUSEF_IP5      ( 1 << 13)
#define CAUSEF_IP4      ( 1 << 12)                                                                                                                     
#define CAUSEF_IP3      ( 1 << 11)
#define CAUSEF_IP2      ( 1 << 10)

static int wdt_enable()
{
*(volatile int *)0xbbef0038 = 5*125000000;
*(volatile int *)0xbbef0030 = 2;
*(volatile int *)0xbbef0034 = 1;
return 0;
}

static int wdt_disable()
{
*(volatile int *)0xbbef0038 = 5*125000000;
*(volatile int *)0xbbef0034 = 1;
*(volatile int *)0xbbef0030 = 0;
return 0;
}

static int wdt_feed()
{
*(volatile int *)0xbbef0034 = 1;
}

#define IRQ_HZ 4
static int wdt_timeout;

void plat_irq_dispatch(struct trapframe *frame)
{
        unsigned int cause = read_c0_cause() & ST0_IM;
        unsigned int status = read_c0_status() & ST0_IM;
        unsigned int pending = cause & status;
        if(pending & CAUSEF_IP7)
        {
		static int cnt=0;
		cnt++;
		write_c0_compare(read_c0_count()+400000000/IRQ_HZ);
		if(cnt<wdt_timeout*IRQ_HZ)
		{
			wdt_feed();
		}
        }
        else
        {
                printf("spurious interrupt\n");
        }

}


void init_IRQ()
{
	int wdt;
	write_c0_compare(400000000/4);
	write_c0_count(0);
	set_c0_status(0x8001);
	wdt = *(unsigned short *)(0xbfc00000+NVRAM_OFFS+WDT_OFFS);
	if(wdt==0xffff||wdt==0) wdt_timeout=300;
	else wdt_timeout=wdt;
	wdt_enable();
}

void uninit_IRQ()
{
	clear_c0_status(0x8001);
	wdt_disable();
}

