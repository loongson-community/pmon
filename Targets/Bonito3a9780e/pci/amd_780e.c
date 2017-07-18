#include "rs780_cmn.h"
#include "amd_780e.h"

#include "rs780_pcie.c"
#include "rs780_gfx.c"
#include "rs780.c"
#include "sb700_usb.c"
#include "sb700_lpc.c"
#include "sb700_ide.c"
#include "sb700_sata.c"
#include "sb700_sm.c"
#include "sb700_pci.c"
//#include "blade_io.h"

#define INB(addr) (*(volatile unsigned char *) (addr))
#define INW(addr) (*(volatile unsigned short *) (addr))
#define INL(addr) (*(volatile unsigned int *) (addr))

#define OUTB(b,addr) (*(volatile unsigned char *) (addr) = (b))
#define OUTW(b,addr) (*(volatile unsigned short *) (addr) = (b))
#define OUTL(b,addr) (*(volatile unsigned int *) (addr) = (b))

static u8 get_sb700_revision(void);

void rs780_por_pcicfg_init(device_t nb_tag)
{
	printk_info("enter rs780_por_pcicfg_init\n");
	/* enable PCI Memory Access */
	set_nbcfg_enable_bits_8(nb_tag, 0x04, (u8)(~0xfd), 0x02);
	/* Set RCRB Enable */
	set_nbcfg_enable_bits_8(nb_tag, 0x84, (u8)(~0xFF), 0x1);
	
	/* allow decode of 640k-1MB */
	set_nbcfg_enable_bits_8(nb_tag, 0x84, (u8)(~0xEF), 0x10);
#if 1	
	/* Enable PM2_CNTL(BAR2) IO mapped cfg write access to be broadcast to both NB and SB */
	set_nbcfg_enable_bits_8(nb_tag, 0x84, (u8)(~0xFF), 0x4);
	/* Power Management Register Enable */
	set_nbcfg_enable_bits_8(nb_tag, 0x84, (u8)(~0xFF), 0x80);

	/* Reg4Ch[1]=1 (APIC_ENABLE) force cpu request with address 0xFECx_xxxx to south-bridge
	 * Reg4Ch[6]=1 (BMMsgEn) enable BM_Set message generation
	 * BMMsgEn */
	//set_nbcfg_enable_bits_8(nb_tag, 0x4C, (u8)(~0x00), 0x42 | 1);
	//lycheng disable APIC_ENABLE
	set_nbcfg_enable_bits_8(nb_tag, 0x4C, (u8)(~0x00), 0x40 | 1);

	/* Reg4Ch[16]=1 (WakeC2En) enable Wake_from_C2 message generation.
	 * Reg4Ch[18]=1 (P4IntEnable) Enable north-bridge to accept MSI with address 0xFEEx_xxxx from south-bridge */
	//set_nbcfg_enable_bits_8(nb_tag, 0x4E, (u8)(~0xFF), 0x05);
	//lycheng  disable P4IntEnable
	set_nbcfg_enable_bits_8(nb_tag, 0x4E, (u8)(~0xFF), 0x01);
#endif
	/* Reg94h[4:0] = 0x0  P drive strength offset 0
	 * Reg94h[6:5] = 0x2  P drive strength additive adjust */
	set_nbcfg_enable_bits_8(nb_tag, 0x94, (u8)(~0x80), 0x40);

	/* Reg94h[20:16] = 0x0  N drive strength offset 0
	 * Reg94h[22:21] = 0x2  N drive strength additive adjust */
	set_nbcfg_enable_bits_8(nb_tag, 0x96, (u8)(~0x80), 0x40);

	/* Reg80h[4:0] = 0x0  Termination offset
	 * Reg80h[6:5] = 0x2  Termination additive adjust */
	set_nbcfg_enable_bits_8(nb_tag, 0x80, (u8)(~0x80), 0x40);

	/* Reg80h[14] = 0x1   Enable receiver termination control */
	set_nbcfg_enable_bits_8(nb_tag, 0x81, (u8)(~0xFF), 0x40);

	/* Reg94h[15] = 0x1 Enables HT transmitter advanced features to be turned on
	 * Reg94h[14] = 0x1  Enable drive strength control */
	set_nbcfg_enable_bits_8(nb_tag, 0x95, (u8)(~0x3F), 0xC4);

	/* Reg94h[31:29] = 0x7 Enables HT transmitter de-emphasis */
	set_nbcfg_enable_bits_8(nb_tag, 0x97, (u8)(~0x1F), 0xE0);

	/*Reg8Ch[10:9] = 0x3 Enables Gfx Debug BAR,
	 * force this BAR as mem type in rs780_gfx.c */
	set_nbcfg_enable_bits_8(nb_tag, 0x8D, (u8)(~0xFF), 0x03);

	printk_info("exit rs780_por_pcicfg_init\n");
}


/*****************************************
* Compliant with CIM_33's ATINB_MCIndex_POR_TABLE
*****************************************/
static void rs780_por_mc_index_init(device_t nb_dev)
{
	printk_info("enter rs780_por_mc_index_init\n");
	set_nbmc_enable_bits(nb_dev, 0x7A, ~0xFFFFFF80, 0x0000005F);
	set_nbmc_enable_bits(nb_dev, 0xD8, ~0x00000000, 0x00600060);
	set_nbmc_enable_bits(nb_dev, 0xD9, ~0x00000000, 0x00600060);
	set_nbmc_enable_bits(nb_dev, 0xE0, ~0x00000000, 0x00000000);
	set_nbmc_enable_bits(nb_dev, 0xE1, ~0x00000000, 0x00000000);
	set_nbmc_enable_bits(nb_dev, 0xE8, ~0x00000000, 0x003E003E);
	set_nbmc_enable_bits(nb_dev, 0xE9, ~0x00000000, 0x003E003E);
	printk_info("exit rs780_por_mc_index_init\n");
}

/*****************************************
* Compliant with CIM_33's ATINB_MISCIND_POR_TABLE
* Compliant with CIM_33's MISC_INIT_TBL
*****************************************/
static void rs780_por_misc_index_init(device_t nb_dev)
{
	printk_info("enter rs780_por_misc_index_init\n");
	/* NB_MISC_IND_WR_EN + IOC_PCIE_CNTL
	 * Block non-snoop DMA request if PMArbDis is set.
	 * Set BMSetDis */
	set_nbmisc_enable_bits(nb_dev, 0x0B, ~0xFFFF0000, 0x00000180);
	set_nbmisc_enable_bits(nb_dev, 0x01, ~0xFFFFFFFF, 0x00000040);

	/* NBCFG (NBMISCIND 0x0): NB_CNTL -
	 *   HIDE_NB_AGP_CAP  ([0], default=1)HIDE
	 *   HIDE_P2P_AGP_CAP ([1], default=1)HIDE
	 *   HIDE_NB_GART_BAR ([2], default=1)HIDE
	 *   AGPMODE30        ([4], default=0)DISABLE
	 *   AGP30ENCHANCED   ([5], default=0)DISABLE
	 *   HIDE_AGP_CAP     ([8], default=1)ENABLE */
	set_nbmisc_enable_bits(nb_dev, 0x00, ~0xFFFF0000, 0x00000506);	/* set bit 10 for MSI */
	//lycheng
	//set_nbmisc_enable_bits(nb_dev, 0x00, ~0xFFFF0000, 0x00000004);	/* set bit 10 for MSI */
	set_nbmisc_enable_bits(nb_dev, 0x00, 1 << 10, 1 << 10);   //added by oldtai
	set_nbmisc_enable_bits(nb_dev, 0x01, 1 << 8, 1 << 8);   //added by oldtai


	/* NBMISCIND:0x6A[16]= 1 SB link can get a full swing
	 *      set_nbmisc_enable_bits(nb_dev, 0x6A, 0ffffffffh, 000010000);
	 * NBMISCIND:0x6A[17]=1 Set CMGOOD_OVERRIDE. */
	set_nbmisc_enable_bits(nb_dev, 0x6A, ~0xffffffff, 0x00020000);

	/* NBMISIND:0x40 Bit[8]=1 and Bit[10]=1 following bits are required to set in order to allow LVDS or PWM features to work. */
	set_nbmisc_enable_bits(nb_dev, 0x40, ~0xffffffff, 0x00000500);

	/* NBMISIND:0xC Bit[13]=1 Enable GSM mode for C1e or C3 with pop-up. */
	set_nbmisc_enable_bits(nb_dev, 0x0C, ~0xffffffff, 0x00002000);

	/* Set NBMISIND:0x1F[3] to map NB F2 interrupt pin to INTB# */
	set_nbmisc_enable_bits(nb_dev, 0x1F, ~0xffffffff, 0x00000008);

	/* Compliant with CIM_33's MISC_INIT_TBL, except Hide NB_BAR3_PCIE
	 * Enable access to DEV8
	 * Enable setPower message for all ports
	 */
	set_nbmisc_enable_bits(nb_dev, 0x00, 1 << 6, 1 << 6);
	set_nbmisc_enable_bits(nb_dev, 0x0b, 1 << 20, 1 << 20);
	set_nbmisc_enable_bits(nb_dev, 0x51, 1 << 20, 1 << 20);
	set_nbmisc_enable_bits(nb_dev, 0x53, 1 << 20, 1 << 20);
	set_nbmisc_enable_bits(nb_dev, 0x55, 1 << 20, 1 << 20);
	set_nbmisc_enable_bits(nb_dev, 0x57, 1 << 20, 1 << 20);
	set_nbmisc_enable_bits(nb_dev, 0x59, 1 << 20, 1 << 20);
	set_nbmisc_enable_bits(nb_dev, 0x5B, 1 << 20, 1 << 20);
	//VGA lycheng
	set_nbmisc_enable_bits(nb_dev, 0x5D, 1 << 20, 1 << 20);
	set_nbmisc_enable_bits(nb_dev, 0x5F, 1 << 20, 1 << 20);

	set_nbmisc_enable_bits(nb_dev, 0x00, 1 << 7, 1 << 7);
	set_nbmisc_enable_bits(nb_dev, 0x07, 0x000000f0, 0x30);
	/* Disable bus-master trigger event from SB and Enable set_slot_power message to SB */
	set_nbmisc_enable_bits(nb_dev, 0x0B, 0xffffffff, 0x500180);
	printk_info("exit rs780_por_misc_index_init\n");
}

/*****************************************
* Compliant with CIM_33's ATINB_HTIUNBIND_POR_TABLE
*****************************************/
static void rs780_por_htiu_index_init(device_t nb_dev)
{
	printk_info("enter rs780_por_htiu_index_init\n");
	//vga lycheng
	set_htiu_enable_bits(nb_dev, 0x05, (1<<10|1<<9), 1<<10 | 1<<9);
	set_htiu_enable_bits(nb_dev, 0x06, ~0xFFFFFFFE, 0x04203A202);

	set_htiu_enable_bits(nb_dev, 0x07, ~0xFFFFFFF9, 0x8001/*  | 7 << 8 */); /* fam 10 */

	set_htiu_enable_bits(nb_dev, 0x15, ~0xFFFFFFFF, 1<<31| 1<<30 | 1<<27);
	set_htiu_enable_bits(nb_dev, 0x1C, ~0xFFFFFFFF, 0xFFFE0000);

	set_htiu_enable_bits(nb_dev, 0x4B, (1<<11), 1<<11);

	set_htiu_enable_bits(nb_dev, 0x0C, ~0xFFFFFFC0, 1<<0|1<<3);

	set_htiu_enable_bits(nb_dev, 0x17, (1<<27|1<<1), 0x1<<1);
	set_htiu_enable_bits(nb_dev, 0x17, 0x1 << 30, 0x1<<30);

	set_htiu_enable_bits(nb_dev, 0x19, (0xFFFFF+(1<<31)), 0x186A0+(1<<31));

	set_htiu_enable_bits(nb_dev, 0x16, (0x3F<<10), 0x7<<10);

	set_htiu_enable_bits(nb_dev, 0x23, 0xFFFFFFF, 1<<28);

	set_htiu_enable_bits(nb_dev, 0x1E, 0xFFFFFFFF, 0xFFFFFFFF);

/* here we set lower of top of dram2 to 0x0 and enabled*/
	printk_info("before lower tom2 is %x, upper tom2 %x\n",
			htiu_read_indexN(nb_dev, 0x30),
			htiu_read_indexN(nb_dev, 0x31));

	htiu_write_indexN(nb_dev, 0x30, 0x01);
	htiu_write_indexN(nb_dev, 0x31, 0x80);


/* here we set upper of top of dram2 to 0x0 and enabled, so the top
 * of dram 2 is 0x80 0000 0000 = 512GB*/
	printk_info("lower tom2 is %x, upper tom2 %x\n",
			htiu_read_indexN(nb_dev, 0x30),
			htiu_read_indexN(nb_dev, 0x31));

	
	printk_info("exit rs780_por_htiu_index_init\n");
}



void rs780_por_init(device_t nb_tag)
{
	/* ATINB_PCICFG_POR_TABLE, initialize the values for rs780 PCI Config registers */
	rs780_por_pcicfg_init(nb_tag);
	/* ATINB_MCIND_POR_TABLE */
	rs780_por_mc_index_init(nb_tag);

	/* ATINB_MISCIND_POR_TABLE */
	rs780_por_misc_index_init(nb_tag);

	/* ATINB_HTIUNBIND_POR_TABLE */
	rs780_por_htiu_index_init(nb_tag);

	/* ATINB_CLKCFG_PORT_TABLE */
	/* rs780 A11 SB Link full swing? */
}


void rs780_early_setup(void)
{
	device_t  nb_tag;
	nb_tag = _pci_make_tag(0, 0, 0);
	rs780_por_init(nb_tag);
}


#define SMBUS_IO_BASE 0x1000  //ynn

/* sbDevicesPorInitTable */
void sb700_devices_por_init(void)
{
	device_t dev;
	u8 byte;

	/* SMBus Device, BDF:0-20-0 */
	
	dev = _pci_make_tag(0, 20, 0);

	/* sbPorAtStartOfTblCfg */
	/* Set A-Link bridge access address. This address is set at device 14h, function 0, register 0xf0.
	 * This is an I/O address. The I/O address must be on 16-byte boundry.  */
	printk_info("set a-link bridge access address\n");
	//pci_write_config32(dev, 0xf0, AB_INDX);
	pci_write_config32(dev, 0xf0, 0x00000cd8);

	/* To enable AB/BIF DMA access, a specific register inside the BIF register space needs to be configured first. */
	/*Enables the SB600 to send transactions upstream over A-Link Express interface. */
	printk_info("To enable ab bif dam access\n");
	axcfg_reg(0x04, 1 << 2, 1 << 2);
	axindxc_reg(0x21, 0xff, 0);

	/* 2.3.5:Enabling Non-Posted Memory Write for the K8 Platform */
	printk_info("Enabling Non-Posted Memory Write for the K8 Platform\n");
	axindxc_reg(0x10, 1 << 9, 1 << 9);
	/* END of sbPorAtStartOfTblCfg */
	
	/* sbDevicesPorInitTables */
	/* set smbus iobase */
	printk_info("set smbus iobase\n");
	//pci_write_config32(dev, 0x10, SMBUS_IO_BASE | 1);
	//vga lycheng
	pci_write_config32(dev, 0x90, SMBUS_IO_BASE | 1);

	/* enable smbus controller interface */
	printk_info("enable smbus controller interface\n");
	byte = pci_read_config8(dev, 0xd2);
	byte |= (1 << 0);
	pci_write_config8(dev, 0xd2, byte);

#if 0
	/* set smbus 1, ASF 2.0 (Alert Standard Format), iobase */
	printk_info("enable smbus 1, ASF 2.0\n");
	pci_write_config16(dev, 0x58, SMBUS_IO_BASE | 0x11);
#endif
	/* TODO: I don't know the useage of followed two lines. I copied them from CIM. */
	pci_write_config8(dev, 0x0a, 0x1);
	pci_write_config8(dev, 0x0b, 0x6);

	/* KB2RstEnable */
	printk_info("KB2RstEnable\n");
	pci_write_config8(dev, 0x40, 0xd4);

	/* Enable ISA Address 0-960K decoding */
	printk_info("Enable ISA address decoding\n");
	pci_write_config8(dev, 0x48, 0x0f);

	/* Enable ISA  Address 0xC0000-0xDFFFF decode */
	printk_info("Enable ISA address 0xc0000-0xdffff decode\n");
	pci_write_config8(dev, 0x49, 0xff);

	/* Enable decode cycles to IO C50, C51, C52 GPM controls. */
	printk_info("Enable decode cycles to IO controls\n");
	byte = pci_read_config8(dev, 0x41);
	byte &= 0x80;
	byte |= 0x33;
	pci_write_config8(dev, 0x41, byte);

	/* Legacy DMA Prefetch Enhancement, CIM masked it. */
	/* pci_write_config8(dev, 0x43, 0x1); */

	/* Disabling Legacy USB Fast SMI# */
	printk_info("Disabling Legacy USB Fast SMI\n");
	byte = pci_read_config8(dev, 0x62);
	byte |= 0x24;
	pci_write_config8(dev, 0x62, byte);

	/* Features Enable */
	printk_info("Features Enable\n");
	pci_write_config32(dev, 0x64, 0x829E7DBF); /* bit10: Enables the HPET interrupt. */

	/* SerialIrq Control */
	printk_info("SerialIrq Control\n");
	pci_write_config8(dev, 0x69, 0x90);

	/* Test Mode, PCIB_SReset_En Mask is set. */
	pci_write_config8(dev, 0x6c, 0x20);

	/* IO Address Enable, CIM set 0x78 only and masked 0x79. */
	printk_info("IO Address Enable\n");
	/*pci_write_config8(dev, 0x79, 0x4F); */
	pci_write_config8(dev, 0x78, 0xFF);
//#ifndef ENABLE_SATA
#if 1
	/* TODO: set ide as primary, if you want to boot from IDE, you'd better set it.Or add a configuration line.*/
	printk_info("set ide as primary\n");
	byte = pci_read_config8(dev, 0xAD);
	byte |= 0x1<<3;
	byte &= ~(0x1<<4);
	pci_write_config8(dev, 0xAD, byte);

	/* This register is not used on sb700. It came from older chipset. */
	/*pci_write_config8(dev, 0x95, 0xFF); */
#endif	

	/* Set smbus iospace enable, I don't know why write 0x04 into reg5 that is reserved */
	printk_info("Set smbus iospace enable\n");
	pci_write_config16(dev, 0x4, 0x0407);
#if 1
	/* clear any lingering errors, so the transaction will run */
	printk_info("IO Address Enable\n");
	//OUTB(INB(0xba000000 + SMBUS_IO_BASE + SMBHSTSTAT), 0xba000000 + SMBUS_IO_BASE + SMBHSTSTAT);
	OUTB(INB(BONITO_PCIIO_BASE_VA + SMBUS_IO_BASE + SMBHSTSTAT), BONITO_PCIIO_BASE_VA + SMBUS_IO_BASE + SMBHSTSTAT);
#endif
	/* IDE Device, BDF:0-20-1 */
	printk_info("sb700_devices_por_init(): IDE Device, BDF:0-20-1\n");
	//dev = pci_locate_device(PCI_ID(0x1002, 0x438C), 0);
	
	dev = _pci_make_tag(0, 20, 1);
	/* Disable prefetch */
	printk_info("Disable prefetch\n");
	byte = pci_read_config8(dev, 0x63);
	byte |= 0x1;
	pci_write_config8(dev, 0x63, byte);

	/* LPC Device, BDF:0-20-3 */
	printk_info("sb700_devices_por_init(): LPC Device, BDF:0-20-3\n");
	//dev = pci_locate_device(PCI_ID(0x1002, 0x438D), 0);
	dev = _pci_make_tag(0, 20, 3);
	/* DMA enable */
	printk_info("DMA enable\n");
	pci_write_config8(dev, 0x40, 0x04);

	/* IO Port Decode Enable */
	printk_info("IO Port Decode Enable\n");
	pci_write_config8(dev, 0x44, 0xFF);
	pci_write_config8(dev, 0x45, 0xFF);
	pci_write_config8(dev, 0x46, 0xC3);
	pci_write_config8(dev, 0x47, 0xFF);

	/* IO/Mem Port Decode Enable, I don't know why CIM disable some ports.
	 *  Disable LPC TimeOut counter, enable SuperIO Configuration Port (2e/2f),
	 * Alternate SuperIO Configuration Port (4e/4f), Wide Generic IO Port (64/65).
	 * Enable bits for LPC ROM memory address range 1&2 for 1M ROM setting.*/
	printk_info("IO/Mem Port Decode Enable\n");
	byte = pci_read_config8(dev, 0x48);
	byte |= (1 << 1) | (1 << 0);	/* enable Super IO config port 2e-2h, 4e-4f */
	byte |= (1 << 3) | (1 << 4);	/* enable for LPC ROM address range1&2, Enable 512KB rom access at 0xFFF80000 - 0xFFFFFFFF */
	byte |= 1 << 6;		/* enable for RTC I/O range */
	pci_write_config8(dev, 0x48, byte);
	pci_write_config8(dev, 0x49, 0xFF);
	/* Enable 0x480-0x4bf, 0x4700-0x470B */
	byte = pci_read_config8(dev, 0x4A);
	byte |= ((1 << 1) + (1 << 6));	/*0x42, save the configuraion for port 0x80. */
	pci_write_config8(dev, 0x4A, byte);

	/* Set LPC ROM size, it has been done in sb700_lpc_init().
	 * enable LPC ROM range, 0xfff8: 512KB, 0xfff0: 1MB;
	 * enable LPC ROM range, 0xfff8: 512KB, 0xfff0: 1MB
	 * pci_write_config16(dev, 0x68, 0x000e)
	 * pci_write_config16(dev, 0x6c, 0xfff0);*/

	/* Enable Tpm12_en and Tpm_legacy. I don't know what is its usage and copied from CIM. */
	printk_info("Enable Tpm12_en and Tpm_legacy\n");
	pci_write_config8(dev, 0x7C, 0x05);

	/* P2P Bridge, BDF:0-20-4, the configuration of the registers in this dev are copied from CIM,
	 * TODO: I don't know what are their mean? */
	printk_info("sb700_devices_por_init(): P2P Bridge, BDF:0-20-4\n");
	//dev = pci_locate_device(PCI_ID(0x1002, 0x4384), 0);
  	dev = _pci_make_tag(0, 20, 4);
	/* I don't know why CIM tried to write into a read-only reg! */
	/*pci_write_config8(dev, 0x0c, 0x20) */ ;

	/* Arbiter enable. */
	printk_info("Arbiter enable\n");
	pci_write_config8(dev, 0x43, 0xff);

	/* Set PCDMA request into hight priority list. */
	/* pci_write_config8(dev, 0x49, 0x1) */ ;

	pci_write_config8(dev, 0x40, 0x26);

	/* I don't know why CIM set reg0x1c as 0x11.
	 * System will block at sdram_initialize() if I set it before call sdram_initialize().
	 * If it is necessary to set reg0x1c as 0x11, please call this function after sdram_initialize().
	 * pci_write_config8(dev, 0x1c, 0x11);
	 * pci_write_config8(dev, 0x1d, 0x11);*/

	/*CIM set this register; but I didn't find its description in RPR.
	On DBM690T platform, I didn't find different between set and skip this register.
	But on Filbert platform, the DEBUG message from serial port on Peanut board can't be displayed
	after the bit0 of this register is set.
	pci_write_config8(dev, 0x04, 0x21);
	*/
	printk_info("CIM set this register\n");
	pci_write_config8(dev, 0x0d, 0x40);
	pci_write_config8(dev, 0x1b, 0x40);
	/* Enable PCIB_DUAL_EN_UP will fix potential problem with PCI cards. */
	printk_info("enable pcib_dual_en_up\n");
	pci_write_config8(dev, 0x50, 0x01);
#ifdef ENABLE_SATA
	/* SATA Device, BDF:0-17-0, Non-Raid-5 SATA controller */
	printk_info("sb700_devices_por_init(): SATA Device, BDF:0-17-0\n");
	//dev = pci_locate_device(PCI_ID(0x1002, 0x4380), 0);
	dev = _pci_make_tag(0, 17, 0);
	/*PHY Global Control, we are using A14.
	 * default:  0x2c40 for ASIC revision A12 and below
	 *      0x2c00 for ASIC revision A13 and above.*/
	printk_info("PHY Global Control\n");
	pci_write_config16(dev, 0x86, 0x2C00);
#endif
}
static void pmio_write(u8 reg, u8 value)
{
	OUTB(reg, PM_INDEX);
	OUTB(value, PM_INDEX + 1);
}

static u8 pmio_read(u8 reg)
{
	OUTB(reg, PM_INDEX);
	return INB(PM_INDEX + 1);
}
void sb700_pmio_por_init(void)
{
	u8 byte;

#if 1
	printk_info("sb700_pmio_por_init()\n");
	/* K8KbRstEn, KB_RST# control for K8 system. */
	byte = pmio_read(0x66);
	byte |= 0x20;
	pmio_write(0x66, byte);

	/* RPR2.31 PM_TURN_OFF_MSG during ASF Shutdown. */
	if (get_sb700_revision() <= 0x12) {
		byte = pmio_read(0x65);
		byte &= ~(1 << 7);
		pmio_write(0x65, byte);

		byte = pmio_read(0x75);
		byte &= 0xc0;
		byte |= 0x05;
		pmio_write(0x75, byte);

		byte = pmio_read(0x52);
		byte &= 0xc0;
		byte |= 0x08;
		pmio_write(0x52, byte);
	} else {
		byte = pmio_read(0xD7);
		byte |= 1 << 0;
		pmio_write(0xD7, byte);

		byte = pmio_read(0x65);
		byte |= 1 << 7;
		pmio_write(0x65, byte);

		byte = pmio_read(0x75);
		byte &= 0xc0;
		byte |= 0x01;
		pmio_write(0x75, byte);

		byte = pmio_read(0x52);
		byte &= 0xc0;
		byte |= 0x02;
		pmio_write(0x52, byte);

	}	

	pmio_write(0x6c, 0xf0);
	pmio_write(0x6d, 0x00);
	pmio_write(0x6e, 0xc0);
	pmio_write(0x6f, 0xfe);

	/* rpr2.19: Enabling Spread Spectrum */
	printk_info("Enabling Spread Spectrum\n");
	byte = pmio_read(0x42);
	byte |= 1 << 7;
	pmio_write(0x42, byte);
	/* TODO: Check if it is necessary. IDE reset */
	byte = pmio_read(0xB2);
	byte |= 1 << 0;
	pmio_write(0xB2, byte);
#endif
}


/*
* Compliant with CIM_48's ATSBPowerOnResetInitJSP
*/
void sb700_por_init(void)
{
	/* sbDevicesPorInitTable + sbK8PorInitTable */
	sb700_devices_por_init();

	/* sbPmioPorInitTable + sbK8PmioPorInitTable */
	sb700_pmio_por_init();
}

void sb700_early_setup(void)
{
	sb700_por_init();
}

int optimize_link_incoherent_ht(void)
{

	return 0;
}

void soft_reset(void)
{

}

void rs780_before_pci_fixup(void)
{

}

/* Get SB ASIC Revision.*/
static u8 get_sb700_revision(void)
{
	device_t dev;
	//dev = pci_locate_device(PCI_ID(0x1002, 0x4385), 0);
	dev = _pci_make_tag(0, 20, 0);
	return pci_read_config8(dev, 0x08);
}


/*
* Compliant with CIM_48's sbPciCfg.
* Add any south bridge setting.
*/
static void sb700_pci_cfg()
{
	device_t dev;
	u8 byte;

	/* SMBus Device, BDF:0-20-0 */
	//dev = pci_locate_device(PCI_ID(0x1002, 0x4385), 0);
	dev = _pci_make_tag(0, 20, 0);

#if 1
	/* Enable watchdog decode timer */
	printk_info("enable watchdog decode timer\n");
	byte = pci_read_config8(dev, 0x41);
	byte |= (1 << 3);
	pci_write_config8(dev, 0x41, byte);
	/* Set to 1 to reset USB on the software (such as IO-64 or IO-CF9 cycles)
	 * generated PCIRST#. */
	byte = pmio_read(0x65);
	byte |= (1 << 4);
	pmio_write(0x65, byte);
#endif
	/* IDE Device, BDF:0-20-1 */
	//dev = pci_locate_device(PCI_ID(0x1002, 0x438C), 0);
	dev = _pci_make_tag(0, 20, 1);
	/* Enable IDE Explicit prefetch, 0x63[0] clear */
	printk_info("enable IDE explicit prefetch\n");
	byte = pci_read_config8(dev, 0x63);
	byte &= 0xfe;
	pci_write_config8(dev, 0x63, byte);

	/* LPC Device, BDF:0-20-3 */
	//dev = pci_locate_device(PCI_ID(0x1002, 0x438D), 0);
	dev = _pci_make_tag(0, 20, 3);
	/* rpr7.2 Enabling LPC DMA function. */
	printk_info("enabling lpc dma function\n");
	byte = pci_read_config8(dev, 0x40);
	byte |= (1 << 2);
	pci_write_config8(dev, 0x40, byte);
	/* rpr7.3 Disabling LPC TimeOut. 0x48[7] clear. */
	printk_info("disable lpc timeout\n");
	byte = pci_read_config8(dev, 0x48);
	byte &= 0x7f;
	pci_write_config8(dev, 0x48, byte);
	/* rpr7.5 Disabling LPC MSI Capability, 0x78[1] clear. */
	printk_info("disable LPC MSI Capability\n");
	byte = pci_read_config8(dev, 0x78);
	byte &= 0xfd;
	pci_write_config8(dev, 0x78, byte);
#ifdef ENABLE_SATA
	/* SATA Device, BDF:0-18-0, Non-Raid-5 SATA controller */
	//dev = pci_locate_device(PCI_ID(0x1002, 0x4380), 0);
	dev = _pci_make_tag(0, 17, 0);
	/* rpr7.12 SATA MSI and D3 Power State Capability.
	 * TODO: We assume S1 is supported. What if it isn't support? */
	byte = pci_read_config8(dev, 0x40);
	byte |= 1 << 0;
	pci_write_config8(dev, 0x40, byte);
	if (get_sb700_revision() <= 0x12)
		pci_write_config8(dev, 0x34, 0x70);
	else
		pci_write_config8(dev, 0x34, 0x50);
	byte &= ~(1 << 0);
	pci_write_config8(dev, 0x40, byte);
#endif
	/* TODO: There are several pairs of USB devices.
	 * Two 4396s, two 4397s, two 4398s.
	 * The code below only set one of each two. The other
	 * will be done in sb700_usb.c after all.
	 * So we don't take the trouble to set them both. */
	/* EHCI Device, BDF:0-19-2, ehci usb controller */
	//dev = pci_locate_device(PCI_ID(0x1002, 0x4396), 0);
	dev = _pci_make_tag(0, 19, 2);
	/* rpr6.16 Disabling USB EHCI MSI Capability. 0x50[6]. */
	byte = pci_read_config8(dev, 0x50);
	byte |= (1 << 6);
	pci_write_config8(dev, 0x50, byte);
	
	//lycheng add disabling usb ohci and ehci msi capability
	
	/* EHCI Device, BDF:0-18-2, ehci usb controller */
        //dev = pci_locate_device(PCI_ID(0x1002, 0x4396), 0);
        dev = _pci_make_tag(0, 18, 2);
        /* rpr6.16 Disabling USB EHCI MSI Capability. 0x50[6]. */
        byte = pci_read_config8(dev, 0x50);
        byte |= (1 << 6);
        pci_write_config8(dev, 0x50, byte);


	/* OHCI0 Device, BDF:0-19-0, ohci usb controller #0 */
	//dev = pci_locate_device(PCI_ID(0x1002, 0x4387), 0);
	dev = _pci_make_tag(0, 19, 0);
	/* rpr5.11 Disabling USB OHCI MSI Capability. 0x40[12:8]=0x1f. */
	printk_info("disable USB OHCI MSI Capability\n");
	byte = pci_read_config8(dev, 0x41);
	byte |= 0x3;
	pci_write_config8(dev, 0x41, byte);

	dev = _pci_make_tag(0, 18, 0);
        /* rpr5.11 Disabling USB OHCI MSI Capability. 0x40[12:8]=0x1f. */
        printk_info("disable USB OHCI MSI Capability\n");
        byte = pci_read_config8(dev, 0x41);
        byte |= 0x3;
        pci_write_config8(dev, 0x41, byte);


	/* OHCI0 Device, BDF:0-19-1, ohci usb controller #0 */
	//dev = pci_locate_device(PCI_ID(0x1002, 0x4398), 0);
	dev = _pci_make_tag(0, 19, 1);
	byte = pci_read_config8(dev, 0x41);
	byte |= 0x3;
	pci_write_config8(dev, 0x41, byte);

	dev = _pci_make_tag(0, 18, 1);
        byte = pci_read_config8(dev, 0x41);
        byte |= 0x3;
        pci_write_config8(dev, 0x41, byte);


	/* OHCI0 Device, BDF:0-20-5, ohci usb controller #0 */
	//dev = pci_locate_device(PCI_ID(0x1002, 0x4399), 0);
	dev = _pci_make_tag(0, 20, 5);
	byte = pci_read_config8(dev, 0x41);
	byte |= 0x3;
	pci_write_config8(dev, 0x41, byte);
}

void sb700_before_pci_fixup(void){
	sb700_pci_cfg();
}



/***********************************************
*	0:00.0  NBCFG	:
*	0:00.1  CLK	: bit 0 of nb_cfg 0x4c : 0 - disable, default
*	0:01.0  P2P Internal:
*	0:02.0  P2P	: bit 2 of nbmiscind 0x0c : 0 - enable, default	   + 32 * 2
*	0:03.0  P2P	: bit 3 of nbmiscind 0x0c : 0 - enable, default	   + 32 * 2
*	0:04.0  P2P	: bit 4 of nbmiscind 0x0c : 0 - enable, default	   + 32 * 2
*	0:05.0  P2P	: bit 5 of nbmiscind 0x0c : 0 - enable, default	   + 32 * 2
*	0:06.0  P2P	: bit 6 of nbmiscind 0x0c : 0 - enable, default	   + 32 * 2
*	0:07.0  P2P	: bit 7 of nbmiscind 0x0c : 0 - enable, default	   + 32 * 2
*	0:08.0  NB2SB	: bit 6 of nbmiscind 0x00 : 0 - disable, default   + 32 * 1
* case 0 will be called twice, one is by cpu in hypertransport.c line458,
* the other is by rs780.
***********************************************/

void rs780_enable(device_t dev)
{
        device_t nb_dev, sb_dev;
	int dev_ind;

        nb_dev = _pci_make_tag(0, 0, 0);
        sb_dev = _pci_make_tag(0, 8, 0);
	
	_pci_break_tag(dev, NULL, &dev_ind, NULL);

        switch(dev_ind){
        case 0:
        	printk_info("enable_pcie_bar3\n");
		enable_pcie_bar3(nb_dev);       /* PCIEMiscInit */
        	printk_info("config_gpp_core\n");
                config_gpp_core(nb_dev, sb_dev);
        	printk_info("rs780_gpp_sb_init\n");
                rs780_gpp_sb_init(nb_dev, sb_dev, 8);
                /* set SB payload size: 64byte */
        	printk_info("set sb payload size:64byte\n");
                set_pcie_enable_bits(nb_dev, 0x10 | PCIE_CORE_INDEX_GPPSB, 3 << 11, 2 << 11);

                /* Bus0Dev0Fun1Clock control init, we have to do it here, for dev0 Fun1 doesn't have a vendor or device ID */
                //rs780_config_misc_clk(nb_dev);
		{	/* BTDC: NBPOR_InitPOR function. */
			u8 temp8;
			u16 temp16;
			u32 temp32;
			
			/* BTDC: Program NB PCI table. */
        		printk_info("Program NB PCI table\n");
			temp16 = pci_read_config16(nb_dev, 0x04);
			printk_debug("BTDC: NB_PCI_REG04 = %x.\n", temp16);
			temp32 = pci_read_config32(nb_dev, 0x84);
			printk_debug("BTDC: NB_PCI_REG84 = %x.\n", temp32);
	
			pci_write_config8(nb_dev, 0x4c, 0x42);
			temp8 = pci_read_config8(nb_dev, 0x4e);
			temp8 |= 0x05;
			pci_write_config8(nb_dev, 0x4e, temp8);
			temp32 = pci_read_config32(nb_dev, 0x4c);
			printk_debug("BTDC: NB_PCI_REG4C = %x.\n", temp32);
			/* BTDC: disable GFX debug. */
        		printk_info("disable gfx debug\n");
			temp8 = pci_read_config8(nb_dev, 0x8d);
			temp8 &= ~(1<<1);
			pci_write_config8(nb_dev, 0x8d, temp8);
			/* BTDC: set temporary NB TOM to 0x40000000. */
        		//printk_info("set temporary NB TOM to 0xf0000000\n");
			//pci_write_config32(nb_dev, 0x90, 0x40000000);
			//pci_write_config32(nb_dev, 0x90, 0xf0000000);
        		printk_info("set temporary NB TOM to 0xffffffff\n");
			pci_write_config32(nb_dev, 0x90, 0xffffffff);
			/* BTDC: Program NB HTIU table. */
        		printk_info("Program NB HTIU table\n");
			set_htiu_enable_bits(nb_dev, 0x05, 1<<10 | 1<<9, 1<<10|1<<9);
			set_htiu_enable_bits(nb_dev, 0x06, 1, 0x4203a202);
			set_htiu_enable_bits(nb_dev, 0x07, 1<<1 | 1<<2, 0x8001);
			set_htiu_enable_bits(nb_dev, 0x15, 0, 1<<31 | 1<<30 | 1<<27);
			set_htiu_enable_bits(nb_dev, 0x1c, 0, 0xfffe0000);
			set_htiu_enable_bits(nb_dev, 0x4b, 1<<11, 1<<11);
			set_htiu_enable_bits(nb_dev, 0x0c, 0x3f, 1 | 1<<3);
			set_htiu_enable_bits(nb_dev, 0x17, 1<<1 | 1<<27, 1<<1);
			set_htiu_enable_bits(nb_dev, 0x17, 0, 1<<30);
			set_htiu_enable_bits(nb_dev, 0x19, 0xfffff+(1<<31), 0x186a0+(1<<31));
			set_htiu_enable_bits(nb_dev, 0x16, 0x3f<<10, 0x7<<10);
			set_htiu_enable_bits(nb_dev, 0x23, 0, 1<<28);
			/* BTDC: Program NB MISC table. */
        		printk_info("set NB MISC table\n");
			set_nbmisc_enable_bits(nb_dev, 0x0b, 0xffff, 0x00000180);
			set_nbmisc_enable_bits(nb_dev, 0x00, 0xffff, 0x00000106);
			set_nbmisc_enable_bits(nb_dev, 0x51, 0xffffffff, 0x00100100);
			set_nbmisc_enable_bits(nb_dev, 0x53, 0xffffffff, 0x00100100);
			set_nbmisc_enable_bits(nb_dev, 0x55, 0xffffffff, 0x00100100);
			set_nbmisc_enable_bits(nb_dev, 0x57, 0xffffffff, 0x00100100);
			set_nbmisc_enable_bits(nb_dev, 0x59, 0xffffffff, 0x00100100);
			set_nbmisc_enable_bits(nb_dev, 0x5b, 0xffffffff, 0x00100100);
			set_nbmisc_enable_bits(nb_dev, 0x5d, 0xffffffff, 0x00100100);
			set_nbmisc_enable_bits(nb_dev, 0x5f, 0xffffffff, 0x00100100);
			set_nbmisc_enable_bits(nb_dev, 0x20, 1<<1, 0);
			set_nbmisc_enable_bits(nb_dev, 0x37, 1<<11|1<<12|1<<13|1<<26, 0);
			set_nbmisc_enable_bits(nb_dev, 0x68, 1<<5|1<<6, 1<<5);
			set_nbmisc_enable_bits(nb_dev, 0x6b, 1<<22, 1<<10);
			set_nbmisc_enable_bits(nb_dev, 0x67, 1<<26, 1<<14|1<<10);
			set_nbmisc_enable_bits(nb_dev, 0x24, 1<<28|1<<26|1<<25|1<<16, 1<<29|1<<25);
			set_nbmisc_enable_bits(nb_dev, 0x38, 1<<24|1<<25, 1<<24);
			set_nbmisc_enable_bits(nb_dev, 0x36, 1<<29, 1<<29|1<<28);
			set_nbmisc_enable_bits(nb_dev, 0x0c, 0, 1<<13);
			set_nbmisc_enable_bits(nb_dev, 0x34, 1<<22, 1<<10);
			set_nbmisc_enable_bits(nb_dev, 0x39, 1<<10, 1<<30);
			set_nbmisc_enable_bits(nb_dev, 0x22, 1<<3, 0);
			set_nbmisc_enable_bits(nb_dev, 0x68, 1<<19, 0);
			set_nbmisc_enable_bits(nb_dev, 0x24, 1<<16|1<<17, 1<<17);
			set_nbmisc_enable_bits(nb_dev, 0x6a, 1<<22|1<<23, 1<<17|1<<23);
			set_nbmisc_enable_bits(nb_dev, 0x35, 1<<21|1<<22, 1<<22);
			set_nbmisc_enable_bits(nb_dev, 0x01, 0xffffffff, 0x48);
			/* BTDC: the last two step. */
			set_nbmisc_enable_bits(nb_dev, 0x01, 1<<8, 1<<8);
			set_htiu_enable_bits(nb_dev, 0x2d, 1<<6|1<<4, 1<<6|1<<4);
		}
                break;
	case 1: /* bus0, dev1, APC. */
		printk_info("Bus-0, Dev-1, Fun-0.\n");
        rs780_internal_gfx_enable(nb_dev,dev);
		break;
        case 2:
	case 3:
		set_nbmisc_enable_bits(nb_dev, 0x0c, 1 << dev_ind,
				       (1 ? 0 : 1) << dev_ind);
			rs780_gfx_init(nb_dev, dev, dev_ind);
		break;
	case 4:		/* bus0, dev4-7, four GPP */
	case 5:
	case 6:
	case 7:
		enable_pcie_bar3(nb_dev);	/* PCIEMiscInit */
		set_nbmisc_enable_bits(nb_dev, 0x0c, 1 << dev_ind,
				       (1 ? 0 : 1) << dev_ind);
			rs780_gpp_sb_init(nb_dev, dev, dev_ind);
		break;
	case 8:		/* bus0, dev8, SB */
		set_nbmisc_enable_bits(nb_dev, 0x00, 1 << 6,
				       (1 ? 0 : 1) << dev_ind);
			rs780_gpp_sb_init(nb_dev, dev, dev_ind);
		disable_pcie_bar3(nb_dev);
		break;
	case 9:		/* bus 0, dev 9,10, GPP */
	case 10:
		enable_pcie_bar3(nb_dev);	/* PCIEMiscInit */
		set_nbmisc_enable_bits(nb_dev, 0x0c, 1 << (7 + dev_ind),
				       (1 ? 0 : 1) << (7 + dev_ind));
			rs780_gpp_sb_init(nb_dev, dev, dev_ind);
		break;
	default:
		printk_debug("unknown dev: %s\n", dev_ind);
	}
}

void rs780_after_pci_fixup(void){
 	device_t dev;
	u32 reg32;
	/* bus0, dev0, fun0; */	
        dev = _pci_make_tag(0, 0, 0);
	rs780_enable(dev);

#if 1
	/* bus0, dev1, APC. */	
	printk_info("Bus-0, Dev-1, Fun-0.\n");
        dev = _pci_make_tag(0, 1, 0);
    	rs780_internal_gfx_init(_pci_make_tag(0,0,0) , _pci_make_tag(0,1,0));
	rs780_enable(dev);

	/* bus0, dev2,3, two GFX */
	printk_info("Bus-0, Dev-2, Fun-0\n");
        dev = _pci_make_tag(0, 2, 0);
	rs780_enable(dev);
	printk_info("Bus-0, Dev-3, Fun-0\n");
        dev = _pci_make_tag(0, 3, 0);
	set_nbmisc_enable_bits(_pci_make_tag(0, 0, 0), 0x0c, 1 << 3,0 << 3);
	rs780_gfx_3_init(_pci_make_tag(0, 0, 0), dev, 3); 

	/* bus0, dev4-7, four GPPSB */	
	printk_info("Bus-0, Dev-4, Fun-0\n");
        dev = _pci_make_tag(0, 4, 0);
	rs780_enable(dev);
	printk_info("Bus-0, Dev-5, Fun-0\n");
        dev = _pci_make_tag(0, 5, 0);
	rs780_enable(dev);

	printk_info("Bus-0, Dev-6, Fun-0\n");
        dev = _pci_make_tag(0, 6, 0);
	rs780_enable(dev);
	printk_info("Bus-0, Dev-7, Fun-0\n");
        dev = _pci_make_tag(0, 7, 0);
	rs780_enable(dev);

	/* bus 0, dev 9,10, GPP */
	printk_info("Bus-0, Dev-9, Fun-0\n");
        dev = _pci_make_tag(0, 9, 0);
	rs780_enable(dev);

	printk_info("Bus-0, Dev-10, Fun-0\n");
        dev = _pci_make_tag(0, 10, 0);
	rs780_enable(dev);

	/* bus0, dev8, SB */
	printk_info("Bus-0, Dev-8, Fun-0\n");
        dev = _pci_make_tag(0, 8, 0);
	rs780_enable(dev);

#endif	
}

/************
*       0:11.0  SATA  
*       0:12.0  OHCI0-USB1   
*       0:12.1  OHCI1-USB1  
*       0:12.2  EHCI-USB1  
*       0:13.0  OHCI0-USB2 
*       0:13.1  OHCI1-USB2  
*       0:13.2  EHCI-USB2  
*       0:14.5  OHCI0-USB3  
*       0:14.0  SMBUS                                                   0
*       0:14.1  IDE                                                     1
*       0:14.2  HDA   
*       0:14.3  LPC  
*       0:14.4  PCI 
*************/

void sb700_after_pci_fixup(void){
#ifdef ENABLE_SATA
	printk_info("sata init\n");
	sata_init(_pci_make_tag(0, 0x11, 0));
#endif
	printk_info("OHCI0-USB1 init\n");
	usb_init(_pci_make_tag(0, 0x12, 0));

	printk_info("OHCI1-USB1 init\n");
	usb_init(_pci_make_tag(0, 0x12, 1));
#if  1
	//printk_info("EHCI-USB1 init\n");
	usb_init2(_pci_make_tag(0, 0x12, 2));
	printk_info("OHCI0-USB2 init\n");
	usb_init(_pci_make_tag(0, 0x13, 0));
	printk_info("OHCI1-USB2 init\n");
	usb_init(_pci_make_tag(0, 0x13, 1));
	//printk_info("EHCI-USB2 init\n");
	usb_init2(_pci_make_tag(0, 0x13, 2));
	printk_info("OHCI0-USB3 init\n");
	usb_init(_pci_make_tag(0, 0x14, 5));
#endif
	printk_info("lpc init\n");
	lpc_init(_pci_make_tag(0, 0x14, 3));
	printk_info("ide init\n");
	ide_init(_pci_make_tag(0, 0x14, 1));
	//vga test
	printk_info("pci init\n");
	pci_init(_pci_make_tag(0, 0x14, 4));
	printk_info("sm init\n");
	sm_init(_pci_make_tag(0, 0x14, 0));
#ifdef USE_780E_VGA
	printk_info("rs780_internal_gfx_init\n");
	internal_gfx_pci_dev_init(_pci_make_tag(0,0,0) , _pci_make_tag(1,0x5,0));
#endif
}
