#ifndef _RS780_CHIP_H_
#define _RS780_CHIP_H_

#include <sys/linux/types.h>


struct southbridge_amd_rs780_config
{
	u32 vga_rom_address;		/* The location that the VGA rom has been appened. */
	u8 gppsb_configuration;	/* The configuration of General Purpose Port, A/B/C/D/E. */
	u8 gpp_configuration;	/* The configuration of General Purpose Port, C/D. */
	//u8 gpp_configuration;	/* The configuration of General Purpose Port, A/B/C/D/E. */
	u16 port_enable;		/* Which port is enabled? GFX(2,3), GPP(4,5,6,7) */
	u8 gfx_dev2_dev3;	/* for GFX Core initialization REFCLK_SEL */
	u8 gfx_dual_slot;		/* Is it dual graphics slots */
	u8 gfx_lane_reversal;	/* Single/Dual slot lan reversal */
	u8 gfx_tmds;		/* whether support TMDS? */
	u8 gfx_compliance;	/* whether support compliance? */
	u8 gfx_reconfiguration;	/* Dynamic Lind Width Control */
	u8 gfx_link_width;	/* Desired width of lane 2 */
};

#endif

