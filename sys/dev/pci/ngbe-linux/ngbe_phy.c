/*
 * WangXun Gigabit PCI Express Linux driver
 * Copyright (c) 2015 - 2017 Beijing WangXun Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * The full GNU General Public License is included in this distribution in
 * the file called "COPYING".
 *
 */

#include "ngbe_phy.h"

/**
 * ngbe_check_reset_blocked - check status of MNG FW veto bit
 * @hw: pointer to the hardware structure
 *
 * This function checks the MMNGC.MNG_VETO bit to see if there are
 * any constraints on link from manageability.  For MAC's that don't
 * have this bit just return faluse since the link can not be blocked
 * via this method.
 **/
bool ngbe_check_reset_blocked(struct ngbe_hw *hw)
{
	u32 mmngc;

	DEBUGFUNC("ngbe_check_reset_blocked");

	mmngc = rd32(hw, NGBE_MIS_ST);
	if (mmngc & NGBE_MIS_ST_MNG_VETO) {
		ERROR_REPORT1(NGBE_ERROR_SOFTWARE,
			      "MNG_VETO bit detected.\n");
		return true;
	}

	return false;
}



/* For internal phy only */
s32 ngbe_phy_read_reg(	struct ngbe_hw *hw,
						u32 reg_offset,
						u32 page,
						u16 *phy_data)
{
	bool page_select = false;

	/* clear input */
	*phy_data = 0;

	if (0 != page) {
		/* select page */
		if (0xa42 == page || 0xa43 == page || 0xa46 == page || 0xa47 == page ||
			0xd04 == page) {
			wr32(hw, NGBE_PHY_CONFIG(NGBE_INTERNAL_PHY_PAGE_SELECT_OFFSET),
					page);
			page_select = true;
		}
	}

	if (reg_offset >= NGBE_INTERNAL_PHY_OFFSET_MAX) {
		ERROR_REPORT1(NGBE_ERROR_UNSUPPORTED,
						"input reg offset %d exceed maximum 31.\n", reg_offset);
		return NGBE_ERR_INVALID_ARGUMENT;
	}

	*phy_data = 0xFFFF & rd32(hw, NGBE_PHY_CONFIG(reg_offset));

	if (page_select)
		wr32(hw, NGBE_PHY_CONFIG(NGBE_INTERNAL_PHY_PAGE_SELECT_OFFSET), 0);

	return NGBE_OK;
}

/* For internal phy only */
s32 ngbe_phy_write_reg(	struct ngbe_hw *hw,
						u32 reg_offset,
						u32 page,
						u16 phy_data)
{
	bool page_select = false;

	if (0 != page) {
		/* select page */
		if (0xa42 == page || 0xa43 == page || 0xa46 == page || 0xa47 == page ||
			0xd04 == page) {
			wr32(hw, NGBE_PHY_CONFIG(NGBE_INTERNAL_PHY_PAGE_SELECT_OFFSET),
					page);
			page_select = true;
		}
	}

	if (reg_offset >= NGBE_INTERNAL_PHY_OFFSET_MAX) {
		ERROR_REPORT1(NGBE_ERROR_UNSUPPORTED,
						"input reg offset %d exceed maximum 31.\n", reg_offset);
		return NGBE_ERR_INVALID_ARGUMENT;
	}

	wr32(hw, NGBE_PHY_CONFIG(reg_offset), phy_data);

	if (page_select)
		wr32(hw, NGBE_PHY_CONFIG(NGBE_INTERNAL_PHY_PAGE_SELECT_OFFSET), 0);

	return NGBE_OK;
}

s32 ngbe_check_internal_phy_id(struct ngbe_hw *hw)
{
	u16 phy_id_high = 0;
	u16 phy_id_low = 0;
	u16 phy_id = 0;

	DEBUGFUNC("ngbe_check_internal_phy_id");

	ngbe_phy_read_reg(hw, NGBE_MDI_PHY_ID1_OFFSET, 0, &phy_id_high);
	phy_id = phy_id_high << 6;
	ngbe_phy_read_reg(hw, NGBE_MDI_PHY_ID2_OFFSET, 0, &phy_id_low);
	phy_id |= (phy_id_low & NGBE_MDI_PHY_ID_MASK) >> 10;

	if (NGBE_INTERNAL_PHY_ID != phy_id) {
		ERROR_REPORT1(NGBE_ERROR_UNSUPPORTED,
					"internal phy id 0x%x not supported.\n", phy_id);
		return NGBE_ERR_DEVICE_NOT_SUPPORTED;
	} else
		hw->phy.id = (u32)phy_id;

	return NGBE_OK;
}


/**
 *  ngbe_read_phy_mdi - Reads a value from a specified PHY register without
 *  the SWFW lock
 *  @hw: pointer to hardware structure
 *  @reg_addr: 32 bit address of PHY register to read
 *  @phy_data: Pointer to read data from PHY register
 **/
s32 ngbe_phy_read_reg_mdi(	struct ngbe_hw *hw,
							u32 reg_addr,
							u32 device_type,
							u16 *phy_data)
{
	u32 command;
	s32 status = 0;

	/* setup and write the address cycle command */
	command = NGBE_MSCA_RA(reg_addr) |
		NGBE_MSCA_PA(hw->phy.addr) |
		NGBE_MSCA_DA(device_type);
	wr32(hw, NGBE_MSCA, command);

	command = NGBE_MSCC_CMD(NGBE_MSCA_CMD_READ) |
			  NGBE_MSCC_BUSY |
			  NGBE_MDIO_CLK(6);
	wr32(hw, NGBE_MSCC, command);

	/* wait to complete */
	status = po32m(hw, NGBE_MSCC,
		NGBE_MSCC_BUSY, ~NGBE_MSCC_BUSY,
		NGBE_MDIO_TIMEOUT, 10);
	if (status != 0) {
		ERROR_REPORT1(NGBE_ERROR_POLLING,
			      "PHY address command did not complete.\n");
		return NGBE_ERR_PHY;
	}

	/* read data from MSCC */
	*phy_data = 0xFFFF & rd32(hw, NGBE_MSCC);

	return 0;
}

/**
 *  ngbe_write_phy_reg_mdi - Writes a value to specified PHY register
 *  without SWFW lock
 *  @hw: pointer to hardware structure
 *  @reg_addr: 32 bit PHY register to write
 *  @device_type: 5 bit device type
 *  @phy_data: Data to write to the PHY register
 **/
s32 ngbe_phy_write_reg_mdi(	struct ngbe_hw *hw,
							u32 reg_addr,
							u32 device_type,
							u16 phy_data)
{
	u32 command;
	s32 status = 0;

	/* setup and write the address cycle command */
	command = NGBE_MSCA_RA(reg_addr) |
		NGBE_MSCA_PA(hw->phy.addr) |
		NGBE_MSCA_DA(device_type);
	wr32(hw, NGBE_MSCA, command);

	command = phy_data | NGBE_MSCC_CMD(NGBE_MSCA_CMD_WRITE) |
		  NGBE_MSCC_BUSY | NGBE_MDIO_CLK(6);
	wr32(hw, NGBE_MSCC, command);

	/* wait to complete */
	status = po32m(hw, NGBE_MSCC,
		NGBE_MSCC_BUSY, ~NGBE_MSCC_BUSY,
		NGBE_MDIO_TIMEOUT, 10);
	if (status != 0) {
		ERROR_REPORT1(NGBE_ERROR_POLLING,
			      "PHY address command did not complete.\n");
		return NGBE_ERR_PHY;
	}

	return 0;
}

s32 ngbe_check_mdi_phy_id(struct ngbe_hw *hw)
{
	u16 phy_id_high = 0;
	u16 phy_id_low = 0;
	u32 phy_id = 0;

	DEBUGFUNC("ngbe_check_mdi_phy_id");

	/* select page 0 */
	ngbe_phy_write_reg_mdi(hw, 22, 0, 0);

	ngbe_phy_read_reg_mdi(hw, NGBE_MDI_PHY_ID1_OFFSET, 0, &phy_id_high);
	phy_id = phy_id_high << 6;
	ngbe_phy_read_reg_mdi(hw, NGBE_MDI_PHY_ID2_OFFSET, 0, &phy_id_low);
	phy_id |= (phy_id_low & NGBE_MDI_PHY_ID_MASK) >> 10;

	if (NGBE_M88E1512_PHY_ID != phy_id) {
		ERROR_REPORT1(NGBE_ERROR_UNSUPPORTED,
					"MDI phy id 0x%x not supported.\n", phy_id);
		return NGBE_ERR_DEVICE_NOT_SUPPORTED;
	} else
		hw->phy.id = phy_id;

	return NGBE_OK;
}

s32 ngbe_check_zte_phy_id(struct ngbe_hw *hw)
{
	u16 phy_id_high = 0;
	u16 phy_id_low = 0;
	u16 phy_id = 0;

	DEBUGFUNC("ngbe_check_zte_phy_id");

	ngbe_phy_read_reg_mdi(hw, NGBE_MDI_PHY_ID1_OFFSET, 0, &phy_id_high);
	phy_id = phy_id_high << 6;
	ngbe_phy_read_reg_mdi(hw, NGBE_MDI_PHY_ID2_OFFSET, 0, &phy_id_low);
	phy_id |= (phy_id_low & NGBE_MDI_PHY_ID_MASK) >> 10;

	if (NGBE_INTERNAL_PHY_ID != phy_id) {
		ERROR_REPORT1(NGBE_ERROR_UNSUPPORTED,
					"MDI phy id 0x%x not supported.\n", phy_id);
		return NGBE_ERR_DEVICE_NOT_SUPPORTED;
	} else
		hw->phy.id = (u32)phy_id;

	return NGBE_OK;
}

/**
 *  ngbe_init_phy_ops - PHY/SFP specific init
 *  @hw: pointer to hardware structure
 *
 *  Initialize any function pointers that were not able to be
 *  set during init_shared_code because the PHY/SFP type was
 *  not known.  Perform the SFP init if necessary.
 *
**/
s32 ngbe_phy_init(struct ngbe_hw *hw)
{
	s32 ret_val = 0;
	u16 value = 0;
	int i;

	DEBUGFUNC("\n");

	/* set fwsw semaphore mask for phy first */
	if (!hw->phy.phy_semaphore_mask) {
		hw->phy.phy_semaphore_mask = NGBE_MNG_SWFW_SYNC_SW_PHY;
	}

	/* init phy.addr according to HW design */
	if (hw->bus.lan_id == 0)
		hw->phy.addr = 3;
	else
		hw->phy.addr = 0;

	/* Identify the PHY or SFP module */
	ret_val = TCALL(hw, phy.ops.identify);
	if (ret_val == NGBE_ERR_SFP_NOT_SUPPORTED)
		return ret_val;

	/* enable interrupts, only link status change and an done is allowed */
	if (hw->phy.type == ngbe_phy_internal) {
		value = NGBE_INTPHY_INT_LSC | NGBE_INTPHY_INT_ANC;
		TCALL(hw, phy.ops.write_reg, 0x12, 0xa42, value);
	} else if (hw->phy.type == ngbe_phy_m88e1512) {
		ret_val = TCALL(hw, phy.ops.reset);
		if (ret_val) {
			return ret_val;
		}

		TCALL(hw, phy.ops.write_reg_mdi, 22, 0, 2);
		TCALL(hw, phy.ops.read_reg_mdi, 21, 0, &value);
		value &= ~NGBE_M88E1512_RGM_TTC;
		value |= NGBE_M88E1512_RGM_RTC;
		TCALL(hw, phy.ops.write_reg_mdi, 21, 0, value);
		TCALL(hw, phy.ops.write_reg_mdi, 22, 0, 0);
		TCALL(hw, phy.ops.write_reg_mdi, 0, 0, NGBE_MDI_PHY_RESET);
		for (i = 0; i < 15; i++) {
			TCALL(hw, phy.ops.read_reg_mdi, 0, 0, &value);
			if (value & NGBE_MDI_PHY_RESET)
				msleep(1);
			else
				break;
		}

		if (i == 15) {
			ERROR_REPORT1(NGBE_ERROR_POLLING,
					"phy reset exceeds maximum waiting period.\n");
			return NGBE_ERR_PHY_TIMEOUT;
		}

		/* set LED2 to interrupt output and INTn active low */
		TCALL(hw, phy.ops.write_reg_mdi, 22, 0, 3);
		TCALL(hw, phy.ops.read_reg_mdi, 18, 0, &value);
		value |= NGBE_M88E1512_INT_EN;
		value &= ~(NGBE_M88E1512_INT_POL);
		TCALL(hw, phy.ops.write_reg_mdi, 18, 0, value);
		/* enable link status change and AN complete interrupts */
		value = NGBE_M88E1512_INT_ANC | NGBE_M88E1512_INT_LSC;
		TCALL(hw, phy.ops.write_reg_mdi, 22, 0, 0);
		TCALL(hw, phy.ops.write_reg_mdi, 18, 0, value);

		/* LED control */
		TCALL(hw, phy.ops.write_reg_mdi, 22, 0, 3);
		TCALL(hw, phy.ops.read_reg_mdi, 16, 0, &value);
		value &= ~0x00FF;
		value |= (NGBE_M88E1512_LED1_CONF << 4) | NGBE_M88E1512_LED0_CONF;
		TCALL(hw, phy.ops.write_reg_mdi, 16, 0, value);
		TCALL(hw, phy.ops.read_reg_mdi, 17, 0, &value);
		value &= ~0x000F;
		value |= (NGBE_M88E1512_LED1_POL << 2) | NGBE_M88E1512_LED0_POL;
		TCALL(hw, phy.ops.write_reg_mdi, 17, 0, value);
	}

	return ret_val;
}


/**
 *  ngbe_identify_module - Identifies module type
 *  @hw: pointer to hardware structure
 *
 *  Determines HW type and calls appropriate function.
 **/
s32 ngbe_phy_identify(struct ngbe_hw *hw)
{
	s32 status = 0;

	DEBUGFUNC("ngbe_phy_identify");

	switch(hw->phy.type) {
		case ngbe_phy_internal:
			status = ngbe_check_internal_phy_id(hw);
			break;
		case ngbe_phy_m88e1512:
			status = ngbe_check_mdi_phy_id(hw);
			break;
		case ngbe_phy_zte:
			status = ngbe_check_zte_phy_id(hw);
			break;
		default:
			status = NGBE_ERR_PHY_TYPE;
	}

	return status;
}

s32 ngbe_phy_reset(struct ngbe_hw *hw)
{
	s32 status = 0;

	u16 value = 0;
	int i;

	DEBUGFUNC("ngbe_phy_reset");

	/* only support internal phy */
	if (hw->phy.type != ngbe_phy_internal)
		return NGBE_ERR_PHY_TYPE;

	/* Don't reset PHY if it's shut down due to overtemp. */
	if (!hw->phy.reset_if_overtemp &&
		(NGBE_ERR_OVERTEMP == TCALL(hw, phy.ops.check_overtemp))) {
		ERROR_REPORT1(NGBE_ERROR_CAUTION,
						"OVERTEMP! Skip PHY reset.\n");
		return NGBE_ERR_OVERTEMP;
	}

	/* Blocked by MNG FW so bail */
	if (ngbe_check_reset_blocked(hw))
		return NGBE_ERR_MNG_ACCESS_FAILED;

	value |= NGBE_MDI_PHY_RESET;
	status = TCALL(hw, phy.ops.write_reg, 0, 0, value);
	for (i = 0; i < NGBE_PHY_RST_WAIT_PERIOD; i++) {
		status = TCALL(hw, phy.ops.read_reg, 0, 0, &value);
		if (!(value & NGBE_MDI_PHY_RESET))
			break;
		msleep(1);
	}

	if (i == NGBE_PHY_RST_WAIT_PERIOD) {
		ERROR_REPORT1(NGBE_ERROR_POLLING,
						"PHY MODE RESET did not complete.\n");
		return NGBE_ERR_RESET_FAILED;
	}

	return status;
}

u32 ngbe_phy_setup_link(struct ngbe_hw *hw,
						u32 speed,
						bool autoneg_wait_to_complete)
{
	u16 value = 0;

	DEBUGFUNC("ngbe_phy_setup_link");

	UNREFERENCED_PARAMETER(autoneg_wait_to_complete);




	/* disable 10/100M Half Duplex */
	TCALL(hw, phy.ops.read_reg, 4, 0, &value);
	value &= 0xFF5F;
	TCALL(hw, phy.ops.write_reg, 4, 0, value);

	/* set advertise enable according to input speed */
	if (!(speed & NGBE_LINK_SPEED_1GB_FULL)) {
		TCALL(hw, phy.ops.read_reg, 9, 0, &value);
		value &= 0xFDFF;
		TCALL(hw, phy.ops.write_reg, 9, 0, value);
	} else {
		TCALL(hw, phy.ops.read_reg, 9, 0, &value);
		value |= 0x200;
		TCALL(hw, phy.ops.write_reg, 9, 0, value);
	}

	if (!(speed & NGBE_LINK_SPEED_100_FULL)) {
		TCALL(hw, phy.ops.read_reg, 4, 0, &value);
		value &= 0xFEFF;
		TCALL(hw, phy.ops.write_reg, 4, 0, value);
	} else {
		TCALL(hw, phy.ops.read_reg, 4, 0, &value);
		value |= 0x100;
		TCALL(hw, phy.ops.write_reg, 4, 0, value);
	}

	if (!(speed & NGBE_LINK_SPEED_10_FULL)) {
		TCALL(hw, phy.ops.read_reg, 4, 0, &value);
		value &= 0xFFBF;
		TCALL(hw, phy.ops.write_reg, 4, 0, value);
	} else {
		TCALL(hw, phy.ops.read_reg, 4, 0, &value);
		value |= 0x40;
		TCALL(hw, phy.ops.write_reg, 4, 0, value);
	}

	/* restart AN and wait AN done interrupt */
	value = NGBE_MDI_PHY_RESTART_AN | NGBE_MDI_PHY_ANE;
	TCALL(hw, phy.ops.write_reg, 0, 0, value);

	value = 0x205B;
	TCALL(hw, phy.ops.write_reg, 16, 0xd04, value);
	TCALL(hw, phy.ops.write_reg, 17, 0xd04, 0);

	TCALL(hw, phy.ops.read_reg, 18, 0xd04, &value);
	value = value & 0xFFFC;
	value |= 0x2;
	TCALL(hw, phy.ops.write_reg, 18, 0xd04, value);

	return NGBE_OK;
}

s32 ngbe_phy_reset_m88e1512(struct ngbe_hw *hw)
{
	s32 status = 0;

	u16 value = 0;
	int i;

	DEBUGFUNC("ngbe_phy_reset_m88e1512");

	if (hw->phy.type != ngbe_phy_m88e1512)
		return NGBE_ERR_PHY_TYPE;

	/* Don't reset PHY if it's shut down due to overtemp. */
	if (!hw->phy.reset_if_overtemp &&
		(NGBE_ERR_OVERTEMP == TCALL(hw, phy.ops.check_overtemp))) {
		ERROR_REPORT1(NGBE_ERROR_CAUTION,
						"OVERTEMP! Skip PHY reset.\n");
		return NGBE_ERR_OVERTEMP;
	}

	/* Blocked by MNG FW so bail */
	if (ngbe_check_reset_blocked(hw))
		return NGBE_ERR_MNG_ACCESS_FAILED;

	/* select page 18 reg 20 */
	status = TCALL(hw, phy.ops.write_reg_mdi, 22, 0, 18);
	/* mode select to RGMII-to-copper */
	value = 0;
	status = TCALL(hw, phy.ops.write_reg_mdi, 20, 0, value);
	/* mode reset */
	value |= NGBE_MDI_PHY_RESET;
	status = TCALL(hw, phy.ops.write_reg_mdi, 20, 0, value);

	for (i = 0; i < NGBE_PHY_RST_WAIT_PERIOD; i++) {
		status = TCALL(hw, phy.ops.read_reg_mdi, 20, 0, &value);
		if (!(value & NGBE_MDI_PHY_RESET))
			break;
		msleep(1);
	}

	if (i == NGBE_PHY_RST_WAIT_PERIOD) {
		ERROR_REPORT1(NGBE_ERROR_POLLING,
						"M88E1512 MODE RESET did not complete.\n");
		return NGBE_ERR_RESET_FAILED;
	}

	return status;
}

u32 ngbe_phy_setup_link_m88e1512(	struct ngbe_hw *hw,
									u32 speed,
									bool autoneg_wait_to_complete)
{
	u16 value_r4 = 0;
	u16 value_r9 = 0;
	u16 value;

	DEBUGFUNC("\n");
	UNREFERENCED_PARAMETER(autoneg_wait_to_complete);

	hw->phy.autoneg_advertised = 0;

	if (speed & NGBE_LINK_SPEED_1GB_FULL) {
		value_r9 |=NGBE_M88E1512_1000BASET_FULL;
		hw->phy.autoneg_advertised |= NGBE_LINK_SPEED_1GB_FULL;
	}

	if (speed & NGBE_LINK_SPEED_100_FULL) {
		value_r4 |= NGBE_M88E1512_100BASET_FULL;
		hw->phy.autoneg_advertised |= NGBE_LINK_SPEED_100_FULL;
	}

	if (speed & NGBE_LINK_SPEED_10_FULL) {
		value_r4 |= NGBE_M88E1512_10BASET_FULL;
		hw->phy.autoneg_advertised |= NGBE_LINK_SPEED_10_FULL;
	}

	TCALL(hw, phy.ops.write_reg_mdi, 22, 0, 0);
	TCALL(hw, phy.ops.read_reg_mdi, 4, 0, &value);
	value &= ~(NGBE_M88E1512_100BASET_FULL |
			   NGBE_M88E1512_100BASET_HALF |
			   NGBE_M88E1512_10BASET_FULL |
			   NGBE_M88E1512_10BASET_HALF);
	value_r4 |= value;
	TCALL(hw, phy.ops.write_reg_mdi, 4, 0, value_r4);

	TCALL(hw, phy.ops.read_reg_mdi, 9, 0, &value);
	value &= ~(NGBE_M88E1512_1000BASET_FULL |
			   NGBE_M88E1512_1000BASET_HALF);
	value_r9 |= value;
	TCALL(hw, phy.ops.write_reg_mdi, 9, 0, value_r9);

	value = NGBE_MDI_PHY_RESTART_AN | NGBE_MDI_PHY_ANE;
	TCALL(hw, phy.ops.write_reg_mdi, 0, 0, value);

	TCALL(hw, phy.ops.check_event);


	return speed;
}

s32 ngbe_phy_reset_zte(struct ngbe_hw *hw)
{
	s32 status = 0;
    u16 value = 0;
	int i;

	DEBUGFUNC("ngbe_phy_reset_zte");

	if (hw->phy.type != ngbe_phy_zte)
		return NGBE_ERR_PHY_TYPE;

	/* Don't reset PHY if it's shut down due to overtemp. */
	if (!hw->phy.reset_if_overtemp &&
		(NGBE_ERR_OVERTEMP == TCALL(hw, phy.ops.check_overtemp))) {
		ERROR_REPORT1(NGBE_ERROR_CAUTION,
						"OVERTEMP! Skip PHY reset.\n");
		return NGBE_ERR_OVERTEMP;
	}

	/* Blocked by MNG FW so bail */
	if (ngbe_check_reset_blocked(hw))
		return NGBE_ERR_MNG_ACCESS_FAILED;

    /*zte phy*/
    /*set control register[0x0] to reset mode*/
    value = 1;
    /* mode reset */
	value |= NGBE_MDI_PHY_RESET;
	status = TCALL(hw, phy.ops.write_reg_mdi, 0, 0, value);

	for (i = 0; i < NGBE_PHY_RST_WAIT_PERIOD; i++) {
		status = TCALL(hw, phy.ops.read_reg_mdi, 0, 0, &value);
		if (!(value & NGBE_MDI_PHY_RESET))
			break;
		msleep(1);
	}

	if (i == NGBE_PHY_RST_WAIT_PERIOD) {
		ERROR_REPORT1(NGBE_ERROR_POLLING,
						"ZTE MODE RESET did not complete.\n");
		return NGBE_ERR_RESET_FAILED;
	}

	return status;
}

u32 ngbe_phy_setup_link_zte(struct ngbe_hw *hw,
							u32 speed,
							bool autoneg_wait_to_complete)
{
	u16 ngbe_phy_ccr = 0;

	DEBUGFUNC("\n");
	UNREFERENCED_PARAMETER(autoneg_wait_to_complete);
	/*
	 * Clear autoneg_advertised and set new values based on input link
	 * speed.
	 */
	hw->phy.autoneg_advertised = 0;
	TCALL(hw, phy.ops.read_reg_mdi, 0, 0, &ngbe_phy_ccr);

	if (speed & NGBE_LINK_SPEED_1GB_FULL) {
		hw->phy.autoneg_advertised |= NGBE_LINK_SPEED_1GB_FULL;
		ngbe_phy_ccr |= NGBE_MDI_PHY_SPEED_SELECT1;/*bit 6*/
	}
	else if (speed & NGBE_LINK_SPEED_100_FULL) {
		hw->phy.autoneg_advertised |= NGBE_LINK_SPEED_100_FULL;
		ngbe_phy_ccr |= NGBE_MDI_PHY_SPEED_SELECT0;/*bit 13*/
	}
	else if (speed & NGBE_LINK_SPEED_10_FULL)
		hw->phy.autoneg_advertised |= NGBE_LINK_SPEED_10_FULL;
	else
		return NGBE_LINK_SPEED_UNKNOWN;

	ngbe_phy_ccr |= NGBE_MDI_PHY_DUPLEX;/*restart autonegotiation*/
	TCALL(hw, phy.ops.write_reg_mdi, 0, 0, ngbe_phy_ccr);

	return speed;
}

/**
 *  ngbe_tn_check_overtemp - Checks if an overtemp occurred.
 *  @hw: pointer to hardware structure
 *
 *  Checks if the LASI temp alarm status was triggered due to overtemp
 **/
s32 ngbe_phy_check_overtemp(struct ngbe_hw *hw)
{
	s32 status = 0;
	u32 ts_state;

	DEBUGFUNC("ngbe_phy_check_overtemp");

	/* Check that the LASI temp alarm status was triggered */
	ts_state = rd32(hw, NGBE_TS_ALARM_ST);

	if (ts_state & NGBE_TS_ALARM_ST_DALARM)
		status = NGBE_ERR_UNDERTEMP;
	else if (ts_state & NGBE_TS_ALARM_ST_ALARM)
		status = NGBE_ERR_OVERTEMP;

	return status;
}

s32 ngbe_phy_check_event(struct ngbe_hw *hw)
{
	u16 value = 0;
	struct ngbe_adapter *adapter = hw->back;

	TCALL(hw, phy.ops.read_reg, 0x1d, 0xa43, &value);
	if (value & 0x10) {
		adapter->flags |= NGBE_FLAG_NEED_LINK_UPDATE;
	} else if (value & 0x08) {
		adapter->flags |= NGBE_FLAG_NEED_ANC_CHECK;
	}

	return NGBE_OK;
}

s32 ngbe_phy_check_event_m88e1512(struct ngbe_hw *hw)
{
	u16 value = 0;
	struct ngbe_adapter *adapter = hw->back;

	TCALL(hw, phy.ops.write_reg_mdi, 22, 0, 0);
	TCALL(hw, phy.ops.read_reg_mdi, 19, 0, &value);

	if (value & NGBE_M88E1512_LSC) {
		adapter->flags |= NGBE_FLAG_NEED_LINK_UPDATE;
	}

	if (value & NGBE_M88E1512_ANC) {
		adapter->flags |= NGBE_FLAG_NEED_ANC_CHECK;
	}

	return NGBE_OK;
}


s32 ngbe_phy_get_advertised_pause(struct ngbe_hw *hw, u8 *pause_bit)
{
	u16 value;
	s32 status = 0;

	status = TCALL(hw, phy.ops.read_reg, 4, 0, &value);
	*pause_bit = (u8)((value >> 10) & 0x3);
	return status;
}

s32 ngbe_phy_get_advertised_pause_m88e1512(struct ngbe_hw *hw, u8 *pause_bit)
{
	u16 value;
	s32 status = 0;

	status = TCALL(hw, phy.ops.write_reg_mdi, 22, 0, 0);
	status = TCALL(hw, phy.ops.read_reg_mdi, 4, 0, &value);
	*pause_bit = (u8)((value >> 10) & 0x3);
	return status;
}


s32 ngbe_phy_get_lp_advertised_pause(struct ngbe_hw *hw, u8 *pause_bit)
{
	u16 value;
	s32 status = 0;

	status = TCALL(hw, phy.ops.read_reg, 0x1d, 0xa43, &value);

	status = TCALL(hw, phy.ops.read_reg, 0x1, 0, &value);
	value = (value >> 5) & 0x1;

    /* if AN complete then check lp adv pause */
	status = TCALL(hw, phy.ops.read_reg, 5, 0, &value);
	*pause_bit = (u8)((value >> 10) & 0x3);
	return status;
}

s32 ngbe_phy_get_lp_advertised_pause_m88e1512(struct ngbe_hw *hw,
												u8 *pause_bit)
{
	u16 value;
	s32 status = 0;

	status = TCALL(hw, phy.ops.write_reg_mdi, 22, 0, 0);
	status = TCALL(hw, phy.ops.read_reg_mdi, 5, 0, &value);
	*pause_bit = (u8)((value >> 10) & 0x3);
	return status;
}

s32 ngbe_phy_set_pause_advertisement(struct ngbe_hw *hw, u16 pause_bit)
{
	u16 value;
	s32 status = 0;

	status = TCALL(hw, phy.ops.read_reg, 4, 0, &value);
	value &= ~0xC00;
	value |= pause_bit;

	status = TCALL(hw, phy.ops.write_reg, 4, 0, value);
	return status;
}

s32 ngbe_phy_set_pause_advertisement_m88e1512(struct ngbe_hw *hw,
												u16 pause_bit)
{
	u16 value;
	s32 status = 0;

	status = TCALL(hw, phy.ops.write_reg_mdi, 22, 0, 0);
	status = TCALL(hw, phy.ops.read_reg_mdi, 4, 0, &value);
	value &= ~0xC00;
	value |= pause_bit;

	status = TCALL(hw, phy.ops.write_reg_mdi, 4, 0, value);
	return status;
}

s32 ngbe_phy_setup(struct ngbe_hw *hw)
{
	int i;
	u16 value = 0;

	for (i = 0;i < 15;i++) {
		if (!rd32m(hw, NGBE_MIS_ST, NGBE_MIS_ST_GPHY_IN_RST(hw->bus.lan_id))) {
			break;
		}
		msleep(1);
	}

	if (i == 15) {
		ERROR_REPORT1(NGBE_ERROR_POLLING,
						"GPhy reset exceeds maximum times.\n");
		return NGBE_ERR_PHY_TIMEOUT;
	}

	for (i = 0; i<1000;i++) {
		TCALL(hw, phy.ops.read_reg, 29, 0xa43, &value);
		if (value & 0x20)
			break;
	}
	if (i == 1000){
			ERROR_REPORT1(NGBE_ERROR_POLLING,
						"efuse enable exceeds 1000 tries.\n");
	}

	TCALL(hw, phy.ops.write_reg, 20, 0xa46, 1);
	for (i = 0; i<1000;i++) {
		TCALL(hw, phy.ops.read_reg, 29, 0xa43, &value);
		if (value & 0x20)
			break;
	}
	if (i == 1000) {
		ERROR_REPORT1(NGBE_ERROR_POLLING,
						"efuse config exceeds maximum 1000 tries.\n");
		return NGBE_ERR_PHY_TIMEOUT;
	}

	TCALL(hw, phy.ops.write_reg, 20, 0xa46, 2);
	for (i = 0; i<1000;i++) {
		TCALL(hw, phy.ops.read_reg, 29, 0xa43, &value);
		if (value & 0x20)
			break;
	}

	if (i == 1000) {
		ERROR_REPORT1(NGBE_ERROR_POLLING,
						"efuse enable exceeds maximum 1000 tries.\n");
		return NGBE_ERR_PHY_TIMEOUT;

	}

	for (i = 0; i<1000;i++) {
		TCALL(hw, phy.ops.read_reg, 16, 0xa42, &value);
		if ((value & 0x7) == 3)
			break;
	}

	if (i == 1000) {
		ERROR_REPORT1(NGBE_ERROR_POLLING,
						"wait Lan on exceeds maximum 1000 tries.\n");
		return NGBE_ERR_PHY_TIMEOUT;
	}

	return NGBE_OK;
}

s32 ngbe_init_phy_ops_common(struct ngbe_hw *hw)
{
	struct ngbe_phy_info *phy = &hw->phy;

	phy->ops.reset = ngbe_phy_reset;
	phy->ops.read_reg = ngbe_phy_read_reg;
	phy->ops.write_reg = ngbe_phy_write_reg;
	phy->ops.setup_link = ngbe_phy_setup_link;
	phy->ops.check_overtemp = ngbe_phy_check_overtemp;
	phy->ops.identify = ngbe_phy_identify;
	phy->ops.init = ngbe_phy_init;
	phy->ops.check_event = ngbe_phy_check_event;
	phy->ops.get_adv_pause = ngbe_phy_get_advertised_pause;
	phy->ops.get_lp_adv_pause = ngbe_phy_get_lp_advertised_pause;
	phy->ops.set_adv_pause = ngbe_phy_set_pause_advertisement;
	phy->ops.setup_once = ngbe_phy_setup;

	return NGBE_OK;
}
