#include <unistd.h>
#include "rs780_chip.h"

extern struct southbridge_amd_rs780_config chip_info;

#if 0
/* step 12 ~ step 14 from rpr */
static void single_port_configuration(device_t nb_dev, device_t dev)
{
	u8 result, width;
	u32 reg32;
	struct southbridge_amd_rs780_config *cfg = &chip_info;

	printk_info("rs780_gfx_init single_port_configuration.\n");

	/* step 12 training, releases hold training for GFX port 0 (device 2) */
	PcieReleasePortTraining(nb_dev, dev, 2);
	result = PcieTrainPort(nb_dev, dev, 2);
	printk_info("rs780_gfx_init single_port_configuration step12.\n");

	/* step 13 Power Down Control */
	/* step 13.1 Enables powering down transmitter and receiver pads along with PLL macros. */
	set_pcie_enable_bits(nb_dev, 0x40, 1 << 0, 1 << 0);

	/* step 13.a Link Training was NOT successful */
	if (!result) {
		set_nbmisc_enable_bits(nb_dev, 0x8, 0, 0x3 << 4); /* prevent from training. */
		set_nbmisc_enable_bits(nb_dev, 0xc, 0, 0x3 << 2); /* hide the GFX bridge. */
		if (cfg->gfx_tmds)
			nbpcie_ind_write_index(nb_dev, 0x65, 0xccf0f0);
		else {
			nbpcie_ind_write_index(nb_dev, 0x65, 0xffffffff);
			set_nbmisc_enable_bits(nb_dev, 0x7, 1 << 3, 1 << 3);
		}
	} else {		/* step 13.b Link Training was successful */
		set_pcie_enable_bits(dev, 0xA2, 0xFF, 0x1);
		reg32 = nbpcie_p_read_index(dev, 0x29);
		width = reg32 & 0xFF;
		printk_debug("GFX Inactive Lanes = 0x%x.\n", width);
		switch (width) {
		case 1:
		case 2:
			nbpcie_ind_write_index(nb_dev, 0x65,
					       cfg->gfx_lane_reversal ? 0x7f7f : 0xccfefe);
			break;
		case 4:
			nbpcie_ind_write_index(nb_dev, 0x65,
					       cfg->gfx_lane_reversal ? 0x3f3f : 0xccfcfc);
			break;
		case 8:
			nbpcie_ind_write_index(nb_dev, 0x65,
					       cfg->gfx_lane_reversal ? 0x0f0f : 0xccf0f0);
			break;
		}
	}
	printk_info("rs780_gfx_init single_port_configuration step13.\n");

	/* step 14 Reset Enumeration Timer, disables the shortening of the enumeration timer */
	set_pcie_enable_bits(dev, 0x70, 1 << 19, 0 << 19);
	printk_info("rs780_gfx_init single_port_configuration step14.\n");
}

/* step 15 ~ step 18 from rpr */
static void dual_port_configuration(device_t nb_dev, device_t dev)
{
	u8 result, width;
	u32 reg32, dev_ind;
	struct southbridge_amd_rs780_config *cfg = &chip_info;
	
	_pci_break_tag(dev, NULL, &dev_ind, NULL);
	/* 5.4.1.2 Dual Port Configuration */
	set_nbmisc_enable_bits(nb_dev, 0x36, 1 << 31, 1 << 31);
	set_nbmisc_enable_bits(nb_dev, 0x08, 0xF << 8, 0x5 << 8);
	set_nbmisc_enable_bits(nb_dev, 0x36, 1 << 31, 0 << 31);

	/* Training for Device 2 */
	/* Releases hold training for GFX port 0 (device 2) */
	PcieReleasePortTraining(nb_dev, dev, dev_ind);
	/* PCIE Link Training Sequence */
	result = PcieTrainPort(nb_dev, dev, dev_ind);

	/* step 16: Power Down Control for Device 2 */
	/* step 16.a Link Training was NOT successful */
	if (!result) {
		/* Powers down all lanes for port A */
		/* nbpcie_ind_write_index(nb_dev, 0x65, 0x0f0f); */
		/* Note: I have to disable the slot where there isnt a device,
		 * otherwise the system will hang. I dont know why. Do you guys know? */
		set_nbmisc_enable_bits(nb_dev, 0x0c, 1 << dev_ind, 1 << dev_ind);
	} else {		/* step 16.b Link Training was successful */

		reg32 = nbpcie_p_read_index(dev, 0xa2);
		width = (reg32 >> 4) & 0x7;
		switch (width) {
		case 1:
		case 2:
			nbpcie_ind_write_index(nb_dev, 0x65,
					       cfg->gfx_lane_reversal ? 0x0707 : 0x0e0e);
			break;
		case 4:
			nbpcie_ind_write_index(nb_dev, 0x65,
					       cfg->gfx_lane_reversal ? 0x0303 : 0x0c0c);
			break;
		}
	}
#if 0
	/* step 17: Training for Device 3 */
	set_nbmisc_enable_bits(nb_dev, 0x8, 1 << 5, 0 << 5);
	/* Releases hold training for GFX port 0 (device 3) */
	PcieReleasePortTraining(nb_dev, dev, 3);
	/* PCIE Link Training Sequence */
	result = PcieTrainPort(nb_dev, dev, 3);

	/*step 18: Power Down Control for Device 3 */
	/* step 18.a Link Training was NOT successful */
	if (!result) {
		/* Powers down all lanes for port B and PLL1 */
		nbpcie_ind_write_index(nb_dev, 0x65, 0xccf0f0);
	} else {		/* step 18.b Link Training was successful */

		reg32 = nbpcie_p_read_index(dev, 0xa2);
		width = (reg32 >> 4) & 0x7;
		switch (width) {
		case 1:
		case 2:
			nbpcie_ind_write_index(nb_dev, 0x65,
					       cfg->gfx_lane_reversal ? 0x7070 : 0xe0e0);
			break;
		case 4:
			nbpcie_ind_write_index(nb_dev, 0x65,
					       cfg->gfx_lane_reversal ? 0x3030 : 0xc0c0);
			break;
		}
	}
#endif
}

/* For single port GFX configuration Only
* width:
* 	000 = x16
* 	001 = x1
*	010 = x2
*	011 = x4
*	100 = x8
*	101 = x12 (not supported)
*	110 = x16
*/
static void dynamic_link_width_control(device_t nb_dev, device_t dev, u8 width)
{
	u32 reg32;
	device_t sb_dev;
	struct southbridge_amd_rs780_config *cfg = &chip_info;

	/* step 5.9.1.1 */
	reg32 = nbpcie_p_read_index(dev, 0xa2);

	/* step 5.9.1.2 */
	set_pcie_enable_bits(nb_dev, 0x40, 1 << 0, 1 << 0);
	/* step 5.9.1.3 */
	set_pcie_enable_bits(dev, 0xa2, 3 << 0, width << 0);
	/* step 5.9.1.4 */
	set_pcie_enable_bits(dev, 0xa2, 1 << 8, 1 << 8);
	/* step 5.9.2.4 */
	if (0 == cfg->gfx_reconfiguration)
		set_pcie_enable_bits(dev, 0xa2, 1 << 11, 1 << 11);

	/* step 5.9.1.5 */
	do {
		reg32 = nbpcie_p_read_index(dev, 0xa2);
	}
	while (reg32 & 0x100);

	/* step 5.9.1.6 */
	//sb_dev = dev_find_slot(0, PCI_DEVFN(8, 0));
	sb_dev = _pci_make_tag(0, 8, 0);
	do {
		reg32 = pci_ext_read_config16(nb_dev, sb_dev,
					  PCIE_VC0_RESOURCE_STATUS);
	} while (reg32 & VC_NEGOTIATION_PENDING);

	/* step 5.9.1.7 */
	reg32 = nbpcie_p_read_index(dev, 0xa2);
	if (((reg32 & 0x70) >> 4) != 0x6) {
		/* the unused lanes should be powered off. */
	}

	/* step 5.9.1.8 */
	set_pcie_enable_bits(nb_dev, 0x40, 1 << 0, 0 << 0);
}


/*
* GFX Core initialization, dev2, dev3
*/
void rs780_gfx_init(device_t nb_dev, device_t dev, u32 port)
{
	u16 reg16;
	u32 reg32;
	void set_pcie_reset();
	void set_pcie_dereset();
	struct southbridge_amd_rs780_config *cfg =
		&chip_info;

	printk_info("rs780_gfx_init, nb_dev=0x%x, dev=0x%x, port=0x%x.\n",
		    nb_dev, dev, port);

	/* step 1, lane reversal (only need if CMOS option is enabled) */
	if (cfg->gfx_lane_reversal) {
		set_nbmisc_enable_bits(nb_dev, 0x33, 1 << 2, 1 << 2);
		if (cfg->gfx_dual_slot)
			set_nbmisc_enable_bits(nb_dev, 0x33, 1 << 3, 1 << 3);
	}
	printk_info("rs780_gfx_init step1.\n");
	/* step 1.1, dual-slot gfx configuration (only need if CMOS option is enabled) */
	/* AMD calls the configuration CrossFire */
	if (cfg->gfx_dual_slot)
		set_nbmisc_enable_bits(nb_dev, 0x0, 0xf << 8, 5 << 8);
	printk_info("rs780_gfx_init step2.\n");
	/* step 2, TMDS, (only need if CMOS option is enabled) */
	if (cfg->gfx_tmds) {
	}

#if 1				/* external clock mode */
	/* table 5-22, 5.9.1. REFCLK */
	/* 5.9.1.1. Disables the GFX REFCLK transmitter so that the GFX
	 * REFCLK PAD can be driven by an external source. */
	/* 5.9.1.2. Enables GFX REFCLK receiver to receive the REFCLK from an external source. */
	set_nbmisc_enable_bits(nb_dev, 0x38, 1 << 29 | 1 << 28, 0 << 29 | 1 << 28);

	/* 5.9.1.3 Selects the GFX REFCLK to be the source for PLL A. */
	/* 5.9.1.4 Selects the GFX REFCLK to be the source for PLL B. */
	/* 5.9.1.5 Selects the GFX REFCLK to be the source for PLL C. */
	set_nbmisc_enable_bits(nb_dev, 0x28, 3 << 6 | 3 << 8 | 3 << 10,
			       1 << 6 | 1 << 8 | 1 << 10);
	reg32 = nbmisc_read_index(nb_dev, 0x28);
	printk_info("misc 28 = %x\n", reg32);

	/* 5.9.1.6.Selects the single ended GFX REFCLK to be the source for core logic. */
	set_nbmisc_enable_bits(nb_dev, 0x6C, 1 << 31, 1 << 31);
#else				/* internal clock mode */
	/* table 5-23, 5.9.1. REFCLK */
	/* 5.9.1.1. Enables the GFX REFCLK transmitter so that the GFX
	 * REFCLK PAD can be driven by the SB REFCLK. */
	/* 5.9.1.2. Disables GFX REFCLK receiver from receiving the
	 * REFCLK from an external source.*/
	set_nbmisc_enable_bits(nb_dev, 0x38, 1 << 29 | 1 << 28, 1 << 29 | 0 << 28);

	/* 5.9.1.3 Selects the GFX REFCLK to be the source for PLL A. */
	/* 5.9.1.4 Selects the GFX REFCLK to be the source for PLL B. */
	/* 5.9.1.5 Selects the GFX REFCLK to be the source for PLL C. */
	set_nbmisc_enable_bits(nb_dev, 0x28, 3 << 6 | 3 << 8 | 3 << 10,
			       0);
	reg32 = nbmisc_read_index(nb_dev, 0x28);
	printk_info("misc 28 = %x\n", reg32);

	/* 5.9.1.6.Selects the single ended GFX REFCLK to be the source for core logic. */
	set_nbmisc_enable_bits(nb_dev, 0x6C, 1 << 31, 0 << 31);
#endif
	/* step 5.9.3, GFX overclocking, (only need if CMOS option is enabled) */
	/* 5.9.3.1. Increases PLL BW for 6G operation.*/
	/* set_nbmisc_enable_bits(nb_dev, 0x36, 0x3FF << 4, 0xB5 << 4); */
	/* skip */

	/* step 5.9.4, reset the GFX link */
	/* step 5.9.4.1 asserts both calibration reset and global reset */
	set_nbmisc_enable_bits(nb_dev, 0x8, 0x3 << 14, 0x3 << 14);

	/* step 5.9.4.2 de-asserts calibration reset */
	set_nbmisc_enable_bits(nb_dev, 0x8, 1 << 14, 0 << 14);

	/* step 5.9.4.3 wait for at least 200us */
	delay(3);

	/* step 5.9.4.4 de-asserts global reset */
	set_nbmisc_enable_bits(nb_dev, 0x8, 1 << 15, 0 << 15);

	/* 5.9.5 Reset PCIE_GFX Slot */
	/* It is done in mainboard.c */
	set_pcie_reset();
	delay(1);
	set_pcie_dereset();
	/* step 5.9.8 program PCIE memory mapped configuration space */
	/* done by enable_pci_bar3() before */

	/* step 7 compliance state, (only need if CMOS option is enabled) */
	/* the compliance stete is just for test. refer to 4.2.5.2 of PCIe specification */
	if (cfg->gfx_compliance) {
		/* force compliance */
		set_nbmisc_enable_bits(nb_dev, 0x32, 1 << 6, 1 << 6);
		/* release hold training for device 2. GFX initialization is done. */
		set_nbmisc_enable_bits(nb_dev, 0x8, 1 << 4, 0 << 4);
		dynamic_link_width_control(nb_dev, dev, cfg->gfx_link_width);
		printk_info("rs780_gfx_init step7.\n");
		return;
	}

	/* 5.9.12 Core Initialization. */
	/* 5.9.12.1 sets RCB timeout to be 25ms */
	/* 5.9.12.2. RCB Cpl timeout on link down. */
	set_pcie_enable_bits(dev, 0x70, 7 << 16 | 1 << 19, 4 << 16 | 1 << 19);
	printk_info("rs780_gfx_init step5.9.12.1.\n");

	/* step 5.9.12.3 disables slave ordering logic */
	set_pcie_enable_bits(nb_dev, 0x20, 1 << 8, 1 << 8);
	printk_info("rs780_gfx_init step5.9.12.3.\n");

	/* step 5.9.12.4 sets DMA payload size to 64 bytes */
	set_pcie_enable_bits(nb_dev, 0x10, 7 << 10, 4 << 10);
	/* 5.9.12.5. Blocks DMA traffic during C3 state. */
	set_pcie_enable_bits(dev, 0x10, 1 << 0, 0 << 0);

	/* 5.9.12.6. Disables RC ordering logic */
	set_pcie_enable_bits(nb_dev, 0x20, 1 << 9, 1 << 9);

	/* Enabels TLP flushing. */
	/* Note: It is got from RS690. The system will hang without this action. */
	set_pcie_enable_bits(dev, 0x20, 1 << 19, 0 << 19);

	/* 5.9.12.7. Ignores DLLPs during L1 so that txclk can be turned off */
	set_pcie_enable_bits(nb_dev, 0x2, 1 << 0, 1 << 0);

	/* 5.9.12.8 Prevents LC to go from L0 to Rcv_L0s if L1 is armed. */
	set_pcie_enable_bits(dev, 0xA1, 1 << 11, 1 << 11);

	/* 5.9.12.9 CMGOOD_OVERRIDE for end point initiated lane degradation. */
	set_nbmisc_enable_bits(nb_dev, 0x6a, 1 << 17, 1 << 17);
	printk_info("rs780_gfx_init step5.9.12.9.\n");

	/* 5.9.12.10 Sets the timer in Config state from 20us to */
	/* 5.9.12.11 De-asserts RX_EN in L0s. */
	/* 5.9.12.12 Enables de-assertion of PG2RX_CR_EN to lock clock
	 * recovery parameter when lane is in electrical idle in L0s.*/
	set_pcie_enable_bits(dev, 0xB1, 1 << 23 | 1 << 19 | 1 << 28, 1 << 23 | 1 << 19 | 1 << 28);

	/* 5.9.12.13. Turns off offset calibration. */
	/* 5.9.12.14. Enables Rx Clock gating in CDR */
	set_nbmisc_enable_bits(nb_dev, 0x34, 1 << 10/* | 1 << 22 */, 1 << 10/* | 1 << 22 */);

	/* 5.9.12.15. Sets number of TX Clocks to drain TX Pipe to 3. */
	set_pcie_enable_bits(dev, 0xA0, 0xF << 4, 3 << 4);

	/* 5.9.12.16. Lets PI use Electrical Idle from PHY when
	 * turning off PLL in L1 at Gen2 speed instead Inferred Electrical Idle. */
	set_pcie_enable_bits(nb_dev, 0x40, 3 << 14, 2 << 14);

	/* 5.9.12.17. Prevents the Electrical Idle from causing a transition from Rcv_L0 to Rcv_L0s. */
	set_pcie_enable_bits(dev, 0xB1, 1 << 20, 1 << 20);

	/* 5.9.12.18. Prevents the LTSSM from going to Rcv_L0s if it has already
	 * acknowledged a request to go to L1. */
	set_pcie_enable_bits(dev, 0xA1, 1 << 11, 1 << 11);

	/* 5.9.12.19. LDSK only taking deskew on deskewing error detect */
	set_pcie_enable_bits(nb_dev, 0x40, 1 << 28, 0 << 28);

	/* 5.9.12.20. Bypasses lane de-skew logic if in x1 */
	set_pcie_enable_bits(nb_dev, 0xC2, 1 << 14, 1 << 14);

	/* 5.9.12.21. Sets Electrical Idle Threshold. */
	set_nbmisc_enable_bits(nb_dev, 0x35, 3 << 21, 2 << 21);

	/* 5.9.12.22. Advertises -6 dB de-emphasis value in TS1 Data Rate Identifier
	 * Only if CMOS Option in section. skip */

	/* 5.9.12.23. Disables GEN2 capability of the device. */
	set_pcie_enable_bits(dev, 0xA4, 1 << 0, 0 << 0);

	/* 5.9.12.24.Disables advertising Upconfigure Support. */
	set_pcie_enable_bits(dev, 0xA2, 1 << 13, 1 << 13);

	/* 5.9.12.25. No comment in RPR. */
	set_nbmisc_enable_bits(nb_dev, 0x39, 1 << 10, 0 << 10);

	/* 5.9.12.26. This capacity is required since links wider than x1 and/or multiple link
	 * speed are supported */
	set_pcie_enable_bits(nb_dev, 0xC1, 1 << 0, 1 << 0);

	/* 5.9.12.27. Enables NVG86 ECO. A13 above only. */
	/* TODO: Check if it is A13. */
	if (0)			/* A12 */
		set_pcie_enable_bits(dev, 0x02, 1 << 11, 1 << 11);

	/* 5.9.12.28 Hides and disables the completion timeout method. */
	set_pcie_enable_bits(nb_dev, 0xC1, 1 << 2, 0 << 2);

	/* 5.9.12.29. Use the bif_core de-emphasis strength by default. */
	/* set_nbmisc_enable_bits(nb_dev, 0x36, 1 << 28, 1 << 28); */

	/* 5.9.12.30. Set TX arbitration algorithm to round robin */
	set_pcie_enable_bits(nb_dev, 0x1C,
			     1 << 0 | 0x1F << 1 | 0x1F << 6,
			     1 << 0 | 0x04 << 1 | 0x04 << 6);

	/* Single-port/Dual-port configureation. */
	switch (cfg->gfx_dual_slot) {
	case 0:
		single_port_configuration(nb_dev, dev);
		break;
	case 1:
		dual_port_configuration(nb_dev, dev);
		break;
	default:
		printk_info("Incorrect configuration of external gfx slot.\n");
		break;
	}
}
#endif



//BTDC
static void pcie_commoncoreinit(device_t nb_dev, device_t dev)
{
	u32 reg;
	u16 word;
	u8  byte;

	set_pcie_enable_bits(nb_dev,0x10, 0x7 << 10, 0x4 << 10);
	set_pcie_enable_bits(nb_dev,0x20, 0x3 << 8,  0x3 << 8);
	set_pcie_enable_bits(nb_dev,0x02, 0x0, 1 << 0 | 1 << 8);
	set_pcie_enable_bits(nb_dev,0x40, 0x3 << 14, 0x2 << 14);
	set_pcie_enable_bits(nb_dev,0x40, 0x1 << 28, 0x0 << 28);
	set_pcie_enable_bits(nb_dev,0xc2, 1 << 14 | 1 << 25, 1 << 14 | 1 << 25);
	set_pcie_enable_bits(nb_dev,0xc1, 1 << 0 | 1 << 2, 1 << 0);
	set_pcie_enable_bits(nb_dev,0x1c, 0xffffffff, 4 << 6 | 4 << 1);


}

static void pcie_commonportinit(device_t nb_dev, device_t dev)
{
	u32 reg;
	u16 word;
	u8  byte;

	reg = pci_read_config32(dev,0x88);
	reg &= 0xfffffff0;
	reg |= 0x1;
	pci_write_config32(dev, 0x88, reg);

	set_pcie_enable_bits(dev, 0x02, 1 << 11, 1 << 11);
	set_pcie_enable_bits(dev, 0xa4, 1 << 0 , 0 << 0);
	set_pcie_enable_bits(dev, 0xa0, 0xf << 4 , 0x3 << 4);
	set_pcie_enable_bits(dev, 0xa1, 1 << 11 | 1 << 24 | 1 << 26, 1 << 11);
	set_pcie_enable_bits(dev, 0xb1, 0x0 , 1 << 28 | 1 << 23 | 1 << 19 | 1<< 20);
	set_pcie_enable_bits(dev, 0xa2, 0x0 , 1 << 13);
	set_pcie_enable_bits(dev, 0x70, 1 << 16 | 1 << 17 | 1 << 18 | 1 << 19, 1 << 18 | 1 << 19); 
}

//this include PCIE_InitGen2(Port, 1) and PCIE_InitGen2(Port, 0)

static void pcie_initgen2(device_t nb_dev, device_t dev)
{
	u32 reg;
	u16 word;
	u8  byte;

	set_pcie_enable_bits(dev, 0xa4, 1 << 0, 0 << 0);
	reg = pci_read_config32(dev,0x88);
	reg &= 0xfffffff0;
	reg |= 0x1;
	pci_write_config32(dev, 0x88, reg);
	set_nbmisc_enable_bits(nb_dev, 0x34, 1 << 5, 0 << 5);


	set_pcie_enable_bits(dev, 0xa4, 1 << 0, 1 << 0);
	reg = pci_read_config32(dev, 0x88);
	reg &= 0xfffffff0;
	reg |= 0x2;
	pci_write_config32(dev, 0x88, reg);
	set_nbmisc_enable_bits(nb_dev, 0x34, 1 << 5, 1 << 5);


	set_pcie_enable_bits(dev, 0xa4, 0, 1 << 29);
	set_pcie_enable_bits(dev, 0xc0, 1 << 15, 0 << 15);
	set_pcie_enable_bits(dev, 0xa2, 1 << 13, 0 << 13);

//set interrupt pin info
	pci_write_config8(dev, 0x3d, 0x1);


//	while(1);

}

static void pcie_gen2workaround(device_t nb_dev, device_t dev)
{
	u32 reg;
	u16 word;
	u8  byte;
	void set_pcie_reset();
	void set_pcie_dereset();



	set_pcie_enable_bits(dev, 0xa4, 1 << 0, 0 << 0);
	reg = pci_read_config32(dev,0x88);
	reg &= 0xfffffff0;
	reg |= 0x1;
	pci_write_config32(dev, 0x88, reg);
	set_nbmisc_enable_bits(nb_dev, 0x34, 1 << 5, 0 << 5);

	set_pcie_enable_bits(dev, 0xa4, 1 << 29, 0 << 29);
	set_pcie_enable_bits(dev, 0xc0, 1 << 15, 1 << 15);
	set_pcie_enable_bits(dev, 0xa2, 1 << 13, 1 << 13);

	reg = htiu_read_index(nb_dev, 0x15);
	reg &= ~ (1 << 4);
	htiu_write_index(nb_dev, 0x15, 0);

	set_pcie_reset();
	delay(1000);
	set_pcie_dereset();


}


 

/* step 12 ~ step 14 from rpr */
static void single_port_configuration(device_t nb_dev, device_t dev)
{
	u8 result, width;
	u32 reg32;
	struct southbridge_amd_rs780_config *cfg =&chip_info;

	printk_info("rs780_gfx_init single_port_configuration.\n");

	/* step 12 training, releases hold training for GFX port 0 (device 2) */
	PcieReleasePortTraining(nb_dev, dev, 2);
	result = PcieTrainPort(nb_dev, dev, 2);
	printk_info("rs780_gfx_init single_port_configuration step12.\n");

	/* step 13 Power Down Control */
	/* step 13.1 Enables powering down transmitter and receiver pads along with PLL macros. */
	set_pcie_enable_bits(nb_dev, 0x40, 1 << 0, 1 << 0);

	/* step 13.a Link Training was NOT successful */
	if (!result) {
		set_nbmisc_enable_bits(nb_dev, 0x8, 0x1 << 4, 0x1 << 4); /* prevent from training. */
		set_nbmisc_enable_bits(nb_dev, 0xc, 0x1 << 2, 0x1 << 2); /* hide the GFX bridge. */

		if (cfg->gfx_tmds)
			nbpcie_ind_write_index(nb_dev, 0x65, 0xccf0f0);
		else {
			nbpcie_ind_write_index(nb_dev, 0x65, 0xffffffff);
			set_nbmisc_enable_bits(nb_dev, 0x7, 1 << 3, 1 << 3);
		}
	} else {		/* step 13.b Link Training was successful */

		set_pcie_enable_bits(dev, 0xA2, 0xFF, 0x1);
			
		reg32 = nbpcie_p_read_index(dev, 0x29);
		width = reg32 & 0xFF;
		printk_debug("GFX Inactive Lanes = 0x%x.\n", width);
		switch (width) {
		case 1:
		case 2:
			nbpcie_ind_write_index(nb_dev, 0x65,
					       cfg->gfx_lane_reversal ? 0x7f7f : 0xccfefe);
			break;
		case 4:
			nbpcie_ind_write_index(nb_dev, 0x65,
					       cfg->gfx_lane_reversal ? 0x3f3f : 0xccfcfc);
			break;
		case 8:
			nbpcie_ind_write_index(nb_dev, 0x65,
					       cfg->gfx_lane_reversal ? 0x0f0f : 0xccf0f0);
			break;
		}



	}
	printk_info("rs780_gfx_init single_port_configuration step13.\n");



	/* step 14 Reset Enumeration Timer, disables the shortening of the enumeration timer */
	set_pcie_enable_bits(dev, 0x70, 1 << 19, 1 << 19);
	printk_info("rs780_gfx_init single_port_configuration step14.\n");
}

/* step 15 ~ step 18 from rpr */
static void dual_port_configuration(device_t nb_dev, device_t dev)
{
	u8 result, width;
	u32 reg32, dev_ind;
	struct southbridge_amd_rs780_config *cfg =&chip_info;

        _pci_break_tag(dev, NULL, &dev_ind, NULL);
	/* 5.4.1.2 Dual Port Configuration */
	set_nbmisc_enable_bits(nb_dev, 0x36, 1 << 31, 1 << 31);
	set_nbmisc_enable_bits(nb_dev, 0x08, 0xF << 8, 0x5 << 8);
	set_nbmisc_enable_bits(nb_dev, 0x36, 1 << 31, 0 << 31);

	/* Training for Device 2 */
	/* Releases hold training for GFX port 0 (device 2) */
	PcieReleasePortTraining(nb_dev, dev, dev_ind);
	/* PCIE Link Training Sequence */
	result = PcieTrainPort(nb_dev, dev, dev_ind);

	/* step 16: Power Down Control for Device 2 */
	/* step 16.a Link Training was NOT successful */
	if (!result) {
		/* Powers down all lanes for port A */
		/* nbpcie_ind_write_index(nb_dev, 0x65, 0x0f0f); */
		/* Note: I have to disable the slot where there isnt a device,
		 * otherwise the system will hang. I dont know why. Do you guys know? */
		set_nbmisc_enable_bits(nb_dev, 0x0c, 1 << dev_ind, 1 << dev_ind);

	} else {		/* step 16.b Link Training was successful */
		/* FIXME: I am confused about this. It looks like about powerdown or something. */
		reg32 = nbpcie_p_read_index(dev, 0xa2);
		width = (reg32 >> 4) & 0x7;
		printk_debug("GFX LC_LINK_WIDTH = 0x%x.\n", width);
		switch (width) {
		case 1:
		case 2:
			nbpcie_ind_write_index(nb_dev, 0x65,
					       cfg->gfx_lane_reversal ? 0x0707 : 0x0e0e);
			break;
		case 4:
			nbpcie_ind_write_index(nb_dev, 0x65,
					       cfg->gfx_lane_reversal ? 0x0303 : 0x0c0c);
			break;
		}
	}
#if 0		/* This function will be called twice, so we dont have to do dev 3 here. */
	/* step 17: Training for Device 3 */
	//set_nbmisc_enable_bits(nb_dev, 0x8, 1 << 5, 0 << 5);
	/* Releases hold training for GFX port 0 (device 3) */
	PcieReleasePortTraining(nb_dev, dev, 3);
	/* PCIE Link Training Sequence */
	result = PcieTrainPort(nb_dev, dev, 3);

	/*step 18: Power Down Control for Device 3 */
	/* step 18.a Link Training was NOT successful */
	if (!result) {
		/* Powers down all lanes for port B and PLL1 */
		nbpcie_ind_write_index(nb_dev, 0x65, 0xccf0f0);
	} else {		/* step 18.b Link Training was successful */

		reg32 = nbpcie_p_read_index(dev, 0xa2);
		width = (reg32 >> 4) & 0x7;
		printk_debug("GFX LC_LINK_WIDTH = 0x%x.\n", width);
		switch (width) {
		case 1:
		case 2:
			nbpcie_ind_write_index(nb_dev, 0x65,
					       cfg->gfx_lane_reversal ? 0x7070 : 0xe0e0);
			break;
		case 4:
			nbpcie_ind_write_index(nb_dev, 0x65,
					       cfg->gfx_lane_reversal ? 0x3030 : 0x0f0f);
			break;
		}
	}
#endif
}


/* For single port GFX configuration Only
* width:
* 	000 = x16
* 	001 = x1
*	010 = x2
*	011 = x4
*	100 = x8
*	101 = x12 (not supported)
*	110 = x16
*/
static void dynamic_link_width_control(device_t nb_dev, device_t dev, u8 width)
{
	u32 reg32;
	device_t sb_dev;
	struct southbridge_amd_rs780_config *cfg =&chip_info;

	/* step 5.9.1.1 */
	reg32 = nbpcie_p_read_index(dev, 0xa2);

	/* step 5.9.1.2 */
	set_pcie_enable_bits(nb_dev, 0x40, 1 << 0, 1 << 0);
	/* step 5.9.1.3 */
	set_pcie_enable_bits(dev, 0xa2, 3 << 0, width << 0);
	/* step 5.9.1.4 */
	set_pcie_enable_bits(dev, 0xa2, 1 << 8, 1 << 8);
	/* step 5.9.2.4 */
	if (0 == cfg->gfx_reconfiguration)
		set_pcie_enable_bits(dev, 0xa2, 1 << 11, 1 << 11);

	/* step 5.9.1.5 */
	do {
		reg32 = nbpcie_p_read_index(dev, 0xa2);
	}
	while (reg32 & 0x100);

	/* step 5.9.1.6 */
	sb_dev = _pci_make_tag(0, 8, 0);
	do {
		reg32 = pci_ext_read_config16(nb_dev, sb_dev,
					  PCIE_VC0_RESOURCE_STATUS);
	} while (reg32 & VC_NEGOTIATION_PENDING);

	/* step 5.9.1.7 */
	reg32 = nbpcie_p_read_index(dev, 0xa2);
	if (((reg32 & 0x70) >> 4) != 0x6) {
		/* the unused lanes should be powered off. */
	}

	/* step 5.9.1.8 */
	set_pcie_enable_bits(nb_dev, 0x40, 1 << 0, 0 << 0);
}

/*
* GFX Core initialization, dev2, dev3
*/
void rs780_gfx_init(device_t nb_dev, device_t dev, u32 port)
{
	u8  byte;
	u16 reg16;
	u32 reg32;
        u32 dev_ind;
	void set_pcie_reset();
	void set_pcie_dereset();
	//u8   is_dev3_present();

	struct southbridge_amd_rs780_config *cfg =
		&chip_info;
        _pci_break_tag(dev, NULL, &dev_ind, NULL);
	printk_info("rs780_gfx_init, nb_dev=0x%p, dev=0x%p, port=0x%x.\n",
		    nb_dev, dev, port);

	/* GFX Core Initialization */
	//if (port == 2) return;



	/* step 2, TMDS, (only need if CMOS option is enabled) */
	if (cfg->gfx_tmds) {
	}

#if 1				/* external clock mode */
	/* table 5-22, 5.9.1. REFCLK */
	/* 5.9.1.1. Disables the GFX REFCLK transmitter so that the GFX
	 * REFCLK PAD can be driven by an external source. */
	/* 5.9.1.2. Enables GFX REFCLK receiver to receive the REFCLK from an external source. */
	set_nbmisc_enable_bits(nb_dev, 0x38, 1 << 29 | 1 << 28, 0 << 29 | 1 << 28);

	/* 5.9.1.3 Selects the GFX REFCLK to be the source for PLL A. */
	/* 5.9.1.4 Selects the GFX REFCLK to be the source for PLL B. */
	/* 5.9.1.5 Selects the GFX REFCLK to be the source for PLL C. */
	set_nbmisc_enable_bits(nb_dev, 0x28, 3 << 6 | 3 << 8 | 3 << 10,
			       1 << 6 | 1 << 8 | 1 << 10);
	reg32 = nbmisc_read_index(nb_dev, 0x28);
	printk_info("misc 28 = %x\n", reg32);

	/* 5.9.1.6.Selects the single ended GFX REFCLK to be the source for core logic. */
	set_nbmisc_enable_bits(nb_dev, 0x6C, 1 << 31, 1 << 31);
#else				/* internal clock mode */
	/* table 5-23, 5.9.1. REFCLK */
	/* 5.9.1.1. Enables the GFX REFCLK transmitter so that the GFX
	 * REFCLK PAD can be driven by the SB REFCLK. */
	/* 5.9.1.2. Disables GFX REFCLK receiver from receiving the
	 * REFCLK from an external source.*/
	set_nbmisc_enable_bits(nb_dev, 0x38, 1 << 29 | 1 << 28, 1 << 29 | 0 << 28);

	/* 5.9.1.3 Selects the GFX REFCLK to be the source for PLL A. */
	/* 5.9.1.4 Selects the GFX REFCLK to be the source for PLL B. */
	/* 5.9.1.5 Selects the GFX REFCLK to be the source for PLL C. */
	set_nbmisc_enable_bits(nb_dev, 0x28, 3 << 6 | 3 << 8 | 3 << 10,
			       0);
	reg32 = nbmisc_read_index(nb_dev, 0x28);
	printk_info("misc 28 = %x\n", reg32);

	/* 5.9.1.6.Selects the single ended GFX REFCLK to be the source for core logic. */
	set_nbmisc_enable_bits(nb_dev, 0x6C, 1 << 31, 0 << 31);
#endif

	/* step 5.9.3, GFX overclocking, (only need if CMOS option is enabled) */
	/* 5.9.3.1. Increases PLL BW for 6G operation.*/
	/* set_nbmisc_enable_bits(nb_dev, 0x36, 0x3FF << 4, 0xB5 << 4); */
	/* skip */

	/* step 5.9.4, reset the GFX link */
	/* step 5.9.4.1 asserts both calibration reset and global reset */
	set_nbmisc_enable_bits(nb_dev, 0x8, 0x3 << 14, 0x3 << 14);

	/* step 5.9.4.2 de-asserts calibration reset */
	set_nbmisc_enable_bits(nb_dev, 0x8, 1 << 14, 0 << 14);

	/* step 5.9.4.3 wait for at least 200us */
	udelay(300);

	/* step 5.9.4.4 de-asserts global reset */
	set_nbmisc_enable_bits(nb_dev, 0x8, 1 << 15, 0 << 15);

	/* 5.9.5 Reset PCIE_GFX Slot */
	/* It is done in mainboard.c */
	set_pcie_reset();
	delay(1000);
	set_pcie_dereset();

	/* step 5.9.8 program PCIE memory mapped configuration space */
	/* done by enable_pci_bar3() before */

	/* step 7 compliance state, (only need if CMOS option is enabled) */
	/* the compliance stete is just for test. refer to 4.2.5.2 of PCIe specification */
	if (cfg->gfx_compliance) {
		/* force compliance */
		set_nbmisc_enable_bits(nb_dev, 0x32, 1 << 6, 1 << 6);
		/* release hold training for device 2. GFX initialization is done. */
		set_nbmisc_enable_bits(nb_dev, 0x8, 1 << 4, 0 << 4);
		dynamic_link_width_control(nb_dev, dev, cfg->gfx_link_width);
		printk_info("rs780_gfx_init step7.\n");
		return;
	}

	/* 5.9.12 Core Initialization. */
	/* 5.9.12.1 sets RCB timeout to be 25ms */
	/* 5.9.12.2. RCB Cpl timeout on link down. */
	set_pcie_enable_bits(dev, 0x70, 7 << 16 | 1 << 19, 4 << 16 | 1 << 19);
	printk_info("rs780_gfx_init step5.9.12.1.\n");

	/* step 5.9.12.3 disables slave ordering logic */
	set_pcie_enable_bits(nb_dev, 0x20, 1 << 8, 1 << 8);
	printk_info("rs780_gfx_init step5.9.12.3.\n");

	/* step 5.9.12.4 sets DMA payload size to 64 bytes */
	set_pcie_enable_bits(nb_dev, 0x10, 7 << 10, 4 << 10);
	/* 5.9.12.5. Blocks DMA traffic during C3 state. */
	set_pcie_enable_bits(dev, 0x10, 1 << 0, 0 << 0);

	/* 5.9.12.6. Disables RC ordering logic */
	set_pcie_enable_bits(nb_dev, 0x20, 1 << 9, 1 << 9);

	/* Enabels TLP flushing. */
	/* Note: It is got from RS690. The system will hang without this action. */
	set_pcie_enable_bits(dev, 0x20, 1 << 19, 0 << 19);

	/* 5.9.12.7. Ignores DLLPs during L1 so that txclk can be turned off */
	set_pcie_enable_bits(nb_dev, 0x2, 1 << 0, 1 << 0);

	/* 5.9.12.8 Prevents LC to go from L0 to Rcv_L0s if L1 is armed. */
	set_pcie_enable_bits(dev, 0xA1, 1 << 11, 1 << 11);

	/* 5.9.12.9 CMGOOD_OVERRIDE for end point initiated lane degradation. */
	set_nbmisc_enable_bits(nb_dev, 0x6a, 1 << 17, 1 << 17);
	printk_info("rs780_gfx_init step5.9.12.9.\n");

	/* 5.9.12.10 Sets the timer in Config state from 20us to */
	/* 5.9.12.11 De-asserts RX_EN in L0s. */
	/* 5.9.12.12 Enables de-assertion of PG2RX_CR_EN to lock clock
	 * recovery parameter when lane is in electrical idle in L0s.*/
	set_pcie_enable_bits(dev, 0xB1, 1 << 23 | 1 << 19 | 1 << 28, 1 << 23 | 1 << 19 | 1 << 28);

	/* 5.9.12.13. Turns off offset calibration. */
	/* 5.9.12.14. Enables Rx Clock gating in CDR */
	set_nbmisc_enable_bits(nb_dev, 0x34, 1 << 10/* | 1 << 22 */, 1 << 10/* | 1 << 22 */);

	/* 5.9.12.15. Sets number of TX Clocks to drain TX Pipe to 3. */
	set_pcie_enable_bits(dev, 0xA0, 0xF << 4, 3 << 4);

	/* 5.9.12.16. Lets PI use Electrical Idle from PHY when
	 * turning off PLL in L1 at Gen2 speed instead Inferred Electrical Idle. */
	set_pcie_enable_bits(nb_dev, 0x40, 3 << 14, 2 << 14);

	/* 5.9.12.17. Prevents the Electrical Idle from causing a transition from Rcv_L0 to Rcv_L0s. */
	set_pcie_enable_bits(dev, 0xB1, 1 << 20, 1 << 20);

	/* 5.9.12.18. Prevents the LTSSM from going to Rcv_L0s if it has already
	 * acknowledged a request to go to L1. */
	set_pcie_enable_bits(dev, 0xA1, 1 << 11, 1 << 11);

	/* 5.9.12.19. LDSK only taking deskew on deskewing error detect */
	set_pcie_enable_bits(nb_dev, 0x40, 1 << 28, 0 << 28);

	/* 5.9.12.20. Bypasses lane de-skew logic if in x1 */
	set_pcie_enable_bits(nb_dev, 0xC2, 1 << 14, 1 << 14);

	/* 5.9.12.21. Sets Electrical Idle Threshold. */
	set_nbmisc_enable_bits(nb_dev, 0x35, 3 << 21, 2 << 21);

	/* 5.9.12.22. Advertises -6 dB de-emphasis value in TS1 Data Rate Identifier
	 * Only if CMOS Option in section. skip */

	/* 5.9.12.23. Disables GEN2 capability of the device. */
	set_pcie_enable_bits(dev, 0xA4, 1 << 0, 0 << 0);

	/* 5.9.12.24.Disables advertising Upconfigure Support. */
	set_pcie_enable_bits(dev, 0xA2, 1 << 13, 1 << 13);

	/* 5.9.12.25. No comment in RPR. */
	set_nbmisc_enable_bits(nb_dev, 0x39, 1 << 10, 0 << 10);

	/* 5.9.12.26. This capacity is required since links wider than x1 and/or multiple link
	 * speed are supported */
	set_pcie_enable_bits(nb_dev, 0xC1, 1 << 0, 1 << 0);

	/* 5.9.12.27. Enables NVG86 ECO. A13 above only. */
	/* TODO: Check if it is A13. */
	if (0)			/* A12 */
		set_pcie_enable_bits(dev, 0x02, 1 << 11, 1 << 11);

	/* 5.9.12.28 Hides and disables the completion timeout method. */
	set_pcie_enable_bits(nb_dev, 0xC1, 1 << 2, 0 << 2);

	/* 5.9.12.29. Use the bif_core de-emphasis strength by default. */
	/* set_nbmisc_enable_bits(nb_dev, 0x36, 1 << 28, 1 << 28); */

	/* 5.9.12.30. Set TX arbitration algorithm to round robin */
	set_pcie_enable_bits(nb_dev, 0x1C,
			     1 << 0 | 0x1F << 1 | 0x1F << 6,
			     1 << 0 | 0x04 << 1 | 0x04 << 6);

	pcie_commoncoreinit(nb_dev, dev);
	pcie_commonportinit(nb_dev, dev);
	pcie_initgen2(nb_dev, dev);
	pcie_gen2workaround(nb_dev, dev);


	/* Single-port/Dual-port configureation. */
	switch (cfg->gfx_dual_slot) {
	case 0:
		/* step 1, lane reversal (only need if CMOS option is enabled) */
		if (cfg->gfx_lane_reversal) {
			set_nbmisc_enable_bits(nb_dev, 0x33, 1 << 2, 1 << 2);
		}
		printk_info("rs780_gfx_init step1.\n");
		printk_info("rs780_gfx_init step2.\n");

		printk_info("device = %x\n", dev_ind);
		if(dev_ind == 2)
			single_port_configuration(nb_dev, dev);
		else{
			set_nbmisc_enable_bits(nb_dev, 0xc, 0x2 << 2, 0x2 << 2); /* hide the GFX bridge. */
			printk_info("If dev3.., single port. Do nothing.\n");
		}

		break;
	case 1:
		/* step 1, lane reversal (only need if CMOS option is enabled) */
		if (cfg->gfx_lane_reversal) {
			set_nbmisc_enable_bits(nb_dev, 0x33, 1 << 2, 1 << 2);
			set_nbmisc_enable_bits(nb_dev, 0x33, 1 << 3, 1 << 3);
		}
		printk_info("rs780_gfx_init step1.\n");
		/* step 1.1, dual-slot gfx configuration (only need if CMOS option is enabled) */
		/* AMD calls the configuration CrossFire */
		set_nbmisc_enable_bits(nb_dev, 0x0, 0xf << 8, 5 << 8);
		printk_info("rs780_gfx_init step2.\n");

		printk_info("device = %x\n", dev_ind);
		dual_port_configuration(nb_dev, dev);
		break;
	default:
		printk_info("Incorrect configuration of external gfx slot.\n");
		break;
	}
}