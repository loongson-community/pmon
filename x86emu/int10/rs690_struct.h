#include <sys/linux/types.h>

struct ati_integrated_system_info {
	u16	struct_size;				// this struct size
	u8	major_ver;					// major version
	u8	minor_ver;					// minor version
	u32	bootup_engine_clock;		// unit : 10KHz
	u32	bootup_memory_clock;		// unit : 10KHz
	u32	max_system_memory_clock;	// unit : 10KHz
	u32 min_system_memory_clock;	// unit : 10KHz
	u8	num_of_cycles_in_period_hi;	
	u8	reserved1;
	u16	reserved2;
	u16	inter_nb_voltage_low;		// intermidiate PWM value to set the voltage
	u16	inter_nb_voltage_high;		
	u32	reserved3;
	u16	fsb;						// unit : MHz
#define	CAP_FLAG_FAKE_HDMI_SUPPORT		0x01
#define	CAP_FLAG_CLOCK_GATING_ENABLE	0x02

#define	CAP_FLAG_NO_CARD				0x00
#define	CAP_FLAG_AC_CARD				0x04
#define	CAP_FLAG_SDVO					0x08
	u16	cap_flag;

	u16	pcie_nbcfg_reg7;			// NBMISC 0x37 value
	u16 k8_memory_clock;			// k8 memory clock
	u16 k8_sync_start_delay;
	u16 k8_data_return_time;
	u8	max_nb_voltage;
	u8	min_nb_voltage;
	u8	memory_type;				// bits[7:4] = '0001'DDR1 '0010'DDR2 '0011'DDR3

	u8	num_of_cycles_in_period;
	u8	start_pwm_high_time;
	u8	ht_link_width;
	u8	max_nb_voltage_high;
	u8	min_nb_voltage_high;
};
