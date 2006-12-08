#define nr_printf printf
#define nr_gets gets
#define nr_strtol strtoul
//-------------------------------------------PNP------------------------------------------
// MB PnP configuration register

#define PNP_KEY_ADDR (PCI_IO_SPACE_BASE+0x3f0)
#define PNP_DATA_ADDR (PCI_IO_SPACE_BASE+0x3f1)


void PNPSetConfig(char Index, char data);
char PNPGetConfig(char Index);

#define SUPERIO_CFG_REG 0x85

void EnterMBPnP(void)
{
	pcitag_t tag;
	char confval;
	tag=_pci_make_tag(VTSB_BUS,VTSB_DEV, VTSB_ISA_FUNC);
	confval=_pci_conf_readn(tag,SUPERIO_CFG_REG,1);
	_pci_conf_writen(tag,SUPERIO_CFG_REG,confval|2,1);	
}

void ExitMBPnP(void)
{
	pcitag_t tag;
	char confval,val;
	tag=_pci_make_tag(VTSB_BUS,VTSB_DEV, VTSB_ISA_FUNC);
	confval=_pci_conf_readn(tag,SUPERIO_CFG_REG,1);
	_pci_conf_writen(tag,SUPERIO_CFG_REG,confval&~2,1);	
}

void PNPSetConfig(char Index, char data)
{
        EnterMBPnP();                                // Enter IT8712 MB PnP mode
        outb(PNP_KEY_ADDR,Index);
        outb(PNP_DATA_ADDR,data);
        ExitMBPnP();
}

char PNPGetConfig(char Index)
{
        char rtn;

        EnterMBPnP();                                // Enter IT8712 MB PnP mode
        outb(PNP_KEY_ADDR,Index);
        rtn = inb(PNP_DATA_ADDR);
        ExitMBPnP();
        return rtn;
}


static int PnpRead(int argc,char **argv)
{
	unsigned char Index,data;
		if(argc!=2){return -1;}
		
		Index=nr_strtol(argv[1],0,0);
data=PNPGetConfig(Index);
nr_printf("pnpread index=0x%02x,value=0x%02x\n",Index,data);
return 0;
}

static int PnpWrite(int argc,char **argv)
{
        unsigned char Index,data;
        if(argc!=3){return -1;}
		Index=nr_strtol(argv[1],0,0);
		data=nr_strtol(argv[2],0,0);
PNPSetConfig(Index,data);
nr_printf("pnpwrite index=0x%02x,value=0x%02x,",Index,data);
data=PNPGetConfig(Index);
nr_printf("result=0x%02x\n",data);
return 0;
}

int dumpsis(int argc,char **argv)
{
int i;
volatile unsigned char *p=0xbfd003c4;
unsigned char c;
for(i=0;i<0x15;i++)
{
p[0]=i;
c=p[1];
printf("sr%x=0x%02x\n",i,c);
}
p[0]=5;
p[1]=0x86;
printf("after set 0x86 to sr5\n");
for(i=0;i<0x15;i++)
{
p[0]=i;
c=p[1];
printf("sr%x=0x%02x\n",i,c);
}
return 0;
}

unsigned char i2cread(char slot,char offset);

int cmd_i2cread(int argc,char **argv)
{
int addr,slot;
unsigned char c;
int count,i;
 if(argc!=4)return -1;
 slot=strtoul(argv[1],0,0);
 addr=strtoul(argv[2],0,0);
 count=strtoul(argv[3],0,0);
 for(i=0;i<count;i++,addr++)
 {
	 if(i%16==0)printf("\n%02x:",addr);
	 printf(" %02x",i2cread((slot<<1)+0xa1,addr));
 }
 printf("\n");
 return 0;
}

static const Cmd Cmds[] =
{
	{"MyCmds"},
	{"pnpr",	"LDN index", 0, "pnpr LDN(logic device NO) index", PnpRead, 0, 99, CMD_REPEAT},
	{"pnpw",	"LDN index value", 0, "pnpw index value", PnpWrite, 0, 99, CMD_REPEAT},
	{"dumpsis",	"", 0, "dump sis registers", dumpsis, 0, 99, CMD_REPEAT},
	{"i2cread",	"slot offset count", 0, "read i2c info", cmd_i2cread, 0, 99, CMD_REPEAT},
	{0, 0}
};

static void init_cmd __P((void)) __attribute__ ((constructor));

static void
init_cmd()
{
	cmdlist_expand(Cmds, 1);
}

