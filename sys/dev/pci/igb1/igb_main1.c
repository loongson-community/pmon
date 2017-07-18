#include "linux_net_head.h"
enum latency_range {
	lowest_latency = 0,
	low_latency = 1,
	bulk_latency = 2,
	latency_invalid = 255
};
static int debug = NETIF_MSG_DRV | NETIF_MSG_PROBE;

static const struct pci_device_id igb_pci_tbl[] = {
	{ PCI_VDEVICE(INTEL, E1000_DEV_ID_I354_BACKPLANE_1GBPS) },
	{ PCI_VDEVICE(INTEL, E1000_DEV_ID_I354_SGMII) },
	{ PCI_VDEVICE(INTEL, E1000_DEV_ID_I354_BACKPLANE_2_5GBPS) },
	{ PCI_VDEVICE(INTEL, E1000_DEV_ID_I210_COPPER) },
	{ PCI_VDEVICE(INTEL, E1000_DEV_ID_I210_FIBER) },
	{ PCI_VDEVICE(INTEL, E1000_DEV_ID_I210_SERDES) },
	{ PCI_VDEVICE(INTEL, E1000_DEV_ID_I210_SGMII) },
	{ PCI_VDEVICE(INTEL, E1000_DEV_ID_I210_COPPER_FLASHLESS) },
	{ PCI_VDEVICE(INTEL, E1000_DEV_ID_I210_SERDES_FLASHLESS) },
	{ PCI_VDEVICE(INTEL, E1000_DEV_ID_I211_COPPER) },
	{ PCI_VDEVICE(INTEL, E1000_DEV_ID_I350_COPPER) },
	{ PCI_VDEVICE(INTEL, E1000_DEV_ID_I350_FIBER) },
	{ PCI_VDEVICE(INTEL, E1000_DEV_ID_I350_SERDES) },
	{ PCI_VDEVICE(INTEL, E1000_DEV_ID_I350_SGMII) },
	{ PCI_VDEVICE(INTEL, E1000_DEV_ID_82580_COPPER) },
	{ PCI_VDEVICE(INTEL, E1000_DEV_ID_82580_FIBER) },
	{ PCI_VDEVICE(INTEL, E1000_DEV_ID_82580_QUAD_FIBER) },
	{ PCI_VDEVICE(INTEL, E1000_DEV_ID_82580_SERDES) },
	{ PCI_VDEVICE(INTEL, E1000_DEV_ID_82580_SGMII) },
	{ PCI_VDEVICE(INTEL, E1000_DEV_ID_82580_COPPER_DUAL) },
	{ PCI_VDEVICE(INTEL, E1000_DEV_ID_DH89XXCC_SGMII) },
	{ PCI_VDEVICE(INTEL, E1000_DEV_ID_DH89XXCC_SERDES) },
	{ PCI_VDEVICE(INTEL, E1000_DEV_ID_DH89XXCC_BACKPLANE) },
	{ PCI_VDEVICE(INTEL, E1000_DEV_ID_DH89XXCC_SFP) },
	{ PCI_VDEVICE(INTEL, E1000_DEV_ID_82576) },
	{ PCI_VDEVICE(INTEL, E1000_DEV_ID_82576_NS) },
	{ PCI_VDEVICE(INTEL, E1000_DEV_ID_82576_NS_SERDES) },
	{ PCI_VDEVICE(INTEL, E1000_DEV_ID_82576_FIBER) },
	{ PCI_VDEVICE(INTEL, E1000_DEV_ID_82576_SERDES) },
	{ PCI_VDEVICE(INTEL, E1000_DEV_ID_82576_SERDES_QUAD) },
	{ PCI_VDEVICE(INTEL, E1000_DEV_ID_82576_QUAD_COPPER_ET2) },
	{ PCI_VDEVICE(INTEL, E1000_DEV_ID_82576_QUAD_COPPER) },
	{ PCI_VDEVICE(INTEL, E1000_DEV_ID_82575EB_COPPER) },
	{ PCI_VDEVICE(INTEL, E1000_DEV_ID_82575EB_FIBER_SERDES) },
	{ PCI_VDEVICE(INTEL, E1000_DEV_ID_82575GB_QUAD_COPPER) },
	/* required last entry */
	{0, }
};


u32 e1000_read_reg(struct e1000_hw *hw, u32 reg)
{
	struct igb_adapter *igb = container_of(hw, struct igb_adapter, hw);
	u8 __iomem *hw_addr = ACCESS_ONCE(hw->hw_addr);
	u32 value = 0;

	if (E1000_REMOVED(hw_addr))
		return ~value;

	value = readl(&hw_addr[reg]);

	/* reads should not return all F's */
	if (!(~value) && (!reg || !(~readl(hw_addr)))) {
		struct net_device *netdev = igb->netdev;
		hw->hw_addr = NULL;
		//netif_device_detach(netdev);
		netdev_err(netdev, "PCIe link lost, device now detached\n");
	}

	return value;
}


void e1000_read_pci_cfg(struct e1000_hw *hw, u32 reg, u16 *value)
{
	struct igb_adapter *adapter = hw->back;

	pci_read_config_word(adapter->pdev, reg, value);
}

void e1000_write_pci_cfg(struct e1000_hw *hw, u32 reg, u16 *value)
{
	struct igb_adapter *adapter = hw->back;

	pci_write_config_word(adapter->pdev, reg, *value);
}


s32 e1000_read_pcie_cap_reg(struct e1000_hw *hw, u32 reg, u16 *value)
{
	struct igb_adapter *adapter = hw->back;
	u16 cap_offset;

	cap_offset = pci_find_capability(adapter->pdev, PCI_CAP_ID_EXP);
	if (!cap_offset)
		return -E1000_ERR_CONFIG;

	pci_read_config_word(adapter->pdev, cap_offset + reg, value);

	return E1000_SUCCESS;
}


s32 e1000_write_pcie_cap_reg(struct e1000_hw *hw, u32 reg, u16 *value)
{
	struct igb_adapter *adapter = hw->back;
	u16 cap_offset;

	cap_offset = pci_find_capability(adapter->pdev, PCI_CAP_ID_EXP);
	if (!cap_offset)
		return -E1000_ERR_CONFIG;

	pci_write_config_word(adapter->pdev, cap_offset + reg, *value);

	return E1000_SUCCESS;
}

//-----------------------------------------------

/**
 * igb_setup_rx_resources - allocate Rx resources (Descriptors)
 * @rx_ring:    rx descriptor ring (for a specific queue) to setup
 *
 * Returns 0 on success, negative on failure
 **/
int igb_setup_rx_resources(struct igb_ring *rx_ring)
{
	struct device *dev = rx_ring->dev;
	int size, desc_len;

	size = sizeof(struct igb_rx_buffer) * rx_ring->count;
	rx_ring->rx_buffer_info = vzalloc(size);
	if (!rx_ring->rx_buffer_info)
		goto err;

	desc_len = sizeof(union e1000_adv_rx_desc);

	/* Round up to nearest 4K */
	rx_ring->size = rx_ring->count * desc_len;
	rx_ring->size = ALIGN(rx_ring->size, 4096);

	rx_ring->desc = dma_alloc_coherent(dev, rx_ring->size,
					   &rx_ring->dma, GFP_KERNEL);

	if (!rx_ring->desc)
		goto err;

	rx_ring->next_to_alloc = 0;
	rx_ring->next_to_clean = 0;
	rx_ring->next_to_use = 0;

	return 0;

err:
	vfree(rx_ring->rx_buffer_info);
	rx_ring->rx_buffer_info = NULL;
	dev_err(dev,
		"Unable to allocate memory for the receive descriptor ring\n");
	return -ENOMEM;
}

/**
 * igb_setup_all_rx_resources - wrapper to allocate Rx resources
 *				  (Descriptors) for all queues
 * @adapter: board private structure
 *
 * Return 0 on success, negative on failure
 **/
static int igb_setup_all_rx_resources(struct igb_adapter *adapter)
{
	struct pci_dev *pdev = adapter->pdev;
	int i, err = 0;

	for (i = 0; i < adapter->num_rx_queues; i++) {
		err = igb_setup_rx_resources(adapter->rx_ring[i]);
		if (err) {
			dev_err(pci_dev_to_dev(pdev),
				"Allocation for Rx Queue %u failed\n", i);
			for (i--; i >= 0; i--)
				igb_free_rx_resources(adapter->rx_ring[i]);
			break;
		}
	}

	return err;
}

/**
 * igb_setup_tx_resources - allocate Tx resources (Descriptors)
 * @tx_ring: tx descriptor ring (for a specific queue) to setup
 *
 * Return 0 on success, negative on failure
 **/
int igb_setup_tx_resources(struct igb_ring *tx_ring)
{
	struct device *dev = tx_ring->dev;
	int size;

	size = sizeof(struct igb_tx_buffer) * tx_ring->count;
	tx_ring->tx_buffer_info = vzalloc(size);
	if (!tx_ring->tx_buffer_info)
		goto err;

	/* round up to nearest 4K */
	tx_ring->size = tx_ring->count * sizeof(union e1000_adv_tx_desc);
	tx_ring->size = ALIGN(tx_ring->size, 4096);

	tx_ring->desc = dma_alloc_coherent(dev, tx_ring->size,
					   &tx_ring->dma, GFP_KERNEL);

	if (!tx_ring->desc)
		goto err;

	tx_ring->next_to_use = 0;
	tx_ring->next_to_clean = 0;

	return 0;

err:
	vfree(tx_ring->tx_buffer_info);
	dev_err(dev,
		"Unable to allocate memory for the transmit descriptor ring\n");
	return -ENOMEM;
}


/**
 * igb_setup_all_tx_resources - wrapper to allocate Tx resources
 *				  (Descriptors) for all queues
 * @adapter: board private structure
 *
 * Return 0 on success, negative on failure
 **/
static int igb_setup_all_tx_resources(struct igb_adapter *adapter)
{
	struct pci_dev *pdev = adapter->pdev;
	int i, err = 0;

	for (i = 0; i < adapter->num_tx_queues; i++) {
		err = igb_setup_tx_resources(adapter->tx_ring[i]);
		if (err) {
			dev_err(pci_dev_to_dev(pdev),
				"Allocation for Tx Queue %u failed\n", i);
			for (i--; i >= 0; i--)
				igb_free_tx_resources(adapter->tx_ring[i]);
			break;
		}
	}

	return err;
}


/**
 * igb_set_rx_mode - Secondary Unicast, Multicast and Promiscuous mode set
 * @netdev: network interface device structure
 *
 * The set_rx_mode entry point is called whenever the unicast or multicast
 * address lists or the network interface flags are updated.  This routine is
 * responsible for configuring the hardware for proper unicast, multicast,
 * promiscuous mode, and all-multi behavior.
 **/
static void igb_set_rx_mode(struct net_device *netdev)
{
	struct igb_adapter *adapter = netdev_priv(netdev);
	struct e1000_hw *hw = &adapter->hw;
	unsigned int vfn = adapter->vfs_allocated_count;
	u32 rctl, vmolr = 0;
	int count;

	/* Check for Promiscuous and All Multicast modes */
	rctl = E1000_READ_REG(hw, E1000_RCTL);

	/* clear the effected bits */
	rctl &= ~(E1000_RCTL_UPE | E1000_RCTL_MPE | E1000_RCTL_VFE);

	if (netdev->flags & IFF_PROMISC) {
		rctl |= (E1000_RCTL_UPE | E1000_RCTL_MPE);
		vmolr |= (E1000_VMOLR_ROPE | E1000_VMOLR_MPME);
		/* retain VLAN HW filtering if in VT mode */
		if (adapter->vfs_allocated_count || adapter->vmdq_pools)
			rctl |= E1000_RCTL_VFE;
	} else {
		if (netdev->flags & IFF_ALLMULTI) {
			rctl |= E1000_RCTL_MPE;
			vmolr |= E1000_VMOLR_MPME;
		} else {
			/*
			 * Write addresses to the MTA, if the attempt fails
			 * then we should just turn on promiscuous mode so
			 * that we can at least receive multicast traffic
			 */
#ifndef PMON
			count = igb_write_mc_addr_list(netdev);
			if (count < 0) {
				rctl |= E1000_RCTL_MPE;
				vmolr |= E1000_VMOLR_MPME;
			} else if (count) {
				vmolr |= E1000_VMOLR_ROMPE;
			}
#endif
		}
#ifdef HAVE_SET_RX_MODE
		/*
		 * Write addresses to available RAR registers, if there is not
		 * sufficient space to store all the addresses then enable
		 * unicast promiscuous mode
		 */
		count = igb_write_uc_addr_list(netdev);
		if (count < 0) {
			rctl |= E1000_RCTL_UPE;
			vmolr |= E1000_VMOLR_ROPE;
		}
#endif /* HAVE_SET_RX_MODE */
		rctl |= E1000_RCTL_VFE;
	}
	E1000_WRITE_REG(hw, E1000_RCTL, rctl);

	/*
	 * In order to support SR-IOV and eventually VMDq it is necessary to set
	 * the VMOLR to enable the appropriate modes.  Without this workaround
	 * we will have issues with VLAN tag stripping not being done for frames
	 * that are only arriving because we are the default pool
	 */
	if (hw->mac.type < e1000_82576)
		return;

	vmolr |= E1000_READ_REG(hw, E1000_VMOLR(vfn)) &
		~(E1000_VMOLR_ROPE | E1000_VMOLR_MPME | E1000_VMOLR_ROMPE);
	E1000_WRITE_REG(hw, E1000_VMOLR(vfn), vmolr);
#ifndef PMON
	igb_restore_vf_multicasts(adapter);
#endif
}

/**
 * igb_setup_tctl - configure the transmit control registers
 * @adapter: Board private structure
 **/
void igb_setup_tctl(struct igb_adapter *adapter)
{
	struct e1000_hw *hw = &adapter->hw;
	u32 tctl;

	/* disable queue 0 which is enabled by default on 82575 and 82576 */
	E1000_WRITE_REG(hw, E1000_TXDCTL(0), 0);

	/* Program the Transmit Control Register */
	tctl = E1000_READ_REG(hw, E1000_TCTL);
	tctl &= ~E1000_TCTL_CT;
	tctl |= E1000_TCTL_PSP | E1000_TCTL_RTLC |
		(E1000_COLLISION_THRESHOLD << E1000_CT_SHIFT);

	igb_e1000_config_collision_dist(hw);

	/* Enable transmits */
	tctl |= E1000_TCTL_EN;

	E1000_WRITE_REG(hw, E1000_TCTL, tctl);
}

/**
 *  igb_set_uta - Set unicast filter table address
 *  @adapter: board private structure
 *
 *  The unicast table address is a register array of 32-bit registers.
 *  The table is meant to be used in a way similar to how the MTA is used
 *  however due to certain limitations in the hardware it is necessary to
 *  set all the hash bits to 1 and use the VMOLR ROPE bit as a promiscuous
 *  enable bit to allow vlan tag stripping when promiscuous mode is enabled
 **/
static void igb_set_uta(struct igb_adapter *adapter)
{
	struct e1000_hw *hw = &adapter->hw;
	int i;

	/* The UTA table only exists on 82576 hardware and newer */
	if (hw->mac.type < e1000_82576)
		return;

	/* we only need to do this if VMDq is enabled */
	if (!adapter->vmdq_pools)
		return;

	for (i = 0; i < hw->mac.uta_reg_count; i++)
		E1000_WRITE_REG_ARRAY(hw, E1000_UTA, i, ~0);
}


static void igb_rar_set_qsel(struct igb_adapter *adapter, u8 *addr, u32 index,
			     u8 qsel)
{
	u32 rar_low, rar_high;
	struct e1000_hw *hw = &adapter->hw;

	/* HW expects these in little endian so we reverse the byte order
	 * from network order (big endian) to little endian
	 */
	rar_low = ((u32) addr[0] | ((u32) addr[1] << 8) |
		   ((u32) addr[2] << 16) | ((u32) addr[3] << 24));
	rar_high = ((u32) addr[4] | ((u32) addr[5] << 8));

	/* Indicate to hardware the Address is Valid. */
	rar_high |= E1000_RAH_AV;

	if (hw->mac.type == e1000_82575)
		rar_high |= E1000_RAH_POOL_1 * qsel;
	else
		rar_high |= E1000_RAH_POOL_1 << qsel;

	E1000_WRITE_REG(hw, E1000_RAL(index), rar_low);
	E1000_WRITE_FLUSH(hw);
	E1000_WRITE_REG(hw, E1000_RAH(index), rar_high);
	E1000_WRITE_FLUSH(hw);
}


static inline void igb_set_vmolr(struct igb_adapter *adapter,
				 int vfn, bool aupe)
{
	struct e1000_hw *hw = &adapter->hw;
	u32 vmolr;

	/*
	 * This register exists only on 82576 and newer so if we are older then
	 * we should exit and do nothing
	 */
	if (hw->mac.type < e1000_82576)
		return;

	vmolr = E1000_READ_REG(hw, E1000_VMOLR(vfn));

	if (aupe)
		vmolr |= E1000_VMOLR_AUPE;        /* Accept untagged packets */
	else
		vmolr &= ~(E1000_VMOLR_AUPE); /* Tagged packets ONLY */

	/* clear all bits that might not be set */
	vmolr &= ~E1000_VMOLR_RSSE;

	if (adapter->rss_queues > 1 && vfn == adapter->vfs_allocated_count)
		vmolr |= E1000_VMOLR_RSSE; /* enable RSS */

	vmolr |= E1000_VMOLR_BAM;	   /* Accept broadcast */
	vmolr |= E1000_VMOLR_LPE;	   /* Accept long packets */

	E1000_WRITE_REG(hw, E1000_VMOLR(vfn), vmolr);
}


/**
 * igb_configure_rx_ring - Configure a receive ring after Reset
 * @adapter: board private structure
 * @ring: receive ring to be configured
 *
 * Configure the Rx unit of the MAC after a reset.
 **/
void igb_configure_rx_ring(struct igb_adapter *adapter,
			   struct igb_ring *ring)
{
	struct e1000_hw *hw = &adapter->hw;
	u64 rdba = ring->dma;
	int reg_idx = ring->reg_idx;
	u32 srrctl = 0, rxdctl = 0;

#ifdef CONFIG_IGB_DISABLE_PACKET_SPLIT
	/*
	 * RLPML prevents us from receiving a frame larger than max_frame so
	 * it is safe to just set the rx_buffer_len to max_frame without the
	 * risk of an skb over panic.
	 */
	ring->rx_buffer_len = max_t(u32, adapter->max_frame_size,
				    MAXIMUM_ETHERNET_VLAN_SIZE);

#endif
	/* disable the queue */
	E1000_WRITE_REG(hw, E1000_RXDCTL(reg_idx), 0);

	/* Set DMA base address registers */
	E1000_WRITE_REG(hw, E1000_RDBAL(reg_idx),
			rdba & 0x00000000ffffffffULL);
	E1000_WRITE_REG(hw, E1000_RDBAH(reg_idx), rdba >> 32);
	E1000_WRITE_REG(hw, E1000_RDLEN(reg_idx),
			ring->count * sizeof(union e1000_adv_rx_desc));

	/* initialize head and tail */
	ring->tail = hw->hw_addr + E1000_RDT(reg_idx);
	E1000_WRITE_REG(hw, E1000_RDH(reg_idx), 0);
	writel(0, ring->tail);

	/* reset next-to- use/clean to place SW in sync with hardwdare */
	ring->next_to_clean = 0;
	ring->next_to_use = 0;
#ifndef CONFIG_IGB_DISABLE_PACKET_SPLIT
	ring->next_to_alloc = 0;

#endif
	/* set descriptor configuration */
#ifndef CONFIG_IGB_DISABLE_PACKET_SPLIT
	srrctl = IGB_RX_HDR_LEN << E1000_SRRCTL_BSIZEHDRSIZE_SHIFT;
	srrctl |= IGB_RX_BUFSZ >> E1000_SRRCTL_BSIZEPKT_SHIFT;
#else /* CONFIG_IGB_DISABLE_PACKET_SPLIT */
	srrctl = ALIGN(ring->rx_buffer_len, 1024) >>
		E1000_SRRCTL_BSIZEPKT_SHIFT;
#endif /* CONFIG_IGB_DISABLE_PACKET_SPLIT */
	srrctl |= E1000_SRRCTL_DESCTYPE_ADV_ONEBUF;
#ifdef HAVE_PTP_1588_CLOCK
	if (hw->mac.type >= e1000_82580)
		srrctl |= E1000_SRRCTL_TIMESTAMP;
#endif /* HAVE_PTP_1588_CLOCK */
	/*
	 * We should set the drop enable bit if:
	 *  SR-IOV is enabled
	 *   or
	 *  Flow Control is disabled and number of RX queues > 1
	 *
	 *  This allows us to avoid head of line blocking for security
	 *  and performance reasons.
	 */
	if (adapter->vfs_allocated_count ||
	    (adapter->num_rx_queues > 1 &&
	     (hw->fc.requested_mode == e1000_fc_none ||
	      hw->fc.requested_mode == e1000_fc_rx_pause)))
		srrctl |= E1000_SRRCTL_DROP_EN;

	E1000_WRITE_REG(hw, E1000_SRRCTL(reg_idx), srrctl);

	/* set filtering for VMDQ pools */
	igb_set_vmolr(adapter, reg_idx & 0x7, true);

	rxdctl |= IGB_RX_PTHRESH;
	rxdctl |= IGB_RX_HTHRESH << 8;
	rxdctl |= IGB_RX_WTHRESH << 16;

	/* enable receive descriptor fetching */
	rxdctl |= E1000_RXDCTL_QUEUE_ENABLE;
	E1000_WRITE_REG(hw, E1000_RXDCTL(reg_idx), rxdctl);
}

/**
 * igb_configure_rx - Configure receive Unit after Reset
 * @adapter: board private structure
 *
 * Configure the Rx unit of the MAC after a reset.
 **/
static void igb_configure_rx(struct igb_adapter *adapter)
{
	int i;

	/* set UTA to appropriate mode */
	igb_set_uta(adapter);

	/* set the correct pool for the PF default MAC address in entry 0 */
	igb_rar_set_qsel(adapter, adapter->hw.mac.addr, 0,
			 adapter->vfs_allocated_count);

	/* Setup the HW Rx Head and Tail Descriptor Pointers and
	 * the Base and Length of the Rx Descriptor Ring */
	for (i = 0; i < adapter->num_rx_queues; i++)
		igb_configure_rx_ring(adapter, adapter->rx_ring[i]);
}

static u32 igb_tx_wthresh(struct igb_adapter *adapter)
{
	struct e1000_hw *hw = &adapter->hw;
	switch (hw->mac.type) {
	case e1000_i354:
		return 4;
	case e1000_82576:
		if (adapter->msix_entries)
			return 1;
	default:
		break;
	}

	return 16;
}

/**
 * igb_configure_tx_ring - Configure transmit ring after Reset
 * @adapter: board private structure
 * @ring: tx ring to configure
 *
 * Configure a transmit ring after a reset.
 **/
void igb_configure_tx_ring(struct igb_adapter *adapter,
			   struct igb_ring *ring)
{
	struct e1000_hw *hw = &adapter->hw;
	u32 txdctl = 0;
	u64 tdba = ring->dma;
	int reg_idx = ring->reg_idx;

	/* disable the queue */
	E1000_WRITE_REG(hw, E1000_TXDCTL(reg_idx), 0);
	E1000_WRITE_FLUSH(hw);
	mdelay(10);

	E1000_WRITE_REG(hw, E1000_TDLEN(reg_idx),
			ring->count * sizeof(union e1000_adv_tx_desc));
	E1000_WRITE_REG(hw, E1000_TDBAL(reg_idx),
			tdba & 0x00000000ffffffffULL);
	E1000_WRITE_REG(hw, E1000_TDBAH(reg_idx), tdba >> 32);

	ring->tail = hw->hw_addr + E1000_TDT(reg_idx);
	E1000_WRITE_REG(hw, E1000_TDH(reg_idx), 0);
	writel(0, ring->tail);

	txdctl |= IGB_TX_PTHRESH;
	txdctl |= IGB_TX_HTHRESH << 8;
	txdctl |= igb_tx_wthresh(adapter) << 16;

	txdctl |= E1000_TXDCTL_QUEUE_ENABLE;
	E1000_WRITE_REG(hw, E1000_TXDCTL(reg_idx), txdctl);
}

/**
 * igb_configure_tx - Configure transmit Unit after Reset
 * @adapter: board private structure
 *
 * Configure the Tx unit of the MAC after a reset.
 **/
static void igb_configure_tx(struct igb_adapter *adapter)
{
	int i;

	for (i = 0; i < adapter->num_tx_queues; i++)
		igb_configure_tx_ring(adapter, adapter->tx_ring[i]);
}

/**
 * igb_setup_rctl - configure the receive control registers
 * @adapter: Board private structure
 **/
void igb_setup_rctl(struct igb_adapter *adapter)
{
	struct e1000_hw *hw = &adapter->hw;
	u32 rctl;

	rctl = E1000_READ_REG(hw, E1000_RCTL);

	rctl &= ~(3 << E1000_RCTL_MO_SHIFT);
	rctl &= ~(E1000_RCTL_LBM_TCVR | E1000_RCTL_LBM_MAC);

	rctl |= E1000_RCTL_EN | E1000_RCTL_BAM | E1000_RCTL_RDMTS_HALF |
		(hw->mac.mc_filter_type << E1000_RCTL_MO_SHIFT);

	/*
	 * enable stripping of CRC. It's unlikely this will break BMC
	 * redirection as it did with e1000. Newer features require
	 * that the HW strips the CRC.
	 */
	rctl |= E1000_RCTL_SECRC;

	/* disable store bad packets and clear size bits. */
	rctl &= ~(E1000_RCTL_SBP | E1000_RCTL_SZ_256);

	/* enable LPE to prevent packets larger than max_frame_size */
	rctl |= E1000_RCTL_LPE;

	/* disable queue 0 to prevent tail write w/o re-config */
	E1000_WRITE_REG(hw, E1000_RXDCTL(0), 0);

	/* Attention!!!  For SR-IOV PF driver operations you must enable
	 * queue drop for all VF and PF queues to prevent head of line blocking
	 * if an un-trusted VF does not provide descriptors to hardware.
	 */
	if (adapter->vfs_allocated_count) {
		/* set all queue drop enable bits */
		E1000_WRITE_REG(hw, E1000_QDE, ALL_QUEUES);
	}

	E1000_WRITE_REG(hw, E1000_RCTL, rctl);
}


/**
 * igb_enable_mdd
 * @adapter - board private structure
 *
 * Enable the HW to detect malicious driver and sends an interrupt to
 * the driver.
 **/
static void igb_enable_mdd(struct igb_adapter *adapter)
{
	struct e1000_hw *hw = &adapter->hw;
	u32 reg;

	/* Only available on i350 device */
	if (hw->mac.type != e1000_i350)
		return;

	reg = E1000_READ_REG(hw, E1000_DTXCTL);
	reg |= E1000_DTXCTL_MDP_EN;
	E1000_WRITE_REG(hw, E1000_DTXCTL, reg);
}

static void igb_vmm_control(struct igb_adapter *adapter)
{
	struct e1000_hw *hw = &adapter->hw;
	int count;
	u32 reg;

	switch (hw->mac.type) {
	case e1000_82575:
	default:
		/* replication is not supported for 82575 */
		return;
	case e1000_82576:
		/* notify HW that the MAC is adding vlan tags */
		reg = E1000_READ_REG(hw, E1000_DTXCTL);
		reg |= (E1000_DTXCTL_VLAN_ADDED |
			E1000_DTXCTL_SPOOF_INT);
		E1000_WRITE_REG(hw, E1000_DTXCTL, reg);
		/* Fall through */
	case e1000_82580:
		/* enable replication vlan tag stripping */
		reg = E1000_READ_REG(hw, E1000_RPLOLR);
		reg |= E1000_RPLOLR_STRVLAN;
		E1000_WRITE_REG(hw, E1000_RPLOLR, reg);
		/* Fall through */
	case e1000_i350:
	case e1000_i354:
		/* none of the above registers are supported by i350 */
		break;
	}

	/* Enable Malicious Driver Detection */
	if ((adapter->vfs_allocated_count) &&
	    (adapter->mdd)) {
		if (hw->mac.type == e1000_i350)
			igb_enable_mdd(adapter);
	}

		/* enable replication and loopback support */
		count = adapter->vfs_allocated_count || adapter->vmdq_pools;
		if (adapter->flags & IGB_FLAG_LOOPBACK_ENABLE && count)
			e1000_vmdq_set_loopback_pf(hw, 1);
		e1000_vmdq_set_anti_spoofing_pf(hw,
			adapter->vfs_allocated_count || adapter->vmdq_pools,
			adapter->vfs_allocated_count);
	e1000_vmdq_set_replication_pf(hw, adapter->vfs_allocated_count ||
				      adapter->vmdq_pools);
}

/**
 * igb_setup_mrqc - configure the multiple receive queue control registers
 * @adapter: Board private structure
 **/
static void igb_setup_mrqc(struct igb_adapter *adapter)
{
	struct e1000_hw *hw = &adapter->hw;
	u32 mrqc, rxcsum;
	u32 j, num_rx_queues;
#ifndef ETHTOOL_SRXFHINDIR
	u32 shift = 0, shift2 = 0;
#endif /* ETHTOOL_SRXFHINDIR */
	static const u32 rsskey[10] = { 0xDA565A6D, 0xC20E5B25, 0x3D256741,
					0xB08FA343, 0xCB2BCAD0, 0xB4307BAE,
					0xA32DCB77, 0x0CF23080, 0x3BB7426A,
					0xFA01ACBE };

	/* Fill out hash function seeds */
	for (j = 0; j < 10; j++)
		E1000_WRITE_REG(hw, E1000_RSSRK(j), rsskey[j]);

	num_rx_queues = adapter->rss_queues;

#ifdef ETHTOOL_SRXFHINDIR
	if (hw->mac.type == e1000_82576) {
		/* 82576 supports 2 RSS queues for SR-IOV */
		if (adapter->vfs_allocated_count)
			num_rx_queues = 2;
	}
	if (adapter->rss_indir_tbl_init != num_rx_queues) {
		for (j = 0; j < IGB_RETA_SIZE; j++)
			adapter->rss_indir_tbl[j] =
				(j * num_rx_queues) / IGB_RETA_SIZE;
		adapter->rss_indir_tbl_init = num_rx_queues;
	}
	igb_write_rss_indir_tbl(adapter);
#else
	/* 82575 and 82576 supports 2 RSS queues for VMDq */
	switch (hw->mac.type) {
	case e1000_82575:
		if (adapter->vmdq_pools) {
			shift = 2;
			shift2 = 6;
		}
		shift = 6;
		break;
	case e1000_82576:
		/* 82576 supports 2 RSS queues for SR-IOV */
		if (adapter->vfs_allocated_count || adapter->vmdq_pools) {
			shift = 3;
			num_rx_queues = 2;
		}
		break;
	default:
		break;
	}

	/*
	 * Populate the redirection table 4 entries at a time.  To do this
	 * we are generating the results for n and n+2 and then interleaving
	 * those with the results with n+1 and n+3.
	 */
	for (j = 0; j < 32; j++) {
		/* first pass generates n and n+2 */
		u32 base = ((j * 0x00040004) + 0x00020000) * num_rx_queues;
		u32 reta = (base & 0x07800780) >> (7 - shift);

		/* second pass generates n+1 and n+3 */
		base += 0x00010001 * num_rx_queues;
		reta |= (base & 0x07800780) << (1 + shift);

		/* generate 2nd table for 82575 based parts */
		if (shift2)
			reta |= (0x01010101 * num_rx_queues) << shift2;

		E1000_WRITE_REG(hw, E1000_RETA(j), reta);
	}
#endif /* ETHTOOL_SRXFHINDIR */

	/*
	 * Disable raw packet checksumming so that RSS hash is placed in
	 * descriptor on writeback.  No need to enable TCP/UDP/IP checksum
	 * offloads as they are enabled by default
	 */
	rxcsum = E1000_READ_REG(hw, E1000_RXCSUM);
	rxcsum |= E1000_RXCSUM_PCSD;

	if (adapter->hw.mac.type >= e1000_82576)
		/* Enable Receive Checksum Offload for SCTP */
		rxcsum |= E1000_RXCSUM_CRCOFL;

	/* Don't need to set TUOFL or IPOFL, they default to 1 */
	E1000_WRITE_REG(hw, E1000_RXCSUM, rxcsum);

	/* Generate RSS hash based on packet types, TCP/UDP
	 * port numbers and/or IPv4/v6 src and dst addresses
	 */
	mrqc = E1000_MRQC_RSS_FIELD_IPV4 |
	       E1000_MRQC_RSS_FIELD_IPV4_TCP |
	       E1000_MRQC_RSS_FIELD_IPV6 |
	       E1000_MRQC_RSS_FIELD_IPV6_TCP |
	       E1000_MRQC_RSS_FIELD_IPV6_TCP_EX;

	if (adapter->flags & IGB_FLAG_RSS_FIELD_IPV4_UDP)
		mrqc |= E1000_MRQC_RSS_FIELD_IPV4_UDP;
	if (adapter->flags & IGB_FLAG_RSS_FIELD_IPV6_UDP)
		mrqc |= E1000_MRQC_RSS_FIELD_IPV6_UDP;

	/* If VMDq is enabled then we set the appropriate mode for that, else
	 * we default to RSS so that an RSS hash is calculated per packet even
	 * if we are only using one queue */
	if (adapter->vfs_allocated_count || adapter->vmdq_pools) {
		if (hw->mac.type > e1000_82575) {
			/* Set the default pool for the PF's first queue */
			u32 vtctl = E1000_READ_REG(hw, E1000_VT_CTL);

			vtctl &= ~(E1000_VT_CTL_DEFAULT_POOL_MASK |
				   E1000_VT_CTL_DISABLE_DEF_POOL);
			vtctl |= adapter->vfs_allocated_count <<
				E1000_VT_CTL_DEFAULT_POOL_SHIFT;
			E1000_WRITE_REG(hw, E1000_VT_CTL, vtctl);
		}
		if (adapter->rss_queues > 1)
			mrqc |= E1000_MRQC_ENABLE_VMDQ_RSS_2Q;
		else
			mrqc |= E1000_MRQC_ENABLE_VMDQ;
	} else {
		if (hw->mac.type != e1000_i211)
			mrqc |= E1000_MRQC_ENABLE_RSS_4Q;
	}
	igb_vmm_control(adapter);

	E1000_WRITE_REG(hw, E1000_MRQC, mrqc);
}


/**
 * igb_get_hw_control - get control of the h/w from f/w
 * @adapter: address of board private structure
 *
 * igb_get_hw_control sets CTRL_EXT:DRV_LOAD bit.
 * For ASF and Pass Through versions of f/w this means that
 * the driver is loaded.
 *
 **/
static void igb_get_hw_control(struct igb_adapter *adapter)
{
	struct e1000_hw *hw = &adapter->hw;
	u32 ctrl_ext;

	/* Let firmware know the driver has taken over */
	ctrl_ext = E1000_READ_REG(hw, E1000_CTRL_EXT);
	E1000_WRITE_REG(hw, E1000_CTRL_EXT,
			ctrl_ext | E1000_CTRL_EXT_DRV_LOAD);
}


/**
 * igb_configure - configure the hardware for RX and TX
 * @adapter: private board structure
 **/
static void igb_configure(struct igb_adapter *adapter)
{
	struct net_device *netdev = adapter->netdev;
	int i;

	igb_get_hw_control(adapter);
	igb_set_rx_mode(netdev);

#ifndef PMON
	igb_restore_vlan(adapter);
#endif

	igb_setup_tctl(adapter);
	igb_setup_mrqc(adapter);
	igb_setup_rctl(adapter);

	igb_configure_tx(adapter);
	igb_configure_rx(adapter);

	e1000_rx_fifo_flush_82575(&adapter->hw);
#ifdef CONFIG_NETDEVICES_MULTIQUEUE
	if (adapter->num_tx_queues > 1)
		netdev->features |= NETIF_F_MULTI_QUEUE;
	else
		netdev->features &= ~NETIF_F_MULTI_QUEUE;
#endif

	/* call igb_desc_unused which always leaves
	 * at least 1 descriptor unused to make sure
	 * next_to_use != next_to_clean */
	for (i = 0; i < adapter->num_rx_queues; i++) {
		struct igb_ring *ring = adapter->rx_ring[i];
		igb_alloc_rx_buffers(ring, igb_desc_unused(ring));
	}
}

/**
 *  igb_write_ivar - configure ivar for given MSI-X vector
 *  @hw: pointer to the HW structure
 *  @msix_vector: vector number we are allocating to a given ring
 *  @index: row index of IVAR register to write within IVAR table
 *  @offset: column offset of in IVAR, should be multiple of 8
 *
 *  This function is intended to handle the writing of the IVAR register
 *  for adapters 82576 and newer.  The IVAR table consists of 2 columns,
 *  each containing an cause allocation for an Rx and Tx ring, and a
 *  variable number of rows depending on the number of queues supported.
 **/
static void igb_write_ivar(struct e1000_hw *hw, int msix_vector,
			   int index, int offset)
{
	u32 ivar = E1000_READ_REG_ARRAY(hw, E1000_IVAR0, index);

	/* clear any bits that are currently set */
	ivar &= ~((u32)0xFF << offset);

	/* write vector and valid bit */
	ivar |= (msix_vector | E1000_IVAR_VALID) << offset;

	E1000_WRITE_REG_ARRAY(hw, E1000_IVAR0, index, ivar);
}

#define IGB_N0_QUEUE -1
static void igb_assign_vector(struct igb_q_vector *q_vector, int msix_vector)
{
	struct igb_adapter *adapter = q_vector->adapter;
	struct e1000_hw *hw = &adapter->hw;
	int rx_queue = IGB_N0_QUEUE;
	int tx_queue = IGB_N0_QUEUE;
	u32 msixbm = 0;

	if (q_vector->rx.ring)
		rx_queue = q_vector->rx.ring->reg_idx;
	if (q_vector->tx.ring)
		tx_queue = q_vector->tx.ring->reg_idx;

	switch (hw->mac.type) {
	case e1000_82575:
		/* The 82575 assigns vectors using a bitmask, which matches the
		   bitmask for the EICR/EIMS/EIMC registers.  To assign one
		   or more queues to a vector, we write the appropriate bits
		   into the MSIXBM register for that vector. */
		if (rx_queue > IGB_N0_QUEUE)
			msixbm = E1000_EICR_RX_QUEUE0 << rx_queue;
		if (tx_queue > IGB_N0_QUEUE)
			msixbm |= E1000_EICR_TX_QUEUE0 << tx_queue;
		if (!adapter->msix_entries && msix_vector == 0)
			msixbm |= E1000_EIMS_OTHER;
		E1000_WRITE_REG_ARRAY(hw, E1000_MSIXBM(0), msix_vector, msixbm);
		q_vector->eims_value = msixbm;
		break;
	case e1000_82576:
		/*
		 * 82576 uses a table that essentially consists of 2 columns
		 * with 8 rows.  The ordering is column-major so we use the
		 * lower 3 bits as the row index, and the 4th bit as the
		 * column offset.
		 */
		if (rx_queue > IGB_N0_QUEUE)
			igb_write_ivar(hw, msix_vector,
				       rx_queue & 0x7,
				       (rx_queue & 0x8) << 1);
		if (tx_queue > IGB_N0_QUEUE)
			igb_write_ivar(hw, msix_vector,
				       tx_queue & 0x7,
				       ((tx_queue & 0x8) << 1) + 8);
		q_vector->eims_value = 1 << msix_vector;
		break;
	case e1000_82580:
	case e1000_i350:
	case e1000_i354:
	case e1000_i210:
	case e1000_i211:
		/*
		 * On 82580 and newer adapters the scheme is similar to 82576
		 * however instead of ordering column-major we have things
		 * ordered row-major.  So we traverse the table by using
		 * bit 0 as the column offset, and the remaining bits as the
		 * row index.
		 */
		if (rx_queue > IGB_N0_QUEUE)
			igb_write_ivar(hw, msix_vector,
				       rx_queue >> 1,
				       (rx_queue & 0x1) << 4);
		if (tx_queue > IGB_N0_QUEUE)
			igb_write_ivar(hw, msix_vector,
				       tx_queue >> 1,
				       ((tx_queue & 0x1) << 4) + 8);
		q_vector->eims_value = 1 << msix_vector;
		break;
	default:
		BUG();
		break;
	}

	/* add q_vector eims value to global eims_enable_mask */
	adapter->eims_enable_mask |= q_vector->eims_value;

	/* configure q_vector to set itr on first interrupt */
	q_vector->set_itr = 1;
}

static void igb_add_ring(struct igb_ring *ring,
			 struct igb_ring_container *head)
{
	head->ring = ring;
	head->count++;
}

/**
 * igb_alloc_q_vector - Allocate memory for a single interrupt vector
 * @adapter: board private structure to initialize
 * @v_count: q_vectors allocated on adapter, used for ring interleaving
 * @v_idx: index of vector in adapter struct
 * @txr_count: total number of Tx rings to allocate
 * @txr_idx: index of first Tx ring to allocate
 * @rxr_count: total number of Rx rings to allocate
 * @rxr_idx: index of first Rx ring to allocate
 *
 * We allocate one q_vector.  If allocation fails we return -ENOMEM.
 **/
static int igb_alloc_q_vector(struct igb_adapter *adapter,
			      unsigned int v_count, unsigned int v_idx,
			      unsigned int txr_count, unsigned int txr_idx,
			      unsigned int rxr_count, unsigned int rxr_idx)
{
	struct igb_q_vector *q_vector;
	struct igb_ring *ring;
	int ring_count, size;

	/* igb only supports 1 Tx and/or 1 Rx queue per vector */
	if (txr_count > 1 || rxr_count > 1)
		return -ENOMEM;

	ring_count = txr_count + rxr_count;
	size = sizeof(struct igb_q_vector) +
	       (sizeof(struct igb_ring) * ring_count);

	/* allocate q_vector and rings */
	q_vector = kzalloc(size, GFP_KERNEL);
	if (!q_vector)
		return -ENOMEM;

#ifndef IGB_NO_LRO
	/* initialize LRO */
	__skb_queue_head_init(&q_vector->lrolist.active);

#endif
#ifndef PMON
	/* initialize NAPI */
	netif_napi_add(adapter->netdev, &q_vector->napi,
		       igb_poll, 64);
#endif

	/* tie q_vector and adapter together */
	adapter->q_vector[v_idx] = q_vector;
	q_vector->adapter = adapter;

	/* initialize work limits */
	q_vector->tx.work_limit = adapter->tx_work_limit;

	/* initialize ITR configuration */
	q_vector->itr_register = adapter->hw.hw_addr + E1000_EITR(0);
	q_vector->itr_val = IGB_START_ITR;

	/* initialize pointer to rings */
	ring = q_vector->ring;

	/* intialize ITR */
	if (rxr_count) {
		/* rx or rx/tx vector */
		if (!adapter->rx_itr_setting || adapter->rx_itr_setting > 3)
			q_vector->itr_val = adapter->rx_itr_setting;
	} else {
		/* tx only vector */
		if (!adapter->tx_itr_setting || adapter->tx_itr_setting > 3)
			q_vector->itr_val = adapter->tx_itr_setting;
	}

	if (txr_count) {
		/* assign generic ring traits */
		ring->dev = &adapter->pdev->dev;
		ring->netdev = adapter->netdev;

		/* configure backlink on ring */
		ring->q_vector = q_vector;

		/* update q_vector Tx values */
		igb_add_ring(ring, &q_vector->tx);

		/* For 82575, context index must be unique per ring. */
		if (adapter->hw.mac.type == e1000_82575)
			set_bit(IGB_RING_FLAG_TX_CTX_IDX, &ring->flags);

		/* apply Tx specific ring traits */
		ring->count = adapter->tx_ring_count;
		ring->queue_index = txr_idx;

		/* assign ring to adapter */
		adapter->tx_ring[txr_idx] = ring;

		/* push pointer to next ring */
		ring++;
	}

	if (rxr_count) {
		/* assign generic ring traits */
		ring->dev = &adapter->pdev->dev;
		ring->netdev = adapter->netdev;

		/* configure backlink on ring */
		ring->q_vector = q_vector;

		/* update q_vector Rx values */
		igb_add_ring(ring, &q_vector->rx);

#ifndef HAVE_NDO_SET_FEATURES
		/* enable rx checksum */
		set_bit(IGB_RING_FLAG_RX_CSUM, &ring->flags);

#endif
		/* set flag indicating ring supports SCTP checksum offload */
		if (adapter->hw.mac.type >= e1000_82576)
			set_bit(IGB_RING_FLAG_RX_SCTP_CSUM, &ring->flags);

		if ((adapter->hw.mac.type == e1000_i350) ||
		    (adapter->hw.mac.type == e1000_i354))
			set_bit(IGB_RING_FLAG_RX_LB_VLAN_BSWAP, &ring->flags);

		/* apply Rx specific ring traits */
		ring->count = adapter->rx_ring_count;
		ring->queue_index = rxr_idx;

		/* assign ring to adapter */
		adapter->rx_ring[rxr_idx] = ring;
	}

	return 0;
}

/**
 * igb_alloc_q_vectors - Allocate memory for interrupt vectors
 * @adapter: board private structure to initialize
 *
 * We allocate one q_vector per queue interrupt.  If allocation fails we
 * return -ENOMEM.
 **/
static int igb_alloc_q_vectors(struct igb_adapter *adapter)
{
	int q_vectors = adapter->num_q_vectors;
	int rxr_remaining = adapter->num_rx_queues;
	int txr_remaining = adapter->num_tx_queues;
	int rxr_idx = 0, txr_idx = 0, v_idx = 0;
	int err;

	if (q_vectors >= (rxr_remaining + txr_remaining)) {
		for (; rxr_remaining; v_idx++) {
			err = igb_alloc_q_vector(adapter, q_vectors, v_idx,
						 0, 0, 1, rxr_idx);

			if (err)
				goto err_out;

			/* update counts and index */
			rxr_remaining--;
			rxr_idx++;
		}
	}

	for (; v_idx < q_vectors; v_idx++) {
		int rqpv = DIV_ROUND_UP(rxr_remaining, q_vectors - v_idx);
		int tqpv = DIV_ROUND_UP(txr_remaining, q_vectors - v_idx);

		err = igb_alloc_q_vector(adapter, q_vectors, v_idx,
					 tqpv, txr_idx, rqpv, rxr_idx);

		if (err)
			goto err_out;

		/* update counts and index */
		rxr_remaining -= rqpv;
		txr_remaining -= tqpv;
		rxr_idx++;
		txr_idx++;
	}

	return 0;

err_out:
	adapter->num_tx_queues = 0;
	adapter->num_rx_queues = 0;
	adapter->num_q_vectors = 0;

	while (v_idx--)
		igb_free_q_vector(adapter, v_idx);

	return -ENOMEM;
}


/**
 * igb_set_interrupt_capability - set MSI or MSI-X if supported
 *
 * Attempt to configure interrupts using the best available
 * capabilities of the hardware and kernel.
 **/
static void igb_set_interrupt_capability(struct igb_adapter *adapter, bool msix)
{
	struct pci_dev *pdev = adapter->pdev;
	int err;
	int numvecs, i;

	if (!msix)
		adapter->int_mode = IGB_INT_MODE_MSI;

	/* Number of supported queues. */
	adapter->num_rx_queues = adapter->rss_queues;

	if (adapter->vmdq_pools > 1)
		adapter->num_rx_queues += adapter->vmdq_pools - 1;

#ifdef HAVE_TX_MQ
	if (adapter->vmdq_pools)
		adapter->num_tx_queues = adapter->vmdq_pools;
	else
		adapter->num_tx_queues = adapter->num_rx_queues;
#else
	adapter->num_tx_queues = max_t(u32, 1, adapter->vmdq_pools);
#endif

	switch (adapter->int_mode) {
#ifndef PMON
	case IGB_INT_MODE_MSIX:
		/* start with one vector for every Tx/Rx queue */
		numvecs = max_t(int, adapter->num_tx_queues,
				adapter->num_rx_queues);

		/* if tx handler is seperate make it 1 for every queue */
		if (!(adapter->flags & IGB_FLAG_QUEUE_PAIRS))
			numvecs = adapter->num_tx_queues +
				adapter->num_rx_queues;

		/* store the number of vectors reserved for queues */
		adapter->num_q_vectors = numvecs;

		/* add 1 vector for link status interrupts */
		numvecs++;
		adapter->msix_entries = kcalloc(numvecs,
						sizeof(struct msix_entry),
						GFP_KERNEL);
		if (adapter->msix_entries) {
			for (i = 0; i < numvecs; i++)
				adapter->msix_entries[i].entry = i;

			err = pci_enable_msix(pdev,
					      adapter->msix_entries, numvecs);
			if (err == 0)
				break;
		}
		/* MSI-X failed, so fall through and try MSI */
		dev_warn(pci_dev_to_dev(pdev),
			 "Failed to initialize MSI-X interrupts. Falling back to MSI interrupts.\n");
		igb_reset_interrupt_capability(adapter);
	case IGB_INT_MODE_MSI:
		if (!pci_enable_msi(pdev))
			adapter->flags |= IGB_FLAG_HAS_MSI;
		else
			dev_warn(pci_dev_to_dev(pdev),
				"Failed to initialize MSI interrupts.  Falling back to legacy interrupts.\n");
		/* Fall through */
#endif
	case IGB_INT_MODE_LEGACY:
		/* disable advanced features and set number of queues to 1 */
		igb_reset_sriov_capability(adapter);
		adapter->vmdq_pools = 0;
		adapter->rss_queues = 1;
		adapter->flags |= IGB_FLAG_QUEUE_PAIRS;
		adapter->num_rx_queues = 1;
		adapter->num_tx_queues = 1;
		adapter->num_q_vectors = 1;
		/* Don't do anything; this is system default */
		break;
	}
}

#define Q_IDX_82576(i) (((i & 0x1) << 3) + (i >> 1))

/**
 * igb_cache_ring_register - Descriptor ring to register mapping
 * @adapter: board private structure to initialize
 *
 * Once we know the feature-set enabled for the device, we'll cache
 * the register offset the descriptor ring is assigned to.
 **/
static void igb_cache_ring_register(struct igb_adapter *adapter)
{
	int i = 0, j = 0;
	u32 rbase_offset = adapter->vfs_allocated_count;

	switch (adapter->hw.mac.type) {
	case e1000_82576:
		/* The queues are allocated for virtualization such that VF 0
		 * is allocated queues 0 and 8, VF 1 queues 1 and 9, etc.
		 * In order to avoid collision we start at the first free queue
		 * and continue consuming queues in the same sequence
		 */
		if ((adapter->rss_queues > 1) && adapter->vmdq_pools) {
			for (; i < adapter->rss_queues; i++)
				adapter->rx_ring[i]->reg_idx = rbase_offset +
								Q_IDX_82576(i);
		}
	break;
	case e1000_82575:
	case e1000_82580:
	case e1000_i350:
	case e1000_i354:
	case e1000_i210:
	case e1000_i211:
		/* Fall through */
	default:
		for (; i < adapter->num_rx_queues; i++)
			adapter->rx_ring[i]->reg_idx = rbase_offset + i;
		for (; j < adapter->num_tx_queues; j++)
			adapter->tx_ring[j]->reg_idx = rbase_offset + j;
		break;
	}
}

/**
 * igb_init_interrupt_scheme - initialize interrupts, allocate queues/vectors
 *
 * This function initializes the interrupts and allocates all of the queues.
 **/
static int igb_init_interrupt_scheme(struct igb_adapter *adapter, bool msix)
{
	struct pci_dev *pdev = adapter->pdev;
	int err;

	igb_set_interrupt_capability(adapter, msix);

	err = igb_alloc_q_vectors(adapter);
	if (err) {
		dev_err(pci_dev_to_dev(pdev), "Unable to allocate memory for vectors\n");
		goto err_alloc_q_vectors;
	}

	igb_cache_ring_register(adapter);

	return 0;

err_alloc_q_vectors:
	igb_reset_interrupt_capability(adapter);
	return err;
}

/**
 * igb_request_irq - initialize interrupts
 *
 * Attempts to configure interrupts using the best available
 * capabilities of the hardware and kernel.
 **/
static int igb_request_irq(struct igb_adapter *adapter)
{
	struct net_device *netdev = adapter->netdev;
	struct pci_dev *pdev = adapter->pdev;
	int err = 0;

#ifndef PMON
	if (adapter->msix_entries) {
		err = igb_request_msix(adapter);
		if (!err)
			goto request_done;
		/* fall back to MSI */
		igb_free_all_tx_resources(adapter);
		igb_free_all_rx_resources(adapter);

		igb_clear_interrupt_scheme(adapter);
		igb_reset_sriov_capability(adapter);
		err = igb_init_interrupt_scheme(adapter, false);
		if (err)
			goto request_done;
		igb_setup_all_tx_resources(adapter);
		igb_setup_all_rx_resources(adapter);
		igb_configure(adapter);
	}
#endif

	igb_assign_vector(adapter->q_vector[0], 0);

#ifndef PMON
	if (adapter->flags & IGB_FLAG_HAS_MSI) {
		err = request_irq(pdev->irq, &igb_intr_msi, 0,
				  netdev->name, adapter);
		if (!err)
			goto request_done;

		/* fall back to legacy interrupts */
		igb_reset_interrupt_capability(adapter);
		adapter->flags &= ~IGB_FLAG_HAS_MSI;
	}
#endif

	err = request_irq(pdev->irq, &igb_intr, IRQF_SHARED,
			  netdev->name, adapter);

	if (err)
		dev_err(pci_dev_to_dev(pdev), "Error %d getting interrupt\n",
			err);

request_done:
	return err;
}


static void igb_configure_lli(struct igb_adapter *adapter)
{
	struct e1000_hw *hw = &adapter->hw;
	u16 port;

	/* LLI should only be enabled for MSI-X or MSI interrupts */
	if (!adapter->msix_entries && !(adapter->flags & IGB_FLAG_HAS_MSI))
		return;

	if (adapter->lli_port) {
		/* use filter 0 for port */
		port = htons((u16)adapter->lli_port);
		E1000_WRITE_REG(hw, E1000_IMIR(0),
			(port | E1000_IMIR_PORT_IM_EN));
		E1000_WRITE_REG(hw, E1000_IMIREXT(0),
			(E1000_IMIREXT_SIZE_BP | E1000_IMIREXT_CTRL_BP));
	}

	if (adapter->flags & IGB_FLAG_LLI_PUSH) {
		/* use filter 1 for push flag */
		E1000_WRITE_REG(hw, E1000_IMIR(1),
			(E1000_IMIR_PORT_BP | E1000_IMIR_PORT_IM_EN));
		E1000_WRITE_REG(hw, E1000_IMIREXT(1),
			(E1000_IMIREXT_SIZE_BP | E1000_IMIREXT_CTRL_PSH));
	}

	if (adapter->lli_size) {
		/* use filter 2 for size */
		E1000_WRITE_REG(hw, E1000_IMIR(2),
			(E1000_IMIR_PORT_BP | E1000_IMIR_PORT_IM_EN));
		E1000_WRITE_REG(hw, E1000_IMIREXT(2),
			(adapter->lli_size | E1000_IMIREXT_CTRL_BP));
	}

}

/**
 * igb_irq_enable - Enable default interrupt generation settings
 * @adapter: board private structure
 **/
static void igb_irq_enable(struct igb_adapter *adapter)
{
	struct e1000_hw *hw = &adapter->hw;

#ifndef PMON
	if (adapter->msix_entries) {
		u32 ims = E1000_IMS_LSC | E1000_IMS_DOUTSYNC | E1000_IMS_DRSTA;
		u32 regval = E1000_READ_REG(hw, E1000_EIAC);

		E1000_WRITE_REG(hw, E1000_EIAC, regval
				| adapter->eims_enable_mask);
		regval = E1000_READ_REG(hw, E1000_EIAM);
		E1000_WRITE_REG(hw, E1000_EIAM, regval
				| adapter->eims_enable_mask);
		E1000_WRITE_REG(hw, E1000_EIMS, adapter->eims_enable_mask);
		if (adapter->vfs_allocated_count) {
			E1000_WRITE_REG(hw, E1000_MBVFIMR, 0xFF);
			ims |= E1000_IMS_VMMB;
			if (adapter->mdd)
				if ((adapter->hw.mac.type == e1000_i350) ||
				    (adapter->hw.mac.type == e1000_i354))
				ims |= E1000_IMS_MDDET;
		}
		E1000_WRITE_REG(hw, E1000_IMS, ims);
	} else 
#endif	
	{
		E1000_WRITE_REG(hw, E1000_IMS, IMS_ENABLE_MASK |
				E1000_IMS_DRSTA);
		E1000_WRITE_REG(hw, E1000_IAM, IMS_ENABLE_MASK |
				E1000_IMS_DRSTA);
	}
}

static void igb_free_irq(struct igb_adapter *adapter)
{
#ifndef PMON
	if (adapter->msix_entries) {
		int vector = 0, i;

		free_irq(adapter->msix_entries[vector++].vector, adapter);

		for (i = 0; i < adapter->num_q_vectors; i++)
			free_irq(adapter->msix_entries[vector++].vector,
				 adapter->q_vector[i]);
	} else 
#endif
	{
		free_irq(adapter->pdev->irq, adapter);
	}
}

/**
 * igb_release_hw_control - release control of the h/w to f/w
 * @adapter: address of board private structure
 *
 * igb_release_hw_control resets CTRL_EXT:DRV_LOAD bit.
 * For ASF and Pass Through versions of f/w this means that the
 * driver is no longer loaded.
 *
 **/
static void igb_release_hw_control(struct igb_adapter *adapter)
{
	struct e1000_hw *hw = &adapter->hw;
	u32 ctrl_ext;

	/* Let firmware take over control of h/w */
	ctrl_ext = E1000_READ_REG(hw, E1000_CTRL_EXT);
	E1000_WRITE_REG(hw, E1000_CTRL_EXT,
			ctrl_ext & ~E1000_CTRL_EXT_DRV_LOAD);
}

/**
 * igb_power_down_link - Power down the phy/serdes link
 * @adapter: address of board private structure
 */
static void igb_power_down_link(struct igb_adapter *adapter)
{
	if (adapter->hw.phy.media_type == e1000_media_type_copper)
		e1000_power_down_phy(&adapter->hw);
	else
		e1000_shutdown_fiber_serdes_link(&adapter->hw);
}


/**
 * igb_power_up_link - Power up the phy/serdes link
 * @adapter: address of board private structure
 **/
void igb_power_up_link(struct igb_adapter *adapter)
{
	igb_e1000_phy_hw_reset(&adapter->hw);

	if (adapter->hw.phy.media_type == e1000_media_type_copper)
		igb_e1000_power_up_phy(&adapter->hw);
	else
		e1000_power_up_fiber_serdes_link(&adapter->hw);
}

/**
 * igb_open - Called when a network interface is made active
 * @netdev: network interface device structure
 *
 * Returns 0 on success, negative value on failure
 *
 * The open entry point is called when a network interface is made
 * active by the system (IFF_UP).  At this point all resources needed
 * for transmit and receive operations are allocated, the interrupt
 * handler is registered with the OS, the watchdog timer is started,
 * and the stack is notified that the interface is ready.
 **/
static int __igb_open(struct net_device *netdev, bool resuming)
{
	struct igb_adapter *adapter = netdev_priv(netdev);
	struct e1000_hw *hw = &adapter->hw;
#ifdef CONFIG_PM_RUNTIME
	struct pci_dev *pdev = adapter->pdev;
#endif /* CONFIG_PM_RUNTIME */
	int err;
	int i;

#ifndef PMON
	/* disallow open during test */
	if (test_bit(__IGB_TESTING, &adapter->state)) {
		WARN_ON(resuming);
		return -EBUSY;
	}
#endif

#ifdef CONFIG_PM_RUNTIME
	if (!resuming)
		pm_runtime_get_sync(&pdev->dev);
#endif /* CONFIG_PM_RUNTIME */

	netif_carrier_off(netdev);

	/* allocate transmit descriptors */
	err = igb_setup_all_tx_resources(adapter);
	if (err)
		goto err_setup_tx;

	/* allocate receive descriptors */
	err = igb_setup_all_rx_resources(adapter);
	if (err)
		goto err_setup_rx;

	igb_power_up_link(adapter);

	/* before we allocate an interrupt, we must be ready to handle it.
	 * Setting DEBUG_SHIRQ in the kernel makes it fire an interrupt
	 * as soon as we call pci_request_irq, so we have to setup our
	 * clean_rx handler before we do so.  */
	igb_configure(adapter);

	err = igb_request_irq(adapter);
	if (err)
		goto err_req_irq;

	/* Notify the stack of the actual queue counts. */
	netif_set_real_num_tx_queues(netdev,
				     adapter->vmdq_pools ? 1 :
				     adapter->num_tx_queues);

	err = netif_set_real_num_rx_queues(netdev,
					   adapter->vmdq_pools ? 1 :
					   adapter->num_rx_queues);
	if (err)
		goto err_set_queues;

	/* From here on the code is the same as igb_up() */
	clear_bit(__IGB_DOWN, &adapter->state);

	for (i = 0; i < adapter->num_q_vectors; i++)
		napi_enable(&(adapter->q_vector[i]->napi));
	igb_configure_lli(adapter);

	/* Clear any pending interrupts. */
	E1000_READ_REG(hw, E1000_ICR);

	igb_irq_enable(adapter);

	/* notify VFs that reset has been completed */
	if (adapter->vfs_allocated_count) {
		u32 reg_data = E1000_READ_REG(hw, E1000_CTRL_EXT);

		reg_data |= E1000_CTRL_EXT_PFRSTD;
		E1000_WRITE_REG(hw, E1000_CTRL_EXT, reg_data);
	}

	netif_tx_start_all_queues(netdev);

	if (adapter->flags & IGB_FLAG_DETECT_BAD_DMA)
		schedule_work(&adapter->dma_err_task);

	/* start the watchdog. */
	hw->mac.get_link_status = 1;
	schedule_work(&adapter->watchdog_task);

	return E1000_SUCCESS;

err_set_queues:
	igb_free_irq(adapter);
err_req_irq:
	igb_release_hw_control(adapter);
	igb_power_down_link(adapter);
	igb_free_all_rx_resources(adapter);
err_setup_rx:
	igb_free_all_tx_resources(adapter);
err_setup_tx:
	igb_reset(adapter);

#ifdef CONFIG_PM_RUNTIME
	if (!resuming)
		pm_runtime_put(&pdev->dev);
#endif /* CONFIG_PM_RUNTIME */

	return err;
}

static int igb_open(struct net_device *netdev)
{
	return __igb_open(netdev, false);
}

#define IGB_SET_FLAG(_input, _flag, _result) \
	((_flag <= _result) ? \
	 ((u32)(_input & _flag) * (_result / _flag)) : \
	 ((u32)(_input & _flag) / (_flag / _result)))


static void igb_tx_olinfo_status(struct igb_ring *tx_ring,
				 union e1000_adv_tx_desc *tx_desc,
				 u32 tx_flags, unsigned int paylen)
{
	u32 olinfo_status = paylen << E1000_ADVTXD_PAYLEN_SHIFT;

	/* 82575 requires a unique index per ring */
	if (test_bit(IGB_RING_FLAG_TX_CTX_IDX, &tx_ring->flags))
		olinfo_status |= tx_ring->reg_idx << 4;

	/* insert L4 checksum */
	olinfo_status |= IGB_SET_FLAG(tx_flags,
				      IGB_TX_FLAGS_CSUM,
				      (E1000_TXD_POPTS_TXSM << 8));

	/* insert IPv4 checksum */
	olinfo_status |= IGB_SET_FLAG(tx_flags,
				      IGB_TX_FLAGS_IPV4,
				      (E1000_TXD_POPTS_IXSM << 8));

	tx_desc->read.olinfo_status = cpu_to_le32(olinfo_status);
}


static u32 igb_tx_cmd_type(struct sk_buff *skb, u32 tx_flags)
{
	/* set type for advanced descriptor with frame checksum insertion */
	u32 cmd_type = E1000_ADVTXD_DTYP_DATA |
		       E1000_ADVTXD_DCMD_DEXT |
		       E1000_ADVTXD_DCMD_IFCS;

	/* set HW vlan bit if vlan is present */
	cmd_type |= IGB_SET_FLAG(tx_flags, IGB_TX_FLAGS_VLAN,
				 (E1000_ADVTXD_DCMD_VLE));

	/* set segmentation bits for TSO */
	cmd_type |= IGB_SET_FLAG(tx_flags, IGB_TX_FLAGS_TSO,
				 (E1000_ADVTXD_DCMD_TSE));

	/* set timestamp bit if present */
	cmd_type |= IGB_SET_FLAG(tx_flags, IGB_TX_FLAGS_TSTAMP,
				 (E1000_ADVTXD_MAC_TSTAMP));

	return cmd_type;
}

static int __igb_maybe_stop_tx(struct igb_ring *tx_ring, const u16 size)
{
	struct net_device *netdev = netdev_ring(tx_ring);

	if (netif_is_multiqueue(netdev))
		netif_stop_subqueue(netdev, ring_queue_index(tx_ring));
	else
		netif_stop_queue(netdev);

	/* Herbert's original patch had:
	 *  smp_mb__after_netif_stop_queue();
	 * but since that doesn't exist yet, just open code it. */
	smp_mb();

	/* We need to check again in a case another CPU has just
	 * made room available. */
	if (igb_desc_unused(tx_ring) < size)
		return -EBUSY;

	/* A reprieve! */
	if (netif_is_multiqueue(netdev))
		netif_wake_subqueue(netdev, ring_queue_index(tx_ring));
	else
		netif_wake_queue(netdev);

	tx_ring->tx_stats.restart_queue++;

	return 0;
}

static inline int igb_maybe_stop_tx(struct igb_ring *tx_ring, const u16 size)
{
	if (igb_desc_unused(tx_ring) >= size)
		return 0;
	return __igb_maybe_stop_tx(tx_ring, size);
}


static void igb_tx_map(struct igb_ring *tx_ring,
		       struct igb_tx_buffer *first,
		       const u8 hdr_len)
{
	struct sk_buff *skb = first->skb;
	struct igb_tx_buffer *tx_buffer;
	union e1000_adv_tx_desc *tx_desc;
	struct skb_frag_struct *frag;
	dma_addr_t dma;
	unsigned int data_len, size;
	u32 tx_flags = first->tx_flags;
	u32 cmd_type = igb_tx_cmd_type(skb, tx_flags);
	u16 i = tx_ring->next_to_use;

	tx_desc = IGB_TX_DESC(tx_ring, i);

	igb_tx_olinfo_status(tx_ring, tx_desc, tx_flags, skb->len - hdr_len);

	size = skb_headlen(skb);
	data_len = skb->data_len;

	dma = dma_map_single(tx_ring->dev, skb->data, size, DMA_TO_DEVICE);

	tx_buffer = first;

#ifndef PMON
	for (frag = &skb_shinfo(skb)->frags[0];; frag++) {
		if (dma_mapping_error(tx_ring->dev, dma))
			goto dma_error;

		/* record length, and DMA address */
		dma_unmap_len_set(tx_buffer, len, size);
		dma_unmap_addr_set(tx_buffer, dma, dma);

		tx_desc->read.buffer_addr = cpu_to_le64(dma);

		while (unlikely(size > IGB_MAX_DATA_PER_TXD)) {
			tx_desc->read.cmd_type_len =
				cpu_to_le32(cmd_type ^ IGB_MAX_DATA_PER_TXD);

			i++;
			tx_desc++;
			if (i == tx_ring->count) {
				tx_desc = IGB_TX_DESC(tx_ring, 0);
				i = 0;
			}
			tx_desc->read.olinfo_status = 0;

			dma += IGB_MAX_DATA_PER_TXD;
			size -= IGB_MAX_DATA_PER_TXD;

			tx_desc->read.buffer_addr = cpu_to_le64(dma);
		}

		if (likely(!data_len))
			break;

		tx_desc->read.cmd_type_len = cpu_to_le32(cmd_type ^ size);

		i++;
		tx_desc++;
		if (i == tx_ring->count) {
			tx_desc = IGB_TX_DESC(tx_ring, 0);
			i = 0;
		}
		tx_desc->read.olinfo_status = 0;

		size = skb_frag_size(frag);
		data_len -= size;

		dma = skb_frag_dma_map(tx_ring->dev, frag, 0,
				       size, DMA_TO_DEVICE);

		tx_buffer = &tx_ring->tx_buffer_info[i];
	}
#endif

	/* write last descriptor with RS and EOP bits */
	cmd_type |= size | IGB_TXD_DCMD;
	tx_desc->read.cmd_type_len = cpu_to_le32(cmd_type);

	netdev_tx_sent_queue(txring_txq(tx_ring), first->bytecount);
	/* set the timestamp */
	first->time_stamp = jiffies;

	/*
	 * Force memory writes to complete before letting h/w know there
	 * are new descriptors to fetch.  (Only applicable for weak-ordered
	 * memory model archs, such as IA-64).
	 *
	 * We also need this memory barrier to make certain all of the
	 * status bits have been updated before next_to_watch is written.
	 */
	wmb();

	/* set next_to_watch value indicating a packet is present */
	first->next_to_watch = tx_desc;

	i++;
	if (i == tx_ring->count)
		i = 0;

	tx_ring->next_to_use = i;

	writel(i, tx_ring->tail);

	/* we need this if more than one processor can write to our tail
	 * at a time, it syncronizes IO on IA64/Altix systems */
	mmiowb();

	return;

dma_error:
	dev_err(tx_ring->dev, "TX DMA map failed\n");

	/* clear dma mappings for failed tx_buffer_info map */
	for (;;) {
		tx_buffer = &tx_ring->tx_buffer_info[i];
		igb_unmap_and_free_tx_resource(tx_ring, tx_buffer);
		if (tx_buffer == first)
			break;
		if (i == 0)
			i = tx_ring->count;
		i--;
	}

	tx_ring->next_to_use = i;
}



netdev_tx_t igb_xmit_frame_ring(struct sk_buff *skb,
				struct igb_ring *tx_ring)
{
	struct igb_tx_buffer *first;
	int tso;
	u32 tx_flags = 0;
#if PAGE_SIZE > IGB_MAX_DATA_PER_TXD
	unsigned short f;
#endif
	u16 count = TXD_USE_COUNT(skb_headlen(skb));
	__be16 protocol = vlan_get_protocol(skb);
	u8 hdr_len = 0;

	/*
	 * need: 1 descriptor per page * PAGE_SIZE/IGB_MAX_DATA_PER_TXD,
	 *       + 1 desc for skb_headlen/IGB_MAX_DATA_PER_TXD,
	 *       + 2 desc gap to keep tail from touching head,
	 *       + 1 desc for context descriptor,
	 * otherwise try next time
	 */
#ifndef PMON
#if PAGE_SIZE > IGB_MAX_DATA_PER_TXD
	for (f = 0; f < skb_shinfo(skb)->nr_frags; f++)
		count += TXD_USE_COUNT(skb_shinfo(skb)->frags[f].size);
#else
	count += skb_shinfo(skb)->nr_frags;
#endif
	if (igb_maybe_stop_tx(tx_ring, count + 3)) {
		/* this is a hard error */
		return NETDEV_TX_BUSY;
	}
#endif

	/* record the location of the first descriptor for this packet */
	first = &tx_ring->tx_buffer_info[tx_ring->next_to_use];
	first->skb = skb;
	first->bytecount = skb->len;
	first->gso_segs = 1;

#ifndef PMON
#ifdef HAVE_PTP_1588_CLOCK
	if (unlikely(skb_shinfo(skb)->tx_flags & SKBTX_HW_TSTAMP)) {
		struct igb_adapter *adapter = netdev_priv(tx_ring->netdev);

		if (!test_and_set_bit_lock(__IGB_PTP_TX_IN_PROGRESS,
					   &adapter->state)) {
			skb_shinfo(skb)->tx_flags |= SKBTX_IN_PROGRESS;
			tx_flags |= IGB_TX_FLAGS_TSTAMP;

			adapter->ptp_tx_skb = skb_get(skb);
			adapter->ptp_tx_start = jiffies;
			if (adapter->hw.mac.type == e1000_82576)
				schedule_work(&adapter->ptp_tx_work);
		}
	}
#endif /* HAVE_PTP_1588_CLOCK */
	skb_tx_timestamp(skb);
	if (vlan_tx_tag_present(skb)) {
		tx_flags |= IGB_TX_FLAGS_VLAN;
		tx_flags |= (vlan_tx_tag_get(skb) << IGB_TX_FLAGS_VLAN_SHIFT);
	}
#endif

	/* record initial flags and protocol */
	first->tx_flags = tx_flags;
	first->protocol = protocol;

#ifndef PMON
	tso = igb_tso(tx_ring, first, &hdr_len);
	if (tso < 0)
		goto out_drop;
	else if (!tso)
		igb_tx_csum(tx_ring, first);
#endif

	igb_tx_map(tx_ring, first, hdr_len);

#ifndef HAVE_TRANS_START_IN_QUEUE
	netdev_ring(tx_ring)->trans_start = jiffies;

#endif
	/* Make sure there is space in the ring for the next send. */
	igb_maybe_stop_tx(tx_ring, DESC_NEEDED);

	return NETDEV_TX_OK;

out_drop:
	igb_unmap_and_free_tx_resource(tx_ring, first);

	return NETDEV_TX_OK;
}



static netdev_tx_t igb_xmit_frame(struct sk_buff *skb,
				  struct net_device *netdev)
{
	struct igb_adapter *adapter = netdev_priv(netdev);

	if (test_bit(__IGB_DOWN, &adapter->state)) {
		dev_kfree_skb_any(skb);
		return NETDEV_TX_OK;
	}

	if (skb->len <= 0) {
		dev_kfree_skb_any(skb);
		return NETDEV_TX_OK;
	}

	/*
	 * The minimum packet size with TCTL.PSP set is 17 so pad the skb
	 * in order to meet this minimum size requirement.
	 */
	if (skb->len < 17) {
		if (skb_padto(skb, 17))
			return NETDEV_TX_OK;
		skb->len = 17;
	}

	return igb_xmit_frame_ring(skb, igb_tx_queue_mapping(adapter, skb));
}


/**
 * igb_process_skb_fields - Populate skb header fields from Rx descriptor
 * @rx_ring: rx descriptor ring packet is being transacted on
 * @rx_desc: pointer to the EOP Rx descriptor
 * @skb: pointer to current skb being populated
 *
 * This function checks the ring, descriptor, and packet information in
 * order to populate the hash, checksum, VLAN, timestamp, protocol, and
 * other fields within the skb.
 **/
static void igb_process_skb_fields(struct igb_ring *rx_ring,
				   union e1000_adv_rx_desc *rx_desc,
				   struct sk_buff *skb)
{
	struct net_device *dev = rx_ring->netdev;
	__le16 pkt_info = rx_desc->wb.lower.lo_dword.hs_rss.pkt_info;
	bool notype;

#ifdef NETIF_F_RXHASH
	igb_rx_hash(rx_ring, rx_desc, skb);

#endif
	igb_rx_checksum(rx_ring, rx_desc, skb);

	/* update packet type stats */
	switch (pkt_info & E1000_RXDADV_PKTTYPE_ILMASK) {
	case E1000_RXDADV_PKTTYPE_IPV4:
		rx_ring->pkt_stats.ipv4_packets++;
		break;
	case E1000_RXDADV_PKTTYPE_IPV4_EX:
		rx_ring->pkt_stats.ipv4e_packets++;
		break;
	case E1000_RXDADV_PKTTYPE_IPV6:
		rx_ring->pkt_stats.ipv6_packets++;
		break;
	case E1000_RXDADV_PKTTYPE_IPV6_EX:
		rx_ring->pkt_stats.ipv6e_packets++;
		break;
	default:
		notype = true;
		break;
	}

	switch (pkt_info & E1000_RXDADV_PKTTYPE_TLMASK) {
	case E1000_RXDADV_PKTTYPE_TCP:
		rx_ring->pkt_stats.tcp_packets++;
		break;
	case E1000_RXDADV_PKTTYPE_UDP:
		rx_ring->pkt_stats.udp_packets++;
		break;
	case E1000_RXDADV_PKTTYPE_SCTP:
		rx_ring->pkt_stats.sctp_packets++;
		break;
	case E1000_RXDADV_PKTTYPE_NFS:
		rx_ring->pkt_stats.nfs_packets++;
		break;
	case E1000_RXDADV_PKTTYPE_NONE:
		if (notype)
			rx_ring->pkt_stats.other_packets++;
		break;
	default:
		break;
	}
#ifndef PMON

#ifdef HAVE_PTP_1588_CLOCK
	if (igb_test_staterr(rx_desc, E1000_RXDADV_STAT_TS) &&
	    !igb_test_staterr(rx_desc, E1000_RXDADV_STAT_TSIP))
		igb_ptp_rx_rgtstamp(rx_ring->q_vector, skb);

#endif /* HAVE_PTP_1588_CLOCK */
#ifdef NETIF_F_HW_VLAN_CTAG_RX
	if ((dev->features & NETIF_F_HW_VLAN_CTAG_RX) &&
#else
	if ((dev->features & NETIF_F_HW_VLAN_RX) &&
#endif
	    igb_test_staterr(rx_desc, E1000_RXD_STAT_VP)) {
		u16 vid = 0;

		if (igb_test_staterr(rx_desc, E1000_RXDEXT_STATERR_LB) &&
		    test_bit(IGB_RING_FLAG_RX_LB_VLAN_BSWAP, &rx_ring->flags))
			vid = be16_to_cpu(rx_desc->wb.upper.vlan);
		else
			vid = le16_to_cpu(rx_desc->wb.upper.vlan);
#ifdef HAVE_VLAN_RX_REGISTER
		IGB_CB(skb)->vid = vid;
	} else {
		IGB_CB(skb)->vid = 0;
#else
		__vlan_hwaccel_put_tag(skb, htons(ETH_P_8021Q), vid);
#endif
	}

	skb_record_rx_queue(skb, rx_ring->queue_index);
#endif

	skb->protocol = eth_type_trans(skb, dev);
}



static bool igb_alloc_mapped_skb(struct igb_ring *rx_ring,
				 struct igb_rx_buffer *bi)
{
	struct sk_buff *skb = bi->skb;
	dma_addr_t dma = bi->dma;

	if (dma)
		return true;

	if (likely(!skb)) {
		skb = netdev_alloc_skb_ip_align(netdev_ring(rx_ring),
						rx_ring->rx_buffer_len);
		bi->skb = skb;
		if (!skb) {
			rx_ring->rx_stats.alloc_failed++;
			return false;
		}

		/* initialize skb for ring */
		skb_record_rx_queue(skb, ring_queue_index(rx_ring));
	}

	dma = dma_map_single(rx_ring->dev, skb->data,
			     rx_ring->rx_buffer_len, DMA_FROM_DEVICE);

	/* if mapping failed free memory back to system since
	 * there isn't much point in holding memory we can't use
	 */
	if (dma_mapping_error(rx_ring->dev, dma)) {
		dev_kfree_skb_any(skb);
		bi->skb = NULL;

		rx_ring->rx_stats.alloc_failed++;
		return false;
	}

	bi->dma = dma;
	return true;
}

/**
 * igb_alloc_rx_buffers - Replace used receive buffers; packet split
 * @adapter: address of board private structure
 **/
void igb_alloc_rx_buffers(struct igb_ring *rx_ring, u16 cleaned_count)
{
	union e1000_adv_rx_desc *rx_desc;
	struct igb_rx_buffer *bi;
	u16 i = rx_ring->next_to_use;

	/* nothing to do */
	if (!cleaned_count)
		return;

	rx_desc = IGB_RX_DESC(rx_ring, i);
	bi = &rx_ring->rx_buffer_info[i];
	i -= rx_ring->count;

	do {
#ifdef CONFIG_IGB_DISABLE_PACKET_SPLIT
		if (!igb_alloc_mapped_skb(rx_ring, bi))
#else
		if (!igb_alloc_mapped_page(rx_ring, bi))
#endif /* CONFIG_IGB_DISABLE_PACKET_SPLIT */
			break;

		/*
		 * Refresh the desc even if buffer_addrs didn't change
		 * because each write-back erases this info.
		 */
#ifdef CONFIG_IGB_DISABLE_PACKET_SPLIT
		rx_desc->read.pkt_addr = cpu_to_le64(bi->dma);
#else
		rx_desc->read.pkt_addr = cpu_to_le64(bi->dma + bi->page_offset);
#endif

		rx_desc++;
		bi++;
		i++;
		if (unlikely(!i)) {
			rx_desc = IGB_RX_DESC(rx_ring, 0);
			bi = rx_ring->rx_buffer_info;
			i -= rx_ring->count;
		}

		/* clear the hdr_addr for the next_to_use descriptor */
		rx_desc->read.hdr_addr = 0;

		cleaned_count--;
	} while (cleaned_count);

	i += rx_ring->count;

	if (rx_ring->next_to_use != i) {
		/* record the next descriptor to use */
		rx_ring->next_to_use = i;

#ifndef CONFIG_IGB_DISABLE_PACKET_SPLIT
		/* update next to alloc since we have filled the ring */
		rx_ring->next_to_alloc = i;

#endif
		/*
		 * Force memory writes to complete before letting h/w
		 * know there are new descriptors to fetch.  (Only
		 * applicable for weak-ordered memory model archs,
		 * such as IA-64).
		 */
		wmb();
		writel(i, rx_ring->tail);
	}
}

/**
 * igb_is_non_eop - process handling of non-EOP buffers
 * @rx_ring: Rx ring being processed
 * @rx_desc: Rx descriptor for current buffer
 *
 * This function updates next to clean.  If the buffer is an EOP buffer
 * this function exits returning false, otherwise it will place the
 * sk_buff in the next buffer to be chained and return true indicating
 * that this is in fact a non-EOP buffer.
 **/
static bool igb_is_non_eop(struct igb_ring *rx_ring,
			   union e1000_adv_rx_desc *rx_desc)
{
	u32 ntc = rx_ring->next_to_clean + 1;

	/* fetch, update, and store next to clean */
	ntc = (ntc < rx_ring->count) ? ntc : 0;
	rx_ring->next_to_clean = ntc;

	prefetch(IGB_RX_DESC(rx_ring, ntc));

	if (likely(igb_test_staterr(rx_desc, E1000_RXD_STAT_EOP)))
		return false;

	return true;
}

static bool igb_clean_rx_irq(struct igb_q_vector *q_vector, int budget)
{
	struct igb_ring *rx_ring = q_vector->rx.ring;
	unsigned int total_bytes = 0, total_packets = 0;
	u16 cleaned_count = igb_desc_unused(rx_ring);

	do {
		struct igb_rx_buffer *rx_buffer;
		union e1000_adv_rx_desc *rx_desc;
		struct sk_buff *skb;
		u16 ntc;

		/* return some buffers to hardware, one at a time is too slow */
		if (cleaned_count >= IGB_RX_BUFFER_WRITE) {
			igb_alloc_rx_buffers(rx_ring, cleaned_count);
			cleaned_count = 0;
		}

		ntc = rx_ring->next_to_clean;
		rx_desc = IGB_RX_DESC(rx_ring, ntc);
		rx_buffer = &rx_ring->rx_buffer_info[ntc];

		if (!igb_test_staterr(rx_desc, E1000_RXD_STAT_DD))
			break;

		/*
		 * This memory barrier is needed to keep us from reading
		 * any other fields out of the rx_desc until we know the
		 * RXD_STAT_DD bit is set
		 */
		rmb();

		skb = rx_buffer->skb;

		prefetch(skb->data);

		/* pull the header of the skb in */
		__skb_put(skb, le16_to_cpu(rx_desc->wb.upper.length));

		/* clear skb reference in buffer info structure */
		rx_buffer->skb = NULL;

		cleaned_count++;

		BUG_ON(igb_is_non_eop(rx_ring, rx_desc));

		dma_unmap_single(rx_ring->dev, rx_buffer->dma,
				 rx_ring->rx_buffer_len,
				 DMA_FROM_DEVICE);
		rx_buffer->dma = 0;

		if (igb_test_staterr(rx_desc,
				     E1000_RXDEXT_ERR_FRAME_ERR_MASK)) {
			dev_kfree_skb_any(skb);
			continue;
		}

		total_bytes += skb->len;

		/* populate checksum, timestamp, VLAN, and protocol */
		igb_process_skb_fields(rx_ring, rx_desc, skb);

#ifndef IGB_NO_LRO
		if (igb_can_lro(rx_ring, rx_desc, skb))
			igb_lro_receive(q_vector, skb);
		else
#endif
#ifdef HAVE_VLAN_RX_REGISTER
			igb_receive_skb(q_vector, skb);
#else
			napi_gro_receive(&q_vector->napi, skb);
#endif

#ifndef NETIF_F_GRO
		netdev_ring(rx_ring)->last_rx = jiffies;

#endif
		/* update budget accounting */
		total_packets++;
	} while (likely(total_packets < budget));

	rx_ring->rx_stats.packets += total_packets;
	rx_ring->rx_stats.bytes += total_bytes;
	q_vector->rx.total_packets += total_packets;
	q_vector->rx.total_bytes += total_bytes;

	if (cleaned_count)
		igb_alloc_rx_buffers(rx_ring, cleaned_count);

#ifndef IGB_NO_LRO
	igb_lro_flush_all(q_vector);

#endif /* IGB_NO_LRO */
	return (total_packets < budget);
}

/**
 * igb_clean_tx_irq - Reclaim resources after transmit completes
 * @q_vector: pointer to q_vector containing needed info
 * returns TRUE if ring is completely cleaned
 **/
static bool igb_clean_tx_irq(struct igb_q_vector *q_vector)
{
	struct igb_adapter *adapter = q_vector->adapter;
	struct igb_ring *tx_ring = q_vector->tx.ring;
	struct igb_tx_buffer *tx_buffer;
	union e1000_adv_tx_desc *tx_desc;
	unsigned int total_bytes = 0, total_packets = 0;
	unsigned int budget = q_vector->tx.work_limit;
	unsigned int i = tx_ring->next_to_clean;

	if (test_bit(__IGB_DOWN, &adapter->state))
		return true;

	tx_buffer = &tx_ring->tx_buffer_info[i];
	tx_desc = IGB_TX_DESC(tx_ring, i);
	i -= tx_ring->count;

	do {
		union e1000_adv_tx_desc *eop_desc = tx_buffer->next_to_watch;

		/* if next_to_watch is not set then there is no work pending */
		if (!eop_desc)
			break;

		/* prevent any other reads prior to eop_desc */
		read_barrier_depends();

		/* if DD is not set pending work has not been completed */
		if (!(eop_desc->wb.status & cpu_to_le32(E1000_TXD_STAT_DD)))
			break;

		/* clear next_to_watch to prevent false hangs */
		tx_buffer->next_to_watch = NULL;

		/* update the statistics for this packet */
		total_bytes += tx_buffer->bytecount;
		total_packets += tx_buffer->gso_segs;

		/* free the skb */
		dev_kfree_skb_any(tx_buffer->skb);

		/* unmap skb header data */
		dma_unmap_single(tx_ring->dev,
				 dma_unmap_addr(tx_buffer, dma),
				 dma_unmap_len(tx_buffer, len),
				 DMA_TO_DEVICE);

		/* clear tx_buffer data */
		tx_buffer->skb = NULL;
		dma_unmap_len_set(tx_buffer, len, 0);

		/* clear last DMA location and unmap remaining buffers */
		while (tx_desc != eop_desc) {
			tx_buffer++;
			tx_desc++;
			i++;
			if (unlikely(!i)) {
				i -= tx_ring->count;
				tx_buffer = tx_ring->tx_buffer_info;
				tx_desc = IGB_TX_DESC(tx_ring, 0);
			}

			/* unmap any remaining paged data */
			if (dma_unmap_len(tx_buffer, len)) {
				dma_unmap_page(tx_ring->dev,
					       dma_unmap_addr(tx_buffer, dma),
					       dma_unmap_len(tx_buffer, len),
					       DMA_TO_DEVICE);
				dma_unmap_len_set(tx_buffer, len, 0);
			}
		}

		/* move us one more past the eop_desc for start of next pkt */
		tx_buffer++;
		tx_desc++;
		i++;
		if (unlikely(!i)) {
			i -= tx_ring->count;
			tx_buffer = tx_ring->tx_buffer_info;
			tx_desc = IGB_TX_DESC(tx_ring, 0);
		}

		/* issue prefetch for next Tx descriptor */
		prefetch(tx_desc);

		/* update budget accounting */
		budget--;
	} while (likely(budget));

	netdev_tx_completed_queue(txring_txq(tx_ring),
				  total_packets, total_bytes);

	i += tx_ring->count;
	tx_ring->next_to_clean = i;
	tx_ring->tx_stats.bytes += total_bytes;
	tx_ring->tx_stats.packets += total_packets;
	q_vector->tx.total_bytes += total_bytes;
	q_vector->tx.total_packets += total_packets;

#ifdef DEBUG
	if (test_bit(IGB_RING_FLAG_TX_DETECT_HANG, &tx_ring->flags) &&
	    !(adapter->disable_hw_reset && adapter->tx_hang_detected)) {
#else
	if (test_bit(IGB_RING_FLAG_TX_DETECT_HANG, &tx_ring->flags)) {
#endif
		struct e1000_hw *hw = &adapter->hw;

		/* Detect a transmit hang in hardware, this serializes the
		 * check with the clearing of time_stamp and movement of i */
		clear_bit(IGB_RING_FLAG_TX_DETECT_HANG, &tx_ring->flags);
		if (tx_buffer->next_to_watch &&
		    time_after(jiffies, tx_buffer->time_stamp +
			       (adapter->tx_timeout_factor * HZ))
		    && !(E1000_READ_REG(hw, E1000_STATUS) &
			 E1000_STATUS_TXOFF)) {

			/* detected Tx unit hang */
#ifdef DEBUG
			adapter->tx_hang_detected = TRUE;
			if (adapter->disable_hw_reset) {
				DPRINTK(DRV, WARNING,
					"Deactivating netdev watchdog timer\n");
				if (del_timer(&netdev_ring(tx_ring)->watchdog_timer))
					dev_put(netdev_ring(tx_ring));
#ifndef HAVE_NET_DEVICE_OPS
				netdev_ring(tx_ring)->tx_timeout = NULL;
#endif
			}
#endif /* DEBUG */
			dev_err(tx_ring->dev,
				"Detected Tx Unit Hang\n"
				"  Tx Queue             <%d>\n"
				"  TDH                  <%x>\n"
				"  TDT                  <%x>\n"
				"  next_to_use          <%x>\n"
				"  next_to_clean        <%x>\n"
				"buffer_info[next_to_clean]\n"
				"  time_stamp           <%lx>\n"
				"  next_to_watch        <%p>\n"
				"  jiffies              <%lx>\n"
				"  desc.status          <%x>\n",
				tx_ring->queue_index,
				E1000_READ_REG(hw, E1000_TDH(tx_ring->reg_idx)),
				readl(tx_ring->tail),
				tx_ring->next_to_use,
				tx_ring->next_to_clean,
				tx_buffer->time_stamp,
				tx_buffer->next_to_watch,
				jiffies,
				tx_buffer->next_to_watch->wb.status);
#ifndef PMON
			if (netif_is_multiqueue(netdev_ring(tx_ring)))
				netif_stop_subqueue(netdev_ring(tx_ring),
						    ring_queue_index(tx_ring));
			else
				netif_stop_queue(netdev_ring(tx_ring));
#endif

			/* we are about to reset, no point in enabling stuff */
			return true;
		}
	}

#ifndef PMON
#define TX_WAKE_THRESHOLD (DESC_NEEDED * 2)
	if (unlikely(total_packets &&
		     netif_carrier_ok(netdev_ring(tx_ring)) &&
		     igb_desc_unused(tx_ring) >= TX_WAKE_THRESHOLD)) {
		/* Make sure that anybody stopping the queue after this
		 * sees the new next_to_clean.
		 */
		smp_mb();
		if (netif_is_multiqueue(netdev_ring(tx_ring))) {
			if (__netif_subqueue_stopped(netdev_ring(tx_ring),
						ring_queue_index(tx_ring)) &&
			    !(test_bit(__IGB_DOWN, &adapter->state))) {
				netif_wake_subqueue(netdev_ring(tx_ring),
						    ring_queue_index(tx_ring));
				tx_ring->tx_stats.restart_queue++;
			}
		} else {
			if (netif_queue_stopped(netdev_ring(tx_ring)) &&
			    !(test_bit(__IGB_DOWN, &adapter->state))) {
				netif_wake_queue(netdev_ring(tx_ring));
				tx_ring->tx_stats.restart_queue++;
			}
		}
	}
#endif

	return !!budget;
}

/**
 * igb_update_itr - update the dynamic ITR value based on statistics
 *      Stores a new ITR value based on packets and byte
 *      counts during the last interrupt.  The advantage of per interrupt
 *      computation is faster updates and more accurate ITR for the current
 *      traffic pattern.  Constants in this function were computed
 *      based on theoretical maximum wire speed and thresholds were set based
 *      on testing data as well as attempting to minimize response time
 *      while increasing bulk throughput.
 *      this functionality is controlled by the InterruptThrottleRate module
 *      parameter (see igb_param.c)
 *      NOTE:  These calculations are only valid when operating in a single-
 *             queue environment.
 * @q_vector: pointer to q_vector
 * @ring_container: ring info to update the itr for
 **/
static void igb_update_itr(struct igb_q_vector *q_vector,
			   struct igb_ring_container *ring_container)
{
	unsigned int packets = ring_container->total_packets;
	unsigned int bytes = ring_container->total_bytes;
	u8 itrval = ring_container->itr;

	/* no packets, exit with status unchanged */
	if (packets == 0)
		return;

	switch (itrval) {
	case lowest_latency:
		/* handle TSO and jumbo frames */
		if (bytes/packets > 8000)
			itrval = bulk_latency;
		else if ((packets < 5) && (bytes > 512))
			itrval = low_latency;
		break;
	case low_latency:  /* 50 usec aka 20000 ints/s */
		if (bytes > 10000) {
			/* this if handles the TSO accounting */
			if (bytes/packets > 8000) {
				itrval = bulk_latency;
			} else if ((packets < 10) || ((bytes/packets) > 1200)) {
				itrval = bulk_latency;
			} else if ((packets > 35)) {
				itrval = lowest_latency;
			}
		} else if (bytes/packets > 2000) {
			itrval = bulk_latency;
		} else if (packets <= 2 && bytes < 512) {
			itrval = lowest_latency;
		}
		break;
	case bulk_latency: /* 250 usec aka 4000 ints/s */
		if (bytes > 25000) {
			if (packets > 35)
				itrval = low_latency;
		} else if (bytes < 1500) {
			itrval = low_latency;
		}
		break;
	}

	/* clear work counters since we have the values we need */
	ring_container->total_bytes = 0;
	ring_container->total_packets = 0;

	/* write updated itr to ring container */
	ring_container->itr = itrval;
}

static void igb_set_itr(struct igb_q_vector *q_vector)
{
	struct igb_adapter *adapter = q_vector->adapter;
	u32 new_itr = q_vector->itr_val;
	u8 current_itr = 0;

	/* for non-gigabit speeds, just fix the interrupt rate at 4000 */
	switch (adapter->link_speed) {
	case SPEED_10:
	case SPEED_100:
		current_itr = 0;
		new_itr = IGB_4K_ITR;
		goto set_itr_now;
	default:
		break;
	}

	igb_update_itr(q_vector, &q_vector->tx);
	igb_update_itr(q_vector, &q_vector->rx);

	current_itr = max(q_vector->rx.itr, q_vector->tx.itr);

	/* conservative mode (itr 3) eliminates the lowest_latency setting */
	if (current_itr == lowest_latency &&
	    ((q_vector->rx.ring && adapter->rx_itr_setting == 3) ||
	     (!q_vector->rx.ring && adapter->tx_itr_setting == 3)))
		current_itr = low_latency;

	switch (current_itr) {
	/* counts and packets in update_itr are dependent on these numbers */
	case lowest_latency:
		new_itr = IGB_70K_ITR; /* 70,000 ints/sec */
		break;
	case low_latency:
		new_itr = IGB_20K_ITR; /* 20,000 ints/sec */
		break;
	case bulk_latency:
		new_itr = IGB_4K_ITR;  /* 4,000 ints/sec */
		break;
	default:
		break;
	}

set_itr_now:
	if (new_itr != q_vector->itr_val) {
		/* this attempts to bias the interrupt rate towards Bulk
		 * by adding intermediate steps when interrupt rate is
		 * increasing */
		new_itr = new_itr > q_vector->itr_val ?
				max((new_itr * q_vector->itr_val) /
				    (new_itr + (q_vector->itr_val >> 2)),
				 new_itr) : new_itr;
		/* Don't write the value here; it resets the adapter's
		 * internal timer, and causes us to delay far longer than
		 * we should between interrupts.  Instead, we write the ITR
		 * value at the beginning of the next interrupt so the timing
		 * ends up being correct.
		 */
		q_vector->itr_val = new_itr;
		q_vector->set_itr = 1;
	}
}

/**
 * igb_update_ring_itr - update the dynamic ITR value based on packet size
 *
 *      Stores a new ITR value based on strictly on packet size.  This
 *      algorithm is less sophisticated than that used in igb_update_itr,
 *      due to the difficulty of synchronizing statistics across multiple
 *      receive rings.  The divisors and thresholds used by this function
 *      were determined based on theoretical maximum wire speed and testing
 *      data, in order to minimize response time while increasing bulk
 *      throughput.
 *      This functionality is controlled by the InterruptThrottleRate module
 *      parameter (see igb_param.c)
 *      NOTE:  This function is called only when operating in a multiqueue
 *             receive environment.
 * @q_vector: pointer to q_vector
 **/
static void igb_update_ring_itr(struct igb_q_vector *q_vector)
{
	int new_val = q_vector->itr_val;
	int avg_wire_size = 0;
	struct igb_adapter *adapter = q_vector->adapter;
	unsigned int packets;

	/* For non-gigabit speeds, just fix the interrupt rate at 4000
	 * ints/sec - ITR timer value of 120 ticks.
	 */
	switch (adapter->link_speed) {
	case SPEED_10:
	case SPEED_100:
		new_val = IGB_4K_ITR;
		goto set_itr_val;
	default:
		break;
	}

	packets = q_vector->rx.total_packets;
	if (packets)
		avg_wire_size = q_vector->rx.total_bytes / packets;

	packets = q_vector->tx.total_packets;
	if (packets)
		avg_wire_size = max_t(u32, avg_wire_size,
				      q_vector->tx.total_bytes / packets);

	/* if avg_wire_size isn't set no work was done */
	if (!avg_wire_size)
		goto clear_counts;

	/* Add 24 bytes to size to account for CRC, preamble, and gap */
	avg_wire_size += 24;

	/* Don't starve jumbo frames */
	avg_wire_size = min(avg_wire_size, 3000);

	/* Give a little boost to mid-size frames */
	if ((avg_wire_size > 300) && (avg_wire_size < 1200))
		new_val = avg_wire_size / 3;
	else
		new_val = avg_wire_size / 2;

	/* conservative mode (itr 3) eliminates the lowest_latency setting */
	if (new_val < IGB_20K_ITR &&
	    ((q_vector->rx.ring && adapter->rx_itr_setting == 3) ||
	     (!q_vector->rx.ring && adapter->tx_itr_setting == 3)))
		new_val = IGB_20K_ITR;

set_itr_val:
	if (new_val != q_vector->itr_val) {
		q_vector->itr_val = new_val;
		q_vector->set_itr = 1;
	}
clear_counts:
	q_vector->rx.total_bytes = 0;
	q_vector->rx.total_packets = 0;
	q_vector->tx.total_bytes = 0;
	q_vector->tx.total_packets = 0;
}

void igb_ring_irq_enable(struct igb_q_vector *q_vector)
{
	struct igb_adapter *adapter = q_vector->adapter;
	struct e1000_hw *hw = &adapter->hw;

	if ((q_vector->rx.ring && (adapter->rx_itr_setting & 3)) ||
	    (!q_vector->rx.ring && (adapter->tx_itr_setting & 3))) {
		if ((adapter->num_q_vectors == 1) && !adapter->vf_data)
			igb_set_itr(q_vector);
		else
			igb_update_ring_itr(q_vector);
	}

	if (!test_bit(__IGB_DOWN, &adapter->state)) {
		if (adapter->msix_entries)
			E1000_WRITE_REG(hw, E1000_EIMS, q_vector->eims_value);
		else
			igb_irq_enable(adapter);
	}
}

/**
 * igb_poll - NAPI Rx polling callback
 * @napi: napi polling structure
 * @budget: count of how many packets we should handle
 **/
static int igb_poll(struct napi_struct *napi, int budget)
{
	struct igb_q_vector *q_vector = container_of(napi,
						     struct igb_q_vector, napi);
	bool clean_complete = true;

#ifdef IGB_DCA
	if (q_vector->adapter->flags & IGB_FLAG_DCA_ENABLED)
		igb_update_dca(q_vector);
#endif
	if (q_vector->tx.ring)
		clean_complete = igb_clean_tx_irq(q_vector);

	if (q_vector->rx.ring)
		clean_complete &= igb_clean_rx_irq(q_vector, budget);

#ifndef PMON
#ifndef HAVE_NETDEV_NAPI_LIST
	/* if netdev is disabled we need to stop polling */
	if (!netif_running(q_vector->adapter->netdev))
		clean_complete = true;

#endif
	/* If all work not completed, return budget and keep polling */
	if (!clean_complete)
		return budget;

	/* If not enough Rx work done, exit the polling mode */
	napi_complete(napi);
#endif
	igb_ring_irq_enable(q_vector);

	return 0;
}


static void igb_write_itr(struct igb_q_vector *q_vector)
{
	struct igb_adapter *adapter = q_vector->adapter;
	u32 itr_val = q_vector->itr_val & 0x7FFC;

	if (!q_vector->set_itr)
		return;

	if (!itr_val)
		itr_val = 0x4;

	if (adapter->hw.mac.type == e1000_82575)
		itr_val |= itr_val << 16;
	else
		itr_val |= E1000_EITR_CNT_IGNR;

	writel(itr_val, q_vector->itr_register);
	q_vector->set_itr = 0;
}

/**
 * igb_intr - Legacy Interrupt Handler
 * @irq: interrupt number
 * @data: pointer to a network interface device structure
 **/
static irqreturn_t igb_intr(int irq, void *data)
{
	struct igb_adapter *adapter = data;
	struct igb_q_vector *q_vector = adapter->q_vector[0];
	struct e1000_hw *hw = &adapter->hw;
	/* Interrupt Auto-Mask...upon reading ICR, interrupts are masked.  No
	 * need for the IMC write */
	u32 icr = E1000_READ_REG(hw, E1000_ICR);

	/* IMS will not auto-mask if INT_ASSERTED is not set, and if it is
	 * not set, then the adapter didn't send an interrupt */
	if (!(icr & E1000_ICR_INT_ASSERTED))
		return IRQ_NONE;

	igb_write_itr(q_vector);

#ifndef PMON
	if (icr & E1000_ICR_DRSTA)
		schedule_work(&adapter->reset_task);
#endif

	if (icr & E1000_ICR_DOUTSYNC) {
		/* HW is reporting DMA is out of sync */
		adapter->stats.doosync++;
	}

	if (icr & (E1000_ICR_RXSEQ | E1000_ICR_LSC)) {
		hw->mac.get_link_status = 1;
		/* guard against interrupt when we're going down */
		if (!test_bit(__IGB_DOWN, &adapter->state))
			mod_timer(&adapter->watchdog_timer, jiffies + 1);
	}

#ifndef PMON
#ifdef HAVE_PTP_1588_CLOCK
	if (icr & E1000_ICR_TS) {
		u32 tsicr = E1000_READ_REG(hw, E1000_TSICR);

		if (tsicr & E1000_TSICR_TXTS) {
			/* acknowledge the interrupt */
			E1000_WRITE_REG(hw, E1000_TSICR, E1000_TSICR_TXTS);
			/* retrieve hardware timestamp */
			schedule_work(&adapter->ptp_tx_work);
		}
	}
#endif /* HAVE_PTP_1588_CLOCK */

	napi_schedule(&q_vector->napi);
#else
	igb_poll(&q_vector->napi,30);
#endif

	return IRQ_HANDLED;
}


/**
 * igb_irq_disable - Mask off interrupt generation on the NIC
 * @adapter: board private structure
 **/
static void igb_irq_disable(struct igb_adapter *adapter)
{
	struct e1000_hw *hw = &adapter->hw;

#ifndef PMON
	/*
	 * we need to be careful when disabling interrupts.  The VFs are also
	 * mapped into these registers and so clearing the bits can cause
	 * issues on the VF drivers so we only need to clear what we set
	 */
	if (adapter->msix_entries) {
		u32 regval = E1000_READ_REG(hw, E1000_EIAM);

		E1000_WRITE_REG(hw, E1000_EIAM, regval
				& ~adapter->eims_enable_mask);
		E1000_WRITE_REG(hw, E1000_EIMC, adapter->eims_enable_mask);
		regval = E1000_READ_REG(hw, E1000_EIAC);
		E1000_WRITE_REG(hw, E1000_EIAC, regval
				& ~adapter->eims_enable_mask);
	}
#endif

	E1000_WRITE_REG(hw, E1000_IAM, 0);
	E1000_WRITE_REG(hw, E1000_IMC, ~0);
	E1000_WRITE_FLUSH(hw);

#ifndef PMON
	if (adapter->msix_entries) {
		int vector = 0, i;

		synchronize_irq(adapter->msix_entries[vector++].vector);

		for (i = 0; i < adapter->num_q_vectors; i++)
			synchronize_irq(adapter->msix_entries[vector++].vector);
	} else {
		synchronize_irq(adapter->pdev->irq);
	}
#endif
}

/**
 * igb_free_tx_resources - Free Tx Resources per Queue
 * @tx_ring: Tx descriptor ring for a specific queue
 *
 * Free all transmit software resources
 **/
void igb_free_tx_resources(struct igb_ring *tx_ring)
{
	igb_clean_tx_ring(tx_ring);

	vfree(tx_ring->tx_buffer_info);
	tx_ring->tx_buffer_info = NULL;

	/* if not set, then don't free */
	if (!tx_ring->desc)
		return;

	dma_free_coherent(tx_ring->dev, tx_ring->size,
			  tx_ring->desc, tx_ring->dma);

	tx_ring->desc = NULL;
}

/**
 * igb_free_all_tx_resources - Free Tx Resources for All Queues
 * @adapter: board private structure
 *
 * Free all transmit software resources
 **/
static void igb_free_all_tx_resources(struct igb_adapter *adapter)
{
	int i;

	for (i = 0; i < adapter->num_tx_queues; i++)
		igb_free_tx_resources(adapter->tx_ring[i]);
}

void igb_unmap_and_free_tx_resource(struct igb_ring *ring,
				    struct igb_tx_buffer *tx_buffer)
{
	if (tx_buffer->skb) {
		dev_kfree_skb_any(tx_buffer->skb);
		if (dma_unmap_len(tx_buffer, len))
			dma_unmap_single(ring->dev,
					 dma_unmap_addr(tx_buffer, dma),
					 dma_unmap_len(tx_buffer, len),
					 DMA_TO_DEVICE);
	} else if (dma_unmap_len(tx_buffer, len)) {
		dma_unmap_page(ring->dev,
				dma_unmap_addr(tx_buffer, dma),
				dma_unmap_len(tx_buffer, len),
				DMA_TO_DEVICE);
	}
	tx_buffer->next_to_watch = NULL;
	tx_buffer->skb = NULL;
	dma_unmap_len_set(tx_buffer, len, 0);
	/* buffer_info must be completely set up in the transmit path */
}

/**
 * igb_clean_tx_ring - Free Tx Buffers
 * @tx_ring: ring to be cleaned
 **/
static void igb_clean_tx_ring(struct igb_ring *tx_ring)
{
	struct igb_tx_buffer *buffer_info;
	unsigned long size;
	u16 i;

	if (!tx_ring->tx_buffer_info)
		return;
	/* Free all the Tx ring sk_buffs */

	for (i = 0; i < tx_ring->count; i++) {
		buffer_info = &tx_ring->tx_buffer_info[i];
		igb_unmap_and_free_tx_resource(tx_ring, buffer_info);
	}

	netdev_tx_reset_queue(txring_txq(tx_ring));

	size = sizeof(struct igb_tx_buffer) * tx_ring->count;
	memset(tx_ring->tx_buffer_info, 0, size);

	/* Zero out the descriptor ring */
	memset(tx_ring->desc, 0, tx_ring->size);

	tx_ring->next_to_use = 0;
	tx_ring->next_to_clean = 0;
}

/**
 * igb_clean_all_tx_rings - Free Tx Buffers for all queues
 * @adapter: board private structure
 **/
static void igb_clean_all_tx_rings(struct igb_adapter *adapter)
{
	int i;

	for (i = 0; i < adapter->num_tx_queues; i++)
		igb_clean_tx_ring(adapter->tx_ring[i]);
}

/**
 * igb_free_rx_resources - Free Rx Resources
 * @rx_ring: ring to clean the resources from
 *
 * Free all receive software resources
 **/
void igb_free_rx_resources(struct igb_ring *rx_ring)
{
	igb_clean_rx_ring(rx_ring);

	vfree(rx_ring->rx_buffer_info);
	rx_ring->rx_buffer_info = NULL;

	/* if not set, then don't free */
	if (!rx_ring->desc)
		return;

	dma_free_coherent(rx_ring->dev, rx_ring->size,
			  rx_ring->desc, rx_ring->dma);

	rx_ring->desc = NULL;
}

/**
 * igb_free_all_rx_resources - Free Rx Resources for All Queues
 * @adapter: board private structure
 *
 * Free all receive software resources
 **/
static void igb_free_all_rx_resources(struct igb_adapter *adapter)
{
	int i;

	for (i = 0; i < adapter->num_rx_queues; i++)
		igb_free_rx_resources(adapter->rx_ring[i]);
}

/**
 * igb_clean_rx_ring - Free Rx Buffers per Queue
 * @rx_ring: ring to free buffers from
 **/
void igb_clean_rx_ring(struct igb_ring *rx_ring)
{
	unsigned long size;
	u16 i;

	if (!rx_ring->rx_buffer_info)
		return;

#ifndef CONFIG_IGB_DISABLE_PACKET_SPLIT
	if (rx_ring->skb)
		dev_kfree_skb(rx_ring->skb);
	rx_ring->skb = NULL;

#endif
	/* Free all the Rx ring sk_buffs */
	for (i = 0; i < rx_ring->count; i++) {
		struct igb_rx_buffer *buffer_info = &rx_ring->rx_buffer_info[i];
#ifdef CONFIG_IGB_DISABLE_PACKET_SPLIT
		if (buffer_info->dma) {
			dma_unmap_single(rx_ring->dev,
					 buffer_info->dma,
					 rx_ring->rx_buffer_len,
					 DMA_FROM_DEVICE);
			buffer_info->dma = 0;
		}

		if (buffer_info->skb) {
			dev_kfree_skb(buffer_info->skb);
			buffer_info->skb = NULL;
		}
#else
		if (!buffer_info->page)
			continue;

		dma_unmap_page(rx_ring->dev,
			       buffer_info->dma,
			       PAGE_SIZE,
			       DMA_FROM_DEVICE);
		__free_page(buffer_info->page);

		buffer_info->page = NULL;
#endif
	}

	size = sizeof(struct igb_rx_buffer) * rx_ring->count;
	memset(rx_ring->rx_buffer_info, 0, size);

	/* Zero out the descriptor ring */
	memset(rx_ring->desc, 0, rx_ring->size);

	rx_ring->next_to_alloc = 0;
	rx_ring->next_to_clean = 0;
	rx_ring->next_to_use = 0;
}

/**
 * igb_clean_all_rx_rings - Free Rx Buffers for all queues
 * @adapter: board private structure
 **/
static void igb_clean_all_rx_rings(struct igb_adapter *adapter)
{
	int i;

	for (i = 0; i < adapter->num_rx_queues; i++)
		igb_clean_rx_ring(adapter->rx_ring[i]);
}

/**
 * igb_update_stats - Update the board statistics counters
 * @adapter: board private structure
 **/

void igb_update_stats(struct igb_adapter *adapter)
{
#ifdef HAVE_NETDEV_STATS_IN_NETDEV
	struct net_device_stats *net_stats = &adapter->netdev->stats;
#else
	struct net_device_stats *net_stats = &adapter->net_stats;
#endif /* HAVE_NETDEV_STATS_IN_NETDEV */
	struct e1000_hw *hw = &adapter->hw;
#ifdef HAVE_PCI_ERS
	struct pci_dev *pdev = adapter->pdev;
#endif
	u32 reg, mpc;
	u16 phy_tmp;
	int i;
	u64 bytes, packets;
#ifndef IGB_NO_LRO
	u32 flushed = 0, coal = 0;
	struct igb_q_vector *q_vector;
#endif

#define PHY_IDLE_ERROR_COUNT_MASK 0x00FF

	/*
	 * Prevent stats update while adapter is being reset, or if the pci
	 * connection is down.
	 */
	if (adapter->link_speed == 0)
		return;
#ifdef HAVE_PCI_ERS
	if (pci_channel_offline(pdev))
		return;

#endif
#ifndef IGB_NO_LRO
	for (i = 0; i < adapter->num_q_vectors; i++) {
		q_vector = adapter->q_vector[i];
		if (!q_vector)
			continue;
		flushed += q_vector->lrolist.stats.flushed;
		coal += q_vector->lrolist.stats.coal;
	}
	adapter->lro_stats.flushed = flushed;
	adapter->lro_stats.coal = coal;

#endif
	bytes = 0;
	packets = 0;
	for (i = 0; i < adapter->num_rx_queues; i++) {
		struct igb_ring *ring = adapter->rx_ring[i];
		u32 rqdpc_tmp = E1000_READ_REG(hw, E1000_RQDPC(i)) & 0x0FFF;

		if (hw->mac.type >= e1000_i210)
			E1000_WRITE_REG(hw, E1000_RQDPC(i), 0);
		ring->rx_stats.drops += rqdpc_tmp;
		net_stats->rx_fifo_errors += rqdpc_tmp;
#ifdef CONFIG_IGB_VMDQ_NETDEV
		if (!ring->vmdq_netdev) {
			bytes += ring->rx_stats.bytes;
			packets += ring->rx_stats.packets;
		}
#else
		bytes += ring->rx_stats.bytes;
		packets += ring->rx_stats.packets;
#endif
	}

	net_stats->rx_bytes = bytes;
	net_stats->rx_packets = packets;

	bytes = 0;
	packets = 0;
	for (i = 0; i < adapter->num_tx_queues; i++) {
		struct igb_ring *ring = adapter->tx_ring[i];
#ifdef CONFIG_IGB_VMDQ_NETDEV
		if (!ring->vmdq_netdev) {
			bytes += ring->tx_stats.bytes;
			packets += ring->tx_stats.packets;
		}
#else
		bytes += ring->tx_stats.bytes;
		packets += ring->tx_stats.packets;
#endif
	}
	net_stats->tx_bytes = bytes;
	net_stats->tx_packets = packets;

	/* read stats registers */
	adapter->stats.crcerrs += E1000_READ_REG(hw, E1000_CRCERRS);
	adapter->stats.gprc += E1000_READ_REG(hw, E1000_GPRC);
	adapter->stats.gorc += E1000_READ_REG(hw, E1000_GORCL);
	E1000_READ_REG(hw, E1000_GORCH); /* clear GORCL */
	adapter->stats.bprc += E1000_READ_REG(hw, E1000_BPRC);
	adapter->stats.mprc += E1000_READ_REG(hw, E1000_MPRC);
	adapter->stats.roc += E1000_READ_REG(hw, E1000_ROC);

	adapter->stats.prc64 += E1000_READ_REG(hw, E1000_PRC64);
	adapter->stats.prc127 += E1000_READ_REG(hw, E1000_PRC127);
	adapter->stats.prc255 += E1000_READ_REG(hw, E1000_PRC255);
	adapter->stats.prc511 += E1000_READ_REG(hw, E1000_PRC511);
	adapter->stats.prc1023 += E1000_READ_REG(hw, E1000_PRC1023);
	adapter->stats.prc1522 += E1000_READ_REG(hw, E1000_PRC1522);
	adapter->stats.symerrs += E1000_READ_REG(hw, E1000_SYMERRS);
	adapter->stats.sec += E1000_READ_REG(hw, E1000_SEC);

	mpc = E1000_READ_REG(hw, E1000_MPC);
	adapter->stats.mpc += mpc;
	net_stats->rx_fifo_errors += mpc;
	adapter->stats.scc += E1000_READ_REG(hw, E1000_SCC);
	adapter->stats.ecol += E1000_READ_REG(hw, E1000_ECOL);
	adapter->stats.mcc += E1000_READ_REG(hw, E1000_MCC);
	adapter->stats.latecol += E1000_READ_REG(hw, E1000_LATECOL);
	adapter->stats.dc += E1000_READ_REG(hw, E1000_DC);
	adapter->stats.rlec += E1000_READ_REG(hw, E1000_RLEC);
	adapter->stats.xonrxc += E1000_READ_REG(hw, E1000_XONRXC);
	adapter->stats.xontxc += E1000_READ_REG(hw, E1000_XONTXC);
	adapter->stats.xoffrxc += E1000_READ_REG(hw, E1000_XOFFRXC);
	adapter->stats.xofftxc += E1000_READ_REG(hw, E1000_XOFFTXC);
	adapter->stats.fcruc += E1000_READ_REG(hw, E1000_FCRUC);
	adapter->stats.gptc += E1000_READ_REG(hw, E1000_GPTC);
	adapter->stats.gotc += E1000_READ_REG(hw, E1000_GOTCL);
	E1000_READ_REG(hw, E1000_GOTCH); /* clear GOTCL */
	adapter->stats.rnbc += E1000_READ_REG(hw, E1000_RNBC);
	adapter->stats.ruc += E1000_READ_REG(hw, E1000_RUC);
	adapter->stats.rfc += E1000_READ_REG(hw, E1000_RFC);
	adapter->stats.rjc += E1000_READ_REG(hw, E1000_RJC);
	adapter->stats.tor += E1000_READ_REG(hw, E1000_TORH);
	adapter->stats.tot += E1000_READ_REG(hw, E1000_TOTH);
	adapter->stats.tpr += E1000_READ_REG(hw, E1000_TPR);

	adapter->stats.ptc64 += E1000_READ_REG(hw, E1000_PTC64);
	adapter->stats.ptc127 += E1000_READ_REG(hw, E1000_PTC127);
	adapter->stats.ptc255 += E1000_READ_REG(hw, E1000_PTC255);
	adapter->stats.ptc511 += E1000_READ_REG(hw, E1000_PTC511);
	adapter->stats.ptc1023 += E1000_READ_REG(hw, E1000_PTC1023);
	adapter->stats.ptc1522 += E1000_READ_REG(hw, E1000_PTC1522);

	adapter->stats.mptc += E1000_READ_REG(hw, E1000_MPTC);
	adapter->stats.bptc += E1000_READ_REG(hw, E1000_BPTC);

	adapter->stats.tpt += E1000_READ_REG(hw, E1000_TPT);
	adapter->stats.colc += E1000_READ_REG(hw, E1000_COLC);

	adapter->stats.algnerrc += E1000_READ_REG(hw, E1000_ALGNERRC);
	/* read internal phy sepecific stats */
	reg = E1000_READ_REG(hw, E1000_CTRL_EXT);
	if (!(reg & E1000_CTRL_EXT_LINK_MODE_MASK)) {
		adapter->stats.rxerrc += E1000_READ_REG(hw, E1000_RXERRC);

		/* this stat has invalid values on i210/i211 */
		if ((hw->mac.type != e1000_i210) &&
		    (hw->mac.type != e1000_i211))
			adapter->stats.tncrs += E1000_READ_REG(hw, E1000_TNCRS);
	}
	adapter->stats.tsctc += E1000_READ_REG(hw, E1000_TSCTC);
	adapter->stats.tsctfc += E1000_READ_REG(hw, E1000_TSCTFC);

	adapter->stats.iac += E1000_READ_REG(hw, E1000_IAC);
	adapter->stats.icrxoc += E1000_READ_REG(hw, E1000_ICRXOC);
	adapter->stats.icrxptc += E1000_READ_REG(hw, E1000_ICRXPTC);
	adapter->stats.icrxatc += E1000_READ_REG(hw, E1000_ICRXATC);
	adapter->stats.ictxptc += E1000_READ_REG(hw, E1000_ICTXPTC);
	adapter->stats.ictxatc += E1000_READ_REG(hw, E1000_ICTXATC);
	adapter->stats.ictxqec += E1000_READ_REG(hw, E1000_ICTXQEC);
	adapter->stats.ictxqmtc += E1000_READ_REG(hw, E1000_ICTXQMTC);
	adapter->stats.icrxdmtc += E1000_READ_REG(hw, E1000_ICRXDMTC);

	/* Fill out the OS statistics structure */
	net_stats->multicast = adapter->stats.mprc;
	net_stats->collisions = adapter->stats.colc;

	/* Rx Errors */

	/* RLEC on some newer hardware can be incorrect so build
	 * our own version based on RUC and ROC */
	net_stats->rx_errors = adapter->stats.rxerrc +
		adapter->stats.crcerrs + adapter->stats.algnerrc +
		adapter->stats.ruc + adapter->stats.roc +
		adapter->stats.cexterr;
	net_stats->rx_length_errors = adapter->stats.ruc +
				      adapter->stats.roc;
	net_stats->rx_crc_errors = adapter->stats.crcerrs;
	net_stats->rx_frame_errors = adapter->stats.algnerrc;
	net_stats->rx_missed_errors = adapter->stats.mpc;

	/* Tx Errors */
	net_stats->tx_errors = adapter->stats.ecol +
			       adapter->stats.latecol;
	net_stats->tx_aborted_errors = adapter->stats.ecol;
	net_stats->tx_window_errors = adapter->stats.latecol;
	net_stats->tx_carrier_errors = adapter->stats.tncrs;

	/* Tx Dropped needs to be maintained elsewhere */

	/* Phy Stats */
	if (hw->phy.media_type == e1000_media_type_copper) {
		if ((adapter->link_speed == SPEED_1000) &&
		   (!igb_e1000_read_phy_reg(hw, PHY_1000T_STATUS, &phy_tmp))) {
			phy_tmp &= PHY_IDLE_ERROR_COUNT_MASK;
			adapter->phy_stats.idle_errors += phy_tmp;
		}
	}

	/* Management Stats */
	adapter->stats.mgptc += E1000_READ_REG(hw, E1000_MGTPTC);
	adapter->stats.mgprc += E1000_READ_REG(hw, E1000_MGTPRC);
	if (hw->mac.type > e1000_82580) {
		adapter->stats.o2bgptc += E1000_READ_REG(hw, E1000_O2BGPTC);
		adapter->stats.o2bspc += E1000_READ_REG(hw, E1000_O2BSPC);
		adapter->stats.b2ospc += E1000_READ_REG(hw, E1000_B2OSPC);
		adapter->stats.b2ogprc += E1000_READ_REG(hw, E1000_B2OGPRC);
	}
}

static void igb_ping_all_vfs(struct igb_adapter *adapter)
{
	struct e1000_hw *hw = &adapter->hw;
	u32 ping;
	int i;

	for (i = 0 ; i < adapter->vfs_allocated_count; i++) {
		ping = E1000_PF_CONTROL_MSG;
		if (adapter->vf_data[i].flags & IGB_VF_FLAG_CTS)
			ping |= E1000_VT_MSGTYPE_CTS;
		e1000_write_mbx(hw, &ping, 1, i);
	}
}

static void igb_init_dmac(struct igb_adapter *adapter, u32 pba)
{
	struct e1000_hw *hw = &adapter->hw;
	u32 dmac_thr;
	u16 hwm;
	u32 status;

	if (hw->mac.type == e1000_i211)
		return;

	if (hw->mac.type > e1000_82580) {
		if (adapter->dmac != IGB_DMAC_DISABLE) {
			u32 reg;

			/* force threshold to 0.  */
			E1000_WRITE_REG(hw, E1000_DMCTXTH, 0);

			/*
			 * DMA Coalescing high water mark needs to be greater
			 * than the Rx threshold. Set hwm to PBA - max frame
			 * size in 16B units, capping it at PBA - 6KB.
			 */
			hwm = 64 * pba - adapter->max_frame_size / 16;
			if (hwm < 64 * (pba - 6))
				hwm = 64 * (pba - 6);
			reg = E1000_READ_REG(hw, E1000_FCRTC);
			reg &= ~E1000_FCRTC_RTH_COAL_MASK;
			reg |= ((hwm << E1000_FCRTC_RTH_COAL_SHIFT)
				& E1000_FCRTC_RTH_COAL_MASK);
			E1000_WRITE_REG(hw, E1000_FCRTC, reg);

			/*
			 * Set the DMA Coalescing Rx threshold to PBA - 2 * max
			 * frame size, capping it at PBA - 10KB.
			 */
			dmac_thr = pba - adapter->max_frame_size / 512;
			if (dmac_thr < pba - 10)
				dmac_thr = pba - 10;
			reg = E1000_READ_REG(hw, E1000_DMACR);
			reg &= ~E1000_DMACR_DMACTHR_MASK;
			reg |= ((dmac_thr << E1000_DMACR_DMACTHR_SHIFT)
				& E1000_DMACR_DMACTHR_MASK);

			/* transition to L0x or L1 if available..*/
			reg |= (E1000_DMACR_DMAC_EN | E1000_DMACR_DMAC_LX_MASK);

			/* Check if status is 2.5Gb backplane connection
			 * before configuration of watchdog timer, which is
			 * in msec values in 12.8usec intervals
			 * watchdog timer= msec values in 32usec intervals
			 * for non 2.5Gb connection
			 */
			if (hw->mac.type == e1000_i354) {
				status = E1000_READ_REG(hw, E1000_STATUS);
				if ((status & E1000_STATUS_2P5_SKU) &&
				    (!(status & E1000_STATUS_2P5_SKU_OVER)))
					reg |= ((adapter->dmac * 5) >> 6);
				else
					reg |= ((adapter->dmac) >> 5);
			} else {
				reg |= ((adapter->dmac) >> 5);
			}

			/*
			 * Disable BMC-to-OS Watchdog enable
			 * on devices that support OS-to-BMC
			 */
			if (hw->mac.type != e1000_i354)
				reg &= ~E1000_DMACR_DC_BMC2OSW_EN;
			E1000_WRITE_REG(hw, E1000_DMACR, reg);

			/* no lower threshold to disable coalescing
			 * (smart fifb)-UTRESH=0
			 * */
			E1000_WRITE_REG(hw, E1000_DMCRTRH, 0);

			/* This sets the time to wait before requesting
			 * transition to low power state to number of usecs
			 * needed to receive 1 512 byte frame at gigabit
			 * line rate. On i350 device, time to make transition
			 * to Lx state is delayed by 4 usec with flush disable
			 * bit set to avoid losing mailbox interrupts
			 */
			reg = E1000_READ_REG(hw, E1000_DMCTLX);
			if (hw->mac.type == e1000_i350)
				reg |= IGB_DMCTLX_DCFLUSH_DIS;

			/* in 2.5Gb connection, TTLX unit is 0.4 usec
			 * which is 0x4*2 = 0xA. But delay is still 4 usec
			 */
			if (hw->mac.type == e1000_i354) {
				status = E1000_READ_REG(hw, E1000_STATUS);
				if ((status & E1000_STATUS_2P5_SKU) &&
				    (!(status & E1000_STATUS_2P5_SKU_OVER)))
					reg |= 0xA;
				else
					reg |= 0x4;
			} else {
				reg |= 0x4;
			}
			E1000_WRITE_REG(hw, E1000_DMCTLX, reg);

			/* free space in tx pkt buffer to wake from DMA coal */
			E1000_WRITE_REG(hw, E1000_DMCTXTH, (IGB_MIN_TXPBSIZE -
				(IGB_TX_BUF_4096 + adapter->max_frame_size))
				>> 6);

			/* low power state decision controlled by DMA coal */
			reg = E1000_READ_REG(hw, E1000_PCIEMISC);
			reg &= ~E1000_PCIEMISC_LX_DECISION;
			E1000_WRITE_REG(hw, E1000_PCIEMISC, reg);
		} /* endif adapter->dmac is not disabled */
	} else if (hw->mac.type == e1000_82580) {
		u32 reg = E1000_READ_REG(hw, E1000_PCIEMISC);
		E1000_WRITE_REG(hw, E1000_PCIEMISC,
				reg & ~E1000_PCIEMISC_LX_DECISION);
		E1000_WRITE_REG(hw, E1000_DMACR, 0);
	}
}

/**
 * igb_enable_mas - Media Autosense re-enable after swap
 *
 * @adapter: adapter struct
 **/
static s32  igb_enable_mas(struct igb_adapter *adapter)
{
	struct e1000_hw *hw = &adapter->hw;
	u32 connsw;
	s32 ret_val = E1000_SUCCESS;

	connsw = E1000_READ_REG(hw, E1000_CONNSW);
	if (hw->phy.media_type == e1000_media_type_copper) {
		/* configure for SerDes media detect */
		if (!(connsw & E1000_CONNSW_SERDESD)) {
			connsw |= E1000_CONNSW_ENRGSRC;
			connsw |= E1000_CONNSW_AUTOSENSE_EN;
			E1000_WRITE_REG(hw, E1000_CONNSW, connsw);
			E1000_WRITE_FLUSH(hw);
		} else if (connsw & E1000_CONNSW_SERDESD) {
			/* already SerDes, no need to enable anything */
			return ret_val;
		} else {
			dev_info(pci_dev_to_dev(adapter->pdev),
			"%s:MAS: Unable to configure feature, disabling..\n",
			adapter->netdev->name);
			adapter->flags &= ~IGB_FLAG_MAS_ENABLE;
		}
	}
	return ret_val;
}



void igb_reset(struct igb_adapter *adapter)
{
	struct pci_dev *pdev = adapter->pdev;
	struct e1000_hw *hw = &adapter->hw;
	struct e1000_mac_info *mac = &hw->mac;
	struct e1000_fc_info *fc = &hw->fc;
	u32 pba = 0, tx_space, min_tx_space, min_rx_space, hwm;

	/* Repartition Pba for greater than 9k mtu
	 * To take effect CTRL.RST is required.
	 */
	switch (mac->type) {
	case e1000_i350:
	case e1000_82580:
	case e1000_i354:
		pba = E1000_READ_REG(hw, E1000_RXPBS);
		pba = e1000_rxpbs_adjust_82580(pba);
		break;
	case e1000_82576:
		pba = E1000_READ_REG(hw, E1000_RXPBS);
		pba &= E1000_RXPBS_SIZE_MASK_82576;
		break;
	case e1000_82575:
	case e1000_i210:
	case e1000_i211:
	default:
		pba = E1000_PBA_34K;
		break;
	}

	if ((adapter->max_frame_size > ETH_FRAME_LEN + ETH_FCS_LEN) &&
	    (mac->type < e1000_82576)) {
		/* adjust PBA for jumbo frames */
		E1000_WRITE_REG(hw, E1000_PBA, pba);

		/* To maintain wire speed transmits, the Tx FIFO should be
		 * large enough to accommodate two full transmit packets,
		 * rounded up to the next 1KB and expressed in KB.  Likewise,
		 * the Rx FIFO should be large enough to accommodate at least
		 * one full receive packet and is similarly rounded up and
		 * expressed in KB. */
		pba = E1000_READ_REG(hw, E1000_PBA);
		/* upper 16 bits has Tx packet buffer allocation size in KB */
		tx_space = pba >> 16;
		/* lower 16 bits has Rx packet buffer allocation size in KB */
		pba &= 0xffff;
		/* the tx fifo also stores 16 bytes of information about the tx
		 * but don't include ethernet FCS because hardware appends it */
		min_tx_space = (adapter->max_frame_size +
				sizeof(union e1000_adv_tx_desc) -
				ETH_FCS_LEN) * 2;
		min_tx_space = ALIGN(min_tx_space, 1024);
		min_tx_space >>= 10;
		/* software strips receive CRC, so leave room for it */
		min_rx_space = adapter->max_frame_size;
		min_rx_space = ALIGN(min_rx_space, 1024);
		min_rx_space >>= 10;

		/* If current Tx allocation is less than the min Tx FIFO size,
		 * and the min Tx FIFO size is less than the current Rx FIFO
		 * allocation, take space away from current Rx allocation */
		if (tx_space < min_tx_space &&
		    ((min_tx_space - tx_space) < pba)) {
			pba = pba - (min_tx_space - tx_space);

			/* if short on rx space, rx wins and must trump tx
			 * adjustment */
			if (pba < min_rx_space)
				pba = min_rx_space;
		}
		E1000_WRITE_REG(hw, E1000_PBA, pba);
	}

	/* flow control settings */
	/* The high water mark must be low enough to fit one full frame
	 * (or the size used for early receive) above it in the Rx FIFO.
	 * Set it to the lower of:
	 * - 90% of the Rx FIFO size, or
	 * - the full Rx FIFO size minus one full frame */
	hwm = min(((pba << 10) * 9 / 10),
			((pba << 10) - 2 * adapter->max_frame_size));

	fc->high_water = hwm & 0xFFFFFFF0;	/* 16-byte granularity */
	fc->low_water = fc->high_water - 16;
	fc->pause_time = 0xFFFF;
	fc->send_xon = 1;
	fc->current_mode = fc->requested_mode;

	/* disable receive for all VFs and wait one second */
	if (adapter->vfs_allocated_count) {
		int i;

		/*
		 * Clear all flags except indication that the PF has set
		 * the VF MAC addresses administratively
		 */
		for (i = 0 ; i < adapter->vfs_allocated_count; i++)
			adapter->vf_data[i].flags &= IGB_VF_FLAG_PF_SET_MAC;

		/* ping all the active vfs to let them know we are going down */
		igb_ping_all_vfs(adapter);

		/* disable transmits and receives */
		E1000_WRITE_REG(hw, E1000_VFRE, 0);
		E1000_WRITE_REG(hw, E1000_VFTE, 0);
	}

	/* Allow time for pending master requests to run */
	igb_e1000_reset_hw(hw);
	E1000_WRITE_REG(hw, E1000_WUC, 0);

	if (adapter->flags & IGB_FLAG_MEDIA_RESET) {
		e1000_setup_init_funcs(hw, TRUE);
		igb_check_options(adapter);
		igb_e1000_get_bus_info(hw);
		adapter->flags &= ~IGB_FLAG_MEDIA_RESET;
	}
	if (adapter->flags & IGB_FLAG_MAS_ENABLE) {
		if (igb_enable_mas(adapter))
			dev_err(pci_dev_to_dev(pdev),
				"Error enabling Media Auto Sense\n");
	}
	if (igb_e1000_init_hw(hw))
		dev_err(pci_dev_to_dev(pdev), "Hardware Error\n");

	/*
	 * Flow control settings reset on hardware reset, so guarantee flow
	 * control is off when forcing speed.
	 */
	if (!hw->mac.autoneg)
		igb_e1000_force_mac_fc(hw);

	igb_init_dmac(adapter, pba);
	/* Re-initialize the thermal sensor on i350 devices. */
	if (mac->type == e1000_i350 && hw->bus.func == 0) {
		/*
		 * If present, re-initialize the external thermal sensor
		 * interface.
		 */
		if (adapter->ets)
			e1000_set_i2c_bb(hw);
		e1000_init_thermal_sensor_thresh(hw);
	}

	/*Re-establish EEE setting */
	if (hw->phy.media_type == e1000_media_type_copper) {
		switch (mac->type) {
		case e1000_i350:
		case e1000_i210:
		case e1000_i211:
			e1000_set_eee_i350(hw);
			break;
		case e1000_i354:
			e1000_set_eee_i354(hw);
			break;
		default:
			break;
		}
	}

	if (!netif_running(adapter->netdev))
		igb_power_down_link(adapter);

#ifndef PMON
	igb_update_mng_vlan(adapter);
#endif

	/* Enable h/w to recognize an 802.1Q VLAN Ethernet packet */
	E1000_WRITE_REG(hw, E1000_VET, ETHERNET_IEEE_VLAN_TYPE);


#ifdef HAVE_PTP_1588_CLOCK
	/* Re-enable PTP, where applicable. */
	igb_ptp_reset(adapter);
#endif /* HAVE_PTP_1588_CLOCK */

	e1000_get_phy_info(hw);

	adapter->devrc++;
}


void igb_down(struct igb_adapter *adapter)
{
	struct net_device *netdev = adapter->netdev;
	struct e1000_hw *hw = &adapter->hw;
	u32 tctl, rctl;
	int i;

	/* signal that we're down so the interrupt handler does not
	 * reschedule our watchdog timer */
	set_bit(__IGB_DOWN, &adapter->state);

	/* disable receives in the hardware */
	rctl = E1000_READ_REG(hw, E1000_RCTL);
	E1000_WRITE_REG(hw, E1000_RCTL, rctl & ~E1000_RCTL_EN);
	/* flush and sleep below */

	netif_tx_stop_all_queues(netdev);

	/* disable transmits in the hardware */
	tctl = E1000_READ_REG(hw, E1000_TCTL);
	tctl &= ~E1000_TCTL_EN;
	E1000_WRITE_REG(hw, E1000_TCTL, tctl);
	/* flush both disables and wait for them to finish */
	E1000_WRITE_FLUSH(hw);
	usleep_range(10000, 20000);

	for (i = 0; i < adapter->num_q_vectors; i++)
		napi_disable(&(adapter->q_vector[i]->napi));

	igb_irq_disable(adapter);

	adapter->flags &= ~IGB_FLAG_NEED_LINK_UPDATE;

	del_timer_sync(&adapter->watchdog_timer);
	if (adapter->flags & IGB_FLAG_DETECT_BAD_DMA)
		del_timer_sync(&adapter->dma_err_timer);
	del_timer_sync(&adapter->phy_info_timer);

	netif_carrier_off(netdev);

	/* record the stats before reset*/
	igb_update_stats(adapter);

	adapter->link_speed = 0;
	adapter->link_duplex = 0;

#ifdef HAVE_PCI_ERS
	if (!pci_channel_offline(adapter->pdev))
		igb_reset(adapter);
#else
	igb_reset(adapter);
#endif
	igb_clean_all_tx_rings(adapter);
	igb_clean_all_rx_rings(adapter);
#ifdef IGB_DCA
	/* since we reset the hardware DCA settings were cleared */
	igb_setup_dca(adapter);
#endif
}

static int __igb_close(struct net_device *netdev, bool suspending)
{
	struct igb_adapter *adapter = netdev_priv(netdev);
#ifdef CONFIG_PM_RUNTIME
	struct pci_dev *pdev = adapter->pdev;
#endif /* CONFIG_PM_RUNTIME */

	WARN_ON(test_bit(__IGB_RESETTING, &adapter->state));

#ifdef CONFIG_PM_RUNTIME
	if (!suspending)
		pm_runtime_get_sync(&pdev->dev);
#endif /* CONFIG_PM_RUNTIME */

	igb_down(adapter);

	igb_release_hw_control(adapter);

	igb_free_irq(adapter);

	igb_free_all_tx_resources(adapter);
	igb_free_all_rx_resources(adapter);

#ifdef CONFIG_PM_RUNTIME
	if (!suspending)
		pm_runtime_put_sync(&pdev->dev);
#endif /* CONFIG_PM_RUNTIME */

	return 0;
}

static int igb_close(struct net_device *netdev)
{
	return __igb_close(netdev, false);
}


/**
 * igb_sw_init - Initialize general software structures (struct igb_adapter)
 * @adapter: board private structure to initialize
 *
 * igb_sw_init initializes the Adapter private data structure.
 * Fields are initialized based on PCI device information and
 * OS network device settings (MTU size).
 **/
static int igb_sw_init(struct igb_adapter *adapter)
{
	struct e1000_hw *hw = &adapter->hw;
	struct net_device *netdev = adapter->netdev;
	struct pci_dev *pdev = adapter->pdev;

	/* PCI config space info */

	hw->vendor_id = pdev->vendor;
	hw->device_id = pdev->device;
	hw->subsystem_vendor_id = pdev->subsystem_vendor;
	hw->subsystem_device_id = pdev->subsystem_device;

	pci_read_config_byte(pdev, PCI_REVISION_ID, &hw->revision_id);

	pci_read_config_word(pdev, PCI_COMMAND, &hw->bus.pci_cmd_word);

	/* set default ring sizes */
	adapter->tx_ring_count = IGB_DEFAULT_TXD;
	adapter->rx_ring_count = IGB_DEFAULT_RXD;

	/* set default work limits */
	adapter->tx_work_limit = IGB_DEFAULT_TX_WORK;

	adapter->max_frame_size = netdev->mtu + ETH_HLEN + ETH_FCS_LEN +
					      VLAN_HLEN;

	/* Initialize the hardware-specific values */
	if (e1000_setup_init_funcs(hw, TRUE)) {
		dev_err(pci_dev_to_dev(pdev), "Hardware Initialization Failure\n");
		return -EIO;
	}

	igb_check_options(adapter);

	adapter->mac_table = kzalloc(sizeof(struct igb_mac_addr) *
				     hw->mac.rar_entry_count,
				     GFP_ATOMIC);

	/* Setup and initialize a copy of the hw vlan table array */
	adapter->shadow_vfta = kzalloc(sizeof(u32) * E1000_VFTA_ENTRIES,
					GFP_ATOMIC);

	/* These calls may decrease the number of queues */
	if (hw->mac.type < e1000_i210)
		igb_set_sriov_capability(adapter);

	if (igb_init_interrupt_scheme(adapter, true)) {
		dev_err(pci_dev_to_dev(pdev), "Unable to allocate memory for queues\n");
		return -ENOMEM;
	}

	/* Explicitly disable IRQ since the NIC can be in any state. */
	igb_irq_disable(adapter);

	set_bit(__IGB_DOWN, &adapter->state);
	return 0;
}

/**
 * igb_free_q_vector - Free memory allocated for specific interrupt vector
 * @adapter: board private structure to initialize
 * @v_idx: Index of vector to be freed
 *
 * This function frees the memory allocated to the q_vector.  In addition if
 * NAPI is enabled it will delete any references to the NAPI struct prior
 * to freeing the q_vector.
 **/
static void igb_free_q_vector(struct igb_adapter *adapter, int v_idx)
{
	struct igb_q_vector *q_vector = adapter->q_vector[v_idx];

	if (q_vector->tx.ring)
		adapter->tx_ring[q_vector->tx.ring->queue_index] = NULL;

	if (q_vector->rx.ring)
		adapter->tx_ring[q_vector->rx.ring->queue_index] = NULL;

	adapter->q_vector[v_idx] = NULL;
	netif_napi_del(&q_vector->napi);
#ifndef IGB_NO_LRO
	__skb_queue_purge(&q_vector->lrolist.active);
#endif

	kfree_rcu(q_vector, rcu);
}

/**
 * igb_free_q_vectors - Free memory allocated for interrupt vectors
 * @adapter: board private structure to initialize
 *
 * This function frees the memory allocated to the q_vectors.  In addition if
 * NAPI is enabled it will delete any references to the NAPI struct prior
 * to freeing the q_vector.
 **/
static void igb_free_q_vectors(struct igb_adapter *adapter)
{
	int v_idx = adapter->num_q_vectors;

	adapter->num_tx_queues = 0;
	adapter->num_rx_queues = 0;
	adapter->num_q_vectors = 0;

	while (v_idx--)
		igb_free_q_vector(adapter, v_idx);
}


/**
 * igb_clear_interrupt_scheme - reset the device to a state of no interrupts
 *
 * This function resets the device so that it has 0 rx queues, tx queues, and
 * MSI-X interrupts allocated.
 */
static void igb_clear_interrupt_scheme(struct igb_adapter *adapter)
{
	igb_free_q_vectors(adapter);
	igb_reset_interrupt_capability(adapter);
}


void igb_rar_set(struct igb_adapter *adapter, u32 index)
{
	u32 rar_low, rar_high;
	struct e1000_hw *hw = &adapter->hw;
	u8 *addr = adapter->mac_table[index].addr;
	/* HW expects these in little endian so we reverse the byte order
	 * from network order (big endian) to little endian
	 */
	rar_low = ((u32) addr[0] | ((u32) addr[1] << 8) |
		  ((u32) addr[2] << 16) | ((u32) addr[3] << 24));
	rar_high = ((u32) addr[4] | ((u32) addr[5] << 8));

	/* Indicate to hardware the Address is Valid. */
	if (adapter->mac_table[index].state & IGB_MAC_STATE_IN_USE)
		rar_high |= E1000_RAH_AV;

	if (hw->mac.type == e1000_82575)
		rar_high |= E1000_RAH_POOL_1 * adapter->mac_table[index].queue;
	else
		rar_high |= E1000_RAH_POOL_1 << adapter->mac_table[index].queue;

	E1000_WRITE_REG(hw, E1000_RAL(index), rar_low);
	E1000_WRITE_FLUSH(hw);
	E1000_WRITE_REG(hw, E1000_RAH(index), rar_high);
	E1000_WRITE_FLUSH(hw);
}


/**
 * igb_set_fw_version - Configure version string for ethtool
 * @adapter: adapter struct
 *
 **/
static void igb_set_fw_version(struct igb_adapter *adapter)
{
	struct e1000_hw *hw = &adapter->hw;
	struct e1000_fw_version fw;

	e1000_get_fw_version(hw, &fw);

	switch (hw->mac.type) {
	case e1000_i210:
	case e1000_i211:
		if (!(e1000_get_flash_presence_i210(hw))) {
			snprintf(adapter->fw_version,
			    sizeof(adapter->fw_version),
			    "%2d.%2d-%d",
			    fw.invm_major, fw.invm_minor, fw.invm_img_type);
			break;
		}
		/* fall through */
	default:
		/* if option rom is valid, display its version too*/
		if (fw.or_valid) {
			snprintf(adapter->fw_version,
			    sizeof(adapter->fw_version),
			    "%d.%d, 0x%08x, %d.%d.%d",
			    fw.eep_major, fw.eep_minor, fw.etrack_id,
			    fw.or_major, fw.or_build, fw.or_patch);
		/* no option rom */
		} else {
			if (fw.etrack_id != 0X0000) {
				snprintf(adapter->fw_version,
				    sizeof(adapter->fw_version),
				    "%d.%d, 0x%08x",
				    fw.eep_major, fw.eep_minor, fw.etrack_id);
			} else {
			snprintf(adapter->fw_version,
			    sizeof(adapter->fw_version),
			    "%d.%d.%d",
			    fw.eep_major, fw.eep_minor, fw.eep_build);
			}
		}
		break;
	}
}


static void igb_init_fw(struct igb_adapter *adapter)
{
	struct e1000_fw_drv_info fw_cmd;
	struct e1000_hw *hw = &adapter->hw;
	int i;
	u16 mask;

	if (hw->mac.type == e1000_i210)
		mask = E1000_SWFW_EEP_SM;
	else
		mask = E1000_SWFW_PHY0_SM;
	/* i211 parts do not support this feature */
	if (hw->mac.type == e1000_i211)
		hw->mac.arc_subsystem_valid = false;

	if (!hw->mac.ops.acquire_swfw_sync(hw, mask)) {
		for (i = 0; i <= FW_MAX_RETRIES; i++) {
			E1000_WRITE_REG(hw, E1000_FWSTS, E1000_FWSTS_FWRI);
			fw_cmd.hdr.cmd = FW_CMD_DRV_INFO;
			fw_cmd.hdr.buf_len = FW_CMD_DRV_INFO_LEN;
			fw_cmd.hdr.cmd_or_resp.cmd_resv = FW_CMD_RESERVED;
			fw_cmd.port_num = hw->bus.func;
			fw_cmd.drv_version = FW_FAMILY_DRV_VER;
			fw_cmd.hdr.checksum = 0;
			fw_cmd.hdr.checksum =
				e1000_calculate_checksum((u8 *)&fw_cmd,
							 (FW_HDR_LEN +
							 fw_cmd.hdr.buf_len));
			 e1000_host_interface_command(hw, (u8 *)&fw_cmd,
						      sizeof(fw_cmd));
			if (fw_cmd.hdr.cmd_or_resp.ret_status
			    == FW_STATUS_SUCCESS)
				break;
		}
	} else
		dev_warn(pci_dev_to_dev(adapter->pdev),
			 "Unable to get semaphore, firmware init failed.\n");
	hw->mac.ops.release_swfw_sync(hw, mask);
}


/**
 * igb_init_mas - init Media Autosense feature if enabled in the NVM
 *
 * @adapter: adapter struct
 **/
static void igb_init_mas(struct igb_adapter *adapter)
{
	struct e1000_hw *hw = &adapter->hw;
	u16 eeprom_data;

	e1000_read_nvm(hw, NVM_COMPAT, 1, &eeprom_data);
	switch (hw->bus.func) {
	case E1000_FUNC_0:
		if (eeprom_data & IGB_MAS_ENABLE_0)
			adapter->flags |= IGB_FLAG_MAS_ENABLE;
		break;
	case E1000_FUNC_1:
		if (eeprom_data & IGB_MAS_ENABLE_1)
			adapter->flags |= IGB_FLAG_MAS_ENABLE;
		break;
	case E1000_FUNC_2:
		if (eeprom_data & IGB_MAS_ENABLE_2)
			adapter->flags |= IGB_FLAG_MAS_ENABLE;
		break;
	case E1000_FUNC_3:
		if (eeprom_data & IGB_MAS_ENABLE_3)
			adapter->flags |= IGB_FLAG_MAS_ENABLE;
		break;
	default:
		/* Shouldn't get here */
		dev_err(pci_dev_to_dev(adapter->pdev),
			"%s:AMS: Invalid port configuration, returning\n",
			adapter->netdev->name);
		break;
	}
}


/**
 * igb_set_mac - Change the Ethernet Address of the NIC
 * @netdev: network interface device structure
 * @p: pointer to an address structure
 *
 * Returns 0 on success, negative on failure
 **/
static int igb_set_mac(struct net_device *netdev, void *p)
{
	struct igb_adapter *adapter = netdev_priv(netdev);
	struct e1000_hw *hw = &adapter->hw;
	struct sockaddr *addr = p;

	if (!is_valid_ether_addr(addr->sa_data))
		return -EADDRNOTAVAIL;

	memcpy(netdev->dev_addr, addr->sa_data, netdev->addr_len);
	memcpy(hw->mac.addr, addr->sa_data, netdev->addr_len);

	/* set the correct pool for the new PF MAC address in entry 0 */
	igb_rar_set_qsel(adapter, hw->mac.addr, 0,
			 adapter->vfs_allocated_count);

	return 0;
}



/* Detect and switch function for Media Auto Sense */
static void igb_check_swap_media(struct igb_adapter *adapter)
{
	struct e1000_hw *hw = &adapter->hw;
	u32 ctrl_ext, connsw;
	bool swap_now = false;
	bool link;

	ctrl_ext = E1000_READ_REG(hw, E1000_CTRL_EXT);
	connsw = E1000_READ_REG(hw, E1000_CONNSW);
	link = igb_has_link(adapter);

	/* need to live swap if current media is copper and we have fiber/serdes
	 * to go to.
	 */

	if ((hw->phy.media_type == e1000_media_type_copper) &&
	    (!(connsw & E1000_CONNSW_AUTOSENSE_EN))) {
		swap_now = true;
	} else if (!(connsw & E1000_CONNSW_SERDESD)) {
		/* copper signal takes time to appear */
		if (adapter->copper_tries < 3) {
			adapter->copper_tries++;
			connsw |= E1000_CONNSW_AUTOSENSE_CONF;
			E1000_WRITE_REG(hw, E1000_CONNSW, connsw);
			return;
		} else {
			adapter->copper_tries = 0;
			if ((connsw & E1000_CONNSW_PHYSD) &&
			    (!(connsw & E1000_CONNSW_PHY_PDN))) {
				swap_now = true;
				connsw &= ~E1000_CONNSW_AUTOSENSE_CONF;
				E1000_WRITE_REG(hw, E1000_CONNSW, connsw);
			}
		}
	}

	if (swap_now) {
		switch (hw->phy.media_type) {
		case e1000_media_type_copper:
			dev_info(pci_dev_to_dev(adapter->pdev),
				 "%s:MAS: changing media to fiber/serdes\n",
			adapter->netdev->name);
			ctrl_ext |=
				E1000_CTRL_EXT_LINK_MODE_PCIE_SERDES;
			adapter->flags |= IGB_FLAG_MEDIA_RESET;
			adapter->copper_tries = 0;
			break;
		case e1000_media_type_internal_serdes:
		case e1000_media_type_fiber:
			dev_info(pci_dev_to_dev(adapter->pdev),
				 "%s:MAS: changing media to copper\n",
				 adapter->netdev->name);
			ctrl_ext &=
				~E1000_CTRL_EXT_LINK_MODE_PCIE_SERDES;
			adapter->flags |= IGB_FLAG_MEDIA_RESET;
			break;
		default:
			/* shouldn't get here during regular operation */
			dev_err(pci_dev_to_dev(adapter->pdev),
				"%s:AMS: Invalid media type found, returning\n",
				adapter->netdev->name);
			break;
		}
		E1000_WRITE_REG(hw, E1000_CTRL_EXT, ctrl_ext);
	}
}

/**
 * igb_has_link - check shared code for link and determine up/down
 * @adapter: pointer to driver private info
 **/
bool igb_has_link(struct igb_adapter *adapter)
{
	struct e1000_hw *hw = &adapter->hw;
	bool link_active = FALSE;

	/* get_link_status is set on LSC (link status) interrupt or
	 * rx sequence error interrupt.  get_link_status will stay
	 * false until the igb_e1000_check_for_link establishes link
	 * for copper adapters ONLY
	 */
	switch (hw->phy.media_type) {
	case e1000_media_type_copper:
		if (!hw->mac.get_link_status)
			return true;
	case e1000_media_type_internal_serdes:
		igb_e1000_check_for_link(hw);
		link_active = !hw->mac.get_link_status;
		break;
	case e1000_media_type_unknown:
	default:
		break;
	}

	if (((hw->mac.type == e1000_i210) ||
	     (hw->mac.type == e1000_i211)) &&
	     (hw->phy.id == I210_I_PHY_ID)) {
		if (!netif_carrier_ok(adapter->netdev)) {
			adapter->flags &= ~IGB_FLAG_NEED_LINK_UPDATE;
		} else if (!(adapter->flags & IGB_FLAG_NEED_LINK_UPDATE)) {
			adapter->flags |= IGB_FLAG_NEED_LINK_UPDATE;
			adapter->link_check_timeout = jiffies;
		}
	}

	return link_active;
}


#define IGB_STAGGERED_QUEUE_OFFSET 8

static void igb_spoof_check(struct igb_adapter *adapter)
{
	int j;

	if (!adapter->wvbr)
		return;

	switch (adapter->hw.mac.type) {
	case e1000_82576:
		for (j = 0; j < adapter->vfs_allocated_count; j++) {
			if (adapter->wvbr & (1 << j) ||
			    adapter->wvbr & (1 << (j
				+ IGB_STAGGERED_QUEUE_OFFSET))) {
				DPRINTK(DRV, WARNING,
					"Spoof event(s) detected on VF %d\n",
					j);
				adapter->wvbr &=
					~((1 << j) |
					  (1 << (j +
					  IGB_STAGGERED_QUEUE_OFFSET)));
			}
		}
		break;
	case e1000_i350:
		for (j = 0; j < adapter->vfs_allocated_count; j++) {
			if (adapter->wvbr & (1 << j)) {
				DPRINTK(DRV, WARNING,
					"Spoof event(s) detected on VF %d\n",
					j);
				adapter->wvbr &= ~(1 << j);
			}
		}
		break;
	default:
		break;
	}
}

static void igb_watchdog_task(struct work_struct *work)
{
	struct igb_adapter *adapter = container_of(work,
						   struct igb_adapter,
						   watchdog_task);
	struct e1000_hw *hw = &adapter->hw;
	struct net_device *netdev = adapter->netdev;
	u32 thstat, ctrl_ext, link;
	int i;
	u32 connsw;

	link = igb_has_link(adapter);

	/* Force link down if we have fiber to swap to */
	if (adapter->flags & IGB_FLAG_MAS_ENABLE) {
		if (hw->phy.media_type == e1000_media_type_copper) {
			connsw = E1000_READ_REG(hw, E1000_CONNSW);
			if (!(connsw & E1000_CONNSW_AUTOSENSE_EN))
				link = 0;
		}
	}
	if (adapter->flags & IGB_FLAG_NEED_LINK_UPDATE) {
		if (time_after(jiffies, (adapter->link_check_timeout + HZ)))
			adapter->flags &= ~IGB_FLAG_NEED_LINK_UPDATE;
		else
			link = FALSE;
	}

	if (link) {
		/* Perform a reset if the media type changed. */
		if (hw->dev_spec._82575.media_changed) {
			hw->dev_spec._82575.media_changed = false;
			adapter->flags |= IGB_FLAG_MEDIA_RESET;
			igb_reset(adapter);
		}

		/* Cancel scheduled suspend requests. */
		pm_runtime_resume(netdev->dev.parent);

		if (!netif_carrier_ok(netdev)) {
			u32 ctrl;

			igb_e1000_get_speed_and_duplex(hw,
						   &adapter->link_speed,
						   &adapter->link_duplex);

			ctrl = E1000_READ_REG(hw, E1000_CTRL);
			/* Links status message must follow this format */
			netdev_info(netdev,
				"igb: %s NIC Link is Up %d Mbps %s, Flow Control: %s\n",
				 netdev->name,
				adapter->link_speed,
				adapter->link_duplex == FULL_DUPLEX ?
				 "Full Duplex" : "Half Duplex",
				((ctrl & E1000_CTRL_TFCE) &&
				(ctrl & E1000_CTRL_RFCE)) ? "RX/TX" :
				((ctrl & E1000_CTRL_RFCE) ?  "RX" :
				((ctrl & E1000_CTRL_TFCE) ?  "TX" : "None")));
			/* adjust timeout factor according to speed/duplex */
			adapter->tx_timeout_factor = 1;
			switch (adapter->link_speed) {
			case SPEED_10:
				adapter->tx_timeout_factor = 14;
				break;
			case SPEED_100:
				/* maybe add some timeout factor ? */
				break;
			default:
				break;
			}

			netif_carrier_on(netdev);
			netif_tx_wake_all_queues(netdev);

			igb_ping_all_vfs(adapter);
#ifdef IFLA_VF_MAX
			igb_check_vf_rate_limit(adapter);
#endif /* IFLA_VF_MAX */

			/* link state has changed, schedule phy info update */
			if (!test_bit(__IGB_DOWN, &adapter->state))
				mod_timer(&adapter->phy_info_timer,
					  round_jiffies(jiffies + 2 * HZ));
		}
	} else {
		if (netif_carrier_ok(netdev)) {
			adapter->link_speed = 0;
			adapter->link_duplex = 0;
			/* check for thermal sensor event on i350 */
			if (hw->mac.type == e1000_i350) {
				thstat = E1000_READ_REG(hw, E1000_THSTAT);
				ctrl_ext = E1000_READ_REG(hw, E1000_CTRL_EXT);
				if ((hw->phy.media_type ==
					e1000_media_type_copper) &&
					!(ctrl_ext &
					E1000_CTRL_EXT_LINK_MODE_SGMII)) {
					if (thstat & E1000_THSTAT_PWR_DOWN) {
						netdev_err(netdev,
							"igb: %s The network adapter was stopped because it overheated.\n",
							netdev->name);
					}
					if (thstat &
						E1000_THSTAT_LINK_THROTTLE) {
						netdev_err(netdev,
							"igb: %s The network adapter supported link speed was downshifted because it overheated.\n",
							netdev->name);
					}
				}
			}

			/* Links status message must follow this format */
			netdev_info(netdev, "igb: %s NIC Link is Down\n",
			       netdev->name);
			netif_carrier_off(netdev);
			netif_tx_stop_all_queues(netdev);

			igb_ping_all_vfs(adapter);

			/* link state has changed, schedule phy info update */
			if (!test_bit(__IGB_DOWN, &adapter->state))
				mod_timer(&adapter->phy_info_timer,
					  round_jiffies(jiffies + 2 * HZ));
			/* link is down, time to check for alternate media */
			if (adapter->flags & IGB_FLAG_MAS_ENABLE) {
				igb_check_swap_media(adapter);
				if (adapter->flags & IGB_FLAG_MEDIA_RESET) {
					schedule_work(&adapter->reset_task);
					/* return immediately */
					return;
				}
			}
			pm_schedule_suspend(netdev->dev.parent,
					    MSEC_PER_SEC * 5);

		/* also check for alternate media here */
		} else if (!netif_carrier_ok(netdev) &&
			   (adapter->flags & IGB_FLAG_MAS_ENABLE)) {
			hw->mac.ops.power_up_serdes(hw);
			igb_check_swap_media(adapter);
			if (adapter->flags & IGB_FLAG_MEDIA_RESET) {
				schedule_work(&adapter->reset_task);
				/* return immediately */
				return;
			}
		}
	}

	igb_update_stats(adapter);

	for (i = 0; i < adapter->num_tx_queues; i++) {
		struct igb_ring *tx_ring = adapter->tx_ring[i];
		if (!netif_carrier_ok(netdev)) {
			/* We've lost link, so the controller stops DMA,
			 * but we've got queued Tx work that's never going
			 * to get done, so reset controller to flush Tx.
			 * (Do the reset outside of interrupt context). */
			if (igb_desc_unused(tx_ring) + 1 < tx_ring->count) {
				adapter->tx_timeout_count++;
				schedule_work(&adapter->reset_task);
				/* return immediately since reset is imminent */
				return;
			}
		}

		/* Force detection of hung controller every watchdog period */
		set_bit(IGB_RING_FLAG_TX_DETECT_HANG, &tx_ring->flags);
	}

	/* Cause software interrupt to ensure rx ring is cleaned */
	if (adapter->msix_entries) {
		u32 eics = 0;

		for (i = 0; i < adapter->num_q_vectors; i++)
			eics |= adapter->q_vector[i]->eims_value;
		E1000_WRITE_REG(hw, E1000_EICS, eics);
	} else {
		E1000_WRITE_REG(hw, E1000_ICS, E1000_ICS_RXDMT0);
	}

	igb_spoof_check(adapter);

	/* Reset the timer */
	if (!test_bit(__IGB_DOWN, &adapter->state)) {
		if (adapter->flags & IGB_FLAG_NEED_LINK_UPDATE)
			mod_timer(&adapter->watchdog_timer,
				  round_jiffies(jiffies +  HZ));
		else
			mod_timer(&adapter->watchdog_timer,
				  round_jiffies(jiffies + 2 * HZ));
	}
}


static void igb_watchdog(unsigned long data)
{
	struct igb_adapter *adapter = (struct igb_adapter *)data;
	/* Do the rest outside of interrupt context */
	schedule_work(&adapter->watchdog_task);
}


/**
 * igb_up - Open the interface and prepare it to handle traffic
 * @adapter: board private structure
 **/
int igb_up(struct igb_adapter *adapter)
{
	struct e1000_hw *hw = &adapter->hw;
	int i;

	/* hardware has been reset, we need to reload some things */
	igb_configure(adapter);

	clear_bit(__IGB_DOWN, &adapter->state);

	for (i = 0; i < adapter->num_q_vectors; i++)
		napi_enable(&(adapter->q_vector[i]->napi));

	if (adapter->msix_entries)
		igb_configure_msix(adapter);
	else
		igb_assign_vector(adapter->q_vector[0], 0);

	igb_configure_lli(adapter);

	/* Clear any pending interrupts. */
	E1000_READ_REG(hw, E1000_ICR);
	igb_irq_enable(adapter);

	/* notify VFs that reset has been completed */
	if (adapter->vfs_allocated_count) {
		u32 reg_data = E1000_READ_REG(hw, E1000_CTRL_EXT);

		reg_data |= E1000_CTRL_EXT_PFRSTD;
		E1000_WRITE_REG(hw, E1000_CTRL_EXT, reg_data);
	}

	netif_tx_start_all_queues(adapter->netdev);

	if (adapter->flags & IGB_FLAG_DETECT_BAD_DMA)
		schedule_work(&adapter->dma_err_task);
	/* start the watchdog. */
	hw->mac.get_link_status = 1;
	schedule_work(&adapter->watchdog_task);

	if ((adapter->flags & IGB_FLAG_EEE) &&
	    (!hw->dev_spec._82575.eee_disable))
		adapter->eee_advert = MDIO_EEE_100TX | MDIO_EEE_1000T;

	return 0;
}


void igb_reinit_locked(struct igb_adapter *adapter)
{
	WARN_ON(in_interrupt());
	while (test_and_set_bit(__IGB_RESETTING, &adapter->state))
		usleep_range(1000, 2000);
	igb_down(adapter);
	igb_up(adapter);
	clear_bit(__IGB_RESETTING, &adapter->state);
}

static void igb_reset_task(struct work_struct *work)
{
	struct igb_adapter *adapter;
	adapter = container_of(work, struct igb_adapter, reset_task);

	igb_reinit_locked(adapter);
}


static void igb_dma_err_task(struct work_struct *work)
{
	struct igb_adapter *adapter = container_of(work,
						   struct igb_adapter,
						   dma_err_task);
	int vf;
	struct e1000_hw *hw = &adapter->hw;
	struct net_device *netdev = adapter->netdev;
	u32 hgptc;
	u32 ciaa, ciad;

	hgptc = E1000_READ_REG(hw, E1000_HGPTC);
	if (hgptc) /* If incrementing then no need for the check below */
		goto dma_timer_reset;
	/*
	 * Check to see if a bad DMA write target from an errant or
	 * malicious VF has caused a PCIe error.  If so then we can
	 * issue a VFLR to the offending VF(s) and then resume without
	 * requesting a full slot reset.
	 */

	for (vf = 0; vf < adapter->vfs_allocated_count; vf++) {
		ciaa = (vf << 16) | 0x80000000;
		/* 32 bit read so align, we really want status at offset 6 */
		ciaa |= PCI_COMMAND;
		E1000_WRITE_REG(hw, E1000_CIAA, ciaa);
		ciad = E1000_READ_REG(hw, E1000_CIAD);
		ciaa &= 0x7FFFFFFF;
		/* disable debug mode asap after reading data */
		E1000_WRITE_REG(hw, E1000_CIAA, ciaa);
		/* Get the upper 16 bits which will be the PCI status reg */
		ciad >>= 16;
		if (ciad & (PCI_STATUS_REC_MASTER_ABORT |
			    PCI_STATUS_REC_TARGET_ABORT |
			    PCI_STATUS_SIG_SYSTEM_ERROR)) {
			netdev_err(netdev, "VF %d suffered error\n", vf);
			/* Issue VFLR */
			ciaa = (vf << 16) | 0x80000000;
			ciaa |= 0xA8;
			E1000_WRITE_REG(hw, E1000_CIAA, ciaa);
			ciad = 0x00008000;  /* VFLR */
			E1000_WRITE_REG(hw, E1000_CIAD, ciad);
			ciaa &= 0x7FFFFFFF;
			E1000_WRITE_REG(hw, E1000_CIAA, ciaa);
		}
	}
dma_timer_reset:
	/* Reset the timer */
	if (!test_bit(__IGB_DOWN, &adapter->state))
		mod_timer(&adapter->dma_err_timer,
			  round_jiffies(jiffies + HZ / 10));
}


/**
 * igb_dma_err_timer - Timer Call-back
 * @data: pointer to adapter cast into an unsigned long
 **/
static void igb_dma_err_timer(unsigned long data)
{
	struct igb_adapter *adapter = (struct igb_adapter *)data;
	/* Do the rest outside of interrupt context */
	schedule_work(&adapter->dma_err_task);
}


/* Need to wait a few seconds after link up to get diagnostic information from
 * the phy */
static void igb_update_phy_info(unsigned long data)
{
	struct igb_adapter *adapter = (struct igb_adapter *) data;
	e1000_get_phy_info(&adapter->hw);
}
#if 1
/**
 * igb_probe - Device Initialization Routine
 * @pdev: PCI device information struct
 * @ent: entry in igb_pci_tbl
 *
 * Returns 0 on success, negative on failure
 *
 * igb_probe initializes an adapter identified by a pci_dev structure.
 * The OS initialization, configuring of the adapter private structure,
 * and a hardware reset occur.
 **/
int igb_probe(struct net_device *netdev,struct pci_dev *pdev,
			       const struct pci_device_id *ent)
{
	struct igb_adapter *adapter;
	struct e1000_hw *hw;
	u16 eeprom_data = 0;
	u8 pba_str[E1000_PBANUM_LENGTH];
	s32 ret_val;
	static int global_quad_port_a; /* global quad port a indication */
	int err, pci_using_dac;
	static int cards_found;
	u8 mac_addr0[6] = { 0,1,2,3,4,5 };


#ifndef PMON
	err = pci_enable_device_mem(pdev);
	if (err)
		return err;

	pci_using_dac = 0;
	err = dma_set_mask(pci_dev_to_dev(pdev), DMA_BIT_MASK(64));
	if (!err) {
		err = dma_set_coherent_mask(pci_dev_to_dev(pdev),
			DMA_BIT_MASK(64));
		if (!err)
			pci_using_dac = 1;
	} else {
		err = dma_set_mask(pci_dev_to_dev(pdev), DMA_BIT_MASK(32));
		if (err) {
			err = dma_set_coherent_mask(pci_dev_to_dev(pdev),
				DMA_BIT_MASK(32));
			if (err) {
				IGB_ERR(
				  "No usable DMA configuration, aborting\n");
				goto err_dma;
			}
		}
	}
#endif

#ifndef PMON
#ifndef HAVE_ASPM_QUIRKS
	/* 82575 requires that the pci-e link partner disable the L0s state */
	switch (pdev->device) {
	case E1000_DEV_ID_82575EB_COPPER:
	case E1000_DEV_ID_82575EB_FIBER_SERDES:
	case E1000_DEV_ID_82575GB_QUAD_COPPER:
		pci_disable_link_state(pdev, PCIE_LINK_STATE_L0S);
	default:
		break;
	}

#endif /* HAVE_ASPM_QUIRKS */
#endif

#ifndef PMON
	err = pci_request_selected_regions(pdev,
					   pci_select_bars(pdev,
							   IORESOURCE_MEM),
					   igb_driver_name);
	if (err)
		goto err_pci_reg;

	pci_enable_pcie_error_reporting(pdev);

	pci_set_master(pdev);

	err = -ENOMEM;
#ifdef HAVE_TX_MQ
	netdev = alloc_etherdev_mq(sizeof(struct igb_adapter),
				   IGB_MAX_TX_QUEUES);
#else
	netdev = alloc_etherdev(sizeof(struct igb_adapter));
#endif /* HAVE_TX_MQ */
	if (!netdev)
		goto err_alloc_etherdev;

	SET_MODULE_OWNER(netdev);
	SET_NETDEV_DEV(netdev, &pdev->dev);

	pci_set_drvdata(pdev, netdev);
#endif

	adapter = netdev_priv(netdev);
	adapter->netdev = netdev;
	adapter->pdev = pdev;
	hw = &adapter->hw;
	hw->back = adapter;
	adapter->port_num = hw->bus.func;
	adapter->msg_enable = (1 << debug) - 1;

#ifdef HAVE_PCI_ERS
	err = pci_save_state(pdev);
	if (err)
		goto err_ioremap;
#endif
	err = -EIO;
	hw->hw_addr = ioremap(pci_resource_start(pdev, 0),
			      pci_resource_len(pdev, 0));
	if (!hw->hw_addr)
		goto err_ioremap;

#ifdef HAVE_NET_DEVICE_OPS
	netdev->netdev_ops = &igb_netdev_ops;
#else /* HAVE_NET_DEVICE_OPS */
	netdev->open = &igb_open;
	netdev->stop = &igb_close;
#ifndef PMON
	netdev->get_stats = &igb_get_stats;
#ifdef HAVE_SET_RX_MODE
	netdev->set_rx_mode = &igb_set_rx_mode;
#endif
	netdev->set_multicast_list = &igb_set_rx_mode;
	netdev->set_mac_address = &igb_set_mac;
	netdev->change_mtu = &igb_change_mtu;
	netdev->do_ioctl = &igb_ioctl;
#ifdef HAVE_TX_TIMEOUT
	netdev->tx_timeout = &igb_tx_timeout;
#endif
	netdev->vlan_rx_register = igb_vlan_mode;
	netdev->vlan_rx_add_vid = igb_vlan_rx_add_vid;
	netdev->vlan_rx_kill_vid = igb_vlan_rx_kill_vid;
#ifdef CONFIG_NET_POLL_CONTROLLER
	netdev->poll_controller = igb_netpoll;
#endif
#endif
	netdev->hard_start_xmit = &igb_xmit_frame;
#endif /* HAVE_NET_DEVICE_OPS */
#ifndef PMON
	igb_set_ethtool_ops(netdev);
#ifdef HAVE_TX_TIMEOUT
	netdev->watchdog_timeo = 5 * HZ;
#endif

	strncpy(netdev->name, pci_name(pdev), sizeof(netdev->name) - 1);
#endif

	adapter->bd_number = cards_found;

	/* setup the private structure */
	err = igb_sw_init(adapter);
	if (err)
		goto err_sw_init;

	igb_e1000_get_bus_info(hw);

	hw->phy.autoneg_wait_to_complete = FALSE;
	hw->mac.adaptive_ifs = FALSE;

	/* Copper options */
	if (hw->phy.media_type == e1000_media_type_copper) {
		hw->phy.mdix = AUTO_ALL_MODES;
		hw->phy.disable_polarity_correction = FALSE;
		hw->phy.ms_type = e1000_ms_hw_default;
	}

	if (e1000_check_reset_block(hw))
		dev_info(pci_dev_to_dev(pdev),
			"PHY reset is blocked due to SOL/IDER session.\n");

	/*
	 * features is initialized to 0 in allocation, it might have bits
	 * set by igb_sw_init so we should use an or instead of an
	 * assignment.
	 */
	netdev->features |= NETIF_F_SG |
			    NETIF_F_IP_CSUM |
#ifdef NETIF_F_IPV6_CSUM
			    NETIF_F_IPV6_CSUM |
#endif
#ifdef NETIF_F_TSO
			    NETIF_F_TSO |
#ifdef NETIF_F_TSO6
			    NETIF_F_TSO6 |
#endif
#endif /* NETIF_F_TSO */
#ifdef NETIF_F_RXHASH
			    NETIF_F_RXHASH |
#endif
			    NETIF_F_RXCSUM |
#ifdef NETIF_F_HW_VLAN_CTAG_RX
			    NETIF_F_HW_VLAN_CTAG_RX |
			    NETIF_F_HW_VLAN_CTAG_TX;
#else
			    NETIF_F_HW_VLAN_RX |
			    NETIF_F_HW_VLAN_TX;
#endif

	if (hw->mac.type >= e1000_82576)
		netdev->features |= NETIF_F_SCTP_CSUM;

#ifdef HAVE_NDO_SET_FEATURES
	/* copy netdev features into list of user selectable features */
	netdev->hw_features |= netdev->features;
#ifndef IGB_NO_LRO

	/* give us the option of enabling LRO later */
	netdev->hw_features |= NETIF_F_LRO;
#endif
#else
#ifdef NETIF_F_GRO

	/* this is only needed on kernels prior to 2.6.39 */
	netdev->features |= NETIF_F_GRO;
#endif
#endif

	/* set this bit last since it cannot be part of hw_features */
#ifdef NETIF_F_HW_VLAN_CTAG_FILTER
	netdev->features |= NETIF_F_HW_VLAN_CTAG_FILTER;
#else
	netdev->features |= NETIF_F_HW_VLAN_FILTER;
#endif

#ifdef HAVE_NETDEV_VLAN_FEATURES
	netdev->vlan_features |= NETIF_F_TSO |
				 NETIF_F_TSO6 |
				 NETIF_F_IP_CSUM |
				 NETIF_F_IPV6_CSUM |
				 NETIF_F_SG;

#endif
	if (pci_using_dac)
		netdev->features |= NETIF_F_HIGHDMA;

	adapter->en_mng_pt = igb_e1000_enable_mng_pass_thru(hw);
#ifdef DEBUG
	if (adapter->dmac != IGB_DMAC_DISABLE)
		netdev_info(netdev, "%s: DMA Coalescing is enabled..\n",
			    netdev->name);
#endif

	/* before reading the NVM, reset the controller to put the device in a
	 * known good starting state */
	igb_e1000_reset_hw(hw);

	/* make sure the NVM is good */
	if (e1000_validate_nvm_checksum(hw) < 0) {
		dev_err(pci_dev_to_dev(pdev),
			"The NVM Checksum Is Not Valid\n");
		err = -EIO;
		goto err_eeprom;
	}

	/* copy the MAC address out of the NVM */
	if (igb_e1000_read_mac_addr(hw))
		dev_err(pci_dev_to_dev(pdev), "NVM Read Error\n");

	if (!is_valid_ether_addr(hw->mac.addr)) {
	struct sockaddr addr;
	dev_err(pci_dev_to_dev(pdev), "Invalid MAC Address\n");
	get_ethernet_addr(addr.sa_data);
	igb_set_mac(netdev, &addr);
	memcpy(hw->mac.addr, addr.sa_data, netdev->addr_len);
	}

	memcpy(netdev->dev_addr, hw->mac.addr, netdev->addr_len);
#ifdef ETHTOOL_GPERMADDR
	memcpy(netdev->perm_addr, hw->mac.addr, netdev->addr_len);

	if (!is_valid_ether_addr(netdev->perm_addr)) {
#else
	if (!is_valid_ether_addr(netdev->dev_addr)) {
#endif
		dev_err(pci_dev_to_dev(pdev), "Invalid MAC Address\n");
		err = -EIO;
		goto err_eeprom;
	}

	memcpy(&adapter->mac_table[0].addr, hw->mac.addr, netdev->addr_len);
	adapter->mac_table[0].queue = adapter->vfs_allocated_count;
	adapter->mac_table[0].state = (IGB_MAC_STATE_DEFAULT
					| IGB_MAC_STATE_IN_USE);
	igb_rar_set(adapter, 0);

	/* get firmware version for ethtool -i */
	igb_set_fw_version(adapter);

	/* Check if Media Autosense is enabled */
	if (hw->mac.type == e1000_82580)
		igb_init_mas(adapter);
	setup_timer(&adapter->watchdog_timer, &igb_watchdog,
		    (unsigned long) adapter);
	INIT_WORK(&adapter->watchdog_task, igb_watchdog_task);
	if (adapter->flags & IGB_FLAG_DETECT_BAD_DMA)
		setup_timer(&adapter->dma_err_timer, &igb_dma_err_timer,
			    (unsigned long) adapter);
	setup_timer(&adapter->phy_info_timer, &igb_update_phy_info,
		    (unsigned long) adapter);

	INIT_WORK(&adapter->reset_task, igb_reset_task);
	if (adapter->flags & IGB_FLAG_DETECT_BAD_DMA)
		INIT_WORK(&adapter->dma_err_task, igb_dma_err_task);

	/* Initialize link properties that are user-changeable */
	adapter->fc_autoneg = true;
	hw->mac.autoneg = true;
	hw->phy.autoneg_advertised = 0x2f;

	hw->fc.requested_mode = e1000_fc_default;
	hw->fc.current_mode = e1000_fc_default;

	igb_e1000_validate_mdi_setting(hw);

	/* By default, support wake on port A */
	if (hw->bus.func == 0)
		adapter->flags |= IGB_FLAG_WOL_SUPPORTED;

	/* Check the NVM for wake support for non-port A ports */
	if (hw->mac.type >= e1000_82580)
		hw->nvm.ops.read(hw, NVM_INIT_CONTROL3_PORT_A +
				 NVM_82580_LAN_FUNC_OFFSET(hw->bus.func), 1,
				 &eeprom_data);
	else if (hw->bus.func == 1)
		e1000_read_nvm(hw, NVM_INIT_CONTROL3_PORT_B, 1, &eeprom_data);

	if (eeprom_data & IGB_EEPROM_APME)
		adapter->flags |= IGB_FLAG_WOL_SUPPORTED;

	/* now that we have the eeprom settings, apply the special cases where
	 * the eeprom may be wrong or the board simply won't support wake on
	 * lan on a particular port */
	switch (pdev->device) {
	case E1000_DEV_ID_82575GB_QUAD_COPPER:
		adapter->flags &= ~IGB_FLAG_WOL_SUPPORTED;
		break;
	case E1000_DEV_ID_82575EB_FIBER_SERDES:
	case E1000_DEV_ID_82576_FIBER:
	case E1000_DEV_ID_82576_SERDES:
		/* Wake events only supported on port A for dual fiber
		 * regardless of eeprom setting */
		if (E1000_READ_REG(hw, E1000_STATUS) & E1000_STATUS_FUNC_1)
			adapter->flags &= ~IGB_FLAG_WOL_SUPPORTED;
		break;
	case E1000_DEV_ID_82576_QUAD_COPPER:
	case E1000_DEV_ID_82576_QUAD_COPPER_ET2:
		/* if quad port adapter, disable WoL on all but port A */
		if (global_quad_port_a != 0)
			adapter->flags &= ~IGB_FLAG_WOL_SUPPORTED;
		else
			adapter->flags |= IGB_FLAG_QUAD_PORT_A;
		/* Reset for multiple quad port adapters */
		if (++global_quad_port_a == 4)
			global_quad_port_a = 0;
		break;
	default:
		break;
	}

	/* initialize the wol settings based on the eeprom settings */
	if (adapter->flags & IGB_FLAG_WOL_SUPPORTED)
		adapter->wol |= E1000_WUFC_MAG;

	/* Some vendors want WoL disabled by default, but still supported */
	if ((hw->mac.type == e1000_i350) &&
	    (pdev->subsystem_vendor == PCI_VENDOR_ID_HP)) {
		adapter->flags |= IGB_FLAG_WOL_SUPPORTED;
		adapter->wol = 0;
	}
#ifndef PMON
	device_set_wakeup_enable(pci_dev_to_dev(adapter->pdev),
				 adapter->flags & IGB_FLAG_WOL_SUPPORTED);
#endif

	/* reset the hardware with the new settings */
	igb_reset(adapter);
	adapter->devrc = 0;

#ifdef HAVE_I2C_SUPPORT
	/* Init the I2C interface */
	err = igb_init_i2c(adapter);
	if (err) {
		dev_err(&pdev->dev, "failed to init i2c interface\n");
		goto err_eeprom;
	}
#endif /* HAVE_I2C_SUPPORT */

	/* let the f/w know that the h/w is now under the control of the
	 * driver.
	 */
	igb_get_hw_control(adapter);

#ifndef PMON
	strncpy(netdev->name, "eth%d", IFNAMSIZ);
	err = register_netdev(netdev);
	if (err)
		goto err_register;

#ifdef CONFIG_IGB_VMDQ_NETDEV
	err = igb_init_vmdq_netdevs(adapter);
	if (err)
		goto err_register;
#endif
	/* carrier off reporting is important to ethtool even BEFORE open */
	netif_carrier_off(netdev);

#ifdef IGB_DCA
	if (dca_add_requester(&pdev->dev) == E1000_SUCCESS) {
		adapter->flags |= IGB_FLAG_DCA_ENABLED;
		dev_info(pci_dev_to_dev(pdev), "DCA enabled\n");
		igb_setup_dca(adapter);
	}

#endif
#ifdef HAVE_PTP_1588_CLOCK
	/* do hw tstamp init after resetting */
	igb_ptp_init(adapter);
#endif /* HAVE_PTP_1588_CLOCK */
#endif

	dev_info(pci_dev_to_dev(pdev), "Intel(R) Gigabit Ethernet Network Connection\n");
	/* print bus type/speed/width info */
	dev_info(pci_dev_to_dev(pdev), "%s: (PCIe:%s:%s) ",
		 netdev->name,
		 ((hw->bus.speed == e1000_bus_speed_2500) ? "2.5GT/s" :
		  (hw->bus.speed == e1000_bus_speed_5000) ? "5.0GT/s" :
		  (hw->mac.type == e1000_i354) ? "integrated" : "unknown"),
		 ((hw->bus.width == e1000_bus_width_pcie_x4) ? "Width x4" :
		  (hw->bus.width == e1000_bus_width_pcie_x2) ? "Width x2" :
		  (hw->bus.width == e1000_bus_width_pcie_x1) ? "Width x1" :
		  (hw->mac.type == e1000_i354) ? "integrated" : "unknown"));
	netdev_info(netdev, "MAC: %pM\n", netdev->dev_addr);

	ret_val = e1000_read_pba_string(hw, pba_str, E1000_PBANUM_LENGTH);
	if (ret_val)
		strcpy(pba_str, "Unknown");
	dev_info(pci_dev_to_dev(pdev), "%s: PBA No: %s\n", netdev->name,
		 pba_str);


	/* Initialize the thermal sensor on i350 devices. */
	if (hw->mac.type == e1000_i350) {
		if (hw->bus.func == 0) {
			u16 ets_word;

			/*
			 * Read the NVM to determine if this i350 device
			 * supports an external thermal sensor.
			 */
			e1000_read_nvm(hw, NVM_ETS_CFG, 1, &ets_word);
			if (ets_word != 0x0000 && ets_word != 0xFFFF)
				adapter->ets = true;
			else
				adapter->ets = false;
		}
#ifndef PMON
#ifdef IGB_HWMON

		igb_sysfs_init(adapter);
#else
#ifdef IGB_PROCFS

		igb_procfs_init(adapter);
#endif /* IGB_PROCFS */
#endif /* IGB_HWMON */
#endif
	} else {
		adapter->ets = false;
	}

	if (hw->phy.media_type == e1000_media_type_copper) {
		switch (hw->mac.type) {
		case e1000_i350:
		case e1000_i210:
		case e1000_i211:
			/* Enable EEE for internal copper PHY devices */
			err = e1000_set_eee_i350(hw);
			if ((!err) &&
			    (adapter->flags & IGB_FLAG_EEE))
				adapter->eee_advert =
					MDIO_EEE_100TX | MDIO_EEE_1000T;
			break;
		case e1000_i354:
			if ((E1000_READ_REG(hw, E1000_CTRL_EXT)) &
			    (E1000_CTRL_EXT_LINK_MODE_SGMII)) {
				err = e1000_set_eee_i354(hw);
				if ((!err) &&
				    (adapter->flags & IGB_FLAG_EEE))
					adapter->eee_advert =
					   MDIO_EEE_100TX | MDIO_EEE_1000T;
			}
			break;
		default:
			break;
		}
	}

	/* send driver version info to firmware */
	if ((hw->mac.type >= e1000_i350) &&
	    (e1000_get_flash_presence_i210(hw)))
		igb_init_fw(adapter);

#ifndef IGB_NO_LRO
	if (netdev->features & NETIF_F_LRO)
		dev_info(pci_dev_to_dev(pdev), "Internal LRO is enabled\n");
	else
		dev_info(pci_dev_to_dev(pdev), "LRO is disabled\n");
#endif
	dev_info(pci_dev_to_dev(pdev),
		 "Using %s interrupts. %d rx queue(s), %d tx queue(s)\n",
		 adapter->msix_entries ? "MSI-X" :
		 (adapter->flags & IGB_FLAG_HAS_MSI) ? "MSI" : "legacy",
		 adapter->num_rx_queues, adapter->num_tx_queues);

	cards_found++;

#ifndef PMON
	pm_runtime_put_noidle(&pdev->dev);
#endif
	return 0;

err_register:
	igb_release_hw_control(adapter);
#ifdef HAVE_I2C_SUPPORT
	memset(&adapter->i2c_adap, 0, sizeof(adapter->i2c_adap));
#endif /* HAVE_I2C_SUPPORT */
err_eeprom:
	if (!e1000_check_reset_block(hw))
		igb_e1000_phy_hw_reset(hw);

	if (hw->flash_address)
		iounmap(hw->flash_address);
err_sw_init:
	igb_clear_interrupt_scheme(adapter);
	igb_reset_sriov_capability(adapter);
	iounmap(hw->hw_addr);
err_ioremap:
#ifndef PMON
	free_netdev(netdev);
#endif
err_alloc_etherdev:
#ifndef PMON
	pci_release_selected_regions(pdev,
				     pci_select_bars(pdev, IORESOURCE_MEM));
#endif
err_pci_reg:
err_dma:
#ifndef PMON
	pci_disable_device(pdev);
#endif
	return err;
}
#endif

int igb_test()
{
igb_open(NULL);
}

#include "linux_net_tail.h"
