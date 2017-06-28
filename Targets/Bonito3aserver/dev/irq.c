#include <pmon.h>
#include <errno.h>
#include <asm/mipsregs.h>
#define ST0_IM  0x0000ff00
#define CAUSEF_IP7      ( 1 << 15)
#define CAUSEF_IP6      ( 1 << 14)
#define CAUSEF_IP5      ( 1 << 13)
#define CAUSEF_IP4      ( 1 << 12)                                                                                                                     
#define CAUSEF_IP3      ( 1 << 11)
#define CAUSEF_IP2      ( 1 << 10)

static int wdt_enable()
{
int d = *(volatile int *)0xbfe0011c;
int oe= *(volatile int *)0xbfe00120;
d=(d&~0x2000)|0x38;
oe=oe&~0x2038;
*(volatile int *)0xbfe0011c = d;
*(volatile int *)0xbfe00120 = oe;
return 0;
}

static int wdt_disable()
{
int d = *(volatile int *)0xbfe0011c;
int oe= *(volatile int *)0xbfe00120;
d=(d|0x2008)&~0x30;
oe=oe&~0x2038;

*(volatile int *)0xbfe0011c = d;
*(volatile int *)0xbfe00120 = oe;
return 0;
}

static int wdt_feed()
{
int d = *(volatile int *)0xbfe0011c;
int oe= *(volatile int *)0xbfe00120;
int d0, d1;
oe=oe&~0x2038;
d0=d&~(1<<14);
d1=d|(1<<14);
*(volatile int *)0xbfe0011c = d0;
*(volatile int *)0xbfe00120 = oe;
*(volatile int *)0xbfe0011c = d1;
}

#define IRQ_HZ 4

void plat_irq_dispatch(struct trapframe *frame)
{
        unsigned int cause = read_c0_cause() & ST0_IM;
        unsigned int status = read_c0_status() & ST0_IM;
        unsigned int pending = cause & status;
        if(pending & CAUSEF_IP7)
        {
		static int cnt=0;
		//tgt_printf("cnt %d\n",cnt++);
		write_c0_compare(read_c0_count()+400000000/IRQ_HZ);
		if(cnt<60*IRQ_HZ)
			wdt_feed();
        }
        else
        {
                printf("spurious interrupt\n");
        }

}


void init_IRQ()
{
	write_c0_compare(400000000/4);
	write_c0_count(0);
	set_c0_status(0x8001);
	wdt_enable();
}

void uninit_IRQ()
{
	clear_c0_status(0x8001);
	wdt_disable();
}

