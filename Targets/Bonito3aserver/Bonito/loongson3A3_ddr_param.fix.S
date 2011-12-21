    .align  5
//One DIMM
N0_fix_ddr_reg_data:
N0_fix_ddr_reg_data_mc0:
N0_RDLVL_GATE_REG: .dword 0x0608080606080608
N0_RDLVL_DQSP_REG: .dword 0x1e1e201e20221c20
N0_RDLVL_DQSN_REG: .dword 0x222222222222201e
N0_WRLVL_DQS__REG: .dword 0x2e1a141416182e2e
N0_WRLVL_DQ___REG: .dword 0x201e1e1c22221e20

N1_fix_ddr_reg_data:
N0_fix_ddr_reg_data_mc1:
N1_RDLVL_GATE_REG: .dword 0x0c0a080a080a0a0e
N1_RDLVL_DQSP_REG: .dword 0x201c1c1c1a1c201c
N1_RDLVL_DQSN_REG: .dword 0x1e2020201e202020
N1_WRLVL_DQS__REG: .dword 0x2e2e2e1a2e2e302e
N1_WRLVL_DQ___REG: .dword 0x2020202020202220

//Two DIMM
N2_fix_ddr_reg_data:
N1_fix_ddr_reg_data_mc0:
N2_RDLVL_GATE_REG: .dword 0x080a080808060808
N2_RDLVL_DQSP_REG: .dword 0x1e202220221a1c20
N2_RDLVL_DQSN_REG: .dword 0x1e24242420222020
N2_WRLVL_DQS__REG: .dword 0x2e1a18141430302c
N2_WRLVL_DQ___REG: .dword 0x201e1e1e22222222

N3_fix_ddr_reg_data:
N1_fix_ddr_reg_data_mc1:
N3_RDLVL_GATE_REG: .dword 0x0e0c0a080c0c0e10
N3_RDLVL_DQSP_REG: .dword 0x1c1c1e1e1c1e1c1c
N3_RDLVL_DQSN_REG: .dword 0x1c1e20201c1a1c1c
N3_WRLVL_DQS__REG: .dword 0x2e2e2e2e302c2c2e
N3_WRLVL_DQ___REG: .dword 0x2020202022242422

N2_fix_ddr_reg_data_mc0:
N3_fix_ddr_reg_data_mc0:
    .align  5
