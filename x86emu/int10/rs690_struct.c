#include "rs690_struct.h"
/*--------------------------------------------------------------------------------*/

/*
 * internal used, most will be filled by vuma init
 */
struct ati_integrated_system_info ati_int_info = {
/* basic */
	0,							// size of this struct, filled by init
	
	0,							// major_ver, filled by init

	0,							// minor_ver, filled by init

	0,							// bootup_engine_clock, filled by init, 10KHz unit

	0,							// bootup_memory_clock, filled by init, 10KHz unit

	0,							// max_system_memory_clock, filled by init, 10KHz unit

	0,							// min_system_memory_clock, filled by init, 10KHz unit

	0,							// num_of_cycle_in_period_hi, filled by init

	0,							// Reserved1

	0,							// Reserved2

	0,							// inter_nb_voltage_low

	0,							// inter_nb_voltage_high

	0,							// Reserved3

	0,							// fsb, MHz unit

	0,							// cap_flag

/* upstream part */
	0,							// pcie_nbcfg_reg7, filled by init

	0,							// k8 memory clock, filled by init

	0,							// k8 sync start delay, filled by init

	0,							// k8 data return time, filled by init

	0,							// max_nb_voltage, filled by init
	
	0,							// min_nb_voltage, filled by init

	0,							// memory_type, filled by init
	
/* others */
	0,							// num of cycles in period, filled by init

	0,							// start_pwm_high_time, filled by init
	
	0,							// ht_link_width, filled by init
	
	0,							// max_nb_voltage_high, filled by init
	
	0							// min_nb_voltage_high, filled by init
};
