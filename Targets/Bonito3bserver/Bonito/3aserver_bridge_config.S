/*--------------------------------------------------------------------------*/
//add by lycheng(liych@3adawning.com.cn)
#define PCI_ADDR(BUS, DEV, FN, WHERE) ( \
        (((BUS) & 0xFF) << 16) | \
        (((DEV) & 0x1f) << 11) | \
        (((FN) & 0x07) << 8) | \
        ((WHERE) & 0xFF))
#define PCI_DEV(BUS, DEV, FN) ( \
        (((BUS) & 0xFF) << 16) | \
        (((DEV) & 0x1f) << 11) | \
        (((FN)  & 0x7) << 8))
#define CPU_POST_PORT  0xbff00080
#define HT_CONFIG_ADDR 0x90001efdfe000000  //type0: 0x90000efdfe000000   type1:0x90000efdff000000
#define NBMISC_INDEX 0x60

//end by lycheng

//lycheng
	PRINTSTR("\r\n enable rs780 dev8...\r\n")
        nop
        bal enable_rs780_dev8
        nop
        PRINTSTR("\r\n test GPPSB\r\n")
        li   a0,  0x0
        li   a1,  0x0
        bal    nbmisc_read_index_mips
        nop
        li     t0,  0x40  // (1 << 6)
        and    v1, v0, t0
        bnez    v1, 1f
        nop
        PRINTSTR("\r\n Enabled GPPSB fail...\r\n")
        nop
        b 2f
        nop
1:
        PRINTSTR("\r\n Enabled GPPSB success...\r\n")
        nop
2:
        PRINTSTR("\r\n sb700 lpc init...\r\n")
        nop
	/* sb700_lpc_init();*/
        /* SMBUS controller */
        /* NOTE: Set BootTimerDisable, otherwise it would keep rebooting!! */
        PRINTSTR("\r\n set boottimerdisable\r\n")
        li a0, PCI_DEV(0x0,0x14,0x0)
        li a1, 0x4C
        bal pci_read_config32_mips
        nop
        or a2, v0, 1<<31
        bal pci_write_config32_mips
        nop
        PRINTSTR("\r\n enable lpc controller\r\n")
        /* Enable lpc controller */
        li a1, 0x64
        bal pci_read_config32_mips
        nop
        or a2, v0, 1<<20
        bal pci_write_config32_mips
        nop
        PRINTSTR("\r\n enable port 80 LPC decode\r\n")
        /* Enable port 80 LPC decode in pci function 3 configuration space. */
        li a0, PCI_DEV(0x0, 0x14, 0x3)
        li a1, 0x4a
        bal pci_read_config8_mips
        nop
        or a2, v0, 1<<5
        bal pci_write_config8_mips
        nop
        /* Decode port 0x3f8-0x3ff (Serial 0) */
        PRINTSTR("\r\n Decode port 0x3f8-0x3ff\r\n")
	li a1, 0x44
        bal pci_read_config8_mips
        nop
        or a2, v0, 1<<6
        bal pci_write_config8_mips
        nop
        /* Decode port 0x60 & 0x64 (PS/2 keyboard) and port 0x62 & 0x66 (ACPI)*/
        PRINTSTR("\r\n Decode port 0x60-0x66\r\n")
        li a1, 0x47
        bal pci_read_config8_mips
        nop
        or a2, v0, (1<<5)|(1<<6)
        bal pci_write_config8_mips
        nop

        PRINTSTR("\r\n SuperIO RTC\r\n")
        /* SuperIO, RTC */
        li a1, 0x48
        bal pci_read_config8_mips
        nop
        or a2, v0, (1<<1)|(1<<0)|(1<<6)
        bal pci_write_config8_mips
        nop
        li a0, 0x01
        bal post_code_mips
        nop


