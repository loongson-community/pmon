/*
 * WPCE775x.h
 *	
 *	Author : huangwei <huangw@lemote.com>
 *	Date   : 2010-03-11
 */

#include <linux/io.h>
#include "include/shm.h"

/* base address for io access */
#undef	MIPS_IO_BASE
#define	MIPS_IO_BASE	(0xbfd00000)
#define SIO_INDEX_PORT	0x2E
#define SIO_DATA_PORT	0x2F

/* EC access port for sci communication */
#define	EC_CMD_PORT			0x66
#define	EC_STS_PORT			0x66
#define	EC_DATA_PORT		0x62

#ifdef	LOONGSON2F_3GNB
/* We should make ec rom content and pmon port number equal. */
#define	WPCE775_HIGH_PORT	0xFF1E
#define WPCE775_MID_PORT	0xFF1D
#define	WPCE775_LOW_PORT	0xFF1C
#define	WPCE775_DATA_PORT	0xFF1F
#define	RD_REG_DELAY		30000
#define CMD_RD_VER			0x4F
#define VER_START_INDEX		0
#define	OEMVER_MAX_SIZE		36 

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

#define	rdcore(x)		rdwbec(x)
#define	wrcore(x, y)	wrwbec(x, y)
#define	SMB_DELAY	100

/* delay function */
extern void delay(int microseconds);
/* version array */
extern unsigned char ec_ver[OEMVER_MAX_SIZE];
extern unsigned char *get_ecver(void);

/* access WPCE775x EC register content */
static inline void wrwbec(unsigned long reg, unsigned char val)
{
	delay(10);
	*( (volatile unsigned char *)(MIPS_IO_BASE | WPCE775_HIGH_PORT) ) = (reg & 0xff0000) >> 16;
	*( (volatile unsigned char *)(MIPS_IO_BASE | WPCE775_MID_PORT) ) = (reg & 0x00ff00) >> 8;
	*( (volatile unsigned char *)(MIPS_IO_BASE | WPCE775_LOW_PORT) ) = (reg & 0x00ff);
	*( (volatile unsigned char *)(MIPS_IO_BASE | WPCE775_DATA_PORT) ) = val;
}

static inline unsigned char rdwbec(unsigned long reg)
{
	delay(10);
	*( (volatile unsigned char *)(MIPS_IO_BASE | WPCE775_HIGH_PORT) ) = (reg & 0xff0000) >> 16;
	*( (volatile unsigned char *)(MIPS_IO_BASE | WPCE775_MID_PORT) ) = (reg & 0x00ff00) >> 8;
	*( (volatile unsigned char *)(MIPS_IO_BASE | WPCE775_LOW_PORT) ) = (reg & 0x00ff);
	return (*( (volatile unsigned char *)(MIPS_IO_BASE | WPCE775_DATA_PORT) ));
}

/* 07h - 2Fh: SuperI/O Control and Configuration Registers */
/* Standard Control Registers */
#define REG_SLDN			0x7		/* Logical Device Number */
#define REG_SID				0x20	/* SuperI/O ID */
#define REG_SIOCF1			0x21	/* SuperI/O Configuration 1 */
#define CF1_GLOBEN		(1 << 0)	/* Global Device Enable. */
#define REG_SIOCF5			0x25    /* SuperI/O Configuration 5 */
#define CF5_SMIIRQ2_EN	(1 << 4) /* SMI to IRQ2 Enable. */
#define REG_SIOCF6			0x26    /* SuperI/O Configuration 6 */
#define REG_SRID			0x27	/* SuperI/O Revision ID */
#define REG_SIOGPS			0x28	/* SuperI/O General-Purpose Scratch */
#define REG_SIOCF9   		0x29    /* SuperI/O Configuration 9 */
#define REG_SIOCFD   		0x2D    /* SuperI/O Configuration D */

/* 30h - FFh: Logical Device Control and Configuration Registers -
 * one per Logical Device(some are optional) */
/* Logical Device Activate Register */
#define REG_ACTIVATE		0x30 	/* Logical Device Control (Activate) */
/* I/O Space Configuration Registers */
#define REG_BADDR0_H		0x60	/* I/O Base Address Descriptor 0 Bits 15-8 */
#define REG_BADDR0_L		0x61	/* I/O Base Address Descriptor 0 Bits 7-0 */
#define REG_BADDR1_H		0x62	/* I/O Base Address Descriptor 1 Bits 15-8 */
#define REG_BADDR1_L		0x63	/* I/O Base Address Descriptor 1 Bits 7-0 */
/* Interrupt Configuration Registers */
#define REG_INT_CONFIG  	0x70	/* Interrupt Number and Wake-Up on IRQ Enable */
#define REG_IRQSEL			0x71	/* IRQ Type Select */
/* DMA Configuration Registers */
#define REG_DMA_CHNSEL0		0x74	/* DMA Channel Select 0 */
#define REG_DMA_CHNSEL1		0x75	/* DMA Channel Select 1 */
/* F0h - FFh: Specific Logical Device Configuration */
/* Shared Memory Configuration Registers */
#define SHM_CFG				0xF0	/* Shared Memory Configuration register. */
#define WIN_CFG				0xF1	/* Shared Access Windows Configuration register. */
#define SHAW1BA_0			0xF4	/* Shared Access Window 1, Base Address 0 register. */
#define SHAW1BA_1			0xF5	/* Shared Access Window 1, Base Address 1 register. */
#define SHAW1BA_2			0xF6	/* Shared Access Window 1, Base Address 2 register. */
#define SHAW1BA_3			0xF7	/* Shared Access Window 1, Base Address 3 register. */
#define SHAW2BA_0			0xF8	/* Shared Access Window 2, Base Address 0 register. */
#define SHAW2BA_1			0xF9	/* Shared Access Window 2, Base Address 1 register. */
#define SHAW2BA_2			0xFA	/* Shared Access Window 2, Base Address 2 register. */
#define SHAW2BA_3			0xFB	/* Shared Access Window 2, Base Address 3 register. */

// Shared Memory Registers' bits
#define SHW_FWH_ID_POS      4
#define FLASH_ACC_EN        0x4
#define SHWIN_ACC_FWH       0x40
#define BIOS_LPC_EN         0x1
#define BIOS_FWH_EN         0x8

#define FWIN1_SIZE_MASK     0xF
#define FWIN1_SIZE_512_KB   0x9         // In prototype, for FWH this is actually only 64 Kbyte
#define FWIN1_SIZE_1_MB     0xA         // In prototype, for FWH this is actually only 64 Kbyte
#define FWIN1_SIZE_2_MB     0xB         // In prototype, for FWH this is actually only 128 Kbyte
#define FWIN1_SIZE_4_MB     0xC         // In prototype, for FWH this is actually only 256 Kbyte

/* Logical Device Number (LDN) Assignments */
#define LDN_MSWC            0x4		/* Mobile System Wake-Up Control (MSWC) */
#define LDN_MOUSE			0x5 	/* Keyboard and Mouse Controller (KBC) - Mouse Interface */
#define LDN_KBD    			0x6 	/* Keyboard and Mouse Controller (KBC) - Keyboard Interface */
#define LDN_HGPIO           0x7 	/*  */
#define LDN_SHM             0xF 	/* Shared Memory (SHM) */
#define LDN_PM1             0x11	/* Power Management I/F Channel 1 (PM1) */
#define LDN_PM2             0x12	/* Power Management I/F Channel 2 (PM2) */
#define LDN_PM3             0x17	/* Power Management I/F Channel 3 (PM3) */
#define DOCKING_LPC_LDN     0x19

/* SuperI/O Values */
#define LDN_SHM             0xF		/* LDN of Shared Memory logical device */
#define SID_WPCE775l		0xFC	/* Identity Number of the device family */
#define CHIPID_WPCE775L	(0xA << 5)	/* Chip ID. Identifies a specific device of a family. */
 
/* Logical Device values */
#define ACTIVATE_EN         0x1

// Memory Map definitions
// Device memory
#define MAX_FLASH_CAPACITY  (2 * 1024 * 1024)

// PC memory
#define PC_WCB_BASE   0xB8000000 // 4GB-8MB
#define WCB_BASE_ADDR 0xB8000000

// Firmware Update Protocol definitions
/* Semaphore bits */
#define SHAW2_SEM_EXE           0x1
#define SHAW2_SEM_ERR           0x40
#define SHAW2_SEM_RDY           0x80

/* Command buffer size is 16 bytes - maximum data size is 8 bytes */
#define WCB_MAX_DATA             8
#define WCB_SIZE_ERASE_CMD       5 // 1 command byte, 4 address bytes
#define WCB_SIZE_READ_IDS_CMD    1
#define WCB_SIZE_ENTER_CMD       5 // 1 command byte, 4 code bytes
#define WCB_SIZE_EXIT_CMD        1
#define WCB_SIZE_ADDRESS_SET_CMD 5
#define WCB_SIZE_BYTE_CMD        2
#define WCB_SIZE_WORD_CMD        (WCB_SIZE_BYTE_CMD + 1)  
#define WCB_SIZE_DWORD_CMD       (WCB_SIZE_WORD_CMD + 2)
#define WCB_SIZE_LONGLONG_CMD    (WCB_SIZE_DWORD_CMD + 4)
#define WCB_SIZE_GENERIC_CMD     8   
#define WCB_SIZE_SET_WINDOW      9 // 1 command byte, 8 code bytes

/* Command buffer offsets */
#define OFFSET_SEMAPHORE        0
#define OFFSET_COMMAND          3
#define OFFSET_ADDRESS          4
#define OFFSET_DATA             4    // Commands have either address or data
#define OFFSET_DATA_GENERIC     8    // Generic commands have both address and data

/* WCB Commands */
#define FLASH_COMMANDS_INIT_OP  0x5A
#define WCB_ENTER_CODE          0xBECDAA55
#define ENTER_OP                0x10
#define EXIT_OP                 0x20
#define RESET_EC_OP             0x21
#define GOTO_BOOT_BLOCK_OP      0x22
#define ERASE_OP                0x80
#define SECTOR_ERASE_OP         0x82
#define ADDRESS_SET_OP          0xA0
#define PROGRAM_OP              0xB0
#define READ_IDS_OP             0xC0
#define GENERIC_OP              0xD0
#define SET_WINDOW_OP           0xC5

#define DEV_SIZE_UNIT           0x20000 // 128 Kb
#define PROGRAM_FLASH_BADDR		0x20000	// to program flash base address

// Write Command Buffer (WCB) commands structure
#pragma pack(1)
typedef struct
{
    unsigned char	Command;		// Byte  3
    union
    {
        FLASH_device_commands_t InitCommands;
        unsigned long   EnterCode;  // Bytes 4-7 For Flash Update "Unlock" code
        unsigned long	Address;    // Bytes 4-7 For Erase and Set Address
        unsigned char	Byte;		// Byte  4
        unsigned short	Word;		// Bytes 4-5
        unsigned long	DWord;		// Bytes 4-7
        struct
        {
            unsigned long DWord1;   // Bytes 4-7
            unsigned long DWord2;   // Bytes 8-11
        } EightBytes;
    } Param;
} WCB_struct;
#pragma pack()

/* Used standard Control Registers to access WPCE775x resource by superio index/data port. */
static inline void wrsio(unsigned char reg, unsigned char data)
{
	delay(10);
	*( (volatile unsigned char *)(MIPS_IO_BASE | SIO_INDEX_PORT) ) = reg;
	*( (volatile unsigned char *)(MIPS_IO_BASE | SIO_DATA_PORT) ) = data;
}

static inline unsigned char rdsio(unsigned char reg)
{
	delay(10);
	*( (volatile unsigned char *)(MIPS_IO_BASE | SIO_INDEX_PORT) ) = reg;
	return (*( (volatile unsigned char *)(MIPS_IO_BASE | SIO_DATA_PORT) ));
}

static inline void wrwcb(unsigned short addr, unsigned char data)
{
	delay(10);
	*( (volatile unsigned char *)(WCB_BASE_ADDR | addr) ) = data;
}

static inline unsigned char rdwcb(unsigned short addr)
{
	delay(10);
	return (*( (volatile unsigned char *)(WCB_BASE_ADDR | addr) ));
}

static inline void wr_wcb(unsigned short addr, unsigned char data)
{
	delay(10);
	*( (volatile unsigned char *)(addr) ) = data;
}

static inline unsigned char rd_wcb(unsigned int addr)
{
	delay(10);
	return (*( (volatile unsigned char *)(addr) ));
}

/* Access WPCE775x LDN register by superio index/data port. */
static inline void wrldn(unsigned char ldn_bank, unsigned char index, unsigned char data)
{
	delay(10);
	*( (volatile unsigned char *)(MIPS_IO_BASE | SIO_INDEX_PORT) ) = REG_SLDN;
	*( (volatile unsigned char *)(MIPS_IO_BASE | SIO_DATA_PORT) ) = ldn_bank;
	*( (volatile unsigned char *)(MIPS_IO_BASE | SIO_INDEX_PORT) ) = index;
	*( (volatile unsigned char *)(MIPS_IO_BASE | SIO_DATA_PORT) ) = data;
}

static inline unsigned char rdldn(unsigned char ldn_bank, unsigned char index)
{
	delay(10);
	*( (volatile unsigned char *)(MIPS_IO_BASE | SIO_INDEX_PORT) ) = REG_SLDN;
	*( (volatile unsigned char *)(MIPS_IO_BASE | SIO_DATA_PORT) ) = ldn_bank;
	*( (volatile unsigned char *)(MIPS_IO_BASE | SIO_INDEX_PORT) ) = index;
	return (*( (volatile unsigned char *)(MIPS_IO_BASE | SIO_DATA_PORT) ));
}

/* access WPCE775l EC Space content for ACPI */
static inline void wracpi(unsigned char cmd, unsigned char index, unsigned char data)
{
	delay(10);
	*( (volatile unsigned char *)(MIPS_IO_BASE | EC_CMD_PORT) ) = cmd;
	delay(RD_REG_DELAY);
	*( (volatile unsigned char *)(MIPS_IO_BASE | EC_DATA_PORT) ) = index;
	delay(RD_REG_DELAY);
	*( (volatile unsigned char *)(MIPS_IO_BASE | EC_DATA_PORT) ) = data;
}

static inline unsigned char rdacpi(unsigned char cmd, unsigned char index)
{
	delay(10);
	*( (volatile unsigned char *)(MIPS_IO_BASE | EC_CMD_PORT) ) = cmd;
	delay(RD_REG_DELAY);
	*( (volatile unsigned char *)(MIPS_IO_BASE | EC_DATA_PORT) ) = index;
	delay(RD_REG_DELAY);
	return (*( (volatile unsigned char *)(MIPS_IO_BASE | EC_DATA_PORT) ));
}
#endif
