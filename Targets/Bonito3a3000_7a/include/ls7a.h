#ifndef _LS7A_H
#define _LS7A_H

#define LS7A_MISC_BASE					                0xb0080000

#define LS7A_GPIO_REG_BASE                              (LS7A_MISC_BASE + 0x60000)
#define LS7A_GPIO_OEN_REG                               LS7A_GPIO_REG_BASE
#define LS7A_GPIO_O_REG                                 (LS7A_GPIO_REG_BASE + 0x10)
#define LS7A_GPIO_I_REG                                 (LS7A_GPIO_REG_BASE + 0x20)

/* RTC regs */
#define LS7A_RTC_REG_BASE                               (LS7A_MISC_BASE + 0x50100)
#define LS7A_TOY_TRIM_REG                               (LS7A_RTC_REG_BASE + 0x0020)
#define LS7A_TOY_WRITE0_REG                             (LS7A_RTC_REG_BASE + 0x0024)
#define LS7A_TOY_WRITE1_REG                             (LS7A_RTC_REG_BASE + 0x0028)
#define LS7A_TOY_READ0_REG                              (LS7A_RTC_REG_BASE + 0x002c)
#define LS7A_TOY_READ1_REG                              (LS7A_RTC_REG_BASE + 0x0030)
#define LS7A_TOY_MATCH0_REG                             (LS7A_RTC_REG_BASE + 0x0034)
#define LS7A_TOY_MATCH1_REG                             (LS7A_RTC_REG_BASE + 0x0038)
#define LS7A_TOY_MATCH2_REG                             (LS7A_RTC_REG_BASE + 0x003c)
#define LS7A_RTC_CTRL_REG                               (LS7A_RTC_REG_BASE + 0x0040)
#define LS7A_RTC_TRIM_REG                               (LS7A_RTC_REG_BASE + 0x0060)
#define LS7A_RTC_WRITE0_REG                             (LS7A_RTC_REG_BASE + 0x0064)
#define LS7A_RTC_READ0_REG                              (LS7A_RTC_REG_BASE + 0x0068)
#define LS7A_RTC_MATCH0_REG                             (LS7A_RTC_REG_BASE + 0x006c)
#define LS7A_RTC_MATCH1_REG                             (LS7A_RTC_REG_BASE + 0x0070)
#define LS7A_RTC_MATCH2_REG                             (LS7A_RTC_REG_BASE + 0x0074)

#define LS7A_ACPI_REG_BASE                              (LS7A_MISC_BASE + 0x50000)
#define LS7A_ACPI_PM1_STS_REG                           (LS7A_ACPI_REG_BASE + 0xc)
#define LS7A_ACPI_PM1_CNT_REG                           (LS7A_ACPI_REG_BASE + 0x14)
#define LS7A_ACPI_RST_CNT_REG                           (LS7A_ACPI_REG_BASE + 0x30)
/*PWM*/
#define LS7A_PWM0_REG_BASE				(LS7A_MISC_BASE + 0x20000)
#define LS7A_PWM0_LOW                   (LS7A_PWM0_REG_BASE + 0x4)
#define LS7A_PWM0_FULL                  (LS7A_PWM0_REG_BASE + 0x8)
#define LS7A_PWM0_CTRL                  (LS7A_PWM0_REG_BASE + 0xc)

#define LS7A_PWM1_REG_BASE				(LS7A_MISC_BASE + 0x20100)
#define LS7A_PWM1_LOW                   (LS7A_PWM1_REG_BASE + 0x4)
#define LS7A_PWM1_FULL                  (LS7A_PWM1_REG_BASE + 0x8)
#define LS7A_PWM1_CTRL                  (LS7A_PWM1_REG_BASE + 0xc)

#define LS7A_PWM2_REG_BASE				(LS7A_MISC_BASE + 0x20200)
#define LS7A_PWM2_LOW                   (LS7A_PWM2_REG_BASE + 0x4)
#define LS7A_PWM2_FULL                  (LS7A_PWM2_REG_BASE + 0x8)
#define LS7A_PWM2_CTRL                  (LS7A_PWM2_REG_BASE + 0xc)

#define LS7A_PWM3_REG_BASE				(LS7A_MISC_BASE + 0x20300)
#define LS7A_PWM3_LOW                   (LS7A_PWM3_REG_BASE + 0x4)
#define LS7A_PWM3_FULL                  (LS7A_PWM3_REG_BASE + 0x8)
#define LS7A_PWM3_CTRL                  (LS7A_PWM3_REG_BASE + 0xc)

#define LS7A_I2C0_REG_BASE				(LS7A_MISC_BASE + 0x10000)
#define LS7A_I2C0_PRER_LO_REG				(LS7A_I2C0_REG_BASE + 0x0)
#define LS7A_I2C0_PRER_HI_REG				(LS7A_I2C0_REG_BASE + 0x1)
#define LS7A_I2C0_CTR_REG   				(LS7A_I2C0_REG_BASE + 0x2)
#define LS7A_I2C0_TXR_REG   				(LS7A_I2C0_REG_BASE + 0x3)
#define LS7A_I2C0_RXR_REG    				(LS7A_I2C0_REG_BASE + 0x3)
#define LS7A_I2C0_CR_REG     				(LS7A_I2C0_REG_BASE + 0x4)
#define LS7A_I2C0_SR_REG     				(LS7A_I2C0_REG_BASE + 0x4)

#define LS7A_I2C1_REG_BASE				(LS7A_MISC_BASE + 0x10100)
#define LS7A_I2C1_PRER_LO_REG				(LS7A_I2C1_REG_BASE + 0x0)
#define LS7A_I2C1_PRER_HI_REG				(LS7A_I2C1_REG_BASE + 0x1)
#define LS7A_I2C1_CTR_REG   				(LS7A_I2C1_REG_BASE + 0x2)
#define LS7A_I2C1_TXR_REG   				(LS7A_I2C1_REG_BASE + 0x3)
#define LS7A_I2C1_RXR_REG    				(LS7A_I2C1_REG_BASE + 0x3)
#define LS7A_I2C1_CR_REG     				(LS7A_I2C1_REG_BASE + 0x4)
#define LS7A_I2C1_SR_REG     				(LS7A_I2C1_REG_BASE + 0x4)

/* HPET */
#define LS7A_HPET_BASE 		0xB0001000
#define LS7A_HPET_PERIOD	LS7A_HPET_BASE + 0x4
#define LS7A_HPET_CONF		LS7A_HPET_BASE + 0x10 
#define LS7A_HPET_MAIN		LS7A_HPET_BASE + 0xF0 


#define CR_START					0x80
#define CR_STOP						0x40
#define CR_READ						0x20
#define CR_WRITE					0x10
#define CR_ACK						0x8
#define CR_IACK						0x1

#define SR_NOACK					0x80
#define SR_BUSY						0x40
#define SR_AL						0x20
#define SR_TIP						0x2
#define	SR_IF						

#endif /*_LS7A_H*/

