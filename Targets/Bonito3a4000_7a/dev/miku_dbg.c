
#include <sys/linux/types.h>
#include <pmon.h>
#include <stdio.h>
#include <machine/pio.h>
#include "target/ls7a.h"
#include "miku.h"

static int smc_kick()
{
    printf("SMC Kick up\n");
    readl(0xbfe00420) |= 0x100;
    return 0;
}

static int miku_flash(int ac, char *av[])
{
	char    buf[512] = {0};

	if (ac == 2) {
		sprintf(buf,"load -r -f 0xbd600000 %s", av[1]);
        return do_cmd(buf);
	} else {
        printf("Invaled file path\n");
        return -1;
    }
}

static int calc_freq(int ac, char *av[])
{
	int i, timeout, cur, sec, cnt;
	unsigned long md_pipefreq, md_cpufreq ;

	md_pipefreq = 660000000;
	md_cpufreq = 60000000;

/* whd : USE HPET to calculate the frequency,
 *       reduce the booting delay and improve the frequency accuracy.
 *       when use the RTC counter of 7A, it cost 160us+ for one read,
 *       but if we use the HPET counter, it only cost ~300ns for one read,
 *       so the HPET has good accuracy even use less time */

	outl(LS7A_HPET_CONF, 0x1);//Enable main clock

	/*
	 * Do the next twice to make sure we run from cache
	 */
	for (i = 2; i != 0; i--) {
		timeout = 10000000;

		sec = inl(LS7A_HPET_MAIN);//get time now
		cnt = CPU_GetCOUNT();
		cur = (inl(LS7A_HPET_PERIOD) / 1000000);
		sec = sec + (100000000 / cur);//go 100 ms
		do {
			timeout--;
			cur = (inl(LS7A_HPET_MAIN));
		} while (timeout != 0 && (cur < sec));

		cnt = CPU_GetCOUNT() - cnt;
		if (timeout == 0) {
			printf("time out!\n");
			break;	/* Get out if clock is not running */
		}
	}

	/*
	 *  Calculate the external bus clock frequency.
	 */
	if (timeout != 0) {
		md_pipefreq = cnt / 1000;

		if((cnt % 1000) >= 500)//to make rounding
			md_pipefreq = md_pipefreq + 1;

		md_pipefreq *= 20000;
		/* we have no simple way to read multiplier value
		 */
		md_cpufreq = 66000000;
	}
		cur = (inl(LS7A_HPET_PERIOD) / 1000000);
	printf("cpu freq %u, cnt %u\n", md_pipefreq, cnt);

	outl(LS7A_HPET_CONF, 0x0);//Disable main clock
	return 0;
}

static int en_miku_fe()
{
	u32 arg;
	do_service_request(CMD_GET_FEATURES, &arg);
	printf("MIKU Feature FLAG: %x\n", arg);
	do_service_request(CMD_SET_ENABLED_FEATURES, &arg);
	return 0;
}

static int ask_cpu_temp()
{
	struct sensor_info_args arg;
	arg.sensor_id = 0x0;
	arg.info_type = SENSOR_INFO_TYPE_FLAGS;
	do_service_request(CMD_GET_SENSOR_STATUS, &arg);
	printf("PMON: MIKU sensor info FLAG: %x\n", arg.val);
	arg.sensor_id = 0x0;
	arg.info_type = SENSOR_INFO_TYPE_TEMP;
	do_service_request(CMD_GET_SENSOR_STATUS, &arg);
	printf("PMON: MIKU CPUTEMP: %d\n", arg.temp);
	return 0;
}

static int miku_fan_manual()
{
	struct fan_info_args arg;
	arg.fan_id = 0x0;
	arg.info_type = FAN_INFO_FLAGS;
	do_service_request(CMD_GET_FAN_INFO, &arg);
	printf("PMON: MIKU Fan FLAG: %x\n", arg.val);
	arg.fan_id = 0x0;
	arg.info_type = FAN_INFO_FLAGS;
	arg.val = FAN_FLAG_MANUAL;
	do_service_request(CMD_SET_FAN_INFO, &arg);
	arg.fan_id = 0x0;
	arg.info_type = FAN_INFO_LEVEL;
	arg.val = 0x8;
	do_service_request(CMD_SET_FAN_INFO, &arg);
	return 0;
}

static int miku_dvfs_get_levels()
{
	struct freq_level_args arg_l;
	struct freq_info_args arg_f;
	int i;

	do_service_request(CMD_GET_FREQ_LEVELS, &arg_l);
	printf("PMON: MIKU_DVFS Levels - Min: %u, Normal %u, boost: %u\n", arg_l.min_level, arg_l.max_normal_level, arg_l.max_boost_level);

	for (i = arg_l.min_level; i <= arg_l.max_boost_level; i++) {
		arg_f.index = FREQ_INFO_INDEX_LEVEL_FREQ;
		arg_f.info = i;
		do_service_request(CMD_GET_FREQ_INFO, &arg_f);
		printf("PMON: MIKU_DVFS Levels %u, Freq: %u\n", i, arg_f.info);
	}


	return 0;
}

static int miku_dvfs_set_level(int ac, unsigned char *av[])
{
	unsigned char mask = (unsigned char)strtoul(av[1], NULL, 0);
	unsigned char lvl = (unsigned char)strtoul(av[2], NULL, 0);
	struct freq_level_setting_args arg; 

	if (ac != 3) {
		printf("WTF is your input?\n");
		return -1;
	}

	printf("Setting Core Mask: %u, Level: %u\n", mask, lvl);
	arg.cpumask = mask;
	arg.level = lvl;

	do_service_request(CMD_SET_CPU_LEVEL, &arg);
}

static int miku_dvfs_get_cur_freq()
{
	struct freq_info_args arg_f;
	int i;

	for (i = 0; i < 4; i++) {
		arg_f.index = FREQ_INFO_INDEX_CORE_FREQ;
		arg_f.info = i;
		do_service_request(CMD_GET_FREQ_INFO, &arg_f);
		printf("PMON: MIKU_DVFS Core %u, Current Freq: %u\n", i, arg_f.info);
	}

	return 0;
}

static const Cmd Cmds[] = {
	{"Miku"},
	{"smc_kick", "", NULL, "Kick UP SMC and deadloop CPU", smc_kick, 1, 5, 0},
	{"calc_freq", "", NULL, "Recalc Main freq", calc_freq, 1, 5, 0},
	{"miku_flash", "", NULL, "Burn Miku binary to SPI flash", miku_flash, 1, 5, 0},
	{"miku_en_fe", "", NULL, "Enable all miku features", en_miku_fe, 1, 5, 0},
	{"miku_cputemp", "", NULL, "Ask CPU Temp", ask_cpu_temp, 1, 5, 0},
	{"miku_fan_manual", "", NULL, "Set Fan into manual mode", miku_fan_manual, 1, 5, 0},
	{"miku_dvfs_get_levels", "", NULL, "Get Freq Levels", miku_dvfs_get_levels, 1, 5, 0},
	{"miku_dvfs_set_level", "", NULL, "Set Freq level (param: mask, level)", miku_dvfs_set_level, 1, 5, 0},
	{"miku_dvfs_get_cur_freq", "", NULL, "Get Current Freq for all cores", miku_dvfs_get_cur_freq, 1, 5, 0},
	{0, 0}
};

static void init_cmd __P((void)) __attribute__ ((constructor));
static void init_cmd()
{
	cmdlist_expand(Cmds, 1);
}
