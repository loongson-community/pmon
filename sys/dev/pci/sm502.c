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

#define _VIDEO_FB_H_

#define CONSOLE_BG_COL            0x00
#define CONSOLE_FG_COL            0xa0

#define DISPLAY_WIDTH 640
#define DISPLAY_HEIGHT 480

/*
 * Graphic Data Format (GDF) bits for VIDEO_DATA_FORMAT
 */
#define GDF__8BIT_INDEX         0
#define GDF_15BIT_555RGB        1
#define GDF_16BIT_565RGB        2
#define GDF_32BIT_X888RGB       3
#define GDF_24BIT_888RGB        4
#define GDF__8BIT_332RGB        5

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

//#include <common_uboot.h>

#if 1
const SMI_REGS init_regs[] =
{

#if 0 /*800*600*/
        {0x00004, 0x0},
        {0x00048, 0x00021807},
        {0x0004C, 0x091a0a01},
        {0x00054, 0x1},
        {0x00040, 0x00021807},
        {0x00044, 0x091a0a01},
        {0x00054, 0x0},
        {0x80000, 0x0f013105}, //0x0f013106
        {0x80004, 0xc428bb17},
        {0x8000C, 0x00000000},
        {0x80010, 0x06400640},//0x0a000a00//administrate fb width
        {0x80014, 0x03200000},//0x02800000->640;0x03200000->800
        {0x80018, 0x02600000}, //0x01e00000->30(480);0x02600000->600
        {0x8001C, 0x00000000},
        {0x80020, 0x02600320},//0x01e00280->640x480;0x02600320->800x600
        {0x80024, 0x033a031f},//HDE-0x02fa027f->640x480;
        {0x80028, 0x004a032b},//0x004a028b->640x480;
        {0x8002C, 0x02840257},//VDE-0x020c01df->640x480;
        {0x80030, 0x00020261},//0x000201e9->640x480;
        {0x80040, 0x00010002},//rgb565
        {0x8004c, 0x00800000},//rgb565
        {0x80080, 0x00010001},//rgb565
        {0x80200, 0x00010000},
#endif

#if 0 /* CRT only */
        {0x00004, 0x0},
        {0x00048, 0x00021807},
        {0x0004C, 0x10090a01},
        {0x00054, 0x1},
        {0x00040, 0x00021807},
        {0x00044, 0x10090a01},
        {0x00054, 0x0},
        {0x80200, 0x00010000},
        {0x80204, 0x0},
        {0x80208, 0x0A000A00},
        {0x8020C, 0x02fa027f},
        {0x80210, 0x004a028b},
        {0x80214, 0x020c01df},
        {0x80218, 0x000201e9},
        {0x80200, 0x00013306},
#else  /* panel + CRT */
        {0x00004, 0x0},
        {0x00048, 0x00021807},
        {0x0004C, 0x091a0a01},
        {0x00054, 0x1},
        {0x00040, 0x00021807},
        {0x00044, 0x091a0a01},
        {0x00054, 0x0},
        {0x80000, 0x0f013105}, //0x0f013106
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
        {0x80040, 0x00010002},//rgb565
        {0x8004c, 0x00800000},//rgb565
        {0x80080, 0x00010001},//rgb565
        {0x80200, 0x00010000},
#endif
        {0, 0}
};

#endif

#define CONFIG_VIDEO_SM502

#ifdef CONFIG_VIDEO_SM502

//#include <sm501.h>

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

GraphicDevice sm502;


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
	unsigned int *vm, i;
	
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
	     
#define CONFIG_VIDEO_SM501_16BPP 1
     
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
	SmiSetRegs ();
	/* (see board/RPXClassic/RPXClassic.c) */
	board_validate_screen (sm502.isaBase);
     
	/* Clear video memory */
	vm = (unsigned int *)sm502.frameAdrs;
//	while(i--)
//		*vm++ = 0x00000000;
/*        printf("begin init fb\n");	
	fb_init(sm502.frameAdrs,sm502.isaBase);  
        printf("after fb_init\n");
  
     if(!getenv("novga")&&!novga) 
	vga_available=1;*/
	return 1;
//	return (&sm501);
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
