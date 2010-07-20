#ifndef _SB700_CHIP_H_
#define _SB700_CHIP_H_

#include <sys/linux/types.h>



struct southbridge_amd_sb700_config
{
	u32 ide0_enable : 1;
	u32 sata0_enable : 1;
	u32 hda_viddid;
};

#endif

