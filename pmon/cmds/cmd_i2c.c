#include <stdio.h>
#include <termio.h>
#include <setjmp.h>
#include <sys/endian.h>
#include <sys/linux/io.h>
#include <ctype.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <target/cs5536.h>

#include <machine/cpu.h>
#include <machine/bus.h>
#include <pmon.h>

#ifdef _KERNEL
#undef _KERNEL
#include <sys/ioctl.h>
#define _KERNEL
#else
#include <sys/ioctl.h>
#endif

unsigned int 	smb_base;

static unsigned int get_smb_base(void) 
{
	unsigned int hi, lo;
	_rdmsr(DIVIL_MSR_REG(DIVIL_LBAR_SMB), &hi, &lo);
	return lo;
}

static int i2c_wait(void)
{
    char	c;
    int 	i;
    
	delay(1000);
    for (i = 0; i < 20; i++) {
		c = linux_inb(smb_base | SMB_STS);
		if (c & (SMB_STS_BER | SMB_STS_NEGACK)) {
	    	return -1;
		}
		if (c & SMB_STS_SDAST)
	    	return 0;
		delay(100);
    }
    return -2;
}

static char i2c_read_single(int addr, int regNo, char *value)
{
    unsigned char 	c;
	
	/* Start condition */
	c = linux_inb(smb_base | SMB_CTRL1);
	linux_outb(c | SMB_CTRL1_START, smb_base | SMB_CTRL1);
    if(i2c_wait())
		return -1;	
	
	/* Send slave address */
	linux_outb(addr & 0xfe, smb_base | SMB_SDA);
    if(i2c_wait())
		return -1;
	
   	/* Acknowledge smbus */
	c = linux_inb(smb_base | SMB_CTRL1);
	linux_outb(c | SMB_CTRL1_ACK, smb_base | SMB_CTRL1);	
	
	/* Send register index */
	linux_outb(regNo, smb_base | SMB_SDA);
    if(i2c_wait())
		return -1;

   	/* Acknowledge smbus */
	c = linux_inb(smb_base | SMB_CTRL1);
	linux_outb(c | SMB_CTRL1_ACK, smb_base | SMB_CTRL1);	
	
	/* Start condition again */
	c = linux_inb(smb_base | SMB_CTRL1);
	linux_outb(c | SMB_CTRL1_START, smb_base | SMB_CTRL1);
	if(i2c_wait())
		return -1;

	/* Send salve address again */
	linux_outb(1 | addr, smb_base | SMB_SDA);
	if(i2c_wait())
		return -1;

   	/* Acknowledge smbus */
	c = linux_inb(smb_base | SMB_CTRL1);
	linux_outb(c | SMB_CTRL1_ACK, smb_base | SMB_CTRL1);	

	/* Read data */
	*value = linux_inb(smb_base | SMB_SDA);

	/* Stop condition */
	linux_outb(SMB_CTRL1_STOP, smb_base | SMB_CTRL1);
	if(i2c_wait())
		return -1;
}

static void i2c_write_single(int addr, int regNo, char value)
{
    unsigned char 	c;
    
	/* Start condition */
	c = linux_inb(smb_base | SMB_CTRL1);
	linux_outb(c | SMB_CTRL1_START, smb_base | SMB_CTRL1);
    if(i2c_wait())
   		return -1; 
	/* Send slave address */
    linux_outb(addr & 0xfe, smb_base | SMB_SDA);
    if(i2c_wait());
		return -1;

	/* Send register index */
    linux_outb(regNo, smb_base | SMB_SDA);
    if(i2c_wait())
		return -1;	

	/* Write data */
    linux_outb(value, smb_base | SMB_SDA);
    if(i2c_wait())
		return -1;
	/* Stop condition */
    linux_outb(SMB_CTRL1_STOP, smb_base | SMB_CTRL1);
    if(i2c_wait())
		return -1;

}

void cmd_i2cread(int argc, char *argv[])
{
	char value; 
	unsigned long av1, av2;	
	
	smb_base = get_smb_base();
	get_rsa(&av1, argv[1]);
	get_rsa(&av2, argv[2]);	
	
	i2c_read_single(av1, av2, &value);
	printf("reg value = 0x%02x\n", (unsigned char)value);
}

void cmd_i2cwrite(int argc, char *argv[])
{
	unsigned long av1, av2, av3;

	smb_base = get_smb_base();
	get_rsa(&av1, argv[1]);
	get_rsa(&av2, argv[2]);
	get_rsa(&av3, argv[3]);

	i2c_write_single(av1, av2, av3);
}	

static const Optdesc cmd_i2cread_opts[] = {
	{"<address>", "Device address"},
	{"<index>", "Register index"},
	{0},
};

static const Optdesc cmd_i2cwrite_opts[] = {
	{"<address>", "Device address"},
	{"<index>", "Register index"},
	{"<value>", "the value write to the register"},
	{0},
};

static const Cmd Cmds[] = {
	{"i2c_func"},
	{"i2cread", "<address> <index>", 
			cmd_i2cread_opts,
			"read the index register",
			cmd_i2cread, 3, 3, 0
	},
	{"i2cwrite", "<address> <index>",
			cmd_i2cwrite_opts,
			"write the index register",
			cmd_i2cwrite, 4, 4, 0
	},
	{0, 0}
};

static void init_cmd __P((void)) __attribute__ ((constructor));

void init_cmd()
{
	cmdlist_expand(Cmds, 1);
}
