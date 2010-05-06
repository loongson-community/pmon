/*
 * KB3310.h
 *	
 *	Author : liujl <liujl@lemote.com>
 *	Date : 2008-04-15
 */

/* We should make ec rom content and pmon port number equal. */
/* if you use MX serial EC ROM, please select the option, otherwise mask it */

#include <linux/io.h>

#define	HIGH_PORT	0x0381
#define	LOW_PORT	0x0382
#define	DATA_PORT	0x0383

/* xbi register in kb3310 */
#define	XBI_BANK	0xFE00
#define	XBISEG0		0xA0
#define	XBISEG1		0xA1
#define	XBIRSV2		0xA2
#define	XBIRSV3		0xA3
#define	XBIRSV4		0xA4
#define	XBICFG		0xA5
#define	XBICS		0xA6
#define	XBIWE		0xA7
#define	XBISPIA0	0xA8
#define	XBISPIA1	0xA9
#define	XBISPIA2	0xAA
#define	XBISPIDAT	0xAB
#define	XBISPICMD	0xAC
#define	XBISPICFG	0xAD
#define	XBISPIDATR	0xAE
#define	XBISPICFG2	0xAF

/* XBI relative registers */
#define REG_XBISEG0     0xFEA0
#define REG_XBISEG1     0xFEA1
#define REG_XBIRSV2     0xFEA2
#define REG_XBIRSV3     0xFEA3
#define REG_XBIRSV4     0xFEA4
#define REG_XBICFG      0xFEA5
#define REG_XBICS       0xFEA6
#define REG_XBIWE       0xFEA7
#define REG_XBISPIA0    0xFEA8
#define REG_XBISPIA1    0xFEA9
#define REG_XBISPIA2    0xFEAA
#define REG_XBISPIDAT   0xFEAB
#define REG_XBISPICMD   0xFEAC
#define REG_XBISPICFG   0xFEAD
/* bits definition for REG_XBISPICFG */
#define SPICFG_AUTO_CHECK       0x01
#define SPICFG_SPI_BUSY         0x02
#define SPICFG_DUMMY_READ       0x04
#define SPICFG_EN_SPICMD        0x08
#define SPICFG_LOW_SPICS        0x10
#define SPICFG_EN_SHORT_READ    0x20
#define SPICFG_EN_OFFSET_READ   0x40
#define SPICFG_EN_FAST_READ     0x80

#define SPICMD_WRITE_STATUS     0x01
#define SPICMD_BYTE_PROGRAM     0x02
#define SPICMD_READ_BYTE        0x03
#define SPICMD_WRITE_DISABLE	0x04
#define SPICMD_READ_STATUS      0x05
#define SPICMD_WRITE_ENABLE     0x06
#define SPICMD_HIGH_SPEED_READ 	0x0B
#define SPICMD_POWER_DOWN       0xB9
#define SPICMD_SST_EWSR         0x50
#define SPICMD_SST_SEC_ERASE    0x20
#define SPICMD_SST_BLK_ERASE    0x52
#define SPICMD_SST_CHIP_ERASE   0x60
#define SPICMD_FRDO             0x3B
#define SPICMD_SEC_ERASE        0xD7
#define SPICMD_BLK_ERASE        0xD8
#define SPICMD_CHIP_ERASE       0xC7

/* SMBUS relative register block according to the EC datasheet. */
#define	REG_SMBTCRC		0xff92
#define	REG_SMBPIN		0xff93
#define	REG_SMBCFG		0xff94
#define	REG_SMBEN		0xff95
#define	REG_SMBPF		0xff96
#define	REG_SMBRCRC		0xff97
#define	REG_SMBPRTCL	0xff98
#define	REG_SMBSTS		0xff99
#define	REG_SMBADR		0xff9a
#define	REG_SMBCMD		0xff9b
#define	REG_SMBDAT_START		0xff9c
#define	REG_SMBDAT_END			0xffa3
#define	SMBDAT_SIZE				8
#define	REG_SMBRSA		0xffa4
#define	REG_SMBCNT		0xffbc
#define	REG_SMBAADR		0xffbd
#define	REG_SMBADAT0	0xffbe
#define	REG_SMBADAT1	0xffbf

/* battery registers */
#define REG_BAT_CURRENT_HIGH        0xF784  // battery in/out current high byte
#define REG_BAT_CURRENT_LOW         0xF785  // battery in/out current low byte
#define REG_BAT_VOLTAGE_HIGH        0xF786  // battery current voltage high byte
#define REG_BAT_VOLTAGE_LOW         0xF787  // battery current voltage low byte
#define REG_BAT_TEMPERATURE_HIGH    0xF788  // battery current temperature high byte
#define REG_BAT_TEMPERATURE_LOW     0xF789  // battery current temperature low byte
#define REG_BAT_RELATIVE_CAP_HIGH   0xF492  // relative capacity high byte
#define REG_BAT_RELATIVE_CAP_LOW    0xF493  // relative capacity low byte

#define REG_BAT_STATUS              0xF4B0
#define BIT_BAT_STATUS_LOW              (1 << 5)
#define BIT_BAT_STATUS_DESTROY          (1 << 2)
#define BIT_BAT_STATUS_FULL             (1 << 1)
#define BIT_BAT_STATUS_IN               (1 << 0)	// 1=has battery, 0=no battery

/* temperature & fan registers */
#define REG_TEMPERATURE_VALUE       0xF458  // current temperature value
#define REG_FAN_CONTROL             0xF4D2  // fan control
#define BIT_FAN_CONTROL_ON              (1 << 0)
#define BIT_FAN_CONTROL_OFF             (0 << 0)
#define REG_FAN_STATUS              0xF4DA  // fan status 
#define BIT_FAN_STATUS_ON               (1 << 0)
#define BIT_FAN_STATUS_OFF              (0 << 0)
#define REG_FAN_SPEED_HIGH          0xFE22  // fan speed high byte
#define REG_FAN_SPEED_LOW           0xFE23  // fan speed low byte
#define REG_FAN_SPEED_LEVEL         0xF4E4  // fan speed level, from 1 to 5
/* fan speed divider */
#define FAN_SPEED_DIVIDER       480000  // (60 * 1000 * 1000 / 62.5 / 2)
/* fan status : on or off */
#define FAN_STATUS_ON           0x01
#define FAN_STATUS_OFF          0x00
/* fan is not running */
#define FAN_SPEED_NONE          0x00
/* temperature negative or positive */
#define TEMPERATURE_POSITIVE        0x00
#define TEMPERATURE_NEGATIVE        0x01
/* temperature value is zero */
#define TEMPERATURE_NONE        0x00

/* watchdog timer registers */
#define	REG_WDTCFG		0xfe80
#define	REG_WDTPF		0xfe81
#define REG_WDT         0xfe82

/* lpc configure register */
#define	REG_LPCCFG		0xfe95

/* 8051 reg */
#define	REG_PXCFG		0xff14

/* Fan register in KB3310 */
#define	REG_ECFAN_SPEED_LEVEL	0xf4e4
#define	REG_ECFAN_SWITCH		0xf4d2

#define EC_ROM_PRODUCT_ID_SPANSION	0x01
#define EC_ROM_PRODUCT_ID_MXIC		0xC2
#define EC_ROM_PRODUCT_ID_AMIC		0x37
#define EC_ROM_PRODUCT_ID_EONIC		0x1C

/* version burned address */
#define	VER_ADDR		0xf7a2
#define	VER_MAX_SIZE	7
#define	EC_ROM_MAX_SIZE	0xf7a8

/* ec delay time 500us for register and status access */
#define	EC_REG_DELAY	500	//unit : us
/* timeout value for programming */
#define EC_FLASH_TIMEOUT        0x1000  // ec program timeout
#define EC_CMD_TIMEOUT          0x1000  // command checkout timeout including cmd to port or state flag check
#define EC_SPICMD_STANDARD_TIMEOUT      (4 * 1000)      // unit : us
#define EC_MAX_DELAY_UNIT       (10)                    // every time for polling
#define SPI_FINISH_WAIT_TIME    10
/* EC content max size */
#define EC_CONTENT_MAX_SIZE     (64 * 1024)
/* ec rom flash id size and array : Manufacture ID[1], Device ID[2] */
//#ifndef NO_ROM_ID_NEEDED
#define	EC_ROM_ID_SIZE	3
extern unsigned char ec_rom_id[EC_ROM_ID_SIZE];
//#endif
/* delay function */
extern void delay(int microseconds);
/* version array */
extern unsigned char ec_ver[VER_MAX_SIZE];

/* base address for io access */
#undef	MIPS_IO_BASE
#define	MIPS_IO_BASE	(0xbfd00000)

/* EC access port for sci communication */
#define	EC_CMD_PORT			0x66
#define	EC_STS_PORT			0x66
#define	EC_DATA_PORT		0x62
#define	CMD_INIT_IDLE_MODE	0xdd
#define	CMD_EXIT_IDLE_MODE	0xdf
#define	CMD_INIT_RESET_MODE	0xd8
#define	CMD_REBOOT_SYSTEM	0x8c

/* ec internal register */
#define	REG_POWER_MODE		0xF710
#define	FLAG_NORMAL_MODE	0x00
#define	FLAG_IDLE_MODE		0x01
#define	FLAG_RESET_MODE		0x02

/* read and write operation */
#undef	read_port
#undef	write_port
#define	read_port(x)	 (*(volatile unsigned char *)(MIPS_IO_BASE | x))
#define	write_port(x, y) (*(volatile unsigned char *)(MIPS_IO_BASE | x) = y)	

/* ec update program flag */
#define	PROGRAM_FLAG_NONE		0x00
#define	PROGRAM_FLAG_VERSION	0x01
#define	PROGRAM_FLAG_ROM		0x02

/* EC state */
#define EC_STATE_IDLE   		0x00    // ec in idle state
#define EC_STATE_BUSY   		0x01    // ec in busy state

//extern void delay(int us);
/* access ec register content */
static inline void wrec(unsigned short reg, unsigned char val)
{
	*( (volatile unsigned char *)(mips_io_port_base| HIGH_PORT) ) = (reg & 0xff00) >> 8;
	*( (volatile unsigned char *)(mips_io_port_base| LOW_PORT) ) = (reg & 0x00ff);
	*( (volatile unsigned char *)(mips_io_port_base| DATA_PORT) ) = val;
}

static inline unsigned char rdec(unsigned short reg)
{
	*( (volatile unsigned char *)(mips_io_port_base| HIGH_PORT) ) = (reg & 0xff00) >> 8;
	*( (volatile unsigned char *)(mips_io_port_base| LOW_PORT) ) = (reg & 0x00ff);
	return (*( (volatile unsigned char *)(mips_io_port_base| DATA_PORT) ));
}
