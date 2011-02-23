/* -- FLUPDATE.exe default file --
 * Winbond W25x80
 * ------------------------------- */
#define FLASH_ID   			0x13EF  /* device ID & Manufacturer */
#define EC_FLASH_SIZE		1024    /* (1M) Flash size in KBytes */
#define EC_SECTOR_SIZE 		4       /* (4K) sector size in Kbytes. */
#define EC_BLOCK_SIZE 		64		/* (64K) block size in Kbytes. If block erase is not supported then */
                                	/* block size should be equal to FLASH_SIZE */
#define EC_PAGE_SIZE  		256     /* Page size in bytes */
#define PROG_SIZE  			0xFF	/* Max number of bytes that can be programmed at on time. FF --> means page size */
#define READ_DEV_ID_TYPE 	0		/* Read Device ID type. Read with/without dummy */

/* Define spi Flash commands */
#define CMD_READ_DEV_ID		0x90	/* read device id command. */
#define CMD_READ_JEDEC_ID	0x9f	/* read JEDEC ID command. */
#define CMD_WRITE_STAT_EN 	0x06  	/* enable write to status register */
#define CMD_WRITE_EN      	0x06  	/* write enable */
#define CMD_READ_STAT     	0x05  	/* read status register */
#define CMD_WRITE_STAT    	0x01  	/* write status register */
#define CMD_PROG          	0x02  	/* page program, or byte program. */
#define CMD_SECTOR_ERASE  	0x20  	/* sector erase */
#define CMD_BLOCK_ERASE   	0xD8  	/* block erase */

/* Define device busy bits in status register */
#define STATUS_BUSY_MASK  	0x01  	/* Location (by mask) of busy bit located in the status register */
#define STATUS_REG_VAL    	0x00  	/* Value of status register busy bit, when device is not busy. */
                                	/* The device is consider not busy when: */
									/* [staus register value] & STATUS_BUSY_MASK = STATUS_REG_VAL */

/* Define the protected sectors. */
#define PROTECTED_SECTORS 	0,1,2,3	/* Protected sectors (0 to 3) */

