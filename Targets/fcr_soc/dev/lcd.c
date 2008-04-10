#include <target/types.h>
#include <target/lcd.h>
#define HMAX 320    //z 320  384
#define VMAX 240    //z 256
#define HREAL 320   //z 240  384
/* FRAME Buffer */
unsigned char __SD_LCD_BAR_BASE[VMAX*HMAX/8] __attribute__ ((aligned(64)));
unsigned char SD_LCD_BAR_dum_BASE[VMAX*HMAX/8] __attribute__ ((aligned(64)));
static int count = 0;

void init_lcd()
{
    memset(SD_LCD_BAR_BASE, 0x00, HMAX*VMAX/8);
    memset(SD_LCD_BAR_BASE_2, 0x00, HMAX*VMAX/8);
    init_lcd_regs();
}

void init_lcd_regs()
{
    int i=0;
    LCD_SET32(REG_LCD_VBARa, PHY(SD_LCD_BAR_BASE));
    LCD_SET32(REG_LCD_VBARb, PHY(SD_LCD_BAR_BASE_2));
#if 1 // CMC STN 320x240
    LCD_SET32(REG_LCD_CTRL,  0x00030000);
    LCD_SET32(REG_LCD_HTIM,  0x0807013f);
    LCD_SET32(REG_LCD_VTIM,  0x000000ef);
    LCD_SET32(REG_LCD_HVLEN, 0x00260000);
    LCD_SET32(REG_LCD_CTRL,  0x00030001);
#else // PrimeView TFT 800x600
    LCD_SET32(REG_LCD_CTRL,  0x00008088);
    LCD_SET32(REG_LCD_HTIM,  0x5f0f027f);
    LCD_SET32(REG_LCD_VTIM,  0x010901df);
    LCD_SET32(REG_LCD_HVLEN, 0x031f020c);
    LCD_SET32(REG_LCD_CTRL,  0x00008089);
#endif
    for(i=0;i<0x800;i=i+4) // fill clut
        LCD_SET32((REG_LCD_PCLT+i), 0x0003ffff);
}
