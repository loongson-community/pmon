#define RDOE_SUB_TRDDATA_ADD	\
	bne		a0, 0x4, 934f;\
	nop;\
	dli		t4, 0x28;\
	or		t4, t4, t8;\
	dli		t0, 0x8;\
933: ;\
	lb		a0, 0x7(t4);\
	dsubu	a0, a0, 0x1;\
	sb		a0, 0x7(t4);\
	lb		a0, 0x6(t4);\
	dsubu	a0, a0, 0x1;\
	sb		a0, 0x6(t4);\
	daddu	t4, t4, 0x20;\
	dsubu	t0, t0, 0x1;\
	bnez	t0, 933b;\
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
	dli		t4, 0x28;\
	or		t4, t4, t8;\
	dli		t0, 0x8;\
933: ;\
	lb		a0, 0x7(t4);\
	daddu	a0, a0, 0x1;\
	sb		a0, 0x7(t4);\
	lb		a0, 0x6(t4);\
	daddu	a0, a0, 0x1;\
	sb		a0, 0x6(t4);\
	daddu	t4, t4, 0x20;\
	dsubu	t0, t0, 0x1;\
	bnez	t0, 933b;\
	nop;\
	dli		t4, 0x1c0;\
	or		t4, t4, t8;\
	lb		a0, 0x0(t4);\
	dsubu	a0, a0, 0x1;\
	sb		a0, 0x0(t4);\
934: ;

