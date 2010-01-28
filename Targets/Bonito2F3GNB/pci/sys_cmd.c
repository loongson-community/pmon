#include <sys/linux/types.h>
#include <sys/param.h>
#include <sys/device.h>
#include <sys/systm.h>
#include <sys/malloc.h>

#include <dev/pci/pcivar.h>
#include <dev/pci/pcireg.h>
#include <dev/pci/nppbreg.h>

#include <machine/bus.h>

#include <include/bonito.h>
#include <include/kb3310.h>
#include <include/cs5536.h>
#include <include/cs5536_pci.h>
#include <pmon.h>
#include "cs5536_io.h"

/*********************************MSR debug*********************************/

static int cmd_rdmsr(int ac, char *av[])
{
	u32 msr, hi, lo;

	if(!get_rsa(&msr, av[1])) {
		printf("msr : access error!\n");
		return -1;
	}

	_rdmsr(msr, &hi, &lo);
	
	printf("msr : address  %x    hi  %x   lo  %x\n", msr, hi, lo);
	
	return 0;
}

static int cmd_wrmsr(int ac, char *av[])
{
	u32 msr, hi, lo;

	if(!get_rsa(&msr, av[1])){
		printf("msr : access error!\n");
		return -1;
	}else if(!get_rsa(&hi, av[2])){
		printf("hi : access error!\n");
		return -1;
	}else if(!get_rsa(&lo, av[3])){
		printf("lo : access error!\n");
		return -1;
	}

	_wrmsr(msr, hi, lo);
	
	return 0;
}

/*******************************Display debug*******************************/

#define	DISPLAY_MODE_NORMAL		0	// Screen(on);	Hsync(on);	Vsync(on)
#define	DISPLAY_MODE_STANDBY	1	// Screen(off);	Hsync(off);	Vsync(on)
#define	DISPLAY_MODE_SUSPEND	2	// Screen(off);	Hsync(on);	Vsync(off)
#define	DISPLAY_MODE_OFF		3	// Screen(off);	Hsync(off);	Vsync(off)
#define	SEQ_INDEX	0x3c4
#define	SEQ_DATA	0x3c5
static inline unsigned char read_seq(int reg)
{
	*((volatile unsigned char *)(0xbfd00000 | SEQ_INDEX)) = reg;
	return (*(volatile unsigned char *)(0xbfd00000 | SEQ_DATA));
}

static inline void write_seq(int reg, unsigned char data)
{
	*((volatile unsigned char *)(0xbfd00000 | SEQ_INDEX)) = reg;
	*((volatile unsigned char *)(0xbfd00000 | SEQ_DATA)) = data;

	return;
}

static u8 cur_mode;
static u8 SavedSR01, SavedSR20, SavedSR21, SavedSR22, SavedSR23, SavedSR24, SavedSR31, SavedSR34;
static int cmd_powerdebug(int ac, char *av[])
{
	u8 index;
	u8 SR01, SR20, SR21, SR22, SR23, SR24, SR31, SR34;

	if(ac < 2){
		printf("usage : powerdebug %index\n");
		return -1;
	}

	if(!get_rsa(&index, av[1])) {
		printf("powerdebug : access error!\n");
		return -1;
	}
	
	if(cur_mode == DISPLAY_MODE_NORMAL){
		SavedSR01 = read_seq(0x01);
		SavedSR20 = read_seq(0x20);
		SavedSR21 = read_seq(0x21);
		SavedSR22 = read_seq(0x22);
		SavedSR23 = read_seq(0x23);
		SavedSR24 = read_seq(0x24);
		SavedSR31 = read_seq(0x31);
		SavedSR34 = read_seq(0x34);
		printf("normal power register : \n");
		printf("SR01 0x%x   SR20 0x%x   SR21 0x%x   SR22 0x%x   SR23 0x%x\n", 
						SavedSR01, SavedSR20, SavedSR21, SavedSR22, SavedSR23, SavedSR24);
		printf("SR24 0x%x   SR31 0x%x   SR34 0x%x\n", SavedSR24, SavedSR31, SavedSR34);
	}
	SR01 = read_seq(0x01);
	SR20 = read_seq(0x20);
	SR21 = read_seq(0x21);
	SR22 = read_seq(0x22);
	SR23 = read_seq(0x23);
	SR24 = read_seq(0x24);
	SR31 = read_seq(0x31);
	SR34 = read_seq(0x34);

	printf("powerdebug : mode %d\n", index);
	switch(index){
		case	DISPLAY_MODE_NORMAL :
				SR01 &= ~0x20;
				SR20 = SavedSR20;
		        SR21 = SavedSR21;
				SR22 &= ~0x30; 
				SR23 &= ~0xC0;
				SR24 |= 0x01;
				SR31 = SavedSR31;
				SR34 = SavedSR34;
				break;
		case	DISPLAY_MODE_STANDBY :
				SR01 |= 0x20;
				SR20 = (SR20 & ~0xB0) | 0x10;
				SR21 |= 0x88;
				SR22 = (SR22 & ~0x30) | 0x10;
				SR23 = (SR23 & ~0x07) | 0xD8;
				SR24 &= ~0x01;
				SR31 = (SR31 & ~0x07) | 0x00;
				SR34 |= 0x80;
				break;
		case	DISPLAY_MODE_SUSPEND :
				SR01 |= 0x20;
				SR20 = (SR20 & ~0xB0) | 0x10;
				SR21 |= 0x88;
				SR22 = (SR22 & ~0x30) | 0x20;
				SR23 = (SR23 & ~0x07) | 0xD8;
				SR24 &= ~0x01;
				SR31 = (SR31 & ~0x07) | 0x00;
				SR34 |= 0x80;
				break;
		case	DISPLAY_MODE_OFF :
				SR01 |= 0x20;
			    SR20 = (SR20 & ~0xB0) | 0x10;
				SR21 |= 0x88;
				SR22 = (SR22 & ~0x30) | 0x30;
				SR23 = (SR23 & ~0x07) | 0xD8;
				SR24 &= ~0x01;
				SR31 = (SR31 & ~0x07) | 0x00;
				SR34 |= 0x80;
				break;
		default :
				printf("powerdebug : not supported mode setting for display.\n");
				break;
	}

	if( (*((volatile unsigned char *)(0xbfd00000 | 0x3cc))) & 0x01 ){
		while( (*((volatile unsigned char *)(0xbfd00000 | 0x3da))) & 0x08 );
		while( !((*((volatile unsigned char *)(0xbfd00000 | 0x3da))) & 0x08) );
	}else{
		while( (*((volatile unsigned char *)(0xbfd00000 | 0x3ba))) & 0x08 );
		while( !((*((volatile unsigned char *)(0xbfd00000 | 0x3ba))) & 0x08) );
	}

	write_seq(0x01, SR01);
	write_seq(0x20, SR20);
	write_seq(0x21, SR21);
	write_seq(0x22, SR22);
	write_seq(0x23, SR23);
	write_seq(0x24, SR24);
	write_seq(0x31, SR31);
	write_seq(0x34, SR34);

	cur_mode = index;

	return 0;
}

/***********************************EC debug*******************************/

/*
 * rdecreg : read a ec register area.
 */
static int cmd_rdecreg(int ac, char *av[])
{
	u32 start, size, reg;

	size = 0;

	if(ac < 3){
		printf("usage : rdecreg start_addr size\n");
		return -1;
	}

	if(!get_rsa(&start, av[1])) {
		printf("ecreg : access error!\n");
		return -1;
	}

	if(!get_rsa(&size, av[2])){
		printf("ecreg : size error\n");
		return -1;
	}

	if((start > 0x10000) || (size > 0x10000)){
		printf("ecreg : start not available.\n");
	}

	printf("ecreg : reg=0x%x size=0x%x\n",start, size);
	reg = start;
	while(size > 0){
		printf("reg address : 0x%x,  value : 0x%x, value : %c\n", reg, rdec(reg), rdec(reg));
		reg++;
		size--;
	}

	return 0;
}

/*
 * wrecreg : write a ec register area.
 */
static int cmd_wrecreg(int ac, char *av[])
{
	u32 addr;
	u32 value;
	u8 val8;

	if(ac < 3){
		printf("usage : wrecreg addr val\n");
		return -1;
	}

	if(!get_rsa(&addr, av[1])) {
		printf("ecreg : access error!\n");
		return -1;
	}

	if(!get_rsa(&value, av[2])){
		printf("ecreg : size error\n");
		return -1;
	}

	val8 = (u8)value;

	if((addr > 0x10000)){
		printf("ecreg : addr not available.\n");
	}


	printf("ecreg : addr=0x%x value=0x%x\n",addr , val8);
	wrec(addr,val8);

	return 0;
}

//#ifdef WPCE775_DEBUG
#if 1

/* SMBUS relative register block according to the EC(WPCE775x) datasheet. */
#define SMB_BASE_ADDR	0xFFF500
#define SMB_CHNL_SIZE	0x40
#define SMB_CHNL1		0
#define SMB_CHNL2		1
#define SMB_CHNL3		2
#define SMB_CHNL4		3

#define REG_SMB1SDA		0xFFF500
#define REG_SMB2SDA		0xFFF540
#define REG_SMB3SDA		0xFFF580
#define REG_SMB4SDA		0xFFF5C0
#define	SMBSDA_SIZE		8
#define REG_SMBSDA		0x00
#define REG_SMBST		0x02
#define REG_SMB2SDA		0xFFF540
#define REG_SMB2ST		0xFFF542
#define SMBST_XMIT	(1 << 0)	/* Transmit Mode(RO).*/
							/* 0:SMB not in master/slave Transmit mode
							 * 1:SMB in master/slave Transmit mode */
#define SMBST_MASTER	(1 << 1)	/* Master Mode(RO). */
							/* 0:Arbitration loss(BER is set) or Stop condition occurred 
							 * 1:SMB in Master mode(successful request for bus mastership) */
#define SMBST_NMATCH	(1 << 2)	/* New Match(R/W1C). */
#define SMBST_STASTR	(1 << 3)	/* Stall Atfer Start(R/W1C) */
							/* Set by the successful completion of the sending of an address
							 * (i.e.,a Start condition sent without a bus error or negative
							 * acknowledge) if STASTRE in SMBnCTL1 register is set. */
#define SMBST_NEGACK	(1 << 4)	/* Negative Acknowledge(R/W1C).*/
							/* Set by hardware when a transmission is not acknowledged
							 * on the ninth clock(in this case,SDAST is not set).
							 * After a Stop condition, writing 1 to NEGACK clears it. */
#define SMBST_BER		(1 << 5)	/* Bus Error(R/W1C). */
							/* Set by the hardware either when an invalid Start/Stop condition
							 * is detected(i.g.,during data transfer or acknowledge cycle) or
							 * when an arbitration problem is detected(such as a stalled clock
							 * transaction). Writing 1 to BER clears it. */
#define SMBST_SDAST	(1 << 6)	/* SDA Status(RO). */
							/* When set, indicates that the SDA data register is waiting for
							 * data(Transmit mode - master or slave) or holds data that should
							 * be read(Receive mode - master or slave). This bit is cleared
							 * when reading from SMBnSDA register during a receive or when
							 * written to during a transmit. */
#define SMBST_SLVSTP	(1 << 7)	/* Slave Stop(R/W1C). */
							/* When set, indicates that a Stop condition was detected after
							 * a slave transfer(i.e., after a slave transfer in which MATCH,
							 * ARPMATCH or GCMATCH was set). Writing 1 to SLVSTP clears it. */
#define REG_SMBCST		0x04
#define REG_SMB2CST		0xFFF544
#define SMBCST_BUSY	(1 << 0)	/* Busy (RO) */
							/* when set(1),indicates that the SMB module is one of the
							 * following states:
							 * -Generating a Start condition
							 * -In Master mode(MASTER in SMBnST register is set)
							 * -In Slave mode(MATCH,GMATCH or ARPMATCH in SMBnCST reg is set)
							 * -In the period between detecting a Start condition and
							 *  completing of the address byte; after this, the SMB either
							 *  becomes not busy or enters Slave mode. */
#define SMBCST_BB		(1 << 1)	/* Bus Busy (R/W1C) */
							/* When set(1), indicates the bus is busy. It is set either when
							 * the bus is active(i.g., a low level on either SDA or SCL) or
							 * by a Start condition. It is cleared when the module is disabled
							 * or on detection of a valid Stop condition or by writing 1 to
							 * this bit. */
#define SMBCST_MATCH	(1 << 2)	/* Address Match (RO). */
							/* In Slave mode, set(1) when SAEN in SMBnADDR1 or SMBnADDR2
							 * register is set and the first seven bits of the address byte
							 * (the first byte transferred after a Start condition) match the
							 * 7-bit ADDR field in the respective register.It is cleared by
							 * Start, Repeated Start or Stop condition(including illegal Start
							 * or Stop condition). */
#define SMBCST_GCMATCH	(1 << 3)	/* Global Call Match (RO). */
							/* In Slave mode, set(1) when GCMEN in SMBnCTL1 register is set
							 * and the address byte(the first byte transferred afte a Start
							 * condition) is 00h. It is cleared by a Start Repeated Start or
							 * Stop condition(including illegal Start or Stop condition). */
#define SMBCST_TSDA	(1 << 4)	/* Test SDA Line (RO). */
							/* Reads the current value of the SDA line. This bit can be used
							 * while recovering from an error condition in which the SDA line
							 * is constantly pulled low by a slave that went out of synch. 
							 * Data written to this bit is ignored. */
#define SMBCST_TGSCL	(1 << 5)	/* Toggle SCL line (R/W). */
							/* Enables toggling the SCL line during the process of error
							 * recovery. When the SDA line is low, writing 1 to this bit
							 * toggles the SCL line for one cycle.
							 * TGSCL bit is cleared when the SCL line toggle is completed. */
#define SMBCST_MATCHAF	(1 << 6)	/* Match Address Field (RO). */
							/* When MATCH bit is set, MATCHAF bit indicates which slave
							 * address was matched. MATCHAF is cleared for a match with ADDR
							 * in SMBnADDR1 register and is set for a match with ADDR in
							 * SMBnADDR2 register. */
#define SMBCST_ARPMATCH (1 << 7)	/* ARP Address Match (RO). */
							/* In Slave mode,set(1) when ARPMEN in SMBnCTL3 reg is set and the
							 * address byte(the first byte transferred after a Start condition)
							 * is 110001b. It is cleared by a Start, Repeated Start or Stop
							 * condition(including illegal Start or Stop condition). */
#define REG_SMBCTL1	0x06
#define REG_SMB2CTL1	0xFFF546
#define SMBCTL1_START	(1 << 0) /*Should be set to generate a Start condition on the SMBus.*/
							/* -If the WPCE775x is not the active bus master(MASTER in SMBnST
							 *  register is set to 0), setting START generates a Start
							 *  condition as soon as the Smbus is free(BB in SMBnCST register
							 *  is set to 0). An address transmission sequence should then
							 *  be performed.
							 * -If the WPCE775x is the active master of the bus(MASTER in
							 *  SMBnST register is set to 1), when START is set, a write to
							 *  SMBnSDA register generates a Start condition. SMBnSDA data,
							 *  containing the slave address and the transfer direction, is
							 *  then transmitted. In case of a Repeated Start condition, the
							 *  set bit can be used to switch the direction of the data flow
							 *  between the master and the slave or to choose another slave
							 *  device. without requiring a Stop condition in either case.
							 * This bit is cleared either when the Start condition is sent or
							 * on detection of a bus error(BER in SMBnST register is set to 1).
							 * START should be set only when in Master mode or when requesting
							 * Master mode. */
#define SMBCTL1_STOP (1 << 1) /*In Master mode,setting this bit generates a Stop condition,*/
							/* which completes or aborts the current message transfer. This bit
							 * clears itself after the Stop condition is generated. */
#define SMBCTL1_INTEN	(1 << 2)	/* Interrupt Enable. 0:disable, 1:enable */
#define SMBCTL1_ACK	(1 << 4)	/* Acknowledge. */
							/* When acting as a receiver, holds the value of the next
							 * acknowledge cycle. It should be set when a negative acknowledge
							 * must be issued on the next byte. Cleared(0) after each
							 * acknowledge cycle. It cannot be reset by software. */
#define SMBCTL1_GCMEN	(1 << 5)	/*Global Call Match Enable. */
							/* When set,enables the matching of an incoming address byte to
							 * the general call address(start condition followed by address
							 * byte of 00h) while the SMB is in Slave mode. When cleared,
							 * the SMB does not respond to a global call. */
#define SMBCTL1_NMINTE	(1 << 6)	/* New Match Interrupt Enable. */
							/* When set, enables the interrupt on a new match(i.e., when
							 * NMATCH in SMBnST register is set). The interrupt is issued
							 * only if INTEN in SMBnCTL1 reg is set. */
#define SMBCTL1_STASTRE	(1 << 7)	/* Stall After Start Enable. */
							/* When set(1), enables the Stall After Start mechanism. In this
							 * case,the SMB stalls the bus after the address byte. When STASTRE
							 * is cleared, STASTR bit in SMBnST register cannot be set. */

/**************************************************************/

#if 0
/* access WPCE775x EC register content */
static inline void wrwbec(unsigned long reg, unsigned char val)
{
	delay(10);
	*( (volatile unsigned char *)(0xbfd00000 | WPCE775_HIGH_PORT) ) = (reg & 0xff0000) >> 16;
	*( (volatile unsigned char *)(0xbfd00000 | WPCE775_MID_PORT) ) = (reg & 0x00ff00) >> 8;
	*( (volatile unsigned char *)(0xbfd00000 | WPCE775_LOW_PORT) ) = (reg & 0x00ff);
	*( (volatile unsigned char *)(0xbfd00000 | WPCE775_DATA_PORT) ) = val;
}

static inline unsigned char rdwbec(unsigned long reg)
{
	delay(10);
	*( (volatile unsigned char *)(0xbfd00000 | WPCE775_HIGH_PORT) ) = (reg & 0xff0000) >> 16;
	*( (volatile unsigned char *)(0xbfd00000 | WPCE775_MID_PORT) ) = (reg & 0x00ff00) >> 8;
	*( (volatile unsigned char *)(0xbfd00000 | WPCE775_LOW_PORT) ) = (reg & 0x00ff);
	return (*( (volatile unsigned char *)(0xbfd00000 | WPCE775_DATA_PORT) ));
}
#endif

#define	rdcore(x)		rdwbec(x)
#define	wrcore(x, y)	wrwbec(x, y)
#define	SMB_DELAY	100


#if			1

//static void smbus_stop(void)
static void smbus_stop(unsigned char chnl)
{
	unsigned char value;
	// set the stop bit and clear some flag
	
#if 1
	unsigned long chnl_base_addr;

	chnl_base_addr = SMB_BASE_ADDR + chnl * SMB_CHNL_SIZE;

	value = rdcore(chnl_base_addr + REG_SMBCTL1);
	value |= SMBCTL1_STOP;
	wrcore(chnl_base_addr + REG_SMBCTL1, value);

	wrcore(chnl_base_addr + REG_SMBST, SMBST_BER | SMBST_NEGACK | SMBST_STASTR);
#else
	value = rdcore(REG_SMB2CTL1);
	value |= SMBCTL1_STOP;
	wrcore(REG_SMB2CTL1, value);

	wrcore(REG_SMB2ST, SMBST_BER | SMBST_NEGACK | SMBST_STASTR);
#endif
	return;
}

static void smbus_error(unsigned char chnl)
//static void smbus_error(void)
{
	unsigned char value;
	// set the stop bit and clear some flag
	
#if 1
	unsigned long chnl_base_addr;

	chnl_base_addr = SMB_BASE_ADDR + chnl * SMB_CHNL_SIZE;

	value = rdcore(chnl_base_addr + REG_SMBCTL1);
	value |= SMBCTL1_STOP;
	value &= ~SMBCTL1_STASTRE;
	wrcore(chnl_base_addr + REG_SMBCTL1, value);

	wrcore(chnl_base_addr + REG_SMBST, SMBST_BER | SMBST_NEGACK | SMBST_STASTR);
#else
	value = rdcore(REG_SMB2CTL1);
	value |= SMBCTL1_STOP;
	value &= ~SMBCTL1_STASTRE;
	wrcore(REG_SMB2CTL1, value);

	wrcore(REG_SMB2ST, SMBST_BER | SMBST_NEGACK | SMBST_STASTR);
#endif
	return;
}

static void smbus_start(unsigned char chnl)
//static void smbus_start(void)
{
	unsigned char value;

#if 1
	unsigned long chnl_base_addr;

	chnl_base_addr = SMB_BASE_ADDR + chnl * SMB_CHNL_SIZE;

	/* clear int */
	value = rdcore(chnl_base_addr + REG_SMBCTL1);
	value &= ~(SMBCTL1_NMINTE | SMBCTL1_GCMEN | SMBCTL1_INTEN | SMBCTL1_ACK);
	wrcore(chnl_base_addr + REG_SMBCTL1, value);
	/* clear status bit */
	wrcore(chnl_base_addr + REG_SMBST, SMBST_NMATCH | SMBST_STASTR | SMBST_NEGACK | SMBST_BER | SMBST_SLVSTP);

	value = rdcore(chnl_base_addr + REG_SMBCTL1);
	value |= SMBCTL1_START;
	wrcore(chnl_base_addr + REG_SMBCTL1, value);
	delay(SMB_DELAY);
#else
	// clear int
	value = rdcore(REG_SMB2CTL1);
	value &= ~(SMBCTL1_NMINTE | SMBCTL1_GCMEN | SMBCTL1_INTEN | SMBCTL1_ACK);
	wrcore(REG_SMB2CTL1, value);
	// clear status bit
	wrcore(REG_SMB2ST, SMBST_NMATCH | SMBST_STASTR 
			| SMBST_NEGACK | SMBST_BER | SMBST_SLVSTP);
	
	value = rdcore(REG_SMB2CTL1);
	value |= SMBCTL1_START;
	wrcore(REG_SMB2CTL1, value);
	delay(SMB_DELAY);
#endif
//	printf("start stage : status 0x%x, ctrl 0x%x\n", 
//			rdcore(REG_SMB2ST), rdcore(REG_SMB2CTL1));
	return;
}


static int smbus_addr(unsigned char chnl, unsigned char addr)
//static int smbus_addr(unsigned char addr)
{
	unsigned char value;
	unsigned char status;

#if 1
	unsigned long chnl_base_addr;

	chnl_base_addr = SMB_BASE_ADDR + chnl * SMB_CHNL_SIZE;

	status = rdcore(chnl_base_addr + REG_SMBST);
	if(status & (SMBST_BER + SMBST_NEGACK)){
		printf("addr stage : err 0 : status 0x%x, ctrl 0x%x\n",
				rdcore(chnl_base_addr + REG_SMBST), rdcore(chnl_base_addr + REG_SMBCTL1));
		smbus_error(chnl);
		return -1;
	}else if((status & (SMBST_MASTER + SMBST_SDAST)) == (SMBST_MASTER + SMBST_SDAST)){
		/* enable after send address stall the bus */
		value = rdcore(chnl_base_addr + REG_SMBCTL1);
		value |= SMBCTL1_STASTRE;
		wrcore(chnl_base_addr + REG_SMBCTL1, value);
		delay(SMB_DELAY);

		wrcore(chnl_base_addr + REG_SMBSDA, addr);
		delay(SMB_DELAY);
	}else{
		printf("addr stage : err 1 : status 0x%x, ctrl 0x%x\n",
				rdcore(chnl_base_addr + REG_SMBST), rdcore(chnl_base_addr + REG_SMBCTL1));
		smbus_error(chnl);
		return -1;
	}
#else
	status = rdcore(REG_SMB2ST);
	if(status & (SMBST_BER + SMBST_NEGACK)){
		printf("addr stage : err 0 : status 0x%x, ctrl 0x%x\n", 
			rdcore(REG_SMB2ST), rdcore(REG_SMB2CTL1));
		smbus_error();
		return -1;
	}else if( (status & (SMBST_MASTER + SMBST_SDAST)) == (SMBST_MASTER + SMBST_SDAST) ){
		// enable after send address stall the bus
		value = rdcore(REG_SMB2CTL1);
		value |= SMBCTL1_STASTRE;
		wrcore(REG_SMB2CTL1, value);
		delay(SMB_DELAY);

		wrcore(REG_SMB2SDA, addr);
		delay(SMB_DELAY);
	}else{
		printf("addr stage : err 1 : status 0x%x, ctrl 0x%x\n", 
			rdcore(REG_SMB2ST), rdcore(REG_SMB2CTL1));
		smbus_error();
		return -1;
	}
#endif
	//printf("addr stage : status 0x%x, ctrl 0x%x\n", 
	//		rdcore(REG_SMB2ST), rdcore(REG_SMB2CTL1));
	delay(SMB_DELAY);

	return 0;
}

static int smbus_send(unsigned char chnl, unsigned char index)
//static int smbus_send(unsigned char index)
{
	unsigned char status;
	unsigned char value;

#if 1
	unsigned long chnl_base_addr;

	chnl_base_addr = SMB_BASE_ADDR + chnl * SMB_CHNL_SIZE;

	status = rdcore(chnl_base_addr + REG_SMBST);
	if(status & (SMBST_BER + SMBST_NEGACK)){
		printf("send stage : err 1 : status 0x%x, ctrl 0x%x\n", 
			rdcore(chnl_base_addr + REG_SMBST), rdcore(chnl_base_addr + REG_SMBCTL1));
		smbus_error(chnl);
		return -1;
	}else if(status & SMBST_STASTR){
		// start send cmd or data
		if(rdcore(chnl_base_addr + REG_SMBCTL1) & SMBCTL1_STASTRE)
			wrcore(chnl_base_addr + REG_SMBST, SMBST_STASTR);
		delay(SMB_DELAY);
		wrcore(chnl_base_addr + REG_SMBSDA, index);		
		delay(SMB_DELAY);
	}else{
		printf("send stage : err 2 : status 0x%x, ctrl 0x%x\n", 
			rdcore(chnl_base_addr + REG_SMBST), rdcore(chnl_base_addr + REG_SMBCTL1));
		smbus_error(chnl);
		return -1;
	}
		
//	printf("send stage : status 0x%x, ctrl 0x%x\n", 
//			rdcore(REG_SMB2ST), rdcore(REG_SMB2CTL1));

	/////////////////////////// stop internal stage
	
	delay(SMB_DELAY);
	status = rdcore(chnl_base_addr + REG_SMBST);
	if(status & (SMBST_BER + SMBST_NEGACK)){
		printf("send stop stage : err 1 : status 0x%x, ctrl 0x%x\n", 
			rdcore(chnl_base_addr + REG_SMBST), rdcore(chnl_base_addr + REG_SMBCTL1));
		smbus_error(chnl);
		return -1;
	}else if(status & SMBST_SDAST){
		smbus_stop(chnl);
	}else{
		printf("send stop stage : err 2 : status 0x%x, ctrl 0x%x\n", 
			rdcore(chnl_base_addr + REG_SMBST), rdcore(chnl_base_addr + REG_SMBCTL1));
		smbus_error(chnl);
		return -1;
	}
#else
	status = rdcore(REG_SMB2ST);
	if(status & (SMBST_BER + SMBST_NEGACK)){
		printf("send stage : err 1 : status 0x%x, ctrl 0x%x\n", 
			rdcore(REG_SMB2ST), rdcore(REG_SMB2CTL1));
		smbus_error();
		return -1;
	}else if(status & SMBST_STASTR){
		// start send cmd or data
		if(rdcore(REG_SMB2CTL1) & SMBCTL1_STASTRE)
			wrcore(REG_SMB2ST, SMBST_STASTR);
		delay(SMB_DELAY);
		wrcore(REG_SMB2SDA, index);		
		delay(SMB_DELAY);
	}else{
		printf("send stage : err 2 : status 0x%x, ctrl 0x%x\n", 
			rdcore(REG_SMB2ST), rdcore(REG_SMB2CTL1));
		smbus_error();
		return -1;
	}
		
//	printf("send stage : status 0x%x, ctrl 0x%x\n", 
//			rdcore(REG_SMB2ST), rdcore(REG_SMB2CTL1));

	/////////////////////////// stop internal stage
	
	delay(SMB_DELAY);
	status = rdcore(REG_SMB2ST);
	if(status & (SMBST_BER + SMBST_NEGACK)){
		printf("send stop stage : err 1 : status 0x%x, ctrl 0x%x\n", 
			rdcore(REG_SMB2ST), rdcore(REG_SMB2CTL1));
		smbus_error();
		return -1;
	}else if(status & SMBST_SDAST){
		smbus_stop();
	}else{
		printf("send stop stage : err 2 : status 0x%x, ctrl 0x%x\n", 
			rdcore(REG_SMB2ST), rdcore(REG_SMB2CTL1));
		smbus_error();
		return -1;
	}
#endif
//	printf("send stop stage : status 0x%x, ctrl 0x%x\n", 
//			rdcore(REG_SMB2ST), rdcore(REG_SMB2CTL1));

	return 0;
}

static int smbus_recv(unsigned char chnl, unsigned char *val)
//static int smbus_recv(unsigned char *val)
{
	unsigned char status, value;

#if 1
	unsigned long chnl_base_addr;

	chnl_base_addr = SMB_BASE_ADDR + chnl * SMB_CHNL_SIZE;

	status = rdcore(chnl_base_addr + REG_SMBST);
	if(status & (SMBST_BER + SMBST_NEGACK)){
		printf("recv stage : err 1 : status 0x%x, ctrl 0x%x\n", 
			rdcore(chnl_base_addr + REG_SMBST), rdcore(chnl_base_addr + REG_SMBCTL1));
		smbus_error(chnl);
		return -1;
	}else if(status & SMBST_STASTR){
		// have send out address, and recv now
		if(rdcore(chnl_base_addr + REG_SMBCTL1) & SMBCTL1_STASTRE)
			wrcore(chnl_base_addr + REG_SMBST, SMBST_STASTR);	
		delay(SMB_DELAY);
		value = rdcore(chnl_base_addr + REG_SMBCTL1);
		value &= ~SMBCTL1_STASTRE;		// ??? now STASTR AGAIN, maybe no need
		value |= SMBCTL1_ACK;
		wrcore(chnl_base_addr + REG_SMBCTL1, value);
		delay(SMB_DELAY);
		//delay(1000);
	}else{
		printf("recv stage : err 2 : status 0x%x, ctrl 0x%x\n", 
			rdcore(chnl_base_addr + REG_SMBST), rdcore(chnl_base_addr + REG_SMBCTL1));
		smbus_error(chnl);
		return -1;
	}
	

	//////////////////////////////// stop internal stage
	delay(SMB_DELAY);
	status = rdcore(chnl_base_addr + REG_SMBST);
//	printf("recv stage : status 0x%x, ctrl 0x%x\n", 
//			rdcore(REG_SMB2ST), rdcore(REG_SMB2CTL1));

	if(status & SMBST_SDAST){
		// issue stop and get data
		value = rdcore(chnl_base_addr + REG_SMBCTL1);
		value |= SMBCTL1_STOP;
		wrcore(chnl_base_addr + REG_SMBCTL1, value);
		delay(SMB_DELAY);

		*val = rdcore(chnl_base_addr + REG_SMBSDA);
	}else{
		printf("recv stop stage : err 1 : status 0x%x, ctrl 0x%x\n", 
			rdcore(chnl_base_addr + REG_SMBST), rdcore(chnl_base_addr + REG_SMBCTL1));
		smbus_error(chnl);
		return -1;
	}

#else	
	status = rdcore(REG_SMB2ST);
	if(status & (SMBST_BER + SMBST_NEGACK)){
		printf("recv stage : err 1 : status 0x%x, ctrl 0x%x\n", 
			rdcore(REG_SMB2ST), rdcore(REG_SMB2CTL1));
		smbus_error();
		return -1;
	}else if(status & SMBST_STASTR){
		// have send out address, and recv now
		if(rdcore(REG_SMB2CTL1) & SMBCTL1_STASTRE)
			wrcore(REG_SMB2ST, SMBST_STASTR);	
		delay(SMB_DELAY);
		value = rdcore(REG_SMB2CTL1);
		value &= ~SMBCTL1_STASTRE;		// ??? now STASTR AGAIN, maybe no need
		value |= SMBCTL1_ACK;
		wrcore(REG_SMB2CTL1, value);
		delay(SMB_DELAY);
		//delay(1000);
	}else{
		printf("recv stage : err 2 : status 0x%x, ctrl 0x%x\n", 
			rdcore(REG_SMB2ST), rdcore(REG_SMB2CTL1));
		smbus_error();
		return -1;
	}
	

	//////////////////////////////// stop internal stage
	delay(SMB_DELAY);
	status = rdcore(REG_SMB2ST);
//	printf("recv stage : status 0x%x, ctrl 0x%x\n", 
//			rdcore(REG_SMB2ST), rdcore(REG_SMB2CTL1));

	if(status & SMBST_SDAST){
		// issue stop and get data
		value = rdcore(REG_SMB2CTL1);
		value |= SMBCTL1_STOP;
		wrcore(REG_SMB2CTL1, value);
		delay(SMB_DELAY);

		*val = rdcore(REG_SMB2SDA);
	}else{
		printf("recv stop stage : err 1 : status 0x%x, ctrl 0x%x\n", 
			rdcore(REG_SMB2ST), rdcore(REG_SMB2CTL1));
		smbus_error();
		return -1;
	}
#endif	
//	printf("recv stop stage : status 0x%x, ctrl 0x%x\n", 
//			rdcore(REG_SMB2ST), rdcore(REG_SMB2CTL1));
	return 0;
}

static int cmd_rdsmbus(int ac, char *av[])
{
    unsigned long addr;
    unsigned long index;
	unsigned char data;
	unsigned char chnl;
	int ret = 0;

	if(ac < 3){
	    printf("usage : rdsmbus addr index\n");
	    return -1;
	}
#if 1
	if(!get_rsa(&chnl, av[1])) {
	    printf("ecreg : access error!\n");
	    return -1;
	}
#endif
	
	if(!get_rsa(&addr, av[2])) {
	//if(!get_rsa(&addr, av[1])) {
	    printf("ecreg : access error!\n");
	    return -1;
	}
	
	if(!get_rsa(&index, av[3])){
	//if(!get_rsa(&index, av[2])){
	    printf("ecreg : size error\n");
		return -1;
	}

	printf("channel SMB%d, addr 0x%x, index 0x%x\n", chnl + 1, addr, index, data);
	//printf("addr 0x%x, index 0x%x\n", addr, index, data);
	smbus_start(chnl);
	ret = smbus_addr(chnl, addr & 0xfe);
	//smbus_start();
	//ret = smbus_addr(addr & 0xfe);
	if(ret < 0){
		return ret;
	}
	ret = smbus_send(chnl, index);
	//ret = smbus_send(index);
	if(ret < 0){
		return ret;
	}
	
	smbus_start(chnl);
	//smbus_start();
	
	ret = smbus_addr(chnl, addr | 0x01);
	//ret = smbus_addr(addr | 0x01);
	if(ret < 0){
		return ret;
	}
	ret = smbus_recv(chnl, &data);
	//ret = smbus_recv(&data);
	if(ret < 0){
		return ret;
	}
	printf("final : channel SMB%d, addr 0x%x, index 0x%x, data 0x%x\n", chnl, addr, index, data);
	//printf("final : addr 0x%x, index 0x%x, data 0x%x\n", addr, index, data);

	return 0;
}

#endif



/*
 * wr775reg : write a WPEC775x EC register area.
 */
static int cmd_wr775reg(int ac, char *av[])
{
    u32 addr;
    u32 value;
    u8 val8;

	if(ac < 3){
	    printf("usage : wrecreg addr val\n");
	    return -1;
	}
	 
	if(!get_rsa(&addr, av[1])) {
	//addr = strtoul(av[1], NULL, 16);
	//if(!addr) {
	    printf("ecreg : access error!\n");
	    return -1;
	}
	
	if(!get_rsa(&value, av[2])){
	//value = strtoul(av[2], NULL, 16);
	//if(!value){
	    printf("ecreg : size error\n");
		return -1;
	}
	 
	val8 = (u8)value;
	
	if((addr > 0x1000000)){
	    printf("ecreg : addr not available.\n");
	}
	
	printf("ecreg : addr=0x%x value=0x%x\n", addr , val8);
	wrwbec(addr, val8);
	
	return 0;
}

/*
 * WPCE775_rdecreg : read a WPEC775x EC register area.
 */
static int cmd_rd775reg(int ac, char *av[])
{
	u32 start, size, reg;
	u8 value;

	size = 0;

	if(ac < 2){
		printf("usage : rdecreg start_addr size\n");
		return -1;
	}

	//start = strtoul(av[1], NULL, 16);
	//size = strtoul(av[2], NULL, 16);

	if(!get_rsa(&start, av[1])) {
	//if(!start) {
		printf("ecreg : access error!\n");
		return -1;
	}

//	if(!get_rsa(&size, av[2])){
//	//if(!size){
//		printf("ecreg : size error\n");
//		return -1;
//	}

//	if((start > 0x1000000) || (size > 0x1000000)){
//		printf("ecreg : start not available.\n");
//	}

//	printf("ecreg : reg=0x%x, size=0x%x\n", start, size);
	reg = start;
//	while(size > 0){
		//printf("reg address : 0x%x, value1 : 0x%x, value2 : %c\n", reg, WPCE775_rdec(reg), WPCE775_rdec(reg));
		printf("reg address : 0x%x, value : 0x%x\n", reg, rdwbec(reg));
//		reg++;
//		size--;
//	}

	return 0;
}
#endif	//end ifdef 775_DEBUG


#define	RD_REG_DELAY	30000
static int cmd_wr775(int ac, char *av[])
{
	//u8 i;
	u8 cmd;
	u8 value = 0;

	//printf("wr775 command, ac = %d\n", ac);
	if(ac < 2){
		printf("usage : rd775 cmd index\n");
		return -1;
	}
	//if(ac > 3){
	//	if(!get_rsa(&data, av[3])) {
	//		printf("rd775 : access error!\n");
	//		return -1;
	//	}
	//}
	cmd = strtoul(av[1], NULL, 16);
	value = strtoul(av[2], NULL, 16);

	printf("Write EC 775 : command 0x%x, data 0x%x \n", cmd, value);

	/* Write index value to EC 775 space */
	*((volatile unsigned char *)(0xbfd00066)) = cmd;
	delay(RD_REG_DELAY);
	*((volatile unsigned char *)(0xbfd00062)) = value;
	delay(RD_REG_DELAY);

	//val = *((volatile unsigned char *)(0xbfd00062));

	//printf("Read EC 775 : index 0x%x, value 0x%x %d\n", index, val, val);
	return 0;
} 

/*
 * rd775 : read a space of EC 775.
 * usage : rd775 cmd index (hex)
 *  e.g. : rd775 41 0
 * return: value (hex, dec)
 * daway added 2010-01-13
 */
static int cmd_rd775(int ac, char *av[])
{
	u8 i;
	u8 cmd, index;
	u8 size = 1, val = 0;
	
//	printf("rd775 command, ac = %d\n", ac);
	if(ac < 2){
		printf("usage : rd775 cmd index\n");
		return -1;
	}
	if(ac > 3){
		if(!get_rsa(&size, av[3])) {
			printf("rd775 : access error!\n");
			return -1;
		}
	}
	
	cmd = strtoul(av[1], NULL, 16);
	index = strtoul(av[2], NULL, 16);

	printf("Read EC 775 : command 0x%x, index 0x%x, size %d \n", cmd, index, size);

	for(i = 0; i < size; i++){
		/* Read EC 775 space value */
		*((volatile unsigned char *)(0xbfd00066)) = cmd;
		delay(RD_REG_DELAY);
		*((volatile unsigned char *)(0xbfd00062)) = index;
		delay(RD_REG_DELAY);

		val = *((volatile unsigned char *)(0xbfd00062));

		printf("Read EC 775 : index 0x%x, value 0x%x %d\n", index, val, val);
		index++;
	}
    #if 0

    val16 |= val;

    *((unsigned char *)(0xbfd00066)) = 0x54;
    delay(1000);
    *((unsigned char *)(0xbfd00062)) = 0x02;
    
    val = *((unsigned char *)(0xbfd00062));
    val16 |=(val<<8);

    printf("fan speed:%x %d\n",val16,val16);



	for(i = 0; i < SMBDAT_SIZE; i++){
		wrec(REG_SMBDAT_START + i, 0x00);
	}
	printf("battery : index 0x%x \t", index);
	wrec(REG_SMBSTS, 0x00);
	wrec(REG_SMBCNT, 0x01);
	val = rdec(REG_SMBPIN);
	val = (val & 0xfc) | (1 << 0);
	wrec(REG_SMBPIN, val);
	wrec(REG_SMBADR, 0x6c|0x01);
	wrec(REG_SMBCMD, index);
	wrec(REG_SMBPRTCL, 0x07);
	while(!(rdec(REG_SMBSTS) & (1 << 7)));

	val = rdec(REG_SMBDAT_START);
	printf("value 0x%x\n", val);
    #endif
	return 0;
}



/*
 * rdbat : read a register of battery.
 */
static int cmd_rdbat(int ac, char *av[])
{
	//u8 i;
	//u8 index;
	u8 val;
	
	/* Read CPU Current temperature */
#if 1
    *((volatile unsigned char *)(0xbfd00066)) = 0x80;
    delay(RD_REG_DELAY);
    *((volatile unsigned char *)(0xbfd00062)) = 0x1B;
    delay(RD_REG_DELAY);
#else
    *((volatile unsigned char *)(0xbfd00066)) = 0x54;
    delay(RD_REG_DELAY);
    *((volatile unsigned char *)(0xbfd00062)) = 0x0;
    delay(RD_REG_DELAY);
#endif
    val = *((volatile unsigned char *)(0xbfd00062));

    printf("CPU current temperature: 0x%x %d\n",val,val);
    #if 0

    val16 |= val;

    *((unsigned char *)(0xbfd00066)) = 0x54;
    delay(1000);
    *((unsigned char *)(0xbfd00062)) = 0x02;
    
    val = *((unsigned char *)(0xbfd00062));
    val16 |=(val<<8);

    printf("fan speed:%x %d\n",val16,val16);


	if(ac < 2){
		printf("usage : rdbat %index\n");
		return -1;
	}

	if(!get_rsa(&index, av[1])) {
		printf("rdbat : access error!\n");
		return -1;
	}

	for(i = 0; i < SMBDAT_SIZE; i++){
		wrec(REG_SMBDAT_START + i, 0x00);
	}
	printf("battery : index 0x%x \t", index);
	wrec(REG_SMBSTS, 0x00);
	wrec(REG_SMBCNT, 0x01);
	val = rdec(REG_SMBPIN);
	val = (val & 0xfc) | (1 << 0);
	wrec(REG_SMBPIN, val);
	wrec(REG_SMBADR, 0x6c|0x01);
	wrec(REG_SMBCMD, index);
	wrec(REG_SMBPRTCL, 0x07);
	while(!(rdec(REG_SMBSTS) & (1 << 7)));

	val = rdec(REG_SMBDAT_START);
	printf("value 0x%x\n", val);
    #endif
	return 0;
}

/* 
 * rdfan : read a register of temperature IC. 
 */
static int cmd_rdfan(int ac, char *av[])
{
	//u8 index;
	u8 val = 0;
    u16 val16 = 0;
	//u8 i;

    //wrec();
    *((volatile unsigned char *)(0xbfd00066)) = 0x80;
    delay(RD_REG_DELAY);
    *((volatile unsigned char *)(0xbfd00062)) = 0x08;

    delay(RD_REG_DELAY);
    val = *((volatile unsigned char *)(0xbfd00062));
    val16 |= val;

    delay(RD_REG_DELAY);
    *((volatile unsigned char *)(0xbfd00066)) = 0x80;
    delay(RD_REG_DELAY);
    *((volatile unsigned char *)(0xbfd00062)) = 0x09;
    
    delay(RD_REG_DELAY);
    val = *((volatile unsigned char *)(0xbfd00062));
    val16 |= (val << 8);

    printf("fan speed: 0x%x %d\n", val16, val16);
    #if 0
	wrec(REG_SMBCFG, rdec(REG_SMBCFG) & (~0x1f) | 0x04);
	if(ac < 2){
		printf("usage : rdfan %index\n");
		return -1;
	}

	if(!get_rsa(&index, av[1])) {
		printf("rdfan : access error!\n");
		return -1;
	}

	for(i = 0; i < SMBDAT_SIZE; i++){
		wrec(REG_SMBDAT_START + i, 0x00);
	}
	printf("fan : index 0x%x \t", index);
	wrec(REG_SMBSTS, 0x00);
	wrec(REG_SMBCNT, 0x02);
	val = rdec(REG_SMBPIN);
	val = (val & 0xfc) | (1 << 1);
	wrec(REG_SMBPIN, val);
	wrec(REG_SMBADR, 0x90|0x01);
	wrec(REG_SMBCMD, index);
	wrec(REG_SMBPRTCL, 0x09);
	while(!(rdec(REG_SMBSTS) & (1 << 7)));

	val = rdec(REG_SMBDAT_START);
	printf("value 0x%x\t", val);
	val = rdec(REG_SMBDAT_START + 1);
	printf("value2 0x%x\n", val);
    #endif

	return 0;
}

static inline int ec_flash_busy(void)
{
	unsigned char count = 0;

	while(count < 10){
		wrec(XBI_BANK | XBISPICMD, 5);
		while( (rdec(XBI_BANK | XBISPICFG)) & (1 << 1) );
		if( (rdec(XBI_BANK | XBISPIDAT) & (1 << 0)) == 0x00 ){
			return 0x00;
		}
		count++;
	}

	return 0x01;
}

extern void delay(int us);
extern void ec_init_idle_mode(void);
extern void ec_exit_idle_mode(void);
extern void ec_get_product_id(void);

#ifndef NO_ROM_ID_NEEDED
extern unsigned char ec_rom_id[3];
static int cmd_readspiid(int ac, char *av[])
{
	u8 cmd;

	if(!get_rsa(&cmd, av[1])) {
		printf("xbi : access error!\n");
		return -1;
	}
	printf("read spi id command : 0x%x\n", cmd);


	/* goto idle mode */
	ec_init_idle_mode();

	/* get product id */
	ec_get_product_id();

	/* out of idle mode */
	ec_exit_idle_mode();
	
	printf("Manufacture ID 0x%x, Device ID : 0x%x 0x%x\n", ec_rom_id[0], ec_rom_id[1], ec_rom_id[2]);
	return 0;
}
#endif

/*
 * xbird : read an EC rom address data througth XBI interface.
 */
#ifndef NO_ROM_ID_NEEDED
extern unsigned char ec_rom_id[3];
#endif
static int cmd_xbird(int ac, char *av[])
{
	u8 val;
	u32 addr;
	u8 data;
	int timeout;

	if(ac < 2){
		printf("usage : xbird start_address\n");
		return -1;
	}

	if(!get_rsa(&addr, av[1])) {
		printf("xbi : access error!\n");
		return -1;
	}

	/* enable spicmd writing. */
	val = rdec(XBI_BANK | XBISPICFG);
	wrec(XBI_BANK | XBISPICFG, val | (1 << 3) | (1 << 0));

	/* check is it busy. */
	if(ec_flash_busy()){
		printf("xbi : flash busy 1.\n");
		return -1;
	}

	/* enable write spi flash */
	wrec(XBI_BANK | XBISPICMD, 6);

	/* write the address */
	wrec(XBI_BANK | XBISPIA2, (addr & 0xff0000) >> 16);
	wrec(XBI_BANK | XBISPIA1, (addr & 0x00ff00) >> 8);
	wrec(XBI_BANK | XBISPIA0, (addr & 0x0000ff) >> 0);

	/* start action */
#ifndef  NO_ROM_ID_NEEDED
	switch(ec_rom_id[0]){
		case EC_ROM_PRODUCT_ID_SPANSION :
			wrec(XBI_BANK | XBISPICMD, 0x03);
			break;
		case EC_ROM_PRODUCT_ID_MXIC :
			wrec(XBI_BANK | XBISPICMD, 0x0b);
			break;
		case EC_ROM_PRODUCT_ID_AMIC :
			wrec(XBI_BANK | XBISPICMD, 0x0b);
			break;
		case EC_ROM_PRODUCT_ID_EONIC :
			wrec(XBI_BANK | XBISPICMD, 0x0b);
			break;
		default :
			printf("ec rom type not supported.\n");
			return -1;
	}
#else
    wrec(XBI_BANK | XBISPICMD, 0x0b);
#endif

	timeout = 0x10000;
	while(timeout-- >= 0){
		if( !(rdec(XBI_BANK | XBISPICFG) & (1 << 1)) )
				break;
	}
	if(timeout <= 0){
		printf("xbi : read timeout.\n");
		return -1;
	}

	/* get data */
	data = rdec(XBI_BANK | XBISPIDAT);
	printf("ec : addr 0x%x\t, data 0x%x, %c\n", addr, data, data);

	val = rdec(XBI_BANK | XBISPICFG) & (~((1 << 3) | (1 << 0)));
	wrec(XBI_BANK | XBISPICFG, val);

	return 0;
}

/*
 * xbiwr : write a byte to EC flash.
 */
static int cmd_xbiwr(int ac, char *av[])
{
	u8 val;
	u8 cmd;
	u32 addr;
	u8 data;
	int timeout;

	if(ac < 3){
		printf("usage : xbiwr start_address val\n");
		return -1;
	}

	if(!get_rsa(&addr, av[1])) {
		printf("xbi : access error!\n");
		return -1;
	}
	if(!get_rsa(&data, av[2])) {
		printf("xbi : access error!\n");
		return -1;
	}

	/* enable spicmd writing. */
	val = rdec(XBI_BANK | XBISPICFG);
	wrec(XBI_BANK | XBISPICFG, val | (1 << 3) | (1 << 0));
	/* enable write spi flash */
	wrec(XBI_BANK | XBISPICMD, 6);
	timeout = 0x1000;
	while(timeout-- >= 0){
		if( !(rdec(XBI_BANK | XBISPICFG) & (1 << 1)) )
				break;
	}
	if(timeout <= 0){
		printf("xbi : write timeout 1.\n");
		return -1;
	}

	/* write the address and data */
	wrec(XBI_BANK | XBISPIA2, (addr & 0xff0000) >> 16);
	wrec(XBI_BANK | XBISPIA1, (addr & 0x00ff00) >> 8);
	wrec(XBI_BANK | XBISPIA0, (addr & 0x0000ff) >> 0);
	wrec(XBI_BANK | XBISPIDAT, data);
	/* start action */
	wrec(XBI_BANK | XBISPICMD, 2);
	timeout = 0x1000;
	while(timeout-- >= 0){
		if( !(rdec(XBI_BANK | XBISPICFG) & (1 << 1)) )
				break;
	}
	if(timeout <= 0){
		printf("xbi : read timeout.\n");
		return -1;
	}
	val = rdec(XBI_BANK | XBISPICFG) & (~((1 << 3) | (1 << 0)));
	wrec(XBI_BANK | XBISPICFG, val);

	return 0;
}

int cmd_testvideo(int ac, char *av[])
{
	unsigned long addr;
	if(ac < 2)
			return -1;
	addr = strtoul(av[1], NULL, 16);
	printf("addr = 0x%x", addr);
	video_display_bitmap(addr, 280, 370);
	return 1;
}

static const Cmd Cmds[] =
{
	{"cs5536 debug"},
	{"powerdebug", "reg", NULL, "for debug the power state of sm712 graphic", cmd_powerdebug, 2, 99, CMD_REPEAT},
	{"rdmsr", "reg", NULL, "msr read test", cmd_rdmsr, 2, 99, CMD_REPEAT},
	{"wrmsr", "reg", NULL, "msr write test", cmd_wrmsr, 2, 99, CMD_REPEAT},
	
	{"rdecreg", "reg", NULL, "KB3310 EC reg read test", cmd_rdecreg, 2, 99, CMD_REPEAT},
	{"wrecreg", "reg", NULL, "KB3310 EC reg write test", cmd_wrecreg, 2, 99, CMD_REPEAT},
//	{"readspiid", "reg", NULL, "KB3310 read spi device id test", cmd_readspiid, 2, 99, CMD_REPEAT},
	{"wri", "reg", NULL, "WPEC775x EC reg read test", cmd_wr775reg, 2, 99, CMD_REPEAT},
	{"rdi", "reg", NULL, "WPEC775x EC reg read test", cmd_rd775reg, 2, 99, CMD_REPEAT},
	{"rdsmbus", "reg", NULL, "WPEC775x EC reg read test", cmd_rdsmbus, 2, 99, CMD_REPEAT},
	{"wr775", "reg", NULL, "WPCE775L Space read test", cmd_wr775, 2, 99, CMD_REPEAT},
	{"rd775", "reg", NULL, "WPCE775L Space read test", cmd_rd775, 2, 99, CMD_REPEAT},
	{"rdbat", "reg", NULL, "KB3310 smbus battery reg read test", cmd_rdbat, 0, 99, CMD_REPEAT},
	{"rdfan", "reg", NULL, "KB3310 smbus fan reg read test", cmd_rdfan, 0, 99, CMD_REPEAT},
	{"xbiwr", "reg", NULL, "for debug write data to xbi interface of ec", cmd_xbiwr, 2, 99, CMD_REPEAT},
	{"xbird", "reg", NULL, "for debug read data from xbi interface of ec", cmd_xbird, 2, 99, CMD_REPEAT},
	{"testvideo", "reg", NULL, "for debug read data from xbi interface of ec", cmd_testvideo, 2, 99, CMD_REPEAT},
	{0},
};


static void init_cmd __P((void)) __attribute__ ((constructor));

static void
init_cmd()
{
	cmdlist_expand(Cmds, 1);
}

