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
void plat_irq_dispatch(struct trapframe *frame)
{
        unsigned int cause = read_c0_cause() & ST0_IM;
        unsigned int status = read_c0_status() & ST0_IM;
        unsigned int pending = cause & status;
        if(pending & CAUSEF_IP7)
        {
		static int cnt=0;
		printf("cnt %d\n",cnt++);
		write_c0_compare(read_c0_count()+100000000/100);
        }
        else
        {
                printf("spurious interrupt\n");
        }

}


void init_IRQ()
{
	write_c0_compare(100000000/100);
	write_c0_count(0);
	set_c0_status(0x8001);
}

