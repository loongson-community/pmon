#include <pmon.h>
#include <linux/types.h>
static int LS2K_I2C_BASE=0xbfe01800;

#define readb(addr)     (*(volatile unsigned char *)(addr))
#define writeb(b,addr) ((*(volatile unsigned char *)(addr)) = (b))
#define ls2k_i2c_writeb(val, addr)	writeb(val, LS2K_I2C_BASE + addr)
#define ls2k_i2c_readb(addr)		readb(LS2K_I2C_BASE +  addr)


/* All transfers are described by this data structure */
struct i2c_msg {
	u16 addr;	/* slave address			*/
	u16 flags;
#define I2C_M_TEN		0x0010	/* this is a ten bit chip address */
#define I2C_M_RD		0x0001	/* read data, from slave to master */
#define I2C_M_NOSTART		0x4000	/* if I2C_FUNC_PROTOCOL_MANGLING */
#define I2C_M_REV_DIR_ADDR	0x2000	/* if I2C_FUNC_PROTOCOL_MANGLING */
#define I2C_M_IGNORE_NAK	0x1000	/* if I2C_FUNC_PROTOCOL_MANGLING */
#define I2C_M_NO_RD_ACK		0x0800	/* if I2C_FUNC_PROTOCOL_MANGLING */
#define I2C_M_RECV_LEN		0x0400	/* length will be first received byte */
	u16 len;		/* msg length				*/
	u8 *buf;		/* pointer to msg data			*/
};


#define	CR_START			0x80
#define	CR_STOP				0x40
#define	CR_READ				0x20
#define	CR_WRITE			0x10
#define	CR_ACK				0x8
#define	CR_IACK				0x1

#define	SR_NOACK			0x80
#define	SR_BUSY				0x40
#define	SR_AL				0x20
#define	SR_TIP				0x2
#define	SR_IF				0x1

#define LS2K_I2C_PRER_LO_REG	0x0
#define LS2K_I2C_PRER_HI_REG	0x1
#define LS2K_I2C_CTR_REG    	0x2
#define LS2K_I2C_TXR_REG    	0x3
#define LS2K_I2C_RXR_REG    	0x3
#define LS2K_I2C_CR_REG     	0x4
#define LS2K_I2C_SR_REG     	0x4

#define ls2k_i2c_debug(fmt, args...)	//printf(fmt, ##args)
#define pr_info printf

static void ls2k_i2c_stop(void)
{
again:
        ls2k_i2c_writeb(CR_STOP, LS2K_I2C_CR_REG);
        ls2k_i2c_readb(LS2K_I2C_SR_REG);
        while (ls2k_i2c_readb(LS2K_I2C_SR_REG) & SR_BUSY)
                goto again;
}

static int ls2k_i2c_start(int dev_addr, int flags)
{
	int retry = 5;
	unsigned char addr = (dev_addr & 0x7f) << 1;
	addr |= (flags & I2C_M_RD)? 1:0;

start:
	ls2k_i2c_writeb(addr, LS2K_I2C_TXR_REG);
	ls2k_i2c_debug("%s <line%d>: i2c device address: 0x%x\n",
			__func__, __LINE__, addr);
	ls2k_i2c_writeb((CR_START | CR_WRITE), LS2K_I2C_CR_REG);
	while (ls2k_i2c_readb(LS2K_I2C_SR_REG) & SR_TIP) ;

	if (ls2k_i2c_readb(LS2K_I2C_SR_REG) & SR_NOACK) {
		ls2k_i2c_stop();
		while (retry--)
			goto start;
		pr_info("There is no i2c device ack\n");
		return 0;
	}
	return 1;
}

static int ls2k_i2c_read(unsigned char *buf, int count)
{
        int i;

        for (i = 0; i < count; i++) {
                ls2k_i2c_writeb((i == count - 1)?
				(CR_READ | CR_ACK) : CR_READ,
				LS2K_I2C_CR_REG);
                while (ls2k_i2c_readb(LS2K_I2C_SR_REG) & SR_TIP) ;
                buf[i] = ls2k_i2c_readb(LS2K_I2C_RXR_REG);
		ls2k_i2c_debug("%s <line%d>: read buf[%d] <= %02x\n",
				__func__, __LINE__, i, buf[i]);
        }

        return i;
}

static int ls2k_i2c_write(unsigned char *buf, int count)
{
        int i;

        for (i = 0; i < count; i++) {
		ls2k_i2c_writeb(buf[i], LS2K_I2C_TXR_REG);
		ls2k_i2c_debug("%s <line%d>: write buf[%d] => %02x\n",
				__func__, __LINE__, i, buf[i]);
		ls2k_i2c_writeb(CR_WRITE, LS2K_I2C_CR_REG);
		while (ls2k_i2c_readb(LS2K_I2C_SR_REG) & SR_TIP) ;

		if (ls2k_i2c_readb(LS2K_I2C_SR_REG) & SR_NOACK) {
			ls2k_i2c_debug("%s <line%d>: device no ack\n",
					__func__, __LINE__);
			ls2k_i2c_stop();
			return 0;
		}
        }

        return i;
}


static int i2c_transfer(struct i2c_msg *msgs, int num)
{
	struct i2c_msg *m = msgs;
	int i;

	for(i = 0; i < num; i++) {
		if (!(m->flags & I2C_M_NOSTART) && !ls2k_i2c_start(m->addr, m->flags)) {
			return 0;
		}
		if (m->flags & I2C_M_RD)
			ls2k_i2c_read(m->buf, m->len);
		else
			ls2k_i2c_write(m->buf, m->len);
		++m;
	}

	ls2k_i2c_stop();

	return i;
}

/* ------------------------------------------------------------------------ */
/* API Functions                                                            */
/* ------------------------------------------------------------------------ */

void ls2k_i2c_init(int speed,  int slaveaddr)
{
	if(slaveaddr)
		LS2K_I2C_BASE = slaveaddr;
        ls2k_i2c_writeb(0, LS2K_I2C_CTR_REG);
        ls2k_i2c_writeb(0x64, LS2K_I2C_PRER_LO_REG);
        ls2k_i2c_writeb(0x4, LS2K_I2C_PRER_HI_REG);
        ls2k_i2c_writeb(0x80, LS2K_I2C_CTR_REG);
}



/**
 * i2c_read: - Read multiple bytes from an i2c device
 *
 * The higher level routines take into account that this function is only
 * called with len < page length of the device (see configuration file)
 *
 * @chip:	address of the chip which is to be read
 * @addr:	i2c data address within the chip
 * @alen:	length of the i2c data address (1..2 bytes)
 * @buffer:	where to write the data
 * @len:	how much byte do we want to read
 * @return:	0 in case of success
 */

int i2c_read(u8 chip, uint addr, int alen, u8 *buffer, int len)
{
	struct i2c_msg msg[2] = { { chip, 0, alen, (u8 *)&addr },
	                          { chip, I2C_M_RD, len, buffer }
	                        };

	ls2k_i2c_debug("i2c_read(chip=0x%02x, addr=0x%02x, alen=0x%02x, len=0x%02x)\n",chip,addr,alen,len);

	i2c_transfer(msg, 2);
	return 0;
}


/**
 * i2c_write: -  Write multiple bytes to an i2c device
 *
 * The higher level routines take into account that this function is only
 * called with len < page length of the device (see configuration file)
 *
 * @chip:	address of the chip which is to be written
 * @addr:	i2c data address within the chip
 * @alen:	length of the i2c data address (1..2 bytes)
 * @buffer:	where to find the data to be written
 * @len:	how much byte do we want to read
 * @return:	0 in case of success
 */

int i2c_write(u8 chip, uint addr, int alen, u8 *buffer, int len)
{
	struct i2c_msg msg[2] = { { chip, 0, alen, (u8 *)&addr },
	                          { chip, I2C_M_NOSTART, len, buffer }
	                        };
	ls2k_i2c_debug("i2c_write(chip=0x%02x, addr=0x%02x, alen=0x%02x, len=0x%02x)\n",chip,addr,alen,len);

	i2c_transfer(msg, 2);
	return 0;
}


union commondata{
		unsigned char data1;
		unsigned short data2;
		unsigned int data4;
		unsigned int data8[2];
		unsigned char c[8];
};
extern unsigned int syscall_addrtype;
extern int (*syscall1)(int type,long long addr,union commondata *mydata);
extern int (*syscall2)(int type,long long addr,union commondata *mydata);


static int syscall_i2c_addrlen;
static char syscall_i2c_chip;
static int syscall_i2c_addrlen;

static int i2c_read_syscall(int type,long long addr,union commondata *mydata)
{
	char c;
	switch(type)
	{
		case 1:
			i2c_read(syscall_i2c_chip, addr, syscall_i2c_addrlen, &mydata->data1, 1);

			break;

		default: return -1;break;
	}
	return 0;
}

static int i2c_write_syscall(int type,long long addr,union commondata *mydata)
{
	char c;
	switch(type)
	{
		case 1:
			i2c_write(syscall_i2c_chip, addr, syscall_i2c_addrlen, &mydata->data1, 1);

			break;

		default: return -1;break;
	}
	return 0;
}
//----------------------------------------

static int i2cs(int argc,char **argv)
{
	volatile int bus;
	if(argc<2) 
		return -1;

	bus=strtoul(argv[1],0,0);
	syscall_i2c_chip = strtoul(argv[2],0,0);
	syscall_i2c_addrlen = argc>3?strtoul(argv[3],0,0):1;

	ls2k_i2c_init(0, 0xbfe01000+bus*0x800);

	syscall1=(void*)i2c_read_syscall;
	syscall2=(void*)i2c_write_syscall;

	syscall_addrtype=0;

	return 0;
}

static const Cmd Cmds[] =
{
	{"MyCmds"},
	{"i2cs","bus chip [alen]", 0, "test i2c", i2cs, 0, 99, CMD_REPEAT},
	{0, 0}
};


static void init_cmd __P((void)) __attribute__ ((constructor));
static void init_cmd()
{
     cmdlist_expand(Cmds, 1);
}

