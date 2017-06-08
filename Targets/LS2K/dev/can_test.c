#include <pmon.h>

#define CAN0_BASE	0xbfe00c00
#define CAN1_BASE	0xbfe00d00

#define CONFGMAC	0xbfe10420

#define readb(x) *(volatile unsigned char*)(x)
#define readw(x) *(volatile unsigned int*)(x)
#define writeb(x,y) *(volatile unsigned char*)(x) = (y)

static const can0_buf[] = {0xb1,0x28,0x11,0x22,0x33,0x44,0x55,0x66,0x77,0x88};
static const can1_buf[] = {0xa0,0x28,0x12,0x34,0x56,0x78,0x90,0x6c,0xde,0xf0};

int can_config(unsigned int base)
{
	int i;

	writeb(base + 0x0,0x1);
	writeb(base + 0x6,0x80); 
	writeb(base + 0x7,0xe);
	writeb(base + 0x1,0x0);
	if (base == CAN0_BASE)
		writeb(base + 0x4,0xa0);
	else
		writeb(base + 0x4,0xb1);
	writeb(base + 0x5,0x0);
	writeb(base + 0x0,0x40);
	writeb(base + 0x0,0x1e);

	for(i = 0;i < 10000;i++){;}

	for(i = 0;i < 10;i++){
		if (base == CAN0_BASE) {
			writeb(base + 0xa + i,can0_buf[i]);
			printf("can0 [%d] ->0x%x ",(0xa + i),readb(base + 0xa + i));
		}
		else {
			writeb(base + 0xa + i,can1_buf[i]);
			printf("can1 [%d] ->0x%x ",(0xa + i),readb(base + 0xa + i));
		}
	}
	printf("\n");

	return 0;
}

int can_rx(unsigned int base)
{
	unsigned char i, tmp, num, err = 0;

	tmp = readb(base + 0x3);
	printf("[0x3] = 0x%x\n",tmp);
		
	do {
		tmp = readb(base + 0x3);
	printf("[0x3] = 0x%x\n",tmp);
	}while(!(tmp & 0x1));

	tmp = readb(base + 0x14);
	if (base == CAN0_BASE)
		if ((tmp ^ 0xa0) != 0)
			printf("ID ERROR!\n");
	else
		if ((tmp ^ 0xb1) != 0)
			printf("ID ERROR!\n");

	num = readb(base + 0x15);
	num &= 0xf;
	printf("received num is %d\n",num);
	for (i = 0;i < num;i++){
		tmp = readb(base + 0x16 + i);
		if (base == CAN0_BASE)
			if (tmp != can1_buf[i + 2]){
				err++;
				printf("the err value [%d]-> %d\n", i, tmp);
			}
		else
			if (tmp != can0_buf[i + 2]){
				err++;
				printf("the err value [%d]-> %d\n", i, tmp);
			}
	}
	printf("\n");

	if (err)
		printf("ERROR!\n");
	else
		printf("PASS!\n");
	return 0;
}
//can1 to can0,can0 to can1;
void can_test(void)
{	
	//set gpio to can mode
	readw(CONFGMAC) |= (3 << 16);	
	
	printf("can config begin!\n");
	can_config(CAN0_BASE);
	can_config(CAN1_BASE);
	writeb(CAN1_BASE + 0x1,0x1);
	printf("can test begin!\n");
	printf("can 0 rx > > >\n");
	can_rx(CAN0_BASE);
	writeb(CAN1_BASE + 0x1,0x0);
	writeb(CAN0_BASE + 0x1,0x1);
	printf("can 1 rx > > >\n");
	can_rx(CAN1_BASE);
}

void can1_selftest()
{
	unsigned char can_ext_code[4]    = {0xa6,0x01,0x5a,0xa8};
	unsigned char can_ext_mask[4]    = {0xff,0xff,0xff,0xff};
	unsigned char can_ext_header[5]  = {0x88,0xa6,0x01,0x5a,0xa8};
	unsigned char can_ext_messege[8] = {0x12,0x23,0x34,0x45,0x56,0x67,0x78,0x89};
	unsigned int rdata;
	unsigned int i,id;


	readw(CONFGMAC) |= (3 << 16);	

	readb(CAN1_BASE + 0x0)= 0x1;
	readb(CAN1_BASE + 0x6)= 0x80;
	readb(CAN1_BASE + 0x7)= 0x4a;
	readb(CAN1_BASE + 0x1)= 0x80;
	readb(CAN1_BASE + 0x4)= 0xff;
	readb(CAN1_BASE + 0x5)= 0xff;

	for(i=0;i<4;i++)
	{readb(CAN1_BASE + 0x10 + i)= can_ext_code[i];
	    rdata =  readb(CAN1_BASE + 0x10 + i);
	    printf("verification code%0x: %0x\n",i,rdata);
	}

	for(i=0;i<4;i++)
	{readb(CAN1_BASE + 0x14 + i)= can_ext_mask[i];
	    rdata =  readb(CAN1_BASE + 0x14 + i);
	    printf("verification mask%0x: %0x\n",i,rdata);
	}

	for(i=0;i<1000;i++)
    	  {;}
	readb(CAN1_BASE + 0x0)= 0x4;
	printf("enable self and ext mode\n");

	for(i=0;i<1000;i++)
	    {;}

	for(i=0;i<5;i++)
	{readb(CAN1_BASE + 0x10 + i)= can_ext_header[i];
	    printf("can_ext_header is: %0x\n",can_ext_header[i]);
	}

	for(i=0;i<8;i++)
	{readb(CAN1_BASE + 0x15 + i)= can_ext_messege[i];
	    printf("can_ext_messege is: %0x\n",can_ext_messege[i]);
	}

	readb(CAN1_BASE + 0x1)= 0x10;


	readb(CAN1_BASE + 0x1)= 0x1;

	for(i=0;i<1000;i++)
	    {;}

	printf("can0 self tx done !\n");
}

static void can1_rx_read()
{
  unsigned int i,rdata;

    for(i=0;i<8;i++)
    {
      rdata = readb(CAN1_BASE + 0x15 + i);
      printf("can rx 0x%0x\n",rdata);
    }
}
void can_selftest()
{
	unsigned char can_ext_code[4]    = {0xa6,0x01,0x5a,0xa8};
	unsigned char can_ext_mask[4]    = {0xff,0xff,0xff,0xff};
	unsigned char can_ext_header[5]  = {0x88,0xa6,0x01,0x5a,0xa8};
	unsigned char can_ext_messege[8] = {0x11,0x22,0x33,0x44,0x55,0x66,0x77,0x88};
	unsigned int rdata;
	unsigned int i,id;

	readw(CONFGMAC) |= (3 << 16);	

	readb(CAN0_BASE + 0x0)= 0x1;
	readb(CAN0_BASE + 0x6)= 0x80;
	readb(CAN0_BASE + 0x7)= 0x4a;
	readb(CAN0_BASE + 0x1)= 0x80;
	readb(CAN0_BASE + 0x4)= 0xff;
	readb(CAN0_BASE + 0x5)= 0xff;

	for(i=0;i<4;i++)
	{readb(CAN0_BASE + 0x10 + i)= can_ext_code[i];
	    rdata =  readb(CAN0_BASE + 0x10 + i);
	    printf("verification code%0x: %0x\n",i,rdata);
	}

	for(i=0;i<4;i++)
	{readb(CAN0_BASE + 0x14 + i)= can_ext_mask[i];
	    rdata =  readb(CAN0_BASE + 0x14 + i);
	    printf("verification mask%0x: %0x\n",i,rdata);
	}

	for(i=0;i<1000;i++)
    	  {;}
	readb(CAN0_BASE + 0x0)= 0x4;
	printf("enable self and ext mode\n");

	for(i=0;i<1000;i++)
	    {;}

	for(i=0;i<5;i++)
	{readb(CAN0_BASE + 0x10 + i)= can_ext_header[i];
	    printf("can_ext_header is: %0x\n",can_ext_header[i]);
	}

	for(i=0;i<8;i++)
	{readb(CAN0_BASE + 0x15 + i)= can_ext_messege[i];
	    printf("can_ext_messege is: %0x\n",can_ext_messege[i]);
	}

	readb(CAN0_BASE + 0x1)= 0x10;


	readb(CAN0_BASE + 0x1)= 0x1;

	for(i=0;i<1000;i++)
	    {;}
#if 0
	while(~(rdata & 0x1))
	{rdata =  readb(CAN0_BASE + 0x3 + i);}

	while(~(rdata & 0x1))
	{rdata =  readb(CAN0_BASE + 0x3);}
	while(~(rdata & 0x1))
	{rdata =  readb(CAN0_BASE + 0x3);
	    printf("rdata is: %0x\n",rdata);
	}
#endif

	printf("can0 self tx done !\n");
}

static void can_rx_read()
{
  unsigned int i,rdata;

    for(i=0;i<8;i++)
    {
      rdata = readb(CAN0_BASE + 0x15 + i);
      printf("can rx 0x%0x\n",rdata);
    }
}

static void can_cfg_mask()
{
	unsigned int rdata;
	unsigned int i;

	readw(CONFGMAC) |= (3 << 16);	

	readb(CAN0_BASE + 0x0)= 0x1;
//	readb(CAN0_BASE + 0x6)= 0x80;//33M
//	readb(CAN0_BASE + 0x6)= 0x81;//66M
//	readb(CAN0_BASE + 0x6)= 0x82;//99M
	readb(CAN0_BASE + 0x6)= 0x83;//132M
//	readb(CAN0_BASE + 0x6)= 0xc3;
	readb(CAN0_BASE + 0x7)= 0x3a;
	readb(CAN0_BASE + 0x1)= 0x0;
	readb(CAN0_BASE + 0x4)= 0xea;
	readb(CAN0_BASE + 0x5)= 0;
	
	for(i=0;i<1000;i++)
	  {;}
	readb(CAN0_BASE + 0x0)= 0x40;
	for(i=0;i<1000;i++)
	  {;}
	readb(CAN0_BASE + 0x0)= 0x1e;
	
	printf("can0 config done !\n");
}
static void can_cfg()
{
	unsigned int rdata;
	unsigned int i;

	readw(CONFGMAC) |= (3 << 16);	

	readb(CAN0_BASE + 0x0)= 0x1;
//	readb(CAN0_BASE + 0x6)= 0x80;
//	readb(CAN0_BASE + 0x6)= 0x81;
//	readb(CAN0_BASE + 0x6)= 0x82;//99M
	readb(CAN0_BASE + 0x6)= 0x83;//132M
//	readb(CAN0_BASE + 0x7)= 0x49;
	readb(CAN0_BASE + 0x7)= 0x3a;
	readb(CAN0_BASE + 0x1)= 0x0;
	readb(CAN0_BASE + 0x4)= 0xea;
	readb(CAN0_BASE + 0x5)= 0xff;
	
	for(i=0;i<1000;i++)
	  {;}
	readb(CAN0_BASE + 0x0)= 0x40;
	for(i=0;i<1000;i++)
	  {;}
	readb(CAN0_BASE + 0x0)= 0x1e;
	
	printf("can0 config done !\n");
}

static void can1_cfg()
{
	unsigned int rdata;
	unsigned int i;

	readw(CONFGMAC) |= (3 << 16);	

	readb(CAN1_BASE + 0x0)= 0x1;
	readb(CAN1_BASE + 0x6)= 0x80;
	readb(CAN1_BASE + 0x7)= 0x3a;
	readb(CAN1_BASE + 0x1)= 0x0;
	readb(CAN1_BASE + 0x4)= 0xee;
	readb(CAN1_BASE + 0x5)= 0xff;
	
	for(i=0;i<1000;i++)
	  {;}
	readb(CAN1_BASE + 0x0)= 0x40;
	for(i=0;i<1000;i++)
	  {;}
	readb(CAN1_BASE + 0x0)= 0x1e;
	
	printf("can0 config done !\n");
}
static void can_tx_test()
{
	unsigned int i;
	static int messege_test[] = {0xea,0x28,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55};

	for(i=0;i<10;i++)
	{
	  readb(CAN0_BASE + 0xa + i)= messege_test[i];
	    if(i>1)
	    printf("can tx 0x%0x\n",messege_test[i]);
	}

	for(i=0;i<1000;i++)
	  {;}

	readb(CAN0_BASE + 0x1)= 0x1;

	printf("can0 tx now !\n");
}

static void can1_tx_test()
{
	unsigned int i;
	static int messege_test[] = {0xee,0x28,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55};

	for(i=0;i<10;i++)
	{
	  readb(CAN1_BASE + 0xa + i)= messege_test[i];
	    if(i>1)
	    printf("can tx 0x%0x\n",messege_test[i]);
	}

	for(i=0;i<1000;i++)
	  {;}

	readb(CAN1_BASE + 0x1)= 0x1;

	printf("can0 tx now !\n");
}

static void can_rx_test()
{
  unsigned int i,rdata;

    for(i=0;i<8;i++)
    {
      rdata = readb(CAN0_BASE + 0x16 + i);
      printf("can rx 0x%0x\n",rdata);
    }
}

static void can1_rx_test()
{
  unsigned int i,rdata;

    for(i=0;i<8;i++)
    {
      rdata = readb(CAN1_BASE + 0x16 + i);
      printf("can rx 0x%0x\n",rdata);
    }
}

static const Cmd Cmds[] = {
	{"ls2k can"},
	{"can", "", 0, "test the can function", can_test, 1, 99, 0},
	{"can_sel", "", 0, "test the can function", can_selftest, 1, 99, 0},
	{"can1_sel", "", 0, "test the can function", can1_selftest, 1, 99, 0},
	{"can_rx", "", 0, "test the can function", can_rx_read, 1, 99, 0},
	{"can1_rx", "", 0, "test the can function", can1_rx_read, 1, 99, 0},
	{"can_cfg", "", 0, "test the can function", can_cfg, 1, 99, 0},
	{"can_cfg_mask", "", 0, "test the can function", can_cfg_mask, 1, 99, 0},
	{"can_tx_test", "", 0, "test the can function", can_tx_test, 1, 99, 0},
	{"can1_cfg", "", 0, "test the can function", can1_cfg, 1, 99, 0},
	{"can1_tx_test", "", 0, "test the can function", can1_tx_test, 1, 99, 0},
	{"can_rx_test", "", 0, "test the can function", can_rx_test, 1, 99, 0},
	{"can1_rx_test", "", 0, "test the can function", can1_rx_test, 1, 99, 0},
	{0, 0}
};

static void init_cmd __P((void)) __attribute__ ((constructor));
static void init_cmd()
{
	cmdlist_expand(Cmds, 1);
}
