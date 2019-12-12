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
#include "ngbe.h"
#else
#include "ngbe_pmon_head.h"
#endif

/** @file
 *
 * Sapphire 10 Gigabit Ethernet network card driver
 *
 */
#ifndef PMON
/** VM transmit profiler */
static struct profiler emerald_vm_tx_profiler __profiler =
	{ .name = "emerald.vm_tx" };

/** VM receive refill profiler */
static struct profiler emerald_vm_refill_profiler __profiler =
	{ .name = "emerald.vm_refill" };

/** VM poll profiler */
static struct profiler emerald_vm_poll_profiler __profiler =
	{ .name = "emerald.vm_poll" };
#endif

/**
 * Disable descriptor ring
 *
 * @v emerald		Sapphire device
 * @v reg		Register block
 * @ret rc		Return status code
 */
static int emerald_disable_ring ( struct emerald_nic *emerald, unsigned int reg ) {
	uint32_t dctl;
	unsigned int i;

	/* Disable ring */
	dctl = readl ( emerald->regs + reg + EMERALD_DCTL );
	dctl &= ~EMERALD_DCTL_ENABLE;
	writel (dctl, ( emerald->regs + reg + EMERALD_DCTL ) );

	/* Wait for disable to complete */
	for ( i = 0 ; i < EMERALD_DISABLE_MAX_WAIT_MS ; i++ ) {

		/* Check if ring is disabled */
		dctl = readl ( emerald->regs + reg + EMERALD_DCTL );
		if ( ! ( dctl & EMERALD_DCTL_ENABLE ) )
			return 0;

		/* Delay */
		mdelay ( 1 );
	}

	DBGC ( emerald, "EMERALD %p ring %05x timed out waiting for disable "
	       "(dctl %08x)\n", emerald, reg, dctl );
	return -ETIMEDOUT;
}


/**
 * Reset descriptor ring
 *
 * @v emerald		Sapphire device
 * @v reg		Register block
 * @ret rc		Return status code
 */
void emerald_reset_ring ( struct emerald_nic *emerald, unsigned int reg ) {

	/* Disable ring.  Ignore errors and continue to reset the ring anyway */
	emerald_disable_ring ( emerald, reg );

	/* Clear ring length */
//	writel ( 0, ( emerald->regs + reg + EMERALD_DLEN ) );

	/* Clear ring address */
	writel ( 0, ( emerald->regs + reg + EMERALD_DBAH ) );
	writel ( 0, ( emerald->regs + reg + EMERALD_DBAL ) );

	/* Reset head and tail pointers */
	writel ( 0, ( emerald->regs + reg + EMERALD_DH ) );
	writel ( 0, ( emerald->regs + reg + EMERALD_DT ) );
}


/**
 * Create descriptor ring
 *
 * @v emerald		Sapphire device
 * @v ring		Descriptor ring
 * @ret rc		Return status code
 */
int emerald_create_ring ( struct emerald_nic *emerald, struct emerald_ring *ring,
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
		 ( emerald->regs + ring->reg + EMERALD_DBAL ) );
	if ( sizeof ( physaddr_t ) > sizeof ( uint32_t ) ) {
		writel ( ( ( ( uint64_t ) address ) >> 32 ),
			 ( emerald->regs + ring->reg + EMERALD_DBAH ) );
	} else {
		writel ( 0, emerald->regs + ring->reg + EMERALD_DBAH );
	}


	/* Reset head and tail pointers */
	writel ( 0, ( emerald->regs + ring->reg + EMERALD_DH ) );
	writel ( 0, ( emerald->regs + ring->reg + EMERALD_DT ) );

	/* Enable ring */
	if (is_tx_ring) {
		dctl = readl ( emerald->regs + ring->reg + EMERALD_DCTL );
		dctl |= EMERALD_DCTL_ENABLE |
			(ring->len / sizeof ( ring->desc[0] ) / 128) << EMERALD_DCTL_RING_SIZE_SHIFT
			| 0x20 << EMERALD_DCTL_TX_WTHRESH_SHIFT;
		writel ( dctl, emerald->regs + ring->reg + EMERALD_DCTL );
	} else {
		dctl = readl ( emerald->regs + ring->reg + EMERALD_DCTL );
		dctl |= EMERALD_DCTL_ENABLE |
			(ring->len / sizeof ( ring->desc[0] ) / 128) << EMERALD_DCTL_RING_SIZE_SHIFT
			| 0x1 << EMERALD_DCTL_RX_THER_SHIFT;
		dctl &= ~(EMERALD_DCTL_RX_HDR_SZ |
			  EMERALD_DCTL_RX_BUF_SZ |
			  EMERALD_DCTL_RX_SPLIT_MODE);
		dctl |= EMERALD_RX_HDR_SIZE << EMERALD_DCTL_RX_BSIZEHDRSIZE_SHIFT;
		dctl |= EMERALD_RX_BSIZE_DEFAULT >> EMERALD_DCTL_RX_BSIZEPKT_SHIFT;
		writel ( dctl, emerald->regs + ring->reg + EMERALD_DCTL );
	}

	DBGC ( emerald, "EMERALD %p ring %05x is at [%08llx,%08llx)\n",
	       emerald, ring->reg, ( ( unsigned long long ) address ),
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
 * @v emerald		Sapphire device
 * @v ral0		RAL0 register address
 * @v hw_addr		Hardware address to fill in
 * @ret rc		Return status code
 */
static int emerald_try_fetch_mac ( struct emerald_nic *emerald, unsigned int ral0,
				  uint8_t *hw_addr ) {
	union emerald_receive_address mac;
//	uint8_t raw[ETH_ALEN];
	uint8_t i;

	/* Read current address from RAL0/RAH0 */
	writel(0, emerald->regs + EMERALD_PSR_MAC_SWC_IDX);
	mac.reg.low = cpu_to_le32 ( readl ( emerald->regs + ral0 ) );
	mac.reg.high = cpu_to_le32 ( readl ( emerald->regs + ral0 + 4 ) );

	for (i = 0; i < 2; i++)
		hw_addr[i] = (u8)(mac.reg.high >> (1 - i) * 8);

	for (i = 0; i < 4; i++)
		hw_addr[i + 2] = (u8)(mac.reg.low >> (3 - i) * 8);

	/* Use current address if valid */
	if ( /*is_valid_ether_addr ( hw_addr )*/1 ) {
		DBGC ( emerald, "EMERALD %p has autoloaded MAC address %s at "
		       "%#05x\n", emerald, eth_ntoa ( hw_addr ), ral0 );
		//memcpy ( hw_addr, raw, ETH_ALEN );
		return 0;
	}

	return -ENOENT;
}

/**
 * Fetch initial MAC address
 *
 * @v emerald		Sapphire device
 * @v hw_addr		Hardware address to fill in
 * @ret rc		Return status code
 */
static int emerald_fetch_mac ( struct emerald_nic *emerald, uint8_t *hw_addr ) {
	int rc;

	/* Try to fetch address from RAL0 */
	if ( ( rc = emerald_try_fetch_mac ( emerald, EMERALD_PSR_MAC_SWC_AD_L,
					   hw_addr ) ) == 0 ) {
		return 0;
	}

	DBGC ( emerald, "EMERALD %p has no MAC address to use\n", emerald );
	return -ENOENT;
}


static inline uint32_t
emerald_rd32m(struct emerald_nic *emerald,
				uint32_t reg,
				uint32_t mask)
{
	uint32_t val;

	val = readl(emerald->regs + reg);
	if (val == EMERALD_FAILED_READ_REG)
		return val;

	return (val & mask);
}

static inline void
emerald_wr32m(struct emerald_nic *emerald,
				uint32_t reg,
				uint32_t mask,
				uint32_t field)
{
	uint32_t val;

	val = readl(emerald->regs + reg);
	if (val == EMERALD_FAILED_READ_REG)
		return;

	val = ((val & ~mask) | (field & mask));
	writel(val, emerald->regs + reg);
}

static inline int
emerald_po32m(struct emerald_nic *emerald,
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
		value = readl(emerald->regs + reg);
		if ((value & mask) == (field & mask)) {
			break;
		}

		if (loop-- <= 0)
			break;

		usec_delay(usecs);
	} while (true);

	return (count - loop <= count ? 0 : EMERALD_ERR_TIMEOUT);
}

s32 emerald_phy_read_reg(struct emerald_nic *emerald,
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
			writel(page,
				emerald->regs + EMERALD_PHY_CONFIG(EMERALD_INTPHY_PAGE_SELECT));
			page_select = true;
		}
	}

	if (reg_offset >= EMERALD_INTPHY_PAGE_MAX) {
		DBGC ( emerald, "read reg offset %d exceed maximum 31.\n", reg_offset);
		return EMERALD_ERR_INVALID_PARAM;
	}

	*phy_data = 0xFFFF & readl(emerald->regs + EMERALD_PHY_CONFIG(reg_offset));

	if (page_select)
		writel(0,
			emerald->regs + EMERALD_PHY_CONFIG(EMERALD_INTPHY_PAGE_SELECT));

	return 0;
}

/* For internal phy only */
s32 emerald_phy_write_reg( struct emerald_nic *emerald,
						u32 reg_offset,
						u32 page,
						u16 phy_data)
{
	bool page_select = false;

	if (0 != page) {
		/* select page */
		if (0xa42 == page || 0xa43 == page || 0xa46 == page || 0xa47 == page ||
			0xd04 == page) {
			writel(page,
				emerald->regs + EMERALD_PHY_CONFIG(EMERALD_INTPHY_PAGE_SELECT));
			page_select = true;
		}
	}

	if (reg_offset >= EMERALD_INTPHY_PAGE_MAX) {
		DBGC ( emerald, "write reg offset %d exceed maximum 31.\n", reg_offset);
		return EMERALD_ERR_INVALID_PARAM;
	}

	writel(phy_data, emerald->regs + EMERALD_PHY_CONFIG(reg_offset));

	if (page_select)
		writel(0,
			emerald->regs + EMERALD_PHY_CONFIG(EMERALD_INTPHY_PAGE_SELECT));

	return 0;
}

static inline int
emerald_read_phy_reg_mdi(struct emerald_nic *emerald,
						 uint32_t reg_addr,
						 uint32_t device_type,
						 uint16_t *phy_data)
{
	uint32_t command;
	int32_t status = 0;

	/* setup and write the address cycle command */
	command = EMERALD_MSCA_RA(reg_addr) |
		EMERALD_MSCA_PA(emerald->phy_addr) |
		EMERALD_MSCA_DA(device_type);
	writel(command, emerald->regs + EMERALD_MSCA);

	command = EMERALD_MSCC_CMD(EMERALD_MSCA_CMD_POST_READ) |
			  EMERALD_MSCC_BUSY |
			  EMERALD_MDIO_CLK(6);
	writel(command, emerald->regs + EMERALD_MSCC);

	/* wait to complete */
	status = emerald_po32m(emerald, EMERALD_MSCC,
						   EMERALD_MSCC_BUSY, ~EMERALD_MSCC_BUSY,
						   EMERALD_MDIO_TIMEOUT, 10);
	if (status != 0) {
		DBGC ( emerald, "EMERALD PHY address command did not complete.\n" );
		return EMERALD_ERR_PHY;
	}

	/* read data from MSCC */
	*phy_data = 0xFFFF & readl(emerald->regs + EMERALD_MSCC);

	return 0;
}

/**
 *	ngbe_write_phy_reg_mdi - Writes a value to specified PHY register
 *	without SWFW lock
 *	@hw: pointer to hardware structure
 *	@reg_addr: 32 bit PHY register to write
 *	@device_type: 5 bit device type
 *	@phy_data: Data to write to the PHY register
 **/
static inline int
emerald_write_phy_reg_mdi(struct emerald_nic *emerald,
						  uint32_t reg_addr,
						  uint32_t device_type,
						  uint16_t phy_data)
{
	uint32_t command;
	int32_t status = 0;

	/* setup and write the address cycle command */
	command = EMERALD_MSCA_RA(reg_addr) |
			  EMERALD_MSCA_PA(emerald->phy_addr) |
			  EMERALD_MSCA_DA(device_type);
	writel(command, emerald->regs + EMERALD_MSCA);

	command = phy_data | EMERALD_MSCC_CMD(EMERALD_MSCA_CMD_WRITE) |
			  EMERALD_MSCC_BUSY | EMERALD_MDIO_CLK(6);
	writel(command, emerald->regs + EMERALD_MSCC);

	/* wait to complete */
	status = emerald_po32m(emerald, EMERALD_MSCC,
						   EMERALD_MSCC_BUSY, ~EMERALD_MSCC_BUSY,
						   EMERALD_MDIO_TIMEOUT, 10);
	if (status != 0) {
		DBGC ( emerald, "write PHY address command did not complete.\n" );
		return EMERALD_ERR_PHY;
	}

	return 0;
}

/******************************************************************************
 *
 * Link state
 *
 ******************************************************************************
 */

/* check link up/down and speed, reading from phy data */
static inline bool
emerald_link_is_up(struct emerald_nic *emerald)
{
	uint16_t value = 0;
	uint32_t lan_speed = 2;
	bool link_status = false;


//	writel(0x1, emerald->regs + EMERALD_GPIO_DR);
//	writel(0x1, emerald->regs + EMERALD_GPIO_DDR);
	writel(0xF, emerald->regs + EMERALD_MDIO_CLAUSE22);


	emerald_phy_read_reg(emerald, 0x1A, 0xA43, &value);

#if 0
	switch (emerald->subsystem_device) {
		case 0x0000:
			emerald_phy_read_reg(emerald, 0x1A, 0xA43, &value);
			break;
		case 0x0100:
		case 0x0200:
			emerald_write_phy_reg_mdi(emerald, 22, 0x01, 0);
			emerald_read_phy_reg_mdi(emerald, 17, 0x01, &value);
			break;
		default:
			return EMERALD_ERR_DEV_ID;
	}
#endif
	if ((value & 0x4) && (value != 0xFFFF)) {
		link_status = true;
		emerald->link_up = true;
	} else {
		emerald->link_up = false;
		return link_status;
	}
	/* refresh speed */
	if (link_status) {
		if ((value & 0x38) == 0x28) {
			emerald->speed = EMERALD_LINK_SPEED_1GB_FULL;
		} else if ((value & 0x38) == 0x18) {
			emerald->speed = EMERALD_LINK_SPEED_100M_FULL;
		} else if ((value & 0x38) == 0x8) {
			emerald->speed = EMERALD_LINK_SPEED_10M_FULL;
		}
	} else
		emerald->speed = EMERALD_LINK_SPEED_UNKNOWN;



	switch (emerald->speed) {
		case EMERALD_LINK_SPEED_100M_FULL:
			lan_speed = 1;//2‘b01
			break;
		case EMERALD_LINK_SPEED_1GB_FULL:
			lan_speed = 2;//2‘b10
			break;
		case EMERALD_LINK_SPEED_10M_FULL:
			lan_speed = 0;//2‘b00
			break;
		default:
			break;
		}
//	emerald_wr32m(lan_speed, emerald->regs + EMERALD_CFG_LAN_SPEED);
	emerald_wr32m(emerald, EMERALD_CFG_LAN_SPEED,
					0x3,
					lan_speed);

	return link_status;
}


/**
 * Check link state
 *
 * @v netdev		Network device
 */
static void emerald_check_link ( struct net_device *netdev ) {
	struct emerald_nic *emerald = netdev->priv;

	int i;

	for (i=0; i<36; i++) {
		if (emerald_link_is_up(emerald)) {
			netdev_link_up ( netdev );
			break;
		}
		msleep(100);
	}
	//XXDBG("====%d===\n", i);
	/* Update network device */
	if (i == 36) {
		netdev_link_down ( netdev );
	}
}

/**
 * Destroy descriptor ring
 *
 * @v emerald		Sapphire device
 * @v ring		Descriptor ring
 */
void emerald_destroy_ring ( struct emerald_nic *emerald, struct emerald_ring *ring ) {

	/* Reset ring */
	emerald_reset_ring ( emerald, ring->reg );

	/* Free descriptor ring */
	free_dma ( ring->desc, ring->len );
	ring->desc = NULL;
	ring->prod = 0;
	ring->cons = 0;
}


/**
 * Refill receive descriptor ring
 *
 * @v emerald		Sapphire device
 */
void emerald_refill_rx ( struct emerald_nic *emerald ) {
	union emerald_descriptor *rx;
	struct io_buffer *iobuf;
	unsigned int rx_idx;
	unsigned int rx_tail;
	physaddr_t address;
	unsigned int refilled = 0;

	/* Refill ring */
	while ( ( emerald->rx.prod - emerald->rx.cons ) < EMERALD_RX_FILL ) {

		/* Allocate I/O buffer */
		iobuf = alloc_iob ( EMERALD_RX_MAX_LEN );
		if ( ! iobuf ) {
			/* Wait for next refill */
			break;
		}

		/* Get next receive descriptor */
		rx_idx = ( emerald->rx.prod++ % EMERALD_NUM_RX_DESC );
		rx = &emerald->rx.desc[rx_idx];

		/* Populate receive descriptor */
		address = virt_to_bus ( iobuf->data );
		emerald->rx.describe ( rx, address, 0 );

		/* Record I/O buffer */
		assert ( emerald->rx_iobuf[rx_idx] == NULL );
		emerald->rx_iobuf[rx_idx] = iobuf;

		DBGC2 ( emerald, "EMERALD %p RX %d is [%llx,%llx)\n", emerald, rx_idx,
			( ( unsigned long long ) address ),
			( ( unsigned long long ) address + EMERALD_RX_MAX_LEN ) );
		refilled++;
	}

	/* Push descriptors to card, if applicable */
	if ( refilled ) {
		wmb();
		rx_tail = ( emerald->rx.prod % EMERALD_NUM_RX_DESC );
		profile_start ( &emerald_vm_refill_profiler );
		writel ( rx_tail, emerald->regs + emerald->rx.reg + EMERALD_DH );
		profile_stop ( &emerald_vm_refill_profiler );
		profile_exclude ( &emerald_vm_refill_profiler );
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
void emerald_describe_tx ( union emerald_descriptor *tx, physaddr_t addr,
			     size_t len ) {

	/* Populate advanced transmit descriptor */
	tx->tx.address = cpu_to_le64 ( addr );
	tx->tx.length = cpu_to_le16 ( len );
	tx->tx.flags = 0;
	tx->tx.command = EMERALD_TXD_DTYP_DATA | EMERALD_TXD_RS|
			EMERALD_TXD_IFCS | EMERALD_TXD_EOP;
	tx->tx.status = cpu_to_le32 ( EMERALD_DESC_STATUS_PAYLEN ( len ) );
}

/**
 * Populate receive descriptor
 *
 * @v rx		Receive descriptor
 * @v addr		Data buffer address
 * @v len		Length of data
 */
void emerald_describe_rx ( union emerald_descriptor *rx, physaddr_t addr,
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
 * @v emerald		Sapphire device
 */
void emerald_empty_rx ( struct emerald_nic *emerald ) {
	unsigned int i;

	for ( i = 0 ; i < EMERALD_NUM_RX_DESC ; i++ ) {
		if ( emerald->rx_iobuf[i] )
			free_iob ( emerald->rx_iobuf[i] );
		emerald->rx_iobuf[i] = NULL;
	}
}

#if 0
static inline uint32_t
emerald_rd32_epcs(struct emerald_nic *emerald,
					uint32_t addr)
{
	unsigned int portRegOffset;
	uint32_t data;
	//Set the LAN port indicator to portRegOffset[1]
	//1st, write the regOffset to IDA_ADDR register
	portRegOffset = EMERALD_XPCS_IDA_ADDR;
	writel(addr, emerald->regs + portRegOffset);

	//2nd, read the data from IDA_DATA register
	portRegOffset = EMERALD_XPCS_IDA_DATA;
	data = readl(emerald->regs + portRegOffset);

	return data;
}

static inline void
emerald_wr32_epcs(struct emerald_nic *emerald,
					uint32_t addr,
					uint32_t data)
{
	unsigned int portRegOffset;

	//Set the LAN port indicator to portRegOffset[1]
	//1st, write the regOffset to IDA_ADDR register
	portRegOffset = EMERALD_XPCS_IDA_ADDR;
	writel(addr, emerald->regs + portRegOffset);

	//2nd, read the data from IDA_DATA register
	portRegOffset = EMERALD_XPCS_IDA_DATA;
	writel(data, emerald->regs + portRegOffset);
}
#endif




#if 0
void emerald_init_i2c(struct emerald_nic *emerald)
{
	writel(0, emerald->regs + EMERALD_I2C_ENABLE);
	writel((EMERALD_I2C_CON_MASTER_MODE |
		EMERALD_I2C_CON_SPEED(1) | EMERALD_I2C_CON_RESTART_EN |
		EMERALD_I2C_CON_SLAVE_DISABLE), emerald->regs + EMERALD_I2C_CON);
	writel(EMERALD_I2C_SLAVE_ADDR, emerald->regs + EMERALD_I2C_TAR);
	writel(600, emerald->regs + EMERALD_I2C_SS_SCL_HCNT);
	writel(600, emerald->regs + EMERALD_I2C_SS_SCL_LCNT);
	writel(0, emerald->regs + EMERALD_I2C_RX_TL); /* 1byte for rx full signal */
	writel(4, emerald->regs + EMERALD_I2C_TX_TL);
	writel(0xFFFFFF, emerald->regs + EMERALD_I2C_SCL_STUCK_TIMEOUT);
	writel(0xFFFFFF, emerald->regs + EMERALD_I2C_SDA_STUCK_TIMEOUT);

	writel(0, emerald->regs + EMERALD_I2C_INTR_MASK);
	writel(1, emerald->regs + EMERALD_I2C_ENABLE);
}

static int emerald_read_i2c(struct emerald_nic *emerald,
								u8 byte_offset,
				  				u8 *data)
{
	int status = 0;

	/* wait tx empty */
	status = emerald_po32m(emerald, EMERALD_I2C_RAW_INTR_STAT,
		EMERALD_I2C_INTR_STAT_TX_EMPTY, EMERALD_I2C_INTR_STAT_TX_EMPTY,
		EMERALD_I2C_TIMEOUT, 10);
	if (status != 0)
		goto out;

	/* read data */
	writel(byte_offset | EMERALD_I2C_DATA_CMD_STOP,
			emerald->regs + EMERALD_I2C_DATA_CMD);
	writel(EMERALD_I2C_DATA_CMD_READ, emerald->regs + EMERALD_I2C_DATA_CMD);

	/* wait for read complete */
	status = emerald_po32m(emerald, EMERALD_I2C_RAW_INTR_STAT,
		EMERALD_I2C_INTR_STAT_RX_FULL, EMERALD_I2C_INTR_STAT_RX_FULL,
		EMERALD_I2C_TIMEOUT, 10);
	if (status != 0)
		goto out;

	*data = 0xFF & readl(emerald->regs + EMERALD_I2C_DATA_CMD);
out:
	return status;
}
#endif
#if 0
int emerald_identify_sfp(struct emerald_nic *emerald)
{
	int status = 0;
	u8 identifier = 0;
	u8 comp_codes_1g = 0;
	u8 comp_codes_10g = 0;
	u8 cable_tech = 0;


	emerald_init_i2c(emerald);
	status = emerald_read_i2c(emerald, EMERALD_SFF_IDENTIFIER,
								&identifier);
	if (status != 0) {
		return EMERALD_ERR_I2C;
	}

	if (identifier != EMERALD_SFF_IDENTIFIER_SFP) {
		return EMERALD_ERR_I2C_SFP_NOT_SUPPORTED;
	} else {
		status = emerald_read_i2c(emerald, EMERALD_SFF_1GBE_COMP_CODES,
									&comp_codes_1g);
		if (status != 0)
			return EMERALD_ERR_I2C;

		status = emerald_read_i2c(emerald, EMERALD_SFF_10GBE_COMP_CODES,
									&comp_codes_10g);
		if (status != 0)
			return EMERALD_ERR_I2C;

		status = emerald_read_i2c(emerald, EMERALD_SFF_CABLE_TECHNOLOGY,
									&cable_tech);
		if (status != 0)
			return EMERALD_ERR_I2C;
	}
	if (cable_tech & EMERALD_SFF_DA_PASSIVE_CABLE) {
		emerald->sfp_type = emerald_sfp_type_da_cu_core0;
	} else {
		emerald->sfp_type = emerald_sfp_type_unknown;
	}
	return 0;
}
#endif
#if 0
void emerald_select_speed(struct emerald_nic *emerald,
					uint32_t speed)
{
	uint32_t esdp_reg = readl(emerald->regs + EMERALD_GPIO_DR);

	switch (speed) {
	case EMERALD_LINK_SPEED_10GB_FULL:
			esdp_reg |= EMERALD_GPIO_DR_5 | EMERALD_GPIO_DR_4;
		break;
	case EMERALD_LINK_SPEED_1GB_FULL:
			esdp_reg &= ~(EMERALD_GPIO_DR_5 | EMERALD_GPIO_DR_4);
		break;
	default:
		DBGC ( emerald, "EMERALD invalid module speed\n" );
		return;
	}

	writel( EMERALD_GPIO_DDR_5 | EMERALD_GPIO_DDR_4 |
			EMERALD_GPIO_DDR_1 | EMERALD_GPIO_DDR_0,
		emerald->regs + EMERALD_GPIO_DDR);

	writel(esdp_reg, emerald->regs + EMERALD_GPIO_DR);

	readl(emerald->regs + EMERALD_CFG_PORT_ST);
}
#endif

#if 0
int emerald_enable_rx_adapter(struct emerald_nic *emerald)
{
	uint32_t value;

	value = emerald_rd32_epcs(emerald, EMERALD_PHY_RX_EQ_CTL);
	value |= 1 << 12;
	emerald_wr32_epcs(emerald, EMERALD_PHY_RX_EQ_CTL, value);

	value = 0;
	while (!(value >> 11)) {
		value = emerald_rd32_epcs(emerald, EMERALD_PHY_RX_AD_ACK);
		msleep(1);
	}

	value = emerald_rd32_epcs(emerald, EMERALD_PHY_RX_EQ_CTL);
	value &= ~(1 << 12);
	emerald_wr32_epcs(emerald, EMERALD_PHY_RX_EQ_CTL, value);

	return 0;

}
#endif

int emerald_intphy_init(struct emerald_nic *emerald)
{
	int i;
	u16 value = 0;

	for (i=0;i<15;i++) {
		if (!emerald_rd32m(emerald, EMERALD_MIS_ST,
						EMERALD_MIS_ST_GPHY_IN_RST(emerald->port))) {
			break;
		}
		msleep(1);
	}
	if (i == 15) {
		return EMERALD_ERR_GPHY;
	}

	for (i = 0; i<1000;i++) {
		emerald_phy_read_reg(emerald, 29, 0xa43, &value);
		if (value & 0x20)
			break;
	}
	//if (i == 1000)
		//XXDBG ( "*0***efuse enable excced 1000 tries****\n");

	emerald_phy_write_reg(emerald, 20, 0xa46, 1);
	for (i = 0; i<1000;i++) {
		emerald_phy_read_reg(emerald, 29, 0xa43, &value);
		if (value & 0x20)
			break;
	}
	//if (i == 1000)
	//	XXDBG ( "*1***efuse enable excced 1000 tries****\n");

	emerald_phy_write_reg(emerald, 20, 0xa46, 2);
	for (i = 0; i<1000;i++) {
		emerald_phy_read_reg(emerald, 29, 0xa43, &value);
		if (value & 0x20)
			break;
	}
	//if (i == 1000)
	//	XXDBG ( "*2***efuse enable excced 1000 tries****\n");

	for (i = 0; i<1000;i++) {
		emerald_phy_read_reg(emerald, 16, 0xa42, &value);
		if ((value & 0x7) == 3)
			break;
	}

	//if (i == 1000)
	//	XXDBG ("*0***wait lanon excced 1000 tries****\n");
	value = 0x205B;
	emerald_phy_write_reg(emerald, 16, 0xd04, value);
	emerald_phy_write_reg(emerald, 17, 0xd04, 0);

	return 0;
}



//#if 0

int emerald_setup_link_internal(struct emerald_nic *emerald,
						uint32_t speed)
{
//	int i;
//	u16 value = 0;
	u16 emerald_phy_bmcr = 0;

	if (speed & EMERALD_LINK_SPEED_1GB_FULL) {
		emerald_phy_bmcr |= EMERALD_PHY_SPEED_SELECT1;
	} else if (speed & EMERALD_LINK_SPEED_100M_FULL) {
		emerald_phy_bmcr |= EMERALD_PHY_SPEED_SELECT0;
	} else if (speed & EMERALD_LINK_SPEED_10M_FULL)
		emerald_phy_bmcr |= 0;
	else
		return EMERALD_ERR_LINK_SPEED;

	/* set duplex full and autoneg on */
	emerald_phy_bmcr |= EMERALD_PHY_DUPLEX | EMERALD_PHY_ANE;

	emerald_phy_write_reg(emerald, 0, 0, emerald_phy_bmcr);
	emerald_phy_bmcr = 0;
	emerald_phy_read_reg(emerald, 0, 0, &emerald_phy_bmcr);


	return 0;
}

//#endif



int emerald_setup_link_mdi(struct emerald_nic *emerald,
						uint32_t speed)
{
	uint32_t i;
	uint16_t value = 0;
	int flag = 0;
	int ret = 0;

	emerald_read_phy_reg_mdi(emerald, 9, 0x01, &value);
	if (value) {
		flag = 1;
		value = 0;
		emerald_write_phy_reg_mdi(emerald, 9, 0x01, value);
	}

	/* set phy to 100M-FULL */
	if (speed == EMERALD_LINK_SPEED_100M_FULL) {
		ret = emerald_read_phy_reg_mdi(emerald, 0, 0x01, &value);
		if (ret || flag || (value != 0x3100)) {
			value = EMERALD_EXPHY_DUPLEX |
					EMERALD_EXPHY_AUTONEG |
					EMERALD_EXPHY_SPEED_SELECT_L |
					EMERALD_EXPHY_RESET;
			emerald_write_phy_reg_mdi(emerald, 0, 0x01, value);
		}
	}

	/* set phy to 1GB-FULL */
	if (speed == EMERALD_LINK_SPEED_1GB_FULL) {
		ret = emerald_read_phy_reg_mdi(emerald, 0, 0x01, &value);
		if (ret || flag || (value != 0x1140)) {
			value = EMERALD_EXPHY_DUPLEX |
					EMERALD_EXPHY_AUTONEG |
					EMERALD_EXPHY_SPEED_SELECT_H |
					EMERALD_EXPHY_RESET;
			emerald_write_phy_reg_mdi(emerald, 0, 0x01, value);
		}
	}

	//select page 2
	emerald_write_phy_reg_mdi(emerald, 22, 0x01, 0x2);
	ret = emerald_read_phy_reg_mdi(emerald, 21, 0x01, &value);
	if (!ret) {
		value |= (~EMERALD_EXPHY_TX_TIMING_CTRL) |
				 EMERALD_EXPHY_RX_TIMING_CTRL;
		emerald_write_phy_reg_mdi(emerald, 21, 0x01, value);
	}

	//reset phy
	ret = emerald_read_phy_reg_mdi(emerald, 0, 0x01, &value);
	if (!ret) {
		value |= EMERALD_EXPHY_RESET;
		emerald_write_phy_reg_mdi(emerald, 0, 0x01, value);
	}

	for (i = 0; i < EMERALD_EXPHY_RESET_POLLING_TIME; i++) {
		if (!emerald_read_phy_reg_mdi(emerald, 0, 0x01, &value)) {
			if (!(value & EMERALD_EXPHY_RESET))
			break;
		} else
			return EMERALD_ERR_PHY;
		msleep(1);
	}

	if (i == EMERALD_EXPHY_RESET_POLLING_TIME)
		DBGC ( emerald, "EMERALD phy reset timeout\n" );

	return ret;
}



int emerald_setup_link(struct emerald_nic *emerald,
						uint32_t speed)
{

	if (speed != EMERALD_LINK_SPEED_1GB_FULL &&
		speed != EMERALD_LINK_SPEED_100M_FULL)
		return EMERALD_ERR_INVALID_PARAM;

	emerald_wr32m(emerald, EMERALD_MAC_TX_CFG,
					EMERALD_MAC_TX_CFG_TE,
					~EMERALD_MAC_TX_CFG_TE);

//	writel(0x1, emerald->regs + EMERALD_GPIO_DR);
//	writel(0x1, emerald->regs + EMERALD_GPIO_DDR);
	writel(0xF, emerald->regs + EMERALD_MDIO_CLAUSE22);

	return emerald_setup_link_internal(emerald, speed);
//return 0;
#if 0
	switch (emerald->subsystem_device) {
		case 0x0000:
			return emerald_setup_link_internal(emerald, speed);
		case 0x0100:
		case 0x0200:
			return emerald_setup_link_mdi(emerald, speed);
		default:
			DBGC ( emerald, "EMERALD unknown subsystem id\n" );
			return EMERALD_ERR_DEV_ID;
	}
#endif
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
 * @v emerald		Sapphire device
 * @ret rc		Return status code
 */
static int emerald_reset ( struct net_device *netdev ) {
	uint32_t ctrl;
	struct emerald_nic *emerald = netdev->priv;

	/* Perform a global software reset */
	ctrl = readl ( emerald->regs + EMERALD_MIS_RST);
	if (emerald->port == 0)
		writel ( ( ctrl | EMERALD_MIS_RST_LAN0_RST),
			emerald->regs + EMERALD_MIS_RST );
	else if (emerald->port == 1)
		writel ( ( ctrl | EMERALD_MIS_RST_LAN1_RST),
			emerald->regs + EMERALD_MIS_RST );
	else if (emerald->port == 2)
		writel ( ( ctrl | EMERALD_MIS_RST_LAN2_RST),
			emerald->regs + EMERALD_MIS_RST );
	else
		writel ( ( ctrl | EMERALD_MIS_RST_LAN3_RST),
			emerald->regs + EMERALD_MIS_RST );


	mdelay ( EMERALD_RESET_DELAY_MS );


	//XXDBG("setup---------------%d------\n", emerald->port);
	emerald_intphy_init(emerald);
	emerald_setup_link(emerald, EMERALD_LINK_SPEED_1GB_FULL);

	if (emerald->link_up)
		emerald_check_link(netdev);

	return 0;
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
static int emerald_open ( struct net_device *netdev ) {
	struct emerald_nic *emerald = netdev->priv;
	//union emerald_receive_address mac;
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
	uint32_t rx_dis;
	uint32_t tx_dis;

	uint16_t value = 0;
	/* Set initial link state */


	//emerald_intphy_init(emerald);

	//if ( ! emerald_link_is_up(emerald) )
	//	emerald_setup_link(emerald, EMERALD_LINK_SPEED_1GB_FULL);

	writel(42 << 10, emerald->regs + 0x19020);

	/* Create transmit descriptor ring */
	if ( ( rc = emerald_create_ring ( emerald, &emerald->tx, 1 ) ) != 0 )
		goto err_create_tx;

	/* Create receive descriptor ring */
	if ( ( rc = emerald_create_ring ( emerald, &emerald->rx, 0 ) ) != 0 )
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
	writel ( ral0, emerald->regs + INTELX_RAL0 );
	writel ( rah0, emerald->regs + INTELX_RAH0 );
	writel ( ral0, emerald->regs + INTELX_RAL0_ALT );
	writel ( rah0, emerald->regs + INTELX_RAH0_ALT );
#endif

	/* Allocate interrupt vectors */
	writel ( ( EMERALD_PX_IVAR_RX0_DEFAULT | EMERALD_PX_IVAR_RX0_VALID |
		   EMERALD_PX_IVAR_TX0_DEFAULT | EMERALD_PX_IVAR_TX0_VALID ),
		 emerald->regs + EMERALD_PX_IVAR0 );
	writel((EMERALD_PX_MISC_IVAR_DEFAULT | EMERALD_PX_MISC_IVAR_VALID),
		emerald->regs + EMERALD_PX_MISC_IVAR);
	writel(EMERALD_DEFAULT_ITR | EMERALD_PX_ITR_CNT_WDIS,
		emerald->regs + EMERALD_PX_ITR0);

	emerald->isb_dma = virt_to_bus(emerald->isb_mem);
	writel(emerald->isb_dma & 0xffffffff, emerald->regs + EMERALD_PX_ISB_ADDR_L);
	if ( sizeof ( physaddr_t ) > sizeof ( uint32_t ) )
		writel((uint64_t)(emerald->isb_dma) >> 32, emerald->regs + EMERALD_PX_ISB_ADDR_H);
	else
		writel(0, emerald->regs + EMERALD_PX_ISB_ADDR_H);
#if 0
	/* Enable transmitter  */
	dmatxctl = readl ( emerald->regs + EMERALD_DMATXCTL );
	dmatxctl |= EMERALD_DMATXCTL_TE;
	writel ( dmatxctl, emerald->regs + EMERALD_DMATXCTL );
#endif
	mactxcfg = readl(emerald->regs + EMERALD_MACTXCFG);

//emerald
//	if (emerald->speed == EMERALD_LINK_SPEED_1GB_FULL)
		mactxcfg = (mactxcfg & ~EMERALD_MACTXCFG_SPEED_MASK) |
				EMERALD_MACTXCFG_SPEED_1G |
				EMERALD_MACTXCFG_TE;
//	else
//		mactxcfg = (mactxcfg & ~EMERALD_MACTXCFG_SPEED_MASK) |
//				EMERALD_MACTXCFG_SPEED_10G |
//				EMERALD_MACTXCFG_TE;
	writel(mactxcfg, emerald->regs + EMERALD_MACTXCFG);

	/* Configure receive filter */
	fctrl = readl ( emerald->regs + EMERALD_PSR_CTL );
	fctrl |= (EMERALD_PSR_CTL_BAM | EMERALD_PSR_CTL_UPE |
		EMERALD_PSR_CTL_MPE);
	writel ( fctrl, emerald->regs + EMERALD_PSR_CTL );

	vlanctl = readl ( emerald->regs + EMERALD_PSR_VLAN_CTL);
	vlanctl &= ~EMERALD_PSR_VLAN_CTL_VFE;
	writel ( vlanctl, emerald->regs + EMERALD_PSR_VLAN_CTL );

	flow_ctrl = readl ( emerald->regs + EMERALD_MAC_RX_FLOW_CTRL );
	flow_ctrl |= EMERALD_MAC_RX_FLOW_CTRL_RFE;
	writel ( flow_ctrl, emerald->regs + EMERALD_MAC_RX_FLOW_CTRL );

	pkt_flt = readl ( emerald->regs + EMERALD_MAC_PKT_FLT );
	pkt_flt |= EMERALD_MAC_PKT_FLT_PR;
	writel ( pkt_flt, emerald->regs + EMERALD_MAC_PKT_FLT );

	rx_dis = readl ( emerald->regs + EMERALD_RSEC_CTL );
	rx_dis &= ~0x2;
	writel ( rx_dis, emerald->regs + EMERALD_RSEC_CTL );

	tx_dis = readl ( emerald->regs + EMERALD_TSEC_CTL );
	tx_dis &= ~0x2;
	writel ( tx_dis, emerald->regs + EMERALD_TSEC_CTL );

	/* Configure jumbo frames.  Required to allow the extra 4-byte
	 * headroom for VLANs, since we don't use the hardware's
	 * native VLAN offload.
	 */
	hlreg0 = readl ( emerald->regs + EMERALD_MAC_RX_CFG );
	hlreg0 |= EMERALD_MAC_RX_CFG_JE;
	writel ( hlreg0, emerald->regs + EMERALD_MAC_RX_CFG );

	/* Configure frame size */
	maxfrs = EMERALD_MAXFRS_MFS_DEFAULT;
	writel ( maxfrs, emerald->regs + EMERALD_PSR_MAX_SZ );

	/* Configure receive DMA */
	rdrxctl = readl ( emerald->regs + EMERALD_RSEC_CTL );
	rdrxctl |= EMERALD_RSEC_CTL_CRC_STRIP;
	writel ( rdrxctl, emerald->regs + EMERALD_RSEC_CTL );
#if 0
	/* Enable receiver */
	rxctrl = readl ( emerald->regs + EMERALD_MAC_RX_CFG );
	rxctrl |= EMERALD_MAC_RX_CFG_RE;
	writel ( rxctrl, emerald->regs + EMERALD_MAC_RX_CFG );
	rxctrl = readl(emerald->regs + EMERALD_RDB_PB_CTL);
	rxctrl |= EMERALD_RDB_PB_CTL_RXEN;
	writel ( rxctrl, emerald->regs + EMERALD_RDB_PB_CTL );
#endif
	/* Fill receive ring */
	emerald_refill_rx ( emerald );

	/* Enable transmitter  */
	dmatxctl = readl ( emerald->regs + EMERALD_DMATXCTL );
	dmatxctl |= EMERALD_DMATXCTL_TE;
	writel ( dmatxctl, emerald->regs + EMERALD_DMATXCTL );

	/* Enable receiver */
	rxctrl = readl ( emerald->regs + EMERALD_MAC_RX_CFG );
	rxctrl |= EMERALD_MAC_RX_CFG_RE;
	writel ( rxctrl, emerald->regs + EMERALD_MAC_RX_CFG );
	rxctrl = readl(emerald->regs + EMERALD_RDB_PB_CTL);
	rxctrl |= EMERALD_RDB_PB_CTL_RXEN;
	writel ( rxctrl, emerald->regs + EMERALD_RDB_PB_CTL );

	//XXDBG("hhhhhhhhhhhhhhhhhhhh\n");

	//emerald_reset(netdev);

	/* Set initial link state */
//	if ( ! emerald_link_is_up(emerald))
	//	emerald_setup_link(emerald, EMERALD_LINK_SPEED_1GB_FULL);

	/* Update link state */
//import............	emerald_check_link ( netdev );

	emerald_check_link ( netdev );
	//XXDBG("====open=====:%x\n", value);

#if 0
#ifdef LEGACY
	mdelay(1000);
	XXDBG("legacy=======\n");
#endif
#endif

//	if (emerald_link_is_up(emerald)) {
//		netdev_link_up ( netdev );
//	} else
//		netdev_link_down ( netdev );

	emerald_phy_read_reg(emerald, 0x1A, 0xA43, &value);
//	XXDBG("====open=====:%x\n", value);

	return 0;

err_create_rx:
	emerald_destroy_ring ( emerald, &emerald->rx );
err_create_tx:
	emerald_destroy_ring ( emerald, &emerald->tx );

	return rc;
}

/**
 * Close network device
 *
 * @v netdev		Network device
 */
static void emerald_close ( struct net_device *netdev ) {
	struct emerald_nic *emerald = netdev->priv;
	uint32_t rxctrl;
	uint32_t dmatxctl;

	/* Disable receiver */
	rxctrl = readl ( emerald->regs + EMERALD_MAC_RX_CFG );
	rxctrl &= ~EMERALD_MAC_RX_CFG_RE;
	writel ( rxctrl, emerald->regs + EMERALD_MAC_RX_CFG );
	rxctrl = readl(emerald->regs + EMERALD_RDB_PB_CTL);
	rxctrl &= ~EMERALD_RDB_PB_CTL_RXEN;
	writel ( rxctrl, emerald->regs + EMERALD_RDB_PB_CTL );


	/* Disable transmitter  */
	dmatxctl = readl ( emerald->regs + EMERALD_DMATXCTL );
	dmatxctl &= ~EMERALD_DMATXCTL_TE;
	writel ( dmatxctl, emerald->regs + EMERALD_DMATXCTL );
	dmatxctl = readl(emerald->regs + EMERALD_MACTXCFG);
	dmatxctl &= ~EMERALD_MACTXCFG_TE;
	writel(dmatxctl, emerald->regs + EMERALD_MACTXCFG);

	/* Destroy receive descriptor ring */
	emerald_destroy_ring ( emerald, &emerald->rx );

	/* Discard any unused receive buffers */
	emerald_empty_rx ( emerald );

	/* Destroy transmit descriptor ring */
	emerald_destroy_ring ( emerald, &emerald->tx );

	/* Reset the NIC, to flush the transmit and receive FIFOs */
//temp
	emerald_reset ( netdev );
}

/**
 * Poll for completed packets
 *
 * @v netdev		Network device
 */
void emerald_poll_tx ( struct net_device *netdev ) {
	struct emerald_nic *emerald = netdev->priv;
	union emerald_descriptor *tx;
	unsigned int tx_idx;

	/* Check for completed packets */
	while ( emerald->tx.cons != emerald->tx.prod ) {

		/* Get next transmit descriptor */
		tx_idx = ( emerald->tx.cons % EMERALD_NUM_TX_DESC );
		tx = &emerald->tx.desc[tx_idx];

		/* Stop if descriptor is still in use */
		if ( ! ( tx->tx.status & cpu_to_le32 ( EMERALD_DESC_STATUS_DD ) ) )
			return;

		DBGC2 ( emerald, "EMERALD %p TX %d complete\n", emerald, tx_idx );

		/* Complete TX descriptor */
		netdev_tx_complete_next ( netdev );
		emerald->tx.cons++;
	}
}

/**
 * Poll for received packets
 *
 * @v netdev		Network device
 */
void emerald_poll_rx ( struct net_device *netdev ) {
	struct emerald_nic *emerald = netdev->priv;
	union emerald_descriptor *rx;
	struct io_buffer *iobuf;
	unsigned int rx_idx;
	size_t len;

	/* Check for received packets */
	while ( emerald->rx.cons != emerald->rx.prod ) {

		/* Get next receive descriptor */
		rx_idx = ( emerald->rx.cons % EMERALD_NUM_RX_DESC );
		rx = &emerald->rx.desc[rx_idx];

		/* Stop if descriptor is still in use */
		if ( ! ( rx->rx.status_error & cpu_to_le32 ( EMERALD_DESC_STATUS_DD ) ) )
			return;

		/* Populate I/O buffer */
		iobuf = emerald->rx_iobuf[rx_idx];
		emerald->rx_iobuf[rx_idx] = NULL;
		len = le16_to_cpu ( rx->rx.length );
		iob_put ( iobuf, len );

		/* Hand off to network stack */
		if ( rx->rx.status_error & cpu_to_le32 ( EMERALD_DESC_STATUS_RXE ) ) {
			DBGC ( emerald, "EMERALD %p RX %d error (length %zd, "
			       "status %08x)\n", emerald, rx_idx, len,
			       le32_to_cpu ( rx->rx.status_error ) );
			netdev_rx_err ( netdev, iobuf, -EIO );
		} else {
			DBGC2 ( emerald, "EMERALD %p RX %d complete (length %zd)\n",
				emerald, rx_idx, len );
			netdev_rx ( netdev, iobuf );
		}
		emerald->rx.cons++;
	}
}


/**
 * Poll for completed and received packets
 *
 * @v netdev		Network device
 */
static void emerald_poll ( struct net_device *netdev ) {
	struct emerald_nic *emerald = netdev->priv;
	uint32_t reg = 0;
	uint32_t rxcfg = 0;
//	uint32_t eicr, eicr_misc;
// DBGC2 ( emerald, "EMERALD %p in poll\n",
//                                emerald);

	/* Check for and acknowledge interrupts */
	profile_start ( &emerald_vm_poll_profiler );
//	eicr = emerald->isb_mem[EMERALD_ISB_VEC0];
	profile_stop ( &emerald_vm_poll_profiler );
	profile_exclude ( &emerald_vm_poll_profiler );
//	if ( ! eicr )
//		return;
//	eicr_misc = emerald->isb_mem[EMERALD_ISB_MISC];
	/* Poll for TX completions, if applicable */
	emerald_poll_tx ( netdev );

	/* Poll for RX completions, if applicable */
	emerald_poll_rx ( netdev );

	/* Check link state, if applicable */
//	if ( eicr_misc & (EMERALD_PX_MISC_IC_ETH_LK | EMERALD_PX_MISC_IC_ETH_LKDN) )
//		emerald_check_link ( netdev );

	emerald->count++;
	if (!(emerald->count % 10000)) {
		rxcfg |= EMERALD_MAC_RX_CFG_JE | EMERALD_MAC_RX_CFG_RE;
		writel ( rxcfg, emerald->regs + EMERALD_MAC_RX_CFG );
		writel ( EMERALD_MAC_PKT_FLT_PR,
					emerald->regs + EMERALD_MAC_PKT_FLT );
		reg = readl ( emerald->regs + EMERALD_MAC_WDG_TIMEOUT );
		writel ( reg, emerald->regs + EMERALD_MAC_WDG_TIMEOUT );
//		emerald_check_link ( netdev );
		if (emerald_link_is_up(emerald)) {
			netdev_link_up ( netdev );
		} else
			netdev_link_down ( netdev );
	}

	/* Refill RX ring */
	emerald_refill_rx ( emerald );
}

/**
 * Enable or disable interrupts
 *
 * @v netdev		Network device
 * @v enable		Interrupts should be enabled
 */
static void emerald_irq ( struct net_device *netdev, int enable ) {
	struct emerald_nic *emerald = netdev->priv;
	uint32_t mask;
	uint32_t qmask;

	mask = EMERALD_PX_MISC_IEN_ETH_LKDN | EMERALD_PX_MISC_IEN_ETH_LK;
	writel(mask, emerald->regs + EMERALD_PX_MISC_IEN);
//	mask = ( INTELX_EIRQ_LSC | INTELX_EIRQ_RXO | INTELX_EIRQ_TX0 |
//		 INTELX_EIRQ_RX0 );
	qmask = 0x3;
	if ( enable ) {
		writel ( qmask, emerald->regs + EMERALD_PX_IMC0 );
	} else {
		writel ( qmask, emerald->regs + EMERALD_PX_IMS0 );
	}
	DBGC2 ( emerald, "EMERALD %p irq is enabled? [%d)\n", emerald, enable);
}

/**
 * Transmit packet
 *
 * @v netdev		Network device
 * @v iobuf		I/O buffer
 * @ret rc		Return status code
 */
int emerald_transmit ( struct net_device *netdev, struct io_buffer *iobuf ) {
	struct emerald_nic *emerald = netdev->priv;
	union emerald_descriptor *tx;
	unsigned int tx_idx;
	unsigned int tx_tail;
	physaddr_t address;
	size_t len;
writel ( 0x1, emerald->regs + 0x130 );
	/* Get next transmit descriptor */
	if ( ( emerald->tx.prod - emerald->tx.cons ) >= EMERALD_TX_FILL ) {
		DBGC ( emerald, "EMERALD %p out of transmit descriptors\n", emerald );
		return -ENOBUFS;
	}
	tx_idx = ( emerald->tx.prod++ % EMERALD_NUM_TX_DESC );
	tx_tail = ( emerald->tx.prod % EMERALD_NUM_TX_DESC );
	tx = &emerald->tx.desc[tx_idx];

	/* Populate transmit descriptor */
	address = virt_to_bus ( iobuf->data );
	len = iob_len ( iobuf );
	emerald->tx.describe ( tx, address, len );
	wmb();

	/* Notify card that there are packets ready to transmit */
	profile_start ( &emerald_vm_tx_profiler );
	writel ( tx_tail, emerald->regs + emerald->tx.reg + EMERALD_DH );
	profile_stop ( &emerald_vm_tx_profiler );
	profile_exclude ( &emerald_vm_tx_profiler );

	DBGC2 ( emerald, "EMERALD %p TX %d is [%llx,%llx)\n", emerald, tx_idx,
		( ( unsigned long long ) address ),
		( ( unsigned long long ) address + len ) );

	return 0;
}


#ifndef PMON
/** Network device operations */
static struct net_device_operations emerald_operations = {
	.open		= emerald_open,
	.close		= emerald_close,
	.transmit	= emerald_transmit,
	.poll		= emerald_poll,
	.irq		= emerald_irq,
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
static int emerald_probe ( struct pci_device *pci ) {
	struct net_device *netdev;
	struct emerald_nic *emerald;
	int rc;

//	uint16_t value = 0;

	/* Allocate and initialise net device */
	netdev = alloc_etherdev ( sizeof ( *emerald ) );
	if ( ! netdev ) {
		rc = -ENOMEM;
		goto err_alloc;
	}
	netdev_init ( netdev, &emerald_operations );
	emerald = netdev->priv;
#ifndef PMON
	pci_set_drvdata ( pci, netdev );
	netdev->dev = &pci->dev;
#endif
	memset ( emerald, 0, sizeof ( *emerald ) );
	emerald->port = PCI_FUNC ( pci->busdevfn );
	emerald_init_ring ( &emerald->tx, EMERALD_NUM_TX_DESC, EMERALD_TD,
			  emerald_describe_tx );
	emerald_init_ring ( &emerald->rx, EMERALD_NUM_RX_DESC, EMERALD_RD,
			  emerald_describe_rx );

	/* Fix up PCI device */
	adjust_pci_device ( pci );

	/* Map registers */
	emerald->regs = ioremap ( pci->membase, INTEL_BAR_SIZE );
	if ( ! emerald->regs ) {
		rc = -ENODEV;
		goto err_ioremap;
	}
	printf("regs = 0x%x\n", emerald->regs);

	emerald->pdev = pci;
#ifndef PMON
	pci_read_config_word(emerald->pdev, PCI_SUBSYSTEM_ID,
						&emerald->subsystem_device);
#endif

	/* init phy */
	if (emerald->port == 0)
		emerald->phy_addr = 3;
	else
		emerald->phy_addr = 0;

	/* Reset the NIC */
	if ( ( rc = emerald_reset ( netdev ) ) != 0 )
		goto err_reset;
	/* Fetch MAC address */
	if ( ( rc = emerald_fetch_mac ( emerald, netdev->hw_addr ) ) != 0 )
		goto err_fetch_mac;

	/* Register network device */
	if ( ( rc = register_netdev ( netdev ) ) != 0 )
		goto err_register_netdev;

//	emerald_intphy_init(emerald);

	/* Set initial link state */
//	if ( ! emerald_link_is_up(emerald))
//		emerald_setup_link(emerald, EMERALD_LINK_SPEED_1GB_FULL);

#if 0
	if ( ! ( readl ( emerald->regs + EMERALD_CFG_PORT_ST )
		& EMERALD_CFG_PORT_ST_LINK_UP))
		emerald_setup_link(emerald, EMERALD_LINK_SPEED_10GB_FULL);
#endif
//	emerald_check_link ( netdev );
//	emerald_phy_read_reg(emerald, 0x1A, 0xA43, &value);




	return 0;

 err_register_netdev:
	unregister_netdev ( netdev );
 err_fetch_mac:
	emerald_reset ( netdev );
 err_reset:
	iounmap ( emerald->regs );
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
static void emerald_remove ( struct pci_device *pci ) {
	struct net_device *netdev = pci_get_drvdata ( pci );
	struct emerald_nic *emerald = netdev->priv;

	/* Unregister network device */
	unregister_netdev ( netdev );

	/* Reset the NIC */
	emerald_reset ( netdev );

	/* Free network device */
	iounmap ( emerald->regs );
	netdev_nullify ( netdev );
	netdev_put ( netdev );
}
#endif

/** PCI device IDs */
static struct pci_device_id emerald_nics[] = {
	PCI_ROM ( 0x8088, 0x0101, "emerald-rgmii-A2", "emerald (RJ45)", 0 ),
	PCI_ROM ( 0x8088, 0x0102, "emerald-rgmii-A2S", "emerald (RJ45)", 0 ),
	PCI_ROM ( 0x8088, 0x0103, "emerald-rgmii-A4", "emerald (RJ45)", 0 ),
	PCI_ROM ( 0x8088, 0x0104, "emerald-rgmii-A4S", "emerald (RJ45)", 0 ),
	PCI_ROM ( 0x8088, 0x0105, "emerald-rgmii-AL2", "emerald (RJ45)", 0 ),
	PCI_ROM ( 0x8088, 0x0106, "emerald-rgmii-AL2S", "emerald (RJ45)", 0 ),
	PCI_ROM ( 0x8088, 0x0107, "emerald-rgmii-AL4", "emerald (RJ45)", 0 ),
	PCI_ROM ( 0x8088, 0x0108, "emerald-rgmii-AL4S", "emerald (RJ45)", 0 ),
	PCI_ROM ( 0x8088, 0x0100, "emerald-rgmii-AL-W", "emerald (RJ45)", 0 ),

//	PCI_ROM ( 0x8088, 0x1002, "emerald-combo-backplane", "emerald (combined backplane; KR/KX4/KX)", 0 ),
//	PCI_ROM ( 0x8088, 0x1003, "emerald-xaui", "emerald (XAUI)", 0 ),
//	PCI_ROM ( 0x8088, 0x1004, "emerald-sgmii", "emerald (SGMII)", 0 ),
};

#ifndef PMON
/** PCI driver */
struct pci_driver emerald_driver __pci_driver = {
	.ids = emerald_nics,
	.id_count = ( sizeof ( emerald_nics ) / sizeof ( emerald_nics[0] ) ),
	.probe = emerald_probe,
	.remove = emerald_remove,
};
#else
#include "ngbe_pmon_tail.h"
#endif
