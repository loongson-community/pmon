/*
 * Modifications to support Loongson Arch:
 * Copyright (c) 2008  Lemote.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice unmodified, this list of conditions, and the following
 *    disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */

/*
 * SiS315E 2D acceleration support. Author: Lj.Peng <penglj@lemote.com>
 */


#include <stdio.h>
#include <stdlib.h>
#include <linux/types.h>

#include <dev/pci/pcivar.h>
#include "linux/io.h"

extern struct pci_device *vga_dev;

/* Some general registers for 315 series */
#define SRC_ADDR        0x8200
#define SRC_PITCH       0x8204
#define AGP_BASE        0x8206 /* color-depth dependent value */
#define SRC_Y           0x8208
#define SRC_X           0x820A
#define DST_Y           0x820C
#define DST_X           0x820E
#define DST_ADDR        0x8210
#define DST_PITCH       0x8214
#define DST_HEIGHT      0x8216
#define RECT_WIDTH      0x8218
#define RECT_HEIGHT     0x821A
#define PAT_FGCOLOR     0x821C
#define PAT_BGCOLOR     0x8220
#define SRC_FGCOLOR     0x8224
#define SRC_BGCOLOR     0x8228
#define MONO_MASK       0x822C
#define LEFT_CLIP       0x8234
#define TOP_CLIP        0x8236
#define RIGHT_CLIP      0x8238
#define BOTTOM_CLIP     0x823A
#define COMMAND_READY       0x823C
#define FIRE_TRIGGER        0x8240

#define PATTERN_REG     0x8300  /* 384 bytes pattern buffer */

/* Transparent bitblit registers */
#define TRANS_DST_KEY_HIGH  PAT_FGCOLOR
#define TRANS_DST_KEY_LOW   PAT_BGCOLOR
#define TRANS_SRC_KEY_HIGH  SRC_FGCOLOR
#define TRANS_SRC_KEY_LOW   SRC_BGCOLOR

/* MMIO access macros */
#define MMIO_IN8(base, offset)  *(volatile u8*)(base + offset)
#define MMIO_IN16(base, offset) *(volatile u16*)(base + offset)
#define MMIO_IN32(base, offset) *(volatile u32*)(base + offset)

#ifdef SIS_DEBUG

#define MMIO_OUT8(base, offset, val) do { \
	*(volatile u8*)(base + offset) = val; \
	prom_printf("mmio base %x offset %x w %02x r %02x\n", base, offset, val, MMIO_IN8(base, offset)); \
} while (0);
#define MMIO_OUT16(base, offset, val) do { \
	*(volatile u16*)(base + offset) = val; \
	prom_printf("mmio base %x offset %x w %04x r %04x\n", base, offset, val, MMIO_IN16(base, offset)); \
} while (0);
#define MMIO_OUT32(base, offset, val) do { \
	*(volatile u32*)(base + offset) = val; \
	prom_printf("mmio base %x offset %x w %08x r %08x\n", base, offset, val, MMIO_IN32(base, offset)); \
} while (0);

#else

#define MMIO_OUT8(base, offset, val) do { \
	*(volatile u8*)(base + offset) = val; \
} while (0);
#define MMIO_OUT16(base, offset, val) do { \
	*(volatile u16*)(base + offset) = val; \
} while (0);
#define MMIO_OUT32(base, offset, val) do { \
	*(volatile u32*)(base + offset) = val; \
} while (0);

#endif /* SIS_DEBUG */

/* Queue control MMIO registers */
#define Q_BASE_ADDR     0x85C0  /* Base address of software queue */
#define Q_WRITE_PTR     0x85C4  /* Current write pointer */
#define Q_READ_PTR      0x85C8  /* Current read pointer */
#define Q_STATUS        0x85CC  /* queue status */

#define SiS310Idle \
	  { \
		  while((MMIO_IN16(ivideo->mmio_vbase, Q_STATUS+2) & 0x8000) != 0x8000){}; \
		  while((MMIO_IN16(ivideo->mmio_vbase, Q_STATUS+2) & 0x8000) != 0x8000){}; \
		  while((MMIO_IN16(ivideo->mmio_vbase, Q_STATUS+2) & 0x8000) != 0x8000){}; \
		  while((MMIO_IN16(ivideo->mmio_vbase, Q_STATUS+2) & 0x8000) != 0x8000){}; \
		  CmdQueLen = 0; \
	  }

#define SiS310SetupSRCBase(base) \
	    if(CmdQueLen <= 0) SiS310Idle;\
    MMIO_OUT32(ivideo->mmio_vbase, SRC_ADDR, base);\
    CmdQueLen--;

#define SiS310SetupDSTBase(base) \
	    if(CmdQueLen <= 0) SiS310Idle;\
    MMIO_OUT32(ivideo->mmio_vbase, DST_ADDR, base);\
    CmdQueLen--;

#define SiS310SetupRect(w,h) \
	    if(CmdQueLen <= 0) SiS310Idle;\
    MMIO_OUT32(ivideo->mmio_vbase, RECT_WIDTH, (h)<<16 | (w) );\
    CmdQueLen--;

#define SiS310SetupSRCXY(x,y) \
	    if(CmdQueLen <= 0) SiS310Idle;\
    MMIO_OUT32(ivideo->mmio_vbase, SRC_Y, (x)<<16 | (y) );\
    CmdQueLen--;

#define SiS310SetupDSTXY(x,y) \
	    if(CmdQueLen <= 0) SiS310Idle;\
    MMIO_OUT32(ivideo->mmio_vbase, DST_Y, (x)<<16 | (y) );\
    CmdQueLen--;

#define SiS310SetupDSTColorDepth(bpp) \
		   if(CmdQueLen <= 0) SiS310Idle;\
		   MMIO_OUT16(ivideo->mmio_vbase, AGP_BASE, bpp);\
		   CmdQueLen--;

#define SiS310SetupSRCPitch(pitch) \
	    if(CmdQueLen <= 0) SiS310Idle;\
    MMIO_OUT16(ivideo->mmio_vbase, SRC_PITCH, pitch);\
    CmdQueLen--;


#define SiS310SetupDSTRect(x,y) \
	    if(CmdQueLen <= 0) SiS310Idle;\
    MMIO_OUT32(ivideo->mmio_vbase, DST_PITCH, (y)<<16 | (x) );\
    CmdQueLen--;

#define SiS310SetupROP(rop) \
	    ivideo->CommandReg = (rop) << 8;

#define SiS310SetupSRCTrans(color) \
	    if(CmdQueLen <= 1) SiS310Idle;\
    MMIO_OUT32(ivideo->mmio_vbase, TRANS_SRC_KEY_HIGH, color);\
    MMIO_OUT32(ivideo->mmio_vbase, TRANS_SRC_KEY_LOW, color);\
    CmdQueLen -= 2;

#define SiS310SetupCMDFlag(flags) \
	    ivideo->CommandReg |= (flags);

#define SiS310DoCMD \
	    if(CmdQueLen <= 1) SiS310Idle;\
    MMIO_OUT32(ivideo->mmio_vbase, COMMAND_READY, ivideo->CommandReg); \
    MMIO_OUT32(ivideo->mmio_vbase, FIRE_TRIGGER, 0); \
    CmdQueLen -= 2;

/* Store queue length in par */
#define CmdQueLen ivideo->cmdqueuelength

/* SiS300 engine commands */
#define BITBLT                  0x00000000  /* Blit */
#define COLOREXP                0x00000001  /* Color expand */
#define ENCOLOREXP              0x00000002  /* Enhanced color expand */
#define MULTIPLE_SCANLINE       0x00000003  /* ? */
#define LINE                    0x00000004  /* Draw line */
#define TRAPAZOID_FILL          0x00000005  /* Fill trapezoid */
#define TRANSPARENT_BITBLT      0x00000006  /* Transparent Blit */



#define IND_SIS_CMDQUEUE_SET		0x26
#define IND_SIS_CMDQUEUE_THRESHOLD	0x27

#define COMMAND_QUEUE_THRESHOLD	0x1F
#define SIS_CMD_QUEUE_RESET		0x01

#define MMIO_QUEUE_PHYBASE      Q_BASE_ADDR
#define MMIO_QUEUE_WRITEPORT    Q_WRITE_PTR
#define MMIO_QUEUE_READPORT     Q_READ_PTR

#define SIS_AGP_CMDQUEUE_ENABLE		0x80  /* 315/330/340 series SR26 */
#define SIS_VRAM_CMDQUEUE_ENABLE	0x40
#define SIS_MMIO_CMD_ENABLE		0x20
#define SIS_CMD_QUEUE_SIZE_512k		0x00
#define SIS_CMD_QUEUE_SIZE_1M		0x04
#define SIS_CMD_QUEUE_SIZE_2M		0x08
#define SIS_CMD_QUEUE_SIZE_4M		0x0C
#define SIS_CMD_QUEUE_RESET		0x01
#define SIS_CMD_AUTO_CORR		0x02



/* SIS io port w/r */
#define inSISREG(reg, idx, val) do { \
	*(volatile unsigned char *)((reg) + sis_video_info.io_vbase) = idx; \
	val = *(volatile unsigned char *)((reg) + 1 + sis_video_info.io_vbase); \
} while (0);


#define outSISREG(reg, idx, val) do { \
	*(volatile unsigned char *)((reg) + sis_video_info.io_vbase) = idx; \
	*(volatile unsigned char *)((reg) + 1 + sis_video_info.io_vbase) = (u8)(val); \
} while (0);


static const u8 sisALUConv[] =
{
	0x00,       /* dest = 0;            0,      GXclear,        0 */
	0x88,       /* dest &= src;         DSa,    GXand,          0x1 */
	0x44,       /* dest = src & ~dest;  SDna,   GXandReverse,   0x2 */
	0xCC,       /* dest = src;          S,      GXcopy,         0x3 */
	0x22,       /* dest &= ~src;        DSna,   GXandInverted,  0x4 */
	0xAA,       /* dest = dest;         D,      GXnoop,         0x5 */
	0x66,       /* dest = ^src;         DSx,    GXxor,          0x6 */
	0xEE,       /* dest |= src;         DSo,    GXor,           0x7 */
	0x11,       /* dest = ~src & ~dest; DSon,   GXnor,          0x8 */
	0x99,       /* dest ^= ~src ;       DSxn,   GXequiv,        0x9 */
	0x55,       /* dest = ~dest;        Dn,     GXInvert,       0xA */
	0xDD,       /* dest = src|~dest ;   SDno,   GXorReverse,    0xB */
	0x33,       /* dest = ~src;         Sn,     GXcopyInverted, 0xC */
	0xBB,       /* dest |= ~src;        DSno,   GXorInverted,   0xD */
	0x77,       /* dest = ~src|~dest;   DSan,   GXnand,         0xE */
	0xFF,       /* dest = 0xFF;         1,      GXset,          0xF */
};

struct sis_video_info {
	int cmdqueuelength;
	int cmdQueueSize;
	u32 mmio_vbase;
	u32 io_vbase;
	u32 DstColor;
	u32 video_linelength;
	u32 CommandReg;
	u32 SiS310_AccelDepth;
	u32 video_offset;
};

static struct sis_video_info sis_video_info = {0};
    
static void SiS310SetupForScreenToScreenCopy(struct sis_video_info *ivideo, int rop, int trans_color)
{
	SiS310SetupDSTColorDepth(ivideo->DstColor);
	SiS310SetupSRCPitch(ivideo->video_linelength)
	SiS310SetupDSTRect(ivideo->video_linelength, 0x0fff)
	if(trans_color != -1) {
		SiS310SetupROP(0x0A)
		SiS310SetupSRCTrans(trans_color)
		SiS310SetupCMDFlag(TRANSPARENT_BITBLT)
	} else {
		SiS310SetupROP(sisALUConv[rop])
		/* Set command - not needed, both 0 */
		/* SiSSetupCMDFlag(BITBLT | SRCVIDEO) */
	}
	SiS310SetupCMDFlag(ivideo->SiS310_AccelDepth)
	/* The chip is smart enough to know the direction */
}   

static void SiS310SubsequentScreenToScreenCopy(struct sis_video_info *ivideo, int src_x, int src_y,
		             int dst_x, int dst_y, int width, int height)
{
	u32 srcbase = 0, dstbase = 0;

	srcbase += ivideo->video_offset;
	dstbase += ivideo->video_offset;

	SiS310SetupSRCBase(srcbase);
	SiS310SetupDSTBase(dstbase);
	SiS310SetupRect(width, height)
	SiS310SetupSRCXY(src_x, src_y)
	SiS310SetupDSTXY(dst_x, dst_y)
	SiS310DoCMD
}

void video_hw_bitblt(int bpp, int sx, int sy, int dx, int dy, int w, int h)
{
	struct sis_video_info *ivideo = &sis_video_info;
	SiS310SetupForScreenToScreenCopy(ivideo, 3, -1);
	SiS310SubsequentScreenToScreenCopy(ivideo, sx, sy, dx, dy, w, h);
	SiS310Idle;
}

int  sis315e_init()
{

	u32 io_address, mmio_address;
	u32 tmp;

	io_address  =_pci_conf_read(vga_dev->pa.pa_tag,0x18);
	mmio_address  =_pci_conf_read(vga_dev->pa.pa_tag,0x14);
	sis_video_info.io_vbase = (io_address & ~1) | 0xbfd00000;
	sis_video_info.mmio_vbase = (mmio_address & ~0xf)| 0xb4000000;

#define COMMAND_QUEUE_AREA_SIZE	(512 * 1024)	/* 512K */
	sis_video_info.cmdQueueSize = COMMAND_QUEUE_AREA_SIZE;

	/* Unlock SR */
	inSISREG(0x44, 0x5, tmp);
	if (tmp != 0xa1)
		outSISREG(0x44, 0x5,0x86);

	/* Enable PCI_LINEAR_ADDRESSING and MMIO_ENABLE */
	inSISREG(0x44, 0x20, tmp);
	outSISREG(0x44, 0x20, (u8)tmp | 0xa1);
	inSISREG(0x44, 0x1e, tmp);
	outSISREG(0x44, 0x1e, (u8)tmp | 0xda);

	/*Init 2D engine */
	outSISREG(0x44, IND_SIS_CMDQUEUE_THRESHOLD, COMMAND_QUEUE_THRESHOLD);
	outSISREG(0x44, IND_SIS_CMDQUEUE_SET, SIS_CMD_QUEUE_RESET);
	tmp = MMIO_IN32(sis_video_info.mmio_vbase, MMIO_QUEUE_READPORT);
	MMIO_OUT32(sis_video_info.mmio_vbase, MMIO_QUEUE_WRITEPORT, tmp);
	tmp = (SIS_MMIO_CMD_ENABLE | SIS_CMD_AUTO_CORR);
	outSISREG(0x44, IND_SIS_CMDQUEUE_SET, tmp);
	tmp = (u32)(0x1000000 - sis_video_info.cmdQueueSize);/* Assume framebuffer size 16MB*/
	MMIO_OUT32(sis_video_info.mmio_vbase, MMIO_QUEUE_PHYBASE, tmp);


	sis_video_info.video_linelength = GetXRes() * GetBytesPP();
	switch (GetBytesPP()) {
		case 1:
			sis_video_info.DstColor = 0x0000;
			sis_video_info.SiS310_AccelDepth = 0x00000000;
			break;
		case 2:
			sis_video_info.DstColor = 0x8000;
			sis_video_info.SiS310_AccelDepth = 0x00010000;
		case 4:
			sis_video_info.DstColor = 0xC000;
			sis_video_info.SiS310_AccelDepth = 0x00020000;
			break;
		default:
			sis_video_info.DstColor = 0xC000;
			sis_video_info.SiS310_AccelDepth = 0x00020000;
			break;
	}


	printf("SiS 315E Init complete. MMIO Virtual base %x\n", sis_video_info.mmio_vbase);

	return 0;
}
