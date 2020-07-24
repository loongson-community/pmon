#include <sys/linux/types.h>
#include <sys/linux/io.h>
#include <stdio.h>

#include <pmon.h>
#include "target/ls7a.h"
#define W83795_ADDR (0X2c << 1)
//#define W83795_ADDR (0X2f << 1)
#define BANK_SEL_REG			0x0

#define SMBUS_BASE_ADDR			0x90
#define I2C_BUS_CONF			0xd2

#define TEMP_CTRL1_REG			0x4
#define TEMP_CTRL2_REG			0x5
#define W83795_REG_FAN(index)		(0x2E + (index))
#define W83795_REG_FAN_MIN_HL(index)	(0xB6 + (index))
#define W83795_REG_FAN_MIN_LSB(index)	(0xC4 + (index) / 2)
#define W83795_REG_FAN_MIN_LSB_SHIFT(index) \
	(((index) & 1) ? 4 : 0)

#define W83795_REG_VID_CTRL		0x6A

#define W83795_REG_ALARM_CTRL		0x40
#define ALARM_CTRL_RTSACS		(1 << 7)
#define W83795_REG_ALARM(index)		(0x41 + (index))
#define W83795_REG_CLR_CHASSIS		0x4D
#define W83795_REG_BEEP(index)		(0x50 + (index))

#define W83795_REG_OVT_CFG		0x58
#define OVT_CFG_SEL			(1 << 7)


#define W83795_REG_FCMS1		0x201
#define W83795_REG_FCMS2		0x208
#define W83795_REG_TFMR(index)		(0x202 + (index))
#define W83795_REG_FOMC			0x20F

#define W83795_REG_TSS(index)		(0x209 + (index))

#define TSS_MAP_RESERVED		0xff

#define PWM_OUTPUT			0
#define PWM_FREQ			1
#define PWM_START			2
#define PWM_NONSTOP			3
#define PWM_STOP_TIME			4
#define W83795_REG_PWM(index, nr)	(0x210 + (nr) * 8 + (index))

#define W83795_REG_FTSH(index)		(0x240 + (index) * 2)
#define W83795_REG_FTSL(index)		(0x241 + (index) * 2)
#define W83795_REG_TFTS			0x250

#define TEMP_PWM_TTTI			0
#define TEMP_PWM_CTFS			1
#define TEMP_PWM_HCT			2
#define TEMP_PWM_HOT			3
#define W83795_REG_TTTI(index)		(0x260 + (index))
#define W83795_REG_CTFS(index)		(0x268 + (index))
#define W83795_REG_HT(index)		(0x270 + (index))

#define SF4_TEMP			0
#define SF4_PWM				1
#define W83795_REG_SF4_TEMP(temp_num, index) \
	(0x280 + 0x10 * (temp_num) + (index))
#define W83795_REG_SF4_PWM(temp_num, index) \
	(0x288 + 0x10 * (temp_num) + (index))

#define W83795_REG_DTSC			0x301
#define W83795_REG_DTSE			0x302
#define W83795_REG_DTS(index)		(0x26 + (index))
#define W83795_REG_PECI_TBASE(index)	(0x320 + (index))

#define DTS_CRIT			0
#define DTS_CRIT_HYST			1
#define DTS_WARN			2
#define DTS_WARN_HYST			3
#define W83795_REG_DTS_EXT(index)	(0xB2 + (index))

#define SETUP_PWM_DEFAULT		0
#define SETUP_PWM_UPTIME		1
#define SETUP_PWM_DOWNTIME		2
#define W83795_REG_SETUP_PWM(index)	(0x20C + (index))

#define FANCTL2
//#define W83795_DEBUG

extern void ls_i2c_init(void);
extern int ls_i2c_read_seq_rand(unsigned char dev_addr,unsigned int data_addr,
				unsigned char *buf, int count);
extern int ls_i2c_write_seq(unsigned char dev_addr,unsigned int data_addr, unsigned char *buf, int count);
extern unsigned int i2c_base_addr;
extern unsigned char word_offset;

static void w83795_set_bank(u8 dev_addr,u8 bank)
{
	u8 tmp;
	//ls_i2c_read_seq_rand(read 1 byte, device addr, command, return value, length is 1);
	ls_i2c_read_seq_rand(dev_addr, BANK_SEL_REG, &tmp, 1);
	//get the set bank value.
	tmp = ((tmp & 0xf0 )| bank);
	//ls_i2c_write_seq(write 1 byte, device addr, command, write value, length is 1);
	ls_i2c_write_seq(dev_addr, BANK_SEL_REG, &tmp, 1);

}

static u8 w83795_read(u8 dev_addr,u16 reg)
{
	u8 tmp;

	/*(reg >> 8) get the bank number.*/
	w83795_set_bank(dev_addr, reg >> 8);

	ls_i2c_read_seq_rand(dev_addr, (reg & 0xff), &tmp, 1);

	return tmp;
}
static void w83795_write(u8 dev_addr,u16 reg,u8 value)
{
	u8 tmp = value;

	/*(reg >> 8) get the bank number.*/
	w83795_set_bank(dev_addr, reg >> 8);

	ls_i2c_write_seq(dev_addr, (reg & 0xff), &tmp, 1);
}

int w83795_config(void)
{
	char i;
	u8 tmp = 0;
	u8 dev_addr;
	i2c_base_addr = LS7A_I2C1_REG_BASE;
	ls_i2c_init();

	for (i = 0;i < 4;i++) {
		dev_addr = 0x58 + (i << 1);
		/*Nuvoton Vender ID*/
		tmp = w83795_read(dev_addr, 0x0fd);
		if (tmp == 0x5c)
			break;
	}
	if (tmp != 0x5c)
		return -1;

	tmp = w83795_read(dev_addr, 0x0);
#ifdef W83795_DEBUG
	printf("bank 0 Address 0x%x\n", tmp);
#endif
	tmp = w83795_read(dev_addr, 0xfe);
#ifdef W83795_DEBUG
	printf("chip ID 0x79: 0x%x\n", tmp);
#endif

	tmp = w83795_read(dev_addr, 0xfc);
#ifdef W83795_DEBUG
	printf("I2C w83795 address: 0x%x\n", tmp);
#endif

	tmp = w83795_read(dev_addr, 0x01);
#ifdef W83795_DEBUG
	printf("I2C config 0x%x\n", tmp);
#endif

	//TEMP CTRL2 (Temperature Monitoring Control Register)p37 bank0 reg0x05
	tmp = w83795_read(dev_addr, 0x05);
	tmp |= 0xf;
	w83795_write(dev_addr, 0x5, tmp);
	tmp = w83795_read(dev_addr, 0x5);
#ifdef W83795_DEBUG
	printf("bank 0 Reg0x05 temp ctrl2=0x%x\n", tmp);
#endif

	//TEMP CTRL1 (Temperature Monitoring Control Register)p36 bank0 reg0x04
	tmp = w83795_read(dev_addr, 0x04);
#ifdef W83795_DEBUG
	printf("bank 0 Reg0x04 temp ctrl1=0x%x\n", tmp);
#endif


	//set TSS(Temperature Source Selection Register)
	//Temp1:sensor2 cpu1(TR1/VDSEN14)
	//Temp2:sensor1 cpu0(TR2)
	w83795_write(dev_addr, 0x209, 0);
	tmp = w83795_read(dev_addr, 0x209);
#ifdef W83795_DEBUG
	printf("bank 2 Reg0x9=0x%x\n", tmp);
#endif

#ifdef FANCTL2
	//set TSS(Temperature Source Selection Register)
	//Temp4:sensor3 sys fan(TR6)
	w83795_write(dev_addr, 0x20a, 0x30);
	tmp = w83795_read(dev_addr, 0x20a);
#ifdef W83795_DEBUG
	printf("bank 2 Reg0xa=0x%x\n", tmp);
#endif
#endif
	//set TFMR(Temperature to Fan mapping Relationships Register)
	tmp = w83795_read(dev_addr, 0x202); //T1FMR
	tmp |= 0x01;
	w83795_write(dev_addr, 0x202, tmp);
	tmp = w83795_read(dev_addr, 0x202);
#ifdef W83795_DEBUG
	printf("bank 2 Reg0x2=0x%x\n", tmp); //temp1 maping fanctl1
#endif

	tmp = w83795_read(dev_addr, 0x203); //T2FMR
	tmp |= 0x01;
	w83795_write(dev_addr, 0x203, tmp);
	tmp = w83795_read(dev_addr, 0x203);
#ifdef W83795_DEBUG
	printf("bank 2 Reg0x3=0x%x\n", tmp); //temp2 maping fanctl1
#endif

#ifdef FANCTL2
	tmp = w83795_read(dev_addr, 0x205); //T4FMR
	tmp |= 0x02;
	w83795_write(dev_addr, 0x205, tmp);
	tmp = w83795_read(dev_addr, 0x205);
#ifdef W83795_DEBUG
	printf("bank 2 Reg0x5=0x%x\n", tmp); //temp4 maping fanctl2
#endif
#endif

	//set DFSP(Default Fan Speed at Power-on)
	tmp = w83795_read(dev_addr, 0x20c);
#ifdef W83795_DEBUG
	printf("Default Fan Speed at Power-on bank 2 Reg0xc=?0x4d: %x\n", tmp); //Default Fan Speed at Power-on
#endif

	//set FCMS(Fan Control Mode Selection)
	//if 0x01 0x08 = 0x0,set FANCTL1 FANCTL2 Thermal Cruise mode
	tmp = w83795_read(dev_addr, 0x201);
#ifdef W83795_DEBUG
	printf("bank 2 Reg0x01 0x%x\n", tmp);
#endif

	tmp = w83795_read(dev_addr, 0x208);
#ifdef W83795_DEBUG
	printf("bank 2 Reg0x08 0x%x\n", tmp);
#endif

	//set CTFS(Critical Temperature to Full Speed all fan)
	w83795_write(dev_addr, 0x268, 60);//T1CTFS
	tmp = w83795_read(dev_addr, 0x268);
#ifdef W83795_DEBUG
	printf("bank 2 Reg0x68 T1CTFS=%d\n", tmp); //set T1CTFS as 65c
#endif

	w83795_write(dev_addr, 0x269, 60);//T2CTFS
	tmp = w83795_read(dev_addr, 0x269);
#ifdef W83795_DEBUG
	printf("bank 2 Reg0x69 T2CTFS %d\n", tmp);//ususall is 65c
#endif

#ifdef FANCTL2
	w83795_write(dev_addr, 0x26b, 60);//T4CTFS
	tmp = w83795_read(dev_addr, 0x26b);
#ifdef W83795_DEBUG
	printf("bank 2 Reg0x69 T2CTFS %d\n", tmp);//ususall is 65c
#endif
#endif

	//TTTI(Target Temperature of Temperature Inputs)
	w83795_write(dev_addr, 0x260, 30);//T1TTI
	tmp = w83795_read(dev_addr, 0x260);
#ifdef W83795_DEBUG
	printf("bank 2 Reg0x60 T1TTI=%d\n", tmp); //set T1TTI as 45c
#endif

	w83795_write(dev_addr, 0x261, 30);//T2TTI
	tmp = w83795_read(dev_addr, 0x261);
#ifdef W83795_DEBUG
	printf("bank 2 Reg0x61 T2TTI=%d\n", tmp); //set T2TTI as 45c
#endif

#ifdef FANCTL2
	w83795_write(dev_addr, 0x263, 30);//T4TTI
	tmp = w83795_read(dev_addr, 0x263);
#ifdef W83795_DEBUG
	printf("bank 2 Reg0x63 T4TTI=%d\n", tmp); //set T2TTI as 45c
#endif
#endif

	//FOMC(Fan Output Mode Control)
	w83795_write(dev_addr, 0x20f, 0);//FOMC
	tmp = w83795_read(dev_addr, 0x20f);

	//Fan Output Nonstop Value (FONV)
	w83795_write(dev_addr, 0x228, 0x80);//FANCTL1
	tmp = w83795_read(dev_addr, 0x228);
#ifdef W83795_DEBUG
	printf("bank 2 Fan nostop value=0x%x\n", tmp);
#endif

#ifdef FANCTL2
	w83795_write(dev_addr, 0x229, 0x80);//FANCTL2
	tmp = w83795_read(dev_addr, 0x229);
#ifdef W83795_DEBUG
	printf("bank 2 Fan nostop value=0x%x\n", tmp);
#endif
#endif

	//FOSV Fan output Start-up Value
	w83795_write(dev_addr, 0x220, 80);//F1OSV
	tmp = w83795_read(dev_addr, 0x220);
#ifdef W83795_DEBUG
	printf("bank 2 Fan Output startup value=%d\n", tmp);
#endif

#ifdef FANCTL2
	w83795_write(dev_addr, 0x221, 80);//F2OSV
	tmp = w83795_read(dev_addr, 0x221);
#ifdef W83795_DEBUG
	printf("bank 2 Fan Output startup value=%d\n", tmp);
#endif
#endif

	//SmartFan Output Step Up Time (SFOSUT)
	w83795_write(dev_addr, 0x20d, 50);//Step Up Time 5s
	tmp = w83795_read(dev_addr, 0x20d);
#ifdef W83795_DEBUG
	printf("bank 2 step up time 0x%x\n", tmp);
#endif

	//SmartFan Output Step Down Time (SFOSDT)
	w83795_write(dev_addr, 0x20e, 50);//Step down Time 5s
	tmp = w83795_read(dev_addr, 0x20e);
#ifdef W83795_DEBUG
	printf("bank 2 step down time 0x%x\n", tmp);
#endif

	//FOST Fan Output Stop Time
	w83795_write(dev_addr, 0x230, 10);//F1OST
	tmp = w83795_read(dev_addr, 0x230);
#ifdef FANCTL2
	w83795_write(dev_addr, 0x231, 10);//F2OST
	tmp = w83795_read(dev_addr, 0x231);
#endif

	//FANIN COUNT(Fan Tachometer Readout high/low Byte Register)
	//make offset of ctfs as 0
	tmp = w83795_read(dev_addr, 0x270);//HT1
	tmp &= 0x0f;
	w83795_write(dev_addr, 0x270, tmp);
	tmp = w83795_read(dev_addr, 0x270);
#ifdef W83795_DEBUG
	printf("bank 2 Fan Output Mode Control=0x%x\n", tmp);
#endif

	tmp = w83795_read(dev_addr, 0x271);//HT2
	tmp &= 0x0f;
	w83795_write(dev_addr, 0x271, tmp);
	tmp = w83795_read(dev_addr, 0x271);
#ifdef W83795_DEBUG
	printf("bank 2 Fan Output Mode Control=0x%x\n", tmp);
#endif

	tmp = w83795_read(dev_addr, 0x272);//HT3
	tmp &= 0x0f;
	w83795_write(dev_addr, 0x272, tmp);
	tmp = w83795_read(dev_addr, 0x272);
#ifdef W83795_DEBUG
	printf("bank 2 Fan Output Mode Control=0x%x\n", tmp);
#endif

	tmp = w83795_read(dev_addr, 0x273);//HT4
	tmp &= 0x0f;
	w83795_write(dev_addr, 0x273, tmp);
	tmp = w83795_read(dev_addr, 0x273);
#ifdef W83795_DEBUG
	printf("bank 2 Fan Output Mode Control=0x%x\n", tmp);
#endif

	tmp = w83795_read(dev_addr, 0x274);//HT5
	tmp &= 0x0f;
	w83795_write(dev_addr, 0x274, tmp);
	tmp = w83795_read(dev_addr, 0x274);
#ifdef W83795_DEBUG
	printf("bank 2 Fan Output Mode Control=0x%x\n", tmp);
#endif

	tmp = w83795_read(dev_addr, 0x275);//HT6
	tmp &= 0x0f;
	w83795_write(dev_addr, 0x275, tmp);
	tmp = w83795_read(dev_addr, 0x275);
#ifdef W83795_DEBUG
	printf("bank 2 Fan Output Mode Control=0x%x\n", tmp);
#endif

	return 0;
}

static void w83795_read_test(void)
{
	u8 tmp,dev_addr = W83795_ADDR;
	u32 dev, val;
	int i;

	i2c_base_addr = LS7A_I2C1_REG_BASE;
	word_offset = 0;
	ls_i2c_init();
	tmp = w83795_read(dev_addr, W83795_REG_FCMS1);
	printf("---  W83795_REG_FCMS1 = %x\n",tmp);
	tmp = w83795_read(dev_addr, W83795_REG_FCMS2);
	printf("---  W83795_REG_FCMS2 = %x\n",tmp);

	for (i = 0; i < 6; i++) {
		tmp = w83795_read(dev_addr, W83795_REG_TFMR(i));
		printf("---  W83795_REG_TFMR(%d) = %x\n",i,tmp);
	}

	for (i = 0; i < 3; i++) {
		tmp = w83795_read(dev_addr, W83795_REG_TSS(i));
		printf("---  W83795_REG_TSS(%d) = %x\n",i,tmp);
	}

	for (i = 0; i < 6; i++) {
		tmp = w83795_read(dev_addr, W83795_REG_CTFS(i));
		printf("---  W83795_REG_CTFS(%d) = %x\n",i,tmp);
	}

	for (i = 0; i < 6; i++) {
		tmp = w83795_read(dev_addr, W83795_REG_TTTI(i));
		printf("---  W83795_REG_TTTI(%d) = %x\n",i,tmp);
	}

	for (i = 0; i < 6; i++) {
		tmp = w83795_read(dev_addr, W83795_REG_HT(i));
		printf("---  W83795_REG_HT(%d) = %x\n",i,tmp);
	}
}

static const Cmd Cmds[] = {
	{"w83795"},
	{"w83795_read", "", NULL, "w83579 test", w83795_read_test, 1, 5, 0},
	{"w83795_cf", "", NULL, "w83579 test", w83795_config, 1, 5, 0},
	{0, 0}
};

static void init_cmd __P((void)) __attribute__ ((constructor));
static void init_cmd()
{
	cmdlist_expand(Cmds, 1);
}

