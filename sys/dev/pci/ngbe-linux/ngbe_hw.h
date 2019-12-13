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
 */

#ifndef _NGBE_HW_H_
#define _NGBE_HW_H_

#define NGBE_EMC_INTERNAL_DATA         0x00
#define NGBE_EMC_INTERNAL_THERM_LIMIT  0x20
#define NGBE_EMC_DIODE1_DATA           0x01
#define NGBE_EMC_DIODE1_THERM_LIMIT    0x19
#define NGBE_EMC_DIODE2_DATA           0x23
#define NGBE_EMC_DIODE2_THERM_LIMIT    0x1A
#define NGBE_EMC_DIODE3_DATA           0x2A
#define NGBE_EMC_DIODE3_THERM_LIMIT    0x30

/**
 * Packet Type decoding
 **/
/* ngbe_dec_ptype.mac: outer mac */
enum ngbe_dec_ptype_mac {
	NGBE_DEC_PTYPE_MAC_IP = 0,
	NGBE_DEC_PTYPE_MAC_L2 = 2,
	NGBE_DEC_PTYPE_MAC_FCOE = 3,
};

/* ngbe_dec_ptype.[e]ip: outer&encaped ip */
#define NGBE_DEC_PTYPE_IP_FRAG (0x4)
enum ngbe_dec_ptype_ip {
	NGBE_DEC_PTYPE_IP_NONE = 0,
	NGBE_DEC_PTYPE_IP_IPV4 = 1,
	NGBE_DEC_PTYPE_IP_IPV6 = 2,
	NGBE_DEC_PTYPE_IP_FGV4 =
		(NGBE_DEC_PTYPE_IP_FRAG | NGBE_DEC_PTYPE_IP_IPV4),
	NGBE_DEC_PTYPE_IP_FGV6 =
		(NGBE_DEC_PTYPE_IP_FRAG | NGBE_DEC_PTYPE_IP_IPV6),
};

/* ngbe_dec_ptype.etype: encaped type */
enum ngbe_dec_ptype_etype {
	NGBE_DEC_PTYPE_ETYPE_NONE = 0,
	NGBE_DEC_PTYPE_ETYPE_IPIP = 1, /* IP+IP */
	NGBE_DEC_PTYPE_ETYPE_IG = 2, /* IP+GRE */
	NGBE_DEC_PTYPE_ETYPE_IGM = 3, /* IP+GRE+MAC */
	NGBE_DEC_PTYPE_ETYPE_IGMV = 4, /* IP+GRE+MAC+VLAN */
};

/* ngbe_dec_ptype.proto: payload proto */
enum ngbe_dec_ptype_prot {
	NGBE_DEC_PTYPE_PROT_NONE = 0,
	NGBE_DEC_PTYPE_PROT_UDP = 1,
	NGBE_DEC_PTYPE_PROT_TCP = 2,
	NGBE_DEC_PTYPE_PROT_SCTP = 3,
	NGBE_DEC_PTYPE_PROT_ICMP = 4,
	NGBE_DEC_PTYPE_PROT_TS = 5, /* time sync */
};

/* ngbe_dec_ptype.layer: payload layer */
enum ngbe_dec_ptype_layer {
	NGBE_DEC_PTYPE_LAYER_NONE = 0,
	NGBE_DEC_PTYPE_LAYER_PAY2 = 1,
	NGBE_DEC_PTYPE_LAYER_PAY3 = 2,
	NGBE_DEC_PTYPE_LAYER_PAY4 = 3,
};

struct ngbe_dec_ptype {
	u32 ptype:8;
	u32 known:1;
	u32 mac:2; /* outer mac */
	u32 ip:3; /* outer ip*/
	u32 etype:3; /* encaped type */
	u32 eip:3; /* encaped ip */
	u32 prot:4; /* payload proto */
	u32 layer:3; /* payload layer */
};
typedef struct ngbe_dec_ptype ngbe_dptype;


u16 ngbe_get_pcie_msix_count(struct ngbe_hw *hw);
s32 ngbe_init_hw(struct ngbe_hw *hw);
s32 ngbe_start_hw(struct ngbe_hw *hw);
s32 ngbe_clear_hw_cntrs(struct ngbe_hw *hw);
s32 ngbe_read_pba_string(struct ngbe_hw *hw, u8 *pba_num,
				  u32 pba_num_size);
s32 ngbe_get_mac_addr(struct ngbe_hw *hw, u8 *mac_addr);
s32 ngbe_get_bus_info(struct ngbe_hw *hw);
void ngbe_set_pci_config_data(struct ngbe_hw *hw, u16 link_status);
void ngbe_set_lan_id_multi_port_pcie(struct ngbe_hw *hw);
s32 ngbe_stop_adapter(struct ngbe_hw *hw);

s32 ngbe_led_on(struct ngbe_hw *hw, u32 index);
s32 ngbe_led_off(struct ngbe_hw *hw, u32 index);

s32 ngbe_set_rar(struct ngbe_hw *hw, u32 index, u8 *addr, u64 pools,
			  u32 enable_addr);
s32 ngbe_clear_rar(struct ngbe_hw *hw, u32 index);
s32 ngbe_init_rx_addrs(struct ngbe_hw *hw);
s32 ngbe_update_mc_addr_list(struct ngbe_hw *hw, u8 *mc_addr_list,
				      u32 mc_addr_count,
				      ngbe_mc_addr_itr func, bool clear);
s32 ngbe_update_uc_addr_list(struct ngbe_hw *hw, u8 *addr_list,
				      u32 addr_count, ngbe_mc_addr_itr func);
s32 ngbe_enable_mc(struct ngbe_hw *hw);
s32 ngbe_disable_mc(struct ngbe_hw *hw);
s32 ngbe_disable_sec_rx_path(struct ngbe_hw *hw);
s32 ngbe_enable_sec_rx_path(struct ngbe_hw *hw);

s32 ngbe_fc_enable(struct ngbe_hw *hw);
void ngbe_fc_autoneg(struct ngbe_hw *hw);
s32 ngbe_setup_fc(struct ngbe_hw *hw);

s32 ngbe_validate_mac_addr(u8 *mac_addr);
s32 ngbe_acquire_swfw_sync(struct ngbe_hw *hw, u32 mask);
void ngbe_release_swfw_sync(struct ngbe_hw *hw, u32 mask);
s32 ngbe_disable_pcie_master(struct ngbe_hw *hw);

s32 ngbe_set_vmdq(struct ngbe_hw *hw, u32 rar, u32 vmdq);
s32 ngbe_set_vmdq_san_mac(struct ngbe_hw *hw, u32 vmdq);
s32 ngbe_clear_vmdq(struct ngbe_hw *hw, u32 rar, u32 vmdq);
s32 ngbe_insert_mac_addr(struct ngbe_hw *hw, u8 *addr, u32 vmdq);
s32 ngbe_init_uta_tables(struct ngbe_hw *hw);
s32 ngbe_set_vfta(struct ngbe_hw *hw, u32 vlan,
			 u32 vind, bool vlan_on);
s32 ngbe_set_vlvf(struct ngbe_hw *hw, u32 vlan, u32 vind,
			   bool vlan_on, bool *vfta_changed);
s32 ngbe_clear_vfta(struct ngbe_hw *hw);
s32 ngbe_find_vlvf_slot(struct ngbe_hw *hw, u32 vlan);

void ngbe_set_mac_anti_spoofing(struct ngbe_hw *hw, bool enable, int pf);
void ngbe_set_vlan_anti_spoofing(struct ngbe_hw *hw, bool enable, int vf);
void ngbe_set_ethertype_anti_spoofing(struct ngbe_hw *hw,
					bool enable, int vf);
s32 ngbe_get_device_caps(struct ngbe_hw *hw, u16 *device_caps);
void ngbe_set_rxpba(struct ngbe_hw *hw, int num_pb, u32 headroom,
			     int strategy);
s32 ngbe_set_fw_drv_ver(struct ngbe_hw *hw, u8 maj, u8 min,
				 u8 build, u8 ver);
s32 ngbe_reset_hostif(struct ngbe_hw *hw);
u8 ngbe_calculate_checksum(u8 *buffer, u32 length);
s32 ngbe_host_interface_command(struct ngbe_hw *hw, u32 *buffer,
				 u32 length, u32 timeout, bool return_data);

void ngbe_clear_tx_pending(struct ngbe_hw *hw);
void ngbe_stop_mac_link_on_d3(struct ngbe_hw *hw);
bool ngbe_mng_present(struct ngbe_hw *hw);
bool ngbe_check_mng_access(struct ngbe_hw *hw);

s32 ngbe_get_thermal_sensor_data(struct ngbe_hw *hw);
s32 ngbe_init_thermal_sensor_thresh(struct ngbe_hw *hw);
void ngbe_enable_rx(struct ngbe_hw *hw);
void ngbe_disable_rx(struct ngbe_hw *hw);
s32 ngbe_setup_mac_link_multispeed_fiber(struct ngbe_hw *hw,
					  u32 speed,
					  bool autoneg_wait_to_complete);
int ngbe_check_flash_load(struct ngbe_hw *hw, u32 check_bit);

/* @ngbe_api.h */
void ngbe_atr_compute_perfect_hash(union ngbe_atr_input *input,
					  union ngbe_atr_input *mask);
u32 ngbe_atr_compute_sig_hash(union ngbe_atr_hash_dword input,
				     union ngbe_atr_hash_dword common);

s32 ngbe_get_link_capabilities(struct ngbe_hw *hw,
				      u32 *speed, bool *autoneg);
enum ngbe_media_type ngbe_get_media_type(struct ngbe_hw *hw);
void ngbe_disable_tx_laser_multispeed_fiber(struct ngbe_hw *hw);
void ngbe_enable_tx_laser_multispeed_fiber(struct ngbe_hw *hw);
void ngbe_flap_tx_laser_multispeed_fiber(struct ngbe_hw *hw);
void ngbe_set_hard_rate_select_speed(struct ngbe_hw *hw,
					u32 speed);
s32 ngbe_setup_mac_link(struct ngbe_hw *hw, u32 speed,
						bool autoneg_wait_to_complete);
void ngbe_init_mac_link_ops(struct ngbe_hw *hw);
s32 ngbe_reset_hw(struct ngbe_hw *hw);
s32 ngbe_identify_phy(struct ngbe_hw *hw);
s32 ngbe_init_ops_common(struct ngbe_hw *hw);
s32 ngbe_enable_rx_dma(struct ngbe_hw *hw, u32 regval);
s32 ngbe_init_ops(struct ngbe_hw *hw);
s32 ngbe_setup_eee(struct ngbe_hw *hw, bool enable_eee);

s32 ngbe_init_flash_params(struct ngbe_hw *hw);
s32 ngbe_read_flash_buffer(struct ngbe_hw *hw, u32 offset,
					  u32 dwords, u32 *data);
s32 ngbe_write_flash_buffer(struct ngbe_hw *hw, u32 offset,
					  u32 dwords, u32 *data);

s32 ngbe_read_eeprom(struct ngbe_hw *hw,
				   u16 offset, u16 *data);
s32 ngbe_read_eeprom_buffer(struct ngbe_hw *hw, u16 offset,
					  u16 words, u16 *data);
s32 ngbe_init_eeprom_params(struct ngbe_hw *hw);
s32 ngbe_update_eeprom_checksum(struct ngbe_hw *hw);
s32 ngbe_calc_eeprom_checksum(struct ngbe_hw *hw);
s32 ngbe_validate_eeprom_checksum(struct ngbe_hw *hw,
					    u16 *checksum_val);
s32 ngbe_update_flash(struct ngbe_hw *hw);
s32 ngbe_write_ee_hostif_buffer(struct ngbe_hw *hw,
				u16 offset, u16 words, u16 *data);
s32 ngbe_write_ee_hostif(struct ngbe_hw *hw, u16 offset,
				u16 data);
s32 ngbe_read_ee_hostif_buffer(struct ngbe_hw *hw,
				u16 offset, u16 words, u16 *data);
s32 ngbe_read_ee_hostif(struct ngbe_hw *hw, u16 offset, u16 *data);

s32 ngbe_read_ee_hostif32(struct ngbe_hw *hw, u16 offset, u32 *data);

u32 rd32_epcs(struct ngbe_hw *hw, u32 addr);
void wr32_epcs(struct ngbe_hw *hw, u32 addr, u32 data);
void wr32_ephy(struct ngbe_hw *hw, u32 addr, u32 data);
s32 ngbe_upgrade_flash_hostif(struct ngbe_hw *hw,  u32 region,
				const u8 *data, u32 size);

s32 ngbe_check_mac_link_zte(struct ngbe_hw *hw,
							u32 *speed,
							bool *link_up,
							bool link_up_wait_to_complete);

s32 ngbe_eepromcheck_cap(struct ngbe_hw *hw, u16 offset,
							   u32 *data);

#endif /* _NGBE_HW_H_ */
