#include <stdio.h>
//#include "include/fcr.h"
#include <stdlib.h>
#include <ctype.h>
#undef _KERNEL
#include <errno.h>
#include <pmon.h>
//#include <include/types.h>
#include <pflash.h>
#include <fcntl.h>

#define ls7a_spi_BASE  (*(volatile unsigned int *)(0xba000010 | (22 << 11)) & 0xfffffff0 | 0xc0000000)

#define SPCR      0x0
#define SPSR      0x1
#define TXFIFO    0x2
#define RXFIFO    0x2
#define SPER      0x3
#define PARAM     0x4
#define SOFTCS    0x5
#define PARAM2    0x6

#define RFEMPTY 1
#define write8(addr,val)	 *(volatile char *)(addr) = val
#define read8(addr)	 *(volatile char *)(addr)

#define SET_SPI(addr,val)        write8(ls7a_spi_BASE+addr,val)
#define GET_SPI(addr)            read8(ls7a_spi_BASE+addr)

#define VBIOS_SIZE 0x1E000
unsigned int POLYNOMIAL = 0xEDB88320 ;
int have_table = 0 ;
unsigned int table[256] ;

unsigned char mac_read_spi_buf[22] = {0};

int ls7a_spi_write_sr(char val);
void ls7a_spi_initw()
{
	SET_SPI(SPSR, 0xc0);
	SET_SPI(PARAM, 0x40);             //espr:0100
	SET_SPI(SPER, 0x05); //spre:01
	SET_SPI(PARAM2,0x01);
	SET_SPI(SPCR, 0x50);
}

void ls7a_spi_initr()
{
	SET_SPI(PARAM, 0x47);             //espr:0100
}




///////////////////read status reg /////////////////

int ls7a_spi_read_sr(void)
{
	int val;

	SET_SPI(SOFTCS,0x01);
	SET_SPI(TXFIFO,0x05);
	while((GET_SPI(SPSR))&RFEMPTY);

	val = GET_SPI(RXFIFO);
	SET_SPI(TXFIFO,0x00);

	while((GET_SPI(SPSR))&RFEMPTY == RFEMPTY);

	val = GET_SPI(RXFIFO);

	SET_SPI(SOFTCS,0x11);

	return val;
}


////////////set write enable//////////
int ls7a_spi_set_wren(void)
{
	int res;

	res = ls7a_spi_read_sr();
	while(res&0x01 == 1)
	{
		res = ls7a_spi_read_sr();
	}

	SET_SPI(SOFTCS,0x01);

	SET_SPI(TXFIFO,0x6);
	while((GET_SPI(SPSR))&RFEMPTY == RFEMPTY){
	}
	GET_SPI(RXFIFO);

	SET_SPI(SOFTCS,0x11);

	return 1;
}

///////////////////////write status reg///////////////////////
int ls7a_spi_write_sr(char val)
{
	int res;

	ls7a_spi_set_wren();

	res = ls7a_spi_read_sr();
	while(res&0x01 == 1)
	{
		res = ls7a_spi_read_sr();
	}

	SET_SPI(SOFTCS,0x01);

	SET_SPI(TXFIFO,0x01);
	while((GET_SPI(SPSR))&RFEMPTY == RFEMPTY){
	}
	GET_SPI(RXFIFO);

	SET_SPI(TXFIFO,val);
	while((GET_SPI(SPSR))&RFEMPTY == RFEMPTY){
	}
	GET_SPI(RXFIFO);
	SET_SPI(SOFTCS,0x11);

	return 1;

}

///////////erase all memory/////////////
int ls7a_spi_erase_all(void)
{
	int res;
	int i=1;
	ls7a_spi_initw();
	ls7a_spi_set_wren();
	res = ls7a_spi_read_sr();
	while(res&0x01 == 1)
	{
		res = ls7a_spi_read_sr();
	}
	SET_SPI(SOFTCS,0x1);

	SET_SPI(TXFIFO,0xC7);
	while((GET_SPI(SPSR))&RFEMPTY == RFEMPTY){
	}
	GET_SPI(RXFIFO);

	SET_SPI(SOFTCS,0x11);
	while(i++){
		if(ls7a_spi_read_sr() & 0x1 == 0x1){
			if(i % 10000 == 0)
				printf(".");
		}else{
			printf("done...\n");
			break;
		}
	}
	return 1;
}



void ls7a_spi_read_id(void)
{
	unsigned char val;
	ls7a_spi_initw();
	val = ls7a_spi_read_sr();
	while(val&0x01 == 1)
	{
		val = ls7a_spi_read_sr();
	}
	/*CE 0*/
	SET_SPI(SOFTCS,0x01);
	/*READ ID CMD*/
	SET_SPI(TXFIFO,0x9f);
	while((GET_SPI(SPSR))&RFEMPTY == RFEMPTY){
	}
	GET_SPI(RXFIFO);

	/*Manufacturer’s ID*/
	SET_SPI(TXFIFO,0x00);
	while((GET_SPI(SPSR))&RFEMPTY == RFEMPTY){
	}
	val = GET_SPI(RXFIFO);
	printf("Manufacturer's ID:         %x\n",val);

	/*Device ID:Memory Type*/
	SET_SPI(TXFIFO,0x00);
	while((GET_SPI(SPSR))&RFEMPTY == RFEMPTY){
	}
	val = GET_SPI(RXFIFO);
	printf("Device ID-memory_type:     %x\n",val);

	/*Device ID:Memory Capacity*/
	SET_SPI(TXFIFO,0x00);
	while((GET_SPI(SPSR))&RFEMPTY == RFEMPTY){
	}
	val = GET_SPI(RXFIFO);
	printf("Device ID-memory_capacity: %x\n",val);

	/*CE 1*/
	SET_SPI(SOFTCS,0x11);

}

void write_byte(unsigned int addr,unsigned char data)
{
	/*byte_program,CE 0, cmd 0x2,addr2,addr1,addr0,data in,CE 1*/
	unsigned char addr2,addr1,addr0;
	unsigned char val;
	addr2 = (addr & 0xff0000)>>16;
	addr1 = (addr & 0x00ff00)>>8;
	addr0 = (addr & 0x0000ff);
	ls7a_spi_set_wren();
	val = ls7a_spi_read_sr();
	while(val&0x01 == 1)
	{
		val = ls7a_spi_read_sr();
	}
	SET_SPI(SOFTCS,0x01);/*CE 0*/

	SET_SPI(TXFIFO,0x2);/*byte_program */
	while((GET_SPI(SPSR))&RFEMPTY == RFEMPTY){
	}
	val = GET_SPI(RXFIFO);

	/*send addr2*/
	SET_SPI(TXFIFO,addr2);
	while((GET_SPI(SPSR))&RFEMPTY == RFEMPTY){
	}
	val = GET_SPI(RXFIFO);

	/*send addr1*/
	SET_SPI(TXFIFO,addr1);
	while((GET_SPI(SPSR))&RFEMPTY == RFEMPTY){
	}
	val = GET_SPI(RXFIFO);

	/*send addr0*/
	SET_SPI(TXFIFO,addr0);
	while((GET_SPI(SPSR))&RFEMPTY == RFEMPTY){
	}
	val = GET_SPI(RXFIFO);

	/*send data(one byte)*/
	SET_SPI(TXFIFO,data);
	while((GET_SPI(SPSR))&RFEMPTY == RFEMPTY){
	}
	val = GET_SPI(RXFIFO);

	/*CE 1*/
	SET_SPI(SOFTCS,0x11);

}
int ls7a_spi_write_byte(int argc,char ** argv)
{
	unsigned int addr;
	unsigned char val;
	if(argc != 3){
		printf("\nuse: ls7a_spi_write_byte  dst(flash addr) data\n");
		return -1;
	}
	addr = strtoul(argv[1],0,0);
	val = strtoul(argv[2],0,0);
	write_byte(addr,val);
	return 0;

}

int ls7a_spi_erase_area(unsigned int saddr,unsigned int eaddr,unsigned sectorsize)
{
	unsigned int addr;
	ls7a_spi_initw();

	for(addr=saddr;addr<eaddr;addr+=sectorsize)
	{

		SET_SPI(SOFTCS,0x11);

		ls7a_spi_set_wren();

		ls7a_spi_write_sr(0x00);

		while(ls7a_spi_read_sr()&1);

		ls7a_spi_set_wren();

		SET_SPI(SOFTCS,0x01);

		/*
		* 0x20 erase 4kbyte of memory array
		* 0x52 erase 32kbyte of memory array
		* 0xd8 erase 64kbyte of memory array
		*/
		SET_SPI(TXFIFO,0x20);
		while((GET_SPI(SPSR))&RFEMPTY);
		GET_SPI(RXFIFO);
		SET_SPI(TXFIFO,addr >> 16);

		while((GET_SPI(SPSR))&RFEMPTY);
		GET_SPI(RXFIFO);

		SET_SPI(TXFIFO,addr >> 8);
		while((GET_SPI(SPSR))&RFEMPTY);
		GET_SPI(RXFIFO);

		SET_SPI(TXFIFO,addr);

		while((GET_SPI(SPSR))&RFEMPTY);
		GET_SPI(RXFIFO);

		SET_SPI(SOFTCS,0x11);

		while(ls7a_spi_read_sr()&1);
	}
	SET_SPI(SOFTCS,0x11);
	delay(10);

	return 0;
}



int ls7a_spi_write(int argc,char **argv)
{
	long int j=0;
	unsigned char val;
	unsigned int flashaddr,size,write_spi_size;
	unsigned char * ramaddr;
	char path[256];
	int fd;
	int readsize = 512 *1024;
	int n2;
	int count = 0;

	ramaddr = 0x80200000;

	if(argc != 4){
		printf("\nuse: ls7a_spi_write src(file) dst(flash addr) size\n");
		return -1;
	}

	strcpy(path,argv[1]);
	flashaddr = strtoul(argv[2],0,0);
	size = strtoul(argv[3],0,0);

	if (flashaddr + size > 0x100000) {
		printf("flashaddr + size is more than 1M\n");
		return EXIT_FAILURE;
	}
	if ((fd = open (path, O_RDONLY | O_NONBLOCK)) < 0) {
		perror (path);
		return EXIT_FAILURE;
	}

	do {
		n2 = read (fd, ramaddr, readsize);
		ramaddr = (ramaddr + n2);
		count += n2;
	} while (n2 >= readsize);
	ramaddr = 0x80200000;
	printf("\nLoaded %d bytes\n", count);
	close(fd);

	if(count < size)
		write_spi_size = count;
	else
		write_spi_size = size;

	ls7a_spi_initw();

	ls7a_spi_write_sr(0);
	// read flash id command
	ls7a_spi_read_id();
	val = GET_SPI(SPSR);
	printf("====spsr value:%x\n",val);

	SET_SPI(0x5,0x10);
	// erase the flash
	ls7a_spi_write_sr(0x00);
	ls7a_spi_erase_area(flashaddr, flashaddr + size, 0x1000);
	printf("\nfrom ram 0x%08x  to flash 0x%08x size 0x%08x \n\nprogramming            ",ramaddr,flashaddr,size);
	for(j=0;write_spi_size > 0;flashaddr++,ramaddr++,write_spi_size--,j++)
	{
		write_byte(flashaddr,*((unsigned char*)ramaddr));
		if(j % 0x1000 == 0)
		printf("\b\b\b\b\b\b\b\b\b\b0x%08x",j);
	}
	printf("\b\b\b\b\b\b\b\b\b\b0x%08x end...\n",j);

	SET_SPI(0x5,0x11);
	return 1;
}

int ls7a_spi_read_byte(unsigned int addr,unsigned int num)
{
	unsigned char val,data;
	val = ls7a_spi_read_sr();
	while(val&0x01 == 1)
	{
	val = ls7a_spi_read_sr();
	}

	SET_SPI(0x5,0x01);
	// read flash command
	SET_SPI(TXFIFO,0x03);
	while((GET_SPI(SPSR))&RFEMPTY == RFEMPTY){
	}
	GET_SPI(RXFIFO);

	// addr
	SET_SPI(TXFIFO,0x00);
	while((GET_SPI(SPSR))&RFEMPTY == RFEMPTY){
	}
	GET_SPI(RXFIFO);

	SET_SPI(TXFIFO,0x00);
	while((GET_SPI(SPSR))&RFEMPTY == RFEMPTY){
	}
	GET_SPI(RXFIFO);

	SET_SPI(TXFIFO,0x00);
	while((GET_SPI(SPSR))&RFEMPTY == RFEMPTY){
	}
	GET_SPI(RXFIFO);


	SET_SPI(TXFIFO,0x00);
	while((GET_SPI(SPSR))&RFEMPTY == RFEMPTY){
	}
	data = GET_SPI(RXFIFO);
	SET_SPI(0x5,0x11);
	return data;
}


void make_table()
{
	int i, j;
	have_table = 1 ;
	for (i = 0 ; i < 256 ; i++)
		for (j = 0, table[i] = i ; j < 8 ; j++)
			table[i] = (table[i]>>1)^((table[i]&1)?POLYNOMIAL:0) ;
}


uint lscrc32(uint crc, char *buff, int len)
{
	int i;
	if (!have_table) make_table();
	crc = ~crc;
	for (i = 0; i < len; i++)
		crc = (crc >> 8) ^ table[(crc ^ buff[i]) & 0xff];
	return ~crc;
}

int ls7a_vgabios_crc_check(unsigned char * vbios){
	unsigned int crc;

	crc = lscrc32(0,(unsigned char *)vbios, VBIOS_SIZE - 0x4);
	if(*(unsigned int *)((unsigned char *)vbios + VBIOS_SIZE - 0x4) != crc){
		printf("VBIOS crc check is wrong,use default setting!\n");
		return 1;
	}
	return 0;
}

void ls7a_spi_read_vgabios(unsigned char * buf)
{
	unsigned char addr2,addr1,addr0;
	unsigned char data;
	int val,base=0;
	int addr;
	int i;

	addr = 0x1000;
	i = 0x1000 * 32;
	ls7a_spi_initw();
	val = ls7a_spi_read_sr();
	while(val&0x01 == 1)
	{
		val = ls7a_spi_read_sr();
	}

	SET_SPI(0x5,0x01);
	// read flash command
	SET_SPI(TXFIFO,0x03);
	while((GET_SPI(SPSR))&RFEMPTY == RFEMPTY){

	}
	GET_SPI(RXFIFO);

	// addr
	SET_SPI(TXFIFO,((addr >> 16)&0xff));
	while((GET_SPI(SPSR))&RFEMPTY == RFEMPTY){

	}
	GET_SPI(RXFIFO);

	SET_SPI(TXFIFO,((addr >> 8)&0xff));
	while((GET_SPI(SPSR))&RFEMPTY == RFEMPTY){

	}
	GET_SPI(RXFIFO);

	SET_SPI(TXFIFO,(addr & 0xff));
	while((GET_SPI(SPSR))&RFEMPTY == RFEMPTY){

	}
	GET_SPI(RXFIFO);
	// addr end

	while(i--)
	{
		SET_SPI(TXFIFO,0x00);
		while((GET_SPI(SPSR))&RFEMPTY == RFEMPTY){
		}
		buf[base++] = GET_SPI(RXFIFO);
	}
	SET_SPI(SOFTCS,0x11);

}

int ls7a_spi_read(int argc,char **argv)
{
	unsigned char addr2,addr1,addr0;
	unsigned char data;
	int val,base=0;
	int addr;
	int i;
	if(argc != 3)
	{
		printf("\nuse: ls7a_spi_read addr(flash) size\n");
		return -1;
	}
	addr = strtoul(argv[1],0,0);
	i = strtoul(argv[2],0,0);
	ls7a_spi_initw();
	val = ls7a_spi_read_sr();
	while(val&0x01 == 1)
	{
		val = ls7a_spi_read_sr();
	}

	SET_SPI(0x5,0x01);
	// read flash command
	SET_SPI(TXFIFO,0x03);
	while((GET_SPI(SPSR))&RFEMPTY == RFEMPTY){

	}
	GET_SPI(RXFIFO);

	// addr
	SET_SPI(TXFIFO,((addr >> 16)&0xff));
	while((GET_SPI(SPSR))&RFEMPTY == RFEMPTY){

	}
	GET_SPI(RXFIFO);

	SET_SPI(TXFIFO,((addr >> 8)&0xff));
	while((GET_SPI(SPSR))&RFEMPTY == RFEMPTY){

	}
	GET_SPI(RXFIFO);

	SET_SPI(TXFIFO,(addr & 0xff));
	while((GET_SPI(SPSR))&RFEMPTY == RFEMPTY){

	}
	GET_SPI(RXFIFO);
	// addr end


	printf("\n");
	while(i--)
	{
		SET_SPI(TXFIFO,0x00);
		while((GET_SPI(SPSR))&RFEMPTY == RFEMPTY){
		}
		data = GET_SPI(RXFIFO);
		if(base % 16 == 0 ){
			printf("0x%08x    ",base);
		}
		printf("%02x ",data);
		if(base % 16 == 7)
			printf("  ");
		if(base % 16 == 15)
			printf("\n");
		base++;
	}
	printf("\n");
	SET_SPI(SOFTCS,0x11);
	return 1;

}

int ls7a_spi_write_area(int flashaddr,char *buffer,int size)
{
	int j;
	ls7a_spi_initw();
	SET_SPI(0x5,0x10);
	ls7a_spi_write_sr(0x00);
	for(j=0;size > 0;flashaddr++,size--,j++)
	{
		write_byte(flashaddr,buffer[j]);
		dotik(32, 0);
	}

	SET_SPI(SOFTCS,0x11);
	delay(10);
	return 0;
}


int ls7a_spi_read_area(int flashaddr,char *buffer,int size)
{
	int i;
	ls7a_spi_initw();

	SET_SPI(SOFTCS,0x01);

	SET_SPI(TXFIFO,0x03);

	while((GET_SPI(SPSR))&RFEMPTY);
	GET_SPI(RXFIFO);

	SET_SPI(TXFIFO,flashaddr>>16);
	while((GET_SPI(SPSR))&RFEMPTY);
	GET_SPI(RXFIFO);

	SET_SPI(TXFIFO,flashaddr>>8);
	while((GET_SPI(SPSR))&RFEMPTY);
	GET_SPI(RXFIFO);

	SET_SPI(TXFIFO,flashaddr);
	while((GET_SPI(SPSR))&RFEMPTY);
	GET_SPI(RXFIFO);


	for(i=0;i<size;i++)
	{
		SET_SPI(TXFIFO,0);
		while((GET_SPI(SPSR))&RFEMPTY);
		buffer[i] = GET_SPI(RXFIFO);
	}

	SET_SPI(SOFTCS,0x11);
	delay(10);
	return 0;
}

void ls7a_spi_read_mac(unsigned char * Inbuf,int num){

	unsigned char * buf;
	int i,v,j;
	if(num == 0){
		ls7a_spi_initw();
		ls7a_spi_read_area(0x0,Inbuf,22);
	}
	if(num == 0){
		buf = Inbuf;
	}else if(num == 1){
		buf = Inbuf + 16;
	}
	if (!is_valid_ether_addr_linux(buf)){
		printf("syn%d Mac is invalid, now get a new mac\n",num);
		generate_mac_val(buf);
		printf("set syn%d  Mac address: ",num);
		for (v = 0;v < 6;v++)
			printf("%2x%s",*(buf + v) & 0xff,(5-v)?":":" ");
		printf("\n");
		printf("syn%d Mac is invalid, please use set_mac to update spi mac address\n",num);
	} else {
		printf("syn%d Mac address: ",num);
		for (j = 0; j < 6; j++)
			printf("%02x%s", buf[j], (5-j)?":":" ");
		printf("\n");
	}

}
int cmd_set_mac(int ac, unsigned char *av[])
{
	int i, j, v, count, num, param = 0;
	unsigned char *s = NULL;
	unsigned int data_addr;
	unsigned char  buf[32] = {0};

	if (av[2]) s = av[2];
	else goto warning;

	count = strlen(s) / 3 + 1;
	if (count - 6) goto warning;

	for (i = 0; i < count; i++) {
		gethex(&v, s, 2);
		buf[i] = v;
		s += 3;
	}

	ls7a_spi_initw();
	ls7a_spi_read_area(0x0,mac_read_spi_buf,22);

	data_addr = strtoul(av[1] + 3, NULL, 0);

	if(data_addr == 0)
		memcpy(mac_read_spi_buf,buf, 6);
	else if(data_addr == 1)
		memcpy((mac_read_spi_buf + 16),buf, 6);

	ls7a_spi_erase_area(0x0,0x1000,0x1000);
	ls7a_spi_write_area(0x0,mac_read_spi_buf,22);
	printf("set syn%d  Mac address: %s\n",data_addr / 6, av[2]);
	printf("The machine should be restarted to make the mac change to take effect!!\n");
	return 0;


warning:
	printf("Please accord to correct format.\nFor example:\n");
	printf("\tsetmac syn1 \"00:11:22:33:44:55\"\n");
	printf("\tThis means set syn1's Mac address 00:11:22:33:44:55\n");
	return 0;
}


static const Cmd Cmds[] =
{
	{"MyCmds"},
	{"ls7a_spi_initw","",0,"spi_initw()",ls7a_spi_initw,0,99,CMD_REPEAT},
	{"ls7a_spi_read","",0,"ls7a_spi_read()",ls7a_spi_read,0,99,CMD_REPEAT},
	{"ls7a_spi_write","",0,"ls7a_spi_write()",ls7a_spi_write,0,99,CMD_REPEAT},
	{"ls7a_spi_erase_all","",0,"ls7a_spi_erase_all()",ls7a_spi_erase_all,0,99,CMD_REPEAT},
	{"ls7a_spi_write_byte","",0,"ls7a_spi_write_byte()",ls7a_spi_write_byte,0,99,CMD_REPEAT},
	{"read_flash_id","",0,"read_flash_id()",ls7a_spi_read_id,0,99,CMD_REPEAT},
	{"set_mac", "", NULL, "set the Mac address of LS7A syn0 and syn1", cmd_set_mac, 1, 5, 0},
	{0,0}
};

static void init_cmd __P((void)) __attribute__ ((constructor));

static void
init_cmd()
{
	cmdlist_expand(Cmds,1);
}

