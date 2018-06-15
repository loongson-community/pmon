//PCB DQ swap info, default no swap
#define WLVL_RESP_MAP 0x000000000
//(1+8)slice:ECC_B7_B6_B5_B4_B3_B2_B1_B0, each byte takes 4 bits(1 hex number)
//value = MC DQ offset that connected to DRAM DQ0 of each Byte
//for example, if PCB connect DRAM side DQ0 to CPU side DQ3, DRAM side DQ16 to CPU side DQ17,
//and others keep one to one, than you need modify this macro to 0x0_0000_0103

#define GET_LVL_BYTE_t2 \
    dsubu   a1, t2, t8; \
    dsubu   a1, a1, 0x180;

#define GET_WLVL_RESP_a0 \
    dsll    a1, a1, 2; \
    dli     a2, WLVL_RESP_MAP; \
    dsrl    a2, a2, a1; \
    and     a2, a2, 0x7; \
    dsrl    a0, a0, a2; \
	and 	a0, a0, 0x1;

#define GET_GLVL_RESP_a0 \
    and     a0, a0, 0x1;

#define GET_GLVL_RESP_2BIT_a0 \
    and     a0, a0, 0x3;

#define GET_NUMBER_OF_SLICES	\
    li      t0, 0x8;\
	lb		a0, 0x1f2(t8);\
	beq		a0, 0x3, 934f;\
	nop;\
	beq		a0, 0x7, 935f;\
	nop;\
	b		936f;\
	nop;\
934:;\
    li      t0, 0x2;\
	b		936f;\
	nop;\
935:;\
    li      t0, 0x4;\
	b		936f;\
	nop;\
936:;\
    dli     t1, 0x250;\
    or      t1, t1, t8;\
    lb      a0, 0x2(t1);\
    dli     t1, 0x1;\
    and     a0, a0, t1;\
    bne     a0, t1, 933f ;\
    nop;\
    daddu   t0, t0, 0x1;\
933:;
	
//    GET_NODE_ID_a0;\
//    dli     t5, 0x90000e0040000000;\
//    or      t5, t5, a0;\
//
#define PRINT_THE_MC_PARAM \
    dli     t4, DDR_PARAM_NUM;\
    move    t5, t8; \
1:;\
    ld      t6, 0x0(t5);\
    move    a0, t5;\
    and     a0, a0, 0xfff;\
    bal     hexserial;\
    nop;\
    PRINTSTR(":  ");\
    dsrl    a0, t6, 32;\
    bal     hexserial;\
    nop;\
    move    a0, t6;\
    bal     hexserial;\
    nop;\
    PRINTSTR("\r\n");\
    daddiu  t4, t4, -1;\
    daddiu  t5, t5, 8;\
    bnez    t4, 1b;\
    nop;

#define WRDQS_ADJUST_LOOP \
933:;\
    subu    t0, t0, 0x1;\
    beq     t0, 0x0, 936f;\
    nop;\
    daddu   t1, t1, 0x20;\
    lb      a0, OFFSET_DLL_WRDQS(t1);\
    bgeu    a0, a2, 933b;\
    nop;\
    bleu    a0, a3, 933b;\
    nop;\
    dli     t4, 0x8;\
    and     t4, t4, a0;\
    beqz    t4, 934f;\
    nop;\
    sb      a3, OFFSET_DLL_WRDQS(t1);\
    b       935f;\
    nop;\
934:;\
    sb      a2, OFFSET_DLL_WRDQS(t1);\
935:;\
	lb		a0, OFFSET_DLL_WRDQS(t1);\
	blt		a0, WRDQS_LTHF_STD, 937f;\
	nop;\
	li		t4, 0x0;\
	sb		t4, OFFSET_WRDQS_LTHF(t1);\
	b		938f;\
	nop;\
937:;\
	li		t4, 0x1;\
	sb		t4, OFFSET_WRDQS_LTHF(t1);\
938:;\
	dsubu	a0, a0, 0x20;\
	dli		t4, 0x7f;\
	and		a0, a0, t4;\
	sb		a0, OFFSET_DLL_WRDQ(t1);\
	blt		a0, WRDQ_LTHF_STD, 937f;\
	nop;\
	li		t4, 0x0;\
	sb		t4, OFFSET_WRDQ_LTHF(t1);\
	b		938f;\
	nop;\
937:;\
	li		t4, 0x1;\
	sb		t4, OFFSET_WRDQ_LTHF(t1);\
938:;\
    b       933b;\
    nop;\
936:;\

#define RDOE_SUB_TRDDATA_ADD	\
	bne		a0, 0x4, 934f;\
	nop;\
    li      a1, 0x8;\
    dli     t4, 0x250;\
    or      t4, t4, t8;\
    lb      a0, 0x2(t4);\
    dli     t4, 0x1;\
    and     a0, a0, t4;\
    bne     a0, t4, 932f ;\
    nop;\
    daddu   a1, a1, 0x1;\
932: ;\
	dli		t4, 0x28;\
	or		t4, t4, t8;\
933: ;\
	lb		a0, 0x7(t4);\
	dsubu	a0, a0, 0x1;\
	sb		a0, 0x7(t4);\
	lb		a0, 0x6(t4);\
	dsubu	a0, a0, 0x1;\
	sb		a0, 0x6(t4);\
	daddu	t4, t4, 0x20;\
	dsubu	a1, a1, 0x1;\
	bnez	a1, 933b;\
	nop;\
	dli		t4, 0x1c0;\
	or		t4, t4, t8;\
	lb		a0, 0x0(t4);\
	daddu	a0, a0, 0x1;\
	sb		a0, 0x0(t4);\
934: ;
#define	RDOE_ADD_TRDDATA_SUB		\
	bne		a0, 0x0, 934f;\
	nop ;\
    li      a1, 0x8;\
    dli     t4, 0x250;\
    or      t4, t4, t8;\
    lb      a0, 0x2(t4);\
    dli     t4, 0x1;\
    and     a0, a0, t4;\
    bne     a0, t4, 932f ;\
    nop;\
    daddu   a1, a1, 0x1;\
932: ;\
	dli		t4, 0x28;\
	or		t4, t4, t8;\
933: ;\
	lb		a0, 0x7(t4);\
	daddu	a0, a0, 0x1;\
	sb		a0, 0x7(t4);\
	lb		a0, 0x6(t4);\
	daddu	a0, a0, 0x1;\
	sb		a0, 0x6(t4);\
	daddu	t4, t4, 0x20;\
	dsubu	a1, a1, 0x1;\
	bnez	a1, 933b;\
	nop;\
	dli		t4, 0x1c0;\
	or		t4, t4, t8;\
	lb		a0, 0x0(t4);\
	dsubu	a0, a0, 0x1;\
	sb		a0, 0x0(t4);\
934: ;

