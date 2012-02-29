    .align  5
//One DIMM
N0_fix_ddr_reg_data:
N0_fix_ddr_reg_data_mc0:
N0_RDLVL_GATE_REG: .dword 0x0800000000000808
N0_RDLVL_DQSP_REG: .dword 0x161e1c16181c161a
N0_RDLVL_DQSN_REG: .dword 0x222222222020201e
N0_WRLVL_DQS__REG: .dword 0x2e302e302e2e2e2e
N0_WRLVL_DQ___REG: .dword 0x201e201e22202020

N1_fix_ddr_reg_data:
N0_fix_ddr_reg_data_mc1:
N1_RDLVL_GATE_REG: .dword 0x0808080808080808
N1_RDLVL_DQSP_REG: .dword 0x201e1a161a1e201e
N1_RDLVL_DQSN_REG: .dword 0x20222020201e201e
N1_WRLVL_DQS__REG: .dword 0x303032302e2e2e2e
N1_WRLVL_DQ___REG: .dword 0x1e1c1e1e20202020

//Two DIMM
N2_fix_ddr_reg_data:
N1_fix_ddr_reg_data_mc0:
N2_RDLVL_GATE_REG: .dword 0x0800000000000808
N2_RDLVL_DQSP_REG: .dword 0x161c1c16181c181c
N2_RDLVL_DQSN_REG: .dword 0x2222222220202020
N2_WRLVL_DQS__REG: .dword 0x2e3030302e2e2e32
N2_WRLVL_DQ___REG: .dword 0x201e1e1e22222220

N3_fix_ddr_reg_data:
N1_fix_ddr_reg_data_mc1:
N3_RDLVL_GATE_REG: .dword 0x0808080808080808
N3_RDLVL_DQSP_REG: .dword 0x1e1e1a181a1e2020
N3_RDLVL_DQSN_REG: .dword 0x1e222220201e2020
N3_WRLVL_DQS__REG: .dword 0x303032302e302e2e
N3_WRLVL_DQ___REG: .dword 0x1c1c1e1e22222020

N2_fix_ddr_reg_data_mc0:
N3_fix_ddr_reg_data_mc0:
    .align  5
