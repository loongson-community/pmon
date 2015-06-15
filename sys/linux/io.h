#if defined(LS3_HT) || defined(LS2G_HT)
#include <target/bonito.h>
#endif

#ifndef __LINUXIO_H_
#define __LINUXIO_H_
#if defined(FCRSOC)||defined(BONITOEL)

#if defined(LS3_HT)||defined(LS2G_HT)
#if defined(LOONGSON_3A2H)
#define mips_io_port_base 0xbff00000
#ifdef PCIE_GRAPHIC_CARD
#define pci_mips_io_port_base 0xb8100000
#endif
#elif defined(LOONGSON_3C2H)
#define mips_io_port_base 0xbbf00000 // for keyboard connected with ls3c lpc 
#else
#define mips_io_port_base BONITO_PCIIO_BASE_VA
#endif
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

#if !defined(LS2G_AMD) /* modify for x86emu */
#define readb(addr)             (*(volatile unsigned char *)(0xa0000000|(long)(addr)))
#define readw(addr)             ((*(volatile unsigned short *)(0xa0000000|(long)(addr))))
#define readl(addr)             ((*(volatile unsigned int *)(0xa0000000|(long)(addr))))
#else
#define readb(addr)             (*(volatile unsigned char *)(0x00000000|(long)(addr)))
#define readw(addr)             ((*(volatile unsigned short *)(0x00000000|(long)(addr))))
#define readl(addr)             ((*(volatile unsigned int *)(0x00000000|(long)(addr))))
#endif

#ifdef PCIE_GRAPHIC_CARD
#define ___MYPCI_SLOW_DOWN_IO \
       __asm__ __volatile__( \
               "sb\t$0,0x80(%0)" \
               : : "r" (pci_mips_io_port_base));

#define MYPCI_SLOW_DOWN_IO {___MYPCI_SLOW_DOWN_IO; ___MYPCI_SLOW_DOWN_IO; ___MYPCI_SLOW_DOWN_IO; \
               ___MYPCI_SLOW_DOWN_IO; ___MYPCI_SLOW_DOWN_IO; ___MYPCI_SLOW_DOWN_IO; ___MYPCI_SLOW_DOWN_IO;  \
               ___MYPCI_SLOW_DOWN_IO; }

static inline unsigned char pci_linux_inb(unsigned long port)
{
        return (*(volatile unsigned char *)(pci_mips_io_port_base + port));
}

static inline unsigned short pci_linux_inw(unsigned long port)
{
        return (*(volatile unsigned short *)(pci_mips_io_port_base + port));
}

static inline unsigned int pci_linux_inl(unsigned long port)
{
        return (*(volatile unsigned long *)(pci_mips_io_port_base + port));
}


#define pci_linux_outb(val,port)\
do {\
*(volatile unsigned char *)(pci_mips_io_port_base + (port)) = (val);  \
} while(0)

#define pci_linux_outw(val,port)                                                     \
do {                                                                   \
       *(volatile unsigned short *)(pci_mips_io_port_base + (port)) = (val); \
} while(0)

#define pci_linux_outl(val,port)                                                     \
do {                                                                   \
       *(volatile unsigned long *)(pci_mips_io_port_base + (port)) = (val);\
} while(0)

#define pci_linux_outb_p(val,port)                                                \
do {                                                                    \
        *(volatile unsigned char *)(pci_mips_io_port_base + (port)) = (val);           \
        MYPCI_SLOW_DOWN_IO;                                                   \
} while(0)

static inline unsigned char pci_linux_inb_p(unsigned long port)
{
       unsigned char __val;

        __val = *(volatile unsigned char *)(pci_mips_io_port_base + port);
        MYPCI_SLOW_DOWN_IO;

        return __val;
}
#endif

#endif /* __LINUXIO_H_ */
