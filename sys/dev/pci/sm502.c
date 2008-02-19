/*
 * (C) Copyright 2002
 * Stäubli Faverges - <www.staubli.com>
 * Pierre AUBERT  p.aubert@staubli.com
 *
 * (C) Copyright 2005
 * Martin Krause TQ-Systems GmbH martin.krause@tqs.de
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

/*
 * Basic video support for SMI SM501 "Voyager" graphic controller
 */

/*
* Silicon Motion graphic interface for sm810/sm710/sm712 accelerator
 *
 *
 *  modification history
 *  --------------------
 *  04-18-2002 Rewritten for U-Boot <fgottschling@eltec.de>.
 */
#include <stdio.h>
#include <stdlib.h>

#include <dev/pci/pcivar.h>
#include <linux/types.h>
#include "linux/io.h"
#include <linux/pci.h>

#include "sm502.h"
#include "smtc2d.h"
#include "sm501hw.h"
#define _VIDEO_FB_H_

#define CONSOLE_BG_COL            0x00
#define CONSOLE_FG_COL            0xa0

#define DISPLAY_WIDTH 640
#define DISPLAY_HEIGHT 480
extern void delay(int);
/*
 * Graphic Data Format (GDF) bits for VIDEO_DATA_FORMAT
 */
#define GDF__8BIT_INDEX         0
#define GDF_15BIT_555RGB        1
#define GDF_16BIT_565RGB        2
#define GDF_32BIT_X888RGB       3
#define GDF_24BIT_888RGB        4
#define GDF__8BIT_332RGB        5
#define SCREEN_X_RES    640
#define SCREEN_Y_RES    480
#define SCREEN_BPP      16

#define isACCELENT      0 /* is Accelent platform */
#define FB_PHYSICAL_ADDR 0x08000000 /* Default for Accelent PXA255, other platform need to change */
#define REG_PHYSICAL_ADDR 0x0BE00000 /* Default for Accelent PXA255, other platform need to change */

#define waitforvsync() delay(400)

#if isACCELENT
#undef isPC
#undef isPCI
#define isPC 0
#define isPCI 0
#endif

#define NR_PALETTE      256
#define NR_RGB          2

#ifndef FIELD_OFFSET
#define FIELD_OFSFET(type, field)       ((unsigned long) (PUCHAR) &(((type *)0)->field))
#endif

#define RGB565_R_MASK                      0xF8          // Mask for red color
#define RGB565_G_MASK                      0xFC          // Mask for green color
#define RGB565_B_MASK                      0xF8          // Mask for blue color

#define RGB565_R_SHIFT                     8             // Number of bits to shift for red color
#define RGB565_G_SHIFT                     3             // Number of bits to shift for green color
#define RGB565_B_SHIFT                     3             // Number of bits to shift for blue color

#define RGB16(r, g, b) ( (unsigned short) ((((r) & RGB565_R_MASK) << RGB565_R_SHIFT) | (((g) & RGB565_G_MASK) << RGB565_G_SHIFT) | (((b) & RGB565_B_MASK) >> RGB565_B_SHIFT)) )


////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// Internal macros                                                            //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

#define _F_START(f)             (0 ? f)
#define _F_END(f)               (1 ? f)
#define _F_SIZE(f)              (1 + _F_END(f) - _F_START(f))
#define _F_MASK(f)              (((1 << _F_SIZE(f)) - 1) << _F_START(f))
#define _F_NORMALIZE(v, f)      (((v) & _F_MASK(f)) >> _F_START(f))
#define _F_DENORMALIZE(v, f)    (((v) << _F_START(f)) & _F_MASK(f))

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// Global macros                                                              //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

#define FIELD_GET(x, reg, field) \
( \
    _F_NORMALIZE((x), reg ## _ ## field) \
)

#define FIELD_SET(x, reg, field, value) \
( \
    (x & ~_F_MASK(reg ## _ ## field)) \
    | _F_DENORMALIZE(reg ## _ ## field ## _ ## value, reg ## _ ## field) \
)

#define FIELD_VALUE(x, reg, field, value) \
( \
    (x & ~_F_MASK(reg ## _ ## field)) \
    | _F_DENORMALIZE(value, reg ## _ ## field) \
)

#define FIELD_CLEAR(reg, field) \
( \
    ~ _F_MASK(reg ## _ ## field) \
)

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// Field Macros                                                               //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

#define FIELD_START(field)              (0 ? field)
#define FIELD_END(field)                (1 ? field)
#define FIELD_SIZE(field)               (1 + FIELD_END(field) - FIELD_START(field))
#define FIELD_MASK(field)               (((1 << (FIELD_SIZE(field)-1)) | ((1 << (FIELD_SIZE(field)-1)) - 1)) << FIELD_START(field))
#define FIELD_NORMALIZE(reg, field)     (((reg) & FIELD_MASK(field)) >> FIELD_START(field))
#define FIELD_DENORMALIZE(field, value) (((value) << FIELD_START(field)) & FIELD_MASK(field))

#define FIELD_INIT(reg, field, value)   FIELD_DENORMALIZE(reg ## _ ## field, \
                                                          reg ## _ ## field ## _ ## value)
#define FIELD_INIT_VAL(reg, field, value) \
                                        (FIELD_DENORMALIZE(reg ## _ ## field, value))
#define FIELD_VAL_SET(x, r, f, v)       x = x & ~FIELD_MASK(r ## _ ## f) \
                                              | FIELD_DENORMALIZE(r ## _ ## f, r ## _ ## f ## _ ## v)


extern struct pci_device *vga_dev;
extern int pci_read_config_dword(struct pci_device *linuxpd, int reg, u32 *val);
/******************************************************************************/
/* Export Graphic Driver Control                                              */
/******************************************************************************/

typedef struct {
    int isaBase;
    unsigned int pciBase;
    unsigned int dprBase;
    unsigned int vprBase;
    unsigned int cprBase;
    int frameAdrs;
    unsigned int memSize;
    unsigned int mode;
    unsigned int gdfIndex;
    unsigned int gdfBytesPP;
    unsigned int fg;
    unsigned int bg;
    unsigned int plnSizeX;
    unsigned int plnSizeY;
    unsigned int winSizeX;
    unsigned int winSizeY;
    char modeIdent[80];
} GraphicDevice;
/******************************************************************************/
/* Export Graphic Functions                                                   */
/******************************************************************************/

int video_hw_init (void);       /* returns GraphicDevice struct or NULL */

#ifdef VIDEO_HW_BITBLT
void video_hw_bitblt (
    unsigned int bpp,             /* bytes per pixel */
    unsigned int src_x,           /* source pos x */
    unsigned int src_y,           /* source pos y */
    unsigned int dst_x,           /* dest pos x */
    unsigned int dst_y,           /* dest pos y */
    unsigned int dim_x,           /* frame width */
    unsigned int dim_y            /* frame height */
    );
#endif

#ifdef VIDEO_HW_RECTFILL
void video_hw_rectfill (
    unsigned int bpp,             /* bytes per pixel */
    unsigned int dst_x,           /* dest pos x */
    unsigned int dst_y,           /* dest pos y */
    unsigned int dim_x,           /* frame width */
    unsigned int dim_y,           /* frame height */
    unsigned int color            /* fill color */
     );
#endif

void video_set_lut (
    unsigned int index,           /* color number */
    unsigned char r,              /* red */
    unsigned char g,              /* green */
    unsigned char b               /* blue */
    );
#ifdef CONFIG_VIDEO_HW_CURSOR
void video_set_hw_cursor(int x, int y); /* x y in pixel */
void video_init_hw_cursor(int font_width, int font_height);
#endif

//#endif /*_VIDEO_FB_H_ */                                                                                                                                 

const SMI_REGS init_regs[] =
{

#ifdef X640x480
        {0x00004, 0x0},
        {0x00048, 0x00021807}, //0x00021807
        {0x0004C, 0x091a0a01},
        {0x00054, 0x1},
		{0x000040,0x0002180f},
		{0x000044,0x60090208},
        {0x00054, 0x0},
#if defined(CONFIG_VIDEO_SM501_8BPP)
{0x80000, 0x0f013104}, //0x0f013106
#endif
#if defined(CONFIG_VIDEO_SM501_16BPP)        
{0x80000, 0x0f013105}, //0x0f013106
#endif
#if defined(CONFIG_VIDEO_SM501_32BPP)
        {0x80000, 0x0f013106}, //0x0f013106
#endif
        {0x80004, 0xc428bb17},
        {0x8000C, 0x00000000},
        {0x80010, 0x05000500},//0x0a000a00//administrate fb width
        {0x80014, 0x02800000},//0x02800000->640;0x03200000->800
        {0x80018, 0x01e00000}, //0x01e00000->30(480);0x02600000->600
        {0x8001C, 0x00000000},
        {0x80020, 0x01e00280},//0x01e00280->640x480;0x02600320->800x600
        {0x80024, 0x02fa027f},//HDE-0x02fa027f->640x480;
        {0x80028, 0x004a028b},//0x004a028b->640x480;
        {0x8002C, 0x020c01df},//VDE-0x020c01df->640x480;
        {0x80030, 0x000201e9},//0x000201e9->640x480;
#if defined(CONFIG_VIDEO_SM501_8BPP)
        {0x80040, 0x00010000},
        {0x80080, 0x00010000},
#endif
#if defined(CONFIG_VIDEO_SM501_16BPP)
        {0x80040, 0x00010001},//rgb565
        {0x80080, 0x00010001},//rgb565
#endif
#if defined(CONFIG_VIDEO_SM501_32BPP)
        {0x80040, 0x00010002},//rgb565
        {0x80080, 0x00010001},//rgb565
#endif
        {0x8004c, 0x00800000},//rgb565
//        {0x80080, 0x00010001},//rgb565
{0x000048,0x0002180f},
{0x00004c,0x60090208},
        {0x00054, 0x0},
	{0x80200, 0x00010000},
	{0x000074,0x00020f1f},
#endif
#ifdef X800x600
{0x00000004,0x00000000}, //0x00000001
{0x00000048,0x0002180f},
{0x0000004c,0x60120208},
{0x00000054,0x00000001},//0x00000000
{0x00000040,0x0002180f},
{0x00000044,0x60120208},
{0x00000054,0x00000000},//0x00000000
{0x00000074,0x0002030a},

#if defined(CONFIG_VIDEO_SM501_8BPP)
{0x80000, 0x0f010104},
#endif
#if defined(CONFIG_VIDEO_SM501_16BPP)
{0x80000, 0x0f010105},
#endif
#if defined(CONFIG_VIDEO_SM501_32BPP)
        {0x80000, 0x0f010106},
#endif

{0x00080004,0xf532ea1e},
{0x0008000c,0x00000000},
{0x00080010,0x06400640},
{0x00080014,0x03200000},
{0x00080018,0x02580000},
{0x0008001c,0x00000000},
{0x00080020,0x02580320},
{0x00080024,0x041f031f},
{0x00080028,0x00800347},
{0x0008002c,0x02730257},
{0x00080030,0x00040258},

#if defined(CONFIG_VIDEO_SM501_8BPP)
        {0x80040, 0x00010000},
        {0x80080, 0x00010000},
#endif
#if defined(CONFIG_VIDEO_SM501_16BPP)
        {0x80040, 0x00010001},//rgb565
        {0x80080, 0x00010001},//rgb565
#endif
#if defined(CONFIG_VIDEO_SM501_32BPP)
        {0x80040, 0x00010002},//rgb565
        {0x80080, 0x00010001},//rgb565
#endif

{0x8004c, 0x00800000},//rgb565
{0x00080200,0x00010000},
{0x00000004,0x00000001},
#endif

#ifdef X1024x768
//{0x00000000,0x04100000},
{0x00000048,0x0002180f},
{0x0000004c,0x60010208},
{0x00000054,0x00000001},//0x00000000
{0x00000040,0x0002180f},
{0x00000044,0x60010208},
{0x00000054,0x00000000},
{0x00000074,0x0002051b},

#if defined(CONFIG_VIDEO_SM501_8BPP)
{0x80000, 0x0f013104},
#endif
#if defined(CONFIG_VIDEO_SM501_16BPP)
{0x80000, 0x0f013105},
#endif
#if defined(CONFIG_VIDEO_SM501_32BPP)
{0x80000, 0x0f013106},
#endif
{0x00080004,0xf532ea1e},
{0x0008000c,0x00000000},
{0x00080010,0x08000800},
{0x00080014,0x04000000},
{0x00080018,0x03000000},
{0x0008001c,0x00000000},
{0x00080020,0x02ff03ff},
{0x00080024,0x052503ff},
{0x00080028,0x00C80424},//0x 00880417
{0x0008002c,0x032502ff},
{0x00080030,0x00060302},

#if defined(CONFIG_VIDEO_SM501_8BPP)
        {0x80040, 0x00010000},
        {0x80080, 0x00010000},
#endif
#if defined(CONFIG_VIDEO_SM501_16BPP)
        {0x80040, 0x00010001},//rgb565
        {0x80080, 0x00010001},//rgb565
#endif
#if defined(CONFIG_VIDEO_SM501_32BPP)
        {0x80040, 0x00010002},//rgb565
        {0x80080, 0x00010001},//rgb565
#endif
{0x8004c, 0x00800000},//rgb565
{0x00080200,0x00013005},
{0x00000004,0x00000001}, //0x00000001
#endif
        {0, 0}
};

#define CONFIG_VIDEO_SM502

#ifdef CONFIG_VIDEO_SM502

#define read8(ptrReg)                \
    *(volatile unsigned char *)(sm502.isaBase + ptrReg)

#define write8(ptrReg,value) \
    *(volatile unsigned char *)(sm502.isaBase + ptrReg) = value

#define read16(ptrReg) \
    (*(volatile unsigned short *)(sm502.isaBase + ptrReg))

#define write16(ptrReg,value) \
    (*(volatile unsigned short *)(sm502.isaBase + ptrReg) = value)

#define read32(ptrReg) \
    (*(volatile unsigned int *)(sm502.isaBase + ptrReg))

#define write32(ptrReg, value) \
    (*(volatile unsigned int *)(sm502.isaBase + ptrReg) = value)

#define regWrite32(ptrReg, value)  (*(volatile unsigned int *)(sm502.isaBase + 0x100000 + ptrReg) = value)

#define SMTC_write2Dreg(ptrReg, value) (*(volatile unsigned int *)(sm502.isaBase + 0x100000 + ptrReg) = value)

#define smi_mmiowl_1(dat,reg) (*(volatile unsigned int *)(sm502.isaBase + reg) = dat)

#define regRead32(ptrReg)  (*(volatile unsigned int *)(sm502.isaBase + ptrReg))

GraphicDevice sm502;

/////////////////////////////////
#if 0
void smi_setmode(void)
{

	int crt_out=0;

        /* Just blast in some control values based upon the chip
         * documentation.  We use the internal memory, I don't know
         * how to determine the amount available yet.
         */
        smi_mmiowl_1(0x00021827, POWER_MODE1_GATE);
        smi_mmiowl_1(0x011A0A09, POWER_MODE1_CLOCK);
        smi_mmiowl_1(0x00000001, POWER_MODE_CTRL);
        smi_mmiowl_1(0x08000800, PANEL_FB_WIDTH);
        smi_mmiowl_1(0x04000000, PANEL_WINDOW_WIDTH);
        smi_mmiowl_1(0x03000000, PANEL_WINDOW_HEIGHT);
        smi_mmiowl_1(0x00000000, PANEL_PLANE_TL);
        smi_mmiowl_1(0x02FF03FF, PANEL_PLANE_BR);
        smi_mmiowl_1(0x05D003FF, PANEL_HORIZONTAL_TOTAL);
        smi_mmiowl_1(0x00C80424, PANEL_HORIZONTAL_SYNC);
        smi_mmiowl_1(0x032502FF, PANEL_VERTICAL_TOTAL);
        smi_mmiowl_1(0x00060302, PANEL_VERTICAL_SYNC);
        smi_mmiowl_1(0x00013905, PANEL_DISPLAY_CTRL);
        smi_mmiowl_1(0x0002187F, POWER_MODE1_GATE);
        smi_mmiowl_1(0x01011801, POWER_MODE1_CLOCK);
        smi_mmiowl_1(0x00000001, POWER_MODE_CTRL);
        smi_mmiowl_1(0x00000001, MISC_CTRL);
        if (crt_out) {
                /* Just sent the panel out to the CRT for now.
                */
                smi_mmiowl_1(0xb4000000, CRT_FB_ADDRESS);
                smi_mmiowl_1(0x08000800, CRT_FB_WIDTH);
                smi_mmiowl_1(0x05D003FF, CRT_HORIZONTAL_TOTAL);
                smi_mmiowl_1(0x00C80424, CRT_HORIZONTAL_SYNC);
                smi_mmiowl_1(0x032502FF, CRT_VERTICAL_TOTAL);
                smi_mmiowl_1(0x00060302, CRT_VERTICAL_SYNC);
                smi_mmiowl_1(0x007FF800, CRT_HWC_ADDRESS);
                smi_mmiowl_1(0x00010305, CRT_DISPLAY_CTRL);
                smi_mmiowl_1(0x00000001, MISC_CTRL);
        }
}

 void smi_setmode_1024(void)
{
        smi_mmiowl_1(0x07F127C2, DRAM_CTRL);
//        smi_mmiowl_1(0x02000020, PANEL_HWC_ADDRESS);
//        smi_mmiowl_1(0x007FF800, PANEL_HWC_ADDRESS);
        smi_mmiowl_1(0x00021827, POWER_MODE1_GATE);
        smi_mmiowl_1(0x011A0A09, POWER_MODE1_CLOCK);
        smi_mmiowl_1(0x00000001, POWER_MODE_CTRL);
        smi_mmiowl_1(0x08000800, PANEL_FB_WIDTH);
        smi_mmiowl_1(0x04000000, PANEL_WINDOW_WIDTH);
        smi_mmiowl_1(0x03000000, PANEL_WINDOW_HEIGHT);
        smi_mmiowl_1(0x00000000, PANEL_PLANE_TL);
        smi_mmiowl_1(0x02FF03FF, PANEL_PLANE_BR);
        smi_mmiowl_1(0x05D003FF, PANEL_HORIZONTAL_TOTAL);
        smi_mmiowl_1(0x00C80424, PANEL_HORIZONTAL_SYNC);
        smi_mmiowl_1(0x032502FF, PANEL_VERTICAL_TOTAL);
        smi_mmiowl_1(0x00060302, PANEL_VERTICAL_SYNC);
        smi_mmiowl_1(0x00013905, PANEL_DISPLAY_CTRL);
        smi_mmiowl_1(0x01013105, PANEL_DISPLAY_CTRL);
        smi_mmiowl_1(0x03013905, PANEL_DISPLAY_CTRL);
        smi_mmiowl_1(0x07013905, PANEL_DISPLAY_CTRL);
        smi_mmiowl_1(0x0F013905, PANEL_DISPLAY_CTRL);
        smi_mmiowl_1(0x0002187F, POWER_MODE1_GATE);
        smi_mmiowl_1(0x01011801, POWER_MODE1_CLOCK);
        smi_mmiowl_1(0x00000001, POWER_MODE_CTRL);

        smi_mmiowl_1(0x00000000, PANEL_PAN_CTRL);
        smi_mmiowl_1(0x00000000, PANEL_COLOR_KEY);
        smi_mmiowl_1(0x00000001, MISC_CTRL);

}
#endif

/////////////////////////////////

int pci_read_config_dword(struct pci_device *linuxpd, int reg, u32 *val)
{
        if ((reg & 3) || reg < 0 || reg >= 0x100) {
                printf ("pci_read_config_dword: bad reg %x\n", reg);
                return -1;
        }
        *val=_pci_conf_read(linuxpd->pa.pa_tag, reg);
        return 0;
}

//////////////////////////////////


/*-----------------------------------------------------------------------------
 * board_video_init -- init de l'EPSON, config du CS
 *-----------------------------------------------------------------------------
 */
int board_video_init (void)
{
    int mimoaddr;	
    struct pci_device *pdev;
    if (vga_dev != NULL){
	pdev = vga_dev;
    	pci_read_config_dword(pdev,0x14,(int *)&mimoaddr);
    	mimoaddr = 0xb0000000|mimoaddr;
	printf("mimobase=0x%x\n",mimoaddr);
	}
    return(mimoaddr);
}

/*-----------------------------------------------------------------------------
 * board_validate_screen --
 *-----------------------------------------------------------------------------
 */
void board_validate_screen (unsigned int base)
{
}

/*-----------------------------------------------------------------------------
 * board_get_regs --
 *-----------------------------------------------------------------------------
 */
const SMI_REGS *board_get_regs (void)
{
    return (init_regs);
}

/*-----------------------------------------------------------------------------
 * board_get_width --
 *-----------------------------------------------------------------------------
 */
int board_get_width (void)
{
    return (DISPLAY_WIDTH);
}

/*-----------------------------------------------------------------------------
 * board_get_height --
 *-----------------------------------------------------------------------------
 */
int board_get_height (void)
{
    return (DISPLAY_HEIGHT);
}

int  board_video_get_fb (void)
{

    int fbaddr;
    struct pci_device *pdev;
    if (vga_dev != NULL)
	{
            pdev = vga_dev;
    	    pci_read_config_dword(pdev,0x10,(int *)&fbaddr);
    	    fbaddr = 0xb0000000|fbaddr;
	    printf("fbaddr=0x%x\n",fbaddr);
	}
    return(fbaddr);
}

/*-----------------------------------------------------------------------------
 * SmiSetRegs --
 *-----------------------------------------------------------------------------
 */
extern void delay(int msec);
		
static void SmiSetRegs (void)
{
	/*
	 * The content of the chipset register depends on the board (clocks,
	 * ...)
	 */
	SMI_REGS *preg;
	preg = board_get_regs();
	while (preg->Index) {
		write32 (preg->Index, preg->Value);
		/*
		 * Insert a delay between
		 */
		delay(5);
		preg ++;
	}		
#ifdef DEVBD2F_SM502
if(!getenv("rgb24"))
write32(0x80000,0x0f413105); // bit21-20=1: 9-bit RGB 3:3:3.
#endif
		
}

/*-----------------------------------------------------------------------------
 * 2D ACCRATE --
 *-----------------------------------------------------------------------------
 */

#define CONFIG_FB_SM501 1
typedef unsigned int ULONG;
typedef unsigned char UCHAR;
typedef int INT ;
unsigned char smtc_de_busy = 0;

void deWaitForNotBusy(void)
{
        unsigned long i = 0x1000000;
        while (i--)
        {
#ifdef CONFIG_FB_SM501
        unsigned long dwVal = regRead32(CMD_INTPR_STATUS);
        if ((FIELD_GET(dwVal, CMD_INTPR_STATUS, 2D_ENGINE)      == CMD_INTPR_STATUS_2D_ENGINE_IDLE) &&
            (FIELD_GET(dwVal, CMD_INTPR_STATUS, 2D_FIFO)        == CMD_INTPR_STATUS_2D_FIFO_EMPTY) &&
            (FIELD_GET(dwVal, CMD_INTPR_STATUS, 2D_SETUP)       == CMD_INTPR_STATUS_2D_SETUP_IDLE) &&
            (FIELD_GET(dwVal, CMD_INTPR_STATUS, CSC_STATUS)     == CMD_INTPR_STATUS_CSC_STATUS_IDLE) &&
            (FIELD_GET(dwVal, CMD_INTPR_STATUS, 2D_MEMORY_FIFO) == CMD_INTPR_STATUS_2D_MEMORY_FIFO_EMPTY) &&
            (FIELD_GET(dwVal, CMD_INTPR_STATUS, COMMAND_FIFO)   == CMD_INTPR_STATUS_COMMAND_FIFO_EMPTY))
            break;
#endif
#ifdef CONFIG_FB_SM7XX
        if ((smtc_seqr(0x16) & 0x18) == 0x10)
            break;
#endif
        }
        smtc_de_busy = 0;
}

// Program new power mode.
void setPower(unsigned long nGates, unsigned long Clock)
{
        unsigned long gate_reg, clock_reg;
        unsigned long control_value;

        // Get current power mode.
        control_value = FIELD_GET(regRead32(POWER_MODE_CTRL),
                                                          POWER_MODE_CTRL,
                                                          MODE);

        switch (control_value)
        {
        case POWER_MODE_CTRL_MODE_MODE0:

                // Switch from mode 0 to mode 1.
                gate_reg = POWER_MODE1_GATE;
                clock_reg = POWER_MODE1_CLOCK;
                control_value = FIELD_SET(control_value,
                                          POWER_MODE_CTRL, MODE, MODE1);
                break;

        case POWER_MODE_CTRL_MODE_MODE1:
        case POWER_MODE_CTRL_MODE_SLEEP:

                // Switch from mode 1 or sleep to mode 0.
                gate_reg = POWER_MODE0_GATE;
                clock_reg = POWER_MODE0_CLOCK;
                control_value = FIELD_SET(control_value, POWER_MODE_CTRL, MODE, MODE0);
                break;

        default:
                // Invalid mode
                return;
        }

        // Program new power mode.
        regWrite32(gate_reg, nGates);
        regWrite32(clock_reg, Clock);
        regWrite32(POWER_MODE_CTRL, control_value);

        // When returning from sleep, wait until finished.
        while (FIELD_GET(regRead32(POWER_MODE_CTRL),
                                         POWER_MODE_CTRL,
                                         SLEEP_STATUS) == POWER_MODE_CTRL_SLEEP_STATUS_ACTIVE) ;
}


INT deFillRectModify( ULONG nDestBase,ULONG nX1, ULONG nY1, ULONG nX2, ULONG nY2,
ULONG nColor)
{
    INT result;
        ULONG DeltaX;
        ULONG DeltaY;

   
            /* Determine delta X */
            if (nX2 < nX1)
            {
                    DeltaX = nX1 - nX2 + 1;
                    nX1 = nX2;
            }
            else
            {
                    DeltaX = nX2 - nX1 + 1;
            }

            /* Determine delta Y */
            if (nY2 < nY1)
            {
                    DeltaY = nY1 - nY2 + 1;
                    nY1 = nY2;
            }
            else
            {
                    DeltaY = nY2 - nY1 + 1;
            }

//            deWaitForNotBusy();
            regWrite32(DE_WINDOW_DESTINATION_BASE,          
                                FIELD_VALUE(0, DE_WINDOW_DESTINATION_BASE, ADDRESS, nDestBase));
            regWrite32(DE_FOREGROUND,
                    FIELD_VALUE(0, DE_FOREGROUND, COLOR, nColor));
            regWrite32(DE_DESTINATION,
                    FIELD_SET  (0, DE_DESTINATION, WRAP, DISABLE) |
                    FIELD_VALUE(0, DE_DESTINATION, X,    nX1)     |
                    FIELD_VALUE(0, DE_DESTINATION, Y,    nY1));                

            regWrite32(DE_DIMENSION,
                    FIELD_VALUE(0, DE_DIMENSION, X,    DeltaX) |
                    FIELD_VALUE(0, DE_DIMENSION, Y_ET, DeltaY));                
                
            regWrite32(DE_CONTROL,
                    FIELD_SET  (0, DE_CONTROL, STATUS,     START)          |
                    FIELD_SET  (0, DE_CONTROL, DIRECTION,  LEFT_TO_RIGHT)  |
                    FIELD_SET  (0, DE_CONTROL, LAST_PIXEL, OFF)            |
                    FIELD_SET  (0, DE_CONTROL, COMMAND,    RECTANGLE_FILL) |
                    FIELD_SET  (0, DE_CONTROL, ROP_SELECT, ROP2)           |
                    FIELD_VALUE(0, DE_CONTROL, ROP,        0x0C));

        result = DDK_OK;
   
    return result;
}

/**********************************************************************
 *
 * deCopy
 *
 * Purpose
 *    Copy a rectangular area of the source surface to a destination surface
 *
 * Remarks
 *       Source bitmap must have the same color depth (BPP) as the destination bitmap.
 *
**********************************************************************/
void deCopy(unsigned long dst_base,
            unsigned long dst_pitch,
            unsigned long dst_BPP,
            unsigned long dst_X,
            unsigned long dst_Y,
            unsigned long dst_width,
            unsigned long dst_height,
            unsigned long src_base,
            unsigned long src_pitch,
            unsigned long src_X,
            unsigned long src_Y,
            pTransparent pTransp,
            unsigned char nROP2)
{
    unsigned long nDirection = 0;
    unsigned long nTransparent = 0;
    unsigned long opSign = 1;    // Direction of ROP2 operation: 1 = Left to Right, (-1) = Right to Left
    unsigned long xWidth = 192 / (dst_BPP / 8); // xWidth is in pixels
    unsigned long de_ctrl = 0;

//    deWaitForNotBusy();

    SMTC_write2Dreg(DE_WINDOW_DESTINATION_BASE, FIELD_VALUE(0, DE_WINDOW_DESTINATION_BASE, ADDRESS, dst_base));

    SMTC_write2Dreg(DE_WINDOW_SOURCE_BASE, FIELD_VALUE(0, DE_WINDOW_SOURCE_BASE, ADDRESS, src_base));

    if (dst_pitch && src_pitch)
    {
        SMTC_write2Dreg(DE_PITCH,
            FIELD_VALUE(0, DE_PITCH, DESTINATION, dst_pitch) |
            FIELD_VALUE(0, DE_PITCH, SOURCE,      src_pitch));

        SMTC_write2Dreg(DE_WINDOW_WIDTH,
            FIELD_VALUE(0, DE_WINDOW_WIDTH, DESTINATION, dst_pitch) |
            FIELD_VALUE(0, DE_WINDOW_WIDTH, SOURCE,      src_pitch));
    }

   /* Set transparent bits if necessary */
    if (pTransp != NULL)
    {
        nTransparent = pTransp->match | pTransp->select | pTransp->control;

        /* Set color compare register */
        SMTC_write2Dreg(DE_COLOR_COMPARE,
            FIELD_VALUE(0, DE_COLOR_COMPARE, COLOR, pTransp->color));
    }

    /* Determine direction of operation */
    if (src_Y < dst_Y)
    {
    /* +----------+
    |S         |
    |   +----------+
    |   |      |   |
    |   |      |   |
    +---|------+   |
    |         D|
        +----------+ */

        nDirection = BOTTOM_TO_TOP;
    }
    else if (src_Y > dst_Y)
    {
    /* +----------+
    |D         |
    |   +----------+
    |   |      |   |
    |   |      |   |
    +---|------+   |
    |         S|
        +----------+ */

        nDirection = TOP_TO_BOTTOM;
    }
    else
    {
        /* src_Y == dst_Y */

        if (src_X <= dst_X)
        {
        /* +------+---+------+
        |S     |   |     D|
        |      |   |      |
        |      |   |      |
        |      |   |      |
            +------+---+------+ */

            nDirection = RIGHT_TO_LEFT;
        }
        else
        {
            /* src_X > dst_X */

            /* +------+---+------+
            |D     |   |     S|
            |      |   |      |
            |      |   |      |
            |      |   |      |
            +------+---+------+ */

            nDirection = LEFT_TO_RIGHT;
        }
    }

    if ((nDirection == BOTTOM_TO_TOP) || (nDirection == RIGHT_TO_LEFT))
    {
        src_X += dst_width - 1;
        src_Y += dst_height - 1;
        dst_X += dst_width - 1;
        dst_Y += dst_height - 1;
        opSign = (-1);
    }

    /* Workaround for 192 byte hw bug */
    if ((nROP2 != 0x0C) && ((dst_width * (dst_BPP / 8)) >= 192))
    {
        /* Perform the ROP2 operation in chunks of (xWidth * dst_height) */
        while (1)
        {
  //          deWaitForNotBusy();
            SMTC_write2Dreg(DE_SOURCE,
                FIELD_SET  (0, DE_SOURCE, WRAP, DISABLE) |
                FIELD_VALUE(0, DE_SOURCE, X_K1, src_X)   |
                FIELD_VALUE(0, DE_SOURCE, Y_K2, src_Y));
            SMTC_write2Dreg(DE_DESTINATION,
                FIELD_SET  (0, DE_DESTINATION, WRAP, DISABLE) |
                FIELD_VALUE(0, DE_DESTINATION, X,    dst_X)  |
                FIELD_VALUE(0, DE_DESTINATION, Y,    dst_Y));
            SMTC_write2Dreg(DE_DIMENSION,
                FIELD_VALUE(0, DE_DIMENSION, X,    xWidth) |
                FIELD_VALUE(0, DE_DIMENSION, Y_ET, dst_height));
            de_ctrl = FIELD_VALUE(0, DE_CONTROL, ROP, nROP2) |
                nTransparent |
                FIELD_SET(0, DE_CONTROL, ROP_SELECT, ROP2) |
                FIELD_SET(0, DE_CONTROL, COMMAND, BITBLT) |
                ((nDirection == 1) ? FIELD_SET(0, DE_CONTROL, DIRECTION, RIGHT_TO_LEFT)
                : FIELD_SET(0, DE_CONTROL, DIRECTION, LEFT_TO_RIGHT)) |
                FIELD_SET(0, DE_CONTROL, STATUS, START);
            SMTC_write2Dreg(DE_CONTROL, de_ctrl);

            src_X += (opSign * xWidth);
            dst_X += (opSign * xWidth);
            dst_width -= xWidth;

            if (dst_width <= 0)
            {
                /* ROP2 operation is complete */
                break;
            }

            if (xWidth > dst_width)
            {
                xWidth = dst_width;
            }
        }
    }
    else
    {
//        deWaitForNotBusy();
        SMTC_write2Dreg(DE_SOURCE,
            FIELD_SET  (0, DE_SOURCE, WRAP, DISABLE) |
            FIELD_VALUE(0, DE_SOURCE, X_K1, src_X)   |
            FIELD_VALUE(0, DE_SOURCE, Y_K2, src_Y));
        SMTC_write2Dreg(DE_DESTINATION,
            FIELD_SET  (0, DE_DESTINATION, WRAP, DISABLE) |
            FIELD_VALUE(0, DE_DESTINATION, X,    dst_X)  |
           FIELD_VALUE(0, DE_DESTINATION, Y,    dst_Y));
        SMTC_write2Dreg(DE_DIMENSION,
            FIELD_VALUE(0, DE_DIMENSION, X,    dst_width) |
            FIELD_VALUE(0, DE_DIMENSION, Y_ET, dst_height));
        de_ctrl = FIELD_VALUE(0, DE_CONTROL, ROP, nROP2) |
            nTransparent |
            FIELD_SET(0, DE_CONTROL, ROP_SELECT, ROP2) |
            FIELD_SET(0, DE_CONTROL, COMMAND, BITBLT) |
            ((nDirection == 1) ? FIELD_SET(0, DE_CONTROL, DIRECTION, RIGHT_TO_LEFT)
            : FIELD_SET(0, DE_CONTROL, DIRECTION, LEFT_TO_RIGHT)) |
            FIELD_SET(0, DE_CONTROL, STATUS, START);
        SMTC_write2Dreg(DE_CONTROL, de_ctrl);
    }

//    smtc_de_busy = 1;
}

/**********************************************************************
 *
 * deInit
 *
 * Purpose
 *    Drawing engine initialization.
 *
 **********************************************************************/
void deInit(unsigned int nModeWidth, unsigned int nModeHeight, unsigned int bpp)
{
#ifdef CONFIG_FB_SM501
        // Get current power configuration.
        unsigned int gate, clock;

        gate  = regRead32(CURRENT_POWER_GATE);
        clock = regRead32(CURRENT_POWER_CLOCK);

        // Enable 2D Drawing Engine
        gate = FIELD_SET(gate, CURRENT_POWER_GATE, 2D, ENABLE);
        setPower(gate, clock);
#endif

        SMTC_write2Dreg(DE_CLIP_TL,
                FIELD_VALUE(0, DE_CLIP_TL, TOP,     0)       |
                FIELD_SET  (0, DE_CLIP_TL, STATUS,  DISABLE) |
                FIELD_SET  (0, DE_CLIP_TL, INHIBIT, OUTSIDE) |
                FIELD_VALUE(0, DE_CLIP_TL, LEFT,    0));

    SMTC_write2Dreg(DE_PITCH,
                FIELD_VALUE(0, DE_PITCH, DESTINATION, nModeWidth) |
                FIELD_VALUE(0, DE_PITCH, SOURCE,      nModeWidth));

    SMTC_write2Dreg(DE_WINDOW_WIDTH,
                FIELD_VALUE(0, DE_WINDOW_WIDTH, DESTINATION, nModeWidth) |
                FIELD_VALUE(0, DE_WINDOW_WIDTH, SOURCE,      nModeWidth));

    switch (bpp)
    {
    case 8:
        SMTC_write2Dreg(DE_STRETCH_FORMAT,
            FIELD_SET  (0, DE_STRETCH_FORMAT, PATTERN_XY,    NORMAL) |
            FIELD_VALUE(0, DE_STRETCH_FORMAT, PATTERN_Y,     0)      |
            FIELD_VALUE(0, DE_STRETCH_FORMAT, PATTERN_X,     0)      |
            FIELD_SET  (0, DE_STRETCH_FORMAT, PIXEL_FORMAT,  8)      |
            FIELD_SET  (0, DE_STRETCH_FORMAT, ADDRESSING,        XY)     |
            FIELD_VALUE(0, DE_STRETCH_FORMAT, SOURCE_HEIGHT, 3));
        break;
    case 32:
        SMTC_write2Dreg(DE_STRETCH_FORMAT,
            FIELD_SET  (0, DE_STRETCH_FORMAT, PATTERN_XY,    NORMAL) |
            FIELD_VALUE(0, DE_STRETCH_FORMAT, PATTERN_Y,     0)      |
            FIELD_VALUE(0, DE_STRETCH_FORMAT, PATTERN_X,     0)      |
            FIELD_SET  (0, DE_STRETCH_FORMAT, PIXEL_FORMAT,  32)     |
            FIELD_SET  (0, DE_STRETCH_FORMAT, ADDRESSING,        XY)     |
            FIELD_VALUE(0, DE_STRETCH_FORMAT, SOURCE_HEIGHT, 3));
        break;
    case 16:
    default:
        SMTC_write2Dreg(DE_STRETCH_FORMAT,
            FIELD_SET  (0, DE_STRETCH_FORMAT, PATTERN_XY,    NORMAL) |
            FIELD_VALUE(0, DE_STRETCH_FORMAT, PATTERN_Y,     0)      |
            FIELD_VALUE(0, DE_STRETCH_FORMAT, PATTERN_X,     0)      |
            FIELD_SET  (0, DE_STRETCH_FORMAT, PIXEL_FORMAT,  16)     |
            FIELD_SET  (0, DE_STRETCH_FORMAT, ADDRESSING,        XY)     |
            FIELD_VALUE(0, DE_STRETCH_FORMAT, SOURCE_HEIGHT, 3));
        break;
    }

        SMTC_write2Dreg(DE_MASKS,
                FIELD_VALUE(0, DE_MASKS, BYTE_MASK, 0xFFFF) |
                FIELD_VALUE(0, DE_MASKS, BIT_MASK,  0xFFFF));
        SMTC_write2Dreg(DE_COLOR_COMPARE_MASK,
                FIELD_VALUE(0, DE_COLOR_COMPARE_MASK, MASKS, 0xFFFFFF));
        SMTC_write2Dreg(DE_COLOR_COMPARE,
                FIELD_VALUE(0, DE_COLOR_COMPARE, COLOR, 0xFFFFFF));
}


void deCopyModify(ULONG nbpp,ULONG nSrcBase,ULONG nSrcWidth, ULONG nSrcX, ULONG nSrcY,
                        ULONG nDestBase,ULONG nDestWidth, ULONG nDestX,ULONG nDestY,
                        ULONG nWidth, ULONG nHeight,  
                        UCHAR nROP2)
{
        INT ret_value;
        ULONG nDirection = 0;
    ULONG opSign = 1;    // Direction of ROP2 operation: 1 = Left to Right, (-1) =Right to Left
    ULONG xWidth = 192 / (nbpp / 8); // xWidth is in pixels
    ULONG de_ctrl = 0;

        regWrite32(DE_PITCH,FIELD_VALUE(0, DE_PITCH, DESTINATION, nDestWidth) |FIELD_VALUE(0, DE_PITCH, SOURCE, nSrcWidth));
        regWrite32(DE_WINDOW_WIDTH,FIELD_VALUE(0, DE_WINDOW_WIDTH, DESTINATION, nDestWidth) | FIELD_VALUE(0, DE_WINDOW_WIDTH, SOURCE,nSrcWidth));
        regWrite32(DE_WINDOW_SOURCE_BASE,FIELD_VALUE(0, DE_WINDOW_SOURCE_BASE, ADDRESS, nSrcBase));
        regWrite32(DE_WINDOW_DESTINATION_BASE, FIELD_VALUE(0, DE_WINDOW_DESTINATION_BASE, ADDRESS, nDestBase));

        if (nSrcY < nDestY)
            {
                /* +----------+
                   |S         |
                   |   +----------+
                   |   |      |   |
                   |   |      |   |
                   +---|------+   |
                       |         D|
                       +----------+ */

                nDirection = BOTTOM_TO_TOP;
            }
            else if (nSrcY > nDestY)
            {
                /* +----------+
                   |D         |
                   |   +----------+
                   |   |      |   |
                   |   |      |   |
                   +---|------+   |
                       |         S|
                       +----------+ */

                nDirection = TOP_TO_BOTTOM;
            }
            else
            {
                /* nSrcY == nDestY */

                if (nSrcX <= nDestX)
                {
                    /* +------+---+------+
                       |S     |   |     D|
                       |      |   |      |
                       |      |   |      |
                       |      |   |      |
                       +------+---+------+ */

                    nDirection = RIGHT_TO_LEFT;
                }
                else
                {
                    /* nSrcX > nDestX */

                    /* +------+---+------+
                       |D     |   |     S|
                       |      |   |      |
                       |      |   |      |
                       |      |   |      |
                       +------+---+------+ */

                    nDirection = LEFT_TO_RIGHT;
                }
            }

            if ((nDirection == BOTTOM_TO_TOP) || (nDirection == RIGHT_TO_LEFT))
            {
                nSrcX += nWidth - 1;
                nSrcY += nHeight - 1;
                nDestX += nWidth - 1;
                nDestY += nHeight - 1;
                opSign = (-1);
            } 
                        if ((nROP2 != 0x0C) && ((nWidth * nbpp/ 8) >= 192))
            {
                /* Perform the ROP2 operation in chunks of (xWidth * nHeight) */
                while (1)
                {
            //        deWaitForNotBusy();
                                        regWrite32(DE_SOURCE,
                                                FIELD_SET  (0, DE_SOURCE, WRAP, DISABLE) |
                                                FIELD_VALUE(0, DE_SOURCE, X_K1, nSrcX)   |
                                                FIELD_VALUE(0, DE_SOURCE, Y_K2, nSrcY));
                                        regWrite32(DE_DESTINATION,
                                                FIELD_SET  (0, DE_DESTINATION, WRAP, DISABLE) |
                                                FIELD_VALUE(0, DE_DESTINATION, X,    nDestX)  |
                                                FIELD_VALUE(0, DE_DESTINATION, Y,    nDestY));
                                        regWrite32(DE_DIMENSION,
                                                FIELD_VALUE(0, DE_DIMENSION, X,    xWidth) |
                                                FIELD_VALUE(0, DE_DIMENSION, Y_ET, nHeight));
                    de_ctrl = FIELD_VALUE(0, DE_CONTROL, ROP, nROP2) |
                              FIELD_SET(0, DE_CONTROL, ROP_SELECT, ROP2) |
                              FIELD_SET(0, DE_CONTROL, COMMAND, BITBLT) |
                              ((nDirection == 1) ? FIELD_SET(0, DE_CONTROL,DIRECTION, RIGHT_TO_LEFT): FIELD_SET(0, DE_CONTROL,DIRECTION, LEFT_TO_RIGHT)) |
                              FIELD_SET(0, DE_CONTROL, STATUS, START);
                    regWrite32(DE_CONTROL, de_ctrl);

                    nSrcX += (opSign * xWidth);
                    nDestX += (opSign * xWidth);
                    nWidth -= xWidth;

                    if (nWidth <= 0)
                    {
                        /* ROP2 operation is complete */
                        break;
                    }

                    if (xWidth > nWidth)
                    {
                        xWidth = nWidth;
                    }
                }
            }
            else
            {
              //  deWaitForNotBusy();
                                regWrite32(DE_SOURCE,
                                        FIELD_SET  (0, DE_SOURCE, WRAP, DISABLE) |
                                        FIELD_VALUE(0, DE_SOURCE, X_K1, nSrcX)   |
                                        FIELD_VALUE(0, DE_SOURCE, Y_K2, nSrcY));
                                regWrite32(DE_DESTINATION,
                                        FIELD_SET  (0, DE_DESTINATION, WRAP, DISABLE) |
                                        FIELD_VALUE(0, DE_DESTINATION, X,    nDestX)  |
                                        FIELD_VALUE(0, DE_DESTINATION, Y,    nDestY));
                                regWrite32(DE_DIMENSION,
                                        FIELD_VALUE(0, DE_DIMENSION, X,    nWidth) |
                                        FIELD_VALUE(0, DE_DIMENSION, Y_ET, nHeight));
                de_ctrl = FIELD_VALUE(0, DE_CONTROL, ROP, nROP2) |
                          FIELD_SET(0, DE_CONTROL, ROP_SELECT, ROP2) |
                          FIELD_SET(0, DE_CONTROL, COMMAND, BITBLT) |
                          ((nDirection == 1) ? FIELD_SET(0, DE_CONTROL, DIRECTION,RIGHT_TO_LEFT): FIELD_SET(0, DE_CONTROL, DIRECTION,LEFT_TO_RIGHT)) |
                          FIELD_SET(0, DE_CONTROL, STATUS, START);
                regWrite32(DE_CONTROL, de_ctrl);
            }     

//	    regWrite32(DE_CONTROL,0xffffffff);
//	    regWrite32(0x00,0x0f0f0f0f);
	    

}

void set_current_gate(void)
{
                unsigned long value, gate;

                //change to mode0

                value = regRead32(POWER_MODE_CTRL);
                value = FIELD_SET(value, POWER_MODE_CTRL, MODE, MODE0);
                regWrite32(POWER_MODE_CTRL, value);

                // Don't forget to set up power mode0 gate properly.
                gate = regRead32(CURRENT_POWER_GATE);
                gate = FIELD_SET(gate, CURRENT_POWER_GATE, 2D,  ENABLE);
                regWrite32(POWER_MODE0_GATE, gate);
	
}

/*-----------------------------------------------------------------------------
 * video_hw_init --
 *-----------------------------------------------------------------------------
 */
extern int vga_available ;
extern int novga;
extern int fb_init(unsigned long,unsigned long);

int video_hw_init (void)
{
	unsigned int i;
	memset (&sm502, 0, sizeof (GraphicDevice));
     
	/*
	 * Initialization of the access to the graphic chipset Retreive base
	 * address of the chipset (see board/RPXClassic/eccx.c)
	 */
	if ((sm502.isaBase = board_video_init ()) == 0) {
		return 0 ;
	}
     
	if ((sm502.frameAdrs = board_video_get_fb ()) == 0) {
		return 0;
	}
     
#if defined(CONFIG_VIDEO_SM501_8BPP)
	sm502.gdfIndex = GDF__8BIT_INDEX;
	sm502.gdfBytesPP = 1;

#elif defined(CONFIG_VIDEO_SM501_16BPP)
	sm502.gdfIndex = GDF_16BIT_565RGB;
	sm502.gdfBytesPP = 2;
     
#elif defined(CONFIG_VIDEO_SM501_32BPP)
	sm502.gdfIndex = GDF_32BIT_X888RGB;
	sm502.gdfBytesPP = 4;
#else
	printf("error Unsupported SM502 BPP\n");
#endif
     
	/* Load Smi registers */
	SmiSetRegs (); //640x480

	/* (see board/RPXClassic/RPXClassic.c) */
	board_validate_screen (sm502.isaBase);
	
	set_current_gate();

#if defined(X1024x768)

#if defined(CONFIG_VIDEO_SM501_8BPP)	
	deInit(1024,768,8);
#endif
#if defined(CONFIG_VIDEO_SM501_16BPP)
	deInit(1024,768,16);
#endif
#if defined(CONFIG_VIDEO_SM501_32BPP)
        deInit(1024,768,32);
#endif

#elif defined(X800x600)

#if defined(CONFIG_VIDEO_SM501_8BPP)
        deInit(800,600,8);
#endif
#if defined(CONFIG_VIDEO_SM501_16BPP)
        deInit(800,600,16);
#endif
#if defined(CONFIG_VIDEO_SM501_32BPP)
        deInit(800,600,32);
#endif

#else
#if defined(CONFIG_VIDEO_SM501_8BPP)
        deInit(640,480,8);
#endif
#if defined(CONFIG_VIDEO_SM501_16BPP)
        deInit(640,480,16);
#endif
#if defined(CONFIG_VIDEO_SM501_32BPP)
        deInit(640,480,32);
#endif

#endif
	/* Clear video memory */
//	i = x * y * sm502.gdfBytesPP / 8 ;
//	vm = (unsigned int *)sm502.frameAdrs;
//	while(i--)
//		*vm++ = 0xffffffff;
	return 1;
}

/*-----------------------------------------------------------------------------
 * video_set_lut --
 *-----------------------------------------------------------------------------
 */
void video_set_lut (
	unsigned int index,           /* color number */
	unsigned char r,              /* red */
	unsigned char g,              /* green */
	unsigned char b               /* blue */
	)
{
}

#endif /* CONFIG_VIDEO_SM501 */
