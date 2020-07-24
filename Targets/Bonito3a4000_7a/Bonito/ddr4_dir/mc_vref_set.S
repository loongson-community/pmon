//#define VREF 0x0701
#define COMPVREF_IN 1//1:in 0:out
#define COMP_MANUAL 0//1:manual 0:auto
#define COMP_PD     0
#define COMP_PCODE  0x8 //smaller, resistance smaller
#define COMP_NCODE  0xb//smaller, resistance bigger
#define COMP_CTRL   ((COMP_NCODE<<8)|(COMP_PCODE<<3)|(COMP_MANUAL<<2)|(COMPVREF_IN<<1)|(COMP_PD))
//close vref_inen of both mc



//1. open mc0 conf space
//2. set mc_en to 2'b01
//3. clear vref_ctrl
//4. comp
//5. close mc0 conf space

//6. open mc1 conf space
//7. set mc_en to 2'b01
//8. clear vref_ctrl
//9. comp
//10. close mc0 conf space
//11. default mc_en is mc0

#define VREF_SET(VREF) \
    	GET_NODE_ID_a0; \
;\
	dli     t0, 0x900000003ff00180 ;\
    	or      t0, a0 ;\
	lw      t1, 0x0(t0) ;\
	ori     t1, 0x10; ;\
	xori    t1, 0x10 ;\
	sw      t1, 0x0(t0) ;\
;\
	dli     t0, 0x900000003ff00400 ;\
    	or      t0, a0 ;\
	lw      t1, 0x0(t0) ;\
	lui     t2, 0x4000 ;\
	or      t1, t2 ;\
	lui     t2, 0x8000 ;\
	or      t1, t2 ;\
	xor     t1, t2 ;\
	sw      t1, 0x0(t0) ;\
;\
	dli     t0, 0x900000000ff00810 ;\
    	or      t0, a0 ;\
	dli     t3, (VREF<<48)|(VREF<<32)|(VREF<<16)|VREF ;\
	sd      t3, 0x0(t0) ;\
	sd      t3, 0x8(t0) ;\
	sd      t3, 0x10(t0) ;\
;\
	dli     t2, COMP_CTRL ;\
	sd      t2, 0x20(t0) ;\
;\
	dli     t0, 0x900000003ff00180 ;\
    	or      t0, a0 ;\
	lw      t1, 0x0(t0) ;\
	ori     t1, 0x10 ;\
	sw      t1, 0x0(t0) ;\
;\
	dli     t0, 0x900000003ff00180 ;\
    	or      t0, a0 ;\
	lw      t1, 0x0(t0) ;\
	ori     t1, 0x200 ;\
	xori    t1, 0x200 ;\
	sw      t1, 0x0(t0) ;\
;\
	dli     t0, 0x900000003ff00400 ;\
    	or      t0, a0 ;\
	lw      t1, 0x0(t0) ;\
	lui     t2, 0x8000 ;\
	or      t1, t2 ;\
	lui     t2, 0x4000 ;\
	or      t1, t2 ;\
	xor     t1, t2 ;\
	sw      t1, 0x0(t0) ;\
;\
	dli     t0, 0x900000000ff00810 ;\
    	or      t0, a0 ;\
	sd      t3, 0x0(t0) ;\
	sd      t3, 0x8(t0) ;\
	sd      t3, 0x10(t0) ;\
;\
	dli     t2, COMP_CTRL ;\
	sd      t2, 0x20(t0) ;\
;\
	dli     t0, 0x900000003ff00180 ;\
    	or      t0, a0 ;\
	lw      t1, 0x0(t0) ;\
	ori     t1, 0x200 ;\
	sw      t1, 0x0(t0) ;\
;\
	dli     t0, 0x900000003ff00400 ;\
    	or      t0, a0 ;\
	lw      t1, 0x0(t0) ;\
	lui     t2, 0x4000 ;\
	or      t1, t2 ;\
	lui     t2, 0x8000 ;\
	or      t1, t2 ;\
	xor     t1, t2 ;\
	sw      t1, 0x0(t0)
