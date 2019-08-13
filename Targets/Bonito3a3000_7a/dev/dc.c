//Created by xiexin for Display Controller pmon test 
//Oct 6th,2009
#include <pmon.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/malloc.h>
#include <machine/pio.h>
typedef unsigned int u32;
typedef unsigned short u16;
typedef unsigned char u8;
typedef signed long s32;
typedef signed short s16;
typedef signed char s8;
typedef int bool;
typedef unsigned long dma_addr_t;

#define writeb(val, addr) (*(volatile u8*)(addr) = (val))
#define writew(val, addr) (*(volatile u16*)(addr) = (val))
#define writel(val, addr) (*(volatile u32*)(addr) = (val))
#define readb(addr) (*(volatile u8*)(addr))
#define readw(addr) (*(volatile u16*)(addr))
#define readl(addr) (*(volatile u32*)(addr))

#define write_reg(addr,val) writel(val,addr)

#define FB_XSIZE 800
#define FB_YSIZE 600
//#define FB_XSIZE 1920
//#define FB_YSIZE 1080
#define DIS_WIDTH  FB_XSIZE
#define DIS_HEIGHT FB_YSIZE
#define EXTRA_PIXEL  0
#define LO_OFF	0
#define HI_OFF	8
static struct pix_pll {
	unsigned int l2_div;
	unsigned int l1_loopc;
	unsigned int l1_refc;
}pll_cfg;

#undef USE_GMEM
#ifdef USE_GMEM
static char *ADDR_CURSOR = 0xc6000000;
static char *MEM_ptr = 0xc2000000;
#else
static char *ADDR_CURSOR = 0x86000000;
static char *MEM_ptr = 0x87000000;
#endif
unsigned int dc_base_addr;
static int MEM_ADDR = 0;

struct vga_struc {
	float pclk;
	int hr, hss, hse, hfl;
	int vr, vss, vse, vfl;
} vgamode[] = {
//	{	28.56,	640,	664,	728,	816,	480,	481,	484,	500,	},	/*"640x480_70.00" */	
	{	23.86,	640,	656,	720,	800,	480,	481,	484,	497,	},	/*"640x480_60.00" */	
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
	{	71.38,	1152,	1208,	1328,	1504,	764,	765,	768,	791,	},	/*"1152x764_60.00" */	
	{	83.46,	1280,	1344,	1480,	1680,	800,	801,	804,	828,	},	/*"1280x800_60.00" */	
	{	98.60,	1280,	1352,	1488,	1696,	1024,	1025,	1028,	1057,	},	/*"1280x1024_55.00" */	
	{	108.88,	1280,	1360,	1496,	1712,	1024,	1025,	1028,	1060,	},	/*"1280x1024_60.00" */	
	{	93.80,	1440,	1512,	1664,	1888,	800,	801,	804,	828,	},	/*"1440x800_60.00" */	
	{	120.28,	1440,	1528,	1680,	1920,	900,	901,	904,	935,	},	/*"1440x900_67.00" */
	{	172.80,	1920,	2040,	2248,	2576,	1080,	1081,	1084,	1118,	},	/*"1920x1080_60.00" */
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

#define PLL_REF_CLK_MHZ    100
#define PCLK_PRECISION_INDICATOR 10000

//for PLL using configuration: (refc, loopc, div). Like LS7A1000 PLL
//for C env without decimal fraction support.
static int calc_pll(unsigned int pixclock_khz)
{
    unsigned int refc_set[] = {4, 5, 3};
    unsigned int prec_set[] = {1, 5, 10, 50, 100};   //in 1/PCLK_PRECISION_INDICATOR
    unsigned int pstdiv, loopc, refc;
    int i, j;
    unsigned int precision_req, precision;
    unsigned int loopc_min, loopc_max, loopc_mid;
    unsigned long long real_dvo, req_dvo;
    int loopc_offset;

    //try precsion from high to low
    for (j = 0; j < sizeof(prec_set)/sizeof(int); j++){
        precision_req = prec_set[j];
        //try each refc
        for (i = 0; i < sizeof(refc_set)/sizeof(int); i++) {
            refc = refc_set[i];
            loopc_min = (1200 / PLL_REF_CLK_MHZ) * refc;  //1200 / (PLL_REF_CLK_MHZ / refc)
            loopc_max = (3200 / PLL_REF_CLK_MHZ) * refc;  //3200 / (PLL_REF_CLK_MHZ / refc)
            loopc_mid = (2200 / PLL_REF_CLK_MHZ) * refc;  //(loopc_min + loopc_max) / 2;

            loopc_offset = -1;
            //try each loopc
            for (loopc = loopc_mid; (loopc <= loopc_max) && (loopc >= loopc_min); loopc += loopc_offset) {
                if(loopc_offset < 0){
                    loopc_offset = -loopc_offset;
                }else{
                    loopc_offset = -(loopc_offset+1);
                }

                pstdiv = loopc * PLL_REF_CLK_MHZ * 1000 / refc / pixclock_khz;
                if((pstdiv > 127) || (pstdiv < 1)) continue;
                //real_freq / req_freq = (real_freq * pstdiv) / (req_freq * pstdiv) = (loopc * PLL_REF_CLK_MHZ * 1000 / refc) / (req_freq * pstdiv).
                //real_freq is float type which is not available, but read_freq * pstdiv is available.
                real_dvo = (loopc * PLL_REF_CLK_MHZ * 1000 / refc);
                req_dvo  = (pixclock_khz * pstdiv);
                precision = abs(real_dvo * PCLK_PRECISION_INDICATOR / req_dvo - PCLK_PRECISION_INDICATOR);

                if(precision < precision_req){
                    pll_cfg.l2_div = pstdiv;
                    pll_cfg.l1_loopc = loopc;
                    pll_cfg.l1_refc = refc;
                    printf("for pixclock = %d khz, found: pstdiv = %d, "
                            "loopc = %d, refc = %d, precision = %d / %d.\n", pixclock_khz, 
                            pll_cfg.l2_div, pll_cfg.l1_loopc, pll_cfg.l1_refc, precision+1, PCLK_PRECISION_INDICATOR);
                    if(j > 1){
                        printf("Warning: PIX clock precision degraded to %d / %d\n", precision_req, PCLK_PRECISION_INDICATOR);
                    }

                    return 1;
                }
            }
        }
    }
    return 0;
}

int config_cursor(unsigned long base)
{
	write_reg(base + 0x1520, 0x00020200);
	write_reg(base + 0x1530, ADDR_CURSOR);
	write_reg(base + 0x1540, 0x00060122);
	write_reg(base + 0x1550, 0x00eeeeee);
	write_reg(base + 0x1560, 0x00aaaaaa);
}

//For LS7A1000, configure PLL
static void config_pll(unsigned int pll_base, struct pix_pll pll_cfg)
{
	unsigned int val;

	//printf("div = %d, loopc = %d, refc = %d.\n", 
	//		pll_cfg.l2_div, pll_cfg.l1_loopc, pll_cfg.l1_refc);

	/* set sel_pll_out0 0 */
	val = readl(pll_base + 0x4);
	val &= ~(1UL << 8);
	writel(val, pll_base + 0x4);
	/* pll_pd 1 */
	val = readl(pll_base + 0x4);
	val |= (1UL << 13);
	writel(val, pll_base + 0x4);
	/* set_pll_param 0 */
	val = readl(pll_base + 0x4);
	val &= ~(1UL << 11);
	writel(val, pll_base + 0x4);

	/* set new div ref, loopc, div out */
	/* clear old value first*/
	val = readl(pll_base + 0x4);
	val &= ~(0x7fUL << 0);
	val |= (pll_cfg.l1_refc << 0);
	writel(val, pll_base + 0x4);

	val = readl(pll_base + 0x0);
	val &= ~(0x7fUL << 0);
	val |= (pll_cfg.l2_div << 0);
	val &= ~(0x1ffUL << 21);
	val |= (pll_cfg.l1_loopc << 21);
	writel(val, pll_base + 0x0);

	/* set_pll_param 1 */
	val = readl(pll_base + 0x4);
	val |= (1UL << 11);
	writel(val, pll_base + 0x4);
	/* pll_pd 0 */
	val = readl(pll_base + 0x4);
	val &= ~(1UL << 13);
	writel(val, pll_base + 0x4);
    /* wait pll lock */
    while(!(readl(pll_base + 0x4) & 0x80));
	/* set sel_pll_out0 1 */
	val = readl(pll_base + 0x4);
	val |= (1UL << 8);
	writel(val, pll_base + 0x4);
}

int config_fb(unsigned long base)
{
	int i, mode = -1;
	int j;
	unsigned int confbus;

	confbus = *(volatile unsigned int *)0xba00a810;
	confbus &= 0xfffffff0;
	confbus |= 0xa0000000;

    //printf("confbus = %x\n", confbus);
    for (i = 0; i < sizeof(vgamode) / sizeof(struct vga_struc); i++) {
        if (vgamode[i].hr == FB_XSIZE && vgamode[i].vr == FB_YSIZE) {
            mode = i;
            if(calc_pll((unsigned int)(vgamode[i].pclk * 1000))){
                config_pll(confbus + 0x4b0, pll_cfg);
                config_pll(confbus + 0x4c0, pll_cfg);
            }else{
                printf("\n\nError: Fail to find a proper PLL configuration.\n\n");
            }
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

	unsigned int dvo0_base_addr, dvo1_base_addr;

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
#ifdef USE_GMEM
	MEM_ADDR |= 0x40000000;
#endif
	if (MEM_ptr == NULL) {
		printf("frame buffer memory malloc failed!\n ");
		exit(0);
	}

	for (ii = 0; ii < 0x1000; ii += 4)
		*(volatile unsigned int *)(ADDR_CURSOR + ii) = 0x88f31f4f;

	ADDR_CURSOR = (long)ADDR_CURSOR & 0x0fffffff;
	printf("frame buffer addr: %x \n", MEM_ADDR);

    /* get dc base addr from pci config space */
	dc_base_addr = *(volatile unsigned int *)0xba003110;
	dc_base_addr &= 0xfffffff0;
	dc_base_addr |= 0x80000000;
	dvo0_base_addr = dc_base_addr + 0x1240;
	dvo1_base_addr = dc_base_addr + 0x1250;

	/* dvo1 */
	config_fb(dvo1_base_addr);
	config_cursor(dvo1_base_addr);
	/* dvo0 */
	config_fb(dvo0_base_addr);
	config_cursor(dvo0_base_addr);

	printf("display controller reg config complete!\n");

	return MEM_ptr;
}
void dc_close(void)
{
    //disable dc out put
    readl(dc_base_addr + 0x1240 + OF_BUF_CONFIG) &= ~(1 << 8);
    readl(dc_base_addr + 0x1250 + OF_BUF_CONFIG) &= ~(1 << 8);
}
