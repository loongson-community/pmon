#ifdef LS3_HT
#include "../../Targets/Bonito3adawning/include/bonito.h"
#endif
#ifndef __LINUXIO_H_
#define __LINUXIO_H_
#if defined(FCRSOC)||defined(BONITOEL)
#ifdef LS3_HT
#define mips_io_port_base BONITO_PCIIO_BASE_VA
#else
#define mips_io_port_base 0xbfd00000
#endif
#else
#ifdef CONFIG_PCI0_GAINT_MEM
#define mips_io_port_base 0xbea00000
#else
#define mips_io_port_base 0xb0100000
#endif
#endif
#define __SLOW_DOWN_IO \
	__asm__ __volatile__( \
		"sb\t$0,0x80(%0)" \
		: : "r" (mips_io_port_base));

#define SLOW_DOWN_IO {__SLOW_DOWN_IO; __SLOW_DOWN_IO; __SLOW_DOWN_IO; \
		__SLOW_DOWN_IO; __SLOW_DOWN_IO; __SLOW_DOWN_IO; __SLOW_DOWN_IO;  \
		__SLOW_DOWN_IO; }

static inline unsigned char linux_inb(unsigned long port)
{
        return (*(volatile unsigned char *)(mips_io_port_base + port));
}

static inline unsigned short linux_inw(unsigned long port)
{
        return (*(volatile unsigned short *)(mips_io_port_base + port));
}

static inline unsigned int linux_inl(unsigned long port)
{
        return (*(volatile unsigned long *)(mips_io_port_base + port));
}

#define linux_outb(val,port)\
do {\
*(volatile unsigned char *)(mips_io_port_base + (port)) = (val);  \
} while(0)

#define linux_outw(val,port)							\
do {									\
	*(volatile unsigned short *)(mips_io_port_base + (port)) = (val);	\
} while(0)

#define linux_outl(val,port)							\
do {									\
	*(volatile unsigned long *)(mips_io_port_base + (port)) = (val);\
} while(0)

#define linux_outb_p(val,port)                                                \
do {                                                                    \
        *(volatile unsigned char *)(mips_io_port_base + (port)) = (val);           \
        SLOW_DOWN_IO;                                                   \
} while(0)

static inline unsigned char linux_inb_p(unsigned long port)
{
	unsigned char __val;

        __val = *(volatile unsigned char *)(mips_io_port_base + port);
        SLOW_DOWN_IO;

        return __val;
}




#define readb(addr)             (*(volatile unsigned char *)(0xa0000000|(long)(addr)))
#define readw(addr)             ((*(volatile unsigned short *)(0xa0000000|(long)(addr))))
#define readl(addr)             ((*(volatile unsigned int *)(0xa0000000|(long)(addr))))

#endif /* __LINUXIO_H_ */
