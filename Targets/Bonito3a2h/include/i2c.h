#include "target/ls2h.h"

#define GS_SOC_I2C_PRER_LO	(volatile unsigned char *)(LS2H_I2C0_REG_BASE + 0x0)
#define GS_SOC_I2C_PRER_HI	(volatile unsigned char *)(LS2H_I2C0_REG_BASE + 0x1)
#define GS_SOC_I2C_CTR    	(volatile unsigned char *)(LS2H_I2C0_REG_BASE + 0x2)
#define GS_SOC_I2C_TXR    	(volatile unsigned char *)(LS2H_I2C0_REG_BASE + 0x3)
#define GS_SOC_I2C_RXR    	(volatile unsigned char *)(LS2H_I2C0_REG_BASE + 0x3)
#define GS_SOC_I2C_CR     	(volatile unsigned char *)(LS2H_I2C0_REG_BASE + 0x4)
#define GS_SOC_I2C_SR     	(volatile unsigned char *)(LS2H_I2C0_REG_BASE + 0x4)

void i2c_send(char ctrl,char addr);
char i2c_stat();
char i2c_recv();
