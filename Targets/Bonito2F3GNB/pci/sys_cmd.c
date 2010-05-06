#include <sys/linux/types.h>
#include <sys/param.h>
#include <sys/device.h>
#include <sys/systm.h>
#include <sys/malloc.h>

#include <dev/pci/pcivar.h>
#include <dev/pci/pcireg.h>
#include <dev/pci/nppbreg.h>

#include <machine/bus.h>

#include <include/bonito.h>
#include <include/wpce775x.h>
#include <include/cs5536.h>
#include <include/cs5536_pci.h>
#include <pmon.h>
#include "cs5536_io.h"

/*********************************MSR debug*********************************/

static int cmd_rdmsr(int ac, char *av[])
{
	u32 msr, hi, lo;

	if(!get_rsa(&msr, av[1])) {
		printf("msr : access error!\n");
		return -1;
	}

	_rdmsr(msr, &hi, &lo);
	
	printf("msr : address  %x    hi  %x   lo  %x\n", msr, hi, lo);
	
	return 0;
}

static int cmd_wrmsr(int ac, char *av[])
{
	u32 msr, hi, lo;

	if(!get_rsa(&msr, av[1])){
		printf("msr : access error!\n");
		return -1;
	}else if(!get_rsa(&hi, av[2])){
		printf("hi : access error!\n");
		return -1;
	}else if(!get_rsa(&lo, av[3])){
		printf("lo : access error!\n");
		return -1;
	}

	_wrmsr(msr, hi, lo);
	
	return 0;
}

/*******************************Display debug*******************************/

#define	DISPLAY_MODE_NORMAL		0	// Screen(on);	Hsync(on);	Vsync(on)
#define	DISPLAY_MODE_STANDBY	1	// Screen(off);	Hsync(off);	Vsync(on)
#define	DISPLAY_MODE_SUSPEND	2	// Screen(off);	Hsync(on);	Vsync(off)
#define	DISPLAY_MODE_OFF		3	// Screen(off);	Hsync(off);	Vsync(off)
#define	SEQ_INDEX	0x3c4
#define	SEQ_DATA	0x3c5
static inline unsigned char read_seq(int reg)
{
	*((volatile unsigned char *)(0xbfd00000 | SEQ_INDEX)) = reg;
	return (*(volatile unsigned char *)(0xbfd00000 | SEQ_DATA));
}

static inline void write_seq(int reg, unsigned char data)
{
	*((volatile unsigned char *)(0xbfd00000 | SEQ_INDEX)) = reg;
	*((volatile unsigned char *)(0xbfd00000 | SEQ_DATA)) = data;

	return;
}

static u8 cur_mode;
static u8 SavedSR01, SavedSR20, SavedSR21, SavedSR22, SavedSR23, SavedSR24, SavedSR31, SavedSR34;
static int cmd_powerdebug(int ac, char *av[])
{
	u8 index;
	u8 SR01, SR20, SR21, SR22, SR23, SR24, SR31, SR34;

	if(ac < 2){
		printf("usage : powerdebug %index\n");
		return -1;
	}

	if(!get_rsa(&index, av[1])) {
		printf("powerdebug : access error!\n");
		return -1;
	}
	
	if(cur_mode == DISPLAY_MODE_NORMAL){
		SavedSR01 = read_seq(0x01);
		SavedSR20 = read_seq(0x20);
		SavedSR21 = read_seq(0x21);
		SavedSR22 = read_seq(0x22);
		SavedSR23 = read_seq(0x23);
		SavedSR24 = read_seq(0x24);
		SavedSR31 = read_seq(0x31);
		SavedSR34 = read_seq(0x34);
		printf("normal power register : \n");
		printf("SR01 0x%x   SR20 0x%x   SR21 0x%x   SR22 0x%x   SR23 0x%x\n", 
						SavedSR01, SavedSR20, SavedSR21, SavedSR22, SavedSR23, SavedSR24);
		printf("SR24 0x%x   SR31 0x%x   SR34 0x%x\n", SavedSR24, SavedSR31, SavedSR34);
	}
	SR01 = read_seq(0x01);
	SR20 = read_seq(0x20);
	SR21 = read_seq(0x21);
	SR22 = read_seq(0x22);
	SR23 = read_seq(0x23);
	SR24 = read_seq(0x24);
	SR31 = read_seq(0x31);
	SR34 = read_seq(0x34);

	printf("powerdebug : mode %d\n", index);
	switch(index){
		case	DISPLAY_MODE_NORMAL :
				SR01 &= ~0x20;
				SR20 = SavedSR20;
		        SR21 = SavedSR21;
				SR22 &= ~0x30; 
				SR23 &= ~0xC0;
				SR24 |= 0x01;
				SR31 = SavedSR31;
				SR34 = SavedSR34;
				break;
		case	DISPLAY_MODE_STANDBY :
				SR01 |= 0x20;
				SR20 = (SR20 & ~0xB0) | 0x10;
				SR21 |= 0x88;
				SR22 = (SR22 & ~0x30) | 0x10;
				SR23 = (SR23 & ~0x07) | 0xD8;
				SR24 &= ~0x01;
				SR31 = (SR31 & ~0x07) | 0x00;
				SR34 |= 0x80;
				break;
		case	DISPLAY_MODE_SUSPEND :
				SR01 |= 0x20;
				SR20 = (SR20 & ~0xB0) | 0x10;
				SR21 |= 0x88;
				SR22 = (SR22 & ~0x30) | 0x20;
				SR23 = (SR23 & ~0x07) | 0xD8;
				SR24 &= ~0x01;
				SR31 = (SR31 & ~0x07) | 0x00;
				SR34 |= 0x80;
				break;
		case	DISPLAY_MODE_OFF :
				SR01 |= 0x20;
			    SR20 = (SR20 & ~0xB0) | 0x10;
				SR21 |= 0x88;
				SR22 = (SR22 & ~0x30) | 0x30;
				SR23 = (SR23 & ~0x07) | 0xD8;
				SR24 &= ~0x01;
				SR31 = (SR31 & ~0x07) | 0x00;
				SR34 |= 0x80;
				break;
		default :
				printf("powerdebug : not supported mode setting for display.\n");
				break;
	}

	if( (*((volatile unsigned char *)(0xbfd00000 | 0x3cc))) & 0x01 ){
		while( (*((volatile unsigned char *)(0xbfd00000 | 0x3da))) & 0x08 );
		while( !((*((volatile unsigned char *)(0xbfd00000 | 0x3da))) & 0x08) );
	}else{
		while( (*((volatile unsigned char *)(0xbfd00000 | 0x3ba))) & 0x08 );
		while( !((*((volatile unsigned char *)(0xbfd00000 | 0x3ba))) & 0x08) );
	}

	write_seq(0x01, SR01);
	write_seq(0x20, SR20);
	write_seq(0x21, SR21);
	write_seq(0x22, SR22);
	write_seq(0x23, SR23);
	write_seq(0x24, SR24);
	write_seq(0x31, SR31);
	write_seq(0x34, SR34);

	cur_mode = index;

	return 0;
}

/*******************************************************************************/

#ifdef LOONGSON2F_3GNB
static void smbus_stop(unsigned char chnl)
{
	unsigned char value;
	unsigned long chnl_base_addr;

	// set the stop bit and clear some flag
	chnl_base_addr = SMB_BASE_ADDR + chnl * SMB_CHNL_SIZE;

	value = rdcore(chnl_base_addr + REG_SMBCTL1);
	value |= SMBCTL1_STOP;
	wrcore(chnl_base_addr + REG_SMBCTL1, value);

	wrcore(chnl_base_addr + REG_SMBST, SMBST_BER | SMBST_NEGACK | SMBST_STASTR);

	return;
}

static void smbus_error(unsigned char chnl)
{
	unsigned char value;
	unsigned long chnl_base_addr;

	// set the stop bit and clear some flag
	chnl_base_addr = SMB_BASE_ADDR + chnl * SMB_CHNL_SIZE;

	value = rdcore(chnl_base_addr + REG_SMBCTL1);
	value |= SMBCTL1_STOP;
	value &= ~SMBCTL1_STASTRE;
	wrcore(chnl_base_addr + REG_SMBCTL1, value);

	wrcore(chnl_base_addr + REG_SMBST, SMBST_BER | SMBST_NEGACK | SMBST_STASTR);
	
	return;
}

static void smbus_start(unsigned char chnl)
{
	unsigned char value;
	unsigned long chnl_base_addr;

	chnl_base_addr = SMB_BASE_ADDR + chnl * SMB_CHNL_SIZE;

	/* clear int */
	value = rdcore(chnl_base_addr + REG_SMBCTL1);
	value &= ~(SMBCTL1_NMINTE | SMBCTL1_GCMEN | SMBCTL1_INTEN | SMBCTL1_ACK);
	wrcore(chnl_base_addr + REG_SMBCTL1, value);
	/* clear status bit */
	wrcore(chnl_base_addr + REG_SMBST, SMBST_NMATCH | SMBST_STASTR | SMBST_NEGACK | SMBST_BER | SMBST_SLVSTP);

	value = rdcore(chnl_base_addr + REG_SMBCTL1);
	value |= SMBCTL1_START;
	wrcore(chnl_base_addr + REG_SMBCTL1, value);
	delay(SMB_DELAY);

	return;
}

static int smbus_addr(unsigned char chnl, unsigned char addr)
{
	unsigned char value;
	unsigned char status;
	unsigned long chnl_base_addr;

	chnl_base_addr = SMB_BASE_ADDR + chnl * SMB_CHNL_SIZE;

	status = rdcore(chnl_base_addr + REG_SMBST);
	if(status & (SMBST_BER + SMBST_NEGACK)){
		printf("addr stage : err 0 : status 0x%x, ctrl 0x%x\n",
				rdcore(chnl_base_addr + REG_SMBST), rdcore(chnl_base_addr + REG_SMBCTL1));
		smbus_error(chnl);
		return -1;
	}else if((status & (SMBST_MASTER + SMBST_SDAST)) == (SMBST_MASTER + SMBST_SDAST)){
		/* enable after send address stall the bus */
		value = rdcore(chnl_base_addr + REG_SMBCTL1);
		value |= SMBCTL1_STASTRE;
		wrcore(chnl_base_addr + REG_SMBCTL1, value);
		delay(SMB_DELAY);

		wrcore(chnl_base_addr + REG_SMBSDA, addr);
		delay(SMB_DELAY);
	}else{
		printf("addr stage : err 1 : status 0x%x, ctrl 0x%x\n",
				rdcore(chnl_base_addr + REG_SMBST), rdcore(chnl_base_addr + REG_SMBCTL1));
		smbus_error(chnl);
		return -1;
	}
	delay(SMB_DELAY);

	return 0;
}

static int smbus_send(unsigned char chnl, unsigned char index)
{
	unsigned char status;
	unsigned char value;
	unsigned long chnl_base_addr;

	chnl_base_addr = SMB_BASE_ADDR + chnl * SMB_CHNL_SIZE;

	status = rdcore(chnl_base_addr + REG_SMBST);
	if(status & (SMBST_BER + SMBST_NEGACK)){
		printf("send stage : err 1 : status 0x%x, ctrl 0x%x\n", 
			rdcore(chnl_base_addr + REG_SMBST), rdcore(chnl_base_addr + REG_SMBCTL1));
		smbus_error(chnl);
		return -1;
	}else if(status & SMBST_STASTR){
		// start send cmd or data
		if(rdcore(chnl_base_addr + REG_SMBCTL1) & SMBCTL1_STASTRE)
			wrcore(chnl_base_addr + REG_SMBST, SMBST_STASTR);
		delay(SMB_DELAY);
		wrcore(chnl_base_addr + REG_SMBSDA, index);		
		delay(SMB_DELAY);
	}else{
		printf("send stage : err 2 : status 0x%x, ctrl 0x%x\n", 
			rdcore(chnl_base_addr + REG_SMBST), rdcore(chnl_base_addr + REG_SMBCTL1));
		smbus_error(chnl);
		return -1;
	}
		
	// stop internal stage
	delay(SMB_DELAY);
	status = rdcore(chnl_base_addr + REG_SMBST);
	if(status & (SMBST_BER + SMBST_NEGACK)){
		printf("send stop stage : err 1 : status 0x%x, ctrl 0x%x\n", 
			rdcore(chnl_base_addr + REG_SMBST), rdcore(chnl_base_addr + REG_SMBCTL1));
		smbus_error(chnl);
		return -1;
	}else if(status & SMBST_SDAST){
		smbus_stop(chnl);
	}else{
		printf("send stop stage : err 2 : status 0x%x, ctrl 0x%x\n", 
			rdcore(chnl_base_addr + REG_SMBST), rdcore(chnl_base_addr + REG_SMBCTL1));
		smbus_error(chnl);
		return -1;
	}

	return 0;
}

static int smbus_recv(unsigned char chnl, unsigned char *val)
{
	unsigned char status, value;
	unsigned long chnl_base_addr;

	chnl_base_addr = SMB_BASE_ADDR + chnl * SMB_CHNL_SIZE;

	status = rdcore(chnl_base_addr + REG_SMBST);
	if(status & (SMBST_BER + SMBST_NEGACK)){
		printf("recv stage : err 1 : status 0x%x, ctrl 0x%x\n", 
			rdcore(chnl_base_addr + REG_SMBST), rdcore(chnl_base_addr + REG_SMBCTL1));
		smbus_error(chnl);
		return -1;
	}else if(status & SMBST_STASTR){
		// have send out address, and recv now
		if(rdcore(chnl_base_addr + REG_SMBCTL1) & SMBCTL1_STASTRE)
			wrcore(chnl_base_addr + REG_SMBST, SMBST_STASTR);	
		delay(SMB_DELAY);
		value = rdcore(chnl_base_addr + REG_SMBCTL1);
		value &= ~SMBCTL1_STASTRE;		// ??? now STASTR AGAIN, maybe no need
		value |= SMBCTL1_ACK;
		wrcore(chnl_base_addr + REG_SMBCTL1, value);
		delay(SMB_DELAY);
		//delay(1000);
	}else{
		printf("recv stage : err 2 : status 0x%x, ctrl 0x%x\n", 
			rdcore(chnl_base_addr + REG_SMBST), rdcore(chnl_base_addr + REG_SMBCTL1));
		smbus_error(chnl);
		return -1;
	}

	//////////////////////////////// stop internal stage
	delay(SMB_DELAY);
	status = rdcore(chnl_base_addr + REG_SMBST);

	if(status & SMBST_SDAST){
		// issue stop and get data
		value = rdcore(chnl_base_addr + REG_SMBCTL1);
		value |= SMBCTL1_STOP;
		wrcore(chnl_base_addr + REG_SMBCTL1, value);
		delay(SMB_DELAY);

		*val = rdcore(chnl_base_addr + REG_SMBSDA);
	}else{
		printf("recv stop stage : err 1 : status 0x%x, ctrl 0x%x\n", 
			rdcore(chnl_base_addr + REG_SMBST), rdcore(chnl_base_addr + REG_SMBCTL1));
		smbus_error(chnl);
		return -1;
	}

	return 0;
}

static int cmd_rdsmbus(int ac, char *av[])
{
    unsigned long addr;
    unsigned long index;
	unsigned char data;
	unsigned char chnl;
	int ret = 0;

	if(ac < 3){
	    printf("usage : rdsmbus channel addr index\n");
	    return -1;
	}

	if(!get_rsa(&chnl, av[1])) {
	    printf("ecreg : access error!\n");
	    return -1;
	}
	
	if(!get_rsa(&addr, av[2])) {
	    printf("ecreg : access error!\n");
	    return -1;
	}
	
	if(!get_rsa(&index, av[3])){
	    printf("ecreg : size error\n");
		return -1;
	}

	printf("channel SMB%d, addr 0x%x, index 0x%x\n", chnl + 1, addr, index, data);
	smbus_start(chnl);
	ret = smbus_addr(chnl, addr & 0xfe);
	if(ret < 0){
		return ret;
	}
	ret = smbus_send(chnl, index);
	if(ret < 0){
		return ret;
	}
	
	smbus_start(chnl);
	
	ret = smbus_addr(chnl, addr | 0x01);
	if(ret < 0){
		return ret;
	}
	ret = smbus_recv(chnl, &data);
	if(ret < 0){
		return ret;
	}
	printf("final : channel SMB%d, addr 0x%x, index 0x%x, data 0x%x\n", chnl, addr, index, data);

	return 0;
}

/*
 * wr775reg : write a WPEC775x EC register area.
 */
static int cmd_wr775reg(int ac, char *av[])
{
    u32 addr;
    u32 value;
    u8 val8;

	if(ac < 3){
	    printf("usage : wrecreg addr val\n");
	    return -1;
	}
	 
	if(!get_rsa(&addr, av[1])) {
	    printf("ecreg : access error!\n");
	    return -1;
	}
	
	if(!get_rsa(&value, av[2])){
	    printf("ecreg : size error\n");
		return -1;
	}
	 
	val8 = (u8)value;
	
	if((addr > 0x1000000)){
	    printf("ecreg : addr not available.\n");
	}
	
	printf("ecreg : addr=0x%x value=0x%x\n", addr , val8);
	wrwbec(addr, val8);
	
	return 0;
}

/*
 * WPCE775_rdecreg : read a WPEC775x EC register area.
 */
static int cmd_rd775reg(int ac, char *av[])
{
	u32 start, size, reg;
	u8 value;

	size = 0;

	if(ac < 2){
		printf("usage : rdecreg start_addr size\n");
		return -1;
	}

	//start = strtoul(av[1], NULL, 16);
	//size = strtoul(av[2], NULL, 16);

	if(!get_rsa(&start, av[1])) {
	//if(!start) {
		printf("ecreg : access error!\n");
		return -1;
	}

	reg = start;
//	while(size > 0){
		//printf("reg address : 0x%x, value1 : 0x%x, value2 : %c\n", reg, WPCE775_rdec(reg), WPCE775_rdec(reg));
		printf("reg address : 0x%x, value : 0x%x\n", reg, rdwbec(reg));
//		reg++;
//		size--;
//	}

	return 0;
}

static int cmd_wr775(int ac, char *av[])
{
	u8 index;
	u8 cmd;
	u8 value = 0;

	if(ac < 4){
		printf("usage : wr775 cmd index value\n");
		return -1;
	}

	cmd = strtoul(av[1], NULL, 16);
	index = strtoul(av[2], NULL, 16);
	value = strtoul(av[3], NULL, 16);
	printf("Write EC 775 : command 0x%x, index 0x%x, data 0x%x \n", cmd, index, value);
	/* Write index value to EC 775 space */
	wracpi(cmd, index, value);

	return 0;
} 

/*
 * rd775 : read a space of EC 775.
 * usage : rd775 cmd index size (hex)
 *  e.g. : rd775 4f [0] [21] (read EC version)
 * return: value (hex, dec, char)
 * daway added 2010-01-13
 */
static int cmd_rd775(int ac, char *av[])
{
	u8 i;
	u8 cmd, index;
	u8 size = 1, val = 0;
	
	if(ac < 2){
		printf("usage : rd775 cmd [index] [size]\n");
		return -1;
	}
	
	if(ac > 3){
		if(!get_rsa(&size, av[3])){
			printf("ERROR : arguments error!\n");
			return -1;
		}
	}

	cmd = strtoul(av[1], NULL, 16);
	index = strtoul(av[2], NULL, 16);

	printf("Read EC 775 : command 0x%x, index 0x%x, size %d \n", cmd, index, size);

	for(i = 0; i < size; i++){
		/* Read EC 775 space value */
		val = rdacpi(cmd, index);
		printf("Read EC 775 : command 0x%x, index 0x%x, value 0x%x %d '%c'\n", cmd, index, val, val, val);
		index++;
	}

	return 0;
}

static int cmd_wrsio(int ac, char *av[])
{
    u8 index, data;
 
    if(ac < 2){
        printf("usage : wrsio index data\n");
        return -1;
    }

	index = strtoul(av[1], NULL, 16);
	data = strtoul(av[2], NULL, 16);

	printf("Write EC775 SuperIO : index 0x%x, data 0x%x\n", index, data);

	wrsio(index, data);

	return 0;
}

static int cmd_rdsio(int ac, char *av[])
{
    u8 index;
    u8 val = 0;
 
    if(ac < 2){
        printf("usage : rdsio index\n");
        return -1;
    }

	index = strtoul(av[1], NULL, 16);

	printf("Read EC775 SuperIO : index 0x%x, ", index);

	val = rdsio(index);
	printf("data 0x%x\n", val);

	return 0;
}

static int cmd_wrwcb(int ac, char *av[])
{
    u16 wcb_addr;
	u8 data;
 
    if(ac < 2){
        printf("usage : wrwcb index data\n");
        return -1;
    }

	wcb_addr = strtoul(av[1], NULL, 16);
	data = strtoul(av[2], NULL, 16);

	printf("Write EC775 WCB : address 0x%x, data 0x%x\n", wcb_addr, data);

	wrwcb(wcb_addr, data);

	return 0;
}

static int cmd_rdwcb(int ac, char *av[])
{
    u16 wcb_addr;
	u8 size = 1;
    u8 val = 0;
	int i;
 
    if(ac < 2){
        printf("usage : rdwcb index [size]\n");
        return -1;
    }

	wcb_addr = strtoul(av[1], NULL, 16);
	size = strtoul(av[2], NULL, 16);

	if(size > 64){
		printf("ERROR : out of size range.\n");
		return -1;
	}

	if(size < 1 || size == ""){
		size = 1;
	}

	printf("Read EC775 WCB : start address 0x%x, size %d\n", wcb_addr, size);

	for(i = 0; i < size; i++){
		val = rdwcb(wcb_addr + i);
		printf("WCB address 0x%x, data 0x%x\n", wcb_addr + i, val);
	}

	return 0;
}

static int cmd_wrldn(int ac, char *av[])
{
    u8 ld_num, index, data;
 
    if(ac < 3){
        printf("usage : wrldn logical_device_num index data\n");
        return -1;
    }

	ld_num = strtoul(av[1], NULL, 16);
	index = strtoul(av[2], NULL, 16);
	data = strtoul(av[3], NULL, 16);

	printf("Write EC775 LDN : logical device number 0x%x, index 0x%x, data 0x%x\n", ld_num, index, data);

	wrldn(ld_num, index, data);

	return 0;
}

static int cmd_rdldn(int ac, char *av[])
{
    u8 ld_num, index;
    u8 val = 0;
 
    if(ac < 2){
        printf("usage : rdldn logical_device_num index\n");
        return -1;
    }

	ld_num = strtoul(av[1], NULL, 16);
	index = strtoul(av[2], NULL, 16);

	printf("Read EC775 LDN : logical device number 0x%x, index 0x%x, ", ld_num, index);

	val = rdldn(ld_num, index);
	printf("data 0x%x\n", val);

	return 0;
}

static int cmd_loop_rdwcb(int ac, char *av[])
{
    u32 wcb_addr;
	u8 size = 1;
    u8 val = 0;
	int i;
 
    if(ac < 2){
        printf("usage : rd_wcb address size\n");
        return -1;
    }

	wcb_addr = strtoul(av[1], NULL, 16);
	size = strtoul(av[2], NULL, 16);

	if(size > 64){
		printf("ERROR : out of size range.\n");
		return -1;
	}

	if(size < 1 || size == ""){
		size = 1;
	}

	printf("Read EC775 WCB : start address 0x%x, size %d\n", wcb_addr, size);

	while(1){
		for(i = 0; i < size; i++){
			val = rd_wcb(wcb_addr + i);
			printf("WCB address 0x%x, data 0x%x\n", wcb_addr + i, val);
			delay(50000);
		}
	}

	return 0;
}

#include "include/flupdate.h"
extern unsigned short Read_ECROM_IDs(void);
static int cmd_rdids(int ac, char *av[])
{
    u16 ec_ids;
/* 
    if(ac < 2){
        printf("usage : rdids\n");
        return -1;
    }
*/
	//wcb_addr = strtoul(av[1], NULL, 16);
	//size = strtoul(av[2], NULL, 16);


	Update_Flash_Init();
	//wrwbec(0x10fe0, CMD_READ_DEV_ID);
	ec_ids = Read_ECROM_IDs();
	printf("Read EC775 ROM ID : ");
	printf("0x%x\n", ec_ids);

	return 0;
}


#if 0	// test SHM module for wpce775l, daway 2010-03-19




static int cmd_rdids(int ac, char *av[])
{
    //u8 index;
    unsigned short val = 0;
 
    if(ac < 2){
        printf("usage : rdids\n");
        return -1;
    }

	//index = strtoul(av[1], NULL, 16);

	Sio_index = 0x2E;
	Sio_data = 0x2F;
	pc_wcb_base = getPcWcbBase();
	printf("Get WCB base address 0x%x\n", pc_wcb_base);
	printf("Read EC775 EC ID : ");
	//val = SHMReadIDs();
	printf("data 0x%x\n", val);

	return 0;
}
#endif

/*
 * rdbat : read a register of battery.
 */
static int cmd_rdbat(int ac, char *av[])
{
	u8 bat_present;
	u8 bat_capacity;
	u16 bat_voltage;
	u16 bat_current;
	u16 bat_temperature;
	
	/* read battery status */
	bat_present = rdacpi(0x80, 0xA2) & (1 << 6);

	/* read battery voltage */
	bat_voltage = (u16) rdacpi(0x80, 0x90);
	bat_voltage |= (((u16) rdacpi(0x80, 0x91)) << 8);
	bat_voltage = bat_voltage * 2;

	/* read battery current */
	bat_current = (u16) rdacpi(0x80, 0x96);
	bat_current |= (((u16) rdacpi(0x80, 0x97)) << 8);
	bat_current = bat_current * 357 / 2000;

	/* read battery capacity % */
	bat_capacity = rdacpi(0x80, 0x92);

	/* read battery temperature */
	bat_temperature = (u16) rdacpi(0x80, 0x93);
	bat_temperature |= (((u16) rdacpi(0x80, 0x94)) << 8);
	if(bat_present && bat_temperature <= 0x5D4){	/* temp <= 100 (0x5D4 = 1492(d)) */
		bat_temperature = bat_temperature / 4 - 273;
	}else{
		bat_temperature = 0;
	}

    printf("Read battery current information:\n");
	printf("Voltage %dmV, Current %dmA, Capacity %d%%, Temperature %d\n",
			bat_voltage, bat_current, bat_capacity, bat_temperature);

	return 0;
}

/* 
 * rdfan : read a register of temperature IC. 
 */
static int cmd_rdfan(int ac, char *av[])
{
    u16 speed = 0;
	u8 temperature;

	speed = (u16)rdacpi(0x80, 0x08);

    speed |= (((u16)rdacpi(0x80, 0x09)) << 8); 

	temperature = rdacpi(0x80, 0x1B);

    printf("Fan speed: %dRPM, CPU temperature: %d.\n", speed, temperature);

	return 0;
}
#endif	//end ifdef LOONGSON2F_3GNB

int cmd_testvideo(int ac, char *av[])
{
	unsigned long addr;
	if(ac < 2)
			return -1;
	addr = strtoul(av[1], NULL, 16);
	printf("addr = 0x%x", addr);
	video_display_bitmap(addr, 280, 370);
	return 1;
}

static const Cmd Cmds[] =
{
	{"cs5536 debug"},
	{"powerdebug", "reg", NULL, "for debug the power state of sm712 graphic", cmd_powerdebug, 2, 99, CMD_REPEAT},
	{"rdmsr", "reg", NULL, "msr read test", cmd_rdmsr, 2, 99, CMD_REPEAT},
	{"wrmsr", "reg", NULL, "msr write test", cmd_wrmsr, 2, 99, CMD_REPEAT},
#ifdef LOONGSON2F_3GNB	
	{"wri", "reg", NULL, "WPEC775L EC reg write test", cmd_wr775reg, 2, 99, CMD_REPEAT},
	{"rdi", "reg", NULL, "WPEC775L EC reg read test", cmd_rd775reg, 2, 99, CMD_REPEAT},
	{"rdsmbus", "reg", NULL, "WPEC775L sumbus reg read test", cmd_rdsmbus, 2, 99, CMD_REPEAT},
	{"wr775", "reg", NULL, "WPCE775L Space write test", cmd_wr775, 2, 99, CMD_REPEAT},
	{"rd775", "reg", NULL, "WPCE775L Space read test", cmd_rd775, 2, 99, CMD_REPEAT},
	{"wrsio", "reg", NULL, "WPCE775L Super IO port write test", cmd_wrsio, 2, 99, CMD_REPEAT},
	{"rdsio", "reg", NULL, "WPCE775L Super IO port read test", cmd_rdsio, 2, 99, CMD_REPEAT},
	{"wrwcb", "reg", NULL, "WPCE775L WCB(Write Command Buffer) write test", cmd_wrwcb, 2, 99, CMD_REPEAT},
	{"rdwcb", "reg", NULL, "WPCE775L WCB(Write Command Buffer) read test", cmd_rdwcb, 2, 99, CMD_REPEAT},
	{"wrldn", "reg", NULL, "WPCE775L logical device write test", cmd_wrldn, 2, 99, CMD_REPEAT},
	{"rdldn", "reg", NULL, "WPCE775L logical device read test", cmd_rdldn, 2, 99, CMD_REPEAT},
	{"rd_wcb", "reg", NULL, "WPCE775L WCB(Write Command Buffer) read test", cmd_loop_rdwcb, 2, 99, CMD_REPEAT},
	{"rdids", "reg", NULL, "WPCE775L EC ID read test", cmd_rdids, 0, 99, CMD_REPEAT},
	{"rdbat", "reg", NULL, "WPCE775L battery reg read test", cmd_rdbat, 0, 99, CMD_REPEAT},
	{"rdfan", "reg", NULL, "WPCE775L CPU temperature and fan reg read test", cmd_rdfan, 0, 99, CMD_REPEAT},
#endif
	{"testvideo", "reg", NULL, "for debug read data from xbi interface of ec", cmd_testvideo, 2, 99, CMD_REPEAT},
	{0},
};


static void init_cmd __P((void)) __attribute__ ((constructor));

static void
init_cmd()
{
	cmdlist_expand(Cmds, 1);
}

