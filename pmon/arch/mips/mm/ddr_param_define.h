/*****************************
    Macro defination for DDR MC parameters
    Author: Chen Xinke
    v0.1    
*******************************/
#ifdef  loongson3A3
#define LSMCD3_2
#else
#ifdef  LS3B
#define LSMCD3_2
#else
#ifdef  LS2HMC
#define LSMCD3_2
#endif
#endif
#endif

#define MAX_ROW_SIZE            15

#define DDR_MC_CONFIG_BASE      0x900000000ff00000
#define MC_CONFIG_REG_BASE_ADDR 0x900000000ff00000
#ifdef  LSMCD3_2
#define DDR_PARAM_NUM   180
#else
#define DDR_PARAM_NUM   152
#endif

#define TRFC_MARGIN             0x8
#define MC_RESYNC_DLL_ADDR      (0x980)
#define MC_RESYNC_DLL_OFFSET    16
#define SREFRESH_ADDR           (0x30)
#define SREFRESH_OFFSET         32
#define START_ADDR              (0x30)
#define START_OFFSET            40
#define DLLLOCKREG_ADDR         (0x10)
#define DLLLOCKREG_OFFSET       0
#define INIT_COMPLETE_OFFSET    8
#define MC_INT_STATUS_ADDR      (0x960)
#define MC_INT_STATUS_OFFSET    0
#define MC_INT_ACK_ADDR         (0x970)
#define MC_INT_ACK_OFFSET       24
#define MC_INT_ACK_CLEAR_VALUE  0x3ffff
#define MC_INT_MASK_ADDR        (0x950)
#define MC_INT_MASK_OFFSET      24
#define MC_INT_MASK_VALUE       0x3ffcf

#define PWRUP_SREFRESH_EXIT_ADDR    (0x30)
#define PWRUP_SREFRESH_EXIT_OFFSET  0
#define TRST_PWRON_ADDR         (0x8e0)
#define TRST_PWRON_OFFSET       0
#define TRST_PWRON_MASK         0xffffffff
#define TCKE_INACTIVE_ADDR      (0x8d0)
#define TCKE_INACTIVE_OFFSET    24
#define TCKE_INACTIVE_MASK      0xffffffff
#define TFAW_ADDR               (0x0a0)
#define TFAW_OFFSET             0
#define TFAW_MASK               0x3f
#define TRFC_ADDR               (0x0c0)
#define TRFC_OFFSET             40
#define TRFC_MASK               0xff
#define TXSNR_ADDR              (0x140)
#define TXSNR_OFFSET            0
#define TXSNR_MASK              0xffff
#define TREF_ADDR               (0x110)
#define TREF_OFFSET             0
#define TREF_MASK               0x3fff

#define ODT_MAP_CS_ADDR          (0x080)
#define ADDRESS_MIRROR_ADDR      (0x760)
#define ADDRESS_MIRROR_OFFSET    32

#define CLKLVL_DELAY_2_ADDR      (0x8f0)
#define CLKLVL_DELAY_1_ADDR      (0x8f0)
#define CLKLVL_DELAY_0_ADDR      (0x8f0)
#define CLKLVL_DELAY_2_OFFSET    24
#define CLKLVL_DELAY_1_OFFSET    16
#define CLKLVL_DELAY_0_OFFSET    8

#define PAD_CTRL_REG_ADDR             (0x2d0)
#define PAD_CTRL_CLK_OFFSET           7
#define PAD_CTRL_CLK_MASK             (0x3)
#define PAD_CTRL_COMP_OFFSET          18
#define PAD_CTRL_COMP_MASK            (0xff)

#define PAD_OUTPUT_WINDOW_8_ADDR      (0x310)
#define PAD_OUTPUT_WINDOW_7_ADDR      (0x310)
#define PAD_OUTPUT_WINDOW_6_ADDR      (0x300)
#define PAD_OUTPUT_WINDOW_5_ADDR      (0x300)
#define PAD_OUTPUT_WINDOW_4_ADDR      (0x2f0)
#define PAD_OUTPUT_WINDOW_3_ADDR      (0x2f0)
#define PAD_OUTPUT_WINDOW_2_ADDR      (0x2e0)
#define PAD_OUTPUT_WINDOW_1_ADDR      (0x2e0)
#define PAD_OUTPUT_WINDOW_0_ADDR      (0x2d0)
#define PAD_OUTPUT_WINDOW_8_OFFSET    32
#define PAD_OUTPUT_WINDOW_7_OFFSET    0
#define PAD_OUTPUT_WINDOW_6_OFFSET    32
#define PAD_OUTPUT_WINDOW_5_OFFSET    0
#define PAD_OUTPUT_WINDOW_4_OFFSET    32
#define PAD_OUTPUT_WINDOW_3_OFFSET    0
#define PAD_OUTPUT_WINDOW_2_OFFSET    32
#define PAD_OUTPUT_WINDOW_1_OFFSET    0
#define PAD_OUTPUT_WINDOW_0_OFFSET    32
#define PAD_OUTPUT_WINDOW_MASK     (0xffff)

#define PHY_CTRL_0_8_ADDR        (0x310)
#define PHY_CTRL_0_7_ADDR        (0x310)
#define PHY_CTRL_0_6_ADDR        (0x300)
#define PHY_CTRL_0_5_ADDR        (0x300)
#define PHY_CTRL_0_4_ADDR        (0x2f0)
#define PHY_CTRL_0_3_ADDR        (0x2f0)
#define PHY_CTRL_0_2_ADDR        (0x2e0)
#define PHY_CTRL_0_1_ADDR        (0x2e0)
#define PHY_CTRL_0_0_ADDR        (0x2d0)
#define PHY_CTRL_0_8_OFFSET      32
#define PHY_CTRL_0_7_OFFSET      0
#define PHY_CTRL_0_6_OFFSET      32
#define PHY_CTRL_0_5_OFFSET      0
#define PHY_CTRL_0_4_OFFSET      32
#define PHY_CTRL_0_3_OFFSET      0
#define PHY_CTRL_0_2_OFFSET      32
#define PHY_CTRL_0_1_OFFSET      0
#define PHY_CTRL_0_0_OFFSET      32
#define PHY_CTRL_0_MASK          (0xffffffff)
#define PHY_CTRL_0_PREAMBLE_SHIFT 18
#define ADD_HALF_CLK_SHIFT       17
#define PHY_CTRL_0_ADDWLDLY_SHIFT 16
#define PHY_CTRL_0_ADJ_MASK      (0xf)
#define DQS_OUT_WINDOW_MASK      (0xff)
#define DQS_OUT_WINDOW_SHIFT     8
#define DQSDQ_OUT_WINDOW_MASK    (0xffff)
#define DQSDQ_OUT_WINDOW_SHIFT   0
#define PHY_CTRL_0_POP_DELAY_SHIFT  24
#define PHY_CTRL_0_POP_DELAY_MASK   (0x7)

#define PHY_CTRL_1_8_ADDR        (0x360)
#define PHY_CTRL_1_7_ADDR        (0x350)
#define PHY_CTRL_1_6_ADDR        (0x350)
#define PHY_CTRL_1_5_ADDR        (0x340)
#define PHY_CTRL_1_4_ADDR        (0x340)
#define PHY_CTRL_1_3_ADDR        (0x330)
#define PHY_CTRL_1_2_ADDR        (0x330)
#define PHY_CTRL_1_1_ADDR        (0x320)
#define PHY_CTRL_1_0_ADDR        (0x320)
#define PHY_CTRL_1_8_OFFSET      0
#define PHY_CTRL_1_7_OFFSET      32
#define PHY_CTRL_1_6_OFFSET      0
#define PHY_CTRL_1_5_OFFSET      32
#define PHY_CTRL_1_4_OFFSET      0
#define PHY_CTRL_1_3_OFFSET      32
#define PHY_CTRL_1_2_OFFSET      0
#define PHY_CTRL_1_1_OFFSET      32
#define PHY_CTRL_1_0_OFFSET      0 
#define PHY_CTRL_1_MASK          (0xffffffff)
#define PHY_CTRL_1_GATE_MASK     (0xfff)
#define PHY_CTRL_1_RD_ODT_SHIFT  24
#define PHY_CTRL_1_RD_ODT_MASK   (0xff)

#define PHY_CTRL_2_ADDR          (0x360)
#define PHY_CTRL_2_OFFSET        32 
#define PHY_CTRL_2_POP_DELAY_MASK   (0xf)
#define PHY_CTRL_2_POP_DELAY_SHIFT 13
#define RESET_GFIFO_SHIFT          26

#define WRLVL_DQ_DELAY_8_ADDR      (0x230)
#define WRLVL_DQ_DELAY_7_ADDR      (0x230)
#define WRLVL_DQ_DELAY_6_ADDR      (0x220)
#define WRLVL_DQ_DELAY_5_ADDR      (0x220)
#define WRLVL_DQ_DELAY_4_ADDR      (0x210)
#define WRLVL_DQ_DELAY_3_ADDR      (0x210)
#define WRLVL_DQ_DELAY_2_ADDR      (0x200)
#define WRLVL_DQ_DELAY_1_ADDR      (0x200)
#define WRLVL_DQ_DELAY_0_ADDR      (0x1f0)
#define WRLVL_DQ_DELAY_8_OFFSET    48
#define WRLVL_DQ_DELAY_7_OFFSET    16
#define WRLVL_DQ_DELAY_6_OFFSET    48
#define WRLVL_DQ_DELAY_5_OFFSET    16
#define WRLVL_DQ_DELAY_4_OFFSET    48
#define WRLVL_DQ_DELAY_3_OFFSET    16
#define WRLVL_DQ_DELAY_2_OFFSET    48
#define WRLVL_DQ_DELAY_1_OFFSET    16
#define WRLVL_DQ_DELAY_0_OFFSET    48

#define RDLVL_DQSN_DELAY_8_ADDR      (0x280)
#define RDLVL_DQSN_DELAY_7_ADDR      (0x270)
#define RDLVL_DQSN_DELAY_6_ADDR      (0x270)
#define RDLVL_DQSN_DELAY_5_ADDR      (0x260)
#define RDLVL_DQSN_DELAY_4_ADDR      (0x260)
#define RDLVL_DQSN_DELAY_3_ADDR      (0x250)
#define RDLVL_DQSN_DELAY_2_ADDR      (0x250)
#define RDLVL_DQSN_DELAY_1_ADDR      (0x240)
#define RDLVL_DQSN_DELAY_0_ADDR      (0x240)
#define RDLVL_DQSN_DELAY_8_OFFSET    8
#define RDLVL_DQSN_DELAY_7_OFFSET    40
#define RDLVL_DQSN_DELAY_6_OFFSET    8
#define RDLVL_DQSN_DELAY_5_OFFSET    40
#define RDLVL_DQSN_DELAY_4_OFFSET    8
#define RDLVL_DQSN_DELAY_3_OFFSET    40
#define RDLVL_DQSN_DELAY_2_OFFSET    8
#define RDLVL_DQSN_DELAY_1_OFFSET    40
#define RDLVL_DQSN_DELAY_0_OFFSET    8

#ifdef  LSMCD3_2
#define MR2_DATA_0_ADDR         (0x9e0)
#define MR2_DATA_0_OFFSET       48
#define MR2_DATA_1_ADDR         (0x9f0)
#define MR2_DATA_1_OFFSET       0
#define MR2_DATA_2_ADDR         (0x9f0)
#define MR2_DATA_2_OFFSET       16
#define MR2_DATA_3_ADDR         (0x9f0)
#define MR2_DATA_3_OFFSET       32

#define CLKLVL_DELAY_MASK       (0xff)
#define RDLVL_GATE_DELAY_MASK   (0xffff)
#define RDLVL_DELAY_MASK        (0xffff)
#define RDLVL_DQSN_DELAY_MASK   (0xffff)
#define WRLVL_DELAY_MASK        (0xffff)
#define WRLVL_DQ_DELAY_MASK     (0xffff)

#define WRLVL_DELAY_8_ADDR      (0xb10)
#define WRLVL_DELAY_7_ADDR      (0xb10)
#define WRLVL_DELAY_6_ADDR      (0xb10)
#define WRLVL_DELAY_5_ADDR      (0xb00)
#define WRLVL_DELAY_4_ADDR      (0xb00)
#define WRLVL_DELAY_3_ADDR      (0xb00)
#define WRLVL_DELAY_2_ADDR      (0xb00)
#define WRLVL_DELAY_1_ADDR      (0xaf0)
#define WRLVL_DELAY_0_ADDR      (0xaf0)
#define WRLVL_DELAY_8_OFFSET    32
#define WRLVL_DELAY_7_OFFSET    16
#define WRLVL_DELAY_6_OFFSET    0
#define WRLVL_DELAY_5_OFFSET    48
#define WRLVL_DELAY_4_OFFSET    32
#define WRLVL_DELAY_3_OFFSET    16
#define WRLVL_DELAY_2_OFFSET    0
#define WRLVL_DELAY_1_OFFSET    48
#define WRLVL_DELAY_0_OFFSET    32

#define RDLVL_GATE_DELAY_8_ADDR (0xa90)
#define RDLVL_GATE_DELAY_7_ADDR (0xa90)
#define RDLVL_GATE_DELAY_6_ADDR (0xa90)
#define RDLVL_GATE_DELAY_5_ADDR (0xa90)
#define RDLVL_GATE_DELAY_4_ADDR (0xa80)
#define RDLVL_GATE_DELAY_3_ADDR (0xa80)
#define RDLVL_GATE_DELAY_2_ADDR (0xa80)
#define RDLVL_GATE_DELAY_1_ADDR (0xa80)
#define RDLVL_GATE_DELAY_0_ADDR (0xa70)
#define RDLVL_GATE_DELAY_8_OFFSET    48
#define RDLVL_GATE_DELAY_7_OFFSET    32
#define RDLVL_GATE_DELAY_6_OFFSET    16
#define RDLVL_GATE_DELAY_5_OFFSET    0
#define RDLVL_GATE_DELAY_4_OFFSET    48
#define RDLVL_GATE_DELAY_3_OFFSET    32
#define RDLVL_GATE_DELAY_2_OFFSET    16
#define RDLVL_GATE_DELAY_1_OFFSET    0
#define RDLVL_GATE_DELAY_0_OFFSET    48

#define RDLVL_DELAY_8_ADDR      (0xa50)
#define RDLVL_DELAY_7_ADDR      (0xa50)
#define RDLVL_DELAY_6_ADDR      (0xa40)
#define RDLVL_DELAY_5_ADDR      (0xa40)
#define RDLVL_DELAY_4_ADDR      (0xa40)
#define RDLVL_DELAY_3_ADDR      (0xa40)
#define RDLVL_DELAY_2_ADDR      (0xa30)
#define RDLVL_DELAY_1_ADDR      (0xa30)
#define RDLVL_DELAY_0_ADDR      (0xa30)
#define RDLVL_DELAY_8_OFFSET    16
#define RDLVL_DELAY_7_OFFSET    0
#define RDLVL_DELAY_6_OFFSET    48
#define RDLVL_DELAY_5_OFFSET    32
#define RDLVL_DELAY_4_OFFSET    16
#define RDLVL_DELAY_3_OFFSET    0
#define RDLVL_DELAY_2_OFFSET    48
#define RDLVL_DELAY_1_OFFSET    32
#define RDLVL_DELAY_0_OFFSET    16
#else
#define MR2_DATA_0_ADDR         (0x110)
#define MR2_DATA_0_OFFSET       32
#define MR2_DATA_1_ADDR         (0x110)
#define MR2_DATA_1_OFFSET       48
#define MR2_DATA_2_ADDR         (0x120)
#define MR2_DATA_2_OFFSET       0
#define MR2_DATA_3_ADDR         (0x120)
#define MR2_DATA_3_OFFSET       16

#define CLKLVL_DELAY_MASK       (0xff)
#define RDLVL_GATE_DELAY_MASK   (0xff)
#define RDLVL_DELAY_MASK        (0xff)
#define RDLVL_DQSN_DELAY_MASK   (0xff)
#define WRLVL_DELAY_MASK        (0xff)
#define WRLVL_DQ_DELAY_MASK     (0xff)

#define WRLVL_DELAY_8_ADDR      (0x840)
#define WRLVL_DELAY_7_ADDR      (0x840)
#define WRLVL_DELAY_6_ADDR      (0x840)
#define WRLVL_DELAY_5_ADDR      (0x830)
#define WRLVL_DELAY_4_ADDR      (0x830)
#define WRLVL_DELAY_3_ADDR      (0x830)
#define WRLVL_DELAY_2_ADDR      (0x830)
#define WRLVL_DELAY_1_ADDR      (0x830)
#define WRLVL_DELAY_0_ADDR      (0x830)
#define WRLVL_DELAY_8_OFFSET    16
#define WRLVL_DELAY_7_OFFSET    8
#define WRLVL_DELAY_6_OFFSET    0
#define WRLVL_DELAY_5_OFFSET    56
#define WRLVL_DELAY_4_OFFSET    48
#define WRLVL_DELAY_3_OFFSET    40
#define WRLVL_DELAY_2_OFFSET    32
#define WRLVL_DELAY_1_OFFSET    24
#define WRLVL_DELAY_0_OFFSET    16

#define RDLVL_GATE_DELAY_8_ADDR     (0x7f0)
#define RDLVL_GATE_DELAY_7_ADDR     (0x7f0)
#define RDLVL_GATE_DELAY_6_ADDR     (0x7f0)
#define RDLVL_GATE_DELAY_5_ADDR     (0x7f0)
#define RDLVL_GATE_DELAY_4_ADDR     (0x7f0)
#define RDLVL_GATE_DELAY_3_ADDR     (0x7f0)
#define RDLVL_GATE_DELAY_2_ADDR     (0x7e0)
#define RDLVL_GATE_DELAY_1_ADDR     (0x7e0)
#define RDLVL_GATE_DELAY_0_ADDR     (0x7e0)
#define RDLVL_GATE_DELAY_8_OFFSET    40
#define RDLVL_GATE_DELAY_7_OFFSET    32
#define RDLVL_GATE_DELAY_6_OFFSET    24
#define RDLVL_GATE_DELAY_5_OFFSET    16
#define RDLVL_GATE_DELAY_4_OFFSET    8
#define RDLVL_GATE_DELAY_3_OFFSET    0
#define RDLVL_GATE_DELAY_2_OFFSET    56
#define RDLVL_GATE_DELAY_1_OFFSET    48
#define RDLVL_GATE_DELAY_0_OFFSET    40

#define RDLVL_DELAY_8_ADDR      (0x230)
#define RDLVL_DELAY_7_ADDR      (0x230)
#define RDLVL_DELAY_6_ADDR      (0x220)
#define RDLVL_DELAY_5_ADDR      (0x220)
#define RDLVL_DELAY_4_ADDR      (0x210)
#define RDLVL_DELAY_3_ADDR      (0x210)
#define RDLVL_DELAY_2_ADDR      (0x200)
#define RDLVL_DELAY_1_ADDR      (0x200)
#define RDLVL_DELAY_0_ADDR      (0x1f0)
#define RDLVL_DELAY_8_OFFSET    40
#define RDLVL_DELAY_7_OFFSET    8
#define RDLVL_DELAY_6_OFFSET    40
#define RDLVL_DELAY_5_OFFSET    8
#define RDLVL_DELAY_4_OFFSET    40
#define RDLVL_DELAY_3_OFFSET    8
#define RDLVL_DELAY_2_OFFSET    40
#define RDLVL_DELAY_1_OFFSET    8
#define RDLVL_DELAY_0_OFFSET    40
#endif

//------------------------
//define for ddr configure register param location
#define EIGHT_BANK_MODE_ADDR     (0x10)
#define COLUMN_SIZE_ADDR         (0x50)
#define ADDR_PINS_ADDR           (0x50)
#define CS_MAP_ADDR              (0x70)
#define REDUC_ADDR               (0x30)
#define CTRL_RAW_ADDR            (0x40)
#define ECC_DISABLE_W_UC_ERR_ADDR   (0x10)
#define EIGHT_BANK_MODE_OFFSET  32
#define COLUMN_SIZE_OFFSET      24
#define ADDR_PINS_OFFSET        8
#define CS_MAP_OFFSET           16
#define REDUC_OFFSET            8
#define CTRL_RAW_OFFSET         48 
#define ECC_DISABLE_W_UC_ERR_OFFSET 24
//------------------------

//------------------------------------
//for ddr3_level.S
#define SWLVL_RESP_8_ADDR       (0x7b0)
#define SWLVL_RESP_7_ADDR       (0x7b0)
#define SWLVL_RESP_6_ADDR       (0x7a0)
#define SWLVL_RESP_5_ADDR       (0x7a0)
#define SWLVL_RESP_4_ADDR       (0x7a0)
#define SWLVL_RESP_3_ADDR       (0x7a0)
#define SWLVL_RESP_2_ADDR       (0x7a0)
#define SWLVL_RESP_1_ADDR       (0x7a0)
#define SWLVL_RESP_0_ADDR       (0x7a0)

#define SWLVL_START_ADDR        (0x960)
#define SWLVL_START_OFFSET      40
#define SWLVL_LOAD_ADDR         (0x960)
#define SWLVL_LOAD_OFFSET       32
#define SWLVL_EXIT_ADDR         (0x960)
#define SWLVL_EXIT_OFFSET       24
#define SW_LEVELING_MODE_ADDR   (0x750)
#define SW_LEVELING_MODE_OFFSET 48
#define SWLVL_OP_DONE_ADDR      (0x720)
#define SWLVL_OP_DONE_OFFSET    8

#ifdef  LSMCD3_2
#define WRLVL_EN_ADDR           (0x980)
#define WRLVL_EN_OFFSET         32
#define WRLVL_REG_EN_ADDR       (0x980)
#define WRLVL_REG_EN_OFFSET     40
#define RDLVL_GATE_REG_EN_ADDR  (0x980)
#define RDLVL_GATE_REG_EN_OFFSET 0
#define RDLVL_REG_EN_ADDR       (0x980)
#define RDLVL_REG_EN_OFFSET     8
#else
#define WRLVL_EN_ADDR           (0x710)
#define WRLVL_EN_OFFSET         48
#endif
#define WRLVL_CS_ADDR           (0x750)
#define WRLVL_CS_OFFSET         56
#define RDLVL_GATE_EN_ADDR      (0x720)
#define RDLVL_GATE_EN_OFFSET    40
#define RDLVL_EN_ADDR           (0x720)
#define RDLVL_EN_OFFSET         32
#define RDLVL_CS_ADDR           (0x750)
#define RDLVL_CS_OFFSET         40
#define RDLVL_EDGE_ADDR         (0x940)
#define RDLVL_EDGE_OFFSET       24

//cmd extern interval
#define ONE_BYTE_MASK           0xff
#define THREE_BITS_MASK         0x7
#define FOUR_BITS_MASK          0xf

#define W2R_SAMECS_DLY_ADDR     (0x940)
#define W2R_SAMECS_DLY_OFFSET   48
#define W2R_DIFFCS_DLY_ADDR     (0x940)
#define W2R_DIFFCS_DLY_OFFSET   40
#define W2W_SAMECS_DLY_ADDR     (0x990)
#define W2W_SAMECS_DLY_OFFSET   56
#define W2W_DIFFCS_DLY_ADDR     (0x990)
#define W2W_DIFFCS_DLY_OFFSET   48
#define R2W_SAMECS_DLY_ADDR     (0x990)
#define R2W_SAMECS_DLY_OFFSET   32
#define R2W_DIFFCS_DLY_ADDR     (0x990)
#define R2W_DIFFCS_DLY_OFFSET   24
#define R2R_SAMECS_DLY_ADDR     (0x990)
#define R2R_SAMECS_DLY_OFFSET   16
#define R2R_DIFFCS_DLY_ADDR     (0x990)
#define R2R_DIFFCS_DLY_OFFSET   8

#define ADD_ODT_CLK_DTDC_ADDR   (0x9a0)
#define ADD_ODT_CLK_DTDC_OFFSET 0x24
#define ADD_ODT_CLK_STDC_ADDR   (0x9a0)
#define ADD_ODT_CLK_STDC_OFFSET 0x8
#define ADD_ODT_CLK_DTSC_ADDR   (0x9a0)
#define ADD_ODT_CLK_DTSC_OFFSET 0x0

//-------------------
//lowpower mode
#define LP_AUTO_EN_ADDR         (0x790)
#define LP_AUTO_EN_OFFSET       32
#define LP_CONTROL_ADDR         (0x790)
#define LP_CONTROL_OFFSET       40
