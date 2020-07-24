#define DLL_DATA 0x0000000010002010

//ref 50M vco 3.0G, well
#if (DDR_FREQ == 333)
#define PLL_LOOP   36  //9bit
#define PLL_DIVOUT 9   //7bit
#define PLL_DIVREF 2   //7bit
#elif   (DDR_FREQ == 400 )
#define PLL_LOOP   28  //9bit
#define PLL_DIVOUT 7   //7bit
#define PLL_DIVREF 2   //7bit
#else
#define PLL_LOOP   20  //9bit
#define PLL_DIVOUT 5   //7bit
#define PLL_DIVREF 2   //7bit
#endif

#if ( SEL_DBL == 0 )
#define PLL_PD     0x0 //1bit
#else
#define PLL_PD     0x1 //1bit
#endif
#define PLL_CG     0x1 //1bit

//#define SEL_DBL    0x1

#define PLL_CFG ((PLL_CG<<25)|(PLL_PD<<24)|(PLL_DIVREF<<17)|(PLL_DIVOUT<<10)|(PLL_LOOP<<1)|(PLL_BYPASS))

#define doub_DLL_DLY 0x2a //8bit
#define doub_comp    0  //1bit
#define doub_compbypass 0 //1bit
#define doub_callown    2 //3bit
#define doub_callowp    0 //3bit
#define doub_calhighn   0 //3bit
#define doub_calhighp   0 //3bit
#define doub_bypass     0 //3bit
    
#define DOUB_CFG ((doub_bypass<<46)|(doub_calhighp<<43)|(doub_calhighn<<40)|(doub_callowp<<37)|(doub_callown<<34)|(doub_compbypass<<33)|(doub_comp<<32)|(doub_DLL_DLY))

#define RDODT_CTRL  0x82
//#define RDGATE_MODE 0x3
//#define RDGATE_LEN  0x3
//#define RDGATE_CTRL 0x88
#define RDGATE_MODE 0x0
#define RDGATE_LEN  0x4
#define RDGATE_CTRL 0x88

#define MC1_RDGATE_MODE 0x0
#define MC1_RDGATE_LEN  0x2
#define MC1_RDGATE_CTRL 0x00


#define MC0_DLL_WDQ0 0x00
#define MC0_DLL_WDQ1 0x00
#define MC0_DLL_WDQ2 0x00
#define MC0_DLL_WDQ3 0x00
#define MC0_DLL_WDQ4 0x00
#define MC0_DLL_WDQ5 0x00
#define MC0_DLL_WDQ6 0x00
#define MC0_DLL_WDQ7 0x00
#define MC0_DLL_WDQ8 0x00

#define MC0_DLL_GATE0 0x00
#define MC0_DLL_GATE1 0x00
#define MC0_DLL_GATE2 0x00
#define MC0_DLL_GATE3 0x00
#define MC0_DLL_GATE4 0x00
#define MC0_DLL_GATE5 0x00
#define MC0_DLL_GATE6 0x00
#define MC0_DLL_GATE7 0x00
#define MC0_DLL_GATE8 0x00


#define MC1_DLL_WDQ0 0x00
#define MC1_DLL_WDQ1 0x00
#define MC1_DLL_WDQ2 0x00
#define MC1_DLL_WDQ3 0x00
#define MC1_DLL_WDQ4 0x00
#define MC1_DLL_WDQ5 0x00
#define MC1_DLL_WDQ6 0x00
#define MC1_DLL_WDQ7 0x00
#define MC1_DLL_WDQ8 0x00

#define MC1_DLL_GATE0 0x00
#define MC1_DLL_GATE1 0x00
#define MC1_DLL_GATE2 0x00
#define MC1_DLL_GATE3 0x00
#define MC1_DLL_GATE4 0x00
#define MC1_DLL_GATE5 0x00
#define MC1_DLL_GATE6 0x00
#define MC1_DLL_GATE7 0x00
#define MC1_DLL_GATE8 0x00

#define MC0_DLY_2X0 0x00
#define MC0_DLY_2X1 0x00
#define MC0_DLY_2X2 0x00
#define MC0_DLY_2X3 0x00
#define MC0_DLY_2X4 0x00
#define MC0_DLY_2X5 0x00
#define MC0_DLY_2X6 0x00
#define MC0_DLY_2X7 0x00
#define MC0_DLY_2X8 0x00

#define MC1_DLY_2X0 0x00
#define MC1_DLY_2X1 0x00
#define MC1_DLY_2X2 0x00
#define MC1_DLY_2X3 0x00
#define MC1_DLY_2X4 0x00
#define MC1_DLY_2X5 0x00
#define MC1_DLY_2X6 0x00
#define MC1_DLY_2X7 0x00
#define MC1_DLY_2X8 0x00
#define RD_DQS      0x1c1c

#define DQ_OE          0x80
//#define DQ_OE          0x00
//#define VREF 0x0081

#define RDDQS_DLY     0x19
#define MC1_RDDQS_DLY 0x19

.rdata
.align 5
ddr3_reg_data:
ddr3_reg_data_mc1:
ddr3_RDIMM_reg_data:
ddr3_RDIMM_reg_data_mc1:

ddr4_reg_data:
ddr4_RDIMM_reg_data:
 MC0_PHY_000_DATA : .dword 0x0000000000000010  #useless
 MC0_PHY_008_DATA : .dword 0x0000000000000020
 MC0_PHY_010_DATA : .dword 0x0000000000000000
 MC0_PHY_018_DATA : .dword 0x0000000000000000  #useless
 MC0_PHY_020_DATA : .dword 0x0000000000000001
 MC0_PHY_028_DATA : .dword 0x0000000000000000
 MC0_PHY_030_DATA : .dword 0x0058010100040510
 MC0_PHY_038_DATA : .dword 0x0000000000000144
 MC0_PHY_040_DATA : .dword DOUB_CFG
 MC0_PHY_048_DATA : .dword (PLL_CFG<<32)|SEL_DBL
 MC0_PHY_050_DATA : .dword DOUB_CFG   
 MC0_PHY_058_DATA : .dword (PLL_CFG<<32)|SEL_DBL
 MC0_PHY_060_DATA : .dword DOUB_CFG
 MC0_PHY_068_DATA : .dword (PLL_CFG<<32)|SEL_DBL
 MC0_PHY_070_DATA : .dword DOUB_CFG
 MC0_PHY_078_DATA : .dword (PLL_CFG<<32)|SEL_DBL
 MC0_PHY_080_DATA : .dword DOUB_CFG
 MC0_PHY_088_DATA : .dword (PLL_CFG<<32)|SEL_DBL
 MC0_PHY_090_DATA : .dword DOUB_CFG
 MC0_PHY_098_DATA : .dword (PLL_CFG<<32)|SEL_DBL
 MC0_PHY_0A0_DATA : .dword DOUB_CFG
 MC0_PHY_0A8_DATA : .dword (PLL_CFG<<32)|SEL_DBL
 MC0_PHY_0B0_DATA : .dword DOUB_CFG
 MC0_PHY_0B8_DATA : .dword (PLL_CFG<<32)|SEL_DBL
 MC0_PHY_0C0_DATA : .dword DOUB_CFG
 MC0_PHY_0C8_DATA : .dword (PLL_CFG<<32)|SEL_DBL
 MC0_PHY_0D0_DATA : .dword DOUB_CFG
 MC0_PHY_0D8_DATA : .dword (PLL_CFG<<32)|SEL_DBL
 MC0_PHY_0E0_DATA : .dword 0x0000000000000f00
 MC0_PHY_0E8_DATA : .dword 0x0000000000000000  #useless
 MC0_PHY_0F0_DATA : .dword 0x0000000000000000  #useless
 MC0_PHY_0F8_DATA : .dword 0x0000000000000000  #useless
 MC0_PHY_100_DATA : .dword 0x0000000000002000|(MC0_DLL_WDQ0<<24)|(MC0_DLL_WDQ0)
 MC0_PHY_108_DATA : .dword 0x0000000000000000|(MC0_DLL_GATE0<<16)|(RDDQS_DLY<<8)|RDDQS_DLY
 MC0_PHY_110_DATA : .dword 0x0000000000000000|(RDODT_CTRL<<56)|(RDGATE_LEN<<48)|(RDGATE_MODE<<40)|(RDGATE_CTRL<<32)|DQ_OE
 MC0_PHY_118_DATA : .dword 0x0000000000000000|(MC0_DLY_2X0<<16)
 MC0_PHY_120_DATA : .dword 0x0000000000000000
 MC0_PHY_128_DATA : .dword 0x0000000000000000
 MC0_PHY_130_DATA : .dword 0x0000000000000000
 MC0_PHY_138_DATA : .dword 0x0000000000000000
 MC0_PHY_140_DATA : .dword 0x0000000000000000
 MC0_PHY_148_DATA : .dword 0x0000000000000000  #useless
 MC0_PHY_150_DATA : .dword 0x0000000000000000
 MC0_PHY_158_DATA : .dword 0x0000000000000000
 MC0_PHY_160_DATA : .dword 0x0000000000000000
 MC0_PHY_168_DATA : .dword 0x0000000000000000
 MC0_PHY_170_DATA : .dword 0x0000000000000000
 MC0_PHY_178_DATA : .dword 0x0000000000000000
 MC0_PHY_180_DATA : .dword 0x0000000000002000|(MC0_DLL_WDQ1<<24)|(MC0_DLL_WDQ1)
 MC0_PHY_188_DATA : .dword 0x0000000000000000|(MC0_DLL_GATE1<<16)|(RDDQS_DLY<<8)|RDDQS_DLY
 MC0_PHY_190_DATA : .dword 0x0000000000000000|(RDODT_CTRL<<56)|(RDGATE_LEN<<48)|(RDGATE_MODE<<40)|(RDGATE_CTRL<<32)|DQ_OE
 MC0_PHY_198_DATA : .dword 0x0000000000000000|(MC0_DLY_2X1<<16)
 MC0_PHY_1A0_DATA : .dword 0x0000000000000000
 MC0_PHY_1A8_DATA : .dword 0x0000000000000000
 MC0_PHY_1B0_DATA : .dword 0x0000000000000000
 MC0_PHY_1B8_DATA : .dword 0x0000000000000000
 MC0_PHY_1C0_DATA : .dword 0x0000000000000000
 MC0_PHY_1C8_DATA : .dword 0x0000000000000000  #useless
 MC0_PHY_1D0_DATA : .dword 0x0000000000000000
 MC0_PHY_1D8_DATA : .dword 0x0000000000000000
 MC0_PHY_1E0_DATA : .dword 0x0000000000000000
 MC0_PHY_1E8_DATA : .dword 0x0000000000000000
 MC0_PHY_1F0_DATA : .dword 0x0000000000000000
 MC0_PHY_1F8_DATA : .dword 0x0000000000000000
 MC0_PHY_200_DATA : .dword 0x0000000000002000|(MC0_DLL_WDQ2<<24)|(MC0_DLL_WDQ2)
 MC0_PHY_208_DATA : .dword 0x0000000000000000|(MC0_DLL_GATE2<<16)|(RDDQS_DLY<<8)|RDDQS_DLY
 MC0_PHY_210_DATA : .dword 0x0000000000000000|(RDODT_CTRL<<56)|(RDGATE_LEN<<48)|(RDGATE_MODE<<40)|(RDGATE_CTRL<<32)|DQ_OE
 MC0_PHY_218_DATA : .dword 0x0000000000000000|(MC0_DLY_2X2<<16)
 MC0_PHY_220_DATA : .dword 0x0000000000000000
 MC0_PHY_228_DATA : .dword 0x0000000000000000
 MC0_PHY_230_DATA : .dword 0x0000000000000000
 MC0_PHY_238_DATA : .dword 0x0000000000000000
 MC0_PHY_240_DATA : .dword 0x0000000000000000
 MC0_PHY_248_DATA : .dword 0x0000000000000000  #useless
 MC0_PHY_250_DATA : .dword 0x0000000000000000
 MC0_PHY_258_DATA : .dword 0x0000000000000000
 MC0_PHY_260_DATA : .dword 0x0000000000000000
 MC0_PHY_268_DATA : .dword 0x0000000000000000
 MC0_PHY_270_DATA : .dword 0x0000000000000000
 MC0_PHY_278_DATA : .dword 0x0000000000000000
 MC0_PHY_280_DATA : .dword 0x0000000000002000|(MC0_DLL_WDQ3<<24)|(MC0_DLL_WDQ3)
 MC0_PHY_288_DATA : .dword 0x0000000000000000|(MC0_DLL_GATE3<<16)|(RDDQS_DLY<<8)|RDDQS_DLY
 MC0_PHY_290_DATA : .dword 0x0000000000000000|(RDODT_CTRL<<56)|(RDGATE_LEN<<48)|(RDGATE_MODE<<40)|(RDGATE_CTRL<<32)|DQ_OE
 MC0_PHY_298_DATA : .dword 0x0000000000000000|(MC0_DLY_2X3<<16)
 MC0_PHY_2A0_DATA : .dword 0x0000000000000000
 MC0_PHY_2A8_DATA : .dword 0x0000000000000000
 MC0_PHY_2B0_DATA : .dword 0x0000000000000000
 MC0_PHY_2B8_DATA : .dword 0x0000000000000000
 MC0_PHY_2C0_DATA : .dword 0x0000000000000000
 MC0_PHY_2C8_DATA : .dword 0x0000000000000000  #useless
 MC0_PHY_2D0_DATA : .dword 0x0000000000000000
 MC0_PHY_2D8_DATA : .dword 0x0000000000000000
 MC0_PHY_2E0_DATA : .dword 0x0000000000000000
 MC0_PHY_2E8_DATA : .dword 0x0000000000000000
 MC0_PHY_2F0_DATA : .dword 0x0000000000000000
 MC0_PHY_2F8_DATA : .dword 0x0000000000000000
 MC0_PHY_300_DATA : .dword 0x0000000000002000|(MC0_DLL_WDQ4<<24)|(MC0_DLL_WDQ4)
 MC0_PHY_308_DATA : .dword 0x0000000000000000|(MC0_DLL_GATE4<<16)|(RDDQS_DLY<<8)|RDDQS_DLY
 MC0_PHY_310_DATA : .dword 0x0000000000000000|(RDODT_CTRL<<56)|(RDGATE_LEN<<48)|(RDGATE_MODE<<40)|(RDGATE_CTRL<<32)|DQ_OE
 MC0_PHY_318_DATA : .dword 0x0000000000000000|(MC0_DLY_2X4<<16)
 MC0_PHY_320_DATA : .dword 0x0000000000000000
 MC0_PHY_328_DATA : .dword 0x0000000000000000
 MC0_PHY_330_DATA : .dword 0x0000000000000000
 MC0_PHY_338_DATA : .dword 0x0000000000000000
 MC0_PHY_340_DATA : .dword 0x0000000000000000
 MC0_PHY_348_DATA : .dword 0x0000000000000000  #useless
 MC0_PHY_350_DATA : .dword 0x0000000000000000
 MC0_PHY_358_DATA : .dword 0x0000000000000000
 MC0_PHY_360_DATA : .dword 0x0000000000000000
 MC0_PHY_368_DATA : .dword 0x0000000000000000
 MC0_PHY_370_DATA : .dword 0x0000000000000000
 MC0_PHY_378_DATA : .dword 0x0000000000000000
 MC0_PHY_380_DATA : .dword 0x0000000000002000|(MC0_DLL_WDQ5<<24)|(MC0_DLL_WDQ5)
 MC0_PHY_388_DATA : .dword 0x0000000000000000|(MC0_DLL_GATE5<<16)|(RDDQS_DLY<<8)|RDDQS_DLY
 MC0_PHY_390_DATA : .dword 0x0000000000000000|(RDODT_CTRL<<56)|(RDGATE_LEN<<48)|(RDGATE_MODE<<40)|(RDGATE_CTRL<<32)|DQ_OE
 MC0_PHY_398_DATA : .dword 0x0000000000000000|(MC0_DLY_2X5<<16)
 MC0_PHY_3A0_DATA : .dword 0x0000000000000000
 MC0_PHY_3A8_DATA : .dword 0x0000000000000000
 MC0_PHY_3B0_DATA : .dword 0x0000000000000000
 MC0_PHY_3B8_DATA : .dword 0x0000000000000000
 MC0_PHY_3C0_DATA : .dword 0x0000000000000000
 MC0_PHY_3C8_DATA : .dword 0x0000000000000000  #useless
 MC0_PHY_3D0_DATA : .dword 0x0000000000000000
 MC0_PHY_3D8_DATA : .dword 0x0000000000000000
 MC0_PHY_3E0_DATA : .dword 0x0000000000000000
 MC0_PHY_3E8_DATA : .dword 0x0000000000000000
 MC0_PHY_3F0_DATA : .dword 0x0000000000000000
 MC0_PHY_3F8_DATA : .dword 0x0000000000000000
 MC0_PHY_400_DATA : .dword 0x0000000000002000|(MC0_DLL_WDQ6<<24)|(MC0_DLL_WDQ6)
 MC0_PHY_408_DATA : .dword 0x0000000000000000|(MC0_DLL_GATE6<<16)|(RDDQS_DLY<<8)|RDDQS_DLY
 MC0_PHY_410_DATA : .dword 0x0000000000000000|(RDODT_CTRL<<56)|(RDGATE_LEN<<48)|(RDGATE_MODE<<40)|(RDGATE_CTRL<<32)|DQ_OE
 MC0_PHY_418_DATA : .dword 0x0000000000000000|(MC0_DLY_2X6<<16)
 MC0_PHY_420_DATA : .dword 0x0000000000000000
 MC0_PHY_428_DATA : .dword 0x0000000000000000
 MC0_PHY_430_DATA : .dword 0x0000000000000000
 MC0_PHY_438_DATA : .dword 0x0000000000000000
 MC0_PHY_440_DATA : .dword 0x0000000000000000
 MC0_PHY_448_DATA : .dword 0x0000000000000000  #useless
 MC0_PHY_450_DATA : .dword 0x0000000000000000
 MC0_PHY_458_DATA : .dword 0x0000000000000000
 MC0_PHY_460_DATA : .dword 0x0000000000000000
 MC0_PHY_468_DATA : .dword 0x0000000000000000
 MC0_PHY_470_DATA : .dword 0x0000000000000000
 MC0_PHY_478_DATA : .dword 0x0000000000000000
 MC0_PHY_480_DATA : .dword 0x0000000000002000|(MC0_DLL_WDQ7<<24)|(MC0_DLL_WDQ7)
 MC0_PHY_488_DATA : .dword 0x0000000000000000|(MC0_DLL_GATE7<<16)|(RDDQS_DLY<<8)|RDDQS_DLY
 MC0_PHY_490_DATA : .dword 0x0000000000000000|(RDODT_CTRL<<56)|(RDGATE_LEN<<48)|(RDGATE_MODE<<40)|(RDGATE_CTRL<<32)|DQ_OE
 MC0_PHY_498_DATA : .dword 0x0000000000000000|(MC0_DLY_2X7<<16)
 MC0_PHY_4A0_DATA : .dword 0x0000000000000000
 MC0_PHY_4A8_DATA : .dword 0x0000000000000000
 MC0_PHY_4B0_DATA : .dword 0x0000000000000000
 MC0_PHY_4B8_DATA : .dword 0x0000000000000000
 MC0_PHY_4C0_DATA : .dword 0x0000000000000000
 MC0_PHY_4C8_DATA : .dword 0x0000000000000000  #useless
 MC0_PHY_4D0_DATA : .dword 0x0000000000000000
 MC0_PHY_4D8_DATA : .dword 0x0000000000000000
 MC0_PHY_4E0_DATA : .dword 0x0000000000000000
 MC0_PHY_4E8_DATA : .dword 0x0000000000000000
 MC0_PHY_4F0_DATA : .dword 0x0000000000000000
 MC0_PHY_4F8_DATA : .dword 0x0000000000000000
 MC0_PHY_500_DATA : .dword 0x0000000000002000|(MC0_DLL_WDQ8<<24)|(MC0_DLL_WDQ8)
 MC0_PHY_508_DATA : .dword 0x0000000000000000|(MC0_DLL_GATE8<<16)|(RDDQS_DLY<<8)|RDDQS_DLY
 MC0_PHY_510_DATA : .dword 0x0000000000000000|(RDODT_CTRL<<56)|(RDGATE_LEN<<48)|(RDGATE_MODE<<40)|(RDGATE_CTRL<<32)|DQ_OE
 MC0_PHY_518_DATA : .dword 0x0000000000000000|(MC0_DLY_2X8<<16)
 MC0_PHY_520_DATA : .dword 0x0000000000000000
 MC0_PHY_528_DATA : .dword 0x0000000000000000
 MC0_PHY_530_DATA : .dword 0x0000000000000000
 MC0_PHY_538_DATA : .dword 0x0000000000000000
 MC0_PHY_540_DATA : .dword 0x0000000000000000
 MC0_PHY_548_DATA : .dword 0x0000000000000000  #useless
 MC0_PHY_550_DATA : .dword 0x0000000000000000
 MC0_PHY_558_DATA : .dword 0x0000000000000000
 MC0_PHY_560_DATA : .dword 0x0000000000000000
 MC0_PHY_568_DATA : .dword 0x0000000000000000
 MC0_PHY_570_DATA : .dword 0x0000000000000000
 MC0_PHY_578_DATA : .dword 0x0000000000000000
                    .dword 0x0:48
 MC0_PHY_700_DATA : .dword 0x0000000001300000
                    .dword 0x0:31
#define PAD_DS_OCD_DQS 0x7//drive strength:max7
#define PAD_DS_OCD_DQ  0x7
#define PAD_DS_COMP    0x1
#define PAD_DS_DCC     0x8
#define PAD_DS_ENZI    0x0//0x1
//#define PAD_DS_ODT_DQS 0x4//odt value:max7
//#define PAD_DS_ODT_DQ  0x4
#define PAD_DS_ODT_DQS 0x6//odt value:max7
#define PAD_DS_ODT_DQ  0x4
#define PAD_DS_CODEIN  0x1//0:parameter outside
#define PAD_DS_CTRL    ((PAD_DS_CODEIN<<18)|(PAD_DS_ODT_DQ<<15)|(PAD_DS_ODT_DQS<<12)|(PAD_DS_ENZI<<11)|(PAD_DS_DCC<<7)|(PAD_DS_COMP<<6)|(PAD_DS_OCD_DQ<<3)|(PAD_DS_OCD_DQS))
#define PAD_DS_SPLIT ((PAD_DS_DCC<<12)|(PAD_DS_ODT_DQ<<9)|(PAD_DS_ODT_DQS<<6)|(PAD_DS_OCD_DQ<<3)|(PAD_DS_OCD_DQS))
 MC0_PHY_800_DATA : .dword 0x050000000026d2df|(PAD_DS_CTRL<<32)
 MC0_PHY_808_DATA : .dword 0x000201010003ed79
 MC0_PHY_810_DATA : .dword 0x0000000000000000|(VREF<<48)|(VREF<<32)|(VREF<<16)|(VREF<<0)
 MC0_PHY_818_DATA : .dword 0x0000000000000000|(VREF<<48)|(VREF<<32)|(VREF<<16)|(VREF<<0)
 MC0_PHY_820_DATA : .dword 0x0000000000000000|(VREF<<0)
 MC0_PHY_828_DATA : .dword 0x0000000000000000  #useless
 MC0_PHY_830_DATA : .dword COMP_CTRL
 MC0_PHY_838_DATA : .dword 0x0000000000000000
 MC0_PHY_840_DATA : .dword (PAD_DS_SPLIT<<48)|(PAD_DS_SPLIT<<32)|(PAD_DS_SPLIT<<16)|(PAD_DS_SPLIT)
 MC0_PHY_848_DATA : .dword (PAD_DS_SPLIT<<48)|(PAD_DS_SPLIT<<32)|(PAD_DS_SPLIT<<16)|(PAD_DS_SPLIT)
 MC0_PHY_850_DATA : .dword (PAD_DS_SPLIT)
 MC0_PHY_858_DATA : .dword 0x0000000000000000
 MC0_PHY_860_DATA : .dword 0x0000000000000000
 MC0_PHY_868_DATA : .dword 0x0000000000000000
 MC0_PHY_870_DATA : .dword 0x0000000000000000
 MC0_PHY_878_DATA : .dword 0x0000000000000000
 MC0_PHY_880_DATA : .dword 0x0000000000000000
 MC0_PHY_888_DATA : .dword 0x0000000000000000
 MC0_PHY_890_DATA : .dword 0x0000000000000000#
 MC0_PHY_898_DATA : .dword 0x0000000000000000#
 MC0_PHY_8a0_DATA : .dword 0x0000000000000000#
 MC0_PHY_8a8_DATA : .dword 0x0000000000000000#
 MC0_PHY_8b0_DATA : .dword 0x0000000000000000#
 MC0_PHY_8b8_DATA : .dword 0x0000000000000000#
 MC0_PHY_8c0_DATA : .dword 0x0000000000000000#
 MC0_PHY_8c8_DATA : .dword 0x0000000000000000#
 MC0_PHY_8d0_DATA : .dword 0x0000000000000000#
 MC0_PHY_8d8_DATA : .dword 0x0000000000000000#
 MC0_PHY_8e0_DATA : .dword 0x0000000000000000#
 MC0_PHY_8e8_DATA : .dword 0x0000000000000000#
 MC0_PHY_8f0_DATA : .dword 0x0000000000000000#
 MC0_PHY_8f8_DATA : .dword 0x0000000000000000#
 MC0_PHY_900_DATA : .dword 0x0000000000000000#
 MC0_PHY_908_DATA : .dword 0x0000000000000000#
                    .dword 0x0:222
//#define DDR_FREQ 275
#define T2C(x)          (DDR_FREQ*(x)/500)
//#define MAX(x,y)        #if((x)>(y)) (x) #else (y) #endif
#define tRP             (T2C(15) & 0xff)
#define tWLDQSEN        (0x19 & 0xff)
#define tMOD            (0x18 & 0xff)
#define tRFC            (T2C(350) & 0xfff)
#define tXS             ((T2C(10)+tRFC) & 0xfff)
//#define tXPR            (MAX(tXS,5) & 0xfff)
#define tXPR            (5 & 0xfff)
#define tCKE            (0x4f & 0xff) //TODO not found
#define tRESET          (0xc4 & 0xff) //TODO not found
#define tODTL           (0xa & 0xff) //TODO not found
#define tREF            ((T2C(7800)/16) & 0xfff)
#define tREFretention   (0x10000 & 0xffffffff) //TODO not found
#define tREF_IDLE       (0xf & 0xff)
#define tRFC_dlr        (0x0 & 0xfff)
#define tXSRD           (0x3 & 0xff) //TODO not defined
#define tCKESR          (4 & 0xff)
#define tXP             (4 & 0xff)
#define tXPDLL          (0x14 & 0xff) //TODO not found
#define tCPDED          (0x4 & 0xff)
#define tZQ_CMD         (0x4 & 0xff)  //not found    
#define tZQCS           (0x80 & 0xff) //not found
#define tZQCL           (0x4 & 0xff)  //not found
#define tZQperiod       (0x20 & 0xff) //not found
#define tRAS_min        (T2C(35) & 0x3f)
#define tRRD            (4 & 0xf)
#define tRRD_L_slr      (6 & 0xf)
#define tRRD_S_slr      (4 & 0xf)
#define tRCD            (T2C(14) & 0x1f)
#define tFAW            (T2C(35) & 0x3f)
#define tFAW_slr        (T2C(70) & 0x3f)
#define tWR             (T2C(24) & 0x1f)
#define tWR_CRC_DM      (T2C(60) & 0x1f)
#define tRTP            (5 & 0xf)
#define tCCD            (0x6 & 0xf) //Can NOT be 0x5 when using 3A4000A
#define tCCD_S          (0x4 & 0xf)
#define tWTR            (10 & 0xf)
#define tWTR_S          (4 & 0xf)
#define tWTR_L_CRC_DM   ((5 + tWTR) & 0xf)
#define tWTR_S_CRC_DM   ((5 + tWTR_S) & 0xf)
#define tRL             (0xb & 0x1f)
#define tRDPDEN         ((tRL+4+1) & 0xff)
#define tPHY_RDLAT      ((tRL + 7) & 0x3f)

#define CA_TIMING           (0x0 & 0x1)
// CTL_448_DATA : .dword CA_TIMING

#define tRDDATA         ((tRL - 4 + CA_TIMING) & 0x3f)
#define tWL             (0xa & 0x1f)
#define tPHY_WRLAT      ((tWL - 6 + CA_TIMING) & 0x1f)
#define tPL             (0x0 & 0xf)
#define tCAL            (0x0 & 0x5)
#define tR2R_sameba_adj (0x0 & 0x3f)
#define tR2W_sameba_adj (0x0 & 0x3f)
#define tR2P_sameba_adj (0x0 & 0x3f)
#define tW2R_sameba_adj (0x0 & 0x3f)
#define tW2W_sameba_adj (0x0 & 0x3f)
#define tW2P_sameba_adj (0x0 & 0x3f)
#define tR2R_samebg_adj (0x2 & 0x3f)
#define tR2W_samebg_adj (0x4 & 0x3f)
#define tR2P_samebg_adj (0x0 & 0x3f)
#define tW2R_samebg_adj (0x0 & 0x3f)
#define tW2W_samebg_adj (0x0 & 0x3f)
#define tW2P_samebg_adj (0x0 & 0x3f)
#define tR2R_samec_adj  (0x2 & 0x3f)
#define tR2W_samec_adj  (0x4 & 0x3f)
#define tR2P_samec_adj  (0x0 & 0x3f)
#define tW2R_samec_adj  (0x0 & 0x3f)
#define tW2W_samec_adj  (0x0 & 0x3f)
#define tW2P_samec_adj  (0x0 & 0x3f)
#define tR2R_samecs_adj (0x0 & 0x3f)
#define tR2W_samecs_adj (0x0 & 0x3f)
#define tR2P_samecs_adj (0x0 & 0x3f)
#define tW2R_samecs_adj (0x0 & 0x3f)
#define tW2W_samecs_adj (0x0 & 0x3f)
#define tW2P_samecs_adj (0x0 & 0x3f)
#define tR2R_diffcs_adj (0x4 & 0x3f)
#define tR2W_diffcs_adj (0x4 & 0x3f)
#define tR2P_diffcs_adj (0x0 & 0x3f)
#define tW2R_diffcs_adj (0x4 & 0x3f)
#define tW2W_diffcs_adj (0x4 & 0x3f)
#define tW2P_diffcs_adj (0x0 & 0x3f)

 MC0_CTL_000_DATA : .dword (tRP<<48)|(tWLDQSEN<<40)|(tMOD<<32)|(tXPR<<16)|(tCKE<<8)|(tRESET)  

 MC0_CTL_008_DATA : .dword (tODTL)
                                         
 MC0_CTL_010_DATA : .dword (tREFretention<<32)|(tRFC<<16)|(tREF)

 MC0_CTL_018_DATA : .dword (tCKESR<<56)|(tXSRD<<48)|(tXS<<32)|(tRFC_dlr<<16)|(tREF_IDLE)
                                         
 MC0_CTL_020_DATA : .dword (tRDPDEN<<24)|(tCPDED<<16)|(tXPDLL<<8)|(tXP)

 MC0_CTL_028_DATA : .dword (tZQperiod<<24)|(tZQCL<<16)|(tZQCS<<8)|(tZQ_CMD)
                                         
 MC0_CTL_030_DATA : .dword 0x0000000000000000  #useless

 MC0_CTL_038_DATA : .dword 0x0000000000000000  #useless

 MC0_CTL_040_DATA : .dword (tRCD<<56)|(tRRD_S_slr<<48)|(tRRD_L_slr<<40)|(tRRD<<32)|(tRAS_min)

 MC0_CTL_048_DATA : .dword (tRTP<<32)|(tWR_CRC_DM<<24)|(tWR<<16)|(tFAW_slr<<8)|(tFAW)
                                         
 MC0_CTL_050_DATA : .dword (tWTR_S_CRC_DM<<56)|(tWTR_L_CRC_DM<<48)|(tWTR_S<<40)|(tWTR<<32)|(tCCD<<16)|(tCCD_S<<8)|(tCCD)
 MC0_CTL_058_DATA : .dword 0x0000000000000000  #useless
                                         
 //MC0_CTL_060_DATA : .dword (tPHY_WRLAT<<40)|(tWL<<32)|(tRDDATA<<16)|(tPHY_RDLAT<<8)|(tRL)
 MC0_CTL_060_DATA : .dword 0x00000408000b120b //UDIMM/RDIMM

 MC0_CTL_068_DATA : .dword (tCAL<<32)|(tPL)
                                         
 MC0_CTL_070_DATA : .dword (tW2P_sameba_adj<<40)|(tW2W_sameba_adj<<32)|(tW2R_sameba_adj<<24)|(tR2P_sameba_adj<<16)|(tR2W_sameba_adj<<8)|(tR2R_sameba_adj)

 MC0_CTL_078_DATA : .dword (tW2P_samebg_adj<<40)|(tW2W_samebg_adj<<32)|(tW2R_samebg_adj<<24)|(tR2P_samebg_adj<<16)|(tR2W_samebg_adj<<8)|(tR2R_samebg_adj)
                                         
 MC0_CTL_080_DATA : .dword (tW2P_samec_adj<<40)|(tW2W_samec_adj<<32)|(tW2R_samec_adj<<24)|(tR2P_samec_adj<<16)|(tR2W_samec_adj<<8)|(tR2R_samec_adj)

 MC0_CTL_088_DATA : .dword 0x0f0f0f0f0f0f0f0f  #useless
                                         
 MC0_CTL_090_DATA : .dword (tW2P_samecs_adj<<40)|(tW2W_samecs_adj<<32)|(tW2R_samecs_adj<<24)|(tR2P_samecs_adj<<16)|(tR2W_samecs_adj<<8)|(tR2R_samecs_adj)

 MC0_CTL_098_DATA : .dword (tW2P_diffcs_adj<<40)|(tW2W_diffcs_adj<<32)|(tW2R_diffcs_adj<<24)|(tR2P_diffcs_adj<<16)|(tR2W_diffcs_adj<<8)|(tR2R_diffcs_adj)
                    .dword 0x0:12
#define CS_ENABLE   (0x1 & 0xff)
#define CS_MRS      (0x1 & 0xff)
#define CS_ZQ       (0x1 & 0xff)
#define CS_ZQCL     (0x1 & 0xff)
#define CS_RESYNC   (0x1 & 0xff)
 MC0_CTL_100_DATA : .dword (0x1<<40)|(CS_RESYNC<<32)|(CS_ZQCL<<24)|(CS_ZQ<<16)|(CS_MRS<<8)|(CS_ENABLE)


#define CS_MAP      (0x76503214 & 0xffffffff)
#define CKE_MAP     (0x84218124 & 0xffffffff)
 MC0_CTL_108_DATA : .dword (CKE_MAP<<32)|(CS_MAP)

#define CID_MAP      (0x0 & 0xff)
#define CS2CID       (0x0 & 0xff)
 MC0_CTL_110_DATA : .dword (CS2CID<<32)|(CID_MAP)
                                         
 MC0_CTL_118_DATA : .dword 0x0000000000000000  #useless

#define COMMAND_MODE       (0x0 & 0x1)
#define CMD_REQ            (0x0 & 0x1)
#define CMD_CMD            (0x0 & 0xf)
#define PRE_ALL_REQ        (0x0 & 0x1)
#define MRS_REQ            (0x0 & 0x1)
 MC0_CTL_120_DATA : .dword (MRS_REQ<<48)|(PRE_ALL_REQ<<32)|(CMD_CMD<<24)|(CMD_REQ<<8)|(COMMAND_MODE)
#RO: STATUS_CMD
#RO: PRE_ALL_DONE
#RO: MRS_DONE

#define CMD_CS       (0x0 & 0xff)
#define CMD_C        (0x0 & 0x7)
#define CMD_BG       (0x0 & 0x3)
#define CMD_BA       (0x0 & 0x7)
#define CMD_A        (0x0 & 0x3ffff)
 MC0_CTL_128_DATA : .dword (CMD_A<<32)|(CMD_BA<<24)|(CMD_BG<<16)|(CMD_C<<8)|(CMD_CS)

#define CMD_PDA       (0x0 & 0x1)
 MC0_CTL_130_DATA : .dword CMD_PDA

#define CMD_DQ0       (0x0 & 0x3ffff)
 MC0_CTL_138_DATA : .dword CMD_DQ0

#define MR_0_CS_0     (0x0d60 & 0xffff)
#define MR_1_CS_0     (0x0006 & 0xffff)
#define MR_2_CS_0     (0x0018 & 0xffff)
#define MR_3_CS_0     (0x0000 & 0xffff)
 MC0_CTL_140_DATA : .dword (MR_3_CS_0<<48)|(MR_2_CS_0<<32)|(MR_1_CS_0<<16)|(MR_0_CS_0)

#define MR_0_CS_1     (0x0d60 & 0xffff)
#define MR_1_CS_1     (0x0004 & 0xffff)
#define MR_2_CS_1     (0x0018 & 0xffff)
#define MR_3_CS_1     (0x0000 & 0xffff)
 MC0_CTL_148_DATA : .dword (MR_3_CS_1<<48)|(MR_2_CS_1<<32)|(MR_1_CS_1<<16)|(MR_0_CS_1)

#define MR_0_CS_2     (0x0d60 & 0xffff)
#define MR_1_CS_2     (0x0004 & 0xffff)
#define MR_2_CS_2     (0x0018 & 0xffff)
#define MR_3_CS_2     (0x0000 & 0xffff)
 MC0_CTL_150_DATA : .dword (MR_3_CS_2<<48)|(MR_2_CS_2<<32)|(MR_1_CS_2<<16)|(MR_0_CS_2)

#define MR_0_CS_3     (0x0d60 & 0xffff)
#define MR_1_CS_3     (0x0004 & 0xffff)
#define MR_2_CS_3     (0x0018 & 0xffff)
#define MR_3_CS_3     (0x0000 & 0xffff)
 MC0_CTL_158_DATA : .dword (MR_3_CS_3<<48)|(MR_2_CS_3<<32)|(MR_1_CS_3<<16)|(MR_0_CS_3)

#define MR_0_CS_4     (0x0d60 & 0xffff)
#define MR_1_CS_4     (0x0004 & 0xffff)
#define MR_2_CS_4     (0x0018 & 0xffff)
#define MR_3_CS_4     (0x0000 & 0xffff)
 MC0_CTL_160_DATA : .dword (MR_3_CS_4<<48)|(MR_2_CS_4<<32)|(MR_1_CS_4<<16)|(MR_0_CS_4)

#define MR_0_CS_5     (0x0d60 & 0xffff)
#define MR_1_CS_5     (0x0004 & 0xffff)
#define MR_2_CS_5     (0x0018 & 0xffff)
#define MR_3_CS_5     (0x0000 & 0xffff)
 MC0_CTL_168_DATA : .dword (MR_3_CS_5<<48)|(MR_2_CS_5<<32)|(MR_1_CS_5<<16)|(MR_0_CS_5)

#define MR_0_CS_6     (0x0d60 & 0xffff)
#define MR_1_CS_6     (0x0004 & 0xffff)
#define MR_2_CS_6     (0x0018 & 0xffff)
#define MR_3_CS_6     (0x0000 & 0xffff)
 MC0_CTL_170_DATA : .dword (MR_3_CS_6<<48)|(MR_2_CS_6<<32)|(MR_1_CS_6<<16)|(MR_0_CS_6)

#define MR_0_CS_7     (0x0d60 & 0xffff)
#define MR_1_CS_7     (0x0004 & 0xffff)
#define MR_2_CS_7     (0x0018 & 0xffff)
#define MR_3_CS_7     (0x0000 & 0xffff)
 MC0_CTL_178_DATA : .dword (MR_3_CS_7<<48)|(MR_2_CS_7<<32)|(MR_1_CS_7<<16)|(MR_0_CS_7)

#define MR_0_CS_0_DDR4     (0x0114 & 0xffff)
#define MR_1_CS_0_DDR4     (0x0501 & 0xffff)
#define MR_2_CS_0_DDR4     (0x0018 & 0xffff)
#define MR_3_CS_0_DDR4     (0x0200 & 0xffff)
 MC0_CTL_180_DATA : .dword (MR_3_CS_0_DDR4<<48)|(MR_2_CS_0_DDR4<<32)|(MR_1_CS_0_DDR4<<16)|(MR_0_CS_0_DDR4)

#define MR_4_CS_0_DDR4     (0x0000 & 0xffff)
#define MR_5_CS_0_DDR4     (0x0480 & 0xffff)
#define MR_6_CS_0_DDR4     (0x0400 & 0xffff)
 MC0_CTL_188_DATA : .dword (MR_6_CS_0_DDR4<<32)|(MR_5_CS_0_DDR4<<16)|(MR_4_CS_0_DDR4)

#define MR_0_CS_1_DDR4     (0x0104 & 0xffff)
#define MR_1_CS_1_DDR4     (0x0501 & 0xffff)
#define MR_2_CS_1_DDR4     (0x0000 & 0xffff)
#define MR_3_CS_1_DDR4     (0x0200 & 0xffff)
 MC0_CTL_190_DATA : .dword (MR_3_CS_1_DDR4<<48)|(MR_2_CS_1_DDR4<<32)|(MR_1_CS_1_DDR4<<16)|(MR_0_CS_1_DDR4)

#define MR_4_CS_1_DDR4     (0x0000 & 0xffff)
#define MR_5_CS_1_DDR4     (0x0480 & 0xffff)
#define MR_6_CS_1_DDR4     (0x0400 & 0xffff)
 MC0_CTL_198_DATA : .dword (MR_6_CS_1_DDR4<<32)|(MR_5_CS_1_DDR4<<16)|(MR_4_CS_1_DDR4)

#define MR_0_CS_2_DDR4     (0x0104 & 0xffff)
#define MR_1_CS_2_DDR4     (0x0501 & 0xffff)
#define MR_2_CS_2_DDR4     (0x0000 & 0xffff)
#define MR_3_CS_2_DDR4     (0x0200 & 0xffff)
 MC0_CTL_1A0_DATA : .dword (MR_3_CS_2_DDR4<<48)|(MR_2_CS_2_DDR4<<32)|(MR_1_CS_2_DDR4<<16)|(MR_0_CS_2_DDR4)

#define MR_4_CS_2_DDR4     (0x0000 & 0xffff)
#define MR_5_CS_2_DDR4     (0x0480 & 0xffff)
#define MR_6_CS_2_DDR4     (0x0400 & 0xffff)
 MC0_CTL_1A8_DATA : .dword (MR_6_CS_2_DDR4<<32)|(MR_5_CS_2_DDR4<<16)|(MR_4_CS_2_DDR4)

#define MR_0_CS_3_DDR4     (0x0104 & 0xffff)
#define MR_1_CS_3_DDR4     (0x0501 & 0xffff)
#define MR_2_CS_3_DDR4     (0x0000 & 0xffff)
#define MR_3_CS_3_DDR4     (0x0200 & 0xffff)
 MC0_CTL_1B0_DATA : .dword (MR_3_CS_3_DDR4<<48)|(MR_2_CS_3_DDR4<<32)|(MR_1_CS_3_DDR4<<16)|(MR_0_CS_3_DDR4)

#define MR_4_CS_3_DDR4     (0x0000 & 0xffff)
#define MR_5_CS_3_DDR4     (0x0480 & 0xffff)
#define MR_6_CS_3_DDR4     (0x0400 & 0xffff)
 MC0_CTL_1B8_DATA : .dword (MR_6_CS_3_DDR4<<32)|(MR_5_CS_3_DDR4<<16)|(MR_4_CS_3_DDR4)

#define MR_0_CS_4_DDR4     (0x0104 & 0xffff)
#define MR_1_CS_4_DDR4     (0x0501 & 0xffff)
#define MR_2_CS_4_DDR4     (0x0000 & 0xffff)
#define MR_3_CS_4_DDR4     (0x0200 & 0xffff)
 MC0_CTL_1C0_DATA : .dword (MR_3_CS_4_DDR4<<48)|(MR_2_CS_4_DDR4<<32)|(MR_1_CS_4_DDR4<<16)|(MR_0_CS_4_DDR4)

#define MR_4_CS_4_DDR4     (0x0000 & 0xffff)
#define MR_5_CS_4_DDR4     (0x0480 & 0xffff)
#define MR_6_CS_4_DDR4     (0x0400 & 0xffff)
 MC0_CTL_1C8_DATA : .dword (MR_6_CS_4_DDR4<<32)|(MR_5_CS_4_DDR4<<16)|(MR_4_CS_4_DDR4)

#define MR_0_CS_5_DDR4     (0x0104 & 0xffff)
#define MR_1_CS_5_DDR4     (0x0501 & 0xffff)
#define MR_2_CS_5_DDR4     (0x0000 & 0xffff)
#define MR_3_CS_5_DDR4     (0x0200 & 0xffff)
 MC0_CTL_1D0_DATA : .dword (MR_3_CS_5_DDR4<<48)|(MR_2_CS_5_DDR4<<32)|(MR_1_CS_5_DDR4<<16)|(MR_0_CS_5_DDR4)

#define MR_4_CS_5_DDR4     (0x0000 & 0xffff)
#define MR_5_CS_5_DDR4     (0x0480 & 0xffff)
#define MR_6_CS_5_DDR4     (0x0400 & 0xffff)
 MC0_CTL_1D8_DATA : .dword (MR_6_CS_5_DDR4<<32)|(MR_5_CS_5_DDR4<<16)|(MR_4_CS_5_DDR4)

#define MR_0_CS_6_DDR4     (0x0104 & 0xffff)
#define MR_1_CS_6_DDR4     (0x0501 & 0xffff)
#define MR_2_CS_6_DDR4     (0x0000 & 0xffff)
#define MR_3_CS_6_DDR4     (0x0200 & 0xffff)
 MC0_CTL_1E0_DATA : .dword (MR_3_CS_6_DDR4<<48)|(MR_2_CS_6_DDR4<<32)|(MR_1_CS_6_DDR4<<16)|(MR_0_CS_6_DDR4)

#define MR_4_CS_6_DDR4     (0x0000 & 0xffff)
#define MR_5_CS_6_DDR4     (0x0480 & 0xffff)
#define MR_6_CS_6_DDR4     (0x0400 & 0xffff)
 MC0_CTL_1E8_DATA : .dword (MR_6_CS_6_DDR4<<32)|(MR_5_CS_6_DDR4<<16)|(MR_4_CS_6_DDR4)

#define MR_0_CS_7_DDR4     (0x0104 & 0xffff)
#define MR_1_CS_7_DDR4     (0x0501 & 0xffff)
#define MR_2_CS_7_DDR4     (0x0000 & 0xffff)
#define MR_3_CS_7_DDR4     (0x0200 & 0xffff)
 MC0_CTL_1F0_DATA : .dword (MR_3_CS_7_DDR4<<48)|(MR_2_CS_7_DDR4<<32)|(MR_1_CS_7_DDR4<<16)|(MR_0_CS_7_DDR4)

#define MR_4_CS_7_DDR4     (0x0000 & 0xffff)
#define MR_5_CS_7_DDR4     (0x0480 & 0xffff)
#define MR_6_CS_7_DDR4     (0x0400 & 0xffff)
 MC0_CTL_1F8_DATA : .dword (MR_6_CS_7_DDR4<<32)|(MR_5_CS_7_DDR4<<16)|(MR_4_CS_7_DDR4)
                                         
 MC0_CTL_200_DATA : .dword 0x0000010003140400
// MC0_CTL_200_DATA : .dword 0x0000010003000000
// MC0_CTL_200_DATA : .dword 0x0000010301000000
 MC0_CTL_208_DATA : .dword 0x00000000002000aa
                                         
 MC0_CTL_210_DATA : .dword 0x0000000000000000
 MC0_CTL_218_DATA : .dword 0x0000000000000000  #useless
                                         
 MC0_CTL_220_DATA : .dword 0x0fff0000fff00000
 MC0_CTL_228_DATA : .dword 0x0000000000000000  #useless
                                         
 MC0_CTL_230_DATA : .dword 0x0000030300010202   //2G UDIMM MICRON
 //MC0_CTL_230_DATA : .dword 0x0000000000000000   //2G UDIMM MICRON
// MC0_CTL_230_DATA : .dword 0x0000020300010302   
// MC0_CTL_230_DATA : .dword 0x0000030301010302 //2G
 MC0_CTL_238_DATA : .dword 0x000000ff00000000
                                         
 MC0_CTL_240_DATA : .dword 0x0f28100001010101
 MC0_CTL_248_DATA : .dword 0x0000000000ff010f  #useless
                                         
 MC0_CTL_250_DATA : .dword 0x0000000000000080
 MC0_CTL_258_DATA : .dword 0x0055004400330022
                                         
 MC0_CTL_260_DATA : .dword 0x0000000000000000
 MC0_CTL_268_DATA : .dword 0x0000000000000000

 MC0_CTL_270_DATA : .dword 0x0000000000000000
 MC0_CTL_278_DATA : .dword 0x0000000000000000  #useless
                                         
 //MC0_CTL_280_DATA : .dword 0x0000000300000000//ecc enable
 MC0_CTL_280_DATA : .dword 0x0000000000000000
 MC0_CTL_288_DATA : .dword 0x0000000000000000  #useless
                                         
 MC0_CTL_290_DATA : .dword 0x0000000000000000  #useless
 MC0_CTL_298_DATA : .dword 0x0000000000000000  #useless

 MC0_CTL_2A0_DATA : .dword 0x0000000000000000
 MC0_CTL_2A8_DATA : .dword 0x0000000000000000  #useless
                                         
 MC0_CTL_2B0_DATA : .dword 0x0000000000000000  #useless
 MC0_CTL_2B8_DATA : .dword 0x0000000000000000  #useless

 MC0_CTL_2C0_DATA : .dword 0x0000000000000000  #useless
 MC0_CTL_2C8_DATA : .dword 0x0000000000000000  #useless

 MC0_CTL_2D0_DATA : .dword 0x0000000000000000  #useless
 MC0_CTL_2D8_DATA : .dword 0x0000000000000000  #useless

 MC0_CTL_2E0_DATA : .dword 0x0000000000000000  #useless
 MC0_CTL_2E8_DATA : .dword 0x0000000000000000  #useless

 MC0_CTL_2F0_DATA : .dword 0x0000000000000000  #useless
 MC0_CTL_2F8_DATA : .dword 0x0000000000000000  #useless

 MC0_CTL_300_DATA : .dword 0x0000000000000703
 MC0_CTL_308_DATA : .dword 0x0000000000000000
                                         
 MC0_CTL_310_DATA : .dword 0x0000000000000000  #useless
 MC0_CTL_318_DATA : .dword 0x0000000000000000  #useless

 MC0_CTL_320_DATA : .dword 0x0000000000000000  #useless
 MC0_CTL_328_DATA : .dword 0x0000000000000000  #useless

 MC0_CTL_330_DATA : .dword 0x0000000000000000  #useless
 MC0_CTL_338_DATA : .dword 0x0000000000000000  #useless

 MC0_CTL_340_DATA : .dword 0x0000000000000000
 MC0_CTL_348_DATA : .dword 0x0000000000000000  #useless
                                         
 MC0_CTL_350_DATA : .dword 0x0018001400140010
 MC0_CTL_358_DATA : .dword 0x0000000000000008
                                         
 MC0_CTL_360_DATA : .dword 0x0000000000000000  #useless
 MC0_CTL_368_DATA : .dword 0x0000000000000000  #useless

 MC0_CTL_370_DATA : .dword 0x0000000000000000  #useless
 MC0_CTL_378_DATA : .dword 0x0000000000000000  #useless

 MC0_CTL_380_DATA : .dword 0x000000000000000f
 MC0_CTL_388_DATA : .dword 0x0000000000000001

 MC0_CTL_390_DATA : .dword 0x0000000000000000  #useless
 MC0_CTL_398_DATA : .dword 0x0000000000000000  #useless
                                         
 MC0_CTL_3A0_DATA : .dword 0x0000000000000000  #useless
 MC0_CTL_3A8_DATA : .dword 0x0000000000000000  #useless
                                         
 MC0_CTL_3B0_DATA : .dword 0x0000000000000000  #useless
 MC0_CTL_3B8_DATA : .dword 0x0000000000000000  #useless
                                         
 MC0_CTL_3C0_DATA : .dword 0x8040201081248124
// MC0_CTL_3C0_DATA : .dword 0x0000000000000000
 MC0_CTL_3C8_DATA : .dword 0x0000000000000600
                                         
 MC0_CTL_3D0_DATA : .dword 0x0000000000000000
 MC0_CTL_3D8_DATA : .dword 0x0000000000000000
                                         
 MC0_CTL_3E0_DATA : .dword 0x0000000000000000  #useless
 MC0_CTL_3E8_DATA : .dword 0x0000000000000000  #useless
                                         
 MC0_CTL_3F0_DATA : .dword 0x0000000000000000  #useless
 MC0_CTL_3F8_DATA : .dword 0x0000000000000000  #useless
 MC0_CTL_400_DATA : .dword 0x0000000824083010
 MC0_CTL_408_DATA : .dword 0x0000000000000000  #useless
 MC0_CTL_410_DATA : .dword 0x0000000000000000  #useless
 MC0_CTL_418_DATA : .dword 0x0000000000000000  #useless
 MC0_CTL_420_DATA : .dword 0x0000000000000000  #useless
 MC0_CTL_428_DATA : .dword 0x0000000000000000  #useless
 MC0_CTL_430_DATA : .dword 0x0000000000000000  #useless
 MC0_CTL_438_DATA : .dword 0x0000000000000000  #useless
 MC0_CTL_440_DATA : .dword 0x0000000000010307
 MC0_CTL_448_DATA : .dword 0x0000000000000000
 MC0_CTL_450_DATA : .dword 0x0000000000000000
 MC0_CTL_458_DATA : .dword 0x0000000000000000
 MC0_CTL_460_DATA : .dword 0x0000000000000000
 MC0_CTL_468_DATA : .dword 0x0000000000000000
 MC0_CTL_470_DATA : .dword 0x0000000000000000
 MC0_CTL_478_DATA : .dword 0x0000000000000000  #useless
                    .dword 0x0:16
 MC0_CTL_500_DATA : .dword 0x0000000000000000
 MC0_CTL_508_DATA : .dword 0x0000040000000000
 MC0_CTL_510_DATA : .dword 0x0000000000000000
 MC0_CTL_518_DATA : .dword 0x0000000000000000
 MC0_CTL_520_DATA : .dword 0x0000000000000000
 MC0_CTL_528_DATA : .dword 0x0000000000000000
 MC0_CTL_530_DATA : .dword 0x0000000000000000
 MC0_CTL_538_DATA : .dword 0x0000000000000000
 MC0_CTL_540_DATA : .dword 0x0000000000000000  #useless
 MC0_CTL_548_DATA : .dword 0x0000000000000000  #useless
 MC0_CTL_550_DATA : .dword 0x0000000000000000  #useless
 MC0_CTL_558_DATA : .dword 0x0000000000000000  #useless
 MC0_CTL_560_DATA : .dword 0x0000000000000000  #useless
 MC0_CTL_568_DATA : .dword 0x0000000000000000  #useless
 MC0_CTL_570_DATA : .dword 0x0000000000000000  #useless
 MC0_CTL_578_DATA : .dword 0x0000000000000000  #useless
 MC0_CTL_580_DATA : .dword 0xfffffffff0000000
 MC0_CTL_588_DATA : .dword 0xffffff0000000000
 MC0_CTL_590_DATA : .dword 0x0000000000000000
 MC0_CTL_598_DATA : .dword 0x0000000000000000
 MC0_CTL_5A0_DATA : .dword 0x0000000000000000
 MC0_CTL_5A8_DATA : .dword 0x0000000000000000
 MC0_CTL_5B0_DATA : .dword 0x0000000000000000
 MC0_CTL_5B8_DATA : .dword 0x0000000000000000
 MC0_CTL_5C0_DATA : .dword 0x0000000000000000  #useless
 MC0_CTL_5C8_DATA : .dword 0x0000000000000000  #useless
 MC0_CTL_5D0_DATA : .dword 0x0000000000000000  #useless
 MC0_CTL_5D8_DATA : .dword 0x0000000000000000  #useless
 MC0_CTL_5E0_DATA : .dword 0x0000000000000000  #useless
 MC0_CTL_5E8_DATA : .dword 0x0000000000000000  #useless
 MC0_CTL_5F0_DATA : .dword 0x0000000000000000  #useless
 MC0_CTL_5F8_DATA : .dword 0x0000000000000000  #useless
 MC0_CTL_600_DATA : .dword 0x00000000000000f0
 MC0_CTL_608_DATA : .dword 0x00000000000000f0
 MC0_CTL_610_DATA : .dword 0x0000000080000000
 MC0_CTL_618_DATA : .dword 0x0000000000000000
 MC0_CTL_620_DATA : .dword 0x0000000000000000
 MC0_CTL_628_DATA : .dword 0x0000000000000000
 MC0_CTL_630_DATA : .dword 0x0000000000000000
 MC0_CTL_638_DATA : .dword 0x0000000000000000
                    .dword 0x0:24
 MC0_CTL_700_DATA : .dword 0x0000000000000000
 MC0_CTL_708_DATA : .dword 0xaaaaaaaafffffffe
 MC0_CTL_710_DATA : .dword 0x0000000000000000  #useless
 MC0_CTL_718_DATA : .dword 0x0000000000000000  #useless
 MC0_CTL_720_DATA : .dword 0x0000000000000000
 MC0_CTL_728_DATA : .dword 0x0000000000000000  #useless
 MC0_CTL_730_DATA : .dword 0x0000000000000000
 MC0_CTL_738_DATA : .dword 0x0000000000000000  #useless

ddr4_reg_data_mc1:
ddr4_RDIMM_reg_data_mc1:
 MC1_PHY_000_DATA : .dword 0x0000000000000010  #useless
 MC1_PHY_008_DATA : .dword 0x0000000000000020
 MC1_PHY_010_DATA : .dword 0x0000000000000000
 MC1_PHY_018_DATA : .dword 0x0000000000000000  #useless
 MC1_PHY_020_DATA : .dword 0x0000000000000001
 MC1_PHY_028_DATA : .dword 0x0000000000000000
 MC1_PHY_030_DATA : .dword 0x0058010100040510
 MC1_PHY_038_DATA : .dword 0x0000000000000144
 MC1_PHY_040_DATA : .dword DOUB_CFG
 MC1_PHY_048_DATA : .dword (PLL_CFG<<32)|SEL_DBL
 MC1_PHY_050_DATA : .dword DOUB_CFG
 MC1_PHY_058_DATA : .dword (PLL_CFG<<32)|SEL_DBL
 MC1_PHY_060_DATA : .dword DOUB_CFG
 MC1_PHY_068_DATA : .dword (PLL_CFG<<32)|SEL_DBL
 MC1_PHY_070_DATA : .dword DOUB_CFG
 MC1_PHY_078_DATA : .dword (PLL_CFG<<32)|SEL_DBL
 MC1_PHY_080_DATA : .dword DOUB_CFG
 MC1_PHY_088_DATA : .dword (PLL_CFG<<32)|SEL_DBL
 MC1_PHY_090_DATA : .dword DOUB_CFG
 MC1_PHY_098_DATA : .dword (PLL_CFG<<32)|SEL_DBL
 MC1_PHY_0A0_DATA : .dword DOUB_CFG
 MC1_PHY_0A8_DATA : .dword (PLL_CFG<<32)|SEL_DBL
 MC1_PHY_0B0_DATA : .dword DOUB_CFG
 MC1_PHY_0B8_DATA : .dword (PLL_CFG<<32)|SEL_DBL
 MC1_PHY_0C0_DATA : .dword DOUB_CFG
 MC1_PHY_0C8_DATA : .dword (PLL_CFG<<32)|SEL_DBL
 MC1_PHY_0D0_DATA : .dword DOUB_CFG
 MC1_PHY_0D8_DATA : .dword (PLL_CFG<<32)|SEL_DBL
 MC1_PHY_0E0_DATA : .dword 0x0000000000001000
 MC1_PHY_0E8_DATA : .dword 0x0000000000000000  #useless
 MC1_PHY_0F0_DATA : .dword 0x0000000000000000  #useless
 MC1_PHY_0F8_DATA : .dword 0x0000000000000000  #useless
 MC1_PHY_100_DATA : .dword 0x0000000000002000|(MC1_DLL_WDQ0<<24)|(MC1_DLL_WDQ0)
 MC1_PHY_108_DATA : .dword 0x0000000000000000|(MC1_DLL_GATE0<<16)|(MC1_RDDQS_DLY<<8)|MC1_RDDQS_DLY
 MC1_PHY_110_DATA : .dword 0x0000000000000000|(RDODT_CTRL<<56)|(RDGATE_LEN<<48)|(RDGATE_MODE<<40)|(RDGATE_CTRL<<32)|DQ_OE
 MC1_PHY_118_DATA : .dword 0x0000000000000000|(MC1_DLY_2X0<<16)
 MC1_PHY_120_DATA : .dword 0x0000000000000000
 MC1_PHY_128_DATA : .dword 0x0000000000000000
 MC1_PHY_130_DATA : .dword 0x0000000000000000
 MC1_PHY_138_DATA : .dword 0x0000000000000000
 MC1_PHY_140_DATA : .dword 0x0000000000000000
 MC1_PHY_148_DATA : .dword 0x0000000000000000  #useless
 MC1_PHY_150_DATA : .dword 0x0000000000000000
 MC1_PHY_158_DATA : .dword 0x0000000000000000
 MC1_PHY_160_DATA : .dword 0x0000000000000000
 MC1_PHY_168_DATA : .dword 0x0000000000000000
 MC1_PHY_170_DATA : .dword 0x0000000000000000
 MC1_PHY_178_DATA : .dword 0x0000000000000000
 MC1_PHY_180_DATA : .dword 0x0000000000002000|(MC1_DLL_WDQ1<<24)|(MC1_DLL_WDQ1)
 MC1_PHY_188_DATA : .dword 0x0000000000000000|(MC1_DLL_GATE1<<16)|(MC1_RDDQS_DLY<<8)|MC1_RDDQS_DLY
 MC1_PHY_190_DATA : .dword 0x0000000000000000|(RDODT_CTRL<<56)|(RDGATE_LEN<<48)|(RDGATE_MODE<<40)|(RDGATE_CTRL<<32)|DQ_OE
 MC1_PHY_198_DATA : .dword 0x0000000000000000|(MC1_DLY_2X1<<16)
 MC1_PHY_1A0_DATA : .dword 0x0000000000000000
 MC1_PHY_1A8_DATA : .dword 0x0000000000000000
 MC1_PHY_1B0_DATA : .dword 0x0000000000000000
 MC1_PHY_1B8_DATA : .dword 0x0000000000000000
 MC1_PHY_1C0_DATA : .dword 0x0000000000000000
 MC1_PHY_1C8_DATA : .dword 0x0000000000000000  #useless
 MC1_PHY_1D0_DATA : .dword 0x0000000000000000
 MC1_PHY_1D8_DATA : .dword 0x0000000000000000
 MC1_PHY_1E0_DATA : .dword 0x0000000000000000
 MC1_PHY_1E8_DATA : .dword 0x0000000000000000
 MC1_PHY_1F0_DATA : .dword 0x0000000000000000
 MC1_PHY_1F8_DATA : .dword 0x0000000000000000
 MC1_PHY_200_DATA : .dword 0x0000000000002000|(MC1_DLL_WDQ2<<24)|(MC1_DLL_WDQ2)
 MC1_PHY_208_DATA : .dword 0x0000000000000000|(MC1_DLL_GATE2<<16)|(MC1_RDDQS_DLY<<8)|MC1_RDDQS_DLY
 MC1_PHY_210_DATA : .dword 0x0000000000000000|(RDODT_CTRL<<56)|(RDGATE_LEN<<48)|(RDGATE_MODE<<40)|(RDGATE_CTRL<<32)|DQ_OE
 MC1_PHY_218_DATA : .dword 0x0000000000000000|(MC1_DLY_2X2<<16)
 MC1_PHY_220_DATA : .dword 0x0000000000000000
 MC1_PHY_228_DATA : .dword 0x0000000000000000
 MC1_PHY_230_DATA : .dword 0x0000000000000000
 MC1_PHY_238_DATA : .dword 0x0000000000000000
 MC1_PHY_240_DATA : .dword 0x0000000000000000
 MC1_PHY_248_DATA : .dword 0x0000000000000000  #useless
 MC1_PHY_250_DATA : .dword 0x0000000000000000
 MC1_PHY_258_DATA : .dword 0x0000000000000000
 MC1_PHY_260_DATA : .dword 0x0000000000000000
 MC1_PHY_268_DATA : .dword 0x0000000000000000
 MC1_PHY_270_DATA : .dword 0x0000000000000000
 MC1_PHY_278_DATA : .dword 0x0000000000000000
 MC1_PHY_280_DATA : .dword 0x0000000000002000|(MC1_DLL_WDQ3<<24)|(MC1_DLL_WDQ3)
 MC1_PHY_288_DATA : .dword 0x0000000000000000|(MC1_DLL_GATE3<<16)|(MC1_RDDQS_DLY<<8)|MC1_RDDQS_DLY
 MC1_PHY_290_DATA : .dword 0x0000000000000000|(RDODT_CTRL<<56)|(RDGATE_LEN<<48)|(RDGATE_MODE<<40)|(RDGATE_CTRL<<32)|DQ_OE
 MC1_PHY_298_DATA : .dword 0x0000000000000000|(MC1_DLY_2X3<<16)
 MC1_PHY_2A0_DATA : .dword 0x0000000000000000
 MC1_PHY_2A8_DATA : .dword 0x0000000000000000
 MC1_PHY_2B0_DATA : .dword 0x0000000000000000
 MC1_PHY_2B8_DATA : .dword 0x0000000000000000
 MC1_PHY_2C0_DATA : .dword 0x0000000000000000
 MC1_PHY_2C8_DATA : .dword 0x0000000000000000  #useless
 MC1_PHY_2D0_DATA : .dword 0x0000000000000000
 MC1_PHY_2D8_DATA : .dword 0x0000000000000000
 MC1_PHY_2E0_DATA : .dword 0x0000000000000000
 MC1_PHY_2E8_DATA : .dword 0x0000000000000000
 MC1_PHY_2F0_DATA : .dword 0x0000000000000000
 MC1_PHY_2F8_DATA : .dword 0x0000000000000000
 MC1_PHY_300_DATA : .dword 0x0000000000002000|(MC1_DLL_WDQ4<<24)|(MC1_DLL_WDQ4)
 MC1_PHY_308_DATA : .dword 0x0000000000000000|(MC1_DLL_GATE4<<16)|(MC1_RDDQS_DLY<<8)|MC1_RDDQS_DLY
 MC1_PHY_310_DATA : .dword 0x0000000000000000|(RDODT_CTRL<<56)|(RDGATE_LEN<<48)|(RDGATE_MODE<<40)|(RDGATE_CTRL<<32)|DQ_OE
 MC1_PHY_318_DATA : .dword 0x0000000000000000|(MC1_DLY_2X4<<16)
 MC1_PHY_320_DATA : .dword 0x0000000000000000
 MC1_PHY_328_DATA : .dword 0x0000000000000000
 MC1_PHY_330_DATA : .dword 0x0000000000000000
 MC1_PHY_338_DATA : .dword 0x0000000000000000
 MC1_PHY_340_DATA : .dword 0x0000000000000000
 MC1_PHY_348_DATA : .dword 0x0000000000000000  #useless
 MC1_PHY_350_DATA : .dword 0x0000000000000000
 MC1_PHY_358_DATA : .dword 0x0000000000000000
 MC1_PHY_360_DATA : .dword 0x0000000000000000
 MC1_PHY_368_DATA : .dword 0x0000000000000000
 MC1_PHY_370_DATA : .dword 0x0000000000000000
 MC1_PHY_378_DATA : .dword 0x0000000000000000
 MC1_PHY_380_DATA : .dword 0x0000000000002000|(MC1_DLL_WDQ5<<24)|(MC1_DLL_WDQ5)
 MC1_PHY_388_DATA : .dword 0x0000000000000000|(MC1_DLL_GATE5<<16)|(MC1_RDDQS_DLY<<8)|MC1_RDDQS_DLY
 MC1_PHY_390_DATA : .dword 0x0000000000000000|(RDODT_CTRL<<56)|(RDGATE_LEN<<48)|(RDGATE_MODE<<40)|(RDGATE_CTRL<<32)|DQ_OE
 MC1_PHY_398_DATA : .dword 0x0000000000000000|(MC1_DLY_2X5<<16)
 MC1_PHY_3A0_DATA : .dword 0x0000000000000000
 MC1_PHY_3A8_DATA : .dword 0x0000000000000000
 MC1_PHY_3B0_DATA : .dword 0x0000000000000000
 MC1_PHY_3B8_DATA : .dword 0x0000000000000000
 MC1_PHY_3C0_DATA : .dword 0x0000000000000000
 MC1_PHY_3C8_DATA : .dword 0x0000000000000000  #useless
 MC1_PHY_3D0_DATA : .dword 0x0000000000000000
 MC1_PHY_3D8_DATA : .dword 0x0000000000000000
 MC1_PHY_3E0_DATA : .dword 0x0000000000000000
 MC1_PHY_3E8_DATA : .dword 0x0000000000000000
 MC1_PHY_3F0_DATA : .dword 0x0000000000000000
 MC1_PHY_3F8_DATA : .dword 0x0000000000000000
 MC1_PHY_400_DATA : .dword 0x0000000000002000|(MC1_DLL_WDQ6<<24)|(MC1_DLL_WDQ6)
 MC1_PHY_408_DATA : .dword 0x0000000000000000|(MC1_DLL_GATE6<<16)|(MC1_RDDQS_DLY<<8)|MC1_RDDQS_DLY
 MC1_PHY_410_DATA : .dword 0x0000000000000000|(RDODT_CTRL<<56)|(RDGATE_LEN<<48)|(RDGATE_MODE<<40)|(RDGATE_CTRL<<32)|DQ_OE
 MC1_PHY_418_DATA : .dword 0x0000000000000000|(MC1_DLY_2X6<<16)
 MC1_PHY_420_DATA : .dword 0x0000000000000000
 MC1_PHY_428_DATA : .dword 0x0000000000000000
 MC1_PHY_430_DATA : .dword 0x0000000000000000
 MC1_PHY_438_DATA : .dword 0x0000000000000000
 MC1_PHY_440_DATA : .dword 0x0000000000000000
 MC1_PHY_448_DATA : .dword 0x0000000000000000  #useless
 MC1_PHY_450_DATA : .dword 0x0000000000000000
 MC1_PHY_458_DATA : .dword 0x0000000000000000
 MC1_PHY_460_DATA : .dword 0x0000000000000000
 MC1_PHY_468_DATA : .dword 0x0000000000000000
 MC1_PHY_470_DATA : .dword 0x0000000000000000
 MC1_PHY_478_DATA : .dword 0x0000000000000000
 MC1_PHY_480_DATA : .dword 0x0000000000002000|(MC1_DLL_WDQ7<<24)|(MC1_DLL_WDQ7)
 MC1_PHY_488_DATA : .dword 0x0000000000000000|(MC1_DLL_GATE7<<16)|(MC1_RDDQS_DLY<<8)|MC1_RDDQS_DLY
 MC1_PHY_490_DATA : .dword 0x0000000000000000|(RDODT_CTRL<<56)|(RDGATE_LEN<<48)|(RDGATE_MODE<<40)|(RDGATE_CTRL<<32)|DQ_OE
 MC1_PHY_498_DATA : .dword 0x0000000000000300|(MC1_DLY_2X7<<16)
 MC1_PHY_4A0_DATA : .dword 0x0000000000000000
 MC1_PHY_4A8_DATA : .dword 0x0000000000000000
 MC1_PHY_4B0_DATA : .dword 0x0000000000000000
 MC1_PHY_4B8_DATA : .dword 0x0000000000000000
 MC1_PHY_4C0_DATA : .dword 0x0000000000000000
 MC1_PHY_4C8_DATA : .dword 0x0000000000000000  #useless
 MC1_PHY_4D0_DATA : .dword 0x0000000000000000
 MC1_PHY_4D8_DATA : .dword 0x0000000000000000
 MC1_PHY_4E0_DATA : .dword 0x0000000000000000
 MC1_PHY_4E8_DATA : .dword 0x0000000000000000
 MC1_PHY_4F0_DATA : .dword 0x0000000000000000
 MC1_PHY_4F8_DATA : .dword 0x0000000000000000
 MC1_PHY_500_DATA : .dword 0x0000000000002000|(MC1_DLL_WDQ8<<24)|(MC1_DLL_WDQ8)
 MC1_PHY_508_DATA : .dword 0x0000000000000000|(MC1_DLL_GATE8<<16)|(MC1_RDDQS_DLY<<8)|MC1_RDDQS_DLY
 MC1_PHY_510_DATA : .dword 0x0000000000000000|(RDODT_CTRL<<56)|(RDGATE_LEN<<48)|(RDGATE_MODE<<40)|(RDGATE_CTRL<<32)|DQ_OE
 MC1_PHY_518_DATA : .dword 0x0000000000000000|(MC1_DLY_2X8<<16)
 MC1_PHY_520_DATA : .dword 0x0000000000000000
 MC1_PHY_528_DATA : .dword 0x0000000000000000
 MC1_PHY_530_DATA : .dword 0x0000000000000000
 MC1_PHY_538_DATA : .dword 0x0000000000000000
 MC1_PHY_540_DATA : .dword 0x0000000000000000
 MC1_PHY_548_DATA : .dword 0x0000000000000000  #useless
 MC1_PHY_550_DATA : .dword 0x0000000000000000
 MC1_PHY_558_DATA : .dword 0x0000000000000000
 MC1_PHY_560_DATA : .dword 0x0000000000000000
 MC1_PHY_568_DATA : .dword 0x0000000000000000
 MC1_PHY_570_DATA : .dword 0x0000000000000000
 MC1_PHY_578_DATA : .dword 0x0000000000000000
                    .dword 0x0:48
 MC1_PHY_700_DATA : .dword 0x0000000001300000
                    .dword 0x0:31
#define MC1_PAD_DS_OCD_DQS 0x7//drive strength:max7
#define MC1_PAD_DS_OCD_DQ  0x7
#define MC1_PAD_DS_COMP    0x1
#define MC1_PAD_DS_DCC     0x8
#define MC1_PAD_DS_ENZI    0x0//0x1
#define MC1_PAD_DS_ODT_DQS 0x6 //odt value:max7
#define MC1_PAD_DS_ODT_DQ  0x4
#define MC1_PAD_DS_CODEIN  0x1//0:parameter outside
#define MC1_PAD_DS_CTRL    ((MC1_PAD_DS_CODEIN<<18)|(MC1_PAD_DS_ODT_DQ<<15)|(MC1_PAD_DS_ODT_DQS<<12)|(MC1_PAD_DS_ENZI<<11)|(MC1_PAD_DS_DCC<<7)|(MC1_PAD_DS_COMP<<6)|(MC1_PAD_DS_OCD_DQ<<3)|(MC1_PAD_DS_OCD_DQS))
#define MC1_PAD_DS_SPLIT ((MC1_PAD_DS_DCC<<12)|(MC1_PAD_DS_ODT_DQ<<9)|(MC1_PAD_DS_ODT_DQS<<6)|(MC1_PAD_DS_OCD_DQ<<3)|(MC1_PAD_DS_OCD_DQS))
 MC1_PHY_800_DATA : .dword 0x050000000026d2df|(MC1_PAD_DS_CTRL<<32)
 MC1_PHY_808_DATA : .dword 0x000201010003ed79
 MC1_PHY_810_DATA : .dword 0x0000000000000000|(VREF<<48)|(VREF<<32)|(VREF<<16)|(VREF<<0)
 MC1_PHY_818_DATA : .dword 0x0000000000000000|(VREF<<48)|(VREF<<32)|(VREF<<16)|(VREF<<0)
 MC1_PHY_820_DATA : .dword 0x0000000000000000|(VREF<<0)
 MC1_PHY_828_DATA : .dword 0x0000000000000000
 MC1_PHY_830_DATA : .dword COMP_CTRL
 MC1_PHY_838_DATA : .dword 0x0000000000000000
 MC1_PHY_840_DATA : .dword (MC1_PAD_DS_SPLIT<<48)|(MC1_PAD_DS_SPLIT<<32)|(MC1_PAD_DS_SPLIT<<16)|(MC1_PAD_DS_SPLIT)
 MC1_PHY_848_DATA : .dword (MC1_PAD_DS_SPLIT<<48)|(MC1_PAD_DS_SPLIT<<32)|(MC1_PAD_DS_SPLIT<<16)|(MC1_PAD_DS_SPLIT)
 MC1_PHY_850_DATA : .dword (MC1_PAD_DS_SPLIT)
 MC1_PHY_858_DATA : .dword 0x0000000000000000
 MC1_PHY_860_DATA : .dword 0x0000000000000000
 MC1_PHY_868_DATA : .dword 0x0000000000000000
 MC1_PHY_870_DATA : .dword 0x0000000000000000
 MC1_PHY_878_DATA : .dword 0x0000000000000000
 MC1_PHY_880_DATA : .dword 0x0000000000000000
 MC1_PHY_888_DATA : .dword 0x0000000000000000
 MC1_PHY_890_DATA : .dword 0x0000000000000000 
 MC1_PHY_898_DATA : .dword 0x0000000000000000 
 MC1_PHY_8a0_DATA : .dword 0x0000000000000000 
 MC1_PHY_8a8_DATA : .dword 0x0000000000000000 
 MC1_PHY_8b0_DATA : .dword 0x0000000000000000 
 MC1_PHY_8b8_DATA : .dword 0x0000000000000000 
 MC1_PHY_8c0_DATA : .dword 0x0000000000000000 
 MC1_PHY_8c8_DATA : .dword 0x0000000000000000 
 MC1_PHY_8d0_DATA : .dword 0x0000000000000000 
 MC1_PHY_8d8_DATA : .dword 0x0000000000000000 
 MC1_PHY_8e0_DATA : .dword 0x0000000000000000 
 MC1_PHY_8e8_DATA : .dword 0x0000000000000000 
 MC1_PHY_8f0_DATA : .dword 0x0000000000000000 
 MC1_PHY_8f8_DATA : .dword 0x0000000000000000 
 MC1_PHY_900_DATA : .dword 0x0000000000000000 
 MC1_PHY_908_DATA : .dword 0x0000000000000000 
                    .dword 0x0:222
 MC1_CTL_000_DATA : .dword (tRP<<48)|(tWLDQSEN<<40)|(tMOD<<32)|(tXPR<<16)|(tCKE<<8)|(tRESET)  

 MC1_CTL_008_DATA : .dword (tODTL)
                                         
 MC1_CTL_010_DATA : .dword (tREFretention<<32)|(tRFC<<16)|(tREF)

 MC1_CTL_018_DATA : .dword (tCKESR<<56)|(tXSRD<<48)|(tXS<<32)|(tRFC_dlr<<16)|(tREF_IDLE)
                                         
 MC1_CTL_020_DATA : .dword (tRDPDEN<<24)|(tCPDED<<16)|(tXPDLL<<8)|(tXP)

 MC1_CTL_028_DATA : .dword (tZQperiod<<24)|(tZQCL<<16)|(tZQCS<<8)|(tZQ_CMD)
                                         
 MC1_CTL_030_DATA : .dword 0x0000000000000000  #useless

 MC1_CTL_038_DATA : .dword 0x0000000000000000  #useless

 MC1_CTL_040_DATA : .dword (tRCD<<56)|(tRRD_S_slr<<48)|(tRRD_L_slr<<40)|(tRRD<<32)|(tRAS_min)

 MC1_CTL_048_DATA : .dword (tRTP<<32)|(tWR_CRC_DM<<24)|(tWR<<16)|(tFAW_slr<<8)|(tFAW)
                                         
 MC1_CTL_050_DATA : .dword (tWTR_S_CRC_DM<<56)|(tWTR_L_CRC_DM<<48)|(tWTR_S<<40)|(tWTR<<32)|(tCCD<<16)|(tCCD_S<<8)|(tCCD)
 MC1_CTL_058_DATA : .dword 0x0000000000000000  #useless
                                         
 //MC1_CTL_060_DATA : .dword (tPHY_WRLAT<<40)|(tWL<<32)|(tRDDATA<<16)|(tPHY_RDLAT<<8)|(tRL)
 MC1_CTL_060_DATA : .dword 0x0000080c000b120c //UDIMM/RDIMM
 MC1_CTL_068_DATA : .dword (tCAL<<32)|(tPL)
                                         
 MC1_CTL_070_DATA : .dword (tW2P_sameba_adj<<40)|(tW2W_sameba_adj<<32)|(tW2R_sameba_adj<<24)|(tR2P_sameba_adj<<16)|(tR2W_sameba_adj<<8)|(tR2R_sameba_adj)

 MC1_CTL_078_DATA : .dword (tW2P_samebg_adj<<40)|(tW2W_samebg_adj<<32)|(tW2R_samebg_adj<<24)|(tR2P_samebg_adj<<16)|(tR2W_samebg_adj<<8)|(tR2R_samebg_adj)
                                         
 MC1_CTL_080_DATA : .dword (tW2P_samec_adj<<40)|(tW2W_samec_adj<<32)|(tW2R_samec_adj<<24)|(tR2P_samec_adj<<16)|(tR2W_samec_adj<<8)|(tR2R_samec_adj)

 MC1_CTL_088_DATA : .dword 0x0f0f0f0f0f0f0f0f  #useless
                                         
 MC1_CTL_090_DATA : .dword (tW2P_samecs_adj<<40)|(tW2W_samecs_adj<<32)|(tW2R_samecs_adj<<24)|(tR2P_samecs_adj<<16)|(tR2W_samecs_adj<<8)|(tR2R_samecs_adj)

 MC1_CTL_098_DATA : .dword (tW2P_diffcs_adj<<40)|(tW2W_diffcs_adj<<32)|(tW2R_diffcs_adj<<24)|(tR2P_diffcs_adj<<16)|(tR2W_diffcs_adj<<8)|(tR2R_diffcs_adj)
                    .dword 0x0:12

mc1_ctl_data_100:
 MC1_CTL_100_DATA : .dword (0x1<<40)|(CS_RESYNC<<32)|(CS_ZQCL<<24)|(CS_ZQ<<16)|(CS_MRS<<8)|(CS_ENABLE)


 MC1_CTL_108_DATA : .dword (CKE_MAP<<32)|(CS_MAP)

 MC1_CTL_110_DATA : .dword (CS2CID<<32)|(CID_MAP)
                                         
 MC1_CTL_118_DATA : .dword 0x0000000000000000  #useless

 MC1_CTL_120_DATA : .dword (MRS_REQ<<48)|(PRE_ALL_REQ<<32)|(CMD_CMD<<24)|(CMD_REQ<<8)|(COMMAND_MODE)
#RO: STATUS_CMD
#RO: PRE_ALL_DONE
#RO: MRS_DONE

 MC1_CTL_128_DATA : .dword (CMD_A<<32)|(CMD_BA<<24)|(CMD_BG<<16)|(CMD_C<<8)|(CMD_CS)

 MC1_CTL_130_DATA : .dword CMD_PDA

 MC1_CTL_138_DATA : .dword CMD_DQ0

 MC1_CTL_140_DATA : .dword (MR_3_CS_0<<48)|(MR_2_CS_0<<32)|(MR_1_CS_0<<16)|(MR_0_CS_0)

 MC1_CTL_148_DATA : .dword (MR_3_CS_1<<48)|(MR_2_CS_1<<32)|(MR_1_CS_1<<16)|(MR_0_CS_1)

 MC1_CTL_150_DATA : .dword (MR_3_CS_2<<48)|(MR_2_CS_2<<32)|(MR_1_CS_2<<16)|(MR_0_CS_2)

 MC1_CTL_158_DATA : .dword (MR_3_CS_3<<48)|(MR_2_CS_3<<32)|(MR_1_CS_3<<16)|(MR_0_CS_3)

 MC1_CTL_160_DATA : .dword (MR_3_CS_4<<48)|(MR_2_CS_4<<32)|(MR_1_CS_4<<16)|(MR_0_CS_4)

 MC1_CTL_168_DATA : .dword (MR_3_CS_5<<48)|(MR_2_CS_5<<32)|(MR_1_CS_5<<16)|(MR_0_CS_5)

 MC1_CTL_170_DATA : .dword (MR_3_CS_6<<48)|(MR_2_CS_6<<32)|(MR_1_CS_6<<16)|(MR_0_CS_6)

 MC1_CTL_178_DATA : .dword (MR_3_CS_7<<48)|(MR_2_CS_7<<32)|(MR_1_CS_7<<16)|(MR_0_CS_7)

 MC1_CTL_180_DATA : .dword (MR_3_CS_0_DDR4<<48)|(MR_2_CS_0_DDR4<<32)|(MR_1_CS_0_DDR4<<16)|(MR_0_CS_0_DDR4)

 MC1_CTL_188_DATA : .dword (MR_6_CS_0_DDR4<<32)|(MR_5_CS_0_DDR4<<16)|(MR_4_CS_0_DDR4)

 MC1_CTL_190_DATA : .dword (MR_3_CS_1_DDR4<<48)|(MR_2_CS_1_DDR4<<32)|(MR_1_CS_1_DDR4<<16)|(MR_0_CS_1_DDR4)

 MC1_CTL_198_DATA : .dword (MR_6_CS_1_DDR4<<32)|(MR_5_CS_1_DDR4<<16)|(MR_4_CS_1_DDR4)

 MC1_CTL_1A0_DATA : .dword (MR_3_CS_2_DDR4<<48)|(MR_2_CS_2_DDR4<<32)|(MR_1_CS_2_DDR4<<16)|(MR_0_CS_2_DDR4)

 MC1_CTL_1A8_DATA : .dword (MR_6_CS_2_DDR4<<32)|(MR_5_CS_2_DDR4<<16)|(MR_4_CS_2_DDR4)

 MC1_CTL_1B0_DATA : .dword (MR_3_CS_3_DDR4<<48)|(MR_2_CS_3_DDR4<<32)|(MR_1_CS_3_DDR4<<16)|(MR_0_CS_3_DDR4)

 MC1_CTL_1B8_DATA : .dword (MR_6_CS_3_DDR4<<32)|(MR_5_CS_3_DDR4<<16)|(MR_4_CS_3_DDR4)

 MC1_CTL_1C0_DATA : .dword (MR_3_CS_4_DDR4<<48)|(MR_2_CS_4_DDR4<<32)|(MR_1_CS_4_DDR4<<16)|(MR_0_CS_4_DDR4)

 MC1_CTL_1C8_DATA : .dword (MR_6_CS_4_DDR4<<32)|(MR_5_CS_4_DDR4<<16)|(MR_4_CS_4_DDR4)

 MC1_CTL_1D0_DATA : .dword (MR_3_CS_5_DDR4<<48)|(MR_2_CS_5_DDR4<<32)|(MR_1_CS_5_DDR4<<16)|(MR_0_CS_5_DDR4)

 MC1_CTL_1D8_DATA : .dword (MR_6_CS_5_DDR4<<32)|(MR_5_CS_5_DDR4<<16)|(MR_4_CS_5_DDR4)

 MC1_CTL_1E0_DATA : .dword (MR_3_CS_6_DDR4<<48)|(MR_2_CS_6_DDR4<<32)|(MR_1_CS_6_DDR4<<16)|(MR_0_CS_6_DDR4)

 MC1_CTL_1E8_DATA : .dword (MR_6_CS_6_DDR4<<32)|(MR_5_CS_6_DDR4<<16)|(MR_4_CS_6_DDR4)

 MC1_CTL_1F0_DATA : .dword (MR_3_CS_7_DDR4<<48)|(MR_2_CS_7_DDR4<<32)|(MR_1_CS_7_DDR4<<16)|(MR_0_CS_7_DDR4)

 MC1_CTL_1F8_DATA : .dword (MR_6_CS_7_DDR4<<32)|(MR_5_CS_7_DDR4<<16)|(MR_4_CS_7_DDR4)
                                         
 MC1_CTL_200_DATA : .dword 0x0000010003140400
// MC1_CTL_200_DATA : .dword 0x0000010003000000
// MC1_CTL_200_DATA : .dword 0x0000010301000000
 MC1_CTL_208_DATA : .dword 0x00000000002000aa
                                         
 MC1_CTL_210_DATA : .dword 0x0000000000000000
 MC1_CTL_218_DATA : .dword 0x0000000000000000  #useless
                                         
 MC1_CTL_220_DATA : .dword 0x0fff0000fff00000
 MC1_CTL_228_DATA : .dword 0x0000000000000000  #useless
                                         
 MC1_CTL_230_DATA : .dword 0x0000030300010202   //2G UDIMM MICRON
// MC1_CTL_230_DATA : .dword 0x0000030300010102   //2G UDIMM MICRON
 //MC1_CTL_230_DATA : .dword 0x0000000000000000   //2G UDIMM MICRON
// MC1_CTL_230_DATA : .dword 0x0000020300010302   
// MC1_CTL_230_DATA : .dword 0x0000030301010302 //2G
 MC1_CTL_238_DATA : .dword 0x000000ff00000000
                                         
 MC1_CTL_240_DATA : .dword 0x0f28100001010101
 MC1_CTL_248_DATA : .dword 0x0000000000ff010f  #useless
                                         
 MC1_CTL_250_DATA : .dword 0x0000000000000080
 MC1_CTL_258_DATA : .dword 0x0055004400330022
                                         
 MC1_CTL_260_DATA : .dword 0x0000000000000000
 MC1_CTL_268_DATA : .dword 0x0000000000000000

 MC1_CTL_270_DATA : .dword 0x0000000000000000
 MC1_CTL_278_DATA : .dword 0x0000000000000000  #useless
                                         
 //MC1_CTL_280_DATA : .dword 0x0000000300000000//ecc enable
 MC1_CTL_280_DATA : .dword 0x0000000000000000
 MC1_CTL_288_DATA : .dword 0x0000000000000000  #useless
                                         
 MC1_CTL_290_DATA : .dword 0x0000000000000000  #useless
 MC1_CTL_298_DATA : .dword 0x0000000000000000  #useless

 MC1_CTL_2A0_DATA : .dword 0x0000000000000000
 MC1_CTL_2A8_DATA : .dword 0x0000000000000000  #useless
                                         
 MC1_CTL_2B0_DATA : .dword 0x0000000000000000  #useless
 MC1_CTL_2B8_DATA : .dword 0x0000000000000000  #useless

 MC1_CTL_2C0_DATA : .dword 0x0000000000000000  #useless
 MC1_CTL_2C8_DATA : .dword 0x0000000000000000  #useless

 MC1_CTL_2D0_DATA : .dword 0x0000000000000000  #useless
 MC1_CTL_2D8_DATA : .dword 0x0000000000000000  #useless

 MC1_CTL_2E0_DATA : .dword 0x0000000000000000  #useless
 MC1_CTL_2E8_DATA : .dword 0x0000000000000000  #useless

 MC1_CTL_2F0_DATA : .dword 0x0000000000000000  #useless
 MC1_CTL_2F8_DATA : .dword 0x0000000000000000  #useless

 MC1_CTL_300_DATA : .dword 0x0000000000000700
 MC1_CTL_308_DATA : .dword 0x0000000000000000
                                         
 MC1_CTL_310_DATA : .dword 0x0000000000000000  #useless
 MC1_CTL_318_DATA : .dword 0x0000000000000000  #useless

 MC1_CTL_320_DATA : .dword 0x0000000000000000  #useless
 MC1_CTL_328_DATA : .dword 0x0000000000000000  #useless

 MC1_CTL_330_DATA : .dword 0x0000000000000000  #useless
 MC1_CTL_338_DATA : .dword 0x0000000000000000  #useless

 MC1_CTL_340_DATA : .dword 0x0000000000000000
 MC1_CTL_348_DATA : .dword 0x0000000000000000  #useless
                                         
 MC1_CTL_350_DATA : .dword 0x0018001400140010
 MC1_CTL_358_DATA : .dword 0x0000000000000008
                                         
 MC1_CTL_360_DATA : .dword 0x0000000000000000  #useless
 MC1_CTL_368_DATA : .dword 0x0000000000000000  #useless

 MC1_CTL_370_DATA : .dword 0x0000000000000000  #useless
 MC1_CTL_378_DATA : .dword 0x0000000000000000  #useless

 MC1_CTL_380_DATA : .dword 0x000000000000000f
 MC1_CTL_388_DATA : .dword 0x0000000000000001

 MC1_CTL_390_DATA : .dword 0x0000000000000000  #useless
 MC1_CTL_398_DATA : .dword 0x0000000000000000  #useless
                                         
 MC1_CTL_3A0_DATA : .dword 0x0000000000000000  #useless
 MC1_CTL_3A8_DATA : .dword 0x0000000000000000  #useless
                                         
 MC1_CTL_3B0_DATA : .dword 0x0000000000000000  #useless
 MC1_CTL_3B8_DATA : .dword 0x0000000000000000  #useless
                                         
 MC1_CTL_3C0_DATA : .dword 0x8040201081248124
// MC1_CTL_3C0_DATA : .dword 0x0000000000000000
 MC1_CTL_3C8_DATA : .dword 0x0000000000000600
                                         
 MC1_CTL_3D0_DATA : .dword 0x0000000000000000
 MC1_CTL_3D8_DATA : .dword 0x0000000000000000
                                         
 MC1_CTL_3E0_DATA : .dword 0x0000000000000000  #useless
 MC1_CTL_3E8_DATA : .dword 0x0000000000000000  #useless
                                         
 MC1_CTL_3F0_DATA : .dword 0x0000000000000000  #useless
 MC1_CTL_3F8_DATA : .dword 0x0000000000000000  #useless
 MC1_CTL_400_DATA : .dword 0x0000000824083010
 MC1_CTL_408_DATA : .dword 0x0000000000000000  #useless
 MC1_CTL_410_DATA : .dword 0x0000000000000000  #useless
 MC1_CTL_418_DATA : .dword 0x0000000000000000  #useless
 MC1_CTL_420_DATA : .dword 0x0000000000000000  #useless
 MC1_CTL_428_DATA : .dword 0x0000000000000000  #useless
 MC1_CTL_430_DATA : .dword 0x0000000000000000  #useless
 MC1_CTL_438_DATA : .dword 0x0000000000000000  #useless
 MC1_CTL_440_DATA : .dword 0x0000000000010307
 MC1_CTL_448_DATA : .dword 0x0000000000000000
 MC1_CTL_450_DATA : .dword 0x0000000000000000
 MC1_CTL_458_DATA : .dword 0x0000000000000000
 MC1_CTL_460_DATA : .dword 0x0000000000000000
 MC1_CTL_468_DATA : .dword 0x0000000000000000
 MC1_CTL_470_DATA : .dword 0x0000000000000000
 MC1_CTL_478_DATA : .dword 0x0000000000000000  #useless
                    .dword 0x0:16
 MC1_CTL_500_DATA : .dword 0x0000000000000000
 MC1_CTL_508_DATA : .dword 0x0000050000000000
 MC1_CTL_510_DATA : .dword 0x0000000000000000
 MC1_CTL_518_DATA : .dword 0x0000000000000000
 MC1_CTL_520_DATA : .dword 0x0000000000000000
 MC1_CTL_528_DATA : .dword 0x0000000000000000
 MC1_CTL_530_DATA : .dword 0x0000000000000000
 MC1_CTL_538_DATA : .dword 0x0000000000000000
 MC1_CTL_540_DATA : .dword 0x0000000000000000  #useless
 MC1_CTL_548_DATA : .dword 0x0000000000000000  #useless
 MC1_CTL_550_DATA : .dword 0x0000000000000000  #useless
 MC1_CTL_558_DATA : .dword 0x0000000000000000  #useless
 MC1_CTL_560_DATA : .dword 0x0000000000000000  #useless
 MC1_CTL_568_DATA : .dword 0x0000000000000000  #useless
 MC1_CTL_570_DATA : .dword 0x0000000000000000  #useless
 MC1_CTL_578_DATA : .dword 0x0000000000000000  #useless
 MC1_CTL_580_DATA : .dword 0xfffffffff0000000
 MC1_CTL_588_DATA : .dword 0xffffff0000000000
 MC1_CTL_590_DATA : .dword 0x0000000000000000
 MC1_CTL_598_DATA : .dword 0x0000000000000000
 MC1_CTL_5A0_DATA : .dword 0x0000000000000000
 MC1_CTL_5A8_DATA : .dword 0x0000000000000000
 MC1_CTL_5B0_DATA : .dword 0x0000000000000000
 MC1_CTL_5B8_DATA : .dword 0x0000000000000000
 MC1_CTL_5C0_DATA : .dword 0x0000000000000000  #useless
 MC1_CTL_5C8_DATA : .dword 0x0000000000000000  #useless
 MC1_CTL_5D0_DATA : .dword 0x0000000000000000  #useless
 MC1_CTL_5D8_DATA : .dword 0x0000000000000000  #useless
 MC1_CTL_5E0_DATA : .dword 0x0000000000000000  #useless
 MC1_CTL_5E8_DATA : .dword 0x0000000000000000  #useless
 MC1_CTL_5F0_DATA : .dword 0x0000000000000000  #useless
 MC1_CTL_5F8_DATA : .dword 0x0000000000000000  #useless
 MC1_CTL_600_DATA : .dword 0x00000000000000f0
 MC1_CTL_608_DATA : .dword 0x00000000000000f0
 MC1_CTL_610_DATA : .dword 0x0000000080000000
 MC1_CTL_618_DATA : .dword 0x0000000000000000
 MC1_CTL_620_DATA : .dword 0x0000000000000000
 MC1_CTL_628_DATA : .dword 0x0000000000000000
 MC1_CTL_630_DATA : .dword 0x0000000000000000
 MC1_CTL_638_DATA : .dword 0x0000000000000000
                    .dword 0x0:24
 MC1_CTL_700_DATA : .dword 0x0000000000000000
 MC1_CTL_708_DATA : .dword 0xaaaaaaaafffffffe
 MC1_CTL_710_DATA : .dword 0x0000000000000000  #useless
 MC1_CTL_718_DATA : .dword 0x0000000000000000  #useless
 MC1_CTL_720_DATA : .dword 0x0000000000000000
 MC1_CTL_728_DATA : .dword 0x0000000000000000  #useless
 MC1_CTL_730_DATA : .dword 0x0000000000000000
 MC1_CTL_738_DATA : .dword 0x0000000000000000  #useless


