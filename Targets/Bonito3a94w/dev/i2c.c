#include <sys/linux/types.h>
#include <sys/linux/io.h>

#include <pmon.h>

#include <target/via686b.h>
#include "../pci/rs780_cmn.h"
#include "lib/libc/memset.c"

#define W83795_ADDR (0X2c << 1)
#define BANK_SEL_REG 0X0

#define SMBUS_BASE_ADDR 0X90
#define I2C_BUS_CONF 0Xd2

#define TEMP_CTRL1_REG			0X4
#define TEMP_CTRL2_REG			0X5
#define W83795_REG_FAN(index)           (0x2E + (index))
#define W83795_REG_FAN_MIN_HL(index)    (0xB6 + (index))
#define W83795_REG_FAN_MIN_LSB(index)   (0xC4 + (index) / 2)
#define W83795_REG_FAN_MIN_LSB_SHIFT(index) \
        (((index) & 1) ? 4 : 0)

#define W83795_REG_VID_CTRL             0x6A

#define W83795_REG_ALARM_CTRL           0x40
#define ALARM_CTRL_RTSACS               (1 << 7)
#define W83795_REG_ALARM(index)         (0x41 + (index))
#define W83795_REG_CLR_CHASSIS          0x4D
#define W83795_REG_BEEP(index)          (0x50 + (index))

#define W83795_REG_OVT_CFG              0x58
#define OVT_CFG_SEL                     (1 << 7)


#define W83795_REG_FCMS1                0x201
#define W83795_REG_FCMS2                0x208
#define W83795_REG_TFMR(index)          (0x202 + (index))
#define W83795_REG_FOMC                 0x20F

#define W83795_REG_TSS(index)           (0x209 + (index))

#define TSS_MAP_RESERVED                0xff

#define PWM_OUTPUT                      0
#define PWM_FREQ                        1
#define PWM_START                       2
#define PWM_NONSTOP                     3
#define PWM_STOP_TIME                   4
#define W83795_REG_PWM(index, nr)       (0x210 + (nr) * 8 + (index))

#define W83795_REG_FTSH(index)          (0x240 + (index) * 2)
#define W83795_REG_FTSL(index)          (0x241 + (index) * 2)
#define W83795_REG_TFTS                 0x250

#define TEMP_PWM_TTTI                   0
#define TEMP_PWM_CTFS                   1
#define TEMP_PWM_HCT                    2
#define TEMP_PWM_HOT                    3
#define W83795_REG_TTTI(index)          (0x260 + (index))
#define W83795_REG_CTFS(index)          (0x268 + (index))
#define W83795_REG_HT(index)            (0x270 + (index))

#define SF4_TEMP                        0
#define SF4_PWM                         1
#define W83795_REG_SF4_TEMP(temp_num, index) \
        (0x280 + 0x10 * (temp_num) + (index))
#define W83795_REG_SF4_PWM(temp_num, index) \
        (0x288 + 0x10 * (temp_num) + (index))

#define W83795_REG_DTSC                 0x301
#define W83795_REG_DTSE                 0x302
#define W83795_REG_DTS(index)           (0x26 + (index))
#define W83795_REG_PECI_TBASE(index)    (0x320 + (index))

#define DTS_CRIT                        0
#define DTS_CRIT_HYST                   1
#define DTS_WARN                        2
#define DTS_WARN_HYST                   3
#define W83795_REG_DTS_EXT(index)       (0xB2 + (index))

#define SETUP_PWM_DEFAULT               0
#define SETUP_PWM_UPTIME                1
#define SETUP_PWM_DOWNTIME              2
#define W83795_REG_SETUP_PWM(index)    (0x20C + (index))

//the i2c_read and i2c_write are for the sb700.
static int i2c_read(int type,u8 addr,u8 reg,u8 *buf,int count)
{
	int i;
	int device,offset;
	char c;
	device = addr;
	offset = reg;
	device |= 1;
	memset(buf,-1,count);
	switch(type&0xff)
	{
		case I2C_SINGLE:
			for(i=0;i<count;i++)
			{
				linux_outb(device,SMBUS_HOST_ADDRESS);
				linux_outb(offset+i,SMBUS_HOST_COMMAND);
				linux_outb(0x8,SMBUS_HOST_CONTROL);
				if((c=linux_inb(SMBUS_HOST_STATUS))&0x1f)
				{
					//clear the status
					linux_outb(c,SMBUS_HOST_STATUS);
				}

				linux_outb(linux_inb(SMBUS_HOST_CONTROL)|0x40,SMBUS_HOST_CONTROL);

				while(linux_inb(SMBUS_HOST_STATUS)&SMBUS_HOST_STATUS_BUSY);

				if((c=linux_inb(SMBUS_HOST_STATUS))&0x1f)
				{
					linux_outb(c,SMBUS_HOST_STATUS);
				}

				buf[i]=linux_inb(SMBUS_HOST_DATA0);
			}
			break;
		case I2C_SMB_BLOCK:
			linux_outb(device,SMBUS_HOST_ADDRESS); //0xd3
			linux_outb(offset,SMBUS_HOST_COMMAND);
			linux_outb(count,SMBUS_HOST_DATA0);
			linux_outb(0x14,SMBUS_HOST_CONTROL); //0x14
			if((c=linux_inb(SMBUS_HOST_STATUS))&0x1f)
			{
				linux_outb(c,SMBUS_HOST_STATUS);
			}

			linux_outb(linux_inb(SMBUS_HOST_CONTROL)|0x40,SMBUS_HOST_CONTROL);

			while(linux_inb(SMBUS_HOST_STATUS)&SMBUS_HOST_STATUS_BUSY);

			if((c=linux_inb(SMBUS_HOST_STATUS))&0x1f)
			{
				linux_outb(c,SMBUS_HOST_STATUS);
			}

			for(i=0;i<count;i++)
			{
				buf[i]=linux_inb(SMBUS_HOST_DATA1+1);
			}
			break;

		default: return 0;break;
	}
	return count;
}

static int i2c_write(int type,u8 addr,u8 reg,u8 *buf,int count)
{
	int i;
	int device,offset;
	char c;
	device = addr;
	offset = reg;
	device &= ~1;
	switch(type)
	{
		case I2C_SINGLE:
			for(i=0;i<count;i++)
			{
				linux_outb(device,SMBUS_HOST_ADDRESS);
				linux_outb(offset+i,SMBUS_HOST_COMMAND);
				linux_outb(0x8,SMBUS_HOST_CONTROL); 
				if((c=linux_inb(SMBUS_HOST_STATUS))&0x1f)
				{
					linux_outb(c,SMBUS_HOST_STATUS);
				}

				linux_outb(buf[i],SMBUS_HOST_DATA0);

				linux_outb(linux_inb(SMBUS_HOST_CONTROL)|0x40,SMBUS_HOST_CONTROL);

				while(linux_inb(SMBUS_HOST_STATUS)&SMBUS_HOST_STATUS_BUSY);

				if((c=linux_inb(SMBUS_HOST_STATUS))&0x1f)
				{
					linux_outb(c,SMBUS_HOST_STATUS);
				}

			}
			break;
		case I2C_SMB_BLOCK:
			linux_outb(device,SMBUS_HOST_ADDRESS); //0xd3
			linux_outb(offset,SMBUS_HOST_COMMAND);
			linux_outb(count,SMBUS_HOST_DATA0);
			linux_outb(0x14,SMBUS_HOST_CONTROL); //0x14
			if((c=linux_inb(SMBUS_HOST_STATUS))&0x1f)
			{
				linux_outb(c,SMBUS_HOST_STATUS);
			}

			for(i=0;i<count;i++)
				linux_outb(buf[i],SMBUS_HOST_DATA1+1);
			c=linux_inb(SMBUS_HOST_CONTROL);
			linux_outb(c|0x40,SMBUS_HOST_CONTROL);

			while(linux_inb(SMBUS_HOST_STATUS)&SMBUS_HOST_STATUS_BUSY);

			if((c=linux_inb(SMBUS_HOST_STATUS))&0x1f)
			{
				linux_outb(c,SMBUS_HOST_STATUS);
			}
			break;
		default:return -1;break;
	}
	return count;
}

static void init_i2c0(void)
{
	u32 dev, val;

	//SMbus bus 00:devcie 20:func 0
	dev = _pci_make_tag(0x0, 0x14, 0x0);
	//copy form function PROBE_NODE_DIMM;
	pci_write_config16(dev, SMBUS_BASE_ADDR, 0xeee1);
	val = pci_read_config8(dev, SMBUS_BASE_ADDR);
	pci_write_config8(dev, SMBUS_BASE_ADDR, val | 0x1);
}

static void w83795_set_bank(u8 bank)
{
	u8 tmp;
	//i2c_read(read 1 byte, device addr, command, return value, length is 1);
	i2c_read(I2C_SINGLE, W83795_ADDR, BANK_SEL_REG, &tmp, 1);
	//get the set bank value.
	tmp = ((tmp & 0xf0 )| bank);
	//i2c_write(write 1 byte, device addr, command, write value, length is 1);
	i2c_write(I2C_SINGLE, W83795_ADDR, BANK_SEL_REG, &tmp, 1);

}

static u8 w83795_read(u16 reg)
{
	u8 tmp;

	/*(reg >> 8) get the bank number.*/
	w83795_set_bank(reg >> 8);

	i2c_read(I2C_SINGLE, W83795_ADDR, (reg & 0xff), &tmp, 1);

	return tmp;
}
static void w83795_write(u16 reg,u8 value)
{
	u8 tmp = value;

	/*(reg >> 8) get the bank number.*/
	w83795_set_bank(reg >> 8);

	i2c_write(I2C_SINGLE, W83795_ADDR, (reg & 0xff), &tmp, 1);
}

void w83795_config(void)
{
	u8 tmp;

	init_i2c0();

	//configuration W83795 in Thermal Cruise Mode.
	/*set fan output mode,make the fanctl1 in dc,fanctl2 in pwm.*/
	tmp = w83795_read(W83795_REG_FOMC);
	w83795_write(W83795_REG_FOMC, (tmp & (~0x2) | 0x1));
	/*selet the temperature2/5 source TR6*/
	tmp = w83795_read(W83795_REG_TSS(0)) & 0xf;
	w83795_write(W83795_REG_TSS(0),(0x3 << 4) | tmp);
	tmp = w83795_read(W83795_REG_TSS(1)) & 0xf;
	w83795_write(W83795_REG_TSS(1),(0x3 << 4) | tmp);
	/*make the temperature6 mapping fanctl2,the temp6 source is TR6*/
	w83795_write(W83795_REG_TFMR(5), 0x2);
	/*ensure fan control in Thermal Cruise Mode*/
	tmp = w83795_read(W83795_REG_FCMS1);
	w83795_write(W83795_REG_FCMS1, tmp & 0xfc);
	tmp = w83795_read(W83795_REG_FCMS2);
	w83795_write(W83795_REG_FCMS2, tmp & 0xfc);
	/*set the full speed temperature*/
	tmp = w83795_read(W83795_REG_CTFS(5));
	w83795_write(W83795_REG_CTFS(5),70);
	/*set the target temperature 32*/
	tmp = w83795_read(W83795_REG_TTTI(5));
	w83795_write(W83795_REG_TTTI(5),38);

	/*configuration the two register for enable voltage monitor*/
	tmp = w83795_read(TEMP_CTRL1_REG);
	w83795_write(TEMP_CTRL1_REG,(tmp & 0x1c) | 0x2);
	tmp = w83795_read(TEMP_CTRL2_REG);
	w83795_write(TEMP_CTRL2_REG,(tmp & 0xf) | 0xa0);
}

static void w83795_read_test(void)
{
	u8 tmp;
	u32 dev, val;
        int i;

        tmp = w83795_read(W83795_REG_FCMS1);
	printf("---  W83795_REG_FCMS1 = %x\n",tmp);
        tmp = w83795_read(W83795_REG_FCMS2);
	printf("---  W83795_REG_FCMS2 = %x\n",tmp);
        for (i = 0; i < 6; i++) {
		tmp = w83795_read(W83795_REG_TFMR(i));
		printf("---  W83795_REG_TFMR(%d) = %x\n",i,tmp);
        }
        for (i = 0; i < 3; i++) {
		tmp = w83795_read(W83795_REG_TSS(i));
		printf("---  W83795_REG_TSS(%d) = %x\n",i,tmp);
        }
        for (i = 0; i < 6; i++) {
		tmp = w83795_read(W83795_REG_CTFS(i));
		printf("---  W83795_REG_CTFS(%d) = %x\n",i,tmp);
        }
	for (i = 0; i < 6; i++) {
		tmp = w83795_read(W83795_REG_TTTI(i));
		printf("---  W83795_REG_TTTI(%d) = %x\n",i,tmp);
        }
        for (i = 0; i < 6; i++) {
		tmp = w83795_read(W83795_REG_HT(i));
		printf("---  W83795_REG_HT(%d) = %x\n",i,tmp);
        }
}

static const Cmd Cmds[] = {
	{"w83795"},
	{"w83795_read", "", NULL, "w83579 test", w83795_read_test, 1, 5, 0},
	{0, 0}
};

static void init_cmd __P((void)) __attribute__ ((constructor));
static void init_cmd()
{
	cmdlist_expand(Cmds, 1);
}

