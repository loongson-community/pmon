/* whd: loongson3C_ddr3_leveling.S
   2012.9.1
   first written by Leping from pfunc.s
   USE t8 to pass the CONFIG address
   ECC slice in not included yet
   2012.9.25 add ECC slice
*/
    b    lvl_req_set0
/* t1(0x20,0x40,...), t2(0x180,0x181,...), is used for loop, t0 is the loop count */
/* a0, a1 is used for load and store */
/* a2, a3 is used for set some parameters/judge some edges */
/* t4 is the tmp varible always used */

/* in PRINTSTR: a0, a1, a2, v0, v1 will be changed */
/* in GET_NUMBER_OF_SLICES: t0, t1 will be changed and t0 is the output*/
/* in RDOE_SUB_TRDDATA_ADD: a0, a1, t4 will be changed*/
/* in hexserial: ra, a0, a1, a2, a3 will be changed*/

#include "ddr_leveling_define.h"
#define	PREAMBLE_CHECK_DEBUG
//#define	PRINT_PREAMBLE_CHECK
//#define PRINT_DDR_LEVELING
//#define SIGNAL_DEPICT_DEBUG
//#define LVL_DEBUG
#define CHANGE_DQ_WITH_DQS
#define NO_EDGE_CHECK

#define WLVL_CHKBIT         0x1
#define ORDER_OF_UDIMM		0x876543210
#define ORDER_OF_RDIMM		0x765401238
//#define	ORDER_OF_UDIMM		0x847652013 //for SODIMM (2 cs and 8 chips per cs)
#define WRDQS_LTHF_STD 		0x40
#define WRDQ_LTHF_STD 		0x40 //less then STD will be set1
#define	RDDQS_LTHF_STD1		0x3a //greater then STD1 and less then STD2 will be set1
#define	RDDQS_LTHF_STD2		0x10
#define DLL_WRDQ_SUB		0x20
#define DLL_GATE_SUB		0x20
#define	WR_FILTER_LENGTH 	0x6
#define GATE_FILTER_LENGTH	0x6
#define PREAMBLE_LENGTH_3A9 0x60
#define PREAMBLE_LENGTH_3A8 0x60
#define MDL_CNT 0x500
#define GCL_CNT 10

#define	OFFSET_DLL_WRDQ 	0x19  // from 0x20/40/....
#define OFFSET_DLL_WRDQS 	0x1a
#define OFFSET_DLL_GATE 	0x18
#define OFFSET_WRDQ_LTHF 	0x0
#define	OFFSET_WRDQS_LTHF 	0x1
#define OFFSET_RDDQS_LTHF 	0x2
#define OFFSET_RDOE_BEGIN	0xe
#define OFFSET_RDOE_END		0xf
#define OFFSET_ODTOE_BEGIN 	0x14
#define OFFSET_ODTOE_END	0x15

        .global ddr3_leveling
        .ent    ddr3_leveling
ddr3_leveling:
 
	  move s5,ra

//#define PM_DPD_FRE// change parameters depend on frequency
#ifdef PM_DPD_FRE

#for 3a8, different frequency will use different rd_oe_start/stop
#frequency 500M, rd_oe_begin/end 0x03030202
#frequency 600M, rd_oe_begin/end 0x03030000
	li      t1, 0xbfe001c0
    lw      a1, 0x0(t1)
    dsrl    t1, a1, 14 //DDR_LOOPC
    and     t1, t1, 0x3ff
    dsrl    a1, a1, 24 //DDR_DIV
    and     a1, a1, 0x3f

    //DDR_DIV: 4 or 8
    dli     t4, 0x4
    beq     a1, t4, 1f
    nop
    dsrl    t1, t1, 1
1:
    dsrl    t1, t1, 2 

    dli     t4, 15
    bgt     t1, t4, 3f
    nop

    //<= 500M, for udimm, add rd_oe_start/stop by 0x2 and sub tPHY_RDDATA by 0x1
    //         for rdimm, only sub tPHY_RDDATA by 0x1
    GET_DIMM_TYPE
    bnez    a1, 4f //RDIMM
    nop

    //temp code for Kinston 2G UDIMM, at 400MHz, only sub tPHY_RDDATA by 0x1
    dli     t4, 12
    beq     t1, t4, 4f
    nop


/* identify wheather there is ecc slice */
	GET_NUMBER_OF_SLICES	

    dli     t1, 0x28
    or      t1, t1, t8

2:
    ld      a0, 0x0(t1)
    dli     t4, 0x020200000000
    daddu   a0, a0, t4
    sd      a0, 0x0(t1)
    daddu   t1, t1, 0x20
    dsubu   t0, t0, 0x1
    bnez    t0, 2b
    nop

4: //FOR RDIMM
    ld      a0, 0x1c0(t8)
    dsubu   a0, a0, 0x1
    sd      a0, 0x1c0(t8)

    //> 500M
3:


#endif
#only the gate dll is bypassed at the beginning of leveling
#while other dlls' bypass is set at the end of leveling
#ifdef DDR_DLL_BYPASS
    dli     t1, 0x0
    or      t1, t1, t8
    ld      a1, 0x0(t1)
    dli     t4, 0x0000ffff00000000
    and     a1, a1, t4 
    dsrl    a1, a1, 32 // dll_value store in a1
//    daddu   a1, a1, 2

    //set dll_ck0
    dli     t1, 0x18
    or      t1, t1, t8
    lb      a0, 0x4(t1)
    dmulou  a0, a1, a0
    dsrl    a0, a0, 0x7
    ori     a0, a0, 0x80
    sb      a0, 0x4(t1)
    
    //set dll_ck1
    dli     t1, 0x18
    or      t1, t1, t8
    lb      a0, 0x5(t1)
    dmulou  a0, a1, a0
    dsrl    a0, a0, 0x7
    ori     a0, a0, 0x80
    sb      a0, 0x5(t1)
    
    //set dll_ck2
    dli     t1, 0x18
    or      t1, t1, t8
    lb      a0, 0x6(t1)
    dmulou  a0, a1, a0
    dsrl    a0, a0, 0x7
    ori     a0, a0, 0x80
    sb      a0, 0x6(t1)
    
    //set dll_ck3
    dli     t1, 0x18
    or      t1, t1, t8
    lb      a0, 0x7(t1)
    dmulou  a0, a1, a0
    dsrl    a0, a0, 0x7
    ori     a0, a0, 0x80
    sb      a0, 0x7(t1)

#endif
/* 1. wait until init done */
    dli     t1, 0x160
    or      t1, t1, t8
wait_dram_init_done:
    ld      a0, 0x0(t1)
    dli     t4, 0x00000000ff000000
    and     a0, a0, t4
    beqz    a0, wait_dram_init_done
    nop

write_leveling:
    PRINTSTR("\r\nwrite leveling begin\r\n")

/* 2. set all dll to be 0 */
	GET_NUMBER_OF_SLICES
    dli     t1, 0x0
    or      t1, t1, t8
dll_wrdqs_set0:
    daddu   t1, t1, 0x20
	li		a0, 0x0
    sb      a0, OFFSET_DLL_WRDQS(t1)
    subu    t0, t0, 0x1 
    bnez    t0, dll_wrdqs_set0 
    nop
 
    PRINTSTR("\r\nall dll_wrdqs set 0\r\n")

/* 3. set leveling mode to be WRITE LEVELING */
lvl_mode_set01:
	dli		a0, 0x1
	sb		a0, 0x180(t8)

    PRINTSTR("\r\nset leveling mode to be WRITE LEVELING\r\n")

/* 4. check whether to start leveling */
lvl_ready_sampling:
    lb      a0, 0x185(t8)
    beqz    a0, lvl_ready_sampling
    nop

    PRINTSTR("\r\nwrite leveling ready\r\n")

/* 5. Set leveling req */

	GET_NUMBER_OF_SLICES
	dli		t1, 0x20
	or		t1, t1, t8
	dli		t2, 0x180
	or		t2, t2, t8

        dli             t6, 0x0
lvl_req_set0:
	dli		a0, 0x1
	sb		a0, 0x181(t8)
	dli		a0, 0x0
	sb		a0, 0x181(t8)

#ifdef LVL_DEBUG
    PRINTSTR("\r\nwrite leveling req set0\r\n")
#endif

/* 6. check whether this leveling request done */
lvl_done_sampling0:
    lb		a0, 0x186(t8)
	beqz	a0, lvl_done_sampling0
	nop

#ifdef LVL_DEBUG
    PRINTSTR("\r\nwrite leveling done\r\n")
#endif

lvl_resp_set0:
	lb		a0, 0x7(t2)
    GET_LVL_BYTE_t2
    GET_WLVL_RESP_a0
	beqz	a0, resp_set0_done
	nop

dll_wrdqs_add0:
#ifdef LVL_DEBUG
	PRINTSTR("\r\nslice ")
	dli		a0, 0x8
	dsubu	a0, a0, t0
	bal		hexserial4
	nop
	PRINTSTR(" add to get 0\r\n")
#endif
	lb		a0, OFFSET_DLL_WRDQS(t1)
	daddu	a0, a0, 0x1
	dli		t4, 0x7f
	and		a0, a0, t4
	sb		a0, OFFSET_DLL_WRDQS(t1)

#ifdef CHANGE_DQ_WITH_DQS
	lb		a0, OFFSET_DLL_WRDQS(t1) // get dll_wrdqs
	blt		a0, WRDQS_LTHF_STD, 1f
	nop
	li		t4, 0x0
	sb		t4, OFFSET_WRDQS_LTHF(t1) //set wrdqs_lt_half
	b		2f
	nop
1:
	li		t4, 0x1
	sb		t4, OFFSET_WRDQS_LTHF(t1) //set wrdqs_lt_half
2:
	dsubu	a0, a0, 0x20
	dli		t4, 0x7f
	and		a0, a0, t4
	sb		a0, OFFSET_DLL_WRDQ(t1) // set dll_wrdata

	blt		a0, WRDQ_LTHF_STD, 1f
	nop
	li		t4, 0x0
	sb		t4, OFFSET_WRDQ_LTHF(t1) //set wrdq_lt_half
	b		2f
	nop
1:
	li		t4, 0x1
	sb		t4, OFFSET_WRDQ_LTHF(t1) //set wrdq_lt_half
2:
#endif
        blt             t6, MDL_CNT,  1f
        nop
#ifdef	DLL_DELAY_LOOP
	PRINTSTR("\r\n sample 0 is time out \r\n")    
    b   leveling_failed    
    nop
#else
2:    
	PRINTSTR("\r\n sample 0 is time out \r\n")    
    b   2b
    nop
#endif
1:    
    daddu   t6, t6, 0x1
	b 	lvl_req_set0
	nop

resp_set0_done:
#ifdef	LVL_DEBUG
	PRINTSTR("\r\n 0 is found\r\n")
#endif
	dsubu	t0, t0, 0x1
	daddu	t1, t1, 0x20
	daddu	t2, t2, 0x1
        dli     t6, 0x0
	bnez	t0, lvl_req_set0
	nop

#if 1
//filter 0to1 giltch
	GET_NUMBER_OF_SLICES
	dli		t1, 0x20
	or		t1, t1, t8
	dli		t2, 0x180
	or		t2, t2, t8

dll_wrdqs_add:
	lb		a0, OFFSET_DLL_WRDQS(t1)
	daddu	a0, a0, 0x10
	dli		t4, 0x7f
	and		a0, a0, t4
	sb		a0, OFFSET_DLL_WRDQS(t1)

#ifdef CHANGE_DQ_WITH_DQS
	lb		a0, OFFSET_DLL_WRDQS(t1) // get dll_wrdqs
	blt		a0, WRDQS_LTHF_STD, 1f
	nop
	li		t4, 0x0
	sb		t4, OFFSET_WRDQS_LTHF(t1) //set wrdqs_lt_half
	b		2f
	nop
1:
	li		t4, 0x1
	sb		t4, OFFSET_WRDQS_LTHF(t1) //set wrdqs_lt_half
2:
	dsubu	a0, a0, 0x20
	dli		t4, 0x7f
	and		a0, a0, t4
	sb		a0, OFFSET_DLL_WRDQ(t1) // set dll_wrdata

	blt		a0, WRDQ_LTHF_STD, 1f
	nop
	li		t4, 0x0
	sb		t4, OFFSET_WRDQ_LTHF(t1) //set wrdq_lt_half
	b		2f
	nop
1:
	li		t4, 0x1
	sb		t4, OFFSET_WRDQ_LTHF(t1) //set wrdq_lt_half
2:

#endif

    daddu   t1, t1, 0x20
    dsubu   t0, t0, 0x1
    bnez    t0, dll_wrdqs_add
    nop
#endif


/* 0 to 1 */
	GET_NUMBER_OF_SLICES
	dli		t1, 0x20
	or		t1, t1, t8
	dli		t2, 0x180
	or		t2, t2, t8
	dli		s7, WR_FILTER_LENGTH
lvl_req_set1:
	dli		a0, 0x1
	sb		a0, 0x181(t8)
	dli		a0, 0x0
	sb		a0, 0x181(t8)

#ifdef LVL_DEBUG
    PRINTSTR("\r\nwrite leveling req set1\r\n")
#endif

lvl_done_sampling1:
    lb		a0, 0x186(t8)
	beqz	a0, lvl_done_sampling1
	nop

#ifdef LVL_DEBUG
    PRINTSTR("\r\nwrite leveling done\r\n")
#endif

lvl_resp_set1:
	lb		a0, 0x7(t2)
    GET_LVL_BYTE_t2
    GET_WLVL_RESP_a0
	bnez	a0, resp_set1_done
	nop

	dli		s7, WR_FILTER_LENGTH
dll_wrdqs_add1:
	lb		a0, OFFSET_DLL_WRDQS(t1)
	daddu	a0, a0, 0x1
	dli		t4, 0x7f
	and		a0, a0, t4
	sb		a0, OFFSET_DLL_WRDQS(t1)

#ifdef CHANGE_DQ_WITH_DQS
	lb		a0, OFFSET_DLL_WRDQS(t1) // get dll_wrdqs
	blt		a0, WRDQS_LTHF_STD, 1f
	nop
	li		t4, 0x0
	sb		t4, OFFSET_WRDQS_LTHF(t1) //set wrdqs_lt_half
	b		2f
	nop
1:
	li		t4, 0x1
	sb		t4, OFFSET_WRDQS_LTHF(t1) //set wrdqs_lt_half
2:
	dsubu	a0, a0, 0x20
	dli		t4, 0x7f
	and		a0, a0, t4
	sb		a0, OFFSET_DLL_WRDQ(t1) // set dll_wrdata

	blt		a0, WRDQ_LTHF_STD, 1f
	nop
	li		t4, 0x0
	sb		t4, OFFSET_WRDQ_LTHF(t1) //set wrdq_lt_half
	b		2f
	nop
1:
	li		t4, 0x1
	sb		t4, OFFSET_WRDQ_LTHF(t1) //set wrdq_lt_half
2:
#endif
        blt             t6, MDL_CNT,  1f
        nop
#ifdef	DLL_DELAY_LOOP
	PRINTSTR("\r\n sample 1 is time out \r\n")    
    b   leveling_failed    
    nop
#else
2:    
	PRINTSTR("\r\n sample 1 is time out \r\n")    
    b   2b
    nop
#endif
1:
    daddu   t6, t6, 0x1
	b   lvl_req_set1
	nop

resp_set1_done:
#ifdef LVL_DEBUG 
	PRINTSTR("\r\n 1 is found @ slice")
	dli		a0, 0x8
	dsubu	a0, a0, t0
	bal 	hexserial
	nop
#endif
	dsubu	s7, s7, 0x1
	bnez	s7, dll_wrdqs_add1
	nop
	dli		s7, WR_FILTER_LENGTH

//  return the more add
	lb		a0, OFFSET_DLL_WRDQS(t1)
	dsubu	a0, a0, WR_FILTER_LENGTH
	daddu	a0, a0, 0x1
	dli		t4, 0x7f
	and		a0, a0, t4
	sb		a0, OFFSET_DLL_WRDQS(t1)

#ifdef CHANGE_DQ_WITH_DQS
	lb		a0, OFFSET_DLL_WRDQS(t1) // get dll_wrdqs
	blt		a0, WRDQS_LTHF_STD, 1f
	nop
	li		t4, 0x0
	sb		t4, OFFSET_WRDQS_LTHF(t1) //set wrdqs_lt_half
	b		2f
	nop
1:
	li		t4, 0x1
	sb		t4, OFFSET_WRDQS_LTHF(t1) //set wrdqs_lt_half
2:
	dsubu	a0, a0, 0x20
	dli		t4, 0x7f
	and		a0, a0, t4
	sb		a0, OFFSET_DLL_WRDQ(t1) // set dll_wrdata

	blt		a0, WRDQ_LTHF_STD, 1f
	nop
	li		t4, 0x0
	sb		t4, OFFSET_WRDQ_LTHF(t1) //set wrdq_lt_half
	b		2f
	nop
1:
	li		t4, 0x1
	sb		t4, OFFSET_WRDQ_LTHF(t1) //set wrdq_lt_half
2:
#endif
	dsubu	t0, t0, 0x1
	daddu	t1, t1, 0x20
	daddu	t2, t2, 0x1
	dli	t6, 0x0
	bnez	t0, lvl_req_set1
	nop
	
write_leveling_done:
#ifdef PRINT_DDR_LEVELING
	PRINTSTR("\r\n The MC param after write leveling 0 to 1 is:\r\n")
	PRINT_THE_MC_PARAM
#endif
	
/* 8. All 1 found, set params according to wrdqs */

//    GET_DIMM_TYPE
//    beqz    a1, 81f
//    nop

/* adjust wrdqs carefully */
#if 0   //def  DEBUG_DDR_PARAM   //print registers
    PRINTSTR("\r\nThe MC param before carefully adjust is:\r\n")
	PRINT_THE_MC_PARAM
#endif
wrdqs_adjust:
#if 1
#ifdef LVL_DEBUG
    PRINTSTR("\r\nwrdqs around 0x00 carefully adjust begin\r\n")
#endif
	GET_NUMBER_OF_SLICES
	daddu	t0, t0, 0x1
    dli     a2, 0x08
    dli     a3, 0x78
	dli     t1, 0x00
    or      t1, t1, t8
	WRDQS_ADJUST_LOOP

	GET_NUMBER_OF_SLICES
	daddu	t0, t0, 0x1
    dli     a2, 0x28
    dli     a3, 0x18
	dli     t1, 0x00
    or      t1, t1, t8
	WRDQS_ADJUST_LOOP

	GET_NUMBER_OF_SLICES
	daddu	t0, t0, 0x1
    dli     a2, 0x48
    dli     a3, 0x38
	dli     t1, 0x00
    or      t1, t1, t8
	WRDQS_ADJUST_LOOP

	GET_NUMBER_OF_SLICES
	daddu	t0, t0, 0x1
    dli     a2, 0x68
    dli     a3, 0x58
	dli     t1, 0x00
    or      t1, t1, t8
	WRDQS_ADJUST_LOOP


#ifdef LVL_DEBUG
    PRINTSTR("\r\nwrdqs around 0x00 carefully adjust end\r\n")
#endif
#endif

#if 0   //def  DEBUG_DDR_PARAM   //print registers
    PRINTSTR("\r\nThe MC param after carefully adjust is:\r\n")
	PRINT_THE_MC_PARAM
#endif
81:

#if 1
/* 8.1 adjust wrdata */

/* t0 is used to indicate 8 slices */
	GET_NUMBER_OF_SLICES
    dli     t1, 0x20
    or      t1, t1, t8
dll_wrdata_set:
	lb		a0, OFFSET_DLL_WRDQS(t1) // get dll_wrdqs
	blt		a0, WRDQS_LTHF_STD, 1f
	nop
	li		t4, 0x0
	sb		t4, OFFSET_WRDQS_LTHF(t1) //set wrdqs_lt_half
	b		2f
	nop
1:
	li		t4, 0x1
	sb		t4, OFFSET_WRDQS_LTHF(t1) //set wrdqs_lt_half
2:
	dsubu	a0, a0, DLL_WRDQ_SUB
	dli		t4, 0x7f
	and		a0, a0, t4
	sb		a0, OFFSET_DLL_WRDQ(t1) // set dll_wrdata

	blt		a0, WRDQ_LTHF_STD, 1f
	nop
	li		t4, 0x0
	sb		t4, OFFSET_WRDQ_LTHF(t1) //set wrdq_lt_half
	b		2f
	nop
1:
	li		t4, 0x1
	sb		t4, OFFSET_WRDQ_LTHF(t1) //set wrdq_lt_half
2:
	daddu	t1, t1, 0x20
	dsubu	t0, t0, 0x1
	bnez	t0, dll_wrdata_set
	nop
#endif

	
wrdq_lt_half_test:
	dli	s7, 0x0 // s7 represent whether find 1 to 0 or not
    GET_DIMM_TYPE
    bnez    a1, rdimm_wrdq_lt_half_test
    nop
	GET_NUMBER_OF_SLICES
	dsubu	t0, t0, 0x1 // onlt loop 7 times

    dli     t2, 0x0
wrdq_lt_half_test_loop:
	dli		a0, ORDER_OF_UDIMM
	dli	t4, 0x4
	mulou	a1,	t2, t4
	dsrl	a0, a0, a1
	and		a0, a0, 0xf
	daddu	a0, a0, 0x1
	dli	t4, 0x20
	mulou	t1, a0, t4
	or		t1, t1, t8
#if 0
	bal		hexserial
	nop
#endif

    daddu   t2, t2, 0x1
    bgt     t2, t0, record_slice_num
    nop
    lb      a0, 0x0(t1)
    beqz    a0, wrdq_lt_half_test_loop
    nop
	
	dli		a0, ORDER_OF_UDIMM
	dli	t4, 0x4
	mulou	a1,	t2, t4
	dsrl	a0, a0, a1
	and		a0, a0, 0xf
	daddu	a0, a0, 0x1
	dli	t4, 0x20
	mulou	t1, a0, t4
	or		t1, t1, t8
#if 0
	bal		hexserial
	nop
#endif

	lb		a0, 0x0(t1)
    beqz    a0, record_slice_num
    nop
    b       wrdq_lt_half_test_loop
    nop

record_slice_num:
    move    t3, t2 //the slice number save in t3
    beq     t3, 0x8, first_slice_wrdq_lt_half_test
    nop

wrdq_clkdelay_set:
//    li      t0, 0x7 //only loop 7 times
    dli     t2, 0x0
wrdq_clkdelay_set_loop:
    daddu   t2, t2, 0x1
    bgt     t2, t0, first_slice_wrdq_lt_half_test
    nop

	dli		a0, ORDER_OF_UDIMM
	dli	t4, 0x4
	mulou	a1,	t2, t4
	dsrl	a0, a0, a1
	and		a0, a0, 0xf
	daddu	a0, a0, 0x1
	dli	t4, 0x20
	mulou	t1, a0, t4
	or		t1, t1, t8
   	daddu	t1, t1, 0x10 
    
	ld      a0, 0x0(t1)
    blt     t2, t3, wrdq_clkdelay_set0
    nop
    b       wrdq_clkdelay_set1
    nop

wrdq_clkdelay_set0:
    dli     t4, 0xffffff00ffffffff
    and     a0, a0, t4
    sd      a0, 0x0(t1)
    b       wrdq_clkdelay_set_loop
    nop
    
wrdq_clkdelay_set1:
	dli 	s7, 0x1
    dli     t4, 0xffffff00ffffffff
    and     a0, a0, t4
    dli     t4, 0x0000000100000000
    or      a0, a0, t4
    sd      a0, 0x0(t1)
    b       wrdq_clkdelay_set_loop
    nop

first_slice_wrdq_lt_half_test:
	beq	s7, 0x1, trddata_tphywrdata_sub
	nop
	dli		a0, ORDER_OF_UDIMM
	and		a0, a0, 0xf
	daddu	a0, a0, 0x1
	dli	t4, 0x20
	mulou	t1, a0, t4
	or		t1, t1, t8
    
    ld      a0, 0x0(t1)
    dli     t4, 0x00000000000000ff
    and     a0, a0, t4
    beqz     a0, write_leveling_exit
    nop
    
    
trddata_tphywrdata_sub:
    /* tRDDATA sub one */
    dli     t2, 0x1c0
    or      t2, t2, t8
    ld      a0, 0x0(t2)
    dli     t4, 0x01
    dsubu   a0, a0, t4
    sd      a0, 0x0(t2)
   /* tPHY_WRDATA sub one */
    dli     t2, 0x1d0
    or      t2, t2, t8
    ld      a0, 0x0(t2)
    dli     t4, 0x100000000
    dsubu   a0, a0, t4
    sd      a0, 0x0(t2)
    b       write_leveling_exit
    nop

rdimm_wrdq_lt_half_test:
/* identify wheather there is ecc slice */
    dli     t1, 0x250
    or      t1, t1, t8
    lb      a0, 0x2(t1)
    dli     t1, 0x1
    and     a0, a0, t1
//    dli     t2, 0x0
    bne     a0, t1, rdimm_wrdq_lt_half_test_3210
    nop

rdimm_wrdq_lt_half_test_83:
    li      t0, 0x4
    dli     t2, 0x0
	dli		a0, ORDER_OF_RDIMM
	dli	t4, 0x4
	mulou	a1,	t2, t4
	dsrl	a0, a0, a1
	and		a0, a0, 0xf
	daddu	a0, a0, 0x1
	dli	t4, 0x20
	mulou	t1, a0, t4
	or		t1, t1, t8
    lb      a0, 0x0(t1)
    daddu   t2, t2, 0x1
    beqz    a0, rdimm_wrdq_lt_half_test_loop_3210
    nop
	dli		a0, ORDER_OF_RDIMM
	dli	t4, 0x4
	mulou	a1,	t2, t4
	dsrl	a0, a0, a1
	and		a0, a0, 0xf
	daddu	a0, a0, 0x1
	dli	t4, 0x20
	mulou	t1, a0, t4
	or		t1, t1, t8
    lb      a0, 0x0(t1)
    beqz    a0, rdimm_record_slice_num_83210
    nop
    b       rdimm_wrdq_lt_half_test_loop_3210
    nop


rdimm_wrdq_lt_half_test_3210:
    li      t0, 0x4
    dli     t2, 0x1
	dli		a0, ORDER_OF_RDIMM
	dli	t4, 0x4
	mulou	a1,	t2, t4
	dsrl	a0, a0, a1
	and		a0, a0, 0xf
	daddu	a0, a0, 0x1
	dli	t4, 0x20
	mulou	t1, a0, t4
	or		t1, t1, t8
    
rdimm_wrdq_lt_half_test_loop_3210:
	dli		a0, ORDER_OF_RDIMM
	dli	t4, 0x4
	mulou	a1,	t2, t4
	dsrl	a0, a0, a1
	and		a0, a0, 0xf
	daddu	a0, a0, 0x1
	dli	t4, 0x20
	mulou	t1, a0, t4
	or		t1, t1, t8
    daddu   t2, t2, 0x1
    bgt     t2, t0, rdimm_wrdq_lt_half_test_4567
    nop
#ifdef LVL_DEBUG
	move	a0, t1
	bal	hexserial
	nop
#endif
    lb      a0, 0x0(t1)
    beqz    a0, rdimm_wrdq_lt_half_test_loop_3210
    nop
	dli		a0, ORDER_OF_RDIMM
	dli	t4, 0x4
	mulou	a1,	t2, t4
	dsrl	a0, a0, a1
	and		a0, a0, 0xf
	daddu	a0, a0, 0x1
	dli	t4, 0x20
	mulou	t1, a0, t4
	or		t1, t1, t8
    lb      a0, 0x0(t1)
    beqz    a0, rdimm_record_slice_num_3210
    nop
    b       rdimm_wrdq_lt_half_test_loop_3210
    nop

rdimm_record_slice_num_3210:
rdimm_record_slice_num_83210:
    move    t3, t2 
#ifdef LVL_DEBUG
	PRINTSTR("\r\nt3=")
	move 	a0, t3
	bal	hexserial
	nop
#endif

/* identify wheather there is ecc slice */
    dli     t1, 0x250
    or      t1, t1, t8
    lb      a0, 0x2(t1)
    dli     t1, 0x1
    and     a0, a0, t1
    bne     a0, t1, rdimm_wrdq_clkdelay_set_3210
    nop
rdimm_wrdq_clkdelay_set_8:
    li      t0, 0x4
    dli     t2, 0x0
	dli		a0, ORDER_OF_RDIMM
	dli	t4, 0x4
	mulou	a1,	t2, t4
	dsrl	a0, a0, a1
	and		a0, a0, 0xf
	daddu	a0, a0, 0x1
	dli	t4, 0x20
	mulou	t1, a0, t4
	daddu	t1, t1, 0x10
	or		t1, t1, t8
//    daddu   t2, t2, 0x1
    ld      a0, 0x0(t1)
    blt     t2, t3, rdimm_wrdq_clkdelay_set0_8
    nop
    b       rdimm_wrdq_clkdelay_set1_8
    nop

rdimm_wrdq_clkdelay_set0_8:
    dli     t4, 0xffffff00ffffffff
    and     a0, a0, t4
    sd      a0, 0x0(t1)
    dli     t1, 0xb0 //here set 0xb0 because it will sub 0x20 later
    or      t1, t1, t8
    b       rdimm_wrdq_clkdelay_set_loop_3210
    nop
    
rdimm_wrdq_clkdelay_set1_8:
	dli 	s7, 0x1
    dli     t4, 0xffffff00ffffffff
    and     a0, a0, t4
    dli     t4, 0x0000000100000000
    or      a0, a0, t4
    sd      a0, 0x0(t1)
    dli     t1, 0xb0 //here set 0xb0 because it will sub 0x20 later
    or      t1, t1, t8
    b       rdimm_wrdq_clkdelay_set_loop_3210
    nop

rdimm_wrdq_clkdelay_set_3210:
    li      t0, 0x4 
    dli     t2, 0x1
rdimm_wrdq_clkdelay_set_loop_3210:
1:
    daddu   t2, t2, 0x1
    bgt     t2, t0, rdimm_wrdq_lt_half_test_4567
    nop
	dli		a0, ORDER_OF_RDIMM
	dli	t4, 0x4
	mulou	a1,	t2, t4
	dsrl	a0, a0, a1
	and		a0, a0, 0xf
	daddu	a0, a0, 0x1
	dli	t4, 0x20
	mulou	t1, a0, t4
	daddu	t1, t1, 0x10
	or		t1, t1, t8
    ld      a0, 0x0(t1)
    blt     t2, t3, rdimm_wrdq_clkdelay_set0_3210
    nop
    b       rdimm_wrdq_clkdelay_set1_3210
    nop

rdimm_wrdq_clkdelay_set0_3210:
    dli     t4, 0xffffff00ffffffff
    and     a0, a0, t4
    sd      a0, 0x0(t1)
    b       1b
    nop
    
rdimm_wrdq_clkdelay_set1_3210:
	dli 	s7, 0x1
    dli     t4, 0xffffff00ffffffff
    and     a0, a0, t4
    dli     t4, 0x0000000100000000
    or      a0, a0, t4
    sd      a0, 0x0(t1)
    b       1b
    nop


rdimm_wrdq_lt_half_test_4567:
    li      t0, 0x8 
    dli     t2, 0x5

rdimm_wrdq_lt_half_test_loop_4567:
	dli		a0, ORDER_OF_RDIMM
	dli	t4, 0x4
	mulou	a1,	t2, t4
	dsrl	a0, a0, a1
	and		a0, a0, 0xf
	daddu	a0, a0, 0x1
	dli	t4, 0x20
	mulou	t1, a0, t4
	or		t1, t1, t8
    daddu   t2, t2, 0x1
    bgt     t2, t0, slice_8_wrdq_lt_half_test
    nop
    lb      a0, 0x0(t1)
    beqz    a0, rdimm_wrdq_lt_half_test_loop_4567
    nop
	dli		a0, ORDER_OF_RDIMM
	dli	t4, 0x4
	mulou	a1,	t2, t4
	dsrl	a0, a0, a1
	and		a0, a0, 0xf
	daddu	a0, a0, 0x1
	dli	t4, 0x20
	mulou	t1, a0, t4
	or		t1, t1, t8
    	lb      a0, 0x0(t1)
    beqz    a0, rdimm_record_slice_num_4567
    nop
    b       rdimm_wrdq_lt_half_test_loop_4567
    nop

rdimm_record_slice_num_4567:
    move    t3, t2 //the slice number save in t3
    dli     t5, 0x0

rdimm_wrdq_clkdelay_set_4567:
    li      t0, 0x8 //only loop 7 times
    dli     t2, 0x5
rdimm_wrdq_clkdelay_set_loop_4567:
    daddu   t2, t2, 0x1
    bgt     t2, t0, slice_8_wrdq_lt_half_test
    nop
	dli		a0, ORDER_OF_RDIMM
	dli	t4, 0x4
	mulou	a1,	t2, t4
	dsrl	a0, a0, a1
	and		a0, a0, 0xf
	daddu	a0, a0, 0x1
	dli	t4, 0x20
	mulou	t1, a0, t4
	daddu	t1, t1, 0x10
	or		t1, t1, t8
    ld      a0, 0x0(t1)
    blt     t2, t3, rdimm_wrdq_clkdelay_set0_4567
    nop
    b       rdimm_wrdq_clkdelay_set1_4567
    nop

rdimm_wrdq_clkdelay_set0_4567:
    dli     t4, 0xffffff00ffffffff
    and     a0, a0, t4
    sd      a0, 0x0(t1)
    b       rdimm_wrdq_clkdelay_set_loop_4567
    nop
    
rdimm_wrdq_clkdelay_set1_4567:
	dli 	s7, 0x1
    dli     t4, 0xffffff00ffffffff
    and     a0, a0, t4
    dli     t4, 0x0000000100000000
    or      a0, a0, t4
    sd      a0, 0x0(t1)

    bnez    t5, 1f
    nop
/* identify wheather there is ecc slice */
    dli     t1, 0x250
    or      t1, t1, t8
    lb      a0, 0x2(t1)
    dli     t1, 0x1
    and     a0, a0, t1
//    dli     t2, 0x0
    bne     a0, t1, noecc
    nop

	dli	t4, 0x20
        dli     a0, 0x9
        mulou   t1, a0, t4
        or      t1, t1, t8
    lb      a0, 0x0(t1)
    bnez    a0, 1f
    nop
noecc:
	dli	t4, 0x20
	dli	a0, 0x4
	mulou	t1, a0, t4
	or		t1, t1, t8
    lb      a0, 0x0(t1)
    bnez    a0, 1f
    nop

	dli	a0, 0x3
	mulou	t1, a0, t4
	or		t1, t1, t8
    lb      a0, 0x0(t1)
    bnez    a0, 1f
    nop
    
	dli	a0, 0x2
	mulou	t1, a0, t4
	or		t1, t1, t8
    lb      a0, 0x0(t1)
    bnez    a0, 1f
    nop

	dli	a0, 0x1
	mulou	t1, a0, t4
	or		t1, t1, t8
    lb      a0, 0x0(t1)
    bnez    a0, 1f
    nop

    ld      a0, 0x30(t8)
    dli     t4, 0xffffff00ffffffff
    and     a0, a0, t4
    dli     t4, 0x0000000100000000
    or      a0, a0, t4
    sd      a0, 0x30(t8)

    ld      a0, 0x50(t8)
    dli     t4, 0xffffff00ffffffff
    and     a0, a0, t4
    dli     t4, 0x0000000100000000
    or      a0, a0, t4
    sd      a0, 0x50(t8)

    ld      a0, 0x70(t8)
    dli     t4, 0xffffff00ffffffff
    and     a0, a0, t4
    dli     t4, 0x0000000100000000
    or      a0, a0, t4
    sd      a0, 0x70(t8)

    ld      a0, 0x90(t8)
    dli     t4, 0xffffff00ffffffff
    and     a0, a0, t4
    dli     t4, 0x0000000100000000
    or      a0, a0, t4
    sd      a0, 0x90(t8)

    dli     t1, 0x250
    or      t1, t1, t8
    lb      a0, 0x2(t1)
    dli     t1, 0x1
    and     a0, a0, t1
//    dli     t2, 0x0
    bne     a0, t1, 1f
    nop
    ld      a0, 0x130(t8)
    dli     t4, 0xffffff00ffffffff
    and     a0, a0, t4
    dli     t4, 0x0000000100000000
    or      a0, a0, t4
    sd      a0, 0x130(t8)
1:
    daddu   t5, t5, 0x1
    b       rdimm_wrdq_clkdelay_set_loop_4567
    nop

slice_8_wrdq_lt_half_test:
	beq	s7, 0x1, rdimm_trddata_tphywrdata_sub
	nop
    dli     t1, 0x250
    or      t1, t1, t8
    lb      a0, 0x2(t1)
    dli     t1, 0x1
    and     a0, a0, t1
    bne     a0, t1, slice_3_wrdq_lt_half_test
    nop
	dli		a0, ORDER_OF_RDIMM
	dli	t2, 0x0
	dli	t4, 0x4
	mulou	a1,	t2, t4
	dsrl	a0, a0, a1
	and		a0, a0, 0xf
	daddu	a0, a0, 0x1
	dli	t4, 0x20
	mulou	t1, a0, t4
	or		t1, t1, t8
    ld      a0, 0x0(t1)
    dli     t4, 0x00000000000000ff
    and     a0, a0, t4
    bnez    a0, rdimm_trddata_tphywrdata_sub
    nop
    b       slice_4_wrdq_lt_half_test
    nop

slice_3_wrdq_lt_half_test:
	beq	s7, 0x1, rdimm_trddata_tphywrdata_sub
	nop
	dli		a0, ORDER_OF_RDIMM
	dli	t2, 0x1
	dli	t4, 0x4
	mulou	a1,	t2, t4
	dsrl	a0, a0, a1
	and		a0, a0, 0xf
	daddu	a0, a0, 0x1
	dli	t4, 0x20
	mulou	t1, a0, t4
	or		t1, t1, t8
    ld      a0, 0x0(t1)
    dli     t4, 0x00000000000000ff
    and     a0, a0, t4
    bnez    a0, rdimm_trddata_tphywrdata_sub
    nop
    
slice_4_wrdq_lt_half_test:
	beq	s7, 0x1, rdimm_trddata_tphywrdata_sub
	nop
	dli		a0, ORDER_OF_RDIMM
	dli	t2, 0x5
	dli	t4, 0x4
	mulou	a1,	t2, t4
	dsrl	a0, a0, a1
	and		a0, a0, 0xf
	daddu	a0, a0, 0x1
	dli	t4, 0x20
	mulou	t1, a0, t4
	or		t1, t1, t8
    ld      a0, 0x0(t1)
    dli     t4, 0x00000000000000ff
    and     a0, a0, t4
    beqz    a0, write_leveling_exit
    nop

rdimm_trddata_tphywrdata_sub:
    /* tRDDATA sub one */
    dli     t2, 0x1c0
    or      t2, t2, t8
    ld      a0, 0x0(t2)
    dli     t4, 0x01
    dsubu   a0, a0, t4
    sd      a0, 0x0(t2)
   /* tPHY_WRDATA sub one */
    dli     t2, 0x1d0
    or      t2, t2, t8
    ld      a0, 0x0(t2)
    dli     t4, 0x100000000
    dsubu   a0, a0, t4
    sd      a0, 0x0(t2)

write_leveling_exit:
    dli     t1, 0x180
    or      t1, t1, t8
    ld      a0, 0x0(t1)
    dli     t4, 0xffffffffffffff00
    and     a0, a0, t4
    sd      a0, 0x0(t1)

    b       gate_leveling
//    b       100f
    nop

gate_leveling:
#if 1 //3a3000 new
//    PRINTSTR("\r\nset cs_zq to be same with cs_enable\r\n")
    lb      a0, 0x169(t8)
    sb      a0, 0x16a(t8)

reset_init_start_new:
    dli     t1, 0x18
    or      t1, t1, t8
	dli		a0, 0x0
    sb      a0, 0x0(t1)

    dli     a0, 0x1
    sb      a0, 0x0(t1)

wait_init_done_new:
    dli     t1, 0x160
    or      t1, t1, t8
    lb      a0, 0x3(t1)
    beqz    a0, wait_init_done_new
    nop

reset_init_start_new2:
    dli     t1, 0x18
    or      t1, t1, t8
	dli		a0, 0x0
    sb      a0, 0x0(t1)

    dli     a0, 0x1
    sb      a0, 0x0(t1)

wait_init_done_new2:
    dli     t1, 0x160
    or      t1, t1, t8
    lb      a0, 0x3(t1)
    beqz    a0, wait_init_done_new2
    nop
#endif

    PRINTSTR("\r\nwrite leveling finish and gate leveling begin\r\n")
#ifdef  PRINT_DDR_LEVELING   //print registers
    PRINTSTR("\r\nThe MC param after write leveling is:\r\n")
	PRINT_THE_MC_PARAM
#endif

/* identify wheather there is ecc slice */
	GET_NUMBER_OF_SLICES
    dli     t1, 0x20
    or      t1, t1, t8
dll_gate_set0:
	dli		a0, 0x0
#ifdef DDR_DLL_BYPASS
	dli		a0, 0x80
#endif
    sb      a0, OFFSET_DLL_GATE(t1)
    subu    t0, t0, 0x1
	daddu	t1, t1, 0x20
    bnez    t0, dll_gate_set0 
    nop
 
glvl_mode_set10:
    dli     t1, 0x180
    or      t1, t1, t8
	dli		a0, 0x2
    sb      a0, 0x0(t1)

    dli     a1, 0x1
glvl_ready_sampling:
    dli     t1, 0x180
    or      t1, t1, t8
    lb      a0, 0x5(t1)
    bne     a0, a1, glvl_ready_sampling
    nop

#ifdef SIGNAL_DEPICT_DEBUG
	PRINTSTR("\r\nthe signal depict begin:\r\n")
	GET_NUMBER_OF_SLICES
	dli		t1, 0x28     // save the init para before signal depict
	or 		t1, t1, t8
	lb		a0, 0x7(t1)
	dli		t1, 0x350
	or		t1, t1, t8
	sb		a0, 0x7(t1)
	dli		t1, 0x1c0
	or		t1, t1, t8
	lb		a0, 0x0(t1)
	dli		t1, 0x350
	or		t1, t1, t8
	sb		a0, 0x6(t1)
	
	dli		t1, 0x28
	or		t1, t1, t8
	dli		t5, 0x180
	or		t5, t5, t8
	dli		t2, 0x0
	dli		s6, 0x0
	dli		s7, 0x0
t_glvl_req_set:
	bne		s6, 0x15, 1f
	nop
	dli		s6, 0x0			//reset trddata
	lb		a0, 0x356(t8)
	sb		a0, 0x1c0(t8)
	dsubu	t0, t0, 0x1
	beqz	t0, signal_depict_end
	nop
	daddu	t1, t1, 0x20
	daddu	t5, t5, 0x1
	PRINTSTR("\r\nthe above is slice ")
	dli		t4, 0x8
	dsubu 	a0, t4, t0
	bal		hexserial
	nop
	PRINTSTR("\r\n")
1:
    dli     t4, 0x180
    or      t4, t4, t8
    dli     a0, 0x1
    sb      a0, 0x1(t4)
    dli		a0, 0x0
    sb      a0, 0x1(t4)
	
1:						//glvl_done_sampling
    dli     t4, 0x180
    or      t4, t4, t8
    lb      a0, 0x6(t4)
    bne     a0, 0x1, 1b
    nop
	
	lb		a0, 0x7(t5)
    GET_GLVL_RESP_a0
	move	a1, a0
#if 1
    dli     t4, 0x180
    or      t4, t4, t8
    dli     a0, 0x1
    sb      a0, 0x1(t4)
    dli		a0, 0x0
    sb      a0, 0x1(t4)
	
1:						//glvl_done_sampling
    dli     t4, 0x180
    or      t4, t4, t8
    lb      a0, 0x6(t4)
    bne     a0, 0x1, 1b
    nop
	
	lb		a0, 0x7(t5)
    GET_GLVL_RESP_a0
	or		a0, a0, a1
	move	a1, a0
#endif
#if 1
    dli     t4, 0x180
    or      t4, t4, t8
    dli     a0, 0x1
    sb      a0, 0x1(t4)
    dli		a0, 0x0
    sb      a0, 0x1(t4)
	
1:						//glvl_done_sampling
    dli     t4, 0x180
    or      t4, t4, t8
    lb      a0, 0x6(t4)
    bne     a0, 0x1, 1b
    nop
	
	lb		a0, 0x7(t5)
    GET_GLVL_RESP_a0
	or		a0, a0, a1
#endif

	sll		a0, a0, 0x1f
	srl		a0, a0, s7
	or		t2, t2, a0
	daddu	s7, s7, 0x1
	blt		s7, 0x20, 1f // every 0x20 print the status
	nop
	move 	a0, t2
	bal		hexserial
	nop
	PRINTSTR(" ")
	dli		t2, 0x0
	dli		s7, 0x0
	daddu	s6, s6, 0x1
1:

#if 1
	lb		a0, 0x10(t1)
	daddu	a0, a0, 0x1
	dli		t4, 0x7f
	and		a0, a0, t4
	sb		a0, 0x10(t1)
	bnez	a0, 1f
	nop
	lb		a0, 0x1c0(t8)
	daddu	a0, a0, 0x1
	sb		a0, 0x1c0(t8)
1:
#else
	lb		a0, 0x10(t1)
	dsubu	a0, a0, 0x1
	dli		t4, 0x7f
	and		a0, a0, t4
	sb		a0, 0x10(t1)
	bne		a0, 0x7f,1f
	nop
	lb		a0, 0x1c0(t8)
	dsubu	a0, a0, 0x1
	sb		a0, 0x1c0(t8)
1:
#endif
	b		t_glvl_req_set
	nop

signal_depict_end:
//identify wheather there is ecc slice
	GET_NUMBER_OF_SLICES
	dli		t1, 0x28
	or		t1, t1, t8
reset_rd_oe:
	dli		t4, 0x350
	or		t4, t4, t8
	lb		a0, 0x7(t4)
	sb		a0, 0x7(t1)
	sb		a0, 0x6(t1)
	daddu	t1, t1, 0x20
	dsubu	t0, t0, 0x1
	bnez	t0, reset_rd_oe
	nop

	dli		t1, 0x350 // reset trddata
	or		t1, t1, t8
	lb		a0, 0x6(t1)
	dli		t1, 0x1c0
	or		t1, t1, t8
	sb		a0, 0x0(t1)
	
	GET_NUMBER_OF_SLICES
    dli     t1, 0x20
    or      t1, t1, t8
11:
	dli		a0, 0x0
#ifdef DDR_DLL_BYPASS
	dli		a0, 0x80
#endif
    sb      a0, OFFSET_DLL_GATE(t1)
    subu    t0, t0, 0x1
	daddu	t1, t1, 0x20
    bnez    t0, 11b 
    nop
	PRINTSTR("\r\n")
#endif

/* gate leveling set 1 to 0 */
    GET_NUMBER_OF_SLICES
	dli		t1, 0x20
	or		t1, t1, t8
	dli		t2, 0x180
	or		t2, t2, t8

    dli     t6, 0x0
glvl_req_set0:
	dli		a0, 0x1
	sb		a0, 0x181(t8)
	dli		a0, 0x0
	sb		a0, 0x181(t8)

glvl_done_sampling0:
    lb		a0, 0x186(t8)
	beqz	a0, glvl_done_sampling0
	nop

glvl_resp_set0:
	lb		a0, 0x7(t2)
    GET_GLVL_RESP_2BIT_a0
	beqz	a0, glvl_resp_set0_done
	nop

dll_gate_add0:
	lb		a0, OFFSET_DLL_GATE(t1)
	daddu	a0, a0, 0x1
	dli		t4, 0x7f
	and		a0, a0, t4
#ifdef DDR_DLL_BYPASS
	lb		t4, 0x4(t8)
	daddu	t4, t4, 0x2
1:
	blt		a0, t4, 2f
	nop
	dsubu	a0, a0, t4
	b		1b
	nop
2:
	ori		a0, 0x80
#endif
	sb		a0, OFFSET_DLL_GATE(t1)
	dli		t4, 0x7f
	and		a0, a0, t4
	bnez	a0, 1f
	nop

	lb		a0, OFFSET_RDOE_BEGIN(t1)
	daddu	a0, a0, 0x1
	sb		a0, OFFSET_RDOE_BEGIN(t1)
	lb		a0, OFFSET_RDOE_END(t1)
	daddu	a0, a0, 0x1
	sb		a0, OFFSET_RDOE_END(t1)
	RDOE_SUB_TRDDATA_ADD
/*
	lb		a0, OFFSET_ODTOE_BEGIN(t1)
	daddu	a0, a0, 0x1
	sb		a0, OFFSET_ODTOE_BEGIN(t1)
	lb		a0, OFFSET_ODTOE_END(t1)
	daddu	a0, a0, 0x1
	sb		a0, OFFSET_ODTOE_END(t1)
*/
1:
        bgt             t6, MDL_CNT,  leveling_failed
        nop
        daddu           t6, t6, 0x1
	b 		glvl_req_set0
	nop

glvl_resp_set0_done:
#ifdef LVL_DEBUG
	PRINTSTR("\r\n gate leveling 0 is found\r\n")
#endif
	dsubu	t0, t0, 0x1
	daddu	t1, t1, 0x20
	daddu	t2, t2, 0x1
        dli     t6, 0x0
	bnez	t0, glvl_req_set0
	nop

#ifdef  PRINT_DDR_LEVELING   //print registers
    PRINTSTR("\r\nThe MC param after gate leveling 1 to 0 is:\r\n")
	PRINT_THE_MC_PARAM
#endif

/* unknown reason to reset init_start */
reset_init_start:
    dli     t1, 0x18
    or      t1, t1, t8
	dli		a0, 0x0
    sb      a0, 0x0(t1)

    dli     a0, 0x1
    sb      a0, 0x0(t1)

wait_init_done:
    dli     t1, 0x160
    or      t1, t1, t8
    lb      a0, 0x3(t1)
    beqz    a0, wait_init_done
    nop

/* 0 to 1 */
    GET_NUMBER_OF_SLICES
	dli		t1, 0x20
	or		t1, t1, t8
	dli		t2, 0x180
	or		t2, t2, t8
	dli		s7, GATE_FILTER_LENGTH
glvl_req_set1:
#ifdef LVL_DEBUG
	PRINTSTR("\r\ngate leveling req\r\n")
#endif
	dli		a0, 0x1
	sb		a0, 0x181(t8)
	dli		a0, 0x0
	sb		a0, 0x181(t8)

glvl_done_sampling1:
    lb		a0, 0x186(t8)
	beqz	a0, glvl_done_sampling1
	nop

glvl_resp_set1:
	lb		a0, 0x7(t2)
    GET_GLVL_RESP_2BIT_a0
	bnez	a0, glvl_resp_set1_done
	nop
	dli		s7, GATE_FILTER_LENGTH

dll_gate_add1:
	lb		a0, OFFSET_DLL_GATE(t1)
	daddu	a0, a0, 0x1
	dli		t4, 0x7f
	and		a0, a0, t4
#ifdef DDR_DLL_BYPASS
	lb		t4, 0x4(t8)
	daddu	t4, t4, 0x2
1:
	blt		a0, t4, 2f
	nop
	dsubu	a0, a0, t4
	b		1b
	nop
2:
	ori		a0, 0x80
#endif
	sb		a0, OFFSET_DLL_GATE(t1)
	dli		t3, 0x7f
	and		a0, a0, t3
	bnez	a0, 1f
	nop

	lb		a0, OFFSET_RDOE_BEGIN(t1)
	daddu	a0, a0, 0x1
	sb		a0, OFFSET_RDOE_BEGIN(t1)
	lb		a0, OFFSET_RDOE_END(t1)
	daddu	a0, a0, 0x1
	sb		a0, OFFSET_RDOE_END(t1)
	RDOE_SUB_TRDDATA_ADD
/*
	lb		a0, OFFSET_ODTOE_BEGIN(t1)
	daddu	a0, a0, 0x1
	sb		a0, OFFSET_ODTOE_BEGIN(t1)
	lb		a0, OFFSET_ODTOE_END(t1)
	daddu	a0, a0, 0x1
	sb		a0, OFFSET_ODTOE_END(t1)
*/
1:
        bgt             t6, MDL_CNT,  leveling_failed
        nop
        daddu           t6, t6, 0x1
	b 		glvl_req_set1
	nop

glvl_resp_set1_done:
#ifdef LVL_DEBUG
	PRINTSTR("\r\n gate leveling 1 is found @ slice")
	dli		a0, 0x8
	dsubu	a0, a0, t0
	bal 	hexserial
	nop
#endif
	dsubu	s7, s7, 0x1
	bnez	s7, dll_gate_add1
	nop
	dli		s7, GATE_FILTER_LENGTH

//return the more add
	lb		a0, OFFSET_DLL_GATE(t1)
	and		a0, a0, 0x7f
	dli		t4, GATE_FILTER_LENGTH
	dsubu	t4, t4, 0x1
	blt		a0, t4, 1f	// if a0 less then t4, sub t4
	nop
	dsubu	a0, a0, t4
#ifdef DDR_DLL_BYPASS
	ori		a0, a0, 0x80
#endif
	sb		a0, OFFSET_DLL_GATE(t1)
	b		2f
	nop
1:
	dli		a1, 0x80
#ifdef DDR_DLL_BYPASS
	lb		a1, 0x4(t8)
	daddu	a1, a1, 0x2
#endif
	lb		a0, OFFSET_DLL_GATE(t1)
	dli		t4, GATE_FILTER_LENGTH
	dsubu	t4, t4, 0x1
	daddu	a0, a0, a1
	dsubu	a0, a0 ,t4
	sb		a0, OFFSET_DLL_GATE(t1)

	lb		a0, OFFSET_RDOE_BEGIN(t1)
	dsubu	a0, a0, 0x1
	sb		a0, OFFSET_RDOE_BEGIN(t1)
	lb		a0, OFFSET_RDOE_END(t1)
	dsubu	a0, a0, 0x1
	sb		a0, OFFSET_RDOE_END(t1)
	RDOE_ADD_TRDDATA_SUB
/*
	lb		a0, OFFSET_ODTOE_BEGIN(t1)
	dsubu	a0, a0, 0x1
	sb		a0, OFFSET_ODTOE_BEGIN(t1)
	lb		a0, OFFSET_ODTOE_END(t1)
	dsubu	a0, a0, 0x1
	sb		a0, OFFSET_ODTOE_END(t1)
*/
2:

	dsubu	t0, t0, 0x1
	daddu	t1, t1, 0x20
	daddu	t2, t2, 0x1
        dli     t6, 0x0
	bnez	t0, glvl_req_set1
	nop



#ifdef  PRINT_DDR_LEVELING   //print registers
    PRINTSTR("\r\nThe MC param after gate leveling 0 to 1 is:\r\n")
	PRINT_THE_MC_PARAM
#endif

#ifdef	PREAMBLE_CHECK_DEBUG

    GET_NUMBER_OF_SLICES
    move    s7, t0 
//	dli	s7, 0x5
	dli		t1, 0x28
	or 		t1, t1, t8
	dli		t2, 0x180
	or		t2, t2, t8

        dli     t5,     0x0
preamble_check_init:
/* check the preamble exist */
	PRINTSTR("\r\nPREAMBLE CHECK!!\r\n")
// set the gate signal 0.75 period before
    dli     a1, 0x0
	dli 	s6,	PREAMBLE_LENGTH_3A9 //s6 represents 0.75 period to be checked	
	dli		a3, 0x80
	dli		t4, 0x0
	or		t4, t4, t8
	lb		a0, 0x0(t4)
	beq		a0, 0x2, 1f
	nop
	dli		s6, PREAMBLE_LENGTH_3A8
1:
#ifdef DDR_DLL_BYPASS
	lb		a2, 0x4(t8)
	and		a2, a2, 0x7f
	daddu	a2, a2, 0x2
	move	a3, a2
	dsrl	a2, a2, 0x2
	dsubu	a2, a3, a2
	dli		t4, 0x7f
	and		a2, a2, t4
	move 	s6, a2
#endif

	lb		a0, 0x7(t1)	// if the rd_oe > 4 the set the rd_oe = 3
	blt		a0, 0x4, 1f
	nop
	dli		a0, 0x3
	sb		a0, 0x7(t1)
1:
	lb		a0, 0x6(t1)
	blt		a0, 0x4, 1f
	nop
	dli		a0, 0x3
	sb		a0, 0x6(t1)
1:

	lb		a0, 0x10(t1)
	and		a0, a0, 0x7f
	bgeu	a0, s6, 1f
	nop
	daddu	a0, a0, a3
	dsubu	a0, a0, s6
#if 0
	move	t4, a0
	bal	hexserial
	nop
2:
	bal	hexserial
	nop
	lb	a0, 0x10(t1)
	daddu	a0, a0, 0x1
	sb	a0, 0x10(t1)
	bne	a0, t4, 2b
	nop
#endif
#if 1	
#ifdef DDR_DLL_BYPASS
	ori		a0, a0, 0x80
#endif
	sb		a0, 0x10(t1)
#endif
	lb		a0, 0x7(t1)
	dsubu	a0, a0, 0x1
	sb		a0, 0x7(t1)
	lb		a0, 0x6(t1)
	dsubu	a0, a0, 0x1
	sb		a0, 0x6(t1)
	RDOE_ADD_TRDDATA_SUB
	b	3f
	nop
1:
	dsubu	a0, a0, s6
#ifdef DDR_DLL_BYPASS
	ori		a0, a0, 0x80
#endif
	sb		a0, 0x10(t1)
3:
/*	dli	a0, 0xa1
	sb	a0, 0x10(t1)*/
    dli     t4, 0x180
    or      t4, t4, t8
    li		a0, 0x1
	sb      a0, 0x1(t4)
    li		a0, 0x0
	sb		a0, 0x1(t4)
    li		a0, 0x1
	sb      a0, 0x1(t4)
    li		a0, 0x0
	sb		a0, 0x1(t4)
	
	dli 	t3, 0x2
	dli		t6, 0x5
	and		s6, s6, 0x7f
	dsubu	s6, s6, 0x6
	b 		glvl_redo_req_set_0
	nop
glvl_check_preamble:


	dsubu	s6, s6, 0x1
	bnez	s6, 1f
	nop
	daddu	s6, s6, 0x1
1:

    lb      a0, 0x7(t2)
    GET_GLVL_RESP_2BIT_a0
	bnez	a0, test_continuous5_0
	nop
#ifdef LVL_DEBUG
	PRINTSTR("The 1 is not found\r\n")
#endif
	lb		a0, 0x10(t1)
	daddu	a0, a0, 0x1
	dli		t4, 0x7f
	and 	a0, a0, t4
#ifdef DDR_DLL_BYPASS
	lb		t4, 0x4(t8)
	daddu	t4, t4, 0x2
1:
	blt		a0, t4, 2f
	nop
	dsubu	a0, a0, t4
	b		1b
	nop
2:
	ori		a0, 0x80
#endif
	sb		a0, 0x10(t1)
	dli		t4, 0x7f
	and		a0, a0, t4
	bnez	a0,	1f
	nop
	
	lb		a0, 0x6(t1)
	daddu	a0, a0, 0x1
	sb		a0, 0x6(t1)
	lb		a0, 0x7(t1)
	daddu	a0, a0, 0x1
	sb		a0, 0x7(t1)
	lb		a0, 0x7(t1)
	RDOE_SUB_TRDDATA_ADD
1:
	dli		t6, 0x5
	b 		glvl_redo_req_set_0
	nop

test_continuous5_0:
	dsubu	t6, t6, 0x1
	bnez	t6, 1f
	nop
	beq		s6, 0x1, glvl_check_preamble_end
	nop
        bgt             t5, GCL_CNT,  leveling_failed
        nop
        daddu           t5, t5, 0x1
	b		glvl_check_preamble_fail
	nop
1:
#ifdef PRINT_PREAMBLE_CHECK 
	PRINTSTR("The 1 found in preamble test at position")
	move	a0, s6
	bal 	hexserial
	nop
	PRINTSTR("\r\ncontinued 1 is found")
	move	a0, t6
	bal 	hexserial
	nop
	PRINTSTR("\r\n")
#endif

	lb		a0, 0x10(t1)
	daddu	a0, a0, 0x1
	dli		t4, 0x7f
	and 	a0, a0, t4
#ifdef DDR_DLL_BYPASS
	lb		t4, 0x4(t8)
	daddu	t4, t4, 0x2
1:
	blt		a0, t4, 2f
	nop
	dsubu	a0, a0, t4
	b		1b
	nop
2:
	ori		a0, 0x80
#endif
	sb		a0, 0x10(t1)
	dli		t4, 0x7f
	and		a0, a0, t4
	bnez	a0,	1f
	nop
	
	lb		a0, 0x6(t1)
	daddu	a0, a0, 0x1
	sb		a0, 0x6(t1)
	lb		a0, 0x7(t1)
	daddu	a0, a0, 0x1
	sb		a0, 0x7(t1)
	lb		a0, 0x7(t1)
	RDOE_SUB_TRDDATA_ADD	
1:
	b		glvl_redo_req_set_0
	nop

glvl_check_preamble_fail:
	PRINTSTR("\r\nThe preamble check not found @")
	move	a0, s6
	bal 	hexserial
	nop
	PRINTSTR("training again ... \r\n")

	dli		s6, 0x0
	lb		a0, 0x6(t1)
	dsubu	a0, a0, 0x1
	sb		a0, 0x6(t1)
	lb		a0, 0x7(t1)
	dsubu	a0, a0, 0x1
	sb		a0, 0x7(t1)
	bnez	a0, 1f
	nop
	PRINTSTR("\r\nThe rd_oe become 0 in the preamble check!\r\n")
	RDOE_ADD_TRDDATA_SUB
1:

	
	dli		t3, 0x0
glvl_redo_req_set_0:
    bgt     a1, MDL_CNT,  leveling_failed
    nop
    daddu   a1, a1, 0x1
    dli     t4, 0x180
    or      t4, t4, t8
    dli     a0, 0x1
    sb      a0, 0x1(t4)
    dli		a0, 0x0
    sb      a0, 0x1(t4)
	
1:						//glvl_done_sampling
    dli     t4, 0x180
    or      t4, t4, t8
    lb      a0, 0x6(t4)
    bne     a0, 0x1, 1b
    nop

#ifdef LVL_DEBUG
	PRINTSTR("\r\npreamble req\r\nrd_oe is")
	ld	a0, 0x0(t1)
	dsrl	a0, a0, 48
	and	a0, a0, 0xffff
	bal	hexserial
	nop	
	lb	a0, 0x1c0(t8)
	bal	hexserial
	nop
	PRINTSTR("\r\n t1 & t2 is")
	move	a0, t1
	bal	hexserial
	nop
	move	a0, t2
	bal	hexserial
	nop
	PRINTSTR("\r\n 0x118")
	lb	a0, 0x118(t8)
	bal	hexserial
	nop
#endif

	beq		t3, 0x1, glvl_redo_resp_set1_0	
	nop

	beq		t3, 0x2, glvl_check_preamble
	nop


	dli		t3, 0x1
#ifdef LVL_DEBUG
	ld	a0, 0x188(t8)
	dsrl	a0, a0, 32
	bal	hexserial
	nop
#endif
    lb      a0, 0x7(t2)
    GET_GLVL_RESP_2BIT_a0
    beq     a0, 0x0, glvl_redo_set0_end
    nop
#ifdef LVL_DEBUG
	PRINTSTR("\r\nglvl redo set 0 add\r\n")
#endif	
	lb		a0, 0x10(t1)
	daddu	a0, a0, 0x1
	dli		t4, 0x7f
	and 	a0, a0, t4
#ifdef DDR_DLL_BYPASS
	lb		t4, 0x4(t8)
	daddu	t4, t4, 0x2
1:
	blt		a0, t4, 2f
	nop
	dsubu	a0, a0, t4
	b		1b
	nop
2:
	ori		a0, 0x80
#endif
	sb		a0, 0x10(t1)
#ifdef LVL_DEBUG
	bal	hexserial
	nop
	lb		a0, 0x10(t1)
#endif
	dli		t4, 0x7f
	and		a0, a0, t4
	dli		t3, 0x0
	bnez	a0,	glvl_redo_set0_end
	nop

#ifdef LVL_DEBUG
	PRINTSTR("\r\nrd_oe add 1\r\n")
#endif
    /* rd_oe_begin and rd_oe_end add 1 */
    ld      a0, 0x0(t1)
    dli     t4, 0x0101000000000000
    daddu   a0, a0, t4
    sd      a0, 0x0(t1)
	lb		a0, 0x7(t1)
	RDOE_SUB_TRDDATA_ADD
    /* odt_oe_begin and odt_oe_end add 1 */
    ld      a0, 0x8(t1)
    dli     t4, 0x0000000001010000
    daddu   a0, a0, t4
    sd      a0, 0x8(t1)

glvl_redo_set0_end:
	b		glvl_redo_req_set_0
	nop

glvl_redo_resp_set1_0:
#ifdef LVL_DEBUG
	PRINTSTR("\r\nglvl redo resp set 1\r\n")
#endif	
    lb      a0, 0x7(t2)
    GET_GLVL_RESP_2BIT_a0
    bnez    a0, preamble_check_init
    nop

	lb		a0, 0x10(t1)
	daddu	a0, a0, 0x1
	dli		t4, 0x7f
	and 	a0, a0, t4
#ifdef DDR_DLL_BYPASS
	lb		t4, 0x4(t8)
	daddu	t4, t4, 0x2
1:
	blt		a0, t4, 2f
	nop
	dsubu	a0, a0, t4
	b		1b
	nop
2:
	ori		a0, 0x80
#endif
	sb		a0, 0x10(t1)
#ifdef LVL_DEBUG
	bal	hexserial
	nop
	lb		a0, 0x10(t1)
#endif
	dli		t4, 0x7f
	and		a0, a0, t4
	bnez	a0,	1f
	nop

#ifdef LVL_DEBUG
	PRINTSTR("\r\nrd oe add 1 @ glvl redo add\r\n")
#endif
    /* rd_oe_begin and rd_oe_end add 1 */
    ld      a0, 0x0(t1)
    dli     t4, 0x0101000000000000
    daddu   a0, a0, t4
    sd      a0, 0x0(t1)
	lb		a0, 0x7(t1)
	RDOE_SUB_TRDDATA_ADD
    /* odt_oe_begin and odt_oe_end add 1 */
    ld      a0, 0x8(t1)
    dli     t4, 0x0000000001010000
    daddu   a0, a0, t4
    sd      a0, 0x8(t1)

1:
	
	b		glvl_redo_req_set_0
	nop


glvl_check_preamble_end:
#if 0  //PRINT_PREAMBLE_CHECK   //print registers
    PRINTSTR("\r\nThe MC param after preamble check is:\r\n")
	PRINT_THE_MC_PARAM
#endif
	dli		s6, 0x0
	PRINTSTR("\r\nThe preamble check success\r\n")
	
	lb		a0, 0x7(t1)
	blt		a0, 0x4, 1f
	nop
	dsubu	a0, a0, 0x4
	sb		a0, 0x7(t1)
	sb		a0, 0x6(t1)
	RDOE_ADD_TRDDATA_SUB
1:
	dli		a3, 0x80
#ifdef DDR_DLL_BYPASS
	lb		a3, 0x4(t8)
	daddu	a3, a3, 0x2
	and		a3, a3, 0x7f
#endif
	lb		a0, 0x10(t1)
	and		a0, a0, 0x7f
	bgeu	a0, 0x4, 1f
	nop
	daddu	a0, a0, a3
	dsubu	a0, a0, 0x4
#ifdef DDR_DLL_BYPASS
	ori		a0, a0, 0x80
#endif
	sb		a0, 0x10(t1)

	lb		a0, 0x7(t1)
	dsubu	a0, a0, 0x1
	sb		a0, 0x7(t1)
	lb		a0, 0x6(t1)
	dsubu	a0, a0, 0x1
	sb		a0, 0x6(t1)
	RDOE_ADD_TRDDATA_SUB
1:
	dsubu	a0, a0, 0x4
#ifdef DDR_DLL_BYPASS
	ori		a0, a0, 0x80
#endif
	sb		a0, 0x10(t1)
	
#if 1
/* unknown reason to reset init_start */
    dli     t4, 0x18
    or      t4, t4, t8
    dli     a0, 0x0
    sb      a0, 0x0(t4)

    dli     t4, 0x18
    or      t4, t4, t8
    dli     a0, 0x1
    sb      a0, 0x0(t4)
1:
    dli     t4, 0x160
    or      t4, t4, t8
    lb      a0, 0x3(t4)
    beqz    a0, 1b
    nop
#endif

#if 0
get_burst_length: //save in t9
	dli		t4, 0x168
	or		t4, t4, t8
	lb		t9, 0x4(t4)
	daddu	t9, t9, 0x1
	dsrl	t9, t9, 0x1

	dli		t4, 0x180//send glvl request
	or		t4, t4, t8
	dli		a0, 0x1
	sb		a0, 0x1(t4)
1:
	lb		a0, 0x6(t4) //glvl done
	bne		a0, 0x1, 1b
	nop
	lb		s3, 0x7(t2)
	
	dli		t4, 0x180
	or		t4, t4, t8
	dli		a0, 0x1
	sb		a0, 0x1(t4)
1:
	lb		a0, 0x6(t4)
	bne		a0, 0x1, 1b
	nop
	lb		t6, 0x7(t2)

//glvl response check
	dli		t4, 0x1c
	and		s3, s3, t4
	and		t6, t6, t4
	dsrl	s3, s3, 0x2
	dsrl	t6, t6, 0x2
	blt		s3, 0x4, 1f
	nop
	or		t6, t6, 0x8
1:
	dsubu	t6, t6, s3
	beq		t6, t9, glvl_last_check_end
	nop
	
	lb		a0, 0x7(t1)
	dsubu	a0, a0, 0x1
	sb		a0, 0x7(t1)
	lb		a0,	0x6(t1)
	dsubu	a0, a0, 0x1
	sb		a0, 0x6(t1)
	RDOE_ADD_TRDDATA_SUB
	PRINTSTR("\r\nThe edges number is incorrect!\r\n")
	b		preamble_check_init
	nop
#endif
glvl_last_check_end:
	daddu	t1, t1, 0x20
	daddu	t2, t2, 0x1
	dsubu	s7, s7, 0x1
        dli     t5,     0x0
	bnez	s7, preamble_check_init
	nop
#endif

/* set rddqs_lt_half */
	GET_NUMBER_OF_SLICES
	dli		t1, 0x20
	or		t1, t1, t8
rddqs_lt_half_set:
#ifdef LVL_DEBUG
	PRINTSTR("\r\nsetting rddqs lt_half\r\n")
#endif
	lb		a0, OFFSET_DLL_GATE(t1)
	dli		t4, 0x7f
	and		a0, a0, t4
#ifdef DDR_DLL_BYPASS
    dsll    a0, a0, 0x7 // x 128
    lw      t5, 0x4(t8) //get dll_ck value, store at t5
    daddu   t5, t5, 0x2
    divu    a0, a0, t5 //get dll_gate, no bypass mode
#endif
 	lb		a1, OFFSET_DLL_WRDQ(t1)
	daddu	a0, a0, a1
	and		a0, a0, t4
#if 0
	move	a1, a0
	bal		hexserial
	nop
	move	a0, a1
#endif
	bgeu	a0, RDDQS_LTHF_STD1, rddqs_lthalf_set1
	nop
	bltu	a0,	RDDQS_LTHF_STD2, rddqs_lthalf_set1
	nop
	b		rddqs_lthalf_set0
	nop
rddqs_lthalf_set0:
	dli		a0, 0x0
	sb		a0, OFFSET_RDDQS_LTHF(t1)
	b		1f
	nop
rddqs_lthalf_set1:
	dli		a0, 0x1
	sb		a0, OFFSET_RDDQS_LTHF(t1)
1:
	daddu	t1, t1, 0x20
	dsubu	t0, t0, 0x1
	bnez	t0, rddqs_lt_half_set
	nop
	
#if 1
/* unknown reason to reset init_start */
    dli     t4, 0x18
    or      t4, t4, t8
    dli     a0, 0x0
    sb      a0, 0x0(t4)

    dli     t4, 0x18
    or      t4, t4, t8
    dli     a0, 0x1
    sb      a0, 0x0(t4)
1:
    dli     t4, 0x160
    or      t4, t4, t8
    lb      a0, 0x3(t4)
    beqz    a0, 1b
    nop
#endif

#if 1
	GET_NUMBER_OF_SLICES
	dli		t1, 0x20
	or		t1, t1, t8
dll_gate_set_loop:
    beqz     t0, gate_sub_end
    nop
#ifdef LVL_DEBUG
	PRINTSTR("\r\n setting dll_gate_sub \r\n")
#endif
#ifdef DDR_DLL_BYPASS
    lb      a2, 0x4(t8) //dll_value_ck
    daddu   a2, a2, 0x2
	move	a3, a2
    dsrl    a2, a2, 0x2
    dli     t4, 0xff 
    and     a2, a2, t4
#else
	dli		a3, 0x80
    dli     a2, DLL_GATE_SUB
#endif
    lb      a0, OFFSET_DLL_GATE(t1)
    and		a0, a0, 0x7f
	bgeu    a0, a2, dll_gate_sub20
    nop
#ifdef DDR_DLL_BYPASS
    ori     a0, a0, 0x80
	dsubu	a0, a0, a2
	daddu	a0, a0, a3
#else
	daddu	a0, a0, a3
	dsubu	a0, a0, a2
#endif
    sb      a0, OFFSET_DLL_GATE(t1)

	lb		a0, OFFSET_RDOE_BEGIN(t1)
	dsubu	a0, a0, 0x1
	sb		a0, OFFSET_RDOE_BEGIN(t1)
	lb		a0, OFFSET_RDOE_END(t1)
	dsubu	a0, a0, 0x1
	sb		a0, OFFSET_RDOE_END(t1)
	RDOE_ADD_TRDDATA_SUB
/*
	lb		a0, OFFSET_ODTOE_BEGIN(t1)
	dsubu	a0, a0, 0x1
	sb		a0, OFFSET_ODTOE_BEGIN(t1)
	lb		a0, OFFSET_ODTOE_END(t1)
	dsubu	a0, a0, 0x1
	sb		a0, OFFSET_ODTOE_END(t1)
*/
	daddu	t1, t1, 0x20
	dsubu	t0, t0, 0x1
    b       dll_gate_set_loop
    nop
dll_gate_sub20:
    dsubu    a0, a0, a2
#ifdef DDR_DLL_BYPASS
    ori     a0, a0, 0x80
#endif
    sb      a0, OFFSET_DLL_GATE(t1)
	daddu	t1, t1, 0x20
	dsubu	t0, t0, 0x1
    b       dll_gate_set_loop
    nop
gate_sub_end:
#endif

#ifdef NO_EDGE_CHECK	
#else

#if 1
/* unknown reason to reset init_start */
    dli     t1, 0x18
    or      t1, t1, t8
    ld      a0, 0x0(t1)
    dli     t4, 0xffffffffffffff00
    and     a0, a0, t4
    sd      a0, 0x0(t1)

    dli     t1, 0x18
    or      t1, t1, t8
    ld      a0, 0x0(t1)
    dli     t4, 0xffffffffffffff00
    and     a0, a0, t4
    ori     a0, a0, 0x1
    sd      a0, 0x0(t1)

1:
    dli     t1, 0x160
    or      t1, t1, t8
    ld      a0, 0x0(t1)
    dli     t4, 0x00000000ff000000
    and     a0, a0, t4
    beqz    a0, 1b
    nop
#endif

    dli     t0, 0x0
rd_oe_sub:
    bgt     t0, GCL_CNT,  leveling_failed
    nop
    daddu   t0, t0, 0x1

get_burst_length_half: //save in t9
    dli     t1, 0x168
    or      t1, t1, t8
    ld      t9, 0x0(t1)
    dli     t4, 0x000000ff00000000
    and     t9, t9, t4
    daddu   t9, t9, 0x0000000100000000
    dsrl    t9, t9, 33 // div 2 
    

    dli     s6, 0x1
glvl_req_set_last_0:
#ifdef LVL_DEBUG
	PRINTSTR("\r\n last 0 req")
#endif
    dli     t1, 0x180
    or      t1, t1, t8
    ld      a0, 0x0(t1)
    dli     t4, 0xffffffffffff00ff
    and     a0, a0, t4
    ori     a0, a0, 0x100
    sd      a0, 0x0(t1)

    dli     a1, 0x1
glvl_done_sampling_last_0:
    dli     t1, 0x180
    or      t1, t1, t8
    ld      a0, 0x0(t1)
    dli     t4, 0x00ff000000000000
    and     a0, a0, t4
    dsrl    a0, a0, 48
    bne     a0, a1, glvl_done_sampling_last_0
    nop

glvl_resp_last_0:
    dli     s7, 0x0
    dli     t1, 0x180
    or      t1, t1, t8
    ld      s3, 0x0(t1) //save 0x180
    ld      s4, 0x8(t1) //save 0x188

glvl_req_set_last_1:
#ifdef LVL_DEBUG
	PRINTSTR("\r\n last 1 req")
#endif
    dli     t1, 0x180
    or      t1, t1, t8
    ld      a0, 0x0(t1)
    dli     t4, 0xffffffffffff00ff
    and     a0, a0, t4
    ori     a0, a0, 0x100
    sd      a0, 0x0(t1)

    dli     a1, 0x1
glvl_done_sampling_last_1:
    dli     t1, 0x180
    or      t1, t1, t8
    ld      a0, 0x0(t1)
    dli     t4, 0x00ff000000000000
    and     a0, a0, t4
    dsrl    a0, a0, 48
    bne     a0, a1, glvl_done_sampling_last_1
    nop

glvl_resp_last_1:
    dli     t1, 0x180
    or      t1, t1, t8
    ld      t2, 0x0(t1) //lvl_resp 0
    ld      t6, 0x8(t1) //lvl_resp 1-8

#if 1 // print the two sequence samples of leveling responds
#ifdef LVL_DEBUG
    move    t6, s3
    dli     a0, 0x180
    and     a0, a0, 0xfff
    bal     hexserial
    nop
    PRINTSTR(":  ")
    dsrl    a0, t6, 32
    bal     hexserial
    nop
    //PRINTSTR("  ")
    move    a0, t6
    bal     hexserial
    nop
    PRINTSTR("\r\n")

    move    t6, s4
    dli     a0, 0x188
    and     a0, a0, 0xfff
    bal     hexserial
    nop
    PRINTSTR(":  ")
    dsrl    a0, t6, 32
    bal     hexserial
    nop
    //PRINTSTR("  ")
    move    a0, t6
    bal     hexserial
    nop
    PRINTSTR("\r\n")

    dli     t1, 0x180
    or      t1, t1, t8
    ld      t6, 0x0(t1) //lvl_resp 0
    move    a0, t1
    and     a0, a0, 0xfff
    bal     hexserial
    nop
    PRINTSTR(":  ")
    dsrl    a0, t6, 32
    bal     hexserial
    nop
    //PRINTSTR("  ")
    move    a0, t6
    bal     hexserial
    nop
    PRINTSTR("\r\n")

    dli     t1, 0x188
    or      t1, t1, t8
    ld      t6, 0x0(t1) //lvl_resp 0
    move    a0, t1
    and     a0, a0, 0xfff
    bal     hexserial
    nop
    PRINTSTR(":  ")
    dsrl    a0, t6, 32
    bal     hexserial
    nop
    //PRINTSTR("  ")
    move    a0, t6
    bal     hexserial
    nop
    PRINTSTR("\r\n")
#endif
#endif

    dli     t1, 0x180
    or      t1, t1, t8
    ld      t2, 0x0(t1) //lvl_resp 0
    ld      t6, 0x8(t1) //lvl_resp 1-8

#if 1 //debug
glvl_resp_check_0:
    dli     t4, 0x1c00000000000000
    and     t3, t2, t4 //second sample
    and     t5, s3, t4 //first sample
    dsrl    t3, t3, 58
    dsrl    t5, t5, 58
    dli     t4, 0x4
    bge     t5, t4, 1f //lvl_resp[4:2] ge 0x4
    nop
    b       2f
    nop
1:  
    ori     t3, t3, 0x8
2:
    dsubu   t3, t3, t5
    bne     t3, t9, rd_oe_0_sub
    nop

glvl_resp_check_1:
    dli     t4, 0x000000000000001c
    and     t3, t6, t4 //second sample
    and     t5, s4, t4 //first sample
    dsrl    t3, t3, 2
    dsrl    t5, t5, 2
    dli     t4, 0x4
    bge     t5, t4, 1f
    nop
    b       2f
    nop
1:  
    ori     t3, t3, 0x8
2:
    dsubu   t3, t3, t5
    bne     t3, t9, rd_oe_1_sub
    nop

glvl_resp_check_2:
    dli     t4, 0x0000000000001c00
    and     t3, t6, t4 //second sample
    and     t5, s4, t4 //first sample
    dsrl    t3, t3, 10
    dsrl    t5, t5, 10
    dli     t4, 0x4
    bge     t5, t4, 1f
    nop
    b       2f
    nop
1:  
    ori     t3, t3, 0x8
2:
    dsubu   t3, t3, t5
    bne     t3, t9, rd_oe_2_sub
    nop

glvl_resp_check_3:
    dli     t4, 0x00000000001c0000
    and     t3, t6, t4 //second sample
    and     t5, s4, t4 //first sample
    dsrl    t3, t3, 18
    dsrl    t5, t5, 18
    dli     t4, 0x4
    bge     t5, t4, 1f
    nop
    b       2f
    nop
1:  
    ori     t3, t3, 0x8
2:
    dsubu   t3, t3, t5
    bne     t3, t9, rd_oe_3_sub
    nop

glvl_resp_check_4:
    dli     t4, 0x000000001c000000
    and     t3, t6, t4 //second sample
    and     t5, s4, t4 //first sample
    dsrl    t3, t3, 26
    dsrl    t5, t5, 26
    dli     t4, 0x4
    bge     t5, t4, 1f
    nop
    b       2f
    nop
1:  
    ori     t3, t3, 0x8
2:
    dsubu   t3, t3, t5
    bne     t3, t9, rd_oe_4_sub
    nop

glvl_resp_check_5:
    dli     t4, 0x0000001c00000000
    and     t3, t6, t4 //second sample
    and     t5, s4, t4 //first sample
    dsrl    t3, t3, 34
    dsrl    t5, t5, 34
    dli     t4, 0x4
    bge     t5, t4, 1f
    nop
    b       2f
    nop
1:  
    ori     t3, t3, 0x8
2:
    dsubu   t3, t3, t5
    bne     t3, t9, rd_oe_5_sub
    nop

glvl_resp_check_6:
    dli     t4, 0x00001c0000000000
    and     t3, t6, t4 //second sample
    and     t5, s4, t4 //first sample
    dsrl    t3, t3, 42
    dsrl    t5, t5, 42
    dli     t4, 0x4
    bge     t5, t4, 1f
    nop
    b       2f
    nop
1:  
    ori     t3, t3, 0x8
2:
    dsubu   t3, t3, t5
    bne     t3, t9, rd_oe_6_sub
    nop

glvl_resp_check_7:
    dli     t4, 0x001c000000000000
    and     t3, t6, t4 //second sample
    and     t5, s4, t4 //first sample
    dsrl    t3, t3, 50
    dsrl    t5, t5, 50
    dli     t4, 0x4
    bge     t5, t4, 1f
    nop
    b       2f
    nop
1:  
    ori     t3, t3, 0x8
2:
    dsubu   t3, t3, t5
    bne     t3, t9, rd_oe_7_sub
    nop

glvl_resp_check_8:
/* identify wheather there is ecc slice */
    dli     t1, 0x250
    or      t1, t1, t8
    lb      a0, 0x2(t1)
    dli     t1, 0x1
    and     a0, a0, t1
    bne     a0, t1, 3f //when rd_after_write enabled, the 9th device may don't need leveling
    nop

    dli     t4, 0x1c00000000000000
    and     t3, t6, t4 //second sample
    and     t5, s4, t4 //first sample
    dsrl    t3, t3, 58
    dsrl    t5, t5, 58
    dli     t4, 0x4
    bge     t5, t4, 1f
    nop
    b       2f
    nop
1:  
    ori     t3, t3, 0x8
2:
    dsubu   t3, t3, t5
    bne     t3, t9, rd_oe_8_sub
    nop

3:
    beq     s7, s6, rd_oe_sub
    nop

    b       gate_leveling_exit
    nop

rd_oe_0_sub:
#ifdef LVL_DEBUG
	PRINTSTR("\r\n rd_oe_0 and rd_odt_0 sub")
#endif
    dli     t1, 0x028
    or      t1, t1, t8
    ld      a0, 0x0(t1)
    dli     t2, 0x0101000000000000
    dsubu   a0, a0, t2
    sd      a0, 0x0(t1)

    dli     t1, 0x030
    or      t1, t1, t8
    ld      a0, 0x0(t1)
    dli     t2, 0x0000000001010000
    dsubu   a0, a0, t2
    sd      a0, 0x0(t1)
    b       glvl_resp_check_1
    dli     s7, 0x1 

rd_oe_1_sub:
#ifdef LVL_DEBUG
	PRINTSTR("\r\n rd_oe_1 and rd_odt_1 sub")
#endif
    dli     t1, 0x048
    or      t1, t1, t8
    ld      a0, 0x0(t1)
    dli     t2, 0x0101000000000000
    dsubu   a0, a0, t2
    sd      a0, 0x0(t1)

    dli     t1, 0x050
    or      t1, t1, t8
    ld      a0, 0x0(t1)
    dli     t2, 0x0000000001010000
    dsubu   a0, a0, t2
    sd      a0, 0x0(t1)
    b       glvl_resp_check_2
    dli     s7, 0x1 

rd_oe_2_sub:
#ifdef LVL_DEBUG
	PRINTSTR("\r\n rd_oe_2 and rd_odt_2 sub")
#endif
    dli     t1, 0x068
    or      t1, t1, t8
    ld      a0, 0x0(t1)
    dli     t2, 0x0101000000000000
    dsubu   a0, a0, t2
    sd      a0, 0x0(t1)

    dli     t1, 0x070
    or      t1, t1, t8
    ld      a0, 0x0(t1)
    dli     t2, 0x0000000001010000
    dsubu   a0, a0, t2
    sd      a0, 0x0(t1)
    b       glvl_resp_check_3
    dli     s7, 0x1 

rd_oe_3_sub:
#ifdef LVL_DEBUG
	PRINTSTR("\r\n rd_oe_3 and rd_odt_3 sub")
#endif
    dli     t1, 0x088
    or      t1, t1, t8
    ld      a0, 0x0(t1)
    dli     t2, 0x0101000000000000
    dsubu   a0, a0, t2
    sd      a0, 0x0(t1)

    dli     t1, 0x090
    or      t1, t1, t8
    ld      a0, 0x0(t1)
    dli     t2, 0x0000000001010000
    dsubu   a0, a0, t2
    sd      a0, 0x0(t1)
    b       glvl_resp_check_4
    dli     s7, 0x1 

rd_oe_4_sub:
#ifdef LVL_DEBUG
	PRINTSTR("\r\n rd_oe_4 and rd_odt_4 sub")
#endif
    dli     t1, 0x0a8
    or      t1, t1, t8
    ld      a0, 0x0(t1)
    dli     t2, 0x0101000000000000
    dsubu   a0, a0, t2
    sd      a0, 0x0(t1)

    dli     t1, 0x0b0
    or      t1, t1, t8
    ld      a0, 0x0(t1)
    dli     t2, 0x0000000001010000
    dsubu   a0, a0, t2
    sd      a0, 0x0(t1)
    b       glvl_resp_check_5
    dli     s7, 0x1 

rd_oe_5_sub:
#ifdef LVL_DEBUG
	PRINTSTR("\r\n rd_oe_5 and rd_odt_5 sub")
#endif
    dli     t1, 0x0c8
    or      t1, t1, t8
    ld      a0, 0x0(t1)
    dli     t2, 0x0101000000000000
    dsubu   a0, a0, t2
    sd      a0, 0x0(t1)

    dli     t1, 0x0d0
    or      t1, t1, t8
    ld      a0, 0x0(t1)
    dli     t2, 0x0000000001010000
    dsubu   a0, a0, t2
    sd      a0, 0x0(t1)
    b       glvl_resp_check_6
    dli     s7, 0x1 

rd_oe_6_sub:
#ifdef LVL_DEBUG
	PRINTSTR("\r\n rd_oe_6 and rd_odt_6 sub")
#endif
    dli     t1, 0x0e8
    or      t1, t1, t8
    ld      a0, 0x0(t1)
    dli     t2, 0x0101000000000000
    dsubu   a0, a0, t2
    sd      a0, 0x0(t1)

    dli     t1, 0x0f0
    or      t1, t1, t8
    ld      a0, 0x0(t1)
    dli     t2, 0x0000000001010000
    dsubu   a0, a0, t2
    sd      a0, 0x0(t1)
    b       glvl_resp_check_7
    dli     s7, 0x1 

rd_oe_7_sub:
#ifdef LVL_DEBUG
	PRINTSTR("\r\n rd_oe_7 and rd_odt_7 sub")
#endif
    dli     t1, 0x108
    or      t1, t1, t8
    ld      a0, 0x0(t1)
    dli     t2, 0x0101000000000000
    dsubu   a0, a0, t2
    sd      a0, 0x0(t1)

    dli     t1, 0x110
    or      t1, t1, t8
    ld      a0, 0x0(t1)
    dli     t2, 0x0000000001010000
    dsubu   a0, a0, t2
    sd      a0, 0x0(t1)
    b       glvl_resp_check_8
    dli     s7, 0x1 

rd_oe_8_sub:
#ifdef LVL_DEBUG
	PRINTSTR("\r\n rd_oe_8 and rd_odt_8 sub")
#endif
    dli     t1, 0x128
    or      t1, t1, t8
    ld      a0, 0x0(t1)
    dli     t2, 0x0101000000000000
    dsubu   a0, a0, t2
    sd      a0, 0x0(t1)

    dli     t1, 0x130
    or      t1, t1, t8
    ld      a0, 0x0(t1)
    dli     t2, 0x0000000001010000
    dsubu   a0, a0, t2
    sd      a0, 0x0(t1)
    b       rd_oe_sub
    dli     s7, 0x1 

#endif //debug
#endif

gate_leveling_exit:

#if 1 //set odt
	GET_NUMBER_OF_SLICES
	dli		t1, 0x0
	or		t1, t1, t8
odt_start_set:
	daddu	t1, t1, 0x20
	lb		a0, 0xe(t1)
	bnez	a0, 1f
	nop
	li		a0, 0x0
	sb		a0, 0x12(t1)
	sb		a0, 0x10(t1)
	b		odt_end_set
	nop
1:
	lb		a0, 0xc(t1)
	bgeu	a0, 0x2, 2f
	nop
	daddu	a0, a0, 0x2
	sb		a0, 0x10(t1)
	lb		a0, 0xe(t1)
	dsubu	a0, a0, 0x1
	sb		a0, 0x12(t1)
	b		odt_end_set
	nop
2:
	lb		a0, 0xc(t1)
	dsubu	a0, a0, 0x2
	sb		a0, 0x10(t1)
	lb		a0, 0xe(t1)
	sb		a0, 0x12(t1)
	b		odt_end_set
	nop

odt_end_set:
	lb		a0, 0xf(t1)
	bne		a0, 0x3, 1f
	nop
	li		a0, 0x3
	sb		a0, 0x13(t1)
	sb		a0, 0x11(t1)
	b		odt_set_loop
	nop
1:
	lb		a0, 0xd(t1)
	bgeu	a0, 0x2, 2f
	nop
	daddu	a0, a0, 0x2
	sb		a0, 0x11(t1)
	lb		a0, 0xf(t1)
	sb		a0, 0x13(t1)
	b		odt_set_loop
	nop
2:
	lb		a0, 0xd(t1)
	dsubu	a0, a0, 0x2
	sb		a0, 0x11(t1)
	lb		a0, 0xf(t1)
	daddu	a0, a0, 0x1
	sb		a0, 0x13(t1)
	b		odt_set_loop
	nop

odt_set_loop:
	dsubu	t0, t0, 0x1
	bnez	t0, odt_start_set
	nop
#endif	
    dli     t1, 0x180
    or      t1, t1, t8
    ld      a0, 0x0(t1)
    dli     t4, 0xffffffffffffff00
    and     a0, a0, t4
    sd      a0, 0x0(t1)


leveling_failed:
    dli     t1, 0x180
    or      t1, t1, t8
    ld      a0, 0x0(t1)
    dli     t4, 0xffffffffffffff00
    and     a0, a0, t4
    sd      a0, 0x0(t1)


//   dli      t1, 0x0000002020187803
//   sd       t1, 0xb8(t8)
/* unknown reason to reset init_start */
reset_init_start3:
    dli     t1, 0x18
    or      t1, t1, t8
    ld      a0, 0x0(t1)
    dli     t4, 0xffffffffffffff00
    and     a0, a0, t4
    sd      a0, 0x0(t1)

    dli     t1, 0x18
    or      t1, t1, t8
    ld      a0, 0x0(t1)
    dli     t4, 0xffffffffffffff00
    and     a0, a0, t4
    ori     a0, a0, 0x1
    sd      a0, 0x0(t1)

wait_init_done3:
    dli     t1, 0x160
    or      t1, t1, t8
    ld      a0, 0x0(t1)
    dli     t4, 0x00000000ff000000
    and     a0, a0, t4
    beqz    a0, wait_init_done3
    nop

#ifdef DDR_DLL_BYPASS //bypass dll_wrdqs, dll_wrdata and  dll_rddqs_p/n
    dli     t1, 0x0
    or      t1, t1, t8
    ld      a1, 0x0(t1)
    dli     t4, 0x0000ffff00000000
    and     a1, a1, t4 
    dsrl    a1, a1, 32 // dll_value store in a1
//    daddu   a1, a1, 0x2
    
/* identify wheather there is ecc slice */
    dli     t1, 0x250
    or      t1, t1, t8
    lb      a0, 0x2(t1)
    dli     t1, 0x1
    and     a0, a0, t1
    bne     a0, t1, 1f //when rd_after_write enabled, the 9th device may don't need leveling
    nop
    dli     t3, 0x9 //loop times
    b       2f
    nop

1:  
    dli     t3, 0x8 //loop times

2:  

    dli     t1, 0x38
    or      t1, t1, t8
3:	
    //set dll_wrdata
    lb      a0, 0x1(t1)
    dmulou  a0, a1, a0
    dsrl    a0, a0, 0x7
    ori     a0, a0, 0x80
    sb      a0, 0x1(t1)
    
    //set dll_wrdqs
    lb      a0, 0x2(t1)
    dmulou  a0, a1, a0
    dsrl    a0, a0, 0x7
    ori     a0, a0, 0x80
    sb      a0, 0x2(t1)
    
    //set dll_rddqs_p
    lb      a0, 0x3(t1)
    dmulou  a0, a1, a0
    dsrl    a0, a0, 0x7
    ori     a0, a0, 0x80
    sb      a0, 0x3(t1)
    
    //set dll_rddqs_n
    lb      a0, 0x4(t1)
    dmulou  a0, a1, a0
    dsrl    a0, a0, 0x7
    ori     a0, a0, 0x80
    sb      a0, 0x4(t1)
    
    subu    t3, t3, 0x1
    daddu   t1, t1, 0x20
    bnez    t3, 3b
    nop

#endif

#ifdef PM_DPD_FRE
//when rd_oe_start/stop is set to 0x2, the rddqs_lt_half should be reversed
//because the rd_oe_start/stop only changed in this file, and all the rd_oe_start/stop change at the same time, here we only consider the rd_oe_start/stop of slice0
    lh      a0, 0x2c(t8)
    dli     t4, 0x0202
    bne     t4, a0, 3f
    nop

/* identify wheather there is ecc slice */
	GET_NUMBER_OF_SLICES
1:

    dli     t1, 0x20
    or      t1, t1, t8

2:  
    lb      a0, 0x2(t1)
    xori    a0, 0x1
    sb      a0, 0x2(t1)
    daddu   t1, t1, 0x20
    dsubu   t0, t0, 0x1
    bnez    t0, 2b
    nop

3:
        
#endif




100:
#if 0
test_memory:
	dli     t0, 0x9000000000000000
	GET_NODE_ID_a0
	or      t0, t0, a0
   	dli     a0, 0x5555555555555555
    sd      a0, 0x0(t0)
    dli     a0, 0xaaaaaaaaaaaaaaaa
    sd      a0, 0x8(t0)
	dli     a0, 0x3333333333333333
	sd      a0, 0x10(t0)
	dli     a0, 0xcccccccccccccccc
	sd      a0, 0x18(t0)
	dli     a0, 0x7777777777777777
	sd      a0, 0x20(t0)
	dli     a0, 0x8888888888888888
	sd      a0, 0x28(t0)
	dli     a0, 0x1111111111111111
	sd      a0, 0x30(t0)
	dli     a0, 0xeeeeeeeeeeeeeeee
	sd      a0, 0x38(t0)

	dli     t5, 0x9000000000000000
    GET_NODE_ID_a0
	or      t5, t5, a0
	ld      t6, 0x30(t5)
    dli     t2, 0x5555555555555555
	beq		t6, t2, 2f
	nop
	ld		t6, 0x20(t5)
	beq		t6, t2, 2f
	nop
	ld		t6, 0x10(t5)
	beq		t6, t2, 2f
	nop
	ld		t6, 0x00(t5)
	beq		t6, t2, 3f
	nop
	PRINTSTR("\r\nthe memory test failed!\r\n")
	b		4f
	nop

2:
	dli		t1, 0x1d0
	or		t1, t1, t8
	lb		a0, 0x4(t1)
	dsubu	a0,	a0, 0x1
	sb		a0, 0x4(t1)
	b		test_memory
	nop
3:
	PRINTSTR("the memory test sucess!\r\n")
	nop
4:
#endif

//set pm_dll_bypass
    dli     t1, 0x1
    sb      t1, 0x19(t8)
//remove dll_close_disable and dll_reync_disable
    dli     t1, 0x0
    sb      t1, 0x7(t8)

    move    ra, s5
    jr      ra
    nop
    .end    ddr3_leveling
 
LEAF(hexserial4)
	move	a2, ra
	move	a1, a0
	li	a3, 0
1:
	rol	a0, a1, 4
	move	a1, a0
	and	a0, 0xf
	la	v0, hexchar
	addu	v0, s0
	addu	v0, a0
	bal	tgt_putchar
	lbu	a0, 0(v0)

	bnez	a3, 1b
	addu	a3, -1

	move	ra, a2
	j	ra
	nop
END(hexserial4)
