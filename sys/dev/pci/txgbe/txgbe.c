/*
 * Copyright (C) 2013 Michael Brown <mbrown@fensystems.co.uk>.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 *
 * You can also choose to distribute this program under the terms of
 * the Unmodified Binary Distribution Licence (as given in the file
 * COPYING.UBDL), provided that you have satisfied its requirements.
 */

FILE_LICENCE ( GPL2_OR_LATER_OR_UBDL );
#ifndef PMON
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <byteswap.h>
#include <ipxe/netdevice.h>
#include <ipxe/ethernet.h>
#include <ipxe/if_ether.h>
#include <ipxe/iobuf.h>
#include <ipxe/malloc.h>
#include <ipxe/pci.h>
#include <ipxe/profile.h>
#include "txgbe.h"
#else
#include "txgbe_pmon_head.h"
#endif

/** @file
 *
 * Sapphire 10 Gigabit Ethernet network card driver
 *
 */
#ifndef PMON
/** VM transmit profiler */
static struct profiler sapphire_vm_tx_profiler __profiler =
	{ .name = "sapphire.vm_tx" };

/** VM receive refill profiler */
static struct profiler sapphire_vm_refill_profiler __profiler =
	{ .name = "sapphire.vm_refill" };

/** VM poll profiler */
static struct profiler sapphire_vm_poll_profiler __profiler =
	{ .name = "sapphire.vm_poll" };
#endif

/**
 * Disable descriptor ring
 *
 * @v sapphire		Sapphire device
 * @v reg		Register block
 * @ret rc		Return status code
 */
static int sapphire_disable_ring ( struct sapphire_nic *sapphire, unsigned int reg ) {
	uint32_t dctl;
	unsigned int i;

	/* Disable ring */
	dctl = readl ( sapphire->regs + reg + SAPPHIRE_DCTL );
	dctl &= ~SAPPHIRE_DCTL_ENABLE;
	writel (dctl, ( sapphire->regs + reg + SAPPHIRE_DCTL ) );

	/* Wait for disable to complete */
	for ( i = 0 ; i < SAPPHIRE_DISABLE_MAX_WAIT_MS ; i++ ) {

		/* Check if ring is disabled */
		dctl = readl ( sapphire->regs + reg + SAPPHIRE_DCTL );
		if ( ! ( dctl & SAPPHIRE_DCTL_ENABLE ) )
			return 0;

		/* Delay */
		mdelay ( 1 );
	}

	DBGC ( sapphire, "SAPPHIRE %p ring %05x timed out waiting for disable "
	       "(dctl %08x)\n", sapphire, reg, dctl );
	return -ETIMEDOUT;
}


/**
 * Reset descriptor ring
 *
 * @v sapphire		Sapphire device
 * @v reg		Register block
 * @ret rc		Return status code
 */
void sapphire_reset_ring ( struct sapphire_nic *sapphire, unsigned int reg ) {

	/* Disable ring.  Ignore errors and continue to reset the ring anyway */
	sapphire_disable_ring ( sapphire, reg );

	/* Clear ring length */
//	writel ( 0, ( sapphire->regs + reg + SAPPHIRE_DLEN ) );

	/* Clear ring address */
	writel ( 0, ( sapphire->regs + reg + SAPPHIRE_DBAH ) );
	writel ( 0, ( sapphire->regs + reg + SAPPHIRE_DBAL ) );

	/* Reset head and tail pointers */
	writel ( 0, ( sapphire->regs + reg + SAPPHIRE_DH ) );
	writel ( 0, ( sapphire->regs + reg + SAPPHIRE_DT ) );
}


/**
 * Create descriptor ring
 *
 * @v sapphire		Sapphire device
 * @v ring		Descriptor ring
 * @ret rc		Return status code
 */
int sapphire_create_ring ( struct sapphire_nic *sapphire, struct sapphire_ring *ring,
			int is_tx_ring) {
	physaddr_t address;
	uint32_t dctl;

	/* Allocate descriptor ring.  Align ring on its own size to
	 * prevent any possible page-crossing errors due to hardware
	 * errata.
	 */
	ring->desc = malloc_dma ( ring->len, ring->len );
	if ( ! ring->desc )
		return -ENOMEM;

	/* Initialise descriptor ring */
	memset ( ring->desc, 0, ring->len );

	/* Program ring address */
	address = virt_to_bus ( ring->desc );
	writel ( ( address & 0xffffffffUL ),
		 ( sapphire->regs + ring->reg + SAPPHIRE_DBAL ) );
	if ( sizeof ( physaddr_t ) > sizeof ( uint32_t ) ) {
		writel ( ( ( ( uint64_t ) address ) >> 32 ),
			 ( sapphire->regs + ring->reg + SAPPHIRE_DBAH ) );
	} else {
		writel ( 0, sapphire->regs + ring->reg + SAPPHIRE_DBAH );
	}


	/* Reset head and tail pointers */
	writel ( 0, ( sapphire->regs + ring->reg + SAPPHIRE_DH ) );
	writel ( 0, ( sapphire->regs + ring->reg + SAPPHIRE_DT ) );

	/* Enable ring */
	if (is_tx_ring) {
		dctl = readl ( sapphire->regs + ring->reg + SAPPHIRE_DCTL );
		dctl |= SAPPHIRE_DCTL_ENABLE | 
			(ring->len / sizeof ( ring->desc[0] ) / 128) << SAPPHIRE_DCTL_RING_SIZE_SHIFT
			| 0x20 << SAPPHIRE_DCTL_TX_WTHRESH_SHIFT;
		writel ( dctl, sapphire->regs + ring->reg + SAPPHIRE_DCTL );
	} else {
		dctl = readl ( sapphire->regs + ring->reg + SAPPHIRE_DCTL );
		dctl |= SAPPHIRE_DCTL_ENABLE | 
			(ring->len / sizeof ( ring->desc[0] ) / 128) << SAPPHIRE_DCTL_RING_SIZE_SHIFT
			| 0x1 << SAPPHIRE_DCTL_RX_THER_SHIFT;
		dctl &= ~(SAPPHIRE_DCTL_RX_HDR_SZ |
			  SAPPHIRE_DCTL_RX_BUF_SZ |
			  SAPPHIRE_DCTL_RX_SPLIT_MODE);
		dctl |= SAPPHIRE_RX_HDR_SIZE << SAPPHIRE_DCTL_RX_BSIZEHDRSIZE_SHIFT;
		dctl |= SAPPHIRE_RX_BSIZE_DEFAULT >> SAPPHIRE_DCTL_RX_BSIZEPKT_SHIFT;
		writel ( dctl, sapphire->regs + ring->reg + SAPPHIRE_DCTL );		
	}

	DBGC ( sapphire, "SAPPHIRE %p ring %05x is at [%08llx,%08llx)\n",
	       sapphire, ring->reg, ( ( unsigned long long ) address ),
	       ( ( unsigned long long ) address + ring->len ) );

	return 0;
}


/******************************************************************************
 *
 * MAC address
 *
 ******************************************************************************
 */

/**
 * Try to fetch initial MAC address
 *
 * @v sapphire		Sapphire device
 * @v ral0		RAL0 register address
 * @v hw_addr		Hardware address to fill in
 * @ret rc		Return status code
 */
static int sapphire_try_fetch_mac ( struct sapphire_nic *sapphire, unsigned int ral0,
				  uint8_t *hw_addr ) {
	union sapphire_receive_address mac;
//	uint8_t raw[ETH_ALEN];
	uint8_t i;
	
	/* Read current address from RAL0/RAH0 */
	writel(0, sapphire->regs + SAPPHIRE_PSR_MAC_SWC_IDX);
	mac.reg.low = cpu_to_le32 ( readl ( sapphire->regs + ral0 ) );
	mac.reg.high = cpu_to_le32 ( readl ( sapphire->regs + ral0 + 4 ) );

	for (i = 0; i < 2; i++)
		hw_addr[i] = (u8)(mac.reg.high >> (1 - i) * 8);

	for (i = 0; i < 4; i++)
		hw_addr[i + 2] = (u8)(mac.reg.low >> (3 - i) * 8);
	
	/* Use current address if valid */
	if ( is_valid_ether_addr ( hw_addr ) ) {
		DBGC ( sapphire, "SAPPHIRE %p has autoloaded MAC address %s at "
		       "%#05x\n", sapphire, eth_ntoa ( hw_addr ), ral0 );
		//memcpy ( hw_addr, raw, ETH_ALEN );
		return 0;
	}

	return -ENOENT;
}

/**
 * Fetch initial MAC address
 *
 * @v sapphire		Sapphire device
 * @v hw_addr		Hardware address to fill in
 * @ret rc		Return status code
 */
static int sapphire_fetch_mac ( struct sapphire_nic *sapphire, uint8_t *hw_addr ) {
	int rc;

	/* Try to fetch address from RAL0 */
	if ( ( rc = sapphire_try_fetch_mac ( sapphire, SAPPHIRE_PSR_MAC_SWC_AD_L,
					   hw_addr ) ) == 0 ) {
		return 0;
	}

	DBGC ( sapphire, "SAPPHIRE %p has no MAC address to use\n", sapphire );
	return -ENOENT;
}

/******************************************************************************
 *
 * Device reset
 *
 ******************************************************************************
 */

/**
 * Reset hardware
 *
 * @v sapphire		Sapphire device
 * @ret rc		Return status code
 */
static int sapphire_reset ( struct sapphire_nic *sapphire ) {
	uint32_t ctrl;

	/* Perform a global software reset */
	ctrl = readl ( sapphire->regs + SAPPHIRE_MIS_RST);
	if (sapphire->port == 0)
		writel ( ( ctrl | SAPPHIRE_MIS_RST_LAN0_RST),
		 	sapphire->regs + SAPPHIRE_MIS_RST );
	else
		writel ( ( ctrl | SAPPHIRE_MIS_RST_LAN1_RST),
		 	sapphire->regs + SAPPHIRE_MIS_RST );
	mdelay ( SAPPHIRE_RESET_DELAY_MS );

	DBGC ( sapphire, "sapphire %p reset (%08x)\n", sapphire, ctrl );
	return 0;
}

/******************************************************************************
 *
 * Link state
 *
 ******************************************************************************
 */

/**
 * Check link state
 *
 * @v netdev		Network device
 */
static void sapphire_check_link ( struct net_device *netdev ) {
	struct sapphire_nic *sapphire = netdev->priv;
	uint32_t links;

	/* Read link status */
	links = readl ( sapphire->regs + SAPPHIRE_CFG_PORT_ST );
	DBGC ( sapphire, "SAPPHIRE %p link status is %08x\n", sapphire, links );

	/* Update network device */
	if ( links & SAPPHIRE_CFG_PORT_ST_LINK_UP) {
		netdev_link_up ( netdev );
	} else {
		netdev_link_down ( netdev );
	}
}

/**
 * Destroy descriptor ring
 *
 * @v sapphire		Sapphire device
 * @v ring		Descriptor ring
 */
void sapphire_destroy_ring ( struct sapphire_nic *sapphire, struct sapphire_ring *ring ) {

	/* Reset ring */
	sapphire_reset_ring ( sapphire, ring->reg );

	/* Free descriptor ring */
	free_dma ( ring->desc, ring->len );
	ring->desc = NULL;
	ring->prod = 0;
	ring->cons = 0;
}


/**
 * Refill receive descriptor ring
 *
 * @v sapphire		Sapphire device
 */
void sapphire_refill_rx ( struct sapphire_nic *sapphire ) {
	union sapphire_descriptor *rx;
	struct io_buffer *iobuf;
	unsigned int rx_idx;
	unsigned int rx_tail;
	physaddr_t address;
	unsigned int refilled = 0;

	/* Refill ring */
	while ( ( sapphire->rx.prod - sapphire->rx.cons ) < SAPPHIRE_RX_FILL ) {

		/* Allocate I/O buffer */
		iobuf = alloc_iob ( SAPPHIRE_RX_MAX_LEN );
		if ( ! iobuf ) {
			/* Wait for next refill */
			break;
		}

		/* Get next receive descriptor */
		rx_idx = ( sapphire->rx.prod++ % SAPPHIRE_NUM_RX_DESC );
		rx = &sapphire->rx.desc[rx_idx];

		/* Populate receive descriptor */
		address = virt_to_bus ( iobuf->data );
		sapphire->rx.describe ( rx, address, 0 );

		/* Record I/O buffer */
		assert ( sapphire->rx_iobuf[rx_idx] == NULL );
		sapphire->rx_iobuf[rx_idx] = iobuf;

		DBGC2 ( sapphire, "SAPPHIRE %p RX %d is [%llx,%llx)\n", sapphire, rx_idx,
			( ( unsigned long long ) address ),
			( ( unsigned long long ) address + SAPPHIRE_RX_MAX_LEN ) );
		refilled++;
	}

	/* Push descriptors to card, if applicable */
	if ( refilled ) {
		wmb();
		rx_tail = ( sapphire->rx.prod % SAPPHIRE_NUM_RX_DESC );
		profile_start ( &sapphire_vm_refill_profiler );
		writel ( rx_tail, sapphire->regs + sapphire->rx.reg + SAPPHIRE_DH );
		profile_stop ( &sapphire_vm_refill_profiler );
		profile_exclude ( &sapphire_vm_refill_profiler );
	}
}

/******************************************************************************
 *
 * Descriptors
 *
 ******************************************************************************
 */

/**
 * Populate advanced transmit descriptor
 *
 * @v tx		Transmit descriptor
 * @v addr		Data buffer address
 * @v len		Length of data
 */
void sapphire_describe_tx ( union sapphire_descriptor *tx, physaddr_t addr,
			     size_t len ) {

	/* Populate advanced transmit descriptor */
	tx->tx.address = cpu_to_le64 ( addr );
	tx->tx.length = cpu_to_le16 ( len );
	tx->tx.flags = 0;
	tx->tx.command = SAPPHIRE_TXD_DTYP_DATA | SAPPHIRE_TXD_RS|
			SAPPHIRE_TXD_IFCS | SAPPHIRE_TXD_EOP;
	tx->tx.status = cpu_to_le32 ( SAPPHIRE_DESC_STATUS_PAYLEN ( len ) );
}

/**
 * Populate receive descriptor
 *
 * @v rx		Receive descriptor
 * @v addr		Data buffer address
 * @v len		Length of data
 */
void sapphire_describe_rx ( union sapphire_descriptor *rx, physaddr_t addr,
			 size_t len __unused ) {

	/* Populate transmit descriptor */
	rx->rx.address = cpu_to_le64 ( addr );
	rx->rx.status_error = 0;
	rx->rx.length = 0;
	rx->rx.vlan = 0;
}

/**
 * Discard unused receive I/O buffers
 *
 * @v sapphire		Sapphire device
 */
void sapphire_empty_rx ( struct sapphire_nic *sapphire ) {
	unsigned int i;

	for ( i = 0 ; i < SAPPHIRE_NUM_RX_DESC ; i++ ) {
		if ( sapphire->rx_iobuf[i] )
			free_iob ( sapphire->rx_iobuf[i] );
		sapphire->rx_iobuf[i] = NULL;
	}
}

static inline uint32_t
sapphire_rd32_epcs(struct sapphire_nic *sapphire,
					uint32_t addr)
{
	unsigned int portRegOffset;
	uint32_t data;
	//Set the LAN port indicator to portRegOffset[1]
	//1st, write the regOffset to IDA_ADDR register
	portRegOffset = SAPPHIRE_XPCS_IDA_ADDR;
	writel(addr, sapphire->regs + portRegOffset);

	//2nd, read the data from IDA_DATA register
	portRegOffset = SAPPHIRE_XPCS_IDA_DATA;
	data = readl(sapphire->regs + portRegOffset);

	return data;
}

static inline void
sapphire_wr32_epcs(struct sapphire_nic *sapphire,
					uint32_t addr,
					uint32_t data)
{
	unsigned int portRegOffset;

	//Set the LAN port indicator to portRegOffset[1]
	//1st, write the regOffset to IDA_ADDR register
	portRegOffset = SAPPHIRE_XPCS_IDA_ADDR;
	writel(addr, sapphire->regs + portRegOffset);

	//2nd, read the data from IDA_DATA register
	portRegOffset = SAPPHIRE_XPCS_IDA_DATA;
	writel(data, sapphire->regs + portRegOffset);
}

static inline void
sapphire_wr32m(struct sapphire_nic *sapphire,
				uint32_t reg,
				uint32_t mask,
				uint32_t field)
{
	uint32_t val;

	val = readl(sapphire->regs + reg);
	if (val == SAPPHIRE_FAILED_READ_REG)
		return;

	val = ((val & ~mask) | (field & mask));
	writel(val, sapphire->regs + reg);
}

static inline int
sapphire_po32m(struct sapphire_nic *sapphire,
				uint32_t reg,
				uint32_t mask,
				uint32_t field,
				int usecs,
				int count)
{
	int loop;
	uint32_t value;

	loop = (count ? count : (usecs + 9) / 10);
	usecs = (loop ? (usecs + loop - 1) / loop : 0);

	count = loop;
	do {
		value = readl(sapphire->regs + reg);
		if ((value & mask) == (field & mask)) {
			break;
		}

		if (loop-- <= 0)
			break;

		usec_delay(usecs);
	} while (true);

	return (count - loop <= count ? 0 : SAPPHIRE_ERR_TIMEOUT);
}

void sapphire_init_i2c(struct sapphire_nic *sapphire)
{
	writel(0, sapphire->regs + SAPPHIRE_I2C_ENABLE);
	writel((SAPPHIRE_I2C_CON_MASTER_MODE |
		SAPPHIRE_I2C_CON_SPEED(1) | SAPPHIRE_I2C_CON_RESTART_EN |
		SAPPHIRE_I2C_CON_SLAVE_DISABLE), sapphire->regs + SAPPHIRE_I2C_CON);
	writel(SAPPHIRE_I2C_SLAVE_ADDR, sapphire->regs + SAPPHIRE_I2C_TAR);
	writel(600, sapphire->regs + SAPPHIRE_I2C_SS_SCL_HCNT);
	writel(600, sapphire->regs + SAPPHIRE_I2C_SS_SCL_LCNT);
	writel(0, sapphire->regs + SAPPHIRE_I2C_RX_TL); /* 1byte for rx full signal */
	writel(4, sapphire->regs + SAPPHIRE_I2C_TX_TL);
	writel(0xFFFFFF, sapphire->regs + SAPPHIRE_I2C_SCL_STUCK_TIMEOUT);
	writel(0xFFFFFF, sapphire->regs + SAPPHIRE_I2C_SDA_STUCK_TIMEOUT);

	writel(0, sapphire->regs + SAPPHIRE_I2C_INTR_MASK);
	writel(1, sapphire->regs + SAPPHIRE_I2C_ENABLE);
}

static int sapphire_read_i2c(struct sapphire_nic *sapphire,
								u8 byte_offset,
				  				u8 *data)
{
	int status = 0;

	/* wait tx empty */
	status = sapphire_po32m(sapphire, SAPPHIRE_I2C_RAW_INTR_STAT,
		SAPPHIRE_I2C_INTR_STAT_TX_EMPTY, SAPPHIRE_I2C_INTR_STAT_TX_EMPTY,
		SAPPHIRE_I2C_TIMEOUT, 10);
	if (status != 0)
		goto out;

	/* read data */
	writel(byte_offset | SAPPHIRE_I2C_DATA_CMD_STOP,
			sapphire->regs + SAPPHIRE_I2C_DATA_CMD);
	writel(SAPPHIRE_I2C_DATA_CMD_READ, sapphire->regs + SAPPHIRE_I2C_DATA_CMD);

	/* wait for read complete */
	status = sapphire_po32m(sapphire, SAPPHIRE_I2C_RAW_INTR_STAT,
		SAPPHIRE_I2C_INTR_STAT_RX_FULL, SAPPHIRE_I2C_INTR_STAT_RX_FULL,
		SAPPHIRE_I2C_TIMEOUT, 10);
	if (status != 0)
		goto out;

	*data = 0xFF & readl(sapphire->regs + SAPPHIRE_I2C_DATA_CMD);
out:
	return status;
}

int sapphire_identify_sfp(struct sapphire_nic *sapphire)
{
	int status = 0;
	u8 identifier = 0;
	u8 comp_codes_1g = 0;
	u8 comp_codes_10g = 0;
	u8 cable_tech = 0;


	sapphire_init_i2c(sapphire);
	status = sapphire_read_i2c(sapphire, SAPPHIRE_SFF_IDENTIFIER,
								&identifier);
	if (status != 0) {
		return SAPPHIRE_ERR_I2C;
	}

	if (identifier != SAPPHIRE_SFF_IDENTIFIER_SFP) {
		return SAPPHIRE_ERR_I2C_SFP_NOT_SUPPORTED;
	} else {
		status = sapphire_read_i2c(sapphire, SAPPHIRE_SFF_1GBE_COMP_CODES,
									&comp_codes_1g);
		if (status != 0)
			return SAPPHIRE_ERR_I2C;

		status = sapphire_read_i2c(sapphire, SAPPHIRE_SFF_10GBE_COMP_CODES,
									&comp_codes_10g);
		if (status != 0)
			return SAPPHIRE_ERR_I2C;

		status = sapphire_read_i2c(sapphire, SAPPHIRE_SFF_CABLE_TECHNOLOGY,
									&cable_tech);
		if (status != 0)
			return SAPPHIRE_ERR_I2C;
	}
	if (cable_tech & SAPPHIRE_SFF_DA_PASSIVE_CABLE) {
		sapphire->sfp_type = sapphire_sfp_type_da_cu_core0;
	} else {
		sapphire->sfp_type = sapphire_sfp_type_unknown;
	}
	return 0;
}


void sapphire_select_speed(struct sapphire_nic *sapphire,
					uint32_t speed)
{
	uint32_t esdp_reg = readl(sapphire->regs + SAPPHIRE_GPIO_DR);

	switch (speed) {
	case SAPPHIRE_LINK_SPEED_10GB_FULL:
			esdp_reg |= SAPPHIRE_GPIO_DR_5 | SAPPHIRE_GPIO_DR_4;
		break;
	case SAPPHIRE_LINK_SPEED_1GB_FULL:
			esdp_reg &= ~(SAPPHIRE_GPIO_DR_5 | SAPPHIRE_GPIO_DR_4);
		break;
	default:
		DBGC ( sapphire, "SAPPHIRE invalid module speed\n" );
		return;
	}

	writel( SAPPHIRE_GPIO_DDR_5 | SAPPHIRE_GPIO_DDR_4 |
			SAPPHIRE_GPIO_DDR_1 | SAPPHIRE_GPIO_DDR_0,
		sapphire->regs + SAPPHIRE_GPIO_DDR);

	writel(esdp_reg, sapphire->regs + SAPPHIRE_GPIO_DR);

	readl(sapphire->regs + SAPPHIRE_CFG_PORT_ST);
}

int sapphire_enable_rx_adapter(struct sapphire_nic *sapphire)
{
	uint32_t value;
	
	value = sapphire_rd32_epcs(sapphire, SAPPHIRE_PHY_RX_EQ_CTL);
	value |= 1 << 12;
	sapphire_wr32_epcs(sapphire, SAPPHIRE_PHY_RX_EQ_CTL, value);
	
	value = 0;
	while (!(value >> 11)) {
		value = sapphire_rd32_epcs(sapphire, SAPPHIRE_PHY_RX_AD_ACK);
		msleep(1);
	}
	
	value = sapphire_rd32_epcs(sapphire, SAPPHIRE_PHY_RX_EQ_CTL);
	value &= ~(1 << 12);
	sapphire_wr32_epcs(sapphire, SAPPHIRE_PHY_RX_EQ_CTL, value);
	
	return 0;

}

int sapphire_setup_link(struct sapphire_nic *sapphire,
						uint32_t speed)
{
	uint32_t i;
	uint32_t value = 0;
	int status = 0;

	//10G LINK has already been supportted by firmware
	if (speed != SAPPHIRE_LINK_SPEED_1GB_FULL &&
		speed != SAPPHIRE_LINK_SPEED_10GB_FULL)
		return SAPPHIRE_ERR_INVALID_PARAM;

	if (sapphire_identify_sfp(sapphire) != 0)
		return SAPPHIRE_ERR_NOT_SFP;

	sapphire_select_speed(sapphire, speed);

	for (i = 0; i < SAPPHIRE_XPCS_POWER_GOOD_MAX_POLLING_TIME; i++) {
		if ((sapphire_rd32_epcs(sapphire, SAPPHIRE_VR_XS_OR_PCS_MMD_DIGI_STATUS) &
			SAPPHIRE_VR_XS_OR_PCS_MMD_DIGI_STATUS_PSEQ_MASK) ==
			SAPPHIRE_VR_XS_OR_PCS_MMD_DIGI_STATUS_PSEQ_POWER_GOOD)
			break;
		msleep(10);
	}
	if (i == SAPPHIRE_XPCS_POWER_GOOD_MAX_POLLING_TIME) {
		return SAPPHIRE_ERR_TIMEOUT;
	}

	sapphire_wr32m(sapphire, SAPPHIRE_MAC_TX_CFG,
					SAPPHIRE_MAC_TX_CFG_TE,
					~SAPPHIRE_MAC_TX_CFG_TE);

	sapphire_wr32_epcs(sapphire, SAPPHIRE_SR_AN_MMD_CTL, 0x0);

	if (speed == SAPPHIRE_LINK_SPEED_1GB_FULL) {

		//@. Set SR PCS Control2 Register Bits[1:0] = 2'b00  //PCS_TYPE_SEL: KR
		sapphire_wr32_epcs(sapphire, SAPPHIRE_SR_PCS_CTL2, 0x1);
		//Set SR PMA MMD Control1 Register Bit[13] = 1'b0  //SS13: 1G speed
		sapphire_wr32_epcs(sapphire, SAPPHIRE_SR_PMA_MMD_CTL1, 0x0000);
		//Set SR MII MMD Control Register to corresponding speed:
		sapphire_wr32_epcs(sapphire, SAPPHIRE_SR_MII_MMD_CTL, 0x0140);

		value = sapphire_rd32_epcs(sapphire, SAPPHIRE_PHY_TX_GENCTRL1);
		value = (value & ~0x710) | 0x500;
		sapphire_wr32_epcs(sapphire, SAPPHIRE_PHY_TX_GENCTRL1, value);
		//4. Set VR_XS_PMA_Gen5_12G_MISC_CTRL0 Register Bit[12:8](RX_VREF_CTRL)
		//= 5'hF
		sapphire_wr32_epcs(sapphire, SAPPHIRE_PHY_MISC_CTL0, 0xCF00);
		//5. Set VR_XS_PMA_Gen5_12G_TX_EQ_CTRL0 Register Bit[13:8](TX_EQ_MAIN)
		//= 6'd30, Bit[5:0](TX_EQ_PRE) = 6'd4
		value = sapphire_rd32_epcs(sapphire, SAPPHIRE_PHY_TX_EQ_CTL0);
		value = (value & ~0x3F3F) | (24 << 8) | 4;
		sapphire_wr32_epcs(sapphire, SAPPHIRE_PHY_TX_EQ_CTL0, value);
		//6. Set VR_XS_PMA_Gen5_12G_TX_EQ_CTRL1 Register Bit[6](TX_EQ_OVR_RIDE)
		//= 1'b1, Bit[5:0](TX_EQ_POST) = 6'd36
		value = sapphire_rd32_epcs(sapphire, SAPPHIRE_PHY_TX_EQ_CTL1);
		value = (value & ~0x7F) | 16 | (1 << 6);
		sapphire_wr32_epcs(sapphire, SAPPHIRE_PHY_TX_EQ_CTL1, value);

		if (sapphire->sfp_type == sapphire_sfp_type_da_cu_core0) {
			sapphire_wr32_epcs(sapphire, SAPPHIRE_PHY_RX_EQ_CTL0, 0x774F);
		} else {
			//7. Set VR_XS_PMA_Gen5_12G_RX_EQ_CTRL0 Register Bit[15:8]
			//(VGA1/2_GAIN_0) = 8'h00, Bit[7:5](CTLE_POLE_0) = 3'h2,
			//Bit[4:0](CTLE_BOOST_0) = 4'hA
			value = sapphire_rd32_epcs(sapphire, SAPPHIRE_PHY_RX_EQ_CTL0);
			value = (value & ~0xFFFF) | 0x7706;
			sapphire_wr32_epcs(sapphire, SAPPHIRE_PHY_RX_EQ_CTL0, value);
		}

		value = sapphire_rd32_epcs(sapphire, SAPPHIRE_PHY_RX_EQ_ATT_LVL0);
		value = (value & ~0x7) | 0x0;
		sapphire_wr32_epcs(sapphire, SAPPHIRE_PHY_RX_EQ_ATT_LVL0, value);
		//8. Set VR_XS_PMA_Gen5_12G_DFE_TAP_CTRL0 Register Bit[7:0](DFE_TAP1_0)
		//= 8'd00
		sapphire_wr32_epcs(sapphire, SAPPHIRE_PHY_DFE_TAP_CTL0, 0x0);
		//Set VR_XS_PMA_Gen5_12G_RX_GENCTRL3 Register Bit[2:0] LOS_TRSHLD_0 = 4
		value = sapphire_rd32_epcs(sapphire, SAPPHIRE_PHY_RX_GEN_CTL3);
		value = (value & ~0x7) | 0x4;
		sapphire_wr32_epcs(sapphire, SAPPHIRE_PHY_RX_GEN_CTL3, value);
		//Set VR XS, PMA, or MII Synopsys Enterprise Gen5 12G PHY
		//MPLLA Control 0 Register Bit[7:0] = 8'd32  //MPLLA_MULTIPLIER
		sapphire_wr32_epcs(sapphire, SAPPHIRE_PHY_MPLLA_CTL0, 0x0020);
		//Set VR XS, PMA or MII Synopsys Enterprise Gen5 12G PHY MPLLA Control 3 Register Bit[10:0] = 11'd70  //MPLLA_BANDWIDTH
		sapphire_wr32_epcs(sapphire, SAPPHIRE_PHY_MPLLA_CTL3, 0x0046);
		//Set VR XS, PMA, or MII Synopsys Enterprise Gen5 12G PHY VCO Calibration Load 0 Register  Bit[12:0] = 13'd1344  //VCO_LD_VAL_0
		sapphire_wr32_epcs(sapphire, SAPPHIRE_PHY_VCO_CAL_LD0, 0x0540);
		//Set VR XS, PMA, or MII Synopsys Enterprise Gen5 12G PHY VCO Calibration Reference 0 Register Bit[5:0] = 6'd42  //VCO_REF_LD_0
		sapphire_wr32_epcs(sapphire, SAPPHIRE_PHY_VCO_CAL_REF0, 0x002A);
		//Set VR XS, PMA, or MII Synopsys Enterprise Gen5 12G PHY AFE-DFE Enable Register Bit[4], Bit[0] = 1'b0  //AFE_EN_0, DFE_EN_0
		sapphire_wr32_epcs(sapphire, SAPPHIRE_PHY_AFE_DFE_ENABLE, 0x0);
		//Set  VR XS, PMA, or MII Synopsys Enterprise Gen5 12G PHY Rx Equalization Control 4 Register Bit[0] = 1'b0  //CONT_ADAPT_0
		sapphire_wr32_epcs(sapphire, SAPPHIRE_PHY_RX_EQ_CTL, 0x0010);
		//Set VR XS, PMA, or MII Synopsys Enterprise Gen5 12G PHY Tx Rate Control Register Bit[2:0] = 3'b011  //TX0_RATE
		sapphire_wr32_epcs(sapphire, SAPPHIRE_PHY_TX_RATE_CTL, 0x0003);
		//Set VR XS, PMA, or MII Synopsys Enterprise Gen5 12G PHY Rx Rate Control Register Bit[2:0] = 3'b011 //RX0_RATE
		sapphire_wr32_epcs(sapphire, SAPPHIRE_PHY_RX_RATE_CTL, 0x0003);
		//Set VR XS, PMA, or MII Synopsys Enterprise Gen5 12G PHY Tx General Control 2 Register Bit[9:8] = 2'b01  //TX0_WIDTH: 10bits
		sapphire_wr32_epcs(sapphire, SAPPHIRE_PHY_TX_GEN_CTL2, 0x0100);
		//Set VR XS, PMA, or MII Synopsys Enterprise Gen5 12G PHY Rx General Control 2 Register Bit[9:8] = 2'b01  //RX0_WIDTH: 10bits
		sapphire_wr32_epcs(sapphire, SAPPHIRE_PHY_RX_GEN_CTL2, 0x0100);
		//Set VR XS, PMA, or MII Synopsys Enterprise Gen5 12G PHY MPLLA Control 2 Register Bit[10:8] = 3'b010  //MPLLA_DIV16P5_CLK_EN=0, MPLLA_DIV10_CLK_EN=1, MPLLA_DIV8_CLK_EN=0
		sapphire_wr32_epcs(sapphire, SAPPHIRE_PHY_MPLLA_CTL2, 0x0200);
		//VR MII MMD AN Control Register Bit[8] = 1'b1 //MII_CTRL
		sapphire_wr32_epcs(sapphire, SAPPHIRE_SR_MII_MMD_AN_CTL, 0x0100);

		sapphire->speed = SAPPHIRE_LINK_SPEED_1GB_FULL;
	} else {
		//@. Set SR PCS Control2 Register Bits[1:0] = 2'b00  //PCS_TYPE_SEL: KR
		sapphire_wr32_epcs(sapphire, SAPPHIRE_SR_PCS_CTL2,0);
		value = sapphire_rd32_epcs(sapphire, SAPPHIRE_SR_PMA_MMD_CTL1);
		value = value | 0x2000;
		sapphire_wr32_epcs(sapphire, SAPPHIRE_SR_PMA_MMD_CTL1, value);
		//@. Set VR_XS_PMA_Gen5_12G_MPLLA_CTRL0 Register Bit[7:0] = 8'd33
		//MPLLA_MULTIPLIER
		sapphire_wr32_epcs(sapphire, SAPPHIRE_PHY_MPLLA_CTL0, 0x0021);
		//3. Set VR_XS_PMA_Gen5_12G_MPLLA_CTRL3 Register
		//Bit[10:0](MPLLA_BANDWIDTH) = 11'd0
		sapphire_wr32_epcs(sapphire, SAPPHIRE_PHY_MPLLA_CTL3, 0);
		value = sapphire_rd32_epcs(sapphire, SAPPHIRE_PHY_TX_GENCTRL1);
		value = (value & ~0x700) | 0x500;
		sapphire_wr32_epcs(sapphire, SAPPHIRE_PHY_TX_GENCTRL1, value);
		//4. Set VR_XS_PMA_Gen5_12G_MISC_CTRL0 Register Bit[12:8](RX_VREF_CTRL)
		//= 5'hF
		sapphire_wr32_epcs(sapphire, SAPPHIRE_PHY_MISC_CTL0, 0xCF00);
		//@. Set VR_XS_PMA_Gen5_12G_VCO_CAL_LD0 Register  Bit[12:0] = 13'd1353
		//VCO_LD_VAL_0
		sapphire_wr32_epcs(sapphire, SAPPHIRE_PHY_VCO_CAL_LD0, 0x0549);
		//@. Set VR_XS_PMA_Gen5_12G_VCO_CAL_REF0 Register Bit[5:0] = 6'd41
		//VCO_REF_LD_0
		sapphire_wr32_epcs(sapphire, SAPPHIRE_PHY_VCO_CAL_REF0, 0x0029);
		//@. Set VR_XS_PMA_Gen5_12G_TX_RATE_CTRL Register Bit[2:0] = 3'b000
		//TX0_RATE
		sapphire_wr32_epcs(sapphire, SAPPHIRE_PHY_TX_RATE_CTL, 0);
		//@. Set VR_XS_PMA_Gen5_12G_RX_RATE_CTRL Register Bit[2:0] = 3'b000
		//RX0_RATE
		sapphire_wr32_epcs(sapphire, SAPPHIRE_PHY_RX_RATE_CTL, 0);
		//@. Set VR_XS_PMA_Gen5_12G_TX_GENCTRL2 Register Bit[9:8] = 2'b11
		//TX0_WIDTH: 20bits
		sapphire_wr32_epcs(sapphire, SAPPHIRE_PHY_TX_GEN_CTL2, 0x0300);
		//@. Set VR_XS_PMA_Gen5_12G_RX_GENCTRL2 Register Bit[9:8] = 2'b11
		//RX0_WIDTH: 20bits
		sapphire_wr32_epcs(sapphire, SAPPHIRE_PHY_RX_GEN_CTL2, 0x0300);
		//@. Set VR_XS_PMA_Gen5_12G_MPLLA_CTRL2 Register Bit[10:8] = 3'b110
		//MPLLA_DIV16P5_CLK_EN=1, MPLLA_DIV10_CLK_EN=1, MPLLA_DIV8_CLK_EN=0
		sapphire_wr32_epcs(sapphire, SAPPHIRE_PHY_MPLLA_CTL2, 0x0600);
		//5. Set VR_XS_PMA_Gen5_12G_TX_EQ_CTRL0 Register Bit[13:8](TX_EQ_MAIN)
		//= 6'd30, Bit[5:0](TX_EQ_PRE) = 6'd4
		value = sapphire_rd32_epcs(sapphire, SAPPHIRE_PHY_TX_EQ_CTL0);
		value = (value & ~0x3F3F) | (24 << 8) | 4;
		sapphire_wr32_epcs(sapphire, SAPPHIRE_PHY_TX_EQ_CTL0, value);
		//6. Set VR_XS_PMA_Gen5_12G_TX_EQ_CTRL1 Register Bit[6](TX_EQ_OVR_RIDE)
		//= 1'b1, Bit[5:0](TX_EQ_POST) = 6'd36
		value = sapphire_rd32_epcs(sapphire, SAPPHIRE_PHY_TX_EQ_CTL1);
		value = (value & ~0x7F) | 16 | (1 << 6);
		sapphire_wr32_epcs(sapphire, SAPPHIRE_PHY_TX_EQ_CTL1, value);
		if (sapphire->sfp_type == sapphire_sfp_type_da_cu_core0) {
			//7. Set VR_XS_PMA_Gen5_12G_RX_EQ_CTRL0 Register
			//Bit[15:8](VGA1/2_GAIN_0) = 8'h77, Bit[7:5]
			//(CTLE_POLE_0) = 3'h2, Bit[4:0](CTLE_BOOST_0) = 4'hF
			sapphire_wr32_epcs(sapphire, SAPPHIRE_PHY_RX_EQ_CTL0, 0x774F);

		} else {
			//7. Set VR_XS_PMA_Gen5_12G_RX_EQ_CTRL0 Register Bit[15:8]
			//(VGA1/2_GAIN_0) = 8'h00, Bit[7:5](CTLE_POLE_0) = 3'h2,
			//Bit[4:0](CTLE_BOOST_0) = 4'hA
			value = sapphire_rd32_epcs(sapphire, SAPPHIRE_PHY_RX_EQ_CTL0);
			value = (value & ~0xFFFF) | (2 << 5) | 0x05;
			sapphire_wr32_epcs(sapphire, SAPPHIRE_PHY_RX_EQ_CTL0, value);
		}
		value = sapphire_rd32_epcs(sapphire, SAPPHIRE_PHY_RX_EQ_ATT_LVL0);
		value = (value & ~0x7) | 0x0;
		sapphire_wr32_epcs(sapphire, SAPPHIRE_PHY_RX_EQ_ATT_LVL0, value);

		if (sapphire->sfp_type == sapphire_sfp_type_da_cu_core0) {
			//8. Set VR_XS_PMA_Gen5_12G_DFE_TAP_CTRL0 Register Bit[7:0](DFE_TAP1_0)
			//= 8'd20
			sapphire_wr32_epcs(sapphire, SAPPHIRE_PHY_DFE_TAP_CTL0, 0x0014);
			value = sapphire_rd32_epcs(sapphire, SAPPHIRE_PHY_AFE_DFE_ENABLE);
			value = (value & ~0x11) | 0x11;
			sapphire_wr32_epcs(sapphire, SAPPHIRE_PHY_AFE_DFE_ENABLE, value);
		} else {
			//8. Set VR_XS_PMA_Gen5_12G_DFE_TAP_CTRL0 Register Bit[7:0](DFE_TAP1_0)
			//= 8'd20
			sapphire_wr32_epcs(sapphire, SAPPHIRE_PHY_DFE_TAP_CTL0, 0xBE);
			//9. Set VR_MII_Gen5_12G_AFE_DFE_EN_CTRL Register Bit[4](DFE_EN_0) =
			//1'b0, Bit[0](AFE_EN_0) = 1'b0
			value = sapphire_rd32_epcs(sapphire, SAPPHIRE_PHY_AFE_DFE_ENABLE);
			value = (value & ~0x11) | 0x0;
			sapphire_wr32_epcs(sapphire, SAPPHIRE_PHY_AFE_DFE_ENABLE, value);
		}
		value = sapphire_rd32_epcs(sapphire, SAPPHIRE_PHY_RX_EQ_CTL);
		value = value & ~0x1;
		sapphire_wr32_epcs(sapphire, SAPPHIRE_PHY_RX_EQ_CTL, value);

		sapphire->speed = SAPPHIRE_LINK_SPEED_10GB_FULL;
	}
	//10. Initialize the mode by setting VR XS or PCS MMD Digital Control1
	//Register Bit[15](VR_RST)
	sapphire_wr32_epcs(sapphire, SAPPHIRE_VR_XS_OR_PCS_MMD_DIGI_CTL1, 0xA000);
	/* wait phy initialization done */
	for (i = 0; i < SAPPHIRE_PHY_INIT_DONE_POLLING_TIME; i++) {
		if ((sapphire_rd32_epcs(sapphire, SAPPHIRE_VR_XS_OR_PCS_MMD_DIGI_CTL1) &
			SAPPHIRE_VR_XS_OR_PCS_MMD_DIGI_CTL1_VR_RST) == 0)
			break;
		msleep(100);
	}
	if (i == SAPPHIRE_PHY_INIT_DONE_POLLING_TIME) {
		status = SAPPHIRE_ERR_TIMEOUT;
		goto out;
	}
	if (sapphire->sfp_type == sapphire_sfp_type_da_cu_core0) {
		msleep(500);
		status = sapphire_enable_rx_adapter(sapphire);
	}
out:
	return status;
}





/******************************************************************************
 *
 * Network device interface
 *
 ******************************************************************************
 */

/**
 * Open network device
 *
 * @v netdev		Network device
 * @ret rc		Return status code
 */
static int sapphire_open ( struct net_device *netdev ) {
	struct sapphire_nic *sapphire = netdev->priv;
	//union sapphire_receive_address mac;
	uint32_t dmatxctl;
	uint32_t mactxcfg;
	uint32_t fctrl;
	uint32_t hlreg0;
	uint32_t maxfrs;
	uint32_t rdrxctl;
	uint32_t rxctrl;
	uint32_t vlanctl;
	int rc;
	//int i;
	uint32_t flow_ctrl;
	uint32_t pkt_flt;

	writel(512 << 10, sapphire->regs + 0x19020);

	if ( ! ( readl ( sapphire->regs + SAPPHIRE_CFG_PORT_ST )
		& SAPPHIRE_CFG_PORT_ST_LINK_UP)) {
		sapphire_setup_link(sapphire, sapphire->speed );
	}

	/* Create transmit descriptor ring */
	if ( ( rc = sapphire_create_ring ( sapphire, &sapphire->tx, 1 ) ) != 0 )
		goto err_create_tx;

	/* Create receive descriptor ring */
	if ( ( rc = sapphire_create_ring ( sapphire, &sapphire->rx, 0 ) ) != 0 )
		goto err_create_rx;
#if 0
	/* Program MAC address */
	memset ( &mac, 0, sizeof ( mac ) );
	for (i = 0; i < 2; i++)
		netdev->ll_addr[i] = (u8)(mac.reg.high >> (1 - i) * 8);

	for (i = 0; i < 4; i++)
		netdev->ll_addr[i + 2] = (u8)(mac.reg.low >> (3 - i) * 8);
	memcpy(netdev->ll_addr, netdev->hw_addr, 6);
#endif
#if 0
	//memcpy ( mac.raw, netdev->ll_addr, sizeof ( mac.raw ) );
	ral0 = le32_to_cpu ( mac.reg.low );
	rah0 = ( le32_to_cpu ( mac.reg.high ) | INTELX_RAH0_AV );
	writel ( ral0, sapphire->regs + INTELX_RAL0 );
	writel ( rah0, sapphire->regs + INTELX_RAH0 );
	writel ( ral0, sapphire->regs + INTELX_RAL0_ALT );
	writel ( rah0, sapphire->regs + INTELX_RAH0_ALT );
#endif

	/* Allocate interrupt vectors */
	writel ( ( SAPPHIRE_PX_IVAR_RX0_DEFAULT | SAPPHIRE_PX_IVAR_RX0_VALID |
		   SAPPHIRE_PX_IVAR_TX0_DEFAULT | SAPPHIRE_PX_IVAR_TX0_VALID ),
		 sapphire->regs + SAPPHIRE_PX_IVAR0 );
	writel((SAPPHIRE_PX_MISC_IVAR_DEFAULT | SAPPHIRE_PX_MISC_IVAR_VALID), 
		sapphire->regs + SAPPHIRE_PX_MISC_IVAR);
	writel(SAPPHIRE_DEFAULT_ITR | SAPPHIRE_PX_ITR_CNT_WDIS, 
		sapphire->regs + SAPPHIRE_PX_ITR0);
	
	sapphire->isb_dma = virt_to_bus(sapphire->isb_mem);
	writel(sapphire->isb_dma & 0xffffffff, sapphire->regs + SAPPHIRE_PX_ISB_ADDR_L);
	if ( sizeof ( physaddr_t ) > sizeof ( uint32_t ) ) 
		writel((uint64_t)(sapphire->isb_dma) >> 32, sapphire->regs + SAPPHIRE_PX_ISB_ADDR_H);
	else
		writel(0, sapphire->regs + SAPPHIRE_PX_ISB_ADDR_H);
#if 0
	/* Enable transmitter  */
	dmatxctl = readl ( sapphire->regs + SAPPHIRE_DMATXCTL );
	dmatxctl |= SAPPHIRE_DMATXCTL_TE;
	writel ( dmatxctl, sapphire->regs + SAPPHIRE_DMATXCTL );
#endif
	mactxcfg = readl(sapphire->regs + SAPPHIRE_MACTXCFG);

	if (sapphire->speed == SAPPHIRE_LINK_SPEED_1GB_FULL)
		mactxcfg = (mactxcfg & ~SAPPHIRE_MACTXCFG_SPEED_MASK) | 
				SAPPHIRE_MACTXCFG_SPEED_1G |
				SAPPHIRE_MACTXCFG_TE;
	else
		mactxcfg = (mactxcfg & ~SAPPHIRE_MACTXCFG_SPEED_MASK) | 
				SAPPHIRE_MACTXCFG_SPEED_10G |
				SAPPHIRE_MACTXCFG_TE;
	writel(mactxcfg, sapphire->regs + SAPPHIRE_MACTXCFG);

	/* Configure receive filter */
	fctrl = readl ( sapphire->regs + SAPPHIRE_PSR_CTL );
	fctrl |= (SAPPHIRE_PSR_CTL_BAM | SAPPHIRE_PSR_CTL_UPE | 
		SAPPHIRE_PSR_CTL_MPE);
	writel ( fctrl, sapphire->regs + SAPPHIRE_PSR_CTL );

	vlanctl = readl ( sapphire->regs + SAPPHIRE_PSR_VLAN_CTL);
	vlanctl &= ~SAPPHIRE_PSR_VLAN_CTL_VFE;
	writel ( vlanctl, sapphire->regs + SAPPHIRE_PSR_VLAN_CTL );

	flow_ctrl = readl ( sapphire->regs + SAPPHIRE_MAC_RX_FLOW_CTRL );
	flow_ctrl |= SAPPHIRE_MAC_RX_FLOW_CTRL_RFE;
	writel ( flow_ctrl, sapphire->regs + SAPPHIRE_MAC_RX_FLOW_CTRL );

	pkt_flt = readl ( sapphire->regs + SAPPHIRE_MAC_PKT_FLT );
	pkt_flt |= SAPPHIRE_MAC_PKT_FLT_PR;
	writel ( pkt_flt, sapphire->regs + SAPPHIRE_MAC_PKT_FLT );

	/* Configure jumbo frames.  Required to allow the extra 4-byte
	 * headroom for VLANs, since we don't use the hardware's
	 * native VLAN offload.
	 */
	hlreg0 = readl ( sapphire->regs + SAPPHIRE_MAC_RX_CFG );
	hlreg0 |= SAPPHIRE_MAC_RX_CFG_JE;
	writel ( hlreg0, sapphire->regs + SAPPHIRE_MAC_RX_CFG );

	/* Configure frame size */
	maxfrs = SAPPHIRE_MAXFRS_MFS_DEFAULT;
	writel ( maxfrs, sapphire->regs + SAPPHIRE_PSR_MAX_SZ );

	/* Configure receive DMA */
	rdrxctl = readl ( sapphire->regs + SAPPHIRE_RSC_CTL );
	rdrxctl |= SAPPHIRE_RSC_CTL_CRC_STRIP;
	writel ( rdrxctl, sapphire->regs + SAPPHIRE_RSC_CTL );
#if 0
	/* Enable receiver */
	rxctrl = readl ( sapphire->regs + SAPPHIRE_MAC_RX_CFG );
	rxctrl |= SAPPHIRE_MAC_RX_CFG_RE;
	writel ( rxctrl, sapphire->regs + SAPPHIRE_MAC_RX_CFG );
	rxctrl = readl(sapphire->regs + SAPPHIRE_RDB_PB_CTL);
	rxctrl |= SAPPHIRE_RDB_PB_CTL_RXEN;
	writel ( rxctrl, sapphire->regs + SAPPHIRE_RDB_PB_CTL );
#endif
	/* Fill receive ring */
	sapphire_refill_rx ( sapphire );

	/* Enable transmitter  */
	dmatxctl = readl ( sapphire->regs + SAPPHIRE_DMATXCTL );
	dmatxctl |= SAPPHIRE_DMATXCTL_TE;
	writel ( dmatxctl, sapphire->regs + SAPPHIRE_DMATXCTL );

	/* Enable receiver */
	rxctrl = readl ( sapphire->regs + SAPPHIRE_MAC_RX_CFG );
	rxctrl |= SAPPHIRE_MAC_RX_CFG_RE;
	writel ( rxctrl, sapphire->regs + SAPPHIRE_MAC_RX_CFG );
	rxctrl = readl(sapphire->regs + SAPPHIRE_RDB_PB_CTL);
	rxctrl |= SAPPHIRE_RDB_PB_CTL_RXEN;
	writel ( rxctrl, sapphire->regs + SAPPHIRE_RDB_PB_CTL );

	/* Update link state */
	sapphire_check_link ( netdev );

	return 0;

	sapphire_destroy_ring ( sapphire, &sapphire->rx );
 err_create_rx:
	sapphire_destroy_ring ( sapphire, &sapphire->tx );
 err_create_tx:
	return rc;
}

/**
 * Close network device
 *
 * @v netdev		Network device
 */
static void sapphire_close ( struct net_device *netdev ) {
	struct sapphire_nic *sapphire = netdev->priv;
	uint32_t rxctrl;
	uint32_t dmatxctl;

	/* Disable receiver */
	rxctrl = readl ( sapphire->regs + SAPPHIRE_MAC_RX_CFG );
	rxctrl &= ~SAPPHIRE_MAC_RX_CFG_RE;
	writel ( rxctrl, sapphire->regs + SAPPHIRE_MAC_RX_CFG );
	rxctrl = readl(sapphire->regs + SAPPHIRE_RDB_PB_CTL);
	rxctrl &= ~SAPPHIRE_RDB_PB_CTL_RXEN;
	writel ( rxctrl, sapphire->regs + SAPPHIRE_RDB_PB_CTL );
	

	/* Disable transmitter  */
	dmatxctl = readl ( sapphire->regs + SAPPHIRE_DMATXCTL );
	dmatxctl &= ~SAPPHIRE_DMATXCTL_TE;
	writel ( dmatxctl, sapphire->regs + SAPPHIRE_DMATXCTL );
	dmatxctl = readl(sapphire->regs + SAPPHIRE_MACTXCFG);
	dmatxctl &= ~SAPPHIRE_MACTXCFG_TE;
	writel(dmatxctl, sapphire->regs + SAPPHIRE_MACTXCFG);

	/* Destroy receive descriptor ring */
	sapphire_destroy_ring ( sapphire, &sapphire->rx );

	/* Discard any unused receive buffers */
	sapphire_empty_rx ( sapphire );

	/* Destroy transmit descriptor ring */
	sapphire_destroy_ring ( sapphire, &sapphire->tx );

	/* Reset the NIC, to flush the transmit and receive FIFOs */
	sapphire_reset ( sapphire );
}

/**
 * Poll for completed packets
 *
 * @v netdev		Network device
 */
void sapphire_poll_tx ( struct net_device *netdev ) {
	struct sapphire_nic *sapphire = netdev->priv;
	union sapphire_descriptor *tx;
	unsigned int tx_idx;

	/* Check for completed packets */
	while ( sapphire->tx.cons != sapphire->tx.prod ) {

		/* Get next transmit descriptor */
		tx_idx = ( sapphire->tx.cons % SAPPHIRE_NUM_TX_DESC );
		tx = &sapphire->tx.desc[tx_idx];

		/* Stop if descriptor is still in use */
		if ( ! ( tx->tx.status & cpu_to_le32 ( SAPPHIRE_DESC_STATUS_DD ) ) )
			return;

		DBGC2 ( sapphire, "SAPPHIRE %p TX %d complete\n", sapphire, tx_idx );

		/* Complete TX descriptor */
		netdev_tx_complete_next ( netdev );
		sapphire->tx.cons++;
	}
}

/**
 * Poll for received packets
 *
 * @v netdev		Network device
 */
void sapphire_poll_rx ( struct net_device *netdev ) {
	struct sapphire_nic *sapphire = netdev->priv;
	union sapphire_descriptor *rx;
	struct io_buffer *iobuf;
	unsigned int rx_idx;
	size_t len;

	/* Check for received packets */
	while ( sapphire->rx.cons != sapphire->rx.prod ) {

		/* Get next receive descriptor */
		rx_idx = ( sapphire->rx.cons % SAPPHIRE_NUM_RX_DESC );
		rx = &sapphire->rx.desc[rx_idx];

		/* Stop if descriptor is still in use */
		if ( ! ( rx->rx.status_error & cpu_to_le32 ( SAPPHIRE_DESC_STATUS_DD ) ) )
			return;

		/* Populate I/O buffer */
		iobuf = sapphire->rx_iobuf[rx_idx];
		sapphire->rx_iobuf[rx_idx] = NULL;
		len = le16_to_cpu ( rx->rx.length );
		iob_put ( iobuf, len );

		/* Hand off to network stack */
		if ( rx->rx.status_error & cpu_to_le32 ( SAPPHIRE_DESC_STATUS_RXE ) ) {
			DBGC ( sapphire, "SAPPHIRE %p RX %d error (length %zd, "
			       "status %08x)\n", sapphire, rx_idx, len,
			       le32_to_cpu ( rx->rx.status_error ) );
			netdev_rx_err ( netdev, iobuf, -EIO );
		} else {
			DBGC2 ( sapphire, "SAPPHIRE %p RX %d complete (length %zd)\n",
				sapphire, rx_idx, len );
			netdev_rx ( netdev, iobuf );
		}
		sapphire->rx.cons++;
	}
}


/**
 * Poll for completed and received packets
 *
 * @v netdev		Network device
 */
static void sapphire_poll ( struct net_device *netdev ) {
	struct sapphire_nic *sapphire = netdev->priv;
//	uint32_t eicr, eicr_misc;
// DBGC2 ( sapphire, "SAPPHIRE %p in poll\n",
//                                sapphire);

	/* Check for and acknowledge interrupts */
	profile_start ( &sapphire_vm_poll_profiler );
//	eicr = sapphire->isb_mem[SAPPHIRE_ISB_VEC0];
	profile_stop ( &sapphire_vm_poll_profiler );
	profile_exclude ( &sapphire_vm_poll_profiler );
//	if ( ! eicr )
//		return;
//	eicr_misc = sapphire->isb_mem[SAPPHIRE_ISB_MISC];
	/* Poll for TX completions, if applicable */
	sapphire_poll_tx ( netdev );

	/* Poll for RX completions, if applicable */
	sapphire_poll_rx ( netdev );

	/* Check link state, if applicable */
//	if ( eicr_misc & (SAPPHIRE_PX_MISC_IC_ETH_LK | SAPPHIRE_PX_MISC_IC_ETH_LKDN) )
//		sapphire_check_link ( netdev );

	/* Refill RX ring */
	sapphire_refill_rx ( sapphire );
}

/**
 * Enable or disable interrupts
 *
 * @v netdev		Network device
 * @v enable		Interrupts should be enabled
 */
static void sapphire_irq ( struct net_device *netdev, int enable ) {
	struct sapphire_nic *sapphire = netdev->priv;
	uint32_t mask;
	uint32_t qmask;

	mask = SAPPHIRE_PX_MISC_IEN_ETH_LKDN | SAPPHIRE_PX_MISC_IEN_ETH_LK;
	writel(mask, sapphire->regs + SAPPHIRE_PX_MISC_IEN);
//	mask = ( INTELX_EIRQ_LSC | INTELX_EIRQ_RXO | INTELX_EIRQ_TX0 |
//		 INTELX_EIRQ_RX0 );
	qmask = 0x3;
	if ( enable ) {
		writel ( qmask, sapphire->regs + SAPPHIRE_PX_IMC0 );
	} else {
		writel ( qmask, sapphire->regs + SAPPHIRE_PX_IMS0 );
	}
	DBGC2 ( sapphire, "SAPPHIRE %p irq is enabled? [%d)\n", sapphire, enable);
}

/**
 * Transmit packet
 *
 * @v netdev		Network device
 * @v iobuf		I/O buffer
 * @ret rc		Return status code
 */
int sapphire_transmit ( struct net_device *netdev, struct io_buffer *iobuf ) {
	struct sapphire_nic *sapphire = netdev->priv;
	union sapphire_descriptor *tx;
	unsigned int tx_idx;
	unsigned int tx_tail;
	physaddr_t address;
	size_t len;
writel ( 0x1, sapphire->regs + 0x130 );
	/* Get next transmit descriptor */
	if ( ( sapphire->tx.prod - sapphire->tx.cons ) >= SAPPHIRE_TX_FILL ) {
		DBGC ( sapphire, "SAPPHIRE %p out of transmit descriptors\n", sapphire );
		return -ENOBUFS;
	}
	tx_idx = ( sapphire->tx.prod++ % SAPPHIRE_NUM_TX_DESC );
	tx_tail = ( sapphire->tx.prod % SAPPHIRE_NUM_TX_DESC );
	tx = &sapphire->tx.desc[tx_idx];

	/* Populate transmit descriptor */
	address = virt_to_bus ( iobuf->data );
	len = iob_len ( iobuf );
	sapphire->tx.describe ( tx, address, len );
	wmb();

	/* Notify card that there are packets ready to transmit */
	profile_start ( &sapphire_vm_tx_profiler );
	writel ( tx_tail, sapphire->regs + sapphire->tx.reg + SAPPHIRE_DH );
	profile_stop ( &sapphire_vm_tx_profiler );
	profile_exclude ( &sapphire_vm_tx_profiler );

	DBGC2 ( sapphire, "SAPPHIRE %p TX %d is [%llx,%llx)\n", sapphire, tx_idx,
		( ( unsigned long long ) address ),
		( ( unsigned long long ) address + len ) );

	return 0;
}


#ifndef PMON
/** Network device operations */
static struct net_device_operations sapphire_operations = {
	.open		= sapphire_open,
	.close		= sapphire_close,
	.transmit	= sapphire_transmit,
	.poll		= sapphire_poll,
	.irq		= sapphire_irq,
};
#endif

/******************************************************************************
 *
 * PCI interface
 *
 ******************************************************************************
 */

/**
 * Probe PCI device
 *
 * @v pci		PCI device
 * @ret rc		Return status code
 */
static int sapphire_probe ( struct pci_device *pci ) {
	struct net_device *netdev;
	struct sapphire_nic *sapphire;
	int rc;

	/* Allocate and initialise net device */
	netdev = alloc_etherdev ( sizeof ( *sapphire ) );
	if ( ! netdev ) {
		rc = -ENOMEM;
		goto err_alloc;
	}
	netdev_init ( netdev, &sapphire_operations );
	sapphire = netdev->priv;
#ifndef PMON
	pci_set_drvdata ( pci, netdev );
	netdev->dev = &pci->dev;
#endif
	memset ( sapphire, 0, sizeof ( *sapphire ) );
	sapphire->port = PCI_FUNC ( pci->busdevfn );
	sapphire_init_ring ( &sapphire->tx, SAPPHIRE_NUM_TX_DESC, SAPPHIRE_TD,
			  sapphire_describe_tx );
	sapphire_init_ring ( &sapphire->rx, SAPPHIRE_NUM_RX_DESC, SAPPHIRE_RD,
			  sapphire_describe_rx );

	/* Fix up PCI device */
	adjust_pci_device ( pci );

	/* Map registers */
	sapphire->regs = ioremap ( pci->membase, INTEL_BAR_SIZE );
	if ( ! sapphire->regs ) {
		rc = -ENODEV;
		goto err_ioremap;
	}

	/* Reset the NIC */
	if ( ( rc = sapphire_reset ( sapphire ) ) != 0 )
		goto err_reset;

	/* Fetch MAC address */
	if ( ( rc = sapphire_fetch_mac ( sapphire, netdev->hw_addr ) ) != 0 )
		goto err_fetch_mac;

	/* Register network device */
	if ( ( rc = register_netdev ( netdev ) ) != 0 )
		goto err_register_netdev;

	/* Set initial link state */
	if ( ! ( readl ( sapphire->regs + SAPPHIRE_CFG_PORT_ST )
		& SAPPHIRE_CFG_PORT_ST_LINK_UP)) {
		sapphire_setup_link(sapphire, SAPPHIRE_LINK_SPEED_1GB_FULL);
	}

	if ( ! ( readl ( sapphire->regs + SAPPHIRE_CFG_PORT_ST )
		& SAPPHIRE_CFG_PORT_ST_LINK_UP)) {
		sapphire_setup_link(sapphire, SAPPHIRE_LINK_SPEED_10GB_FULL);
	}

	sapphire_check_link ( netdev );

	return 0;

 err_register_netdev:
	unregister_netdev ( netdev );
 err_fetch_mac:
	sapphire_reset ( sapphire );
 err_reset:
	iounmap ( sapphire->regs );
 err_ioremap:
	netdev_nullify ( netdev );
	netdev_put ( netdev );
 err_alloc:
	return rc;
}

#ifndef PMON
/**
 * Remove PCI device
 *
 * @v pci		PCI device
 */
static void sapphire_remove ( struct pci_device *pci ) {
	struct net_device *netdev = pci_get_drvdata ( pci );
	struct sapphire_nic *sapphire = netdev->priv;

	/* Unregister network device */
	unregister_netdev ( netdev );

	/* Reset the NIC */
	sapphire_reset ( sapphire );

	/* Free network device */
	iounmap ( sapphire->regs );
	netdev_nullify ( netdev );
	netdev_put ( netdev );
}
#endif

/** PCI device IDs */
static struct pci_device_id sapphire_nics[] = {
	PCI_ROM ( 0x8088, 0x1001, "sapphire-sfp", "sapphire (SFI/SFP+)", 0 ),
	PCI_ROM ( 0x8088, 0x1002, "sapphire-combo-backplane", "sapphire (combined backplane; KR/KX4/KX)", 0 ),
	PCI_ROM ( 0x8088, 0x1003, "sapphire-xaui", "sapphire (XAUI)", 0 ),
	PCI_ROM ( 0x8088, 0x1004, "sapphire-sgmii", "sapphire (SGMII)", 0 ),
	PCI_ROM ( 0x8088, 0x2001, "WX1820AL", "WX1820AL (SFP+)", 0 ),
};

#ifndef PMON
/** PCI driver */
struct pci_driver sapphire_driver __pci_driver = {
	.ids = sapphire_nics,
	.id_count = ( sizeof ( sapphire_nics ) / sizeof ( sapphire_nics[0] ) ),
	.probe = sapphire_probe,
	.remove = sapphire_remove,
};
#else
#include "txgbe_pmon_tail.h"
#endif
