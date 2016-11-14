//cpu fan pwm1 output,pwm2 intput.
#include <stdio.h>
#include <pmon.h>

#define LS2H_PWM_REG_BASE	0xbbea0000 //2H 0x1fea0000
#define PWM_LOWBUF_OFFSET	0x4
#define PWM_FULLBUF_OFFSET	0x8
#define PWM_CTRL_OFFSET		0xc
#define PWM_CTRL_EN		(1 << 0)
#define PWM_CTRL_CAPTE		(1 << 8)

#define LS2H_GPIOCFG		0xbbd000c0 //2H gpiocfg

#define read_w(x) (*(volatile unsigned int*)(x))
#define write_w(x,val) (*(volatile unsigned int*)(x) = val)

//#define DEBUG
#ifdef DEBUG
#define dbg(format, arg...) printf(format, ## arg)
#else
#define dbg(format, arg...)
#endif

static int fan_speed(ac,av)
int ac;
char *av[];
{
	unsigned int tmp, tmp1, tmp_speed, total_val = 0, i;

	read_w(LS2H_GPIOCFG) |= ((0xf << 12) | (1 << 17));//pwm enable,pwm2 output
	dbg("gpiocfg->%x\n", read_w(LS2H_GPIOCFG));
	delay(0x400000);//wait for the pwm intput value valide.
	for (i = 0;i < 20;i++) {
		read_w(LS2H_PWM_REG_BASE + (0x10 * 2) + PWM_CTRL_OFFSET) |= (PWM_CTRL_CAPTE | PWM_CTRL_EN);
		dbg("pwm2 ctrl->%x\n", read_w(LS2H_GPIOCFG));
speed_read:
		tmp = read_w(LS2H_PWM_REG_BASE + (0x10 * 2) + PWM_LOWBUF_OFFSET);
		dbg("tmp->%x\n", tmp);
		tmp1 = read_w(LS2H_PWM_REG_BASE + (0x10 * 2) + PWM_FULLBUF_OFFSET);
		dbg("tmp1->%x\n", tmp1);
		if ((tmp > tmp1) || (tmp < 0x1000) || (tmp < 0x1000))
			goto speed_read;
		
//		tmp_speed = (1000000000 * 60)/((1000 / 125) * tmp1 * 2); //this code is overflow.
		tmp_speed = (62500000 * 6)/(tmp1 / 10);
		dbg("fan speed -> %d\n", tmp_speed);
		total_val += tmp_speed;
	}
	dbg("total fan speed -> %d\n", total_val);
	tmp_speed = total_val / 20;
	if(ac == 1)
		printf("fan speed -> %d\n", tmp_speed);
	return tmp_speed;
}

static int fan_set(ac, av)
int ac;
char *av[];
{
	unsigned int speed, tmp_speed, tmp_speed1, goal_speed, tmp_reg;
	int i;
	if (ac < 2) {
		printf("parameter is error!\n");
		return -1;
	}
	
	if (read_w(LS2H_PWM_REG_BASE + (0x10 * 1) + PWM_FULLBUF_OFFSET) == 0) {
		read_w(LS2H_GPIOCFG) |= ((0xf << 12) | (1 << 17));//pwm enable
		dbg("gpiocfg->%x\n", read_w(LS2H_GPIOCFG));
		write_w(LS2H_PWM_REG_BASE + (0x10 * 1) + PWM_FULLBUF_OFFSET,0x1000);
		write_w(LS2H_PWM_REG_BASE + (0x10 * 1) + PWM_LOWBUF_OFFSET,0x800);
		write_w(LS2H_PWM_REG_BASE + (0x10 * 1) + PWM_CTRL_OFFSET,PWM_CTRL_EN);
	}
	goal_speed = atoi(av[1]);
	printf("goal fan speed %d\n",goal_speed);
	if ((goal_speed > 4700) && (goal_speed < 2200)) {
		printf("goal speed can not complete!The fan speed is (2200 - 4600)\n");
		return -2;
	}
	speed = fan_speed();
	tmp_reg = read_w(LS2H_PWM_REG_BASE + (0x10 * 1) + PWM_LOWBUF_OFFSET);

	if (goal_speed > speed){
		for (i = tmp_reg;i > 0x80; i -= 0x40) {
			dbg("value 0x%x\n",i);
			write_w(LS2H_PWM_REG_BASE + (0x10 * 1) + PWM_LOWBUF_OFFSET, i);
			speed = fan_speed();
			if (speed > goal_speed) {
				tmp_speed1 = speed - goal_speed;
				dbg("tmp_speed1 %d tmp_speed %d\n",tmp_speed1,tmp_speed);
				if(tmp_speed1 > tmp_speed)
					write_w(LS2H_PWM_REG_BASE + (0x10 * 1) + PWM_LOWBUF_OFFSET, i + 0x40);
				break;
			}
			tmp_speed = goal_speed - speed;
		}
	} else if (goal_speed < speed) {
		for (i = tmp_reg;i < 0x1000; i += 0x40) {
			dbg("2 value 0x%x\n",i);
			write_w(LS2H_PWM_REG_BASE + (0x10 * 1) + PWM_LOWBUF_OFFSET, i);
			speed = fan_speed();
			if (speed < goal_speed) {
				tmp_speed1 = goal_speed - speed;
				dbg("2 tmp_speed1 %d tmp_speed %d\n",tmp_speed1,tmp_speed);
				if(tmp_speed1 > tmp_speed)
					write_w(LS2H_PWM_REG_BASE + (0x10 * 1) + PWM_LOWBUF_OFFSET, i - 0x40);
				break;
			}
			tmp_speed = speed - goal_speed;
		}
	}
	printf("ctual fan speed %d\n",fan_speed());
	return 0;	
}

static const Cmd Cmds[] =
{
	{"Misc"},
	{"fan_speed",	"", 0, "3a2h fan speed test ", fan_speed, 1, 99, 0},
	{"fan_set",	"", 0, "3a2h set fan speed test ", fan_set, 1, 99, 0},
	{0, 0}
};

static void init_cmd __P((void)) __attribute__ ((constructor));

void
init_cmd()
{
	cmdlist_expand(Cmds, 1);
}
