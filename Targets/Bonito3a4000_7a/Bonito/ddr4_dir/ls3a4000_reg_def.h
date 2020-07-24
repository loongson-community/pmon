//DDR REG OFFSET DEFINE
#define PHY_ADDRESS 0x0000
#define CTL_ADDRESS 0x1000
#define MON_ADDRESS 0x2000
#define TST_ADDRESS 0x3000
#define DDR_CFG_BASE 0xaff00000
#define DDR_CONFIG_DISABLE_OFFSET 0x9
#define CLKSEL_CKCA_OFFSET 0x48
#define CLKSEL_DS0_OFFSET 0x58
#define CLKSEL_DS1_OFFSET 0xd8
#define CLKSEL_DS2_OFFSET 0x158
#define CLKSEL_DS3_OFFSET 0x1d8
#define CLKSEL_DS4_OFFSET 0x258
#define CLKSEL_DS5_OFFSET 0x2d8
#define CLKSEL_DS6_OFFSET 0x358
#define CLKSEL_DS7_OFFSET 0x3d8
#define PLL_CTRL_CKCA_OFFSET 0x4c
#define PLL_CTRL_DS_0_OFFSET 0x5c
#define PLL_CTRL_DS_1_OFFSET 0x6c
#define PLL_CTRL_DS_2_OFFSET 0x7c
#define PLL_CTRL_DS_3_OFFSET 0x8c
#define PLL_CTRL_DS_4_OFFSET 0x9c
#define PLL_CTRL_DS_5_OFFSET 0xac
#define PLL_CTRL_DS_6_OFFSET 0xbc
#define PLL_CTRL_DS_7_OFFSET 0xcc
#define PLL_CTRL_DS_8_OFFSET 0xdc
#define DRAM_INIT            0x10
#define DLL_LOCK_MODE_OFFSET 0x34
#define DDR3_MODE_OFFSET 0xc

#define DDR4_SPD_REFERENCE_RAW_CARDS_OFFSET 0x82
#define DDR4_2T_OFFSET      0x1448

#define DDR4_DLL_WRDQ_OFFSET         0x0100
#define DDR4_DLL_WRDQ1_OFFSET        0x0180
#define DDR4_DLL_WRDQ2_OFFSET        0x0200
#define DDR4_DLL_WRDQ3_OFFSET        0x0280
#define DDR4_DLL_WRDQ4_OFFSET        0x0300
#define DDR4_DLL_WRDQ5_OFFSET        0x0380
#define DDR4_DLL_WRDQ6_OFFSET        0x0400
#define DDR4_DLL_WRDQ7_OFFSET        0x0480
#define DDR4_DLL_WRDQ8_OFFSET        0x0500
#define DLL_WRDQS_OFFSET       0x0101
#define DLL_1XGEN_OFFSET       0x0102
#define DLL_1XDLY_OFFSET       0x0103
#define DLL_RDDQS0_OFFSET      0x0108
#define DLL_RDDQS1_OFFSET      0x0109
#define DLL_GATE_OFFSET        0x010a
#define DQ_OE_CTRL_OFFSET      0x0110
#define DQS_OE_CTRL_OFFSET     0x0111
#define RDGATE_CTRL_OFFSET     0x0114
#define RDGATE_MODE_OFFSET     0x0115
#define RDGATE_LEN_OFFSET      0x0116
#define RDODT_CTRL_OFFSET      0x0117
#define RDDQS_PHASE_OFFSET     0x0118
#define RDEDGE_SEL_OFFSET      0x0119
#define DLY_2X_OFFSET          0x011a

#define RDQSP_BDLY00_OFFSET    0x0150
#define RDQSP_BDLY01_OFFSET    0x0151
#define RDQSP_BDLY02_OFFSET    0x0152
#define RDQSP_BDLY03_OFFSET    0x0153
#define RDQSP_BDLY04_OFFSET    0x0154
#define RDQSP_BDLY05_OFFSET    0x0155
#define RDQSP_BDLY06_OFFSET    0x0156
#define RDQSP_BDLY07_OFFSET    0x0157
#define RDQSP_BDLY08_OFFSET    0x0158

#define RDQSN_BDLY00_OFFSET    0x0160
#define RDQSN_BDLY01_OFFSET    0x0161
#define RDQSN_BDLY02_OFFSET    0x0162
#define RDQSN_BDLY03_OFFSET    0x0163
#define RDQSN_BDLY04_OFFSET    0x0164
#define RDQSN_BDLY05_OFFSET    0x0165
#define RDQSN_BDLY06_OFFSET    0x0166
#define RDQSN_BDLY07_OFFSET    0x0167
#define RDQSN_BDLY08_OFFSET    0x0168

#define WRDQ_BDLY00_OFFSET     0x0120
#define WRDQS0_BDLY_OFFSET     0x012b

#define LVL_MODE_OFFSET        0x0700
#define LVL_REQ_OFFSET         0x0701
#define LVL_DLY_OFFSET         0x0702
#define LVL_CS_OFFSET          0x0703
#define LVL_RDY_OFFSET         0x0708
#define LVL_DONE_OFFSET        0x0709

#define LVL_RESP_OFFSET        0x0710
#define DLL_CTRL               0x908
#define DLL_OUT                0x90c

#define TPHY_RDDATA_OFFSET     0x1062
#define DDR4_TPHY_WRLAT_OFFSET 0x1065

#define CMD_OFFSET        0x1120
#define CMD_REQ_OFFSET    0x1121
#define CMD_STATUS_OFFSET 0x1122
#define CMD_CMD_OFFSET    0x1123
#define CMD_CS_OFFSET     0x1128
#define CMD_C_OFFSET      0x1129
#define CMD_BG_OFFSET     0x112a
#define CMD_BA_OFFSET     0x112b
#define CMD_A_OFFSET      0x112c
#define CMD_CKE_OFFSET    0x112f
#define CMD_PDA_OFFSET    0x1130
#define CMD_DQ0_OFFSET    0x1138
#define CMD_PREALL_OFFSET   0x1124
#define CMD_PREDONE_OFFSET  0x1125

#define DLL_VREF_OFFSET        0x00e0
#define VREF_DLY_OFFSET        0x00e1
#define VREF_NUM_OFFSET        0x00e2
#define VREF_SAMPLE_OFFSET     0x00e3
#define VREF_CTRL_DS0_OFFSET   0x0810
#define DS0_ODT_OFFSET         0x840

#define BASE_ADDR_OFFSET  0x1140
#define VALID_BITS_OFFSET 0x1148
#define VALID_BITS_ECC_OFFSET   0x115b
#define PAGE_SIZE_OFFSET  0x1150
#define PAGE_NUM_OFFSET   0x1154

#define DDR3_MR0_CS0_REG  0x1140
#define DDR3_MR1_CS0_REG  0x1142
#define DDR3_MR2_CS0_REG  0x1144
#define DDR3_MR3_CS0_REG  0x1146

#define DDR4_MR0_CS0_REG  0x1180
#define DDR4_MR1_CS0_REG  0x1182
#define DDR4_MR2_CS0_REG  0x1184
#define DDR4_MR3_CS0_REG  0x1186
#define DDR4_MR4_CS0_REG  0x1188
#define DDR4_MR5_CS0_REG  0x118a
#define DDR4_MR6_CS0_REG  0x118c

#define DDR4_MR0_CS1_REG  0x1190
#define DDR4_MR1_CS1_REG  0x1192
#define DDR4_MR2_CS1_REG  0x1194
#define DDR4_MR3_CS1_REG  0x1196
#define DDR4_MR4_CS1_REG  0x1198
#define DDR4_MR5_CS1_REG  0x119a
#define DDR4_MR6_CS1_REG  0x119c

#define DDR4_MR3_REG      0x1186
#define DDR4_MR5_REG      0x118a
#define DDR4_MR6_REG      0x118c
#define MR3_CS0_OFFSET         0x1184 
#define MR6_CS0_OFFSET         0x118c 
#define BIT_NUM    8
#define SAMPLE_NUM 128
#define N32bit    (SAMPLE_NUM/32)
#define ECC_ENABLE_BIT              0x1284
#define DDR4_CS_ENABLE_OFFSET 0x1100
#define DDR4_CS_MRS_OFFSET    0x1101
#define DDR4_CS_ZQ_OFFSET     0x1102
#define DDR4_CS_ZQCL_OFFSET   0x1103
#define DDR4_CS_RESYNC_OFFSET 0x1104
#define DDR4_CS_REF_OFFSET    0x1105
#define DDR4_CS_MAP_OFFSET    0x1108
#define DDR4_CKE_MAP_OFFSET   0x110c
#define DDR4_WRODT_MAP_OFFSET 0x13c0
#define DDR4_RDODT_MAP_OFFSET 0x13d0
#define DDR4_CHANNEL_WIDTH_OFFSET   0x1203

#define DDR4_COL_DIFF_OFFSET 0x1230
#define DDR4_ROW_DIFF_OFFSET 0x1231
#define DDR4_BA_DIFF_OFFSET  0x1232
#define DDR4_BG_DIFF_OFFSET  0x1233
#define DDR4_C_DIFF_OFFSET   0x1234
#define DDR4_CS_DIFF_OFFSET  0x1235
#define DDR4_ECC_EN_OFFSET   0x1284


#define RDDQS_DLL_NUM 32
#define RDQS_BDLY_NUM 16

#define DLL_VALUE_OFFSET 0x0036

//DDR REG VALUE DEFINE
#define SEL_DBL 0x0
#define PLL_BYPASS 0x0
#define PLL_LOOP   18  //9bit
#define PLL_DIVOUT 3   //7bit
#define PLL_DIVREF 3   //7bit
#define PLL_PD     0x0 //1bit
#define PLL_CG     0x1 //1bit
//#define PLL_CFG (PLL_CG<<25)|(PLL_PD<<24)|(PLL_DIVREF<<17)|(PLL_DIVOUT<<10)|(PLL_LOOP<<1)|(PLL_BYPASS)
#define PLL_CFG 0x02060c24

#define GET_SPD(shift)  \
    GET_NODE_ID_a0;\
    move a2, a0;\
    GET_I2C_ADDR_V1;\
    move        a0, a1;\
    dsll        a0, 1;\
    daddu       a0, 1;\
    or          a0, 0xa0;\
    dli         a1, shift;\
    bal         i2cread;\
    nop;\
    andi        v0, 0xff

#define GET_SPD_SLOT(shift,slotid)  \
    GET_NODE_ID_a0;\
    move a2, a0;\
    GET_I2C_ADDR_SLOT_V1(slotid);\
    move        a0, a1;\
    dsll        a0, 1;\
    daddu       a0, 1;\
    or          a0, 0xa0;\
    dli         a1, shift;\
    bal         i2cread;\
    nop;\
    andi        v0, 0xff

/*this macro used t0,t1,t5,t6,a0,a1,a2,a3,v0,v1,*/
#define TEST_MEMORY(node_id) \
	move	t0, msize; \
	and	t0, (0xff << (node_id * 8)); \
	beqz	t0, 11f; \
	nop; \
	dli     t0, 0xb800000000020000; \
	or	t0, (node_id << 44); \
	dli     a0, 0x6666666666666666; \
	sd      a0, 0x0(t0); \
	dli     a0, 0x5555555555555555; \
	sd      a0, 0x8(t0); \
	dli     a0, 0x4444444444444444; \
	sd      a0, 0x10(t0); \
	dli     a0, 0x3333333333333333; \
	sd      a0, 0x18(t0); \
	dli     a0, 0x2222222222222222; \
	sd      a0, 0x20(t0); \
	dli     a0, 0x1111111111111111; \
	sd      a0, 0x28(t0); \
	dli     a0, 0x0000000000000000; \
	sd      a0, 0x30(t0); \
	dli     a0, 0x8888888888888888; \
	sd      a0, 0x38(t0); \
	dli     t0, 0xb800000000020000 ^ 1<<MC_INTERLEAVE_OFFSET ; \
	or	t0, (node_id << 44); \
	dli     a0, 0x6666666666666666; \
	sd      a0, 0x0(t0); \
	dli     a0, 0x5555555555555555; \
	sd      a0, 0x8(t0); \
	dli     a0, 0x4444444444444444; \
	sd      a0, 0x10(t0); \
	dli     a0, 0x3333333333333333; \
	sd      a0, 0x18(t0); \
	dli     a0, 0x2222222222222222; \
	sd      a0, 0x20(t0); \
	dli     a0, 0x1111111111111111; \
	sd      a0, 0x28(t0); \
	dli     a0, 0x0000000000000000; \
	sd      a0, 0x30(t0); \
	dli     a0, 0x8888888888888888; \
	sd      a0, 0x38(t0); \
	/*test memory*/ \
	dli     t0, 0x9000000000000000; \
	or	t0, (node_id << 44); \
	dli     a0, 0x5555555555555555; \
	sd      a0, 0x0(t0); \
	dli     a0, 0xaaaaaaaaaaaaaaaa; \
	sd      a0, 0x8(t0); \
	dli     a0, 0x3333333333333333; \
	sd      a0, 0x10(t0); \
	dli     a0, 0xcccccccccccccccc; \
	sd      a0, 0x18(t0); \
	dli     a0, 0x7777777777777777; \
	sd      a0, 0x20(t0); \
	dli     a0, 0x8888888888888888; \
	sd      a0, 0x28(t0); \
	dli     a0, 0x1111111111111111; \
	sd      a0, 0x30(t0); \
	dli     a0, 0xeeeeeeeeeeeeeeee; \
	sd      a0, 0x38(t0); \
	PRINTSTR("The uncache data is:\r\n"); \
	dli     t1, 8; \
	move 	t5, t0; \
	b	1f; \
	nop; \
2: \
	PRINTSTR("The cached  data is:\r\n"); \
	dli     t1, 8; \
	dli     t5, 0x9800000000000000; \
	or	t5, (node_id << 44); \
1: \
	cache   0x13, 0x0(t5); \
	ld      t6, 0x0(t5); \
	move    a0, t5; \
	and     a0, a0, 0xfff; \
	bal     hexserial; \
	nop; \
	PRINTSTR(":  "); \
	dsrl    a0, t6, 32; \
	bal     hexserial; \
	nop; \
	move    a0, t6; \
	bal     hexserial; \
	nop; \
	PRINTSTR("\r\n"); \
	daddiu  t1, t1, -1; \
	daddiu  t5, t5, 8; \
	bnez    t1, 1b; \
	nop; \
	and	t5, 0x800000000000000; \
	beqz    t5, 2b; \
	nop; \
	/*check and reboot */ \
	dli	t0, 0x9800000000000000; \
	or	t0, (node_id << 44); \
	dli     a0, 0x5555555555555555; \
	ld      a1, 0x0(t0); \
	bne	a0, a1, 213f; \
	nop;	\
	dli     a0, 0xaaaaaaaaaaaaaaaa; \
	ld      a1, 0x8(t0); \
	bne	a0, a1, 213f; \
	nop;	\
	dli     a0, 0x3333333333333333; \
	ld      a1, 0x10(t0); \
	bne	a0, a1, 213f; \
	nop;	\
	dli     a0, 0xcccccccccccccccc; \
	ld      a1, 0x18(t0); \
	bne	a0, a1, 213f; \
	nop;	\
	dli     a0, 0x7777777777777777; \
	ld      a1, 0x20(t0); \
	bne	a0, a1, 213f; \
	nop;	\
	dli     a0, 0x8888888888888888; \
	ld      a1, 0x28(t0); \
	bne	a0, a1, 213f; \
	nop;	\
	dli     a0, 0x1111111111111111; \
	ld      a1, 0x30(t0); \
	bne	a0, a1, 213f; \
	nop;	\
	dli     a0, 0xeeeeeeeeeeeeeeee; \
	ld      a1, 0x38(t0); \
	beq	a0, a1, 11f; \
	nop;	\
213: \
	li	t0, 0xb00d0030; \
	li	t1, 1; \
	sw	t1, 0x0(t0); \
11:
	
/*
213: \
	b	213b; \
	nop; \
11:

*/
