/* ------------------------------------------------------------------------- */
/* i2c-algo-bit.c i2c driver algorithms for bit-shift adapters		     */
/* ------------------------------------------------------------------------- */
/*   Copyright (C) 1995-2000 Simon G. Vogl

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.		     */
/* ------------------------------------------------------------------------- */

/* With some changes from Frodo Looijaard <frodol@dds.nl>, Kyösti Mälkki
   <kmalkki@cc.hut.fi> and Jean Delvare <khali@linux-fr.org> */

#include <stdio.h>
#include <pmon.h>
/* ----- global defines ----------------------------------------------- */
#define DEB(x) if (i2c_debug>=1) x;
#define DEB2(x) if (i2c_debug>=2) x;
#define DEBSTAT(x) if (i2c_debug>=3) x; /* print several statistical values*/
#define DEBPROTO(x) if (i2c_debug>=9) { x; }
#define printk printf

#define NB_RS780E_I2C
//#define RMMIO 0x9000000044000000//vxworks
#define RMMIO 0xc8000000 //pci 1,5,0 base_reg_2
#define MAX_WAYS 8
#define USE_NB_I2C_3
#define EREMOTEIO 10
#define ETIMEDOUT 20
#define ENODEV 30
#define EFAULT 40
#define UDELAY 20

 	/* debug the protocol by showing transferred bits */

struct i2c_msg {
	unsigned short addr;	/* slave address			*/
 	unsigned short flags;		
#define I2C_M_TEN	0x10	/* we have a ten bit chip address	*/
#define I2C_M_RD	0x01
#define I2C_M_NOSTART	0x4000
#define I2C_M_REV_DIR_ADDR	0x2000
#define I2C_M_IGNORE_NAK	0x1000
#define I2C_M_NO_RD_ACK		0x0800
 	unsigned short len;		/* msg length				*/
 	unsigned char *buf;		/* pointer to msg data			*/
};

/* ----- global variables ---------------------------------------------	*/

static int i2c_debug = 10;
static int bit_test;	/* see if the line-setting functions work	*/
static int lenthzmz;
struct radeon_i2c_bus_record {
	/* regs and bits */
	unsigned int mask_clk_reg;
	unsigned int mask_data_reg;
	unsigned int a_clk_reg;
	unsigned int a_data_reg;
	unsigned int en_clk_reg;
	unsigned int en_data_reg;
	unsigned int y_clk_reg;
	unsigned int y_data_reg;
	unsigned int mask_clk_mask;
	unsigned int mask_data_mask;
	unsigned int a_clk_mask;
	unsigned int a_data_mask;
	unsigned int en_clk_mask;
	unsigned int en_data_mask;
	unsigned int y_clk_mask;
	unsigned int y_data_mask;
} _gRadeon_i2c_bus_rec[MAX_WAYS];

inline void ___delay(unsigned int loops)
{
	__asm__ __volatile__ (
	"	.set	noreorder				\n"
	"	.align	3					\n"
	"1:	bnez	%0, 1b					\n"
	"	subu	%0, 1					\n"
	"	.set	reorder					\n"
	: "=r" (loops)
	: "0" (loops));
}
EXPORT_SYMBOL(__delay);

void __udelay(unsigned long us)
{
	unsigned int lpj = 1277952;//raw_current_cpu_data.udelay_val;

	___delay((us * 0x000010c7ull * 250 * lpj) >> 36);
}

//vxworks
static inline void udelay(unsigned long us)
{
		__udelay(us);
}
/* --- setting states on the bus with the right timing: ---------------	*/

static void setsda(int ivalue,unsigned int way);
static void setscl(int ivalue,unsigned int way);
static inline void tgt_i2cinit(unsigned int way)
{

#ifndef NB_RS780E_I2C
#if defined(DEVBD2F_SM502)
pcitag_t tag;
static int inited=0;
int tmp;
		if(!inited)
		{
		tag=_pci_make_tag(0,14,0);
	
		mmio = _pci_conf_readn(tag,0x14,4);
		mmio =(int)mmio|(0xb0000000);
		tmp = *(volatile int *)(mmio + 0x40);
		*(volatile int *)(mmio + 0x40) =tmp|0x40;

		//tgt_printf("clock enable bit 40 = %x\n", *(volatile int *)(mmio + 0x40));
		inited=1;
		}
#endif
#else

		unsigned long  tag;
		int mmio=0;
		static int inited = 0;
		static int pre_way = -1;

		tag=_pci_make_tag(1,5,0);
		mmio = _pci_conf_readn(tag,0x18,4);
		printf("mmio_base=0x%x\n",mmio);


		if(way >= MAX_WAYS)
		{
		  printf("I2C Ways exceed MAX_WAYS\r\n");
		  return;
		}

        //avoiding duplicate initialize the same way 
		if(pre_way == way)
		{
			inited =1;
			pre_way =way;
		}else
		{
			inited =0;
		}

		if(!inited)
		{
			_gRadeon_i2c_bus_rec[way].mask_clk_mask  = 0x1;
			_gRadeon_i2c_bus_rec[way].mask_data_mask = 0x100;
			_gRadeon_i2c_bus_rec[way].a_clk_mask     = 0x1;
			_gRadeon_i2c_bus_rec[way].a_data_mask    = 0x100;
			_gRadeon_i2c_bus_rec[way].en_clk_mask    = 0x1;
			_gRadeon_i2c_bus_rec[way].en_data_mask   = 0x100;
			_gRadeon_i2c_bus_rec[way].y_clk_mask     = 0x1;
			_gRadeon_i2c_bus_rec[way].y_data_mask    = 0x100;

			switch(way)
			{
				case 0:
					_gRadeon_i2c_bus_rec[0].mask_clk_reg  = 0x7e20;
					_gRadeon_i2c_bus_rec[0].mask_data_reg = 0x7e20;
					_gRadeon_i2c_bus_rec[0].a_clk_reg     = 0x7e24;
					_gRadeon_i2c_bus_rec[0].a_data_reg    = 0x7e24;
					_gRadeon_i2c_bus_rec[0].en_clk_reg    = 0x7e28;
					_gRadeon_i2c_bus_rec[0].en_data_reg   = 0x7e28;
					_gRadeon_i2c_bus_rec[0].y_clk_reg     = 0x7e2c;
					_gRadeon_i2c_bus_rec[0].y_data_reg    = 0x7e2c;
					break;

				case 1://VGA
					_gRadeon_i2c_bus_rec[1].mask_clk_reg  = 0x7e40;
					_gRadeon_i2c_bus_rec[1].mask_data_reg = 0x7e40;
					_gRadeon_i2c_bus_rec[1].a_clk_reg     = 0x7e44;
					_gRadeon_i2c_bus_rec[1].a_data_reg    = 0x7e44;
					_gRadeon_i2c_bus_rec[1].en_clk_reg    = 0x7e48;
					_gRadeon_i2c_bus_rec[1].en_data_reg   = 0x7e48;
					_gRadeon_i2c_bus_rec[1].y_clk_reg     = 0x7e4c;
					_gRadeon_i2c_bus_rec[1].y_data_reg    = 0x7e4c;
					break;
					
				case 2://I2C (use vbios vgarom-780.h)
					_gRadeon_i2c_bus_rec[2].mask_clk_reg  = 0x7e50;
					_gRadeon_i2c_bus_rec[2].mask_data_reg = 0x7e50;
					_gRadeon_i2c_bus_rec[2].a_clk_reg     = 0x7e54;
					_gRadeon_i2c_bus_rec[2].a_data_reg    = 0x7e54;
					_gRadeon_i2c_bus_rec[2].en_clk_reg    = 0x7e58;
					_gRadeon_i2c_bus_rec[2].en_data_reg   = 0x7e58;
					_gRadeon_i2c_bus_rec[2].y_clk_reg     = 0x7e5c;
					_gRadeon_i2c_bus_rec[2].y_data_reg    = 0x7e5c;
					break;

				case 3://DDC_CLK0,DDC_DATA0 OK (use vbios vgarom-780.i2cok)
					_gRadeon_i2c_bus_rec[3].mask_clk_reg  = 0x7e60;
					_gRadeon_i2c_bus_rec[3].mask_data_reg = 0x7e60;
					_gRadeon_i2c_bus_rec[3].a_clk_reg     = 0x7e64;
					_gRadeon_i2c_bus_rec[3].a_data_reg    = 0x7e64;
					_gRadeon_i2c_bus_rec[3].en_clk_reg    = 0x7e68;
					_gRadeon_i2c_bus_rec[3].en_data_reg   = 0x7e68;
					_gRadeon_i2c_bus_rec[3].y_clk_reg     = 0x7e6c;
					_gRadeon_i2c_bus_rec[3].y_data_reg    = 0x7e6c;
					break;

				case 4:
					_gRadeon_i2c_bus_rec[4].mask_clk_reg  = 0x7e70;
					_gRadeon_i2c_bus_rec[4].mask_data_reg = 0x7e70;
					_gRadeon_i2c_bus_rec[4].a_clk_reg     = 0x7e74;
					_gRadeon_i2c_bus_rec[4].a_data_reg    = 0x7e74;
					_gRadeon_i2c_bus_rec[4].en_clk_reg    = 0x7e78;
					_gRadeon_i2c_bus_rec[4].en_data_reg   = 0x7e78;
					_gRadeon_i2c_bus_rec[4].y_clk_reg     = 0x7e7c;
					_gRadeon_i2c_bus_rec[4].y_data_reg    = 0x7e7c;
					break;
					
				case 5:
					_gRadeon_i2c_bus_rec[5].mask_clk_reg  = 0x7e30;
					_gRadeon_i2c_bus_rec[5].mask_data_reg = 0x7e30;
					_gRadeon_i2c_bus_rec[5].a_clk_reg     = 0x7e34;
					_gRadeon_i2c_bus_rec[5].a_data_reg    = 0x7e34;
					_gRadeon_i2c_bus_rec[5].en_clk_reg    = 0x7e38;
					_gRadeon_i2c_bus_rec[5].en_data_reg   = 0x7e38;
					_gRadeon_i2c_bus_rec[5].y_clk_reg     = 0x7e3c;
					_gRadeon_i2c_bus_rec[5].y_data_reg    = 0x7e3c;
					break;



			}

			setsda(1,way);
			setscl(1,way);
			inited=1;
		}
#endif
}

static void setsda(int ivalue,unsigned int way)
{
	unsigned int val;
#if 1
	/* _gRadeon_i2c_bus_rec[way].en_data_mask   = 0x100;*/
	val =(*(volatile unsigned int *)(RMMIO + (_gRadeon_i2c_bus_rec[way].en_data_reg))) & (~_gRadeon_i2c_bus_rec[way].en_data_mask);
	val |= ivalue ? 0 : _gRadeon_i2c_bus_rec[way].en_data_mask;
	(*(volatile unsigned int *)(RMMIO + (_gRadeon_i2c_bus_rec[way].en_data_reg))) = val;
#else
	val = RREG32(_gRadeon_i2c_bus_rec.en_data_reg) & ~_gRadeon_i2c_bus_rec.en_data_mask;
	val |= ivalue ? 0 : _gRadeon_i2c_bus_rec.en_data_mask;
	WREG32(_gRadeon_i2c_bus_rec.en_data_reg, val);
#endif
}
static void setscl(int ivalue,unsigned int way)
{
	unsigned int val;
#if 1
	/* _gRadeon_i2c_bus_rec[way].en_data_mask   = 0x1;*/
	val =(*(volatile unsigned int *)(RMMIO + (_gRadeon_i2c_bus_rec[way].en_clk_reg))) & (~_gRadeon_i2c_bus_rec[way].en_clk_mask);
	val |= ivalue ? 0 : _gRadeon_i2c_bus_rec[way].en_clk_mask;
	(*(volatile unsigned int *)(RMMIO + (_gRadeon_i2c_bus_rec[way].en_clk_reg))) = val;
#else
	val = RREG32(_gRadeon_i2c_bus_rec.en_clk_reg) & ~_gRadeon_i2c_bus_rec.en_clk_mask;
	val |= ivalue ? 0 : _gRadeon_i2c_bus_rec.en_clk_mask;
	WREG32(_gRadeon_i2c_bus_rec.en_clk_reg, val);
#endif
}

static int getscl(unsigned int way)
{
		unsigned int val;
		val =(*(volatile unsigned int *)(RMMIO + (_gRadeon_i2c_bus_rec[way].y_clk_reg))) & (_gRadeon_i2c_bus_rec[way].y_clk_mask);
		return (val != 0);
}
static int getsda(unsigned int way)
{
		unsigned int val;
		#if 1
		val =(*(volatile unsigned int *)(RMMIO + (_gRadeon_i2c_bus_rec[way].y_data_reg))) & (_gRadeon_i2c_bus_rec[way].y_data_mask);
		#else
val = RREG32(_gRadeon_i2c_bus_rec.y_data_reg) & ~_gRadeon_i2c_bus_rec.y_data_mask;
		#endif
	//	printf("------------------val:0x%x----------------\n",val);
		return (val != 0);
}

static inline void sdalo(unsigned int way)
{
	setsda(0,way);
	udelay((UDELAY+1)/2 );
}

static inline void sdahi(unsigned int way)
{
	setsda(1,way);
	udelay((UDELAY+1)/2 );
}

static inline void scllo(unsigned int way)
{
	setscl(0,way);
	udelay((UDELAY)/2 );
}

/*
 * Raise scl line, and do checking for delays. This is necessary for slower
 * devices.
 */
static inline int sclhi(unsigned int way)
{
	unsigned long start;

	setscl(1,way);
	udelay(UDELAY);
	
	return 0;
} 


/* --- other auxiliary functions --------------------------------------	*/
static void i2c_start(unsigned int way) 
{
//	printf("----------------%s--------------\n",__func__);
	sdahi(way);
	sclhi(way);
	/* assert: scl, sda are high */
#if 0
	setsda(0,way);
	udelay(UDELAY);
	scllo(way);	
#endif
	setscl(1,way);
	udelay(UDELAY);
	setscl(0,way);
	udelay(UDELAY);
	setscl(1,way);
	udelay(UDELAY);
	setscl(0,way);
	udelay(UDELAY);
	setscl(1,way);
	udelay(UDELAY);
	setscl(0,way);
	udelay(UDELAY);
	setscl(1,way);
}

static void i2c_repstart(unsigned int way) 
{
	/* assert: scl is low */
	sdahi(way);
	sclhi(way);
	setsda(0,way);
	udelay(UDELAY);
	scllo(way);
}


static void i2c_stop(unsigned int way) 
{
//	printf("----------------%s--------------\n",__func__);
	/* assert: scl is low */
	sdalo(way);
	sclhi(way); 
	setsda(1,way);
	udelay(UDELAY);
}



/* send a byte without start cond., look for arbitration, 
   check ackn. from slave */
/* returns:
 * 1 if the device acknowledged
 * 0 if the device did not ack
 * -ETIMEDOUT if an error occurred (while raising the scl line)
 */
static int i2c_outb(char c,unsigned int way)
{
	int i;
	int sb;
	int ack;

	/* assert: scl is low */
	//c = 0xa0;
	for ( i=7 ; i>=0 ; i-- ) {
		sb = c & ( 1 << i );
		setsda(sb,way);
		udelay((UDELAY+1)/2);
//		DEBPROTO(printk(  "----------%d",sb!=0));
		if (sclhi(way)<0) { /* timed out */
			DEB2(printk(  " i2c_outb: 0x%02x, timeout at bit #%d\n", c&0xff, i));
			return -ETIMEDOUT;
		};
		/* do arbitration here: 
		 * if ( sb && ! getsda(adap) ) -> ouch! Get out of here.
		 */
		udelay((UDELAY+1)/2);
		scllo(way);
	}
//		printf("\n");
	sdahi(way);

	if (sclhi(way)<0){ /* timeout */
	    DEB2(printk(  " i2c_outb: 0x%02x, timeout at ack\n", c&0xff));
	    return -ETIMEDOUT;
	};
	/* read ack: SDA should be pulled down by slave */
	ack=!getsda(way);	/* ack: sda is pulled low ->success.	 */
//	DEB2(printk(  " i2c_outb: 0x%02x , getsda() = %d\n", c & 0xff, ack));

//	DEBPROTO( printk(  "[%2.2x]",c&0xff) );
//	DEBPROTO(if (0==ack){ printk(  " A \n");} else printk(  " NA \n") );
	scllo(way);
	return ack;		/* return 1 if device acked	 */
	/* assert: scl is low (sda undef) */
}


static int i2c_inb(unsigned int way) 
{
	/* read byte via i2c port, without start/stop sequence	*/
	/* acknowledge is sent in i2c_read.			*/
	int i;
	unsigned char indata=0;

	/* assert: scl is low */
	sdahi(way);
	for (i=0;i<8;i++) {
		if (sclhi(way)<0) { /* timeout */
			DEB2(printk(  " i2c_inb: timeout at bit #%d\n", 7-i));
			return -ETIMEDOUT;
		};
		udelay((UDELAY+1)/2);
		indata *= 2;
		if ( getsda(way) ) 
			indata |= 0x01;
	//	setscl( 0,way);
		scllo(way);
		udelay(i == 7 ? UDELAY / 2 : UDELAY);

	}
	/* assert: scl is low */
//	DEB2(printk(  "i2c_inb: 0x%02x\n", indata & 0xff));

//	DEBPROTO(printk(  " 0x%02x", indata & 0xff));
	return (int) (indata & 0xff);
}


/* ----- Utility functions
 */

/* try_address tries to contact a chip for a number of
 * times before it gives up.
 * return values:
 * 1 chip answered
 * 0 chip did not answer
 * -x transmission error
 */
static inline int try_address( unsigned char addr, int retries,unsigned int way )
{
	int i,ret = -1;
	for (i=0; i<=retries; i++) {
		ret = i2c_outb(addr,way);
		if (ret==1 || i==retries)
			break;	/* success! */
		i2c_stop(way);
		udelay(UDELAY/*adap->udelay*/);
		i2c_start(way);
	}
	DEB2(if (i)
	     printk(  "i2c-algo-bit.o: Used %d tries to %s client at 0x%02x : %s\n",
		    i+1, addr & 1 ? "read" : "write", addr>>1,
		    ret==1 ? "success" : ret==0 ? "no ack" : "failed, timeout?" )
	    );
	return ret;
}

static int sendbytes( struct i2c_msg *msg,unsigned int way)
{
	char c;
	const char *temp = msg->buf;
	int count = msg->len;
	unsigned short nak_ok = msg->flags & I2C_M_IGNORE_NAK; 
	int retval;
	int wrcount=0;

	while (count > 0) {
		c = *temp;
//		DEB2(dev_dbg(&i2c_adap->dev, "sendbytes: writing %2.2X\n", c&0xff));
		retval = i2c_outb(c,way);
		if ((retval>0) || (nak_ok && (retval==0)))  { /* ok or ignored NAK */
			count--; 
			temp++;
			wrcount++;
		} else { /* arbitration or no acknowledge */
//			dev_err(&i2c_adap->dev, "sendbytes: error - bailout.\n");
			i2c_stop(way);
			return (retval<0)? retval : -EFAULT;
			        /* got a better one ?? */
		}
#if 0
		/* from asm/delay.h */
		__delay(adap->mdelay * (loops_per_sec / 1000) );
#endif
	}
	return wrcount;
}

static inline int readbytes(struct i2c_msg *msg,unsigned int way)
{
	int inval;
	int rdcount=0;   	/* counts bytes read */
	char *temp = msg->buf;
	int count = msg->len;

	while (count > 0) {
		inval = i2c_inb(way);
		if (inval>=0) {
			*temp = inval;
			rdcount++;
		} else {   /* read timed out */
			printk("i2c-algo-bit.o: readbytes: i2c_inb timed out.\n");
			break;
		}

		temp++;
		count--;

		if (msg->flags & I2C_M_NO_RD_ACK)
			continue;

		if ( count > 0 ) {		/* send ack */
			sdalo(way);
			 udelay((UDELAY+1)/2);
//			DEBPROTO(printk(" Am "));
		} else {
			sdahi(way);	/* neg. ack on last byte */
	//		DEBPROTO(printk(" NAm "));
		}
		if (sclhi(way)<0) {	/* timeout */
			sdahi(way);
			printk( "i2c-algo-bit.o: readbytes: Timeout at ack\n");
			return -ETIMEDOUT;
		};
			 udelay((UDELAY+1)/2);
		scllo(way);
		sdahi(way);
	}
	return rdcount;
}

/* doAddress initiates the transfer by generating the start condition (in
 * try_address) and transmits the address in the necessary format to handle
 * reads, writes as well as 10bit-addresses.
 * returns:
 *  0 everything went okay, the chip ack'ed, or IGNORE_NAK flag was set
 * -x an error occurred (like: -EREMOTEIO if the device did not answer, or
 *	-ETIMEDOUT, for example if the lines are stuck...) 
 */
static inline int bit_doAddress( struct i2c_msg *msg,unsigned int way) 
{
	unsigned short flags = msg->flags;
	unsigned short nak_ok = msg->flags & I2C_M_IGNORE_NAK;

	unsigned char addr;
	int ret, retries;

	retries = nak_ok ? 0 : 3;
	
	if ( (flags & I2C_M_TEN)  ) { 
		/* a ten bit address */
		addr = 0xf0 | (( msg->addr >> 7) & 0x03);
		DEB2(printk(  "addr0: %d\n",addr));
		/* try extended address code...*/
		ret = try_address(addr, retries, way);
		if ((ret != 1) && !nak_ok)  {
			printk( "died at extended address code.\n");
			return -EREMOTEIO;
		}
		/* the remaining 8 bit address */
		ret = i2c_outb(msg->addr & 0x7f, way);
		if ((ret != 1) && !nak_ok) {
			/* the chip did not ack / xmission error occurred */
			printk( "died at 2nd address code.\n");
			return -EREMOTEIO;
		}
		if ( flags & I2C_M_RD ) {
			i2c_repstart(way);
			/* okay, now switch into reading mode */
			addr |= 0x01;
			ret = try_address(addr, retries, way);
			if ((ret!=1) && !nak_ok) {
				printk("died at extended address code.\n");
				return -EREMOTEIO;
			}
		}
	} else {		/* normal 7bit address	*/
		//addr = ( msg->addr << 1 );/*zmz remove*/
		addr = msg->addr ;/*zmz add*/
//		printf("addr 0x%x\n",addr);
		if (flags & I2C_M_RD )
			addr |= 1;
		if (flags & I2C_M_REV_DIR_ADDR )
			addr ^= 1;
		ret = try_address(addr, retries, way);
		if ((ret!=1) && !nak_ok)
			return -EREMOTEIO;
	}

	return 0;
}

int bit_xfer(struct i2c_msg msgs[], int num,unsigned int way)
{
	struct i2c_msg *pmsg;
	
	int i,ret;
	unsigned short nak_ok;

	tgt_i2cinit(way);
	i2c_start(way);
	for (i=0;i<num;i++) {
		pmsg = &msgs[i];
		nak_ok = pmsg->flags & I2C_M_IGNORE_NAK; 
		if (!(pmsg->flags & I2C_M_NOSTART)) {
			if (i) {
				i2c_repstart(way);
			}
			ret = bit_doAddress(pmsg,way);
			if ((ret != 0) && !nak_ok) {
			    DEB2(printk(  "i2c-algo-bit.o: NAK from device addr %2.2x msg #%d\n"
					,msgs[i].addr,i));

				goto bailout;
			}
		}
		if (pmsg->flags & I2C_M_RD ) {
			/* read bytes into buffer*/
			ret = readbytes(pmsg, way);
			DEB2(printk(  "i2c-algo-bit.o: read %d bytes.\n",ret));
			if (ret < pmsg->len ) {
				 if(ret>=0) 
				 ret = -EREMOTEIO;
				goto bailout;
			}
		} else {
			/* write bytes from buffer */
			i2c_outb(0,way);/*the address in the e2prom ,here is 0*/
			ret = sendbytes(pmsg, way);/* the data should be written*/
			DEB2(printk(  "i2c-algo-bit.o: wrote %d bytes.\n",ret));
			if (ret < pmsg->len ) {
				if(ret>=0) 
				 ret = -EREMOTEIO;
				goto bailout;
			}
		}
	}
	ret =i;

bailout:
	i2c_stop(way);
	return ret;
}

int bit_receive(struct i2c_msg msgs[], int num,unsigned int way)
{
	struct i2c_msg *pmsg;
	
	int i,ret;
	unsigned short nak_ok;
	//const char *temp = msgs->buf;
	char *temp = msgs->buf;
	tgt_i2cinit(way);

	i2c_start(way);
	printf(" -----------i2c_start-->\r\n");	
	return 0;

	pmsg = &msgs[0];
			ret = bit_doAddress(pmsg,way);
		*temp = 0x0;
			i2c_outb(0,way);
//			DEB2(printk(  "i2c-algo-bit.o: wrote %d bytes.\n",ret));
			 i2c_start(way);
			*temp = 0xa1;
			i2c_outb(161,way);
			ret = readbytes(pmsg, way);
			DEB2(printk(  "i2c-algo-bit.o: read %d bytes.\n",ret));
	i2c_stop(way);
//	printf("ret:%d\n",ret);
	return ret;
}
int i2c_transfer(struct i2c_msg *msgs, int num, unsigned int way)
{
	bit_xfer(msgs, num, way);
}

#if !defined(_VXWORKS_)
int nb_i2c_send(int argc , char **argv);
int
cmd_nb_i2c_send (argc, argv)
    int argc; char **argv;
{
    int ret;
    ret = spawn ("nb_i2c", nb_i2c_send, argc, argv);
    return (ret & ~0xff) ? 1 : (signed char)ret;
}

const Optdesc         cmd_nb_i2c_opts[] =
{
	
	{"-rh", "hahahaha"},
	{"path", "path and filename"},
	{"NB I2C TEST\n"},
	{0}
};


/*
 *  Command table registration
 *  ==========================
 */
int cmd_nb_i2c_receive(int argc , char **argv)
{
	unsigned char buf[]={0x10,0xaa};
	unsigned char buf_tx[]={0xdc,0xd1,0xaa};
	unsigned char buf_rec[64]={0};
	int way=2,i=0;
	struct i2c_msg msgs[] = {
		{
			.addr	= 0x60,
			.flags	= 0,
			.len	= 2,
			.buf	= buf,
		}, {
			.addr	= 0x60,
			.flags	= I2C_M_RD,
			.len	= 2,
			.buf	= buf + 1,
		}
	};

	if(argc < 3)
	{
		printf("Too few arguments,at least 3\r\n");
		printf("1.nb_i2c\r\n");
		printf("2.address\r\n");
		printf("3.way\r\n");
		return -1;
	}
	msgs[0].addr = 0xa0;
	msgs[0].flags =0;
	msgs[0].len  =3;
	msgs[0].buf  =buf_tx; 
	
	msgs[1].addr = 0xa0;
	msgs[1].flags =0;
	msgs[1].len  =2;
	msgs[1].buf  =buf_rec;
	
	msgs[0].addr = atoi(argv[1]);
	msgs[1].addr = atoi(argv[1]);
	msgs[1].len = lenthzmz;
//	printf("msgs[0].addr:0x%x\n",msgs[0].addr);
//	printf("msgs[1].addr:0x%x\n",msgs[1].addr);
	way = atoi(argv[2]);
	printf("%s_way_%d:0x%x\n",__FUNCTION__,way, msgs[0].addr);
  if( bit_receive(&msgs[1],1,way) == lenthzmz )
  	{
		printf("the read data are:");
  		for(i=0; i < msgs[1].len;i++)
		{
			printf("%c",(unsigned char)buf_rec[i]);
			}
	printf("\n");
  	return 0;
  }
//#endif
	return -1;
}

static const Cmd Cmds[] =
{
   {" NB I2C Test"},
   {"nb_i2c","[Address][way]",
		   cmd_nb_i2c_opts,
		   "NB_I2C_TEST",
 cmd_nb_i2c_send, 1, 16, 0},
   {"i2c_read","[Address][way]",
		   cmd_nb_i2c_opts,
		   "NB_I2C_TEST",
 cmd_nb_i2c_receive, 1, 16, 0},
   {0, 0}
};

static void init_cmd __P((void)) __attribute__ ((constructor));

static void
   init_cmd()
{
   cmdlist_expand(Cmds, 1);
}
#define DDC_ADDR 0x50
int nb_i2c_send(int argc , char **argv)
{
	unsigned char buf[]={0x0,0xa2,0xa3,0xa4,0x32,};
	//unsigned char buf_tx[]={0xcc,0x01,0xaa};
	unsigned char buf_tx[]={0xdc,0xd1,0xaa};
	unsigned char buf_rec[64]={0};
	int way=2,i=0;
	struct i2c_msg msgs[] = {
		{
			.addr	= DDC_ADDR,
			.flags	= 0,
			.len	= 5,
			.buf	= buf,
		}, {
			.addr	= DDC_ADDR,
			.flags	= I2C_M_RD,
			.len	= 2,
			.buf	= buf + 1,
		}
	};

	if(argc < 4)
	{
		printf("Too few arguments,at least 3\r\n");
		printf("1.nb_i2c\r\n");
		printf("2.address\r\n");
		printf("3.way\r\n");
		printf("4.the date to be written\r\n");
		return -1;
	}
	msgs[0].addr = atoi(argv[1]);
	way = atoi(argv[2]);
	buf[1] = argv[3][0];
	msgs[0].buf = argv[3];
	if( strlen(argv[3]) > 8 )/* one time only can write 8 bytes*/
	 {
		msgs[0].len = 8;
		lenthzmz = msgs[0].len;
	 }
	else
	{
		msgs[0].len = strlen(argv[3]);
		lenthzmz = msgs[0].len;
	}
	printf("%s_way_%d:0x%x\n",__FUNCTION__,way,msgs[0].addr);
	if( bit_xfer(msgs,1,way) == 2 )
		return 0;
	return 0;
}

#endif



