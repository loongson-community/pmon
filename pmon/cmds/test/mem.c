#if 0
extern char           *heaptop;
static int
allmemtst()
{
	int cflag, vflag, err;
	u_int32_t saddr, eaddr;

	saddr = heaptop;
        eaddr = (memorysize & 0x3fffffff) + ((int)heaptop & 0xc0000000);
	asm("
	li $2,-1
	li $3,%0
1:
	sd $2,($3)
	daddu $3,8
	bne $3,%1,1b
	nop
	");
	
}
#endif

static int memtest()
{
	char cmd[20];
	printf("memory test\n");
	strcpy(cmd,"mt");
	do_cmd(cmd);
	return 0;
}
