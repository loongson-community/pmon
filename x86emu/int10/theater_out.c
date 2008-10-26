/*********************************************************************
 * tv output of radeon 7000.
 * some code borrowed from gatos.sourceforge.net
 * Copyright 2007, Fuxin Zhang, Lemote Corp. 
 * History:
 *
 * First version. 2007.1.13
 *********************************************************************/

#include "theater_out.h"

//#define RADEON_DEBUG 1
#if RADEON_DEBUG
#define RTTRACE(x)    { prom_printf x; }
#else
#define RTTRACE(x)
#endif

/**********************************************************************
 *
 * MASK_N_BIT
 *
 **********************************************************************/

#define MASK_N_BIT(n)	(1UL << (n))

/**********************************************************************
 *
 * Constants
 *
 **********************************************************************/

/*
 * Reference frequency
 * FIXME: This should be extracted from BIOS data
 */
#define REF_FREQUENCY		27000

#define TV_PLL_FINE_INI			0X10000000

/*
 * VIP_TV_PLL_CNTL
 */
#define VIP_TV_PLL_CNTL_M_SHIFT		0
#define VIP_TV_PLL_CNTL_NLO		0x1ff
#define VIP_TV_PLL_CNTL_NLO_SHIFT	8
#define VIP_TV_PLL_CNTL_NHI		0x600
#define VIP_TV_PLL_CNTL_NHI_SHIFT	(21-9)
#define VIP_TV_PLL_CNTL_P_SHIFT		24

/*
 * VIP_CRT_PLL_CNTL
 */
#define VIP_CRT_PLL_CNTL_M		0xff
#define VIP_CRT_PLL_CNTL_M_SHIFT	0
#define VIP_CRT_PLL_CNTL_NLO		0x1ff
#define VIP_CRT_PLL_CNTL_NLO_SHIFT	8
#define VIP_CRT_PLL_CNTL_NHI		0x600
#define VIP_CRT_PLL_CNTL_NHI_SHIFT	(21-9)
#define VIP_CRT_PLL_CNTL_CLKBY2		MASK_N_BIT(25)

/*
 * Value for VIP_PLL_CNTL0
 */
#define VIP_PLL_CNTL0_INI		0x00acac18
#define VIP_PLL_CNTL0_TVSLEEPB		MASK_N_BIT(3)
#define VIP_PLL_CNTL0_CRTSLEEPB		MASK_N_BIT(4)

/*
 * Value for VIP_PLL_TEST_CNTL
 */
#define VIP_PLL_TEST_CNTL_INI		0

/*
 * VIP_CLOCK_SEL_CNTL
 */
#define VIP_CLOCK_SEL_CNTL_INI		0x33
#define VIP_CLOCK_SEL_CNTL_BYTCLK_SHIFT	2
#define VIP_CLOCK_SEL_CNTL_BYTCLK	0xc
#define VIP_CLOCK_SEL_CNTL_REGCLK	MASK_N_BIT(5)
#define VIP_CLOCK_SEL_CNTL_BYTCLKD_SHIFT 8

/*
 * Value for VIP_CLKOUT_CNTL
 */
#define VIP_CLKOUT_CNTL_INI		0x29

/*
 * Value for VIP_SYNC_LOCK_CNTL
 */
#define VIP_SYNC_LOCK_CNTL_INI		0x01000000

/*
 * Value for VIP_TVO_SYNC_PAT_EXPECT
 */
#define VIP_TVO_SYNC_PAT_EXPECT_INI	0x00000001

/*
 * VIP_RGB_CNTL
 */
#define VIP_RGB_CNTL_RGB_IS_888_PACK	MASK_N_BIT(0)

/*
 * Value for VIP_VSCALER_CNTL2
 */
#define VIP_VSCALER_CNTL2_INI		0x10000000

/*
 * Value for VIP_Y_FALL_CNTL
 */
/* #define VIP_Y_FALL_CNTL_INI		0x00010200 */
#define VIP_Y_FALL_CNTL_INI		0x80030400

/*
 * VIP_UV_ADR
 */
#define VIP_UV_ADR_INI			0xc8
#define VIP_UV_ADR_HCODE_TABLE_SEL	0x06000000
#define VIP_UV_ADR_HCODE_TABLE_SEL_SHIFT 25
#define VIP_UV_ADR_VCODE_TABLE_SEL	0x18000000
#define VIP_UV_ADR_VCODE_TABLE_SEL_SHIFT 27
#define VIP_UV_ADR_MAX_UV_ADR		0x000000ff
#define VIP_UV_ADR_MAX_UV_ADR_SHIFT	0
#define VIP_UV_ADR_TABLE1_BOT_ADR	0x0000ff00
#define VIP_UV_ADR_TABLE1_BOT_ADR_SHIFT	8
#define VIP_UV_ADR_TABLE3_TOP_ADR	0x00ff0000
#define VIP_UV_ADR_TABLE3_TOP_ADR_SHIFT	16
#define MAX_FIFO_ADDR_RT		0x1a7
#define MAX_FIFO_ADDR_ERT		0x1ff

/*
 * VIP_HOST_RD_WT_CNTL
 */
#define VIP_HOST_RD_WT_CNTL_RD		MASK_N_BIT(12)
#define VIP_HOST_RD_WT_CNTL_RD_ACK	MASK_N_BIT(13)
#define VIP_HOST_RD_WT_CNTL_WT		MASK_N_BIT(14)
#define VIP_HOST_RD_WT_CNTL_WT_ACK	MASK_N_BIT(15)

/*
 * Value for VIP_SYNC_CNTL
 */
#define VIP_SYNC_CNTL_INI		0x28

/*
 * VIP_VSCALER_CNTL1
 */
#define VIP_VSCALER_CNTL1_UV_INC	0xffff
#define VIP_VSCALER_CNTL1_UV_INC_SHIFT	0

/*
 * VIP_TIMING_CNTL
 */
#define VIP_TIMING_CNTL_UV_OUT_POST_SCALE_SHIFT	24
#define VIP_TIMING_CNTL_INI		0x000b0000
#define VIP_TIMING_CNTL_H_INC_SHIFT	0
#define VIP_TIMING_CNTL_H_INC		0xfff

/*
 * Value for VIP_PRE_DAC_MUX_CNTL
 */
#define VIP_PRE_DAC_MUX_CNTL_INI	0x0000000f

/*
 * VIP_TV_DAC_CNTL
 */
#define VIP_TV_DAC_CNTL_NBLANK		MASK_N_BIT(0)
#define VIP_TV_DAC_CNTL_DASLEEP		MASK_N_BIT(3)
#define VIP_TV_DAC_CNTL_BGSLEEP		MASK_N_BIT(6)

/*
 * Value for VIP_FRAME_LOCK_CNTL
 */
#define VIP_FRAME_LOCK_CNTL_INI		0x0000000f

/*
 * Value for VIP_HW_DEBUG
 */
#define VIP_HW_DEBUG_INI 		0x00000200

/*
 * VIP_MASTER_CNTL
 */
#define VIP_MASTER_CNTL_TV_ASYNC_RST	MASK_N_BIT(0)
#define VIP_MASTER_CNTL_CRT_ASYNC_RST	MASK_N_BIT(1)
#define VIP_MASTER_CNTL_RESTART_PHASE_FIX	MASK_N_BIT(3)
#define VIP_MASTER_CNTL_TV_FIFO_ASYNC_RST	MASK_N_BIT(4)
#define VIP_MASTER_CNTL_VIN_ASYNC_RST	MASK_N_BIT(5)
#define VIP_MASTER_CNTL_AUD_ASYNC_RST	MASK_N_BIT(6)
#define VIP_MASTER_CNTL_DVS_ASYNC_RST	MASK_N_BIT(7)
#define VIP_MASTER_CNTL_CRT_FIFO_CE_EN	MASK_N_BIT(9)
#define VIP_MASTER_CNTL_TV_FIFO_CE_EN	MASK_N_BIT(10)
#define VIP_MASTER_CNTL_ON_INI		(VIP_MASTER_CNTL_RESTART_PHASE_FIX | \
					 VIP_MASTER_CNTL_VIN_ASYNC_RST | \
					 VIP_MASTER_CNTL_AUD_ASYNC_RST | \
					 VIP_MASTER_CNTL_DVS_ASYNC_RST | \
					 VIP_MASTER_CNTL_CRT_FIFO_CE_EN | \
					 VIP_MASTER_CNTL_TV_FIFO_CE_EN)
#define VIP_MASTER_CNTL_OFF_INI		(VIP_MASTER_CNTL_TV_ASYNC_RST | \
					 VIP_MASTER_CNTL_CRT_ASYNC_RST | \
					 VIP_MASTER_CNTL_RESTART_PHASE_FIX | \
					 VIP_MASTER_CNTL_TV_FIFO_ASYNC_RST | \
					 VIP_MASTER_CNTL_VIN_ASYNC_RST | \
					 VIP_MASTER_CNTL_AUD_ASYNC_RST | \
					 VIP_MASTER_CNTL_DVS_ASYNC_RST | \
					 VIP_MASTER_CNTL_CRT_FIFO_CE_EN | \
					 VIP_MASTER_CNTL_TV_FIFO_CE_EN)

/*
 * Value for VIP_LINEAR_GAIN_SETTINGS
 */
#define VIP_LINEAR_GAIN_SETTINGS_INI	0x01000100

/*
 * Value for VIP_GAIN_LIMIT_SETTINGS_INI
 */
#define VIP_GAIN_LIMIT_SETTINGS_INI	0x017f05ff

/*
 * Value for VIP_UPSAMP_AND_GAIN_CNTL 
 */
#define VIP_UPSAMP_AND_GAIN_CNTL_INI	0x00000005

/*
 * RADEON_VCLK_ECP_CNTL
 */
#define RADEON_VCLK_ECP_CNTL_BYTECLK_POSTDIV	0x00030000
#define RADEON_VCLK_ECP_CNTL_BYTECLK_NODIV	0x00000000

/*
 * RADEON_PLL_TEST_CNTL
 */
#define RADEON_PLL_TEST_CNTL_PLL_MASK_READ_B	MASK_N_BIT(9)

/*
 * RADEON_DAC_CNTL
 */
#define RADEON_DAC_CNTL_DAC_TVO_EN	MASK_N_BIT(10)

#define RADEON_PPLL_POST3_DIV_BY_2	0x10000
#define RADEON_PPLL_POST3_DIV_BY_3	0x40000
#define RADEON_PPLL_FB3_DIV_SHIFT	0
#define RADEON_PPLL_POST3_DIV_SHIFT	16

/*
 * RADEON_DISP_MERGE_CNTL
 */
#define	RADEON_DISP_MERGE_CNTL_INI	0xffff0000

/*
 * RADEON_HTOTAL_CNTL
 */
#define RADEON_HTOTAL_CNTL_HTOT_PIX_SLIP_SHIFT	0
#define RADEON_HTOTAL_CNTL_HTOT_CNTL_VGA_EN	MASK_N_BIT(28)

/*
 * RADEON_DISP_OUTPUT_CNTL
 */
#define RADEON_DISP_TV_SOURCE		MASK_N_BIT(16)
#define RADEON_DISP_TV_MODE_MASK	(3 << 17)
#define RADEON_DISP_TV_MODE_888		(0 << 17)
#define RADEON_DISP_TV_MODE_565		(1 << 17)
#define RADEON_DISP_TV_YG_DITH_EN	MASK_N_BIT(19)
#define RADEON_DISP_TV_CBB_CRR_DITH_EN	MASK_N_BIT(20)
#define RADEON_DISP_TV_BIT_WIDTH	MASK_N_BIT(21)
#define RADEON_DISP_TV_SYNC_MODE_MASK	(3 << 22)
#define RADEON_DISP_TV_SYNC_COLOR_MASK	(3 << 25)

/*
 * ERT registers
 */
#define TV_MASTER_CNTL		0x0800
#define		TV_MASTER_CNTL_TVCLK_ALWAYS_ON	MASK_N_BIT(30)
#define		TV_MASTER_CNTL_TV_ON	MASK_N_BIT(31)
#define		TV_MASTER_CNTL_ON_INI	(VIP_MASTER_CNTL_VIN_ASYNC_RST | \
					 VIP_MASTER_CNTL_CRT_FIFO_CE_EN | \
					 VIP_MASTER_CNTL_TV_FIFO_CE_EN | \
					 TV_MASTER_CNTL_TVCLK_ALWAYS_ON | \
					 TV_MASTER_CNTL_TV_ON)
#define		TV_MASTER_CNTL_OFF_INI	(VIP_MASTER_CNTL_TV_ASYNC_RST | \
					 VIP_MASTER_CNTL_CRT_ASYNC_RST | \
					 VIP_MASTER_CNTL_TV_FIFO_ASYNC_RST | \
					 VIP_MASTER_CNTL_CRT_FIFO_CE_EN | \
					 VIP_MASTER_CNTL_TV_FIFO_CE_EN | \
					 TV_MASTER_CNTL_TVCLK_ALWAYS_ON)
#define TV_RGB_CNTL		0x0804
#define		TV_RGB_CNTL_INI	0x007b0004
#define TV_SYNC_CNTL		0x0808
#define TV_HTOTAL		0x080c
#define TV_HDISP		0x0810
#define TV_HSTART		0x0818
#define TV_HCOUNT		0x081c
#define TV_VTOTAL		0x0820
#define TV_VDISP		0x0824
#define TV_VCOUNT		0x0828
#define TV_FTOTAL		0x082c
#define TV_FCOUNT		0x0830
#define TV_FRESTART		0x0834
#define TV_HRESTART		0x0838
#define TV_VRESTART		0x083c
#define TV_HOST_READ_DATA	0x0840
#define TV_HOST_WRITE_DATA	0x0844
#define TV_HOST_RD_WT_CNTL	0x0848
#define TV_VSCALER_CNTL1	0x084c
#define		TV_VSCALER_CNTL1_RESTART_FIELD	MASK_N_BIT(29)
#define TV_TIMING_CNTL		0x0850
#define TV_VSCALER_CNTL2	0x0854
#define TV_Y_FALL_CNTL		0x0858
#define TV_Y_RISE_CNTL		0x085c
#define TV_Y_SAWTOOTH_CNTL	0x0860
#define TV_UPSAMP_AND_GAIN_CNTL 0x0864
#define TV_GAIN_LIMIT_SETTINGS	0x0868
#define TV_LINEAR_GAIN_SETTINGS 0x086c
#define TV_MODULATOR_CNTL1	0x0870
#define TV_MODULATOR_CNTL2	0x0874
#define TV_PRE_DAC_MUX_CNTL	0x0888
#define TV_DAC_CNTL		0x088c
#define		TV_DAC_CNTL_NBLANK	MASK_N_BIT(0)
#define		TV_DAC_CNTL_NHOLD	MASK_N_BIT(1)
#define		TV_DAC_CNTL_BGSLEEP	MASK_N_BIT(6)
#define		TV_DAC_CNTL_RDACPD	MASK_N_BIT(24)
#define		TV_DAC_CNTL_GDACPD	MASK_N_BIT(25)
#define		TV_DAC_CNTL_BDACPD	MASK_N_BIT(26)
#define TV_CRC_CNTL		0x0890
#define TV_UV_ADR		0x08ac

/*
 * ERT PLL registers
 */
#define TV_PLL_CNTL		0x21
#define TV_PLL_CNTL1		0x22
#define		TV_PLL_CNTL1_TVPLL_RESET	MASK_N_BIT(1)
#define		TV_PLL_CNTL1_TVPLL_SLEEP	MASK_N_BIT(3)
#define		TV_PLL_CNTL1_TVPDC_SHIFT	14
#define		TV_PLL_CNTL1_TVPDC_MASK		(3 << 14)
#define		TV_PLL_CNTL1_TVCLK_SRC_SEL	MASK_N_BIT(30)

/*
 * Constant upsampler coefficients
 */
static
    const
unsigned int upsamplerCoeffs[] = {
	0x3f010000,
	0x7b008002,
	0x00003f01,
	0x341b7405,
	0x7f3a7617,
	0x00003d04,
	0x2d296c0a,
	0x0e316c2c,
	0x00003e7d,
	0x2d1f7503,
	0x2927643b,
	0x0000056f,
	0x29257205,
	0x25295050,
	0x00000572
};

#define N_UPSAMPLER_COEFFS	(sizeof(upsamplerCoeffs) / sizeof(upsamplerCoeffs[ 0 ]))

/*
 * Maximum length of horizontal/vertical code timing tables for state storage
 */
#define MAX_H_CODE_TIMING_LEN	32
#define MAX_V_CODE_TIMING_LEN	32

/*
 * Type of VIP bus
 */
#define VIP_TYPE	"ATI VIP BUS"

/*
 * Limits of h/v positions (hPos & vPos in TheaterOutRec)
 */
#define MAX_H_POSITION		5	/* Range: [-5..5], negative is on the lef 0 is default, positive is on the right */
#define MAX_V_POSITION		5	/* Range: [-5..5], negative is up, 0 is defaul positive is down */

/*
 * Unit for hPos (in TV clock periods)
 */
#define H_POS_UNIT		10

/*
 * Indexes in h. code timing table for horizontal line position adjustment
 */
#define H_TABLE_POS1		6
#define H_TABLE_POS2		8

/*
 * Limits of hor. size (hSize in TheaterOutRec)
 */
#define MAX_H_SIZE		5	/* Range: [-5..5], negative is smaller, positive is larger */

/**********************************************************************
 *
 * TimingTableEl
 *
 * Elements of H/V code timing tables
 *
 **********************************************************************/

typedef unsigned short TimingTableEl;	/* Bits 0 to 13 only are actually used */

/**********************************************************************
 *
 * ModeConstants
 *
 * Storage of constants related to a single video mode
 *
 **********************************************************************/

typedef struct {
	unsigned short horResolution;
	unsigned short verResolution;
	TVStd standard;
	unsigned short horTotal;
	unsigned short verTotal;
	unsigned short horStart;
	unsigned short horSyncStart;
	unsigned short verSyncStart;
	unsigned defRestart;
	unsigned int vScalerCntl1;
	unsigned int yRiseCntl;
	unsigned int ySawtoothCntl;
	unsigned short crtcPLL_N;
	unsigned char crtcPLL_M;
	Bool crtcPLL_divBy2;
	unsigned char crtcPLL_byteClkDiv;
	unsigned char crtcPLL_postDiv;
	Bool use888RGB;		/* False: RGB data is 565 packed (2 bytes/pixel) */
	/* True : RGB data is 888 packed (3 bytes/pixel) */
	unsigned pixToTV;
	unsigned char byteClkDelay;
	unsigned int tvoDataDelayA;
	unsigned int tvoDataDelayB;
	const TimingTableEl *horTimingTable;
	const TimingTableEl *verTimingTable;
} ModeConstants;

/**********************************************************************
 *
 * TheaterState
 *
 * Storage of RT state
 *
 **********************************************************************/

typedef struct {
	unsigned int clkout_cntl;
	unsigned int clock_sel_cntl;
	unsigned int crc_cntl;
	unsigned int crt_pll_cntl;
	unsigned int dfrestart;
	unsigned int dhrestart;
	unsigned int dvrestart;
	unsigned int frame_lock_cntl;
	unsigned int gain_limit_settings;
	unsigned int hdisp;
	unsigned int hstart;
	unsigned int htotal;
	unsigned int hw_debug;
	unsigned int linear_gain_settings;
	unsigned int master_cntl;
	unsigned int modulator_cntl1;
	unsigned int modulator_cntl2;
	unsigned int pll_cntl0;
	unsigned int pll_test_cntl;
	unsigned int pre_dac_mux_cntl;
	unsigned int rgb_cntl;
	unsigned int sync_cntl;
	unsigned int sync_lock_cntl;
	unsigned int sync_size;
	unsigned int timing_cntl;
	unsigned int tvo_data_delay_a;
	unsigned int tvo_data_delay_b;
	unsigned int tvo_sync_pat_expect;
	unsigned int tvo_sync_threshold;
	unsigned int tv_dac_cntl;
	unsigned int tv_pll_cntl;
	unsigned int tv_pll_fine_cntl;
	unsigned int upsamp_and_gain_cntl;
	unsigned int upsamp_coeffs[N_UPSAMPLER_COEFFS];
	unsigned int uv_adr;
	unsigned int vdisp;
	unsigned int vftotal;
	unsigned int vscaler_cntl1;
	unsigned int vscaler_cntl2;
	unsigned int vtotal;
	unsigned int y_fall_cntl;
	unsigned int y_rise_cntl;
	unsigned int y_saw_tooth_cntl;
	unsigned int disp_merge_cntl;

	TimingTableEl h_code_timing[MAX_H_CODE_TIMING_LEN];
	TimingTableEl v_code_timing[MAX_V_CODE_TIMING_LEN];
} TheaterState, *TheaterStatePtr;

/**********************************************************************
 *
 * TVConstants
 *
 * Constants that depend on tv standard only
 *
 **********************************************************************/

typedef struct {
	unsigned char tvPLL_M;
	unsigned short tvPLL_N;
	unsigned char tvPLL_postDiv;
	unsigned int tvClockT;	/* Period of TV clock (unit = 100 psec) */
	unsigned int modulatorCntl1;
	unsigned int modulatorCntl2;
	unsigned int vip_tvDAC_Cntl;
	unsigned int ert_tvDAC_Cntl;
	unsigned int vftotal;
	unsigned linesFrame;
	unsigned zeroHSize;	/* Length of the picture part of a hor. line for hSize = 0 (unit = 100 psec) */
	unsigned hSizeUnit;	/* Value of hSize = 1 (unit = 100 psec) */
} TVConstants;

/**********************************************************************
 *
 * tvStdConsts
 *
 * Table of constants for tv standards (index is a TVStd)
 *
 **********************************************************************/

static
    const
TVConstants tvStdConsts[] = {
	/*
	 * NTSC
	 */
	{
	 22,			/* tvPLL_M */
	 175,			/* tvPLL_N */
	 5,			/* tvPLL_postDiv */
	 233,			/* tvClockT */
	 0x60bb468c,		/* modulatorCntl1 */
	 0x00000191,		/* modulatorCntl2 */
	 0x00000113,		/* vip_tvDAC_Cntl */
	 0x00680113,		/* ert_tvDAC_Cntl */
	 1,			/* vftotal */
	 525,			/* linesFrame */
	 479166,		/* zeroHSize */
	 9478			/* hSizeUnit */
	 },
	/*
	 * PAL
	 */
	{
	 113,			/* tvPLL_M */
	 668,			/* tvPLL_N */
	 3,			/* tvPLL_postDiv */
	 188,			/* tvClockT */
	 0x60bb3bcc,		/* modulatorCntl1 */
	 0x003e01b2,		/* modulatorCntl2 */
	 0x00000013,		/* vip_tvDAC_Cntl */
	 0x00680013,		/* ert_tvDAC_Cntl */
	 3,			/* vftotal */
	 625,			/* linesFrame */
	 473200,		/* zeroHSize */
	 9360			/* hSizeUnit */
	 }
};

/**********************************************************************
 *
 * availableModes
 *
 * Table of all allowed modes for tv output
 *
 **********************************************************************/

static
    const
TimingTableEl horTimingNTSC_BIOS[] = {
	0x0007,
	0x003f,
	0x0263,
	0x0a24,
	0x2a6b,
	0x0a36,
	0x126d,			/* H_TABLE_POS1 */
	0x1bfe,
	0x1a8f,			/* H_TABLE_POS2 */
	0x1ec7,
	0x3863,
	0x1bfe,
	0x1bfe,
	0x1a2a,
	0x1e95,
	0x0e31,
	0x201b,
	0
};

static
    const
TimingTableEl verTimingNTSC_BIOS[] = {
	0x2001,
	0x200d,
	0x1006,
	0x0c06,
	0x1006,
	0x1818,
	0x21e3,
	0x1006,
	0x0c06,
	0x1006,
	0x1817,
	0x21d4,
	0x0002,
	0
};

static
    const
TimingTableEl horTimingPAL_BIOS[] = {
	0x0007,
	0x0058,
	0x027c,
	0x0a31,
	0x2a77,
	0x0a95,
	0x124f,			/* H_TABLE_POS1 */
	0x1bfe,
	0x1b22,			/* H_TABLE_POS2 */
	0x1ef9,
	0x387c,
	0x1bfe,
	0x1bfe,
	0x1b31,
	0x1eb5,
	0x0e43,
	0x201b,
	0
};

static
    const
TimingTableEl verTimingPAL_BIOS[] = {
	0x2001,
	0x200c,
	0x1005,
	0x0c05,
	0x1005,
	0x1401,
	0x1821,
	0x2240,
	0x1005,
	0x0c05,
	0x1005,
	0x1401,
	0x1822,
	0x2230,
	0x0002,
	0
};

static
    const
ModeConstants availableModes[] = {
	{
	 800,			/* horResolution */
	 600,			/* verResolution */
	 TV_STD_NTSC,		/* standard */
	 990,			/* horTotal */
	 740,			/* verTotal */
	 813,			/* horStart */
	 824,			/* horSyncStart */
	 632,			/* verSyncStart */
	 625592,		/* defRestart */
	 0x0900b46b,		/* vScalerCntl1 */
	 0x00012c00,		/* yRiseCntl */
	 0x10002d1a,		/* ySawtoothCntl */
	 592,			/* crtcPLL_N */
	 91,			/* crtcPLL_M */
	 1,			/* crtcPLL_divBy2 */
	 0,			/* crtcPLL_byteClkDiv */
	 4,			/* crtcPLL_postDiv */
	 0,			/* use888RGB */
	 1022,			/* pixToTV */
	 1,			/* byteClkDelay */
	 0x0a0b0907,		/* tvoDataDelayA */
	 0x060a090a,		/* tvoDataDelayB */
	 horTimingNTSC_BIOS,	/* horTimingTable */
	 verTimingNTSC_BIOS	/* verTimingTable */
	 },
	{
	 800,			/* horResolution */
	 600,			/* verResolution */
	 TV_STD_PAL,		/* standard */
	 1144,			/* horTotal */
	 706,			/* verTotal */
	 812,			/* horStart */
	 824,			/* horSyncStart */
	 669,			/* verSyncStart */
	 696700,		/* defRestart */
	 0x09009097,		/* vScalerCntl1 */
	 0x000007da,		/* yRiseCntl */
	 0x10002426,		/* ySawtoothCntl */
	 1382,			/* crtcPLL_N */
	 231,			/* crtcPLL_M */
	 1,			/* crtcPLL_divBy2 */
	 0,			/* crtcPLL_byteClkDiv */
	 4,			/* crtcPLL_postDiv */
	 0,			/* use888RGB */
	 759,			/* pixToTV */
	 1,			/* byteClkDelay */
	 0x0a0b0907,		/* tvoDataDelayA */
	 0x060a090a,		/* tvoDataDelayB */
	 horTimingPAL_BIOS,	/* horTimingTable */
	 verTimingPAL_BIOS	/* verTimingTable */
	 }
};

#define N_AVAILABLE_MODES	(sizeof(availableModes) / sizeof(availableModes[ 0 ]))

TheaterState st;

/**********************************************************************
 *
 * ert_read
 *
 * Read from an ERT register
 *
 **********************************************************************/

static
void ert_read(unsigned int reg, unsigned int *data)
{
	*data = MMINL(reg);
	RTTRACE(("ert_read : %x = %x\n", reg, *data));
}

/**********************************************************************
 *
 * ert_write
 *
 * Write to an ERT register
 *
 **********************************************************************/
static
void ert_write(unsigned int reg, unsigned int data)
{
	RTTRACE(("ert_write: %x = %x\n", reg, data));
	MMOUTL(reg, data);
}

/**********************************************************************
 *
 * waitPLL_lock
 *
 * Wait for PLLs to lock
 *
 **********************************************************************/

static
void waitPLL_lock(unsigned nTests, unsigned nWaitLoops, unsigned cntThreshold)
{
	unsigned int savePLLTest;
	unsigned i;
	unsigned j;

	MMOUTL(RADEON_TEST_DEBUG_MUX,
	       (MMINL(RADEON_TEST_DEBUG_MUX) & 0xffff60ff) | 0x100);

	savePLLTest = INPLL(RADEON_PLL_TEST_CNTL);

	OUTPLL(RADEON_PLL_TEST_CNTL,
	       savePLLTest & ~RADEON_PLL_TEST_CNTL_PLL_MASK_READ_B);

	MMOUTB(RADEON_CLOCK_CNTL_INDEX, RADEON_PLL_TEST_CNTL);

	for (i = 0; i < nTests; i++) {
		MMOUTB(RADEON_CLOCK_CNTL_DATA + 3, 0);

		for (j = 0; j < nWaitLoops; j++)
			if (MMINB(RADEON_CLOCK_CNTL_DATA + 3) >= cntThreshold)
				break;
	}

	OUTPLL(RADEON_PLL_TEST_CNTL, savePLLTest);

	MMOUTL(RADEON_TEST_DEBUG_MUX,
	       MMINL(RADEON_TEST_DEBUG_MUX) & 0xffffe0ff);
}

/**********************************************************************
 *
 * writeFIFO
 *
 * Write to RT FIFO RAM
 *
 **********************************************************************/

static
void writeFIFO(unsigned short addr, unsigned int value)
{
	unsigned int tmp;

	ert_write(TV_HOST_WRITE_DATA, value);

	ert_write(TV_HOST_RD_WT_CNTL, addr | VIP_HOST_RD_WT_CNTL_WT);

	do {
		ert_read(TV_HOST_RD_WT_CNTL, &tmp);
	}
	while ((tmp & VIP_HOST_RD_WT_CNTL_WT_ACK) == 0);

	ert_write(TV_HOST_RD_WT_CNTL, 0);
}

/**********************************************************************
 *
 * readFIFO
 *
 * Read from RT FIFO RAM
 *
 **********************************************************************/

static
void readFIFO(unsigned short addr, unsigned int *value)
{
	unsigned int tmp;

	ert_write(TV_HOST_RD_WT_CNTL, addr | VIP_HOST_RD_WT_CNTL_RD);

	do {
		ert_read(TV_HOST_RD_WT_CNTL, &tmp);
	}
	while ((tmp & VIP_HOST_RD_WT_CNTL_RD_ACK) == 0);

	ert_write(TV_HOST_RD_WT_CNTL, 0);

	ert_read(TV_HOST_READ_DATA, value);
}

/**********************************************************************
 *
 * getTimingTablesAddr
 *
 * Get FIFO addresses of horizontal & vertical code timing tables from
 * settings of uv_adr register.
 *
 **********************************************************************/

static
void
getTimingTablesAddr(unsigned int uv_adr,
		    Bool isERT, unsigned short *hTable, unsigned short *vTable)
{
	switch ((uv_adr & VIP_UV_ADR_HCODE_TABLE_SEL) >>
		VIP_UV_ADR_HCODE_TABLE_SEL_SHIFT) {
	case 0:
		*hTable = isERT ? MAX_FIFO_ADDR_ERT : MAX_FIFO_ADDR_RT;
		break;

	case 1:
		*hTable =
		    ((uv_adr & VIP_UV_ADR_TABLE1_BOT_ADR) >>
		     VIP_UV_ADR_TABLE1_BOT_ADR_SHIFT) * 2;
		break;

	case 2:
		*hTable =
		    ((uv_adr & VIP_UV_ADR_TABLE3_TOP_ADR) >>
		     VIP_UV_ADR_TABLE3_TOP_ADR_SHIFT) * 2;
		break;

	default:
		/*
		 * Of course, this should never happen
		 */
		*hTable = 0;
		break;
	}

	switch ((uv_adr & VIP_UV_ADR_VCODE_TABLE_SEL) >>
		VIP_UV_ADR_VCODE_TABLE_SEL_SHIFT) {
	case 0:
		*vTable =
		    ((uv_adr & VIP_UV_ADR_MAX_UV_ADR) >>
		     VIP_UV_ADR_MAX_UV_ADR_SHIFT) * 2 + 1;
		break;

	case 1:
		*vTable =
		    ((uv_adr & VIP_UV_ADR_TABLE1_BOT_ADR) >>
		     VIP_UV_ADR_TABLE1_BOT_ADR_SHIFT) * 2 + 1;
		break;

	case 2:
		*vTable =
		    ((uv_adr & VIP_UV_ADR_TABLE3_TOP_ADR) >>
		     VIP_UV_ADR_TABLE3_TOP_ADR_SHIFT) * 2 + 1;
		break;

	default:
		/*
		 * Of course, this should never happen
		 */
		*vTable = 0;
		break;
	}
}

/**********************************************************************
 *
 * saveTimingTables
 *
 * Save horizontal/vertical timing code tables
 *
 **********************************************************************/
static
void saveTimingTables(TheaterStatePtr save)
{
	unsigned short hTable;
	unsigned short vTable;
	unsigned int tmp;
	unsigned i;

	ert_read(TV_UV_ADR, &save->uv_adr);
	getTimingTablesAddr(save->uv_adr, 1, &hTable, &vTable);

	/*
	 * Reset FIFO arbiter in order to be able to access FIFO RAM
	 */
	ert_write(TV_MASTER_CNTL, save->master_cntl | TV_MASTER_CNTL_TV_ON);

	RTTRACE(("saveTimingTables: reading timing tables\n"));

	for (i = 0; i < MAX_H_CODE_TIMING_LEN; i += 2) {
		readFIFO(hTable--, &tmp);
		save->h_code_timing[i] = (unsigned short)((tmp >> 14) & 0x3fff);
		save->h_code_timing[i + 1] = (unsigned short)(tmp & 0x3fff);

		if (save->h_code_timing[i] == 0
		    || save->h_code_timing[i + 1] == 0)
			break;
	}

	for (i = 0; i < MAX_V_CODE_TIMING_LEN; i += 2) {
		readFIFO(vTable++, &tmp);
		save->v_code_timing[i] = (unsigned short)(tmp & 0x3fff);
		save->v_code_timing[i + 1] =
		    (unsigned short)((tmp >> 14) & 0x3fff);

		if (save->v_code_timing[i] == 0
		    || save->v_code_timing[i + 1] == 0)
			break;
	}
}

/**********************************************************************
 *
 * restoreTimingTables
 *
 * Load horizontal/vertical timing code tables
 *
 **********************************************************************/

static
void restoreTimingTables(TheaterStatePtr restore)
{
	unsigned short hTable;
	unsigned short vTable;
	unsigned int tmp;
	unsigned i;

	ert_write(TV_UV_ADR, restore->uv_adr);
	getTimingTablesAddr(restore->uv_adr, 1, &hTable, &vTable);

	for (i = 0; i < MAX_H_CODE_TIMING_LEN; i += 2, hTable--) {
		tmp =
		    ((unsigned int)restore->
		     h_code_timing[i] << 14) | ((unsigned int)restore->
						h_code_timing[i + 1]);
		writeFIFO(hTable, tmp);
		if (restore->h_code_timing[i] == 0
		    || restore->h_code_timing[i + 1] == 0)
			break;
	}

	for (i = 0; i < MAX_V_CODE_TIMING_LEN; i += 2, vTable++) {
		tmp =
		    ((unsigned int)restore->
		     v_code_timing[i +
				   1] << 14) | ((unsigned int)restore->
						v_code_timing[i]);
		writeFIFO(vTable, tmp);
		if (restore->v_code_timing[i] == 0
		    || restore->v_code_timing[i + 1] == 0)
			break;
	}
}

/**********************************************************************
 *
 * ERT_RestorePLL
 *
 * Set ERT PLLs
 *
 **********************************************************************/
static
void ERT_RestorePLL(TheaterStatePtr restore)
{
	OUTPLLP(TV_PLL_CNTL1, 0, ~TV_PLL_CNTL1_TVCLK_SRC_SEL);
	OUTPLL(TV_PLL_CNTL, restore->tv_pll_cntl);
	OUTPLLP(TV_PLL_CNTL1, TV_PLL_CNTL1_TVPLL_RESET,
		~TV_PLL_CNTL1_TVPLL_RESET);

	waitPLL_lock(200, 800, 135);

	OUTPLLP(TV_PLL_CNTL1, 0, ~TV_PLL_CNTL1_TVPLL_RESET);

	waitPLL_lock(300, 160, 27);
	waitPLL_lock(200, 800, 135);

	OUTPLLP(TV_PLL_CNTL1, 0, ~0xf);
	OUTPLLP(TV_PLL_CNTL1, TV_PLL_CNTL1_TVCLK_SRC_SEL,
		~TV_PLL_CNTL1_TVCLK_SRC_SEL);

	OUTPLLP(TV_PLL_CNTL1, (1 << TV_PLL_CNTL1_TVPDC_SHIFT),
		~TV_PLL_CNTL1_TVPDC_MASK);
	OUTPLLP(TV_PLL_CNTL1, 0, ~TV_PLL_CNTL1_TVPLL_SLEEP);
}

/**********************************************************************
 *
 * ERT_RestoreHV
 *
 * Set ERT horizontal/vertical settings
 *
 **********************************************************************/

static
void ERT_RestoreHV(TheaterStatePtr restore)
{
	ert_write(TV_RGB_CNTL, restore->rgb_cntl);

	ert_write(TV_HTOTAL, restore->htotal);
	ert_write(TV_HDISP, restore->hdisp);
	ert_write(TV_HSTART, restore->hstart);

	ert_write(TV_VTOTAL, restore->vtotal);
	ert_write(TV_VDISP, restore->vdisp);

	ert_write(TV_FTOTAL, restore->vftotal);

	ert_write(TV_VSCALER_CNTL1, restore->vscaler_cntl1);
	ert_write(TV_VSCALER_CNTL2, restore->vscaler_cntl2);

	ert_write(TV_Y_FALL_CNTL, restore->y_fall_cntl);
	ert_write(TV_Y_RISE_CNTL, restore->y_rise_cntl);
	ert_write(TV_Y_SAWTOOTH_CNTL, restore->y_saw_tooth_cntl);
}

/**********************************************************************
 *
 * ERT_RestoreRestarts
 *
 * Set ERT TV_*RESTART registers
 *
 **********************************************************************/

static
void ERT_RestoreRestarts(TheaterStatePtr restore)
{
	ert_write(TV_FRESTART, restore->dfrestart);
	ert_write(TV_HRESTART, restore->dhrestart);
	ert_write(TV_VRESTART, restore->dvrestart);
}

/**********************************************************************
 *
 * ERT_RestoreOutputStd
 *
 * Set tv standard & output muxes
 *
 **********************************************************************/
static
void ERT_RestoreOutputStd(TheaterStatePtr restore)
{
	ert_write(TV_SYNC_CNTL, restore->sync_cntl);

	ert_write(TV_TIMING_CNTL, restore->timing_cntl);

	ert_write(TV_MODULATOR_CNTL1, restore->modulator_cntl1);
	ert_write(TV_MODULATOR_CNTL2, restore->modulator_cntl2);

	ert_write(TV_PRE_DAC_MUX_CNTL, restore->pre_dac_mux_cntl);

	ert_write(TV_CRC_CNTL, restore->crc_cntl);
}

/**********************************************************************
 *
 * ERT_IsOn
 *
 * Test if tv output would be enabled with a given value in TV_DAC_CNTL
 *
 **********************************************************************/
static Bool ERT_IsOn(unsigned int tv_dac_cntl)
{
	if (tv_dac_cntl & TV_DAC_CNTL_BGSLEEP)
		return 0;
	else if ((tv_dac_cntl &
		  (TV_DAC_CNTL_RDACPD | TV_DAC_CNTL_GDACPD |
		   TV_DAC_CNTL_BDACPD)) ==
		 (TV_DAC_CNTL_RDACPD | TV_DAC_CNTL_GDACPD | TV_DAC_CNTL_BDACPD))
		return 0;
	else
		return 1;
}

/**********************************************************************
 *
 * ERT_Restore
 *
 * Restore state of ERT
 *
 **********************************************************************/
static
void ERT_Restore(TheaterStatePtr restore)
{
	RTTRACE(("Entering ERT_Restore\n"));

	ert_write(TV_MASTER_CNTL, restore->master_cntl | TV_MASTER_CNTL_TV_ON);

	ert_write(TV_MASTER_CNTL,
		  restore->master_cntl |
		  VIP_MASTER_CNTL_TV_ASYNC_RST |
		  VIP_MASTER_CNTL_CRT_ASYNC_RST |
		  VIP_MASTER_CNTL_RESTART_PHASE_FIX |
		  VIP_MASTER_CNTL_TV_FIFO_ASYNC_RST);

	/*
	 * Temporarily turn the TV DAC off
	 */
	ert_write(TV_DAC_CNTL,
		  (restore->tv_dac_cntl & ~TV_DAC_CNTL_NBLANK) |
		  TV_DAC_CNTL_BGSLEEP |
		  TV_DAC_CNTL_RDACPD | TV_DAC_CNTL_GDACPD | TV_DAC_CNTL_BDACPD);

	RTTRACE(("ERT_Restore: checkpoint 1\n"));
	ERT_RestorePLL(restore);

	RTTRACE(("ERT_Restore: checkpoint 2\n"));
	ERT_RestoreHV(restore);

	ert_write(TV_MASTER_CNTL,
		  restore->master_cntl |
		  VIP_MASTER_CNTL_TV_ASYNC_RST |
		  VIP_MASTER_CNTL_CRT_ASYNC_RST |
		  VIP_MASTER_CNTL_RESTART_PHASE_FIX);

	RTTRACE(("ERT_Restore: checkpoint 3\n"));
	ERT_RestoreRestarts(restore);

	RTTRACE(("ERT_Restore: checkpoint 4\n"));

	/*
	 * Timing tables are only restored when tv output is active
	 */
	if (ERT_IsOn(restore->tv_dac_cntl))
		restoreTimingTables(restore);

	ert_write(TV_MASTER_CNTL,
		  restore->master_cntl |
		  VIP_MASTER_CNTL_TV_ASYNC_RST |
		  VIP_MASTER_CNTL_RESTART_PHASE_FIX);

	RTTRACE(("ERT_Restore: checkpoint 5\n"));
	ERT_RestoreOutputStd(restore);

	ert_write(TV_MASTER_CNTL, restore->master_cntl);

	ert_write(RADEON_DISP_MERGE_CNTL, restore->disp_merge_cntl);

	ert_write(TV_GAIN_LIMIT_SETTINGS, restore->gain_limit_settings);
	ert_write(TV_LINEAR_GAIN_SETTINGS, restore->linear_gain_settings);

	ert_write(TV_DAC_CNTL, restore->tv_dac_cntl);

	RTTRACE(("Leaving ERT_Restore\n"));
}

/**********************************************************************
 *
 * computeRestarts
 *
 * Compute F,V,H restarts from default restart position and 
 * hPos & vPos
 * Return 1 when code timing table was changed
 *
 **********************************************************************/

static
    Bool
computeRestarts(const ModeConstants * constPtr,
		TVStd tvStd,
		int hPos, int vPos, int hSize, TheaterStatePtr save)
{
	int restart;
	const TVConstants *pTvStd = &tvStdConsts[tvStd];
	unsigned hTotal;
	unsigned vTotal;
	unsigned fTotal;
	int vOffset;
	int hOffset;
	TimingTableEl p1;
	TimingTableEl p2;
	Bool hChanged;
	unsigned short hInc;

	hTotal = constPtr->horTotal;
	vTotal = constPtr->verTotal;
	fTotal = pTvStd->vftotal + 1;

	/*
	 * Adjust positions 1&2 in hor. code timing table
	 */
	hOffset = hPos * H_POS_UNIT;

	p1 = constPtr->horTimingTable[H_TABLE_POS1];
	p2 = constPtr->horTimingTable[H_TABLE_POS2];

	p1 = (TimingTableEl) ((int)p1 + hOffset);
	p2 = (TimingTableEl) ((int)p2 - hOffset);

	hChanged = (p1 != save->h_code_timing[H_TABLE_POS1] ||
		    p2 != save->h_code_timing[H_TABLE_POS2]);

	save->h_code_timing[H_TABLE_POS1] = p1;
	save->h_code_timing[H_TABLE_POS2] = p2;

	/*
	 * Convert hOffset from n. of TV clock periods to n. of CRTC clock periods (CRTC pixels)
	 */
	hOffset = (hOffset * (int)(constPtr->pixToTV)) / 1000;

	/*
	 * Adjust restart
	 */
	restart = constPtr->defRestart;

	/*
	 * Convert vPos TV lines to n. of CRTC pixels
	 * Be verrrrry careful when mixing signed & unsigned values in C..
	 */
	vOffset =
	    ((int)(vTotal * hTotal) * 2 * vPos) / (int)(pTvStd->linesFrame);

	restart -= vOffset + hOffset;

	RTTRACE(("computeRestarts: def = %u, h = %d , v = %d , p1=%04x , p2=%04x , restart = %d\n", constPtr->defRestart, hPos, vPos, p1, p2, restart));

	save->dhrestart = restart % hTotal;
	restart /= hTotal;
	save->dvrestart = restart % vTotal;
	restart /= vTotal;
	save->dfrestart = restart % fTotal;

	RTTRACE(("computeRestarts: F/H/V=%u,%u,%u\n", save->dfrestart,
		 save->dvrestart, save->dhrestart));

	/*
	 * Compute H_INC from hSize
	 */
	hInc =
	    (unsigned
	     short)((int)(constPtr->horResolution * 4096 * pTvStd->tvClockT) /
		    (hSize * (int)(pTvStd->hSizeUnit) +
		     (int)(pTvStd->zeroHSize)));
	save->timing_cntl =
	    (save->
	     timing_cntl & ~VIP_TIMING_CNTL_H_INC) | ((unsigned int)hInc <<
						      VIP_TIMING_CNTL_H_INC_SHIFT);

	RTTRACE(("computeRestarts: hSize=%d,hInc=%u\n", hSize, hInc));

	return hChanged;
}

/**********************************************************************
 *
 * RT_Init
 *
 * Define RT state for a given standard/resolution combination
 *
 **********************************************************************/

static
void
RT_Init(const ModeConstants * constPtr,
	TVStd tvStd,
	Bool enable, int hPos, int vPos, int hSize, TheaterStatePtr save)
{
	unsigned i;
	unsigned int tmp;
	const TVConstants *pTvStd = &tvStdConsts[tvStd];

	save->clkout_cntl = VIP_CLKOUT_CNTL_INI;

	save->clock_sel_cntl = VIP_CLOCK_SEL_CNTL_INI |
	    (constPtr->crtcPLL_byteClkDiv << VIP_CLOCK_SEL_CNTL_BYTCLK_SHIFT) |
	    (constPtr->byteClkDelay << VIP_CLOCK_SEL_CNTL_BYTCLKD_SHIFT);

	save->crc_cntl = 0;

	tmp = ((unsigned int)constPtr->crtcPLL_M << VIP_CRT_PLL_CNTL_M_SHIFT) |
	    (((unsigned int)constPtr->
	      crtcPLL_N & VIP_CRT_PLL_CNTL_NLO) << VIP_CRT_PLL_CNTL_NLO_SHIFT) |
	    (((unsigned int)constPtr->
	      crtcPLL_N & VIP_CRT_PLL_CNTL_NHI) << VIP_CRT_PLL_CNTL_NHI_SHIFT);
	if (constPtr->crtcPLL_divBy2)
		tmp |= VIP_CRT_PLL_CNTL_CLKBY2;
	save->crt_pll_cntl = tmp;

	save->frame_lock_cntl = VIP_FRAME_LOCK_CNTL_INI;

	save->gain_limit_settings = VIP_GAIN_LIMIT_SETTINGS_INI;

	save->hdisp = constPtr->horResolution - 1;
	save->hstart = constPtr->horStart;
	save->htotal = constPtr->horTotal - 1;

	save->hw_debug = VIP_HW_DEBUG_INI;

	save->linear_gain_settings = VIP_LINEAR_GAIN_SETTINGS_INI;

	/*
	 * TEST   TEST   TEST   TEST   TEST   TEST   TEST   TEST   TEST   
	 */
	save->master_cntl =
	    enable ? TV_MASTER_CNTL_ON_INI : TV_MASTER_CNTL_OFF_INI;

	save->modulator_cntl1 = pTvStd->modulatorCntl1;
	save->modulator_cntl2 = pTvStd->modulatorCntl2;

	save->pll_cntl0 = VIP_PLL_CNTL0_INI;
	save->pll_test_cntl = VIP_PLL_TEST_CNTL_INI;

	save->pre_dac_mux_cntl = VIP_PRE_DAC_MUX_CNTL_INI;

	save->rgb_cntl = TV_RGB_CNTL_INI;

	save->sync_cntl = VIP_SYNC_CNTL_INI;

	save->sync_lock_cntl = VIP_SYNC_LOCK_CNTL_INI;

	save->sync_size = constPtr->horResolution + 8;

	tmp =
	    (constPtr->
	     vScalerCntl1 >> VIP_VSCALER_CNTL1_UV_INC_SHIFT) &
	    VIP_VSCALER_CNTL1_UV_INC;
	tmp = ((16384 * 256 * 10) / tmp + 5) / 10;
	tmp = (tmp << VIP_TIMING_CNTL_UV_OUT_POST_SCALE_SHIFT) |
	    VIP_TIMING_CNTL_INI;
	save->timing_cntl = tmp;

	save->tvo_data_delay_a = constPtr->tvoDataDelayA;
	save->tvo_data_delay_b = constPtr->tvoDataDelayB;

	save->tvo_sync_pat_expect = VIP_TVO_SYNC_PAT_EXPECT_INI;

	if (constPtr->use888RGB)
		save->tvo_sync_threshold =
		    constPtr->horResolution + constPtr->horResolution / 2;
	else
		save->tvo_sync_threshold = constPtr->horResolution;

	if (enable)
		save->tv_dac_cntl = pTvStd->ert_tvDAC_Cntl;
	else
		save->tv_dac_cntl =
		    (pTvStd->
		     ert_tvDAC_Cntl & ~(TV_DAC_CNTL_NBLANK | TV_DAC_CNTL_NHOLD))
		    | (TV_DAC_CNTL_BGSLEEP | TV_DAC_CNTL_RDACPD |
		       TV_DAC_CNTL_GDACPD | TV_DAC_CNTL_BDACPD);

	tmp = ((unsigned int)(pTvStd->tvPLL_M) << VIP_TV_PLL_CNTL_M_SHIFT) |
	    (((unsigned int)(pTvStd->
			     tvPLL_N) & VIP_TV_PLL_CNTL_NLO) <<
	     VIP_TV_PLL_CNTL_NLO_SHIFT) | (((unsigned int)(pTvStd->tvPLL_N) &
					    VIP_TV_PLL_CNTL_NHI) <<
					   VIP_TV_PLL_CNTL_NHI_SHIFT) |
	    ((unsigned int)(pTvStd->tvPLL_postDiv) << VIP_TV_PLL_CNTL_P_SHIFT);
	save->tv_pll_cntl = tmp;
	save->tv_pll_fine_cntl = TV_PLL_FINE_INI;

	save->upsamp_and_gain_cntl = VIP_UPSAMP_AND_GAIN_CNTL_INI;

	memcpy(&save->upsamp_coeffs[0], upsamplerCoeffs,
	       sizeof(save->upsamp_coeffs));

	save->uv_adr = VIP_UV_ADR_INI;

	save->vdisp = constPtr->verResolution - 1;
	save->vftotal = pTvStd->vftotal;

	save->vscaler_cntl1 = constPtr->vScalerCntl1;
	save->vscaler_cntl1 |= TV_VSCALER_CNTL1_RESTART_FIELD;
	save->vscaler_cntl2 = VIP_VSCALER_CNTL2_INI;

	save->vtotal = constPtr->verTotal - 1;

	save->y_fall_cntl = VIP_Y_FALL_CNTL_INI;
	save->y_rise_cntl = constPtr->yRiseCntl;
	save->y_saw_tooth_cntl = constPtr->ySawtoothCntl;

	save->disp_merge_cntl = RADEON_DISP_MERGE_CNTL_INI;

	for (i = 0; i < MAX_H_CODE_TIMING_LEN; i++) {
		if ((save->h_code_timing[i] = constPtr->horTimingTable[i]) == 0)
			break;
	}

	for (i = 0; i < MAX_V_CODE_TIMING_LEN; i++) {
		if ((save->v_code_timing[i] = constPtr->verTimingTable[i]) == 0)
			break;
	}

	/*
	 * This must be called AFTER loading timing tables as they are modified by this function
	 */
	computeRestarts(constPtr, tvStd, hPos, vPos, hSize, save);
}

/**********************************************************************
 *
 * ERTAutoDetect
 *
 **********************************************************************/
static
Bool ERTAutoDetect(void)
{
	unsigned int saveReg = MMINL(TV_LINEAR_GAIN_SETTINGS);
	int detected = 0;

	/*
	 * Ultra-dumb way of detecting an ERT: check that a register is present 
	 * @ TV_LINEAR_GAIN_SETTINGS (this is probably one of the most harmless
	 * register to touch)
	 */
	MMOUTL(TV_LINEAR_GAIN_SETTINGS, 0x15500aa);

	if (MMINL(TV_LINEAR_GAIN_SETTINGS) == 0x15500aa) {
		MMOUTL(TV_LINEAR_GAIN_SETTINGS, 0x0aa0155);
		if (MMINL(TV_LINEAR_GAIN_SETTINGS) == 0x0aa0155) {
			detected = 1;
			RTTRACE(("tv out module found!\n"));
		}
	}

	MMOUTL(TV_LINEAR_GAIN_SETTINGS, saveReg);

	return detected;
}

void radeon_tvout_init(int tvmode)
{
	RTTRACE(("tv out module initing...\n"));
	if (!ERTAutoDetect())
		return;
	RT_Init(&availableModes[tvmode], tvmode, 1, -3, 0, 0, &st);
	ERT_Restore(&st);
	RTTRACE(("done!\n"));
}
