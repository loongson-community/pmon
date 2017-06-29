#include <unistd.h>
#include "rs780.h"
#include "rs780_chip.h"

/*------------------------------------------------
* Global variable
------------------------------------------------*/
PCIE_CFG AtiPcieCfg = {
	PCIE_ENABLE_STATIC_DEV_REMAP,	/* Config */
	0,			/* ResetReleaseDelay */
	0,			/* Gfx0Width */
	0,			/* Gfx1Width */
	0,			/* GfxPayload */
	0,			/* GppPayload */
	0,			/* PortDetect, filled by GppSbInit */
	0,			/* PortHp */
	0,			/* DbgConfig */
	0,			/* DbgConfig2 */
	0,			/* GfxLx */
	0,			/* GppLx */
	0,			/* NBSBLx */
	0,			/* PortSlotInit */
	0,			/* Gfx0Pwr */
	0,			/* Gfx1Pwr */
	0			/* GppPwr */
};
//get from mainboard\amd\mahogany\Config.lb
struct southbridge_amd_rs780_config chip_info = {
	0xfff0000,
	1,
	3,
	0x6fc,
	1,
	1,
	0,
	0,
	0,
	1,
	0
};

static void PciePowerOffGppPorts(device_t nb_dev, device_t dev, u32 port);
static void ValidatePortEn(device_t nb_dev);

static void ValidatePortEn(device_t nb_dev)
{
}

#define	PCIE_OFF_UNUSED_GPP_LANES		(1 << 5)
#define	PCIE_DISABLE_HIDE_UNUSED_PORTS	(1 << 7)
#define	PCIE_GFX_COMPLIANCE			(1 << 14)


/*****************************************************************
* Compliant with CIM_33's PCIEPowerOffGppPorts
* Power off unused GPP lines
*****************************************************************/
static void PciePowerOffGppPorts(device_t nb_dev, device_t dev, u32 port)
{
	u32 reg;
	u16 state_save;
	struct southbridge_amd_rs780_config *cfg =
	//    (struct southbridge_amd_rs690_config *)nb_dev->chip_info;
		&chip_info;
	u16 state = cfg->port_enable;

	if (!(AtiPcieCfg.Config & PCIE_DISABLE_HIDE_UNUSED_PORTS))
		state &= AtiPcieCfg.PortDetect;
	state = ~state;
	state &= (1 << 4) + (1 << 5) + (1 << 6) + (1 << 7);
	state_save = state << 17;
	state &= !(AtiPcieCfg.PortHp);
	reg = nbmisc_read_index(nb_dev, 0x0c);
	reg |= state;
	nbmisc_write_index(nb_dev, 0x0c, reg);

	reg = nbmisc_read_index(nb_dev, 0x08);
	reg |= state_save;
	nbmisc_write_index(nb_dev, 0x08, reg);

	if ((AtiPcieCfg.Config & PCIE_OFF_UNUSED_GPP_LANES)
	    && !(AtiPcieCfg.
		 Config & (PCIE_DISABLE_HIDE_UNUSED_PORTS +
			   PCIE_GFX_COMPLIANCE))) {
	}
	/* step 3 Power Down Control for Southbridge */
	printk_info("Power Down Control for Southbridge\n");
	reg = nbpcie_p_read_index(dev, 0xa2);

	switch ((reg >> 4) & 0x7) {	/* get bit 4-6, LC_LINK_WIDTH_RD */
	case 1:
		nbpcie_ind_write_index(nb_dev, 0x65, 0x0e0e);
		break;
	case 2:
		nbpcie_ind_write_index(nb_dev, 0x65, 0x0c0c);
		break;
	default:
		break;
	}
}

/**********************************************************************
**********************************************************************/
static void switching_gppsb_configurations(device_t nb_dev, device_t sb_dev)
{
	u32 reg;
	struct southbridge_amd_rs780_config *cfg =
	    &chip_info;
	/* 5.5.7.1-3 enables GPP reconfiguration */
	printk_info("enable GPP reconfiguration\n");
	reg = nbmisc_read_index(nb_dev, PCIE_NBCFG_REG7);
	reg |=
	    (RECONFIG_GPPSB_EN + RECONFIG_GPPSB_LINK_CONFIG +
	     RECONFIG_GPPSB_ATOMIC_RESET);
	nbmisc_write_index(nb_dev, PCIE_NBCFG_REG7, reg);
	printk_info("De-asserts STRAP)BIF_all_valid for PCIE-GPPSB core\n");
	/* 5.5.7.4a. De-asserts STRAP_BIF_all_valid for PCIE-GPPSB core */
	reg = nbmisc_read_index(nb_dev, 0x66);
	reg |= 1 << 31;
	nbmisc_write_index(nb_dev, 0x66, reg);
	printk_info("sets desired GPPSB configurations\n");
	/* 5.5.7.4b. sets desired GPPSB configurations, bit4-7 */
	reg = nbmisc_read_index(nb_dev, 0x67);
	reg &= 0xFFFFff0f;		/* clean */
	reg |= cfg->gppsb_configuration << 4;
	nbmisc_write_index(nb_dev, 0x67, reg);
#if 1
	/* NOTE:
	 * In CIMx 4.5.0 and RPR, 4c is done before 5 & 6. But in this way,
	 * a x4 device in port B (dev 4) of Configuration B can only be detected
	 * as x1, instead of x4. When the port B is being trained, the
	 * LC_CURRENT_STATE is 6 and the LC_LINK_WIDTH_RD is 1. We have
	 * to set the PCIEIND:0x65 as 0xE0E0 and reset the slot. Then the card
	 * seems to work in x1 mode.
	 * In the 2nd way below, we do the 5 & 6 before 4c. it conforms the
	 * CIMx 4.3.0. It conflicts with RPR. But based on the test result I've
	 * made so far, I haven't found any mistake. A coworker from Shanghai
	 * said it can not detect the x1 card when it first boots. I haven't got
	 * the result like this.
	 * The x4 device can work in both way, dont worry about it. We are just waiting
	 * for the chipset team to solve this problem.
	 */
	/* 5.5.7.4c. Asserts STRAP_BIF_all_valid for PCIE-GPPSB core */
	printk_info("Asserts STRAP_BIF_all_valid for PCIE-GPPSB core\n");
	reg = nbmisc_read_index(nb_dev, 0x66);
	reg &= ~(1 << 31);
	nbmisc_write_index(nb_dev, 0x66, reg);
	/* 5.5.7.5-6. read bit14 and write back its inverst value */
	printk_info("read bit14 and write back its inverst value\n");
	reg = nbmisc_read_index(nb_dev, PCIE_NBCFG_REG7);
	reg ^= RECONFIG_GPPSB_GPPSB;
	nbmisc_write_index(nb_dev, PCIE_NBCFG_REG7, reg);
#else
	/* 5.5.7.5-6. read bit14 and write back its inverst value */
	reg = nbmisc_read_index(nb_dev, PCIE_NBCFG_REG7);
	reg ^= RECONFIG_GPPSB_GPPSB;
	nbmisc_write_index(nb_dev, PCIE_NBCFG_REG7, reg);
	/* 5.5.7.4c. Asserts STRAP_BIF_all_valid for PCIE-GPPSB core */
	reg = nbmisc_read_index(nb_dev, 0x66);
	reg &= ~(1 << 31);
	nbmisc_write_index(nb_dev, 0x66, reg);
#endif
	/* 5.5.7.7. delay 1ms */
	delay(1);
	/* 5.5.7.8. waits until SB has trained to L0, poll for bit0-5 = 0x10 */
	printk_info("waits until SB has trained to L0, poll for bit0-5 = 0x10\n");
	do {
		reg = nbpcie_p_read_index(sb_dev, PCIE_LC_STATE0);
		reg &= 0x3f;	/* remain LSB [5:0] bits */
	} while (LC_STATE_RECONFIG_GPPSB != reg);
#if 0
	/* 5.5.7.9.ensures that virtual channel negotiation is completed. poll for bit1 = 0 */
	printk_info("ensures that virtual channel negotiation is completed. poll for bit1 = 0\n");
	do {
		reg =
		    pci_ext_read_config32(nb_dev, sb_dev,
					  PCIE_VC0_RESOURCE_STATUS);
	} while (reg & VC_NEGOTIATION_PENDING);
#endif

#if 1
	/* 5.5.7.9.ensures that virtual channel negotiation is completed. poll for bit1 = 0 */
	printk_info("ensures that virtual channel negotiation is completed. poll for bit1 = 0\n");
	do {
		reg =
		    pci_ext_read_config16(nb_dev, sb_dev,
					  PCIE_VC0_RESOURCE_STATUS);
	} while (reg & VC_NEGOTIATION_PENDING);
#endif

}



/**********************************************************************
**********************************************************************/
static void switching_gpp_configurations(device_t nb_dev, device_t sb_dev)
{
	u32 reg;
	struct southbridge_amd_rs780_config *cfg =
	    //(struct southbridge_amd_rs690_config *)nb_dev->chip_info;
	    &chip_info;
	/* 5.6.2.1. De-asserts STRAP_BIF_all_valid for PCIE-GPP core */
	printk("De-asserts STRAP_BIF_all_valid for PCIE-GPP core\n");
	reg = nbmisc_read_index(nb_dev, 0x22);
	reg |= 1 << 14;
	nbmisc_write_index(nb_dev, 0x22, reg);
	printk("sets desired GPPSB configurations, bit4-7\n");
	/* 5.6.2.2. sets desired GPPSB configurations, bit4-7 */
	reg = nbmisc_read_index(nb_dev, 0x2D);
	reg &= ~(0xF << 7);		/* clean */
	reg |= cfg->gpp_configuration << 7;
	nbmisc_write_index(nb_dev, 0x2D, reg);
	printk("Asserts STRAP_BIF_all_valid for PCIE-GPP core\n");
	/* 5.6.2.3. Asserts STRAP_BIF_all_valid for PCIE-GPP core */
	reg = nbmisc_read_index(nb_dev, 0x22);
	reg &= ~(1 << 14);
	nbmisc_write_index(nb_dev, 0x22, reg);
}




/*****************************************************************
* The rs690 uses NBCONFIG:0x1c (BAR3) to map the PCIE Extended Configuration
* Space to a 256MB range within the first 4GB of addressable memory.
*****************************************************************/
void enable_pcie_bar3(device_t nb_dev)
{
/*
43291_rs780_rpr_nda_1.05.pdf
5.9.8
Program PCIE Memory Mapped Configuration Space
43734_rs780_bdg_nda_1.06.pdf
2.3
HTIU Indirect Register Space (HTIUIND)
*/
	set_nbcfg_enable_bits(nb_dev, 0x7C, 1 << 30, 1 << 30);	/* Enables writes to the BAR3 register. */
	set_nbcfg_enable_bits(nb_dev, 0x84, 7 << 16, 0 << 16);

	pci_write_config32(nb_dev, 0x1C, EXT_CONF_BASE_ADDRESS);	/* PCIEMiscInit */
	pci_write_config32(nb_dev, 0x20, 0x00000000);
	//set_htiu_enable_bits(nb_dev, 0x32, 1 << 28, 1 << 28);	/* PCIEMiscInit */
	htiu_write_indexN(nb_dev, 0x32, htiu_read_indexN(nb_dev, 0x32) | (1<<28));
}

/*****************************************************************
* We should disable bar3 when we want to exit rs690_enable, because bar3 will be
* remapped in set_resource later.
*****************************************************************/
void disable_pcie_bar3(device_t nb_dev)
{
	printk_info("clear BAR3 address\n");
	pci_write_config32(nb_dev, 0x1C, 0);	/* clear BAR3 address */
	printk_info("Disable writes to the BAR3\n");
	set_nbcfg_enable_bits(nb_dev, 0x7C, 1 << 30, 0 << 30);	/* Disable writes to the BAR3. */
	//set_htiu_enable_bits(nb_dev, 0x32, 1 << 28, 0 << 28);	/* PCIEMiscInit */
	htiu_write_indexN(nb_dev, 0x32, htiu_read_indexN(nb_dev, 0x32) & ~(1<<28));
}

/*****************************************
* Compliant with CIM_33's PCIEGPPInit
* nb_dev:
*	root bridge struct
* dev:
*	p2p bridge struct
* port:
*	p2p bridge number, 4-8
*****************************************/
void rs780_gpp_sb_init(device_t nb_dev, device_t dev, u32 port)
{
	u8 reg8;
	u16 reg16;
	device_t sb_dev;
	//add for rs780
	u32 gfx_gpp_sb_sel;

	struct southbridge_amd_rs780_config *cfg =
	    //(struct southbridge_amd_rs690_config *)nb_dev->chip_info;
		&chip_info;
	//add for rs780
	gfx_gpp_sb_sel = port >= 4 && port <= 8 ? 
					PCIE_CORE_INDEX_GPPSB : /* 4,5,6,7,8 */
					PCIE_CORE_INDEX_GPP;    /* 9,10 */
	/* init GPP core */
	printk_info("init GPP core\n");
	printk_info("Disable slave ordering logic\n");
	set_pcie_enable_bits(nb_dev, 0x20 | gfx_gpp_sb_sel, 1 << 8,
			     1 << 8);
	/* PCIE initialization 5.10.2: rpr 2.12*/
	printk_info("PCIE initialization\n");
	set_pcie_enable_bits(nb_dev, 0x02 | gfx_gpp_sb_sel, 1 << 0, 1 << 0);	/* no description in datasheet. */
	
	/* init GPPSB port. rpr 5.10.8 */
	/* 5.10.8.1-5.10.8.2. Sets RCB timeout to be 100ms/4=25ms by setting bits[18:16] to 3 h4
	 * and shortens the enumeration timer by setting bit[19] to 1
	 */
	printk_info("init GPPSB port.\n");
	set_pcie_enable_bits(dev, 0x70, 0xF << 16, 0x4 << 16 | 1 << 19);
	/* 5.10.8.4. Sets DMA payload size to 64 bytes. */
	 printk_info("Sets DMA payload size to 64 bytes\n");
	set_pcie_enable_bits(nb_dev, 0x10 | gfx_gpp_sb_sel, 7 << 10, 4 << 10);
	/* 5.10.8.6. Disable RC ordering logic */
	printk_info("Disable RC ordering logic\n");
	set_pcie_enable_bits(nb_dev, 0x20 | gfx_gpp_sb_sel, 1 << 9, 1 << 9);
	/* 5.10.8.7. Ignores DLLs druing L1 */
 	printk_info("Ignores DLLs druing L1\n");	
	set_pcie_enable_bits(nb_dev, 0x02 | gfx_gpp_sb_sel, 1 << 0, 1 << 0);
	/* 5.10.8.8. Prevents LCto go from L0 to Rcv_L0s if L1 is armed. */
	printk_info("Prevents LCto go from L0 to Rcv_L0s if L1 is armed\n");
	set_pcie_enable_bits(dev, 0xA1, 1 << 11, 1 << 11);
	/* 5.10.8.9. Sets timer in Config state from 20us to 1us.
	 * 5.10.8.10. De-asserts RX_EN in L0s
	 * 5.10.8.11. Enables de-assertion of PG2RX_CR_EN to lock clock recovery parameter when .. */
	printk_info("Sets timer in Config state from 20us to 1us\n");
	set_pcie_enable_bits(dev, 0xB1, 1 << 23 | 1 << 19 | 1 << 28, 1 <<23 | 1 << 19 | 1 << 28);
	/* 5.10.8.12. Turns off offset calibration */
	/* 5.10.8.13. Enables Rx  Clock gating in CDR */
	printk_info("Turns off offset calibration\n");
	if (gfx_gpp_sb_sel == PCIE_CORE_INDEX_GPPSB)
		set_nbmisc_enable_bits(nb_dev, 0x67, 1 << 14 | 1 << 26, 1 << 14 | 1 << 26); /* 4,5,6,7 */
	else
		set_nbmisc_enable_bits(nb_dev, 0x24, 1 << 29 | 1 << 28, 1 << 29 | 1 << 28); /* 9,10 */
	/* 5.10.8.14. Sets number of TX Clocks to drain TX Pipe to 3 */
	printk_info("Sets number of TX Clocks to drain TX Pipe to 3\n");
	set_pcie_enable_bits(dev, 0xA0, 0xF << 4, 0x3 << 4);
	/* 5.10.8.15. empty */
	/* 5.10.8.16. P_ELEC_IDLE_MODE */
	printk_info("empty\n");
	set_pcie_enable_bits(nb_dev, 0x40 | gfx_gpp_sb_sel, 0x3 << 14, 0x2 << 14);
	/* 5.10.8.17. LC_BLOCK_EL_IDLE_IN_L0 */
	printk_info("LC_BLOCK_EL_IDLE_IN_L0\n");
	set_pcie_enable_bits(dev, 0xB1, 1 << 20, 1 << 20);
	/* 5.10.8.18. LC_DONT_GO_TO_L0S_IFL1_ARMED */
	printk_info("LC_DONT_GO_TO_L0S_IFL1_ARMED\n");
	set_pcie_enable_bits(dev, 0xA1, 1 << 11, 1 << 11);
	/* 5.10.8.19. RXP_REALIGN_ON_EACH_TSX_OR_SKP */
	printk_info("RXP_REALIGN_ON_EACH_TSX_OR_SKP\n");
	set_pcie_enable_bits(nb_dev, 0x40 | gfx_gpp_sb_sel, 1 << 28, 0 << 28);
	/* 5.10.8.20. Bypass lane de-skew logic if in x1 */
	printk_info("Bypass lane de-skew logic if in x1\n");
	set_pcie_enable_bits(nb_dev, 0xC2 | gfx_gpp_sb_sel, 1 << 14, 1 << 14);
	/* 5.10.8.21. sets electrical idle threshold. */
	printk_info("sets electrical idle threshold\n");
	if (gfx_gpp_sb_sel == PCIE_CORE_INDEX_GPPSB)
		set_nbmisc_enable_bits(nb_dev, 0x6A, 3 << 22, 2 << 22);
	else
		set_nbmisc_enable_bits(nb_dev, 0x24, 3 << 16, 2 << 16);
	/* 5.10.8.22. Disable GEN2 */
	/* TODO: should be 2 seperated cases. */
	printk_info("Disable GEN2\n");
	set_nbmisc_enable_bits(nb_dev, 0x39, 1 << 31, 0 << 31);
	set_nbmisc_enable_bits(nb_dev, 0x22, 1 << 5, 0 << 5);
	set_nbmisc_enable_bits(nb_dev, 0x34, 1 << 31, 0 << 31);
	set_nbmisc_enable_bits(nb_dev, 0x37, 7 << 5, 0 << 5);
	/* 5.10.8.23. Disables GEN2 capability of the device. RPR says enable? No! */
	printk_info("Disables GEN2 capability of the device\n");
	set_pcie_enable_bits(dev, 0xA4, 1 << 0, 0 << 0);
	/* 5.10.8.24. Disable advertising upconfigure support. */
	printk_info("Disable advertising upconfigure support\n");
	set_pcie_enable_bits(dev, 0xA2, 1 << 13, 1 << 13);
	/* 5.10.8.25-26. STRAP_BIF_DSN_EN */
	printk_info("STRAP_BIF_DSN_EN\n");
	if (gfx_gpp_sb_sel == PCIE_CORE_INDEX_GPPSB)
		set_nbmisc_enable_bits(nb_dev, 0x68, 1 << 19, 0 << 19);
	else
		set_nbmisc_enable_bits(nb_dev, 0x22, 1 << 3, 0 << 3);
	/* 5.10.8.27-28. */
	printk_info(" 5.10.8.27-28\n");
	set_pcie_enable_bits(nb_dev, 0xC1 | gfx_gpp_sb_sel, 1 << 0 | 1 << 2, 1 << 0 | 0 << 2);
	/* 5.10.8.29. Uses the bif_core de-emphasis strength by default. */
	printk_info("Uses the bif_core de-emphasis strength by default\n");
	if (gfx_gpp_sb_sel == PCIE_CORE_INDEX_GPPSB) {
		set_nbmisc_enable_bits(nb_dev, 0x67, 1 << 10, 1 << 10);
		set_nbmisc_enable_bits(nb_dev, 0x36, 1 << 29, 1 << 29);
	}
	else {
		set_nbmisc_enable_bits(nb_dev, 0x39, 1 << 30, 1 << 30);
	}
	/* 5.10.8.30. Set TX arbitration algorithm to round robin. */
	printk_info("Set TX arbitration algorithm to round robin\n");
	set_pcie_enable_bits(nb_dev, 0x1C | gfx_gpp_sb_sel,
			     1 << 0 | 0x1F << 1 | 0x1F << 6,
			     1 << 0 | 0x04 << 1 | 0x04 << 6);
	/* check compliance rpr step 2.1*/
	printk_info("check compliance\n");
	if (AtiPcieCfg.Config & PCIE_GPP_COMPLIANCE) {
		u32 tmp;
		tmp = nbmisc_read_index(nb_dev, 0x67);
		tmp |= 1 << 3;
		nbmisc_write_index(nb_dev, 0x67, tmp);
	}

	/* step 5: dynamic slave CPL buffer allocation. Disable it, otherwise linux hangs. Why? */
	/* set_pcie_enable_bits(nb_dev, 0x20 | gfx_gpp_sb_sel, 1 << 11, 1 << 11); */
	/* step 5a: Training for GPP devices */
	/* init GPP */
	switch (port) {
	case 4:		/* GPP */
	case 5:
	case 6:
	case 7:
	case 9:
	case 10:
		/*
		* 5.5.2.5.12 Enable the GPP training bit.
		*
		* 3A_server mainboard used one port gpp x4.
		*
		* Note: When I only enable bit 21, the lc_state after four bit is 0x8,
		* but if all enabled, the lc_state is 0x10. I don't know why.
		* Do you guys know?
		*/
		set_nbmisc_enable_bits(nb_dev, 0x8, 0x1 << 21 | 0x1 << 22 | 0x1 << 23 |
				0x1 << 24, 0x0 << 21 | 0x0 << 22 | 0x0 << 23 | 0x0 << 24);

		/* 5.6.1.5.18 Enable GPP configureation D training bit. */
		set_nbmisc_enable_bits(nb_dev, 0x8, 0x1 << 25 | 0x1 << 26, 0x0 << 25 |
				0x0 << 26);
		set_nbmisc_enable_bits(nb_dev, 0x2d, 0x1 << 4 | 0x1 << 5, 0x0 << 4 |
				0x0 << 5);

		/* 5.10.8.5. Blocks DMA traffic during C3 state */
		printk_info("Blocks DMA traffic during C3 state\n");
		set_pcie_enable_bits(dev, 0x10, 1 << 0, 0 << 0);
		/* Enabels TLP flushing */
		printk_info("Enabels TLP flushing\n");
		set_pcie_enable_bits(dev, 0x20, 1 << 19, 0 << 19);
		/* check port enable */
		printk_info("check port enable\n");
		if (cfg->port_enable & (1 << port)) {
                //  if(1){
			PcieReleasePortTraining(nb_dev, dev, port);
			if (!(AtiPcieCfg.Config & PCIE_GPP_COMPLIANCE)) {
				u8 res = PcieTrainPort(nb_dev, dev, port);
				printk_debug("PcieTrainPort port=0x%x result=%d\n", port, res);
				if (res) {
					AtiPcieCfg.PortDetect |= 1 << port;
				}
			}
		}
		break;
	case 8:		/* SB */
		break;
	}
	PciePowerOffGppPorts(nb_dev, dev, port);
	/* step 5b: GFX devices in a GPP slot */
#if 0
	/* step 6a: VCI */
	sb_dev = dev_find_slot(0, PCI_DEVFN(8, 0));
	if (port == 8) {
		/* Clear bits 7:1 */
		pci_ext_write_config32(nb_dev, sb_dev, 0x114, 0x3f << 1, 0 << 1);
		/* Maps Traffic Class 1-7 to VC1 */
		pci_ext_write_config32(nb_dev, sb_dev, 0x120, 0x7f << 1, 0x7f << 1);
		/* Assigns VC ID to 1 */
		pci_ext_write_config32(nb_dev, sb_dev, 0x120, 7 << 24, 1 << 24);
		/* Enables VC1 */
		pci_ext_write_config32(nb_dev, sb_dev, 0x120, 1 << 31, 1 << 31);
#if 0
		do {
			reg16 = pci_ext_read_config32(nb_dev, sb_dev, 0x124);
			reg16 &= 0x2;
		} while (reg16); /*bit[1] = 0 means VC1 flow control initialization is successful */
#endif
	}
#endif
#if 0
	/* step 6b: L0s for the southbridge link */
	/* To enalbe L0s in the southbridage*/
	/* step 6c: L0s for the GPP link(s) */
	/* To eable L0s in the RS780 for the GPP port(s) */
	printk_info("To enalbe L0s in the southbridage\n");
	set_pcie_enable_bits(nb_dev, 0xf9, 3 << 13, 2 << 13);
	set_pcie_enable_bits(dev, 0xa0, 0xf << 8, 0x9 << 8);
	reg16 = pci_read_config16(dev, 0x68);
	reg16 |= 1 << 0;
	pci_write_config16(dev, 0x68, reg16);
	/* step 6d: ASPM L1 for the southbridge link */
	/* To enalbe L1s in the southbridage*/
	/* step 6e: ASPM L1 for GPP link(s) */;
	printk_info("To enalbe L1s in the southbridage\n");
	set_pcie_enable_bits(nb_dev, 0xf9, 3 << 13, 2 << 13);
	set_pcie_enable_bits(dev, 0xa0, 3 << 12, 3 << 12);
	set_pcie_enable_bits(dev, 0xa0, 0xf << 4, 3 << 4);
	reg16 = pci_read_config16(dev, 0x68);
	reg16 &= ~0xff;
	reg16 |= 1 << 1;
	pci_write_config16(dev, 0x68, reg16);
	/* step 6f: Turning off PLL during L1/L23 */
	printk_info("Turning off PLL during L1/L23\n");
	set_pcie_enable_bits(nb_dev, 0x40, 1 << 3, 1 << 3);
	set_pcie_enable_bits(nb_dev, 0x40, 1 << 9, 1 << 9);
	/* step 6g: TXCLK clock gating */
	set_nbmisc_enable_bits(nb_dev, 0x7, 3 << 4, 3 << 4);
	set_nbmisc_enable_bits(nb_dev, 0x7, 1 << 22, 1 << 22);
	set_pcie_enable_bits(nb_dev, 0x11, 0xf << 4, 0xc << 4);
#endif
	/* step 6h: LCLK clock gating, done in rs780_config_misc_clk() */
}


/*****************************************
* Compliant with CIM_33's PCIEConfigureGPPCore
*****************************************/
void config_gpp_core(device_t nb_dev, device_t sb_dev)
{
	u32 reg;
	struct southbridge_amd_rs780_config *cfg =
	    //(struct southbridge_amd_rs690_config *)nb_dev->chip_info;
		&chip_info;
	printk_info("enter config_gpp_core\n");
	reg = nbmisc_read_index(nb_dev, 0x20);
	if (AtiPcieCfg.Config & PCIE_ENABLE_STATIC_DEV_REMAP)
		reg &= 0xfffffffd;	/* set bit1 = 0 */
	else
		reg |= 0x2;	/* set bit1 = 1 */
	nbmisc_write_index(nb_dev, 0x20, reg);

	printk_info("get STRAP_BIF_LINK_CONFIG_GPPSB\n");
	reg = nbmisc_read_index(nb_dev, 0x67);	/* get STRAP_BIF_LINK_CONFIG_GPPSB at bit 4-7 */
	if (cfg->gppsb_configuration != ((reg >> 4) & 0xf))
		//switching_gpp_configurations(nb_dev, sb_dev);
		switching_gppsb_configurations(nb_dev, sb_dev);
	printk_info("get STRAP_BIF_LINK_CONFIG_GPP\n");
	reg = nbmisc_read_index(nb_dev, 0x2D);	/* get STRAP_BIF_LINK_CONFIG_GPP at bit 7-10 */
	if (cfg->gpp_configuration != ((reg >> 7) & 0xf))
		switching_gpp_configurations(nb_dev, sb_dev);
	ValidatePortEn(nb_dev);
	printk_info("exit config_gpp_core\n");
}

/*****************************************
* Compliant with CIM_33's PCIEMiscClkProg
*****************************************/
void pcie_config_misc_clk(device_t nb_dev)
{
	u32 reg;
	//struct bus pbus; /* fake bus for dev0 fun1 */

	reg = pci_read_config32(nb_dev, 0x4c);
	reg |= 1 << 0;
	pci_write_config32(nb_dev, 0x4c, reg);

	if (AtiPcieCfg.Config & PCIE_GFX_CLK_GATING) {
		/* TXCLK Clock Gating */
		set_nbmisc_enable_bits(nb_dev, 0x07, 3 << 0, 3 << 0);
		set_nbmisc_enable_bits(nb_dev, 0x07, 1 << 22, 1 << 22);
		set_pcie_enable_bits(nb_dev, 0x11 | PCIE_CORE_INDEX_GFX, (3 << 6) | (~0xf), 3 << 6);

		/* LCLK Clock Gating */
		//reg =  pci_cf8_conf1.read32(&pbus, 0, 1, 0x94);
		reg =  pci_read_config32(_pci_make_tag(0, 0, 1), 0x94);
		reg &= ~(1 << 16);
		//pci_cf8_conf1.write32(&pbus, 0, 1, 0x94, reg);
		pci_write_config32(_pci_make_tag(0, 0, 1), 0x94, reg);
	}

	if (AtiPcieCfg.Config & PCIE_GPP_CLK_GATING) {
		/* TXCLK Clock Gating */
		set_nbmisc_enable_bits(nb_dev, 0x07, 3 << 4, 3 << 4);
		set_nbmisc_enable_bits(nb_dev, 0x07, 1 << 22, 1 << 22);
		set_pcie_enable_bits(nb_dev, 0x11 | PCIE_CORE_INDEX_GPPSB, (3 << 6) | (~0xf), 3 << 6);

		/* LCLK Clock Gating */
		//reg =  pci_cf8_conf1.read32(&pbus, 0, 1, 0x94);
		reg =  pci_read_config32(_pci_make_tag(0, 0, 1), 0x94);
		reg &= ~(1 << 24);
		//pci_cf8_conf1.write32(&pbus, 0, 1, 0x94, reg);
		pci_write_config32(_pci_make_tag(0, 0, 1), 0x94, reg);
	}

	reg = pci_read_config32(nb_dev, 0x4c);
	reg &= ~(1 << 0);
	pci_write_config32(nb_dev, 0x4c, reg);
}



