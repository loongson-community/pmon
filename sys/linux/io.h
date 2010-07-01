#ifndef __LINUXIO_H_
#define __LINUXIO_H_

#ifndef PTR_PAD_IO
#if (_MIPS_SZPTR == 32)
#define PTR_PAD_IO(x) (x)
#endif
#if (_MIPS_SZPTR == 64)
#ifndef _LOCORE
#define PTR_PAD_IO(x) ((0xffffffffUL <<32)|(x))
#else
#define PTR_PAD_IO(x) ((0xffffffff <<32)|(x))
#endif
#endif
#endif

#ifndef	BONITOEL
#ifdef CONFIG_PCI0_GAINT_MEM
#define mips_io_port_base PTR_PAD_IO(0xbea00000)
#else
#define mips_io_port_base PTR_PAD_IO(0xb0100000)
#endif
#else 
#define mips_io_port_base PTR_PAD_IO(0xbfd00000) 
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
        return (*(volatile unsigned int *)(mips_io_port_base + port));
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
	*(volatile unsigned int *)(mips_io_port_base + (port)) = (val);\
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




#define readb(addr)             (*(volatile unsigned char *)(PTR_PAD_IO(0xa0000000)|(long)(addr)))
#define readw(addr)             ((*(volatile unsigned short *)(PTR_PAD_IO(0xa0000000)|(long)(addr))))
#define readl(addr)             ((*(volatile unsigned int *)(PTR_PAD_IO(0xa0000000)|(long)(addr))))

#endif /* __LINUXIO_H_ */
