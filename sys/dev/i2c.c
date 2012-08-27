#include <pmon.h>
#include <stdio.h>
#include <target/i2c.h>

//----------------------------------------------------
void i2c_stop()
{
	*GS_SOC_I2C_CR = CR_STOP;
	*GS_SOC_I2C_SR;
	while (*GS_SOC_I2C_SR & SR_BUSY) ;
}

///////////////////////////////////////////////////o
// below added by xqch to make i2c_read like ls3a
unsigned char i2c_read(unsigned char baseaddr, unsigned char regoff)
{

	int i, j;
	unsigned char ret = 0x5a;
	unsigned char addr[2];

	addr[0] = baseaddr;
	addr[1] = regoff;

	printf("base addr = %08x, regoff = %08x\n", baseaddr, regoff);
	printf("addr[0] = %08x, addr[1] = %08x\n", addr[0], addr[1]);

	tgt_i2cinit();
	for (j = 0; j < 32; j++) {
		*GS_SOC_I2C_TXR = addr[0] & 0xfe;
		*GS_SOC_I2C_CR = CR_START | CR_WRITE;
		while (*GS_SOC_I2C_SR & SR_TIP) ;
		*GS_SOC_I2C_TXR = j;
		*GS_SOC_I2C_CR = CR_STOP | CR_WRITE;
		while (*GS_SOC_I2C_SR & SR_TIP) ;

		/* why set slave address again */
		*GS_SOC_I2C_TXR = addr[0] | 1;
		*GS_SOC_I2C_CR = CR_START | CR_WRITE;	/* start on first addr */
		while (*GS_SOC_I2C_SR & SR_TIP) ;

		*GS_SOC_I2C_CR = CR_READ | CR_ACK;	/* last read not send ack */
		while (*GS_SOC_I2C_SR & SR_TIP) ;

		ret = *GS_SOC_I2C_TXR;
		printf("%2d: ret = %08x\n", j, ret);
		*GS_SOC_I2C_CR = CR_STOP;
		*GS_SOC_I2C_SR;
		while (*GS_SOC_I2C_SR & SR_BUSY) ;
	}
	//ret = * GS_SOC_I2C_TXR;
	//printf("1: ret = %08x\n", ret);

//      i2c_stop();
	return ret;
}

////////////////////////////////////////////////////////////////
unsigned char i2c_rec_s(unsigned char *addr, int addrlen, unsigned char *buf,
			int count)
{
	int i;
	int j;
	unsigned char value;
	for (i = 0; i < count; i++) {
again:
		for (j = 0; j < addrlen; j++) {
			/*write slave_addr */
			*GS_SOC_I2C_TXR = addr[j];
			*GS_SOC_I2C_CR =
			    (j == 0) ? (CR_START | CR_WRITE) : CR_WRITE;
			while (*GS_SOC_I2C_SR & SR_TIP) ;

			if ((*GS_SOC_I2C_SR) & SR_NOACK) {
				printf("j= %d ,read no ack %d\n", j, __LINE__);
				i2c_stop();
				goto again;
			}
		}

		/*write slave_addr */
		*GS_SOC_I2C_TXR = addr[0] | 1;
		*GS_SOC_I2C_CR = CR_START | CR_WRITE;	/* start on first addr */
		while (*GS_SOC_I2C_SR & SR_TIP) ;

		if ((*GS_SOC_I2C_SR) & SR_NOACK) {
			printf("read no ack %d\n", __LINE__);
			i2c_stop();
			goto again;
		}

		*GS_SOC_I2C_CR = CR_READ | CR_ACK;	/*last read not send ack */
		while (*GS_SOC_I2C_SR & SR_TIP) ;

		buf[i] = *GS_SOC_I2C_TXR;
		*GS_SOC_I2C_CR = CR_STOP;
		*GS_SOC_I2C_SR;
		while (*GS_SOC_I2C_SR & SR_BUSY) ;

	}

	return count;
}

unsigned char i2c_send_s(unsigned char *addr, int addrlen, unsigned char *buf,
			 int count)
{
	int i;
	int j;
	for (i = 0; i < count; i++) {
again:
		for (j = 0; j < addrlen; j++) {
			/*write slave_addr */
			*GS_SOC_I2C_TXR = addr[j];
			*GS_SOC_I2C_CR = j == 0 ? (CR_START | CR_WRITE) : CR_WRITE;	/* start on first addr */
			while (*GS_SOC_I2C_SR & SR_TIP) ;

			if ((*GS_SOC_I2C_SR) & SR_NOACK) {
				printf("write no ack %d\n", __LINE__);
				i2c_stop();
				goto again;
			}
		}

		*GS_SOC_I2C_TXR = buf[i];
		*GS_SOC_I2C_CR = CR_WRITE | CR_STOP;
		while (*GS_SOC_I2C_SR & SR_TIP) ;

		if ((*GS_SOC_I2C_SR) & SR_NOACK) {
			printf("write no ack %d\n", __LINE__);
			i2c_stop();
			goto again;
		}
	}
	while (*GS_SOC_I2C_SR & SR_BUSY) ;
	return count;
}

unsigned char i2c_rec_b(unsigned char *addr, int addrlen, unsigned char *buf,
			int count)
{
	int i;
	int j;

	unsigned char value;

	for (j = 0; j < addrlen; j++) {
		/*write slave_addr */
		*GS_SOC_I2C_TXR = addr[j];
		*GS_SOC_I2C_CR = j == 0 ? (CR_START | CR_WRITE) : CR_WRITE;	/* start on first addr */
		while (*GS_SOC_I2C_SR & SR_TIP) ;

		if ((*GS_SOC_I2C_SR) & SR_NOACK)
			return i;
	}

	*GS_SOC_I2C_TXR = addr[0] | 1;
	*GS_SOC_I2C_CR = CR_START | CR_WRITE;
	if ((*GS_SOC_I2C_SR) & SR_NOACK)
		return i;

	for (i = 0; i < count; i++) {
		*GS_SOC_I2C_CR = CR_READ;
		while (*GS_SOC_I2C_SR & SR_TIP) ;

		buf[i] = *GS_SOC_I2C_TXR;
	}
	*GS_SOC_I2C_CR = CR_STOP;

	return count;
}

unsigned char i2c_send_b(unsigned char *addr, int addrlen, unsigned char *buf,
			 int count)
{
	int i;
	int j;
	for (j = 0; j < addrlen; j++) {
		/*write slave_addr */
		*GS_SOC_I2C_TXR = addr[j];
		*GS_SOC_I2C_CR = j == 0 ? (CR_START | CR_WRITE) : CR_WRITE;	/* start on first addr */
		while (*GS_SOC_I2C_SR & SR_TIP) ;

		if ((*GS_SOC_I2C_SR) & SR_NOACK)
			return i;
	}

	for (i = 0; i < count; i++) {
		*GS_SOC_I2C_TXR = buf[i];
		*GS_SOC_I2C_CR = CR_WRITE;
		while (*GS_SOC_I2C_SR & SR_TIP) ;

		if ((*GS_SOC_I2C_SR) & SR_NOACK)
			return i;

	}
	*GS_SOC_I2C_CR = CR_STOP;
	while (*GS_SOC_I2C_SR & SR_BUSY) ;
	return count;
}

/*
 * 0 single 
 * 1 block
 * 2 smb block
 */
int tgt_i2cread(int type, unsigned char *addr, int addrlen, unsigned char *buf,
		int count)
{
	int i;
	tgt_i2cinit();
	memset(buf, -1, count);
	switch (type) {
	case I2C_SINGLE:
		return i2c_rec_s(addr, addrlen, buf, count);
		break;
	case I2C_BLOCK:
		return i2c_rec_b(addr, addrlen, buf, count);
		break;

	default:
		return 0;
		break;
	}
	return 0;
}

int tgt_i2cwrite(int type, unsigned char *addr, int addrlen, unsigned char *buf,
		 int count)
{
	tgt_i2cinit();
	switch (type & 0xff) {
	case I2C_SINGLE:
		i2c_send_s(addr, addrlen, buf, count);
		break;
	case I2C_BLOCK:
		return i2c_send_b(addr, addrlen, buf, count);
		break;
	case I2C_SMB_BLOCK:
		break;
	default:
		return -1;
		break;
	}
	return -1;
}

int tgt_i2cinit()
{
	static int inited = 0;
	if (inited)
		return 0;
	inited = 1;
	*GS_SOC_I2C_PRER_LO = 0x64;
	*GS_SOC_I2C_PRER_HI = 0;
	*GS_SOC_I2C_CTR = 0x80;
}


int cmd_i2cread(int ac, char *av[])
{
	int i, len = 1;
	unsigned char addr[16] = { 0xa3, 0x02, 0x02, 0x02, 0x02, 0x02 };
	unsigned char slave = 0xa3;
	unsigned char regoff = 0x02;
	unsigned char buf[16];

	for (i = 0; i < ac; i++)
		printf("av[%d] : %s\n", i, av[i]);

	slave = strtoul(av[1], NULL, 0);
	printf("slave = %08x\n ", slave);
	if ((slave & 0x1) == 0)	// Fix bad input
		slave = 0xa3;

	if (ac > 2)
		regoff = strtol(av[2], NULL, 0);
	printf("regoff = %08x\n", regoff);

	printf("%08x\n", i2c_read(slave, regoff));

	return 0;
}

static const Cmd Cmds[] = {
	{"Misc"},
	{"i2cread", "", NULL, "i2cread a address from ls2h chip", cmd_i2cread,
	 1, 5, 0},
	{0, 0}
};

static void init_cmd __P((void)) __attribute__ ((constructor));

void init_cmd()
{
	cmdlist_expand(Cmds, 1);
}
