//Created by xiexin for Display Controller pmon test 
//Oct 6th,2009
#include <pmon.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/malloc.h>
#include <machine/pio.h>
typedef unsigned long u32;
typedef unsigned short u16;
typedef unsigned char u8;
typedef signed long s32;
typedef signed short s16;
typedef signed char s8;
typedef int bool;
typedef unsigned long dma_addr_t;

#define DC_FB1 1

#define writeb(val, addr) (*(volatile u8*)(addr) = (val))
#define writew(val, addr) (*(volatile u16*)(addr) = (val))
#define writel(val, addr) (*(volatile u32*)(addr) = (val))
#define readb(addr) (*(volatile u8*)(addr))
#define readw(addr) (*(volatile u16*)(addr))
#define readl(addr) (*(volatile u32*)(addr))

#define write_reg(addr,val) writel(val,addr)

#define DIS_WIDTH  FB_XSIZE
#define DIS_HEIGHT FB_YSIZE
#define EXTRA_PIXEL  0

#define DC_BASE_ADDR	0xbbe51240
#define DC_BASE_ADDR_1	0xbbe51250

#define RANDOM_HEIGHT_Z 37

static char *ADDR_CURSOR = 0xc6000000;
static char *MEM_ptr = 0xc2000000;	/* frame buffer address register on ls2h mem */

static int MEM_ADDR = 0;

struct vga_struc {
	float pclk;
	int hr, hss, hse, hfl;
	int vr, vss, vse, vfl;
} vgamode[] = {
	{	28.56,	640,	664,	728,	816,	480,	481,	484,	500,	},	/*"640x480_70.00" */	
	{	33.10,	640,	672,	736,	832,	640,	641,	644,	663,	},	/*"640x640_60.00" */	
	{	39.69,	640,	672,	736,	832,	768,	769,	772,	795,	},	/*"640x768_60.00" */	
	{	42.13,	640,	680,	744,	848,	800,	801,	804,	828,	},	/*"640x800_60.00" */	
	{	35.84,	800,	832,	912,	1024,	480,	481,	484,	500,	},	/*"800x480_70.00" */	
	{	38.22,	800,	832,	912,	1024,	600,	601,	604,	622,	},	/*"800x600_60.00" */	
	{	40.73,	800,	832,	912,	1024,	640,	641,	644,	663,	},	/*"800x640_60.00" */	
	{	40.01,	832,	864,	952,	1072,	600,	601,	604,	622,	},	/*"832x600_60.00" */	
	{	40.52,	832,	864,	952,	1072,	608,	609,	612,	630,	},	/*"832x608_60.00" */	
	{	38.17,	1024,	1048,	1152,	1280,	480,	481,	484,	497,	},	/*"1024x480_60.00" */	
	{	48.96,	1024,	1064,	1168,	1312,	600,	601,	604,	622,	},	/*"1024x600_60.00" */	
	{	52.83,	1024,	1072,	1176,	1328,	640,	641,	644,	663,	},	/*"1024x640_60.00" */	
	{	64.11,	1024,	1080,	1184,	1344,	768,	769,	772,	795,	},	/*"1024x768_60.00" */	
	{	64.11,	1024,	1080,	1184,	1344,	768,	769,	772,	795,	},	/*"1024x768_60.00" */	
	{	64.11,	1024,	1080,	1184,	1344,	768,	769,	772,	795,	},	/*"1024x768_60.00" */	
	{	71.38,	1152,	1208,	1328,	1504,	764,	765,	768,	791,	},	/*"1152x764_60.00" */	
	{	83.46,	1280,	1344,	1480,	1680,	800,	801,	804,	828,	},	/*"1280x800_60.00" */	
	{	98.60,	1280,	1352,	1488,	1696,	1024,	1025,	1028,	1057,	},	/*"1280x1024_55.00" */	
	{	93.80,	1440,	1512,	1664,	1888,	800,	801,	804,	828,	},	/*"1440x800_60.00" */	
	{	120.28,	1440,	1528,	1680,	1920,	900,	901,	904,	935,	},	/*"1440x900_67.00" */
};

enum {
	OF_BUF_CONFIG = 0,
	OF_BUF_ADDR = 0x20,
	OF_BUF_STRIDE = 0x40,
	OF_BUF_ORIG = 0x60,
	OF_DITHER_CONFIG = 0x120,
	OF_DITHER_TABLE_LOW = 0x140,
	OF_DITHER_TABLE_HIGH = 0x160,
	OF_PAN_CONFIG = 0x180,
	OF_PAN_TIMING = 0x1a0,
	OF_HDISPLAY = 0x1c0,
	OF_HSYNC = 0x1e0,
	OF_VDISPLAY = 0x240,
	OF_VSYNC = 0x260,
	OF_DBLBUF = 0x340,
};

int caclulatefreq(float PCLK)
{
	int pstdiv, ODF, LDF, idf, inta, intb;
	int mpd, modf, mldf, rodf;
	int out;
	float a, b, c, min;

	min = 100;
	for (pstdiv = 1; pstdiv < 32; pstdiv++)
		for (ODF = 1; ODF <= 8; ODF *= 2) {
			a = PCLK * pstdiv;
			b = a * ODF;
			LDF = ((int)(b / (50 * ODF))) * ODF;
			intb = 50 * LDF;
			inta = intb / ODF;
			c = b - intb;
			if (inta < 75 || inta > 1800)
				continue;
			if (intb < 600 || intb > 1800)
				continue;
			if (LDF < 8 || LDF > 225)
				continue;
			if (c < min) {
				min = c;
				mpd = pstdiv;
				modf = ODF;
				mldf = LDF;
			}
		}
	rodf = modf == 8 ? 3 : modf == 4 ? 2 : modf == 2 ? 1 : 0;
	out = (mpd << 24) + (mldf << 16) + (rodf << 5) + (5 << 2) + 1;
	printf("ODF=%d, LDF=%d, IDF=5, pstdiv=%d, prediv=1\n", rodf, mldf, mpd);
	return out;
}

int config_cursor()
{
	printf("framebuffer Cursor Configuration\n");
	write_reg((0xbbe51520 + 0x00), 0x00020200);
	printf("framebuffer Cursor Address\n");
	write_reg((0xbbe51530 + 0x00), ADDR_CURSOR);
	printf("framebuffer Cursor Location\n");
	write_reg((0xbbe51540 + 0x00), 0x00060122);
	printf("framebuffer Cursor Background\n");
	write_reg((0xbbe51550 + 0x00), 0x00eeeeee);
	printf("what hell is this register for ?\n");
	write_reg((0xbbe51560 + 0x00), 0x00aaaaaa);
}

int config_fb(unsigned long base)
{
	int i, mode = -1;
	int j;
	unsigned int chip_reg;

	//this line code make the HD DVI display normally.
	*(volatile unsigned int *)(0xbbd0020c) |= (0x7 << 12);

	for (i = 0; i < sizeof(vgamode) / sizeof(struct vga_struc); i++) {
		int out;
		if (vgamode[i].hr == FB_XSIZE && vgamode[i].vr == FB_YSIZE) {
			mode = i;
			out = caclulatefreq(vgamode[i].pclk);
			printf("out=%x\n", out);
			/* change to refclk */
#ifdef DC_FB0
			*(volatile unsigned int *)(0xbbd00234) = 0x0;
#else
			*(volatile unsigned int *)(0xbbd0023c) = 0x0;
#endif
			/* pll_powerdown set pstdiv */
#ifdef DC_FB0
			*(volatile unsigned int *)(0xbbd00230) =
			    out | 0x80000080;
#else
			*(volatile unsigned int *)(0xbbd00238) =
			    out | 0x80000080;
#endif
			/* wait 10000ns */
			for (j = 1; j <= 300; j++)
				chip_reg =
				    *(volatile unsigned int *)(0xbbd00210);
			/* pll_powerup unset pstdiv */
#ifdef DC_FB0
			*(volatile unsigned int *)(0xbbd00230) = out;
#else
			*(volatile unsigned int *)(0xbbd00238) = out;
#endif
			/* wait pll_lock */
			while ((*(volatile unsigned int *)(0xbbd00210)) &
			       0x00001800 != 0x00001800) {
			}
			/* change to pllclk */
#ifdef DC_FB0
			*(volatile unsigned int *)(0xbbd00234) = 0x1;
#else
			*(volatile unsigned int *)(0xbbd0023c) = 0x1;
#endif
			break;
		}
	}

	if (mode < 0) {
		printf("\n\n\nunsupported framebuffer resolution\n\n\n");
		return;
	}

	/* Disable the panel 0 */
	write_reg((base + OF_BUF_CONFIG), 0x00000000);
	/* framebuffer configuration RGB565 */
	write_reg((base + OF_BUF_CONFIG), 0x00000003);
	write_reg((base + OF_BUF_ADDR), MEM_ADDR);
	write_reg(base + OF_DBLBUF, MEM_ADDR);
	write_reg((base + OF_DITHER_CONFIG), 0x00000000);
	write_reg((base + OF_DITHER_TABLE_LOW), 0x00000000);
	write_reg((base + OF_DITHER_TABLE_HIGH), 0x00000000);
	write_reg((base + OF_PAN_CONFIG), 0x80001311);
	write_reg((base + OF_PAN_TIMING), 0x00000000);

	write_reg((base + OF_HDISPLAY),
		  (vgamode[mode].hfl << 16) | vgamode[mode].hr);
	write_reg((base + OF_HSYNC),
		  0x40000000 | (vgamode[mode].hse << 16) | vgamode[mode].hss);
	write_reg((base + OF_VDISPLAY),
		  (vgamode[mode].vfl << 16) | vgamode[mode].vr);
	write_reg((base + OF_VSYNC),
		  0x40000000 | (vgamode[mode].vse << 16) | vgamode[mode].vss);

#if defined(CONFIG_VIDEO_32BPP)
	write_reg((base + OF_BUF_CONFIG), 0x00100104);
	write_reg((base + OF_BUF_STRIDE), (FB_XSIZE * 4 + 255) & ~255);
#elif defined(CONFIG_VIDEO_16BPP)
	write_reg((base + OF_BUF_CONFIG), 0x00100103);
	write_reg((base + OF_BUF_STRIDE), (FB_XSIZE * 2 + 255) & ~255);
#elif defined(CONFIG_VIDEO_15BPP)
	write_reg((base + OF_BUF_CONFIG), 0x00100102);
	write_reg((base + OF_BUF_STRIDE), (FB_XSIZE * 2 + 255) & ~255);
#elif defined(CONFIG_VIDEO_12BPP)
	write_reg((base + OF_BUF_CONFIG), 0x00100101);
	write_reg((base + OF_BUF_STRIDE), (FB_XSIZE * 2 + 255) & ~255);
#else /* 640x480-32Bits */
	write_reg((base + OF_BUF_CONFIG), 0x00100104);
	write_reg((base + OF_BUF_STRIDE), (FB_XSIZE * 4 + 255) & ~255);
#endif /* 32Bits */

}

int dc_init()
{
	int print_count;
	int i;
	int PIXEL_COUNT = DIS_WIDTH * DIS_HEIGHT + EXTRA_PIXEL;
	int MEM_SIZE;
	int init_R = 0;
	int init_G = 0;
	int init_B = 0;
	int j;
	int ii = 0, tmp = 0;
	int MEM_SIZE_3 = MEM_SIZE / 6;

	int line_length = 0;

	int print_addr;
	int print_data;
	printf("enter dc_init...\n");

#if defined(CONFIG_VIDEO_32BPP)
	MEM_SIZE = PIXEL_COUNT * 4;
	line_length = FB_XSIZE * 4;
#elif defined(CONFIG_VIDEO_16BPP)
	MEM_SIZE = PIXEL_COUNT * 2;
	line_length = FB_XSIZE * 2;
#elif defined(CONFIG_VIDEO_15BPP)
	MEM_SIZE = PIXEL_COUNT * 2;
	line_length = FB_XSIZE * 2;
#elif defined(CONFIG_VIDEO_12BPP)
	MEM_SIZE = PIXEL_COUNT * 2;
	line_length = FB_XSIZE * 2;
#else
	MEM_SIZE = PIXEL_COUNT * 4;
	line_length = FB_XSIZE * 4;
#endif

	MEM_ADDR = (long)MEM_ptr & 0x0fffffff;
	if (MEM_ptr == NULL) {
		printf("frame buffer memory malloc failed!\n ");
		exit(0);
	}

	for (ii = 0; ii < 0x1000; ii += 4)
		*(volatile unsigned int *)(ADDR_CURSOR + ii) = 0x88f31f4f;

	ADDR_CURSOR = (long)ADDR_CURSOR & 0x0fffffff;
	printf("frame buffer addr: %x \n", MEM_ADDR);
	/* Improve the DC DMA's priority */
	outb(0xbbd80636, 0x36);
	/* Make DVO from panel1, it's the same with VGA*/
	outl(0xbbe51240, 0);
	outl(0xbbe51240, 0x100200);

#ifdef DC_FB0
	config_fb(DC_BASE_ADDR);
#else
	config_fb(DC_BASE_ADDR_1);
#endif
	config_cursor();

	printf("display controller reg config complete!\n");

	return MEM_ptr;
}

static int cmd_dc_freq(int argc, char **argv)
{
	int out;
	long sysclk;
	float pclk;
	int j;
	unsigned int chip_reg;
	if (argc < 2)
		return -1;
	pclk = strtoul(argv[1], 0, 0);
	if (argc > 2)
		sysclk = strtoul(argv[2], 0, 0);
	else
		sysclk = 33333;
	out = caclulatefreq(pclk);
	printf("out=%x\n", out);
	/* change to refclk */
#ifdef DC_FB0
	*(volatile unsigned int *)(0xbbd00234) = 0x1;
#else
	*(volatile unsigned int *)(0xbbd0023c) = 0x1;
#endif
	/* pll_powerdown set pstdiv */
#ifdef DC_FB0
	*(volatile unsigned int *)(0xbbd00230) = out | 0x80000080;
#else
	*(volatile unsigned int *)(0xbbd00238) = out | 0x80000080;
#endif
	/* wait 10000ns */
	for (j = 1; j <= 200; j++)
		chip_reg = *(volatile unsigned int *)(0xbbd00210);
	/* pll_powerup unset pstdiv */
#ifdef DC_FB0
	*(volatile unsigned int *)(0xbbd00230) = out;
#else
	*(volatile unsigned int *)(0xbbd00238) = out;
#endif
	/* wait pll_lock */
	while ((*(volatile unsigned int *)(0xbbd00210)) & 0x00001800 !=
	       0x00001800) {
	}
	/* change to pllclk */
#ifdef DC_FB0
	*(volatile unsigned int *)(0xbbd00234) = 0x0;
#else
	*(volatile unsigned int *)(0xbbd0023c) = 0x0;
#endif

	return 0;
}

static const Cmd Cmds[] = {
	{"MyCmds"},
	{"dc_freq", " pclk sysclk", 0, "config dc clk(khz)", cmd_dc_freq, 0, 99,
	 CMD_REPEAT},
	{0, 0}
};

static void init_cmd __P((void)) __attribute__ ((constructor));

static void init_cmd()
{
	cmdlist_expand(Cmds, 1);
}
