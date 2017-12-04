/*
 * This file is for LTC2946 for LS7A 2 way.
 *
 */
#include <sys/linux/types.h>
#include <pmon.h>
#include <stdio.h>
#include <machine/pio.h>
#include "target/ls7a.h"

#define PRER_LO_REG	0x0
#define PRER_HI_REG	0x1
#define CTR_REG    	0x2
#define TXR_REG    	0x3
#define RXR_REG    	0x3
#define CR_REG     	0x4
#define SR_REG     	0x4

#define ls7a_i2c_writeb(reg, val)   outb(LS7A_I2C1_REG_BASE + reg, val)
#define ls7a_i2c_readb(reg)         inb(LS7A_I2C1_REG_BASE + reg)

#define LTC_REG_COUNT   0x44
//unit mV
#define LTC_DLTSNS_RESOLUTION   25  //uV
#define LTC_VIN_RESOLUTION      25  //mV
#define LTC_ADDIN_RESOLUTION    0.5 //mV
#define LTC_RSNS    5   //mO

//#define LTC_DEBUG

#ifdef  LTC_DEBUG
#define _LTCVERBOSE 5
#else
#define _LTCVERBOSE 0
#endif
#define LTC_I2C_MASS_ADDR   0xcc

int _ltcverbose = _LTCVERBOSE;
unsigned char ltc_mass_addr = LTC_I2C_MASS_ADDR;

static void ls7a_i2c_stop(void)
{
again:
    ls7a_i2c_writeb(CR_REG, CR_STOP);
    ls7a_i2c_readb(SR_REG);
    while (ls7a_i2c_readb(SR_REG) & SR_BUSY)
        goto again;
}

static void ls7a_i2c_init(void)
{
    //delay(1000);
    ls7a_i2c_writeb(CTR_REG, 0x0);
    //delay(1000);
    ls7a_i2c_writeb(PRER_LO_REG, 0x71);
    ls7a_i2c_writeb(PRER_HI_REG, 0x2);
    ls7a_i2c_writeb(CTR_REG, 0x80);
}

static int i2c_tx_byte(u8 data, u8 opt)
{
    ls7a_i2c_writeb(TXR_REG, data);
    ls7a_i2c_writeb(CR_REG, opt);
    while (ls7a_i2c_readb(SR_REG) & SR_TIP) ;

    if (ls7a_i2c_readb(SR_REG) & SR_NOACK) {
        printf("ltc has no ack, Pls check the hardware!\n");
        ls7a_i2c_stop();
        return -1;
    }

    return 0;
}

static int ltc_send_addr(u8 dev_addr ,u8 data_addr)
{

    if (i2c_tx_byte(dev_addr, CR_START | CR_WRITE) < 0)
        return 0;
    if (i2c_tx_byte(data_addr, CR_WRITE) < 0)
        return 0;

    return 1;
}

int ltc_write(u8 dev_addr,u8 data_addr, u8 *buf, int count)
{
    u8 i;
    if (!ltc_send_addr(dev_addr,data_addr))
        return 0;

    for (i = 0; i < count; i++)
        if (i2c_tx_byte(buf[i] & 0xff, CR_WRITE) < 0)
            return 0;
    ls7a_i2c_stop();

    return i;
}

static int ltc_read_cur(u8 dev_addr,u8 *buf, int count)
{
    u8 i;
    dev_addr |= 0x1;

    if (i2c_tx_byte(dev_addr, CR_START | CR_WRITE) < 0)
        return 0;

    for (i = 0; i < count; i++) {
        ls7a_i2c_writeb(CR_REG, ((i == count - 1) ?
                    (CR_READ | CR_ACK) : CR_READ));
        while (ls7a_i2c_readb(SR_REG) & SR_TIP) ;
        buf[i] = ls7a_i2c_readb(RXR_REG);
    }

    ls7a_i2c_stop();
    return i;
}

int ltc_read(u8 dev_addr,u8 data_addr, u8 *buf,int count)
{
    if (!ltc_send_addr(dev_addr,data_addr))
        return 0;

    if (ltc_read_cur(dev_addr,buf,count) != count)
        return 0;
    return 1;
}

#define MAX_LTC_NUM 4
int ltc_init(ac, av)
    int ac;
    char *av[];
{
    int i, j;
    u8 dev_addr[MAX_LTC_NUM];
    u8 buf[LTC_REG_COUNT] = {0};
    int ltc_num = 0;
    //get parameters
    if(ac < 2){
            printf("usage: ltc_init i2c_addr\n");
            return 0;
    }
    for(i=0; i < MAX_LTC_NUM; i++){
            if(ac > i+1){
                dev_addr[i] = (unsigned char)strtoul(av[i+1], 0, 0);
                printf("info: i2c_addr[%d]: 0x%x\n", i, dev_addr[i]);
                ltc_num++;
            }
    }
    if(ac > (MAX_LTC_NUM + 1)){
            printf("Warning: support %d LTC devices at the most.\n", MAX_LTC_NUM);
    }

    ls7a_i2c_init();

    for(i=0; i < ltc_num; i++){
        //check chip ID
        if(!ltc_read(dev_addr[i], 0xe7, buf, 1)){
            printf("Error: ltc read ID e7 fail!\n");
            return 0;
        };
        if(!ltc_read(dev_addr[i], 0xe8, &buf[1], 1)){
            printf("Error: ltc read ID e8 fail!\n");
            return 0;
        };
        if(buf[0] != 0x60 || buf[1] != 0x01){
            printf("Info: ltc ID is: %02x %02x\n", buf[0], buf[1]);
            printf("Error: ltc ID check fail!\n");
            return 0;
        }

        printf("check ltc %d(addr 0x%02x) info success.\n", i, dev_addr[i]);
        //read all registers out for test
        if(_ltcverbose > 2){
            if(!ltc_read(dev_addr[i], 0, buf, LTC_REG_COUNT)){
                printf("Error: ltc read rand fail!\n");
                return 0;
            };

            //printf("");
            for (j = 0; j < LTC_REG_COUNT; j++) {
                if (!(j % 8))
                    printf("\n0x%02x: ", j);
                printf("%02x ", buf[j]);
            }
            printf("\n");
        }

        //reset
        buf[0] = (0x3 << 0);
        ltc_write(dev_addr[i], 1, buf, 1);
        //set initial MIN/MAX
        buf[0] = 0x0;
        buf[1] = 0x0;
        buf[2] = 0x0;

        buf[4] = 0xff;
        buf[5] = 0xff;
        buf[6] = 0xff;
        buf[7] = 0xf0;
        //Power
        ltc_write(dev_addr[i], 0x8, &buf[0], 3);
        ltc_write(dev_addr[i], 0xb, &buf[4], 3);

        //deltaSense
        ltc_write(dev_addr[i], 0x16, &buf[0], 2);
        ltc_write(dev_addr[i], 0x18, &buf[6], 2);

        //VIN
        ltc_write(dev_addr[i], 0x20, &buf[0], 2);
        ltc_write(dev_addr[i], 0x22, &buf[6], 2);

        //ADIN
        ltc_write(dev_addr[i], 0x2A, &buf[0], 2);
        ltc_write(dev_addr[i], 0x2C, &buf[6], 2);

        //set threshold TODO
        //enable alert  TODO
        //configure monitor mode
        buf[0] = (0x3 << 0) | (0x3 << 3) | (0x2 << 5);
        ltc_write(dev_addr[i], 0, buf, 1);

        printf("configure ltc %d(addr 0x%02x) success.\n", i, dev_addr[i]);
    }

}

int ltc_result_proc(unsigned char *buf){
    unsigned int val_cur, val_max, val_min;
    unsigned int volt_cur, volt_max, volt_min;
#if 1   //ADDIN
    printf("ADDIN(mV),  ");
    val_cur = (buf[0x28 + 0] << 4) | (buf[0x28 + 1] >> 4);
    val_max = (buf[0x28 + 2] << 4) | (buf[0x28 + 3] >> 4);
    val_min = (buf[0x28 + 4] << 4) | (buf[0x28 + 5] >> 4);

    volt_cur = val_cur * LTC_ADDIN_RESOLUTION;
    volt_max = val_max * LTC_ADDIN_RESOLUTION;
    volt_min = val_min * LTC_ADDIN_RESOLUTION;

    printf("%d, %d, %d, \n", volt_cur, volt_max, volt_min);
    if(_ltcverbose > 3){
        printf("%02x%02x, %02x%02x, %02x%02x, \n", buf[0x28 + 0], buf[0x28 + 1], buf[0x28 + 2], buf[0x28 + 3], buf[0x28 + 4], buf[0x28 + 5]);
    }
#endif
#if 1   //Vin
    printf("Vin(mV),  ");
    val_cur = (buf[0x1E + 0] << 4) | (buf[0x1E + 1] >> 4);
    val_max = (buf[0x1E + 2] << 4) | (buf[0x1E + 3] >> 4);
    val_min = (buf[0x1E + 4] << 4) | (buf[0x1E + 5] >> 4);

    volt_cur = val_cur * LTC_VIN_RESOLUTION;
    volt_max = val_max * LTC_VIN_RESOLUTION;
    volt_min = val_min * LTC_VIN_RESOLUTION;

    printf("%d, %d, %d, \n", volt_cur, volt_max, volt_min);
    if(_ltcverbose > 3){
        printf("%02x%02x, %02x%02x, %02x%02x, \n", buf[0x1E + 0], buf[0x1E + 1], buf[0x1E + 2], buf[0x1E + 3], buf[0x1E + 4], buf[0x1E + 5]);
    }
#endif
#if 1   //deltaSense
    printf("deltaSense(uV),  ");

    val_cur = (buf[0x14 + 0] << 4) | (buf[0x14 + 1] >> 4);
    val_max = (buf[0x14 + 2] << 4) | (buf[0x14 + 3] >> 4);
    val_min = (buf[0x14 + 4] << 4) | (buf[0x14 + 5] >> 4);

    volt_cur = val_cur * LTC_DLTSNS_RESOLUTION;
    volt_max = val_max * LTC_DLTSNS_RESOLUTION;
    volt_min = val_min * LTC_DLTSNS_RESOLUTION;

    printf("%d, %d, %d, \n", volt_cur, volt_max, volt_min);
    if(_ltcverbose > 3){
        printf("%02x%02x, %02x%02x, %02x%02x, \n", buf[0x14 + 0], buf[0x14 + 1], buf[0x14 + 2], buf[0x14 + 3], buf[0x14 + 4], buf[0x14 + 5]);
    }
#endif
#if 1   //Power
    printf("Power(uW)(Rsns %d m ohm),  ", LTC_RSNS);
    val_cur = (buf[0x5 + 0] << 16) | (buf[0x5 + 1] << 8) | (buf[0x5 + 2]);
    val_max = (buf[0x8 + 0] << 16) | (buf[0x8 + 1] << 8) | (buf[0x8 + 2]);
    val_min = (buf[0xb + 0] << 16) | (buf[0xb + 1] << 8) | (buf[0xb + 2]);

    volt_cur = val_cur * LTC_VIN_RESOLUTION * LTC_DLTSNS_RESOLUTION / LTC_RSNS;
    volt_max = val_max * LTC_VIN_RESOLUTION * LTC_DLTSNS_RESOLUTION / LTC_RSNS;
    volt_min = val_min * LTC_VIN_RESOLUTION * LTC_DLTSNS_RESOLUTION / LTC_RSNS;

    printf("%d, %d, %d, \n", volt_cur, volt_max, volt_min);
    if(_ltcverbose > 3){
        printf("%02x%02x%02x, ", buf[0x5 + 0], buf[0x5 + 1], buf[0x5 + 2]);
        printf("%02x%02x%02x, ", buf[0x8 + 0], buf[0x8 + 1], buf[0x8 + 2]);
        printf("%02x%02x%02x, ", buf[0xb + 0], buf[0xb + 1], buf[0xb + 2]);
        printf("\n");
    }
#endif

    return 0;
}

int ltc_check(ac, av)
    int ac;
    char *av[];
{
    int i;
    u8 dev_addr[MAX_LTC_NUM];
    u8 buf[LTC_REG_COUNT] = {0};
        int ltc_num = 0;
        //get parameters
        if(ac < 2){
                printf("usage: ltc_init i2c_addr\n");
                return 0;
        }
        for(i=0; i < MAX_LTC_NUM; i++){
            if(ac > i+1){
                dev_addr[i] = (unsigned char)strtoul(av[i+1], 0, 0);
                if(_ltcverbose > 10){
                    printf("info: i2c_addr[%d]: 0x%x\n", i, dev_addr[i]);
                }
                ltc_num++;
            }
        }
        if(ac > (MAX_LTC_NUM + 1)){
            printf("Warning: support %d LTC devices at the most.\n", MAX_LTC_NUM);
        }

        //check fault TODO

        //read results
        printf("voltage, cur, max, min,\n");
        for(i=0; i < ltc_num; i++){
            printf("readout ltc %d(addr 0x%02x) results:\n", i, dev_addr[i]);
            if(!ltc_read(dev_addr[i], 0, buf, LTC_REG_COUNT)){
                printf("Error: read results fail!\n");
                return 0;
            }else{
                ltc_result_proc(buf);
            };
        }

}

static const Cmd Cmds[] = {
    {"Misc"},
    {"ltc_init", "i2c addr0 [addr1] [addr2] ...", 0, "ltc init", ltc_init, 1, 5, 0},
    {"ltc_check", "i2c addr0 [addr1] [addr2] ...", 0, "check ltc results", ltc_check, 1, 5, 0},
    {0, 0}
};

static void init_cmd __P((void)) __attribute__ ((constructor));
static void init_cmd()
{
    cmdlist_expand(Cmds, 1);
}
