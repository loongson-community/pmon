/*
 * KB3310.h
 *	
 *	Author : liujl <liujl@lemote.com>
 *	Date : 2008-04-15
 */

/* We should make ec rom content and pmon port number equal. */
#if	0
#define	HIGH_PORT	0xff2d
#define	LOW_PORT	0xff2e
#define	DATA_PORT	0xff2f
#else
#define	HIGH_PORT	0x0381
#define	LOW_PORT	0x0382
#define	DATA_PORT	0x0383
#endif

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

/* Fan register in KB3310 */
#define	REG_ECFAN_SPEED_LEVEL	0xf4e4
#define	REG_ECFAN_SWITCH		0xf4d2

/* access ec register content */
static inline void wrec(unsigned short reg, unsigned char val)
{
	*( (volatile unsigned char *)(0xbfd00000 | HIGH_PORT) ) = (reg & 0xff00) >> 8;
	*( (volatile unsigned char *)(0xbfd00000 | LOW_PORT) ) = (reg & 0x00ff);
	*( (volatile unsigned char *)(0xbfd00000 | DATA_PORT) ) = val;
}

static inline unsigned char rdec(unsigned short reg)
{
	*( (volatile unsigned char *)(0xbfd00000 | HIGH_PORT) ) = (reg & 0xff00) >> 8;
	*( (volatile unsigned char *)(0xbfd00000 | LOW_PORT) ) = (reg & 0x00ff);
	return (*( (volatile unsigned char *)(0xbfd00000 | DATA_PORT) ));
}

